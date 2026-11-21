typedef unsigned long new_size_t;
#include "game/types.h"
#include "game/wvector.h"
#include "classes/CFxSpray.h"

// Configuration begins at CFxSpray::particlesFollowEmitter. These private
// views describe embedded ranges/lists without changing the shared class.
// Unresolved members are identified by configuration-relative offset. Assignment
// proves their extent and copy type only; it cannot establish a tunable particle role.
// Range endpoints have no proven min/max ordering or coordinate space here.
// Compile-time displacement only; retaining the original load type is intentional.
#define SPRAY_OFFSET(member) ((unsigned int)&((SprayConfiguration*)0)->member)

struct SprayVectorRange { WVector first; WVector second; };
struct SprayTrackList { unsigned int size; void* last; void* first; };
struct SprayConfiguration {
    unsigned char particlesFollowEmitter, reserved_01, billboardEnabled, attachedToParent;
    unsigned int parentRotationAxisMask;
    unsigned int spriteResource;
    char petResourceName[40];
    unsigned char petLightingEnabled, sphericalEmission;
    char reserved_36[2];
    SprayVectorRange emissionBounds;
    unsigned char regenerateAtSamePosition;
    char reserved_51[3];
    float cameraOffset;
    unsigned int minLifetime, maxLifetime;
    float opacityMultiplier, volumeCoreScaleX, volumeCoreScaleY, friction, groundFriction;
    WVector unresolvedVector74;
    float groundHeightOffset, groundRestitution, groundCollisionRadius;
    unsigned int spriteBlendMode, trailBlendMode, colorModulationEnabled, lightingColor;
    unsigned int minInitialFrame, maxInitialFrame, trailSegmentCount;
    float trailSampleInterval;
    unsigned char trailEndAlpha, trailEndRed, trailEndGreen, trailEndBlue;
    unsigned int resourceId;
    float trailWidthScale;
    unsigned char trailMonolithEnabled, trailCameraSideEnabled;
    char reserved_BA[2];
    SprayVectorRange unresolvedRangeBC;
    float minWindScale, maxWindScale;
    WVector unresolvedVectorDC;
    unsigned int externalOrientationAxisMask;
    SprayTrackList unresolvedTrackEC;
    SprayVectorRange unresolvedRangeF8;
    SprayVectorRange unresolvedRange110;
    SprayTrackList angularSpeedTrack, velocityRotationTrack, particleOrbitTrack, emitterOrbitTrack;
    SprayTrackList unresolvedTrack158;
    SprayTrackList spawnTrack, gravityPointTrack, colorTrack;
    float minInitialSize, maxInitialSize, spriteAspectRatio;
    SprayTrackList sizeTrack, spriteFrameTrack;
    SprayTrackList unresolvedTrack1AC, unresolvedTrack1B8;
    float minDrawDistance, maxDrawDistance;
    SprayTrackList unresolvedTrack1CC, unresolvedTrack1D8, unresolvedTrack1E4;
    unsigned char circularEmission;
};
typedef char SprayConfiguration_trackOffset[
    ((int)&((SprayConfiguration*)0)->angularSpeedTrack ==
     (int)&((CFxSpray*)0)->angularSpeedTrackCount -
     (int)&((CFxSpray*)0)->particlesFollowEmitter) ? 1 : -1];
typedef char SprayConfiguration_endOffset[
    ((int)&((SprayConfiguration*)0)->circularEmission == 0x1F0) ? 1 : -1];
// Translation unit 0x80111E8C-0x801134F0 (5 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void __as__7WVectorFRC7WVector(void *, const void *);
void AABB_Copy(void *, const void *);
void FxSprayEmitterOrbitTrackList_Assign(void *, const void *);
void FxSpraySpawnTrackList_Assign(void *, const void *);
void FxSprayGravityPointTrackList_Assign(void *, const void *);
void FxSprayColorTrackList_Assign(void *, const void *);
void FxSpraySizeTrackList_Assign(void *, const void *);
void FxSpraySpriteFrameTrackList_Assign(void *, const void *);
void VectorFlagsRecordList_Assign(void *, const void *);
void ThreeWordRecordList_Assign(void *, const void *);
void VectorPairRecordList_Assign(void *, const void *);
void IdRecordList_Assign(void *, const void *);
void RecordAmountList_Assign(void *, const void *);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);

// Use the supplied storage for placement construction without allocating.
inline void* operator new(new_size_t, void* p) { return p; }
// Placement delete leaves caller-owned storage untouched.
inline void operator delete(void*, void*) {}

// All three list specializations use [size, last, first]. The sentinel is &list.last:
// its prev/next words alias last/first, and no payload exists at the sentinel. Real nodes
// own their inline payload at +8 and are allocated/freed through appMallocDummy/appFreeUni.
// Iterators borrow nodes; allocator tag objects have no ownership. Temporary lists own
// only newly allocated suffix nodes until splice clears their size and resets both links.
// Payload ABI: scalar=[u32,float,u32,float], vector=[Vec3,int,float,int,float],
// angular-speed prefix copy=[float,float,float,float]; its suffix construction
// reuses the scalar word-copy view. Value12938 therefore describes storage access,
// not a proven mixed-type angular-speed schema. Key roles remain unresolved;
// assignment alone establishes word order and copy behavior, not physical units.
// Editable payload map (byte offsets, not inferred keyframe semantics):
//   Value1237C / Value12938: +00 u32, +04 float, +08 u32, +0C float.
//   Value12EF4: +00 WVector, +0C int, +10 float, +14 int, +18 float.
// Angular-speed common-prefix copying uses floats at +00/+04/+08/+0C;
// its appended suffix uses the first map. Keep both paths when editing.
// a/c are stored integer words and b/d are float-accessed words; assignment
// provides no evidence of time, interpolation, or units. Do not tune by these letters.
// Storage-only vocabulary for the 16-byte scalar/mixed record. Offsets, not
// guessed time/interpolation units, are the safe way to discuss these words.
// A four-float LOAD/STORE view is used only for the angular-speed common prefix.
struct AngularSpeedPrefixCopyView {
    float word00, word04, word08, word0C;
};
struct Value1237C {
    unsigned int a;
    float b;
    unsigned int c;
    float d;
};
struct Node1237C {
    Node1237C* prev;
    Node1237C* next;
    Value1237C value;
};
struct List1237C {
    unsigned int size;
    Node1237C* last;
    Node1237C* first;
};

