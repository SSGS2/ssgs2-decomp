#ifndef CLASSES_CDARTSSYSTEM_H
#define CLASSES_CDARTSSYSTEM_H

#include "game/types.h"

struct DartsInfo;
class CDartsSystem {
public:
    void* vtbl;            // 0x0: original vtable pointer
    char pad_004[0x14];    // 0x4
    int hasAimPosition;    // 0x18
    char aimPosition[0xc]; // 0x1c
    float position[3];     // 0x28: CDartsSystem_HandleMsg cases 1/4 subtract from ball position
    float heading;         // 0x34
    int holeEntryIds
        [18];            // 0x38: DartsResource and UpdatePlayerRanks select CMiniGameAim_GetEntry entry by current hole
    void* aimTable;      // 0x80
    void* target;        // 0x84
    void* plates;        // 0x88
    void* dialog;        // 0x8c
    int dialogRequested; // 0x90

    void OnDisplay();
    void OnProcess(float deltaSeconds);
    void OnDestroy();
    void OnLoad();
    void OnInit();
    void OnPreLoadInit();

    static void* GetRTTI();
    void HandleMsg(const DartsInfo* info);
    void UpdatePlayerRanks(void* /*info*/);

    ~CDartsSystem();
    CDartsSystem();
};

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

#endif // CLASSES_CDARTSSYSTEM_H
