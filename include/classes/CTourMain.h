#ifndef CLASSES_CTOURMAIN_H
#define CLASSES_CTOURMAIN_H

#include "game/types.h"

class CTourMain {
public:
    void* vtbl;                   // 0x0: original vtable pointer
    char pad_004[0x14];           // 0x4
    void* secondaryVtable;        // 0x18
    void* tourSelect;             // 0x1c
    void* tourConfirm;            // 0x20
    int state;                    // 0x24
    int pendingState;             // 0x28
    int exitDestination;          // 0x2c
    int messageLatch;             // 0x30
    char playerRecordCopy[0x828]; // 0x34
    void* saveMenu;               // 0x85c

    void InitItemMap(float dt);
    int PollItemMapResources();
    void UpdateItemMenuVisuals(float dt);
    void ResetItemMapControls();
    int PrepareSelectedEncounter(void* data);
    int ProcessSelectedMapItem();
    void UpdateItemMapLayout();
    void DrawItemMenu();
    void HandleItemMapNavigation();
    int ProcessItemMenu(float dt);

    static void OnPreLoadInit();
    static void OnLoad();
    static void OnInit();
    static void OnDestroy();
    static void* GetRTTI();
    void HandleMsg(int* state);
    void OnDisplay();
    void OnProcess(float deltaSeconds);

    CTourMain();
    ~CTourMain();

    // Destructor entry for the secondary interface's vtable: its this pointer is
    // 0x18 bytes past the object's start.
    void DtorThunk18(int deleting);
};

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

#endif // CLASSES_CTOURMAIN_H
