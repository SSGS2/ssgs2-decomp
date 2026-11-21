typedef unsigned long new_size_t;
#include "classes/ItemRequestState80300EC8.h"
// Translation unit 0x80300EC8-0x80301720 (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern int ItemRequestState_ProcessPopupQueue(void* self);
extern int ItemRequestDialogs_ProcessQueue(void* self);
extern void appFreeUni__FPv(void* ptr);
extern void __dl__FPv(void* ptr);
extern int SaveRewards_TestPartFlag(void*, int, int);
extern int SaveData_TestAllSetItemPartFlags(void*, unsigned char, int);
extern int SaveRewards_TestClubSetFlag(void*, unsigned char, int);
extern int SaveRewards_TestBallFlag(void*, unsigned char, int);
extern int SaveRewards_TestCharacterFlag(void*, unsigned char, int);
extern int SaveRewards_TestCaddieFlag(void*, unsigned char, int);
extern int SaveRewards_TestCaddieItemFlag(void*, unsigned char, int);
extern int SaveRewards_IsCourseUnlocked(void*, unsigned char);
extern void ItemRequestVector_AppendCopy(void*, void*);
extern void* appMallocDummy__FUl(unsigned int size);
extern void WString_ReserveCapacity(void*, int);
extern void WString_ReplaceRange(void*, int, int, const unsigned short*, const unsigned short*, void*);

// Mark completion only after both popup and dialog processors have drained their queues.
void ItemRequestState_UpdateCompletion(void* self) {
    if (ItemRequestState_ProcessPopupQueue(self)) {
        if (ItemRequestDialogs_ProcessQueue(self)) {
            AsItemRequestState80300EC8(self)->complete = 1;
        }
    }
}

// Return the stored completion flag; this call does not poll either processor.
int ItemRequestState_IsComplete(void* self) {
    return AsItemRequestState80300EC8(self)->complete;
}

// Return the combined pending reward-request and text-entry counts.
int ItemRequestState_GetEntryCount(void* self) {
    return AsItemRequestState80300EC8(self)->pendingRequestCount + AsItemRequestState80300EC8(self)->textEntryCount;
}

}

struct VObjDtor { 
virtual void v0(int); virtual void v1(); virtual void v2();
};
struct Arr80301294 {
    char* data;
    int count;
};
// Owned text-entry vector prefix: data points at count live 0x14-byte records.
// Each record is {opaque word, string flags, length, UTF-16 pointer, opaque word}.
// Bit 31 of flags owns the allocation at +0xC; inline strings must not be freed.
// D0..D3 below preserve the compiler's nested library-destructor expansion, not four resources.

// Destroy text entries in reverse order and free their owned vector allocation.
static inline void Arr80301294_Free(Arr80301294* a) {
    if (a->data != 0) {
        int count = a->count;
        char* item = a->data + count * 0x14;
        a->count = count;
        a->count -= count;
        while (count != 0) {
            item -= 0x14;
            if (item != 0 && item + 4 != 0 && (*(unsigned int*)(item + 4) >> 31) != 0)
                appFreeUni__FPv(*(void**)(item + 0xC));
            --count;
        }
        appFreeUni__FPv(a->data);
    }
}

// Null-checked destructor layer forwarding to owned text-vector cleanup.
static inline void Arr80301294_D3(Arr80301294* a) {
    if (a != 0) Arr80301294_Free(a);
}

// Preserve the next null-checked library destructor layer.
static inline void Arr80301294_D2(Arr80301294* a) {
    if (a != 0) Arr80301294_D3(a);
}

// Forward the text-vector destructor without releasing the enclosing object.
static inline void Arr80301294_D1(Arr80301294* a) {
    if (a != 0) Arr80301294_D2(a);
}

// Enter the complete text-vector member destructor chain.
static inline void Arr80301294_D0(Arr80301294* a) {
    if (a != 0) Arr80301294_D1(a);
}

struct Arr8_80301294 {
    void* data;
    int count;
};

// Clear the trivial request records and free the vector storage (no per-entry allocations).
static inline void Arr8_80301294_Free(Arr8_80301294* a) {
    if (a->data != 0) {
        int count = a->count;
        a->count = count;
        a->count -= count;
        appFreeUni__FPv(a->data);
    }
}

// Null-checked cleanup of the request-vector prefix.
static inline void Arr8_80301294_D3(Arr8_80301294* a) {
    if (a != 0) Arr8_80301294_Free(a);
}

