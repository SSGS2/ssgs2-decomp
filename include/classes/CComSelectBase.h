// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMSELECTBASE_H
#define CLASSES_CCOMSELECTBASE_H

#include "game/types.h"

class CComSelectBase {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int bpEntryId;                         // 0x18: 7 uses in CComSelectBase_SetPage, CComSelectBase_StartMenuScripts, CComSelectBase_Draw...
    int selectedEntry;                         // 0x1c: 6 uses in CComSelectBase_Update, CComSelectBase_DrawPrimaryRowSprites, CComSelectBase_DrawSecondarySprites...
    int pageIndex;                         // 0x20: 9 uses in CComSelectBase_SetPage, CComSelectBase_DrawEntryLabel, CComSelectBase_DrawPrimaryRowSprites...
    char pad_024[0x4];                  // 0x24
    float inputDelay;                       // 0x28: 7 uses in CComSelectBase_Update
    int requestedPageIndex;                         // 0x2c: 6 uses in CComSelectBase_Update
    void* textureResource;                       // 0x30: 2 uses in CComSelectBase_StartMenuScripts
    void* layout;                       // 0x34: 20 uses in CComSelectBase_Update, CComSelectBase_SetPage, CComSelectBase_DrawPrimaryRowSprites...
    void* textRenderer;                       // 0x38: 30 uses in CComSelectBase_DrawEntryText, CComSelectBase_DrawEntryLabel, CComSelectBase_DrawHoveredEntryText
    void* optionDialog;                       // 0x3c: 6 uses in CComSelectBase_Update, CComSelectBase_Draw
    unsigned char optionDialogPending;               // 0x40: 3 uses in CComSelectBase_Update
    char pad_041[0x3];                  // 0x41
    void* courseClearDialog;                       // 0x44: 1 use in CComSelectBase_Draw
    unsigned char courseClearNotificationPending;               // 0x48: 1 use in CComSelectBase_StartMenuScripts
    char pad_049[0x3];                  // 0x49
    int secondaryBpEntryId;                         // 0x4c: 3 uses in CComSelectBase_StartMenuScripts, CComSelectBase_Draw, CComSelectBase_Update
    int tertiaryBpEntryId;                         // 0x50: 3 uses in CComSelectBase_StartMenuScripts, CComSelectBase_Draw, CComSelectBase_Update
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CCOMSELECTBASE_H
