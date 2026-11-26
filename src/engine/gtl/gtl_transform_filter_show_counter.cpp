// Transform filter and display counter: the sample handling and running state of the transform filter,
// and the revolution show counter that counts display callbacks and selects timing for the TV mode.

#include "classes/gtl_IFilter.h"
#include "classes/GtlMediaSample.h"
#include "classes/GtlShowCounter.h"
#include "classes/gtl_CTransformFilter.h"
#include "classes/gtl_CRevolutionShowCounter.h"

extern "C" {

struct RefPrefix {
    char pad[4];
};
struct RefValue : RefPrefix {
    virtual void release(int);
};

extern unsigned int s_revolutionShowCounterTicks;
void Video_SetDisplayTickCallback(void*);
int gtl_MediaFormat_Equals(void*, void*);
extern unsigned int g_gtlRuntimeConfig[];
extern unsigned int __vt__Q23gtl22CRevolutionShowCounter[];

enum FilterState {
    kFilterStopped = 0,
    kFilterRunning = 3
};
enum ShowCounterState {
    kCounterCounting = 0,
    kCounterNeedsReset = 2
};
// Display callbacks per 100 seconds: 59.94 Hz NTSC, 50 Hz PAL. The update in
// gtl_revolution_show_counter_part1.cpp divides by this value after multiplying elapsed time by 100.
enum DisplayTiming {
    kNtscDisplayTiming = 5994,
    kPalDisplayTiming = 5000,
    kPalRuntimeMode = 1
};
enum MediaFormatScratchSize {
    kMediaFormatStorageBytes = 0xA4,
    kLeftFormatScratchBytes = 0xAC
};

// Invoke the filter's reset slot before its second lifecycle slot.
// Slot indices 6 then 4 are called on the complete object, not the +4 vtable field;
// reload the vtable after reset, which may change the dynamic implementation.
struct TransformDispatch : RefPrefix {
    virtual void destroy(int);
    virtual void unused0C();
    virtual void lifecycle();
    virtual void unused14();
    virtual void reset();
};

} // extern "C"

// Filter transform member functions, in original address order.

// Put the transform filter into its running state.
int gtl::CTransformFilter::SetRunning() {
    ((gtl::IFilter*)this)->SetState(kFilterRunning);
    return 1;
}

// Release the retained input sample and stop the filter.
// Returns 1 even when no input is retained; the borrowed output pointer is untouched.
int gtl::CTransformFilter::Reset() {
    int* value = inputSample;
    if (value != 0) {
        int count = *value - 1;
        *value = count;
        if (count == 0 && value != 0) {
            ((RefValue*)value)->release(1);
        }
        inputSample = 0;
    }
    ((gtl::IFilter*)this)->SetState(kFilterStopped);
    return 1;
}

// Reset the filter, then restart it through its lifecycle slot.
int gtl::CTransformFilter::ResetAndStart() {
    ((TransformDispatch*)this)->reset();
    ((TransformDispatch*)this)->lifecycle();
    return 1;
}

// Publish the output sample and give the caller one reference.
// Returns 0 only for a null output address. Otherwise outputSample must exist;
// the caller must release the added reference. The middle argument is ignored.
int gtl::CTransformFilter::GetOutputSample(void*, unsigned int** output) {
    if (output == 0) {
        return 0;
    }
    unsigned int* value = outputSample;
    *output = value;
    (*value)++;
    return 1;
}

// Replace the retained input sample, releasing the previous reference first.
// value must be non-null and remain alive through release of the previous input
// (including when both pointers alias). Returns 1; retains one reference to value.
int gtl::CTransformFilter::SetInputSample(unsigned int* value) {
    int* old = inputSample;
    if (old != 0) {
        int count = *old - 1;
        *old = count;
        if (count == 0 && old != 0) {
            ((RefValue*)old)->release(1);
        }
        inputSample = 0;
    }
    inputSample = (int*)value;
    (*value)++;
    return 1;
}

// Compare the copied media-format descriptions of two non-null samples.
// Samples are borrowed. Returns 0 for either null, otherwise the format equality
// result. Scratch extents intentionally differ; neither buffer owns a sample.
int gtl::CTransformFilter::FormatsMatch(void* left, void* right) {
    if (left == 0 || right == 0) {
        return 0;
    }
    char leftTemp[kLeftFormatScratchBytes];
    char rightTemp[kMediaFormatStorageBytes];
    ((gtl::CMediaSample*)left)->CopyFormat(leftTemp);
    ((gtl::CMediaSample*)right)->CopyFormat(rightTemp);
    return gtl_MediaFormat_Equals(leftTemp, rightTemp);
}

// Count one invocation of the registered display callback. The callback is global, not per instance.
void gtl::CRevolutionShowCounter::Tick() {
    s_revolutionShowCounterTicks++;
}

// Set the vtable and select the display timing for the configured television mode.
void* gtl::CRevolutionShowCounter::Init() {
    IShowCounter::Init();
    vtbl = (void*)__vt__Q23gtl22CRevolutionShowCounter;
    unsigned int value = kNtscDisplayTiming;
    if ((int)g_gtlRuntimeConfig[2] == kPalRuntimeMode) {
        value = kPalDisplayTiming;
    }
    ticksPerHundredSeconds = value;
    return this;
}

// Start display counting, clear ticks, and register the display-tick callback.
// Returns 0 if already counting (state 0), leaving ticks/callback unchanged;
// state 2 additionally resets the base counts before starting.
int gtl::CRevolutionShowCounter::StartDisplayCounting() {
    unsigned int state;
    GetState(&state);
    if (state == kCounterCounting) {
        return 0;
    }
    if ((int)state == kCounterNeedsReset) {
        ResetCounts();
    }
    SetState(kCounterCounting);
    s_revolutionShowCounterTicks = 0;
    Video_SetDisplayTickCallback((void*)Tick);
    return 1;
}
