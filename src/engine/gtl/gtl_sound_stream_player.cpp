#include <decomp/static_init.h>
#include "classes/SoundStreamLoopFormat.h" // gtl_ISoundStream_Play, 0x804024B0: embedded +0x548 format
#include "classes/SoundStreamSource.h"
#define AsSoundStreamPlayer804022BC(p) ((SoundStreamPlayer804022BC*)(p))
#include "classes/SoundStreamPlayer804022BC.h"
#define AsSoundStreamHandle804022BC(p) ((SoundStreamHandle804022BC*)(p))
#include "classes/SoundStreamHandle804022BC.h"
// Translation unit 0x804022BC-0x804033F8 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

#include "sdk_protos.h"

extern void __dl__FPv(void*);
extern void __destroy_arr(void*, void*, unsigned long, unsigned long);
extern void gtl_IntrusiveList_Dtor(void*, int);
extern void dtor_804033F8(void*, int);
extern void InterruptGuard_dtor(void*);

extern void gtl_Audio_Free(void*);
extern void* gtl_AudioEnvelope_Initialize(void*, ...);
extern void* gtl_AudioLfo_Initialize(void*, ...);
extern void* gtl_AudioScale_Initialize(void*, ...);
extern void* gtl_Audio_Alloc(unsigned int);
extern void gtl_nw4r_detail_StrmPlayer_Ctor(void*);
extern void fn_804033F8(void*); // Destroy the constructed stream player and its parameter blocks.
extern void IUnknownCustom_dtor(void*);
extern void* __vt__Q23gtl12ISoundStream[];
extern void* s_soundBufferParameterDefaults[];
extern char defaultAudioEnvelopeDescription[0x28];
extern char defaultAudioLfoParameters[0x18];
extern char kAudioBufferScaleDefaults[0x10];
extern float kFloat_1_AudioBufferDefaults;
extern float kFloat_0_SoundStreamPlayer;
extern float s_soundStreamFullTurnRadians;
extern float kFloat_6_2831855;
extern float kFloat_0_AudioResource;
extern float kFloat_0_SoundBuffers;
extern float kFloat_0_SpatialAudio;
extern float kFloat_1_AudioSpatial;
extern void* gtl_AudioController_GetInstance();
extern void gtl_nw4r_detail_StrmPlayer_PreparePlayback(void*, void*, int, int, int, int, int, void*);
extern void gtl_AudioEnvelope_Reset(void*);
extern void gtl_AudioLfo_Reset(void*);
extern float GtlAudio_GetMainSend();
// Native parameter arrays: the callee's shared layout has not yet decoded their bus meanings.
extern void GtlBasicPlayer_SetAuxiliarySend(void*, int, float); // Three-element array: default send, then two zeros.
extern void GtlBasicPlayer_SetAdditionalOutputMixControl(void*, int, float); // Third four-element array: replicate the default send.
extern void GtlBasicPlayer_SetRemoteVolume(void*, int, float); // First four-element array: copy the per-channel levels.
extern void GtlBasicPlayer_SetRemoteSendOffset(void*, int, float); // Second four-element array: initialize all channels to one.
extern int gtl_Audio_GetListener();
extern int gtl_Audio_CalculateSpatialParameters(int, void*, void*, int);

// Release an audio allocation through the audio allocator callback.
void dtor_8040336C(void* self) {
	gtl_Audio_Free(self);
}

// Report support for this stream capability.
int SoundStream_DefaultCapabilityTrue() {
	return 1;
}

// Initialize this audio unit's full-turn angle constant.
void SoundStream_InitFullTurnConstant() {
	s_soundStreamFullTurnRadians = kFloat_6_2831855;
}


// Playback interface state starts at player +8; scalar mix parameters follow its vtable.
struct NativeStreamMix {
    void* vtable;
    unsigned int reserved;
    float volume, pitch, pan, surroundPan;
    char paddingToScale[8];
    float outputScale;
    char paddingToOutputMask[4];
    unsigned int outputMask;
    char paddingToMainSend[4];
    float mainSend;
};
// Addresses into the player remain byte cursors to preserve CodeWarrior scheduling.
enum {
    kStreamNativePlayer = 8, kStreamLoopFormat = 0x548,
    kStreamSpatialDefaults = 0x608, kStreamEnvelope = 0x660,
    kStreamLfo = 0x69C, kStreamScale = 0x6C4,
    kStreamMixBegin = 0x6CC, kStreamMixEnd = 0x6DC,
    kStreamLoopBegin = 0x6E0, kStreamLoopEnd = 0x6E8,
    kStreamSendsBegin = 0x6EC, kStreamSendsEnd = 0x700,
    kStreamAuxSends = 0x6F4, kStreamSourceInfo = 0x5F0,
    kAudioControllerStreamState = 0xF4,
    kStreamAllowPitch = 0x20, kStreamAllowVolume = 0x80,
    kStreamUseListener = 0x10, kStreamLoop = 1,
    kStreamClearPendingStartAtReturn = 0x10000, kStreamStartPrepared = 0x20000,
    kStreamFadeStopping = 7, kStreamStopFade = 1, kStreamStopLooping = 2
};

