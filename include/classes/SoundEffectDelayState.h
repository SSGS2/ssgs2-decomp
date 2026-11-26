// Layout of the delay sound-effect wrapper. The embedded delay processor
// (gtl::nw4r::FxDelay, virtual dispatch) sits at +0x14. Methods are defined in
// gtl_sound_effect_delay_chorus.cpp, gtl_sound_effect_delay_params.cpp,
// sound_effect_delay.cpp and gtl_audio_runtime_and_delay_effect.cpp.
#ifndef CLASSES_SOUNDEFFECTDELAYSTATE_H
#define CLASSES_SOUNDEFFECTDELAYSTATE_H

namespace gtl {
class CSoundEffectDelay {
public:
    // Teardown. Frees the wrapper when shouldDelete > 0. Returns this, possibly already freed.
    CSoundEffectDelay* Destroy(int shouldDelete);
    // Forward the enable mode (0/1) to the embedded effect.
    void SetEnabled(void* enabledValue);
    void GetEnabled(void* enabledOutput);
    // Update one delay parameter (or the full block) and apply it. Always returns 1.
    int SetParameter(int selector, int unused, void* inputValue);
    // Read one delay parameter (or all three) from the embedded effect. Always returns 1.
    int GetParameter(int index, int unused, void* output);
    // Factory helpers. The first argument is not used.
    static int GetObjectSize(void* unused, int* size);
    static int Create(void* unused, CSoundEffectDelay** output);

    int referenceCount;     // 0x00: wrapper ownership count, starts at 1
    void* vtable;           // 0x04
    int effectEnabled;      // 0x08: enable mode, starts at 0
    int registryIndex;      // 0x0C: starts at -1
    void* auxiliaryStorage; // 0x10: optional separate allocation
    char effect[0x88];      // 0x14: embedded delay processor
};
}

#define OFF(f, o) typedef char CSoundEffectDelay_off_##f[((int)&((gtl::CSoundEffectDelay*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x04);
OFF(effectEnabled, 0x08);
OFF(effect, 0x14);
#undef OFF
typedef char CSoundEffectDelay_size[(sizeof(gtl::CSoundEffectDelay) == 0x9C) ? 1 : -1];
#endif
