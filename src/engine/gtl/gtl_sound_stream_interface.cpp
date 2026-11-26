// Stream interface of gtl::ISoundStream: teardown, per-frame parameter and modulation updates,
// and the playback header accessor. Each works on the player block the stream points to.
#include <decomp/static_init.h>
#include "classes/SoundStreamPlayer804022BC.h"
#include "classes/GtlSoundStreamInterface.h"
#include "classes/GtlBasicPlayer.h"
#include "classes/gtl_CSoundBufferSource.h"
#include "sdk_protos.h"

extern "C" {

extern float kFloat_6_2831855;
extern float s_soundStreamTwoPi;
extern float kFloat_0_SpatialAudio;
extern float kFloat_0_SoundBuffers;
extern float kFloat_1_AudioSpatial;
void __dl__FPv(void*);
void gtl_AudioEnvelope_Update(void*, void*);
void gtl_AudioLfo_Update(void*, void*);
void gtl_Audio_Free(void*);
void* gtl_Audio_GetListener(void);
int gtl_Audio_CalculateSpatialParameters(void*, void*, void*, int);
void gtl_IntrusiveList_Dtor(void*, int);
void __destroy_arr(void*, void* (*)(void*, int), int, int);
void dtor_80402C3C(void*);
void* dtor_804033F8(void*, int);
extern char __vt__Q23gtl18CSoundBufferSource[];
extern char __vt__Q23gtl12ISoundStream[];

// Reference count block of the shared sample owner.
struct SoundShared {
    int refs;
    void** vtable;
};

// Embedded player storage: offsets are derived from the checked shared layout.
enum StreamStorageOffset {
    kStreamHeader = 0x548,
    kStreamQueue = 8,
    kPendingStreamList = 0x104,
    kFreeStreamList = 0x110,
    kStreamEntries = 0x11C,
    kSpatialParameters = 0x608,
    kPitchLfo = 0x69C,
    kStreamEntryBytes = 0x1C,
    kStreamEntryCount = 32,
    kUseListener = 0x10,
    kListenerCalculationFlagShift = 17, // One flag bit passed to the calculation, not spatialMode.
    kSpatialParametersDisabled = 2,     // Value of the separate stored spatialMode enum.
    kEnvelopeRelease = 7,
    kStreamDestroySlot = 2,
    kPlaybackStopSlot = 4,
    kPlaybackStartSlot = 3,
    kPlaybackStatusAESlot = 7,
    kPlaybackStatusADSlot = 8
};

struct InterruptGuard_u804033F8 {
    BOOL level;
    bool active;
    // Start unarmed so an early return cannot restore an uninitialized level.
    InterruptGuard_u804033F8() : active(false) {}
    // Restore the previous interrupt level if explicit unlock was bypassed.
    ~InterruptGuard_u804033F8() {
        if (active)
            OSRestoreInterrupts(level);
    }
};
// Save the previous interrupt level, then arm restoration for this scope.
static inline void lock(InterruptGuard_u804033F8* g) {
    g->level = OSDisableInterrupts();
    g->active = true;
}
// Disarm scope cleanup and restore the level saved by the matching lock.
static inline void unlock(InterruptGuard_u804033F8* g) {
    g->active = false;
    OSRestoreInterrupts(g->level);
}

// Stop a borrowed native player, if present; this releases no source/state ownership.
static inline void StopNativeSoundStream(void* stream) {
    if (stream != 0)
        ((void (**)(void*))(*(void***)stream))[kPlaybackStopSlot](stream);
}

// Delete one stream queue entry when invoked as a deleting destructor.
void* dtor_804033F8(void* self, int flags) {
    if (self != 0 && flags > 0)
        __dl__FPv(self);
    return self;
}

} // extern "C"

