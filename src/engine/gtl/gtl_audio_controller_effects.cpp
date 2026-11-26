// Audio controller effects: the three auxiliary bus ramps, attaching, fading and clearing
// effects, callback lists, and the controller's update and output mode.

#include <decomp/static_init.h>
#include <decomp/offset_of.h>
#include <decomp/local_class.h>
#pragma push
#include "classes/GtlAudioCallbackNode.h"
#include "classes/GtlAudioControllerState.h"

#include "sdk_protos.h"

enum {
    kAudioChannelCount = 3,
    kAuxA = 0,
    kAuxB = 1,
    kAuxC = 2,
    kStereoOutput = 1,
    kSurroundOutput = 2,
    kFourBufferOutput = 3,
    kEffectBlockBytes = 0x180,
    kInitialOutputBytes = 0x100,
    kAudioPoolBytes = 0xA0000,
    kAudioPoolAlignment = 4,
    kSilentCallbacksAfterAttach = 2,
    kUnityGainFixed = 0x8000,
    kVoiceDriveStatusChanged = 2,
    kVoiceMasterGainChanged = 8,
    kVoiceOutputModeChanged = 16,
    kDvdReady = 0,
    kDvdBusy = 1,
    kAudioTicksPerCallback = 3
};

// index maps directly to AX auxiliary buses: 0=A, 1=B, 2=C (not speaker lanes).
// Each bus has two 16-byte ramp entries: fade at self+0x54+16*index and scale
// at self+0x84+16*index. Callback lists start at +0xB4 with a 12-byte stride.
// A list is {count, circular link, circular link}; the sentinel is list+4.
// Effects embed their list link at object+4 and remain caller-owned.
// Ramp duration and frame count calls to Update. FadeOutEffects converts caller
// ticks by ceil(ticks/3); the caller tick's physical unit is not established here.
// Gains are linear [0,1], converted to AX fixed point with unity=0x8000.
struct AudioRamp {
    float start, end;
    int duration, frame;
};
enum {
    kAudioRampBytes = sizeof(AudioRamp),
    kAudioChannelListBytes = sizeof(GtlAudioCallbackList),
    kChannelFadeOffset = 0x54,
    kChannelScaleOffset = 0x84,
    kEffectLinkOffset = 4,
    kEffectStopVtableSlot = 5,
    kEffectProcessVtableSlot = 8
};
// This strided view models one bus's two ramps, not a shifted controller object.
// Adding 16*bus to the view selects fade[bus] and scale[bus] at the original offsets.
struct AudioBusRampView {
    char beforeFade[kChannelFadeOffset];
    AudioRamp fade;
    char betweenRampTables[kChannelScaleOffset - kChannelFadeOffset - sizeof(AudioRamp)];
    AudioRamp scale;
};
struct AudioControllerPoolView {
    GtlAudioControllerState controller;
    unsigned char blockPool[1]; // Opaque subobject initialized by BitmapBlockPool_Initialize.
};
typedef char AudioPool_offset[((int)DECOMP_OFFSETOF(AudioControllerPoolView, blockPool) == 0xF4) ? 1 : -1];
#define AsAudioPool(p) ((AudioControllerPoolView*)(p))

// Compiler-generated unsigned conversions use this ABI name; the original
// runtime is retained under its address name in auto_03_803669D0_text.o.
// Forward the compiler conversion ABI to the retained unsigned-conversion runtime.
extern "C" unsigned long __cvt_fp2unsigned(double value) {
    return Runtime_ConvertDoubleToUnsigned(value);
}

