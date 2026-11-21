// OpenItemTreePair: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_OPENITEMTREEPAIR_H
#define CLASSES_OPENITEMTREEPAIR_H

class OpenItemTreePair {
public:
    char pad_000[0x4];
    void* item; // 0x4: CTourMain_InitItemMap, OpenItemMenu_BuildEntries
};

#define OFF(f, o) typedef char OpenItemTreePair_off_##f[((int)&((OpenItemTreePair*)0)->f == (o)) ? 1 : -1]
OFF(item, 0x4);
#undef OFF

#endif
