// CPartyItemLuckyBracelet: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMLUCKYBRACELET_H
#define CLASSES_CPARTYITEMLUCKYBRACELET_H

class CPartyItemLuckyBracelet {
public:
    void* vtbl; // 0x0: CPartyItemLuckyBracelet_Ctor
    char pad_004[0x8];
    int ownerIndex; // 0xc: CPartyItemLuckyBracelet_ApplyOwnerModifiers, CPartyItemLuckyBracelet_ApplyModifiersIfActive
    int active; // 0x10: CPartyItemLuckyBracelet_Ctor, CPartyItemLuckyBracelet_ApplyModifiersIfActive
    float pangyaZoneWidthBonus; // 0x14: CPartyItemLuckyBracelet_Ctor, CPartyItemLuckyBracelet_ApplyOwnerModifiers, CPartyItemLuckyBracelet_ApplyModifiersIfActive
    float positiveShotOutcomeBonusRate; // 0x18: CPartyItemLuckyBracelet_Ctor, CPartyItemLuckyBracelet_ApplyOwnerModifiers, CPartyItemLuckyBracelet_ApplyModifiersIfActive
};

#define OFF(f, o) typedef char CPartyItemLuckyBracelet_off_##f[((int)&((CPartyItemLuckyBracelet*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(active, 0x10);
OFF(pangyaZoneWidthBonus, 0x14);
OFF(positiveShotOutcomeBonusRate, 0x18);
#undef OFF

#endif
