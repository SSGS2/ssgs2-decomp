// Ball preview and guideline actor: the previewed ball's model, ring and attached
// effects, and the guideline that draws a shot's predicted path.
//
// The preview draws into its own viewports using borrowed camera state, and the
// guideline runs its own trajectory simulation and fades its quad colours.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "iff/IffStruct.h"
#include "game/golfball.h"
#include "classes/WView.h"
#include "classes/CFxSequence.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))

#include "sdk_protos.h"
#include "game/wvector.h"
#include "classes/CGIBallRingRenderer.h"
#include "classes/CGIBallRenderer.h"
#include "classes/CGuideLine.h"
#include "classes/CQuadTree.h"
#include "classes/CScreen.h"
#include "classes/CFxBox.h"
#include "classes/CFxSpray.h"
#include "classes/CFx.h"

#define AsCGuideLine(p) ((CGuideLine*)(p))

#define AsCGIBallRingRenderer(p) ((CGIBallRingRenderer*)(p))

// Shared preview prefix: selected resource, three renderers and screen position.
struct BallPreviewResource {
    void* ballData;
    void* model;
};
struct BallPreviewStorage {
    char reserved00[0x0c];
    BallPreviewResource* selectedResource;
    void* primaryRenderer;
    void* secondaryRenderer;
    void* ringRenderer;
    float screenX, screenY;
    char reserved24[0x40];
    char renderState[0x68];
    char reservedcc[0x48];
    float viewMatrix[12];
    char viewport[0x18];
    char reserved15c[0x5c];
    float worldMatrix[16];
    char reserved1f8[0x10];
    char effects[1];
};
struct PreviewRendererStorage {
    void* vtable;
    char viewport[0x18];
    void* resource;
    char renderState[0x68];
};
struct PreviewBoneEntry {
    void* bone;
    void* transform;
};
struct PreviewBoneStorage {
    char reserved00[0x78];
    char worldMatrix[0x30];
};
struct PreviewTransformStorage {
    char reserved00[0x18];
    char matrix[0x30];
};
struct PreviewSprayAttachment {
    char reserved00[8];
    void* attachment;
    float x, y;
    char positionSource[0x10], rotationSource[0x10];
};
struct PreviewRingVertex {
    float x, y, z;
    unsigned color;
    float u, v;
};
struct PreviewRingGeometry {
    char reserved00[0x2c];
    PreviewRingVertex vertices[4];
};
struct BallConditionStringPool {
    char ringBitmap[20], percentageBitmap[20], modelFormat[7], spinActor[5];
    char centerBone[7], sprayExtension[4];
};
struct GuideLineQuadStorage {
    char reserved00[0xcb8];
    char vertices[4][0x38];
    void* drawOrder[5];
};
struct PreviewLightStorage {
    char reserved00[0x20];
    char light[1];
};
// Guideline effects start at self+0x34; the sequence pointer is entry+0x24.
// Unknown attachment words remain opaque; entries have a 0x28-byte stride.
struct GuideEffectEntry {
    char attachmentWords[0x24];
    void* sequence;
};
struct GuideEffectStorage {
    char prefix[0x34];
    GuideEffectEntry entries[80];
};
// Preserve the byte cursor's base while deriving its offset from the entry schema.
struct GuideEffectCursor {
    char prefix[0x34];
    GuideEffectEntry entry;
};
typedef char GuideEffectStrideCheck[sizeof(GuideEffectEntry) == 0x28 ? 1 : -1];
typedef char GuideEffectSequenceOffsetCheck[(int)&((GuideEffectCursor*)0)->entry.sequence == 0x58 ? 1 : -1];

// Each backend list consists of ID/value pairs, never six independent states.
struct PreviewStatePair {
    unsigned int stateId, value;
};
struct StrokeRenderPreset {
    int pairs[16][2];
};
struct StrokeStagePreset {
    PreviewStatePair pairs[6];
};
struct StrokeSamplerPreset {
    PreviewStatePair pairs[5];
};
struct StrokePointPresetView {
    char prefix[0x118];
    StrokeRenderPreset render;
    StrokeStagePreset textureStage;
    StrokeSamplerPreset sampler;
};
struct PreviewViewportCopy {
    int left, top, width, height;
    float minimumDepth, maximumDepth;
};
// Copy-only camera state: retain the original grouping of word transfers.
// This unit does not interpret these words; do not infer ID/value pairs here.
struct PreviewCopyWordPair {
    unsigned int first, second;
};
struct PreviewCameraStateCopy {
    int firstWord;
    PreviewCopyWordPair words04, words0c, words14, words1c, words24, words2c, words34;
    int word3c;
    PreviewCopyWordPair words40;
    int word48;
    float parameter4c, parameter50, parameter54, parameter58, parameter5c, parameter60, parameter64;
};
#define PREVIEW_LAYOUT_OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
PREVIEW_LAYOUT_OFF(BallPreviewStorage, selectedResource, 0xc);
PREVIEW_LAYOUT_OFF(BallPreviewStorage, screenX, 0x1c);
PREVIEW_LAYOUT_OFF(BallPreviewStorage, viewMatrix, 0x114);
PREVIEW_LAYOUT_OFF(BallPreviewStorage, viewport, 0x144);
PREVIEW_LAYOUT_OFF(BallPreviewStorage, worldMatrix, 0x1b8);
PREVIEW_LAYOUT_OFF(BallPreviewStorage, effects, 0x208);
PREVIEW_LAYOUT_OFF(GuideLineQuadStorage, drawOrder, 0xd98);
#undef PREVIEW_LAYOUT_OFF

