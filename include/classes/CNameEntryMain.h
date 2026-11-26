#ifndef CLASSES_CNAMEENTRYMAIN_H
#define CLASSES_CNAMEENTRYMAIN_H

#include "game/types.h"

class CNameEntryMain {
public:
    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x1C]; // 0x4
    void* keyboard;     // 0x20
    char pendingPlayerRecord
        [0x828]; // 0x24: NameEntry_CopyNonBlankTextToPendingRecord copies wide text, CNameEntryMain_UpdateStateMachine submits it (name_entry_soft_keyboard.cpp); view extends to next known field
    void* dialog;        // 0x84c
    void* state;         // 0x850
    void* packedTexture; // 0x854

    int IsKeyboardReady();
    void StartMenu();
    void UpdateWhileWaiting();
    void Draw();
    int UpdateStateMachine();

    CNameEntryMain();
    ~CNameEntryMain();
};

#define CNAMEENTRYMAIN_OFF(f, o) typedef char CNameEntryMain_off_##f[((int)&((CNameEntryMain*)0)->f == (o)) ? 1 : -1]
CNAMEENTRYMAIN_OFF(keyboard, 0x20);
CNAMEENTRYMAIN_OFF(dialog, 0x84c);
CNAMEENTRYMAIN_OFF(pendingPlayerRecord, 0x24);
CNAMEENTRYMAIN_OFF(state, 0x850);
CNAMEENTRYMAIN_OFF(packedTexture, 0x854);
#undef CNAMEENTRYMAIN_OFF

#endif // CLASSES_CNAMEENTRYMAIN_H
