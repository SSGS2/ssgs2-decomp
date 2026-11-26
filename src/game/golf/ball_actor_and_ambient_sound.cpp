// Golf ball actor, ambient sound actor, and the aim path and callback thread helpers.
//
// CBall draws the ball, its ground shadow and its equipment puppet, and runs its effects.
// CAmbientSound plays a course's looping and positional ambient sounds from sorted name trees.

#include <decomp/static_init.h>
#include "MSL/error_message.h"
struct SoundPositionVector;
struct SoundPlaybackScale2;
// The golf ball actor, the course's ambient loop sounds, the shot-path recovery
// helpers and the beach waves.
//
// The ball caches one loaded puppet per equipped ball, draws the puppet and its
// ground shadow, and follows the simulated ball. Ambient box sources are mixed by
// name. The file-local views below name fields of classes that have no header.
// Their type spellings are used by exception-cleanup relocations, so keep them.

struct BallStrings {
    char shadowScript[17];
    char ballObject[7];
    char ballShadowObject[9];
    char petModelFormat[7];
    char centerBone[7];
    char specialEffect[11];
    char specialEffectSound[7];
    char sprayExtension[4];
    char actorType[6];
};
enum BallStringOffsets {
    kBallStringsShadowScriptOffset = (int)&((BallStrings*)0)->shadowScript,
    kBallStringsBallObjectOffset = (int)&((BallStrings*)0)->ballObject,
    kBallStringsBallShadowObjectOffset = (int)&((BallStrings*)0)->ballShadowObject,
    kBallStringsPetModelFormatOffset = (int)&((BallStrings*)0)->petModelFormat,
    kBallStringsCenterBoneOffset = (int)&((BallStrings*)0)->centerBone,
    kBallStringsSpecialEffectOffset = (int)&((BallStrings*)0)->specialEffect,
    kBallStringsSpecialEffectSoundOffset = (int)&((BallStrings*)0)->specialEffectSound,
    kBallStringsSprayExtensionOffset = (int)&((BallStrings*)0)->sprayExtension,
    kBallStringsActorTypeOffset = (int)&((BallStrings*)0)->actorType,
};

// Packed string-pool view; byte extents come from the globals research.
struct ActorStrings {
    char vectorLengthError[20];
    char lengthErrorType[55];
    char padding4B[144]; // 0x4B..0xDA: other strings of the pool that this unit does not reference
    char actorType[7];
    char dummyType[7];
};
enum ActorStringOffsets {
    kActorStringsVectorLengthErrorOffset = (int)&((ActorStrings*)0)->vectorLengthError,
    kActorStringsLengthErrorTypeOffset = (int)&((ActorStrings*)0)->lengthErrorType,
    kActorStringsActorTypeOffset = (int)&((ActorStrings*)0)->actorType,
    kActorStringsDummyTypeOffset = (int)&((ActorStrings*)0)->dummyType,
};
#include "iff/IffStruct.h"
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "classes/CBeach.h"
#define AsCBeach(p) ((CBeach*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CBall.h"
#include "classes/CAnotherHand.h"
#include "classes/CThread.h"
#include "classes/CAmbientSound.h"
#include "classes/CDummy.h"
#include "classes/CQuadTree.h"
#include "classes/CFx.h"

// Layout views for subobjects not yet represented in the shared class headers.
// The common message every actor receives; only the action word is read here.
struct ActorMessage {
    char pad_00[4];
    int type;
};
struct CallbackThreadMutex {
    char ownerState[0x330];
    char mutex[0x18];
};
struct AmbientBoxAxes {
    float center[3];
    float axis[3][3];
};
struct BallBoneTransform {
    char boneState[0x78];
    float attachmentMatrix[12];
};
// Cache payload is separate from its 12-byte circular-list node (prev,next,payload).
// CBall embeds the sentinel at +0x20, first at +0x24, count at +0x1C.
// IFF record is borrowed, puppet is retained through the resource manager, and
// spray/sequence handles refer to FX-managed effects. OnDestroy releases puppet,
// payload, nodes and bitmap; the destructor alone frees list nodes and base only.
struct BallPuppetCacheEntry {
    IFF_STRUCT::sBall* record;
    void* puppet;
    void* sprays[4];
    void* sequences[4];
};
// Circular ball cache: links own their payload; the payload borrows IFF data.
// The embedded sentinel has only previous/next words, so never read its payload.
struct BallCacheLink {
    BallCacheLink* previous;
    BallCacheLink* next;
    BallPuppetCacheEntry* payload;
};
typedef char BallCacheLinkSize[(sizeof(BallCacheLink) == 0xC) ? 1 : -1];
typedef char BallCachePayloadOffset[((int)&((BallCacheLink*)0)->payload == 8) ? 1 : -1];
struct BallShadowCameraMatrices {
    char cameraState[0x20];
    float transform0[16];
    float transform1[16];
};
typedef char CallbackThread_mutex_offset[((int)&((CallbackThreadMutex*)0)->mutex == 0x330) ? 1 : -1];
typedef char BallCacheEntry_size[(sizeof(BallPuppetCacheEntry) == 0x28) ? 1 : -1];
typedef char BallCacheEntry_sequences_offset[((int)&((BallPuppetCacheEntry*)0)->sequences == 0x18) ? 1 : -1];
typedef char BallShadow_transform1_offset[((int)&((BallShadowCameraMatrices*)0)->transform1 == 0x60) ? 1 : -1];

// Each shared-document player slot stores its equipped IFF ball ID at +0x88.
// This view includes the complete slot stride; inventory data and puppets remain borrowed.
struct BallPlayerEquipmentSlot {
    char unknown00[0x88];
    unsigned int ballItemId;
    char unknown8C[0x1D4 - 0x8C];
};
typedef char BallEquipmentSlotSize[(sizeof(BallPlayerEquipmentSlot) == 0x1D4) ? 1 : -1];
typedef char BallEquipmentIdOffset[((int)&((BallPlayerEquipmentSlot*)0)->ballItemId == 0x88) ? 1 : -1];
// Event identity is unresolved; this retained-sample index bounds sink playback's lead-in.
enum BallSinkPlaybackContract {
    kBallSinkAnchorEventSlot = 8
};

enum BallMessageAction {
    kBallShow = 0,
    kBallHide = 1,
    kBallRefreshLighting = 2,
    kBallPrepareSinkPlayback = 3,
    kBallRefreshGroundSink = 4,
    kBallSelectEquipment = 5,
    kBallRefreshEquipment = 6,
    kBallRebuildIdleEffects = 7,
    kBallCreateFlightEffects = 8,
    kBallStoreMessageValue = 9
};
enum BallShadowDraw {
    kBallShadowVertexFormat = 0x110001,
    kBallShadowTriangleStrip = 0x98
};
enum BallStorageConstants {
    kBallEffectKindCount = 7,
    kBallCachedEffectSlots = 4,
    kBallPlayerRecordStride = 0x1D4,
    kBallSinkLeadFrames = 60,
    kBallJitterFrames = 30
};

#define AsCAnotherHand(p) ((CAnotherHand*)(p))

#define AsCBall(p) ((CBall*)(p))

// Declarations the member definitions below need at namespace scope. They are
// the same C functions and data the class bodies use, declared once here so the
// out-of-class definitions can name them directly.
extern "C" {
extern void* s_dummyRttiDescriptor;
extern void dtor_8029EA18(void*, int);
extern void __dla__FPv(void*);
extern void* __nwa__FUl(unsigned int);
extern int OSIsThreadTerminated(void*);
extern unsigned char __vt__12CAnotherHand[];
extern int g_anotherHandThreadStackPointer;
extern unsigned char s_ambientSoundTypeInfo;
extern unsigned char s_ballTypeInfo;
}

// Return caller-provided storage for placement construction.
inline void* operator new(size_t, void* p) {
    return p;
}

struct NativeBase80201D18 {
    char data[0x18];
    NativeBase80201D18();
    ~NativeBase80201D18();
};

// ---- merged from ball_actor_and_ambient_sound.cpp ----
enum AmbientSourceOffsets {
    kAmbientSourceSpatialTailOffset = 0x2C
};

namespace ActorRegistryViews {

extern "C" {

extern void* s_dummyRttiDescriptor;
extern void* s_actorTypeInfo;
extern int __vt__13CAmbientSound[];
extern void* __nw__FUl(unsigned int);
extern void dtor_8029EA18(void*, int);
extern void __dl__FPv(void*);
extern void __ct__18NativeBase80201D18Fv(void*);
extern void __dt__18NativeBase80201D18Fv(void*);
extern void IActor_AddSkipFlags(void*, int);
extern void appFreeUni__FPv(void*);
extern void StringValueTree_DestroySubtree(void*, void*);
extern CSharedDoc* g_sharedDoc;
extern void* g_soundManager;
extern const float kFloat_0_5_805DDCE4;
extern const float kFloat_200_CourseHelpers;
extern double kAmbientSoundDistanceRange;
extern int SoundManager_IsAliasPrefixOrStreamStemActive(void*, void*);
extern void Sound_PlayByName(unsigned char*, const char*, const SoundPositionVector*, unsigned int, const char*,
                             const SoundPlaybackScale2*, float, float, float);
extern void StringMap_DestroyStringPair(void*, void*);
extern void AmbientSoundNameTree_Clear(void*);
extern float CAmbientSound_GetVolume(void*, float);
extern void AmbientSoundNameTree_Find(void*, void*, void*);
extern void* AmbientSoundNameTree_GetOrInsertValue(void*, void*);
extern void AmbientSoundTree_AdvanceIterator(void*);
extern void* SoundNameMap_dtor(void*, int);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
extern void SoundManager_SetNamedVolumeScale(void*, void*, float);
extern unsigned int strlen(const char*);
extern const float kFloat_0_Actor;
extern const float kFloat_6_2831855;
extern float s_twoPi_800CBF34_805E2920;
extern signed char s_taskRegistryInitialized;
extern int s_iObjectTypeInfo;
extern const char actorAndBallErrorStrings[];
extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];
extern const char __vt__Q23std12length_error[];
extern const char s_taskRegistryDtorRegistration[];
extern const char s_actorTaskRegistryKey[7];
extern const char s_dummyActorFactoryKey[7];
extern const char s_actorFactoryKey[7];
extern void* IntPair_Set(void*, const char*, void*);
extern void* ActorFactoryRegistry_GetInstance();
extern void StringMap_SetByName(void*, void*, const char*);
extern void IActor_Create();
extern void CDummy_Create();
extern void TaskRegistry_Dtor();
extern void __register_global_object(void*, void*, void*);
extern unsigned int __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern char* strcpy(char*, const char*);
extern void __throw(void*, void*, void*);
extern void __dt__Q23std12length_errorFv(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);

extern "C" void __dt__Q23std9exceptionFv(void*, int);
// Keep these local type spellings: generated cleanup relocations include their names.
struct fn_800CC390_Allocation {
    char* pointer;
    char* allocator;
    // Guard a newly allocated error-message buffer until ownership transfers.
    fn_800CC390_Allocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Release the message buffer if construction did not transfer ownership.
    ~fn_800CC390_Allocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct fn_800CC390_ControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize reference counts and the shared-character deleter vtable.
    fn_800CC390_ControlBase() {
        strong = 1;
        weak = 1;
        table = (void*)s_sharedCharDeleterVtable;
    }
};
struct fn_800CC390_Control : fn_800CC390_ControlBase {
    char* pointer;
    // Retain the message buffer in its reference-counted control object.
    fn_800CC390_Control(char* p) {
        pointer = p;
    }
};
struct fn_800CC390_ErrorBase {
    void* table;
    // Install the standard logic-error base vtable.
    fn_800CC390_ErrorBase() {
        table = (void*)__vt__Q23std11logic_error;
    }
    // Delegate standard exception-base cleanup to the runtime.
    ~fn_800CC390_ErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : fn_800CC390_ErrorBase {
    ::std::__error_message<fn_800CC390_Allocation, fn_800CC390_Control> message_;
    // Copy the vector error text into a standard length-error payload.
    length_error() {
        const char* message = actorAndBallErrorStrings;
        message_.assign(message);
        table = (void*)__vt__Q23std12length_error;
    }
};
}
// Construct and throw the standard vector-length exception with the original cleanup ABI.
static void fn_800CC390_ThrowLengthError() {
    std::length_error error;
    __throw((void*)(actorAndBallErrorStrings + kActorStringsLengthErrorTypeOffset), &error,
            (void*)__dt__Q23std12length_errorFv);
}

struct fn_800CC390_Buffer {
    const char** data;
    volatile unsigned size;
    unsigned capacity;
    unsigned* owner;
    volatile unsigned offset;
    // Start an empty temporary pointer array for vector growth.
    fn_800CC390_Buffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release the pointer array still held by this growth guard.
    ~fn_800CC390_Buffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Return the larger vector growth increment by reference.
inline const unsigned& fn_800CC390_Max(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}
struct fn_800CC390_Vector {
    const char** volatile data;
    unsigned size;
    unsigned capacity;
};
struct fn_800CC390_Global {
    void* a;
    void* b;
    void* c;
    void* d;
    fn_800CC390_Vector vector;
};
extern fn_800CC390_Global s_taskRegistry;

// Register actor/dummy RTTI and factories, lazily initializing the task registry.
void TaskRegistry_RegisterActorAndDummyTypes() {
    const char* base8220 = actorAndBallErrorStrings;
    s_twoPi_800CBF34_805E2920 = kFloat_6_2831855;
    IntPair_Set(&s_actorTypeInfo, base8220 + kActorStringsActorTypeOffset, &s_iObjectTypeInfo);

    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), (void*)IActor_Create, s_actorFactoryKey);
    IntPair_Set(&s_dummyRttiDescriptor, base8220 + kActorStringsDummyTypeOffset, &s_actorTypeInfo);

    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), (void*)CDummy_Create, s_dummyActorFactoryKey);
    if ((signed char)s_taskRegistryInitialized == 0) {
        s_taskRegistry.a = 0;
        s_taskRegistry.b = 0;
        s_taskRegistry.d = (char*)&s_taskRegistry + 4;
        s_taskRegistry.vector.data = 0;
        s_taskRegistry.vector.size = 0;
        s_taskRegistry.vector.capacity = 0;
        __register_global_object(&s_taskRegistry, (void*)TaskRegistry_Dtor, (void*)s_taskRegistryDtorRegistration);
        s_taskRegistryInitialized = 1;
    }
    // Append a borrowed factory key; on growth, copy the pointer prefix and exchange buffers.
    const char* value = s_actorTaskRegistryKey;
    fn_800CC390_Global* g = &s_taskRegistry;
    unsigned int size = g->vector.size;
    if (size < g->vector.capacity) {
        const char** items = g->vector.data;
        unsigned int next = 1 + size;
        *(volatile unsigned*)&g->vector.size = next;
        items[next - 1] = value;
    } else {
        if (0x3FFFFFFFu - g->vector.capacity < 1)
            fn_800CC390_ThrowLengthError();
        unsigned grow1, grow2;
        unsigned needed = 1 + g->vector.size - g->vector.capacity;
        fn_800CC390_Buffer temp(&g->vector.capacity);
        unsigned capacity = g->vector.capacity;
        if (needed > 0x3FFFFFFFu - capacity)
            fn_800CC390_ThrowLengthError();
        if (capacity < 0x15555555u) {
            grow1 = (3 * (capacity + 1)) / 5;
            capacity += fn_800CC390_Max(grow1, needed);
        } else if (capacity < 0x2AAAAAAAu) {
            grow2 = (capacity + 1) / 2;
            capacity += fn_800CC390_Max(grow2, needed);
        } else
            capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu)
            fn_800CC390_ThrowLengthError();
        const char** data = (const char**)appMallocDummy__FUl(capacity * 4);
        unsigned int oldSize = g->vector.size;
        temp.offset = oldSize;
        temp.data = data;
        (oldSize + data)[temp.size++] = value;
        const char** oldData = g->vector.data;
        int count = ((char*)(oldData + g->vector.size) - (char*)oldData) / 4;
        temp.capacity = capacity;
        unsigned offset = oldSize - count;
        temp.offset = offset;
        int bytes = count * 4;
        memcpy(offset + data, oldData, bytes);
        memset(oldData, 0, bytes);
        unsigned int newSize = temp.size + count;
        const char** swapData = g->vector.data;
        temp.size = newSize;
        const char** newData = temp.data;
        unsigned oldCapacity = g->vector.capacity;
        unsigned newCapacity = temp.capacity;
        g->vector.size = 0;
        g->vector.capacity = newCapacity;
        temp.capacity = oldCapacity;
        g->vector.data = newData;
        temp.data = swapData;
        g->vector.size = newSize;
        temp.size = 0;
    }
}

// Default actor destruction hook; subclasses supply work when needed.
void IActor_OnDestroy() {}
// Default actor drawing hook; subclasses supply work when needed.
void IActor_OnDisplay() {}
// Default actor update hook; subclasses supply work when needed.
void IActor_OnProcess() {}
// Default actor resource-load hook; subclasses supply work when needed.
void IActor_OnLoad() {}
// Default actor preload hook; subclasses supply work when needed.
void IActor_OnPreLoadInit() {}
// Dummy actors intentionally ignore messages.
void CDummy::HandleMsg(void*) {}
// Return the dummy actor's runtime type descriptor.
void** CDummy::GetRTTI() {
    return &s_dummyRttiDescriptor;
}
// Return the base actor's runtime type descriptor.
void** IActor_GetRTTI() {
    return &s_actorTypeInfo;
}

// Destroy the native actor base and optionally release the dummy actor allocation.
CDummy::~CDummy() {
    if (this != 0)
        dtor_8029EA18(this, 0);
}

// Construction view of CAmbientSound; canonical members: classes/CAmbientSound.h.
// 18-byte actor base; shared header only describes the first 8 bytes.
struct CAmbientSoundConstructorView : NativeBase80201D18 {
    // Install the ambient actor vtable and its original skip-mask bits.
    CAmbientSoundConstructorView() {
        *(void**)this = (void*)__vt__13CAmbientSound;
        IActor_AddSkipFlags(this, 0x85040);
    }
};

} // extern "C"

} // namespace ActorRegistryViews

// Allocate and construct the ambient-sound actor.
void* CAmbientSound::Create() {
    using namespace ActorRegistryViews;
    return new CAmbientSoundConstructorView;
}

namespace ActorRegistryViews {

extern "C" {

// Two inline/owned string objects; only the long-string ownership bit permits freeing data.
struct fn_800CC328_First {
    unsigned int keyFlags;
    unsigned int keyLength;
    void* keyData;
    unsigned int valueFlags;
    unsigned int valueLength;
    void* valueData;
};

struct fn_800CC328_Second {
    unsigned int keyFlags;
    unsigned int keyLength;
    unsigned int keyData;
    unsigned int valueFlags;
    unsigned int valueLength;
    void* valueData;
};

struct fn_800CBFB8_Member {
    unsigned int flags;
    unsigned int unk4;
    void* pointer;
};

struct fn_800CBFB8_Pair {
    fn_800CBFB8_Member first;
    fn_800CBFB8_Member second;
};

struct fn_800CBFB8_Node {
    fn_800CBFB8_Node* left;
    fn_800CBFB8_Node* right;
    char pad[4];
    fn_800CBFB8_Pair data;
};

// Destroy an owned string/string subtree in left-right-node order.
// node is required and must not be the sentinel. Payload strings are released
// before each node is freed; forwardedContext is only passed to cleanup calls.
// Three explicit descent levels reproduce the existing unrolled recursion:
// child -> grandchild -> great-grandchild -> recursive remainder. Keeping that
// depth preserves cleanup/call order and the compiler's stack/register shape.
void StringPairMap_DestroySubtree(void* forwardedContext, fn_800CBFB8_Node* node) {
    fn_800CBFB8_Node* greatGrandchild;
    fn_800CBFB8_Node* grandchild;
    fn_800CBFB8_Node* child = node->left;
    if (child) {
        grandchild = child->left;
        if (grandchild) {
            greatGrandchild = grandchild->left;
            if (greatGrandchild) {
                if (greatGrandchild->left)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->left);
                if (greatGrandchild->right)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->right);
                StringMap_DestroyStringPair(forwardedContext, &greatGrandchild->data);
                appFreeUni__FPv(greatGrandchild);
            }
            greatGrandchild = grandchild->right;
            if (greatGrandchild) {
                if (greatGrandchild->left)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->left);
                if (greatGrandchild->right)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->right);
                StringMap_DestroyStringPair(forwardedContext, &greatGrandchild->data);
                appFreeUni__FPv(greatGrandchild);
            }
            StringMap_DestroyStringPair(forwardedContext, &grandchild->data);
            appFreeUni__FPv(grandchild);
        }
        grandchild = child->right;
        if (grandchild) {
            greatGrandchild = grandchild->left;
            if (greatGrandchild) {
                if (greatGrandchild->left)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->left);
                if (greatGrandchild->right)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->right);
                StringMap_DestroyStringPair(forwardedContext, &greatGrandchild->data);
                appFreeUni__FPv(greatGrandchild);
            }
            greatGrandchild = grandchild->right;
            if (greatGrandchild) {
                if (greatGrandchild->left)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->left);
                if (greatGrandchild->right)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->right);
                StringMap_DestroyStringPair(forwardedContext, &greatGrandchild->data);
                appFreeUni__FPv(greatGrandchild);
            }
            StringMap_DestroyStringPair(forwardedContext, &grandchild->data);
            appFreeUni__FPv(grandchild);
        }
        StringMap_DestroyStringPair(forwardedContext, &child->data);
        appFreeUni__FPv(child);
    }
    child = node->right;
    if (child) {
        grandchild = child->left;
        if (grandchild) {
            greatGrandchild = grandchild->left;
            if (greatGrandchild) {
                if (greatGrandchild->left)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->left);
                if (greatGrandchild->right)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->right);
                StringMap_DestroyStringPair(forwardedContext, &greatGrandchild->data);
                appFreeUni__FPv(greatGrandchild);
            }
            greatGrandchild = grandchild->right;
            if (greatGrandchild) {
                if (greatGrandchild->left)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->left);
                if (greatGrandchild->right)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->right);
                StringMap_DestroyStringPair(forwardedContext, &greatGrandchild->data);
                appFreeUni__FPv(greatGrandchild);
            }
            StringMap_DestroyStringPair(forwardedContext, &grandchild->data);
            appFreeUni__FPv(grandchild);
        }
        grandchild = child->right;
        if (grandchild) {
            greatGrandchild = grandchild->left;
            if (greatGrandchild) {
                if (greatGrandchild->left)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->left);
                if (greatGrandchild->right)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->right);
                StringMap_DestroyStringPair(forwardedContext, &greatGrandchild->data);
                appFreeUni__FPv(greatGrandchild);
            }
            greatGrandchild = grandchild->right;
            if (greatGrandchild) {
                if (greatGrandchild->left)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->left);
                if (greatGrandchild->right)
                    StringPairMap_DestroySubtree(forwardedContext, greatGrandchild->right);
                StringMap_DestroyStringPair(forwardedContext, &greatGrandchild->data);
                appFreeUni__FPv(greatGrandchild);
            }
            StringMap_DestroyStringPair(forwardedContext, &grandchild->data);
            appFreeUni__FPv(grandchild);
        }
        StringMap_DestroyStringPair(forwardedContext, &child->data);
        appFreeUni__FPv(child);
    }
    // Keep the original value-before-key cleanup and its interior-address guards.
    if (&node->data != 0) {
        if (&node->data.second != 0) {
            if (node->data.second.flags >> 31)
                appFreeUni__FPv(node->data.second.pointer);
        }
        if (&node->data.first != 0) {
            if (node->data.first.flags >> 31)
                appFreeUni__FPv(node->data.first.pointer);
        }
    }
    appFreeUni__FPv(node);
}

