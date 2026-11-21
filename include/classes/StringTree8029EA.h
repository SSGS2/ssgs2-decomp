// StringTree8029EA: layout view from GMapBase_DestroyStorage / StringTree_DestroyStorage; provisional type name.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_STRINGTREE8029EA_H
#define CLASSES_STRINGTREE8029EA_H

struct StringTree8029EA {
    char pad_000[0x4];
    void* root; // 0x4: GMapBase_DestroyStorage / StringTree_DestroyStorage begin recursive destruction
};

#define OFF(f, o) typedef char StringTree8029EA_off_##f[((int)&((StringTree8029EA*)0)->f == (o)) ? 1 : -1]
OFF(root, 0x4);
#undef OFF
#define AsStringTree8029EA(p) ((StringTree8029EA*)(p))

#endif

