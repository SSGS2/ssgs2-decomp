// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFILEIOMENU_H
#define CLASSES_CFILEIOMENU_H

#include "game/types.h"

class CFileIoMenu {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* fileAccess;                       // 0x18: 1 use in CFileIoMenu_QueryResourcePredicate11
    char pad_01C[0x4];                  // 0x1c
    void* commonDialog;                 // 0x20: CFileIoMenu_ShowAcknowledgeDialog/F4F0/F724 create CPangyaCommonDialog; F9BC releases
    unsigned char commonDialogComplete;               // 0x24: 1 use in CFileIoMenu_dtor
    char pad_025[0xB];
    void* bpObject;                     // 0x30: CFileIoMenu_SetScriptEnabled passes to BpObject_SetEnabled; EFC4 draws

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x34
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFILEIOMENU_OFF(f, o) typedef char CFileIoMenu_off_##f[((int)&((CFileIoMenu*)0)->f == (o)) ? 1 : -1]
CFILEIOMENU_OFF(fileAccess, 0x18);
CFILEIOMENU_OFF(commonDialogComplete, 0x24);
CFILEIOMENU_OFF(commonDialog, 0x20);
CFILEIOMENU_OFF(bpObject, 0x30);
#undef CFILEIOMENU_OFF

#endif  // CLASSES_CFILEIOMENU_H
