// Standard reverb shutdown: releases the buffers of an active reverb effect while the effect's
// allocator hooks are installed.
//
// gtl_nw4r_FxReverbStd_Shutdown clears the active flag, releases the buffers, then restores the
// previous hooks. The effect object itself stays allocated.

#include "sdk_protos.h"

namespace gtl {
namespace nw4r {
struct FxReverbStd {
    char effectBase[12];
    unsigned char active; // +0x0C: first byte of the allocator/effect context passed below.
    char effectParameters[31];
    char reverb[1]; // +0x2C: opaque reverb state; this view does not describe its extent.
};
}
}

extern "C" {
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void AudioReverbStd_ReleaseBuffers(void*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);

// Release active reverb buffers while temporarily installing the effect's allocator hooks.
// self is borrowed and remains allocated. Inactive effects are a no-op.
// The flag is cleared before release; saved process-wide hooks are restored afterward.
// Restoration is on the normal path only; no exception guard is installed here.
void gtl_nw4r_FxReverbStd_Shutdown(gtl::nw4r::FxReverbStd* self) {
    if (self->active) {
        unsigned int previousAllocHook;
        unsigned int previousFreeHook;
        self->active = 0;
        gtl_AudioEffects_InstallAllocatorHooks(&self->active, &previousAllocHook, &previousFreeHook);
        AudioReverbStd_ReleaseBuffers(self->reverb);
        gtl_AudioEffects_RestoreAllocatorHooks(&self->active, previousAllocHook, previousFreeHook);
    }
}

} // extern "C"
