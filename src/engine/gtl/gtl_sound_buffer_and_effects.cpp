#include <decomp/static_init.h>
// gtl sound buffers, spatial controls and audio-effect registry/lifecycle.
// Mod lookup: attenuation/cones/Doppler -> SetterTarget and setters below,
// consumed by CalculateSpatialParameters/CalculatePanAndAttenuation (gtl_audio_spatial_and_runtime.cpp);
// defaults -> s_soundBufferParameterDefaults, copied in ISoundBuffer_Ctor;
// effect activation -> SetEnabled/GetEnabled; effect ownership -> Initialize,
// Finalize and registry Shutdown. Buffer layout is classes/GtlSoundBuffer.h;
// custom distance curves are classes/GtlAudioCurve.h, NOT numeric mode IDs.
// Voice updates guard interrupt state; control setters do not provide locking.
// Translation unit 0x80400B80-0x804022BC (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/GtlSoundBuffer.h"
extern "C" char s_bpCtrl[];

extern "C" {

extern const float kFloat_6_2831855;
float s_audioTwoPiA;
float s_audioTwoPiB;

void gtl_Audio_Free(void* allocation);
void* gtl_Audio_AllocStorage(unsigned int size);
void* gtl_Audio_Alloc(unsigned int bytes);
void* gtl_AudioController_GetInstance();
void gtl_AudioController_AttachEffect(void*, int, void*);
void* __nw__FUl(unsigned int);
void GtlVoice_Release(void*);
void gtl_AudioEnvelope_Update(void*, int);
void gtl_AudioLfo_Update(void*, int);
void dtor_8040034C();
void InterruptGuard_dtor();
void* gtl_AudioController_GetInstance();
void gtl_AudioController_FadeOutEffects(void*, int, int);
int gtl_Audio_CalculateSpatialParameters(void*, void*, void*, int);
void GtlVoice_Stop(void*);
void gtl_ISoundBuffer_Stop(void*, int);
void gtl_nw4r_detail_Voice_SetVolumeScale(void*, float);
void GtlVoice_SetPitchScale(void*, float);
void GtlVoice_SetPan(void*, float);
void GtlVoice_SetSurroundPan(void*, float);
void* gtl_Audio_GetListener();

struct CleanupOwner {
    void* child;
    void* values;
};

// Clear the vector slot, free its owned child storage, and optionally delete the wrapper.
void* dtor_80402170(CleanupOwner* self, int shouldDelete) {
    if (self != 0) {
        self->values = 0;
        if (self != 0 && self->child != 0) {
            self->values = 0;
            gtl_Audio_Free(self->child);
        }
        if (shouldDelete > 0)
            delete self;
    }
    return self;
}

// Clear the vector slot, free its owned child storage, and optionally delete the wrapper.
void* dtor_804021E4(CleanupOwner* self, int shouldDelete) {
    if (self != 0) {
        self->values = 0;
        if (self != 0) {
            void* child = self->child;
            if (child != 0) {
                self->values = 0;
                gtl_Audio_Free(child);
            }
        }
        if (shouldDelete > 0)
            delete self;
    }
    return self;
}

struct PairOwner;
PairOwner* gtl_EffectRegistry_InitStorage(PairOwner*);
int gtl_EffectRegistry_RegisterBuiltins(void*);
extern const unsigned int s_builtinAudioEffectCreatorPairs[10]; // Five built-in effect ID/creator pairs.
extern void* g_gtlEffectRegistry;
extern "C" void GtlEffectRegistry_DeleteWrapper();
extern "C" void* memcpy(void*, const void*, unsigned long);
extern "C" void* memset(void*, int, unsigned long);
extern "C" char s_effectRegistryRegistrationSlot[]; // Effect-registry singleton destructor registration.
extern void* s_gtlEffectRegistry[2];
extern signed char s_audioEffectRegistryInitialized;
struct InitMapPair { unsigned int first; unsigned int second; };
struct PtrVector { InitMapPair* begin; InitMapPair* end; InitMapPair* cap; };
struct PairVector { unsigned int* begin; unsigned int* end; unsigned int* cap; };
struct InitMapNode { void* left; void* right; void* parent; void* value; };
struct InitMap { void* header; unsigned int size; };
struct InitMapValue { void* key; PtrVector values; };
// Exception-string pool used by bounded registry-vector growth and index checks.
struct AudioVectorExceptionStrings {
    char lengthError[0x14];
    char lengthErrorTypeName[0x37];
    char indexOutOfRange[0x1E];
    char outOfRangeTypeName[0x37];
};
extern const AudioVectorExceptionStrings audioTrackVectorExceptionStrings;
extern char __vt__Q23std11logic_error[], __vt__Q23std12length_error[], __vt__Q23std12out_of_range[];
extern char s_sharedCharDeleterVtable[];
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std12length_errorFv(void*);
void __dt__Q23std12out_of_rangeFv(void*);
// Construct a shared-message standard exception and throw it using the supplied destructor and type.
static void AllocSize_ThrowInit(const char* text, const char* typeName, void* destroy, void* finalTable) {
    struct Error { void* table; char* message; void* control; } error;
    struct Control { void* table; int strong; int weak; char* data; };
    // Release message bytes if allocating their shared control block fails.
    struct Guard { char* data; ~Guard() { if (data) __dla__FPv(data); } } guard = { 0 };
    error.table = (void*)__vt__Q23std11logic_error;
    guard.data = (char*)__nwa__FUl(strlen(text) + 1);
    error.message = guard.data;
    Control* control = (Control*)__nw__FUl(0x10);
    if (control) {
        control->table = (void*)s_sharedCharDeleterVtable;
        control->strong = control->weak = 1;
        control->data = guard.data;
    }
    error.control = control;
    guard.data = 0;
    strcpy(error.message, text);
    error.table = (void*)finalTable;
    __throw(typeName, &error, destroy);
}

struct InitVector { void* data; unsigned int count; unsigned int capacity; };
// Both vectors own audio-allocator storage. Factories contain 8-byte {ID, creator}
// entries; instances contain 4-byte reference-counted object pointers at matching indices.
// Growing the instance vector transfers pointers (zeroing old slots), not references.
struct InitEntry { unsigned int key; int (*create)(unsigned int*, void**); };
struct RegistryVectors { InitVector* factories; InitVector* instances; };
struct RegistryOwnerView { int initialized; RegistryVectors* vectors; };
struct RegisteredEffectView { int refs; void* vtable; int enabled; unsigned int registryIndex; };
// Choose the original bounded vector growth capacity or throw a length error.
static unsigned int InitVector_Grow(unsigned int capacity, unsigned int count, unsigned int limit) {
    unsigned int need = count + 1 - capacity;
    if (1u > limit - capacity || need > limit - capacity)
        AllocSize_ThrowInit(audioTrackVectorExceptionStrings.lengthError, audioTrackVectorExceptionStrings.lengthErrorTypeName,
                            (void*)__dt__Q23std12length_errorFv, __vt__Q23std12length_error);
    if (capacity < limit / 3) {
        unsigned int extra = (capacity + 1) * 3 / 5;
        return capacity + (extra < need ? need : extra);
    }
    if (capacity < (limit / 3) * 2) {
        unsigned int extra = (capacity + 1) / 2;
        return capacity + (extra < need ? need : extra);
    }
    return limit;
}
// Register five built-in effect factories and create their matching reference-counted effect instances.
// Returns 1 only after all five; a failed creator returns 0 with earlier entries retained.
// Creator receives the index by address and writes the owned object to its output slot.
int gtl_EffectRegistry_RegisterBuiltins(void* owner) {
    InitEntry pairs[5];
    for (int i = 0; i < 10; ++i) ((unsigned int*)pairs)[i] = s_builtinAudioEffectCreatorPairs[i];
    for (int n = 0; n < 5; ++n) {
        // Phase 1: append the factory; keep its pre-append index for instance creation.
        InitVector* first = ((RegistryOwnerView*)owner)->vectors->factories;
        unsigned int index = first->count;
        if (first->count < first->capacity) {
            ((InitEntry*)first->data)[first->count++] = pairs[n];
        } else {
            unsigned int next = InitVector_Grow(first->capacity, first->count, 0x1fffffff);
            InitEntry* fresh = (InitEntry*)gtl_Audio_Alloc(next * 8);
            InitEntry* old = (InitEntry*)first->data;
            fresh[first->count] = pairs[n];
            for (unsigned int j = first->count; j != 0; --j) fresh[j - 1] = old[j - 1];
            first->data = fresh;
            first->capacity = next;
            first->count = first->count + 1;
            if (old) gtl_Audio_Free(old);
        }
        // Phase 2: publish the singleton, then invoke its factory at that index.
        if (g_gtlEffectRegistry == 0) {
            if (!s_audioEffectRegistryInitialized) {
                s_gtlEffectRegistry[0] = 0;
                s_gtlEffectRegistry[1] = 0;
                __register_global_object(s_gtlEffectRegistry, (void*)GtlEffectRegistry_DeleteWrapper, s_effectRegistryRegistrationSlot);
                s_audioEffectRegistryInitialized = 1;
            }
            g_gtlEffectRegistry = s_gtlEffectRegistry;
        }
        InitVector* table = ((RegistryOwnerView*)g_gtlEffectRegistry)->vectors->factories;
        void* created = 0;
        if (index >= table->count)
            AllocSize_ThrowInit(audioTrackVectorExceptionStrings.indexOutOfRange, audioTrackVectorExceptionStrings.outOfRangeTypeName,
                                (void*)__dt__Q23std12out_of_rangeFv, __vt__Q23std12out_of_range);
        if (!((InitEntry*)table->data)[index].create(&index, &created)) return 0;
        ((RegisteredEffectView*)created)->registryIndex = index;
        // Phase 3: append the created object, transferring storage on vector growth.
        InitVector* second = ((RegistryOwnerView*)owner)->vectors->instances;
        if (second->count < second->capacity) {
            ((void**)second->data)[second->count++] = created;
        } else {
            unsigned int next = InitVector_Grow(second->capacity, second->count, 0x3fffffff);
            void** fresh = (void**)gtl_Audio_Alloc(next * 4);
            void** old = (void**)second->data;
            fresh[second->count] = created;
            memcpy(fresh, old, second->count * 4);
            memset(old, 0, second->count * 4);
            second->data = fresh;
            second->capacity = next;
            second->count = second->count + 1;
            if (old) gtl_Audio_Free(old);
        }
    }
    return 1;
}
struct InitOwner { int initialized; void* child; };
// Allocate the two-vector registry, register built-ins, and mark successful initialization.
// Legacy pointer return encodes Boolean 0/1, never an owner address.
InitOwner* gtl_EffectRegistry_Initialize(InitOwner* self) {
    if (self->initialized != 0)
        return 0;
    void* child = gtl_Audio_AllocStorage(8);
    if (child != 0)
        gtl_EffectRegistry_InitStorage((PairOwner*)child);
    self->child = child;
    if (gtl_EffectRegistry_RegisterBuiltins(self) == 0)
        return 0;
    self->initialized = 1;
    return (InitOwner*)1;
}

// Spatial setter view aliases SoundBufferData at the same offsets. All setters
// require live self/target, copy unvalidated values verbatim, return status 1,
// and allocate/retain nothing. Callers serialize with voice parameter updates.
// Consumer map (gtl_audio_spatial_and_runtime.cpp, source state begins at buffer +BC):
// +EC/+F0 full cone angles in radians; consumer compares acos(dot) to half angles.
// +F4/+F8 cone gains: unitless amplitude factors, interpolated between boundaries.
// +FC distanceCurve: Wii ADDRESS BITS of a borrowed GtlAudioCurve, not an enum.
// +100 parameter A: zero bypasses ALL spatial processing; otherwise multiplies
//      parameter B for Doppler amount. Physical interpretation of A is unresolved.
// +104 parameter B: Doppler factor; zero disables Doppler only, not attenuation.
// +108/+10C reference/max distances: same units as source/listener positions.
// +110 mode: 0 transforms world -> listener space; 2 bypasses spatial processing;
//      other values use supplied source coordinates directly for pan.
// Default rolloff: gain=1 within minDistance, then minDistance/distance with
// distance capped to maxDistance. Flag bit 17 can instead mute at/beyond max.
// Use positive reference distance and max >= reference for ordinary rolloff;
// setters enforce neither condition. No meter/second conversion is established.
// Custom curve: {s32 mode,u32 count,borrowed {f32 distance,gain}* points};
// mode 0/count 0 falls back to rolloff; 1 is linear, 2 Catmull-Rom; other nonzero
// modes yield zero for interior evaluations. Points must be ordered by distance
// with nonzero spans; endpoints clamp. Caller keeps curve/points alive for updates.
struct SetterTarget {
    char reserved00[0xEC];
    float innerConeAngle, outerConeAngle;
    float innerConeGain, outerConeGain;
    int distanceCurve; // pointer bits in the legacy Wii ABI; never a curve selector
    float distanceParameterA;
    float distanceParameterB;
    float minDistance;
    float maxDistance;
    int spatialMode;
};

struct SetterOwner {
    char reserved00[8];
    SetterTarget* target;
};

// Store the full-gain reference distance; increasing it extends the full-gain region.
int gtl_ISoundBuffer_SetMinDistance(SetterOwner* self, int unused, float value) {
    self->target->minDistance = value;
    return 1;
}

// Store the rolloff cap/culling boundary; flag bit 17 chooses mute versus capped gain.
int gtl_ISoundBuffer_SetMaxDistance(SetterOwner* self, int unused, float value) {
    self->target->maxDistance = value;
    return 1;
}

// Borrow a curve ADDRESS (or zero for default rolloff), with no AddRef/copy.
// Kept as int to preserve the existing Wii ABI; this is not a numeric curve kind.
int gtl_ISoundBuffer_SetDistanceCurve(SetterOwner* self, int curveAddressBits) {
    self->target->distanceCurve = curveAddressBits;
    return 1;
}

// Store spatial gate/Doppler scale A; zero bypasses pan, attenuation AND Doppler.
// Nonzero A*B scales projected velocities; safe numeric limits are unestablished.
int gtl_ISoundBuffer_SetDistanceParameterA(SetterOwner* self, int unused, float value) {
    self->target->distanceParameterA = value;
    return 1;
}

// Store Doppler factor B; zero keeps unit pitch while preserving spatial gain/pan.
// Nonzero B combines with A; the consumer's supersonic branches bound the ratio.
int gtl_ISoundBuffer_SetDistanceParameterB(SetterOwner* self, int unused, float value) {
    self->target->distanceParameterB = value;
    return 1;
}

// Store coordinate mode: 0 listener-relative transform, 2 bypass, others origin-based.
int gtl_ISoundBuffer_SetSpatialMode(SetterOwner* self, int value) {
    self->target->spatialMode = value;
    return 1;
}

// 0x80400D44, 0x804013F8, 0x804013FC are bare tail calls: the argument in r3 is forwarded.
// Release the supplied audio allocation.
void dtor_80400D44(void* p) {
    gtl_Audio_Free(p);
}

// Allocate the requested number of bytes from the audio allocator.
void* gtl_Audio_AllocStorage(unsigned int size) {
    return gtl_Audio_Alloc(size);
}

// Release the supplied audio allocation.
void dtor_804013FC(void* p) {
    gtl_Audio_Free(p);
}

// Return the playback-storage address immediately after the buffer flags.
void* gtl_ISoundBuffer_GetPlaybackHeader(void* p) {
    return ((SoundBufferOwner*)p)->data->playbackStorage;
}

// Initialize the first audio angular-period constant to two pi.
void InitAudioTwoPiConstantA() {
    s_audioTwoPiA = kFloat_6_2831855;
}

// Initialize the second audio angular-period constant to two pi.
void InitAudioTwoPiConstantB() {
    s_audioTwoPiB = kFloat_6_2831855;
}

enum SoundEffectMode { kSoundEffectDisabled = 0, kSoundEffectEnabled = 1 };
enum SoundBufferFlags { kSoundBufferSpatial = 0x10, kSoundBufferSpatialDefaults = 0xA0 };
enum { kEnvelopeFinished = 7, kSpatialModeBypass = 2 };
// Word indices relative to SoundBufferData::entries (+0xBC). Coordinate units
// match the source/listener positions; full cone angles are radians (gtl_audio_spatial_and_runtime.cpp).
// Preserve the integer copy of defaults so float bit patterns are copied verbatim.
enum BufferParameterWord {
    kPositionXWord = 0, kPositionYWord = 1, kPositionZWord = 2,
    kVelocityXWord = 3, kVelocityYWord = 4, kVelocityZWord = 5,
    kForwardXWord = 6, kForwardYWord = 7, kForwardZWord = 8,
    kUpXWord = 9, kUpYWord = 10, kUpZWord = 11,
    kInnerConeAngleWord = 12, kOuterConeAngleWord = 13,
    kInnerConeGainWord = 14, kOuterConeGainWord = 15,
    kDistanceCurveWord = 16, kDistanceParameterAWord = 17, kDistanceParameterBWord = 18,
    kMinDistanceWord = 19, kMaxDistanceWord = 20, kSpatialModeWord = 21,
    kBufferParameterWords = 22, kBufferStateBytes = 0x1B8,
    kBufferUnitScaleCount = 5, kBufferLoopWordCount = 3, kBufferSendCount = 6
};

struct SoundEffectState {
    int refs;
    void* vtable;
    int enabled;
    int reserved;
    void* storage;
};

struct ModeInterface {
    virtual ~ModeInterface();
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void modeOne();
    virtual void modeZero();
};

// Switch the borrowed embedded SDK effect, then remember its accepted mode in
// required self. object names the effect subobject (typically wrapper +14), not
// the wrapper's reference-count/vtable prefix. No retain, allocation or deletion.
// mode=1 calls enable, 0 calls disable; other values return 0 with state untouched,
// except an already-equal cached mode returns 1 before validation. Unchanged mode
// makes no virtual call. Virtual returns are ignored; status 1 means dispatch,
// not a report that SDK setup succeeded. External callers must serialize audio use.
int gtl_SoundEffect_SetEnabled(int* self, ModeInterface* object, int mode) {
    if (((SoundEffectState*)self)->enabled == mode) return 1;
    if (mode == kSoundEffectEnabled) object->modeOne();
    else if (mode == kSoundEffectDisabled) object->modeZero();
    else return 0;
    ((SoundEffectState*)self)->enabled = mode;
    return 1;
}

// SoundBufferOwner/Data layouts: classes/GtlSoundBuffer.h.
struct RefVtable { void* slot0; void* slot1; void (*destroy)(void*, int); };
struct RefObject { int refs; RefVtable* vtable; };
extern const unsigned int s_soundBufferParameterDefaults[22];
extern void* __vt__Q23gtl12ISoundBuffer[3];
extern const char defaultAudioEnvelopeDescription[];
extern const char defaultAudioLfoParameters[];
extern const char kAudioBufferScaleDefaults[];
void gtl_AudioEnvelope_Initialize(void*, const char*);
void gtl_AudioLfo_Initialize(void*, const char*);
void gtl_AudioScale_Initialize(void*, const char*);

// Allocate default buffer state, initialize modulation, copy source flags, and enable spatial defaults.
SoundBufferOwner* gtl_ISoundBuffer_Ctor(SoundBufferOwner* self, unsigned int* source) {
    self->refs = 1;
    self->vtable = &__vt__Q23gtl12ISoundBuffer;
    SoundBufferData* data = (SoundBufferData*)gtl_Audio_Alloc(kBufferStateBytes);
    if (data != 0) {
        data->flags = 0;
        data->activeVoice = 0;
        data->refObject = 0;
        for (int i = 0; i < kBufferParameterWords; ++i) data->entries[i] = s_soundBufferParameterDefaults[i];
        gtl_AudioEnvelope_Initialize(data->envelope, defaultAudioEnvelopeDescription); // original data +0x114
        gtl_AudioLfo_Initialize(data->lfo, defaultAudioLfoParameters); // original +0x150
        gtl_AudioScale_Initialize(data->bufferScaleState, kAudioBufferScaleDefaults); // original +0x178
        float one = 1.0f, zero = 0.0f;
        // +0x180: volume, listener volume, pitch, listener pitch, pan (all initially one).
        // +0x194: loop requested/start/end; +0x1A0: main/aux/remote[4] sends.
        // Each array retains its original store order and count.
        for (int i = 0; i < kBufferUnitScaleCount; ++i) data->volumePitchAndPan[i] = one;
        for (int i = 0; i < kBufferLoopWordCount; ++i) data->loopParameters[i] = 0;
        for (int i = 0; i < kBufferSendCount; ++i) data->outputSendLevels[i] = zero;
    }
    self->data = data;
    data->flags = *source;
    if ((data->flags & kSoundBufferSpatial) != 0) data->flags |= kSoundBufferSpatialDefaults;
    return self;
}

// Stop playback, release the voice and reference object under interrupt protection, then free storage.
void* gtl_ISoundBuffer_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        ((SoundBufferOwner*)self)->vtable = (void*)__vt__Q23gtl12ISoundBuffer;
        gtl_ISoundBuffer_Stop(self, 0);
        SoundBufferData* data = ((SoundBufferOwner*)self)->data;
        if (data != 0) {
            char* voiceSlot = (char*)&data->activeVoice;
            if (voiceSlot != 0) {
                BOOL levelSlot;
                BOOL* levelPtr = &levelSlot;
                levelSlot = OSDisableInterrupts();
                void* voice = *(void**)voiceSlot;
                if (voice == 0) OSRestoreInterrupts(levelSlot);
                else {
                    GtlVoice_Stop(voice);
                    OSRestoreInterrupts(*levelPtr);
                }
                void* ref = data->refObject;
                if (ref != 0) {
                    int count = --*(int*)ref;
                    if (count == 0) ((RefVtable*)((RefObject*)ref)->vtable)->destroy(ref, 1);
                    data->refObject = 0;
                }
            }
            gtl_Audio_Free(data);
        }
        ((SoundBufferOwner*)self)->data = 0;
        if (shouldDelete > 0) gtl_Audio_Free(self);
    }
    return self;
}

