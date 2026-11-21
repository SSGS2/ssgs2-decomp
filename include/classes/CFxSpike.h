// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXSPIKE_H
#define CLASSES_CFXSPIKE_H

#include "game/types.h"

class CFxSpike {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* effectManager;                        // 0x8: 10 uses in CFxSpike_UpdateAndAdvanceState
    void* ball;                        // 0xc: 19 uses in CFxSpike_UpdateAndAdvanceState
    void* initialSequence;                       // 0x10: CFxSpike_UpdateAndAdvanceState registers in state 0, clears sequence flag in state 2
    void* delayedSequence;                       // 0x14: CFxSpike_UpdateAndAdvanceState registers in state 2, clears sequence flag in state 4
    unsigned char notifyTrajectoryOnImpact;               // 0x18: 1 use in CFxSpike_UpdateAndAdvanceState
    char pad_019[0x3];                  // 0x19
    int state;                          // 0x1c: 5 uses in CFxSpike_UpdateAndAdvanceState

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x3ac
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXSPIKE_OFF(f, o) typedef char CFxSpike_off_##f[((int)&((CFxSpike*)0)->f == (o)) ? 1 : -1]
CFXSPIKE_OFF(effectManager, 0x8);
CFXSPIKE_OFF(ball, 0xc);
CFXSPIKE_OFF(initialSequence, 0x10);
CFXSPIKE_OFF(delayedSequence, 0x14);
CFXSPIKE_OFF(notifyTrajectoryOnImpact, 0x18);
CFXSPIKE_OFF(state, 0x1c);
#undef CFXSPIKE_OFF

#endif  // CLASSES_CFXSPIKE_H
