// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CNODECONTAINER_H
#define CLASSES_CNODECONTAINER_H

#include "game/types.h"

class CNodeContainer {
public:
    void* vtbl;                        // 0x0: 1 use in CNodeContainer_dtor
    unsigned int nameStringFlags;                 // 0x4: 1 use in CNodeContainer_dtor
    char pad_008[0x4];                  // 0x8
    void* nameStringData;                        // 0xc: 1 use in CNodeContainer_dtor
    unsigned int metadataWord;                // 0x10: 1 use in CNodeContainer_CtorFromName
    unsigned int entries;                // 0x14: 2 uses in CNodeContainer_dtor, CNodeContainer_CtorFromName
    unsigned int entryCount;                // 0x18: 2 uses in CNodeContainer_dtor, CNodeContainer_CtorFromName
    unsigned int entryCapacity;                // 0x1c: 1 use in CNodeContainer_CtorFromName

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CNODECONTAINER_OFF(f, o) typedef char CNodeContainer_off_##f[((int)&((CNodeContainer*)0)->f == (o)) ? 1 : -1]
CNODECONTAINER_OFF(nameStringFlags, 0x4);
CNODECONTAINER_OFF(nameStringData, 0xc);
CNODECONTAINER_OFF(metadataWord, 0x10);
CNODECONTAINER_OFF(entries, 0x14);
CNODECONTAINER_OFF(entryCount, 0x18);
CNODECONTAINER_OFF(entryCapacity, 0x1c);
#undef CNODECONTAINER_OFF

#endif  // CLASSES_CNODECONTAINER_H
