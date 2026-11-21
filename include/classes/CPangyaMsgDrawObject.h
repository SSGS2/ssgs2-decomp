// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPANGYAMSGDRAWOBJECT_H
#define CLASSES_CPANGYAMSGDRAWOBJECT_H

#include "game/types.h"

class CPangyaMsgDrawObject {
public:
    void* unk_0;                        // 0x0: 1 use in CPangyaMsgDrawObject_Create
    void* drawVtable;                        // 0x4: 1 use in CPangyaMsgDrawObject_dtor
    void* textFactory;                        // 0x8: 2 uses in CPangyaMsgDrawObject_dtor
    void* messageDraw;                        // 0xc: 2 uses in CPangyaMsgDrawObject_dtor
    void* stringFlags; // 0x10: CPangyaMsgDrawObject_Create (0x8026897C) zero-initializes
    void* stringLength; // 0x14: CPangyaMsgDrawObject_Create (0x8026897C) zero-initializes
    void* stringData;                       // 0x18: 1 use in CPangyaMsgDrawObject_dtor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPANGYAMSGDRAWOBJECT_OFF(f, o) typedef char CPangyaMsgDrawObject_off_##f[((int)&((CPangyaMsgDrawObject*)0)->f == (o)) ? 1 : -1]
CPANGYAMSGDRAWOBJECT_OFF(drawVtable, 0x4);
CPANGYAMSGDRAWOBJECT_OFF(textFactory, 0x8);
CPANGYAMSGDRAWOBJECT_OFF(messageDraw, 0xc);
CPANGYAMSGDRAWOBJECT_OFF(stringFlags, 0x10);
CPANGYAMSGDRAWOBJECT_OFF(stringLength, 0x14);
CPANGYAMSGDRAWOBJECT_OFF(stringData, 0x18);
#undef CPANGYAMSGDRAWOBJECT_OFF

#endif  // CLASSES_CPANGYAMSGDRAWOBJECT_H
