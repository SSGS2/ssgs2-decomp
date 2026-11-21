// Translation unit 0x803FB5A4-0x803FB67C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

namespace gtl {
struct CSoundEffectDelay {
    char effectWrapper[0x14];
    char delay[1];
};
}

struct DelayParameters {
    // Raw float order: delay milliseconds, feedback ratio, output ratio.
    // The shared SDK wrapper converts ratios to percentages when it applies them.
    float delayMilliseconds, feedbackRatio, outputRatio;
};

enum DelayParameterSelector {
    kDelayTime, kDelayFeedback, kDelayLevel, kDelayAllParameters
};

extern "C" {

DelayParameters* gtl_nw4r_FxDelay_GetParams(void*);
void gtl_nw4r_FxDelay_SetParams(void*, const DelayParameters*);

// Change one delay parameter, or copy all three from the caller's parameter pointer.
// inputValue borrows float* for selectors 0..2, or DelayParameters** for selector 3.
// Unknown selectors reapply the unchanged snapshot. Returns 1 even if the SDK
// setter rejects the settings; its result is deliberately ignored. No ownership transfers.
int CSoundEffectDelay_SetParameter(gtl::CSoundEffectDelay* self, int selector, int unused, void* inputValue) {
    // Snapshot first so an individual edit preserves the other two controls.
    DelayParameters parameters = *gtl_nw4r_FxDelay_GetParams(self->delay);
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
    gtl_nw4r_FxDelay_SetParams(self->delay, &parameters);
    return 1;
}

} // extern "C"
