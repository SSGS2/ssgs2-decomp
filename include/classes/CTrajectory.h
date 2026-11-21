// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTRAJECTORY_H
#define CLASSES_CTRAJECTORY_H

#include "game/types.h"

class CTrajectory {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];
    int unk_8;                         // 0x8: NativeOwner80201D18 constructor initializes 1
    char pad_00C[0xc];
    void* secondaryVtable;             // 0x18: NativeOwner80201D18 installs __vt__11CTrajectory + 0x3c
    void* activeEffect0;                       // 0x1c: 3 uses in CTrajectory_HandleMsg, CTrajectory_OnProcess
    int activeEffect1;                         // 0x20: 1 use in CTrajectory_HandleMsg
    int activeEffect2;                         // 0x24: 1 use in CTrajectory_HandleMsg
    int activeEffect3;                         // 0x28: 1 use in CTrajectory_HandleMsg
    int activeEffect4;                         // 0x2c: 1 use in CTrajectory_HandleMsg
    void* comet;                       // 0x30: 4 uses in CTrajectory_OnDestroy, CTrajectory_OnLoad, CTrajectory_HandleMsg
    void* doughnut;                       // 0x34: 4 uses in CTrajectory_OnDestroy, CTrajectory_OnLoad, CTrajectory_HandleMsg
    void* puttLine;                       // 0x38: 4 uses in CTrajectory_OnDestroy, CTrajectory_OnLoad, CTrajectory_HandleMsg
    void* ballTrack;                       // 0x3c: 4 uses in CTrajectory_OnDestroy, CTrajectory_OnLoad, CTrajectory_HandleMsg
    void* skidMark;                       // 0x40: 5 uses in CTrajectory_OnDestroy, CTrajectory_HandleMsg, CTrajectory_OnLoad
    void* ballWindTrack;                       // 0x44: 4 uses in CTrajectory_OnDestroy, CTrajectory_OnLoad, CTrajectory_HandleMsg
    void* cobraSkidMark;                       // 0x48: 5 uses in CTrajectory_OnDestroy, CTrajectory_HandleMsg, CTrajectory_OnLoad
    void* groundMarker;                       // 0x4c: 6 uses in CTrajectory_HandleMsg, CTrajectory_OnDestroy, CTrajectory_OnLoad

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x18
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTRAJECTORY_OFF(f, o) typedef char CTrajectory_off_##f[((int)&((CTrajectory*)0)->f == (o)) ? 1 : -1]
CTRAJECTORY_OFF(activeEffect0, 0x1c);
CTRAJECTORY_OFF(activeEffect1, 0x20);
CTRAJECTORY_OFF(activeEffect2, 0x24);
CTRAJECTORY_OFF(activeEffect3, 0x28);
CTRAJECTORY_OFF(activeEffect4, 0x2c);
CTRAJECTORY_OFF(comet, 0x30);
CTRAJECTORY_OFF(doughnut, 0x34);
CTRAJECTORY_OFF(puttLine, 0x38);
CTRAJECTORY_OFF(ballTrack, 0x3c);
CTRAJECTORY_OFF(skidMark, 0x40);
CTRAJECTORY_OFF(ballWindTrack, 0x44);
CTRAJECTORY_OFF(cobraSkidMark, 0x48);
CTRAJECTORY_OFF(groundMarker, 0x4c);
CTRAJECTORY_OFF(unk_8, 0x8);
CTRAJECTORY_OFF(secondaryVtable, 0x18);
#undef CTRAJECTORY_OFF

#endif  // CLASSES_CTRAJECTORY_H