// Start view: nameWord00 is the first word of an inline name;
// taking its address passes characters. repeatInterval is at +0x2C.
struct AmbientStartEntry {
    AmbientStartEntry* prev;
    AmbientStartEntry* next;
    void* nameWord00;
    unsigned char unkC[0x20];
    float repeatInterval;
};
struct AmbientStartList {
    AmbientStartEntry* prev;
    AmbientStartEntry* next;
    unsigned size;
};
struct AmbientStartCourseLists {
    unsigned char pad[0x4904];
    AmbientStartList boxSources;
    AmbientStartList positionalSources;
};
struct AmbientStartCourseOwner {
    unsigned char pad[8];
    AmbientStartCourseLists* context;
};
struct AmbientStartSharedDocument {
    unsigned char pad[0xB90];
    AmbientStartCourseOwner* root;
};
extern float s_actorRenderScaleTween[3];

// Course data owns both circular lists; this actor borrows their names as playback
// aliases. Negative interval means a persistent source, positive interval is timed
// (handled by OnProcess); zero is not started here. boxSources uses box attenuation,
// positionalSources passes a world-position pointer at name +0x2C to the sound system.
// Start eligible repeating ambient sounds in the box and positional source lists.
void CAmbientSound::OnInit() {
    using namespace ActorRegistryViews;
    // The parser numbers function-local cleanup types for the whole translation
    // unit, so the declarations in this function shift the numbers of the ones in
    // OnProcess. These 59 unused declarations hold that numbering where the
    // exception tables expect it. They produce no code or stack and are never read.
    char cleanupIdentity00, cleanupIdentity01, cleanupIdentity02, cleanupIdentity03, cleanupIdentity04,
        cleanupIdentity05, cleanupIdentity06, cleanupIdentity07;
    char cleanupIdentity08, cleanupIdentity09, cleanupIdentity10, cleanupIdentity11, cleanupIdentity12,
        cleanupIdentity13, cleanupIdentity14, cleanupIdentity15;
    char cleanupIdentity16, cleanupIdentity17, cleanupIdentity18, cleanupIdentity19, cleanupIdentity20,
        cleanupIdentity21, cleanupIdentity22, cleanupIdentity23;
    char cleanupIdentity24, cleanupIdentity25, cleanupIdentity26, cleanupIdentity27, cleanupIdentity28,
        cleanupIdentity29, cleanupIdentity30, cleanupIdentity31;
    char cleanupIdentity32, cleanupIdentity33, cleanupIdentity34, cleanupIdentity35, cleanupIdentity36,
        cleanupIdentity37, cleanupIdentity38, cleanupIdentity39;
    char cleanupIdentity40, cleanupIdentity41, cleanupIdentity42, cleanupIdentity43, cleanupIdentity44,
        cleanupIdentity45, cleanupIdentity46, cleanupIdentity47;
    char cleanupIdentity48, cleanupIdentity49, cleanupIdentity50, cleanupIdentity51, cleanupIdentity52,
        cleanupIdentity53, cleanupIdentity54, cleanupIdentity55;
    char cleanupIdentity56, cleanupIdentity57, cleanupIdentity58;
    // The course lists are borrowed from g_sharedDoc.
    // Each list is circular with its header as end, and stays intact during calls.
    {
        float zero;
        void* name;
        AmbientStartEntry* end;
        AmbientStartCourseLists* context;
        context = ((AmbientStartSharedDocument*)g_sharedDoc)->root->context;
        AmbientStartEntry* entry;
        end = (AmbientStartEntry*)&context->boxSources;
        entry = context->boxSources.next;
        zero = kFloat_0_Actor;
        float scale = s_actorRenderScaleTween[1];
        while (entry != end) {
            name = &entry->nameWord00;
            if (entry->repeatInterval < zero && SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, name) == 0)
                ((void (*)(void*, void*, void*, int, void*, double*, float, float, float))Sound_PlayByName)(
                    g_soundManager, name, (void*)0, 1, name, &kAmbientSoundDistanceRange, kFloat_0_5_805DDCE4,
                    kFloat_200_CourseHelpers, scale);
            entry = entry->next;
        }
    }
    {
        AmbientStartEntry* entry;
        void* name;
        AmbientStartCourseLists* context;
        // The positional list's first entry is read before the end sentinel is
        // formed; the list is circular and intact for the whole loop.
        AmbientStartEntry* end;
        context = ((AmbientStartSharedDocument*)g_sharedDoc)->root->context;
        AmbientStartEntry* first = context->positionalSources.next;
        float scale = s_actorRenderScaleTween[1];
        entry = first;
        float zero = kFloat_0_Actor;
        end = (AmbientStartEntry*)&context->positionalSources;
        while (entry != end) {
            name = &entry->nameWord00;
            if (entry->repeatInterval < zero && SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, name) == 0)
                ((void (*)(void*, void*, void*, int, void*, double*, float, float, float))Sound_PlayByName)(
                    g_soundManager, name, (void*)((char*)name + kAmbientSourceSpatialTailOffset), 1, name,
                    &kAmbientSoundDistanceRange, kFloat_0_5_805DDCE4, kFloat_200_CourseHelpers, scale);
            entry = entry->next;
        }
    }
}

// Free the long-string allocations held by a string/string tree entry.
void StringMap_DestroyStringPair(void* unused, void* rawData) {
    fn_800CC328_Second* data;
    fn_800CC328_Second* p;
    data = (fn_800CC328_Second*)rawData;
    if (data == 0)
        return;
    if (&data->valueFlags != 0) {
        if (data->valueFlags >> 31)
            appFreeUni__FPv(data->valueData);
    }
    p = data;
    if (p == 0)
        return;
    if (p->keyFlags >> 31)
        appFreeUni__FPv((void*)p->keyData);
end:; // 0x800CC328: retain the label's effect on compiler-generated local type names.
}

struct fn_800CCE68_Data {
    unsigned int unk0;
    unsigned int unk4;
    unsigned int sentinelNodePtr;
};

// Initialize an empty sound-name tree with its sentinel pointing into the header.
void AmbientSoundNameTree_Init(fn_800CCE68_Data* data) {
    data->unk0 = 0;
    data->unk4 = 0;
    data->sentinelNodePtr = (unsigned int)data + 4;
}

struct dtor_800CCE80_Node {
    dtor_800CCE80_Node* first;
    dtor_800CCE80_Node* second;
    unsigned int parentAndColor;
    unsigned int flags;
    unsigned int keyLength;
    void* owned;
};

// Destroy the two deeper subtrees, then the owned name and this node.
static inline void destroySoundNameLeaf(void* object, dtor_800CCE80_Node* descendant) {
    if (descendant->first)
        StringValueTree_DestroySubtree(object, descendant->first);
    if (descendant->second)
        StringValueTree_DestroySubtree(object, descendant->second);
    if (&descendant->flags != 0) {
        if (&descendant->flags != 0) {
            if ((descendant->flags >> 31) != 0)
                appFreeUni__FPv(descendant->owned);
        }
    }
    appFreeUni__FPv(descendant);
}

// Destroy this child branch in postorder; the caller has checked the node.
static inline void destroySoundNameChildBranch(void* object, dtor_800CCE80_Node*& childBranch) {
    dtor_800CCE80_Node* descendant = childBranch->first;
    if (descendant)
        destroySoundNameLeaf(object, descendant);
    descendant = childBranch->second;
    if (descendant)
        destroySoundNameLeaf(object, descendant);
    if (&childBranch->flags != 0) {
        if (&childBranch->flags != 0) {
            if ((childBranch->flags >> 31) != 0)
                appFreeUni__FPv(childBranch->owned);
        }
    }
    appFreeUni__FPv(childBranch);
}

// Destroy sound-name tree strings/nodes and optionally free the tree object.
// In plain C: for every node of the string-keyed tree (children first, up to four levels unrolled here; deeper
// levels go through StringValueTree_DestroySubtree), free the key's heap text when its flag bit 31 is set, then free the
// node. The address guards preserve the original generated string cleanup.
// Node layout: first/second = child nodes (+0/+4), flags (+0xC) = small-string tag, owned (+0x14) = heap text.
void* SoundNameMap_dtor(void* object, int flags) {
    if (object == 0)
        return object;
    {
        dtor_800CCE80_Node* root = *(dtor_800CCE80_Node**)((char*)object + 4);
        if (root) {
            dtor_800CCE80_Node* rootChild = root->first;
            if (rootChild) {
                dtor_800CCE80_Node* childBranch = rootChild->first;
                if (childBranch) {
                    dtor_800CCE80_Node* descendant = childBranch->first;
                    if (descendant)
                        destroySoundNameLeaf(object, descendant);
                    __typeof__(childBranch->second) secondDescendant = childBranch->second;
                    descendant = secondDescendant;
                    if (descendant)
                        destroySoundNameLeaf(object, descendant);
                    if (&childBranch->flags != 0) {
                        if (&childBranch->flags != 0) {
                            if ((childBranch->flags >> 31) != 0)
                                appFreeUni__FPv(childBranch->owned);
                        }
                    }
                    appFreeUni__FPv(childBranch);
                }
                __typeof__(rootChild->second) secondBranch = rootChild->second;
                childBranch = secondBranch;
                if (childBranch)
                    destroySoundNameChildBranch(object, childBranch);
                if (&rootChild->flags != 0) {
                    if (&rootChild->flags != 0) {
                        if ((rootChild->flags >> 31) != 0)
                            appFreeUni__FPv(rootChild->owned);
                    }
                }
                appFreeUni__FPv(rootChild);
            }
            __typeof__(root->second) secondRootChild = root->second;
            rootChild = secondRootChild;
            if (rootChild) {
                __typeof__(rootChild->first) firstBranch = rootChild->first;
                dtor_800CCE80_Node* childBranch = firstBranch;
                if (childBranch)
                    destroySoundNameChildBranch(object, childBranch);
                childBranch = rootChild->second;
                if (childBranch)
                    destroySoundNameChildBranch(object, childBranch);
                if (&rootChild->flags != 0) {
                    if (&rootChild->flags != 0) {
                        if ((rootChild->flags >> 31) != 0)
                            appFreeUni__FPv(rootChild->owned);
                    }
                }
                appFreeUni__FPv(rootChild);
            }
            if (&root->flags != 0) {
                if (&root->flags != 0) {
                    if ((root->flags >> 31) != 0)
                        appFreeUni__FPv(root->owned);
                }
            }
            appFreeUni__FPv(root);
        }
        if (flags > 0)
            __dl__FPv(object);
    }
    return object;
}

struct AmbientRawNode {
    AmbientRawNode* next;
    unsigned char pad[4];
    unsigned char data[0x30];
};
struct AmbientRawChain {
    unsigned char pad[0x4904];
    AmbientRawNode* first;
    AmbientRawNode* sentinel;
};
struct AmbientRawOwner {
    unsigned char pad[0xB90];
    AmbientRawChain* list;
};
struct AmbientRawDocument {
    unsigned char pad[8];
    AmbientRawOwner* root;
};
// Ambient work map: count/root/first, with &root as end. Nodes are three links,
// a 12-byte owned string key and float attenuation at +0x18. Input lists retain
// their own fixed name[32], timer, interval, fadeDistance, then box/position data.
// A positive interval accumulates delta; a negative interval is persistent.
// delta and interval share the ambient update clock; its seconds scale is unresolved.
struct AmbientNameStorage {
    unsigned int flags, size;
    void* data;
};
struct AmbientAllocatorTag {
    char value;
    char pad[3];
};
// The header stores count/root/first; &root is the end sentinel.
// Node links are left/right followed by tagged parentAndColor.
struct AmbientAttenuationMap {
    unsigned int count;
    unsigned int root;
    void* first;
};
struct AmbientAttenuationNode {
    void* left;
    void* right;
    unsigned int parentAndColor;
    AmbientNameStorage key;
    float value;
};
// Source layout relative to name: +0x20 elapsed clock, +0x24 repeat interval,
// +0x28 fade distance, +0x2C box bounds or positional XYZ in the trailing record.
// AmbientSourceEntry names these elapsedRepeatTime/repeatInterval/fadeDistance;
// AmbientSourceStorage::repeatInterval is the same repeat interval, not a timestamp.
struct AmbientSourceStorage {
    char name[0x20];
    float time;
    float repeatInterval;
    float value;
    float tail;
};
struct AmbientSourceEntry {
    void* prev;
    AmbientSourceEntry* next;
    char name[0x20];
    float elapsedRepeatTime;
    float repeatInterval;
    float fadeDistance;
    unsigned int tail;
};
// Name-relative payload views: intrusive links precede these records by eight bytes.
// The lists share timer fields; their spatial tails are a box or a world-space XYZ.
struct AmbientBoxSourceData {
    char name[0x20];
    float elapsedRepeatTime, repeatInterval, fadeDistance;
    AmbientBoxAxes box;
};
struct AmbientPositionalSourceData {
    char name[0x20];
    float elapsedRepeatTime, repeatInterval, fadeDistance;
    float position[3];
};
typedef char AmbientBoxTailOffset[(int)&((AmbientBoxSourceData*)0)->box == 0x2C ? 1 : -1];
typedef char AmbientPositionTailOffset[(int)&((AmbientPositionalSourceData*)0)->position == 0x2C ? 1 : -1];
struct AmbientSourceList {
    unsigned int prev;
    AmbientSourceEntry* next;
};
struct AmbientCourseLists {
    unsigned char pad[0x4904];
    AmbientSourceList boxSources;
    unsigned int pad2;
    AmbientSourceList positionalSources;
};

#pragma opt_lifetimes off
// Course lists are borrowed. The temporary map owns copied names and combines
// overlapping box sources by MAX attenuation for each sound name, never by sum.
// Positional sources bypass the map and use the global ambient scale directly.
// Advance ambient timers, combine box attenuation by sound name, and update positional volumes.
void CAmbientSound::OnProcess(float delta) {
    using namespace ActorRegistryViews;
    AmbientNameStorage previousValueKey;
    AmbientNameStorage replacementKey;
    AmbientNameStorage newEntryKey;
    AmbientNameStorage lookupKey;
    AmbientAttenuationMap work;
    unsigned int lookupNodeAddress;
    AmbientAttenuationNode* found;
    volatile AmbientAllocatorTag lookupAllocatorTag;
    AmbientAllocatorTag lookupStringAllocator;
    volatile AmbientAllocatorTag insertionAllocatorTag;
    AmbientAllocatorTag insertionStringAllocator;
    volatile AmbientAllocatorTag replacementAllocatorTag;
    AmbientAllocatorTag replacementStringAllocator;
    volatile AmbientAllocatorTag previousValueAllocatorTag;
    AmbientAllocatorTag previousValueStringAllocator;
    unsigned char lookupAllocatorByte;
    AmbientCourseLists* courseLists;
    AmbientSourceEntry* entry;
    AmbientSourceEntry* end;
    unsigned int zeroWord;
    char* sourceName;
    unsigned int len;
    float zero;
    float scale;
    float sourceAttenuation;
    float previousAttenuation;
    void* golfDocument;
    const char* s;
    char* mapEnd;

    golfDocument = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    courseLists = (AmbientCourseLists*)AsCGolfDoc(golfDocument)->courseData;
    ((void (*)(void*, void*))AmbientSoundNameTree_Init)(&work, golfDocument);
    struct WorkCleanup {
        AmbientAttenuationMap* work;
        // Destroy the temporary attenuation map if normal cleanup did not release it.
        ~WorkCleanup() {
            if (work)
                SoundNameMap_dtor(work, -1);
        }
    } workCleanup = {&work};
    struct StringCleanup {
        AmbientNameStorage* string;
        // Release the temporary key text only when it uses heap storage.
        ~StringCleanup() {
            if (string->flags >> 31)
                appFreeUni__FPv(string->data);
        }
    };
    // Phase 1: evaluate box sources and retain the largest attenuation for each name.
    AmbientSoundNameTree_Clear(&work);
    entry = courseLists->boxSources.next;
    end = (AmbientSourceEntry*)&courseLists->boxSources;
    mapEnd = (char*)&work + 4;
    zero = kFloat_0_Actor;
    scale = s_actorRenderScaleTween[1];
    lookupAllocatorByte = lookupAllocatorTag.value;
    zeroWord = 0;
    while (entry != end) {
        sourceName = entry->name;
        if (entry->repeatInterval > zero) {
            ((AmbientBoxSourceData*)sourceName)->elapsedRepeatTime += delta;
            if (((AmbientBoxSourceData*)sourceName)->elapsedRepeatTime >
                ((AmbientBoxSourceData*)sourceName)->repeatInterval) {
                ((AmbientBoxSourceData*)sourceName)->elapsedRepeatTime = zero;
                Sound_PlayByName((unsigned char*)g_soundManager, sourceName, 0, 0, sourceName, 0, kFloat_0_5_805DDCE4,
                                 kFloat_200_CourseHelpers, scale);
            }
            if (SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, (void*)sourceName) != 0) {
                sourceAttenuation = CAmbientSound::GetVolume(&((AmbientBoxSourceData*)sourceName)->box,
                                                             ((AmbientBoxSourceData*)sourceName)->fadeDistance);
            } else {
                sourceAttenuation = kFloat_0_Actor;
            }
        } else {
            sourceAttenuation = CAmbientSound::GetVolume(&((AmbientBoxSourceData*)sourceName)->box,
                                                         ((AmbientBoxSourceData*)sourceName)->fadeDistance);
        }
        {
            lookupKey.flags = zeroWord;
            lookupKey.size = zeroWord;
            lookupKey.data = (void*)zeroWord;
            StringCleanup keyCleanup = {&lookupKey};
            len = ::strlen(sourceName);
            String_ReserveCapacity(&lookupKey, len);
            lookupStringAllocator.value = lookupAllocatorByte;
            String_ReplaceRange(&lookupKey, 0, 0, sourceName, sourceName + len, (const char*)&lookupStringAllocator);
            AmbientSoundNameTree_Find(&lookupNodeAddress, &work, &lookupKey);
            found = (AmbientAttenuationNode*)lookupNodeAddress;
        }
        if (found == (AmbientAttenuationNode*)mapEnd) {
            newEntryKey.flags = zeroWord;
            newEntryKey.size = zeroWord;
            newEntryKey.data = (void*)zeroWord;
            StringCleanup secondCleanup = {&newEntryKey};
            len = ::strlen(sourceName);
            String_ReserveCapacity(&newEntryKey, len);
            insertionStringAllocator.value = insertionAllocatorTag.value;
            String_ReplaceRange(&newEntryKey, 0, 0, sourceName, sourceName + len,
                                (const char*)&insertionStringAllocator);
            *(float*)AmbientSoundNameTree_GetOrInsertValue(&work, &newEntryKey) = sourceAttenuation;
        } else {
            replacementKey.flags = zeroWord;
            replacementKey.size = zeroWord;
            replacementKey.data = (void*)zeroWord;
            StringCleanup secondCleanup = {&replacementKey};
            len = ::strlen(sourceName);
            String_ReserveCapacity(&replacementKey, len);
            replacementStringAllocator.value = replacementAllocatorTag.value;
            String_ReplaceRange(&replacementKey, 0, 0, sourceName, sourceName + len,
                                (const char*)&replacementStringAllocator);
            previousValueKey.flags = zeroWord;
            previousValueKey.size = zeroWord;
            previousValueKey.data = (void*)zeroWord;
            StringCleanup thirdCleanup = {&previousValueKey};
            len = ::strlen(sourceName);
            String_ReserveCapacity(&previousValueKey, len);
            previousValueStringAllocator.value = previousValueAllocatorTag.value;
            String_ReplaceRange(&previousValueKey, 0, 0, sourceName, sourceName + len,
                                (const char*)&previousValueStringAllocator);
            previousAttenuation = *(float*)AmbientSoundNameTree_GetOrInsertValue(&work, &previousValueKey);
            sourceAttenuation = previousAttenuation < sourceAttenuation ? sourceAttenuation : previousAttenuation;
            *(float*)AmbientSoundNameTree_GetOrInsertValue(&work, &replacementKey) = sourceAttenuation;
        }
        entry = entry->next;
    }
    // Phase 2: apply one scaled volume per unique name, then destroy the temporary map.
    {
        AmbientSourceEntry* mapEnd2 = (AmbientSourceEntry*)&work.root;
        found = *(AmbientAttenuationNode**)((char*)&work + 8);
        while ((void*)found != (void*)mapEnd2) {
            if ((found->key.flags >> 31) == 0)
                s = (char*)&found->key.flags + 1;
            else
                s = (const char*)found->key.data;
            SoundManager_SetNamedVolumeScale(g_soundManager, (void*)s, scale * found->value);
            AmbientSoundTree_AdvanceIterator(&found);
        }
    }
    workCleanup.work = 0;
    SoundNameMap_dtor(&work, -1);
    // Phase 3: tick positional sources independently and apply the ambient scale.
    {
        zero = kFloat_0_Actor;
        scale = s_actorRenderScaleTween[1];
        AmbientCourseLists* positionalLists =
            (AmbientCourseLists*)AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData;
        char* positionalName;
        AmbientSourceEntry* end2 = (AmbientSourceEntry*)&positionalLists->positionalSources;
        AmbientSourceEntry* entry2 = positionalLists->positionalSources.next;
        while (entry2 != end2) {
            positionalName = entry2->name;
            if (entry2->repeatInterval > zero) {
                ((AmbientPositionalSourceData*)positionalName)->elapsedRepeatTime += delta;
                if (((AmbientPositionalSourceData*)positionalName)->elapsedRepeatTime >
                    ((AmbientPositionalSourceData*)positionalName)->repeatInterval) {
                    ((AmbientPositionalSourceData*)positionalName)->elapsedRepeatTime = zero;
                    Sound_PlayByName(
                        (unsigned char*)g_soundManager, positionalName,
                        (const SoundPositionVector*)((AmbientPositionalSourceData*)positionalName)->position, 0,
                        positionalName, (const SoundPlaybackScale2*)&kAmbientSoundDistanceRange, kFloat_0_5_805DDCE4,
                        kFloat_200_CourseHelpers, scale);
                }
            }
            SoundManager_SetNamedVolumeScale(g_soundManager, (void*)positionalName, scale);
            entry2 = entry2->next;
        }
    }
}

} // extern "C"

} // namespace ActorRegistryViews

