# pangya.iff on the Wii

`DATA/files/pangya.iff` is a ZIP archive (PangYa PC does the same) holding 13 tables: Character, Part, Club, ClubSet,
Ball, Item, Caddie, CaddieItem, SetItem, Course, Desc, HairStyle, AuxPart. `ItemInventory_LoadIffArchive`
(0x8018839C) opens it, reads each entry and dispatches by file name to an `ItemInventory_Read*Table` function.

## Byte order and the swap formats

The tables are the PC little-endian format, version 11. The Wii converts each record in place with
`CSwapEndianFormat` (0x80068998 / 0x800693F0) driven by one format string per table (string pool `iffSchemaAndCourseStrings`):

| Token | Meaning |
|---|---|
| `C40`, `C120`, `C280` | byte string of that length (one or several 40-byte names) |
| `C`, `C3`, `C5` | 1, 3 or 5 bytes (no swap) |
| `S`, `S5` | u16, or five u16 |
| `I`, `I2`, `I4`, `I8` | u32 or an array of them |
| `F` | float |
| `M[ a b ]` | one byte holding two bit fields of `a` and `b` bits; the swap re-packs them from PC order (first field in the low bits) to the Wii compiler's order (first field in the high bits) |

The re-pack is confirmed by code: the course variant (see Course) is read as `byte & 0xF` and only the re-packed
value gives the right course names.

## File header (8 bytes)

`u16 recordCount, u16 bindingId, u32 version` (version is 11 in every Wii table). Same as the PC
(retreev/Documentation `iff.md`, pangbox `iff/common.go`).

## Common head of every item record (0x5C bytes)

The Wii head is shorter than the PC one (PC: active, typeId, name, level, icon, prices, shop flags, dates; 0x90
bytes in the 2011 PC client). Format `C40 C40 I I M[ 7 1 ] M[ 3 5 ] S`:

| Off | Size | Field | Evidence |
|---|---|---|---|
| 0x00 | 40 | name: message id, e.g. `MSG_CLUBSETNAME_AI1` | data; shop copies it |
| 0x28 | 40 | icon name, e.g. `club01_01` | data; clubset preview loads it (`src/game/ui/exhibition_model_preview.cpp`) |
| 0x50 | 4 | typeId (table key) | loaders copy it into each entry's key (`tmp.first = body[0x50]`) |
| 0x54 | 4 | price | data (3500, 1400, 300 ...), 0 for starter items |
| 0x58 | 1 | bits: level 7 + flag 1 | first field = required level (6, 11, 16, 21, 26, 31 = start of each PangYa rank); PC "level: low 7 bits" |
| 0x59 | 1 | bits: 3 + 5 | first field nonzero: item starts unlocked (new-save bitsets in 0x80072824 / 0x80073060; hair shop check in `CShopProductHair::CheckUnlockAndOwnership`) |
| 0x5A | 2 | unknown (0, 18 or 166) | |

## Type ids

`kind = typeId >> 26` (`ItemInventory_FindDataById`, same switch as PC `CItemManager::FindCommonItem`):
1 Character 0x04000000, 2 Part 0x08000000, 3 Club 0x0C000000, 4 ClubSet 0x10000000, 5 Ball 0x14000000, 6 Item
0x18000000, 7 Caddie 0x1C000000, 8 CaddieItem 0x20000000, 9 SetItem 0x24000000, 10 Course 0x28000000, 15 HairStyle
0x3C000000, 28 AuxPart 0x70000000.

Part ids: bits 18-25 character index, bits 13-17 body slot (the bit `1 << slot` is set in the part's hide mask in
779 of 782 parts; the 3 exceptions are default parts with both 0x400 and 0x200 set), bit 10 (0x400) default part, bit 9 (0x200) extra sub part, low 8 bits serial. Set item ids: bits 13-20
character index, bits 21-25 a kind (1 = costume set for that character).

## Records (offsets from the record start; the in-memory table entry is a u32 key + the record)

