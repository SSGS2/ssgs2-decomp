// High-quality reverb parameters: the accessor that gives a borrowed view of the six
// reverb settings held in the effect.
//
// The layout is classes/FxReverbHiFlags.h.

#include "classes/FxReverbHiFlags.h"

extern "C" {

// Return a borrowed, writable view of the reverb parameter block at effect +0x18.
// effect must point to a valid FxReverbHi object; its lifetime bounds this view.
// Callers access six float words: pre-delay, time, coloration, damping, crosstalk,
// mix. The header's one-byte array is an address-only layout view, not its size.
void* gtl_nw4r_FxReverbHi_GetParams(void* effect) {
    return AsFxReverbHiFlags(effect)->parameters;
}

} // extern "C"
