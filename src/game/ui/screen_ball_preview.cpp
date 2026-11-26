// Ball preview screen: a 3D view of each active player's ball model and ring, with lighting,
// viewports and the effect box. Rebuilds the ball resources when the players' ball types change.

#include "MSL/list.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::list<void*, app::app_allocator<void*> > MSLPointerList;
#include "classes/COption.h"
#include "classes/CGIBallRenderer.h"
#include "classes/CGIBallRingRenderer.h"
struct PreviewBallSimulationView {
    char beforeRotation[0x2C];
    float rotation[2];
    char beforeEvent[0x34];
    int firstEventPreDataIndex;
};
// Explicit local views for the renderer's viewport and fixed-function light ABI.
struct PreviewViewport {
    int x, y, width, height;
    float minDepth, maxDepth;
};
struct PreviewLight {
    int type;
    float diffuse[4], specular[4], ambient[4];
    float position[3], direction[3];
    float range, falloff, constantAttenuation, linearAttenuation, quadraticAttenuation;
    float innerCone, outerCone;
};
struct PreviewBallRendererView {
    void* vtable;
    PreviewViewport viewport;
    void* resource;
    PreviewLight light;
};
// Ring colors are four RGBA bytes after XYZ in each 24-byte vertex.
struct PreviewRingVertex {
    float position[3];
    unsigned char rgba[4];
    float uv[2];
};
struct PreviewRingConstructionView {
    void* vtable;
    PreviewViewport viewport;
    int liePenaltyBits;
    int segmentCount;
    void* ringTexture;
    void* percentageTexture;
    PreviewRingVertex vertices[4];
};
typedef char PreviewRingVertexOffset[((int)&((PreviewRingConstructionView*)0)->vertices == 0x2C) ? 1 : -1];
struct PreviewRingRendererView {
    void* vtable;
    PreviewViewport viewport;
};
struct PreviewListView {
    int count;
    void* previous;
    void* next;
};
// Circular list: count at +0, embedded sentinel at self+4, first node in self+8.
// A node owns its PreviewResourcePair; the pair borrows its IFF ball record and retains
// a model resource-manager handle. Release model handles and pairs before freeing nodes.
struct PreviewPlayerEntryView {
    char reserved[8];
    int playerType;
};
struct PreviewBallSelectionView {
    char reserved[0x88];
    void* ballTypeId;
};
enum PreviewConstants {
    kPreviewDisabledPlayer = 3,
    kPreviewRingSegments = 12,
    kPreviewFirstActiveTutorialStage = 3
};
enum PreviewPixelDefaults {
    kBallLeft = 5,
    kBallTop = 360,
    kBallWidth = 128,
    kBallHeight = 101,
    kStrokePointLeft = 83,
    kStrokePointTop = 123,
    kStrokePointWidth = 204,
    kStrokePointHeight = 154,
    kRingLeft = 17,
    kRingTop = 353,
    kRingWidth = 152,
    kRingHeight = 114
};

