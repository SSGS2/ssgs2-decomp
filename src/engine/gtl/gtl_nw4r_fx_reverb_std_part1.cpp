#include <decomp/static_init.h>
#include "iff/IffStruct.h"
// Translation unit 0x8040AD68-0x8040B02C (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void AudioEffect_SetOutputBuffer(void*, int, unsigned int);
void AudioController_ClearOutputBuffer(void*);
extern float s_twoPi_AudioReverb;
extern const float kFloat_6_2831855;
extern int g_audioEffectAllocatedBytes;
extern void* __vt__Q33gtl4nw4r11FxReverbStd[];
extern const float kFloat_0_1_Reverb;
extern const float kFloat_0_Color;
extern const float kFloat_10_Reverb_805E23D8;
extern const float kFloat_1_ReverbClamp;
// Raw requested settings in this order; the separate trailing scalars are clamped DSP settings.
// predelay/time use the AXFX time scale (0..0.1 and 0..10); the other values use 0..1.
// The precise time unit is not established by this wrapper.
struct ReverbParams { float predelay, time, coloration, damping, mix; };
extern ReverbParams defaultStandardReverbParameters;
void AudioReverbStd_ApplySettings(void*);
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);

struct ReverbStd {
    void* vtable;
    unsigned int effectListLinks[2]; // AudioController_AttachEffect links the node at +4.
    unsigned char active;
    char pad0D[3];
    unsigned int outputBuffer; // AudioEffect_SetOutputBuffer receives &active, then writes +4.
    unsigned int reservedContext;
    ReverbParams params;
    char state[0xE8];
    float coloration, mix, time, damping, predelay;
};

// Clamp one DSP setting to [0, max], retaining the original ordered comparisons (including NaN).
inline float clampReverb(float value, float max) {
    return value > max ? max : value < kFloat_0_Color ? kFloat_0_Color : value;
}

// Initialize standard reverb defaults and clamp each DSP parameter to its supported range.
ReverbStd* gtl_nw4r_FxReverbStd_Ctor(ReverbStd* self) {
    // Phase 1: initialize detached effect links and controller state, then save raw defaults.
    self->effectListLinks[0] = 0;
    self->effectListLinks[1] = 0;
    self->vtable = __vt__Q33gtl4nw4r11FxReverbStd;
    self->active = 0;
    self->outputBuffer = 0;
    self->reservedContext = 0;
    ReverbParams params = defaultStandardReverbParameters;
    self->params.predelay = params.predelay;
    self->params.time = params.time;
    self->params.coloration = params.coloration;
    self->params.damping = params.damping;
    self->params.mix = params.mix;
    // Phase 2: publish bounded working values without modifying the raw parameter record.
    self->predelay = clampReverb(params.predelay, kFloat_0_1_Reverb);
    self->time = clampReverb(params.time, kFloat_10_Reverb_805E23D8);
    self->coloration = clampReverb(params.coloration, kFloat_1_ReverbClamp);
    self->damping = clampReverb(params.damping, kFloat_1_ReverbClamp);
    self->mix = clampReverb(params.mix, kFloat_1_ReverbClamp);
    // Phase 3: apply settings only for an active DSP. The constructor initialized active to zero.
    if (self->active) {
        unsigned int savedFreeHook, savedAllocateHook;
        gtl_AudioEffects_InstallAllocatorHooks(&self->active, &savedAllocateHook, &savedFreeHook);
        AudioReverbStd_ApplySettings(self->state);
        gtl_AudioEffects_RestoreAllocatorHooks(&self->active, savedAllocateHook, savedFreeHook);
    }
    return self;
}

struct ReverbBuffer {
    unsigned int words[4]; // Four borrowed channel-buffer addresses, not four sample values.
};
struct ReverbState {
    char effectBase[0xC];
    unsigned char active;
    char reserved[0x23];
    char dspState[1]; // 0x30: high-quality DPL2 reverb state, distinct from standard state.
};
void AXFXReverbHiCallbackDpl2(ReverbBuffer*, void*);
typedef char ReverbStd_dsp_offset[(IFF_OFFSET(ReverbStd, state) == 0x2C) ? 1 : -1];
typedef char ReverbDpl2_dsp_offset[(IFF_OFFSET(ReverbState, dspState) == 0x30) ? 1 : -1];
int AudioReverbStd_InitializeDspState(void*); // Initialize the AXFX standard-reverb DSP state.
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);

// Report whether the DPL2 reverb processor is active.
bool gtl_nw4r_FxReverbHiDpl2_vfunc_1(ReverbState* self) {
    return self->active != 0;
}

// Process four channel-buffer addresses through the active DPL2 reverb.
// The address bundle is copied for the SDK; the buffers it points to are processed in place.
// Lane ordering and sample count are defined by the caller/SDK, not by this wrapper.
void gtl_nw4r_FxReverbHiDpl2_ProcessChannelBuffers(ReverbState* self, void*, ReverbBuffer* buffer) {
    if (self->active) {
        ReverbBuffer local;
        local.words[0] = buffer->words[0];
        local.words[1] = buffer->words[1];
        local.words[2] = buffer->words[2];
        local.words[3] = buffer->words[3];
        AXFXReverbHiCallbackDpl2(&local, self->dspState);
    }
}

// Initialize the standard DSP state under the effect allocator hooks and mark it active.
// Returns the SDK initialization result as bool (or true if already active).
// Existing behavior sets active even when initialization fails; preserve that ordering.
bool gtl_nw4r_FxReverbStd_Initialize(ReverbState* self) {
    if (self->active) return true;
    unsigned int savedAllocateHook;
    unsigned int savedFreeHook;
    gtl_AudioEffects_InstallAllocatorHooks(&self->active, &savedAllocateHook, &savedFreeHook);
    g_audioEffectAllocatedBytes = 0;
    int result = AudioReverbStd_InitializeDspState(((ReverbStd*)self)->state);
    gtl_AudioEffects_RestoreAllocatorHooks(&self->active, savedAllocateHook, savedFreeHook);
    self->active = 1;
    return result != 0;
}

// Store a borrowed output-buffer address; AudioEffect_SetOutputBuffer ignores its middle argument.
void gtl_nw4r_FxReverbStd_SetOutputBuffer(void* self, int unusedBufferArgument, unsigned int outputBuffer) {
    AudioEffect_SetOutputBuffer(&((ReverbStd*)self)->active, unusedBufferArgument, outputBuffer);
}

// Clear this effect's output buffer.
void gtl_nw4r_FxReverbStd_ClearOutputBuffer(void* self) {
    AudioController_ClearOutputBuffer(&((ReverbStd*)self)->active);
}

// Initialize the audio-effect angle constant to one full turn.
void AudioReverb_InitializeAngleConstant() {
    s_twoPi_AudioReverb = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626D8", _ctors_u8040AD68_0, AudioReverb_InitializeAngleConstant);
#pragma pop
