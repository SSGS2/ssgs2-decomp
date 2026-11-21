// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDARTSDIALOG_H
#define CLASSES_CDARTSDIALOG_H

#include "game/types.h"

class CDartsDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* targetValue;                        // 0x4: 1 use in CDartsDialog_Ctor
    void* clearType;                        // 0x8: 1 use in CDartsDialog_Ctor
    void* layout;                        // 0xc: 5 uses in CDartsDialog_dtor, CDartsDialog_Ctor
    int bpEntryId;                         // 0x10: 5 uses in CDartsDialog_dtor, CDartsDialog_Ctor
    int bpObject;                         // 0x14: 2 uses in CDartsDialog_Ctor
    void* textTable;                       // 0x18: 5 uses in CDartsDialog_dtor, CDartsDialog_Ctor
    void* textFactory;                       // 0x1c: 4 uses in CDartsDialog_Ctor
    void* messageResource;                       // 0x20: 3 uses in CDartsDialog_dtor, CDartsDialog_Ctor
    union {
        void* textDraw0;                // 0x24: existing first-slot name retained
        void* textDraws[4];             // 0x24: CDartsDialog_SetTaggedTextAtLayoutElement rejects index >= 4 and updates this message slot
    };
    int state;                         // 0x34: 2 uses in CDartsDialog_Ctor
    int fadeState;                         // 0x38: 1 use in CDartsDialog_Ctor
    float fadeTime;                       // 0x3c: 1 use in CDartsDialog_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x10, 0x20, 0x24
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CDARTSDIALOG_H
