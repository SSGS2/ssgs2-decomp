// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CNAMEENTRYMAIN_H
#define CLASSES_CNAMEENTRYMAIN_H

#include "game/types.h"

class CNameEntryMain {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x20];                 // 0x4
    char pendingPlayerRecord[0x828];            // 0x24: NameEntry_CopyNonBlankTextToPendingRecord copies wide text, CNameEntryMain_UpdateStateMachine submits it (name_entry_soft_keyboard.cpp); view extends to next known field
    void* dialog;                      // 0x84c: 1 use in initialize
    void* state;                      // 0x850: 1 use in initialize

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x24
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CNAMEENTRYMAIN_OFF(f, o) typedef char CNameEntryMain_off_##f[((int)&((CNameEntryMain*)0)->f == (o)) ? 1 : -1]
CNAMEENTRYMAIN_OFF(dialog, 0x84c);
CNAMEENTRYMAIN_OFF(pendingPlayerRecord, 0x24);
CNAMEENTRYMAIN_OFF(state, 0x850);
#undef CNAMEENTRYMAIN_OFF

#endif  // CLASSES_CNAMEENTRYMAIN_H
