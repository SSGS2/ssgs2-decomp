// FxReverbHi work area: ctor and vfunc_4 owned by gtl_fx_reverb_hi.cpp.
// 0x8040A568/0x8040A764: passed to the SDK setup/update routines.
#ifndef CLASSES_FXREVERBHIWORK_H
#define CLASSES_FXREVERBHIWORK_H
struct FxReverbHiWork {
    char pad_000[0x30];
    char sdkWork[0x148]; // +0x30: existing local layout's opaque work region
};
#define OFF(f, o) typedef char FxReverbHiWork_off_##f[((int)&((FxReverbHiWork*)0)->f == (o)) ? 1 : -1]
OFF(sdkWork, 0x30);
#undef OFF
#endif
