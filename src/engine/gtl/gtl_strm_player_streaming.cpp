// Streaming for the stream player (gtl::nw4r::detail::StrmPlayer): finding the start block, setting up
// buffers and the voice, requesting file blocks each update, and releasing buffers and voices.

#include "classes/Voice8040E9BC.h"
#include "classes/StreamWorker8040C2C8.h"
#include "classes/GtlBasicPlayer.h"
#include "classes/GtlStrmPlayer.h"

#include "sdk_protos.h"
#pragma use_lmw_stmw on

extern "C" {

enum StreamConstants {
    kSeekSamples = 0,
    kSeekMilliseconds = 1,
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
    kPrepared = 0,
    kRegistered = 1,
    kInitialReadsComplete = 2,
    kReadFailed = 3,
    kPauseRequested = 4,
    kDriveError = 5,
    kVoicePaused = 6,
    kWaitingForReads = 7,
    kWholeFileBuffered = 8,
    kLoopBoundaryPending = 9,
    kLoopContextPending = 10,
    kFinalRangeSet = 11,
    kAllReadsQueued = 12
};
// Address-only pool view: each allocation has the configured byte size.
struct StreamBufferPoolView {
    char header[8];
    unsigned int blockBytes;
};
struct Reader;

struct Object {
    virtual void unused0();
    virtual void unused1();
    virtual void update();
    char padding04[0x70];
    Node node;
};

extern List s_activeStreamPlayers;
void gtl_nw4r_detail_StrmPlayer_UpdateStreaming(Object*);
typedef gtl::nw4r::detail::StrmPlayer State;
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

void gtl_nw4r_detail_BasicPlayer_ResetParameters(State*);
void GtlVoice_Release(void*);
void BitmapBlockPool_Free(void*, void*);
void* BitmapBlockPool_Allocate(void*);
void* gtl_VoicePool_GetInstance();
long long __div2i(long long, long long);
void* gtl_VoicePool_AcquireVoice(void*, int, int, int, void (*)(void*, int, State*), State*);

} // extern "C"

// Convert the seek position to samples, wrap looping streams, and return block and in-block offsets.
int gtl::nw4r::detail::StrmPlayer::ResolveStartBlock(int* blockIndex, unsigned int* sampleOffset, int* completedLoops) {
    unsigned int samplePosition;
    if (!samplesPerBlock)
        return 0;
    if (seekUnit == kSeekSamples) {
        samplePosition = seekPosition;
    } else if (seekUnit == kSeekMilliseconds) {
        samplePosition = __div2i((long long)seekPosition * sampleRate, kMillisecondsPerSecond);
    }
    *completedLoops = 0;
    if (samplePosition >= totalSamples) {
        if (loopEnabled) {
            int loopStart = loopStartSample;
            int pastEndSamples = samplePosition - totalSamples;
            int loopSamples = totalSamples - loopStart;
            *completedLoops = pastEndSamples / loopSamples + 1;
            samplePosition = loopStart + pastEndSamples % loopSamples;
        } else {
            return 0;
        }
    }
    *blockIndex = (int)samplePosition / samplesPerBlock;
    if (!sampleFormat)
        *sampleOffset = samplePosition % samplesPerBlock;
    return 1;
}

extern "C" {

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
    void* result = gtl_VoicePool_AcquireVoice(gtl_VoicePool_GetInstance(), count, arg, kVoicePriority,
                                              gtl::nw4r::detail::StrmPlayer::OnVoiceTerminalEvent, self);
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
    InterruptLock() {
        level = OSDisableInterrupts();
    }
    ~InterruptLock() {
        OSRestoreInterrupts(level);
    }
};

unsigned int GtlVoiceGroup_GetSamplePosition(void*);
void gtl_nw4r_detail_StrmPlayer_RequestNextBlock(State*, int);

