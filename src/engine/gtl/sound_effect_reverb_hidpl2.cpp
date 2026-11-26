// DPL2 high reverb sound effect getter: CSoundEffectReverbHiDpl2::GetParameter reads one of
// the six reverb controls, or all six, back from the effect.

#include "sdk_protos.h"
#include "classes/SoundEffectReverbHiDpl2State.h"

extern "C" {
float* gtl_nw4r_FxReverbHiDpl2_GetParams(void*);
void* memcpy(void*, const void*, unsigned long);
}

// Public order verified against GetParams/SetParams in fx_reverb_hi_dpl2.cpp, not SDK work order.
// Pre-delay/decay use the SDK time scale (physical units unresolved); the other
// four controls are dimensionless. This getter returns the raw, unclamped requests.
enum ReverbParameterSelector {
    kReverbPreDelay,
    kReverbDecayTime,
    kReverbColoration,
    kReverbDamping,
    kReverbCrosstalk,
    kReverbMix,
    kAllReverbParameters,
    kReverbParameterCount = 6
};

namespace gtl {

// Read one of six reverb parameters, or copy all six through the caller's output pointer.
// Selectors 0..5 take a writable float*. Selector 6 takes a void** whose pointee
// is a writable 24-byte destination. Neither pointer transfers ownership.
// Unknown selectors perform no write and still return 1. No null/size checks occur.
int CSoundEffectReverbHiDpl2::GetParameter(unsigned int index, void*, void* output) {
    float* parameters = gtl_nw4r_FxReverbHiDpl2_GetParams(effect);
    switch (index) {
    case kReverbPreDelay:
        *(float*)output = parameters[kReverbPreDelay];
        break;
    case kReverbDecayTime:
        *(float*)output = parameters[kReverbDecayTime];
        break;
    case kReverbColoration:
        *(float*)output = parameters[kReverbColoration];
        break;
    case kReverbDamping:
        *(float*)output = parameters[kReverbDamping];
        break;
    case kReverbCrosstalk:
        *(float*)output = parameters[kReverbCrosstalk];
        break;
    case kReverbMix:
        *(float*)output = parameters[kReverbMix];
        break;
    case kAllReverbParameters:
        memcpy(*(void**)output, parameters, kReverbParameterCount * sizeof(float));
        break;
    }
    return 1;
}

}
