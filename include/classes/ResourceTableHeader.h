// ResourceTableHeader: offset-preserving view; no constructors, bases, or virtual methods.
// Evidence: ResourceTable_FindEntryByName (0x802D...) loops over entryCount; ResourceTable_RelocateSharedBuffer skips relocated tables and sets relocated after pointer fixups.
#ifndef CLASSES_RESOURCETABLEHEADER_H
#define CLASSES_RESOURCETABLEHEADER_H

struct ResourceTableHeader {
    char pad_000[0x4];
    int entryCount; // 0x4
    char pad_008[0x4];
    int relocated; // 0xC
};

#define OFF(f, o) typedef char ResourceTableHeader_off_##f[((int)&((ResourceTableHeader*)0)->f == (o)) ? 1 : -1]
OFF(entryCount, 0x4);
OFF(relocated, 0xC);
#undef OFF
typedef char ResourceTableHeader_size[(sizeof(ResourceTableHeader) == 0x10) ? 1 : -1];
#define AsResourceTableHeader(p) ((ResourceTableHeader*)(p))

#endif // CLASSES_RESOURCETABLEHEADER_H