// Native vtable indices include RTTI/null header entries. Status names deliberately describe
// observed bytes, not inferred states: vfunc_4/5/6/7 read AC/AE/AD/B0 respectively.
// See __vt__Q43gtl4nw4r6detail10StrmPlayer in phase1_class_tables.cpp.
enum NativeStreamSlot {
    kNativeStartSlot = 3, kNativeStopSlot = 4,
    kNativeStatusACSlot = 6, kNativeStatusAESlot = 7,
    kNativeStatusADSlot = 8, kNativeStatusB0Slot = 9
};
// CSoundStreamSource table words include RTTI/null headers; these operations borrow their outputs.
enum StreamSourceSlot {
    kSourceFillPlaybackHeaderSlot = 4, // vfunc_2 writes GetPlaybackHeader(self).
    kSourceQueryPointerSlot = 5       // vfunc_3 copies +0x9C; pointee semantics unresolved.
};
enum { kStreamBlockAlignment = 32, kStreamRemoteChannelCount = 4, kStreamSpatialBypassMode = 2 };
// A view over send1..send4; ownership stays with the enclosing player.
// Biased cursor preserves native load addressing as the channel cursor advances.
struct StreamRemoteLevels { char playerPrefix[kStreamAuxSends]; float channelLevel; };

enum PublicStreamStatus {
    kStatusFallback = 8, kStatusAEWithAC = 1,
    kStatusACAndAD = 0x10000, kStatusB0WithACAndAD = 0x20000,
    kStatusACWithoutAEOrAD = 0x40000, kStatusLoopRequested = 0x80000
};

struct State804022BC {
	char paddingPrefix[0x8];
	void* state;
	char paddingAfterState[0xA0];
	unsigned char flagAC; // Required for all AC-dependent public status bits.
	unsigned char flagAD; // Nonzero blocks start; combines with AC in GetStatusFlags.
	unsigned char flagAE; // Must be nonzero before a start is attempted.
	char paddingBeforeStatus[1];
	unsigned char flagB0; // Adds a public bit only when AC and AD are both nonzero.
};

struct Values804022BC {
	char paddingPrefix[0x638];
	float innerConeAngle;
	float outerConeAngle;
	float innerConeGain;
	float outerConeGain;
	int distanceCurve;
	float distanceParameter1;
	float distanceParameter2;
	float minimumDistance;
	float maximumDistance;
	char paddingBeforeVolume[0x70];
	float volume;
	char paddingBeforePitch[0x4];
	float pitch;
	char paddingBeforePan[0x4];
	float pan;
	char paddingBeforeLoopRange[0x4];
	int loopStart;
	int loopEnd;
};

// At source +8: name, sourceData, sampleRate, format/channelCount, sampleCount, streamData.
// Pointer-typed scalar storage is copied verbatim; it does not imply ownership.
struct StreamInfo804022BC {
	char paddingPrefix[0x8];
	void* name;
	void* sourceData;
	void* sampleRate;
	unsigned short format;
	unsigned short channelCount;
	void* sampleCount;
	void* streamData;
};

// Borrowed output with the same seven metadata fields.
struct StreamInfoOut804022BC {
	void* name;
	void* sourceData;
	void* sampleRate;
	unsigned short format;
	unsigned short channelCount;
	void* sampleCount;
	void* streamData;
};

struct SoundResource804022BC {
	void* referenceCountWord;
	void* vtable;
	void* resource;
	void* trailingResourceState;
};

// Native player-owned list and channel-entry regions used only during destruction.
struct NativePlayerOwnedStorage {
    char prefix[0xFC];
    char firstList[0xC];
    char secondList[0xC];
    char channelEntries[0x20][0x1C];
};
struct StrmPlayer804022BC {
	char paddingPrefix[8];
	void* state;
	char paddingAfterState[0x5E0];
	void* source;
	char paddingAfterSource[0x208];
};

struct StreamData804022BC {
	char paddingPrefix[0x5E8];
	void* source;
	void* source2;
	char paddingAfterSource[0x14];
	char spatialParameters[0x5C];
	char envelopeAndMixParameters[0xA4];
	float allocationEndMarker;
};

struct ConstructedStreamData804022BC {
	StreamData804022BC data;
	// Construct native playback and local parameter blocks with default mix/send values.
	ConstructedStreamData804022BC() {
		data.source = 0;
		data.source2 = 0;
		gtl_nw4r_detail_StrmPlayer_Ctor((char*)&data + 8);
		data.source = 0;
		data.source2 = 0;
		for (int i = 0; i < 11; ++i) ((void**)((char*)&data + 0x604))[i] = s_soundBufferParameterDefaults[i];
		gtl_AudioEnvelope_Initialize((char*)&data + kStreamEnvelope, &defaultAudioEnvelopeDescription);
		gtl_AudioLfo_Initialize((char*)&data + kStreamLfo, &defaultAudioLfoParameters);
		gtl_AudioScale_Initialize((char*)&data + kStreamScale, &kAudioBufferScaleDefaults);
		for (int i = kStreamMixBegin; i <= kStreamMixEnd; i += 4) *(float*)((char*)&data + i) = kFloat_1_AudioBufferDefaults;
		for (int i = kStreamLoopBegin; i <= kStreamLoopEnd; i += 4) *(int*)((char*)&data + i) = 0;
		for (int i = kStreamSendsBegin; i <= kStreamSendsEnd; i += 4) *(float*)((char*)&data + i) = kFloat_0_SoundStreamPlayer;
	}
	// Destroy the embedded native player/parameters, leaving this enclosing storage to its caller.
	~ConstructedStreamData804022BC() { fn_804033F8(&data); }
};

