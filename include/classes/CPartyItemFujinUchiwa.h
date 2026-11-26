// CPartyItemFujinUchiwa: the fan party item. A gesture steers the wind it blows
// at the opponents, within a limited heading from the owner's aim, and it
// refreshes a wind effect and sound while the gesture is held.
#ifndef CLASSES_CPARTYITEMFUJINUCHIWA_H
#define CLASSES_CPARTYITEMFUJINUCHIWA_H

#include "game/wvector.h"

class CPartyItemFujinUchiwa {
public:
    CPartyItemFujinUchiwa(void* arg);

    int InitWindAndHeading();
    int UpdateOpponentWind(void* unused, void* out, int index);

    void* vtbl;                   // 0x00
    char pad_004[0x8];            // 0x04
    int ownerIndex;               // 0x0C
    void* inputState;             // 0x10
    WVector initialWind;          // 0x14  wind captured when the shot starts
    WVector initialWindCopy;      // 0x20  starts as a copy of initialWind
    float windHeading;            // 0x2C
    char effectAttachment[0x8];   // 0x30  effect attachment initialized in place
    void* effectAttachmentMatrix; // 0x38
    char pad_03C[0xC];            // 0x3C
    void* effectOffsetPointer;    // 0x48
    char pad_04C[0x48];           // 0x4C
    float effectOffset[3];        // 0x94
    union {
        int effectHandleValue; // 0xA0
        void* effectHandle;    // 0xA0
    };
    int cooldown; // 0xA4  ticks until the effect may be recreated
};

#define CPARTYITEMFUJINUCHIWA_OFF(f, o) \
    typedef char CPartyItemFujinUchiwa_off_##f[((int)&((CPartyItemFujinUchiwa*)0)->f == (o)) ? 1 : -1]
CPARTYITEMFUJINUCHIWA_OFF(vtbl, 0x00);
CPARTYITEMFUJINUCHIWA_OFF(ownerIndex, 0x0C);
CPARTYITEMFUJINUCHIWA_OFF(inputState, 0x10);
CPARTYITEMFUJINUCHIWA_OFF(initialWind, 0x14);
CPARTYITEMFUJINUCHIWA_OFF(initialWindCopy, 0x20);
CPARTYITEMFUJINUCHIWA_OFF(windHeading, 0x2C);
CPARTYITEMFUJINUCHIWA_OFF(effectAttachmentMatrix, 0x38);
CPARTYITEMFUJINUCHIWA_OFF(effectOffsetPointer, 0x48);
CPARTYITEMFUJINUCHIWA_OFF(effectOffset, 0x94);
CPARTYITEMFUJINUCHIWA_OFF(effectHandleValue, 0xA0);
CPARTYITEMFUJINUCHIWA_OFF(effectHandle, 0xA0);
CPARTYITEMFUJINUCHIWA_OFF(cooldown, 0xA4);
#undef CPARTYITEMFUJINUCHIWA_OFF
typedef char CPartyItemFujinUchiwa_size[(sizeof(CPartyItemFujinUchiwa) == 0xA8) ? 1 : -1];

#endif
