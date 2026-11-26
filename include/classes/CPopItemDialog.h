#ifndef CLASSES_CPOPITEMDIALOG_H
#define CLASSES_CPOPITEMDIALOG_H

#include "game/types.h"

class CPopItemDialog {
public:
    void* vtbl;              // 0x0: original vtable pointer
    unsigned int itemId;     // 0x4
    void* bpEntryId;         // 0x8
    void* state;             // 0xc
    int descriptionMode;     // 0x10
    unsigned char isItemSet; // 0x14
    char pad_015[0x3];       // 0x15
    void* textRenderer;      // 0x18
    void* taggedText;        // 0x1c
    void* texturePane;       // 0x20
    void* sharedFile;        // 0x24
    void* texturePack;       // 0x28

    int UpdateUntilDismissed();
    int AreResourcesLoaded();
    void LoadItemIconAndLayout();
    void LayoutItemText();
    void DrawDialogTextAndIcon() const;

    ~CPopItemDialog();
    CPopItemDialog(void* arg1, void* arg2);
};

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

#endif // CLASSES_CPOPITEMDIALOG_H
