#include <decomp/static_init.h>
#include <decomp/record_sizes.h>
typedef unsigned long new_size_t;
extern "C" {
extern unsigned char s_impactTimingQuadTemplate[];
}
#include "game/types.h"
#include "classes/CScreen.h"
#include "iff/IffStruct.h"
#include "classes/CAuxMenuUILayout.h"
#define AsCAuxMenuUILayout(p) ((CAuxMenuUILayout*)(p))
#include "classes/HeadIconTreeNode.h"
#define AsHeadIconTreeNode(p) ((HeadIconTreeNode*)(p))
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/types.h"
#include "game/wvector.h"
#include "classes/CHeadIcon.h"

#define AsCHeadIcon(p) ((CHeadIcon*)(p))
// Translation unit 0x80183098-0x80184750 (11 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

enum { kHeadIconPlayerCount = 4, kNoHeadIcon = 0xFFFF,
       kHeadIconInventorySlots = 13, kHeadIconTextureCount = 9,
       kHeadIconPlayerStride = 0x30, kHeadIconTextureStride = 0x28, kOverlayResourceFlags = 0x80000000,
       kClubOverlayLight = 7, kEngineLightCount = 8 };
struct AuxOverlayStringPool {
    char otherScreenStrings[0x3F8];
    char optionBase[0x11];
    char optionBlack[0x12];
    char optionWhite[0x12];
};
struct HeadIconStringPool { char defaultOverlay[0x19]; char itemOverlayFormat[0xE]; };
struct HeadIconKeyValue { unsigned int key; void* overlay; };
// Field-derived byte offsets retain the original CSE and register allocation.
static const int kHeadIconMapOffset = (int)&((CHeadIcon*)0)->overlayNodeCount;
static const int kHeadIconRootOffset = (int)&((CHeadIcon*)0)->overlayTreeRoot;
static const int kHeadIconLeftOffset = (int)&((HeadIconTreeNode*)0)->left;
static const int kHeadIconRightOffset = (int)&((HeadIconTreeNode*)0)->right;
static const int kHeadIconKeyOffset = (int)&((HeadIconTreeNode*)0)->key;
static const int kHeadIconFormatOffset = (int)&((HeadIconStringPool*)0)->itemOverlayFormat;
struct HeadIconMapStorage { int count; void* root; void* firstNode; };
struct HeadIconPlayerPointView {
    char prefix[0x1C];
    float screenX, screenY, screenZ;
    char reserved28[0xC];
    Vec3f cameraRelativePosition;
};
struct ClubOverlayAnimationView { char prefix[0x1E8]; float revealAngle; char reserved1EC[4]; float wobbleAngle; };
struct ClubOverlayRenderState {
    char reserved00[0x20];
    float viewMatrix[16];
    float projectionMatrix[16];
};
static const int kClubViewMatrixOffset = (int)&((ClubOverlayRenderState*)0)->viewMatrix;
static const int kClubProjectionMatrixOffset = (int)&((ClubOverlayRenderState*)0)->projectionMatrix;
struct ClubOverlaySelection { char reserved00[0xA]; unsigned char modelVariant; };
struct ClubOverlayInstances { void* reserved00; ClubOverlaySelection* selection; };
struct SharedClubSelectionsView {
    char prefix[0x1CC];
    struct Slot { void* clubItem; char remainingPlayerData[0x1D0]; } players[4];
};

void Object_DeleteVirtual(void* ignored, void* object);
void WView_ProjectWorldPoint(void* a, void* b, void* c);
void __mi__FRC7WVectorRC7WVector(void* a, void* b, void* c);
void __as__7WVectorFRC7WVector(void* a, void* b);
// Recursively free the overlay-map subtree, including object itself.
// allocator is the borrowed map owner address.
void OverlayMapTree_FreeSubtree(void* allocator, void* object);
void appFreeUni__FPv(void* object);
void BaseObject_ctor(void* object);
void* WOverlay_Create(void* a, void* b, int c);
void* __nwa__FUl(int size);
void* ItemInventory_GetItemIdAt(void* base, unsigned char idx);
void* ItemInventory_FindItemData(void* base, void* arg);
void* String_FormatTemporary(void* fmt, ...);
void* HeadIconMap_FindInsertionPosition(void* a, void* b, void* c, void* d, void* e);
void* HudFontConfigMap_InsertAndRebalance(void* a, void* b, unsigned char c, unsigned char d, void* e);
void CBpTex_Init(void* object);
void* __construct_new_array(void* block, void* ctor, void* dtor, unsigned int size, unsigned int n);
void __destroy_new_array(void* object, void* dtor);
void* BaseObject_dtor(void* object, int shouldDelete);
void __dl__FPv(void* object);
void* CScreen_dtor(void* self, int shouldDelete);
void CBpTex_dtor(void* object, int flag);

