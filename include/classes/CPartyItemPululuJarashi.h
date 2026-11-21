// CPartyItemPululuJarashi: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMPULULUJARASHI_H
#define CLASSES_CPARTYITEMPULULUJARASHI_H

class CPartyItemPululuJarashi {
public:
    void* vtbl; // 0x0: CPartyItemPululuJarashi_Ctor
    char pad_004[0x8];
    int ownerIndex; // 0xc: CPartyItemPululuJarashi_UpdateTickleStrength
    float strength; // 0x10: CPartyItemPululuJarashi_Ctor, CPartyItemPululuJarashi_UpdateTickleStrength
    int activeTicks; // 0x14: CPartyItemPululuJarashi_Ctor, party_CPartyItemPululuJarashi_vfunc_3
};

#define OFF(f, o) typedef char CPartyItemPululuJarashi_off_##f[((int)&((CPartyItemPululuJarashi*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(strength, 0x10);
OFF(activeTicks, 0x14);
#undef OFF

#endif
