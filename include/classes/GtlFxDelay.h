// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_GTLFXDELAY_H
#define CLASSES_GTLFXDELAY_H

#include "game/types.h"

class GtlFxDelay {
public:
    char pad_000[0x4];                  // 0x0: uninterpreted prefix
    unsigned int unk_4;                 // 0x4: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int unk_8;                 // 0x8: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned char initializationAttempted;          // 0xc: gtl_nw4r_FxDelay_Ctor (0x8040A194), gtl_nw4r_FxDelay_InitializeOnce (0x8040A2E4)
    char pad_00D[0x3];                  // 0xd
    unsigned int outputBuffer;                // 0x10: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int unk_14;                // 0x14: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    float parameters[3];               // 0x18: gtl_nw4r_FxDelay_Ctor (0x8040A194), copied delay/feedback/level defaults
    char delayState[0x40];             // 0x24: same ctor and vfunc_4 (0x8040A2E4) pass to delay initialization; extent ends at delay0
    unsigned int delay0;             // 0x64: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int delay1;            // 0x68: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int delay2;         // 0x6c: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int feedback0;          // 0x70: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int feedback1;         // 0x74: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int feedback2;      // 0x78: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int level0;             // 0x7c: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int level1;            // 0x80: gtl_nw4r_FxDelay_Ctor (0x8040A194)
    unsigned int level2;         // 0x84: gtl_nw4r_FxDelay_Ctor (0x8040A194)
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define GTLFXDELAY_OFF(f, o) typedef char GtlFxDelay_off_##f[((int)&((GtlFxDelay*)0)->f == (o)) ? 1 : -1]
GTLFXDELAY_OFF(unk_4, 0x4);
GTLFXDELAY_OFF(unk_8, 0x8);
GTLFXDELAY_OFF(initializationAttempted, 0xc);
GTLFXDELAY_OFF(outputBuffer, 0x10);
GTLFXDELAY_OFF(unk_14, 0x14);
GTLFXDELAY_OFF(parameters, 0x18);
GTLFXDELAY_OFF(delayState, 0x24);
GTLFXDELAY_OFF(delay0, 0x64);
GTLFXDELAY_OFF(delay1, 0x68);
GTLFXDELAY_OFF(delay2, 0x6c);
GTLFXDELAY_OFF(feedback0, 0x70);
GTLFXDELAY_OFF(feedback1, 0x74);
GTLFXDELAY_OFF(feedback2, 0x78);
GTLFXDELAY_OFF(level0, 0x7c);
GTLFXDELAY_OFF(level1, 0x80);
GTLFXDELAY_OFF(level2, 0x84);
#undef GTLFXDELAY_OFF

#endif  // CLASSES_GTLFXDELAY_H
