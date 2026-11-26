// Graphics resource slot tables: resizing and slot access for the vertex buffer, index buffer, texture
// and palette tables, plus the shared handle that adds a reference to a resource.
// Each table is a vector of pointer slots; the objects those slots name are not owned here.

#include <decomp/ct_member.h>
// gtl graphics-resource tables: resize pointer-slot arrays for vertex buffers,
// index buffers, textures and palettes. This unit manages table STORAGE, not
// the lifetime of objects named by its slots; it exposes no gameplay tuning.
// Mod lookup: each *_Resize controls its table's live slot count; *_GetSourceSlot
// and *_GetDestinationSlot borrow the same unchecked slot; AddRefToHandle below
// retains a separate packed handle. Allocator callbacks are s_genericResourceAllocatorFunctions;
// exception message/type pool is s_lengthErrorInfo. gtl_vertex_declaration_vector.cpp has the sibling resize.

#include "types.h"
#include "game/stdexcept_local.h"
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
// Construct into reserved slot storage; storage remains owned by its vector.
inline void* operator new(size_t, void* p) {
    return p;
}

#pragma use_lmw_stmw on

extern "C" {

extern char __vt__Q23std11logic_error[], __vt__Q23std12length_error[], s_sharedCharDeleterVtable[], s_lengthErrorInfo[];
struct GtlVertexAllocator {
    void* (*allocate)(unsigned long);
    void (*deallocate)(void*);
};
extern GtlVertexAllocator s_genericResourceAllocatorFunctions;
void* __nwa__FUl(unsigned long);
void __dla__FPv(void*);
void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q23std12length_errorFv(void*, int);

// Common Wii layout: {owned allocation, live slot count, allocated slot count}.
// Each slot is a four-byte pointer/handle bit pattern. Copying/transferring slots
// does not AddRef/Release. Shrink merely lowers size: removed slots are neither
// cleared nor disposed, so callers must release payloads beforehand if required.
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
    // Stage a replacement allocation; offset marks the untransferred old prefix.
    GtlVertexBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Roll back replacement storage, or free old storage after commit swaps it in.
    // Never dispose pointees: this temporary owns only the pointer-slot allocation.
    ~GtlVertexBuffer() {
        if (data) {
            size = 0;
            s_genericResourceAllocatorFunctions.deallocate(data);
        }
    }
};

struct GtlVertexErrorBase {
    void* table;
    // Establish the base exception before shared-message construction.
    GtlVertexErrorBase() {
        table = __vt__Q23std11logic_error;
    }
    // Run runtime base cleanup during normal destruction/unwinding.
    ~GtlVertexErrorBase() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};

struct GtlVertexAllocation {
    char* pointer;
    char* allocator;
    // Guard new[] message bytes until the shared-control block takes ownership.
    GtlVertexAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free untransferred message bytes if control-block construction throws.
    ~GtlVertexAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};

struct GtlVertexControl {
    void* table;
    unsigned strong;
    unsigned weak;
    char* pointer;
    // Own message bytes with one strong and one weak shared-control reference.
    GtlVertexControl(char* p) {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
        pointer = p;
    }
};

struct GtlFlag {
    char value;
    char pad[3];
};

