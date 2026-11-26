// Audio stream tasks and voice setup: the stream task manager with its three priority queues,
// the audio message thread that drains them, and the voice playback parameters and teardown.
// Also holds the intrusive lists and per-channel stream data used by streaming playback.

#include <decomp/static_init.h>
#include "classes/Voice8040E9BC.h"
#include "classes/GtlAudioControllerState.h"

// AX handles form a 2-by-4 grid at +4: [column][row], with 0x10-byte
// column stride. Per-row parameters instead start at +0x24, stride 0x1C.
// These are storage axes; their speaker/channel meanings depend on the caller.
// Task manager: mutex +0, sleep queue +0x18, active task +0x20,
// three 12-byte priority lists +0x24/+0x30/+0x3C (highest first: 2,1,0),
// unit heap +0x48. A list is {count, sentinel.next, sentinel.prev}.
// Tasks own links at +4 and an owner tag at +0xC. The heap owns task storage;
// running/cancelling a task does not destroy the externally owned owner tag.

// This message worker has its own layout; it is not StreamWorker8040C2C8.
struct AudioMessageWorker {
    char thread[0x318];
    char messageQueue[0x20];
    void* messages[2];
    char* stack;
    unsigned char running;
};
struct StreamChannelBlockData {
    char* data;
    char reserved[0x34];
};
struct StreamLoadData {
    char reserved_000[0x7C];
    unsigned char channelStateLocked;
    char reserved_07D[0x39];
    unsigned char channelStateDirty;
    char reserved_0B7[0x11];
    int blockStride;
    char reserved_0CC[0x3D8];
    int channelCount;
    char reserved_4A8[4];
    StreamChannelBlockData channels[2];
    unsigned short channelStates[2];
};
struct StreamLoadContext {
    char reserved_000[4];
    StreamLoadData* data;
    char reserved_008[8];
    int blockIndex;
};
struct AxVoiceStateView {
    char reserved[0x38];
    unsigned short playbackState;
};
enum {
    kAudioMessageQueueOffset = (int)&((AudioMessageWorker*)0)->messageQueue
};
static const int kStreamMessageRunTasks = 1;
static const int kStreamMessageStop = 2;
static const int kStreamMessageCapacity = 2;
static const int kStreamTaskPriorityCount = 3;
static const int kVoiceMaxColumns = 2;
static const int kVoiceMaxRows = 4;
static const int kAxVoicePriorityNormal = 0x10;
static const int kAxVoicePrioritySteal = 0x11;
static const int kAxVoicePriorityHighest = 0x1F;
static const int kVoiceHighestPriorityRequest = 0xFF;
static const int kVoiceStartPending = 0x01;
static const int kVoicePausePending = 0x02;
static const int kVoicePitchDirty = 0x04;
static const int kVoiceVolumeDirty = 0x08;
static const int kVoiceOutputMixDirty = 0x10;
static const int kVoiceLowPassDirty = 0x20;
static const int kVoiceBiquadDirty = 0x80;
static const int kVoiceAfterStartMask = 0xFA;
static const int kVoiceAfterPitchMask = 0xFB;
static const int kVoiceAfterPauseMask = 0xFD;

#include "sdk_protos.h"