// Physics layout is needed by path and ball helpers below.
#include "game/golfball.h"
// Ambient sources are held in two circular lists: box-volume and positional sounds.
struct AmbientSourceLists {
    char padding[0x4904];
    void* boxSentinel;
    void* firstBox;
    unsigned int boxCount;
    void* positionalSentinel;
    void* firstPositional;
    unsigned int positionalCount;
};
struct AimPathStorage {
    void* points;
    unsigned int count, capacity;
    WVector closestPoint;
};
struct BallMessage {
    unsigned int sender, action, value;
    char padding[0x10];
    unsigned int time;
};
enum ActorHelperConstants {
    kCallbackThreadStackBytes = 0x10000,
    kCallbackThreadPriority = 24,
    kBeachWaveRising = 0,
    kBeachWaveFalling = 1,
    kBeachWaveWaiting = 2
};

// ---- merged from ball_actor_and_ambient_sound.cpp ----
namespace CallbackThreadViews {

struct Vec {
    unsigned int x, y, z;
};

extern "C" {

void OSInitMutex(void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void Memory_Clear(void*, unsigned int);
int OSCreateThread(void*, void*, void*, void*, unsigned int, unsigned int, unsigned int);
void OSResumeThread(void*);
int OBB_ContainsPoint(void*, const char*);
float Magnitude__7WVectorCFv(void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __ml__FfRC7WVector(void*, void*, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
void __as__7WVectorFRC7WVector(void*, void*);
void Normalize__7WVectorFv(void*);
void* GolfBall__Fv();
void* PreData__9CGolfBallFi(void*, int);
void* CQuadTree_GetInstance();
unsigned char GetGroundType__9CQuadTreeF7WVector(void*, Vec*);
int CourseData_IsPointOutsidePlayableRegion(void*, void*);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, Vec*, int, int, int);
extern CSharedDoc* g_sharedDoc;
extern float kFloat_3200_CourseHelpers, kFloat_1_CourseGeometry, kRecoveryPointSearchEpsilon, kFloat_0_CourseHelpers,
    kFloat_32_CourseHelpers, kFloat_192_CourseHelpers;
float __ml__FRC7WVectorRC7WVector(void*, void*);
float Math_SquareRootApprox(float);
double ceil(double);
int OSIsThreadTerminated(void*);
void __dt__18NativeBase80201D18Fv(void*, int);
void* __nwa__FUl(int);
void __dl__FPv(void*);
void IntPair_Set(unsigned char*, const char*, unsigned char*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void OSCancelThread(void*);
void Sound_StopByAlias(void*, void*);
void* OSGetThreadPriority(void*);
void OSSetThreadPriority(void*, int);
unsigned int time(int);
void srand(unsigned int);
void appFreeUni__FPv(void*);
void StringValueTree_DestroySubtree(void*, void*);
void* StringTree_FindInsertionPosition(void*, void*, unsigned char*, unsigned char*, unsigned char*);
void* NamedObjectMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned char*);
void __dla__FPv(void*);
extern unsigned char __vt__12CAnotherHand[];
extern char s_ambientSoundFactoryKey[];
extern char s_ambientSoundTaskRegistryKey[];
extern const char ambientActorTypeStrings[];
extern int g_anotherHandThreadStackPointer;
extern CSharedDoc* g_sharedDoc;
extern void* g_soundManager;
extern unsigned char s_ambientSoundTypeInfo;
extern unsigned char s_actorTypeInfo;
extern const float kFloat_6_2831855;
extern float s_twoPi_800CBF34;
extern volatile float s_ambientSoundTwoPi;

// Sound-name trees use [count,root,first] headers; end is header+4. Nodes
// contain left/right/tagged parent at +0/+4/+8, string at +0xC, float at +0x18.
// Parent bit 0 is color. Clear/destroy own keys and nodes; Find/GetOrInsert borrow keys.
struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;
};
// Legacy 16-byte string overlay; these three static helpers are emitted even
// though this unit has no call sites. Keep them separate from the 12-byte
// TargetString below. The +8 word's capacity meaning is not established by a caller.
struct String {
    unsigned char flags;
    unsigned char data[3];
    unsigned int size;
    unsigned int heapWord08;
    unsigned char* ptr;
};
extern unsigned int s_targetValueDefaultWord;

// Read the legacy overlay's heap length at +4 or its inline length byte.
static unsigned int str_size(String* s) {
    return (s->flags & 0x80) ? s->size : (s->flags & 0x7f);
}
// str_capacity returns an unresolved heap word at +8, or the inline length.
// No caller proves capacity semantics; the shared name is provisional.
// Read the legacy overlay's heap word at +8 or inline length.
static unsigned int str_capacity(String* s) {
    return (s->flags & 0x80) ? s->heapWord08 : (s->flags & 0x7f);
}
// Borrow the legacy overlay's +0xC heap pointer or inline bytes at +1.
static unsigned char* str_data(String* s) {
    return (s->flags & 0x80) ? s->ptr : (unsigned char*)s + 1;
}

struct TargetString {
    unsigned int flags, size;
    unsigned char* ptr;
    // Leave storage untouched for callers that immediately construct all string words.
    TargetString() {}
    TargetString(const TargetString* key);
    // Release owned heap text; inline bytes need no cleanup.
    ~TargetString() {
        if (flags >> 31)
            appFreeUni__FPv(ptr);
    }
};
// Read inline length or the heap-string length word.
static unsigned int target_size(TargetString* s) {
    unsigned int size;
    if (!(s->flags >> 31))
        size = *(unsigned char*)s & 0x7f;
    else
        size = s->size;
    return size;
}
// Compare unsigned lengths and return -1, 0, or 1.
static int target_order(unsigned int a, unsigned int b) {
    int r;
    if (a < b)
        r = -1;
    else if (a == b)
        r = 0;
    else
        r = 1;
    return r;
}
// Return the smaller string comparison length by reference.
static const unsigned int& target_min(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}

#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Writes an exact match or the header+4 end sentinel, not a lower-bound-only result.
// result/head/key must be valid; returned node is borrowed until tree mutation.
// Find the lower-bound sound-name node, returning the sentinel when the name is absent.
void AmbientSoundNameTree_Find(TreeNode** result, const TreeNode* head, const TargetString* key) {
    TreeNode* parent = (TreeNode*)((char*)head + 4);
    TreeNode* node = head->right;
    while (node != 0) {
        unsigned int limit;
        unsigned int len;
        unsigned int keyLen;
        unsigned int m;
        unsigned int n;
        unsigned int length;
        unsigned char* data;
        unsigned char* other;
        if (!(key->flags >> 31)) {
            n = *(unsigned char*)key & 0x7f;
            data = (unsigned char*)key + 1;
        } else {
            data = key->ptr;
            n = key->size;
        }
        keyLen = n;
        TargetString* value = (TargetString*)((char*)node + 12);
        m = !(value->flags >> 31) ? *(unsigned char*)value & 0x7f : value->size;
        if (!(value->flags >> 31)) {
            other = (unsigned char*)value + 1;
            length = *(unsigned char*)value & 0x7f;
        } else {
            other = value->ptr;
            length = value->size;
        }
        len = length;
        limit = target_min(m, len);
        int cmp = memcmp(other, data, target_min(keyLen, limit));
        if (!cmp) {
            cmp = limit < n ? -1 : limit == n ? 0 : 1;
        }
        if (cmp >= 0) {
            parent = node;
            node = node->left;
        } else
            node = node->right;
    }
    if (parent != (TreeNode*)((char*)head + 4)) {
        unsigned int n;
        unsigned char* data;
        TargetString* value;
        unsigned int m;
        value = (TargetString*)((char*)parent + 12);
        if (!(value->flags >> 31)) {
            n = *(unsigned char*)value & 0x7f;
            data = (unsigned char*)value + 1;
        } else {
            data = value->ptr;
            n = value->size;
        }
        unsigned int keyLen = n;
        m = !(key->flags >> 31) ? *(unsigned char*)key & 0x7f : key->size;
        unsigned int length;
        unsigned char* other;
        if (!(key->flags >> 31)) {
            other = (unsigned char*)key + 1;
            length = *(unsigned char*)key & 0x7f;
        } else {
            other = key->ptr;
            length = key->size;
        }
        unsigned int len = length;
        unsigned int limit = target_min(m, len);
        int cmp = memcmp(other, data, target_min(keyLen, limit));
        if (!cmp) {
            cmp = limit < n ? -1 : limit == n ? 0 : 1;
        }
        if (cmp < 0)
            parent = (TreeNode*)((char*)head + 4);
    }
    *result = parent;
}
#pragma use_lmw_stmw reset
#pragma opt_strength_reduction reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

// Copy the key, allocating independent text for a heap-backed string.
inline TargetString::TargetString(const TargetString* key) {
    unsigned int flags = key->flags;
    if (!(flags >> 31)) {
        unsigned int size = key->size;
        unsigned char* ptr = key->ptr;
        this->flags = flags;
        this->size = size;
        this->ptr = ptr;
    } else {
        this->flags = 0;
        this->size = 0;
        this->ptr = 0;
        TargetString* out = this;
        String_ReserveCapacity(out, key->size);
        volatile unsigned char unused;
        struct Alloc {
            unsigned char c;
        } copy;
        unsigned char allocator = unused;
        unsigned int begin = (unsigned int)key->ptr;
        unsigned int end = begin + key->size;
        copy.c = allocator;
        String_ReplaceRange(out, 0, 0, begin, end, &copy.c);
    }
}
struct TargetValue {
    TargetString str;
    unsigned int g;
    // Construct an owned key and initialize the attenuation value word.
    TargetValue(const TargetString* key) : str(key), g(s_targetValueDefaultWord) {}
};
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Returns a borrowed float-value address; insertion copies the key and initializes
// the value from s_targetValueDefaultWord. Existing values are preserved.
// Find or insert a sound-name entry and return its stored volume value.
void* AmbientSoundNameTree_GetOrInsertValue(void* self, const TargetString* key) {
    unsigned char a, b;
    void* candidate;
    void* node = StringTree_FindInsertionPosition(self, (void*)key, (unsigned char*)&candidate, &a, &b);
    TargetString* found;
    do {
        if (candidate) {
            found = (TargetString*)((char*)candidate + 12);
            unsigned int n;
            unsigned char* data;
            if (!(key->flags >> 31)) {
                n = *(unsigned char*)key & 0x7f;
                data = (unsigned char*)key + 1;
            } else {
                data = key->ptr;
                n = key->size;
            }
            unsigned int m;
            unsigned int keyLen = n;
            m = !(found->flags >> 31) ? *(unsigned char*)found & 0x7f : found->size;
            unsigned int length;
            unsigned char* other;
            if (!(found->flags >> 31)) {
                other = (unsigned char*)found + 1;
                length = *(unsigned char*)found & 0x7f;
            } else {
                other = found->ptr;
                length = found->size;
            }
            unsigned int len = length;
            unsigned int limit = target_min(m, len);
            int cmp = memcmp(other, data, target_min(keyLen, limit));
            if (!cmp) {
                cmp = limit < n ? -1 : limit == n ? 0 : 1;
            }
            if (cmp >= 0)
                break;
        }
        {
            TargetValue value(key);
            found = (TargetString*)((char*)NamedObjectMap_InsertNode(self, node, a, b, &value) + 12);
        }
    } while (0);
    return (char*)found + 12;
}
#pragma use_lmw_stmw reset
#pragma opt_strength_reduction reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset

// Stop aliases in both ambient-source lists when the stop message arrives.
void CAmbientSound::HandleMsg(const ActorMessage* message) {
    using namespace CallbackThreadViews;
    typedef CallbackThreadViews::TreeNode TreeNode;
    switch (message->type) {
    case 1:
        break;
    case 2:
        TreeNode* sentinel =
            (TreeNode*)(&((AmbientSourceLists*)AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData)
                             ->boxSentinel);
        TreeNode* current =
            (TreeNode*)((AmbientSourceLists*)AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData)
                ->firstBox;
        while (current != sentinel) {
            Sound_StopByAlias(g_soundManager, (char*)current + 8);
            current = current->right;
        }
        TreeNode* cur2 =
            (TreeNode*)((AmbientSourceLists*)AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData)
                ->firstPositional;
        TreeNode* end2 =
            (TreeNode*)(&((AmbientSourceLists*)AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData)
                             ->positionalSentinel);
        while (cur2 != end2) {
            Sound_StopByAlias(g_soundManager, (char*)cur2 + 8);
            cur2 = cur2->right;
        }
        break;
    }
}

// Cancel a live callback thread, release its stack, and optionally free the owner.
CAnotherHand::~CAnotherHand() {
    void* value = thread;
    vtbl = __vt__12CAnotherHand;
    if (value != 0) {
        OSCancelThread((OSThread*)value);
        thread = 0;
    }
    void* array = threadStack;
    if (array != 0) {
        __dla__FPv(array);
        threadStack = 0;
    }
    g_anotherHandThreadStackPointer = 0;
}

struct CallbackObject {
    virtual void slot2(void*, int);
};

} // extern "C"

} // namespace CallbackThreadViews

// Run the configured callback under the mutex after seeding randomness and applying priority.
int CAnotherHand::CallbackThreadEntry(void* self) {
    using namespace CallbackThreadViews;
    char* s = (char*)self;
    ((void (*)(void*))OSLockMutex)(s + 0x330);
    srand(time(0));
    {
        int r0;
        if (AsCAnotherHand(s)->thread != 0)
            r0 = (int)((char*)OSGetThreadPriority(AsCAnotherHand(s)->thread) - 0x18);
        else
            r0 = 0;
        if ((*(int*)&AsCAnotherHand(s)->threadPriorityDelta) != r0 && AsCAnotherHand(s)->thread != 0)
            OSSetThreadPriority(AsCAnotherHand(s)->thread, (*(int*)&AsCAnotherHand(s)->threadPriorityDelta) + 0x18);
    }
    CallbackObject* obj = (*(CallbackObject**)&AsCAnotherHand(s)->callbackReceiver);
    obj->slot2(AsCAnotherHand(s)->callbackArgument, (*(int*)&AsCAnotherHand(s)->callbackValue));
    AsCAnotherHand(s)->thread = 0;
    ((void (*)(void*))OSUnlockMutex)(s + 0x300 + 0x30);
    return 0;
}

namespace CallbackThreadViews {

extern "C" {

// Advance to the next ordered node, masking the color bit from parent links.
void AmbientSoundTree_AdvanceIterator(TreeNode** node) {
    TreeNode* current = *node;
    TreeNode* child = current->right;
    if (child != 0) {
        while (child->left != 0)
            child = child->left;
        *node = child;
        return;
    }
    TreeNode* parent;
    while (*node != (parent = (TreeNode*)((unsigned int)(*node)->parent & ~1u))->left)
        *node = parent;
    *node = parent;
}

// Destroy the native actor base and optionally free the ambient-sound object.
CAmbientSound::~CAmbientSound() {
    using namespace CallbackThreadViews;
    __dt__18NativeBase80201D18Fv(this, 0);
}

// Initialize callback-thread ownership, mutex state, and its separate thread stack.
CAnotherHand::CAnotherHand() {
    thread = 0;
    vtbl = __vt__12CAnotherHand;
    threadStack = 0;
    ((void (*)(void*))OSInitMutex)(((CallbackThreadMutex*)this)->mutex);
    void* value = __nwa__FUl(0x10000);
    threadStack = value;
    g_anotherHandThreadStackPointer = (int)value;
}

// Initialize the mutex passed by the caller.
void Mutex_Init(void* receiver) {
    OSInitMutex(receiver);
}

// Lock the inventory mutex supplied by the caller.
void ItemInventory_LockMutex(void* forwarded0) {
    OSLockMutex(forwarded0);
}

// Unlock the inventory mutex supplied by the caller.
void ItemInventory_UnlockMutex(void* forwarded0) {
    OSUnlockMutex(forwarded0);
}

// Return the ambient-sound actor's runtime type descriptor.
unsigned char* CAmbientSound::GetRTTI() {
    return &s_ambientSoundTypeInfo;
}

// Lock this callback-thread owner's embedded mutex.
void CAnotherHand::Lock() {
    ((void (*)(void*))OSLockMutex)(((CallbackThreadMutex*)this)->mutex);
}

// Unlock this callback-thread owner's embedded mutex.
void CAnotherHand::Unlock() {
    ((void (*)(void*))OSUnlockMutex)(((CallbackThreadMutex*)this)->mutex);
}

// Treat a missing thread as finished; otherwise query whether it has terminated.
int CThread::IsFinished() {
    int value = thread;
    if (value != 0)
        return OSIsThreadTerminated((void*)value) != 0;
    return 1;
}

// Initialize the callback/path helper's cached two-pi value.
void ActorThread_InitTwoPi() {
    s_twoPi_800CBF34 = kFloat_6_2831855;
}

} // extern "C"

} // namespace CallbackThreadViews

// Register the ambient-sound RTTI and factory with the actor system.
void CAmbientSound::RegisterType() {
    using namespace CallbackThreadViews;
    float fullTurnRadians = 6.2831854820251465f;
    const char* name = ambientActorTypeStrings;
    s_ambientSoundTwoPi = fullTurnRadians;
    IntPair_Set(&CallbackThreadViews::s_ambientSoundTypeInfo, name + 0xE1, &s_actorTypeInfo);
    void* state = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(state, (void*)CAmbientSound::Create, s_ambientSoundFactoryKey);
    void* ambient = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(ambient, s_ambientSoundTaskRegistryKey);
}

namespace CallbackThreadViews {

extern "C" {

// Cancel an existing nonterminated thread and clear its handle.
void CThread::CancelIfRunning() {
    int value = thread;
    int terminated = value != 0 ? OSIsThreadTerminated((void*)value) != 0 : 1;
    if (!terminated) {
        OSCancelThread((OSThread*)thread);
        thread = 0;
    }
}

// Clear and release the path point buffer, optionally freeing its owning object.
void* GolfAimPath_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        int* count = (int*)((char*)self + 4);
        *count = *count - (int)*count;
        if (self != 0 && self != 0 && self != 0 && self != 0) {
            void* value = *(void**)self;
            if (value != 0) {
                int* count2 = (int*)((char*)self + 4);
                *count2 = *count2 - (int)*count2;
                appFreeUni__FPv(value);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Clear the path's logical point count while retaining its allocated buffer.
void GolfAimPath_Clear(void* self) {
    int* value = (int*)((char*)self + 4);
    *value = *value - (int)*value;
}

extern float kFloat_0_Actor, kFloat_0_5_805DDCE4, kFloat_1_Actor, kFloat_neg1_Actor, kFloat_3_2_Actor;
extern char s_cameraTransform[];

} // extern "C"

} // namespace CallbackThreadViews

// Store callback inputs and start a suspended thread on the fixed-size stack, then resume it.
// Callback pointers are borrowed through asynchronous completion. The owner must
// have an allocated 64-KiB stack and no live callback thread. The callback executes
// under the embedded mutex; priority is 24 + priorityDelta. Returns 1 if created,
// 0 if OSCreateThread fails. This function publishes the handle before unlocking.
#pragma use_lmw_stmw on
int CAnotherHand::StartCallbackThread(void* receiver, void* argument, void* value, void* unused,
                                      unsigned int priorityDelta) {
    using namespace CallbackThreadViews;
    // The lock takes the mutex by raw offset; the unlocks below go through the mutex view.
    ((void (*)(void*))OSLockMutex)((char*)this + 0x330);
    callbackReceiver = receiver;
    callbackArgument = argument;
    callbackValue = value;
    threadPriorityDelta = priorityDelta;
    Memory_Clear(threadStack, (unsigned int)kCallbackThreadStackBytes);
    int ok;
    if (!OSCreateThread(osThread, (void*)CAnotherHand::CallbackThreadEntry, this,
                        (void*)((char*)threadStack + kCallbackThreadStackBytes), kCallbackThreadStackBytes,
                        kCallbackThreadPriority, 1)) {
        ok = 0;
    } else {
        OSResumeThread(osThread);
        ok = 1;
        thread = osThread;
    }
    if (!ok) {
        ((void (*)(void*))OSUnlockMutex)(((CallbackThreadMutex*)this)->mutex);
        return 0;
    } else {
        ((void (*)(void*))OSUnlockMutex)(((CallbackThreadMutex*)this)->mutex);
        return 1;
    }
}
#pragma use_lmw_stmw reset

namespace CallbackThreadViews {

extern "C" {

// The shared camera transform stores three basis vectors before its world position.
struct AmbientListenerTransform {
    float basis[9];
    float worldPosition[3];
};
enum {
    kAmbientListenerPositionOffset = (int)&((AmbientListenerTransform*)0)->worldPosition
};
// Subtract world-space vectors through the existing out-of-line WVector operator.
static inline void AmbientDisplacement(void* out, void* to, void* from) {
    __mi__FRC7WVectorRC7WVector(out, to, from);
}
// Scale into caller-provided vector storage, preserving the existing operator call.
static inline void AmbientScale(void* out, void* vector, float scale) {
    __ml__FRC7WVectorf(out, vector, scale);
}
// Return a dot product without normalizing either vector.
static inline float AmbientDot(void* left, void* right) {
    return __ml__FRC7WVectorRC7WVector(left, right);
}

// self is an oriented box center followed by three full-size axis vectors.
// Listener is the camera world position at s_cameraTransform+0x24. Axes must be
// nonzero and orthogonal. fadeDistance is in yards: distance is divided by 3.2
// world units/yard before fading. Negative distance disables attenuation; zero
// is not guarded outside the box. Inside always returns 1; outside is clamped at 0.
// Measure listener distance outside an oriented box and fade volume to zero over the given distance.
// Entry: required 48-byte AmbientBoxAxes and initialized camera transform; finite
// center/listener/full-axis components, three mutually orthogonal nonzero axes.
// Each axis's LENGTH is a full box extent, not a unit direction. Valid tuning
// domain is finite fadeDistance>0 yards, or finite negative to disable fading.
// Producer: CAmbientSound_OnProcess passes the stored source's box/fadeDistance;
// consumer multiplies returned gain with the source gain. No allocation/writes.
// Degenerates are not repaired: OBB_ContainsPoint runs FIRST even with negative
// fadeDistance. If it returns true, or distance is negative, return 1. Otherwise
// each zero-length axis causes a 0/0 projection; no meaningful box is guaranteed.
// For a valid box and zero fade, positive outside distance gives -infinity then
// 0; a zero-distance boundary can give NaN then 0 via !(0<volume). NaN volume is
// likewise clamped to 0, not propagated; NaN inputs are outside the useful domain.
// Infinity/NaN and nonorthogonal axes are not supported geometry contracts.
float CAmbientSound::GetVolume(AmbientBoxAxes* box, float fadeDistance) {
    using namespace CallbackThreadViews;
    float negativeOne;
    float oneHalf;
    float squaredDistance;
    float halfAxisLength;
    float axisDistance;
    float (*axis)[3];
    int axisIndex;
    if (OBB_ContainsPoint(box, s_cameraTransform + kAmbientListenerPositionOffset) ||
        fadeDistance < (squaredDistance = kFloat_0_Actor))
        return kFloat_1_Actor;
    oneHalf = kFloat_0_5_805DDCE4;
    negativeOne = kFloat_neg1_Actor;
    axis = box->axis;
    // For each full-size box axis A: signed distance = dot((listener-center)/2,A)/(|A|/2).
    // Accumulate only the excess beyond [-|A|/2,+|A|/2], then take one square root.
    for (axisIndex = 0; axisIndex < 3; ++axisIndex, ++axis) {
        halfAxisLength = oneHalf * Magnitude__7WVectorCFv(axis);
        float halfListenerDisplacement[3];
        float listenerFromBoxCenter[3];
        AmbientDisplacement(listenerFromBoxCenter, s_cameraTransform + kAmbientListenerPositionOffset, box);
        AmbientScale(halfListenerDisplacement, listenerFromBoxCenter, kFloat_0_5_805DDCE4);
        axisDistance = AmbientDot(halfListenerDisplacement, axis) / halfAxisLength;
        if (axisDistance < negativeOne * halfAxisLength) {
            float excessAlongAxis = axisDistance + halfAxisLength;
            squaredDistance = excessAlongAxis * excessAlongAxis + squaredDistance;
        } else if (axisDistance > halfAxisLength) {
            float excessAlongAxis = axisDistance - halfAxisLength;
            squaredDistance = excessAlongAxis * excessAlongAxis + squaredDistance;
        }
    }
    float volume = kFloat_1_Actor - Math_SquareRootApprox(squaredDistance) / (kFloat_3_2_Actor * fadeDistance);
    if (!(kFloat_0_Actor < volume))
        return kFloat_0_Actor;
    return volume;
}

#pragma use_lmw_stmw on
// Requires at least two retained path points (count and segment loop fit a byte).
// pointArg is borrowed world XYZ. Writes closestPoint on every return: 1 is
// recovered ground plus ball radius; 0 no recovery; 2 terrain code 9 blocks
// the scan; 3 input terrain <3; 4 outside playable region. Codes 0/2/3/4 use
// pre-shot position as fallback. The two search phases use different metrics.
// Search path-adjacent playable ground and store a recovery point or the initial fallback.
enum RecoveryPointResult {
    kRecoveryNotFound = 0,
    kRecoveryFound = 1,
    kRecoveryBlocked = 2,
    kRecoveryInputTerrainExcluded = 3,
    kRecoveryInputOutsideRegion = 4
};
enum RecoveryTerrainPolicy {
    kRecoveryAcceptedTerrain = 1,
    kRecoveryMinimumInputTerrain = 3,
    kRecoveryBlockingTerrain = 9
};
// Resolve playable terrain near the retained path; always publish recovery/fallback XYZ and return a RecoveryPointResult.
// A segment candidate must beat the existing 3200-unit bound before the horizontal direction is initialized.
int GolfAim_FindPlayableRecoveryPoint(void* self, void* pointArg) {
    unsigned int segmentStartByteOffset;
    void* pathPointBuffer;
    Vec* point = (Vec*)pointArg;
    // Vec is a three-u32 copy view, not integer geometry: WVector calls interpret its bits as XYZ.
    // Each named temporary retains its original storage/lifetime and geometric role.
    Vec preShotFallback, segmentCandidate, horizontalSearchDirection, initialGroundQuery, interpolatedSegmentCandidate,
        candidateFromInput, selectedCandidateFromInput, probeGroundQuery;
    Vec probePosition;
    Vec probeDisplacement;
    Vec projectedGroundPoint;
    Vec acceptedGroundQuery;
    Vec acceptedProbePosition;
    Vec acceptedProbeDisplacement;
    Vec weightedBestSegmentEnd;
    Vec weightedBestSegmentStart;
    void* liveBall = GolfBall__Fv();
    Vec ballFromSample;
    Vec sampleGroundQuery;
    Vec weightedSegmentEnd;
    Vec weightedSegmentStart;
    Vec interpolatedSample;
    Vec segmentDisplacement;
    Vec samplePosition;
    void* preShotSample = PreData__9CGolfBallFi(liveBall, 0);

    enum VectorWord {
        kVectorWordZero = 0,
        kVectorWordAllBits = 0xFFFFFFFFu
    };
    VectorWord inputZBits, preShotZBits, inputYBits, inputXBits, preShotYBits, preShotXBits;
    preShotXBits = (VectorWord)((Vec*)preShotSample)->x;
    preShotYBits = (VectorWord)((Vec*)preShotSample)->y;
    inputXBits = (VectorWord)point->x;
    preShotFallback.x = preShotXBits;
    inputYBits = (VectorWord)point->y;
    preShotFallback.y = preShotYBits;
    inputZBits = (VectorWord)point->z;
    preShotZBits = (VectorWord)((Vec*)preShotSample)->z;
    preShotFallback.z = preShotZBits;
    initialGroundQuery.x = inputXBits;
    initialGroundQuery.y = inputYBits;
    initialGroundQuery.z = inputZBits;
    if (GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &initialGroundQuery) <
        kRecoveryMinimumInputTerrain) {
        ((AimPathStorage*)self)->closestPoint = *(WVector*)&preShotFallback;
        return kRecoveryInputTerrainExcluded;
    }
    void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (CourseData_IsPointOutsidePlayableRegion(AsCGolfDoc(manager)->courseData, point)) {
        ((AimPathStorage*)self)->closestPoint = *(WVector*)&preShotFallback;
        return kRecoveryInputOutsideRegion;
    }
    float step, zero, epsilon, minDistance, increment, minT, t, bestDistance;

    int segmentEndByteOffset;
    bestDistance = kFloat_3200_CourseHelpers;
    unsigned char i = 0;
    step = kFloat_1_CourseGeometry;
    epsilon = kRecoveryPointSearchEpsilon;
    zero = kFloat_0_CourseHelpers;
    // Phase 1: sample each path segment at ~16-world-unit spacing. Choose its nearest
    // terrain-1 sample to the ball, then the segment candidate nearest pointArg.
    for (; i < ((AimPathStorage*)self)->count - 1; ++i) {
        t = kFloat_0_CourseHelpers;
        minT = kFloat_1_CourseGeometry;
        VectorWord segmentPoints;
        VectorWord nextIndex;
        segmentPoints = (VectorWord)(unsigned int)((AimPathStorage*)self)->points;
        nextIndex = (VectorWord)(i + 1);
        segmentStartByteOffset = i * 12;
        segmentEndByteOffset = nextIndex * 12;
        minDistance = kFloat_3200_CourseHelpers;
        __mi__FRC7WVectorRC7WVector(&segmentDisplacement, (void*)(segmentPoints + segmentEndByteOffset),
                                    (void*)(segmentPoints + segmentStartByteOffset));
        float len = WVectorLen(*(WVector*)&segmentDisplacement);
        increment = step / (float)ceil((len * epsilon));
        while (t < step) {
            pathPointBuffer = ((AimPathStorage*)self)->points;
            __ml__FfRC7WVector(&weightedSegmentEnd, (char*)pathPointBuffer + segmentEndByteOffset, t);
            __ml__FfRC7WVector(&weightedSegmentStart, (char*)pathPointBuffer + segmentStartByteOffset, step - t);
            __pl__FRC7WVectorRC7WVector(&interpolatedSample, &weightedSegmentStart, &weightedSegmentEnd);
            *(WVector*)&samplePosition = *(WVector*)&interpolatedSample;
            sampleGroundQuery = samplePosition;
            if (GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &sampleGroundQuery) ==
                kRecoveryAcceptedTerrain) {
                __mi__FRC7WVectorRC7WVector(&ballFromSample, GolfBall__Fv(), &samplePosition);
                float d = WVectorLen(*(WVector*)&ballFromSample);
                if (d < minDistance) {
                    minDistance = d;
                    minT = t;
                }
            }
            t += increment;
        }
        pathPointBuffer = ((AimPathStorage*)self)->points;
        __ml__FfRC7WVector(&weightedBestSegmentEnd, (char*)pathPointBuffer + segmentEndByteOffset, minT);
        __ml__FfRC7WVector(&weightedBestSegmentStart, (char*)pathPointBuffer + segmentStartByteOffset, step - minT);
        __pl__FRC7WVectorRC7WVector(&interpolatedSegmentCandidate, &weightedBestSegmentStart, &weightedBestSegmentEnd);
        *(WVector*)&segmentCandidate = *(WVector*)&interpolatedSegmentCandidate;
        __mi__FRC7WVectorRC7WVector(&candidateFromInput, &segmentCandidate, point);
        float dist = WVectorLen(*(WVector*)&candidateFromInput);
        if (dist < bestDistance) {
            float distValue = dist;
            __mi__FRC7WVectorRC7WVector(&selectedCandidateFromInput, &segmentCandidate, point);
            bestDistance = distValue;
            *(WVector*)&horizontalSearchDirection = *(WVector*)&selectedCandidateFromInput;
            *(float*)((char*)&horizontalSearchDirection + 4) = zero;
        }
    }
    // Phase 2: flatten/normalize toward that selected segment candidate; probe 32..160 world units
    // from pointArg in steps of 32, stopping at terrain 9 or accepting terrain 1.
    t = kFloat_32_CourseHelpers;
    (*(WVector*)&horizontalSearchDirection).Normalize();
    zero = kFloat_32_CourseHelpers;
    step = kFloat_192_CourseHelpers;
    while (t < step) {
        __ml__FfRC7WVector(&probeDisplacement, &horizontalSearchDirection, t);
        __pl__FRC7WVectorRC7WVector(&probePosition, point, &probeDisplacement);
        probeGroundQuery = probePosition;
        unsigned char kind = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &probeGroundQuery);
        if (kRecoveryBlockingTerrain == kind) {
            ((AimPathStorage*)self)->closestPoint = *(WVector*)&preShotFallback;
            return kRecoveryBlocked;
        }
        if (kind == kRecoveryAcceptedTerrain) {
            __ml__FfRC7WVector(&acceptedProbeDisplacement, &horizontalSearchDirection, t);
            __pl__FRC7WVectorRC7WVector(&acceptedProbePosition, point, &acceptedProbeDisplacement);
            acceptedGroundQuery = acceptedProbePosition;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&projectedGroundPoint, CQuadTree_GetInstance(),
                                                    &acceptedGroundQuery, 0, 0, 0);
            ((AimPathStorage*)self)->closestPoint = *(WVector*)&projectedGroundPoint;
            float f = ((CGolfBall*)GolfBall__Fv())->m_radius;
            ((AimPathStorage*)self)->closestPoint.y = ((AimPathStorage*)self)->closestPoint.y + f;
            return kRecoveryFound;
        }
        t = t + zero;
    }
    ((AimPathStorage*)self)->closestPoint = *(WVector*)&preShotFallback;
    return kRecoveryNotFound;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
struct TreeState {
    int count;
    void* root;
    void* sentinel;
};
// Free all sound-name nodes and reset the ordered-tree header to empty.
// Same teardown as SoundNameMap_dtor with raw offsets: node +0/+4 = child nodes, +12 = small-string tag (bit 31 = heap text),
// +20 = heap text pointer. The '(x + 0xC) && (x + 0xC)' tests are always true (compiler-matching residue).
void AmbientSoundNameTree_Clear(TreeState* self) {
    register void* root = self->root;
    if (root) {
        // The first three levels are unrolled; deeper children recurse through the helper.
        void* a = *(void**)root;
        if (a) {
            void* b = *(void**)a;
            if (b != 0) {
                void* c = *(void**)b;
                if (c) {
                    if (*(void**)c)
                        StringValueTree_DestroySubtree(self, *(void**)c);
                    if (*(void**)((char*)c + 4))
                        StringValueTree_DestroySubtree(self, *(void**)((char*)c + 4));
                    if ((((unsigned int)c + 0xC) && ((unsigned int)c + 0xC)) &&
                        (((unsigned int)*(void**)((char*)c + 12) >> 31) != 0))
                        appFreeUni__FPv(*(void**)((char*)c + 20));
                    appFreeUni__FPv(c);
                    c = 0;
                }
                c = *(void**)((char*)b + 4);
                if (c) {
                    if (*(void**)c)
                        StringValueTree_DestroySubtree(self, *(void**)c);
                    if (*(void**)((char*)c + 4))
                        StringValueTree_DestroySubtree(self, *(void**)((char*)c + 4));
                    if ((((unsigned int)c + 0xC) && ((unsigned int)c + 0xC)) &&
                        (((unsigned int)*(void**)((char*)c + 12) >> 31) != 0))
                        appFreeUni__FPv(*(void**)((char*)c + 20));
                    appFreeUni__FPv(c);
                    c = 0;
                }
                if ((((unsigned int)b + 0xC) && ((unsigned int)b + 0xC)) &&
                    (((unsigned int)*(void**)((char*)b + 12) >> 31) != 0))
                    appFreeUni__FPv(*(void**)((char*)b + 20));
                appFreeUni__FPv(b);
                b = 0;
            }
            b = *(void**)((char*)a + 4);
            if (b) {
                void* c = *(void**)b;
                if (0 != c) {
                    if (*(void**)c)
                        StringValueTree_DestroySubtree(self, *(void**)c);
                    if (*(void**)((char*)c + 4))
                        StringValueTree_DestroySubtree(self, *(void**)((char*)c + 4));
                    if ((((unsigned int)c + 0xC) && ((unsigned int)c + 0xC)) &&
                        (((unsigned int)*(void**)((char*)c + 12) >> 31) != 0))
                        appFreeUni__FPv(*(void**)((char*)c + 20));
                    appFreeUni__FPv(c);
                    c = 0;
                }
                c = *(void**)((char*)b + 4);
                if (c) {
                    if (*(void**)c)
                        StringValueTree_DestroySubtree(self, *(void**)c);
                    if (*(void**)((char*)c + 4))
                        StringValueTree_DestroySubtree(self, *(void**)((char*)c + 4));
                    if ((((unsigned int)c + 0xC) && ((unsigned int)c + 0xC)) &&
                        (((unsigned int)*(void**)((char*)c + 12) >> 31) != 0))
                        appFreeUni__FPv(*(void**)((char*)c + 20));
                    appFreeUni__FPv(c);
                    c = 0;
                }
                if ((((unsigned int)b + 0xC) && ((unsigned int)b + 0xC)) &&
                    (((unsigned int)*(void**)((char*)b + 12) >> 31) != 0))
                    appFreeUni__FPv(*(void**)((char*)b + 20));
                appFreeUni__FPv(b);
                b = 0;
            }
            if ((((unsigned int)a + 0xC) && ((unsigned int)a + 0xC)) &&
                (((unsigned int)*(void**)((char*)a + 12) >> 31) != 0))
                appFreeUni__FPv(*(void**)((char*)a + 20));
            appFreeUni__FPv(a);
            a = 0;
        }
        a = *(void**)((char*)root + 4);
        if (a) {
            void* b = *(void**)a;
            if (b) {
                void* c = *(void**)b;
                if (c) {
                    if (*(void**)c)
                        StringValueTree_DestroySubtree(self, *(void**)c);
                    if (*(void**)((char*)c + 4))
                        StringValueTree_DestroySubtree(self, *(void**)((char*)c + 4));
                    if ((((unsigned int)c + 0xC) && ((unsigned int)c + 0xC)) &&
                        (((unsigned int)*(void**)((char*)c + 12) >> 31) != 0))
                        appFreeUni__FPv(*(void**)((char*)c + 20));
                    appFreeUni__FPv(c);
                    c = 0;
                }
                c = *(void**)((char*)b + 4);
                if (c) {
                    if (*(void**)c)
                        StringValueTree_DestroySubtree(self, *(void**)c);
                    if (*(void**)((char*)c + 4))
                        StringValueTree_DestroySubtree(self, *(void**)((char*)c + 4));
                    if ((((unsigned int)c + 0xC) && ((unsigned int)c + 0xC)) &&
                        (((unsigned int)*(void**)((char*)c + 12) >> 31) != 0))
                        appFreeUni__FPv(*(void**)((char*)c + 20));
                    appFreeUni__FPv(c);
                    c = 0;
                }
                if ((((unsigned int)b + 0xC) && ((unsigned int)b + 0xC)) &&
                    (((unsigned int)*(void**)((char*)b + 12) >> 31) != 0))
                    appFreeUni__FPv(*(void**)((char*)b + 20));
                appFreeUni__FPv(b);
                b = 0;
            }
            b = *(void**)((char*)a + 4);
            if (b) {
                void* c = *(void**)b;
                if (c) {
                    if (*(void**)c)
                        StringValueTree_DestroySubtree(self, *(void**)c);
                    if (*(void**)((char*)c + 4))
                        StringValueTree_DestroySubtree(self, *(void**)((char*)c + 4));
                    if ((((unsigned int)c + 0xC) && ((unsigned int)c + 0xC)) &&
                        (((unsigned int)*(void**)((char*)c + 12) >> 31) != 0))
                        appFreeUni__FPv(*(void**)((char*)c + 20));
                    appFreeUni__FPv(c);
                    c = 0;
                }
                c = *(void**)((char*)b + 4);
                if (c) {
                    if (*(void**)c)
                        StringValueTree_DestroySubtree(self, *(void**)c);
                    if (*(void**)((char*)c + 4))
                        StringValueTree_DestroySubtree(self, *(void**)((char*)c + 4));
                    if ((((unsigned int)c + 0xC) && ((unsigned int)c + 0xC)) &&
                        (((unsigned int)*(void**)((char*)c + 12) >> 31) != 0))
                        appFreeUni__FPv(*(void**)((char*)c + 20));
                    appFreeUni__FPv(c);
                    c = 0;
                }
                if ((((unsigned int)b + 0xC) && ((unsigned int)b + 0xC)) &&
                    (((unsigned int)*(void**)((char*)b + 12) >> 31) != 0))
                    appFreeUni__FPv(*(void**)((char*)b + 20));
                appFreeUni__FPv(b);
                b = 0;
            }
            if ((((unsigned int)a + 0xC) && ((unsigned int)a + 0xC)) &&
                (((unsigned int)*(void**)((char*)a + 12) >> 31) != 0))
                appFreeUni__FPv(*(void**)((char*)a + 20));
            appFreeUni__FPv(a);
            a = 0;
        }
        if ((((unsigned int)root + 0xC) && ((unsigned int)root + 0xC)) &&
            (((unsigned int)*(void**)((char*)root + 12) >> 31) != 0))
            appFreeUni__FPv(*(void**)((char*)root + 20));
        appFreeUni__FPv(root);
        self->count = 0;
        self->root = 0;
        self->sentinel = &self->root;
        root = 0;
    }
}

} // extern "C"

} // namespace CallbackThreadViews

