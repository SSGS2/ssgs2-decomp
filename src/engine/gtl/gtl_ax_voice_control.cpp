// GTL AX voice control: volume envelopes and ramps, pitch and sample-rate interpolation, mix levels,
// biquad and low-pass filters, and the voice pool. The AX parameter block is borrowed, not owned.

#include <decomp/static_init.h>
// GTL AX audio voice control: envelope, pitch/SRC, mixer lanes, filters and
// ADPCM loop decoder context. Mod index: normalized envelope levels ->
// gtl_AxVoice_SetVolumeEnvelope/GtlVoice_UpdateVolumeRamp; playback-rate multiplier ->
// gtl_AxVoice_SetPitch; loop restart history -> AudioVoice_SetAdpcmLoopContext.
// SDK layout/flags: revolution/AX/AXPB.h and AXVPB.h. Wrapper flags accumulate
// changes for later DSP synchronization; these routines borrow the live AXVPB
// and restore nested OS interrupt levels. The wrapper itself must be nonnull;
// an absent object/PB makes parameter setters no-ops. No ownership transfer.

#include "sdk_protos.h"
extern "C" void* memcpy(void*, const void*, unsigned int);

// AX parameter-block fields written by the voice wrapper, in DSP ABI order.
// All pointers below are borrowed; interrupt levels must be restored in nesting order.
// Levels are 16-bit DSP words; deltas are signed 16-bit words per output sample.
// Twelve-lane cache order (AX main plus three auxiliary buses):
//   0 main L, 1 main R, 2 main S; 3 aux A L, 4 aux A R, 5 aux A S;
//   6 aux B L, 7 aux B R, 8 aux B S; 9 aux C L, 10 aux C R, 11 aux C S.
// S is the surround lane. PB pair order puts all L/R pairs before the four S pairs.
// The separate eight-pair path at +0x102 is not this twelve-lane cache; this unit
// establishes only its positional ABI, not the destinations of its eight channels.
struct AxParameterBlock {
    char reserved00[0x1c];
    unsigned int syncFlags;
    char reserved20[0x10];
    unsigned short sourceType, sourceCoefficient;
    unsigned int mixerControl;
    unsigned short running;
    char reserved3a[2];
    unsigned short mixPairs[24];
    char reserved6c[0x26];
    unsigned short volume;
    short volumeStep;
    char reserved96[0x38];
    unsigned short sourceRatioHigh, sourceRatioLow;
    unsigned short sourceHistory[5];
    // ADPCM loop history in the AX parameter block:
    // AXVPB+DC = pb(+28).adpcmLoop(+B4), NOT an auxiliary mixer block.
    // Three DSP words: loop_pred_scale, loop_yn1, loop_yn2 (AXPBADPCMLOOP).
    unsigned short adpcmLoopContext[3];
    unsigned short lowPass[4];
    char reservedea[0x16];
    unsigned short eightMixMask, eightMixPairs[16];
    char reserved122[0x1a];
    unsigned short biquad[10];
};
struct AxVoiceMixCache {
    unsigned int object, flags;
    char reserved08[0x14];
    unsigned char resetMix;
    char reserved1d;
    unsigned short currentMix[12];
};
#define AX_LAYOUT_OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
AX_LAYOUT_OFF(AxParameterBlock, syncFlags, 0x1c);
AX_LAYOUT_OFF(AxParameterBlock, sourceType, 0x30);
AX_LAYOUT_OFF(AxParameterBlock, mixPairs, 0x3c);
AX_LAYOUT_OFF(AxParameterBlock, volume, 0x92);
AX_LAYOUT_OFF(AxParameterBlock, sourceRatioHigh, 0xce);
AX_LAYOUT_OFF(AxParameterBlock, adpcmLoopContext, 0xdc);
AX_LAYOUT_OFF(AxParameterBlock, lowPass, 0xe2);
AX_LAYOUT_OFF(AxParameterBlock, eightMixPairs, 0x102);
AX_LAYOUT_OFF(AxParameterBlock, biquad, 0x13c);
AX_LAYOUT_OFF(AxVoiceMixCache, currentMix, 0x1e);
#undef AX_LAYOUT_OFF

