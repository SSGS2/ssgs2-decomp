#include <decomp/static_init.h>
#include "classes/SoundEffectReverbHiState.h"
// Translation unit 0x803FA728-0x803FA8D4 (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

int gtl_CSoundEffectReverbHi_GetObjectSize(void*, void*);
void SoundEffectReverbHi_InitializeStatics();
void __dl__FPv(void*);
void* gtl_Audio_Alloc(unsigned int);
void gtl_nw4r_FxReverbHi_Ctor(void*);
void gtl_SoundEffect_Initialize(void*, void*);

enum { kReverbHiObjectBytes = 0x1A4 };

float s_reverbHiAngularPeriod;
extern const float kFloat_6_2831855;
extern const float kReverbHiInitSlot0;
extern const float kReverbHiInitSlot1;
extern const float kReverbHiInitSlot2;
extern const float kReverbHiInitSlot3;
extern const float kReverbHiInitSlot4;
extern const float kReverbHiInitSlot5;
extern const float kReverbHiInitSlot6;
extern const float kReverbHiInitSlot7;
extern const float kReverbHiInitSlot8;
extern const float kReverbHiInitSlot9;
extern const float kReverbHiInitSlot10;
extern const float kReverbHiInitSlot11;
extern char __vt__Q23gtl20CSoundEffectReverbHi[];
// Writable copies of the twelve initialization slots, in the same order as their source constants.
// SDK property identities remain unproved; keep duplicate values in distinct slots.
float s_reverbHiInitialSlot0;
float s_reverbHiInitialSlot1;
float s_reverbHiInitialSlot2;
float s_reverbHiInitialSlot3;
float s_reverbHiInitialSlot4;
float s_reverbHiInitialSlot5;
float s_reverbHiInitialSlot6;
float s_reverbHiInitialSlot7;
float s_reverbHiInitialSlot8;
float s_reverbHiInitialSlot9;
float s_reverbHiInitialSlot10;
float s_reverbHiInitialSlot11;

// Write the high-quality reverb allocation size in bytes; return 0 for null output, otherwise 1.
int gtl_CSoundEffectReverbHi_GetObjectSize(void*, void* outSizeBytes) {
    if (!outSizeBytes) return 0;
    *(int*)outSizeBytes = kReverbHiObjectBytes;
    return 1;
}

// Copy the full-turn value and twelve initialization slots into writable static storage.
// Only the first value is identified (2*pi radians); the other slots' properties/units are unresolved.
void SoundEffectReverbHi_InitializeStatics() {
    s_reverbHiAngularPeriod = kFloat_6_2831855;
    s_reverbHiInitialSlot0 = kReverbHiInitSlot0;
    s_reverbHiInitialSlot1 = kReverbHiInitSlot1;
    s_reverbHiInitialSlot2 = kReverbHiInitSlot2;
    s_reverbHiInitialSlot3 = kReverbHiInitSlot3;
    s_reverbHiInitialSlot4 = kReverbHiInitSlot4;
    s_reverbHiInitialSlot5 = kReverbHiInitSlot5;
    s_reverbHiInitialSlot6 = kReverbHiInitSlot6;
    s_reverbHiInitialSlot7 = kReverbHiInitSlot7;
    s_reverbHiInitialSlot8 = kReverbHiInitSlot8;
    s_reverbHiInitialSlot9 = kReverbHiInitSlot9;
    s_reverbHiInitialSlot10 = kReverbHiInitSlot10;
    s_reverbHiInitialSlot11 = kReverbHiInitSlot11;
}

// Allocate the reverb owner, construct its embedded NW4R effect and initialize the wrapper.
// Return 0 only for null outEffect; allocation failure writes null and still returns 1.
// A non-null result transfers the allocated effect to the caller with reference count 1.
int gtl_CSoundEffectReverbHi_Create(void* unused, void** outEffect) {
    if (!outEffect) return 0;
    void* effect = gtl_Audio_Alloc(kReverbHiObjectBytes);
    if (effect) {
        struct Reverb { int referenceCount; void* vtable; int enabled; int reservedSlot; int workStorage; };
        Reverb* state = (Reverb*)effect;
        // Initialize wrapper state before constructing and attaching the embedded SDK effect.
        state->referenceCount = 1;
        state->enabled = 0;
        state->reservedSlot = -1;
        state->workStorage = 0;
        state->vtable = __vt__Q23gtl20CSoundEffectReverbHi;
        gtl_nw4r_FxReverbHi_Ctor(&AsSoundEffectReverbHiState(effect)->effectVtable); // 0x803FA748.
        gtl_SoundEffect_Initialize(effect, &AsSoundEffectReverbHiState(effect)->effectVtable);
    }
    *outEffect = effect;
    return 1;
}

struct FxReverbHiDpl2Vtable {
    char pad[0x14];
    void (*destroy)(void*);
    char pad2[4];
    void (*finish)(void*);
};

struct FxReverbHiDpl2 { FxReverbHiDpl2Vtable* vtable; };
extern FxReverbHiDpl2Vtable __vt__Q33gtl4nw4r14FxReverbHiDpl2;

// Finish and destroy the embedded DPL2 reverb effect, then optionally free it.
// Null is accepted; shouldDelete > 0 releases storage. Returned address may already be freed.
FxReverbHiDpl2* FxReverbHiDpl2_dtor(FxReverbHiDpl2* self, int shouldDelete) {
    if (self) {
        self->vtable = &__vt__Q33gtl4nw4r14FxReverbHiDpl2;
        self->vtable->finish(self);
        self->vtable->destroy(self);
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462674", _ctors_u803FA728_0, SoundEffectReverbHi_InitializeStatics);
#pragma pop
