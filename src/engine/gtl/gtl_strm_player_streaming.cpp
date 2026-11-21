#include "classes/StreamWorker8040C2C8.h"
#include "classes/GtlBasicPlayer.h"
// Translation unit 0x8040D400-0x8040E908 (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#pragma use_lmw_stmw on

extern "C" {

enum StreamConstants {
    kSeekSamples = 0, kSeekMilliseconds = 1,
    kMillisecondsPerSecond = 1000,
    kChannelRecordBytes = 0x38,
    kMaxVoiceChannels = 2,
    kMaxBlockBytes = 0x8000,
    kMinBufferedBlocks = 4,
    kMaxBufferedBlocks = 32,
    kReadAlignment = 32,
    kVoicePriority = 255
};
enum StreamFlagIndex {
    kPrepared = 0, kRegistered = 1, kInitialReadsComplete = 2, kReadFailed = 3,
    kPauseRequested = 4, kDriveError = 5, kVoicePaused = 6, kWaitingForReads = 7,
    kWholeFileBuffered = 8, kLoopBoundaryPending = 9, kLoopContextPending = 10,
    kFinalRangeSet = 11, kAllReadsQueued = 12
};
// Address-only pool view: each allocation has the configured byte size.
struct StreamBufferPoolView { char header[8]; unsigned int blockBytes; };
struct Reader;


struct Node {
    Node* next;
    Node* prev;
};

struct Object {
    virtual void unused0();
    virtual void unused1();
    virtual void update();
    char padding04[0x70];
    Node node;
};

struct List {
    unsigned int count;
    Node head;
};

extern List s_activeStreamPlayers;
void gtl_nw4r_detail_StrmPlayer_UpdateStreaming(Object*);
struct State;
void gtl_AudioObject_UpdateVoiceParameters(State*);

// Advance streaming for each registered player, saving the next link before a callback can unlink it.
void gtl_AudioObjects_ProcessAudioFrame() {
    Node* node = s_activeStreamPlayers.head.next;
    Node* end = &s_activeStreamPlayers.head;
    while (node != end) {
        Node* current = node;
        node = node->next;
        gtl_nw4r_detail_StrmPlayer_UpdateStreaming((Object*)((char*)current - 0x74));
    }
}

// Apply pending voice parameters to each registered stream player.
void gtl_AudioObjects_ProcessWorkerMessage() {
    Node* node = s_activeStreamPlayers.head.next;
    Node* end = &s_activeStreamPlayers.head;
    while (node != end) {
        Node* current = node;
        node = node->next;
        gtl_AudioObject_UpdateVoiceParameters((State*)((char*)current - 0x74));
    }
}

// Dispatch each registered audio object's virtual update.
void gtl_Audio_UpdateRegisteredObjects() {
    Node* node = s_activeStreamPlayers.head.next;
    Node* end = &s_activeStreamPlayers.head;
    while (node != end) {
        Node* current = node;
        node = node->next;
        ((Object*)((char*)current - 0x74))->update();
    }
}

struct State {
    char padding00[8];
    float volumeScale, pitchScale;
    float pan, surroundPan, panOffset, surroundPanOffset;
    float lowPassFilterOffset;
    int unknown24;
    int outputLineFlags;
    float mainOutputVolume, mainSend;
    char padding34[0x3c];
    unsigned char biquadFilter;
    char padding71[3];
    Node activePlayerNode;
    unsigned char sampleFormat;
    unsigned char loopEnabled;
    unsigned char sourceChannelCount;
    char padding7F;
    unsigned short sampleRate;
    unsigned short blockHeaderBytes;
    int loopStartSample;
    unsigned int totalSamples;
    int fileDataOffset;
    int fileBlockCount;
    int blockBytes;
    int samplesPerBlock;
    int lastBlockBytes;
    unsigned int lastBlockSamples;
    int lastBlockAlignedBytes;
    char paddingA8[4];
    unsigned char flags[13];
    char paddingB9[3];
    int loopCount;
    int preloadedBlocks;
    int initialReadBlocks;
    int bufferBlockBytes;
    int bufferBlockCapacity;
    int readAheadBlocks;
    int nextCycleBlocks;
    int nextReadBufferBlock;
    int nextReadFileBlock;
    int currentCycleBlocks;
    int playingBufferBlock;
    int playingFileBlock;
    int loopStartBlock;
    int lastFileBlock;
    int seekUnit;
    int seekPosition;
    List freeReadPackets;
    List pendingReadPackets;
    char padding114[0x380];
    void* bufferPool;
    Reader* reader;
    int readerUserData;
    void* voice;
    int voiceChannels;
    void* voiceUserData;
    void* channelBuffer;
    int channelBufferBytes;
    char channelDecoderState[0x28];
    unsigned short channelLoopContext[4];
    int secondChannelBuffer;
    int secondChannelBufferBytes;
    char secondChannelDecoderState[0x30];
    unsigned short pendingLoopPredictorScale[2];
};

void gtl_nw4r_detail_BasicPlayer_ResetParameters(State*);
void GtlVoice_Release(void*);
void BitmapBlockPool_Free(void*, void*);
void* BitmapBlockPool_Allocate(void*);
void* gtl_VoicePool_GetInstance();
long long __div2i(long long, long long);
void GtlStrmPlayer_OnVoiceTerminalEvent(void*, int, State*);
void* gtl_VoicePool_AcquireVoice(void*, int, int, int, void (*)(void*, int, State*), State*);

// Convert the seek position to samples, wrap looping streams, and return block and in-block offsets.
int gtl_StrmPlayer_ResolveStartBlock(State* self, int* blockIndex, unsigned int* sampleOffset, int* completedLoops) {
    unsigned int samplePosition;
    if (!self->samplesPerBlock) return 0;
    if (self->seekUnit == kSeekSamples) {
        samplePosition = self->seekPosition;
    } else if (self->seekUnit == kSeekMilliseconds) {
        samplePosition = __div2i((long long)self->seekPosition * self->sampleRate, kMillisecondsPerSecond);
    }
    *completedLoops = 0;
    if (samplePosition >= self->totalSamples) {
        if (self->loopEnabled) {
            int loopStart = self->loopStartSample;
            int pastEndSamples = samplePosition - self->totalSamples;
            int loopSamples = self->totalSamples - loopStart;
            *completedLoops = pastEndSamples / loopSamples + 1;
            samplePosition = loopStart + pastEndSamples % loopSamples;
        } else {
            return 0;
        }
    }
    *blockIndex = (int)samplePosition / self->samplesPerBlock;
    if (!self->sampleFormat) *sampleOffset = samplePosition % self->samplesPerBlock;
    return 1;
}

// Allocate channel buffers and a voice group; release all allocated buffers on failure.
int gtl_nw4r_detail_StrmPlayer_AllocateChannelBuffersAndVoices(State* self, int count, int arg) {
    int i = 0;
    State* current = self;
    for (; i < count; ++i, current = (State*)((char*)current + kChannelRecordBytes)) {
        void* entry = BitmapBlockPool_Allocate(self->bufferPool);
        if (!entry) {
            current = self;
            for (arg = 0; arg < i; current = (State*)((char*)current + kChannelRecordBytes), ++arg)
                BitmapBlockPool_Free(self->bufferPool, current->channelBuffer);
            return 0;
        }
        current->channelBuffer = entry;
        current->channelBufferBytes = ((StreamBufferPoolView*)self->bufferPool)->blockBytes;
    }
    void* result = gtl_VoicePool_AcquireVoice(gtl_VoicePool_GetInstance(), count, arg, kVoicePriority, GtlStrmPlayer_OnVoiceTerminalEvent, self);
    if (!result) {
        current = self;
        for (arg = 0; arg < count; current = (State*)((char*)current + kChannelRecordBytes), ++arg)
            BitmapBlockPool_Free(self->bufferPool, current->channelBuffer);
        return 0;
    }
    self->voice = result;
    return 1;
}

// Reset playback flags and buffer ownership after resetting the base player parameters.
void gtl_nw4r_detail_StrmPlayer_ResetState(State* self, void* arg) {
    gtl_nw4r_detail_BasicPlayer_ResetParameters(self);
    self->voiceUserData = arg;
    self->flags[kPrepared] = 0;
    self->flags[kRegistered] = 0;
    self->flags[kInitialReadsComplete] = 0;
    self->flags[kReadFailed] = 0;
    self->flags[kAllReadsQueued] = 0;
    self->flags[kPauseRequested] = 0;
    self->flags[kDriveError] = 0;
    self->flags[kVoicePaused] = 0;
    self->flags[kWaitingForReads] = 0;
    self->flags[kWholeFileBuffered] = 0;
    self->flags[kFinalRangeSet] = 0;
    self->flags[kLoopBoundaryPending] = 0;
    self->flags[kLoopContextPending] = 0;
    self->voiceChannels = 0;
    self->loopCount = 0;
    self->channelBuffer = 0;
    self->channelBufferBytes = 0;
    self->secondChannelBuffer = 0;
    self->secondChannelBufferBytes = 0;
}

// The original cleanup entries name LockGuardBase_dtor and InterruptGuard_dtor.
struct InterruptBase {
    InterruptBase() {}
    ~InterruptBase() {}
};

struct InterruptLock : InterruptBase {
    BOOL level;
    InterruptLock() { level = OSDisableInterrupts(); }
    ~InterruptLock() { OSRestoreInterrupts(level); }
};

void StrmPlayer_SetPauseFlag(void*, unsigned int);
unsigned int GtlVoiceGroup_GetSamplePosition(void*);
void GtlStrmPlayer_AdvanceStreamBlock(State*);
void gtl_nw4r_detail_StrmPlayer_RequestNextBlock(State*, int);

// Pause the voice while explicitly paused or waiting for streaming reads.
inline void updatePause(State* self) {
    InterruptLock lock;
    unsigned int paused = 0;
    if (self->flags[kPauseRequested]) paused = 1;
    if (self->flags[kWaitingForReads]) paused = 1;
    if (paused != self->flags[kVoicePaused]) {
        if (self->voice) StrmPlayer_SetPauseFlag(self->voice, paused);
        self->flags[kVoicePaused] = paused;
    }
}

// Track playback progress and queue replacement blocks, pausing on drive errors or a full read backlog.
void gtl_nw4r_detail_StrmPlayer_UpdateStreaming(Object* object) {
    State* self = (State*)object;
    if (self->flags[kRegistered] && self->voice) {
        int status = DVDGetDriveStatus();
        if (status == 0) {
            self->flags[kDriveError] = 0;
            updatePause(self);
        } else if (status != 1) {
            self->flags[kDriveError] = 1;
            self->flags[kWaitingForReads] = 1;
            updatePause(self);
        }
        if (self->flags[kWaitingForReads] && !self->pendingReadPackets.count) {
            self->flags[kWaitingForReads] = 0;
            updatePause(self);
        }
        if (!self->flags[kFinalRangeSet] && !self->flags[kWholeFileBuffered] && !self->flags[kWaitingForReads]) {
            int index = GtlVoiceGroup_GetSamplePosition(self->voice) / (unsigned int)self->samplesPerBlock;
            while (self->playingBufferBlock != index) {
                if (!self->flags[kWaitingForReads] && self->pendingReadPackets.count >= (unsigned int)self->readAheadBlocks - 2) {
                    self->flags[kWaitingForReads] = 1;
                    updatePause(self);
                    break;
                }
                GtlStrmPlayer_AdvanceStreamBlock(self);
                gtl_nw4r_detail_StrmPlayer_RequestNextBlock(self, 1);
            }
        }
    }
}

// Return channel buffers to their pool, then release the voice with interrupts disabled.
void gtl_nw4r_detail_StrmPlayer_ReleaseChannelBuffersAndVoice(State* self) {
    State* current = self;
    int i = 0;
    for (; i < self->voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i) {
        if (current->channelBuffer) {
            BitmapBlockPool_Free(self->bufferPool, current->channelBuffer);
            current->channelBuffer = 0;
            current->channelBufferBytes = 0;
        }
    }
    self->voiceChannels = 0;
    InterruptLock lock;
    if (self->voice) {
        GtlVoice_Release(self->voice);
        self->voice = 0;
    }
}

void* gtl_AudioWorker_GetInstance();
void GtlVoice_Stop(void*);
void IntrusiveList_UnlinkNode(List*, Node*);
void IntrusiveList_UnlinkNodeAndDecrementCount(List*, Node**);
void gtl_IntrusiveList_InsertBefore(List*, Node**, Node*);

struct MutexBase {
    ~MutexBase() {}
};
struct MutexLock : MutexBase {
    MutexLock() { OSLockMutex((OSMutex*)((StreamWorker8040C2C8*)gtl_AudioWorker_GetInstance())->mutex); }
    ~MutexLock() { OSUnlockMutex((OSMutex*)((StreamWorker8040C2C8*)gtl_AudioWorker_GetInstance())->mutex); }
};

float GtlBasicPlayer_GetAuxiliarySend(State*, int); // Read an auxiliary-bus send level.
float GtlBasicPlayer_GetRemoteVolume(State*, int); // Read a Wii Remote output volume.
float GtlBasicPlayer_GetRemoteSendOffset(State*, int); // Read a Wii Remote send offset.
float GtlBasicPlayer_GetAdditionalOutputMixControl(State*, int); // Read an additional output-mix control.
void gtl_nw4r_detail_Voice_SetVolumeScale(void*, float);
void GtlVoice_SetPitchScale(void*, float);
void GtlVoice_SetPan(void*, float);
void GtlVoice_SetSurroundPan(void*, float);
void GtlVoice_SetPanOffset(void*, float);
void GtlVoice_SetSurroundPanOffset(void*, float);
void GtlVoice_SetLowPassFilter(void*, float);
void GtlVoice_SetBiquadFilterSelector(void*, unsigned char);
void GtlVoice_SetOutputLineMask(void*, int);
void GtlVoice_SetMainOutputVolume(void*, float);
void GtlVoice_SetMainSendOffset(void*, float);
void GtlVoice_SetAuxSend(void*, int, float);
void GtlVoice_SetRemoteVolume(void*, int, float);
void GtlVoice_SetRemoteSendOffset(void*, int, float);
void GtlVoice_SetAdditionalOutputMixControl(void*, int, float);
void gtl_nw4r_detail_StrmPlayer_ReleaseResources(State*);

// Send scalar, auxiliary and per-output parameters to the voice, or release a failed stream.
void gtl_AudioObject_UpdateVoiceParameters(State* self) {
    if (self->flags[kRegistered]) {
        MutexLock mutex;
        if (self->flags[kReadFailed]) {
            gtl_nw4r_detail_StrmPlayer_ReleaseResources(self);
            return;
        }
        if (!self->voice) {
            gtl_nw4r_detail_StrmPlayer_ReleaseResources(self);
            return;
        }
        float volumeScale = 1.0f;
        volumeScale *= AsGtlBasicPlayer(self)->volumeScale;
        float pitchScale = 1.0f;
        pitchScale *= AsGtlBasicPlayer(self)->pitchScale;
        float pan = 0.0f;
        pan += self->pan;
        float surroundPan = 0.0f;
        surroundPan += self->surroundPan;
        float panOffset = 0.0f;
        panOffset += self->panOffset;
        float surroundPanOffset = 0.0f;
        surroundPanOffset += self->surroundPanOffset;
        float lowPassFilter = 1.0f;
        lowPassFilter += self->lowPassFilterOffset;
        float mainSendOffset = 0.0f;
        mainSendOffset += AsGtlBasicPlayer(self)->mainSend;
        unsigned char biquadFilter = self->biquadFilter;
        float auxiliarySends[3];
        float remoteVolumes[4];
        float remoteSendOffsets[4];
        float additionalOutputMix[4];
        for (int i = 0; i < 3; ++i) {
            auxiliarySends[i] = 0.0f;
            auxiliarySends[i] += GtlBasicPlayer_GetAuxiliarySend(self, i);
        }
        for (int i = 0; i < 4; ++i) {
            remoteVolumes[i] = GtlBasicPlayer_GetRemoteVolume(self, i);
            remoteSendOffsets[i] = 0.0f;
            remoteSendOffsets[i] += GtlBasicPlayer_GetRemoteSendOffset(self, i);
            additionalOutputMix[i] = 0.0f;
            additionalOutputMix[i] += GtlBasicPlayer_GetAdditionalOutputMixControl(self, i);
        }
        gtl_nw4r_detail_Voice_SetVolumeScale(self->voice, volumeScale);
        GtlVoice_SetPitchScale(self->voice, pitchScale);
        GtlVoice_SetPan(self->voice, pan);
        GtlVoice_SetSurroundPan(self->voice, surroundPan);
        GtlVoice_SetPanOffset(self->voice, panOffset);
        GtlVoice_SetSurroundPanOffset(self->voice, surroundPanOffset);
        GtlVoice_SetLowPassFilter(self->voice, lowPassFilter);
        GtlVoice_SetBiquadFilterSelector(self->voice, biquadFilter);
        GtlVoice_SetOutputLineMask(self->voice, AsGtlBasicPlayer(self)->outputLineFlags);
        float mainOutputVolume = self->mainOutputVolume;
        GtlVoice_SetMainOutputVolume(self->voice, mainOutputVolume);
        GtlVoice_SetMainSendOffset(self->voice, mainSendOffset);
        for (int i = 0; i < 3; ++i)
            GtlVoice_SetAuxSend(self->voice, i, auxiliarySends[i]);
        for (int i = 0; i < 4; ++i) {
            GtlVoice_SetRemoteVolume(self->voice, i, remoteVolumes[i]);
            GtlVoice_SetRemoteSendOffset(self->voice, i, remoteSendOffsets[i]);
            GtlVoice_SetAdditionalOutputMixControl(self->voice, i, additionalOutputMix[i]);
        }
    }
}

void gtl_Voice_SetChannelLoopSampleAddress(void*, int, void*, unsigned int);
void GtlVoiceGroup_SetRowEndSampleAddress(void*, int, void*, unsigned int);
void GtlVoiceGroup_SetLoopFlag(void*, int);
int GtlVoiceGroup_GetFirstSampleFormat(void*);
void GtlVoiceGroup_SetHardwareVoiceType(void*, int);
void GtlVoice_SetRowStateTriplet(void*, int, void*);
void GtlVoiceGroup_SetRowNonLoopingSampleRange(void*, int, void*, unsigned int);
void gtl_nw4r_detail_StrmPlayer_RequestNextBlock(State*, int);
void gtl_nw4r_detail_StrmPlayer_ReleaseResources(State*);

// Resolve the seek position and configure bounded circular buffering before acquiring channels.
int gtl_StrmPlayer_SetupStreamingBuffers(State* self, void*) {
    unsigned int size = ((StreamBufferPoolView*)self->bufferPool)->blockBytes;
    int index = 0;
    unsigned int fraction = 0;
    int loops = 0;
    if (!gtl_StrmPlayer_ResolveStartBlock(self, &index, &fraction, &loops)) return 0;
    int chunk = self->blockBytes;
    int endIndex = self->fileBlockCount - 1;
    unsigned int start = (unsigned int)self->loopStartSample / (unsigned int)self->samplesPerBlock;
    self->bufferBlockBytes = chunk;
    self->lastFileBlock = endIndex;
    self->loopStartBlock = start;
    if (self->bufferBlockBytes > kMaxBlockBytes) return 0;
    self->bufferBlockCapacity = size / (unsigned int)self->bufferBlockBytes;
    if (self->bufferBlockCapacity < kMinBufferedBlocks) return 0;
    if (self->bufferBlockCapacity > kMaxBufferedBlocks) self->bufferBlockCapacity = kMaxBufferedBlocks;
    int count = self->bufferBlockCapacity - 1;
    self->readAheadBlocks = count;
    self->initialReadBlocks = count;
    self->playingFileBlock = index;
    int currentIndex = index;
    self->nextReadFileBlock = currentIndex;
    self->nextReadBufferBlock = 0;
    self->playingBufferBlock = 0;
    if (self->flags[kWholeFileBuffered]) {
        self->nextCycleBlocks = self->fileBlockCount;
    } else {
        int end = self->lastFileBlock;
        int next = count + 1;
        int remaining = end - currentIndex + 1;
        int length = end - self->loopStartBlock + 1;
        if ((next - remaining) % length == 0) count = next;
        self->nextCycleBlocks = count;
    }
    self->currentCycleBlocks = self->nextCycleBlocks;
    InterruptLock lock;
    int channels = kMaxVoiceChannels;
    if ((int)self->sourceChannelCount <= kMaxVoiceChannels) channels = self->sourceChannelCount;
    self->voiceChannels = channels;
    if (!gtl_nw4r_detail_StrmPlayer_AllocateChannelBuffersAndVoices(self, channels, (int)self->voiceUserData)) return 0;
    return 1;
}

// Queue the initial read-ahead blocks and configure the final range for short non-looping streams.
void gtl_nw4r_detail_StrmPlayer_PreloadBlocks(State* self, void* arg) {
    int channel;
    unsigned int last;
    State* current;
    int i;
    MutexLock mutex;
    if (!gtl_StrmPlayer_SetupStreamingBuffers(self, arg)) {
        gtl_nw4r_detail_StrmPlayer_ReleaseResources(self);
        return;
    }
    self->preloadedBlocks = 0;
    for (i = 0; i < self->readAheadBlocks; ++i) {
        gtl_nw4r_detail_StrmPlayer_RequestNextBlock(self, 0);
        ++self->preloadedBlocks;
        if (self->flags[kAllReadsQueued]) break;
    }
    if ((unsigned int)self->fileBlockCount <= 2 && !self->loopEnabled) {
        last = self->fileBlockCount - 1;
        {
            InterruptLock lock;
            channel = 0;
            current = self;
            for (; channel < self->voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++channel)
                GtlVoiceGroup_SetRowNonLoopingSampleRange(self->voice, channel, current->channelBuffer, self->lastBlockSamples + last * self->samplesPerBlock);
        }
        self->flags[kFinalRangeSet] = 1;
    }
}

// Set voice loop/end addresses at a file boundary, including the decoder loop context.
void GtlStrmPlayer_ConfigureBlockBoundaryAddresses(State* self, int index) {
    unsigned int offset;
    unsigned int address;
    int i;
    State* current;
    int next;
    if (self->loopEnabled) {
        next = index + 1;
        if (next >= self->currentCycleBlocks) next -= self->currentCycleBlocks;
        InterruptLock outer;
        address = self->lastBlockSamples + index * self->samplesPerBlock;
        offset = (unsigned int)self->loopStartSample % (unsigned int)self->samplesPerBlock + next * self->samplesPerBlock;
        {
            InterruptLock inner;
            i = 0;
            current = self;
            for (; i < self->voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i) {
                gtl_Voice_SetChannelLoopSampleAddress(self->voice, i, current->channelBuffer, offset);
                GtlVoiceGroup_SetRowEndSampleAddress(self->voice, i, current->channelBuffer, address);
            }
            GtlVoiceGroup_SetLoopFlag(self->voice, 1);
        }
        if (!self->sampleFormat) {
            if (!GtlVoiceGroup_GetFirstSampleFormat(self->voice)) {
                GtlVoiceGroup_SetHardwareVoiceType(self->voice, 0);
                int i = 0;
                State* current = (State*)self->channelLoopContext;
                for (; i < self->voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i)
                    GtlVoice_SetRowStateTriplet(self->voice, i, current);
            }
            if (index == self->currentCycleBlocks - 1) self->flags[kLoopBoundaryPending] = 1;
        }
    } else {
        {
            InterruptLock lock;
            i = 0;
            current = self;
            for (; i < self->voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i)
                GtlVoiceGroup_SetRowNonLoopingSampleRange(self->voice, i, current->channelBuffer, self->lastBlockSamples + index * self->samplesPerBlock);
        }
        self->flags[kFinalRangeSet] = 1;
    }
}

// Advance file and circular-buffer playback indices and update loop ranges at either boundary.
void GtlStrmPlayer_AdvanceStreamBlock(State* self) {
    int size;
    if (++self->playingFileBlock > self->lastFileBlock && self->loopEnabled) {
        self->playingFileBlock = self->loopStartBlock;
        if (self->loopCount < 0x7fffffff) ++self->loopCount;
        size = self->currentCycleBlocks * self->samplesPerBlock;
        InterruptLock lock;
        int i = 0;
        State* current = self;
        for (; i < self->voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i) {
            gtl_Voice_SetChannelLoopSampleAddress(self->voice, i, current->channelBuffer, 0);
            GtlVoiceGroup_SetRowEndSampleAddress(self->voice, i, current->channelBuffer, size);
        }
        GtlVoiceGroup_SetLoopFlag(self->voice, 1);
    }
    if (++self->playingBufferBlock >= self->currentCycleBlocks) {
        int chunk = self->nextCycleBlocks;
        self->playingBufferBlock = 0;
        size = chunk * self->samplesPerBlock;
        self->currentCycleBlocks = chunk;
        InterruptLock lock;
        int i = 0;
        State* current = self;
        for (; i < self->voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i) {
            gtl_Voice_SetChannelLoopSampleAddress(self->voice, i, current->channelBuffer, 0);
            GtlVoiceGroup_SetRowEndSampleAddress(self->voice, i, current->channelBuffer, size);
        }
        GtlVoiceGroup_SetLoopFlag(self->voice, 1);
    }
    if (self->playingBufferBlock == self->currentCycleBlocks - 1 && !GtlVoiceGroup_GetFirstSampleFormat(self->voice)) {
        if (!self->flags[kLoopBoundaryPending] && self->flags[kLoopContextPending]) {
            InterruptLock lock;
            int i = 0;
            // Shift the byte base to address consecutive two-byte predictor scales;
            // these positions are not separate State objects or channel records.
            char* channelByteCursor = (char*)self;
            for (; i < self->voiceChannels; channelByteCursor += 2, ++i) {
                unsigned short params[3];
                params[0] = *(unsigned short*)(channelByteCursor + (int)&((State*)0)->pendingLoopPredictorScale);
                params[1] = 0;
                params[2] = 0;
                GtlVoice_SetRowStateTriplet(self->voice, i, params);
            }
            GtlVoiceGroup_SetHardwareVoiceType(self->voice, 1);
        }
        self->flags[kLoopContextPending] = 0;
        self->flags[kLoopBoundaryPending] = 0;
    }
    if (self->playingFileBlock == self->lastFileBlock - 1) GtlStrmPlayer_ConfigureBlockBoundaryAddresses(self, self->playingBufferBlock + 1);
}

struct ReadPacket {
    virtual void finish(int);
    int unused4;
    int asynchronous;
    int index;
    int chunk;
    Node node;
};
struct Reader {
    virtual void unused0();
    virtual void unused1();
    virtual int read(void*, unsigned int, int, int, int, int, int, ReadPacket*, State*, int);
};
extern char s_streamReadScratchBuffer[];
// Test whether an intrusive packet list has any entries.
inline bool listEmpty(List* list) { return list->count == 0; }
// Append a packet link before the list sentinel.
inline void packetInsert(List* list, Node* node) {
    Node* anchor = &list->head;
    gtl_IntrusiveList_InsertBefore(list, &anchor, node);
}

// Take a free read packet, submit one aligned file block, and advance file and buffer write indices.
void gtl_nw4r_detail_StrmPlayer_RequestNextBlock(State* self, int asynchronous) {
    if (self->flags[kAllReadsQueued]) return;
    ReadPacket* packet;
    {
        InterruptLock lock;
        if (listEmpty(&self->freeReadPackets) & 1) return;
        Node* node = self->freeReadPackets.head.next;
        packet = (ReadPacket*)((char*)node - 0x14);
        IntrusiveList_UnlinkNodeAndDecrementCount(&self->freeReadPackets, &node);
    }
    packet->asynchronous = asynchronous;
    packet->chunk = self->nextReadBufferBlock;
    packet->index = self->nextReadFileBlock;
    int readBlockBytes;
    if (*(volatile int*)&self->nextReadFileBlock < self->lastFileBlock) readBlockBytes = self->blockBytes;
    else readBlockBytes = self->lastBlockAlignedBytes;
    unsigned int alignedReadBytes = (self->blockHeaderBytes + readBlockBytes * self->voiceChannels + (kReadAlignment - 1)) & ~(kReadAlignment - 1);
    int fileOffset = self->fileDataOffset + self->nextReadFileBlock * (self->blockHeaderBytes + self->blockBytes * self->sourceChannelCount);
    packetInsert(&self->pendingReadPackets, &packet->node);
    int initializeDecoder = 0;
    if (!self->nextReadBufferBlock && !self->sampleFormat) initializeDecoder = 1;
    int result = self->reader->read(s_streamReadScratchBuffer, alignedReadBytes, fileOffset, self->voiceChannels, readBlockBytes, self->blockHeaderBytes, initializeDecoder, packet, self, self->readerUserData);
    switch (result) {
    case -1:
        break;
    case 2:
        break;
    case 3:
        break;
    case 1:
        self->flags[kReadFailed] = 1;
        return;
    case 0:
        packet->finish(1);
        break;
    }
    if (++self->nextReadFileBlock > self->lastFileBlock) {
        if (self->loopEnabled) self->nextReadFileBlock = self->loopStartBlock;
        else {
            self->flags[kAllReadsQueued] = 1;
            return;
        }
    }
    if (++self->nextReadBufferBlock >= self->nextCycleBlocks) {
        int remaining = self->nextReadFileBlock;
        remaining = self->lastFileBlock - remaining;
        int span = self->lastFileBlock - self->loopStartBlock;
        self->nextReadBufferBlock = 0;
        int count = self->readAheadBlocks;
        int next = count + 1;
        remaining += 1;
        span += 1;
        if ((next - remaining) % span == 0) count = next;
        self->nextCycleBlocks = count;
    }
}
struct Callback {
    virtual void unused0();
    virtual void unused1();
    virtual void unused2();
    virtual void release(State*, int);
};

// Stop/unregister playback, return pending packets to the free list, and release buffers and voice.
void gtl_nw4r_detail_StrmPlayer_ReleaseResources(State* self) {
    MutexLock mutex;
    if (self->voice) GtlVoice_Stop(self->voice);
    if (self->flags[kRegistered]) {
        IntrusiveList_UnlinkNode(&s_activeStreamPlayers, &self->activePlayerNode);
        self->flags[kRegistered] = 0;
        self->flags[kInitialReadsComplete] = 0;
    }
    ((Callback*)self->reader)->release(self, self->readerUserData);
    {
        InterruptLock lock;
        if (self->pendingReadPackets.count) {
            Node* end = &self->freeReadPackets.head;
            while (self->pendingReadPackets.count) {
                Node* node = self->pendingReadPackets.head.next;
                Node* position = node;
                IntrusiveList_UnlinkNodeAndDecrementCount(&self->pendingReadPackets, &position);
                Node* anchor = end;
                gtl_IntrusiveList_InsertBefore(&self->freeReadPackets, &anchor, node);
            }
        }
    }
    gtl_nw4r_detail_StrmPlayer_ReleaseChannelBuffersAndVoice(self);
    self->flags[kPrepared] = 0;
}

// Clear the player's voice on every terminal callback; release it explicitly for callback reasons 0 and 1.
void GtlStrmPlayer_OnVoiceTerminalEvent(void* arg, int mode, State* self) {
    switch (mode) {
    case 0:
    case 1:
        GtlVoice_Release(arg);
        self->voice = 0;
        break;
    case 2:
    case 3:
        self->voice = 0;
        break;
    }
}

} // extern "C"
