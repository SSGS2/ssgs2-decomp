// Translation unit 0x803FA8D4-0x803FA970 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct FxReverbHiDpl2 {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void clearOutputBuffer(); // Slot 3: clear output buffer.
    virtual void slot4();
    virtual void shutdownSdkWork();
    // Slot 5: shut down SDK work.
};

// ABI: +0x00 opaque base word, +0x04 wrapper dispatch, +0x08..+0x13
// owner/base bookkeeping (individual fields unresolved), +0x14 effect dispatch.
// Finalize detaches that embedded effect; only the wrapper owns its allocation.
struct SoundEffectReverbHiDpl2 {
    char baseState[4];
    void* vtable;
    char effectOwnerState[12];
    FxReverbHiDpl2 effect;
};

extern "C" {
extern char __vt__Q23gtl24CSoundEffectReverbHiDpl2[];
extern char __vt__Q33gtl4nw4r14FxReverbHiDpl2[];
void gtl_SoundEffect_Finalize(void*, void*);
void gtl_Audio_Free(void*);

// Detach the effect, release its DSP resources, then optionally free the wrapper.
// deleting > 0 frees self through the audio allocator. Return the original address,
// possibly freed; a non-deleting call still tears down the embedded effect.
SoundEffectReverbHiDpl2* gtl_CSoundEffectReverbHiDpl2_dtor(SoundEffectReverbHiDpl2* self, int deleting) {
    if (self) {
        // Restore wrapper dispatch and remove the effect from its owner's processing.
        self->vtable = __vt__Q23gtl24CSoundEffectReverbHiDpl2;
        gtl_SoundEffect_Finalize(self, &self->effect);
        // CodeWarrior emits the embedded-base destructor guard even though a
        // nonnull wrapper has a nonnull +0x14 effect. Install base dispatch
        // before calling its teardown slots (the derived effect is detached).
        if (&self->effect) {
            *(void**)&self->effect = __vt__Q33gtl4nw4r14FxReverbHiDpl2;
            self->effect.shutdownSdkWork(); // SDK work shutdown (slot 5).
            self->effect.clearOutputBuffer(); // Clear output buffer (slot 3).
        }
        if (deleting > 0) {
            gtl_Audio_Free(self);
        }
    }
    return self;
}

} // extern "C"
