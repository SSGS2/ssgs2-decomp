// CIngameStatus: explicit layout view; no inheritance, virtual methods, or RTTI.
// Field comments cite the original function addresses that establish their use.
#ifndef CLASSES_CINGAMESTATUS_H
#define CLASSES_CINGAMESTATUS_H

#include "game/types.h"

class CIngameStatus {
public:
    void* shotCountInfo; // 0x0: CIngameStatus_Ctor/80328EF8: shot-count object pointer
    int state; // 0x4: CIngameStatus_Update/803289EC: state machine and reset
    unsigned int savedPlayerFlags; // 0x8: CIngameStatus_Update: cached comparison with player +0x358
    int bpEntryId; // 0xC: CIngameStatus_Dtor: CBpCtrl entry ID
    void* bpObject; // 0x10: CIngameStatus_Update: BpObject_PollLoad/Draw
    int animationState; // 0x14: CIngameStatus_ResetState: cleared on reset
    int initialized; // 0x18: CIngameStatus_Update: one-time UI initialization guard
    void* textTable; // 0x1C: CIngameStatus_Update: CFontNumDraw_RegisterEntries receiver
    void* layout; // 0x20: CIngameStatus_Update: layout/status receiver
};

#define CINGAMESTATUS_OFF(f, o) typedef char CIngameStatus_off_##f[((int)&((CIngameStatus*)0)->f == (o)) ? 1 : -1]
CINGAMESTATUS_OFF(shotCountInfo, 0x0);
CINGAMESTATUS_OFF(state, 0x4);
CINGAMESTATUS_OFF(savedPlayerFlags, 0x8);
CINGAMESTATUS_OFF(bpEntryId, 0xC);
CINGAMESTATUS_OFF(bpObject, 0x10);
CINGAMESTATUS_OFF(animationState, 0x14);
CINGAMESTATUS_OFF(initialized, 0x18);
CINGAMESTATUS_OFF(textTable, 0x1C);
CINGAMESTATUS_OFF(layout, 0x20);
#undef CINGAMESTATUS_OFF

#endif