enum AxVoiceConstants {
    kMixCacheOffset = (int)&((AxVoiceMixCache*)0)->currentMix,
    kRampSamples = 96,
    kMaxVolume = 0x7fff,
    kSyncSourceType = 1,
    kSyncMix = 0x12,
    kSyncVolume = 0x100,
    kSyncVolumeDelta = 0x200,
    kSyncSourceReset = 0x10000,
    kSyncSourceRatio = 0x20000,
    kSyncAdpcmLoop = 0x40000,
    kSyncLowPass = 0x80000,
    kSyncLowPassCoefficients = 0x100000,
    kSyncEightMix = 0x03000000,
    kSyncBiquad = 0x10000000,
    kSyncTwoFilterCoefficients = 0x20000000,
    kSyncFiveFilterCoefficients = 0x40000000,
    kFilterTwoCoefficients = 1,
    kFilterFiveCoefficients = 2,
    kLowPassBypassFrequency = 16000
};
enum VoiceInterpolationMode {
    kSourceNoInterpolation = 0,
    kSourceLinear = 1,
    kSourceTable0 = 2,
    kSourceTable1 = 3,
    kSourceTable2 = 4
};
enum AxInterpolationType {
    kAxSourceTable = 0,
    kAxSourceLinear = 1,
    kAxSourceNoInterpolation = 2
};

extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_AudioVoice;

void AudioVoice_SetEightMixPairs(void*, unsigned short*);
void AudioVoice_SetMixCoefficients(unsigned int*, unsigned short*, int);
void AudioControl_GetFiveLookupCoefficients(void*, unsigned short*, unsigned short*, unsigned short*, unsigned short*,
                                            unsigned short*);
void AXGetLpfCoefs(unsigned int, unsigned short*, unsigned short*);
void GtlVoicePool_dtor(void*);
void InterruptGuard_dtor(void*, int);
void dtor_804068E8(void*, int);
extern signed char s_audioVoicePoolInitialized;
extern unsigned int s_audioVoicePoolDtorRegistration[3];
extern float kFloat_32767_AudioVolume;
void* __va_arg(void*, unsigned int);

// Plain saved-level holders: the original restores each level exactly once and calls no destructor.
struct SavedInterruptLevel {
    unsigned int level;
};
struct MixInterruptLevel {
    unsigned int level;
};

struct VoiceEnvelopeState {
    unsigned int object;
    unsigned char reserved04[4];
    unsigned short currentVolume;
    unsigned char reserved0a[2];
    unsigned char pending;
    unsigned char reserved0d;
    unsigned short targetVolume;
};

// Set normalized linear amplitude: 0 = silence, 1 = full scale (DSP 32767).
// targetLevel/initialLevel should be finite in [0,1]; neither is clamped here.
// Conversion multiplies in float then narrows to u16; out-of-range behavior is
// not a supported gain boost. currentVolume changes only when pending is set,
// clearing that flag; targetVolume changes on every request with a live PB.
// GtlVoice_UpdateVolumeRamp consumes these Q15-scale words in 96-sample ramps.
void gtl_AxVoice_SetVolumeEnvelope(VoiceEnvelopeState* voice, float targetLevel, float initialLevel) {
    SavedInterruptLevel lock;
    lock.level = OSDisableInterrupts();
    if (voice->object) {
        SavedInterruptLevel lock2;
        lock2.level = OSDisableInterrupts();
        if (voice->object) {
            if (voice->pending) {
                voice->pending = 0;
                voice->currentVolume = (unsigned short)(kFloat_32767_AudioVolume * initialLevel);
            }
            voice->targetVolume = (unsigned short)(kFloat_32767_AudioVolume * targetLevel);
            OSRestoreInterrupts(lock2.level);
        } else {
            OSRestoreInterrupts(lock2.level);
        }
    }
    OSRestoreInterrupts(lock.level);
}

// Select the AX sample-rate-converter mode and mark its source configuration dirty.
void AudioVoice_SetSourceInterpolationMode(unsigned int* voice, int sourceMode) {
    // API -> AX SRC type/coefficient: 0 -> 2/unchanged, 1 -> 1/unchanged,
    // 2 -> 0/0, 3 -> 0/1, 4 -> 0/2. AX types 2/1/0 select no interpolation,
    // linear interpolation, and coefficient-table interpolation respectively.
    // Coefficient-table frequency bands are not established here. An unknown
    // API mode still marks source sync dirty without changing the PB fields.
    MixInterruptLevel lock;
    lock.level = OSDisableInterrupts();
    if (*voice) {
        switch (sourceMode) {
        case kSourceNoInterpolation:
            ((AxParameterBlock*)*voice)->sourceType = kAxSourceNoInterpolation;
            break;
        case kSourceLinear:
            ((AxParameterBlock*)*voice)->sourceType = kAxSourceLinear;
            break;
        case kSourceTable0:
            ((AxParameterBlock*)*voice)->sourceType = kAxSourceTable;
            ((AxParameterBlock*)*voice)->sourceCoefficient = 0;
            break;
        case kSourceTable1:
            ((AxParameterBlock*)*voice)->sourceType = kAxSourceTable;
            ((AxParameterBlock*)*voice)->sourceCoefficient = 1;
            break;
        case kSourceTable2:
            ((AxParameterBlock*)*voice)->sourceType = kAxSourceTable;
            ((AxParameterBlock*)*voice)->sourceCoefficient = 2;
            break;
        }
        voice[1] |= 1;
    }
    OSRestoreInterrupts(lock.level);
}

