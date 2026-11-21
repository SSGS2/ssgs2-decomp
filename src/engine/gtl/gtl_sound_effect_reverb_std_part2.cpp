// Translation unit 0x803FADB0-0x803FADCC (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/SoundEffectReverbStdState.h"

extern "C" {

void gtl_Audio_Free(void* self);
void gtl_SoundEffect_SetEnabled(void* self, void* reverb, int enabledMode);
void gtl_SoundEffect_GetEnabled(void* self, void* reverb, int outputAddress);

// Release this allocation through the audio allocator; no effect teardown occurs here.
void dtor_803FADB0(void* self) {
    gtl_Audio_Free(self);
}

// Request enable mode (0 disables, 1 enables) on the borrowed embedded effect at +0x14.
// The helper rejects other modes unless already cached; its return value is discarded.
void gtl_CSoundEffectReverbStd_SetEnabled(void* self, int enabledMode) {
    gtl_SoundEffect_SetEnabled(self, ((SoundEffectReverbStdState*)self)->effect, enabledMode);
}

// Write the embedded effect's boolean enable state to the borrowed int at outputAddress.
// This int parameter carries a Wii address, not an enable value; it must be writable.
void gtl_CSoundEffectReverbStd_GetEnabled(void* self, int outputAddress) {
    gtl_SoundEffect_GetEnabled(self, ((SoundEffectReverbStdState*)self)->effect, outputAddress);
}

} // extern "C"
