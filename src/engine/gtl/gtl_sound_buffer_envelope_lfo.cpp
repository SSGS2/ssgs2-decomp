#include <decomp/static_init.h>
#include "ppc_double_words.h"
#include "classes/GtlSoundBuffer.h"
// Translation unit 0x803FF5B8-0x80400B80 (26 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern float kFloat_6_2831855;
extern float s_audioEnvelopeTwoPi;
extern float s_audioEnvelopeResetTwoPi;
extern float s_audioLfoTwoPi;
extern float kFloat_1_AudioEnvelopeLfo;
// LFO phase divisor: 43690.
extern float kAudioLfoPhaseCountsPerCycle;
// Zero threshold selecting the positive/negative sine scale.
extern float kFloat_0_AudioLfoSine;
// Unsigned-integer-to-double conversion bias: 2^52.
extern double kDouble_4503599627370496_AudioLfoPhase;
// Signed-integer-to-double conversion bias: 2^52 + 2^31.
extern double kDouble_4503601774854144_AudioLfoDepth;
extern float kFloat_0_5_AudioEnvelopeLfo;
// LFO depth unit maximum: 127.
extern float kAudioLfoDepthUnitMax;
extern double kAudioScaleUnsignedConversionBias;
extern float kAudioScaleUnitsPerUnity;
extern float kFloat_1_audioEnvelope;
extern float kFloat_0_audioEnvelope;
extern float kFloat_0_soundBuffer;
extern float kFloat_0_AudioResource;
extern float kSoundBufferUnity;
extern float kFloat_0_SpatialAudio;
extern float kFloat_0_SoundBuffers;
extern float kFloat_1_AudioSpatial;
// Envelope sustain-level unit maximum: 255.
extern float kFloat_255_AudioEnvelopeSustain;
// Unsigned-integer-to-double conversion bias: 2^52.
extern double kDouble_4503599627370496_AudioEnvelope;

int GtlVoice_Stop(void*);
void GtlVoice_Release(void*);
void Vector_CopyComponents(void*, const void*);
void __dl__FPv(void*);
#include "sdk_protos.h"
float Math_SinApprox(float);
void StrmPlayer_SetPauseFlag(void*, int);

enum EnvelopeMode {
    kEnvelopeDisabled = 0, kEnvelopeFull = 1, kEnvelopeAttackOnly = 2, kEnvelopeHoldFirst = 3
};
enum EnvelopeStage {
    kEnvelopeBegin = 0, kEnvelopeDelay = 1, kEnvelopeAttack = 2, kEnvelopeHoldBegin = 3,
    kEnvelopeHold = 4, kEnvelopeDecay = 5, kEnvelopeSustain = 6, kEnvelopeRelease = 7
};
enum SoundStopMode { kStopImmediately = 0, kStopWithRelease = 1, kStopLooping = 2 };
enum SoundWritableFlags { kUseListener = 0x10, kPitchWritable = 0x20, kVolumeWritable = 0x80 };
enum SoundSpatialPolicy { kSpatialModeShift = 17, kSpatialModeDisabled = 2 };
enum AudioScaleUnits { kDefaultScaleUnits = 32000 };
enum LfoPhaseCounts {
    // One LFO cycle is 43690 (0xAAAA) phase counts; the wrap subtracts one cycle as (0x10000 - 0x5556).
    kLfoPhaseLimit = 0xAAAA, kLfoPhaseWrap = 0xAAAB, kLfoPhaseModulus = 0x10000, kLfoPhaseWrapBias = 0x5556
};

struct InterruptState {
    int level;
    // Restoration is explicit at each exit; this destructor does not unlock.
    ~InterruptState() {}
};
void InterruptGuard_dtor(InterruptState*, int);

// The final two settings are copied verbatim; their consumers are outside this unit.
struct EnvelopeSettings {
    void* configTag;
    int mode;
    unsigned int delayTicks;
    unsigned int attackTicks;
    unsigned int holdTicks;
    unsigned int decayTicks;
    unsigned int releaseTicks;
    unsigned int sustainUnits;
    unsigned int reservedConfig0;
    unsigned int reservedConfig1;
};
struct EnvelopeState {
    int state;
    float level;
    float releaseStartLevel;
    float sustainLevel;
    unsigned int remainingTicks;
    unsigned int configTag;
    int mode;
    unsigned int delayTicks;
    unsigned int attackTicks;
    unsigned int holdTicks;
    unsigned int decayTicks;
    unsigned int releaseTicks;
    unsigned int sustainUnits;
    unsigned int reservedConfig0;
    unsigned int reservedConfig1;
};

// Copy the envelope settings and select the starting ADSR stage and sustain level.
int gtl_AudioEnvelope_Initialize(EnvelopeState* out, EnvelopeSettings* source) {
    int mode = source->mode;
    unsigned int sustainUnits = source->sustainUnits;
    out->configTag = (unsigned int)source->configTag;
    out->mode = mode;
    out->delayTicks = source->delayTicks;
    out->attackTicks = source->attackTicks;
    out->holdTicks = source->holdTicks;
    out->decayTicks = source->decayTicks;
    out->releaseTicks = source->releaseTicks;
    out->sustainUnits = sustainUnits;
    out->reservedConfig0 = source->reservedConfig0;
    out->reservedConfig1 = source->reservedConfig1;
    union { PpcDoubleWords words; double d; } conv;
    conv.words.hi = 0x43300000;
    conv.words.lo = sustainUnits;
    float sustainFloat = (float)(conv.d - kDouble_4503599627370496_AudioEnvelope);
    out->sustainLevel = sustainFloat / kFloat_255_AudioEnvelopeSustain;
    out->remainingTicks = 0;
    switch (mode) {
    case kEnvelopeDisabled: out->state = kEnvelopeSustain; out->level = kFloat_1_audioEnvelope; out->releaseStartLevel = kFloat_1_audioEnvelope; break;
    case kEnvelopeFull: out->state = kEnvelopeBegin; out->level = kFloat_0_audioEnvelope; out->releaseStartLevel = kFloat_0_audioEnvelope; break;
    case kEnvelopeAttackOnly: out->state = kEnvelopeAttack; out->level = kFloat_0_audioEnvelope; out->releaseStartLevel = kFloat_0_audioEnvelope; break;
    case kEnvelopeHoldFirst: out->state = kEnvelopeHoldBegin; out->level = kFloat_1_audioEnvelope; out->releaseStartLevel = kFloat_1_audioEnvelope; break;
    }
    return 1;
}