// Write two or five filter coefficients from promoted varargs and set the matching sync flag.
void AudioVoice_SetFilterCoefficients(unsigned int* voice, unsigned int filterType, ...) {
    // Varargs are promoted integer coefficient bit patterns, not floats.
    // The shared filter block is [mode, history, two coefficients, history,
    // five coefficients]. Mode 1 updates slots 2..3, mode 2 slots 5..9.
    // This routine does not change mode/history. Exact fixed-point scaling and
    // feedback sign convention belong to the coefficient providers/DSP ABI.
    SavedInterruptLevel lock;
    lock.level = OSDisableInterrupts();
    if (!*voice) {
        OSRestoreInterrupts(lock.level);
        return;
    }
    unsigned int count;
    if (filterType == kFilterTwoCoefficients)
        count = 2;
    else if (filterType == kFilterFiveCoefficients)
        count = 5;
    else {
        OSRestoreInterrupts(lock.level);
        return;
    }
    unsigned short values[5];
    struct {
        unsigned int a;
        void* b;
        void* c;
    } ap; // va_list {gpr, fpr, reserved, overflow area, register save area}
    __va_setup(
        &ap); // same compiler intrinsic as Stream_ScanFormat; &voice is the spill slot, not the varargs save area
    for (unsigned int i = 0; i < count; ++i) {
        void* p = __va_arg(&ap, 1);
        values[i] = (unsigned short)*(unsigned int*)p;
    }
    if (filterType == kFilterTwoCoefficients) {
        ((AxParameterBlock*)*voice)->biquad[2] = values[0];
        ((AxParameterBlock*)*voice)->biquad[3] = values[1];
        voice[1] |= kSyncTwoFilterCoefficients;
    } else {
        ((AxParameterBlock*)*voice)->biquad[5] = values[0];
        ((AxParameterBlock*)*voice)->biquad[6] = values[1];
        ((AxParameterBlock*)*voice)->biquad[7] = values[2];
        ((AxParameterBlock*)*voice)->biquad[8] = values[3];
        ((AxParameterBlock*)*voice)->biquad[9] = values[4];
        voice[1] |= kSyncFiveFilterCoefficients;
    }
    OSRestoreInterrupts(lock.level);
}

// Copy the saved parameter tail and return its final three halfword values.
void AudioVoice_ReadParameterTailTriplet(unsigned short* arg0, unsigned short* arg1, unsigned short* arg2, void* arg3,
                                         void* arg4, void* arg5) {
    // arg5 borrows at least 40 readable bytes; outputs receive halfwords 17,18,19
    // in native order. arg3/arg4 are unused ABI positions. No pointer is retained.
    unsigned char temp[40];
    memcpy(temp, arg5, 32);
    unsigned short* source = (unsigned short*)arg5;
    unsigned short a = source[16];
    unsigned short b = source[17];
    unsigned short c = source[18];
    unsigned short d = source[19];
    *(unsigned short*)(temp + 0x20) = a;
    *(unsigned short*)(temp + 0x22) = b;
    *(unsigned short*)(temp + 0x24) = c;
    *(unsigned short*)(temp + 0x26) = d;
    *arg0 = b;
    *arg1 = *(unsigned short*)(temp + 0x24);
    *arg2 = *(unsigned short*)(temp + 0x26);
}

extern unsigned int s_audioVoicePool[];

// Lazily initialize three empty voice lists and register the pool destructor.
unsigned int* GtlVoicePool_GetInstance() {
    if (!s_audioVoicePoolInitialized) {
        unsigned int* p = s_audioVoicePool;
        p[1] = 0;
        p[2] = 0;
        p[4] = 0;
        p[5] = 0;
        p[7] = 0;
        p[8] = 0;
        p[0] = 0;
        p[1] = (unsigned int)&p[1];
        p[2] = (unsigned int)&p[1];
        p[3] = 0;
        p[4] = (unsigned int)&p[4];
        p[5] = (unsigned int)&p[4];
        p[6] = 0;
        p[7] = (unsigned int)&p[7];
        p[8] = (unsigned int)&p[7];
        *(unsigned char*)(p + 9) = 0;
        __register_global_object(p, (void*)GtlVoicePool_dtor, &s_audioVoicePoolDtorRegistration);
        s_audioVoicePoolInitialized = 1;
    }
    return s_audioVoicePool;
}

