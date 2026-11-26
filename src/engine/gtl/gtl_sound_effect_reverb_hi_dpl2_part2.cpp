// High-quality DPL2 reverb sound effect: the storage-release entry, and the enable and
// enable-query calls that forward to the embedded effect.

#include "classes/SoundEffectReverbHiDpl2State.h"

extern "C" {

void gtl_Audio_Free(void*);
void gtl_SoundEffect_SetEnabled(void*, void*, void*);
void gtl_SoundEffect_GetEnabled(void*, void*, void*);

// Release the high-quality DPL2 reverb wrapper's allocation through the audio allocator.
// Effect finalization must already be complete; this entry only releases storage.
void dtor_803FA970(void* self) {
    gtl_Audio_Free(self);
}

} // extern "C"

namespace gtl {

// Forward scalar enable mode 0/1 to the embedded effect without changing ownership.
// enabledMode uses a pointer-shaped ABI slot; it is not dereferenced as a pointer.
void CSoundEffectReverbHiDpl2::SetEnabled(void* enabledMode) {
    gtl_SoundEffect_SetEnabled(this, effect, enabledMode);
}

// Write the embedded effect's enable state (0/1) to caller-owned int storage.
// outEnabled must address a writable int; the existing void return ABI is retained.
void CSoundEffectReverbHiDpl2::GetEnabled(void* outEnabled) {
    gtl_SoundEffect_GetEnabled(this, effect, outEnabled);
}

}