struct AssignIter1237C {
    Node1237C* p;
    // End a borrowed iterator lifetime without freeing its node.
    ~AssignIter1237C() {}
    // Borrow the current real node payload; dereferencing the sentinel is invalid.
    Value1237C& operator*() const { return p->value; }
    // Advance the borrowed iterator to its next node and return this iterator.
    AssignIter1237C& operator++() { p = p->next; return *this; }
    // Move the borrowed iterator to its previous node and return this iterator.
    AssignIter1237C& operator--() { p = p->prev; return *this; }
};
struct AssignAllocator1237C {
    unsigned char tag;
    // End the allocator-tag lifetime without releasing node storage.
    ~AssignAllocator1237C() {}
};
// Advance a circular-list node forward or backward by a signed distance.
static inline Node1237C* advanceAssign1237C(int distance, Node1237C* node) {
    if (distance >= 0) { while (distance > 0) { node = node->next; --distance; } }
    else { while (distance < 0) { node = node->prev; ++distance; } }
    return node;
}
#pragma push
#pragma inline_depth(1024)
#pragma use_lmw_stmw on
#pragma optimize_for_size off
#pragma scheduling on
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
// Unlink a half-open node range, free each node, and reduce the list size.
static inline void eraseAssign1237C(List1237C* list, AssignIter1237C& first, AssignIter1237C& last) {
    if (first.p != last.p) {
        Node1237C* back = last.p->prev;
        first.p->prev->next = back->next;
        back->next->prev = first.p->prev;
        while (first.p != last.p) {
            Node1237C* victim = first.p;
            ++first;
            appFreeUni__FPv((void*)victim);
            --list->size;
        }
    }
}
// Temporary lists own newly allocated suffix nodes until they are spliced into the destination.
struct AssignTempBase1237C : List1237C {
    // Initialize an empty circular temporary list with its embedded sentinel.
    AssignTempBase1237C() { Node1237C* e = (Node1237C*)&last; size = 0; first = e; last = e; }
    // Unlink and free every node still owned by the temporary list.
    ~AssignTempBase1237C() {
        Node1237C* p = first;
        Node1237C* end = (Node1237C*)&last;
        if (p != end) {
            Node1237C* back = end->prev;
            p->prev->next = back->next;
            back->next->prev = p->prev;
            while (p != end) {
                Node1237C* n = p;
                p = p->next;
                appFreeUni__FPv((void*)n);
                --size;
            }
        }
    }
};
// Release-on-failure guard for a node whose value has not finished construction.
struct AssignAllocation1237C {
    Node1237C* p;
    // Guard a newly allocated node until construction/linking transfers ownership.
    AssignAllocation1237C(Node1237C* n) : p(n) {}
    // Free the node allocation if construction/linking did not release it.
    ~AssignAllocation1237C() { if (p) appFreeUni__FPv((void*)p); }
    // Transfer the guarded node to its list and return its address.
    Node1237C* release() { Node1237C* n = p; p = 0; return n; }
};
struct AssignTemp1237C : AssignTempBase1237C {
    // Copy a source suffix into a separately owned list, preserving unwind cleanup.
    AssignTemp1237C(AssignIter1237C& rangeMid, AssignIter1237C& rangeEnd) {
        AssignAllocator1237C allocator = {0};
        AssignIter1237C finish = rangeEnd;
        AssignIter1237C sourceIterator = rangeMid;
        if (sourceIterator.p != finish.p)
        for (; sourceIterator.p != finish.p; ++sourceIterator) {
            AssignAllocator1237C allocator = {0};
            Node1237C* destinationSentinel = (Node1237C*)&last;
            Node1237C* source = sourceIterator.p;
            Node1237C* newNode = (Node1237C*)appMallocDummy__FUl(sizeof(Node1237C));
            AssignIter1237C allocated = {newNode};
            AssignIter1237C reserved = allocated;
            AssignIter1237C position = {destinationSentinel};
            AssignAllocation1237C allocation(newNode);
            new (&newNode->value) Value1237C(source->value);
            newNode = allocation.release();
            destinationSentinel->prev->next = newNode;
            newNode->prev = destinationSentinel->prev;
            destinationSentinel->prev = newNode;
            newNode->next = destinationSentinel;
            ++size;
        }
    }
};
// Select the shorter list size by reference.
static inline const unsigned int& minAssign1237C(const unsigned int& a, const unsigned int& b) {
    return b < a ? b : a;
}
// Return the stored list length.
static inline unsigned int sizeAssign1237C(const List1237C* l) { return l->size; }
// Locate a list element by walking from whichever end is nearer.
static inline Node1237C* nthAssign1237C(const List1237C* sourceList, unsigned int index) {
    unsigned int sourceSize = sourceList->size;
    if (index <= sourceSize / 2) {
        AssignAllocator1237C allocator = {0};
        return advanceAssign1237C(index, sourceList->first);
    } else {
        AssignAllocator1237C allocator = {0};
        return advanceAssign1237C(index - sourceSize, (Node1237C*)&sourceList->last);
    }
}

// These aliases name roles without renaming EH helper classes in object relocations.
typedef List1237C ScalarTrackListStorage;
typedef Node1237C ScalarTrackNode;
typedef AssignIter1237C ScalarTrackIterator;
typedef AssignTemp1237C ScalarTrackSuffixOwner;