enum BallPreviewConstants {
    kGroundGreen = 2,
    kTutorialShowStrokePoint = 3,
    kTutorialShowGuideLine = 6,
    kNormalRingSegments = 12,
    kPenaltyRingSegments = 16,
    kGuideEffectCapacity = 80,
    kGuideActorSkipFlags = 0x85806,
    kRingVertexOffset = (int)&((PreviewRingGeometry*)0)->vertices,
    kBallPreviewVertexFormat = 0x11f001,
    kTriangleFan = 0xa0,
    kBallModelFormatOffset = (int)&((BallConditionStringPool*)0)->modelFormat,
    kDefaultBoneNameOffset = (int)&((BallConditionStringPool*)0)->centerBone,
    kSprayExtensionOffset = (int)&((BallConditionStringPool*)0)->sprayExtension,
    kMessageInterfaceOffset = 0x18,
    kActorSecondaryVtableOffset = 0x3c,
    kPreviewEffectsOffset = (int)&((BallPreviewStorage*)0)->effects,
    kPreviewPutterGroup = 3,
    kPreviewLightIndex = 7,
    kPreviewLightCount = 8,
    kBeforeModelPresetA = 0x1F0,
    kBeforeModelPresetB = 0x208,
    kAfterModelPreset = 0x220
};

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_BallPreview;
extern const volatile float kFloat_0_805DECF4;
extern char __vt__10CGuideLine[];
extern void* g_resourceManager;
extern void* g_fx;
extern void* g_sharedDoc;
extern void* g_sceneManager;
extern char ballConditionStrings[];
extern char ballPreviewRenderStatePresets[];
extern char s_cameraTransform[];
extern float kFloat_1_PvsEffects, kFloat_255_PvsEffects;
extern float kFloat_0_PvsFxBox, kFloat_1_0_PvsEffects, kFloat_neg1_PvsEffects, kFloat_2_PreviewEffects;
extern float s_pvsRingRadius, kFloat_0_03_PreviewEffects;
extern float kFloat_6_2831855_ActorRotation, kFloat_0_1328125_ScreenBallPreview, kPvsRingAtlasUStep;
extern float kFloat_neg0_2268928_CourseView, kFloat_100_CourseView, kFloat_0_1_CourseView;
extern float kFloat_0_75_ScreenBallPreview, kFloat_1_5707964_PreviewProjection, kFloat_1_3333334_PreviewEffects,
    kFloat_10_BallPreview;
extern void* g_view;

extern void __ct__18NativeBase80201D18Fv(void*);
extern void IActor_AddSkipFlags(void*, int);
extern void ResourceManager_ReleaseId(void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void Vector_CrossProduct(void*, void*, void*);
extern void Normalize__7WVectorFv(void*);
extern void Vec3_Zero(void*);
extern void Matrix_SetAxisRotation(void*, float, int);
extern void Matrix_ConcatAffine(void*, void*, void*);
extern void Matrix_CopyAffine(void*, void*);
extern void* GolfBall__Fv();
extern void* ItemInventory_FindBallData(void*, void*);
extern void* __nw__FUl(unsigned int);
extern void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void* = 0);
extern void ScreenBallPreview_AttachBallEffects(void*);
extern void* Model_SetMatrixRecursive(void*, int, void*); /* (object, mode, matrix) */
extern void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
extern void Engine_GetLightEnabled(int, void*);
extern void Engine_SetLightEnabled(int, int);
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetLight(int, void*);
extern void* Camera_Apply(void*, float, float);
extern void* Camera_GetRenderState(void*);
extern void SetTransform(int, void*);
extern void gtl_GenericRenderer_SetProjectionMatrix(void*);
extern void gtl_GenericRenderer_SetViewMatrix(void*);
extern void* WPuppet_GetSkinMatrices(void*);
extern void ResourcePreloadQueue_AddRequest(void*, int, int, float, void*, float, const char*, int, int, int);
extern const char guideLineTrackStrings[];
extern void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern void* CQuadTree_GetInstance();
extern unsigned char GetGroundType__9CQuadTreeF7WVector(void*, void*);
extern void* GolfPlayer_GetEntryByIndex(void*, int);
extern void* ItemInventory_GetBallIdAt(void*, int);
extern void FxAttachment_Init(void*);
extern void* WPuppet_FindOrCacheBone(void*, void*);
extern void* WPuppet_FindNamedBoneEntry(void*, void*);
extern int CString_Compare(const char*, const char*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* CFxSequence_Create(void*, void*, int, int, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Camera_SetFov(void*, float);
extern void Camera_SetClip(void*, float, float, int);
extern void WView_SetScreenCenter(void*, float, float);
extern void WView_SetViewMatrix(void*, void*);
extern void Camera_BuildFrustumPlanes(void*);
extern void Matrix_ExtractAffineFrom4x4(void*, void*);
extern void Engine_GetViewport(void*);
extern void Engine_SetViewport(void*);
extern void* InstanceTable_Get();
extern void* Matrix_ExpandAffineTo4x4(void*, void*);
extern void Engine_SetType7Resource(int);
extern void Engine_SetFlexibleVertexFormat(int);
extern float Math_SinApprox(float);
extern float Math_CosApprox(float);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);
extern void DrawPrimitiveUP(int, int, void*, int);

// Render the selected preview with its viewport and camera, then restore the game camera.
void ScreenBallPreview_DrawStrokePoint(void* self) {
    StrokePointPresetView* base = (StrokePointPresetView*)ballPreviewRenderStatePresets;
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage < kTutorialShowStrokePoint)
        return;

    StrokeStagePreset textureStagePreset;
    StrokeRenderPreset renderPreset;
    StrokeSamplerPreset samplerPreset;

    char savedViewport[sizeof(PreviewViewportCopy)];
    // Phase 1: install render, combiner and sampler presets in separate domains.
    // Render: depth/write enabled, compare always, source-alpha blending, alpha
    // test GREATER than zero. Combiner: texture*previous color and alpha.
    renderPreset = base->render;
    textureStagePreset = base->textureStage;
    samplerPreset = base->sampler;
    Engine_SetRenderStateList(&renderPreset, 16);
    Engine_SetTextureStageStateList(0, &textureStagePreset, 6);
    Engine_SetTextureStageParameterList(0, &samplerPreset, 5);

    struct Tgt {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void f(void*, void*) DECOMP_LOCAL_VIRTUAL;
    };
    struct Cam {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual void g(int) DECOMP_LOCAL_VIRTUAL;
        char pad0[0x174];
        float nearZ;
        float farZ;
        char pad1[8];
        float w, h;
        char pad2[0x20];
        float fov;
    };
#define CAM ((Cam*)g_view)
    // Phase 2: borrow camera clip/FOV/viewport geometry for the screen preview.
    // Screen coordinates are pixels; matrix storage is world/camera space.
    float oldNear = CAM->nearZ;
    oldNear = (kFloat_0_PvsFxBox < oldNear) ? oldNear : kFloat_0_PvsFxBox;
    float oldFar = CAM->farZ;
    float w = CAM->w;
    float h = CAM->h;
    float fov0 = CAM->fov;
    float savedH = h;
    float savedW = w;
    float inv = kFloat_1_5707964_PreviewProjection * (h / w);
    float asp = kFloat_0_75_ScreenBallPreview * (savedW / savedH);
    float savedAsp = asp;
    float savedFov = fov0;
    float angle = savedFov / savedAsp;
    Camera_SetFov(g_view, kFloat_1_3333334_PreviewEffects * inv);
    Camera_SetClip(g_view, kFloat_0_1_CourseView, kFloat_10_BallPreview, 0);
    WView_SetScreenCenter(g_view, ((BallPreviewStorage*)self)->screenX, ((BallPreviewStorage*)self)->screenY);

    void* list = (*(void**)&((BallPreviewStorage*)self)->selectedResource);
    if (list) {
        if (*(void**)((char*)list + 4)) {
            WView_SetViewMatrix(g_view, ((BallPreviewStorage*)self)->viewMatrix);
            Camera_BuildFrustumPlanes(g_view);
            ((PreviewRendererStorage*)((BallPreviewStorage*)self)->secondaryRenderer)->resource =
                (*(void**)&((BallPreviewStorage*)self)->selectedResource);
            *(PreviewViewportCopy*)((PreviewRendererStorage*)((BallPreviewStorage*)self)->secondaryRenderer)->viewport =
                *(PreviewViewportCopy*)(((BallPreviewStorage*)self)->viewport);
            char obj20[0x30];
            *(PreviewCameraStateCopy*)((PreviewRendererStorage*)((BallPreviewStorage*)self)->secondaryRenderer)
                 ->renderState = *(PreviewCameraStateCopy*)(((BallPreviewStorage*)self)->renderState);
            Matrix_ExtractAffineFrom4x4(((BallPreviewStorage*)self)->worldMatrix, obj20);
            Tgt* target;
            void* camera = g_view;
            target = (Tgt*)((BallPreviewStorage*)self)->secondaryRenderer;
            // Phase 3: the nested renderer borrows the engine viewport temporarily.
            Engine_GetViewport(savedViewport);
            Engine_SetViewport(((PreviewRendererStorage*)target)->viewport);
            target->f(camera, obj20);
            Engine_SetViewport(savedViewport);
        }
    }
    // Phase 4: restore game clip/FOV/view, then rebuild its frustum. The render
    // state presets installed above are not part of this camera restoration.
    CAM->g(0);
    Camera_SetClip(g_view, oldNear, oldFar, 0);
    WView_SetViewMatrix(g_view, s_cameraTransform);
    Camera_SetFov(g_view, angle);
    Camera_BuildFrustumPlanes(g_view);
}