// Preserve the second request-vector library destructor layer.
static inline void Arr8_80301294_D2(Arr8_80301294* a) {
    if (a != 0) Arr8_80301294_D3(a);
}

// Forward request-vector cleanup without deleting the enclosing object.
static inline void Arr8_80301294_D1(Arr8_80301294* a) {
    if (a != 0) Arr8_80301294_D2(a);
}

// Enter the complete request-vector member destructor chain.
static inline void Arr8_80301294_D0(Arr8_80301294* a) {
    if (a != 0) Arr8_80301294_D1(a);
}

extern "C" {

// Destroy an owned text vector; positive shouldDelete also frees self; return the original address.
void* ItemRequestTextVector_Destroy(void* self, int shouldDelete) {
    if (self != 0) {
        Arr80301294_D1((Arr80301294*)self);
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

#pragma push
#pragma opt_strength_reduction off
// Delete both owned processors and vectors, optionally free self, and return its original address.
void* ItemRequestState_Dtor(void* self, int shouldDelete) {
    char* object = (char*)self;
    if (self != 0) {
        void* first = AsItemRequestState80300EC8(object)->primaryProcessor;
        if (first != 0) {
            if (first != 0) ((VObjDtor*)first)->v0(1);
            AsItemRequestState80300EC8(object)->primaryProcessor = 0;
        }
        void* second = AsItemRequestState80300EC8(object)->secondaryProcessor;
        if (second != 0) {
            if (second != 0) ((VObjDtor*)second)->v0(1);
            AsItemRequestState80300EC8(object)->secondaryProcessor = 0;
        }
        Arr80301294_D0((Arr80301294*)(object + 0x14));
        Arr8_80301294_D0((Arr8_80301294*)(object + 8));
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Append a copied 12-byte request unless its kind-specific save test already succeeds.
// Order: opaque request word, packed item type, selector, part/index; selectors truncate to u16
// in the queued payload (and to u8 for save tests except TestPartFlag). Unknown kinds still queue.
// type bits 31..26 select the kind; lower bits and the first word are forwarded unchanged.
enum RequestItemKind {
    kRequestCharacter = 1, kRequestPart = 2, kRequestClubSet = 4, kRequestBall = 5,
    kRequestCaddie = 7, kRequestCaddieItem = 8, kRequestSetItemParts = 9, kRequestCourse = 10
};
// Filter by the saved item-kind flag, then copy the packed request into the pending vector.
void ItemRequestState_QueueRequestIfNeeded(void* self, int requestWord, unsigned int type, unsigned int selector, unsigned int partOrIndex) {
    // 0x803015E8: skip requests whose saved flag/unlock test already succeeds; queue the rest.
    unsigned int kind = type >> 26;
    switch (kind) {
    case kRequestPart:
        if (SaveRewards_TestPartFlag(self, selector, partOrIndex) != 0) return;
        break;
    case kRequestSetItemParts:
        if (SaveData_TestAllSetItemPartFlags(self, (unsigned char)selector, partOrIndex) != 0) return;
        break;
    case kRequestClubSet:
        if (SaveRewards_TestClubSetFlag(self, (unsigned char)selector, partOrIndex) != 0) return;
        break;
    case kRequestBall:
        if (SaveRewards_TestBallFlag(self, (unsigned char)selector, partOrIndex) != 0) return;
        break;
    case kRequestCharacter:
        if (SaveRewards_TestCharacterFlag(self, (unsigned char)selector, partOrIndex) != 0) return;
        break;
    case kRequestCaddie:
        if (SaveRewards_TestCaddieFlag(self, (unsigned char)selector, partOrIndex) != 0) return;
        break;
    case kRequestCaddieItem:
        if (SaveRewards_TestCaddieItemFlag(self, (unsigned char)selector, partOrIndex) != 0) return;
        break;
    case kRequestCourse:
        if (SaveRewards_IsCourseUnlocked(self, (unsigned char)selector) != 0) return;
        break;
    case 0: case 3: case 6:
    default:
        break;
    }
    AsItemRequestState80300EC8(self)->complete = 0;
    int data[3];
    // Stack payload is copied, never retained: +0 request word, +4 type, +8/+10 u16 selectors.
    data[0] = requestWord;
    data[1] = type;
    *(unsigned short*)((char*)data + 8) = (unsigned short)selector;
    *(unsigned short*)((char*)data + 10) = (unsigned short)partOrIndex;
    ItemRequestVector_AppendCopy(&AsItemRequestState80300EC8(self)->pendingRequests, data);
    return;
}

struct VObj_80301500 { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4(); virtual void v5();
};

// Draw the primary processor when present, otherwise the secondary, while dispatch is enabled.
void TourItemRequest_DispatchDisplay(void* self) {
    char* object = (char*)self;
    if (AsItemRequestState80300EC8(object)->dispatchEnabled == 0) return;
    void* first = AsItemRequestState80300EC8(object)->primaryProcessor;
    if (first != 0) ((VObj_80301500*)first)->v2();
    else {
        void* second = AsItemRequestState80300EC8(object)->secondaryProcessor;
        if (second != 0) ((VObj_80301500*)second)->v3();
    }
}

// Free a trivial request-vector allocation and optionally its wrapper; return the original address.
void* ItemRequestRecordVector_Destroy(void* self, int shouldDelete) {
    if (self != 0) {
        void* first = *(void**)self;
        if (first != 0) {
            int __v = *(int*)((char*)self + 4);
            *(int*)((char*)self + 4) = __v;
            *(int*)((char*)self + 4) -= __v;
            appFreeUni__FPv(first);
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

} // extern C

// Construct a copied entry in vector-provided storage.
inline void* operator new(new_size_t, void* p) { return p; }
extern "C" void __throw(void*, void*, void*) __attribute__((noreturn));
// Allocator tag constructors have no payload semantics; retain their original byte initialization.
struct EmptyAllocator { char value; /* Initialize the empty vector allocator tag. */ EmptyAllocator() : value(0) {} };
struct StringAllocator { char value; /* Preserve the empty string-allocator constructor. */ StringAllocator() {} };

// Queue payload layout: a = opaque request word, b = packed type (kind in high 6 bits),
// c = two u16 selectors at +8/+10 in target byte order. No entry owns extra storage.
struct Entry12 {
    int a, b, c;
    // Copy the three payload words through the original exception-aware construction path.
    Entry12(const Entry12& x) {
        int first = x.a;
        int second = x.b;
        try { a = first; b = second; c = x.c; }
        catch (...) { __throw(0, 0, 0); }
    }
};
// Text payload: a/last are forwarded opaque words; their consumer meaning is unresolved.
// The middle 12 bytes form an inline/heap UTF-16 string, with flags bit 31 marking ownership.
struct Entry20 {
    int a;
    unsigned int flags;
    int length;
    unsigned short* text;
    int last;
    // Copy inline text directly; deep-copy heap text so each vector entry owns its allocation.
    Entry20(const Entry20& x, const StringAllocator& dummy = StringAllocator()) {
        try {
            a = x.a;
            if ((x.flags >> 31) == 0) {
                unsigned int f = x.flags;
                int n = x.length;
                flags = f; length = n; text = x.text;
            } else {
                flags = 0; length = 0; text = 0;
                WString_ReserveCapacity(&flags, x.length);
                StringAllocator copy(dummy);
                WString_ReplaceRange(&flags, 0, 0, x.text, x.text + x.length, &copy);
            }
            last = x.last;
        } catch (...) { __throw(0, 0, 0); }
    }
};
struct Buffer12 {
    Entry12* data;
    unsigned int count, capacity;
    // Create an empty owned vector of trivial request records.
    Buffer12() : data(0), count(0), capacity(0) {}
    // Free request storage; trivial records require no individual destruction.
    ~Buffer12() {
        EmptyAllocator alloc;
        if (data) { unsigned int n = count; count = n; count -= n; EmptyAllocator copy; appFreeUni__FPv(data); }
    }
    // Exchange allocation ownership, count and capacity without copying payloads.
    void swap(Buffer12& x) {
        Entry12* p = data; Entry12* q = x.data; x.data = p;
        unsigned int c = capacity; capacity = x.capacity; x.capacity = c;
        data = q;
        unsigned int n = count; count = x.count; x.count = n;
    }
    // Grow to n entries using a temporary that frees the old storage after the swap.
    void reserve(unsigned int n) {
        if (capacity < n) {
            Buffer12 temp;
            EmptyAllocator alloc;
            temp.data = (Entry12*)appMallocDummy__FUl(n * sizeof(Entry12));
            temp.capacity = n;
            Entry12* src = data;
            Entry12* end = src + count;
            Entry12* dst = temp.data + temp.count;
            while (src < end) {
                new (dst) Entry12(*src);
                ++src; ++dst; ++temp.count;
            }
            count = 0;
            swap(temp);
        }
    }
};
struct Buffer20 {
    Entry20* data;
    unsigned int count, capacity;
    // Create an empty owned text-entry vector.
    Buffer20() : data(0), count(0), capacity(0) {}
    // Destroy owned strings in reverse order while keeping vector capacity available for reuse.
    void clear() {
        int n = count;
        Entry20* item = data + n;
        count -= n;
        while (n != 0) {
            --item;
            if (item && &item->flags && (item->flags >> 31)) appFreeUni__FPv(item->text);
            --n;
        }
    }
    // Destroy text entries, then free their containing allocation.
    ~Buffer20() { if (data) { clear(); appFreeUni__FPv(data); } }
    // Exchange both vectors' allocation ownership and live ranges.
    void swap(Buffer20& x) {
        Entry20* p = data; Entry20* q = x.data; x.data = p;
        unsigned int c = capacity; capacity = x.capacity; x.capacity = c;
        data = q;
        unsigned int n = count; count = x.count; x.count = n;
    }
    // Deep-copy live text into n slots, then let the temporary destroy the previous entries.
    void reserve(unsigned int n) {
        if (capacity < n) {
            Buffer20 temp;
            EmptyAllocator alloc;
            temp.data = (Entry20*)appMallocDummy__FUl(n * sizeof(Entry20));
            temp.capacity = n;
            Entry20* src = data;
            Entry20* end = src + count;
            Entry20* dst = temp.data + temp.count;
            while (src < end) {
                new (dst) Entry20(*src);
                ++src; ++dst; ++temp.count;
            }
            swap(temp);
        }
    }
};
extern "C" {
#pragma push
#pragma use_lmw_stmw on
// Initialize caller-owned state and reserve 16 entries per vector; unwind both vectors on failure.
void* ItemRequestState_Ctor(void* self) {
    char* o = (char*)self;
    Buffer12* first = (Buffer12*)&AsItemRequestState80300EC8(o)->pendingRequests;
    Buffer20* second = (Buffer20*)&AsItemRequestState80300EC8(o)->textEntries;
    first->data = 0; first->count = 0; first->capacity = 0;
    second->data = 0; second->count = 0; second->capacity = 0;
    AsItemRequestState80300EC8(o)->complete = 1;
    AsItemRequestState80300EC8(o)->dispatchEnabled = 0;
    *(int*)&AsItemRequestState80300EC8(o)->primaryProcessor = 0;
    *(int*)&AsItemRequestState80300EC8(o)->secondaryProcessor = 0;
    try {
        first->reserve(16);
        second->reserve(16);
    } catch (...) {
        second->~Buffer20();
        first->~Buffer12();
        throw;
    }
    // Finish with empty live ranges while retaining both reserved allocations.
    int count = second->count;
    int previous = first->count;
    first->count = previous;
    first->count -= previous;
    Entry20* item = second->data + count;
    second->count = count;
    second->count -= count;
    while (count != 0) {
        --item;
        if (item && &item->flags && (item->flags >> 31)) appFreeUni__FPv(item->text);
        --count;
    }
    return self;
}
#pragma pop

// Clear both live ranges and free heap strings, retaining vector allocations and processor state.
void ItemRequestState_ClearEntries(void* self) {
    char* item;
    int count = AsItemRequestState80300EC8(self)->textEntryCount;
    int previous = AsItemRequestState80300EC8(self)->pendingRequestCount;
    AsItemRequestState80300EC8(self)->pendingRequestCount = previous;
    AsItemRequestState80300EC8(self)->pendingRequestCount -= previous;
    char* base = *(char**)&AsItemRequestState80300EC8(self)->textEntries;
    item = base + count * 0x14;
    AsItemRequestState80300EC8(self)->textEntryCount = count;
    AsItemRequestState80300EC8(self)->textEntryCount -= count;
    while (count != 0) {
        item -= 0x14;
        if (item != 0 && item + 4 != 0 && (*(unsigned int*)(item + 4) >> 31) != 0)
            appFreeUni__FPv(*(void**)(item + 0xC));
        --count;
    }
}
} // extern "C"
