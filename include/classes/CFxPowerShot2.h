// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXPOWERSHOT2_H
#define CLASSES_CFXPOWERSHOT2_H

#include "game/types.h"

class CFxPowerShot2 {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    unsigned char sequenceCreationAttempted;                // 0x8: 2 uses in CFxPowerShot2_TryCreateWoodWaveSequence
    char pad_009[0x3];                  // 0x9
    void* sequenceOwner;               // 0xc: 2 uses in CFxPowerShot2_TryCreateWoodWaveSequence
    void* golfBall;                       // 0x10: 1 use in CFxPowerShot2_TryCreateWoodWaveSequence

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x3ac
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXPOWERSHOT2_OFF(f, o) typedef char CFxPowerShot2_off_##f[((int)&((CFxPowerShot2*)0)->f == (o)) ? 1 : -1]
CFXPOWERSHOT2_OFF(sequenceCreationAttempted, 0x8);
CFXPOWERSHOT2_OFF(sequenceOwner, 0xc);
CFXPOWERSHOT2_OFF(golfBall, 0x10);
#undef CFXPOWERSHOT2_OFF

#endif  // CLASSES_CFXPOWERSHOT2_H
