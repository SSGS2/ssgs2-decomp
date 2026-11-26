// CFxShotImpact: the effect played where a shot lands.
//
// The sequence is chosen from the ground type at the landing point and its
// particles are turned once to face along the shot.
#ifndef CLASSES_CFXSHOTIMPACT_H
#define CLASSES_CFXSHOTIMPACT_H

#include "game/types.h"

class CFxShotImpact {
public:
    CFxShotImpact(void* owner, void* ball);
    ~CFxShotImpact();

    int OrientParticlesToShotDirection();

    void* vtbl;                           // 0x0
    int serialNumber;                     // 0x4
    void* sequenceOwner;                  // 0x8
    void* ball;                           // 0xc
    void* sequence;                       // 0x10
    unsigned char particlesAlignedToShot; // 0x14
    char pad_015[3];                      // 0x15
};

#define CFXSHOTIMPACT_OFF(f, o) typedef char CFxShotImpact_off_##f[((int)&((CFxShotImpact*)0)->f == (o)) ? 1 : -1]
CFXSHOTIMPACT_OFF(vtbl, 0x0);
CFXSHOTIMPACT_OFF(serialNumber, 0x4);
CFXSHOTIMPACT_OFF(sequenceOwner, 0x8);
CFXSHOTIMPACT_OFF(ball, 0xc);
CFXSHOTIMPACT_OFF(sequence, 0x10);
CFXSHOTIMPACT_OFF(particlesAlignedToShot, 0x14);
#undef CFXSHOTIMPACT_OFF
typedef char CFxShotImpact_size[(sizeof(CFxShotImpact) == 0x18) ? 1 : -1];

#endif // CLASSES_CFXSHOTIMPACT_H
