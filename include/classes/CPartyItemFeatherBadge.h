// CPartyItemFeatherBadge: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMFEATHERBADGE_H
#define CLASSES_CPARTYITEMFEATHERBADGE_H

class CPartyItemFeatherBadge {
public:
    CPartyItemFeatherBadge(int size);

    void* vtbl; // 0x0
    char pad_004[0x8];
    int ownerIndex; // 0xc: CPartyItemFeatherBadge_ApplyModifiersToPlayer, CPartyItemFeatherBadge_ApplyModifiersIfActive
    int active;     // 0x10: PartyItem_ClearActivationWord, CPartyItemFeatherBadge_ApplyModifiersIfActive
    int spinRangeOverrideEnabled; // 0x14: CPartyItemFeatherBadge_ApplyModifiersToPlayer
    float
        spinRangeY; // 0x18: CPartyItemFeatherBadge_ApplyModifiersToPlayer, CPartyItemFeatherBadge_ApplyModifiersIfActive
    float
        spinRangeX; // 0x1c: CPartyItemFeatherBadge_ApplyModifiersToPlayer, CPartyItemFeatherBadge_ApplyModifiersIfActive
    float
        launchVerticalVelocityBonus; // 0x20: CPartyItemFeatherBadge_ApplyModifiersToPlayer, CPartyItemFeatherBadge_ApplyModifiersIfActive

    void ApplyModifiersToPlayer(void* out, int index);
    int ApplyModifiersIfActive(int, void* out, int index);
};

#define OFF(f, o) typedef char CPartyItemFeatherBadge_off_##f[((int)&((CPartyItemFeatherBadge*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(active, 0x10);
OFF(spinRangeOverrideEnabled, 0x14);
OFF(spinRangeY, 0x18);
OFF(spinRangeX, 0x1c);
OFF(launchVerticalVelocityBonus, 0x20);
#undef OFF

#endif
