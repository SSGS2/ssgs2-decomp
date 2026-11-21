// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOINOPENITEMDATA_H
#define CLASSES_CCOINOPENITEMDATA_H

#include "game/types.h"

class CCoinOpenItemData {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* itemFile;                        // 0x4: 1 use in CTourMain_InitItemMap
    void* messageFile;                        // 0x8: 2 uses in CTourMain_InitItemMap
    void* cameraController;                        // 0xc: 5 uses in CTourMain_InitItemMap
    char pad_010[0x24];                 // 0x10
    void* mapData;                       // 0x34: 1 use in CTourMain_InitItemMap
    void* coinItemData;                       // 0x38: 1 use in CTourMain_InitItemMap
    char pad_03C[0x8];                  // 0x3c
    void* numberDraw;                       // 0x44: 2 uses in CTourMain_InitItemMap
    void* menuContext;                       // 0x48: 1 use in CTourMain_InitItemMap
    char pad_04C[0x4];                  // 0x4c
    void* selectionRecord;                       // 0x50: 1 use in CTourMain_InitItemMap
    char pad_054[0x18];                 // 0x54
    void* modeGroup;                       // 0x6c: 1 use in CTourMain_InitItemMap
    void* itemGroup;                       // 0x70: 3 uses in CTourMain_InitItemMap
    void* numberGroup;                       // 0x74: 2 uses in CTourMain_InitItemMap
    char pad_078[0x18];                 // 0x78
    unsigned char mode;               // 0x90: 2 uses in CTourMain_InitItemMap
    char pad_091[0x7];                  // 0x91
    short initialSelection;                       // 0x98: 1 use in CTourMain_InitItemMap

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10, 0x88
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOINOPENITEMDATA_OFF(f, o) typedef char CCoinOpenItemData_off_##f[((int)&((CCoinOpenItemData*)0)->f == (o)) ? 1 : -1]
CCOINOPENITEMDATA_OFF(itemFile, 0x4);
CCOINOPENITEMDATA_OFF(messageFile, 0x8);
CCOINOPENITEMDATA_OFF(cameraController, 0xc);
CCOINOPENITEMDATA_OFF(mapData, 0x34);
CCOINOPENITEMDATA_OFF(coinItemData, 0x38);
CCOINOPENITEMDATA_OFF(numberDraw, 0x44);
CCOINOPENITEMDATA_OFF(menuContext, 0x48);
CCOINOPENITEMDATA_OFF(selectionRecord, 0x50);
CCOINOPENITEMDATA_OFF(modeGroup, 0x6c);
CCOINOPENITEMDATA_OFF(itemGroup, 0x70);
CCOINOPENITEMDATA_OFF(numberGroup, 0x74);
CCOINOPENITEMDATA_OFF(mode, 0x90);
CCOINOPENITEMDATA_OFF(initialSelection, 0x98);
#undef CCOINOPENITEMDATA_OFF

#endif  // CLASSES_CCOINOPENITEMDATA_H
