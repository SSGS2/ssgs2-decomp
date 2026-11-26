#ifndef CLASSES_CPANGYAMSGDRAWOBJECT_H
#define CLASSES_CPANGYAMSGDRAWOBJECT_H

#include "game/types.h"

class CPangyaMsgDrawObject {
public:
    void* unk_0;        // 0x0
    void* drawVtable;   // 0x4
    void* textFactory;  // 0x8
    void* messageDraw;  // 0xc
    void* stringFlags;  // 0x10: CPangyaMsgDrawObject_Create (0x8026897C) zero-initializes
    void* stringLength; // 0x14: CPangyaMsgDrawObject_Create (0x8026897C) zero-initializes
    void* stringData;   // 0x18

    int Create();

    ~CPangyaMsgDrawObject();
};

#define CPANGYAMSGDRAWOBJECT_OFF(f, o) \
    typedef char CPangyaMsgDrawObject_off_##f[((int)&((CPangyaMsgDrawObject*)0)->f == (o)) ? 1 : -1]
CPANGYAMSGDRAWOBJECT_OFF(drawVtable, 0x4);
CPANGYAMSGDRAWOBJECT_OFF(textFactory, 0x8);
CPANGYAMSGDRAWOBJECT_OFF(messageDraw, 0xc);
CPANGYAMSGDRAWOBJECT_OFF(stringFlags, 0x10);
CPANGYAMSGDRAWOBJECT_OFF(stringLength, 0x14);
CPANGYAMSGDRAWOBJECT_OFF(stringData, 0x18);
#undef CPANGYAMSGDRAWOBJECT_OFF

#endif // CLASSES_CPANGYAMSGDRAWOBJECT_H
