// Translation unit 0x8040A7F0-0x8040A850 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

namespace gtl { namespace nw4r {
struct FxReverbHi {
    char effectBase[0xC];
    unsigned char active;
    char reservedAfterActive[0x23];
    char reverb[1]; // +0x30: address-only view of AXFX work state, not its allocation size.
};
} }

extern "C" {
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void AXFXReverbHiShutdown(void*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);

// Shut down active high-quality reverb under the effect's temporary allocator hooks.
// self must be valid. Inactive effects are a no-op; active effects become inactive
// before SDK teardown releases their buffers. The wrapper itself remains allocated.
// Install/restore change global AXFX hooks; calls must follow the audio system's
// existing serialization. Restore also clears the temporary allocator context.
void gtl_nw4r_FxReverbHi_ShutdownReverb(gtl::nw4r::FxReverbHi* self) {
    if (self->active) {
        unsigned int savedAllocateHook;
        unsigned int savedFreeHook;
        self->active = 0;
        gtl_AudioEffects_InstallAllocatorHooks(&self->active, &savedAllocateHook, &savedFreeHook);
        AXFXReverbHiShutdown(self->reverb);
        gtl_AudioEffects_RestoreAllocatorHooks(&self->active, savedAllocateHook, savedFreeHook);
    }
}

} // extern "C"
