#include <decomp/static_init.h>
#include "classes/SoundStreamPlayer804022BC.h"
#include "classes/GtlSoundStreamInterface.h"
#include "classes/GtlBasicPlayer.h"
// Borrow the stream interface layout; no allocation or reference transfer occurs.
// Inline-function replacement renumbers local relocation targets; preserve this macro.
#define AsGtlSoundStreamInterface(p) ((GtlSoundStreamInterface*)(p))
// Translation unit 0x804033F8-0x80403A8C (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
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
void gtl_Audio_Free(void*);
void __destroy_arr(void*, void* (*)(void*, int), int, int);
void dtor_80402C3C(void*);
void* dtor_804033F8(void*, int);
extern char __vt__Q23gtl18CSoundBufferSource[];
extern char __vt__Q23gtl12ISoundStream[];

struct SoundStreamHeaderOwner {
    char interfacePrefix[8];
    void* player;
};

// Source metadata is copied verbatim. Quantities are 32-bit ABI words;
// addresses retain pointer types. Counts are samples, sampleBytes is bytes.
// sampleOwner is the retained reference-counted allocation; sampleData is its view.
struct SoundBufferSource {
    int referenceCount;
    void* vtable;
    void* name;
    void* format;
    unsigned int sampleRate;
    unsigned short sampleFormat;
    unsigned short channelCount;
    unsigned int sampleCount;
    void* loopData;
    void* sampleOwner;
    void* sampleData;
    unsigned int sampleBytes;
};

struct SoundShared {
    int refs;
    void** vtable;
};

struct SoundCopyData {
    void* name;
    void* format;
    unsigned int sampleRate;
    unsigned short sampleFormat;
    unsigned short channelCount;
    unsigned int sampleCount;
    void* loopData;
};
struct SoundRefData { void* sampleOwner; void* sampleData; unsigned int sampleBytes; };

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
    kSpatialParametersDisabled = 2, // Value of the separate stored spatialMode enum.
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
    ~InterruptGuard_u804033F8() { if (active) OSRestoreInterrupts(level); }
};
// Save the previous interrupt level, then arm restoration for this scope.
static void lock(InterruptGuard_u804033F8* g) {
    g->level = OSDisableInterrupts();
    g->active = true;
}
// Disarm scope cleanup and restore the level saved by the matching lock.
static void unlock(InterruptGuard_u804033F8* g) {
    g->active = false;
    OSRestoreInterrupts(g->level);
}

// Stop a borrowed native player, if present; this releases no source/state ownership.
static inline void StopNativeSoundStream(void* stream) {
    if (stream != 0) ((void (**)(void*))(*(void***)stream))[kPlaybackStopSlot](stream);
}

// Stop playback, release source references and destroy the embedded stream queues.
// self owns state and its 32 embedded entries; source is retained separately.
// flags > 0 also frees self. The returned address may therefore already be freed.
void* gtl_ISoundStream_dtor(void* self, int flags);

// Apply fade, listener attenuation, pitch modulation and pan to active playback.
// Requires a constructed, nonnull state. Listener output lanes are horizontal pan,
// surround pan, volume multiplier, Doppler pitch ratio; these are mixer parameters,
// not position coordinates. Shared listenerFactor is the pitch multiplier here.
void gtl_ISoundStream_UpdateParameters(void* self);

// Advance the envelope and pitch LFO while the stream is active.
// elapsedTicks is passed unchanged in a register to both updaters; this legacy
// pointer-shaped declaration carries a tick count, not an address to dereference.
void gtl_ISoundStream_UpdateModulation(void* self, void* elapsedTicks);

// Copy sample metadata and retain the shared sample allocation.
// from/shared are borrowed for the call; only shared->sampleOwner gains a reference.
SoundBufferSource* gtl_CSoundBufferSource_Ctor(SoundBufferSource* self, const SoundCopyData* from, const SoundRefData* shared);

// Release the shared sample allocation and optionally free this source.
// flags > 0 frees self; the returned address is for destructor ABI compatibility.
void* gtl_CSoundBufferSource_dtor(SoundBufferSource* self, int flags);

// Return the playback-format header embedded in the stream player.
// Borrowed address: requires self->player and remains valid only while that player lives.
void* gtl_ISoundStream_GetPlaybackHeader(SoundStreamHeaderOwner* self);

// Initialize this stream unit's cached two-pi constant.
void gtl_SoundStream_InitializeTwoPi();

// Delete one stream queue entry when invoked as a deleting destructor.
void* dtor_804033F8(void* self, int flags);

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626A8", _ctors_u804033F8_0, gtl_SoundStream_InitializeTwoPi);
#pragma pop


// Function definitions follow the original address order.