// The renderer owns no separately allocated members.
CGIBallRenderer::~CGIBallRenderer() {}

struct BallPreviewAttachment {
    float x, y;
    void* positionMatrix;
    char pad[0x0C];
    void* orientationMatrix;
    char pad2[0x0C];
};
struct BallEffectRecord {
    char pad0[0x50];
    void* ballIdWord;
    char pad1[0x08];
    char effectFiles[7][40];
    char effectBones[1][40];
};
struct BallPreviewResourcePair {
    BallEffectRecord* record;
    void* model;
};
struct BallPreviewEffectView {
    char pad0[0x0C];
    BallPreviewResourcePair* list;
    char pad1[0x0C];
    float x, y;
    char pad2[0x1E4];
    char sub[1];
};
// Volatile reload is intentional: callbacks can change selectedResource.
// A function wrapper changes register allocation in this routine.
#define SELECTED_BALL_EFFECT_PAIR(self) \
    (*(BallPreviewResourcePair* volatile*)&((BallPreviewStorage*)(self))->selectedResource)
enum PreviewSuppressedBallEffects {
    kPreviewSuppressedEffectSlot1 = 1,
    kPreviewSuppressedEffectSlot4 = 4,
    kPreviewSuppressedEffectSlot15 = 15,
    kPreviewSuppressedEffectSlot20 = 20
};
// Resolve borrowed bone matrix sources into a caller-owned attachment descriptor.
// Pair/bone scratch remain explicit to preserve CodeWarrior register lifetimes.
static inline void Preview_ResolveEffectBone(void* self, BallPreviewAttachment& attachment,
                                             BallPreviewResourcePair*& selectedPair, void*& boneOrEffect) {
    boneOrEffect = WPuppet_FindOrCacheBone(selectedPair->model, selectedPair->record->effectBones[0]);
    if (boneOrEffect)
        attachment.positionMatrix = ((PreviewBoneStorage*)boneOrEffect)->worldMatrix;
    else {
        void* namedBoneEntry;
        selectedPair = SELECTED_BALL_EFFECT_PAIR(self);
        BallEffectRecord* fallbackRecord = selectedPair->record;
        if (fallbackRecord->effectBones[0][0])
            namedBoneEntry = WPuppet_FindNamedBoneEntry(selectedPair->model, fallbackRecord->effectBones[0]);
        else
            namedBoneEntry =
                WPuppet_FindNamedBoneEntry(selectedPair->model, ballConditionStrings + kDefaultBoneNameOffset);
        if (namedBoneEntry) {
            if (*(void**)namedBoneEntry)
                attachment.positionMatrix =
                    ((PreviewBoneStorage*)((PreviewBoneEntry*)namedBoneEntry)->bone)->worldMatrix;
            attachment.orientationMatrix =
                ((PreviewTransformStorage*)((PreviewBoneEntry*)namedBoneEntry)->transform)->matrix;
        }
    }
}

