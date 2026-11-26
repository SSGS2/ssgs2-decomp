// Destroy for a streamed sound source (gtl::CSoundStreamSource): releases the owned stream resource
// and optionally frees the source's own storage.
//
// Construction is in gtl_sound_stream_source_part1.cpp.

#include "classes/SoundStreamSource.h"

extern "C" {
extern char __vt__Q23gtl18CSoundStreamSource[];
void gtl_Audio_Free(void*);
}

// Destroy the owned stream resource and optionally free the enclosing audio allocation.
// deleteFlag > 0 frees this object; otherwise its storage remains allocated.
// The resource pointer is not cleared: do not destroy twice.
gtl::CSoundStreamSource* gtl::CSoundStreamSource::Destroy(int deleteFlag) {
    if (this) {
        vtbl = __vt__Q23gtl18CSoundStreamSource;
        // Release child ownership before releasing the enclosing allocation.
        if (callback) {
            delete callback;
        }
        if (deleteFlag > 0) {
            gtl_Audio_Free(this);
        }
    }
    return this;
}