// Stop playback, release the source reference and destroy the embedded stream queues.
// The player block is owned here; the returned address may already be freed when flags > 0.
#pragma push
void* gtl::ISoundStream::Destroy(int flags) {
    if (this != 0) {
        vtbl = __vt__Q23gtl12ISoundStream;
        void* owner = player;
        // Stop before tearing down ownership; nested guards restore the caller's level.
        if (((CSoundStreamPlayer*)owner)->stream != 0) {
            InterruptGuard_u804033F8 g;
            lock(&g);
            void* stream = ((CSoundStreamPlayer*)owner)->stream;
            StopNativeSoundStream(stream);
            unlock(&g);
            ((CSoundStreamPlayer*)owner)->stream = 0;
            (((CSoundStreamPlayer*)owner)->startPending) = 0;
        }
        owner = player;
        if (owner != 0) {
            // Expanded member cleanup checks its address, not whether playback is active.
            if (&((CSoundStreamPlayer*)owner)->stream != 0) {
                InterruptGuard_u804033F8 g;
                lock(&g);
                void* stream = ((CSoundStreamPlayer*)owner)->stream;
                StopNativeSoundStream(stream);
                unlock(&g);
                // Release the retained source only after playback has stopped.
                void* shared = ((CSoundStreamPlayer*)owner)->source;
                if (shared != 0) {
                    int refs = --((SoundShared*)shared)->refs;
                    if (refs == 0)
                        ((void (**)(void*, int))((SoundShared*)shared)->vtable)[2](shared, 1);
                    ((CSoundStreamPlayer*)owner)->source = 0;
                }
            }
            // Entries belong to the state allocation; list destructors unlink their storage.
            if ((char*)owner + kStreamQueue != 0) {
                __destroy_arr((char*)owner + kStreamEntries, dtor_804033F8, kStreamEntryBytes, kStreamEntryCount);
                gtl_IntrusiveList_Dtor((char*)owner + kFreeStreamList, 0);
                gtl_IntrusiveList_Dtor((char*)owner + kPendingStreamList, 0);
            }
            gtl_Audio_Free(owner);
        }
        player = 0;
        if (flags > 0)
            gtl_Audio_Free(this);
    }
    return this;
}
#pragma pop

// Apply fade, listener attenuation, pitch modulation and pan to active playback.
// Requires a constructed, nonnull state. Listener output lanes are horizontal pan,
// surround pan, volume multiplier, Doppler pitch ratio; these are mixer parameters,
// not position coordinates. Shared listenerFactor is the pitch multiplier here.
#pragma push
void gtl::ISoundStream::UpdateParameters() {
    CSoundStreamPlayer* owner = player;
    void* stream = owner->stream;
    if (stream == 0)
        return;
    // Status AD clear + AE set permits the START call; slot 3 is not a state getter.
    // The two status bytes' broader meanings are unresolved (gtl_sound_stream_player.cpp accessors).
    if (owner->startPending != 0) {
        int statusAD = ((int (**)(void*))(*(void***)stream))[kPlaybackStatusADSlot](stream);
        if (statusAD == 0 && ((int (**)(void*))(*(void***)stream))[kPlaybackStatusAESlot](stream) != 0 &&
            ((int (**)(void*))(*(void***)stream))[kPlaybackStartSlot](stream) == 1)
            owner->startPending = 0;
    }
    void* listener = (owner->flags & kUseListener) ? gtl_Audio_GetListener() : 0;
    int listenerCalculationFlag = (owner->flags >> kListenerCalculationFlagShift) & 1;
    InterruptGuard_u804033F8 guard;
    // Outer lock makes the full parameter update atomic. Inner guards deliberately
    // nest: restoring their saved level keeps interrupts disabled until this unlock.
    lock(&guard);
    if (owner->stream != 0) {
        // A release at zero gain stops the stream instead of writing mixer parameters.
        int releasing = owner->fadeState == kEnvelopeRelease;
        if (releasing) {
            float volume = owner->fadeRunning == 0 ? 0.0f : owner->fadeFactor;
            if (volume == 0.0f) {
                InterruptGuard_u804033F8 inner;
                lock(&inner);
                void* cur = owner->stream;
                StopNativeSoundStream(cur);
                unlock(&inner);
                unlock(&guard);
                return;
            }
        }
        // Neutral listener scales and authored pan are the fallback without spatial output.
        owner->listenerVolume = 1.0f;
        owner->listenerDopplerRatio = 1.0f;
        float horizontalPan = owner->pan, surroundPan = kFloat_0_SoundBuffers;
        if (listener && owner->spatialMode != kSpatialParametersDisabled) {
            float spatialResult[4];
            if (gtl_Audio_CalculateSpatialParameters(listener, (char*)owner + kSpatialParameters, spatialResult,
                                                     listenerCalculationFlag) == 1) {
                owner->listenerVolume = spatialResult[2];
                owner->listenerDopplerRatio = spatialResult[3];
                horizontalPan = spatialResult[0] + kFloat_0_SpatialAudio;
                surroundPan = spatialResult[1] + kFloat_1_AudioSpatial;
            }
        }
        // Gain = authored volume * listener volume * envelope.
        // Pitch = authored pitch * listener pitch * LFO; pan uses the two lanes above.
        InterruptGuard_u804033F8 g1;
        lock(&g1);
        stream = owner->stream;
        if (stream) {
            float envelopeGain =
                owner->fadeRunning == 0 ? (owner->fadeState == kEnvelopeRelease ? 0.0f : 1.0f) : owner->fadeFactor;
            AsGtlBasicPlayer(stream)->volumeScale = owner->volume * owner->listenerVolume * envelopeGain;
        }
        unlock(&g1);
        InterruptGuard_u804033F8 g2;
        lock(&g2);
        stream = owner->stream;
        if (stream)
            AsGtlBasicPlayer(stream)->pitchScale = owner->pitch * owner->listenerDopplerRatio * owner->pitchScale;
        unlock(&g2);
        stream = owner->stream;
        AsGtlBasicPlayer(stream)->pan = horizontalPan;
        AsGtlBasicPlayer(stream)->surroundPan = surroundPan;
    }
    unlock(&guard);
}
#pragma pop

