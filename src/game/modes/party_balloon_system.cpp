// Party balloon mini-game: CBalloonSystem and its small, medium and big balloons, with spawn,
// idle and hit animation, scale sequences, explosion effects and sphere collision checks.
// It also holds the balloon list and vector helpers the system uses.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
extern "C" {
extern unsigned char s_hitBalloonQueue[];
}
#include "MSL/vector.h"
// Vector destroyed by dtor_80069EA0; element type not known yet (Elem_80069EA0 is a tag);
// its size reset loads the size twice (form 2).
struct Elem_80069EA0;
namespace std {
template <> struct __vec_destroys_elements<Elem_80069EA0*> {
    enum {
        value = 2
    };
    // Pointer slots have no element destructor; the owning vector frees only its array.
    static void destroy(Elem_80069EA0**) {}
};
}
typedef std::vector<Elem_80069EA0*, app::app_allocator<Elem_80069EA0*> > Elem_80069EA0Vector;
struct BalloonEffectStrings {
    char redCharacterModel[22];     // +0x0: "balloon_chara_red.pet"
    char ballModel[17];             // +0x16: "balloon_ball.pet"
    char objectNameFormat[12];      // +0x27: "BALLOON%03d"
    char vectorLengthError[20];     // +0x33: "vector length error"
    char lengthErrorType[55];       // +0x47: "!std::exception!!std::logic_error!!std::length_error!!"
    char hammerEffect[17];          // +0x7E: "Stamp_hammer.seq"
    char hammerSound[24];           // +0x8F: "party_goods_piko_hammer"
    char recoverSound[28];          // +0xA7: "party_goods_recover_balloon"
    char smallExplosionEffect[20];  // +0xC3: "party_bln_bom01.seq"
    char smallExplosionSound[26];   // +0xD7: "party_balloon_explosion_s"
    char mediumExplosionEffect[20]; // +0xF1: "party_bln_bom02.seq"
    char mediumExplosionSound[26];  // +0x105: "party_balloon_explosion_m"
    char largeExplosionEffect[20];  // +0x11F: "party_bln_bom03.seq"
    char largeExplosionSound[26];   // +0x133: "party_balloon_explosion_l"
};

#include "classes/BalloonState80069C.h"

// Spawn kinds select size and whether the red character model/random sound slot is used.
enum BalloonSpawnKind {
    kSmallBalloon = 1,
    kMediumBalloon = 2,
    kLargeBalloon = 3,
    kSmallCharacterBalloon = 4,
    kMediumCharacterBalloon = 5,
    kLargeCharacterBalloon = 6
};
enum BalloonConstants {
    kSmallScaleEndpointCount = 2,
    kMediumScaleEndpointCount = 3,
    kLargeScaleEndpointCount = 4,
    kRandomPhaseSteps = 1000,
    kBalloonSoundSlots = 12,
    kBalloonOpaque = 255,
    kSmallBalloonSize = 10,
    kMediumBalloonSize = 17,
    kLargeBalloonSize = 25,
    kSpawnedQueueWord = 3,
    kUnhitQueueWord = 9,
    kHitQueueWord = 15,
    kHammerTargetScale = 7,
    kBalloonVectorMaxWords = 0x3FFFFFFF,
    kBalloonVectorGrowthLimit = 0x15555555,
    kBalloonVectorHalfGrowthLimit = 0x2AAAAAAA,
    kBalloonTransformOffset = 0x30,
    kBalloonAnimationStepsOffset = 0x8C,
    kBalloonAttachmentOffset = 0xA4,
    kBalloonOriginOffset = 0xE0,
    kBalloonVectorLengthErrorOffset = 0x33,
    kBalloonLengthErrorTypeOffset = 0x47
};

// Balloon units/contracts: position and origin are world-space vectors; scale is
// also the sphere-collision radius. Scale is divided by 2.56 only for the model
// matrix. bobPhase/heading are radians; opacity is a signed short clamped to 0
// during fading and sent to the puppet as an eight-bit value. Animation steps
// apply once per UpdateAnimation call (there is no elapsed-time parameter).
// The scale vector stores float endpoints as raw 32-bit words in the reconstructed
// vector helpers. currentIndex<0 means idle bob/rotation; >=0 selects the growing,
// fading hit sequence. targetScale/scaleDelta implement a separate integer step
// toward a radius, not a duration. The model and vector storage belong to each
// balloon; effectPosition borrows that balloon's own position array.
// Repeated reserve/append code keeps allocation guards and exception lifetimes:
// reserve -> append endpoint -> copy/zero old words -> exchange -> free old buffer.
struct BalloonCameraMatrices {
    char cameraParameters[0x20];
    float view[16];
    float projection[16];
};
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char balloonEffectStrings[];
}
}

#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CBigBalloon.h"
#include "classes/CMediumBalloon.h"
#include "classes/CSmallBalloon.h"
#include "classes/CBalloonSystem.h"
#include "classes/CFx.h"

#define AsCBigBalloon(p) ((CBigBalloon*)(p))
// Return caller-provided placement storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}

// Shared symbols used by the balloon classes' member functions.
extern "C" {
extern void* g_fx;
extern void* g_soundManager;
extern void* g_resourceManager;
extern int s_liveBalloonCount;
extern char balloonEffectStrings[];
extern char balloonDefaultScaleValues[];
extern const unsigned int bigBalloonInitialFloatValues[];
extern unsigned int kBalloonParameterWords[2];
extern char s_balloonInterfaceVtable[];
extern char s_unhitBalloonQueue[];
extern unsigned int s_spawnedBalloonList[];
extern unsigned int s_balloonQueueStatics[];
extern char __vt__13CSmallBalloon[];
extern char __vt__14CMediumBalloon[];
extern char __vt__11CBigBalloon[];
extern char __vt__14CBalloonSystem[];
extern float kFloat_0_5_Balloon;
extern float kFloat_1_Balloon;
extern const float kFloat_200_805DD8D0;
extern const float kFloat_51_Balloon;
extern const float kFloat_2_balloon;
extern const float kFloat_1_5_Balloon;
extern float kFloat_0_05235988_805DD8C8;
extern float kFloat_0_041887902_Balloon;
extern const float kFloat_6_2831855_Balloon;
extern const float kFloat_1e03_Balloon;
extern const float kFloat_0_Balloon;

void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned long);
void appFreeUni__FPv(void*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void __dla__FPv(void*);
void __dl__FPv(void*);
int rand();
void ResourceManager_ReleaseObject(void*, void*, int);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void BalloonState_QueueModelPreload(void*);
void Balloon_PrepareForDestroy(void*);
void* IBalloon_dtor(void*, int);
void FxAttachment_Init(void*);
void Vec3_CopySourceToDestination(void*, void*);
void PSMTX44Identity(void*);
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void __ct__18NativeBase80201D18Fv(void*);
void __dt__18NativeBase80201D18Fv(void*, int);
void PartyStampHammer_EnsureSharedQueue();
}