struct SoundBufferStopHandle { char padding000[8]; void* state; };
struct SoundBufferStopState {
    char padding000[0x9C]; void* resource;
    char padding0A0[0x74]; int envelopeStage; float envelopeLevel;
    float releaseStartLevel; char padding120[0x74]; int loopRequested;
};
void gtl_AudioEnvelope_Update(EnvelopeState*, unsigned int);

// Release immediately, enter the envelope release stage, or disable looping under the interrupt lock.
int gtl_ISoundBuffer_Stop(SoundBufferStopHandle* self, int mode) {
    SoundBufferStopState* state;
    InterruptState guard;
    guard.level = OSDisableInterrupts();
    state = (SoundBufferStopState*)self->state;
    if (state->resource == 0) {
        OSRestoreInterrupts(guard.level);
        return 0;
    }
    if (mode == kStopWithRelease) {
        state->releaseStartLevel = state->envelopeLevel;
        state->envelopeStage = kEnvelopeRelease;
        OSRestoreInterrupts(guard.level);
        return 1;
    }
    if (mode == kStopLooping) {
        state->loopRequested = 0;
        OSRestoreInterrupts(guard.level);
        return 1;
    }
    {
        InterruptState guard2;
        guard2.level = OSDisableInterrupts();
        if (state->resource != 0) GtlVoice_Stop(state->resource);
        OSRestoreInterrupts(guard2.level);
    }
    GtlVoice_Release(state->resource);
    state->resource = 0;
    OSRestoreInterrupts(guard.level);
    return 1;
}

// Advance delay, attack, hold, decay, sustain or release by elapsed ticks; stages may fall through.
// delta and all *Ticks settings use the caller's tick domain, not seconds or samples.
// Each reached stage receives the FULL delta, even when a previous stage ended
// partway through it. No overshoot is subtracted at delay/hold/attack boundaries.
// Zero-length stages fall through immediately; sustain clamps any decay overshoot.
void gtl_AudioEnvelope_Update(EnvelopeState* self, unsigned int delta) {
    if (self->mode == 0) return;
    switch (self->state) {
    case kEnvelopeBegin:
        self->state = kEnvelopeDelay; self->level = kFloat_0_audioEnvelope; self->remainingTicks = self->delayTicks;
        // fall through
    case kEnvelopeDelay:
        self->level = kFloat_0_audioEnvelope;
        if (self->delayTicks == 0 || self->remainingTicks <= delta) self->state = kEnvelopeAttack;
        else { self->remainingTicks -= delta; return; }
        // fall through
    case kEnvelopeAttack:
        if (self->attackTicks == 0) self->state = kEnvelopeHoldBegin;
        else {
            self->level += (float)delta / (float)self->attackTicks;
            if (!(self->level >= kFloat_1_audioEnvelope)) return;
            self->state = kEnvelopeHoldBegin;
        }
        // fall through
    case kEnvelopeHoldBegin:
        self->level = kFloat_1_audioEnvelope; self->state = kEnvelopeHold; self->remainingTicks = self->holdTicks;
        // fall through
    case kEnvelopeHold:
        self->level = kFloat_1_audioEnvelope;
        if (self->holdTicks == 0 || self->remainingTicks <= delta) self->state = kEnvelopeDecay;
        else { self->remainingTicks -= delta; return; }
        // fall through
    case kEnvelopeDecay:
        if (self->decayTicks == 0 || self->level <= self->sustainLevel) self->state = kEnvelopeSustain;
        else {
            self->level += (float)delta * (self->sustainLevel - kFloat_1_audioEnvelope) / (float)self->decayTicks;
            if (!(self->level <= self->sustainLevel)) return;
            self->state = kEnvelopeSustain;
        }
        // fall through
    case kEnvelopeSustain: self->level = self->sustainLevel; return;
    case kEnvelopeRelease:
        if (self->releaseTicks == 0 || self->level <= kFloat_0_audioEnvelope) self->level = kFloat_0_audioEnvelope;
        else {
            self->level += (float)delta * -self->releaseStartLevel / (float)self->releaseTicks;
            if (self->level <= kFloat_0_audioEnvelope) self->level = kFloat_0_audioEnvelope;
        }
        return;
    }
}

struct SoundBufferPauseHandle { char padding000[8]; void* state; };

// Pause the active playback voice; report failure when no voice is attached.
int gtl_ISoundBuffer_Pause(SoundBufferPauseHandle* self) {
    void* state = self->state;
    InterruptState interruptState;
    interruptState.level = OSDisableInterrupts();
    void* value = ((SoundBufferData*)state)->activeVoice; // 0x804004A0: active playback resource +0x9C
    if (value == 0) {
        OSRestoreInterrupts(interruptState.level);
        return 0;
    }
    StrmPlayer_SetPauseFlag(value, 1);
    OSRestoreInterrupts(interruptState.level);
    return 1;
}

// Resume the active playback voice; report failure when no voice is attached.
int gtl_ISoundBuffer_Resume(SoundBufferPauseHandle* self) {
    void* state = self->state;
    InterruptState interruptState;
    interruptState.level = OSDisableInterrupts();
    void* value = ((SoundBufferData*)state)->activeVoice; // 0x8040050C: active playback resource +0x9C
    if (value == 0) {
        OSRestoreInterrupts(interruptState.level);
        return 0;
    }
    StrmPlayer_SetPauseFlag(value, 0);
    OSRestoreInterrupts(interruptState.level);
    return 1;
}

