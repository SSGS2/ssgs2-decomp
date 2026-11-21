#include <decomp/cw_bitfields.h>
// Wii pangya.iff v11 records, after CSwapEndianFormat converts them in place.
// Evidence and the on-disk swap formats: docs/iff-formats.md.
#ifndef IFF_IFFSTRUCT_H
#define IFF_IFFSTRUCT_H

// A few byte-oriented string/copy loops need pointer-addition expressions to
// retain CodeWarrior's instruction scheduling. Derive their offsets from fields.
#include <decomp/offset_of.h>
#define IFF_OFFSET(type, field) ((int)DECOMP_OFFSETOF(type, field))

struct IffLevelBits {
    CW_BITS2(
        unsigned char level : 7,
        unsigned char unk_58 : 1)
};

struct IffItemCommon {
    char name[40];                     // 0x00: message id
    char icon[40];                     // 0x28: icon resource name
    unsigned int typeId;               // 0x50: record key; kind = typeId >> 26
    int price;                        // 0x54
    unsigned char levelFlags;         // 0x58: required level = levelFlags >> 1 (Wii bit order)
    unsigned char shopFlags;          // 0x59: nonzero upper three bits mean initially unlocked
    unsigned short commonFlagWord;
};

namespace IFF_STRUCT {
// Every stat array uses Power, Control, Accuracy, Spin, Curve, in that order.
struct sChar {
    IffItemCommon common;
    char model[40];                    // 0x5C
    char hairTexture[40];              // 0x84
    char baseTexture[40];              // 0xAC
    char faceTexture[40];              // 0xD4
    unsigned int stats[5];             // 0xFC
    float scale;                      // 0x110
    unsigned char defaultPartCount;    // 0x114
    unsigned char optionalPartCount;
    unsigned char baseStats[5];        // 0x116
    unsigned char unk_11B;
};

struct sPart {
    IffItemCommon common;
    char model[40];                    // 0x5C
    char textures[3][40];              // 0x84
    char secondTextures[3][40];        // 0xFC
    unsigned short stats[5];           // 0x174
    unsigned short slotStats[5];       // 0x17E
    unsigned int subPartIds[2];        // 0x188
    unsigned int hideMask;             // 0x190: body slots covered by this part
    unsigned int modelHideMask;
    unsigned char partType;            // 0x198: top, bottom, hat, gloves, shoes, base
    unsigned char unk_199[3];
};

struct sClub {
    IffItemCommon common;
    char model[40];                    // 0x5C
    unsigned char clubKind;            // 0x84: wood, iron, wedge, putter
    unsigned char unk_85[3];
};

struct sClubSet {
    IffItemCommon common;
    unsigned int stats[5];             // 0x5C
    unsigned int clubIds[4];           // 0x70: wood, iron, wedge, putter
    unsigned short slotStats[5];       // 0x80: second stat array
    unsigned short unk_8A;
};

struct sBall {
    IffItemCommon common;
    char effectFiles[7][40];           // 0x5C
    char effectBones[7][40];           // 0x174
    char model[40];                    // 0x28C
    unsigned int unk_2B4;
    unsigned int unk_2B8;
    unsigned short unk_2BC;
    unsigned short unk_2BE;
};

struct sItem {
    IffItemCommon common;
    unsigned short failureChancePercent;
    unsigned short unk_5E;
};

struct sCaddie {
    IffItemCommon common;
    char model[40];                    // 0x5C
    unsigned short stats[5];           // 0x84: stat bonuses
    unsigned short unk_8E;
};

struct sCadItem {
    IffItemCommon common;
    char model[40];                    // 0x5C
    unsigned short initialPowerShotMeter;
    unsigned short unk_86;
};

struct sSetItem {
    IffItemCommon common;
    unsigned int itemIds[8];           // 0x5C
    unsigned char itemCount;           // 0x7C
    unsigned char hairStyleInventoryIndex;
    unsigned short unk_7E;
};

struct sCourse {
    IffItemCommon common;
    char mapName[40];                  // 0x5C
    char ambientSound[40];             // 0x84
    char propertyFile[40];             // 0xAC
    char skyEffect[40];                // 0xD4
    float pangRate;                    // 0xFC
    unsigned char courseFlags;        // 0x100: low nibble is shared-map variant
    unsigned char unk_101[3];
};

// Desc.iff is the exception: no common item header.
struct sDesc {
    char name[40];                     // 0x00: description message id
    unsigned int typeId;               // 0x28: id of the described item
};

struct sHairStyle {
    IffItemCommon common;
    unsigned char colourIndex;         // 0x5C
    unsigned char characterIndex;      // 0x5D
    unsigned short unk_5E;
};

struct sAuxPart {
    IffItemCommon common;
    unsigned char stats[5];            // 0x5C
    unsigned char slotStats[5];        // 0x61
    unsigned short unk_66;
    unsigned short propertyRates[3];   // 0x68: percentages for properties 1..3
    signed char property0Bonus;        // 0x6E
    unsigned char unk_6F;
};
} // namespace IFF_STRUCT

