// PartyItemActivation: explicit layout view; no inheritance, virtual methods, or RTTI.
// Pairs reached through CPartyItemSelect::unk_2C in CPartyItemSelect_UpdateSelectionAndDetail; stride 8.
#ifndef CLASSES_PARTYITEMACTIVATION_H
#define CLASSES_PARTYITEMACTIVATION_H

#include "game/types.h"

class PartyItemActivation {
public:
    int elementId; // 0x0: CPartyItemSelect_UpdateSelectionAndDetail: switch on elementId - 8, stores choice + 8
    int feedbackFramesRemaining; // 0x4: CPartyItemSelect_UpdateSelectionAndDetail: tested for nonzero, assigned 4
};

#define OFF(f, o) typedef char PartyItemActivation_off_##f[((int)&((PartyItemActivation*)0)->f == (o)) ? 1 : -1]
OFF(elementId, 0x0);
OFF(feedbackFramesRemaining, 0x4);
#undef OFF

#endif
