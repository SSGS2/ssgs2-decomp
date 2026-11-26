// CCharacterPreviewRecord: one player's character-preview record.
//
// It holds the inventory indices of the character, caddie and equipment to show,
// the animation name requested for the avatar, and the renderer that displays
// the avatar. The animation name is a tagged string: bit 31 of the first word
// means the text is heap-allocated and its pointer is in the third word;
// otherwise the text is stored inline starting after the first byte.
#ifndef CLASSES_CCHARACTERPREVIEWRECORD_H
#define CLASSES_CCHARACTERPREVIEWRECORD_H

#include "game/types.h"

class CCharacterPreviewRecord {
public:
    // Copy the saved selections and outfit into this record without creating a
    // renderer.
    void FillFromSaveData(int characterIndex, void* saveRecord);

    // Create or refresh the avatar and apply its equipment. Returns 0 when the
    // character or the avatar cannot be resolved.
    int SetupAvatar(void* viewport, int suppliedSelection, float scale);

    static int BuildItemSelection(void* descriptor, void* dest);
    static void* CopyPreservingRenderer(void* obj, void* src);

    int characterInventoryIndex;     // 0x00
    int hairStyleIndex;              // 0x04
    unsigned int baseTextureVariant; // 0x08
    int caddieInventoryIndex;        // 0x0C
    int clubSetInventoryIndex;       // 0x10
    int ballInventoryIndex;          // 0x14
    char pad_18[0x4];
    unsigned char levelIndex;         // 0x1C
    char auxiliaryPartInventoryIndex; // 0x1D
    unsigned short flags;             // 0x1E  0x4000: animation name waits to be applied
    short partInventoryIndices[24];   // 0x20
    char pad_50[0x4];
    unsigned int animationName[3]; // 0x54
    char pad_60[0x8];
    int equippedItemInventoryIndex0; // 0x68
    int equippedItemInventoryIndex1; // 0x6C
    int equippedItemInventoryIndex2; // 0x70
    int equippedItemInventoryIndex3; // 0x74
    int equippedItemInventoryIndex4; // 0x78
    int equippedItemInventoryIndex5; // 0x7C
    int equippedItemInventoryIndex6; // 0x80
    int equippedItemInventoryIndex7; // 0x84
    int caddieItemInventoryIndex;    // 0x88
    unsigned char caddieRankBand;    // 0x8C
    char pad_8D[0xE];
    char characterStatLevels[5]; // 0x9B
    void* renderObject;          // 0xA0  the avatar, owned by this record
};

#define CCHARACTERPREVIEWRECORD_OFF(f, o) \
    typedef char CCharacterPreviewRecord_off_##f[((int)&((CCharacterPreviewRecord*)0)->f == (o)) ? 1 : -1]
CCHARACTERPREVIEWRECORD_OFF(characterInventoryIndex, 0x0);
CCHARACTERPREVIEWRECORD_OFF(flags, 0x1E);
CCHARACTERPREVIEWRECORD_OFF(partInventoryIndices, 0x20);
CCHARACTERPREVIEWRECORD_OFF(animationName, 0x54);
CCHARACTERPREVIEWRECORD_OFF(caddieRankBand, 0x8C);
CCHARACTERPREVIEWRECORD_OFF(renderObject, 0xA0);
#undef CCHARACTERPREVIEWRECORD_OFF
typedef char CCharacterPreviewRecord_size[(sizeof(CCharacterPreviewRecord) == 0xA4) ? 1 : -1];

#endif // CLASSES_CCHARACTERPREVIEWRECORD_H