struct VoiceParameterStorage {
    unsigned int object;
    unsigned int flags;
    char reserved08[0xCC];
    unsigned char data[6];
};

// Ramp twelve mix channels over 96 samples and report whether a target differed.
int AudioVoice_RampMixCoefficients(unsigned int* voice, unsigned short* targetMix) {
    // Phase 1: seed the cache from the target when stopped/reset; there is then
    // no change to report. A missing voice also returns 0. No arrays are retained.
    unsigned int level = OSDisableInterrupts();
    unsigned int object = *voice;
    int changed = 0;
    if (!object) {
        OSRestoreInterrupts(level);
        return 0;
    }
    if (((AxVoiceMixCache*)voice)->resetMix || ((AxParameterBlock*)object)->running != 1) {
        unsigned short* dst = (unsigned short*)((char*)voice + kMixCacheOffset);
        dst[0] = targetMix[0];
        dst[1] = targetMix[1];
        dst[2] = targetMix[2];
        dst[3] = targetMix[3];
        dst[4] = targetMix[4];
        dst[5] = targetMix[5];
        dst[6] = targetMix[6];
        dst[7] = targetMix[7];
        dst[8] = targetMix[8];
        dst[9] = targetMix[9];
        dst[10] = targetMix[10];
        dst[11] = targetMix[11];
        ((AxVoiceMixCache*)voice)->resetMix = 0;
    }
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 0) != targetMix[0])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 1) != targetMix[1])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 2) != targetMix[2])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 3) != targetMix[3])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 4) != targetMix[4])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 5) != targetMix[5])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 6) != targetMix[6])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 7) != targetMix[7])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 8) != targetMix[8])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 9) != targetMix[9])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 10) != targetMix[10])
        changed = 1;
    if (*((unsigned short*)((char*)voice + kMixCacheOffset) + 11) != targetMix[11])
        changed = 1;
    // AudioVoice_SetMixCoefficients takes 12 (current, step) pairs, in the same order GtlVoice_StopMixAndVolumeRamps builds them.
    unsigned short values[24];
    unsigned short step[12];
    unsigned short* cur = (unsigned short*)((char*)voice + kMixCacheOffset);
    // Phase 2: cache -> PB permutation: main LR, A LR, B LR, C LR, main/A/B/C S.
    // Truncation toward zero leaves a residual of less than 96 level units.
    static const unsigned char order[12] = {0, 1, 3, 4, 6, 7, 9, 10, 2, 5, 8, 11};
    for (int n = 0; n < 12; ++n)
        step[n] = (short)(((int)targetMix[n] - (int)cur[n]) / kRampSamples);
    for (int p = 0; p < 12; ++p) {
        values[p * 2] = cur[order[p]];
        values[p * 2 + 1] = step[order[p]];
    }
    AudioVoice_SetMixCoefficients(voice, values, 0);
    // Phase 3: predict the level after this 96-sample block. Silence and zero
    // delta snap the software cache to target; otherwise preserve the residual
    // for a later update. The return value records pre-ramp differences.
    if (!targetMix[0] || !step[0])
        cur[0] = targetMix[0];
    else
        cur[0] = (unsigned short)(cur[0] + step[0] * kRampSamples);
    if (!targetMix[1] || !step[1])
        cur[1] = targetMix[1];
    else
        cur[1] = (unsigned short)(cur[1] + step[1] * kRampSamples);
    if (!targetMix[2] || !step[2])
        cur[2] = targetMix[2];
    else
        cur[2] = (unsigned short)(cur[2] + step[2] * kRampSamples);
    if (!targetMix[3] || !step[3])
        cur[3] = targetMix[3];
    else
        cur[3] = (unsigned short)(cur[3] + step[3] * kRampSamples);
    if (!targetMix[4] || !step[4])
        cur[4] = targetMix[4];
    else
        cur[4] = (unsigned short)(cur[4] + step[4] * kRampSamples);
    if (!targetMix[5] || !step[5])
        cur[5] = targetMix[5];
    else
        cur[5] = (unsigned short)(cur[5] + step[5] * kRampSamples);
    if (!targetMix[6] || !step[6])
        cur[6] = targetMix[6];
    else
        cur[6] = (unsigned short)(cur[6] + step[6] * kRampSamples);
    if (!targetMix[7] || !step[7])
        cur[7] = targetMix[7];
    else
        cur[7] = (unsigned short)(cur[7] + step[7] * kRampSamples);
    if (!targetMix[8] || !step[8])
        cur[8] = targetMix[8];
    else
        cur[8] = (unsigned short)(cur[8] + step[8] * kRampSamples);
    if (!targetMix[9] || !step[9])
        cur[9] = targetMix[9];
    else
        cur[9] = (unsigned short)(cur[9] + step[9] * kRampSamples);
    if (!targetMix[10] || !step[10])
        cur[10] = targetMix[10];
    else
        cur[10] = (unsigned short)(cur[10] + step[10] * kRampSamples);
    if (!targetMix[11] || !step[11])
        cur[11] = targetMix[11];
    else
        cur[11] = (unsigned short)(cur[11] + step[11] * kRampSamples);
    OSRestoreInterrupts(level);
    return changed;
}

