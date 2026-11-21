// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPACKEDFILE_H
#define CLASSES_CPACKEDFILE_H

#include "game/types.h"

class CPackedFile {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int loadState;                          // 0x4: 1 use in CPackedFile_BeginArchiveLoad
    void* sharedFileHandle;                        // 0x8: 5 uses in CPackedFile_BeginArchiveLoad, CPackedFile_dtor
    void* resources;                        // 0xc: 4 uses in CPackedFile_dtor
    unsigned int resourceCount;                // 0x10: 4 uses in CPackedFile_dtor
    union {
        char pad_014[0x4]; // Existing padding name retained.
        int resourceCapacity; // 0x14: CPackedFile_Create clears this word.
    };
    void* sharedFileHandles;                       // 0x18: 4 uses in CPackedFile_dtor
    unsigned int sharedFileHandleCount;                // 0x1c: 4 uses in CPackedFile_dtor
    int sharedFileHandleCapacity;                        // 0x20: CPackedFile_Create clears this word; meaning unknown.
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPACKEDFILE_OFF(f, o) typedef char CPackedFile_off_##f[((int)&((CPackedFile*)0)->f == (o)) ? 1 : -1]
CPACKEDFILE_OFF(loadState, 0x4);
CPACKEDFILE_OFF(sharedFileHandle, 0x8);
CPACKEDFILE_OFF(resources, 0xc);
CPACKEDFILE_OFF(resourceCount, 0x10);
CPACKEDFILE_OFF(sharedFileHandles, 0x18);
CPACKEDFILE_OFF(sharedFileHandleCount, 0x1c);
CPACKEDFILE_OFF(resourceCapacity, 0x14);
CPACKEDFILE_OFF(sharedFileHandleCapacity, 0x20);
#undef CPACKEDFILE_OFF

#endif  // CLASSES_CPACKEDFILE_H
