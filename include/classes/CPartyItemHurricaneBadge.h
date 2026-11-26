// CPartyItemHurricaneBadge: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMHURRICANEBADGE_H
#define CLASSES_CPARTYITEMHURRICANEBADGE_H

class CPartyItemHurricaneBadge {
public:
    CPartyItemHurricaneBadge(int size);
    ~CPartyItemHurricaneBadge();

    void ApplyWindOverrides(void* out, int index);

    void* vtbl;                   // 0x0
    char pad_004[0x8];            // 0x4
    int ownerIndex;               // 0xc
    int unk_10;                   // 0x10
    float minimum;                // 0x14  low end of the random wind range
    float maximum;                // 0x18  high end of the random wind range
    int windSpeedOverrideEnabled; // 0x1c
    int oneMeterWindEnabled;      // 0x20
};

#define OFF(f, o) \
    typedef char CPartyItemHurricaneBadge_off_##f[((int)&((CPartyItemHurricaneBadge*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(unk_10, 0x10);
OFF(minimum, 0x14);
OFF(maximum, 0x18);
OFF(windSpeedOverrideEnabled, 0x1c);
OFF(oneMeterWindEnabled, 0x20);
#undef OFF

#endif