struct VoiceRampState {
    unsigned int object;
    unsigned int flags;
    unsigned short currentVolume;
    short volumeStep;
    unsigned char resetVolume[2];
    unsigned short targetVolume;
    unsigned char reserved10[8];
    unsigned int sampleRateHz;
};

struct VoiceInterruptLevel {
    unsigned int level;
};

// Write eight level/step pairs and build the nonzero-halfword enable mask.
void AudioVoice_SetEightMixPairs(void* voiceStorage, unsigned short* mixPairs) {
    unsigned int* voice = (unsigned int*)voiceStorage;
    unsigned int level = OSDisableInterrupts();
    unsigned int object = *voice;
    if (!object) {
        OSRestoreInterrupts(level);
    } else {
        unsigned short mask = 0;
        unsigned short* dst = ((AxParameterBlock*)object)->eightMixPairs;
        for (int i = 0; i < 16; ++i) {
            unsigned short value = mixPairs[i];
            dst[i] = value;
            if (value)
                mask |= (unsigned short)(1 << i);
        }
        ((AxParameterBlock*)object)->eightMixMask = mask;
        voice[1] |= kSyncEightMix;
        OSRestoreInterrupts(level);
    }
}

// Store one mixer halfword and add its channel-enable bits when it is nonzero.
inline void copyMixValue(unsigned int object, unsigned short value, int index, unsigned int bit, unsigned int& mask) {
    ((AxParameterBlock*)object)->mixPairs[index] = value;
    if (value)
        mask |= bit;
}

// Write twelve level/step pairs, build mixer enable bits and optionally sync immediately.
void AudioVoice_SetMixCoefficients(unsigned int* voice, unsigned short* mixPairs, int syncImmediately) {
    // Each adjacent pair is {level, signed delta bits}. Control bits by PB pair:
    //   main L/R: 0/1, shared LR delta: 2; main S: 3, delta: 4;
    //   aux A L/R: 16/17, LR delta: 18; A S: 19, delta: 20;
    //   aux B L/R: 21/22, LR delta: 23; B S: 24, delta: 25;
    //   aux C L/R: 26/27, LR delta: 28; C S: 29, delta: 30.
    // A nonzero delta enables both its channel and delta processing. Test the
    // raw halfword, including negative delta encodings. Zero disables that term.
    MixInterruptLevel lock;
    lock.level = OSDisableInterrupts();
    unsigned int object = *voice;
    if (object) {
        unsigned int mask = 0;
        copyMixValue(object, mixPairs[0], 0, 0x00000001, mask);
        copyMixValue(object, mixPairs[1], 1, 0x00000005, mask);
        copyMixValue(object, mixPairs[2], 2, 0x00000002, mask);
        copyMixValue(object, mixPairs[3], 3, 0x00000006, mask);
        copyMixValue(object, mixPairs[4], 4, 0x00010000, mask);
        copyMixValue(object, mixPairs[5], 5, 0x00050000, mask);
        copyMixValue(object, mixPairs[6], 6, 0x00020000, mask);
        copyMixValue(object, mixPairs[7], 7, 0x00060000, mask);
        copyMixValue(object, mixPairs[8], 8, 0x00200000, mask);
        copyMixValue(object, mixPairs[9], 9, 0x00A00000, mask);
        copyMixValue(object, mixPairs[10], 10, 0x00400000, mask);
        copyMixValue(object, mixPairs[11], 11, 0x00C00000, mask);
        copyMixValue(object, mixPairs[12], 12, 0x04000000, mask);
        copyMixValue(object, mixPairs[13], 13, 0x14000000, mask);
        copyMixValue(object, mixPairs[14], 14, 0x08000000, mask);
        copyMixValue(object, mixPairs[15], 15, 0x18000000, mask);
        copyMixValue(object, mixPairs[16], 16, 0x00000008, mask);
        copyMixValue(object, mixPairs[17], 17, 0x00000018, mask);
        copyMixValue(object, mixPairs[18], 18, 0x00080000, mask);
        copyMixValue(object, mixPairs[19], 19, 0x00180000, mask);
        copyMixValue(object, mixPairs[20], 20, 0x01000000, mask);
        copyMixValue(object, mixPairs[21], 21, 0x03000000, mask);
        copyMixValue(object, mixPairs[22], 22, 0x20000000, mask);
        copyMixValue(object, mixPairs[23], 23, 0x60000000, mask);
        ((AxParameterBlock*)object)->mixerControl = mask;
        if (syncImmediately)
            ((AxParameterBlock*)object)->syncFlags |= 0x12;
        else
            voice[1] |= 0x12;
    }
    OSRestoreInterrupts(lock.level);
}

