// Layout of the DPL2 high-reverb sound effect wrapper. The embedded DPL2 processor
// (gtl::nw4r::FxReverbHiDpl2, virtual dispatch) sits at +0x14. Methods are defined in
// gtl_sound_effect_reverb_hi_dpl2_part1.cpp, gtl_sound_effect_reverb_hi_dpl2_part2.cpp,
// gtl_sound_effect_reverb_hidpl2.cpp, sound_effect_reverb_hidpl2.cpp and sound_effect_reverb_hi_dpl2.cpp.
#ifndef CLASSES_SOUNDEFFECTREVERBHIDPL2STATE_H
#define CLASSES_SOUNDEFFECTREVERBHIDPL2STATE_H

namespace gtl {
class CSoundEffectReverbHiDpl2 {
public:
    // Teardown. Frees the wrapper when deleting > 0. Returns this, possibly already freed.
    CSoundEffectReverbHiDpl2* Destroy(int deleting);
    // Forward the enable mode (0/1) to the embedded effect.
    void SetEnabled(void* enabledMode);
    void GetEnabled(void* outEnabled);
    // Update one parameter (or the full block) and apply it. Always returns 1.
    int SetParameter(unsigned int parameter, unsigned int unused, const void* value);
    // Read one parameter (or all six) from the embedded effect. Always returns 1.
    int GetParameter(unsigned int index, void* unused, void* output);
    // Factory helpers. The first argument is not used.
    static int GetObjectSize(void* unused, unsigned int* output);
    static int Create(void* unused, CSoundEffectReverbHiDpl2** output);

    unsigned int referenceCount; // 0x00: wrapper ownership count, starts at 1
    void* vtable;                // 0x04
    unsigned int effectEnabled;  // 0x08
    int registryIndex;           // 0x0C: starts at -1, meaning not established
    unsigned int ownedStorage;   // 0x10: optional separate allocation, freed by finalize
    char effect[0x1C0];          // 0x14: embedded FxReverbHiDpl2 processor
};
}

#define OFF(f, o) \
    typedef char CSoundEffectReverbHiDpl2_off_##f[((int)&((gtl::CSoundEffectReverbHiDpl2*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x04);
OFF(effect, 0x14);
#undef OFF
typedef char CSoundEffectReverbHiDpl2_size[(sizeof(gtl::CSoundEffectReverbHiDpl2) == 0x1D4) ? 1 : -1];
#endif