struct SoundBufferSource {
    char padding000[8];
    unsigned int name;
    unsigned int format;
    unsigned int sampleRate;
    unsigned short sampleFormat;
    unsigned short channelCount;
    unsigned int sampleCount;
    unsigned int loopData;
    unsigned int sampleOwner;
};
struct SoundBufferInfo {
    unsigned int name;
    unsigned int format;
    unsigned int sampleRate;
    unsigned short sampleFormat;
    unsigned short channelCount;
    unsigned int sampleCount;
    unsigned int loopData;
    unsigned int sampleOwner;
};

// Config flags and the final two settings are copied without interpretation here.
struct LfoSettings {
    unsigned int configFlags;
    unsigned int initialDelay;
    unsigned int phaseStep;
    unsigned int depth;
    unsigned int reservedConfig0;
    unsigned int reservedConfig1;
};

struct LfoState {
    int state;
    float scale;
    unsigned int delayRemaining;
    float depthScale;
    unsigned int configFlags;
    unsigned int initialDelay;
    unsigned int phaseStep;
    unsigned int depth;
    unsigned int reservedConfig0;
    unsigned int reservedConfig1;
};

// Copy oscillator settings, compute the step scale and begin with the configured delay.
int gtl_AudioLfo_Initialize(LfoState* out, const LfoSettings* source) {
    unsigned int depth = source->depth;
    union { PpcDoubleWords words; double d; } conv;
    conv.words.hi = 0x43300000;
    conv.words.lo = depth ^ 0x80000000;
    float delta = (float)(conv.d - kDouble_4503601774854144_AudioLfoDepth);
    out->depthScale = delta * kFloat_0_5_AudioEnvelopeLfo / kAudioLfoDepthUnitMax;
    out->delayRemaining = source->initialDelay;
    out->configFlags = source->configFlags;
    out->initialDelay = source->initialDelay;
    out->phaseStep = source->phaseStep;
    out->depth = depth;
    out->reservedConfig0 = source->reservedConfig0;
    out->reservedConfig1 = source->reservedConfig1;
    out->state = 0;
    out->scale = kFloat_1_AudioEnvelopeLfo;
    return 1;
}

// Normalize two fixed-point scale words, supplying unity and zero when settings are absent.
int gtl_AudioScale_Initialize(float* out, unsigned int* source) {
    unsigned int zero = source[0];
    unsigned int value;
    if (zero == 0) value = kDefaultScaleUnits;
    else value = source[1];
    union { PpcDoubleWords words; double d; } conv;
    conv.words.hi = 0x43300000;
    conv.words.lo = value;
    out[0] = (float)(conv.d - kAudioScaleUnsignedConversionBias) / kAudioScaleUnitsPerUnity;
    zero = source[0];
    if (zero == 0) value = 0;
    else value = source[2];
    conv.words.lo = value;
    out[1] = (float)(conv.d - kAudioScaleUnsignedConversionBias) / kAudioScaleUnitsPerUnity;
    return 1;
}

struct EnvelopeLevels { int state; float level; float releaseStartLevel; };

// Restart the envelope at the stage selected by its configured mode.
void gtl_AudioEnvelope_Reset(EnvelopeLevels* self) {
    switch (((EnvelopeState*)self)->mode) { // 0x803FF6CC: envelope mode +0x18
    case kEnvelopeDisabled: { float value = kFloat_1_audioEnvelope; self->state = kEnvelopeSustain; self->level = value; self->releaseStartLevel = value; break; }
    case kEnvelopeFull: { float value = kFloat_0_audioEnvelope; self->state = kEnvelopeBegin; self->level = value; self->releaseStartLevel = value; break; }
    case kEnvelopeAttackOnly: { float value = kFloat_0_audioEnvelope; self->state = kEnvelopeAttack; self->level = value; self->releaseStartLevel = value; break; }
    case kEnvelopeHoldFirst: { float value = kFloat_1_audioEnvelope; self->state = kEnvelopeHoldBegin; self->level = value; self->releaseStartLevel = value; break; }
    }
}

// Copy sample format metadata to the caller; leave the trailing owner word untouched.
int gtl_CSoundBufferSource_GetBufferInfo(const SoundBufferSource* self, SoundBufferInfo* out) {
    if (out == 0) return 0;
    register unsigned int name = self->name;
    register unsigned int format = self->format;
    register unsigned int sampleRate = self->sampleRate;
    register unsigned short sampleFormat = self->sampleFormat;
    register unsigned short channelCount = self->channelCount;
    register unsigned int sampleCount = self->sampleCount;
    register unsigned int loopData = self->loopData;
    out->name = name;
    out->format = format;
    out->sampleRate = sampleRate;
    out->sampleFormat = sampleFormat;
    out->channelCount = channelCount;
    out->sampleCount = sampleCount;
    out->loopData = loopData;
    return 1;
}

struct SoundBufferPitchState {
    unsigned int flags;
    char padding004[0x184];
    float pitch;
};

struct SoundBufferVolumeState {
    unsigned int flags;
    char padding004[0x17C];
    float volume;
};

struct SoundBufferVolumeHandle {
    char padding000[8];
    SoundBufferVolumeState* state;
};

struct SoundBufferPitchHandle {
    char padding000[8];
    SoundBufferPitchState* state;
};

struct LfoResetState {
    int state;
    float scale;
    int delayRemaining;
    char padding00C[8];
    int initialDelay;
};

struct SoundBufferEnvelopeHandle {
    char padding000[8];
    char* state;
};

// Reset the oscillator phase state and restore its initial delay.
void gtl_AudioLfo_Reset(LfoResetState* self) {
    self->state = 0;
    self->scale = kFloat_1_AudioEnvelopeLfo;
    self->delayRemaining = self->initialDelay;
}

