// CPartyItemTitanWristband: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMTITANWRISTBAND_H
#define CLASSES_CPARTYITEMTITANWRISTBAND_H

class CPartyItemTitanWristband {
public:
    void* vtbl; // 0x0: CPartyItemTitanWristband_Ctor, party_CPartyItemTitanWristband_dtor
    char pad_004[0x8];
    int ownerIndex; // 0xc: party_CPartyItemTitanWristband_vfunc_3, party_CPartyItemTitanWristband_vfunc_4
    float baseValue; // 0x10: CPartyItemTitanWristband_Ctor, party_CPartyItemTitanWristband_vfunc_3, party_CPartyItemTitanWristband_vfunc_4
    float maximumRampModifier; // 0x14: CPartyItemTitanWristband_Ctor, party_CPartyItemTitanWristband_vfunc_4
    float collisionRadiusIncreaseRatio; // 0x18: CPartyItemTitanWristband_Ctor, party_CPartyItemTitanWristband_vfunc_4
    int rampLength; // 0x1c: CPartyItemTitanWristband_Ctor, party_CPartyItemTitanWristband_vfunc_4
};

#define OFF(f, o) typedef char CPartyItemTitanWristband_off_##f[((int)&((CPartyItemTitanWristband*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(baseValue, 0x10);
OFF(maximumRampModifier, 0x14);
OFF(collisionRadiusIncreaseRatio, 0x18);
OFF(rampLength, 0x1c);
#undef OFF

#endif
