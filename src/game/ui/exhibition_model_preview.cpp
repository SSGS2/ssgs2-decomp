// The item preview (CExhibition): a model of an avatar, caddie, ball or club set, shown in its
// own viewport with its own camera, lights, animation and attached effects.
//
// Also the model loading state, and the animation and sound commands in the preview's scripts.

#include "classes/GtlIGenericEnvironmentResource.h"
#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "MSL/error_message.h"
// Byte layout of the existing string pool; no new string storage.
struct ExhibitionStringPool {
    char text_0_pet[5];                  // ".pet"
    char text_5_ui_pet[8];               // "_ui.pet"
    char text_d_vector_length_error[20]; // "vector length error"
    char text_21_std_exception_std_logic_error_std_length_error
        [55];               // "!std::exception!!std::logic_error!!std::length_error!!"
    char text_58_center[7]; // "center"
    char text_5f_spr[4];    // "spr"
    char text_63_seq[4];    // "seq"
    char text_67_mpet[6];   // ".mpet"
};
enum {
    kExhibitionStringPool_text_0_pet = (int)&((ExhibitionStringPool*)0)->text_0_pet,
    kExhibitionStringPool_text_5_ui_pet = (int)&((ExhibitionStringPool*)0)->text_5_ui_pet,
    kExhibitionStringPool_text_d_vector_length_error = (int)&((ExhibitionStringPool*)0)->text_d_vector_length_error,
    kExhibitionStringPool_text_21_std_exception_std_logic_error_std_length_error = (int)&((ExhibitionStringPool*)0)
        ->text_21_std_exception_std_logic_error_std_length_error,
    kExhibitionStringPool_text_58_center = (int)&((ExhibitionStringPool*)0)->text_58_center,
    kExhibitionStringPool_text_5f_spr = (int)&((ExhibitionStringPool*)0)->text_5f_spr,
    kExhibitionStringPool_text_63_seq = (int)&((ExhibitionStringPool*)0)->text_63_seq,
    kExhibitionStringPool_text_67_mpet = (int)&((ExhibitionStringPool*)0)->text_67_mpet,
};

// Constants retain the original integer types and encoded values.
enum PreviewModelKind {
    kPreviewAvatar = 0,
    kPreviewCaddie = 1,
    kPreviewBall = 2,
    kPreviewClubSet = 3,
    kPreviewStaticModel = 4,
    kPreviewNoModel = 5
};
enum PreviewLoadState {
    kPreviewUninitialized = 0,
    kPreviewLoadModel = 1,
    kPreviewReloadParts = 2,
    kPreviewWaitFrames = 3,
    kPreviewReady = 4
};
// Load-state contract: 0 has no pending load; 1 replaces the whole model after
// both preloaders are ready; 2 applies avatar parts; 3 waits a frame threshold;
// 4 accepts new work. Readiness also depends on the optional file preloader.
// All camera/model operations below use the shared g_view; setters can mutate
// its projection, not just this preview's stored rectangle.
enum IffItemKind {
    kItemKindShift = 26,
    kItemClubSet = 4,
    kItemBall = 5,
    kItemCaddie = 7,
    kItemCaddieEquipment = 8
};
enum {
    kPreviewScreenWidth = 640,
    kPreviewScreenHeight = 480,
    kHardwareLightCount = 8,
    kPreviewLightLimit = 3,
    kPreviewCallbackOffset = 0x540,
    kPreviewAttachmentCount = 5,
    kPreviewResourceOptionsOffset = 0x640,
    kPreviewTokenCapacity = 64,
    kMinimumEffectFilenameLength = 5,
    kPreviewMinimumExtensionLength = 4,
    kPreviewModelKindOffset = 0x468,
    kBallEffectSlotTwelve = 12,
    kBallEffectSlotOneTwentyEight = 128
};
static const unsigned kMaxPointerElements = 0x3FFFFFFFU;
static const unsigned kPointerGrowthThreeFifthsLimit = 0x15555555U;
static const unsigned kPointerGrowthHalfLimit = 0x2AAAAAAAU;
static const unsigned kBoneSuppressAttachedEffects = 0x40000U;

#include "iff/IffStruct.h"
#include "classes/WPuppet.h"
#include "classes/CSceneManager.h"
// Layout-only views of the attachment fields written by this preview.
struct PreviewSprayAttachment {
    char effectBase[8];
    void* sourceAttachment;
    float screenX, screenY;
    void* transforms[4];
    void* positions[4];
};
struct PreviewSequenceAttachment {
    char effectBase[0x14];
    void* sourceAttachment;
    float screenX, screenY;
};
struct PreviewEffectTreeNode {
    void* left;
    void* right;
    void* taggedParent;
    void* key;
    void* effect;
};
struct PreviewEffectTreeHeaders {
    char previewState[0x208];
    char spraySentinel[0x0c];
    char sequenceSentinel[0x0c];
};
struct PreviewOwnedRecord {
    char recordState[0x80];
    void* ownedData;
};
struct PreviewBoneTransform {
    char boneState[0x78];
    float attachmentMatrix[12];
};
// Named-bone entries contain a live bone plus metadata. Both are puppet-owned.
struct PreviewBoneMetadata {
    char state[0x18];
    float position[3];
    char reserved24[4];
    char* script;
};
struct PreviewNamedBone {
    PreviewBoneTransform* bone;
    PreviewBoneMetadata* metadata;
};
enum PreviewBoneOffsets {
    kPreviewBoneMatrixOffset = 0x78,
    kPreviewBoneFlagsOffset = 0x74,
    kPreviewResolvedSelectionOffset = 0x1BC
};
// Four borrowed transform/point slots, preceded by a pixel center; FxAttachment_Init
// initializes unused slots. This view is call scratch, never an owner of the bones.
struct PreviewAttachmentDescription {
    float x, y;
    void* transforms[4];
    void* points[4];
};
typedef char PreviewAttachmentDescription_size[(sizeof(PreviewAttachmentDescription) == 0x28) ? 1 : -1];
struct PreviewAnimationName {
    char rangeState[0x18];
    char* name;
};
typedef char PreviewSpray_screen_offset[((int)&((PreviewSprayAttachment*)0)->screenX == 0x0c) ? 1 : -1];
typedef char PreviewSequence_screen_offset[((int)&((PreviewSequenceAttachment*)0)->screenX == 0x18) ? 1 : -1];
typedef char PreviewTree_effect_offset[((int)&((PreviewEffectTreeNode*)0)->effect == 0x10) ? 1 : -1];
typedef char PreviewTree_sequence_offset[((int)&((PreviewEffectTreeHeaders*)0)->sequenceSentinel == 0x214) ? 1 : -1];
struct PreviewAttachmentSlots {
    char previewState[kPreviewResourceOptionsOffset];
    void* attachments[5];
};
// WView layout: 0x80065F84; viewport setup: 0x802B436C.
#include "classes/WView.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char exhibitionResourceStrings[];
}
}

#include "sdk_protos.h"
#include "game/types.h"
#include "game/wvector.h"
#include "game/exception.h"
#include "classes/CPvsFxBox.h"
#include "classes/CExhibition.h"
#include "MSL/vector.h"
#include "classes/CFxBox.h"
#include "classes/CSetScissorRect.h"
#include "classes/CFxSpray.h"
#include "classes/CPreloadFileAsync.h"
#include "classes/CCallback.h"
typedef std::vector<void*, app::app_allocator<void*> > ExhibitionPtrVector;

#define AsCExhibition(p) ((CExhibition*)(p))

// Derive compiler-shaped byte addresses from the checked shared layout. Direct
// field-address expressions cause CodeWarrior to retain extra pointer registers
// across model calls; these offsets keep the same ABI without handwritten numbers.
enum PreviewLayoutOffset {
    kPreviewCameraMatrixOffset = (int)&((CExhibition*)0)->cameraMatrixPrefix,
    kPreviewModelTransformOffset = (int)&((CExhibition*)0)->modelTransform,
    kPreviewAvatarSelectionOffset = (int)&((CExhibition*)0)->avatarCharacterId,
    kPreviewResolvedSelectionStorageOffset = (int)&((CExhibition*)0)->resolvedCharacterId
};

#define AsCPvsFxBox(p) ((CPvsFxBox*)(p))

// Construct in caller-owned storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}

extern "C" {

// Complete preview storage passed to construction; CExhibition supplies the field view.
struct ExhibitionConstructionStorage {
    unsigned char data[0x650];
};

void CExhibition_InitializeAngleCache();
void* __ct__11CExhibitionFPif(ExhibitionConstructionStorage* self, int* rect, float cameraDistance);
void* __dt__11CExhibitionFv(void* self, int deleting);
void* dtor_802BA96C(void* self, int deleting);
void* dtor_802BA9EC(void* self, int deleting);
void* __dt__15CSetScissorRectFv(void* self, int deleting);

extern float kFloat_6_2831855;
extern float s_exhibitionFullCircleAngle;

struct PreviewAffineCopy {
    float m[12];
};
struct PreviewVectorCopy {
    float v[3];
};

extern void Matrix_SetBasisElements(float*, float, float, float, float, float, float, float, float, float);
extern void Matrix_SetIdentityAffine(void*);
extern void ItemSelection_Init(void*);
extern void ResolvedItemSelection_Clear(void*);
extern int Camera_SetFov(void*, float);
extern int WView_SetViewMatrix(void*, void*);
extern int Camera_BuildFrustumPlanes(void*);
extern int Camera_Apply(void*, float, float);
extern int Matrix_TransformPoint(void*, void*, void*);
extern int WView_ProjectViewPointWithoutClipTest(void*, void*, void*);
extern int Mtx44_SetEulerRotationZXY(void*, float, float, float);
extern int PSMTX44Concat(void*, void*, void*);
extern int Mtx44_SetTranslation(void*, float, float, float);
extern int Matrix_ExtractAffineFrom4x4(void*, void*);
extern void dtor_8013AA08(void*);
extern "C" int __dt__6CFxBoxFv(void*, int);
extern void dtor_8013AA48(void*);
extern void dtor_8013AAC8(void*);
extern void ListWithCount_Clear(void*);

extern char __vt__11CExhibition[];
extern unsigned char __vt__9CPvsFxBox[];
extern char s_cameraTransform[];
extern void* g_view;
extern void* g_resourceManager;
extern float kFloat_0_exhibition;
extern float kFloat_1_Intro;
extern float kPreviewInitialMinRotationX;
extern float kPreviewInitialMaxRotationX;
extern const float kPreviewRotationYLowerBound;
extern const float kPreviewRotationYUpperBound;
extern float kPreviewInitialMinCameraDistance;
extern float kPreviewInitialMaxCameraDistance;
extern float kExhibitionCameraFovRadians;
extern const float kFloat_neg1_Exhibition;
extern float kFloat_0_5_exhibition;
extern float kFloat_33_333_PreviewEffect;

int ParseConditionedQuotedToken(void* self, int arg2, int arg3, void* arg4, int arg5);

int FxAttachment_ParseBoneReferences(void* self, int arg2, int arg3, void* arg4, void* arg5, int arg6, int arg7);

int FxBox_CreateAndAttachEffectByExtension(void* self, void* output, int arg3, void* arg4);

int Parse_NonemptyQuotedTokenUnder64Bytes(void* self, void* arg2, void* result);
extern void* g_soundManager;
extern const float kFloat_200_PreviewSound;
int Sound_PlayByName(void* arg1, void* arg2, int arg4, int arg6, int arg8, int arg9, float arg3, float arg5,
                     float arg7);

struct Sub802B8AD8 {
    virtual void m0();
    virtual int m3(int, void*);
    virtual int m4(int);
};

struct Sub802B87F4 {
    virtual void m0();
    virtual void m1();
    virtual int m2();
    virtual void m3();
    virtual int m4();
};

int AvatarModel_SetAttachment(void*, unsigned int,
                              void*); /* defined (void*, unsigned char, void*); index passed through unmasked */
extern int PlayerModel_GetPuppet(void*);
extern int ObjectList_GetSelected(void*);
int WPuppet_ParseMaterialLayerAnimationCommand(void*, int, void*);

// Accept model changes when the file preloader is ready and the normal load state is initial (0) or ready (4).
static inline int ExhibitionIsReady(void* self) {
    void* ptr = (*(void**)&AsCExhibition(self)->preloadFile);
    int valid = 1;
    if (ptr && !((Sub802B87F4*)ptr)->m2())
        valid = 0;
    if (valid) {
        unsigned int value = (*(unsigned int*)&AsCExhibition(self)->loadState) - 1;
        unsigned int shift = __cntlzw(value ^ 2);
        return (value << shift) >> 31;
    }
    return 0;
}

// Resolve the drawable puppet from an avatar controller or a directly loaded preview model.
static inline void* fn_802BA4AC_a(void* self) {
    if (AsCExhibition(self)->modelKind == kPreviewAvatar) {
        void* ptr = AsCExhibition(self)->modelController;
        if (ptr != 0)
            return (void*)PlayerModel_GetPuppet(ptr);
    } else {
        return AsCExhibition(self)->puppet;
    }
    return 0;
}

// Resolve the selected avatar object or directly loaded preview model.
static inline void* fn_802BA4AC_b(void* self) {
    if (AsCExhibition(self)->modelKind == kPreviewAvatar) {
        void* ptr = AsCExhibition(self)->modelController;
        if (ptr != 0)
            return (void*)ObjectList_GetSelected(ptr);
    } else {
        return AsCExhibition(self)->puppet;
    }
    return 0;
}

extern char __vt__15CSetScissorRect[];
extern "C" int Engine_SetScissorRect(void*);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern "C" int AvatarModel_ApplyTextureVariant(void*, void*,
                                               int); /* defined with an unsigned char mode; passed through unmasked */
extern "C" void ItemSelection_SetStateIfSameCharacter(void*, void*, int);
extern "C" void* String_ParseQuotedToken(void*, void*);
extern "C" int CString_Compare(void*, void*);
extern "C" char* strchr(const char*, int);
extern "C" char* strcat(char*, const char*);
extern "C" int ResourceModel_HasPartModelNamed(void*, void*);
extern "C" int WPuppet_SetBoneVisibilityRecursive(void*, int, int, void*);
extern char exhibitionResourceStrings[];

struct ResolvedAvatarSelectionCopy {
    unsigned int value;
    unsigned char bytes[4];
    unsigned int words[48];
};

// Return the avatar controller's puppet, or null when there is no controller.
inline void* fn_802BA578_inner(void* self) {
    void* ptr = AsCExhibition(self)->modelController;
    if (ptr != 0)
        return (void*)PlayerModel_GetPuppet(ptr);
    return 0;
}

// Resolve the puppet used for animation script events.
inline void* fn_802BA578_get(void* self) {
    if (AsCExhibition(self)->modelKind == kPreviewAvatar) {
        void* ptr = AsCExhibition(self)->modelController;
        if (ptr != 0)
            return (void*)PlayerModel_GetPuppet(ptr);
    } else {
        return AsCExhibition(self)->puppet;
    }
    return 0;
}

// Copy records include the character id, four packed bytes, and opaque part words.
struct ResolvedAvatarRecordCopy {
    int head;
    unsigned char b[4];
    int rest[48];
};
struct AvatarSelectionRecordCopy {
    int head;
    unsigned char b[4];
    int rest[24];
};

int ResourceModel_SetMatrixRecursive(void*, void*, void*);
int ResourceModel_UpdateBoneWorldMatrices(void*, int, float, int);
int Model_SetMatrixRecursive(void*, void*, void*);
extern int WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
int WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
int Matrix_SetAxisRotation(void*, float, int);
int Matrix_ConcatAffine(void*, void*, void*);
extern const float kClubPreviewAxisRotation;

extern const float kPreviewRotationYWrapStep;

int ObjectList_FindAnimationRangeByName(void*, void*, int, int);
int PlayerModel_SetDefaultPuppetLayerEnabled(void*, int);
int WPuppet_FindAnimationRangeByName(void*, void*);
extern int AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int);

struct Owner6888 {
    char pad[0x68];
    float current;
    void* animation;
    void* controller;
    void* object;
    char pad78[0x3f0];
    int kind;
};
// Resolve the animation-event model from the selected avatar object or direct puppet.
static inline void* target6888(Owner6888* owner) {
    switch (owner->kind) {
    case 0:
        if (owner->controller)
            return (void*)ObjectList_GetSelected(owner->controller);
        break;
    default:
        return owner->object;
    }
    return 0;
}