// Assign a scalar-track list, reusing existing nodes and splicing or erasing the size difference.
// Both lists must be initialized circular lists. Return the destination; self-assignment
// does nothing. Build a missing suffix before overwriting the common prefix so allocation
// failure leaves the destination intact. Payloads contain no separately owned allocations.
ScalarTrackListStorage* FxSprayScalarTrackList_Assign(ScalarTrackListStorage* destinationList, const ScalarTrackListStorage* sourceList) {
    if (destinationList != sourceList) { ScalarTrackIterator copyEnd;  ScalarTrackIterator sourceEnd;  ScalarTrackIterator suffixBegin; 

        unsigned int selfSize;
        unsigned int otherSize;

        ScalarTrackIterator suffixEnd;

        ScalarTrackIterator sourceBegin;
        otherSize = sizeAssign1237C(sourceList);
        selfSize = sizeAssign1237C(destinationList);
        ScalarTrackNode* middle = nthAssign1237C(sourceList, minAssign1237C(selfSize, otherSize));
        copyEnd.p = middle;
        sourceEnd.p = (ScalarTrackNode*)&sourceList->last;
        suffixEnd = sourceEnd;
        suffixBegin.p = middle;
        sourceBegin.p = sourceList->first;
        // Source is longer: construct an owned suffix, then overwrite the reusable prefix.
        if (copyEnd.p != sourceEnd.p) {
            ScalarTrackSuffixOwner newNodes(suffixBegin, suffixEnd);
            ScalarTrackNode* last;

            ScalarTrackNode* first; ScalarTrackNode* destinationNode;
            ScalarTrackIterator finishCopy = copyEnd;
            destinationNode = destinationList->first;
            for (ScalarTrackIterator sourceIterator = sourceBegin; sourceIterator.p != finishCopy.p; ++sourceIterator) {
                destinationNode->value = *sourceIterator;
                destinationNode = destinationNode->next;
            }
            // Splice the complete suffix before the destination sentinel; transfer node ownership.
            if (newNodes.size) {
                first = newNodes.first;
                last = newNodes.last;
                newNodes.first = newNodes.last = (ScalarTrackNode*)&newNodes.last;
                destinationNode->prev->next = first;
                first->prev = destinationNode->prev;
                destinationNode->prev = last;
                last->next = destinationNode;
                destinationList->size += newNodes.size;
                newNodes.size = 0;
            }
        } else {
            ScalarTrackNode* destinationNode = destinationList->first;
            ScalarTrackIterator finishCopy = copyEnd;

            for (ScalarTrackIterator sourceIterator = sourceBegin; sourceIterator.p != finishCopy.p; ++sourceIterator) {
                destinationNode->value = *sourceIterator;
                destinationNode = destinationNode->next;
            }
            // Source fits: the prefix is copied; unlink and free the remaining destination suffix.
            ScalarTrackIterator eraseBegin = {destinationNode};
            ScalarTrackNode* destinationEnd = (ScalarTrackNode*)&destinationList->last;
            if (eraseBegin.p != destinationEnd) {
                ScalarTrackNode* back = destinationEnd->prev;
                eraseBegin.p->prev->next = back->next;
                back->next->prev = eraseBegin.p->prev;
                ScalarTrackNode* victim;
                while ((victim = eraseBegin.p) != destinationEnd) {
                    eraseBegin.p = victim->next;
                    appFreeUni__FPv((void*)victim);
                    destinationList->size--;
                }
            }
        }
    }
    return destinationList;
}

struct Vec12EF4 {
    float x, y, z;
    // Copy vector components through the canonical WVector assignment operation.
    Vec12EF4& operator=(const Vec12EF4& o) { __as__7WVectorFRC7WVector(this, &o); return *this; }
};
// v is a copied WVector; the four following words retain their observed types.
// Their relation to v and their physical units are unresolved in this copy unit.
struct Value12EF4 {
    Vec12EF4 v;
    int a;
    float b;
    int c;
    float d;
    // Copy the inline vector and four trailing track words without sharing storage.
    Value12EF4(const Value12EF4& o) : v(o.v), a(o.a), b(o.b), c(o.c), d(o.d) {}
};
struct Node12EF4 {
    Node12EF4* prev;
    Node12EF4* next;
    Value12EF4 value;
};
struct List12EF4 {
    unsigned int size;
    Node12EF4* last;
    Node12EF4* first;
};