extern void* __vt__Q23gtl14ISoundResource[];

// Free the owned audio resource and optionally the reference-counted wrapper.
void* gtl_ISoundResource_dtor(SoundResource804022BC* self, int freeMemory) {
	if (self != 0) {
		self->vtable = __vt__Q23gtl14ISoundResource;
		if (self->resource != 0) {
			gtl_Audio_Free(self->resource);
			self->resource = 0;
		}
		self->trailingResourceState = 0;
		if (freeMemory > 0) gtl_Audio_Free(self);
	}
	return self;
}

struct ResourceVtable804022BC {
	virtual void destroy(void*, int) = 0;
	virtual void unused1() = 0;
	virtual void unused2() = 0;
	virtual void setState(int) = 0;
	virtual int queryStatusAC() = 0; // Active AC-byte query.
	virtual int queryStatusAE() = 0; // Active AE-byte query.
	virtual int queryStatusAD() = 0; // Active AD-byte query.
	virtual int queryStatusB0() = 0;
	// Active B0-byte query.
};

struct SoundStreamInterruptGuard {
	BOOL level;
	// Save the current interrupt level and disable interrupts for this lexical scope.
	SoundStreamInterruptGuard() : level(OSDisableInterrupts()) {}
	// Restore exactly the saved level, allowing guards to nest.
	~SoundStreamInterruptGuard() { OSRestoreInterrupts(level); }
};

// Pause the active native stream while interrupts are disabled; fail if none is attached.
int gtl_ISoundStream_Pause(void* self) {
	void* player = AsSoundStreamHandle804022BC(self)->player;
	SoundStreamInterruptGuard guard;
	void* resource = AsSoundStreamPlayer804022BC(player)->stream;
	if (resource == 0) return 0;
	((ResourceVtable804022BC*)resource)->setState(1);
	return 1;
}

// Resume the active native stream while interrupts are disabled; fail if none is attached.
int gtl_ISoundStream_Resume(void* self) {
	void* player = AsSoundStreamHandle804022BC(self)->player;
	SoundStreamInterruptGuard guard;
	void* resource = AsSoundStreamPlayer804022BC(player)->stream;
	if (resource == 0) return 0;
	((ResourceVtable804022BC*)resource)->setState(0);
	return 1;
}

// Destroy 32 native-player entries and both intrusive lists, then optionally free the player.
void* StrmPlayer_dtor(StrmPlayer804022BC* self, int freeMemory) {
	if (self != 0) {
		__destroy_arr((char*)&((NativePlayerOwnedStorage*)self)->channelEntries, (void*)dtor_804033F8, 0x1C, 0x20);
		if ((char*)&((NativePlayerOwnedStorage*)self)->secondList != 0) gtl_IntrusiveList_Dtor((char*)&((NativePlayerOwnedStorage*)self)->secondList, 0);
		if ((char*)&((NativePlayerOwnedStorage*)self)->firstList != 0) gtl_IntrusiveList_Dtor((char*)&((NativePlayerOwnedStorage*)self)->firstList, 0);
		if (freeMemory > 0) __dl__FPv(self);
	}
	return self;
}

// Reference-counted stream source: the count comes first, the class table follows.
struct SourceBase804022BC {
	int refs;
	virtual void destroy(int) = 0;
	virtual int attach(void*) = 0;
};

// Release the previous source, retain the replacement and attach its stream-description output.
// The replacement gains a reference even if attach returns failure. Null detaches and returns 0.
// Caller must hold its own replacement reference if it aliases the previously attached source.
int gtl_ISoundStream_SetSource(void* self, const SourceBase804022BC* source) {
	StrmPlayer804022BC* player = (*(StrmPlayer804022BC**)&AsSoundStreamHandle804022BC(self)->player);
	if (player->source != 0) {
		int refs = *(int*)player->source - 1;
		*(int*)player->source = refs;
		if (refs == 0) {
		((SourceBase804022BC*)player->source)->destroy(1);
		}
		player->source = 0;
	}
	if (source == 0) return 0;
	player->source = (void*)source;
	++*(int*)source;
	return ((SourceBase804022BC*)player->source)->attach((char*)player + kStreamSourceInfo) != 0;
}

// Request fade-out, cancel looping, or stop immediately according to the stop mode.
int gtl_ISoundStream_Stop(void* self, int mode) {
	void* player = AsSoundStreamHandle804022BC(self)->player;
	void* state = AsSoundStreamPlayer804022BC(player)->stream;
	if (state == 0) return 0;
	switch (mode) {
	case kStreamStopFade:
		AsSoundStreamPlayer804022BC(player)->fadeStartFactor = AsSoundStreamPlayer804022BC(player)->fadeFactor;
		AsSoundStreamPlayer804022BC(player)->fadeState = kStreamFadeStopping;
		return 1;
	case kStreamStopLooping:
		(*(int*)&AsSoundStreamPlayer804022BC(player)->loopRequested) = 0;
		return 1;
	default:
		break;
	}
	{
		SoundStreamInterruptGuard guard;
		state = AsSoundStreamPlayer804022BC(player)->stream;
		if (state != 0) ((void (**)(void*))*(void**)state)[4](state);
	}
	player = AsSoundStreamHandle804022BC(self)->player;
	AsSoundStreamPlayer804022BC(player)->stream = 0;
	(*(void**)&AsSoundStreamPlayer804022BC(player)->startPending) = 0;
	return 1;
}

