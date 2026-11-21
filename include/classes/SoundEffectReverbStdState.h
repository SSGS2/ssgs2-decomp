// CSoundEffectReverbStd view owned by gtl_sound_effect_chorus_reverb.cpp.
// gtl_CSoundEffectReverbStd_Create (0x803FAFC8) initializes the wrapper and
// constructs FxReverbStd at +0x14; GetObjectSize returns 0x13C.
#ifndef CLASSES_SOUNDEFFECTREVERBSTDSTATE_H
#define CLASSES_SOUNDEFFECTREVERBSTDSTATE_H

struct SoundEffectReverbStdState {
    int referenceCount;
    void* vtable;
    void* enabled;
    int unk_C;
    void* auxiliaryStorage;
    char effect[0x128];
};

#define OFF(f, o) typedef char SoundEffectReverbStdState_off_##f[((int)&((SoundEffectReverbStdState*)0)->f == (o)) ? 1 : -1]
OFF(referenceCount, 0x0);
OFF(vtable, 0x4);
OFF(enabled, 0x8);
OFF(unk_C, 0xC);
OFF(auxiliaryStorage, 0x10);
OFF(effect, 0x14);
#undef OFF
typedef char SoundEffectReverbStdState_size[(sizeof(SoundEffectReverbStdState) == 0x13C) ? 1 : -1];

#endif
