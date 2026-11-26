// Pet body: animation, flight movement, look-at, lighting and script commands.
//
// A body owns a puppet and keeps the puppet's world transform in step with an
// animation, an optional sampled root motion and a small flight controller.
// Script commands can queue animations, play sounds and register attachments.
#include <decomp/static_init.h>
#include "MSL/vector.h"

// Elements of the temporary vector released by dtor_801BBCE8. The tag is not
// otherwise used; pointer elements have no per-element destruction.
struct Elem_801BBCE8;
namespace std {
template <> struct __vec_destroys_elements<Elem_801BBCE8*> {
    enum {
        value = 3
    };
    static void destroy(Elem_801BBCE8**) {}
};
}
typedef std::vector<Elem_801BBCE8*, app::app_allocator<Elem_801BBCE8*> > Elem_801BBCE8Vector;

#include "classes/CGolfDoc.h"
#include "classes/CPetBody.h"
#include "classes/CSharedDoc.h"
#include "classes/WPolySoup.h"
#include "classes/WPuppet.h"
#include "game/types.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CQuadTree.h"

// Fields of other classes that this file reads. They belong in those classes'
// headers once those exist.
struct PetBoneControllerRecord {
    char prefix[0x68];
    void* controllerKey; // 0x68
};
struct PetRenderParameter {
    unsigned int kind;             // 0x00
    float transformedDirection[3]; // 0x04
    char middle[0xC];              // 0x10
    float sourceDirection[3];      // 0x1C
};
typedef char PetRenderParameterSize[(sizeof(PetRenderParameter) == 0x28) ? 1 : -1];
struct PetCameraMatrices {
    char prefix[0x20];
    float view[16];       // 0x20
    float projection[16]; // 0x60
};
struct PetTypeInfoLink {
    void* type;   // 0x00
    void* parent; // 0x04
};
struct PetAttachmentLink {
    PetAttachmentLink* previous; // 0x00
    PetAttachmentLink* next;     // 0x04
    PetAttachment* value;        // 0x08
};
struct PetInstanceNode {
    void* value;      // 0x00
    char prefix[0xC]; // 0x04
    void* next;       // 0x10
};
struct PetNamedBoneEntry {
    void* bone;   // 0x00
    void* bounds; // 0x04
};
struct PetBoneMatrix {
    char prefix[0x78];
    float matrix[12]; // 0x78
};
struct PetLookAtBone {
    char reserved00[0x78];
    float worldBasis[9];  // 0x78
    float worldOrigin[3]; // 0x9C
};
struct PetMovementBall {
    float position[3];    // 0x00
    char prefix[0x28];    // 0x0C
    float movementScalar; // 0x34
};
struct PetRenderDefaults {
    unsigned int renderStates[26];  // 0x00
    unsigned int stageStates[12];   // 0x68
    unsigned int samplerStates[10]; // 0x98
};
struct Blob26 {
    unsigned int w[26];
};
struct Blob12 {
    unsigned int w[12];
};
struct Blob10 {
    unsigned int w[10];
};

// A node of the callback ring. Payloads are deleted separately; the nodes
// themselves are recycled through the free ring.
struct PetCallbackRingNode {
    void* data;                // 0x00
    unsigned int pad[2];       // 0x04
    PetCallbackRingNode* prev; // 0x0C
    PetCallbackRingNode* next; // 0x10
};
// An attachment-list node owns only its payload pointer.
struct AttachmentListNode {
    AttachmentListNode* prev; // 0x00
    AttachmentListNode* next; // 0x04
    PetAttachment* data;      // 0x08
};

enum PetBodyFlags {
    kPetRootMotionSampled = 1,
    kPetUsesRootMotion = 2,
    kPetAnimationPlaying = 4,
    kPetMatrixDirty = 0x10,
    kPetHeadAnglesDirty = 0x20,
    kPetLookAtDirty = 0x30,
    kPetHidden = 0x2000
};
// The original tests the dirty bit together with another state bit.
enum PetDrawMasks {
    kPetNeedsAnimationOrMatrix = 0x14,
    kPetNeedsHeadCheck = 0x34
};
enum PetEulerAxis {
    kPetAxisX = 0,
    kPetAxisY = 1,
    kPetAxisZ = 2
};
enum PetMovementPhase {
    kPetTakeoff = 0,
    kPetCruise = 1,
    kPetLanding = 2
};
enum {
    kCourseModelInlineName = 0x60,
    kCourseModelInlineNameBytes = 0x28
};

// Byte positions in the shared string pool. The text is in the original data.
enum {
    kPetBody_headBone = 0x14,              // "Head"
    kPetBody_chanceMarker = 0x64,          // "チャンス"
    kPetBody_scoreTextSuffix = 0x6D,       // "が文"
    kPetBody_birdieText = 0x72,            // "バーディー"
    kPetBody_facialAnimationMarker = 0x7D, // "MT_FACIAL_ANIMATION"
    kPetBody_caddieGroup = 0x91            // "caddie"
};

extern "C" {

void __as__7WVectorFRC7WVector(void* destination, const void* source);
void __mi__FRC7WVectorRC7WVector(void* result, const void* left, const void* right);
void __pl__FRC7WVectorRC7WVector(void* result, const void* left, const void* right);
void __apl__7WVectorFRC7WVector(void* self, const void* value);
void __ml__FfRC7WVector(void* result, const void* vector, float scale);
void __ml__FRC7WVectorf(void* result, const void* vector, float scale);
float __ml__FRC7WVectorRC7WVector(const void* left, const void* right);
void Normalize__7WVectorFv(void* self);
float Magnitude__7WVectorCFv(const void* self);
void Matrix_CopyAffine(void* destination, const void* source);
void Matrix_ConcatAffine(void* result, const void* left, const void* right);
void Matrix_ExtractUniformScaleNormalizedRotation(void* result, const void* matrix);
void Matrix_SetAxisRotation(void* result, float angle, int axis);
void Matrix_SetIdentityAffine(void* matrix);
void Matrix_SetBasisElements(void* matrix, float a0, float a1, float a2, float a3, float a4, float a5, float a6,
                             float a7, float a8);
void Matrix_SetBasisScale(void* matrix, const void* scale);
void Matrix_TransformDirection(void* result, const void* direction, const void* matrix);
void* Matrix_TransformPoint(void* result, const void* point, const void* matrix);
void Matrix_AppendEulerRotation(void* matrix, const void* angles);
void Vec2_Zero(void* vector);
void Vec3_Zero(void* vector);
struct FloatPair {
    float first;
    float second;
};
FloatPair Vec2_Subtract(void* left, void* right);
float Vec2_CalculateLength(void* vector);
void Vector_CrossProduct(void* result, const void* left, const void* right);
float Vector_SignedAngleXZ(const void* left, const void* right);
float Math_SquareRootApprox(float value);
void String_ReplaceRange(void* string, int start, unsigned int length, const char* source, const char* sourceEnd,
                         void* result);
char* String_ParseQuotedToken(const char* source, char* output);
int Parse_NonemptyQuotedTokenUnder64Bytes(void* self, void* commandText, char* output);
int CString_Compare(const char* left, const char* right);
void* memset(void* destination, int value, unsigned long size);
void* __nw__FUl(unsigned int size);
void* appMallocDummy__FUl(unsigned int size);
void appFreeUni__FPv(void* value);
void __dl__FPv(void* value);

int WPuppet_FindBoneByName(void* self, const char* name);
void* WPuppet_HasBoneControllerEntry(void* self, void* key);
void WPuppet_SetBoneControllerVector(void* self, void* key, void* value);
void WPuppet_EraseBoneControllerEntry(void* self, void* key);
void WPuppet_SetAnimationTimeFromRanges(void* self, int animation, float time, int useRootMotion, int flags);
void WPuppet_GetRootMotionDelta(void* result, float startFrame, void* self, float endFrame, void* rangeName);
void WPuppet_GetInverseRootMotionOffsetForRange(void* result, void* self, float frame, int rangeName);
void* WPuppet_FindAnimationRangeByName(void* self, void* name);
void WPuppet_UpdateBoneWorldMatrices(void* self, int arg1, int arg2, float scale);
void WPuppet_UpdateBoundsFromBone(void* self);
void WPuppet_UpdateBound(void* self, int arg1);
void WPuppet_SetOpacityRecursive(void* self, unsigned int opacity);
void WPuppet_ParseMaterialLayerAnimationCommand(void* self, void* commandText, void* value);
void* WPuppet_FindNamedBoneEntry(void* self, void* name);
void* WPuppet_GetSkinMatrices(void* self);

void Model_SetMatrixRecursive(void* self, void* value, void* matrix);
void AnimationEventParser_ProcessFrameRange(void* self, void* puppet, float startTime, float endTime, void* range,
                                            int arg5, int arg6);
void FxAttachment_ParseBoneReferences(void* self, void* commandText, void* effectContext, void* puppet,
                                      void* currentAnimation, int arg5, int arg6);
void ShadowModel_SetNamedProjectionEnabled(void* shadowManager, const char* groupName, int enabled);
void Sound_PlayByName(void* soundManager, const char* name, void* position, void* arg3, void* arg4, void* options,
                      float volume, float distance, float pitch);
void* Object_GetChild(void* taskManager);
int ResourceManager_PollResourceRead(void* resourceManager);
void* ResourceManager_PollPuppetResourceLoad(void* resourceManager);
void ResourceManager_RequestModelLoad(void* resourceManager, void* modelName);
void ResourceManager_ReleaseObject(void* resourceManager, void* object, int arg2);
void* GolfPlayer_GetEntryByIndex(void* golfDoc, unsigned char player);
void* GolfBall__Fv(void);
void* CQuadTree_GetInstance(float y, float z);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void* result, void* quadTree, void* position, int arg3, int arg4,
                                             int arg5);
