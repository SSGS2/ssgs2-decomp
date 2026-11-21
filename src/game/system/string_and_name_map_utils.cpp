// Translation unit 0x801169F4-0x801177B4 (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

inline void* operator new(size_t, void* p) { return p; }
inline void operator delete(void*, void*) {}

extern "C" {

enum StringStorageConstants {
    kHeapStringBit = 31,
    kShortStringLengthMask = 0x7f,
    kInlineStringDataOffset = 1
};
// List sentinels reuse the adjacent last/first pointers as a prev/next node header.
// Keep the iterator/allocation scopes: their destructors describe exception cleanup.

struct StringRep {
    unsigned char flags;
    unsigned char pad[3];
    unsigned int length;
    const char* data;
};

struct StringObj {
    unsigned int word;
    unsigned int length;
    const char* data;
    StringObj(const StringObj&);
};

extern StringObj* StringTree_FindInsertionPosition(void*, StringObj*, StringObj**, unsigned char*, unsigned char*);
extern StringObj* NamedObjectMap_InsertNode(void*, StringObj*, unsigned char, unsigned char, StringObj*);
extern void String_ReserveCapacity(StringObj*, unsigned int);
extern void String_ReplaceRange(StringObj*, int, int, const char*, const char*, unsigned char*);
extern void appFreeUni__FPv(const char*);
extern unsigned int s_fxSprayNameDefaultValue;
extern void StringObj_dtor(StringObj*);
extern void* appMallocDummy__FUl(unsigned int);

struct ListNode {
    ListNode* prev;
    ListNode* next;
    struct Value {
        unsigned int id;
        unsigned char recordBytes[0x104];
        unsigned char recordFlag;
        float amount;
        unsigned int trailingValue;
    } value;
};
struct ListRep {
    unsigned int size;
    ListNode* last;
    ListNode* first;
};

struct AssignIter {
    ListNode* p;
    ~AssignIter() {}
    AssignIter& operator++() { p = p->next; return *this; }
    AssignIter& operator--() { p = p->prev; return *this; }
};
struct AssignAllocator {
    unsigned char tag;
    ~AssignAllocator() {}
};
// Walk a circular list forward or backward by a signed node count.
static inline ListNode* advanceAssign(int n, ListNode* p) {
    if (n >= 0) { while (n > 0) { p = p->next; --n; } }
    else { while (n < 0) { p = p->prev; ++n; } }
    return p;
}
#pragma push
#pragma inline_depth(1024)
#pragma use_lmw_stmw on
#pragma optimize_for_size off
#pragma scheduling on
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
// Unlink and free the half-open range, updating the owning list's size.
static inline void eraseAssign(ListRep* l, AssignIter& first, AssignIter& last) {
    if (first.p != last.p) {
        ListNode* back = last.p->prev;
        first.p->prev->next = back->next;
        back->next->prev = first.p->prev;
        while (first.p != last.p) {
            ListNode* n = first.p;
            ++first;
            appFreeUni__FPv((const char*)n);
            --l->size;
        }
    }
}
struct AssignTempBase : ListRep {
    AssignTempBase() { size = 0; last = first = (ListNode*)&last; }
    ~AssignTempBase() {
        ListNode* p = first;
        ListNode* end = (ListNode*)&last;
        if (p != end) {
            ListNode* back = end->prev;
            p->prev->next = back->next;
            back->next->prev = p->prev;
            while (p != end) {
                ListNode* n = p;
                p = p->next;
                appFreeUni__FPv((const char*)n);
                --size;
            }
        }
    }
};
struct AssignAllocation {
    ListNode* p;
    AssignAllocation(ListNode* n) : p(n) {}
    ~AssignAllocation() { if (p) appFreeUni__FPv((const char*)p); }
    ListNode* release() { ListNode* n = p; p = 0; return n; }
};
struct AssignTemp : AssignTempBase {
    AssignTemp(AssignIter& rangeMid, AssignIter& rangeEnd) {
        AssignAllocator allocator = {0};
        AssignIter finish = rangeEnd;
        for (AssignIter p = rangeMid; p.p != finish.p; ++p) {
            AssignAllocator allocator = {0};
            ListNode* t = (ListNode*)&last;
            ListNode* source = p.p;
            ListNode* n = (ListNode*)appMallocDummy__FUl(sizeof(ListNode));
            AssignIter allocated = {n};
            AssignIter reserved = allocated;
            AssignIter position = {t};
            AssignAllocation allocation(n);
            new (&n->value) ListNode::Value(source->value);
            n = allocation.release();
            t->prev->next = n;
            n->prev = t->prev;
            t->prev = n;
            n->next = t;
            ++size;
        }
    }
};
static inline const unsigned int& minAssign(const unsigned int& a, const unsigned int& b) {
    return b < a ? b : a;
}
static inline unsigned int sizeAssign(const ListRep* l) { return l->size; }
// Reach an index from the nearer end; the sentinel follows the last node.
static inline ListNode* nthAssign(const ListRep* other, unsigned int count) {
    unsigned int total = other->size;
    if (count <= total / 2) {
        AssignAllocator allocator = {0};
        return advanceAssign(count, other->first);
    } else {
        AssignAllocator allocator = {0};
        return advanceAssign(count - total, (ListNode*)&other->last);
    }
}

// Assign the record list, reusing the shared prefix and allocating or erasing the size difference.
ListRep* IdRecordList_Assign(ListRep* self, const ListRep* other) {
    if (self != other) {
        ListNode* middle = nthAssign(other, minAssign(sizeAssign(self), sizeAssign(other)));
        AssignIter mid = {middle};
        AssignIter end = {(ListNode*)&other->last};
        AssignIter begin = {other->first};

        AssignIter rangeMid = mid;
        if (mid.p != end.p) {
            AssignTemp extraRecords(rangeMid, (end));
            ListNode* destinationNode = self->first;
            AssignIter finishCopy = mid;
            for (AssignIter sourceIterator = begin; sourceIterator.p != finishCopy.p; ++sourceIterator) {
                destinationNode->value = sourceIterator.p->value;
                destinationNode = destinationNode->next;
            }
            if (extraRecords.size) {
                ListNode* last = extraRecords.last;
                ListNode* first = extraRecords.first;
                extraRecords.first = extraRecords.last = (ListNode*)&extraRecords.last;
                destinationNode->prev->next = first;
                first->prev = destinationNode->prev;
                destinationNode->prev = last;
                last->next = destinationNode;
                self->size += extraRecords.size;
                extraRecords.size = 0;
            }
        } else { AssignIter finishCopy = mid; 
            ListNode* destinationNode = self->first;

            for (AssignIter sourceIterator = begin; sourceIterator.p != finishCopy.p; ++sourceIterator) {
                destinationNode->value = sourceIterator.p->value;
                destinationNode = destinationNode->next;
            }
            AssignIter eraseIterator = {destinationNode};
            ListNode* destinationEnd = (ListNode*)&self->last;
            if (eraseIterator.p != destinationEnd) {
                ListNode* back = destinationEnd->prev;
                eraseIterator.p->prev->next = back->next;
                back->next->prev = eraseIterator.p->prev;
                ListNode* erasedNode;
                while ((erasedNode = eraseIterator.p) != destinationEnd) {
                    eraseIterator.p = erasedNode->next;
                    appFreeUni__FPv((const char*)erasedNode);
                    --self->size;
                }
            }
        }
    }
    return self;
}
#pragma pop

struct FxSprayNameNodePrefix {
    unsigned int treeLinks[3];
    unsigned char record[1];
};

// Skip the tree's three link words and return this iterator's stored record.
void* FxSprayNameIterator_GetRecord(void** iterator) {
    return (char*)*iterator + (int)&((FxSprayNameNodePrefix*)0)->record;
}

struct Val {
    unsigned char recordBytes[0x104];
    float amount;
};
struct LNode {
    LNode* prev;
    LNode* next;
    Val record;
};
struct LList {
    unsigned int size;
    LNode* last;
    LNode* first;
};

struct LIter {
    LNode* p;
    LIter& operator++() { p = p->next; return *this; }
    LIter& operator--() { p = p->prev; return *this; }
};

// Advance the second record-list iterator in either direction.
static inline void adv(LIter& it, int n) {
    if (n > 0) {
        for (; n > 0; --n) ++it;
    } else {
        for (; n < 0; ++n) --it;
    }
}

struct TmpList : LList {
    TmpList() {
        size = 0;
        last = first = (LNode*)&last;
    }
    ~TmpList() {
        LNode* e = first;
        LNode* te = (LNode*)&last;
        if (e != te) {
            e->prev->next = te->next;
            te->next->prev = e->prev;
            while (e != te) {
                LNode* nx = e->next;
                appFreeUni__FPv((const char*)e);
                --size;
                e = nx;
            }
        }
    }
};

// Assign the second record-list specialization, retaining nodes for the common prefix.
LList* RecordAmountList_Assign(LList* self, LList* other) {
    if (self != other) {
        unsigned int destinationSize = self->size;
        unsigned int sourceSize = other->size;
        const unsigned int* minimumSize = &destinationSize;
        if (sourceSize < destinationSize) minimumSize = &sourceSize;
        unsigned int commonCount = *minimumSize;
        LIter middleIterator;
        LNode* sourceEnd = (LNode*)&other->last;
        if (commonCount > other->size / 2) {
            middleIterator.p = sourceEnd;
            adv(middleIterator, (int)(commonCount - other->size));
        } else {
            middleIterator.p = other->first;
            adv(middleIterator, (int)commonCount);
        }
        LNode* sourceMiddle = middleIterator.p;
        LNode* sourceBegin = other->first;
        LNode* destinationEnd = (LNode*)&self->last;
        if (sourceMiddle != sourceEnd) {
            TmpList extraRecords;
            for (LNode* sourceNode = sourceMiddle; sourceNode != sourceEnd; sourceNode = sourceNode->next) {
                LNode* newNode = (LNode*)appMallocDummy__FUl(sizeof(LNode));
                if (newNode) newNode->record = sourceNode->record;
                LNode* temporarySentinel = (LNode*)&extraRecords.last;
                LNode* temporaryTail = temporarySentinel->prev;
                newNode->prev = temporaryTail;
                newNode->next = temporarySentinel;
                temporarySentinel->prev = newNode;
                temporaryTail->next = newNode;
                ++extraRecords.size;
            }
            LNode* destinationNode = self->first;
            for (LNode* sourceNode = sourceBegin; sourceNode != sourceMiddle; sourceNode = sourceNode->next) {
                destinationNode->record = sourceNode->record;
                destinationNode = destinationNode->next;
            }
            if (extraRecords.size) {
                LNode* copiedFirst = extraRecords.first;
                LNode* copiedLast = extraRecords.last;
                extraRecords.first = extraRecords.last = (LNode*)&extraRecords.last;
                destinationNode->prev->next = copiedFirst;
                copiedFirst->prev = destinationNode->prev;
                destinationNode->prev = copiedLast;
                copiedLast->next = destinationNode;
                self->size += extraRecords.size;
                extraRecords.size = 0;
            }
        } else {
            LNode* destinationNode = self->first;
            for (LNode* sourceNode = sourceBegin; sourceNode != sourceMiddle; sourceNode = sourceNode->next) {
                destinationNode->record = sourceNode->record;
                destinationNode = destinationNode->next;
            }
            LNode* eraseNode = destinationNode;
            if (eraseNode != destinationEnd) {
                eraseNode->prev->next = destinationEnd;
                destinationEnd->prev = eraseNode->prev;
                while (eraseNode != destinationEnd) {
                    LNode* nextNode = eraseNode->next;
                    appFreeUni__FPv((const char*)eraseNode);
                    --self->size;
                    eraseNode = nextNode;
                }
            }
        }
    }
    return self;
}

#pragma push
#pragma inline_depth(1024)
// The high bit selects heap storage; short strings keep their length in the low seven header bits.
static inline bool longString(const StringObj* s) { return (s->word >> kHeapStringBit) != 0; }
static inline unsigned int stringSize(const StringObj* s) {
    unsigned int n;
    if (!longString(s)) n = *(const unsigned char*)s & kShortStringLengthMask;
    else n = s->length;
    return n;
}
static inline const unsigned int& minString(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Compare the key against the stored string lexicographically, using its bounded byte range.
static inline int compareString(const StringObj* key, const StringObj* b) {
    unsigned int count, lenCopy, ncopy, limit;
    unsigned int n;
    const char* data;
    if (!longString(b)) { data = (const char*)b + kInlineStringDataOffset; n = *(const unsigned char*)b & kShortStringLengthMask; }
    else { data = b->data; n = b->length; }
    ncopy = n;
    limit = longString(key) ? key->length : (*(const unsigned char*)key & kShortStringLengthMask);
    unsigned int len;
    const char* keydata;
    if (!longString(key)) { keydata = (const char*)key + kInlineStringDataOffset; len = *(const unsigned char*)key & kShortStringLengthMask; }
    else { keydata = key->data; len = key->length; }
    lenCopy = len;
    count = minString(limit, lenCopy);
    int cmp = memcmp(keydata, data, minString(ncopy, count));
    if (!cmp) {
        if (count < n) return -1;
        if (count == n) return 0;
        return 1;
    }
    return cmp;
}
struct StringAllocator {
    unsigned char tag;
    StringAllocator() {}
    StringAllocator(const StringAllocator& a) : tag(a.tag) {}
    ~StringAllocator() {}
};
static inline void copyStringRange(StringObj* s, const char* begin, const char* end, StringAllocator allocator) {
    String_ReplaceRange(s, 0, 0, begin, end, &allocator.tag);
}
inline StringObj::StringObj(const StringObj& b) {
    StringObj& str = *this;
        if (!longString(&b)) {
            unsigned int header = b.word;
            unsigned int size = b.length;
            const char* data = b.data;
            str.word = header;
            str.length = size;
            str.data = data;
        }
        else {
            unsigned int size = b.length;
            str.word = 0;
            str.length = 0; str.data = 0;
            String_ReserveCapacity(&str, size);
            StringAllocator allocator;
            copyStringRange(&str, b.data, b.data + b.length, allocator);
        }
}
struct StringValue {
    StringObj str;
    unsigned int value;
    StringValue(const StringObj& b) : str(b), value(s_fxSprayNameDefaultValue) {}
    ~StringValue() { if (longString(&str)) appFreeUni__FPv(str.data); }
};
// Look up a spray-name key, inserting its default value when absent, and return the value address.
void* FxSprayNameTree_GetOrInsertValue(void* tree, StringObj* key) {
    unsigned char insertSide, insertionFlags;
    StringObj* found;
    StringObj* end;
    StringObj* result;
    end = StringTree_FindInsertionPosition(tree, key, &found, &insertSide, &insertionFlags);
    // 0x801175DC: reuse the existing value or construct the missing entry.
    if (!found || (result = (StringObj*)((char*)found + (int)&((FxSprayNameNodePrefix*)0)->record), compareString(result, key) < 0)) {
        StringValue temp(*key);
        result = (StringObj*)((char*)NamedObjectMap_InsertNode(tree, end, insertSide, insertionFlags, &temp.str) + (int)&((FxSprayNameNodePrefix*)0)->record);
    }
    return &((StringValue*)result)->value;
}
#pragma pop

} // extern "C"
