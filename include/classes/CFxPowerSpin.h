// CFxPowerSpin: the spin effect played around the ball after a Power Spin shot.
//
// The sequence is chosen from the ground type under the ball and registered with
// the effect box that owns the control.
#ifndef CLASSES_CFXPOWERSPIN_H
#define CLASSES_CFXPOWERSPIN_H

#include "game/types.h"

class CFxPowerSpin {
public:
    CFxPowerSpin(void* owner, void* ball);

    // Orient the particles once along the shot direction, then follow the ball.
    int OrientAndAttachParticles();
    static void* vfunc_0(void* ptr, int shouldDelete);

    void* vtbl;                      // 0x0
    int instanceId;                  // 0x4
    void* owner;                     // 0x8
    void* golfBall;                  // 0xc
    void* sequence;                  // 0x10
    unsigned char particlesOriented; // 0x14
    char pad_015[3];                 // 0x15
};

#define CFXPOWERSPIN_OFF(f, o) typedef char CFxPowerSpin_off_##f[((int)&((CFxPowerSpin*)0)->f == (o)) ? 1 : -1]
CFXPOWERSPIN_OFF(vtbl, 0x0);
CFXPOWERSPIN_OFF(instanceId, 0x4);
CFXPOWERSPIN_OFF(owner, 0x8);
CFXPOWERSPIN_OFF(golfBall, 0xc);
CFXPOWERSPIN_OFF(sequence, 0x10);
CFXPOWERSPIN_OFF(particlesOriented, 0x14);
#undef CFXPOWERSPIN_OFF
typedef char CFxPowerSpin_size[(sizeof(CFxPowerSpin) == 0x18) ? 1 : -1];

#endif // CLASSES_CFXPOWERSPIN_H
