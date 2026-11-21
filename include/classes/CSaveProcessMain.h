// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSAVEPROCESSMAIN_H
#define CLASSES_CSAVEPROCESSMAIN_H

#include "game/types.h"

class CSaveProcessMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;              // 0x18: 1 use in CSaveProcessMain_Ctor
    int retainedRequestWord0;                         // 0x1c: 2 uses in CSaveProcessMain_Ctor, CSaveProcessMain_HandleMsg
    int transitionArg0;                         // 0x20: 1 use in CSaveProcessMain_Ctor
    int transitionArg1;                         // 0x24: 1 use in CSaveProcessMain_Ctor
    int transitionArg2;                         // 0x28: 1 use in CSaveProcessMain_Ctor
    int transitionArg3;                         // 0x2c: 1 use in CSaveProcessMain_Ctor
    int retainedRequestArg0;                         // 0x30: 1 use in CSaveProcessMain_Ctor
    int retainedRequestArg1;                         // 0x34: 1 use in CSaveProcessMain_Ctor
    int transitionArg4;                         // 0x38: 1 use in CSaveProcessMain_Ctor
    int saveMenu;                         // 0x3c: 2 uses in CSaveProcessMain_Ctor, CSaveProcessMain_OnDisplay
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSAVEPROCESSMAIN_OFF(f, o) typedef char CSaveProcessMain_off_##f[((int)&((CSaveProcessMain*)0)->f == (o)) ? 1 : -1]
CSAVEPROCESSMAIN_OFF(secondaryVtable, 0x18);
CSAVEPROCESSMAIN_OFF(retainedRequestWord0, 0x1c);
CSAVEPROCESSMAIN_OFF(transitionArg0, 0x20);
CSAVEPROCESSMAIN_OFF(transitionArg1, 0x24);
CSAVEPROCESSMAIN_OFF(transitionArg2, 0x28);
CSAVEPROCESSMAIN_OFF(transitionArg3, 0x2c);
CSAVEPROCESSMAIN_OFF(retainedRequestArg0, 0x30);
CSAVEPROCESSMAIN_OFF(retainedRequestArg1, 0x34);
CSAVEPROCESSMAIN_OFF(transitionArg4, 0x38);
CSAVEPROCESSMAIN_OFF(saveMenu, 0x3c);
#undef CSAVEPROCESSMAIN_OFF

#endif  // CLASSES_CSAVEPROCESSMAIN_H
