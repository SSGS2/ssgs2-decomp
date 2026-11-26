// The stream callback of CSoundStreamSource: its buffer record, and the destructor that
// releases the buffer data the callback owns and then the callback itself if asked.

namespace gtl {
struct CSoundStreamSource {
    struct StrmCallback {
        void* vtable;
        struct Buffer {
            unsigned int flags; // Bit 31 makes this callback responsible for data.
            unsigned int size;  // Not inspected by this destructor; units unverified here.
            void* data;         // Owned when flags bit 31 is set, otherwise borrowed.
        } buffer;
    };
};
}

extern "C" {

void gtl_Audio_Free(void*);
enum {
    kOwnsBufferFlagShift = 31
};

// Release owned stream-buffer data, then optionally release the callback itself.
// Null is accepted; deleting > 0 releases self via the same audio allocator.
// Return the incoming address, which may be freed. No fields are reset, so this
// cleanup is not repeatable on an object whose owned data has already been freed.
gtl::CSoundStreamSource::StrmCallback*
gtl_CSoundStreamSource_StrmCallback_dtor(gtl::CSoundStreamSource::StrmCallback* self, int deleting) {
    if (self) {
        if (&self->buffer) {
            if (self->buffer.flags >> kOwnsBufferFlagShift) {
                gtl_Audio_Free(self->buffer.data);
            }
        }
        if (deleting > 0) {
            gtl_Audio_Free(self);
        }
    }
    return self;
}

} // extern "C"