// Set pitch only when the source advertises pitch control.
int gtl_ISoundStream_SetPitch(State804022BC* self, float value) {
	Values804022BC* values = (Values804022BC*)self->state;
	if ((*(unsigned int*)values & kStreamAllowPitch) == 0) return 0;
	values->pitch = value;
	return 1;
}

// Optionally delete a source wrapper that owns no additional resources.
void* dtor_80402C3C(void* self, int freeMemory) {
	if (self != 0 && freeMemory > 0) __dl__FPv(self);
	return self;
}

// Copy the source format description to the caller; fail for a null output.
int gtl_CSoundStreamSource_GetStreamInfo(const StreamInfo804022BC* self, StreamInfoOut804022BC* out) {
	if (out == 0) return 0;
	void* sourceName;
	void* sourceData;
	void* sampleRate;
	unsigned short encoding;
	unsigned short channelCount;
	void* sampleCount;
	void* streamData;
	// Keep the original cast-away-const access so CodeWarrior preserves load scheduling.
	sourceName = ((StreamInfo804022BC*)self)->name;
	sourceData = ((StreamInfo804022BC*)self)->sourceData;
	sampleRate = ((StreamInfo804022BC*)self)->sampleRate;
	encoding = ((StreamInfo804022BC*)self)->format;
	channelCount = ((StreamInfo804022BC*)self)->channelCount;
	sampleCount = ((StreamInfo804022BC*)self)->sampleCount;
	streamData = ((StreamInfo804022BC*)self)->streamData;
	out->name = sourceName;
	out->sourceData = sourceData;
	out->sampleRate = sampleRate;
	out->format = encoding;
	out->channelCount = channelCount;
	out->sampleCount = sampleCount;
	out->streamData = streamData;
	return 1;
}

// Set gain factors inside and outside the spatial-audio cone.
int gtl_ISoundStream_SetConeGains(State804022BC* self, int unused, float value1, float value2) {
	Values804022BC* values = (Values804022BC*)self->state;
	values->innerConeGain = value1;
	values->outerConeGain = value2;
	return 1;
}

// Initialize the envelope when enabled; disabling returns null without changing it.
void* gtl_ISoundStream_SetEnvelope(State804022BC* self, int enabled) {
	void* state = self->state;
	if (enabled == 0) return 0;
	// The settings pointer arrives as `enabled` and is still in r4 on the Wii: pass it on.
	return ((void* (*)(void*, void*))gtl_AudioEnvelope_Initialize)((char*)state + kStreamEnvelope, (void*)enabled);
}

// Set volume only when the source advertises volume control.
int gtl_ISoundStream_SetVolume(State804022BC* self, float value) {
	Values804022BC* values = (Values804022BC*)self->state;
	if ((*(unsigned int*)values & kStreamAllowVolume) == 0) return 0;
	values->volume = value;
	return 1;
}

// Return whether native player status byte AE is set; its broader state meaning is unresolved.
bool gtl_nw4r_detail_StrmPlayer_vfunc_5(State804022BC* self) {
	return self->flagAE != 0;
}

// Store the loop start and end sample positions.
int gtl_ISoundStream_SetLoopRange(State804022BC* self, int value1, int value2) {
	Values804022BC* values = (Values804022BC*)self->state;
	values->loopStart = value1;
	values->loopEnd = value2;
	return 1;
}

// Store the inner and outer spatial-audio cone angles.
int gtl_ISoundStream_SetConeAngles(State804022BC* self, int unused, float value1, float value2) {
	Values804022BC* values = (Values804022BC*)self->state;
	values->innerConeAngle = value1;
	values->outerConeAngle = value2;
	return 1;
}

// Store the stream's horizontal pan.
int gtl_ISoundStream_SetPan(State804022BC* self, float value) {
	((Values804022BC*)self->state)->pan = value;
	return 1;
}

// Return whether native player status byte AC is set; its broader state meaning is unresolved.
bool gtl_nw4r_detail_StrmPlayer_vfunc_4(State804022BC* self) {
	return self->flagAC != 0;
}

// Store the minimum spatial attenuation distance.
int gtl_ISoundStream_SetMinDistance(State804022BC* self, int unused, float value) {
	((Values804022BC*)self->state)->minimumDistance = value;
	return 1;
}

// Return whether native player status byte AD is set; its broader state meaning is unresolved.
bool gtl_nw4r_detail_StrmPlayer_vfunc_6(State804022BC* self) {
	return self->flagAD != 0;
}

// Return whether native player status byte B0 is set; its broader state meaning is unresolved.
bool gtl_nw4r_detail_StrmPlayer_vfunc_7(State804022BC* self) {
	return self->flagB0 != 0;
}

