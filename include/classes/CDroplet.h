// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CDROPLET_H
#define CLASSES_CDROPLET_H

#include "game/types.h"

class CDroplet {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x124];                // 0x4
    int particleCount;                        // 0x128: 1 use in CDroplet_DrawParticles
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CDROPLET_OFF(f, o) typedef char CDroplet_off_##f[((int)&((CDroplet*)0)->f == (o)) ? 1 : -1]
CDROPLET_OFF(particleCount, 0x128);
#undef CDROPLET_OFF

#endif  // CLASSES_CDROPLET_H
