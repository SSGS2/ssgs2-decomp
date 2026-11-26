#ifndef CLASSES_CSAVEFORMBANNER_H
#define CLASSES_CSAVEFORMBANNER_H

#include "game/types.h"

class CSaveFormBanner {
public:
    void* vtbl;               // 0x0: original vtable pointer
    unsigned int recordSize;  // 0x4
    unsigned int recordCount; // 0x8
    unsigned char hasMd5;     // 0xc

    static void* Create();
    int vfunc_2();

    ~CSaveFormBanner();
};

#define CSAVEFORMBANNER_OFF(f, o) typedef char CSaveFormBanner_off_##f[((int)&((CSaveFormBanner*)0)->f == (o)) ? 1 : -1]
CSAVEFORMBANNER_OFF(recordSize, 0x4);
CSAVEFORMBANNER_OFF(recordCount, 0x8);
CSAVEFORMBANNER_OFF(hasMd5, 0xc);
#undef CSAVEFORMBANNER_OFF

#endif // CLASSES_CSAVEFORMBANNER_H
