#ifndef CLASSES_CSPDPANGYAICON_H
#define CLASSES_CSPDPANGYAICON_H

#include "game/types.h"

class CSPDPangyaIcon {
public:
    void* vtbl;                        // 0x0: original vtable pointer
    char pad_004[0x44];                // 0x4
    void* ownedChild0;                 // 0x48
    void* ownedChild1;                 // 0x4c
    void* ownedChild2;                 // 0x50
    void* ownedChild3;                 // 0x54
    unsigned char primaryChildIndex;   // 0x58
    unsigned char alternateChildIndex; // 0x59
};

#define CSPDPANGYAICON_OFF(f, o) typedef char CSPDPangyaIcon_off_##f[((int)&((CSPDPangyaIcon*)0)->f == (o)) ? 1 : -1]
CSPDPANGYAICON_OFF(ownedChild0, 0x48);
CSPDPANGYAICON_OFF(ownedChild1, 0x4c);
CSPDPANGYAICON_OFF(ownedChild2, 0x50);
CSPDPANGYAICON_OFF(ownedChild3, 0x54);
CSPDPANGYAICON_OFF(primaryChildIndex, 0x58);
CSPDPANGYAICON_OFF(alternateChildIndex, 0x59);
#undef CSPDPANGYAICON_OFF

#endif // CLASSES_CSPDPANGYAICON_H
