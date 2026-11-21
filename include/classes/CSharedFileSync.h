// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSHAREDFILESYNC_H
#define CLASSES_CSHAREDFILESYNC_H

#include "game/types.h"

class CSharedFileSync {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* buffer;                        // 0x4: 3 uses in CSharedFile_FreeBuffer, CSharedFile_AllocateBuffer
    char pad_008[0x10];                 // 0x8
    int fileSize;                         // 0x18: 1 use in CSharedFileSync_vfunc_1
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSHAREDFILESYNC_OFF(f, o) typedef char CSharedFileSync_off_##f[((int)&((CSharedFileSync*)0)->f == (o)) ? 1 : -1]
CSHAREDFILESYNC_OFF(buffer, 0x4);
CSHAREDFILESYNC_OFF(fileSize, 0x18);
#undef CSHAREDFILESYNC_OFF

#endif  // CLASSES_CSHAREDFILESYNC_H
