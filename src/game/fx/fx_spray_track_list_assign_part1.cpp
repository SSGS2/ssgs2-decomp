// Copy assignment for three spray track lists: spawn, emitter orbit and gravity point.
// Each is a circular list with a 12-byte header. Nodes are reused, and only the tail is grown or freed.

#include "game/wvector.h"

extern "C" {

// Three owning circular-list specializations, with a 12-byte header:
// +0 count, +4 sentinel.prev (tail), +8 sentinel.next (first).
// End is the embedded pair at header+4; an empty list links both words to End.
// Nodes start with prev/next at +0/+4 and inline payload at +8. Spawn and
// gravity nodes are 0x20 bytes; orbit nodes are 0x30. All active Node* views
// store prev first. OrbitTrackList::sentinelPrev is that prev link,
// so &list->sentinelPrev is End, whereas list->sentinelPrev is the tail node.
// Payload copies do not establish ownership of pointed-to resources: meanings
// of opaque words are unresolved here. appFreeUni frees the node allocation.
// Assignment reuses the common prefix, stages a growing tail before touching
// destination values, then splices it; shrinking detaches and frees excess nodes.
// All Assign calls support self-assignment and return self; source is borrowed.
//
// Assignment map for all three specializations (m = min(oldCount, sourceCount)):
// source: [ reusable payloads 0..m ) [ missing suffix m..sourceCount ) End
// self:   [ reused nodes     0..m ) [ surplus nodes m..oldCount    ) End
// Growth: stage the missing suffix -> overwrite m payloads -> splice at self End.
// Shrink: overwrite m payloads -> detach surplus -> free detached nodes.
// savedStart/tailFirst mark source[m]; sourceFirst marks source[0]. growFirst /
// shrinkFirst are the traversal cursors, growStop / shrinkStop their boundaries.
// The volatile aliases and allocator-tag stores reproduce iterator/unwind code;
// they neither add list nodes nor encode additional gameplay state.

struct ListNode80114108 {
    ListNode80114108* prev;
    ListNode80114108* next;
    unsigned int a;
    unsigned int b;
    unsigned int c;
    float x;
    float y;
    float z;
};

struct GravityPointTrackList {
    unsigned int size;
    ListNode80114108* sentinelPrev;
    ListNode80114108* begin;
    // Read the logical gravity-node count, excluding the embedded sentinel.
    unsigned int count() const {
        return size;
    }
};

void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void __as__7WVectorFRC7WVector(void*, const void*);
void dtor_801102B4(void*);
void dtor_80110364(void*);
void dtor_80110204(void*);
void dtor_80110150(void*);
void* appMallocDummy__FUl(unsigned int);

struct SpawnTrackPayload {
    // Payload +0/+4/+8 are integer words, +0xC is float, +0x10 integer,
    // +0x14 float. This assignment unit does not resolve their gameplay roles.
    int a;
    int b;
    int c;
    float d;
    int e;
    float f;
};

struct SpawnTrackLink {
    SpawnTrackLink* prev;
    SpawnTrackLink* next;
};

struct SpawnTrackNode : SpawnTrackLink {
    SpawnTrackPayload v;
};

struct SpawnTrackList {
    unsigned int size;
    SpawnTrackLink end;
    // Read the logical spawn-node count, excluding the embedded sentinel.
    unsigned int count() const {
        return size;
    }
};

struct BidirTag80113B1C {
    volatile char unused;
    char padding[3];
};

struct SpawnTrackTempBase {
    volatile unsigned int size;
    struct {
        SpawnTrackLink* volatile prev;
        SpawnTrackLink* volatile next;
    } end;
    // Free any staged spawn suffix not transferred to the destination list.
    ~SpawnTrackTempBase() {
        SpawnTrackLink* node;
        SpawnTrackLink* e;
        e = (SpawnTrackLink*)&end;
        node = end.next;
        if (node != e) {
            SpawnTrackLink* last = e->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            while (node != e) {
                SpawnTrackLink* cur = node;
                node = node->next;
                appFreeUni__FPv(cur);
                --size;
            }
        }
    }
};

struct NodeGuard80113B1C {
    SpawnTrackLink* volatile end;
    SpawnTrackNode* node;
    // Own a new spawn allocation until payload construction succeeds.
    NodeGuard80113B1C(SpawnTrackLink* e, SpawnTrackNode* n) : end(e), node(n) {}
    // Free the node if construction failed before ownership transfer.
    ~NodeGuard80113B1C() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// Advance by signed n; caller guarantees the destination remains in the valid range.
static inline void advance80113B1C(SpawnTrackLink*& it, int n, const BidirTag80113B1C&) {
    if (n >= 0) {
        while (n > 0) {
            it = it->next;
            --n;
        }
    } else {
        while (n < 0) {
            it = it->prev;
            ++n;
        }
    }
}

struct Range80113B1C {
    SpawnTrackLink* volatile first;
    SpawnTrackLink* volatile last;
};

// Borrow the smaller count; preserve the library's reference-return convention.
static inline const unsigned int& min80113B1C(const unsigned int& selfCount, const unsigned int& otherCount) {
    return otherCount < selfCount ? otherCount : selfCount;
}

// Copy the six spawn payload words into a reused destination node.
static inline void copyValue80113B1C(const SpawnTrackNode* src, SpawnTrackNode* dst) {
    const SpawnTrackPayload* from = &src->v;
    SpawnTrackPayload* to = &dst->v;
    to->a = from->a;
    to->b = from->b;
    to->c = from->c;
    to->d = from->d;
    to->e = from->e;
    to->f = from->f;
}

// Assign a spawn payload while retaining the destination's links.
static inline void assignValue80113B1C(SpawnTrackNode* dst, const SpawnTrackNode* src) {
    SpawnTrackPayload* to = &dst->v;
    const SpawnTrackPayload* from = &src->v;
    to->a = from->a;
    to->b = from->b;
    to->c = from->c;
    to->d = from->d;
    to->e = from->e;
    to->f = from->f;
}

// Initialize a newly allocated spawn payload with the original unwind boundary.
static inline void constructValue80113B1C(const SpawnTrackNode* src, SpawnTrackNode* dst) {
    unsigned char* to = (unsigned char*)&dst->v;
    const unsigned char* from = (const unsigned char*)&src->v;
    if (to) {
        unsigned int a = *(const unsigned int*)(from + 0x0);
        unsigned int b = *(const unsigned int*)(from + 0x4);
        try {
            *(unsigned int*)(to + 0x0) = a;
            *(unsigned int*)(to + 0x4) = b;
            struct Words4 {
                unsigned int v[4];
            };
            *(Words4*)(to + 8) = *(const Words4*)(from + 8);
        } catch (...) {
            throw;
        }
    }
}

struct SpawnTrackTempList : SpawnTrackTempBase {
    // Initialize the temporary's empty embedded sentinel and logical count.
    void initialize(unsigned int count, SpawnTrackLink* endp) {
        end.next = endp;
        size = count;
        end.prev = endp;
    }
    // Copy [first,last) into a separately owned suffix, updating unwind cursors.
    SpawnTrackTempList(SpawnTrackLink* first, SpawnTrackLink* last, SpawnTrackLink* volatile& cur,
                       SpawnTrackLink* volatile& lastCopy, SpawnTrackNode* volatile& allocation,
                       SpawnTrackNode* volatile& constructed) {
        struct Tag {
            volatile char value;
            char padding[3];
        } innerTag, tag;
        SpawnTrackLink* endp;
        unsigned int count;
        endp = (SpawnTrackLink*)&end;
        count = 0;
        initialize(count, endp);
        SpawnTrackTempBase* target = this;
        tag.value = 0;
        lastCopy = last;
        cur = first;
        if (first != last) {
            while (cur != lastCopy) {
                {
                    SpawnTrackLink* insertion;
                    SpawnTrackNode* source;
                    source = (SpawnTrackNode*)cur;
                    innerTag.value = 0;
                    insertion = (SpawnTrackLink*)&target->end;
                    SpawnTrackNode* n = (SpawnTrackNode*)appMallocDummy__FUl(0x20);
                    allocation = n;
                    constructed = n;
                    NodeGuard80113B1C guard(insertion, n);
                    constructValue80113B1C(source, n);
                    SpawnTrackNode* owned = guard.node;
                    guard.node = 0;
                    n = owned;
                    insertion->prev->next = n;
                    n->prev = insertion->prev;
                    insertion->prev = n;
                    n->next = insertion;
                    ++target->size;
                }
                cur = cur->next;
            }
        }
    }
};

// Locate the common-prefix boundary using the nearer end of the source list.
static inline SpawnTrackLink* select80113B1C(const SpawnTrackList* other, const volatile unsigned int& minimum) {
    BidirTag80113B1C tagB;
    BidirTag80113B1C tagA;
    unsigned int otherSize = ((const volatile SpawnTrackList*)other)->size;
    unsigned int m = minimum;
    SpawnTrackLink* start;
    if (m <= otherSize / 2) {
        tagA.unused = 0;
        start = other->end.next;
        advance80113B1C(start, (int)m, tagA);
    } else {
        tagB.unused = 0;
        start = (SpawnTrackLink*)&other->end;
        advance80113B1C(start, (int)(m - otherSize), tagB);
    }
    return start;
}

// Copy the spawn track, reusing nodes and growing or shrinking only its tail.
SpawnTrackList* FxSpraySpawnTrackList_Assign(SpawnTrackList* self, const SpawnTrackList* other) {
    if (self != other) {
        SpawnTrackLink* volatile savedStart;
        unsigned int selfCount;
        unsigned int otherCount;
        SpawnTrackLink* volatile sourceLast;
        SpawnTrackLink* volatile sourceFirst;
        SpawnTrackLink* volatile tailLast;
        SpawnTrackLink* volatile tailFirst;
        SpawnTrackNode* volatile constructed;
        SpawnTrackNode* volatile allocation;
        SpawnTrackLink* volatile lastCopy;
        SpawnTrackLink* volatile cur;
        SpawnTrackLink* volatile shrinkStop;
        SpawnTrackLink* volatile shrinkFirst;
        SpawnTrackLink* volatile erase;
        SpawnTrackLink* volatile growStop;
        SpawnTrackLink* volatile growFirst;
        otherCount = other->count();
        selfCount = self->count();
        const volatile unsigned int& minimum = min80113B1C(selfCount, otherCount);
        SpawnTrackLink* start = select80113B1C(other, minimum);

        SpawnTrackLink* end = (SpawnTrackLink*)&other->end;
        SpawnTrackLink* sourceStart = other->end.next;
        tailFirst = start;
        tailLast = end;
        sourceLast = end;
        savedStart = start;
        sourceFirst = sourceStart;

        // Growth: stage source[minimum..end) under a temporary owning sentinel.
        if (start != end) {
            SpawnTrackTempList temp(start, end, cur, lastCopy, allocation, constructed);
            SpawnTrackLink* firstValue;
            SpawnTrackLink* stopValue;
            SpawnTrackLink* lastT;
            SpawnTrackLink* firstT;
            SpawnTrackNode* s;
            SpawnTrackLink* dst;
            stopValue = tailFirst;
            firstValue = sourceFirst;
            SpawnTrackLink* volatile& stopCopy = growStop;
            stopCopy = stopValue;
            dst = self->end.next;
            SpawnTrackLink* volatile& firstCopy = growFirst;
            firstCopy = firstValue;
            // Tail allocation succeeded: overwrite the reused common prefix.
            while (firstCopy != stopCopy) {
                s = (SpawnTrackNode*)firstCopy;
                copyValue80113B1C(s, (SpawnTrackNode*)dst);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            // Splice staged nodes before destination End; temp then owns none.
            if (temp.size != 0) {
                SpawnTrackLink* endp = (SpawnTrackLink*)&temp.end;
                firstT = endp->next;
                lastT = endp->prev;
                endp->next = endp;
                endp->prev = endp;
                dst->prev->next = firstT;
                firstT->prev = dst->prev;
                dst->prev = lastT;
                lastT->next = dst;
                self->size += temp.size;
                temp.size = 0;
            }
        } else {
            // Equal/shrinking: assign the source prefix, then free excess nodes.
            SpawnTrackNode* s;
            SpawnTrackLink* dst;
            SpawnTrackLink* volatile& stopCopy = shrinkStop;
            stopCopy = start;
            dst = self->end.next;
            SpawnTrackLink* volatile& firstCopy = shrinkFirst;
            firstCopy = sourceStart;
            while (firstCopy != stopCopy) {
                s = (SpawnTrackNode*)firstCopy;
                assignValue80113B1C((SpawnTrackNode*)dst, s);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            SpawnTrackLink* e = (SpawnTrackLink*)&self->end;
            erase = dst;
            if (dst != e) {
                SpawnTrackLink* lastNode = e->prev;
                dst->prev->next = lastNode->next;
                lastNode->next->prev = dst->prev;
                SpawnTrackLink* cur;
                while ((cur = erase) != e) {
                    erase = cur->next;
                    appFreeUni__FPv(cur);
                    --self->size;
                }
            }
        }
    }
    return self;
}

struct OrbitTrackLink {
    OrbitTrackLink* prev;
    OrbitTrackLink* next;
};

struct OrbitTrackNode : OrbitTrackLink {
    // +0x00/+0x0C: two WVector-compatible triples; +0x18..+0x27: four
    // copied words: uint at +18, float at +1C, uint at +20, float at +24.
    // Gameplay meanings and coordinate spaces are unresolved; copying is bitwise.
    // Offsets are relative to value, not the node's link prefix.
    unsigned char value[0x28];
};

struct OrbitTrackList {
    unsigned int size;
    OrbitTrackLink* sentinelPrev; // Embedded sentinel.prev; take its ADDRESS to get End.
    OrbitTrackLink* begin;
    // Read the logical orbit-node count, excluding the embedded sentinel.
    unsigned int count() const {
        return size;
    }
};

// Borrow the smaller count for the orbit/gravity common-prefix calculation.
static inline const unsigned int& min801134F0(const unsigned int& selfCount, const unsigned int& otherCount) {
    return otherCount < selfCount ? otherCount : selfCount;
}

struct OrbitTrackTempBase {
    volatile unsigned int size;
    OrbitTrackLink* volatile tail;
    OrbitTrackLink* volatile head;
    // Free the staged orbit suffix if it has not been spliced into self.
    ~OrbitTrackTempBase() {
        OrbitTrackLink* node;
        OrbitTrackLink* e;
        e = (OrbitTrackLink*)&tail;
        node = head;
        if (node != e) {
            OrbitTrackLink* last = e->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            while (node != e) {
                OrbitTrackLink* cur = node;
                node = node->next;
                appFreeUni__FPv(cur);
                --size;
            }
        }
    }
};

struct BidirTag801134F0 {
    volatile char unused;
    char padding[3];
};

// Move an orbit iterator by signed distance within its valid circular list.
static inline void advance801134F0(OrbitTrackLink*& it, int n, const BidirTag801134F0&) {
    if (n >= 0) {
        while (n > 0) {
            it = it->next;
            --n;
        }
    } else {
        while (n < 0) {
            it = it->prev;
            ++n;
        }
    }
}

// Copy two vectors and four trailing words, retaining the orbit node links.
static inline void assign801134F0(OrbitTrackNode* dst, const OrbitTrackNode* src) {
    unsigned char* to;
    const unsigned char* from;
    to = dst->value;
    from = src->value;
    *(WVector*)to = *(WVector*)from;
    *(WVector*)(to + 0xc) = *(WVector*)(from + 0xc);
    *(unsigned int*)(to + 0x18) = *(const unsigned int*)(from + 0x18);
    *(float*)(to + 0x1c) = *(const float*)(from + 0x1c);
    *(unsigned int*)(to + 0x20) = *(const unsigned int*)(from + 0x20);
    *(float*)(to + 0x24) = *(const float*)(from + 0x24);
}

struct Range801134F0 {
    OrbitTrackLink* volatile first;
    OrbitTrackLink* volatile last;
};

struct OrbitNodeGuard {
    OrbitTrackLink* volatile end;
    OrbitTrackNode* node;
    // Own one orbit allocation until its inline payload is constructed.
    OrbitNodeGuard(OrbitTrackLink* e, OrbitTrackNode* n) : end(e), node(n) {}
    // Release an orbit allocation still held by this construction guard.
    ~OrbitNodeGuard() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// Initialize the 0x28-byte inline orbit payload under its original unwind scope.
static inline void construct801134F0(OrbitTrackNode* n, const OrbitTrackNode* source) {
    unsigned char* to = n->value;
    const unsigned char* from = source->value;
    if (to) {
        unsigned int a = *(const unsigned int*)from;
        unsigned int b = *(const unsigned int*)(from + 4);
        try {
            *(unsigned int*)to = a;
            *(unsigned int*)(to + 4) = b;
            *(unsigned int*)(to + 8) = *(const unsigned int*)(from + 8);
            struct Words3 {
                unsigned int v[3];
            };
            *(Words3*)(to + 0xc) = *(const Words3*)(from + 0xc);
            *(unsigned int*)(to + 0x18) = *(const unsigned int*)(from + 0x18);
            *(float*)(to + 0x1c) = *(const float*)(from + 0x1c);
            *(unsigned int*)(to + 0x20) = *(const unsigned int*)(from + 0x20);
            *(float*)(to + 0x24) = *(const float*)(from + 0x24);
        } catch (...) {
            throw;
        }
    }
}

struct OrbitTrackTempList : OrbitTrackTempBase {
    // Link both temporary orbit ends to its embedded empty sentinel.
    void initialize(OrbitTrackLink* endp) {
        head = endp;
        size = 0;
        tail = endp;
    }
    // Copy the missing orbit suffix under temporary node ownership.
    OrbitTrackTempList(OrbitTrackLink* first, OrbitTrackLink* last, OrbitTrackLink* volatile& cur,
                       OrbitTrackLink* volatile& lastCopy, OrbitTrackNode* volatile& allocation,
                       OrbitTrackNode* volatile& constructed) {
        BidirTag801134F0 innerTag, outerTag;
        OrbitTrackLink* endp;
        endp = (OrbitTrackLink*)&tail;
        initialize(endp);
        OrbitTrackTempBase* target = this;
        outerTag.unused = 0;
        lastCopy = last;
        cur = first;
        if (first != last) {
            while (cur != lastCopy) {
                {
                    OrbitTrackNode* source;
                    OrbitTrackLink* insertion;
                    source = (OrbitTrackNode*)cur;
                    innerTag.unused = 0;
                    insertion = (OrbitTrackLink*)&target->tail;
                    OrbitTrackNode* n = (OrbitTrackNode*)appMallocDummy__FUl(0x30);
                    allocation = n;
                    constructed = n;
                    OrbitNodeGuard guard(insertion, n);
                    construct801134F0(n, source);
                    OrbitTrackNode* owned = guard.node;
                    guard.node = 0;
                    n = owned;
                    insertion->prev->next = n;
                    n->prev = insertion->prev;
                    insertion->prev = n;
                    n->next = insertion;
                    ++target->size;
                }
                cur = cur->next;
            }
        }
    }
};

// Locate the source orbit node at the end of the reusable common prefix.
static inline OrbitTrackLink* select801134F0(const OrbitTrackList* other, const volatile unsigned int& minimum) {
    BidirTag801134F0 tagB;
    BidirTag801134F0 tagA;
    unsigned int otherSize = ((const volatile OrbitTrackList*)other)->size;
    unsigned int m = minimum;
    OrbitTrackLink* start;
    if (m <= otherSize / 2) {
        tagA.unused = 0;
        start = other->begin;
        advance801134F0(start, (int)m, tagA);
    } else {
        tagB.unused = 0;
        start = (OrbitTrackLink*)&other->sentinelPrev;
        advance801134F0(start, (int)(m - otherSize), tagB);
    }
    return start;
}

// Copy an orbit-layout track, staging a growing tail or freeing a shrinking tail.
OrbitTrackList* FxSprayEmitterOrbitTrackList_Assign(OrbitTrackList* self, const OrbitTrackList* other) {
    if (self != other) {
        // Phase 1: locate source[m] and stage iterator aliases for either tail path.
        OrbitTrackLink* volatile savedStart;
        OrbitTrackLink* volatile sourceLast;
        unsigned int otherCount;
        OrbitTrackLink* volatile tailLast;
        OrbitTrackLink* volatile sourceFirst;
        OrbitTrackNode* volatile constructed;
        OrbitTrackLink* volatile tailFirst;
        OrbitTrackLink* volatile lastCopy;
        OrbitTrackNode* volatile allocation;
        OrbitTrackLink* volatile shrinkFirst;
        OrbitTrackLink* volatile cur;
        OrbitTrackLink* volatile shrinkStop;
        OrbitTrackLink* volatile growStop;
        OrbitTrackLink* volatile erase;
        otherCount = other->count();

        unsigned int selfCount;
        selfCount = self->count();
        OrbitTrackLink* volatile growFirst;
        unsigned int savedSelfCount = selfCount;
        const volatile unsigned int& minimum = min801134F0(savedSelfCount, otherCount);
        OrbitTrackLink* start = select801134F0(other, minimum);
        tailFirst = start;
        OrbitTrackLink* sourceStart = other->begin;
        OrbitTrackLink* end = (OrbitTrackLink*)&other->sentinelPrev;
        sourceLast = end;
        tailLast = end;
        savedStart = start;
        sourceFirst = sourceStart;

        // Growth: construct a detached tail before overwriting the reused prefix.
        if (start != end) {
            const unsigned char* from;
            OrbitTrackNode* dst;
            unsigned char* to;
            OrbitTrackLink* firstValue;
            OrbitTrackLink* stopValue;
            OrbitTrackTempList temp(start, end, cur, lastCopy, allocation, constructed);
            OrbitTrackLink* last;
            OrbitTrackLink* first;
            stopValue = tailFirst;
            OrbitTrackNode* s;
            firstValue = sourceFirst;
            OrbitTrackLink* volatile& stopCopy = growStop;
            stopCopy = stopValue;
            dst = (OrbitTrackNode*)self->begin;
            OrbitTrackLink* volatile& firstCopy = growFirst;
            firstCopy = firstValue;
            while (stopCopy != firstCopy) {
                s = (OrbitTrackNode*)firstCopy;
                to = dst->value;
                from = s->value;
                *(WVector*)to = *(WVector*)from;
                *(WVector*)(to + 0xc) = *(WVector*)(from + 0xc);
                *(unsigned int*)(to + 0x18) = *(const unsigned int*)(from + 0x18);
                *(float*)(to + 0x1c) = *(const float*)(from + 0x1c);
                *(unsigned int*)(to + 0x20) = *(const unsigned int*)(from + 0x20);
                *(float*)(to + 0x24) = *(const float*)(from + 0x24);
                dst = (OrbitTrackNode*)dst->next;
                firstCopy = firstCopy->next;
            }
            // Transfer every staged node into self; leave the temporary empty.
            if (temp.size) {
                OrbitTrackLink* at = dst;
                first = temp.head;
                last = temp.tail;
                temp.head = (OrbitTrackLink*)&temp.tail;
                temp.tail = (OrbitTrackLink*)&temp.tail;
                at->prev->next = first;
                first->prev = at->prev;
                at->prev = last;
                last->next = at;
                self->size += temp.size;
                temp.size = 0;
            }
        } else {
            // Equal/shrinking: source[m] is End; copy the entire source then erase
            // self's remaining tail. No allocation occurs on this path.
            OrbitTrackLink* dst;
            OrbitTrackLink* volatile& stopCopy = shrinkStop;

            OrbitTrackNode* s;
            stopCopy = start;
            dst = self->begin;
            OrbitTrackLink* volatile& firstCopy = shrinkFirst;
            firstCopy = sourceStart;
            while (firstCopy != stopCopy) {
                s = (OrbitTrackNode*)firstCopy;
                assign801134F0((OrbitTrackNode*)dst, s);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            // Prefix now matches the source; detach/free the remaining old suffix.
            OrbitTrackLink* e = (OrbitTrackLink*)&self->sentinelPrev;
            erase = dst;
            if (dst != e) {
                OrbitTrackLink* last = e->prev;
                dst->prev->next = last->next;
                last->next->prev = dst->prev;
                OrbitTrackLink* cur;
                while ((cur = erase) != e) {
                    erase = cur->next;
                    appFreeUni__FPv(cur);
                    self->size--;
                }
            }
        }
    }
    return self;
}

struct GravityPointTrackNode {
    GravityPointTrackNode* prev;
    GravityPointTrackNode* next;
    unsigned int a, b, c;
    float x, y, z;
};

struct GravityPointTrackTempBase {
    volatile unsigned int size;
    GravityPointTrackNode* volatile tail;
    GravityPointTrackNode* volatile head;
    // Free the staged gravity suffix if it remains owned on scope exit.
    ~GravityPointTrackTempBase() {
        GravityPointTrackNode* node;
        GravityPointTrackNode* e;
        e = (GravityPointTrackNode*)&tail;
        node = head;
        if (node != e) {
            GravityPointTrackNode* last = e->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            while (node != e) {
                GravityPointTrackNode* cur = node;
                node = node->next;
                appFreeUni__FPv(cur);
                --size;
            }
        }
    }
};

struct Range80114108 {
    GravityPointTrackNode* volatile first;
    GravityPointTrackNode* volatile last;
};

struct GravityPointNodeGuard {
    GravityPointTrackNode* volatile end;
    GravityPointTrackNode* node;
    // Retain one gravity allocation until its payload has been copied.
    GravityPointNodeGuard(GravityPointTrackNode* e, GravityPointTrackNode* n) : end(e), node(n) {}
    // Free a gravity node that was not transferred to its temporary list.
    ~GravityPointNodeGuard() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// Initialize the six-word gravity payload without assigning node links.
static inline void construct80114108(GravityPointTrackNode* n, const GravityPointTrackNode* source) {
    unsigned int* to = &n->a;
    const unsigned int* from = &source->a;
    if (to) {
        unsigned int a = from[0];
        unsigned int b = from[1];
        try {
            to[0] = a;
            to[1] = b;
            to[2] = from[2];
            *(float*)(to + 3) = *(const float*)(from + 3);
            *(float*)(to + 4) = *(const float*)(from + 4);
            *(float*)(to + 5) = *(const float*)(from + 5);
        } catch (...) {
            throw;
        }
    }
}

struct Tag80114108 {
    volatile char value;
    char padding[3];
};

struct GravityPointTrackTempList : GravityPointTrackTempBase {
    // Initialize the empty gravity suffix sentinel and zero count.
    void initialize(GravityPointTrackNode* endp) {
        size = 0;
        head = endp;
        tail = endp;
    }
    // Copy a missing gravity suffix into an independently owned circular list.
    GravityPointTrackTempList(GravityPointTrackNode* first, GravityPointTrackNode* last,
                              GravityPointTrackNode* volatile& cur, GravityPointTrackNode* volatile& lastCopy,
                              GravityPointTrackNode* volatile& allocation,
                              GravityPointTrackNode* volatile& constructed) {
        Tag80114108 innerTag;
        Tag80114108 outerTag;
        initialize((GravityPointTrackNode*)&tail);
        GravityPointTrackTempBase* target = this;
        outerTag.value = 0;
        lastCopy = last;
        cur = first;
        if (first != last) {
            while (cur != lastCopy) {
                {
                    GravityPointTrackNode* insertion;
                    GravityPointTrackNode* source;
                    source = cur;
                    innerTag.value = 0;
                    insertion = (GravityPointTrackNode*)&target->tail;
                    GravityPointTrackNode* n = (GravityPointTrackNode*)appMallocDummy__FUl(0x20);
                    allocation = n;
                    constructed = n;
                    GravityPointNodeGuard guard(insertion, n);
                    construct80114108(n, source);
                    GravityPointTrackNode* owned = guard.node;
                    guard.node = 0;
                    n = owned;
                    insertion->prev->next = n;
                    n->prev = insertion->prev;
                    insertion->prev = n;
                    n->next = insertion;
                    ++target->size;
                }
                cur = cur->next;
            }
        }
    }
};

// Move a gravity iterator by signed distance within its list.
static inline void advance80114108(GravityPointTrackNode*& it, int n) {
    if (n >= 0) {
        while (n > 0) {
            it = it->next;
            --n;
        }
    } else {
        while (n < 0) {
            it = it->prev;
            ++n;
        }
    }
}

// Assign the gravity payload (vector plus three floats), retaining node links.
static inline void assign80114108(GravityPointTrackNode* dst, const GravityPointTrackNode* src) {
    unsigned int* to = &dst->a;
    const unsigned int* from = &src->a;
    *(WVector*)to = *(WVector*)from;
    *(float*)(to + 3) = *(const float*)(from + 3);
    *(float*)(to + 4) = *(const float*)(from + 4);
    *(float*)(to + 5) = *(const float*)(from + 5);
}

// Copy a reused gravity payload on the tail-growth path.
static inline void assignGrow80114108(const GravityPointTrackNode* src, GravityPointTrackNode* dst) {
    const unsigned int* from = &src->a;
    unsigned int* to = &dst->a;
    *(WVector*)to = *(WVector*)from;
    *(float*)(to + 3) = *(const float*)(from + 3);
    *(float*)(to + 4) = *(const float*)(from + 4);
    *(float*)(to + 5) = *(const float*)(from + 5);
}

// Locate the gravity source's common-prefix boundary from its nearer end.
static inline GravityPointTrackNode* select80114108(const GravityPointTrackList* other,
                                                    const volatile unsigned int& minimum) {
    Tag80114108 tagB;
    Tag80114108 tagA;
    unsigned int otherSize = ((const volatile GravityPointTrackList*)other)->size;
    unsigned int m = minimum;
    GravityPointTrackNode* start;
    if (m <= otherSize / 2) {
        tagA.value = 0;
        start = (GravityPointTrackNode*)other->begin;
        advance80114108(start, (int)m);
    } else {
        tagB.value = 0;
        start = (GravityPointTrackNode*)&other->sentinelPrev;
        advance80114108(start, (int)(m - otherSize));
    }
    return start;
}

// Copy a gravity track, reusing its common prefix and adjusting its owning tail.
GravityPointTrackList* FxSprayGravityPointTrackList_Assign(GravityPointTrackList* self,
                                                           const GravityPointTrackList* other) {
    if (self != other) {
        GravityPointTrackNode* volatile savedStart;
        unsigned int selfCount;
        unsigned int otherCount;
        GravityPointTrackNode* volatile sourceLast;
        GravityPointTrackNode* volatile sourceFirst;
        GravityPointTrackNode* volatile tailLast;
        GravityPointTrackNode* volatile tailFirst;
        GravityPointTrackNode* volatile constructed;
        GravityPointTrackNode* volatile allocation;
        GravityPointTrackNode* volatile lastCopy;
        GravityPointTrackNode* volatile cur;
        GravityPointTrackNode* volatile shrinkStop;
        GravityPointTrackNode* volatile shrinkFirst;
        GravityPointTrackNode* volatile erase;
        GravityPointTrackNode* volatile growStop;
        GravityPointTrackNode* volatile growFirst;
        otherCount = other->count();
        selfCount = self->count();
        const volatile unsigned int& minimum = min801134F0(selfCount, otherCount);
        GravityPointTrackNode* start = select80114108(other, minimum);
        GravityPointTrackNode* end = (GravityPointTrackNode*)&other->sentinelPrev;
        GravityPointTrackNode* sourceStart = (GravityPointTrackNode*)other->begin;
        tailFirst = start;
        tailLast = end;
        sourceLast = end;
        savedStart = start;
        sourceFirst = sourceStart;
        // Growth: build a temporary tail, assign existing values, then splice.
        if (start != end) {
            GravityPointTrackTempList temp(start, end, cur, lastCopy, allocation, constructed);
            GravityPointTrackNode* firstValue;
            GravityPointTrackNode* stopValue;
            stopValue = tailFirst;
            firstValue = sourceFirst;
            GravityPointTrackNode* volatile& stopCopy = growStop;
            stopCopy = stopValue;
            GravityPointTrackNode* dst = (GravityPointTrackNode*)self->begin;
            GravityPointTrackNode* volatile& firstCopy = growFirst;
            firstCopy = firstValue;
            while (firstCopy != stopCopy) {
                assignGrow80114108(firstCopy, dst);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            // Empty the temporary sentinel as ownership transfers to self.
            if (temp.size != 0) {
                GravityPointTrackNode* last;
                GravityPointTrackNode* first;
                first = temp.head;
                last = temp.tail;
                temp.head = (GravityPointTrackNode*)&temp.tail;
                temp.tail = (GravityPointTrackNode*)&temp.tail;
                dst->prev->next = first;
                first->prev = dst->prev;
                dst->prev = last;
                last->next = dst;
                self->size += temp.size;
                temp.size = 0;
            }
        } else {
            GravityPointTrackNode* volatile& stopCopy = shrinkStop;
            stopCopy = start;
            GravityPointTrackNode* dst = (GravityPointTrackNode*)self->begin;
            GravityPointTrackNode* volatile& firstCopy = shrinkFirst;
            firstCopy = sourceStart;
            while (firstCopy != stopCopy) {
                assign80114108(dst, firstCopy);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            // Shrink only after the reusable prefix contains all source values.
            GravityPointTrackNode* e = (GravityPointTrackNode*)&self->sentinelPrev;
            erase = dst;
            if (dst != e) {
                GravityPointTrackNode* last = e->prev;
                dst->prev->next = last->next;
                last->next->prev = dst->prev;
                GravityPointTrackNode* cur;
                while ((cur = erase) != e) {
                    erase = cur->next;
                    appFreeUni__FPv(cur);
                    --self->size;
                }
            }
        }
    }
    return self;
}

} // extern "C"