extern char __vt__9CHeadIcon[];
extern char __vt__10CAuxMenuUI[];
extern char s_headIconBaseVtable[]; // base vtable installed before CHeadIcon
extern char screenOverlayStrings[];
extern void* g_resourceManager;
extern void* g_headIcon;
extern CSharedDoc* g_sharedDoc;
extern char headIconResourceStrings[];
extern int kHeadIconTextureIds[]; // nine texture IDs passed to CBpTex initialization
extern void* g_headIconMapDefaultValue;
extern const float kFloat_0_0_HeadIcon, kFloat_64_HeadIcon, kFloat_0_015625_HeadIcon;
extern const float kFloat_6_2831855;
extern float s_twoPi_80183098;

// --- CScreen_DrawClubModelOverlay ---
void* InstanceTable_Get();
void* ClubModel_FindCachedForItem(void* object, void* key);
double pow(double x, double y);
void Camera_SetFov(void* camera, float fov);
void Camera_SetClip(void* camera, float nearClip, float farClip, int index);
void Camera_Apply(void* camera, float x, float y);
void Camera_BuildFrustumPlanes(void* camera);
void WView_SetViewMatrix(void* camera, void* matrix);
void Matrix_BuildCameraTransform(void* out, void* from, void* to);
void WView_SetViewportRect(void* camera, int flag);
void* Camera_GetRenderState(void* camera);
void Matrix_SetAxisRotation(void* out, float angle, signed char axis);
void Matrix_SetIdentityAffine(void* out);
void Matrix_ConcatAffine(void* out, void* a, void* b);
void Model_SetMatrixRecursive(void* object, int mode, void* matrix);
void WPuppet_UpdateBoneWorldMatrices(void* object, int a, int b, float value);
void WPuppet_SetOpacityRecursive(void* object, unsigned char value);
float Math_SinApprox(float angle);
int WPuppet_GetSkinMatrices(void* object);
void SetTransform(int state, void* matrix);
void gtl_GenericRenderer_SetProjectionMatrix(void* matrix);
void gtl_GenericRenderer_SetViewMatrix(void* matrix);
void Engine_GetViewport(void* state);
void Engine_SetViewport(void* state);
void Engine_SetRenderStateList(void* state, int count);
void Engine_SetTextureStageStateList(int index, void* state, int count);
void Engine_SetTextureStageParameterList(int index, void* state, int count);
void Engine_GetLightEnabled(int index, void* out);
void Engine_SetLightEnabled(int index, int value);
void Engine_SetLight(int index, void* value);

extern WView* g_view;
extern CSharedDoc* g_sharedDoc;
struct RenderData80183098;
extern RenderData80183098 s_clubOverlayRenderDefaults; // club-preview viewport/render/texture defaults
extern char s_cameraTransform[];
extern double kDouble_3_ViewportScale;
extern float kFloat_0_AuxMenu, kFloat_1_AuxMenu, kFloat_6_2831855_Screen, kFloat_0_3_AuxMenu, kFloat_255_Screen;
extern float kFloat_1_2_ScreenOverlay, kFloat_640_ClubModelOverlay, kScreenOverlayReferenceHeight, kFloat_0_75_ScreenOverlay, kFloat_1_3333334_ScreenOverlay;
extern float kFloat_1_0471976_ClubModelOverlay, kFloat_2_25_ClubModelOverlay, kFloat_neg1_37_ScreenOverlay, kFloat_neg2_4_ClubModelOverlay, kFloat_neg3_3_ClubModelOverlay;
extern float kFloat_neg4_2_ClubModelOverlay, kFloat_0_08726646_ScreenOverlay, kFloat_2000_ScreenOverlay;


struct Mtx { Vec3f a, b, c, d; };
struct MtxBlob { float m[12]; };
struct Block72 { int w[18]; };
struct Block48 { int w[12]; };
struct Block40 { int w[10]; };
struct Block24 { int w[6]; };

struct RenderData80183098 {
    char pad0[0x98];
    Block24 pre;
    Block72 a;
    Block48 b;
    Block40 c;
};

struct ClubModelCacheView {
    char reserved00[0x14];
    void* models[4];
    Mtx viewMatrices[4];
};
static const int kClubMatrixOffset = (int)&((ClubModelCacheView*)0)->viewMatrices;
static const int kClubMatrixStride = sizeof(Mtx);

struct Frame80183098 {
    float lookAtTarget[3];
    float lookFrom[3];
    int savedLightEnabled[8];
    Block24 overlayViewport;
    Block24 savedViewport;
    Mtx modelTransform;
    Block40 textureParameters;
    Block48 textureStates;
    Mtx modelTranslation;
    Mtx modelRotation;
    Mtx cameraTransform;
    Mtx cachedViewMatrix;
    Block72 renderStates;
};

struct CameraObj {
    virtual void v0();
    virtual void v1();
    virtual void setup(int unused);
};

struct DrawObj {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void draw(void* camera, int a, int b, int c, float d, int e, int f, int g);
};

