// Delay and chorus audio effects: the delay output buffer, its setup from the default
// parameters, and the chorus initialization check.
//
// It also sets the full-circle angle constant used by the audio effects at startup.

#include <decomp/static_init.h>
#include "classes/GtlFxChorus.h"
#define AsGtlFxChorus(p) ((GtlFxChorus*)(p))
#include "classes/GtlFxDelay.h"
#define AsGtlFxDelay(p) ((GtlFxDelay*)(p))

extern "C" {

// Parameter order/units: delay in milliseconds, feedback and output as ratios
// converted to integer percent below. Each value is copied to SDK lanes 0/1/2;
// this unit does not establish the speaker identity of each lane.
struct DelayDefaults {
    float v[3];
};
enum DelayParameter {
    kDelayMilliseconds,
    kFeedbackRatio,
    kOutputRatio
};
static const unsigned int kMinimumDelayMilliseconds = 1;
static const unsigned int kMaximumDelayMilliseconds = 5000;
static const unsigned int kMaximumFeedbackPercent = 99;
static const unsigned int kMaximumOutputPercent = 100;

#include "sdk_protos.h"

extern void AudioEffect_SetOutputBuffer(void*, int, unsigned int);
extern void AudioController_ClearOutputBuffer(void*);
extern float kFloat_6_2831855;
extern float s_audioDelayTwoPi;
extern const float kFloat_100_AudioReverb;
extern const DelayDefaults s_audioDelayDefaultParameters;
extern unsigned int __vt__Q33gtl4nw4r7FxDelay[];
extern void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
extern void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);
extern unsigned int AXFXDelayInit(void*);
extern void AXFXDelaySettings(void*);
extern unsigned int g_audioEffectAllocatedBytes;

// Forward output-buffer settings to the embedded audio-effect controller.
// buffer is a borrowed address encoded in 32 bits; bufferIndex is forwarded as-is.
void gtl_nw4r_FxDelay_SetOutputBuffer(void* self, int bufferIndex, unsigned int buffer) {
    AudioEffect_SetOutputBuffer(&AsGtlFxDelay(self)->initializationAttempted, bufferIndex, buffer);
}

// Detach the delay controller's output buffer.
void gtl_nw4r_FxDelay_ShutdownOutput(void* self) {
    AudioController_ClearOutputBuffer(&AsGtlFxDelay(self)->initializationAttempted);
}

// Initialize this effect module's cached full-circle angle at startup.
void AudioDelay_InitAngleConstant() {
    s_audioDelayTwoPi = kFloat_6_2831855;
}

// Report whether the chorus SDK state has been initialized.
int gtl_nw4r_FxChorus_vfunc_1(void* self) {
    return AsGtlFxChorus(self)->initializationAttempted != 0;
}

// Copy defaults and clamp all three delay channels to SDK parameter limits.
// self is caller-provided storage and is returned; this does not allocate SDK work.
void* gtl_nw4r_FxDelay_Ctor(void* self) {
    // Phase 1: establish the effect/controller and keep the public float defaults.
    DelayDefaults values = *(DelayDefaults*)&s_audioDelayDefaultParameters;
    AsGtlFxDelay(self)->unk_4 = 0;
    AsGtlFxDelay(self)->unk_8 = 0;
    *(unsigned int*)self = (unsigned int)__vt__Q33gtl4nw4r7FxDelay;
    AsGtlFxDelay(self)->initializationAttempted = 0;
    AsGtlFxDelay(self)->outputBuffer = 0;
    AsGtlFxDelay(self)->unk_14 = 0; // Clear the remaining embedded controller word before applying delay defaults.
    *(DelayDefaults*)AsGtlFxDelay(self)->parameters = values;
    // Phase 2: derive SDK integers: 1..5000 ms, feedback <=99%, output <=100%.
    // Conversion to unsigned precedes clamping; preserve that order for negatives.
    unsigned int delay = Runtime_ConvertDoubleToUnsigned((double)values.v[kDelayMilliseconds]);
    if (delay > kMaximumDelayMilliseconds)
        delay = kMaximumDelayMilliseconds;
    else if (delay < kMinimumDelayMilliseconds)
        delay = kMinimumDelayMilliseconds;
    AsGtlFxDelay(self)->delay2 = delay;
    AsGtlFxDelay(self)->delay1 = delay;
    AsGtlFxDelay(self)->delay0 = delay;
    unsigned int feedback =
        Runtime_ConvertDoubleToUnsigned((double)(kFloat_100_AudioReverb * values.v[kFeedbackRatio]));
    if (feedback > kMaximumFeedbackPercent)
        feedback = kMaximumFeedbackPercent;
    AsGtlFxDelay(self)->feedback2 = feedback;
    AsGtlFxDelay(self)->feedback1 = feedback;
    AsGtlFxDelay(self)->feedback0 = feedback;
    unsigned int level = Runtime_ConvertDoubleToUnsigned((double)(kFloat_100_AudioReverb * values.v[kOutputRatio]));
    if (level > kMaximumOutputPercent)
        level = kMaximumOutputPercent;
    AsGtlFxDelay(self)->level2 = level;
    AsGtlFxDelay(self)->level1 = level;
    AsGtlFxDelay(self)->level0 = level;
    // Phase 3: reapply settings if the controller reports initialization. The plain
    // store above clears that flag, but retain this original conditional SDK path.
    if (AsGtlFxDelay(self)->initializationAttempted) {
        unsigned int savedFreeHook, savedAllocateHook;
        gtl_AudioEffects_InstallAllocatorHooks(&AsGtlFxDelay(self)->initializationAttempted, &savedAllocateHook,
                                               &savedFreeHook);
        AXFXDelaySettings(AsGtlFxDelay(self)->delayState);
        gtl_AudioEffects_RestoreAllocatorHooks(&AsGtlFxDelay(self)->initializationAttempted, savedAllocateHook,
                                               savedFreeHook);
    }
    return self;
}

// Initialize the SDK delay under temporary allocator hooks, preserving its status result.
// Returns SDK success on the first attempt, then 1 while the flag remains set.
// Shutdown clears the flag. It is set even after SDK failure: it records an
// attempt, not guaranteed success.
int gtl_nw4r_FxDelay_InitializeOnce(void* self) {
    void* controller = &AsGtlFxDelay(self)->initializationAttempted;
    if (AsGtlFxDelay(self)->initializationAttempted)
        return 1;
    // Hook state is process-wide; restore both hooks before publishing the flag.
    unsigned int savedAllocateHook, savedFreeHook;
    gtl_AudioEffects_InstallAllocatorHooks(controller, &savedAllocateHook, &savedFreeHook);
    g_audioEffectAllocatedBytes = 0;
    unsigned int ok = AXFXDelayInit(AsGtlFxDelay(self)->delayState);
    gtl_AudioEffects_RestoreAllocatorHooks(controller, savedAllocateHook, savedFreeHook);
    AsGtlFxDelay(self)->initializationAttempted = 1;
    return ok != 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626CC", _ctors_u8040A174_0, AudioDelay_InitAngleConstant);
#pragma pop
