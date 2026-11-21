// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDARTSSYSTEM_H
#define CLASSES_CDARTSSYSTEM_H

#include "game/types.h"

class CDartsSystem {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int hasAimPosition;                         // 0x18: 1 use in CDartsSystem_HandleMsg
    char pad_01C[0xc];                 // 0x1c
    float position[3]; // 0x28: CDartsSystem_HandleMsg cases 1/4 subtract from ball position
    float heading;                       // 0x34: 1 use in CDartsSystem_HandleMsg
    int holeEntryIds[18];               // 0x38: DartsResource and UpdatePlayerRanks select CMiniGameAim_GetEntry entry by current hole
    void* aimTable;                       // 0x80: 1 use in CDartsSystem_HandleMsg
    void* target;                       // 0x84: 1 use in CDartsSystem_OnProcess
    void* plates;                       // 0x88: 1 use in CDartsSystem_OnProcess
    void* dialog;                       // 0x8c: 10 uses in CDartsSystem_OnProcess, CDartsSystem_HandleMsg
    int dialogRequested;                         // 0x90: 3 uses in CDartsSystem_HandleMsg

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x1c, 0x38
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDARTSSYSTEM_OFF(f, o) typedef char CDartsSystem_off_##f[((int)&((CDartsSystem*)0)->f == (o)) ? 1 : -1]
CDARTSSYSTEM_OFF(hasAimPosition, 0x18);
CDARTSSYSTEM_OFF(position, 0x28);
CDARTSSYSTEM_OFF(heading, 0x34);
CDARTSSYSTEM_OFF(holeEntryIds, 0x38);
CDARTSSYSTEM_OFF(aimTable, 0x80);
CDARTSSYSTEM_OFF(target, 0x84);
CDARTSSYSTEM_OFF(plates, 0x88);
CDARTSSYSTEM_OFF(dialog, 0x8c);
CDARTSSYSTEM_OFF(dialogRequested, 0x90);
#undef CDARTSSYSTEM_OFF

#endif  // CLASSES_CDARTSSYSTEM_H
