// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXTOMAHAWK_H
#define CLASSES_CFXTOMAHAWK_H

#include "game/types.h"

class CFxTomahawk {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* sequenceOwner;                        // 0x8: 4 uses in CFxTomahawk_AdvancePhase
    void* ball;                        // 0xc: 10 uses in CFxTomahawk_AdvancePhase
    void* sequence;                       // 0x10: 9 uses in CFxTomahawk_AdvancePhase
    unsigned char notifyTrajectoryOnImpact;               // 0x14: 1 use in CFxTomahawk_AdvancePhase
    char pad_015[0x3];                  // 0x15
    int state;                         // 0x18: 4 uses in CFxTomahawk_AdvancePhase

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8, 0x3ac
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXTOMAHAWK_OFF(f, o) typedef char CFxTomahawk_off_##f[((int)&((CFxTomahawk*)0)->f == (o)) ? 1 : -1]
CFXTOMAHAWK_OFF(sequenceOwner, 0x8);
CFXTOMAHAWK_OFF(ball, 0xc);
CFXTOMAHAWK_OFF(sequence, 0x10);
CFXTOMAHAWK_OFF(notifyTrajectoryOnImpact, 0x14);
CFXTOMAHAWK_OFF(state, 0x18);
#undef CFXTOMAHAWK_OFF

#endif  // CLASSES_CFXTOMAHAWK_H
