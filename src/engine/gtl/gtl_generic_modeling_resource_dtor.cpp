// gtl model-resource teardown: release graphics handles, their six owned pointer
// arrays, the retained resource buffer, then optionally the wrapper allocation.
// API lookup: this vfunc_0 is the deleting destructor; population/release of the
// handle arrays is gtl_GenericModel_LoadGraphicsResources/ReleaseGraphicsResources
// (gtl_generic_model_graphics_load.cpp). Allocation/free callbacks are the pair at s_genericResourceAllocatorFunctions.
// No gameplay tuning knobs. Change resource lifetime only with that producer.
// Translation unit 0x803D9250-0x803D9498 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct RcBase {
    int rc;
};

struct GenericRefObj : RcBase {
    virtual void Release(int arg);
};

struct GenericEntry {
    void* ptr; // Owned pointer-array allocation; handles are released separately.
    int size;  // Live element count, not bytes.
    int unk8;  // Vector capacity word; this destructor does not read it.
};

struct GenericModelingResource {
    int refCount;
    void* vtable;
    GenericRefObj* backingResource; // Retained backing resource; release after all arrays.
    // Six 12-byte pointer vectors, starting at wrapper +0xC. Producer evidence:
    // gtl_generic_model_graphics_load.cpp loads textures (+0), palettes (+0xC), vertex buffers (+0x18),
    // vertex declarations (+0x24), index buffers (+0x30), and an additional
    // vertex-buffer vector (+0x3C) sized when model description +0x18 is nonzero.
    // Its exact extra-buffer use is unresolved; it still owns retained handles.
    // ReleaseGraphicsResources clears/relinquishes elements in forward order;
    // the destructor below then frees array storage in reverse order.
    GenericEntry entries[6];
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

// Free one vector's array after graphics-handle release has made its size zero.
// ptr is not nulled: the containing object is being destroyed and cannot be reused.
// The five identical tests are nested inlined member/base destructor guards, all
// on this same entry; they do not indicate five buffers. Keep the size read and
// subtract-to-zero store before invoking the allocator's free callback.
static inline void GenericEntry_Free(GenericEntry* e) {
    if (e && e && e && e && e && e->ptr) {
        GenericFreeFn* f = &s_genericResourceAllocatorFunctions;
        int n = e->size;
        e->size -= n;
        f->fn(e->ptr, f);
    }
}

// Disabling propagation preserves the expanded destructor's repeated null guards
// and load/store/callback boundaries, including the member-address check below.
#pragma opt_propagation off
// Nullable self; flags>0 also frees wrapper storage through s_genericResourceAllocatorFunctions.fn,
// flags<=0 destroys members only (caller still owns the raw storage). No status
// is returned: the result is the incoming address, null for null input and a
// dangling identity only after deletion. Never dereference it after flags>0.
// Parent references are decremented once, Release(1) only at zero; parent is then
// cleared. No new resources are allocated and no rollback/failure code exists.
void* gtl_IGenericModelingResource_dtor(GenericModelingResource* self, int flags) {
    if (self) {
        self->vtable = __vt__Q23gtl24IGenericModelingResource;
        gtl_GenericModel_ReleaseGraphicsResources(&self->entries[0]);
        if (&self->entries[0]) {
            GenericEntry_Free(&self->entries[5]);
            GenericEntry_Free(&self->entries[4]);
            GenericEntry_Free(&self->entries[3]);
            GenericEntry_Free(&self->entries[2]);
            GenericEntry_Free(&self->entries[1]);
            GenericEntry_Free(&self->entries[0]);
        }
        if (self) {
            GenericRefObj* parent = self->backingResource;
            self->vtable = __vt__Q23gtl20IGenericResourceFile;
            if (parent) {
                if (--parent->rc == 0 && parent) {
                    parent->Release(1);
                }
                self->backingResource = 0;
            }
        }
        if (flags > 0) {
            GenericFreeFn* f = &s_genericResourceAllocatorFunctions;
            f->fn(self, f);
        }
    }
    return self;
}
#pragma opt_propagation reset

} // extern "C"
