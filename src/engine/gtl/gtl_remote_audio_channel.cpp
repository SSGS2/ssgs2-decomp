// Wii Remote audio: the four remote speaker channels, their sample streaming and timeouts,
// and the audio runtime startup and shutdown.
//
// Each speaker is enabled or disabled through command completion callbacks, with retries
// when the remote is busy.

#include <decomp/static_init.h>
#include "sdk_protos.h"
#include "classes/GtlRemoteAudioState.h"
// Channel layout: gtl_RemoteAudioChannel_InitState (0x8040B280), audio pump gtl_RemoteAudioChannel_SendSamples.
#define AsRemoteAudio(p) ((GtlRemoteAudioState*)(p))
#define REMOTE_FIELD_OFFSET(field) ((int)&((GtlRemoteAudioState*)0)->field)

struct RemoteAudioSingletonView {
    unsigned char timerRunning;
    char padding[7];
    OSAlarm timer;
    GtlRemoteAudioState channels[4];
};
typedef char RemoteAudioSingletonView_size[(sizeof(RemoteAudioSingletonView) == 0x2D8) ? 1 : -1];
enum {
    kRemoteChannelsOffset = (int)&((RemoteAudioSingletonView*)0)->channels,
    kRemoteChannelBytes = sizeof(GtlRemoteAudioState),
    kRemoteSingletonBytes = sizeof(RemoteAudioSingletonView)
};
enum RemoteAudioConstants {
    kRemoteChannelCount = 4,
    kPcmSamplesPerPacket = 40,
    kPcmWordsPerPacket = 20,
    kAdpcmBytesPerPacket = 20,
    kWpadSuccess = 0,
    kWpadDisconnected = -1,
    kWpadBusy = -2,
    kWpadTransferError = -3,
    kWpadReady = 3,
    kEnableRetryMilliseconds = 5,
    kSendTimeoutSeconds = 480
};
enum RemoteAudioPhase {
    kPhaseIdle = 0,
    kPhaseEnabling = 1,
    kPhaseEnabled = 2,
    kPhaseStartingStream = 3,
    kPhaseStreaming = 4,
    kPhaseDisabling = 5,
    kPhaseDisabled = 6
};
enum RemoteAudioAction {
    kActionNone = 0,
    kActionEnable = 1,
    kActionStartStream = 2,
    kActionDisable = 3
};
enum SpeakerCommand {
    kSpeakerDisable = 0,
    kSpeakerEnable = 1,
    kSpeakerStartStream = 4
};

namespace gtl {
namespace nw4r {
struct FxReverbStd {
    char reverbParameters[0xC];
    unsigned char active;
};
}
}