// ---- merged from party_balloon_system.cpp ----
namespace ns0 {

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_80069C94;
extern int g_sceneManager;
extern const float kFloat_1_Balloon;
extern const float kFloat_0_Balloon;
extern int g_resourceManager;
extern int s_liveBalloonCount;
extern int g_fx;
extern int g_soundManager;
extern int g_view;
extern const char s_balloonInterfaceVtable[];
extern const char kIdentityMatrix4x4[];
extern const char balloonEffectStrings[];
extern const double kDouble_4503601774854144_LayoutText;
extern const float kFloat_6_2831855_Balloon;
extern const float kFloat_1e03_Balloon;
extern const float kFloat_2_56_Balloon;
extern const float kFloat_0_5_Balloon;
extern const float kFloat_51_Balloon;
extern const float kFloat_2_balloon;
extern const float kFloat_1_5_Balloon;
extern float kFloat_0_05235988_805DD8C8;
extern float kFloat_0_041887902_Balloon;
extern const float kFloat_200_805DD8D0;
extern unsigned int kBalloonParameterWords[2];
extern const char __vt__13CSmallBalloon[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern void* __vt__;
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void __throw(const char*, void*, void*);
void __dt__Q23std12length_errorFv();
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void FxAttachment_Init(void*);
void Vec3_CopySourceToDestination(void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void PSMTX44Identity(void*);
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void Memory_Clear(void*, int);
void sprintf(void*, const char*, ...);
void ResourceManager_ReleaseObject(void*, void*, int);
void Engine_GetViewport(void*);
void Camera_Apply(void*, float, float);
void* Camera_GetRenderState(void*);
void Vec3_ProjectToViewport(float*, void*, void*, void*, void*, const void*);
void DrawScene_SetNamedObjectPuppet(void*, const void*, int);
void ResourcePreloadQueue_AddRequest(void*, int, void*, float, int, float, const void*, int, int, int);
void WPuppet_SetOpacityRecursive(void*, unsigned char);
int rand();
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, const void*);
float Magnitude__7WVectorCFv(void*);
float fabsf(float);
double fabs(double);
float Math_SinApprox(float);
void Mtx44_SetScale(void*, float, float, float);
void Mtx44_SetRotationY(void*, float);
void Mtx44_SetTranslation(void*, float, float, float);
void* PSMTX44Concat(void*, void*, void*);
void Matrix_ExtractAffineFrom4x4(void*, void*);
void Model_SetMatrixRecursive(void*, int, void*);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void Balloon_StartHammerEffect(void*);

// Initialize balloon animation/collision state and load the ball or red-character model.
void* Balloon_InitState(char* obj, void* spawnPosition, int baseScale, int flag) {
    // Phase 1: initialize empty owned storage and copy the borrowed world origin.
    *reinterpret_cast<void**>(obj) = const_cast<char*>(s_balloonInterfaceVtable);
    AsBalloonState80069C(obj)->scaleValues = 0;
    (*(void**)&AsBalloonState80069C(obj)->scaleCount) = 0;
    (*(void**)&AsBalloonState80069C(obj)->scaleCapacity) = 0;
    FxAttachment_Init(obj + kBalloonAttachmentOffset);
    AsBalloonState80069C(obj)->forceSphereCollision = 0;
    (*(int*)&AsBalloonState80069C(obj)->model) = 0;
    (*(short*)&AsBalloonState80069C(obj)->baseScale) = baseScale;
    AsBalloonState80069C(obj)->currentIndex = -1;
    AsBalloonState80069C(obj)->active = 1;
    AsBalloonState80069C(obj)->randomizedSound = flag;
    AsBalloonState80069C(obj)->targetScale = -1;
    AsBalloonState80069C(obj)->scaleDelta = 0;
    Vec3_CopySourceToDestination(spawnPosition, obj + kBalloonOriginOffset);
    AsBalloonState80069C(obj)->scale = static_cast<float>(AsBalloonState80069C(obj)->baseScale);
    AsBalloonState80069C(obj)->opacity = 0xFF;
    (*(WVector*)&AsBalloonState80069C(obj)->position) = (*(WVector*)&AsBalloonState80069C(obj)->origin);
    AsBalloonState80069C(obj)->bobPhase =
        kFloat_6_2831855_Balloon * (static_cast<float>(rand() % kRandomPhaseSteps) / kFloat_1e03_Balloon);
    AsBalloonState80069C(obj)->heading =
        kFloat_6_2831855_Balloon * (static_cast<float>(rand() % kRandomPhaseSteps) / kFloat_1e03_Balloon);
    PSMTX44Identity(obj + kBalloonTransformOffset);
    // Phase 2: acquire the selected model and reset the attachment's borrowed links.
    AsBalloonState80069C(obj)->model =
        ResourceManager_LoadByExtension(reinterpret_cast<void*>(g_resourceManager),
                                        flag ? ((BalloonEffectStrings*)balloonEffectStrings)->redCharacterModel
                                             : ((BalloonEffectStrings*)balloonEffectStrings)->ballModel,
                                        0, 1, 0);
    Memory_Clear(obj + kBalloonAnimationStepsOffset, 0x14);
    AsBalloonState80069C(obj)->attachmentTransform0 = 0;
    AsBalloonState80069C(obj)->attachmentScreenX = kFloat_0_Balloon;
    AsBalloonState80069C(obj)->attachmentScreenY = kFloat_0_Balloon;
    (*(int*)&AsBalloonState80069C(obj)->effectPosition) = 0;
    AsBalloonState80069C(obj)->attachmentTransform1 = 0;
    AsBalloonState80069C(obj)->attachmentPosition1 = 0;
    AsBalloonState80069C(obj)->attachmentTransform2 = 0;
    AsBalloonState80069C(obj)->attachmentPosition2 = 0;
    AsBalloonState80069C(obj)->attachmentTransform3 = 0;
    AsBalloonState80069C(obj)->attachmentPosition3 = 0;
    s_liveBalloonCount = (s_liveBalloonCount + 1);
    sprintf(obj + 4, ((BalloonEffectStrings*)balloonEffectStrings)->objectNameFormat);
    return obj;
}

struct StringValue {
    void* vtable;
    int refs;
    int length;
    char* text;
};

} // extern "C"
extern "C" {
// These legacy type names survive in exception cleanup symbols. The aliases below
// distinguish scale-word growth storage from shared exception-message ownership.
struct LayoutVectorAllocation {
    char* pointer;
    char* allocator;
    // Own the temporary character array until its shared control block is built.
    LayoutVectorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free the temporary character array if ownership was not transferred.
    ~LayoutVectorAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct LayoutVectorControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize both shared character ownership counts to one.
    LayoutVectorControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct LayoutVectorControl : LayoutVectorControlBase {
    char* pointer;
    // Attach the character buffer to its shared deletion control block.
    LayoutVectorControl(char* p) {
        pointer = p;
    }
};
struct LayoutVectorErrorBase {
    void* table;
    // Install the reconstructed logic-error base vtable.
    LayoutVectorErrorBase() {
        table = __vt__Q23std11logic_error;
    }
    // Destroy the reconstructed exception base without freeing this storage.
    ~LayoutVectorErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
typedef LayoutVectorAllocation BalloonErrorMessageGuard;
typedef LayoutVectorControl BalloonErrorMessageOwner;
namespace std {
struct length_error : LayoutVectorErrorBase {
    char* text;
    void* control;
    // Construct the vector length-error text and its shared character owner.
    length_error() {
        const char* message = ((BalloonEffectStrings*)balloonEffectStrings)->vectorLengthError;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            BalloonErrorMessageGuard allocation(p, &allocator.value);
            control = new BalloonErrorMessageOwner(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
struct LayoutVectorBuffer {
    unsigned int* data;
    unsigned size;
    unsigned capacity;
    unsigned* owner;
    volatile unsigned offset;
    // Initialize empty growth storage with a borrowed original-capacity pointer.
    LayoutVectorBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free whichever vector allocation this temporary owns after exchange.
    ~LayoutVectorBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Return the larger growth increment by reference.
inline const unsigned& LayoutVectorMax(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}
typedef LayoutVectorBuffer BalloonScaleGrowthStorage;

struct UIntVector {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
};
struct UIntTempVector {
    unsigned int* data;
    unsigned int size, cap;
    // Initialize an empty temporary scale-word vector.
    UIntTempVector() : data(0), size(0), cap(0) {}
    // Clear the temporary count and free its owned scale-word storage.
    ~UIntTempVector() {
        if (data) {
            unsigned int n = size;
            size -= n;
            appFreeUni__FPv(data);
        }
    }
};
struct UIntTempVectorZ {
    unsigned int* data;
    unsigned int size, cap;
    // Initialize temporary vector fields from the caller-supplied zero word.
    UIntTempVectorZ(unsigned int z) : data((unsigned int*)z), size(z), cap(z) {}
    // Clear the temporary count and free its owned scale-word storage.
    ~UIntTempVectorZ() {
        if (data) {
            unsigned int n = size;
            size -= n;
            appFreeUni__FPv(data);
        }
    }
};
struct FiveFloats {
    float opacityStep, scaleStep, bobAmplitude, bobPhaseStep, headingStep;
    // Leave animation-step fields for the caller to fill.
    FiveFloats() {}
    // Construct the five animation-step values in their stored field order.
    FiveFloats(float a_, float b_, float c_, float d_, float e_)
        : opacityStep(a_), scaleStep(b_), bobAmplitude(c_), bobPhaseStep(d_), headingStep(e_) {}
};
// Build the five default opacity, scale, bob, phase, and heading increments.
inline FiveFloats MakeFiveFloats() {
    FiveFloats result;
    result.opacityStep = kFloat_51_Balloon;
    result.scaleStep = kFloat_2_balloon;
    result.bobAmplitude = kFloat_1_5_Balloon;
    result.bobPhaseStep = kFloat_0_05235988_805DD8C8;
    result.headingStep = kFloat_0_041887902_Balloon;
    return result;
}
// Store the five balloon animation increments without changing their order.
inline void SetFiveFloats(char* obj, FiveFloats v) {
    AsBalloonState80069C(obj)->opacityStep = v.opacityStep;
    AsBalloonState80069C(obj)->scaleStep = v.scaleStep;
    AsBalloonState80069C(obj)->bobAmplitude = v.bobAmplitude;
    AsBalloonState80069C(obj)->bobPhaseStep = v.bobPhaseStep;
    AsBalloonState80069C(obj)->headingStep = v.headingStep;
}

// Reset the scale sequence for a hammer hit, then attach its effect and play its sound.
void Balloon_StartHammerEffect(void* self) {
    // Hit recipe, in order: target radius 7; delta = (7-currentRadius)/2,
    // truncated to short; replace float endpoints with {30,20}; restore the five
    // animation steps; attach Stamp_hammer.seq at this balloon; play the hit sound.
    // Endpoints are radii in world units, not durations. Appending below copies
    // their bit patterns from kBalloonParameterWords (see its data definition).
    // Phase 1: clear endpoints and set the short-valued radius ramp.
    char* obj = reinterpret_cast<char*>(self);
    UIntVector* scaleValues = reinterpret_cast<UIntVector*>(&AsBalloonState80069C(obj)->scaleValues);
    unsigned int oldSize = scaleValues->size;
    short targetRadius = kHammerTargetScale;
    AsBalloonState80069C(obj)->targetScale = targetRadius;
    scaleValues->size = oldSize - scaleValues->size;
    AsBalloonState80069C(obj)->scaleDelta =
        (short)((targetRadius - AsBalloonState80069C(obj)->scale) * kFloat_0_5_Balloon);
    if (scaleValues->capacity < kSmallScaleEndpointCount) {
        UIntTempVector temp;
        temp.data = (unsigned int*)appMallocDummy__FUl(8);
        temp.cap = kSmallScaleEndpointCount;
        unsigned int* first = scaleValues->data;
        int copiedWordCount =
            (int)(reinterpret_cast<char*>(scaleValues->data + scaleValues->size) - reinterpret_cast<char*>(first)) / 4;
        memcpy(temp.data + temp.size, first, copiedWordCount * 4);
        memset(first, 0, copiedWordCount * 4);
        temp.size += copiedWordCount;
        unsigned int* p = scaleValues->data;
        unsigned int oldCapacity = scaleValues->capacity;
        scaleValues->size = 0;
        scaleValues->capacity = temp.cap;
        temp.cap = oldCapacity;
        scaleValues->data = temp.data;
        temp.data = p;
        scaleValues->size = temp.size;
        temp.size = 0;
    }
    // Phase 2: append radius 30, then radius 20; the expanded growth path
    // owns only the endpoint allocation and preserves the length-error ABI.
    unsigned int* defaultScaleWords = kBalloonParameterWords;
    for (int i = 0; i < kSmallScaleEndpointCount; ++i) {
        unsigned int size = scaleValues->size;
        if (size < scaleValues->capacity) {
            unsigned int next = size + 1;
            scaleValues->size = next;
            scaleValues->data[next - 1] = *defaultScaleWords;
        } else {
            if (kBalloonVectorMaxWords - scaleValues->capacity < 1) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(std::length_error)];
                } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned int needed = scaleValues->size + 1 - scaleValues->capacity;
            BalloonScaleGrowthStorage temp(&scaleValues->capacity);
            unsigned int capacity = scaleValues->capacity;
            if (needed > kBalloonVectorMaxWords - capacity) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(std::length_error)];
                } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            if (capacity < kBalloonVectorGrowthLimit) {
                capacity += LayoutVectorMax((3 * (capacity + 1)) / 5, needed);
            } else if (capacity < kBalloonVectorHalfGrowthLimit) {
                capacity += LayoutVectorMax((capacity + 1) / 2, needed);
            } else
                capacity = kBalloonVectorMaxWords;
            if (capacity > kBalloonVectorMaxWords) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(std::length_error)];
                } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(capacity * 4);
            unsigned int appendIndex = scaleValues->size;
            temp.offset = appendIndex;
            (data + appendIndex)[temp.size] = *defaultScaleWords;
            unsigned int* oldData = scaleValues->data;
            int count = (int)(reinterpret_cast<char*>(scaleValues->data + scaleValues->size) -
                              reinterpret_cast<char*>(oldData)) /
                        4;
            temp.data = data;
            temp.capacity = capacity;
            temp.size = temp.size + 1;
            unsigned int offset = appendIndex - count;
            temp.offset = offset;
            memcpy(data + offset, oldData, count * 4);
            memset(oldData, 0, count * 4);
            unsigned int newSize = temp.size + count;
            oldData = scaleValues->data;
            data = temp.data;
            unsigned int oldCapacity = scaleValues->capacity;
            temp.data = oldData;
            scaleValues->size = 0;
            scaleValues->capacity = temp.capacity;
            temp.capacity = oldCapacity;
            scaleValues->data = data;
            scaleValues->size = newSize;
            temp.size = 0;
        }
        ++defaultScaleWords;
    }
    // Phase 3: restore per-update animation rates, then attach and sound the hit.
    SetFiveFloats(obj, MakeFiveFloats());
    AsBalloonState80069C(obj)->effectPosition = AsBalloonState80069C(obj)->position;
    ((CFx*)(reinterpret_cast<void*>(g_fx)))
        ->CreateAttachedEffect((Vec2_8010*)(&AsBalloonState80069C(obj)->attachmentScreenX),
                               ((BalloonEffectStrings*)balloonEffectStrings)->hammerEffect, 1, 0, 0);
    Sound_PlayByName(reinterpret_cast<void*>(g_soundManager),
                     ((BalloonEffectStrings*)balloonEffectStrings)->hammerSound, 0, 0,
                     ((BalloonEffectStrings*)balloonEffectStrings)->hammerSound, 0, kFloat_0_5_Balloon,
                     kFloat_200_805DD8D0, kFloat_1_Balloon);
}

} // extern "C"

} // namespace ns0

// The scale sequence is a vector of raw float words. These aliases name the
// vector helpers that the balloon sizes share.
typedef ns0::UIntVector BalloonScaleVector;
typedef ns0::UIntTempVectorZ BalloonScaleTemp;
typedef ns0::LayoutVectorBuffer BalloonScaleGrowth;
typedef ns0::std::length_error BalloonLengthError;
using ns0::LayoutVectorMax;
using ns0::MakeFiveFloats;
using ns0::SetFiveFloats;

// Create a small balloon with its initial scale sequence and default animation
// increments. It is named rather than a constructor because the constructor
// form compiles differently.
CSmallBalloon* CSmallBalloon::Init(void* spawnPosition, int characterModel) {
    // Base radius 10, then the two kBalloonParameterWords endpoints.
    unsigned int* defaultScaleWords;
    int i;
    ns0::Balloon_InitState((char*)this, spawnPosition, kSmallBalloonSize, characterModel);
    try {
        BalloonScaleVector* scaleSequence = reinterpret_cast<BalloonScaleVector*>(&scaleValues);
        unsigned int oldSize = scaleSequence->size;
        vtbl = __vt__13CSmallBalloon;
        scaleSequence->size = oldSize - scaleSequence->size;
        if (scaleSequence->capacity < kSmallScaleEndpointCount) {
            unsigned int zero = 0;
            BalloonScaleTemp temp(zero);
            temp.data = (unsigned int*)appMallocDummy__FUl(8);
            temp.cap = kSmallScaleEndpointCount;
            unsigned int* first = scaleSequence->data;
            int copiedWordCount = (int)(reinterpret_cast<char*>(scaleSequence->data + scaleSequence->size) -
                                        reinterpret_cast<char*>(first)) /
                                  4;
            memcpy(temp.data + zero, first, copiedWordCount * 4);
            memset(first, 0, copiedWordCount * 4);
            temp.size += copiedWordCount;
            unsigned int* p = scaleSequence->data;
            unsigned int oldCapacity = scaleSequence->capacity;
            scaleSequence->size = 0;
            scaleSequence->capacity = temp.cap;
            temp.cap = oldCapacity;
            scaleSequence->data = temp.data;
            temp.data = p;
            scaleSequence->size = temp.size;
            temp.size = 0;
        }
        defaultScaleWords = kBalloonParameterWords;
        for (i = 0; i < kSmallScaleEndpointCount; ++i) {
            unsigned int size = scaleSequence->size;
            if (size < scaleSequence->capacity) {
                unsigned int next = size + 1;
                scaleSequence->size = next;
                scaleSequence->data[next - 1] = *defaultScaleWords;
            } else {
                if (kBalloonVectorMaxWords - scaleSequence->capacity < 1) {
                    union {
                        double alignment;
                        unsigned char bytes[sizeof(BalloonLengthError)];
                    } storage;
                    BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(
                        ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                unsigned int needed = scaleSequence->size + 1 - scaleSequence->capacity;
                BalloonScaleGrowth temp(&scaleSequence->capacity);
                unsigned int capacity = scaleSequence->capacity;
                if (needed > kBalloonVectorMaxWords - capacity) {
                    union {
                        double alignment;
                        unsigned char bytes[sizeof(BalloonLengthError)];
                    } storage;
                    BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(
                        ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                if (capacity < kBalloonVectorGrowthLimit) {
                    capacity += LayoutVectorMax((3 * (capacity + 1)) / 5, needed);
                } else if (capacity < kBalloonVectorHalfGrowthLimit) {
                    capacity += LayoutVectorMax((capacity + 1) / 2, needed);
                } else
                    capacity = kBalloonVectorMaxWords;
                if (capacity > kBalloonVectorMaxWords) {
                    union {
                        double alignment;
                        unsigned char bytes[sizeof(BalloonLengthError)];
                    } storage;
                    BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(
                        ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                unsigned int* data = (unsigned int*)appMallocDummy__FUl(capacity * 4);
                unsigned int appendIndex = scaleSequence->size;
                temp.offset = appendIndex;
                (data + appendIndex)[temp.size] = *defaultScaleWords;
                unsigned int* oldData = scaleSequence->data;
                int count = (int)(reinterpret_cast<char*>(scaleSequence->data + scaleSequence->size) -
                                  reinterpret_cast<char*>(oldData)) /
                            4;
                temp.data = data;
                temp.capacity = capacity;
                temp.size = temp.size + 1;
                unsigned int offset = appendIndex - count;
                temp.offset = offset;
                memcpy(data + offset, oldData, count * 4);
                memset(oldData, 0, count * 4);
                unsigned int newSize = temp.size + count;
                oldData = scaleSequence->data;
                data = temp.data;
                unsigned int oldCapacity = scaleSequence->capacity;
                temp.data = oldData;
                scaleSequence->size = 0;
                scaleSequence->capacity = temp.capacity;
                temp.capacity = oldCapacity;
                scaleSequence->data = data;
                scaleSequence->size = newSize;
                temp.size = 0;
            }
            ++defaultScaleWords;
        }
        // Install the per-update rates only after the endpoint vector is ready.
        SetFiveFloats((char*)this, MakeFiveFloats());
        sizeClass = 1;
        return this;
    } catch (...) {
        IBalloon_dtor(this, -1);
        throw;
    }
}

namespace ns0 {
extern "C" {

// Release the balloon model and scale buffer, decrement the live count, and optionally delete the object.
// Repeated subobject guards retain the original inlined vector-destruction branches.
void* IBalloon_dtor(void* self, int deallocate) {
    if (self != 0) {
        *reinterpret_cast<void**>(self) = const_cast<char*>(s_balloonInterfaceVtable);
        ResourceManager_ReleaseObject(reinterpret_cast<void*>(g_resourceManager), AsBalloonState80069C(self)->model, 0);
        --s_liveBalloonCount;
        if (&AsBalloonState80069C(self)->scaleValues != 0) {
            if (&AsBalloonState80069C(self)->scaleValues != 0) {
                if (&AsBalloonState80069C(self)->scaleValues != 0) {
                    if (&AsBalloonState80069C(self)->scaleValues != 0) {
                        if (&AsBalloonState80069C(self)->scaleValues != 0) {
                            void* resource = *reinterpret_cast<void**>(&AsBalloonState80069C(self)->scaleValues);
                            if (resource != 0) {
                                volatile int* pc =
                                    reinterpret_cast<volatile int*>(&AsBalloonState80069C(self)->scaleCount);
                                int v = *pc;
                                *pc = v - v;
                                appFreeUni__FPv(resource);
                            }
                        }
                    }
                }
            }
        }
        if (deallocate > 0) {
            __dl__FPv(self);
        }
    }
    return (void*)self;
}

} // extern "C"

} // namespace ns0

// The original tests the scale vector's address before each nested subobject
// release; the address is non-null and the checks remain in the code.
static inline int SmallBalloonScaleStorageAddress(CSmallBalloon* self) {
    return (int)self + (int)&((CSmallBalloon*)0)->scaleValues;
}

// Release the small balloon's model and scale storage. The original tests the
// receiver again before the body, so the nested check stays.
CSmallBalloon::~CSmallBalloon() {
    if (this) {
        vtbl = (void*)s_balloonInterfaceVtable;
        ResourceManager_ReleaseObject(g_resourceManager, model, 0);
        --s_liveBalloonCount;
        if (SmallBalloonScaleStorageAddress(this) && SmallBalloonScaleStorageAddress(this) &&
            SmallBalloonScaleStorageAddress(this) && SmallBalloonScaleStorageAddress(this) &&
            SmallBalloonScaleStorageAddress(this)) {
            void* resource = scaleValues;
            if (resource != 0) {
                // The original reads the count word back through an aliasing view
                // before zeroing it, which keeps the subtract in the code.
                scaleCount -= *(unsigned int*)&scaleCount;
                appFreeUni__FPv(resource);
            }
        }
    }
}

namespace ns0 {
extern "C" {

// Project a balloon's world position and clamp screen coordinates to the viewport.
void Balloon_ProjectAndClampScreenPosition(void* self, void* source) {
    char cameraData[0x18];
    Engine_GetViewport(cameraData);
    Camera_Apply(reinterpret_cast<void*>(g_view), kFloat_0_Balloon, kFloat_0_Balloon);
    void* camera = Camera_GetRenderState(reinterpret_cast<void*>(g_view));
    float bounds[3];
    Vec3_ProjectToViewport(bounds, AsBalloonState80069C(source)->position, cameraData,
                           ((BalloonCameraMatrices*)camera)->projection, ((BalloonCameraMatrices*)camera)->view,
                           kIdentityMatrix4x4);
    if (bounds[0] < kFloat_0_Balloon)
        bounds[0] = kFloat_0_Balloon;
    float limit0 = AsWView(g_view)->viewportWidth;
    if (bounds[0] > limit0)
        bounds[0] = limit0;
    if (bounds[1] < kFloat_0_Balloon)
        bounds[1] = kFloat_0_Balloon;
    float limit1 = AsWView(g_view)->viewportHeight;
    if (bounds[1] > limit1)
        bounds[1] = limit1;
    int tmp1;
    int tmp0 = *reinterpret_cast<int*>(bounds);
    bounds[2] = kFloat_0_Balloon;
    tmp1 = *reinterpret_cast<int*>(reinterpret_cast<char*>(bounds) + 4);
    *reinterpret_cast<int*>(self) = tmp0;
    *reinterpret_cast<int*>(reinterpret_cast<char*>(self) + 4) = tmp1;
    int tmp2 = *reinterpret_cast<int*>(reinterpret_cast<char*>(bounds) + 8);
    *reinterpret_cast<int*>(reinterpret_cast<char*>(self) + 8) = tmp2;
}

struct BalloonScaleBuffer {
    void* resource;
    int count;
};

// Clear/free scale-buffer storage and optionally delete the buffer object.
void* dtor_80069EA0(BalloonScaleBuffer* self, int deallocate) {
    return ::std::__vector_deleting_dtor((Elem_80069EA0Vector*)self, deallocate);
}

// Return the balloon's mutable world-position vector.
void* Balloon_GetPosition(BalloonState80069C* self) {
    return AsBalloonState80069C(self)->position;
}

// Return the balloon's current scale/collision radius.
float Balloon_GetScale(BalloonState80069C* self) {
    return self->scale;
}

// Return the active word using the original pointer-sized ABI.
void* Balloon_IsActive(BalloonState80069C* self) {
    return (void*)self->active;
}

// Return the current scale-sequence index; negative means the idle animation.
short Balloon_GetCurrentScaleIndex(void* self) {
    return AsBalloonState80069C(self)->currentIndex;
}

// Choose one of twelve sound slots for character balloons, otherwise return the disabled sentinel.
int Balloon_GetRandomSlot(void* self) {
    if (AsBalloonState80069C(self)->randomizedSound != 0) {
        int value = rand();
        return value % kBalloonSoundSlots;
    } else {
        return -1;
    }
}

// Test sphere overlap against the balloon radius, unless collision is forced.
// value is the other sphere's radius; arg borrows its world-space center. Tangency
// returns false (strict distance < summed radii). No active/opacity check occurs.
int Balloon_TestSphereCollision(void* self, float value, const void* arg) {
    if (AsBalloonState80069C(self)->forceSphereCollision != 0) {
        return 1;
    }
    WVector storage;
    __mi__FRC7WVectorRC7WVector(&storage, AsBalloonState80069C(self)->position, arg);
    float distance = storage.Magnitude();
    float limit = value + AsBalloonState80069C(self)->scale;
    float ad = (float)__fabs((double)distance);
    return ad < limit;
}

// Initialize the base balloon helper's two-pi cache.
void BalloonBase_InitTwoPiCache() {
    s_twoPi_80069C94 = kFloat_6_2831855;
}

// Detach the balloon's named puppet from the draw scene before destruction.
void Balloon_PrepareForDestroy(const void* self) {
    if (g_sceneManager != 0) {
        DrawScene_SetNamedObjectPuppet(reinterpret_cast<void*>(g_sceneManager), AsBalloonState80069C(self)->name, 0);
    }
}

// Enqueue the balloon model as a named scene preload.
void BalloonState_QueueModelPreload(void* self) {
    ResourcePreloadQueue_AddRequest(reinterpret_cast<void*>(g_sceneManager), 4, AsBalloonState80069C(self)->model,
                                    kFloat_1_Balloon, 0, kFloat_0_Balloon, AsBalloonState80069C(self)->name, 0, 0, 0);
}

// Set visibility subject to scale-sequence state and apply opacity recursively to the model.
// value is a boolean visibility request, not an alpha level: idle balloons map
// nonzero to 255; hit-sequence balloons preserve fading alpha unless hidden.
void Balloon_SetModelVisibility(void* self, int value) {
    short scaleSequenceIndex = AsBalloonState80069C(self)->currentIndex;
    if (scaleSequenceIndex < 0) {
        int flag = (-value | value) >> 31;
        AsBalloonState80069C(self)->opacity = flag & 0xFF;
    } else if (value == 0) {
        AsBalloonState80069C(self)->opacity = 0;
    }
    void* target = AsBalloonState80069C(self)->model;
    short selected = AsBalloonState80069C(self)->opacity;
    WPuppet_SetOpacityRecursive(target, (unsigned char)selected);
}

struct SetIndexInterface {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void setIndex();
};

// Clamp the selected scale-sequence index, clear scale interpolation, and run the selection hook.
// scaleCount must be nonzero. value first narrows to a signed 16-bit index, then
// clamps to [0,scaleCount-1]; the selection hook also attaches the explosion effect.
void Balloon_SetCurrentIndex(void* self, int value) {
    short sv = (short)value;
    AsBalloonState80069C(self)->currentIndex = (short)value;
    if (sv < 0) {
        AsBalloonState80069C(self)->currentIndex = 0;
    } else {
        unsigned int count = AsBalloonState80069C(self)->scaleCount;
        if ((unsigned int)sv >= count) {
            AsBalloonState80069C(self)->currentIndex = (short)(count - 1);
        }
    }
    AsBalloonState80069C(self)->targetScale = -1;
    AsBalloonState80069C(self)->scaleDelta = 0;
    reinterpret_cast<SetIndexInterface*>(self)->setIndex();
}

struct UpdateInterface8006A294 {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void update();
};

// Advance scale/fade or idle bob/heading, update the model transform, and stop fully faded balloons.
void Balloon_UpdateAnimation(void* self) {
    char* obj = reinterpret_cast<char*>(self);
    if (AsBalloonState80069C(obj)->scaleCount != 0) {
        reinterpret_cast<UpdateInterface8006A294*>(self)->update();

        int delta;
        // Phase 1: apply the optional short-valued target-radius ramp.
        int targetRadius = AsBalloonState80069C(obj)->targetScale;
        if (targetRadius >= 0) {
            delta = AsBalloonState80069C(obj)->scaleDelta;
            float position = AsBalloonState80069C(obj)->scale + (float)delta;
            AsBalloonState80069C(obj)->scale = position;
            int done = 0;
            if (delta > 0 && (float)targetRadius <= position) {
                done = 1;
            } else if (delta < 0) {
                if ((float)AsBalloonState80069C(obj)->targetScale >= AsBalloonState80069C(obj)->scale)
                    done = 1;
            }
            if (done) {
                AsBalloonState80069C(obj)->scale = (float)AsBalloonState80069C(obj)->targetScale;
                AsBalloonState80069C(obj)->targetScale = -1;
            }
        }

        // Phase 2: select an endpoint, retire a fully faded balloon, otherwise
        // update idle bob/heading or grow/fade the selected hit sequence.
        short index = AsBalloonState80069C(obj)->currentIndex;
        float endpoint;
        if (index < 0)
            endpoint = *reinterpret_cast<float*>((*(float**)&AsBalloonState80069C(obj)->scaleValues));
        else if ((unsigned int)index >= (*(volatile unsigned int*)&AsBalloonState80069C(obj)->scaleCount))
            endpoint = reinterpret_cast<float*>(
                (*(float**)&AsBalloonState80069C(obj)->scaleValues))[AsBalloonState80069C(obj)->scaleCount - 1];
        else
            endpoint = reinterpret_cast<float*>((*(float**)&AsBalloonState80069C(obj)->scaleValues))[index];

        if (AsBalloonState80069C(obj)->scale >= endpoint && AsBalloonState80069C(obj)->opacity <= 0) {
            AsBalloonState80069C(obj)->active = 0;
            return;
        } else if (index < 0) {
            (*(WVector*)&AsBalloonState80069C(obj)->position) = (*(WVector*)&AsBalloonState80069C(obj)->origin);
            float angle = AsBalloonState80069C(obj)->bobPhase + AsBalloonState80069C(obj)->bobPhaseStep;
            AsBalloonState80069C(obj)->bobPhase = angle;
            if (angle >= kFloat_6_2831855_Balloon)
                AsBalloonState80069C(obj)->bobPhase = kFloat_0_Balloon;
            float y;
            float sine = Math_SinApprox(AsBalloonState80069C(obj)->bobPhase);
            float currentY;
            float scaleY = AsBalloonState80069C(obj)->bobAmplitude;
            currentY = AsBalloonState80069C(obj)->position[1];
            y = AsBalloonState80069C(obj)->heading + AsBalloonState80069C(obj)->headingStep;
            AsBalloonState80069C(obj)->heading = y;
            AsBalloonState80069C(obj)->position[1] = scaleY * sine + currentY;
            if (y >= kFloat_6_2831855_Balloon)
                AsBalloonState80069C(obj)->heading = kFloat_0_Balloon;
        } else {
            short opacity = AsBalloonState80069C(obj)->opacity;
            int remain = (int)((float)opacity - AsBalloonState80069C(obj)->opacityStep);
            AsBalloonState80069C(obj)->opacity = remain;
            if ((short)remain < 0)
                AsBalloonState80069C(obj)->opacity = 0;
            float progress = AsBalloonState80069C(obj)->scale + AsBalloonState80069C(obj)->scaleStep;
            AsBalloonState80069C(obj)->scale = progress;
            if (progress >= endpoint)
                AsBalloonState80069C(obj)->scale = endpoint;
        }
        // Phase 3: construct scale, Y rotation and world translation, concatenate
        // in the existing helper argument order, then extract the affine model
        // matrix. Do not infer conventional matrix multiplication from call names.
        char matrix[0xF0];
        Mtx44_SetScale(matrix + 0xB0, AsBalloonState80069C(obj)->scale / kFloat_2_56_Balloon,
                       AsBalloonState80069C(obj)->scale / kFloat_2_56_Balloon,
                       AsBalloonState80069C(obj)->scale / kFloat_2_56_Balloon);
        Mtx44_SetRotationY(matrix + 0x70, AsBalloonState80069C(obj)->heading);
        Mtx44_SetTranslation(matrix + 0x30, (*(float*)&AsBalloonState80069C(obj)->position),
                             AsBalloonState80069C(obj)->position[1], AsBalloonState80069C(obj)->position[2]);
        void* combined = PSMTX44Concat(obj + kBalloonTransformOffset, matrix + 0x70, matrix + 0x30);
        PSMTX44Concat(obj + kBalloonTransformOffset, matrix + 0xB0, combined);
        Matrix_ExtractAffineFrom4x4(obj + kBalloonTransformOffset, matrix);
        Model_SetMatrixRecursive(AsBalloonState80069C(obj)->model, 0, matrix);
        WPuppet_SetOpacityRecursive(AsBalloonState80069C(obj)->model,
                                    (unsigned char)AsBalloonState80069C(obj)->opacity);
    }
}

} // extern "C"

} // namespace ns0

// ---- merged from party_balloon_system.cpp ----
namespace ns1 {

extern "C" {

// Small balloons have no extra per-frame update hook.
void CSmallBalloon::vfunc_3() {}
// Medium balloons have no extra per-frame update hook.
void CMediumBalloon::vfunc_3() {}

extern char __vt__14CMediumBalloon[];
extern char balloonDefaultScaleValues[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
void Balloon_InitState(void*, void*, int, void*);
void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void appFreeUni__FPv(void*);
extern void __throw(const char*, void*, void*);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern "C" unsigned int strlen(const char*);
extern "C" char* strcpy(char*, const char*);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void* memset(void*, int, unsigned int);
extern void __dt__Q23std12length_errorFv();
extern char balloonEffectStrings[];
extern const float kFloat_51_Balloon;
extern const float kFloat_2_balloon;
extern const float kFloat_1_5_Balloon;
extern float kFloat_0_05235988_805DD8C8;
extern float kFloat_0_041887902_Balloon;

extern void* g_fx;
extern void* g_soundManager;
extern void* g_resourceManager;
extern int s_liveBalloonCount;
extern char s_balloonInterfaceVtable[];
extern char balloonEffectStrings[];
extern float kFloat_0_5_Balloon;
extern const float kFloat_200_805DD8D0;
extern float kFloat_1_Balloon;
extern const double kDouble_4503601774854144_LayoutText;
extern unsigned int kBalloonParameterWords[2];

void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
extern "C" void __dt__Q23std9exceptionFv(void*, int);

void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
void appFreeUni__FPv(void*);

} // extern "C"
extern "C" {

struct LengthVectorAllocation {
    char* pointer;
    char* allocator;
    // Own the temporary character array until its shared control block is built.
    LengthVectorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free the temporary character array if ownership was not transferred.
    ~LengthVectorAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct LengthVectorControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize both shared character ownership counts to one.
    LengthVectorControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct LengthVectorControl : LengthVectorControlBase {
    char* pointer;
    // Attach the character buffer to its shared deletion control block.
    LengthVectorControl(char* p) {
        pointer = p;
    }
};
struct LengthVectorErrorBase {
    void* table;
    // Install the reconstructed logic-error base vtable.
    LengthVectorErrorBase() {
        table = __vt__Q23std11logic_error;
    }
    // Destroy the reconstructed exception base without freeing this storage.
    ~LengthVectorErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : LengthVectorErrorBase {
    char* text;
    void* control;
    // Construct the vector length-error text and its shared character owner.
    length_error() {
        const char* message = ((BalloonEffectStrings*)balloonEffectStrings)->vectorLengthError;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            LengthVectorAllocation allocation(p, &allocator.value);
            control = new LengthVectorControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}

struct LengthVectorBuffer {
    unsigned int* data;
    unsigned size;
    unsigned capacity;
    unsigned* owner;
    volatile unsigned offset;
    // Initialize empty growth storage with a borrowed original-capacity pointer.
    LengthVectorBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free whichever vector allocation this temporary owns after exchange.
    ~LengthVectorBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Return the larger growth increment by reference.
inline const unsigned& LengthVectorMax(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}
struct UIntVector {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
};
struct UIntTempVector {
    unsigned int* data;
    unsigned int size, cap;
    // Initialize an empty temporary scale-word vector.
    UIntTempVector() : data(0), size(0), cap(0) {}
    // Clear the temporary count and free its owned scale-word storage.
    ~UIntTempVector() {
        if (data) {
            size -= size;
            appFreeUni__FPv(data);
        }
    }
};
struct SmallBalloonFloats {
    float opacityStep, scaleStep, bobAmplitude, bobPhaseStep, headingStep;
    // Leave animation-step fields for the caller to fill.
    SmallBalloonFloats() {}
    // Construct the five animation-step values in their stored field order.
    SmallBalloonFloats(float a_, float b_, float c_, float d_, float e_)
        : opacityStep(a_), scaleStep(b_), bobAmplitude(c_), bobPhaseStep(d_), headingStep(e_) {}
};
// Build the default five animation increments for a small/medium balloon.
inline SmallBalloonFloats MakeSmallBalloonFloats() {
    SmallBalloonFloats result;
    result.opacityStep = kFloat_51_Balloon;
    result.scaleStep = kFloat_2_balloon;
    result.bobAmplitude = kFloat_1_5_Balloon;
    result.bobPhaseStep = kFloat_0_05235988_805DD8C8;
    result.headingStep = kFloat_0_041887902_Balloon;
    return result;
}
// Store the five default animation increments.
inline void SetSmallBalloonFloats(char* obj, SmallBalloonFloats v) {
    AsBalloonState80069C(obj)->opacityStep = v.opacityStep;
    AsBalloonState80069C(obj)->scaleStep = v.scaleStep;
    AsBalloonState80069C(obj)->bobAmplitude = v.bobAmplitude;
    AsBalloonState80069C(obj)->bobPhaseStep = v.bobPhaseStep;
    AsBalloonState80069C(obj)->headingStep = v.headingStep;
}

} // extern "C"

} // namespace ns1

typedef ns0::UIntTempVector BalloonScaleTempDefault;
using ns1::MakeSmallBalloonFloats;
using ns1::SetSmallBalloonFloats;

// Restore a small balloon's scale sequence and play its recovery sound.
void CSmallBalloon::RestoreScaleSequence() {
    // Phase 1: reset the target-radius ramp and reserve endpoint storage.
    BalloonScaleVector* scaleSequence = reinterpret_cast<BalloonScaleVector*>(&scaleValues);
    unsigned int oldSize = scaleSequence->size;
    short targetRadius = kSmallBalloonSize;
    targetScale = targetRadius;
    scaleSequence->size = oldSize - scaleSequence->size;
    scaleDelta = (short)((targetRadius - scale) * kFloat_0_5_Balloon);
    if (scaleSequence->capacity < kSmallScaleEndpointCount) {
        BalloonScaleTempDefault temp;
        temp.data = (unsigned int*)appMallocDummy__FUl(8);
        temp.cap = kSmallScaleEndpointCount;
        unsigned int* first = scaleSequence->data;
        int copiedWordCount =
            (int)(reinterpret_cast<char*>(scaleSequence->data + scaleSequence->size) - reinterpret_cast<char*>(first)) /
            4;
        memcpy(temp.data + temp.size, first, copiedWordCount * 4);
        memset(first, 0, copiedWordCount * 4);
        unsigned int* oldData = scaleSequence->data;
        unsigned int* newData = temp.data;
        unsigned int newSize = temp.size + copiedWordCount;
        unsigned int oldCapacity = scaleSequence->capacity;
        unsigned int newCapacity = temp.cap;
        temp.data = oldData;
        scaleSequence->size = 0;
        scaleSequence->capacity = newCapacity;
        temp.cap = oldCapacity;
        scaleSequence->data = newData;
        scaleSequence->size = newSize;
        temp.size = 0;
    }
    // Phase 2: append the default endpoints, growing/copying/exchanging if full.
    unsigned int* defaultScaleWords = kBalloonParameterWords;
    for (int i = 0; i < kSmallScaleEndpointCount; ++i) {
        unsigned int size = scaleSequence->size;
        if (size < scaleSequence->capacity) {
            unsigned int next = size + 1;
            scaleSequence->size = next;
            scaleSequence->data[next - 1] = *defaultScaleWords;
        } else {
            if (kBalloonVectorMaxWords - scaleSequence->capacity < 1) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(BalloonLengthError)];
                } storage;
                BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned int needed = scaleSequence->size + 1 - scaleSequence->capacity;
            BalloonScaleGrowth temp(&scaleSequence->capacity);
            unsigned int capacity = scaleSequence->capacity;
            if (needed > kBalloonVectorMaxWords - capacity) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(BalloonLengthError)];
                } storage;
                BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            if (capacity < kBalloonVectorGrowthLimit) {
                capacity += LayoutVectorMax((3 * (capacity + 1)) / 5, needed);
            } else if (capacity < kBalloonVectorHalfGrowthLimit) {
                capacity += LayoutVectorMax((capacity + 1) / 2, needed);
            } else
                capacity = kBalloonVectorMaxWords;
            if (capacity > kBalloonVectorMaxWords) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(BalloonLengthError)];
                } storage;
                BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(capacity * 4);
            unsigned int appendIndex = scaleSequence->size;
            temp.offset = appendIndex;
            (data + appendIndex)[temp.size] = *defaultScaleWords;
            unsigned int* oldData = scaleSequence->data;
            int count = (int)(reinterpret_cast<char*>(scaleSequence->data + scaleSequence->size) -
                              reinterpret_cast<char*>(oldData)) /
                        4;
            temp.data = data;
            temp.capacity = capacity;
            temp.size = temp.size + 1;
            unsigned int offset = appendIndex - count;
            temp.offset = offset;
            memcpy(data + offset, oldData, count * 4);
            memset(oldData, 0, count * 4);
            unsigned int newSize = temp.size + count;
            oldData = scaleSequence->data;
            data = temp.data;
            unsigned int oldCapacity = scaleSequence->capacity;
            temp.data = oldData;
            scaleSequence->size = 0;
            scaleSequence->capacity = temp.capacity;
            temp.capacity = oldCapacity;
            scaleSequence->data = data;
            scaleSequence->size = newSize;
            temp.size = 0;
        }
        ++defaultScaleWords;
    }
    // Phase 3: restore per-update rates and play the recovery sound.
    SetSmallBalloonFloats((char*)this, MakeSmallBalloonFloats());
    Sound_PlayByName(g_soundManager, ((BalloonEffectStrings*)balloonEffectStrings)->recoverSound, 0, 0,
                     ((BalloonEffectStrings*)balloonEffectStrings)->recoverSound, 0, kFloat_0_5_Balloon,
                     kFloat_200_805DD8D0, kFloat_1_Balloon);
}
// Restore a medium balloon's scale sequence and play its recovery sound.
void CMediumBalloon::RestoreScaleSequence() {
    // Phase 1: reset the target-radius ramp and reserve endpoint storage.
    BalloonScaleVector* scaleSequence = reinterpret_cast<BalloonScaleVector*>(&scaleValues);
    unsigned int oldSize = scaleSequence->size;
    short targetRadius = kMediumBalloonSize;
    targetScale = targetRadius;
    scaleSequence->size = oldSize - oldSize;
    scaleDelta = (short)((targetRadius - scale) * kFloat_0_5_Balloon);
    if (scaleSequence->capacity < kMediumScaleEndpointCount) {
        BalloonScaleTempDefault temp;
        temp.data = (unsigned int*)appMallocDummy__FUl(12);
        temp.cap = kMediumScaleEndpointCount;
        unsigned int* first = scaleSequence->data;
        int copiedWordCount =
            (int)(reinterpret_cast<char*>(scaleSequence->data + scaleSequence->size) - reinterpret_cast<char*>(first)) /
            4;
        memcpy(temp.data + temp.size, first, copiedWordCount * 4);
        memset(first, 0, copiedWordCount * 4);
        temp.size += copiedWordCount;
        unsigned int* oldData = scaleSequence->data;
        unsigned int* newData = temp.data;
        temp.data = oldData;
        unsigned int newSize = temp.size + copiedWordCount;
        unsigned int oldCapacity = scaleSequence->capacity;
        unsigned int newCapacity = temp.cap;
        scaleSequence->size = 0;
        scaleSequence->capacity = newCapacity;
        temp.cap = oldCapacity;
        scaleSequence->data = newData;
        scaleSequence->size = newSize;
        temp.size = 0;
    }
    unsigned int* defaultScaleWords = reinterpret_cast<unsigned int*>(balloonDefaultScaleValues);
    // Phase 2: append the default endpoints, growing/copying/exchanging if full.
    for (int i = 0; i < kMediumScaleEndpointCount; ++i) {
        unsigned int size = scaleSequence->size;
        if (size < scaleSequence->capacity) {
            unsigned int next = size + 1;
            scaleSequence->size = next;
            scaleSequence->data[next - 1] = *defaultScaleWords;
        } else {
            if (kBalloonVectorMaxWords - scaleSequence->capacity < 1) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(BalloonLengthError)];
                } storage;
                BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            BalloonScaleGrowth temp(&scaleSequence->capacity);
            unsigned int needed = scaleSequence->size + 1 - scaleSequence->capacity;
            unsigned int capacity = scaleSequence->capacity;
            if (needed > kBalloonVectorMaxWords - capacity) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(BalloonLengthError)];
                } storage;
                BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            if (capacity < kBalloonVectorGrowthLimit) {
                capacity += LayoutVectorMax((3 * (capacity + 1)) / 5, needed);
            } else if (capacity < kBalloonVectorHalfGrowthLimit) {
                capacity += LayoutVectorMax((capacity + 1) / 2, needed);
            } else
                capacity = kBalloonVectorMaxWords;
            if (capacity > kBalloonVectorMaxWords) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(BalloonLengthError)];
                } storage;
                BalloonLengthError* error = (BalloonLengthError*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            unsigned int* data = (unsigned int*)appMallocDummy__FUl(capacity * 4);
            unsigned int appendIndex = scaleSequence->size;
            temp.offset = appendIndex;
            (data + appendIndex)[temp.size] = *defaultScaleWords;
            unsigned int* oldData = scaleSequence->data;
            int count = (int)(reinterpret_cast<char*>(scaleSequence->data + scaleSequence->size) -
                              reinterpret_cast<char*>(oldData)) /
                        4;
            temp.data = data;
            temp.capacity = capacity;
            temp.size = temp.size + 1;
            unsigned int offset = appendIndex - count;
            temp.offset = offset;
            memcpy(data + offset, oldData, count * 4);
            memset(oldData, 0, count * 4);
            unsigned int newSize = temp.size + count;
            oldData = scaleSequence->data;
            data = temp.data;
            unsigned int oldCapacity = scaleSequence->capacity;
            temp.data = oldData;
            scaleSequence->size = 0;
            scaleSequence->capacity = temp.capacity;
            temp.capacity = oldCapacity;
            scaleSequence->data = data;
            scaleSequence->size = newSize;
            temp.size = 0;
        }
        ++defaultScaleWords;
    }
    // Phase 3: restore per-update rates and play the recovery sound.
    SetSmallBalloonFloats((char*)this, MakeSmallBalloonFloats());
    Sound_PlayByName(g_soundManager, ((BalloonEffectStrings*)balloonEffectStrings)->recoverSound, 0, 0,
                     ((BalloonEffectStrings*)balloonEffectStrings)->recoverSound, 0, kFloat_0_5_Balloon,
                     kFloat_200_805DD8D0, kFloat_1_Balloon);
}
namespace ns1 {
extern "C" {
// Retain the compiled legacy medium-balloon scale reset, including its length-error construction.
static void old_8006BE4C(void* self) {
    char* p = (char*)self;
    unsigned int size = (*(volatile unsigned int*)&AsBalloonState80069C(p)->scaleCount);
    unsigned int cap = AsBalloonState80069C(p)->scaleCapacity;
    (*(unsigned short*)&AsBalloonState80069C(p)->targetScale) = 0x11;
    AsBalloonState80069C(p)->scaleCount = size - size;
    AsBalloonState80069C(p)->scaleDelta =
        (short)(((float)(unsigned int)(*(unsigned short*)&AsBalloonState80069C(p)->targetScale) -
                 AsBalloonState80069C(p)->scale) *
                kFloat_0_5_Balloon);
    if (cap < 3) {
        void* old = AsBalloonState80069C(p)->scaleValues;
        unsigned int n = 3;
        unsigned int keep = 0;
        void* fresh = appMallocDummy__FUl(12);
        unsigned int oldSize = (unsigned int)(((char*)old + AsBalloonState80069C(p)->scaleCount) - (char*)old) / 4;
        memcpy((char*)fresh + keep * 4, old, oldSize * 4);
        memset(old, 0, oldSize * 4);
        AsBalloonState80069C(p)->scaleValues = fresh;
        AsBalloonState80069C(p)->scaleCount = keep + oldSize;
        AsBalloonState80069C(p)->scaleCapacity = n;
        if (old) {
            appFreeUni__FPv(old);
        }
    }
    void** entries = (void**)&balloonDefaultScaleValues;
    for (int i = 0; i < kMediumScaleEndpointCount; ++i) {
        size = AsBalloonState80069C(p)->scaleCount;
        cap = AsBalloonState80069C(p)->scaleCapacity;
        unsigned int value = (unsigned int)entries[i];
        if (size < cap) {
            ((unsigned int*)AsBalloonState80069C(p)->scaleValues)[size] = value;
            AsBalloonState80069C(p)->scaleCount = size + 1;
            continue;
        }
        unsigned int newSize = size + 1;
        if (cap >= kBalloonVectorMaxWords) {
            char* msg = (char*)balloonEffectStrings + kBalloonVectorLengthErrorOffset;
            unsigned int len = strlen(msg) + 1;
            void* buf = (void*)__nwa__FUl(len * 2 - len);
            void* obj = __nw__FUl(0x10);
            if (obj != 0) {
                *(int*)((char*)obj + 4) = 1;
                *(int*)((char*)obj + 8) = 1;
                *(void**)obj = &s_sharedCharDeleterVtable;
                *(void**)((char*)obj + 12) = buf;
            }
            strcpy((char*)buf, msg);
            void* exc[3];
            exc[0] = &__vt__Q23std12length_error;
            exc[1] = buf;
            exc[2] = obj;
            __throw((char*)balloonEffectStrings + kBalloonLengthErrorTypeOffset, exc,
                    (void*)__dt__Q23std12length_errorFv);
        }
        unsigned int diff = newSize - cap;
        unsigned int newCap;
        if (diff > kBalloonVectorMaxWords - cap) {
            char* msg = (char*)balloonEffectStrings + kBalloonVectorLengthErrorOffset;
            unsigned int len = strlen(msg) + 1;
            void* buf = (void*)__nwa__FUl(len * 2 - len);
            void* obj = __nw__FUl(0x10);
            if (obj != 0) {
                *(int*)((char*)obj + 4) = 1;
                *(int*)((char*)obj + 8) = 1;
                *(void**)obj = &s_sharedCharDeleterVtable;
                *(void**)((char*)obj + 12) = buf;
            }
            strcpy((char*)buf, msg);
            void* exc[3];
            exc[0] = &__vt__Q23std12length_error;
            exc[1] = buf;
            exc[2] = obj;
            __throw((char*)balloonEffectStrings + kBalloonLengthErrorTypeOffset, exc,
                    (void*)__dt__Q23std12length_errorFv);
        }
        if (cap < kBalloonVectorGrowthLimit) {
            unsigned int growth = (3 * (cap + 1)) / 5;
            newCap = cap + (growth < diff ? diff : growth);
        } else if (cap < kBalloonVectorHalfGrowthLimit) {
            unsigned int growth = (cap + 1) / 2;
            newCap = cap + (growth < diff ? diff : growth);
        } else {
            newCap = kBalloonVectorMaxWords;
        }
        if (newCap > kBalloonVectorMaxWords) {
            char* msg = (char*)balloonEffectStrings + kBalloonVectorLengthErrorOffset;
            unsigned int len = strlen(msg) + 1;
            void* buf = (void*)__nwa__FUl(len * 2 - len);
            void* obj = __nw__FUl(0x10);
            if (obj != 0) {
                *(int*)((char*)obj + 4) = 1;
                *(int*)((char*)obj + 8) = 1;
                *(void**)obj = &s_sharedCharDeleterVtable;
                *(void**)((char*)obj + 12) = buf;
            }
            strcpy((char*)buf, msg);
            void* exc[3];
            exc[0] = &__vt__Q23std12length_error;
            exc[1] = buf;
            exc[2] = obj;
            __throw((char*)balloonEffectStrings + kBalloonLengthErrorTypeOffset, exc,
                    (void*)__dt__Q23std12length_errorFv);
        }
        {
            void* oldbegin = AsBalloonState80069C(p)->scaleValues;
            unsigned int oldSize = (unsigned int)(((char*)oldbegin + size * 4) - (char*)oldbegin) / 4;
            void* fresh = (void*)appMallocDummy__FUl(newCap * 4);
            ((unsigned int*)fresh)[size] = value;
            memcpy(fresh, oldbegin, oldSize * 4);
            memset(oldbegin, 0, oldSize * 4);
            AsBalloonState80069C(p)->scaleValues = fresh;
            AsBalloonState80069C(p)->scaleCount = size + 1;
            AsBalloonState80069C(p)->scaleCapacity = newCap;
            if (oldbegin) {
                appFreeUni__FPv(oldbegin);
            }
        }
    }
    {
        struct FiveF {
            float opacityStep, scaleStep, bobAmplitude, bobPhaseStep, headingStep;
        };
        FiveF v;
        v.opacityStep = kFloat_51_Balloon;
        v.scaleStep = kFloat_2_balloon;
        v.bobAmplitude = kFloat_1_5_Balloon;
        v.bobPhaseStep = kFloat_0_05235988_805DD8C8;
        v.headingStep = kFloat_0_041887902_Balloon;
        (*(FiveF*)&AsBalloonState80069C(p)->opacityStep) = v;
    }
    Sound_PlayByName(g_soundManager, balloonEffectStrings + 0xA7, 0, 0, balloonEffectStrings + 0xA7, 0,
                     kFloat_0_5_Balloon, kFloat_200_805DD8D0, kFloat_1_Balloon);
}
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void ResourceManager_ReleaseObject(void*, void*, int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

} // extern "C"

} // namespace ns1

// Attach the small explosion effect at the balloon position and play its sound.
void CSmallBalloon::StartExplosionEffect() {
    effectPosition = position;
    ((CFx*)g_fx)
        ->CreateAttachedEffect((Vec2_8010*)(&attachmentScreenX),
                               ((BalloonEffectStrings*)balloonEffectStrings)->smallExplosionEffect, 1, 0, 0);
    Sound_PlayByName(g_soundManager, ((BalloonEffectStrings*)balloonEffectStrings)->smallExplosionSound, 0, 0,
                     ((BalloonEffectStrings*)balloonEffectStrings)->smallExplosionSound, 0, kFloat_0_5_Balloon,
                     kFloat_200_805DD8D0, kFloat_1_Balloon);
}

namespace ns1 {
extern "C" {

// Temporary vector-like objects used by CMediumBalloon::Init (layout: begin, size, capacity).
#pragma opt_propagation off
// XVec owns temporary medium-balloon float-word storage; YVec extends it
// with growth metadata. Names are retained for exception relocation identity.
struct XVec {
    void** buf;
    unsigned int size;
    unsigned int cap;
    // Initialize temporary vector metadata from the supplied zero word.
    XVec(unsigned int z) : buf((void**)z), size(z), cap(z) {}
    // Clear the temporary size and release its owned vector allocation.
    ~XVec() {
        if (buf) {
            size = size - size;
            appFreeUni__FPv(buf);
        }
    }
};

struct YVec : XVec {
    unsigned int* owner;
    volatile unsigned int offset;
    // Initialize empty insertion storage and borrow the original-capacity pointer.
    YVec(unsigned int* p) : XVec(0), owner(p), offset(0) {}
};

struct LenErrObj {
    void* vt;
    void* str;
    void* rep;
};

struct PtrGuard {
    void* p;
    void* a;
    // Free character storage whose shared ownership was not transferred.
    ~PtrGuard() {
        if (p != 0) {
            __dla__FPv(p);
        }
    }
};

// Return the larger value by reference without copying it.
template <class T> inline const T& xmax(const T& a, const T& b) {
    return a < b ? b : a;
}

struct FiveF {
    float opacityStep, scaleStep, bobAmplitude, bobPhaseStep, headingStep;
};

// Copy the five adjacent balloon animation rates without changing their aggregate-copy ABI.
inline void setFive(void* dst, FiveF v) {
    *(FiveF*)dst = v;
}

// Construct and throw the legacy medium-vector length error; currently uncalled.
static inline void ThrowLegacyMediumVectorLengthError() {
    char flag = 0;
    LenErrObj e;
    void* sbuf;
    e.vt = (void*)&__vt__Q23std11logic_error;
    const char* msg = ((BalloonEffectStrings*)balloonEffectStrings)->vectorLengthError;
    {
        PtrGuard g;
        unsigned int msglen = strlen(msg) + 1;
        sbuf = (void*)__nwa__FUl(msglen * 2 - msglen);
        e.str = sbuf;
        g.p = sbuf;
        g.a = (void*)&flag;
        void* rep = __nw__FUl(0x10);
        if (rep != 0) {
            ((unsigned int*)rep)[1] = 1;
            ((unsigned int*)rep)[2] = 1;
            ((void**)rep)[0] = (void*)&s_sharedCharDeleterVtable;
            ((void**)rep)[3] = sbuf;
        }
        e.rep = rep;
        g.p = 0;
    }
    strcpy((char*)e.str, msg);
    e.vt = (void*)&__vt__Q23std12length_error;
    __throw(((BalloonEffectStrings*)balloonEffectStrings)->lengthErrorType, &e, (void*)__dt__Q23std12length_errorFv);
}

// Despite void** storage, each element is a raw IEEE-754 float word, never
// a pointee: copy/zero/free the allocation without deleting individual elements.
struct MediumVector {
    char pad[0x24];
    void** data;
    unsigned int size, capacity;
};
// Adopt the temporary scale vector plus appended elements; leave its old allocation for cleanup.
inline void MediumTargetSwap(MediumVector* v, YVec& t, int count) {
    unsigned int capacity;
    unsigned int size;
    void** fresh;
    unsigned int newCapacity;
    void** old;
    old = v->data;
    fresh = t.buf;
    t.buf = old;
    size = t.size + count;
    t.size = size;
    capacity = v->capacity;
    newCapacity = t.cap;
    v->size = 0;
    v->capacity = newCapacity;
    t.cap = capacity;
    v->data = fresh;
    v->size = size;
    t.size = 0;
}
// Exchange two storage metadata values.
template <class T> inline void MediumExchange(T& a, T& b) {
    T value = a;
    a = b;
    b = value;
}
#pragma opt_propagation reset

} // extern "C"

} // namespace ns1

// Choose the medium/small explosion effect and sound from the stored balloon type.
void CMediumBalloon::StartExplosionEffect() {
    const char* text = ((BalloonEffectStrings*)balloonEffectStrings)->mediumExplosionEffect;
    effectPosition = position;
    text =
        (baseScale != kMediumBalloonSize) ? ((BalloonEffectStrings*)balloonEffectStrings)->smallExplosionEffect : text;
    ((CFx*)g_fx)->CreateAttachedEffect((Vec2_8010*)(&attachmentScreenX), text, 1, 0, 0);
    const char* text2 = ((BalloonEffectStrings*)balloonEffectStrings)->mediumExplosionSound;
    if (baseScale != kMediumBalloonSize)
        text2 = ((BalloonEffectStrings*)balloonEffectStrings)->smallExplosionSound;
    Sound_PlayByName(g_soundManager, text2, 0, 0, text2, 0, kFloat_0_5_Balloon, kFloat_200_805DD8D0, kFloat_1_Balloon);
}

// The original tests the scale vector's address before each nested subobject
// release; the address is non-null and the checks remain in the code.
static inline int MediumBalloonScaleStorageAddress(CMediumBalloon* self) {
    return (int)self + (int)&((CMediumBalloon*)0)->scaleValues;
}

// Release the medium balloon's model and scale storage. The original tests the
// receiver again before the body, so the nested check stays.
CMediumBalloon::~CMediumBalloon() {
    if (this) {
        vtbl = (void*)s_balloonInterfaceVtable;
        ResourceManager_ReleaseObject(g_resourceManager, model, 0);
        --s_liveBalloonCount;
        if (MediumBalloonScaleStorageAddress(this) && MediumBalloonScaleStorageAddress(this) &&
            MediumBalloonScaleStorageAddress(this) && MediumBalloonScaleStorageAddress(this) &&
            MediumBalloonScaleStorageAddress(this)) {
            // Endpoint floats are stored as word bits; destroy the allocation, never the endpoint values.
            void** scaleWordAllocation = (*(void***)&scaleValues);
            if (scaleWordAllocation != 0) {
                // The original reads the count word back through an aliasing view
                // before zeroing it, which keeps the subtract in the code.
                scaleCount -= *(unsigned int*)&scaleCount;
                appFreeUni__FPv(scaleWordAllocation);
            }
        }
    }
}

// ---- merged from party_balloon_system.cpp ----
namespace ns2 {

extern "C" {

extern char __vt__11CBigBalloon[];
extern char balloonEffectStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern const unsigned int bigBalloonInitialFloatValues[];
extern float kFloat_51_Balloon, kFloat_2_balloon, kFloat_1_5_Balloon, kFloat_0_05235988_805DD8C8,
    kFloat_0_041887902_Balloon;
void Balloon_InitState(void*, void*, int, int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void* __nwa__FUl(unsigned int);
void appFreeUni__FPv(void*);
void dtor_80046918(void*);
void __dt__Q33std6detail16char_array_guardFv(void*);
void dtor_80046988(void*);
void __dt__Q23std12length_errorFv();
void* appMallocDummy__FUl(int);
void appFreeUni__FPv(void*);

struct C4Vector {
    char pad[0x24];
    void** data;
    unsigned int size, capacity;
};
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
struct C4Storage {
    void** data;
    unsigned int size, capacity;
    // Initialize an empty temporary scale-word allocation.
    C4Storage() : data(0), size(0), capacity(0) {}
    // Clear the element count and free the allocation held by this temporary.
    ~C4Storage() {
        if (data) {
            *(unsigned int*)((char*)this + 4) -= ((unsigned int*)this)[1];
            appFreeUni__FPv(data);
        }
    }
};
struct C4Insert : C4Storage {
    unsigned int* owner;
    volatile unsigned int offset;
    // Borrow the original capacity and initialize the insertion offset.
    C4Insert(unsigned int* p) : owner(p), offset(0) {}
};
struct C4Ref {
    void* table;
    int refs, owners;
    char* text;
    // Initialize shared character ownership and attach its deletion vtable.
    C4Ref(char* p) {
        refs = 1;
        owners = 1;
        table = s_sharedCharDeleterVtable;
        text = p;
    }
};
struct C4Guard {
    char* text;
    void* allocator;
    // Release character storage if no shared owner adopted it.
    ~C4Guard() {
        if (text)
            __dla__FPv(text);
    }
};
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct C4ErrorBase {
    void* table;
    // Install the reconstructed logic-error base vtable.
    C4ErrorBase() {
        table = __vt__Q23std11logic_error;
    }
    // Destroy the reconstructed exception base without freeing this storage.
    ~C4ErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : C4ErrorBase {
    ~length_error();
    char* text;
    void* control;
    // Construct the vector length-error text and its shared character owner.
    length_error() {
        const char* message = ((BalloonEffectStrings*)balloonEffectStrings)->vectorLengthError;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            C4Guard guard = {p, &allocator.value};
            control = new C4Ref(p);
            guard.text = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
} // extern "C"
} // namespace ns2
namespace std {
struct exception {
    void* table;
    // Initialize the reconstructed exception base for this local instantiation.
    exception() {
        table = ns2::__vt__Q23std11logic_error;
    }
    ~exception();
};
namespace detail {
struct CharArrayGuard_u80069C94 {
    char* text;
    void* allocator;
    // Release character storage if no shared owner adopted it.
    ~CharArrayGuard_u80069C94() {
        if (text)
            ns2::__dla__FPv(text);
    }
};
}
struct length_error : exception {
    ~length_error();
    char* text;
    void* control;
    // Construct the vector length-error text and its shared character owner.
    length_error() {
        const char* message = ((BalloonEffectStrings*)ns2::balloonEffectStrings)->vectorLengthError;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)ns2::__nwa__FUl(2 * n - n);
        {
            char* p = text;
            detail::CharArrayGuard_u80069C94 guard = {p, &allocator.value};
            control = new ns2::C4Ref(p);
            guard.text = 0;
        }
        strcpy(text, message);
        table = ns2::__vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
namespace ns1 {
extern "C" {
// Construct and throw the original vector length error using aligned stack storage.
inline void MediumFail() {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(::std::length_error)];
        } storage;
        ::std::length_error* error = (::std::length_error*)storage.bytes;
        error->__ct();
        exception_runtime::__throw(((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType,
                                   error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
} // extern "C"

} // namespace ns1

typedef ns1::MediumVector BalloonVectorView;
typedef ns1::XVec BalloonVectorTemp;
typedef ns1::YVec BalloonVectorGrowth;
using ns1::MediumFail;
using ns1::MediumTargetSwap;

#pragma opt_propagation off
// Construct a medium balloon and reserve/append its default scale values with exception cleanup.
// It is named rather than a constructor because the constructor form compiles differently.
CMediumBalloon* CMediumBalloon::Init(void* spawnPosition, int characterModel) {
    // Recipe: base radius 17, three balloonDefaultScaleValues endpoints, then five default increments.
    // The pointer-shaped endpoint slots are raw float words, not owned object pointers.
    int i;
    BalloonVectorView* scaleSequence;
    unsigned int zero;
    int reserveWordCount;
    unsigned int bytes;
    unsigned int capacity;
    unsigned int growthThreeFifths;
    unsigned int growthHalf;
    unsigned int additionalWords;
    unsigned int grownCapacity;
    void** newWords;
    unsigned int oldSize;
    int oldWordCount;
    unsigned int offset;
    const unsigned int* endpointWords;

    ns0::Balloon_InitState((char*)this, spawnPosition, kMediumBalloonSize, characterModel);
    scaleSequence = (BalloonVectorView*)this;
    unsigned int size;
    size = scaleSequence->size;
    vtbl = __vt__14CMediumBalloon;
    scaleSequence->size = size - scaleSequence->size;
    // Reserve three float words, then append defaults in table order.
    if (scaleSequence->capacity < kMediumScaleEndpointCount) {
        void** old;

        zero = 0;
        BalloonVectorTemp reservedStorage(zero);
        reservedStorage.buf = (void**)appMallocDummy__FUl(12);
        reservedStorage.cap = kMediumScaleEndpointCount;
        old = scaleSequence->data;
        reserveWordCount = ((char*)(old + scaleSequence->size) - (char*)old) / 4;
        bytes = reserveWordCount * 4;
        memcpy(reservedStorage.buf + zero, old, bytes);
        memset(old, 0, bytes);
        reservedStorage.size += reserveWordCount;
        scaleSequence->size = zero;
        ns1::MediumExchange(scaleSequence->capacity, reservedStorage.cap);
        ns1::MediumExchange(scaleSequence->data, reservedStorage.buf);
        ns1::MediumExchange(scaleSequence->size, reservedStorage.size);
    }
    struct Values {
        float opacityStep, scaleStep, bobAmplitude, bobPhaseStep, headingStep;
    };
    struct ValueBits {
        unsigned a, b, c, d, e;
    };
    Values copy;
    endpointWords = (const unsigned int*)balloonDefaultScaleValues;
    for (i = 0; i < kMediumScaleEndpointCount; ++i, ++endpointWords) {
        unsigned int size = scaleSequence->size;
        capacity = scaleSequence->capacity;
        if (size < capacity) {
            ++size;
            scaleSequence->size = size;
            scaleSequence->data[size - 1] = (void*)*endpointWords;
        } else {
            if (kBalloonVectorMaxWords - scaleSequence->capacity < 1)
                MediumFail();
            additionalWords = scaleSequence->size + 1 - scaleSequence->capacity;
            BalloonVectorGrowth temp(&scaleSequence->capacity);
            grownCapacity = scaleSequence->capacity;
            if (additionalWords > kBalloonVectorMaxWords - grownCapacity)
                MediumFail();
            if (grownCapacity < kBalloonVectorGrowthLimit) {
                growthThreeFifths = (grownCapacity + 1) * 3 / 5;
                grownCapacity += ns1::xmax(growthThreeFifths, additionalWords);
            } else if (grownCapacity < kBalloonVectorHalfGrowthLimit) {
                growthHalf = (grownCapacity + 1) / 2;
                grownCapacity += ns1::xmax(growthHalf, additionalWords);
            } else
                grownCapacity = kBalloonVectorMaxWords;
            if (grownCapacity > kBalloonVectorMaxWords)
                MediumFail();
            newWords = (void**)appMallocDummy__FUl(grownCapacity * 4);
            oldSize = scaleSequence->size;
            temp.offset = oldSize;
            (newWords + oldSize)[temp.size] = (void*)*endpointWords;
            void** old = scaleSequence->data;
            oldWordCount = ((char*)(old + scaleSequence->size) - (char*)old) / 4;
            temp.buf = newWords;
            temp.cap = grownCapacity;
            ++temp.size;
            offset = oldSize - oldWordCount;
            temp.offset = offset;
            memcpy(newWords + offset, old, oldWordCount * 4);
            memset(old, 0, oldWordCount * 4);
            MediumTargetSwap(scaleSequence, temp, oldWordCount);
        }
    }
    // Install animation increments and the medium size class.
    float vals[5] = {kFloat_51_Balloon, kFloat_2_balloon, kFloat_1_5_Balloon, kFloat_0_05235988_805DD8C8,
                     kFloat_0_041887902_Balloon};
    *(ValueBits*)&copy = *(ValueBits*)vals;
    *(Values*)&opacityStep = copy;
    sizeClass = 2;
    return this;
}
#pragma opt_propagation reset

namespace ns2 {
extern "C" {
// Throw the large-balloon vector's fixed length error; the supplied message is unused.
inline void C4Fail(const char* message) {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(::std::length_error)];
        } storage;
        ::std::length_error* error = (::std::length_error*)storage.bytes;
        error->__ct();
        exception_runtime::__throw(((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType,
                                   error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
// Return the larger growth increment by reference.
inline const unsigned int& C4Max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Adopt temporary scale storage and transfer the old allocation to its cleanup guard.
inline void C4Swap(C4Vector* v, C4Storage& t) {
    void** p = v->data;
    void** q = t.data;
    t.data = p;
    unsigned int s = t.size;
    unsigned int c = v->capacity;
    v->size = 0;
    v->capacity = t.capacity;
    t.capacity = c;
    v->data = q;
    v->size = s;
    t.size = 0;
}
// Exchange two vector metadata values.
template <class T> inline void C4Exchange(T& a, T& b) {
    T value = a;
    a = b;
    b = value;
}
// Exchange all vector storage fields after clearing the destination's active element count.
inline void C4ExchangeVector(C4Vector* v, C4Storage& t, unsigned int zero) {
    v->size = zero;
    C4Exchange(v->capacity, t.capacity);
    C4Exchange(v->data, t.data);
    C4Exchange(v->size, t.size);
}
// Adopt a grown scale vector while adding the appended element count to its size.
inline void C4TargetSwap(C4Vector* v, C4Storage& t, int count) {
    unsigned int capacity;
    unsigned int size;
    void** fresh;
    unsigned int newCapacity;
    void** old;
    old = v->data;
    fresh = t.data;
    t.data = old;
    size = t.size + count;
    t.size = size;
    capacity = v->capacity;
    newCapacity = t.capacity;
    v->size = 0;
    v->capacity = newCapacity;
    t.capacity = capacity;
    v->data = fresh;
    v->size = size;
    t.size = 0;
}
// Reserve four scale entries, transfer existing words, and clear the relinquished storage.
inline void C4Reserve(C4Vector* v) {
    unsigned int bytes;
    void** old;
    int n;
    unsigned int zero;
    zero = 0;
    C4Storage temp;
    temp.data = (void**)appMallocDummy__FUl(0x10);
    temp.capacity = kLargeScaleEndpointCount;
    old = v->data;
    n = ((char*)(old + v->size) - (char*)old) / 4;
    bytes = n * 4;
    memcpy(temp.data + zero, old, bytes);
    memset(old, 0, bytes);
    temp.size += n;
    C4ExchangeVector(v, temp, zero);
}
} // extern "C"

} // namespace ns2

typedef ns2::C4Vector BalloonWordVector;
typedef ns2::C4Insert BalloonWordInsert;
using ns2::C4Fail;
using ns2::C4Reserve;
using ns2::C4TargetSwap;

#pragma opt_propagation off
// Construct a large balloon and reserve/append its default scale values with exception cleanup.
// It is named rather than a constructor because the constructor form compiles differently.
CBigBalloon* CBigBalloon::Init(void* spawnPosition, int characterModel) {
    // Recipe: base radius 25, four table endpoints, then the five shared animation increments.
    // The void* slots below hold raw float bits; they never own pointed-to objects.
    ns0::Balloon_InitState((char*)this, spawnPosition, kLargeBalloonSize, characterModel);
    BalloonWordVector* scaleWords = (BalloonWordVector*)this;
    unsigned int capacity;
    unsigned int size;
    size = scaleWords->size;
    capacity = scaleWords->capacity;
    vtbl = __vt__11CBigBalloon;
    scaleWords->size = size - scaleWords->size;
    if (capacity < 4) {
        C4Reserve(scaleWords);
    }
    struct Values {
        float opacityStep, scaleStep, bobAmplitude, bobPhaseStep, headingStep;
    };
    struct ValueBits {
        unsigned a, b, c, d, e;
    };
    Values copy;
    const unsigned int* endpointWords = bigBalloonInitialFloatValues;
    // Append the endpoint bit patterns in table order before publishing animation rates.
    for (int i = 0; i < kLargeScaleEndpointCount; ++i, ++endpointWords) {
        unsigned int size = scaleWords->size;
        unsigned int capacity = scaleWords->capacity;
        if (size < capacity) {
            ++size;
            scaleWords->size = size;
            scaleWords->data[size - 1] = (void*)*endpointWords;
        } else {
            unsigned int threeFifthsIncrement;
            unsigned int halfIncrement;
            unsigned int requiredAdditionalWords;
            if (0x3fffffff - scaleWords->capacity < 1)
                C4Fail(((BalloonEffectStrings*)balloonEffectStrings)->vectorLengthError);
            requiredAdditionalWords = scaleWords->size + 1 - scaleWords->capacity;
            BalloonWordInsert growthStorage(&scaleWords->capacity);
            unsigned int newCapacity = scaleWords->capacity;
            if (requiredAdditionalWords > 0x3fffffff - newCapacity)
                C4Fail(((BalloonEffectStrings*)balloonEffectStrings)->vectorLengthError);
            if (newCapacity < 0x15555555) {
                threeFifthsIncrement = (newCapacity + 1) * 3 / 5;
                newCapacity += ns2::C4Max(threeFifthsIncrement, requiredAdditionalWords);
            } else if (newCapacity < 0x2aaaaaaa) {
                halfIncrement = (newCapacity + 1) / 2;
                newCapacity += ns2::C4Max(halfIncrement, requiredAdditionalWords);
            } else
                newCapacity = 0x3fffffff;
            if (newCapacity > 0x3fffffff)
                C4Fail(((BalloonEffectStrings*)balloonEffectStrings)->vectorLengthError);
            void** replacementWords = (void**)appMallocDummy__FUl(newCapacity * 4);
            unsigned int oldSize = scaleWords->size;
            growthStorage.offset = oldSize;
            (replacementWords + oldSize)[growthStorage.size] = (void*)*endpointWords;
            void** old = scaleWords->data;
            int n = ((char*)(old + scaleWords->size) - (char*)old) / 4;
            growthStorage.data = replacementWords;
            growthStorage.capacity = newCapacity;
            ++growthStorage.size;
            unsigned int offset = oldSize - n;
            growthStorage.offset = offset;
            memcpy(replacementWords + offset, old, n * 4);
            memset(old, 0, n * 4);
            C4TargetSwap(scaleWords, growthStorage, n);
        }
    }
    float vals[5] = {kFloat_51_Balloon, kFloat_2_balloon, kFloat_1_5_Balloon, kFloat_0_05235988_805DD8C8,
                     kFloat_0_041887902_Balloon};
    *(ValueBits*)&copy = *(ValueBits*)vals;
    *(Values*)&opacityStep = copy;
    sizeClass = 3;
    return this;
}
#pragma opt_propagation reset

namespace ns2 {
extern "C" {

extern float kFloat_0_5_Balloon, kFloat_1_Balloon, kFloat_200_805DD8D0;
extern void* g_soundManager;
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
// Convert a stored unsigned parameter word numerically to float.
inline float C4U2F(unsigned int v) {
    return (float)v;
}
struct BBigFloats {
    float opacityStep, scaleStep, bobAmplitude, bobPhaseStep, headingStep;
    // Leave animation-step fields for the caller to fill.
    BBigFloats() {}
    // Construct the five animation-step values in their stored field order.
    BBigFloats(float a_, float b_, float c_, float d_, float e_)
        : opacityStep(a_), scaleStep(b_), bobAmplitude(c_), bobPhaseStep(d_), headingStep(e_) {}
};
// Build the five default animation increments for a large balloon.
inline BBigFloats MakeBigFloats() {
    BBigFloats result;
    result.opacityStep = kFloat_51_Balloon;
    result.scaleStep = kFloat_2_balloon;
    result.bobAmplitude = kFloat_1_5_Balloon;
    result.bobPhaseStep = kFloat_0_05235988_805DD8C8;
    result.headingStep = kFloat_0_041887902_Balloon;
    return result;
}
// Store the five large-balloon animation increments.
inline void SetBigFloats(char* obj, BBigFloats v) {
    AsBalloonState80069C(obj)->opacityStep = v.opacityStep;
    AsBalloonState80069C(obj)->scaleStep = v.scaleStep;
    AsBalloonState80069C(obj)->bobAmplitude = v.bobAmplitude;
    AsBalloonState80069C(obj)->bobPhaseStep = v.bobPhaseStep;
    AsBalloonState80069C(obj)->headingStep = v.headingStep;
}
} // extern "C"

} // namespace ns2

using ns2::C4Insert;
using ns2::C4Max;
using ns2::C4Storage;
using ns2::C4Swap;
using ns2::C4Vector;
using ns2::MakeBigFloats;
using ns2::SetBigFloats;

// Restore the large balloon's scale sequence, animation increments, and recovery sound.
void CBigBalloon::ResetScaleSequence() {
    // Phase 1: reset the target-radius ramp and reserve endpoint storage.
    int targetRadius = kLargeBalloonSize;
    scaleCount -= scaleCount;
    targetScale = targetRadius;
    scaleDelta = (unsigned short)((targetRadius - scale) * kFloat_0_5_Balloon);
    C4Vector* scaleWords = (C4Vector*)this;
    if (scaleWords->capacity < kLargeScaleEndpointCount) {
        C4Storage temp;
        temp.data = (void**)appMallocDummy__FUl(0x10);
        temp.capacity = kLargeScaleEndpointCount;
        void** first = scaleWords->data;
        int copiedWordCount = (int)((char*)(scaleWords->data + scaleWords->size) - (char*)first) / 4;
        memcpy(temp.data + temp.size, first, copiedWordCount * 4);
        memset(first, 0, copiedWordCount * 4);
        temp.size += copiedWordCount;
        C4Swap(scaleWords, temp);
    }
    void** defaultScaleWords = (void**)bigBalloonInitialFloatValues;
    // Phase 2: append the default endpoints, growing/copying/exchanging if full.
    for (int i = 0; i < kLargeScaleEndpointCount; ++i) {
        unsigned int size = scaleWords->size;
        if (size < scaleWords->capacity) {
            unsigned int next = size + 1;
            scaleWords->size = next;
            scaleWords->data[next - 1] = *defaultScaleWords;
        } else {
            if (kBalloonVectorMaxWords - scaleWords->capacity < 1) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(::std::length_error)];
                } storage;
                ::std::length_error* error = (::std::length_error*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            C4Insert temp(&scaleWords->capacity);
            unsigned int need = scaleWords->size + 1 - scaleWords->capacity;
            unsigned int capacity = scaleWords->capacity;
            if (need > kBalloonVectorMaxWords - capacity) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(::std::length_error)];
                } storage;
                ::std::length_error* error = (::std::length_error*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            if (capacity < 0x15555555) {
                capacity += C4Max((3 * (capacity + 1)) / 5, need);
            } else if (capacity < 0x2AAAAAAA) {
                capacity += C4Max((capacity + 1) / 2, need);
            } else
                capacity = kBalloonVectorMaxWords;
            if (capacity > kBalloonVectorMaxWords) {
                union {
                    double alignment;
                    unsigned char bytes[sizeof(::std::length_error)];
                } storage;
                ::std::length_error* error = (::std::length_error*)storage.bytes;
                error->__ct();
                exception_runtime::__throw(
                    ((BalloonEffectStrings*)exception_runtime::balloonEffectStrings)->lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
            void** data = (void**)appMallocDummy__FUl(capacity * 4);
            unsigned int appendIndex = scaleWords->size;
            temp.offset = appendIndex;
            (data + appendIndex)[temp.size] = *defaultScaleWords;
            void** oldData = scaleWords->data;
            int count = (int)((char*)(scaleWords->data + scaleWords->size) - (char*)oldData) / 4;
            temp.data = data;
            temp.capacity = capacity;
            temp.size = temp.size + 1;
            unsigned int offset = appendIndex - count;
            temp.offset = offset;
            memcpy(data + offset, oldData, count * 4);
            memset(oldData, 0, count * 4);
            temp.size += count;
            C4Swap(scaleWords, temp);
        }
        ++defaultScaleWords;
    }
    // Phase 3: restore per-update rates and play the recovery sound.
    SetBigFloats((char*)scaleWords, MakeBigFloats());
    Sound_PlayByName(g_soundManager, (char*)((BalloonEffectStrings*)balloonEffectStrings)->recoverSound, 0, 0,
                     (char*)((BalloonEffectStrings*)balloonEffectStrings)->recoverSound, 0, kFloat_0_5_Balloon,
                     kFloat_200_805DD8D0, kFloat_1_Balloon);
}

namespace ns2 {
extern "C" {

extern float kBalloonBallCollisionRadius;
extern const float kFloat_6_2831855;
extern float s_twoPi_Balloon;
extern const float kFloat_5_BalloonCollision;
extern float kFloat_0_5_Balloon;
extern float kFloat_200_805DD8D0;
extern float kFloat_1_Balloon;
extern char s_unhitBalloonQueue[];
extern char balloonEffectStrings[];
extern void* g_fx;
extern void* g_soundManager;
extern void* g_resourceManager;
extern int s_liveBalloonCount;
extern unsigned int s_spawnedBalloonList[];
extern unsigned int s_balloonQueueStatics[];
extern char s_balloonInterfaceVtable[];
extern char __vt__14CBalloonSystem[];
void ResourceManager_ReleaseObject(void*, void*, int);
void appFreeUni__FPv(void*);
void* appMallocDummy__FUl(int);
void BalloonState_QueueModelPreload(void*);
void Balloon_PrepareForDestroy(void*);
void __dl__FPv(void*);
void* __nw__FUl(unsigned int);
void __ct__18NativeBase80201D18Fv(void*);
void __dt__18NativeBase80201D18Fv(void*, int);
void Memory_Clear(void*, int);
void PartyStampHammer_EnsureSharedQueue();

void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);

// Large balloons have no extra per-frame update hook.
void CBigBalloon::vfunc_3() {}

} // extern "C"

} // namespace ns2

// Queue word ABI: list[0]=count; list+1 is the {prev,next} sentinel.
// Real nodes are {prev,next,balloon}. Spawned owns balloons; unhit/hit borrow
// them. Destroy spawned payloads first, then free nodes in all three lists.
// Initialize the balloon system after destroying/resetting its shared balloon queues.
// Named rather than a constructor because the generated constructor does not
// return the object.
#pragma push
#pragma use_lmw_stmw on
CBalloonSystem* CBalloonSystem::Init() {
    // Phase 1: destroy owning-list payloads before freeing owning and borrowing nodes.
    unsigned int* base = s_balloonQueueStatics;
    __ct__18NativeBase80201D18Fv(this);
    vtbl = __vt__14CBalloonSystem;
    balloonsHitThisFrame = 0;
    {
        unsigned int* sentinel = (unsigned int*)((char*)(base + kSpawnedQueueWord)) + 1;
        unsigned int* it = (unsigned int*)sentinel[1];
        for (; sentinel != it; it = (unsigned int*)it[1]) {
            Balloon_PrepareForDestroy((void*)it[2]);
            void* obj = (void*)it[2];
            if (obj) {
                if (obj) {
                    typedef void (*Dtor)(void*, int);
                    Dtor dtor = *(Dtor*)((char*)*(void**)obj + 8);
                    dtor(obj, 1);
                }
                it[2] = 0;
            }
        }
    }
    {
        unsigned int* list = (unsigned int*)((char*)(base + kSpawnedQueueWord));
        unsigned int* sentinel = list + 1;
        unsigned int* cur = (unsigned int*)sentinel[1];
        if (cur != sentinel) {
            unsigned int* tail = (unsigned int*)*sentinel;
            ((unsigned int*)cur[0])[1] = tail[1];
            *(unsigned int*)tail[1] = cur[0];
            while (cur != sentinel) {
                unsigned int* node = cur;
                cur = (unsigned int*)cur[1];
                appFreeUni__FPv(node);
                --list[0];
            }
        }
    }
    {
        unsigned int* list = (unsigned int*)((char*)(base + kUnhitQueueWord));
        unsigned int* sentinel = list + 1;
        unsigned int* cur = (unsigned int*)sentinel[1];
        if (cur != sentinel) {
            unsigned int* tail = (unsigned int*)*sentinel;
            ((unsigned int*)cur[0])[1] = tail[1];
            *(unsigned int*)tail[1] = cur[0];
            while (cur != sentinel) {
                unsigned int* node = cur;
                cur = (unsigned int*)cur[1];
                appFreeUni__FPv(node);
                --list[0];
            }
        }
    }
    {
        unsigned int* list = (unsigned int*)((char*)(base + kHitQueueWord));
        unsigned int* sentinel = list + 1;
        unsigned int* cur = (unsigned int*)sentinel[1];
        if (cur != sentinel) {
            unsigned int* tail = (unsigned int*)*sentinel;
            ((unsigned int*)cur[0])[1] = tail[1];
            *(unsigned int*)tail[1] = cur[0];
            while (cur != sentinel) {
                unsigned int* node = cur;
                cur = (unsigned int*)cur[1];
                appFreeUni__FPv(node);
                --list[0];
            }
        }
    }
    // Phase 2: clear the separate hammer queue after balloon/list cleanup.
    CBalloonSystem::DestroySharedQueue();
    unk_8 = 3;
    Memory_Clear((char*)&lastHitSlotIndex, 0x10);
    lastHitSlotIndex = -1;
    return this;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// A balloon seen through its vtable: slot 2 destroys the object.
struct BVirtualBalloon {
    virtual void destroy(int);
};
#pragma opt_common_subs off
// Destroy queued balloons and list nodes, then the native base. This is the
// original destructor, as a named member because the generated destructor
// reorders the queue base setup and no longer matches.
CBalloonSystem* CBalloonSystem::Destroy(int deleting) {
    // Phase 1: destroy owning-list payloads before freeing owning and borrowing nodes.
    unsigned int* base = s_balloonQueueStatics;
    if (this) {
        vtbl = __vt__14CBalloonSystem;
        {
            unsigned int* list = (unsigned int*)((char*)(base + kSpawnedQueueWord));
            unsigned int* it = (unsigned int*)list[2];
            unsigned int* sentinel = list + 1;
            for (; it != sentinel; it = (unsigned int*)it[1]) {
                Balloon_PrepareForDestroy((void*)it[2]);
                void* obj = (void*)it[2];
                if (obj) {
                    if (obj) {
                        ((BVirtualBalloon*)obj)->destroy(1);
                    }
                    it[2] = 0;
                }
            }
        }
        {
            unsigned int* list = (unsigned int*)((char*)(base + kSpawnedQueueWord));
            unsigned int* sentinel = list + 1;
            unsigned int* cur = (unsigned int*)sentinel[1];
            if (cur != sentinel) {
                unsigned int* tail = (unsigned int*)*sentinel;
                ((unsigned int*)cur[0])[1] = tail[1];
                *(unsigned int*)tail[1] = cur[0];
                while (cur != sentinel) {
                    unsigned int* node = cur;
                    cur = (unsigned int*)cur[1];
                    appFreeUni__FPv(node);
                    --list[0];
                }
            }
        }
        {
            unsigned int* list = (unsigned int*)((char*)(base + kUnhitQueueWord));
            unsigned int* sentinel = list + 1;
            unsigned int* cur = (unsigned int*)sentinel[1];
            if (cur != sentinel) {
                unsigned int* tail = (unsigned int*)*sentinel;
                ((unsigned int*)cur[0])[1] = tail[1];
                *(unsigned int*)tail[1] = cur[0];
                while (cur != sentinel) {
                    unsigned int* node = cur;
                    cur = (unsigned int*)cur[1];
                    appFreeUni__FPv(node);
                    --list[0];
                }
            }
        }
        {
            unsigned int* list = (unsigned int*)((char*)(base + kHitQueueWord));
            unsigned int* sentinel = list + 1;
            unsigned int* cur = (unsigned int*)sentinel[1];
            if (cur != sentinel) {
                unsigned int* tail = (unsigned int*)*sentinel;
                ((unsigned int*)cur[0])[1] = tail[1];
                *(unsigned int*)tail[1] = cur[0];
                while (cur != sentinel) {
                    unsigned int* node = cur;
                    cur = (unsigned int*)cur[1];
                    appFreeUni__FPv(node);
                    --list[0];
                }
            }
        }
        // Phase 2: clear the separate hammer queue after balloon/list cleanup.
        CBalloonSystem::DestroySharedQueue();
        __dt__18NativeBase80201D18Fv(this, 0);
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
}
#pragma pop

namespace ns2 {
extern "C" {

// Restore the default ball radius used for balloon collisions.
void BalloonCollision_ResetBallRadius() {
    kBalloonBallCollisionRadius = kFloat_5_BalloonCollision;
}
// Return the shared queue of balloons still eligible for a hit.
char* Balloon_GetUnhitBalloonQueue() {
    return s_unhitBalloonQueue;
}
// Return the ball collision radius shared with the balloon mode.
float BalloonCollision_GetBallRadius() {
    return kBalloonBallCollisionRadius;
}
// Initialize the balloon-system helper's two-pi cache.
void Balloon_InitTwoPi() {
    s_twoPi_Balloon = kFloat_6_2831855;
}
// Set the ball radius used by balloon collision checks and report success.
int Balloon_SetBallCollisionRadius(float value) {
    kBalloonBallCollisionRadius = value;
    return 1;
}
// The balloon system has no additional preload initialization.
void CBalloonSystem::OnPreLoadInit() {}
// The balloon system has no additional load-stage work.
void CBalloonSystem::OnLoad() {}

} // extern "C"

} // namespace ns2

struct BNode {
    BNode* prev;
    BNode* next;
    void* value;
};
struct BList {
    int size;
    BNode head;
};
// Shared list ABI: count followed by {prev,next} sentinel; value exists only on
// real 12-byte nodes. The spawned list owns balloons; hit/unhit lists are indexes
// borrowing those objects. Cleanup destroys each spawned balloon once, then frees
// nodes from all three lists. Unhit is independently verified by OnProcess moving
// collision candidates from that list to the hit list; its shared name is retained.
struct BSpill {
    void* volatile v20;
    void* volatile v24;
    void* volatile v28;
    void* volatile v2c;
    void* volatile v30;
    void* volatile v34;
    void* volatile v38;
    void* volatile v3c;
};
struct BAlloc {
    char value;
};
struct BTempBase {
    int size;
    BNode head;
    // Unlink and free temporary list nodes without destroying borrowed balloons.
    ~BTempBase() {
        BNode* sentinel = &head;
        BNode* node = head.next;
        if (node != sentinel) {
            BNode* last = sentinel->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            while (node != sentinel) {
                BNode* old = node;
                node = node->next;
                appFreeUni__FPv(old);
                size -= 1;
            }
        }
    }
};
struct BTemp : BTempBase {
    // Copy borrowed balloon pointers into a temporary list of newly allocated nodes.
    BTemp(BNode*& it, BNode* stop, volatile BAlloc& nodeAllocator, volatile BAlloc& valueCopyAllocator,
          volatile BAlloc& temporaryListAllocator, void* volatile& pendingNode, void* volatile& insertionPosition,
          void* volatile& pendingBalloonValue, void* volatile& nodeStorage, void* volatile& allocationResult,
          void*& value) {
        size = 0;
        head.prev = head.next = &head;
        temporaryListAllocator.value = 0;
        for (; it != stop; it = it->next) {
            value = it->value;
            valueCopyAllocator.value = 0;
            BNode* pos = &head;
            pendingBalloonValue = value;
            nodeAllocator.value = 0;
            BNode* node = (BNode*)appMallocDummy__FUl(0xC);
            void** slot = &node->value;
            allocationResult = node;
            nodeStorage = node;
            insertionPosition = pos;
            pendingNode = node;
            if (slot) {
                try {
                    *slot = value;
                } catch (...) {
                    __throw(0, 0, 0);
                }
            }
            BNode* made = (BNode*)pendingNode;
            pendingNode = 0;
            pos->prev->next = made;
            made->prev = pos->prev;
            pos->prev = made;
            made->next = pos;
            ++size;
        }
    }
};
// Borrow the shared unhit-list header; each access still resolves its global address.
static inline BList* UnhitBalloonList() {
    return (BList*)s_unhitBalloonQueue;
}

#pragma push
#pragma use_lmw_stmw on
// Copy spawned balloons into the unhit queue, reusing nodes and splicing or freeing the remainder.
void CBalloonSystem::OnInit() {
    // Queue roles: spawned = source; unhit = destination. Nodes own links, not balloon objects.
    // pendingNode/nodeStorage/allocationResult track allocation until splice commits;
    // copiedValue temporaries preserve the volatile stores on each reuse path.
    // Phase 1: find the source position after as many nodes as the destination owns.
    void* volatile pendingNode;
    void* volatile insertionPosition;
    void* volatile shortSourceCopiedValue;
    void* volatile longSourceCopiedValue;
    void* volatile pendingBalloonValue;
    void* volatile nodeStorage;
    void* volatile allocationResult;
    BNode* volatile retiredNodeCursor;
    volatile BAlloc shortSourceCopyAllocator, longSourceCopyAllocator, temporaryListAllocator, valueCopyAllocator,
        nodeAllocator, scanAllocator;
    BNode* sourceEnd;
    BNode* copyBoundary;
    BNode* sourceTailIterator;
    BNode* sourceCursor;
    void* pendingBalloon;
    sourceEnd = &((BList*)s_spawnedBalloonList)->head;
    sourceCursor = ((BList*)s_spawnedBalloonList)->head.next;
    copyBoundary = sourceCursor;
    scanAllocator.value = 0;
    int count = UnhitBalloonList()->size;
    while (count != 0 && copyBoundary != sourceEnd) {
        copyBoundary = copyBoundary->next;
        --count;
    }
    // Phase 2: if source is longer, copy the shared prefix and splice new tail nodes.
    if (copyBoundary != sourceEnd) {
        sourceTailIterator = copyBoundary;
        BTemp temp(sourceTailIterator, sourceEnd, nodeAllocator, valueCopyAllocator, temporaryListAllocator,
                   pendingNode, insertionPosition, pendingBalloonValue, nodeStorage, allocationResult, pendingBalloon);
        BNode* target = UnhitBalloonList()->head.next;
        while (sourceCursor != copyBoundary) {
            longSourceCopyAllocator.value = 0;
            void* v = sourceCursor->value;
            target->value = v;
            longSourceCopiedValue = v;
            sourceCursor = sourceCursor->next;
            target = target->next;
        }
        if (temp.size != 0) {
            BNode* first = temp.head.next;
            BNode* tail = temp.head.prev;
            temp.head.next = &temp.head;
            temp.head.prev = &temp.head;
            target->prev->next = first;
            first->prev = target->prev;
            target->prev = tail;
            tail->next = target;
            int n = temp.size;
            temp.size = 0;
            UnhitBalloonList()->size += n;
        }
    } else {
        // Phase 3: source fits existing nodes; reuse its prefix and free excess nodes.
        BNode* target = UnhitBalloonList()->head.next;
        while (sourceCursor != copyBoundary) {
            shortSourceCopyAllocator.value = 0;
            void* v = sourceCursor->value;
            target->value = v;
            shortSourceCopiedValue = v;
            sourceCursor = sourceCursor->next;
            target = target->next;
        }
        retiredNodeCursor = target;
        BNode* listHead = &UnhitBalloonList()->head;
        if (target != listHead) {
            BNode* last = listHead->prev;
            target->prev->next = last->next;
            last->next->prev = target->prev;
            BNode* old;
            while ((old = retiredNodeCursor) != listHead) {
                retiredNodeCursor = old->next;
                appFreeUni__FPv(old);
                UnhitBalloonList()->size -= 1;
            }
        }
    }
    PartyStampHammer_EnsureSharedQueue();
}
#pragma pop

#pragma opt_common_subs off

// The original tests the scale vector's address before each nested subobject
// release; the address is non-null and the checks remain in the code.
static inline int BigBalloonScaleStorageAddress(CBigBalloon* self) {
    return (int)self + (int)&((CBigBalloon*)0)->scaleValues;
}

// Release the large balloon's model and scale storage. The original tests the
// receiver again before the body, so the nested check stays.
CBigBalloon::~CBigBalloon() {
    if (this) {
        vtbl = (void*)s_balloonInterfaceVtable;
        ResourceManager_ReleaseObject(g_resourceManager, model, 0);
        --s_liveBalloonCount;
        if (BigBalloonScaleStorageAddress(this) && BigBalloonScaleStorageAddress(this) &&
            BigBalloonScaleStorageAddress(this) && BigBalloonScaleStorageAddress(this) &&
            BigBalloonScaleStorageAddress(this)) {
            if (scaleValues) {
                // The original reads the count word back through an aliasing view
                // before zeroing it, which keeps the subtract in the code.
                scaleCount -= *(unsigned int*)&scaleCount;
                appFreeUni__FPv(scaleValues);
            }
        }
    }
}
#pragma opt_common_subs reset

struct CSmallBalloonNew {
    char unk[0xF0];
    // Construct a small balloon in this allocation using a borrowed spawn origin.
    CSmallBalloonNew(void* o, int f) {
        ((CSmallBalloon*)this)->Init(o, f);
    }
};
struct CMediumBalloonNew {
    char unk[0xF0];
    // Construct a medium balloon in this allocation using a borrowed spawn origin.
    CMediumBalloonNew(void* o, int f) {
        ((CMediumBalloon*)this)->Init(o, f);
    }
};
struct CBigBalloonNew {
    char unk[0xF0];
    // Construct a large balloon in this allocation using a borrowed spawn origin.
    CBigBalloonNew(void* o, int f) {
        ((CBigBalloon*)this)->Init(o, f);
    }
};

struct SpawnNodeGuard {
    void* p;
    // Start with no unlinked spawn-list allocation.
    SpawnNodeGuard() : p(0) {}
    // Free an unlinked node if insertion did not transfer ownership.
    ~SpawnNodeGuard() {
        if (p)
            appFreeUni__FPv(p);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Construct the selected balloon size/model, preload it, append it to the spawned list, and return the count.
// spawnPosition is borrowed and copied by construction. type must be 1..6 and
// allocations must succeed: this path uses balloon immediately without null
// validation. Returns the new spawned-list count, not a balloon pointer.
int CBalloonSystem::Spawn(void* spawnPosition, int type) {
    // Phase 1: choose size/model; character cases deliberately fall through.
    void* base;
    unsigned int* sentinel;
    int useCharacterModel;
    void* balloon;
    balloon = 0;
    useCharacterModel = 0;
    switch (type) {
    case kSmallCharacterBalloon:
        useCharacterModel = 1;
    case kSmallBalloon:
        balloon = new CSmallBalloonNew(spawnPosition, useCharacterModel);
        break;
    case kMediumCharacterBalloon:
        useCharacterModel = 1;
    case kMediumBalloon:
        balloon = new CMediumBalloonNew(spawnPosition, useCharacterModel);
        break;
    case kLargeCharacterBalloon:
        useCharacterModel = 1;
    case kLargeBalloon:
        balloon = new CBigBalloonNew(spawnPosition, useCharacterModel);
        break;
    }
    // Phase 2: enqueue its model, then transfer a new node to the owning list.
    BalloonState_QueueModelPreload(balloon);
    {
        SpawnNodeGuard guard;
        void* volatile v18;
        void* volatile v34;
        void* volatile v10;
        void* volatile v0c;
        base = s_spawnedBalloonList;
        volatile char v8 = 0;
        sentinel = (unsigned int*)base + 1;
        void* node = appMallocDummy__FUl(0xC);
        v10 = node;
        v0c = node;
        v18 = sentinel;
        guard.p = node;
        void** slot = &((BNode*)node)->value;
        if (slot) {
            v34 = base;
            try {
                *slot = balloon;
            } catch (...) {
                __throw(0, 0, 0);
            }
        }
        void* tail = (void*)sentinel[0];
        guard.p = 0;
        ((BNode*)tail)->next = (BNode*)node;
        *(void**)node = (void*)sentinel[0];
        sentinel[0] = (unsigned int)node;
        ((BNode*)node)->next = (BNode*)sentinel;
        ++s_spawnedBalloonList[0];
    }
    return (int)s_spawnedBalloonList[0];
}
#pragma pop

// Detach/destroy queued balloons and clear the three shared lists and hammer queue.
void CBalloonSystem::DestroyAllQueuedBalloons() {
    // Phase 1: detach and destroy payloads in the owning spawned list.
    unsigned int* base = s_balloonQueueStatics;
    unsigned int* it = (unsigned int*)((unsigned int*)((char*)(base + 3)))[2];
    unsigned int* sentinel = ((unsigned int*)((char*)(base + 3)) + 1);
    for (; it != sentinel; it = (unsigned int*)it[1]) {
        Balloon_PrepareForDestroy((void*)it[2]);
        if (it[2]) {
            if (it[2]) {
                void* obj = (void*)it[2];
                typedef void (*Dtor)(void*, int);

                Dtor dtor = *(Dtor*)((char*)*(void**)obj + 8);
                dtor(obj, 1);
            }
            it[2] = 0;
        }
    }
    {
        // Phase 2: free owning-list nodes, then the two borrowing-list node sets.
        unsigned int* list = ((unsigned int*)((char*)(3 + base)));
        unsigned int* s = (list + 1);
        unsigned int* cur = (unsigned int*)s[1];
        if (cur != s) {
            unsigned int* tail = (unsigned int*)*s;
            ((unsigned int*)cur[0])[1] = tail[1];
            *(unsigned int*)tail[1] = cur[0];
            while (cur != s) {
                unsigned int* node = cur;
                cur = (unsigned int*)cur[1];
                appFreeUni__FPv(node);
                --list[0];
            }
        }
    }
    {
        unsigned int* list = ((unsigned int*)((char*)(base + kUnhitQueueWord)));
        unsigned int* s = 1 + list;
        unsigned int* cur = (unsigned int*)s[1];
        if (cur != s) {
            unsigned int* tail = (unsigned int*)*s;
            ((unsigned int*)cur[0])[1] = tail[1];
            *(unsigned int*)tail[1] = cur[0];
            while (cur != s) {
                unsigned int* node = cur;
                cur = (unsigned int*)cur[1];
                appFreeUni__FPv(node);
                --list[0];
            }
        }
    }
    {
        unsigned int* list = ((unsigned int*)((char*)(kHitQueueWord + base)));
        unsigned int* s = (1 + list);
        unsigned int* cur = (unsigned int*)s[1];
        if (cur != s) {
            unsigned int* tail = (unsigned int*)*s;
            ((unsigned int*)cur[0])[1] = tail[1];
            *(unsigned int*)tail[1] = cur[0];
            while (cur != s) {
                unsigned int* node = cur;
                cur = (unsigned int*)cur[1];
                appFreeUni__FPv(node);
                list[0]--;
            }
        }
    }
    CBalloonSystem::DestroySharedQueue();
}

// Choose the large/small explosion effect and sound from the stored balloon type.
void CBigBalloon::StartExplosionEffect() {
    void* first = position;
    effectPosition = first;
    unsigned short baseRadius = baseScale;
    const char* effectName;
    if (baseRadius != kLargeBalloonSize)
        effectName = ((BalloonEffectStrings*)balloonEffectStrings)->smallExplosionEffect;
    else
        effectName = ((BalloonEffectStrings*)balloonEffectStrings)->largeExplosionEffect;
    ((CFx*)g_fx)->CreateAttachedEffect((Vec2_8010*)(&attachmentScreenX), effectName, 1, 0, 0);

    baseRadius = baseScale;
    const char* label = ((BalloonEffectStrings*)balloonEffectStrings)->largeExplosionSound;
    if (baseRadius != kLargeBalloonSize)
        label = ((BalloonEffectStrings*)balloonEffectStrings)->smallExplosionSound;
    Sound_PlayByName(g_soundManager, label, 0, 0, label, 0, kFloat_0_5_Balloon, kFloat_200_805DD8D0, kFloat_1_Balloon);
}

extern "C" {
void BalloonBase_InitTwoPiCache();
void Balloon_InitTwoPi();
}
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FF8", _ctors_u80069C94_0, BalloonBase_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU80461FF8", _ctors_u80069C94_1, Balloon_InitTwoPi);
#pragma pop