struct CallbackOwner {
    char reserved00[8];
    void* object;
};

struct InterfaceA {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2(int, int);
    virtual void slot3();
    virtual void slot4(int, int);
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
};

struct InterfaceB {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
};

// Initialize an already-constructed effect wrapper and its embedded SDK processor.
// self and object required; object is normally self+14 (Create in gtl_sound_effect_reverb_hi_part3.cpp etc.).
// Call order: clear self-owned work storage; virtual +10 with (0,0); attach object
// to controller group 0; virtual +1C. The latter is also used by enable mode 0
// and Finalize below. Slot names stay numeric because SDK-wide postconditions
// have not been recovered; attachment and call order are the established contract.
// Return status 1 unconditionally; virtual results/errors are not checked. Does
// not initialize refs/enabled/vtable: factory already did so. No rollback on throw.
// storage must initially be empty: overwriting an old pointer here leaks it.
// Controller receives a borrowed subobject; wrapper must outlive controller use.
int gtl_SoundEffect_Initialize(void* self, InterfaceA* object) {
    ((SoundEffectState*)self)->storage = 0;
    object->slot2(0, 0);
    gtl_AudioController_AttachEffect(gtl_AudioController_GetInstance(), 0, object);
    object->slot5();
    return 1;
}

// Stop/finalize an embedded SDK effect BEFORE freeing wrapper-owned work bytes.
// Required self/object have the same relationship as Initialize. Call virtual
// +1C then +14, then gtl_Audio_Free(storage) and clear it. The subobject and wrapper
// themselves are not freed; the caller's destructor handles them afterwards.
// No refcount/enabled-word change or controller-detach call occurs here. Caller
// must stop concurrent playback/controller use before destroying the wrapper.
// No status/exception handling; a throwing virtual leaves storage allocated.
// storage==0 only skips the free: virtual calls still run, so repeat safety
// depends on the SDK processor, not on this function's pointer clearing.
void gtl_SoundEffect_Finalize(void* self, InterfaceB* object) {
    object->slot5();
    object->slot3();
    if (((SoundEffectState*)self)->storage != 0) {
        gtl_Audio_Free(((SoundEffectState*)self)->storage);
        ((SoundEffectState*)self)->storage = 0;
    }
}

