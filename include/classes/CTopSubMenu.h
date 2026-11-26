#ifndef CLASSES_CTOPSUBMENU_H
#define CLASSES_CTOPSUBMENU_H

#include "game/types.h"

class CTopSubMenu {
public:
    void* vtbl;                // 0x0: original vtable pointer
    char pad_004[0x14];        // 0x4
    void* interfaceVtable;     // 0x18
    void* layout;              // 0x1c
    void* bpObject;            // 0x20
    char pad_024[0x4];         // 0x24
    unsigned char resultReady; // 0x28: CTopSubMenu_HandleSelection sets; CTopSubMenu_TryGetResult gates result output
    unsigned char selectionComplete;     // 0x29
    char pad_02A[0x2];                   // 0x2a
    int menuItemCount;                   // 0x2c
    unsigned char scriptRunning;         // 0x30
    char pad_031[0x3];                   // 0x31
    void* menuMessage;                   // 0x34
    void* optionDialog;                  // 0x38
    unsigned char optionDialogRequested; // 0x3c
    char pad_03D[0x13];                  // 0x3d
    int menuMode;                        // 0x50
    int result;                          // 0x54
    void* playerInfo;                    // 0x58
    unsigned char playerInfoRequested;   // 0x5c
    char pad_05D[0x3];                   // 0x5d
    int previousScriptIndex;             // 0x60

    int UpdateScriptsDialogsAndInput(float dt);
    void Show();
    int HandleSelection();
    int PollLoadComplete();
    void Draw();
    int TryGetResult(int* out);
    void UpdateLayout();
    int HandleSelectionThunk24();
    void* DtorThunk24(int n);

    CTopSubMenu(int a, int b, int c, int d);
    ~CTopSubMenu();
};

#define CTOPSUBMENU_OFF(f, o) typedef char CTopSubMenu_off_##f[((int)&((CTopSubMenu*)0)->f == (o)) ? 1 : -1]
CTOPSUBMENU_OFF(interfaceVtable, 0x18);
CTOPSUBMENU_OFF(layout, 0x1c);
CTOPSUBMENU_OFF(bpObject, 0x20);
CTOPSUBMENU_OFF(resultReady, 0x28);
CTOPSUBMENU_OFF(selectionComplete, 0x29);
CTOPSUBMENU_OFF(menuItemCount, 0x2c);
CTOPSUBMENU_OFF(scriptRunning, 0x30);
CTOPSUBMENU_OFF(menuMessage, 0x34);
CTOPSUBMENU_OFF(optionDialog, 0x38);
CTOPSUBMENU_OFF(optionDialogRequested, 0x3c);
CTOPSUBMENU_OFF(menuMode, 0x50);
CTOPSUBMENU_OFF(result, 0x54);
CTOPSUBMENU_OFF(playerInfo, 0x58);
CTOPSUBMENU_OFF(playerInfoRequested, 0x5c);
CTOPSUBMENU_OFF(previousScriptIndex, 0x60);
#undef CTOPSUBMENU_OFF

#endif // CLASSES_CTOPSUBMENU_H
