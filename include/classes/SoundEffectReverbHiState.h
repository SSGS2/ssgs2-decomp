// gtl::CSoundEffectReverbHi: the high-quality reverb sound effect. It owns an embedded NW4R reverb effect that starts
// at +0x14 and is finalized and destroyed together with the owner.
#ifndef CLASSES_SOUNDEFFECTREVERBHISTATE_H
#define CLASSES_SOUNDEFFECTREVERBHISTATE_H

namespace gtl {

class CSoundEffectReverbHi {
public:
    // Destruction and enable state.
    void* Destroy(int deleting);
    void SetEnabled(void* enabledMode);
    void GetEnabled(void* enabledOutput);

    // Parameters.
    int vfunc_3(unsigned int selector, void* unused, void* inputValue);
    int GetParameter(unsigned int parameter, unsigned int unused, void* output);

    // Allocation.
    int GetObjectSize(void* outSizeBytes);
    static int Create(void* unused, void** outEffect);

    int referenceCount; // 0x0
    void* vtable;       // 0x4
    int enabled;        // 0x8
    int reservedSlot;   // 0xC
    int workStorage;    // 0x10
    void* effectVtable; // 0x14, embedded NW4R effect starts here
    char pad_018[0x14]; // 0x18, embedded effect parameters follow
};

#define CSOUNDEFFECTREVERBHI_OFF(f, o) \
    typedef char CSoundEffectReverbHi_off_##f[((int)&((gtl::CSoundEffectReverbHi*)0)->f == (o)) ? 1 : -1]
CSOUNDEFFECTREVERBHI_OFF(referenceCount, 0x0);
CSOUNDEFFECTREVERBHI_OFF(vtable, 0x4);
CSOUNDEFFECTREVERBHI_OFF(enabled, 0x8);
CSOUNDEFFECTREVERBHI_OFF(reservedSlot, 0xC);
CSOUNDEFFECTREVERBHI_OFF(workStorage, 0x10);
CSOUNDEFFECTREVERBHI_OFF(effectVtable, 0x14);
#undef CSOUNDEFFECTREVERBHI_OFF

} // namespace gtl

#endif
