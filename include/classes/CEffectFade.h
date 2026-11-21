// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEFFECTFADE_H
#define CLASSES_CEFFECTFADE_H

#include "game/types.h"

class CEffectFade {
public:
    unsigned char alpha;                // 0x0: 1 use in CEffectFade_Ctor
    unsigned char red;                // 0x1: 1 use in CEffectFade_Ctor
    unsigned char green;                // 0x2: 1 use in CEffectFade_Ctor
    unsigned char blue;                // 0x3: 1 use in CEffectFade_Ctor
    float topLeftX;                        // 0x4: 1 use in CEffectFade_Ctor
    float topLeftY;                        // 0x8: 1 use in CEffectFade_Ctor
    float topLeftZ;                        // 0xc: 1 use in CEffectFade_Ctor
    unsigned char topLeftRed;               // 0x10: 1 use in CEffectFade_Ctor
    unsigned char topLeftGreen;               // 0x11: 1 use in CEffectFade_Ctor
    unsigned char topLeftBlue;               // 0x12: 1 use in CEffectFade_Ctor
    unsigned char topLeftAlpha;               // 0x13: 1 use in CEffectFade_Ctor
    float topRightX;                       // 0x14: 1 use in CEffectFade_Ctor
    float topRightY;                       // 0x18: 1 use in CEffectFade_Ctor
    float topRightZ;                       // 0x1c: 1 use in CEffectFade_Ctor
    unsigned char topRightRed;               // 0x20: 1 use in CEffectFade_Ctor
    unsigned char topRightGreen;               // 0x21: 1 use in CEffectFade_Ctor
    unsigned char topRightBlue;               // 0x22: 1 use in CEffectFade_Ctor
    unsigned char topRightAlpha;               // 0x23: 1 use in CEffectFade_Ctor
    float bottomLeftX;                       // 0x24: 1 use in CEffectFade_Ctor
    float bottomLeftY;                       // 0x28: 1 use in CEffectFade_Ctor
    float bottomLeftZ;                       // 0x2c: 1 use in CEffectFade_Ctor
    unsigned char bottomLeftRed;               // 0x30: 1 use in CEffectFade_Ctor
    unsigned char bottomLeftGreen;               // 0x31: 1 use in CEffectFade_Ctor
    unsigned char bottomLeftBlue;               // 0x32: 1 use in CEffectFade_Ctor
    unsigned char bottomLeftAlpha;               // 0x33: 1 use in CEffectFade_Ctor
    float bottomRightX;                       // 0x34: 1 use in CEffectFade_Ctor
    float bottomRightY;                       // 0x38: 1 use in CEffectFade_Ctor
    float bottomRightZ;                       // 0x3c: 1 use in CEffectFade_Ctor
    unsigned char bottomRightRed;               // 0x40: 1 use in CEffectFade_Ctor
    unsigned char bottomRightGreen;               // 0x41: 1 use in CEffectFade_Ctor
    unsigned char bottomRightBlue;               // 0x42: 1 use in CEffectFade_Ctor
    unsigned char bottomRightAlpha;               // 0x43: 1 use in CEffectFade_Ctor
    void* primitiveCount;                       // 0x44: 1 use in CEffectFade_Ctor
    unsigned int vtbl;                // 0x48: 1 use in CEffectFade_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEFFECTFADE_OFF(f, o) typedef char CEffectFade_off_##f[((int)&((CEffectFade*)0)->f == (o)) ? 1 : -1]
CEFFECTFADE_OFF(red, 0x1);
CEFFECTFADE_OFF(green, 0x2);
CEFFECTFADE_OFF(blue, 0x3);
CEFFECTFADE_OFF(topLeftX, 0x4);
CEFFECTFADE_OFF(topLeftY, 0x8);
CEFFECTFADE_OFF(topLeftZ, 0xc);
CEFFECTFADE_OFF(topLeftRed, 0x10);
CEFFECTFADE_OFF(topLeftGreen, 0x11);
CEFFECTFADE_OFF(topLeftBlue, 0x12);
CEFFECTFADE_OFF(topLeftAlpha, 0x13);
CEFFECTFADE_OFF(topRightX, 0x14);
CEFFECTFADE_OFF(topRightY, 0x18);
CEFFECTFADE_OFF(topRightZ, 0x1c);
CEFFECTFADE_OFF(topRightRed, 0x20);
CEFFECTFADE_OFF(topRightGreen, 0x21);
CEFFECTFADE_OFF(topRightBlue, 0x22);
CEFFECTFADE_OFF(topRightAlpha, 0x23);
CEFFECTFADE_OFF(bottomLeftX, 0x24);
CEFFECTFADE_OFF(bottomLeftY, 0x28);
CEFFECTFADE_OFF(bottomLeftZ, 0x2c);
CEFFECTFADE_OFF(bottomLeftRed, 0x30);
CEFFECTFADE_OFF(bottomLeftGreen, 0x31);
CEFFECTFADE_OFF(bottomLeftBlue, 0x32);
CEFFECTFADE_OFF(bottomLeftAlpha, 0x33);
CEFFECTFADE_OFF(bottomRightX, 0x34);
CEFFECTFADE_OFF(bottomRightY, 0x38);
CEFFECTFADE_OFF(bottomRightZ, 0x3c);
CEFFECTFADE_OFF(bottomRightRed, 0x40);
CEFFECTFADE_OFF(bottomRightGreen, 0x41);
CEFFECTFADE_OFF(bottomRightBlue, 0x42);
CEFFECTFADE_OFF(bottomRightAlpha, 0x43);
CEFFECTFADE_OFF(primitiveCount, 0x44);
CEFFECTFADE_OFF(vtbl, 0x48);
#undef CEFFECTFADE_OFF

#endif  // CLASSES_CEFFECTFADE_H
