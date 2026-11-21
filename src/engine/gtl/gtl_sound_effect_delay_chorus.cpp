#include <decomp/static_init.h>
#include "classes/SoundEffectFields803FB370.h"
// Translation unit 0x803FB370-0x803FB5A4 (8 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void gtl_Audio_Free(void*);
void* gtl_Audio_Alloc(unsigned int);
void gtl_SoundEffect_Finalize(void*, void*);
void gtl_SoundEffect_Initialize(void*, void*);
void gtl_nw4r_FxChorus_Ctor(void*);
void gtl_Audio_Free(void*);
void gtl_SoundEffect_SetEnabled(void*, void*, void*);
void gtl_SoundEffect_GetEnabled(void*, void*, void*);
extern unsigned char __vt__Q33gtl4nw4r7FxDelay[];
extern unsigned char __vt__Q23gtl17CSoundEffectDelay[];
extern unsigned char __vt__Q23gtl18CSoundEffectChorus[];
void __dl__FPv(void*);
extern float kFloat_6_2831855;
extern float kFloat_1_AudioEffectsA;
extern float kFloat_50_AudioEffects;
extern float kFloat_0_AudioEffects;
extern float kFloat_1_AudioEffectsB;
extern float kFloat_0_1_AudioEffects;
extern float kFloat_2_AudioEffects;
extern float s_audioEffectTwoPi;
extern float s_audioEffectInitialOneA;
extern float s_audioEffectInitialFifty;
extern float s_audioEffectInitialZero;
extern float s_audioEffectInitialOneB;
extern float s_audioEffectInitialTenth;
extern float s_audioEffectInitialTwo;

enum SoundEffectSizes { kChorusObjectSize = sizeof(SoundEffectFields803FB370) };
// NW4R effect vtables carry RTTI and a base offset before their function slots.
struct DelayDispatchTable {
    void* rtti;
    int baseOffset;
    void (*destruct)(void*);
    void (*slot0)(void*);
    void (*slot1)(void*);
    void (*releaseBuffers)(void*);
    void (*slot3)(void*);
    void (*shutdown)(void*);
};

struct FxDelayDispatch {
    // Supply the ABI slot layout; these empty bodies are dispatch placeholders.
    // They do not model the native effect implementation installed in the vtable.
	// Provide the destructor slot in the local dispatch view.
	virtual ~FxDelayDispatch() {}
	// Preserve native virtual slot 1.
	virtual void unused0() {}
	// Preserve native virtual slot 2.
	virtual void unused1() {}
	// Preserve the teardown dispatch slot used below (table +0x14).
	virtual void unused2() {}
	// Preserve native virtual slot 4.
	virtual void unused3() {}
	// Preserve the teardown dispatch slot used below (table +0x1C).
	virtual void destroy() {}
	// Preserve the remaining unused virtual slot.
	virtual void finalize() {}
};

struct SoundEffectDelayObject {
	char pad[0x14];
	FxDelayDispatch delay;
};


// Report the allocation size required by the chorus wrapper and embedded effect.
// Returns 0 for a null output, otherwise writes bytes and returns 1.
int gtl_CSoundEffectChorus_GetObjectSize(void* unused0, int* objectSize) {
	if (objectSize == 0) return 0;
	*objectSize = kChorusObjectSize;
	return 1;
}

// Initialize this unit's writable copies of the effect tuning constants.
void AudioEffects_InitializeMathConstants() {
	s_audioEffectTwoPi = kFloat_6_2831855;
	s_audioEffectInitialOneA = kFloat_1_AudioEffectsA;
	s_audioEffectInitialFifty = kFloat_50_AudioEffects;
	s_audioEffectInitialZero = kFloat_0_AudioEffects;
	s_audioEffectInitialOneB = kFloat_1_AudioEffectsB;
	s_audioEffectInitialTenth = kFloat_0_1_AudioEffects;
	s_audioEffectInitialTwo = kFloat_2_AudioEffects;
}

