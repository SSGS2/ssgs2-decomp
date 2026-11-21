// CPartyItemHurricaneBadge: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMHURRICANEBADGE_H
#define CLASSES_CPARTYITEMHURRICANEBADGE_H

class CPartyItemHurricaneBadge {
public:
    void* vtbl; // 0x0: CPartyItemHurricaneBadge_Ctor
    char pad_004[0x8];
    int ownerIndex; // 0xc: CPartyItemHurricaneBadge_ApplyWindOverrides
    int unk_10; // 0x10: CPartyItemHurricaneBadge_Ctor
    float minimum; // 0x14: CPartyItemHurricaneBadge_Ctor, CPartyItemHurricaneBadge_ApplyWindOverrides
    float maximum; // 0x18: CPartyItemHurricaneBadge_Ctor, CPartyItemHurricaneBadge_ApplyWindOverrides
    int windSpeedOverrideEnabled; // 0x1c: CPartyItemHurricaneBadge_Ctor, CPartyItemHurricaneBadge_ApplyWindOverrides
    int oneMeterWindEnabled; // 0x20: CPartyItemHurricaneBadge_Ctor, CPartyItemHurricaneBadge_ApplyWindOverrides
};

#define OFF(f, o) typedef char CPartyItemHurricaneBadge_off_##f[((int)&((CPartyItemHurricaneBadge*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(unk_10, 0x10);
OFF(minimum, 0x14);
OFF(maximum, 0x18);
OFF(windSpeedOverrideEnabled, 0x1c);
OFF(oneMeterWindEnabled, 0x20);
#undef OFF

#endif
