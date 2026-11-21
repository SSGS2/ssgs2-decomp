// GtlShowCounter: layout-only view owned by gtl_pointer_ring_and_sample_deque.cpp (gtl_IShowCounter_Ctor, gtl_IShowCounter_StartCounting).
// No virtual methods, constructors, or generated RTTI.
#ifndef CLASSES_GTLSHOWCOUNTER_H
#define CLASSES_GTLSHOWCOUNTER_H

class GtlShowCounter {
public:
    unsigned int unk_0; // 0x0: gtl_IShowCounter_Ctor initializes to 1
    void* vtbl; // 0x4: gtl_IShowCounter_Ctor installs __vt__Q23gtl12IShowCounter
    int state; // 0x8: gtl_IShowCounter_Pause/gtl_IShowCounter_ArmResetOnStart select 1/2; vfunc_1 selects 0
    int count; // 0xc: gtl_IShowCounter_GetCount returns; vfunc_1 clears when leaving state 2
    int previousCount; // 0x10: gtl_IShowCounter_GetPreviousCount returns; vfunc_1 clears when leaving state 2
};

#define OFF(f, o) typedef char GtlShowCounter_off_##f[((int)&((GtlShowCounter*)0)->f == (o)) ? 1 : -1]
OFF(unk_0, 0x0);
OFF(vtbl, 0x4);
OFF(state, 0x8);
OFF(count, 0xc);
OFF(previousCount, 0x10);
#undef OFF

#endif
