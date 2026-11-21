// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSCORE_H
#define CLASSES_CSCORE_H

#include "game/types.h"

class CScore {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    int bonusOverlayEnabled;                         // 0x1c: 3 uses in CScore_OnProcess, CScore_HandleMsg, CScore_OnDisplay
    float elapsedTime;                       // 0x20: 2 uses in CScore_OnProcess, CScore_HandleMsg
    int transitionState;                         // 0x24: 7 uses in CScore_HandleMsg, CScore_OnProcess
    char pad_028[0x4];                  // 0x28
    float transitionProgress;                       // 0x2c: 10 uses in CScore_OnProcess, CScore_HandleMsg
    char pad_030[0x8];                  // 0x30
    void* bonusOverlay;                       // 0x38: 7 uses in CScore_OnProcess, CScore_HandleMsg, CScore_OnDisplay
    int result;                         // 0x3c: 8 uses in CScore_OnProcess, CScore_HandleMsg
    int scoreMenuActive;                         // 0x40: 4 uses in CScore_OnProcess, CScore_HandleMsg, CScore_OnDisplay
    int currentPlayerId;                         // 0x44: 2 uses in CScore_HandleMsg
    char pad_048[0x8];                  // 0x48
    int rankViewActive;                         // 0x50: 6 uses in CScore_OnProcess, CScore_HandleMsg, CScore_OnDisplay
    int menusInitialized;                         // 0x54: 6 uses in CScore_OnProcess, CScore_OnDisplay, CScore_HandleMsg
    char pad_058[0x4];                  // 0x58
    void* scoreForm;                       // 0x5c: 22 uses in CScore_HandleMsg, CScore_OnProcess, CScore_OnDisplay
    void* rankView;                       // 0x60: 10 uses in CScore_OnProcess, CScore_HandleMsg, CScore_OnDisplay
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSCORE_OFF(f, o) typedef char CScore_off_##f[((int)&((CScore*)0)->f == (o)) ? 1 : -1]
CSCORE_OFF(bonusOverlayEnabled, 0x1c);
CSCORE_OFF(elapsedTime, 0x20);
CSCORE_OFF(transitionState, 0x24);
CSCORE_OFF(transitionProgress, 0x2c);
CSCORE_OFF(bonusOverlay, 0x38);
CSCORE_OFF(result, 0x3c);
CSCORE_OFF(scoreMenuActive, 0x40);
CSCORE_OFF(currentPlayerId, 0x44);
CSCORE_OFF(rankViewActive, 0x50);
CSCORE_OFF(menusInitialized, 0x54);
CSCORE_OFF(scoreForm, 0x5c);
CSCORE_OFF(rankView, 0x60);
#undef CSCORE_OFF

#endif  // CLASSES_CSCORE_H
