// Translation unit 0x804043EC-0x80404470 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct StreamResource {
    // Release the resource through its dynamic type; implementation lives elsewhere.
    virtual ~StreamResource();
};

namespace gtl {
struct CSoundStreamSource {
    unsigned int unk0;
    void* vtable;
    char unk8[0x94];
    StreamResource* resource;
};
}

extern "C" {
extern char __vt__Q23gtl18CSoundStreamSource[];
void gtl_Audio_Free(void*);

// Destroy the owned stream resource and optionally free the enclosing audio allocation.
// Null is accepted. deleteFlag > 0 frees self; otherwise its storage remains allocated.
// The returned address may be freed. The resource pointer is not cleared: do not destroy twice.
gtl::CSoundStreamSource* gtl_CSoundStreamSource_dtor(gtl::CSoundStreamSource* self, int deleteFlag) {
    if (self) {
        self->vtable = __vt__Q23gtl18CSoundStreamSource;
        // Release child ownership before releasing the enclosing allocation.
        if (self->resource) {
            delete self->resource;
        }
        if (deleteFlag > 0) {
            gtl_Audio_Free(self);
        }
    }
    return self;
}

} // extern "C"