// pitch is a dimensionless playback-rate multiplier, not a semitone offset.
// sampleRateHz/32000 converts source Hz to the 32 kHz DSP output timebase;
// ratio = pitch*sampleRateHz/32000, stored unsigned 16.16, high word then low.
// Example: pitch=1, source=32000 Hz -> {1,0}; source=16000 -> {0,0x8000}.
// Requires finite nonnegative representable conversion; no clamping or hardware
// maximum is established here. Preserve float evaluation order in both branches.
// resetHistory!=0 also clears fractional source address and four sample-history
// words, replacing ratio-only sync with full SRC reset. Otherwise retain history
// and set ratio-only sync unless full reset is already pending.
void gtl_AxVoice_SetPitch(VoiceRampState* voice, float pitch, int resetHistory) {
    SavedInterruptLevel lock;
    lock.level = OSDisableInterrupts();
    if (!voice->object) {
        OSRestoreInterrupts(lock.level);
        return;
    } else if (resetHistory) {
        unsigned int ticks =
            Runtime_ConvertDoubleToUnsigned(65536.0f * (pitch * (float)(int)voice->sampleRateHz / 32000.0f));
        unsigned short values[7] = {(unsigned short)(ticks >> 16), (unsigned short)ticks, 0, 0, 0, 0, 0};
        SavedInterruptLevel lock2;
        lock2.level = OSDisableInterrupts();
        if (voice->object) {
            memcpy((char*)&((AxParameterBlock*)voice->object)->sourceRatioHigh, values, 0x0E);
            voice->flags = (voice->flags & 0xFFFDFFFF) | 0x00010000;
        }
        OSRestoreInterrupts(lock2.level);
    } else {
        float fraction = pitch * (float)(int)voice->sampleRateHz / 32000.0f;
        SavedInterruptLevel lock2;
        lock2.level = OSDisableInterrupts();
        unsigned int object = voice->object;
        if (object) {
            unsigned int ticks = Runtime_ConvertDoubleToUnsigned(65536.0f * fraction);
            ((AxParameterBlock*)object)->sourceRatioHigh = (unsigned short)(ticks >> 16);
            ((AxParameterBlock*)object)->sourceRatioLow = (unsigned short)ticks;
            if (!(voice->flags & 0x00010000))
                voice->flags |= kSyncSourceRatio;
        }
        OSRestoreInterrupts(lock2.level);
    }
    OSRestoreInterrupts(lock.level);
}