// Draw the current player club in an animated lit preview, restoring viewport, lights, and camera afterward.
void CScreen_DrawClubModelOverlay(char* self) {
    Frame80183098 frame;
    RenderData80183098* data = &s_clubOverlayRenderDefaults;
    char* shared = (char*)g_sharedDoc;
    unsigned char index = AsCGolfDoc((*(void**)&AsCSharedDoc(shared)->golfDoc))->currentPlayer;
    void* clubCache = ClubModel_FindCachedForItem(InstanceTable_Get(), ((SharedClubSelectionsView*)shared)->players[index].clubItem);

    // Phase 1: resolve the selected club variant and capture camera restoration arguments.
    // Capture restoration arguments before installing the preview camera. Near
    // clip is deliberately clamped to zero; restoration is not a raw state copy.
    float savedNearClip = AsWView(g_view)->nearClip;
    savedNearClip = (kFloat_0_AuxMenu < savedNearClip) ? savedNearClip : kFloat_0_AuxMenu;
    float savedFarClip = AsWView(g_view)->farClip;

    pow(AsWView(g_view)->viewportWidth / kFloat_640_ClubModelOverlay, kDouble_3_ViewportScale);
    pow(AsWView(g_view)->viewportHeight / kScreenOverlayReferenceHeight, kDouble_3_ViewportScale);

    unsigned char modelVariant = ((ClubOverlayInstances*)InstanceTable_Get())->selection->modelVariant;
    char* entry = (char*)clubCache + modelVariant * kClubMatrixStride;
    // One logical camera-frame load from the variant cache. Repeated typed/blob
    // copies preserve the original compiler's load scheduling and alias behavior.
    frame.cachedViewMatrix = *(Mtx*)(entry + kClubMatrixOffset);
    frame.cachedViewMatrix = *(Mtx*)(entry + kClubMatrixOffset);
    *(MtxBlob*)&frame.cachedViewMatrix = *(MtxBlob*)(entry + kClubMatrixOffset);
    frame.cachedViewMatrix = *(Mtx*)(entry + kClubMatrixOffset);
    frame.cachedViewMatrix = *(Mtx*)(entry + kClubMatrixOffset);
    *(MtxBlob*)&frame.cachedViewMatrix = *(MtxBlob*)(entry + kClubMatrixOffset);

    float restoreFovArgument;
    {
        float d;
        float h;
        float w;
        w = AsWView(g_view)->viewportWidth;
        h = AsWView(g_view)->viewportHeight;
        d = AsWView(g_view)->fieldOfView;
        // Compute the aspect-adjusted restoration argument from the old FOV.
        // Camera_SetFov stores its argument directly (wview_camera_projection.cpp); it does not undo
        // this division, so the final FOV can differ from the originally read value.
        restoreFovArgument = d / (kFloat_0_75_ScreenOverlay * (w / h));
        Camera_SetFov(g_view, kFloat_1_3333334_ScreenOverlay * (kFloat_1_0471976_ClubModelOverlay * (h / w)));

        WView_SetViewMatrix(g_view, &frame.cachedViewMatrix);
        Camera_BuildFrustumPlanes(g_view);

        modelVariant = ((ClubOverlayInstances*)InstanceTable_Get())->selection->modelVariant;
        void* clubModel = ((ClubModelCacheView*)clubCache)->models[modelVariant];
        if (clubModel != 0) {
            float baseRotation;
            float modelOffset;

            // Phase 2: install preview viewport/camera only when this variant has a model.
            // Install the preview viewport and aim a temporary camera at the origin.
            frame.overlayViewport = *(Block24*)((char*)(&data->pre));
            Engine_GetViewport(&frame.savedViewport);
            Engine_SetViewport(&frame.overlayViewport);

            frame.lookAtTarget[0] = kFloat_0_AuxMenu;
            frame.lookAtTarget[1] = kFloat_0_AuxMenu;
            frame.lookAtTarget[2] = kFloat_0_AuxMenu;
            frame.lookFrom[0] = kFloat_2_25_ClubModelOverlay;
            frame.lookFrom[1] = kFloat_neg1_37_ScreenOverlay;
            frame.lookFrom[2] = kFloat_1_2_ScreenOverlay;
            Matrix_BuildCameraTransform(&frame.cameraTransform, frame.lookFrom, frame.lookAtTarget);
            WView_SetViewMatrix(g_view, &frame.cameraTransform);
            Camera_BuildFrustumPlanes(g_view);

            if (((ClubOverlayInstances*)InstanceTable_Get())->selection->modelVariant != 0)
                baseRotation = kFloat_neg2_4_ClubModelOverlay;
            else
                baseRotation = kFloat_neg3_3_ClubModelOverlay;

            if (((ClubOverlayInstances*)InstanceTable_Get())->selection->modelVariant != 0)
                modelOffset = kFloat_neg3_3_ClubModelOverlay;
            else
                modelOffset = kFloat_neg4_2_ClubModelOverlay;

            // Phase 3: animate the model in preview space; opacity is revealAngle / full turn.
            // Compose the club's animated rotation and translation in preview space.
            Matrix_SetAxisRotation(&frame.modelRotation, kFloat_0_08726646_ScreenOverlay * Math_SinApprox(((ClubOverlayAnimationView*)self)->wobbleAngle) +
                            (-((ClubOverlayAnimationView*)self)->revealAngle + baseRotation),
                        0);
            Matrix_SetIdentityAffine(&frame.modelTranslation);
            frame.modelTranslation.d.x = modelOffset;
            Matrix_ConcatAffine(&frame.modelTransform, &frame.modelRotation, &frame.modelTranslation);
            Model_SetMatrixRecursive(clubModel, 0, &frame.modelTransform);

            {
                float lo;
                float hi = kFloat_255_Screen;
                float value = hi * ((ClubOverlayAnimationView*)self)->revealAngle / kFloat_6_2831855_Screen;
                lo = kFloat_0_AuxMenu;
                lo = value < lo ? lo : (value > hi ? hi : value);
                WPuppet_SetOpacityRecursive(clubModel, (unsigned char)lo);
            }
            WPuppet_UpdateBoneWorldMatrices(clubModel, 0, 0, kFloat_1_AuxMenu);

            {
                Block72* gxa = (Block72*)((char*)(&data->a));
                Block48* gxb = (Block48*)((char*)(&data->b));
                Block40* gxc = (Block40*)((char*)(&data->c));
                frame.renderStates = *gxa;
                frame.textureStates = *gxb;
                frame.textureParameters = *gxc;
            }
            Engine_SetRenderStateList(&frame.renderStates, 9);
            Engine_SetTextureStageStateList(0, &frame.textureStates, 6);
            Engine_SetTextureStageParameterList(0, &frame.textureParameters, 5);

            Camera_SetClip(g_view, kFloat_0_3_AuxMenu, kFloat_2000_ScreenOverlay, 0);
            Camera_Apply(g_view, kFloat_0_AuxMenu, kFloat_0_AuxMenu);

            {
                char* view = (char*)Camera_GetRenderState(g_view);
                SetTransform(1, view + kClubProjectionMatrixOffset);
                gtl_GenericRenderer_SetProjectionMatrix(view + kClubProjectionMatrixOffset);
                SetTransform(0, view + kClubViewMatrixOffset);
                gtl_GenericRenderer_SetViewMatrix(view + kClubViewMatrixOffset);
            }

            {
                // Phase 4: submit once using preview lighting; restore all enable flags and viewport.
                // Save all light enables; draw using only the preview light in slot 7.
                char* saved = (char*)frame.savedLightEnabled;
                int i;
                for (i = 0; i < kEngineLightCount; i++) {
                    Engine_GetLightEnabled(i, saved);
                    Engine_SetLightEnabled(i, 0);
                    saved += 4;
                }
                Engine_SetLight(kClubOverlayLight, &((CScreen*)self)->lightType);
                Engine_SetLightEnabled(kClubOverlayLight, 1);

                {
                    int count = WPuppet_GetSkinMatrices(clubModel);
                    ((DrawObj*)clubModel)->draw(g_view, 1, 1, 0, kFloat_1_AuxMenu, 0, count, 0);
                }

                Engine_SetLightEnabled(kClubOverlayLight, 0);
                saved = (char*)frame.savedLightEnabled;
                for (i = 0; i < kEngineLightCount; i++) {
                    Engine_SetLightEnabled(i, *(int*)saved);
                    saved += 4;
                }
                Engine_SetViewport(&frame.savedViewport);
            }
        }

        // Phase 5: rebuild normal camera even when the selected model was absent.
        // Rebuild normal camera/frustum state from the global transform and saved
        // setter arguments. The preview matrix is not the matrix restored here.
        ((CameraObj*)g_view)->setup(0);
        Camera_SetClip(g_view, savedNearClip, savedFarClip, 0);
        WView_SetViewMatrix(g_view, s_cameraTransform);
        Camera_SetFov(g_view, restoreFovArgument);
        Camera_BuildFrustumPlanes(g_view);
        WView_SetViewportRect(g_view, 0);
    }
}

