// ntSkidMark_Cobra: owned by its virtual methods in skidmark_ball.cpp.
#ifndef CLASSES_NTSKIDMARKCOBRA_H
#define CLASSES_NTSKIDMARKCOBRA_H
struct ntSkidMarkCobra {
    char pad_000[0x1C];
    unsigned char active; // 0x1C: ntSkidMark_Cobra_AllocateFrameRange sets after allocation; vfunc_2 gates update
};
#define OFF(f, o) typedef char ntSkidMarkCobra_off_##f[((int)&((ntSkidMarkCobra*)0)->f == (o)) ? 1 : -1]
OFF(active, 0x1C);
#undef OFF
#define AsntSkidMarkCobra(p) ((ntSkidMarkCobra*)(p))
#endif
