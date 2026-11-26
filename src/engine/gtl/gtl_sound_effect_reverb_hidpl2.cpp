// Parameter updates for the DPL2 high-quality reverb effect (CSoundEffectReverbHiDpl2).
// Each update reads the current six-value block, changes one value or the whole block, and
// applies it to the NW4R effect.

#include "sdk_protos.h"
#include "classes/SoundEffectReverbHiDpl2State.h"

namespace gtl {
struct ReverbParameters {
    float coloration;
    float mix;
    float time;
    float damping;
    float preDelay;
    float crosstalk;
};
}

extern "C" {
gtl::ReverbParameters* gtl_nw4r_FxReverbHiDpl2_GetParams(void*);
void gtl_nw4r_FxReverbHiDpl2_SetParams(void*, const gtl::ReverbParameters*);
}

// This is the six-float parameter block passed to the effect wrapper. Preserve its
// order: coloration, mix, time, damping, preDelay, crosstalk. This setter performs
// no scaling or range checks; units/accepted ranges belong to the effect API and
// are not established by this translation unit alone.
enum ReverbParameter {
    kColoration = 0,
    kMix = 1,
    kTime = 2,
    kDamping = 3,
    kPreDelay = 4,
    kCrosstalk = 5,
    kAllParameters = 6
};

namespace gtl {

// Update one reverb parameter, or copy the complete parameter block, then apply it.
// IDs 0..5 read value as const float*. ID 6 reads it as a pointer to a borrowed
// ReverbParameters pointer (one extra indirection), not as a direct block pointer.
// Recognized IDs require valid non-null input storage; no input ownership is retained.
// Other IDs reapply the unchanged current block. unused is ignored; always returns 1,
// regardless of the wrapper's ability to apply the parameters.
int CSoundEffectReverbHiDpl2::SetParameter(unsigned int parameter, unsigned int unused, const void* value) {
    // Snapshot current values so a single-parameter update preserves the other five.
    ReverbParameters params = *gtl_nw4r_FxReverbHiDpl2_GetParams(effect);
    switch (parameter) {
    case kColoration:
        params.coloration = *(const float*)value;
        break;
    case kMix:
        params.mix = *(const float*)value;
        break;
    case kTime:
        params.time = *(const float*)value;
        break;
    case kDamping:
        params.damping = *(const float*)value;
        break;
    case kPreDelay:
        params.preDelay = *(const float*)value;
        break;
    case kCrosstalk:
        params.crosstalk = *(const float*)value;
        break;
    case kAllParameters:
        const ReverbParameters* source = *(const ReverbParameters* const*)value;
        params.coloration = source->coloration;
        params.mix = source->mix;
        params.time = source->time;
        params.damping = source->damping;
        params.preDelay = source->preDelay;
        params.crosstalk = source->crosstalk;
        break;
    }
    // Apply once, including the unchanged snapshot for an unrecognized parameter ID.
    gtl_nw4r_FxReverbHiDpl2_SetParams(effect, &params);
    return 1;
}

}
