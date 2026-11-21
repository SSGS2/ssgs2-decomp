// GTL animation-resource teardown: relinquish the wrapper's one retained backing
// file reference, then optionally return wrapper storage to the GTL allocator.
// API: gtl_IGenericAnimationResource_dtor is the deleting destructor;
// gtl_IGenericResourceFile_release tears down only its resource-file base.
// Related lookup: gtl_generic_resource_file_chunk.cpp returns borrowed elements from that backing file.
// No gameplay settings, time units or coordinate conversion in this unit.
// Translation unit 0x803DB94C-0x803DB9F8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern void* s_genericResourceAllocatorFunctions[2];
extern void* __vt__Q23gtl20IGenericResourceFile[];

// CodeWarrior puts the non-polymorphic base's refcount at +0 and the derived
// vptr at +4. This is NOT a conventional vptr-at-zero object. GenericRefObj and
// GenericAnimRes describe different objects: the latter owns a pointer at +8
// to the former. Its +0 is the wrapper's own base word, untouched by release.
// Thus ref->rc aliases backing+0, not the backing dispatch pointer at +4.
struct GenericRefBase {
    int rc;
};

struct GenericRefObj : GenericRefBase {
    virtual void Release(int);
};

struct GenericAnimRes {
    char unk0[4];
    void* vt;
    GenericRefObj* ref;
};

// Null wrapper is allowed. Install base dispatch at wrapper+4, decrement the
// optional backing reference once, call backing->Release(1) only at zero, then
// clear wrapper+8. Other aliases must own their own references; borrowed chunk
// addresses become invalid if this was the last owner. No retain, allocation or
// wrapper free here. Counts must be positive; no underflow or concurrency guard.
static inline void gtl_IGenericResourceFile_release(GenericAnimRes* self) {
    GenericRefObj* r;
    if (self != 0) {
        r = self->ref;
        self->vt = __vt__Q23gtl20IGenericResourceFile;
        if (r != 0) {
            if (--r->rc == 0 && r != 0) {
                r->Release(1);
            }
            self->ref = 0;
        }
    }
}

// flags <= 0 destroys the base without freeing wrapper storage; flags > 0 also
// invokes allocator callback s_genericResourceAllocatorFunctions[1] with the complete wrapper address.
// Callback table belongs to the GTL memory operator, not the backing vtable.
// Null is a no-op. Returns the original address, dangling after a freeing call;
// this is destructor ABI bookkeeping, never a live returned resource/status.
GenericAnimRes* gtl_IGenericAnimationResource_dtor(GenericAnimRes* self, int flags) {
    if (self != 0) {
        gtl_IGenericResourceFile_release(self);
        if (flags > 0) {
            ((void (*)(void*))s_genericResourceAllocatorFunctions[1])(self);
        }
    }
    return self;
}

} // extern "C"