// ---- merged from ball_actor_and_ambient_sound.cpp ----
namespace AimAndBallModelViews {

extern "C" {

extern float s_twoPi_800CBF34_805E2938;
extern const float kFloat_6_2831855;
extern const float kFloat_0_Ball;
extern const float kFloat_3_2_GolfAim;
extern const float kFloat_96_CourseHelpers;
extern const float kFloat_32_CourseHelpers;
extern const float kFloat_0_CourseHelpers;
extern const float kFloat_1_CourseHelpers;
extern const float kFloat_0_1_CourseHelpers;
extern const float kFloat_1_5_Scene;
extern const float kFloat_1_2_CourseHelpers;
extern const float kFloat_0_00001_CourseHelpers;
extern const float kFloat_neg1_CourseGeometry;
extern const float kFloat_1_CourseGeometry;
extern const unsigned char golfBallResourceStrings[];
extern void* g_sceneManager;
extern const float s_sceneLighting[];
extern const float s_cameraTransform[];
extern const unsigned char __vt__5CBall[];
extern void* g_resourceManager;
extern void* g_view;
extern CSharedDoc* g_sharedDoc;
void Matrix_SetIdentityAffine(void*);
void __as__7WVectorFRC7WVector(void*, const float*);
void __ct__18NativeBase80201D18Fv(void*);
unsigned char WPuppet_GetBoneFlags(void*, int);
void* GolfBall__Fv();
void ResourceManager_ReleaseObject(void*, void*, int);
void appFreeUni__FPv(void*);
void ResourceManager_ReleaseId(void*, void*);
void __ml__FfRC7WVector(void*, const void*, float);
void __mi__FRC7WVectorRC7WVector(void*, void*, const void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, const void*);
void* Matrix_TransformPoint(void*, const void*, const void*);
void WView_ProjectViewPoint(void*, void*, const void*, int);
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
void ResourcePreloadQueue_AddRequest(void*, int, int, float, void*, float, const char*, void*, int, int);
void DrawScene_SetNamedObjectPuppet(void*, const char*, int);
void Model_SetMatrixRecursive(void*, int, const void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
void WPuppet_UpdateBoundsFromBone(void*);
void Matrix_CopyScaledBasis(void*, const void*, float);
void* PartyItemSlots_GetInstance();
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void* ItemInventory_FindBallData(void*, const char*);
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void* appMallocDummy__FUl(unsigned int);
void sprintf(char*, const char*, ...);
void* memset(void*, int, unsigned int);
float Math_SquareRootApprox(float);
void* InstanceTable_Get();
float GolfParameter_ResolveFloat(void*, void*);
void* CQuadTree_GetInstance();
int GetGroundType__9CQuadTreeF7WVector(void*, const void*);
float __ml__FRC7WVectorRC7WVector(void*, const void*);
float Magnitude__7WVectorCFv(const void*);
void GolfBall_IntersectPathSegmentWithRadiusXZ(void*, void*, int, float);
extern const float kFloat_2_Ball, kBallEffectRemovalSpeedThreshold, kFloat_0_00001_CourseHelpers_805DDD38,
    kBallPlaybackPositionJitterStep;
float Magnitude__7WVectorCFv(const void*);
void Vector_CrossProduct(void*, const void*, const void*);
int Vec3_NearlyEqualWithTolerance(void*, const void*, float);
void Matrix_SetNegativeAxisAngleRotation(void*, const void*, float);
void Matrix_CopyAffine(void*, const void*);
void __apl__7WVectorFRC7WVector(void*, const void*);
int rand();

// Initialize the ball/aim helper's cached two-pi value.
void GolfAim_InitTwoPi() {
    s_twoPi_800CBF34_805E2938 = kFloat_6_2831855;
}

typedef Vec3f Vec3;
struct AimVectorCopyBits {
    int x, y, z;
};
// Hole entries are 32 bytes. XYZ at +0xC is the cup position in world units;
// word copies intentionally preserve float bits and the original load order.
// +0x18 is par, also read as par by CPangInfo_CalculateShotRewards and club selection.
enum AimDirectCupPolicy {
    kAimLongDistanceTier = 4,
    kAimTeeParLimit = 5
};
// CClub::SelectBestClub sets +0x48 to tiers <10/<15/<28/<58/>=58 yards.
struct AimClubSelection {
    char reserved00[0x48];
    unsigned char distanceTier;
};
struct AimHoleRecord {
    char prefix[0xC];
    union {
        Vec3 position;
        AimVectorCopyBits positionBits;
    };
    unsigned char par;
    char suffix[7];
};
typedef char AimHoleRecord_size[(sizeof(AimHoleRecord) == 0x20) ? 1 : -1];

// Measure how far the cup exceeds club reach in the horizontal world plane; negative means within reach.
// Inputs are borrowed world XYZ and a radius in world units; the caller selects the 96/32-unit tolerance.
// dx/dz are caller-owned scratch to retain CW floating-point register assignments.
static inline float Aim_ClubReachShortfallXZ(const float* ballWorld, const Vec3& cupWorld, float clubRadius, float& dx,
                                             float& dz) {
    dx = ballWorld[0] - cupWorld.x;
    dz = ballWorld[2] - cupWorld.z;
    return Math_SquareRootApprox(dx * dx + dz * dz) - clubRadius;
}

// Geometric input map: path -> AimPathStorage.points/count (retained world-space polyline);
// query -> borrowed club-parameter query (resolved to yards); GolfBall -> live world position;
// CGolfDoc.holeRecords[currentHoleIndex] -> world-space cup. Recovery uses the same path
// but additionally reads PreData(0) for fallback and the quad tree for terrain/projection.
// out is float[3] world XYZ; path borrows its point vector, query selects club
// parameters. Club distance is converted from yards to world units by 3.2.
// Direct/fallback cup copies preserve cup Y; a successful path intersection
// explicitly sets output Y=0. Segment helper below writes X/Z only.
// Segment radius errors are signed XZ distance minus club radius. Opposite signs
// select a crossing; require a positive dot(direction-to-crossing, direction-to-cup)
// to reject opposite directions, then require the candidate to be closer to the cup.
// Choose a cup target or a forward path/radius intersection, falling back to the cup position.
void GolfAim_FindTargetAlongPath(float* out, void* path, void* clubParameterQuery) {
    AimPathStorage* pathStorage = (AimPathStorage*)path;
    Vec3 cupDirection;
    Vec3 intersectionDirection;
    Vec3 worldIntersection;
    Vec3 directCupPosition;
    int initialTerrainPosition[3];
    unsigned int teeConfirmationPosition[3];
    Vec3 candidateCupPosition;
    Vec3 intersection;
    Vec3 ballToIntersection;
    Vec3 ballToCup;
    Vec3 pathCupPosition;
    float clubRadius = GolfParameter_ResolveFloat(InstanceTable_Get(), clubParameterQuery) * kFloat_3_2_GolfAim;
    Vec3 intersectionToCup;
    // Phase 1: try direct cup policy before intersecting the path.
    // A non-null parameter query skips the tee/par restriction. At distance tier 4
    // (>=58 yards), the horizontal cup distance may exceed clubRadius by <96 world
    // units (30 yards), versus <32 (10 yards) for the default/null query.
    // Shorter tiers skip that reach gate. The default query on tee terrain (0)
    // additionally requires a second tee query and par<5; other terrain passes.
    // These are reach shortfalls relative to club radius, not cup-distance limits.
    do {
        if (clubParameterQuery) {
            if (((AimClubSelection*)InstanceTable_Get())->distanceTier >= kAimLongDistanceTier) {
                char* state = static_cast<char*>((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc));
                AimHoleRecord* hole =
                    (AimHoleRecord*)AsCGolfDoc(state)->holeRecords + AsCGolfDoc(state)->currentHoleIndex;
                *reinterpret_cast<AimVectorCopyBits*>(&directCupPosition) = hole->positionBits;
                float* pos = static_cast<float*>(GolfBall__Fv());
                float cupDeltaXWorld, cupDeltaZWorld;
                if (!(Aim_ClubReachShortfallXZ(pos, directCupPosition, clubRadius, cupDeltaXWorld, cupDeltaZWorld) <
                      kFloat_96_CourseHelpers))
                    break;
            }
            {
                char* state = static_cast<char*>((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc));
                AimHoleRecord* hole =
                    (AimHoleRecord*)AsCGolfDoc(state)->holeRecords + AsCGolfDoc(state)->currentHoleIndex;
                int cupYBits = hole->positionBits.y;
                int cupXBits = hole->positionBits.x;
                reinterpret_cast<int*>(out)[0] = cupXBits;
                reinterpret_cast<int*>(out)[1] = cupYBits;
                reinterpret_cast<int*>(out)[2] = hole->positionBits.z;
            }
            return;
        }
        {
            unsigned int found = 0;
            int eligible = 1;
            float* pos = static_cast<float*>(GolfBall__Fv());
            int ballYBits = reinterpret_cast<int*>(pos)[1];
            int ballXBits = reinterpret_cast<int*>(pos)[0];
            initialTerrainPosition[0] = ballXBits;
            initialTerrainPosition[1] = ballYBits;
            initialTerrainPosition[2] = reinterpret_cast<int*>(pos)[2];
            if (!static_cast<unsigned char>(
                    GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), initialTerrainPosition))) {
                int ok = 0;
                pos = static_cast<float*>(GolfBall__Fv());
                int teeXBits = reinterpret_cast<int*>(pos)[0];
                int teeYBits = reinterpret_cast<int*>(pos)[1];
                teeConfirmationPosition[1] = teeYBits;
                teeConfirmationPosition[0] = teeXBits;
                teeConfirmationPosition[2] = reinterpret_cast<int*>(pos)[2];
                if (!static_cast<unsigned char>(
                        GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), teeConfirmationPosition))) {
                    char* state = static_cast<char*>((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc));
                    AimHoleRecord* hole =
                        (AimHoleRecord*)AsCGolfDoc(state)->holeRecords + AsCGolfDoc(state)->currentHoleIndex;
                    if (hole->par < kAimTeeParLimit)
                        ok = 1;
                }
                if (ok == 0)
                    eligible = 0;
            }
            if (eligible) {
                int ok = 1;
                if (((AimClubSelection*)InstanceTable_Get())->distanceTier >= kAimLongDistanceTier) {
                    char* state = static_cast<char*>((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc));
                    AimHoleRecord* hole =
                        (AimHoleRecord*)AsCGolfDoc(state)->holeRecords + AsCGolfDoc(state)->currentHoleIndex;
                    *reinterpret_cast<AimVectorCopyBits*>(&candidateCupPosition) = hole->positionBits;
                    pos = static_cast<float*>(GolfBall__Fv());
                    float cupDeltaXWorld, cupDeltaZWorld;
                    if (!(Aim_ClubReachShortfallXZ(pos, candidateCupPosition, clubRadius, cupDeltaXWorld,
                                                   cupDeltaZWorld) < kFloat_32_CourseHelpers))
                        ok = 0;
                }
                if (ok)
                    found = 1;
            }
            if (!found)
                break;
            {
                char* state = static_cast<char*>((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc));
                AimHoleRecord* hole =
                    (AimHoleRecord*)AsCGolfDoc(state)->holeRecords + AsCGolfDoc(state)->currentHoleIndex;
                int cupYBits = hole->positionBits.y;
                int cupXBits = hole->positionBits.x;
                reinterpret_cast<int*>(out)[0] = cupXBits;
                reinterpret_cast<int*>(out)[1] = cupYBits;
                reinterpret_cast<int*>(out)[2] = hole->positionBits.z;
            }
            return;
        }
    } while (0);
    // Phase 2: direct cup rules did not select a target. Search path segments backward for
    // a radius crossing toward the cup and closer to it; otherwise copy the cup.
    {
        int segmentIndex = (int)pathStorage->count - 2;
        float zero = kFloat_0_CourseHelpers;
        for (; segmentIndex >= 0; --segmentIndex) {
            unsigned int m = (1 + segmentIndex);
            Vec3* segmentEnd = (Vec3*)pathStorage->points + m;
            float* cur = static_cast<float*>(GolfBall__Fv());
            float endRadiusError = Math_SquareRootApprox((cur[0] - segmentEnd->x) * (cur[0] - segmentEnd->x) +
                                                         (cur[2] - segmentEnd->z) * (cur[2] - segmentEnd->z)) -
                                   clubRadius;
            Vec3* segmentStart = (Vec3*)pathStorage->points + segmentIndex;
            cur = static_cast<float*>(GolfBall__Fv());
            float startRadiusError = Math_SquareRootApprox((cur[0] - segmentStart->x) * (cur[0] - segmentStart->x) +
                                                           (cur[2] - segmentStart->z) * (cur[2] - segmentStart->z)) -
                                     clubRadius;
            if (startRadiusError * endRadiusError < zero) {
                cur = static_cast<float*>(GolfBall__Fv());
                GolfBall_IntersectPathSegmentWithRadiusXZ(&intersection, path, static_cast<unsigned char>(segmentIndex),
                                                          clubRadius);
                __pl__FRC7WVectorRC7WVector(&worldIntersection, &intersection, cur);
                __mi__FRC7WVectorRC7WVector(&ballToIntersection, &worldIntersection,
                                            static_cast<float*>(GolfBall__Fv()));
                __as__7WVectorFRC7WVector(&intersectionDirection, reinterpret_cast<const float*>(&ballToIntersection));
                intersectionDirection.y = zero;
                char* state = static_cast<char*>((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc));
                AimHoleRecord* hole =
                    (AimHoleRecord*)AsCGolfDoc(state)->holeRecords + AsCGolfDoc(state)->currentHoleIndex;
                int cupXBits = hole->positionBits.x;
                int cupYBits = hole->positionBits.y;
                *reinterpret_cast<int*>(&pathCupPosition) = cupXBits;
                *reinterpret_cast<int*>(reinterpret_cast<char*>(&pathCupPosition) + 4) = cupYBits;
                *reinterpret_cast<int*>(reinterpret_cast<char*>(&pathCupPosition) + 8) = hole->positionBits.z;
                __mi__FRC7WVectorRC7WVector(&ballToCup, &pathCupPosition, static_cast<float*>(GolfBall__Fv()));
                __as__7WVectorFRC7WVector(&cupDirection, reinterpret_cast<const float*>(&ballToCup));
                cupDirection.y = zero;
                if (__ml__FRC7WVectorRC7WVector(&intersectionDirection, &cupDirection) > zero) {
                    float len = Magnitude__7WVectorCFv(&cupDirection);
                    __mi__FRC7WVectorRC7WVector(&intersectionToCup, &cupDirection, &intersectionDirection);
                    if (Magnitude__7WVectorCFv(&intersectionToCup) < len) {
                        worldIntersection.y = kFloat_0_CourseHelpers;
                        int intersectionXBits = reinterpret_cast<int*>(&worldIntersection)[0];
                        int intersectionYBits = reinterpret_cast<int*>(&worldIntersection)[1];
                        reinterpret_cast<int*>(out)[0] = intersectionXBits;
                        reinterpret_cast<int*>(out)[1] = intersectionYBits;
                        reinterpret_cast<int*>(out)[2] = reinterpret_cast<int*>(&worldIntersection)[2];
                        return;
                    }
                }
            }
        }
    }
    {
        char* state = static_cast<char*>((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc));
        AimHoleRecord* hole = (AimHoleRecord*)AsCGolfDoc(state)->holeRecords + AsCGolfDoc(state)->currentHoleIndex;
        int fallbackCupXBits = hole->positionBits.x;
        unsigned int fallbackCupYBits = hole->positionBits.y;
        reinterpret_cast<int*>(out)[0] = fallbackCupXBits;
        reinterpret_cast<int*>(out)[1] = fallbackCupYBits;
        reinterpret_cast<int*>(out)[2] = hole->positionBits.z;
    }
}

} // extern C
extern "C" {
struct CF0Guard {
    char* volatile first;
    char* volatile copy;
    char* volatile unused;
    char* volatile sentinel;
    char* node;
    // Guard an unlinked ball-list node until insertion transfers ownership.
    inline CF0Guard(char* p, char* s) : first(p), copy(p), sentinel(s), node(p) {}
    // Free the node only if insertion did not clear this guard.
    inline ~CF0Guard() {
        if (node)
            appFreeUni__FPv(node);
    }
};
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Cache keys are equipped IFF ball IDs; duplicate equipment shares one puppet
// entry. Player state 3 is skipped. Nodes and payloads are owned by this actor,
// IFF records are borrowed, and loaded puppets must be released through their manager.
// Load each player's equipped ball puppet and cache its resource and effect handles.
void CBall::OnLoad() {
    using namespace AimAndBallModelViews;
    char* base = (char*)this;
    int offset = 0;
    int index = 0;
    for (; index < AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->playerCount;
         offset += 0x1D4, ++index) { // +0xB90, +0x2B9
        if (*reinterpret_cast<int*>(
                static_cast<char*>(GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
                                                              static_cast<unsigned char>(index))) +
                8) == 3)
            continue;
        void* manager = g_sharedDoc;
        char* sentinel = base + 0x20;
        char* node = (*(char**)&firstPuppetEntry);
        char* volatile found[2];
        void* volatile wanted[2];
        void* key = *reinterpret_cast<void**>(static_cast<char*>(manager) + offset + 0x88);
        wanted[0] = key;
        wanted[1] = key;
        while (node != sentinel &&
               wanted[1] !=
                   *reinterpret_cast<void**>(
                       static_cast<char*>(*reinterpret_cast<void**>(*reinterpret_cast<void**>(node + 8))) + 0x50))
            node = *reinterpret_cast<char**>(node + 4);
        found[0] = node;
        found[1] = node;
        void* data;
        if (node != base + 0x20)
            data = *reinterpret_cast<void**>(node + 8);
        else
            data = 0;
        if (!data) {
            char* entry;
            void* resource = ItemInventory_FindBallData(AsCSharedDoc(manager)->itemInventory,
                                                        static_cast<const char*>(key)); // +0xA04
            if (resource) {
                entry = static_cast<char*>(__nw__FUl(0x28));
                if (entry) {
                    *reinterpret_cast<int*>(entry) = 0;
                    *reinterpret_cast<int*>(entry + 4) = 0;
                    memset(entry + 8, 0, 0x10);
                    memset(entry + 0x18, 0, 0x10);
                }
                *reinterpret_cast<void**>(entry) = resource;
                char name[0x20];
                ::sprintf(name,
                          reinterpret_cast<const char*>(golfBallResourceStrings + kBallStringsPetModelFormatOffset),
                          static_cast<char*>(resource) + 0x28C);
                resource = 0;
                int zero = reinterpret_cast<int>(resource);
                void* sub = ResourceManager_LoadByExtension(g_resourceManager, name, zero, 1, zero);
                *reinterpret_cast<void**>(entry + 4) = sub;
                sentinel = base + 0x20;
                volatile unsigned char allocator = zero;
                char* list = static_cast<char*>(appMallocDummy__FUl(0xC));
                CF0Guard guard(list, sentinel);
                char* slot = list + 8;
                try {
                    new (slot) void*(entry);
                } catch (...) {
                    appFreeUni__FPv(list);
                    throw;
                }
                char* tail = *reinterpret_cast<char**>(sentinel);
                guard.node = 0;
                *reinterpret_cast<char**>(tail + 4) = list;
                *reinterpret_cast<char**>(list) = *reinterpret_cast<char**>(sentinel);
                *reinterpret_cast<char**>(sentinel) = list;
                *reinterpret_cast<char**>(list + 4) = sentinel;
                ++puppetEntryCount;
            }
        }
    }
    // All eligible IDs are cached; make the first cache entry the active puppet.
    char* node = (*(char**)&firstPuppetEntry);
    if (node != base + 0x20) {
        void* data = *reinterpret_cast<void**>(node + 8);
        puppet = *reinterpret_cast<void**>(static_cast<char*>(data) + 4);
    }
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

// Take the magnitude by inspecting the float sign bit, preserving the original NaN/sign behavior.
static inline float fabsBits(float x) {
    return (*reinterpret_cast<unsigned int*>(&x) & 0x80000000u) ? -x : x;
}

// Preconditions: index and index+1 exist and the projected line crosses the circle.
// radius uses world units. out must have writable X/Z lanes and an already valid
// Y lane if the caller reads all three components. For near-vertical lines, the
// first positive-Z root is retained by the X-range test; no Z-range test is performed.
// Intersect a path segment with the ball-centered XZ circle; write relative X/Z and preserve Y.
void GolfBall_IntersectPathSegmentWithRadiusXZ(void* out, void* array, int index, float radius) {
    float pb[3];
    float pa[3];

    float xc[2];
    float zc[2];
    float dx;
    char* p = *reinterpret_cast<char**>(array) + index * 12;
    WVector a = *(WVector*)p - *(WVector*)(GolfBall__Fv());
    *(WVector*)pa = a;
    p = *reinterpret_cast<char**>(array) + (index + 1) * 12;
    WVector b = *(WVector*)p - *(WVector*)(GolfBall__Fv());
    *(WVector*)pb = b;
    dx = pa[0] - pb[0];
    bool dSmall = fabsBits(dx) < kFloat_0_00001_CourseHelpers;
    if (dSmall) {
        xc[1] = pa[0];
        xc[0] = pa[0];
        zc[0] = Math_SquareRootApprox(radius * radius - pa[0] * pa[0]);
        zc[1] = kFloat_neg1_CourseGeometry * zc[0];
    } else {
        float slope = (pb[2] - pa[2]) / (pb[0] - pa[0]);
        float neg = kFloat_neg1_CourseGeometry * slope;
        float intercept = neg * pa[0] + pa[2];
        float radicand = radius * (slope * slope + kFloat_1_CourseGeometry) * radius - intercept * intercept;
        float root = Math_SquareRootApprox(radicand);
        neg = kFloat_neg1_CourseGeometry * slope;
        xc[0] = (neg * intercept + root) / (slope * slope + kFloat_1_CourseGeometry);
        float root2 = Math_SquareRootApprox(radicand);
        neg = kFloat_neg1_CourseGeometry * slope;
        xc[1] = (neg * intercept - root2) / (slope * slope + kFloat_1_CourseGeometry);
        zc[0] = slope * xc[0] + intercept;
        zc[1] = slope * xc[1] + intercept;
    }
    float lo = pa[0] > pb[0] ? pb[0] : pa[0];
    if (xc[0] >= lo && xc[0] <= (pa[0] < pb[0] ? pb[0] : pa[0])) {
        *reinterpret_cast<float*>(out) = xc[0];
        *reinterpret_cast<float*>(static_cast<char*>(out) + 8) = zc[0];
    } else {
        *reinterpret_cast<float*>(out) = xc[1];
        *reinterpret_cast<float*>(static_cast<char*>(out) + 8) = zc[1];
    }
}

// Optionally free the native member allocation; no additional teardown is required here.
void* __dt__20NativeMember80201D18Fv(void* self, int deleting) {
    if (self && deleting > 0)
        __dl__FPv(self);
    return self;
}

// Reset the ball transform/sinking fields and copy the scene light direction.
void CBall::OnInit() {
    using namespace AimAndBallModelViews;
    Matrix_SetIdentityAffine((char*)matrixBasis);
    groundSinkFactor = kFloat_0_Ball;
    sinkStartFrame = -1;
    (*(WVector*)&lightDirection[0]) = *(WVector*)(s_sceneLighting + 1);
}

// Only visible actors update. Inactive simulation copies position and applies
// full sink depth, then returns. Active simulation derives rolling orientation,
// applies short post-event jitter and playback-interpolated sinking, and finally
// rescales the puppet when the active cache entry has retained effect handles.
// Follow simulated ball position/rotation, update sinking and effects, and refresh the puppet.
void CBall::OnProcess() {
    using namespace AimAndBallModelViews;
    char* base = (char*)this;
    if (!visible)
        return;
    char* ballPhysics = static_cast<char*>(GolfBall__Fv());
    if (!*reinterpret_cast<int*>(ballPhysics + 0x18)) {
        *(WVector*)position = *(WVector*)(reinterpret_cast<const float*>(ballPhysics));
        float sink = groundSinkFactor;
        if (sink > kFloat_0_Ball) {
            float ballRadius = *reinterpret_cast<float*>(ballPhysics + 0xC0);
            position[1] -= sink * ((kFloat_2_Ball * ballRadius));
        }
        char* state = *(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc;
        if ((*(unsigned int*)&AsCGolfDoc(state)->gameStateFlags) & 0x80) {
            DetachFx(3);
            DetachFx(5);
        }
        return;
    }
    *(int*)&lightColor =
        *reinterpret_cast<int*>(const_cast<char*>(reinterpret_cast<const char*>(s_sceneLighting)) + 0x10);
    if ((((CGolfBall*)GolfBall__Fv())->m_velocity).Magnitude() < kBallEffectRemovalSpeedThreshold) {
        DetachFx(3);
        DetachFx(5);
    }

    // Build a world-space roll axis from world up crossed with simulation direction.
    WVector cross;
    float axis[3];
    float zero[3];
    float one[3];
    float jitter[3];
    axis[0] = kFloat_0_Ball;
    axis[1] = kFloat_1_CourseHelpers;
    axis[2] = kFloat_0_Ball;
    Vector_CrossProduct(&cross, axis, ballPhysics + 0xC);
    *(WVector*)rotationAxis = cross;
    zero[0] = kFloat_0_Ball;
    zero[1] = kFloat_0_Ball;
    zero[2] = kFloat_0_Ball;
    if (Vec3_NearlyEqualWithTolerance((char*)rotationAxis, zero, kFloat_0_00001_CourseHelpers_805DDD38) != 0) {
        one[0] = kFloat_0_Ball;
        one[1] = kFloat_1_CourseHelpers;
        one[2] = kFloat_0_Ball;
        *(WVector*)rotationAxis = *(WVector*)one;
    }
    float matrix[12];
    Matrix_SetNegativeAxisAngleRotation(matrix, (char*)rotationAxis, *reinterpret_cast<float*>(ballPhysics + 0x30));
    Matrix_CopyAffine((char*)matrixBasis, matrix);
    *(WVector*)position = *(WVector*)(reinterpret_cast<const float*>(ballPhysics));
    // During the 30-frame post-event window, add small random XZ-only visual jitter.
    int lim;
    int idx = *reinterpret_cast<int*>(ballPhysics + 0x68);
    if (idx > 0) {
        lim = *reinterpret_cast<int*>(ballPhysics + 0x84);
        if (lim > 0) {
            if (idx >= lim) {
                if (idx < kBallJitterFrames + lim) {
                    float f = kBallPlaybackPositionJitterStep * static_cast<float>(::rand() % 11 - 5);

                    jitter[0] = static_cast<float>((::rand() % 11 - 5)) * kBallPlaybackPositionJitterStep;
                    jitter[1] = kFloat_0_Ball;
                    jitter[2] = f;
                    *(WVector*)position += *(WVector*)jitter;
                }
            }
        }
    }
    // Fade the visual sink depth over the recorded playback interval.
    if (groundSinkFactor > kFloat_0_Ball) {
        float ballRadius;
        float accel;
        float drop;
        float ratio;
        if (sinkStartFrame == ((CGolfBall*)GolfBall__Fv())->m_preDataCount) {
            ratio = kFloat_1_CourseHelpers;
        } else {
            float far = static_cast<float>(((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] - sinkStartFrame);
            far = kFloat_0_Ball < far ? far : kFloat_0_Ball;
            ratio = far / static_cast<float>(((CGolfBall*)GolfBall__Fv())->m_preDataCount - sinkStartFrame);
        }
        ballRadius = *reinterpret_cast<float*>(ballPhysics + 0xC0);
        accel = kFloat_2_Ball * ballRadius;
        drop = groundSinkFactor * accel;
        position[1] -= drop * ratio;
    }
    if (previousPlaybackFrame < ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[1]) {

        if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[1] <=
            (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0]))
            AttachFx(4);
    }
    previousPlaybackFrame = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0];
    char* global = reinterpret_cast<char*>(g_sharedDoc);
    char* mgr = *(char**)&AsCSharedDoc(global)->golfDoc;
    unsigned char slot = AsCGolfDoc(mgr)->currentPlayer;
    char* sentinel = base + 0x20;
    void* wanted = *reinterpret_cast<void**>(global + slot * 0x1D4 + 0x88);
    char* node = (char*)firstPuppetEntry;
    while (node != sentinel &&
           *reinterpret_cast<void**>(
               static_cast<char*>(*reinterpret_cast<void**>(*reinterpret_cast<void**>(node + 8))) + 0x50) != wanted)
        node = *reinterpret_cast<char**>(4 + node);
    char* data;
    if (node != base + 0x20)
        data = *reinterpret_cast<char**>(node + 8);
    else
        data = 0;
    if (data) {
        int* flags = reinterpret_cast<int*>(data + 8);
        int any = 0;
        if (flags[0])
            any = 1;
        else if (flags[1])
            any = 1;
        else if (flags[2])
            any = 1;
        else if (flags[3])
            any = 1;
        if (!any) {
            if (flags[4])
                any = 1;
            else if (flags[5])
                any = 1;
            else if (flags[6])
                any = 1;
            else if (flags[7])
                any = 1;
        }
        if (any)
            TransformBall();
    }
}

