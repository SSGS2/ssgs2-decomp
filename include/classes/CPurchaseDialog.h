// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPURCHASEDIALOG_H
#define CLASSES_CPURCHASEDIALOG_H

#include "game/types.h"

class CPurchaseDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int unk_4;                          // 0x4: 1 use in CPurchaseDialog_Ctor
    void* layout;                        // 0x8: 3 uses in CPurchaseDialog_dtor, CPurchaseDialog_Ctor
    int bpObject;                          // 0xc: 2 uses in CPurchaseDialog_Ctor
    int bpEntryId;                         // 0x10: 5 uses in CPurchaseDialog_dtor, CPurchaseDialog_Ctor
    int slot0Texture;                         // 0x14: 1 use in CPurchaseDialog_Ctor
    int slot1Texture;                         // 0x18: 1 use in CPurchaseDialog_Ctor
    int slot2Texture;                         // 0x1c: 1 use in CPurchaseDialog_Ctor
    int slot3Texture;                         // 0x20: 1 use in CPurchaseDialog_Ctor
    int slot4Texture;                         // 0x24: 1 use in CPurchaseDialog_Ctor
    int slot5Texture;                         // 0x28: 1 use in CPurchaseDialog_Ctor
    int slot6Texture;                         // 0x2c: 1 use in CPurchaseDialog_Ctor
    int slot7Texture;                         // 0x30: 1 use in CPurchaseDialog_Ctor
    int result;                         // 0x34: 1 use in CPurchaseDialog_Ctor
    void* textTable;                       // 0x38: 5 uses in CPurchaseDialog_dtor, CPurchaseDialog_Ctor
    int selectionDelayFrames;                         // 0x3c: 1 use in CPurchaseDialog_Ctor
    int selectedElement;                         // 0x40: 1 use in CPurchaseDialog_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPURCHASEDIALOG_OFF(f, o) typedef char CPurchaseDialog_off_##f[((int)&((CPurchaseDialog*)0)->f == (o)) ? 1 : -1]
CPURCHASEDIALOG_OFF(unk_4, 0x4);
CPURCHASEDIALOG_OFF(layout, 0x8);
CPURCHASEDIALOG_OFF(bpObject, 0xc);
CPURCHASEDIALOG_OFF(bpEntryId, 0x10);
CPURCHASEDIALOG_OFF(slot0Texture, 0x14);
CPURCHASEDIALOG_OFF(slot1Texture, 0x18);
CPURCHASEDIALOG_OFF(slot2Texture, 0x1c);
CPURCHASEDIALOG_OFF(slot3Texture, 0x20);
CPURCHASEDIALOG_OFF(slot4Texture, 0x24);
CPURCHASEDIALOG_OFF(slot5Texture, 0x28);
CPURCHASEDIALOG_OFF(slot6Texture, 0x2c);
CPURCHASEDIALOG_OFF(slot7Texture, 0x30);
CPURCHASEDIALOG_OFF(result, 0x34);
CPURCHASEDIALOG_OFF(textTable, 0x38);
CPURCHASEDIALOG_OFF(selectionDelayFrames, 0x3c);
CPURCHASEDIALOG_OFF(selectedElement, 0x40);
#undef CPURCHASEDIALOG_OFF

#endif  // CLASSES_CPURCHASEDIALOG_H
