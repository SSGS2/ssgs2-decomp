#include <decomp/static_init.h>
#include <decomp/offset_of.h>
// Pet/model animation, flight movement, look-at, lighting and script commands.
// ABI/address map: config/R2PE9B/symbols.txt; shared object: classes/CPetBody.h;
// animation ranges/bones belong to WPuppet; constants live in u801B9214_data.cpp.
// Mod lookup: movement rates/height -> UpdateMovement and StartMovement;
// animation rate/root displacement -> Update, ComputeAnimationStep, animationSpeed
// and rootMotionScale; command callbacks -> DispatchCommandCallbacks;
// quoted attachment grammar/ownership -> vfunc_10. These are separate controls.
// Clock boundary: Update (old virtual slot 27) accepts seconds, as do its callers
// such as CCoin_UpdateCoinsAndSprites in course_ball_shot_effects.cpp. Main-frame seconds come from
// CMainFrame_UpdateVirtualHooks (main_frame_letter_and_mouse_cursor.cpp), then CProjectG_Update's task time
// scale (project_g_app_and_devices.cpp). Render cadence and the 30-frame/s clip sampling constant
// are separate: never replace seconds with a frame count for a frame-rate mod.
#include "MSL/vector.h"
// Vector destroyed by dtor_801BBCE8; element type not known yet (Elem_801BBCE8 is a tag);
// its size reset stores 0 (form 3).
struct Elem_801BBCE8;
namespace std {
template <> struct __vec_destroys_elements<Elem_801BBCE8*> {
    enum { value = 3 };
    // Pointer elements have no per-element destruction work.
    static void destroy(Elem_801BBCE8**) {}
};
}
typedef std::vector<Elem_801BBCE8*, app::app_allocator<Elem_801BBCE8*> > Elem_801BBCE8Vector;
#include "classes/CPetBody.h"
// Byte offsets derived from the checked shared layout retain original optimizer lifetimes.
enum PetByteOffsets {
    kPetByteOffset_rotationX = ((int)DECOMP_OFFSETOF(CPetBody, rotationX)),
    kPetByteOffset_baseMatrixPositionX = ((int)DECOMP_OFFSETOF(CPetBody, baseMatrixPositionX)),
    kPetByteOffset_matrixPosition_v_1_ = ((int)DECOMP_OFFSETOF(CPetBody, matrixPosition.v[1])),
    kPetByteOffset_matrixPosition_v_2_ = ((int)DECOMP_OFFSETOF(CPetBody, matrixPosition.v[2])),
    kPetByteOffset_movementDirection = ((int)DECOMP_OFFSETOF(CPetBody, movementDirection)),
    kPetByteOffset_animationFinished = ((int)DECOMP_OFFSETOF(CPetBody, animationFinished)),
    kPetByteOffset_ambientLightColor = ((int)DECOMP_OFFSETOF(CPetBody, ambientLightColor)),
    kPetByteOffset_rootMotionDeltaX = ((int)DECOMP_OFFSETOF(CPetBody, rootMotionDeltaX)),
    kPetByteOffset_baseMatrixBasis = ((int)DECOMP_OFFSETOF(CPetBody, baseMatrixBasis)),
    kPetByteOffset_lightDirectionX = ((int)DECOMP_OFFSETOF(CPetBody, lightDirectionX)),
    kPetByteOffset_matrixPosition = ((int)DECOMP_OFFSETOF(CPetBody, matrixPosition)),
    kPetByteOffset_lightingMode = ((int)DECOMP_OFFSETOF(CPetBody, lightingMode)),
    kPetByteOffset_matrixBasis = ((int)DECOMP_OFFSETOF(CPetBody, matrixBasis)),
    kPetByteOffset_lightColor = ((int)DECOMP_OFFSETOF(CPetBody, lightColor)),
    kPetByteOffset_positionX = ((int)DECOMP_OFFSETOF(CPetBody, positionX)),
    kPetByteOffset_flags = ((int)DECOMP_OFFSETOF(CPetBody, flags)),
    kPetByteOffset_label = ((int)DECOMP_OFFSETOF(CPetBody, queuedAnimationName)),
};

// Plain layout views for the records passed through the pet's virtual interfaces.
struct PetAnimationRange {
    float startTime, endTime, maximumTime;
    char paddingToNames[0xC];
    void* name;
    void* repeatName;
};
struct PetBoneControllerRecord { char prefix[0x68]; void* controllerKey; };
struct PetRenderParameter {
    unsigned int kind;
    float transformedDirection[3];
    char middle[0xC];
    float sourceDirection[3];
};
struct PetCameraMatrices { char prefix[0x20]; float view[16]; float projection[16]; };
struct PetTypeInfoLink { void* type; void* parent; };
struct PetAttachment { char animation[0x40]; char name[0x40]; void* resource; char state[0x10]; };
struct PetAttachmentLink { void* previous; void* next; PetAttachment* value; };
struct PetInstanceNode { void* value; char prefix[0xC]; void* next; };
struct PetNamedBoneEntry { void* bone; void* bounds; };
struct PetBoneMatrix { char prefix[0x78]; float matrix[12]; };
struct PetLookAtBone { char reserved00[0x78]; float worldBasis[9]; float worldOrigin[3]; };
typedef char PetLookAtBoneOriginCheck[((int)DECOMP_OFFSETOF(PetLookAtBone, worldOrigin) == 0x9C) ? 1 : -1];
struct PetMovementBall { float position[3]; char prefix[0x28]; float movementScalar; };
struct PetRenderDefaults { unsigned int renderStates[26]; unsigned int stageStates[12]; unsigned int samplerStates[10]; };
typedef char PetRenderParameterSize[(sizeof(PetRenderParameter) == 0x28) ? 1 : -1];
typedef char PetAttachmentSize[(sizeof(PetAttachment) == 0x94) ? 1 : -1];
enum PetBodyFlags {
    kCourseModelInlineName = 0x60, kCourseModelInlineNameBytes = 0x28,
    kPetRootMotionSampled = 1, kPetUsesRootMotion = 2,
    kPetAnimationPlaying = 4, kPetMatrixDirty = 0x10,
    kPetHeadAnglesDirty = 0x20, kPetLookAtDirty = 0x30, kPetHidden = 0x2000
};
// Original composites include the dirty matrix bit as well as playback.
enum PetDrawMasks { kPetNeedsAnimationOrMatrix = 0x14, kPetNeedsHeadCheck = 0x34 };
enum PetEulerAxis { kPetAxisX=0, kPetAxisY=1, kPetAxisZ=2 };
enum PetMovementPhase { kPetTakeoff = 0, kPetCruise = 1, kPetLanding = 2 };
// Byte positions in the external packed string pool; preserve its original storage and encoding.
enum {
    kPetBody_headBone = 0x14, // "Head"
    kPetBody_chanceMarker = 0x64, // "チャンス"
    kPetBody_birdieText = 0x72, // "バーディー"
    kPetBody_scoreTextSuffix = 0x6D, // "が文"
    kPetBody_caddieGroup = 0x91, // "caddie"
    kPetBody_facialAnimationMarker = 0x7D, // "MT_FACIAL_ANIMATION"
};

#include "classes/WPuppet.h"
// WPuppet bounds fields: CPetBody_SetupLighting and CPetBody bounds queries, original pet_body_animation_and_movement.cpp.
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#include "classes/WPolySoup.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#define AsWPolySoup(p) ((WPolySoup*)(p))
#include "sdk_protos.h"
#include "game/wvector.h"
#include "game/types.h"
#include "classes/CPetBody.h"

#define AsCPetBody(p) ((CPetBody*)(p))

// ---- merged from pet_body_animation_and_movement.cpp ----
namespace ns0 {
// Translation unit 0x801B9214-0x801BA728 (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct Blob26 { unsigned int w[26]; };
struct Blob12 { unsigned int w[12]; };
struct Blob10 { unsigned int w[10]; };

void __as__7WVectorFRC7WVector(void*, ...);
void Matrix_CopyAffine(void*, void*);
void Matrix_ConcatAffine(void*, void*, void*);
void Matrix_ExtractUniformScaleNormalizedRotation(void*, void*);
void Matrix_SetAxisRotation(void*, float, int);
void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, const char*);
unsigned int strlen(const char*);
void Vec2_Zero(void*);
void Vec3_Zero(void*);
void Matrix_TransformDirection(void*, void*, void*);
void __ml__FfRC7WVector(void*, void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
int WPuppet_FindBoneByName(void*, const char*);
void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int);
void CPetBody_UpdateMovement(void*, float);
void CPetBody_UpdateLookAt(void*, float);
float CPetBody_ComputeAnimationStep(void*, float);
void CPetBody_PlayAnimationByName(void*, float, void*, int, int);
void CPetBody_UpdateAnimation(void*, float);
void CPetBody_Update(void*, float);
void WPuppet_UpdateBound(void*, int);
void* Object_GetChild(void*);
void* CQuadTree_GetInstance(float, float);
// Terrain-lighting multiplier hook; the current implementation returns 1.0f.
float CQuadTree_DefaultLightingFactor(void*, void*);
void* Color_ScaleRGB(void*, float);
void Normalize__7WVectorFv(void*);
void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
void WPuppet_GetRootMotionDelta(void*, float, void*, float, void*);
void __mi__FRC7WVectorRC7WVector(void*, const char*, const char*);
char* strchr(char*, int);
int strncmp(const char*, const char*, int);
void strncpy(char*, const char*, unsigned int);
char* strcat(char*, const char*);
char* strcpy(char*, const char*);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void* GolfBall__Fv(void);
void Sound_PlayByName(void*, const char*, void*, void*, void*, void*, float, float, float);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void ResourceManager_ReleaseObject(void*, void*, int);
void* WPuppet_HasBoneControllerEntry(void*, void*);
void WPuppet_SetBoneControllerVector(void*, void*, void*);
void WPuppet_EraseBoneControllerEntry(void*, void*);
void Model_SetMatrixRecursive(void*, void*, void*);
void __ami__7WVectorFRC7WVector(void*, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
void WPuppet_UpdateBoundsFromBone(void*);
int WView_DefaultCapabilityTrueA(void*, void*);
void* WView_DefaultAcceptVisibility(void*, void*);
void Memory_Clear(void*, int);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Camera_Apply(void*, float, float);
void* Camera_GetRenderState(void*);
void gtl_GenericRenderer_SetProjectionMatrix(void*);
void gtl_GenericRenderer_SetViewMatrix(void*);
void SetTransform(int, void*);
void* WPuppet_GetSkinMatrices(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __dt__13WInstanceLockFv(void*);
extern void* g_resourceManager;
extern char __vt__8CPetBody[];
extern void* g_taskManager;
extern void* g_petBodyTaskChild;
extern void* s_golfTaskTypeInfo;
extern unsigned int s_sceneLighting[];
extern const float kFloat_1_petBody;
extern float kFloat_neg0_7853982_PetBody;
extern const float kFloat_0_petBody;
extern const float kFloat_50_PetBody;
extern void* g_sharedDoc;
extern void* g_soundManager;
extern const float kFloat_0_5_PetBody;
extern const float kFloat_200_PetBody;
extern const char s_cameraTransform[];
extern const char petBodyStrings[];
extern const unsigned int kPetPositionalSoundOptionWord0;
extern const unsigned int kFloat_1000_PetSound;
extern float s_petLookAtPitchReference, s_petLookAtYawReference, kFloat_0_00001_PetAnimation, kPetAnimationFramesPerSecond;
extern const float kFloat_neg1_PetBody;
extern const float kFloat_0_petBody;
extern void* g_view;
extern const unsigned int s_petBodyDrawStateDefaults[];

// Clear playback flags, queued animation and look-at state; restore animation/root-motion speed scales.
void CPetBody_ResetPlaybackAndLookAt(void* self);

#pragma push
#pragma opt_common_subs off
struct PetDevice {
    virtual void d0();
    virtual void d1();
    virtual void d2();
    virtual void v5(void* a, int b, int c);
    virtual void v6(void* a, int b, void* c, int d, float e, int f);
    virtual void d5();
    virtual void d6();
    virtual void d7();
    virtual void v10(void* a, int b, int c, float d, int e, int f, void* g, int h);
};
// Perform an ordered float comparison used by head-angle change detection.
inline int lt(float a, float b) { return a < b; }
// Clear a float's negative sign by the original bit test (including negative zero).
static inline float fabsd(float f) {
    union { float f; int i; } u;
    u.f = f;
    if (u.i & 0x80000000) f = -f;
    return f;
}
#pragma opt_loop_invariants off
// Apply animation and temporary head controllers, update matrices and bounds, then draw the visible pet.
// useViewDraw selects the camera-aware path; drawEnabled gates drawing only,
// not matrix/bounds refresh. All model pointers are borrowed. A head controller
// installed by this call must be removed before returning; preexisting ones stay.
void CPetBody_Draw(void* self, void* useViewDraw, void* drawEnabled);
#pragma pop

// Replace the stored bone name used when positioning the pet against the ground.
void CPetBody_SetNameString(void* self, const char* text);

// Copy the requested position into the pet matrices and mark them dirty.
void CPetBody_SetPosition(void* self, const void* sourcePosition);

// Build the pet rotation matrix from Euler angles while retaining its translation.
void CPetBody_SetRotation(void* self, const void* sourceEulerRadians);

// Compose an external rotation with the pet's normalized rotation while retaining translation.
void CPetBody_ApplyRotationMatrix(void* self, void* rotationMatrix);
// Set the movement destination and initialize flight pitch, height and vertical speed.
void CPetBody_StartMovement(void* self, void* destination);

#pragma push
#pragma opt_common_subs off
// Rebuild base/current orientation for the requested yaw and clear movement direction.
void CPetBody_SetYaw(char* self, float value);
#pragma pop
// Advance animation, apply root motion to position and update movement and head tracking.
void CPetBody_Update(void* self, float dt);

// Advance within the animation range, process crossed events and continue into the queued or repeat animation.
void CPetBody_UpdateAnimation(void* self, float dt);

struct CVirtObj {
    virtual void* vfunc_00();
    virtual void* vfunc_04();
    virtual void* vfunc_08();
};

// Return whether the object's runtime type ancestry includes the golf-task descriptor.
inline int FindInChain(void* obj) {
    if (obj) {
        void* q = ((CVirtObj*)obj)->vfunc_00();
        while (q) {
            if (q == &s_golfTaskTypeInfo) return 1;
            q = ((PetTypeInfoLink*)q)->parent;
        }
        return 0;
    }
    return 0;
}

// Use course lighting for golf tasks, otherwise set a normalized default light and white colors.
void CPetBody_SetupLighting(void* self);

// Clamp the animation step at its maximum and sample or accumulate root motion for that interval.
float CPetBody_ComputeAnimationStep(void* self, float dt);

struct WInstanceLock {
    void* mutex;
    // Acquire the borrowed instance mutex during cleanup traversal.
    WInstanceLock(void* p) : mutex(p) { OSLockMutex(p); }
    // Release the cleanup traversal's mutex acquisition.
    ~WInstanceLock() { OSUnlockMutex(mutex); }
};
// Callback payloads are individually deleted; their intrusive nodes are recycled.
// poolAllocationHead is a separate ring of allocations, freed only in phase 4.
struct PetCallbackRingNode { void* data; unsigned int pad[2]; PetCallbackRingNode* prev; PetCallbackRingNode* next; };
// Attachments use a sentinel list, with separately allocated node and payload.
struct PetAttachmentListNode { PetAttachmentListNode* prev; PetAttachmentListNode* next; PetAttachment* data; };
struct PetRenderParameterSlot { char data[0x28]; };
struct PetCleanupView {
    void* vtable; void* puppetResource; char pad08[0xC4];
    PetCallbackRingNode* callbackHead; PetCallbackRingNode* callbackCursor; PetCallbackRingNode* poolAllocationHead; PetCallbackRingNode* recycledCallbackHead;
    char padDC[8]; int bucketCount; void** callbackBuckets; char mutex[0x18];
    char* renderParameterData; int renderParameterCount; char pad10C[0x24];
    unsigned int stringFlags; unsigned int stringLength; void* stringData;
    char pad13C[0x44]; int attachmentNodeCount; PetAttachmentListNode attachmentSentinel;
    // Return the current ring payload and advance the iterator, or null at exhaustion.
    inline void* NextCallbackPayload() {
        void* b = this->mutex;
        OSLockMutex(b);
        PetCallbackRingNode* node = this->callbackCursor;
        if (node) {
            PetCallbackRingNode* next = node->next;
            void* value = node->data;
            if (next == this->callbackHead) next = 0;
            this->callbackCursor = next;
            OSUnlockMutex(b);
            return value;
        }
        OSUnlockMutex(b);
        return 0;
    }
};
// Detach and free list nodes only; payload cleanup is performed by the caller.
inline void CleanupClearList(PetCleanupView* s) {
    PetAttachmentListNode* it = s->attachmentSentinel.next;
    PetAttachmentListNode* attachmentSentinel = &s->attachmentSentinel;
    if (it != attachmentSentinel) {
        PetAttachmentListNode* lastAttachmentNode = attachmentSentinel->prev;
        it->prev->next = lastAttachmentNode->next;
        lastAttachmentNode->next->prev = it->prev;
        while (it != attachmentSentinel) {
            void* cur = it;
            it = it->next;
            appFreeUni__FPv(cur);
            --s->attachmentNodeCount;
        }
    }
}

#pragma push
#pragma use_lmw_stmw on
// Release instance callbacks, attachment resources, puppet, render parameters and name storage.
void* CPetBody_dtor(PetCleanupView* self, int deleting);
#pragma pop

// Resolve pet sound commands and play at the pet position with task-specific sound parameters.
void CPetBody_PlaySound(void* self, char* text);

} // extern "C"

} // namespace ns0

