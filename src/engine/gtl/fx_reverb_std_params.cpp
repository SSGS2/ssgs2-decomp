// Standard reverb effect settings: keeps the requested values for GetParams, clamps a
// separate copy for the SDK and pushes changes to an active effect.

#include "classes/FxReverbStdWork.h"

extern "C" {

// Public parameter order: pre-delay, decay time, coloration, damping, wet level.
// Time units are not established here; the last three SDK controls are clamped to [0,1].
// Requested values are preserved verbatim; only the SDK-facing fields below are clamped.
struct Params {
    float preDelay, decayTime, coloration, damping, wetLevel;
};
struct Object {
    char effectBase[12];
    unsigned char active;
    char allocatorHookState[11];
    Params params;
    char sdkWork[0x114 - 0x2C];
    float coloration, wetLevel, decayTime, damping, preDelay;
};

void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
int AudioReverbStd_ApplySettings(void*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);
extern const float kFloat_0_1_Reverb, kFloat_0_Color, kFloat_10_Reverb_805E23D8, kFloat_1_ReverbClamp;

// Keep the requested parameters for GetParams; clamp the SDK copy and update an active effect.
// requested is borrowed and must be non-null. Inactive effects return true after storing settings;
// active effects return whether the SDK accepted them. Failure does not roll back either copy.
bool gtl_nw4r_FxReverbStd_SetParams(Object* self, const Params* requested) {
    // Snapshot the public settings before producing the independently clamped SDK copy.
    float wetLevel, damping, coloration, decayTime, preDelay;
    preDelay = requested->preDelay;
    float maxPreDelay = kFloat_0_1_Reverb;
    decayTime = requested->decayTime;
    coloration = requested->coloration;
    damping = requested->damping;
    wetLevel = requested->wetLevel;
    self->params.preDelay = preDelay;
    self->params.decayTime = decayTime;
    self->params.coloration = coloration;
    self->params.damping = damping;
    self->params.wetLevel = wetLevel;
    // SDK limits: pre-delay 0..0.1, decay 0..10; coloration, damping and wet level 0..1.
    // Preserve the ordered comparisons: replacing them with a generic clamp can affect NaNs.
    float clampedPreDelay = preDelay > maxPreDelay      ? maxPreDelay
                            : preDelay < kFloat_0_Color ? kFloat_0_Color
                                                        : preDelay;
    float requestedDecayTime = requested->decayTime;
    float maxDecayTime = kFloat_10_Reverb_805E23D8;
    self->preDelay = clampedPreDelay;
    float clampedDecayTime = requestedDecayTime > maxDecayTime     ? maxDecayTime
                             : requestedDecayTime < kFloat_0_Color ? kFloat_0_Color
                                                                   : requestedDecayTime;
    float requestedColoration = requested->coloration;
    float maxColoration = kFloat_1_ReverbClamp;
    self->decayTime = clampedDecayTime;
    float clampedColoration = requestedColoration > maxColoration    ? maxColoration
                              : requestedColoration < kFloat_0_Color ? kFloat_0_Color
                                                                     : requestedColoration;
    float requestedDamping = requested->damping;
    float maxDamping = kFloat_1_ReverbClamp;
    self->coloration = clampedColoration;
    float clampedDamping = requestedDamping > maxDamping       ? maxDamping
                           : requestedDamping < kFloat_0_Color ? kFloat_0_Color
                                                               : requestedDamping;
    float requestedWetLevel = requested->wetLevel;
    float maxWetLevel = kFloat_1_ReverbClamp;
    self->damping = clampedDamping;
    self->wetLevel = requestedWetLevel > maxWetLevel      ? maxWetLevel
                     : requestedWetLevel < kFloat_0_Color ? kFloat_0_Color
                                                          : requestedWetLevel;
    // Install this effect's allocation hooks only around the active SDK update, then restore them.
    if (!self->active)
        return true;
    unsigned int savedAllocator, savedDeallocator;
    gtl_AudioEffects_InstallAllocatorHooks(&self->active, &savedAllocator, &savedDeallocator);
    int result = AudioReverbStd_ApplySettings(((FxReverbStdWork*)self)->sdkWork);
    gtl_AudioEffects_RestoreAllocatorHooks(&self->active, savedAllocator, savedDeallocator);
    return result != 0;
}

// Return the original requested settings, which may exceed the SDK's clamp limits.
// Borrowed mutable pointer, valid for the object's lifetime; writing through it alone does not apply settings.
Params* gtl_nw4r_FxReverbStd_GetParams(Object* self) {
    return &self->params;
}

} // extern "C"
