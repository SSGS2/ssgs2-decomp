// CFxSpike: the spike shot, from launch through the beam to the impact.
//
// Each stage creates its own sequence; the initial and delayed sequences are
// registered with the effect box for the whole life of the control.
#ifndef CLASSES_CFXSPIKE_H
#define CLASSES_CFXSPIKE_H

#include "game/types.h"

class CFxSpike {
public:
    ~CFxSpike();

    int UpdateAndAdvanceState();

    void* vtbl;                             // 0x0: original vtable pointer
    char pad_004[0x4];                      // 0x4
    void* effectManager;                    // 0x8
    void* ball;                             // 0xc
    void* initialSequence;                  // 0x10
    void* delayedSequence;                  // 0x14
    unsigned char notifyTrajectoryOnImpact; // 0x18
    char pad_019[0x3];                      // 0x19
    int state;                              // 0x1c
};

#define CFXSPIKE_OFF(f, o) typedef char CFxSpike_off_##f[((int)&((CFxSpike*)0)->f == (o)) ? 1 : -1]
CFXSPIKE_OFF(vtbl, 0x0);
CFXSPIKE_OFF(effectManager, 0x8);
CFXSPIKE_OFF(ball, 0xc);
CFXSPIKE_OFF(initialSequence, 0x10);
CFXSPIKE_OFF(delayedSequence, 0x14);
CFXSPIKE_OFF(notifyTrajectoryOnImpact, 0x18);
CFXSPIKE_OFF(state, 0x1c);
#undef CFXSPIKE_OFF
typedef char CFxSpike_size[(sizeof(CFxSpike) == 0x20) ? 1 : -1];

#endif // CLASSES_CFXSPIKE_H
