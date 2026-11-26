// FxReverbHiFlags: layout view; gtl_nw4r_fx_reverb_hi_dpl2.cpp: FxReverbHi vfunc_1 queries +0xC; Dpl2 vfunc_4 tests/sets initialization byte around setup
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_FXREVERBHIFLAGS_H
#define CLASSES_FXREVERBHIFLAGS_H

struct FxReverbHiFlags {
    char pad_000[0xC];
    unsigned char initializationAttempted; // 0xC
    char pad_00D[0xB];
    unsigned char
        parameters[1]; // 0x18: gtl_nw4r_FxReverbHi_GetParams (0x8040A850); address-only view, record size unknown
};

#define OFF(f, o) typedef char FxReverbHiFlags_off_##f[((int)&((FxReverbHiFlags*)0)->f == (o)) ? 1 : -1]
OFF(initializationAttempted, 0xC);
OFF(parameters, 0x18);
#undef OFF
#define AsFxReverbHiFlags(p) ((FxReverbHiFlags*)(p))

#endif
