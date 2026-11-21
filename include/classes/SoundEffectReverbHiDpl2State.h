// Non-virtual layout view; methods in gtl_sound_effect_reverb_hi_dpl2_part2.cpp.
// 0x803FA974 / 0x803FA980 forward the embedded effect at +0x14.
#ifndef CLASSES_SOUNDEFFECTREVERBHIDPL2STATE_H
#define CLASSES_SOUNDEFFECTREVERBHIDPL2STATE_H
struct SoundEffectReverbHiDpl2State {
    char pad_000[0x14];
    unsigned char effect[1]; // 0x14: address-only view of the embedded FxReverbHiDpl2 object
};
#define OFF(f, o) typedef char SoundEffectReverbHiDpl2State_off_##f[((int)&((SoundEffectReverbHiDpl2State*)0)->f == (o)) ? 1 : -1]
OFF(effect, 0x14);
#undef OFF
#endif
