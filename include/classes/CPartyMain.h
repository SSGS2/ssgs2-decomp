// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPARTYMAIN_H
#define CLASSES_CPARTYMAIN_H

#include "game/types.h"

class CPartyMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x20];                 // 0x4
    void* primaryBpEntryId;                       // 0x24: 1 use in CPartyMain_ProcessSelectionScreens
    void* secondaryBpEntryId;                       // 0x28: 1 use in CPartyMain_ProcessSelectionScreens

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x24, 0x28, 0x30
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPARTYMAIN_OFF(f, o) typedef char CPartyMain_off_##f[((int)&((CPartyMain*)0)->f == (o)) ? 1 : -1]
CPARTYMAIN_OFF(primaryBpEntryId, 0x24);
CPARTYMAIN_OFF(secondaryBpEntryId, 0x28);
#undef CPARTYMAIN_OFF

#endif  // CLASSES_CPARTYMAIN_H