// Attach the selected ball's first spray or sequence to its named model bone.
void ScreenBallPreview_AttachBallEffects(void* self) {
    // Phase 1: suppress inventory slots 1/4/15/20, then reject empty/short filenames.
    // Their item identities and the reason for suppression are not established by
    // this unit. These are slot indices, not literal encoded IFF ball IDs.
    // ballIdWord is an encoded IFF type ID compared through the accessor ABI,
    // not an allocation to dereference or release. Only effect slot zero is used.
    BallPreviewEffectView* preview = (BallPreviewEffectView*)self;
    if (!preview->list || !preview->list->record || !preview->list->model)
        return;
    if (SELECTED_BALL_EFFECT_PAIR(self)->record->ballIdWord ==
        ItemInventory_GetBallIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, kPreviewSuppressedEffectSlot1))
        return;
    void* excludedSlot4BallId =
        ItemInventory_GetBallIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, kPreviewSuppressedEffectSlot4);
    BallEffectRecord* record = SELECTED_BALL_EFFECT_PAIR(self)->record;
    if (record->ballIdWord == excludedSlot4BallId)
        return;
    if (record->ballIdWord ==
            ItemInventory_GetBallIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, kPreviewSuppressedEffectSlot15) ||
        record->ballIdWord ==
            ItemInventory_GetBallIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, kPreviewSuppressedEffectSlot20))
        return;
    if (!record->effectFiles[0][0])
        return;
    int filenameLength = strlen(record->effectFiles[0]);
    if (filenameLength < 5)
        return;
    // Phase 2: seed the attachment with screen coordinates and borrowed bone
    // matrix sources; named-entry fallback may provide separate sources.
    BallPreviewAttachment attachment;
    FxAttachment_Init(&attachment);
    attachment.x = preview->x;
    attachment.y = preview->y;
    BallPreviewResourcePair* selectedPair = SELECTED_BALL_EFFECT_PAIR(self);
    void* boneOrEffect;
    Preview_ResolveEffectBone(self, attachment, selectedPair, boneOrEffect);
    // Phase 3: choose spray by the three-character extension; otherwise create
    // a sequence. Registration transfers effect lifetime to the preview box.
    // Both paths store &attachment in the effect. Registration retains the effect
    // pointer; CFxBox::RegisterSpray/Sequence do not themselves copy this descriptor.
    // The subsequent virtual insert callback may consume it, but this unit does
    // not establish a lifetime guarantee. Keep the descriptor in this caller's
    // frame when splitting helpers; changing ownership is a separate behavior fix.
    selectedPair = SELECTED_BALL_EFFECT_PAIR(self);
    if (CString_Compare(((IFF_STRUCT::sBall*)selectedPair->record)->effectFiles[0] + filenameLength - 3,
                        ballConditionStrings + kSprayExtensionOffset) == 0) {
        selectedPair = SELECTED_BALL_EFFECT_PAIR(self);
        boneOrEffect =
            CFxSpray::CreateFromScript((CFxBox*)((char*)self + kPreviewEffectsOffset),
                                       (const char*)(((IFF_STRUCT::sBall*)selectedPair->record)->effectFiles[0]), 0, 0);
        if (!boneOrEffect)
            return;
        ((PreviewSprayAttachment*)boneOrEffect)->attachment = &attachment;
        ((PreviewSprayAttachment*)boneOrEffect)->x = attachment.x;
        ((PreviewSprayAttachment*)boneOrEffect)->y = attachment.y;
        memcpy(((PreviewSprayAttachment*)boneOrEffect)->positionSource, &attachment.positionMatrix, 0x10);
        memcpy(((PreviewSprayAttachment*)boneOrEffect)->rotationSource, &attachment.orientationMatrix, 0x10);
        ((CFxBox*)(preview->sub))->RegisterSpray(((CFxSpray*)(boneOrEffect)));
    } else {
        selectedPair = SELECTED_BALL_EFFECT_PAIR(self);
        boneOrEffect = CFxSequence_Create((char*)self + kPreviewEffectsOffset,
                                          ((IFF_STRUCT::sBall*)selectedPair->record)->effectFiles[0], 1, 0, 0);
        if (!boneOrEffect)
            return;
        (*(void**)&((CFxSequence*)boneOrEffect)->attachment) = &attachment;
        ((CFxSequence*)boneOrEffect)->attachmentPositionX = attachment.x;
        ((CFxSequence*)boneOrEffect)->attachmentPositionY = attachment.y;
        ((CFxBox*)(preview->sub))->RegisterSequence(((CFxSequence*)(boneOrEffect)));
    }
}

// The renderer owns no separately allocated members.
CGIBallRingRenderer::~CGIBallRingRenderer() {}

struct PreviewClubDescriptor {
    char reserved00[0xA];
    unsigned char group;
};
struct PreviewClubSelection {
    char reserved00[4];
    PreviewClubDescriptor* selectedClub;
};
struct RingAtlasRect {
    float a, b, c, d;
};
struct PreviewCameraMatrices {
    char pad[0x20];
    float m20[16];
    float m60[16];
};
struct PreviewTextureDevice {
    virtual void* u02();
    virtual void* u03();
    virtual void* u04();
    virtual void* u05();
    virtual void* u06();
    virtual void* u07();
    virtual void* u08();
    virtual void* u09();
    virtual void* u10();
    virtual void* u11();
    virtual void* u12();
    virtual void* u13();
    virtual void* u14();
    virtual void* u15();
    virtual void* u16();
    virtual void* u17();
    virtual void* u18();
    virtual void* u19();
    virtual void* u20();
    virtual void* u21();
    virtual void* u22();
    virtual void* u23();
    virtual void* u24();
    virtual void* u25();
    virtual void* u26();
    virtual void* u27();
    virtual void* u70(int);
    virtual void* u74(int);
};