// Rebuild the envelope from the buffer settings when requested.
int gtl_ISoundBuffer_SetEnvelope(SoundBufferEnvelopeHandle* self, int enabled) {
    char* state = self->state;
    if (enabled == 0) return 0;
// The settings pointer arrives as `enabled` and stays in r4 on the Wii (0x804005E4 tail-calls with r4 untouched).
    return gtl_AudioEnvelope_Initialize((EnvelopeState*)(((SoundBufferData*)state)->envelope), (EnvelopeSettings*)state);
}

// Set the volume only when the source permits volume changes.
int gtl_ISoundBuffer_SetVolume(SoundBufferVolumeHandle* self, float volume) {
    SoundBufferVolumeState* state = self->state;
    if (!(state->flags & kVolumeWritable)) {
        return 0;
    }
    state->volume = volume;
    return 1;
}

// Free this handle only when invoked as a deleting destructor.
void* dtor_8040034C(void* self, int shouldFree) {
    if (self != 0 && shouldFree > 0) {
        __dl__FPv(self);
    }
    return self;
}

struct SoundBufferPanLoopState {
    char padding000[0x190];
    float pan;
    char padding194[4];
    int loopStart;
    int loopEnd;
};

struct SoundBufferPanLoopHandle {
    char padding000[8];
    SoundBufferPanLoopState* state;
};

struct SoundBufferStatusHandle { char padding000[8]; char* state; };
struct SoundVoiceStatus {
    int refs;
    void** vtable;
    char padding008[0x9F];
    unsigned char flag;
};

// Report playing, paused, stopped and loop-requested status under the interrupt lock.
int gtl_ISoundBuffer_GetStatusFlags(SoundBufferStatusHandle* self, int* out) {
    if (out == 0) return 0;
    InterruptState guard;
    guard.level = OSDisableInterrupts();
    char* state = self->state;
    SoundVoiceStatus* ref = *(SoundVoiceStatus**)&((SoundBufferData*)state)->activeVoice;
    if (ref == 0) {
        *out = 8;
    } else if (ref->vtable != 0) {
        int flags = 1;
        *out = flags;
        if (ref->flag) { flags |= 2; *out = flags; }
    } else {
        *out = 8;
    }
    if (*(int*)&((SoundBufferData*)state)->loopParameters[0]) *out |= 4;
    OSRestoreInterrupts(guard.level);
    return 1;
}

struct SoundBufferSourceHandle { char padding000[8]; char* state; };
struct SoundSourceVtableView { int refs; void** vtable; };
struct SoundSourceRefCount { int refs; };
struct SoundSourceVirtualBase { 
virtual void destroy(int); virtual int bind(char*);
};
struct SoundSourceReference : SoundSourceRefCount, SoundSourceVirtualBase {
};

// Vtable slots of a sample source (vtable lives at +4, after the reference count):
// slot 2 = destroy(this, freeMemory), slot 3 = bind(this, sourceInfo) -> nonzero on success.
enum { kSourceSlotDestroy = 2, kSourceSlotBind = 3 };
// Invoke the source's deleting destructor after its last reference is released.
static inline void SoundSource_Destroy(SoundSourceReference* src) {
    ((void (*)(void*, int))(((SoundSourceVtableView*)src)->vtable)[kSourceSlotDestroy])(src, 1);
}
// Bind borrowed sample metadata through the source interface; return its success flag.
static inline int SoundSource_Bind(SoundSourceReference* src, char* sourceInfo) {
    return ((int (*)(void*, char*))(((SoundSourceVtableView*)src)->vtable)[kSourceSlotBind])(src, sourceInfo);
}

// Release the old source, retain the replacement and bind its sample metadata.
int gtl_ISoundBuffer_SetSource(SoundBufferSourceHandle* self, SoundSourceReference* next) {
    char* state = self->state;
    SoundSourceReference* old = *(SoundSourceReference**)&((SoundBufferData*)state)->refObject;
    if (old != 0) {
        --old->refs;
        // The original calls slot 0x8 of the vtable at +4 with this = the object start (no +4 adjust).
        if (old->refs == 0) SoundSource_Destroy(old); // last reference dropped: destroy and free
        *(SoundSourceReference**)&((SoundBufferData*)state)->refObject = 0;
    }
    if (next == 0) return 0;
    *(SoundSourceReference**)&((SoundBufferData*)state)->refObject = next;
    ++next->refs;
    SoundSourceReference* cur = *(SoundSourceReference**)&((SoundBufferData*)state)->refObject;
    return SoundSource_Bind(cur, (char*)((SoundBufferData*)state)->sourceInfo) != 0;
}

struct LfoUpdateState {
    int state;
    float scale;
    unsigned int phase; // Same +8 word as LfoState.delayRemaining; see update contract.
    char padding00C[0x0C];
    int phaseStep;
    int depth;
};

// Consume the initial delay, then advance the wrapped phase and evaluate the asymmetric sine scale.
// State 0: +8 counts delay ticks down; state 1: +8 is an oscillator phase count.
// On delay expiry it is cleared, then advanced by phaseStep * FULL elapsedTicks
// in this same call, rather than just the ticks left after the delay. A single
// 0xAAAA subtraction wraps it when >0xAAAA; this is not a general modulo loop.
// The computed sine has positive amplitude 1 and negative amplitude 0.5 about 1.
// depth gates updates here; its normalized depthScale is initialized elsewhere.
void gtl_AudioLfo_Update(LfoUpdateState* self, unsigned int elapsedTicks) {
    if (!self->phaseStep || !self->depth) return;
    if (self->state == 0) {
        self->scale = kFloat_1_AudioEnvelopeLfo;
        if (self->phase <= elapsedTicks) {
            self->state = 1;
            self->phase = 0;
        } else {
            self->phase -= elapsedTicks;
            return;
        }
    } else if (self->state < 0 || self->state > 1) return;
    unsigned int phase = self->phase + self->phaseStep * elapsedTicks;
    self->phase = phase;
    if (phase > kLfoPhaseLimit) self->phase = phase - kLfoPhaseModulus + kLfoPhaseWrapBias; // == phase - 43690: back by one cycle
    // Convert the phase count to an angle: 2*pi * phase / 43690 (one full cycle), then take its sine.
    float phaseFloat = (float)self->phase;
    float result = Math_SinApprox((kFloat_6_2831855 * phaseFloat) / kAudioLfoPhaseCountsPerCycle);
    self->scale = kFloat_1_AudioEnvelopeLfo + result * (result > kFloat_0_AudioLfoSine ? kFloat_1_AudioEnvelopeLfo : kFloat_0_5_AudioEnvelopeLfo);
}