struct Triple {
    void* a;
    void* b;
    void* c;
};

struct PairOwner {
    Triple* first;
    Triple* second;
};

// Allocate empty factory and effect-instance vectors for a registry.
PairOwner* gtl_EffectRegistry_InitStorage(PairOwner* self) {
    Triple* first = (Triple*)__nw__FUl(12);
    if (first != 0) {
        first->a = 0;
        first->b = 0;
        first->c = 0;
    }
    self->first = first;
    Triple* second = (Triple*)__nw__FUl(12);
    if (second != 0) {
        second->a = 0;
        second->b = 0;
        second->c = 0;
    }
    self->second = second;
    return self;
}

// Only modes 0 and 1 stop the voice (0x80401130-0x80401160: mode > 1 and mode < 0 branch to 0x80401160);
// GtlVoice_Release takes only the voice pointer.
// Release voices for callback modes zero/one and clear the buffer voice slot for every mode.
void gtl_ISoundBuffer_VoiceCallback(void* arg, int mode, CallbackOwner* self) {
    switch (mode) {
    case 0: GtlVoice_Release(arg); break;
    case 1: GtlVoice_Release(arg); break;
    }
    ((SoundBufferData*)self->object)->activeVoice = 0;
}

struct CallbackObject {
    virtual int callback0();
    virtual int callback1();
    virtual int callback2();
    virtual int callback3();
};