struct CBallVertexInit {
    char pad00[16];
    unsigned int diffuse;
    char pad14[4];
    float tu, tv;
    char pad20[24];
};
// Prepare ball/shadow vertices and enqueue named scene-resource preload requests.
void CBall::OnPreLoadInit() {
    using namespace AimAndBallModelViews;
    char* base = (char*)this;
    float vec[3] = {kFloat_1_CourseHelpers, kFloat_0_Ball, kFloat_0_Ball};
    (*(WVector*)&rotationAxis[0]) = *(WVector*)vec;
    for (int i = 0; i < 4; ++i) {
        reinterpret_cast<CBallVertexInit*>(base + 0xB0)[i].diffuse = 0x80FFFFFF;
        reinterpret_cast<CBallVertexInit*>(base + 0xB0)[i].tu = (i & 2) ? kFloat_1_CourseHelpers : kFloat_0_Ball;
        reinterpret_cast<CBallVertexInit*>(base + 0xB0)[i].tv = (i & 1) ? kFloat_0_Ball : kFloat_1_CourseHelpers;
    }
    vertexStreams[0] = base + 0xB0;
    vertexStreams[1] = base + 0xE8;
    vertexStreams[2] = base + 0x158;
    vertexStreams[3] = base + 0x120;
    unk_AC = 0;
    lightingMode = 2;
    bitmap =
        ResourceManager_LoadBitmap(g_resourceManager, reinterpret_cast<const char*>(golfBallResourceStrings), 0, 0, 0);
    ResourcePreloadQueue_AddRequest(
        g_sceneManager, 4, 0, kFloat_1_CourseHelpers, 0, kFloat_0_Ball,
        reinterpret_cast<const char*>(golfBallResourceStrings + kBallStringsBallObjectOffset), base + 0x30, 8, 0);
    if (base)
        base += 0x18;
    ResourcePreloadQueue_AddRequest(
        g_sceneManager, 1, 0, kFloat_1_CourseHelpers, base, kFloat_0_Ball,
        reinterpret_cast<const char*>(golfBallResourceStrings + kBallStringsBallShadowObjectOffset), 0, 0, 0);
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
struct CBallVT_ {
    virtual void s0();
    virtual void s1();
    virtual void s2();
    virtual void s3();
};
// Rebuild the ball's active scene resources and puppet state after a reload.
void CBall::OnReload() {
    using namespace AimAndBallModelViews;
    char* base = (char*)this;
    char* node;
    void* zero;
    char* sentinel;
    zero = 0;
    sentinel = (char*)&puppetListSentinel;
    node = (*(char**)&firstPuppetEntry);
    while (node != sentinel) {
        void* data = *reinterpret_cast<void**>(node + 8);
        if (data) {
            void* manager = g_resourceManager;
            if (manager) {
                void* sub = *reinterpret_cast<void**>(static_cast<char*>(data) + 4);
                if (sub) {
                    ResourceManager_ReleaseObject(manager, sub, 0);
                    *reinterpret_cast<void**>(static_cast<char*>(*reinterpret_cast<void**>(node + 8)) + 4) = zero;
                }
            }
            if (*reinterpret_cast<void**>(node + 8)) {
                __dl__FPv(*reinterpret_cast<void**>(node + 8));
                *reinterpret_cast<void**>(node + 8) = zero;
            }
        }
        node = *reinterpret_cast<char**>(node + 4);
    }
    // Puppet/payload release is finished; unlink and free the remaining list nodes.
    char* sentinel2 = (char*)&puppetListSentinel;
    char* current = (*(char**)&firstPuppetEntry);
    if (current != sentinel2) {
        char* head = *reinterpret_cast<char**>(sentinel2);
        *reinterpret_cast<void**>(*reinterpret_cast<char**>(current) + 4) = *reinterpret_cast<void**>(head + 4);
        *reinterpret_cast<void**>(*reinterpret_cast<char**>(head + 4)) = *reinterpret_cast<char**>(current);
        while (current != sentinel2) {
            char* old = current;
            current = *reinterpret_cast<char**>(current + 4);
            appFreeUni__FPv(old);
            --puppetEntryCount;
        }
    }
    DrawScene_SetNamedObjectPuppet(
        g_sceneManager, reinterpret_cast<const char*>(golfBallResourceStrings + kBallStringsBallObjectOffset), 0);
    ((CBallVT_*)this)->s3();
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

// Project ball +/- camera-up*radius through world->view then the view projection.
// Vertical screen diameter is clamped to [0.1,1.5]; model scale is
// 1.5/diameter * 1.2 * (1 + accessibility setting). Negative playback index starts
// from unit scale before the last two factors. Only the basis is scaled; translation stays.
// Scale the displayed ball using camera projection, simulated radius, and the selected settings.
void CBall::TransformBall() {
    using namespace AimAndBallModelViews;
    float amount;
    char* ballPhysics = static_cast<char*>(GolfBall__Fv());
    char* base = (char*)this;
    char projectedTop[0x38];
    char projectedBottom[0x38];
    char matrix[0x30];
    float viewBottom[3];
    float worldBottom[3];
    float radiusOffsetBottom[3];
    float viewTop[3];
    float worldTop[3];
    float radiusOffsetTop[3];

    if (*reinterpret_cast<int*>(ballPhysics + 0x68) < 0) {
        amount = kFloat_1_CourseHelpers;
    } else {
        __ml__FfRC7WVector(radiusOffsetBottom, s_cameraTransform + 3, ((CGolfBall*)GolfBall__Fv())->m_radius);
        __mi__FRC7WVectorRC7WVector(worldBottom, GolfBall__Fv(), radiusOffsetBottom);
        Matrix_TransformPoint(viewBottom, worldBottom, (char*)AsWView(g_view)->inputTransform);
        WView_ProjectViewPoint(g_view, projectedBottom, viewBottom, 0);
        float pf = ((CGolfBall*)GolfBall__Fv())->m_radius;
        __ml__FfRC7WVector(radiusOffsetTop, s_cameraTransform + 3, pf);
        __pl__FRC7WVectorRC7WVector(worldTop, GolfBall__Fv(), radiusOffsetTop);
        Matrix_TransformPoint(viewTop, worldTop, (char*)AsWView(g_view)->inputTransform);
        WView_ProjectViewPoint(g_view, projectedTop, viewTop, 0);
        float delta = *reinterpret_cast<float*>(projectedBottom + 4) - *reinterpret_cast<float*>(projectedTop + 4);
        delta = delta > kFloat_0_Ball ? delta : -delta;
        float scale;
        if (delta < kFloat_0_1_CourseHelpers)
            scale = kFloat_0_1_CourseHelpers;
        else if (delta > kFloat_1_5_Scene)
            scale = kFloat_1_5_Scene;
        else
            scale = delta;
        amount = kFloat_1_5_Scene / scale;
    }
    amount *= kFloat_1_2_CourseHelpers;
    void* accessibilitySettings = PartyItemSlots_GetInstance();
    amount *= kFloat_1_CourseHelpers + *reinterpret_cast<float*>(static_cast<char*>(accessibilitySettings) + 0x40);
    if (puppet) { // CBall_TransformBall: puppet at +0x28
        Matrix_CopyScaledBasis(matrix, base + 0x30, amount);
        Model_SetMatrixRecursive(puppet, 0, matrix);
        WPuppet_UpdateBoneWorldMatrices(puppet, 0, 0, kFloat_1_CourseHelpers);
        WPuppet_UpdateBoundsFromBone(puppet);
    }
}

} // extern "C"
} // namespace AimAndBallModelViews

