// Translation unit 0x803FA970-0x803FA98C (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

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

// Forward scalar enable mode 0/1 to the embedded effect without changing ownership.
// enabledModeWord uses a pointer-shaped ABI slot; it is not dereferenced as a pointer.
void gtl_CSoundEffectReverbHiDpl2_SetEnabled(void* self, void* enabledModeWord) {
    gtl_SoundEffect_SetEnabled(self, ((SoundEffectReverbHiDpl2State*)self)->effect, enabledModeWord);
}

// Write the embedded effect's enable state (0/1) to caller-owned int storage.
// outEnabled must address a writable int; the existing void return ABI is retained.
void gtl_CSoundEffectReverbHiDpl2_GetEnabled(void* self, void* outEnabled) {
    gtl_SoundEffect_GetEnabled(self, ((SoundEffectReverbHiDpl2State*)self)->effect, outEnabled);
}

} // extern "C"
