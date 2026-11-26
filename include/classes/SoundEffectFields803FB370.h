// gtl::CSoundEffectChorus: a chorus sound effect. Its embedded FxChorus effect sits at +0x14
// inside the same allocation. Methods are defined in gtl_sound_effect_chorus_reverb.cpp,
// gtl_sound_effect_delay_chorus.cpp, sound_effect_chorus_params_part1.cpp and
// sound_effect_chorus_params_part2.cpp.
#ifndef CLASSES_SOUNDEFFECTFIELDS803FB370_H
#define CLASSES_SOUNDEFFECTFIELDS803FB370_H

namespace gtl {

class CSoundEffectChorus {
public:
    // Teardown. Frees the wrapper when shouldDelete > 0. Returns this, possibly already freed.
    CSoundEffectChorus* Destroy(int shouldDelete);
    static int GetObjectSize(void* unused, int* objectSize);
    static int Create(void* unused, CSoundEffectChorus** outChorus);

    // Enable state, forwarded to the embedded effect.
    void SetEnabled(int enabledValue);
    void GetEnabled(int enabledOutputAddress);

    // Parameters. Always return 1.
    int SetParameter(int parameter, int unused, void* value);
    int GetParameter(int parameter, int unused, void* output);

    int referenceCount;     // 0x00: starts at 1
    void* vtable;           // 0x04
    int effectEnabled;      // 0x08: enable mode, starts at 0
    int registryIndex;      // 0x0C: starts at -1
    void* auxiliaryStorage; // 0x10
    char effect[0xd0];      // 0x14: embedded FxChorus
};

} // namespace gtl

#define CSOUNDEFFECTCHORUS_OFF(f, o) \
    typedef char CSoundEffectChorus_off_##f[((int)&((gtl::CSoundEffectChorus*)0)->f == (o)) ? 1 : -1]
CSOUNDEFFECTCHORUS_OFF(referenceCount, 0x0);
CSOUNDEFFECTCHORUS_OFF(vtable, 0x4);
CSOUNDEFFECTCHORUS_OFF(effectEnabled, 0x8);
CSOUNDEFFECTCHORUS_OFF(registryIndex, 0xc);
CSOUNDEFFECTCHORUS_OFF(auxiliaryStorage, 0x10);
CSOUNDEFFECTCHORUS_OFF(effect, 0x14);
#undef CSOUNDEFFECTCHORUS_OFF
typedef char CSoundEffectChorus_size[(sizeof(gtl::CSoundEffectChorus) == 0xe4) ? 1 : -1];

#endif