// Loaders copy complete records as words, including padding and float bit patterns.
// Keep that copy representation alongside the semantic fields.
template <class Record> union IffRecordStorage {
    Record value;
    unsigned int words[sizeof(Record) / sizeof(unsigned int)];
};

// In memory each record is preceded by its lookup key. No ownership or code.
template <class Record> struct IffTableEntry {
    unsigned int key;
    Record record;
};

template <class Record> struct IffTable {
    IffTableEntry<Record>* data;
    unsigned int size;
    unsigned int capacity;
};

typedef IffTableEntry<IFF_STRUCT::sChar> CharacterTableEntry;
typedef IffTableEntry<IFF_STRUCT::sPart> PartTableEntry;
typedef IffTableEntry<IFF_STRUCT::sClub> ClubTableEntry;
typedef IffTableEntry<IFF_STRUCT::sClubSet> ClubSetTableEntry;
typedef IffTableEntry<IFF_STRUCT::sBall> BallTableEntry;
typedef IffTableEntry<IFF_STRUCT::sItem> ItemTableEntry;
typedef IffTableEntry<IFF_STRUCT::sCaddie> CaddieTableEntry;
typedef IffTableEntry<IFF_STRUCT::sCadItem> CaddieItemTableEntry;
typedef IffTableEntry<IFF_STRUCT::sSetItem> SetItemTableEntry;
typedef IffTableEntry<IFF_STRUCT::sCourse> CourseTableEntry;
typedef IffTableEntry<IFF_STRUCT::sDesc> DescTableEntry;
typedef IffTableEntry<IFF_STRUCT::sHairStyle> HairStyleTableEntry;
typedef IffTableEntry<IFF_STRUCT::sAuxPart> AuxPartTableEntry;

#define IFF_SIZE(t, n) typedef char iff_size_##t[(sizeof(t) == (n)) ? 1 : -1]
IFF_SIZE(IffItemCommon, 0x5C);
namespace IFF_STRUCT {
IFF_SIZE(sChar, 0x11C);
IFF_SIZE(sPart, 0x19C);
IFF_SIZE(sClub, 0x88);
IFF_SIZE(sClubSet, 0x8C);
IFF_SIZE(sBall, 0x2C0);
IFF_SIZE(sItem, 0x60);
IFF_SIZE(sCaddie, 0x90);
IFF_SIZE(sCadItem, 0x88);
IFF_SIZE(sSetItem, 0x80);
IFF_SIZE(sCourse, 0x104);
IFF_SIZE(sDesc, 0x2C);
IFF_SIZE(sHairStyle, 0x60);
IFF_SIZE(sAuxPart, 0x70);
}
IFF_SIZE(CharacterTableEntry, 0x120);
IFF_SIZE(PartTableEntry, 0x1A0);
IFF_SIZE(ClubTableEntry, 0x8C);
IFF_SIZE(ClubSetTableEntry, 0x90);
IFF_SIZE(BallTableEntry, 0x2C4);
IFF_SIZE(ItemTableEntry, 0x64);
IFF_SIZE(CaddieTableEntry, 0x94);
IFF_SIZE(CaddieItemTableEntry, 0x8C);
IFF_SIZE(SetItemTableEntry, 0x84);
IFF_SIZE(CourseTableEntry, 0x108);
IFF_SIZE(DescTableEntry, 0x30);
IFF_SIZE(HairStyleTableEntry, 0x64);
IFF_SIZE(AuxPartTableEntry, 0x74);
#undef IFF_SIZE