extern "C" {

void __dl__FPv(void*);
void OSCancelAlarm(void*);
void* fn_8033E838(void*); // OS alarm user data; the retry alarm stores an index, channel alarms store a pointer.
char* gtl_RemoteAudio_GetInstance();
void* gtl_RemoteAudio_GetChannel(char*, int);
int gtl_RemoteAudioChannel_RequestEnable(void*, void (*)(int, int));
void gtl_RemoteAudioChannel_RequestDisable(void*, int);
extern unsigned char s_audioRuntimeInitialized;
void* gtl_AudioWorker_GetInstance();
void gtl_AudioWorker_Stop(void*);
void* GtlStreamTaskManager_GetInstance();
void gtl_AudioWorkQueue_CancelAll(void*);
void* AudioMessageWorker_GetInstance();
void AudioRuntime_StopWorkerThread(void*);
void* gtl_VoicePool_GetInstance();
void gtl_VoicePool_Shutdown(void*);
void* GtlVoicePool_GetInstance();
void AudioVoicePool_Shutdown(void*);
struct PollState {
    char pad[0x50];
    unsigned int pending;
};
PollState* gtl_AudioController_GetInstance();
void gtl_AudioController_Shutdown(void*);
extern OSAlarm s_remoteAudioEnableRetryAlarms[4];
void gtl_RemoteAudio_RetryEnableAlarm(OSAlarm*, OSContext*);
void fn_8033E0B4(OSAlarm*); // Initialize an OS alarm before attaching user data or scheduling it.
void OSSetAlarmUserData(OSAlarm*, void*);
void OSSetPeriodicAlarm(OSAlarm*, OSTime, OSTime, OSAlarmHandler);
void gtl_RemoteAudio_TimerCallback(OSAlarm*, OSContext*);
extern signed char s_remoteAudioInitialized;
extern char s_remoteAudio[];
void* gtl_RemoteAudioChannel_InitState(char*);

// Retry a busy Wii Remote enable request after five milliseconds.
void gtl_RemoteAudio_OnEnableResult(int index, int status) {
    if (status == kWpadBusy) {
        OSSetAlarm(&s_remoteAudioEnableRetryAlarms[index],
                   ((*(unsigned int*)0x800000F8 / 4) / 1000) * kEnableRetryMilliseconds,
                   gtl_RemoteAudio_RetryEnableAlarm);
    }
}

// Release the empty remote-audio helper only when called as a deleting destructor.
void* dtor_8040B3C4(void* self, int deleting) {
    if (self != 0) {
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Release the empty timer helper only when called as a deleting destructor.
void* dtor_8040C288(void* self, int deleting) {
    if (self != 0) {
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

void gtl_RemoteAudio_StopTimer(unsigned char* self);

extern const float kFloat_6_2831855;
extern float s_remoteAudioFullTurnAngle0;
extern float s_remoteAudioFullTurnAngle1;
extern float s_remoteAudioFullTurnAngle2;
extern float s_remoteAudioFullTurnAngle3;

// Report whether the embedded standard reverb controller is active.
bool gtl_nw4r_FxReverbStd_vfunc_1(gtl::nw4r::FxReverbStd* self) {
    return self->active != 0;
}

// Recover the channel index from alarm user data and retry enabling its speaker.
void gtl_RemoteAudio_RetryEnableAlarm(OSAlarm* alarm, OSContext*) {
    int index = (int)fn_8033E838(alarm);
    gtl_RemoteAudioChannel_RequestEnable(gtl_RemoteAudio_GetChannel(gtl_RemoteAudio_GetInstance(), index),
                                         gtl_RemoteAudio_OnEnableResult);
}

void gtl_RemoteAudio_RequestDisableAll() {
    for (int index = 0; index < kRemoteChannelCount; ++index) {
        gtl_RemoteAudioChannel_RequestDisable(gtl_RemoteAudio_GetChannel(gtl_RemoteAudio_GetInstance(), index), 0);
    }
}

void gtl_RemoteAudio_RequestEnableAll() {
    for (int index = 0; index < kRemoteChannelCount; ++index) {
        gtl_RemoteAudioChannel_RequestEnable(gtl_RemoteAudio_GetChannel(gtl_RemoteAudio_GetInstance(), index),
                                             gtl_RemoteAudio_OnEnableResult);
    }
}

// Enable a connected remote's speaker or request disable after disconnection.
void gtl_RemoteAudio_OnChannelStatus(int index, int status) {
    if (status == kWpadSuccess) {
        gtl_RemoteAudioChannel_RequestEnable(gtl_RemoteAudio_GetChannel(gtl_RemoteAudio_GetInstance(), index),
                                             gtl_RemoteAudio_OnEnableResult);
    } else if (status == kWpadDisconnected) {
        gtl_RemoteAudioChannel_RequestDisable(gtl_RemoteAudio_GetChannel(gtl_RemoteAudio_GetInstance(), index), 0);
    }
}

struct InterruptBase {
    ~InterruptBase() {
        dtor_8040B3C4(this, -1);
    }
};

struct InterruptGuard_u8040B260 : InterruptBase {
    BOOL level;
    InterruptGuard_u8040B260() {
        level = OSDisableInterrupts();
    }
    ~InterruptGuard_u8040B260() {
        OSRestoreInterrupts(level);
    }
};

int WPADCanSendStreamData(int);
int WPADSendStreamData(int, void*, int);
void WENCGetEncodeData(void*, int, void*, int, void*);
void gtl_RemoteAudioChannel_OnSendTimeout(void*);
void gtl_RemoteAudioChannel_OnStopAlarm(void*);

// Stop sample activity and cancel both per-channel alarms.
inline void ResetAlarms(unsigned char* state) {
    AsRemoteAudio(state)->sendingAudio = 0;
    AsRemoteAudio(state)->checkSilence = 0;
    OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(sendAlarm)));
    AsRemoteAudio(state)->sendAlarmArmed = 0;
    OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(stopAlarm)));
    (*(volatile unsigned char*)&AsRemoteAudio(state)->stopAlarmArmed) = 0;
}

#pragma use_lmw_stmw on
// Encode one PCM packet, send it to the Wii Remote, and maintain silence/timeout alarms.
void gtl_RemoteAudioChannel_SendSamples(unsigned char* state, void* samples) {
    if (AsRemoteAudio(state)->phase != kPhaseStreaming)
        return;
    bool stop;
    bool active = true;
    int i;
    bool silent;
    if (AsRemoteAudio(state)->checkSilence) {
        int* p = (int*)samples;
        silent = true;
        for (i = 0; i < kPcmWordsPerPacket; ++p, ++i) {
            if (*p) {
                silent = false;
                break;
            }
        }
    } else {
        silent = true;
    }
    if (silent || AsRemoteAudio(state)->sendTimedOut)
        active = false;
    bool start = false;
    if (!AsRemoteAudio(state)->sendingAudio && active)
        start = true;
    stop = false;
    if (AsRemoteAudio(state)->sendingAudio && !active)
        stop = true;
    if (active) {
        InterruptGuard_u8040B260 guard;
        if (!WPADCanSendStreamData(AsRemoteAudio(state)->channel))
            return;
        bool reset = !AsRemoteAudio(state)->firstPacket;
        AsRemoteAudio(state)->firstPacket = 0;
        char output[kAdpcmBytesPerPacket];
        WENCGetEncodeData(&AsRemoteAudio(state)->encoderState, reset, samples, kPcmSamplesPerPacket, output);
        if (WPADSendStreamData(AsRemoteAudio(state)->channel, output, kAdpcmBytesPerPacket)) {
            AsRemoteAudio(state)->phase = kPhaseIdle;
            AsRemoteAudio(state)->retryAction = kActionEnable;
            ResetAlarms(state);
            AsRemoteAudio(state)->sendTimedOut = 0;
            AsRemoteAudio(state)->sendAlarmArmed = 0;
            AsRemoteAudio(state)->sendingAudio = 0;
            AsRemoteAudio(state)->checkSilence = 1;
            AsRemoteAudio(state)->stopAlarmArmed = 0;
            return;
        }
    }
    if (start) {
        InterruptGuard_u8040B260 guard;
        if (!AsRemoteAudio(state)->sendAlarmArmed) {
            OSSetAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(sendAlarm)),
                       (OSTime)(*(unsigned int*)0x800000F8 >> 2) * kSendTimeoutSeconds,
                       (OSAlarmHandler)gtl_RemoteAudioChannel_OnSendTimeout);
            AsRemoteAudio(state)->sendStartTime = OSGetTime();
            AsRemoteAudio(state)->sendAlarmArmed = 1;
        }
        OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(stopAlarm)));
        AsRemoteAudio(state)->stopAlarmArmed = 0;
    }
    if (stop) {
        InterruptGuard_u8040B260 guard;
        AsRemoteAudio(state)->stopAlarmArmed = 1;
        OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(stopAlarm)));
        OSSetAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(stopAlarm)), (OSTime)(*(unsigned int*)0x800000F8 >> 2),
                   (OSAlarmHandler)gtl_RemoteAudioChannel_OnStopAlarm);
    }
    AsRemoteAudio(state)->sendingAudio = active;
}
#pragma use_lmw_stmw off

