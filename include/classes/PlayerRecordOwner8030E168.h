// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_PLAYERRECORDOWNER8030E168_H
#define CLASSES_PLAYERRECORDOWNER8030E168_H

#include "game/types.h"

class PlayerRecordOwner8030E168 {
public:
    int bpEntryId0;                          // 0x0: DolfiniSession_Ctor original object access
    int bpEntryId1;                          // 0x4: DolfiniSession_Ctor original object access
    int bpEntryId2;                          // 0x8: DolfiniSession_Ctor original object access
    int bpEntryId3;                          // 0xc: DolfiniSession_Ctor original object access
    int bpEntryId4;                         // 0x10: DolfiniSession_Ctor original object access
    int bpEntryId5;                         // 0x14: DolfiniSession_Ctor original object access
    int bpEntryId6;                         // 0x18: DolfiniSession_Ctor original object access
    int bpEntryId7;                         // 0x1c: DolfiniSession_Ctor original object access
    unsigned int state;                // 0x20: CRoundDolMain_DrawStateUI original object access
    int speedLevel;                         // 0x24: DolfiniSession_Ctor original object access
    int peakSpeedLevel;                         // 0x28: DolfiniSession_Ctor original object access
    int preloadAnimationIndex;                         // 0x2c: DolfiniSession_Ctor original object access
    char pad_030[0x4];                  // 0x30
    int spinInputArmed;                         // 0x34: DolfiniSession_Ctor original object access
    void* roundDol3D;                   // 0x38: DolfiniSession_LoadResources original object access
    void* numberDraw;                       // 0x3c: DolfiniSession_Ctor original object access
    int rewardTable;                         // 0x40: DolfiniSession_Ctor original object access
    int galleryImageTable;                         // 0x44: DolfiniSession_Ctor original object access
    int prizeGalleryTable;                         // 0x48: DolfiniSession_Ctor original object access
    int rewardKind;                         // 0x4c: CRoundDolMain_DrawStateUI original object access
    void* texture0;                     // 0x50: CRoundDolMain_DrawStateUI original object access
    void* texture1;                     // 0x54: CRoundDolMain_DrawStateUI original object access
    char pad_058[0x4c4];                // 0x58
    unsigned char profileAttribute0;              // 0x51c: DolfiniSession_Ctor original object access
    unsigned char profileAttribute1;              // 0x51d: DolfiniSession_Ctor original object access
    unsigned char profileAttribute2;              // 0x51e: DolfiniSession_Ctor original object access
    unsigned char profileAttribute3;              // 0x51f: DolfiniSession_Ctor original object access
    unsigned char profileAttribute4;              // 0x520: DolfiniSession_Ctor original object access
    unsigned char profileAttribute5;              // 0x521: DolfiniSession_Ctor original object access
    char pad_522[0x35e];                // 0x522
    void* dialog;                      // 0x880: CRoundDolMain_DrawStateUI original object access
    void* layout;                       // 0x884: DolfiniSession_LoadResources original object access
    int messageResource;                        // 0x888: DolfiniSession_Ctor original object access
    void* textFactory;                      // 0x88c: DolfiniSession_LoadResources original object access
    int messageDraw;                        // 0x890: DolfiniSession_Ctor original object access
    void* saveMenu;                      // 0x894: CRoundDolMain_DrawStateUI original object access
    int resourcesLoaded;                        // 0x898: CRoundDolMain_DrawStateUI original object access
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define PLAYERRECORDOWNER8030E168_OFF(f, o) typedef char PlayerRecordOwner8030E168_off_##f[((int)&((PlayerRecordOwner8030E168*)0)->f == (o)) ? 1 : -1]
PLAYERRECORDOWNER8030E168_OFF(bpEntryId1, 0x4);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId2, 0x8);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId3, 0xc);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId4, 0x10);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId5, 0x14);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId6, 0x18);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId7, 0x1c);
PLAYERRECORDOWNER8030E168_OFF(state, 0x20);
PLAYERRECORDOWNER8030E168_OFF(speedLevel, 0x24);
PLAYERRECORDOWNER8030E168_OFF(peakSpeedLevel, 0x28);
PLAYERRECORDOWNER8030E168_OFF(preloadAnimationIndex, 0x2c);
PLAYERRECORDOWNER8030E168_OFF(spinInputArmed, 0x34);
PLAYERRECORDOWNER8030E168_OFF(roundDol3D, 0x38);
PLAYERRECORDOWNER8030E168_OFF(numberDraw, 0x3c);
PLAYERRECORDOWNER8030E168_OFF(rewardTable, 0x40);
PLAYERRECORDOWNER8030E168_OFF(galleryImageTable, 0x44);
PLAYERRECORDOWNER8030E168_OFF(prizeGalleryTable, 0x48);
PLAYERRECORDOWNER8030E168_OFF(rewardKind, 0x4c);
PLAYERRECORDOWNER8030E168_OFF(texture0, 0x50);
PLAYERRECORDOWNER8030E168_OFF(texture1, 0x54);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute0, 0x51c);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute1, 0x51d);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute2, 0x51e);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute3, 0x51f);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute4, 0x520);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute5, 0x521);
PLAYERRECORDOWNER8030E168_OFF(dialog, 0x880);
PLAYERRECORDOWNER8030E168_OFF(layout, 0x884);
PLAYERRECORDOWNER8030E168_OFF(messageResource, 0x888);
PLAYERRECORDOWNER8030E168_OFF(textFactory, 0x88c);
PLAYERRECORDOWNER8030E168_OFF(messageDraw, 0x890);
PLAYERRECORDOWNER8030E168_OFF(saveMenu, 0x894);
PLAYERRECORDOWNER8030E168_OFF(resourcesLoaded, 0x898);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId0, 0x0);
#undef PLAYERRECORDOWNER8030E168_OFF

#endif  // CLASSES_PLAYERRECORDOWNER8030E168_H