// Store the maximum spatial attenuation distance.
int gtl_ISoundStream_SetMaxDistance(State804022BC* self, int unused, float value) {
	((Values804022BC*)self->state)->maximumDistance = value;
	return 1;
}

// Select the spatial attenuation curve.
int gtl_ISoundStream_SetDistanceCurve(State804022BC* self, int value) {
	((Values804022BC*)self->state)->distanceCurve = value;
	return 1;
}

// Store the first additional distance-curve parameter; exact curve semantics remain unresolved.
int gtl_ISoundStream_SetDistanceParameterA(State804022BC* self, int unused, float value) {
	((Values804022BC*)self->state)->distanceParameter1 = value;
	return 1;
}

// Store the second additional distance-curve parameter; exact curve semantics remain unresolved.
int gtl_ISoundStream_SetDistanceParameterB(State804022BC* self, int unused, float value) {
	((Values804022BC*)self->state)->distanceParameter2 = value;
	return 1;
}

// Output-mask bits from the send levels: bit 0 from +0x6EC, bits 1-4 from +0x6F4..+0x700.
// The original skips +0x6F0 (0x8040264C-0x804026B0: lfs 0x6ec, 0x6f4, 0x6f8, 0x6fc, 0x700).
static unsigned int OutputBits804022BC(char* player) {
	unsigned int bits = 0;
	if (AsSoundStreamPlayer804022BC(player)->mainSend > 0.0f) bits |= 1;
	if (AsSoundStreamPlayer804022BC(player)->send1 > 0.0f) bits |= 2;
	if (AsSoundStreamPlayer804022BC(player)->send2 > 0.0f) bits |= 4;
	if (AsSoundStreamPlayer804022BC(player)->send3 > 0.0f) bits |= 8;
	if (AsSoundStreamPlayer804022BC(player)->send4 > 0.0f) bits |= 0x10;
	return bits;
}

// Fade factor: +0x664 while a fade runs (+0x678 != 0), else 0 when stopping (+0x660 == 7), else 1.
// Original 0x80402A40-0x80402A74 and 0x80402B60-0x80402B88.
static float Fade804022BC(char* player) {
	if (AsSoundStreamPlayer804022BC(player)->fadeRunning != 0) return AsSoundStreamPlayer804022BC(player)->fadeFactor;
	return AsSoundStreamPlayer804022BC(player)->fadeState == kStreamFadeStopping ? 0.0f : 1.0f;
}

// Apply loop-end sample/block arithmetic and final-block byte alignment.
// Scratch references preserve the caller's variable lifetimes; only the embedded format is published.
static inline void ConfigureStreamLoopExtent(char*& state, unsigned int& loopEndSamples, bool& loop, unsigned int& loopStartSamples, unsigned int& previousBlockCount, unsigned int& samplesPerBlock) {
    if (loop && loopStartSamples != loopEndSamples && loopEndSamples) {
        AsSoundStreamLoopFormat(state)->loopStart = loopStartSamples;
        AsSoundStreamLoopFormat(state)->loopEnd = loopEndSamples;
        samplesPerBlock = AsSoundStreamLoopFormat(state)->samplesPerBlock;
        previousBlockCount = AsSoundStreamLoopFormat(state)->blockCount;
        unsigned int requiredBlockCount = (samplesPerBlock + loopEndSamples - 1) / samplesPerBlock;
        AsSoundStreamLoopFormat(state)->blockCount = requiredBlockCount;
        AsSoundStreamLoopFormat(state)->lastBlockSamples = loopEndSamples - (loopEndSamples / AsSoundStreamLoopFormat(state)->samplesPerBlock) * AsSoundStreamLoopFormat(state)->samplesPerBlock;
        if (previousBlockCount != requiredBlockCount) AsSoundStreamLoopFormat(state)->lastBlockBytes = AsSoundStreamLoopFormat(state)->blockBytes;
    }
    if (AsSoundStreamLoopFormat(state)->lastBlockSamples == 0) {
        AsSoundStreamLoopFormat(state)->lastBlockSamples = AsSoundStreamLoopFormat(state)->samplesPerBlock;
        AsSoundStreamLoopFormat(state)->lastBlockBytes = AsSoundStreamLoopFormat(state)->blockBytes;
    }
    AsSoundStreamLoopFormat(state)->lastBlockAlignedBytes = (AsSoundStreamLoopFormat(state)->lastBlockBytes + kStreamBlockAlignment - 1) & ~(kStreamBlockAlignment - 1);
}

