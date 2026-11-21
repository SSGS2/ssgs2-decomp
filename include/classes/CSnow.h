// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSNOW_H
#define CLASSES_CSNOW_H

#include "game/types.h"

class CSnow {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    float fallSpeedMultiplier;                        // 0x4: 2 uses in CSnow_Ctor, CSnow_UpdateParticles
    char pad_008[0x18];                 // 0x8
    void* particles;                       // 0x20: 32 uses in CSnow_UpdateParticles, CSnow_dtor, CSnow_Ctor
    char pad_024[0x20];                 // 0x24
    int particleCount;                         // 0x44: 1 use in CSnow_UpdateParticles
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSNOW_OFF(f, o) typedef char CSnow_off_##f[((int)&((CSnow*)0)->f == (o)) ? 1 : -1]
CSNOW_OFF(fallSpeedMultiplier, 0x4);
CSNOW_OFF(particles, 0x20);
CSNOW_OFF(particleCount, 0x44);
#undef CSNOW_OFF

#endif  // CLASSES_CSNOW_H
