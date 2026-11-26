// SavePlayerRecord: 0x828-byte slot payload, owned by SaveData_InitPlayerRecord
// and SaveData_GetCurrentRecord/SaveData_GetSlotRecord in save_data_records_and_defaults.cpp.
#ifndef CLASSES_SAVEPLAYERRECORD_H
#define CLASSES_SAVEPLAYERRECORD_H

struct SavePlayerRecord {
    unsigned short firstNameCodeUnit; // 0x00: SaveData_IsSlotUsed (0x80073DF8) tests this word
    char pad_002[0x2A];
    signed char characterIndex; // 0x2C: CPlayerStatus_InitEquipmentDisplay -> ItemInventory_GetCharacterIdAt
    signed char
        clubSetIndex; // 0x2D: CPlayerStatus_UpdateEquipmentTextures; CPlayerStatus_InitEquipmentDisplay club upgrade lookup
    signed char
        ballIndex; // 0x2E: CPlayerStatus_UpdateEquipmentTextures; inventory categories in SaveData_CopyBallRecord/80074CD4
    signed char
        caddieIndex; // 0x2F: CPlayerStatus_InitEquipmentDisplay -> ItemInventory_GetCaddieIdAt; -1 means no caddie
    char pad_030[0xE4];
    unsigned int ownedHairStyles[2]; // 0x114: CShopProductHair_CheckUnlockAndOwnership (0x80238C4C), index < 0x33
    char pad_11C[0x13C];
    unsigned int unlockedCourses; // 0x258: CCourseSelect_DrawCourseCards course bit tests, bit < 14
    char pad_25C[0x5CC];
};

#define OFF(f, o) typedef char SavePlayerRecord_off_##f[((int)&((SavePlayerRecord*)0)->f == (o)) ? 1 : -1]
OFF(firstNameCodeUnit, 0x00);
OFF(characterIndex, 0x2C);
OFF(clubSetIndex, 0x2D);
OFF(ballIndex, 0x2E);
OFF(caddieIndex, 0x2F);
OFF(ownedHairStyles, 0x114);
OFF(unlockedCourses, 0x258);
#undef OFF
typedef char SavePlayerRecord_size[(sizeof(SavePlayerRecord) == 0x828) ? 1 : -1];
#define AsSavePlayerRecord(p) ((SavePlayerRecord*)(p))

#endif
