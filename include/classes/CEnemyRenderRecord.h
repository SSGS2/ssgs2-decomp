// CEnemyRenderRecord: layout view for 0xA4-byte enemy-render records in character_preview_avatars.cpp.
// Native construction/EH classes stay in the translation unit; this view emits no methods or vtable.
#ifndef CLASSES_CENEMYRENDERRECORD_H
#define CLASSES_CENEMYRENDERRECORD_H

class CEnemyRenderRecord {
public:
    int characterInventoryIndex; // 0x0: CCharacterPreviewRecord_FillFromSaveData writes descriptor index; CCharacterPreviewRecord_BuildItemSelection resolves it
    int hairStyleIndex; // 0x4: CCharacterPreviewRecord_FillFromSaveData sign-extends entry +0x2FE
    unsigned int baseTextureVariant; // 0x8: CCharacterPreviewRecord_BuildItemSelection takes low byte as count
    int caddieInventoryIndex; // 0xC: CCharacterPreviewRecord_FillFromSaveData bounds 0..7 and chooses character resource
    int clubSetInventoryIndex; // 0x10: CCharacterPreviewRecord_FillFromSaveData copies signed src byte +0x2D
    int ballInventoryIndex; // 0x14: CCharacterPreviewRecord_FillFromSaveData copies signed src byte +0x2E
    char pad_18[0x4];
    unsigned char levelIndex; // 0x1C: CCharacterPreviewRecord_FillFromSaveData stores PlayerRank_FindTierForValue result
    char auxiliaryPartInventoryIndex; // 0x1D: CCharacterPreviewRecord_FillFromSaveData copies entry +0x2FF
    unsigned short flags; // 0x1E: 0x4000 marks changed animationName; 0x3000 copies src flag bits
    short partInventoryIndices[24]; // 0x20: CCharacterPreviewRecord_BuildItemSelection resolves 24 signed shorts via ItemInventory_GetPartIdAt
    char pad_50[0x4];
    unsigned int animationName[3]; // 0x54: CCharacterPreviewTable_SetAnimationName edits tagged string; update sends to CAsyncExhibition_PlayFixedNameAnimation
    char pad_60[0x8];
    int equippedItemInventoryIndex0; // 0x68: CCharacterPreviewRecord_FillFromSaveData copies signed source attribute byte
    int equippedItemInventoryIndex1; // 0x6C: CCharacterPreviewRecord_FillFromSaveData copies signed source attribute byte
    int equippedItemInventoryIndex2; // 0x70: CCharacterPreviewRecord_FillFromSaveData copies signed source attribute byte
    int equippedItemInventoryIndex3; // 0x74: CCharacterPreviewRecord_FillFromSaveData copies signed source attribute byte
    int equippedItemInventoryIndex4; // 0x78: CCharacterPreviewRecord_FillFromSaveData copies signed source attribute byte
    int equippedItemInventoryIndex5; // 0x7C: CCharacterPreviewRecord_FillFromSaveData copies signed source attribute byte
    int equippedItemInventoryIndex6; // 0x80: CCharacterPreviewRecord_FillFromSaveData copies signed source attribute byte
    int equippedItemInventoryIndex7; // 0x84: CCharacterPreviewRecord_FillFromSaveData copies signed source attribute byte
    int caddieItemInventoryIndex; // 0x88: CCharacterPreviewRecord_FillFromSaveData copies src byte +0x30
    unsigned char caddieRankBand; // 0x8C: CCharacterPreviewRecord_FillFromSaveData stores PlayerRank_FindThresholdBand result
    char pad_8D[0xE];
    char characterStatLevels[5]; // 0x9B: CCharacterPreviewRecord_FillFromSaveData clears five bytes
    void* renderObject; // 0xA0: CCharacterPreviewRecord_SetupAvatar creates; update dispatches; destructor releases
};

#define OFF(f, o) typedef char CEnemyRenderRecord_off_##f[((int)&((CEnemyRenderRecord*)0)->f == (o)) ? 1 : -1]
OFF(characterInventoryIndex, 0x0);
OFF(hairStyleIndex, 0x4);
OFF(baseTextureVariant, 0x8);
OFF(caddieInventoryIndex, 0xC);
OFF(clubSetInventoryIndex, 0x10);
OFF(ballInventoryIndex, 0x14);
OFF(levelIndex, 0x1C);
OFF(auxiliaryPartInventoryIndex, 0x1D);
OFF(flags, 0x1E);
OFF(partInventoryIndices, 0x20);
OFF(animationName, 0x54);
OFF(equippedItemInventoryIndex0, 0x68);
OFF(equippedItemInventoryIndex1, 0x6C);
OFF(equippedItemInventoryIndex2, 0x70);
OFF(equippedItemInventoryIndex3, 0x74);
OFF(equippedItemInventoryIndex4, 0x78);
OFF(equippedItemInventoryIndex5, 0x7C);
OFF(equippedItemInventoryIndex6, 0x80);
OFF(equippedItemInventoryIndex7, 0x84);
OFF(caddieItemInventoryIndex, 0x88);
OFF(caddieRankBand, 0x8C);
OFF(characterStatLevels, 0x9B);
OFF(renderObject, 0xA0);
#undef OFF

#endif
