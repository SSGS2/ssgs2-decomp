// CPartyItemStampHammer: the hammer party item. Its update aims at the nearest
// balloon in front of the player and drives the shared release queue.
#ifndef CLASSES_CPARTYITEMSTAMPHAMMER_H
#define CLASSES_CPARTYITEMSTAMPHAMMER_H

class CPartyItemStampHammer {
public:
    CPartyItemStampHammer(void* arg);

    int UpdateBalloons(int unused, int unused2, int index, int suppressHistoryRelease);

    void* vtbl;        // 0x00
    char pad_004[0x8]; // 0x04
    int ownerIndex;    // 0x0C
    char pad_010[0xC]; // 0x10
    int unk_1C;        // 0x1C
    int unk_20;        // 0x20
    int unk_24;        // 0x24
};

#define CPARTYITEMSTAMPHAMMER_OFF(f, o) \
    typedef char CPartyItemStampHammer_off_##f[((int)&((CPartyItemStampHammer*)0)->f == (o)) ? 1 : -1]
CPARTYITEMSTAMPHAMMER_OFF(vtbl, 0x00);
CPARTYITEMSTAMPHAMMER_OFF(ownerIndex, 0x0C);
CPARTYITEMSTAMPHAMMER_OFF(unk_1C, 0x1C);
CPARTYITEMSTAMPHAMMER_OFF(unk_20, 0x20);
CPARTYITEMSTAMPHAMMER_OFF(unk_24, 0x24);
#undef CPARTYITEMSTAMPHAMMER_OFF
typedef char CPartyItemStampHammer_size[(sizeof(CPartyItemStampHammer) == 0x28) ? 1 : -1];

#endif
