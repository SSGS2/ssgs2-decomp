// FxReverbStd work area: SetParams (0x8040B08C) passes it to SDK update.
#ifndef CLASSES_FXREVERBSTDWORK_H
#define CLASSES_FXREVERBSTDWORK_H
struct FxReverbStdWork {
    char pad_000[0x2C];
    char sdkWork[0xE8]; // +0x2C: existing local layout's opaque work region
};
#define OFF(f, o) typedef char FxReverbStdWork_off_##f[((int)&((FxReverbStdWork*)0)->f == (o)) ? 1 : -1]
OFF(sdkWork, 0x2C);
#undef OFF
#endif
