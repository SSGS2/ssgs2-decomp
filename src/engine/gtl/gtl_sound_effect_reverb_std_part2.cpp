// Helpers for the standard reverb effect (CSoundEffectReverbStd): the enable setter and getter,
// and a plain release of the wrapper's allocation.
//
// Destroy and the effect's teardown are in gtl_sound_effect_reverb_std_part1.cpp.

#include "classes/SoundEffectReverbStdState.h"

extern "C" {

void gtl_Audio_Free(void* self);
void gtl_SoundEffect_SetEnabled(void* self, void* reverb, int enabledMode);
void gtl_SoundEffect_GetEnabled(void* self, void* reverb, int outputAddress);

// Release this allocation through the audio allocator; no effect teardown occurs here.
void dtor_803FADB0(void* self) {
    gtl_Audio_Free(self);
}

} // extern "C"

// Request enable mode (0 disables, 1 enables) on the borrowed embedded effect at +0x14.
// The helper rejects other modes unless already cached; its return value is discarded.
void gtl::CSoundEffectReverbStd::SetEnabled(int enabledMode) {
    gtl_SoundEffect_SetEnabled(this, effect, enabledMode);
}

// Write the embedded effect's boolean enable state to the borrowed int at outputAddress.
// This int parameter carries a Wii address, not an enable value; it must be writable.
void gtl::CSoundEffectReverbStd::GetEnabled(int outputAddress) {
    gtl_SoundEffect_GetEnabled(this, effect, outputAddress);
}
