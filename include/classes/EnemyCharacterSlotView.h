// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CHARASLOT8024745C_H
#define CLASSES_CHARASLOT8024745C_H

#include "game/types.h"

class EnemyCharacterSlotView {
public:
    int characterInventoryIndex;                         // 0x0: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    int hairStyle;                          // 0x4: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    int baseVariant;                          // 0x8: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    int characterIndex;                          // 0xc: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    int clubSetInventoryIndex;                         // 0x10: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    int ballInventoryIndex;                         // 0x14: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    void* context;                      // 0x18: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    char levelIndex;                        // 0x1c: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    char pad_01D[0x1];                  // 0x1d
    unsigned short flags;               // 0x1e: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    short partIndices[24];              // 0x20: EnemyData_BuildCharacterSlot stores ItemInventory_FindPartIndex results
    char pad_050[0x18];
    int auxiliaryValues[8];                      // 0x68: EnemyData_BuildCharacterSlot clears eight words, copies three record attributes
    int caddieInventoryIndex;                         // 0x88: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    unsigned char rankTier;               // 0x8c: EnemyData_BuildCharacterSlot: Table_GetEntry key, caller context, flags |= 0x2000/0x8000 and &= ~0x1000
    char packedAttributes[14];                    // 0x8D: EnemyData_BuildCharacterSlot copies record +0x45 bytes
    char characterAttributes[5];                     // 0x9B: EnemyData_BuildCharacterSlot copies record +0x54 bytes
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CHARASLOT8024745C_OFF(f, o) typedef char CharaSlot8024745C_off_##f[((int)&((EnemyCharacterSlotView*)0)->f == (o)) ? 1 : -1]
CHARASLOT8024745C_OFF(hairStyle, 0x4);
CHARASLOT8024745C_OFF(baseVariant, 0x8);
CHARASLOT8024745C_OFF(characterIndex, 0xc);
CHARASLOT8024745C_OFF(clubSetInventoryIndex, 0x10);
CHARASLOT8024745C_OFF(ballInventoryIndex, 0x14);
CHARASLOT8024745C_OFF(context, 0x18);
CHARASLOT8024745C_OFF(levelIndex, 0x1c);
CHARASLOT8024745C_OFF(flags, 0x1e);
CHARASLOT8024745C_OFF(caddieInventoryIndex, 0x88);
CHARASLOT8024745C_OFF(rankTier, 0x8c);
CHARASLOT8024745C_OFF(characterInventoryIndex, 0x0);
CHARASLOT8024745C_OFF(partIndices, 0x20);
CHARASLOT8024745C_OFF(auxiliaryValues, 0x68);
CHARASLOT8024745C_OFF(packedAttributes, 0x8D);
CHARASLOT8024745C_OFF(characterAttributes, 0x9B);
#undef CHARASLOT8024745C_OFF

#endif  // CLASSES_CHARASLOT8024745C_H