// Pause the voice while explicitly paused or waiting for streaming reads.
inline void updatePause(State* self) {
    InterruptLock lock;
    unsigned int paused = 0;
    if (self->flags[kPauseRequested])
        paused = 1;
    if (self->flags[kWaitingForReads])
        paused = 1;
    if (paused != self->flags[kVoicePaused]) {
        if (self->voice)
            ((gtl::nw4r::detail::Voice*)(self->voice))->SetPauseFlag(paused);
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
                if (!self->flags[kWaitingForReads] &&
                    self->pendingReadPackets.count >= (unsigned int)self->readAheadBlocks - 2) {
                    self->flags[kWaitingForReads] = 1;
                    updatePause(self);
                    break;
                }
                self->AdvanceStreamBlock();
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
    MutexLock() {
        OSLockMutex((OSMutex*)((StreamWorker8040C2C8*)gtl_AudioWorker_GetInstance())->mutex);
    }
    ~MutexLock() {
        OSUnlockMutex((OSMutex*)((StreamWorker8040C2C8*)gtl_AudioWorker_GetInstance())->mutex);
    }
};

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
            auxiliarySends[i] += ((gtl::nw4r::detail::BasicPlayer*)self)->GetAuxiliarySend(i);
        }
        for (int i = 0; i < 4; ++i) {
            remoteVolumes[i] = ((gtl::nw4r::detail::BasicPlayer*)self)->GetRemoteVolume(i);
            remoteSendOffsets[i] = 0.0f;
            remoteSendOffsets[i] += ((gtl::nw4r::detail::BasicPlayer*)self)->GetRemoteSendOffset(i);
            additionalOutputMix[i] = 0.0f;
            additionalOutputMix[i] += ((gtl::nw4r::detail::BasicPlayer*)self)->GetAdditionalOutputMixControl(i);
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

void GtlVoiceGroup_SetRowEndSampleAddress(void*, int, void*, unsigned int);
void GtlVoiceGroup_SetLoopFlag(void*, int);
int GtlVoiceGroup_GetFirstSampleFormat(void*);
void GtlVoiceGroup_SetHardwareVoiceType(void*, int);
void GtlVoice_SetRowStateTriplet(void*, int, void*);
void GtlVoiceGroup_SetRowNonLoopingSampleRange(void*, int, void*, unsigned int);
void gtl_nw4r_detail_StrmPlayer_RequestNextBlock(State*, int);
void gtl_nw4r_detail_StrmPlayer_ReleaseResources(State*);

} // extern "C"

// Resolve the seek position and configure bounded circular buffering before acquiring channels.
int gtl::nw4r::detail::StrmPlayer::SetupStreamingBuffers(void*) {
    unsigned int size = ((StreamBufferPoolView*)bufferPool)->blockBytes;
    int index = 0;
    unsigned int fraction = 0;
    int loops = 0;
    if (!ResolveStartBlock(&index, &fraction, &loops))
        return 0;
    int chunk = blockBytes;
    int endIndex = fileBlockCount - 1;
    unsigned int start = (unsigned int)loopStartSample / (unsigned int)samplesPerBlock;
    bufferBlockBytes = chunk;
    lastFileBlock = endIndex;
    loopStartBlock = start;
    if (bufferBlockBytes > kMaxBlockBytes)
        return 0;
    bufferBlockCapacity = size / (unsigned int)bufferBlockBytes;
    if (bufferBlockCapacity < kMinBufferedBlocks)
        return 0;
    if (bufferBlockCapacity > kMaxBufferedBlocks)
        bufferBlockCapacity = kMaxBufferedBlocks;
    int count = bufferBlockCapacity - 1;
    readAheadBlocks = count;
    initialReadBlocks = count;
    playingFileBlock = index;
    int currentIndex = index;
    nextReadFileBlock = currentIndex;
    nextReadBufferBlock = 0;
    playingBufferBlock = 0;
    if (flags[kWholeFileBuffered]) {
        nextCycleBlocks = fileBlockCount;
    } else {
        int end = lastFileBlock;
        int next = count + 1;
        int remaining = end - currentIndex + 1;
        int length = end - loopStartBlock + 1;
        if ((next - remaining) % length == 0)
            count = next;
        nextCycleBlocks = count;
    }
    currentCycleBlocks = nextCycleBlocks;
    InterruptLock lock;
    int channels = kMaxVoiceChannels;
    if ((int)sourceChannelCount <= kMaxVoiceChannels)
        channels = sourceChannelCount;
    voiceChannels = channels;
    if (!gtl_nw4r_detail_StrmPlayer_AllocateChannelBuffersAndVoices(this, channels, (int)voiceUserData))
        return 0;
    return 1;
}