// The original calls vtable slot +0xC (callback1), 0x80401330-0x8040133C; this called +0x14.
// Write the interface enable state as 0/1 to required result; return status 1.
// The first ABI argument is unused. object is borrowed and must support the
// query at vtable +0xC; neither its lifetime nor enable state changes here.
int gtl_SoundEffect_GetEnabled(void*, CallbackObject* object, int* result) {
    int value = object->callback1();
    if (value != 0) {
        *result = 1;
    } else {
        *result = 0;
    }
    return 1;
}

// This update uses the same owner/data layout as construction and destruction.
typedef SoundBufferOwner InterruptOwner;

struct CleanupSelf { void* child; RegistryVectors* pair; };

// Fade both effect groups, release every instance reference, and free registry vectors and storage.
void gtl_EffectRegistry_Shutdown(CleanupSelf* self) {
    if (self->child == 0) return;
    // Phase 1: fade both controller groups before tearing down registered effects.
    gtl_AudioController_FadeOutEffects(gtl_AudioController_GetInstance(), 0, 0);
    gtl_AudioController_FadeOutEffects(gtl_AudioController_GetInstance(), 1, 0);
    RegistryVectors* vectors = (RegistryVectors*)self->pair;
    if (vectors != 0) {
        vectors->factories->count = 0;
        InitVector* instances = vectors->instances;
        RefObject** items = (RefObject**)instances->data;
        RefObject** end = items + (int)instances->count;
        for (; items != end; ++items) {
            RefObject* item = *items;
            if (item != 0) {
                // Global shutdown forcibly drains ALL references, including external ones.
                // Callers must have stopped using effects before shutdown; this is not
                // an ordinary single-reference release. The zero transition deletes item.
                int refs;
                do {
                    refs = --item->refs;
                    if (refs == 0) {
                        if (item != 0) item->vtable->destroy(item, 1);
                        refs = 0;
                    } else refs = item->refs;
                } while (refs > 0);
            }
            *items = 0;
        }
        vectors->instances->count = 0;
    }
    // Phase 2: free both vectors after their payloads have been destroyed.
    vectors = (RegistryVectors*)self->pair;
    if (vectors != 0) {
        for (int i = 0; i < 2; ++i) {
            InitVector* block = (&vectors->factories)[i];
            if (block != 0) {
                if (block->data != 0) {
                    block->count = 0;
                    gtl_Audio_Free((void*)block->data);
                }
                delete block;
            }
        }
        gtl_Audio_Free(vectors);
    }
    self->pair = 0;
    self->child = 0;
}