void* Color_ScaleRGB(void* color, float scale);
void Engine_SetRenderStateList(void* states, int count);
void Engine_SetTextureStageStateList(int stage, void* states, int count);
void Engine_SetTextureStageParameterList(int stage, void* states, int count);
void Camera_Apply(void* view, float x, float y);
void* Camera_GetRenderState(void* view);
void gtl_GenericRenderer_SetProjectionMatrix(void* matrix);
void gtl_GenericRenderer_SetViewMatrix(void* matrix);
void SetTransform(int type, void* matrix);
int WView_DefaultCapabilityTrueA(void* view, void* object);
void* WView_DefaultAcceptVisibility(void* view, void* object);
int BaseObject_ctor(void* self, void* arg1);

extern void* g_resourceManager;
extern void* g_taskManager;
extern void* g_petBodyTaskChild;
extern void* s_golfTaskTypeInfo;
extern void* g_sharedDoc;
extern void* g_soundManager;
extern void* g_shadowManager;
extern void* g_view;
extern unsigned int s_sceneLighting[];
extern const char s_cameraTransform[];
extern const char petBodyStrings[];
extern const unsigned int kPetPositionalSoundOptionWord0;
extern const unsigned int kFloat_1000_PetSound;
extern const unsigned int s_petBodyDrawStateDefaults[];
extern char __vt__8CPetBody[];
extern char __vt__9WPolySoup[];
extern char __vt__Q29WPolySoup13CStaticPetGrp[];
extern float s_petLookAtPitchReference;
extern float s_petLookAtYawReference;
extern float s_petLandingPitchTargetRadians;
extern float s_initializedTwoPi_801BBB28;
extern char s_petBodyBasisMatrix[];
extern const float kFloat_0_petBody;
extern const float kFloat_1_petBody;
extern const float kFloat_neg1_PetBody;
extern const float kFloat_0_5_PetBody;
extern const float kFloat_50_PetBody;
extern const float kFloat_200_PetBody;
extern const float kFloat_neg0_7853982_PetBody;
extern const float kFloat_neg0_34906584_PetBody;
extern const float kFloat_0_7853982_PetBody;
extern const float kFloat_neg0_7853982_PetBody;
extern const float kFloat_0_017453292_PetBody;
extern const float kFloat_1_5707964_PetMotion;
extern const float kFloat_0_00001_PetAnimation;
extern const float kPetAnimationFramesPerSecond;
extern float kFloat_0_4_PetBody;
extern float kFloat_25_PetBody;
extern float kFloat_10_PetBody;
extern float kFloat_1_5_PetBody;
extern float kFloat_3_PetBody;
extern float kFloat_0_3_PetBody;
extern float kFloat_neg0_3926991_PetPitch;
extern float kFloat_36_PetBody;
extern float kFloat_1_2_PetBody;
extern float kFloat_6_2831855;
extern float kFloat_0_PetBody;
extern float kFloat_800_PolySoup;
extern float kFloat_960_PolySoup;

} // extern "C"

#define kZero kFloat_0_petBody
#define kOne kFloat_1_petBody
#define kHalf kFloat_0_5_PetBody
#define kTwoPi kFloat_6_2831855
#define kEpsilon kFloat_0_00001_PetAnimation
#define kFramesPerSecond kPetAnimationFramesPerSecond
#define kOneDegree kFloat_0_017453292_PetBody

// A borrowed lock for the callback ring and the instance mutex.
struct MutexLock {
    void* mutex;
    MutexLock(void* value) : mutex(value) {
        OSLockMutex((OSMutex*)value);
    }
    ~MutexLock() {
        OSUnlockMutex((OSMutex*)mutex);
    }
};

// Return the next callback payload and advance the iterator, or null at the end.
static inline void* NextCallbackPayload(CPetBody* self) {
    void* mutex = self->instanceMutex;
    OSLockMutex((OSMutex*)mutex);
    PetCallbackRingNode* node = self->instanceIterator;
    if (node) {
        PetCallbackRingNode* next = node->next;
        void* value = node->data;
        if (next == self->instanceHead)
            next = 0;
        self->instanceIterator = next;
        OSUnlockMutex((OSMutex*)mutex);
        return value;
    }
    OSUnlockMutex((OSMutex*)mutex);
    return 0;
}

// Unlink and free the attachment-list nodes; the payloads are released by the caller.
static inline void ClearAttachmentList(CPetBody* self) {
    AttachmentListNode* it = ((AttachmentListNode*)&self->attachmentTail)->next;
    AttachmentListNode* sentinel = (AttachmentListNode*)&self->attachmentTail;
    if (it != sentinel) {
        AttachmentListNode* lastAttachmentNode = sentinel->prev;
        it->prev->next = lastAttachmentNode->next;
        lastAttachmentNode->next->prev = it->prev;
        while (it != sentinel) {
            void* cur = it;
            it = it->next;
            appFreeUni__FPv(cur);
            --self->attachmentCount;
        }
    }
}

// Clear the render-parameter count. The original reads the field once and
// subtracts that value from itself, which the compiler keeps as a subtraction
// instead of folding it to zero; the result is always zero.
static inline void ClearRenderParameterCount(CPetBody* self) {
    self->renderParameterCount = self->renderParameterCount - *(unsigned int*)&self->renderParameterCount;
}

// Ordered float comparison used by the head-angle change test.
static inline int lt(float a, float b) {
    return a < b;
}

// Clear the sign bit of a float, including for negative zero.
static inline float fabsd(float f) {
    union {
        float f;
        int i;
    } u;
    u.f = f;
    if (u.i & 0x80000000)
        f = -f;
    return f;
}

// The renderer interface of a puppet, reduced to the virtuals used here.
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

// The first word of an object that has a type-info chain.
struct CVirtObj {
    virtual void* vfunc_00();
    virtual void* vfunc_04();
    virtual void* vfunc_08();
};

// Return whether the object's runtime type ancestry includes the golf-task descriptor.
static inline int FindInChain(void* obj) {
    if (obj) {
        void* q = ((CVirtObj*)obj)->vfunc_00();
        while (q) {
            if (q == &s_golfTaskTypeInfo)
                return 1;
            q = ((PetTypeInfoLink*)q)->parent;
        }
        return 0;
    }
    return 0;
}

typedef Vec3f PetVector;

// Compare the signed value against the movement tolerance; negative values pass.
static inline int Pet_IsBelowEpsilon(float value) {
    return value < kEpsilon;
}
// Compare the absolute magnitude against the movement tolerance.
static inline int Pet_IsMagnitudeBelowEpsilon(float value) {
    double magnitude;
    if (*(unsigned int*)&value & 0x80000000u)
        magnitude = -value;
    else
        magnitude = value;
    return magnitude < kEpsilon;
}
static inline int Pet_IsZero(unsigned int value) {
    return !value;
}
// Store the components in the order the original writes them.
static inline void Pet_SetVector(PetVector* v, float x, float y, float z) {
    v->x = x;
    v->z = z;
    v->y = y;
}

// Absolute value with the original sign-bit test and double result.
static inline double AnimationAbs(float value) {
    if (*(unsigned int*)&value & 0x80000000u)
        return -value;
    return value;
}

// Clamp the head pitch in radians to [-20 degrees, +45 degrees].
static inline float Pet_ClampLookAtPitch(float value) {
    return value < kFloat_neg0_34906584_PetBody ? kFloat_neg0_34906584_PetBody
                                                : (value > kFloat_0_7853982_PetBody ? kFloat_0_7853982_PetBody : value);
}

// Clamp the head yaw in radians to [-45 degrees, +45 degrees].
static inline float Pet_ClampLookAtYaw(float value) {
    return value < kFloat_neg0_7853982_PetBody ? kFloat_neg0_7853982_PetBody
                                               : (value > kFloat_0_7853982_PetBody ? kFloat_0_7853982_PetBody : value);
}

// Rebuild the flight orientation from the base yaw and movement pitch, keeping
// the world translation. Scratch stays in the caller to retain the stack layout.
static inline void Pet_RebuildFlightRotation(CPetBody* self, PetVector& direction, PetVector& axis, PetVector& rotation,
                                             float* rotationZMatrix, float* rotationYMatrix, float* rotationXMatrix,
                                             float* rotationXYMatrix, float* rotationXYZMatrix) {
    Pet_SetVector(&axis, kFloat_0_petBody, kFloat_0_petBody, kFloat_neg1_PetBody);
    __ml__FfRC7WVector(&direction, &self->baseMatrixBasis[6], kFloat_neg1_PetBody);
    float angle = Vector_SignedAngleXZ(&direction, &axis);
    rotation.x = -self->movementPitchRadians;
    rotation.y = angle;
    rotation.z = kFloat_0_petBody;
    __as__7WVectorFRC7WVector(&self->rotationX, &rotation);
    Matrix_SetAxisRotation(rotationZMatrix, self->rotationZ, 2);
    Matrix_SetAxisRotation(rotationYMatrix, self->rotationY, 1);
    Matrix_SetAxisRotation(rotationXMatrix, self->rotationX, 0);
    Matrix_ConcatAffine(rotationXYMatrix, rotationXMatrix, rotationYMatrix);
    Matrix_ConcatAffine(rotationXYZMatrix, rotationXYMatrix, rotationZMatrix);
    Matrix_CopyAffine(&self->matrixBasis, rotationXYZMatrix);
    __as__7WVectorFRC7WVector(&self->matrixPosition, &self->positionX);
    self->flags |= kPetMatrixDirty;
}

