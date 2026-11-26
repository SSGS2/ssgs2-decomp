// Delay effect settings: the accessor for the three stored parameters (delay in milliseconds,
// feedback and output percent) and the setter, which clamps them and applies them when initialized.
//
// The layout is the GtlFxDelay class (classes/GtlFxDelay.h); processing and shutdown are in other files.

#include "classes/GtlFxDelay.h"

extern "C" {

extern const float kFloat_100_AudioReverb;
unsigned long Runtime_ConvertDoubleToUnsigned(double x);
int gtl_AudioEffects_InstallAllocatorHooks(void* p, unsigned long* a, unsigned long* b);
int gtl_AudioEffects_RestoreAllocatorHooks(void* p, unsigned long a, unsigned long b);
int AXFXDelaySettings(void* p);

struct Obj_8040A3D0 {
    unsigned char pad00[0x0C];
    unsigned char initializationAttempted;
    unsigned char pad0D[0x0B];
    float parameters[3];
    unsigned char pad24[0x40];
    unsigned long delay0;
    unsigned long delay1;
    unsigned long delay2;
    unsigned long feedback0;
    unsigned long feedback1;
    unsigned long feedback2;
    unsigned long level0;
    unsigned long level1;
    unsigned long level2;
};
// This partial view aliases GtlFxDelay: flag is initialized (+0x0C), vec is
// parameters (+0x18), and the nine words at +0x64..+0x84 are three lanes each
// of delay milliseconds, feedback percent and output percent. Lane identities
// are not established here; each lane receives the same setting.
// Member names are provisional; the layout is exact.
enum DelayParameter {
    kDelayMilliseconds,
    kFeedbackRatio,
    kOutputRatio
};
enum {
    kMinimumDelayMilliseconds = 1,
    kMaximumDelayMilliseconds = 5000,
    kMaximumFeedbackPercent = 99,
    kMaximumOutputPercent = 100
};

// Return a borrowed, writable pointer to {delay ms, feedback ratio, output ratio}.
// Writing through this pointer alone does not apply settings to the SDK delay.
unsigned char* gtl_nw4r_FxDelay_GetParams(void* p) {
    return (unsigned char*)((GtlFxDelay*)p)->parameters; // 0x8040A3D0: parameter block +0x18
}

// Copy three input floats and apply the clamped SDK settings if already initialized.
// Input order/units match GetParams; raw floats remain unclamped in the stored copy.
// Returns 1 when deferred, otherwise the Boolean AXFXDelaySettings result. Conversion
// precedes unsigned clamping; this routine adds no negative/NaN input validation.
int gtl_nw4r_FxDelay_SetParams(Obj_8040A3D0* self, const float* input) {
    // Retain the caller's raw settings, then replicate converted values across lanes.
    self->parameters[kDelayMilliseconds] = input[kDelayMilliseconds];
    self->parameters[kFeedbackRatio] = input[kFeedbackRatio];
    self->parameters[kOutputRatio] = input[kOutputRatio];
    unsigned long sdkValue = Runtime_ConvertDoubleToUnsigned(input[kDelayMilliseconds]);
    sdkValue = sdkValue > kMaximumDelayMilliseconds
                   ? kMaximumDelayMilliseconds
                   : (sdkValue < kMinimumDelayMilliseconds ? kMinimumDelayMilliseconds : sdkValue);
    self->delay2 = sdkValue;
    self->delay1 = sdkValue;
    self->delay0 = sdkValue;
    sdkValue = Runtime_ConvertDoubleToUnsigned((double)(kFloat_100_AudioReverb * input[kFeedbackRatio]));
    sdkValue = sdkValue > kMaximumFeedbackPercent ? kMaximumFeedbackPercent : sdkValue;
    self->feedback2 = sdkValue;
    self->feedback1 = sdkValue;
    self->feedback0 = sdkValue;
    sdkValue = Runtime_ConvertDoubleToUnsigned((double)(kFloat_100_AudioReverb * input[kOutputRatio]));
    sdkValue = sdkValue > kMaximumOutputPercent ? kMaximumOutputPercent : sdkValue;
    self->level2 = sdkValue;
    self->level1 = sdkValue;
    self->level0 = sdkValue;
    // An inactive effect only stages values. An active effect temporarily replaces
    // the global AXFX allocator hooks and restores them after the synchronous update.
    if (!self->initializationAttempted)
        return 1;
    unsigned long savedAllocateHook, savedFreeHook;
    gtl_AudioEffects_InstallAllocatorHooks(&((GtlFxDelay*)self)->initializationAttempted, &savedAllocateHook,
                                           &savedFreeHook);
    int result = AXFXDelaySettings(((GtlFxDelay*)self)->delayState);
    gtl_AudioEffects_RestoreAllocatorHooks(&((GtlFxDelay*)self)->initializationAttempted, savedAllocateHook,
                                           savedFreeHook);
    return result != 0;
}

} // extern "C"
