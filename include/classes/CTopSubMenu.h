// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTOPSUBMENU_H
#define CLASSES_CTOPSUBMENU_H

#include "game/types.h"

class CTopSubMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* interfaceVtable;                       // 0x18: 2 uses in CTopSubMenu_dtor, CTopSubMenu_Ctor
    void* layout;                       // 0x1c: 8 uses in CTopSubMenu_Show, CTopSubMenu_UpdateScriptsDialogsAndInput, CTopSubMenu_PollLoadComplete...
    void* bpObject;                       // 0x20: 9 uses in CTopSubMenu_Show, CTopSubMenu_UpdateScriptsDialogsAndInput, CTopSubMenu_dtor...
    char pad_024[0x4];                  // 0x24
    unsigned char resultReady;          // 0x28: CTopSubMenu_HandleSelection sets; CTopSubMenu_TryGetResult gates result output
    unsigned char selectionComplete;               // 0x29: 1 use in CTopSubMenu_UpdateScriptsDialogsAndInput
    char pad_02A[0x2];                  // 0x2a
    int menuItemCount;                         // 0x2c: 9 uses in CTopSubMenu_Show, CTopSubMenu_UpdateScriptsDialogsAndInput
    unsigned char scriptRunning;               // 0x30: 4 uses in CTopSubMenu_UpdateScriptsDialogsAndInput, CTopSubMenu_Show
    char pad_031[0x3];                  // 0x31
    void* menuMessage;                       // 0x34: 10 uses in CTopSubMenu_Show, CTopSubMenu_UpdateScriptsDialogsAndInput, CTopSubMenu_Draw
    void* optionDialog;                       // 0x38: 6 uses in CTopSubMenu_UpdateScriptsDialogsAndInput, CTopSubMenu_Draw
    unsigned char optionDialogRequested;               // 0x3c: 2 uses in CTopSubMenu_UpdateScriptsDialogsAndInput
    char pad_03D[0x13];                 // 0x3d
    int menuMode;                         // 0x50: 5 uses in CTopSubMenu_UpdateScriptsDialogsAndInput, CTopSubMenu_Show, CTopSubMenu_Ctor
    int result;                         // 0x54: 1 use in CTopSubMenu_Ctor
    void* playerInfo;                       // 0x58: 7 uses in CTopSubMenu_UpdateScriptsDialogsAndInput, CTopSubMenu_Draw, CTopSubMenu_Ctor
    unsigned char playerInfoRequested;               // 0x5c: 3 uses in CTopSubMenu_UpdateScriptsDialogsAndInput, CTopSubMenu_Ctor
    char pad_05D[0x3];                  // 0x5d
    int previousScriptIndex;                         // 0x60: 3 uses in CTopSubMenu_Show, CTopSubMenu_dtor, CTopSubMenu_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CTOPSUBMENU_H