// Update envelope and LFO state for an active voice while interrupts are disabled.
void gtl_ISoundBuffer_UpdateModulation(InterruptOwner* self, int value) {
    SoundBufferData* target = ((SoundBufferOwner*)self)->data;
    int savedValue = value;
    BOOL level;
    level = OSDisableInterrupts();
    if (target->activeVoice != 0) {
        gtl_AudioEnvelope_Update(target->envelope, savedValue);
        gtl_AudioLfo_Update(target->lfo, savedValue);
    }
    OSRestoreInterrupts(level);
}

struct AudioTarget {
    unsigned int flags; char reserved04[0x98]; int voiceHandle;
    char reservedA0[0x1C]; unsigned int spatialParameters[21]; int spatialMode;
    int envelopeState; float envelopeLevel; char reserved11C[0x10]; int envelopeEnabled;
    char reserved130[0x24]; float pitchModulation; char reserved158[0x28];
    float volumeScale; float spatialVolume; float pitchScale; float spatialPitch; float pan;
};
struct AudioSelf { char reserved00[8]; AudioTarget* target; };
extern float kFloat_0_SpatialAudio;
extern float kFloat_0_SoundBuffers;
extern float kFloat_1_AudioSpatial;

// Spatial result slots are pan, surround pan, volume multiplier, pitch multiplier.
// Pan offsets are converted to voice coordinates by the constants used below.
enum SpatialResultSlot { kSpatialPan = 0, kSpatialSurroundPan = 1,
                         kSpatialVolumeScale = 2, kSpatialPitchScale = 3 };
