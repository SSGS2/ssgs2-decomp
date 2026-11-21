#include <decomp/static_init.h>
// Translation unit 0x8040A8A8-0x8040AB50 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/FxReverbHiFlags.h"
extern "C" {

// Object layout: +0x0C is the allocator/controller context, +0x18 holds raw
// requested controls, and +0x30 starts SDK work (including the trailing settings).
// Legacy outputBuffer/outputBufferBytes at +4/+8 are only cleared here; the
// controller's actual external-buffer word is at object +0x10 (allocatorState[0]).
struct FxReverbHiDpl2Layout {
	unsigned int vtable;
	unsigned int effectListLink0;
	unsigned int effectListLink1;
	unsigned char initializationAttempted; // Latched after an initialization attempt, including SDK failure.
	unsigned char pad0d[3];
	unsigned int controllerWords10And14[2]; // +10 borrowed output buffer; +14 only cleared here, role unresolved.
	// Public GetParams order; raw requests are retained even when SDK settings clamp.
	float requestedParameters[6]; // [0] preDelay [1] decayTime [2] coloration [3] damping [4] crosstalk [5] mix
	unsigned char sdkWork[0x178];
	float coloration;
	float mix;
	float decayTime;
	float damping;
	float preDelay;
	float crosstalk;
};


extern float kFloat_6_2831855;
extern float g_audioReverbFullTurnAngle;
extern float kFloat_0_1_ReverbPreDelay;
extern float kFloat_0_Reverb;
extern float kFloat_10_Reverb;
extern float kFloat_1_Reverb;
extern unsigned int g_audioEffectAllocatedBytes;
// Six external IEEE-754 words in requestedParameters order. This unit does not
// define their values: edit the defining table to change defaults, and keep the
// clamps below synchronized with the SDK contract. Word loads/reinterpretation
// preserve bit patterns (including NaNs); these are not integer-to-float conversions.
extern unsigned int s_reverbHiDpl2DefaultParameters[];
extern unsigned int __vt__Q33gtl4nw4r14FxReverbHiDpl2[];

void* AudioEffect_SetOutputBuffer(void*, int, unsigned int);
void* AudioController_ClearOutputBuffer(void*);
void gtl_AudioEffects_InstallAllocatorHooks(void*, unsigned int*, unsigned int*);
void gtl_AudioEffects_RestoreAllocatorHooks(void*, unsigned int, unsigned int);
int AXFXReverbHiInitDpl2(void*);
void AXFXReverbHiSettingsDpl2(void*);

// Virtual slot 1 queries the attempt latch; slot 4 below performs the attempt.
// Report whether initialization was attempted; a set latch does not prove SDK success.
unsigned int gtl_nw4r_FxReverbHi_vfunc_1(void* self) {
	return AsFxReverbHiFlags(self)->initializationAttempted != 0;
}

// Initialize SDK reverb work under the effect allocator hooks and latch initialization.
// self is caller-owned storage. Returns the SDK success predicate on the first
// attempt, then 1 on later calls even if that first attempt failed. No rollback.
unsigned int gtl_nw4r_FxReverbHiDpl2_InitializeOnce(void* self) {
	FxReverbHiDpl2Layout* base = (FxReverbHiDpl2Layout*)self;
	if (AsFxReverbHiFlags(base)->initializationAttempted)
		return 1;
	unsigned int savedAllocateHook, savedFreeHook;
	gtl_AudioEffects_InstallAllocatorHooks(&base->initializationAttempted, &savedAllocateHook, &savedFreeHook);
	g_audioEffectAllocatedBytes = 0;
	int result = AXFXReverbHiInitDpl2(base->sdkWork);
	gtl_AudioEffects_RestoreAllocatorHooks(&base->initializationAttempted, savedAllocateHook, savedFreeHook);
	base->initializationAttempted = 1;
	return result != 0;
}



// Copy default controls and clamp them to the SDK parameter ranges.
// Constructs in caller-provided storage and returns self; no allocation here.
// Time controls use the SDK scale (physical units unverified here); the other
// four controls are dimensionless [0,1]. NaNs pass through ordinary comparisons.
FxReverbHiDpl2Layout* gtl_nw4r_FxReverbHiDpl2_Ctor(FxReverbHiDpl2Layout* self) {
	// Establish the vtable, clear the attempt latch and reset controller bookkeeping.
	self->effectListLink0 = 0;
	self->effectListLink1 = 0;
	self->vtable = (unsigned int)__vt__Q33gtl4nw4r14FxReverbHiDpl2;
	self->initializationAttempted = 0;
	self->controllerWords10And14[0] = 0;
	self->controllerWords10And14[1] = 0;
	// Preserve the six default float bit patterns in public GetParams order.
	volatile unsigned int* source = (volatile unsigned int*)s_reverbHiDpl2DefaultParameters;
	unsigned int preDelayBits = source[0];
	unsigned int decayTimeBits = source[1];
	unsigned int colorationBits = source[2];
	unsigned int dampingBits = source[3];
	unsigned int crosstalkBits = source[4];
	unsigned int mixBits = source[5];
	float requestedPreDelay = *(float*)&preDelayBits;
	float requestedDecayTime = *(float*)&decayTimeBits;
	float requestedColoration = *(float*)&colorationBits;
	float requestedDamping = *(float*)&dampingBits;
	float requestedCrosstalk = *(float*)&crosstalkBits;
	float requestedMix = *(float*)&mixBits;
	self->requestedParameters[0] = requestedPreDelay;
	self->requestedParameters[1] = requestedDecayTime;
	self->requestedParameters[2] = requestedColoration;
	self->requestedParameters[3] = requestedDamping;
	self->requestedParameters[4] = requestedCrosstalk;
	self->requestedParameters[5] = requestedMix;
	// Build bounded SDK settings; keep raw requested controls unchanged above.
	float boundedParameter = requestedPreDelay;
	if (boundedParameter > kFloat_0_1_ReverbPreDelay) boundedParameter = kFloat_0_1_ReverbPreDelay;
	else if (boundedParameter < kFloat_0_Reverb) boundedParameter = kFloat_0_Reverb;
	self->preDelay = boundedParameter;
	boundedParameter = requestedDecayTime;
	if (boundedParameter > kFloat_10_Reverb) boundedParameter = kFloat_10_Reverb;
	else if (boundedParameter < kFloat_0_Reverb) boundedParameter = kFloat_0_Reverb;
	self->decayTime = boundedParameter;
	boundedParameter = requestedColoration;
	if (boundedParameter > kFloat_1_Reverb) boundedParameter = kFloat_1_Reverb;
	else if (boundedParameter < kFloat_0_Reverb) boundedParameter = kFloat_0_Reverb;
	self->coloration = boundedParameter;
	boundedParameter = requestedDamping;
	if (boundedParameter > kFloat_1_Reverb) boundedParameter = kFloat_1_Reverb;
	else if (boundedParameter < kFloat_0_Reverb) boundedParameter = kFloat_0_Reverb;
	self->damping = boundedParameter;
	boundedParameter = requestedCrosstalk;
	if (boundedParameter > kFloat_1_Reverb) boundedParameter = kFloat_1_Reverb;
	else if (boundedParameter < kFloat_0_Reverb) boundedParameter = kFloat_0_Reverb;
	self->crosstalk = boundedParameter;
	boundedParameter = requestedMix;
	if (boundedParameter > kFloat_1_Reverb) boundedParameter = kFloat_1_Reverb;
	else if (boundedParameter < kFloat_0_Reverb) boundedParameter = kFloat_0_Reverb;
	self->mix = boundedParameter;
	// Retain the original guarded refresh path; the constructor cleared the latch.
	if (self->initializationAttempted) {
		unsigned int savedAllocateHook, savedFreeHook;
		gtl_AudioEffects_InstallAllocatorHooks(&((FxReverbHiDpl2Layout*)self)->initializationAttempted, &savedAllocateHook, &savedFreeHook);
		AXFXReverbHiSettingsDpl2(((FxReverbHiDpl2Layout*)self)->sdkWork);
		gtl_AudioEffects_RestoreAllocatorHooks(&((FxReverbHiDpl2Layout*)self)->initializationAttempted, savedAllocateHook, savedFreeHook);
	}
	return self;
}

// Store a borrowed external buffer address; the middle ABI argument is ignored downstream.
// Legacy pointer return declaration is retained; the callee actually returns integer 1.
void* gtl_nw4r_FxReverbHiDpl2_vfunc_2(void* self, int unused, unsigned int bufferAddress) {
	return AudioEffect_SetOutputBuffer(&((FxReverbHiDpl2Layout*)self)->initializationAttempted, unused, bufferAddress);
}

// Detach the embedded effect controller output buffer.
// Does not free that borrowed buffer. The callee is void; ignore this legacy return value.
void* gtl_nw4r_FxReverbHiDpl2_DetachOutputBuffer(void* self) {
	return AudioController_ClearOutputBuffer(&((FxReverbHiDpl2Layout*)self)->initializationAttempted);
}

// Initialize the shared full-turn angle constant during static startup.
void AudioReverb_InitFullTurnAngle() {
	g_audioReverbFullTurnAngle = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804626D4", _ctors_u8040A8A8_0, AudioReverb_InitFullTurnAngle);
#pragma pop
