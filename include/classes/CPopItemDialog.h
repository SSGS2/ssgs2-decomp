// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPOPITEMDIALOG_H
#define CLASSES_CPOPITEMDIALOG_H

#include "game/types.h"

class CPopItemDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    unsigned int itemId;                 // 0x4: 7 uses in CPopItemDialog_LayoutItemText, CPopItemDialog_UpdateUntilDismissed, CPopItemDialog_DrawDialogTextAndIcon
    void* bpEntryId;                        // 0x8: 2 uses in CPopItemDialog_DrawDialogTextAndIcon, CPopItemDialog_LayoutItemText
    void* state;                        // 0xc: 1 use in CPopItemDialog_DrawDialogTextAndIcon
    int descriptionMode;                         // 0x10: 3 uses in CPopItemDialog_LayoutItemText
    unsigned char isItemSet;               // 0x14: 6 uses in CPopItemDialog_UpdateUntilDismissed, CPopItemDialog_LoadItemIconAndLayout, CPopItemDialog_LayoutItemText
    char pad_015[0x3];                  // 0x15
    void* textRenderer;                       // 0x18: 17 uses in CPopItemDialog_LayoutItemText, CPopItemDialog_DrawDialogTextAndIcon
    void* taggedText;                       // 0x1c: 11 uses in CPopItemDialog_LayoutItemText
    void* texturePane;                       // 0x20: 5 uses in CPopItemDialog_LoadItemIconAndLayout, CPopItemDialog_AreResourcesLoaded, CPopItemDialog_DrawDialogTextAndIcon
    void* sharedFile;                       // 0x24: 4 uses in CPopItemDialog_LoadItemIconAndLayout, CPopItemDialog_AreResourcesLoaded
    void* texturePack;                       // 0x28: 2 uses in CPopItemDialog_AreResourcesLoaded, CPopItemDialog_LoadItemIconAndLayout

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8, 0x1c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPOPITEMDIALOG_OFF(f, o) typedef char CPopItemDialog_off_##f[((int)&((CPopItemDialog*)0)->f == (o)) ? 1 : -1]
CPOPITEMDIALOG_OFF(itemId, 0x4);
CPOPITEMDIALOG_OFF(bpEntryId, 0x8);
CPOPITEMDIALOG_OFF(state, 0xc);
CPOPITEMDIALOG_OFF(descriptionMode, 0x10);
CPOPITEMDIALOG_OFF(isItemSet, 0x14);
CPOPITEMDIALOG_OFF(textRenderer, 0x18);
CPOPITEMDIALOG_OFF(taggedText, 0x1c);
CPOPITEMDIALOG_OFF(texturePane, 0x20);
CPOPITEMDIALOG_OFF(sharedFile, 0x24);
CPOPITEMDIALOG_OFF(texturePack, 0x28);
#undef CPOPITEMDIALOG_OFF

#endif  // CLASSES_CPOPITEMDIALOG_H
