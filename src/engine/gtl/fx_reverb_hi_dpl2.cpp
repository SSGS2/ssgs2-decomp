// Translation unit 0x8040AB50-0x8040AD68 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

struct ReverbParams {
    // Requested controls in public API order. preDelay/time use the SDK time scale;
    // this unit proves limits of 0.1/10 but not their physical units.
    // coloration/damping/crosstalk/mix are dimensionless controls clamped to [0,1].
    float preDelay, time, coloration, damping, crosstalk, mix;
};

struct FxReverbHiDpl2 {
    char baseEffectStorage[12];
    unsigned char initializationAttempted; // +0x0c: legacy name for the initialized/shutdown guard, not playback activity.
    char reservedBeforeParams[11];
    ReverbParams params; // Unclamped requested values, preserved for GetParams.
    char effect[0x178]; // SDK reverb work storage; trailing floats below are its settings.
    // SDK order differs from ReverbParams: coloration, mix, time, damping, preDelay, crosstalk.
    float coloration, mix, time, damping, preDelay, crosstalk;
};

extern "C" {
extern const float kFloat_0_1_ReverbPreDelay, kFloat_0_Reverb, kFloat_10_Reverb, kFloat_1_Reverb;
}

// Clamp a finite control to [0, high]; ordinary comparisons leave NaN unchanged.
inline float clamp(float value, float high) {
    return value > high ? high : value < kFloat_0_Reverb ? kFloat_0_Reverb : value;
}

extern "C" {
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);
void AXFXReverbHiShutdownDpl2(void*);
int AXFXReverbHiSettingsDpl2(void*);

// Return the requested reverb controls before clamping to the hardware ranges.
// Returns borrowed mutable storage valid with self; editing it alone does not reconfigure the SDK.
ReverbParams* gtl_nw4r_FxReverbHiDpl2_GetParams(FxReverbHiDpl2* self) {
    return &self->params;
}

// Shut down an active reverb while temporarily installing the effect allocator.
// Clears active before shutdown; retains the wrapper, requested controls and work storage.
void gtl_nw4r_FxReverbHiDpl2_ShutdownEffect(FxReverbHiDpl2* self) {
    if (self->initializationAttempted) {
        unsigned int previousAllocateHook, previousFreeHook;
        self->initializationAttempted = 0;
        gtl_AudioEffects_InstallAllocatorHooks(&self->initializationAttempted, &previousAllocateHook, &previousFreeHook);
        AXFXReverbHiShutdownDpl2(self->effect);
        gtl_AudioEffects_RestoreAllocatorHooks(&self->initializationAttempted, previousAllocateHook, previousFreeHook);
    }
}

// Save requested controls, clamp hardware values, and reconfigure the active effect.
// params is borrowed and copied. Inactive effects return true without an SDK call.
// An SDK failure returns false after storing both parameter sets; there is no rollback.
bool gtl_nw4r_FxReverbHiDpl2_SetParams(FxReverbHiDpl2* self, const ReverbParams* params) {
    // Preserve the requested values separately from the bounded settings used by the SDK.
    self->params = *params;
    self->preDelay = clamp(params->preDelay, kFloat_0_1_ReverbPreDelay);
    self->time = clamp(params->time, kFloat_10_Reverb);
    self->coloration = clamp(params->coloration, kFloat_1_Reverb);
    self->damping = clamp(params->damping, kFloat_1_Reverb);
    self->crosstalk = clamp(params->crosstalk, kFloat_1_Reverb);
    self->mix = clamp(params->mix, kFloat_1_Reverb);
    if (!self->initializationAttempted) return true;
    // Reconfigure under the effect's allocator, restoring both global hooks even on failure.
    unsigned int previousAllocateHook, previousFreeHook;
    gtl_AudioEffects_InstallAllocatorHooks(&self->initializationAttempted, &previousAllocateHook, &previousFreeHook);
    int result = AXFXReverbHiSettingsDpl2(self->effect);
    gtl_AudioEffects_RestoreAllocatorHooks(&self->initializationAttempted, previousAllocateHook, previousFreeHook);
    return result != 0;
}

} // extern "C"
