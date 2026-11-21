// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXBOOSTERWARP_H
#define CLASSES_CFXBOOSTERWARP_H

#include "game/types.h"

class CFxBoosterWarp {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x8];                  // 0x4
    void* ball;                        // 0xc: 5 uses in CFxBoosterWarp_Update
    void* ballFollowSequence;                       // 0x10: 4 uses in CFxBoosterWarp_Update
    void* launchSequence;                       // 0x14: 7 uses in CFxBoosterWarp_Update
    unsigned char initialSetupComplete;               // 0x18: 2 uses in CFxBoosterWarp_Update

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x3ac, 0x3b0
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXBOOSTERWARP_OFF(f, o) typedef char CFxBoosterWarp_off_##f[((int)&((CFxBoosterWarp*)0)->f == (o)) ? 1 : -1]
CFXBOOSTERWARP_OFF(ball, 0xc);
CFXBOOSTERWARP_OFF(ballFollowSequence, 0x10);
CFXBOOSTERWARP_OFF(launchSequence, 0x14);
CFXBOOSTERWARP_OFF(initialSetupComplete, 0x18);
#undef CFXBOOSTERWARP_OFF

#endif  // CLASSES_CFXBOOSTERWARP_H
