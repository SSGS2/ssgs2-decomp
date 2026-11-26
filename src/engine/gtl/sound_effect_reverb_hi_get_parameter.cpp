// High reverb sound effect getter: CSoundEffectReverbHi::GetParameter reads pre-delay, time,
// coloration, damping, crosstalk and mix, or all six, back from the effect.

#include "sdk_protos.h"
#include "MSL/internal/mem.h"
#include "classes/SoundEffectReverbHiState.h"

extern "C" {
float* gtl_nw4r_FxReverbHi_GetParams(void*);

enum ReverbParameter {
    kPreDelay,
    kTime,
    kColoration,
    kDamping,
    kCrosstalk,
    kMix,
    kAllParameters
};

} // extern "C"

// Parameter IDs index the raw request block: 0 pre-delay, 1 time, 2 coloration, 3 damping, 4 crosstalk, 5 mix,
// 6 all. The SDK field order differs; the mapping is in the reverb backend. Values pass through unchanged.
// Parameter 0..5 writes one float to output; parameter 6 copies six floats to the buffer that output points to.
// Unknown indices leave output untouched. Always returns 1.
int gtl::CSoundEffectReverbHi::GetParameter(unsigned int parameter, unsigned int unused, void* output) {
    float* values = gtl_nw4r_FxReverbHi_GetParams(&effectVtable);
    switch (parameter) {
    case kPreDelay:
        *(float*)output = values[kPreDelay];
        break;
    case kTime:
        *(float*)output = values[kTime];
        break;
    case kColoration:
        *(float*)output = values[kColoration];
        break;
    case kDamping:
        *(float*)output = values[kDamping];
        break;
    case kCrosstalk:
        *(float*)output = values[kCrosstalk];
        break;
    case kMix:
        *(float*)output = values[kMix];
        break;
    case kAllParameters:
        memcpy(*(void**)output, values, kAllParameters * sizeof(float));
        break;
    }
    return 1;
}
