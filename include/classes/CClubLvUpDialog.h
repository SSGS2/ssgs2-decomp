// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCLUBLVUPDIALOG_H
#define CLASSES_CCLUBLVUPDIALOG_H

#include "game/types.h"

class CClubLvUpDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 5 uses in CClubLvUpDialog_dtor, CClubLvUpDialog_Ctor
    int bpObject;                          // 0x8: 2 uses in CClubLvUpDialog_Ctor
    int bpEntryId;                          // 0xc: 5 uses in CClubLvUpDialog_dtor, CClubLvUpDialog_Ctor
    int clubIcon;                         // 0x10: 1 use in CClubLvUpDialog_Ctor
    void* playerData;                       // 0x14: 1 use in CClubLvUpDialog_Ctor
    void* clubHouseMessage;                       // 0x18: 5 uses in CClubLvUpDialog_dtor, CClubLvUpDialog_Ctor
    void* textTable;                       // 0x1c: 5 uses in CClubLvUpDialog_dtor, CClubLvUpDialog_Ctor
    int selectionDelayFrames;                         // 0x20: 1 use in CClubLvUpDialog_Ctor
    int selectedElement;                         // 0x24: 1 use in CClubLvUpDialog_Ctor
    int result;                         // 0x28: 1 use in CClubLvUpDialog_Ctor
    int clubIndex;                         // 0x2c: 1 use in CClubLvUpDialog_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0xc
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCLUBLVUPDIALOG_OFF(f, o) typedef char CClubLvUpDialog_off_##f[((int)&((CClubLvUpDialog*)0)->f == (o)) ? 1 : -1]
CCLUBLVUPDIALOG_OFF(layout, 0x4);
CCLUBLVUPDIALOG_OFF(bpObject, 0x8);
CCLUBLVUPDIALOG_OFF(bpEntryId, 0xc);
CCLUBLVUPDIALOG_OFF(clubIcon, 0x10);
CCLUBLVUPDIALOG_OFF(playerData, 0x14);
CCLUBLVUPDIALOG_OFF(clubHouseMessage, 0x18);
CCLUBLVUPDIALOG_OFF(textTable, 0x1c);
CCLUBLVUPDIALOG_OFF(selectionDelayFrames, 0x20);
CCLUBLVUPDIALOG_OFF(selectedElement, 0x24);
CCLUBLVUPDIALOG_OFF(result, 0x28);
CCLUBLVUPDIALOG_OFF(clubIndex, 0x2c);
#undef CCLUBLVUPDIALOG_OFF

#endif  // CLASSES_CCLUBLVUPDIALOG_H
