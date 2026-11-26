#ifndef CLASSES_BINARYTABLE800699F0_H
#define CLASSES_BINARYTABLE800699F0_H

#include "game/types.h"

class BinaryTableView {
public:
    char pad_000[4];   // 0x0: unmodelled bytes
    char pad_004[0x8]; // 0x4
    int entryStride;   // 0xc: Table_GetEntry original object access
    int dataOffset;    // 0x10: Table_ResolveDataOffset original object access
};

#define BINARYTABLE800699F0_OFF(f, o) \
    typedef char BinaryTable800699F0_off_##f[((int)&((BinaryTableView*)0)->f == (o)) ? 1 : -1]
BINARYTABLE800699F0_OFF(entryStride, 0xc);
BINARYTABLE800699F0_OFF(dataOffset, 0x10);
#undef BINARYTABLE800699F0_OFF

#endif // CLASSES_BINARYTABLE800699F0_H