extern "C" {

// Queue the initial read-ahead blocks and configure the final range for short non-looping streams.
void gtl_nw4r_detail_StrmPlayer_PreloadBlocks(State* self, void* arg) {
    int channel;
    unsigned int last;
    State* current;
    int i;
    MutexLock mutex;
    if (!self->SetupStreamingBuffers(arg)) {
        gtl_nw4r_detail_StrmPlayer_ReleaseResources(self);
        return;
    }
    self->preloadedBlocks = 0;
    for (i = 0; i < self->readAheadBlocks; ++i) {
        gtl_nw4r_detail_StrmPlayer_RequestNextBlock(self, 0);
        ++self->preloadedBlocks;
        if (self->flags[kAllReadsQueued])
            break;
    }
    if ((unsigned int)self->fileBlockCount <= 2 && !self->loopEnabled) {
        last = self->fileBlockCount - 1;
        {
            InterruptLock lock;
            channel = 0;
            current = self;
            for (; channel < self->voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++channel)
                GtlVoiceGroup_SetRowNonLoopingSampleRange(self->voice, channel, current->channelBuffer,
                                                          self->lastBlockSamples + last * self->samplesPerBlock);
        }
        self->flags[kFinalRangeSet] = 1;
    }
}

} // extern "C"

// Set voice loop/end addresses at a file boundary, including the decoder loop context.
void gtl::nw4r::detail::StrmPlayer::ConfigureBlockBoundaryAddresses(int index) {
    unsigned int offset;
    unsigned int address;
    int i;
    State* current;
    int next;
    if (loopEnabled) {
        next = index + 1;
        if (next >= currentCycleBlocks)
            next -= currentCycleBlocks;
        InterruptLock outer;
        address = lastBlockSamples + index * samplesPerBlock;
        offset = (unsigned int)loopStartSample % (unsigned int)samplesPerBlock + next * samplesPerBlock;
        {
            InterruptLock inner;
            i = 0;
            current = this;
            for (; i < voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i) {
                ((gtl::nw4r::detail::Voice*)(voice))
                    ->SetChannelLoopSampleAddress(i, (int)current->channelBuffer, offset);
                GtlVoiceGroup_SetRowEndSampleAddress(voice, i, current->channelBuffer, address);
            }
            GtlVoiceGroup_SetLoopFlag(voice, 1);
        }
        if (!sampleFormat) {
            if (!GtlVoiceGroup_GetFirstSampleFormat(voice)) {
                GtlVoiceGroup_SetHardwareVoiceType(voice, 0);
                int i = 0;
                State* current = (State*)channelLoopContext;
                for (; i < voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i)
                    GtlVoice_SetRowStateTriplet(voice, i, current);
            }
            if (index == currentCycleBlocks - 1)
                flags[kLoopBoundaryPending] = 1;
        }
    } else {
        {
            InterruptLock lock;
            i = 0;
            current = this;
            for (; i < voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i)
                GtlVoiceGroup_SetRowNonLoopingSampleRange(voice, i, current->channelBuffer,
                                                          lastBlockSamples + index * samplesPerBlock);
        }
        flags[kFinalRangeSet] = 1;
    }
}

