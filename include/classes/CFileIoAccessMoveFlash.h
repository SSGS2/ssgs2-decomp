// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFILEIOACCESSMOVEFLASH_H
#define CLASSES_CFILEIOACCESSMOVEFLASH_H

#include "game/types.h"

class CFileIoAccessMoveFlash {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x8];                  // 0x4
    int state;                          // 0xc: 7 uses in CFileIoAccessMove_Update, CFileIoAccessMove_IsFinished
    unsigned int secondPathFlags;                // 0x10: 1 use in CFileIoAccessMove_Update
    char pad_014[0x4];                  // 0x14
    void* secondPathData;                       // 0x18: 1 use in CFileIoAccessMove_Update
    unsigned int firstPathFlags;                // 0x1c: 1 use in CFileIoAccessMove_Update
    char pad_020[0x4];                  // 0x20
    void* firstPathData;                       // 0x24: 1 use in CFileIoAccessMove_Update

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x11, 0x1d
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFILEIOACCESSMOVEFLASH_OFF(f, o) typedef char CFileIoAccessMoveFlash_off_##f[((int)&((CFileIoAccessMoveFlash*)0)->f == (o)) ? 1 : -1]
CFILEIOACCESSMOVEFLASH_OFF(state, 0xc);
CFILEIOACCESSMOVEFLASH_OFF(secondPathFlags, 0x10);
CFILEIOACCESSMOVEFLASH_OFF(secondPathData, 0x18);
CFILEIOACCESSMOVEFLASH_OFF(firstPathFlags, 0x1c);
CFILEIOACCESSMOVEFLASH_OFF(firstPathData, 0x24);
#undef CFILEIOACCESSMOVEFLASH_OFF

#endif  // CLASSES_CFILEIOACCESSMOVEFLASH_H
