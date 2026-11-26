#ifndef CLASSES_CDARTSDIALOG_H
#define CLASSES_CDARTSDIALOG_H

#include "game/types.h"

class CDartsDialog {
public:
    void* vtbl;            // 0x0: original vtable pointer
    void* targetValue;     // 0x4
    void* clearType;       // 0x8
    void* layout;          // 0xc
    int bpEntryId;         // 0x10
    int bpObject;          // 0x14
    void* textTable;       // 0x18
    void* textFactory;     // 0x1c
    void* messageResource; // 0x20
    union {
        void* textDraw0; // 0x24: existing first-slot name retained
        void* textDraws
            [4]; // 0x24: CDartsDialog_SetTaggedTextAtLayoutElement rejects index >= 4 and updates this message slot
    };
    int state;      // 0x34
    int fadeState;  // 0x38
    float fadeTime; // 0x3c

    void Draw();
    int Update(float delta);
    int UpdateFadeSequence(float delta);
    void SetTaggedTextAtLayoutElement(int textSlot, const char* messageKey, const char* alignmentTag,
                                      int layoutElement);

    CDartsDialog(void* targetValue, void* clearType);
    void* Destroy(int deleting);
};

#define CDARTSDIALOG_OFF(f, o) typedef char CDartsDialog_off_##f[((int)&((CDartsDialog*)0)->f == (o)) ? 1 : -1]
CDARTSDIALOG_OFF(targetValue, 0x4);
CDARTSDIALOG_OFF(clearType, 0x8);
CDARTSDIALOG_OFF(layout, 0xc);
CDARTSDIALOG_OFF(bpEntryId, 0x10);
CDARTSDIALOG_OFF(bpObject, 0x14);
CDARTSDIALOG_OFF(textTable, 0x18);
CDARTSDIALOG_OFF(textFactory, 0x1c);
CDARTSDIALOG_OFF(messageResource, 0x20);
CDARTSDIALOG_OFF(textDraw0, 0x24);
CDARTSDIALOG_OFF(textDraws, 0x24);
CDARTSDIALOG_OFF(state, 0x34);
CDARTSDIALOG_OFF(fadeState, 0x38);
CDARTSDIALOG_OFF(fadeTime, 0x3c);
#undef CDARTSDIALOG_OFF

#endif // CLASSES_CDARTSDIALOG_H
