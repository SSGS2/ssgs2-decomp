#ifndef CLASSES_CLOADDATAMENUINENTRY_H
#define CLASSES_CLOADDATAMENUINENTRY_H

#include "game/types.h"

class CLoadDataMenuInEntry {
public:
    void* vtbl; // 0x0: original vtable pointer
    char pad_004[0x14];
    void* layout;                    // 0x18: ResetSlotLayout (0x8033B5B8): element rectangles and player flags
    void* bpObject;                  // 0x1C: ClearSlotDetails (0x8033B904): UIGroup_GetOrCreateById receiver
    int bpEntryId;                   // 0x20
    unsigned char selectionComplete; // 0x24
    char pad_025[0x3];               // 0x25
    unsigned short firstSlotUsed;    // 0x28
    char pad_02A[0x28c6];            // 0x2a
    int selectedItem;                // 0x28f0
    int selectionDelayFrames;        // 0x28f4
    char pad_28F8[0x4];
    void* slotDetailsText;   // 0x28FC: UpdateSlotDetails (0x8033B6F8): Text_SetContent / CMsgDraw_ResetTextState
    void* playTimeTextTable; // 0x2900: ShowSlotPlayTime (0x8033B9F8): entries 0x27/0x26/0x25 are hours/minutes/seconds
    void* memoryLoadMenu;    // 0x2904: UpdateMemoryLoad (0x8033BB1C): MPBase1_Process / CPadMemoryMenuLoad_CopySaveData
    char pad_2908[0x4];      // 0x2908
    unsigned char memoryRecordLoaded; // 0x290c

    void PollTextureAndUpdateLayout();
    int Update(float delta);
    int CopySelectedRecord(void* out);
    int PollLoadComplete();
    void EnterMenu();
    void Draw();
    int ProcessSlotSelection();
    int InitBpGroups();

    int ClearSlotDetails();
    int UpdateMemoryLoad(float dt);
    int ResetSlotLayout();
    int ShowSlotPlayTime(int index);
    int UpdateSlotDetails(int index);
    int UpdateSelectionDelay();

    ~CLoadDataMenuInEntry();
    CLoadDataMenuInEntry(int arg);
};

#define CLOADDATAMENUINENTRY_OFF(f, o) \
    typedef char CLoadDataMenuInEntry_off_##f[((int)&((CLoadDataMenuInEntry*)0)->f == (o)) ? 1 : -1]
CLOADDATAMENUINENTRY_OFF(layout, 0x18);
CLOADDATAMENUINENTRY_OFF(bpObject, 0x1C);
CLOADDATAMENUINENTRY_OFF(slotDetailsText, 0x28FC);
CLOADDATAMENUINENTRY_OFF(playTimeTextTable, 0x2900);
CLOADDATAMENUINENTRY_OFF(memoryLoadMenu, 0x2904);
CLOADDATAMENUINENTRY_OFF(bpEntryId, 0x20);
CLOADDATAMENUINENTRY_OFF(selectionComplete, 0x24);
CLOADDATAMENUINENTRY_OFF(firstSlotUsed, 0x28);
CLOADDATAMENUINENTRY_OFF(selectedItem, 0x28f0);
CLOADDATAMENUINENTRY_OFF(selectionDelayFrames, 0x28f4);
CLOADDATAMENUINENTRY_OFF(memoryRecordLoaded, 0x290c);
#undef CLOADDATAMENUINENTRY_OFF

#endif // CLASSES_CLOADDATAMENUINENTRY_H