extern "C" {

struct InterruptUnwindCleanup {
    BOOL level;
    bool active;
    // Arrange to restore the caller's interrupt state during exception unwinding.
    explicit InterruptUnwindCleanup(BOOL saved) : level(saved), active(true) {}
    // Transfer restoration responsibility to the explicit normal-exit path.
    BOOL release() {
        active = false;
        return level;
    }
    // Restore interrupts unless a normal-exit path has already taken responsibility.
    ~InterruptUnwindCleanup() {
        if (active)
            OSRestoreInterrupts(level);
    }
};

extern float s_twoPi_AudioEffects;
extern float kFloat_6_2831855;
extern float s_twoPi_AudioController;
extern unsigned char* g_audioEffectAllocatedBytes;
extern unsigned int g_audioEffectsAllocatorContext;
extern volatile signed char s_audioControllerInitialized;
extern unsigned char s_gtlAudioControllerRegistration[];
extern unsigned char s_gtlAudioController[];
extern float kAudioEffectSampleRateHz;

void AXFXSetHooks(void*, void*);
void AXFXGetHooks(void*, void*);
void __dl__FPv(void*);
void gtl_IntrusiveList_Dtor(void*, int);
void IntrusiveList_UnlinkNode(void*, void*);
void InterruptGuard_dtor(void*);
void* gtl_AudioEffects_Alloc(unsigned int);
void gtl_AudioEffects_Free(void*);
void gtl_AudioController_InitMembers(void*);
void* __destroy_arr(void*, void*, int, int);
void* gtl_AudioController_Dtor(void*, int);
void gtl_IntrusiveList_InsertBefore(void*, void*, void*);
void AXSetMode(int);
void AXSetAuxAReturnVolume(unsigned int);
void AXSetAuxBReturnVolume(unsigned int);
void AXSetAuxCReturnVolume(unsigned int);
void* gtl_VoicePool_GetInstance(void);
void gtl_AudioController_MarkActiveVoices(void*, int);
void* AXRegisterCallback(void*);
void gtl_AudioController_ClearEffects(void*, int);
void gtl_AudioController_ProcessEffects(void*, int);
void AXRegisterAuxACallback(void*, void*);
void AXRegisterAuxBCallback(void*, void*);
void AXRegisterAuxCCallback(void*, void*);
void BitmapBlockPool_ResetStorage(void*);
void gtl_Audio_Free(void*);
extern unsigned int g_audioBlockPoolBuffer;
extern float kFloat_0_AudioValueAnimation;
extern unsigned char s_audioInitialOutputBuffer[];
void* gtl_Audio_Alloc(unsigned int);
void BitmapBlockPool_Initialize(void*, void*, unsigned int, unsigned int);
void AXGetAuxACallback(void*, void*);
void AXGetAuxBCallback(void*, void*);
void AXGetAuxCCallback(void*, void*);
void gtl_AudioEffectList_UnlinkAll(void*);
void* memset(void*, int, unsigned int);
void gtl_AudioController_DispatchCallbacks(void);
int DVDGetDriveStatus(void);
void AXSetMasterVolume(unsigned int);

// Initialize the audio-effect helper's full-turn constant.
void AudioEffects_InitializeAngleConstant(void);

// Set the controller output buffer and report success.
// outputBuffer is a borrowed address encoded as a word; the middle ABI argument is unused.
int AudioEffect_SetOutputBuffer(void* p, int, unsigned int outputBuffer);

// Clear the controller's external output-buffer pointer.
void AudioController_ClearOutputBuffer(void* p);

// Restore the saved AXFX allocation hooks and clear the allocator context.
void gtl_AudioEffects_RestoreAllocatorHooks(void* unusedContext, void* savedAllocate, void* savedFree);

// Save existing AXFX hooks and install the controller's allocation wrappers.
void gtl_AudioEffects_InstallAllocatorHooks(void* context, void* outAllocateHook, void* outFreeHook);

void gtl_Audio_Free(void*);

// Release effect storage through the audio allocator.
void gtl_AudioEffects_Free(void* p);

// Optionally release the helper's storage.
void* dtor_80405334(void* self, int flag);

// Destroy the intrusive-list subobject and optionally release its storage.
void* dtor_80405374(void* self, int flag);

// Destroy one channel callback list during controller cleanup or unwinding.
void* AudioChannelCallbackList_Dtor(void* self, int flag);

// Initialize the audio-controller helper's full-turn constant.
void AudioController_InitializeAngleConstant(void);

// Account for aligned effect bytes and request audio-allocator storage.
// g_audioEffectAllocatedBytes is used as a byte counter despite its pointer type.
// Return the allocated address to AXFX; the byte counter tracks aligned requested bytes.
void* gtl_AudioEffects_Alloc(unsigned int size);

// Initialize a channel list with a circular sentinel and zero entries.
void gtl_AudioChannelList_InitEmpty(volatile unsigned int* list);

extern void __construct_array(void*, void (*)(volatile unsigned int*), void* (*)(void*, int), int, int);

// Initialize unity gain ramps, callback sentinels, and saved effect hooks.
void gtl_AudioController_InitMembers(void* self);

// Return the configured output-buffer address.
unsigned int gtl_AudioController_GetOutputBuffer(void* controller);

// Return the controller's output routing mode.
unsigned int gtl_AudioController_GetOutputMode(void* controller);

// Insert a callback node while interrupts are disabled.
// node is a borrowed intrusive node; callback is callback(node). Keep both alive
// until UnregisterCallback removes the same node. No allocation or retain occurs.
void gtl_AudioController_RegisterCallback(void* self, void* node, void* callback);

// Map controller routing to AX mode and mark voices for refresh.
// Stored modes 0/1 -> AX stereo (0), 2 -> surround (1), 3 -> four-buffer (2).
// Other codes are still stored/marked but do not issue AXSetMode.
void gtl_AudioController_SetOutputMode(int* p, int mode);

// Restore previous callbacks, detach effects, and release the audio block pool.
void gtl_AudioController_Shutdown(void* self);

// Unlink a callback node while interrupts are disabled.
void gtl_AudioController_UnregisterCallback(void* self, void* node);

// Lazily initialize and register destruction of the audio-controller singleton.
void* gtl_AudioController_GetInstance(void);

// Interpolate the master gain ramp, returning its endpoint when complete.
float gtl_AudioController_GetMasterGain(const void* controller);

// Destroy channel lists and controller callbacks before optional deletion.
void* gtl_AudioController_Dtor(void* self, int flag);

// Start a channel fade from its current gain or detach immediately for zero ticks.
// Requires index in [0,3) and nonnegative ticks; no range validation is performed.
void gtl_AudioController_FadeOutEffects(void* self, int index, int ticks);

// Stop channel effects, unlink them, and clear the AX callback under interrupt protection.
// Calls each effect's stop hook (vtable slot 5), but does not delete the effect.
void gtl_AudioController_ClearEffects(void* self, int index);

// Allocate audio storage and install dispatch/effect callbacks once.
void gtl_AudioController_Initialize(void* self);

// Run registered callbacks and then chain to the previous AX callback.
void gtl_AudioController_DispatchCallbacks(void);

// Advance gain ramps, handle drive-status changes, and submit fixed-point output gains.
void gtl_AudioController_Update(void* self);

// Attach an effect, reset its channel fade, and prime two silent callbacks.
// Requires index in [0,3) and an unlinked, live effect. Returns 1; no retain occurs.
int gtl_AudioController_AttachEffect(void* self, int index, void* effect);

// Clear priming/empty buffers or process each effect through its channel callback.
// AX supplies three lanes (four for mode 3), each kEffectBlockBytes bytes; lane
// order passes through unchanged. Speaker identities are not established here.
// Slot 8 receives (effect, laneCount, lanePointers, bytesPerLane, sampleRateHz,
// 0, outputMode). Buffers are borrowed for this call; effects run in list order.
void gtl_AudioController_ProcessEffects(void* values, int index);

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626B0", _ctors_u80404DCC_0, AudioEffects_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU804626B0", _ctors_u80404DCC_1, AudioController_InitializeAngleConstant);
#pragma pop

