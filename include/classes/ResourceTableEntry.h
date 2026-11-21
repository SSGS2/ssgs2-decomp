// ResourceTableEntry: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: ResourceTable_FindEntryByName compares name and returns count/items; ResourceTable_RelocateSharedBuffer relocates name/items and count pointed-to entries.
#ifndef CLASSES_RESOURCETABLEENTRY_H
#define CLASSES_RESOURCETABLEENTRY_H

struct ResourceTableEntry {
    void* name; // 0x0
    int count; // 0x4
    void* items; // 0x8
    int unk_C; // 0xC
};

#define OFF(f, o) typedef char ResourceTableEntry_off_##f[((int)&((ResourceTableEntry*)0)->f == (o)) ? 1 : -1]
OFF(name, 0x0);
OFF(count, 0x4);
OFF(items, 0x8);
OFF(unk_C, 0xC);
#undef OFF
typedef char ResourceTableEntry_size[(sizeof(ResourceTableEntry) == 0x10) ? 1 : -1];
#define AsResourceTableEntry(p) ((ResourceTableEntry*)(p))

#endif // CLASSES_RESOURCETABLEENTRY_H