// KILL1..KILL4 / DESTROY_LEAF / ERASE_* below are one idea: "free every node of the overlay map tree, children
// first". They are macros (not functions) because an inline-function rewrite changed the generated code
// (register allocation in CHeadIcon_ctor / CHeadIcon_dtor). KILLn frees n expanded levels below a node.
// Overlay map ownership: +F8 is the map context and +FC its root/sentinel slot.
// Every +0/+4 access below follows a left/right CHILD NODE, at every depth; it
// does not load a payload pointer. OverlayMapTree_FreeSubtree recursively cleans
// those remaining child subtrees. Each expanded ancestor is then freed once.
// Key/overlay payload starts at +0x0C (HeadIconKeyValue), separate from the links;
// no additional overlay-release call occurs in this traversal. Parent +8 has a
// color tag in bit 0. Keep the map alive until all child cleanup calls complete.
// Release the two remaining child subtrees; this helper leaves their parent allocated.
static inline void ClearHeadIconChildSubtrees(void* self, void* node) {
    void* child = (*(void**)((char*)node + kHeadIconLeftOffset));
    if (child != 0) OverlayMapTree_FreeSubtree(((char*)self + kHeadIconMapOffset), child);
    child = (*(void**)((char*)node + kHeadIconRightOffset));
    if (child != 0) OverlayMapTree_FreeSubtree(((char*)self + kHeadIconMapOffset), child);
}

#define KILL1(n)                                                     \
    do {                                                             \
        void* _n1 = (n);                                             \
        if (_n1 != 0) {                                              \
            ClearHeadIconChildSubtrees(self, _n1);                                           \
            appFreeUni__FPv(_n1);                                        \
        }                                                            \
    } while (0)

