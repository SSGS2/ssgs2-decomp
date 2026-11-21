// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_MENUGROUP802FB4D0_H
#define CLASSES_MENUGROUP802FB4D0_H

#include "game/types.h"

class MenuGroup802FB4D0 {
public:
    int selectedArea;                          // 0x0: TourMenuGroup_DrawActiveEntries: count bounds BP drawing loop; TourAreaSelect_RefreshLevelAndConnections: secondLayout
    char pad_004[0x4];                  // 0x4
    int bpEntryId;                          // 0x8: TourMenuGroup_DrawActiveEntries: count bounds BP drawing loop; TourAreaSelect_RefreshLevelAndConnections: secondLayout
    char pad_00C[0x4];                  // 0xc
    int entryCount;                     // 0x10: TourMenuGroup_DrawActiveEntries: count bounds BP drawing loop; TourAreaSelect_RefreshLevelAndConnections: secondLayout
    int state;                          // 0x14: TourMenuGroup_DrawActiveEntries: count bounds BP drawing loop; TourAreaSelect_RefreshLevelAndConnections: secondLayout
    void* firstLayout;                  // 0x18: TourMenuGroup_DrawActiveEntries: count bounds BP drawing loop; TourAreaSelect_RefreshLevelAndConnections: secondLayout
    void* secondLayout;                 // 0x1c: TourMenuGroup_DrawActiveEntries: count bounds BP drawing loop; TourAreaSelect_RefreshLevelAndConnections: secondLayout
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define MENUGROUP802FB4D0_OFF(f, o) typedef char MenuGroup802FB4D0_off_##f[((int)&((MenuGroup802FB4D0*)0)->f == (o)) ? 1 : -1]
MENUGROUP802FB4D0_OFF(bpEntryId, 0x8);
MENUGROUP802FB4D0_OFF(entryCount, 0x10);
MENUGROUP802FB4D0_OFF(state, 0x14);
MENUGROUP802FB4D0_OFF(firstLayout, 0x18);
MENUGROUP802FB4D0_OFF(secondLayout, 0x1c);
MENUGROUP802FB4D0_OFF(selectedArea, 0x0);
#undef MENUGROUP802FB4D0_OFF

#endif  // CLASSES_MENUGROUP802FB4D0_H
