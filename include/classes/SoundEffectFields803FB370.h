// Sound effect view owned by gtl_sound_effect_delay_chorus.cpp. gtl_CSoundEffectChorus_Create (0x803FB390) initializes the wrapper and constructs its effect at +0x14; Delay_vfunc_1/2 pass the same embedded effect address. Unknown wrapper words retain offset names.
#ifndef CLASSES_SOUNDEFFECTFIELDS803FB370_H
#define CLASSES_SOUNDEFFECTFIELDS803FB370_H

struct SoundEffectFields803FB370 {
    int referenceCount;
    void* vtable;
    int effectEnabled;
    int registryIndex;
    int auxiliaryStorage;
    char effect[0xd0];
};

#define OFF(f, o) typedef char SoundEffectFields803FB370_off_##f[((int)&((SoundEffectFields803FB370*)0)->f == (o)) ? 1 : -1]
OFF(referenceCount, 0x0);
OFF(vtable, 0x4);
OFF(effectEnabled, 0x8);
OFF(registryIndex, 0xc);
OFF(auxiliaryStorage, 0x10);
OFF(effect, 0x14);
#undef OFF

#endif
