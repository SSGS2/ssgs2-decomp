// BaseObject constructor/destructor at 0x80045BF0 / 0x80045C64 (gsr_resource_bitmap_and_heap.cpp).
// Layout view only; no inheritance or generated virtual methods.
#ifndef CLASSES_BASEOBJECTLAYOUT_H
#define CLASSES_BASEOBJECTLAYOUT_H
struct BaseObjectLayout {
    void* vtable;      // 0x0: __vt__10BaseObject
    int creationIndex; // 0x4: BaseObject_ctor saves s_baseObjectCreationIndex, then increments that counter
    int unk_8;         // 0x8: BaseObject_ctor initializes to zero
    int unk_C;         // 0xc: BaseObject_ctor initializes to zero
};
#define OFF(f, o) typedef char BaseObjectLayout_off_##f[((int)&((BaseObjectLayout*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0);
OFF(creationIndex, 4);
OFF(unk_8, 8);
OFF(unk_C, 0xc);
#undef OFF
#endif
