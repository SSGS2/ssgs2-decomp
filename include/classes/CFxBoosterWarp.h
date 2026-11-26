// CFxBoosterWarp: the gate and trail effects of a booster item.
//
// Both sequences belong to the effect box that created the control and are
// removed along with it.
#ifndef CLASSES_CFXBOOSTERWARP_H
#define CLASSES_CFXBOOSTERWARP_H

#include "game/types.h"

class CFxBoosterWarp {
public:
    ~CFxBoosterWarp();

    int Update();

    void* vtbl;                         // 0x0
    char pad_004[0x4];                  // 0x4
    void* owner;                        // 0x8: effect box that owns both sequences
    void* ball;                         // 0xc
    void* ballFollowSequence;           // 0x10
    void* launchSequence;               // 0x14
    unsigned char initialSetupComplete; // 0x18
    char pad_019[3];                    // 0x19
};

#define CFXBOOSTERWARP_OFF(f, o) typedef char CFxBoosterWarp_off_##f[((int)&((CFxBoosterWarp*)0)->f == (o)) ? 1 : -1]
CFXBOOSTERWARP_OFF(vtbl, 0x0);
CFXBOOSTERWARP_OFF(owner, 0x8);
CFXBOOSTERWARP_OFF(ball, 0xc);
CFXBOOSTERWARP_OFF(ballFollowSequence, 0x10);
CFXBOOSTERWARP_OFF(launchSequence, 0x14);
CFXBOOSTERWARP_OFF(initialSetupComplete, 0x18);
#undef CFXBOOSTERWARP_OFF
typedef char CFxBoosterWarp_size[(sizeof(CFxBoosterWarp) == 0x1c) ? 1 : -1];

#endif // CLASSES_CFXBOOSTERWARP_H
