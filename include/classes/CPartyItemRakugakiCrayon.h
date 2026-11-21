// CPartyItemRakugakiCrayon: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMRAKUGAKICRAYON_H
#define CLASSES_CPARTYITEMRAKUGAKICRAYON_H

class CPartyItemRakugakiCrayon {
public:
    void* vtbl; // 0x0: CPartyItemRakugakiCrayon_Ctor, party_CPartyItemRakugakiCrayon_dtor
    char pad_004[0x8];
    int ownerIndex; // 0xc: CPartyItemRakugakiCrayon_Ctor, CPartyItemRakugakiCrayon_UpdateDrawing
    void* ribbon; // 0x10: CPartyItemRakugakiCrayon_Ctor, party_CPartyItemRakugakiCrayon_dtor, CPartyItemRakugakiCrayon_UpdateDrawing, CPartyItemRakugakiCrayon_DrawRibbon
};

#define OFF(f, o) typedef char CPartyItemRakugakiCrayon_off_##f[((int)&((CPartyItemRakugakiCrayon*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(ribbon, 0x10);
#undef OFF

#endif
