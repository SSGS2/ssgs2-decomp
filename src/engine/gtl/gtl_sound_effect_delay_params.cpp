// Delay sound effect parameters: reads one delay control (delay time, feedback ratio or output
// ratio) or the whole parameter block from gtl::CSoundEffectDelay.

#include "sdk_protos.h"
#include "classes/SoundEffectDelayState.h"

struct DelayParameters {
    float delayMilliseconds; // Milliseconds; raw value before SDK clamping.
    float feedbackRatio;     // Ratio; SetParams converts to percent for the SDK.
    float outputRatio;       // Output-level ratio, not an established wet/dry blend.
};

extern "C" {
void* memcpy(void*, const void*, unsigned long);
DelayParameters* gtl_nw4r_FxDelay_GetParams(void*);

enum DelayParameterIndex {
    kDelayMilliseconds,
    kDelayFeedbackRatio,
    kDelayOutputRatio,
    kAllDelayParameters
};

// Read one delay control, or copy the complete parameter block through the output pointer.
// Selectors 0..2 require a writable float*. Selector 3 requires a pointer to a
// writable 12-byte destination (void**); it copies {delay ms, feedback ratio, output ratio}.
// Both destinations are borrowed. Unknown selectors leave output untouched and still return 1.
// No output-pointer validation occurs, and querying does not apply or clamp settings.
} // extern "C"

namespace gtl {

int CSoundEffectDelay::GetParameter(int index, int unused, void* output) {
    DelayParameters* parameters = gtl_nw4r_FxDelay_GetParams(effect);
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

}