// Choose a bounded 96-sample volume step, publish pending flags and clear the wrapper flags.
void GtlVoice_UpdateVolumeRamp(VoiceRampState* voice) {
    // Phase 1: compare endpoints for delta/96 (truncated) and a one-unit step.
    // Preserve the unusual fallback exactly: direction is +1 when steps != 0,
    // otherwise -1; it is NOT sign(delta). Ties select the fallback candidate.
    // Time is one 96-sample AX block (3 ms at the 32 kHz output rate).
    VoiceInterruptLevel lock;
    lock.level = OSDisableInterrupts();
    unsigned int object = voice->object;
    if (object) {
        ((AxParameterBlock*)object)->volume = voice->currentVolume;
        int delta = (int)voice->targetVolume - (int)voice->currentVolume;
        int steps = delta / kRampSamples;
        int direction = steps ? 1 : -1;
        int candidate1 = (int)voice->currentVolume + direction * kRampSamples;
        int candidate2 = (int)voice->currentVolume + steps * kRampSamples;
        int dist1 = (int)voice->targetVolume - candidate1;
        int dist2 = (int)voice->targetVolume - candidate2;
        if (dist1 < 0)
            dist1 = -dist1;
        if (dist2 < 0)
            dist2 = -dist2;
        short selected = dist2 < dist1 ? (short)steps : (short)direction;
        ((AxParameterBlock*)object)->volumeStep = selected;
        if (((AxParameterBlock*)object)->volumeStep == 0 && voice->volumeStep == 0)
            ((AxParameterBlock*)object)->volume = voice->targetVolume;
        int end = (int)voice->currentVolume + ((AxParameterBlock*)object)->volumeStep * kRampSamples;
        // Phase 2: bound the predicted endpoint to [0, 32767]. Division still
        // truncates toward zero, so the endpoint can stop short of the bound.
        if (end < 0)
            ((AxParameterBlock*)object)->volumeStep = (short)(-(int)voice->currentVolume / kRampSamples);
        else if (end > kMaxVolume)
            ((AxParameterBlock*)object)->volumeStep = (short)((kMaxVolume - (int)voice->currentVolume) / kRampSamples);
        unsigned int flags = voice->flags;
        // Phase 3: publish level+delta together, suppress the delta-only flag,
        // then clear pending wrapper sync bits. Cached currentVolume is the PB
        // START level (or snapped target), not the predicted end of the block.
        flags = (flags & 0xFFFFFDFF) | 0x100;
        voice->flags = flags;
        voice->currentVolume = ((AxParameterBlock*)object)->volume;
        voice->volumeStep = ((AxParameterBlock*)object)->volumeStep;
        ((AxParameterBlock*)object)->syncFlags |= flags;
        voice->flags = 0;
    }
    OSRestoreInterrupts(lock.level);
}

// Disable or configure the five-coefficient biquad filter under the interrupt lock.
void AudioVoice_SetBiquadFilter(unsigned int* voice, unsigned int filterParameters) {
    // Zero disables the filter; only temp[0] is initialized in that path.
    // Preserve that existing full-block copy. Reconfiguring mode 2 updates only
    // its five coefficients; entering mode 2 also clears four history words.
    VoiceInterruptLevel lock;
    lock.level = OSDisableInterrupts();
    unsigned int object = *voice;
    if (object) {
        if (!filterParameters) {
            unsigned short temp[10];
            temp[0] = 0;
            VoiceInterruptLevel lock2;
            lock2.level = OSDisableInterrupts();
            object = *voice;
            if (object) {
                memcpy((char*)&((AxParameterBlock*)object)->biquad, temp, 0x14);
                voice[1] |= kSyncBiquad;
                OSRestoreInterrupts(lock2.level);
            } else
                OSRestoreInterrupts(lock2.level);
        } else if (((AxParameterBlock*)object)->biquad[0] == kFilterFiveCoefficients) {
            unsigned short a, b, c, d, e;
            AudioControl_GetFiveLookupCoefficients((void*)filterParameters, &a, &b, &c, &d, &e);
            AudioVoice_SetFilterCoefficients(voice, kFilterFiveCoefficients, a, b, c, d, e);
        } else {
            unsigned short temp[10];
            temp[0] = kFilterFiveCoefficients;
            temp[1] = 0;
            temp[2] = 0;
            temp[3] = 0;
            temp[4] = 0;
            AudioControl_GetFiveLookupCoefficients((void*)filterParameters, &temp[5], &temp[6], &temp[7], &temp[8],
                                                   &temp[9]);
            VoiceInterruptLevel lock2;
            lock2.level = OSDisableInterrupts();
            object = *voice;
            if (object) {
                memcpy((char*)&((AxParameterBlock*)object)->biquad, temp, 0x14);
                voice[1] |= kSyncBiquad;
                OSRestoreInterrupts(lock2.level);
            } else
                OSRestoreInterrupts(lock2.level);
        }
    }
    OSRestoreInterrupts(lock.level);
}

