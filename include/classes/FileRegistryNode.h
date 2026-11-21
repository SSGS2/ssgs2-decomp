// FileRegistryNode: explicit layout view; no inheritance, virtual methods, or RTTI.
// Field comments cite the original function addresses that establish their use.
#ifndef CLASSES_FILEREGISTRYNODE_H
#define CLASSES_FILEREGISTRYNODE_H

#include "game/types.h"

class FileRegistryNode {
public:
    void* left; // 0x0: gtl_FileIO_CheckFmemSaveArea/80417E90: left child
    void* right; // 0x4: gtl_FileIO_CheckFmemSaveArea/80417E90: right child
    unsigned int parentAndColor; // 0x8: FileRegistry_RebalanceAfterInsert: parent pointer with low color bit
    unsigned int stringFlags; // 0xC: Len8041766C: high bit selects heap string
    unsigned int stringLength; // 0x10: Len8041766C: heap string length
    char* stringData; // 0x14: Text8041766C: heap string pointer
    void* file; // 0x18: gtl_FileIO_CheckFmemSaveArea: file lookup result
};

#define FILEREGISTRYNODE_OFF(f, o) typedef char FileRegistryNode_off_##f[((int)&((FileRegistryNode*)0)->f == (o)) ? 1 : -1]
FILEREGISTRYNODE_OFF(left, 0x0);
FILEREGISTRYNODE_OFF(right, 0x4);
FILEREGISTRYNODE_OFF(parentAndColor, 0x8);
FILEREGISTRYNODE_OFF(stringFlags, 0xC);
FILEREGISTRYNODE_OFF(stringLength, 0x10);
FILEREGISTRYNODE_OFF(stringData, 0x14);
FILEREGISTRYNODE_OFF(file, 0x18);
#undef FILEREGISTRYNODE_OFF

#endif