// Mark the alarm's channel as timed out so the next sample pump stops sending.
void gtl_RemoteAudioChannel_OnSendTimeout(void* self) {
    InterruptGuard_u8040B260 guard;
    unsigned char* state = (unsigned char*)fn_8033E838(self);
    AsRemoteAudio(state)->sendTimedOut = 1;
    AsRemoteAudio(state)->sendAlarmArmed = 0;
}

// Cancel the long send timeout after a pending silence stop.
void gtl_RemoteAudioChannel_OnStopAlarm(void* self) {
    InterruptGuard_u8040B260 guard;
    unsigned char* state = (unsigned char*)fn_8033E838(self);
    if (AsRemoteAudio(state)->stopAlarmArmed) {
        OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(sendAlarm)));
        AsRemoteAudio(state)->sendTimedOut = 0;
        AsRemoteAudio(state)->sendAlarmArmed = 0;
    }
    AsRemoteAudio(state)->stopAlarmArmed = 0;
}

// Stop the initialized audio workers, remote timer, voices, and controller before shutdown.
void gtl_Audio_ShutdownRuntime() {
    if (s_audioRuntimeInitialized) {
        gtl_AudioWorker_Stop(gtl_AudioWorker_GetInstance());
        gtl_AudioWorkQueue_CancelAll(GtlStreamTaskManager_GetInstance());
        AudioRuntime_StopWorkerThread(AudioMessageWorker_GetInstance());
        gtl_RemoteAudio_StopTimer((unsigned char*)gtl_RemoteAudio_GetInstance());
        gtl_VoicePool_Shutdown(gtl_VoicePool_GetInstance());
        AudioVoicePool_Shutdown(GtlVoicePool_GetInstance());
        gtl_AudioController_Shutdown(gtl_AudioController_GetInstance());
        s_audioRuntimeInitialized = 0;
    }
}

