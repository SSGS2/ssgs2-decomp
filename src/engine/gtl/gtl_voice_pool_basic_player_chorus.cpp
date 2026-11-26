// Voice pool free list, the basic audio player (BasicPlayer) parameter setters, and the chorus
// effect (FxChorus) constructor, output buffer and initialization.

#include <decomp/static_init.h>
#include "classes/GtlBasicPlayer.h"
#include "classes/GtlFxChorus.h"
#define AsGtlFxChorus(p) ((GtlFxChorus*)(p))

#include "sdk_protos.h"

extern "C" {

enum {
    kVoiceEntryBytes = 0x48,
    kVoiceLinkOffset = 0x40,
    kReservedVoiceCount = 16,
    kAXVoiceStopped = 0,
    kAXVoiceRunning = 1,
    kCallbackShutdown = 0,
    kCallbackStolen = 1,
    kChorusMinimumDelay = 1,
    kChorusMaximumDelay = 50,
    kChorusMinimumPeriod = 500,
    kChorusMaximumPeriod = 10000,
    kBasicPlayerInvalidId = -1,
    kBasicPlayerMainOutput = 1
};
struct AudioVoiceList {
    unsigned int count;
    void* first;
    void* last;
};
// Lists embed a two-word sentinel at &first: empty first/last point there,
// and count excludes the sentinel. Nodes are links at voice+0x40; FromLink
// recovers the enclosing 0x48-byte entry. Entry storage is supplied by the
// caller to InitializeFreeList and is never freed by the list destructor.
// Lifecycle: free -> active -> pending (AX callback) -> free (drain), or
// active -> free (explicit release). pendingRelease chooses the source list.
struct AudioVoicePoolLayout {
    AudioVoiceList active, free, pending;
    unsigned char initialized;
    char padding[3];
    unsigned int capacity;
};
// Keep byte cursors at calls: typed subobject addresses make CodeWarrior hoist them.
enum {
    kAudioFreeListOffset = (int)&((AudioVoicePoolLayout*)0)->free,
    kAudioPendingListOffset = (int)&((AudioVoicePoolLayout*)0)->pending,
    kAudioActiveHeadOffset = (int)&((AudioVoicePoolLayout*)0)->active.first,
    kAudioFreeHeadOffset = (int)&((AudioVoicePoolLayout*)0)->free.first,
    kAudioPendingHeadOffset = (int)&((AudioVoicePoolLayout*)0)->pending.first,
    kChorusControllerOffset = (int)&((GtlFxChorus*)0)->initializationAttempted,
    kChorusWorkOffset = (int)&((GtlFxChorus*)0)->chorusState
};
struct AudioVoiceEntry {
    void* axVoice;
    char padding04[0x19];
    unsigned char pendingRelease;
    char padding1E[0x1A];
    void (*callback)(void*, int, unsigned int);
    unsigned int callbackData;
    void* links[2];
};
struct AXVoiceStateView {
    char padding[0x38];
    unsigned short state;
};
// View caller-supplied pool storage without changing ownership.
inline AudioVoicePoolLayout* AsAudioVoicePool(void* pool) {
    return (AudioVoicePoolLayout*)pool;
}
// View one fixed-size voice entry without changing ownership.
inline AudioVoiceEntry* AsAudioVoiceEntry(void* entry) {
    return (AudioVoiceEntry*)entry;
}
// List iterators refer to the embedded link, rather than the voice's beginning.
inline char* AudioVoice_FromLink(void* link) {
    return (char*)link - kVoiceLinkOffset;
}

struct InterruptUnwindCleanup {
    BOOL level;
    bool active;
    // Restore the saved interrupt level if acquisition exits by exception.
    explicit InterruptUnwindCleanup(BOOL saved) : level(saved), active(true) {}
    // Hand restoration to the explicit normal-exit path.
    BOOL release() {
        active = false;
        return level;
    }
    // Restore interrupts only while the unwind guard still owns that duty.
    ~InterruptUnwindCleanup() {
        if (active)
            OSRestoreInterrupts(level);
    }
};

extern float kFloat_6_2831855;
extern float s_voicePoolTwoPi;
extern float s_basicPlayerTwoPi;
extern float kBasicPlayerResetScale;
extern float kBasicPlayerResetOffset;
extern char __vt__Q43gtl4nw4r6detail11BasicPlayer[];
void AudioEffect_SetOutputBuffer(void*, int, unsigned int);
void AudioController_ClearOutputBuffer(void*);
void gtl_IntrusiveList_Dtor(void*, int);
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
// SDK chorus initialization expands integer parameters into the floating-point work state.
int AXFXChorusInit(void*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);
void IntrusiveList_UnlinkNode(void*, void*);
void gtl_IntrusiveList_InsertBefore(void*, void**, void*);
void* AudioVoicePool_TakeFreeVoice(void*);
void* AXAcquireVoice(void*, void*, void*);
void AudioVoice_InitializeVolumeEnvelopeState(void*, void*);
void IntrusiveList_UnlinkNode(void*, void*);
void GtlVoice_ReturnValuePairToPool();
void AXFreeVoice(void*);
void AudioVoiceAux_Dtor(void*, int);
void InterruptGuard_dtor(void*);

// Repeated iterator words preserve the original by-value library iterator ABI.
struct LocalIter {
    void* a;
    void* b;
    void* c;
    void* d;
    void* e;
    void* f;
};
void AXSetVoiceState(void*, int);
void gtl_nw4r_detail_BasicPlayer_ResetParameters(void*);
// Return the SDK's configured maximum AX voice count.
int AXGetMaxVoices();
void __dl__FPv(void*);
extern int g_audioEffectAllocatedBytes;
void IntrusiveList_UnlinkNodeAndDecrementCount(void*, void**);
void* AudioVoice_ResetParameters(unsigned int);
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void Chorus_ReconfigureAxfxState(void*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);
extern unsigned int kChorusDefaultParameters[3];
extern float kFloat_1000_FxChorus;
extern char __vt__Q33gtl4nw4r8FxChorus[];
extern char s_audioVoicePool[];
extern char s_audioVoicePoolDtorRegistration[];
extern signed char s_audioVoicePoolInitialized;
void IntrusiveList_UnlinkNode(void*, void*);
void* __register_global_object(void*, void*, void*);
void gtl_IntrusiveList_InsertBefore(void*, void**, void*);
void AXFreeVoice(void*);
void AudioVoiceAux_Dtor(void*, int);

// Destroy the pending, free, and active voice lists, then optionally free the pool.
void* GtlVoicePool_dtor(void* self, int deleting);

// Initialize chorus work memory under the effect allocator hooks and mark initialization attempted.
int gtl_nw4r_FxChorus_Initialize(void* self);

// Move an active voice to the pending-release list with interrupts disabled.
void AudioVoicePool_QueuePendingRelease(void* pool, void* voice);

// Size voice storage for available AX voices plus the reserved software entries.
int GtlVoicePool_GetRequiredMemorySize();

// Link fixed-size voice entries into the free list once and record their capacity.
void AudioVoicePool_InitializeFreeList(void* self, void* entries, unsigned int size);

// Stop voices, notify their callbacks, release AX resources, and drain all pool lists.
void AudioVoicePool_Shutdown(void* self);

#pragma use_lmw_stmw on
// Take a free entry, acquire an AX voice, and store its callback/context under interrupt protection.
void* AudioVoicePool_AcquireAXVoice(void* self, void* priority, unsigned int callbackAddress,
                                    unsigned int callbackData);
#pragma use_lmw_stmw reset

// Free the AX voice and recycle its entry from the active or pending list to the free list.
void gtl_AxVoicePool_ReleaseVoice(void* self, void* item);

// Remove/reset a free voice and link it into the active list while interrupts are disabled.
void* AudioVoicePool_TakeFreeVoice(void* self);

// Install chorus defaults, clamp derived delay/period parameters, and refresh initialized work memory.
void* gtl_nw4r_FxChorus_Ctor(void* self);

#pragma use_lmw_stmw on
// Notify stolen-voice callbacks and recycle pending voices into the singleton pool.
void GtlVoicePool_DrainPendingReleases(void* self);
#pragma use_lmw_stmw reset

// Optionally free the voice-pool-related allocation; no member teardown is required here.
void* dtor_804093A8(void* self, int deleting);

// Optionally free the basic-player-related allocation; no member teardown is required here.
void* dtor_80409D14(void* self, int deleting);

// Optionally free the chorus-related allocation; no member teardown is required here.
void* dtor_80409EB0(void* self, int deleting);

// Install the basic-player vtable, invalidate its identifier, and reset playback parameters.
void* gtl_nw4r_detail_BasicPlayer_Ctor(void* self);

// Restore playback gain/pitch, output sends, channel defaults, and flags.
void gtl_nw4r_detail_BasicPlayer_ResetParameters(void* self);

// Forward output-buffer configuration to the embedded audio-effect controller.
void gtl_nw4r_FxChorus_SetOutputBuffer(void* self, int forwarded1, unsigned int forwarded2);

// Clear the embedded audio-effect controller's output buffer.
void gtl_nw4r_FxChorus_ClearOutputBuffer(void* self);

// Initialize the voice-pool module's cached two-pi value.
void AudioVoicePool_InitTwoPi();

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626C4", _ctors_u80409324_0, AudioVoicePool_InitTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU804626C4", _ctors_u80409324_1, gtl::nw4r::detail::BasicPlayer::InitTwoPi);
#pragma pop

