// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTEFFECTSC_H
#define CLASSES_CEVENTEFFECTSC_H

#include "game/types.h"

class CEventEffectSC {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    unsigned char fadeDirection;                // 0x4: 1 use in CEventEffectSC_EFF_FADE_UpdateFadeAlpha
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CEVENTEFFECTSC_OFF(f, o) typedef char CEventEffectSC_off_##f[((int)&((CEventEffectSC*)0)->f == (o)) ? 1 : -1]
CEVENTEFFECTSC_OFF(fadeDirection, 0x4);
#undef CEVENTEFFECTSC_OFF

#endif  // CLASSES_CEVENTEFFECTSC_H
