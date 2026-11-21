// OpenItemFlags: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_OPENITEMFLAGS_H
#define CLASSES_OPENITEMFLAGS_H

class OpenItemFlags {
public:
    char pad_000[0x2];
    short selectedItem; // 0x2: CTourMain_InitItemMap
};

#define OFF(f, o) typedef char OpenItemFlags_off_##f[((int)&((OpenItemFlags*)0)->f == (o)) ? 1 : -1]
OFF(selectedItem, 0x2);
#undef OFF

#endif