// In gtl_audio_spatial_and_runtime.cpp the calculator treats this argument as a boolean: nonzero silences
// sources at/beyond maximumDistance. This differs from bit 4, which enables spatial audio.
enum { kCullAtMaximumDistanceFlagShift = 17 };

// Combine envelope, LFO, and spatial parameters, stop finished envelopes, and update the voice.
void gtl_ISoundBuffer_UpdateParameters(AudioSelf* self) {
    AudioTarget* t = self->target;
    if (t->voiceHandle == 0) return;
    void* listener = 0;
    if ((t->flags & kSoundBufferSpatial) != 0) listener = gtl_Audio_GetListener();
    t = self->target;
    int cullAtMaximumDistance = (t->flags >> kCullAtMaximumDistanceFlagShift) & 1;
    BOOL outer;
    // The callback can clear the voice slot. Recheck after acquiring this outer
    // guard; keep stop, spatial calculation and the final pan writes in its scope.
    outer = OSDisableInterrupts();
    if (t->voiceHandle != 0) {
        int state = t->envelopeState;
        float factor = t->envelopeEnabled ? t->envelopeLevel : (state == kEnvelopeFinished ? 0.0f : 1.0f);
        if (state == kEnvelopeFinished && factor == 0.0f) {
            // Inlined stop has its own guard. Restore the inner saved level first
            // (still disabled), then the caller's outer level before returning.
            BOOL inner = OSDisableInterrupts();
            if (t->voiceHandle != 0) GtlVoice_Stop((void*)t->voiceHandle);
            OSRestoreInterrupts(inner);
            OSRestoreInterrupts(outer);
            return;
        }
        t->spatialVolume = 1.0f;
        t->spatialPitch = 1.0f;
        float pan = t->pan;
        float surroundPan = kFloat_0_SoundBuffers;
        // A missing/bypassed listener or failed calculation leaves unit multipliers
        // and the buffer's pan defaults; only success (1) consumes the result array.
        if (listener != 0 && t->spatialMode != kSpatialModeBypass) {
            float spatialResult[4];
            if (gtl_Audio_CalculateSpatialParameters(listener, t->spatialParameters, spatialResult, cullAtMaximumDistance) == 1) {
                t->spatialVolume = spatialResult[kSpatialVolumeScale];
                t->spatialPitch = spatialResult[kSpatialPitchScale];
                pan = spatialResult[kSpatialPan] + kFloat_0_SpatialAudio;
                surroundPan = spatialResult[kSpatialSurroundPan] + kFloat_1_AudioSpatial;
            }
        }
        // Volume and pitch setters each retain their inlined guard and voice recheck.
        // Nested restores preserve the outer disabled state; do not merge scopes.
        BOOL level = OSDisableInterrupts();
        if (t->voiceHandle != 0) {
            float f = t->envelopeEnabled ? t->envelopeLevel : (t->envelopeState == kEnvelopeFinished ? 0.0f : 1.0f);
            gtl_nw4r_detail_Voice_SetVolumeScale((void*)t->voiceHandle, t->volumeScale * t->spatialVolume * f);
        }
        OSRestoreInterrupts(level);
        level = OSDisableInterrupts();
        if (t->voiceHandle != 0) GtlVoice_SetPitchScale((void*)t->voiceHandle, t->pitchScale * t->spatialPitch * t->pitchModulation);
        OSRestoreInterrupts(level);
        GtlVoice_SetPan((void*)t->voiceHandle, pan);
        GtlVoice_SetSurroundPan((void*)t->voiceHandle, surroundPan);
    }
    OSRestoreInterrupts(outer);
}

