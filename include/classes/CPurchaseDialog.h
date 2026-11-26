#ifndef CLASSES_CPURCHASEDIALOG_H
#define CLASSES_CPURCHASEDIALOG_H

#include "game/types.h"

class CPurchaseDialog {
public:
    void* vtbl;               // 0x0: original vtable pointer
    int unk_4;                // 0x4
    void* layout;             // 0x8
    int bpObject;             // 0xc
    int bpEntryId;            // 0x10
    int slot0Texture;         // 0x14
    int slot1Texture;         // 0x18
    int slot2Texture;         // 0x1c
    int slot3Texture;         // 0x20
    int slot4Texture;         // 0x24
    int slot5Texture;         // 0x28
    int slot6Texture;         // 0x2c
    int slot7Texture;         // 0x30
    int result;               // 0x34
    void* textTable;          // 0x38
    int selectionDelayFrames; // 0x3c
    int selectedElement;      // 0x40

    int SetSlotDrawable(int idx, int val);
    int UpdateLayoutAndScript();
    int UpdateSelectionDelay();
    static int Update(void* p);
    int Draw();
    int SetPriceBreakdown(int balance, int price);
    int DrawSlotQuads();
    int TryCloseWithResult(void* out);
    int Show();
    int InitLayout();
    int ProcessInput();
    void* Init();

    ~CPurchaseDialog();
};

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

#endif // CLASSES_CPURCHASEDIALOG_H