struct AssignIter12EF4 {
    Node12EF4* p;
    // End a borrowed iterator lifetime without freeing its node.
    ~AssignIter12EF4() {}
    // Borrow the current real node payload; dereferencing the sentinel is invalid.
    Value12EF4& operator*() const { return p->value; }
    // Advance the borrowed iterator to its next node and return this iterator.
    AssignIter12EF4& operator++() { p = p->next; return *this; }
    // Move the borrowed iterator to its previous node and return this iterator.
    AssignIter12EF4& operator--() { p = p->prev; return *this; }
};
struct AssignAllocator12EF4 {
    unsigned char tag;
    // End the allocator-tag lifetime without releasing node storage.
    ~AssignAllocator12EF4() {}
};
// Advance a circular-list node forward or backward by a signed distance.
static inline Node12EF4* advanceAssign12EF4(int distance, Node12EF4* node) {
    if (distance >= 0) { while (distance > 0) { node = node->next; --distance; } }
    else { while (distance < 0) { node = node->prev; ++distance; } }
    return node;
}
#pragma inline_depth(1024)
#pragma use_lmw_stmw on
#pragma optimize_for_size off
#pragma scheduling on
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
// Unlink a half-open node range, free each node, and reduce the list size.
static inline void eraseAssign12EF4(List12EF4* list, AssignIter12EF4& first, AssignIter12EF4& last) {
    if (first.p != last.p) {
        Node12EF4* back = last.p->prev;
        first.p->prev->next = back->next;
        back->next->prev = first.p->prev;
        while (first.p != last.p) {
            Node12EF4* victim = first.p;
            ++first;
            appFreeUni__FPv((void*)victim);
            --list->size;
        }
    }
}
struct AssignTempBase12EF4 : List12EF4 {
    // Initialize an empty circular temporary list with its embedded sentinel.
    AssignTempBase12EF4() { Node12EF4* e = (Node12EF4*)&last; size = 0; first = e; last = e; }
    // Unlink and free every node still owned by the temporary list.
    ~AssignTempBase12EF4() {
        Node12EF4* p = first;
        Node12EF4* end = (Node12EF4*)&last;
        if (p != end) {
            Node12EF4* back = end->prev;
            p->prev->next = back->next;
            back->next->prev = p->prev;
            while (p != end) {
                Node12EF4* n = p;
                p = p->next;
                appFreeUni__FPv((void*)n);
                --size;
            }
        }
    }
};
struct AssignAllocation12EF4 {
    Node12EF4* p;
    // Guard a newly allocated node until construction/linking transfers ownership.
    AssignAllocation12EF4(Node12EF4* n) : p(n) {}
    // Free the node allocation if construction/linking did not release it.
    ~AssignAllocation12EF4() { if (p) appFreeUni__FPv((void*)p); }
    // Transfer the guarded node to its list and return its address.
    Node12EF4* release() { Node12EF4* n = p; p = 0; return n; }
};
struct AssignTemp12EF4 : AssignTempBase12EF4 {
    // Copy a source suffix into a separately owned list, preserving unwind cleanup.
    AssignTemp12EF4(AssignIter12EF4& rangeMid, AssignIter12EF4& rangeEnd) {
        AssignAllocator12EF4 allocator = {0};
        AssignIter12EF4 finish = rangeEnd;
        AssignIter12EF4 sourceIterator = rangeMid;
        if (sourceIterator.p != finish.p)
        for (; sourceIterator.p != finish.p; ++sourceIterator) {
            AssignAllocator12EF4 allocator = {0};
            Node12EF4* destinationSentinel = (Node12EF4*)&last;
            Node12EF4* source = sourceIterator.p;
            Node12EF4* newNode = (Node12EF4*)appMallocDummy__FUl(sizeof(Node12EF4));
            AssignIter12EF4 allocated = {newNode};
            AssignIter12EF4 reserved = allocated;
            AssignIter12EF4 position = {destinationSentinel};
            AssignAllocation12EF4 allocation(newNode);
            new (&newNode->value) Value12EF4(source->value);
            newNode = allocation.release();
            destinationSentinel->prev->next = newNode;
            newNode->prev = destinationSentinel->prev;
            destinationSentinel->prev = newNode;
            newNode->next = destinationSentinel;
            ++size;
        }
    }
};
// Select the shorter list size by reference.
static inline const unsigned int& minAssign12EF4(const unsigned int& a, const unsigned int& b) {
    return b < a ? b : a;
}
// Return the stored list length.
static inline unsigned int sizeAssign12EF4(const List12EF4* l) { return l->size; }
// Locate a list element by walking from whichever end is nearer.
static inline Node12EF4* nthAssign12EF4(const List12EF4* sourceList, unsigned int index) {
    unsigned int sourceSize = sourceList->size;
    if (index <= sourceSize / 2) {
        AssignAllocator12EF4 allocator = {0};
        return advanceAssign12EF4(index, sourceList->first);
    } else {
        AssignAllocator12EF4 allocator = {0};
        return advanceAssign12EF4(index - sourceSize, (Node12EF4*)&sourceList->last);
    }
}

typedef List12EF4 VectorTrackListStorage;
typedef Node12EF4 VectorTrackNode;
typedef AssignIter12EF4 VectorTrackIterator;
typedef AssignTemp12EF4 VectorTrackSuffixOwner;

// Assign vector-track entries, copying existing values before splicing or erasing nodes.
// Both lists must be initialized circular lists. Return the destination; self-assignment
// does nothing. Build a missing suffix before overwriting the common prefix so allocation
// failure leaves the destination intact. Payloads contain no separately owned allocations.
VectorTrackListStorage* FxSprayVectorTrackList_Assign(VectorTrackListStorage* destinationList, const VectorTrackListStorage* sourceList) {
    if (destinationList != sourceList) {
        VectorTrackNode* middle = nthAssign12EF4(sourceList, minAssign12EF4(sizeAssign12EF4(destinationList), sizeAssign12EF4(sourceList)));
        VectorTrackIterator copyEnd = {middle};
        VectorTrackIterator sourceEnd = {(VectorTrackNode*)&sourceList->last};
        VectorTrackIterator sourceBegin = {sourceList->first};

        VectorTrackIterator suffixBegin = copyEnd;
        // Source is longer: construct an owned suffix, then overwrite the reusable prefix.
        if (copyEnd.p != sourceEnd.p) {
            VectorTrackSuffixOwner newNodes(suffixBegin, (sourceEnd));
            VectorTrackNode* destinationNode = destinationList->first;
            VectorTrackIterator finishCopy = copyEnd;
            for (VectorTrackIterator sourceIterator = sourceBegin; sourceIterator.p != finishCopy.p; ++sourceIterator) {
                destinationNode->value = *sourceIterator;
                destinationNode = destinationNode->next;
            }
            // Splice the complete suffix before the destination sentinel; transfer node ownership.
            if (newNodes.size) {
                VectorTrackNode* last;
                VectorTrackNode* first;
                first = newNodes.first;
                last = newNodes.last;
                newNodes.first = newNodes.last = (VectorTrackNode*)&newNodes.last;
                destinationNode->prev->next = first;
                first->prev = destinationNode->prev;
                destinationNode->prev = last;
                last->next = destinationNode;
                destinationList->size += newNodes.size;
                newNodes.size = 0;
            }
        } else {
            VectorTrackIterator finishCopy = copyEnd;
            VectorTrackNode* destinationNode = destinationList->first;

            for (VectorTrackIterator sourceIterator = sourceBegin; sourceIterator.p != finishCopy.p; ++sourceIterator) {
                destinationNode->value = *sourceIterator;
                __typeof__(destinationNode->next) _p12EF4 = destinationNode->next; destinationNode = _p12EF4;
            }
            // Source fits: the prefix is copied; unlink and free the remaining destination suffix.
            VectorTrackIterator eraseBegin = {destinationNode};
            VectorTrackNode* destinationEnd = (VectorTrackNode*)&destinationList->last;
            if (eraseBegin.p != destinationEnd) {
                VectorTrackNode* back = destinationEnd->prev;
                eraseBegin.p->prev->next = back->next;
                back->next->prev = eraseBegin.p->prev;
                VectorTrackNode* victim;
                while ((victim = eraseBegin.p) != destinationEnd) {
                    eraseBegin.p = victim->next;
                    appFreeUni__FPv((void*)victim);
                    --destinationList->size;
                }
            }
        }
    }
    return destinationList;
}
#pragma pop

