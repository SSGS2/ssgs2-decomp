// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFILEIOACCESSCHECKFLASH_H
#define CLASSES_CFILEIOACCESSCHECKFLASH_H

#include "game/types.h"

class CFileIoAccessCheckFlash {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x8];                  // 0x4
    int state;                          // 0xc: 7 uses in CFileIoAccessCheck_Update, CFileIoAccessCheck_IsFinished
    void* checkItems;                       // 0x10: 1 use in CFileIoAccessCheck_Update
    int checkItemCount;                         // 0x14: 1 use in CFileIoAccessCheck_Update
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFILEIOACCESSCHECKFLASH_OFF(f, o) typedef char CFileIoAccessCheckFlash_off_##f[((int)&((CFileIoAccessCheckFlash*)0)->f == (o)) ? 1 : -1]
CFILEIOACCESSCHECKFLASH_OFF(state, 0xc);
CFILEIOACCESSCHECKFLASH_OFF(checkItems, 0x10);
CFILEIOACCESSCHECKFLASH_OFF(checkItemCount, 0x14);
#undef CFILEIOACCESSCHECKFLASH_OFF

#endif  // CLASSES_CFILEIOACCESSCHECKFLASH_H
