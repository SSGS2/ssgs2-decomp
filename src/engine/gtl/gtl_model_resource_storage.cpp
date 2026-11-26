// Gtl model-resource storage: initialize an empty model header and destroy its
// six-vector storage subobject. Clone in gtl_generic_modeling_resource_clone.cpp allocates a 0x54-byte model;
// gtl_generic_model_resource.cpp fills it; gtl_generic_modeling_resource_dtor.cpp releases graphics handles and the retained parent.
// This file's gtl_ModelHandleVectorBlock_DestroyStorage takes the vector block at model+0x0C, NOT that header.
// Mod map: vector ordering/layout below must agree with those three consumers.
// No gameplay tuning constants. s_genericResourceAllocatorFunctions is the resource storage allocator
// also used by vector resize in gtl_graphics_resource_vectors.cpp. Preserve exported ABI/address order.

// PPC32, four-byte alignment: each vector owns an allocation of handle slots.
// size/capacity count slots, not bytes. Pointee release is a separate operation.
struct ModelHandleVectorStorage {
    void* allocation;
    unsigned int size;
    unsigned int capacity;
};
struct ModelHandleVectorBlock {
    ModelHandleVectorStorage textures;       // +00 (model +0C)
    ModelHandleVectorStorage palettes;       // +0C (model +18)
    ModelHandleVectorStorage vertexBuffers;  // +18 (model +24)
    ModelHandleVectorStorage declarations;   // +24 (model +30)
    ModelHandleVectorStorage indexBuffers;   // +30 (model +3C)
    ModelHandleVectorStorage vertexBuffers2; // +3C (model +48)
};
typedef char ModelHandleVectorBlock_size[(sizeof(ModelHandleVectorBlock) == 0x48) ? 1 : -1];

#include "classes/GtlIGenericModelingResource.h"

extern "C" {
typedef struct DtorTable {
    void* unk0;
    void (*unk4)(void*, struct DtorTable*);
} DtorTable;
extern DtorTable s_genericResourceAllocatorFunctions;
void __dl__FPv(void* p);
extern char __vt__Q23gtl24IGenericModelingResource[];
}

// Initialize uninitialized storage of an empty model: acquire no resources. All vectors become {null,0,0},
// the parent is null, and the reference count is one. Calling this on a populated object overwrites ownership
// without releasing anything.
#pragma opt_propagation off
void gtl::IGenericModelingResource::InitEmpty() {
    refCount = 1;
    backingResource = 0;
    vtable = __vt__Q23gtl24IGenericModelingResource;
    textures.data = 0;
    textures.size = 0;
    textures.capacity = 0;
    palettes.data = 0;
    palettes.size = 0;
    palettes.capacity = 0;
    vertexBuffers.data = 0;
    vertexBuffers.size = 0;
    vertexBuffers.capacity = 0;
    vertexDecls.data = 0;
    vertexDecls.size = 0;
    vertexDecls.capacity = 0;
    indexBuffers.data = 0;
    indexBuffers.size = 0;
    indexBuffers.capacity = 0;
    vertexBuffers2.data = 0;
    vertexBuffers2.size = 0;
    vertexBuffers2.capacity = 0;
}
#pragma opt_propagation reset

extern "C" {
#pragma opt_propagation off
// Free vector slot storage only; caller has already released graphics handles.
// The five address checks retain the nested container/base cleanup shape with
// propagation disabled. They test the subobject address, NOT allocation success;
// normal valid enclosing objects cannot have a null member address.
// Keep size -= size before the callback. Capacity/pointer stay stale: terminal
// destruction, not reusable clear. Raw byte-address comparisons and separately
// cast loads are required: using &typedMember coalesces their address registers
// and loses the original-exact destructor. Offsets below derive from the schema;
// the macro may only be used with a constant offset and a borrowed char* self.
enum ModelVectorOffset {
    kTexturesOffset = (int)&((ModelHandleVectorBlock*)0)->textures,
    kPalettesOffset = (int)&((ModelHandleVectorBlock*)0)->palettes,
    kVerticesOffset = (int)&((ModelHandleVectorBlock*)0)->vertexBuffers,
    kDeclarationsOffset = (int)&((ModelHandleVectorBlock*)0)->declarations,
    kIndicesOffset = (int)&((ModelHandleVectorBlock*)0)->indexBuffers,
    kVertices2Offset = (int)&((ModelHandleVectorBlock*)0)->vertexBuffers2
};
#define RELEASE_MODEL_VECTOR(offset)                                                                         \
    do {                                                                                                     \
        if (self + (offset) == 0)                                                                            \
            break;                                                                                           \
        if (self + (offset) == 0)                                                                            \
            break;                                                                                           \
        if (self + (offset) == 0)                                                                            \
            break;                                                                                           \
        if (self + (offset) == 0)                                                                            \
            break;                                                                                           \
        if (self + (offset) == 0)                                                                            \
            break;                                                                                           \
        void* allocation = *(void**)(self + (offset));                                                       \
        if (allocation == 0)                                                                                 \
            break;                                                                                           \
        DtorTable* allocator = &s_genericResourceAllocatorFunctions;                                         \
        *(int*)(self + (offset) + 4) = *(unsigned int*)(self + (offset) + 4) - *(int*)(self + (offset) + 4); \
        allocator->unk4(allocation, allocator);                                                              \
    } while (0)

// Destroy nullable vector-block self in reverse order (model+48 down to +0C).
// No model header/vtable/parent is touched. Each allocation goes to s_genericResourceAllocatorFunctions;
// handles inside are NOT released here. flags>0 also operator-deletes this exact
// self address: embedded blocks must use flags<=0. Return the original address
// (possibly freed). No failure status/rollback; callbacks must complete normally.
void* gtl_ModelHandleVectorBlock_DestroyStorage(char* self, int flags) {
    if (self) {
        RELEASE_MODEL_VECTOR(kVertices2Offset);
        RELEASE_MODEL_VECTOR(kIndicesOffset);
        RELEASE_MODEL_VECTOR(kDeclarationsOffset);
        RELEASE_MODEL_VECTOR(kVerticesOffset);
        RELEASE_MODEL_VECTOR(kPalettesOffset);
        RELEASE_MODEL_VECTOR(kTexturesOffset);
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_propagation reset
#undef RELEASE_MODEL_VECTOR
} // extern "C"
