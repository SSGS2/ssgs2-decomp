// High-quality reverb effect: its constructor (which copies and clamps the default parameters), its SDK
// initialization, and its output buffer hooks.
//
// The work block and flags are in classes/FxReverbHiWork.h and classes/FxReverbHiFlags.h.

#include <decomp/static_init.h>

#include "classes/FxReverbHiWork.h"
#include "classes/FxReverbHiFlags.h"
#include "classes/GtlFxDelay.h"

extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_HighQualityReverb;
extern float kFloat_0_1_AudioReverb;
extern float kReverbParameterMinimum;
extern float kReverbTimeMaximum;
extern float kFloat_1_805E2364;
extern float kFloat_6_2831855;
extern unsigned int defaultHighQualityReverbParameterWords[];
extern char __vt__Q33gtl4nw4r10FxReverbHi[];
extern int g_audioEffectAllocatedBytes;

void AudioEffect_SetOutputBuffer(void*, int, unsigned int);
void AudioController_ClearOutputBuffer(void*);
int gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
int gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);
int AXFXReverbHiInit(void*);
void AXFXReverbHiSettings(void*);
void dtor_80409EB0(void*);

// Effect wrapper: +0x0C begins the allocator context, +0x18 is the raw six-float
// request block, +0x30 begins embedded AXFX work. The six floats at +0x178 are
// its SDK settings in a DIFFERENT order. Neither block is an owned pointer.
struct FxReverbHi {
    void* vtable;
    unsigned int nextEffect;
    unsigned int previousEffect;
    unsigned char active;
    unsigned char pad0D[3];
    unsigned int outputBuffer;
    unsigned int unk14; // Initialized here; no established meaning for this allocator-context word.
    float params[6];    // pre-delay, time, coloration, damping, crosstalk, mix.
    char pad30[0x148];
    float coloration;
    float mix;
    float reverbTime;
    float damping;
    float preDelay;
    float crosstalk;
};

enum ReverbParameterIndex {
    kPreDelay,
    kReverbTime,
    kColoration,
    kDamping,
    kCrosstalk,
    kMix,
    kReverbParameterCount
};

// Return the delay's initialization latch; this is not the SDK init result.
int gtl_nw4r_FxDelay_vfunc_1(GtlFxDelay* self) {
    return self->initializationAttempted != 0;
}

// Attempt SDK high-quality reverb initialization, preserving the caller's allocator hooks.
// Returns SDK success on the first call, 1 when already active. active is latched
// even if AXFXReverbHiInit returns failure; it is not a reliable success record.
int gtl_nw4r_FxReverbHi_Initialize(FxReverbHi* self) {
    if (self->active)
        return 1;
    unsigned int savedAllocateHook, savedFreeHook;
    gtl_AudioEffects_InstallAllocatorHooks(&self->active, &savedAllocateHook, &savedFreeHook);
    g_audioEffectAllocatedBytes = 0;
    int result = AXFXReverbHiInit(((FxReverbHiWork*)self)->sdkWork);
    gtl_AudioEffects_RestoreAllocatorHooks(&self->active, savedAllocateHook, savedFreeHook);
    self->active = 1;
    return result != 0;
}

// Initialize the effect and clamp its six default parameters into the SDK state.
// Returns self; does not initialize AXFX allocations. Raw params remain unchanged.
// Raw order: pre-delay/time/coloration/damping/crosstalk/mix; SDK order below:
// coloration/mix/time/damping/pre-delay/crosstalk. Time fields pass through in
// SDK units (not established here); the other four are unitless [0,1] controls.
FxReverbHi* gtl_nw4r_FxReverbHi_Ctor(FxReverbHi* self) {
    // Start detached and inactive, with no output buffer or allocator bookkeeping.
    self->nextEffect = 0;
    self->previousEffect = 0;
    self->vtable = __vt__Q33gtl4nw4r10FxReverbHi;
    self->active = 0;
    self->outputBuffer = 0;
    self->unk14 = 0;
    // Copy defaults as words to preserve their exact float representations.
    unsigned int words[kReverbParameterCount];
    for (int i = 0; i < kReverbParameterCount; ++i)
        words[i] = defaultHighQualityReverbParameterWords[i];
    float* params = self->params;
    for (int i = 0; i < kReverbParameterCount; ++i)
        ((unsigned int*)params)[i] = words[i];
    // Map the raw block into SDK fields: pre-delay [0,0.1], time [0,10], others [0,1].
    float parameterValue = self->params[kPreDelay];
    if (parameterValue > kFloat_0_1_AudioReverb)
        parameterValue = kFloat_0_1_AudioReverb;
    else if (parameterValue < kReverbParameterMinimum)
        parameterValue = kReverbParameterMinimum;
    self->preDelay = parameterValue;
    parameterValue = self->params[kReverbTime];
    if (parameterValue > kReverbTimeMaximum)
        parameterValue = kReverbTimeMaximum;
    else if (parameterValue < kReverbParameterMinimum)
        parameterValue = kReverbParameterMinimum;
    self->reverbTime = parameterValue;
    parameterValue = self->params[kColoration];
    if (parameterValue > kFloat_1_805E2364)
        parameterValue = kFloat_1_805E2364;
    else if (parameterValue < kReverbParameterMinimum)
        parameterValue = kReverbParameterMinimum;
    self->coloration = parameterValue;
    parameterValue = self->params[kDamping];
    if (parameterValue > kFloat_1_805E2364)
        parameterValue = kFloat_1_805E2364;
    else if (parameterValue < kReverbParameterMinimum)
        parameterValue = kReverbParameterMinimum;
    self->damping = parameterValue;
    parameterValue = self->params[kCrosstalk];
    if (parameterValue > kFloat_1_805E2364)
        parameterValue = kFloat_1_805E2364;
    else if (parameterValue < kReverbParameterMinimum)
        parameterValue = kReverbParameterMinimum;
    self->crosstalk = parameterValue;
    parameterValue = self->params[kMix];
    if (parameterValue > kFloat_1_805E2364)
        parameterValue = kFloat_1_805E2364;
    else if (parameterValue < kReverbParameterMinimum)
        parameterValue = kReverbParameterMinimum;
    self->mix = parameterValue;
    // Keep the existing settings-update path; construction has already cleared active.
    if (self->active) {
        unsigned int savedAllocateHook, savedFreeHook;
        gtl_AudioEffects_InstallAllocatorHooks(&self->active, &savedAllocateHook, &savedFreeHook);
        AXFXReverbHiSettings(((FxReverbHiWork*)self)->sdkWork);
        gtl_AudioEffects_RestoreAllocatorHooks(&self->active, savedAllocateHook, savedFreeHook);
    }
    return self;
}

// Attach the output buffer through the embedded effect allocator context.
// The buffer is supplied externally. forwardedArgument passes through unchanged;
// its meaning is not established by this wrapper.
void gtl_nw4r_FxReverbHi_SetOutputBuffer(void* self, int forwardedArgument, unsigned int outputBuffer) {
    AudioEffect_SetOutputBuffer(&AsFxReverbHiFlags(self)->initializationAttempted, forwardedArgument, outputBuffer);
}

// Clear the effect's externally supplied output buffer.
void gtl_nw4r_FxReverbHi_ClearOutputBuffer(void* self) {
    AudioController_ClearOutputBuffer(&AsFxReverbHiFlags(self)->initializationAttempted);
}

// Initialize this module's full-turn angle constant.
void AudioReverbHi_InitializeAngleConstant() {
    s_twoPi_HighQualityReverb = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626D0", _ctors_u8040A548_0, AudioReverbHi_InitializeAngleConstant);
#pragma pop