#pragma push
#pragma inline_depth(16)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Stop a completed fade or apply listener volume, Doppler pitch, and horizontal/surround pan.
// Caller holds the outer interrupt guard; result is four borrowed spatial-result floats.
// Reference parameters retain caller scratch lifetimes; they do not acquire resources.
static inline void ApplyStreamFadeAndSpatialMix(char*& player, void*& stream, int& listener, int& spatialCalculationMode, float& horizontalPan, float& surroundPan, float* result) {
    if (stream) {
        // Original 0x80402A34-0x80402A84: stop only when stopping (+0x660 == 7) and the fade
        // factor is 0; the factor prefers +0x664 while a fade runs.
        if (AsSoundStreamPlayer804022BC(player)->fadeState == kStreamFadeStopping && Fade804022BC(player) == 0.0f) {
            SoundStreamInterruptGuard guard;
            stream = AsSoundStreamPlayer804022BC(player)->stream;
            if (stream) ((void (*)(void*))(*(void***)stream)[kNativeStopSlot])(stream);
        }
        else {
            AsSoundStreamPlayer804022BC(player)->listenerVolume = 1.0f;
            AsSoundStreamPlayer804022BC(player)->listenerDopplerRatio = 1.0f;
            horizontalPan = AsSoundStreamPlayer804022BC(player)->pan;
            surroundPan = 0.0f;
            // Original 0x80402AE4: skipped when there is no listener.
            if (listener != 0 && AsSoundStreamPlayer804022BC(player)->spatialMode != kStreamSpatialBypassMode) {
                if (gtl_Audio_CalculateSpatialParameters(listener, player + kStreamSpatialDefaults, result, spatialCalculationMode) == 1) {
                    AsSoundStreamPlayer804022BC(player)->listenerVolume = result[2];
                    AsSoundStreamPlayer804022BC(player)->listenerDopplerRatio = result[3];
                    horizontalPan = result[0] + 0.0f;
                    surroundPan = result[1] + 1.0f;
                }
            }
            {
                SoundStreamInterruptGuard guard;
                stream = AsSoundStreamPlayer804022BC(player)->stream;
                if (stream) ((NativeStreamMix*)stream)->volume = AsSoundStreamPlayer804022BC(player)->volume * AsSoundStreamPlayer804022BC(player)->listenerVolume * Fade804022BC(player);
            }
            {
                SoundStreamInterruptGuard guard;
                stream = AsSoundStreamPlayer804022BC(player)->stream;
                if (stream) ((NativeStreamMix*)stream)->pitch = AsSoundStreamPlayer804022BC(player)->pitch * AsSoundStreamPlayer804022BC(player)->listenerDopplerRatio * AsSoundStreamPlayer804022BC(player)->pitchScale;
            }
            stream = AsSoundStreamPlayer804022BC(player)->stream;
            ((NativeStreamMix*)stream)->pan = horizontalPan;
            ((NativeStreamMix*)stream)->surroundPan = surroundPan;
        }
    }
}