// Check field offsets as well as record sizes, including the float and byte tails.
#define IFF_FIELD(type, field, off) typedef char iff_##type##_##field[(IFF_OFFSET(type, field) == (off)) ? 1 : -1]
IFF_FIELD(IffItemCommon, name, 0x00);
IFF_FIELD(IffItemCommon, icon, 0x28);
IFF_FIELD(IffItemCommon, typeId, 0x50);
IFF_FIELD(IffItemCommon, price, 0x54);
IFF_FIELD(IffItemCommon, levelFlags, 0x58);
IFF_FIELD(IffItemCommon, shopFlags, 0x59);
IFF_FIELD(IffItemCommon, commonFlagWord, 0x5A);
namespace IFF_STRUCT {
IFF_FIELD(sChar, model, 0x5C);
IFF_FIELD(sChar, hairTexture, 0x84);
IFF_FIELD(sChar, baseTexture, 0xAC);
IFF_FIELD(sChar, faceTexture, 0xD4);
IFF_FIELD(sChar, stats, 0xFC);
IFF_FIELD(sChar, scale, 0x110);
IFF_FIELD(sChar, defaultPartCount, 0x114);
IFF_FIELD(sChar, baseStats, 0x116);
IFF_FIELD(sPart, model, 0x5C);
IFF_FIELD(sPart, textures, 0x84);
IFF_FIELD(sPart, secondTextures, 0xFC);
IFF_FIELD(sPart, stats, 0x174);
IFF_FIELD(sPart, slotStats, 0x17E);
IFF_FIELD(sPart, subPartIds, 0x188);
IFF_FIELD(sPart, hideMask, 0x190);
IFF_FIELD(sPart, modelHideMask, 0x194);
IFF_FIELD(sPart, partType, 0x198);
IFF_FIELD(sClub, model, 0x5C);
IFF_FIELD(sClub, clubKind, 0x84);
IFF_FIELD(sClubSet, stats, 0x5C);
IFF_FIELD(sClubSet, clubIds, 0x70);
IFF_FIELD(sClubSet, slotStats, 0x80);
IFF_FIELD(sBall, effectFiles, 0x5C);
IFF_FIELD(sBall, effectBones, 0x174);
IFF_FIELD(sBall, model, 0x28C);
IFF_FIELD(sBall, unk_2B4, 0x2B4);
IFF_FIELD(sBall, unk_2B8, 0x2B8);
IFF_FIELD(sBall, unk_2BC, 0x2BC);
IFF_FIELD(sItem, failureChancePercent, 0x5C);
IFF_FIELD(sCaddie, model, 0x5C);
IFF_FIELD(sCaddie, stats, 0x84);
IFF_FIELD(sCadItem, model, 0x5C);
IFF_FIELD(sCadItem, initialPowerShotMeter, 0x84);
IFF_FIELD(sSetItem, itemIds, 0x5C);
IFF_FIELD(sSetItem, itemCount, 0x7C);
IFF_FIELD(sCourse, mapName, 0x5C);
IFF_FIELD(sCourse, ambientSound, 0x84);
IFF_FIELD(sCourse, propertyFile, 0xAC);
IFF_FIELD(sCourse, skyEffect, 0xD4);
IFF_FIELD(sCourse, pangRate, 0xFC);
IFF_FIELD(sCourse, courseFlags, 0x100);
IFF_FIELD(sDesc, name, 0x00);
IFF_FIELD(sDesc, typeId, 0x28);
IFF_FIELD(sHairStyle, colourIndex, 0x5C);
IFF_FIELD(sHairStyle, characterIndex, 0x5D);
IFF_FIELD(sAuxPart, stats, 0x5C);
IFF_FIELD(sAuxPart, slotStats, 0x61);
IFF_FIELD(sAuxPart, propertyRates, 0x68);
IFF_FIELD(sAuxPart, property0Bonus, 0x6E);
}
IFF_FIELD(CharacterTableEntry, record, 0x04);
typedef IffTable<IFF_STRUCT::sChar> IffCharacterTable;
typedef char iff_table_size[(sizeof(IffCharacterTable) == 0x0C) ? 1 : -1];
IFF_FIELD(IffCharacterTable, data, 0x00);
IFF_FIELD(IffCharacterTable, size, 0x04);
IFF_FIELD(IffCharacterTable, capacity, 0x08);
#undef IFF_FIELD

#endif
