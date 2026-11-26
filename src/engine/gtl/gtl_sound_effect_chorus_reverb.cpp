// Chorus and standard reverb sound effects: creation, destruction, enable and enable-query,
// and the parameter bounds. gtl::CSoundEffectChorus and gtl::CSoundEffectReverbStd.

#include <decomp/static_init.h>
#include "classes/SoundEffectFields803FB370.h"
#include "classes/SoundEffectReverbStdState.h"
enum {
    kReverbStdObjectBytes = sizeof(gtl::CSoundEffectReverbStd)
};

extern "C" {

void gtl_Audio_Free(void* self);
void gtl_SoundEffect_Finalize(void* self, void* member);
void* gtl_Audio_Alloc(unsigned int size);
void gtl_nw4r_FxReverbStd_Ctor(void* self);
void gtl_SoundEffect_Initialize(void* self, void* member);
extern char __vt__Q23gtl18CSoundEffectChorus[];
extern char __vt__Q23gtl21CSoundEffectReverbStd[];
void gtl_SoundEffect_SetEnabled(void* self, void* effect, int enabledValue);
void gtl_SoundEffect_GetEnabled(void* self, void* effect, int enabledOutputAddress);
void __dl__FPv(void* p);
extern char __vt__Q33gtl4nw4r8FxChorus[];
extern float kFloat_6_2831855;
// Bounds are indexed pairs: parameter 0 [0,0.1], 1 [0,10], 2/3/4 [0,1].
// Only their startup copies are used in source; parameter meanings/units remain unresolved.
extern float kReverbStdParameter0Minimum;
extern float kReverbStdParameter0Maximum;
extern float kReverbStdParameter1Minimum;
extern float kReverbStdParameter1Maximum;
extern float kReverbStdParameter2Minimum;
extern float kReverbStdParameter2Maximum;
extern float kReverbStdParameter3Minimum;
extern float kReverbStdParameter3Maximum;
extern float kReverbStdParameter4Minimum;
extern float kReverbStdParameter4Maximum;
extern float s_reverbStdTwoPi;
extern float s_reverbStdParameter0Minimum;
extern float s_reverbStdParameter0Maximum;
extern float s_reverbStdParameter1Minimum;
extern float s_reverbStdParameter1Maximum;
extern float s_reverbStdParameter2Minimum;
extern float s_reverbStdParameter2Maximum;
extern float s_reverbStdParameter3Minimum;
extern float s_reverbStdParameter3Maximum;
extern float s_reverbStdParameter4Minimum;
extern float s_reverbStdParameter4Maximum;
}

// Report the allocation size of the standard reverb wrapper; fail for a null output.
// Returns 1 after writing bytes, 0 without writing; unusedFactory is not inspected.
int gtl::CSoundEffectReverbStd::GetObjectSize(void* unusedFactory, int* outBytes) {
    if (outBytes == 0)
        return 0;
    *outBytes = kReverbStdObjectBytes;
    return 1;
}

// The SDK vtable includes two metadata words before its virtual slots.
struct FxChorusVtable {
    void* metadataAndSlots0To2[5];
    void (*teardownSlot3)(void*);
    void* slot4;
    void (*teardownSlot5)(void*);
};

class FxChorus {
public:
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void teardownSlot3();
    virtual void slot4();
    virtual void teardownSlot5();
};

