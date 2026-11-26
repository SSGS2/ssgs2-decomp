// CPartyItemRandomClub: party item 11, the random club. Only its vtable and latch are named.
#ifndef CLASSES_CPARTYITEMRANDOMCLUB_H
#define CLASSES_CPARTYITEMRANDOMCLUB_H

class CPartyItemRandomClub {
public:
    CPartyItemRandomClub(int size);
    ~CPartyItemRandomClub();

    void* vtbl;            // 0x00
    char pad_004[0xC];     // 0x04
    int randomClubApplied; // 0x10  set once the club is chosen
};

#define OFF(f, o) typedef char CPartyItemRandomClub_off_##f[((int)&((CPartyItemRandomClub*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x00);
OFF(randomClubApplied, 0x10);
#undef OFF
typedef char CPartyItemRandomClub_size[(sizeof(CPartyItemRandomClub) == 0x14) ? 1 : -1];

#endif // CLASSES_CPARTYITEMRANDOMCLUB_H
