// CPartyItemStampHammer: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMSTAMPHAMMER_H
#define CLASSES_CPARTYITEMSTAMPHAMMER_H

class CPartyItemStampHammer {
public:
    void* vtbl; // 0x0: CPartyItemStampHammer_Ctor
    char pad_004[0x8];
    int ownerIndex; // 0xc: CPartyItemStampHammer_UpdateBalloons
    char pad_010[0xc];
    int unk_1C; // 0x1c: CPartyItemStampHammer_Ctor
    int unk_20; // 0x20: CPartyItemStampHammer_Ctor
    int unk_24; // 0x24: CPartyItemStampHammer_Ctor
};

#define OFF(f, o) typedef char CPartyItemStampHammer_off_##f[((int)&((CPartyItemStampHammer*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(unk_1C, 0x1c);
OFF(unk_20, 0x20);
OFF(unk_24, 0x24);
#undef OFF

#endif