#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
void AudioEffects_InitializeAngleConstant(void) {
    s_twoPi_AudioEffects = kFloat_6_2831855;
}
#pragma pop

#pragma push
int AudioEffect_SetOutputBuffer(void* p, int, unsigned int outputBuffer) {
    AsAudioController80404E94(p)->outputBuffer = outputBuffer;
    return 1;
}
#pragma pop

#pragma push
void AudioController_ClearOutputBuffer(void* p) {
    AsAudioController80404E94(p)->outputBuffer = 0;
}
#pragma pop

#pragma push
void gtl_AudioEffects_InstallAllocatorHooks(void* context, void* outAllocateHook, void* outFreeHook) {
    AXFXGetHooks(outAllocateHook, outFreeHook);
    AXFXSetHooks((void*)gtl_AudioEffects_Alloc, (void*)gtl_AudioEffects_Free);
    g_audioEffectsAllocatorContext = (unsigned int)context;
}
#pragma pop

#pragma push
void gtl_AudioEffects_RestoreAllocatorHooks(void* unusedContext, void* savedAllocate, void* savedFree) {
    AXFXSetHooks(savedAllocate, savedFree);
    g_audioEffectsAllocatorContext = 0;
}
#pragma pop

#pragma push
void* gtl_AudioEffects_Alloc(unsigned int size) {
    g_audioEffectAllocatedBytes += (size + 3) & ~3;
    return gtl_Audio_Alloc(size);
}
#pragma pop

#pragma push
void gtl_AudioEffects_Free(void* p) {
    gtl_Audio_Free(p);
}
#pragma pop

#pragma push
void AudioController_InitializeAngleConstant(void) {
    s_twoPi_AudioController = kFloat_6_2831855;
}
#pragma pop

#pragma push
void gtl_AudioController_InitMembers(void* self) {
    // Initialize controller flags and its non-owning dispatch-callback sentinel.
    char* p = (char*)self;
    AsAudioController80404E94(p)->mode = kStereoOutput;
    AsAudioController80404E94(p)->outputBuffer = 0;
    AsAudioController80404E94(p)->callbackCount = 0;
    enum {
        callbackSentinelOffset = (int)DECOMP_OFFSETOF(GtlAudioControllerState, callbackFirst)
    };
    AsAudioController80404E94(p)->callbackFirst = (unsigned int)(p + callbackSentinelOffset);
    AsAudioController80404E94(p)->callbackLast = (unsigned int)(p + callbackSentinelOffset);
    AsAudioController80404E94(p)->initialized = 0;
    AsAudioController80404E94(p)->unk_19 = 1;
    AsAudioController80404E94(p)->driveStatusChanged = 0;
    // Clear every ramp/counter first; unity endpoints are assigned below.
    AsAudioController80404E94(p)->masterStart = 0.0f;
    AsAudioController80404E94(p)->masterEnd = 0.0f;
    (*(unsigned int*)&AsAudioController80404E94(p)->masterDuration) = 0;
    (*(unsigned int*)&AsAudioController80404E94(p)->masterFrame) = 0;
    AsAudioController80404E94(p)->masterFadeStart = 0.0f;
    AsAudioController80404E94(p)->masterFadeEnd = 0.0f;
    (*(unsigned int*)&AsAudioController80404E94(p)->masterFadeDuration) = 0;
    (*(unsigned int*)&AsAudioController80404E94(p)->masterFadeFrame) = 0;
    AsAudioController80404E94(p)->masterScaleStart = 0.0f;
    AsAudioController80404E94(p)->masterScaleEnd = 0.0f;
    (*(unsigned int*)&AsAudioController80404E94(p)->masterScaleDuration) = 0;
    (*(unsigned int*)&AsAudioController80404E94(p)->masterScaleFrame) = 0;
    AsAudioController80404E94(p)->updateCallback = 0;
    AsAudioController80404E94(p)->stopCountdown = -1;
    AsAudioController80404E94(p)->channelFade0Start = 0.0f;
    AsAudioController80404E94(p)->channelFade0End = 0.0f;
    (*(unsigned int*)&AsAudioController80404E94(p)->channelFade0Duration) = 0;
    (*(unsigned int*)&AsAudioController80404E94(p)->channelFade0Frame) = 0;
    for (int i = 0; i < 2; ++i) {
        char* q = p + kChannelFadeOffset + kAudioRampBytes + i * kAudioRampBytes;
        for (int j = 0; j < 4; ++j)
            ((unsigned int*)q)[j] = 0;
    }
    for (int i = 0; i < kAudioChannelCount; ++i) {
        char* q = p + kChannelScaleOffset + i * kAudioRampBytes;
        ((float*)q)[0] = 0;
        ((float*)q)[1] = 0;
        ((unsigned int*)q)[2] = 0;
        ((unsigned int*)q)[3] = 0;
    }
    __construct_array(AsAudioController80404E94(p)->channelLists, gtl_AudioChannelList_InitEmpty,
                      AudioChannelCallbackList_Dtor, 0xC, 3);
    // With duration=frame=0, interpolation returns each unity endpoint immediately.
    AsAudioController80404E94(p)->masterFadeStart = 1.0f;
    AsAudioController80404E94(p)->masterFadeEnd = 1.0f;
    AsAudioController80404E94(p)->masterStart = 1.0f;
    AsAudioController80404E94(p)->masterEnd = 1.0f;
    AsAudioController80404E94(p)->masterScaleStart = 1.0f;
    AsAudioController80404E94(p)->masterScaleEnd = 1.0f;
    AsAudioController80404E94(p)->channelFade0Start = 1.0f;
    AsAudioController80404E94(p)->channelFade0End = 1.0f;
    // 0x804052B8/0x804052BC: the original also sets 0x84/0x88 (first entry of the second fade table) to 1.0;
    // leaving them 0.0 made channel 0's fade start from silence.
    AsAudioController80404E94(p)->channelScale0Start = 1.0f;
    AsAudioController80404E94(p)->channelScale0End = 1.0f;
    AsAudioController80404E94(p)->channelFade1Start = 1.0f;
    AsAudioController80404E94(p)->channelFade1End = 1.0f;
    AsAudioController80404E94(p)->channelFade2Start = 1.0f;
    AsAudioController80404E94(p)->channelFade2End = 1.0f;
    AsAudioController80404E94(p)->channelScale1Start = 1.0f;
    AsAudioController80404E94(p)->channelScale1End = 1.0f;
    AsAudioController80404E94(p)->channelScale2Start = 1.0f;
    AsAudioController80404E94(p)->channelScale2End = 1.0f;
    AsAudioController80404E94(p)->savedEffectCallback0 = 0;
    AsAudioController80404E94(p)->savedEffectCallback1 = 0;
    AsAudioController80404E94(p)->savedEffectCallback2 = 0;
    AsAudioController80404E94(p)->savedEffectContext0 = 0;
    AsAudioController80404E94(p)->savedEffectContext1 = 0;
    AsAudioController80404E94(p)->savedEffectContext2 = 0;
}
#pragma pop

