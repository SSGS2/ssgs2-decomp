// CPartyMain: the party-mode main task.
//
// It owns the active selection screen, the two BP entries behind it, and the
// state that walks through the player-count, character, course and party-item
// screens.
#ifndef CLASSES_CPARTYMAIN_H
#define CLASSES_CPARTYMAIN_H

#include "game/types.h"

class CPartyMain {
public:
    ~CPartyMain();

    static void OnLoad();
    void OnDestroy();

    // Forward party lifecycle messages, or start party setup from message 4.
    void HandleMsg(void* message);

    // Destructor entry of the task-main interface, which is 0x18 bytes past the object's start.
    void* DestroyTaskMainInterface(int deleteFlag);

    // Advance the active selection screen and switch to the next one when it
    // finishes, or request the golf task after the last one.
    void ProcessSelectionScreens(float delta);

    void DrawSelectionScreens();

    static void* GetRTTI();

    void* vtbl;                 // 0x00  original vtable pointer
    char pad_004[0x14];         // 0x04
    void* taskMainVtable;       // 0x18  the task-main interface's vtable
    int state;                  // 0x1C  which selection screen is active
    void* selectionScreen;      // 0x20  the one owned screen, or null
    void* primaryBpEntryId;     // 0x24  background BP entry
    void* secondaryBpEntryId;   // 0x28  caption BP entry
    int initialized;            // 0x2C  background and caption set up
    char courseSelection[0x0C]; // 0x30  result of the course selection screen
    char pad_03C[0x10];         // 0x3C
};

#define CPARTYMAIN_OFF(f, o) typedef char CPartyMain_off_##f[((int)&((CPartyMain*)0)->f == (o)) ? 1 : -1]
CPARTYMAIN_OFF(taskMainVtable, 0x18);
CPARTYMAIN_OFF(state, 0x1C);
CPARTYMAIN_OFF(selectionScreen, 0x20);
CPARTYMAIN_OFF(primaryBpEntryId, 0x24);
CPARTYMAIN_OFF(secondaryBpEntryId, 0x28);
CPARTYMAIN_OFF(initialized, 0x2C);
CPARTYMAIN_OFF(courseSelection, 0x30);
#undef CPARTYMAIN_OFF
typedef char CPartyMain_size[(sizeof(CPartyMain) == 0x4C) ? 1 : -1];

#endif // CLASSES_CPARTYMAIN_H
