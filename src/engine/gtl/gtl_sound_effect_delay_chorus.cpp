// Delay and chorus sound effects: the delay effect's destroy and enable calls, the chorus
// creation and destructors, and the effect math constants.

#include <decomp/static_init.h>
#include "classes/SoundEffectFields803FB370.h"
#include "classes/SoundEffectDelayState.h"

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
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Return sound-effect storage to the audio allocator.
void dtor_803FB588(void* self) {
    gtl_Audio_Free(self);
}

} // extern "C"

namespace gtl {

// Report the allocation size required by the chorus wrapper and embedded effect.
// Returns 0 for a null output, otherwise writes bytes and returns 1.
int CSoundEffectChorus::GetObjectSize(void* unused0, int* objectSize) {
    if (objectSize == 0)
        return 0;
    *objectSize = sizeof(CSoundEffectChorus);
    return 1;
}

// Forward the enable request to the embedded delay effect.
// enabledValue carries integer mode 0 or 1 in an ABI pointer slot, not an address.
// The wrapper discards the backend's success/failure result.
void CSoundEffectDelay::SetEnabled(void* enabledValue) {
    gtl_SoundEffect_SetEnabled(this, effect, enabledValue);
}

// Read the enable state of the embedded delay effect.
// enabledOutput is a required borrowed int* receiving 0 or 1 synchronously.
void CSoundEffectDelay::GetEnabled(void* enabledOutput) {
    gtl_SoundEffect_GetEnabled(this, effect, enabledOutput);
}

// Detach the wrapper, destroy its embedded delay, then optionally free storage.
// Embedded effect starts at +0x14; the wrapper is freed only for shouldDelete > 0.
CSoundEffectDelay* CSoundEffectDelay::Destroy(int shouldDelete) {
    if (this != 0) {
        vtable = __vt__Q23gtl17CSoundEffectDelay;
        // Phase 1: detach from processing and release wrapper-owned auxiliary storage.
        gtl_SoundEffect_Finalize(this, effect);
        if (effect != 0) {
            // Phase 2: run native teardown at +0x1C then +0x14; do not free the subobject.
            *(void**)effect = __vt__Q33gtl4nw4r7FxDelay;
            ((FxDelayDispatch*)effect)->destroy();
            ((FxDelayDispatch*)effect)->unused2();
        }
        if (shouldDelete > 0)
            gtl_Audio_Free(this);
    }
    return this;
}

} // namespace gtl

// Allocate a chorus wrapper, construct its effect, and attach it to audio processing.
// Returns 0 only for a null output slot. Returns 1 even on allocation failure,
// storing null in that case. A non-null result is owned by the caller and must
// be destroyed through the sound-effect interface (which uses gtl_Audio_Free).
int gtl::CSoundEffectChorus::Create(void* unused, CSoundEffectChorus** outChorus) {
    if (outChorus == 0)
        return 0;
    void* chorus = gtl_Audio_Alloc(sizeof(CSoundEffectChorus));
    if (chorus != 0) {
        // Phase 1: initialize the reference-counted wrapper and effect state.
        // effectEnabled is the enable mode (see SoundEffect_SetEnabled in gtl_sound_buffer_and_effects.cpp).
        ((CSoundEffectChorus*)chorus)->referenceCount = 1;
        ((CSoundEffectChorus*)chorus)->effectEnabled = 0;
        ((CSoundEffectChorus*)chorus)->registryIndex = -1;
        ((CSoundEffectChorus*)chorus)->auxiliaryStorage = 0;
        ((CSoundEffectChorus*)chorus)->vtable = __vt__Q23gtl18CSoundEffectChorus;
        // Phase 2: construct the in-place effect, then register it with audio processing.
        void* wrapper = chorus;
        gtl_nw4r_FxChorus_Ctor(((CSoundEffectChorus*)chorus)->effect);
        gtl_SoundEffect_Initialize(wrapper, ((CSoundEffectChorus*)wrapper)->effect);
    }
    *outChorus = (CSoundEffectChorus*)chorus;
    return 1;
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462680", _ctors_u803FB370_0, AudioEffects_InitializeMathConstants);
#pragma pop