// Shut down the delay effect and release its buffers before optional deletion.
// shouldDelete > 0 invokes operator delete. Returns self, possibly freed.
void* gtl_nw4r_FxDelay_dtor(void* self, int shouldDelete) {
	if (self != 0) {
		*(void**)self = __vt__Q33gtl4nw4r7FxDelay;
		((DelayDispatchTable*)__vt__Q33gtl4nw4r7FxDelay)->shutdown(self);
		(*(DelayDispatchTable**)self)->releaseBuffers(self);
		if (shouldDelete > 0) __dl__FPv(self);
	}
	return self;
}

// Return sound-effect storage to the audio allocator.
void dtor_803FB588(void* self) {
	gtl_Audio_Free(self);
}

// Forward the enable request to the embedded delay effect.
// enabledValue carries integer mode 0 or 1 in an ABI pointer slot, not an address.
// The wrapper discards the backend's success/failure result.
void gtl_CSoundEffectDelay_SetEnabled(void* self, void* enabledValue) {
	gtl_SoundEffect_SetEnabled(self, ((SoundEffectFields803FB370*)self)->effect, enabledValue);
}

// Read the enable state of the embedded delay effect.
// enabledOutput is a required borrowed int* receiving 0 or 1 synchronously.
void gtl_CSoundEffectDelay_GetEnabled(void* self, void* enabledOutput) {
	gtl_SoundEffect_GetEnabled(self, ((SoundEffectFields803FB370*)self)->effect, enabledOutput);
}

// Detach the wrapper, destroy its embedded delay, then optionally free storage.
// Embedded effect starts at +0x14; the wrapper is freed only for shouldDelete > 0.
void* gtl_CSoundEffectDelay_dtor(void* self, int shouldDelete) {
	if (self != 0) {
		((SoundEffectFields803FB370*)self)->vtable = __vt__Q23gtl17CSoundEffectDelay;
		SoundEffectDelayObject* object = (SoundEffectDelayObject*)self;
		// Phase 1: detach from processing and release wrapper-owned auxiliary storage.
		gtl_SoundEffect_Finalize(self, &object->delay);
		if (&object->delay != 0) {
			// Phase 2: run native teardown at +0x1C then +0x14; do not free the subobject.
			*(void**)&object->delay = __vt__Q33gtl4nw4r7FxDelay;
			object->delay.destroy();
			object->delay.unused2();
		}
		if (shouldDelete > 0) gtl_Audio_Free(self);
	}
	return self;
}

// Allocate a chorus wrapper, construct its effect, and attach it to audio processing.
// Returns 0 only for a null output slot. Returns 1 even on allocation failure,
// storing null in that case. A non-null result is owned by the caller and must
// be destroyed through the sound-effect interface (which uses gtl_Audio_Free).
int gtl_CSoundEffectChorus_Create(void* unused, void** outChorus) {
	if (outChorus == 0) return 0;
	void* chorus = gtl_Audio_Alloc(kChorusObjectSize);
	if (chorus != 0) {
		// Phase 1: initialize the reference-counted wrapper and effect state.
		// Historical field 'enabled' at +0 is the reference count; effectEnabled
		// at +8 is the enable mode (see SoundEffect_SetEnabled in gtl_sound_buffer_and_effects.cpp).
		((SoundEffectFields803FB370*)chorus)->referenceCount = 1;
		((SoundEffectFields803FB370*)chorus)->effectEnabled = 0;
		((SoundEffectFields803FB370*)chorus)->registryIndex = -1;
		((SoundEffectFields803FB370*)chorus)->auxiliaryStorage = 0;
		((SoundEffectFields803FB370*)chorus)->vtable = __vt__Q23gtl18CSoundEffectChorus;
		// Phase 2: construct the in-place effect, then register it with audio processing.
		void* wrapper = chorus;
		gtl_nw4r_FxChorus_Ctor(((SoundEffectFields803FB370*)chorus)->effect);
		gtl_SoundEffect_Initialize(wrapper, ((SoundEffectFields803FB370*)wrapper)->effect);
	}
	*outChorus = chorus;
	return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462680", _ctors_u803FB370_0, AudioEffects_InitializeMathConstants);
#pragma pop
