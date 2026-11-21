// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCLUBDIALOG_H
#define CLASSES_CCLUBDIALOG_H

#include "game/types.h"

class CClubDialog {
public:
    void* vtable;                        // 0x0: 2 uses in CClubDialog_dtor, CClubDialog_Ctor
    void* layout;                        // 0x4: 4 uses in CClubDialog_dtor, CClubDialog_Ctor
    int mode;                          // 0x8: 1 use in CClubDialog_Ctor
    int result;                          // 0xc: 1 use in CClubDialog_Ctor
    void* message;                       // 0x10: 3 uses in CClubDialog_dtor, CClubDialog_Ctor
    int selectionDelayFrames;                         // 0x14: 1 use in CClubDialog_Ctor
    int selectedElement;                         // 0x18: 1 use in CClubDialog_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCLUBDIALOG_OFF(f, o) typedef char CClubDialog_off_##f[((int)&((CClubDialog*)0)->f == (o)) ? 1 : -1]
CCLUBDIALOG_OFF(layout, 0x4);
CCLUBDIALOG_OFF(mode, 0x8);
CCLUBDIALOG_OFF(result, 0xc);
CCLUBDIALOG_OFF(message, 0x10);
CCLUBDIALOG_OFF(selectionDelayFrames, 0x14);
CCLUBDIALOG_OFF(selectedElement, 0x18);
#undef CCLUBDIALOG_OFF

#endif  // CLASSES_CCLUBDIALOG_H
