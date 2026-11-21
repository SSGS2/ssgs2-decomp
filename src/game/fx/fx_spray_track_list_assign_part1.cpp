#include "game/wvector.h"
// Translation unit 0x801134F0-0x801146EC (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Three owning circular-list specializations, with a 12-byte header:
// +0 count, +4 sentinel.prev (tail), +8 sentinel.next (first).
// End is the embedded pair at header+4; an empty list links both words to End.
// Nodes start with prev/next at +0/+4 and inline payload at +8. Spawn and
// gravity nodes are 0x20 bytes; orbit nodes are 0x30. All active Node* views
// store prev first. List801134F0::sentinelPrev is that prev link,
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

struct List80114108 {
    unsigned int size;
    ListNode80114108* sentinelPrev;
    ListNode80114108* begin;
    // Read the logical gravity-node count, excluding the embedded sentinel.
    unsigned int count() const { return size; }
};

void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
void __as__7WVectorFRC7WVector(void*, const void*);
void dtor_801102B4(void*);
void dtor_80110364(void*);
void dtor_80110204(void*);
void dtor_80110150(void*);
void* appMallocDummy__FUl(unsigned int);

struct Elem80113B1C {
    // Payload +0/+4/+8 are integer words, +0xC is float, +0x10 integer,
    // +0x14 float. This assignment unit does not resolve their gameplay roles.
    int a;
    int b;
    int c;
    float d;
    int e;
    float f;
};

struct NodeBase80113B1C {
    NodeBase80113B1C* prev;
    NodeBase80113B1C* next;
};

struct Node80113B1C : NodeBase80113B1C {
    Elem80113B1C v;
};

struct List80113B1C {
    unsigned int size;
    NodeBase80113B1C end;
    // Read the logical spawn-node count, excluding the embedded sentinel.
    unsigned int count() const { return size; }
};

struct BidirTag80113B1C {
    volatile char unused;
    char padding[3];
};

struct TempBase80113B1C {
    volatile unsigned int size;
    struct {
        NodeBase80113B1C* volatile prev;
        NodeBase80113B1C* volatile next;
    } end;
    // Free any staged spawn suffix not transferred to the destination list.
    ~TempBase80113B1C() {
        NodeBase80113B1C* node;
        NodeBase80113B1C* e;
        e = (NodeBase80113B1C*)&end;
        node = end.next;
        if (node != e) {
            NodeBase80113B1C* last = e->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            while (node != e) {
                NodeBase80113B1C* cur = node;
                node = node->next;
                appFreeUni__FPv(cur);
                --size;
            }
        }
    }
};

struct NodeGuard80113B1C {
    NodeBase80113B1C* volatile end;
    Node80113B1C* node;
    // Own a new spawn allocation until payload construction succeeds.
    NodeGuard80113B1C(NodeBase80113B1C* e, Node80113B1C* n) : end(e), node(n) {}
    // Free the node if construction failed before ownership transfer.
    ~NodeGuard80113B1C() {
        if (node) appFreeUni__FPv(node);
    }
};

// Advance by signed n; caller guarantees the destination remains in the valid range.
static inline void advance80113B1C(NodeBase80113B1C*& it, int n, const BidirTag80113B1C&) {
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
    NodeBase80113B1C* volatile first;
    NodeBase80113B1C* volatile last;
};

// Borrow the smaller count; preserve the library's reference-return convention.
static inline const unsigned int& min80113B1C(const unsigned int& a, const unsigned int& b) {
    return b < a ? b : a;
}

// Copy the six spawn payload words into a reused destination node.
static inline void copyValue80113B1C(const Node80113B1C* src, Node80113B1C* dst) {
    const Elem80113B1C* from = &src->v;
    Elem80113B1C* to = &dst->v;
    to->a = from->a;
    to->b = from->b;
    to->c = from->c;
    to->d = from->d;
    to->e = from->e;
    to->f = from->f;
}