// Draw ring quads and a texture-atlas label rounded up to a five-percent lie value.
// InstanceTable_Get()+4 holds the selected club descriptor; descriptor byte +0x0A
// is clubGroup (see ClubParameterView in club_selection_and_shot_params.cpp). Group 3 is the putter:
// force 12 ring segments and a 100-percent label regardless of lie penalty.
// source is the borrowed affine model transform; camera supplies view/projection.
void CGIBallRingRenderer::Draw(char* camera, void* source) {
    float matrix[16];
    if (((PreviewClubSelection*)InstanceTable_Get())->selectedClub->group == kPreviewPutterGroup)
        segmentCount = kNormalRingSegments;
    Camera_Apply(camera, kFloat_0_PvsFxBox, kFloat_0_PvsFxBox);
    PreviewCameraMatrices* view = (PreviewCameraMatrices*)Camera_GetRenderState(camera);
    SetTransform(1, view->m60);
    gtl_GenericRenderer_SetProjectionMatrix(view->m60);
    SetTransform(0, view->m20);
    gtl_GenericRenderer_SetViewMatrix(view->m20);
    Matrix_ExpandAffineTo4x4(source, matrix);
    SetTransform(11, matrix);
    Engine_SetType7Resource(0);
    Engine_SetFlexibleVertexFormat(kBallPreviewVertexFormat);

    char* vertex;
    int segmentIndex;
    int cornerIndex;
    int liePercent;
    if (((PreviewClubSelection*)InstanceTable_Get())->selectedClub->group == kPreviewPutterGroup)
        liePercent = 100;
    else
        liePercent = 100 - (*(int*)&liePenaltyPercent);

    float radiusMultiplier = kFloat_2_PreviewEffects;
    float fullTurn = kFloat_6_2831855_ActorRotation;
    float ringHalfHeight = kFloat_0_03_PreviewEffects;
    float one = kFloat_1_0_PvsEffects;
    float zero = kFloat_0_PvsFxBox;
    float atlasVStep = kFloat_0_1328125_ScreenBallPreview;
    float atlasUStep = kPvsRingAtlasUStep;
    char* ringBase = (char*)this;
    for (segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
        vertex = ringBase;
        for (cornerIndex = 0; cornerIndex < 4; ++cornerIndex) {
            float angle = fullTurn * ((float)(segmentIndex + (cornerIndex & 1)) / (float)segmentCount);
            if (segmentIndex % (segmentCount / 2) < 1)
                ((PreviewRingGeometry*)vertex)->vertices[0].x =
                    radiusMultiplier * (s_pvsRingRadius * Math_SinApprox(angle));
            else
                ((PreviewRingGeometry*)vertex)->vertices[0].x = s_pvsRingRadius * Math_SinApprox(angle);
            ((PreviewRingGeometry*)vertex)->vertices[0].z = s_pvsRingRadius * Math_CosApprox(angle);
            ((PreviewRingGeometry*)vertex)->vertices[0].y =
                ringHalfHeight * ((cornerIndex & 2) ? kFloat_neg1_PvsEffects : kFloat_1_0_PvsEffects);
            vertex += 0x18;
        }
        float savedVertex2Z;
        float savedVertex2X = vertex2X;
        savedVertex2Z = vertex2Z;
        vertex2X = vertex3X;
        vertex3X = savedVertex2X;
        vertex2Z = vertex3Z;
        vertex3Z = savedVertex2Z;
        if (segmentIndex % (segmentCount / 2) < 1) {
            if (liePercent > 95)
                liePercent = 100;
            else if (liePercent > 90)
                liePercent = 95;
            else if (liePercent > 85)
                liePercent = 90;
            else if (liePercent > 80)
                liePercent = 85;
            else if (liePercent > 75)
                liePercent = 80;
            else if (liePercent > 70)
                liePercent = 75;
            else
                liePercent = 70;
            int atlasIndex = (100 - liePercent) / 5;
            int atlasColumn = atlasIndex / 7;
            int atlasRow = atlasIndex % 7;
            float x0 = atlasUStep * (float)atlasColumn;
            float x1 = atlasUStep * (float)(atlasColumn + 1);
            float y0 = atlasVStep * (float)atlasRow;
            float y1 = atlasVStep * (float)(atlasRow + 1);
            vertex0U = x0;
            vertex1U = x1;
            vertex3U = x0;
            vertex0V = y0;
            vertex1V = y0;
            vertex3V = y1;
            vertex2U = x1;
            vertex2V = y1;
            RingAtlasRect tmp;
            tmp.a = x0;
            tmp.c = x1;
            tmp.b = y0;
            tmp.d = y1;
            PreviewTextureDevice* obj = (*(PreviewTextureDevice**)&((WView*)camera)->renderTarget);
            Engine_SetTextureStageResource(0, obj->u70(percentageTexture));
            obj = (*(PreviewTextureDevice**)&((WView*)camera)->renderTarget);
            Engine_SetTexturePalette(0, obj->u74(percentageTexture));
            DrawPrimitiveUP(kTriangleFan, 2, ringBase + kRingVertexOffset, 0x18);
        } else {
            vertex0U = zero;
            vertex0V = zero;
            vertex1U = one;
            vertex1V = zero;
            vertex2U = zero;
            vertex2V = one;
            vertex3U = one;
            vertex3V = one;
            PreviewTextureDevice* obj = (*(PreviewTextureDevice**)&((WView*)camera)->renderTarget);
            Engine_SetTextureStageResource(0, obj->u70(ringTexture));
            obj = (*(PreviewTextureDevice**)&((WView*)camera)->renderTarget);
            Engine_SetTexturePalette(0, obj->u74(ringTexture));
            DrawPrimitiveUP(kTriangleFan, 2, ringBase + kRingVertexOffset, 0x18);
        }
    }
}

// Install actor interfaces, clear the bitmap handle and set the guideline actor skip flags.
CGuideLine::CGuideLine() {
    __ct__18NativeBase80201D18Fv(this);
    vtbl = __vt__10CGuideLine;
    secondaryVtbl = (char*)vtbl + kActorSecondaryVtableOffset;
    bitmap = 0;
    IActor_AddSkipFlags(this, kGuideActorSkipFlags);
}

// Reserved load hook; guideline resources are acquired during preload.
void CGuideLine::OnLoad() {}

// Initialize this preview unit's cached full-turn angle.
void BallPreview_InitializeUnitConstants() {
    s_twoPi_BallPreview = kFloat_6_2831855;
}

struct GuideLineStateView {
    char reserved00[0x1C];
    unsigned char wasRequested;
    unsigned char requested;
    char reserved1e[0x0A];
    unsigned char enabled;
    char reserved29[3];
    float remainingTime;
    unsigned char simulating;
};

// Clear guideline request latches, enable state, lifetime and simulation state.
void CGuideLine::OnInit() {
    GuideLineStateView* self = (GuideLineStateView*)this;
    float value = kFloat_0_805DECF4;
    self->requested = 0;
    self->wasRequested = 0;
    self->enabled = 0;
    self->remainingTime = value;
    self->simulating = 0;
}

// On the green, simulate a released request, expire its timer and fade the guideline to full opacity.
void CGuideLine::OnProcess(float delta) {
    GuideLineStateView* self = (GuideLineStateView*)this;
    float deltaTime = delta;
    if (AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage < kTutorialShowGuideLine)
        return;
    if (((CGolfBall*)GolfBall__Fv())->m_groundType != 2)
        return;
    if (self->enabled != 0) {
    } else
        return;
    if (self->wasRequested && !self->requested)
        CGuideLine::SimulateTrajectory(self);
    float __t = self->remainingTime;
    float z = kFloat_0_805DECF4;
    if (__t > z) {
        __t -= deltaTime;
        self->remainingTime = __t;
        if (__t < z)
            SetEnabled(0);
    }
    float value = AsCGuideLine(self)->opacity;
    self->wasRequested = self->requested;
    self->requested = 0;
    if (value > kFloat_0_805DECF4 && value < kFloat_255_PvsEffects) {
        float nv = kFloat_255_PvsEffects * deltaTime + value;
        double clampedOpacity = (nv > kFloat_255_PvsEffects) ? kFloat_255_PvsEffects : nv;
        AsCGuideLine(self)->opacity = clampedOpacity;
        value = clampedOpacity;
        AsCGuideLine(self)->vertexColor0 = (((int)value) << 24) | 0x00FFFFFF;
        AsCGuideLine(self)->vertexColor1 = (((int)value) << 24) | 0x00FFFFFF;
        AsCGuideLine(self)->vertexColor2 = (((int)value) << 24) | 0x00FFFFFF;
        AsCGuideLine(self)->vertexColor3 = (((int)value) << 24) | 0x00FFFFFF;
    }
}