// Disable the low-pass filter above its cutoff or calculate and publish new coefficients.
void gtl_AxVoice_SetLowPassFrequency(unsigned int* voice, unsigned int frequencyHz) {
    // A 32 kHz output has a 16 kHz Nyquist limit: at/above it bypass the LPF.
    // [enabled, sample history, a0, b0] is the AX one-pole low-pass block;
    // live coefficient changes retain history, first enable clears it.
    VoiceInterruptLevel lock;
    lock.level = OSDisableInterrupts();
    unsigned int object = *voice;
    if (object) {
        if (frequencyHz >= kLowPassBypassFrequency) {
            unsigned short temp[4] = {};
            VoiceInterruptLevel lock2;
            lock2.level = OSDisableInterrupts();
            object = *voice;
            if (object) {
                memcpy((char*)&((AxParameterBlock*)object)->lowPass, temp, 8);
                voice[1] |= kSyncLowPass;
                OSRestoreInterrupts(lock2.level);
            } else
                OSRestoreInterrupts(lock2.level);
        } else if (((AxParameterBlock*)object)->lowPass[0] == 1) {
            unsigned short a, b;
            AXGetLpfCoefs(frequencyHz, &a, &b);
            VoiceInterruptLevel lock2;
            lock2.level = OSDisableInterrupts();
            object = *voice;
            if (object) {
                ((AxParameterBlock*)object)->lowPass[2] = a;
                ((AxParameterBlock*)object)->lowPass[3] = b;
                voice[1] |= kSyncLowPassCoefficients;
                OSRestoreInterrupts(lock2.level);
            } else
                OSRestoreInterrupts(lock2.level);
        } else {
            unsigned short temp[4];
            temp[0] = 1;
            temp[1] = 0;
            temp[2] = 0;
            temp[3] = 0;
            AXGetLpfCoefs(frequencyHz, &temp[2], &temp[3]);
            VoiceInterruptLevel lock2;
            lock2.level = OSDisableInterrupts();
            object = *voice;
            if (object) {
                memcpy((char*)&((AxParameterBlock*)object)->lowPass, temp, 8);
                voice[1] |= kSyncLowPass;
                OSRestoreInterrupts(lock2.level);
            } else
                OSRestoreInterrupts(lock2.level);
        }
    }
    OSRestoreInterrupts(lock.level);
}

// Publish three borrowed ADPCM-loop words [predictor/scale, history yn1, yn2].
// They are decoder bit patterns, not mix levels. Wrapper voice[5] nonzero selects
// an all-zero decoder context instead of the supplied words; its producer/meaning
// is unresolved, so this branch does not establish a mute contract. loopContext
// must contain three readable u16s when voice[5]==0 and a PB exists; otherwise
// it is not read. Copy completes before return; no buffer retention.
void AudioVoice_SetAdpcmLoopContext(unsigned int* voice, unsigned short* loopContext) {
    unsigned int level = OSDisableInterrupts();
    if (!*voice) {
        OSRestoreInterrupts(level);
    } else {
        unsigned short temp[3];
        if (!voice[5]) {
            temp[0] = loopContext[0];
            temp[1] = loopContext[1];
            temp[2] = loopContext[2];
        } else {
            temp[0] = 0;
            temp[1] = 0;
            temp[2] = 0;
        }
        unsigned int level2 = OSDisableInterrupts();
        unsigned int object = *voice;
        if (object) {
            memcpy((char*)&((AxParameterBlock*)object)->adpcmLoopContext, temp, 6);
            voice[1] |= kSyncAdpcmLoop;
        }
        OSRestoreInterrupts(level2);
        OSRestoreInterrupts(level);
    }
}

// Initialize this AX voice unit's cached full-turn angle.
void AudioVoice_InitializeUnitConstants() {
    s_twoPi_AudioVoice = kFloat_6_2831855;
}

struct VoiceEnvelopeInitState {
    unsigned int object;
    unsigned int flags;
    unsigned short currentVolume;
    unsigned short volumeStep;
    unsigned char resetVolume;
    unsigned char reserved0d;
    unsigned short targetVolume;
};

// Bind the AX parameter block and initialize volume words, delta and the reset flag.
void AudioVoice_InitializeVolumeEnvelopeState(VoiceEnvelopeInitState* voice, unsigned int parameterBlock) {
    voice->object = parameterBlock;
    voice->currentVolume = 0x18000;
    voice->flags = 0;
    voice->resetVolume = 1;
    voice->targetVolume = 0x18000;
    voice->volumeStep = 0;
}

// Expand eight mix levels to level/zero-step pairs before publishing them.
void gtl_AxVoice_SetEightMixCoefficients(void* voice, const unsigned short* coefficients) {
    unsigned short temp[16];
    unsigned short h, g, f, e, d, c, b, a;
    a = coefficients[0];
    b = coefficients[1];
    c = coefficients[2];
    d = coefficients[3];
    e = coefficients[4];
    f = coefficients[5];
    g = coefficients[6];
    h = coefficients[7];
    temp[0] = a;
    temp[1] = 0;
    temp[2] = b;
    temp[3] = 0;
    temp[4] = c;
    temp[5] = 0;
    temp[6] = d;
    temp[7] = 0;
    temp[8] = e;
    temp[9] = 0;
    temp[10] = f;
    temp[11] = 0;
    temp[12] = g;
    temp[13] = 0;
    temp[14] = h;
    temp[15] = 0;
    AudioVoice_SetEightMixPairs(voice, temp);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626C0", _ctors_u80407BD4_0, AudioVoice_InitializeUnitConstants);
#pragma pop
