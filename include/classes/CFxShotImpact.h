// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXSHOTIMPACT_H
#define CLASSES_CFXSHOTIMPACT_H

#include "game/types.h"

class CFxShotImpact {
public:
    void* vtbl;                        // 0x0: 1 use in CFxShotImpact_Ctor
    int serialNumber;                          // 0x4: 1 use in CFxShotImpact_Ctor
    void* sequenceOwner;                        // 0x8: 9 uses in CFxShotImpact_Ctor
    void* ball;                        // 0xc: 2 uses in CFxShotImpact_Ctor
    void* sequence;                       // 0x10: 12 uses in CFxShotImpact_Ctor, CFxShotImpact_OrientParticlesToShotDirection
    unsigned char particlesAlignedToShot;               // 0x14: 1 use in CFxShotImpact_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXSHOTIMPACT_OFF(f, o) typedef char CFxShotImpact_off_##f[((int)&((CFxShotImpact*)0)->f == (o)) ? 1 : -1]
CFXSHOTIMPACT_OFF(serialNumber, 0x4);
CFXSHOTIMPACT_OFF(sequenceOwner, 0x8);
CFXSHOTIMPACT_OFF(ball, 0xc);
CFXSHOTIMPACT_OFF(sequence, 0x10);
CFXSHOTIMPACT_OFF(particlesAlignedToShot, 0x14);
#undef CFXSHOTIMPACT_OFF

#endif  // CLASSES_CFXSHOTIMPACT_H
