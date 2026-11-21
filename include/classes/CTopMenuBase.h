// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTOPMENUBASE_H
#define CLASSES_CTOPMENUBASE_H

#include "game/types.h"

class CTopMenuBase {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* layout;                       // 0x4: 5 uses in CTopMenuBase_Ctor, __dt__7MPBase2Fv, CTopMenuBase_InitMembers
    void* bpObject;                     // 0x8: 3 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    int result;                          // 0xc: 2 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    unsigned char resultReady;               // 0x10: 2 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    unsigned char backRequested;               // 0x11: 2 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    char pad_012[0x2];                  // 0x12
    int menuItemCount;                  // 0x14: 2 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    unsigned char scriptRunning;               // 0x18: 2 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    char pad_019[0x3];                  // 0x19
    void* message;                      // 0x1c: 6 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers, __dt__7MPBase2Fv
    int optionDialog;                         // 0x20: 4 uses in __dt__7MPBase2Fv, CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    unsigned char optionDialogRequested;               // 0x24: 3 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers, CTopMenuBase_RequestOptionsDialog
    char pad_025[0x3];                  // 0x25
    int bpEntryId;                      // 0x28: 6 uses in __dt__7MPBase2Fv, CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    int elementIds;                     // 0x2c: 2 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers
    unsigned char borrowsResources;     // 0x30: 3 uses in CTopMenuBase_Ctor, CTopMenuBase_InitMembers, __dt__7MPBase2Fv
    char pad_031[0x3];                  // 0x31
    void* textureResource;              // 0x34: 5 uses in CTopMenuBase_Ctor, __dt__7MPBase2Fv, CTopMenuBase_InitMembers

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x28
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTOPMENUBASE_OFF(f, o) typedef char CTopMenuBase_off_##f[((int)&((CTopMenuBase*)0)->f == (o)) ? 1 : -1]
CTOPMENUBASE_OFF(layout, 0x4);
CTOPMENUBASE_OFF(bpObject, 0x8);
CTOPMENUBASE_OFF(result, 0xc);
CTOPMENUBASE_OFF(resultReady, 0x10);
CTOPMENUBASE_OFF(backRequested, 0x11);
CTOPMENUBASE_OFF(menuItemCount, 0x14);
CTOPMENUBASE_OFF(scriptRunning, 0x18);
CTOPMENUBASE_OFF(message, 0x1c);
CTOPMENUBASE_OFF(optionDialog, 0x20);
CTOPMENUBASE_OFF(optionDialogRequested, 0x24);
CTOPMENUBASE_OFF(bpEntryId, 0x28);
CTOPMENUBASE_OFF(elementIds, 0x2c);
CTOPMENUBASE_OFF(borrowsResources, 0x30);
CTOPMENUBASE_OFF(textureResource, 0x34);
#undef CTOPMENUBASE_OFF

#endif  // CLASSES_CTOPMENUBASE_H