// Set the pitch only when the source permits pitch changes.
int gtl_ISoundBuffer_SetPitch(SoundBufferPitchHandle* self, float pitch) {
    SoundBufferPitchState* state = self->state;
    if (!(state->flags & kPitchWritable)) {
        return 0;
    }
    state->pitch = pitch;
    return 1;
}

// Copy the source position into the spatial-parameter block.
int gtl_ISoundBuffer_SetPosition(SoundBufferPanLoopHandle* self, const void* source) {
    Vector_CopyComponents(((SoundBufferData*)self->state)->entries, source);
    return 1;
}

// Default sound-buffer hook: always report success.
int gtl_ISoundBuffer_DefaultSuccess() {
    return 1;
}

// Initialize the envelope unit's cached two-pi constant.
void gtl_AudioEnvelope_InitializeTwoPi() {
    s_audioEnvelopeTwoPi = kFloat_6_2831855;
}

// Initialize the envelope-reset unit's cached two-pi constant.
void gtl_AudioEnvelopeReset_InitializeTwoPi() {
    s_audioEnvelopeResetTwoPi = kFloat_6_2831855;
}

// Initialize the LFO unit's cached two-pi constant.
void gtl_AudioLfo_InitializeTwoPi() {
    s_audioLfoTwoPi = kFloat_6_2831855;
}

struct SoundBufferPlayHandle { char padding000[8]; char* state; };
struct SoundBufferPlayState {
    unsigned int flags;
    unsigned char playbackTag; unsigned char loopEnabled; char playbackPadding[6];
    unsigned int playbackLoopStart; unsigned int playbackLoopEnd;
    char playbackRest[0x88]; void* resource; void* source;
    char padding0A4[0x0E]; unsigned short channelCount;
    char padding0B4[0x5C]; int spatialMode; int envelopeStage; float envelopeLevel;
    char padding11C[0x10]; int envelopeMode;
    char padding130[0x20]; int lfoStage; float pitchScale; int lfoDelay;
    char padding15C[0x1C]; float lowPassFilter; char padding17C[4];
    float volume; float listenerVolume; float pitch; float listenerPitch;
    float pan; unsigned int loopRequested; unsigned int loopStart; unsigned int loopEnd;
    float mainSendOffset; float auxSend; float remoteVolume0; float remoteVolume1;
    float remoteVolume2; float remoteVolume3;
};
extern void* gtl_VoicePool_GetInstance();
extern void* gtl_VoicePool_AcquireVoice(void*, unsigned short, int, int, void*, void*);
extern void GtlVoice_ConfigureSamplePlayback(void*, void*, int);
// Set the voice low-pass filter (Voice +0xC0).
extern void GtlVoice_SetLowPassFilter(void*, float);
// Set the voice main-send offset (clamped after adding one).
extern void GtlVoice_SetMainSendOffset(void*, float);
// Set the voice output-line mask.
extern void GtlVoice_SetOutputLineMask(void*, int);
// Set one of the three voice auxiliary sends.
extern void GtlVoice_SetAuxSend(void*, int, float);
// Set one of the four additional remote parameters (Voice +0xFC).
extern void GtlVoice_SetAdditionalOutputMixControl(void*, int, float);
// Set one of the four remote voice volumes.
extern void GtlVoice_SetRemoteVolume(void*, int, float);
// Set one of the four remote-send offsets (clamped after adding one).
extern void GtlVoice_SetRemoteSendOffset(void*, int, float);
extern void GtlVoice_RequestStart(void*);
extern float GtlAudio_GetMainSend();
extern void* gtl_Audio_GetListener();
extern int gtl_Audio_CalculateSpatialParameters(void*, void*, void*, int);
extern void gtl_nw4r_detail_Voice_SetVolumeScale(void*, float);
// Set voice pitch scale.
extern void GtlVoice_SetPitchScale(void*, float);
// Set voice pan.
extern void GtlVoice_SetPan(void*, float);
// Set voice surround pan.
extern void GtlVoice_SetSurroundPan(void*, float);
extern void gtl_ISoundBuffer_VoiceCallback();



enum SoundOutputMask {
    kOutputMain = 1, kOutputRemote0 = 2, kOutputRemote1 = 4,
    kOutputRemote2 = 8, kOutputRemote3 = 16
};

// Enable main/remote output bits for strictly positive stored send levels.
static inline int SoundBufferOutputFlags(const SoundBufferPlayState* s) {
    return ((s->mainSendOffset > kFloat_0_soundBuffer ? kOutputMain : 0) |
            (s->remoteVolume0 > kFloat_0_soundBuffer ? kOutputRemote0 : 0) |
            (s->remoteVolume1 > kFloat_0_soundBuffer ? kOutputRemote1 : 0) |
            (s->remoteVolume2 > kFloat_0_soundBuffer ? kOutputRemote2 : 0) |
            (s->remoteVolume3 > kFloat_0_soundBuffer ? kOutputRemote3 : 0));
}

