// SoundEffectReverbHiState: layout view; gtl_sound_effect_reverb_hi_part1.cpp: gtl_CSoundEffectReverbHi_dtor installs the owner and embedded FxReverbHi vtables
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_SOUNDEFFECTREVERBHISTATE_H
#define CLASSES_SOUNDEFFECTREVERBHISTATE_H

struct SoundEffectReverbHiState {
    char pad_000[0x4];
    void* vtable; // 0x4
    char pad_008[0xC];
    void* effectVtable; // 0x14
};

#define OFF(f, o) typedef char SoundEffectReverbHiState_off_##f[((int)&((SoundEffectReverbHiState*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x4);
OFF(effectVtable, 0x14);
#undef OFF
#define AsSoundEffectReverbHiState(p) ((SoundEffectReverbHiState*)(p))

#endif
