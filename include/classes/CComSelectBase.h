#ifndef CLASSES_CCOMSELECTBASE_H
#define CLASSES_CCOMSELECTBASE_H

#include "game/types.h"

class CComSelectBase {
public:
    void* vtbl;                                   // 0x0: original vtable pointer
    char pad_004[0x14];                           // 0x4
    int bpEntryId;                                // 0x18
    int selectedEntry;                            // 0x1c
    int pageIndex;                                // 0x20
    char pad_024[0x4];                            // 0x24
    float inputDelay;                             // 0x28
    int requestedPageIndex;                       // 0x2c
    void* textureResource;                        // 0x30
    void* layout;                                 // 0x34
    void* textRenderer;                           // 0x38
    void* optionDialog;                           // 0x3c
    unsigned char optionDialogPending;            // 0x40
    char pad_041[0x3];                            // 0x41
    void* courseClearDialog;                      // 0x44
    unsigned char courseClearNotificationPending; // 0x48
    char pad_049[0x3];                            // 0x49
    int secondaryBpEntryId;                       // 0x4c
    int tertiaryBpEntryId;                        // 0x50

    int SetPage(int pageIndex);
    void DrawRowSprite(void* object, float* position);
    void DrawPrimaryRowSprites();
    void DrawSecondarySprites();
    void DrawEntryText(int rowIndex);
    int SetPageCursor(int pageIndex);
    int GetEntryIndex(int pageIndex, int rowIndex);
    void StartMenuScripts();
    void PollResourceLoading();
    int PollLoadComplete();
    void Draw();
    int Update(float deltaTime);
    void* Init();

    void DrawEntryLabel(int idx);
    void DrawHoveredEntryText();
};

#define CCOMSELECTBASE_OFF(f, o) typedef char CComSelectBase_off_##f[((int)&((CComSelectBase*)0)->f == (o)) ? 1 : -1]
CCOMSELECTBASE_OFF(bpEntryId, 0x18);
CCOMSELECTBASE_OFF(selectedEntry, 0x1c);
CCOMSELECTBASE_OFF(pageIndex, 0x20);
CCOMSELECTBASE_OFF(inputDelay, 0x28);
CCOMSELECTBASE_OFF(requestedPageIndex, 0x2c);
CCOMSELECTBASE_OFF(textureResource, 0x30);
CCOMSELECTBASE_OFF(layout, 0x34);
CCOMSELECTBASE_OFF(textRenderer, 0x38);
CCOMSELECTBASE_OFF(optionDialog, 0x3c);
CCOMSELECTBASE_OFF(optionDialogPending, 0x40);
CCOMSELECTBASE_OFF(courseClearDialog, 0x44);
CCOMSELECTBASE_OFF(courseClearNotificationPending, 0x48);
CCOMSELECTBASE_OFF(secondaryBpEntryId, 0x4c);
CCOMSELECTBASE_OFF(tertiaryBpEntryId, 0x50);
#undef CCOMSELECTBASE_OFF

#endif // CLASSES_CCOMSELECTBASE_H
