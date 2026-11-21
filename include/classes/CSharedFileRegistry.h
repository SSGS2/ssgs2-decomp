// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHAREDFILEREGISTRY_H
#define CLASSES_CSHAREDFILEREGISTRY_H

#include "game/types.h"

class CSharedFileRegistry {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* filePointers;                        // 0x4: 3 uses in CSharedFileRegistry_dtor
    int filePointerCount;                          // 0x8: 7 uses in CSharedFileRegistry_dtor
    char pad_00C[0x4];                  // 0xc
    void* fileRecords;                       // 0x10: 3 uses in CSharedFileRegistry_dtor
    int fileRecordCount;                         // 0x14: 4 uses in CSharedFileRegistry_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x10
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHAREDFILEREGISTRY_OFF(f, o) typedef char CSharedFileRegistry_off_##f[((int)&((CSharedFileRegistry*)0)->f == (o)) ? 1 : -1]
CSHAREDFILEREGISTRY_OFF(filePointers, 0x4);
CSHAREDFILEREGISTRY_OFF(filePointerCount, 0x8);
CSHAREDFILEREGISTRY_OFF(fileRecords, 0x10);
CSHAREDFILEREGISTRY_OFF(fileRecordCount, 0x14);
#undef CSHAREDFILEREGISTRY_OFF

#endif  // CLASSES_CSHAREDFILEREGISTRY_H
