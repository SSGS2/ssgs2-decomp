// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CMULTIPLAYTOPMENU_H
#define CLASSES_CMULTIPLAYTOPMENU_H

#include "game/types.h"

class CMultiPlayTopMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;                       // 0x18: 1 use in CMultiPlayTopMenu_dtor
    void* layout;                       // 0x1c: 8 uses in CMultiPlayTopMenu_StartMenu, CMultiPlayTopMenu_Draw, CMultiPlayTopMenu_UpdateLayout
    void* bpObject;                       // 0x20: 5 uses in CMultiPlayTopMenu_StartMenu, CMultiPlayTopMenu_Draw
    int result;                        // 0x24: CMultiPlayTopMenu_HandleSelection maps selected menu index to result 3..7
    unsigned char resultReady;          // 0x28: CMultiPlayTopMenu_HandleSelection sets after selection
    union {
        char pad_029[0x7];              // 0x29: preserve the existing padding view
        unsigned char exitRequested;   // 0x29: CMultiPlayTopMenu_UpdateMenu (title_top_and_multiplay_menu.cpp) returns one when set
    };
    unsigned char scriptRunning;               // 0x30: 1 use in CMultiPlayTopMenu_StartMenu
    char pad_031[0x3];                  // 0x31
    void* menuMessage;                       // 0x34: 4 uses in CMultiPlayTopMenu_StartMenu, CMultiPlayTopMenu_Draw
    void* optionDialog;                       // 0x38: 1 use in CMultiPlayTopMenu_Draw
    union {
        char pad_03C[0x20];             // 0x3c: preserve the existing padding view
        unsigned char optionDialogRequested; // 0x3c: vfunc_5 creates/processes COptionDialog, then clears
    };
    void* menuTexture;                       // 0x5c: 3 uses in CMultiPlayTopMenu_dtor, CMultiPlayTopMenu_Draw

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CMULTIPLAYTOPMENU_OFF(f, o) typedef char CMultiPlayTopMenu_off_##f[((int)&((CMultiPlayTopMenu*)0)->f == (o)) ? 1 : -1]
CMULTIPLAYTOPMENU_OFF(secondaryVtable, 0x18);
CMULTIPLAYTOPMENU_OFF(layout, 0x1c);
CMULTIPLAYTOPMENU_OFF(bpObject, 0x20);
CMULTIPLAYTOPMENU_OFF(result, 0x24);
CMULTIPLAYTOPMENU_OFF(resultReady, 0x28);
CMULTIPLAYTOPMENU_OFF(exitRequested, 0x29);
CMULTIPLAYTOPMENU_OFF(scriptRunning, 0x30);
CMULTIPLAYTOPMENU_OFF(menuMessage, 0x34);
CMULTIPLAYTOPMENU_OFF(optionDialog, 0x38);
CMULTIPLAYTOPMENU_OFF(optionDialogRequested, 0x3c);
CMULTIPLAYTOPMENU_OFF(menuTexture, 0x5c);
#undef CMULTIPLAYTOPMENU_OFF
typedef char CMultiPlayTopMenu_size[(sizeof(CMultiPlayTopMenu) == 0x60) ? 1 : -1];

#endif  // CLASSES_CMULTIPLAYTOPMENU_H
