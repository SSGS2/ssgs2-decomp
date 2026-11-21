// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPRELOADFILE_H
#define CLASSES_CPRELOADFILE_H

#include "game/types.h"

class CPreloadFile {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    unsigned int filenameFlags;                 // 0x4: 2 uses in CPreloadFile_dtor, CPreloadFile_Ctor
    char pad_008[0x4];                  // 0x8
    void* filenameBuffer;                        // 0xc: 2 uses in CPreloadFile_dtor, CPreloadFile_Ctor
    void* fileStream;                       // 0x10: 4 uses in CPreloadFile_dtor, CPreloadFile_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPRELOADFILE_OFF(f, o) typedef char CPreloadFile_off_##f[((int)&((CPreloadFile*)0)->f == (o)) ? 1 : -1]
CPRELOADFILE_OFF(filenameFlags, 0x4);
CPRELOADFILE_OFF(filenameBuffer, 0xc);
CPRELOADFILE_OFF(fileStream, 0x10);
#undef CPRELOADFILE_OFF

#endif  // CLASSES_CPRELOADFILE_H
