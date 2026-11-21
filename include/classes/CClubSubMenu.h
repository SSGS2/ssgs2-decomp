// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCLUBSUBMENU_H
#define CLASSES_CCLUBSUBMENU_H

#include "game/types.h"

class CClubSubMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                        // 0x4: 1 use in CClubSubMenu_Ctor
    int selectedElementIndex;                          // 0x8: 1 use in CClubSubMenu_Ctor
    int menuVariant;                          // 0xc: 1 use in CClubSubMenu_Ctor
    int selectionDelayFrames;                         // 0x10: 1 use in CClubSubMenu_Ctor
    int selectionResult;                         // 0x14: 1 use in CClubSubMenu_Ctor
    unsigned char firstChoiceMarked;               // 0x18: 1 use in CClubSubMenu_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCLUBSUBMENU_OFF(f, o) typedef char CClubSubMenu_off_##f[((int)&((CClubSubMenu*)0)->f == (o)) ? 1 : -1]
CCLUBSUBMENU_OFF(layout, 0x4);
CCLUBSUBMENU_OFF(selectedElementIndex, 0x8);
CCLUBSUBMENU_OFF(menuVariant, 0xc);
CCLUBSUBMENU_OFF(selectionDelayFrames, 0x10);
CCLUBSUBMENU_OFF(selectionResult, 0x14);
CCLUBSUBMENU_OFF(firstChoiceMarked, 0x18);
#undef CCLUBSUBMENU_OFF

#endif  // CLASSES_CCLUBSUBMENU_H
