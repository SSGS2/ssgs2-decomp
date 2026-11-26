// Layout view for the per-channel audio state constructed by gtl_RemoteAudioChannel_InitState.
// Methods and singleton storage are in gtl_remote_audio_channel.cpp; no vtable or RTTI is introduced.
#ifndef CLASSES_GTLREMOTEAUDIOSTATE_H
#define CLASSES_GTLREMOTEAUDIOSTATE_H
#include "sdk_protos.h"

struct GtlRemoteAudioState {
    unsigned char
        enabled; // 0x0: gtl_RemoteAudioChannel_RequestEnable enables; gtl_RemoteAudioChannel_RequestDisable disables.
    unsigned char sendingAudio; // 0x1: gtl_RemoteAudioChannel_SendSamples stores whether this frame sent audio.
    unsigned char checkSilence; // 0x2: gtl_RemoteAudioChannel_SendSamples scans samples for nonzero words when set.
    unsigned char
        firstPacket; // 0x3: gtl_RemoteAudioChannel_OnCommandCompletePhase2 sets; gtl_RemoteAudioChannel_SendSamples consumes and passes its inverse to the encoder.
    unsigned char callbackPending; // 0x4: gtl_RemoteAudioChannel_DispatchCommand sets before async commands.
    unsigned char commandBusy;     // 0x5: gtl_RemoteAudioChannel_DispatchCommand gate; completion callbacks clear.
    unsigned char
        sendTimedOut; // 0x6: gtl_RemoteAudioChannel_OnSendTimeout sets; gtl_RemoteAudioChannel_SendSamples stops sending.
    unsigned char
        sendAlarmArmed; // 0x7: gtl_RemoteAudioChannel_SendSamples schedules; gtl_RemoteAudioChannel_OnSendTimeout clears.
    unsigned char
        stopAlarmArmed; // 0x8: gtl_RemoteAudioChannel_SendSamples sets; gtl_RemoteAudioChannel_OnStopAlarm consumes.
    char pad_9[3];
    int phase;           // 0xC: completion transitions 2/4/6; audio requires phase 4.
    int requestedAction; // 0x10: gtl_RemoteAudioChannel_RequestEnable/B404 queue actions 1/3.
    int retryAction;     // 0x14: callbacks queue another action on status -2.
    char encoderState
        [0x20]; // 0x18: cleared by gtl_RemoteAudioChannel_OnCommandCompletePhase2; WENCGetEncodeData converts 40 samples to 20 bytes.
    int channel;                // 0x38: singleton initializes 0..3; passed to async command/send calls.
    void (*callback)(int, int); // 0x3C: called with channel and completion status.
    OSAlarm sendAlarm;          // 0x40: gtl_RemoteAudioChannel_OnSendTimeout timeout.
    OSAlarm stopAlarm;          // 0x70: gtl_RemoteAudioChannel_OnStopAlarm delayed stop.
    OSTime sendStartTime;       // 0xA0: gtl_RemoteAudioChannel_SendSamples stores OSGetTime when scheduling sendAlarm.
};
#define OFF(f, o) typedef char GtlRemoteAudioState_off_##f[((int)&((GtlRemoteAudioState*)0)->f == (o)) ? 1 : -1]
OFF(enabled, 0x0);
OFF(sendingAudio, 0x1);
OFF(checkSilence, 0x2);
OFF(firstPacket, 0x3);
OFF(callbackPending, 0x4);
OFF(commandBusy, 0x5);
OFF(sendTimedOut, 0x6);
OFF(sendAlarmArmed, 0x7);
OFF(stopAlarmArmed, 0x8);
OFF(phase, 0xC);
OFF(requestedAction, 0x10);
OFF(retryAction, 0x14);
OFF(encoderState, 0x18);
OFF(channel, 0x38);
OFF(callback, 0x3C);
OFF(sendAlarm, 0x40);
OFF(stopAlarm, 0x70);
OFF(sendStartTime, 0xA0);
#undef OFF
typedef char GtlRemoteAudioState_size[(sizeof(GtlRemoteAudioState) == 0xA8) ? 1 : -1];
#endif
