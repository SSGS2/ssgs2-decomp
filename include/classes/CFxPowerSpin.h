// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXPOWERSPIN_H
#define CLASSES_CFXPOWERSPIN_H

#include "game/types.h"

class CFxPowerSpin {
public:
    void* vtable;                        // 0x0: 1 use in CFxPowerSpin_Ctor
    int instanceId;                          // 0x4: 1 use in CFxPowerSpin_Ctor
    void* owner;                        // 0x8: 4 uses in CFxPowerSpin_Ctor
    void* golfBall;                        // 0xc: 1 use in CFxPowerSpin_Ctor
    void* sequence;                       // 0x10: 4 uses in CFxPowerSpin_Ctor, CFxPowerSpin_OrientAndAttachParticles
    unsigned char particlesOriented;               // 0x14: 1 use in CFxPowerSpin_Ctor
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXPOWERSPIN_OFF(f, o) typedef char CFxPowerSpin_off_##f[((int)&((CFxPowerSpin*)0)->f == (o)) ? 1 : -1]
CFXPOWERSPIN_OFF(instanceId, 0x4);
CFXPOWERSPIN_OFF(owner, 0x8);
CFXPOWERSPIN_OFF(golfBall, 0xc);
CFXPOWERSPIN_OFF(sequence, 0x10);
CFXPOWERSPIN_OFF(particlesOriented, 0x14);
#undef CFXPOWERSPIN_OFF

#endif  // CLASSES_CFXPOWERSPIN_H
