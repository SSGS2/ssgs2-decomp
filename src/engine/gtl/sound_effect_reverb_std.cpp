// Standard reverb sound effect setter: CSoundEffectReverbStd::SetParameter changes one reverb
// control or replaces all five, then applies the block to the effect.

#include "classes/SoundEffectReverbStdState.h"

// Standard reverb parameter adapter: selectors below feed fx_reverb_std_params.cpp's setter;
// gtl_nw4r_fx_reverb_std_part1.cpp constructs the effect. Mod pre-delay/decay and spectral/wet controls
// here as a group; the setter keeps raw values and clamps a separate SDK copy.
// Public order: pre-delay, decay time, coloration, damping, wet mix.
// Numeric time scales passed through unchanged: pre-delay 0..0.1, decay 0..10;
// this wrapper and the linked setter do not prove a physical unit. Do not label
// these milliseconds without tracing the SDK consumer. Other controls are ratios.
struct ReverbParameters {
    float preDelay;   // +0x00: pre-delay, SDK clamp [0,0.1].
    float decayTime;  // +0x04: decay time, SDK clamp [0,10].
    float coloration; // +0x08: coloration, SDK clamp [0,1].
    float damping;    // +0x0C: damping, SDK clamp [0,1].
    float wetMix;     // +0x10: wet mix, SDK clamp [0,1].
};

enum ReverbParameter {
    kReverbPreDelay = 0,
    kReverbDecayTime = 1,
    kReverbColoration = 2,
    kReverbDamping = 3,
    kReverbWetMix = 4,
    kReverbAllParameters = 5
};

extern "C" {
ReverbParameters* gtl_nw4r_FxReverbStd_GetParams(void*);
void gtl_nw4r_FxReverbStd_SetParams(void*, const ReverbParameters*);
}

// Update one float parameter, or copy the full parameter block supplied indirectly through value.
// IDs 0..4: value points to one float. ID 5: value points to a POINTER to the
// five-float ReverbParameters block (one extra indirection). Both are borrowed
// only during the call. unused is ignored. Unknown IDs still reapply the current
// block. Returns 1 unconditionally; this wrapper does not report SDK failure.
// The effect at +0x14 is embedded, not an owned pointer. Neither getter result nor
// stack snapshot escapes here.
// For IDs 0..5, value (and ID 5's pointee) must be readable; for other IDs value
// is unused and may be null. The setter stores requested and clamped copies
// before applying active DSP settings; a rejected SDK update rolls neither back.
// Inactive effects only store values. Active updates temporarily install global
// allocation hooks: serialize with effect creation/destruction and audio updates.
int gtl::CSoundEffectReverbStd::SetParameter(int parameter, int unused, const void* value) {
    // Snapshot the current block so a single-parameter update preserves all other controls.
    ReverbParameters parameters = *gtl_nw4r_FxReverbStd_GetParams(effect);
    switch (parameter) {
    case kReverbPreDelay:
        parameters.preDelay = *(const float*)value;
        break;
    case kReverbDecayTime:
        parameters.decayTime = *(const float*)value;
        break;
    case kReverbColoration:
        parameters.coloration = *(const float*)value;
        break;
    case kReverbDamping:
        parameters.damping = *(const float*)value;
        break;
    case kReverbWetMix:
        parameters.wetMix = *(const float*)value;
        break;
    case kReverbAllParameters:
        const ReverbParameters* source = *(const ReverbParameters* const*)value;
        parameters.preDelay = source->preDelay;
        parameters.decayTime = source->decayTime;
        parameters.coloration = source->coloration;
        parameters.damping = source->damping;
        parameters.wetMix = source->wetMix;
        break;
    }
    // Publish a complete block even when the ID did not select any field.
    gtl_nw4r_FxReverbStd_SetParams(effect, &parameters);
    return 1;
}