#pragma push
// Word-copy storage view only; see the four-float prefix loop in Assign below.
struct Value12938 {
    unsigned int a;
    float b;
    unsigned int c;
    float d;
};
struct Node12938 {
    Node12938* prev;
    Node12938* next;
    Value12938 value;
};
struct List12938 {
    unsigned int size;
    Node12938* last;
    Node12938* first;
};

struct AssignIter12938 {
    Node12938* p;
    // End a borrowed iterator lifetime without freeing its node.
    ~AssignIter12938() {}
    // Borrow the current real node payload; dereferencing the sentinel is invalid.
    Value12938& operator*() const { return p->value; }
    // Advance the borrowed iterator to its next node and return this iterator.
    AssignIter12938& operator++() { p = p->next; return *this; }
    // Move the borrowed iterator to its previous node and return this iterator.
    AssignIter12938& operator--() { p = p->prev; return *this; }
};
struct AssignAllocator12938 {
    unsigned char tag;
    // End the allocator-tag lifetime without releasing node storage.
    ~AssignAllocator12938() {}
};
// Advance a circular-list node forward or backward by a signed distance.
static inline Node12938* advanceAssign12938(int distance, Node12938* node) {
    if (distance >= 0) { while (distance > 0) { node = node->next; --distance; } }
    else { while (distance < 0) { node = node->prev; ++distance; } }
    return node;
}
#pragma inline_depth(1024)
#pragma use_lmw_stmw on
#pragma optimize_for_size off
#pragma scheduling on
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
// Unlink a half-open node range, free each node, and reduce the list size.
static inline void eraseAssign12938(List12938* list, AssignIter12938& first, AssignIter12938& last) {
    if (first.p != last.p) {
        Node12938* back = last.p->prev;
        first.p->prev->next = back->next;
        back->next->prev = first.p->prev;
        while (first.p != last.p) {
            Node12938* victim = first.p;
            ++first;
            appFreeUni__FPv((void*)victim);
            --list->size;
        }
    }
}
struct AssignTempBase12938 : List12938 {
    // Initialize an empty circular temporary list with its embedded sentinel.
    AssignTempBase12938() { last = first = (Node12938*)&last; size = 0; }
    // Unlink and free every node still owned by the temporary list.
    ~AssignTempBase12938() {
        Node12938* p = first;
        Node12938* end = (Node12938*)&last;
        if (p != end) {
            Node12938* back = end->prev;
            p->prev->next = back->next;
            back->next->prev = p->prev;
            while (p != end) {
                Node12938* n = p;
                p = p->next;
                appFreeUni__FPv((void*)n);
                --size;
            }
        }
    }
};
struct AssignAllocation12938 {
    Node12938* p;
    // Guard a newly allocated node until construction/linking transfers ownership.
    AssignAllocation12938(Node12938* n) : p(n) {}
    // Free the node allocation if construction/linking did not release it.
    ~AssignAllocation12938() { if (p) appFreeUni__FPv((void*)p); }
    // Transfer the guarded node to its list and return its address.
    Node12938* release() { Node12938* n = p; p = 0; return n; }
};
struct AssignTemp12938 : AssignTempBase12938 {
    // Copy a source suffix into a separately owned list, preserving unwind cleanup.
    AssignTemp12938(AssignIter12938& rangeMid, AssignIter12938& rangeEnd) {
        AssignAllocator12938 allocator = {0};
        AssignIter12938 finish = rangeEnd;
        AssignIter12938 sourceIterator = rangeMid;
        if (sourceIterator.p != finish.p)
        for (; sourceIterator.p != finish.p; ++sourceIterator) {
            AssignAllocator12938 allocator = {0};
            Node12938* destinationSentinel = (Node12938*)&last;
            Node12938* source = sourceIterator.p;
            Node12938* newNode = (Node12938*)appMallocDummy__FUl(0x18);
            AssignIter12938 allocated = {newNode};
            AssignIter12938 reserved = allocated;
            AssignIter12938 position = {destinationSentinel};
            AssignAllocation12938 allocation(newNode);
            new (&newNode->value) Value12938(source->value);
            newNode = allocation.release();
            destinationSentinel->prev->next = newNode;
            newNode->prev = destinationSentinel->prev;
            destinationSentinel->prev = newNode;
            newNode->next = destinationSentinel;
            ++size;
        }
    }
};
// Select the shorter list size by reference.
static inline const unsigned int& minAssign12938(const unsigned int& a, const unsigned int& b) {
    return b < a ? b : a;
}
// Return the stored list length.
static inline unsigned int sizeAssign12938(const List12938* l) { return l->size; }
// Locate a list element by walking from whichever end is nearer.
static inline Node12938* nthAssign12938(const List12938* sourceList, unsigned int index) {
    unsigned int sourceSize = sourceList->size;
    if (index <= sourceSize / 2) {
        AssignAllocator12938 allocator = {0};
        return advanceAssign12938(index, sourceList->first);
    } else {
        AssignAllocator12938 allocator = {0};
        return advanceAssign12938(index - sourceSize, (Node12938*)&sourceList->last);
    }
}