// ---- merged from pet_body_animation_and_movement.cpp ----
namespace ns1 {
// Translation unit 0x801BA728-0x801BBD68 (27 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void OSLockMutex(void* self);
void OSUnlockMutex(void* self);
int strstr(void* arg0, void* arg1);
int BaseObject_ctor(void* self, void* arg1);
void* memset(void* destination, int value, unsigned int size);
extern char __vt__9WPolySoup[];
extern char __vt__Q29WPolySoup13CStaticPetGrp[];
extern const float kFloat_0_PetBody;
extern float kFloat_800_PolySoup;
extern float kFloat_960_PolySoup;

char* strcpy(char* destination, const char* source);
void WPuppet_SetOpacityRecursive(void* self, unsigned int opacity);
void WPuppet_ParseMaterialLayerAnimationCommand(void* arg0, void* arg1, void* arg2);
void ShadowModel_SetNamedProjectionEnabled(void* arg0, const char* arg1, int arg2);
void FxAttachment_ParseBoneReferences(void* self, void* arg1, void* arg2, void* arg3, void* arg4, int arg5, int arg6);
int Parse_NonemptyQuotedTokenUnder64Bytes(void* self, void* arg1, void* out);
void CPetBody_PlaySound(void* self, void* value);
void Matrix_SetBasisElements(void* arg0, float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7, float arg8, float arg9);
void Matrix_SetBasisScale(void* arg0, void* arg1);
int ResourceManager_PollResourceRead(void* arg0);
void* ResourceManager_PollPuppetResourceLoad(void* arg0);
int WPuppet_FindBoneByName(void* arg0, const char* arg1);
void ResourceManager_ReleaseObject(void* arg0, void* arg1, int arg2);
void ResourceManager_RequestModelLoad(void* arg0, void* arg1);
void appFreeUni__FPv(void* arg0);
void __dl__FPv(void* arg0);
void WPuppet_SetAnimationTimeFromRanges(void* arg0, int arg1, float arg2, int arg3, int arg4);
void Model_SetMatrixRecursive(void* arg0, void* arg1, void* arg2);
void WPuppet_GetRootMotionDelta(void* result, float startFrame, void* self, float endFrame, void* rangeName);
void Matrix_TransformDirection(void* out, void* arg1, void* arg2);
void __ami__7WVectorFRC7WVector(void* out, void* arg1);
void __as__7WVectorFRC7WVector(void* out, void* arg1);
void __ml__FRC7WVectorf(void* out, void* arg1, float arg2);
void __pl__FRC7WVectorRC7WVector(void* out, void* arg1, void* arg2);
void WPuppet_GetInverseRootMotionOffsetForRange(void* out, void* arg1, float arg3, int arg2);
void* WPuppet_FindAnimationRangeByName(void* arg0, void* arg1);
void AnimationEventParser_ProcessFrameRange(void* self, void* manager, float startTime, float endTime, void* bounds, void* handlerContext, void* singleFrame);
void WPuppet_UpdateBoneWorldMatrices(void* arg0, int arg1, int arg3, float arg2);
void WPuppet_UpdateBoundsFromBone(void* arg0);
char* String_ParseQuotedToken(const char* source, char* output);
int CString_Compare(const char* left, const char* right);
unsigned int strlen(const char* value);
void* __nw__FUl(unsigned int size);
void* appMallocDummy__FUl(unsigned int size);
extern void* g_shadowManager;
extern void* g_resourceManager;
extern const char petBodyStrings[];
extern float s_initializedTwoPi_801BBB28;
extern char s_petBodyBasisMatrix[];
extern const float kFloat_0_petBody;
extern const float kFloat_0_00001_PetAnimation;
extern float kFloat_6_2831855;
extern const float kFloat_1_petBody;
extern const float kFloat_neg1_PetBody;
extern const float kPetAnimationFramesPerSecond;
extern const float kFloat_0_5_PetBody;

void CPetBody_PlayAnimationByName(void* self, float delta, void* arg2, void* arg3, void* arg4);
void __mi__FRC7WVectorRC7WVector(void* out, void* arg1, void* arg2);
float Vector_SignedAngleXZ(void* arg0, void* arg1);
float Math_SquareRootApprox(float value);
double atan(double value);
float fabsf(float value);
void __pl__FRC7WVectorRC7WVector(void* out, void* arg1, void* arg2);
void __as__7WVectorFRC7WVector(void* out, void* arg1);
void Matrix_SetAxisRotation(void* out, float value, int axis);
void Matrix_ConcatAffine(void* out, void* arg1, void* arg2);
void Matrix_CopyAffine(void* out, void* arg1);
void Vec2_Zero(void* self);
struct FloatPair { float first; float second; };
FloatPair Vec2_Subtract(void* arg0, void* arg1);
float Vec2_CalculateLength(void* arg0);
extern const float kFloat_neg0_34906584_PetBody;
extern const float kFloat_0_7853982_PetBody;
extern const float kFloat_neg0_7853982_PetBody;
extern const float kFloat_0_017453292_PetBody;
extern const float kFloat_1_5707964_PetMotion;

// Return the current animation range's name, or null when no animation is selected.
void* CPetBody_GetCurrentAnimationName(void* self);

// Forward a material-layer animation command to the puppet if loaded.
void CPetBody_ParseMaterialLayerAnimationCommand(void* self, void* commandText);

// Hide the pet and disable its named shadow projection.
void CPetBody_Hide(void* self);

// Show the pet and enable its named shadow projection.
void CPetBody_Show(void* self);

// Parse a bounded sound token and play it through the pet sound handler.
void CPetBody_PlaySoundCommand(void* self, void* commandText);

#pragma push
#pragma scheduling 604
// Initialize the pet basis matrix and this unit's full-turn constant.
void PetBody_InitBasisAndFullTurn(void);
#pragma pop

// Enable the flight movement path.
void CPetBody_vfunc_16(void* self);

// Set the puppet's opacity recursively when its model is loaded.
void CPetBody_SetOpacity(void* self, unsigned int opacity);

// Parse effect attachment bone references against the current puppet and animation.
void CPetBody_ParseBoneReferences(void* self, void* commandText, void* effectContext);

// Return the loaded puppet pointer.
void* CPetBody_GetPuppet(void* self);

// Return the mutable queued-animation name buffer.
void* CPetBody_GetNextAnimationName(void* self);

// Return the animation-finished word through the original pointer-sized ABI.
void* CPetBody_GetAnimationFinished(void* self);

// Return the raw animation-time word through the original pointer-sized ABI.
void* Object_GetWord10(void* self);

// Apply uniform scale to the pet and loaded puppet matrices.
void CPetBody_SetUniformScale(void* self, float value);

// Poll an outstanding model read and publish the resulting puppet when complete.
int CPetBody_PollModelLoad(void* self);

// Set the tracking target and turn behavior when the puppet has a head bone.
void CPetBody_SetLookAtTarget(void* self, void* target, unsigned char turnBody);

// Queue an animation name and its playback option.
char* CPetBody_SetNextAnimation(void* self, const char* source, unsigned char useRootMotion);

// Return the loaded puppet's minimum bounds vector.
void* CPetBody_GetPuppetBoundsMin(void* self);

// Release the old puppet and request the replacement model asynchronously.
int CPetBody_RequestModelLoad(void* self, void* modelName);

// Return the animation magnitude with the original float sign test and double result.
inline double AnimationAbs(float value) {
    if (*(unsigned int*)&value & 0x80000000u) return -value;
    return value;
}
#pragma opt_common_subs off
// Select an animation range and playback flags, updating root-motion alignment when required.
void CPetBody_PlayAnimationByName(void* self, float startOffset, void* animationName, void* useRootMotion, void* preserveRootMotion);

#pragma opt_common_subs reset
// Clamp head pitch in radians to [-20 degrees,+45 degrees].
static inline float Pet_ClampLookAtPitch(float value) {
    return value < kFloat_neg0_34906584_PetBody ? kFloat_neg0_34906584_PetBody : (value > kFloat_0_7853982_PetBody ? kFloat_0_7853982_PetBody : value);
}

// Clamp head yaw in radians to [-45 degrees,+45 degrees].
static inline float Pet_ClampLookAtYaw(float value) {
    return value < kFloat_neg0_7853982_PetBody ? kFloat_neg0_7853982_PetBody : (value > kFloat_0_7853982_PetBody ? kFloat_0_7853982_PetBody : value);
}

#pragma opt_common_subs off
// Smooth head pitch/yaw toward a target and optionally rotate the body to follow it.
void CPetBody_UpdateLookAt(void* self, float delta);
#pragma opt_common_subs reset

void* GolfBall__Fv(void* self);
float Magnitude__7WVectorCFv(void* value);
void __ml__FfRC7WVector(void* out, void* arg1, float scalar);
void Matrix_TransformPoint(void* out, void* arg1, void* arg2);
void Vector_CrossProduct(void* out, void* arg1, void* arg2);
void Normalize__7WVectorFv(void* value);
float __ml__FRC7WVectorRC7WVector(void* out, void* arg1);
void Vec3_Zero(void* value);
void* WPuppet_FindNamedBoneEntry(void* arg0, void* arg1);
void* CQuadTree_GetInstance(void);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void* out, void* arg1, void* arg2, int arg3, int arg4, int arg5);
void __apl__7WVectorFRC7WVector(void* out, void* arg1);
extern float kFloat_0_4_PetBody, kFloat_25_PetBody, kFloat_10_PetBody, kFloat_1_5_PetBody;
extern float kFloat_3_PetBody, kFloat_0_3_PetBody, kFloat_neg0_3926991_PetPitch, kFloat_36_PetBody, kFloat_1_2_PetBody;
extern float s_petLandingPitchTargetRadians;