// Initialize the ball actor, empty cache sentinel and default render fields.
inline CBall::CBall() {
    char* self = reinterpret_cast<char*>(this);
    AimAndBallModelViews::__ct__18NativeBase80201D18Fv(self);
    unsigned int zero = 0;
    void* head = (char*)&AsCBall(self)->puppetListSentinel;
    AsCBall(self)->vtbl = const_cast<unsigned char*>(AimAndBallModelViews::__vt__5CBall);
    AsCBall(self)->secondaryVtable = const_cast<unsigned char*>(AimAndBallModelViews::__vt__5CBall) + 0x3C;
    unsigned int mask = 0x00FFFFFF;
    AsCBall(self)->puppetEntryCount = zero;
    AsCBall(self)->firstPuppetEntry = head;
    AsCBall(self)->puppetListSentinel = head;
    (*(int*)&AsCBall(self)->puppet) = zero;
    (*(int*)&AsCBall(self)->bitmap) = zero;
    AsCBall(self)->lightingMode = zero;
    (*(int*)&AsCBall(self)->lightColor) = zero;
    (*(int*)&AsCBall(self)->ambientLightColor) = zero;
    AsCBall(self)->unk_90 = mask;
    AsCBall(self)->lightDirection[2] = AimAndBallModelViews::kFloat_0_Ball;
    AsCBall(self)->lightDirection[1] = AimAndBallModelViews::kFloat_0_Ball;
    AsCBall(self)->lightDirection[0] = AimAndBallModelViews::kFloat_0_Ball;
    AsCBall(self)->visible = 1;
    AsCBall(self)->previousPlaybackFrame = -1;
    AsCBall(self)->effectMessageValue = zero;
    AsCBall(self)->unk_8 = 1;
}
// Allocate and construct the visible ball actor.
void* CBall::Create() {
    return new CBall;
}