inline bool PollDone() {
    return gtl_AudioController_GetInstance()->pending == 0;
}

// Wait for controller updates to drain, with a one-second limit.
void gtl_Audio_WaitForPendingUpdates() {
    if (s_audioRuntimeInitialized) {
        OSTick start = OSGetTick();
        while (!PollDone()) {
            if ((OSGetTick() - start) / (*(unsigned int*)0x800000F8 >> 2))
                break;
        }
    }
}

// Cancel the remote-audio pump alarm if its running latch is set.
void gtl_RemoteAudio_StopTimer(unsigned char* self) {
    if (*self != 0) {
        OSCancelAlarm(self + 8);
        *self = 0;
    }
}

void gtl_RemoteAudio_StartTimer(unsigned char*);
void* memset(void*, int, unsigned long);

// On speaker-enable completion, reset the encoder and schedule streaming, or retry a busy command.
void gtl_RemoteAudioChannel_OnCommandCompletePhase2(int index, int status) {
    unsigned char* state = (unsigned char*)gtl_RemoteAudio_GetChannel(gtl_RemoteAudio_GetInstance(), index);
    switch (status) {
    case kWpadSuccess:
        AsRemoteAudio(state)->firstPacket = 1;
        memset(&AsRemoteAudio(state)->encoderState, 0, 0x20);
        AsRemoteAudio(state)->phase = kPhaseEnabled;
        AsRemoteAudio(state)->retryAction = kActionStartStream;
        break;
    case kWpadBusy:
        AsRemoteAudio(state)->retryAction = kActionEnable;
        break;
    case kWpadTransferError:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    case kWpadDisconnected:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    default:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    }
    if (status != kWpadSuccess && status != kWpadBusy && AsRemoteAudio(state)->callbackPending &&
        AsRemoteAudio(state)->callback) {
        (AsRemoteAudio(state)->callback)(index, status);
        AsRemoteAudio(state)->callback = 0;
    }
    AsRemoteAudio(state)->commandBusy = 0;
}

int WPADGetStatus();
void WPADSetConnectCallback(int, void (*)(int, int));
void gtl_AudioController_Initialize(PollState*);
int GtlVoicePool_GetRequiredMemorySize(void*);
void AudioVoicePool_InitializeFreeList(void*, void*, int);
int gtl_VoicePool_GetRequiredMemorySize(void*);
void gtl_VoicePool_Initialize(void*, void*, int);
void gtl_AudioMessageThread_Start(void*, int, void*, int);
void gtl_AudioWorker_Start(void*, int, void*, int);

#pragma use_lmw_stmw on
// Partition caller memory between worker stacks and voice pools, then start the audio runtime.
void gtl_Audio_InitializeRuntime(int* config, char* memory) {
    if (!s_audioRuntimeInitialized) {
        s_audioRuntimeInitialized = 1;
        while (WPADGetStatus() != kWpadReady) {
        }
        OSAlarm* alarm = s_remoteAudioEnableRetryAlarms;
        for (int index = 0; index < kRemoteChannelCount; ++index, ++alarm) {
            WPADSetConnectCallback(index, gtl_RemoteAudio_OnChannelStatus);
            fn_8033E0B4(alarm);
            OSSetAlarmUserData(alarm, (void*)index);
        }
        gtl_AudioController_Initialize(gtl_AudioController_GetInstance());
        gtl_RemoteAudio_StartTimer((unsigned char*)gtl_RemoteAudio_GetInstance());
        char* cursor = memory + config[3];
        char* first = cursor;
        cursor += config[1];
        int size;
        char* second = cursor;
        cursor += GtlVoicePool_GetRequiredMemorySize(GtlVoicePool_GetInstance());
        size = GtlVoicePool_GetRequiredMemorySize(GtlVoicePool_GetInstance());
        AudioVoicePool_InitializeFreeList(GtlVoicePool_GetInstance(), second, size);
        gtl_VoicePool_GetRequiredMemorySize(gtl_VoicePool_GetInstance());
        size = gtl_VoicePool_GetRequiredMemorySize(gtl_VoicePool_GetInstance());
        gtl_VoicePool_Initialize(gtl_VoicePool_GetInstance(), cursor, size);
        gtl_AudioMessageThread_Start(AudioMessageWorker_GetInstance(), config[2], memory, config[3]);
        gtl_AudioWorker_Start(gtl_AudioWorker_GetInstance(), config[0], first, config[1]);
    }
}
#pragma use_lmw_stmw off