typedef Vec3f PetVector;
// Test the signed value against the movement tolerance; negative values pass.
inline int Pet_IsBelowEpsilon(float value) { return value < kFloat_0_00001_PetAnimation; }
// Test absolute magnitude against the movement tolerance using the original sign-bit rule.
inline int Pet_IsMagnitudeBelowEpsilon(float value) { double magnitude; if (*(unsigned int*)&value & 0x80000000u) magnitude = -value; else magnitude = value; return magnitude < kFloat_0_00001_PetAnimation; }
// Convert a zero word into a true result.
inline int Pet_IsZero(unsigned int value) { return !value; }
// Store vector components in the original X/Z/Y write order.
inline void Pet_SetVector(PetVector* v, float x, float y, float z) { v->x = x; v->z = z; v->y = y; }

// Rebuild flight orientation from base yaw and movement pitch, retaining world translation.
// Scratch stays in the caller to retain the original stack layout; no storage escapes.
static inline void Pet_RebuildFlightRotation(void* self,
        PetVector& direction, PetVector& axis, PetVector& rotation,
        float* rotationZMatrix, float* rotationYMatrix, float* rotationXMatrix,
        float* rotationXYMatrix, float* rotationXYZMatrix) {
    Pet_SetVector(&axis, kFloat_0_petBody, kFloat_0_petBody, kFloat_neg1_PetBody);
    __ml__FfRC7WVector(&direction, ((char*)&AsCPetBody(self)->baseMatrixBasis[6]), kFloat_neg1_PetBody);
    float angle = Vector_SignedAngleXZ(&direction, &axis);
    rotation.x = -AsCPetBody(self)->movementPitchRadians; rotation.y = angle; rotation.z = kFloat_0_petBody;
    __as__7WVectorFRC7WVector(((char*)&AsCPetBody(self)->rotationX), &rotation);
    Matrix_SetAxisRotation(rotationZMatrix, AsCPetBody(self)->rotationZ, 2);
    Matrix_SetAxisRotation(rotationYMatrix, AsCPetBody(self)->rotationY, 1);
    Matrix_SetAxisRotation(rotationXMatrix, AsCPetBody(self)->rotationX, 0);
    Matrix_ConcatAffine(rotationXYMatrix, rotationXMatrix, rotationYMatrix); Matrix_ConcatAffine(rotationXYZMatrix, rotationXYMatrix, rotationZMatrix);
    Matrix_CopyAffine(((char*)&AsCPetBody(self)->matrixBasis), rotationXYZMatrix);
    __as__7WVectorFRC7WVector(((char*)&AsCPetBody(self)->matrixPosition), ((char*)&AsCPetBody(self)->positionX)); (*(int*)&AsCPetBody(self)->flags) |= kPetMatrixDirty;
}
#pragma opt_common_subs off
// Advance takeoff, cruise and landing behavior and steer toward the moving ball.
// Positions and ground heights are world-space, Y-up; pitch is in radians.
// delta is seconds from Update (no further conversion here).
// The first two distances use current.y=0, but retain the stored target/start Y.
// Requires a live ball and initialized movementStart/Target/Direction.
void CPetBody_UpdateMovement(void* self, float delta);
#pragma opt_common_subs reset

// Clear and release a temporary vector, then optionally delete its wrapper.
void* dtor_801BBCE8(void* self, int arg1);

#pragma opt_common_subs off
// Apply the current animation time and compensate the puppet matrix for sampled root motion.
void CPetBody_ApplyAnimationToPuppet(void* self);
#pragma opt_common_subs reset

struct WInstanceLock {
    void* value;
    // Acquire the borrowed mutex during instance callback traversal.
    WInstanceLock(void* arg) : value(arg) { OSLockMutex(arg); }
    // Release this callback traversal's mutex acquisition.
    ~WInstanceLock() { OSUnlockMutex(value); }
};

struct CallbackArgs;
struct InstanceCallback {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void Invoke(CallbackArgs* args);
};

struct InstanceEntry {
    const char* matchText; // +0: borrowed substring sought in command text, not a list link.
    InstanceCallback* callback;
    unsigned int flags;
};

struct CallbackArgs {
    unsigned int index;
    unsigned int value;
    int matched;
    unsigned int pad[4];
    unsigned int tail;
};

// Return the current borrowed callback entry and advance the circular iterator
// under the mutex. The local guard unlocks BEFORE the entry reaches the caller;
// it protects iterator access, not the returned entry's lifetime or invocation.
static inline InstanceEntry* NextInstance(char* self) {
    WInstanceLock lock(((char*)&AsCPetBody(self)->instanceMutex));
    void* node = *(void**)(((char*)&AsCPetBody(self)->instanceIterator));
    if (node != 0) {
        void* next = ((PetInstanceNode*)node)->next;
        InstanceEntry* entry = *(InstanceEntry**)node;
        if (next == *(void**)(((char*)&AsCPetBody(self)->instanceHead))) next = 0;
        *(void**)(((char*)&AsCPetBody(self)->instanceIterator)) = next;
        return entry;
    }
    return 0;
}

#pragma push
#pragma use_lmw_stmw on
// Match registered substrings; iterator access is locked, callback invocation is unlocked.
void CPetBody_DispatchCommandCallbacks(void* self, void* commandText);
#pragma pop

struct Scratch {
    volatile char flag;
    void* volatile linkCopy;
    void* volatile link;
    char* volatile copy;
    char* volatile pThird;
    void* volatile found;
    void* volatile foundCopy;
};

#pragma exceptions on
struct PetLinkGuard {
    void* list;
    void* node;
    // Guard an uncommitted list node, borrowing the list identity for its scope.
    PetLinkGuard(void* l, void* n) {
        void* saved = l;
        list = saved;
        node = n;
    }
    // Free the pending node unless insertion relinquished the guard's ownership.
    ~PetLinkGuard() { if (node) appFreeUni__FPv(node); }
};

struct PetLinkValue {
    void* value;
    // Placement construction borrows the caller-provided payload slot.
    static void* operator new(size_t, void* p) { return p; }
    // Initialize the node's attachment pointer without taking separate storage ownership.
    PetLinkValue(void* p) {
        try { value = p; }
        catch (...) { throw; }
    }
};

// Test equality of two borrowed attachment names using the engine string comparison.
inline bool PetNameMatches(const char* a, const char* b) {
    return CString_Compare(a, b) == 0;
};

#pragma exceptions on
#pragma opt_common_subs off
// Parse and register a unique named attachment, optionally restricted to the current animation.
void CPetBody_ParseAndRegisterAttachment(void* self, void* inout);
#pragma opt_common_subs reset
#pragma exceptions reset

// Initialize course-model containers, intrusive lists, pending filename and fog defaults.
void* WPolySoup_Ctor(void* self, void* entryContainer);

} // extern "C"

} // namespace ns1

extern "C" { void PetBody_InitBasisAndFullTurn(); }
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462188", _ctors_u801B9214_0, PetBody_InitBasisAndFullTurn);
#pragma pop



// Function definitions follow the original address order.

