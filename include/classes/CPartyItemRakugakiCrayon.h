// CPartyItemRakugakiCrayon: the crayon party item. It owns a trail ribbon that
// follows the pointer and draws in the owner's player color.
#ifndef CLASSES_CPARTYITEMRAKUGAKICRAYON_H
#define CLASSES_CPARTYITEMRAKUGAKICRAYON_H

class CPartyItemRakugakiCrayon {
public:
    CPartyItemRakugakiCrayon(int index, void* resource);
    ~CPartyItemRakugakiCrayon();

    int UpdateDrawing(void* unused, void* unused2, int index);
    int DrawRibbon();

    void* vtbl;        // 0x00
    char pad_004[0x8]; // 0x04
    int ownerIndex;    // 0x0C
    void* ribbon;      // 0x10  owned trail ribbon
};

#define CPARTYITEMRAKUGAKICRAYON_OFF(f, o) \
    typedef char CPartyItemRakugakiCrayon_off_##f[((int)&((CPartyItemRakugakiCrayon*)0)->f == (o)) ? 1 : -1]
CPARTYITEMRAKUGAKICRAYON_OFF(vtbl, 0x00);
CPARTYITEMRAKUGAKICRAYON_OFF(ownerIndex, 0x0C);
CPARTYITEMRAKUGAKICRAYON_OFF(ribbon, 0x10);
#undef CPARTYITEMRAKUGAKICRAYON_OFF
typedef char CPartyItemRakugakiCrayon_size[(sizeof(CPartyItemRakugakiCrayon) == 0x14) ? 1 : -1];

#endif
