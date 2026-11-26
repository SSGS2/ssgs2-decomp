#ifndef CLASSES_CGURUGURU_H
#define CLASSES_CGURUGURU_H

#include "game/types.h"

class CGuruguru {
public:
    void* vtbl;          // 0x0: original vtable pointer
    void* unk_4;         // 0x4
    void* unk_8;         // 0x8
    void* unk_C;         // 0xc
    void* unk_10;        // 0x10
    char pad_014[0x8];   // 0x14
    void* unk_1C;        // 0x1c
    char pad_020[0x86c]; // 0x20
    void* fontResource;  // 0x88c

    int vfunc_8();
    int vfunc_7();
    int vfunc_6();
    int vfunc_5();
    int vfunc_4();
    int vfunc_3();
    int vfunc_2();
    int vfunc_1();
    void* vfunc_0(int flag);
};

#define CGURUGURU_OFF(f, o) typedef char CGuruguru_off_##f[((int)&((CGuruguru*)0)->f == (o)) ? 1 : -1]
CGURUGURU_OFF(unk_4, 0x4);
CGURUGURU_OFF(unk_8, 0x8);
CGURUGURU_OFF(unk_C, 0xc);
CGURUGURU_OFF(unk_10, 0x10);
CGURUGURU_OFF(unk_1C, 0x1c);
CGURUGURU_OFF(fontResource, 0x88c);
#undef CGURUGURU_OFF

#endif // CLASSES_CGURUGURU_H
