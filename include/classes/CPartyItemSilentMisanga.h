// CPartyItemSilentMisanga: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMSILENTMISANGA_H
#define CLASSES_CPARTYITEMSILENTMISANGA_H

class CPartyItemSilentMisanga {
public:
    void* vtbl; // 0x0: CPartyItemSilentMisanga_Ctor
    char pad_004[0x8];
    int ownerIndex; // 0xc: party_CPartyItemSilentMisanga_vfunc_3, party_CPartyItemSilentMisanga_vfunc_4
    int active; // 0x10: CPartyItemSilentMisanga_Ctor, party_CPartyItemSilentMisanga_vfunc_4
    int oneMeterWindEnabled; // 0x14: CPartyItemSilentMisanga_Ctor, party_CPartyItemSilentMisanga_vfunc_3
    float pangyaZoneWidthBonus; // 0x18: CPartyItemSilentMisanga_Ctor, party_CPartyItemSilentMisanga_vfunc_3, party_CPartyItemSilentMisanga_vfunc_4
    float gaugeSpeedMultiplier; // 0x1c: CPartyItemSilentMisanga_Ctor, party_CPartyItemSilentMisanga_vfunc_3, party_CPartyItemSilentMisanga_vfunc_4
    float impactZoneWidthBonus; // 0x20: CPartyItemSilentMisanga_Ctor, party_CPartyItemSilentMisanga_vfunc_3, party_CPartyItemSilentMisanga_vfunc_4
};

#define OFF(f, o) typedef char CPartyItemSilentMisanga_off_##f[((int)&((CPartyItemSilentMisanga*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(active, 0x10);
OFF(oneMeterWindEnabled, 0x14);
OFF(pangyaZoneWidthBonus, 0x18);
OFF(gaugeSpeedMultiplier, 0x1c);
OFF(impactZoneWidthBonus, 0x20);
#undef OFF

#endif