extern "C" {

struct InterruptUnwindCleanup {
    BOOL level;
    bool active;
    // Retain the caller's interrupt level for exceptional exits.
    explicit InterruptUnwindCleanup(BOOL saved) : level(saved), active(true) {}
    // Transfer restoration responsibility to the normal exit path.
    BOOL release() {
        active = false;
        return level;
    }
    // Restore interrupts only when normal code has not taken responsibility.
    ~InterruptUnwindCleanup() {
        if (active)
            OSRestoreInterrupts(level);
    }
};

extern float s_audioStreamFullTurnRadians;
extern float s_gtlVoiceFullTurnRadians;
extern const float kFloat_6_2831855;

struct ListNode {
    void* volatile next;
    void* volatile prev;
};

struct List {
    volatile int count;
    ListNode node;
};

extern List s_activeStreamPlayers;
extern char s_activeStreamPlayersDtorRegistration[12];
extern volatile float s_activeStreamPlayersFullTurnRadians;
void* AudioStreamPlayerList_Dtor(void* self, int flag);
void gtl_IntrusiveList_Dtor(void*, int);
void OSJamMessage(void*, int, int);
void OSJoinThread(void*, int);
void __dl__FPv(void*);
int SoundStreamTaskQueue_RunNext(void*);
void SoundStreamTaskList_InitEmpty(List* self);

// Destroy the active-player intrusive list and optionally free its storage.
void* AudioStreamPlayerList_Dtor(void* self, int flag) {
    if (self) {
        gtl_IntrusiveList_Dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy an intrusive task list and optionally free its storage.
void* SoundStreamTaskList_dtor(void* self, int flag) {
    if (self) {
        gtl_IntrusiveList_Dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Free an empty task-list wrapper when called as a deleting destructor.
void* dtor_8040EC7C(void* self, int flag) {
    if (self && flag > 0)
        __dl__FPv(self);
    return self;
}

#pragma push
#pragma dont_inline on
// Run queued streaming tasks until the queue has no work.
void SoundStreamTaskQueue_Drain(void* self) {
    do {
    } while (SoundStreamTaskQueue_RunNext(self));
}
#pragma pop

// Initialize an empty intrusive list with a self-linked sentinel.
void SoundStreamTaskList_InitEmpty(List* self) {
    ListNode* node = &self->node;
    node->next = 0;
    node->prev = 0;
    self->count = 0;
    node->next = node;
    node->prev = node;
}

// Initialize the active streaming-player list and register its global destructor.
void AudioStreamPlayers_InitializeRegistry() {
    List* list = &s_activeStreamPlayers;
    float angle = *(const volatile float*)&kFloat_6_2831855;
    ListNode* node = &list->node;
    node->next = 0;
    node->prev = 0;
    s_activeStreamPlayersFullTurnRadians = angle;
    *(int*)&list->count = 0;
    *(void**)&node->next = node;
    node->prev = node;
    __register_global_object(&s_activeStreamPlayers, (void*)AudioStreamPlayerList_Dtor,
                             s_activeStreamPlayersDtorRegistration);
}

typedef AudioMessageWorker Singleton;

extern Singleton s_audioRuntimeWorker;
extern signed char s_audioWorkerStateInitialized;

// Post the stop command, join the message thread, and clear its running flag.
void AudioRuntime_StopWorkerThread(Singleton* self) {
    if (self->running) {
        OSJamMessage(((AudioMessageWorker*)self)->messageQueue, kStreamMessageStop, 1);
        OSJoinThread(self, 0);
        self->running = 0;
    }
}

// Return the lazily initialized audio message-worker singleton.
Singleton* AudioMessageWorker_GetInstance() {
    if (!s_audioWorkerStateInitialized) {
        s_audioRuntimeWorker.stack = 0;
        s_audioRuntimeWorker.running = 0;
        s_audioWorkerStateInitialized = 1;
    }
    return &s_audioRuntimeWorker;
}

typedef StreamLoadData Data;
typedef StreamLoadContext Context;

// Return one channel block at the current block index, or null when the channel index is too large.
// Borrowed data; caller must supply a nonnegative channel and a valid block index.
// blockStride is bytes per block in each channel's separate data allocation.
char* gtl_StreamLoadTask_GetChannelBlockData(Context* self, int channelIndex) {
    Data* data = self->data;
    if (channelIndex >= data->channelCount)
        return 0;
    return data->channels[channelIndex].data + data->blockStride * self->blockIndex;
}

// Initialize this translation unit's first two-pi cache.
void AudioStream_InitializeFullTurnConstant() {
    s_audioStreamFullTurnRadians = kFloat_6_2831855;
}

// Initialize this translation unit's second two-pi cache.
void GtlVoice_InitializeFullTurnConstant() {
    s_gtlVoiceFullTurnRadians = kFloat_6_2831855;
}

// Request asynchronous task processing without blocking on a full message queue.
void SoundStream_PostMessageOne(char* self) {
    OSSendMessage((OSMessageQueue*)(self + kAudioMessageQueueOffset), (OSMessage)kStreamMessageRunTasks, 0);
}

// Free the message-worker wrapper when called as a deleting destructor.
void* dtor_8040F368(void* self, int flag) {
    if (self && flag > 0)
        __dl__FPv(self);
    return self;
}

extern char __vt__Q43gtl4nw4r6detail5Voice[];

// Install the voice vtable and clear playback flags, callback state and AX voice handles.
void gtl_nw4r_detail_Voice_Ctor(Voice* self) {
    self->eventCallback = 0;
    self->vtable = __vt__Q43gtl4nw4r6detail5Voice;
    self->allocated = 0;
    self->playbackRequested = 0;
    self->hardwareStarted = 0;
    self->pauseRequested = 0;
    self->pendingChanges = 0;
    self->poolLinkNext = 0;
    self->poolLinkPrevious = 0;
    self->hardwareVoices[0] = 0;
    self->hardwareVoices[1] = 0;
    self->hardwareVoices[2] = 0;
    self->hardwareVoices[3] = 0;
    self->hardwareVoices[4] = 0;
    self->hardwareVoices[5] = 0;
    self->hardwareVoices[6] = 0;
    self->hardwareVoices[7] = 0;
}

// Update up to two stream-channel states unless locked, then mark them dirty.
void StreamLoadTask_SetChannelStates(Context* self, int count, unsigned short* source) {
    if (self->data->channelStateLocked)
        return;
    for (int i = 0; i < count && i < 2; ++i) {
        self->data->channelStates[i] = *source++;
    }
    self->data->channelStateDirty = 1;
}

void* GtlStreamTaskManager_GetInstance();
void __destroy_arr(void*, void*, unsigned long, unsigned long);

// Drain streaming tasks for run messages and exit when a stop message arrives.
void* SoundStream_MessageThreadMain(char* self) {
    OSMessage message;
    for (;;) {
        OSReceiveMessage((OSMessageQueue*)(self + kAudioMessageQueueOffset), &message, 1);
        if ((unsigned long)message == kStreamMessageRunTasks) {
            SoundStreamTaskQueue_Drain(GtlStreamTaskManager_GetInstance());
        } else if ((unsigned long)message == kStreamMessageStop) {
            return 0;
        }
    }
}

int OSResumeThread(OSThread*);
// Initialize the two-message queue and start the worker using the supplied stack and priority.
// The stack remains borrowed until the thread is joined. Returns 0 on creation
// failure, 1 on success/already running; failure still leaves running set to 1.
int gtl_AudioMessageThread_Start(char* self, int priority, char* stack, unsigned long size) {
    if (((AudioMessageWorker*)self)->running)
        return true;
    ((AudioMessageWorker*)self)->running = 1;
    OSInitMessageQueue((OSMessageQueue*)((AudioMessageWorker*)self)->messageQueue,
                       (OSMessage*)((AudioMessageWorker*)self)->messages, kStreamMessageCapacity);
    ((AudioMessageWorker*)self)->stack = stack;
    int result = OSCreateThread((OSThread*)self, (OSThreadFunc)SoundStream_MessageThreadMain, self, stack + size, size,
                                priority, 0);
    if (result)
        OSResumeThread((OSThread*)self);
    return result != 0;
}

extern const float kFloat_1_0_AudioVoice;
extern const float kFloat_0_AudioVoice;

// Set voice dimensions and event callback, then reset pitch, volume, routing and send defaults.
void GtlVoice_InitializePlaybackParameters(char* self, int columns, int rows, int eventCallback, int callbackArgument) {
    ((Voice*)self)->sourceChannelCount = columns;
    ((Voice*)self)->outputVoiceCount = rows;
    ((Voice*)self)->eventCallback = eventCallback;
    ((Voice*)self)->callbackArgument = callbackArgument;
    ((Voice*)self)->pendingChanges = 0;
    ((Voice*)self)->pauseRequested = 0;
    ((Voice*)self)->hardwarePaused = 0;
    ((Voice*)self)->hardwareStarted = 0;
    ((Voice*)self)->volumeScale = kFloat_1_0_AudioVoice;
    ((Voice*)self)->initialVolume = kFloat_0_AudioVoice;
    ((Voice*)self)->targetVolume = kFloat_1_0_AudioVoice;
    ((Voice*)self)->lowPassFilter = kFloat_1_0_AudioVoice;
    ((Voice*)self)->pan = kFloat_0_AudioVoice;
    ((Voice*)self)->surroundPan = kFloat_0_AudioVoice;
    ((Voice*)self)->panOffset = kFloat_0_AudioVoice;
    ((Voice*)self)->surroundPanOffset = kFloat_0_AudioVoice;
    ((Voice*)self)->outputLineMask = 1;
    ((Voice*)self)->mainVolume = kFloat_1_0_AudioVoice;
    ((Voice*)self)->mainSend = kFloat_1_0_AudioVoice;
    ((Voice*)self)->auxASend = kFloat_0_AudioVoice;
    ((Voice*)self)->auxBSend = kFloat_0_AudioVoice;
    ((Voice*)self)->auxCSend = kFloat_0_AudioVoice;
    ((Voice*)self)->remoteVolume0 = kFloat_1_0_AudioVoice;
    ((Voice*)self)->remoteSend0 = kFloat_1_0_AudioVoice;
    ((Voice*)self)->additionalOutputMix0 = kFloat_0_AudioVoice;
    ((Voice*)self)->remoteVolume1 = kFloat_1_0_AudioVoice;
    ((Voice*)self)->remoteSend1 = kFloat_1_0_AudioVoice;
    ((Voice*)self)->additionalOutputMix1 = kFloat_0_AudioVoice;
    ((Voice*)self)->remoteVolume2 = kFloat_1_0_AudioVoice;
    ((Voice*)self)->remoteSend2 = kFloat_1_0_AudioVoice;
    ((Voice*)self)->additionalOutputMix2 = kFloat_0_AudioVoice;
    ((Voice*)self)->remoteVolume3 = kFloat_1_0_AudioVoice;
    ((Voice*)self)->remoteSend3 = kFloat_1_0_AudioVoice;
    ((Voice*)self)->additionalOutputMix3 = kFloat_0_AudioVoice;
    ((Voice*)self)->pitchScale = kFloat_1_0_AudioVoice;
    self[0xAA] = 0;
    ((Voice*)self)->stereoPanMode = 1;
    ((Voice*)self)->panGainMode = 1;
}

// activeTaskWord stores the active-task pointer bits in the +0x20 slot (see map above).
struct Manager {
    char mutex[0x18];
    char queue[8];
    int activeTaskWord;
    List lists[3];
    void* pool;
    void init(char*);
};

// Destroy all three task-priority lists and optionally free the task manager.
void* SoundStreamTaskManager_Dtor(void* self, int flag) {
    if (self) {
        __destroy_arr(((Manager*)self)->lists, (void*)SoundStreamTaskList_dtor, 12, 3);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

extern "C" {
struct ManagerGlobals {
    char pool[0x2044];
    char regmem[12];
    Manager manager;
};
extern ManagerGlobals s_streamTaskManagerStorage;
extern signed char s_streamTaskManagerInitialized;
void* MEMCreateUnitHeapEx(void*, unsigned long, unsigned long, int, int);
void __construct_array(void*, void*, void*, unsigned long, unsigned long);
}
// Initialize the priority queues, synchronization objects and fixed-size task heap.
inline void Manager::init(char* base) {
    activeTaskWord = 0;
    __construct_array(lists, (void*)SoundStreamTaskList_InitEmpty, (void*)SoundStreamTaskList_dtor, 12, 3);
    OSInitMutex((OSMutex*)this);
    OSInitThreadQueue((OSThreadQueue*)queue);
    pool = MEMCreateUnitHeapEx(base, 0x2044, 0x40, 4, 0);
}
extern "C" {
// Create the task manager, fixed-size work-item heap and global cleanup registration on first use.
void* GtlStreamTaskManager_GetInstance() {
    ManagerGlobals* base = &s_streamTaskManagerStorage;
    if (!s_streamTaskManagerInitialized) {
        Manager* self = &base->manager;
        self->init(base->pool);
        __register_global_object(self, (void*)SoundStreamTaskManager_Dtor, base->regmem);
        s_streamTaskManagerInitialized = 1;
    }
    return &base->manager;
}

void* MEMAllocFromUnitHeap(void*);

// Note: GtlVoicePool_GetInstance (0x80409280, pool with the AX acquire/release calls) and gtl_VoicePool_GetInstance
// (0x80411818, whose list entries AllocateAxVoiceGrid walks) are two different singleton getters, not one renamed.
void* GtlVoicePool_GetInstance();
void gtl_AxVoicePool_ReleaseVoice(void*, void*);
int GtlVoice_IsReadingOutputBuffer(void*);
void gtl_IntrusiveList_InsertBefore(void*, void*, void*);
int AudioVoicePool_AcquireAXVoice(void*, int, void*, void*);
void* gtl_VoicePool_GetInstance();
void AudioVoice_SetPriorityIfAllocated(void*, int);
void gtl_AxVoice_SetPitch(void*, float, int);
void gtl_AxVoice_SetLowPassFrequency(void*, unsigned short);
void AudioVoice_SetBiquadFilter(void*, unsigned char);
void gtl_nw4r_detail_Voice_ApplyVolume(void*);
int AudioVolume_NormalizedToExponentialGain32000(float);

extern char GtlVoice_HandleAxVoiceRemoval[];

#pragma push
#pragma use_lmw_stmw on
// Accessors for a voice-pool list entry given its intrusive link pointer (fields sit at negative offsets).
static inline int PoolEntryPriority(void* link) {
    return *(int*)((char*)link - 0x78);
}
// The link is the Voice::poolLinkNext member, so the owning Voice sits kVoicePoolLinkOffset bytes before it.
enum {
    kVoicePoolLinkOffset = (int)&((Voice*)0)->poolLinkNext
};
static inline Voice* PoolEntryVoice(void* link) {
    return (Voice*)((char*)link - kVoicePoolLinkOffset);
}
// Number of AX voices the pooled voice holds: columns (sourceChannelCount) times rows (outputVoiceCount).
static inline int PoolEntryColumns(void* link) {
    return PoolEntryVoice(link)->sourceChannelCount;
}
static inline int PoolEntryRows(void* link) {
    return PoolEntryVoice(link)->outputVoiceCount;
}
// Acquire a bounded grid of AX voices, attempting requestPriority-based stealing and releasing partial allocations on failure.
int gtl::nw4r::detail::Voice::AllocateAxVoiceGrid(int columns, int rows, int requestPriority, int eventCallback,
                                                  int callbackArgument) {
    int columnCount = columns > kVoiceMaxColumns ? kVoiceMaxColumns : (columns < 1 ? 1 : columns);
    int rowCount = rows > kVoiceMaxRows ? kVoiceMaxRows : (rows < 1 ? 1 : rows);
    BOOL interruptLevel = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup0(interruptLevel);
    void* acquired[kVoiceMaxColumns * kVoiceMaxRows]; // AX voice handles acquired so far (column-major)
    // Phase 1: acquire every handle before publishing any of them into self.
    // requestPriority is the logical stealing requestPriority, not a third grid dimension.
    int axVoicePriority =
        requestPriority == kVoiceHighestPriorityRequest ? kAxVoicePriorityHighest : kAxVoicePriorityNormal;
    int total = columnCount * rowCount;
    int acquiredCount = 0;
    for (; acquiredCount < total; ++acquiredCount) {
        void* object = (void*)AudioVoicePool_AcquireAXVoice(GtlVoicePool_GetInstance(), axVoicePriority,
                                                            &GtlVoice_HandleAxVoiceRemoval, this);
        if (!object) {
            // Check that eligible lower/equal-requestPriority voices can cover the shortfall.
            int shortfall = total - acquiredCount;
            void* node = gtl_VoicePool_GetInstance();
            void** end = (void**)((char*)node + 8);
            void* it = *end;
            while (it != end) {
                if (requestPriority < PoolEntryPriority(it))
                    break;
                shortfall -= PoolEntryColumns(it) * PoolEntryRows(it);
                if (shortfall <= 0)
                    break;
                it = *(void**)it;
            }
            if (shortfall > 0) {
                for (int i = 0; i < acquiredCount; ++i) {
                    gtl_AxVoicePool_ReleaseVoice(GtlVoicePool_GetInstance(), acquired[i]);
                }
                OSRestoreInterrupts(interruptCleanup0.release());
                return 0;
            }
            int fallback = axVoicePriority == kAxVoicePriorityHighest ? kAxVoicePriorityHighest : kAxVoicePrioritySteal;
            object = (void*)AudioVoicePool_AcquireAXVoice(GtlVoicePool_GetInstance(), fallback,
                                                          &GtlVoice_HandleAxVoiceRemoval, this);
            if (!object) {
                for (int i = 0; i < acquiredCount; ++i) {
                    gtl_AxVoicePool_ReleaseVoice(GtlVoicePool_GetInstance(), acquired[i]);
                }
                OSRestoreInterrupts(interruptCleanup0.release());
                return 0;
            }
        }
        acquired[acquiredCount] = object;
    }
    // Phase 2: publish the complete column-major grid and reset playback defaults.
    // Returns 1 with ownership transferred to self; 0 releases partial acquisition.
    char* row = (char*)this;
    int index = 0;
    for (int i = 0; i < columnCount; ++i) {
        char* cell = row;
        for (int j = 0; j < rowCount; ++j) {
            void* value = acquired[index++];
            AudioVoice_SetPriorityIfAllocated(value, axVoicePriority);
            *(void**)(cell + 4) = value;
            cell += 4;
        }
        row += 0x10;
    }
    GtlVoice_InitializePlaybackParameters((char*)this, columnCount, rowCount, eventCallback, callbackArgument);
    allocated = 1;
    OSRestoreInterrupts(interruptCleanup0.release());
    return 1;
}
#pragma pop

// Apply dirty pitch, volume, output mix and filter parameters to allocated AX voices.
extern "C" void gtl_nw4r_detail_Voice_ApplyPendingParameters(Voice* self) {
    if (!self->playbackRequested)
        return;
    if (self->pendingChanges & kVoicePitchDirty) {
        float one = kFloat_1_0_AudioVoice;
        float zero = kFloat_0_AudioVoice;
        char* column = (char*)self;
        for (int i = 0; i < ((Voice*)self)->outputVoiceCount; ++i, column += 0x1C) {
            float amount = ((Voice*)column)->channels[0].channelPitchScale;
            if (amount > one)
                amount = one;
            else if (amount < zero)
                amount = zero;
            float scaled = ((Voice*)self)->pitchScale * amount;
            char* cell = (char*)self + i * 4;
            for (int j = 0; j < ((Voice*)self)->sourceChannelCount; ++j, cell += 0x10) {
                void* object = *(void**)(cell + 4);
                if (object)
                    gtl_AxVoice_SetPitch(object, scaled, 0);
            }
        }
        self->pendingChanges &= ~kVoicePitchDirty;
    }
    if (self->pendingChanges & kVoiceVolumeDirty) {
        gtl_nw4r_detail_Voice_ApplyVolume(self);
        self->pendingChanges &= ~kVoiceVolumeDirty;
    }
    if ((self->pendingChanges & kVoiceOutputMixDirty) &&
        !((gtl::nw4r::detail::Voice*)self)->ApplyOutputMixCoefficients())
        self->pendingChanges &= ~kVoiceOutputMixDirty;
    if (self->pendingChanges & kVoiceLowPassDirty) {
        char* row = (char*)self;
        for (int i = 0; i < ((Voice*)self)->outputVoiceCount; ++i, row += 0x1C) {
            unsigned short value = (unsigned short)AudioVolume_NormalizedToExponentialGain32000(
                ((Voice*)self)->lowPassFilter + ((Voice*)row)->channels[0].lowPassFilterOffset);
            char* cell = (char*)self + i * 4;
            for (int j = 0; j < ((Voice*)self)->sourceChannelCount; ++j, cell += 0x10) {
                void* object = *(void**)(cell + 4);
                if (object)
                    gtl_AxVoice_SetLowPassFrequency(object, value);
            }
        }
        self->pendingChanges &= ~kVoiceLowPassDirty;
    }
    if (self->pendingChanges & kVoiceBiquadDirty) {
        char* row = (char*)self;
        for (int i = 0; i < ((Voice*)self)->outputVoiceCount; ++i) {
            char* cell = row;
            for (int j = 0; j < ((Voice*)self)->sourceChannelCount; ++j, cell += 0x10) {
                void* object = *(void**)(cell + 4);
                if (object)
                    AudioVoice_SetBiquadFilter(object, *(unsigned char*)((Voice*)self)->biquadFilterState);
            }
            row += 4;
        }
        self->pendingChanges &= ~kVoiceBiquadDirty;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Release every allocated AX voice in the grid and optionally free the wrapper.
Voice* gtl_nw4r_detail_Voice_Dtor(Voice* self, int flag) {
    if (self) {
        self->vtable = __vt__Q43gtl4nw4r6detail5Voice;
        char* row = (char*)self;
        char* column;
        for (int i = 0; i < 2; ++i) {
            column = row;
            for (int j = 0; j < 4; ++j) {
                void* value = *(void**)(column + 4);
                if (value)
                    gtl_AxVoicePool_ReleaseVoice(GtlVoicePool_GetInstance(), value);
                column += 4;
            }
            row += 16;
        }
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Notify event 0 when the first AX handle's DSP cursor reaches the silence buffer, then clear playback state.
// The callback receives (self, 0, callbackArgument); this condition alone does not
// establish whether the event means natural completion or a hardware transition.
void GtlVoice_NotifyOutputBufferReadStarted(Voice* self) {
    if (self->allocated && self->hardwareStarted) {
        bool ready = false;
        if (self->hardwareVoices[0] && GtlVoice_IsReadingOutputBuffer((void*)self->hardwareVoices[0]))
            ready = true;
        if (ready) {
            if (self->eventCallback)
                ((void (*)(Voice*, int, int))self->eventCallback)(self, 0, self->callbackArgument);
            self->hardwareStarted = 0;
            self->playbackRequested = 0;
        }
    }
}

} // extern "C"

struct InterruptBase {
    // Empty base preserves the guard's original exception-cleanup shape.
    ~InterruptBase() {}
};

struct InterruptGuard : InterruptBase {
    int level;
    // Save and disable interrupts for the guarded operation.
    InterruptGuard() : level(OSDisableInterrupts()) {}
    // Restore the incoming interrupt level, including during unwinding.
    ~InterruptGuard() {
        OSRestoreInterrupts(level);
    }
};

extern "C" void AXSetVoiceState(void*, int);
extern "C" void* gtl_AudioController_GetInstance();
extern "C" void GtlVoice_UpdateVolumeRamp(void*);
// Apply pending start/pause changes and update volume ramps while interrupts are disabled.
extern "C" void GtlVoice_UpdatePlaybackState(char* self) {
    InterruptGuard guard;
    if (!((Voice*)self)->allocated)
        return;
    // Phase 1: push pitch before starting all hardware handles; acknowledge start/pitch bits.
    if ((((Voice*)self)->pendingChanges & kVoiceStartPending) && ((Voice*)self)->playbackRequested &&
        !((Voice*)self)->hardwareStarted) {
        float zero = kFloat_0_AudioVoice, one = kFloat_1_0_AudioVoice;
        char* row = self;
        int offset = 0;
        for (int i = 0; i < ((Voice*)self)->outputVoiceCount; ++i) {
            float amount = ((Voice*)row)->channels[0].channelPitchScale;
            if (amount > one)
                amount = one;
            else if (amount < zero)
                amount = zero;
            float scaled = ((Voice*)self)->pitchScale * amount;
            char* cell = self + offset;
            for (int j = 0; j < ((Voice*)self)->sourceChannelCount; ++j) {
                void* object = *(void**)(cell + 4);
                if (object)
                    gtl_AxVoice_SetPitch(object, scaled, 1);
                cell += 0x10;
            }
            row += 0x1C;
            offset += 4;
        }
        char* outer = self;
        for (int i = 0; i < ((Voice*)self)->sourceChannelCount; ++i) {
            char* cell = outer;
            for (int j = 0; j < ((Voice*)self)->outputVoiceCount; ++j) {
                void* object = *(void**)(cell + 4);
                if (object && *(void**)object)
                    AXSetVoiceState(*(void**)object, 1);
                cell += 4;
            }
            outer += 0x10;
        }
        ((Voice*)self)->hardwareStarted = 1;
        unsigned char flags = ((Voice*)self)->pendingChanges;
        flags &= kVoiceAfterStartMask;
        flags &= kVoiceAfterPitchMask;
        ((Voice*)self)->pendingChanges = flags;
    }
    // Phase 2: reconcile explicit pause and drive-status pause, then service ramps.
    if (((Voice*)self)->hardwareStarted) {
        if ((((Voice*)self)->pendingChanges & kVoicePausePending) && ((Voice*)self)->playbackRequested) {
            if (((Voice*)self)->pauseRequested ||
                AsAudioController80404E94(gtl_AudioController_GetInstance())->driveStatusChanged) {
                char* outer = self;
                for (int i = 0; i < ((Voice*)self)->sourceChannelCount; ++i) {
                    char* cell = outer;
                    for (int j = 0; j < ((Voice*)self)->outputVoiceCount; ++j) {
                        void* object = *(void**)(cell + 4);
                        if (object) {
                            void* child = *(void**)object;
                            if (child && ((AxVoiceStateView*)child)->playbackState == 1)
                                AXSetVoiceState(child, 0);
                        }
                        cell += 4;
                    }
                    outer += 0x10;
                }
                ((Voice*)self)->hardwarePaused = 1;
            } else {
                char* outer = self;
                for (int i = 0; i < ((Voice*)self)->sourceChannelCount; ++i) {
                    char* cell = outer;
                    for (int j = 0; j < ((Voice*)self)->outputVoiceCount; ++j) {
                        void* object = *(void**)(cell + 4);
                        if (object && *(void**)object)
                            AXSetVoiceState(*(void**)object, 1);
                        cell += 4;
                    }
                    outer += 0x10;
                }
                ((Voice*)self)->hardwarePaused = 0;
            }
            ((Voice*)self)->pendingChanges &= kVoiceAfterPauseMask;
        }
        char* outer = self;
        for (int i = 0; i < ((Voice*)self)->sourceChannelCount; ++i) {
            char* cell = outer;
            for (int j = 0; j < ((Voice*)self)->outputVoiceCount; ++j) {
                void* object = *(void**)(cell + 4);
                if (object)
                    GtlVoice_UpdateVolumeRamp(object);
                cell += 4;
            }
            outer += 0x10;
        }
    }
}

struct MutexGuard : InterruptBase {
    OSMutex* mutex;
    // Lock the manager across task execution or bulk cancellation.
    MutexGuard(void* p) : mutex((OSMutex*)p) {
        OSLockMutex(mutex);
    }
    // Unlock on normal return and on exceptions.
    ~MutexGuard() {
        OSUnlockMutex(mutex);
    }
};

extern "C" void IntrusiveList_UnlinkNodeAndDecrementCount(void*, void*);
extern "C" void MEMFreeToUnitHeap(void*, void*);
extern "C" void OSWakeupThread(void*);

struct TaskLink {
    TaskLink* next;
    TaskLink* prev;
    void* owner;
};
struct WorkList {
    int count;
    TaskLink node;
};
// Work-item links follow its vtable; the owner identifies cancellation groups.
struct WorkItem {
    virtual void first();
    virtual void run();
    virtual void cancel();
    TaskLink* next;
    TaskLink* previous;
    void* owner;
};
struct WorkIterator {
    TaskLink* value;
    // Wrap a borrowed link; it never owns or releases the task.
    WorkIterator(TaskLink* p) : value(p) {}
};

// Return a work item to the unit heap and wake threads waiting for free storage.
inline void releaseWork(void* value, char* self) {
    InterruptGuard guard;
    MEMFreeToUnitHeap(*(void**)(self + 0x48), value);
    OSWakeupThread(self + 0x18);
}

// Remove the first task from one priority list under interrupt protection.
inline void* popWork(WorkList* list) {
    WorkIterator node(0);
    {
        InterruptGuard guard;
        if (!list->count)
            return 0;
        node.value = list->node.next;
        TaskLink* position = node.value;
        IntrusiveList_UnlinkNodeAndDecrementCount(list, &position);
    }
    return (char*)node.value - 4;
}

// Run one task from the highest nonempty priority list and recycle its storage.
// Returns 0 if all queues are empty, 1 after running a task. The mutex remains
// held through run(); activeTask (+0x20) is published for cancellation observers.
extern "C" int SoundStreamTaskQueue_RunNext(void* p) {
    char* self = (char*)p;
    MutexGuard mutex(self);
    if (!(*(void**)(self + 0x20) = popWork((WorkList*)(self + 0x3C)))) {
        if (!(*(void**)(self + 0x20) = popWork((WorkList*)(self + 0x30)))) {
            if (!(*(void**)(self + 0x20) = popWork((WorkList*)(self + 0x24))))
                return 0;
        }
    }
    WorkItem* value = *(WorkItem**)(self + 0x20);
    value->run();
    releaseWork(*(void**)(self + 0x20), self);
    *(void**)(self + 0x20) = 0;
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Remove queued tasks for an owner and request cancellation of its active task.
// Does not acquire the manager mutex here: callers must coordinate active-task
// lifetime. Queued storage is freed directly; only the active task gets cancel().
extern "C" void AudioWorkQueue_CancelOwnerTasks(char* self, void* owner) {
    TaskLink* current;
    {
        InterruptGuard guard;
        for (int i = 0; i < 3; ++i) {
            WorkList* list = (WorkList*)(self + 0x24 + i * 12);
            WorkIterator node(list->node.next);
            TaskLink* end = &list->node;
            while (node.value != end) {
                current = node.value;
                node.value = node.value->next;
                if (current->owner == owner) {
                    TaskLink* position = current;
                    IntrusiveList_UnlinkNodeAndDecrementCount(list, &position);
                    InterruptGuard release;
                    MEMFreeToUnitHeap(*(void**)(self + 0x48), (char*)current - 4);
                    OSWakeupThread(self + 0x18);
                }
            }
        }
    }
    WorkItem* value = *(WorkItem**)(self + 0x20);
    if (value && value->owner == owner)
        value->cancel();
}

// Remove all queued tasks and request cancellation of the currently running task.
extern "C" void gtl_AudioWorkQueue_CancelAll(char* self) {
    TaskLink* current;
    MutexGuard mutex(self);
    {
        InterruptGuard guard;
        for (int i = 0; i < 3; ++i) {
            WorkList* list = (WorkList*)(self + 0x24 + i * 12);
            WorkIterator node(list->node.next);
            TaskLink* end = &list->node;
            while (node.value != end) {
                current = node.value;
                node.value = node.value->next;
                TaskLink* position = current;
                IntrusiveList_UnlinkNodeAndDecrementCount(list, &position);
                InterruptGuard release;
                MEMFreeToUnitHeap(*(void**)(self + 0x48), (char*)current - 4);
                OSWakeupThread(self + 0x18);
            }
        }
    }
    WorkItem* value = *(WorkItem**)(self + 0x20);
    if (value)
        value->cancel();
}
#pragma pop

// Append a task to the selected priority list under interrupt protection.
// index must be 0..2. Ownership of a unit-heap task transfers to the queue.
extern "C" void SoundStreamTaskQueue_Enqueue(char* self, char* value, int index) {
    InterruptGuard guard;
    char* list = self + 0x24 + index * 12;
    void* node = list + 4;
    gtl_IntrusiveList_InsertBefore(list, &node, value + 4);
}

// Code matches; exception cleanup symbols still need the original destructor names.
// Wait for a unit-heap block, sleeping until released work makes storage available.
// Returns owned raw task storage; RunNext/cancellation returns it to this heap.
extern "C" void* SoundStreamTaskQueue_AllocateTaskBlocking(char* self) {
    InterruptGuard guard;
    void* result = MEMAllocFromUnitHeap(*(void**)(self + 0x48));
    while (!result) {
        OSSleepThread((OSThreadQueue*)(self + 0x18));
        result = MEMAllocFromUnitHeap(*(void**)(self + 0x48));
    }
    return result;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626F4", _ctors_u8040E9BC_0, AudioStreamPlayers_InitializeRegistry);
STATIC_INIT_ENTRY(".CRT$XCU804626F4", _ctors_u8040E9BC_1, AudioStream_InitializeFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU804626F4", _ctors_u8040E9BC_2, GtlVoice_InitializeFullTurnConstant);
#pragma pop