#pragma push
extern "C" {
void* dtor_804033F8(void* self, int flags) {
    if (self != 0 && flags > 0)
        __dl__FPv(self);
    return self;
}
}
#pragma pop

#pragma push
extern "C" {
void* gtl_ISoundStream_dtor(void* self, int flags) {
    if (self != 0) {
        AsGtlSoundStreamInterface(self)->vtbl = __vt__Q23gtl12ISoundStream;
        void* owner = AsGtlSoundStreamInterface(self)->state;
        // Stop before tearing down ownership; nested guards restore the caller's level.
        if (((SoundStreamPlayer804022BC*)owner)->stream != 0) {
            InterruptGuard_u804033F8 g;
            lock(&g);
            void* stream = ((SoundStreamPlayer804022BC*)owner)->stream;
            StopNativeSoundStream(stream);
            unlock(&g);
            ((SoundStreamPlayer804022BC*)owner)->stream = 0;
            (((SoundStreamPlayer804022BC*)owner)->startPending) = 0;
        }
        owner = AsGtlSoundStreamInterface(self)->state;
        if (owner != 0) {
            // Expanded member cleanup checks its address, not whether playback is active.
            if (&((SoundStreamPlayer804022BC*)owner)->stream != 0) {
                InterruptGuard_u804033F8 g;
                lock(&g);
                void* stream = ((SoundStreamPlayer804022BC*)owner)->stream;
                StopNativeSoundStream(stream);
                unlock(&g);
                // Release the retained source only after playback has stopped.
                void* shared = ((SoundStreamPlayer804022BC*)owner)->source;
                if (shared != 0) {
                    int refs = --((SoundShared*)shared)->refs;
                    if (refs == 0) ((void (**)(void*, int))((SoundShared*)shared)->vtable)[2](shared, 1);
                    ((SoundStreamPlayer804022BC*)owner)->source = 0;
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
        AsGtlSoundStreamInterface(self)->state = 0;
        if (flags > 0) gtl_Audio_Free(self);
    }
    return self;
}
}
#pragma pop

#pragma push
extern "C" {
void gtl_ISoundStream_UpdateParameters(void* self) {
    char* owner = (char*)AsGtlSoundStreamInterface(self)->state;
    void* stream = ((SoundStreamPlayer804022BC*)owner)->stream;
    if (stream == 0) return;
    // Status AD clear + AE set permits the START call; slot 3 is not a state getter.
    // The two status bytes' broader meanings are unresolved (gtl_sound_stream_player.cpp accessors).
    if (((SoundStreamPlayer804022BC*)owner)->startPending != 0) {
        int statusAD = ((int (**)(void*))(*(void***)stream))[kPlaybackStatusADSlot](stream);
        if (statusAD == 0 && ((int (**)(void*))(*(void***)stream))[kPlaybackStatusAESlot](stream) != 0 &&
            ((int (**)(void*))(*(void***)stream))[kPlaybackStartSlot](stream) == 1)
            ((SoundStreamPlayer804022BC*)owner)->startPending = 0;
    }
    void* listener = (((SoundStreamPlayer804022BC*)owner)->flags & kUseListener) ? gtl_Audio_GetListener() : 0;
    int listenerCalculationFlag = (((SoundStreamPlayer804022BC*)owner)->flags >> kListenerCalculationFlagShift) & 1;
    InterruptGuard_u804033F8 guard;
    // Outer lock makes the full parameter update atomic. Inner guards deliberately
    // nest: restoring their saved level keeps interrupts disabled until this unlock.
    lock(&guard);
    if (((SoundStreamPlayer804022BC*)owner)->stream != 0) {
        // A release at zero gain stops the stream instead of writing mixer parameters.
        int releasing = ((SoundStreamPlayer804022BC*)owner)->fadeState == kEnvelopeRelease;
        if (releasing) {
            float volume = ((SoundStreamPlayer804022BC*)owner)->fadeRunning == 0 ? 0.0f : ((SoundStreamPlayer804022BC*)owner)->fadeFactor;
            if (volume == 0.0f) {
                InterruptGuard_u804033F8 inner; lock(&inner);
                void* cur = ((SoundStreamPlayer804022BC*)owner)->stream;
                StopNativeSoundStream(cur);
                unlock(&inner); unlock(&guard); return;
            }
        }
        // Neutral listener scales and authored pan are the fallback without spatial output.
        ((SoundStreamPlayer804022BC*)owner)->listenerVolume = 1.0f;
        ((SoundStreamPlayer804022BC*)owner)->listenerDopplerRatio = 1.0f;
        float horizontalPan = ((SoundStreamPlayer804022BC*)owner)->pan, surroundPan = kFloat_0_SoundBuffers;
        if (listener && ((SoundStreamPlayer804022BC*)owner)->spatialMode != kSpatialParametersDisabled) {
            float spatialResult[4];
            if (gtl_Audio_CalculateSpatialParameters(listener, owner + kSpatialParameters, spatialResult, listenerCalculationFlag) == 1) {
                ((SoundStreamPlayer804022BC*)owner)->listenerVolume = spatialResult[2]; ((SoundStreamPlayer804022BC*)owner)->listenerDopplerRatio = spatialResult[3];
                horizontalPan = spatialResult[0] + kFloat_0_SpatialAudio; surroundPan = spatialResult[1] + kFloat_1_AudioSpatial;
            }
        }
        // Gain = authored volume * listener volume * envelope.
        // Pitch = authored pitch * listener pitch * LFO; pan uses the two lanes above.
        InterruptGuard_u804033F8 g1;
        lock(&g1);
        stream = ((SoundStreamPlayer804022BC*)owner)->stream;
        if (stream) {
            float envelopeGain = ((SoundStreamPlayer804022BC*)owner)->fadeRunning == 0 ? (((SoundStreamPlayer804022BC*)owner)->fadeState == kEnvelopeRelease ? 0.0f : 1.0f) : ((SoundStreamPlayer804022BC*)owner)->fadeFactor;
            AsGtlBasicPlayer(stream)->volumeScale = ((SoundStreamPlayer804022BC*)owner)->volume * ((SoundStreamPlayer804022BC*)owner)->listenerVolume * envelopeGain;
        }
        unlock(&g1);
        InterruptGuard_u804033F8 g2;
        lock(&g2);
        stream = ((SoundStreamPlayer804022BC*)owner)->stream;
        if (stream) AsGtlBasicPlayer(stream)->pitchScale = ((SoundStreamPlayer804022BC*)owner)->pitch * ((SoundStreamPlayer804022BC*)owner)->listenerDopplerRatio * ((SoundStreamPlayer804022BC*)owner)->pitchScale;
        unlock(&g2);
        stream = ((SoundStreamPlayer804022BC*)owner)->stream;
        AsGtlBasicPlayer(stream)->pan = horizontalPan; AsGtlBasicPlayer(stream)->surroundPan = surroundPan;
    }
    unlock(&guard);
}
}
#pragma pop

#pragma push
extern "C" {
void gtl_ISoundStream_UpdateModulation(void* self, void* elapsedTicks) {
    SoundStreamPlayer804022BC* playerState = (SoundStreamPlayer804022BC*)AsGtlSoundStreamInterface(self)->state;
    BOOL level = OSDisableInterrupts();
    if (((SoundStreamPlayer804022BC*)playerState)->stream != 0) {
        gtl_AudioEnvelope_Update(&((SoundStreamPlayer804022BC*)playerState)->fadeState, elapsedTicks);
        gtl_AudioLfo_Update((char*)playerState + kPitchLfo, elapsedTicks);
    }
    OSRestoreInterrupts(level);
}
}
#pragma pop

#pragma push
extern "C" {
void* gtl_ISoundStream_GetPlaybackHeader(SoundStreamHeaderOwner* self) {
    return (char*)self->player + kStreamHeader;
}
}
#pragma pop

#pragma push
extern "C" {
void gtl_SoundStream_InitializeTwoPi() {
    s_soundStreamTwoPi = kFloat_6_2831855;
}
}
#pragma pop

#pragma push
extern "C" {
SoundBufferSource* gtl_CSoundBufferSource_Ctor(SoundBufferSource* self, const SoundCopyData* from, const SoundRefData* shared) {
    self->referenceCount = 1;
    self->vtable = &__vt__Q23gtl18CSoundBufferSource;
    self->name = from->name;
    self->format = from->format;
    self->sampleRate = from->sampleRate;
    self->sampleFormat = from->sampleFormat;
    self->channelCount = from->channelCount;
    self->sampleCount = from->sampleCount;
    self->loopData = from->loopData;
    self->sampleOwner = shared->sampleOwner;
    self->sampleData = shared->sampleData;
    self->sampleBytes = shared->sampleBytes;
    void* ref = shared->sampleOwner;
    if (ref != 0) ++((SoundShared*)ref)->refs;
    return self;
}
}
#pragma pop

#pragma push
extern "C" {
void* gtl_CSoundBufferSource_dtor(SoundBufferSource* self, int flags) {
    if (self != 0) {
        self->vtable = &__vt__Q23gtl18CSoundBufferSource;
        SoundShared* shared = (SoundShared*)self->sampleOwner;
        if (shared != 0) {
            if (--shared->refs == 0 && shared != 0)
                ((void (*)(SoundShared*, int))shared->vtable[2])(shared, 1);
            self->sampleOwner = 0;
        }
        if (flags > 0)
            gtl_Audio_Free(self);
    }
    return self;
}
}
#pragma pop