// Acquire and bind a voice, reset modulation, apply sends and spatial parameters, then start playback.
// playbackFlags bit 0 requests looping; loop endpoints are sample positions.
// Return 0 on acquisition/bind failure, 1 after the update/start path. A bind
// failure leaves the acquired resource attached; no rollback is performed here.
// entryLock holds the entry interrupt state until return. Each nested scope saves
// and restores its own previous state, so its restore leaves the outer lock held.
// Lock map: entryLock covers stop/acquire/bind through RequestStart; setupLock
// covers modulation reset and initial sends; parameterLock covers one parameter
// family; voiceLock covers a nested resource recheck/mask or gain/pitch write.
// A restore returns to that scope's saved level, not necessarily enabled interrupts.
// The global main send is applied after initial sends, then spatial gains are
// calculated, and RequestStart is reached even if that calculation stops the voice.
int gtl_ISoundBuffer_Play(SoundBufferPlayHandle* self, int playbackFlags) {
    // Phase 0: keep the entry interrupt state until every exit; stop/acquire/bind the voice.
    InterruptState entryLock;
    entryLock.level = OSDisableInterrupts();
    SoundBufferPlayState* playback = (SoundBufferPlayState*)self->state;
    void* source = playback->source; // +0xA0: retained source object, not the voice callback.
    if (playback->resource) gtl_ISoundBuffer_Stop((SoundBufferStopHandle*)self, 0);
    playback->loopRequested = playbackFlags & 1;
    playback->resource = gtl_VoicePool_AcquireVoice(gtl_VoicePool_GetInstance(), playback->channelCount, 1, 0xFF, (void*)gtl_ISoundBuffer_VoiceCallback, self);
    if (!playback->resource) {
        OSRestoreInterrupts(entryLock.level);
        return 0;
    }
    if (!((int (*)(void*, void*))(*(void**)(*(char**)((char*)source + 4) + 0x10)))(source, self)) {
        OSRestoreInterrupts(entryLock.level);
        return 0;
    }
    // Phase 1: bind sample metadata and publish the requested loop range.
    playback->loopEnabled = playback->loopRequested != 0;
    if (playback->loopRequested != 0 && playback->loopStart != playback->loopEnd && playback->loopEnd != 0) {
        playback->playbackLoopStart = playback->loopStart;
        playback->playbackLoopEnd = playback->loopEnd;
    }
    GtlVoice_ConfigureSamplePlayback(playback->resource, &playback->playbackTag, 0);

    // Phase 2: reset modulation and set filter, main/aux sends, then remote lines.
    InterruptState setupLock;
    setupLock.level = OSDisableInterrupts();
    // Original 0x803FFBE8: resource setup has a shared update continuation.
    do {
        if (!playback->resource) {
            OSRestoreInterrupts(setupLock.level);
            break;
        }
        gtl_AudioEnvelope_Reset((EnvelopeLevels*)(&playback->envelopeStage));
        gtl_AudioLfo_Reset((LfoResetState*)(&playback->lfoStage));
        {
            InterruptState parameterLock;
            parameterLock.level = OSDisableInterrupts();
            if (!playback->resource) OSRestoreInterrupts(parameterLock.level);
            else {
                GtlVoice_SetLowPassFilter(playback->resource, playback->lowPassFilter);
                OSRestoreInterrupts(parameterLock.level);
            }
        }
        {
            InterruptState parameterLock;
            parameterLock.level = OSDisableInterrupts();
            if (!playback->resource) OSRestoreInterrupts(parameterLock.level);
            else {
                GtlVoice_SetMainSendOffset(playback->resource, playback->mainSendOffset);
                {
                    InterruptState voiceLock;
                    voiceLock.level = OSDisableInterrupts();
                    if (!playback->resource) OSRestoreInterrupts(voiceLock.level);
                    else {
                        int flags = SoundBufferOutputFlags(playback);
                        GtlVoice_SetOutputLineMask(playback->resource, flags);
                        OSRestoreInterrupts(voiceLock.level);
                    }
                }
                OSRestoreInterrupts(parameterLock.level);
            }
        }
        {
            InterruptState parameterLock;
            parameterLock.level = OSDisableInterrupts();
            if (!playback->resource) OSRestoreInterrupts(parameterLock.level);
            else {
                GtlVoice_SetAuxSend(playback->resource, 0, playback->auxSend);
                GtlVoice_SetAuxSend(playback->resource, 1, kFloat_0_AudioResource);
                GtlVoice_SetAuxSend(playback->resource, 2, kFloat_0_AudioResource);
                GtlVoice_SetAdditionalOutputMixControl(playback->resource, 0, playback->auxSend);
                GtlVoice_SetAdditionalOutputMixControl(playback->resource, 1, playback->auxSend);
                GtlVoice_SetAdditionalOutputMixControl(playback->resource, 2, playback->auxSend);
                GtlVoice_SetAdditionalOutputMixControl(playback->resource, 3, playback->auxSend);
                OSRestoreInterrupts(parameterLock.level);
            }
        }
        {
            InterruptState parameterLock;
            parameterLock.level = OSDisableInterrupts();
            if (!playback->resource) OSRestoreInterrupts(parameterLock.level);
            else {
                GtlVoice_SetRemoteVolume(playback->resource, 0, playback->remoteVolume0);
                GtlVoice_SetRemoteVolume(playback->resource, 1, playback->remoteVolume1);
                GtlVoice_SetRemoteVolume(playback->resource, 2, playback->remoteVolume2);
                GtlVoice_SetRemoteVolume(playback->resource, 3, playback->remoteVolume3);
                GtlVoice_SetRemoteSendOffset(playback->resource, 0, kSoundBufferUnity);
                GtlVoice_SetRemoteSendOffset(playback->resource, 1, kSoundBufferUnity);
                GtlVoice_SetRemoteSendOffset(playback->resource, 2, kSoundBufferUnity);
                GtlVoice_SetRemoteSendOffset(playback->resource, 3, kSoundBufferUnity);
                {
                    InterruptState voiceLock;
                    voiceLock.level = OSDisableInterrupts();
                    if (!playback->resource) OSRestoreInterrupts(voiceLock.level);
                    else {
                        int flags = SoundBufferOutputFlags(playback);
                        GtlVoice_SetOutputLineMask(playback->resource, flags);
                        OSRestoreInterrupts(voiceLock.level);
                    }
                }
                OSRestoreInterrupts(parameterLock.level);
            }
        }
        OSRestoreInterrupts(setupLock.level);
    } while (0);
    // Phase 3: the global main send replaces the stored send after initial setup.
    playback->mainSendOffset = GtlAudio_GetMainSend();
    {
        InterruptState parameterLock;
        parameterLock.level = OSDisableInterrupts();
        if (!playback->resource) OSRestoreInterrupts(parameterLock.level);
        else {
            GtlVoice_SetMainSendOffset(playback->resource, playback->mainSendOffset);
            {
                InterruptState voiceLock;
                voiceLock.level = OSDisableInterrupts();
                if (!playback->resource) OSRestoreInterrupts(voiceLock.level);
                else {
                    int flags = SoundBufferOutputFlags(playback);
                    GtlVoice_SetOutputLineMask(playback->resource, flags);
                    OSRestoreInterrupts(voiceLock.level);
                }
            }
            OSRestoreInterrupts(parameterLock.level);
        }
    }
    // Phase 4: apply listener gains, envelope gain, pitch modulation and pan.
    // spatialParameters[] = pan, surround pan, volume factor, pitch factor.
    // Original 0x803FFBE8: both spatial update paths reach the final voice update.
    do {
        if (playback->resource) {
            void* listener = (playback->flags & kUseListener) ? gtl_Audio_GetListener() : 0;
            int spatialFlag = (playback->flags >> kSpatialModeShift) & 1;
            InterruptState parameterLock;
            parameterLock.level = OSDisableInterrupts();
            if (playback->resource) {
                float currentEnvelopeGain = !playback->envelopeMode ? (playback->envelopeStage == kEnvelopeRelease ? kFloat_0_soundBuffer : kSoundBufferUnity) : playback->envelopeLevel;
                if (playback->envelopeStage == kEnvelopeRelease && currentEnvelopeGain == kFloat_0_soundBuffer) {
                    InterruptState voiceLock;
                    voiceLock.level = OSDisableInterrupts();
                    if (playback->resource) GtlVoice_Stop(playback->resource);
                    OSRestoreInterrupts(voiceLock.level);
                    OSRestoreInterrupts(parameterLock.level);
                    break;
                }
                playback->listenerVolume = kSoundBufferUnity;
                playback->listenerPitch = kSoundBufferUnity;
                float pan = playback->pan, surroundPan = kFloat_0_SoundBuffers;
                if (listener && playback->spatialMode != kSpatialModeDisabled) {
                    float spatialParameters[4];
                    if (gtl_Audio_CalculateSpatialParameters(listener, ((SoundBufferData*)playback)->entries, spatialParameters, spatialFlag) == 1) {
                        playback->listenerVolume = spatialParameters[2];
                        playback->listenerPitch = spatialParameters[3];
                        pan = spatialParameters[0]+kFloat_0_SpatialAudio;
                        surroundPan = spatialParameters[1]+kFloat_1_AudioSpatial;
                    }
                }
                {
                    InterruptState voiceLock;
                    voiceLock.level = OSDisableInterrupts();
                    if (playback->resource) {
                        float voiceEnvelopeGain = !playback->envelopeMode ? (playback->envelopeStage == kEnvelopeRelease ? kFloat_0_soundBuffer : kSoundBufferUnity) : playback->envelopeLevel;
                        gtl_nw4r_detail_Voice_SetVolumeScale(playback->resource, playback->volume*playback->listenerVolume*voiceEnvelopeGain);
                    }
                    OSRestoreInterrupts(voiceLock.level);
                }
                {
                    InterruptState voiceLock;
                    voiceLock.level = OSDisableInterrupts();
                    if (playback->resource) GtlVoice_SetPitchScale(playback->resource, playback->pitch*playback->listenerPitch*playback->pitchScale);
                    OSRestoreInterrupts(voiceLock.level);
                }
                GtlVoice_SetPan(playback->resource, pan);
                GtlVoice_SetSurroundPan(playback->resource, surroundPan);
            }
            OSRestoreInterrupts(parameterLock.level);
        }
    } while (0);
    // The original (0xd54) calls this on every path past the update block, including the stop path.
    if (playback->resource) GtlVoice_RequestStart(playback->resource);
    OSRestoreInterrupts(entryLock.level);
    return 1;
}

