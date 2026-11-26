// High-quality DPL2 reverb sound effect (gtl::CSoundEffectReverbHiDpl2): its destructor, which
// detaches the effect, releases its DSP resources, and optionally frees the wrapper.

#include "classes/SoundEffectReverbHiDpl2State.h"

// The embedded DPL2 processor at +0x14. Only its virtual slots are used here.
struct FxReverbHiDpl2 {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void clearOutputBuffer(); // Slot 3: clear output buffer.
    virtual void slot4();
    virtual void shutdownSdkWork();
    // Slot 5: shut down SDK work.
};

extern "C" {
extern char __vt__Q23gtl24CSoundEffectReverbHiDpl2[];
extern char __vt__Q33gtl4nw4r14FxReverbHiDpl2[];
void gtl_SoundEffect_Finalize(void*, void*);
void gtl_Audio_Free(void*);
}

namespace gtl {
// Detach the effect, release its DSP resources, then optionally free the wrapper.
// deleting > 0 frees the wrapper through the audio allocator. Returns this, possibly freed.
CSoundEffectReverbHiDpl2* CSoundEffectReverbHiDpl2::Destroy(int deleting) {
    if (this) {
        // Restore wrapper dispatch and remove the effect from its owner's processing.
        vtable = __vt__Q23gtl24CSoundEffectReverbHiDpl2;
        gtl_SoundEffect_Finalize(this, effect);
        // CodeWarrior emits the embedded-base destructor guard even though a
        // nonnull wrapper has a nonnull +0x14 effect. Install base dispatch
        // before calling its teardown slots (the derived effect is detached).
        if (&effect) {
            *(void**)effect = __vt__Q33gtl4nw4r14FxReverbHiDpl2;
            ((FxReverbHiDpl2*)effect)->shutdownSdkWork();   // SDK work shutdown (slot 5).
            ((FxReverbHiDpl2*)effect)->clearOutputBuffer(); // Clear output buffer (slot 3).
        }
        if (deleting > 0) {
            gtl_Audio_Free(this);
        }
    }
    return this;
}
}
