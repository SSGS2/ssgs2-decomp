typedef unsigned long new_size_t;
// Translation unit 0x801146EC-0x80115838 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma push
#pragma exceptions on
#pragma cpp_exceptions on
// Construct into caller-provided storage without allocating.
inline void* operator new(new_size_t, void* p) { return p; }

extern "C" {

void appFreeUni__FPv(void*);
void* appMallocDummy__FUl(unsigned int);
void dtor_80110418(void*);

// Intrusive circular list links occupy +0/+4; payload starts at +8.
// Color/size nodes allocate 24 bytes (16-byte payload); frame nodes allocate
// 28 bytes (20-byte payload). value[16] is ONLY the common prefix view:
// frame word 5 is accessed through node+24, within that larger allocation.
// Never use sizeof(Node801146EC) to allocate/copy a frame node.
struct Node801146EC {
    Node801146EC* prev;
    Node801146EC* next;
    unsigned char value[16];
};

// size counts real nodes. Only sentinel.prev/next are live sentinel fields;
// its value bytes and the trailing end field are not part of a key.
struct List801146EC {
    unsigned int size;
    Node801146EC sentinel;
    Node801146EC* end;
};

struct Temp146EC {
    unsigned int size;
    Node801146EC* prev;
    Node801146EC* next;
    // Create an empty ring whose sentinel consists only of these two link words.
    Temp146EC() : size(0) { prev = next = (Node801146EC*)&prev; }
    // Release owned nodes, never the embedded sentinel.
    ~Temp146EC() {
        Node801146EC* n = next;
        Node801146EC* end = (Node801146EC*)&prev;
        if (n != end) {
            Node801146EC* last = end->prev;
            n->prev->next = last->next;
            last->next->prev = n->prev;
            while (n != end) {
                Node801146EC* old = n;
                n = n->next;
                appFreeUni__FPv(old);
                --size;
            }
        }
    }
};

// Four raw words, copied without conversion. Assignment interprets color keys as
// {float, u32, float, u32}, size keys as {float, float, float, float}.
// Time units, packed-color channel order and the roles of each pair are unresolved here.
struct Payload146EC {
    unsigned int a, b, c, d;
    // Copy all payload bits with the original exception construction order.
    Payload146EC(const Payload146EC& p) {
        unsigned int first = p.a, second = p.b;
        try {
            a = first; b = second;
            unsigned int third = p.c, fourth = p.d;
            c = third; d = fourth;
        } catch (...) { throw; }
    }
};
struct Tag146EC { char value; char padding[3]; };
struct Allocation146EC {
    Node801146EC* p;
    // Own a newly allocated node until it has been constructed and linked.
    Allocation146EC(Node801146EC* n) : p(n) {}
    // Roll back an uncommitted allocation on exceptional exits.
    ~Allocation146EC() { if (p) appFreeUni__FPv(p); }
};
struct TempCopy146EC {
    volatile unsigned int size;
    Node801146EC* volatile prev;
    Node801146EC* volatile next;
    // Initialize an empty temporary list used to stage an assignment's extra nodes.
    TempCopy146EC() {
        Node801146EC* end = (Node801146EC*)&prev;
        prev = next = end;
        size = 0;
    }
    // Free nodes still owned by the staging list; successful splice sets size to zero.
    ~TempCopy146EC() {
        Node801146EC* n = next;
        Node801146EC* end = (Node801146EC*)&prev;
        if (n != end) {
            Node801146EC* last = end->prev;
            n->prev->next = last->next;
            last->next->prev = n->prev;
            while (n != end) {
                Node801146EC* old = n;
                n = n->next;
                appFreeUni__FPv(old);
                --size;
            }
        }
    }
};
struct Copy146EC : TempCopy146EC {
    // Deep-copy the half-open node range [first,last) into this temporary ring.
    Copy146EC(Node801146EC* first, Node801146EC* last) {
        volatile Tag146EC tag;
        tag.value = 0;
        Node801146EC* volatile stop = last;
        Node801146EC* volatile it = first;
        if (first != last) {
        while (it != stop) {
            Node801146EC* end = (Node801146EC*)&prev;
            Node801146EC* source = it;
            volatile Tag146EC tag;
            tag.value = 0;
            {
            Node801146EC* node = (Node801146EC*)appMallocDummy__FUl(24);
            Node801146EC* volatile allocated = node;
            Node801146EC* volatile original = node;
            Node801146EC* volatile position = end;
            Allocation146EC allocation(node);
            Payload146EC* payload = (Payload146EC*)((char*)node + 8);
            new(payload) Payload146EC(*(Payload146EC*)source->value);
            Node801146EC* mem = allocation.p;
            allocation.p = 0;
            end->prev->next = mem;
            mem->prev = end->prev;
            end->prev = mem;
            mem->next = end;
            ++size;
            }
            it = it->next;
        }
        }
    }
};

// Move by a signed number of links; negative distances walk backwards from end().
inline void Advance146EC(Node801146EC*& cursor, int distance) {
    if (distance >= 0) {
        while (distance > 0) { cursor = cursor->next; --distance; }
    } else {
        while (distance < 0) { cursor = cursor->prev; ++distance; }
    }
}
// Select the shorter list length without changing reference-temporary lifetimes.
inline const unsigned int& Min146EC(const unsigned int& a, const unsigned int& b) {
    return a > b ? b : a;
}
// Copy color keys, reusing the common prefix; return self and leave other unchanged.
// Assignment phases (also used by size/frame below): find split=min(dst,src),
// stage any source suffix before mutating dst, overwrite common nodes, then
// splice staged nodes or unlink/free excess destination nodes. Nodes own no pointers.
// Volatile aliases/tag writes preserve the compiler's iterator/EH spills, not
// synchronization. Keep them when editing phases: removing them can change codegen.
List801146EC* FxSprayColorTrackList_Assign(List801146EC* self, List801146EC* other) {
    if (self != other) {
    const unsigned int count = Min146EC((unsigned int)self->size, (unsigned int)other->size);
    unsigned int total = other->size;
    unsigned int common = count;
    Node801146EC* cursor;
    if (common <= total / 2) {
        volatile Tag146EC tag;
        cursor = other->sentinel.next;
        tag.value = 0;
        int distance = common;
        Advance146EC(cursor, distance);
    } else { volatile Tag146EC tag; 
        cursor = &other->sentinel;

        tag.value = 0;
        Advance146EC(cursor, (common - total));
    }
    Node801146EC* volatile split = cursor;
    Node801146EC* mark = &other->sentinel;
    Node801146EC* volatile srcEnd = mark;
    Node801146EC* volatile endCopy = mark;
    Node801146EC* volatile splitCopy = cursor;
    Node801146EC* begin = other->sentinel.next;
    Node801146EC* volatile srcBegin = begin;
    // Longer source: stage suffix first so allocation failure precedes overwrites.
    if (cursor != &other->sentinel) {
        Copy146EC temp(cursor, mark);

        Node801146EC* step;
        Node801146EC* source;
        Node801146EC* firstValue;
        Node801146EC* stopValue;
        Node801146EC* last;
        Node801146EC* first;
        Node801146EC* out;
        stopValue = split;
        firstValue = srcBegin;
        Node801146EC* volatile stop = stopValue;
        out = self->sentinel.next;
        Node801146EC* volatile it = firstValue;
        while (it != stop) {
            source = it;
            step = it;
            *(float*)&out->value[0] = *(float*)&source->value[0];
            *(unsigned int*)&out->value[4] = *(unsigned int*)&source->value[4];
            *(float*)&out->value[8] = *(float*)&source->value[8];
            *(unsigned int*)&out->value[12] = *(unsigned int*)&source->value[12];
            it = step->next;
            out = out->next;
        }
        // Prefix is now copied; transfer suffix node ownership in one ring splice.
        if (temp.size) {
            first = temp.next;
            last = temp.prev;
            temp.next = (Node801146EC*)&temp.prev;
            temp.prev = (Node801146EC*)&temp.prev;
            Node801146EC* dstEnd = out;
            dstEnd->prev->next = first;
            first->prev = dstEnd->prev;
            dstEnd->prev = last;
            last->next = dstEnd;
            self->size = self->size + temp.size;
            temp.size = 0;
        }
    } else {
        Node801146EC* step;
        Node801146EC* source;
        Node801146EC* out;
        Node801146EC* volatile stop = cursor;
        out = self->sentinel.next;
        Node801146EC* volatile in = begin;
        while (in != stop) {
            source = in;
            step = in;
            *(float*)&out->value[0] = *(float*)&source->value[0];
            *(unsigned int*)&out->value[4] = *(unsigned int*)&source->value[4];
            *(float*)&out->value[8] = *(float*)&source->value[8];
            *(unsigned int*)&out->value[12] = *(unsigned int*)&source->value[12];
            out = out->next;
            in = step->next;
        }
        // Source exhausted: out starts the obsolete destination suffix.
        Node801146EC* volatile n = out;
        Node801146EC* end = &self->sentinel;
        if (out != end) {
            Node801146EC* last = end->prev;
            out->prev->next = last->next;
            last->next->prev = out->prev;
            Node801146EC* old;
            while ((old = n) != end) {
                n = old->next;
                appFreeUni__FPv(old);
                self->size = self->size - 1;
            }
        }
    }
    }
    return self;
}

#pragma pop
#pragma push
#pragma exceptions on
#pragma cpp_exceptions on
// Move a size-track iterator by a signed number of links.
inline void Advance14CA8(Node801146EC*& cursor, int distance) {
    if (distance >= 0) {
        while (distance > 0) { cursor = cursor->next; --distance; }
    } else {
        while (distance < 0) { cursor = cursor->prev; ++distance; }
    }
}
// Copy size keys using the common-prefix/stage/splice-or-erase policy above; return self.
List801146EC* FxSpraySizeTrackList_Assign(List801146EC* self, List801146EC* other) {
    if (self != other) {
    // Phase 1: locate the reusable-prefix boundary from the nearer end of the source.
    const unsigned int count = Min146EC((unsigned int)self->size, (unsigned int)other->size);
    int common = count;
    unsigned int total = other->size;
    Node801146EC* cursor;
    if (!(!(common <= total / 2))) {
        volatile Tag146EC tag;

        tag.value = 0;
        cursor = other->sentinel.next;
        Advance14CA8(cursor, (common));
    } else { volatile Tag146EC tag;

        cursor = &other->sentinel;
        tag.value = 0;
        Advance14CA8(cursor, (common - total));
    }
    Node801146EC* mark = &other->sentinel;
    Node801146EC* volatile split = cursor;
    Node801146EC* volatile srcEnd = mark;
    Node801146EC* volatile endCopy = mark;
    Node801146EC* volatile splitCopy = cursor;
    Node801146EC* begin = other->sentinel.next;
    Node801146EC* volatile srcBegin = begin;
    // Phase 2a: stage source suffix, overwrite reused nodes, then splice staged nodes.
    if (!(!(cursor != mark))) {
        Copy146EC temp(cursor, mark);
        Node801146EC* stopValue = split;
        Node801146EC* firstValue = srcBegin;

        Node801146EC* volatile stop = stopValue;
        Node801146EC* out = self->sentinel.next;
        Node801146EC* volatile it = firstValue;
        while (it != stop) {
            Node801146EC* step = it;
            Node801146EC* source = it;
            *(float*)&out->value[0] = *(float*)&source->value[0];
            *(float*)&out->value[4] = *(float*)&source->value[4];
            *(float*)&out->value[8] = *(float*)&source->value[8];
            *(float*)&out->value[12] = *(float*)&source->value[12];
            out = out->next;
            it = step->next;
        }
        if (temp.size) {
            Node801146EC* first = temp.next;
            Node801146EC* last = temp.prev;
            temp.next = (Node801146EC*)&temp.prev;
            temp.prev = (Node801146EC*)&temp.prev;
            out->prev->next = first;
            first->prev = out->prev;
            last->next = out;
            out->prev = last;
            self->size = (((self->size + temp.size)));
            temp.size = 0;
        }
    } else {
        // Phase 2b: source fits; overwrite its length, then unlink/free the old dst suffix.
        Node801146EC* volatile in = begin;
        Node801146EC* out = self->sentinel.next;
        Node801146EC* volatile stop = cursor;
        while (in != stop) {
            Node801146EC* step = in;
            Node801146EC* source = in;
            *(float*)&out->value[0] = *(float*)&source->value[0];
            *(float*)&out->value[4] = *(float*)&source->value[4];
            *(float*)&out->value[8] = *(float*)&source->value[8];
            *(float*)&out->value[12] = *(float*)&source->value[12];
            in = step->next;
            out = out->next;
        }
        Node801146EC* end = &self->sentinel;
        Node801146EC* volatile n = out;
        if (out != end) {
            Node801146EC* last = end->prev;
            out->prev->next = last->next;
            last->next->prev = out->prev;
            Node801146EC* old;
            while ((old = n) != end) {
                n = old->next;
                appFreeUni__FPv(old);
                self->size = self->size - 1;
            }
        }
    }
    }
    return self;
}
#pragma pop

#pragma push
#pragma exceptions on
#pragma cpp_exceptions on
struct Payload15264 {
    // Frame key: {float, float, u32, u32, u32}; exact per-word roles unresolved.
    unsigned int a, b, c, d, e;
    // Copy the full 20-byte frame key, including the word outside the common node view.
    Payload15264(const Payload15264& p) {
        unsigned int first = p.a, second = p.b;
        try {
            a = first; b = second;
            unsigned int fourth = p.d, third = p.c;
            c = third; d = fourth;
            e = p.e;
        } catch (...) { throw; }
    }
};
struct Copy15264 : TempCopy146EC {
    // Stage frame nodes with 28-byte allocations, rather than the color/size node's 24.
    Copy15264(Node801146EC* first, Node801146EC* last) {
        volatile Tag146EC tag;
        tag.value = 0;
        Node801146EC* volatile stop = last;
        Node801146EC* volatile it = first;
        if (first != last) {
        while (it != stop) {
            Node801146EC* end = (Node801146EC*)&prev;
            Node801146EC* source = it;
            volatile Tag146EC tag;
            tag.value = 0;
            {
            Node801146EC* node = (Node801146EC*)appMallocDummy__FUl(28);
            Node801146EC* volatile allocated = node;
            Node801146EC* volatile original = node;
            Node801146EC* volatile position = end;
            Allocation146EC allocation(node);
            Payload15264* payload = (Payload15264*)((char*)node + 8);
            new(payload) Payload15264(*(Payload15264*)source->value);
            Node801146EC* mem = allocation.p;
            allocation.p = 0;
            end->prev->next = mem;
            mem->prev = end->prev;
            end->prev = mem;
            mem->next = end;
            ++size;
            }
            it = it->next;
        }
        }
    }
};
// Select the shorter frame-track list length.
inline const unsigned int& Min15264(const unsigned int& a, const unsigned int& b) {
    return a > b ? b : a;
}

// Copy full 20-byte frame keys, reusing nodes and transferring only staged ownership; return self.
List801146EC* FxSpraySpriteFrameTrackList_Assign(List801146EC* self, List801146EC* other) {
    if (self != other) {
    // Phase 1: locate split=min(dst.size,src.size), walking from the nearer source end.
    const unsigned int count = Min15264((unsigned int)self->size, (unsigned int)other->size);
    unsigned int total = other->size;
    unsigned int common = count;
    Node801146EC* cursor;
    if (common <= total / 2) {
        volatile Tag146EC tag;
        cursor = other->sentinel.next;
        tag.value = 0;
        int distance = common;
        Advance146EC(cursor, distance);
    } else { volatile Tag146EC tag;
        cursor = &other->sentinel;

        tag.value = 0;
        Advance146EC(cursor, (common - total));
    }
    Node801146EC* volatile split = cursor;
    Node801146EC* mark = &other->sentinel;
    Node801146EC* volatile srcEnd = mark;
    Node801146EC* volatile endCopy = mark;
    Node801146EC* volatile splitCopy = cursor;
    Node801146EC* begin = other->sentinel.next;
    Node801146EC* volatile srcBegin = begin;
    if (cursor != &other->sentinel) {
        // Phase 2a: copy the extra 20-byte keys before overwriting reused destination nodes.
        Copy15264 temp(cursor, mark);

        Node801146EC* step;
        Node801146EC* source;
        Node801146EC* firstValue;
        Node801146EC* stopValue;
        Node801146EC* last;
        Node801146EC* first;
        Node801146EC* out;
        stopValue = split;
        firstValue = srcBegin;
        Node801146EC* volatile stop = stopValue;
        out = self->sentinel.next;
        Node801146EC* volatile it = firstValue;
        while (it != stop) {
            source = it;
            step = it;
            *(float*)&out->value[0] = *(float*)&source->value[0];
            *(float*)&out->value[4] = *(float*)&source->value[4];
            *(unsigned int*)&out->value[8] = *(unsigned int*)&source->value[8];
            *(unsigned int*)&out->value[12] = *(unsigned int*)&source->value[12];
            *(unsigned int*)((char*)out + 24) = *(unsigned int*)((char*)source + 24);
            it = step->next;
            out = out->next;
        }
        if (temp.size) {
            first = temp.next;
            last = temp.prev;
            temp.next = (Node801146EC*)&temp.prev;
            temp.prev = (Node801146EC*)&temp.prev;
            Node801146EC* dstEnd = out;
            dstEnd->prev->next = first;
            first->prev = dstEnd->prev;
            dstEnd->prev = last;
            last->next = dstEnd;
            self->size = self->size + temp.size;
            temp.size = 0;
        }
    } else {
        Node801146EC* step;
        Node801146EC* source;
        Node801146EC* out;
        Node801146EC* volatile stop = cursor;
        out = self->sentinel.next;
        Node801146EC* volatile in = begin;
        while (in != stop) {
            source = in;
            step = in;
            *(float*)&out->value[0] = *(float*)&source->value[0];
            *(float*)&out->value[4] = *(float*)&source->value[4];
            *(unsigned int*)&out->value[8] = *(unsigned int*)&source->value[8];
            *(unsigned int*)&out->value[12] = *(unsigned int*)&source->value[12];
            *(unsigned int*)((char*)out + 24) = *(unsigned int*)((char*)source + 24);
            out = out->next;
            in = step->next;
        }
        // Phase 2b: out begins the destination suffix to unlink and free after copying.
        Node801146EC* volatile n = out;
        Node801146EC* end = &self->sentinel;
        if (out != end) {
            Node801146EC* last = end->prev;
            out->prev->next = last->next;
            last->next->prev = out->prev;
            Node801146EC* old;
            while ((old = n) != end) {
                n = old->next;
                appFreeUni__FPv(old);
                self->size = self->size - 1;
            }
        }
    }
    }
    return self;
}
#pragma pop

} // extern "C"
