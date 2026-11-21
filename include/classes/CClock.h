// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCLOCK_H
#define CLASSES_CCLOCK_H

#include "game/types.h"

class CClock {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x18];                 // 0x4
    float countdownDisplayValue;                       // 0x1c: 4 uses in CClock_OnProcess, CClock_OnInit, CClock_HandleMsg
    unsigned int displayColor;                // 0x20: RGBA color sent to IActor_DispatchMessage in CClock_OnProcess; initialized/reset in CClock_HandleMsg
    char pad_024[0x1];                  // 0x24
    unsigned char running;               // 0x25: countdown enabled by CClock_HandleMsg case 0, disabled by case 1 and expiry

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x1c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCLOCK_OFF(f, o) typedef char CClock_off_##f[((int)&((CClock*)0)->f == (o)) ? 1 : -1]
CCLOCK_OFF(countdownDisplayValue, 0x1c);
CCLOCK_OFF(displayColor, 0x20);
CCLOCK_OFF(running, 0x25);
#undef CCLOCK_OFF

#endif  // CLASSES_CCLOCK_H
