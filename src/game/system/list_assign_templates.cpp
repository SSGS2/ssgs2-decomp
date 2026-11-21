typedef unsigned long new_size_t;
#include "game/wvector.h"
// Translation unit 0x80115838-0x801169F4 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma cpp_exceptions on
#pragma extern_c_can_throw on
// Return caller-owned storage for placement construction without allocating.
inline void* operator new(new_size_t, void* p) { return p; }

extern "C" {

// Three circular doubly linked list specializations share {size, sentinel}.
// Link names match traversal: sentinel.next is begin, iterator ++ follows next, iterator -- follows prev.
// End is &sentinel. (The link order in memory is {prev, next}; the original symbols are not affected.)
// Each allocated node owns its inline value; there are no separately owned payloads.
// Payloads after the 8-byte links: three words (12 bytes), two vectors (24 bytes),
// or three [vector,flags] pairs plus scalar (52 bytes). Numeric units/flag bits
// are not established by these generic copy routines.
// In plain C++ each *_Assign below is std::list::operator=: overwrite the common prefix element by element,
// then append (via a temporary list, spliced in) or erase the tail. Everything else here is the exception-safe
// plumbing (guards, temp lists, iterator helpers) that the original inlining produced.
// Assign returns destinationList. Both inputs must be valid lists of the SAME
// specialization; self-assignment is allowed. If growth is needed, construct the
// temporary tail BEFORE overwriting the common prefix, then splice it into place.
// A failed tail construction therefore leaves the destination unchanged.
struct Link { Link* prev; Link* next; };
// Three opaque payload words; their game-level meaning is not established here.
struct Item : Link { unsigned word0, word1, word2; };
struct Container { unsigned size; Link sentinel; };
void* appMallocDummy__FUl(unsigned);
void appFreeUni__FPv(void*);
void __as__7WVectorFRC7WVector(void*, void*);

struct BigItem : Link {
    unsigned a[12];
    float value;
};
struct MidItem : Link { unsigned a, b, c, d, e, f; };

// Bitwise vector storage used by copy constructors (assignment uses WVector).
struct BigVector { unsigned x, y, z; };
struct VectorAndFlags { WVector vector; unsigned flags; };
struct BigRecord { VectorAndFlags slots[3]; float scalar; };
enum ListAllocationBytes { kBigNodeBytes = sizeof(BigItem),
    kTriNodeBytes = sizeof(Item), kMidNodeBytes = sizeof(MidItem) };
struct BigPayload {
    BigVector v0; unsigned flag0;
    BigVector v1; unsigned flag1;
    BigVector v2; unsigned flag2;
    float value;
    // Copy three vector bit patterns, flags and scalar with the original exception boundary.
    BigPayload(const BigPayload& p) : v0(p.v0), flag0(p.flag0),
        v1(p.v1), flag1(p.flag1), v2(p.v2), flag2(p.flag2), value(p.value) {
        try {} catch (...) { throw; }
    }
};

struct BigNode : Link {
    BigPayload payload;
    // Construct the inline three-vector payload from a borrowed source.
    BigNode(const BigPayload& p) : payload(p) {}
    // Allocate one three-vector node through the game allocator.
    static void* operator new(new_size_t) { return appMallocDummy__FUl(kBigNodeBytes); }
    // Free one node through the game allocator.
    static void operator delete(void* p) { appFreeUni__FPv(p); }
};

#pragma opt_strength_reduction off
#pragma opt_propagation off
// Own an unlinked node until payload construction succeeds.
struct BigGuard {
    Link* position;
    Link* node;
    // Guard an unlinked allocation until ownership transfers to the list.
    BigGuard(Link* p, Link* n) : position(p), node(n) {}
    // Free the guarded node if it was never linked into its destination list.
    ~BigGuard() { if (node) appFreeUni__FPv(node); }
    // Transfer the guarded node to the caller and disable allocation cleanup.
    Link* release() { Link* p = node; node = 0; return p; }
};

// Allocate and copy one three-vector record, then link it into the list.
static inline void insertBig(Container* list, Link* position, BigItem* item) {
    volatile char allocationTag = 0;
    Link* fresh = (Link*)appMallocDummy__FUl(kBigNodeBytes);
    Link* volatile allocatedBegin = fresh;
    Link* volatile allocatedEnd = fresh;
    BigGuard guard(position, fresh);
    new (fresh + 1) BigPayload(*(BigPayload*)item->a);
    fresh = guard.release();
    position->prev->next = fresh;
    fresh->prev = position->prev;
    position->prev = fresh;
    fresh->next = position;
    ++list->size;
}

// Empty-sentinel construction and reverse-order destruction for temporary nodes.
struct BigBase : Container {
    // Initialize count zero and link both ends of an empty sentinel ring.
    BigBase() { size = 0; sentinel.next = &sentinel; sentinel.prev = &sentinel; }
    // Free every node still owned by this temporary list.
    ~BigBase() {
        Link* first = sentinel.next;
        Link* end = &sentinel;
        if (first != end) {
            Link* last = end->prev;
            first->prev->next = last->next;
            last->next->prev = first->prev;
            while (first != end) {
                Link* node = first;
                first = first->next;
                appFreeUni__FPv(node);
                --size;
            }
        }
    }
};

// Build the extra source tail separately so exceptions leave the destination intact.
struct BigTemp : BigBase {
    // Copy the extra source tail into an independently owned temporary list.
    BigTemp(Link* begin, Link* end) {
        volatile char allocatorTag = 0;
        Link* volatile p = begin;
        Link* volatile finish = end;
        if (begin != end) {
            do {
                {
                BigItem* item = (BigItem*)p;
                insertBig(this, &sentinel, item);
                }
                p = p->next;
            } while (p != finish);
        }
    }
    // Delegate remaining node cleanup to the temporary-list base.
    ~BigTemp() {}
};

#pragma opt_strength_reduction off
#pragma opt_propagation off
struct BigIterator {
    Link* node;
    // Bind an iterator to a borrowed node or the end sentinel.
    BigIterator(Link* p) : node(p) {}
    // Advance to the next logical node through the next link.
    BigIterator& operator++() { node = node->next; return *this; }
    // Move to the previous logical node through the prev link.
    BigIterator& operator--() { node = node->prev; return *this; }
};
// Advance a signed distance following next (positive) or prev (negative) links.
static inline void advanceBig(BigIterator& p, int n) {
    volatile char iteratorTag = 0;
    if (n >= 0) { while (n > 0) { ++p; --n; } }
    else { while (n < 0) { --p; ++n; } }
}

// Assign the three vectors, their flags, and the trailing scalar in field order.
static inline void assignBig(BigItem* destinationNode, BigItem* sourceNode) {
    BigRecord* destination = (BigRecord*)destinationNode->a;
    BigRecord* source = (BigRecord*)sourceNode->a;
    destination->slots[0].vector = source->slots[0].vector;
    destination->slots[0].flags = source->slots[0].flags;
    destination->slots[1].vector = source->slots[1].vector;
    destination->slots[1].flags = source->slots[1].flags;
    destination->slots[2].vector = source->slots[2].vector;
    destination->slots[2].flags = source->slots[2].flags;
    destination->scalar = source->scalar;
}

// Overwrite existing destination records for the half-open source range.
static inline Link* copyBigRange(Link* destination, Link* begin, Link* finish) {
    Link* volatile b = begin;
    Link* volatile end = finish;
    Link* a = destination;
    while (b != end) {
        assignBig((BigItem*)a, (BigItem*)b);
        a = a->next;
        b = b->next;
    }
    return a;
}

// Keep a reference to the smaller count, as in the original template helper.
static inline const unsigned& minBig(const unsigned& a, const unsigned& b) {
    return b < a ? b : a;
}
struct BigBounds { Link* split; Link* end; Link* begin; Link* beginEnd; };
// Assign a three-vector-record list, reusing nodes and splicing or deleting the tail.
// The source stays borrowed; newly spliced nodes become destination-owned.
Container* VectorFlagsRecordList_Assign(Container* destinationList, Container* sourceList) {
    if (destinationList != sourceList) {
        volatile BigBounds bounds; 
        const unsigned commonSize = minBig((unsigned)destinationList->size, (unsigned)sourceList->size);
        unsigned sourceSize = sourceList->size;
        int commonCount = commonSize;
        BigIterator cursor(0);
        // Phase 1: locate the source split from the nearer end.
        if (commonCount <= sourceSize / 2) {
            cursor.node = sourceList->sentinel.next;
            advanceBig(cursor, commonCount);
        } else {
            cursor.node = &sourceList->sentinel;

            advanceBig(cursor, (commonCount - sourceSize));
        }
        Link* srcEnd = &sourceList->sentinel;
        Link* sourceBegin = sourceList->sentinel.next;

        bounds.split = cursor.node;
        bounds.end = srcEnd;
        bounds.beginEnd = srcEnd;
        Link* volatile src = cursor.node;
        bounds.begin = sourceBegin;
        // Tail phase: grow through a guarded temporary list, or erase excess nodes.
        if (cursor.node != srcEnd) {
            BigTemp extraNodes(cursor.node, srcEnd);

            // Copy the shared prefix only after extra tail construction succeeds.
            Link* destinationTail = copyBigRange(destinationList->sentinel.next, bounds.begin, bounds.split);
            // Splice transfers node ownership; clear the temporary sentinel/count.
            if (extraNodes.size) {
                Link* last = extraNodes.sentinel.prev;
                Link* first = extraNodes.sentinel.next;
                extraNodes.sentinel.next = &extraNodes.sentinel;
                extraNodes.sentinel.prev = &extraNodes.sentinel;
                destinationTail->prev->next = first;
                first->prev = destinationTail->prev;
                destinationTail->prev = last;
                last->next = destinationTail;
                destinationList->size = (destinationList->size + extraNodes.size);
                extraNodes.size = 0;
            }
        } else {
            Link* destinationTail = copyBigRange(destinationList->sentinel.next, sourceBegin, cursor.node);
            Link* dstEnd = &destinationList->sentinel;
            Link* volatile eraseCursor = destinationTail;
            if (dstEnd != destinationTail) {
                // Unlink the destination-only tail, then free nodes and reduce size.
                Link* last = dstEnd->prev;
                destinationTail->prev->next = last->next;
                last->next->prev = destinationTail->prev;
                Link* node;
                while ((node = eraseCursor) != dstEnd) {
                    eraseCursor = node->next;
                    appFreeUni__FPv(node);
                    destinationList->size -= 1;
                }
            }
        }
    }
    return destinationList;
}

struct TriPayload {
    struct Pair { unsigned a, b; } pair;
    unsigned c;
    // Copy the three opaque payload words with the original exception boundary.
    TriPayload(const TriPayload& p) : pair(p.pair), c(p.c) {
        try {} catch (...) { throw; }
    }
};

struct TriTag { char value; };

struct TriNode : Link {
    unsigned a, b, c;
    // Initialize the three-word payload; linkage is set when inserted.
    TriNode(unsigned x, unsigned y, unsigned z) : a(x), b(y), c(z) {}
    // Allocate one three-word node through the game allocator.
    static void* operator new(new_size_t) { return appMallocDummy__FUl(kTriNodeBytes); }
    // Free one node through the game allocator.
    static void operator delete(void* p) { appFreeUni__FPv(p); }
};

// Own an unlinked three-word or two-vector node during construction.
struct TriGuard {
    Link* position;
    Link* node;
    // Guard an unlinked allocation until ownership transfers to the list.
    TriGuard(Link* p, Link* n) : position(p), node(n) {}
    // Free the guarded node if it was never linked into its destination list.
    ~TriGuard() { if (node) appFreeUni__FPv(node); }
    // Transfer the guarded node to the caller and disable allocation cleanup.
    Link* release() { Link* p = node; node = 0; return p; }
};

#pragma opt_common_subs off
// Volatile stores preserve the original allocator/template construction sequence.
struct TriBase : Container {
    // Initialize count zero and link both ends of an empty sentinel ring.
    TriBase() {
        Link* end = &sentinel;
        *(volatile unsigned*)&size = 0;
        *(Link* volatile*)&sentinel.next = end;
        *(Link* volatile*)&sentinel.prev = end;
    }
    // Free every node still owned by this temporary list.
    ~TriBase() {
        Link* first = sentinel.next;
        Link* end = &sentinel;
        if (first != end) {
            Link* last = end->prev;
            first->prev->next = last->next;
            last->next->prev = first->prev;
            while (first != end) {
                Link* node = first;
                first = first->next;
                appFreeUni__FPv(node);
                --size;
            }
        }
    }
};

#pragma opt_common_subs off
// Copy a three-word payload into a guarded allocation before linking the node.
static inline void insertTri(Container* list, Link* position, Item* item,
    Link* volatile& allocatedBegin, Link* volatile& allocatedEnd);

// Own the extra three-word nodes until they can be spliced into the destination.
struct TriTemp : TriBase {
    // Copy the extra source tail into an independently owned temporary list.
    TriTemp(Link* begin, Link* end, Link* volatile& p, Link* volatile& finish,
        Link* volatile& allocatedBegin, Link* volatile& allocatedEnd,
        volatile TriTag& allocatorTag) {
        allocatorTag.value = 0;
        finish = end;
        p = begin;
        if (begin != end) {
            while (p != finish) {
                {
                Item* item = (Item*)p;
                insertTri(this, &sentinel, item, allocatedBegin, allocatedEnd);
                }
                p = p->next;
            }
        }
    }
    // Delegate remaining node cleanup to the temporary-list base.
    ~TriTemp() {}
};

#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs on
struct TriIterator {
    Link* node;
    // Bind an iterator to a borrowed node or the end sentinel.
    TriIterator(Link* p) : node(p) {}
    // Advance to the next logical node through the next link.
    TriIterator& operator++() { node = node->next; return *this; }
    // Move to the previous logical node through the prev link.
    TriIterator& operator--() { node = node->prev; return *this; }
};
// Advance an iterator while preserving the template's allocator-tag store.
static inline void advanceTri(TriIterator& p, int n, volatile TriTag& iteratorTag) {
    iteratorTag.value = 0;
    if (n >= 0) { while (n > 0) { ++p; --n; } }
    else { while (n < 0) { --p; ++n; } }
}

// Find the common-length boundary by walking from the nearer end of the source.
static inline Link* splitTri(Container* sourceList, const unsigned& smaller,
    volatile TriTag& firstTag, volatile TriTag& secondTag) {
    unsigned sourceSize = sourceList->size;
    int common = smaller;
    TriIterator cursor(0);
    if (common <= sourceSize / 2) {
        cursor.node = sourceList->sentinel.next;
        int count = common;
        advanceTri(cursor, count, firstTag);
    } else {
        cursor.node = &sourceList->sentinel;
        int count = common;
        advanceTri(cursor, count - sourceSize, secondTag);
    }
    return cursor.node;
}

// Overwrite existing destination nodes with three-word records.
static inline Link* copyTriRange(Link* begin, Link* finish, Container* destinationList,
    Link* volatile& b, Link* volatile& end) {
    end = finish;
    Link* a = destinationList->sentinel.next;
    b = begin;
    while (b != end) {
        Item* x = (Item*)a;
        Item* y = (Item*)b;
        x->word0 = y->word0; x->word1 = y->word1; x->word2 = y->word2;
        a = a->next;
        b = b->next;
    }
    return a;
}

// Load the saved source bounds before copying the common prefix.
static inline Link* copyTriBounds(Container* destinationList, Link* volatile& begin,
    Link* volatile& finish, Link* volatile& b, Link* volatile& end) {
    Link* last = finish;
    Link* first = begin;
    return copyTriRange(first, last, destinationList, b, end);
}

// Copy a three-word payload into a guarded allocation before linking the node.
static inline void insertTri(Container* list, Link* position, Item* item,
    Link* volatile& allocatedBegin, Link* volatile& allocatedEnd) {
    volatile TriTag allocationTag;
    allocationTag.value = 0;
    Link* fresh = (Link*)appMallocDummy__FUl(kTriNodeBytes);
    allocatedBegin = fresh;
    allocatedEnd = fresh;
    TriGuard guard(position, fresh);
    position = guard.position;
    new (fresh + 1) TriPayload(*(TriPayload*)&item->word0);
    fresh = guard.release();
    position->prev->next = fresh;
    fresh->prev = position->prev;
    position->prev = fresh;
    fresh->next = position;
    ++list->size;
}

#pragma opt_common_subs on
#pragma opt_propagation on
#pragma use_lmw_stmw on
// Capture both list sizes and return their smaller count.
static inline const unsigned& minTri(Container* destinationList, Container* sourceList,
    unsigned& dstSize, unsigned& srcSize) {
    srcSize = sourceList->size;
    dstSize = destinationList->size;
    return minBig(dstSize, srcSize);
}
// Assign a three-word-record list, reusing nodes and splicing or deleting the tail.
// The source stays borrowed; newly spliced nodes become destination-owned.
Container* ThreeWordRecordList_Assign(Container* destinationList, Container* sourceList) {
    if (destinationList != sourceList) {
        volatile TriTag firstTag;
        volatile TriTag secondTag;
        volatile TriTag ctorTag;
        Link* volatile source;
        unsigned dstSize;
        unsigned srcSize;
        Link* volatile boundsBeginEnd;
        Link* volatile boundsBegin;
        Link* volatile boundsEnd;
        Link* volatile boundsSplit;
        Link* volatile allocatedEnd;
        Link* volatile allocatedBegin;
        Link* volatile finish;
        Link* volatile p;
        Link* volatile elseEnd;
        Link* volatile elseBegin;
        Link* volatile eraseCursor;
        Link* volatile copyEnd;
        Link* volatile copyBegin;
        const unsigned& commonSize = minTri(destinationList, sourceList, dstSize, srcSize);
        TriIterator cursor(splitTri(sourceList, commonSize, firstTag, secondTag));
        Link* srcEnd = &sourceList->sentinel;
        Link* sourceBegin = sourceList->sentinel.next;
        boundsSplit = cursor.node;
        boundsEnd = srcEnd;
        boundsBeginEnd = srcEnd;
        source = cursor.node;
        boundsBegin = sourceBegin;
        // Tail phase: grow through a guarded temporary list, or erase excess nodes.
        if (cursor.node != srcEnd) {
            TriTemp extraNodes(cursor.node, srcEnd, p, finish, allocatedBegin, allocatedEnd, ctorTag);
            // Copy the shared prefix only after extra tail construction succeeds.
            Link* destinationTail = copyTriBounds(destinationList, boundsBegin, boundsSplit,
                copyBegin, copyEnd);
            // Splice transfers node ownership; clear the temporary sentinel/count.
            if (extraNodes.size) {
                Link* last = extraNodes.sentinel.prev;
                Link* first = extraNodes.sentinel.next;
                extraNodes.sentinel.next = &extraNodes.sentinel;
                extraNodes.sentinel.prev = &extraNodes.sentinel;
                destinationTail->prev->next = first;
                first->prev = destinationTail->prev;
                destinationTail->prev = last;
                last->next = destinationTail;
                destinationList->size += extraNodes.size;
                extraNodes.size = 0;
            }
        } else {
            Link* destinationTail = copyTriRange(sourceBegin, cursor.node, destinationList,
                elseBegin, elseEnd);
            Link* dstEnd = &destinationList->sentinel;
            eraseCursor = destinationTail;
            if (destinationTail != dstEnd) {
                // Unlink the destination-only tail, then free nodes and reduce size.
                Link* last = dstEnd->prev;
                destinationTail->prev->next = last->next;
                last->next->prev = destinationTail->prev;
                Link* node;
                while ((node = eraseCursor) != dstEnd) {
                    eraseCursor = node->next;
                    appFreeUni__FPv(node);
                    --destinationList->size;
                }
            }
        }
    }
    return destinationList;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma opt_propagation reset
#pragma cpp_exceptions reset

#pragma cpp_exceptions on
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs off
struct MidPayload {
    BigVector v0, v1;
    // Copy the two vector bit patterns with the original exception boundary.
    MidPayload(const MidPayload& p) : v0(p.v0), v1(p.v1) {
        try {} catch (...) { throw; }
    }
};

// Construct a two-vector payload in guarded storage, then link the node.
static inline void insertMid(Container* list, Link* position, MidItem* item,
    Link* volatile& allocatedBegin, Link* volatile& allocatedEnd) {
    volatile TriTag allocationTag;
    allocationTag.value = 0;
    Link* fresh = (Link*)appMallocDummy__FUl(kMidNodeBytes);
    allocatedBegin = fresh;
    allocatedEnd = fresh;
    TriGuard guard(position, fresh);
    position = guard.position;
    new (fresh + 1) MidPayload(*(MidPayload*)&item->a);
    fresh = guard.release();
    position->prev->next = fresh;
    fresh->prev = position->prev;
    position->prev = fresh;
    fresh->next = position;
    ++list->size;
}

// Clear an owned temporary list without altering payload assignment semantics.
struct MidBase : Container {
    // Initialize count zero and link both ends of an empty sentinel ring.
    MidBase() {
        Link* end = &sentinel;
        *(volatile unsigned*)&size = 0;
        *(Link* volatile*)&sentinel.next = end;
        *(Link* volatile*)&sentinel.prev = end;
    }
    // Free every node still owned by this temporary list.
    ~MidBase() {
        Link* first = sentinel.next;
        Link* end = &sentinel;
        if (first != end) {
            Link* last = end->prev;
            first->prev->next = last->next;
            last->next->prev = first->prev;
            while (first != end) {
                Link* node = first;
                first = first->next;
                appFreeUni__FPv(node);
                --size;
            }
        }
    }
};

// Own the extra two-vector nodes until assignment commits their links.
struct MidTemp : MidBase {
    // Copy the extra source tail into an independently owned temporary list.
    MidTemp(Link* begin, Link* end, Link* volatile& p, Link* volatile& finish,
        Link* volatile& allocatedBegin, Link* volatile& allocatedEnd,
        volatile TriTag& allocatorTag) {
        allocatorTag.value = 0;
        finish = end;
        p = begin;
        if (begin != end) {
            while (p != finish) {
                MidItem* item = (MidItem*)p;
                insertMid(this, &sentinel, item, allocatedBegin, allocatedEnd);
                p = p->next;
            }
        }
    }
    // Delegate remaining node cleanup to the temporary-list base.
    ~MidTemp() {}
};

#pragma opt_common_subs on
// Assign both vectors for each node in the common source range.
static inline Link* copyMidRange(Link* begin, Link* finish, Container* destinationList,
    Link* volatile& b, Link* volatile& end) {
    end = finish;
    Link* a = destinationList->sentinel.next;
    b = begin;
    while (b != end) {
        unsigned* y = &((MidItem*)b)->a;
        unsigned* x = &((MidItem*)a)->a;
        __as__7WVectorFRC7WVector(x, y);
        __as__7WVectorFRC7WVector(x + 3, y + 3);
        a = a->next;
        b = b->next;
    }
    return a;
}

// Read the volatile saved bounds before assigning the shared prefix.
static inline Link* copyMidBounds(Container* destinationList, Link* volatile& begin,
    Link* volatile& finish, Link* volatile& b, Link* volatile& end) {
    Link* last = finish;
    Link* first = begin;
    return copyMidRange(first, last, destinationList, b, end);
}

// Assign the two WVector values stored consecutively in a node payload.
static inline void assignMid(unsigned* sourcePayload, unsigned* destinationPayload) {
    *(WVector*)destinationPayload = *(WVector*)sourcePayload;
    *(WVector*)(destinationPayload + 3) = *(WVector*)(sourcePayload + 3);
}

// Return the first data node, or the sentinel for an empty list.
static inline Link* midBegin(Container* list) { return list->sentinel.next; }

// Overwrite destination nodes with two-vector records without allocating.
static inline Link* copyMidElse(Link* destination, Link* begin, Link* finish,
    Link* volatile& b, Link* volatile& end) {
    end = finish;
    TriIterator a(destination);
    b = begin;
    while (b != end) {
        assignMid(&((MidItem*)b)->a, &((MidItem*)a.node)->a);
        ++a;
        b = b->next;
    }
    return a.node;
}

#pragma opt_propagation on
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Read the current list size for the two-vector assignment specialization.
static inline unsigned midSize(const Container* list) { return list->size; }
// Read source and destination sizes before choosing the shared prefix length.
static inline const unsigned& minMid(Container* destinationList, Container* sourceList,
    unsigned& dst, unsigned& src) {
    src = midSize(sourceList);
    dst = midSize(destinationList);
    return src < dst ? src : dst;
}
#pragma opt_common_subs on
// Assign a two-vector-record list, reusing nodes and splicing or deleting the tail.
// The source stays borrowed; newly spliced nodes become destination-owned.
Container* VectorPairRecordList_Assign(Container* destinationList, Container* sourceList) {
    if (destinationList != sourceList) {
        register Link* destinationTail;
        volatile TriTag firstTag;
        volatile TriTag secondTag;
        volatile TriTag ctorTag;
        Link* volatile source;
        unsigned dstSize;
        unsigned srcSize;
        Link* volatile boundsBeginEnd;
        Link* volatile boundsBegin;
        Link* volatile boundsEnd;
        Link* volatile boundsSplit;
        Link* volatile allocatedEnd;
        Link* volatile allocatedBegin;
        Link* volatile finish;
        Link* volatile p;
        Link* volatile elseEnd;
        Link* volatile elseBegin;
        Link* volatile eraseCursor;
        Link* volatile copyEnd;
        Link* volatile copyBegin;
        const unsigned& commonSize = minMid(destinationList, sourceList, dstSize, srcSize);
        TriIterator cursor(splitTri(sourceList, commonSize, firstTag, secondTag));
        Link* srcEnd = &sourceList->sentinel;
        Link* sourceBegin = sourceList->sentinel.next;
        boundsSplit = cursor.node;
        boundsEnd = srcEnd;
        boundsBeginEnd = srcEnd;
        source = cursor.node;
        boundsBegin = sourceBegin;
        // Tail phase: grow through a guarded temporary list, or erase excess nodes.
        if (cursor.node != srcEnd) {
            MidTemp extraNodes(cursor.node, srcEnd, p, finish, allocatedBegin, allocatedEnd, ctorTag);
            // Copy the shared prefix only after extra tail construction succeeds.
            destinationTail = copyMidBounds(destinationList, boundsBegin, boundsSplit,
                copyBegin, copyEnd);
            // Splice transfers node ownership; clear the temporary sentinel/count.
            if (extraNodes.size) {
                Link* last;
                Link* first;
                first = *(Link* volatile*)&extraNodes.sentinel.next;
                last = *(Link* volatile*)&extraNodes.sentinel.prev;
                *(Link* volatile*)&extraNodes.sentinel.next = &extraNodes.sentinel;
                *(Link* volatile*)&extraNodes.sentinel.prev = &extraNodes.sentinel;
                destinationTail->prev->next = first;
                first->prev = destinationTail->prev;
                destinationTail->prev = last;
                last->next = destinationTail;
                destinationList->size += extraNodes.size;
                extraNodes.size = 0;
            }
        } else {
            destinationTail = copyMidElse(midBegin(destinationList), sourceBegin, cursor.node,
                elseBegin, elseEnd);
            Link* dstEnd = &destinationList->sentinel;
            eraseCursor = destinationTail;
            if (destinationTail != dstEnd) {
                // Unlink the destination-only tail, then free nodes and reduce size.
                Link* last = dstEnd->prev;
                destinationTail->prev->next = last->next;
                last->next->prev = destinationTail->prev;
                Link* node;
                while ((node = eraseCursor) != dstEnd) {
                    eraseCursor = node->next;
                    appFreeUni__FPv(node);
                    --destinationList->size;
                }
            }
        }
    }
    return destinationList;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma opt_propagation reset
#pragma cpp_exceptions reset

typedef char big_record_size[(sizeof(BigRecord) == 0x34) ? 1 : -1];
typedef char big_node_size[(kBigNodeBytes == 0x3C) ? 1 : -1];
typedef char tri_node_size[(kTriNodeBytes == 0x14) ? 1 : -1];
typedef char mid_node_size[(kMidNodeBytes == 0x20) ? 1 : -1];

} // extern "C"
