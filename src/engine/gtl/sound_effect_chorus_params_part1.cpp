// Translation unit 0x803FB1FC-0x803FB2D4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

namespace gtl {
struct ChorusParameters {
    // Public raw controls: depth_ms = delay_ms * depthRatio;
    // SDK modulation period_ms = 1000 / rateHertz (gtl_fx_chorus_params.cpp).
    float delayMilliseconds;
    float depthRatio;
    float rateHertz;
};

struct CSoundEffectChorus {
    char soundEffectBaseStorage[0x14]; // Base state is not accessed by this parameter setter.
    char chorus[1];
};
}

extern "C" {

gtl::ChorusParameters* gtl_nw4r_FxChorus_GetParams(void*);
void gtl_nw4r_FxChorus_SetParams(void*, const gtl::ChorusParameters*);

enum ChorusParameter { kChorusDelayMilliseconds = 0, kChorusDepthRatio = 1, kChorusRateHertz = 2, kChorusAllParameters = 3 };

// Update one chorus parameter or copy the caller's full parameter block, then apply it.
// Selectors 0/1/2 borrow float* value; selector 3 borrows ChorusParameters** value.
// No null checks. The third ABI argument is ignored. Unknown selectors reapply
// unchanged parameters. Always returns 1, discarding the SDK setter's result.
int gtl_CSoundEffectChorus_SetParameter(gtl::CSoundEffectChorus* self, int parameter, int unused, void* value)
{
    gtl::ChorusParameters parameters = *gtl_nw4r_FxChorus_GetParams(self->chorus);
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
    gtl_nw4r_FxChorus_SetParams(self->chorus, &parameters);
    return 1;
}

} // extern "C"