namespace ns0 {

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw on
extern "C" {
// Delete owned payloads and resources, release container storage, optionally delete self.
void* CPetBody_dtor(PetCleanupView* self, int deleting) {
    // Phase 1: delete callback payloads, then splice the live node ring into the
    // recycled ring and clear hash buckets. This does not free pooled node storage.
    if (self) {
        self->vtable = __vt__8CPetBody;
        void* callbackPayload;
        {
        WInstanceLock lock1(self->mutex);
        self->callbackCursor = self->callbackHead;
        callbackPayload = self->NextCallbackPayload();
        }
        if (callbackPayload) {
        do {
            if (callbackPayload) __dl__FPv(callbackPayload);
            callbackPayload = self->NextCallbackPayload();
        } while (callbackPayload);
        }
        {
        WInstanceLock lock2(self->mutex);
        if (self->callbackHead && self->recycledCallbackHead) {
            self->callbackHead->prev->next = self->recycledCallbackHead->next;
            self->recycledCallbackHead->next->prev = self->callbackHead->prev;
            self->recycledCallbackHead->next = self->callbackHead;
            self->callbackHead->prev = self->recycledCallbackHead;
            self->callbackHead = 0;
        } else if (self->callbackHead) {
            void* b = self->mutex;
            PetCallbackRingNode* cur;
            while ((cur = self->callbackHead) != 0) {
                OSLockMutex(b);
                cur->next->prev = cur->prev;
                cur->prev->next = cur->next;
                PetCallbackRingNode* next = cur->next;
                if (next == cur) next = 0;
                OSUnlockMutex(b);
                PetCallbackRingNode* value = self->callbackHead;
                PetCallbackRingNode* old = self->recycledCallbackHead;
                OSLockMutex(b);
                if (0 == old) {
                    value->prev = value;
                    value->next = value;
                    OSUnlockMutex(b);
                } else {
                    value->next = old;
                    PetCallbackRingNode* savedPrev = old->prev;
                    value->prev = savedPrev;
                    old->prev = value;
                    value->prev->next = value;
                    OSUnlockMutex(b);
                    value = old;
                }
                self->recycledCallbackHead = value;
                self->callbackHead = next;
            }
        }
        for (int i = 0; self->bucketCount > i; ++i)
            self->callbackBuckets[i] = 0;
        }
        {
        // Phase 2: delete attachment payloads/resources before freeing their list nodes.
        PetAttachmentListNode* sentinel = &self->attachmentSentinel;
        PetAttachmentListNode* savedNext = self->attachmentSentinel.next;
        PetAttachmentListNode* it = savedNext;
        while (it != sentinel) {
            PetAttachment* attachmentPayload = it->data;
            if (attachmentPayload) {
                if (attachmentPayload->resource) {
                    __dl__FPv(attachmentPayload->resource);
                    attachmentPayload->resource = 0;
                }
                __dl__FPv(attachmentPayload);
            }
            it = it->next;
        }
        }
        CleanupClearList(self);
        // Phase 3: release the retained puppet, heap name and render-parameter array.
        void* resource = self->puppetResource;
        if (resource) {
            ResourceManager_ReleaseObject(g_resourceManager, resource, 0);
            self->puppetResource = 0;
        }
        self->renderParameterCount = self->renderParameterCount - self->renderParameterCount;
        if (((char*)&AsCPetBody(self)->attachmentCount)) {
        if (((char*)&AsCPetBody(self)->attachmentCount)) {
        if (((char*)&AsCPetBody(self)->attachmentCount)) {
        if (((char*)&AsCPetBody(self)->attachmentCount)) {
        CleanupClearList(self);
        } } } }
        if (((char*)&AsCPetBody(self)->nameStringFlags)) if (self->stringFlags >> 31)
            appFreeUni__FPv(self->stringData);
        if (((char*)&AsCPetBody(self)->renderParameters)) if (((char*)&AsCPetBody(self)->renderParameters)) if (((char*)&AsCPetBody(self)->renderParameters)) if (((char*)&AsCPetBody(self)->renderParameters))
        if (self->renderParameterData) {
            int savedArrayCount = self->renderParameterCount;
            self->renderParameterCount = (PetRenderParameterSlot*)savedArrayCount - (PetRenderParameterSlot*)self->renderParameterCount;
            char* savedArray = self->renderParameterData;
            appFreeUni__FPv(savedArray);
        }
        // Phase 4: free pool allocation nodes and bucket storage; delete self only
        // for a positive deleting flag. Nested mutex acquisitions are recursive.
        if (((char*)&AsCPetBody(self)->instanceHead)) {
            WInstanceLock lock3(self->mutex);
            if (self->poolAllocationHead) {
            PetCallbackRingNode* cur;
            void* b = self->mutex;
            while ((cur = self->poolAllocationHead) != 0) {
                OSLockMutex(b);
                ((PetCallbackRingNode*)cur)->next->prev = ((PetCallbackRingNode*)cur)->prev;
                ((PetCallbackRingNode*)cur)->prev->next = ((PetCallbackRingNode*)cur)->next;
                PetCallbackRingNode* savedNext2 = cur->next;
                PetCallbackRingNode* next = savedNext2;
                if (cur == next) next = 0;
                OSUnlockMutex(b);
                self->poolAllocationHead = next;
                appFreeUni__FPv(cur);
            }
            }
            if (self->callbackBuckets) {
                void** savedBuckets = self->callbackBuckets;
                appFreeUni__FPv(savedBuckets);
                self->callbackBuckets = 0;
            }
        }
        if (0 < deleting) __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Clear playback flags, queued animation and look-at state; restore animation/root-motion speed scales.
void CPetBody_ResetPlaybackAndLookAt(void* self) {
    AsCPetBody(self)->flags = 0;
    *(unsigned int*)(((char*)&AsCPetBody(self)->followTarget)) = 0;
    Vec2_Zero(((char*)&AsCPetBody(self)->targetPitch));
    Vec2_Zero(((char*)&AsCPetBody(self)->lookAtPitch));
    float one = kFloat_1_petBody;
    AsCPetBody(self)->queuedAnimationName[0] = 0;
    AsCPetBody(self)->rootMotionScale = one;
    AsCPetBody(self)->animationSpeed = one;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Replace the stored bone name used when positioning the pet against the ground.
void CPetBody_SetNameString(void* self, const char* text) {
    char* s = (char*)self;
    unsigned int replacedLength;
    if ((*(unsigned int*)(s + 0x130) >> 31) == 0)
        replacedLength = *(unsigned char*)(s + 0x130) & 0x7F;
    else
        replacedLength = *(unsigned int*)(s + 0x134);
    volatile int value;
    unsigned char tmp = *((unsigned char*)&value);
    unsigned int length = strlen(text);
    String_ReplaceRange(((char*)&AsCPetBody(s)->nameStringFlags), 0, replacedLength, text, text + length, (char*)&tmp);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Apply animation and temporary head controllers, update matrices and bounds, then draw the visible pet.
void CPetBody_Draw(void* self, void* useViewDraw, void* drawEnabled) {
    const unsigned int* base = s_petBodyDrawStateDefaults;
    if (!(AsCPetBody(self)->flags & kPetHidden)) {
        void* entry;
        int changed = 0;
        // Phase 1: install a temporary Head adjustment only if none already exists.
        // 0x34 = playback(0x04) | matrix-dirty(0x10) | additional head-check bit(0x20).
        if (AsCPetBody(self)->flags & kPetNeedsHeadCheck) {
            entry = (void*)WPuppet_FindBoneByName(AsCPetBody(self)->puppet, petBodyStrings + kPetBody_headBone);
            if (entry && !WPuppet_HasBoneControllerEntry(AsCPetBody(self)->puppet, ((PetBoneControllerRecord*)entry)->controllerKey)) {
                float x = AsCPetBody(self)->lookAtPitch;
                x -= s_petLookAtPitchReference;
                x = fabsd(x);
                // Original unit 0x801B9214: update if either angle changed.
                float y;
                if (!lt(x, kFloat_0_00001_PetAnimation) ||
                    (y = AsCPetBody(self)->lookAtYaw,
                    y -= s_petLookAtYawReference, y = fabsd(y), !lt(y, kFloat_0_00001_PetAnimation)))
                {
                    Arr3 t;
                    Memory_Clear(&t, 0xC);
                    t.v[0] = -AsCPetBody(self)->lookAtYaw;
                    t.v[2] = -AsCPetBody(self)->lookAtPitch;
                    changed = 1;
                    WPuppet_SetBoneControllerVector(AsCPetBody(self)->puppet, ((PetBoneControllerRecord*)entry)->controllerKey, &t);
                }
            }
        }
        unsigned int flags = AsCPetBody(self)->flags;
        // Phase 2: refresh pose, world matrices and bounds when playing or dirty.
        // flags bit 1 is forwarded as the animation routine's fourth argument.
        if (flags & kPetNeedsAnimationOrMatrix) {
            ((void(*)(void*, int, float, int, int))WPuppet_SetAnimationTimeFromRanges)(AsCPetBody(self)->puppet, 0, AsCPetBody(self)->animationTime, (AsCPetBody(self)->flags >> 1) & 1, 0);
            if (!(AsCPetBody(self)->flags & kPetUsesRootMotion)) {
                Model_SetMatrixRecursive(AsCPetBody(self)->puppet, AsCPetBody(self)->puppet, ((char*)&AsCPetBody(self)->matrixBasis));
            } else {
                void* range = AsCPetBody(self)->currentAnimation;
                float t = AsCPetBody(self)->animationTime - *(float*)range;
                float frame = (float)(int)(kPetAnimationFramesPerSecond * t + kFloat_0_5_PetBody);
                Arr3 matrix;
                ((void(*)(void*, void*, void*, float, float))WPuppet_GetRootMotionDelta)(&matrix, AsCPetBody(self)->puppet, ((PetAnimationRange*)range)->name, kFloat_0_petBody, frame);
                matrix.v[2] *= kFloat_neg1_PetBody;
                // Root motion is local-space with Z reversed; rotate it into
                // world space and subtract only while propagating the model matrix.
                // Restore the pet's stored world translation immediately afterward.
                Arr3 out = AsCPetBody(self)->matrixPosition;
                WVector tmp;
                Matrix_TransformDirection(&tmp, &matrix, ((char*)&AsCPetBody(self)->matrixBasis));
                *(WVector*)(((char*)&AsCPetBody(self)->matrixPosition)) -= tmp;
                Model_SetMatrixRecursive(AsCPetBody(self)->puppet, AsCPetBody(self)->puppet, ((char*)&AsCPetBody(self)->matrixBasis));
                ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(((char*)&AsCPetBody(self)->matrixPosition), &out);
            }
            WPuppet_UpdateBoneWorldMatrices(AsCPetBody(self)->puppet, 0, 0, kFloat_1_petBody);
            WPuppet_UpdateBoundsFromBone(AsCPetBody(self)->puppet);
        }
        // Phase 3: remove only this call's temporary head adjustment after matrix updates.
        if (changed) {
            entry = (void*)WPuppet_FindBoneByName(AsCPetBody(self)->puppet, petBodyStrings + kPetBody_headBone);
            if (entry) WPuppet_EraseBoneControllerEntry(AsCPetBody(self)->puppet, ((PetBoneControllerRecord*)entry)->controllerKey);
        }
        entry = (char*)&AsWPuppet(AsCPetBody(self)->puppet)->boundsCenter;
        if (WView_DefaultCapabilityTrueA(g_view, entry)) {
            unsigned int f2 = AsCPetBody(self)->flags;
            // Phase 4: prepare lighting/directional parameters before either draw path.
            if (f2 & kPetNeedsAnimationOrMatrix) {
                if (AsCPetBody(self)->defaultRenderParametersEnabled) {
                    ((PetDevice*)AsCPetBody(self)->puppet)->v5(((char*)&AsCPetBody(self)->lightingMode), 0, 0);
                }
                if (AsCPetBody(self)->renderParametersEnabled) {
                    void* p = AsCPetBody(self)->renderParameters;
                    while (p != (char*)AsCPetBody(self)->renderParameters + AsCPetBody(self)->renderParameterCount * 0x28) {
                        WVector tmp;
                        Matrix_TransformDirection(&tmp, (char*)&((PetRenderParameter*)p)->sourceDirection, ((char*)&AsCPetBody(self)->matrixBasis));
                        ((void(*)(void*, void*))__as__7WVectorFRC7WVector)((char*)&((PetRenderParameter*)p)->transformedDirection, &tmp);
                        ((PetDevice*)AsCPetBody(self)->puppet)->v5(p, 1, 0);
                        p = (char*)p + sizeof(PetRenderParameter);
                    }
                }
            }
            if (drawEnabled) {
                if (useViewDraw) {
                    void* elem = WView_DefaultAcceptVisibility(g_view, entry);
                    ((PetDevice*)AsCPetBody(self)->puppet)->v6(g_view, 1, elem, 0, AsCPetBody(self)->uniformScale, 0);
                } else {
                    Blob26 c;
                    Blob12 b;
                    Blob10 a;
                    c = *(Blob26*)base;
                    b = *(Blob12*)((char*)&((PetRenderDefaults*)base)->stageStates);
                    a = *(Blob10*)((char*)&((PetRenderDefaults*)base)->samplerStates);
                    Engine_SetRenderStateList(&c, 13);
                    Engine_SetTextureStageStateList(0, &b, 6);
                    Engine_SetTextureStageParameterList(0, &a, 5);
                    float zeroCameraOffset = kFloat_0_petBody;
                    Camera_Apply(g_view, zeroCameraOffset, zeroCameraOffset);
                    void* r28v = Camera_GetRenderState(g_view);
                    SetTransform(1, (char*)&((PetCameraMatrices*)r28v)->projection);
                    gtl_GenericRenderer_SetProjectionMatrix((char*)&((PetCameraMatrices*)r28v)->projection);
                    SetTransform(0, (char*)&((PetCameraMatrices*)r28v)->view);
                    gtl_GenericRenderer_SetViewMatrix((char*)&((PetCameraMatrices*)r28v)->view);
                    void* r9 = WPuppet_GetSkinMatrices(AsCPetBody(self)->puppet);
                    ((PetDevice*)AsCPetBody(self)->puppet)->v10(g_view, 1, 1, AsCPetBody(self)->uniformScale, 0, 0, r9, 0);
                }
            }
        }
        AsCPetBody(self)->flags &= ~kPetLookAtDirty;
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Use course lighting for golf tasks, otherwise set a normalized default light and white colors.
void CPetBody_SetupLighting(void* self) {
    char* s = (char*)self;
    if (AsCPetBody(s)->puppet) WPuppet_UpdateBound(AsCPetBody(s)->puppet, 0);
    AsCPetBody(s)->renderParametersEnabled = 1;
    void* obj = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    g_petBodyTaskChild = obj;
    if (FindInChain(obj)) {
        void* p = AsCPetBody(s)->puppet;
        if (p) {
            float v[3];
            float z = AsCPetBody(s)->positionZ;
            float y = AsWPuppet(p)->boundsMax.y;
            float x = AsCPetBody(s)->positionX;
            v[0] = x;
            v[1] = y;
            v[2] = z;
            float brightness = CQuadTree_DefaultLightingFactor(CQuadTree_GetInstance(v[1], v[2]), v);
            *(void**)(s + kPetByteOffset_lightColor) = Color_ScaleRGB((void*)s_sceneLighting[4], brightness);
        }
        *(unsigned int*)(s + kPetByteOffset_ambientLightColor) = s_sceneLighting[5];
        ((void(*)(void*, void*))__as__7WVectorFRC7WVector)((s + kPetByteOffset_lightDirectionX), s_sceneLighting + 1);
    } else {
        struct Vector {
            float x, y, z;
    // Store the three vector components supplied by the lighting initializer.
    Vector(float a, float b, float c) : x(a), y(b), z(c) {}
        };
        unsigned int k = 0xFFFFFF;
        *(unsigned int*)(s + kPetByteOffset_lightingMode) = 2;
        *(unsigned int*)(s + kPetByteOffset_lightColor) = k;
        *(unsigned int*)(s + kPetByteOffset_ambientLightColor) = k;
        Vector v(kFloat_0_petBody, kFloat_neg1_PetBody, kFloat_1_petBody);
        ((void(*)(void*, void*))__as__7WVectorFRC7WVector)((s + kPetByteOffset_lightDirectionX), &v);
        (*(WVector*)(s + kPetByteOffset_lightDirectionX)).Normalize();
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Consume sampled root motion once, update flight steering, and synchronize translations.
void CPetBody_Update(void* self, float dt) {
    char* s = (char*)self;
    unsigned int flags = AsCPetBody(s)->flags;
    if (flags & kPetAnimationPlaying) {
        CPetBody_UpdateAnimation(self, dt * AsCPetBody(s)->animationSpeed);
        flags = AsCPetBody(s)->flags;
        if (flags & kPetUsesRootMotion) {
            if (flags & kPetRootMotionSampled) {
                // Sampling uses puppet-local axes. Rotate into world axes, scale,
                // consume the sample flag, and add this interval's displacement.
                char b[0x0C], a[0x0C];
                char* p14 = s + kPetByteOffset_baseMatrixBasis;
                char* p13c = s + kPetByteOffset_rootMotionDeltaX;
                unsigned int f = *(volatile unsigned int*)(s + kPetByteOffset_flags);
                f = f & ~kPetRootMotionSampled;
                AsCPetBody(s)->flags = f;
                Matrix_TransformDirection(a, p13c, p14);
                __ml__FfRC7WVector(b, a, AsCPetBody(s)->rootMotionScale);
                ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(s + kPetByteOffset_movementDirection, b);
                __apl__7WVectorFRC7WVector(s + kPetByteOffset_positionX, s + kPetByteOffset_movementDirection);
            }
            CPetBody_UpdateMovement(self, dt);
            // Movement may change Y and lateral position after the root-motion step.
            ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(s + kPetByteOffset_baseMatrixPositionX, s + kPetByteOffset_positionX);
            ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(s + kPetByteOffset_matrixPosition, s + kPetByteOffset_positionX);
        }
    }
    void* obj = AsCPetBody(s)->puppet;
    if (obj && WPuppet_FindBoneByName(obj, petBodyStrings + kPetBody_headBone)) CPetBody_UpdateLookAt(self, dt);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Dispatch interval events, advance time, and recursively spend overflow in the next clip.
void CPetBody_UpdateAnimation(void* self, float dt) {
    char* petBytes = (char*)self;
    if (!AsCPetBody(petBytes)->currentAnimation || ((PetAnimationRange*)AsCPetBody(petBytes)->currentAnimation)->startTime == ((PetAnimationRange*)AsCPetBody(petBytes)->currentAnimation)->endTime) {
        // Missing or zero-duration clips stop both playback and root motion.
        AsCPetBody(petBytes)->flags = AsCPetBody(petBytes)->flags & ~(kPetAnimationPlaying | kPetUsesRootMotion);
        return;
    }
    float step = CPetBody_ComputeAnimationStep(self, dt);
    if (!(step > kFloat_0_petBody)) return;
    // Phase 1: notify events for the bounded interval before publishing its end time.
    float intervalStart = AsCPetBody(petBytes)->animationTime;
    float eventStep = step;
    float eventEnd = intervalStart + eventStep;
    AnimationEventParser_ProcessFrameRange(self, AsCPetBody(petBytes)->puppet, intervalStart, eventEnd, AsCPetBody(petBytes)->currentAnimation, -1, 0);
    float advanceStep = step;
    float intervalEnd = AsCPetBody(petBytes)->animationTime + advanceStep;
    float publishedTime = intervalEnd;
    AsCPetBody(petBytes)->animationTime = publishedTime;
    if (intervalEnd >= ((PetAnimationRange*)AsCPetBody(petBytes)->currentAnimation)->endTime) {
        // Phase 2: a queued clip takes priority over the current range's repeat name.
        // dt-step is unspent time. The recursive call can cross several short clips.
        if (*(signed char*)(petBytes + kPetByteOffset_label)) {
            CPetBody_PlayAnimationByName(self, dt - step, petBytes + kPetByteOffset_label, AsCPetBody(petBytes)->queuedAnimationUsesRootMotion, 0);
            AsCPetBody(petBytes)->queuedAnimationName[0] = 0;
        } else {

            float repeatStep = step;
            CPetBody_PlayAnimationByName(self, dt - repeatStep, (void*)((PetAnimationRange*)AsCPetBody(petBytes)->currentAnimation)->repeatName, ((AsCPetBody(petBytes)->flags) >> 1) & 1, 1);
            *(unsigned int*)(petBytes + kPetByteOffset_animationFinished) = 1;
        }
        float consumedStep = step;
        CPetBody_UpdateAnimation(self, dt - consumedStep);
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Clamp the interval and sample puppet-local root displacement using rounded frame indices.
float CPetBody_ComputeAnimationStep(void* self, float dt) {
    float maximum = ((PetAnimationRange*)AsCPetBody(self)->currentAnimation)->maximumTime;
    float end = AsCPetBody(self)->animationTime + dt;
    end = (maximum > end) ? end : maximum;
    float additionalRootMotion[3];
    unsigned int flags = AsCPetBody(self)->flags;
    if (flags & kPetUsesRootMotion) {
        if (!(flags & kPetRootMotionSampled)) {
            // First interval seeds the accumulator; later intervals add to it until
            // vfunc_27 consumes it. Flip puppet Z into the pet's displacement axes.
            float rangeOffset = AsCPetBody(self)->animationTime - ((PetAnimationRange*)AsCPetBody(self)->currentAnimation)->startTime;
            float sampledRootMotion[3];
            float savedEnd = end;
            float savedRangeOffset = rangeOffset;
            int firstFrame = (int)(kPetAnimationFramesPerSecond*savedRangeOffset+kFloat_0_5_PetBody);
            int lastFrame = (int)((savedEnd - ((PetAnimationRange*)AsCPetBody(self)->currentAnimation)->startTime)*kPetAnimationFramesPerSecond+kFloat_0_5_PetBody);
            WPuppet_GetRootMotionDelta(sampledRootMotion,(float)firstFrame, AsCPetBody(self)->puppet, (float)lastFrame, ((PetAnimationRange*)AsCPetBody(self)->currentAnimation)->name);
            ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(((char*)&AsCPetBody(self)->rootMotionDeltaX), sampledRootMotion);
            AsCPetBody(self)->rootMotionDeltaZ *= kFloat_neg1_PetBody;
            AsCPetBody(self)->flags |= kPetRootMotionSampled;
        } else {
            WPuppet_SetAnimationTimeFromRanges(AsCPetBody(self)->puppet, 0, AsCPetBody(self)->animationTime, (2 & flags) != 0, 0);
            float rangeOffset = AsCPetBody(self)->animationTime - ((PetAnimationRange*)AsCPetBody(self)->currentAnimation)->startTime;
            float savedEnd2 = end;
            int firstFrame = (int)(kPetAnimationFramesPerSecond*rangeOffset+kFloat_0_5_PetBody);
            int lastFrame = (int)(kPetAnimationFramesPerSecond*(savedEnd2 - ((PetAnimationRange*)AsCPetBody(self)->currentAnimation)->startTime)+kFloat_0_5_PetBody);
            WPuppet_GetRootMotionDelta(additionalRootMotion, (float)firstFrame, AsCPetBody(self)->puppet, (float)lastFrame, ((PetAnimationRange*)AsCPetBody(self)->currentAnimation)->name);
            additionalRootMotion[2] *= kFloat_neg1_PetBody;
            *(WVector*)(((char*)&AsCPetBody(self)->rootMotionDeltaX)) += *(WVector*)additionalRootMotion;
        }
    }
    return end - AsCPetBody(self)->animationTime;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Copy a borrowed world-space XYZ vector and synchronize both matrix translations.
void CPetBody_SetPosition(void* self, const void* sourcePosition) {
    char* s = (char*)self;
    ((void (*)(void*, const void*))__as__7WVectorFRC7WVector)(s + kPetByteOffset_positionX, sourcePosition);
    ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(s + kPetByteOffset_baseMatrixPositionX, s + kPetByteOffset_positionX);
    ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(s + kPetByteOffset_matrixPosition, s + kPetByteOffset_positionX);
    unsigned int flags = AsCPetBody(s)->flags;
    flags |= kPetMatrixDirty;
    flags &= ~2u;
    AsCPetBody(s)->flags = flags;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Copy borrowed Euler angles (radians), compose X*Y*Z, and retain world translation.
void CPetBody_SetRotation(void* self, const void* sourceEulerRadians) {
    char* s = (char*)self;
    ((void(*)(void*, const void*))__as__7WVectorFRC7WVector)(s + kPetByteOffset_rotationX, sourceEulerRadians);
    char e[0x30], d[0x30], c[0x30], b[0x30], a[0x30];
    Matrix_SetAxisRotation(a, AsCPetBody(s)->rotationZ, 2);
    Matrix_SetAxisRotation(b, AsCPetBody(s)->rotationY, 1);
    Matrix_SetAxisRotation(c, AsCPetBody(s)->rotationX, 0);
    Matrix_ConcatAffine(d, c, b);
    Matrix_ConcatAffine(e, d, a);
    Matrix_CopyAffine(s + kPetByteOffset_baseMatrixBasis, e);
    ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(s + kPetByteOffset_baseMatrixPositionX, s + kPetByteOffset_positionX);
    Matrix_CopyAffine(s + kPetByteOffset_matrixBasis, s + kPetByteOffset_baseMatrixBasis);
    AsCPetBody(s)->flags |= kPetMatrixDirty;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Rebuild base/current orientation for the requested yaw and clear movement direction.
void CPetBody_SetYaw(char* self, float value) {
    float zero = kFloat_0_petBody;
    char a[0x18];
    char b[0x30];
    struct Pair { unsigned int words[2]; };
    *(Pair*)(a + 0x0C) = *(Pair*)&AsCPetBody(self)->baseMatrixPositionX;
    unsigned int v2 = AsCPetBody(self)->baseMatrixPositionZ;
    *(unsigned int*)(a + 0x14) = v2;
    ((float*)a)[0] = zero;
    ((float*)a)[1] = value;
    ((float*)a)[2] = zero;
    ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(((char*)&AsCPetBody(self)->rotationX), a);
    Matrix_SetAxisRotation(b, value, 1);
    Matrix_CopyAffine(((char*)&AsCPetBody(self)->baseMatrixBasis), b);
    ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(((char*)&AsCPetBody(self)->baseMatrixPositionX), a + 0x0C);
    Matrix_CopyAffine(((char*)&AsCPetBody(self)->matrixBasis), ((char*)&AsCPetBody(self)->baseMatrixBasis));
    AsCPetBody(self)->flags |= kPetMatrixDirty;
    Vec3_Zero(((char*)&AsCPetBody(self)->movementDirection));
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Compose an external rotation with the pet's normalized rotation while retaining translation.
void CPetBody_ApplyRotationMatrix(void* self, void* rotationMatrix) {
    char* s = (char*)self;
    if (rotationMatrix) {
        unsigned int params[3];
        char converted[0x30];
        char result[0x30];
        struct Pair { unsigned int words[2]; };
        *(Pair*)params = *(Pair*)(s + kPetByteOffset_matrixPosition);
        params[2] = *(unsigned int*)(s + kPetByteOffset_matrixPosition_v_2_);
        Matrix_ExtractUniformScaleNormalizedRotation(s + kPetByteOffset_matrixBasis, converted);
        Matrix_ConcatAffine(result, converted, rotationMatrix);
        Matrix_CopyAffine(s + kPetByteOffset_matrixBasis, result);
        ((void(*)(void*, void*))__as__7WVectorFRC7WVector)((s + kPetByteOffset_matrixPosition), params);
        AsCPetBody(s)->flags |= kPetMatrixDirty;
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Set the movement destination and initialize flight pitch, height and vertical speed.
void CPetBody_StartMovement(void* self, void* destination) {
    AsCPetBody(self)->movementState = 0;
    ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(((char*)&AsCPetBody(self)->movementStart), ((char*)&AsCPetBody(self)->positionX));
    ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(((char*)&AsCPetBody(self)->movementTarget), destination);
    float c;
    float b;
    float a;
    a = kFloat_neg0_7853982_PetBody;
    b = kFloat_0_petBody;
    c = kFloat_50_PetBody;
    AsCPetBody(self)->movementPitchRadians = a;
    AsCPetBody(self)->minimumHeight = b;
    AsCPetBody(self)->verticalVelocity = c;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Resolve pet sound commands and play at the pet position with task-specific sound parameters.
void CPetBody_PlaySound(void* self, char* text) {
    if (text) {
        char copy[0x0C]; 
        WVector temp = *(WVector*)(((char*)&AsCPetBody(self)->positionX)) - *(WVector*)(s_cameraTransform + 0x24);
        ((void(*)(void*, void*))__as__7WVectorFRC7WVector)(copy, &temp);
        char* marker = strchr(text, 0x2D);
        if (marker) {
            unsigned int length = marker + 1 - text;
            if (!strncmp(marker + 1, petBodyStrings + kPetBody_chanceMarker, 4)) {
                int difference;
                void* manager = g_sharedDoc;
                if (AsCSharedDoc(manager)->golfRuleType == 1) {
                    void* data = ((void*)AsCSharedDoc(manager)->golfDoc);
                    unsigned char index = AsCGolfDoc(data)->currentHoleIndex;
                    unsigned char* table = (unsigned char*)GolfPlayer_GetEntryByIndex(data, AsCGolfDoc(data)->currentPlayer);
                    void* manager2 = g_sharedDoc;
                    void* data2 = ((void*)AsCSharedDoc(manager2)->golfDoc);
                    index = AsCGolfDoc(data2)->currentHoleIndex;
                    unsigned char* base = (unsigned char*)AsCGolfDoc(data2)->holeRecords;
                    unsigned char* tp = table + index;
                    unsigned char* bp = base;
                    difference = tp[0x28] - bp[index * 0x20 + 0x18];
                } else {
                    void* data = ((void*)AsCSharedDoc(manager)->golfDoc);
                    unsigned char index = AsCGolfDoc(data)->currentHoleIndex;
                    unsigned char* table = (unsigned char*)GolfPlayer_GetEntryByIndex(data, AsCGolfDoc(data)->currentPlayer);
                    void* manager2 = g_sharedDoc;
                    void* data2 = ((void*)AsCSharedDoc(manager2)->golfDoc);
                    index = AsCGolfDoc(data2)->currentHoleIndex;
                    unsigned char* base = (unsigned char*)AsCGolfDoc(data2)->holeRecords;
                    unsigned char* tp = table + index;
                    unsigned char* bp = base;
                    difference = tp[0x28] - bp[index * 0x20 + 0x18];
                }
                if (*((unsigned char*)GolfBall__Fv() + 0x59) == 2 && (unsigned int)(difference + 2) <= 1u) {
                    char rebuilt[0x40];
                    strncpy(rebuilt, text, length);
                    switch (difference) {
                    case -1: strcpy(rebuilt + length, petBodyStrings + kPetBody_birdieText); break;
                    case -2: strcpy(rebuilt + length, petBodyStrings + kPetBody_scoreTextSuffix); break;
                    }
                    strcat(rebuilt, text + length);
                    strcpy(text, rebuilt);
                }
            }
        }
    }
    void* obj = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    g_petBodyTaskChild = obj;
    int found = 0;
    if (obj) {
        void* q = ((void* (**)(void*))(*(void**)obj))[2](obj);
        while (q) {
            if (q == &s_golfTaskTypeInfo) { found = 1; break; }
            q = ((PetTypeInfoLink*)q)->parent;
        }
    }
    if (found) {
        unsigned int pair[2] = {kPetPositionalSoundOptionWord0, kFloat_1000_PetSound};
        Sound_PlayByName(g_soundManager, text, ((char*)&AsCPetBody(self)->positionX), 0, 0, pair, kFloat_0_5_PetBody, kFloat_200_PetBody, kFloat_1_petBody);
    } else {
        Sound_PlayByName(g_soundManager, text, 0, 0, 0, 0, kFloat_0_5_PetBody, kFloat_200_PetBody, kFloat_1_petBody);
    }
}
}
#pragma pop

}

namespace ns1 {

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Resolve the next clip, dispatch any skipped events and reconcile root-motion ownership.
// animationName is borrowed; the two pointer-shaped flags are tested only for nonzero.
// startOffset is in animation time units; frame sampling multiplies by the stored FPS.
void CPetBody_PlayAnimationByName(void* self, float startOffset, void* animationName, void* useRootMotion, void* preserveRootMotion) {
    void* animation = AsCPetBody(self)->currentAnimation;
    AsCPetBody(self)->animationFinished = 1;
    AsCPetBody(self)->currentAnimation = WPuppet_FindAnimationRangeByName(AsCPetBody(self)->puppet, animationName);
    if (AsCPetBody(self)->currentAnimation == 0) {
        AsCPetBody(self)->currentAnimation = WPuppet_FindAnimationRangeByName(AsCPetBody(self)->puppet, (void*)(petBodyStrings + kPetBody_facialAnimationMarker));
    }
    if (AsCPetBody(self)->currentAnimation != 0) {
        // Phase 1: clamp the entry time and deliver events skipped by the offset.
        AsCPetBody(self)->flightMovementEnabled = 0;
        float end = *((float*)(AsCPetBody(self)->currentAnimation) + 1);
        float time = *(float*)(AsCPetBody(self)->currentAnimation) + startOffset;
        AsCPetBody(self)->animationTime = time > end ? end : time;
        AsCPetBody(self)->animationTime = kFloat_0_petBody < AsCPetBody(self)->animationTime ? AsCPetBody(self)->animationTime : kFloat_0_petBody;
        if (AsCPetBody(self)->animationTime > *(float*)(AsCPetBody(self)->currentAnimation)) {
            ((void(*)(void*, void*, float, float, void*, int, int))AnimationEventParser_ProcessFrameRange)(self, AsCPetBody(self)->puppet, *(float*)(AsCPetBody(self)->currentAnimation), AsCPetBody(self)->animationTime, AsCPetBody(self)->currentAnimation, -1, 0);
        }
        AsCPetBody(self)->flags |= kPetAnimationPlaying;
        if (useRootMotion) {
            AsCPetBody(self)->flags |= kPetUsesRootMotion;
        } else {
            // Phase 2: with root motion disabled and preservation requested, add
            // the old clip's inverse terminal offset to keep world position continuous.
            // This runs only if an old clip exists and the new clip has nonzero duration.
            int moving = 0;
            int active = 0;
            if (animation && preserveRootMotion) active = 1;
            if (active) {
                void* current = AsCPetBody(self)->currentAnimation;
                int small = AnimationAbs(*((float*)current + 1) - *(float*)current) < kFloat_0_00001_PetAnimation;
                if (!small) moving = 1;
            }
            if (moving) {

                float transformed[3];

                WVector sampled;
                WPuppet_GetInverseRootMotionOffsetForRange(&sampled, AsCPetBody(self)->puppet, (float)(int)(kPetAnimationFramesPerSecond * (*((float*)animation + 1) - *(float*)animation) + kFloat_0_5_PetBody), (((int)((PetAnimationRange*)animation)->name)));

                *(WVector*)transformed = sampled;
                WVector result;

                Matrix_TransformDirection(&result, transformed, ((char*)&AsCPetBody(self)->baseMatrixBasis));

                *(WVector*)transformed = result;
                transformed[2] *= kFloat_neg1_PetBody;
                float savedLbl_805DF5AC = transformed[0] * kFloat_neg1_PetBody;
                transformed[0] = savedLbl_805DF5AC;
                 WVector rotated = *(WVector*)transformed * (AsCPetBody(self)->rootMotionScale);
                *(WVector*)transformed = rotated;

                WVector diff = *(WVector*)(((char*)&AsCPetBody(self)->positionX)) + *(WVector*)transformed;
                *(WVector*)(((char*)&AsCPetBody(self)->positionX)) = diff;
                *(WVector*)(((char*)&AsCPetBody(self)->baseMatrixPositionX)) = *(WVector*)(((char*)&AsCPetBody(self)->positionX));
                *(WVector*)(((char*)&AsCPetBody(self)->matrixPosition)) = *(WVector*)(((char*)&AsCPetBody(self)->positionX));
                AsCPetBody(self)->flags |= kPetMatrixDirty;
                AsCPetBody(self)->flags &= ~2u;
            }
            AsCPetBody(self)->flags &= ~2u;
        }
        AsCPetBody(self)->animationFinished = 0;
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Apply uniform scale to the pet and loaded puppet matrices.
void CPetBody_SetUniformScale(void* self, float value) {
    AsCPetBody(self)->uniformScale = value;
    float vec[3] = {value, value, value};
    Matrix_SetBasisScale(((char*)&AsCPetBody(self)->matrixBasis), vec);
    void* other = AsCPetBody(self)->puppet;
    if (other != 0) {
        Matrix_SetBasisScale((char*)&AsWPuppet(other)->worldMatrix, vec);
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Resolve head-relative pitch and body-relative yaw from the same borrowed world target.
static inline void Pet_ResolveLookAtAngles(void* self, float* headToTargetWorld,
    float* bodyToTargetWorld, float& yaw) {
    __mi__FRC7WVectorRC7WVector(headToTargetWorld, AsCPetBody(self)->followTarget,
                (char*)((PetLookAtBone*)(unsigned int)WPuppet_FindBoneByName(AsCPetBody(self)->puppet, petBodyStrings + kPetBody_headBone))->worldOrigin);
    AsCPetBody(self)->targetPitch = Pet_ClampLookAtPitch(
        (float)atan((double)(headToTargetWorld[1] / Math_SquareRootApprox(headToTargetWorld[0] * headToTargetWorld[0] + headToTargetWorld[2] * headToTargetWorld[2]))));
    __mi__FRC7WVectorRC7WVector(bodyToTargetWorld, AsCPetBody(self)->followTarget, ((char*)&AsCPetBody(self)->baseMatrixPositionX));
    yaw = Vector_SignedAngleXZ(bodyToTargetWorld, ((char*)&AsCPetBody(self)->baseMatrixBasis[6]));
    AsCPetBody(self)->targetYaw = yaw;
}

// Rebuild the world body frame from stored Euler angles and publish both base/current copies.
// concat order is (Rx * Ry) * Rz; translation is restored from the pet's world position.
// The base frame excludes animated/root-motion offsets; current starts as its exact copy.
static inline void Pet_RebuildBodyFrame(void* self, float* rotationZMatrix, float* rotationYMatrix,
    float* rotationXMatrix, float* rotationXYMatrix, float* rotationXYZMatrix) {
    Matrix_SetAxisRotation(rotationZMatrix, AsCPetBody(self)->rotationZ, kPetAxisZ);
    Matrix_SetAxisRotation(rotationYMatrix, AsCPetBody(self)->rotationY, kPetAxisY);
    Matrix_SetAxisRotation(rotationXMatrix, AsCPetBody(self)->rotationX, kPetAxisX);
    Matrix_ConcatAffine(rotationXYMatrix, rotationXMatrix, rotationYMatrix);
    Matrix_ConcatAffine(rotationXYZMatrix, rotationXYMatrix, rotationZMatrix);
    Matrix_CopyAffine(((char*)&AsCPetBody(self)->baseMatrixBasis), rotationXYZMatrix);
    __as__7WVectorFRC7WVector(((char*)&AsCPetBody(self)->baseMatrixPositionX), ((char*)&AsCPetBody(self)->positionX));
    Matrix_CopyAffine(((char*)&AsCPetBody(self)->matrixBasis), ((char*)&AsCPetBody(self)->baseMatrixBasis));
    AsCPetBody(self)->flags |= kPetMatrixDirty;
}

// Smooth head pitch/yaw toward a world target, with optional body yaw beyond the head's range.
// delta is the unscaled update interval forwarded by vfunc_27 (animationSpeed affects clips only).
// Pitch originates at the Head bone's world translation; yaw uses the base body origin/+Z axis.
// Requires a Head bone and a borrowed target XYZ that stays alive through this call.
// No coincident-target guard is present: zero horizontal distance reaches atan(dY / 0),
// including 0/0 when target=head. Preserve that behavior when editing the smoothing policy.
void CPetBody_UpdateLookAt(void* self, float delta) {
    if (AsCPetBody(self)->followTarget != 0) {
        float headToTargetWorld[3];
        float bodyToTargetWorld[3];
        float nextBodyEuler[3];
        float bodyYawDelta[3];
        float rotationZMatrix[12];
        float rotationYMatrix[12];
        float rotationXMatrix[12];
        float rotationXYMatrix[12];
        float rotationXYZMatrix[12];
        float yaw;
        // Phase 1: derive pitch from the head origin and yaw from the base body frame.
        Pet_ResolveLookAtAngles(self, headToTargetWorld, bodyToTargetWorld, yaw);
        // Phase 2: turn the body only by yaw beyond the 45-degree head range.
        if (AsCPetBody(self)->turnToTarget != 0) {
            float absYaw = AsCPetBody(self)->targetYaw;
            absYaw = absYaw > kFloat_0_petBody ? absYaw : -absYaw;
            if (absYaw > kFloat_0_7853982_PetBody) {
                float absoluteTargetYaw = AsCPetBody(self)->targetYaw;
                absoluteTargetYaw = absoluteTargetYaw > kFloat_0_petBody ? absoluteTargetYaw : -absoluteTargetYaw;
                float excessYaw = absoluteTargetYaw - kFloat_0_7853982_PetBody;
                excessYaw = AsCPetBody(self)->targetYaw > kFloat_0_petBody ? excessYaw : -excessYaw;
                bodyYawDelta[0] = kFloat_0_petBody;
                bodyYawDelta[1] = excessYaw;
                bodyYawDelta[2] = kFloat_0_petBody;
                __pl__FRC7WVectorRC7WVector(nextBodyEuler, ((char*)&AsCPetBody(self)->rotationX), bodyYawDelta);
                __as__7WVectorFRC7WVector(((char*)&AsCPetBody(self)->rotationX), nextBodyEuler);
                Pet_RebuildBodyFrame(self, rotationZMatrix, rotationYMatrix, rotationXMatrix,
                    rotationXYMatrix, rotationXYZMatrix);
            }
        }
        AsCPetBody(self)->targetYaw = Pet_ClampLookAtYaw(AsCPetBody(self)->targetYaw);
    } else {
        Vec2_Zero(((char*)&AsCPetBody(self)->targetPitch));
    }
    // Phase 3: advance head pitch/yaw at pi/2 radians per update-time unit, with a 1-degree deadband.
    float remainingHeadAngles[2];
    FloatPair headAngleDelta = Vec2_Subtract(((char*)&AsCPetBody(self)->targetPitch), ((char*)&AsCPetBody(self)->lookAtPitch));
    remainingHeadAngles[0] = headAngleDelta.first;
    remainingHeadAngles[1] = headAngleDelta.second;
    float remainingPitch = remainingHeadAngles[0];
    if ((remainingPitch > kFloat_0_petBody ? remainingPitch : -remainingPitch) > kFloat_0_017453292_PetBody) {
        AsCPetBody(self)->flags |= kPetHeadAnglesDirty;
        float remainingAngleLength = Vec2_CalculateLength(remainingHeadAngles);
        float stepFraction = kFloat_1_5707964_PetMotion * delta / remainingAngleLength;
        stepFraction = (stepFraction > kFloat_1_petBody) ? kFloat_1_petBody : stepFraction;
        AsCPetBody(self)->lookAtPitch += remainingHeadAngles[0] * stepFraction;
    }
    if ((remainingHeadAngles[1] > kFloat_0_petBody ? remainingHeadAngles[1] : -remainingHeadAngles[1]) > kFloat_0_017453292_PetBody) {
        AsCPetBody(self)->flags |= kPetHeadAnglesDirty;
        float remainingAngleLength = Vec2_CalculateLength(remainingHeadAngles);
        float stepFraction = kFloat_1_5707964_PetMotion * delta / remainingAngleLength;
        stepFraction = (stepFraction > kFloat_1_petBody) ? kFloat_1_petBody : stepFraction;
        AsCPetBody(self)->lookAtYaw += remainingHeadAngles[1] * stepFraction;
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Project world-space ball error onto up x travel, then add the delta-scaled lateral step.
// Scratch vectors stay in the caller to retain CW stack slots; there is no forward/overshoot correction.
static inline void Pet_ApplyLateralBallCorrection(void* self, void* ball, float delta,
    PetVector& lateralAxis, PetVector& up, PetVector& ballOffset, PetVector& steeringStep,
    PetVector& steeringVelocity, PetVector& lateralError) {
    Vector_CrossProduct(&lateralAxis, &up, ((char*)&AsCPetBody(self)->movementDirection)); Normalize__7WVectorFv(&lateralAxis);
    __mi__FRC7WVectorRC7WVector(&ballOffset, ball, ((char*)&AsCPetBody(self)->positionX));
    float projection = __ml__FRC7WVectorRC7WVector(&ballOffset, &lateralAxis);
    __ml__FRC7WVectorf(&lateralError, &lateralAxis, projection);
    __ml__FRC7WVectorf(&steeringVelocity, &lateralError, kFloat_1_2_PetBody);
    __ml__FRC7WVectorf(&steeringStep, &steeringVelocity, delta);
    __apl__7WVectorFRC7WVector(((char*)&AsCPetBody(self)->positionX), &steeringStep);
}

// Advance takeoff/cruise/landing height policy, then steer laterally toward the ball.
// World positions are Y-up. delta is vfunc_27's seconds, unscaled by animationSpeed;
// animationSpeed scales clip/root-motion sampling but does not scale this flight policy.
// With delta=1/60, pitch advances by 0.4/60 radians and upward velocity decreases by
// 25/60 world-units/second per update; descent caps at 36/60 world-units.
// The clock producer/consumer chain is indexed at the top of this file. Higher
// pitch/height-rate coefficients increase change per second, not per rendered
// frame. Preserve the same seconds interval for look-at, movement and animation.
void CPetBody_UpdateMovement(void* self, float delta) {
    void* ball = GolfBall__Fv(self);
    PetVector boundsMaxWorld, boundsMinWorld;
    PetVector current = *(PetVector*)((char*)&AsCPetBody(self)->positionX);
    PetVector boneOriginWorld, lateralAxis;
    current.y = kFloat_0_petBody;
    PetVector routeDelta;
    __mi__FRC7WVectorRC7WVector(&routeDelta, ((char*)&AsCPetBody(self)->movementTarget), ((char*)&AsCPetBody(self)->movementStart));
    float routeDistance = Magnitude__7WVectorCFv(&routeDelta);
    PetVector remainingDelta;
    __mi__FRC7WVectorRC7WVector(&remainingDelta, ((char*)&AsCPetBody(self)->movementTarget), &current);
    float remainingDistance = Magnitude__7WVectorCFv(&remainingDelta);
    // Phase 1: height/pitch transitions use distance fractions of the original route.
    switch ((*(int*)&AsCPetBody(self)->movementState)) {
    case kPetTakeoff:
        if (AsCPetBody(self)->flightMovementEnabled) {
            // Takeoff: flatten negative pitch toward zero by 0.4 * delta radians.
            float turn;
            if ((*(unsigned int*)((char*)&AsCPetBody(self)->movementPitchRadians) & 0x80000000u)) turn = -AsCPetBody(self)->movementPitchRadians;
            else turn = AsCPetBody(self)->movementPitchRadians;
            if (!Pet_IsBelowEpsilon(turn)) {
                float next = kFloat_0_4_PetBody * delta + AsCPetBody(self)->movementPitchRadians;
                if (next < kFloat_0_petBody) AsCPetBody(self)->movementPitchRadians = next;
                else AsCPetBody(self)->movementPitchRadians = kFloat_0_petBody;
                PetVector flightDirection, forwardAxis, eulerRadians;
                float rotationZMatrix[12], rotationYMatrix[12], rotationXMatrix[12], rotationXYMatrix[12], rotationXYZMatrix[12];
                Pet_RebuildFlightRotation(self, flightDirection, forwardAxis, eulerRadians,
                    rotationZMatrix, rotationYMatrix, rotationXMatrix, rotationXYMatrix, rotationXYZMatrix);
            }
            if (((PetMovementBall*)ball)->position[1] > AsCPetBody(self)->minimumHeight) AsCPetBody(self)->minimumHeight = ((PetMovementBall*)ball)->position[1];
            // Follow the ball's rising height floor. Ascend using verticalVelocity,
            // reduce positive velocity by 25*delta, and lower overshoot above floor+10.
            if (AsCPetBody(self)->positionY < AsCPetBody(self)->minimumHeight) {
                float velocity = AsCPetBody(self)->verticalVelocity;
                float height = velocity * delta + AsCPetBody(self)->positionY;
                AsCPetBody(self)->positionY = height;
                if (velocity > kFloat_0_petBody) AsCPetBody(self)->verticalVelocity = velocity - kFloat_25_PetBody * delta;
            }
            if (AsCPetBody(self)->positionY > kFloat_10_PetBody + AsCPetBody(self)->minimumHeight) AsCPetBody(self)->positionY -= kFloat_10_PetBody * delta;
            if (kFloat_1_5_PetBody * remainingDistance <= routeDistance) {
                // remaining <= 2/3 of route: takeoff ends and base orientation resumes.
                AsCPetBody(self)->movementPitchRadians = kFloat_0_petBody; (*(int*)&AsCPetBody(self)->movementState) = kPetCruise;
                Matrix_CopyAffine(((char*)&AsCPetBody(self)->matrixBasis), ((char*)&AsCPetBody(self)->baseMatrixBasis));
            }
        } else {
            AsCPetBody(self)->movementPitchRadians = kFloat_0_petBody; (*(int*)&AsCPetBody(self)->movementState) = kPetCruise;
            Matrix_CopyAffine(((char*)&AsCPetBody(self)->matrixBasis), ((char*)&AsCPetBody(self)->baseMatrixBasis));
        }
        break;
    case kPetCruise:
        // remaining <= 1/3 of route begins landing. With flight disabled, each
        // update advances one phase without applying flight pitch/height policy.
        if (AsCPetBody(self)->flightMovementEnabled) {
            if (kFloat_3_PetBody * remainingDistance <= routeDistance) (*(int*)&AsCPetBody(self)->movementState) = kPetLanding;
        } else (*(int*)&AsCPetBody(self)->movementState) = kPetLanding;
        break;
    case kPetLanding: {
        // Derive clearance from a named bone's transformed bounds and the terrain.
        // A missing name/bone leaves drop=0; descent is capped by 36*delta.
        float drop = kFloat_0_petBody;
        int small = Pet_IsZero(*(unsigned int*)((char*)&AsCPetBody(self)->nameStringFlags) >> 31);
        unsigned int size = small ? (*(unsigned char*)((char*)&AsCPetBody(self)->nameStringFlags) & 0x7F) : (*(int*)&AsCPetBody(self)->nameStringLength);
        if (size) {
            void* node = WPuppet_FindNamedBoneEntry(*(void**)((char*)&AsCPetBody(self)->puppet), small ? ((char*)&AsCPetBody(self)->nameStringFlags + 1) : *(void**)((char*)&AsCPetBody(self)->nameStringData));
            if (node) {
                PetVector* bounds = ((PetVector*)((PetNamedBoneEntry*)node)->bounds);
                struct Pair { unsigned int words[2]; };
                *(Pair*)&boundsMinWorld = *(Pair*)bounds;
                ((int*)&boundsMinWorld)[2] = ((int*)bounds)[2];
                *(Pair*)&boundsMaxWorld = *(Pair*)(bounds + 1);
                ((int*)&boundsMaxWorld)[2] = ((int*)(bounds + 1))[2];
                PetVector t0; Matrix_TransformPoint(&t0, &boundsMinWorld, (char*)&((PetBoneMatrix*)*(void**)node)->matrix); __as__7WVectorFRC7WVector(&boundsMinWorld, &t0);
                PetVector t1; Matrix_TransformPoint(&t1, &boundsMaxWorld, (char*)&((PetBoneMatrix*)*(void**)node)->matrix); __as__7WVectorFRC7WVector(&boundsMaxWorld, &t1);
                float sumX;
                float half;
                float sumZ;
                sumX = boundsMinWorld.x + boundsMaxWorld.x;
                half = kFloat_0_5_PetBody;
                sumZ = boundsMinWorld.z + boundsMaxWorld.z;
                boundsMinWorld.x = sumX * half;
                boundsMinWorld.z = sumZ * half;
                Vec3_Zero(&boneOriginWorld);
                PetVector t2; Matrix_TransformPoint(&t2, &boneOriginWorld, (char*)&((PetBoneMatrix*)*(void**)node)->matrix); __as__7WVectorFRC7WVector(&boneOriginWorld, &t2);
                PetVector ground, position;
                ((int*)&position)[0] = ((int*)&boundsMinWorld)[0]; ((int*)&position)[1] = ((int*)&boundsMinWorld)[1]; ((int*)&position)[2] = ((int*)&boundsMinWorld)[2];
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&ground, CQuadTree_GetInstance(), &position, 1, 0, 1);
                float groundHeight = ground.y;
                drop = boneOriginWorld.y - groundHeight;
            }
        }
        if (AsCPetBody(self)->flightMovementEnabled) {
            // Compare against the stored pitch target, but clamp the decrement
            // independently to -pi/8 radians, at 0.3 radians/time unit.
            float turn = AsCPetBody(self)->movementPitchRadians - s_petLandingPitchTargetRadians;
            if (!Pet_IsMagnitudeBelowEpsilon(turn)) {
                float next = AsCPetBody(self)->movementPitchRadians - kFloat_0_3_PetBody * delta;
                if (next > kFloat_neg0_3926991_PetPitch) AsCPetBody(self)->movementPitchRadians = next;
                else AsCPetBody(self)->movementPitchRadians = kFloat_neg0_3926991_PetPitch;
                PetVector flightDirection, forwardAxis, eulerRadians;
                float rotationZMatrix[12], rotationYMatrix[12], rotationXMatrix[12], rotationXYMatrix[12], rotationXYZMatrix[12];
                Pet_RebuildFlightRotation(self, flightDirection, forwardAxis, eulerRadians,
                    rotationZMatrix, rotationYMatrix, rotationXMatrix, rotationXYMatrix, rotationXYZMatrix);
            }
            float step = kFloat_36_PetBody * delta;
            drop = step > drop ? drop : step;
        } else {
            AsCPetBody(self)->movementPitchRadians = kFloat_0_petBody;
            Matrix_CopyAffine(((char*)&AsCPetBody(self)->matrixBasis), ((char*)&AsCPetBody(self)->baseMatrixBasis));
            float step = kFloat_36_PetBody * delta;
            drop = step > drop ? drop : step;
        }
        AsCPetBody(self)->positionY -= drop;
        break;
    }
    }
    // Phase 2: steer along normalize(up x travel), only while the ball scalar's
    // magnitude exceeds epsilon. Apply projected lateral error * 1.2 * delta;
    // there is no forward correction or explicit overshoot clamp in this step.
    if (AsCPetBody(self)->flightMovementEnabled) {
        float speed;
        if ((*(unsigned int*)&((PetMovementBall*)ball)->movementScalar) & 0x80000000u) speed = -((PetMovementBall*)ball)->movementScalar;
        else speed = ((PetMovementBall*)ball)->movementScalar;
        if (!Pet_IsBelowEpsilon(speed)) {
            PetVector up = {kFloat_0_petBody, kFloat_1_petBody, kFloat_0_petBody};
            PetVector ballOffset, steeringStep, steeringVelocity, lateralError;
            Pet_ApplyLateralBallCorrection(self, ball, delta, lateralAxis, up,
                ballOffset, steeringStep, steeringVelocity, lateralError);
        }
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Set the tracking target and turn behavior when the puppet has a head bone.
void CPetBody_SetLookAtTarget(void* self, void* target, unsigned char turnBody) {
    void* other = AsCPetBody(self)->puppet;
    if (other != 0 && WPuppet_FindBoneByName(other, petBodyStrings + kPetBody_headBone)) {
        AsCPetBody(self)->followTarget = target;
        AsCPetBody(self)->turnToTarget = turnBody;
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Queue an animation name and its playback option.
char* CPetBody_SetNextAnimation(void* self, const char* source, unsigned char useRootMotion) {
    AsCPetBody(self)->queuedAnimationUsesRootMotion = useRootMotion;
    return strcpy(((char*)&AsCPetBody(self)->queuedAnimationName), source);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Apply the current animation time and compensate the puppet matrix for sampled root motion.
void CPetBody_ApplyAnimationToPuppet(void* self) {
    unsigned int flags = AsCPetBody(self)->flags;
    WPuppet_SetAnimationTimeFromRanges(AsCPetBody(self)->puppet, 0, AsCPetBody(self)->animationTime, (flags >> 1) & 1, 0);
    if ((AsCPetBody(self)->flags & kPetUsesRootMotion) == 0) {
        void* value = AsCPetBody(self)->puppet;
        Model_SetMatrixRecursive(value, value, ((char*)&AsCPetBody(self)->matrixBasis));
    } else {
        void* p = AsCPetBody(self)->currentAnimation;
        float result[3];
        ((void(*)(void*, void*, int, float, float))WPuppet_GetRootMotionDelta)(result, AsCPetBody(self)->puppet, ((int)((PetAnimationRange*)p)->name), kFloat_0_petBody, (float)(int)(kPetAnimationFramesPerSecond * (AsCPetBody(self)->animationTime - *(float*)p) + kFloat_0_5_PetBody));
        result[2] = result[2] * kFloat_neg1_PetBody;
        Arr3 old = AsCPetBody(self)->matrixPosition;
        WVector next;
        Matrix_TransformDirection(&next, result, ((char*)&AsCPetBody(self)->matrixBasis));
        *(WVector*)(((char*)&AsCPetBody(self)->matrixPosition)) -= next;
        void* value = AsCPetBody(self)->puppet;
        Model_SetMatrixRecursive(value, value, ((char*)&AsCPetBody(self)->matrixBasis));
        *(WVector*)(((char*)&AsCPetBody(self)->matrixPosition)) = *(WVector*)&old;
    }
    WPuppet_UpdateBoneWorldMatrices(AsCPetBody(self)->puppet, 0, 0, kFloat_1_petBody);
    WPuppet_UpdateBoundsFromBone(AsCPetBody(self)->puppet);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Return the loaded puppet pointer.
void* CPetBody_GetPuppet(void* self) {
    return AsCPetBody(self)->puppet;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Return the current animation range's name, or null when no animation is selected.
void* CPetBody_GetCurrentAnimationName(void* self) {
    void* value = AsCPetBody(self)->currentAnimation;
    if (value == 0) {
        return 0;
    }
    return ((PetAnimationRange*)value)->name;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Set the puppet's opacity recursively when its model is loaded.
void CPetBody_SetOpacity(void* self, unsigned int opacity) {
    void* value = AsCPetBody(self)->puppet;
    if (value != 0) {
        WPuppet_SetOpacityRecursive(value, opacity);
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Return the loaded puppet's minimum bounds vector.
void* CPetBody_GetPuppetBoundsMin(void* self) {
    return (char*)&AsWPuppet(AsCPetBody(self)->puppet)->boundsMin;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Return the mutable queued-animation name buffer.
void* CPetBody_GetNextAnimationName(void* self) {
    return ((char*)&AsCPetBody(self)->queuedAnimationName);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Return the animation-finished word through the original pointer-sized ABI.
void* CPetBody_GetAnimationFinished(void* self) {
    return *(void**)(((char*)&AsCPetBody(self)->animationFinished));
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Release the old puppet and request the replacement model asynchronously.
int CPetBody_RequestModelLoad(void* self, void* modelName) {
    void* value = AsCPetBody(self)->puppet;
    if (value != 0) {
        ResourceManager_ReleaseObject(g_resourceManager, value, 0);
        AsCPetBody(self)->puppet = 0;
    }
    ResourceManager_RequestModelLoad(g_resourceManager, modelName);
    AsCPetBody(self)->resourcePending = 1;
    return 1;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Poll an outstanding model read and publish the resulting puppet when complete.
int CPetBody_PollModelLoad(void* self) {
    if (AsCPetBody(self)->resourcePending != 1) {
        return 1;
    }
    if (!ResourceManager_PollResourceRead(g_resourceManager)) {
        return 0;
    }
    AsCPetBody(self)->puppet = ResourceManager_PollPuppetResourceLoad(g_resourceManager);
    AsCPetBody(self)->resourcePending = 0;
    return 1;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw on
extern "C" {
// Match registered substrings in borrowed, NUL-terminated *commandText; a null
// pointee is a no-op, but the cursor slot itself must be valid. Does not advance
// the cursor. Initial iterator setup and each NextInstance lock only traversal;
// strstr and Invoke run OUTSIDE those scopes. The owner must keep the pet, list
// nodes, match strings and callbacks alive through dispatch, and serialize removal
// or destruction with it. Recursive dispatch on this same pet replaces the shared
// iterator, so it is not reentrant. Callback code may take its own locks.
// Args is borrowed stack storage: index/tail=0, value=entry flags, matched=the
// substring address. Padding is uninitialized; callbacks must not retain &args.
void CPetBody_DispatchCommandCallbacks(void* self, void* commandText) {
    if (*(void**)commandText == 0) return;
    InstanceEntry* entry;
    {
        WInstanceLock outer(((char*)&AsCPetBody(self)->instanceMutex));
        void* head = (*(void**)&AsCPetBody(self)->instanceHead);
        (*(void**)&AsCPetBody(self)->instanceIterator) = head;
        entry = NextInstance((char*)self);
    }
    if (entry == 0) return;
    while (entry != 0) {
        int matched = strstr(*(void**)commandText, *(void**)entry);
        if (matched) {
            CallbackArgs args;
            args.index = 0;
            args.tail = 0;
            args.value = entry->flags;
            args.matched = matched;
            if (entry->callback != 0) entry->callback->Invoke(&args);
        }
        entry = NextInstance((char*)self);
    }
}
}
#pragma pop

#pragma push
#pragma exceptions on
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Register an owned attachment from quoted tokens, advancing the caller's cursor.
// Grammar: "animation" "name", or "@currentAnimation" "animation" "name".
// The @ gate compares against the CURRENT clip now; it is not a later filter.
// All consumed tokens must contain 1..63 bytes, plus NUL. String_ParseQuotedToken
// (club_trail_and_ring_effects.cpp) has NO capacity parameter: require <=63 bytes BEFORE parsing; the
// subsequent strlen check cannot prevent overflow. Leading spaces/tabs are skipped,
// quotes are required, escapes are not decoded, and an unterminated quote consumes
// through NUL. Require self, inout and *inout valid; failures leave the cursor at
// the last consumed token, with no list insertion. A failed @ gate consumes only
// its first token. Duplicate names are rejected after all required tokens parse.
// On success append a 0xC node owning a 0x94 payload {animation[64],name[64],
// resource,state[16]}; the pet destructor releases resource, payload and node.
// Allocation returning null is unchecked before writes, so this API assumes a
// successful/throwing allocator. No mutex is acquired: serialize attachment edits.
void CPetBody_ParseAndRegisterAttachment(void* self, void* inout) {
    char firstToken[0x40];
    char animationToken[0x40];
    char attachmentName[0x40];
    void* volatile foundCopy;
    void* volatile found;
    char* volatile pThird;
    char* volatile copy;
    void* volatile link;
    void* volatile linkCopy;
    volatile char flag;
    firstToken[0] = 0;
    *(char**)inout = String_ParseQuotedToken(*(char**)inout, firstToken);
    if (strlen(firstToken) == 0 || strlen(firstToken) >= 0x40) return;
    if (firstToken[0] == '@') {
        void* source = AsCPetBody(self)->currentAnimation;
        if (source == 0 || CString_Compare(firstToken + 1, ((const char*)((PetAnimationRange*)source)->name)) != 0) return;
        *(char**)inout = String_ParseQuotedToken(*(char**)inout, animationToken);
        if (strlen(animationToken) == 0 || strlen(animationToken) >= 0x40) return;
    } else {
        strcpy(animationToken, firstToken);
    }
    *(char**)inout = String_ParseQuotedToken(*(char**)inout, attachmentName);
    if (strlen(attachmentName) == 0 || strlen(attachmentName) >= 0x40) return;
    copy = attachmentName;
    pThird = attachmentName;
    char* item;
    void* list;
    void* node;
    void* sentinel;
    node = AsCPetBody(self)->attachmentHead;
    sentinel = ((char*)&AsCPetBody(self)->attachmentTail);
    while (node != sentinel && !PetNameMatches(((PetAttachmentLink*)node)->value->name, pThird)) {
        node = ((PetAttachmentLink*)node)->next;
    }
    found = node;
    foundCopy = node;
    if (node != ((char*)&AsCPetBody(self)->attachmentTail)) return;
    item = (char*)__nw__FUl(0x94);
    if (item != 0) {
        memset(item, 0, 0x40);
        memset(item + 0x40, 0, 0x40);
        memset(item + 0x84, 0, 0x10);
        *(unsigned int*)(item + 0x80) = 0;
    }
    strcpy(item, animationToken);
    strcpy(item + 0x40, attachmentName);
    flag = 0;
    list = ((char*)&AsCPetBody(self)->attachmentTail);
    char* newLink = (char*)appMallocDummy__FUl(0xC);
    link = newLink;
    linkCopy = newLink;
    PetLinkGuard guard(((char*)&AsCPetBody(self)->attachmentTail), newLink);
    void** slot = (void**)(newLink + 8);
    new(slot) PetLinkValue(item);
    void* tail = *(void**)list;
    guard.node = 0;
    ((PetAttachmentLink*)tail)->next = newLink;
    *(void**)newLink = *(void**)list;
    *(void**)list = newLink;
    *(void**)(newLink + 4) = list;
    ++AsCPetBody(self)->attachmentCount;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Enable the flight movement path.
void CPetBody_vfunc_16(void* self) {
    AsCPetBody(self)->flightMovementEnabled = 1;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Hide the pet and disable its named shadow projection.
void CPetBody_Hide(void* self) {
    AsCPetBody(self)->flags |= kPetHidden;
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager, petBodyStrings + kPetBody_caddieGroup, 0);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Show the pet and enable its named shadow projection.
void CPetBody_Show(void* self) {
    AsCPetBody(self)->flags &= ~((unsigned int)kPetHidden);
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager, petBodyStrings + kPetBody_caddieGroup, 1);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Forward a material-layer animation command to the puppet if loaded.
void CPetBody_ParseMaterialLayerAnimationCommand(void* self, void* commandText) {
    void* value = AsCPetBody(self)->puppet;
    if (value == 0) {
        return;
    }
    if (value == 0) {
        return;
    }
    WPuppet_ParseMaterialLayerAnimationCommand(value, commandText, value);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Parse effect attachment bone references against the current puppet and animation.
void CPetBody_ParseBoneReferences(void* self, void* commandText, void* effectContext) {
    FxAttachment_ParseBoneReferences(self, commandText, effectContext, AsCPetBody(self)->puppet, AsCPetBody(self)->currentAnimation, 0, 0);
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Parse a bounded sound token and play it through the pet sound handler.
void CPetBody_PlaySoundCommand(void* self, void* commandText) {
    char value[0x40];
    if (Parse_NonemptyQuotedTokenUnder64Bytes(self, commandText, value)) {
        CPetBody_PlaySound(self, value);
    }
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling 604
#pragma use_lmw_stmw reset
extern "C" {
// Initialize the pet basis matrix and this unit's full-turn constant.
void PetBody_InitBasisAndFullTurn(void) {
    float value = kFloat_6_2831855;
    Matrix_SetBasisElements(s_petBodyBasisMatrix,
        kFloat_0_petBody, kFloat_neg1_PetBody, kFloat_0_petBody, kFloat_1_petBody,
        kFloat_0_petBody, kFloat_0_petBody, kFloat_0_petBody, kFloat_0_petBody, (s_initializedTwoPi_801BBB28 = value, kFloat_1_petBody));
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Initialize course-model containers, intrusive lists, pending filename and fog defaults.
void* WPolySoup_Ctor(void* self, void* entryContainer) {
    void* classVtable = __vt__9WPolySoup;
    void* staticPetGroupVtable = __vt__Q29WPolySoup13CStaticPetGrp;
    BaseObject_ctor(self, entryContainer);

    void* list3 = ((char*)&AsWPolySoup(self)->generatedHead);
    void* list1 = ((char*)&AsWPolySoup(self)->auxiliaryNodeHead);
    void* list2 = ((char*)&AsWPolySoup(self)->ambientSoundHead);
    void* end = ((char*)&AsWPolySoup(self)->effectTreeRoot);
    int zero = 0;
    int savedZero = zero;
    *(void**)self = classVtable;
    AsWPolySoup(self)->unk_10 = savedZero;
    int savedZero2 = zero;
    AsWPolySoup(self)->regions = savedZero2;
    int savedZero3 = zero;
    AsWPolySoup(self)->regionCount = zero;
    AsWPolySoup(self)->regionCapacity = zero;
    AsWPolySoup(self)->resourceEntries = zero;
    AsWPolySoup(self)->resourceCount = savedZero3;
    AsWPolySoup(self)->resources = zero;
    int savedZero4 = zero;
    AsWPolySoup(self)->cameras = zero;
    AsWPolySoup(self)->bounds = savedZero4;
    int savedZero5 = zero;
    AsWPolySoup(self)->namedPoints = savedZero5;
    AsWPolySoup(self)->surfaces = zero;
    AsWPolySoup(self)->fallbackSurface = zero;
    AsWPolySoup(self)->effectTreeCount = zero;
    AsWPolySoup(self)->effectTreeRoot = zero;
    AsWPolySoup(self)->effectTreeFirst = end;
    AsWPolySoup(self)->unk_C4 = 0;
    AsWPolySoup(self)->resourceCapabilityLatched = 0;
    AsWPolySoup(self)->staticPetGroupVtable = staticPetGroupVtable;
    AsWPolySoup(self)->staticPetCount = 0;
    AsWPolySoup(self)->staticPetEntries = 0;
    // Empty auxiliary list followed by two pointer vectors; payload kinds remain unresolved.
    AsWPolySoup(self)->auxiliaryNodeCount = 0;
    AsWPolySoup(self)->auxiliaryNodeTail = list1;
    AsWPolySoup(self)->auxiliaryNodeHead = list1;
    AsWPolySoup(self)->auxiliaryVector1Data = 0;
    AsWPolySoup(self)->auxiliaryVector1Count = 0;
    AsWPolySoup(self)->auxiliaryVector1Capacity = 0;
    AsWPolySoup(self)->auxiliaryVector2Data = 0;
    AsWPolySoup(self)->auxiliaryVector2Count = 0;
    AsWPolySoup(self)->auxiliaryVector2Capacity = 0;
    AsWPolySoup(self)->ambientSoundCount = 0;
    AsWPolySoup(self)->ambientSoundTail = list2;
    AsWPolySoup(self)->ambientSoundHead = list2;
    AsWPolySoup(self)->generatedCount = 0;
    AsWPolySoup(self)->generatedTail = list3;
    AsWPolySoup(self)->generatedHead = list3;
    AsWPolySoup(self)->resourceIdCount = 0;
    AsWPolySoup(self)->resourceIds = 0;
    AsWPolySoup(self)->resourceIdCapacity = 0;
    AsWPolySoup(self)->surfaceBinaryLoadFailed = 0;
    AsWPolySoup(self)->resourceObjects = 0;
    AsWPolySoup(self)->resourceObjectCount = 0;
    AsWPolySoup(self)->resourceObjectCapacity = 0;
    AsWPolySoup(self)->pendingFilenameFlags = 0;
    AsWPolySoup(self)->pendingFilenameLength = 0;
    AsWPolySoup(self)->pendingFilenameData = 0;
    AsWPolySoup(self)->entryContainer = entryContainer;
    AsWPolySoup(self)->unk_20 = 0;
    memset((char*)self + kCourseModelInlineName, 0, kCourseModelInlineNameBytes);
    float f2 = kFloat_0_PetBody;
    float f1 = kFloat_800_PolySoup;
    float f0 = kFloat_960_PolySoup;
    AsWPolySoup(self)->surfaceCount = zero;
    AsWPolySoup(self)->unk_491C = f2;
    AsWPolySoup(self)->unk_B8 = f1;
    float savedF = f2;
    AsWPolySoup(self)->defaultFogStart = savedF;
    AsWPolySoup(self)->unk_BC = f0;
    AsWPolySoup(self)->defaultFogEnd = f2;
    AsWPolySoup(self)->alternateFogStart = f2;
    float savedF2 = f2;
    AsWPolySoup(self)->alternateFogEnd = savedF2;
    return self;
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Return the raw animation-time word through the original pointer-sized ABI.
void* Object_GetWord10(void* self) {
    return *(void**)(((char*)&AsCPetBody(self)->animationTime));
}
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
extern "C" {
// Clear and release a temporary vector, then optionally delete its wrapper.
void* dtor_801BBCE8(void* self, int arg1) {
    return ::std::__vector_deleting_dtor((Elem_801BBCE8Vector*)self, arg1);
}
}
#pragma pop

}
