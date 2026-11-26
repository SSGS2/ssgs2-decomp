// CFlashMemoryDebug: the flash-memory debug settings singleton.
//
// One instance lives in .bss and is built on first use. Only the synchronous
// flag is read elsewhere; the remaining words have no established meaning.
#ifndef CLASSES_CFLASHMEMORYDEBUG_H
#define CLASSES_CFLASHMEMORYDEBUG_H

#include "game/types.h"

class CFlashMemoryDebug {
public:
    static CFlashMemoryDebug* GetInstance();
    ~CFlashMemoryDebug();

    void* vtbl;                // 0x0
    char pad_004[6];           // 0x4
    unsigned char synchronous; // 0xA
    char pad_00B;              // 0xB
    int pad_00C;               // 0xC
    int pad_010;               // 0x10
    int pad_014;               // 0x14
};

#define CFLASHMEMORYDEBUG_OFF(f, o) \
    typedef char CFlashMemoryDebug_off_##f[((int)&((CFlashMemoryDebug*)0)->f == (o)) ? 1 : -1]
CFLASHMEMORYDEBUG_OFF(vtbl, 0x0);
CFLASHMEMORYDEBUG_OFF(synchronous, 0xA);
#undef CFLASHMEMORYDEBUG_OFF
typedef char CFlashMemoryDebug_size[(sizeof(CFlashMemoryDebug) == 0x18) ? 1 : -1];

#endif // CLASSES_CFLASHMEMORYDEBUG_H
