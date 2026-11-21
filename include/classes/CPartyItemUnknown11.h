// CPartyItemUnknown11: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMUNKNOWN11_H
#define CLASSES_CPARTYITEMUNKNOWN11_H

class CPartyItemUnknown11 {
public:
    void* vtbl; // 0x0: CPartyItemRandomClub_Ctor
    char pad_004[0xc];
    int randomClubApplied; // 0x10: CPartyItemRandomClub_Ctor
};

#define OFF(f, o) typedef char CPartyItemUnknown11_off_##f[((int)&((CPartyItemUnknown11*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(randomClubApplied, 0x10);
#undef OFF

#endif
