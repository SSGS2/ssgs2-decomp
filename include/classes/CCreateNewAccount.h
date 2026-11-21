// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCREATENEWACCOUNT_H
#define CLASSES_CCREATENEWACCOUNT_H

#include "game/types.h"

class CCreateNewAccount {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* layout; // 0x18: CCreateNewAccount_Ctor/Layout_Init; vfunc_4/Layout_DrawWithOpacity
    void* primaryBpObject; // 0x1C: Ctor stores IdMap_Lookup result; vfunc_4 draws
    void* secondaryBpObject; // 0x20: Ctor stores IdMap_Lookup result; vfunc_4 draws
    int primaryBpId; // 0x24: Ctor passes address to CBpCtrl_CreateEntry
    int secondaryBpId; // 0x28: Ctor passes address to CBpCtrl_CreateEntry
    int auxiliaryBpId; // 0x2C: Ctor passes address to CBpCtrl_CreateEntry
    char accepted;                        // 0x30: 1 use in CCreateNewAccount_Ctor
    char cancelled;                        // 0x31: 1 use in CCreateNewAccount_Ctor
    char selectedCharacterIndex;                        // 0x32: 1 use in CCreateNewAccount_Ctor
    char optionBits;                        // 0x33: 2 uses in CCreateNewAccount_Ctor
    void* child; // 0x34: vfunc_4 virtual draw; dtor destroys
    void* menuMessage; // 0x38: Ctor MenuMessage_Ctor; dtor MenuMessage_Dtor
    void** packedFiles; // 0x3C: Ctor allocates two entries from CPackedFile_Create
    void* textureResource; // 0x40: Ctor calls CPackTextureResource_Init

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x24, 0x28, 0x2c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCREATENEWACCOUNT_OFF(f, o) typedef char CCreateNewAccount_off_##f[((int)&((CCreateNewAccount*)0)->f == (o)) ? 1 : -1]
CCREATENEWACCOUNT_OFF(layout, 0x18);
CCREATENEWACCOUNT_OFF(primaryBpObject, 0x1C);
CCREATENEWACCOUNT_OFF(secondaryBpObject, 0x20);
CCREATENEWACCOUNT_OFF(primaryBpId, 0x24);
CCREATENEWACCOUNT_OFF(secondaryBpId, 0x28);
CCREATENEWACCOUNT_OFF(auxiliaryBpId, 0x2C);
CCREATENEWACCOUNT_OFF(child, 0x34);
CCREATENEWACCOUNT_OFF(menuMessage, 0x38);
CCREATENEWACCOUNT_OFF(packedFiles, 0x3C);
CCREATENEWACCOUNT_OFF(textureResource, 0x40);
CCREATENEWACCOUNT_OFF(accepted, 0x30);
CCREATENEWACCOUNT_OFF(cancelled, 0x31);
CCREATENEWACCOUNT_OFF(selectedCharacterIndex, 0x32);
CCREATENEWACCOUNT_OFF(optionBits, 0x33);
#undef CCREATENEWACCOUNT_OFF

#endif  // CLASSES_CCREATENEWACCOUNT_H
