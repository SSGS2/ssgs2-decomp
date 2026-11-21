// RevolutionShowCounterState: layout view; gtl_transform_filter_show_counter.cpp: +0x10 points to a reference count decremented/released by gtl_CTransformFilter_Reset/gtl_CTransformFilter_SetInputSample; ctor writes numeric +0x14
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_REVOLUTIONSHOWCOUNTERSTATE_H
#define CLASSES_REVOLUTIONSHOWCOUNTERSTATE_H

struct RevolutionShowCounterState {
    char pad_000[0x4];
    void* vtable; // 0x4
    char pad_008[0x8];
    int* reference; // 0x10
    unsigned int unk_14; // 0x14
};

#define OFF(f, o) typedef char RevolutionShowCounterState_off_##f[((int)&((RevolutionShowCounterState*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x4);
OFF(reference, 0x10);
OFF(unk_14, 0x14);
#undef OFF
#define AsRevolutionShowCounterState(p) ((RevolutionShowCounterState*)(p))

#endif
