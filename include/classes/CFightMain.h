// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFIGHTMAIN_H
#define CLASSES_CFIGHTMAIN_H

#include "game/types.h"

class CFightMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;              // 0x18: 1 use in CFightMain_Ctor
    int selectionState;                 // 0x1c: 2 uses in CFightMain_HandleMsg
    void* selectionMenu;                // 0x20: 8 uses in CFightMain_HandleMsg
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFIGHTMAIN_OFF(f, o) typedef char CFightMain_off_##f[((int)&((CFightMain*)0)->f == (o)) ? 1 : -1]
CFIGHTMAIN_OFF(secondaryVtable, 0x18);
CFIGHTMAIN_OFF(selectionState, 0x1c);
CFIGHTMAIN_OFF(selectionMenu, 0x20);
#undef CFIGHTMAIN_OFF

#endif  // CLASSES_CFIGHTMAIN_H
