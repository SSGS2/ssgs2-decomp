// Voice control for the audio system: setters for a voice's pitch, pan, volume, filters and output
// sends, plus start, stop, and sample address control for the hardware voices in a voice group.
// Most setters store the value and mark output mixing for recomputation.

#include "classes/Voice8040E9BC.h" // Voice channel parameters at +0x24, stride 0x1C

#include "sdk_protos.h"

struct dtor_8040F368 {
    BOOL level;
    // Provide the empty saved-interrupt-level base destructor.
    ~dtor_8040F368() {}
};

struct InterruptGuard_dtor : dtor_8040F368 {
    // Disable interrupts and remember the previous enable state.
    InterruptGuard_dtor() {
        level = OSDisableInterrupts();
    }
    // Restore the saved interrupt state on every scope exit.
    ~InterruptGuard_dtor() {
        OSRestoreInterrupts(level);
    }
};

extern "C" {

// Hardware voice wrapper view: +0x14 is the sample-format word set by
// AudioVoice_SetSampleSourceState, despite the legacy field name priority.
// AX priority is changed through AudioVoice_SetPriorityIfAllocated, a different path.
struct Inner {
    char prefix[0x14];
    int sampleFormat;
};

// Shared flags at +0xA4..+0xA9 mean allocated, playback requested, hardware started,
// pause requested, hardware paused, and pending changes.
// values is a two-dimensional hardware-voice grid at +4: [sampleChannel][outputVoice].
// sourceChannelCount is the outer sample-channel count (16-byte stride);
// outputVoiceCount is the inner count (4-byte stride, up to four per channel).
// The inner axis also selects channels[] controls, at +0x24 with a 0x1C-byte stride.
// A null entry is unallocated. Row setters require 0 <= rowIndex < sourceChannelCount;
// send index bounds are aux/outputMix 0..2 and remote 0..3; no bounds checks are added.
typedef Voice Object;
// Small views for controls still inside padding in the shared Voice header.
struct VoiceFilterControls {
    char prefix[0xAA];
    unsigned char biquadFilter;
    char reserved;
    int priority;
};
struct VoiceAdditionalMixControls {
    char prefix[0xD0];
    float auxiliarySend[3];
    float remoteVolume[4];
    float remoteSend[4];
    float outputMix[3];
};
typedef char VoiceOutputMixOffset[(int)&((VoiceAdditionalMixControls*)0)->outputMix == 0xFC ? 1 : -1];
// Grid cursor contract used below:
//   row = self + sourceChannel*16 (points four bytes BEFORE that row's slots);
//   entry[1] = *(self + 4 + sourceChannel*16 + outputVoice*4).
// The +1 skips the vtable-sized prefix. Such row/column cursors are address
// projections, not separate Voice objects; never dereference unrelated members.
// Borrow the first hardware voice pointer from this channel-grid row.
inline Inner* FirstHardwareVoice(Object* voice) {
    return (Inner*)voice->hardwareVoices[0];
}
enum VoiceDirtyFlags {
    kVoiceStartPending = 0x01,
    kVoicePausePending = 0x02,
    kVoicePitchDirty = 0x04,
    kVoiceVolumeDirty = 0x08,
    kVoiceOutputMixDirty = 0x10,
    kVoiceLowPassDirty = 0x20,
    kVoiceBiquadDirty = 0x80,
    kVoiceInitialMixDirty = kVoiceVolumeDirty | kVoiceOutputMixDirty | kVoiceLowPassDirty
};
enum VoiceLimits {
    kBiquadFilterMaximum = 127,
    kNormalVoicePriority = 16,
    kVoiceRowBytes = 16,
    kChannelParameterBytes = 28,
    kSampleParameterStride = 0x44,
    kAutomaticResamplingMode = 5,
    kAxVoiceStopped = 0,
    kAxVoiceRunning = 1
};

struct Active {
    char prefix[0x38];
    unsigned short state;
};
// Twenty DSP halfwords: 16 ADPCM coefficients followed by gain/current decoder state.
// Slots 17..19 are refreshed for sampleOffset before the hardware copy. A separate
// three-halfword loop context follows at +0x4C from the per-source channel base.
struct Params40 {
    // v[0..15] coefficients; v[16] gain; v[17] predictor/scale;
    // v[18]/v[19] previous decoded samples (yn1/yn2), all raw DSP halfwords.
    unsigned short v[20];
    // Leave DSP halfwords uninitialized until the explicit copy below.
    Params40() {}
    // Copy all twenty ADPCM halfwords in their original DSP order.
    void set(unsigned short a0, unsigned short a1, unsigned short a2, unsigned short a3, unsigned short a4,
             unsigned short a5, unsigned short a6, unsigned short a7, unsigned short a8, unsigned short a9,
             unsigned short a10, unsigned short a11, unsigned short a12, unsigned short a13, unsigned short a14,
             unsigned short a15, unsigned short a16, unsigned short a17, unsigned short a18, unsigned short a19) {
        v[0] = a0;
        v[1] = a1;
        v[2] = a2;
        v[3] = a3;
        v[4] = a4;
        v[5] = a5;
        v[6] = a6;
        v[7] = a7;
        v[8] = a8;
        v[9] = a9;
        v[10] = a10;
        v[11] = a11;
        v[12] = a12;
        v[13] = a13;
        v[14] = a14;
        v[15] = a15;
        v[16] = a16;
        v[17] = a17;
        v[18] = a18;
        v[19] = a19;
    }
};
// Header offsets are part of the stream-player ABI, not arbitrary mix controls.
struct Params {
    unsigned char sampleFormat; // +0: sample format; zero selects ADPCM setup
    unsigned char loopEnabled;  // +1: loop enable (zero/nonzero)
    char pad02[2];
    int sourceRate;            // +4: source sample rate, forwarded unchanged
    int loopStartSampleOffset; // +8: loop-start sample offset
    int endSampleOffset;       // +C: end sample offset
    char rest[1];
};
// Read-only prefix view: channel records advance by 0x44 bytes in the source blob.
struct SampleParameterBlock {
    char prefix[0x24];
    Params40 adpcm;
};

int GtlVoice_GetSamplePosition(Inner*);
void gtl_Voice_SetLoopSampleAddress(Inner*, int, int);
void GtlVoice_SetEndSampleAddress(Inner*, int, int);
void AudioVoice_SetAdpcmLoopContext(Inner*, int);
void GtlVoice_SetLoopFlag(Inner*, int);
void GtlVoice_SetNonLoopingSampleRange(Inner*, int, int);
void AudioVoice_SetTypeIfAllocated(Inner*, int);
void AudioVoice_SetPriorityIfAllocated(Inner*, int);
void* GtlVoicePool_GetInstance();
void gtl_AxVoicePool_ReleaseVoice(void*, Inner*);
void* gtl_VoicePool_GetInstance();
void gtl_VoicePool_ReturnVoice(void*, Object*);
extern const float kFloat_1_0_AudioVoice;
extern const float kFloat_0_AudioVoice;
void AXSetVoiceState(Active*, int);
void* gtl_AudioController_GetInstance();
float gtl_AudioController_GetMasterGain(void*);
void gtl_AxVoice_SetVolumeEnvelope(Inner*, float, float);
void AudioVoice_ReadParameterTailTriplet(unsigned short*, unsigned short*, unsigned short*, int, int, Params40*);
void AudioVoice_SetSampleSourceState(Inner*, int, int, int);
void gtl_Voice_SetAddressRange(Inner*, bool, int, int, int, int);
void GtlVoice_SetAdpcmParameters(Inner*, Params40*);
void GtlVoice_SelectResamplingModeForPitch(Inner*, float, int);

// Return the first allocated hardware voice sample position, or zero.
int GtlVoiceGroup_GetSamplePosition(Object* self) {
    Inner* inner = FirstHardwareVoice(self);
    if (inner)
        return GtlVoice_GetSamplePosition(inner);
    return 0;
}

// Return the first hardware voice sample-format word, or 10 if no voice is allocated.
// Legacy symbol/field names say priority; streaming callers test zero for ADPCM.
int GtlVoiceGroup_GetFirstSampleFormat(Object* self) {
    Inner* inner = FirstHardwareVoice(self);
    if (inner)
        return inner->sampleFormat;
    return 10;
}

// Enable playback, clear requested pause, and queue a hardware start.
void GtlVoice_RequestStart(Object* self) {
    self->playbackRequested = 1;
    self->pauseRequested = 0;
    self->pendingChanges |= kVoiceStartPending;
}

} // extern "C"

