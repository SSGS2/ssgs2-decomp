// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCHARANUM_H
#define CLASSES_CCHARANUM_H

#include "game/types.h"

class CCharaNum {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int selectedPlayerCount;                         // 0x18: 5 uses in CCharaNum_UpdatePlayerCountSelection, CCharaNum_Ctor
    int selectionComplete;                         // 0x1c: 2 uses in CCharaNum_UpdatePlayerCountSelection, CCharaNum_Ctor
    void* layout;                       // 0x20: 22 uses in CCharaNum_UpdatePlayerCountSelection, CCharaNum_Ctor, CCharaNum_dtor...
    int bpEntryId;                         // 0x24: 5 uses in CCharaNum_dtor, CCharaNum_Draw, CCharaNum_Ctor
    int selectionMade;                         // 0x28: 5 uses in CCharaNum_UpdatePlayerCountSelection, CCharaNum_Ctor
    int selectionFrames;                         // 0x2c: 3 uses in CCharaNum_UpdatePlayerCountSelection, CCharaNum_Ctor
    int state;                         // 0x30: 4 uses in CCharaNum_UpdatePlayerCountSelection, CCharaNum_Ctor
    void* packTextureResource;                       // 0x34: 8 uses in CCharaNum_Ctor, CCharaNum_dtor, CCharaNum_StartCharaCountMenu...
    void* bpTexture;                       // 0x38: 7 uses in CCharaNum_Draw, CCharaNum_Ctor, CCharaNum_dtor...
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCHARANUM_OFF(f, o) typedef char CCharaNum_off_##f[((int)&((CCharaNum*)0)->f == (o)) ? 1 : -1]
CCHARANUM_OFF(selectedPlayerCount, 0x18);
CCHARANUM_OFF(selectionComplete, 0x1c);
CCHARANUM_OFF(layout, 0x20);
CCHARANUM_OFF(bpEntryId, 0x24);
CCHARANUM_OFF(selectionMade, 0x28);
CCHARANUM_OFF(selectionFrames, 0x2c);
CCHARANUM_OFF(state, 0x30);
CCHARANUM_OFF(packTextureResource, 0x34);
CCHARANUM_OFF(bpTexture, 0x38);
#undef CCHARANUM_OFF

#endif  // CLASSES_CCHARANUM_H