// Resolve the head-relative pitch and the body-relative yaw toward one target.
static inline void Pet_ResolveLookAtAngles(CPetBody* self, float* headToTargetWorld, float* bodyToTargetWorld,
                                           float& yaw) {
    __mi__FRC7WVectorRC7WVector(
        headToTargetWorld, self->followTarget,
        (char*)((PetLookAtBone*)(unsigned int)WPuppet_FindBoneByName(self->puppet, petBodyStrings + kPetBody_headBone))
            ->worldOrigin);
    self->targetPitch = Pet_ClampLookAtPitch((float)atan(
        (double)(headToTargetWorld[1] / Math_SquareRootApprox(headToTargetWorld[0] * headToTargetWorld[0] +
                                                              headToTargetWorld[2] * headToTargetWorld[2]))));
    __mi__FRC7WVectorRC7WVector(bodyToTargetWorld, self->followTarget, &self->baseMatrixPositionX);
    yaw = Vector_SignedAngleXZ(bodyToTargetWorld, &self->baseMatrixBasis[6]);
    self->targetYaw = yaw;
}

// Rebuild the world body frame from the stored Euler angles and publish both
// copies. The concat order is (Rx * Ry) * Rz; the translation comes from the
// pet's world position.
static inline void Pet_RebuildBodyFrame(CPetBody* self, float* rotationZMatrix, float* rotationYMatrix,
                                        float* rotationXMatrix, float* rotationXYMatrix, float* rotationXYZMatrix) {
    Matrix_SetAxisRotation(rotationZMatrix, self->rotationZ, kPetAxisZ);
    Matrix_SetAxisRotation(rotationYMatrix, self->rotationY, kPetAxisY);
    Matrix_SetAxisRotation(rotationXMatrix, self->rotationX, kPetAxisX);
    Matrix_ConcatAffine(rotationXYMatrix, rotationXMatrix, rotationYMatrix);
    Matrix_ConcatAffine(rotationXYZMatrix, rotationXYMatrix, rotationZMatrix);
    Matrix_CopyAffine(&self->baseMatrixBasis, rotationXYZMatrix);
    __as__7WVectorFRC7WVector(&self->baseMatrixPositionX, &self->positionX);
    Matrix_CopyAffine(&self->matrixBasis, &self->baseMatrixBasis);
    self->flags |= kPetMatrixDirty;
}

// Project the ball's world-space error onto the travel's lateral axis, then add
// the delta-scaled steering step. The scratch vectors stay in the caller.
static inline void Pet_ApplyLateralBallCorrection(CPetBody* self, void* ball, float delta, PetVector& lateralAxis,
                                                  PetVector& up, PetVector& ballOffset, PetVector& steeringStep,
                                                  PetVector& steeringVelocity, PetVector& lateralError) {
    Vector_CrossProduct(&lateralAxis, &up, &self->movementDirection);
    Normalize__7WVectorFv(&lateralAxis);
    __mi__FRC7WVectorRC7WVector(&ballOffset, ball, &self->positionX);
    float projection = __ml__FRC7WVectorRC7WVector(&ballOffset, &lateralAxis);
    __ml__FRC7WVectorf(&lateralError, &lateralAxis, projection);
    __ml__FRC7WVectorf(&steeringVelocity, &lateralError, kFloat_1_2_PetBody);
    __ml__FRC7WVectorf(&steeringStep, &steeringVelocity, delta);
    __apl__7WVectorFRC7WVector(&self->positionX, &steeringStep);
}

// A registered command callback and the substring that selects it.
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
    const char* matchText;      // 0x00  substring sought in the command text
    InstanceCallback* callback; // 0x04
    unsigned int flags;         // 0x08
};

struct CallbackArgs {
    unsigned int index; // 0x00
    unsigned int value; // 0x04
    int matched;        // 0x08
    unsigned int pad[4];
    unsigned int tail;
};

// Return the current callback entry and advance the circular iterator under the
// mutex; the lock is released before the entry reaches the caller.
static inline InstanceEntry* NextInstance(CPetBody* self) {
    MutexLock lock(self->instanceMutex);
    void* node = *(void**)&self->instanceIterator;
    if (node != 0) {
        void* next = ((PetInstanceNode*)node)->next;
        InstanceEntry* entry = *(InstanceEntry**)node;
        if (next == *(void**)&self->instanceHead)
            next = 0;
        *(void**)&self->instanceIterator = next;
        return entry;
    }
    return 0;
}

