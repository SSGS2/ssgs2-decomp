#ifndef CLASSES_GTLFXCHORUS_H
#define CLASSES_GTLFXCHORUS_H

#include "game/types.h"

class GtlFxChorus {
public:
    char pad_000[0x4];  // 0x0: uninterpreted prefix
    unsigned int unk_4; // 0x4: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    unsigned int unk_8; // 0x8: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    unsigned char
        initializationAttempted; // 0xc: gtl_nw4r_FxChorus_Ctor (0x80409D60), gtl_nw4r_FxChorus_vfunc_1 (0x8040A174), gtl_nw4r_FxChorus_Initialize (0x80409F00)
    char pad_00D[0x3];           // 0xd
    unsigned int outputBuffer;   // 0x10: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    unsigned int unk_14;         // 0x14: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    float delayParameter;        // 0x18: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    float depthRatio;            // 0x1c: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    float rateParameter;         // 0x20: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    char chorusState[0xa0];      // 0x24
    unsigned int clampedDelay;   // 0xc4: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    unsigned int clampedDepth;   // 0xc8: gtl_nw4r_FxChorus_Ctor (0x80409D60)
    unsigned int clampedPeriod;  // 0xcc: gtl_nw4r_FxChorus_Ctor (0x80409D60)
};

#define GTLFXCHORUS_OFF(f, o) typedef char GtlFxChorus_off_##f[((int)&((GtlFxChorus*)0)->f == (o)) ? 1 : -1]
GTLFXCHORUS_OFF(unk_4, 0x4);
GTLFXCHORUS_OFF(unk_8, 0x8);
GTLFXCHORUS_OFF(initializationAttempted, 0xc);
GTLFXCHORUS_OFF(outputBuffer, 0x10);
GTLFXCHORUS_OFF(unk_14, 0x14);
GTLFXCHORUS_OFF(delayParameter, 0x18);
GTLFXCHORUS_OFF(depthRatio, 0x1c);
GTLFXCHORUS_OFF(rateParameter, 0x20);
GTLFXCHORUS_OFF(clampedDelay, 0xc4);
GTLFXCHORUS_OFF(clampedDepth, 0xc8);
GTLFXCHORUS_OFF(clampedPeriod, 0xcc);
#undef GTLFXCHORUS_OFF

#endif // CLASSES_GTLFXCHORUS_H
