// GTL audio voice AX control: DSP address and sample-offset math for ADPCM, PCM16 and PCM8 sources,
// volume, pan and decibel gain lookups, loop and end-address setup, and sample position reads.
// The controller's shutdown fade is here too. Parameter blocks and voice objects are borrowed, not owned.

#include <decomp/static_init.h>
#include "classes/GtlAudioControllerState.h"

#include "sdk_protos.h"

extern "C" {

// Address domains: source bases are CPU byte addresses; PCM16 DSP addresses
// count 16-bit words, PCM8 addresses count bytes, ADPCM addresses count nibbles.
// An ADPCM frame is 8 encoded bytes: a 2-nibble header and 14 sample nibbles.
enum AxAddressConstants {
    kAdpcmSamplesPerFrame = 14,
    kAdpcmNibblesPerFrame = 16,
    kAdpcmHeaderNibbles = 2,
    kAdpcmEncodedByteMask = 7,
    kAdpcmNibblesPerByte = 2,
    kAdpcmFrameNibbleMask = 15,
    kAdpcmFrameNibbleShift = 4,
    kSilenceAdpcmNibbles = 0x200,
    kSilencePcm16Words = 0x80,
    kSilencePcm8Bytes = 0x100,
    kDspAddressHighShift = 16,
    kRampOutputSamples = 96,
    kNoCachedEnvelopeValue = -0x8000
};
static const unsigned int kCpuToDspAddressBias = 0x80000000U;
enum AxSampleFormat {
    kAxAdpcm = 0,
    kAxPcm16 = 10,
    kAxPcm8 = 25
};
enum VoiceInterpolationChoice {
    kInterpolationTable0 = 2,
    kInterpolationTable1 = 3,
    kInterpolationTable2 = 4,
    kInterpolationChooseFromPitch = 5
};
enum AudioControlConstants {
    kOutputSampleRateHz = 32000,
    kMaximumExponentialGain = 32000,
    kShutdownFadeFrames = 3,
    kShutdownWaitCallbacks = 6,
    kMaximumCoefficientTableIndex = 127,
    kPanTableLastIndex = 256,
    kGainTableZeroDecibelIndex = 0x388,
    kAxSyncType = 8
};
static const float kInterpolationTable0PitchThreshold = 1.3333333730697632f;
enum AxSyncBits {
    kAxSyncVolume = 0x100,
    kAxSyncAllAddresses = 0x400,
    kAxSyncLoopFlag = 0x800,
    kAxSyncLoopAddress = 0x1000,
    kAxSyncEndAddress = 0x2000,
    kAxSyncAdpcm = 0x8000,
    kAxSyncParameterFE = 0x00800000
};
struct AxVoiceParameters {
    char reserved00[0x1C];
    unsigned int syncFlags;
    char reserved20[0x72];
    unsigned short volume;
    short volumeDelta;
    unsigned short loopFlag, sampleFormat;
    unsigned short loopAddressHi, loopAddressLo;
    unsigned short endAddressHi, endAddressLo;
    unsigned short currentAddressHi, currentAddressLo;
    unsigned char adpcm[0x28];
    char reservedCE[0x30];
    short dspParameterFE; // hardware meaning remains unconfirmed
};
struct VoiceSourceState {
    void* voice;
    char reserved04[0xC];
    unsigned int sampleBase;
    unsigned int sampleFormat;
    unsigned int sampleRate;
};

extern float kFloat_6_2831855;
extern float s_audioLookupAngularPeriod;
extern float s_audioVoiceAngularPeriod;
extern const float kFloat_0_5_AudioPan;
extern const float kAudioPanLookupMaximumCoordinate;
extern const float kFloat_6_Audio;
extern const float kFloat_neg90_4_AudioGain;
extern const float kAudioPanGainIndexScale;
extern const float kFloat_0_AudioLookup;
extern float kAudioDecibelGainIndexScale;
extern float kFloat_0_AudioValueAnimation;
extern signed char s_audioControllerInitialized;
extern unsigned char s_audioShutdownMasterSilent;
extern char s_gtlAudioController[];
extern char s_gtlAudioControllerRegistration[];
struct FiveCoefficientEntry {
    unsigned short a, b, c, d, e;
};
extern FiveCoefficientEntry s_audioControlFiveCoefficientTable[];
extern float s_audioDecibelGainTable[];
extern float s_audioPanGainTable[];
void AXSetVoicePriority(void*, int);
void __dl__FPv(void*);
void* AudioInterface_SetDmaCallback(void*);
void gtl_AudioController_InitMembers(void*);
void gtl_AudioController_Dtor(void*, void*, void*);
void AXSetMaxDspCycles(int);
void AudioVoice_SetSourceInterpolationMode(void*, int);
void gtl_AudioController_ShutdownCallback();
void* gtl_AudioController_GetInstance();
unsigned int gtl_AudioController_GetOutputBuffer(void*);
void AXSetVoiceAddr(void*, void*);
void InterruptGuard_dtor(void*, int);

struct VoiceResetView {
    void* hardwareVoice;
    unsigned int pendingSyncFlags;
    short currentVolumeBits;
    short unknownHalfword0A;
    unsigned char envelopePending;
    char padD;
    short targetVolumeBits;
    unsigned int sampleByteBase;
    char pad14[8];
    unsigned char resetMix;
    unsigned char unknownByte1D;
    char pad1E[0x1A];
    unsigned int unknownWord38;
    unsigned int unknownWord3C;
    unsigned int unknownWord40;
    unsigned int unknownWord44;
};

// Reset the hardware binding, sync/envelope cache, sample base and opaque tail words.
// Volume sentinels are 0x8000 bit patterns, not negative acoustic gain. Reset does
// not free a hardware voice or the opaque words; their ownership is not shown here.
// +0x0C/+0x1C are confirmed by SetVolumeEnvelope/SetMixCoefficients in gtl_ax_voice_control.cpp;
// the other unnamed controls deliberately retain offset names.
void AudioVoice_ResetParameters(VoiceResetView* self) {
    short resetVolumeBits = kNoCachedEnvelopeValue;
    self->hardwareVoice = 0;
    self->currentVolumeBits = resetVolumeBits;
    self->pendingSyncFlags = 0;
    self->envelopePending = 0;
    self->targetVolumeBits = kNoCachedEnvelopeValue;
    self->unknownHalfword0A = 0;
    self->sampleByteBase = 0;
    self->resetMix = 0;
    self->unknownByte1D = 0;
    self->unknownWord38 = 0;
    self->unknownWord3C = 0;
    self->unknownWord40 = 0;
    self->unknownWord44 = 0;
}

// Optionally free an audio voice base allocation.
void* dtor_804068E8(void* self, int shouldDelete) {
    if (self != 0 && shouldDelete > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Optionally free an auxiliary audio allocation.
void* AudioVoiceAux_Dtor(void* self, int shouldDelete) {
    if (self != 0 && shouldDelete > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Initialize the audio lookup module's full-turn constant.
void AudioLookup_InitializeAngularConstant() {
    s_audioLookupAngularPeriod = kFloat_6_2831855;
}

// Initialize the voice module's full-turn constant.
void AudioVoice_InitializeAngularConstant() {
    s_audioVoiceAngularPeriod = kFloat_6_2831855;
}

// Set AX voice priority only when a hardware voice is allocated.
void AudioVoice_SetPriorityIfAllocated(void** self, int priority) {
    if (*self != 0) {
        AXSetVoicePriority(*self, priority);
    }
}

struct Struct_804064C0 {
    char pad[0x3C];
    float start, end;
    int count, index;
    void* callback;
    int timer;
};
// Capture current master gain and start a three-frame fade with the shutdown callback.
void gtl_AudioController_BeginShutdownFade(Struct_804064C0* self) {
    if (self->callback == 0) {
        int count = self->count;
        int index = self->index;
        float value;
        if (index >= count)
            value = self->end;
        else
            value = self->start + ((float)index * (self->end - self->start)) / (float)count;
        self->start = value;
        self->end = 0.0f;
        self->count = kShutdownFadeFrames;
        self->index = 0;
        self->timer = -1;
        self->callback = AudioInterface_SetDmaCallback((void*)gtl_AudioController_ShutdownCallback);
    }
}

// Select the requested resampling mode, deriving automatic mode from sample rate and pitch.
void GtlVoice_SelectResamplingModeForPitch(void* self, float value, int mode) {
    BOOL level = OSDisableInterrupts();
    if (*(int*)self == 0) {
        OSRestoreInterrupts(level);
        return;
    }
    if (mode == kInterpolationChooseFromPitch) {
        int count = (*(int*)&((VoiceSourceState*)self)->sampleRate);
        float fraction = (value * (float)count) / (float)kOutputSampleRateHz;
        if (fraction > kInterpolationTable0PitchThreshold)
            mode = kInterpolationTable0;
        else if (fraction > 1.0f)
            mode = kInterpolationTable1;
        else
            mode = kInterpolationTable2;
    }
    AudioVoice_SetSourceInterpolationMode(self, mode);
    OSRestoreInterrupts(level);
}

// Wait for master silence, disable DSP work and count down shutdown callbacks.
void gtl_AudioController_ShutdownCallback() {
    if ((signed char)s_audioControllerInitialized == 0) {
        gtl_AudioController_InitMembers(s_gtlAudioController);
        __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                 s_gtlAudioControllerRegistration);
        s_audioControllerInitialized = 1;
    }
    void* self = s_gtlAudioController;
    ((void (*)(void))((*(void**)&((GtlAudioControllerState*)self)->updateCallback)))();
    if (s_audioShutdownMasterSilent != 0) {
        int timer = (*(int*)&((GtlAudioControllerState*)self)->stopCountdown);
        if (timer < 0) {
            AXSetMaxDspCycles(0);
            (*(int*)&((GtlAudioControllerState*)self)->stopCountdown) = kShutdownWaitCallbacks;
        }
    } else {
        int count = ((GtlAudioControllerState*)self)->masterScaleDuration;
        int index = ((GtlAudioControllerState*)self)->masterScaleFrame;
        float value;
        if (index >= count)
            value = ((GtlAudioControllerState*)self)->masterScaleEnd;
        else {
            float start = ((GtlAudioControllerState*)self)->masterScaleStart;
            float end = ((GtlAudioControllerState*)self)->masterScaleEnd;
            value = start + ((float)index * (end - start)) / (float)count;
        }
        if (value == 0.0f)
            s_audioShutdownMasterSilent = 1;
    }
    int timer = (*(int*)&((GtlAudioControllerState*)self)->stopCountdown);
    if (timer > 0)
        (*(int*)&((GtlAudioControllerState*)self)->stopCountdown) = timer - 1;
}

// Clamp the control-table index to [0,127] and copy all five unsigned coefficients.
void AudioControl_GetFiveLookupCoefficients(int value, unsigned short* out0, unsigned short* out1, unsigned short* out2,
                                            unsigned short* out3, unsigned short* out4) {
    if (value > kMaximumCoefficientTableIndex)
        value = kMaximumCoefficientTableIndex;
    else
        value &= ~(value >> 31);
    FiveCoefficientEntry* entry = s_audioControlFiveCoefficientTable + value;
    out0[0] = entry->a;
    out1[0] = entry->b;
    out2[0] = entry->c;
    out3[0] = entry->d;
    out4[0] = entry->e;
}

// Clamp decibel gain and return its precomputed linear gain.
float AudioGain_DecibelsToLinear(float value) {
    float bounded;
    if (value > kFloat_6_Audio)
        bounded = kFloat_6_Audio;
    else if (value < kFloat_neg90_4_AudioGain)
        bounded = kFloat_neg90_4_AudioGain;
    else
        bounded = value;
    int index = (int)(kAudioDecibelGainIndexScale * bounded) + kGainTableZeroDecibelIndex;
    return s_audioDecibelGainTable[index];
}

// Clamp a [0, 2] pan-table coordinate and return its rounded lookup gain.
float AudioPan_LookupScaledGain(float value) {
    float bounded;
    if (value > kAudioPanLookupMaximumCoordinate)
        bounded = kAudioPanLookupMaximumCoordinate;
    else if (value < kFloat_0_AudioLookup)
        bounded = kFloat_0_AudioLookup;
    else
        bounded = value;
    int index = (int)(kAudioPanGainIndexScale * (bounded * kFloat_0_5_AudioPan) + kFloat_0_5_AudioPan);
    return s_audioPanGainTable[index];
}

// AX address payload, in DSP ABI order; end is inclusive.
struct AxAddressRange {
    unsigned short loopEnabled, sampleFormat;
    unsigned short loopHi, loopLo;
    unsigned short endHi, endLo;
    unsigned short currentHi, currentLo;
};
typedef char AxAddressRange_size[(sizeof(AxAddressRange) == 16) ? 1 : -1];

// Convert a byte base address to the first playable sample address for the AX format.
static unsigned int SourceByteBaseToFirstDspAddress(unsigned int dspByteBase, int sampleFormat) {
    if (sampleFormat == kAxAdpcm)
        return dspByteBase * kAdpcmNibblesPerByte + kAdpcmHeaderNibbles;
    if (sampleFormat == kAxPcm16)
        return dspByteBase >> 1;
    if (sampleFormat == kAxPcm8)
        return dspByteBase;
    return 0;
}

// Convert dspByteBase sample offset and byte base to the AX DSP address representation.
static unsigned int SourceSampleOffsetToDspAddress(unsigned int dspByteBase, unsigned int sampleOffset,
                                                   int sampleFormat) {
    if (sampleFormat == kAxAdpcm)
        return dspByteBase * kAdpcmNibblesPerByte + sampleOffset % kAdpcmSamplesPerFrame +
               (sampleOffset / kAdpcmSamplesPerFrame) * kAdpcmNibblesPerFrame + kAdpcmHeaderNibbles;
    if (sampleFormat == kAxPcm16)
        return (dspByteBase >> 1) + sampleOffset;
    if (sampleFormat == kAxPcm8)
        return dspByteBase + sampleOffset;
    return 0;
}

// Set current/loop/inclusive-end DSP addresses from source-relative sample offsets.
// sampleCount is exclusive; currentSample > sampleCount selects the silence buffer
// and disables looping. A nonlooping source also uses silence for its loop address.
// The two final ABI arguments are unused. Interrupt protection includes AXSetVoiceAddr.
void gtl_Voice_SetAddressRange(void* self, unsigned int loopEnabled, unsigned int sampleByteBase,
                               unsigned int currentSample, unsigned int loopSample, unsigned int sampleCount,
                               unsigned int, unsigned int) {
    BOOL level = OSDisableInterrupts();
    AxAddressRange range;
    if (*(unsigned int*)self == 0) {
        OSRestoreInterrupts(level);
        return;
    }
    unsigned int currentDspAddress;
    unsigned int loopDspAddress;
    unsigned int endDspAddress;
    unsigned int sampleFormat = (*(unsigned int*)&((VoiceSourceState*)self)->sampleFormat);
    unsigned int loopFlag = loopEnabled;
    // Phase 1: choose source or silence addresses, then convert their address domain.
    if (currentSample > sampleCount) {
        loopFlag = 0;
        unsigned int silenceByteBase = gtl_AudioController_GetOutputBuffer(gtl_AudioController_GetInstance());
        if (silenceByteBase != 0)
            silenceByteBase += kCpuToDspAddressBias;
        currentDspAddress = SourceByteBaseToFirstDspAddress(silenceByteBase, sampleFormat);
        loopDspAddress = SourceByteBaseToFirstDspAddress(silenceByteBase, sampleFormat);
        if (sampleFormat == kAxAdpcm)
            endDspAddress = silenceByteBase * 2 + 3;
        else if (sampleFormat == kAxPcm16)
            endDspAddress = (silenceByteBase >> 1) + 1;
        else if (sampleFormat == kAxPcm8)
            endDspAddress = silenceByteBase + 1;
        else
            endDspAddress = 0;
    } else {
        unsigned int dspByteBase = sampleByteBase;
        if (dspByteBase != 0)
            dspByteBase += kCpuToDspAddressBias;
        if (loopEnabled != 0) {
            loopDspAddress = SourceSampleOffsetToDspAddress(dspByteBase, loopSample, sampleFormat);
        } else {
            unsigned int silenceByteBase = gtl_AudioController_GetOutputBuffer(gtl_AudioController_GetInstance());
            if (silenceByteBase != 0)
                silenceByteBase += kCpuToDspAddressBias;
            loopDspAddress = SourceByteBaseToFirstDspAddress(silenceByteBase, sampleFormat);
        }
        currentDspAddress = SourceSampleOffsetToDspAddress(dspByteBase, currentSample, sampleFormat);
        endDspAddress = SourceSampleOffsetToDspAddress(dspByteBase, sampleCount - 1, sampleFormat);
    }
    // Phase 2: split addresses into the exact AX halfword order and publish atomically.
    range.loopEnabled = (unsigned short)loopFlag;
    range.sampleFormat = (unsigned short)sampleFormat;
    range.loopHi = (unsigned short)(loopDspAddress >> 16);
    range.loopLo = (unsigned short)loopDspAddress;
    range.endHi = (unsigned short)(endDspAddress >> 16);
    range.endLo = (unsigned short)endDspAddress;
    range.currentHi = (unsigned short)(currentDspAddress >> 16);
    range.currentLo = (unsigned short)currentDspAddress;
    if (*(unsigned int*)self != 0)
        AXSetVoiceAddr(*(void**)self, &range);
    OSRestoreInterrupts(level);
}

double pow(double, double);
void* memset(void*, int, unsigned int);

// Map clamped [0, 1] volume to 32000 times 2 raised to 10*(volume-1).
int AudioVolume_NormalizedToExponentialGain32000(float value) {
    float bounded;
    if (value > 1.0f)
        bounded = 1.0f;
    else if (value < 0.0f)
        bounded = 0.0f;
    else
        bounded = value;
    float delta = bounded - 1.0f;
    float scaled = 10.0f * delta;
    double gainFactor = pow(2.0, (double)scaled);
    return (int)((float)kMaximumExponentialGain * (float)gainFactor);
}

struct VoiceSourceAndMixView {
    char pad0[0x10];
    unsigned int sampleByteBaseWord;
    unsigned int sampleFormatWord;
    unsigned int sampleRateWord;
    unsigned char resetMix;
    char pad1D;
    char mixCache[0x18];
};

struct AxCurrentCursorView {
    char pad[0xA2];
    unsigned short currentAddressHigh, currentAddressLow;
};
struct VoiceSilenceProbeView {
    AxCurrentCursorView* current;
    char pad4[0xC];
    int sampleByteBase;
    int sampleFormat;
};

// Check whether the DSP read cursor lies inside the format-dependent silence buffer.
int GtlVoice_IsReadingOutputBuffer(VoiceSilenceProbeView* self) {
    BOOL level = OSDisableInterrupts();
    if (self->sampleByteBase == 0) {
        OSRestoreInterrupts(level);
        return 0;
    }
    unsigned int position = 0;
    if (self->current != 0)
        position = ((unsigned int)self->current->currentAddressHigh << 16) + self->current->currentAddressLow;
    unsigned int value = gtl_AudioController_GetOutputBuffer(gtl_AudioController_GetInstance());
    if (value != 0)
        value += kCpuToDspAddressBias;
    unsigned int low = 0;
    switch (self->sampleFormat) {
    case kAxAdpcm:
        low = value * 2 + 2;
        break;
    case kAxPcm16:
        low = value >> 1;
        break;
    case kAxPcm8:
        low = value;
        break;
    }
    unsigned int high = low;
    switch (self->sampleFormat) {
    case kAxAdpcm:
        high += kSilenceAdpcmNibbles;
        break;
    case kAxPcm16:
        high += kSilencePcm16Words;
        break;
    case kAxPcm8:
        high += kSilencePcm8Bytes;
        break;
    }
    int result = low <= position && position < high;
    OSRestoreInterrupts(level);
    return result;
}

struct Struct_80406E4C {
    void* item;
    char pad4[0x10];
    int mode;
};

// Set silence-loop and source-end addresses, then disable looping under interrupt protection.
void GtlVoice_SetNonLoopingSampleRange(Struct_80406E4C* self, unsigned int sourceByteBase, unsigned int sampleCount) {
    BOOL level = OSDisableInterrupts();
    if (self->item == 0) {
        OSRestoreInterrupts(level);
        return;
    }
    unsigned int silenceByteBase = gtl_AudioController_GetOutputBuffer(gtl_AudioController_GetInstance());
    if (silenceByteBase != 0)
        silenceByteBase += kCpuToDspAddressBias;
    unsigned int loopDspAddress = 0;
    switch (self->mode) {
    case kAxAdpcm:
        loopDspAddress = silenceByteBase * 2 + 2;
        break;
    case kAxPcm16:
        loopDspAddress = silenceByteBase >> 1;
        break;
    case kAxPcm8:
        loopDspAddress = silenceByteBase;
        break;
    }
    if (sourceByteBase != 0)
        sourceByteBase += kCpuToDspAddressBias;
    unsigned int inclusiveEndDspAddress = 0;
    switch (self->mode) {
    case kAxAdpcm: {
        unsigned int adjusted = sampleCount - 1;
        unsigned int frameIndex = adjusted / kAdpcmSamplesPerFrame;
        unsigned int sampleInFrame = adjusted - frameIndex * kAdpcmSamplesPerFrame;
        inclusiveEndDspAddress =
            sourceByteBase * 2 + sampleInFrame + frameIndex * kAdpcmNibblesPerFrame + kAdpcmHeaderNibbles;
        break;
    }
    case kAxPcm16:
        inclusiveEndDspAddress = sampleCount + (sourceByteBase >> 1) - 1;
        break;
    case kAxPcm8:
        inclusiveEndDspAddress = sourceByteBase + sampleCount - 1;
        break;
    }
    BOOL nested = OSDisableInterrupts();
    if (self->item != 0) {
        ((AxVoiceParameters*)self->item)->loopAddressHi = (unsigned short)(loopDspAddress >> 16);
        ((AxVoiceParameters*)self->item)->loopAddressLo = (unsigned short)loopDspAddress;
        unsigned int flags = ((AxVoiceParameters*)self->item)->syncFlags;
        if ((flags & kAxSyncAllAddresses) == 0)
            ((AxVoiceParameters*)self->item)->syncFlags = flags | kAxSyncLoopAddress;
    }
    OSRestoreInterrupts(nested);
    nested = OSDisableInterrupts();
    if (self->item != 0) {
        ((AxVoiceParameters*)self->item)->endAddressHi = (unsigned short)(inclusiveEndDspAddress >> 16);
        ((AxVoiceParameters*)self->item)->endAddressLo = (unsigned short)inclusiveEndDspAddress;
        unsigned int flags = ((AxVoiceParameters*)self->item)->syncFlags;
        if ((flags & kAxSyncAllAddresses) == 0)
            ((AxVoiceParameters*)self->item)->syncFlags = flags | kAxSyncEndAddress;
    }
    OSRestoreInterrupts(nested);
    nested = OSDisableInterrupts();
    if (self->item != 0) {
        ((AxVoiceParameters*)self->item)->loopFlag = 0;
        unsigned int flags = ((AxVoiceParameters*)self->item)->syncFlags;
        if ((flags & kAxSyncAllAddresses) == 0)
            ((AxVoiceParameters*)self->item)->syncFlags = flags | kAxSyncLoopFlag;
    }
    OSRestoreInterrupts(nested);
    OSRestoreInterrupts(level);
}

struct AxLoopCursorView {
    char pad[0x9A];
    unsigned short loopAddressHigh, loopAddressLow;
};
struct VoiceLoopAddressView {
    AxLoopCursorView* hardwareVoice;
    char pad4[0x10];
    int sampleFormat;
};

// Convert a sample offset to an ADPCM nibble, PCM16 word or PCM8 byte address.
static unsigned int SampleOffsetToDspAddress(unsigned int sourceByteBase, unsigned int sampleOffset, int sampleFormat) {
    if (sampleFormat == kAxAdpcm) {
        unsigned int frameIndex = sampleOffset / kAdpcmSamplesPerFrame;
        unsigned int sampleInFrame = sampleOffset - frameIndex * kAdpcmSamplesPerFrame;
        return sourceByteBase * 2 + sampleInFrame + frameIndex * kAdpcmNibblesPerFrame + kAdpcmHeaderNibbles;
    }
    if (sampleFormat == kAxPcm16)
        return (sourceByteBase >> 1) + sampleOffset;
    if (sampleFormat == kAxPcm8)
        return sourceByteBase + sampleOffset;
    return 0;
}

// Update the DSP loop address and its synchronization flag.
void gtl_Voice_SetLoopSampleAddress(VoiceLoopAddressView* self, unsigned int sourceByteBase,
                                    unsigned int sampleOffset) {
    BOOL level = OSDisableInterrupts();
    if (self->hardwareVoice == 0) {
        OSRestoreInterrupts(level);
        return;
    }
    if (sourceByteBase != 0)
        sourceByteBase += kCpuToDspAddressBias;
    unsigned int dspAddress = 0;
    switch (self->sampleFormat) {
    case kAxAdpcm: {
        unsigned int frameIndex = sampleOffset / kAdpcmSamplesPerFrame;
        unsigned int sampleInFrame = sampleOffset - frameIndex * kAdpcmSamplesPerFrame;
        dspAddress = sourceByteBase * 2 + sampleInFrame + frameIndex * kAdpcmNibblesPerFrame + kAdpcmHeaderNibbles;
        break;
    }
    case kAxPcm16:
        dspAddress = (sourceByteBase >> 1) + sampleOffset;
        break;
    case kAxPcm8:
        dspAddress = sourceByteBase + sampleOffset;
        break;
    }
    BOOL nested = OSDisableInterrupts();
    AxLoopCursorView* item = self->hardwareVoice;
    if (item != 0) {
        item->loopAddressHigh = (unsigned short)(dspAddress >> 16);
        item->loopAddressLow = (unsigned short)dspAddress;
        unsigned int flags = ((AxVoiceParameters*)item)->syncFlags;
        if ((flags & kAxSyncAllAddresses) == 0)
            ((AxVoiceParameters*)item)->syncFlags = flags | kAxSyncLoopAddress;
    }
    OSRestoreInterrupts(nested);
    OSRestoreInterrupts(level);
}

// Update the inclusive DSP end address from an exclusive sample count.
void GtlVoice_SetEndSampleAddress(VoiceLoopAddressView* self, unsigned int sourceByteBase, unsigned int sampleOffset) {
    BOOL level = OSDisableInterrupts();
    if (self->hardwareVoice == 0) {
        OSRestoreInterrupts(level);
        return;
    }
    if (sourceByteBase != 0)
        sourceByteBase += kCpuToDspAddressBias;
    unsigned int dspAddress = SampleOffsetToDspAddress(sourceByteBase, sampleOffset - 1, self->sampleFormat);
    BOOL nested = OSDisableInterrupts();
    AxLoopCursorView* item = self->hardwareVoice;
    if (item != 0) {
        ((AxVoiceParameters*)item)->endAddressHi = (unsigned short)(dspAddress >> 16);
        ((AxVoiceParameters*)item)->endAddressLo = (unsigned short)dspAddress;
        unsigned int flags = ((AxVoiceParameters*)item)->syncFlags;
        if ((flags & kAxSyncAllAddresses) == 0)
            ((AxVoiceParameters*)item)->syncFlags = flags | kAxSyncEndAddress;
    }
    OSRestoreInterrupts(nested);
    OSRestoreInterrupts(level);
}

// Install source byte-base/AX-format/sample-rate ABI words and reset the 12-lane mix cache.
// The legacy shared symbol says "Pointers", but format and rate are integers:
// GtlVoice_ConfigureSamplePlayback (gtl_voice_control.cpp) supplies them from the source.
// No source memory is copied or freed; +0x1C requests mix-cache initialization.
void AudioVoice_SetSampleSourceState(VoiceSourceAndMixView* self, void* baseWord, void* formatWord, void* rateWord) {
    BOOL level = OSDisableInterrupts();
    self->sampleByteBaseWord = (unsigned int)baseWord;
    self->sampleFormatWord = (unsigned int)formatWord;
    self->sampleRateWord = (unsigned int)rateWord;
    memset(self->mixCache, 0, 0x18);
    self->resetMix = 1;
    OSRestoreInterrupts(level);
}

struct Struct_804073A0 {
    void* current;
    unsigned int flags;
};
struct Item_804073A0 {
    char pad[0x3A];
    short value;
};
// Write AXVPB.pb.type at 0x28 + 0x12 = 0x3A; bit 3 is AX_PBSYNC_TYPE
// (see revolution/AX/AXVPB.h and AXPB.h).
void AudioVoice_SetTypeIfAllocated(Struct_804073A0* self, short value) {
    BOOL level;
    Struct_804073A0* object;
    short input;
    Item_804073A0* item;
    object = self;
    input = value;
    level = OSDisableInterrupts();
    item = (Item_804073A0*)object->current;
    if (item == 0) {
        OSRestoreInterrupts(level);
    } else {
        item->value = input;
        object->flags |= kAxSyncType;
        OSRestoreInterrupts(level);
    }
}

struct Struct_80407414 {
    void* current;
    unsigned int flags;
    char pad[8];
};

// Write the DSP halfword at 0xFE and request synchronization bit 23; its role is unconfirmed.
void AudioVoice_SetDspParameterFEIfAllocated(Struct_80407414* self, short value) {
    BOOL level;
    Struct_80407414* object;
    short input;
    void* current;
    object = self;
    input = value;
    level = OSDisableInterrupts();
    current = object->current;
    if (current == 0) {
        OSRestoreInterrupts(level);
    } else {
        ((AxVoiceParameters*)current)->dspParameterFE = input;
        object->flags |= (unsigned int)kAxSyncParameterFE;
        OSRestoreInterrupts(level);
    }
}

struct Struct_80406DA4 {
    void* current;
};
struct Item_80406DA4 {
    char pad[0x96];
    short value;
};

// Update the DSP loop flag without overriding a pending full-address update.
void GtlVoice_SetLoopFlag(Struct_80406DA4* self, short value) {
    BOOL level = OSDisableInterrupts();
    void* current = self->current;
    if (current == 0) {
        OSRestoreInterrupts(level);
    } else {
        BOOL nestedLevel = OSDisableInterrupts();
        Item_80406DA4* item = (Item_80406DA4*)self->current;
        if (item == 0) {
            OSRestoreInterrupts(nestedLevel);
        } else {
            item->value = value;
            unsigned int flags = ((AxVoiceParameters*)item)->syncFlags;
            if ((flags & kAxSyncAllAddresses) == 0)
                ((AxVoiceParameters*)item)->syncFlags = flags | kAxSyncLoopFlag;
            OSRestoreInterrupts(nestedLevel);
        }
        OSRestoreInterrupts(level);
    }
}

// Report whether an allocated voice's nonzero sample base lies in the inclusive range.
int GtlVoice_IsSourceBaseInRange(void* self, unsigned int lower, unsigned int upper) {
    BOOL level = OSDisableInterrupts();
    void* current = *(void**)self;
    if (current == 0) {
        OSRestoreInterrupts(level);
        return 0;
    }
    unsigned int point = (*(unsigned int*)&((VoiceSourceState*)self)->sampleBase);
    int result = 0;
    if (point != 0 && lower <= point && point <= upper)
        result = 1;
    OSRestoreInterrupts(level);
    return result;
}

void* GtlVoicePool_GetInstance(void*);
void AudioVoicePool_QueuePendingRelease(void*, void*);
void InterruptGuard_dtor(void*, int);

// Map clamped [-1, 1] pan to the rounded 257-entry gain lookup.
float gtl_Audio_LookupPanGain(float value) {
    float bounded;
    if (value > 1.0f)
        bounded = 1.0f;
    else if (value < -1.0f)
        bounded = -1.0f;
    else
        bounded = value;
    float scaled = 0.5f * (1.0f + bounded);
    int index = (int)((float)kPanTableLastIndex * scaled + 0.5f);
    return s_audioPanGainTable[index];
}

// The original (0x804072D0) restores the saved level once, at 0x80407324: the guard's destructor inlined on the
// (an explicit OSRestoreInterrupts plus two InterruptGuard_dtor calls).
struct InterruptLevelGuard {
    BOOL level;
    // Restore exactly the interrupt level saved by the enclosing pool operation.
    ~InterruptLevelGuard() {
        OSRestoreInterrupts(level);
    }
};

struct Struct_804072D0 {
    char pad[0x14];
    unsigned int* values;
};
// Clear a voice value pair and return it to the pool while interrupts are guarded.
void GtlVoice_ReturnValuePairToPool(Struct_804072D0* self) {
    InterruptLevelGuard guard;
    guard.level = OSDisableInterrupts();
    unsigned int* values = self->values;
    values[0] = 0;
    values[1] = 0;
    void* temporary = GtlVoicePool_GetInstance(&guard);
    AudioVoicePool_QueuePendingRelease(temporary, values);
}

// Convert encoded byte offsets to samples for ADPCM, PCM16 or PCM8.
unsigned int AudioStream_ByteOffsetToSampleOffset(unsigned int value, int mode) {
    unsigned int result = 0;
    switch (mode) {
    case kAxAdpcm: {
        unsigned int remainder = value & kAdpcmEncodedByteMask;
        result = (value >> 3) * kAdpcmSamplesPerFrame;
        if (remainder != 0)
            result += (remainder - 1) * 2;
        break;
    }
    case kAxPcm16:
        result = value >> 1;
        break;
    case kAxPcm8:
        result = value;
        break;
    }
    return result;
}

void AudioVoice_SetMixCoefficients(void*, void*, int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);

struct VoicePositionView {
    void* hardwareVoice;
    char reserved04[0xC];
    unsigned int sampleByteBase;
    int sampleFormat;
};
struct DspAddressWord {
    unsigned short bits;
};
enum DspCursorOffsets {
    kDspEndAddress = (int)&((AxVoiceParameters*)0)->endAddressHi,
    kDspCurrentAddress = (int)&((AxVoiceParameters*)0)->currentAddressHi
};
// Read the DSP cursor and convert it back to a source-relative sample offset.
int GtlVoice_GetSamplePosition(VoicePositionView* self) {
    BOOL level = OSDisableInterrupts();
    if (!self->hardwareVoice || !self->sampleByteBase) {
        OSRestoreInterrupts(level);
        return 0;
    }
    // Phase 1: if the current cursor is in silence, report the saved source end.
    int readingSilence = GtlVoice_IsReadingOutputBuffer((VoiceSilenceProbeView*)self);
    void* hardwareVoice = self->hardwareVoice;
    unsigned int dspCursor = 0;
    if (hardwareVoice) {
        unsigned short addressHigh =
            ((DspAddressWord*)((char*)hardwareVoice + (readingSilence ? kDspEndAddress : kDspCurrentAddress)))->bits;
        unsigned short addressLow =
            ((DspAddressWord*)((char*)hardwareVoice + (readingSilence ? kDspEndAddress + 2 : kDspCurrentAddress + 2)))
                ->bits;
        dspCursor = ((unsigned int)addressHigh << 16) + addressLow;
    }
    unsigned int dspByteBase = self->sampleByteBase;
    if (dspByteBase)
        dspByteBase += kCpuToDspAddressBias;
    // Phase 2: subtract the source base in the DSP format, then count samples.
    unsigned int sampleOffset = 0;
    if (self->sampleFormat == kAxAdpcm) {
        unsigned int diff = dspCursor - dspByteBase * 2;
        sampleOffset = (diff & kAdpcmFrameNibbleMask) + (diff >> kAdpcmFrameNibbleShift) * kAdpcmSamplesPerFrame -
                       kAdpcmHeaderNibbles;
    } else if (self->sampleFormat == kAxPcm16)
        sampleOffset = dspCursor - (dspByteBase >> 1);
    else if (self->sampleFormat == kAxPcm8)
        sampleOffset = dspCursor - dspByteBase;
    OSRestoreInterrupts(level);
    // Source end is inclusive; silence means that final sample has been consumed.
    return readingSilence ? sampleOffset + 1 : sampleOffset;
}

struct Struct_80407488 {
    void* current;
    char pad4[4];
    unsigned short value8;
    char padA[0x14];
    unsigned short v1e, v20, v22, v24, v26, v28, v2a, v2c, v2e, v30, v32, v34;
};
// Fold the current volume delta into volume and zero all mix/volume ramps.
void GtlVoice_StopMixAndVolumeRamps(Struct_80407488* self) {
    BOOL level = OSDisableInterrupts();
    if (!self->current) {
        OSRestoreInterrupts(level);
        return;
    }
    BOOL nested = OSDisableInterrupts();
    void* item = self->current;
    if (item) {
        short count = ((AxVoiceParameters*)item)->volumeDelta;
        self->value8 = (unsigned short)(self->value8 + count * kRampOutputSamples);
        ((AxVoiceParameters*)item)->volume = self->value8;
        ((AxVoiceParameters*)item)->volumeDelta = 0;
        ((AxVoiceParameters*)item)->syncFlags |= kAxSyncVolume;
    }
    OSRestoreInterrupts(nested);
    unsigned short data[24] = {self->v1e, 0, self->v20, 0, self->v24, 0, self->v26, 0, self->v2a, 0, self->v2c, 0,
                               self->v30, 0, self->v32, 0, self->v22, 0, self->v28, 0, self->v2e, 0, self->v34, 0};
    AudioVoice_SetMixCoefficients(self, data, 1);
    OSRestoreInterrupts(level);
}

struct Struct_80407A7C {
    void* current;
    unsigned int flags;
    char pad8[0xC];
    int mode;
};
// Install source ADPCM state or format-specific PCM defaults and mark it dirty.
void GtlVoice_SetAdpcmParameters(Struct_80407A7C* self, unsigned short* input) {
    BOOL level = OSDisableInterrupts();
    if (!self->current) {
        OSRestoreInterrupts(level);
        return;
    }
    unsigned char data[0x28];
    if (self->mode == kAxAdpcm) {
        memcpy(data, input, 0x20);
        memcpy(data + 0x20, input + 16, 8);
    } else {
        memset(data, 0, 0x20);
        unsigned short* v = (unsigned short*)(data + 0x20);
        v[0] = self->mode == kAxPcm16 ? 0x800 : 0x100;
        v[1] = v[2] = v[3] = 0;
    }
    BOOL nested = OSDisableInterrupts();
    void* item = self->current;
    if (item) {
        memcpy(((AxVoiceParameters*)item)->adpcm, data, 0x28);
        self->flags |= kAxSyncAdpcm;
    }
    OSRestoreInterrupts(nested);
    OSRestoreInterrupts(level);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626B8", _ctors_u804064C0_0, AudioLookup_InitializeAngularConstant);
STATIC_INIT_ENTRY(".CRT$XCU804626B8", _ctors_u804064C0_1, AudioVoice_InitializeAngularConstant);
#pragma pop