// On stream-start completion, enter streaming or queue a busy-command retry and notify the client.
void RemoteAudio_HandleCommandCompletion(int index, int status) {
    unsigned char* state = (unsigned char*)gtl_RemoteAudio_GetChannel(gtl_RemoteAudio_GetInstance(), index);
    switch (status) {
    case kWpadSuccess:
        AsRemoteAudio(state)->phase = kPhaseStreaming;
        break;
    case kWpadBusy:
        AsRemoteAudio(state)->retryAction = kActionStartStream;
        break;
    case kWpadTransferError:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    case kWpadDisconnected:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    default:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    }
    if (status != kWpadBusy && AsRemoteAudio(state)->callbackPending && AsRemoteAudio(state)->callback) {
        (AsRemoteAudio(state)->callback)(index, status);
        AsRemoteAudio(state)->callback = 0;
    }
    AsRemoteAudio(state)->commandBusy = 0;
}

// On speaker-disable completion, enter the disabled phase or queue a retry and notify the client.
void gtl_RemoteAudioChannel_OnPhase6CommandComplete(int index, int status) {
    unsigned char* state = (unsigned char*)gtl_RemoteAudio_GetChannel(gtl_RemoteAudio_GetInstance(), index);
    switch (status) {
    case kWpadSuccess:
        AsRemoteAudio(state)->phase = kPhaseDisabled;
        break;
    case kWpadBusy:
        AsRemoteAudio(state)->retryAction = kActionDisable;
        break;
    case kWpadTransferError:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    case kWpadDisconnected:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    default:
        AsRemoteAudio(state)->phase = kPhaseIdle;
        break;
    }
    if (status != kWpadBusy && AsRemoteAudio(state)->callbackPending && AsRemoteAudio(state)->callback) {
        (AsRemoteAudio(state)->callback)(index, status);
        AsRemoteAudio(state)->callback = 0;
    }
    AsRemoteAudio(state)->commandBusy = 0;
}

// Return one channel record from the remote-audio singleton's inline array.
void* gtl_RemoteAudio_GetChannel(char* self, int index) {
    return self + index * kRemoteChannelBytes + kRemoteChannelsOffset;
}

// Initialize this remote-audio unit's fourth full-turn angle constant.
void RemoteAudio_InitFullTurnConstant3() {
    s_remoteAudioFullTurnAngle3 = kFloat_6_2831855;
}

// Initialize this remote-audio unit's first full-turn angle constant.
void RemoteAudio_InitFullTurnConstant0() {
    s_remoteAudioFullTurnAngle0 = kFloat_6_2831855;
}

// Initialize this remote-audio unit's second full-turn angle constant.
void RemoteAudio_InitFullTurnConstant1() {
    s_remoteAudioFullTurnAngle1 = kFloat_6_2831855;
}

// Initialize this remote-audio unit's third full-turn angle constant.
void RemoteAudio_InitFullTurnConstant2() {
    s_remoteAudioFullTurnAngle2 = kFloat_6_2831855;
}

// Start the periodic Wii Remote sample pump once.
void gtl_RemoteAudio_StartTimer(unsigned char* self) {
    if (*self == 0) {
        fn_8033E0B4((OSAlarm*)(self + (int)&((RemoteAudioSingletonView*)0)->timer));
        OSSetPeriodicAlarm((OSAlarm*)(self + (int)&((RemoteAudioSingletonView*)0)->timer), OSGetTime(),
                           ((*(unsigned int*)0x800000F8 / 4) / 125000) * 6666667 / 8000, gtl_RemoteAudio_TimerCallback);
        *self = 1;
    }
}