// Prepare or start playback, configure looping and sends, then apply fade and listener parameters.
// self owns player storage; SetSource retains the source, which must be non-null for fresh playback.
// stream points into player+8 after preparation; clearing/stopping it does not free player storage.
// flags: bit 0 requests looping; 0x20000 takes only the prepared-start branch;
// 0x10000 controls the final startPending assignment (do not move that assignment earlier).
// Prepared start returns the native result; fresh path returns 0 on source-query/attach failure,
// 1 after setup. Failure can occur after stopping the old stream; there is no rollback.
// Loop positions/sizes are samples, bytes are aligned to 32. Format samplesPerBlock must be nonzero.
// Nested interrupt guards protect the native parameter writes and retain their original ordering.
int gtl_ISoundStream_Play(void* self, unsigned int flags) {
	// Phase 1: either start an already prepared stream, or stop/detach the previous native stream.
	char* player = (char*)AsSoundStreamHandle804022BC(self)->player;
	void* source = AsSoundStreamPlayer804022BC(player)->source;
	if (flags & kStreamStartPrepared) {
		void* stream = AsSoundStreamPlayer804022BC(player)->stream;
		if (!stream) return 0;
		void** vt = *(void***)stream;
		if (((int (*)(void*))vt[kNativeStatusADSlot])(stream)) return 0;
		if (!((int (*)(void*))vt[kNativeStatusAESlot])(stream)) return 0;
		return ((int (*)(void*))vt[kNativeStartSlot])(stream);
	}
	if (AsSoundStreamPlayer804022BC(player)->stream != 0) {
		SoundStreamInterruptGuard guard;
		void* stream = AsSoundStreamPlayer804022BC(player)->stream;
		if (stream) ((void (*)(void*))(*(void***)stream)[kNativeStopSlot])(stream);
		AsSoundStreamPlayer804022BC(player)->stream = 0;
		(*(void**)&AsSoundStreamPlayer804022BC(player)->startPending) = 0;
	}
	// Phase 2: query the opaque source pointer, fill the playback header, and set loop block extent.
	AsSoundStreamPlayer804022BC(player)->loopRequested = flags & kStreamLoop;
	unsigned int sourcePointerBits = 0; // vfunc_3 copies a borrowed pointer, not a count.
	// Source layout: reference count at +0, class table at +4. Query does not retain its output.
	void** sourceVt = ((void**) ((SoundStreamSource*)source)->vtable);
	if (!((int (*)(void*, unsigned int*))sourceVt[kSourceQueryPointerSlot])(source, &sourcePointerBits)) return 0;
	sourceVt = ((void**) ((SoundStreamSource*)source)->vtable);
	if (!((int (*)(void*, void*))sourceVt[kSourceFillPlaybackHeaderSlot])(source, self)) return 0;
	char* state = player + kStreamLoopFormat;
	// The loop flag is read back from +0x6E0 and it (not the loop start) gates the loop
	// set-up: original 0x804024B0-0x804024E8 (srwi. on +0x6E0 != 0, beq; cmplw; cmpwi).
	bool loop = AsSoundStreamPlayer804022BC(player)->loopRequested != 0;
	AsSoundStreamPlayer804022BC(player)->loopEnabled = loop;
	unsigned int loopStartSamples = AsSoundStreamPlayer804022BC(player)->loopStart;
	unsigned int loopEndSamples = AsSoundStreamPlayer804022BC(player)->loopEnd;
	unsigned int samplesPerBlock, previousBlockCount;
	ConfigureStreamLoopExtent(state, loopEndSamples, loop, loopStartSamples, previousBlockCount, samplesPerBlock);
	AsSoundStreamPlayer804022BC(player)->stream = player + 8;
	void* audioController = gtl_AudioController_GetInstance();
	gtl_nw4r_detail_StrmPlayer_PreparePlayback(AsSoundStreamPlayer804022BC(player)->stream, (char*)audioController + kAudioControllerStreamState, 0, 0, 1, (int)sourcePointerBits, 0, state);
	// Phase 3: reset envelope/LFO and copy output scale, sends and the three native parameter arrays.
	// Array bus/channel identities remain unresolved; A copies send1..4, B is all ones,
	// C repeats defaultSendParameter. The three-element array gets {defaultSendParameter,0,0}.
	// Original 0x804025A8-0x8040289C: every parameter block is nested inside the first lock.
	{
		SoundStreamInterruptGuard guardA;
		if (AsSoundStreamPlayer804022BC(player)->stream) {
			gtl_AudioEnvelope_Reset(player + kStreamEnvelope);
			gtl_AudioLfo_Reset(player + kStreamLfo);
			{
				SoundStreamInterruptGuard guard;
				void* stream = AsSoundStreamPlayer804022BC(player)->stream;
				// Scale the native player's output before applying sends and channel mix levels.
				if (stream) ((NativeStreamMix*)stream)->outputScale = AsSoundStreamPlayer804022BC(player)->outputScale;
			}
			{
				SoundStreamInterruptGuard guard;
				void* stream = AsSoundStreamPlayer804022BC(player)->stream;
				if (stream) {
					((NativeStreamMix*)stream)->mainSend = AsSoundStreamPlayer804022BC(player)->mainSend;
					SoundStreamInterruptGuard inner;
					stream = AsSoundStreamPlayer804022BC(player)->stream;
					if (stream) ((NativeStreamMix*)stream)->outputMask = OutputBits804022BC(player);
				}
			}
			{
				SoundStreamInterruptGuard guard;
				void* stream = AsSoundStreamPlayer804022BC(player)->stream;
				if (stream) {
					GtlBasicPlayer_SetAuxiliarySend(stream, 0, AsSoundStreamPlayer804022BC(player)->defaultSendParameter);
					GtlBasicPlayer_SetAuxiliarySend(stream, 1, 0.0f);
					GtlBasicPlayer_SetAuxiliarySend(stream, 2, 0.0f);
					for (int i = 0; i < kStreamRemoteChannelCount; ++i) GtlBasicPlayer_SetAdditionalOutputMixControl(stream, i, AsSoundStreamPlayer804022BC(player)->defaultSendParameter);
				}
			}
			{
				SoundStreamInterruptGuard guard;
				void* stream = AsSoundStreamPlayer804022BC(player)->stream;
				if (stream) {
					for (int i = 0; i < kStreamRemoteChannelCount; ++i) GtlBasicPlayer_SetRemoteVolume(stream, i, ((StreamRemoteLevels*)(player + 4*i))->channelLevel);
					for (int i = 0; i < kStreamRemoteChannelCount; ++i) GtlBasicPlayer_SetRemoteSendOffset(stream, i, 1.0f);
					SoundStreamInterruptGuard inner;
					stream = AsSoundStreamPlayer804022BC(player)->stream;
					if (stream) ((NativeStreamMix*)stream)->outputMask = OutputBits804022BC(player);
				}
			}
		}
	}
	// Phase 4: refresh the main send once, then attempt a pending start before spatial/fade updates.
	// Original 0x804028A4: GtlAudio_GetMainSend is called once.
	AsSoundStreamPlayer804022BC(player)->mainSend = GtlAudio_GetMainSend();
	{
		SoundStreamInterruptGuard guard;
		void* stream = AsSoundStreamPlayer804022BC(player)->stream;
		if (stream) {
			((NativeStreamMix*)stream)->mainSend = AsSoundStreamPlayer804022BC(player)->mainSend;
			SoundStreamInterruptGuard inner;
			stream = AsSoundStreamPlayer804022BC(player)->stream;
			if (stream) ((NativeStreamMix*)stream)->outputMask = OutputBits804022BC(player);
		}
	}
	// The original has no second send/bits block here; it goes straight on (0x80402974).
	void* stream = AsSoundStreamPlayer804022BC(player)->stream;
	if (stream) {
		// Original 0x80402984-0x804029FC: only while +4 (start pending) is set, start the stream
		// when status AD is clear (vt+0x20 == 0) and AE is set (vt+0x1C != 0); clear +4
		// when the start call (vt+0xC) returns 1. No lock here.
		if (AsSoundStreamPlayer804022BC(player)->startPending != 0) {
			void** vt = *(void***)stream;
			int started = 0;
			if (!((int (*)(void*))vt[kNativeStatusADSlot])(stream) && ((int (*)(void*))vt[kNativeStatusAESlot])(stream))
				started = ((int (*)(void*))vt[kNativeStartSlot])(stream);
			if (started == 1) AsSoundStreamPlayer804022BC(player)->startPending = 0;
		}
		// Phase 5: resolve listener gains/pan and multiply volume by fade, pitch by stored pitch scale.
		// Original 0x80402A00-0x80402A24: the listener is fetched only when flag 0x10 is set, and
		// the spatialCalculationMode argument is bit 0x20000 (extrwi r26, r0, 1, 14 = >> 17 & 1), not >> 14.
		int listener = 0;
		if ((*(unsigned int*)player & kStreamUseListener) != 0) listener = gtl_Audio_GetListener();
		int spatialCalculationMode = (*(unsigned int*)player >> 17) & 1;
		SoundStreamInterruptGuard guardJ;
		stream = AsSoundStreamPlayer804022BC(player)->stream;
		float horizontalPan, surroundPan;
		float result[4];
		ApplyStreamFadeAndSpatialMix(player, stream, listener, spatialCalculationMode, horizontalPan, surroundPan, result);
	}
	// Original 0x80402BFC: mask 0x10000 (rlwinm r0, r30, 0, 15, 15), not 0x8000.
	AsSoundStreamPlayer804022BC(player)->startPending = (flags & kStreamClearPendingStartAtReturn) == 0;
	return 1;
}

