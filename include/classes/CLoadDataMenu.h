// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CLOADDATAMENU_H
#define CLASSES_CLOADDATAMENU_H

#include "game/types.h"

class CLoadDataMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* layout;                       // 0x18: 30 uses in CLoadDataMenu_ProcessInput, CLoadDataMenu_Ctor, CLoadDataMenu_dtor...
    void* bpObject;                       // 0x1c: 5 uses in CLoadDataMenu_Ctor, CLoadDataMenu_Draw, CLoadDataMenu_EnterMenu
    int bpEntryId;                         // 0x20: 5 uses in CLoadDataMenu_dtor, CLoadDataMenu_Ctor
    int selectionDelayFrames;                         // 0x24: 7 uses in CLoadDataMenu_ProcessInput, CLoadDataMenu_Ctor
    unsigned char finished;               // 0x28: 6 uses in CLoadDataMenu_ProcessInput, CLoadDataMenu_Ctor, CLoadDataMenu_StartFadeOrRequestTransition
    char pad_029[0x3];                  // 0x29
    int textFactory;                         // 0x2c: 2 uses in CLoadDataMenu_Ctor
    void* messageDraw;                       // 0x30: 5 uses in CLoadDataMenu_dtor, CLoadDataMenu_Ctor, CLoadDataMenu_Draw
    void* textTable;                       // 0x34: 7 uses in CLoadDataMenu_dtor, CLoadDataMenu_Ctor, CLoadDataMenu_EnterMenu...
    unsigned short slotUsed;              // 0x38: 1 use in CLoadDataMenu_ProcessInput
    char pad_03A[0x209e];               // 0x3a
    int selectedSlot;                       // 0x20d8: 5 uses in CLoadDataMenu_ProcessInput, CLoadDataMenu_Ctor
    int result;                       // 0x20dc: 4 uses in CLoadDataMenu_ProcessInput, CLoadDataMenu_Ctor
    void* packTextureResource;                     // 0x20e0: 9 uses in CLoadDataMenu_Ctor, CLoadDataMenu_dtor, CLoadDataMenu_PollLoadComplete...
    void* bpTexture;                     // 0x20e4: 8 uses in CLoadDataMenu_Draw, CLoadDataMenu_Ctor, CLoadDataMenu_dtor...
    int savedScriptIndex;                       // 0x20e8: 3 uses in CLoadDataMenu_Ctor, CLoadDataMenu_dtor, CLoadDataMenu_EnterMenu
    int secondaryBpEntryId;                       // 0x20ec: 6 uses in CLoadDataMenu_dtor, CLoadDataMenu_Draw, CLoadDataMenu_Ctor
    void* deleteConfirmationDialog;                     // 0x20f0: 10 uses in CLoadDataMenu_ProcessInput, CLoadDataMenu_Ctor, CLoadDataMenu_Draw

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x20, 0x30, 0x38, 0x20d8, 0x20ec
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CLOADDATAMENU_H