// Set up guideline quad pointers and effect slots, then queue and load its resources.
void CGuideLine::OnPreLoadInit() {
    ((GuideLineQuadStorage*)this)->drawOrder[0] = ((GuideLineQuadStorage*)this)->vertices[0];
    ((GuideLineQuadStorage*)this)->drawOrder[1] = ((GuideLineQuadStorage*)this)->vertices[1];
    ((GuideLineQuadStorage*)this)->drawOrder[2] = ((GuideLineQuadStorage*)this)->vertices[3];
    ((GuideLineQuadStorage*)this)->drawOrder[3] = ((GuideLineQuadStorage*)this)->vertices[2];
    ((GuideLineQuadStorage*)this)->drawOrder[4] = 0;
    char* entryCursor = (char*)this;
    for (int i = 0; i < kGuideEffectCapacity; ++i) {
        ((GuideEffectCursor*)entryCursor)->entry.sequence = 0;
        entryCursor += sizeof(GuideEffectEntry);
    }
    effectCount = 0;
    void* owner = this;
    if (this)
        owner = (char*)this + kMessageInterfaceOffset;
    ResourcePreloadQueue_AddRequest(g_sceneManager, 1, 0, kFloat_1_PvsEffects, owner, kFloat_0_805DECF4,
                                    guideLineTrackStrings, 0, 0, 0);
    (*(int*)&bitmap) = (int)ResourceManager_LoadBitmap(g_resourceManager, guideLineTrackStrings + 0xD, 0, 0, 0);
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
struct F030Search {
    struct Object {
        char pad[0x50];
        void* key;
    };
    struct Payload {
        Object* object;
        void* tree;
    };
    struct Node {
        Node* prev;
        Node* next;
        Payload* data;
    };
    int count;
    Node head;
    // Search the circular cache by the borrowed inventory key, returning its sentinel on miss.
    Node* find(void* const volatile* key) {
        Node* i;
        Node* last;
        last = &head;
        i = head.next;
        while (i != last && i->data->object->key != *key)
            i = i->next;
        return i;
    }
};
// Find a cached ball by ID or load its model, then select it and refresh attached effects.
// key is a borrowed inventory ID, not character data. Return a cache-owned pair
// [borrowed ball descriptor, loaded model handle], or null if inventory lookup fails.
// A hit returns the existing pair. Both hit/miss paths clear old preview effects;
// a null result does not replace the previously selected resource pointer.
void* Screen_GetOrLoadBallResource(void* list, void* key) {
    struct LinkGuard {
        void* sentinel;
        void* link;
        // Guard a newly allocated cache link until it is committed to the circular list.
        LinkGuard(void* s, void* l) : sentinel(s), link(l) {}
        // Free an uncommitted link allocation; committed links have been disarmed.
        ~LinkGuard() {
            if (link)
                appFreeUni__FPv(link);
        }
    };
    // Phase 1: search circular cache; list+4 is the sentinel, never a payload.
    // Volatile aliases preserve original observable accesses/compiler scheduling.
    char name[0x30];
    void* volatile unused;
    void* volatile link1;
    void* volatile link0;
    void* volatile found1;
    void* volatile found0;
    void* volatile k;
    void* volatile k0;
    k0 = key;
    k = key;
    void* found = ((F030Search*)list)->find(&k);
    found0 = found;
    found1 = found;
    void* result;
    if (found != (char*)list + 4)
        result = (*(void**)&((F030Search::Node*)found)->data);
    else
        result = 0;
    switch ((unsigned int)result) {
    default:
        break;
    case 0: {
        // Phase 2 on miss: resolve the inventory descriptor, allocate an 8-byte
        // resource pair, format its model path and load the handle into pair[1].
        void* resource =
            ((void* (*)(void*, void*))ItemInventory_FindBallData)(AsCSharedDoc(g_sharedDoc)->itemInventory, key);
        if (resource) {
            void* node = __nw__FUl(8);
            result = node;
            if (node) {
                ((void**)node)[0] = 0;
                ((void**)node)[1] = 0;
            }
            ((void**)node)[0] = resource;
            char* output = name;
            char* path = ((IFF_STRUCT::sBall*)resource)->model;
            char* format = (char*)ballConditionStrings + kBallModelFormatOffset;
            sprintf(output, format, path);
            register int zero = 0;
            ((void**)result)[1] = ResourceManager_LoadByExtension(g_resourceManager, name, zero, 1, zero);
            char* sentinel = (char*)list + 4;
            // Phase 3: allocate a 12-byte [previous,next,pair] link. LinkGuard owns
            // only this link until disarmed; the cache then owns link and pair.
            volatile unsigned char flag = zero;
            void* link = appMallocDummy__FUl(0xC);
            {
                link0 = link;
                link1 = link;
                LinkGuard guard(sentinel, link);
                void** linkdata = (void**)((char*)link + 8);
                if (linkdata) {
                    try {
                        *linkdata = result;
                    } catch (...) {
                        throw;
                    }
                }
                guard.link = 0;
                // Splice before the sentinel and increment count only after payload
                // construction. The two sentinel links still close the circular list.
                void** sentinelNode = (void**)sentinel;
                ((void**)sentinelNode[0])[1] = link;
                ((void**)link)[0] = sentinelNode[0];
                sentinelNode[0] = link;
                ((void**)link)[1] = sentinelNode;
                ++*(int*)list;
            }
        }
    }
    }
    // Phase 4: refresh effects even on a hit; select/attach only with a non-null pair.
    ((CFxBox*)((char*)list + kPreviewEffectsOffset))->ClearEffects(0);
    if (result) {
        (*(void**)&((BallPreviewStorage*)list)->selectedResource) = result;
        ScreenBallPreview_AttachBallEffects(list);
    }
    return result;
}
#pragma pop

struct F1CCNode {
    F1CCNode* prev;
    F1CCNode* next;
    void* value;
};
struct F1CCList {
    int count;
    F1CCNode head;
    // Return the embedded circular-list sentinel, not a resource payload.
    F1CCNode* end() {
        return &head;
    }
    // Search nodes by ball descriptor ID, stopping at the supplied end sentinel.
    static F1CCNode* find(F1CCNode* i, F1CCNode* last, void* const& key) {
        while (i != last && *(void**)((char*)*(void**)i->value + 0x50) != key)
            i = i->next;
        return i;
    }
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
// Select the current player's ball, loading its model into the preview cache if necessary.
void ScreenBallPreview_SelectCurrentPlayerBall(void* list) {
    struct LinkGuard {
        void* sentinel;
        void* link;
        // Construct an empty cache-link allocation guard.
        LinkGuard() : sentinel(0), link(0) {}
        // Guard a newly allocated cache link until it is committed to the circular list.
        LinkGuard(void* s, void* l) : sentinel(s), link(l) {}
        // Free an uncommitted link allocation; committed links have been disarmed.
        ~LinkGuard() {
            if (link)
                appFreeUni__FPv(link);
        }
    };
    char name[0x34];
    F1CCNode* last = ((F1CCList*)list)->end();
    F1CCNode* first = ((F1CCList*)list)->head.next;
    unsigned char index = AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer;
    void* key = *(void**)((char*)g_sharedDoc + index * 0x1D4 + 0x88);
    void* volatile s20 = key;
    void* volatile s1C = key;
    void* found = F1CCList::find(first, last, (void* const&)s1C);
    void* volatile s18 = found;
    void* volatile s14 = found;
    void* result;
    if (found != ((F1CCList*)list)->end())
        result = (*(void**)&((F030Search::Node*)found)->data);
    else
        result = 0;
    if (result) {
    } else {
        IFF_STRUCT::sBall* resource =
            (IFF_STRUCT::sBall*)ItemInventory_FindBallData(AsCSharedDoc(g_sharedDoc)->itemInventory, key);
        if (resource) {
            result = __nw__FUl(8);
            if (result) {
                ((void**)result)[0] = 0;
                ((void**)result)[1] = 0;
            }
            ((void**)result)[0] = resource;
            sprintf(name, (char*)ballConditionStrings + kBallModelFormatOffset, resource->model);
            register int zero = 0;
            ((void**)result)[1] = ResourceManager_LoadByExtension(g_resourceManager, name, zero, 1, zero);
            char* sentinel = (char*)list + 4;
            volatile unsigned char flag = zero;
            void* link = appMallocDummy__FUl(0xC);
            {
                void* volatile s0C = link;
                void* volatile s10 = link;
                LinkGuard guard(sentinel, link);
                void** linkdata = (void**)((char*)link + 8);
                if (linkdata) {
                    try {
                        *linkdata = result;
                    } catch (...) {
                        throw;
                    }
                }
                guard.link = 0;
                void** sentinelNode = (void**)sentinel;
                ((void**)sentinelNode[0])[1] = link;
                ((void**)link)[0] = sentinelNode[0];
                sentinelNode[0] = link;
                ((void**)link)[1] = sentinelNode;
                ++*(int*)list;
            }
        }
    }
    ((CFxBox*)((char*)list + kPreviewEffectsOffset))->ClearEffects(0);
    if (result) {
        (*(void**)&((BallPreviewStorage*)list)->selectedResource) = result;
        ScreenBallPreview_AttachBallEffects(list);
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
// Verified preset pairs (gx_renderer_state_and_efb_copy.cpp/gx_render_state_and_texture_upload.cpp state IDs):
// beforeA: depth write=1, depth test=1, compare=7 (always).
// beforeB: depth write=1, depth test=1, compare=3 (less or equal).
// after:   depth write=0, depth test=1, compare=7 (always).
// Both pre-draw lists are applied in order; the second supersedes compare only.
struct PreviewRenderStateWords {
    PreviewStatePair pairs[3];
};
struct PreviewModelRenderVtable {
    char precedingSlots[0x28];
    void* (*render)(void*, void*, int, int, int, int, float, int, int);
};
struct PreviewModelRenderDispatch {
    PreviewModelRenderVtable* vtable;
};
// Draw the preview model with its dedicated light and restore the previous light-enable states.
void CGIBallRenderer::RenderPreview(void* camera, void* matrix) {
    char* base = ballPreviewRenderStatePresets;
    Model_SetMatrixRecursive(
        ((BallPreviewResource*)ballResource)->model, 0,
        matrix); /* 0x8017764C-0x80177668: r5 = the incoming matrix (the original uses r6 as the temporary) */
    WPuppet_UpdateBoneWorldMatrices(((BallPreviewResource*)ballResource)->model, 0, 0, kFloat_1_0_PvsEffects);
    int params[8];
    for (int i = 0; i < kPreviewLightCount; ++i) {
        Engine_GetLightEnabled(i, &params[i]);
        Engine_SetLightEnabled(i, 0);
    }
    // Phase 1: disable other lights and install the preview presets/light.
    PreviewRenderStateWords beforeModelPresetA;
    beforeModelPresetA = ((PreviewRenderStateWords*)(base + kBeforeModelPresetA))[0];
    Engine_SetRenderStateList(&beforeModelPresetA, 3);
    PreviewRenderStateWords beforeModelPresetB = *(PreviewRenderStateWords*)(base + kBeforeModelPresetB);
    Engine_SetRenderStateList(&beforeModelPresetB, 3);
    Engine_SetLightEnabled(kPreviewLightIndex, 1);
    Engine_SetLight(kPreviewLightIndex, ((PreviewLightStorage*)this)->light);
    Camera_Apply(camera, kFloat_0_PvsFxBox, kFloat_0_PvsFxBox);
    char* view = (char*)Camera_GetRenderState(camera);
    SetTransform(1, view + 0x60);
    gtl_GenericRenderer_SetProjectionMatrix(view + 0x60);
    SetTransform(0, view + 0x20);
    gtl_GenericRenderer_SetViewMatrix(view + 0x20);
    // Phase 2: render with the current skin matrices, then restore presets/lights.
    void* r = WPuppet_GetSkinMatrices(((BallPreviewResource*)ballResource)->model);
    void* resource2 = ((BallPreviewResource*)ballResource)->model;
    ((PreviewModelRenderDispatch*)resource2)
        ->vtable->render(resource2, camera, 1, 1, 1, 1, kFloat_1_0_PvsEffects, (int)r, 0);
    PreviewRenderStateWords afterModelPreset = *(PreviewRenderStateWords*)(base + kAfterModelPreset);
    Engine_SetRenderStateList(&afterModelPreset, 3);
    for (int i = 0; i < kPreviewLightCount; ++i)
        Engine_SetLightEnabled(i, params[i]);
}
#pragma pop

// Release the guideline bitmap and every registered guideline effect sequence.
void CGuideLine::OnDestroy() {
    if (g_resourceManager) {
        if (bitmap) {
            ResourceManager_ReleaseId(g_resourceManager, bitmap);
            bitmap = 0;
        }
    }
    char* item = (char*)this;
    for (int i = 0; i < effectCount; ++i) {
        ((CFx*)g_fx)->RemoveSequence((unsigned int)(((GuideEffectCursor*)item)->entry.sequence), 0);
        ((GuideEffectCursor*)item)->entry.sequence = 0;
        item += sizeof(GuideEffectEntry);
    }
    effectCount = 0;
}

struct PreviewOrientationView {
    char reserved00[0x28];
    WVector direction;
    char reserved34[0x170];
    float yaw;
    char reserved1a8[0x5c];
    float spin;
};
// Build a direction-aligned preview basis, then compose spin and yaw rotations.
// out is a writable 0x30-byte affine matrix: basis vectors at +0,+0xC,+0x18,
// translation at +0x24. The middle axis is copied unchanged from src->direction.
// For an orthonormal result the caller must supply a UNIT direction not parallel
// to reference +Z; only the two cross-product axes are normalized here.
// Cross order: X = direction x referenceZ; Z = X x direction, so X x Y = Z
// under the usual right-handed cross convention. No fallback for degenerate input.
void CScreen::BuildOrientedBasisMatrix(char* out, void* source) {
    PreviewOrientationView* src = (PreviewOrientationView*)source;
    unsigned char orientedResult[0x30];
    unsigned char spinBasisComposition[0x30];
    unsigned char spinRotation[0x30];
    unsigned char yawRotation[0x30];
    WVector basisX;
    float referenceZ[3];
    WVector basisZ;
    *(WVector*)(out + 0x0C) = src->direction;
    referenceZ[0] = kFloat_0_PvsFxBox;
    referenceZ[1] = kFloat_0_PvsFxBox;
    referenceZ[2] = kFloat_1_0_PvsEffects;
    Vector_CrossProduct(&basisX, out + 0x0C, referenceZ);
    *(WVector*)out = basisX;
    Vector_CrossProduct(&basisZ, out, out + 0x0C);
    *(WVector*)(out + 0x18) = basisZ;
    (*(WVector*)out).Normalize();
    (*(WVector*)(out + 0x18)).Normalize();
    Vec3_Zero(out + 0x24);
    // Both rotations use axis selector 1 (Y), in the helper's native angle units.
    // Call order is concat(spinRotation,basis), then concat(result,yawRotation);
    // the yaw input is negated and the spin input doubled. Preserve that order.
    Matrix_SetAxisRotation(yawRotation, kFloat_neg1_PvsEffects * src->yaw, 1);
    Matrix_SetAxisRotation(spinRotation, kFloat_2_PreviewEffects * src->spin, 1);
    Matrix_ConcatAffine(spinBasisComposition, spinRotation, out);
    Matrix_ConcatAffine(orientedResult, spinBasisComposition, yawRotation);
    Matrix_CopyAffine(out, orientedResult);
}

#include "classes/CSharedDoc.h"

struct BallRingView {
    char pad[0x18];
    int* value;
    char pad2[0x0C];
    char worldPosition[1];
};
// Score-record bytes also tested by long-putt Pang eligibility; no stronger meaning is proven.
struct PreviewPlayerHoleStatus {
    char reserved00[0x3A];
    unsigned char perHoleStatus[18];
};
typedef char PreviewHoleStatusOffset[((int)&((PreviewPlayerHoleStatus*)0)->perHoleStatus == 0x3A) ? 1 : -1];
// Refresh live-ball terrain/lie globals and camera pitch; this does not sample the preview point.
// Non-green terrain clears the selected score record's per-hole byte at +0x3A.
// Long-putt rewards test that byte; its full set/clear policy is still unresolved.
static inline void BallRing_RefreshLiveTerrainAndCamera() {
    struct W3 {
        unsigned int a, b, c;
    };
    W3 livePositionBits = *(W3*)GolfBall__Fv();
    void* terrainQuery = CQuadTree_GetInstance();
    unsigned char liveGroundType = GetGroundType__9CQuadTreeF7WVector(terrainQuery, (unsigned int*)&livePositionBits);
    ((CGolfBall*)GolfBall__Fv())->m_groundType = liveGroundType;
    void* lieTerrain = (void*)CQuadTree_GetInstance();
    ((CQuadTree*)lieTerrain)->SetCondition();
    unsigned char refreshedGroundType = ((CGolfBall*)GolfBall__Fv())->m_groundType;
    if (refreshedGroundType == 2) {
        void* liveGolfDocument = (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc);
        AsCGolfDoc(liveGolfDocument)->cameraPitchRadians = kFloat_neg0_2268928_CourseView;
    } else {
        void* liveGolfDocument = (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc);
        AsCGolfDoc(liveGolfDocument)->cameraPitchRadians = kFloat_0_PvsFxBox;
        void* playerScore =
            GolfPlayer_GetEntryByIndex((*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc),
                                       AsCGolfDoc((*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc))->currentPlayer);
        liveGolfDocument = (*(void**)&((CSharedDoc*)g_sharedDoc)->golfDoc);
        ((PreviewPlayerHoleStatus*)playerScore)->perHoleStatus[AsCGolfDoc(liveGolfDocument)->currentHoleIndex] = 0;
    }
}

// Copy the borrowed preview XYZ, refresh live-ball globals, then choose lie penalty/ring segments.
// pos affects only self->worldPosition. All terrain and penalty queries intentionally
// use GolfBall(), even when pos differs; this routine is not a pure preview query.
void BallRing_UpdateGroundLie(BallRingView* self, const void* pos) {
    __as__7WVectorFRC7WVector(self->worldPosition, (void*)pos);
    BallRing_RefreshLiveTerrainAndCamera();
    int computed;
    if (((CGolfBall*)GolfBall__Fv())->m_groundType == 2)
        computed = 0;
    else {
        void* state = GolfBall__Fv();
        float rate = ((CQuadTree*)CQuadTree_GetInstance())->GetMaxPowerOnGroundAtPoint(state);
        computed = 100 - (int)(kFloat_100_CourseView * rate + kFloat_0_1_CourseView);
    }
    *(int*)&AsCGIBallRingRenderer(self->value)->liePenaltyPercent = computed;
    if (computed == 0) {
        AsCGIBallRingRenderer(self->value)->segmentCount = kNormalRingSegments;
    } else {
        AsCGIBallRingRenderer(self->value)->segmentCount = kPenaltyRingSegments;
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046213C", _ctors_u80176A10_0, BallPreview_InitializeUnitConstants);
#pragma pop
