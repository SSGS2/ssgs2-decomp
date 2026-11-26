// Shot-position candidates for the rival's aim search: an array of named
// markers with a radius and a priority, a priority index over them, and the
// bookkeeping that builds and clears both.
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/wvector.h"
#include "classes/CComThinkShotPos.h"

extern "C" {

enum {
    kCandidatePlayerCount = 4,
    kContextPriorityBonus = 3,
    kMaxShotCandidates = 0x4EC4EC4,
    kFirstCandidateGrowthLimit = 0x1A41A41,
    kSecondCandidateGrowthLimit = 0x3483482
};
enum CandidateDiagnosticOffsets {
    kSpecialCandidateMarkerName = 0x4B,
    kCandidateVectorLengthError = 0x4F,
    kCandidateLengthErrorType = 0x63,
    kCandidateAppendReportFormat = 0x9A
};
enum CandidateRecordOffsets {
    kCandidateRecordBytes = sizeof(ShotPositionCandidate),
    kHoleRecordIndexShift = 5,
    kHoleReferenceX = 0xC,
    kHoleReferenceY = 0x10,
    kHoleReferenceZ = 0x14,
    kPriorityBypassesInsideRadiusFilter = -1,
    kDropSurfaceDirectAcceptance = 0,
    kDropSurfaceNeedsFurtherChecks = 1
};

// The candidate array owns its names. The special-marker map owns separate
// copies; BuildCandidateTree stores borrowed candidate pointers, which are
// invalidated by any later growth or clear of the array.

// Clear this player's cached priority adjustment for every shot candidate.
// Requires playerIndex in [0,4); no bounds check is performed.
void CComThinkShotPos::ClearPlayerPriorityAdjustments(int playerIndex) {
    unsigned int candidateIndex = 0;
    int playerByteOffset = playerIndex << 2;
    int resetPriorityAdjustment = (int)candidateIndex;
    int candidateByteOffset = 0;
    // Follow the same player's adjustment column across the 0x34-byte records.
    // Both induction variables stay separate so the final addition keeps PPC operand order.
    while (candidateIndex < candidateCount) {
        char* candidateBase = (char*)candidates;
        candidateIndex = candidateIndex + 1;
        candidateBase = playerByteOffset + candidateBase;
        ShotPositionCandidate* playerColumn = (ShotPositionCandidate*)(candidateByteOffset + (int)candidateBase);
        candidateByteOffset = candidateByteOffset + sizeof(ShotPositionCandidate);
        playerColumn->playerPriorityAdjustment[0] = resetPriorityAdjustment;
    }
}

// Add the context bonus to the matching candidate for the selected player.
// Requires playerIndex in [0,4). Only the first matching context receives +3;
// returns 1 if found, 0 if absent. Duplicate contexts are not accumulated.
int CComThinkShotPos::AddContextPriorityBonus(int contextId, int playerIndex) {
    unsigned int count = candidateCount;
    int candidateByteOffset = 0;
    unsigned int candidateIndex;
    for (candidateIndex = 0; candidateIndex < count; candidateIndex++) {
        ShotPositionCandidate* candidate = (ShotPositionCandidate*)((char*)candidates + candidateByteOffset);
        if (contextId == candidate->contextId) {
            int playerByteOffset = playerIndex << 2;
            int* adjustment = (int*)((char*)candidate + playerByteOffset +
                                     IFF_OFFSET(ShotPositionCandidate, playerPriorityAdjustment));
            int priorityAdjustment = *adjustment;
            *adjustment = priorityAdjustment + kContextPriorityBonus;
            return 1;
        }
        candidateByteOffset += sizeof(ShotPositionCandidate);
    }
    return 0;
}

struct FC4CNode {
    FC4CNode* left;  // 0x0
    FC4CNode* right; // 0x4
    char unk8[4];    // 0x8
    int key;         // 0xc
};

struct FC4CCont {
    char unk0[4];
    FC4CNode* head; // 0x4
};

// Find an integer-tree insertion parent, predecessor, and left/first flags.
void* ComThinkShotPosMap_FindInsertionPosition(void* container, int* keyPtr, void** outNode, unsigned char* insertLeft,
                                               unsigned char* updateFirst) {
    int zero = 0;
    int one = 1;
    *outNode = (void*)zero;
    FC4CCont* c = (FC4CCont*)container;
    FC4CNode** headPtr = &c->head;
    void* last = (void*)headPtr;
    FC4CNode* node = c->head;
    *insertLeft = (unsigned char)one;
    *updateFirst = (unsigned char)one;
    while (node != (FC4CNode*)0) {
        int key = *keyPtr;
        last = (void*)node;
        int nodeKey = node->key;
        if (key < nodeKey) {
            node = node->left;
            *insertLeft = (unsigned char)one;
        } else {
            *outNode = (void*)node;
            node = node->right;
            *insertLeft = (unsigned char)zero;
            *updateFirst = (unsigned char)zero;
        }
    }
    return last;
}

void appFreeUni__FPv(void*);
void ComThinkShotPosMap_DestroySubtree(void*, void*);
void __dl__FPv(void*);
extern char spatialDiagnosticStrings[];
int memcmp(const char*, const char*, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void OSReport(const char*, const char*);
unsigned int strlen(const char*);
// Complete candidate storage; only the first three words are the name.
// The first three words alone are the string ABI; the remaining words are payload.
struct ShotCandidateStorage {
    unsigned int mode;            // 0x0
    unsigned int size;            // 0x4
    unsigned int heapDataAddress; // 0x8 (data pointer for long strings)
    union {
        unsigned int data[10]; // 0xc .. 0x33
        struct {
            unsigned int d0; // position.x bits, 0xc
            unsigned int d1; // position.y bits, 0x10
            unsigned int d2; // position.z bits, 0x14
            float f3;        // radius, world units, 0x18
            unsigned int d4; // base priority; 0x1c
            unsigned int d5; // player 0 priority adjustment; 0x20
            unsigned int d6; // player 1 priority adjustment; 0x24
            unsigned int d7; // player 2 priority adjustment; 0x28
            unsigned int d8; // player 3 priority adjustment; 0x2c
            unsigned int d9; // context id; 0x30
        };
    };
};
struct ShotPositionCandidateView {
    ShotCandidateStorage candidate; // Name and complete marker payload, not only text.
};

void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, unsigned int, unsigned int, void*);
void* ComThinkShotPosMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
void* __as__7WVectorFRC7WVector(void*, void*);
struct U8022F108Pair {
    int key;
    ShotPositionCandidateView e;
};

struct F108Copy {
    unsigned int a0, a1, a2;
    float f0;
    unsigned int a3, a4, a5, a6, a7, a8;
};

// Deep-copy the tagged name, then copy position/radius/priority/player adjustments/context.
// The macro intentionally retains caller-local allocator and copy lifetimes:
// replacing this particular helper by a function changes register allocation.
#define U8022F108_COPY_ENTRY(dst, src)                                                 \
    do {                                                                               \
        VB src8_;                                                                      \
        VB flag_;                                                                      \
        ShotPositionCandidateView* _d = (dst);                                         \
        const ShotPositionCandidateView* _s = (const ShotPositionCandidateView*)(src); \
        unsigned int _flags = _s->candidate.mode;                                      \
        if ((_flags >> 31) == 0) {                                                     \
            _d->candidate.mode = _flags;                                               \
            _d->candidate.size = _s->candidate.size;                                   \
            _d->candidate.heapDataAddress = _s->candidate.heapDataAddress;             \
        } else {                                                                       \
            _d->candidate.mode = 0;                                                    \
            _d->candidate.size = 0;                                                    \
            _d->candidate.heapDataAddress = 0;                                         \
            String_ReserveCapacity(_d, _s->candidate.size);                            \
            flag_.c = src8_.c;                                                         \
            unsigned int _p = _s->candidate.heapDataAddress;                           \
            String_ReplaceRange(_d, 0, 0, _p, _p + _s->candidate.size, &flag_.c);      \
        }                                                                              \
        F108Copy* _dd = (F108Copy*)(&_d->candidate.data[0]);                           \
        const F108Copy* _ss = (const F108Copy*)(&_s->candidate.data[0]);               \
        unsigned int _a = _ss->a0;                                                     \
        unsigned int _b = _ss->a1;                                                     \
        _dd->a0 = _a;                                                                  \
        _a = _ss->a2;                                                                  \
        _dd->a1 = _b;                                                                  \
        float _f = _ss->f0;                                                            \
        _dd->a2 = _a;                                                                  \
        _b = _ss->a3;                                                                  \
        _dd->f0 = _f;                                                                  \
        _a = _ss->a4;                                                                  \
        _dd->a3 = _b;                                                                  \
        _b = _ss->a5;                                                                  \
        _dd->a4 = _a;                                                                  \
        _a = _ss->a6;                                                                  \
        _dd->a5 = _b;                                                                  \
        _b = _ss->a7;                                                                  \
        _dd->a6 = _a;                                                                  \
        _a = _ss->a8;                                                                  \
        _dd->a7 = _b;                                                                  \
        _dd->a8 = _a;                                                                  \
    } while (0)

extern char __vt__Q23std11logic_error[], __vt__Q23std12length_error[], s_sharedCharDeleterVtable[];
void __dt__Q23std12length_errorFv();
void* __nw__FUl(unsigned long);
void* __nwa__FUl(unsigned int);

void __throw(const char*, void*, void*);
char* strcpy(char*, const char*);

struct F108Alloc {
    char value;
    char pad[3];
};

void __dla__FPv(void*);
struct F108Allocation {
    char* pointer;
    char* allocator;
    // Adopt exception text until the shared-control allocation succeeds.
    F108Allocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free text if its shared-control owner was not constructed.
    ~F108Allocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};

struct F108ControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize the shared character deleter's strong and weak counts.
    F108ControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};

