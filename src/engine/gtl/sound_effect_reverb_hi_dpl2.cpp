// DPL2 high reverb sound effect: the wrapper factory and size query, the startup copy of the
// range constants, and the standard reverb destructor (CSoundEffectReverbHiDpl2::Create,
// ReverbHiDpl2_InitModuleConstants and gtl_nw4r_FxReverbStd_Dtor).

#include <decomp/static_init.h>
#include "classes/SoundEffectReverbHiDpl2State.h"
// Audio: DPL2 high-reverb wrapper factory, range-cache startup, and standard
// reverb destruction (0x803FAB68..0x803FAD14). Companion: the data section.
// For audible DPL2 controls use FxReverbHiDpl2_SetParams (fx_reverb_hi_dpl2.cpp) and the
// selector interface in sound_effect_reverb_hidpl2.cpp; startup here copies range endpoints and a two-pi constant.
// Standard-reverb release/clear hooks: nw4r_fx_reverb_std_shutdown.cpp / gtl_nw4r_fx_reverb_std_part1.cpp.

extern "C" {

extern float kFloat_6_2831855;
extern float kReverbHiDpl2Range0Minimum;  // kReverbHiDpl2Range0Minimum.
extern float kReverbHiDpl2Range0Maximum;  // kReverbHiDpl2Range0Maximum.
extern float kReverbHiDpl2Range1Minimum;  // kReverbHiDpl2Range1Minimum.
extern float kReverbHiDpl2Range1Maximum;  // kReverbHiDpl2Range1Maximum.
extern float kReverbHiDpl2Range2Minimum;  // kReverbHiDpl2Range2Minimum.
extern float kReverbHiDpl2Range2Maximum;  // kReverbHiDpl2Range2Maximum.
extern float kReverbHiDpl2Range3Minimum;  // kReverbHiDpl2Range3Minimum.
extern float kReverbHiDpl2Range3Maximum;  // kReverbHiDpl2Range3Maximum.
extern float kReverbHiDpl2Range4Minimum;  // kReverbHiDpl2Range4Minimum.
extern float kReverbHiDpl2Range4Maximum;  // kReverbHiDpl2Range4Maximum.
extern float kReverbHiDpl2Range5Minimum;  // kReverbHiDpl2Range5Minimum.
extern float kReverbHiDpl2Range5Maximum;  // kReverbHiDpl2Range5Maximum.
extern float s_reverbHiDpl2TwoPi;         // s_reverbHiDpl2TwoPi.
extern float s_reverbHiDpl2Range0Minimum; // s_reverbHiDpl2Range0Minimum.
extern float s_reverbHiDpl2Range0Maximum; // s_reverbHiDpl2Range0Maximum.
extern float s_reverbHiDpl2Range1Minimum; // s_reverbHiDpl2Range1Minimum.
extern float s_reverbHiDpl2Range1Maximum; // s_reverbHiDpl2Range1Maximum.
extern float s_reverbHiDpl2Range2Minimum; // s_reverbHiDpl2Range2Minimum.
extern float s_reverbHiDpl2Range2Maximum; // s_reverbHiDpl2Range2Maximum.
extern float s_reverbHiDpl2Range3Minimum; // s_reverbHiDpl2Range3Minimum.
extern float s_reverbHiDpl2Range3Maximum; // s_reverbHiDpl2Range3Maximum.
extern float s_reverbHiDpl2Range4Minimum; // s_reverbHiDpl2Range4Minimum.
extern float s_reverbHiDpl2Range4Maximum; // s_reverbHiDpl2Range4Maximum.
extern float s_reverbHiDpl2Range5Minimum; // s_reverbHiDpl2Range5Minimum.
extern float s_reverbHiDpl2Range5Maximum; // s_reverbHiDpl2Range5Maximum.
extern void __dl__FPv(void*);
extern void* gtl_Audio_Alloc(unsigned int);
extern void gtl_nw4r_FxReverbHiDpl2_Ctor(void*);
extern void gtl_SoundEffect_Initialize(void*, void*);
extern void* __vt__Q23gtl24CSoundEffectReverbHiDpl2[];
extern void* __vt__Q33gtl4nw4r11FxReverbStd[];

} // extern "C"

// Wrapper prefix (+0..+0x10) owns an embedded DPL2 processor at +0x14, not a pointer
// to a separately allocated effect. ownedStorage is an optional separate allocation.
// Create allocates the entire 0x1D4-byte wrapper; its normal finalizer handles nested resources.
enum {
    kReverbHiDpl2ObjectBytes = sizeof(gtl::CSoundEffectReverbHiDpl2),
    kReleaseEffectSlot = 7,
    kClearOutputBufferSlot = 5
};

