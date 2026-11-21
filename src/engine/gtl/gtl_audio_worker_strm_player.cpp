#include <decomp/static_init.h>
#include "classes/StreamWorker8040C2C8.h"
#define AsStreamWorker8040C2C8(p) ((StreamWorker8040C2C8*)(p))
// Translation unit 0x8040C2C8-0x8040D32C (25 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" {

extern float s_twoPi_8040C900;
extern float s_twoPi_8040CBA4;
extern const float kFloat_6_2831855;
extern char __vt__Q53gtl4nw4r6detail10StrmPlayer11LoadCommand[];
void __dl__FPv(void* ptr);
void* memset(void*, int, unsigned long);

// Optionally free the storage of an otherwise empty worker helper.
void* dtor_8040C2C8(void* self, int deleteFlag) {
    if (self && deleteFlag > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Optionally free the interrupt-guard base storage.
void* dtor_8040C988(void* self, int deleteFlag) {
    if (self && deleteFlag > 0) {
        __dl__FPv(self);
    }
    return self;
}

struct InterruptBase {
    // Run the nondeleting interrupt-guard base cleanup.
    ~InterruptBase() { dtor_8040C988(this, 0); }
};

struct InterruptLock : InterruptBase {
    int level;
    // Serialize bitmap edits against interrupt handlers, preserving the prior level.
    InterruptLock() { level = OSDisableInterrupts(); }
    // Restore the interrupt state that existed before this guard.
    ~InterruptLock() { OSRestoreInterrupts(level); }
};

static const unsigned int kAudioMessageUpdate = 1;
static const unsigned int kAudioMessageStop = 2;
static const int kStreamLoadCommandCount = 32;
enum StreamPlaybackConstants {
    kStreamAdpcmFormat = 0,
    kStreamChannelSlots = 2,
    kStreamSeekSamples = 0, kStreamSeekMilliseconds = 1,
    kStreamHardwareVoiceType1 = 1 // hardware voice-type domain is forwarded unchanged
};
enum AdpcmDecoderWord {
    kAdpcmCoefficientCount = 16,
    kAdpcmGain = 16, kAdpcmPredictorScale = 17,
    kAdpcmHistory1 = 18, kAdpcmHistory2 = 19, kAdpcmDecoderWords = 20
};

struct BlockPool {
    char* base;
    int size;
    int stride;
    int capacity;
    int count;
    unsigned char bits[4];
};
// The pool BORROWS [base, base+size); it never allocates/frees that memory.
// Bitmap bit n is 1 while slot n is allocated; stride = size/capacity bytes.
// Initialize requires 1..32 blocks and nonzero stride. count prevents allocation
// into unused bits in the last bitmap byte while frees keep the count consistent.
// Free requires a currently allocated in-range address and does not validate or
// reject double frees. Initialize with blockCount == 0 leaves the pool untouched.

#pragma push
#pragma opt_strength_reduction off
// Reserve the first free bitmap slot while interrupts are disabled.
void* BitmapBlockPool_Allocate(BlockPool* self) {
    InterruptLock lock;
    if (self->count >= self->capacity) return 0;
    int stride;
    int offset;
    int count;
    unsigned char value;
    BlockPool* bitmapByteView;
    int bytes = ((self->capacity + 7) & ~7) / 8;
    for (int i = 0; i < bytes; ++i) {
        bitmapByteView = (BlockPool*)((char*)self + i);
        value = bitmapByteView->bits[0];
        if (value != 0xff) {
            unsigned char mask = 1;
            for (int j = 0; j < 8; ++j) {
                if (!(value & mask)) {
                    bitmapByteView->bits[0] |= mask;
                    count = self->count;
                    stride = self->stride;
                    offset = stride * (j + (i << 3));
                    self->count = count + 1;
                    void* result = (void*)(offset + (unsigned int)self->base);
                    return result;
                }
                mask <<= 1;
            }
        }
    }
    return 0;
}
#pragma pop

// Clear the slot containing address and decrement the allocation count.
void BitmapBlockPool_Free(BlockPool* self, unsigned int address) {
    InterruptLock lock;
    unsigned int index = (address - (unsigned int)self->base) / self->stride;
    unsigned char* byte = self->bits + (index >> 3);
    byte[0] &= ~(1 << (index & 7));
    --self->count;
}

// Detach pool storage without changing its allocation count or bitmap.
void BitmapBlockPool_ResetStorage(BlockPool* self) {
    InterruptLock lock;
    self->base = 0;
    self->size = 0;
    self->stride = 0;
    self->capacity = 0;
}

#pragma push
#pragma use_lmw_stmw on
// Divide storage into blockCount equal slots and clear the allocation bitmap.
void BitmapBlockPool_Initialize(BlockPool* self, unsigned int address, unsigned int size, unsigned int blockCount) {
    if (blockCount) {
        InterruptLock lock;
        self->base = (char*)address;
        self->size = size;
        self->stride = size / blockCount;
        self->capacity = blockCount;
        self->count = 0;
        memset(self->bits, 0, 4);
    }
}
#pragma pop

// Optionally free the startup-lock helper base.
void* LockGuardBase_dtor(void* self, int deleteFlag) {
    if (self && deleteFlag > 0) {
        __dl__FPv(self);
    }
    return self;
}

void gtl_IntrusiveList_Dtor(void* self, int deleteFlag);

struct StreamOwner {
    char pad_00[0x378];
    char member[4];
};

// Destroy the worker's registered-object list and optionally delete the worker.
void* gtl_AudioWorker_Dtor(StreamOwner* self, int deleteFlag) {
    if (self) {
        if (&self->member) {
            gtl_IntrusiveList_Dtor(&self->member, 0);
        }
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Lazily construct the worker queues, mutex, and object-list sentinel.
void* gtl_AudioWorker_GetInstance();

struct GlobalLockBase {
    // Preserve the empty lock-base cleanup hook.
    ~GlobalLockBase() {}
};
struct GlobalLock : GlobalLockBase {
    // Acquire the singleton worker mutex; OS mutex nesting is intentional.
    GlobalLock() { OSLockMutex((OSMutex*)((char*)&AsStreamWorker8040C2C8(gtl_AudioWorker_GetInstance())->mutex)); }
    // Release the same global worker mutex at scope exit.
    ~GlobalLock() { OSUnlockMutex((OSMutex*)((char*)&AsStreamWorker8040C2C8(gtl_AudioWorker_GetInstance())->mutex)); }
};

// Unlock the global worker mutex and optionally delete the lock wrapper.
void* dtor_8040CFA0(void* self, int deleteFlag) {
    if (self) {
        OSUnlockMutex((OSMutex*)((char*)&AsStreamWorker8040C2C8(gtl_AudioWorker_GetInstance())->mutex));
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy the intrusive list and optionally delete its storage.
void* gtl_IntrusiveList_DeletingDtor(void* self, int deleteFlag) {
    if (self) {
        gtl_IntrusiveList_Dtor(self, 0);
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

extern signed char s_audioWorkerInitialized;
extern char s_gtlAudioWorker[];
extern char s_audioWorkerDtorRegistration_VoiceManager[];

struct StreamSingleton {
    char pad[0x350];
    int stackBase;
    char pad354[0x18];
    int field36C;
    int field370;
    int field374;
    int field378;
    void* volatile prev;
    void* volatile next;
    int lastUpdateTicks;
    unsigned char running;
};

// Initialize the four-message queue and circular borrowed-object list once.
inline void InitStreamSingleton() {
    StreamSingleton* self = (StreamSingleton*)s_gtlAudioWorker;
    self->prev = 0;
    self->next = 0;
    self->stackBase = 0;
    self->field36C = 0;
    self->field370 = 0;
    self->field378 = 0;
    self->prev = (void*)&self->prev;
    self->next = (void*)&self->prev;
    self->running = 0;
    OSInitMessageQueue((OSMessageQueue*)((char*)&AsStreamWorker8040C2C8(self)->messageQueue), (OSMessage*)((char*)&AsStreamWorker8040C2C8(self)->messageBuffer), 4);
    OSInitThreadQueue((OSThreadQueue*)((char*)&AsStreamWorker8040C2C8(self)->threadQueue));
    OSInitMutex((OSMutex*)((char*)&AsStreamWorker8040C2C8(self)->mutex));
}

#pragma push
#pragma auto_inline off
// Return the lazily initialized, static-storage worker; the caller does not own it.
void* gtl_AudioWorker_GetInstance() {
    if (!s_audioWorkerInitialized) {
        InitStreamSingleton();
        __register_global_object(s_gtlAudioWorker, (void*)gtl_AudioWorker_Dtor, s_audioWorkerDtorRegistration_VoiceManager);
        s_audioWorkerInitialized = 1;
    }
    return s_gtlAudioWorker;
}
#pragma pop

int OSJamMessage(OSMessageQueue*, OSMessage, int);
int OSJoinThread(OSThread*, void*);

// Send the stop message and join the running audio-worker thread.
void gtl_AudioWorker_Stop(StreamSingleton* self) {
    if (self->running) {
        if (!s_audioWorkerInitialized) {
            volatile StreamSingleton* singleton = (StreamSingleton*)s_gtlAudioWorker;
            int zero = 0;
            void* anchor = (void*)&singleton->prev;
            singleton->prev = (void*)zero;
            singleton->next = (void*)zero;
            singleton->stackBase = zero;
            singleton->field36C = zero;
            singleton->field370 = zero;
            singleton->field378 = zero;
            singleton->prev = anchor;
            singleton->next = anchor;
            singleton->running = zero;
            OSInitMessageQueue((OSMessageQueue*)((char*)&AsStreamWorker8040C2C8(singleton)->messageQueue), (OSMessage*)((char*)&AsStreamWorker8040C2C8(singleton)->messageBuffer), 4);
            OSInitThreadQueue((OSThreadQueue*)((char*)&AsStreamWorker8040C2C8(singleton)->threadQueue));
            OSInitMutex((OSMutex*)((char*)&AsStreamWorker8040C2C8(singleton)->mutex));
            __register_global_object(s_gtlAudioWorker, (void*)gtl_AudioWorker_Dtor, s_audioWorkerDtorRegistration_VoiceManager);
            s_audioWorkerInitialized = 1;
        }
        // Blocking jam puts stop ahead of queued updates; join waits for callback removal.
        OSJamMessage((OSMessageQueue*)((char*)&AsStreamWorker8040C2C8(s_gtlAudioWorker)->messageQueue), (OSMessage)kAudioMessageStop, 1);
        OSJoinThread((OSThread*)self, 0);
        self->running = 0;
    }
}

void gtl_AudioObjects_ProcessAudioFrame();
void* gtl_VoicePool_GetInstance();
void GtlVoicePool_StopActiveVoiceRamps(void*);

// Wake the worker for the next audio frame.
void gtl_AudioWorker_OnAudioFrame(void*) {
    if (!s_audioWorkerInitialized) {
        InitStreamSingleton();
        __register_global_object(s_gtlAudioWorker, (void*)gtl_AudioWorker_Dtor, s_audioWorkerDtorRegistration_VoiceManager);
        s_audioWorkerInitialized = 1;
    }
    // Nonblocking send may drop a wake-up when the four-message queue is full.
    OSSendMessage((OSMessageQueue*)((char*)&AsStreamWorker8040C2C8(s_gtlAudioWorker)->messageQueue), (OSMessage)kAudioMessageUpdate, 0);
    gtl_AudioObjects_ProcessAudioFrame();
    void* extraReceiver6 = (void*)gtl_VoicePool_GetInstance();
    GtlVoicePool_StopActiveVoiceRamps(extraReceiver6);
}

// Unlock the mutex held by this guard and optionally delete the guard.
void* ScopedMutexLock_dtor(OSMutex** self, int deleteFlag) {
    if (self) {
        OSUnlockMutex(*self);
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Detach intrusive-list nodes without freeing their containing objects; optionally free self.
void* dtor_8040CCF4(void* self, int deleteFlag) {
    if (self) {
        gtl_IntrusiveList_Dtor(self, 0);
        if (deleteFlag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

void* gtl_AudioController_GetInstance();
void gtl_AudioWorker_OnAudioFrame(void*);
void gtl_AudioController_RegisterCallback(void*, void*, void (*)(void*));
void gtl_AudioController_UnregisterCallback(void*, void*);
// Process audio-frame updates until a stop message flushes registered objects.
void gtl_AudioWorker_RunMessageLoop(void*);

struct PlayerCommandOwner {
    char pad_00[0x36C];
    char command[4];
};

// Register the frame callback for the duration of the worker message loop.
int gtl_AudioWorker_ThreadEntry(PlayerCommandOwner* self) {
    gtl_AudioController_RegisterCallback(gtl_AudioController_GetInstance(), &self->command, gtl_AudioWorker_OnAudioFrame);
    gtl_AudioWorker_RunMessageLoop(self);
    gtl_AudioController_UnregisterCallback(gtl_AudioController_GetInstance(), &self->command);
    return 0;
}

struct LoadCommand {
    void* vtable;
    void* owner;
    char pad_08[0xC];
    int linkPrevious;
    int linkNext;
};

// Initialize an empty stream-load command and its intrusive-list links.
void gtl_nw4r_detail_StrmPlayerLoadCommand_Ctor(LoadCommand* self) {
    self->linkPrevious = 0;
    self->vtable = __vt__Q53gtl4nw4r6detail10StrmPlayer11LoadCommand;
    self->linkNext = 0;
}

extern char __vt__Q43gtl4nw4r6detail10StrmPlayer[];
void gtl_nw4r_detail_BasicPlayer_Ctor(void*);
void dtor_804033F8(void*, int);
void __construct_array(void*, void*, void*, unsigned long, unsigned long);
void gtl_IntrusiveList_InsertBefore(void*, void*, void*);
void gtl_nw4r_detail_StrmPlayer_ResetState(void*, int);

struct PlayerList {
    volatile int count;
    void* volatile prev;
    void* volatile next;
};
// Empty lists link prev/next to the address of prev, not to the list header.
// Commands are embedded, not heap-owned by the list. linkPrevious at +0x14 is
// passed as the link address; list cleanup only detaches those links.

struct PlayerDraft {
    void* vtable;
    char pad04[0x70];
    volatile int field74;
    volatile int field78;
    char pad7C[0x30];
    volatile unsigned char fieldAC;
    char padAD[0x4f];
    PlayerList freeList;
    PlayerList usedList;
    LoadCommand commands[kStreamLoadCommandCount];
    char pad494[0xc];
    int field4A0;
    char pad4A4[0x84];
    char mutex[0x18];
};

#pragma push
#pragma opt_strength_reduction off
// Initialize the stream player and link its 32 load commands into the free list.
PlayerDraft* gtl_nw4r_detail_StrmPlayer_Ctor(PlayerDraft* self) {
    gtl_nw4r_detail_BasicPlayer_Ctor(self);
    self->freeList.prev = 0;
    self->freeList.next = 0;
    self->usedList.prev = 0;
    self->usedList.next = 0;
    self->vtable = __vt__Q43gtl4nw4r6detail10StrmPlayer;
    self->field74 = 0;
    self->field78 = 0;
    self->fieldAC = 0;
    self->freeList.count = 0;
    self->freeList.prev = (void*)&self->freeList.prev;
    self->freeList.next = (void*)&self->freeList.prev;
    self->usedList.count = 0;
    self->usedList.prev = (void*)&self->usedList.prev;
    self->usedList.next = (void*)&self->usedList.prev;
    __construct_array(self->commands, (void*)gtl_nw4r_detail_StrmPlayerLoadCommand_Ctor, (void*)dtor_804033F8, sizeof(LoadCommand), kStreamLoadCommandCount);
    self->field4A0 = 0;
    OSInitMutex((OSMutex*)&self->mutex);
    void* head = (void*)&self->freeList.prev;
    unsigned int offset;
    unsigned int i;
    i = 0;
    offset = 0;
    for (; i < kStreamLoadCommandCount; ++i, offset += 0x1c) {
        PlayerDraft* entry = (PlayerDraft*)((char*)self + offset);
        entry->commands[0].owner = self;
        void* position = head;
        gtl_IntrusiveList_InsertBefore(&self->freeList, &position, &entry->commands[0].linkPrevious);
    }
    gtl_nw4r_detail_StrmPlayer_ResetState(self, 0);
    return self;
}
#pragma pop

// Initialize the stream-player two-pi cache at static startup.
void InitTwoPiCache_8040C900() {
    s_twoPi_8040C900 = kFloat_6_2831855;
}

// Initialize the worker-helper two-pi cache at static startup.
void InitTwoPiCache_8040CBA4() {
    s_twoPi_8040CBA4 = kFloat_6_2831855;
}

struct StrmPlayer {
    char pad_00[0xAC];
    unsigned char active;
};

void gtl_nw4r_detail_StrmPlayer_ReleaseResources(StrmPlayer* self);

// Release playback resources only while the stream player is active.
void gtl_nw4r_detail_StrmPlayer_ReleaseIfActive(StrmPlayer* self) {
    if (self->active) {
        gtl_nw4r_detail_StrmPlayer_ReleaseResources(self);
    }
}

void* dtor_8040C288(void*, int);
int OSResumeThread(OSThread*);
struct MutexBase {
    // Preserve the empty mutex-guard base cleanup.
    ~MutexBase() {}
};

struct MutexGuard : MutexBase {
    OSMutex* mutex;
    // Lock the supplied borrowed OS mutex for this scope.
    MutexGuard(OSMutex* value) : mutex(value) { OSLockMutex(mutex); }
    // Unlock the OS mutex; it remains owned by its enclosing object.
    ~MutexGuard() { OSUnlockMutex(mutex); }
};


void* GtlVoicePool_GetInstance();
void GtlVoicePool_DrainPendingReleases(void*);
void gtl_AudioController_Update(void*);
void gtl_AudioObjects_ProcessWorkerMessage();
void gtl_VoicePool_UpdateActiveVoices(void*);
void gtl_Audio_UpdateRegisteredObjects();

struct StreamNode {
    StreamNode* next;
    StreamNode* prev;
    virtual void slot8();
    virtual void duringTimedUpdate(); // table +0x0C
    virtual void beforeTimedUpdate(); // table +0x10
    virtual void afterTimedUpdate();
    // table +0x14
};
// Worker callbacks execute on ThreadEntry's thread while its mutex is held.
// Their established responsibility is participation in this phase ordering;
// subtype-specific work is not identified here. They borrow their node/object.
// Each walk saves next before dispatch: self-unlink is safe, but the saved next
// node must remain alive and linked for traversal. This is not a snapshot:
// each new phase starts from the current head. All external list edits must
// serialize with this mutex; interrupt disabling alone does not replace it.
// Callbacks must return before the worker can consume stop/join requests.

#pragma push
#pragma use_lmw_stmw on
#pragma opt_loop_invariants off
#pragma auto_inline off
#pragma inline_depth(2)
#pragma push
#pragma opt_propagation off
// Block for update/stop messages and run the registered objects' three update phases.
void gtl_AudioWorker_RunMessageLoop(void* object) {
    StreamSingleton* self = (StreamSingleton*)object;
    for (;;) {
        OSMessage message;
        OSReceiveMessage((OSMessageQueue*)((char*)&AsStreamWorker8040C2C8(self)->messageQueue), &message, 1);
        if ((unsigned int)message == kAudioMessageUpdate) {
            unsigned int start;
            MutexGuard lock((OSMutex*)((char*)&AsStreamWorker8040C2C8(self)->mutex));
            // Phase 1: let registered objects run before release draining/controller updates.
            // Save each successor before callbacks, which may unlink their own node.
            StreamNode* node = (StreamNode*)self->prev;
            StreamNode* end = (StreamNode*)&self->prev;
            while (node != end) {
                StreamNode* current = node;
                node = node->next;
                current->beforeTimedUpdate();
            }
            // Phase 2: drain releases, update controller and stream voice parameters,
            // then let registered objects update before hardware voices are refreshed.
            // lastUpdateTicks measures only this interval in OS ticks.
            start = OSGetTick();
            void* extraReceiver7 = (void*)GtlVoicePool_GetInstance();
            GtlVoicePool_DrainPendingReleases(extraReceiver7);
            void* extraReceiver8 = (void*)gtl_AudioController_GetInstance();
            gtl_AudioController_Update(extraReceiver8);
            gtl_AudioObjects_ProcessWorkerMessage();
            StreamNode* node2 = (StreamNode*)self->prev;
            StreamNode* end2 = (StreamNode*)&self->prev;
            while (node2 != end2) {
                StreamNode* current = node2;
                node2 = node2->next;
                current->duringTimedUpdate();
            }
            void* extraReceiver9 = (void*)gtl_VoicePool_GetInstance();
            gtl_VoicePool_UpdateActiveVoices(extraReceiver9);
            self->lastUpdateTicks = OSGetTick() - start;
            // Phase 3: notify registered objects after voice updates and timing.
            // The same mutex covers every phase; callbacks run without unlocking.
            StreamNode* node3 = (StreamNode*)self->prev;
            StreamNode* end3 = (StreamNode*)&self->prev;
            while (node3 != end3) {
                StreamNode* current = node3;
                node3 = node3->next;
                current->afterTimedUpdate();
            }
        } else if ((unsigned int)message == kAudioMessageStop) {
            gtl_Audio_UpdateRegisteredObjects();
            return;
        }
    }
}
#pragma pop

// Create and resume the worker thread once, retaining its stack base.
// stack is caller-owned bytes and must outlive Stop()/join; OSCreateThread gets
// the top (stack+stackSize). Failure leaves running set, matching existing behavior.
bool gtl_AudioWorker_Start(StreamSingleton* self, int priority, char* stack, unsigned int stackSize) {
    if (self->running) return true;
    self->stackBase = (int)stack;
    self->running = 1;
    if (!s_audioWorkerInitialized) {
        volatile StreamSingleton* singleton = (StreamSingleton*)s_gtlAudioWorker;
        int zero = 0;
        void* anchor = (void*)&singleton->prev;
        singleton->prev = (void*)zero;
        singleton->next = (void*)zero;
        singleton->stackBase = zero;
        singleton->field36C = zero;
        singleton->field370 = zero;
        singleton->field378 = zero;
        singleton->prev = anchor;
        singleton->next = anchor;
        singleton->running = zero;
        OSInitMessageQueue((OSMessageQueue*)((char*)&AsStreamWorker8040C2C8(singleton)->messageQueue), (OSMessage*)((char*)&AsStreamWorker8040C2C8(singleton)->messageBuffer), 4);
        OSInitThreadQueue((OSThreadQueue*)((char*)&AsStreamWorker8040C2C8(singleton)->threadQueue));
        OSInitMutex((OSMutex*)((char*)&AsStreamWorker8040C2C8(singleton)->mutex));
        __register_global_object(s_gtlAudioWorker, (void*)gtl_AudioWorker_Dtor, s_audioWorkerDtorRegistration_VoiceManager);
        s_audioWorkerInitialized = 1;
    }
    int result = OSCreateThread((OSThread*)self, (void* (*)(void*))gtl_AudioWorker_ThreadEntry, s_gtlAudioWorker,
                            stack + stackSize, stackSize, priority, 0);
    if (result) OSResumeThread((OSThread*)self);
    return result != 0;
}

struct StreamConfig {
    unsigned char sampleFormat, loopFlag, channelCount, reserved; // +00..03
    unsigned short sampleRate, blockHeaderBytes; // +04 Hz, +06 bytes per file block
    unsigned int loopStartSample;       // +08 sample index
    unsigned int totalSamples;          // +0C exclusive sample end (loop end when cropped)
    unsigned int fileDataOffset;        // +10 byte offset of first file block
    unsigned int fileBlockCount;        // +14 number of file blocks
    unsigned int blockBytes;            // +18 full block bytes PER SOURCE CHANNEL
    unsigned int samplesPerBlock;       // +1C divisor for seeks/loop positions
    unsigned int lastBlockBytes;        // +20 unaligned final block bytes per channel
    unsigned int lastBlockSamples;      // +24 valid samples in final block
    unsigned int lastBlockAlignedBytes; // +28 final bytes rounded up to 32
    unsigned int unresolved2C;          // +2C copied verbatim; no consumer identified
};
// Configuration is copied by value; channelCount must fit the two supplied
// decoder-state slots for ADPCM preparation. Field domains are corroborated by
// ResolveStartBlock/RequestNextBlock in gtl_strm_player_streaming.cpp and SoundStreamLoopFormat.
// A file block contains blockHeaderBytes + channelCount*blockBytes, except
// the last block's per-channel extent. Preserve both unaligned/aligned sizes.
// Format zero triggers copying 24 16-bit ADPCM-state words and clearing the two
// per-channel history arrays. Other formats skip that state copy.
struct ChannelSamples {
    unsigned short words[24]; // 48-byte decoder-state copy; see StartSamples below
};
struct StreamChannel {
    ChannelSamples samples;
    char pad[8];
};
struct StreamSettings {
    StreamConfig config;
    StreamChannel channels[kStreamChannelSlots];
};
struct StreamHistoryView {
    char pad_000[0x520];
    unsigned short history1[kStreamChannelSlots], history2[kStreamChannelSlots];
};
struct StreamPlayback {
    char pad00[0x7c];
    StreamConfig config;
    unsigned char active;
    char padAD[0x47];
    int seekUnit;
    int seekPosition;
    char padFC[0x398];
    void* bufferPool;
    void* reader;
    int readerUserData;
    char pad4A0[0x14];
    StreamChannel channels[kStreamChannelSlots];
    char pad524[0xc];
};
void gtl_nw4r_detail_StrmPlayer_PreloadBlocks(void*, int);
// Copy a halfword pair while prefetching the next pair's first word.
// carried/second are caller scratch registers; source needs three readable words.
static inline void copyDecoderStatePair(unsigned short* dest, const unsigned short* source,
                                       unsigned short& carried, unsigned short& second) {
    second = source[1];
    dest[0] = carried;
    carried = source[2];
    dest[1] = second;
}
#pragma inline_depth(2)
#pragma opt_propagation off
// Copy the stream configuration and per-channel ADPCM state, then preload playback blocks.
bool gtl_nw4r_detail_StrmPlayer_PreparePlayback(StreamPlayback* self, void* bufferPool, int seekUnit, int seekPosition, int voiceAllocationArgument,
                   void* reader, int readerUserData, StreamSettings* settings) {
    // settings is copied; bufferPool and reader stay borrowed through resource
    // release. readerUserData is forwarded unchanged to read/release callbacks.
    // seekUnit: 0=samples, 1=milliseconds (ResolveStartBlock in gtl_strm_player_streaming.cpp).
    // Clamp negative seekPosition to zero. voiceAllocationArgument is forwarded
    // through ResetState to voice acquisition; its finer domain is unresolved.
    // Preloading precedes setting active; return is always true even if preload
    // releases resources on failure. Callers must check readiness separately.
    unsigned int unresolved2C,lastBlockAlignedBytes,lastBlockSamples,lastBlockBytes,samplesPerBlock,blockBytes,fileBlockCount,fileDataOffset,totalSamples,loopStartSample;
    unsigned short blockHeaderBytes,sampleRateHz;
    unsigned char unresolved03,sourceChannelCount,loopEnabled,sampleFormat;
    int clampedSeekPosition;
    GlobalLock lock;
    // Phase 1: release prior playback and reset voice-allocation state under the global lock.
    if (self->active) gtl_nw4r_detail_StrmPlayer_ReleaseResources((StrmPlayer*)self);
    gtl_nw4r_detail_StrmPlayer_ResetState(self, voiceAllocationArgument);
    clampedSeekPosition = seekPosition < 0 ? 0 : seekPosition;
    // Phase 2: retain configuration values, not the caller's settings pointer.
    sampleFormat = settings->config.sampleFormat;
    loopEnabled = settings->config.loopFlag;
    sourceChannelCount = settings->config.channelCount;
    unresolved03 = settings->config.reserved;
    sampleRateHz = settings->config.sampleRate;
    blockHeaderBytes = settings->config.blockHeaderBytes;
    loopStartSample = settings->config.loopStartSample;
    totalSamples = settings->config.totalSamples;
    fileDataOffset = settings->config.fileDataOffset;
    fileBlockCount = settings->config.fileBlockCount;
    blockBytes = settings->config.blockBytes;
    samplesPerBlock = settings->config.samplesPerBlock;
    lastBlockBytes = settings->config.lastBlockBytes;
    lastBlockSamples = settings->config.lastBlockSamples;
    lastBlockAlignedBytes = settings->config.lastBlockAlignedBytes;
    unresolved2C = settings->config.unresolved2C;
    self->seekUnit = seekUnit;
    self->seekPosition = clampedSeekPosition;
    self->reader = reader;
    self->readerUserData = readerUserData;
    self->bufferPool = bufferPool;
    self->config.sampleFormat = sampleFormat;
    self->config.loopFlag = loopEnabled;
    self->config.channelCount = sourceChannelCount;
    self->config.reserved = unresolved03;
    self->config.sampleRate = sampleRateHz;
    self->config.blockHeaderBytes = blockHeaderBytes;
    self->config.loopStartSample = loopStartSample;
    self->config.totalSamples = totalSamples;
    self->config.fileDataOffset = fileDataOffset;
    self->config.fileBlockCount = fileBlockCount;
    self->config.blockBytes = blockBytes;
    self->config.samplesPerBlock = samplesPerBlock;
    self->config.lastBlockBytes = lastBlockBytes;
    self->config.lastBlockSamples = lastBlockSamples;
    self->config.lastBlockAlignedBytes = lastBlockAlignedBytes;
    self->config.unresolved2C = unresolved2C;
    // Phase 3: seed format-zero channel state before preloading any stream blocks.
    if (sampleFormat == kStreamAdpcmFormat) {
        StreamChannel* source = settings->channels;
        StreamPlayback* dest = self;
        StreamPlayback* counters = self;
        for (int i = 0; i < self->config.channelCount; ++i) {
            unsigned short w;
            unsigned short v = source->samples.words[0];
            // Pipeline two halfword copies: v carries the first word and each
            // helper prefetches the following pair. Finish without reading word 24.
            copyDecoderStatePair(dest->channels[0].samples.words + 0, source->samples.words + 0, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 2, source->samples.words + 2, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 4, source->samples.words + 4, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 6, source->samples.words + 6, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 8, source->samples.words + 8, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 10, source->samples.words + 10, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 12, source->samples.words + 12, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 14, source->samples.words + 14, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 16, source->samples.words + 16, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 18, source->samples.words + 18, v, w);
            copyDecoderStatePair(dest->channels[0].samples.words + 20, source->samples.words + 20, v, w);
            w = source->samples.words[23];
            dest->channels[0].samples.words[22] = v;
            dest->channels[0].samples.words[23] = w;
            ++source;
            dest = (StreamPlayback*)((char*)dest + sizeof(StreamChannel));
            ((StreamHistoryView*)counters)->history1[0] = 0;
            ((StreamHistoryView*)counters)->history2[0] = 0;
            counters = (StreamPlayback*)((char*)counters + 2);
        }
    }
    gtl_nw4r_detail_StrmPlayer_PreloadBlocks(self, 0);
    self->active = 1;
    return true;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw off
#pragma opt_propagation on
#pragma auto_inline on
#pragma inline_depth(3)
struct StartLockBase {
    // Preserve the original nondeleting startup-guard base cleanup.
    ~StartLockBase() { LockGuardBase_dtor(this, 0); }
};
struct StartLock : StartLockBase {
    // Serialize playback startup against worker updates with the singleton mutex.
    StartLock() { OSLockMutex((OSMutex*)((char*)&AsStreamWorker8040C2C8(gtl_AudioWorker_GetInstance())->mutex)); }
    // Release the startup operation's worker-mutex nesting level.
    ~StartLock() { OSUnlockMutex((OSMutex*)((char*)&AsStreamWorker8040C2C8(gtl_AudioWorker_GetInstance())->mutex)); }
};
struct StartInterrupt : StartLockBase {
    int level;
    // Disable interrupts while merging pause sources and updating the voice.
    StartInterrupt() { level = OSDisableInterrupts(); }
    // Restore the interrupt state saved by this startup scope.
    ~StartInterrupt() { OSRestoreInterrupts(level); }
};
struct StartSamples {
    // Words 0..15 are coefficients, followed by gain, predictor/scale, yn1, yn2.
    // Keep this a 20-word block: its implicit copy uses paired halfword loads.
    unsigned short decoderWords[kAdpcmDecoderWords];
    unsigned short loopPredictorScale, loopHistory1, loopHistory2, unresolved2E;
};
// DSP ADPCM state is raw halfwords, including signed coefficients/history bits.
// ConfigureSamplePlayback in gtl_voice_control.cpp consumes the first 20 words and the loop
// triplet at +0x28. The final halfword is copied but its meaning is unproven.
struct StartSource {
    unsigned char* data;
    int bufferBytes;
    StartSamples samples;
};
struct StartChannel {
    unsigned char* data;
    char pad04[0x10];
    StartSamples samples;
};
struct StartPacket {
    unsigned char format, loopEnabled, channelCount, pad03;
    unsigned int sampleRateHz, loopStartSample, endSample;
    StartChannel channels[kStreamChannelSlots];
};
struct StartPlayer {
    char pad00[0x74];
    void* activeLinkPrevious;
    void* activeLinkNext;
    unsigned char format;
    char pad7D[3];
    unsigned short rate;
    char pad82[0xe];
    unsigned int fileBlockCount;
    char pad94[0x19];
    unsigned char started, ready, padAF, pauseRequested, padB1, voicePaused, waitingForReads;
    char padB4[8];
    unsigned int completedLoopCount;
    char padC0[8];
    int bufferBlockBytes;
    char padCC[0x14];
    int currentCycleBlocks;
    char padE4[0x3bc];
    void* voice;
    int channelCount;
    char pad4A8[4];
    StartSource sources[kStreamChannelSlots];
    char pad51C[4];
    unsigned short history1[kStreamChannelSlots];
    unsigned short history2[kStreamChannelSlots];
};
int gtl_StrmPlayer_ResolveStartBlock(void*, unsigned int*, unsigned int*, unsigned int*);
unsigned int AudioStream_ByteOffsetToSampleOffset(int, unsigned char);
void GtlVoice_ConfigureSamplePlayback(void*, StartPacket*, unsigned int);
void GtlVoiceGroup_SetHardwareVoiceType(void*, int);
void GtlStrmPlayer_ConfigureBlockBoundaryAddresses(void*, int);
void GtlVoice_RequestStart(void*);
void StrmPlayer_SetPauseFlag(void*, unsigned int);
extern char s_activeStreamPlayers[];

// Fill one borrowed channel packet with saved coefficients and current decoder history.
// Caller advances packet/source/history cursors by their distinct 0x44/0x38/2 strides.
static inline void fillStartChannelPacket(StartChannel* dest, StartPlayer* source, StartPlayer* history) {
    dest->data = source->sources[0].data;
    dest->samples = source->sources[0].samples;
    dest->samples.decoderWords[kAdpcmPredictorScale] = source->sources[0].data[0];
    dest->samples.decoderWords[kAdpcmHistory1] = history->history1[0];
    dest->samples.decoderWords[kAdpcmHistory2] = history->history2[0];
}

// Apply requested-pause OR waiting-for-reads only when the hardware state changes.
// Caller holds the worker mutex and an interrupt guard for this atomic merge.
static inline void synchronizeStartPause(StartPlayer* self) {
    unsigned int pause = 0;
    if (self->pauseRequested) pause = 1;
    if (self->waitingForReads) pause = 1;
    if (pause != self->voicePaused) {
        if (self->voice) StrmPlayer_SetPauseFlag(self->voice, pause);
        self->voicePaused = pause;
    }
}

// Configure stream-channel playback, start the voice, and synchronize pause state.
bool gtl_nw4r_detail_StrmPlayer_StartPlayback(StartPlayer* self) {
    // Ready and a voice are required. Repeated calls on a started player succeed
    // without starting twice. The stack packet is consumed by ConfigureSamplePlayback.
    StartLock lock;
    if (!self->ready) return false;
    if (!self->voice) return false;
    if (!self->started) {
        // Phase 1: resolve a FILE block, an in-block sample offset (ADPCM), and
        // completed loops caused by seeking past the end. These are not bytes.
        unsigned int firstFileBlock = 0;
        unsigned int sampleOffset = 0;
        unsigned int completedLoops = 0;
        if (!gtl_StrmPlayer_ResolveStartBlock(self, &firstFileBlock, &sampleOffset, &completedLoops)) return false;
        self->completedLoopCount += completedLoops;
        // Phase 2: describe the circular BUFFER as a looping hardware sample
        // range. Logical file looping is handled separately by block boundaries.
        StartPacket packet;
        packet.format = self->format;
        packet.loopEnabled = 1;
        packet.channelCount = self->channelCount;
        packet.sampleRateHz = self->rate;
        packet.loopStartSample = 0;
        packet.endSample = AudioStream_ByteOffsetToSampleOffset(self->bufferBlockBytes * self->currentCycleBlocks, self->format);
        StartChannel* dest = packet.channels;
        StartPlayer* source = self;
        StartPlayer* history = self;
        for (int i = 0; i < self->channelCount; ++i) {
            fillStartChannelPacket(dest, source, history);
            source = (StartPlayer*)((char*)source + sizeof(StartSource));
            history = (StartPlayer*)((char*)history + 2);
            ++dest;
        }
        // Phase 3: configure the voice, patch a start in either final file block,
        // then request playback. A ring range and a file-loop range differ.
        GtlVoice_ConfigureSamplePlayback(self->voice, &packet, sampleOffset);
        GtlVoiceGroup_SetHardwareVoiceType(self->voice, kStreamHardwareVoiceType1);
        if (firstFileBlock == self->fileBlockCount - 2) GtlStrmPlayer_ConfigureBlockBoundaryAddresses(self, 1);
        else if (firstFileBlock == self->fileBlockCount - 1) GtlStrmPlayer_ConfigureBlockBoundaryAddresses(self, 0);
        GtlVoice_RequestStart(self->voice);
        // Phase 4: requested pause OR read backlog determines effective pause.
        {
            StartInterrupt interrupt;
            synchronizeStartPause(self);
        }
        // Phase 5: publish exactly one borrowed intrusive link for future updates.
        void* position = s_activeStreamPlayers + 4;
        gtl_IntrusiveList_InsertBefore(s_activeStreamPlayers, &position, &self->activeLinkPrevious);
        self->started = 1;
    }
    return true;
}
#pragma pop

// Guard the overlapping local views against the offsets used by preparation,
// streaming, and voice configuration. These typedefs emit no storage or code.
#define STREAM_LOCAL_OFFSET(type, field, offset) \
    typedef char type##_##field##_offset[((int)&((type*)0)->field == (offset)) ? 1 : -1]
STREAM_LOCAL_OFFSET(StreamConfig, blockHeaderBytes, 0x06);
STREAM_LOCAL_OFFSET(StreamConfig, loopStartSample, 0x08);
STREAM_LOCAL_OFFSET(StreamConfig, fileDataOffset, 0x10);
STREAM_LOCAL_OFFSET(StreamConfig, fileBlockCount, 0x14);
STREAM_LOCAL_OFFSET(StreamConfig, blockBytes, 0x18);
STREAM_LOCAL_OFFSET(StreamConfig, samplesPerBlock, 0x1C);
STREAM_LOCAL_OFFSET(StreamConfig, lastBlockBytes, 0x20);
STREAM_LOCAL_OFFSET(StreamConfig, lastBlockSamples, 0x24);
STREAM_LOCAL_OFFSET(StreamConfig, lastBlockAlignedBytes, 0x28);
STREAM_LOCAL_OFFSET(StreamConfig, unresolved2C, 0x2C);
STREAM_LOCAL_OFFSET(StreamPlayback, config, 0x7C);
STREAM_LOCAL_OFFSET(StreamPlayback, seekUnit, 0xF4);
STREAM_LOCAL_OFFSET(StreamPlayback, seekPosition, 0xF8);
STREAM_LOCAL_OFFSET(StreamPlayback, bufferPool, 0x494);
STREAM_LOCAL_OFFSET(StreamPlayback, reader, 0x498);
STREAM_LOCAL_OFFSET(StreamPlayback, readerUserData, 0x49C);
STREAM_LOCAL_OFFSET(StreamPlayback, channels, 0x4B4);
STREAM_LOCAL_OFFSET(StartSamples, loopPredictorScale, 0x28);
STREAM_LOCAL_OFFSET(StartPacket, channels, 0x10);
STREAM_LOCAL_OFFSET(StartPlayer, completedLoopCount, 0xBC);
STREAM_LOCAL_OFFSET(StartPlayer, bufferBlockBytes, 0xC8);
STREAM_LOCAL_OFFSET(StartPlayer, currentCycleBlocks, 0xE0);
STREAM_LOCAL_OFFSET(StartPlayer, sources, 0x4AC);
STREAM_LOCAL_OFFSET(StartPlayer, history1, 0x520);
STREAM_LOCAL_OFFSET(StartPlayer, history2, 0x524);
#undef STREAM_LOCAL_OFFSET
typedef char StreamConfig_size[(sizeof(StreamConfig) == 0x30) ? 1 : -1];
typedef char StartSamples_size[(sizeof(StartSamples) == 0x30) ? 1 : -1];
typedef char StartSource_size[(sizeof(StartSource) == 0x38) ? 1 : -1];
typedef char StartChannel_size[(sizeof(StartChannel) == 0x44) ? 1 : -1];

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626EC", _ctors_u8040C2C8_0, InitTwoPiCache_8040C900);
STATIC_INIT_ENTRY(".CRT$XCU804626EC", _ctors_u8040C2C8_1, InitTwoPiCache_8040CBA4);
#pragma pop
