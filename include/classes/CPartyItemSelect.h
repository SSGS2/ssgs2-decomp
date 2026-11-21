// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPARTYITEMSELECT_H
#define CLASSES_CPARTYITEMSELECT_H

#include "game/types.h"

class CPartyItemSelect {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int selectionComplete;                         // 0x18: 2 uses in CPartyItemSelect_UpdateSelectionAndDetail
    void* selectionLayout;                       // 0x1c: 58 uses in CPartyItemSelect_UpdateSelectionAndDetail, CPartyItemSelect_PollLoadComplete
    void* playerSelections;                       // 0x20: 10 uses in CPartyItemSelect_UpdateSelectionAndDetail
    int selectionBpEntryId; // 0x24: CPartyItemSelect_InitializeSelectionLayout
    void* selectionBpObject; // 0x28: CPartyItemSelect_Draw: BpObject_Draw
    void* playerActivations;                       // 0x2c: 6 uses in CPartyItemSelect_UpdateSelectionAndDetail
    void* selectedItemDisplays;                       // 0x30: 1 use in CPartyItemSelect_UpdateSelectionAndDetail
    int playerCountBpEntryId; // 0x34: CPartyItemSelect_InitializeSelectionLayout
    void* playerCountBpObject; // 0x38: CPartyItemSelect_Draw: BpObject_Draw
    char pad_03C[4]; // 0x3C
    int detailRequested;                         // 0x40: 4 uses in CPartyItemSelect_UpdateSelectionAndDetail
    int detailActive;                         // 0x44: 4 uses in CPartyItemSelect_UpdateSelectionAndDetail
    int detailOpenStarted;                         // 0x48: 3 uses in CPartyItemSelect_UpdateSelectionAndDetail
    int detailCloseStarted;                         // 0x4c: 3 uses in CPartyItemSelect_UpdateSelectionAndDetail
    int detailCloseRequested;                         // 0x50: 4 uses in CPartyItemSelect_UpdateSelectionAndDetail
    void* detailBpObject;                       // 0x54: 6 uses in CPartyItemSelect_UpdateSelectionAndDetail
    int detailBpEntryId; // 0x58: controller entry handle
    void* detailLayout;                       // 0x5c: 5 uses in CPartyItemSelect_UpdateSelectionAndDetail, CPartyItemSelect_PollLoadComplete
    int detailItemIndex;                         // 0x60: 1 use in CPartyItemSelect_UpdateSelectionAndDetail
    int unk_64; // 0x64: controller entry handle
    void* detailItemDisplay;                       // 0x68: 1 use in CPartyItemSelect_UpdateSelectionAndDetail
    void* textRenderer; // 0x6C: CPartyItemSelect_RefreshDetailText: Text_SetContent/Text_SetPosition
    void* textTemplates; // 0x70: CPartyItemSelect_RefreshDetailText: Text_ExpandTaggedText
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPARTYITEMSELECT_OFF(f, o) typedef char CPartyItemSelect_off_##f[((int)&((CPartyItemSelect*)0)->f == (o)) ? 1 : -1]
CPARTYITEMSELECT_OFF(selectionComplete, 0x18);
CPARTYITEMSELECT_OFF(selectionLayout, 0x1c);
CPARTYITEMSELECT_OFF(playerSelections, 0x20);
CPARTYITEMSELECT_OFF(playerActivations, 0x2c);
CPARTYITEMSELECT_OFF(selectedItemDisplays, 0x30);
CPARTYITEMSELECT_OFF(detailRequested, 0x40);
CPARTYITEMSELECT_OFF(detailActive, 0x44);
CPARTYITEMSELECT_OFF(detailOpenStarted, 0x48);
CPARTYITEMSELECT_OFF(detailCloseStarted, 0x4c);
CPARTYITEMSELECT_OFF(detailCloseRequested, 0x50);
CPARTYITEMSELECT_OFF(detailBpObject, 0x54);
CPARTYITEMSELECT_OFF(detailLayout, 0x5c);
CPARTYITEMSELECT_OFF(detailItemIndex, 0x60);
CPARTYITEMSELECT_OFF(detailItemDisplay, 0x68);
CPARTYITEMSELECT_OFF(selectionBpEntryId, 0x24);
CPARTYITEMSELECT_OFF(selectionBpObject, 0x28);
CPARTYITEMSELECT_OFF(playerCountBpEntryId, 0x34);
CPARTYITEMSELECT_OFF(playerCountBpObject, 0x38);
CPARTYITEMSELECT_OFF(detailBpEntryId, 0x58);
CPARTYITEMSELECT_OFF(unk_64, 0x64);
CPARTYITEMSELECT_OFF(textRenderer, 0x6C);
CPARTYITEMSELECT_OFF(textTemplates, 0x70);
#undef CPARTYITEMSELECT_OFF

#endif  // CLASSES_CPARTYITEMSELECT_H
