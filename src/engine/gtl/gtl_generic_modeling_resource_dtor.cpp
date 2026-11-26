// Model resource teardown: release graphics handles, their six owned pointer arrays, the retained resource
// buffer, then optionally the wrapper allocation. Population and release of the handle arrays are in
// gtl_GenericModel_LoadGraphicsResources and gtl_GenericModel_ReleaseGraphicsResources (gtl_generic_model_graphics_load.cpp).
// Allocation and free callbacks are the pair at s_genericResourceAllocatorFunctions.
#include "classes/GtlIGenericModelingResource.h"

struct RcBase {
    int rc;
};

struct GenericRefObj : RcBase {
    virtual void Release(int arg);
};

struct GenericFreeFn {
    int unk0;
    void (*fn)(void* p, GenericFreeFn* self);
};

extern "C" {

extern char __vt__Q23gtl24IGenericModelingResource[];
extern char __vt__Q23gtl20IGenericResourceFile[];
extern GenericFreeFn s_genericResourceAllocatorFunctions;

void gtl_GenericModel_ReleaseGraphicsResources(void* resources);

// Free one vector's array after graphics-handle release has made its size zero. The pointer is not nulled:
// the containing object is being destroyed. The five identical tests are nested inlined member/base
// destructor guards, all on this same vector; they do not indicate five buffers.
static inline void GenericVector_Free(GenericResourceVector* v) {
    if (v && v && v && v && v && v->data) {
        GenericFreeFn* f = &s_genericResourceAllocatorFunctions;
        int n = v->size;
        v->size -= n;
        f->fn(v->data, f);
    }
}
}

// Disabling propagation preserves the expanded destructor's repeated null guards
// and load/store/callback boundaries, including the member-address check below.
#pragma opt_propagation off
// Nullable this; flags>0 also frees the wrapper storage through s_genericResourceAllocatorFunctions.fn, flags<=0
// destroys members only (the caller still owns the raw storage). Returns the incoming address, null for null input.
// Never dereference the result after flags>0. The parent reference is decremented once, and Release(1) runs only
// at zero; the parent is then cleared.
void* gtl::IGenericModelingResource::Destroy(int flags) {
    if (this) {
        vtable = __vt__Q23gtl24IGenericModelingResource;
        gtl_GenericModel_ReleaseGraphicsResources(&textures);
        if (&textures) {
            GenericVector_Free(&vertexBuffers2);
            GenericVector_Free(&indexBuffers);
            GenericVector_Free(&vertexDecls);
            GenericVector_Free(&vertexBuffers);
            GenericVector_Free(&palettes);
            GenericVector_Free(&textures);
        }
        if (this) {
            GenericRefObj* parent = (GenericRefObj*)backingResource;
            vtable = __vt__Q23gtl20IGenericResourceFile;
            if (parent) {
                if (--parent->rc == 0 && parent) {
                    parent->Release(1);
                }
                backingResource = 0;
            }
        }
        if (flags > 0) {
            GenericFreeFn* f = &s_genericResourceAllocatorFunctions;
            f->fn(this, f);
        }
    }
    return this;
}
#pragma opt_propagation reset
