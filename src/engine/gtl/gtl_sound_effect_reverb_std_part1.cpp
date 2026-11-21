// Translation unit 0x803FAD14-0x803FADB0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct FxReverbStd {
    virtual void vfunc0();
    virtual void vfunc1();
    virtual void vfunc2();
    virtual void vfunc3();
    virtual void vfunc4();
    virtual void vfunc5();
};

struct CSoundEffectReverbStd {
    // Partial multiple-interface view: wrapper vtable +0x04, reverb vtable +0x14.
    // The embedded effect is part of the wrapper allocation, never freed alone.
    char prefix[4];
    void** vtable;
    char effectControllerStorage[12];
    FxReverbStd effect;
};

extern "C" {
extern void* __vt__Q23gtl21CSoundEffectReverbStd[];
extern void* __vt__Q33gtl4nw4r11FxReverbStd[];
void gtl_SoundEffect_Finalize(CSoundEffectReverbStd*, FxReverbStd*);
void gtl_Audio_Free(void*);

// Finalize the wrapper and embedded reverb, then optionally free the wrapper.
// Accept null; shouldDelete > 0 releases self through the audio allocator.
// The return is the original address, possibly freed. The effect slot names
// remain unresolved: preserve the observed slot-5 then slot-3 cleanup order.
CSoundEffectReverbStd* CSoundEffectReverbStd_dtor(CSoundEffectReverbStd* self, int shouldDelete) {
    if (self) {
        // Phase 1: restore the wrapper interface and detach/finalize its effect.
        self->vtable = __vt__Q23gtl21CSoundEffectReverbStd;
        gtl_SoundEffect_Finalize(self, &self->effect);
        if (&self->effect) {
            // Phase 2: restore the embedded interface and invoke its cleanup slots.
            *(void***)&self->effect = __vt__Q33gtl4nw4r11FxReverbStd;
            self->effect.vfunc5();
            self->effect.vfunc3();
        }
        if (shouldDelete > 0) {
            // Phase 3: only the deleting form releases the outer allocation.
            gtl_Audio_Free(self);
        }
    }
    return self;
}

} // extern "C"
