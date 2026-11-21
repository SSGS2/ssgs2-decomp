// Layout-only AI shot helper views owned by com_shot_cpu_rival_ai.cpp (0x801DD078-0x801E356C).
// CComShotProc_Ctor installs each concrete vtable before assigning these fields.
// CComPadOnePoint_dtor/CComShotOnePoint_dtor destroy the two correction children.
#ifndef CLASSES_CCOMCORRECTIONCHAIN_H
#define CLASSES_CCOMCORRECTIONCHAIN_H

struct CComOnePointState {
    void* vtable;
    void* next; // Same chain prefix as the local MidNode/LeafNode views.
    void* child;
    void* unk_C;
    void* shotProcessorState;
    void* unk_14;
    void* owner; // CComShotProc_Ctor copies the rival owner to both one-point helpers.
    void* angleCorrection; // Constructed with __vt__23CComShotAngleCorrection.
    void* powerCorrection; // Constructed with __vt__23CComShotPowerCorrection.
};
struct CComAngleCorrectionState {
    void* vtable;
    void* next;
    void* child;
    void* state;
    void* owner; // Copied from the one-point helper by CComShotProc_Ctor.
    char pad_14[0x4C];
    int enabled; // CComShotAngleCorrection_vfunc_5 writes; vfunc_1 gates the correction on it.
};
struct CComPowerCorrectionState {
    void* vtable;
    void* next;
    void* child;
    void* state;
    void* owner;
    char pad_14[0x50];
    int enabled; // CComShotPowerCorrection_vfunc_5 writes; vfunc_1 gates the correction on it.
};

#define OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
OFF(CComOnePointState, vtable, 0x0);
OFF(CComOnePointState, next, 0x4);
OFF(CComOnePointState, child, 0x8);
OFF(CComOnePointState, unk_C, 0xC);
OFF(CComOnePointState, shotProcessorState, 0x10);
OFF(CComOnePointState, unk_14, 0x14);
OFF(CComOnePointState, owner, 0x18);
OFF(CComOnePointState, angleCorrection, 0x1C);
OFF(CComOnePointState, powerCorrection, 0x20);
OFF(CComAngleCorrectionState, vtable, 0x0);
OFF(CComAngleCorrectionState, next, 0x4);
OFF(CComAngleCorrectionState, child, 0x8);
OFF(CComAngleCorrectionState, state, 0xC);
OFF(CComAngleCorrectionState, owner, 0x10);
OFF(CComAngleCorrectionState, enabled, 0x60);
OFF(CComPowerCorrectionState, vtable, 0x0);
OFF(CComPowerCorrectionState, next, 0x4);
OFF(CComPowerCorrectionState, child, 0x8);
OFF(CComPowerCorrectionState, state, 0xC);
OFF(CComPowerCorrectionState, owner, 0x10);
OFF(CComPowerCorrectionState, enabled, 0x64);
#undef OFF
#endif