#pragma pop
// Allocate native playback state, initialize mix defaults and inherit source feature flags.
void* gtl_ISoundStream_Ctor(void* self, void* source) {
	*(int*)self = 1;
	*((void**)self + 1) = __vt__Q23gtl12ISoundStream;
	StreamData804022BC* data = (StreamData804022BC*)gtl_Audio_Alloc(0x704);
	if (data != 0) {
		*(int*)data = 0;
		*((int*)data + 1) = 0;
		gtl_nw4r_detail_StrmPlayer_Ctor((char*)data + 8);
		data->source = 0;
		data->source2 = 0;
		for (int i = 0; i < 22; ++i) ((unsigned int*)((char*)data + kStreamSpatialDefaults))[i] = ((unsigned int*)s_soundBufferParameterDefaults)[i];
		gtl_AudioEnvelope_Initialize((char*)data + kStreamEnvelope, &defaultAudioEnvelopeDescription);
		gtl_AudioLfo_Initialize((char*)data + kStreamLfo, &defaultAudioLfoParameters);
		gtl_AudioScale_Initialize((char*)data + kStreamScale, &kAudioBufferScaleDefaults);
		for (int i = kStreamMixBegin; i <= kStreamMixEnd; i += 4) *(float*)((char*)data + i) = kFloat_1_AudioBufferDefaults;
		for (int i = kStreamLoopBegin; i <= kStreamLoopEnd; i += 4) *(int*)((char*)data + i) = 0;
		for (int i = kStreamSendsBegin; i <= kStreamSendsEnd; i += 4) *(float*)((char*)data + i) = kFloat_0_SoundStreamPlayer;
	}
	AsSoundStreamHandle804022BC(self)->player = data;
	*(void**)data = *(void**)source;
	unsigned int state = *(unsigned int*)data;
	if ((state & 0x10) != 0) *(unsigned int*)data = state | 0xA0;
	return self;
}

// Report stream availability, native playback status and requested looping to the caller.
// result borrows one writable word; null returns 0, otherwise writes the complete mask and returns 1.
// Predicate map: queryStatusAC=AC, queryStatusAE=AE, queryStatusAD=AD, queryStatusB0=B0 (native bytes, not decoded states).
// Base = AC&&AD ? 0x10000 (+0x20000 if B0) : 8. Then AC&&AE adds bit 0;
// otherwise AC&&!AD adds 0x40000. loopRequested independently adds 0x80000, even with no stream.
int gtl_ISoundStream_GetStatusFlags(void* self, unsigned int* result) {
	if (result == 0) return 0;
	void* player = AsSoundStreamHandle804022BC(self)->player;
	void* resource = AsSoundStreamPlayer804022BC(player)->stream;
	if (resource == 0) {
		*result = kStatusFallback;
	} else {
		ResourceVtable804022BC* base = (ResourceVtable804022BC*)resource;
		if (base->queryStatusAC() && base->queryStatusAD()) {
			*result = kStatusACAndAD;
			if (base->queryStatusB0()) *result |= kStatusB0WithACAndAD;
		} else {
			*result = kStatusFallback;
		}
		if (base->queryStatusAC()) {
			if (base->queryStatusAE()) *result |= kStatusAEWithAC;
			else if (!base->queryStatusAD()) *result |= kStatusACWithoutAEOrAD;
		}
	}
	if ((*(int*)&AsSoundStreamPlayer804022BC(player)->loopRequested) != 0) *result |= kStatusLoopRequested;
	return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626A4", _ctors_u804022BC_0, SoundStream_InitFullTurnConstant);
#pragma pop
