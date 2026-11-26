#ifndef CLASSES_CPARTYITEMFIREBANTAGE_H
#define CLASSES_CPARTYITEMFIREBANTAGE_H

#include "game/types.h"

class CPartyItemFireBantage {
public:
    void* vtbl;                  // 0x0: original vtable pointer
    char pad_004[0x8];           // 0x4
    int ownerIndex;              // 0xc: CPartyItemFireBantage_ApplyOwnerModifiers/4: index comparison
    int active;                  // 0x10
    float clubDistanceBonusRate; // 0x14
    float impactZoneWidthBonus;  // 0x18

    void ApplyOwnerModifiers(void* modifiers, int playerIndex);
    int ApplyModifiersIfActive(void* /*unused*/, void* modifiers, int playerIndex);

    CPartyItemFireBantage(void* arg);
};

#define CPARTYITEMFIREBANTAGE_OFF(f, o) \
    typedef char CPartyItemFireBantage_off_##f[((int)&((CPartyItemFireBantage*)0)->f == (o)) ? 1 : -1]
CPARTYITEMFIREBANTAGE_OFF(active, 0x10);
CPARTYITEMFIREBANTAGE_OFF(clubDistanceBonusRate, 0x14);
CPARTYITEMFIREBANTAGE_OFF(impactZoneWidthBonus, 0x18);
CPARTYITEMFIREBANTAGE_OFF(ownerIndex, 0xc);
#undef CPARTYITEMFIREBANTAGE_OFF

#endif // CLASSES_CPARTYITEMFIREBANTAGE_H
