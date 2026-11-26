#ifndef CLASSES_CEVENTEFFECTSC_H
#define CLASSES_CEVENTEFFECTSC_H

#include "game/types.h"

class CEventEffectSC {
public:
    void* vtbl;                  // 0x0: original vtable pointer
    unsigned char fadeDirection; // 0x4
};

#define CEVENTEFFECTSC_OFF(f, o) typedef char CEventEffectSC_off_##f[((int)&((CEventEffectSC*)0)->f == (o)) ? 1 : -1]
CEVENTEFFECTSC_OFF(fadeDirection, 0x4);
#undef CEVENTEFFECTSC_OFF

#endif // CLASSES_CEVENTEFFECTSC_H
