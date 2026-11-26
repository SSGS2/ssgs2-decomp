// Puppet construction and setup: the overlay coordinate mode, the instance and animation indexes,
// the bone sort key reset, and the puppet constructor.

#include <decomp/static_init.h>
#include "classes/WPuppet.h"
#include "classes/WOverlay.h"
#define AsWPuppet(p) ((WPuppet*)(p))
#include "sdk_protos.h"
#include "game/wvector.h"

extern "C" {

// Store the overlay drawing options supplied by its caller.
void WOverlay_SetCoordMode(void* self, int value) {
    AsWOverlay(self)->coordMode = value;
} // 0x8004F8B4

extern float s_twoPi_8004F8B4;
extern float kFloat_6_2831855;
// Initialize this unit\'s full-turn angle during static initialization.
void InitializePuppetFullTurnAngle() {
    s_twoPi_8004F8B4 = kFloat_6_2831855;
}

void BaseObject_ctor(void*);
void Vec3_Zero(void*);
void Matrix_SetIdentityAffine(void*);
void __as__7WVectorFRC7WVector(void*, const float*);
void appFreeUni__FPv(void*);
void* appMallocUni__FUl(unsigned int);
extern const char __vt__7WPuppet[];
extern float kFloat_0_puppet;

void __dt__9WResourceFv(void*, int);
void WPuppetOwnedInstances_dtor(void*, int);
void WPuppetAnimationInstances_dtor(void*, int);
void WPuppetCachedBoneList_dtor(void*, int);
void WPuppetMaterialVector_dtor(void*, int);
void WPuppetChildList_dtor(void*, int);

enum PuppetInitialCapacity {
    kInitialInstanceBlockSize = 16,
    kInitialInstanceBucketCount = 16
};

// Construction view of the instance index; the shared header leaves its middle words opaque.
struct PuppetInstanceIndex {
    int first, cursor, ownedHead, ownedTail;
    int blockSize, bucketMask, bucketCount;
    int* buckets;
    char mutex[0x18];
};
struct PuppetBaseInit {
    char baseState[0x14];
    int resourceState;
};

// Scope-bound locking keeps constructor failure paths paired with OSUnlockMutex.
struct PuppetLock {
    OSMutex* mutex;
    // Acquire a borrowed embedded mutex; never allocate or destroy the mutex itself.
    PuppetLock(char* p) : mutex((OSMutex*)p) {
        OSLockMutex(mutex);
    }
    // Release the constructor lock on normal return and exception unwinding.
    ~PuppetLock() {
        OSUnlockMutex(mutex);
    }
};

// Constructor unwind stages (stage advances only after the listed members are ready):
// 0: WResource base only; 1: ownedInstances; 2: also animationInstances;
// 3: also cached-bone list, materials, child list. Cleanup runs in reverse order.
// self is cleared on success, transferring all members to the completed WPuppet.
// The base resource destructor runs for every active stage, including stage 0.
struct PuppetConstruction {
    char* self;
    int stage;
    // Unwind only the subobjects whose initialization stage completed.
    ~PuppetConstruction() {
        if (!self)
            return;
        if (stage >= 3) {
            WPuppetChildList_dtor(self + (int)&((WPuppet*)0)->childCount, -1);
            WPuppetMaterialVector_dtor(self + (int)&((WPuppet*)0)->materials, -1);
            WPuppetCachedBoneList_dtor(self + (int)&((WPuppet*)0)->cachedBoneCount, -1);
        }
        if (stage >= 2)
            WPuppetAnimationInstances_dtor(self + (int)&((WPuppet*)0)->animationInstances, -1);
        if (stage >= 1)
            WPuppetOwnedInstances_dtor(self + (int)&((WPuppet*)0)->ownedInstances, -1);
        __dt__9WResourceFv(self, -1);
    }
};

#pragma opt_propagation off
// Initialize instance bookkeeping and allocate an empty hash-bucket table.
inline void InitInstanceIndex(char* instanceIndex, int blockSize, int bucketCount) {
    ((PuppetInstanceIndex*)instanceIndex)->first = 0;
    ((PuppetInstanceIndex*)instanceIndex)->ownedTail = 0;
    ((PuppetInstanceIndex*)instanceIndex)->ownedHead = 0;
    ((PuppetInstanceIndex*)instanceIndex)->blockSize = blockSize;
    ((PuppetInstanceIndex*)instanceIndex)->cursor = 0;
    ((PuppetInstanceIndex*)instanceIndex)->bucketCount = bucketCount;
    if (((PuppetInstanceIndex*)instanceIndex)->bucketCount > 0) {
        ((PuppetInstanceIndex*)instanceIndex)->buckets =
            (int*)appMallocUni__FUl(((PuppetInstanceIndex*)instanceIndex)->bucketCount * sizeof(int*));
        ((PuppetInstanceIndex*)instanceIndex)->bucketMask = ((PuppetInstanceIndex*)instanceIndex)->bucketCount - 1;
        for (int i = 0; i < ((PuppetInstanceIndex*)instanceIndex)->bucketCount; ++i)
            (((PuppetInstanceIndex*)instanceIndex)->buckets)[i] = 0;
    } else {
        ((PuppetInstanceIndex*)instanceIndex)->buckets = 0;
    }
}
#pragma opt_propagation on

// Initialize and lock the animation instance index before populating its buckets.
inline void InitAnimationIndex(char* instanceIndex) {
    *(int*)&((PuppetInstanceIndex*)instanceIndex)->buckets = 0;
    OSInitMutex((OSMutex*)(instanceIndex + (int)&((PuppetInstanceIndex*)0)->mutex));
    char* mutex = instanceIndex + (int)&((PuppetInstanceIndex*)0)->mutex;
    PuppetLock lock(mutex);
    InitInstanceIndex(instanceIndex, kInitialInstanceBlockSize, kInitialInstanceBucketCount);
}

#pragma opt_propagation off
// Mark the bone sort key invalid and clear the material count through its existing storage type.
inline void InvalidateBoneSortKeyAndClearCount(unsigned int* p, int* state) {
    unsigned int count = *p;
    *state = -1;
    *p -= count;
}
#pragma opt_propagation on

// Construct mutex-protected instance indices and empty model, bone, material and child state.
// self is caller-provided WPuppet storage; return that same address, now owning its allocations.
// The int/pointer casts below preserve 32-bit ABI stores, not logical integer fields.
// Null-base member addresses are byte offsets; changing them to direct member pointers
// can change register lifetimes and constructor exception tables in this compiler.
void* WPuppet_ctor(void* self) {
    float boundsSphere[4];
    char* puppetBytes = (char*)self;
    BaseObject_ctor(self);
    PuppetConstruction construction = {puppetBytes, 0};

    // Phase 1: initialize the base vtable and both protected instance indices.
    {
        char* ownedMutexAddress = puppetBytes + (int)&((WPuppet*)0)->ownedInstances.mutex;
        int zero = 0;
        (*(int*)&AsWPuppet(puppetBytes)->resourceManager) = zero;
        AsWPuppet(puppetBytes)->vtbl = (void*)__vt__7WPuppet;
        *(int*)&AsWPuppet(puppetBytes)->ownedInstances.storage = zero;
        OSInitMutex((OSMutex*)ownedMutexAddress);
        char* ownedMutex = puppetBytes + (int)&((WPuppet*)0)->ownedInstances.mutex;
        PuppetLock lock(ownedMutex);
        InitInstanceIndex(puppetBytes + (int)&((WPuppet*)0)->ownedInstances, kInitialInstanceBlockSize,
                          kInitialInstanceBucketCount);
    }
    construction.stage = 1;
    InitAnimationIndex(puppetBytes + (int)&((WPuppet*)0)->animationInstances);
    construction.stage = 2;
    // Phase 2: empty model/animation storage and establish self-linked list sentinels.
    int zero = 0;
    *(int*)&AsWPuppet(puppetBytes)->hierarchyContext = zero;
    zero = 0;
    AsWPuppet(puppetBytes)->hasResourceName = (char)zero;
    AsWPuppet(puppetBytes)->pad_0BD[0] = (char)zero;

    *(int*)&AsWPuppet(puppetBytes)->keyRecords = zero;
    AsWPuppet(puppetBytes)->keyRecordCount = zero;
    *(int*)&AsWPuppet(puppetBytes)->triangleCount = zero;
    Vec3_Zero(puppetBytes + (int)&((WPuppet*)0)->boundsMin);
    Vec3_Zero(puppetBytes + (int)&((WPuppet*)0)->boundsMax);
    float initialAnimationAmount = kFloat_0_puppet;

    *(int*)&AsWPuppet(puppetBytes)->namedBones = zero;
    char* resourceName = puppetBytes + (int)&((WPuppet*)0)->resourceName;
    *(int*)&AsWPuppet(puppetBytes)->namedBoneCount = zero;

    *(int*)&AsWPuppet(puppetBytes)->animationRanges = zero;
    AsWPuppet(puppetBytes)->cachedBoneCount = zero;
    char* cachedBoneSentinel = puppetBytes + (int)&((WPuppet*)0)->cachedBoneTail;
    *(int*)&AsWPuppet(puppetBytes)->cachedBoneHead = (int)cachedBoneSentinel;
    *(int*)&AsWPuppet(puppetBytes)->cachedBoneTail = (int)cachedBoneSentinel;

    AsWPuppet(puppetBytes)->animationDuration = initialAnimationAmount;
    AsWPuppet(puppetBytes)->boneCount = zero;
    *(int*)&AsWPuppet(puppetBytes)->bones = zero;
    *(int*)&AsWPuppet(puppetBytes)->boneRoot = zero;
    *(int*)&AsWPuppet(puppetBytes)->modelResourceWord = zero;
    *(int*)&AsWPuppet(puppetBytes)->skinMatrices = zero;
    *(int*)&AsWPuppet(puppetBytes)->boneMatrices = zero;
    AsWPuppet(puppetBytes)->animationDisabled = (char)zero;
    *(int*)&AsWPuppet(puppetBytes)->nameIndexMap = zero;
    *(int*)&AsWPuppet(puppetBytes)->materials = zero;
    *(int*)&AsWPuppet(puppetBytes)->materialCount = zero;
    *(int*)&AsWPuppet(puppetBytes)->materialCapacity = zero;
    *(int*)&AsWPuppet(puppetBytes)->materialMetadata = zero;
    *(int*)&AsWPuppet(puppetBytes)->modelLayerVisibilityWords = zero;
    AsWPuppet(puppetBytes)->childCount = zero;
    char* childSentinel = puppetBytes + (int)&((WPuppet*)0)->childTail;
    *(int*)&AsWPuppet(puppetBytes)->childHead = (int)childSentinel;
    *(int*)&AsWPuppet(puppetBytes)->childTail = (int)childSentinel;
    *(int*)&AsWPuppet(puppetBytes)->animation = zero;
    *(int*)&AsWPuppet(puppetBytes)->keyRange = zero;
    *(int*)&AsWPuppet(puppetBytes)->defaultBone = zero;
    *(int*)&AsWPuppet(puppetBytes)->layerMatrices = zero;
    *(int*)&AsWPuppet(puppetBytes)->layerScratch = zero;

    *(int*)&AsWPuppet(puppetBytes)->layerController = zero;
    *(int*)&AsWPuppet(puppetBytes)->layerResource = zero;
    int defaultRenderValue = 1;
    *(int*)&AsWPuppet(puppetBytes)->renderStateValue = defaultRenderValue;
    *(int*)&AsWPuppet(puppetBytes)->boundObjects = zero;
    *(int*)&AsWPuppet(puppetBytes)->namedResources = zero;
    *(int*)&AsWPuppet(puppetBytes)->boneController = zero;
    *(int*)&AsWPuppet(puppetBytes)->rendererOption = defaultRenderValue;
    construction.stage = 3;
    // Phase 3: normalize empty child/cache lists and initialize world/bounds/render data.
    Memory_Clear(resourceName, 0x40);

    char* childListEnd = puppetBytes + (int)&((WPuppet*)0)->childTail;
    void* childNode = AsWPuppet(puppetBytes)->childHead;
    *(int*)&AsWPuppet(puppetBytes)->boundsBone = zero;
    *(int*)&AsWPuppet(puppetBytes)->boundsData = zero;
    // Plain C: free every node of the (already empty) child list and decrement childCount per node.
    if (childNode != (void*)childListEnd) {
        void* tailNode = *(void**)childListEnd;
        int linkWord = *(int*)&((WPuppetListNode*)tailNode)->next;
        void* linkNode = ((WPuppetListNode*)childNode)->prev;
        *(int*)&((WPuppetListNode*)linkNode)->next = linkWord;
        linkNode = ((WPuppetListNode*)tailNode)->next;
        linkWord = *(int*)&((WPuppetListNode*)childNode)->prev;
        *(int*)&((WPuppetListNode*)linkNode)->prev = linkWord;
        while (childNode != (void*)childListEnd) {
            void* node = childNode;
            childNode = ((WPuppetListNode*)childNode)->next;
            appFreeUni__FPv(node);
            AsWPuppet(puppetBytes)->childCount = AsWPuppet(puppetBytes)->childCount - 1;
        }
    }

    // worldMatrix maps puppet-local coordinates into world coordinates; initially identity.
    Matrix_SetIdentityAffine(puppetBytes + (int)&((WPuppet*)0)->worldMatrix);

    void* cachedBoneListEnd = puppetBytes + (int)&((WPuppet*)0)->cachedBoneTail;
    void* cachedBoneNode = AsWPuppet(puppetBytes)->cachedBoneHead;
    // Plain C: free every node of the cached-bone list and decrement cachedBoneCount per node.
    if (cachedBoneNode != cachedBoneListEnd) {
        void* tailNode = *(void**)cachedBoneListEnd;

        void* linkNode = ((WPuppetListNode*)cachedBoneNode)->prev;
        unsigned int linkWord = *(int*)&((WPuppetListNode*)tailNode)->next;
        *(int*)&((WPuppetListNode*)linkNode)->next = linkWord;
        linkWord = *(int*)&((WPuppetListNode*)cachedBoneNode)->prev;
        linkNode = ((WPuppetListNode*)tailNode)->next;
        *(int*)&((WPuppetListNode*)linkNode)->prev = linkWord;
        while (cachedBoneNode != cachedBoneListEnd) {
            void* node = cachedBoneNode;
            cachedBoneNode = ((WPuppetListNode*)cachedBoneNode)->next;
            appFreeUni__FPv(node);
            AsWPuppet(puppetBytes)->cachedBoneCount = AsWPuppet(puppetBytes)->cachedBoneCount - 1;
        }
    }

    float zeroComponent = kFloat_0_puppet;

    float zeroCenter[3];
    zeroCenter[0] = zeroComponent;
    zeroCenter[1] = zeroComponent;
    zeroCenter[2] = zeroComponent;
    *(WVector*)boundsSphere = *(WVector*)zeroCenter;
    boundsSphere[3] = kFloat_0_puppet;
    AsWPuppet(puppetBytes)->boundsCenter = *(WVector*)boundsSphere;

    AsWPuppet(puppetBytes)->boundsRadius = (boundsSphere[3]);
    ((PuppetBaseInit*)puppetBytes)->resourceState = 0;
    *(int*)&AsWPuppet(puppetBytes)->renderParamsEnabled = 0;
    Memory_Clear(puppetBytes + (int)&((WPuppet*)0)->renderParams, 0x68);

    InvalidateBoneSortKeyAndClearCount((unsigned int*)(puppetBytes + (int)&((WPuppet*)0)->materialCount),
                                       (int*)(puppetBytes + (int)&((WPuppet*)0)->boneSortKey));
    // Phase 4: commit ownership; the unwind guard must no longer destroy these members.
    construction.self = 0;
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FD4", _ctors_u8004F8B4_0, InitializePuppetFullTurnAngle);
#pragma pop