struct GtlLengthError : GtlVertexErrorBase {
    char* text;
    void* control;
    // Copy a required NUL-terminated message; the guard covers control allocation.
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

// Select geometric slack or minimum additional capacity without copying operands.
static inline const unsigned& GtlVertexMax(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}

// Exchange vector ownership words during the nonthrowing publication phase.
template <class T> static inline void GtlVertexSwap(T& a, T& b) {
    T t = a;
    a = b;
    b = t;
}

union GtlLengthStorage {
    int alignment;
    unsigned char bytes[sizeof(GtlLengthError)];
};

// Construct in aligned stack storage and transfer the exception to __throw.
static inline void GtlVertexThrowLengthAt(GtlLengthStorage& storage, GtlFlag& flag, const char* message) {
    GtlLengthError* error = (GtlLengthError*)storage.bytes;
    error->__ct(&flag.value, message);
    __throw(s_lengthErrorInfo + 0x14, error, (void*)__dt__Q23std12length_errorFv);
}

// Throw the table allocator's shared length-error message/type pair.
static inline void GtlVertexThrowLength(GtlLengthStorage& storage, GtlFlag& flag) {
    const char* message = s_lengthErrorInfo;
    GtlVertexThrowLengthAt(storage, flag, message);
}

// Preserve the preflight throw's separate inline/cleanup scope.
static inline void GtlVertexThrowLength1(GtlLengthStorage& storage, GtlFlag& flag) {
    GtlVertexThrowLength(storage, flag);
}

// Bound additional slots so the four-byte allocation cannot overflow u32 bytes.
static inline void GtlVertexCheckLength(unsigned need, unsigned cap, GtlLengthStorage& storage, GtlFlag& flag) {
    if (need > 0x3FFFFFFFu - cap)
        GtlVertexThrowLength(storage, flag);
}

// Repeat the bound check inside the replacement guard's cleanup scope.
static inline void GtlVertexCheckLength2(unsigned need, unsigned cap, GtlLengthStorage& storage, GtlFlag& flag) {
    GtlVertexCheckLength(need, cap, storage, flag);
}

// Allocate cap pointer slots through the installed allocator; no null check follows.
static inline void** GtlVertexAllocate(unsigned cap, GtlLengthStorage& storage, GtlFlag& flag) {
    if (cap > 0x3FFFFFFFu)
        GtlVertexThrowLength(storage, flag);
    return (void**)s_genericResourceAllocatorFunctions.allocate(cap * 4);
}

extern char s_lengthErrorInfo[];
void* memset(void*, int, unsigned long);
void* memcpy(void*, const void*, unsigned long);

struct GraphicsResourceHandle {
    CW_BITS2(unsigned int refCount : 16, unsigned int index : 16)
};

// Write borrowed self to required out, then retain it by incrementing the high
// 16-bit refcount; return status 1. self must name a live handle and out must not
// overlap its four-byte packed word. index occupies the low 16 bits (Wii layout).
// No allocation/locking/overflow check: 65535 wraps to zero. Caller must ensure
// a retain cannot overflow and balance it through the resource owner's release
// path; e.g. gtl::IGenericModelingResource::SetResource (gtl_generic_modeling_resource_part2.cpp) releases replaced
// type-3/5 entries by decrementing this high halfword and invoking the concrete
// destructor at zero. Replacing vector slots directly bypasses that release.
// This routine supplies no destructor and does not extend backing storage
// independently of that owner. Zero is not a safe resurrection count.
int gtl_GraphicsResource_AddRefToHandle(GraphicsResourceHandle* self, GraphicsResourceHandle** out) {
    *out = self;
    self->refCount++;
    return 1;
}

struct VertexBufferVector {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Resize the vertex-buffer slot table to n; see the common resize contract below.
// All four expanded bodies intentionally keep identical policy and cleanup scopes.
// COMMON RESIZE CONTRACT: required self, size <= capacity, and data covers capacity
// slots. n counts slots, max 0x3FFFFFFF. Slots own no pointees here. Growth zeros
// new slots, preserves existing values and may invalidate every borrowed slot
// address. Shrink only reduces size; callers dispose removed resources separately.
// Failure: length_error or allocator exception before publication leaves self
// unchanged. Allocator must return usable storage or throw, not null. No locking.
// PHASES in every body: (1) fit/shrink or preflight bound; (2) guarded replacement
// allocation; (3) zero new tail, copy old prefix and clear moved-from slots;
// (4) swap data/capacity/size, guard frees old allocation. After transfer begins
// only memset/memcpy/scalar operations remain, so publication cannot throw.
// Duplicate volatile preflight loads and throw wrappers preserve actual reads
// and exception scopes; the unused `dead` result is not a second allocation.
void gtl_VertexBufferVector_Resize(GtlVertexDeclVector* self, unsigned n) {
    unsigned add;
    unsigned grow, half, needed;
    GtlLengthStorage storage1;
    GtlFlag flag1, flag3, flag2;
    unsigned size = self->size;
    // Phase 1: extend in place when possible; otherwise validate growth bounds.
    if (n > size) {
        add = n - size;
        if (add > self->capacity || size > self->capacity - add) {
            volatile GtlVertexDeclVector* v = self;
            unsigned need0 = v->size + add - self->capacity;
            unsigned cap0 = v->capacity;
            unsigned dead = cap0;
            if (need0 > 0x3FFFFFFFu - cap0)
                GtlVertexThrowLength1(storage1, flag1);
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
        // Phase 2: guarded replacement allocation, then zero-fill new live tail.
        temp.data = GtlVertexAllocate(cap, storage3, flag3);
        temp.capacity = cap;
        unsigned sz = self->size;
        temp.offset = sz;
        memset(temp.data + sz + temp.size, 0, add * 4);

        int count;
        void** oldData = self->data;
        temp.size += add;
        count = ((char*)(oldData + self->size) - (char*)oldData) / 4;
        temp.offset -= count;
        int bytes = count * 4;
        // Phase 3: transfer old slot bits; clearing old slots transfers no refs.
        memcpy((char*)temp.data + temp.offset * 4, oldData, bytes);
        memset(oldData, 0, bytes);

        unsigned newSize;
        unsigned oldCap;
        void** newData;
        void** oldD;
        temp.size += count;
        self->size = 0;
        // Phase 4: publish all three ownership words; temp now frees old storage.
        GtlVertexSwap(self->capacity, temp.capacity);
        GtlVertexSwap(self->data, temp.data);
        GtlVertexSwap(self->size, temp.size);
        temp.size = 0;
    } else if (n < size) {
        // Shrink discards logical slots only; no Release/delete/clear of payloads.
        unsigned diff = size - n;
        self->size -= diff;
    }
}
#pragma pop

// Borrow slot index (index < size); writable storage, valid only until resize/free.
unsigned int* gtl_VertexBufferVector_GetSourceSlot(VertexBufferVector* self, unsigned int index) {
    return self->data + index;
}

// Same slot as GetSourceSlot; no separate destination allocation or ownership.
unsigned int* gtl_VertexBufferVector_GetDestinationSlot(VertexBufferVector* self, unsigned int index) {
    return self->data + index;
}

struct IndexBufferVector {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Resize index-buffer slots; common resize contract/phases at VertexBuffer_Resize.
void gtl_IndexBufferVector_Resize(GtlVertexDeclVector* self, unsigned n) {
    unsigned add;
    unsigned grow, half, needed;
    GtlLengthStorage storage1;
    GtlFlag flag1, flag3, flag2;
    unsigned size = self->size;
    // Phase 1: extend in place when possible; otherwise validate growth bounds.
    if (n > size) {
        add = n - size;
        if (add > self->capacity || size > self->capacity - add) {
            volatile GtlVertexDeclVector* v = self;
            unsigned need0 = v->size + add - self->capacity;
            unsigned cap0 = v->capacity;
            unsigned dead = cap0;
            if (need0 > 0x3FFFFFFFu - cap0)
                GtlVertexThrowLength1(storage1, flag1);
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
        // Phase 2: guarded replacement allocation, then zero-fill new live tail.
        temp.data = GtlVertexAllocate(cap, storage3, flag3);
        temp.capacity = cap;
        unsigned sz = self->size;
        temp.offset = sz;
        memset(temp.data + sz + temp.size, 0, add * 4);

        int count;
        void** oldData = self->data;
        temp.size += add;
        count = ((char*)(oldData + self->size) - (char*)oldData) / 4;
        temp.offset -= count;
        int bytes = count * 4;
        // Phase 3: transfer old slot bits; clearing old slots transfers no refs.
        memcpy((char*)temp.data + temp.offset * 4, oldData, bytes);
        memset(oldData, 0, bytes);

        unsigned newSize;
        unsigned oldCap;
        void** newData;
        void** oldD;
        temp.size += count;
        self->size = 0;
        // Phase 4: publish all three ownership words; temp now frees old storage.
        GtlVertexSwap(self->capacity, temp.capacity);
        GtlVertexSwap(self->data, temp.data);
        GtlVertexSwap(self->size, temp.size);
        temp.size = 0;
    } else if (n < size) {
        // Shrink discards logical slots only; no Release/delete/clear of payloads.
        unsigned diff = size - n;
        self->size -= diff;
    }
}
#pragma pop

// Borrow slot index (index < size); invalidated by reallocating resize/free.
unsigned int* gtl_IndexBufferVector_GetSourceSlot(IndexBufferVector* self, unsigned int index) {
    return self->data + index;
}

// Alias the same unchecked slot as GetSourceSlot, with no ownership transfer.
unsigned int* gtl_IndexBufferVector_GetDestinationSlot(IndexBufferVector* self, unsigned int index) {
    return self->data + index;
}

struct TextureVector {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Resize texture slots; common resize contract/phases at VertexBuffer_Resize.
void gtl_TextureVector_Resize(GtlVertexDeclVector* self, unsigned n) {
    unsigned add;
    unsigned grow, half, needed;
    GtlLengthStorage storage1;
    GtlFlag flag1, flag3, flag2;
    unsigned size = self->size;
    // Phase 1: extend in place when possible; otherwise validate growth bounds.
    if (n > size) {
        add = n - size;
        if (add > self->capacity || size > self->capacity - add) {
            volatile GtlVertexDeclVector* v = self;
            unsigned need0 = v->size + add - self->capacity;
            unsigned cap0 = v->capacity;
            unsigned dead = cap0;
            if (need0 > 0x3FFFFFFFu - cap0)
                GtlVertexThrowLength1(storage1, flag1);
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
        // Phase 2: guarded replacement allocation, then zero-fill new live tail.
        temp.data = GtlVertexAllocate(cap, storage3, flag3);
        temp.capacity = cap;
        unsigned sz = self->size;
        temp.offset = sz;
        memset(temp.data + sz + temp.size, 0, add * 4);

        int count;
        void** oldData = self->data;
        temp.size += add;
        count = ((char*)(oldData + self->size) - (char*)oldData) / 4;
        temp.offset -= count;
        int bytes = count * 4;
        // Phase 3: transfer old slot bits; clearing old slots transfers no refs.
        memcpy((char*)temp.data + temp.offset * 4, oldData, bytes);
        memset(oldData, 0, bytes);

        unsigned newSize;
        unsigned oldCap;
        void** newData;
        void** oldD;
        temp.size += count;
        self->size = 0;
        // Phase 4: publish all three ownership words; temp now frees old storage.
        GtlVertexSwap(self->capacity, temp.capacity);
        GtlVertexSwap(self->data, temp.data);
        GtlVertexSwap(self->size, temp.size);
        temp.size = 0;
    } else if (n < size) {
        // Shrink discards logical slots only; no Release/delete/clear of payloads.
        unsigned diff = size - n;
        self->size -= diff;
    }
}
#pragma pop

// Borrow slot index (index < size); invalidated by reallocating resize/free.
unsigned int* gtl_TextureVector_GetSourceSlot(TextureVector* self, unsigned int index) {
    return self->data + index;
}

// Alias the same unchecked slot as GetSourceSlot, with no ownership transfer.
unsigned int* gtl_TextureVector_GetDestinationSlot(TextureVector* self, unsigned int index) {
    return self->data + index;
}

struct TexturePaletteVector {
    unsigned int* data;
    unsigned int size;
    unsigned int capacity;
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Resize palette slots; common resize contract/phases at VertexBuffer_Resize.
void gtl_TexturePaletteVector_Resize(GtlVertexDeclVector* self, unsigned n) {
    unsigned add;
    unsigned grow, half, needed;
    GtlLengthStorage storage1;
    GtlFlag flag1, flag3, flag2;
    unsigned size = self->size;
    // Phase 1: extend in place when possible; otherwise validate growth bounds.
    if (n > size) {
        add = n - size;
        if (add > self->capacity || size > self->capacity - add) {
            volatile GtlVertexDeclVector* v = self;
            unsigned need0 = v->size + add - self->capacity;
            unsigned cap0 = v->capacity;
            unsigned dead = cap0;
            if (need0 > 0x3FFFFFFFu - cap0)
                GtlVertexThrowLength1(storage1, flag1);
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
        // Phase 2: guarded replacement allocation, then zero-fill new live tail.
        temp.data = GtlVertexAllocate(cap, storage3, flag3);
        temp.capacity = cap;
        unsigned sz = self->size;
        temp.offset = sz;
        memset(temp.data + sz + temp.size, 0, add * 4);

        int count;
        void** oldData = self->data;
        temp.size += add;
        count = ((char*)(oldData + self->size) - (char*)oldData) / 4;
        temp.offset -= count;
        int bytes = count * 4;
        // Phase 3: transfer old slot bits; clearing old slots transfers no refs.
        memcpy((char*)temp.data + temp.offset * 4, oldData, bytes);
        memset(oldData, 0, bytes);

        unsigned newSize;
        unsigned oldCap;
        void** newData;
        void** oldD;
        temp.size += count;
        self->size = 0;
        // Phase 4: publish all three ownership words; temp now frees old storage.
        GtlVertexSwap(self->capacity, temp.capacity);
        GtlVertexSwap(self->data, temp.data);
        GtlVertexSwap(self->size, temp.size);
        temp.size = 0;
    } else if (n < size) {
        // Shrink discards logical slots only; no Release/delete/clear of payloads.
        unsigned diff = size - n;
        self->size -= diff;
    }
}
#pragma pop

// Borrow slot index (index < size); invalidated by reallocating resize/free.
unsigned int* gtl_TexturePaletteVector_GetSourceSlot(TexturePaletteVector* self, unsigned int index) {
    return self->data + index;
}

// Alias the same unchecked slot as GetSourceSlot, with no ownership transfer.
unsigned int* gtl_TexturePaletteVector_GetDestinationSlot(TexturePaletteVector* self, unsigned int index) {
    return self->data + index;
}

} // extern "C"