#pragma dont_inline on

// The integer parameter is unused; callers incorrectly declare the success result void.
// Store FULL inner/outer angles in radians; integer ABI argument is unused.
// Ordinary cones use 0 <= inner <= outer <= 2*pi; 2*pi is omnidirectional.
// No clamp/swap: CalculateConeGain halves these angles when comparing direction.
int gtl_ISoundBuffer_SetConeAngles(void* self, int unused, float innerRadians, float outerRadians) {
    SetterTarget* state = ((SetterOwner*)self)->target;
    state->innerConeAngle = innerRadians;
    state->outerConeAngle = outerRadians;
    return 1;
}

// The integer parameter is unused; callers incorrectly declare the success result void.
// Store unitless inner/outer amplitude gains; integer ABI argument is unused.
// Consumer uses inner inside inner cone, outer outside outer cone and a blend
// between. No clamping: zero mutes, one is unit gain; safe boost limit unestablished.
int gtl_ISoundBuffer_SetConeGains(void* self, int unused, float innerGain, float outerGain) {
    SetterTarget* state = ((SetterOwner*)self)->target;
    state->innerConeGain = innerGain;
    state->outerConeGain = outerGain;
    return 1;
}

extern unsigned int __vt__Q23gtl14ISoundResource[];
// Initialize reference-counted sound-resource ownership and allocate its audio data.
void* gtl_ISoundResource_Ctor(void* self, void* resource) {
    unsigned int* words = (unsigned int*)self;
    words[0] = 1;
    ((void**)self)[1] = __vt__Q23gtl14ISoundResource;
    ((void**)self)[2] = 0;
    ((void**)self)[3] = resource;
    void* data = ((void* (*)(void*))gtl_Audio_Alloc)(resource);
    ((void**)self)[2] = data;
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046269C", _ctors_u80400B80_0, InitAudioTwoPiConstantA);
STATIC_INIT_ENTRY(".CRT$XCU8046269C", _ctors_u80400B80_1, InitAudioTwoPiConstantB);
#pragma pop