// Lazily initialize the four channel records and assign their remote indices.
char* gtl_RemoteAudio_GetInstance() {
    if (!s_remoteAudioInitialized) {
        s_remoteAudio[0] = 0;
        char* p = s_remoteAudio + kRemoteChannelsOffset;
        do {
            gtl_RemoteAudioChannel_InitState(p);
            p += kRemoteChannelBytes;
        } while (p < s_remoteAudio + kRemoteSingletonBytes);
        ((GtlRemoteAudioState*)(s_remoteAudio + kRemoteChannelsOffset))[0].channel = 0;
        ((GtlRemoteAudioState*)(s_remoteAudio + kRemoteChannelsOffset))[1].channel = 1;
        ((GtlRemoteAudioState*)(s_remoteAudio + kRemoteChannelsOffset))[2].channel = 2;
        ((GtlRemoteAudioState*)(s_remoteAudio + kRemoteChannelsOffset))[3].channel = 3;
        s_remoteAudioInitialized = 1;
    }
    return s_remoteAudio;
}

int AXRmtGetSamplesLeft();
void AXRmtGetSamples(int, void*, int);
void AXRmtAdvancePtr(int);
void gtl_RemoteAudioChannel_SendSamples(unsigned char*, void*);
void gtl_RemoteAudioChannel_DispatchCommand(unsigned char*);

// Pump one fixed-size sample packet and pending speaker commands for all four remotes.
void gtl_RemoteAudio_TimerCallback(OSAlarm*, OSContext*) {
    unsigned char* state;
    char* base = gtl_RemoteAudio_GetInstance();
    if (AXRmtGetSamplesLeft() >= kPcmSamplesPerPacket) {
        state = (unsigned char*)base + kRemoteChannelsOffset;
        short samples[kPcmSamplesPerPacket];
        for (int i = 0; i < kRemoteChannelCount; ++i, base += kRemoteChannelBytes, state += kRemoteChannelBytes) {
            if (*(int*)(base + kRemoteChannelsOffset + REMOTE_FIELD_OFFSET(phase)) == kPhaseStreaming) {
                AXRmtGetSamples(i, samples, kPcmSamplesPerPacket);
                gtl_RemoteAudioChannel_SendSamples(state, samples);
            }
            gtl_RemoteAudioChannel_DispatchCommand(state);
        }
        AXRmtAdvancePtr(kPcmSamplesPerPacket);
    }
}

// Reset channel control flags, callback state, and alarm user-data pointers.
void* gtl_RemoteAudioChannel_InitState(char* self) {
    AsRemoteAudio(self)->enabled = 0;
    AsRemoteAudio(self)->sendingAudio = 0;
    AsRemoteAudio(self)->checkSilence = 0;
    AsRemoteAudio(self)->firstPacket = 0;
    AsRemoteAudio(self)->callbackPending = 0;
    AsRemoteAudio(self)->commandBusy = 0;
    AsRemoteAudio(self)->sendTimedOut = 0;
    AsRemoteAudio(self)->phase = kPhaseIdle;
    AsRemoteAudio(self)->requestedAction = kActionNone;
    AsRemoteAudio(self)->retryAction = kActionNone;
    (*(int*)&AsRemoteAudio(self)->callback) = 0;
    fn_8033E0B4((OSAlarm*)(self + REMOTE_FIELD_OFFSET(sendAlarm)));
    OSSetAlarmUserData((OSAlarm*)(self + REMOTE_FIELD_OFFSET(sendAlarm)), self);
    fn_8033E0B4((OSAlarm*)(self + REMOTE_FIELD_OFFSET(stopAlarm)));
    OSSetAlarmUserData((OSAlarm*)(self + REMOTE_FIELD_OFFSET(stopAlarm)), self);
    return self;
}

// Reset stream alarms and queue speaker enable with the requested completion callback.
int gtl_RemoteAudioChannel_RequestEnable(void* self, void (*callback)(int, int)) {
    InterruptGuard_u8040B260 guard;
    unsigned char* state = (unsigned char*)self;
    AsRemoteAudio(state)->sendingAudio = 0;
    AsRemoteAudio(state)->checkSilence = 0;
    OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(sendAlarm)));
    AsRemoteAudio(state)->sendAlarmArmed = 0;
    OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(stopAlarm)));
    AsRemoteAudio(state)->stopAlarmArmed = 0;
    AsRemoteAudio(state)->sendTimedOut = 0;
    AsRemoteAudio(state)->sendAlarmArmed = 0;
    AsRemoteAudio(state)->sendingAudio = 0;
    AsRemoteAudio(state)->checkSilence = 1;
    AsRemoteAudio(state)->stopAlarmArmed = 0;
    if (AsRemoteAudio(state)->callback) {
        (AsRemoteAudio(state)->callback)(AsRemoteAudio(state)->channel, 0);
        AsRemoteAudio(state)->callbackPending = 0;
    }
    AsRemoteAudio(state)->callback = callback;
    AsRemoteAudio(state)->requestedAction = kActionEnable;
    AsRemoteAudio(state)->enabled = 1;
    return 1;
}

