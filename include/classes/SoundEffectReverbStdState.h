// gtl::CSoundEffectReverbStd: a standard reverb sound effect. Its embedded
// FxReverbStd effect sits at +0x14 inside the same allocation.
#ifndef CLASSES_SOUNDEFFECTREVERBSTDSTATE_H
#define CLASSES_SOUNDEFFECTREVERBSTDSTATE_H

namespace gtl {

struct CSoundEffectReverbStd {
    // Lifetime
    CSoundEffectReverbStd* Destroy(int shouldDelete);
    static int Create(void* unusedFactory, void** outObject);
    static int GetObjectSize(void* unusedFactory, int* outBytes);

    // Enable state
    void SetEnabled(int enabledMode);
    void GetEnabled(int outputAddress);

    // Parameters
    int SetParameter(int parameter, int unused, const void* value);
    int GetParameter(int parameter, int unused, void* result);

    int referenceCount;     // 0x00
    void* vtable;           // 0x04
    void* enabled;          // 0x08
    int registryIndex;      // 0x0C
    void* auxiliaryStorage; // 0x10
    char effect[0x128];     // 0x14: embedded FxReverbStd
};

#define OFF(f, o) typedef char CSoundEffectReverbStd_off_##f[((int)&((CSoundEffectReverbStd*)0)->f == (o)) ? 1 : -1]
OFF(referenceCount, 0x0);
OFF(vtable, 0x4);
OFF(enabled, 0x8);
OFF(registryIndex, 0xC);
OFF(auxiliaryStorage, 0x10);
OFF(effect, 0x14);
#undef OFF
typedef char CSoundEffectReverbStd_size[(sizeof(CSoundEffectReverbStd) == 0x13C) ? 1 : -1];

} // namespace gtl

#endif
