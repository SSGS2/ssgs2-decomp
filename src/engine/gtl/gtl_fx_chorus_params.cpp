// Translation unit 0x80409FEC-0x8040A124 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Public parameter order: delay in milliseconds, depth as a fraction of delay,
// modulation rate in hertz. SetParams stores these RAW floats before SDK clamping.
// The SDK receives integer delay/depth in milliseconds and period in milliseconds.
struct ChorusParameters {
    float delayMilliseconds;
    float depthRatio;
    float rateHertz;
};

#include "sdk_protos.h"
#include "classes/GtlFxChorus.h"

enum {
    kMinimumChorusDelay = 1,
    kMaximumChorusDelay = 50,
    kMaximumChorusDepth = 50,
    kMinimumChorusPeriod = 500,
    kMaximumChorusPeriod = 10000,
    kChorusAllocatorContextOffset = (int)&((GtlFxChorus*)0)->initializationAttempted
};

extern "C" {
unsigned long Runtime_ConvertDoubleToUnsigned(double x);
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned long*, unsigned long*);
int Chorus_ReconfigureAxfxState(void*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned long, unsigned long);
extern float kFloat_1000_FxChorus;

// Return the three contiguous delay/depth/rate parameters.
// Borrowed mutable view, valid for the effect's lifetime; writing it directly does
// not rebuild the SDK state. Use SetParams to publish changed values to playback.
void* gtl_nw4r_FxChorus_GetParams(void* p) {
    return &((GtlFxChorus*)p)->delayParameter;
}

#pragma opt_lifetimes off
// Clamp AXFX chorus values and reconfigure a running effect under its allocator hooks.
// param is borrowed and must point to three floats. Positive finite delay/rate
// and nonnegative finite depth avoid relying on out-of-range unsigned conversion;
// this function performs conversion BEFORE clamping and adds no validation.
// Returns 1 without SDK work if uninitialized, else the SDK success predicate.
// initialized is an attempt latch, not proof of SDK success: gtl_voice_pool_basic_player_chorus.cpp sets it
// even when AXFXChorusInit fails. A set latch permits the reconfigure call here.
int gtl_nw4r_FxChorus_SetParams(GtlFxChorus* self, const ChorusParameters* param) {
    unsigned long depth;
    unsigned long delay;
    unsigned long period;
    // Preserve the caller's requested values for GetParams, independently of clamps.
    self->delayParameter = param->delayMilliseconds;
    self->depthRatio = param->depthRatio;
    self->rateParameter = param->rateHertz;
    // depth_ms = delay_ms * depth_ratio; period_ms = 1000 / rate_hz.
    // Convert/truncate to unsigned SDK words, then clamp delay [1,50], depth <=50,
    // period [500,10000]. Depth has no extra lower clamp after conversion.
    delay = Runtime_ConvertDoubleToUnsigned(param->delayMilliseconds);
    depth = Runtime_ConvertDoubleToUnsigned(param->delayMilliseconds * param->depthRatio);
    period = Runtime_ConvertDoubleToUnsigned(kFloat_1000_FxChorus / param->rateHertz);
    self->clampedDelay = delay > kMaximumChorusDelay ? kMaximumChorusDelay : (delay < kMinimumChorusDelay ? kMinimumChorusDelay : delay);
    self->clampedDepth = depth > kMaximumChorusDepth ? kMaximumChorusDepth : depth;
    self->clampedPeriod = period > kMaximumChorusPeriod ? kMaximumChorusPeriod : (period < kMinimumChorusPeriod ? kMinimumChorusPeriod : period);
    // A stopped effect only caches settings. A running one uses temporary allocator hooks.
    if (!self->initializationAttempted)
        return 1;
    unsigned long savedAllocateHook;
    unsigned long savedFreeHook;
    // Keep a byte-address expression here: direct member addressing changes register allocation.
    gtl_AudioEffects_InstallAllocatorHooks((char*)self + kChorusAllocatorContextOffset, &savedAllocateHook, &savedFreeHook);
    int state = Chorus_ReconfigureAxfxState(self->chorusState);
    gtl_AudioEffects_RestoreAllocatorHooks((char*)self + kChorusAllocatorContextOffset, savedAllocateHook, savedFreeHook);
    return state != 0;
}
#pragma opt_lifetimes reset

} // extern "C"