// Queue a pause update when the requested pause flag changes.
void gtl::nw4r::detail::Voice::SetPauseFlag(unsigned int value) {
    if (pauseRequested != value) {
        pauseRequested = value;
        pendingChanges |= kVoicePausePending;
    }
}

extern "C" {

// Store pitch scaling and defer the hardware pitch update.
void GtlVoice_SetPitchScale(Object* self, float value) {
    if (value != self->pitchScale) {
        self->pitchScale = value;
        self->pendingChanges |= kVoicePitchDirty;
    }
}

// Store stereo pan and mark output mixing for recomputation.
void GtlVoice_SetPan(Object* self, float value) {
    if (value != self->pan) {
        self->pan = value;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Store surround pan and mark output mixing for recomputation.
void GtlVoice_SetSurroundPan(Object* self, float value) {
    if (value != self->surroundPan) {
        self->surroundPan = value;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Store the additional stereo pan offset and mark output mixing dirty.
void GtlVoice_SetPanOffset(Object* self, float value) {
    if (value != self->panOffset) {
        self->panOffset = value;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Store the additional surround pan offset and mark output mixing dirty.
void GtlVoice_SetSurroundPanOffset(Object* self, float value) {
    if (value != self->surroundPanOffset) {
        self->surroundPanOffset = value;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Select output destinations and mark output mixing dirty.
void GtlVoice_SetOutputLineMask(Object* self, int value) {
    if (value != self->outputLineMask) {
        self->outputLineMask = value;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Clamp the biquad filter selector to 0..127 and queue its hardware update.
void GtlVoice_SetBiquadFilterSelector(Object* self, int value) {
    if (value > kBiquadFilterMaximum)
        value = kBiquadFilterMaximum;
    else
        value = value < 0 ? 0 : value;
    if (value != ((VoiceFilterControls*)self)->biquadFilter) {
        ((VoiceFilterControls*)self)->biquadFilter = value;
        self->pendingChanges |= kVoiceBiquadDirty;
    }
}

// Clamp voice gain to [0,1] and queue the envelope update.
void gtl_nw4r_detail_Voice_SetVolumeScale(Object* self, float value) {
    float clamped;
    if (value > kFloat_1_0_AudioVoice)
        clamped = kFloat_1_0_AudioVoice;
    else if (value < kFloat_0_AudioVoice)
        clamped = kFloat_0_AudioVoice;
    else
        clamped = value;
    if (clamped != self->volumeScale) {
        self->volumeScale = clamped;
        self->pendingChanges |= kVoiceVolumeDirty;
    }
}

// Clamp the low-pass control to [0,1] and queue its hardware update.
void GtlVoice_SetLowPassFilter(Object* self, float value) {
    float clamped;
    if (value > kFloat_1_0_AudioVoice)
        clamped = kFloat_1_0_AudioVoice;
    else if (value < kFloat_0_AudioVoice)
        clamped = kFloat_0_AudioVoice;
    else
        clamped = value;
    if (clamped != self->lowPassFilter) {
        self->lowPassFilter = clamped;
        self->pendingChanges |= kVoiceLowPassDirty;
    }
}

// Clamp main-output gain to [0,1] and mark output mixing dirty.
void GtlVoice_SetMainOutputVolume(Object* self, float value) {
    float clamped;
    if (value > kFloat_1_0_AudioVoice)
        clamped = kFloat_1_0_AudioVoice;
    else if (value < kFloat_0_AudioVoice)
        clamped = kFloat_0_AudioVoice;
    else
        clamped = value;
    if (clamped != self->mainVolume) {
        self->mainVolume = clamped;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Translate the main-send offset by one, clamp it, and mark mixing dirty.
void GtlVoice_SetMainSendOffset(Object* self, float value) {
    value += kFloat_1_0_AudioVoice;
    float clamped;
    if (value > kFloat_1_0_AudioVoice)
        clamped = kFloat_1_0_AudioVoice;
    else if (value < kFloat_0_AudioVoice)
        clamped = kFloat_0_AudioVoice;
    else
        clamped = value;
    if (clamped != self->mainSend) {
        self->mainSend = clamped;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Clamp one auxiliary bus send to [0,1] and mark output mixing dirty.
void GtlVoice_SetAuxSend(Object* self, int index, float value) {
    float clamped;
    if (value > kFloat_1_0_AudioVoice)
        clamped = kFloat_1_0_AudioVoice;
    else if (value < kFloat_0_AudioVoice)
        clamped = kFloat_0_AudioVoice;
    else
        clamped = value;
    if (clamped != ((VoiceAdditionalMixControls*)self)->auxiliarySend[index]) {
        ((VoiceAdditionalMixControls*)self)->auxiliarySend[index] = clamped;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Clamp one remote output gain to [0,1] and mark output mixing dirty.
void GtlVoice_SetRemoteVolume(Object* self, int index, float value) {
    float clamped;
    if (value > kFloat_1_0_AudioVoice)
        clamped = kFloat_1_0_AudioVoice;
    else if (value < kFloat_0_AudioVoice)
        clamped = kFloat_0_AudioVoice;
    else
        clamped = value;
    if (clamped != ((VoiceAdditionalMixControls*)self)->remoteVolume[index]) {
        ((VoiceAdditionalMixControls*)self)->remoteVolume[index] = clamped;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Clamp one of three additional output-mix controls and mark mixing dirty.
void GtlVoice_SetAdditionalOutputMixControl(Object* self, int index, float value) {
    float clamped;
    if (value > kFloat_1_0_AudioVoice)
        clamped = kFloat_1_0_AudioVoice;
    else if (value < kFloat_0_AudioVoice)
        clamped = kFloat_0_AudioVoice;
    else
        clamped = value;
    if (clamped != ((VoiceAdditionalMixControls*)self)->outputMix[index]) {
        ((VoiceAdditionalMixControls*)self)->outputMix[index] = clamped;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Translate a remote-send offset by one, clamp it, and mark mixing dirty.
void GtlVoice_SetRemoteSendOffset(Object* self, int index, float value) {
    value += kFloat_1_0_AudioVoice;
    float clamped;
    if (value > kFloat_1_0_AudioVoice)
        clamped = kFloat_1_0_AudioVoice;
    else if (value < kFloat_0_AudioVoice)
        clamped = kFloat_0_AudioVoice;
    else
        clamped = value;
    if (clamped != ((VoiceAdditionalMixControls*)self)->remoteSend[index]) {
        ((VoiceAdditionalMixControls*)self)->remoteSend[index] = clamped;
        self->pendingChanges |= kVoiceOutputMixDirty;
    }
}

// Copy a borrowed three-u16 decoder loop context to every allocated voice in a sample channel.
// loopContextAddress is a pointer encoded as int; the callee copies six bytes synchronously.
// The lower-level legacy name says auxiliary mix; streaming callers supply ADPCM loop state.
void GtlVoice_SetRowStateTriplet(Object* self, int rowIndex, int loopContextAddress) {
    Inner** entry = (Inner**)((char*)self + rowIndex * kVoiceRowBytes);
    for (int i = 0; i < self->outputVoiceCount; ++entry, ++i) {
        Inner* inner = entry[1];
        if (inner)
            AudioVoice_SetAdpcmLoopContext(inner, loopContextAddress);
    }
}

#pragma push
#pragma use_lmw_stmw on
// Under interrupt protection, return allocated hardware voices and release the group.
void GtlVoice_Release(Object* self) {
    InterruptGuard_dtor guard;
    if (!self->allocated)
        return;
    char* row = (char*)self;
    Inner** entry;
    for (int i = 0; i < self->sourceChannelCount; row += kVoiceRowBytes, ++i) {
        entry = (Inner**)row;
        for (int j = 0; j < self->outputVoiceCount; ++entry, ++j) {
            Inner* inner = entry[1];
            if (inner) {
                gtl_AxVoicePool_ReleaseVoice(GtlVoicePool_GetInstance(), inner);
                entry[1] = 0;
            }
        }
    }
    self->sourceChannelCount = 0;
    gtl_VoicePool_ReturnVoice(gtl_VoicePool_GetInstance(), self);
    self->allocated = 0;
}

// Set allocated hardware priorities to 16 unless the software voice priority is one.
// The legacy priorityMode field aliases VoicePriority::priority at +0xAC.
void GtlVoice_SetAllocatedPrioritiesTo16(Object* self) {
    if (((VoiceFilterControls*)self)->priority == 1)
        return;
    char* row = (char*)self;
    Inner** entry;
    for (int i = 0; i < self->sourceChannelCount; row += kVoiceRowBytes, ++i) {
        entry = (Inner**)row;
        for (int j = 0; j < self->outputVoiceCount; ++entry, ++j) {
            Inner* inner = entry[1];
            if (inner)
                AudioVoice_SetPriorityIfAllocated(inner, kNormalVoicePriority);
        }
    }
}

// Apply the hardware voice type selector to every allocated voice.
void GtlVoiceGroup_SetHardwareVoiceType(Object* self, int voiceType) {
    char* row;
    Inner** entry;
    int i = 0;
    row = (char*)self;
    for (; i < self->sourceChannelCount; row += kVoiceRowBytes, ++i) {
        entry = (Inner**)row;
        for (int j = 0; j < self->outputVoiceCount; ++entry, ++j) {
            Inner* inner = entry[1];
            if (inner)
                AudioVoice_SetTypeIfAllocated(inner, voiceType);
        }
    }
}

} // extern "C"

// Set the loop address for allocated voices in the selected row.
void gtl::nw4r::detail::Voice::SetChannelLoopSampleAddress(int rowIndex, int sampleBase, int loopAddress) {
    Inner** entry = (Inner**)((char*)this + rowIndex * kVoiceRowBytes);
    for (int i = 0; i < outputVoiceCount; ++entry, ++i) {
        Inner* inner = entry[1];
        if (inner)
            gtl_Voice_SetLoopSampleAddress(inner, sampleBase, loopAddress);
    }
}

extern "C" {

// Set the end address for allocated voices in the selected row.
void GtlVoiceGroup_SetRowEndSampleAddress(Object* self, int rowIndex, int sampleBase, int endAddress) {
    Inner** entry = (Inner**)((char*)self + rowIndex * kVoiceRowBytes);
    for (int i = 0; i < self->outputVoiceCount; ++entry, ++i) {
        Inner* inner = entry[1];
        if (inner)
            GtlVoice_SetEndSampleAddress(inner, sampleBase, endAddress);
    }
}

// Set loop enablement on every allocated hardware voice.
void GtlVoiceGroup_SetLoopFlag(Object* self, int enabled) {
    char* row = (char*)self;
    Inner** entry;
    for (int i = 0; i < self->sourceChannelCount; row += kVoiceRowBytes, ++i) {
        entry = (Inner**)row;
        for (int j = 0; j < self->outputVoiceCount; ++entry, ++j) {
            Inner* inner = entry[1];
            if (inner)
                GtlVoice_SetLoopFlag(inner, enabled);
        }
    }
}

// Set a non-looping sample range for the selected row.
void GtlVoiceGroup_SetRowNonLoopingSampleRange(Object* self, int rowIndex, int sampleBase, int endAddress) {
    Inner** entry = (Inner**)((char*)self + rowIndex * kVoiceRowBytes);
    for (int i = 0; i < self->outputVoiceCount; ++entry, ++i) {
        Inner* inner = entry[1];
        if (inner)
            GtlVoice_SetNonLoopingSampleRange(inner, sampleBase, endAddress);
    }
}

#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Stop running hardware voices and clear active/requested playback and pause flags.
void GtlVoice_Stop(Object* self) {
    int j;
    int i;
    char* row;
    Inner** entry;
    if (self->hardwareStarted) {
        row = (char*)self;
        for (i = 0; i < self->sourceChannelCount; row += kVoiceRowBytes, ++i) {
            entry = (Inner**)row;
            for (j = 0; j < self->outputVoiceCount; ++entry, ++j) {
                Inner* inner = entry[1];
                if (inner) {
                    Active* active = *(Active**)inner;
                    int valid = 0;
                    if (active && active->state == kAxVoiceRunning)
                        valid = 1;
                    if (valid)
                        AXSetVoiceState(active, kAxVoiceStopped);
                }
            }
        }
        self->hardwareStarted = 0;
    }
    self->hardwarePaused = 0;
    self->pauseRequested = 0;
    self->playbackRequested = 0;
}
#pragma pop

// Combine master, voice, and channel gain to update each hardware envelope.
// Envelope target/initial = masterGain * voiceGain * per-output gain * target/initialVolume.
// These are linear gains; lower-level conversion sets hardware envelope scaling. No
// ramp duration is passed here, and the first-start envelope initialization is deferred.
void gtl_nw4r_detail_Voice_ApplyVolume(Object* self) {
    float combinedGain = kFloat_1_0_AudioVoice;
    combinedGain *= self->volumeScale;
    combinedGain *= gtl_AudioController_GetMasterGain(gtl_AudioController_GetInstance());
    char* column;
    int offset;
    Inner** entry;
    int i;
    // Walk output controls at 28 bytes and grid columns at 4 bytes in lockstep.
    // A column's channel gain is channels[i].volumeScale; entry += 4 then walks
    // source channels (16 bytes) while retaining the same output voice.
    column = (char*)self;
    i = 0;
    offset = 0;
    for (; i < self->outputVoiceCount; column += kChannelParameterBytes, offset += 4, ++i) {
        float channelGain = combinedGain * ((Voice*)column)->channels[0].volumeScale;
        entry = (Inner**)((unsigned int)self + offset);
        float targetGain = channelGain * self->targetVolume;
        float initialGain = channelGain * self->initialVolume;
        for (int j = 0; j < self->sourceChannelCount; entry += 4, ++j) {
            Inner* inner = entry[1];
            if (inner)
                gtl_AxVoice_SetVolumeEnvelope(inner, targetGain, initialGain);
        }
    }
}

// Configure sample addresses, ADPCM state and resampling, then reset channel controls.
// params is borrowed; its header contains format (+0), loop enable (+1), source rate
// (+4), loop start (+8) and end (+0xC), as populated by the stream player's StartPacket.
// Per-source records start at params+0x10 and advance 0x44 bytes; addresses/offsets
// remain in the sample-address units expected by the lower layer, not assumed byte counts.
// Source storage must outlive playback. Hardware setup is skipped when a channel
// has no first output voice, even if later output slots are non-null.
void GtlVoice_ConfigureSamplePlayback(Object* self, Params* params, int sampleOffset) {
    int sampleFormat = params->sampleFormat;
    char* row;
    char* source;
    Params40* sourceAdpcm;
    int sourceRate = params->sourceRate; // header +4, forwarded to hardware source state
    row = (char*)self;
    source = (char*)params;
    sourceAdpcm = &((SampleParameterBlock*)params)->adpcm;
    // For each source channel, copy decoder state once and apply it to all allocated outputs.
    int i = 0;
    for (; i < self->sourceChannelCount; row += kVoiceRowBytes, source += kSampleParameterStride,
                                         sourceAdpcm = (Params40*)((char*)sourceAdpcm + kSampleParameterStride), ++i) {
        if (FirstHardwareVoice((Object*)row)) {
            int sampleAddress = *(int*)(source + 0x10);
            Params40 adpcmParameters;
            if (!sampleFormat) {
                adpcmParameters.set(sourceAdpcm->v[0], sourceAdpcm->v[1], sourceAdpcm->v[2], sourceAdpcm->v[3],
                                    sourceAdpcm->v[4], sourceAdpcm->v[5], sourceAdpcm->v[6], sourceAdpcm->v[7],
                                    sourceAdpcm->v[8], sourceAdpcm->v[9], sourceAdpcm->v[10], sourceAdpcm->v[11],
                                    sourceAdpcm->v[12], sourceAdpcm->v[13], sourceAdpcm->v[14], sourceAdpcm->v[15],
                                    sourceAdpcm->v[16], sourceAdpcm->v[17], sourceAdpcm->v[18], sourceAdpcm->v[19]);
                // Refresh predictor/scale and sample history for this start offset.
                AudioVoice_ReadParameterTailTriplet(&adpcmParameters.v[17], &adpcmParameters.v[18],
                                                    &adpcmParameters.v[19], sampleOffset, sampleAddress,
                                                    &adpcmParameters);
            }
            Inner** entry = (Inner**)row;
            for (int j = 0; j < self->outputVoiceCount; ++entry, ++j) {
                Inner* inner = entry[1];
                if (inner) {
                    AudioVoice_SetSampleSourceState(inner, *(int*)(source + 0x10), sampleFormat, sourceRate);
                    gtl_Voice_SetAddressRange(inner, params->loopEnabled != 0, sampleAddress, sampleOffset,
                                              params->loopStartSampleOffset, params->endSampleOffset);
                    if (!sampleFormat) {
                        GtlVoice_SetAdpcmParameters(inner, &adpcmParameters);
                        // +0x28 follows the 20-halfword state: loop predictor/scale,
                        // yn1 and yn2. The legacy callee name is misleading.
                        AudioVoice_SetAdpcmLoopContext(inner, (int)sourceAdpcm + 0x28);
                    }
                    GtlVoice_SelectResamplingModeForPitch(inner, self->pitchScale, kAutomaticResamplingMode);
                    AudioVoice_SetTypeIfAllocated(inner, 0);
                }
            }
        }
    }
    // Reset per-output gain/pan/filter controls, clear playback requests and queue initial mixing.
    // This is channels[j] reached through a biased base; stride is 0x1C,
    // not sizeof(Voice). Only channels[0] is valid through this projection.
    char* column = (char*)self;
    for (int j = 0; j < self->outputVoiceCount; ++j, column += kChannelParameterBytes) {
        ((Voice*)column)->channels[0].volumeScale = kFloat_1_0_AudioVoice;
        ((Voice*)column)->channels[0].channelPitchScale = kFloat_1_0_AudioVoice;
        ((Voice*)column)->channels[0].panOffset = kFloat_0_AudioVoice;
        ((Voice*)column)->channels[0].surroundPanOffset = kFloat_0_AudioVoice;
        ((Voice*)column)->channels[0].auxASendOffset = kFloat_0_AudioVoice;
        ((Voice*)column)->channels[0].lowPassFilterOffset = kFloat_0_AudioVoice;
        ((Voice*)column)->channels[0].unk_18 = 0; // Reset the retained channel control word.
    }
    self->pauseRequested = 0;
    self->hardwarePaused = 0;
    self->playbackRequested = 0;
    self->hardwareStarted = 0;
    self->pendingChanges |= kVoiceInitialMixDirty;
}
} // extern "C"
