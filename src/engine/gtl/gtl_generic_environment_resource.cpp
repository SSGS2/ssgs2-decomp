// Translation unit 0x803DBAA8-0x803DBB54 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct GtlGenericResourceFileBase {
    int refCount;
};
struct GtlGenericResourceFile : GtlGenericResourceFileBase {
    virtual void dispose(int);
};

struct GtlIGenericEnvironmentResource {
    int unk0;
    void* vtable;
    GtlGenericResourceFile* resource;
};

struct GtlSdaFnSlot {
    int unk0;
    void (*fn)(GtlIGenericEnvironmentResource*);
};

extern "C" {

extern char __vt__Q23gtl20IGenericResourceFile[];
extern GtlSdaFnSlot s_genericResourceAllocatorFunctions;

GtlIGenericEnvironmentResource* gtl_IGenericEnvironmentResource_dtor(GtlIGenericEnvironmentResource* self, int flag) {
    if (self) {
        if (self) {
            GtlGenericResourceFile* res = self->resource;
            self->vtable = __vt__Q23gtl20IGenericResourceFile;
            if (res) {
                if (--res->refCount == 0 && res) {
                    res->dispose(1);
                }
                self->resource = 0;
            }
        }
        if (flag > 0) {
            s_genericResourceAllocatorFunctions.fn(self);
        }
    }
    return self;
}

} // extern "C"