#define KILL2(n)                                                     \
    do {                                                             \
        void* _n2 = (n);                                             \
        if (_n2 != 0) {                                              \
            KILL1((*(void**)((char*)_n2 + kHeadIconLeftOffset)));                      \
            KILL1((*(void**)((char*)_n2 + kHeadIconRightOffset)));                      \
            appFreeUni__FPv(_n2);                                        \
        }                                                            \
    } while (0)

#define KILL3(n)                                                     \
    do {                                                             \
        void* _n3 = (n);                                             \
        if (_n3 != 0) {                                              \
            KILL2((*(void**)((char*)_n3 + kHeadIconLeftOffset)));                      \
            KILL2((*(void**)((char*)_n3 + kHeadIconRightOffset)));                      \
            appFreeUni__FPv(_n3);                                        \
        }                                                            \
    } while (0)

#define KILL4(n)                                                     \
    do {                                                             \
        void* _n4 = (n);                                             \
        if (_n4 != 0) {                                              \
            KILL3((*(void**)((char*)_n4 + kHeadIconLeftOffset)));                      \
            KILL3((*(void**)((char*)_n4 + kHeadIconRightOffset)));                      \
            appFreeUni__FPv(_n4);                                        \
        }                                                            \
    } while (0)

struct Nine { int w[9]; };

struct CBpTex {
    char pad[0x28];
    // Allocate the texture array including the compiler's array header.
    static void* operator new[](new_size_t size) { return __nwa__FUl(size); }
    // Initialize one animated texture element.
    CBpTex() { CBpTex_Init(this); }
    // Destroy one texture without freeing the enclosing array.
    ~CBpTex() { CBpTex_dtor(this, -1); }
};

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    char reserved08[0x48];
    unsigned int key;
    char reserved54[4];
};