extern void* s_previewEnvironmentResourceName;
extern int Resource_OpenResolvedPath(void*, void*, unsigned int, int, int, int, int);
extern int __nwa__FUl(unsigned int);
extern void gtl_IGenericEnvironmentResource_CopyLights(void*, void*, unsigned int);
extern void gtl_GenericLight_GetDescription(void*, void*);
extern int appMallocDummy__FUl(unsigned int);
extern void Resource_Release(void*);
extern void __dla__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
struct VecAllocation802B {
    char* pointer;
    char* allocator;
    // Guard message-array storage until the shared owner is constructed.
    VecAllocation802B(char* p, char* a) : pointer(p), allocator(a) {}
    // Delete the array unless ownership was transferred by clearing pointer.
    ~VecAllocation802B() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct VecControlBase802B {
    void* table;
    unsigned strong, weak;
    // Initialize shared message ownership with one strong and one weak reference.
    VecControlBase802B() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct VecControl802B : VecControlBase802B {
    char* pointer;
    // Store the message array in its shared control block.
    VecControl802B(char* p) {
        pointer = p;
    }
};
struct VecErrorBase802B : std::exception {
    // Install the exception runtime table at the base object's first word.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    // Initialize the logic-error portion of a vector length exception.
    VecErrorBase802B() {
        SetTable(__vt__Q23std11logic_error);
    }
};
} // extern "C"
namespace std {
struct length_error : VecErrorBase802B {
    ::std::__error_message<VecAllocation802B, VecControl802B> message_;
    // Duplicate the supplied error message and install the length-error table.
    length_error(const char* m) {
        const char* message = m;
        message_.assign(message);
        SetTable(__vt__Q23std12length_error);
    }

    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {
struct VecBuffer802B {
    void** data;
    unsigned size;
    unsigned capacity;
    unsigned* owner;
    unsigned offset;
    // Begin an empty temporary pointer-array owner for vector growth.
    VecBuffer802B(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free pointer storage only; the caller manages pointed-to objects separately.
    ~VecBuffer802B() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
struct PtrVec802B {
    void** data;
    unsigned size;
    unsigned capacity;
};
struct RefObj802B {
    int ref;
    virtual ~RefObj802B();
};
struct File802B {
    virtual void v0();
    virtual void v1();
    virtual void Read(void*, unsigned int);
    int size;
};
// Count pointer elements between two vector iterators.
inline int VecDist802B(void** first, void** last) {
    return last - first;
}
// Choose the larger growth amount by reference, matching the original allocator expression.
inline const unsigned& VecMax802B(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}

int WPuppet_UpdateBound(void*, int);
float Random_RangeFloat(int, float, float);
extern int WPuppet_SetOpacityRecursive(void*, int);

extern CSharedDoc* g_sharedDoc;
int ItemInventory_FindBallData(void*, int);
int FxAttachment_Init(void*);
int WPuppet_FindOrCacheBone(void*, const char*);
int WPuppet_FindNamedBoneEntry(void*, const char*);
extern int CFxSequence_Create(void*, const char*, int, int, int);

int String_CompareCString(const char*, const char*);
char* String_ParseSpaceDelimitedToken(void*, void*);
int ResolvedItemSelection_ContainsPart(void*, const char*);
int AvatarModel_ContainsAttachment(void*, const char*);
extern const unsigned char kPreviewBoneEffectMarkerByte0;
extern unsigned char kPreviewBoneEffectMarkerByte1;
extern unsigned char kPreviewBoneEffectMarkerByte2;
extern unsigned char kPreviewBoneEffectMarkerByte3;
int String_ParseDecimalInt(const char*);
void __dt__Q23std12length_errorFv(void*, int);

// Construct a length_error in caller-provided aligned storage.
inline std::length_error* MakeError802B(void* storage, const char* message) {
    return new (storage) std::length_error(message);
}

int PuppetOwner_SetOpacityRecursive(void*, unsigned char);
extern const float kPreviewMaximumOpacity;
extern const float kPreviewFadeAndEffectStepScale;

struct Sub802B9CD4 {
    virtual void m0();
    virtual void m1(float);
};

// Resolve the model receiving animation-frame events.
static inline void* fn_802B9CD4_target(char* base) {
    if (AsCExhibition(base)->modelKind == kPreviewAvatar) {
        void* controller = AsCExhibition(base)->modelController;
        if (controller)
            return (void*)ObjectList_GetSelected(controller);
    } else
        return AsCExhibition(base)->puppet;
    return 0;
}

extern void* s_previewFallbackAnimationName;
int ItemSelection_Equals(void*, void*);
int String_ReplaceRange(void*, int, int, const char*, const char*, const char*);

void WView_InvalidateViewAndProjection(void*);
void Camera_SetClip(void*, float, float, int);
void Matrix_CopyAffine(void*, void*);
void WView_SetScreenCenter(void*, float, float);
void Engine_GetRenderState(int, int*);
void Engine_SetCachedRenderState(int, int);
void Engine_GetScissorRect(void*);
void WPuppet_SetRendererOption(void*, int);
void Engine_SetLightEnabled(int, int);
void Engine_SetLight(int, int);
extern float s_previewNearClip;
extern float s_previewFarClip;
extern float kPreviewReferenceScreenCenterX;
extern float kPreviewReferenceScreenCenterY;
extern const float kPreviewScissorOffsetScale;
extern void* g_sceneManager;
extern char __vt__15CSetRenderState[];

struct ViewObj6A1C {
    virtual void m0();
    virtual void m1();
    virtual void m2(int);
    virtual void m3();
    virtual void m4(void*, int, int, float, int, int);
};
struct CamObj6A1C {
    virtual void m0();
    virtual void m1();
    virtual void m2(int);
};
// Resolve the render interface for the active preview model.
static inline ViewObj6A1C* ExhibitionView(void* self) {
    char* b = (char*)self;
    if (AsCExhibition(b)->modelKind == kPreviewAvatar) {
        if (AsCExhibition(b)->modelController)
            return (ViewObj6A1C*)PlayerModel_GetPuppet(AsCExhibition(b)->modelController);
    } else {
        return (*(ViewObj6A1C**)&AsCExhibition(b)->puppet);
    }
    return 0;
}
// IDs verified against Engine_SetCachedRenderState (gx_renderer_state_and_efb_copy.cpp).
// Values follow its GX depth/blend/cull consumers; guards restore state in reverse order.
enum PreviewRenderState {
    kPreviewCullMode = 0,
    kPreviewLightingEnabled = 2,
    kPreviewDepthCompare = 3,
    kPreviewDepthWrite = 5,
    kPreviewDepthTest = 6,
    kPreviewBlendEnabled = 10,
    kPreviewSourceBlend = 13,
    kPreviewDestinationBlend = 14
};
enum PreviewRenderValue {
    kPreviewDisabled = 0,
    kPreviewEnabled = 1,
    kPreviewCompareLessEqual = 3,
    kPreviewSourceAlpha = 4,
    kPreviewInverseSourceAlpha = 5,
    kPreviewCullNone = 0
};
struct StateGuard6A1C {
    void* vt;
    int id;
    int value;
    // Save one cached render state and install its preview-specific replacement.
    StateGuard6A1C(int i, int state) {
        vt = __vt__15CSetRenderState;
        id = i;
        value = -1;
        Engine_GetRenderState(i, &value);
        Engine_SetCachedRenderState(id, state);
    }
    // Restore the saved cached render-state value on scope exit.
    ~StateGuard6A1C() {
        vt = __vt__15CSetRenderState;
        Engine_SetCachedRenderState(id, value);
    }
};
struct RectGuard6A1C {
    void* vt;
    int r[4];
    // Save the engine scissor rectangle before preview clipping.
    RectGuard6A1C() {
        vt = __vt__15CSetScissorRect;
        Engine_GetScissorRect(&r);
    }
    // Restore the saved scissor rectangle on scope exit.
    ~RectGuard6A1C() {
        vt = __vt__15CSetScissorRect;
        Engine_SetScissorRect(&r);
    }
};
typedef Vec3f Vec6A1C;
struct PreviewCameraBasis {
    Vec6A1C basisX, basisY, basisZ, origin;
};
struct PreviewAffineWordCopy {
    unsigned int w[12];
};
typedef unsigned int (*Cvt6A1C)(double);

struct Del802B734C {
    virtual void m0(int);
};
// Flip the camera X/Z basis vectors and install the resulting view matrix.
static inline void setCam734C_c(PreviewCameraBasis m) {
    *(WVector*)&m *= kFloat_neg1_Exhibition;
    *(WVector*)(&m.basisZ) *= kFloat_neg1_Exhibition;
    WView_SetViewMatrix(g_view, &m);
}
// Forward a camera matrix while retaining the original inline-copy shape.
static inline void setCam734C_b(PreviewCameraBasis m) {
    setCam734C_c(m);
}
// Pass the camera matrix through the original copy wrappers before applying it.
static inline void setCam734C_a(PreviewCameraBasis m) {
    setCam734C_b(m);
}
// Two padded allocator bytes; volatile copies retain the engine string ABI.
struct PreviewStringAllocatorSlots {
    char a;
    char x[3];
    char b;
    char y[3];
};
struct AvatarSelectionPartCopy {
    unsigned char a, b, c, d;
    int rest[24];
};

extern const char* s_previewCaddieAnimationName;
extern const char* s_previewClubSetAnimationName;

typedef Vec3f PreviewVector3;
// Affine camera storage: three basis vectors followed by camera origin.
// basisX/basisZ are inverted before installing WView's opposite-handed view convention.
struct PreviewCameraMatrix {
    PreviewVector3 basisX, basisY, basisZ, origin;
};
struct PreviewCameraCopyOrder {
    PreviewVector3 basisX, origin, basisZ, basisY;
};
// Build the original reordered camera-copy temporary without changing its component values.
inline PreviewCameraCopyOrder MakeCameraCopyInAbiOrder(const PreviewVector3& a, const PreviewCameraMatrix& c) {
    PreviewCameraCopyOrder s = {a, c.origin, c.basisZ, c.basisY};
    return s;
}

struct Del77E8 {
    virtual ~Del77E8();
};
IFF_STRUCT::sCadItem* ItemInventory_FindCaddieItemData(void*, void*);
IFF_STRUCT::sCaddie* ItemInventory_FindCaddieData(void*, unsigned int);
IFF_STRUCT::sClubSet* ItemInventory_FindClubSetData(void*, unsigned int);

// Replace the complete small/heap string with borrowed [start,end) text.
// Keep this scope at the call site: the inline-function form changes .text.
#define REPLACE_PREVIEW_STRING_RANGE(field, start, end)                                                     \
    {                                                                                                       \
        int length = (*(unsigned int*)(base + field) >> 31) == 0 ? (*(unsigned char*)(base + field) & 0x7f) \
                                                                 : *(int*)(base + field + 4);               \
        volatile PreviewStringAllocatorSlots sl;                                                            \
        sl.b = sl.a;                                                                                        \
        String_ReplaceRange(base + field, 0, length, start, end, (const char*)&sl.b);                       \
    }
// Read the string length from small-string storage or its heap-backed representation.
inline int PreviewStringLength(char* p) {
    return (*(unsigned int*)p >> 31) == 0 ? (*(unsigned char*)p & 0x7f) : *(int*)(p + 4);
}
// Assign a NUL-terminated animation name. Both string macros below use the
// caller's base/s locals; inline alternatives changed the generated instructions.
#define ASSIGN_PREVIEW_ANIMATION(field, init)                                               \
    {                                                                                       \
        s = init;                                                                           \
        int length = PreviewStringLength(base + field);                                     \
        volatile PreviewStringAllocatorSlots sl;                                            \
        sl.b = sl.a;                                                                        \
        String_ReplaceRange(base + field, 0, length, s, s + strlen(s), (const char*)&sl.b); \
    }
// Append the borrowed suffix to the current model path; copy text into the string.
#define APPEND_PREVIEW_MODEL_SUFFIX(field, init)                                            \
    {                                                                                       \
        s = init;                                                                           \
        int length = PreviewStringLength(base + field);                                     \
        volatile PreviewStringAllocatorSlots sl;                                            \
        sl.a = sl.b;                                                                        \
        String_ReplaceRange(base + field, length, 0, s, s + strlen(s), (const char*)&sl.a); \
    }

typedef unsigned int (*Round4BB0)(float);

struct Box4BB0 {
    float hw, hh, cx, cy;
};
struct PreviewCameraWordCopy {
    unsigned w[12];
};

typedef Vec3f V825C;
struct DistanceCameraMatrix {
    V825C basisX, basisY, basisZ, origin;
};
struct DistanceCameraCopyOrder {
    V825C basisX, origin, basisZ, basisY;
};
// Build the camera-copy temporary used by distance changes.
inline DistanceCameraCopyOrder MakeDistanceCameraCopyInAbiOrder(const V825C& a, const DistanceCameraMatrix& c) {
    DistanceCameraCopyOrder s = {a, c.origin, c.basisZ, c.basisY};
    return s;
}

struct Rect5048 {
    int x0, y0, x1, y1;
};
// Build the viewport rectangle temporary in the original member order.
inline Rect5048 makeRect5048(int x0, int y0, int x1, int y1) {
    Rect5048 r = {x0, y0, x1, y1};
    return r;
}
struct Box5048 {
    float cx, cy, hw, hh;
};

struct Rect5504 {
    int x0, y0, x1, y1;
};
// Build the rectangle temporary used by vertical viewport-limit changes.
inline Rect5504 makeRect5504(int x0, int y0, int x1, int y1) {
    Rect5504 r = {x0, y0, x1, y1};
    return r;
}
struct Box5504 {
    float cx, cy, hw, hh;
};
typedef Vec3f V3_5504;
struct Cam5504 {
    V3_5504 basisX, basisY, basisZ, origin;
};
struct CamSw5504 {
    V3_5504 basisX, origin, basisZ, basisY;
};
// Reorder the camera-copy temporary for the original inline copy sequence.
inline CamSw5504 toSw5504(const Cam5504& c) {
    CamSw5504 s = {c.basisX, c.origin, c.basisZ, c.basisY};
    return s;
}
// Restore normal camera member order from the reordered temporary.
inline Cam5504 fromSw5504(const CamSw5504& s) {
    Cam5504 c = {s.basisX, s.basisY, s.basisZ, s.origin};
    return c;
}
typedef unsigned int (*Round5504)(float);

extern int PlayerModel_Dtor(void*, int);
extern int ResourceManager_ReleaseObject(void*, void*, int);
void PlayerModel_ClearState(void*);
int AvatarModel_LoadCharacterResources(void*, void*, void*, int, int, int, int, int);
int PlayerModel_DefaultReady(void*, void*);
int ResourceManager_LoadByExtension(void*, void*, int, int, int);
int WPuppet_RebuildBoneController(void*);
int WPuppet_InitializeTextureCoordinateAnimation(void*);
int ItemInventory_GetBallIdAt(void*, int);
int AvatarModel_CopyAppearanceRecord(void*, void*);
int AvatarModel_ApplyItemChanges(void*, void*, int);
int PuppetResourceList_PollLoadAndUpdateBounds(void*);
extern int s_previewLoadWaitFrameLimit;

struct AllocatedController {
    char storage[0x2a0];
    // Initialize the newly allocated preview avatar controller's state.
    AllocatedController() {
        PlayerModel_ClearState(this);
    }
};

struct Controller6388 {
    virtual void m2();
    virtual void update();
    virtual int ready();
};
// Resolved part bytes exclude the leading character id; contents remain opaque.
struct ResolvedAvatarPartCopy {
    unsigned char a, b, c, d;
    int rest[48];
};
// Controller subview borrowed only while copying the resolved selection.
struct PreviewControllerSelectionView {
    char prefix[0x1BC];
    unsigned int characterId;
    ResolvedAvatarPartCopy parts;
};
struct PreviewOwnedStringView {
    unsigned int flags, length;
    void* heapData;
};
struct PreviewOwnedNameVectorView {
    void* vtable;
    void** data;
    unsigned int size, capacity;
};
// Keep raw aliasing at vector operations while deriving offsets from this schema.
enum PreviewNameVectorOffset {
    kPreviewNamesData = (int)&((PreviewOwnedNameVectorView*)0)->data,
    kPreviewNamesSize = (int)&((PreviewOwnedNameVectorView*)0)->size
};
// Treat an absent preloader as ready; otherwise ask its readiness interface.
inline int Ready6388(Controller6388* p) {
    int valid = 1;
    if (p && !p->ready())
        valid = 0;
    return valid;
}
// Subtract an already processed count using the original reference expression.
inline void Reduce6388(unsigned& size, unsigned count) {
    size -= count;
}
// Compare iterator addresses as unsigned words, preserving the original loop form.
inline bool Unequal6388(void* end, void* iter) {
    return (unsigned)end != (unsigned)iter;
}
// Test whether the preview has waited longer than its configured frame limit.
inline bool PreviewWaitLimitExceeded(int limit, char* base) {
    return limit < AsCExhibition(base)->loadWaitFrames;
}
// Release the previous model and owned name strings before a replacement load.
inline void Clear6388(char* base) {
    if (AsCExhibition(base)->modelKind == kPreviewAvatar) {
        void* p = AsCExhibition(base)->modelController;
        if (p) {
            PlayerModel_Dtor(p, 1);
            AsCExhibition(base)->modelController = 0;
        }
        AsCExhibition(base)->puppet = 0;
    } else if (g_resourceManager && AsCExhibition(base)->puppet) {
        ResourceManager_ReleaseObject(g_resourceManager, AsCExhibition(base)->puppet, 0);
        AsCExhibition(base)->puppet = 0;
    }
    void** entries = *(void***)(base + 4);
    for (; Unequal6388(*(void***)(base + 4) + *(unsigned*)(base + 8), entries); ++entries) {
        void* entry = *entries;
        if (entry) {
            if (entry) {
                if (*(unsigned*)entry >> 31)
                    appFreeUni__FPv(*(void**)((char*)entry + 8));
                __dl__FPv(entry);
            }
            *entries = 0;
        }
    }
    Reduce6388(*(unsigned*)(base + 8), *(unsigned*)(base + 8));
}
// Wait for both preloaders, apply resolved avatar parts, and enter the ready state.
inline void StateTwo6388(char* base) {
    int ready = 1;
    Controller6388* p = (*(Controller6388**)&AsCExhibition(base)->modelPreloader);
    if (p && !p->ready())
        ready = 0;
    if (ready) {
        ready = 1;
        p = (*(Controller6388**)&AsCExhibition(base)->preloadFile);
        if (p && !p->ready())
            ready = 0;
        if (ready) {
            if (AsCExhibition(base)->modelController) {
                AvatarModel_CopyAppearanceRecord(AsCExhibition(base)->modelController,
                                                 ((char*)(&AsCExhibition(base)->resolvedCharacterId)));
                AvatarModel_ApplyItemChanges(AsCExhibition(base)->modelController,
                                             ((char*)(base + kPreviewAvatarSelectionOffset)),
                                             AsCExhibition(base)->partReloadMask);
                ((CExhibition*)base)->RebuildAttachedEffects();
                ((CExhibition*)base)->UpdateModelPose();
            }
            AsCExhibition(base)->loadState = kPreviewReady;
            AsCExhibition(base)->loadWaitFrames = 0;
        }
    }
}

extern "C" void ResolvedItemSelection_Destroy(void*, int);
extern "C" void dtor_80293270(void*, int);
extern "C" int FxBoxResourceTree_FreeSubtreeNodes(void*, void*);
// Subtract the cleared element count without changing vector capacity.
inline void ReduceCount5A88(unsigned& size, unsigned count) {
    size -= count;
}
// Reload a pointer-vector base and count to compute its current end iterator.
inline void** ArrayEnd5A88(void*** data, unsigned* size) {
    return *data + *size;
}
// Compare container iterator addresses in the original unsigned-word form.
inline bool Unequal5A88(void* end, void* iter) {
    return (unsigned)end != (unsigned)iter;
}
// Detach and free the preview's circular list nodes, reducing its stored count.
inline void ClearList5A88(char* member) {
    void* node = *(void**)(member + 8);
    void* head = member + 4;
    if (node != head) {
        void* a = *(void**)head;
        void* b = *(void**)node;
        *(void**)((char*)b + 4) = *(void**)((char*)a + 4);
        void* c = *(void**)((char*)a + 4);
        *(void**)c = *(void**)node;
        while (node != head) {
            void* cur = node;
            node = *(void**)((char*)node + 4);
            appFreeUni__FPv(cur);
            --*(unsigned*)member;
        }
    }
}

// These expanded cleanup scopes preserve register allocation; inline helper alternatives change .text.
#define CLEAN_TREE_LEAF(owner, value)                             \
    {                                                             \
        void* leaf = (value);                                     \
        if (leaf) {                                               \
            void* left = *(void**)leaf;                           \
            if (left)                                             \
                FxBoxResourceTree_FreeSubtreeNodes(owner, left);  \
            void* right = *(void**)((char*)leaf + 4);             \
            if (right)                                            \
                FxBoxResourceTree_FreeSubtreeNodes(owner, right); \
            appFreeUni__FPv(leaf);                                \
        }                                                         \
    }
#define CLEAN_TREE_PAIR(owner, value)                           \
    {                                                           \
        void* pair = (value);                                   \
        if (pair) {                                             \
            CLEAN_TREE_LEAF(owner, *(void**)pair);              \
            CLEAN_TREE_LEAF(owner, *(void**)((char*)pair + 4)); \
            appFreeUni__FPv(pair);                              \
        }                                                       \
    }
#define CLEAN_TREE_BRANCH(owner, value)                           \
    {                                                             \
        void* branch = (value);                                   \
        if (branch) {                                             \
            CLEAN_TREE_PAIR(owner, *(void**)branch);              \
            CLEAN_TREE_PAIR(owner, *(void**)((char*)branch + 4)); \
            appFreeUni__FPv(branch);                              \
        }                                                         \
    }
#define CLEAN_TREE(owner)                                                             \
    if (owner)                                                                        \
        if (owner)                                                                    \
            if (owner)                                                                \
                if (owner)                                                            \
                    if (owner)                                                        \
                        if (owner) {                                                  \
                            void* root = *(void**)((char*)(owner) + 4);               \
                            if (root) {                                               \
                                CLEAN_TREE_BRANCH(owner, *(void**)root);              \
                                CLEAN_TREE_BRANCH(owner, *(void**)((char*)root + 4)); \
                                appFreeUni__FPv(root);                                \
                            }                                                         \
                        }
#pragma push
#pragma always_inline on
// Release pointer-array storage; pointed-to objects have separate destruction paths.
static inline void DestroyPreviewPointerArray(void* member) {
    ((ExhibitionPtrVector*)member)->~ExhibitionPtrVector();
}
#pragma pop

// Initialize the exhibition unit's cached full-circle angle.
void CExhibition_InitializeAngleCache() {
    s_exhibitionFullCircleAngle = kFloat_6_2831855;
}

// Alignment equation used by construction and viewport/distance setters:
//   C = viewHeightPixels/2, P = projected pixel Y of the (1,1,0) probe,
//   S = C-P = pixels of upward movement per model Y unit on the Z=0 plane.
// Screen Y increases downward, model Y upward: screenY = C - translationY*S.
// Solving screenY = viewportBottom gives translationY = -(viewportBottom-C)/S.
// This aligns the model's reference origin, not its bounding-box minimum. Change
// the bottom edge to move the anchor; change distance/FOV to change S. Keep the
// probe at Z=0 when calibrating this plane. S must be nonzero: no guard is present.
// The X/Z basis negation converts the camera convention before WView projection;
// the copy-order structs preserve ABI copy scheduling, not an extra axis swap.
// Do not fold repeated constructor stages: UpdateModelPose can call model code
// between projections, and both the shared view and preview state are observable.

#pragma push
#pragma always_inline on
// Compose the preview rotation and translation, publish its affine matrix and update the model.
// Caller owns four disjoint 4x4 scratch matrices. Club previews compose yaw and X
// rotation separately; other kinds use the ZXY Euler call including rotationZ.
static inline void BuildAndApplyPreviewPose(void* self, void* clubRotation, void* modelRotation,
                                            void* translationMatrix, void* modelTransform) {
    if (AsCExhibition(self)->modelKind == kPreviewClubSet) {
        Mtx44_SetEulerRotationZXY(clubRotation, AsCExhibition(self)->rotationY, kFloat_0_exhibition,
                                  kFloat_0_exhibition);
        Mtx44_SetEulerRotationZXY(modelRotation, kFloat_0_exhibition, AsCExhibition(self)->rotationX,
                                  kFloat_0_exhibition);
        PSMTX44Concat(modelRotation, clubRotation, modelRotation);
    } else {
        Mtx44_SetEulerRotationZXY(modelRotation, AsCExhibition(self)->rotationY, AsCExhibition(self)->rotationX,
                                  AsCExhibition(self)->rotationZ);
    }
    Mtx44_SetTranslation(translationMatrix, AsCExhibition(self)->translationX, AsCExhibition(self)->translationY,
                         kFloat_0_exhibition);
    PSMTX44Concat(modelTransform, modelRotation, translationMatrix);
    Matrix_ExtractAffineFrom4x4(modelTransform, AsCExhibition(self)->modelTransform);
    ((CExhibition*)self)->UpdateModelPose();
}
// Solve the documented pixel-bottom alignment equation for model Y translation.
// Probe Y is in pixels; height is the shared view height in pixels; result is model units.
static inline float PreviewBottomTranslation(int bottomPixel, float viewHeightPixels, const float& probePixelY) {
    return -((float)bottomPixel - viewHeightPixels * kFloat_0_5_exhibition) /
           (viewHeightPixels * kFloat_0_5_exhibition - probePixelY);
}
#pragma pop

#pragma push
#pragma always_inline on
// Construct the embedded preview effect box and its two empty list sentinels.
static inline void InitializePreviewEffectBox(unsigned char* effectBoxBytes) {
    ((CFxBox*)effectBoxBytes)->__ct();
    *(void**)effectBoxBytes = __vt__9CPvsFxBox;
    *(int*)(effectBoxBytes + 0x37c) = 0;
    *(int*)(effectBoxBytes + 0x380) = 0;
    *(void**)(effectBoxBytes + 0x384) = effectBoxBytes + 0x380;
    *(int*)(effectBoxBytes + 0x388) = 0;
    *(int*)(effectBoxBytes + 0x38c) = 0;
    *(void**)(effectBoxBytes + 0x390) = effectBoxBytes + 0x38c;
}
#pragma pop

// Initialize preview ownership, embedded effects/selection, and identity model bases.
static inline void InitializePreviewOwnership(unsigned char* previewBytes, float cameraDistance) {
    AsCExhibition(previewBytes)->ownedNameStrings = 0;
    AsCExhibition(previewBytes)->cameraOffsetZ = cameraDistance;
    *(void**)previewBytes = __vt__11CExhibition;
    (*(int*)&AsCExhibition(previewBytes)->ownedNameStringCount) = 0;
    AsCExhibition(previewBytes)->ownedNameStringCapacity = 0;
    AsCExhibition(previewBytes)->unk_10 = 0;
    AsCExhibition(previewBytes)->rotationX = 0.0f;
    AsCExhibition(previewBytes)->rotationY = 0.0f;
    AsCExhibition(previewBytes)->rotationZ = 0.0f;
    AsCExhibition(previewBytes)->animationDelay = 0.0f;
    AsCExhibition(previewBytes)->animationCycleDuration = 0.0f;
    AsCExhibition(previewBytes)->opacity = 0.0f;
    AsCExhibition(previewBytes)->screenCenterX = 0.0f;
    AsCExhibition(previewBytes)->screenCenterY = 0.0f;

    Matrix_SetBasisElements((float*)(previewBytes + kPreviewCameraMatrixOffset), 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 1.0f);
    AsCExhibition(previewBytes)->animationFrame = 0.0f;
    AsCExhibition(previewBytes)->animationRange = 0;
    AsCExhibition(previewBytes)->modelController = 0;
    AsCExhibition(previewBytes)->puppet = 0;
    Matrix_SetBasisElements((float*)(previewBytes + kPreviewModelTransformOffset), 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                            0.0f, 0.0f, 1.0f);

    AsCExhibition(previewBytes)->unk_A8 = 15.0f;
    AsCExhibition(previewBytes)->unk_AC = 25.0f;
    unsigned char* effectBoxBytes = previewBytes + 0xb0;
    InitializePreviewEffectBox(effectBoxBytes);
    AsCExhibition(previewBytes)->attachedSprays = 0;
    AsCExhibition(previewBytes)->attachedSprayCount = 0;
    AsCExhibition(previewBytes)->attachedSprayCapacity = 0;
    AsCExhibition(previewBytes)->attachedSequences = 0;
    AsCExhibition(previewBytes)->attachedSequenceCount = 0;
    AsCExhibition(previewBytes)->attachedSequenceCapacity = 0;
    AsCExhibition(previewBytes)->ownedPreviewListCount = 0;
    AsCExhibition(previewBytes)->ownedPreviewListSentinelPrev = previewBytes + 0x460;
    AsCExhibition(previewBytes)->ownedPreviewListSentinelNext = previewBytes + 0x460;
    AsCExhibition(previewBytes)->modelKind = kPreviewNoModel;
    AsCExhibition(previewBytes)->itemTypeId = 0;
    AsCExhibition(previewBytes)->avatarResourceOptions = 0;
    ItemSelection_Init(previewBytes + kPreviewAvatarSelectionOffset);
    AsCExhibition(previewBytes)->partLoadPending = 0;
}

// Initialize playback strings, resource handles, callbacks and the resolved selection.
static inline void InitializePreviewPlaybackAndResources(unsigned char* previewBytes) {
    AsCExhibition(previewBytes)->translationX = 0.0f;
    AsCExhibition(previewBytes)->translationY = 0.0f;
    AsCExhibition(previewBytes)->loadState = kPreviewUninitialized;
    (*(int*)&AsCExhibition(previewBytes)->animationNameFlags) = 0;
    AsCExhibition(previewBytes)->animationNameLength = 0;
    AsCExhibition(previewBytes)->animationNameData = 0;
    AsCExhibition(previewBytes)->maxAnimationDelay = 0.0f;
    AsCExhibition(previewBytes)->textureVariantIndex = 0;
    AsCExhibition(previewBytes)->modelPreloader = 0;
    AsCExhibition(previewBytes)->animationMode = 0;
    (*(int*)&AsCExhibition(previewBytes)->modelPathFlags) = 0;
    AsCExhibition(previewBytes)->modelPathLength = 0;
    AsCExhibition(previewBytes)->modelPathData = 0;
    AsCExhibition(previewBytes)->partReloadMask = 0;
    AsCExhibition(previewBytes)->lightingResource = 0;
    AsCExhibition(previewBytes)->preloadFile = 0;
    AsCExhibition(previewBytes)->lights = 0;
    (*(int*)&AsCExhibition(previewBytes)->lightCount) = 0;
    AsCExhibition(previewBytes)->lightCapacity = 0;
    AsCExhibition(previewBytes)->displaySuppressed = 0;
    ((CCallback*)(previewBytes + kPreviewCallbackOffset))->__ct();
    AsCExhibition(previewBytes)->loadWaitFrames = 0;
    ResolvedItemSelection_Clear(previewBytes + kPreviewResolvedSelectionStorageOffset);
}

// Initialize the item preview, camera, effects, model selection, and viewport limits.
CExhibition::CExhibition(int* viewportRect, float cameraDistance) {
    ExhibitionConstructionStorage* self = (ExhibitionConstructionStorage*)this;
    // viewportRect borrows four pixel edges. The preview owns its controller,
    // effect box, name records, light descriptions and file preloader; a direct
    // puppet is a resource-manager handle released through that manager.
    unsigned char* previewBytes = (unsigned char*)self;
    // Repeated matrix copies/calculations retain CodeWarrior scheduling; stack declaration order is intentional.
    float firstProjectedProbe[14];       // 0x4F8
    float firstCameraMatrix[12];         // 0x4C8
    float firstFlippedCameraMatrix[12];  // 0x498
    float firstModelTransform[16];       // 0x458
    float firstTranslation[16];          // 0x418
    float firstClubRotation[16];         // 0x3D8
    float firstModelRotation[16];        // 0x398
    float secondModelRotation[16];       // 0x358
    float secondClubRotation[16];        // 0x318
    float secondTranslation[16];         // 0x2D8
    float secondModelTransform[16];      // 0x298
    float secondFlippedCameraMatrix[12]; // 0x268
    float secondCameraMatrix[12];        // 0x238
    float secondProjectedProbe[14];      // 0x200
    float finalModelTransform[16];       // 0x1C0
    float finalTranslation[16];          // 0x180
    float finalClubRotation[16];         // 0x140
    float finalModelRotation[16];        // 0x100
    float firstCameraWordCopy[12];       // 0xD0
    float secondCameraWordCopy[12];      // 0xA0
    float firstProjectionProbe[3];       // 0x94
    float firstViewProbe[3];             // 0x88
    float secondViewProbe[3];            // 0x7C
    float secondProjectionProbe[3];      // 0x70
    float firstCameraBasisY[3];          // 0x64
    float firstCameraBasisZ[3];          // 0x58
    float firstCameraOrigin[3];          // 0x4C
    float secondCameraBasisY[3];         // 0x40
    float secondCameraBasisZ[3];         // 0x34
    float secondCameraOrigin[3];         // 0x28
    float firstCameraBasisX[3];          // 0x1C
    float secondCameraBasisX[3];         // 0x10

    // Phase 1: establish empty owned containers and identity camera/model bases.
    InitializePreviewOwnership(previewBytes, cameraDistance);

    int a0 = viewportRect[0];
    int a1 = viewportRect[1];
    AsCExhibition(previewBytes)->viewportLeft = a0;
    AsCExhibition(previewBytes)->viewportTop = a1;
    a0 = viewportRect[2];
    a1 = viewportRect[3];
    AsCExhibition(previewBytes)->viewportRight = a0;
    AsCExhibition(previewBytes)->viewportBottom = a1;
    InitializePreviewPlaybackAndResources(previewBytes);

    // Phase 2: set angle/distance limits and clamp the requested camera Z offset.
    float initialMinRotationX = kPreviewInitialMinRotationX;
    float initialMaxRotationX = kPreviewInitialMaxRotationX;
    float initialMinRotationY = kPreviewRotationYLowerBound;
    float initialMaxRotationY = kPreviewRotationYUpperBound;
    float initialMinCameraDistance = kPreviewInitialMinCameraDistance;
    float initialMaxCameraDistance = kPreviewInitialMaxCameraDistance;
    AsCExhibition(previewBytes)->minRotationX = initialMinRotationX;
    AsCExhibition(previewBytes)->maxRotationX = initialMaxRotationX;
    AsCExhibition(previewBytes)->minRotationY = initialMinRotationY;
    AsCExhibition(previewBytes)->maxRotationY = initialMaxRotationY;
    AsCExhibition(previewBytes)->minCameraDistance = initialMinCameraDistance;
    AsCExhibition(previewBytes)->maxCameraDistance = initialMaxCameraDistance;
    AsCExhibition(previewBytes)->screenCenterMinX = 0;
    AsCExhibition(previewBytes)->screenCenterMaxX = kPreviewScreenWidth;
    AsCExhibition(previewBytes)->screenCenterMinY = 0;
    AsCExhibition(previewBytes)->screenCenterMaxY = kPreviewScreenHeight;
    Memory_Clear(previewBytes + kPreviewResourceOptionsOffset, 0x14);

    float minimumCameraDistance = AsCExhibition(previewBytes)->minCameraDistance;
    float maximumCameraDistance = AsCExhibition(previewBytes)->maxCameraDistance;
    float clamped = (cameraDistance < minimumCameraDistance)
                        ? minimumCameraDistance
                        : ((cameraDistance > maximumCameraDistance) ? maximumCameraDistance : cameraDistance);
    AsCExhibition(previewBytes)->cameraOffsetZ = clamped;
    AsCExhibition(previewBytes)->cameraDistance = (float)(double)clamped;

    // Phase 3: project the model-space probe (1,1,0) through the preview camera.
    // Flipping camera X/Z bases changes its handedness for WView. The projected
    // Y determines translationY so the model aligns to the rectangle's bottom.
    firstProjectionProbe[0] = 1.0f;
    firstProjectionProbe[1] = 1.0f;
    firstProjectionProbe[2] = 0.0f;
    Matrix_SetIdentityAffine(firstCameraMatrix);
    firstCameraMatrix[11] = AsCExhibition(previewBytes)->cameraOffsetZ;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    *(PreviewVectorCopy*)firstCameraBasisX = *(PreviewVectorCopy*)&firstCameraMatrix[0];
    *(PreviewVectorCopy*)firstCameraBasisY = *(PreviewVectorCopy*)&firstCameraMatrix[3];
    *(PreviewVectorCopy*)firstCameraBasisZ = *(PreviewVectorCopy*)&firstCameraMatrix[6];
    *(PreviewVectorCopy*)firstCameraOrigin = *(PreviewVectorCopy*)&firstCameraMatrix[9];
    *(PreviewAffineCopy*)firstCameraWordCopy = *(PreviewAffineCopy*)firstCameraMatrix;
    *(PreviewAffineCopy*)firstFlippedCameraMatrix = *(PreviewAffineCopy*)firstCameraMatrix;
    *(WVector*)firstFlippedCameraMatrix *= -1.0f;
    *(WVector*)(firstFlippedCameraMatrix + 6) *= -1.0f;
    WView_SetViewMatrix(g_view, firstFlippedCameraMatrix);
    Camera_BuildFrustumPlanes(g_view);
    Camera_Apply(g_view, 0.0f, 0.0f);
    Matrix_TransformPoint(firstViewProbe, firstProjectionProbe, ((WView*)g_view)->inputTransform);
    WView_ProjectViewPointWithoutClipTest(g_view, firstProjectedProbe, firstViewProbe);

    {
        float viewportHeight = ((WView*)g_view)->viewportHeight;
        AsCExhibition(previewBytes)->translationY =
            -((float)AsCExhibition(previewBytes)->viewportBottom - 0.5f * viewportHeight) /
            (0.5f * viewportHeight - firstProjectedProbe[1]);
    }

    if (AsCExhibition(previewBytes)->modelKind == kPreviewClubSet) {
        Mtx44_SetEulerRotationZXY(firstClubRotation, AsCExhibition(previewBytes)->rotationY, 0.0f, 0.0f);
        Mtx44_SetEulerRotationZXY(firstModelRotation, 0.0f, AsCExhibition(previewBytes)->rotationX, 0.0f);
        PSMTX44Concat(firstModelRotation, firstClubRotation, firstModelRotation);
    } else {
        Mtx44_SetEulerRotationZXY(firstModelRotation, AsCExhibition(previewBytes)->rotationY,
                                  AsCExhibition(previewBytes)->rotationX, AsCExhibition(previewBytes)->rotationZ);
    }
    Mtx44_SetTranslation(firstTranslation, AsCExhibition(previewBytes)->translationX,
                         AsCExhibition(previewBytes)->translationY, 0.0f);
    PSMTX44Concat(firstModelTransform, firstModelRotation, firstTranslation);
    Matrix_ExtractAffineFrom4x4(firstModelTransform, previewBytes + kPreviewModelTransformOffset);
    ((CExhibition*)self)->UpdateModelPose();

    // Phase 4: repeat the original camera/probe alignment with the updated pose.
    // These repeated copies/calls are intentional parts of the compiled behavior.
    secondProjectionProbe[0] = 1.0f;
    secondProjectionProbe[1] = 1.0f;
    secondProjectionProbe[2] = 0.0f;
    Matrix_SetIdentityAffine(secondCameraMatrix);
    secondCameraMatrix[11] = AsCExhibition(previewBytes)->cameraOffsetZ;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    *(PreviewVectorCopy*)secondCameraBasisX = *(PreviewVectorCopy*)&secondCameraMatrix[0];
    *(PreviewVectorCopy*)secondCameraBasisY = *(PreviewVectorCopy*)&secondCameraMatrix[3];
    *(PreviewVectorCopy*)secondCameraBasisZ = *(PreviewVectorCopy*)&secondCameraMatrix[6];
    *(PreviewVectorCopy*)secondCameraOrigin = *(PreviewVectorCopy*)&secondCameraMatrix[9];
    *(PreviewAffineCopy*)secondCameraWordCopy = *(PreviewAffineCopy*)secondCameraMatrix;
    *(PreviewAffineCopy*)secondFlippedCameraMatrix = *(PreviewAffineCopy*)secondCameraMatrix;
    *(WVector*)secondFlippedCameraMatrix *= -1.0f;
    *(WVector*)(secondFlippedCameraMatrix + 6) *= -1.0f;
    WView_SetViewMatrix(g_view, secondFlippedCameraMatrix);
    Camera_BuildFrustumPlanes(g_view);
    Camera_Apply(g_view, 0.0f, 0.0f);
    Matrix_TransformPoint(secondViewProbe, secondProjectionProbe, ((WView*)g_view)->inputTransform);
    WView_ProjectViewPointWithoutClipTest(g_view, secondProjectedProbe, secondViewProbe);

    {
        float viewportHeight = ((WView*)g_view)->viewportHeight;
        AsCExhibition(previewBytes)->translationY =
            -((float)AsCExhibition(previewBytes)->viewportBottom - 0.5f * viewportHeight) /
            (0.5f * viewportHeight - secondProjectedProbe[1]);
    }

    if (AsCExhibition(previewBytes)->modelKind == kPreviewClubSet) {
        Mtx44_SetEulerRotationZXY(secondClubRotation, AsCExhibition(previewBytes)->rotationY, 0.0f, 0.0f);
        Mtx44_SetEulerRotationZXY(secondModelRotation, 0.0f, AsCExhibition(previewBytes)->rotationX, 0.0f);
        PSMTX44Concat(secondModelRotation, secondClubRotation, secondModelRotation);
    } else {
        Mtx44_SetEulerRotationZXY(secondModelRotation, AsCExhibition(previewBytes)->rotationY,
                                  AsCExhibition(previewBytes)->rotationX, AsCExhibition(previewBytes)->rotationZ);
    }
    Mtx44_SetTranslation(secondTranslation, AsCExhibition(previewBytes)->translationX,
                         AsCExhibition(previewBytes)->translationY, 0.0f);
    PSMTX44Concat(secondModelTransform, secondModelRotation, secondTranslation);
    Matrix_ExtractAffineFrom4x4(secondModelTransform, previewBytes + kPreviewModelTransformOffset);
    ((CExhibition*)self)->UpdateModelPose();

    if (AsCExhibition(previewBytes)->modelKind == kPreviewClubSet) {
        Mtx44_SetEulerRotationZXY(finalClubRotation, AsCExhibition(previewBytes)->rotationY, 0.0f, 0.0f);
        Mtx44_SetEulerRotationZXY(finalModelRotation, 0.0f, AsCExhibition(previewBytes)->rotationX, 0.0f);
        PSMTX44Concat(finalModelRotation, finalClubRotation, finalModelRotation);
    } else {
        Mtx44_SetEulerRotationZXY(finalModelRotation, AsCExhibition(previewBytes)->rotationY,
                                  AsCExhibition(previewBytes)->rotationX, AsCExhibition(previewBytes)->rotationZ);
    }
    Mtx44_SetTranslation(finalTranslation, AsCExhibition(previewBytes)->translationX,
                         AsCExhibition(previewBytes)->translationY, 0.0f);
    PSMTX44Concat(finalModelTransform, finalModelRotation, finalTranslation);
    Matrix_ExtractAffineFrom4x4(finalModelTransform, previewBytes + kPreviewModelTransformOffset);
    ((CExhibition*)self)->UpdateModelPose();

    // Phase 5: bind effects and propagate the final pixel center to attachments.
    ((CFxBox*)(previewBytes + 0xb0))->SetResourceManager(g_resourceManager);
    ((CFxBox*)(previewBytes + 0xb0))->SetCameraTransform(s_cameraTransform);
    AsCExhibition(previewBytes)->unk_B4 = kFloat_33_333_PreviewEffect;

    AsCExhibition(previewBytes)->screenCenterX =
        (float)((AsCExhibition(previewBytes)->viewportRight + AsCExhibition(previewBytes)->viewportLeft) / 2);
    AsCExhibition(previewBytes)->screenCenterY =
        (float)((AsCExhibition(previewBytes)->viewportBottom + AsCExhibition(previewBytes)->viewportTop) / 2);
    ((CExhibition*)self)->UpdateEffectScreenCenter();
}

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma use_lmw_stmw on
// Set the pixel rectangle [left, top, right, bottom] and clamped camera distance; return 1.
// cameraOffsetZ at +0x2C is a distance; the lens FOV is kExhibitionCameraFovRadians.
int CExhibition::SetViewport(int* rect, float cameraDistance) {
    // Phase 1: clamp the rectangle center to configured screen limits while
    // retaining its integer half extents. Edges use screen pixels, Y downward.
    float projectedProbe[14];
    PreviewCameraMatrix cameraBasis;
    PreviewCameraMatrix flippedCameraBasis;
    float modelTransform4x4[16], translationMatrix[16], clubRotation[16], modelRotation[16];
    PreviewCameraWordCopy cameraWordCopy;
    float modelSpaceProbe[3];
    PreviewVector3 viewSpaceProbe;
    PreviewCameraCopyOrder cameraCopyInAbiOrder;
    Box4BB0 box;
    char* base = (char*)this;
    {
        int y1, x1, y0, x0;
        unsigned int hi;
        x0 = rect[0];
        hi = (*(unsigned int*)&AsCExhibition(base)->screenCenterMaxX);
        AsCExhibition(base)->viewportLeft = x0;
        y0 = rect[1];
        AsCExhibition(base)->viewportTop = y0;
        x1 = rect[2];
        AsCExhibition(base)->viewportRight = x1;
        y1 = rect[3];
        AsCExhibition(base)->viewportBottom = y1;
        box.cy = (float)((y1 + y0) >> 1);
        box.cx = (float)((x1 + x0) >> 1);
        unsigned int v = ((Round4BB0)Runtime_ConvertDoubleToUnsigned)(box.cx);
        unsigned int lo = (*(unsigned int*)&AsCExhibition(base)->screenCenterMinX);
        unsigned int w = v < lo ? lo : (v > hi ? hi : v);
        box.cx = (float)w;
        hi = (*(unsigned int*)&AsCExhibition(base)->screenCenterMaxY);
        v = ((Round4BB0)Runtime_ConvertDoubleToUnsigned)(box.cy);
        lo = (*(unsigned int*)&AsCExhibition(base)->screenCenterMinY);
        w = v < lo ? lo : (v > hi ? hi : v);
        box.cy = (float)w;
        box.hh = (float)((y1 - y0) >> 1);
        box.hw = (float)((x1 - x0) >> 1);
        AsCExhibition(base)->viewportLeft = (int)(box.cx - box.hw);
        AsCExhibition(base)->viewportRight = (int)(box.cx + box.hw);
        AsCExhibition(base)->viewportTop = (int)(box.cy - box.hh);
        AsCExhibition(base)->viewportBottom = (int)(box.cy + box.hh);
    }
    // Phase 2: install the clamped camera Z offset and fixed-radian lens FOV.
    float low = AsCExhibition(base)->minCameraDistance;
    float high = AsCExhibition(base)->maxCameraDistance;
    double current = cameraDistance < low ? low : (cameraDistance > high ? high : cameraDistance);
    AsCExhibition(base)->cameraOffsetZ = current;
    modelSpaceProbe[0] = kFloat_1_Intro;
    modelSpaceProbe[1] = kFloat_1_Intro;
    modelSpaceProbe[2] = kFloat_0_exhibition;
    AsCExhibition(base)->cameraDistance = (float)current;
    Matrix_SetIdentityAffine(&cameraBasis);
    cameraBasis.origin.z = AsCExhibition(base)->cameraOffsetZ;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    PreviewVector3 cameraFirstBasis = cameraBasis.basisX;
    cameraCopyInAbiOrder = MakeCameraCopyInAbiOrder(cameraFirstBasis, cameraBasis);
    cameraWordCopy = *(PreviewCameraWordCopy*)&cameraBasis;
    flippedCameraBasis = *(PreviewCameraMatrix*)&cameraWordCopy;
    *(WVector*)&flippedCameraBasis *= kFloat_neg1_Exhibition;
    *(WVector*)(&flippedCameraBasis.basisZ) *= kFloat_neg1_Exhibition;
    WView_SetViewMatrix(g_view, &flippedCameraBasis);
    Camera_BuildFrustumPlanes(g_view);
    Camera_Apply(g_view, kFloat_0_exhibition, kFloat_0_exhibition);
    Matrix_TransformPoint(&viewSpaceProbe, &modelSpaceProbe, ((WView*)g_view)->inputTransform);
    WView_ProjectViewPointWithoutClipTest(g_view, projectedProbe, &viewSpaceProbe);
    // Phase 3: convert the desired pixel bottom into model translation using
    // the projected (1,1,0) probe; the denominator must be nonzero.
    float viewportHeight = ((WView*)g_view)->viewportHeight;
    AsCExhibition(base)->translationY =
        PreviewBottomTranslation(AsCExhibition(base)->viewportBottom, viewportHeight, projectedProbe[1]);
    BuildAndApplyPreviewPose(this, clubRotation, modelRotation, translationMatrix, modelTransform4x4);
    int sumX = AsCExhibition(base)->viewportRight + AsCExhibition(base)->viewportLeft;
    int sumY = AsCExhibition(base)->viewportBottom + AsCExhibition(base)->viewportTop;
    AsCExhibition(base)->screenCenterX = (float)(sumX / 2);
    AsCExhibition(base)->screenCenterY = (float)(sumY / 2);
    UpdateEffectScreenCenter();
    return 1;
}
#pragma pop

// Translate the preview rectangle and rebuild its camera/model alignment.
int CExhibition::OffsetViewport(int x, int y) {
    // Phase 1: add pixel offsets, then clamp the center without changing half extents.
    float matrix[16];
    float rotation[16];
    float translationMatrix[16];
    float combined[16];
    PreviewCameraMatrix view;
    PreviewCameraMatrix cam;
    float frustum[14];
    PreviewCameraMatrix copy;
    Rect5048 rect = {viewportLeft, 0, 0, 0};
    PreviewVector3 transformed;
    PreviewVector3 origin;
    Box5048 box;
    char* base = (char*)this;
    double current = AsCExhibition(base)->cameraOffsetZ;
    {
        int y0 = AsCExhibition(base)->viewportTop + y, y1 = AsCExhibition(base)->viewportBottom + y;
        int x0 = rect.x0 + x, x1 = AsCExhibition(base)->viewportRight + x;
        box.cy = (float)((y1 + y0) >> 1);
        box.cx = (float)((x1 + x0) >> 1);
        unsigned int hi = (*(unsigned int*)&AsCExhibition(base)->screenCenterMaxX);
        rect.x0 = x0;
        rect.x1 = x1;
        rect.y0 = y0;
        rect.y1 = y1;
        (*(Rect5048*)&AsCExhibition(base)->viewportLeft) = rect;
        unsigned int v = ((Round4BB0)Runtime_ConvertDoubleToUnsigned)(box.cx);
        unsigned int lo = (*(unsigned int*)&AsCExhibition(base)->screenCenterMinX);
        unsigned int w = v < lo ? lo : (v > hi ? hi : v);
        box.cx = (float)w;
        hi = (*(unsigned int*)&AsCExhibition(base)->screenCenterMaxY);
        v = ((Round4BB0)Runtime_ConvertDoubleToUnsigned)(box.cy);
        lo = (*(unsigned int*)&AsCExhibition(base)->screenCenterMinY);
        w = v < lo ? lo : (v > hi ? hi : v);
        box.cy = (float)w;
        float hh = (float)((y1 - y0) >> 1);
        float hw = (float)((x1 - x0) >> 1);
        box.hw = hw;
        box.hh = hh;
        AsCExhibition(base)->viewportLeft = (int)(box.cx - box.hw);
        AsCExhibition(base)->viewportRight = (int)(box.cx + box.hw);
        AsCExhibition(base)->viewportTop = (int)(box.cy - box.hh);
        AsCExhibition(base)->viewportBottom = (int)(box.cy + box.hh);
    }
    // Phase 2: rebuild the shared camera and project the model-space (1,1,0) probe.
    double low = AsCExhibition(base)->minCameraDistance, high = AsCExhibition(base)->maxCameraDistance;
    double cf = current < low ? low : (current > high ? high : current);
    AsCExhibition(base)->cameraOffsetZ = cf;
    origin.x = kFloat_1_Intro;
    origin.y = kFloat_1_Intro;
    origin.z = kFloat_0_exhibition;
    AsCExhibition(base)->cameraDistance = (float)cf;
    Matrix_SetIdentityAffine(&cam);
    cam.origin.z = AsCExhibition(base)->cameraOffsetZ;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    copy = cam;
    PreviewCameraCopyOrder sw = {copy.basisX, cam.origin, cam.basisZ, cam.basisY};
    view = copy;
    *(WVector*)&view *= kFloat_neg1_Exhibition;
    *(WVector*)(&view.basisZ) *= kFloat_neg1_Exhibition;
    WView_SetViewMatrix(g_view, &view);
    Camera_BuildFrustumPlanes(g_view);
    Camera_Apply(g_view, kFloat_0_exhibition, kFloat_0_exhibition);
    Matrix_TransformPoint(&transformed, &origin, ((WView*)g_view)->inputTransform);
    WView_ProjectViewPointWithoutClipTest(g_view, frustum, &transformed);
    float viewportHeight = ((WView*)g_view)->viewportHeight;
    AsCExhibition(base)->translationY =
        PreviewBottomTranslation(AsCExhibition(base)->viewportBottom, viewportHeight, frustum[1]);
    // Phase 3: rebuild the model transform, then publish the new attachment pixel center.
    BuildAndApplyPreviewPose(this, rotation, matrix, translationMatrix, combined);
    AsCExhibition(base)->screenCenterX =
        (float)((AsCExhibition(base)->viewportRight + AsCExhibition(base)->viewportLeft) / 2);
    AsCExhibition(base)->screenCenterY =
        (float)((AsCExhibition(base)->viewportBottom + AsCExhibition(base)->viewportTop) / 2);
    UpdateEffectScreenCenter();
    return 1;
}

#pragma push
#pragma always_inline on
// Flip X/Z basis for the preview view convention; keep translation and the Y basis unchanged.
static inline void InstallViewportLimitCameraView(Cam5504& view) {
    *(WVector*)&view *= kFloat_neg1_Exhibition;
    *(WVector*)(&view.basisZ) *= kFloat_neg1_Exhibition;
    WView_SetViewMatrix(g_view, &view);
}
#pragma pop

// Set vertical pixel-center limits, then align the camera and model to the clamped rectangle.
int CExhibition::SetViewportYLimits(unsigned int minimumY, unsigned int maximumY) {
    // Phase 1: order/cap the pixel limits and clamp the existing rectangle's center.
    char* base = (char*)this;
    if (minimumY > maximumY) {
        unsigned int t = minimumY;
        minimumY = maximumY;
        maximumY = t;
    }
    unsigned int clampedMinimumY = minimumY > kPreviewScreenHeight ? kPreviewScreenHeight : minimumY;
    unsigned int clampedMaximumY = maximumY > kPreviewScreenHeight ? kPreviewScreenHeight : maximumY;
    int y0 = AsCExhibition(base)->viewportTop, y1 = AsCExhibition(base)->viewportBottom;
    int x0 = AsCExhibition(base)->viewportLeft, x1 = AsCExhibition(base)->viewportRight;
    Box5504 box;
    box.cy = (float)((y0 + y1) >> 1);
    box.cx = (float)((x0 + x1) >> 1);
    float cameraDistance = AsCExhibition(base)->cameraOffsetZ;
    unsigned int hi = (*(unsigned int*)&AsCExhibition(base)->screenCenterMaxX);
    (*(unsigned int*)&AsCExhibition(base)->screenCenterMinY) = clampedMinimumY;
    (*(unsigned int*)&AsCExhibition(base)->screenCenterMaxY) = clampedMaximumY;
    (*(Rect5504*)&AsCExhibition(base)->viewportLeft) = makeRect5504(x0, y0, x1, y1);
    unsigned int v = ((Round5504)Runtime_ConvertDoubleToUnsigned)(box.cx);
    unsigned int lo = (*(unsigned int*)&AsCExhibition(base)->screenCenterMinX);
    unsigned int w = v < lo ? lo : (v > hi ? hi : v);
    box.cx = (float)w;
    hi = (*(unsigned int*)&AsCExhibition(base)->screenCenterMaxY);
    v = ((Round5504)Runtime_ConvertDoubleToUnsigned)(box.cy);
    lo = (*(unsigned int*)&AsCExhibition(base)->screenCenterMinY);
    w = v < lo ? lo : (v > hi ? hi : v);
    box.cy = (float)w;
    box.hh = (float)((y1 - y0) >> 1);
    box.hw = (float)((x1 - x0) >> 1);
    AsCExhibition(base)->viewportLeft = (int)(box.cx - box.hw);
    AsCExhibition(base)->viewportRight = (int)(box.cx + box.hw);
    AsCExhibition(base)->viewportTop = (int)(box.cy - box.hh);
    AsCExhibition(base)->viewportBottom = (int)(box.cy + box.hh);
    // Phase 2: repeat camera/probe alignment using the clamped rectangle's bottom.
    float minimumCameraDistance = AsCExhibition(base)->minCameraDistance,
          maximumCameraDistance = AsCExhibition(base)->maxCameraDistance;
    float clampedCameraDistance =
        cameraDistance < minimumCameraDistance
            ? minimumCameraDistance
            : (cameraDistance > maximumCameraDistance ? maximumCameraDistance : cameraDistance);
    AsCExhibition(base)->cameraOffsetZ = clampedCameraDistance;
    AsCExhibition(base)->cameraDistance = (float)(double)clampedCameraDistance;
    // Model-space (1,1,0) probe -> view space -> projected screen result.
    // Keep fractional projection until PreviewBottomTranslation; only the earlier
    // rectangle center/half-extents use integer pixel rounding.
    V3_5504 modelSpaceProbe = {kFloat_1_Intro, kFloat_1_Intro, kFloat_0_exhibition};
    Cam5504 cameraBasis;
    Matrix_SetIdentityAffine(&cameraBasis);
    cameraBasis.origin.z = AsCExhibition(base)->cameraOffsetZ;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    Cam5504 c0 = cameraBasis;
    CamSw5504 sw = toSw5504(cameraBasis);
    Cam5504 view = c0;
    InstallViewportLimitCameraView(view);
    Camera_BuildFrustumPlanes(g_view);
    Camera_Apply(g_view, kFloat_0_exhibition, kFloat_0_exhibition);
    float viewSpaceProbe[3];
    Matrix_TransformPoint(viewSpaceProbe, &modelSpaceProbe, ((WView*)g_view)->inputTransform);
    float frustum[14];
    WView_ProjectViewPointWithoutClipTest(g_view, frustum, viewSpaceProbe);
    float h = ((WView*)g_view)->viewportHeight;
    AsCExhibition(base)->translationY = PreviewBottomTranslation(AsCExhibition(base)->viewportBottom, h, frustum[1]);
    // Phase 3: publish the model transform and propagate its screen center to effects.
    float matrixA[16], matrixB[16], matrixC[16], matrixD[16];
    BuildAndApplyPreviewPose(this, matrixA, matrixB, matrixC, matrixD);
    AsCExhibition(base)->screenCenterX =
        (float)((AsCExhibition(base)->viewportRight + AsCExhibition(base)->viewportLeft) / 2);
    AsCExhibition(base)->screenCenterY =
        (float)((AsCExhibition(base)->viewportBottom + AsCExhibition(base)->viewportTop) / 2);
    UpdateEffectScreenCenter();
    return 1;
}

// Apply the controller attachment, then mirror a valid slot and rebuild effects.
// value is borrowed here. AvatarModel_SetAttachment decides its own acceptance/
// ownership first; the five-slot preview mirror has a separate bound. A return of
// 0 after controller success does NOT roll back that controller mutation. Preserve
// this order when changing slot limits or callers that retry after failure.
int CExhibition::SetAttachment(unsigned int index, void* value) {
    void* ptr = modelController;
    if (ptr == 0)
        return 0;
    if (!AvatarModel_SetAttachment(ptr, index, value))
        return 0;
    if (index >= kPreviewAttachmentCount)
        return 0;
    if ((int)index >= kPreviewAttachmentCount)
        index = 0;
    ((PreviewAttachmentSlots*)this)->attachments[index] = value;
    RebuildAttachedEffects();
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Release preview models, effects, lights, preloaders, strings, and owned container storage.
CExhibition::~CExhibition() {
    char* base = (char*)this;
    int mode = AsCExhibition(base)->modelKind;
    *(void**)base = __vt__11CExhibition;
    if (mode == 0) {
        void* p = AsCExhibition(base)->modelController;
        if (p) {
            PlayerModel_Dtor(p, 1);
            AsCExhibition(base)->modelController = 0;
        }
        AsCExhibition(base)->puppet = 0;
    } else if (g_resourceManager && AsCExhibition(base)->puppet) {
        ResourceManager_ReleaseObject(g_resourceManager, AsCExhibition(base)->puppet, 0);
        AsCExhibition(base)->puppet = 0;
    }
    {
        void** entries = (*(void***)&AsCExhibition(base)->ownedNameStrings);
        for (; Unequal5A88(ArrayEnd5A88((void***)(base + 4), (unsigned*)(base + 8)), entries); ++entries) {
            void* entry = *entries;
            if (entry) {
                if (entry) {
                    if (*(unsigned int*)entry >> 31)
                        appFreeUni__FPv(*(void**)((char*)entry + 8));
                    __dl__FPv(entry);
                }
                *entries = 0;
            }
        }
        ReduceCount5A88(AsCExhibition(base)->ownedNameStringCount, *(unsigned*)(base + 8));
    }
    // Phase 2: clear registered effects, then delete owned preview records.
    ((CFxBox*)(base + 0xb0))->ClearEffects(0);
    {
        void* node = AsCExhibition(base)->ownedPreviewListSentinelPrev;
        void* head = base + 0x460;
        while (node != head) {
            void* child = *(void**)((char*)node + 8);
            if (child) {
                void* x = ((PreviewOwnedRecord*)child)->ownedData;
                if (x) {
                    __dl__FPv(x);
                    ((PreviewOwnedRecord*)child)->ownedData = 0;
                }
                __dl__FPv(child);
            }
            node = *(void**)((char*)node + 4);
        }
    }
    ClearList5A88(base + 0x45c);
    {
        // Phase 3: release lighting/file resources and delete copied light descriptions.
        RefObj802B* obj = (*(RefObj802B**)&AsCExhibition(base)->lightingResource);
        if (obj) {
            int rc = *(int*)obj;
            --rc;
            *(int*)obj = rc;
            if (rc == 0) {
                delete obj;
            }
            (*(void**)&AsCExhibition(base)->lightingResource) = 0;
        }
    }
    {
        Del77E8* obj = (*(Del77E8**)&AsCExhibition(base)->preloadFile);
        if (obj) {
            delete obj;
            (*(void**)&AsCExhibition(base)->preloadFile) = 0;
        }
    }
    {
        void** arr = (*(void***)&AsCExhibition(base)->lights);
        for (; Unequal5A88(ArrayEnd5A88((void***)(base + 0x530), (unsigned*)(base + 0x534)), arr); ++arr) {
            void* p = *arr;
            if (p) {
                __dl__FPv(p);
                *arr = 0;
            }
        }
        ReduceCount5A88(AsCExhibition(base)->lightCount, *(unsigned*)(base + 0x534));
        ResolvedItemSelection_Destroy(base + 0x550, -1);
        dtor_80293270(base + kPreviewCallbackOffset, -1);
        DestroyPreviewPointerArray(base + 0x530);
    }
    // Phase 4: free owned string/container storage and destroy the effect box.
    if (base + 0x518)
        if (AsCExhibition(base)->modelPathFlags >> 31)
            appFreeUni__FPv((*(void**)&AsCExhibition(base)->modelPathData));
    if (base + 0x4fc)
        if (AsCExhibition(base)->animationNameFlags >> 31)
            appFreeUni__FPv((*(void**)&AsCExhibition(base)->animationNameData));
    if (base + 0x45c)
        if (base + 0x45c)
            if (base + 0x45c)
                if (base + 0x45c) {
                    ClearList5A88(base + 0x45c);
                }
    DestroyPreviewPointerArray(base + 0x450);
    DestroyPreviewPointerArray(base + 0x444);
    if (base + 0xb0) {
        CLEAN_TREE(base + 0x438);
        CLEAN_TREE(base + 0x42c);
        __dt__6CFxBoxFv(base + 0xb0, 0);
    }
    DestroyPreviewPointerArray(base + 4);
}
#pragma pop

// Poll preview resources, load the requested item/model or parts, and advance animation/effects.
void CExhibition::UpdateLoadState(float dt, int) {
    // Phase 1: give both optional preloaders a chance to finish before dispatch.
    char* base = (char*)this;
    Controller6388* controller = (*(Controller6388**)&AsCExhibition(base)->modelPreloader);
    if (controller) {
        controller->update();
    }
    controller = (*(Controller6388**)&AsCExhibition(base)->preloadFile);
    if (controller) {
        controller->update();
    }
    bool valid;
    switch (AsCExhibition(base)->loadState) {
    case kPreviewLoadModel: {
        // Full replacement: wait for both preloaders, release old ownership,
        // load avatar resources or a direct puppet, then publish state 4.
        // State 4 is published even when the resource load returns null.
        valid = Ready6388((*(Controller6388**)&AsCExhibition(base)->modelPreloader));
        if (valid) {
            valid = Ready6388((*(Controller6388**)&AsCExhibition(base)->preloadFile));
            if (valid) {
                if (AsCExhibition(base)->modelKind == kPreviewAvatar) {
                    if (AsCExhibition(base)->modelKind == kPreviewAvatar) {
                        void* p = AsCExhibition(base)->modelController;
                        if (p) {
                            PlayerModel_Dtor(p, 1);
                            AsCExhibition(base)->modelController = 0;
                        }
                        AsCExhibition(base)->puppet = 0;
                    } else if (g_resourceManager && AsCExhibition(base)->puppet) {
                        ResourceManager_ReleaseObject(g_resourceManager, AsCExhibition(base)->puppet, 0);
                        AsCExhibition(base)->puppet = 0;
                    }
                    void** entries = *(void***)(base + kPreviewNamesData);
                    for (; Unequal6388(*(void***)(base + kPreviewNamesData) + *(unsigned*)(base + kPreviewNamesSize),
                                       entries);
                         ++entries) {
                        void* entry = *entries;
                        if (entry) {
                            if (entry) {
                                if (((PreviewOwnedStringView*)entry)->flags >> 31)
                                    appFreeUni__FPv(((PreviewOwnedStringView*)entry)->heapData);
                                __dl__FPv(entry);
                            }
                            *entries = 0;
                        }
                    }
                    Reduce6388(*(unsigned*)(base + kPreviewNamesSize), *(unsigned*)(base + kPreviewNamesSize));
                    AllocatedController* object = new AllocatedController;
                    AsCExhibition(base)->modelController = object;
                    if (AvatarModel_LoadCharacterResources(object, base + kPreviewAvatarSelectionOffset,
                                                           base + kPreviewResourceOptionsOffset, 1, 0, 0,
                                                           AsCExhibition(base)->textureVariantIndex, 0)) {
                        AsCExhibition(base)->puppet =
                            (void*)PlayerModel_GetPuppet(AsCExhibition(base)->modelController);
                        PlayerModel_DefaultReady(AsCExhibition(base)->modelController,
                                                 base + kPreviewAvatarSelectionOffset);
                        char* object = (*(char**)&AsCExhibition(base)->modelController);
                        if (object) {
                            AsCExhibition(base)->resolvedCharacterId =
                                ((PreviewControllerSelectionView*)object)->characterId;
                            (*(ResolvedAvatarPartCopy*)&AsCExhibition(base)->resolvedPartData) =
                                ((PreviewControllerSelectionView*)object)->parts;
                        }
                        InitializeLoadedModel();
                    }
                } else {
                    Clear6388(base);
                    int flag = AsCExhibition(base)->modelPathFlags >> 31;
                    void* key = !flag ? (char*)&AsCExhibition(base)->modelPathFlags + 1
                                      : (*(void**)&AsCExhibition(base)->modelPathData);
                    void* result = (void*)ResourceManager_LoadByExtension(g_resourceManager, key, 0, 1, 0);
                    AsCExhibition(base)->puppet = result;
                    if (result) {
                        if (AsCExhibition(base)->modelKind == kPreviewCaddie) {
                            WPuppet_RebuildBoneController(result);
                            WPuppet_InitializeTextureCoordinateAnimation(AsCExhibition(base)->puppet);
                        }
                        if ((*(unsigned*)&AsCExhibition(base)->itemTypeId) ==
                                (unsigned)ItemInventory_GetBallIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, 12) ||
                            (*(unsigned*)&AsCExhibition(base)->itemTypeId) ==
                                (unsigned)ItemInventory_GetBallIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, 128))
                            CreateBallEffects();
                        InitializeLoadedModel();
                    }
                }
                AsCExhibition(base)->loadState = kPreviewReady;
            }
        }
        break;
    }
    case kPreviewReloadParts: {
        // Apply resolved avatar parts only after both preloaders report ready.
        StateTwo6388(base);
        break;
    }
    case kPreviewWaitFrames: {
        // Strict > test: the counter must exceed the limit before becoming ready.
        int limit = s_previewLoadWaitFrameLimit;
        if (PreviewWaitLimitExceeded(limit, base))
            AsCExhibition(base)->loadState = kPreviewReady;
        else
            AsCExhibition(base)->loadWaitFrames = AsCExhibition(base)->loadWaitFrames + 1;
        break;
    }
    default:
        break;
    }
    // Phase 3: animation/effects run in every state; finish any pending part load.
    UpdateAnimationAndEffects(dt);
    if (AsCExhibition(base)->partLoadPending && AsCExhibition(base)->modelController &&
        PuppetResourceList_PollLoadAndUpdateBounds(AsCExhibition(base)->modelController))
        AsCExhibition(base)->partLoadPending = 0;
}

// Select a named animation, process its initial events, and clamp the starting frame to its end.
int CExhibition::PlayAnimationByName(void* nameOrAnimationRange, int flag, float delta) {
    Owner6888* owner = (Owner6888*)this;
    if (0 == nameOrAnimationRange)
        return 0;
    if (flag == 0) {
        ((CFxBox*)(((char*)(fxBox))))->ClearEffects(0);
        RebuildAttachedEffects();
    }
    if (modelKind == kPreviewAvatar) {
        if (!modelController)
            return 0;
        owner->animation = (void*)ObjectList_FindAnimationRangeByName(modelController, nameOrAnimationRange, 1, 0);
        PlayerModel_SetDefaultPuppetLayerEnabled(modelController, 0);
    } else {
        if (!puppet)
            return 0;
        owner->animation = (void*)WPuppet_FindAnimationRangeByName(puppet, nameOrAnimationRange);
        WPuppet_SetBoneVisibilityRecursive(puppet, 1, 0, 0);
    }
    nameOrAnimationRange = owner->animation;
    if (nameOrAnimationRange) {
        float startFrame = *(float*)nameOrAnimationRange;
        void* resource = target6888(owner);
        AnimationEventParser_ProcessFrameRange(this, resource, startFrame, startFrame, nameOrAnimationRange, -1, 1);
        float current = *(float*)owner->animation + delta;
        float end = *((float*)owner->animation + 1);
        end = (current > end) ? end : current;
        animationFrame = end;
        UpdateModelPose();
        return 1;
    }
    return 0;
}

// Draw the preview with its own camera, clipping rectangle, lights, and temporarily overridden render state.
void CExhibition::DrawScene() {
    // Phase 1: resolve the puppet and install this preview's camera/screen center.
    // The global camera/view and lights remain changed after drawing. The local
    // guards restore cached render states and scissor rectangle only; the GX
    // scissor-box offset is explicitly reset to zero at the end.
    char* b = (char*)this;
    volatile int vp[36];
    ViewObj6A1C* view;
    if (AsCExhibition(b)->displaySuppressed)
        return;
    view = ExhibitionView(this);
    if (!view)
        return;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    WView_InvalidateViewAndProjection(g_view);
    Camera_SetClip(g_view, s_previewNearClip, s_previewFarClip, 0);

    PreviewCameraBasis m;
    m = *(PreviewCameraBasis*)(((char*)(AsCExhibition(b)->cameraMatrixPrefix)));
    m = *(PreviewCameraBasis*)(((char*)(AsCExhibition(b)->cameraMatrixPrefix)));
    *(PreviewAffineWordCopy*)&m = *(PreviewAffineWordCopy*)(((char*)(AsCExhibition(b)->cameraMatrixPrefix)));
    m = *(PreviewCameraBasis*)(((char*)(AsCExhibition(b)->cameraMatrixPrefix)));
    m = *(PreviewCameraBasis*)(((char*)(AsCExhibition(b)->cameraMatrixPrefix)));
    *(PreviewAffineWordCopy*)&m = *(PreviewAffineWordCopy*)(((char*)(AsCExhibition(b)->cameraMatrixPrefix)));
    *(WVector*)&m *= kFloat_neg1_Exhibition;
    *(WVector*)(&m.basisZ) *= kFloat_neg1_Exhibition;
    WView_SetViewMatrix(g_view, &m);
    Camera_BuildFrustumPlanes(g_view);
    if (0 == g_view)
        return;
    PreviewCameraBasis p;
    p = *(PreviewCameraBasis*)(((WView*)g_view)->viewMatrix);
    p = *(PreviewCameraBasis*)(((WView*)g_view)->viewMatrix);
    *(PreviewAffineWordCopy*)&p = *(PreviewAffineWordCopy*)(((WView*)g_view)->viewMatrix);
    p = *(PreviewCameraBasis*)(((WView*)g_view)->viewMatrix);
    p = *(PreviewCameraBasis*)(((WView*)g_view)->viewMatrix);
    *(PreviewAffineWordCopy*)&p = *(PreviewAffineWordCopy*)(((WView*)g_view)->viewMatrix);
    *(WVector*)&p *= kFloat_neg1_Exhibition;
    *(WVector*)(&p.basisZ) *= kFloat_neg1_Exhibition;
    Matrix_CopyAffine(s_cameraTransform, &p);
    WView_SetScreenCenter(g_view, AsCExhibition(b)->screenCenterX, AsCExhibition(b)->screenCenterY);

    vp[35] =
        ((Cvt6A1C)Runtime_ConvertDoubleToUnsigned)(AsCExhibition(b)->screenCenterX - kPreviewReferenceScreenCenterX);
    vp[0] =
        ((Cvt6A1C)Runtime_ConvertDoubleToUnsigned)(AsCExhibition(b)->screenCenterY - kPreviewReferenceScreenCenterY);
    vp[7] = kPreviewScreenWidth;
    *(volatile float*)&vp[21] = kFloat_0_exhibition;
    vp[14] = kPreviewScreenHeight;
    *(volatile float*)&vp[28] = kFloat_1_Intro;

    // Phase 2: save/override render states and clip the fixed 640x480 preview.
    int off = (int)(kPreviewScissorOffsetScale *
                    (kFloat_0_5_exhibition * -(AsCExhibition(b)->screenCenterX - kPreviewReferenceScreenCenterX)));
    GXSetScissorBoxOffset(off, 0);
    StateGuard6A1C g0(kPreviewDepthWrite, kPreviewEnabled);
    StateGuard6A1C g1(kPreviewDepthTest, kPreviewEnabled);
    StateGuard6A1C g2(kPreviewDepthCompare, kPreviewCompareLessEqual);
    StateGuard6A1C g3(kPreviewLightingEnabled, kPreviewEnabled);
    StateGuard6A1C g4(kPreviewBlendEnabled, kPreviewEnabled);
    StateGuard6A1C g5(kPreviewSourceBlend, kPreviewSourceAlpha);
    StateGuard6A1C g6(kPreviewDestinationBlend, kPreviewInverseSourceAlpha);
    StateGuard6A1C g7(kPreviewCullMode, kPreviewCullNone);
    int sc[4];
    sc[0] = 0;
    sc[2] = 0x27f;
    if (0 < off)
        sc[0] = off;
    else if (0 > off)
        sc[2] = off + 0x27f;
    sc[1] = 0;
    sc[3] = 0x1df;
    RectGuard6A1C saved;
    Engine_SetScissorRect(sc);
    // Phase 3: replace active lights, draw the puppet, then draw its effect pass.
    WPuppet_SetRendererOption(view, 0);
    for (int i = 0; kHardwareLightCount > i; i++)
        Engine_SetLightEnabled(i, 0);
    int count = AsCExhibition(b)->lightCount;
    for (unsigned int i = 0; count > i; i++) {
        Engine_SetLightEnabled(i, 1);
        Engine_SetLight(i, ((int*)(*(void**)&AsCExhibition(b)->lights))[i]);
    }
    view->m4(g_view, 1, 0, kFloat_1_Intro, 0, 1);
    if (g_sceneManager)
        ((CSceneManager*)g_sceneManager)->effectRenderPassActive = 1;
    ((CFxBox*)(((char*)(AsCExhibition(b)->fxBox))))->DrawSprays(g_view);
    ((CamObj6A1C*)g_view)->m2(4);
    if (g_sceneManager)
        ((CSceneManager*)g_sceneManager)->effectRenderPassActive = 0;
    GXSetScissorBoxOffset(0, 0);
}

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Queue an avatar/equipment description and prepare its animation, resources, and model transform.
void CExhibition::ConfigureAvatarPreview(void* selectedDescription, void* resourceOptions, const char* animationName,
                                         void* textureVariant, float maximumAnimationDelay) {
    // Phase 1: reject an unchanged description or a busy preloader/load state.
    // selectedDescription must supply 0x68 readable bytes; resourceOptions is
    // optional (0x14 bytes copied). animationName is copied; textureVariant is
    // stored as a word for later load processing, with no retain here.
    float origin[3];
    char transform[0x3c];
    char result[0x38];
    PreviewCameraBasis camera;
    char translationMatrix[0x40], rotationMatrix[0x40], combined[0x40], clubRotation[0x40];
    char* base = (char*)this;
    if (ItemSelection_Equals(((char*)(base + kPreviewAvatarSelectionOffset)), selectedDescription))
        return;
    void* old = (*(void**)&AsCExhibition(base)->preloadFile);
    int valid = 1;
    if (old && !((Sub802B87F4*)old)->m2())
        valid = 0;
    int ready;
    if (valid) {
        unsigned int _st = (*(unsigned int*)&AsCExhibition(base)->loadState) - 1;
        ready = _st > 2u;
    } else
        ready = 0;
    if (!ready)
        return;
    {
        void* next = CPreloadFileAsync::Create(s_previewEnvironmentResourceName);
        if (next) {
            old = (*(void**)&AsCExhibition(base)->preloadFile);
            if (old) {
                if (old)
                    ((Del802B734C*)old)->m0(1);
                (*(void**)&AsCExhibition(base)->preloadFile) = 0;
            }
            (*(void**)&AsCExhibition(base)->preloadFile) = next;
        }
    }
    // Phase 2: copy the selected appearance/options and choose animation text.
    AsCExhibition(base)->itemTypeId = *(int*)selectedDescription;
    (*(void**)&AsCExhibition(base)->avatarResourceOptions) = resourceOptions;
    if (resourceOptions) {
        if (resourceOptions)
            Memory_Copy(base + kPreviewResourceOptionsOffset, resourceOptions, 0x14);
    } else
        Memory_Clear(base + kPreviewResourceOptionsOffset, 0x14);
    AsCExhibition(base)->modelKind = kPreviewAvatar;
    (*(int*)&AsCExhibition(base)->avatarCharacterId) = *(int*)selectedDescription;
    (*(AvatarSelectionPartCopy*)&AsCExhibition(base)->avatarSelectionData) =
        *(AvatarSelectionPartCopy*)((char*)selectedDescription + 4);
    if (animationName) {
        int previousAnimationNameLength = (AsCExhibition(base)->animationNameFlags >> 31) == 0
                                              ? ((*(unsigned char*)&AsCExhibition(base)->animationNameFlags) & 0x7f)
                                              : AsCExhibition(base)->animationNameLength;
        volatile PreviewStringAllocatorSlots sl;
        sl.b = sl.a;
        String_ReplaceRange(((char*)(&AsCExhibition(base)->animationNameFlags)), 0, previousAnimationNameLength,
                            animationName, animationName + strlen(animationName), (const char*)&sl.b);
    } else {
        const char* fallback = (const char*)s_previewFallbackAnimationName;
        int previousAnimationNameLength = (AsCExhibition(base)->animationNameFlags >> 31) == 0
                                              ? ((*(unsigned char*)&AsCExhibition(base)->animationNameFlags) & 0x7f)
                                              : AsCExhibition(base)->animationNameLength;
        volatile PreviewStringAllocatorSlots sl;
        sl.b = sl.a;
        String_ReplaceRange(((char*)(&AsCExhibition(base)->animationNameFlags)), 0, previousAnimationNameLength,
                            fallback, fallback + strlen(fallback), (const char*)&sl.b);
    }
    AsCExhibition(base)->maxAnimationDelay = maximumAnimationDelay;
    AsCExhibition(base)->animationMode = 0;
    // Phase 3: align the (1,1,0) probe with the viewport bottom, rebuild the pose,
    // and publish load state 1. Resource loading happens on later update calls.
    origin[0] = kFloat_1_Intro;
    origin[1] = kFloat_1_Intro;
    origin[2] = kFloat_0_exhibition;
    (*(void**)&AsCExhibition(base)->textureVariantIndex) = textureVariant;
    Matrix_SetIdentityAffine(&camera);
    camera.origin.z = AsCExhibition(base)->cameraOffsetZ;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    setCam734C_a(camera);
    Camera_BuildFrustumPlanes(g_view);
    Camera_Apply(g_view, kFloat_0_exhibition, kFloat_0_exhibition);
    Matrix_TransformPoint(transform, origin, ((WView*)g_view)->inputTransform);
    WView_ProjectViewPointWithoutClipTest(g_view, result, transform);
    AsCExhibition(base)->translationY =
        -((float)AsCExhibition(base)->viewportBottom - ((WView*)g_view)->viewportHeight * kFloat_0_5_exhibition) /
        (((WView*)g_view)->viewportHeight * kFloat_0_5_exhibition - ((float*)result)[1]);

    BuildAndApplyPreviewPose(this, clubRotation, rotationMatrix, translationMatrix, combined);
    AsCExhibition(base)->loadState = kPreviewLoadModel;
}
#pragma pop

// Resolve a caddie, caddie item, ball, or club-set IFF ID into the appropriate preview model path.
void CExhibition::SetItemType(unsigned int itemTypeId) {
    // Phase 1: on a changed ID, replace the optional preloader and clear avatar options.
    // The ID/model kind are written before lookup; missing item data can leave
    // those fields changed without scheduling a new load.
    float matrix[16];
    const char* s;

    float rotation[16];

    char* base = (char*)this;
    if ((*(unsigned int*)&AsCExhibition(base)->itemTypeId) != itemTypeId) {
        float translationMatrix[16];
        float combined[16];
        PreviewVector3 transformed;
        if (ExhibitionIsReady(this)) {
            void* replacement = CPreloadFileAsync::Create(s_previewEnvironmentResourceName);
            if (replacement) {
                Del77E8* old = (*(Del77E8**)&AsCExhibition(base)->preloadFile);
                if (old) {
                    delete old;
                    (*(Del77E8**)&AsCExhibition(base)->preloadFile) = 0;
                }
                (*(void**)&AsCExhibition(base)->preloadFile) = replacement;
            }
        }
        (*(unsigned int*)&AsCExhibition(base)->itemTypeId) = itemTypeId;
        AsCExhibition(base)->avatarResourceOptions = 0;
        Memory_Clear(base + kPreviewResourceOptionsOffset, 0x14);
        AsCExhibition(base)->textureVariantIndex = 0;
        AsCExhibition(base)->maxAnimationDelay = kFloat_0_exhibition;

        // Phase 2: resolve the IFF kind into copied model-path/animation strings.
        switch (((*(unsigned int*)&AsCExhibition(base)->itemTypeId)) >> kItemKindShift) {
        case kItemCaddieEquipment: {
            AsCExhibition(base)->modelKind = kPreviewCaddie;
            IFF_STRUCT::sCadItem* path = ItemInventory_FindCaddieItemData(
                AsCSharedDoc(g_sharedDoc)->itemInventory, (void*)(*(unsigned int*)&AsCExhibition(base)->itemTypeId));
            if (0 == path)
                return;
            REPLACE_PREVIEW_STRING_RANGE(0x518, path->model, path->model + strlen(path->model));
            ASSIGN_PREVIEW_ANIMATION(0x4fc, s_previewCaddieAnimationName);
            break;
        }
        case kItemCaddie: {
            AsCExhibition(base)->modelKind = kPreviewCaddie;
            IFF_STRUCT::sCaddie* path = ItemInventory_FindCaddieData(
                AsCSharedDoc(g_sharedDoc)->itemInventory, (*(unsigned int*)&AsCExhibition(base)->itemTypeId));
            if (path == 0)
                return;
            REPLACE_PREVIEW_STRING_RANGE(0x518, path->model, path->model + strlen(path->model));
            APPEND_PREVIEW_MODEL_SUFFIX(0x518, exhibitionResourceStrings);
            ASSIGN_PREVIEW_ANIMATION(0x4fc, s_previewCaddieAnimationName);
            break;
        }
        case kItemBall: {
            AsCExhibition(base)->modelKind = kPreviewBall;
            IFF_STRUCT::sBall* path = (IFF_STRUCT::sBall*)ItemInventory_FindBallData(
                AsCSharedDoc(g_sharedDoc)->itemInventory, AsCExhibition(base)->itemTypeId);
            if (0 == path)
                return;
            REPLACE_PREVIEW_STRING_RANGE(0x518, path->model, path->model + strlen(path->model));
            APPEND_PREVIEW_MODEL_SUFFIX(0x518, exhibitionResourceStrings);
            break;
        }
        case kItemClubSet: {
            AsCExhibition(base)->modelKind = kPreviewClubSet;
            IFF_STRUCT::sClubSet* path = ItemInventory_FindClubSetData(
                AsCSharedDoc(g_sharedDoc)->itemInventory, (*(unsigned int*)&AsCExhibition(base)->itemTypeId));
            if (!path)
                return;
            REPLACE_PREVIEW_STRING_RANGE(0x518, path->common.icon, path->common.icon + strlen(path->common.icon));
            APPEND_PREVIEW_MODEL_SUFFIX(0x518, (exhibitionResourceStrings + kExhibitionStringPool_text_5_ui_pet));
            ASSIGN_PREVIEW_ANIMATION(0x4fc, s_previewClubSetAnimationName);
            AsCExhibition(base)->animationMode = 1;
            break;
        }
        default:
            return;
        }
        // Phase 3: rebuild camera/probe alignment and the pose, then queue state 1.
        PreviewVector3 init = {kFloat_1_Intro, kFloat_1_Intro, kFloat_0_exhibition};
        PreviewCameraMatrix cam;
        Matrix_SetIdentityAffine(&cam);
        cam.origin.z = AsCExhibition(base)->cameraOffsetZ;
        Camera_SetFov(g_view, kExhibitionCameraFovRadians);
        PreviewCameraMatrix copy = cam;

        PreviewCameraCopyOrder sw = MakeCameraCopyInAbiOrder((copy.basisX), cam);
        PreviewCameraMatrix view = copy;
        *(WVector*)&view *= kFloat_neg1_Exhibition;
        *(WVector*)(&view.basisZ) *= kFloat_neg1_Exhibition;
        WView_SetViewMatrix(g_view, &view);
        Camera_BuildFrustumPlanes(g_view);

        Camera_Apply(g_view, kFloat_0_exhibition, kFloat_0_exhibition);
        Matrix_TransformPoint(&transformed, &init, ((WView*)g_view)->inputTransform);
        float frustum[14];
        WView_ProjectViewPointWithoutClipTest(g_view, frustum, &transformed);
        float viewportHeight = ((WView*)g_view)->viewportHeight;
        AsCExhibition(base)->translationY =
            PreviewBottomTranslation(AsCExhibition(base)->viewportBottom, viewportHeight, frustum[1]);
        if (*(int*)(base + kPreviewModelKindOffset) == kPreviewClubSet) {
            Mtx44_SetEulerRotationZXY(rotation, AsCExhibition(base)->rotationY, kFloat_0_exhibition,
                                      kFloat_0_exhibition);
            Mtx44_SetEulerRotationZXY(matrix, kFloat_0_exhibition, AsCExhibition(base)->rotationX, kFloat_0_exhibition);
            PSMTX44Concat(matrix, rotation, matrix);
        } else
            Mtx44_SetEulerRotationZXY(matrix, AsCExhibition(base)->rotationY, AsCExhibition(base)->rotationX,
                                      AsCExhibition(base)->rotationZ);
        Mtx44_SetTranslation(translationMatrix, AsCExhibition(base)->translationX, AsCExhibition(base)->translationY,
                             kFloat_0_exhibition);
        PSMTX44Concat(combined, matrix, translationMatrix);
        Matrix_ExtractAffineFrom4x4(combined, base + 0x78);
        UpdateModelPose();
        AsCExhibition(base)->loadState = kPreviewLoadModel;
    }
}

// Clear the control flag at +0x10.
int CExhibition::ClearControlFlag(int) {
    unk_10 = 0;
    return 1;
}

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
// Set the permitted X rotation range, clamp the current angle, and update the model transform.
int CExhibition::SetRotationLimits(float minimumAngle, float maximumAngle) {
    // Phase 1: order limits, clamp X, and wrap Y using the original repeated steps.
    // Angles use the units of Mtx44_SetEulerRotationZXY and are used as given.
    if (minimumAngle > maximumAngle) {
        float temp = minimumAngle;
        minimumAngle = maximumAngle;
        maximumAngle = temp;
    }
    float currentX;
    float currentY;
    float currentZ;
    float clampedX;
    float maximumX;
    minRotationX = minimumAngle;
    maxRotationX = maximumAngle;
    clampedX = (float)(double)minimumAngle;
    maximumX = (float)(double)maximumAngle;
    currentX = rotationX;
    currentZ = rotationZ;
    currentY = rotationY;
    clampedX = (currentX < clampedX) ? clampedX : (currentX > maximumX) ? maximumX : currentX;
    rotationX = clampedX;
    rotationY = currentY;
    if (kPreviewRotationYUpperBound < currentY) {
        float zero, step;
        step = kPreviewRotationYWrapStep;
        zero = kFloat_0_exhibition;
        while (zero < rotationY) {
            rotationY = rotationY - step;
        }
    } else if (kPreviewRotationYLowerBound > currentY) {
        float zero, step;
        step = kPreviewRotationYWrapStep;
        zero = kFloat_0_exhibition;
        while (zero > rotationY) {
            rotationY = rotationY + step;
        }
    }
    float clampedZ;
    if (currentZ < kFloat_0_exhibition) {
        clampedZ = kFloat_0_exhibition;
    } else {
        if (currentZ > kFloat_0_exhibition) {
            clampedZ = kFloat_0_exhibition;
        } else {
            clampedZ = currentZ;
        }
    }
    rotationZ = clampedZ;
    // Phase 2: compose rotation with translation and publish the puppet pose.
    // The Z comparisons above clamp every finite nonzero value to zero.
    char combinedTransform[0x40];
    char translationMatrix[0x40];
    char clubRotation[0x40];
    char rotationMatrix[0x40];
    if (modelKind == kPreviewClubSet) {
        Mtx44_SetEulerRotationZXY(clubRotation, rotationY, kFloat_0_exhibition, kFloat_0_exhibition);
        Mtx44_SetEulerRotationZXY(rotationMatrix, kFloat_0_exhibition, rotationX, kFloat_0_exhibition);
        PSMTX44Concat(rotationMatrix, clubRotation, rotationMatrix);
    } else {
        Mtx44_SetEulerRotationZXY(rotationMatrix, rotationY, rotationX, (float)(double)clampedZ);
    }
    Mtx44_SetTranslation(translationMatrix, translationX, translationY, kFloat_0_exhibition);
    PSMTX44Concat(combinedTransform, rotationMatrix, translationMatrix);
    Matrix_ExtractAffineFrom4x4(combinedTransform, (char*)modelTransform);
    UpdateModelPose();
    return 1;
}
#pragma pop

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
// Set preview Euler angles, clamp X to its limits, and update the model transform.
int CExhibition::SetRotation(const float* rotationAngles) {
    // rotationAngles borrows X/Y/Z in the model rotation API's native units.
    // Phase 1: clamp X, wrap Y, and clamp finite nonzero Z to zero.
    float x, y, low, z, high;
    x = rotationAngles[0];
    z = rotationAngles[2];
    y = rotationAngles[1];
    low = minRotationX;
    high = maxRotationX;
    float clamped;
    clamped = (x < low) ? low : (x > high) ? high : x;
    rotationX = clamped;
    rotationY = y;
    if (kPreviewRotationYUpperBound < y) {
        float zero, step;
        step = kPreviewRotationYWrapStep;
        zero = kFloat_0_exhibition;
        while (zero < rotationY) {
            rotationY = rotationY - step;
        }
    } else if (kPreviewRotationYLowerBound > y) {
        float zero, step;
        step = kPreviewRotationYWrapStep;
        zero = kFloat_0_exhibition;
        while (zero > rotationY) {
            rotationY = rotationY + step;
        }
    }
    float zclamp;
    if (z < kFloat_0_exhibition)
        zclamp = kFloat_0_exhibition;
    else if (z > kFloat_0_exhibition)
        zclamp = kFloat_0_exhibition;
    else
        zclamp = z;
    rotationZ = zclamp;

    // Phase 2: compose the club's split rotation (or normal Euler rotation)
    // with translation, then copy the affine result and update the puppet.
    char m3[0x40];
    char m2[0x40];
    char m1[0x40];
    char m0[0x40];
    if (modelKind == kPreviewClubSet) {
        Mtx44_SetEulerRotationZXY(m2, rotationY, kFloat_0_exhibition, kFloat_0_exhibition);
        Mtx44_SetEulerRotationZXY(m3, kFloat_0_exhibition, rotationX, kFloat_0_exhibition);
        PSMTX44Concat(m3, m2, m3);
    } else {
        Mtx44_SetEulerRotationZXY(m3, rotationY, rotationX, (float)(double)zclamp);
    }
    Mtx44_SetTranslation(m1, translationX, translationY, kFloat_0_exhibition);
    PSMTX44Concat(m0, m3, m1);
    Matrix_ExtractAffineFrom4x4(m0, (char*)modelTransform);
    UpdateModelPose();
    return 1;
}
#pragma pop

// Clamp the requested preview camera distance and rebuild projection/model alignment.
int CExhibition::SetCameraDistance(float cameraDistance) {
    float matrix[16];
    float rotation[16];
    char* base = (char*)this;
    float low = AsCExhibition(base)->minCameraDistance;
    float high = AsCExhibition(base)->maxCameraDistance;
    double clamped = cameraDistance < low ? low : (cameraDistance > high ? high : cameraDistance);
    AsCExhibition(base)->cameraOffsetZ = clamped;
    V825C init = {kFloat_1_Intro, kFloat_1_Intro, kFloat_0_exhibition};
    AsCExhibition(base)->cameraDistance = (float)(double)clamped;
    DistanceCameraMatrix cam;
    Matrix_SetIdentityAffine(&cam);
    cam.origin.z = AsCExhibition(base)->cameraOffsetZ;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    DistanceCameraMatrix copy = cam;
    DistanceCameraCopyOrder sw = MakeDistanceCameraCopyInAbiOrder(copy.basisX, cam);
    DistanceCameraMatrix view = copy;
    *(WVector*)&view *= kFloat_neg1_Exhibition;
    *(WVector*)(&view.basisZ) *= kFloat_neg1_Exhibition;
    WView_SetViewMatrix(g_view, &view);
    V825C transformed;
    Camera_BuildFrustumPlanes(g_view);
    Camera_Apply(g_view, kFloat_0_exhibition, kFloat_0_exhibition);
    Matrix_TransformPoint(&transformed, &init, ((WView*)g_view)->inputTransform);
    float frustum[14];
    WView_ProjectViewPointWithoutClipTest(g_view, frustum, &transformed);
    float h = ((WView*)g_view)->viewportHeight;
    AsCExhibition(base)->translationY = PreviewBottomTranslation(AsCExhibition(base)->viewportBottom, h, frustum[1]);
    float combined[16];
    float translationMatrix[16];
    BuildAndApplyPreviewPose(this, rotation, matrix, translationMatrix, combined);
    return 1;
}

// Set the camera distance range and clamp/reapply the current distance.
int CExhibition::SetCameraDistanceLimits(float minimumDistance, float maximumDistance) {
    float frustum[14];
    PreviewCameraMatrix cam;
    PreviewCameraMatrix view;
    float combined[16], translationMatrix[16], rotation[16], matrix[16];
    PreviewVector3 transformed;
    char* base = (char*)this;
    if (minimumDistance > maximumDistance) {
        float t = minimumDistance;
        minimumDistance = maximumDistance;
        maximumDistance = t;
    }
    float x = AsCExhibition(base)->cameraOffsetZ;
    AsCExhibition(base)->minCameraDistance = minimumDistance;
    AsCExhibition(base)->maxCameraDistance = maximumDistance;
    double low = AsCExhibition(base)->minCameraDistance, high = AsCExhibition(base)->maxCameraDistance;
    double clamped = x < low ? low : (x > high ? high : x);
    AsCExhibition(base)->cameraOffsetZ = clamped;
    PreviewVector3 init = {kFloat_1_Intro, kFloat_1_Intro, kFloat_0_exhibition};
    AsCExhibition(base)->cameraDistance = (float)clamped;
    Matrix_SetIdentityAffine(&cam);
    cam.origin.z = AsCExhibition(base)->cameraOffsetZ;
    Camera_SetFov(g_view, kExhibitionCameraFovRadians);
    PreviewCameraMatrix copy = cam;
    PreviewVector3 ca = copy.basisX;
    PreviewCameraCopyOrder sw = MakeCameraCopyInAbiOrder(ca, cam);
    view = copy;
    *(WVector*)&view *= kFloat_neg1_Exhibition;
    *(WVector*)(&view.basisZ) *= kFloat_neg1_Exhibition;
    WView_SetViewMatrix(g_view, &view);
    Camera_BuildFrustumPlanes(g_view);
    Camera_Apply(g_view, kFloat_0_exhibition, kFloat_0_exhibition);
    Matrix_TransformPoint(&transformed, &init, ((WView*)g_view)->inputTransform);
    WView_ProjectViewPointWithoutClipTest(g_view, frustum, &transformed);
    float viewportHeight = ((WView*)g_view)->viewportHeight;
    AsCExhibition(base)->translationY =
        PreviewBottomTranslation(AsCExhibition(base)->viewportBottom, viewportHeight, frustum[1]);
    BuildAndApplyPreviewPose(this, rotation, matrix, translationMatrix, combined);
    return 1;
}

// Return the writable three-float Euler rotation stored by the preview.
void* CExhibition::GetRotation() {
    return &rotationX;
}

// Return the preview camera's Z offset, in the same units as the distance limits.
float CExhibition::GetCameraDistance() {
    return cameraOffsetZ;
}

// Report whether the preview can accept another model-change request.
int CExhibition::IsReady() {
    return ExhibitionIsReady(this);
}

#pragma push
#pragma use_lmw_stmw on
// Queue selected and resolved avatar descriptions plus a part-reload mask after readiness checks.
// Copies 0x68 selected bytes and 0xC8 resolved bytes; both inputs must be readable.
// partReloadMask carries a nonzero word, copied without dereferencing it here.
// Returns 0 without changes when busy, controller-less or given a zero mask;
// returns 1 after scheduling state 2 (completion occurs in UpdateLoadState).
int CExhibition::QueueModelUpdate(void* selectedDescription, void* resolvedDescription, int reloadMask) {
    int valid = 1;
    void* controller = (*(void**)&preloadFile);
    if (controller && !((Sub802B87F4*)controller)->m2())
        valid = 0;
    if (valid) {
        unsigned int value = (*(unsigned int*)&loadState) - 1;
        unsigned int shift = __cntlzw(value ^ 2);
        valid = (value << shift) >> 31;
    } else {
        valid = 0;
    }
    if (!valid)
        return 0;
    if (!modelController)
        return 0;
    if (!reloadMask)
        return 0;

    (*(ResolvedAvatarRecordCopy*)&resolvedCharacterId) = *(ResolvedAvatarRecordCopy*)resolvedDescription;
    partReloadMask = reloadMask;
    (*(AvatarSelectionRecordCopy*)&avatarCharacterId) = *(AvatarSelectionRecordCopy*)selectedDescription;
    loadState = kPreviewReloadParts;
    return 1;
}
#pragma pop

// Apply an avatar texture variant and synchronize matching-character selection state on success.
// Return 1 means the request was attempted, even if AvatarModel_ApplyTextureVariant
// failed; return 0 means no texture/controller was supplied.
int CExhibition::ApplyTextureVariant(void* textureVariant, int variantIndex) {
    if (textureVariant == 0)
        return 0;
    void* obj = modelController;
    if (obj == 0)
        return 0;
    if (AvatarModel_ApplyTextureVariant(obj, textureVariant, variantIndex)) {
        ItemSelection_SetStateIfSameCharacter(((char*)(&resolvedCharacterId)), ((char*)(&avatarCharacterId)),
                                              variantIndex);
    }
    return 1;
}

// Copy the avatar controller's resolved character/part description into caller storage.
int CExhibition::CopyResolvedAvatarSelection(void* out) {
    void* obj = modelController;
    if (obj == 0)
        return 0;
    struct ResolvedAvatar {
        char controllerState[0x1bc];
        ResolvedAvatarSelectionCopy selection;
    };
    *(ResolvedAvatarSelectionCopy*)out = ((ResolvedAvatar*)obj)->selection;
    return 1;
}

// Store the inverse visibility flag used to suppress drawing and animation updates.
// The store goes through a signed char view; the original compiles this way.
int CExhibition::SetVisible(int value) {
    unsigned int _c = __cntlzw(value);
    (*(signed char*)&displaySuppressed) = _c >> 5;
    return 1;
}

// Forward (callback, context) to slot 0xC of the embedded preview callback object.
int CExhibition::GetCallbackResult(int callbackAddress, void* callbackContext) {
    return ((Sub802B8AD8*)((char*)this + kPreviewCallbackOffset))->m3(callbackAddress, callbackContext);
}

// Detach the model preloader without destroying it.
int CExhibition::ClearModelPreloader() {
    modelPreloader = 0;
    return 1;
}

// Install a caller-supplied model preloader handle.
int CExhibition::SetModelPreloader(int value) {
    modelPreloader = value;
    return 1;
}

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
// Create the first Ball.iff effect and attach it to its named bone or the center fallback.
void CExhibition::CreateBallEffects() {
    // Phase 1: resolve the first effect filename and its live bone transform.
    // Only slot zero is used; there is no loop over the remaining Ball.iff effects.
    char* condptr;
    char* base = (char*)this;
    // Partial stack attachment view: +8/+24 begin the two four-pointer copy spans.
    // The declared view does not describe every copied stack word; preserve the
    // initializer/copy lengths until the complete attachment ABI is established.
    struct LocalData {
        float position[2];
        void* first;
        unsigned char pad1[0xc];
        void* second;
    } local;
    IFF_STRUCT::sBall* data = (IFF_STRUCT::sBall*)ItemInventory_FindBallData(AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                                             AsCExhibition(base)->itemTypeId);
    if (!data)
        return;
    int len;
    len = strlen(data->effectFiles[0]);
    if (len < kMinimumEffectFilenameLength)
        return;
    FxAttachment_Init(local.position);
    void* candidate = 0;
    if (*(signed char*)(data->effectBones[0]))
        candidate = (void*)WPuppet_FindOrCacheBone(AsCExhibition(base)->puppet, data->effectBones[0]);
    if (candidate) {
        local.first = ((PreviewBoneTransform*)candidate)->attachmentMatrix;
    } else {
        void* found;
        if (*(signed char*)(data->effectBones[0]))
            found = (void*)WPuppet_FindNamedBoneEntry(AsCExhibition(base)->puppet, data->effectBones[0]);
        else
            found = (void*)WPuppet_FindNamedBoneEntry(AsCExhibition(base)->puppet,
                                                      exhibitionResourceStrings + kExhibitionStringPool_text_58_center);
        if (found) {
            void* first = *(void**)found;
            if (first)
                local.first = ((PreviewBoneTransform*)first)->attachmentMatrix;
            local.second = ((PreviewNamedBone*)found)->metadata->position;
        }
    }
    // Phase 2: copy the preview's pixel anchor into a spray or sequence and
    // register it in CFxBox. Bone transform pointers remain borrowed.
    // The effect receives copied screen coordinates and borrowed live bone pointers.
    // sourceAttachment also receives this stack address. It expires on return:
    // do not treat that stored field as an owned or persistent attachment object.
    local.position[0] = AsCExhibition(base)->screenCenterX;
    local.position[1] = AsCExhibition(base)->screenCenterY;
    // Last three filename bytes identify the extension; preserve case-sensitive comparison.
    condptr = data->effectFiles[0] + len - 3;
    if (CString_Compare(condptr, exhibitionResourceStrings + kExhibitionStringPool_text_5f_spr) == 0) {
        void* item = (void*)CFxSpray::CreateFromScript((CFxBox*)(((char*)(AsCExhibition(base)->fxBox))),
                                                       (const char*)(data->effectFiles[0]), 0, 0);
        if (item) {
            ((PreviewSprayAttachment*)item)->sourceAttachment = local.position;
            ((PreviewSprayAttachment*)item)->screenX = local.position[0];
            ((PreviewSprayAttachment*)item)->screenY = local.position[1];
            memcpy(((PreviewSprayAttachment*)item)->transforms, &local.first, 0x10);
            memcpy(((PreviewSprayAttachment*)item)->positions, &local.second, 0x10);
            ((CFxBox*)(((char*)(AsCExhibition(base)->fxBox))))->RegisterSpray(((CFxSpray*)(item)));
        }
    } else {
        void* item = (void*)CFxSequence_Create(((char*)(AsCExhibition(base)->fxBox)), data->effectFiles[0], 1, 0, 0);
        if (item) {
            ((PreviewSequenceAttachment*)item)->sourceAttachment = local.position;
            ((PreviewSequenceAttachment*)item)->screenX = local.position[0];
            ((PreviewSequenceAttachment*)item)->screenY = local.position[1];
            ((CFxBox*)(((char*)(AsCExhibition(base)->fxBox))))->RegisterSequence(((CFxSequence*)(item)));
        }
    }
}
#pragma pop

// Update all registered spray/sequence screen origins and rebuild bone-attached effects.
int CExhibition::UpdateEffectScreenCenter() {
    // Phase 1: update registered sprays in tree order. Both trees embed their
    // sentinel at the owner; parent links carry a low color bit to mask off.
    register void* node;
    register void* next;
    register void* child;
    node = sprayTreeFirst;
    register void* head = ((PreviewEffectTreeHeaders*)this)->spraySentinel;
    int centerX = (int)screenCenterX;
    int centerY = (int)screenCenterY;
    while (node != head) {
        child = ((PreviewEffectTreeNode*)node)->effect;
        if (child) {
            ((PreviewSprayAttachment*)child)->screenX = (float)centerX;
            next = ((PreviewEffectTreeNode*)node)->effect;
            ((PreviewSprayAttachment*)next)->screenY = (float)centerY;
        }
        next = *(void**)((char*)node + 4);
        if (next) {
            while (*(void**)next)
                next = *(void**)next;
            node = next;
            continue;
        } else {
            while (node != *(void**)(next = (void*)((unsigned int)*(void**)((char*)node + 8) & ~1u)))
                node = next;
            node = next;
        }
    }
    // Phase 2: update registered sequences using their different attachment offsets.
    head = ((PreviewEffectTreeHeaders*)this)->sequenceSentinel;
    node = sequenceTreeFirst;
    while (node != head) {
        child = ((PreviewEffectTreeNode*)node)->effect;
        if (child) {
            ((PreviewSequenceAttachment*)child)->screenX = (float)centerX;
            next = ((PreviewEffectTreeNode*)node)->effect;
            ((PreviewSequenceAttachment*)next)->screenY = (float)centerY;
        }
        next = *(void**)((char*)node + 4);
        if (next) {
            while (*(void**)next)
                next = *(void**)next;
            node = next;
            continue;
        } else {
            while (node != *(void**)(next = (void*)((unsigned int)*(void**)((char*)node + 8) & ~1u)))
                node = next;
            node = next;
        }
    }
    RebuildAttachedEffects();
    return 1;
}

// Apply the current animation time and transform according to avatar, caddie, ball, or club-set kind.
void CExhibition::UpdateModelPose() {
    if (!puppet)
        return;
    int mode = modelKind;
    switch (mode) {
    case kPreviewAvatar: {
        void* obj = (void*)ObjectList_GetSelected(modelController);
        if (!obj)
            return;
        WPuppet_SetAnimationTimeFromRanges(obj, 0, animationFrame, 0, 0);
        ResourceModel_SetMatrixRecursive(modelController, obj, ((char*)(modelTransform)));
        ResourceModel_UpdateBoneWorldMatrices(modelController, 0, kFloat_1_Intro, 0);
        break;
    }
    case kPreviewCaddie: {
        WPuppet_SetAnimationTimeFromRanges(puppet, 0, animationFrame, 0, 0);
        Model_SetMatrixRecursive(puppet, puppet, ((char*)(modelTransform)));
        WPuppet_UpdateBoneWorldMatrices(puppet, 0, 0, kFloat_1_Intro);
        break;
    }
    case kPreviewBall:
    case kPreviewStaticModel:
        Model_SetMatrixRecursive(puppet, 0, ((char*)(modelTransform)));
        break;
    case kPreviewClubSet: {
        WPuppet_SetAnimationTimeFromRanges(puppet, 0, animationFrame, 0, 0);
        void* p = puppet;
        char matrix[0x30];
        char rotation[0x30];
        Matrix_SetAxisRotation(rotation, kClubPreviewAxisRotation, 2);
        Matrix_ConcatAffine(matrix, ((char*)(modelTransform)), rotation);
        Model_SetMatrixRecursive(p, p, matrix);
        break;
    }
    default:
        break;
    }
    WPuppet_UpdateBoneWorldMatrices(puppet, 0, 0, kFloat_1_Intro);
}

#pragma push
#pragma use_lmw_stmw on
#pragma inline_depth(16)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Track a registered effect pointer, growing pointer storage without retaining the effect.
inline void AppendEffect802B(PtrVec802B* v, void* const& item) {
    // Phase 1: append directly if possible; otherwise guard a replacement buffer.
    unsigned needed, smallGrowth, largeGrowth;
    char errorStorage2[12], errorStorage3[12], errorStorage1[12];
    unsigned size = v->size;
    unsigned limit = v->capacity;
    if (size < limit) {
        unsigned next = size + 1;
        v->size = next;
        v->data[next - 1] = item;
        return;
    }
    if (kMaxPointerElements - limit < 1) {
        std::length_error* error =
            MakeError802B(errorStorage1, exhibitionResourceStrings + kExhibitionStringPool_text_d_vector_length_error);
        __throw(exhibitionResourceStrings +
                    kExhibitionStringPool_text_21_std_exception_std_logic_error_std_length_error,
                error, (void*)__dt__Q23std12length_errorFv);
    }
    needed = v->size + 1 - v->capacity;
    VecBuffer802B temp(&v->capacity);
    unsigned capacity = v->capacity;
    if (needed > kMaxPointerElements - capacity) {
        std::length_error* error =
            MakeError802B(errorStorage2, exhibitionResourceStrings + kExhibitionStringPool_text_d_vector_length_error);
        __throw(exhibitionResourceStrings +
                    kExhibitionStringPool_text_21_std_exception_std_logic_error_std_length_error,
                error, (void*)__dt__Q23std12length_errorFv);
    }
    if (capacity < kPointerGrowthThreeFifthsLimit)
        capacity += VecMax802B(largeGrowth = (3 * (capacity + 1)) / 5, needed);
    else if (capacity < kPointerGrowthHalfLimit)
        capacity += VecMax802B(smallGrowth = (capacity + 1) / 2, needed);
    else
        capacity = kMaxPointerElements;
    if (capacity > kMaxPointerElements) {
        std::length_error* error =
            MakeError802B(errorStorage3, exhibitionResourceStrings + kExhibitionStringPool_text_d_vector_length_error);
        __throw(exhibitionResourceStrings +
                    kExhibitionStringPool_text_21_std_exception_std_logic_error_std_length_error,
                error, (void*)__dt__Q23std12length_errorFv);
    }
    // Phase 2: construct the new tail, copy existing pointers and clear old slots.
    void** buf = (void**)appMallocDummy__FUl(capacity * 4);
    int oldSize = v->size;
    temp.offset = oldSize;
    temp.data = buf;
    (buf + oldSize)[temp.size++] = item;
    void** old = v->data;
    int n = ((char*)(old + v->size) - (char*)old) / 4;
    temp.capacity = capacity;
    temp.offset = oldSize - n;
    memcpy(buf + temp.offset, old, n * 4);
    memset(old, 0, n * 4);
    unsigned newSize = temp.size + n;
    temp.size = newSize;
    // Phase 3: commit the replacement; temp frees the retired pointer allocation.
    void** oldData = v->data;
    void** newData = temp.data;
    unsigned oldCapacity = v->capacity;
    unsigned newCapacity = temp.capacity;
    v->size = 0;
    temp.data = oldData;
    v->capacity = newCapacity;
    temp.capacity = oldCapacity;
    v->data = newData;
    v->size = newSize;
    temp.size = 0;
}

// Bone-effect script contract (marker bytes come from the shared *fx string):
//   *fx filename.spr                     non-avatar, space-delimited filename
//   *fx("filename.seq" "always")         avatar, unconditional non-& token
//   *fx("filename.spr" "&123" "&456")   avatar, alternative part IDs
// '(' must immediately follow the marker to choose quoted filename parsing.
// The quoted parser skips leading spaces/tabs, has no escapes, and stops at '"'.
// The unquoted parser skips leading spaces/tabs but only space ends its token.
// This routine neither requires nor consumes a closing ')'.
// Names must be nonempty and shorter than 64 bytes, with a dot and an extension
// suffix at least four bytes including the dot. Only exact "spr"/"seq" create.
// Avatar entries additionally consume a quoted condition token. Consecutive
// "&<decimal part ID>" tokens are alternatives: the first match in resolved
// parts OR attachment IDs admits creation. If any '&' occurred, at least one
// must match. A non-& token without prior '&' imposes no part restriction.
// Non-avatar entries do not consume conditions. Bones with flag 0x40000 skip.
// Rebuild removes the previously tracked sprays/sequences before parsing new
// entries, keeps tracking-vector capacity and leaves unrelated box effects alone.
// Token parsers require valid source pointers and enough output capacity; the
// length tests run after copying. Scripts must obey the 63-byte token limit.
// A failed effect creation omits that effect; earlier removals have no rollback.
// CFxBox registers effects; tracking vectors own only their pointer arrays.
// Matrices/points borrow the puppet. The sourceAttachment field receives the
// address of stack scratch exactly as before; it is not a retained valid owner.
// Do not add consumers that dereference it after return without a lifetime fix.
// Replace bone-script effects, respecting part conditions and tracking newly created sprays/sequences.
void CExhibition::RebuildAttachedEffects() {
    // Phase 1: remove the prior bone-generated effects, retaining vector capacity.
    // CFxBox holds registered effects; attachedSprays/Sequences track the subset
    // rebuilt here. Their arrays own pointer storage, not separate effect copies.
    char* effectTokenOrExtension;
    char* base;
    void* namedBoneEntry;
    char* scriptCursor;
    unsigned markerLength;
    int hadPartCondition;
    int partConditionMatched;
    int requiredPartId;
    void* createdSpray;
    base = (char*)this;
    if (!AsCExhibition(base)->puppet)
        return;
    char effectMarker[4];
    effectMarker[0] = kPreviewBoneEffectMarkerByte0;
    effectMarker[1] = kPreviewBoneEffectMarkerByte1;
    effectMarker[2] = kPreviewBoneEffectMarkerByte2;
    effectMarker[3] = kPreviewBoneEffectMarkerByte3;
    for (unsigned i = 0; i < (*(unsigned*)&AsCExhibition(base)->attachedSprayCount); ++i)
        ((CFxBox*)(base + 0xb0))->RemoveSpray(((CFxSpray*)(((*(void***)&AsCExhibition(base)->attachedSprays))[i])));
    for (unsigned i = 0; i < (*(unsigned*)&AsCExhibition(base)->attachedSequenceCount); ++i)
        ((CFxBox*)(base + 0xb0))
            ->RemoveSequence(((CFxSequence*)(((*(void***)&AsCExhibition(base)->attachedSequences))[i])), 1);
    unsigned previousSprayCount = (*(volatile unsigned*)&AsCExhibition(base)->attachedSprayCount);
    unsigned previousSequenceCount = (*(volatile unsigned*)&AsCExhibition(base)->attachedSequenceCount);
    (*(unsigned*)&AsCExhibition(base)->attachedSequenceCount) -= previousSequenceCount;
    (*(unsigned*)&AsCExhibition(base)->attachedSprayCount) -= previousSprayCount;
    // Phase 2: scan bone metadata for effect markers and parse filename/conditions.
    // Avatar '&' conditions match either resolved parts or current attachments.
    for (unsigned i = 0; i < ((WPuppet*)AsCExhibition(base)->puppet)->namedBoneCount; ++i) {
        char name[kPreviewTokenCapacity], condition[kPreviewTokenCapacity];
        PreviewAttachmentDescription description;
        namedBoneEntry = (PreviewNamedBone*)((WPuppet*)AsCExhibition(base)->puppet)->namedBones + i;
        if (!namedBoneEntry)
            continue;
        scriptCursor = ((PreviewNamedBone*)namedBoneEntry)->metadata->script;
        if (!scriptCursor)
            continue;
        if (!*(void**)namedBoneEntry ||
            (*(unsigned*)((char*)*(void**)namedBoneEntry + 0x74) & kBoneSuppressAttachedEffects))
            continue;
        for (;;) {
            effectTokenOrExtension = strstr(scriptCursor, effectMarker);
            if (!effectTokenOrExtension)
                break;
            markerLength = strlen(effectMarker);
            if ((signed char)effectTokenOrExtension[markerLength] == '(')
                ++markerLength;
            if (markerLength > strlen(effectMarker))
                scriptCursor = (char*)String_ParseQuotedToken(effectTokenOrExtension + markerLength, name);
            else
                scriptCursor = String_ParseSpaceDelimitedToken(effectTokenOrExtension + markerLength, name);
            int len = strlen(name);
            if (len == 0 || len >= kPreviewTokenCapacity)
                continue;
            effectTokenOrExtension = strrchr(name, '.');
            if (!effectTokenOrExtension || strlen(effectTokenOrExtension) < kPreviewMinimumExtensionLength)
                continue;
            if (AsCExhibition(base)->modelKind == kPreviewAvatar) {
                scriptCursor = (char*)String_ParseQuotedToken(scriptCursor, condition);
                len = strlen(condition);
                if (len == 0 || len >= kPreviewTokenCapacity)
                    continue;
                hadPartCondition = 0;
                partConditionMatched = 0;
                while (!partConditionMatched && (unsigned char)condition[0] == '&') {
                    hadPartCondition = 1;
                    requiredPartId = String_ParseDecimalInt(condition + 1);
                    char* owner = (*(char**)&AsCExhibition(base)->modelController);
                    if (ResolvedItemSelection_ContainsPart(owner + kPreviewResolvedSelectionOffset,
                                                           (const char*)requiredPartId)) {
                        partConditionMatched = 1;
                        break;
                    } else {
                        if (AvatarModel_ContainsAttachment(AsCExhibition(base)->modelController,
                                                           (const char*)requiredPartId))
                            partConditionMatched = 1;
                        if (partConditionMatched)
                            break;
                        scriptCursor = (char*)String_ParseQuotedToken(scriptCursor, condition);
                        len = strlen(condition);
                        if (len == 0 || len >= kPreviewTokenCapacity)
                            break;
                    }
                }
                if (hadPartCondition && !partConditionMatched)
                    continue;
            }
            // Phase 3: build the bone/screen attachment, register by extension, and
            // retain its pointer in the matching tracking array for the next rebuild.
            // description is stack scratch. Copied matrix/position entries borrow the
            // puppet; sourceAttachment stores &description and is invalid after return.
            // Retained-pointer consumers require a separate lifetime audit before use.
            FxAttachment_Init(&description);
            description.x = AsCExhibition(base)->screenCenterX;
            description.y = AsCExhibition(base)->screenCenterY;
            description.transforms[0] = ((PreviewNamedBone*)namedBoneEntry)->bone->attachmentMatrix;
            description.points[0] = ((PreviewNamedBone*)namedBoneEntry)->metadata->position;
            if (String_CompareCString(effectTokenOrExtension + 1,
                                      exhibitionResourceStrings + kExhibitionStringPool_text_5f_spr) == 0) {
                createdSpray = (void*)CFxSpray::CreateFromScript((CFxBox*)(base + 0xb0), (const char*)(name), 0, 0);
                if (createdSpray) {
                    ((PreviewSprayAttachment*)createdSpray)->sourceAttachment = &description;
                    ((PreviewSprayAttachment*)createdSpray)->screenX = description.x;
                    ((PreviewSprayAttachment*)createdSpray)->screenY = description.y;
                    memcpy(((PreviewSprayAttachment*)createdSpray)->transforms, &description.transforms[0], 0x10);
                    memcpy(((PreviewSprayAttachment*)createdSpray)->positions, &description.points[0], 0x10);
                    ((CFxBox*)(base + 0xb0))->RegisterSpray(((CFxSpray*)(createdSpray)));
                    AppendEffect802B((PtrVec802B*)(base + 0x444), createdSpray);
                }
            } else {
                if (String_CompareCString(effectTokenOrExtension + 1,
                                          exhibitionResourceStrings + kExhibitionStringPool_text_63_seq) == 0) {
                    void* createdSpray = (void*)CFxSequence_Create(base + 0xb0, name, 1, 0, 0);
                    if (createdSpray) {
                        ((PreviewSequenceAttachment*)createdSpray)->sourceAttachment = &description;
                        ((PreviewSequenceAttachment*)createdSpray)->screenX = description.x;
                        ((PreviewSequenceAttachment*)createdSpray)->screenY = description.y;
                        ((CFxBox*)(base + 0xb0))->RegisterSequence(((CFxSequence*)(createdSpray)));
                        AppendEffect802B((PtrVec802B*)(base + 0x450), createdSpray);
                    }
                }
            }
        }
    }
}

#pragma pop

// Load lighting, initialize animation and fade-in, rebuild attachments, and apply the initial pose.
int CExhibition::InitializeLoadedModel() {
    // Requires a loaded puppet. Phase 1: install lighting and refresh identity-pose bounds.
    char* base = (char*)this;
    LoadEnvironmentLights();
    struct Matrices {
        char modelRotation[0x40];
        char clubRotation[0x40];
        char translation[0x40];
        char modelTransform4x4[0x40];
        char identityPose[0x30];
    } matrices;
    Matrix_SetIdentityAffine(matrices.identityPose);
    Model_SetMatrixRecursive(AsCExhibition(base)->puppet, 0, matrices.identityPose);
    WPuppet_UpdateBoneWorldMatrices(AsCExhibition(base)->puppet, 0, 0, kFloat_1_Intro);
    WPuppet_UpdateBound(AsCExhibition(base)->puppet, 0);
    // Phase 2: select the copied animation and initialize its caller-time delay/cycle.
    unsigned int flag = (unsigned int)__cntlzw(AsCExhibition(base)->animationNameFlags >> 31) >> 5;
    unsigned int value = flag ? ((*(unsigned char*)&AsCExhibition(base)->animationNameFlags) & 0x7f)
                              : (*(unsigned int*)&AsCExhibition(base)->animationNameLength);
    if (value != 0) {
        void* name = flag ? (void*)(base + 0x4fd) : (*(void**)&AsCExhibition(base)->animationNameData);
        PlayAnimationByName(name, AsCExhibition(base)->animationMode, kFloat_0_exhibition);
    }
    float time = Random_RangeFloat(-1, kFloat_0_exhibition, AsCExhibition(base)->maxAnimationDelay);
    AsCExhibition(base)->animationDelay = time;
    void* range = AsCExhibition(base)->animationRange;
    if (range) {
        float difference = *(float*)((char*)range + 4) - *(float*)range;
        AsCExhibition(base)->animationCycleDuration = AsCExhibition(base)->animationDelay + difference;
    } else
        AsCExhibition(base)->animationCycleDuration = kFloat_0_exhibition;
    // Phase 3: begin transparent, rebuild bone effects and apply the preview pose.
    AsCExhibition(base)->opacity = kFloat_0_exhibition;
    WPuppet_SetOpacityRecursive(AsCExhibition(base)->puppet, 0);
    RebuildAttachedEffects();
    BuildAndApplyPreviewPose(this, matrices.clubRotation, matrices.modelRotation, matrices.translation,
                             matrices.modelTransform4x4);
    ((Sub802B8AD8*)(base + kPreviewCallbackOffset))->m4(0);
    return 1;
}

struct PreviewAnimationTimingView {
    float startFrame, endFrame;
    char unknown08[0x14];
    void* replayName; // Borrowed +0x1c argument to PlayAnimationByName; distinct from the +0x18 name view.
};
typedef char PreviewAnimationTiming_name_offset[((int)&((PreviewAnimationTimingView*)0)->replayName == 0x1c) ? 1 : -1];
enum {
    kPreviewEffectsSubobjectOffset = 0xb0
};

#pragma push
#pragma always_inline on
// Borrow the selected animation puppet for avatars, or the direct preview puppet otherwise.
static inline void* PreviewAnimationEventModel(char* preview) {
    return fn_802B9CD4_target(preview);
}
#pragma pop

#pragma push
#pragma always_inline on
// Advance the embedded effect box virtual hook, then its effects, using the same caller-time scale.
static inline void AdvancePreviewEffectSimulation(char* preview, float dt) {
    ((Sub802B9CD4*)(preview + kPreviewEffectsSubobjectOffset))->m1(kPreviewFadeAndEffectStepScale * dt);
    ((CFxBox*)(preview + kPreviewEffectsSubobjectOffset))->UpdateEffects(kPreviewFadeAndEffectStepScale * dt);
}
#pragma pop

// Advance preview opacity, animation delay/frame events, pose, and effect simulation.
int CExhibition::UpdateAnimationAndEffects(float dt) {
    // Phase 1: fade opacity in the byte range [0,255]. dt also advances the
    // animation frame and delay below; it is in caller-supplied animation units.
    char* base = (char*)this;
    if (!AsCExhibition(base)->puppet)
        return 0;
    if (AsCExhibition(base)->displaySuppressed)
        return 1;
    float maxProgress;
    float progress = AsCExhibition(base)->opacity;
    maxProgress = kPreviewMaximumOpacity;
    if (progress < maxProgress) {
        progress = kPreviewFadeAndEffectStepScale * dt + progress;
        AsCExhibition(base)->opacity = progress;
        if (progress > maxProgress)
            AsCExhibition(base)->opacity = maxProgress;
        progress = AsCExhibition(base)->opacity;
        int opacityByte = (int)progress;
        if (AsCExhibition(base)->modelKind == kPreviewAvatar) {
            void* ptr = AsCExhibition(base)->modelController;
            if (ptr)
                PuppetOwner_SetOpacityRecursive(ptr, (unsigned char)opacityByte);
        } else {
            void* ptr = AsCExhibition(base)->puppet;
            if (ptr)
                WPuppet_SetOpacityRecursive(ptr, (unsigned char)opacityByte);
        }
    }
    // Phase 2: while the animation delay remains positive, effects also wait.
    float zero = kFloat_0_exhibition;
    float remaining = AsCExhibition(base)->animationDelay;
    remaining -= dt;
    AsCExhibition(base)->animationDelay = remaining;
    if (remaining > zero)
        return 1;
    // Phase 3: process crossed frame events, loop animation and update pose/effects.
    void* animation = AsCExhibition(base)->animationRange;
    if (animation) {
        float current = AsCExhibition(base)->animationFrame;
        float next = current + dt;
        AnimationEventParser_ProcessFrameRange(this, PreviewAnimationEventModel(base), current, next, animation, -1, 0);
        float elapsed = AsCExhibition(base)->animationFrame + dt;
        AsCExhibition(base)->animationFrame = elapsed;
        PreviewAnimationTimingView* currentAnimation = (PreviewAnimationTimingView*)AsCExhibition(base)->animationRange;
        if (elapsed > currentAnimation->endFrame)
            PlayAnimationByName(currentAnimation->replayName, 1, elapsed - currentAnimation->endFrame);
        else
            UpdateModelPose();
    } else
        UpdateModelPose();
    AdvancePreviewEffectSimulation(base, dt);
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Load environment lighting and retain at most three hardware-light descriptions for the preview.
int CExhibition::LoadEnvironmentLights() {
    // Phase 1: read the environment file into temporary bytes, then replace the
    // retained parsed resource. Failure does not promise to preserve old lights.
    char* base = (char*)this;
    if (g_resourceManager == 0)
        return 0;
    File802B* file;
    void* data = 0;
    void* resource = 0;
    int ok = 0;
    file = (File802B*)((int (*)(void*, void*, unsigned int, int, int, int))Resource_OpenResolvedPath)(
        g_resourceManager, s_previewEnvironmentResourceName, 0xffff, 0, 0, 0);
    if (file) {
        if (file->size > 0) {

            data = (void*)__nwa__FUl(file->size);
            file->Read(data, file->size);
            RefObj802B* p = (*(RefObj802B**)&AsCExhibition(base)->lightingResource);
            if (p) {
                if (--p->ref == 0)
                    delete p;
                (*(void**)&AsCExhibition(base)->lightingResource) = 0;
            }
            gtl::IGenericEnvironmentResource::CreateFromMemory(data, file->size, (void**)(base + 0x528));
            unsigned int count;
            ((gtl::IGenericEnvironmentResource*)((*(void**)&AsCExhibition(base)->lightingResource)))
                ->GetLightCount((int*)&count);
            if (count) {
                resource = (void*)__nwa__FUl(count * 0x88);
                if (resource) {
                    gtl_IGenericEnvironmentResource_CopyLights((*(void**)&AsCExhibition(base)->lightingResource),
                                                               resource, count);
                    // Phase 2: replace owned light descriptions with at most three
                    // parsed lights. Temporary resource records are 0x88 bytes each.
                    PtrVec802B* v = (PtrVec802B*)(base + 0x530);
                    for (void** it = v->data; it != v->data + v->size; ++it) {
                        if (*it)
                            __dl__FPv(*it);
                        *it = 0;
                    }
                    __typeof__(v->size) savedSize = v->size;
                    v->size = v->size - (savedSize);
                    if (count > kPreviewLightLimit)
                        count = kPreviewLightLimit;
                    char* src = (char*)resource;
                    for (unsigned int i = 0; count > i; src += 0x88, ++i) {
                        void* item = __nw__FUl(0x68);
                        if (item == 0)
                            continue;
                        gtl_GenericLight_GetDescription(item, src);
                        if (!(!(v->size < v->capacity))) {
                            __typeof__(v->size + 1) _p522 = v->size + 1;
                            __typeof__(1 + v->size) _p763 = _p522;
                            __typeof__(v->size + 1) _p345 = _p763;
                            int next = (1 + v->size);
                            v->size = next;
                            v->data[next - 1] = item;
                        } else {
                            if (kMaxPointerElements - v->capacity < 1) {
                                union {
                                    double alignment;
                                    unsigned char bytes[sizeof(std::length_error)];
                                } storage;
                                std::length_error* error = (std::length_error*)storage.bytes;
                                error->__ct(exhibitionResourceStrings +
                                            kExhibitionStringPool_text_d_vector_length_error);
                                exception_runtime::__throw(
                                    exception_runtime::exhibitionResourceStrings +
                                        kExhibitionStringPool_text_21_std_exception_std_logic_error_std_length_error,
                                    error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
                            }

                            VecBuffer802B temp(&v->capacity);
                            unsigned needed = ((v->size + 1)) - v->capacity;
                            unsigned capacity = v->capacity;
                            if (needed > kMaxPointerElements - capacity) {
                                union {
                                    double alignment;
                                    unsigned char bytes[sizeof(std::length_error)];
                                } storage;
                                std::length_error* error = (std::length_error*)storage.bytes;
                                error->__ct(exhibitionResourceStrings +
                                            kExhibitionStringPool_text_d_vector_length_error);
                                exception_runtime::__throw(
                                    exception_runtime::exhibitionResourceStrings +
                                        kExhibitionStringPool_text_21_std_exception_std_logic_error_std_length_error,
                                    error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
                            }
                            if (capacity < kPointerGrowthThreeFifthsLimit)
                                capacity += VecMax802B((3 * (capacity + 1)) / 5, needed);
                            else if (capacity < kPointerGrowthHalfLimit)
                                capacity += VecMax802B((capacity + 1) / 2, needed);
                            else
                                capacity = kMaxPointerElements;
                            if (capacity > kMaxPointerElements) {
                                union {
                                    double alignment;
                                    unsigned char bytes[sizeof(std::length_error)];
                                } storage;
                                std::length_error* error = (std::length_error*)storage.bytes;
                                error->__ct(exhibitionResourceStrings +
                                            kExhibitionStringPool_text_d_vector_length_error);
                                exception_runtime::__throw(
                                    exception_runtime::exhibitionResourceStrings +
                                        kExhibitionStringPool_text_21_std_exception_std_logic_error_std_length_error,
                                    error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
                            }
                            void** buf = (void**)appMallocDummy__FUl((4 * capacity));
                            int oldSize = v->size;

                            temp.offset = oldSize;
                            temp.capacity = capacity;
                            temp.data = buf;
                            ((buf + oldSize))[temp.size++] = item;
                            void** old = v->data;

                            int n = ((char*)(v->size + old) - (char*)old) / 4;
                            temp.offset = oldSize - n;

                            memcpy(temp.offset + buf, old, (4 * n));
                            memset(old, 0, (4 * n));
                            temp.size = (n + temp.size);
                            void** od = v->data;

                            __typeof__(temp.capacity) savedCapacityCopy = temp.capacity;
                            void** nd = temp.data;
                            temp.data = od;

                            __typeof__(v->capacity) savedCapacity = v->capacity;
                            v->size = 0;
                            v->capacity = (savedCapacityCopy);
                            temp.capacity = (savedCapacity);
                            v->size = (temp.size);
                            v->data = nd;
                            temp.size = 0;
                        }
                    }
                    ok = 1;
                }
            }
        }
    }
    // Phase 3: release file/raw/copied-record storage; parsed resource and light
    // descriptions stay owned by the preview. ok=1 means the copy stage ran.
    if (file) {
        if (file)
            Resource_Release(file);
    }
    if (data)
        __dla__FPv(data);
    if (resource)
        __dla__FPv(resource);
    return ok;
}
#pragma pop

// Dispatch the event to the drawable puppet and selected avatar object when both exist.
int CExhibition::ParseMaterialLayerAnimationCommand(int arg2) {
    void* first = fn_802BA4AC_a(this);
    if (!first)
        return 0;
    void* second = fn_802BA4AC_b(this);
    if (second)
        return WPuppet_ParseMaterialLayerAnimationCommand(first, arg2, second);
    return 0;
}

// Consume quoted bone names and hide each named bone in the resolved preview puppet.
void CExhibition::HideNamedPuppetBones(void* scriptCursor) {
    int boneNameToken[16] = {0};
    int tokenCount = 0;
    void** cursor = (void**)scriptCursor;
    while (1) {
        *cursor = String_ParseQuotedToken(*cursor, boneNameToken);
        if (strlen((char*)boneNameToken) == 0)
            break;
        if ((unsigned int)strlen((char*)boneNameToken) >= 0x40)
            break;
        void* first = fn_802BA578_get(this);
        if (first) {
            WPuppet_SetBoneVisibilityRecursive(first, 0, 1, boneNameToken);
        }
        if (*cursor == 0)
            break;
        if (*(char*)*cursor != (char)0x29) {
            tokenCount++;
            if (tokenCount >= 0x40)
                break;
        } else {
            break;
        }
    }
}

// Consume an optional current-animation condition and two following quoted arguments; no further action is established.
void CExhibition::ParseAnimationGatedCommand(void* scriptCursor) {
    char conditionOrName[0x40];
    char eventName[0x40];
    char eventArgument[0x40];
    void** cursor = (void**)scriptCursor;
    unsigned int tokenLength;
    conditionOrName[0] = 0;
    *cursor = String_ParseQuotedToken(*cursor, conditionOrName);
    tokenLength = (unsigned int)strlen(conditionOrName);
    if (tokenLength == 0)
        return;
    tokenLength = (unsigned int)strlen(conditionOrName);
    if (tokenLength >= kPreviewTokenCapacity)
        return;
    if (conditionOrName[0] == (char)0x40) {
        void* obj = animationRange;
        if (obj == 0)
            return;
        char* other = ((PreviewAnimationName*)obj)->name;
        if (CString_Compare(conditionOrName + 1, other) != 0)
            return;
        *cursor = String_ParseQuotedToken(*cursor, eventName);
        tokenLength = (unsigned int)strlen(eventName);
        if (tokenLength == 0)
            return;
        tokenLength = (unsigned int)strlen(eventName);
        if (tokenLength >= kPreviewTokenCapacity)
            return;
    } else {
        strcpy(eventName, conditionOrName);
    }
    *cursor = String_ParseQuotedToken(*cursor, eventArgument);
    tokenLength = (unsigned int)strlen(eventArgument);
    if (tokenLength == 0)
        return;
    tokenLength = (unsigned int)strlen(eventArgument);
}

#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
// Play an animation when a listed .mpet part is present, preserving the original token-loop control flow.
void CExhibition::PlayAnimationIfPartPresent(void* scriptCursor) {
    char partModelName[0x28];
    char animationName[0x100];
    char partNames[0x400];
    void** cursor = (void**)scriptCursor;
    animationName[0] = 0;
    partNames[0] = 0;
    *cursor = String_ParseQuotedToken(*cursor, partNames);
    *cursor = String_ParseQuotedToken(*cursor, animationName);

    if (partNames[0] == 0)
        return;

    if (!animationName[0])
        return;
    int partNameOffset = 0;
    int partNameLength = 0;
    while (1) {
        char* found = strchr(partNames + partNameOffset, 0x3b);
        int zero = (found == 0);
        if (!zero) {
            partNameLength = (int)(found - partNames) - partNameOffset;
        } else {
            partNameLength = (int)strlen(partNames + partNameOffset);
            if (partNameLength == 0)
                return;
        }
        strncpy(partModelName, partNames + partNameOffset, partNameLength);
        partModelName[partNameLength] = 0;
        strcat(partModelName, exhibitionResourceStrings + kExhibitionStringPool_text_67_mpet);
        void* obj = modelController;
        if (obj != 0 && ResourceModel_HasPartModelNamed(obj, partModelName)) {
            PlayAnimationByName(animationName, 0, kFloat_0_exhibition);
            return;
        }
        if (!zero)
            return;
        partNameOffset = partNameOffset + partNameLength + 1;
    }
}
#pragma pop

// This preview event callback intentionally performs no action.
void CExhibition::vfunc_21() {}

// Parse a conditioned quoted event token against the avatar model controller.
int CExhibition::ParseConditionedQuotedTokenImpl(int arg2, int arg3, int arg4) {
    return ParseConditionedQuotedToken(this, arg2, arg4, modelController, 0);
}

// Seed the effect screen origin and create/attach the event's named effect.
int CExhibition::CreateAttachedEffectAtScreenCenter(float* output, int arg3) {
    output[0] = screenCenterX;
    output[1] = screenCenterY;
    return FxBox_CreateAndAttachEffectByExtension(this, output, arg3, ((char*)(fxBox)));
}

// Process animation event arguments using the current puppet and animation range.
int CExhibition::ParseBoneReferences(int arg2, int arg3) {
    return FxAttachment_ParseBoneReferences(this, arg2, arg3, puppet, animationRange, 1, 0);
}

// Parse a nonempty sound name and play it with the preview sound parameters.
void CExhibition::PlaySoundCommand(void* scriptCursor) {
    unsigned int soundName[18];
    if (Parse_NonemptyQuotedTokenUnder64Bytes(this, scriptCursor, &soundName)) {
        Sound_PlayByName(g_soundManager, soundName, 0, 0, 0, 0, kFloat_0_5_exhibition, kFloat_200_PreviewSound,
                         kFloat_1_Intro);
    }
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Read the pointer vector's current element count.
static inline int size802BA96C(void* self) {
    return *(int*)((char*)self + 4);
}
// Reset and free pointer-vector storage after its owned elements have been cleaned.
static inline void free802BA96C_0(void* self) {
    if (self) {
        void* ptr = *(void**)self;
        if (ptr) {
            int* p = (int*)((char*)self + 4);
            int size = size802BA96C(self);
            *p = *p - size;
            appFreeUni__FPv(ptr);
        }
    }
}
// Preserve the first nullable subobject wrapper around vector cleanup.
static inline void free802BA96C_1(void* self) {
    if (self)
        free802BA96C_0(self);
}
// Preserve the second nullable subobject wrapper around vector cleanup.
static inline void free802BA96C_2(void* self) {
    if (self)
        free802BA96C_1(self);
}
// Preserve the outer nullable wrapper used by both vector specializations.
static inline void free802BA96C_3(void* self) {
    if (self)
        free802BA96C_2(self);
}

// Free a pointer-vector allocation and optionally delete its owner; elements were cleaned by the caller.
void* dtor_802BA96C(void* self, int deleting) {
    if (self) {
        free802BA96C_3(self);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Free the second pointer-vector specialization and optionally delete its owner.
void* dtor_802BA9EC(void* self, int deleting) {
    if (self) {
        free802BA96C_3(self);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset

// Restore the saved scissor rectangle and optionally delete its state guard.
CSetScissorRect::~CSetScissorRect() {
    *(void**)this = (void*)__vt__15CSetScissorRect;
    Engine_SetScissorRect((char*)this + 4);
}

} // extern "C"

#undef REPLACE_PREVIEW_STRING_RANGE
#undef ASSIGN_PREVIEW_ANIMATION
#undef APPEND_PREVIEW_MODEL_SUFFIX
#undef CLEAN_TREE_LEAF
#undef CLEAN_TREE_PAIR
#undef CLEAN_TREE_BRANCH
#undef CLEAN_TREE

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624C8", _ctors_u802B4360_0, CExhibition_InitializeAngleCache);
#pragma pop
