// CPartyItemFujinUchiwa: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMFUJINUCHIWA_H
#define CLASSES_CPARTYITEMFUJINUCHIWA_H

class CPartyItemFujinUchiwa {
public:
    void* vtbl; // 0x0: CPartyItemFujinUchiwa_Ctor, party_CPartyItemFujinUchiwa_dtor
    char pad_004[0x8];
    int ownerIndex; // 0xc: CPartyItemFujinUchiwa_UpdateOpponentWind
    void* inputState; // 0x10: CPartyItemFujinUchiwa_UpdateOpponentWind
    char pad_014[0x18];
    float windHeading; // 0x2c: CPartyItemFujinUchiwa_InitWindAndHeading, CPartyItemFujinUchiwa_UpdateOpponentWind
    char pad_030[0x8];
    void* effectAttachmentMatrix; // 0x38: CPartyItemFujinUchiwa_UpdateOpponentWind
    char pad_03C[0xc];
    void* effectOffsetPointer; // 0x48: CPartyItemFujinUchiwa_UpdateOpponentWind
    char pad_04C[0x48];
    float effectOffset[3]; // 0x94: CPartyItemFujinUchiwa_UpdateOpponentWind zeros this vector and assigns its address to effectOffsetPointer
    union {
        int effectHandleValue; // 0xa0: CPartyItemFujinUchiwa_Ctor, CPartyItemFujinUchiwa_InitWindAndHeading, CPartyItemFujinUchiwa_UpdateOpponentWind
        void* effectHandle; // 0xa0: CPartyItemFujinUchiwa_UpdateOpponentWind
    };
    int cooldown; // 0xa4: CPartyItemFujinUchiwa_Ctor, CPartyItemFujinUchiwa_UpdateOpponentWind
};

#define OFF(f, o) typedef char CPartyItemFujinUchiwa_off_##f[((int)&((CPartyItemFujinUchiwa*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(ownerIndex, 0xc);
OFF(inputState, 0x10);
OFF(windHeading, 0x2c);
OFF(effectAttachmentMatrix, 0x38);
OFF(effectOffsetPointer, 0x48);
OFF(effectOffset, 0x94);
OFF(effectHandleValue, 0xa0);
OFF(effectHandle, 0xa0);
OFF(cooldown, 0xa4);
#undef OFF

#endif
