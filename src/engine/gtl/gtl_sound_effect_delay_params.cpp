// Translation unit 0x803FB67C-0x803FB718 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

struct DelayParameters {
    float delayMilliseconds;    // Milliseconds; raw value before SDK clamping.
    float feedbackRatio; // Ratio; SetParams converts to percent for the SDK.
    float outputRatio;      // Output-level ratio, not an established wet/dry blend.
};

namespace gtl {
struct CSoundEffectDelay {
    char baseEffectStorage[0x14];
    char delayEffect[1]; // +0x14: embedded effect wrapper; GetParams finds its +0x18 payload.
};
}

extern "C" {
void* memcpy(void*, const void*, unsigned long);
DelayParameters* gtl_nw4r_FxDelay_GetParams(void*);

enum DelayParameterIndex { kDelayMilliseconds, kDelayFeedbackRatio, kDelayOutputRatio, kAllDelayParameters };

// Read one delay control, or copy the complete parameter block through the output pointer.
// Selectors 0..2 require a writable float*. Selector 3 requires a pointer to a
// writable 12-byte destination (void**); it copies {delay ms, feedback ratio, output ratio}.
// Both destinations are borrowed. Unknown selectors leave output untouched and still return 1.
// No output-pointer validation occurs, and querying does not apply or clamp settings.
int gtl_CSoundEffectDelay_GetParameter(gtl::CSoundEffectDelay* self, int index, int unused, void* output) {
    DelayParameters* parameters = gtl_nw4r_FxDelay_GetParams(self->delayEffect);
    switch (index) {
    case kDelayMilliseconds:
        *(float*)output = parameters->delayMilliseconds;
        break;
    case kDelayFeedbackRatio:
        *(float*)output = parameters->feedbackRatio;
        break;
    case kDelayOutputRatio:
        *(float*)output = parameters->outputRatio;
        break;
    case kAllDelayParameters:
        memcpy(*(void**)output, parameters, sizeof(DelayParameters));
        break;
    }
    return 1;
}

} // extern "C"
