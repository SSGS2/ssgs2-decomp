// CFxPowerShot2: the wood-wave effect of a Power Shot 2.
//
// The sequence is created once, on the first update, at the ball's position.
#ifndef CLASSES_CFXPOWERSHOT2_H
#define CLASSES_CFXPOWERSHOT2_H

#include "game/types.h"

class CFxPowerShot2 {
public:
    ~CFxPowerShot2();

    int TryCreateWoodWaveSequence();

    void* vtbl;                              // 0x0: original vtable pointer
    char pad_004[0x4];                       // 0x4
    unsigned char sequenceCreationAttempted; // 0x8
    char pad_009[0x3];                       // 0x9
    void* sequenceOwner;                     // 0xc
    void* golfBall;                          // 0x10
};

#define CFXPOWERSHOT2_OFF(f, o) typedef char CFxPowerShot2_off_##f[((int)&((CFxPowerShot2*)0)->f == (o)) ? 1 : -1]
CFXPOWERSHOT2_OFF(vtbl, 0x0);
CFXPOWERSHOT2_OFF(sequenceCreationAttempted, 0x8);
CFXPOWERSHOT2_OFF(sequenceOwner, 0xc);
CFXPOWERSHOT2_OFF(golfBall, 0x10);
#undef CFXPOWERSHOT2_OFF
typedef char CFxPowerShot2_size[(sizeof(CFxPowerShot2) == 0x14) ? 1 : -1];

#endif // CLASSES_CFXPOWERSHOT2_H
