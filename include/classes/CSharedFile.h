// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHAREDFILE_H
#define CLASSES_CSHAREDFILE_H

#include "game/types.h"

class CSharedFile {
public:
    void* vtable;                        // 0x0: 1 use in CSharedFile_Ctor
    int unk_4;                          // 0x4: 1 use in CSharedFile_Ctor
    int referenceCount;                          // 0x8: 1 use in CSharedFile_Ctor
    unsigned int pathFlags;                 // 0xc: 3 uses in CSharedFile_GetPath, CSharedFile_Ctor, CSharedFile_dtor
    int pathLength;                         // 0x10: 1 use in CSharedFile_Ctor
    void* pathData;                       // 0x14: 3 uses in CSharedFile_GetPath, CSharedFile_Ctor, CSharedFile_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xd
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHAREDFILE_OFF(f, o) typedef char CSharedFile_off_##f[((int)&((CSharedFile*)0)->f == (o)) ? 1 : -1]
CSHAREDFILE_OFF(unk_4, 0x4);
CSHAREDFILE_OFF(referenceCount, 0x8);
CSHAREDFILE_OFF(pathFlags, 0xc);
CSHAREDFILE_OFF(pathLength, 0x10);
CSHAREDFILE_OFF(pathData, 0x14);
#undef CSHAREDFILE_OFF

#endif  // CLASSES_CSHAREDFILE_H