struct ArrElem {
    virtual void v0();
    virtual void v1(int value);
};

}
struct AuxMenuBase { char bytes[4]; ~AuxMenuBase(); };
struct CAuxMenuUI : AuxMenuBase {
    char rest[0x20];
    CAuxMenuUI();
};
// Load the base, black, and white auxiliary menu overlay resources.
CAuxMenuUI::CAuxMenuUI() {
    void* self = this;
    char* strBase = screenOverlayStrings;
    int zero = 0;
    AsCAuxMenuUILayout(self)->vtbl = (void*)__vt__10CAuxMenuUI;
    AsCAuxMenuUILayout(self)->firstOverlay = 0;
    void* r3 = WOverlay_Create((void*)g_resourceManager, ((AuxOverlayStringPool*)strBase)->optionBase, kOverlayResourceFlags);
    AsCAuxMenuUILayout(self)->firstOverlay = r3;
    void* r3b = WOverlay_Create((void*)g_resourceManager, ((AuxOverlayStringPool*)strBase)->optionBlack, kOverlayResourceFlags);
    AsCAuxMenuUILayout(self)->secondOverlay = r3b;
    void* r3c = WOverlay_Create((void*)g_resourceManager, ((AuxOverlayStringPool*)strBase)->optionWhite, kOverlayResourceFlags);
    float f0 = kFloat_0_AuxMenu;
    AsCAuxMenuUILayout(self)->thirdOverlay = r3c;
    AsCAuxMenuUILayout(self)->unk_20 = 2;
    AsCAuxMenuUILayout(self)->unk_14 = 0;
    AsCAuxMenuUILayout(self)->unk_1C = f0;
}
extern "C" {

// Delete the three auxiliary menu overlays and optionally free the menu object.
void* CAuxMenuUI_dtor(void* self, int shouldDelete) {
    if (self == 0) {
        // still return? assembly returns self even if null
    }
    if (self != 0) {
        AsCAuxMenuUILayout(self)->vtbl = (void*)__vt__10CAuxMenuUI;
        void* allocator = (void*)g_resourceManager;
        if (allocator != 0) {
            void* o = AsCAuxMenuUILayout(self)->firstOverlay;
            if (o != 0) {
                Object_DeleteVirtual(allocator, o);
                AsCAuxMenuUILayout(self)->firstOverlay = 0;
            }
        }
        // second overlay
        {
            void* allocator2 = (void*)g_resourceManager;
            if (allocator2 != 0) {
                void* o = AsCAuxMenuUILayout(self)->secondOverlay;
                if (o != 0) {
                    Object_DeleteVirtual(allocator2, o);
                    AsCAuxMenuUILayout(self)->secondOverlay = 0;
                }
            }
        }
        // third
        {
            void* allocator3 = (void*)g_resourceManager;
            if (allocator3 != 0) {
                void* o = AsCAuxMenuUILayout(self)->thirdOverlay;
                if (o != 0) {
                    Object_DeleteVirtual(allocator3, o);
                    AsCAuxMenuUILayout(self)->thirdOverlay = 0;
                }
            }
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Initialize the head-icon unit copy of two pi at static startup.
void CHeadIcon_InitStaticConstants(void) {
    s_twoPi_80183098 = kFloat_6_2831855;
}

// Adjust a secondary-base pointer back to CScreen before destruction.
void* CScreen_DtorThunk18(void* self, int shouldDelete) {
    return CScreen_dtor((char*)self - 0x18, shouldDelete);
}

#pragma use_lmw_stmw on
// Initialize four player icons, load inventory-specific overlays, and construct nine animated textures.
void* CHeadIcon_ctor(void* selfIn) {
    char* self = (char*)selfIn;
    // Phase 1: publish the singleton and establish an empty tree plus four inactive player slots.
    BaseObject_ctor(self);
    *(void**)self = &s_headIconBaseVtable;
    g_headIcon = self;
    *(void**)self = &__vt__9CHeadIcon;
    {
        AsCHeadIcon(self)->overlayNodeCount = 0;
        (*(int*)&AsCHeadIcon(self)->overlayTreeRoot) = 0;
        AsCHeadIcon(self)->firstOverlayNode = self + kHeadIconRootOffset;
        AsCHeadIcon(self)->drawingEnabled = 1;
        AsCHeadIcon(self)->player0IconType = kNoHeadIcon;
        AsCHeadIcon(self)->player0IconElapsed = kFloat_0_0_HeadIcon;
        AsCHeadIcon(self)->player0OverlayObject = 0;
        AsCHeadIcon(self)->player1IconType = kNoHeadIcon;
        AsCHeadIcon(self)->player1IconElapsed = kFloat_0_0_HeadIcon;
        AsCHeadIcon(self)->player1OverlayObject = 0;
        AsCHeadIcon(self)->player2IconType = kNoHeadIcon;
        AsCHeadIcon(self)->player2IconElapsed = kFloat_0_0_HeadIcon;
        AsCHeadIcon(self)->player2OverlayObject = 0;
        AsCHeadIcon(self)->player3IconType = kNoHeadIcon;
        AsCHeadIcon(self)->player3IconElapsed = kFloat_0_0_HeadIcon;
        AsCHeadIcon(self)->player3OverlayObject = 0;
        // Phase 2: create the shared icon overlay and derive normalized atlas extents.
        void* ret = WOverlay_Create(g_resourceManager, &headIconResourceStrings, 0);
        float inverseAtlasPixels;
        float iconPixels;
        iconPixels = kFloat_64_HeadIcon;
        inverseAtlasPixels = kFloat_0_015625_HeadIcon;
        void* node = *(void**)(self + kHeadIconRootOffset);
        float iconUvExtent = iconPixels * inverseAtlasPixels;
        AsCHeadIcon(self)->overlay = ret;
        AsCHeadIcon(self)->iconWidth = iconPixels;
        AsCHeadIcon(self)->iconHeight = iconPixels;
        AsCHeadIcon(self)->iconUvWidth = iconUvExtent;
        AsCHeadIcon(self)->iconUvHeight = iconUvExtent;
        if (node != 0) {
            KILL3(*(void**)((char*)node + kHeadIconLeftOffset));
            KILL3(*(void**)((char*)node + kHeadIconRightOffset));
            appFreeUni__FPv(node);
            AsCHeadIcon(self)->overlayNodeCount = 0;
            *(int*)(self + kHeadIconRootOffset) = 0;
            AsCHeadIcon(self)->firstOverlayNode = self + kHeadIconRootOffset;
        }
    }
    // Phase 3: populate inventory overlays; texture-array construction has not started yet.
    // Populate the owned overlay map by unsigned inventory item typeId, not slot.
    // Duplicate typeIds reuse their value slot; node insertion owns the copied key.
    IFF_STRUCT::sItem* found;
    char* base;
    unsigned int i;
    for (i = 0; i < kHeadIconInventorySlots; i++) {
        base = AsCSharedDoc(g_sharedDoc)->itemInventory;
        found = (IFF_STRUCT::sItem*)ItemInventory_FindItemData(base, ItemInventory_GetItemIdAt(base, (unsigned char)i));
        IFF_STRUCT::sItem* item = found;
        if (item != 0) {
            void* itemOverlay = WOverlay_Create(g_resourceManager, String_FormatTemporary(headIconResourceStrings + kHeadIconFormatOffset, item->common.icon), 0);
            if (itemOverlay != 0) {
                int predecessorAddress;
                unsigned char noPredecessor;
                unsigned char insertLeft;
                void* ret = HeadIconMap_FindInsertionPosition(self + kHeadIconMapOffset, &item->common.typeId, &predecessorAddress, &insertLeft, &noPredecessor);
                void* slot;
                if (predecessorAddress == 0 || *(unsigned int*)((char*)predecessorAddress + kHeadIconKeyOffset) < item->common.typeId) {
                    int tmp[2];
                    tmp[0] = (int)item->common.typeId;
                    tmp[1] = (int)g_headIconMapDefaultValue;
                    slot = (char*)HudFontConfigMap_InsertAndRebalance(self + kHeadIconMapOffset, ret, insertLeft, noPredecessor, &tmp) + kHeadIconKeyOffset;
                } else {
                    slot = (char*)predecessorAddress + kHeadIconKeyOffset;
                }
                ((HeadIconKeyValue*)slot)->overlay = itemOverlay;
            }
        }
    }
    (*(int*)&AsCHeadIcon(self)->textures) = 0;
    // Phase 4: allocate exactly nine CBpTex objects, then bind their animation IDs.
    // The array owns its elements; inventory overlays above are stored in the map.
    Nine textureIds = *(Nine*)&kHeadIconTextureIds;
    CBpTex* arr = new CBpTex[kHeadIconTextureCount];
    (*(CBpTex**)&AsCHeadIcon(self)->textures) = arr;
    for (int i = 0; i < kHeadIconTextureCount; i++) {
        int textureId = textureIds.w[i];
        ArrElem* elem = (ArrElem*)((char*)AsCHeadIcon(self)->textures + i * kHeadIconTextureStride);
        elem->v1(textureId);
    }
    return self;
}

#define DESTROY_LEAF(n)                                              \
    do {                                                             \
        void* _v = (*(void**)((char*)n + kHeadIconLeftOffset));                      \
        if (_v != 0) OverlayMapTree_FreeSubtree(((char*)self + kHeadIconMapOffset), _v);            \
        _v = (*(void**)((char*)n + kHeadIconRightOffset));                            \
        if (_v != 0) OverlayMapTree_FreeSubtree(((char*)self + kHeadIconMapOffset), _v);            \
        appFreeUni__FPv(n);                                              \
    } while (0)

#define ERASE_LEVEL2(x)                                              \
    do {                                                             \
        void* _x = (x);                                              \
        if (_x != 0) {                                               \
            void* _y = (*(void**)((char*)_x + kHeadIconLeftOffset));                   \
            if (_y != 0) DESTROY_LEAF(_y);                           \
            _y = (*(void**)((char*)_x + kHeadIconRightOffset));                         \
            if (_y != 0) DESTROY_LEAF(_y);                           \
            appFreeUni__FPv(_x);                                         \
        }                                                            \
    } while (0)

#define ERASE_TREE(root)                                             \
    do {                                                             \
        void* _root = (root);                                        \
        {                                                            \
            void* _l = (*(void**)((char*)_root + kHeadIconLeftOffset));                \
            if (_l != 0) {                                           \
                ERASE_LEVEL2((*(void**)((char*)_l + kHeadIconLeftOffset)));            \
                ERASE_LEVEL2((*(void**)((char*)_l + kHeadIconRightOffset)));            \
                appFreeUni__FPv(_l);                                     \
            }                                                        \
            _l = (*(void**)((char*)_root + kHeadIconRightOffset));                      \
            if (_l != 0) {                                           \
                ERASE_LEVEL2((*(void**)((char*)_l + kHeadIconLeftOffset)));            \
                ERASE_LEVEL2((*(void**)((char*)_l + kHeadIconRightOffset)));            \
                appFreeUni__FPv(_l);                                     \
            }                                                        \
            appFreeUni__FPv(_root);                                      \
        }                                                            \
    } while (0)

// Clear the global head-icon instance, destroy its base, and optionally free it.
void* HeadIcon_dtor(void* self, int deleting) {
    if (self != 0) {
        g_headIcon = 0;
        BaseObject_dtor(self, 0);
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma opt_lifetimes off
#pragma opt_common_subs off
// Find a map insertion parent while tracking the predecessor and which child edge to insert.
// keyPtr points to an unsigned item typeId. Returns the final parent (or &root for
// an empty tree); outInsertLeft selects that parent's edge. outPredecessor receives
// the greatest key <= query, or null. outNoPredecessor is 1 until a right branch
// establishes a predecessor; it is not a duplicate-key flag. The caller checks
// equality before insertion. All outputs are borrowed node addresses/byte flags.
void* HeadIconMap_FindInsertionPosition(void* tree, void* keyPtr, void* outPredecessor, void* outInsertLeft, void* outNoPredecessor) {
    unsigned char one;
    int zero;
    void* parent;
    void* node;
    unsigned int key;
    unsigned int nodeKey;
    zero = 0;
    one = 1;
    *(int*)outPredecessor = zero;
    parent = &((HeadIconMapStorage*)tree)->root;
    node = ((HeadIconMapStorage*)tree)->root;
    *(unsigned char*)outInsertLeft = one;
    *(unsigned char*)outNoPredecessor = one;
    while (node != 0) {
        key = *(unsigned int*)keyPtr;
        parent = node;
        nodeKey = AsHeadIconTreeNode(node)->key;
        if (key < nodeKey) {
            node = (*(void**)((char*)node + kHeadIconLeftOffset));
            *(unsigned char*)outInsertLeft = one;
        } else {
            *(void**)outPredecessor = node;
            node = (*(void**)((char*)node + kHeadIconRightOffset));
            *(unsigned char*)outInsertLeft = (unsigned char)zero;
            *(unsigned char*)outNoPredecessor = (unsigned char)zero;
        }
    }
    return parent;
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_lifetimes off
// Release player/map overlays, erase the map, destroy textures, and clear the singleton.
void* CHeadIcon_dtor(char* self, int deleting) {
    void* head;
    unsigned int i;
    void* node;
    void* end;
    void* root;
    if (self != 0) {
        AsCHeadIcon(self)->vtbl = &__vt__9CHeadIcon;
        if (g_resourceManager != 0 && AsCHeadIcon(self)->overlay != 0) {
            Object_DeleteVirtual(g_resourceManager, AsCHeadIcon(self)->overlay);
            AsCHeadIcon(self)->overlay = 0;
        }

        {
            i = 0;
            head = self;
            do {
                if (g_resourceManager != 0 && (*(void**)&AsCHeadIcon(head)->player0OverlayObject) != 0) {
                    Object_DeleteVirtual(g_resourceManager, (*(void**)&AsCHeadIcon(head)->player0OverlayObject));
                    (*(void**)&AsCHeadIcon(head)->player0OverlayObject) = 0;
                }
                i++;
                head = (char*)head + kHeadIconPlayerStride;
            } while (i < 4);

            node = AsCHeadIcon(self)->firstOverlayNode;
            end = self + kHeadIconRootOffset;
            while (node != end) {
                if (g_resourceManager != 0 && AsHeadIconTreeNode(node)->overlay != 0) {
                    Object_DeleteVirtual(g_resourceManager, AsHeadIconTreeNode(node)->overlay);
                    AsHeadIconTreeNode(node)->overlay = 0;
                }
                {
                    void* next = AsHeadIconTreeNode(node)->right;
                    if (next != 0) {
                        while (*(void**)next != 0) {
                            next = *(void**)next;
                        }
                        node = next;
                    } else {
                        void* parent;
                        while (node !=
                               *(void**)(parent = (void*)(AsHeadIconTreeNode(node)->parentAndColor &
                                                         ~1u))) {
                            node = parent;
                        }
                        node = parent;
                    }
                }
            }
        }

        {
            root = AsCHeadIcon(self)->overlayTreeRoot;
            if (root != 0) {
                ERASE_TREE(root);
                (*(void**)&AsCHeadIcon(self)->overlayNodeCount) = 0;
                AsCHeadIcon(self)->overlayTreeRoot = 0;
                AsCHeadIcon(self)->firstOverlayNode = self + kHeadIconRootOffset;
            }
        }

        if (AsCHeadIcon(self)->textures != 0) {
            __destroy_new_array(AsCHeadIcon(self)->textures, (void*)CBpTex_dtor);
            AsCHeadIcon(self)->textures = 0;
        }

        // (The six identical `!= 0` tests are always true: the member address is never null; they are
        // duplicates of inlined member-destructor guards.)
        // The container at this+0xF8 is already empty here (its root was just cleared), so these
        // null checks of the member destructor guard skip the erase below.
        if (self + kHeadIconMapOffset != 0 && self + kHeadIconMapOffset != 0 && self + kHeadIconMapOffset != 0 &&
            self + kHeadIconMapOffset != 0 && self + kHeadIconMapOffset != 0 && self + kHeadIconMapOffset != 0) {
            void* root = AsCHeadIcon(self)->overlayTreeRoot;
            if (root != 0) {
                ERASE_TREE(root);
            }
        }

        if (self != 0) {
            g_headIcon = 0;
            BaseObject_dtor(self, 0);
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma opt_lifetimes reset
#pragma use_lmw_stmw off

// Release each active player icon object, capped at the four supported player slots.
void CHeadIcon_ReleasePlayerObjects(void* self) {
    int n = 4;
    void* allocator = (void*)g_sharedDoc;
    allocator = (*(void**)&AsCSharedDoc(allocator)->golfDoc);
    int b = AsCGolfDoc(allocator)->playerCount;
    if (b <= 4) n = b;
    int count = n;
    void* cur = self;
    int i = 0;
    void* obj;
    while (i < count) {
        allocator = g_resourceManager;
        if (allocator != 0) {
            obj = (*(void**)&AsCHeadIcon(cur)->player0OverlayObject);
            if (obj != 0) {
                Object_DeleteVirtual(allocator, obj);
                (*(void**)&AsCHeadIcon(cur)->player0OverlayObject) = 0;
            }
        }
        cur = (char*)cur + kHeadIconPlayerStride;
        i++;
    }
}

#pragma opt_common_subs off
// Store a player point in viewport coordinates and relative to the camera position.
void CHeadIcon_ProjectPlayerPoint(void* self, int idx, void* pt) {
    
    float a[12]; // projected point: [0..2] = screen x, y, z (rest unused here)
    char* base;
    if (idx >= 0 && (unsigned int)idx < 4) {
        WView_ProjectWorldPoint(g_view, a, pt);
        base = (char*)self + idx * kHeadIconPlayerStride;
        WVector b = *(WVector*)pt - *(WVector*)&AsWView(g_view)->viewMatrix[9];
        (*(WVector*)&((HeadIconPlayerPointView*)base)->cameraRelativePosition) = b;
        ((HeadIconPlayerPointView*)base)->screenX = a[0];
        ((HeadIconPlayerPointView*)base)->screenY = a[1];
        ((HeadIconPlayerPointView*)base)->screenZ = a[2];
    }
    return;
}
#pragma opt_common_subs reset
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462144", _ctors_u80183098_0, CHeadIcon_InitStaticConstants);
#pragma pop
