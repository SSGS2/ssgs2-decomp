// CDriveSystem: the Driving Contest mini-game actor. It owns the aim table and
// the result dialog, and answers the contest's distance, ranking and dialog
// requests through HandleMsg.
#ifndef CLASSES_CDRIVESYSTEM_H
#define CLASSES_CDRIVESYSTEM_H

#include "game/types.h"

struct DartsInfo;
class CDriveSystem {
public:
    void* vtbl;          // 0x0: original vtable pointer
    char pad_004[0x4];   // 0x4
    int unk_8;           // 0x8
    char pad_00C[0xc];   // 0xc
    void* miniGameAim;   // 0x18
    void* dialog;        // 0x1c
    int dialogRequested; // 0x20

    void OnLoad();
    void OnInit();
    void OnPreLoadInit();
    void OnProcess(float delta);
    void OnDisplay();
    void HandleMsg(const DartsInfo* info);
    void OnDestroy();

    // Snapshot the players' distances and publish the drive contest's tied and
    // ordered ranks. Callers require one to four live players.
    void UpdateDistanceRanks();

    static void* GetRTTI();

    ~CDriveSystem();
    CDriveSystem();
};

#define CDRIVESYSTEM_OFF(f, o) typedef char CDriveSystem_off_##f[((int)&((CDriveSystem*)0)->f == (o)) ? 1 : -1]
CDRIVESYSTEM_OFF(unk_8, 0x8);
CDRIVESYSTEM_OFF(miniGameAim, 0x18);
CDRIVESYSTEM_OFF(dialog, 0x1c);
CDRIVESYSTEM_OFF(dialogRequested, 0x20);
#undef CDRIVESYSTEM_OFF

#endif // CLASSES_CDRIVESYSTEM_H
