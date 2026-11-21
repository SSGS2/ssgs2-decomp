// Translation unit 0x803FA3F0-0x803FA564 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/SoundEffectReverbHiState.h"
extern "C" {

void gtl_Audio_Free(void*);
void gtl_SoundEffect_SetEnabled(void*, void*, void*);
void gtl_SoundEffect_GetEnabled(void*, void*, void*);
void gtl_SoundEffect_Finalize(void*, void*);
void __dl__FPv(void*);

struct FxReverbHi {
	virtual void v0();
	virtual void v1();
	virtual void v2();
	virtual void destroy();
	virtual void v5();
	virtual void deinit();
};

extern char __vt__Q33gtl4nw4r10FxReverbHi[];
extern char __vt__Q23gtl20CSoundEffectReverbHi[];

// Free an audio-allocated base wrapper only when deleting > 0; return its former address.
void* dtor_803FA3F0(void* self, int deleting) {
	if (self && deleting > 0)
		gtl_Audio_Free(self);
	return self;
}

// Deinitialize and destroy the standalone effect, then optionally free it with operator delete.
void* FxReverbHi_dtor(void* self, int deleting) {
	if (self) {
		*(void**)self = &__vt__Q33gtl4nw4r10FxReverbHi;
		((FxReverbHi*)self)->deinit();
		((FxReverbHi*)self)->destroy();
		if (deleting > 0)
			__dl__FPv(self);
	}
	return self;
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Finalize the owner and embedded effect, then optionally free the owner on the audio heap.
// effectVtable at +0x14 is the start of the embedded object, not a pointer to a separate effect.
void* gtl_CSoundEffectReverbHi_dtor(void* self, int deleting) {
	if (self) {
		AsSoundEffectReverbHiState(self)->vtable = &__vt__Q23gtl20CSoundEffectReverbHi;
		// Detach/finalize the owner before dismantling the embedded implementation.
		gtl_SoundEffect_Finalize(self, &AsSoundEffectReverbHiState(self)->effectVtable);
		if (&AsSoundEffectReverbHiState(self)->effectVtable) {
			AsSoundEffectReverbHiState(self)->effectVtable = &__vt__Q33gtl4nw4r10FxReverbHi;
			((FxReverbHi*)(&AsSoundEffectReverbHiState(self)->effectVtable))->deinit();
			((FxReverbHi*)(&AsSoundEffectReverbHiState(self)->effectVtable))->destroy();
		}
		// The owner uses the audio allocator; the standalone destructor above uses delete.
		if (deleting > 0)
			gtl_Audio_Free(self);
	}
	return self;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

// Tail-forward to audio free; the original ABI carries the allocation in incoming r3.
// The argument-less reconstruction intentionally preserves that forwarding thunk.
void dtor_803FA548(void* object) { gtl_Audio_Free(object); }

// Forward enable mode (integer 0/1 in r4, represented by the legacy void* signature).
// The implementation borrows the embedded effect; no ownership changes.
void gtl_CSoundEffectReverbHi_SetEnabled(void* self, void* enabledMode) {
	gtl_SoundEffect_SetEnabled(self, &AsSoundEffectReverbHiState(self)->effectVtable, enabledMode);
}

// Write enable state as an int 0/1 to the caller's borrowed output address.
void gtl_CSoundEffectReverbHi_GetEnabled(void* self, void* enabledOutput) {
	gtl_SoundEffect_GetEnabled(self, &AsSoundEffectReverbHiState(self)->effectVtable, enabledOutput);
}

} // extern "C"