extern "C" {

// Initialize two pi and the ten standard reverb parameter bounds at static startup.
void SoundEffectReverbStd_InitStaticConstants() {
    s_reverbStdTwoPi = kFloat_6_2831855;
    s_reverbStdParameter0Minimum = kReverbStdParameter0Minimum;
    s_reverbStdParameter0Maximum = kReverbStdParameter0Maximum;
    s_reverbStdParameter1Minimum = kReverbStdParameter1Minimum;
    s_reverbStdParameter1Maximum = kReverbStdParameter1Maximum;
    s_reverbStdParameter2Minimum = kReverbStdParameter2Minimum;
    s_reverbStdParameter2Maximum = kReverbStdParameter2Maximum;
    s_reverbStdParameter3Minimum = kReverbStdParameter3Minimum;
    s_reverbStdParameter3Maximum = kReverbStdParameter3Maximum;
    s_reverbStdParameter4Minimum = kReverbStdParameter4Minimum;
    s_reverbStdParameter4Maximum = kReverbStdParameter4Maximum;
}

// Restore the native chorus vtable, run its two teardown slots, and optionally delete it.
// deleting > 0 releases self with operator delete; the returned address may be freed.
// A null self is accepted. Slot 5 runs before slot 3, reloading the vtable in between.
void* gtl_nw4r_FxChorus_dtor(void* self, int deleting) {
    if (self != 0) {
        char* vtable = __vt__Q33gtl4nw4r8FxChorus;
        *(void**)self = vtable;
        ((FxChorusVtable*)vtable)->teardownSlot5(self);
        (*(FxChorusVtable**)self)->teardownSlot3(self);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release a chorus wrapper through the audio allocator.
// Finalization is the caller's responsibility; this entry only releases storage.
void dtor_803FB1E0(void* self) {
    gtl_Audio_Free(self);
}

} // extern "C"

// Forward the enabled-state setter to the embedded chorus effect.
// enabledValue is the scalar mode (0/1), carried through the existing void ABI.
void gtl::CSoundEffectChorus::SetEnabled(int enabledValue) {
    gtl_SoundEffect_SetEnabled(this, effect, enabledValue);
}

// Forward the enabled-state query to the embedded chorus effect.
// enabledOutputAddress is a 32-bit address of writable int storage, not the state itself.
void gtl::CSoundEffectChorus::GetEnabled(int enabledOutputAddress) {
    gtl_SoundEffect_GetEnabled(this, effect, enabledOutputAddress);
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Unregister and tear down the embedded chorus, then optionally free the wrapper.
// effect is embedded at +0x14; it is never a separate allocation to free.
// shouldDelete > 0 frees this; returning this does not retain it. Null this is accepted.
gtl::CSoundEffectChorus* gtl::CSoundEffectChorus::Destroy(int shouldDelete) {
    if (this != 0) {
        // Detach from audio management before invoking the native effect teardown.
        vtable = __vt__Q23gtl18CSoundEffectChorus;
        gtl_SoundEffect_Finalize(this, effect);
        if (effect != 0) {
            *(void**)effect = __vt__Q33gtl4nw4r8FxChorus;
            ((FxChorus*)effect)->teardownSlot5();
            ((FxChorus*)effect)->teardownSlot3();
        }
        // Only the wrapper allocation remains after the embedded effect is torn down.
        if (shouldDelete > 0)
            gtl_Audio_Free(this);
    }
    return this;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

// Allocate and initialize standard reverb; success reports a valid output slot even if allocation fails.
// outObject receives one owned wrapper reference, or null on allocation failure.
// Return 0 means no output slot was supplied; return 1 does not guarantee an object.
int gtl::CSoundEffectReverbStd::Create(void* unusedFactory, void** outObject) {
    if (outObject == 0)
        return 0;
    void* object = gtl_Audio_Alloc(kReverbStdObjectBytes);
    if (object != 0) {
        // Initialize the wrapper header before constructing its embedded effect.
        ((gtl::CSoundEffectReverbStd*)object)->referenceCount = 1;
        ((gtl::CSoundEffectReverbStd*)object)->enabled = 0;
        ((gtl::CSoundEffectReverbStd*)object)->registryIndex = -1;
        ((gtl::CSoundEffectReverbStd*)object)->auxiliaryStorage =
            0; // Clear the wrapper's auxiliary pointer before effect registration.
        ((gtl::CSoundEffectReverbStd*)object)->vtable = __vt__Q23gtl21CSoundEffectReverbStd;
        // Construct in-place at +0x14, then register the effect with its wrapper.
        gtl_nw4r_FxReverbStd_Ctor(((gtl::CSoundEffectReverbStd*)object)->effect);
        gtl_SoundEffect_Initialize(object, ((gtl::CSoundEffectReverbStd*)object)->effect);
    }
    *outObject = object;
    return 1;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046267C", _ctors_u803FAFA8_0, SoundEffectReverbStd_InitStaticConstants);
#pragma pop
