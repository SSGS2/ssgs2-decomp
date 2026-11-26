// CPartyItemLuckyBracelet: the party item that widens the Pangya zone and
// boosts the gain of a positive shot outcome.
#ifndef CLASSES_CPARTYITEMLUCKYBRACELET_H
#define CLASSES_CPARTYITEMLUCKYBRACELET_H

class CPartyItemLuckyBracelet {
public:
    CPartyItemLuckyBracelet(void* arg);
    ~CPartyItemLuckyBracelet();

    void ApplyOwnerModifiers(void* modifiers, int playerIndex);
    int ApplyModifiersIfActive(void* /*unused*/, void* modifiers, int playerIndex);

    void* vtbl;                         // 0x00
    char pad_004[0x8];                  // 0x04
    int ownerIndex;                     // 0x0C
    int active;                         // 0x10
    float pangyaZoneWidthBonus;         // 0x14
    float positiveShotOutcomeBonusRate; // 0x18
};

#define CPARTYITEMLUCKYBRACELET_OFF(f, o) \
    typedef char CPartyItemLuckyBracelet_off_##f[((int)&((CPartyItemLuckyBracelet*)0)->f == (o)) ? 1 : -1]
CPARTYITEMLUCKYBRACELET_OFF(vtbl, 0x00);
CPARTYITEMLUCKYBRACELET_OFF(ownerIndex, 0x0C);
CPARTYITEMLUCKYBRACELET_OFF(active, 0x10);
CPARTYITEMLUCKYBRACELET_OFF(pangyaZoneWidthBonus, 0x14);
CPARTYITEMLUCKYBRACELET_OFF(positiveShotOutcomeBonusRate, 0x18);
#undef CPARTYITEMLUCKYBRACELET_OFF
typedef char CPartyItemLuckyBracelet_size[(sizeof(CPartyItemLuckyBracelet) == 0x1C) ? 1 : -1];

#endif