Stat arrays are always in the PangYa order **Power, Control, Accuracy (Impact), Spin, Curve** (PangLib
`StatsStruct`; confirmed by the ring data: `MSG_RINGNAME_POW` = [1,0,0,0,0], `CTR` = [0,1,0,0,0], `SPN` = [0,0,0,1,0]).

**Character (0x11C)**: 0x5C model (`m_def`), 0x84 hair texture, 0xAC base/shirt texture, 0xD4 face/skin texture,
0xFC u32 stats[5] (added to the stat bars, `CCharaEntry::UpdateStatBars`), 0x110 float scale (1.0-1.2 by character; scales the player
model matrix in `src/game/golf/golf_player_actor.cpp`), 0x114 u8 default part count (`ItemInventory_GetDefaultPartIds`), 0x115 u8 unknown,
0x116 u8[5] (equal to the stats in all 8 records; PC: base level per stat), 0x11B u8.

**Part (0x19C)**: 0x5C model, 0x84 textures[3], 0xFC second textures[3], 0x174 u16 stats[5] (summed for the stat
bars), 0x17E u16 slot stats[5], 0x188 u32 sub-part ids[2], 0x190 u32 hide mask (slots this part covers;
shop model refresh in `src/game/ui/shop_screen_selection_and_ownership.cpp`), 0x194 u32 unknown (2048 in 7 parts), 0x198 u8 part type (0 top, 1 bottom, 2 hat,
3 gloves, 4 shoes, 5 default/base; PangLib `PartType`; shop category and filter mask).

**Club (0x88)**: 0x5C model (`club01_01_w`), 0x84 u8 club kind (0 wood, 1 iron, 2 wedge, 3 putter).

**ClubSet (0x8C)**: 0x5C u32 stats[5], 0x70 u32 club ids[4] (wood, iron, wedge, putter; loaded in `src/game/fx/club_trail_and_ring_effects.cpp`),
0x80 u16 second stat array[5] (PC: slot stats or stat limits), 0x8A u16.

**Ball (0x2C0)**: 0x5C effect files[7] (`.seq`), 0x174 effect bones[7], 0x28C model (`ball_low`; the preview loads
it), 0x2B4 u32, 0x2B8 u32, 0x2BC u16 (100 for most balls), 0x2BE u16.

**Item (0x60)**: two u16 after the head (0 in all records).

**Caddie (0x90)**: 0x5C model (`bon`, `pipin`), 0x84 u16 stat bonus[5] (added to the stat bars).

**CaddieItem (0x88)**: 0x5C model (`Bon_001.pet`), 0x84 u16, 0x86 u16.

**SetItem (0x80)**: 0x5C u32 item ids[8], 0x7C u8 item count, 0x7D u8, 0x7E u16.

**Course (0x104)**: 0x5C map name (`blue`; `%s_%02d.gbin`), 0x84 ambient sound (`sea_wave`), 0xAC property xml,
0xD4 sky effect (`star.seq`), 0xFC float pang rate (1.1 / 1.3), 0x100 bits 4+4 (second field = variant of a shared
map: blue 1 = blue_lagoon, 2 = blue_water; wind 1 = sepia_wind, 2 = wind_hill; `SharedDoc_SelectCourse`). The loader
appends three built-in courses (TUTORIAL, TEST, RANDOM); `ItemInventory_GetArchiveCourseCount` returns count - 3.

**HairStyle (0x60)**: 0x5C u8 colour index, 0x5D u8 character index.

**AuxPart / rings (0x70)**: 0x5C u8 stats[5], 0x61 u8 slot stats[5], 0x66 u16, 0x68/0x6A/0x6C u16 rates in percent
for properties 1-3, 0x6E s8 added to property 0 (`CSharedDoc_ComputeAuxPartModifier`, the PC `CSharedDoc::GetAuxPartProperty`).

**Desc (0x2C)**: 0x00 message id (`MSG_CHRDESC_KEN`), 0x28 u32 typeId of the described item (the key).
