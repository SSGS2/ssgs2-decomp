// Delay sound effect setter: CSoundEffectDelay::SetParameter changes the delay time, feedback
// or output level, or replaces all three, and applies the result to the effect.

#include "classes/SoundEffectDelayState.h"

struct DelayParameters {
    // Raw float order: delay milliseconds, feedback ratio, output ratio.
    // The shared SDK wrapper converts ratios to percentages when it applies them.
    float delayMilliseconds, feedbackRatio, outputRatio;
};

enum DelayParameterSelector {
    kDelayTime,
    kDelayFeedback,
    kDelayLevel,
    kDelayAllParameters
};

extern "C" {

DelayParameters* gtl_nw4r_FxDelay_GetParams(void*);
void gtl_nw4r_FxDelay_SetParams(void*, const DelayParameters*);

} // extern "C"

namespace gtl {

// Change one delay parameter, or copy all three from the caller's parameter pointer.
// inputValue borrows float* for selectors 0..2, or DelayParameters** for selector 3.
// Unknown selectors reapply the unchanged snapshot. Returns 1 even if the SDK
// setter rejects the settings; its result is deliberately ignored. No ownership transfers.
int CSoundEffectDelay::SetParameter(int selector, int unused, void* inputValue) {
    // Snapshot first so an individual edit preserves the other two controls.
    DelayParameters parameters = *gtl_nw4r_FxDelay_GetParams(effect);
    switch (selector) {
    case kDelayTime:
        parameters.delayMilliseconds = *(float*)inputValue;
        break;
    case kDelayFeedback:
        parameters.feedbackRatio = *(float*)inputValue;
        break;
    case kDelayLevel:
        parameters.outputRatio = *(float*)inputValue;
        break;
    case kDelayAllParameters:
        DelayParameters* allParameters = *(DelayParameters**)inputValue;
        parameters.delayMilliseconds = allParameters->delayMilliseconds;
        parameters.feedbackRatio = allParameters->feedbackRatio;
        parameters.outputRatio = allParameters->outputRatio;
        break;
    }
    // Apply the complete snapshot; the embedded effect begins at self+0x14.
    gtl_nw4r_FxDelay_SetParams(effect, &parameters);
    return 1;
}

}
