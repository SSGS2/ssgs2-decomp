// High-quality reverb sound effect (gtl::CSoundEffectReverbHi): its destructors, destroy,
// and enable and enable-query calls that forward to the embedded reverb effect.

#include "classes/SoundEffectReverbHiState.h"
#include "classes/FxReverbHi.h"
extern "C" {

void gtl_Audio_Free(void*);
void gtl_SoundEffect_SetEnabled(void*, void*, void*);
void gtl_SoundEffect_GetEnabled(void*, void*, void*);
void gtl_SoundEffect_Finalize(void*, void*);
void __dl__FPv(void*);

struct FxReverbHiVirtuals {
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

} // extern "C"

// Deinitialize and destroy the standalone effect; the compiler adds the optional operator delete.
gtl::nw4r::FxReverbHi::~FxReverbHi() {
    vtbl = &__vt__Q33gtl4nw4r10FxReverbHi;
    ((FxReverbHiVirtuals*)this)->deinit();
    ((FxReverbHiVirtuals*)this)->destroy();
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Finalize the owner and embedded effect, then optionally free the owner on the audio heap.
// effectVtable at +0x14 is the start of the embedded object, not a pointer to a separate effect.
void* gtl::CSoundEffectReverbHi::Destroy(int deleting) {
    if (this) {
        vtable = &__vt__Q23gtl20CSoundEffectReverbHi;
        // Detach/finalize the owner before dismantling the embedded implementation.
        gtl_SoundEffect_Finalize(this, &effectVtable);
        if (&effectVtable) {
            effectVtable = &__vt__Q33gtl4nw4r10FxReverbHi;
            ((FxReverbHiVirtuals*)(&effectVtable))->deinit();
            ((FxReverbHiVirtuals*)(&effectVtable))->destroy();
        }
        // The owner uses the audio allocator; the standalone destructor above uses delete.
        if (deleting > 0)
            gtl_Audio_Free(this);
    }
    return this;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

extern "C" {

// Tail-forward to audio free; the original ABI carries the allocation in incoming r3.
// The argument-less reconstruction intentionally preserves that forwarding thunk.
void dtor_803FA548(void* object) {
    gtl_Audio_Free(object);
}

} // extern "C"

// Forward the enable mode (0 or 1) to the embedded effect; ownership is unchanged.
void gtl::CSoundEffectReverbHi::SetEnabled(void* enabledMode) {
    gtl_SoundEffect_SetEnabled(this, &effectVtable, enabledMode);
}

// Write the embedded effect's enable state (0 or 1) to the caller's output address.
void gtl::CSoundEffectReverbHi::GetEnabled(void* enabledOutput) {
    gtl_SoundEffect_GetEnabled(this, &effectVtable, enabledOutput);
}
