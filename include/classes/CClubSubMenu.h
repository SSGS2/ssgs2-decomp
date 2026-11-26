#ifndef CLASSES_CCLUBSUBMENU_H
#define CLASSES_CCLUBSUBMENU_H

#include "game/types.h"

class CClubSubMenu {
public:
    void* vtbl;                      // 0x0: original vtable pointer
    void* layout;                    // 0x4
    int selectedElementIndex;        // 0x8
    int menuVariant;                 // 0xc
    int selectionDelayFrames;        // 0x10
    int selectionResult;             // 0x14
    unsigned char firstChoiceMarked; // 0x18

    int IsSecondaryActionSelected();
    int ProcessNavigation();
    int ProcessConfirmation();
    int UpdateSelectionDelay();
    static int UpdateSelectionHighlight(void* menu);
    int GetSelectionResult(int* resultOut);
    static int SetPrimaryElementGroupEnabled(void* menu, int firstGroupEnabled);
    int Update();
    int UpdateLayout();
    int ResetLayoutMode();
    int Draw();
    int OpenVariant(int variant, unsigned char firstChoiceMarked);

    CClubSubMenu();
    ~CClubSubMenu();
};

#define CCLUBSUBMENU_OFF(f, o) typedef char CClubSubMenu_off_##f[((int)&((CClubSubMenu*)0)->f == (o)) ? 1 : -1]
CCLUBSUBMENU_OFF(layout, 0x4);
CCLUBSUBMENU_OFF(selectedElementIndex, 0x8);
CCLUBSUBMENU_OFF(menuVariant, 0xc);
CCLUBSUBMENU_OFF(selectionDelayFrames, 0x10);
CCLUBSUBMENU_OFF(selectionResult, 0x14);
CCLUBSUBMENU_OFF(firstChoiceMarked, 0x18);
#undef CCLUBSUBMENU_OFF

#endif // CLASSES_CCLUBSUBMENU_H