struct SoundBufferParameterHandle { char padding000[8]; char* state; };
struct SoundParameterScalar { char padding000[4]; float value; };
// Borrowed descriptor -> validity wrapper -> scalar. The scalar's leading word is opaque.
struct SoundParameterValue { unsigned short valid; char padding002[2]; SoundParameterScalar* data; };
struct SoundParameterEntry { int mode; SoundParameterValue* value; };
// Parameter mode and values[] index share this order. Remote output mask bits
// are shifted by one because bit 0 belongs to the main line; aux has no mask bit.
enum SoundSendParameter {
    kSendMain = 0, kSendAux = 1,
    kSendRemote0 = 2, kSendRemote1 = 3, kSendRemote2 = 4, kSendRemote3 = 5,
    kSendParameterCount = 6
};
struct SoundBufferSendState {
    void* resource;
    char padding004[0x100];
    float values[kSendParameterCount]; // Indexed by SoundSendParameter; main, aux, remote[4].
};
// Full buffer layout anchors this view to +0x9C; values[] then begins at +0x1A0.
// resource is borrowed here; parameter application never acquires or releases a voice.
struct SoundBufferSendOwnerView { char beforeVoice[0x9C]; SoundBufferSendState sends; };
typedef char SoundBufferSendValuesOffset[((int)&((SoundBufferSendOwnerView*)0)->sends.values == 0x1A0) ? 1 : -1];
void GtlVoice_SetMainSendOffset(void*, float);
void GtlVoice_SetOutputLineMask(void*, int);
void GtlVoice_SetAuxSend(void*, int, float);
void GtlVoice_SetAdditionalOutputMixControl(void*, int, float);
void GtlVoice_SetRemoteVolume(void*, int, float);
void GtlVoice_SetRemoteSendOffset(void*, int, float);