// Cancel stream alarms and queue speaker disable with the supplied callback address.
void gtl_RemoteAudioChannel_RequestDisable(void* self, int callback) {
    InterruptGuard_u8040B260 guard;
    unsigned char* state = (unsigned char*)self;
    AsRemoteAudio(state)->sendingAudio = 0;
    AsRemoteAudio(state)->checkSilence = 0;
    OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(sendAlarm)));
    AsRemoteAudio(state)->sendAlarmArmed = 0;
    OSCancelAlarm((OSAlarm*)(state + REMOTE_FIELD_OFFSET(stopAlarm)));
    AsRemoteAudio(state)->stopAlarmArmed = 0;
    if (AsRemoteAudio(state)->callback) {
        (AsRemoteAudio(state)->callback)(AsRemoteAudio(state)->channel, 0);
        AsRemoteAudio(state)->callbackPending = 0;
    }
    AsRemoteAudio(state)->requestedAction = kActionDisable;
    (*(int*)&AsRemoteAudio(state)->callback) = callback;
    AsRemoteAudio(state)->enabled = 0;
}

void gtl_RemoteAudioChannel_OnCommandCompletePhase2(int, int);
void RemoteAudio_HandleCommandCompletion(int, int);
void gtl_RemoteAudioChannel_OnPhase6CommandComplete(int, int);
void WPADControlSpeaker(int, int, void (*)(int, int));

// Submit one requested (or retried) speaker command when the previous command has completed.
void gtl_RemoteAudioChannel_DispatchCommand(unsigned char* state) {
    if (AsRemoteAudio(state)->commandBusy)
        return;
    int action;
    if (AsRemoteAudio(state)->requestedAction)
        action = AsRemoteAudio(state)->requestedAction;
    else
        action = AsRemoteAudio(state)->retryAction;
    AsRemoteAudio(state)->requestedAction = kActionNone;
    AsRemoteAudio(state)->retryAction = kActionNone;
    switch (action) {
    case kActionNone:
        break;
    case kActionEnable:
        AsRemoteAudio(state)->callbackPending = 1;
        AsRemoteAudio(state)->commandBusy = 1;
        AsRemoteAudio(state)->phase = kPhaseEnabling;
        WPADControlSpeaker(AsRemoteAudio(state)->channel, kSpeakerEnable,
                           gtl_RemoteAudioChannel_OnCommandCompletePhase2);
        break;
    case kActionStartStream:
        AsRemoteAudio(state)->callbackPending = 1;
        AsRemoteAudio(state)->commandBusy = 1;
        AsRemoteAudio(state)->phase = kPhaseStartingStream;
        WPADControlSpeaker(AsRemoteAudio(state)->channel, kSpeakerStartStream, RemoteAudio_HandleCommandCompletion);
        break;
    case kActionDisable:
        AsRemoteAudio(state)->callbackPending = 1;
        AsRemoteAudio(state)->commandBusy = 1;
        AsRemoteAudio(state)->phase = kPhaseDisabling;
        WPADControlSpeaker(AsRemoteAudio(state)->channel, kSpeakerDisable,
                           gtl_RemoteAudioChannel_OnPhase6CommandComplete);
        break;
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626DC", _ctors_u8040B260_0, RemoteAudio_InitFullTurnConstant0);
STATIC_INIT_ENTRY(".CRT$XCU804626DC", _ctors_u8040B260_1, RemoteAudio_InitFullTurnConstant1);
STATIC_INIT_ENTRY(".CRT$XCU804626DC", _ctors_u8040B260_2, RemoteAudio_InitFullTurnConstant2);
STATIC_INIT_ENTRY(".CRT$XCU804626DC", _ctors_u8040B260_3, RemoteAudio_InitFullTurnConstant3);
#pragma pop