#pragma push
void* dtor_80405334(void* self, int flag) {
    if (self && flag > 0)
        __dl__FPv(self);
    return self;
}
#pragma pop

#pragma push
void* dtor_80405374(void* self, int flag) {
    if (self) {
        gtl_IntrusiveList_Dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
void gtl_AudioChannelList_InitEmpty(volatile unsigned int* list) {
    unsigned int* end = (unsigned int*)list + 1;
    unsigned int zero = 0;
    list[1] = zero;
    list[2] = zero;
    list[0] = zero;
    list[1] = (unsigned int)end;
    list[2] = (unsigned int)end;
}
#pragma pop

#pragma push
void* AudioChannelCallbackList_Dtor(void* self, int flag) {
    if (self) {
        gtl_IntrusiveList_Dtor(self, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
void* gtl_AudioController_GetInstance(void) {
    if (!s_audioControllerInitialized) {
        gtl_AudioController_InitMembers(s_gtlAudioController);
        __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                 s_gtlAudioControllerRegistration);
        s_audioControllerInitialized = 1;
    }
    return s_gtlAudioController;
}
#pragma pop

#pragma push
void* gtl_AudioController_Dtor(void* self, int flag) {
    if (self) {
        __destroy_arr(&AsAudioController80404E94(self)->channelLists, (void*)AudioChannelCallbackList_Dtor, 12, 3);
        if (&AsAudioController80404E94(self)->callbackCount)
            gtl_IntrusiveList_Dtor(&AsAudioController80404E94(self)->callbackCount, 0);
        if (flag > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
void gtl_AudioController_Initialize(void* self) {
    if (!AsAudioController80404E94(self)->initialized) {
        // Publish flushed silent output/pool buffers before installing AX callbacks.
        memset(s_audioInitialOutputBuffer, 0, kInitialOutputBytes);
        DCFlushRange(s_audioInitialOutputBuffer, kInitialOutputBytes);
        (*(void**)&AsAudioController80404E94(self)->outputBuffer) = s_audioInitialOutputBuffer;
        unsigned int size = kAudioPoolBytes;
        void* buffer = gtl_Audio_Alloc(size);
        g_audioBlockPoolBuffer = (unsigned int)buffer;
        memset((void*)g_audioBlockPoolBuffer, 0, size);
        DCFlushRange((void*)g_audioBlockPoolBuffer, size);
        BitmapBlockPool_Initialize(AsAudioPool(self)->blockPool, (void*)g_audioBlockPoolBuffer, size,
                                   kAudioPoolAlignment);
        // Save all three previous aux callbacks atomically with the dispatch swap.
        volatile unsigned int level;
        volatile unsigned int* saved_level = &level;
        *saved_level = OSDisableInterrupts();
        AXGetAuxACallback(&AsAudioController80404E94(self)->savedEffectCallback0,
                          &AsAudioController80404E94(self)->savedEffectContext0);
        AXGetAuxBCallback(&AsAudioController80404E94(self)->savedEffectCallback1,
                          &AsAudioController80404E94(self)->savedEffectContext1);
        AXGetAuxCCallback(&AsAudioController80404E94(self)->savedEffectCallback2,
                          &AsAudioController80404E94(self)->savedEffectContext2);
        AXRegisterAuxACallback(0, 0);
        AXRegisterAuxBCallback(0, 0);
        AXRegisterAuxCCallback(0, 0);
        void* callback = (void*)gtl_AudioController_DispatchCallbacks;
        void* task = AXRegisterCallback(callback);
        AsAudioController80404E94(self)->previousCallback = task;
        AsAudioController80404E94(self)->initialized = 1;
        OSRestoreInterrupts(*saved_level);
    }
}
#pragma pop

#pragma push
void gtl_AudioController_Shutdown(void* self) {
    if (AsAudioController80404E94(self)->initialized) {
        // Restore the dispatch chain before detaching each bus's borrowed effects.
        AXRegisterCallback(AsAudioController80404E94(self)->previousCallback);
        gtl_AudioController_ClearEffects(self, 0);
        gtl_AudioController_ClearEffects(self, 1);
        gtl_AudioController_ClearEffects(self, 2);
        AXRegisterAuxACallback((*(void**)&AsAudioController80404E94(self)->savedEffectCallback0),
                               (*(void**)&AsAudioController80404E94(self)->savedEffectContext0));
        AXRegisterAuxBCallback((*(void**)&AsAudioController80404E94(self)->savedEffectCallback1),
                               (*(void**)&AsAudioController80404E94(self)->savedEffectContext1));
        AXRegisterAuxCCallback((*(void**)&AsAudioController80404E94(self)->savedEffectCallback2),
                               (*(void**)&AsAudioController80404E94(self)->savedEffectContext2));
        AsAudioController80404E94(self)->savedEffectCallback0 = 0;
        AsAudioController80404E94(self)->savedEffectContext0 = 0;
        AsAudioController80404E94(self)->savedEffectCallback1 = 0;
        AsAudioController80404E94(self)->savedEffectContext1 = 0;
        AsAudioController80404E94(self)->savedEffectCallback2 = 0;
        AsAudioController80404E94(self)->savedEffectContext2 = 0;
        // All callbacks now belong to their prior owners; release our pool storage.
        BitmapBlockPool_ResetStorage(AsAudioPool(self)->blockPool);
        gtl_Audio_Free((void*)g_audioBlockPoolBuffer);
        g_audioBlockPoolBuffer = 0;
        AsAudioController80404E94(self)->outputBuffer = 0;
        AsAudioController80404E94(self)->initialized = 0;
    }
}
#pragma pop

#pragma push
float gtl_AudioController_GetMasterGain(const void* controller) {
    struct MasterRampView {
        char pad[0x1C];
        float start;
        float end;
        int duration;
        int current;
    };
    const MasterRampView* t = (const MasterRampView*)controller;
    if (t->current >= t->duration)
        return t->end;
    return t->start + ((float)t->current * (t->end - t->start)) / (float)t->duration;
}
#pragma pop

#pragma push
void gtl_AudioController_Update(void* self) {
    // Drive transitions mark voices once when entering/leaving the nonready state.
    int status = DVDGetDriveStatus();
    if (status == kDvdReady) {
        if (AsAudioController80404E94(self)->driveStatusChanged) {
            AsAudioController80404E94(self)->driveStatusChanged = 0;
            gtl_AudioController_MarkActiveVoices(gtl_VoicePool_GetInstance(), kVoiceDriveStatusChanged);
        }
    } else if (status != kDvdBusy && !AsAudioController80404E94(self)->driveStatusChanged) {
        AsAudioController80404E94(self)->driveStatusChanged = 1;
        gtl_AudioController_MarkActiveVoices(gtl_VoicePool_GetInstance(), kVoiceDriveStatusChanged);
    }
    // Advance each auxiliary bus's scale/fade once and submit their clamped product.
    char* entry;
    int i;
    entry = (char*)self;
    for (i = 0; i < kAudioChannelCount; ++i, entry += kAudioRampBytes) {
        int duration = ((AudioBusRampView*)entry)->scale.duration;
        int current = ((AudioBusRampView*)entry)->scale.frame;
        int active = 0;
        if (current < duration) {
            if (current < duration)
                ((AudioBusRampView*)entry)->scale.frame = current + 1;
            active = 1;
        }
        int fadeDuration = ((AudioBusRampView*)entry)->fade.duration;
        int fadeFrame = ((AudioBusRampView*)entry)->fade.frame;
        if (fadeFrame < fadeDuration) {
            if (fadeFrame < fadeDuration)
                ((AudioBusRampView*)entry)->fade.frame = fadeFrame + 1;
            fadeFrame = ((AudioBusRampView*)entry)->fade.frame;
            if (fadeFrame >= fadeDuration)
                gtl_AudioController_ClearEffects(self, i);
            active = 1;
        }
        if (active) {
            current = ((AudioBusRampView*)entry)->scale.frame;
            duration = ((AudioBusRampView*)entry)->scale.duration;
            float channelScale =
                current >= duration
                    ? ((AudioBusRampView*)entry)->scale.end
                    : ((AudioBusRampView*)entry)->scale.start +
                          (float)current *
                              (((AudioBusRampView*)entry)->scale.end - ((AudioBusRampView*)entry)->scale.start) /
                              (float)duration;
            if (channelScale > 1.0f)
                channelScale = 1.0f;
            else if (channelScale < 0.0f)
                channelScale = 0.0f;
            float channelFade =
                fadeFrame >= fadeDuration
                    ? ((AudioBusRampView*)entry)->fade.end
                    : ((AudioBusRampView*)entry)->fade.start +
                          (float)fadeFrame *
                              (((AudioBusRampView*)entry)->fade.end - ((AudioBusRampView*)entry)->fade.start) /
                              (float)fadeDuration;
            if (channelFade > 1.0f)
                channelFade = 1.0f;
            else if (channelFade < 0.0f)
                channelFade = 0.0f;
            unsigned int returnVolume = (unsigned int)((float)kUnityGainFixed * (channelScale * channelFade));
            if (i == 0)
                AXSetAuxAReturnVolume(returnVolume);
            else if (i == 1)
                AXSetAuxBReturnVolume(returnVolume);
            else
                AXSetAuxCReturnVolume(returnVolume);
        }
    }
    // The master gain ramp marks voices; independent fade/scale ramps drive AX.
    int duration = AsAudioController80404E94(self)->masterDuration;
    int current = AsAudioController80404E94(self)->masterFrame;
    if (current < duration) {
        if (current < duration)
            AsAudioController80404E94(self)->masterFrame = current + 1;
        gtl_AudioController_MarkActiveVoices(gtl_VoicePool_GetInstance(), kVoiceMasterGainChanged);
    }
    int masterFadeDuration = AsAudioController80404E94(self)->masterFadeDuration;
    int masterFadeFrame = AsAudioController80404E94(self)->masterFadeFrame;
    if (masterFadeFrame < masterFadeDuration)
        AsAudioController80404E94(self)->masterFadeFrame = masterFadeFrame + 1;
    int masterScaleDuration = AsAudioController80404E94(self)->masterScaleDuration;
    int masterScaleFrame = AsAudioController80404E94(self)->masterScaleFrame;
    if (masterScaleFrame < masterScaleDuration)
        AsAudioController80404E94(self)->masterScaleFrame = masterScaleFrame + 1;
    masterFadeFrame = AsAudioController80404E94(self)->masterFadeFrame;
    masterFadeDuration = AsAudioController80404E94(self)->masterFadeDuration;
    float masterFade = masterFadeFrame >= masterFadeDuration
                           ? AsAudioController80404E94(self)->masterFadeEnd
                           : AsAudioController80404E94(self)->masterFadeStart +
                                 (float)masterFadeFrame *
                                     (AsAudioController80404E94(self)->masterFadeEnd -
                                      AsAudioController80404E94(self)->masterFadeStart) /
                                     (float)masterFadeDuration;
    masterScaleFrame = AsAudioController80404E94(self)->masterScaleFrame;
    masterScaleDuration = AsAudioController80404E94(self)->masterScaleDuration;
    float masterScale = masterScaleFrame >= masterScaleDuration
                            ? AsAudioController80404E94(self)->masterScaleEnd
                            : AsAudioController80404E94(self)->masterScaleStart +
                                  (float)masterScaleFrame *
                                      (AsAudioController80404E94(self)->masterScaleEnd -
                                       AsAudioController80404E94(self)->masterScaleStart) /
                                      (float)masterScaleDuration;
    float masterGain = masterFade * masterScale;
    if (masterGain > 1.0f)
        masterGain = 1.0f;
    else if (masterGain < 0.0f)
        masterGain = 0.0f;
    AXSetMasterVolume((unsigned int)((float)kUnityGainFixed * masterGain));
}
#pragma pop

#pragma push
unsigned int gtl_AudioController_GetOutputBuffer(void* controller) {
    return AsAudioController80404E94(controller)->outputBuffer;
}
#pragma pop

#pragma push
void gtl_AudioController_RegisterCallback(void* self, void* node, void* callback) {
    struct Local {
        void* first;
        unsigned int level;
    } local;
    local.level = OSDisableInterrupts();
    AsAudioCallback80404DCC(node)->callback = callback;
    local.first = &AsAudioController80404E94(self)->callbackFirst;
    gtl_IntrusiveList_InsertBefore(&AsAudioController80404E94(self)->callbackCount, &local, node);
    OSRestoreInterrupts(local.level);
}
#pragma pop

#pragma push
void gtl_AudioController_UnregisterCallback(void* self, void* node) {
    volatile unsigned int level;
    volatile unsigned int* local = &level;
    *local = OSDisableInterrupts();
    IntrusiveList_UnlinkNode(&AsAudioController80404E94(self)->callbackCount, node);
    OSRestoreInterrupts(*local);
}
#pragma pop

#pragma push
void gtl_AudioController_SetOutputMode(int* p, int mode) {
    *p = mode;
    switch (mode) {
    case 1:
        AXSetMode(0);
        break;
    case 2:
        AXSetMode(1);
        break;
    case 3:
        AXSetMode(2);
        break;
    case 0:
        AXSetMode(0);
        break;
    }
    gtl_AudioController_MarkActiveVoices(gtl_VoicePool_GetInstance(), kVoiceOutputModeChanged);
}
#pragma pop

#pragma push
unsigned int gtl_AudioController_GetOutputMode(void* controller) {
    return AsAudioController80404E94(controller)->mode;
}
#pragma pop

#pragma push
void gtl_AudioController_DispatchCallbacks(void) {
    if (!s_audioControllerInitialized) {
        gtl_AudioController_InitMembers(s_gtlAudioController);
        __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                 s_gtlAudioControllerRegistration);
        s_audioControllerInitialized = 1;
    }
    void* sentinel = &AsAudioController80404E94(s_gtlAudioController)->callbackFirst;
    void* node = *(void**)sentinel;
    // Save next before invoking user code, allowing the current node to unregister.
    while (node != sentinel) {
        void* current = node;
        node = *(void**)node;
        void (*callback)(void*) = (void (*)(void*))AsAudioCallback80404DCC(current)->callback;
        callback(current);
        if (!s_audioControllerInitialized) {
            gtl_AudioController_InitMembers(s_gtlAudioController);
            __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                     s_gtlAudioControllerRegistration);
            s_audioControllerInitialized = 1;
        }
    }
    if (!s_audioControllerInitialized) {
        gtl_AudioController_InitMembers(s_gtlAudioController);
        __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                 s_gtlAudioControllerRegistration);
        s_audioControllerInitialized = 1;
    }
    if (AsAudioController80404E94(s_gtlAudioController)->previousCallback) {
        void (*callback)(void*) = (void (*)(void*))AsAudioController80404E94(s_gtlAudioController)->previousCallback;
        callback(s_gtlAudioController);
    }
}
#pragma pop

#pragma push
int gtl_AudioController_AttachEffect(void* self, int index, void* effect) {
    char* entry = (char*)self + index * kAudioRampBytes;
    enum {
        fadeEndOffset = (int)DECOMP_OFFSETOF(GtlAudioControllerState, channelFade0End)
    };
    int current = ((AudioBusRampView*)entry)->fade.frame;
    int duration = ((AudioBusRampView*)entry)->fade.duration;
    if (current < duration)
        gtl_AudioController_ClearEffects(self, index);
    current = ((AudioBusRampView*)entry)->fade.frame;
    duration = ((AudioBusRampView*)entry)->fade.duration;
    float result;
    if (current >= duration) {
        result = *(float*)(entry + fadeEndOffset);
    } else {
        float start = ((AudioBusRampView*)entry)->fade.start;
        float end = *(float*)(entry + fadeEndOffset);
        result = start + ((float)current * (end - start)) / (float)duration;
    }
    ((AudioBusRampView*)entry)->fade.start = result;
    *(float*)(entry + fadeEndOffset) = 1.0f;
    ((AudioBusRampView*)entry)->fade.duration = 0;
    ((AudioBusRampView*)entry)->fade.frame = 0;
    switch (index) {
    case kAuxA:
        AXSetAuxAReturnVolume(kUnityGainFixed);
        break;
    case kAuxB:
        AXSetAuxBReturnVolume(kUnityGainFixed);
        break;
    case kAuxC:
        AXSetAuxCReturnVolume(kUnityGainFixed);
        break;
    }
    // Register an AX callback only for an empty bus, then link the effect atomically.
    volatile unsigned int level;
    volatile unsigned int* saved = &level;
    *saved = OSDisableInterrupts();
    char* list =
        (char*)self + index * kAudioChannelListBytes + (int)DECOMP_OFFSETOF(GtlAudioControllerState, channelLists);
    if (!*(void**)list) {
        switch (index) {
        case kAuxA:
            AXRegisterAuxACallback((void*)gtl_AudioController_ProcessEffects, (void*)index);
            break;
        case kAuxB:
            AXRegisterAuxBCallback((void*)gtl_AudioController_ProcessEffects, (void*)index);
            break;
        case kAuxC:
            AXRegisterAuxCCallback((void*)gtl_AudioController_ProcessEffects, (void*)index);
            break;
        }
        AsAudioController80404E94(self)->silentCallbacks[index] = kSilentCallbacksAfterAttach;
    }
    void* first = list + 4;
    gtl_IntrusiveList_InsertBefore(list, &first, (char*)effect + 4);
    OSRestoreInterrupts(*saved);
    return 1;
}
#pragma pop

#pragma push
void gtl_AudioController_FadeOutEffects(void* self, int index, int ticks) {
    if (ticks == 0) {
        gtl_AudioController_ClearEffects(self, index);
    } else {
        struct Entry {
            char pad[0x54];
            float start;
            float end;
            int duration;
            int current;
        };
        Entry* entry = (Entry*)((char*)self + index * kAudioRampBytes);
        int duration;
        int current;
        duration = entry->duration;
        current = entry->current;
        float value;
        if (current >= duration) {
            value = entry->end;
        } else {
            value = entry->start + ((float)current * (entry->end - entry->start)) / (float)duration;
        }
        entry->start = value;
        entry->end = kFloat_0_AudioValueAnimation;
        entry->duration = (ticks + kAudioTicksPerCallback - 1) / kAudioTicksPerCallback;
        entry->current = 0;
    }
}
#pragma pop

#pragma push
void gtl_AudioController_ClearEffects(void* self, int index) {
    volatile unsigned int level;
    volatile unsigned int* saved_level = &level;
    *saved_level = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup(level);
    char* node =
        (char*)self + index * kAudioChannelListBytes + (int)DECOMP_OFFSETOF(GtlAudioControllerState, channelLists);
    if (!*(void**)node) {
        OSRestoreInterrupts(interruptCleanup.release());
        return;
    }
    {
        void* sentinel = node + 4;
        void* it = *(void**)sentinel;
        while (it != sentinel) {
            void* object = (char*)it - 4;
            struct VBase {
                virtual void slot0() DECOMP_LOCAL_VIRTUAL;
                virtual void slot1() DECOMP_LOCAL_VIRTUAL;
                virtual void slot2() DECOMP_LOCAL_VIRTUAL;
                virtual void slot3() DECOMP_LOCAL_VIRTUAL;
                virtual void slot4() DECOMP_LOCAL_VIRTUAL;
                virtual void slot5() DECOMP_LOCAL_VIRTUAL;
            };
            ((VBase*)object)->slot5();
            it = *(void**)it;
        }
        gtl_AudioEffectList_UnlinkAll(node);
        switch (index) {
        case kAuxA:
            AXRegisterAuxACallback(0, 0);
            break;
        case kAuxB:
            AXRegisterAuxBCallback(0, 0);
            break;
        case kAuxC:
            AXRegisterAuxCCallback(0, 0);
            break;
        }
    }
    OSRestoreInterrupts(interruptCleanup.release());
}
#pragma pop

#pragma push
void gtl_AudioController_ProcessEffects(void* values, int index) {
    void* args[4];
    int count;
    if (!s_audioControllerInitialized) {
        gtl_AudioController_InitMembers(s_gtlAudioController);
        __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                 s_gtlAudioControllerRegistration);
        s_audioControllerInitialized = 1;
    }
    if ((int)AsAudioController80404E94(s_gtlAudioController)->mode == kFourBufferOutput) {
        args[0] = ((void**)values)[0];
        args[1] = ((void**)values)[1];
        args[2] = ((void**)values)[2];
        args[3] = ((void**)values)[3];
        count = 4;
    } else {
        args[0] = ((void**)values)[0];
        args[1] = ((void**)values)[1];
        args[2] = ((void**)values)[2];
        count = 3;
    }
    if (!s_audioControllerInitialized) {
        gtl_AudioController_InitMembers(s_gtlAudioController);
        __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                 s_gtlAudioControllerRegistration);
        s_audioControllerInitialized = 1;
    }
    // Silence the first two callbacks after attachment, consuming one prime count.
    // state addresses the selected byte counter; it is not a complete controller.
    char* state = (char*)s_gtlAudioController + index;
    if (AsAudioController80404E94(state)->silentCallbacks[0]) {
        if (!s_audioControllerInitialized) {
            gtl_AudioController_InitMembers(s_gtlAudioController);
            __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                     s_gtlAudioControllerRegistration);
            s_audioControllerInitialized = 1;
        }
        --AsAudioController80404E94(state)->silentCallbacks[0];
        for (int i = 0; i < count; ++i)
            memset(args[i], 0, kEffectBlockBytes);
        return;
    }
    if (!s_audioControllerInitialized) {
        gtl_AudioController_InitMembers(s_gtlAudioController);
        __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                 s_gtlAudioControllerRegistration);
        s_audioControllerInitialized = 1;
    }
    // An empty list also emits silence; otherwise each effect processes the same lanes.
    // Initialization tests stay at their original callback boundaries: extracting their
    // common helper changes generated relocation symbols even when code bytes match.
    char* item = (char*)s_gtlAudioController + index * kAudioChannelListBytes;
    if (!(*(void**)&AsAudioController80404E94(item)->channelLists[0].count)) {
        for (int i = 0; i < count; ++i)
            memset(args[i], 0, kEffectBlockBytes);
        return;
    }
    if (!s_audioControllerInitialized) {
        gtl_AudioController_InitMembers(s_gtlAudioController);
        __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                 s_gtlAudioControllerRegistration);
        s_audioControllerInitialized = 1;
    }
    void* effectLink = AsAudioController80404E94(item)->channelLists[0].first;
    enum {
        channelSentinelOffset = (int)DECOMP_OFFSETOF(GtlAudioControllerState, channelLists[0].first)
    };
    void* sentinel = item + channelSentinelOffset;
    while (effectLink != sentinel) {
        if (!s_audioControllerInitialized) {
            gtl_AudioController_InitMembers(s_gtlAudioController);
            __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                     s_gtlAudioControllerRegistration);
            s_audioControllerInitialized = 1;
        }
        typedef void (*Callback)(void*, int, void**, int, float, int, unsigned int);
        void** vtable = *(void***)((char*)effectLink - kEffectLinkOffset);
        Callback callback = (Callback)vtable[kEffectProcessVtableSlot];
        callback((char*)effectLink - kEffectLinkOffset, count, args, kEffectBlockBytes, kAudioEffectSampleRateHz, 0,
                 AsAudioController80404E94(s_gtlAudioController)->mode);
        effectLink = *(void**)effectLink;
        if (!s_audioControllerInitialized) {
            gtl_AudioController_InitMembers(s_gtlAudioController);
            __register_global_object(s_gtlAudioController, (void*)gtl_AudioController_Dtor,
                                     s_gtlAudioControllerRegistration);
            s_audioControllerInitialized = 1;
        }
    }
}
#pragma pop
}
