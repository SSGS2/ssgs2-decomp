// Chorus sound effect setter: CSoundEffectChorus::SetParameter changes the delay, depth or
// rate, or replaces all three, and applies the result to the effect.

#include "sdk_protos.h"
#include "classes/SoundEffectFields803FB370.h"

namespace gtl {
struct ChorusParameters {
    // Public raw controls: depth_ms = delay_ms * depthRatio;
    // SDK modulation period_ms = 1000 / rateHertz (gtl_fx_chorus_params.cpp).
    float delayMilliseconds;
    float depthRatio;
    float rateHertz;
};
}

extern "C" {

gtl::ChorusParameters* gtl_nw4r_FxChorus_GetParams(void*);
void gtl_nw4r_FxChorus_SetParams(void*, const gtl::ChorusParameters*);

enum ChorusParameter {
    kChorusDelayMilliseconds = 0,
    kChorusDepthRatio = 1,
    kChorusRateHertz = 2,
    kChorusAllParameters = 3
};

} // extern "C"

// Update one chorus parameter or copy the caller's full parameter block, then apply it.
// Selectors 0/1/2 borrow float* value; selector 3 borrows ChorusParameters** value.
// No null checks. The third ABI argument is ignored. Unknown selectors reapply
// unchanged parameters. Always returns 1, discarding the SDK setter's result.
int gtl::CSoundEffectChorus::SetParameter(int parameter, int unused, void* value) {
    gtl::ChorusParameters parameters = *gtl_nw4r_FxChorus_GetParams(effect);
    switch (parameter) {
    case kChorusDelayMilliseconds:
        parameters.delayMilliseconds = *(float*)value;
        break;
    case kChorusDepthRatio:
        parameters.depthRatio = *(float*)value;
        break;
    case kChorusRateHertz:
        parameters.rateHertz = *(float*)value;
        break;
    case kChorusAllParameters:
        gtl::ChorusParameters* source = *(gtl::ChorusParameters**)value;
        parameters.delayMilliseconds = source->delayMilliseconds;
        parameters.depthRatio = source->depthRatio;
        parameters.rateHertz = source->rateHertz;
        break;
    }
    gtl_nw4r_FxChorus_SetParams(effect, &parameters);
    return 1;
}
