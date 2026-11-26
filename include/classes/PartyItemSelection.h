// PartyItemSelection: explicit layout view; no inheritance, virtual methods, or RTTI.
// Selection container copied by CPartyItemSelect_CopyCompletedSelection; individual records initialized by PartyItemRecord_InitDefaults.
#ifndef CLASSES_PARTYITEMSELECTION_H
#define CLASSES_PARTYITEMSELECTION_H

#include "game/types.h"
#include "classes/PartyItemRecord.h"

class PartyItemSelection {
public:
    PartyItemRecord
        records[4]; // 0x0: CPartyItemSelect_InitializeSelectionLayout: four records constructed with stride 0xA4
    int count; // 0x290: CPartyItemSelect_CopyCompletedSelection: copied after four records; loops use as record count
};

#define OFF(f, o) typedef char PartyItemSelection_off_##f[((int)&((PartyItemSelection*)0)->f == (o)) ? 1 : -1]
OFF(records, 0x0);
OFF(count, 0x290);
#undef OFF

#endif
