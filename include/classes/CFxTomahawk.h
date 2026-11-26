#ifndef CLASSES_CFXTOMAHAWK_H
#define CLASSES_CFXTOMAHAWK_H

#include "game/types.h"

class CFxTomahawk {
public:
    void* vtbl;                             // 0x0: original vtable pointer
    char pad_004[0x4];                      // 0x4
    void* sequenceOwner;                    // 0x8
    void* ball;                             // 0xc
    void* sequence;                         // 0x10
    unsigned char notifyTrajectoryOnImpact; // 0x14
    char pad_015[0x3];                      // 0x15
    int state;                              // 0x18

    int AdvancePhase();

    ~CFxTomahawk();
};

#define CFXTOMAHAWK_OFF(f, o) typedef char CFxTomahawk_off_##f[((int)&((CFxTomahawk*)0)->f == (o)) ? 1 : -1]
CFXTOMAHAWK_OFF(sequenceOwner, 0x8);
CFXTOMAHAWK_OFF(ball, 0xc);
CFXTOMAHAWK_OFF(sequence, 0x10);
CFXTOMAHAWK_OFF(notifyTrajectoryOnImpact, 0x14);
CFXTOMAHAWK_OFF(state, 0x18);
#undef CFXTOMAHAWK_OFF

#endif // CLASSES_CFXTOMAHAWK_H