typedef List12938 AngularSpeedTrackListStorage;

// Copy existing angular-speed words through the observed float-access view only.
// Suffix construction deliberately continues to use the scalar-view owner below.
static inline void CopyAngularSpeedPrefixPayload(AngularSpeedPrefixCopyView& destination,
    const AngularSpeedPrefixCopyView& source) {
    destination.word00 = source.word00;
    destination.word04 = source.word04;
    destination.word08 = source.word08;
    destination.word0C = source.word0C;
}

// Assign the angular-speed track using float prefix copies and scalar-view suffix construction.
// Both lists must be initialized circular lists. Return the destination; self-assignment
// does nothing. Build a missing suffix before overwriting the common prefix so allocation
// failure leaves the destination intact. Payloads contain no separately owned allocations.
AngularSpeedTrackListStorage* FxSprayAngularSpeedTrackList_Assign(AngularSpeedTrackListStorage* selfp, const AngularSpeedTrackListStorage* otherp) {
    // The scalar list cast describes link layout and suffix construction only.
    // It does not establish a uniform four-float node payload.
    ScalarTrackListStorage* destinationList = (ScalarTrackListStorage*)selfp;
    const ScalarTrackListStorage* sourceList = (const ScalarTrackListStorage*)otherp;
    if (destinationList != sourceList) {
        ScalarTrackNode* middle = nthAssign1237C(sourceList, minAssign1237C(sizeAssign1237C(destinationList), sizeAssign1237C(sourceList)));
        ScalarTrackIterator copyEnd = {middle};
        ScalarTrackIterator sourceEnd = {(ScalarTrackNode*)&sourceList->last};
        ScalarTrackIterator sourceBegin = {sourceList->first};

        ScalarTrackIterator suffixBegin = copyEnd;
        // Source is longer: construct an owned suffix, then overwrite the reusable prefix.
        if (copyEnd.p != sourceEnd.p) {
            // New suffix uses [u32,float,u32,float] accesses; existing nodes
            // below use four float accesses. Keep both copy forms explicit.
            ScalarTrackSuffixOwner newNodes(suffixBegin, (sourceEnd));
            ScalarTrackNode* destinationNode = destinationList->first;
            ScalarTrackIterator finishCopy = copyEnd;
            for (ScalarTrackIterator sourceIterator = sourceBegin; sourceIterator.p != finishCopy.p; ++sourceIterator) {
                AngularSpeedPrefixCopyView* destinationPayload =
                    (AngularSpeedPrefixCopyView*)&destinationNode->value;
                const AngularSpeedPrefixCopyView* sourcePayload =
                    (const AngularSpeedPrefixCopyView*)&sourceIterator.p->value;
                CopyAngularSpeedPrefixPayload(*destinationPayload, *sourcePayload);
                destinationNode = destinationNode->next;
            }
            // Splice the complete suffix before the destination sentinel; transfer node ownership.
            if (newNodes.size) {
                ScalarTrackNode* suffixFirst = newNodes.first;
                ScalarTrackNode* suffixLast = newNodes.last;
                newNodes.first = newNodes.last = (ScalarTrackNode*)&newNodes.last;
                destinationNode->prev->next = suffixFirst;
                suffixFirst->prev = destinationNode->prev;
                suffixLast->next = destinationNode;
                destinationNode->prev = suffixLast;
                destinationList->size = destinationList->size + newNodes.size;
                newNodes.size = 0;
            }
        } else {
            ScalarTrackIterator finishCopy = copyEnd;
            ScalarTrackNode* destinationNode = destinationList->first;
            for (ScalarTrackIterator sourceIterator = sourceBegin; finishCopy.p != sourceIterator.p; ++sourceIterator) {
                AngularSpeedPrefixCopyView* destinationPayload =
                    (AngularSpeedPrefixCopyView*)&destinationNode->value;
                const AngularSpeedPrefixCopyView* sourcePayload =
                    (const AngularSpeedPrefixCopyView*)&sourceIterator.p->value;
                CopyAngularSpeedPrefixPayload(*destinationPayload, *sourcePayload);
                __typeof__(destinationNode->next) nextDestinationNode = destinationNode->next;
                destinationNode = nextDestinationNode;
            }
            // Source fits: the prefix is copied; unlink and free the remaining destination suffix.
            ScalarTrackIterator eraseBegin = {destinationNode};
            ScalarTrackNode* destinationEnd = (ScalarTrackNode*)&destinationList->last;
            if (eraseBegin.p != destinationEnd) {
                ScalarTrackNode* back = destinationEnd->prev;
                eraseBegin.p->prev->next = back->next;
                back->next->prev = eraseBegin.p->prev;
                ScalarTrackNode* victim;
                while ((victim = eraseBegin.p) != destinationEnd) {
                    eraseBegin.p = victim->next;
                    appFreeUni__FPv((void*)victim);
                    destinationList->size--;
                }
            }
        }
    }
    return selfp;
}
#pragma pop


#pragma dont_inline on
// Copy both endpoints of an embedded vector range through WVector assignment.
void *FxSprayVectorRange_Assign(void *dst, const void *src) {
    ((SprayVectorRange*)dst)->first = ((const SprayVectorRange*)src)->first;
    ((SprayVectorRange*)dst)->second = ((const SprayVectorRange*)src)->second;
    return dst;
}
#pragma dont_inline off