// Game entry points in original address order.
extern "C" {

void* GtlVoicePool_dtor(void* self, int deleting) {
    if (self) {
        if (((char*)self + kAudioPendingListOffset))
            gtl_IntrusiveList_Dtor(((char*)self + kAudioPendingListOffset), 0);
        if (((char*)self + kAudioFreeListOffset))
            gtl_IntrusiveList_Dtor(((char*)self + kAudioFreeListOffset), 0);
        if (self)
            gtl_IntrusiveList_Dtor(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

void* dtor_804093A8(void* self, int deleting) {
    if (self && deleting > 0)
        __dl__FPv(self);
    return self;
}

int GtlVoicePool_GetRequiredMemorySize() {
    return (AXGetMaxVoices() + kReservedVoiceCount) * kVoiceEntryBytes;
}

void AudioVoicePool_InitializeFreeList(void* self, void* entries, unsigned int size) {
    // entries is borrowed for the pool's lifetime; size is bytes and a trailing
    // partial entry is ignored. The three list headers must already be initialized.
    char* poolBytes = (char*)self;
    if (!AsAudioVoicePool(poolBytes)->initialized) {
        char* voiceBytes = (char*)entries;
        unsigned int voiceIndex = 0;
        AsAudioVoicePool(poolBytes)->capacity = size / kVoiceEntryBytes;
        if (AsAudioVoicePool(poolBytes)->capacity) {
            for (voiceIndex = 0; voiceIndex < AsAudioVoicePool(poolBytes)->capacity; ++voiceIndex) {
                void* item = (poolBytes + kAudioFreeHeadOffset);
                gtl_IntrusiveList_InsertBefore((poolBytes + kAudioFreeListOffset), &item,
                                               AsAudioVoiceEntry(voiceBytes)->links);
                voiceBytes += kVoiceEntryBytes;
            }
        }
        AsAudioVoicePool(poolBytes)->initialized = 1;
    }
}

void AudioVoicePool_Shutdown(void* self) {
    char* poolBytes = (char*)self;
    if (AsAudioVoicePool(poolBytes)->initialized) {
        // Phase 1: remove active entries, stop hardware, notify reason 0, recycle.
        while (AsAudioVoicePool(poolBytes)->active.count) {
            void* voiceLink = AsAudioVoicePool(poolBytes)->active.first;
            char* voiceBytes = AudioVoice_FromLink(voiceLink);
            IntrusiveList_UnlinkNodeAndDecrementCount(self, &voiceLink);
            void* axVoiceHandle = AsAudioVoiceEntry(voiceBytes)->axVoice;
            if (!axVoiceHandle)
                continue;
            if (((AXVoiceStateView*)axVoiceHandle)->state == kAXVoiceRunning)
                AXSetVoiceState(axVoiceHandle, kAXVoiceStopped);
            void (**callback)(void*, int, unsigned int) = &AsAudioVoiceEntry(voiceBytes)->callback;
            if (*callback)
                (*callback)(voiceBytes, kCallbackShutdown, AsAudioVoiceEntry(voiceBytes)->callbackData);
            volatile BOOL outer = OSDisableInterrupts();
            axVoiceHandle = AsAudioVoiceEntry(voiceBytes)->axVoice;
            if (axVoiceHandle)
                AXFreeVoice(axVoiceHandle);
            AudioVoiceAux_Dtor(voiceBytes, -1);
            volatile BOOL inner = OSDisableInterrupts();
            char* node = (char*)AsAudioVoiceEntry(voiceBytes)->links;
            if (AsAudioVoiceEntry(voiceBytes)->pendingRelease)
                IntrusiveList_UnlinkNode((poolBytes + kAudioPendingListOffset), node);
            else
                IntrusiveList_UnlinkNode(self, node);
            void* value = (poolBytes + kAudioFreeHeadOffset);
            gtl_IntrusiveList_InsertBefore((poolBytes + kAudioFreeListOffset), &value, node);
            OSRestoreInterrupts(inner);
            OSRestoreInterrupts(outer);
        }
        // Phase 2: process pending entries with the original active-list unlink
        // argument preserved below. Do not infer conventional list behavior here.
        while (AsAudioVoicePool(poolBytes)->pending.count) {
            void* voiceLink = AsAudioVoicePool(poolBytes)->pending.first;
            // As in the original: the unlinkPosition handed to IntrusiveList_UnlinkNodeAndDecrementCount is list 1's first
            // node (*((poolBytes + kAudioActiveHeadOffset))) and the list is self, not this node's list.
            void* unlinkPosition = AsAudioVoicePool(poolBytes)->active.first;
            IntrusiveList_UnlinkNodeAndDecrementCount(self, &unlinkPosition);
            char* voiceBytes = AudioVoice_FromLink(voiceLink);
            void* axVoiceHandle = AsAudioVoiceEntry(voiceBytes)->axVoice;
            if (!axVoiceHandle)
                continue;
            if (((AXVoiceStateView*)axVoiceHandle)->state == kAXVoiceRunning)
                AXSetVoiceState(axVoiceHandle, kAXVoiceStopped);
            void (**callback)(void*, int, unsigned int) = &AsAudioVoiceEntry(voiceBytes)->callback;
            if (*callback)
                (*callback)(voiceBytes, kCallbackShutdown, AsAudioVoiceEntry(voiceBytes)->callbackData);
            volatile BOOL outer = OSDisableInterrupts();
            axVoiceHandle = AsAudioVoiceEntry(voiceBytes)->axVoice;
            if (axVoiceHandle)
                AXFreeVoice(axVoiceHandle);
            AudioVoiceAux_Dtor(voiceBytes, -1);
            volatile BOOL inner = OSDisableInterrupts();
            char* node = (char*)AsAudioVoiceEntry(voiceBytes)->links;
            if (AsAudioVoiceEntry(voiceBytes)->pendingRelease)
                IntrusiveList_UnlinkNode((poolBytes + kAudioPendingListOffset), node);
            else
                IntrusiveList_UnlinkNode(self, node);
            void* value = (poolBytes + kAudioFreeHeadOffset);
            gtl_IntrusiveList_InsertBefore((poolBytes + kAudioFreeListOffset), &value, node);
            OSRestoreInterrupts(inner);
            OSRestoreInterrupts(outer);
        }
        // Phase 3: destroy each free entry's auxiliary state and empty the list;
        // the caller retains the entry allocation, and initialized is cleared.
        while (AsAudioVoicePool(poolBytes)->free.count) {
            void* voiceLink = AsAudioVoicePool(poolBytes)->free.first;
            IntrusiveList_UnlinkNodeAndDecrementCount((poolBytes + kAudioFreeListOffset), &voiceLink);
            AudioVoiceAux_Dtor(AudioVoice_FromLink(voiceLink), -1);
        }
        AsAudioVoicePool(poolBytes)->initialized = 0;
    }
}

void* AudioVoicePool_TakeFreeVoice(void* self) {
    // Null means no free entries. A non-null result is linked active and reset;
    // callers must eventually release it. This step alone does not acquire AX.
    char* p = (char*)self;
    volatile BOOL level = OSDisableInterrupts();
    if (!AsAudioVoicePool(p)->free.count) {
        OSRestoreInterrupts(level);
        return 0;
    }
    void* entry = AsAudioVoicePool(p)->free.first;
    LocalIter it = {entry, entry, entry, entry, entry, entry};
    IntrusiveList_UnlinkNodeAndDecrementCount((p + kAudioFreeListOffset), (void**)&it);
    void* result = AudioVoice_FromLink(entry);
    if (result) {
        try {
            AudioVoice_ResetParameters((unsigned int)result);
        } catch (...) {
            __throw(0, 0, 0);
        }
    }
    LocalIter out = {(p + kAudioActiveHeadOffset), (p + kAudioActiveHeadOffset), (p + kAudioActiveHeadOffset),
                     (p + kAudioActiveHeadOffset), (p + kAudioActiveHeadOffset), (p + kAudioActiveHeadOffset)};
    gtl_IntrusiveList_InsertBefore(self, (void**)&out, (char*)result + kVoiceLinkOffset);
    OSRestoreInterrupts(level);
    return result;
}

#pragma push
#pragma use_lmw_stmw on
void* AudioVoicePool_AcquireAXVoice(void* self, void* priority, unsigned int callbackAddress,
                                    unsigned int callbackData) {
    // Return a borrowed entry handle, or null. callbackAddress receives
    // (entry, reason, callbackData): 0 on shutdown, 1 when draining a stolen voice.
    // AX acquisition failure returns null AFTER TakeFreeVoice linked an active
    // entry; this function does not recycle it on that failure path.
    volatile BOOL level = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup0(level);
    void* item = AudioVoicePool_TakeFreeVoice(self);
    if (!item) {
        OSRestoreInterrupts(interruptCleanup0.release());
        return 0;
    }
    void* found = AXAcquireVoice(priority, (void*)GtlVoice_ReturnValuePairToPool, item);
    if (!found) {
        OSRestoreInterrupts(interruptCleanup0.release());
        return 0;
    }
    AudioVoice_InitializeVolumeEnvelopeState(item, found);
    *(unsigned int*)&AsAudioVoiceEntry(item)->callback = callbackAddress;
    *(unsigned int*)&AsAudioVoiceEntry(item)->callbackData = callbackData;
    OSRestoreInterrupts(interruptCleanup0.release());
    return item;
}
#pragma pop

void gtl_AxVoicePool_ReleaseVoice(void* self, void* item) {
    volatile BOOL level = OSDisableInterrupts();
    if (AsAudioVoiceEntry(item)->axVoice)
        AXFreeVoice(AsAudioVoiceEntry(item)->axVoice);
    AudioVoiceAux_Dtor(item, -1);
    volatile BOOL inner = OSDisableInterrupts();
    char* p = (char*)AsAudioVoiceEntry(item)->links;
    if (AsAudioVoiceEntry(item)->pendingRelease)
        IntrusiveList_UnlinkNode(((char*)self + kAudioPendingListOffset), p);
    else
        IntrusiveList_UnlinkNode(self, p);
    void* value = ((char*)self + kAudioFreeHeadOffset);
    gtl_IntrusiveList_InsertBefore(((char*)self + kAudioFreeListOffset), &value, p);
    OSRestoreInterrupts(inner);
    OSRestoreInterrupts(level);
}

void AudioVoicePool_QueuePendingRelease(void* pool, void* voice) {
    volatile BOOL old0 = OSDisableInterrupts();
    AsAudioVoiceEntry(voice)->pendingRelease = 1;
    volatile BOOL old1 = OSDisableInterrupts();
    char* node = (char*)AsAudioVoiceEntry(voice)->links;
    IntrusiveList_UnlinkNode(pool, node);
    void* ptr = ((char*)pool + kAudioPendingHeadOffset);
    gtl_IntrusiveList_InsertBefore(((char*)pool + kAudioPendingListOffset), &ptr, node);
    OSRestoreInterrupts(old1);
    OSRestoreInterrupts(old0);
}

#pragma push
#pragma use_lmw_stmw on
void GtlVoicePool_DrainPendingReleases(void* self) {
    char* poolBytes = (char*)self;
    if (AsAudioVoicePool(poolBytes)->pending.count)
        do {
            void* voiceLink = AsAudioVoicePool(poolBytes)->pending.first;
            char* voiceBytes = AudioVoice_FromLink(voiceLink);
            // Notify before releasing AX/auxiliary state: callback can inspect entry.
            if ((void*)AsAudioVoiceEntry(voiceBytes)->callback) {
                void (*onVoiceStolen)(void*, int, void*) =
                    (void (*)(void*, int, void*))AsAudioVoiceEntry(voiceBytes)->callback;
                onVoiceStolen(voiceBytes, kCallbackStolen, (void*)AsAudioVoiceEntry(voiceBytes)->callbackData);
            }
            // Lazily construct the destination singleton's three empty sentinels.
            if (!s_audioVoicePoolInitialized) {
                char* head = s_audioVoicePool;
                char* first = head + 4;
                char* second = head + kAudioFreeHeadOffset;
                char* third = head + kAudioPendingHeadOffset;
                *(void**)(head + 4) = 0;
                *(void**)(head + 8) = 0;
                *(void**)(head + kAudioFreeHeadOffset) = 0;
                *(void**)(head + 0x14) = 0;
                *(void**)(head + kAudioPendingHeadOffset) = 0;
                *(void**)(head + 0x20) = 0;
                *(void**)head = 0;
                *(void**)(head + 4) = first;
                *(void**)(head + 8) = first;
                *(void**)(head + kAudioFreeListOffset) = 0;
                *(void**)(head + kAudioFreeHeadOffset) = second;
                *(void**)(head + 0x14) = second;
                *(void**)(head + kAudioPendingListOffset) = 0;
                *(void**)(head + kAudioPendingHeadOffset) = third;
                *(void**)(head + 0x20) = third;
                *(unsigned char*)(head + 0x24) = 0;
                __register_global_object(head, (void*)GtlVoicePool_dtor, s_audioVoicePoolDtorRegistration);
                s_audioVoicePoolInitialized = 1;
            }
            // Recycle into the SINGLETON pool. self must identify that same pool:
            // the loop condition reads self while the unlink/insert use the singleton.
            volatile BOOL outerInterruptLevel = OSDisableInterrupts();
            if (AsAudioVoiceEntry(voiceBytes)->axVoice)
                AXFreeVoice(AsAudioVoiceEntry(voiceBytes)->axVoice);
            AudioVoiceAux_Dtor(voiceBytes, -1);
            volatile BOOL innerInterruptLevel = OSDisableInterrupts();
            if (AsAudioVoiceEntry(voiceBytes)->pendingRelease)
                IntrusiveList_UnlinkNode((s_audioVoicePool + kAudioPendingListOffset), (voiceBytes + kVoiceLinkOffset));
            else
                IntrusiveList_UnlinkNode(s_audioVoicePool, (voiceBytes + kVoiceLinkOffset));
            void* freeListPosition = (s_audioVoicePool + kAudioFreeHeadOffset);
            gtl_IntrusiveList_InsertBefore((s_audioVoicePool + kAudioFreeListOffset), &freeListPosition,
                                           (voiceBytes + kVoiceLinkOffset));
            OSRestoreInterrupts(innerInterruptLevel);
            OSRestoreInterrupts(outerInterruptLevel);
        } while (AsAudioVoicePool(poolBytes)->pending.count);
}
#pragma pop

void AudioVoicePool_InitTwoPi() {
    s_voicePoolTwoPi = kFloat_6_2831855;
}

void* gtl_nw4r_detail_BasicPlayer_Ctor(void* self) {
    AsGtlBasicPlayer(self)->unk_4 = kBasicPlayerInvalidId;
    AsGtlBasicPlayer(self)->vtbl = __vt__Q43gtl4nw4r6detail11BasicPlayer;
    gtl_nw4r_detail_BasicPlayer_ResetParameters(self);
    return self;
}

void gtl_nw4r_detail_BasicPlayer_ResetParameters(void* self) {
    char* playerBytes = (char*)self;
    float resetOffset = kBasicPlayerResetOffset;
    float resetScale = kBasicPlayerResetScale;
    AsGtlBasicPlayer(playerBytes)->pan = resetOffset;
    AsGtlBasicPlayer(playerBytes)->volumeScale = resetScale;
    AsGtlBasicPlayer(playerBytes)->pitchScale = resetScale;
    AsGtlBasicPlayer(playerBytes)->surroundPan = resetOffset;
    AsGtlBasicPlayer(playerBytes)->panOffset = resetOffset;
    AsGtlBasicPlayer(playerBytes)->surroundPanOffset = resetOffset;
    AsGtlBasicPlayer(playerBytes)->lowPassFilterOffset = resetOffset;
    AsGtlBasicPlayer(playerBytes)->biquadFilterSelector = 0;
    AsGtlBasicPlayer(playerBytes)->outputLineFlags = kBasicPlayerMainOutput;
    AsGtlBasicPlayer(playerBytes)->mainSend = resetOffset;
    AsGtlBasicPlayer(playerBytes)->mainOutputVolume = resetScale;
    AsGtlBasicPlayer(playerBytes)->auxiliarySends[0] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->auxiliarySends[1] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->auxiliarySends[2] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->remoteVolumes[0] = resetScale;
    AsGtlBasicPlayer(playerBytes)->remoteSendOffsets[0] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->additionalOutputMixControls[0] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->remoteVolumes[1] = resetScale;
    AsGtlBasicPlayer(playerBytes)->remoteSendOffsets[1] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->additionalOutputMixControls[1] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->remoteVolumes[2] = resetScale;
    AsGtlBasicPlayer(playerBytes)->remoteSendOffsets[2] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->additionalOutputMixControls[2] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->remoteVolumes[3] = resetScale;
    AsGtlBasicPlayer(playerBytes)->remoteSendOffsets[3] = resetOffset;
    AsGtlBasicPlayer(playerBytes)->additionalOutputMixControls[3] = resetOffset;
}

} // extern "C"

void gtl::nw4r::detail::BasicPlayer::SetAuxiliarySend(int index, float value) {
    auxiliarySends[index] = value;
}

float gtl::nw4r::detail::BasicPlayer::GetAuxiliarySend(int index) {
    return auxiliarySends[index];
}

void gtl::nw4r::detail::BasicPlayer::SetRemoteVolume(int index, float value) {
    remoteVolumes[index] = value;
}

float gtl::nw4r::detail::BasicPlayer::GetRemoteVolume(int index) {
    return remoteVolumes[index];
}

void gtl::nw4r::detail::BasicPlayer::SetRemoteSendOffset(int index, float value) {
    remoteSendOffsets[index] = value;
}

float gtl::nw4r::detail::BasicPlayer::GetRemoteSendOffset(int index) {
    return remoteSendOffsets[index];
}

void gtl::nw4r::detail::BasicPlayer::SetAdditionalOutputMixControl(int index, float value) {
    additionalOutputMixControls[index] = value;
}

float gtl::nw4r::detail::BasicPlayer::GetAdditionalOutputMixControl(int index) {
    return additionalOutputMixControls[index];
}

extern "C" {

void* dtor_80409D14(void* self, int deleting) {
    if (self && deleting > 0)
        __dl__FPv(self);
    return self;
}

} // extern "C"

// Initialize the basic-player module's cached two-pi value.
void gtl::nw4r::detail::BasicPlayer::InitTwoPi() {
    s_basicPlayerTwoPi = kFloat_6_2831855;
}

extern "C" {

void* gtl_nw4r_FxChorus_Ctor(void* self) {
    // delay is converted to the SDK integer delay; depth is delay*depthRatio.
    // period is 1000/rateParameter before clamping (milliseconds for a Hz rate).
    char* p = (char*)self;
    AsGtlFxChorus(p)->unk_4 = 0;
    AsGtlFxChorus(p)->unk_8 = 0;
    *(void**)p = __vt__Q33gtl4nw4r8FxChorus;
    AsGtlFxChorus(p)->initializationAttempted = 0;
    AsGtlFxChorus(p)->outputBuffer = 0;
    AsGtlFxChorus(p)->unk_14 = 0;
    unsigned int* source = kChorusDefaultParameters;
    unsigned int values[3] = {source[0], source[1], source[2]};
    AsGtlFxChorus(p)->delayParameter = *(float*)&values[0];
    AsGtlFxChorus(p)->depthRatio = *(float*)&values[1];
    AsGtlFxChorus(p)->rateParameter = *(float*)&values[2];
    unsigned long delay = Runtime_ConvertDoubleToUnsigned(*(float*)&values[0]);
    unsigned long depth = Runtime_ConvertDoubleToUnsigned(*(float*)&values[0] * *(float*)&values[1]);
    unsigned long period = Runtime_ConvertDoubleToUnsigned(kFloat_1000_FxChorus / *(float*)&values[2]);
    AsGtlFxChorus(p)->clampedDelay =
        delay > kChorusMaximumDelay ? kChorusMaximumDelay : (delay < kChorusMinimumDelay ? kChorusMinimumDelay : delay);
    AsGtlFxChorus(p)->clampedDepth = depth > kChorusMaximumDelay ? kChorusMaximumDelay : depth;
    AsGtlFxChorus(p)->clampedPeriod = period > kChorusMaximumPeriod
                                          ? kChorusMaximumPeriod
                                          : (period < kChorusMinimumPeriod ? kChorusMinimumPeriod : period);
    if (AsGtlFxChorus(p)->initializationAttempted) {
        unsigned int savedAllocator, savedFree;
        gtl_AudioEffects_InstallAllocatorHooks((p + kChorusControllerOffset), &savedAllocator, &savedFree);
        Chorus_ReconfigureAxfxState((p + kChorusWorkOffset));
        gtl_AudioEffects_RestoreAllocatorHooks((p + kChorusControllerOffset), savedAllocator, savedFree);
    }
    return self;
}

void* dtor_80409EB0(void* self, int deleting) {
    if (self && deleting > 0)
        __dl__FPv(self);
    return self;
}

void gtl_nw4r_FxChorus_SetOutputBuffer(void* self, int forwarded1, unsigned int forwarded2) {
    AudioEffect_SetOutputBuffer(((char*)&AsGtlFxChorus(self)->initializationAttempted), forwarded1, forwarded2);
}

void gtl_nw4r_FxChorus_ClearOutputBuffer(void* self) {
    AudioController_ClearOutputBuffer(((char*)&AsGtlFxChorus(self)->initializationAttempted));
}

int gtl_nw4r_FxChorus_Initialize(void* self) {
    char* effectBytes = (char*)self;
    if (AsGtlFxChorus(effectBytes)->initializationAttempted)
        return 1;
    unsigned int savedAllocateHook;
    unsigned int savedFreeHook;
    gtl_AudioEffects_InstallAllocatorHooks(effectBytes + kChorusControllerOffset, &savedAllocateHook, &savedFreeHook);
    g_audioEffectAllocatedBytes = 0;
    int result = AXFXChorusInit(effectBytes + kChorusWorkOffset);
    gtl_AudioEffects_RestoreAllocatorHooks(effectBytes + kChorusControllerOffset, savedAllocateHook, savedFreeHook);
    *(unsigned char*)(effectBytes + kChorusControllerOffset) = 1;
    return result != 0;
}
} // extern "C"
