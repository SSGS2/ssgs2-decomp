// Destroy for the standard reverb effect (CSoundEffectReverbStd): restores its interfaces, finalizes
// the embedded NW4R reverb effect and frees the wrapper when asked.
//
// The enable setter, getter and release helper are in gtl_sound_effect_reverb_std_part2.cpp.

#include "classes/SoundEffectReverbStdState.h"

// View of the embedded nw4r reverb effect; its vtable is at +0x0.
struct FxReverbStd {
    virtual void vfunc0();
    virtual void vfunc1();
    virtual void vfunc2();
    virtual void vfunc3();
    virtual void vfunc4();
    virtual void vfunc5();
};

extern "C" {
extern void* __vt__Q23gtl21CSoundEffectReverbStd[];
extern void* __vt__Q33gtl4nw4r11FxReverbStd[];
void gtl_SoundEffect_Finalize(gtl::CSoundEffectReverbStd*, FxReverbStd*);
void gtl_Audio_Free(void*);
}

// Finalize the wrapper and embedded reverb, then optionally free the wrapper.
// Accepts a null object; shouldDelete > 0 releases it through the audio allocator.
// Returns the original address, which may already be freed. The embedded effect's
// cleanup slots run in the order 5 then 3.
gtl::CSoundEffectReverbStd* gtl::CSoundEffectReverbStd::Destroy(int shouldDelete) {
    if (this) {
        // Phase 1: restore the wrapper interface and detach/finalize its effect.
        vtable = __vt__Q23gtl21CSoundEffectReverbStd;
        gtl_SoundEffect_Finalize(this, (FxReverbStd*)effect);
        if ((FxReverbStd*)effect) {
            // Phase 2: restore the embedded interface and invoke its cleanup slots.
            *(void***)effect = __vt__Q33gtl4nw4r11FxReverbStd;
            ((FxReverbStd*)effect)->vfunc5();
            ((FxReverbStd*)effect)->vfunc3();
        }
        if (shouldDelete > 0) {
            // Phase 3: only the deleting form releases the outer allocation.
            gtl_Audio_Free(this);
        }
    }
    return this;
}
