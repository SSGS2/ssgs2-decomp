#ifndef CLASSES_CEFFECTFADE_H
#define CLASSES_CEFFECTFADE_H

#include "game/types.h"

class CEffectFade {
public:
    unsigned char alpha;            // 0x0
    unsigned char red;              // 0x1
    unsigned char green;            // 0x2
    unsigned char blue;             // 0x3
    float topLeftX;                 // 0x4
    float topLeftY;                 // 0x8
    float topLeftZ;                 // 0xc
    unsigned char topLeftRed;       // 0x10
    unsigned char topLeftGreen;     // 0x11
    unsigned char topLeftBlue;      // 0x12
    unsigned char topLeftAlpha;     // 0x13
    float topRightX;                // 0x14
    float topRightY;                // 0x18
    float topRightZ;                // 0x1c
    unsigned char topRightRed;      // 0x20
    unsigned char topRightGreen;    // 0x21
    unsigned char topRightBlue;     // 0x22
    unsigned char topRightAlpha;    // 0x23
    float bottomLeftX;              // 0x24
    float bottomLeftY;              // 0x28
    float bottomLeftZ;              // 0x2c
    unsigned char bottomLeftRed;    // 0x30
    unsigned char bottomLeftGreen;  // 0x31
    unsigned char bottomLeftBlue;   // 0x32
    unsigned char bottomLeftAlpha;  // 0x33
    float bottomRightX;             // 0x34
    float bottomRightY;             // 0x38
    float bottomRightZ;             // 0x3c
    unsigned char bottomRightRed;   // 0x40
    unsigned char bottomRightGreen; // 0x41
    unsigned char bottomRightBlue;  // 0x42
    unsigned char bottomRightAlpha; // 0x43
    void* primitiveCount;           // 0x44
    unsigned int vtbl;              // 0x48

    CEffectFade();
    ~CEffectFade();
    void ClearColor();
    void SetAlpha(unsigned char value);
    unsigned char GetAlpha();
};

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

#endif // CLASSES_CEFFECTFADE_H
