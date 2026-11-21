// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBPCTRL_H
#define CLASSES_CBPCTRL_H

#include "game/types.h"

class CBpCtrl {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    void* unk_8;                        // 0x8: 1 use in CPlayerStatus_PollResourcesLoaded
    char pad_00C[0x98];                 // 0xc
    void* unk_A4;                       // 0xa4: 1 use in CPlayerStatus_PollResourcesLoaded
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBPCTRL_OFF(f, o) typedef char CBpCtrl_off_##f[((int)&((CBpCtrl*)0)->f == (o)) ? 1 : -1]
CBPCTRL_OFF(unk_8, 0x8);
CBPCTRL_OFF(unk_A4, 0xa4);
#undef CBPCTRL_OFF

#endif  // CLASSES_CBPCTRL_H