namespace gtl {

// Report the complete audio-effect wrapper size; fail only for a null output pointer.
// The first argument is unused; output receives bytes, including the embedded DSP and wrapper prefix.
int CSoundEffectReverbHiDpl2::GetObjectSize(void*, unsigned int* output) {
    if (!output)
        return 0;
    *output = kReverbHiDpl2ObjectBytes;
    return 1;
}

// Allocate and initialize a DPL2 high-reverb wrapper; allocation failure is returned through *output.
// Return 0 means output itself was null. Return 1 can still leave *output == 0 on
// allocation failure; on success the caller owns the wrapper and must use its finalizer.
int CSoundEffectReverbHiDpl2::Create(void*, CSoundEffectReverbHiDpl2** output) {
    if (!output)
        return 0;
    CSoundEffectReverbHiDpl2** outputSlot = output;
    CSoundEffectReverbHiDpl2* object = (CSoundEffectReverbHiDpl2*)gtl_Audio_Alloc(kReverbHiDpl2ObjectBytes);
    if (object) {
        // Initialize wrapper bookkeeping before constructing/registering the embedded processor.
        object->referenceCount = 1;
        object->effectEnabled = 0;
        object->registryIndex = -1;
        object->ownedStorage = 0;
        object->vtable = __vt__Q23gtl24CSoundEffectReverbHiDpl2;
        gtl_nw4r_FxReverbHiDpl2_Ctor(object->effect);
        gtl_SoundEffect_Initialize(object, object->effect);
    }
    *outputSlot = object;
    return 1;
}

}

extern "C" {

// Copy the reverb module constants into mutable runtime storage at startup.
// The companion defines Range0=[0,0.1], Range1=[0,10], Range2..5=[0,1].
// DPL2's PUBLIC parameter order is preDelay, time, coloration, damping,
// crosstalk, mix (ReverbParams in fx_reverb_hi_dpl2.cpp); SDK storage uses another order.
// preDelay/time use the SDK time scale (physical units unproven here); the
// other controls are dimensionless. These cache ordinals cannot be renamed to
// controls just from equal limits: no named cache reader proves the mapping.
// For a control mod edit SetParams/clamps and its GetParams interface together;
// changing these startup copies alone does not establish an audible effect.
// s_reverbHiDpl2TwoPi is the leading float in an eight-byte slot, not a double.
void ReverbHiDpl2_InitModuleConstants() {
    s_reverbHiDpl2TwoPi = kFloat_6_2831855;
    s_reverbHiDpl2Range0Minimum = kReverbHiDpl2Range0Minimum;
    s_reverbHiDpl2Range0Maximum = kReverbHiDpl2Range0Maximum;
    s_reverbHiDpl2Range1Minimum = kReverbHiDpl2Range1Minimum;
    s_reverbHiDpl2Range1Maximum = kReverbHiDpl2Range1Maximum;
    s_reverbHiDpl2Range2Minimum = kReverbHiDpl2Range2Minimum;
    s_reverbHiDpl2Range2Maximum = kReverbHiDpl2Range2Maximum;
    s_reverbHiDpl2Range3Minimum = kReverbHiDpl2Range3Minimum;
    s_reverbHiDpl2Range3Maximum = kReverbHiDpl2Range3Maximum;
    s_reverbHiDpl2Range4Minimum = kReverbHiDpl2Range4Minimum;
    s_reverbHiDpl2Range4Maximum = kReverbHiDpl2Range4Maximum;
    s_reverbHiDpl2Range5Minimum = kReverbHiDpl2Range5Minimum;
    s_reverbHiDpl2Range5Maximum = kReverbHiDpl2Range5Maximum;
}

// Release standard-reverb SDK resources, clear output buffers, then optionally free it.
// This destructor belongs to FxReverbStd despite sharing the DPL2 factory unit.
// Slot 7 releases SDK state; slot 5 clears output buffers (gtl_nw4r_fx_reverb_std_part1.cpp), rather
// than shutting down the SDK. Keep that order and the vtable reload between
// calls. self may be null; shouldDelete > 0 additionally frees self. A returned
// nonnull address then dangles. No separate allocation is returned to the caller.
void* gtl_nw4r_FxReverbStd_Dtor(void* self, int shouldDelete) {
    if (self) {
        void** vtable = __vt__Q33gtl4nw4r11FxReverbStd;
        *(void**)self = vtable;
        ((void (*)(void*))vtable[kReleaseEffectSlot])(self);
        vtable = *(void***)self;
        ((void (*)(void*))vtable[kClearOutputBufferSlot])(self);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462678", _ctors_u803FAB68_0,
                  ReverbHiDpl2_InitModuleConstants); // ReverbHiDpl2_InitModuleConstants.
#pragma pop
