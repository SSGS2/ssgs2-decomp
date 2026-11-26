#ifndef CLASSES_CCHARASELECT_H
#define CLASSES_CCHARASELECT_H

#include "game/types.h"

class CCharaSelect {
public:
    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x14]; // 0x4
    union {
        int playerState;     // Existing scalar name retained.
        int playerStates[4]; // 0x18: 0x80243434/0x80243784/0x802438C0 set indexed states 2/1/0.
    };
    int selectionConfirmed;   // 0x28: CCharaSelect_UpdateAndPollFinished activation result
    void* layout;             // 0x2c
    char pad_030[0x4];        // 0x30
    void* playerTransitions;  // 0x34
    void* primaryBpEntryId;   // 0x38
    void* secondaryBpEntryId; // 0x3c
    int inputReady;           // 0x40: CCharaSelect_UpdateAndPollFinished skips input once, then sets this to 1.
    char pad_044[0x10];
    void* loadDataMenu; // 0x54
    int selectedSlot;   // 0x58: vfunc_5 stores elem before constructing modal object.

    int UpdateHoveredAvatars();

    void ClearSlot(int index, int slot);
    void Draw();
    void ResetPlayerSlot(int index);
    void SetRandomSlot(int playerIndex, int controllerSlot, int computerPlayer, int updatePreview);
    void SetupSlotAvatar(int playerIndex, int controllerSlot, int characterIndex);
    int UpdateAndPollFinished(float deltaTime);
    int IsSelectionConfirmed();

    void InitializeSelectionScreen();
    void UpdateLayoutAndTexture();
    void UpdateSlotInputAvailability();
    int PollLoadComplete();

    CCharaSelect();
    ~CCharaSelect();
};

#define CCHARASELECT_OFF(f, o) typedef char CCharaSelect_off_##f[((int)&((CCharaSelect*)0)->f == (o)) ? 1 : -1]
CCHARASELECT_OFF(playerState, 0x18);
CCHARASELECT_OFF(layout, 0x2c);
CCHARASELECT_OFF(playerTransitions, 0x34);
CCHARASELECT_OFF(primaryBpEntryId, 0x38);
CCHARASELECT_OFF(secondaryBpEntryId, 0x3c);
CCHARASELECT_OFF(loadDataMenu, 0x54);
CCHARASELECT_OFF(playerStates, 0x18);
CCHARASELECT_OFF(selectionConfirmed, 0x28);
CCHARASELECT_OFF(inputReady, 0x40);
CCHARASELECT_OFF(selectedSlot, 0x58);
#undef CCHARASELECT_OFF

#endif // CLASSES_CCHARASELECT_H
