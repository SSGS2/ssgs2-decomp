// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFILEIOFLASH_H
#define CLASSES_CFILEIOFLASH_H

#include "game/types.h"

class CFileIoFlash {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    int statusCode;                          // 0x8: 1 use in CFileIoFlash_HasNonzeroStatus
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFILEIOFLASH_OFF(f, o) typedef char CFileIoFlash_off_##f[((int)&((CFileIoFlash*)0)->f == (o)) ? 1 : -1]
CFILEIOFLASH_OFF(statusCode, 0x8);
#undef CFILEIOFLASH_OFF

#endif  // CLASSES_CFILEIOFLASH_H