// Guard an uncommitted list node and free it unless insertion takes ownership.
struct PetLinkGuard {
    void* list;
    void* node;
    PetLinkGuard(void* l, void* n) {
        void* saved = l;
        list = saved;
        node = n;
    }
    ~PetLinkGuard() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// A payload slot constructed in place by the list insertion.
struct PetLinkValue {
    void* value;
    static void* operator new(size_t, void* p) {
        return p;
    }
    PetLinkValue(void* p) {
        try {
            value = p;
        } catch (...) {
            throw;
        }
    }
};

// Compare two borrowed attachment names.
static inline bool PetNameMatches(const char* a, const char* b) {
    return CString_Compare(a, b) == 0;
};

extern "C" {
void PetBody_InitBasisAndFullTurn();
}
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462188", _ctors_u801B9214_0, PetBody_InitBasisAndFullTurn);
#pragma pop

// Release the instance callbacks and attachments, the puppet, the render
// parameters and the name storage.
#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw on
CPetBody::~CPetBody() {
    vtbl = __vt__8CPetBody;

    // Delete the queued command payloads, then move the live nodes into the
    // recycled ring and clear the bucket table. Pooled nodes are kept.
    void* callbackPayload;
    {
        MutexLock lock(instanceMutex);
        instanceIterator = instanceHead;
        callbackPayload = NextCallbackPayload(this);
    }
    while (callbackPayload) {
        // The original tests the payload again before releasing it.
        if (callbackPayload)
            __dl__FPv(callbackPayload);
        callbackPayload = NextCallbackPayload(this);
    }
    {
        MutexLock lock(instanceMutex);
        if (instanceHead && instanceFreeNodeHead) {
            instanceHead->prev->next = instanceFreeNodeHead->next;
            instanceFreeNodeHead->next->prev = instanceHead->prev;
            instanceFreeNodeHead->next = instanceHead;
            instanceHead->prev = instanceFreeNodeHead;
            instanceHead = 0;
        } else if (instanceHead) {
            void* mutex = instanceMutex;
            PetCallbackRingNode* node;
            while ((node = instanceHead) != 0) {
                OSLockMutex((OSMutex*)mutex);
                node->next->prev = node->prev;
                node->prev->next = node->next;
                PetCallbackRingNode* next = node->next;
                if (next == node)
                    next = 0;
                OSUnlockMutex((OSMutex*)mutex);
                PetCallbackRingNode* value = instanceHead;
                PetCallbackRingNode* old = instanceFreeNodeHead;
                OSLockMutex((OSMutex*)mutex);
                if (old == 0) {
                    value->prev = value;
                    value->next = value;
                    OSUnlockMutex((OSMutex*)mutex);
                } else {
                    value->next = old;
                    value->prev = old->prev;
                    old->prev = value;
                    value->prev->next = value;
                    OSUnlockMutex((OSMutex*)mutex);
                    value = old;
                }
                instanceFreeNodeHead = value;
                instanceHead = next;
            }
        }
        for (int i = 0; instanceHashBucketCount > i; ++i)
            instanceHashBuckets[i] = 0;
    }
    // Delete the attachment payloads and their resources before the nodes that
    // own them.
    {
        AttachmentListNode* sentinel = (AttachmentListNode*)&attachmentTail;
        AttachmentListNode* node = sentinel->next;
        while (node != sentinel) {
            PetAttachment* attachment = node->data;
            if (attachment) {
                if (attachment->resource) {
                    __dl__FPv(attachment->resource);
                    attachment->resource = 0;
                }
                __dl__FPv(attachment);
            }
            node = node->next;
        }
    }
    ClearAttachmentList(this);

    // Release the puppet, the heap string and the render-parameter array. The
    // original's inlined container destructors test the address of the member
    // they clear, so the tests around them are always true.
    void* resource = puppet;
    if (resource) {
        ResourceManager_ReleaseObject(g_resourceManager, resource, 0);
        puppet = 0;
    }
    ClearRenderParameterCount(this);
    if (&attachmentCount) {
        if (&attachmentCount) {
            if (&attachmentCount) {
                if (&attachmentCount) {
                    ClearAttachmentList(this);
                }
            }
        }
    }
    if (&nameStringFlags)
        if (*(unsigned int*)&nameStringFlags >> 31)
            appFreeUni__FPv((void*)nameStringData);
    if (&renderParameters) {
        if (&renderParameters) {
            if (&renderParameters) {
                if (&renderParameters) {
                    if (renderParameters) {
                        ClearRenderParameterCount(this);
                        appFreeUni__FPv(renderParameters);
                    }
                }
            }
        }
    }
    // Free the pooled allocation nodes and the bucket storage. The nested mutex
    // acquisitions are recursive.
    if (&instanceHead) {
        MutexLock lock(instanceMutex);
        if (instanceNodeAllocationHead) {
            PetCallbackRingNode* node;
            void* mutex = instanceMutex;
            while ((node = instanceNodeAllocationHead) != 0) {
                OSLockMutex((OSMutex*)mutex);
                node->next->prev = node->prev;
                node->prev->next = node->next;
                PetCallbackRingNode* next = node->next;
                if (node == next)
                    next = 0;
                OSUnlockMutex((OSMutex*)mutex);
                instanceNodeAllocationHead = next;
                appFreeUni__FPv(node);
            }
        }
        if (instanceHashBuckets) {
            appFreeUni__FPv(instanceHashBuckets);
            instanceHashBuckets = 0;
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
// Clear the playback flags, the queued animation and the look-at state; restore
// the animation and root-motion speed scales.
void CPetBody::ResetPlaybackAndLookAt() {
    flags = 0;
    followTarget = 0;
    Vec2_Zero(&targetPitch);
    Vec2_Zero(&lookAtPitch);
    float one = kOne;
    queuedAnimationName[0] = 0;
    rootMotionScale = one;
    animationSpeed = one;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Replace the stored bone name used when positioning the pet against the ground.
void CPetBody::SetNameString(const char* text) {
    unsigned int replacedLength;
    if ((*(unsigned int*)&nameStringFlags >> 31) == 0)
        replacedLength = *(unsigned char*)&nameStringFlags & 0x7F;
    else
        replacedLength = *(unsigned int*)&nameStringLength;
    volatile int value;
    unsigned char tmp = *((unsigned char*)&value);
    unsigned int length = strlen(text);
    String_ReplaceRange(&nameStringFlags, 0, replacedLength, text, text + length, (char*)&tmp);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Apply the animation and a temporary head controller, update the matrices and
// bounds, then draw the visible pet. useViewDraw selects the camera-aware draw
// path; drawEnabled gates drawing only, not the matrix or bounds refresh.
void CPetBody::Draw(void* useViewDraw, void* drawEnabled) {
    const unsigned int* base = s_petBodyDrawStateDefaults;
    if (!(flags & kPetHidden)) {
        void* entry;
        int changed = 0;
        // Install a temporary head adjustment only when there is none already.
        // 0x34 = playback | matrix-dirty | the additional head-check bit.
        if (flags & kPetNeedsHeadCheck) {
            entry = (void*)WPuppet_FindBoneByName(puppet, petBodyStrings + kPetBody_headBone);
            if (entry && !WPuppet_HasBoneControllerEntry(puppet, ((PetBoneControllerRecord*)entry)->controllerKey)) {
                float x = lookAtPitch;
                x -= s_petLookAtPitchReference;
                x = fabsd(x);
                // Update when either angle has changed.
                float y;
                if (!lt(x, kEpsilon) || (y = lookAtYaw, y -= s_petLookAtYawReference, y = fabsd(y), !lt(y, kEpsilon))) {
                    Arr3 t;
                    Memory_Clear(&t, 0xC);
                    t.v[0] = -lookAtYaw;
                    t.v[2] = -lookAtPitch;
                    changed = 1;
                    WPuppet_SetBoneControllerVector(puppet, ((PetBoneControllerRecord*)entry)->controllerKey, &t);
                }
            }
        }
        unsigned int currentFlags = flags;
        // Refresh the pose, world matrices and bounds when playing or dirty.
        // Bit 1 is forwarded as the animation routine's fourth argument.
        if (currentFlags & kPetNeedsAnimationOrMatrix) {
            WPuppet_SetAnimationTimeFromRanges(puppet, 0, animationTime, (flags >> 1) & 1, 0);
            if (!(flags & kPetUsesRootMotion)) {
                Model_SetMatrixRecursive(puppet, puppet, &matrixBasis);
            } else {
                PetAnimationRange* range = currentAnimation;
                float t = animationTime - range->startTime;
                float frame = (float)(int)(kFramesPerSecond * t + kHalf);
                Arr3 matrix;
                ((void (*)(void*, void*, void*, float, float))WPuppet_GetRootMotionDelta)(
                    &matrix, puppet, ((PetAnimationRange*)range)->name, kFloat_0_petBody, frame);
                matrix.v[2] *= kFloat_neg1_PetBody;
                // Root motion is local-space with Z reversed: rotate it into world
                // space and subtract it only while propagating the model matrix,
                // then restore the pet's stored translation.
                Arr3 out = matrixPosition;
                WVector tmp;
                Matrix_TransformDirection(&tmp, &matrix, &matrixBasis);
                *(WVector*)((char*)&matrixPosition) -= tmp;
                Model_SetMatrixRecursive(puppet, puppet, &matrixBasis);
                __as__7WVectorFRC7WVector(&matrixPosition, &out);
            }
            WPuppet_UpdateBoneWorldMatrices(puppet, 0, 0, kFloat_1_petBody);
            WPuppet_UpdateBoundsFromBone(puppet);
        }
        // Remove only this call's temporary head adjustment, after the updates.
        if (changed) {
            entry = (void*)WPuppet_FindBoneByName(puppet, petBodyStrings + kPetBody_headBone);
            if (entry)
                WPuppet_EraseBoneControllerEntry(puppet, ((PetBoneControllerRecord*)entry)->controllerKey);
        }
        entry = (char*)&((WPuppet*)puppet)->boundsCenter;
        if (WView_DefaultCapabilityTrueA(g_view, entry)) {
            // Prepare the lighting and direction parameters before either path.
            if (flags & kPetNeedsAnimationOrMatrix) {
                if (defaultRenderParametersEnabled) {
                    ((PetDevice*)puppet)->v5(&lightingMode, 0, 0);
                }
                if (renderParametersEnabled) {
                    void* p = renderParameters;
                    while (p != (char*)renderParameters + renderParameterCount * 0x28) {
                        WVector tmp;
                        Matrix_TransformDirection(&tmp, &((PetRenderParameter*)p)->sourceDirection, &matrixBasis);
                        __as__7WVectorFRC7WVector(&((PetRenderParameter*)p)->transformedDirection, &tmp);
                        ((PetDevice*)puppet)->v5(p, 1, 0);
                        p = (char*)p + sizeof(PetRenderParameter);
                    }
                }
            }
            if (drawEnabled) {
                if (useViewDraw) {
                    void* elem = WView_DefaultAcceptVisibility(g_view, entry);
                    ((PetDevice*)puppet)->v6(g_view, 1, elem, 0, uniformScale, 0);
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
                    void* renderState = Camera_GetRenderState(g_view);
                    SetTransform(1, &((PetCameraMatrices*)renderState)->projection);
                    gtl_GenericRenderer_SetProjectionMatrix(&((PetCameraMatrices*)renderState)->projection);
                    SetTransform(0, &((PetCameraMatrices*)renderState)->view);
                    gtl_GenericRenderer_SetViewMatrix(&((PetCameraMatrices*)renderState)->view);
                    void* skinMatrices = WPuppet_GetSkinMatrices(puppet);
                    ((PetDevice*)puppet)->v10(g_view, 1, 1, uniformScale, 0, 0, skinMatrices, 0);
                }
            }
        }
        flags &= ~kPetLookAtDirty;
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Use the course lighting for golf tasks; otherwise install a normalized default
// light and white colors.
void CPetBody::SetupLighting() {
    if (puppet)
        WPuppet_UpdateBound(puppet, 0);
    renderParametersEnabled = 1;
    void* obj = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    g_petBodyTaskChild = obj;
    if (FindInChain(obj)) {
        void* p = puppet;
        if (p) {
            float v[3];
            float z = positionZ;
            float y = ((WPuppet*)p)->boundsMax.y;
            float x = positionX;
            v[0] = x;
            v[1] = y;
            v[2] = z;
            float brightness = ((CQuadTree*)(CQuadTree_GetInstance(v[1], v[2])))->DefaultLightingFactor(v);
            lightColor = (int)Color_ScaleRGB((void*)s_sceneLighting[4], brightness);
        }
        ambientLightColor = s_sceneLighting[5];
        __as__7WVectorFRC7WVector(&lightDirectionX, s_sceneLighting + 1);
    } else {
        struct Vector {
            float x, y, z;
            Vector(float a, float b, float c) : x(a), y(b), z(c) {}
        };
        unsigned int k = 0xFFFFFF;
        lightingMode = 2;
        lightColor = k;
        ambientLightColor = k;
        Vector v(kFloat_0_petBody, kFloat_neg1_PetBody, kFloat_1_petBody);
        __as__7WVectorFRC7WVector(&lightDirectionX, &v);
        (*(WVector*)&lightDirectionX).Normalize();
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Consume sampled root motion once, update the flight steering, and synchronize
// the translations.
void CPetBody::Update(float dt) {
    unsigned int currentFlags = flags;
    if (currentFlags & kPetAnimationPlaying) {
        UpdateAnimation(dt * animationSpeed);
        currentFlags = flags;
        if (currentFlags & kPetUsesRootMotion) {
            if (currentFlags & kPetRootMotionSampled) {
                // Sampling uses puppet-local axes: rotate into world axes, scale,
                // consume the sample flag, and add this interval's displacement.
                char b[0x0C], a[0x0C];
                unsigned int f = *(volatile unsigned int*)&flags;
                f = f & ~kPetRootMotionSampled;
                flags = f;
                Matrix_TransformDirection(a, &rootMotionDeltaX, &baseMatrixBasis);
                __ml__FfRC7WVector(b, a, rootMotionScale);
                __as__7WVectorFRC7WVector(&movementDirection, b);
                __apl__7WVectorFRC7WVector(&positionX, &movementDirection);
            }
            UpdateMovement(dt);
            // Movement may change the height and lateral position after the
            // root-motion step.
            __as__7WVectorFRC7WVector(&baseMatrixPositionX, &positionX);
            __as__7WVectorFRC7WVector(&matrixPosition, &positionX);
        }
    }
    void* obj = puppet;
    if (obj && WPuppet_FindBoneByName(obj, petBodyStrings + kPetBody_headBone))
        UpdateLookAt(dt);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Dispatch the interval's events, advance the time, and spend any overflow in
// the following clip.
void CPetBody::UpdateAnimation(float dt) {
    if (!currentAnimation || currentAnimation->startTime == currentAnimation->endTime) {
        // A missing or zero-duration clip stops playback and root motion.
        flags = flags & ~(kPetAnimationPlaying | kPetUsesRootMotion);
        return;
    }
    float step = ComputeAnimationStep(dt);
    if (!(step > kFloat_0_petBody))
        return;
    // Notify the events in the bounded interval before publishing its end time.
    float intervalStart = animationTime;
    float eventStep = step;
    float eventEnd = intervalStart + eventStep;
    AnimationEventParser_ProcessFrameRange(this, puppet, intervalStart, eventEnd, currentAnimation, -1, 0);
    float advanceStep = step;
    float intervalEnd = animationTime + advanceStep;
    float publishedTime = intervalEnd;
    animationTime = publishedTime;
    if (intervalEnd >= currentAnimation->endTime) {
        // A queued clip takes priority over the current range's repeat name.
        // The unspent time is the recursive call's dt and can cross several clips.
        if (*(signed char*)((char*)queuedAnimationName)) {
            PlayAnimationByName(dt - step, queuedAnimationName, queuedAnimationUsesRootMotion, 0);
            queuedAnimationName[0] = 0;
        } else {
            float repeatStep = step;
            PlayAnimationByName(dt - repeatStep, (void*)currentAnimation->repeatName, (flags >> 1) & 1, 1);
            animationFinished = 1;
        }
        float consumedStep = step;
        UpdateAnimation(dt - consumedStep);
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Clamp the interval and sample the puppet-local root displacement with rounded
// frame indices.
float CPetBody::ComputeAnimationStep(float dt) {
    float maximum = currentAnimation->maximumTime;
    float end = animationTime + dt;
    end = (maximum > end) ? end : maximum;
    float additionalRootMotion[3];
    unsigned int currentFlags = flags;
    if (currentFlags & kPetUsesRootMotion) {
        if (!(currentFlags & kPetRootMotionSampled)) {
            // The first interval seeds the accumulator; later intervals add to it
            // until the update consumes it. Flip puppet Z into the pet's axes.
            float rangeOffset = animationTime - currentAnimation->startTime;
            float sampledRootMotion[3];
            float savedEnd = end;
            float savedRangeOffset = rangeOffset;
            int firstFrame = (int)(kFramesPerSecond * savedRangeOffset + kHalf);
            int lastFrame = (int)((savedEnd - currentAnimation->startTime) * kFramesPerSecond + kHalf);
            WPuppet_GetRootMotionDelta(sampledRootMotion, (float)firstFrame, puppet, (float)lastFrame,
                                       currentAnimation->name);
            __as__7WVectorFRC7WVector(&rootMotionDeltaX, sampledRootMotion);
            rootMotionDeltaZ *= kFloat_neg1_PetBody;
            flags |= kPetRootMotionSampled;
        } else {
            WPuppet_SetAnimationTimeFromRanges(puppet, 0, animationTime, (2 & currentFlags) != 0, 0);
            float rangeOffset = animationTime - currentAnimation->startTime;
            float savedEnd2 = end;
            int firstFrame = (int)(kFramesPerSecond * rangeOffset + kHalf);
            int lastFrame = (int)(kFramesPerSecond * (savedEnd2 - currentAnimation->startTime) + kHalf);
            WPuppet_GetRootMotionDelta(additionalRootMotion, (float)firstFrame, puppet, (float)lastFrame,
                                       currentAnimation->name);
            additionalRootMotion[2] *= kFloat_neg1_PetBody;
            *(WVector*)((char*)&rootMotionDeltaX) += *(WVector*)additionalRootMotion;
        }
    }
    return end - animationTime;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Copy a borrowed world-space vector and synchronize both matrix translations.
void CPetBody::SetPosition(const void* sourcePosition) {
    __as__7WVectorFRC7WVector(&positionX, sourcePosition);
    __as__7WVectorFRC7WVector(&baseMatrixPositionX, &positionX);
    __as__7WVectorFRC7WVector(&matrixPosition, &positionX);
    unsigned int currentFlags = flags;
    currentFlags |= kPetMatrixDirty;
    currentFlags &= ~2u;
    flags = currentFlags;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Copy the borrowed Euler angles (in radians), compose X*Y*Z, and keep the world
// translation.
void CPetBody::SetRotation(const void* sourceEulerRadians) {
    __as__7WVectorFRC7WVector(&rotationX, sourceEulerRadians);
    char e[0x30], d[0x30], c[0x30], b[0x30], a[0x30];
    Matrix_SetAxisRotation(a, rotationZ, 2);
    Matrix_SetAxisRotation(b, rotationY, 1);
    Matrix_SetAxisRotation(c, rotationX, 0);
    Matrix_ConcatAffine(d, c, b);
    Matrix_ConcatAffine(e, d, a);
    Matrix_CopyAffine(&baseMatrixBasis, e);
    __as__7WVectorFRC7WVector(&baseMatrixPositionX, &positionX);
    Matrix_CopyAffine(&matrixBasis, &baseMatrixBasis);
    flags |= kPetMatrixDirty;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Rebuild the base and current orientation for the requested yaw and clear the
// movement direction.
void CPetBody::SetYaw(float value) {
    float zero = kFloat_0_petBody;
    char a[0x18];
    char b[0x30];
    char* self = (char*)this;
    struct Pair {
        unsigned int words[2];
    };
    *(Pair*)(a + 0x0C) = *(Pair*)(self + 0x38);
    unsigned int v2 = *(unsigned int*)(self + 0x40);
    *(unsigned int*)(a + 0x14) = v2;
    ((float*)a)[0] = zero;
    ((float*)a)[1] = value;
    ((float*)a)[2] = zero;
    __as__7WVectorFRC7WVector(self + 0x74, a);
    Matrix_SetAxisRotation(b, value, 1);
    Matrix_CopyAffine(self + 0x14, b);
    __as__7WVectorFRC7WVector(self + 0x38, a + 0x0C);
    Matrix_CopyAffine(self + 0x44, self + 0x14);
    *(unsigned int*)(self + 0xC8) |= kPetMatrixDirty;
    Vec3_Zero(self + 0x8C);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Compose an external rotation with the pet's normalized rotation, keeping the
// translation.
void CPetBody::ApplyRotationMatrix(void* rotationMatrix) {
    if (rotationMatrix) {
        unsigned int params[3];
        char converted[0x30];
        char result[0x30];
        struct Pair {
            unsigned int words[2];
        };
        *(Pair*)params = *(Pair*)&matrixPosition;
        params[2] = ((unsigned int*)&matrixPosition)[2];
        Matrix_ExtractUniformScaleNormalizedRotation(matrixBasis, converted);
        Matrix_ConcatAffine(result, converted, rotationMatrix);
        Matrix_CopyAffine(&matrixBasis, result);
        __as__7WVectorFRC7WVector(&matrixPosition, params);
        flags |= kPetMatrixDirty;
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Set the movement destination and initialize the flight pitch, height and
// vertical speed.
void CPetBody::StartMovement(void* destination) {
    movementState = 0;
    __as__7WVectorFRC7WVector(&movementStart, &positionX);
    __as__7WVectorFRC7WVector(&movementTarget, destination);
    float c;
    float b;
    float a;
    a = kFloat_neg0_7853982_PetBody;
    b = kFloat_0_petBody;
    c = kFloat_50_PetBody;
    movementPitchRadians = a;
    minimumHeight = b;
    verticalVelocity = c;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Resolve a pet sound command and play it at the pet position with the task's
// sound parameters.
void CPetBody::PlaySound(const char* text) {
    if (text) {
        char copy[0x0C];
        WVector temp = *(WVector*)((char*)&positionX) - *(WVector*)(s_cameraTransform + 0x24);
        __as__7WVectorFRC7WVector(copy, &temp);
        char* marker = strchr(text, 0x2D);
        if (marker) {
            unsigned int length = marker + 1 - text;
            if (!strncmp(marker + 1, petBodyStrings + kPetBody_chanceMarker, 4)) {
                int difference;
                void* manager = g_sharedDoc;
                if (((CSharedDoc*)manager)->golfRuleType == 1) {
                    void* data = (void*)((CSharedDoc*)manager)->golfDoc;
                    unsigned char index = ((CGolfDoc*)data)->currentHoleIndex;
                    unsigned char* table =
                        (unsigned char*)GolfPlayer_GetEntryByIndex(data, ((CGolfDoc*)data)->currentPlayer);
                    void* manager2 = g_sharedDoc;
                    void* data2 = (void*)((CSharedDoc*)manager2)->golfDoc;
                    index = ((CGolfDoc*)data2)->currentHoleIndex;
                    unsigned char* base = (unsigned char*)((CGolfDoc*)data2)->holeRecords;
                    unsigned char* tp = table + index;
                    unsigned char* bp = base;
                    difference = tp[0x28] - bp[index * 0x20 + 0x18];
                } else {
                    void* data = (void*)((CSharedDoc*)manager)->golfDoc;
                    unsigned char index = ((CGolfDoc*)data)->currentHoleIndex;
                    unsigned char* table =
                        (unsigned char*)GolfPlayer_GetEntryByIndex(data, ((CGolfDoc*)data)->currentPlayer);
                    void* manager2 = g_sharedDoc;
                    void* data2 = (void*)((CSharedDoc*)manager2)->golfDoc;
                    index = ((CGolfDoc*)data2)->currentHoleIndex;
                    unsigned char* base = (unsigned char*)((CGolfDoc*)data2)->holeRecords;
                    unsigned char* tp = table + index;
                    unsigned char* bp = base;
                    difference = tp[0x28] - bp[index * 0x20 + 0x18];
                }
                if (*((unsigned char*)GolfBall__Fv() + 0x59) == 2 && (unsigned int)(difference + 2) <= 1u) {
                    char rebuilt[0x40];
                    strncpy(rebuilt, text, length);
                    switch (difference) {
                    case -1:
                        strcpy(rebuilt + length, petBodyStrings + kPetBody_birdieText);
                        break;
                    case -2:
                        strcpy(rebuilt + length, petBodyStrings + kPetBody_scoreTextSuffix);
                        break;
                    }
                    strcat(rebuilt, text + length);
                    strcpy((char*)text, rebuilt);
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
            if (q == &s_golfTaskTypeInfo) {
                found = 1;
                break;
            }
            q = ((PetTypeInfoLink*)q)->parent;
        }
    }
    if (found) {
        unsigned int pair[2] = {kPetPositionalSoundOptionWord0, kFloat_1000_PetSound};
        Sound_PlayByName(g_soundManager, text, &positionX, 0, 0, pair, kFloat_0_5_PetBody, kFloat_200_PetBody,
                         kFloat_1_petBody);
    } else {
        Sound_PlayByName(g_soundManager, text, 0, 0, 0, 0, kFloat_0_5_PetBody, kFloat_200_PetBody, kFloat_1_petBody);
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Resolve the next clip, dispatch the events skipped by the offset and reconcile
// root-motion ownership. The name is borrowed; the two pointer-shaped flags are
// tested only for nonzero.
void CPetBody::PlayAnimationByName(float startOffset, void* animationName, int useRootMotion, int preserveRootMotion) {
    PetAnimationRange* animation = currentAnimation;
    animationFinished = 1;
    currentAnimation = (PetAnimationRange*)WPuppet_FindAnimationRangeByName(puppet, animationName);
    if (currentAnimation == 0) {
        currentAnimation = (PetAnimationRange*)WPuppet_FindAnimationRangeByName(
            puppet, (void*)(petBodyStrings + kPetBody_facialAnimationMarker));
    }
    if (currentAnimation != 0) {
        // Clamp the entry time and deliver the events skipped by the offset.
        flightMovementEnabled = 0;
        float end = currentAnimation->endTime;
        float time = currentAnimation->startTime + startOffset;
        animationTime = time > end ? end : time;
        animationTime = kFloat_0_petBody < animationTime ? animationTime : kFloat_0_petBody;
        if (animationTime > currentAnimation->startTime) {
            AnimationEventParser_ProcessFrameRange(this, puppet, currentAnimation->startTime, animationTime,
                                                   currentAnimation, -1, 0);
        }
        flags |= kPetAnimationPlaying;
        if (useRootMotion) {
            flags |= kPetUsesRootMotion;
        } else {
            // With root motion disabled and preservation requested, add the old
            // clip's inverse terminal offset so the world position stays
            // continuous. This needs an old clip and a new clip of nonzero length.
            int moving = 0;
            int active = 0;
            if (animation && preserveRootMotion)
                active = 1;
            if (active) {
                void* current = currentAnimation;
                int small = AnimationAbs(*((float*)current + 1) - *(float*)current) < kEpsilon;
                if (!small)
                    moving = 1;
            }
            if (moving) {
                float transformed[3];
                WVector sampled;
                WPuppet_GetInverseRootMotionOffsetForRange(
                    &sampled, puppet,
                    (float)(int)(kFramesPerSecond * (*((float*)animation + 1) - *(float*)animation) + kHalf),
                    (((int)((PetAnimationRange*)animation)->name)));
                *(WVector*)transformed = sampled;
                WVector result;
                Matrix_TransformDirection(&result, transformed, &baseMatrixBasis);
                *(WVector*)transformed = result;
                transformed[2] *= kFloat_neg1_PetBody;
                float savedLbl_805DF5AC = transformed[0] * kFloat_neg1_PetBody;
                transformed[0] = savedLbl_805DF5AC;
                WVector rotated = *(WVector*)transformed * rootMotionScale;
                *(WVector*)transformed = rotated;
                WVector diff = *(WVector*)&positionX + *(WVector*)transformed;
                *(WVector*)&positionX = diff;
                *(WVector*)&baseMatrixPositionX = *(WVector*)&positionX;
                *(WVector*)&matrixPosition = *(WVector*)&positionX;
                flags |= kPetMatrixDirty;
                flags &= ~2u;
            }
            flags &= ~2u;
        }
        animationFinished = 0;
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Apply the uniform scale to the pet and to the loaded puppet's matrix.
void CPetBody::SetUniformScale(float value) {
    uniformScale = value;
    float vec[3] = {value, value, value};
    Matrix_SetBasisScale(&matrixBasis, vec);
    void* other = puppet;
    if (other != 0) {
        Matrix_SetBasisScale(&((WPuppet*)other)->worldMatrix, vec);
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Smooth the head pitch and yaw toward a world target, optionally turning the
// body beyond the head's range. The pitch comes from the head bone's origin and
// the yaw from the base body frame. There is no coincident-target guard: a zero
// horizontal distance still reaches atan(dY / 0), including 0/0.
void CPetBody::UpdateLookAt(float delta) {
    if (followTarget != 0) {
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
        // Derive the pitch from the head origin and the yaw from the base frame.
        Pet_ResolveLookAtAngles(this, headToTargetWorld, bodyToTargetWorld, yaw);
        // Turn the body only by the yaw beyond the 45-degree head range.
        if (turnToTarget != 0) {
            float absYaw = targetYaw;
            absYaw = absYaw > kFloat_0_petBody ? absYaw : -absYaw;
            if (absYaw > kFloat_0_7853982_PetBody) {
                float absoluteTargetYaw = targetYaw;
                absoluteTargetYaw = absoluteTargetYaw > kFloat_0_petBody ? absoluteTargetYaw : -absoluteTargetYaw;
                float excessYaw = absoluteTargetYaw - kFloat_0_7853982_PetBody;
                excessYaw = targetYaw > kFloat_0_petBody ? excessYaw : -excessYaw;
                bodyYawDelta[0] = kFloat_0_petBody;
                bodyYawDelta[1] = excessYaw;
                bodyYawDelta[2] = kFloat_0_petBody;
                __pl__FRC7WVectorRC7WVector(nextBodyEuler, &rotationX, bodyYawDelta);
                __as__7WVectorFRC7WVector(&rotationX, nextBodyEuler);
                Pet_RebuildBodyFrame(this, rotationZMatrix, rotationYMatrix, rotationXMatrix, rotationXYMatrix,
                                     rotationXYZMatrix);
            }
        }
        targetYaw = Pet_ClampLookAtYaw(targetYaw);
    } else {
        Vec2_Zero(&targetPitch);
    }
    // Advance the head angles at pi/2 radians per unit of update time, with a
    // one-degree deadband.
    float remainingHeadAngles[2];
    FloatPair headAngleDelta = Vec2_Subtract(&targetPitch, &lookAtPitch);
    remainingHeadAngles[0] = headAngleDelta.first;
    remainingHeadAngles[1] = headAngleDelta.second;
    float remainingPitch = remainingHeadAngles[0];
    if ((remainingPitch > kFloat_0_petBody ? remainingPitch : -remainingPitch) > kOneDegree) {
        flags |= kPetHeadAnglesDirty;
        float remainingAngleLength = Vec2_CalculateLength(remainingHeadAngles);
        float stepFraction = kFloat_1_5707964_PetMotion * delta / remainingAngleLength;
        stepFraction = (stepFraction > kFloat_1_petBody) ? kFloat_1_petBody : stepFraction;
        lookAtPitch += remainingHeadAngles[0] * stepFraction;
    }
    if ((remainingHeadAngles[1] > kFloat_0_petBody ? remainingHeadAngles[1] : -remainingHeadAngles[1]) > kOneDegree) {
        flags |= kPetHeadAnglesDirty;
        float remainingAngleLength = Vec2_CalculateLength(remainingHeadAngles);
        float stepFraction = kFloat_1_5707964_PetMotion * delta / remainingAngleLength;
        stepFraction = (stepFraction > kFloat_1_petBody) ? kFloat_1_petBody : stepFraction;
        lookAtYaw += remainingHeadAngles[1] * stepFraction;
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Advance the takeoff, cruise and landing height policy, then steer laterally
// toward the ball. World positions are Y-up and delta is the unscaled update
// interval, so the height and pitch rates are per second, not per frame.
void CPetBody::UpdateMovement(float delta) {
    void* ball = GolfBall__Fv();
    PetVector boundsMaxWorld, boundsMinWorld;
    PetVector current = *(PetVector*)&positionX;
    PetVector boneOriginWorld, lateralAxis;
    current.y = kFloat_0_petBody;
    PetVector routeDelta;
    __mi__FRC7WVectorRC7WVector(&routeDelta, &movementTarget, &movementStart);
    float routeDistance = Magnitude__7WVectorCFv(&routeDelta);
    PetVector remainingDelta;
    __mi__FRC7WVectorRC7WVector(&remainingDelta, &movementTarget, &current);
    float remainingDistance = Magnitude__7WVectorCFv(&remainingDelta);
    // The height and pitch transitions use fractions of the original route.
    switch (*(int*)&movementState) {
    case kPetTakeoff:
        if (flightMovementEnabled) {
            // Takeoff: flatten a negative pitch toward zero by 0.4 * delta radians.
            float turn;
            if (*(unsigned int*)&movementPitchRadians & 0x80000000u)
                turn = -movementPitchRadians;
            else
                turn = movementPitchRadians;
            if (!Pet_IsBelowEpsilon(turn)) {
                float next = kFloat_0_4_PetBody * delta + movementPitchRadians;
                if (next < kFloat_0_petBody)
                    movementPitchRadians = next;
                else
                    movementPitchRadians = kFloat_0_petBody;
                PetVector flightDirection, forwardAxis, eulerRadians;
                float rotationZMatrix[12], rotationYMatrix[12], rotationXMatrix[12], rotationXYMatrix[12],
                    rotationXYZMatrix[12];
                Pet_RebuildFlightRotation(this, flightDirection, forwardAxis, eulerRadians, rotationZMatrix,
                                          rotationYMatrix, rotationXMatrix, rotationXYMatrix, rotationXYZMatrix);
            }
            if (((PetMovementBall*)ball)->position[1] > minimumHeight)
                minimumHeight = ((PetMovementBall*)ball)->position[1];
            // Follow the ball's rising height floor: ascend with verticalVelocity,
            // reduce positive velocity by 25*delta, and lower overshoot above
            // floor+10.
            if (positionY < minimumHeight) {
                float velocity = verticalVelocity;
                float height = velocity * delta + positionY;
                positionY = height;
                if (velocity > kFloat_0_petBody)
                    verticalVelocity = velocity - kFloat_25_PetBody * delta;
            }
            if (positionY > kFloat_10_PetBody + minimumHeight)
                positionY -= kFloat_10_PetBody * delta;
            if (kFloat_1_5_PetBody * remainingDistance <= routeDistance) {
                // Remaining distance is at most 2/3 of the route: takeoff ends
                // and the base orientation resumes.
                movementPitchRadians = kFloat_0_petBody;
                *(int*)&movementState = kPetCruise;
                Matrix_CopyAffine(&matrixBasis, &baseMatrixBasis);
            }
        } else {
            movementPitchRadians = kFloat_0_petBody;
            *(int*)&movementState = kPetCruise;
            Matrix_CopyAffine(&matrixBasis, &baseMatrixBasis);
        }
        break;
    case kPetCruise:
        // Remaining at most 1/3 of the route begins landing. Without flight, each
        // update advances one phase without applying the pitch and height policy.
        if (flightMovementEnabled) {
            if (kFloat_3_PetBody * remainingDistance <= routeDistance)
                *(int*)&movementState = kPetLanding;
        } else
            *(int*)&movementState = kPetLanding;
        break;
    case kPetLanding: {
        // Derive the clearance from a named bone's transformed bounds and the
        // terrain. A missing name or bone leaves drop at zero; descent is capped
        // by 36 * delta.
        float drop = kFloat_0_petBody;
        int small = Pet_IsZero(*(unsigned int*)&nameStringFlags >> 31);
        unsigned int size = small ? (*(unsigned char*)&nameStringFlags & 0x7F) : (*(int*)&nameStringLength);
        if (size) {
            void* node =
                WPuppet_FindNamedBoneEntry(puppet, small ? ((char*)&nameStringFlags + 1) : (void*)nameStringData);
            if (node) {
                PetVector* bounds = ((PetVector*)((PetNamedBoneEntry*)node)->bounds);
                struct Pair {
                    unsigned int words[2];
                };
                *(Pair*)&boundsMinWorld = *(Pair*)bounds;
                ((int*)&boundsMinWorld)[2] = ((int*)bounds)[2];
                *(Pair*)&boundsMaxWorld = *(Pair*)(bounds + 1);
                ((int*)&boundsMaxWorld)[2] = ((int*)(bounds + 1))[2];
                PetVector t0;
                Matrix_TransformPoint(&t0, &boundsMinWorld, &((PetBoneMatrix*)*(void**)node)->matrix[0]);
                __as__7WVectorFRC7WVector(&boundsMinWorld, &t0);
                PetVector t1;
                Matrix_TransformPoint(&t1, &boundsMaxWorld, &((PetBoneMatrix*)*(void**)node)->matrix[0]);
                __as__7WVectorFRC7WVector(&boundsMaxWorld, &t1);
                float sumX;
                float half;
                float sumZ;
                sumX = boundsMinWorld.x + boundsMaxWorld.x;
                half = kFloat_0_5_PetBody;
                sumZ = boundsMinWorld.z + boundsMaxWorld.z;
                boundsMinWorld.x = sumX * half;
                boundsMinWorld.z = sumZ * half;
                Vec3_Zero(&boneOriginWorld);
                PetVector t2;
                Matrix_TransformPoint(&t2, &boneOriginWorld, &((PetBoneMatrix*)*(void**)node)->matrix[0]);
                __as__7WVectorFRC7WVector(&boneOriginWorld, &t2);
                PetVector ground, position;
                ((int*)&position)[0] = ((int*)&boundsMinWorld)[0];
                ((int*)&position)[1] = ((int*)&boundsMinWorld)[1];
                ((int*)&position)[2] = ((int*)&boundsMinWorld)[2];
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&ground, ((void* (*)())CQuadTree_GetInstance)(), &position, 1,
                                                        0, 1);
                float groundHeight = ground.y;
                drop = boneOriginWorld.y - groundHeight;
            }
        }
        if (flightMovementEnabled) {
            // Compare against the stored pitch target, but clamp the decrement
            // independently to -pi/8 radians at 0.3 radians per time unit.
            float turn = movementPitchRadians - s_petLandingPitchTargetRadians;
            if (!Pet_IsMagnitudeBelowEpsilon(turn)) {
                float next = movementPitchRadians - kFloat_0_3_PetBody * delta;
                if (next > kFloat_neg0_3926991_PetPitch)
                    movementPitchRadians = next;
                else
                    movementPitchRadians = kFloat_neg0_3926991_PetPitch;
                PetVector flightDirection, forwardAxis, eulerRadians;
                float rotationZMatrix[12], rotationYMatrix[12], rotationXMatrix[12], rotationXYMatrix[12],
                    rotationXYZMatrix[12];
                Pet_RebuildFlightRotation(this, flightDirection, forwardAxis, eulerRadians, rotationZMatrix,
                                          rotationYMatrix, rotationXMatrix, rotationXYMatrix, rotationXYZMatrix);
            }
            float step = kFloat_36_PetBody * delta;
            drop = step > drop ? drop : step;
        } else {
            movementPitchRadians = kFloat_0_petBody;
            Matrix_CopyAffine(&matrixBasis, &baseMatrixBasis);
            float step = kFloat_36_PetBody * delta;
            drop = step > drop ? drop : step;
        }
        positionY -= drop;
        break;
    }
    }
    // Steer along normalize(up x travel) while the ball's scalar magnitude
    // exceeds the tolerance, applying the projected lateral error * 1.2 * delta.
    // There is no forward correction or explicit overshoot clamp here.
    if (flightMovementEnabled) {
        float speed;
        if (*(unsigned int*)&((PetMovementBall*)ball)->movementScalar & 0x80000000u)
            speed = -((PetMovementBall*)ball)->movementScalar;
        else
            speed = ((PetMovementBall*)ball)->movementScalar;
        if (!Pet_IsBelowEpsilon(speed)) {
            PetVector up = {kFloat_0_petBody, kFloat_1_petBody, kFloat_0_petBody};
            PetVector ballOffset, steeringStep, steeringVelocity, lateralError;
            Pet_ApplyLateralBallCorrection(this, ball, delta, lateralAxis, up, ballOffset, steeringStep,
                                           steeringVelocity, lateralError);
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
// Set the tracking target and turn behavior when the puppet has a head bone.
void CPetBody::SetLookAtTarget(void* target, int turnBody) {
    void* other = puppet;
    if (other != 0 && WPuppet_FindBoneByName(other, petBodyStrings + kPetBody_headBone)) {
        followTarget = target;
        turnToTarget = turnBody;
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Queue an animation name and its playback option.
char* CPetBody::SetNextAnimation(const char* source, int useRootMotion) {
    queuedAnimationUsesRootMotion = useRootMotion;
    return strcpy((char*)queuedAnimationName, source);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Apply the current animation time and compensate the puppet matrix for the
// sampled root motion.
void CPetBody::ApplyAnimationToPuppet() {
    unsigned int currentFlags = flags;
    WPuppet_SetAnimationTimeFromRanges(puppet, 0, animationTime, (currentFlags >> 1) & 1, 0);
    if ((flags & kPetUsesRootMotion) == 0) {
        void* value = puppet;
        Model_SetMatrixRecursive(value, value, &matrixBasis);
    } else {
        PetAnimationRange* range = currentAnimation;
        float result[3];
        ((void (*)(void*, void*, int, float, float))WPuppet_GetRootMotionDelta)(
            result, puppet, ((int)(range->name)), kFloat_0_petBody,
            (float)(int)(kFramesPerSecond * (animationTime - range->startTime) + kHalf));
        result[2] = result[2] * kFloat_neg1_PetBody;
        Arr3 old = matrixPosition;
        WVector next;
        Matrix_TransformDirection(&next, result, &matrixBasis);
        *(WVector*)((char*)&matrixPosition) -= next;
        void* value = puppet;
        Model_SetMatrixRecursive(value, value, &matrixBasis);
        *(WVector*)((char*)&matrixPosition) = *(WVector*)&old;
    }
    WPuppet_UpdateBoneWorldMatrices(puppet, 0, 0, kFloat_1_petBody);
    WPuppet_UpdateBoundsFromBone(puppet);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
void* CPetBody::GetPuppet() {
    return puppet;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Return the current animation range's name, or null when none is selected.
const char* CPetBody::GetCurrentAnimationName() {
    PetAnimationRange* value = currentAnimation;
    if (value == 0) {
        return 0;
    }
    return (const char*)((PetAnimationRange*)value)->name;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Set the puppet's opacity recursively when the model is loaded.
void CPetBody::SetOpacity(unsigned char opacity) {
    void* value = puppet;
    if (value != 0) {
        WPuppet_SetOpacityRecursive(value, opacity);
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Return the loaded puppet's minimum bounds vector.
void* CPetBody::GetPuppetBoundsMin() {
    return &((WPuppet*)puppet)->boundsMin;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Return the mutable queued-animation name buffer.
char* CPetBody::GetNextAnimationName() {
    return (char*)queuedAnimationName;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Return the animation-finished word through the original pointer-sized ABI.
void* CPetBody::GetAnimationFinished() {
    return *(void**)&animationFinished;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Release the old puppet and request the replacement model asynchronously.
int CPetBody::RequestModelLoad(void* modelName) {
    void* value = puppet;
    if (value != 0) {
        ResourceManager_ReleaseObject(g_resourceManager, value, 0);
        puppet = 0;
    }
    ResourceManager_RequestModelLoad(g_resourceManager, modelName);
    resourcePending = 1;
    return 1;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Poll an outstanding model read and publish the resulting puppet when complete.
int CPetBody::PollModelLoad() {
    if (resourcePending != 1) {
        return 1;
    }
    if (!ResourceManager_PollResourceRead(g_resourceManager)) {
        return 0;
    }
    puppet = ResourceManager_PollPuppetResourceLoad(g_resourceManager);
    resourcePending = 0;
    return 1;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw on
// Match the registered substrings in the borrowed command text. The iterator
// access is locked, callback invocation is not. A null pointee is a no-op, but
// the cursor slot must be valid. The owner must keep the body, the list nodes,
// the match strings and the callbacks alive; recursive dispatch replaces the
// shared iterator and is therefore not reentrant.
void CPetBody::DispatchCommandCallbacks(char** commandText) {
    if (*(void**)commandText == 0)
        return;
    InstanceEntry* entry;
    {
        MutexLock outer(instanceMutex);
        void* head = *(void**)&instanceHead;
        *(void**)&instanceIterator = head;
        entry = NextInstance(this);
    }
    if (entry == 0)
        return;
    while (entry != 0) {
        char* matched = strstr(*commandText, entry->matchText);
        if (matched) {
            CallbackArgs args;
            args.index = 0;
            args.tail = 0;
            args.value = entry->flags;
            args.matched = (int)matched;
            if (entry->callback != 0)
                entry->callback->Invoke(&args);
        }
        entry = NextInstance(this);
    }
}
#pragma pop

#pragma push
#pragma exceptions on
#pragma opt_common_subs off
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Register an owned attachment from quoted tokens and advance the caller's
// cursor. The grammar is "animation" "name", or "@currentAnimation" "animation"
// "name"; the @ gate compares against the current clip. Every token must be 1 to
// 63 bytes plus the terminator. Duplicate names are rejected after all tokens
// parse. On success a 0xC node owns a 0x94 payload; the destructor releases the
// resource, the payload and the node. Allocation failure is unchecked, and no
// mutex is taken, so attachment edits must be serialized by the caller.
void CPetBody::ParseAndRegisterAttachment(char** inout) {
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
    *inout = String_ParseQuotedToken(*inout, firstToken);
    if (strlen(firstToken) == 0 || strlen(firstToken) >= 0x40)
        return;
    if (firstToken[0] == '@') {
        PetAnimationRange* source = currentAnimation;
        if (source == 0 || CString_Compare(firstToken + 1, ((const char*)((PetAnimationRange*)source)->name)) != 0)
            return;
        *inout = String_ParseQuotedToken(*inout, animationToken);
        if (strlen(animationToken) == 0 || strlen(animationToken) >= 0x40)
            return;
    } else {
        strcpy(animationToken, firstToken);
    }
    *inout = String_ParseQuotedToken(*inout, attachmentName);
    if (strlen(attachmentName) == 0 || strlen(attachmentName) >= 0x40)
        return;
    copy = attachmentName;
    pThird = attachmentName;
    char* item;
    void* list;
    void* node;
    void* sentinel;
    node = attachmentHead;
    sentinel = ((char*)&attachmentTail);
    while (node != sentinel && !PetNameMatches(((PetAttachmentLink*)node)->value->name, pThird)) {
        node = ((PetAttachmentLink*)node)->next;
    }
    found = node;
    foundCopy = node;
    if (node != ((char*)&attachmentTail))
        return;
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
    list = ((char*)&attachmentTail);
    char* newLink = (char*)appMallocDummy__FUl(0xC);
    link = newLink;
    linkCopy = newLink;
    PetLinkGuard guard(((char*)&attachmentTail), newLink);
    void** slot = (void**)(newLink + 8);
    new (slot) PetLinkValue(item);
    void* tail = *(void**)list;
    guard.node = 0;
    ((PetAttachmentLink*)tail)->next = (PetAttachmentLink*)newLink;
    *(void**)newLink = *(void**)list;
    *(void**)list = newLink;
    *(void**)(newLink + 4) = list;
    ++attachmentCount;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Enable the flight movement path.
void CPetBody::EnableFlightMovement() {
    flightMovementEnabled = 1;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Hide the pet and disable its named shadow projection.
void CPetBody::Hide() {
    flags |= kPetHidden;
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager, petBodyStrings + kPetBody_caddieGroup, 0);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Show the pet and enable its named shadow projection.
void CPetBody::Show() {
    flags &= ~((unsigned int)kPetHidden);
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager, petBodyStrings + kPetBody_caddieGroup, 1);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Forward a material-layer animation command to the puppet if it is loaded.
void CPetBody::ParseMaterialLayerAnimationCommand(void* commandText) {
    void* value = puppet;
    if (value == 0) {
        return;
    }
    if (value == 0) {
        return;
    }
    WPuppet_ParseMaterialLayerAnimationCommand(value, commandText, value);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Parse effect attachment bone references against the current puppet and clip.
void CPetBody::ParseBoneReferences(void* commandText, void* effectContext) {
    FxAttachment_ParseBoneReferences(this, commandText, effectContext, puppet, currentAnimation, 0, 0);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Parse a bounded sound token and play it through the body's sound handler.
void CPetBody::PlaySoundCommand(void* commandText) {
    char value[0x40];
    if (Parse_NonemptyQuotedTokenUnder64Bytes(this, commandText, value)) {
        PlaySound(value);
    }
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling 604
#pragma use_lmw_stmw reset
// Initialize the pet basis matrix and this unit's full-turn constant.
extern "C" void PetBody_InitBasisAndFullTurn() {
    float value = kFloat_6_2831855;
    Matrix_SetBasisElements(s_petBodyBasisMatrix, kFloat_0_petBody, kFloat_neg1_PetBody, kFloat_0_petBody,
                            kFloat_1_petBody, kFloat_0_petBody, kFloat_0_petBody, kFloat_0_petBody, kFloat_0_petBody,
                            (s_initializedTwoPi_801BBB28 = value, kFloat_1_petBody));
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Initialize the course model's containers, intrusive lists, pending filename
// and fog defaults.
extern "C" void* WPolySoup_Ctor(WPolySoup* self, void* entryContainer) {
    void* classVtable = __vt__9WPolySoup;
    void* staticPetGroupVtable = __vt__Q29WPolySoup13CStaticPetGrp;
    BaseObject_ctor(self, entryContainer);

    void* list3 = ((char*)&self->generatedHead);
    void* list1 = ((char*)&self->auxiliaryNodeHead);
    void* list2 = ((char*)&self->ambientSoundHead);
    void* end = ((char*)&self->effectTreeRoot);
    int zero = 0;
    int savedZero = zero;
    *(void**)self = classVtable;
    self->unk_10 = savedZero;
    int savedZero2 = zero;
    self->regions = savedZero2;
    int savedZero3 = zero;
    self->regionCount = zero;
    self->regionCapacity = zero;
    self->resourceEntries = zero;
    self->resourceCount = savedZero3;
    self->resources = zero;
    int savedZero4 = zero;
    self->cameras = zero;
    self->bounds = savedZero4;
    int savedZero5 = zero;
    self->namedPoints = savedZero5;
    self->surfaces = zero;
    self->fallbackSurface = zero;
    self->effectTreeCount = zero;
    self->effectTreeRoot = zero;
    self->effectTreeFirst = end;
    self->unk_C4 = 0;
    self->resourceCapabilityLatched = 0;
    self->staticPetGroupVtable = staticPetGroupVtable;
    self->staticPetCount = 0;
    self->staticPetEntries = 0;
    // Empty auxiliary list, then two pointer vectors.
    self->auxiliaryNodeCount = 0;
    self->auxiliaryNodeTail = list1;
    self->auxiliaryNodeHead = list1;
    self->auxiliaryVector1Data = 0;
    self->auxiliaryVector1Count = 0;
    self->auxiliaryVector1Capacity = 0;
    self->auxiliaryVector2Data = 0;
    self->auxiliaryVector2Count = 0;
    self->auxiliaryVector2Capacity = 0;
    self->ambientSoundCount = 0;
    self->ambientSoundTail = list2;
    self->ambientSoundHead = list2;
    self->generatedCount = 0;
    self->generatedTail = list3;
    self->generatedHead = list3;
    self->resourceIdCount = 0;
    self->resourceIds = 0;
    self->resourceIdCapacity = 0;
    self->surfaceBinaryLoadFailed = 0;
    self->resourceObjects = 0;
    self->resourceObjectCount = 0;
    self->resourceObjectCapacity = 0;
    self->pendingFilenameFlags = 0;
    self->pendingFilenameLength = 0;
    self->pendingFilenameData = 0;
    self->entryContainer = entryContainer;
    self->unk_20 = 0;
    memset((char*)self + kCourseModelInlineName, 0, kCourseModelInlineNameBytes);
    float f2 = kFloat_0_PetBody;
    float f1 = kFloat_800_PolySoup;
    float f0 = kFloat_960_PolySoup;
    self->surfaceCount = zero;
    self->unk_491C = f2;
    self->unk_B8 = f1;
    float savedF = f2;
    self->defaultFogStart = savedF;
    self->unk_BC = f0;
    self->defaultFogEnd = f2;
    self->alternateFogStart = f2;
    float savedF2 = f2;
    self->alternateFogEnd = savedF2;
    return self;
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Return the word at +0x10: several object types keep their first resource
// handle there.
extern "C" void* Object_GetWord10(void* self) {
    return *(void**)(((char*)self) + 0x10);
}
#pragma pop

#pragma push
#pragma exceptions reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma scheduling reset
#pragma use_lmw_stmw reset
// Clear and release a temporary vector, then optionally delete its wrapper.
extern "C" void* dtor_801BBCE8(void* self, int arg1) {
    return ::std::__vector_deleting_dtor((Elem_801BBCE8Vector*)self, arg1);
}
#pragma pop
