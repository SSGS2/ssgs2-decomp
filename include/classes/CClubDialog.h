#ifndef CLASSES_CCLUBDIALOG_H
#define CLASSES_CCLUBDIALOG_H

#include "game/types.h"

class CClubDialog {
public:
    void* vtable;             // 0x0
    void* layout;             // 0x4
    int mode;                 // 0x8
    int result;               // 0xc
    void* message;            // 0x10
    int selectionDelayFrames; // 0x14
    int selectedElement;      // 0x18

    CClubDialog* Init();
    int ProcessInput();
    int SetMode(int mode);
    int UpdateLayout();
    int Draw();
    int Update();
    int TryGetResult(void* resultOut);
    static int UpdateSelectionDelay(void* object);
    static int InitLayout(void* object);

    ~CClubDialog();
};

#define CCLUBDIALOG_OFF(f, o) typedef char CClubDialog_off_##f[((int)&((CClubDialog*)0)->f == (o)) ? 1 : -1]
CCLUBDIALOG_OFF(layout, 0x4);
CCLUBDIALOG_OFF(mode, 0x8);
CCLUBDIALOG_OFF(result, 0xc);
CCLUBDIALOG_OFF(message, 0x10);
CCLUBDIALOG_OFF(selectionDelayFrames, 0x14);
CCLUBDIALOG_OFF(selectedElement, 0x18);
#undef CCLUBDIALOG_OFF

#endif // CLASSES_CCLUBDIALOG_H