// Advance file and circular-buffer playback indices and update loop ranges at either boundary.
void gtl::nw4r::detail::StrmPlayer::AdvanceStreamBlock() {
    int size;
    if (++playingFileBlock > lastFileBlock && loopEnabled) {
        playingFileBlock = loopStartBlock;
        if (loopCount < 0x7fffffff)
            ++loopCount;
        size = currentCycleBlocks * samplesPerBlock;
        InterruptLock lock;
        int i = 0;
        State* current = this;
        for (; i < voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i) {
            ((gtl::nw4r::detail::Voice*)(voice))->SetChannelLoopSampleAddress(i, (int)current->channelBuffer, 0);
            GtlVoiceGroup_SetRowEndSampleAddress(voice, i, current->channelBuffer, size);
        }
        GtlVoiceGroup_SetLoopFlag(voice, 1);
    }
    if (++playingBufferBlock >= currentCycleBlocks) {
        int chunk = nextCycleBlocks;
        playingBufferBlock = 0;
        size = chunk * samplesPerBlock;
        currentCycleBlocks = chunk;
        InterruptLock lock;
        int i = 0;
        State* current = this;
        for (; i < voiceChannels; current = (State*)((char*)current + kChannelRecordBytes), ++i) {
            ((gtl::nw4r::detail::Voice*)(voice))->SetChannelLoopSampleAddress(i, (int)current->channelBuffer, 0);
            GtlVoiceGroup_SetRowEndSampleAddress(voice, i, current->channelBuffer, size);
        }
        GtlVoiceGroup_SetLoopFlag(voice, 1);
    }
    if (playingBufferBlock == currentCycleBlocks - 1 && !GtlVoiceGroup_GetFirstSampleFormat(voice)) {
        if (!flags[kLoopBoundaryPending] && flags[kLoopContextPending]) {
            InterruptLock lock;
            int i = 0;
            // Shift the byte base to address consecutive two-byte predictor scales;
            // these positions are not separate State objects or channel records.
            char* channelByteCursor = (char*)this;
            for (; i < voiceChannels; channelByteCursor += 2, ++i) {
                unsigned short params[3];
                params[0] = *(unsigned short*)(channelByteCursor + (int)&((State*)0)->pendingLoopPredictorScale);
                params[1] = 0;
                params[2] = 0;
                GtlVoice_SetRowStateTriplet(voice, i, params);
            }
            GtlVoiceGroup_SetHardwareVoiceType(voice, 1);
        }
        flags[kLoopContextPending] = 0;
        flags[kLoopBoundaryPending] = 0;
    }
    if (playingFileBlock == lastFileBlock - 1)
        ConfigureBlockBoundaryAddresses(playingBufferBlock + 1);
}

extern "C" {

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
inline bool listEmpty(List* list) {
    return list->count == 0;
}
// Append a packet link before the list sentinel.
inline void packetInsert(List* list, Node* node) {
    Node* anchor = &list->head;
    gtl_IntrusiveList_InsertBefore(list, &anchor, node);
}

// Take a free read packet, submit one aligned file block, and advance file and buffer write indices.
void gtl_nw4r_detail_StrmPlayer_RequestNextBlock(State* self, int asynchronous) {
    if (self->flags[kAllReadsQueued])
        return;
    ReadPacket* packet;
    {
        InterruptLock lock;
        if (listEmpty(&self->freeReadPackets) & 1)
            return;
        Node* node = self->freeReadPackets.head.next;
        packet = (ReadPacket*)((char*)node - 0x14);
        IntrusiveList_UnlinkNodeAndDecrementCount(&self->freeReadPackets, &node);
    }
    packet->asynchronous = asynchronous;
    packet->chunk = self->nextReadBufferBlock;
    packet->index = self->nextReadFileBlock;
    int readBlockBytes;
    if (*(volatile int*)&self->nextReadFileBlock < self->lastFileBlock)
        readBlockBytes = self->blockBytes;
    else
        readBlockBytes = self->lastBlockAlignedBytes;
    unsigned int alignedReadBytes =
        (self->blockHeaderBytes + readBlockBytes * self->voiceChannels + (kReadAlignment - 1)) & ~(kReadAlignment - 1);
    int fileOffset = self->fileDataOffset +
                     self->nextReadFileBlock * (self->blockHeaderBytes + self->blockBytes * self->sourceChannelCount);
    packetInsert(&self->pendingReadPackets, &packet->node);
    int initializeDecoder = 0;
    if (!self->nextReadBufferBlock && !self->sampleFormat)
        initializeDecoder = 1;
    int result =
        self->reader->read(s_streamReadScratchBuffer, alignedReadBytes, fileOffset, self->voiceChannels, readBlockBytes,
                           self->blockHeaderBytes, initializeDecoder, packet, self, self->readerUserData);
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
        if (self->loopEnabled)
            self->nextReadFileBlock = self->loopStartBlock;
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
        if ((next - remaining) % span == 0)
            count = next;
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
    if (self->voice)
        GtlVoice_Stop(self->voice);
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

} // extern "C"

// Clear the player's voice on every terminal callback; release it explicitly for callback reasons 0 and 1.
void gtl::nw4r::detail::StrmPlayer::OnVoiceTerminalEvent(void* arg, int mode, StrmPlayer* self) {
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