// Deep-copy spray configuration, including all owned track lists and reserved_01.
// dst/src point at configuration subobjects, not CFxSpray objects. Both must already
// contain initialized lists; resource IDs and the inline filename are copied as values.
// Each list keeps separate nodes. No particle runtime state or emitter resource is acquired.
void* FxSprayConfig_Assign(void *dst, const void *src) {
    unsigned char *destinationBytes = (unsigned char *)dst;
    const unsigned char *sourceBytes = (const unsigned char *)src;
    unsigned char byteValue;
    unsigned int offset;
    // Preserve byte-copy order for the packed filename and lighting/emission flags.
    destinationBytes[0x1c] = sourceBytes[0x1c];
    // +0x1E..+0x33: filename tail; +0x34/+0x35: lighting/spherical flags.
    for (offset = 0x1e; offset <= 0x35; ++offset) { byteValue=sourceBytes[offset]; destinationBytes[offset]=byteValue; }
    byteValue=sourceBytes[0];
    destinationBytes[0]=byteValue;
    byteValue=sourceBytes[1];
    destinationBytes[1]=byteValue;
    byteValue=sourceBytes[2];
    destinationBytes[2]=byteValue;
    byteValue=sourceBytes[3];
    destinationBytes[3]=byteValue;
    *(unsigned int*)&((SprayConfiguration*)destinationBytes)->parentRotationAxisMask=*(const unsigned int *)(sourceBytes + SPRAY_OFFSET(parentRotationAxisMask));
    *(unsigned int*)&((SprayConfiguration*)destinationBytes)->spriteResource=*(const unsigned int *)(sourceBytes + SPRAY_OFFSET(spriteResource));
    byteValue=sourceBytes[0xc];
    destinationBytes[0xc]=byteValue;
    byteValue=sourceBytes[0xd];
    destinationBytes[0xd]=byteValue;
    byteValue=sourceBytes[0xe];
    destinationBytes[0xe]=byteValue;
    byteValue=sourceBytes[0xf];
    destinationBytes[0xf]=byteValue;
    byteValue=sourceBytes[0x10];
    destinationBytes[0x10]=byteValue;
    byteValue=sourceBytes[0x11];
    destinationBytes[0x11]=byteValue;
    byteValue=sourceBytes[0x12];
    destinationBytes[0x12]=byteValue;
    byteValue=sourceBytes[0x13];
    destinationBytes[0x13]=byteValue;
    byteValue=sourceBytes[0x14];
    destinationBytes[0x14]=byteValue;
    byteValue=sourceBytes[0x15];
    destinationBytes[0x15]=byteValue;
    byteValue=sourceBytes[0x16];
    destinationBytes[0x16]=byteValue;
    byteValue=sourceBytes[0x17];
    destinationBytes[0x17]=byteValue;
    byteValue=sourceBytes[0x18];
    destinationBytes[0x18]=byteValue;
    byteValue=sourceBytes[0x19];
    destinationBytes[0x19]=byteValue;
    byteValue=sourceBytes[0x1a];
    destinationBytes[0x1a]=byteValue;
    byteValue=sourceBytes[0x1b];
    destinationBytes[0x1b]=byteValue;
    byteValue=sourceBytes[0x1d]; destinationBytes[0x1d]=byteValue;
    AABB_Copy(&((SprayConfiguration*)destinationBytes)->emissionBounds, &((const SprayConfiguration*)sourceBytes)->emissionBounds);
    ((SprayConfiguration*)destinationBytes)->regenerateAtSamePosition = ((const SprayConfiguration*)sourceBytes)->regenerateAtSamePosition;
    *(float*)&((SprayConfiguration*)destinationBytes)->cameraOffset = *(const float *)(sourceBytes + SPRAY_OFFSET(cameraOffset));
    *(unsigned int*)&((SprayConfiguration*)destinationBytes)->minLifetime = *(const unsigned int *)(sourceBytes + SPRAY_OFFSET(minLifetime));
    *(unsigned int*)&((SprayConfiguration*)destinationBytes)->maxLifetime = *(const unsigned int *)(sourceBytes + SPRAY_OFFSET(maxLifetime));
    // Copy the five scalar opacity, volume-core and friction controls.
    for (offset = 0x60; offset <= 0x70; offset += 4)
        *(float *)(destinationBytes + offset) = *(const float *)(sourceBytes + offset);
    ((SprayConfiguration*)destinationBytes)->unresolvedVector74 = ((const SprayConfiguration*)sourceBytes)->unresolvedVector74;
    // Ground contact: height offset, restitution, radius (three floats).
    for (offset = 0x80; offset <= 0x88; offset += 4)
        *(float *)(destinationBytes + offset) = *(const float *)(sourceBytes + offset);
    // Copy blend modes, color controls, frame bounds and trail segment count.
    for (offset = 0x8C; offset <= 0xA4; offset += 4)
        *(unsigned int *)(destinationBytes + offset) = *(const unsigned int *)(sourceBytes + offset);
    *(float*)&((SprayConfiguration*)destinationBytes)->trailSampleInterval = *(const float *)(sourceBytes + SPRAY_OFFSET(trailSampleInterval));
    // Packed trail color bytes: alpha, red, green, blue.
    for (offset = 0xAC; offset <= 0xAF; ++offset) destinationBytes[offset] = sourceBytes[offset];
    *(unsigned int*)&((SprayConfiguration*)destinationBytes)->resourceId = *(const unsigned int *)(sourceBytes + SPRAY_OFFSET(resourceId));
    *(float*)&((SprayConfiguration*)destinationBytes)->trailWidthScale = *(const float *)(sourceBytes + SPRAY_OFFSET(trailWidthScale));
    ((SprayConfiguration*)destinationBytes)->trailMonolithEnabled = ((const SprayConfiguration*)sourceBytes)->trailMonolithEnabled;
    ((SprayConfiguration*)destinationBytes)->trailCameraSideEnabled = ((const SprayConfiguration*)sourceBytes)->trailCameraSideEnabled;
    // Copy remaining ranges/orientation before assigning the owned track containers.
    FxSprayVectorRange_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedRangeBC, &((const SprayConfiguration*)sourceBytes)->unresolvedRangeBC);
    *(float*)&((SprayConfiguration*)destinationBytes)->minWindScale = *(const float *)(sourceBytes + SPRAY_OFFSET(minWindScale));
    *(float*)&((SprayConfiguration*)destinationBytes)->maxWindScale = *(const float *)(sourceBytes + SPRAY_OFFSET(maxWindScale));
    ((SprayConfiguration*)destinationBytes)->unresolvedVectorDC = ((const SprayConfiguration*)sourceBytes)->unresolvedVectorDC;
    *(unsigned int*)&((SprayConfiguration*)destinationBytes)->externalOrientationAxisMask = *(const unsigned int *)(sourceBytes + SPRAY_OFFSET(externalOrientationAxisMask));
    // Track assignment coverage (configuration-relative offsets, stride 0x0C):
    // EC unresolved; 128 angular; 134 velocity rotation; 140 particle orbit;
    // 14C emitter orbit; 158 unresolved same representation; 164 spawn;
    // 170 gravity point; 17C color. Each header is {count,last,first}.
    // Deep-copy scalar and vector track lists; each list owns its nodes.
    FxSprayScalarTrackList_Assign((List1237C*)&((SprayConfiguration*)destinationBytes)->unresolvedTrackEC, (const List1237C*)&((const SprayConfiguration*)sourceBytes)->unresolvedTrackEC);
    FxSprayVectorRange_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedRangeF8, &((const SprayConfiguration*)sourceBytes)->unresolvedRangeF8);
    FxSprayVectorRange_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedRange110, &((const SprayConfiguration*)sourceBytes)->unresolvedRange110);
    FxSprayAngularSpeedTrackList_Assign((List12938*)&((SprayConfiguration*)destinationBytes)->angularSpeedTrack, (const List12938*)&((const SprayConfiguration*)sourceBytes)->angularSpeedTrack);
    FxSprayVectorTrackList_Assign((List12EF4*)&((SprayConfiguration*)destinationBytes)->velocityRotationTrack, (const List12EF4*)&((const SprayConfiguration*)sourceBytes)->velocityRotationTrack);
    FxSprayVectorTrackList_Assign((List12EF4*)&((SprayConfiguration*)destinationBytes)->particleOrbitTrack, (const List12EF4*)&((const SprayConfiguration*)sourceBytes)->particleOrbitTrack);
    FxSprayEmitterOrbitTrackList_Assign(&((SprayConfiguration*)destinationBytes)->emitterOrbitTrack, &((const SprayConfiguration*)sourceBytes)->emitterOrbitTrack);
    FxSprayEmitterOrbitTrackList_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedTrack158, &((const SprayConfiguration*)sourceBytes)->unresolvedTrack158);
    FxSpraySpawnTrackList_Assign(&((SprayConfiguration*)destinationBytes)->spawnTrack, &((const SprayConfiguration*)sourceBytes)->spawnTrack);
    FxSprayGravityPointTrackList_Assign(&((SprayConfiguration*)destinationBytes)->gravityPointTrack, &((const SprayConfiguration*)sourceBytes)->gravityPointTrack);
    FxSprayColorTrackList_Assign(&((SprayConfiguration*)destinationBytes)->colorTrack, &((const SprayConfiguration*)sourceBytes)->colorTrack);
    // Remaining list headers: 194 size, 1A0 sprite frame, 1AC vector/flags,
    // 1B8 three words, 1CC vector pair, 1D8 ID, 1E4 record/amount.
    // The latter labels describe the assigner ABI, not proven particle semantics.
    // Finish size/frame/distance controls and their lists; other reserved bytes stay untouched.
    *(unsigned int*)&((SprayConfiguration*)destinationBytes)->minInitialSize = *(const unsigned int *)(sourceBytes + SPRAY_OFFSET(minInitialSize));
    *(unsigned int*)&((SprayConfiguration*)destinationBytes)->maxInitialSize = *(const unsigned int *)(sourceBytes + SPRAY_OFFSET(maxInitialSize));
    *(float*)&((SprayConfiguration*)destinationBytes)->spriteAspectRatio = *(const float *)(sourceBytes + SPRAY_OFFSET(spriteAspectRatio));
    FxSpraySizeTrackList_Assign(&((SprayConfiguration*)destinationBytes)->sizeTrack, &((const SprayConfiguration*)sourceBytes)->sizeTrack);
    FxSpraySpriteFrameTrackList_Assign(&((SprayConfiguration*)destinationBytes)->spriteFrameTrack, &((const SprayConfiguration*)sourceBytes)->spriteFrameTrack);
    VectorFlagsRecordList_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedTrack1AC, &((const SprayConfiguration*)sourceBytes)->unresolvedTrack1AC);
    ThreeWordRecordList_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedTrack1B8, &((const SprayConfiguration*)sourceBytes)->unresolvedTrack1B8);
    *(float*)&((SprayConfiguration*)destinationBytes)->minDrawDistance = *(const float *)(sourceBytes + SPRAY_OFFSET(minDrawDistance));
    *(float*)&((SprayConfiguration*)destinationBytes)->maxDrawDistance = *(const float *)(sourceBytes + SPRAY_OFFSET(maxDrawDistance));
    VectorPairRecordList_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedTrack1CC, &((const SprayConfiguration*)sourceBytes)->unresolvedTrack1CC);
    IdRecordList_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedTrack1D8, &((const SprayConfiguration*)sourceBytes)->unresolvedTrack1D8);
    RecordAmountList_Assign(&((SprayConfiguration*)destinationBytes)->unresolvedTrack1E4, &((const SprayConfiguration*)sourceBytes)->unresolvedTrack1E4);
    ((SprayConfiguration*)destinationBytes)->circularEmission = ((const SprayConfiguration*)sourceBytes)->circularEmission;
    return dst;
}

} // extern "C"


