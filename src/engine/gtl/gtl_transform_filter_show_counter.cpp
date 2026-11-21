// Translation unit 0x803F8E58-0x803F9144 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// These unrelated classes share offsets, but +0x14 has different types and ownership.
struct TransformFilterSamples {
    char referenceHeader[4];
    void* vtable;
    char filterState[8];
    int* inputSample; // Retained; word zero is the reference count, vtable is at +4.
    unsigned int* outputSample; // Borrowed from construction; getters add a caller reference.
};
struct ShowCounterDisplayTiming {
    char referenceHeader[4];
    void* vtable;
    char counterState[0xC];
    unsigned int ticksPerHundredSeconds;
};
typedef char TransformFilterSamples_output_offset[(int)&((TransformFilterSamples*)0)->outputSample == 0x14 ? 1 : -1];
typedef char ShowCounterDisplayTiming_offset[(int)&((ShowCounterDisplayTiming*)0)->ticksPerHundredSeconds == 0x14 ? 1 : -1];
extern "C" {

struct RefPrefix { char pad[4]; };
struct RefValue : RefPrefix {
    virtual void release(int);
};

extern unsigned int s_revolutionShowCounterTicks;
void gtl_IFilter_SetState(void*, unsigned int);
void gtl_IShowCounter_GetState(void*, unsigned int*);
void gtl_IShowCounter_ResetCounts(void*);
void gtl_IShowCounter_SetState(void*, unsigned int);
void Video_SetDisplayTickCallback(void*);
void gtl_IShowCounter_Ctor(void*);
void gtl_CMediaSample_CopyFormat(void*, void*);
int gtl_MediaFormat_Equals(void*, void*);
extern unsigned int g_gtlRuntimeConfig[];
extern unsigned int __vt__Q23gtl22CRevolutionShowCounter[];

enum FilterState { kFilterStopped = 0, kFilterRunning = 3 };
enum ShowCounterState { kCounterCounting = 0, kCounterNeedsReset = 2 };
// Display callbacks per 100 seconds: 59.94 Hz NTSC, 50 Hz PAL. The update in
// gtl_revolution_show_counter_part1.cpp divides by this value after multiplying elapsed time by 100.
enum DisplayTiming { kNtscDisplayTiming = 5994, kPalDisplayTiming = 5000, kPalRuntimeMode = 1 };
enum MediaFormatScratchSize { kMediaFormatStorageBytes = 0xA4, kLeftFormatScratchBytes = 0xAC };

// Count one invocation of the registered display callback.
void gtl_CRevolutionShowCounter_Tick(void);

// Put the transform filter into its running state.
int gtl_CTransformFilter_SetRunning(void* self);

// Release the retained input sample and stop the filter.
// Returns 1 even when no input is retained; the borrowed output pointer is untouched.
int gtl_CTransformFilter_Reset(void* self);

// Publish the output sample and give the caller one reference.
// Returns 0 only for a null output address. Otherwise outputSample must exist;
// the caller must release the added reference. The middle argument is ignored.
int gtl_CTransformFilter_GetOutputSample(void* self, void*, unsigned int** output);

// Replace the retained input sample, releasing the previous reference first.
// value must be non-null and remain alive through release of the previous input
// (including when both pointers alias). Returns 1; retains one reference to value.
int gtl_CTransformFilter_SetInputSample(void* self, unsigned int* value);

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
int gtl_CTransformFilter_ResetAndStart(void* self);

// Select the display timing value for the configured television mode.
void* gtl_CRevolutionShowCounter_Ctor(void* self);

// Compare the copied media-format descriptions of two non-null samples.
// Samples are borrowed. Returns 0 for either null, otherwise the format equality
// result. Scratch extents intentionally differ; neither buffer owns a sample.
int gtl_CTransformFilter_FormatsMatch(void*, void* left, void* right);

// Start display counting, clear ticks, and register the display-tick callback.
// Returns 0 if already counting (state 0), leaving ticks/callback unchanged;
// state 2 additionally resets the base counts before starting. Other states start
// without that reset. This installs a global callback, not one per instance.
int gtl_CRevolutionShowCounter_StartDisplayCounting(void* self);

} // extern "C"

// Game entry points in original address order.
extern "C" {

int gtl_CTransformFilter_SetRunning(void* self) {
    gtl_IFilter_SetState(self, kFilterRunning);
    return 1;
}

int gtl_CTransformFilter_Reset(void* self) {
    int* value = ((TransformFilterSamples*)self)->inputSample;
    if (value != 0) {
        int count = *value - 1;
        *value = count;
        if (count == 0 && value != 0) {
            ((RefValue*)value)->release(1);
        }
        *(unsigned int**)&((TransformFilterSamples*)self)->inputSample = 0;
    }
    gtl_IFilter_SetState(self, kFilterStopped);
    return 1;
}

int gtl_CTransformFilter_ResetAndStart(void* self) {
    ((TransformDispatch*)self)->reset();
    ((TransformDispatch*)self)->lifecycle();
    return 1;
}

int gtl_CTransformFilter_GetOutputSample(void* self, void*, unsigned int** output) {
    if (output == 0) {
        return 0;
    }
    unsigned int* value = ((TransformFilterSamples*)self)->outputSample;
    *output = value;
    (*value)++;
    return 1;
}

int gtl_CTransformFilter_SetInputSample(void* self, unsigned int* value) {
    int* old = ((TransformFilterSamples*)self)->inputSample;
    if (old != 0) {
        int count = *old - 1;
        *old = count;
        if (count == 0 && old != 0) {
            ((RefValue*)old)->release(1);
        }
        *(unsigned int**)&((TransformFilterSamples*)self)->inputSample = 0;
    }
    ((TransformFilterSamples*)self)->inputSample = (int*)value;
    (*value)++;
    return 1;
}

int gtl_CTransformFilter_FormatsMatch(void*, void* left, void* right) {
    if (left == 0 || right == 0) {
        return 0;
    }
    char leftTemp[kLeftFormatScratchBytes];
    char rightTemp[kMediaFormatStorageBytes];
    gtl_CMediaSample_CopyFormat(left, leftTemp);
    gtl_CMediaSample_CopyFormat(right, rightTemp);
    return gtl_MediaFormat_Equals(leftTemp, rightTemp);
}

void gtl_CRevolutionShowCounter_Tick(void) {
    s_revolutionShowCounterTicks++;
}

void* gtl_CRevolutionShowCounter_Ctor(void* self) {
    gtl_IShowCounter_Ctor(self);
    void** vtable = (void**)__vt__Q23gtl22CRevolutionShowCounter;
    ((ShowCounterDisplayTiming*)self)->vtable = vtable;
    unsigned int value = kNtscDisplayTiming;
    if ((int)g_gtlRuntimeConfig[2] == kPalRuntimeMode) {
        value = kPalDisplayTiming;
    }
    ((ShowCounterDisplayTiming*)self)->ticksPerHundredSeconds = value;
    return self;
}

int gtl_CRevolutionShowCounter_StartDisplayCounting(void* self) {
    unsigned int state;
    gtl_IShowCounter_GetState(self, &state);
    if (state == kCounterCounting) {
        return 0;
    }
    if ((int)state == kCounterNeedsReset) {
        gtl_IShowCounter_ResetCounts(self);
    }
    gtl_IShowCounter_SetState(self, kCounterCounting);
    s_revolutionShowCounterTicks = 0;
    Video_SetDisplayTickCallback((void*)gtl_CRevolutionShowCounter_Tick);
    return 1;
}
} // extern "C"