namespace AimAndBallModelViews {
extern "C" {

// Draw the visible ball puppet using its current lighting and transform.
void CBall::OnDisplay() {
    using namespace AimAndBallModelViews;
    struct Dispatch {
        virtual void f0() = 0;
        virtual void f1() = 0;
        virtual void f2() = 0;
        virtual void f5(void*, int, int) = 0;
    };
    if (visible) {
        TransformBall();
        if (puppet && WPuppet_GetBoneFlags(puppet, 0)) {
            char* state = static_cast<char*>(GolfBall__Fv());
            if (*reinterpret_cast<int*>(state + 0x18) || lightUpdatePending) {
                lightUpdatePending = 0;
                reinterpret_cast<Dispatch*>(puppet)->f5((char*)&lightingMode, 0, 0);
            }
        }
    }
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Release cached ball puppets/payloads, unlink their nodes, and release the shadow bitmap.
void CBall::OnDestroy() {
    using namespace AimAndBallModelViews;
    char* base = (char*)this;
    char* node;
    void* zero;
    char* sentinel;
    zero = 0;
    sentinel = (char*)&puppetListSentinel;
    node = (*(char**)&firstPuppetEntry);
    while (node != sentinel) {
        void* data = *reinterpret_cast<void**>(node + 0x8);
        if (data) {
            void* manager = g_resourceManager;
            if (manager) {
                void* sub = *reinterpret_cast<void**>(static_cast<char*>(data) + 0x4);
                if (sub) {
                    ResourceManager_ReleaseObject(manager, sub, 0);
                    *reinterpret_cast<void**>(static_cast<char*>(*reinterpret_cast<void**>(node + 0x8)) + 0x4) = zero;
                }
            }
            if (*reinterpret_cast<void**>(node + 0x8)) {
                __dl__FPv(*reinterpret_cast<void**>(node + 0x8));
                *reinterpret_cast<void**>(node + 0x8) = zero;
            }
        }
        node = *reinterpret_cast<char**>(node + 0x4);
    }
    // Payloads are released; splice the list back to its embedded sentinel and free nodes.
    char* first = (*(char**)&firstPuppetEntry);
    sentinel = (char*)&puppetListSentinel;
    if (first != sentinel) {
        char* last = *reinterpret_cast<char**>(first);
        char* head = *reinterpret_cast<char**>(sentinel);
        *reinterpret_cast<char**>(last + 0x4) = *reinterpret_cast<char**>(head + 0x4);
        *reinterpret_cast<char**>(*reinterpret_cast<char**>(head + 0x4)) = *reinterpret_cast<char**>(first);
        while (first != sentinel) {
            char* current = first;
            first = *reinterpret_cast<char**>(first + 0x4);
            appFreeUni__FPv(current);
            --puppetEntryCount;
        }
    }
    void* manager = g_resourceManager;
    if (manager) {
        void* resource = bitmap;
        if (resource) {
            ResourceManager_ReleaseId(manager, resource);
            bitmap = 0;
        }
    }
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

} // extern "C"

} // namespace AimAndBallModelViews

// ---- merged from ball_actor_and_ambient_sound.cpp ----
namespace ShadowAndEffectViews {

extern "C" {

extern void* g_view;
extern CSharedDoc* g_sharedDoc;
extern float kFloat_0_Ball, kFloat_1_CourseHelpers, kFloat_4_CourseHelpers, kFloat_1_2_CourseHelpers,
    kFloat_0_003_CourseHelpers, kFloat_neg1_CourseHelpers;
extern const float kIdentityMatrix4x4[16];

void* GolfBall__Fv();
void __mi__FRC7WVectorRC7WVector(void*, const void*, void*);
float Magnitude__7WVectorCFv(void*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, const void*, int, int*, int);
void Vector_CrossProduct(void*, const void*, const void*);
void Normalize__7WVectorFv(void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, const void*);
void __as__7WVectorFRC7WVector(void*, const void*);
void __ml__FfRC7WVector(void*, const void*, float);
void __apl__7WVectorFRC7WVector(void*, const void*);
void Vec3_CopyToOutput(void*, void*);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, const void*, int);
void Engine_SetTextureStageParameterList(int, const void*, int);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageResource(int, void*);
void SetTransform(int, const void*);
void DrawPrimitiveUP(int, int, const void*, int);
void* Camera_GetRenderState(void*);
void* fn_801CFoo(void*);

typedef Vec3f GroundVector;
struct GroundPosition {
    unsigned int x, y, z;
};
struct GroundVertex {
    GroundVector position;
    float u, v;
};
struct GroundConfig {
    unsigned int words[30];
};
struct GroundRegs {
    unsigned int words[12];
};
struct GroundTex {
    unsigned int words[10];
};
struct GroundDefaults {
    GroundConfig config;
    GroundRegs regs;
    GroundTex tex;
};
extern const GroundDefaults kBallGroundShadowRenderDefaults;
struct GroundDispatch {
    virtual void v0() = 0;
    virtual void v1() = 0;
    virtual void v2() = 0;
    virtual void v3() = 0;
    virtual void v4() = 0;
    virtual void v5() = 0;
    virtual void v6() = 0;
    virtual void v7() = 0;
    virtual void v8() = 0;
    virtual void v9() = 0;
    virtual void v10() = 0;
    virtual void v11() = 0;
    virtual int active() = 0;
};
struct GroundDevice {
    virtual void v0() = 0;
    virtual void v1() = 0;
    virtual void v2() = 0;
    virtual void v3() = 0;
    virtual void v4() = 0;
    virtual void v5() = 0;
    virtual void v6() = 0;
    virtual void v7() = 0;
    virtual void v8() = 0;
    virtual void v9() = 0;
    virtual void v10() = 0;
    virtual void v11() = 0;
    virtual void v12() = 0;
    virtual void v13() = 0;
    virtual void v14() = 0;
    virtual void v15() = 0;
    virtual void v16() = 0;
    virtual void v17() = 0;
    virtual void v18() = 0;
    virtual void v19() = 0;
    virtual void v20() = 0;
    virtual void v21() = 0;
    virtual void v22() = 0;
    virtual void v23() = 0;
    virtual void v24() = 0;
    virtual void v25() = 0;
    virtual void* state(void*) = 0;
};

// Suppress the shadow within four ball radii of the cup or without a ground face.
// Terrain normal builds tangent vectors a/b. World-space corners are groundPoint
// + (0,0.003,0) +/- a*half +/- b*half, where half=1.2*radius. UVs follow the
// corner bits. Identity world transforms prevent a second object-space transform;
// camera matrices supply the view/projection. The small offset reduces depth fighting.
// Build and draw the ground-projected shadow quad beneath the simulated ball.
void CBall::Display() {
    using namespace ShadowAndEffectViews;
    GroundVertex vertices[4];
    GroundConfig config;
    GroundRegs regs;
    GroundTex tex;
    GroundVector rect, matrix, data, box, a, b;
    GroundVertex* vertex;
    unsigned int y, x, i;
    float small, zero, maxDistance, half;
    const GroundDefaults* defaults = &kBallGroundShadowRenderDefaults;
    if (((GroundDispatch*)this)->active() || !visible)
        return;
    unsigned char* mgr = (unsigned char*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char index = AsCGolfDoc(mgr)->currentHoleIndex;
    void* item = AsCGolfDoc(mgr)->holeRecords;
    unsigned char* rec = (unsigned char*)item + index * 0x20;
    GroundPosition keydata = *(GroundPosition*)(rec + 0xC);
    void* layout = GolfBall__Fv();
    __mi__FRC7WVectorRC7WVector(&rect, &keydata, layout);
    rect.y = kFloat_0_Ball;
    maxDistance = ((CGolfBall*)GolfBall__Fv())->m_radius;
    float distance = (*(WVector*)&rect).Magnitude();
    if (distance < kFloat_4_CourseHelpers * maxDistance)
        return;
    half = kFloat_1_2_CourseHelpers * ((CGolfBall*)GolfBall__Fv())->m_radius;
    layout = GolfBall__Fv();
    GroundPosition pos = *(GroundPosition*)layout;
    GroundVector one, transformed, local, t1, t2;
    int found;
    void* resource = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&data, resource, &pos, 1, &found, 0);
    int face = found;
    if (face == -1)
        return;
    resource = CQuadTree_GetInstance();
    CQuadTree::GetMeshNormal(&box, resource, face);
    one.x = kFloat_0_Ball;
    one.y = kFloat_0_Ball;
    one.z = kFloat_1_CourseHelpers;
    Vector_CrossProduct(&a, &box, &one);
    Vector_CrossProduct(&b, &box, &a);
    (*(WVector*)&a).Normalize();
    (*(WVector*)&b).Normalize();
    zero = kFloat_0_Ball;
    small = kFloat_0_003_CourseHelpers;
    vertex = vertices;
    for (i = 0; i < 4; ++i) {
        local.x = zero;
        local.y = small;
        local.z = zero;
        __pl__FRC7WVectorRC7WVector(&transformed, &data, &local);
        *(WVector*)&matrix = *(WVector*)&transformed;
        x = i & 1;
        float sx = x ? kFloat_1_CourseHelpers : kFloat_neg1_CourseHelpers;
        __ml__FfRC7WVector(&t1, &a, half * sx);
        *(WVector*)&matrix += *(WVector*)&t1;
        y = i & 2;
        float sy = y ? kFloat_1_CourseHelpers : kFloat_neg1_CourseHelpers;
        __ml__FfRC7WVector(&t2, &b, half * sy);
        *(WVector*)&matrix += *(WVector*)&t2;
        Vec3_CopyToOutput(&matrix, vertex);
        vertex->u = y ? kFloat_1_CourseHelpers : kFloat_0_Ball;
        vertex->v = x ? kFloat_1_CourseHelpers : kFloat_0_Ball;
        ++vertex;
    }
    config = defaults->config;
    regs = defaults->regs;
    tex = defaults->tex;
    Engine_SetRenderStateList(&config, 15);
    Engine_SetTextureStageStateList(0, &regs, 6);
    Engine_SetTextureStageParameterList(0, &tex, 5);
    Engine_SetFlexibleVertexFormat(kBallShadowVertexFormat);
    Engine_SetType7Resource(0);
    void* obj = AsWView(g_view)->renderTarget;
    void* state = ((GroundDevice*)obj)->state(bitmap);
    Engine_SetTextureStageResource(0, state);
    SetTransform(11, kIdentityMatrix4x4);
    SetTransform(3, kIdentityMatrix4x4);
    obj = Camera_GetRenderState(g_view);
    SetTransform(1, ((BallShadowCameraMatrices*)obj)->transform1);
    SetTransform(0, ((BallShadowCameraMatrices*)obj)->transform0);
    DrawPrimitiveUP(kBallShadowTriangleStrip, 2, vertices, sizeof(GroundVertex));
}

extern unsigned char s_ballTypeInfo;
extern unsigned char s_actorTypeInfo;
extern float s_twoPi_800CBF34_805E2940;
extern const char s_ballActorFactoryKey;
extern const char s_ballTaskRegistryKey;
extern const float kFloat_6_2831855;
extern const char golfBallResourceStrings[];

// Return the visible ball actor's runtime type descriptor.
void* CBall::GetRTTI() {
    return &s_ballTypeInfo;
}

// Adjust the secondary-interface receiver before drawing the ball shadow.
void CBall::DrawGroundShadowAdjustedThis() {
    ((CBall*)((char*)this - 0x18))->Display();
}

void* IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void* StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void* TaskRegistry_AppendFactory(void*, const char*);
void appFreeUni__FPv(void*);
void __dt__18NativeBase80201D18Fv(void*, int);
void __dl__FPv(void*);

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;
    int key;
};
struct TreeRoot {
    char pad[4];
    TreeNode* head;
};

struct BallCacheSentinel {
    BallCacheLink* previous;
    BallCacheLink* next;
};
struct BallCacheOwnerPrefix {
    char pad[0x1c];
    int count;
    BallCacheSentinel sentinel;
};

struct Entry {
    char pad[0x50];
    void* values[4][7];
};

extern CSharedDoc* g_sharedDoc;
extern void* g_fx;
extern const float kFloat_4_BeachWaves, kFloat_0_VectorMath, kFloat_255_BeachWaves;
extern float kFloat_0_5_BallSound, kFloat_200_CourseHelpers_805DDD5C, kFloat_1_CourseHelpers;
extern void* g_resourceManager;
extern const char waterResourceParseStrings[];
extern float kFloat_38_4_CourseHelpers, kFloat_76_8_CourseHelpers, kFloat_7_5_CourseHelpers,
    kFloat_0_26666668_CourseHelpers, kFloat_0_53333336_CourseHelpers, kFloat_0_2_CourseHelpers, kFloat_0_5_BeachWaves;
extern void* BeachParameterTree_FindInsertionPosition(void*, int*, TreeNode**, unsigned char*, unsigned char*);
extern void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern int SharedDoc_GetCourseId();
extern void* Payload44Tree_InsertAndRebalance(void*, int*, unsigned char, unsigned char, void*);

struct BeachValue {
    int a, b, c, d, e, f, g, h, i, j;
};
struct BeachPair {
    BeachValue zero;
    int key;
    BeachValue value;
    int padding;
    // Build a zero-valued beach-parameter entry for insertion.
    BeachPair(int k) {
        zero.a = 0;
        zero.b = 0;
        zero.c = 0;
        zero.d = 0;
        zero.e = 0;
        zero.f = 0;
        zero.g = 0;
        zero.h = 0;
        zero.i = 0;
        zero.j = 0;
        key = k;
        value.a = 0;
        value.b = 0;
        value.c = 0;
        value.d = 0;
        value.e = 0;
        value.f = 0;
        value.g = 0;
        value.h = 0;
        value.i = 0;
        value.j = 0;
    }
};
// Tree payload: integer key, 32-byte bitmap name, wave height, then duration.
struct BeachParameterPayload {
    int key;
    char bitmapName[32];
    float height;
    float duration;
};
typedef char BeachPayload_height_offset[((int)&((BeachParameterPayload*)0)->height == 0x24) ? 1 : -1];
typedef char BeachPayload_duration_offset[((int)&((BeachParameterPayload*)0)->duration == 0x28) ? 1 : -1];

// name's suffix after '_' selects an integer-keyed parameter record. Missing
// entries are inserted with zero payloads. Otherwise use the default bitmap and
// course/hole-specific height. Height uses world units; duration shares dt's clock.
// Resolve named/default beach-wave parameters and derive gravity and timing values.
// Output schema at self+10: bitmap handle +10, travel amplitude H +14 (world
// units along the strip's motion vector), duration T +18 (task seconds), gravity
// coefficient g +1C, rise R +20, fall F +24, launch delay W +28 (all seconds).
// R=(4/15)*T, F=(8/15)*T, W=(3/15)*T; g=H/(4*R-(0.5*R)*R).
// Default T=7.5 -> R=2,F=4,W=1.5; H=38.4 or 76.8 -> g=6.4 or 12.8.
// The literal 4 in launch velocity is a FOUR-SECOND coefficient: v0=4*g;
// integrating v-=g*dt then x+=dt*v uses world units/s and world units/s^2.
// H describes this motion excursion, not vertical water elevation. Increasing H
// at fixed T scales g/v/x; increasing T stretches fades AND changes g, so duration
// and amplitude must be tuned together. Denominator vanishes at R=0 or R=8;
// positive H with 0<R<8 gives positive g. Other safe ranges are unestablished.
// Missing keyed entries are zero-filled (including bitmap name/H/T), not replaced
// with defaults: such entries violate the positive timing contract. Null name
// returns without writing anything. No validation/rollback is added here.
void CBeach::LoadWaveParameters(const char* name) {
    using namespace ShadowAndEffectViews;
    if (!name)
        return;
    float* out = (float*)((char*)&bitmap);
    int pos;
    if (strchr(name, '_') && sscanf(strchr(name, '_') + 1, waterResourceParseStrings, &pos)) {
        // Resolve/load the bitmap from the keyed parameter record.
        {
            int key = pos;
            void* root =
                &AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->beachParameterTreeCount; // keyed beach-wave parameters
            TreeNode* node;
            unsigned char left, right;
            int* last = (int*)BeachParameterTree_FindInsertionPosition(root, &key, &node, &left, &right);
            int* payload = (int*)node;
            if (!node || *(payload = &node->key) < key) {
                BeachPair data(key);
                void* result = Payload44Tree_InsertAndRebalance(root, last, left, right, &data.key);
                payload = &((TreeNode*)result)->key;
            }
            *(void**)out =
                ResourceManager_LoadBitmap(g_resourceManager, ((BeachParameterPayload*)payload)->bitmapName, 0, 0, 0);
        }
        // Resolve the same record's maximum displacement.
        {
            int key = pos;
            void* root =
                &AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->beachParameterTreeCount; // keyed beach-wave parameters
            TreeNode* node;
            unsigned char left, right;
            int* last = (int*)BeachParameterTree_FindInsertionPosition(root, &key, &node, &left, &right);
            int* payload = (int*)node;
            if (!node || *(payload = &node->key) < key) {
                BeachPair data(key);
                void* result = Payload44Tree_InsertAndRebalance(root, last, left, right, &data.key);
                payload = &((TreeNode*)result)->key;
            }
            out[1] = ((BeachParameterPayload*)payload)->height;
        }
        // Resolve the same record's total wave duration.
        {
            int key = pos;
            void* root =
                &AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->beachParameterTreeCount; // keyed beach-wave parameters
            TreeNode* node;
            unsigned char left, right;
            int* last = (int*)BeachParameterTree_FindInsertionPosition(root, &key, &node, &left, &right);
            int* payload = (int*)node;
            if (!node || *(payload = &node->key) < key) {
                BeachPair data(key);
                void* result = Payload44Tree_InsertAndRebalance(root, last, left, right, &data.key);
                payload = &((TreeNode*)result)->key;
            }
            out[2] = ((BeachParameterPayload*)payload)->duration;
        }
    } else {
        *(void**)out = ResourceManager_LoadBitmap(g_resourceManager, (char*)waterResourceParseStrings + 3, 0, 0, 0);
        if (((unsigned char)SharedDoc_GetCourseId() == 0 &&
             *(unsigned char*)((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x3B7) == 2) ||
            ((unsigned char)SharedDoc_GetCourseId() == 1 &&
             *(unsigned char*)((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x3B7) == 2) ||
            ((unsigned char)SharedDoc_GetCourseId() == 6 &&
             *(unsigned char*)((*(char**)&AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x3B7) == 10))
            out[1] = kFloat_38_4_CourseHelpers;
        else
            out[1] = kFloat_76_8_CourseHelpers;
        out[2] = kFloat_7_5_CourseHelpers;
    }
    // Split duration into rise/fall/wait fractions (4/15, 8/15, 3/15), then
    // derive the acceleration coefficient used by the existing velocity integrator.
    float b, a, half, c, denom, length;
    a = kFloat_0_26666668_CourseHelpers * out[2];
    length = out[2];
    b = kFloat_0_53333336_CourseHelpers * length;
    half = kFloat_0_5_BeachWaves * a;
    c = kFloat_0_2_CourseHelpers * length;
    denom = kFloat_4_BeachWaves * a - half * a;
    out[4] = a;
    out[5] = b;
    out[6] = c;
    out[3] = out[1] / denom;
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
struct BeachWave {
    float x, delay, alpha, time, velocity;
    int state;
};
struct BeachWaves {
    char pad[0x1C];
    float gravity, rise, fall, wait;
    BeachWave* waves;
    unsigned int count;
};
// Wave alpha uses [0,255]; x/velocity are displacement and its rate in the wave
// mesh's local motion axis. Durations must be positive, and count must fit the
// byte loop index. Waiting restarts only when a positive delay becomes NEGATIVE;
// exact zero does not restart. Rise completion schedules the next strip modulo count.
// Advance wave rise/fade/wait phases and trigger the next strip's delayed restart.
// Clock chain: CTask_ProcessActors (task_manager_and_actors.cpp) -> DrawScene_UpdateRegisteredObjects
// (draw_scene_named_effect_list.cpp) -> the beach object's +0C hook, passing task delta unchanged.
// dt is elapsed task seconds, finite/nonnegative; rise/fall/wait are seconds and
// gravity is the derived coefficient documented at CBeach_LoadWaveParameters. This is not
// real-world gravity. Each 0x18-byte record: +00 x world-distance excursion,
// +04 delay seconds, +08 alpha float channel units, +0C accumulated seconds,
// +10 velocity world units/s, +14 state (rising/falling/waiting enum).
// Required waves allocation covers count records; count must be 0..255, since
// the loop counter is u8 (256 would wrap). Empty count performs no access; modulo
// count is reached only inside a nonempty iteration. No catch-up/substeps: even
// a large dt takes at most one phase transition per strip. A successor later in
// array order can consume its newly assigned delay in this SAME update.
// Falling may leave alpha negative; rendering skips waiting strips. Preserve
// strict crossings and velocity-before-position integration when changing timing.
void CBeach::UpdateWaves(float dt) {
    using namespace ShadowAndEffectViews;
    float value;
    const float& four = kFloat_4_BeachWaves;
    unsigned char i = 0;
    const float& zero = kFloat_0_VectorMath;
    const float& max = kFloat_255_BeachWaves;
    for (; i < stripCount; ++i) {
        unsigned int index = i;
        value = ((BeachWave*)stripParameters)[index].time;
        ((BeachWave*)stripParameters)[index].time = value + dt;
        switch (((BeachWave*)stripParameters)[i].state) {
        case kBeachWaveRising: {
            ((BeachWave*)stripParameters)[i].alpha = (max / riseSeconds) * dt + ((BeachWave*)stripParameters)[i].alpha;
            ((BeachWave*)stripParameters)[i].velocity -= gravity * dt;
            BeachWave& moving = ((BeachWave*)stripParameters)[i];
            value = dt * moving.velocity + moving.x;
            moving.x = value;
            if (((BeachWave*)stripParameters)[i].alpha > max) {
                ((BeachWave*)stripParameters)[i].alpha = max;
                ((BeachWave*)stripParameters)[i].state = kBeachWaveFalling;
                ((BeachWave*)stripParameters)[(index + 1U) % stripCount].delay = waitSeconds;
            }
            break;
        }
        case kBeachWaveFalling: {
            ((BeachWave*)stripParameters)[i].alpha -= (max / fallSeconds) * dt;
            ((BeachWave*)stripParameters)[i].velocity -= gravity * dt;
            BeachWave& moving = ((BeachWave*)stripParameters)[i];
            value = dt * moving.velocity + moving.x;
            moving.x = value;
            if (((BeachWave*)stripParameters)[i].alpha < zero)
                ((BeachWave*)stripParameters)[i].state = kBeachWaveWaiting;
            break;
        }
        case kBeachWaveWaiting: {
            if (((BeachWave*)stripParameters)[i].delay > zero) {
                value = ((BeachWave*)stripParameters)[i].delay - dt;
                ((BeachWave*)stripParameters)[i].delay = value;
                if (((BeachWave*)stripParameters)[i].delay < zero) {
                    ((BeachWave*)stripParameters)[i].state = kBeachWaveRising;
                    ((BeachWave*)stripParameters)[i].alpha = zero;
                    ((BeachWave*)stripParameters)[i].time = zero;
                    ((BeachWave*)stripParameters)[i].x = zero;
                    ((BeachWave*)stripParameters)[i].velocity = four * gravity;
                }
            }
            break;
        }
        default:
            break;
        }
    }
}
#pragma opt_strength_reduction reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

extern void FxAttachment_Init(void*);
extern void* WPuppet_FindOrCacheBone(void*, const char*);
extern void* WPuppet_FindNamedBoneEntry(void*, const char*);
extern int CString_Compare(const char*, const char*);
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern void* g_soundManager;

// Preserve the separate indexed name checks and fallback address calculations.
#pragma opt_common_subs off
// kind indexes one of seven filename/bone entries in the IFF ball record.
// Only kinds 0,1,3,5 map to retained slots 0,1,2,3; kinds 2,4,6 are created
// without a cached removal handle. Spray/sequence type is determined by filename.
// Bone lookup falls back to the center bone, which must exist for attachment.
// Resolve an IFF ball effect and its attachment bone, then cache the spray/sequence handle.
void CBall::AttachFx(int kind) {
    using namespace ShadowAndEffectViews;
    if (kind < 0 || kind >= kBallEffectKindCount)
        return;
    void* game = g_sharedDoc;
    char* sentinel = (char*)&puppetListSentinel;
    char* node = (*(char**)&firstPuppetEntry);
    void* manager = (*(void**)&AsCSharedDoc(game)->golfDoc);
    unsigned char index = AsCGolfDoc(manager)->currentPlayer;
    char* slot = (char*)game + index * 0x1D4;
    void* wanted = *(void**)(slot + 0x88);
    for (; node != sentinel && *(void**)((char*)**(void***)(node + 8) + 0x50) != wanted; node = *(char**)(node + 4)) {
    }
    void* entry;
    if ((unsigned int)node != (unsigned int)this + 0x20)
        entry = *(void**)(node + 8);
    else
        entry = 0;
    if (!entry)
        return;
    void* object = *(void**)entry;
    if (!object)
        return;
    int offset = kind * 0x28;
    signed char* nameBase = (signed char*)((IFF_STRUCT::sBall*)object)->effectFiles;
    if (!nameBase[offset])
        return;
    int nameLen = strlen((char*)(nameBase + offset));
    if (nameLen < 5)
        return;
    char local[0x28];
    FxAttachment_Init(local);
    void* found = 0;
    signed char* anim = (signed char*)((IFF_STRUCT::sBall*)*(void**)entry)->effectBones;
    if (anim[offset])
        found = WPuppet_FindOrCacheBone(puppet, (char*)(anim + offset));
    if (found)
        *(void**)(local + 8) = ((BallBoneTransform*)found)->attachmentMatrix;
    else {
        signed char* anim2 = (signed char*)((IFF_STRUCT::sBall*)*(void**)entry)->effectBones;
        if (anim2[offset])
            found = WPuppet_FindNamedBoneEntry(puppet, (char*)(anim2 + offset));
        else
            found = WPuppet_FindNamedBoneEntry(puppet, (char*)golfBallResourceStrings + kBallStringsCenterBoneOffset);
        if (found) {
            if (*(void**)found)
                *(void**)(local + 8) = (char*)*(void**)found + 0x78;
            *(void**)(local + 0x18) = (char*)*(void**)((char*)found + 4) + 0x18;
        } else {
            found = WPuppet_FindNamedBoneEntry(puppet, (char*)golfBallResourceStrings + kBallStringsCenterBoneOffset);
            if (*(void**)found)
                *(void**)(local + 8) = (char*)*(void**)found + 0x78;
            *(void**)(local + 0x18) = (char*)*(void**)((char*)found + 4) + 0x18;
        }
    }
    // Compress persistent effect kinds to the four spray/sequence cache slots.
    int pos;
    switch (kind) {
    case 0:
        pos = 0;
        break;
    case 1:
        pos = 1;
        break;
    case 3:
        pos = 2;
        break;
    case 5:
        pos = 3;
        break;
    default:
        pos = -1;
        break;
    }
    void* result =
        ((CFx*)g_fx)->CreateAttachedEffect((Vec2_8010*)(local), (char*)(*(int*)entry + offset + 0x5C), 1, 0, 0);
    if (pos == -1)
        return;
    if (!CString_Compare((char*)(*(int*)entry + offset + 0x5C),
                         (char*)golfBallResourceStrings + kBallStringsSpecialEffectOffset))
        Sound_PlayByName(g_soundManager, (char*)golfBallResourceStrings + kBallStringsSpecialEffectSoundOffset, 0, 0, 0,
                         0, kFloat_0_5_BallSound, kFloat_200_CourseHelpers_805DDD5C, kFloat_1_CourseHelpers);
    int base = (int)*(void**)entry;
    base += offset;
    char* end = (char*)(base + nameLen);
    if (!((int (*)(const char*, const char*, const char*))CString_Compare)(
            end + 0x59, (char*)golfBallResourceStrings + kBallStringsSprayExtensionOffset, end))
        *(void**)((char*)entry + 8 + pos * 4) = (void*)result;
    else
        ((BallPuppetCacheEntry*)entry)->sequences[pos] = (void*)result;
}
#pragma opt_common_subs reset

// out is the predecessor (greatest encountered key <= requested key). left
// chooses the insertion link; right is actually the no-predecessor flag, cleared
// on right descent. All output pointers are borrowed and must be writable.
// Find the integer-key tree insertion position used for beach parameter entries.
void* BeachParameterTree_FindInsertionPosition(void* root, int* key, TreeNode** out, unsigned char* left,
                                               unsigned char* right) {
    TreeRoot* r = (TreeRoot*)root;
    *out = 0;
    TreeNode* last = (TreeNode*)&r->head;
    TreeNode* node = r->head;
    *left = 1;
    *right = 1;
    while (node) {
        int k = *key;
        last = node;
        if (k < node->key) {
            node = node->left;
            *left = 1;
        } else {
            *out = node;
            node = node->right;
            *left = 0;
            *right = 0;
        }
    }
    return last;
}

// Lifecycle precondition: OnDestroy releases payloads/resources first. This final
// destructor frees list-node allocations only; it does not walk effect handles.
// Destroy the ball actor's cached-entry list and native base, optionally freeing the actor.
// Kept as a named member instead of ~CBall: a real destructor changes the generated code.
void* CBall::Destroy(int del) {
    using namespace ShadowAndEffectViews;
    if (this) {
        if (&puppetEntryCount && &puppetEntryCount && &puppetEntryCount && &puppetEntryCount) {
            BallCacheLink* end = (BallCacheLink*)&puppetListSentinel;
            BallCacheLink* it = (*(BallCacheLink**)&firstPuppetEntry);
            if (it != end) {
                BallCacheLink* lastNode = end->previous;
                BallCacheLink* firstLeft = it->previous;
                firstLeft->next = lastNode->next;
                BallCacheLink* tmp = it->previous;
                BallCacheLink* y = lastNode->next;
                y->previous = tmp;
                while (it != end) {
                    BallCacheLink* cur = it;
                    it = it->next;
                    appFreeUni__FPv(cur);
                    --puppetEntryCount;
                }
            }
        }
        __dt__18NativeBase80201D18Fv(this, 0);
        if (del > 0)
            __dl__FPv(this);
    }
    return this;
}

#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_propagation off
// Remove the requested effect from the active ball entry and clear its cached handle.
void CBall::DetachFx(int kind) {
    using namespace ShadowAndEffectViews;
    register void* entry;
    register int offset;
    register int pos;
    if (kind != 0 && kind != 1 && kind != 3 && kind != 5)
        return;
    void* manager;
    unsigned char* game;
    game = (unsigned char*)g_sharedDoc;
    manager = (*(void**)&AsCSharedDoc(game)->golfDoc);
    unsigned char index = AsCGolfDoc(manager)->currentPlayer;
    void* object;
    void* wanted = *(void**)(game + index * 0x1D4 + 0x88);
    BallCacheLink* sentinel = (BallCacheLink*)((char*)&puppetListSentinel);
    BallCacheLink* it = (*(BallCacheLink**)&firstPuppetEntry);
    // 0x800CBF34 unit: find the puppet associated with the wanted ball.
    void* inner;
    while (it != sentinel && (object = (void*)it->payload, inner = ((BallPuppetCacheEntry*)object)->record,
                              (void*)((IFF_STRUCT::sBall*)inner)->common.typeId != wanted)) {
        it = it->next;
    }
    if (it != (BallCacheLink*)((char*)&puppetListSentinel))
        entry = (void*)it->payload;
    else
        entry = 0;
    if (!entry)
        return;
    switch (kind) {
    case 0:
        pos = 0;
        break;
    case 1:
        pos = 1;
        break;
    case 3:
        pos = 2;
        break;
    case 5:
        pos = 3;
        break;
    default:
        pos = -1;
        break;
    }
    if (pos == -1)
        return;
    offset = pos << 2;
    register void* value = *(void**)((char*)entry + offset + 8);
    if (value) {
        ((CFx*)g_fx)->RemoveSpray((unsigned int)(value));
        *(void**)((char*)entry + offset + 8) = 0;
    }
    entry = (char*)entry + offset;
    value = ((BallPuppetCacheEntry*)entry)->sequences[0];
    if (value) {
        if (pos == 0)
            ((CFx*)g_fx)->RemoveSequence((unsigned int)(value), 1);
        else
            ((CFx*)g_fx)->RemoveSequence((unsigned int)(value), 0);
        ((BallPuppetCacheEntry*)entry)->sequences[0] = 0;
    }
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off

} // extern "C"

} // namespace ShadowAndEffectViews

// Register the visible ball actor's RTTI and factory.
void CBall::RegisterType() {
    using namespace ShadowAndEffectViews;
    const char* base = golfBallResourceStrings;
    volatile float angle = kFloat_6_2831855;
    s_twoPi_800CBF34_805E2940 = angle;
    IntPair_Set(&ShadowAndEffectViews::s_ballTypeInfo, base + 0x45, &s_actorTypeInfo);
    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), (void*)CBall::Create, &s_ballActorFactoryKey);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), &s_ballTaskRegistryKey);
}

namespace ShadowAndEffectViews {
extern "C" {
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset

extern void* g_currentTimeMilliseconds;
extern void* g_sceneManager;
extern volatile const float s_ballAmbientLightScale;
extern void DrawScene_SetNamedObjectEnabled(void*, const char*, unsigned char);
extern void* GetGroundType__9CQuadTreeF7WVector(void*, const void*);
extern void* Color_ScaleRGB(void*, float);
extern void* PreData__9CGolfBallFi(void*, void*);
extern void* ItemInventory_FindBallData(void*, const char*);
extern void* __nw__FUl(unsigned int);
extern void* memset(void*, int, unsigned int);
extern void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
extern void* appMallocDummy__FUl(unsigned int);
extern void DrawScene_SetNamedObjectPuppet(void*, const char*, void*);
extern void* g_resourceManager;
extern void appFreeUni__FPv(void*);

struct BallNodeGuard {
    void* allocator;
    BallCacheLink* volatile end;
    void* node;
    // Guard the newly allocated ball-cache node before linking it.
    BallNodeGuard(void* p, BallCacheLink* e) : end(e), node(p) {}
    // Free an unlinked node on exceptional exit.
    ~BallNodeGuard() {
        if (node)
            appFreeUni__FPv(node);
    }
};
struct BallPosition {
    unsigned int x, y, z;
};
struct BallDispatch {
    virtual void v0() = 0;
    virtual void v1() = 0;
    virtual void v2() = 0;
    virtual void v3() = 0;
    virtual void v4() = 0;
    virtual void v5() = 0;
    virtual void v6() = 0;
    virtual void v7() = 0;
    virtual void v8() = 0;
    virtual void v9() = 0;
    virtual void reset() = 0;
};
struct BallPuppet {
    virtual void v0() = 0;
    virtual void v1() = 0;
    virtual void v2() = 0;
    virtual void setLight(void*, int, int) = 0;
};

// Refresh ball diffuse/ambient lighting from its live terrain position.
static inline void Ball_RefreshTerrainLighting(void* self, void*& liveBall) {
    AsCBall(self)->lightUpdatePending = 1;
    liveBall = GolfBall__Fv();
    void* terrain = CQuadTree_GetInstance();
    extern void* volatile s_sceneLighting[];
    float factor = ((CQuadTree*)terrain)->DefaultLightingFactor(liveBall);
    AsCBall(self)->lightColor = Color_ScaleRGB(s_sceneLighting[4], factor);
    AsCBall(self)->ambientLightColor = Color_ScaleRGB(s_sceneLighting[5], s_ballAmbientLightScale);
}

// Clear sink on firm terrain; derive its factor from the live ball's lower lie scale on codes 3/4/5.
// Position/ball references are borrowed caller scratch, retained only to preserve CW allocation.
static inline void Ball_RefreshLiveGroundSink(void* self, BallPosition& key, void*& liveBall) {

    switch (((unsigned char)GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &key))) {
    case 0:
    case 1:
    case 2:
    case 6:
    case 8:
        AsCBall(self)->groundSinkFactor = kFloat_0_Ball;
        break;
    case 3:
    case 4:
    case 5: {
        liveBall = GolfBall__Fv();
        void* terrain = CQuadTree_GetInstance();
        AsCBall(self)->groundSinkFactor =
            kFloat_1_CourseHelpers - ((CQuadTree*)terrain)->GetMinPowerOnGroundAtPoint(liveBall);
        break;
    }
    }
}

// Advance through the borrowed circular cache until its sentinel or an IFF-ID match.
// The sentinel has no payload; the short-circuit test must remain in this order.
static inline void findBallPuppetCacheEntry(BallCacheLink*& cursor, BallCacheLink* sentinel,
                                            volatile unsigned int& wantedItemId) {
    while (cursor != sentinel && cursor->payload->record->common.typeId != wantedItemId)
        cursor = cursor->next;
}

// Select the cached puppet, apply actor lighting, bind the scene entry and recreate idle effects.
// Cache and resource ownership remain with the ball actor/resource manager.
static inline void activateCachedBallPuppet(void* self, void*& entry) {
    void* puppet = ((BallPuppetCacheEntry*)entry)->puppet;
    AsCBall(self)->puppet = puppet;
    ((BallPuppet*)puppet)->setLight((char*)&AsCBall(self)->lightingMode, 0, 0);
    DrawScene_SetNamedObjectPuppet(g_sceneManager, golfBallResourceStrings + kBallStringsBallObjectOffset,
                                   AsCBall(self)->puppet);
    ((CBall*)self)->AttachFx(0);
    ((CBall*)self)->AttachFx(1);
}

// msg is borrowed; time is compared as an unsigned millisecond timestamp without
// wrap handling. Future messages call reset and return. Action 5 value is a ball
// IFF type ID, action 9 stores an opaque value; other actions ignore that word.
// Handle visibility, lighting, sinking, equipment switching, and effect messages; reset on future timestamps.
#pragma push
#pragma always_inline on
// Transfer a constructed payload to the actor cache after guarding and linking its node.
// The caller owns payload until this succeeds; allocation is assumed to succeed.
// Resource-manager ownership of payload->puppet is released by CBall_OnDestroy.
static inline void LinkBallPuppetCachePayload(void* self, void* newCachePayload, void* volatile& allocatedNodeAlias,
                                              void* volatile& allocatedNodeAliasCopy) {
    BallCacheLink* insertionSentinel = (BallCacheLink*)((unsigned int)self + (int)&((CBall*)0)->puppetListSentinel);
    void* newLinkNode = appMallocDummy__FUl(sizeof(BallCacheLink));
    allocatedNodeAlias = newLinkNode;
    allocatedNodeAliasCopy = newLinkNode;
    BallNodeGuard newLinkGuard(newLinkNode, insertionSentinel);
    void** payloadStorage = (void**)((char*)newLinkNode + 8);
    if (payloadStorage) {
        try {
            *payloadStorage = newCachePayload;
        } catch (...) {
            throw;
        }
    }
    newLinkGuard.node = 0;
    BallCacheLink* previousTail = insertionSentinel->previous;
    previousTail->next = (BallCacheLink*)newLinkNode;
    ((BallCacheLink*)newLinkNode)->previous = insertionSentinel->previous;
    insertionSentinel->previous = (BallCacheLink*)newLinkNode;
    ((BallCacheLink*)newLinkNode)->next = insertionSentinel;
    ++AsCBall(self)->puppetEntryCount;
}
#pragma pop

// Dispatch ball presentation/equipment messages, retaining actor ownership of cached puppets.
void CBall::HandleMsg(void* msg) {
    using namespace ShadowAndEffectViews;
    void* cachePayload;
    unsigned int requestedBallItemId;
    if (((BallMessage*)msg)->time > *(unsigned int*)&g_currentTimeMilliseconds) {
        ((BallDispatch*)this)->reset();
        return;
    }
    switch (((BallMessage*)msg)->action) {
    // Show the puppet and scene object.
    case kBallShow:
        unsigned char visible;
        visible = this->visible = 1;
        DrawScene_SetNamedObjectEnabled(g_sceneManager, (char*)golfBallResourceStrings + kBallStringsBallObjectOffset,
                                        visible);
        break;
    // Hide the puppet and scene object.
    case kBallHide:
        visible = this->visible = 0;
        DrawScene_SetNamedObjectEnabled(g_sceneManager, (char*)golfBallResourceStrings + kBallStringsBallObjectOffset,
                                        visible);
        break;
    // Refresh terrain-derived diffuse and ambient light.
    case kBallRefreshLighting: {
        void* liveBall;
        Ball_RefreshTerrainLighting(this, liveBall);
        break;
    }
    // Update sinking from the live ball position.
    case kBallRefreshGroundSink: {
        void* ballForPosition = GolfBall__Fv();
        BallPosition liveBallPosition = *(BallPosition*)ballForPosition;
        void* liveBall;
        Ball_RefreshLiveGroundSink(this, liveBallPosition, liveBall);
        break;
    }
    // Prepare sinking from the final retained trajectory sample.
    case kBallPrepareSinkPlayback: {
        void* ballWithSampleCount = GolfBall__Fv();
        void* finalHistorySample = GolfBall__Fv();
        finalHistorySample =
            PreData__9CGolfBallFi(finalHistorySample, (void*)((CGolfBall*)ballWithSampleCount)->m_preDataCount);
        BallPosition finalSamplePosition = *(BallPosition*)finalHistorySample;

        switch (((unsigned char)GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &finalSamplePosition))) {
        case 0:
        case 1:
        case 2:
        case 6:
        case 8:
            float zeroGroundSinkFactor = kFloat_0_Ball;
            float zeroGroundSinkFactorCopy = zeroGroundSinkFactor;
            this->sinkStartFrame = -1;
            this->groundSinkFactor = zeroGroundSinkFactorCopy;
            break;
        case 3:
        case 4:
        case 5: {
            void* ballWithSampleCount = GolfBall__Fv();
            void* ballForSampleLookup = GolfBall__Fv();
            void* sinkHistorySample =
                PreData__9CGolfBallFi(ballForSampleLookup, (void*)((CGolfBall*)ballWithSampleCount)->m_preDataCount);
            this->groundSinkFactor =
                kFloat_1_CourseHelpers -
                ((CQuadTree*)CQuadTree_GetInstance())->GetMinPowerOnGroundAtPoint(sinkHistorySample);
            int anchorSampleIndex = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kBallSinkAnchorEventSlot];
            int sinkStartSampleIndex = ((CGolfBall*)GolfBall__Fv())->m_preDataCount - kBallSinkLeadFrames;
            if (sinkStartSampleIndex < anchorSampleIndex)
                sinkStartSampleIndex = anchorSampleIndex;
            this->sinkStartFrame = sinkStartSampleIndex;
            break;
        }
        }
        break;
    }
    // Equipment switch: remove idle effects, find/create a cache entry, publish
    // the player's equipped ID and scene puppet, then recreate idle effects.
    // Load/select a cached puppet for the requested equipped ball ID.
    case kBallSelectEquipment: {
        void* volatile allocatedNodeAliasCopy;
        void* volatile allocatedNodeAlias;
        BallCacheLink* volatile lookupAliasCopy;
        BallCacheLink* volatile lookupAliasResult;
        volatile unsigned int lookupBallItemId;
        volatile unsigned int savedBallItemId;
        requestedBallItemId = ((BallMessage*)msg)->value;
        ((CBall*)this)->DetachFx(0);
        ((CBall*)this)->DetachFx(1);
        savedBallItemId = requestedBallItemId;
        lookupBallItemId = requestedBallItemId;
        // Phase 1: search the actor-owned cache before requesting a resource.
        BallCacheLink* cacheSentinel = (BallCacheLink*)((char*)&this->puppetListSentinel);
        BallCacheLink* cacheCursor = (*(BallCacheLink**)&this->firstPuppetEntry);
        findBallPuppetCacheEntry(cacheCursor, cacheSentinel, lookupBallItemId);
        lookupAliasResult = cacheCursor;
        lookupAliasCopy = cacheCursor;
        cachePayload = cacheCursor != (BallCacheLink*)((unsigned int)this + (int)&((CBall*)0)->puppetListSentinel)
                           ? (void*)cacheCursor->payload
                           : 0;
        switch ((unsigned int)cachePayload) {
        case 0: {
            // Phase 2: construct the payload, borrowing its IFF record and loaded puppet.
            IFF_STRUCT::sBall* ballItemRecord = (IFF_STRUCT::sBall*)ItemInventory_FindBallData(
                (char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, (const char*)requestedBallItemId);
            if (ballItemRecord) {
                const int zeroLoadOption = 0;
                void* newCachePayload = __nw__FUl(sizeof(BallPuppetCacheEntry));
                if (newCachePayload) {
                    ((BallPuppetCacheEntry*)newCachePayload)->record = 0;
                    ((BallPuppetCacheEntry*)newCachePayload)->puppet = 0;
                    memset(((BallPuppetCacheEntry*)newCachePayload)->sprays, 0,
                           sizeof(((BallPuppetCacheEntry*)newCachePayload)->sprays));
                    memset(((BallPuppetCacheEntry*)newCachePayload)->sequences, 0,
                           sizeof(((BallPuppetCacheEntry*)newCachePayload)->sequences));
                }
                ((BallPuppetCacheEntry*)newCachePayload)->record = ballItemRecord;
                char puppetResourceName[0x20];
                sprintf(puppetResourceName, (char*)golfBallResourceStrings + kBallStringsPetModelFormatOffset,
                        ballItemRecord->model);

                volatile unsigned char allocator = zeroLoadOption;
                const int zeroLoadOptionCopy = zeroLoadOption;
                ((BallPuppetCacheEntry*)newCachePayload)->puppet = ResourceManager_LoadByExtension(
                    g_resourceManager, puppetResourceName, zeroLoadOptionCopy, 1, zeroLoadOptionCopy);
                // Phase 3: guard the new list node until its payload is constructed, then link it.
                LinkBallPuppetCachePayload(this, newCachePayload, allocatedNodeAlias, allocatedNodeAliasCopy);
                cachePayload = newCachePayload;
            }
        } break;
        }
        // Phase 4: publish the equipped ID and activate the selected puppet/effects.
        if (cachePayload) {
            char* sharedDocumentBytes = (char*)g_sharedDoc;

            ((BallPlayerEquipmentSlot*)
                 sharedDocumentBytes)[AsCGolfDoc((*(char**)&AsCSharedDoc(sharedDocumentBytes)->golfDoc))->currentPlayer]
                .ballItemId = requestedBallItemId;
            activateCachedBallPuppet(this, cachePayload);
        }
        break;
    }
    // Activate the current player's cached equipment.
    case kBallRefreshEquipment: {
        ((CBall*)this)->DetachFx(0);
        ((CBall*)this)->DetachFx(1);
        this->previousPlaybackFrame = -1;
        BallCacheLink* cacheSentinel = (BallCacheLink*)((char*)&this->puppetListSentinel);
        BallCacheLink* cacheCursor = (*(BallCacheLink**)&this->firstPuppetEntry);
        char* sharedDocumentBytes = (char*)g_sharedDoc;
        unsigned char playerSlot = AsCGolfDoc((*(char**)&AsCSharedDoc(sharedDocumentBytes)->golfDoc))->currentPlayer;
        unsigned int equippedBallItemId = ((BallPlayerEquipmentSlot*)sharedDocumentBytes)[playerSlot].ballItemId;
        volatile unsigned int savedBallItemId = equippedBallItemId;
        volatile unsigned int lookupBallItemId = equippedBallItemId;
        findBallPuppetCacheEntry(cacheCursor, cacheSentinel, lookupBallItemId);
        BallCacheLink* volatile lookupAliasCopy = cacheCursor;
        BallCacheLink* volatile lookupAliasResult = cacheCursor;
        void* cachePayload = cacheCursor != (BallCacheLink*)((unsigned int)this + (int)&((CBall*)0)->puppetListSentinel)
                                 ? (void*)cacheCursor->payload
                                 : 0;
        if (cachePayload) {
            activateCachedBallPuppet(this, cachePayload);
        }
        break;
    }
    // Recreate idle effects and clear flight-effect handles.
    case kBallRebuildIdleEffects: {
        BallCacheLink* cacheSentinel = (BallCacheLink*)((char*)&this->puppetListSentinel);
        BallCacheLink* cacheCursor = (*(BallCacheLink**)&this->firstPuppetEntry);
        char* sharedDocumentBytes = (char*)g_sharedDoc;
        unsigned char playerSlot = AsCGolfDoc((*(char**)&AsCSharedDoc(sharedDocumentBytes)->golfDoc))->currentPlayer;
        unsigned int equippedBallItemId = ((BallPlayerEquipmentSlot*)sharedDocumentBytes)[playerSlot].ballItemId;
        volatile unsigned int savedBallItemId = equippedBallItemId;
        volatile unsigned int lookupBallItemId = equippedBallItemId;
        findBallPuppetCacheEntry(cacheCursor, cacheSentinel, lookupBallItemId);
        BallCacheLink* volatile lookupAliasResult = cacheCursor;
        BallCacheLink* volatile lookupAliasCopy = cacheCursor;
        void* cachePayload = cacheCursor != (BallCacheLink*)((unsigned int)this + (int)&((CBall*)0)->puppetListSentinel)
                                 ? (void*)cacheCursor->payload
                                 : 0;
        if (cachePayload) {
            ((CBall*)this)->AttachFx(0);
            ((CBall*)this)->AttachFx(1);
            ((BallPuppetCacheEntry*)cachePayload)->sprays[3] = 0;
            ((BallPuppetCacheEntry*)cachePayload)->sprays[2] = 0;
            ((BallPuppetCacheEntry*)cachePayload)->sequences[3] = 0;
            ((BallPuppetCacheEntry*)cachePayload)->sequences[2] = 0;
        }
        break;
    }
    // Create both flight-effect kinds.
    case kBallCreateFlightEffects:
        ((CBall*)this)->AttachFx(2);
        ((CBall*)this)->AttachFx(3);
        break;
    // Store the opaque message payload word.
    case kBallStoreMessageValue:
        this->effectMessageValue = (int)((BallMessage*)msg)->value;
        break;
    }
}

} // extern "C"

} // namespace ShadowAndEffectViews

extern "C" {
void TaskRegistry_RegisterActorAndDummyTypes();
void ActorThread_InitTwoPi();
void GolfAim_InitTwoPi();
}
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620C4", _ctors_u800CBF34_0, TaskRegistry_RegisterActorAndDummyTypes);
STATIC_INIT_ENTRY(".CRT$XCU804620C4", _ctors_u800CBF34_1, CAmbientSound::RegisterType);
STATIC_INIT_ENTRY(".CRT$XCU804620C4", _ctors_u800CBF34_2, ActorThread_InitTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU804620C4", _ctors_u800CBF34_3, GolfAim_InitTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU804620C4", _ctors_u800CBF34_4, CBall::RegisterType);
#pragma pop
