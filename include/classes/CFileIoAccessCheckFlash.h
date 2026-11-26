#ifndef CLASSES_CFILEIOACCESSCHECKFLASH_H
#define CLASSES_CFILEIOACCESSCHECKFLASH_H

#include "game/types.h"

class CFileIoAccessCheckFlash {
public:
    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x8];  // 0x4
    int state;          // 0xc
    void* checkItems;   // 0x10
    int checkItemCount; // 0x14

    ~CFileIoAccessCheckFlash();

    static void* CreateFlash(void* a, void* b, void* c);
    int SetupWithBlockList(void* a, void* b, void* src, unsigned int count);
    static void* Create(void* a, void* b, void* src, int count);
};

#define CFILEIOACCESSCHECKFLASH_OFF(f, o) \
    typedef char CFileIoAccessCheckFlash_off_##f[((int)&((CFileIoAccessCheckFlash*)0)->f == (o)) ? 1 : -1]
CFILEIOACCESSCHECKFLASH_OFF(state, 0xc);
CFILEIOACCESSCHECKFLASH_OFF(checkItems, 0x10);
CFILEIOACCESSCHECKFLASH_OFF(checkItemCount, 0x14);
#undef CFILEIOACCESSCHECKFLASH_OFF

#endif // CLASSES_CFILEIOACCESSCHECKFLASH_H
