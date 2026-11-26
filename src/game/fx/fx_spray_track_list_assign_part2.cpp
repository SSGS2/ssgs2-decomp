// Copy assignment for the spray color, size and sprite-frame track lists.
// Existing nodes are reused, and new nodes are staged before they are spliced into the list.

typedef unsigned long new_size_t;

#pragma push
#pragma exceptions on
#pragma cpp_exceptions on
// Construct into caller-provided storage without allocating.
inline void* operator new(new_size_t, void* p) {
    return p;
}

extern "C" {

void appFreeUni__FPv(void*);
void* appMallocDummy__FUl(unsigned int);
void dtor_80110418(void*);

// Intrusive circular list links occupy +0/+4; payload starts at +8.
// Color/size nodes allocate 24 bytes (16-byte payload); frame nodes allocate
// 28 bytes (20-byte payload). value[16] is ONLY the common prefix view:
// frame word 5 is accessed through node+24, within that larger allocation.
// Never use sizeof(CTrackNode) to allocate/copy a frame node.
struct CTrackNode {
    CTrackNode* prev;
    CTrackNode* next;
    unsigned char value[16];
};

// size counts real nodes. Only sentinel.prev/next are live sentinel fields;
// its value bytes and the trailing end field are not part of a key.
struct CTrackList {
    unsigned int size;
    CTrackNode sentinel;
    CTrackNode* end;
};

struct Temp146EC {
    unsigned int size;
    CTrackNode* prev;
    CTrackNode* next;
    // Create an empty ring whose sentinel consists only of these two link words.
    Temp146EC() : size(0) {
        prev = next = (CTrackNode*)&prev;
    }
    // Release owned nodes, never the embedded sentinel.
    ~Temp146EC() {
        CTrackNode* n = next;
        CTrackNode* end = (CTrackNode*)&prev;
        if (n != end) {
            CTrackNode* last = end->prev;
            n->prev->next = last->next;
            last->next->prev = n->prev;
            while (n != end) {
                CTrackNode* old = n;
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
struct CTrackKey {
    unsigned int a, b, c, d;
    // Copy all payload bits with the original exception construction order.
    CTrackKey(const CTrackKey& p) {
        unsigned int first = p.a, second = p.b;
        try {
            a = first;
            b = second;
            unsigned int third = p.c, fourth = p.d;
            c = third;
            d = fourth;
        } catch (...) {
            throw;
        }
    }
};
struct Tag146EC {
    char value;
    char padding[3];
};
struct CNodeAllocation {
    CTrackNode* p;
    // Own a newly allocated node until it has been constructed and linked.
    CNodeAllocation(CTrackNode* n) : p(n) {}
    // Roll back an uncommitted allocation on exceptional exits.
    ~CNodeAllocation() {
        if (p)
            appFreeUni__FPv(p);
    }
};
struct CStagingList {
    volatile unsigned int size;
    CTrackNode* volatile prev;
    CTrackNode* volatile next;
    // Initialize an empty temporary list used to stage an assignment's extra nodes.
    CStagingList() {
        CTrackNode* end = (CTrackNode*)&prev;
        prev = next = end;
        size = 0;
    }
    // Free nodes still owned by the staging list; successful splice sets size to zero.
    ~CStagingList() {
        CTrackNode* n = next;
        CTrackNode* end = (CTrackNode*)&prev;
        if (n != end) {
            CTrackNode* last = end->prev;
            n->prev->next = last->next;
            last->next->prev = n->prev;
            while (n != end) {
                CTrackNode* old = n;
                n = n->next;
                appFreeUni__FPv(old);
                --size;
            }
        }
    }
};
struct Copy146EC : CStagingList {
    // Deep-copy the half-open node range [first,last) into this temporary ring.
    Copy146EC(CTrackNode* first, CTrackNode* last) {
        volatile Tag146EC tag;
        tag.value = 0;
        CTrackNode* volatile stop = last;
        CTrackNode* volatile it = first;
        if (first != last) {
            while (it != stop) {
                CTrackNode* end = (CTrackNode*)&prev;
                CTrackNode* source = it;
                volatile Tag146EC tag;
                tag.value = 0;
                {
                    CTrackNode* node = (CTrackNode*)appMallocDummy__FUl(24);
                    CTrackNode* volatile allocated = node;
                    CTrackNode* volatile original = node;
                    CTrackNode* volatile position = end;
                    CNodeAllocation allocation(node);
                    CTrackKey* payload = (CTrackKey*)((char*)node + 8);
                    new (payload) CTrackKey(*(CTrackKey*)source->value);
                    CTrackNode* mem = allocation.p;
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
inline void Advance146EC(CTrackNode*& cursor, int distance) {
    if (distance >= 0) {
        while (distance > 0) {
            cursor = cursor->next;
            --distance;
        }
    } else {
        while (distance < 0) {
            cursor = cursor->prev;
            ++distance;
        }
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
CTrackList* FxSprayColorTrackList_Assign(CTrackList* self, CTrackList* other) {
    if (self != other) {
        const unsigned int count = Min146EC((unsigned int)self->size, (unsigned int)other->size);
        unsigned int total = other->size;
        unsigned int common = count;
        CTrackNode* cursor;
        if (common <= total / 2) {
            volatile Tag146EC tag;
            cursor = other->sentinel.next;
            tag.value = 0;
            int distance = common;
            Advance146EC(cursor, distance);
        } else {
            volatile Tag146EC tag;
            cursor = &other->sentinel;

            tag.value = 0;
            Advance146EC(cursor, (common - total));
        }
        CTrackNode* volatile split = cursor;
        CTrackNode* mark = &other->sentinel;
        CTrackNode* volatile srcEnd = mark;
        CTrackNode* volatile endCopy = mark;
        CTrackNode* volatile splitCopy = cursor;
        CTrackNode* begin = other->sentinel.next;
        CTrackNode* volatile srcBegin = begin;
        // Longer source: stage suffix first so allocation failure precedes overwrites.
        if (cursor != &other->sentinel) {
            Copy146EC temp(cursor, mark);

            CTrackNode* step;
            CTrackNode* source;
            CTrackNode* firstValue;
            CTrackNode* stopValue;
            CTrackNode* last;
            CTrackNode* first;
            CTrackNode* out;
            stopValue = split;
            firstValue = srcBegin;
            CTrackNode* volatile stop = stopValue;
            out = self->sentinel.next;
            CTrackNode* volatile it = firstValue;
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
                temp.next = (CTrackNode*)&temp.prev;
                temp.prev = (CTrackNode*)&temp.prev;
                CTrackNode* dstEnd = out;
                dstEnd->prev->next = first;
                first->prev = dstEnd->prev;
                dstEnd->prev = last;
                last->next = dstEnd;
                self->size = self->size + temp.size;
                temp.size = 0;
            }
        } else {
            CTrackNode* step;
            CTrackNode* source;
            CTrackNode* out;
            CTrackNode* volatile stop = cursor;
            out = self->sentinel.next;
            CTrackNode* volatile in = begin;
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
            CTrackNode* volatile n = out;
            CTrackNode* end = &self->sentinel;
            if (out != end) {
                CTrackNode* last = end->prev;
                out->prev->next = last->next;
                last->next->prev = out->prev;
                CTrackNode* old;
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
inline void Advance14CA8(CTrackNode*& cursor, int distance) {
    if (distance >= 0) {
        while (distance > 0) {
            cursor = cursor->next;
            --distance;
        }
    } else {
        while (distance < 0) {
            cursor = cursor->prev;
            ++distance;
        }
    }
}
// Copy size keys using the common-prefix/stage/splice-or-erase policy above; return self.
CTrackList* FxSpraySizeTrackList_Assign(CTrackList* self, CTrackList* other) {
    if (self != other) {
        // Phase 1: locate the reusable-prefix boundary from the nearer end of the source.
        const unsigned int count = Min146EC((unsigned int)self->size, (unsigned int)other->size);
        int common = count;
        unsigned int total = other->size;
        CTrackNode* cursor;
        if (!(!(common <= total / 2))) {
            volatile Tag146EC tag;

            tag.value = 0;
            cursor = other->sentinel.next;
            Advance14CA8(cursor, (common));
        } else {
            volatile Tag146EC tag;

            cursor = &other->sentinel;
            tag.value = 0;
            Advance14CA8(cursor, (common - total));
        }
        CTrackNode* mark = &other->sentinel;
        CTrackNode* volatile split = cursor;
        CTrackNode* volatile srcEnd = mark;
        CTrackNode* volatile endCopy = mark;
        CTrackNode* volatile splitCopy = cursor;
        CTrackNode* begin = other->sentinel.next;
        CTrackNode* volatile srcBegin = begin;
        // Phase 2a: stage source suffix, overwrite reused nodes, then splice staged nodes.
        if (!(!(cursor != mark))) {
            Copy146EC temp(cursor, mark);
            CTrackNode* stopValue = split;
            CTrackNode* firstValue = srcBegin;

            CTrackNode* volatile stop = stopValue;
            CTrackNode* out = self->sentinel.next;
            CTrackNode* volatile it = firstValue;
            while (it != stop) {
                CTrackNode* step = it;
                CTrackNode* source = it;
                *(float*)&out->value[0] = *(float*)&source->value[0];
                *(float*)&out->value[4] = *(float*)&source->value[4];
                *(float*)&out->value[8] = *(float*)&source->value[8];
                *(float*)&out->value[12] = *(float*)&source->value[12];
                out = out->next;
                it = step->next;
            }
            if (temp.size) {
                CTrackNode* first = temp.next;
                CTrackNode* last = temp.prev;
                temp.next = (CTrackNode*)&temp.prev;
                temp.prev = (CTrackNode*)&temp.prev;
                out->prev->next = first;
                first->prev = out->prev;
                last->next = out;
                out->prev = last;
                self->size = (((self->size + temp.size)));
                temp.size = 0;
            }
        } else {
            // Phase 2b: source fits; overwrite its length, then unlink/free the old dst suffix.
            CTrackNode* volatile in = begin;
            CTrackNode* out = self->sentinel.next;
            CTrackNode* volatile stop = cursor;
            while (in != stop) {
                CTrackNode* step = in;
                CTrackNode* source = in;
                *(float*)&out->value[0] = *(float*)&source->value[0];
                *(float*)&out->value[4] = *(float*)&source->value[4];
                *(float*)&out->value[8] = *(float*)&source->value[8];
                *(float*)&out->value[12] = *(float*)&source->value[12];
                in = step->next;
                out = out->next;
            }
            CTrackNode* end = &self->sentinel;
            CTrackNode* volatile n = out;
            if (out != end) {
                CTrackNode* last = end->prev;
                out->prev->next = last->next;
                last->next->prev = out->prev;
                CTrackNode* old;
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
struct CFrameTrackKey {
    // Frame key: {float, float, u32, u32, u32}; exact per-word roles unresolved.
    unsigned int a, b, c, d, e;
    // Copy the full 20-byte frame key, including the word outside the common node view.
    CFrameTrackKey(const CFrameTrackKey& p) {
        unsigned int first = p.a, second = p.b;
        try {
            a = first;
            b = second;
            unsigned int fourth = p.d, third = p.c;
            c = third;
            d = fourth;
            e = p.e;
        } catch (...) {
            throw;
        }
    }
};
struct Copy15264 : CStagingList {
    // Stage frame nodes with 28-byte allocations, rather than the color/size node's 24.
    Copy15264(CTrackNode* first, CTrackNode* last) {
        volatile Tag146EC tag;
        tag.value = 0;
        CTrackNode* volatile stop = last;
        CTrackNode* volatile it = first;
        if (first != last) {
            while (it != stop) {
                CTrackNode* end = (CTrackNode*)&prev;
                CTrackNode* source = it;
                volatile Tag146EC tag;
                tag.value = 0;
                {
                    CTrackNode* node = (CTrackNode*)appMallocDummy__FUl(28);
                    CTrackNode* volatile allocated = node;
                    CTrackNode* volatile original = node;
                    CTrackNode* volatile position = end;
                    CNodeAllocation allocation(node);
                    CFrameTrackKey* payload = (CFrameTrackKey*)((char*)node + 8);
                    new (payload) CFrameTrackKey(*(CFrameTrackKey*)source->value);
                    CTrackNode* mem = allocation.p;
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
CTrackList* FxSpraySpriteFrameTrackList_Assign(CTrackList* self, CTrackList* other) {
    if (self != other) {
        // Phase 1: locate split=min(dst.size,src.size), walking from the nearer source end.
        const unsigned int count = Min15264((unsigned int)self->size, (unsigned int)other->size);
        unsigned int total = other->size;
        unsigned int common = count;
        CTrackNode* cursor;
        if (common <= total / 2) {
            volatile Tag146EC tag;
            cursor = other->sentinel.next;
            tag.value = 0;
            int distance = common;
            Advance146EC(cursor, distance);
        } else {
            volatile Tag146EC tag;
            cursor = &other->sentinel;

            tag.value = 0;
            Advance146EC(cursor, (common - total));
        }
        CTrackNode* volatile split = cursor;
        CTrackNode* mark = &other->sentinel;
        CTrackNode* volatile srcEnd = mark;
        CTrackNode* volatile endCopy = mark;
        CTrackNode* volatile splitCopy = cursor;
        CTrackNode* begin = other->sentinel.next;
        CTrackNode* volatile srcBegin = begin;
        if (cursor != &other->sentinel) {
            // Phase 2a: copy the extra 20-byte keys before overwriting reused destination nodes.
            Copy15264 temp(cursor, mark);

            CTrackNode* step;
            CTrackNode* source;
            CTrackNode* firstValue;
            CTrackNode* stopValue;
            CTrackNode* last;
            CTrackNode* first;
            CTrackNode* out;
            stopValue = split;
            firstValue = srcBegin;
            CTrackNode* volatile stop = stopValue;
            out = self->sentinel.next;
            CTrackNode* volatile it = firstValue;
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
                temp.next = (CTrackNode*)&temp.prev;
                temp.prev = (CTrackNode*)&temp.prev;
                CTrackNode* dstEnd = out;
                dstEnd->prev->next = first;
                first->prev = dstEnd->prev;
                dstEnd->prev = last;
                last->next = dstEnd;
                self->size = self->size + temp.size;
                temp.size = 0;
            }
        } else {
            CTrackNode* step;
            CTrackNode* source;
            CTrackNode* out;
            CTrackNode* volatile stop = cursor;
            out = self->sentinel.next;
            CTrackNode* volatile in = begin;
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
            CTrackNode* volatile n = out;
            CTrackNode* end = &self->sentinel;
            if (out != end) {
                CTrackNode* last = end->prev;
                out->prev->next = last->next;
                last->next->prev = out->prev;
                CTrackNode* old;
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
