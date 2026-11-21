// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMTHINKCTRL_H
#define CLASSES_CCOMTHINKCTRL_H

#include "game/types.h"

class CComThinkCtrl {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x20];                 // 0x4
    int unk_24;                         // 0x24: 1 use in CComThinkCtrl_Ctor
    int unk_28;                         // 0x28: 1 use in CComThinkCtrl_Ctor
    char pad_02C[0x18c];                // 0x2c
    void* sharedFile;                      // 0x1b8: 4 uses in CComThinkCtrl_dtor, CComThinkCtrl_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x14
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMTHINKCTRL_OFF(f, o) typedef char CComThinkCtrl_off_##f[((int)&((CComThinkCtrl*)0)->f == (o)) ? 1 : -1]
CCOMTHINKCTRL_OFF(unk_24, 0x24);
CCOMTHINKCTRL_OFF(unk_28, 0x28);
CCOMTHINKCTRL_OFF(sharedFile, 0x1b8);
#undef CCOMTHINKCTRL_OFF

#endif  // CLASSES_CCOMTHINKCTRL_H
