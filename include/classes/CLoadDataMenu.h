#ifndef CLASSES_CLOADDATAMENU_H
#define CLASSES_CLOADDATAMENU_H

#include "game/types.h"

class CLoadDataMenu {
public:
    void* vtbl;                     // 0x0: original vtable pointer
    char pad_004[0x14];             // 0x4
    void* layout;                   // 0x18
    void* bpObject;                 // 0x1c
    int bpEntryId;                  // 0x20
    int selectionDelayFrames;       // 0x24
    unsigned char finished;         // 0x28
    char pad_029[0x3];              // 0x29
    int textFactory;                // 0x2c
    void* messageDraw;              // 0x30
    void* textTable;                // 0x34
    unsigned short slotUsed;        // 0x38
    char pad_03A[0x209e];           // 0x3a
    int selectedSlot;               // 0x20d8
    int result;                     // 0x20dc
    void* packTextureResource;      // 0x20e0
    void* bpTexture;                // 0x20e4
    int savedScriptIndex;           // 0x20e8
    int secondaryBpEntryId;         // 0x20ec
    void* deleteConfirmationDialog; // 0x20f0

    void PollTextureLoadAndUpdateLayout();
    int GetSelectionResult(int* selectedSlotOut);

    int PollLoadComplete();
    void EnterMenu();
    int ProcessInput();
    void StartFadeOrRequestTransition(int enabled);
    void Draw();
    int ResetSlotLayout();
    int ClearSlotDetails();
    int ProcessSlotSelection();
    int ShowSlotPlayTime(int index);
    int UpdateSlotDetails(int index);
    int InitBpGroups();

    CLoadDataMenu();
    ~CLoadDataMenu();
};

#define CLOADDATAMENU_OFF(f, o) typedef char CLoadDataMenu_off_##f[((int)&((CLoadDataMenu*)0)->f == (o)) ? 1 : -1]
CLOADDATAMENU_OFF(layout, 0x18);
CLOADDATAMENU_OFF(bpObject, 0x1c);
CLOADDATAMENU_OFF(bpEntryId, 0x20);
CLOADDATAMENU_OFF(selectionDelayFrames, 0x24);
CLOADDATAMENU_OFF(finished, 0x28);
CLOADDATAMENU_OFF(textFactory, 0x2c);
CLOADDATAMENU_OFF(messageDraw, 0x30);
CLOADDATAMENU_OFF(textTable, 0x34);
CLOADDATAMENU_OFF(slotUsed, 0x38);
CLOADDATAMENU_OFF(selectedSlot, 0x20d8);
CLOADDATAMENU_OFF(result, 0x20dc);
CLOADDATAMENU_OFF(packTextureResource, 0x20e0);
CLOADDATAMENU_OFF(bpTexture, 0x20e4);
CLOADDATAMENU_OFF(savedScriptIndex, 0x20e8);
CLOADDATAMENU_OFF(secondaryBpEntryId, 0x20ec);
CLOADDATAMENU_OFF(deleteConfirmationDialog, 0x20f0);
#undef CLOADDATAMENU_OFF

#endif // CLASSES_CLOADDATAMENU_H
