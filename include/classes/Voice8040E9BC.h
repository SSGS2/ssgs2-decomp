// Voice view owned by gtl_voice_and_stream_tasks.cpp; existing fields retain their names.
// GtlVoice_InitializePlaybackParameters (0x8040F444) stores column/row counts at 0x94/0x98; gtl_nw4r_detail_Voice_ApplyPendingParameters uses the two float parameters.
#ifndef CLASSES_VOICE8040E9BC_H
#define CLASSES_VOICE8040E9BC_H

// Four channel parameter blocks, advanced by 0x1C in gtl_nw4r_detail_Voice_ApplyPendingParameters and
// gtl_nw4r_detail_Voice_ApplyVolume (0x8041086C). Unknown controls keep offset names.
struct VoiceChannelParameters {
    float volumeScale; // +0x00: ApplyVolume multiplies master and voice gain by this
    float channelPitchScale; // gtl_nw4r_detail_Voice_ApplyPendingParameters: clamped to [0,1], multiplied by Voice::pitchScale
    float panOffset;
    float surroundPanOffset;
    float auxASendOffset;
    float lowPassFilterOffset; // gtl_nw4r_detail_Voice_ApplyPendingParameters: added to Voice::lowPassFilter before AudioVolume_NormalizedToExponentialGain32000
    int unk_18;
};
#define OFF(f, o) typedef char VoiceChannelParameters_off_##f[((int)&((VoiceChannelParameters*)0)->f == (o)) ? 1 : -1]
OFF(volumeScale, 0x00);
OFF(channelPitchScale, 0x04);
OFF(panOffset, 0x08);
OFF(surroundPanOffset, 0x0C);
OFF(auxASendOffset, 0x10);
OFF(lowPassFilterOffset, 0x14);
OFF(unk_18, 0x18);
#undef OFF
typedef char VoiceChannelParameters_size[(sizeof(VoiceChannelParameters) == 0x1C) ? 1 : -1];

struct Voice {
    void* vtable;
    int hardwareVoices[8];
    union {
        char channelParameterStorage[0x70];
        VoiceChannelParameters channels[4]; // 0x24: GtlVoice_ConfigureSamplePlayback initializes rowCount 0x1C-byte parameter blocks
    };
    int sourceChannelCount;
    int outputVoiceCount;
    int eventCallback;
    union {
        char callbackArgumentStorage[4];
        int callbackArgument; // 0xA0: GtlVoice_InitializePlaybackParameters saves d; GtlVoice_NotifyOutputBufferReadStarted passes this word to the callback
    };
    unsigned char allocated, playbackRequested, hardwareStarted, pauseRequested, hardwarePaused, pendingChanges;
    char biquadFilterState[0x6];
    float pan, surroundPan, panOffset, surroundPanOffset; // GtlVoice_InitializePlaybackParameters (0x8040F444): initialized float parameters
    float lowPassFilter;
    union {
        char pad_C4[4];
        int outputLineMask; // GtlVoice_InitializePlaybackParameters initializes to one
    };
    float mainVolume, mainSend, auxASend, auxBSend, auxCSend;
    float remoteVolume0, remoteVolume1, remoteVolume2, remoteVolume3;
    float remoteSend0, remoteSend1, remoteSend2, remoteSend3;
    float additionalOutputMix0, additionalOutputMix1, additionalOutputMix2, unk_108;
    float pitchScale;
    union {
        char pad_110[4];
        float volumeScale; // 0x110: gtl_nw4r_detail_Voice_ApplyVolume multiplies this by the AudioController gain
    };
    float initialVolume, targetVolume;
    int stereoPanMode, panGainMode; // GtlVoice_InitializePlaybackParameters initializes both to one
    int poolLinkNext;
    int poolLinkPrevious;
};

#define OFF(f, o) typedef char Voice_off_##f[((int)&((Voice*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(hardwareVoices, 0x4);
OFF(channelParameterStorage, 0x24);
OFF(channels, 0x24);
OFF(sourceChannelCount, 0x94);
OFF(outputVoiceCount, 0x98);
OFF(eventCallback, 0x9c);
OFF(callbackArgumentStorage, 0xa0);
OFF(callbackArgument, 0xa0);
OFF(allocated, 0xa4);
OFF(playbackRequested, 0xa5);
OFF(hardwareStarted, 0xa6);
OFF(pauseRequested, 0xa7);
OFF(hardwarePaused, 0xa8);
OFF(pendingChanges, 0xa9);
OFF(biquadFilterState, 0xaa);
OFF(pan, 0xb0);
OFF(surroundPan, 0xb4);
OFF(panOffset, 0xb8);
OFF(surroundPanOffset, 0xbc);
OFF(lowPassFilter, 0xc0);
OFF(pad_C4, 0xc4);
OFF(outputLineMask, 0xc4);
OFF(mainVolume, 0xc8);
OFF(mainSend, 0xcc);
OFF(auxASend, 0xd0);
OFF(auxBSend, 0xd4);
OFF(auxCSend, 0xd8);
OFF(remoteVolume0, 0xdc);
OFF(remoteVolume1, 0xe0);
OFF(remoteVolume2, 0xe4);
OFF(remoteVolume3, 0xe8);
OFF(remoteSend0, 0xec);
OFF(remoteSend1, 0xf0);
OFF(remoteSend2, 0xf4);
OFF(remoteSend3, 0xf8);
OFF(additionalOutputMix0, 0xfc);
OFF(additionalOutputMix1, 0x100);
OFF(additionalOutputMix2, 0x104);
OFF(unk_108, 0x108);
OFF(pitchScale, 0x10c);
OFF(pad_110, 0x110);
OFF(volumeScale, 0x110);
OFF(initialVolume, 0x114);
OFF(targetVolume, 0x118);
OFF(stereoPanMode, 0x11c);
OFF(panGainMode, 0x120);
OFF(poolLinkNext, 0x124);
OFF(poolLinkPrevious, 0x128);
#undef OFF
typedef char Voice_size[(sizeof(Voice) == 0x12C) ? 1 : -1];

#endif
