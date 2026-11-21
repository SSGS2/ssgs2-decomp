// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CRIPPLE_H
#define CLASSES_CRIPPLE_H

#include "game/types.h"

class CRipple {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int remainingRippleCount;                          // 0x4: 2 uses in CRipple_Ctor, CRipple_Begin
    float positionX;                        // 0x8: 2 uses in CRipple_Draw, CRipple_Begin
    float positionY;                        // 0xc: 1 use in CRipple_Draw
    float positionZ;                       // 0x10: 1 use in CRipple_Draw
    int textureId;                         // 0x14: 3 uses in CRipple_Draw, CRipple_Ctor
    unsigned char active;               // 0x18: 2 uses in CRipple_Draw, CRipple_Begin
    char pad_019[0x3];                  // 0x19
    void* ownedObject1;                       // 0x1c: 3 uses in CRipple_dtor, CRipple_Ctor
    void* ownedObject2;                       // 0x20: 2 uses in CRipple_dtor
    char pad_024[0x100];                // 0x24
    void* ripples;                      // 0x124: 13 uses in CRipple_Begin, CRipple_Draw, CRipple_dtor
    int rippleCount;                        // 0x128: 4 uses in CRipple_Begin, CRipple_Draw
    void* dropletEffect;                      // 0x12c: 5 uses in CRipple_dtor, CRipple_Begin, CRipple_Draw

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x44, 0x7c, 0xb4, 0xec
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CRIPPLE_OFF(f, o) typedef char CRipple_off_##f[((int)&((CRipple*)0)->f == (o)) ? 1 : -1]
CRIPPLE_OFF(remainingRippleCount, 0x4);
CRIPPLE_OFF(positionX, 0x8);
CRIPPLE_OFF(positionY, 0xc);
CRIPPLE_OFF(positionZ, 0x10);
CRIPPLE_OFF(textureId, 0x14);
CRIPPLE_OFF(active, 0x18);
CRIPPLE_OFF(ownedObject1, 0x1c);
CRIPPLE_OFF(ownedObject2, 0x20);
CRIPPLE_OFF(ripples, 0x124);
CRIPPLE_OFF(rippleCount, 0x128);
CRIPPLE_OFF(dropletEffect, 0x12c);
#undef CRIPPLE_OFF

#endif  // CLASSES_CRIPPLE_H
