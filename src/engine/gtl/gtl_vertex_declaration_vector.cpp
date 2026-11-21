#include <decomp/ct_member.h>
// Model vertex-declaration pointer-slot storage, not vertex payload conversion.
// API/mod map: Resize changes logical slot count; GetSourceSlot/GetDestinationSlot
// return the same address for a caller to read/write. No gameplay tuning knobs.
// Schema (Wii, 32-bit, four-byte aligned): {data +0,size +4,capacity +8}; size and
// capacity count four-byte pointer slots. The vector owns data via s_genericResourceAllocatorFunctions
// allocator callbacks, but this unit neither retains nor destroys the pointees.
// Keep allocator/deallocator paired; a resize may invalidate EVERY borrowed slot
// address. Changing count alone does not construct or destroy declarations.
// Translation unit 0x803DB238-0x803DB750 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

// Placement construction borrows caller-provided exception storage.
inline void* operator new(size_t, void* p) { return p; }

extern "C" {

extern char __vt__Q23std11logic_error[], __vt__Q23std12length_error[], s_sharedCharDeleterVtable[], s_lengthErrorInfo[];
struct GtlVertexAllocator {
    void* (*allocate)(unsigned long);
    void (*deallocate)(void*);
};
extern GtlVertexAllocator s_genericResourceAllocatorFunctions;
void* memset(void*, int, unsigned long);
void* memcpy(void*, const void*, unsigned long);
void* __nwa__FUl(unsigned long);
void __dla__FPv(void*);
void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q23std12length_errorFv(void*, int);

struct GtlVertexDeclVector {
    void** data;
    unsigned size;
    unsigned capacity;
};

struct GtlVertexBuffer {
    void** data;
    unsigned size;
    unsigned capacity;
    unsigned* owner;
    unsigned offset;
    // Guard fresh slot storage until exchange; owner is a borrowed capacity word.
    GtlVertexBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // After exchange release old storage; before exchange release new storage.
    ~GtlVertexBuffer() { if (data) { size = 0; s_genericResourceAllocatorFunctions.deallocate(data); } }
};

struct GtlVertexErrorBase {
    void* table;
    // Install the standard logic-error base before copying the message.
    GtlVertexErrorBase() { table = __vt__Q23std11logic_error; }
    // Tear down only the exception base, not the enclosing stack storage.
    ~GtlVertexErrorBase() { __dt__Q23std9exceptionFv(this, 0); }
};

struct GtlVertexAllocation {
    char* pointer;
    char* allocator;
    // Guard diagnostic characters while their shared-control allocation can throw.
    GtlVertexAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Release characters unless ownership was committed to the control block.
    ~GtlVertexAllocation() { if (pointer) __dla__FPv(pointer); }
};

struct GtlVertexControl {
    void* table;
    unsigned strong;
    unsigned weak;
    char* pointer;
    // Own diagnostic characters with one strong/weak control reference.
    GtlVertexControl(char* p) { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; pointer = p; }
};

struct GtlFlag { char value; char pad[3]; };

struct GtlLengthError : GtlVertexErrorBase {
    char* text;
    void* control;
    // Copy required NUL-terminated text; character/control guards unwind on throw.
    GtlLengthError(char* allocatorFlag, const char* message) {
        *allocatorFlag = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            GtlVertexAllocation allocation(p, allocatorFlag);
            control = new GtlVertexControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(GtlLengthError)
};

// Borrow the larger growth amount through the end of the immediate expression.
static inline const unsigned& GtlVertexMax(const unsigned& a, const unsigned& b) { return a < b ? b : a; }

// Exchange one vector ownership/count word without touching its pointees.
template <class T> static inline void GtlVertexSwap(T& a, T& b) { T t = a; a = b; b = t; }

union GtlLengthStorage { int alignment; unsigned char bytes[sizeof(GtlLengthError)]; };

// Construct length_error in aligned storage and transfer it to runtime unwinding.
static inline void GtlVertexThrowLengthAt(GtlLengthStorage& storage, GtlFlag& flag, const char* message) {
    GtlLengthError* error = (GtlLengthError*)storage.bytes;
    error->__ct(&flag.value, message);
    __throw(s_lengthErrorInfo + 0x14, error, (void*)__dt__Q23std12length_errorFv);
}

// Select the shared vector length diagnostic/type pool (s_lengthErrorInfo).
static inline void GtlVertexThrowLength(GtlLengthStorage& storage, GtlFlag& flag) {
    const char* message = s_lengthErrorInfo;
    GtlVertexThrowLengthAt(storage, flag, message);
}

// Preflight throw boundary; separate wrapper preserves inlined cleanup regions.
static inline void GtlVertexThrowLength1(GtlLengthStorage& storage, GtlFlag& flag) {
    GtlVertexThrowLength(storage, flag);
}

// Reject capacity addition beyond the four-byte allocation count limit.
static inline void GtlVertexCheckLength(unsigned need, unsigned cap, GtlLengthStorage& storage, GtlFlag& flag) {
    if (need > 0x3FFFFFFFu - cap) GtlVertexThrowLength(storage, flag);
}

// Repeat validation at the allocation phase with its own exception storage.
static inline void GtlVertexCheckLength2(unsigned need, unsigned cap, GtlLengthStorage& storage, GtlFlag& flag) {
    GtlVertexCheckLength(need, cap, storage, flag);
}

// Allocate cap slots using the installed allocator; it must succeed or throw.
// A null allocator result is not checked before zero-fill/copy by the caller.
static inline void** GtlVertexAllocate(unsigned cap, GtlLengthStorage& storage, GtlFlag& flag) {
    if (cap > 0x3FFFFFFFu) GtlVertexThrowLength(storage, flag);
    return (void**)s_genericResourceAllocatorFunctions.allocate(cap * 4);
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Required live vector with size<=capacity, valid storage for capacity slots,
// and allocator callbacks. n is an element count in [0,0x3fffffff].
// Grow: preserve existing slot values, append nulls; shrink: only reduce size,
// leaving truncated words/pointees untouched. Equal size is a no-op.
// Allocation/length exceptions before exchange leave the published vector intact;
// allocator/deallocator and memcpy/memset must obey their ordinary contracts.
// Growth policy: below 0x15555555 add max(needed,(cap+1)*3/5), below 0x2aaaaaaa
// add max(needed,(cap+1)/2), otherwise saturate at 0x3fffffff (byte overflow cap).
// The duplicate preflight/reload is intentional: volatile reads, separate throw
// storage and RAII lifetimes preserve instruction/exception ordering. Follow
// phases below when editing rather than deleting apparently dead arithmetic.
void gtl_VertexDeclarationVector_Resize(GtlVertexDeclVector* self, unsigned n) {
    unsigned add;
    unsigned grow, half, needed;
    GtlLengthStorage storage1;
    GtlFlag flag1, flag3, flag2;
    unsigned size = self->size;
    if (n > size) {
        add = n - size;
        // Phase 1: choose in-place growth or validate required reallocation.
        if (add > self->capacity || size > self->capacity - add) {
            volatile GtlVertexDeclVector* v = self;
            unsigned need0 = v->size + add - self->capacity;
            unsigned cap0 = v->capacity;
            unsigned dead = cap0;
            if (need0 > 0x3FFFFFFFu - cap0) GtlVertexThrowLength1(storage1, flag1);
            if (cap0 < 0x15555555u) {
                unsigned grow = (cap0 + 1) * 3 / 5;
                dead += GtlVertexMax(grow, need0);
            } else if (cap0 < 0x2AAAAAAAu) {
                unsigned half = (cap0 + 1) / 2;
                dead += GtlVertexMax(half, need0);
            } else {
                dead = 0x3FFFFFFF;
            }
        } else {
            memset((char*)self->data + self->size * 4, 0, add * 4);
            self->size += add;
            return;
        }
        // Phase 2: reload counts, validate capacity again, allocate guarded storage.
        volatile GtlVertexDeclVector* v2 = self;
        unsigned size2 = v2->size;
        unsigned cap2 = v2->capacity;
        GtlLengthStorage storage3;
        GtlVertexBuffer temp(&self->capacity);
        GtlLengthStorage storage2;
        needed = size2 + add - cap2;
        unsigned cap = self->capacity;

        GtlVertexCheckLength2(needed, cap, storage2, flag2);
        if (cap < 0x15555555u) {
            grow = (cap + 1) * 3 / 5;
            cap += GtlVertexMax(grow, needed);
        } else if (cap < 0x2AAAAAAAu) {
            half = (cap + 1) / 2;
            cap += GtlVertexMax(half, needed);
        } else {
            cap = 0x3FFFFFFF;
        }
        temp.data = GtlVertexAllocate(cap, storage3, flag3);
        temp.capacity = cap;
        unsigned sz = self->size;
        temp.offset = sz;
        // Phase 3: prepare null tail, copy old slots, relinquish old pointer words.
        memset(temp.data + sz + temp.size, 0, add * 4);

        int count;
        void** oldData = self->data;
        temp.size += add;
        count = ((char*)(oldData + self->size) - (char*)oldData) / 4;
        temp.offset -= count;
        int bytes = count * 4;
        memcpy((char*)temp.data + temp.offset * 4, oldData, bytes);
        memset(oldData, 0, bytes);

        unsigned newSize;
        unsigned oldCap;
        void** newData;
        void** oldD;
        temp.size += count;
        // Phase 4: publish by exchange; temp now frees only old slot storage.
        self->size = 0;
        GtlVertexSwap(self->capacity, temp.capacity);
        GtlVertexSwap(self->data, temp.data);
        GtlVertexSwap(self->size, temp.size);
        temp.size = 0;
    } else if (n < size) {
        unsigned diff = size - n;
        self->size -= diff;
    }
}
#pragma pop

// Borrow a readable/writable slot, without bounds checks. For access require
// index<size; index==size only forms an end pointer when data is valid storage.
// A reallocation/destruction invalidates it; shrinking past index ends logical
// membership even if the address remains allocated. No pointee ownership transfer.
void** gtl_VertexDeclarationVector_GetSourceSlot(GtlVertexDeclVector* self, unsigned index) {
    return self->data + index;
}

// Same address/lifetime as GetSourceSlot, not allocation or automatic growth.
// Require index<size for writing a logical element; call Resize first to append.
void** gtl_VertexDeclarationVector_GetDestinationSlot(GtlVertexDeclVector* self, unsigned index) {
    return self->data + index;
}

} // extern "C"