// Advance the envelope and pitch LFO while the stream is active.
// elapsedTicks is passed unchanged in a register to both updaters; it is a tick count, not an address.
#pragma push
void gtl::ISoundStream::UpdateModulation(void* elapsedTicks) {
    CSoundStreamPlayer* playerState = player;
    BOOL level = OSDisableInterrupts();
    if (playerState->stream != 0) {
        gtl_AudioEnvelope_Update(&playerState->fadeState, elapsedTicks);
        gtl_AudioLfo_Update((char*)playerState + kPitchLfo, elapsedTicks);
    }
    OSRestoreInterrupts(level);
}
#pragma pop

// Return the playback-format header embedded in the stream player.
// Borrowed address: valid only while the player lives.
#pragma push
void* gtl::ISoundStream::GetPlaybackHeader() {
    return (char*)player + kStreamHeader;
}
#pragma pop

extern "C" {

// Initialize this stream unit's cached two-pi constant.
void gtl_SoundStream_InitializeTwoPi() {
    s_soundStreamTwoPi = kFloat_6_2831855;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626A8", _ctors_u804033F8_0, gtl_SoundStream_InitializeTwoPi);
#pragma pop

// Copy sample metadata and retain the shared sample allocation.
// from/shared are borrowed for the call; only shared->sampleOwner gains a reference.
gtl::CSoundBufferSource* gtl::CSoundBufferSource::Init(const SoundCopyData* from, const SoundRefData* shared) {
    referenceCount = 1;
    vtable = &__vt__Q23gtl18CSoundBufferSource;
    name = from->name;
    format = from->format;
    sampleRate = from->sampleRate;
    sampleFormat = from->sampleFormat;
    channelCount = from->channelCount;
    sampleCount = from->sampleCount;
    loopData = from->loopData;
    sampleOwner = shared->sampleOwner;
    sampleData = shared->sampleData;
    sampleBytes = shared->sampleBytes;
    void* ref = shared->sampleOwner;
    if (ref != 0)
        ++((SoundShared*)ref)->refs;
    return this;
}

// Release the shared sample allocation and optionally free this source.
// flags > 0 frees this; the returned address is for destructor ABI compatibility.
void* gtl::CSoundBufferSource::Destroy(int flags) {
    if (this != 0) {
        vtable = &__vt__Q23gtl18CSoundBufferSource;
        SoundShared* shared = (SoundShared*)sampleOwner;
        if (shared != 0) {
            if (--shared->refs == 0 && shared != 0)
                ((void (*)(SoundShared*, int))shared->vtable[2])(shared, 1);
            sampleOwner = 0;
        }
        if (flags > 0)
            gtl_Audio_Free(this);
    }
    return this;
}

} // extern "C"
