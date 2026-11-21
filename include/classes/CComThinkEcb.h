// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOMTHINKECB_H
#define CLASSES_CCOMTHINKECB_H

#include "game/types.h"

class CComThinkEcb {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x24];                 // 0x4
    void* entryTable;                       // 0x28: 1 use in CComThinkCtrl_FindNextEntryByName
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOMTHINKECB_OFF(f, o) typedef char CComThinkEcb_off_##f[((int)&((CComThinkEcb*)0)->f == (o)) ? 1 : -1]
CCOMTHINKECB_OFF(entryTable, 0x28);
#undef CCOMTHINKECB_OFF

#endif  // CLASSES_CCOMTHINKECB_H
