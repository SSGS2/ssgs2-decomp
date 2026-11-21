// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTOURMAIN_H
#define CLASSES_CTOURMAIN_H

#include "game/types.h"

class CTourMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;              // 0x18: 2 uses in CTourMain_Ctor, CTourMain_dtor
    void* tourSelect;                   // 0x1c: 9 uses in CTourMain_OnProcess, CTourMain_dtor, CTourMain_Ctor...
    void* tourConfirm;                       // 0x20: 12 uses in CTourMain_OnProcess, CTourMain_dtor, CTourMain_Ctor...
    int state;                          // 0x24: 6 uses in CTourMain_OnProcess, CTourMain_Ctor, CTourMain_OnDisplay
    int pendingState;                   // 0x28: 19 uses in CTourMain_OnProcess, CTourMain_Ctor
    int exitDestination;                         // 0x2c: 4 uses in CTourMain_OnProcess, CTourMain_Ctor
    int messageLatch;                         // 0x30: 3 uses in CTourMain_HandleMsg, CTourMain_Ctor
    char playerRecordCopy[0x828];                // 0x34
    void* saveMenu;                     // 0x85c: 7 uses in CTourMain_OnProcess, CTourMain_Ctor, CTourMain_OnDisplay

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x34
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTOURMAIN_OFF(f, o) typedef char CTourMain_off_##f[((int)&((CTourMain*)0)->f == (o)) ? 1 : -1]
CTOURMAIN_OFF(secondaryVtable, 0x18);
CTOURMAIN_OFF(tourSelect, 0x1c);
CTOURMAIN_OFF(tourConfirm, 0x20);
CTOURMAIN_OFF(state, 0x24);
CTOURMAIN_OFF(pendingState, 0x28);
CTOURMAIN_OFF(exitDestination, 0x2c);
CTOURMAIN_OFF(messageLatch, 0x30);
CTOURMAIN_OFF(saveMenu, 0x85c);
#undef CTOURMAIN_OFF

#endif  // CLASSES_CTOURMAIN_H
