#ifndef CLASSES_CSHAREDFILEREGISTRY_H
#define CLASSES_CSHAREDFILEREGISTRY_H

#include "game/types.h"

class CSharedFileRegistry {
public:
    void* vtbl;           // 0x0: original vtable pointer
    void* filePointers;   // 0x4
    int filePointerCount; // 0x8
    char pad_00C[0x4];    // 0xc
    void* fileRecords;    // 0x10
    int fileRecordCount;  // 0x14

    ~CSharedFileRegistry();
};

#define CSHAREDFILEREGISTRY_OFF(f, o) \
    typedef char CSharedFileRegistry_off_##f[((int)&((CSharedFileRegistry*)0)->f == (o)) ? 1 : -1]
CSHAREDFILEREGISTRY_OFF(filePointers, 0x4);
CSHAREDFILEREGISTRY_OFF(filePointerCount, 0x8);
CSHAREDFILEREGISTRY_OFF(fileRecords, 0x10);
CSHAREDFILEREGISTRY_OFF(fileRecordCount, 0x14);
#undef CSHAREDFILEREGISTRY_OFF

#endif // CLASSES_CSHAREDFILEREGISTRY_H