// Apply valid main-send, auxiliary-send and remote-volume entries and refresh active voice output flags.
// Entries and nested data are borrowed. A scalar is read at data+4 only if both
// pointers and valid are nonzero; unknown modes are ignored. Return 0 only for
// a null entries pointer. Stored values update even if there is no active voice.
// Main/remote lines are enabled only for strictly positive stored levels. Aux
// uses aux slot 0 (slots 1/2 get zero) and all four additional output controls.
int gtl_ISoundBuffer_ApplyParameterEntries(SoundBufferParameterHandle* self, SoundParameterEntry* entries, unsigned int count) {
    SoundBufferSendState* state = &((SoundBufferSendOwnerView*)self->state)->sends;
    if (entries == 0) return 0;
    for (unsigned int i = 0; i < count; ++i) {
        SoundParameterEntry* entry = entries + i;
        SoundParameterValue* value = entry->value;
        if (value == 0 || value->data == 0 || value->valid == 0) continue;
        float sendLevel = value->data->value;
        switch (entry->mode) {
        case kSendMain: {
            state->values[kSendMain] = sendLevel;
            InterruptState first;
            first.level = OSDisableInterrupts();
            void* resource = state->resource;
            if (!resource) { OSRestoreInterrupts(first.level); break; }
            GtlVoice_SetMainSendOffset(resource, state->values[kSendMain]);
            InterruptState second;
            second.level = OSDisableInterrupts();
            resource = state->resource;
            if (!resource) { OSRestoreInterrupts(second.level); }
            else {
                int flags = 0;
                if (state->values[kSendMain] > 0.0f) flags |= kOutputMain;
                if (state->values[kSendRemote0] > 0.0f) flags |= kOutputRemote0;
                if (state->values[kSendRemote1] > 0.0f) flags |= kOutputRemote1;
                if (state->values[kSendRemote2] > 0.0f) flags |= kOutputRemote2;
                if (state->values[kSendRemote3] > 0.0f) flags |= kOutputRemote3;
                GtlVoice_SetOutputLineMask(resource, flags);
                OSRestoreInterrupts(second.level);
            }
            OSRestoreInterrupts(first.level);
            break;
        }
        case kSendAux: {
            state->values[kSendAux] = sendLevel;
            InterruptState guard;
            guard.level = OSDisableInterrupts();
            void* resource = state->resource;
            if (!resource) { OSRestoreInterrupts(guard.level); break; }
            GtlVoice_SetAuxSend(resource, 0, state->values[kSendAux]);
            GtlVoice_SetAuxSend(state->resource, 1, kFloat_0_AudioResource);
            GtlVoice_SetAuxSend(state->resource, 2, kFloat_0_AudioResource);
            GtlVoice_SetAdditionalOutputMixControl(state->resource, 0, state->values[kSendAux]);
            GtlVoice_SetAdditionalOutputMixControl(state->resource, 1, state->values[kSendAux]);
            GtlVoice_SetAdditionalOutputMixControl(state->resource, 2, state->values[kSendAux]);
            GtlVoice_SetAdditionalOutputMixControl(state->resource, 3, state->values[kSendAux]);
            OSRestoreInterrupts(guard.level);
            break;
        }
        case kSendRemote0: case kSendRemote1: case kSendRemote2: case kSendRemote3: {
            if (entry->mode < kSendRemote0) break;
            state->values[entry->mode] = sendLevel;
            InterruptState guard;
            guard.level = OSDisableInterrupts();
            void* resource = state->resource;
            if (!resource) { OSRestoreInterrupts(guard.level); break; }
            GtlVoice_SetRemoteVolume(resource, 0, state->values[kSendRemote0]);
            GtlVoice_SetRemoteVolume(state->resource, 1, state->values[kSendRemote1]);
            GtlVoice_SetRemoteVolume(state->resource, 2, state->values[kSendRemote2]);
            GtlVoice_SetRemoteVolume(state->resource, 3, state->values[kSendRemote3]);
            GtlVoice_SetRemoteSendOffset(state->resource, 0, kSoundBufferUnity);
            GtlVoice_SetRemoteSendOffset(state->resource, 1, kSoundBufferUnity);
            GtlVoice_SetRemoteSendOffset(state->resource, 2, kSoundBufferUnity);
            GtlVoice_SetRemoteSendOffset(state->resource, 3, kSoundBufferUnity);
            InterruptState inner;
            inner.level = OSDisableInterrupts();
            resource = state->resource;
            if (!resource) { OSRestoreInterrupts(inner.level); }
            else {
                int flags = 0;
                if (state->values[kSendMain] > 0.0f) flags |= kOutputMain;
                if (state->values[kSendRemote0] > 0.0f) flags |= kOutputRemote0;
                if (state->values[kSendRemote1] > 0.0f) flags |= kOutputRemote1;
                if (state->values[kSendRemote2] > 0.0f) flags |= kOutputRemote2;
                if (state->values[kSendRemote3] > 0.0f) flags |= kOutputRemote3;
                GtlVoice_SetOutputLineMask(resource, flags);
                OSRestoreInterrupts(inner.level);
            }
            OSRestoreInterrupts(guard.level);
            break;
        }
        }
    }
    return 1;
}

// Store the pan value used by the next spatial update.
int gtl_ISoundBuffer_SetPan(SoundBufferPanLoopHandle* self, float pan) {
    self->state->pan = pan;
    return 1;
}

// Store the requested sample-loop start and end positions.
int gtl_ISoundBuffer_SetLoopRange(SoundBufferPanLoopHandle* self, int loopStart, int loopEnd) {
    SoundBufferPanLoopState* state = self->state;
    state->loopStart = loopStart;
    state->loopEnd = loopEnd;
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462690", _ctors_u803FF5B8_0, gtl_AudioEnvelope_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80462690", _ctors_u803FF5B8_1, gtl_AudioEnvelopeReset_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80462690", _ctors_u803FF5B8_2, gtl_AudioLfo_InitializeTwoPi);
#pragma pop
