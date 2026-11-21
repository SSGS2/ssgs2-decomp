// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPARTYITEMFIREBANTAGE_H
#define CLASSES_CPARTYITEMFIREBANTAGE_H

#include "game/types.h"

class CPartyItemFireBantage {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x8]; // 0x4
    int ownerIndex; // 0xc: CPartyItemFireBantage_ApplyOwnerModifiers/4: index comparison
    int active;                         // 0x10: 1 use in CPartyItemFireBantage_Ctor
    float clubDistanceBonusRate;                       // 0x14: 1 use in CPartyItemFireBantage_Ctor
    float impactZoneWidthBonus;                       // 0x18: 1 use in CPartyItemFireBantage_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPARTYITEMFIREBANTAGE_OFF(f, o) typedef char CPartyItemFireBantage_off_##f[((int)&((CPartyItemFireBantage*)0)->f == (o)) ? 1 : -1]
CPARTYITEMFIREBANTAGE_OFF(active, 0x10);
CPARTYITEMFIREBANTAGE_OFF(clubDistanceBonusRate, 0x14);
CPARTYITEMFIREBANTAGE_OFF(impactZoneWidthBonus, 0x18);
CPARTYITEMFIREBANTAGE_OFF(ownerIndex, 0xc);
#undef CPARTYITEMFIREBANTAGE_OFF

#endif  // CLASSES_CPARTYITEMFIREBANTAGE_H