#include "classes/PreviewResourcePair.h"
#include "classes/PreviewResourceNode.h"
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCGolfBall(p) ((CGolfBall*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/Object801752FC.h"
// Byte-address expressions preserve the swing-update temporaries used by local EH names.
enum PreviewRotationOffsets {
    kPreviewYawOffset = (int)&((Object801752FC*)0)->rotationYaw,
    kPreviewPitchOffset = (int)&((Object801752FC*)0)->rotationPitch,
    kPreviewPhaseOffset = (int)&((Object801752FC*)0)->rotationPhase
};

#include "sdk_protos.h"
#include "game/wvector.h"
#include "classes/CPvsFxBox.h"
#include "classes/CFxBox.h"

#define AsCPvsFxBox(p) ((CPvsFxBox*)(p))

struct PreviewOwnedVirtual {
    virtual void dtor(int);
};

struct PreviewEffectVirtual {
    virtual void s0();
    virtual void updateMilliseconds(float);
    virtual void s2();
    virtual void s3();
    virtual void s4();
    virtual void s5();
    virtual void s6();
    virtual void s7();
    virtual void s8();
    virtual void readPreviewRotation(void*);
};

extern "C" {

extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void FxBoxResourceTree_FreeSubtreeNodes(void*, void*);
extern void ResourceManager_ReleaseId(void*, void*);
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern void* ResourceManager_ReleaseObject(void*, void*, int);
extern void* ItemInventory_FindBallData(void*, void*);
extern void* __nw__FUl(unsigned int);
extern void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
extern void* appMallocDummy__FUl(unsigned int);
extern unsigned int g_resourceManager;
extern unsigned int g_sharedDoc;
extern const unsigned int ballConditionStrings[64];

// Packed external string pool; member extents come from the global catalog.
struct BallPreviewStringPool {
    char conditionLineTexture[20];
    char conditionFontTexture[20];
    char ballModelFormat[7];
    char spinActor[5];
    char center[7];
    char sprayExtension[4];
};
#define BallPreviewNames ((BallPreviewStringPool*)ballConditionStrings)

extern unsigned int g_taskManager;
extern const float kFloat_neg1_PvsEffects;
extern const float kFloat_12_566371_PvsFxBox;
extern const float kFloat_6_2831855_ActorRotation;
extern const float kFloat_0_PvsFxBox;
extern const float kFloat_0_62831855_PvsFxBox;
extern const float kFloat_1000_PvsFxBox;
extern const float kFloat_4_PvsFxBox, kFloat_67_PvsFxBox, kFloat_neg1_8_ScreenBallPreview, kFloat_640_PvsFxBox,
    kFloat_neg0_42_PvsFxBox, kFloat_1_0_PvsEffects;
extern const float kFloat_neg0_33_PvsFxBox, kFloat_neg0_09_PvsFxBox, kFloat_255_PvsFxBox, kFloat_128_PvsFxBox,
    kFloat_neg0_529998_PvsFxBox, kFloat_0_569545_PvsFxBox, kFloat_neg0_628268_PvsFxBox, kFloat_33_333332_PvsFxBox;
extern unsigned int g_option, g_view;
extern char __vt__9CPvsFxBox[], __vt__15CGIBallRenderer[], __vt__19CGIBallRingRenderer[];
extern unsigned int s_cameraTransform[];
extern void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern void Vec2_Zero(void*);
extern void Matrix_SetIdentityAffine(void*);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void Vec3_Scale(void*, void*, float);
extern void Vec3_Zero(void*);
extern void PSMTX44Identity(void*);
extern void ScreenBallPreview_RebuildPlayerResources(void*);
extern void __dt__6CFxBoxFv(void*, int);

extern void* GolfBall__Fv();
extern void* Object_GetChild(void*);
extern void* TaskActorTable_FindByName(void*, void*, void*, void*, void*);
extern void Vec2_ScaleAssign(void*, float);
extern void* InstanceTable_Get();
extern void* Mtx44_SetRotationX(void*, float);
extern void* Mtx44_SetRotationY(void*, float);
extern void Mtx44_Concat3(void*, void*, void*, void*);
extern void Vec3_Normalize(void*, void*);
extern void Vec3_Cross(void*, void*, void*);
extern void PSMTX44Concat(void*, void*, void*);
extern void Engine_SetRenderStateList(void*, int);
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
extern void Vector_CrossProduct(void*, void*, void*);
extern void Normalize__7WVectorFv(void*);
extern void Matrix_SetAxisRotation(void*, float, int);
extern void Matrix_ConcatAffine(void*, void*, void*);
extern void Matrix_CopyAffine(void*, void*);
extern const unsigned int ballPreviewRenderStatePresets[];
extern float kFloat_0_75_ScreenBallPreview, kFloat_1_5707964_PreviewProjection, kFloat_1_3333334_PreviewEffects,
    kFloat_0_1_CourseView, kFloat_10_BallPreview, kFloat_2_PreviewEffects;

extern void __dt__9CPvsFxBoxFv(void*, int);
void* dtor_801756B8(void*, int);

// Initialize preview lighting/viewports, ring textures, renderers and the embedded effect box.
void* ScreenBallPreview_Ctor(void* self) {
    // Phase 1: initialize the list and embedded effect containers for exception cleanup.
    void* listSentinel;
    char* previewBytes = (char*)self;
    int zero = 0;
    listSentinel = previewBytes + 4;
    *(int*)(previewBytes) = zero;
    *((void**)listSentinel + 1) = listSentinel;
    *(void**)listSentinel = listSentinel;
    AsObject801752FC(previewBytes)->currentBallResource = zero;
    (*(int*)&AsObject801752FC(previewBytes)->ringBitmapA) = zero;
    (*(int*)&AsObject801752FC(previewBytes)->ringBitmapB) = zero;
    char* effectBoxBytes = previewBytes + 0x208;
    ((CFxBox*)effectBoxBytes)->__ct();
    *(void**)effectBoxBytes = __vt__9CPvsFxBox;
    *(int*)(effectBoxBytes + 0x37c) = zero;
    *(int*)(effectBoxBytes + 0x380) = zero;
    *(void**)(effectBoxBytes + 0x384) = effectBoxBytes + 0x380;
    *(int*)(effectBoxBytes + 0x388) = zero;
    *(int*)(effectBoxBytes + 0x38c) = zero;
    *(void**)(effectBoxBytes + 0x390) = effectBoxBytes + 0x38c;
    try {
        float baseHeight = kFloat_4_PvsFxBox;
        float width = kFloat_67_PvsFxBox;
        float extraHeight = AsCOption(*(void**)&g_option)->hudBaselineY;
        float height = baseHeight + extraHeight;
        AsObject801752FC(previewBytes)->screenCenterX = width;
        AsObject801752FC(previewBytes)->screenCenterY = height;
        ScreenBallPreview_RebuildPlayerResources(self);
        *(void**)(previewBytes + 0x1a8) =
            ResourceManager_LoadBitmap(*(void**)&g_resourceManager, (char*)&ballConditionStrings, 0, 0, 0);
        *(void**)(previewBytes + 0x1ac) =
            ResourceManager_LoadBitmap(*(void**)&g_resourceManager, BallPreviewNames->conditionFontTexture, 0, 0, 0);
        Vec2_Zero(previewBytes + 0x1b0);
        Vec2_Zero(previewBytes + 0x1f8);
        AsObject801752FC(previewBytes)->rotationPhase = kFloat_0_PvsFxBox;
        AsObject801752FC(previewBytes)->ringYaw = kFloat_0_PvsFxBox;
        Matrix_SetIdentityAffine(previewBytes + 0x34);
        float scaledOffset[3];
        float viewOffset[3] = {kFloat_0_PvsFxBox, kFloat_0_PvsFxBox, kFloat_neg1_8_ScreenBallPreview};
        __ml__FfRC7WVector(scaledOffset, viewOffset, AsWView(*(void**)&g_view)->viewportWidth / kFloat_640_PvsFxBox);
        __as__7WVectorFRC7WVector(previewBytes + 0x58, scaledOffset);
        // Phase 2: define three independent preview view matrices and pixel viewports.
        Matrix_SetIdentityAffine(previewBytes + 0xcc);
        // original WView +0x188
        AsObject801752FC(previewBytes)->ballViewTranslationZ = kFloat_neg0_42_PvsFxBox;
        AsObject801752FC(previewBytes)->ballViewportX = kBallLeft;
        AsObject801752FC(previewBytes)->ballViewportY = kBallTop;
        AsObject801752FC(previewBytes)->ballViewportWidth = kBallWidth;
        AsObject801752FC(previewBytes)->ballViewportHeight = kBallHeight;
        AsObject801752FC(previewBytes)->ballViewportMinDepth = kFloat_0_PvsFxBox;
        AsObject801752FC(previewBytes)->ballViewportMaxDepth = kFloat_1_0_PvsEffects;
        Matrix_SetIdentityAffine(previewBytes + 0x114);
        AsObject801752FC(previewBytes)->strokePointViewTranslationZ = kFloat_neg0_42_PvsFxBox;
        AsObject801752FC(previewBytes)->strokePointViewportX = kStrokePointLeft;
        AsObject801752FC(previewBytes)->strokePointViewportY = kStrokePointTop;
        AsObject801752FC(previewBytes)->strokePointViewportWidth = kStrokePointWidth;
        AsObject801752FC(previewBytes)->strokePointViewportHeight = kStrokePointHeight;
        AsObject801752FC(previewBytes)->strokePointViewportMinDepth = kFloat_0_PvsFxBox;
        AsObject801752FC(previewBytes)->strokePointViewportMaxDepth = kFloat_1_0_PvsEffects;
        Matrix_SetIdentityAffine(previewBytes + 0x15c);
        AsObject801752FC(previewBytes)->ringViewTranslationZ = kFloat_neg0_33_PvsFxBox;
        AsObject801752FC(previewBytes)->ringViewTranslationX = kFloat_neg0_09_PvsFxBox;
        AsObject801752FC(previewBytes)->ringViewportX = kRingLeft;
        AsObject801752FC(previewBytes)->ringViewportY = kRingTop;
        AsObject801752FC(previewBytes)->ringViewportWidth = kRingWidth;
        AsObject801752FC(previewBytes)->ringViewportHeight = kRingHeight;
        AsObject801752FC(previewBytes)->ringViewportMinDepth = kFloat_0_PvsFxBox;
        AsObject801752FC(previewBytes)->ringViewportMaxDepth = kFloat_1_0_PvsEffects;
        Memory_Clear(previewBytes + 0x64, 0x68);
        // Phase 3: initialize fixed-function lighting; ballLightType stores integer bits.
        (*(int*)&AsObject801752FC(previewBytes)->ballLightType) = 3;
        AsObject801752FC(previewBytes)->ballLightDiffuseRed = kFloat_255_PvsFxBox;
        AsObject801752FC(previewBytes)->ballLightDiffuseGreen = kFloat_255_PvsFxBox;
        AsObject801752FC(previewBytes)->ballLightDiffuseBlue = kFloat_255_PvsFxBox;
        AsObject801752FC(previewBytes)->ballLightAmbientRed = kFloat_128_PvsFxBox;
        AsObject801752FC(previewBytes)->ballLightAmbientGreen = kFloat_128_PvsFxBox;
        AsObject801752FC(previewBytes)->ballLightAmbientBlue = kFloat_128_PvsFxBox;
        AsObject801752FC(previewBytes)->ballLightDirectionX = kFloat_neg0_529998_PvsFxBox;
        AsObject801752FC(previewBytes)->ballLightDirectionY = kFloat_0_569545_PvsFxBox;
        AsObject801752FC(previewBytes)->ballLightDirectionZ = kFloat_neg0_628268_PvsFxBox;
        Vec3_Scale(previewBytes + 0xa4, previewBytes + 0xa4, kFloat_neg1_PvsEffects);
        Vec3_Normalize(previewBytes + 0xa4, previewBytes + 0xa4);
        Vec3_Zero(previewBytes + 0x28);
        PSMTX44Identity(previewBytes + 0x1b8);
        AsObject801752FC(previewBytes)->unk_59C = previewBytes + 0x5b0;
        AsObject801752FC(previewBytes)->unk_5A0 = previewBytes + 0x5e8;
        AsObject801752FC(previewBytes)->unk_5A4 = previewBytes + 0x658;
        AsObject801752FC(previewBytes)->unk_5A8 = previewBytes + 0x620;
        AsObject801752FC(previewBytes)->unk_5AC = zero;
        ((CFxBox*)(previewBytes + 0x208))->SetResourceManager(*(void**)&g_resourceManager);
        ((CFxBox*)(previewBytes + 0x208))->SetCameraTransform(&s_cameraTransform);
        AsObject801752FC(previewBytes)->unk_20C = kFloat_33_333332_PvsFxBox;
        void* renderer = __nw__FUl(0x88);
        if (renderer) {
            *(void**)renderer = __vt__15CGIBallRenderer;
            *(int*)&((CGIBallRenderer*)renderer)->ballResource = zero;
        }

        AsObject801752FC(previewBytes)->ballRenderer = renderer;
        renderer = __nw__FUl(0x88);
        if (renderer) {
            *(void**)renderer = __vt__15CGIBallRenderer;
            *(int*)&((CGIBallRenderer*)renderer)->ballResource = 0;
        }
        (*(void**)&AsObject801752FC(previewBytes)->secondaryBallRenderer) = renderer;
        renderer = __nw__FUl(0x8c);
        if (renderer) {
            unsigned char* ringBytes = (unsigned char*)renderer;
            *(void**)ringBytes = __vt__19CGIBallRingRenderer;
            ((PreviewRingConstructionView*)ringBytes)->liePenaltyBits = 0;
            ((PreviewRingConstructionView*)ringBytes)->segmentCount = kPreviewRingSegments;
            ((PreviewRingConstructionView*)ringBytes)->vertices[0].rgba[3] = 255;
            ((PreviewRingConstructionView*)ringBytes)->vertices[0].rgba[0] = 0;
            ((PreviewRingConstructionView*)ringBytes)->vertices[0].rgba[1] = 255;
            ((PreviewRingConstructionView*)ringBytes)->vertices[0].rgba[2] = 150;
            ((PreviewRingConstructionView*)ringBytes)->vertices[1].rgba[3] = 255;
            ((PreviewRingConstructionView*)ringBytes)->vertices[1].rgba[0] = 0;
            ((PreviewRingConstructionView*)ringBytes)->vertices[1].rgba[1] = 255;
            ((PreviewRingConstructionView*)ringBytes)->vertices[1].rgba[2] = 150;
            ((PreviewRingConstructionView*)ringBytes)->vertices[2].rgba[3] = 255;
            ((PreviewRingConstructionView*)ringBytes)->vertices[2].rgba[0] = 0;
            ((PreviewRingConstructionView*)ringBytes)->vertices[2].rgba[1] = 255;
            ((PreviewRingConstructionView*)ringBytes)->vertices[2].rgba[2] = 150;
            ((PreviewRingConstructionView*)ringBytes)->vertices[3].rgba[3] = 255;
            ((PreviewRingConstructionView*)ringBytes)->vertices[3].rgba[0] = 0;
            ((PreviewRingConstructionView*)ringBytes)->vertices[3].rgba[1] = 255;
            ((PreviewRingConstructionView*)ringBytes)->vertices[3].rgba[2] = 150;
        }

        (*(void**)&AsObject801752FC(previewBytes)->ballRingRenderer) = renderer;
        void* ringTexture = *(void**)(previewBytes + 0x1a8);
        void* percentageTexture = *(void**)(previewBytes + 0x1ac);
        *(void**)&((CGIBallRingRenderer*)renderer)->percentageTexture = percentageTexture;
        *(void**)&((CGIBallRingRenderer*)renderer)->ringTexture = ringTexture;
        AsObject801752FC(previewBytes)->unk_200 = kFloat_0_PvsFxBox;
        return self;
    } catch (...) {
        __dt__9CPvsFxBoxFv(previewBytes + 0x208, -1);
        dtor_801756B8(previewBytes, -1);
        throw;
    }
}

// Rotate the preview from input or simulated ball orientation, normalize its axes and advance effects.
// delta is seconds: effect updates receive delta*1000 milliseconds. Rotation angles are
// radians; +0x1B8 is a 4x4 preview orientation, separate from the three view transforms.
// The input actor writes two angular increments; once event slot 0 is set, use ball rotation instead.
void ScreenBallPreview_Update(void* self, float delta) {
    char* p = (char*)self;
    float yawMatrix[16];
    float pitchMatrix[16];
    float tmp[3][3];
    void* args[8];
    if (AsCGolfDoc(*(void**)&AsCSharedDoc(*(void**)&g_sharedDoc)->golfDoc)->tutorialStage >=
        kPreviewFirstActiveTutorialStage) {
        if (((PreviewBallSimulationView*)GolfBall__Fv())->firstEventPreDataIndex < 0) {
            void* manager = *(void**)&g_taskManager;
            manager = manager != 0 ? Object_GetChild(manager) : 0;
            void* zero = 0;
            void* one = (void*)1;
            args[0] = zero;
            args[1] = one;
            args[2] = p + kPreviewYawOffset;
            args[3] = p + kPreviewPitchOffset;
            args[4] = zero;
            args[5] = zero;
            args[6] = zero;
            args[7] = zero;
            void* obj =
                TaskActorTable_FindByName(manager, BallPreviewNames->spinActor, p + kPreviewYawOffset, one, zero);
            if (obj != 0) {
                ((PreviewEffectVirtual*)obj)->readPreviewRotation(args);
            }
            float neg = kFloat_neg1_PvsEffects * AsObject801752FC(p)->rotationPitch;
            float scaled = kFloat_12_566371_PvsFxBox * delta;
            *(float*)(p + kPreviewPitchOffset) = neg;
            Vec2_ScaleAssign(p + kPreviewYawOffset, scaled);
            float yaw = AsObject801752FC(p)->rotationYaw;
            if (yaw > kFloat_6_2831855_ActorRotation)
                *(float*)(p + kPreviewYawOffset) = yaw - kFloat_6_2831855_ActorRotation;
            else if (yaw < kFloat_0_PvsFxBox)
                *(float*)(p + kPreviewYawOffset) = yaw + kFloat_6_2831855_ActorRotation;
            float pitch = *(float*)(p + kPreviewPitchOffset);
            if (pitch > kFloat_6_2831855_ActorRotation)
                *(float*)(p + kPreviewPitchOffset) = pitch - kFloat_6_2831855_ActorRotation;
            else if (pitch < kFloat_0_PvsFxBox)
                *(float*)(p + kPreviewPitchOffset) = pitch + kFloat_6_2831855_ActorRotation;
            void* effect = InstanceTable_Get();
            if (*(unsigned char*)((char*)*(void**)((char*)effect + 4) + 0x0a) != 3) {
                Mtx44_SetRotationX(pitchMatrix, *(float*)(p + kPreviewPitchOffset));
                Mtx44_SetRotationY(yawMatrix, *(float*)(p + kPreviewYawOffset));
                Mtx44_Concat3(p + 0x1b8, p + 0x1b8, yawMatrix, pitchMatrix);
                float* d = (float*)tmp;
                float* d2 = d;
                char* src = p;
                for (int i = 0; i < 3; i++) {
                    d[0] = *(float*)(src + 0x1b8);
                    d[1] = *(float*)(src + 0x1bc);
                    d[2] = *(float*)(src + 0x1c0);
                    Vec3_Normalize(d, d2);
                    d += 3;
                    d2 += 3;
                    src += 0x10;
                }
                // Rebuild orthogonal axes after incremental rotations to limit drift.
                Vec3_Cross(tmp[1], tmp[2], tmp[0]);
                Vec3_Cross(tmp[2], tmp[0], tmp[1]);
                float* s = (float*)tmp;
                float* s2 = s;
                char* dst = p;
                for (int i = 0; i < 3; i++) {
                    *(float*)(dst + 0x1b8) = s[0];
                    *(float*)(dst + 0x1bc) = s[1];
                    *(float*)(dst + 0x1c0) = s[2];
                    Vec3_Normalize(s, s2);
                    dst += 0x10;
                    s += 3;
                    s2 += 3;
                }
            }
        } else {
            void* state = GolfBall__Fv();
            Mtx44_SetRotationX(pitchMatrix, ((PreviewBallSimulationView*)state)->rotation[1] * kFloat_neg1_PvsEffects);
            state = GolfBall__Fv();
            Mtx44_SetRotationY(yawMatrix, ((PreviewBallSimulationView*)state)->rotation[0]);
            PSMTX44Concat(p + 0x1b8, yawMatrix, pitchMatrix);
        }
        // Advance the decorative ring in radians and convert the effect clock to milliseconds.
        float twoPi = kFloat_6_2831855_ActorRotation;
        float phase = kFloat_0_62831855_PvsFxBox * delta + AsObject801752FC(p)->rotationPhase;
        *(float*)(p + kPreviewPhaseOffset) = phase;
        if (twoPi < phase)
            *(float*)(p + kPreviewPhaseOffset) = phase - twoPi;
        else if (phase < kFloat_0_PvsFxBox)
            *(float*)(p + kPreviewPhaseOffset) = phase + twoPi;
        ((PreviewEffectVirtual*)(p + 0x208))->updateMilliseconds(kFloat_1000_PvsFxBox * delta);
        ((CFxBox*)(p + 0x208))->UpdateEffects(kFloat_1000_PvsFxBox * delta);
    }
}

// Unlink and free the preview resource-list nodes; payload cleanup belongs to the owning destructor.
void* dtor_801756B8(void* self, int deleting) {
    return ::std::__list_deleting_dtor((MSLPointerList*)self, deleting);
}

#pragma push
#pragma use_lmw_stmw on
#pragma global_optimizer off
// Release old preview resources and load one ball model for each distinct active-player ball type.
void ScreenBallPreview_RebuildPlayerResources(void* self) {
    // Phase 1: release pair payloads and retained model handles, leaving nodes for unlinking.
    struct NodeGuard {
        void* owner;
        void* ptr;
        // Free an allocated node if construction fails before it joins the list.
        ~NodeGuard() {
            if (ptr)
                appFreeUni__FPv(ptr);
        }
    };
    char* base = (char*)self;
    {

        void* sentinel = (base + 4);
        void* volatile iter;
        void* start = AsObject801752FC(base)->listNext;
        void* volatile initial;
        initial = start;
        iter = start;
        while (iter != sentinel) {
            void* resource = AsPreviewResourceNode(iter)->value;
            if (resource) {
                void* mgr = *(void**)&g_resourceManager;
                if (mgr != 0) {
                    if (AsPreviewResourcePair(resource)->resource != 0) {
                        ResourceManager_ReleaseObject(mgr, AsPreviewResourcePair(resource)->resource, 0);
                        AsPreviewResourcePair(AsPreviewResourceNode(iter)->value)->resource = 0;
                    }
                }
                __dl__FPv(AsPreviewResourceNode(iter)->value);
            }
            iter = AsPreviewResourceNode(iter)->next;
        }
    }
    {
        void* sentinel = base + 4;
        void* node = AsObject801752FC(base)->listNext;
        if (node != sentinel) {
            void* last = AsPreviewResourceNode(sentinel)->previous;
            AsPreviewResourceNode(AsPreviewResourceNode(node)->previous)->next = AsPreviewResourceNode(last)->next;
            *(*(void***)&AsPreviewResourceNode(last)->next) = AsPreviewResourceNode(node)->previous;
            while (node != sentinel) {
                void* cur = node;
                node = AsPreviewResourceNode(node)->next;
                appFreeUni__FPv(cur);
                --((PreviewListView*)self)->count;
            }
        }
    }
    // Phase 2: the list is empty; clear effects before constructing the new distinct models.
    ((CFxBox*)(base + 0x208))->ClearEffects(0);
    unsigned char i = 0;
    void* manager;
    while ((manager = (*(void**)&AsCSharedDoc(*(void**)&g_sharedDoc)->golfDoc)), i < AsCGolfDoc(manager)->playerCount) {
        void* item = GolfPlayer_GetEntryByIndex(manager, i);
        if (((PreviewPlayerEntryView*)item)->playerType != kPreviewDisabledPlayer) {
            void* table = *(void**)&g_sharedDoc;
            void* keyValue;
            void* sentinel;
            void* node;
            void* entry = (char*)table + (unsigned int)i * 0x1d4;
            keyValue = ((PreviewBallSelectionView*)entry)->ballTypeId;
            void* volatile allocated;
            // The volatile *Mirror locals hold the same value as found/allocated/key; they keep the original stack slots.
            void* volatile allocatedMirror;
            void* volatile found;
            void* volatile foundMirror;
            void* volatile key;
            void* volatile keyMirror;
            keyMirror = keyValue;
            key = keyValue;
            node = AsObject801752FC(base)->listNext;
            sentinel = 4 + base;
            while (node != sentinel &&
                   (void*)AsPreviewResourcePair(AsPreviewResourceNode(node)->value)->ballRecord->common.typeId != key)
                node = AsPreviewResourceNode(node)->next;
            foundMirror = node;
            found = node;
            void* value = node != base + 4 ? AsPreviewResourceNode(node)->value : 0;
            // Phase 3: reuse a type already seen, or append one newly loaded pair for that type.
            if (!value) {
                IFF_STRUCT::sBall* ballRecord = (IFF_STRUCT::sBall*)ItemInventory_FindBallData(
                    AsCSharedDoc(table)->itemInventory,
                    keyValue); // original r4 = lwz 0x88(r3) at 0x80175FCC, kept to the bl 0x8017602C
                void* ballEntry;
                if (ballRecord) {
                    ballEntry = __nw__FUl(8);
                    if (ballEntry) {
                        AsPreviewResourcePair(ballEntry)->ballRecord = 0;
                        AsPreviewResourcePair(ballEntry)->resource = 0;
                    }
                    AsPreviewResourcePair(ballEntry)->ballRecord = ballRecord;
                    struct TextWorkspace {
                        unsigned char pad[0x18];
                        char text[0x20];
                    } workspace;
                    ((void (*)(char*, const char*, ...))sprintf)(workspace.text, BallPreviewNames->ballModelFormat,
                                                                 ballRecord->model);
                    unsigned int zero = 0;

                    AsPreviewResourcePair(ballEntry)->resource =
                        ResourceManager_LoadByExtension(*(void**)&g_resourceManager, workspace.text, zero, 1, zero);
                    volatile unsigned char allocator = zero;
                    // Append a new list node {next, previous, value = ballEntry} (0xC bytes) before the sentinel at base + 4;
                    // NodeGuard frees the node if linking is abandoned (it is cleared once the node holds the ballEntry).
                    void* end = 4 + base;
                    void* newnode = appMallocDummy__FUl(0xc);
                    void* link = (char*)newnode + 8;
                    allocatedMirror = newnode;
                    allocated = newnode;
                    NodeGuard guard;
                    guard.owner = end;
                    guard.ptr = newnode;
                    if (link) {
                        *(void**)link = ballEntry;
                    }
                    guard.ptr = 0;
                    AsPreviewResourceNode(AsPreviewResourceNode(end)->previous)->next = newnode;
                    AsPreviewResourceNode(newnode)->previous = AsPreviewResourceNode(end)->previous;
                    AsPreviewResourceNode(end)->previous = newnode;
                    AsPreviewResourceNode(newnode)->next = end;
                    ++((PreviewListView*)self)->count;
                }
            }
        }
        i++;
    }
}

#pragma pop
#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Node ownership is separate from effect payload ownership: this walk frees nodes only.
struct PreviewEffectTreeNode {
    PreviewEffectTreeNode* left;
    PreviewEffectTreeNode* right;
};
// The four expanded levels below retain separate variable lifetimes required by mwcc.

// Release ball models, ring textures, renderers and effect trees before freeing the preview owner.
void* ScreenBallPreview_dtor(void* self, int deleting) {
    // Phase 1: release all pair payloads, then unlink/free nodes; the sentinel remains valid.
    if (self != 0) {
        char* previewBytes = (char*)self;
        {
            void* node = AsObject801752FC(previewBytes)->listNext;
            int zero = 0;
            void* sentinel = previewBytes + 4;
            while (node != sentinel) {
                void* entry = AsPreviewResourceNode(node)->value;
                if (entry != 0) {
                    void* resourceManager = *(void**)&g_resourceManager;
                    if (resourceManager != 0) {
                        if (AsPreviewResourcePair(entry)->resource != 0) {
                            ResourceManager_ReleaseObject(resourceManager, AsPreviewResourcePair(entry)->resource, 0);
                            *(int*)&AsPreviewResourcePair(AsPreviewResourceNode(node)->value)->resource = zero;
                        }
                    }
                    __dl__FPv(AsPreviewResourceNode(node)->value);
                }
                node = AsPreviewResourceNode(node)->next;
            }
        }
        {
            void* node = AsObject801752FC(previewBytes)->listNext;
            void* sentinel = previewBytes + 4;
            if (node != sentinel) {
                void* last = AsPreviewResourceNode(sentinel)->previous;
                AsPreviewResourceNode(AsPreviewResourceNode(node)->previous)->next = AsPreviewResourceNode(last)->next;
                *(*(void***)&AsPreviewResourceNode(last)->next) = AsPreviewResourceNode(node)->previous;
                while (node != sentinel) {
                    void* cur = node;
                    node = AsPreviewResourceNode(node)->next;
                    appFreeUni__FPv(cur);
                    --((PreviewListView*)self)->count;
                }
            }
        }

        // Phase 2: release textures and polymorphic renderers before dismantling the effect box.
        void* textureResource = AsObject801752FC(previewBytes)->ringBitmapA;
        if (textureResource != 0) {
            ResourceManager_ReleaseId(*(void**)&g_resourceManager, textureResource);
            AsObject801752FC(previewBytes)->ringBitmapA = 0;
        }
        textureResource = AsObject801752FC(previewBytes)->ringBitmapB;
        if (textureResource != 0) {
            ResourceManager_ReleaseId(*(void**)&g_resourceManager, textureResource);
            AsObject801752FC(previewBytes)->ringBitmapB = 0;
        }
        ((CFxBox*)(previewBytes + 0x208))->ClearEffects(0);
        void* ownedRenderer = AsObject801752FC(previewBytes)->ballRenderer;
        if (ownedRenderer != 0) {
            if (ownedRenderer != 0) {
                ((PreviewOwnedVirtual*)ownedRenderer)->dtor(1);
            }
            AsObject801752FC(previewBytes)->ballRenderer = 0;
        }
        ownedRenderer = AsObject801752FC(previewBytes)->secondaryBallRenderer;
        if (ownedRenderer != 0) {
            if (ownedRenderer != 0) {
                ((PreviewOwnedVirtual*)ownedRenderer)->dtor(1);
            }
            AsObject801752FC(previewBytes)->secondaryBallRenderer = 0;
        }
        ownedRenderer = AsObject801752FC(previewBytes)->ballRingRenderer;
        if (ownedRenderer != 0) {
            if (ownedRenderer != 0) {
                ((PreviewOwnedVirtual*)ownedRenderer)->dtor(1);
            }
            AsObject801752FC(previewBytes)->ballRingRenderer = 0;
        }

        // Phase 3: free the sequence-attachment tree (root in sequenceAttachmentTreeRoot, header at +0x590).
        // Free nodes in postorder; repeated owner-wrapper tests preserve emitted branches.
        if (previewBytes + 0x208) {
            if (previewBytes + 0x590 && previewBytes + 0x590 && previewBytes + 0x590 && previewBytes + 0x590 &&
                previewBytes + 0x590 && previewBytes + 0x590) {
                {
                    void* rootNode = AsObject801752FC(previewBytes)->sequenceAttachmentTreeRoot;
                    if (rootNode) {
                        void* branchNode = ((PreviewEffectTreeNode*)rootNode)->left;
                        if (branchNode) {
                            void* childNode = ((PreviewEffectTreeNode*)branchNode)->left;
                            if (childNode) {
                                void* leafNode = ((PreviewEffectTreeNode*)childNode)->left;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x590,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x590,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                leafNode = ((PreviewEffectTreeNode*)childNode)->right;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x590,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x590,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                appFreeUni__FPv(childNode);
                            }
                            void* rightChildNode = ((PreviewEffectTreeNode*)branchNode)->right;
                            if (rightChildNode) {
                                void* rightLeafNode = ((PreviewEffectTreeNode*)rightChildNode)->left;
                                if (rightLeafNode) {
                                    if (((PreviewEffectTreeNode*)rightLeafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightLeafNode)->left);
                                    if (((PreviewEffectTreeNode*)rightLeafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightLeafNode)->right);
                                    appFreeUni__FPv(rightLeafNode);
                                }
                                rightLeafNode = ((PreviewEffectTreeNode*)rightChildNode)->right;
                                if (rightLeafNode) {
                                    if (((PreviewEffectTreeNode*)rightLeafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightLeafNode)->left);
                                    if (((PreviewEffectTreeNode*)rightLeafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightLeafNode)->right);
                                    appFreeUni__FPv(rightLeafNode);
                                }
                                appFreeUni__FPv(rightChildNode);
                            }
                            appFreeUni__FPv(branchNode);
                        }
                        void* rightBranchNode = ((PreviewEffectTreeNode*)rootNode)->right;
                        if (rightBranchNode) {
                            void* rightBranchChild = ((PreviewEffectTreeNode*)rightBranchNode)->left;
                            if (rightBranchChild) {
                                void* rightBranchLeaf = ((PreviewEffectTreeNode*)rightBranchChild)->left;
                                if (rightBranchLeaf) {
                                    if (((PreviewEffectTreeNode*)rightBranchLeaf)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightBranchLeaf)->left);
                                    if (((PreviewEffectTreeNode*)rightBranchLeaf)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightBranchLeaf)->right);
                                    appFreeUni__FPv(rightBranchLeaf);
                                }
                                rightBranchLeaf = ((PreviewEffectTreeNode*)rightBranchChild)->right;
                                if (rightBranchLeaf) {
                                    if (((PreviewEffectTreeNode*)rightBranchLeaf)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightBranchLeaf)->left);
                                    if (((PreviewEffectTreeNode*)rightBranchLeaf)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightBranchLeaf)->right);
                                    appFreeUni__FPv(rightBranchLeaf);
                                }
                                appFreeUni__FPv(rightBranchChild);
                            }
                            rightBranchChild = ((PreviewEffectTreeNode*)rightBranchNode)->right;
                            if (rightBranchChild) {
                                void* rightBranchLeaf = ((PreviewEffectTreeNode*)rightBranchChild)->left;
                                if (rightBranchLeaf) {
                                    if (((PreviewEffectTreeNode*)rightBranchLeaf)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightBranchLeaf)->left);
                                    if (((PreviewEffectTreeNode*)rightBranchLeaf)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightBranchLeaf)->right);
                                    appFreeUni__FPv(rightBranchLeaf);
                                }
                                rightBranchLeaf = ((PreviewEffectTreeNode*)rightBranchChild)->right;
                                if (rightBranchLeaf) {
                                    if (((PreviewEffectTreeNode*)rightBranchLeaf)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightBranchLeaf)->left);
                                    if (((PreviewEffectTreeNode*)rightBranchLeaf)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(
                                            previewBytes + 0x590, ((PreviewEffectTreeNode*)rightBranchLeaf)->right);
                                    appFreeUni__FPv(rightBranchLeaf);
                                }
                                appFreeUni__FPv(rightBranchChild);
                            }
                            appFreeUni__FPv(rightBranchNode);
                        }
                        appFreeUni__FPv(rootNode);
                    }
                }
            }
            if (previewBytes + 0x584 && previewBytes + 0x584 && previewBytes + 0x584 && previewBytes + 0x584 &&
                previewBytes + 0x584 && previewBytes + 0x584) {
                {
                    void* rootNode = AsObject801752FC(previewBytes)->sprayAttachmentTreeRoot;
                    if (rootNode) {
                        void* branchNode = ((PreviewEffectTreeNode*)rootNode)->left;
                        if (branchNode) {
                            void* childNode = ((PreviewEffectTreeNode*)branchNode)->left;
                            if (childNode) {
                                void* leafNode = ((PreviewEffectTreeNode*)childNode)->left;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                leafNode = ((PreviewEffectTreeNode*)childNode)->right;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                appFreeUni__FPv(childNode);
                            }
                            childNode = ((PreviewEffectTreeNode*)branchNode)->right;
                            if (childNode) {
                                void* leafNode = ((PreviewEffectTreeNode*)childNode)->left;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                leafNode = ((PreviewEffectTreeNode*)childNode)->right;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                appFreeUni__FPv(childNode);
                            }
                            appFreeUni__FPv(branchNode);
                        }
                        branchNode = ((PreviewEffectTreeNode*)rootNode)->right;
                        if (branchNode) {
                            void* childNode = ((PreviewEffectTreeNode*)branchNode)->left;
                            if (childNode) {
                                void* leafNode = ((PreviewEffectTreeNode*)childNode)->left;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                leafNode = ((PreviewEffectTreeNode*)childNode)->right;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                appFreeUni__FPv(childNode);
                            }
                            childNode = ((PreviewEffectTreeNode*)branchNode)->right;
                            if (childNode) {
                                void* leafNode = ((PreviewEffectTreeNode*)childNode)->left;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                leafNode = ((PreviewEffectTreeNode*)childNode)->right;
                                if (leafNode) {
                                    if (((PreviewEffectTreeNode*)leafNode)->left)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->left);
                                    if (((PreviewEffectTreeNode*)leafNode)->right)
                                        FxBoxResourceTree_FreeSubtreeNodes(previewBytes + 0x584,
                                                                           ((PreviewEffectTreeNode*)leafNode)->right);
                                    appFreeUni__FPv(leafNode);
                                }
                                appFreeUni__FPv(childNode);
                            }
                            appFreeUni__FPv(branchNode);
                        }
                        appFreeUni__FPv(rootNode);
                    }
                }
            }
            // Both effect-tree walks free nodes only; the effect box handles its base cleanup.
            __dt__6CFxBoxFv(previewBytes + 0x208, 0);
        }
        if (self) {
            if (self) {
                if (self) {
                    if (self) {
                        void* sentinel = previewBytes + 4;
                        void* end = AsObject801752FC(previewBytes)->listNext;
                        if (end != sentinel) {
                            void* node = AsPreviewResourceNode(sentinel)->previous;
                            AsPreviewResourceNode(AsPreviewResourceNode(end)->previous)->next =
                                AsPreviewResourceNode(node)->next;
                            *(*(void***)&AsPreviewResourceNode(node)->next) = AsPreviewResourceNode(end)->previous;
                            while (end != sentinel) {
                                node = end;
                                end = AsPreviewResourceNode(end)->next;
                                appFreeUni__FPv(node);
                                --((PreviewListView*)self)->count;
                            }
                        }
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_strength_reduction reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma pop

typedef struct {
    unsigned int w[32];
} Blk80;
typedef struct {
    unsigned int w[12];
} Blk30;
typedef struct {
    unsigned int w[10];
} Blk28;
typedef struct {
    unsigned int w[16];
} Blk40;
typedef struct {
    unsigned int w[19];
} Blk76;
typedef struct {
    int a, b;
} Int2;
typedef struct {
    Blk80 a;
    Blk30 b;
    Blk28 c;
    Blk40 d;
} PresetData;

#define CAM (*(void**)&g_view)

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
#include "game/golfball.h"
// Stack-only draw workspace. Affine matrices are 3x4 (12 floats); viewports use
// pixels and depth range, and basis vectors share preview model coordinates.
struct PreviewDrawScratch {
    float crossRight[3];   // +0x000
    float forwardAxis[3];  // +0x00C
    float crossForward[3]; // +0x018
    char unused024[4];
    PreviewViewport savedRingViewport; // +0x028
    PreviewViewport savedBallViewport; // +0x040
    float ringTransform[12];           // +0x058
    float spinInBasis[12];             // +0x088
    float spinRotation[12];            // +0x0B8
    float inverseYaw[12];              // +0x0E8
    float ringBasis[12];               // +0x118: right, normal, forward, translation
    Blk40 ringRenderStates;            // +0x148
    float ballModelTransform[12];      // +0x188
    Blk28 textureParameters;           // +0x1B8
    Blk30 textureStates;               // +0x1E0
    Blk80 renderStates;                // +0x210
};
typedef char PreviewDrawScratchSize[(sizeof(PreviewDrawScratch) == 0x290) ? 1 : -1];
typedef char PreviewDrawScratchBasisOffset[((int)&((PreviewDrawScratch*)0)->ringBasis == 0x118) ? 1 : -1];
typedef char PreviewDrawScratchStatesOffset[((int)&((PreviewDrawScratch*)0)->renderStates == 0x210) ? 1 : -1];

// Draw the lit ball, rotating lie ring and effects, then publish the main camera view.
// Exit uses s_cameraTransform, max(savedNear,0), savedFar, and
// FOV = savedFov / (0.75 * width/height). Screen center and render states stay installed.
// Each renderer separately saves/restores the engine viewport around its draw call.
void ScreenBallPreview_Draw(void* self) {
    char* previewBytes = (char*)self;
    PresetData* presets = (PresetData*)ballPreviewRenderStatePresets;
    PreviewDrawScratch work;

    if (AsCGolfDoc(*(void**)&AsCSharedDoc(*(void**)&g_sharedDoc)->golfDoc)->tutorialStage <
        kPreviewFirstActiveTutorialStage)
        return;

    Blk80* renderStatePreset = &presets->a;
    Blk30* textureStageStates = &presets->b;
    Blk28* textureParameterPreset = &presets->c;
    Blk40* ringStatePreset = &presets->d;
    *(Blk80*)((char*)&work.renderStates) = *renderStatePreset;
    *(Blk30*)((char*)&work.textureStates) = *textureStageStates;
    *(Blk28*)((char*)&work.textureParameters) = *textureParameterPreset;
    Engine_SetRenderStateList((char*)&work.renderStates, 16);
    Engine_SetTextureStageStateList(0, (char*)&work.textureStates, 6);
    Engine_SetTextureStageParameterList(0, (char*)&work.textureParameters, 5);

    // Phase 1: save the main camera projection and install the preview projection.
    float nearClip = AsWView(CAM)->nearClip;
    nearClip = kFloat_0_PvsFxBox < nearClip ? nearClip : kFloat_0_PvsFxBox;
    void* cam = CAM;
    float restoreFovDivisor = kFloat_0_75_ScreenBallPreview;
    float viewportWidth = AsWView(cam)->viewportWidth;
    float viewportHeight = AsWView(cam)->viewportHeight;
    float savedFov = AsWView(cam)->fieldOfView;
    float aspectRatio = viewportWidth / viewportHeight;
    float previewFov = kFloat_1_5707964_PreviewProjection;
    float previewFovScale = kFloat_1_3333334_PreviewEffects;
    float farClip = AsWView(cam)->farClip;
    restoreFovDivisor = restoreFovDivisor * aspectRatio;
    aspectRatio = viewportHeight / viewportWidth;
    previewFov = previewFov * aspectRatio;
    float baseFov = savedFov / restoreFovDivisor;
    previewFov = previewFovScale * previewFov;

    Camera_SetFov(CAM, previewFov);
    Camera_SetClip(CAM, kFloat_0_1_CourseView, kFloat_10_BallPreview, 0);
    WView_SetScreenCenter(CAM, AsObject801752FC(previewBytes)->screenCenterX,
                          AsObject801752FC(previewBytes)->screenCenterY);

    // Phase 2: draw the selected ball with the preview orientation and copied light settings.
    void* obj = (*(void**)&AsObject801752FC(previewBytes)->currentBallResource);
    if (obj != 0 && *(int*)&AsPreviewResourcePair(obj)->resource != 0) {
        WView_SetViewMatrix(CAM, previewBytes + 0xcc);
        Camera_BuildFrustumPlanes(CAM);
        void* v = (*(void**)&AsObject801752FC(previewBytes)->currentBallResource);
        void* d = AsObject801752FC(previewBytes)->ballRenderer;
        ((PreviewBallRendererView*)d)->resource = v;
        d = AsObject801752FC(previewBytes)->ballRenderer;
        ((PreviewBallRendererView*)d)->viewport.x = AsObject801752FC(previewBytes)->ballViewportX;
        ((PreviewBallRendererView*)d)->viewport.y = AsObject801752FC(previewBytes)->ballViewportY;
        ((PreviewBallRendererView*)d)->viewport.width = AsObject801752FC(previewBytes)->ballViewportWidth;
        ((PreviewBallRendererView*)d)->viewport.height = AsObject801752FC(previewBytes)->ballViewportHeight;
        ((PreviewBallRendererView*)d)->viewport.minDepth = AsObject801752FC(previewBytes)->ballViewportMinDepth;
        ((PreviewBallRendererView*)d)->viewport.maxDepth = AsObject801752FC(previewBytes)->ballViewportMaxDepth;
        d = AsObject801752FC(previewBytes)->ballRenderer;
        ((PreviewBallRendererView*)d)->light.type = (*(int*)&AsObject801752FC(previewBytes)->ballLightType);
        *(Int2*)&((PreviewBallRendererView*)d)->light.diffuse[0] =
            *(Int2*)&AsObject801752FC(previewBytes)->ballLightDiffuseRed; // original source +0x68
        *(Int2*)&((PreviewBallRendererView*)d)->light.diffuse[2] =
            *(Int2*)&AsObject801752FC(previewBytes)->ballLightDiffuseBlue; // original source +0x70
        *(Int2*)&((PreviewBallRendererView*)d)->light.specular[0] = *(Int2*)(previewBytes + 0x78);
        *(Int2*)&((PreviewBallRendererView*)d)->light.specular[2] = *(Int2*)(previewBytes + 0x80);
        *(Int2*)&((PreviewBallRendererView*)d)->light.ambient[0] =
            *(Int2*)&AsObject801752FC(previewBytes)->ballLightAmbientRed; // original source +0x88
        *(Int2*)&((PreviewBallRendererView*)d)->light.ambient[2] =
            *(Int2*)&AsObject801752FC(previewBytes)->ballLightAmbientBlue; // original source +0x90
        *(Int2*)&((PreviewBallRendererView*)d)->light.position[0] = *(Int2*)(previewBytes + 0x98);
        *(int*)&((PreviewBallRendererView*)d)->light.position[2] = AsObject801752FC(previewBytes)->ballLightPositionZ;
        *(Int2*)&((PreviewBallRendererView*)d)->light.direction[0] =
            *(Int2*)&AsObject801752FC(previewBytes)->ballLightDirectionX; // original source +0xA4
        *(int*)&((PreviewBallRendererView*)d)->light.direction[2] =
            (*(int*)&AsObject801752FC(previewBytes)->ballLightDirectionZ);
        ((PreviewBallRendererView*)d)->light.range = AsObject801752FC(previewBytes)->ballLightRange;
        ((PreviewBallRendererView*)d)->light.falloff = AsObject801752FC(previewBytes)->ballLightFalloff;
        ((PreviewBallRendererView*)d)->light.constantAttenuation =
            AsObject801752FC(previewBytes)->ballLightConstantAttenuation;
        ((PreviewBallRendererView*)d)->light.linearAttenuation =
            AsObject801752FC(previewBytes)->ballLightLinearAttenuation;
        ((PreviewBallRendererView*)d)->light.quadraticAttenuation =
            AsObject801752FC(previewBytes)->ballLightQuadraticAttenuation;
        ((PreviewBallRendererView*)d)->light.innerCone = AsObject801752FC(previewBytes)->ballLightInnerConeAngle;
        ((PreviewBallRendererView*)d)->light.outerCone = AsObject801752FC(previewBytes)->ballLightOuterConeAngle;

        Matrix_ExtractAffineFrom4x4(previewBytes + 0x1b8, (char*)&work.ballModelTransform);
        void* camv = *(void**)&g_view;
        void* dest = AsObject801752FC(previewBytes)->ballRenderer;
        Engine_GetViewport((char*)&work.savedBallViewport);
        Engine_SetViewport(&((PreviewBallRendererView*)dest)->viewport);
        void** vtDest = *(void***)dest;
        ((void (*)(void*, void*, void*))(vtDest[3]))(dest, camv, (char*)&work.ballModelTransform);
        Engine_SetViewport((char*)&work.savedBallViewport);
    }

    // Phase 3: before the first event, build the ring basis from +0x28 and the +Z axis.
    // Apply the two Y rotations in the existing concat order (spin, basis, inverse ringYaw).
    if (AsCGolfBall(GolfBall__Fv())->m_eventPreDataIndex[0] < 0) {
        WView_SetViewMatrix(CAM, previewBytes + 0x15c);
        Camera_BuildFrustumPlanes(CAM);

        *(Blk40*)((char*)&work.ringRenderStates) = *ringStatePreset;
        Engine_SetRenderStateList((char*)&work.ringRenderStates, 8);

        void* matrix = AsObject801752FC(previewBytes)->ballRingRenderer;
        ((PreviewRingRendererView*)matrix)->viewport.x = AsObject801752FC(previewBytes)->ringViewportX;
        ((PreviewRingRendererView*)matrix)->viewport.y = AsObject801752FC(previewBytes)->ringViewportY;
        ((PreviewRingRendererView*)matrix)->viewport.width = AsObject801752FC(previewBytes)->ringViewportWidth;
        ((PreviewRingRendererView*)matrix)->viewport.height = AsObject801752FC(previewBytes)->ringViewportHeight;
        ((PreviewRingRendererView*)matrix)->viewport.minDepth = AsObject801752FC(previewBytes)->ringViewportMinDepth;
        ((PreviewRingRendererView*)matrix)->viewport.maxDepth = AsObject801752FC(previewBytes)->ringViewportMaxDepth;

        *(WVector*)((char*)&work.ringBasis[3]) = *(WVector*)(previewBytes + 0x28);
        *(float*)((char*)&work.forwardAxis[0]) = kFloat_0_PvsFxBox;
        *(float*)((char*)&work.forwardAxis[1]) = kFloat_0_PvsFxBox;
        *(float*)((char*)&work.forwardAxis[2]) = kFloat_1_0_PvsEffects;
        Vector_CrossProduct((char*)&work.crossRight, (char*)&work.ringBasis[3], (char*)&work.forwardAxis[0]);
        *(WVector*)((char*)&work.ringBasis[0]) = *(WVector*)((char*)&work.crossRight);
        Vector_CrossProduct((char*)&work.crossForward, (char*)&work.ringBasis[0], (char*)&work.ringBasis[3]);
        *(WVector*)((char*)&work.ringBasis[6]) = *(WVector*)((char*)&work.crossForward);
        (*(WVector*)((char*)&work.ringBasis[0])).Normalize();
        (*(WVector*)((char*)&work.ringBasis[6])).Normalize();
        Vec3_Zero((char*)&work.ringBasis[9]);
        Matrix_SetAxisRotation((char*)&work.inverseYaw,
                               kFloat_neg1_PvsEffects * AsObject801752FC(previewBytes)->ringYaw, 1);
        Matrix_SetAxisRotation((char*)&work.spinRotation,
                               kFloat_2_PreviewEffects * AsObject801752FC(previewBytes)->rotationPhase, 1);
        Matrix_ConcatAffine((char*)&work.spinInBasis, (char*)&work.spinRotation, (char*)&work.ringBasis[0]);
        Matrix_ConcatAffine((char*)&work.ringTransform, (char*)&work.spinInBasis, (char*)&work.inverseYaw);
        Matrix_CopyAffine((char*)&work.ringBasis[0], (char*)&work.ringTransform);

        void* camv = *(void**)&g_view;
        void* matrix2 = AsObject801752FC(previewBytes)->ballRingRenderer;
        Engine_GetViewport((char*)&work.savedRingViewport);
        Engine_SetViewport(&((PreviewRingRendererView*)matrix2)->viewport);
        void** vtMatrix = *(void***)matrix2;
        ((void (*)(void*, void*, void*))(vtMatrix[3]))(matrix2, camv, (char*)&work.ringBasis[0]);
        Engine_SetViewport((char*)&work.savedRingViewport);
    }

    // Phase 4: draw embedded effects, then restore the world camera and rebuild its frustum.
    ((CFxBox*)(previewBytes + 0x208))->DrawSprays(CAM);
    {
        void** camVt = *(void***)(char*)CAM;
        ((void (*)(void*, int))(camVt[4]))((char*)CAM, 0);
    }
    Camera_SetClip(CAM, nearClip, farClip, 0);
    WView_SetViewMatrix(CAM, (void*)s_cameraTransform);
    Camera_SetFov(CAM, baseFov);
    Camera_BuildFrustumPlanes(CAM);
}
#pragma opt_strength_reduction reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma pop

} // extern "C"