// Assign a spawn payload while retaining the destination's links.
static inline void assignValue80113B1C(Node80113B1C* dst, const Node80113B1C* src) {
    Elem80113B1C* to = &dst->v;
    const Elem80113B1C* from = &src->v;
    to->a = from->a;
    to->b = from->b;
    to->c = from->c;
    to->d = from->d;
    to->e = from->e;
    to->f = from->f;
}

// Initialize a newly allocated spawn payload with the original unwind boundary.
static inline void constructValue80113B1C(const Node80113B1C* src, Node80113B1C* dst) {
    unsigned char* to = (unsigned char*)&dst->v;
    const unsigned char* from = (const unsigned char*)&src->v;
    if (to) {
        unsigned int a = *(const unsigned int*)(from + 0x0);
        unsigned int b = *(const unsigned int*)(from + 0x4);
        try {
        *(unsigned int*)(to + 0x0) = a;
        *(unsigned int*)(to + 0x4) = b;
        struct Words4 { unsigned int v[4]; };
        *(Words4*)(to + 8) = *(const Words4*)(from + 8);
        } catch (...) { throw; }
    }
}

struct TempList80113B1C : TempBase80113B1C {
    // Initialize the temporary's empty embedded sentinel and logical count.
    void initialize(unsigned int count, NodeBase80113B1C* endp) {
        end.next = endp;
        size = count;
        end.prev = endp;
    }
    // Copy [first,last) into a separately owned suffix, updating unwind cursors.
    TempList80113B1C(NodeBase80113B1C* first, NodeBase80113B1C* last,
        NodeBase80113B1C* volatile& cur, NodeBase80113B1C* volatile& lastCopy,
        Node80113B1C* volatile& allocation, Node80113B1C* volatile& constructed) {
        struct Tag { volatile char value; char padding[3]; } innerTag, tag;
        NodeBase80113B1C* endp;
        unsigned int count;
        endp = (NodeBase80113B1C*)&end;
        count = 0;
        initialize(count, endp);
        TempBase80113B1C* target = this;
        tag.value = 0;
        lastCopy = last;
        cur = first;
        if (first != last) {
            while (cur != lastCopy) {
                {
                    NodeBase80113B1C* insertion;
                    Node80113B1C* source;
                    source = (Node80113B1C*)cur;
                    innerTag.value = 0;
                    insertion = (NodeBase80113B1C*)&target->end;
                    Node80113B1C* n = (Node80113B1C*)appMallocDummy__FUl(0x20);
                    allocation = n;
                    constructed = n;
                    NodeGuard80113B1C guard(insertion, n);
                    constructValue80113B1C(source, n);
                    Node80113B1C* owned = guard.node;
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
static inline NodeBase80113B1C* select80113B1C(const List80113B1C* other, const volatile unsigned int& minimum) {
    BidirTag80113B1C tagB;
    BidirTag80113B1C tagA;
    unsigned int otherSize = ((const volatile List80113B1C*)other)->size;
    unsigned int m = minimum;
    NodeBase80113B1C* start;
    if (m <= otherSize / 2) {
        tagA.unused = 0;
        start = other->end.next;
        advance80113B1C(start, (int)m, tagA);
    } else {
        tagB.unused = 0;
        start = (NodeBase80113B1C*)&other->end;
        advance80113B1C(start, (int)(m - otherSize), tagB);
    }
    return start;
}

// Copy the spawn track, reusing nodes and growing or shrinking only its tail.
List80113B1C* FxSpraySpawnTrackList_Assign(List80113B1C* self, const List80113B1C* other) {
    if (self != other) {
        NodeBase80113B1C* volatile savedStart;
        unsigned int selfCount;
        unsigned int otherCount;
        NodeBase80113B1C* volatile sourceLast;
        NodeBase80113B1C* volatile sourceFirst;
        NodeBase80113B1C* volatile tailLast;
        NodeBase80113B1C* volatile tailFirst;
        Node80113B1C* volatile constructed;
        Node80113B1C* volatile allocation;
        NodeBase80113B1C* volatile lastCopy;
        NodeBase80113B1C* volatile cur;
        NodeBase80113B1C* volatile shrinkStop;
        NodeBase80113B1C* volatile shrinkFirst;
        NodeBase80113B1C* volatile erase;
        NodeBase80113B1C* volatile growStop;
        NodeBase80113B1C* volatile growFirst;
        otherCount = other->count();
        selfCount = self->count();
        const volatile unsigned int& minimum = min80113B1C(selfCount, otherCount);
        NodeBase80113B1C* start = select80113B1C(other, minimum);

        NodeBase80113B1C* end = (NodeBase80113B1C*)&other->end;
        NodeBase80113B1C* sourceStart = other->end.next;
        tailFirst = start;
        tailLast = end;
        sourceLast = end;
        savedStart = start;
        sourceFirst = sourceStart;

        // Growth: stage source[minimum..end) under a temporary owning sentinel.
        if (start != end) {
            TempList80113B1C temp(start, end, cur, lastCopy, allocation, constructed);
            NodeBase80113B1C* firstValue;
            NodeBase80113B1C* stopValue;
            NodeBase80113B1C* lastT;
            NodeBase80113B1C* firstT;
            Node80113B1C* s;
            NodeBase80113B1C* dst;
            stopValue = tailFirst;
            firstValue = sourceFirst;
            NodeBase80113B1C* volatile& stopCopy = growStop;
            stopCopy = stopValue;
            dst = self->end.next;
            NodeBase80113B1C* volatile& firstCopy = growFirst;
            firstCopy = firstValue;
            // Tail allocation succeeded: overwrite the reused common prefix.
            while (firstCopy != stopCopy) {
                s = (Node80113B1C*)firstCopy;
                copyValue80113B1C(s, (Node80113B1C*)dst);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            // Splice staged nodes before destination End; temp then owns none.
            if (temp.size != 0) {
                NodeBase80113B1C* endp = (NodeBase80113B1C*)&temp.end;
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
            Node80113B1C* s;
            NodeBase80113B1C* dst;
            NodeBase80113B1C* volatile& stopCopy = shrinkStop;
            stopCopy = start;
            dst = self->end.next;
            NodeBase80113B1C* volatile& firstCopy = shrinkFirst;
            firstCopy = sourceStart;
            while (firstCopy != stopCopy) {
                s = (Node80113B1C*)firstCopy;
                assignValue80113B1C((Node80113B1C*)dst, s);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            NodeBase80113B1C* e = (NodeBase80113B1C*)&self->end;
            erase = dst;
            if (dst != e) {
                NodeBase80113B1C* lastNode = e->prev;
                dst->prev->next = lastNode->next;
                lastNode->next->prev = dst->prev;
                NodeBase80113B1C* cur;
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

struct Node801134F0 {
    Node801134F0* prev;
    Node801134F0* next;
};

struct Value801134F0 : Node801134F0 {
    // +0x00/+0x0C: two WVector-compatible triples; +0x18..+0x27: four
    // copied words: uint at +18, float at +1C, uint at +20, float at +24.
    // Gameplay meanings and coordinate spaces are unresolved; copying is bitwise.
    // Offsets are relative to value, not the node's link prefix.
    unsigned char value[0x28];
};

struct List801134F0 {
    unsigned int size;
    Node801134F0* sentinelPrev; // Embedded sentinel.prev; take its ADDRESS to get End.
    Node801134F0* begin;
    // Read the logical orbit-node count, excluding the embedded sentinel.
    unsigned int count() const { return size; }
};

// Borrow the smaller count for the orbit/gravity common-prefix calculation.
static inline const unsigned int& min801134F0(const unsigned int& a, const unsigned int& b) {
    return b < a ? b : a;
}

struct TempBase801134F0 {
    volatile unsigned int size;
    Node801134F0* volatile tail;
    Node801134F0* volatile head;
    // Free the staged orbit suffix if it has not been spliced into self.
    ~TempBase801134F0() {
        Node801134F0* node;
        Node801134F0* e;
        e = (Node801134F0*)&tail;
        node = head;
        if (node != e) {
            Node801134F0* last = e->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            while (node != e) {
                Node801134F0* cur = node;
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
static inline void advance801134F0(Node801134F0*& it, int n, const BidirTag801134F0&) {
    if (n >= 0) {
        while (n > 0) { it = it->next; --n; }
    } else {
        while (n < 0) { it = it->prev; ++n; }
    }
}

// Copy two vectors and four trailing words, retaining the orbit node links.
static inline void assign801134F0(Value801134F0* dst, const Value801134F0* src) {
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
    Node801134F0* volatile first;
    Node801134F0* volatile last;
};

struct NodeGuard801134F0 {
    Node801134F0* volatile end;
    Value801134F0* node;
    // Own one orbit allocation until its inline payload is constructed.
    NodeGuard801134F0(Node801134F0* e, Value801134F0* n) : end(e), node(n) {}
    // Release an orbit allocation still held by this construction guard.
    ~NodeGuard801134F0() {
        if (node) appFreeUni__FPv(node);
    }
};

// Initialize the 0x28-byte inline orbit payload under its original unwind scope.
static inline void construct801134F0(Value801134F0* n, const Value801134F0* source) {
    unsigned char* to = n->value;
    const unsigned char* from = source->value;
    if (to) {
        unsigned int a = *(const unsigned int*)from;
        unsigned int b = *(const unsigned int*)(from + 4);
        try {
            *(unsigned int*)to = a;
            *(unsigned int*)(to + 4) = b;
            *(unsigned int*)(to + 8) = *(const unsigned int*)(from + 8);
            struct Words3 { unsigned int v[3]; };
            *(Words3*)(to + 0xc) = *(const Words3*)(from + 0xc);
            *(unsigned int*)(to + 0x18) = *(const unsigned int*)(from + 0x18);
            *(float*)(to + 0x1c) = *(const float*)(from + 0x1c);
            *(unsigned int*)(to + 0x20) = *(const unsigned int*)(from + 0x20);
            *(float*)(to + 0x24) = *(const float*)(from + 0x24);
        } catch (...) { throw; }
    }
}

struct TempList801134F0 : TempBase801134F0 {
    // Link both temporary orbit ends to its embedded empty sentinel.
    void initialize(Node801134F0* endp) {
        head = endp;
        size = 0;
        tail = endp;
    }
    // Copy the missing orbit suffix under temporary node ownership.
    TempList801134F0(Node801134F0* first, Node801134F0* last,
        Node801134F0* volatile& cur, Node801134F0* volatile& lastCopy,
        Value801134F0* volatile& allocation, Value801134F0* volatile& constructed) {
        BidirTag801134F0 innerTag, outerTag;
        Node801134F0* endp;
        endp = (Node801134F0*)&tail;
        initialize(endp);
        TempBase801134F0* target = this;
        outerTag.unused = 0;
        lastCopy = last;
        cur = first;
        if (first != last) {
            while (cur != lastCopy) {
                {
                    Value801134F0* source;
                    Node801134F0* insertion;
                    source = (Value801134F0*)cur;
                    innerTag.unused = 0;
                    insertion = (Node801134F0*)&target->tail;
                    Value801134F0* n = (Value801134F0*)appMallocDummy__FUl(0x30);
                    allocation = n;
                    constructed = n;
                    NodeGuard801134F0 guard(insertion, n);
                    construct801134F0(n, source);
                    Value801134F0* owned = guard.node;
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
static inline Node801134F0* select801134F0(const List801134F0* other, const volatile unsigned int& minimum) {
    BidirTag801134F0 tagB;
    BidirTag801134F0 tagA;
    unsigned int otherSize = ((const volatile List801134F0*)other)->size;
    unsigned int m = minimum;
    Node801134F0* start;
    if (m <= otherSize / 2) {
        tagA.unused = 0;
        start = other->begin;
        advance801134F0(start, (int)m, tagA);
    } else {
        tagB.unused = 0;
        start = (Node801134F0*)&other->sentinelPrev;
        advance801134F0(start, (int)(m - otherSize), tagB);
    }
    return start;
}

// Copy an orbit-layout track, staging a growing tail or freeing a shrinking tail.
List801134F0* FxSprayEmitterOrbitTrackList_Assign(List801134F0* self, const List801134F0* other) {
    if (self != other) {
        // Phase 1: locate source[m] and stage iterator aliases for either tail path.
        Node801134F0* volatile savedStart;
        Node801134F0* volatile sourceLast;
        unsigned int otherCount;
        Node801134F0* volatile tailLast;
        Node801134F0* volatile sourceFirst;
        Value801134F0* volatile constructed;
        Node801134F0* volatile tailFirst;
        Node801134F0* volatile lastCopy;
        Value801134F0* volatile allocation;
        Node801134F0* volatile shrinkFirst;
        Node801134F0* volatile cur;
        Node801134F0* volatile shrinkStop;
        Node801134F0* volatile growStop;
        Node801134F0* volatile erase;
        otherCount = other->count();

        unsigned int selfCount;
        selfCount = self->count();
        Node801134F0* volatile growFirst;
        unsigned int savedSelfCount = selfCount;
        const volatile unsigned int& minimum = min801134F0(savedSelfCount, otherCount);
        Node801134F0* start = select801134F0(other, minimum);
        tailFirst = start;
        Node801134F0* sourceStart = other->begin;
        Node801134F0* end = (Node801134F0*)&other->sentinelPrev;
        sourceLast = end;
        tailLast = end;
        savedStart = start;
        sourceFirst = sourceStart;

        // Growth: construct a detached tail before overwriting the reused prefix.
        if (start != end) {
            const unsigned char* from;
            Value801134F0* dst;
            unsigned char* to;
            Node801134F0* firstValue;
            Node801134F0* stopValue;
            TempList801134F0 temp(start, end, cur, lastCopy, allocation, constructed);
            Node801134F0* last;
            Node801134F0* first;
            stopValue = tailFirst;
            Value801134F0* s;
            firstValue = sourceFirst;
            Node801134F0* volatile& stopCopy = growStop;
            stopCopy = stopValue;
            dst = (Value801134F0*)self->begin;
            Node801134F0* volatile& firstCopy = growFirst;
            firstCopy = firstValue;
            while (stopCopy != firstCopy) {
                s = (Value801134F0*)firstCopy;
                to = dst->value;
                from = s->value;
                *(WVector*)to = *(WVector*)from;
                *(WVector*)(to + 0xc) = *(WVector*)(from + 0xc);
                *(unsigned int*)(to + 0x18) = *(const unsigned int*)(from + 0x18);
                *(float*)(to + 0x1c) = *(const float*)(from + 0x1c);
                *(unsigned int*)(to + 0x20) = *(const unsigned int*)(from + 0x20);
                *(float*)(to + 0x24) = *(const float*)(from + 0x24);
                dst = (Value801134F0*)dst->next;
                firstCopy = firstCopy->next;
            }
            // Transfer every staged node into self; leave the temporary empty.
            if (temp.size) {
                Node801134F0* at = dst;
                first = temp.head;
                last = temp.tail;
                temp.head = (Node801134F0*)&temp.tail;
                temp.tail = (Node801134F0*)&temp.tail;
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
            Node801134F0* dst;
            Node801134F0* volatile& stopCopy = shrinkStop;

            Value801134F0* s;
            stopCopy = start;
            dst = self->begin;
            Node801134F0* volatile& firstCopy = shrinkFirst;
            firstCopy = sourceStart;
            while (firstCopy != stopCopy) {
                s = (Value801134F0*)firstCopy;
                assign801134F0((Value801134F0*)dst, s);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            // Prefix now matches the source; detach/free the remaining old suffix.
            Node801134F0* e = (Node801134F0*)&self->sentinelPrev;
            erase = dst;
            if (dst != e) {
                Node801134F0* last = e->prev;
                dst->prev->next = last->next;
                last->next->prev = dst->prev;
                Node801134F0* cur;
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

struct Node80114108 {
    Node80114108* prev;
    Node80114108* next;
    unsigned int a, b, c;
    float x, y, z;
};

struct TempBase80114108 {
    volatile unsigned int size;
    Node80114108* volatile tail;
    Node80114108* volatile head;
    // Free the staged gravity suffix if it remains owned on scope exit.
    ~TempBase80114108() {
        Node80114108* node;
        Node80114108* e;
        e = (Node80114108*)&tail;
        node = head;
        if (node != e) {
            Node80114108* last = e->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            while (node != e) {
                Node80114108* cur = node;
                node = node->next;
                appFreeUni__FPv(cur);
                --size;
            }
        }
    }
};

struct Range80114108 {
    Node80114108* volatile first;
    Node80114108* volatile last;
};

struct NodeGuard80114108 {
    Node80114108* volatile end;
    Node80114108* node;
    // Retain one gravity allocation until its payload has been copied.
    NodeGuard80114108(Node80114108* e, Node80114108* n) : end(e), node(n) {}
    // Free a gravity node that was not transferred to its temporary list.
    ~NodeGuard80114108() { if (node) appFreeUni__FPv(node); }
};

// Initialize the six-word gravity payload without assigning node links.
static inline void construct80114108(Node80114108* n, const Node80114108* source) {
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
        } catch (...) { throw; }
    }
}

struct Tag80114108 {
    volatile char value;
    char padding[3];
};

struct TempList80114108 : TempBase80114108 {
    // Initialize the empty gravity suffix sentinel and zero count.
    void initialize(Node80114108* endp) {
        size = 0;
        head = endp;
        tail = endp;
    }
    // Copy a missing gravity suffix into an independently owned circular list.
    TempList80114108(Node80114108* first, Node80114108* last,
        Node80114108* volatile& cur, Node80114108* volatile& lastCopy,
        Node80114108* volatile& allocation, Node80114108* volatile& constructed) {
        Tag80114108 innerTag;
        Tag80114108 outerTag;
        initialize((Node80114108*)&tail);
        TempBase80114108* target = this;
        outerTag.value = 0;
        lastCopy = last;
        cur = first;
        if (first != last) {
            while (cur != lastCopy) {
                {
                    Node80114108* insertion;
                    Node80114108* source;
                    source = cur;
                    innerTag.value = 0;
                    insertion = (Node80114108*)&target->tail;
                    Node80114108* n = (Node80114108*)appMallocDummy__FUl(0x20);
                    allocation = n;
                    constructed = n;
                    NodeGuard80114108 guard(insertion, n);
                    construct80114108(n, source);
                    Node80114108* owned = guard.node;
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
static inline void advance80114108(Node80114108*& it, int n) {
    if (n >= 0) {
        while (n > 0) { it = it->next; --n; }
    } else {
        while (n < 0) { it = it->prev; ++n; }
    }
}

// Assign the gravity payload (vector plus three floats), retaining node links.
static inline void assign80114108(Node80114108* dst, const Node80114108* src) {
    unsigned int* to = &dst->a;
    const unsigned int* from = &src->a;
    *(WVector*)to = *(WVector*)from;
    *(float*)(to + 3) = *(const float*)(from + 3);
    *(float*)(to + 4) = *(const float*)(from + 4);
    *(float*)(to + 5) = *(const float*)(from + 5);
}

// Copy a reused gravity payload on the tail-growth path.
static inline void assignGrow80114108(const Node80114108* src, Node80114108* dst) {
    const unsigned int* from = &src->a;
    unsigned int* to = &dst->a;
    *(WVector*)to = *(WVector*)from;
    *(float*)(to + 3) = *(const float*)(from + 3);
    *(float*)(to + 4) = *(const float*)(from + 4);
    *(float*)(to + 5) = *(const float*)(from + 5);
}

// Locate the gravity source's common-prefix boundary from its nearer end.
static inline Node80114108* select80114108(const List80114108* other, const volatile unsigned int& minimum) {
    Tag80114108 tagB;
    Tag80114108 tagA;
    unsigned int otherSize = ((const volatile List80114108*)other)->size;
    unsigned int m = minimum;
    Node80114108* start;
    if (m <= otherSize / 2) {
        tagA.value = 0;
        start = (Node80114108*)other->begin;
        advance80114108(start, (int)m);
    } else {
        tagB.value = 0;
        start = (Node80114108*)&other->sentinelPrev;
        advance80114108(start, (int)(m - otherSize));
    }
    return start;
}

// Copy a gravity track, reusing its common prefix and adjusting its owning tail.
List80114108* FxSprayGravityPointTrackList_Assign(List80114108* self, const List80114108* other) {
    if (self != other) {
        Node80114108* volatile savedStart;
        unsigned int selfCount;
        unsigned int otherCount;
        Node80114108* volatile sourceLast;
        Node80114108* volatile sourceFirst;
        Node80114108* volatile tailLast;
        Node80114108* volatile tailFirst;
        Node80114108* volatile constructed;
        Node80114108* volatile allocation;
        Node80114108* volatile lastCopy;
        Node80114108* volatile cur;
        Node80114108* volatile shrinkStop;
        Node80114108* volatile shrinkFirst;
        Node80114108* volatile erase;
        Node80114108* volatile growStop;
        Node80114108* volatile growFirst;
        otherCount = other->count();
        selfCount = self->count();
        const volatile unsigned int& minimum = min801134F0(selfCount, otherCount);
        Node80114108* start = select80114108(other, minimum);
        Node80114108* end = (Node80114108*)&other->sentinelPrev;
        Node80114108* sourceStart = (Node80114108*)other->begin;
        tailFirst = start;
        tailLast = end;
        sourceLast = end;
        savedStart = start;
        sourceFirst = sourceStart;
        // Growth: build a temporary tail, assign existing values, then splice.
        if (start != end) {
            TempList80114108 temp(start, end, cur, lastCopy, allocation, constructed);
            Node80114108* firstValue;
            Node80114108* stopValue;
            stopValue = tailFirst;
            firstValue = sourceFirst;
            Node80114108* volatile& stopCopy = growStop;
            stopCopy = stopValue;
            Node80114108* dst = (Node80114108*)self->begin;
            Node80114108* volatile& firstCopy = growFirst;
            firstCopy = firstValue;
            while (firstCopy != stopCopy) {
                assignGrow80114108(firstCopy, dst);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            // Empty the temporary sentinel as ownership transfers to self.
            if (temp.size != 0) {
                Node80114108* last;
                Node80114108* first;
                first = temp.head;
                last = temp.tail;
                temp.head = (Node80114108*)&temp.tail;
                temp.tail = (Node80114108*)&temp.tail;
                dst->prev->next = first;
                first->prev = dst->prev;
                dst->prev = last;
                last->next = dst;
                self->size += temp.size;
                temp.size = 0;
            }
        } else {
            Node80114108* volatile& stopCopy = shrinkStop;
            stopCopy = start;
            Node80114108* dst = (Node80114108*)self->begin;
            Node80114108* volatile& firstCopy = shrinkFirst;
            firstCopy = sourceStart;
            while (firstCopy != stopCopy) {
                assign80114108(dst, firstCopy);
                firstCopy = firstCopy->next;
                dst = dst->next;
            }
            // Shrink only after the reusable prefix contains all source values.
            Node80114108* e = (Node80114108*)&self->sentinelPrev;
            erase = dst;
            if (dst != e) {
                Node80114108* last = e->prev;
                dst->prev->next = last->next;
                last->next->prev = dst->prev;
                Node80114108* cur;
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
