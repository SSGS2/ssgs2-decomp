// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CINGAMEDIALOG_H
#define CLASSES_CINGAMEDIALOG_H

#include "game/types.h"

class CInGameDialog {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* callbackVtable;               // 0x4: 2 uses in CInGameDialog_StartOpening, CInGameDialog_StartClosing
    char pad_008[0x30];                 // 0x8
    int blockRegistryClose;             // 0x38: nonzero blocks the registry close sweep
    char pad_03C[0x4];                  // 0x3c
    unsigned char stringStorageFlags;               // 0x40: 2 uses in CInGameDialog_ResetDialog (CONFLICT sizes [1, 4])
    char pad_041[0x3];                  // 0x41
    int stringLength;                         // 0x44: 2 uses in CInGameDialog_Ctor, CInGameDialog_ResetDialog

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x40, 0x42
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CINGAMEDIALOG_OFF(f, o) typedef char CInGameDialog_off_##f[((int)&((CInGameDialog*)0)->f == (o)) ? 1 : -1]
CINGAMEDIALOG_OFF(callbackVtable, 0x4);
CINGAMEDIALOG_OFF(blockRegistryClose, 0x38);
CINGAMEDIALOG_OFF(stringStorageFlags, 0x40);
CINGAMEDIALOG_OFF(stringLength, 0x44);
#undef CINGAMEDIALOG_OFF

#endif  // CLASSES_CINGAMEDIALOG_H