struct F108Control : F108ControlBase {
    char* pointer;
    // Adopt exception text after initializing its shared-control base.
    F108Control(char* p) {
        pointer = p;
    }
};

struct F108ErrorBase {
    void* table;
    // Install the base logic-error runtime type before constructing text.
    F108ErrorBase() {
        table = __vt__Q23std11logic_error;
    }
};

struct VB {
    char c;
};

struct F108LengthError : F108ErrorBase {
    char* text;
    void* control;
    // Copy the error message into guarded storage and publish the derived type.
    F108LengthError(const char* message) {
        VB zeroA;
        VB zeroB;
        zeroA.c = 0;
        zeroB.c = 0;
        unsigned int n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            F108Allocation allocation(p, &zeroB.c);
            control = new F108Control(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
};

void* ShopStringRecordArray_dtor(void*, int);
struct F108Buf {
    ShotPositionCandidateView* data;
    volatile unsigned int size;
    unsigned int capacity;
    unsigned int* owner;
    volatile unsigned int offset;
    // Begin a vector-growth transaction associated with the old capacity word.
    F108Buf(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release constructed names and whichever allocation the transaction owns.
    ~F108Buf() {
        ShopStringRecordArray_dtor(this, -1);
    }
};

// Choose the shorter comparison length while preserving reference identity.
inline const unsigned int& F108Min(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Choose the larger vector-growth increment while preserving reference identity.
inline const unsigned int& F108Max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

#define F108_THROW(msg)                                                                                           \
    do {                                                                                                          \
        F108LengthError _ex((msg));                                                                               \
        __throw(spatialDiagnosticStrings + kCandidateLengthErrorType, &_ex, (void*)__dt__Q23std12length_errorFv); \
    } while (0)

// Decode the active tagged-name length without retaining its character storage.
static inline unsigned int CandidateNameLength(const ShotPositionCandidateView* value) {
    return ((value->candidate.mode >> 31) == 0) ? *(const unsigned char*)&value->candidate.mode & 0x7f
                                                : value->candidate.size;
}

} // extern "C"

// Append named shot markers; special markers are stored once in the integer-priority map.
// value stays borrowed; the destination deep-copies its name. Returns 1 even
// when an already-present special marker suppresses insertion. The marker
// literal at spatialDiagnosticStrings+0x4B is "UNP".
int CComThinkShotPos::AddCandidate(ShotPositionCandidateView* value) {
    const char* specialMarkerName = spatialDiagnosticStrings + kSpecialCandidateMarkerName;
    unsigned int specialMarkerLength = strlen(specialMarkerName);
    const char* candidateName;
    unsigned int reportedNameLength;
    unsigned int storageNameLength;
    unsigned int comparisonNameLength;
    int nameOrdering;
    // Phase 1: compare the full tagged name to select vector versus special-map storage.
    reportedNameLength = CandidateNameLength(value);
    if ((value->candidate.mode >> 31) == 0) {
        candidateName = (const char*)value + 1;
        storageNameLength = *(unsigned char*)&value->candidate.mode & 0x7f;
    } else {
        candidateName = (const char*)value->candidate.heapDataAddress;
        storageNameLength = value->candidate.size;
    }
    comparisonNameLength = F108Min(reportedNameLength, storageNameLength);
    nameOrdering = memcmp(candidateName, specialMarkerName, F108Min(comparisonNameLength, specialMarkerLength));
    if (nameOrdering == 0) {
        if (comparisonNameLength < specialMarkerLength)
            nameOrdering = -1;
        else if (comparisonNameLength == specialMarkerLength)
            nameOrdering = 0;
        else
            nameOrdering = 1;
    }
    if (nameOrdering != 0) {
        // Phase 2a: ordinary markers append in place or grow transactionally.
        if (candidateCount < candidateCapacity) {
            ShotPositionCandidateView* dst = (ShotPositionCandidateView*)candidates + candidateCount;
            if (dst != 0) {
                try {
                    U8022F108_COPY_ENTRY(dst, value);
                } catch (...) {
                    throw;
                }
            }
            candidateCount = candidateCount + 1;
        } else {
            unsigned int oldCapacityLimit = candidateCapacity;
            unsigned int additionalCapacityNeeded;
            if ((unsigned)kMaxShotCandidates - oldCapacityLimit < 1)
                F108_THROW(spatialDiagnosticStrings + kCandidateVectorLengthError);
            additionalCapacityNeeded = (candidateCount + 1) - oldCapacityLimit;
            F108Buf growthTransaction(&candidateCapacity);
            unsigned int newCapacity = candidateCapacity;
            if (additionalCapacityNeeded > (unsigned)kMaxShotCandidates - newCapacity)
                F108_THROW(spatialDiagnosticStrings + kCandidateVectorLengthError);
            // Reserve by the original three capacity tiers. The guard owns the
            // new suffix and copied prefix until the allocation swap commits.
            if (newCapacity < (unsigned)kFirstCandidateGrowthLimit)
                newCapacity += F108Max((3 * (newCapacity + 1)) / 5, additionalCapacityNeeded);
            else if (newCapacity < (unsigned)kSecondCandidateGrowthLimit)
                newCapacity += F108Max((newCapacity + 1) / 2, additionalCapacityNeeded);
            else
                newCapacity = (unsigned)kMaxShotCandidates;
            if (newCapacity > (unsigned)kMaxShotCandidates)
                F108_THROW(spatialDiagnosticStrings + kCandidateVectorLengthError);
            ShotPositionCandidateView* replacementEntries =
                (ShotPositionCandidateView*)appMallocDummy__FUl(newCapacity * sizeof(ShotCandidateStorage));
            unsigned int previousEntryCount = candidateCount;
            growthTransaction.data = replacementEntries;
            growthTransaction.capacity = newCapacity;
            growthTransaction.offset = previousEntryCount;
            {
                unsigned int count = 1;
                ShotPositionCandidateView* appendDestination = replacementEntries + growthTransaction.offset;
                while (count != 0) {
                    if (appendDestination != 0) {
                        try {
                            U8022F108_COPY_ENTRY(appendDestination, value);
                        } catch (...) {
                            throw;
                        }
                    }
                    --count;
                    ++appendDestination;
                    growthTransaction.size = growthTransaction.size + 1;
                }
            }
            // Deep-copy existing names backwards before publishing new storage.
            ShotPositionCandidateView* src = (ShotPositionCandidateView*)candidates + candidateCount;
            ShotPositionCandidateView* dst = replacementEntries + growthTransaction.offset;
            while (src > (ShotPositionCandidateView*)candidates) {
                --src;
                --dst;
                if (dst != 0) {
                    try {
                        U8022F108_COPY_ENTRY(dst, src);
                    } catch (...) {
                        throw;
                    }
                }
                growthTransaction.offset = growthTransaction.offset - 1;
                growthTransaction.size = growthTransaction.size + 1;
            }
            // Commit the vector and destroy old name payloads; growthTransaction frees its block.
            ShotPositionCandidateView* oldEntryRangeBegin =
                (ShotPositionCandidateView*)candidates + growthTransaction.offset;
            growthTransaction.data = (ShotPositionCandidateView*)candidates;
            ShotPositionCandidateView* oldEntryRangeEnd = oldEntryRangeBegin + candidateCount;
            unsigned int previousCapacity = candidateCapacity;
            candidateCapacity = growthTransaction.capacity;
            growthTransaction.capacity = previousCapacity;
            candidates = (ShotPositionCandidate*)replacementEntries;
            candidateCount = growthTransaction.size;
            growthTransaction.size = previousEntryCount;
            while (oldEntryRangeEnd > oldEntryRangeBegin) {
                --oldEntryRangeEnd;
                if (oldEntryRangeEnd != 0 && (oldEntryRangeEnd->candidate.mode >> 31) != 0)
                    appFreeUni__FPv((void*)oldEntryRangeEnd->candidate.heapDataAddress);
            }
            growthTransaction.size = 0;
            // growthTransaction's destructor frees the old entries (the original frees them once, there).
        }
        {
            const char* reportName;
            if ((value->candidate.mode >> 31) == 0)
                reportName = (const char*)value + 1;
            else
                reportName = (const char*)value->candidate.heapDataAddress;
            OSReport(spatialDiagnosticStrings + kCandidateAppendReportFormat, reportName);
        }
    } else {
        // Phase 2b: special markers are unique by basePriority; an existing key wins.
        int basePriorityKey = value->candidate.d4;
        char* header = (char*)this + IFF_OFFSET(CComThinkShotPos, priorityRoot);
        char* node = (char*)priorityRoot;
        char* parent = header;
        char* slot;
        while (node != 0) {
            int nodePriorityKey = *(int*)(node + 0xc);
            if (nodePriorityKey < basePriorityKey) {
                node = *(char**)(node + 4);
            } else {
                parent = node;
                node = *(char**)node;
            }
        }
        if (parent == header || basePriorityKey < *(int*)(parent + 0xc))
            parent = header;
        if (parent != header)
            return 1;
        void* predecessorNode = 0;
        unsigned char insertLeft = 0;
        unsigned char updateFirst = 0;
        void* insertionParent =
            ComThinkShotPosMap_FindInsertionPosition((char*)this + IFF_OFFSET(CComThinkShotPos, priorityCount),
                                                     &basePriorityKey, &predecessorNode, &insertLeft, &updateFirst);
        // Find or insert the slot before assigning its value.
        do {
            if (predecessorNode != 0) {
                slot = (char*)predecessorNode + IFF_OFFSET(FC4CNode, key);
                if (*(int*)slot >= basePriorityKey)
                    break;
            }
            {
                ShotPositionCandidateView emptyCandidate;
                emptyCandidate.candidate.mode = 0;
                emptyCandidate.candidate.size = 0;
                emptyCandidate.candidate.heapDataAddress = 0;
                U8022F108Pair initialMapPayload;
                initialMapPayload.key = basePriorityKey;
                U8022F108_COPY_ENTRY(&initialMapPayload.e, &emptyCandidate);
                char* insertedNode =
                    (char*)ComThinkShotPosMap_InsertNode((char*)this + IFF_OFFSET(CComThinkShotPos, priorityCount),
                                                         insertionParent, insertLeft, updateFirst, &initialMapPayload);
                if ((initialMapPayload.e.candidate.mode >> 31) != 0)
                    appFreeUni__FPv((void*)initialMapPayload.e.candidate.heapDataAddress);
                if ((emptyCandidate.candidate.mode >> 31) != 0)
                    appFreeUni__FPv((void*)emptyCandidate.candidate.heapDataAddress);
                slot = insertedNode + 0xc;
            }
        } while (0);
        {
            ShotPositionCandidateView* dstEntry = (ShotPositionCandidateView*)(slot + 4);
            VB src8;
            VB flag;
            try {
                if ((dstEntry->candidate.mode >> 31) == 0 && (value->candidate.mode >> 31) == 0) {
                    dstEntry->candidate.mode = value->candidate.mode;
                    dstEntry->candidate.size = value->candidate.size;
                    dstEntry->candidate.heapDataAddress = value->candidate.heapDataAddress;
                } else {
                    unsigned int dl = (dstEntry->candidate.mode >> 31)
                                          ? dstEntry->candidate.size
                                          : (*(unsigned char*)&dstEntry->candidate.mode & 0x7f);
                    unsigned int vl = (value->candidate.mode >> 31) ? value->candidate.size
                                                                    : (*(unsigned char*)&value->candidate.mode & 0x7f);
                    const char* vs = (value->candidate.mode >> 31) ? (const char*)value->candidate.heapDataAddress
                                                                   : (const char*)value + 1;
                    flag.c = src8.c;
                    String_ReplaceRange(dstEntry, 0, dl, (unsigned int)vs, (unsigned int)(vs + vl), &flag.c);
                }
                *(WVector*)(&dstEntry->candidate.data[0]) = *(WVector*)(&value->candidate.data[0]);
                dstEntry->candidate.f3 = value->candidate.f3;
                dstEntry->candidate.data[4] = value->candidate.data[4];
                dstEntry->candidate.data[5] = value->candidate.data[5];
                dstEntry->candidate.data[6] = value->candidate.data[6];
                dstEntry->candidate.data[7] = value->candidate.data[7];
                dstEntry->candidate.data[8] = value->candidate.data[8];
                dstEntry->candidate.data[9] = value->candidate.data[9];
            } catch (...) {
                throw;
            }
        }
    }
    return 1;
}

extern "C" {

struct ShotPositionPriorityNodeView {
    ShotPositionPriorityNodeView* child0;
    ShotPositionPriorityNodeView* child1;
    char unk8[8];
    unsigned int flags;      // 0x10
    unsigned int nameLength; // 0x14: long-string length, not accessed during node cleanup.
    void* resource;          // 0x18
};

struct CComThinkShotPosView {
    char vtbl[4];
    void* array;
    unsigned int count;
    unsigned int capacity;
    unsigned int treeSize;
    void* root;
};

// Release a tree node's owned long-string name before freeing the node.
inline void FreeShotCandidateNode(ShotPositionPriorityNodeView* node) {
    if ((char*)node + IFF_OFFSET(FC4CNode, key) != 0 &&
        (char*)node + IFF_OFFSET(ShotPositionPriorityNodeView, flags) != 0 &&
        (char*)node + IFF_OFFSET(ShotPositionPriorityNodeView, flags) != 0 && (node->flags >> 31) != 0)
        appFreeUni__FPv(node->resource);
    appFreeUni__FPv(node);
}
// Destroy remaining descendants after the explicitly unrolled top three tree levels.
inline void DestroyShotCandidateChildren(CComThinkShotPosView* owner, ShotPositionPriorityNodeView* node) {
    if (node->child0 != 0)
        ComThinkShotPosMap_DestroySubtree((char*)owner + IFF_OFFSET(CComThinkShotPosView, treeSize), node->child0);
    if (node->child1 != 0)
        ComThinkShotPosMap_DestroySubtree((char*)owner + IFF_OFFSET(CComThinkShotPosView, treeSize), node->child1);
}

#pragma opt_lifetimes off
#pragma use_lmw_stmw on
// Destroy the cached candidate map and owned names while retaining the vector allocation.
int CComThinkShotPos::ClearCachedData() {
    CComThinkShotPosView* self = (CComThinkShotPosView*)this;
    // Destroy map payload names and nodes first, restoring its empty-tree header.
    char* elem;
    int count;
    CComThinkShotPosView* p = self;
    ShotPositionPriorityNodeView* root = (ShotPositionPriorityNodeView*)p->root;
    if (root != 0) {
        ShotPositionPriorityNodeView* leftChild = root->child0;
        if (leftChild != 0) {
            ShotPositionPriorityNodeView* leftLeft = leftChild->child0;
            if (leftLeft != 0) {
                ShotPositionPriorityNodeView* c = leftLeft->child0;
                if (c != 0) {
                    DestroyShotCandidateChildren(p, c);
                    FreeShotCandidateNode(c);
                }
                c = leftLeft->child1;
                if (c != 0) {
                    DestroyShotCandidateChildren(p, c);
                    FreeShotCandidateNode(c);
                }
                FreeShotCandidateNode(leftLeft);
            }
            ShotPositionPriorityNodeView* leftRight = leftChild->child1;
            if (leftRight != 0) {
                ShotPositionPriorityNodeView* c2 = leftRight->child0;
                if (c2 != 0) {
                    DestroyShotCandidateChildren(p, c2);
                    FreeShotCandidateNode(c2);
                }
                c2 = leftRight->child1;
                if (c2 != 0) {
                    DestroyShotCandidateChildren(p, c2);
                    FreeShotCandidateNode(c2);
                }
                FreeShotCandidateNode(leftRight);
            }
            FreeShotCandidateNode(leftChild);
        }
        ShotPositionPriorityNodeView* rightChild = root->child1;
        if (rightChild != 0) {
            ShotPositionPriorityNodeView* rightLeft = rightChild->child0;
            if (rightLeft != 0) {
                ShotPositionPriorityNodeView* c3 = rightLeft->child0;
                if (c3 != 0) {
                    DestroyShotCandidateChildren(p, c3);
                    FreeShotCandidateNode(c3);
                }
                c3 = rightLeft->child1;
                if (c3 != 0) {
                    DestroyShotCandidateChildren(p, c3);
                    FreeShotCandidateNode(c3);
                }
                FreeShotCandidateNode(rightLeft);
            }
            ShotPositionPriorityNodeView* rightRight = rightChild->child1;
            if (rightRight != 0) {
                ShotPositionPriorityNodeView* c4 = rightRight->child0;
                if (c4 != 0) {
                    DestroyShotCandidateChildren(p, c4);
                    FreeShotCandidateNode(c4);
                }
                c4 = rightRight->child1;
                if (c4 != 0) {
                    DestroyShotCandidateChildren(p, c4);
                    FreeShotCandidateNode(c4);
                }
                FreeShotCandidateNode(rightRight);
            }
            FreeShotCandidateNode(rightChild);
        }
        FreeShotCandidateNode(root);
        unsigned int* hdr = (unsigned int*)((char*)p + IFF_OFFSET(CComThinkShotPosView, treeSize));
        hdr[0] = 0;
        hdr[1] = 0;
        hdr[3] = (unsigned int)((char*)hdr + 4);
    }
    // Then destroy vector names in reverse order, retaining its buffer/capacity.
    count = p->count;
    elem = (char*)p->array + count * 0x34;
    p->count = p->count - count;
    while (count != 0) {
        elem = elem - 0x34;
        if ((char*)elem != 0 && (char*)elem != 0 && ((*(unsigned int*)elem) >> 31) != 0)
            appFreeUni__FPv(*(void**)(elem + 8));
        count = count - 1;
    }
    return 1;
}
#pragma use_lmw_stmw reset
#pragma opt_lifetimes reset

// Wrapper owns one tagged byte string at +4. flag is its capacity/heap tag;
// heapStringData is owned text only when stringStorageTag's high bit is set.
// +8 stores heap length; the wrapper's field names do not change its destructor ABI.
struct CandidateStringWrapperView {
    char unk0[4];
    unsigned int stringStorageTag; // 0x4
    char unk8[4];
    void* heapStringData; // 0xc
};

// Release the owned long-string payload and optionally delete the wrapper.
void* dtor_8022FCAC(CandidateStringWrapperView* self, int flags) {
    if (self != 0) {
        if ((char*)self + 4 != 0) {
            if ((char*)self + 4 != 0) {
                if (self->stringStorageTag >> 31)
                    appFreeUni__FPv(self->heapStringData);
            }
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

extern float kFloat_0_RivalCollision;
extern float kUnplayableDropDistanceAllowance;
extern void* g_sharedDoc;
extern int s_shotCandidateInitialTreeValue;
void __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
float WVectorLen__FRC7WVector(void*);
void* IntegerKeyTree_FindInsertionPosition(void*, void*, void*, void*, void*);
void* BpEntryTree_InsertNode(void*, void*, unsigned char, unsigned char, int*);
void* GolfBall__Fv();
void* __as__7WVectorFRC7WVector(void*, void*);
float Magnitude__7WVectorCFv(void*);
void* CQuadTree_GetInstance(void*);
void* GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
int CQuadTree_OverlappedWithObject(void*, void*, float);
int GetGroundType__9CQuadTreeF7WVector(void*, void*);
int CourseData_IsPointOutsidePlayableRegion(void*, void*);
void* __as__7WVectorFRC7WVector(void*, void*);

struct U802304E0Node {
    U802304E0Node* left;   // 0x0
    U802304E0Node* right;  // 0x4
    U802304E0Node* parent; // 0x8 (low bit: colour)
    char unkC[0x10];
    int data[3]; // 0x1c
};

struct U802304E0Obj {
    char beforeSentinel[0x14];
    void* sentinel[2];
    U802304E0Node* first; // 0x1c
};

struct U802304E0V3 {
    int x, y, z;
};

// Choose the first grounded, unobstructed drop point satisfying the playable-region and distance rules.
// Iterate candidates in tree-key order. Return 1 and write world XYZ to out on
// success; return 0 with out unchanged otherwise. Surface code 0 is accepted
// directly. Code 1 must also be playable and farther from the hole reference
// than (ball's horizontal distance - allowance); all other codes are rejected.
int ComUnplayable_FindDropCandidate(U802304E0Obj* self, void* out) {
    U802304E0Node* end;
    U802304E0Node* node;
    float zero;
    float threshold;
    void* golfDocument;
    unsigned char roundHoleIndex;
    char* holeRecord;
    void* ballPosition;
    void* quadTree;
    int surfaceCode;
    int candidateAccepted;
    // Integer arrays carry IEEE-754 XYZ bits, never integer coordinates.
    // Separate copies preserve the by-value WVector ABI and original call lifetimes.
    int horizontalCandidateDeltaBits[3];
    int horizontalBallDeltaBits[3];
    int groundedCandidateBits[3];
    int ballToHoleDeltaBits[3];
    int holeReferenceBits[3];
    int groundQueryResultBits[3];
    int markerPositionBits[3];
    int objectBoundsQueryBits[3];
    int surfaceQueryBits[3];
    int candidateToHoleDeltaBits[3];
    int currentHoleReferenceBits[3];

    // Phase 1: establish the horizontal distance threshold from the ball to this hole's reference.
    golfDocument = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    roundHoleIndex = AsCGolfDoc(golfDocument)->currentHoleIndex;
    holeRecord = (char*)AsCGolfDoc(golfDocument)->holeRecords + (roundHoleIndex << kHoleRecordIndexShift);
    int holeReferenceXBits = *(int*)(holeRecord + kHoleReferenceX);
    int holeReferenceYBits = *(int*)(holeRecord + kHoleReferenceY);
    holeReferenceBits[0] = holeReferenceXBits;
    holeReferenceBits[1] = holeReferenceYBits;
    holeReferenceBits[2] = *(int*)(holeRecord + kHoleReferenceZ);
    ballPosition = GolfBall__Fv();
    __mi__FRC7WVectorRC7WVector(ballToHoleDeltaBits, ballPosition, holeReferenceBits);
    *(WVector*)horizontalBallDeltaBits = *(WVector*)ballToHoleDeltaBits;
    *(float*)&horizontalBallDeltaBits[1] = kFloat_0_RivalCollision;
    threshold = (*(WVector*)horizontalBallDeltaBits).Magnitude() - kUnplayableDropDistanceAllowance;
    zero = kFloat_0_RivalCollision;
    node = self->first;
    end = (U802304E0Node*)((char*)self->sentinel);
    // Phase 2: ground each marker, reject object bounds, then apply its surface-specific rule.
    while (node != end) {
        int markerXBits = node->data[0];
        int markerYBits = node->data[1];
        markerPositionBits[0] = markerXBits;
        markerPositionBits[1] = markerYBits;
        markerPositionBits[2] = node->data[2];
        quadTree = CQuadTree_GetInstance((void*)markerXBits);
        GetGroundPoint__9CQuadTreeF7WVectorbPib(groundQueryResultBits, quadTree, markerPositionBits, 0, 0, 0);
        *(WVector*)groundedCandidateBits = *(WVector*)groundQueryResultBits;
        objectBoundsQueryBits[0] = groundedCandidateBits[0];
        objectBoundsQueryBits[1] = groundedCandidateBits[1];
        objectBoundsQueryBits[2] = groundedCandidateBits[2];
        quadTree = CQuadTree_GetInstance((void*)groundedCandidateBits[1]);
        if (CQuadTree_OverlappedWithObject(quadTree, objectBoundsQueryBits, kFloat_0_RivalCollision) == 0) {
            candidateAccepted = 0;
            surfaceQueryBits[0] = groundedCandidateBits[0];
            surfaceQueryBits[1] = groundedCandidateBits[1];
            surfaceQueryBits[2] = groundedCandidateBits[2];
            quadTree = CQuadTree_GetInstance((void*)groundedCandidateBits[1]);
            surfaceCode = GetGroundType__9CQuadTreeF7WVector(quadTree, surfaceQueryBits) & 0xFF;
            switch (surfaceCode) {
            case kDropSurfaceDirectAcceptance:
                candidateAccepted = 1;
                break;
            case kDropSurfaceNeedsFurtherChecks:
                if (CourseData_IsPointOutsidePlayableRegion(
                        AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->courseData, groundedCandidateBits) ==
                    0) {
                    golfDocument = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
                    holeRecord = (char*)AsCGolfDoc(golfDocument)->holeRecords +
                                 (AsCGolfDoc(golfDocument)->currentHoleIndex << kHoleRecordIndexShift);
                    currentHoleReferenceBits[0] = *(int*)(holeRecord + kHoleReferenceX);
                    currentHoleReferenceBits[1] = *(int*)(holeRecord + kHoleReferenceY);
                    currentHoleReferenceBits[2] = *(int*)(holeRecord + kHoleReferenceZ);
                    __mi__FRC7WVectorRC7WVector(candidateToHoleDeltaBits, groundedCandidateBits,
                                                currentHoleReferenceBits);
                    *(WVector*)horizontalCandidateDeltaBits = *(WVector*)candidateToHoleDeltaBits;
                    *(float*)&horizontalCandidateDeltaBits[1] = zero;
                    if ((*(WVector*)horizontalCandidateDeltaBits).Magnitude() > threshold)
                        candidateAccepted = 1;
                }
                break;
            }
            if (candidateAccepted) {
                *(WVector*)out = *(WVector*)groundedCandidateBits;
                return 1;
            }
        }
        // Phase 3: in-order successor: descend right/left, otherwise climb tagged parents.
        if (node->right != 0) {
            U802304E0Node* child = node->right;
            while (child->left != 0)
                child = child->left;
            node = child;
        } else {
            U802304E0Node* parent;
            do {
                parent = (U802304E0Node*)((unsigned int)node->parent & ~1u);
                if (node == parent->left)
                    break;
                node = parent;
            } while (1);
            node = parent;
        }
    }
    return 0;
}

struct CandidateWorldVector {
    float x, y, z;
};

struct CandidateVectorBits {
    int x, y, z;
};

// A hole record's pin position: the first 12 bytes are the tee, the next the pin.
struct CandidateHolePositionView {
    char unk0[0xc];
    CandidateVectorBits position;
};

// Advance a candidate index (-1 starts at zero); write a borrowed entry on success.
static inline int NextShotPositionCandidate(const CComThinkShotPos* obj, int* idx, ShotPositionCandidate** out) {
    int ok;
    ShotPositionCandidate* base = obj->candidates;
    int mask = (int)((*idx + 1) | (-1 - *idx)) >> 31;
    int n = (*idx + 1) & mask;
    if (n < 0 || (unsigned int)n >= obj->candidateCount) {
        ok = 0;
    } else {
        *idx = n;
        *out = base + n;
        ok = 1;
    }
    return ok;
}

// The insertion API returns a node pointer; its key and value start at node+0xC.
struct ShotPositionIndexValue {
    int key;
    ShotPositionCandidate* candidate;
};

// Index eligible shot markers by base priority plus the player's cached priority adjustment.
// queryPosition is borrowed world XYZ; maximumEdgeDistance is the exclusive
// upper bound for horizontal distance minus marker radius. destination must be prepared:
// this appends missing priorities and does not clear old entries. At equal
// priority the first encountered marker wins. Returns 1 even for an empty result.
int CComThinkShotPos::BuildCandidateTree(ShotPositionIndex* destination, const WVector* queryPosition,
                                         float maximumEdgeDistance) {
    float zero = kFloat_0_RivalCollision;
    int candidateIndex = -1;
    ShotPositionCandidate* entry;
    ShotPositionIndex* map = destination;
    const void* query = queryPosition;
    CandidateWorldVector queryDelta;
    CandidateWorldVector holeDelta;
    CandidateVectorBits holeReferenceBits;
    CandidateVectorBits horizontalQueryDeltaBits;
    CandidateVectorBits horizontalHoleDeltaBits;
    // Compare horizontal distances from the query and hole reference to each marker.
    while (NextShotPositionCandidate(this, &candidateIndex, &entry)) {
        __mi__FRC7WVectorRC7WVector(&queryDelta, query, &entry->position);
        horizontalQueryDeltaBits = *(CandidateVectorBits*)&queryDelta;
        *(float*)&horizontalQueryDeltaBits.y = zero;
        float queryDistance = WVectorLen(*(WVector*)&horizontalQueryDeltaBits);

        void* golfDocument = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
        char* holeRecordsBase = (char*)AsCGolfDoc(golfDocument)->holeRecords;
        CandidateHolePositionView* holeRecordView =
            (CandidateHolePositionView*)(holeRecordsBase +
                                         (AsCGolfDoc(golfDocument)->currentHoleIndex << kHoleRecordIndexShift));
        holeReferenceBits.x = holeRecordView->position.x;
        holeReferenceBits.y = holeRecordView->position.y;
        holeReferenceBits.z = holeRecordView->position.z;
        __mi__FRC7WVectorRC7WVector(&holeDelta, &holeReferenceBits, &entry->position);
        horizontalHoleDeltaBits = *(CandidateVectorBits*)&holeDelta;
        *(float*)&horizontalHoleDeltaBits.y = zero;
        float holeReferenceDistance = WVectorLen(*(WVector*)&horizontalHoleDeltaBits);

        float distanceToMarkerEdge = queryDistance - entry->radius;
        int priority =
            entry->basePriority +
            entry->playerPriorityAdjustment[AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer];

        // Base priority -1 bypasses only the inside-radius exclusion, not the
        // maximum edge distance. Cached player adjustments still affect the key.
        if (entry->basePriority != kPriorityBypassesInsideRadiusFilter && holeReferenceDistance > entry->radius &&
            distanceToMarkerEdge < zero)
            continue;
        if (distanceToMarkerEdge >= maximumEdgeDistance)
            continue;

        // Index a borrowed entry only if no earlier marker already owns its score key.
        ShotPositionIndexNode* node = map->root;
        ShotPositionIndexNode* lowerBound = (ShotPositionIndexNode*)&map->root;
        while (node != 0) {
            if (node->key >= priority) {
                lowerBound = node;
                node = node->left;
            } else {
                node = node->right;
            }
        }
        if (lowerBound == (ShotPositionIndexNode*)&map->root || priority < lowerBound->key)
            lowerBound = (ShotPositionIndexNode*)&map->root;
        if ((ShotPositionIndexNode*)&map->root == lowerBound) {
            char* predecessorNode;
            unsigned char updateFirst;
            unsigned char insertLeft;
            void* insertionParent =
                IntegerKeyTree_FindInsertionPosition(map, &priority, &predecessorNode, &insertLeft, &updateFirst);
            char* keyValueSlot;
            if (predecessorNode == 0 || *(int*)(predecessorNode + IFF_OFFSET(ShotPositionIndexNode, key)) < priority) {
                int initialKeyValue[2];
                initialKeyValue[0] = priority;
                initialKeyValue[1] = s_shotCandidateInitialTreeValue;
                keyValueSlot =
                    (char*)BpEntryTree_InsertNode(map, insertionParent, insertLeft, updateFirst, initialKeyValue) +
                    IFF_OFFSET(ShotPositionIndexNode, key);
            } else {
                keyValueSlot = predecessorNode + IFF_OFFSET(ShotPositionIndexNode, key);
            }
            ((ShotPositionIndexValue*)keyValueSlot)->candidate = entry;
        }
    }
    return 1;
}

} // extern "C"
