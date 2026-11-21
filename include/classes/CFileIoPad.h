// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFILEIOPAD_H
#define CLASSES_CFILEIOPAD_H

#include "game/types.h"

class CFileIoPad {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    int statusCode;                          // 0x8: 2 uses in CFileIoPad_HasFileError, CFileIoPad_vfunc_9
    char pad_00C[0x4c];                 // 0xc
    int padIndex;                         // 0x58: 1 use in CFileIoPad_SetPathAndAllocateBuffer
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFILEIOPAD_OFF(f, o) typedef char CFileIoPad_off_##f[((int)&((CFileIoPad*)0)->f == (o)) ? 1 : -1]
CFILEIOPAD_OFF(statusCode, 0x8);
CFILEIOPAD_OFF(padIndex, 0x58);
#undef CFILEIOPAD_OFF

#endif  // CLASSES_CFILEIOPAD_H
