// Translation unit 0x803FA564-0x803FA670 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/SoundEffectReverbHiState.h"

extern "C" {

void* gtl_nw4r_FxReverbHi_GetParams(void*);

enum ReverbParameterSelector {
    kParameter0, kParameter1, kParameter2, kParameter3, kParameter4, kParameter5,
    kAllReverbParameters
};

// Read current reverb words and stage requested values locally, then always return 1.
// inputValue borrows float* for selectors 0..5, or a pointer to float* for selector 6.
// The six-float order matches the companion getter, but no setter or write-back
// occurs here: stagedParameters dies on return. Do not infer an audible change.
// The volatile six-word snapshot forces reads even though it is otherwise unused.
// The 28-byte local stride is preserved; it is not the SDK's packed float[6] layout.
int gtl_CSoundEffectReverbHi_vfunc_3(void* self, unsigned int selector, void*, void* inputValue) {
	struct Param { float value; char pad[24]; } stagedParameters[6];
	unsigned int* currentParameterWords = (unsigned int*)gtl_nw4r_FxReverbHi_GetParams(&AsSoundEffectReverbHiState(self)->effectVtable); // 0x803FA564: embedded effect at +0x14
	volatile unsigned int snapshotWords[6] = { currentParameterWords[0], currentParameterWords[1], currentParameterWords[2], currentParameterWords[3], currentParameterWords[4], currentParameterWords[5] };
	switch (selector) {
	case kParameter0: stagedParameters[0].value = *(float*)inputValue; break;
	case kParameter1: stagedParameters[1].value = *(float*)inputValue; break;
	case kParameter2: stagedParameters[2].value = *(float*)inputValue; break;
	case kParameter3: stagedParameters[3].value = *(float*)inputValue; break;
	case kParameter4: stagedParameters[4].value = *(float*)inputValue; break;
	case kParameter5: stagedParameters[5].value = *(float*)inputValue; break;
	case kAllReverbParameters: {
		float* allParameters = (float*)*(void**)inputValue;
		stagedParameters[0].value = allParameters[0]; stagedParameters[1].value = allParameters[1];
		stagedParameters[2].value = allParameters[2]; stagedParameters[3].value = allParameters[3];
		stagedParameters[4].value = allParameters[4]; stagedParameters[5].value = allParameters[5];
		break;
	}
	}
	return 1;
}

} // extern "C"
