#ifndef CLASSES_CFILEIOACCESSMOVEFLASH_H
#define CLASSES_CFILEIOACCESSMOVEFLASH_H

#include "game/types.h"

class CFileIoAccessMoveFlash {
public:
    void* vtbl;                   // 0x0: original vtable pointer
    char pad_004[0x8];            // 0x4
    int state;                    // 0xc
    unsigned int secondPathFlags; // 0x10
    char pad_014[0x4];            // 0x14
    void* secondPathData;         // 0x18
    unsigned int firstPathFlags;  // 0x1c
    char pad_020[0x4];            // 0x20
    void* firstPathData;          // 0x24

    static void* CreateDevice();
    static void* Create(void* destinationPath, void* sourcePath);

    CFileIoAccessMoveFlash();
    ~CFileIoAccessMoveFlash();
};

typedef char CFileIoAccessMoveFlash_size[(sizeof(CFileIoAccessMoveFlash) == 0x28) ? 1 : -1];

#define CFILEIOACCESSMOVEFLASH_OFF(f, o) \
    typedef char CFileIoAccessMoveFlash_off_##f[((int)&((CFileIoAccessMoveFlash*)0)->f == (o)) ? 1 : -1]
CFILEIOACCESSMOVEFLASH_OFF(state, 0xc);
CFILEIOACCESSMOVEFLASH_OFF(secondPathFlags, 0x10);
CFILEIOACCESSMOVEFLASH_OFF(secondPathData, 0x18);
CFILEIOACCESSMOVEFLASH_OFF(firstPathFlags, 0x1c);
CFILEIOACCESSMOVEFLASH_OFF(firstPathData, 0x24);
#undef CFILEIOACCESSMOVEFLASH_OFF

#endif // CLASSES_CFILEIOACCESSMOVEFLASH_H
