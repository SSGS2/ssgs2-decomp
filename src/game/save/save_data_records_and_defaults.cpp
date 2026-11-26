// User Data: four saved player slots and the shared record used without a slot.
// Modding lookup (Controls / Button control): SaveData_InitBlankRecord clears
// kSavedControlAndAudioOptions at record +0x4E8 with kKeepAudioAndOtherOptions.
// kSavedButtonControlBit selects Button; kSavedLeftHandedBit selects Swing Lefty
// only when Button is clear. Stock initialization selects Swing Righty. A later
// Button-default mod must change that policy and Options Default together
// (COptionDialog_UpdateAndProcessInput, option_dialog_control_mode_audio.cpp), preserving audio bits and save widths.
// This code keeps stock behavior; it does not change the selected control.
// Record copies (SaveData_SetCurrentRecord / SaveRecord_CopyCtor) carry these
// flags unchanged; existing User Data is not reset by a new default.
// Equipment/course defaults: SaveData_InitBlankRecord and
// SaveData_InitializeDefaultEquipmentRecord use IFF row indices, not model paths.
// Model/texture replacement belongs to the resource loaders, not these indices.
// Layouts: SavePlayerRecord.h plus the checked local PlayerRecordView below.
// No render resolution, aspect ratio or frame timing is selected in this unit.
#pragma push
#include "iff/IffStruct.h"
#include "classes/SavePlayerRecord.h" // SaveData_InitPlayerRecord (0x80075F38): owned slot layout
#include "classes/CSharedDoc.h"
// Borrow the shared-document fields; no pointer adjustment or ownership transfer.
static inline CSharedDoc* AsCSharedDoc(void* p) {
    return (CSharedDoc*)p;
}
#include "classes/CItemManager.h"

// Save slots are 0x828 bytes. This view and SavePlayerRecord describe the same
// storage; the fuller inventory/tour schemas here do not add a second owner.
// Part-bank policy (callers/consumers, not a guessed "purchased" distinction):
// +03C is the primary grant tier; +0AC is the secondary grant tier. In
// save_rewards_item_unlock_flags.cpp, SaveRewards_SetPartFlags(index,0) sets primary and the notification
// bit; a nonzero secondaryFlag sets BOTH banks and the notification bit. It
// returns unchanged when the requested tier is already set. Thus secondary
// implies primary for these grants, but primary does not imply secondary.
// SaveData_TestPartFlag selects +03C for zero and +0AC for ANY nonzero selector;
// CShopProductParts::AcceptsPart and CShopProductSetItem's member check consume
// the selected tier to admit inventory rows. This selector is independent of
// useSharedRecord/currentSlot, which choose the record containing BOTH banks.
// InitDefaultInventoryRecord (save_data_records_and_inventory_init.cpp) seeds primary from shopFlags bits 5..7,
// then primary/secondary starter rows separately. Save/copy preserves both.
// No source evidence assigns a purchase/payment meaning to the tier: mods can
// choose the required tier from this policy without inventing such a meaning.
enum SaveLimits {
    kSaveSlotCount = 4,
    kCharacterCount = 8,
    kOutfitPartCount = 24,
    kPartFlagCount = 782,
    kCourseFlagCount = 14,
    kNoInventoryIndex = -1,
    kItemKindShift = 26,
    kPartItemKind = 2,
    kCharacterIdShift = 18,
    kCharacterIdMask = 0xFF,
    kUnavailableItemFlag = 0x02000000,
    // 0x1A0 is the BYTE size of characters[8] (8 * 0x34), used by the original as an element-count bound;
    // real records hold kCharacterCount (8) entries. Kept at the original value so callers behave identically.
    kCharacterLookupBound = 0x1A0,
    // 0x1E is the BYTE size of clubSets[15] (15 * 2), used by the original as an element-count bound.
    kClubSetLookupBound = 0x1E
};
struct SavedCharacterEquipment {
    unsigned char flags;
    unsigned char alignmentPadding;
    short partIndices[kOutfitPartCount];
    signed char hairStyleIndex;
    signed char auxPartIndex;
};
struct SavedCaddieEquipment {
    unsigned char flags;
    char reserved[3];
    unsigned int value;
};
struct SavedClubSetEquipment {
    unsigned char flags;
    signed char upgradeLevel;
};
// Eight serialized bytes per course. Initialization proves the widths and empty
// encodings below; it does not establish the gameplay meaning of +0 or +6.
struct SavedCourseRecord {
    unsigned int clearedWord;
    unsigned short scoreOrEmpty; // 0x7FFF means no recorded score at initialization.
    signed char indexOrNone;     // -1 in an empty record; index domain unresolved.
    unsigned char padding;       // left as cleared by Memory_Clear
};
typedef char SavedCourseRecord_size[sizeof(SavedCourseRecord) == 8 ? 1 : -1];
// Tour-mode transfer, relative to record+4EC, big-endian save representation:
// +000..32D = 11 area records of 0x4A bytes (NOT 407 independent progress values).
// Each area: +0 flags byte, +1 padding, +2 signed currentNode (-1 or 0..34),
// +4 u16 nodes[35], a 7-row x 5-column grid. Node bits: 0010 numeric reward
// counted, 0020/0040 down/up edge, 0080/0100 right/left edge, 0200 rewards
// applied, 0400 pending, 8000 completed. Area flag 80 admits the area.
// +32E signed areaOrder[8] maps progression slots to physical areas 0..10;
// +336 signed currentArea indexes that order (0..7 when used, -1 when empty).
// +337 padding, +338 u16 rewardCount, +33A flags (20 all-areas-cleared),
// +33B tail padding. These contain AREA indices, not character equipment.
// See TourModeSave and
// TourMap_CommitNodeCompletionAndRewards in tour_main_task.cpp for the producer/consumer;
// tour selection reads the active record snapshot, completion publishes it with
// SaveData_SetModeRecords. Setter validates none of these domains, owns no
// pointers, copies 32E bytes + 9 bytes + 2 bytes + 1 byte, and skips both pads.
struct SavedTourAreaProgress {
    unsigned char flags, reserved01;
    short currentNode;
    unsigned short nodes[35];
};
typedef char SavedTourAreaProgress_size[sizeof(SavedTourAreaProgress) == 0x4A ? 1 : -1];
struct ModeRecordTransferView {
    SavedTourAreaProgress areas[11];
    signed char areaOrder[8];
    signed char currentArea;
    unsigned char alignmentPadding;
    unsigned short rewardCount;
    unsigned char tourFlags;
};
typedef char ModeRecordTransferView_size[(sizeof(ModeRecordTransferView) == 0x33C) ? 1 : -1];
// Serialized banks between inventory and course flags. Only bank shapes are
// established for +140..+1CC; do not interpret those as gameplay statistics.
// save_data_records_and_inventory_init.cpp initializes +144 with the part-sized bitset helper and +1A8 with
// the six-word helper; +1D0 is the checked 45-bit bank. Counters/scalars after
// +1D8 and the 64-byte table have no proven per-entry meanings.
// These banks are initialized and copied as serialized storage, including words
// that may contain float encodings. Shape alone supplies no safe tuning rule:
// preserve their bit patterns and widths when editing equipment/course defaults.
// New gameplay meaning needs evidence from the consumer, not a numeric default.
struct SavedSecondaryBanks {
    unsigned int unresolvedFlagWord;        // +140
    unsigned int partSizedFlags[25];        // +144: bitset storage, meaning unverified
    unsigned int sixWordFlags[6];           // +1A8
    unsigned int unresolvedSmallFlags[4];   // +1C0..1CC
    unsigned int checked45BitFlags[2];      // +1D0
    unsigned int unresolvedTrailingFlag;    // +1D8
    unsigned int unresolvedScalarWords[15]; // +1DC..214; float/word mix
    unsigned char defaultByteTable[64];     // +218: filled with byte value 8
};
typedef char SavedSecondaryBanks_size[(sizeof(SavedSecondaryBanks) == 0x118) ? 1 : -1];
struct PlayerRecordView {
    unsigned short firstNameCodeUnit; // First name code unit, tested for nonzero to determine occupancy.
    char reservedHeader[0x2A];
    signed char characterIndex, clubSetIndex, ballIndex, caddieIndex;
    signed char caddieItemIndex;
    signed char equippedItemIndices[8];
    char selectionPadding[3];
    unsigned int primaryPartFlags[25];   // 0x03C: 782 bits, plus unused high bits
    unsigned int primaryCaddieItemFlags; // 0x0A0
    unsigned int reservedFlags[2];
    unsigned int alternatePartFlags[25];   // 0x0AC: selected by the test parameter
    unsigned int alternateCaddieItemFlags; // 0x110
    unsigned int ownedHairStyles[2];       // 0x114: shared SavePlayerRecord layout
    short itemCounts[12];                  // 0x11C
    short reservedItemCount;
    signed char auxPartFlags[7]; // 0x136
    char itemPadding[3];
    SavedSecondaryBanks secondaryBanks;
    unsigned int unlockedCourses;                        // 0x258
    SavedCourseRecord courseRecords[kCourseFlagCount];   // 0x25C: fourteen 8-byte records
    SavedCharacterEquipment characters[kCharacterCount]; // 0x2CC
    SavedCaddieEquipment caddies[8];                     // 0x46C
    SavedClubSetEquipment clubSets[15];                  // 0x4AC
    unsigned char balls[11];                             // 0x4CA
    char settingsPadding[3];
    unsigned char systemSettings[0x14]; // 0x4D8
    unsigned char modeRecords[0x33C];   // 0x4EC
};
struct SaveRecordSetView {
    unsigned int currentSlot;
    unsigned char slotChanged;
    unsigned char useSharedRecord;
    unsigned char sharedInitializationFlags;
    unsigned char padding;
    PlayerRecordView slots[kSaveSlotCount]; // 0x008
    PlayerRecordView sharedRecord;          // 0x20A8
};

// Access contract: saveData points to the complete record set, while equipment/
// source/record arguments usually point to one value record, not a record-set header.
// All returned record/inventory pointers are borrowed and mutable; no accessor
// allocates or transfers ownership. Active-record access chooses sharedRecord when
// useSharedRecord!=0, otherwise slots[currentSlot]; currentSlot must remain 0..3.
// Inventory selections are signed row indices (-1 means none), not IFF type IDs.
// Save flag bytes use 0x80 and 0x40 independently; only established meanings are
// described per operation. The two 782-bit banks are primary/secondary grant
// tiers as traced above; alternate is the retained local spelling for secondary.
// Bit references use LSB-first numbering within 32-bit words (word=i>>5, bit=i&31).
// Copy operations preserve field widths and padding gaps: do not replace them
// with a whole-struct assignment/memcpy without rechecking the binary and save format.
enum SaveEncoding {
    kDefaultVolumePercent = 80,
    kDefaultUnlockedCourseCount = 6,
    kBitsPerFlagWord = 32,
    kFlagWordIndexShift = 5,
    kFlagBitIndexMask = 31,
    kChecked45FlagCount = 45,
    kEquipmentInitialFlagMask = 0xC0,
    kEmptyPartFillWord = 0xFFFF,
    kTrailingRecordScratchOffset = 0x28D0,
    kTrailingRecordScratchBytes = 0x47C,
    kClubSetCount = 15,
    kBallCount = 11,
    kCaddieCount = 8,
    kModeProgressCount = 11,
    kClubSetIffStride = 0x90,
    kBallIffStride = 0x2C4,
    kIffRowPayloadOffset = 4,
    kIffEquipmentFlagsOffset = 0x59,
    kStarterLastClubUpgrade = 3,
    kKeepAudioAndOtherOptions = 0x3FFFFFFF,
    kModeInitialFlags = 0xC0
};
// Native big-endian u32 at +0x4E8: Button bit 31, Lefty bit 30, audio
// output mode bits 29..28 (0 Mono, 1 Stereo, 2 Dolby Pro Logic II).
// Button takes precedence over Lefty; the remaining 28 bits are preserved.
// These masks describe the format; no new serialized fields are introduced.
static const unsigned int kSavedButtonControlBit = 0x80000000u;
static const unsigned int kSavedLeftHandedBit = 0x40000000u;
// These names describe the initialization rule, not an unproven ownership/UI
// meaning. For clubs/balls 0x80 mirrors the IFF active bits, while 0x40 marks
// starter rows independently. Caddies have no IFF test here: only row 0 gets both.
// To change starters, change the row tests, not the IFF-derived bit or both bits.
enum StarterEquipmentEncoding {
    kIffActiveEquipmentBit = 0x80,
    kStarterEquipmentBit = 0x40,
    kCaddieStarterBits = 0xC0,
    kEmptyCourseScore = 0x7FFF
};
// Borrow one saved value record; the caller selects the active slot/shared record.
static inline PlayerRecordView* AsPlayerRecord(void* record) {
    return (PlayerRecordView*)record;
}
// Borrow the record-set header, including its four slots and shared record.
static inline SaveRecordSetView* AsSaveRecordSet(void* saveData) {
    return (SaveRecordSetView*)saveData;
}
#define RECORD_OFFSET(field, offset) \
    typedef char PlayerRecordView_##field[((int)&((PlayerRecordView*)0)->field == offset) ? 1 : -1]
RECORD_OFFSET(characterIndex, 0x2C);
RECORD_OFFSET(primaryPartFlags, 0x3C);
RECORD_OFFSET(primaryCaddieItemFlags, 0xA0);
RECORD_OFFSET(alternatePartFlags, 0xAC);
RECORD_OFFSET(alternateCaddieItemFlags, 0x110);
RECORD_OFFSET(ownedHairStyles, 0x114);
RECORD_OFFSET(itemCounts, 0x11C);
RECORD_OFFSET(auxPartFlags, 0x136);
RECORD_OFFSET(unlockedCourses, 0x258);
RECORD_OFFSET(courseRecords, 0x25C);
RECORD_OFFSET(characters, 0x2CC);
RECORD_OFFSET(caddies, 0x46C);
RECORD_OFFSET(clubSets, 0x4AC);
RECORD_OFFSET(balls, 0x4CA);
RECORD_OFFSET(systemSettings, 0x4D8);
RECORD_OFFSET(modeRecords, 0x4EC);
#undef RECORD_OFFSET
typedef char PlayerRecordView_size[(sizeof(PlayerRecordView) == 0x828) ? 1 : -1];
typedef char SavedCharacterEquipment_size[(sizeof(SavedCharacterEquipment) == 0x34) ? 1 : -1];

#define char_array_guard CharArrayGuard_u80073060
#include "game/stdexcept_local.h"
#undef char_array_guard
#include "game/singletons.h"
#include <decomp/cw_bitfields.h>
extern "C" {

extern int ItemInventory_FindPartIndex(void*, void*);

extern CSharedDoc* g_sharedDoc;
extern volatile unsigned int s_saveInventoryInitialized;
extern volatile unsigned int s_saveFlagsSixWordDefault;
extern volatile unsigned int s_saveFlagsPartBankDefault;
extern volatile unsigned int s_saveFlagsTwoWordDefault;
extern volatile unsigned int s_saveRecordDefaultTailWord;
extern volatile unsigned int s_saveRecordZeroWord;
extern volatile unsigned int s_saveRecordDefaultWord;
extern volatile signed char s_saveDataInitialized;
extern unsigned char s_saveData[];
extern void Memory_Copy(void*, void*, unsigned int);
extern void ShotGauge_ResetTotal();
extern int ItemInventory_FindAuxPartIndex(
    void*, void*); /* (table, key): defined in item_inventory_tables.cpp as (Table74* table, void* key) */
extern int ItemInventory_FindItemIndex(void*, unsigned int);
extern int ItemInventory_GetCharacterIdAt(void*, int);
extern int ItemInventory_GetPartIdAt(void*, int);
extern int ItemInventory_GetHairStyleIdAt(void*, int);
extern void* ItemInventory_FindHairStyleData(void*, int);
extern void Memory_Clear(void*, unsigned int);
extern int ItemInventory_FindClubSetIndex(void*, void*);
extern int ItemInventory_FindBallIndex(void*, void*);
extern int ItemInventory_FindCaddieIndex(void*, void*);
extern void* memset(void*, int, unsigned int);
extern int AvatarData_ResolveSavedOutfitIds(unsigned char*, unsigned char*, int);
extern int ItemSelection_BuildOutfitFromRecord(void*, void*);
extern int ItemSelection_Validate(void*);
extern void ItemSelection_RefreshPrimaryItems(void*);
extern void ItemInventory_LoadIffArchive(void*);
extern void SaveData_InitBlankRecord(void*, int, int);
extern void SaveData_InitSlotRecordDefaults(void*);
extern int SaveData_SeedSharedAvailabilityFlags(unsigned char*);
extern int ItemInventory_FindCharacterIndex(void*, void*);
extern int ItemInventory_FindCaddieItemIndex(void*, void*);
extern void SaveData_InitDefaultInventoryRecord(void*);
extern int SaveData_GetSystemSoundMode(void);
extern void Network_GetWirelessMacAddress(void*);

// Initialize flag defaults, then copy each persisted record region without copying alignment gaps.
void* SaveRecord_CopyCtor(void* destination, void* source);

// Seed one character outfit with starter Part-table rows and HairStyle-table row.
// equipment requires a writable 0x34-byte record; characterIndex is 0..7 in the
// game's internal character order. Part slots 0..23 use short row indices (-1
// means empty), not IFF type IDs; signed-byte HairStyle/AuxPart indices have the
// same empty encoding. No model is loaded or retained here. Outside 0..7, clears
// parts/aux/flag bits but leaves hairStyleIndex untouched; preserve that behavior.
// The per-case numeric rows are the stock outfit table. To mod an outfit, obtain
// replacement row indices from the loaded IFF tables (not character names or PET
// filenames); keep each slot's Part compatibility and the save field width.
// These literals are IFF row indices, not type ids; changing archive order requires remapping them.
// Record layout used here: byte 0 = flags (top two bits cleared), shorts [1..24] (bytes 2..0x31) = part-table row
// index per equipment slot (-1 = nothing equipped; the 0x30-byte memset presets every slot to -1), byte 0x32 =
// default hair-style index, byte 0x33 = -1. Which body part each slot position is (shorts [1]..[15]) is not
// established here. The old halfword positions 1..15 are now partIndices[0..14].
// These are resolver slots: ItemSelection_BuildCharacterOutfit (avatar_part_puppet_outfit.cpp) uses
// partIds[slot] and hideMask bit (1<<slot); default IDs encode slot in bits 13..17.
// Slot 0 receives the hair variant when generating default IDs. The partType
// categories (top/bottom/hat/gloves/shoes/base) are a separate IFF field, not
// this 24-slot index. Assigning those six category names to the fifteen positions
// would be misleading. To identify a row visually, resolve its IFF model/textures.
// Slots 0..7 are written for every character; extra slots are character specific.
// Per-character starter rows (characterIndex: first row used, hair index):
//   0: 0, 0    1: 0x66, 6    2: 0xB4, 0xC    3: 0x110, 0x12    4: 0x17E, 0x18    5: 0x1DA, 0x1D
//   6: 0x261, 0x24    7: 0x2B5, 0x2C   (cases 0..7 only; other indices keep the all -1 preset)
// To change a character's starting outfit, edit that case; the numbers index the part table in archive order.
void SaveData_InitializeDefaultEquipmentRecord(unsigned char* equipment, unsigned int characterIndex);

int SaveData_TestPartFlag(void* arg, void* key, int test);

extern char outOfRangeExceptionTypeString[];
extern char bitsetTestRangeErrorText[], bitset14SetRangeErrorText[], bitset45AssignRangeErrorText[],
    s_shopPurchaseBitRangeErrorText[];
extern char bitsetTestRangeError[];
extern char bitsetTestOutOfRangeMessage[], bitsetSetOutOfRangeMessage[], bitset782ResetRangeErrorText[];
}
struct BitReference {
    unsigned int* words;
    unsigned int index;
    // Borrow one word and remember a bit index; validation happens on test().
    BitReference(unsigned int* p, unsigned int i) {
        words = p;
        index = i;
    }
    // Return the selected single-word bit, throwing the original range exception if invalid.
    int test() const {
        unsigned int i = index;
        if (i >= kBitsPerFlagWord) {
            std::out_of_range error(bitsetTestRangeError, outOfRangeExceptionTypeString);
        }
        return (*words & (1u << i)) != 0;
    }
};
// Form a borrowed single-word bit reference without reading the bit.
inline BitReference testBits(unsigned int* p, unsigned int i) {
    BitReference view(p, i);
    return view;
}
// Read one checked bit from a caller-owned 32-bit bank.
inline int checkBits(unsigned int* p, unsigned int i) {
    BitReference value = testBits(p, i);
    return value.test();
}
struct Ref14 {
    unsigned int* words;
    unsigned int index;
    // Borrow a 14-bit bank, retaining its requested bit index.
    Ref14(unsigned int* p, unsigned int i) {
        words = p;
        index = i;
    }
    // Read one of the 14 course bits, throwing for an out-of-range index.
    int test() const {
        unsigned int i = index;
        if (i >= kCourseFlagCount)
            throw std::out_of_range(bitsetTestRangeErrorText);
        return (*words & (1u << i)) != 0;
    }
    // Set or clear the referenced course bit after range checking.
    void set(int value) const {
        unsigned int i = index;
        if (i >= kCourseFlagCount)
            throw std::out_of_range(bitset14SetRangeErrorText);
        unsigned int mask = 1u << i;
        if (value)
            *words |= mask;
        else
            *words &= ~mask;
    }
};
// Construct a borrowed course-bit reference without changing the bank.
inline Ref14 ref14(unsigned int* p, unsigned int i) {
    Ref14 view(p, i);
    return view;
}
struct Bits14 {
    unsigned int word;
    // Read one course bit through the checked reference specialization.
    int test(unsigned int i) {
        Ref14 r = ref14(&word, i);
        return r.test();
    }
    // Assign one course bit through the checked reference specialization.
    void set(unsigned int i, int value) {
        ref14(&word, i).set(value);
    }
};
struct FlagByte {
    CW_BITS3(unsigned char pad : 1, signed char flag : 1, unsigned char rest : 6)
};
struct Ref782 {
    unsigned int* words;
    unsigned int index;
    // Borrow a 25-word part bank and remember a logical bit index.
    Ref782(unsigned int* p, unsigned int i) {
        words = p;
        index = i;
    }
    // Read one of the 782 valid part bits, throwing for an invalid index.
    int test() const {
        unsigned int i = index;
        if (i >= kPartFlagCount) {
            std::out_of_range error(bitsetTestOutOfRangeMessage, outOfRangeExceptionTypeString);
        }
        return (words[i >> 5] & (1u << (i & 31))) != 0;
    }
};
// Construct a borrowed part-bit reference without reading its value.
inline Ref782 ref782(unsigned int* p, unsigned int i) {
    Ref782 view(p, i);
    return view;
}
// Read a checked part flag from the selected bank.
inline int check782(unsigned int* p, unsigned int i) {
    Ref782 view = ref782(p, i);
    return view.test();
}
extern "C" {
// Look up a part id and test its primary or alternate flag in the active save record.
int SaveData_TestPartFlag(void* saveData, void* partKey, int useAlternateFlags);
// Look up caddie equipment and test the selected flag bank in the active record.
int SaveData_TestCaddieItemFlag(void* saveData, void* caddieItemKey, int useAlternateFlags);

// Reject saved outfits whose nonzero part ids have the wrong item kind or character index.
int SaveData_ValidateCharacterOutfitParts(unsigned char* equipmentRecords);

// Resolve a character id and copy its saved outfit flags, part indices, hair and ring.
// equipment needs 0x34 bytes; byte +1 is padding and is not written.
// Existing guard accepts indices below 0x1A0 despite only eight stored outfits:
// callers must supply an actual character index 0..7 (do not trust that guard alone).
int SaveData_CopyCharacterEquipmentRecord(unsigned char* saveData, unsigned char* equipment, void* characterKey);

struct Ref45 {
    unsigned int* words;
    unsigned int index;
};
#pragma use_lmw_stmw on
// Set or clear one of 45 bits after checking the bit index.
Ref45* Bitset45Reference_Assign(Ref45* ref, int value);
#pragma use_lmw_stmw off

// Initialize all 25 words of a part-sized flag bank from its stored default word.
void SaveFlags_InitPartBank(unsigned int* flagWords);

struct CharacterEquipmentTransfer {
    unsigned char flags;
    unsigned short partIndices[kOutfitPartCount];
    unsigned char hairStyleIndex;
    unsigned char auxPartIndex;
};

// Write one character outfit into the active save record, preserving each original field store.
// characterIndex must be 0..7; no bounds check is performed. equipment is borrowed.
void SaveData_SetCharacterEquipmentRecord(unsigned char* saveData, const CharacterEquipmentTransfer* equipment,
                                          int characterIndex);

// Copy an outfit by character inventory index, using the existing lookup limit.
// Same 0x34-byte output and 0..7 caller precondition as the ID-based copy above;
// the broad 0x1A0 guard is retained as existing behavior.
int SaveData_CopyCharacterEquipmentRecordAt(unsigned char* saveData, unsigned char* equipment, int characterIndex);

#pragma use_lmw_stmw on
// Set or clear one checked bit in a single-word flag set.
unsigned int* Bitset32_AssignBit(unsigned int* bits, unsigned int index, int value);

// Set or clear one checked course flag in a 14-bit flag set.
unsigned int* Bitset14_AssignBit(unsigned int* bits, unsigned int index, int value);
#pragma use_lmw_stmw off

#pragma use_lmw_stmw on
// Clear one checked part flag in a 782-bit flag set.
unsigned int* Bitset782_ResetBit(unsigned int* bits, unsigned int index);
#pragma use_lmw_stmw off

#pragma use_lmw_stmw on
// Set or clear one checked part flag in a 782-bit flag set.
unsigned int* Bitset782_AssignBit(unsigned int* bits, unsigned int index, int value);
#pragma use_lmw_stmw off

// Load the IFF inventory once and initialize four save slots plus the shared record.
// saveData requires complete record-set storage. Side effects initialize the
// shared inventory and overwrite all five records. The ABI declares int but
// this body has no defined status return; callers must not interpret it as bool.
// Although declared int in the reconstructed ABI, this body has no return statement;
// callers must not interpret an integer status from it (treat it as void; the declaration is kept as int
// because the recovered signature, used by callers, cannot be changed without altering the compiled output).
// Steps: (1) load the item inventory archive once (guarded by s_saveInventoryInitialized); (2) reset the
// current-slot state; (3) blank all kSaveSlotCount slot records (last argument 0); (4) blank the shared
// record (slot 0, last argument 1).
int SaveData_InitializeBlankRecordSet(unsigned char* saveData);

#pragma use_lmw_stmw on
// Enable or disable shared-record mode; optionally merge character and course unlocks from all slots.
int SaveData_SetSharedRecordMode(unsigned char* saveData, int enableSharedRecord, unsigned char initializationFlags);

// Create starter equipment, inventory flags, settings and mode records for a slot or shared record.
void SaveData_InitBlankRecord(void* saveData, int slotIndex, int useSharedRecord);

// Select a valid save slot, mark the selection changed, and notify the dependent subsystem.
int SaveData_SelectSlot(unsigned int* saveData, int slotIndex);

// Return a part-table record by index, skipping its preceding lookup key.
unsigned char* ItemInventory_GetPartDataAt(unsigned char* inventory, int partIndex);

// Return whether the active save record has a nonzero flag for the requested ring.
int SaveData_HasAuxPart(unsigned char* saveData, void* auxPartKey);

// Check consumable availability from equipped slots or spare count, rejecting the unavailable-id flag.
// With checkInventory!=0, itemTypeId must resolve to a valid itemCounts index;
// a failed lookup is not rejected before indexing. With zero, only the ID bit is tested.
int SaveData_IsConsumableAvailable(unsigned char* saveData, unsigned int itemTypeId, void* checkInventory);

// Expand saved inventory indices into character/part ids and pack the selected hair colour.
int AvatarData_ResolveSavedOutfitIds(unsigned char* saveData, unsigned char* resolvedEquipment, int characterIndex);

// Resolve a ball id and copy its saved flag byte.
int SaveData_CopyBallRecord(unsigned char* saveData, unsigned char* ballFlags, void* ballKey);

// Resolve a caddie id and copy its saved flag byte and value word.
// Output needs 8 bytes; padding +1..3 is untouched. Only eight rows are stored,
// although the preserved lookup guard accepts indices below 0x40.
int SaveData_CopyCaddieRecord(unsigned char* saveData, unsigned char* caddieEquipment, void* caddieKey);

// Return the embedded IFF inventory from the shared document.
unsigned char* SharedDoc_GetItemInventory();

// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_774E8(unsigned int* flags);

// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_776F8(unsigned int* flags);

// Initialize a two-word bitset to 32 set bits followed by 19 set bits.
void SaveFlags_Init51BitsSet(unsigned int* flagWords);

// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_77738(unsigned int* flags);

// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_77744(unsigned int* flags);

// Initialize both words of a small flag bank from the same stored default.
void SaveFlags_InitTwoDefaultWords(unsigned int* flagWords);

// Initialize six flag words from the same stored default.
void SaveFlags_InitSixDefaultWords(unsigned int* flagWords);

// Returns the pair in r3:r4 unchanged (blr at 0x80077774); SaveData_InitDefaultInventoryRecord stores both (0x80072DE0-0x80072DE8).
// Return the two input words as a 64-bit pair, retaining the original volatile saves.
unsigned long long SaveRecord_PreserveWordPair(unsigned int highWord, unsigned int lowWord);

// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_778B0(unsigned int* flags);

// Clear unlockedCourses: save_data_records_and_inventory_init.cpp passes SavePlayerRecord +0x258, then
// seeds course indices 0,2,5 with Bitset14_AssignBit.
void SaveFlags_ClearCourseAvailabilityWord(unsigned int* flags);

// Return the selected save-slot index.
unsigned int SaveData_GetCurrentSlotIndex(unsigned int* saveData);

// Return the flag indicating that the selected save slot changed.
unsigned char SaveData_HasSlotSelectionChanged(unsigned char* saveData);

// Return whether save access currently uses the shared record.
unsigned char SaveData_IsSharedRecordMode(unsigned char* saveData);

// Test the slot-used word for a valid save-slot index.
unsigned int SaveData_IsSlotUsed(unsigned char* saveData, int slotIndex);

// Return a slot record, falling back to slot zero for an invalid index.
unsigned char* SaveData_GetSlotRecord(unsigned char* saveData, int slotIndex);

// Return the shared record when enabled, otherwise the selected slot.
unsigned char* SaveData_GetCurrentRecord(unsigned char* saveData);

// Copy-granularity views of regions of PlayerRecordView, used only by the two record copy functions below.
// Their element widths reproduce the original copy loops; they are not separate on-save types.
struct RecW151 {
    unsigned int a[151];
}; // 0x000..0x25C header, inventory flags, statistics, unlocked courses
struct RecW28 {
    unsigned int a[28];
}; // 0x25C..0x2CC courseRecords
struct RecH208 {
    unsigned short a[208];
}; // 0x2CC..0x46C characters[8]
struct RecW16 {
    unsigned int a[16];
}; // 0x46C..0x4AC caddies[8]
struct RecB24 {
    unsigned char a[24];
}; // 0x4AC..0x4C4 first twelve clubSets entries
struct RecB6 {
    unsigned char a[6];
}; // 0x4C4..0x4CA last three clubSets entries
struct RecB8 {
    unsigned char a[8];
};
struct RecB3 {
    unsigned char a[3];
};
struct RecB11 {
    unsigned char a[11];
}; // 0x4CA..0x4D5 balls[11]
struct RecW5 {
    unsigned int a[5];
};
struct RecH414 {
    unsigned short a[414];
};
struct RecW5H414 {
    unsigned int a[5];
    unsigned short b[414];
}; // 0x4D8.. systemSettings + start of modeRecords

// Prefix of PlayerRecordView (0x000..0x25C): header, selected item indices, ownership flag banks, item counts
// and the statistics block. Field names follow PlayerRecordView; statistics words are named by byte offset
// because their meaning is not established.
struct RecordPrefix0x25C {
    unsigned short headerWords[16];   // 0x000: firstNameCodeUnit + reservedHeader
    unsigned int headerWord20;        // 0x020
    unsigned int headerWord24;        // 0x024
    unsigned int headerWord28;        // 0x028
    unsigned char selectedIndices[5]; // 0x02C: characterIndex, clubSetIndex, ballIndex, caddieIndex, caddieItemIndex
    unsigned char equippedItemIndices[8];  // 0x031
    unsigned int primaryPartFlags[25];     // 0x03C
    unsigned int primaryCaddieItemFlags;   // 0x0A0
    unsigned int reservedFlags[2];         // 0x0A4
    unsigned int alternatePartFlags[25];   // 0x0AC
    unsigned int alternateCaddieItemFlags; // 0x110
    unsigned int ownedHairStyles[2];       // 0x114
    unsigned short itemCounts[12];         // 0x11C
    unsigned short reservedItemCount;      // 0x134
    unsigned char auxPartFlags[7];         // 0x136
    unsigned int unresolvedFlagWord;       // 0x140: secondary banks begin here; keep float lanes below for copy code
    unsigned int partSizedFlags[25];       // 0x144
    unsigned int sixWordFlags[6];          // 0x1A8
    unsigned int unresolvedSmallFlag0, unresolvedSmallFlag1, unresolvedSmallFlag2, unresolvedSmallFlag3; // 0x1C0
    unsigned int checked45BitFlags[2];                                                                   // 0x1D0
    unsigned int unresolvedTrailingFlag, stat1DC, stat1E0, stat1E4, stat1E8, stat1EC, stat1F0, stat1F4, stat1F8,
        stat1FC, stat200;            // 0x1D8
    float stat204, stat208, stat20C; // 0x204
    unsigned int stat210;            // 0x210
    float stat214;                   // 0x214
    unsigned char stat218[64];       // 0x218
    unsigned int unlockedCourses;    // 0x258
};
typedef char RecordPrefix0x25C_size_check[(sizeof(RecordPrefix0x25C) == 0x25C) ? 1 : -1];

// Offsets of the copied regions inside PlayerRecordView.
enum RecordRegionOffsets {
    kRecordCourseRecordsOffset = (int)&((PlayerRecordView*)0)->courseRecords,
    kRecordCharactersOffset = (int)&((PlayerRecordView*)0)->characters,
    kRecordCaddiesOffset = (int)&((PlayerRecordView*)0)->caddies,
    kRecordClubSetsOffset = (int)&((PlayerRecordView*)0)->clubSets,
    kRecordClubSetsTailOffset = kRecordClubSetsOffset + 24,
    kRecordBallsOffset = (int)&((PlayerRecordView*)0)->balls,
    kRecordSystemSettingsOffset = (int)&((PlayerRecordView*)0)->systemSettings
};

// Copy all persisted regions into the active record, retaining the original copy widths.
// destination is the shared record or slots[currentSlot]; source is a full value record, not a record set.
void SaveData_SetCurrentRecord(unsigned char* saveData, unsigned char* source);

// Replace only the active record inventory/statistics prefix through offset 0x25C.
void SaveData_SetInventoryAndStatisticsPrefix(unsigned char* saveData, unsigned char* source);

// Write one saved ball flag byte by inventory index.
void SaveData_SetBallRecordAt(unsigned char* saveData, unsigned char* ballFlags, int ballIndex);

// Resolve a club-set id and copy its saved flag byte and upgrade level.
// The two flag bits are copied unchanged; this accessor does not assign ownership meaning.
// Output needs two bytes. Actual table index must be 0..14; the existing guard
// uses 0x1E (the region's byte size) rather than its element count.
int SaveData_CopyClubSetRecord(unsigned char* saveData, unsigned char* clubSetEquipment, void* clubSetKey);

// Resolve the saved outfit, build the selection, and refresh primary items if validation fails.
int SaveData_LoadValidatedItemSelection(void* saveData, void* selection, signed char characterIndex);

// Replace the active record mode-data region while preserving its original copy boundaries.
void SaveData_SetModeRecords(unsigned char* saveData, unsigned short* modeRecords);

// Write a caddie flag byte and value word into the active save record.
void SaveData_SetCaddieRecordAt(unsigned char* saveData, const unsigned char* caddieEquipment,
                                unsigned int caddieIndex);

// Write a club-set flag byte and upgrade level into the active save record.
void SaveData_SetClubSetRecordAt(unsigned char* saveData, const unsigned char* clubSetEquipment,
                                 unsigned int clubSetIndex);

// Copy the existing 0x47C-byte record prefix; this API does not copy a complete slot.
void* SaveData_CopyRecordPrefix47C(void* destination, void* source);

// Initialize flag defaults for four slot records, then copy the full four-slot array.
void* SaveSlotArray_CopyCtor(void* destination, void* source);

// Initialize flag defaults, then copy the existing 0x47C-byte record prefix.
void* SaveRecordPrefix_CopyCtor(void* destination, void* source);

} // extern "C"

#pragma push
#pragma always_inline on
// Seed primaryPartFlags from its stored default without changing read/store order.
static inline void SeedPrimaryPartDefaults(void* record) {
    AsPlayerRecord(record)->primaryPartFlags[0] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[1] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[2] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[3] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[4] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[5] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[6] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[7] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[8] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[9] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[10] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[11] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[12] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[13] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[14] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[15] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[16] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[17] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[18] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[19] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[20] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[21] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[22] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[23] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[24] = s_saveRecordDefaultWord;
}
// Seed alternatePartFlags from its stored default without changing read/store order.
static inline void SeedAlternatePartDefaults(void* record) {
    AsPlayerRecord(record)->alternatePartFlags[0] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[1] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[2] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[3] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[4] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[5] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[6] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[7] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[8] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[9] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[10] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[11] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[12] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[13] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[14] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[15] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[16] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[17] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[18] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[19] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[20] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[21] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[22] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[23] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->alternatePartFlags[24] = s_saveRecordDefaultWord;
}
// Seed secondaryBanks.partSizedFlags from its stored default without changing read/store order.
static inline void SeedSecondaryPartSizedDefaults(void* record) {
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[0] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[1] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[2] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[3] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[4] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[5] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[6] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[7] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[8] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[9] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[10] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[11] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[12] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[13] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[14] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[15] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[16] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[17] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[18] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[19] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[20] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[21] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[22] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[23] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->secondaryBanks.partSizedFlags[24] = s_saveRecordDefaultWord;
}
// Seed secondaryBanks.sixWordFlags from its stored default without changing read/store order.
static inline void SeedSecondarySixWordDefaults(void* record) {
    AsPlayerRecord(record)->secondaryBanks.sixWordFlags[0] = s_saveRecordDefaultTailWord;
    AsPlayerRecord(record)->secondaryBanks.sixWordFlags[1] = s_saveRecordDefaultTailWord;
    AsPlayerRecord(record)->secondaryBanks.sixWordFlags[2] = s_saveRecordDefaultTailWord;
    AsPlayerRecord(record)->secondaryBanks.sixWordFlags[3] = s_saveRecordDefaultTailWord;
    AsPlayerRecord(record)->secondaryBanks.sixWordFlags[4] = s_saveRecordDefaultTailWord;
    AsPlayerRecord(record)->secondaryBanks.sixWordFlags[5] = s_saveRecordDefaultTailWord;
}
#pragma pop

// These initialization variants share unrolled flag seeding with the copy constructors.
// They differ in both addressing and which banks they initialize; compare each body
// before reusing this map (word indexes are relative to the record start):
//   - primaryPartFlags[0..24] and alternatePartFlags[0..24]: 25 availability words each, set to
//     s_saveRecordDefaultWord (unlock bits owned by the initial state);
//   - primaryCaddieItemFlags / alternateCaddieItemFlags, reservedFlags[0..1], ownedHairStyles[0..1]: zeroed;
//   - secondaryBanks: unresolved single-word, part-sized, six-word and 45-bit
//     flag banks; bank defaults are distinct stored volatile words. Later counters
//     and the byte table are NOT initialized by every constructor. See each body.
// Legacy initializer for the record flag banks and statistics defaults.
extern "C" void fn_80075F38_old(void* record) {
    // Seed the primary part bank; defaults are stored words, not inferred ownership.
    SeedPrimaryPartDefaults(record);
    // Seed the alternate part bank independently of the primary bank.
    SeedAlternatePartDefaults(record);
    SeedSecondaryPartSizedDefaults(record);
    SeedSecondarySixWordDefaults(record);
    AsPlayerRecord(record)->reservedFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->reservedFlags[1] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->ownedHairStyles[0] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->ownedHairStyles[1] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->secondaryBanks.checked45BitFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->secondaryBanks.checked45BitFlags[1] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->primaryCaddieItemFlags = 0;
    AsPlayerRecord(record)->alternateCaddieItemFlags = 0;
    AsPlayerRecord(record)->secondaryBanks.unresolvedFlagWord = 0;
    AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[0] = 0;
    AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[1] = 0;
    AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[2] = 0;
    AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[3] = 0;
    AsPlayerRecord(record)->secondaryBanks.unresolvedTrailingFlag = 0;
    AsPlayerRecord(record)->unlockedCourses = 0;
}
#pragma opt_propagation off
// Lazily initialize the save singleton and its record defaults, then ensure IFF data is loaded.
extern "C" void* SaveData_AcquireInitializedInstance();

// Initialize one save record flag bank and statistics defaults without clearing its other data.
extern "C" void SaveData_InitPlayerRecord(void* record);
// Initialize the flag banks and statistics defaults of all four save-slot records.
extern "C" void SaveData_InitSlotRecordDefaults(void* slotRecords);

// Initialize defaults, then clear the existing 0x47C-byte record prefix.
extern "C" void* SaveRecordPrefix_ClearCtor(void* record);

struct Bits7583C {
    unsigned int word;
    // Set/clear one checked shared-record course-availability bit.
    void set(unsigned int i, int value) {
        if (i >= kCourseFlagCount) {
            std::out_of_range error(bitset14SetRangeErrorText, outOfRangeExceptionTypeString);
        }
        unsigned int mask = 1u << i;
        if (value)
            word |= mask;
        else
            word &= ~mask;
    }
};

struct SharedCourseFlagsView {
    unsigned char pad[0x2300];
    Bits7583C bits;
};

// Seed shared-mode character, course and item flags with the built-in availability bits.
extern "C" int SaveData_SeedSharedAvailabilityFlags(unsigned char* saveData);

#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
// Seed one character outfit with starter part-table indices and the default hair-style index.
void SaveData_InitializeDefaultEquipmentRecord(unsigned char* equipment, unsigned int characterIndex) {
    int noItem = -1;
    SavedCharacterEquipment* outfit = (SavedCharacterEquipment*)equipment;
    outfit->auxPartIndex = noItem;
    *(signed char*)&outfit->flags &= ~kEquipmentInitialFlagMask;
    memset(outfit->partIndices, kEmptyPartFillWord, sizeof(outfit->partIndices));

    switch (characterIndex) {
    case 0:
        outfit->partIndices[0] = 0;
        outfit->partIndices[1] = noItem;
        outfit->partIndices[2] = 0x0D;
        outfit->partIndices[3] = 0x24;
        outfit->partIndices[4] = 0x27;
        outfit->partIndices[5] = 0x31;
        outfit->partIndices[6] = noItem;
        outfit->partIndices[7] = 0x42;
        outfit->partIndices[8] = 0x44;
        outfit->hairStyleIndex = 0;
        break;
    case 1:
        outfit->partIndices[0] = 0x66;
        outfit->partIndices[1] = 0x68;
        outfit->partIndices[2] = 0x77;
        outfit->partIndices[3] = 0x85;
        outfit->partIndices[4] = noItem;
        outfit->partIndices[5] = noItem;
        outfit->partIndices[6] = 0x99;
        outfit->partIndices[7] = 0xA3;
        outfit->hairStyleIndex = 6;
        break;
    case 2:
        outfit->partIndices[0] = 0xB4;
        outfit->partIndices[1] = 0xC0;
        outfit->partIndices[2] = 0xC3;
        outfit->partIndices[3] = 0xD6;
        outfit->partIndices[4] = 0xD7;
        outfit->partIndices[5] = 0xE1;
        outfit->partIndices[6] = 0xE9;
        outfit->partIndices[7] = 0xEE;
        outfit->hairStyleIndex = 0x0C;
        break;
    case 3:
        outfit->partIndices[0] = 0x110;
        outfit->partIndices[1] = 0x111;
        outfit->partIndices[2] = 0x125;
        outfit->partIndices[3] = noItem;
        outfit->partIndices[4] = 0x137;
        outfit->partIndices[5] = noItem;
        outfit->partIndices[6] = 0x14B;
        outfit->partIndices[7] = 0x15A;
        outfit->partIndices[10] = 0x16E;
        outfit->hairStyleIndex = 0x12;
        break;
    case 4:
        outfit->partIndices[0] = 0x17E;
        outfit->partIndices[1] = 0x17F;
        outfit->partIndices[2] = 0x197;
        outfit->partIndices[3] = 0x1A2;
        outfit->partIndices[4] = 0x1B3;
        outfit->partIndices[5] = 0x1C6;
        outfit->partIndices[6] = 0x1C9;
        outfit->partIndices[7] = 0x1CA;
        outfit->hairStyleIndex = 0x18;
        break;
    case 5:
        outfit->partIndices[0] = 0x1DA;
        outfit->partIndices[1] = 0x1DB;
        outfit->partIndices[2] = 0x1F5;
        outfit->partIndices[3] = 0x1F8;
        outfit->partIndices[4] = 0x203;
        outfit->partIndices[5] = noItem;
        outfit->partIndices[6] = 0x220;
        outfit->partIndices[7] = 0x23C;
        outfit->partIndices[8] = 0x240;
        outfit->partIndices[9] = 0x241;
        outfit->partIndices[14] = 0x250;
        outfit->hairStyleIndex = 0x1D;
        break;
    case 6:
        outfit->partIndices[0] = 0x261;
        outfit->partIndices[1] = 0x262;
        outfit->partIndices[2] = 0x275;
        outfit->partIndices[3] = 0x286;
        outfit->partIndices[4] = 0x28D;
        outfit->partIndices[5] = 0x29E;
        outfit->partIndices[6] = 0x2A7;
        outfit->partIndices[7] = 0x2AA;
        outfit->hairStyleIndex = 0x24;
        break;
    case 7:
        outfit->partIndices[0] = 0x2B5;
        outfit->partIndices[1] = 0x2B6;
        outfit->partIndices[2] = 0x2BC;
        outfit->partIndices[3] = noItem;
        outfit->partIndices[4] = 0x2D4;
        outfit->partIndices[5] = noItem;
        outfit->partIndices[6] = noItem;
        outfit->partIndices[7] = 0x2EE;
        outfit->partIndices[8] = noItem;
        outfit->partIndices[9] = 0x300;
        outfit->partIndices[10] = 0x301;
        outfit->partIndices[11] = 0x302;
        outfit->partIndices[12] = 0x304;
        outfit->hairStyleIndex = 0x2C;
        break;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw off
// Load the IFF inventory once and initialize four save slots plus the shared record.
int SaveData_InitializeBlankRecordSet(unsigned char* saveData) {
    if (s_saveInventoryInitialized == 0) {
        ItemInventory_LoadIffArchive((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
        s_saveInventoryInitialized = 1;
    }
    AsSaveRecordSet(saveData)->currentSlot = 0;
    AsSaveRecordSet(saveData)->slotChanged = 0;
    AsSaveRecordSet(saveData)->useSharedRecord = 0;
    AsSaveRecordSet(saveData)->sharedInitializationFlags = 0;
    for (int i = 0; i < kSaveSlotCount; i++)
        SaveData_InitBlankRecord(saveData, i, 0);
    SaveData_InitBlankRecord(saveData, 0, 1);
    // The original returns no value: its callers never read one.
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Save settings: wireless MAC bytes, two volume words (baseline 80 percent),
// then the mixed controller/handedness/audio word. Options consumes +4E0 as
// stream/BGM volume and +4E4 as buffer/SE volume (option_dialog_control_mode_audio.cpp).
struct SavedSystemSettingsView {
    unsigned char wirelessMacAndPadding[8];
    unsigned int streamVolumePercent, bufferVolumePercent, controlAndAudioOptions;
};
// Eleven 0x4A-byte mode records precede the eight-area order suffix.
// The first byte doubles as the mode-wide initial flag byte in this format.
struct SavedModeProgressEntry {
    unsigned char header[2];
    short indexOrNone;
    char rest[0x46];
};
struct SavedModeDefaultsView {
    SavedModeProgressEntry progress[kModeProgressCount];
    unsigned char order[kCharacterCount];
    signed char selection;
};
typedef char SavedSystemSettingsView_size[sizeof(SavedSystemSettingsView) == 0x14 ? 1 : -1];
typedef char SavedModeProgressEntry_size[sizeof(SavedModeProgressEntry) == 0x4A ? 1 : -1];

// Field-derived byte addresses retain the compiler's original settings-base lifetime.
enum SavedSettingsOffsets {
    kSavedStreamVolumePercent = (int)&((PlayerRecordView*)0)->systemSettings +
        (int)&((SavedSystemSettingsView*)0)->streamVolumePercent,
    kSavedBufferVolumePercent = (int)&((PlayerRecordView*)0)->systemSettings +
        (int)&((SavedSystemSettingsView*)0)->bufferVolumePercent,
    kSavedControlAndAudioOptions = (int)&((PlayerRecordView*)0)->systemSettings +
        (int)&((SavedSystemSettingsView*)0)->controlAndAudioOptions
};

// Seed one empty course record without writing the serialized padding byte.
static inline void InitializeEmptyCourseRecord(SavedCourseRecord* course, int invalid) {
    course->clearedWord = 0;
    course->scoreOrEmpty = kEmptyCourseScore;
    course->indexOrNone = invalid;
}

// Seed all fourteen records in serialized order; explicit calls preserve the stores.
static inline void InitializeEmptyCourseRecords(unsigned char* record, int invalid) {
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[0], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[1], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[2], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[3], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[4], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[5], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[6], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[7], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[8], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[9], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[10], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[11], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[12], invalid);
    InitializeEmptyCourseRecord(&AsPlayerRecord(record)->courseRecords[13], invalid);
}

// Set only the two initialization bits; preserve the caddie's other six bits.
static inline void InitializeStarterCaddieFlags(signed char* flags) {
    for (int row = 0; row < kCaddieCount; flags += sizeof(SavedCaddieEquipment), row++) {
        *flags &= ~kCaddieStarterBits;
        if (row == 0)
            *flags |= kCaddieStarterBits;
    }
}

// Seed club starter rows 0/14 independently of each IFF row's active bit.
// Cursors borrow the record/IFF banks; all fifteen IFF rows must exist.
static inline void InitializeStarterClubSets(unsigned char*& flag, unsigned char*& secondary,
                                             unsigned char*& clubSetTableCursor, int& i) {
    for (i = 0; i < kClubSetCount;
         i++, flag += sizeof(SavedClubSetEquipment), secondary += sizeof(SavedClubSetEquipment)) {
        *(signed char*)flag = (*(signed char*)flag | kIffActiveEquipmentBit) & ~kStarterEquipmentBit;
        ((SavedClubSetEquipment*)&AsPlayerRecord(secondary)->clubSets[0])->upgradeLevel = 0;
        if (i == 0 || i == kClubSetCount - 1)
            *flag |= kStarterEquipmentBit;
        CSharedDoc* manager = g_sharedDoc;
        clubSetTableCursor = (unsigned char*)((CItemManager*)&AsCSharedDoc(manager)->itemInventory)->clubSets.data;
        unsigned char* item;
        if (i < 0 || (unsigned int)i >= ((CItemManager*)&AsCSharedDoc(manager)->itemInventory)->clubSets.size)
            item = 0;
        else {
            int index = i;
            while (index != 0) {
                clubSetTableCursor += kClubSetIffStride;
                index--;
            }
            item = clubSetTableCursor + kIffRowPayloadOffset;
        }
        struct Bits {
            CW_BITS2(unsigned char active : 3, unsigned char rest : 5)
        };
        if (((Bits*)(item + kIffEquipmentFlagsOffset))->active != 0)
            *flag |= kIffActiveEquipmentBit;
        else
            *(signed char*)flag &= ~kIffActiveEquipmentBit;
    }
}

// Seed ball starter row 0 independently of each IFF row's active bit.
// All eleven IFF rows must exist; the cursor and save record are borrowed.
static inline void InitializeStarterBalls(unsigned char*& record, unsigned char*& ballTableCursor, int& i) {
    for (i = 0; i < kBallCount; i++) {
        unsigned char* entry = &AsPlayerRecord(record)->balls[i];
        *(signed char*)entry = (*(signed char*)entry | kIffActiveEquipmentBit) & ~kStarterEquipmentBit;
        if (i == 0)
            *entry |= kStarterEquipmentBit;
        CSharedDoc* manager = g_sharedDoc;
        ballTableCursor = (unsigned char*)((CItemManager*)&AsCSharedDoc(manager)->itemInventory)->balls.data;
        unsigned char* item;
        if (i < 0 || (unsigned int)i >= ((CItemManager*)&AsCSharedDoc(manager)->itemInventory)->balls.size)
            item = 0;
        else {
            int index = i;
            while (index != 0) {
                ballTableCursor += kBallIffStride;
                index--;
            }
            item = ballTableCursor + kIffRowPayloadOffset;
        }
        struct Bits {
            CW_BITS2(unsigned char active : 3, unsigned char rest : 5)
        };
        if (((Bits*)(item + kIffEquipmentFlagsOffset))->active != 0)
            *entry |= kIffActiveEquipmentBit;
        else
            *(signed char*)entry &= ~kIffActiveEquipmentBit;
    }
}

// Create starter User Data, including Swing Righty defaults, for one slot/shared record.
// saveData requires a complete SaveRecordSetView; slotIndex must be 0..3 even
// when useSharedRecord != 0. An invalid index returns without writing anything.
// Requires initialized IFF rows (15 ClubSets, 11 Balls). Destroys the previous
// value record in place; owns no resource handles and performs no disk save.
// Preserve field widths, the independent control/audio masks, and IFF row order.
void SaveData_InitBlankRecord(void* saveData, int slotIndex, int useSharedRecord) {
    unsigned char* saveBytes = (unsigned char*)saveData;
    if (slotIndex < 0 || slotIndex >= kSaveSlotCount)
        return;
    unsigned char* record;
    if (useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveBytes)->sharedRecord;
    else {
        saveBytes = saveBytes + slotIndex * sizeof(PlayerRecordView);
        record = saveBytes + 8;
    }

    int invalid = -1;
    // Phase 1: clear the record and seed inventory plus fourteen empty course scores.
    Memory_Clear(record, sizeof(PlayerRecordView));
    SaveData_InitDefaultInventoryRecord(record);
    InitializeEmptyCourseRecords(record, invalid);
    // Phase 2: eight starter outfits and default caddie flags.
    unsigned char* cursor = (unsigned char*)AsPlayerRecord(record)->characters;
    for (int i = 0; i < kCharacterCount; i++) {
        SaveData_InitializeDefaultEquipmentRecord(cursor, i);
        cursor += sizeof(SavedCharacterEquipment);
    }

    signed char* caddieFlags = (signed char*)AsPlayerRecord(record)->caddies;
    InitializeStarterCaddieFlags(caddieFlags);

    // Phase 3: initialize club/ball flags from the IFF shop flags; table rows must
    // exist (15 clubs, 11 balls), since the original dereferences even null lookups.
    unsigned char* flag = (unsigned char*)AsPlayerRecord(record)->clubSets;
    unsigned char* secondary = record;
    unsigned char* clubSetTableCursor;
    int clubRow;
    InitializeStarterClubSets(flag, secondary, clubSetTableCursor, clubRow);

    AsPlayerRecord(record)->clubSets[kClubSetCount - 1].upgradeLevel = kStarterLastClubUpgrade;
    unsigned char* ballTableCursor;
    int ballRow;
    InitializeStarterBalls(record, ballTableCursor, ballRow);

    // Phase 4: volumes, control/handedness and system sound mode, then MAC.
    // Stock clears BOTH control bits (Swing Righty); sound insertion below
    // changes only bits 29..28. Do not call this mask an audio reset: clearing
    *(unsigned int*)(record + kSavedStreamVolumePercent) = kDefaultVolumePercent;
    *(unsigned int*)(record + kSavedBufferVolumePercent) = kDefaultVolumePercent;
    // The stock default record has bit 31 set and bit 30 (Lefty) clear; audio and other bits follow the stock mask.
    *(unsigned int*)(record + kSavedControlAndAudioOptions) &= kKeepAudioAndOtherOptions;
    unsigned int value = SaveData_GetSystemSoundMode();
    struct Packed {
        CW_BITS3(unsigned int first : 2, unsigned int value : 2, unsigned int rest : 28)
    };
    ((Packed*)(record + kSavedControlAndAudioOptions))->value = value;
    Network_GetWirelessMacAddress(AsPlayerRecord(record)->systemSettings);
    // Phase 5: clear mode records and retain the serialized character order.
    memset(AsPlayerRecord(record)->modeRecords, 0, sizeof(AsPlayerRecord(record)->modeRecords));
    for (int i = 0; i < kModeProgressCount; i++)
        ((SavedModeDefaultsView*)AsPlayerRecord(record)->modeRecords)->progress[i].indexOrNone = invalid;
    for (int i = 0; i < kCharacterCount; i++)
        ((SavedModeDefaultsView*)AsPlayerRecord(record)->modeRecords)->order[i] = (unsigned char)i;
    AsPlayerRecord(record)->modeRecords[0] |= kModeInitialFlags;
    ((SavedModeDefaultsView*)AsPlayerRecord(record)->modeRecords)->selection = invalid;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Select a valid save slot, mark the selection changed, and notify the dependent subsystem.
int SaveData_SelectSlot(unsigned int* saveData, int slotIndex) {
    if (slotIndex < 0 || slotIndex >= kSaveSlotCount)
        return 0;
    *saveData = slotIndex;
    AsSaveRecordSet(saveData)->slotChanged = 1;
    ShotGauge_ResetTotal();
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return the selected save-slot index.
unsigned int SaveData_GetCurrentSlotIndex(unsigned int* saveData) {
    return AsSaveRecordSet(saveData)->currentSlot;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return the flag indicating that the selected save slot changed.
unsigned char SaveData_HasSlotSelectionChanged(unsigned char* saveData) {
    return AsSaveRecordSet(saveData)->slotChanged;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Enable or disable shared-record mode; optionally merge character and course unlocks from all slots.
int SaveData_SetSharedRecordMode(unsigned char* saveData, int enableSharedRecord, unsigned char initializationFlags) {
    // Phase 1: create and seed a fresh shared record; disabling simply selects slot mode.
    if (enableSharedRecord != 0) {
        AsSaveRecordSet(saveData)->sharedInitializationFlags = initializationFlags;
        AsSaveRecordSet(saveData)->useSharedRecord = 1;
        SaveData_InitBlankRecord(saveData, 0, 1);
        SaveData_SeedSharedAvailabilityFlags(saveData);
        // Phase 2 (flags==0 only): OR character/course availability across all four slots.
        if (AsSaveRecordSet(saveData)->sharedInitializationFlags == 0) {
            SavedCharacterEquipment* slotCharacters = AsSaveRecordSet(saveData)->slots[0].characters;
            unsigned char* slotCourseFlags = (unsigned char*)&AsSaveRecordSet(saveData)->slots[0].unlockedCourses;
            Bits14* sharedCourseFlags = (Bits14*)&AsSaveRecordSet(saveData)->sharedRecord.unlockedCourses;
            for (int slotIndex = 0; slotIndex < kSaveSlotCount; slotIndex++) {
                ((FlagByte*)&AsSaveRecordSet(saveData)->sharedRecord.characters[0].flags)->flag |=
                    ((FlagByte*)&slotCharacters[0].flags)->flag;
                ((FlagByte*)&AsSaveRecordSet(saveData)->sharedRecord.characters[1].flags)->flag |=
                    ((FlagByte*)&slotCharacters[1].flags)->flag;
                ((FlagByte*)&AsSaveRecordSet(saveData)->sharedRecord.characters[2].flags)->flag |=
                    ((FlagByte*)&slotCharacters[2].flags)->flag;
                ((FlagByte*)&AsSaveRecordSet(saveData)->sharedRecord.characters[3].flags)->flag |=
                    ((FlagByte*)&slotCharacters[3].flags)->flag;
                ((FlagByte*)&AsSaveRecordSet(saveData)->sharedRecord.characters[4].flags)->flag |=
                    ((FlagByte*)&slotCharacters[4].flags)->flag;
                ((FlagByte*)&AsSaveRecordSet(saveData)->sharedRecord.characters[5].flags)->flag |=
                    ((FlagByte*)&slotCharacters[5].flags)->flag;
                ((FlagByte*)&AsSaveRecordSet(saveData)->sharedRecord.characters[6].flags)->flag |=
                    ((FlagByte*)&slotCharacters[6].flags)->flag;
                ((FlagByte*)&AsSaveRecordSet(saveData)->sharedRecord.characters[7].flags)->flag |=
                    ((FlagByte*)&slotCharacters[7].flags)->flag;
                for (int i = 0; i < kCourseFlagCount; i++) {
                    int availableInEitherRecord =
                        (sharedCourseFlags->test(i) | ((Bits14*)slotCourseFlags)->test(i)) != 0;
                    sharedCourseFlags->set(i, availableInEitherRecord);
                }
                slotCourseFlags += sizeof(PlayerRecordView);
                slotCharacters = (SavedCharacterEquipment*)((unsigned char*)slotCharacters + sizeof(PlayerRecordView));
            }
            // Phase 3: reset selected shared secondary fields; slot contents remain intact.
            // These are not the primary/alternate inventory banks merged above.
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.unresolvedFlagWord = 0;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[0] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[1] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[2] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[3] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[4] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[5] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[6] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[7] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[8] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[9] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[10] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[11] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[12] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[13] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[14] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[15] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[16] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[17] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[18] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[19] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[20] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[21] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[22] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[23] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.partSizedFlags[24] = s_saveFlagsPartBankDefault;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.unresolvedSmallFlags[0] = 0;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.unresolvedSmallFlags[1] = 0;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.unresolvedSmallFlags[2] = 0;
            AsSaveRecordSet(saveData)->sharedRecord.secondaryBanks.unresolvedSmallFlags[3] = 0;
        }
    } else {
        AsSaveRecordSet(saveData)->useSharedRecord = 0;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return whether save access currently uses the shared record.
unsigned char SaveData_IsSharedRecordMode(unsigned char* saveData) {
    return AsSaveRecordSet(saveData)->useSharedRecord;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Test the slot-used word for a valid save-slot index.
unsigned int SaveData_IsSlotUsed(unsigned char* saveData, int slotIndex) {
    if (slotIndex < 0 || slotIndex >= kSaveSlotCount)
        return 0;
    unsigned int value = AsSaveRecordSet(saveData)->slots[slotIndex].firstNameCodeUnit;
    return value != 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return a slot record, falling back to slot zero for an invalid index.
unsigned char* SaveData_GetSlotRecord(unsigned char* saveData, int slotIndex) {
    if (slotIndex < 0 || slotIndex >= kSaveSlotCount)
        return (unsigned char*)AsSaveRecordSet(saveData)->slots;
    return (unsigned char*)&AsSaveRecordSet(saveData)->slots[slotIndex];
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return the shared record when enabled, otherwise the selected slot.
unsigned char* SaveData_GetCurrentRecord(unsigned char* saveData) {
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        return (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    return (unsigned char*)&AsSaveRecordSet(saveData)->slots[AsSaveRecordSet(saveData)->currentSlot];
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Copy all persisted regions into the active record, retaining the original copy widths.
void SaveData_SetCurrentRecord(unsigned char* saveData, unsigned char* source) {
    unsigned char* destination;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        destination = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else
        destination = (unsigned char*)&AsSaveRecordSet(saveData)->slots[AsSaveRecordSet(saveData)->currentSlot];

    *(RecW151*)(destination + 0x000) = *(RecW151*)(source + 0x000);
    *(RecW28*)(destination + kRecordCourseRecordsOffset) = *(RecW28*)(source + kRecordCourseRecordsOffset);
    *(RecH208*)(destination + kRecordCharactersOffset) = *(RecH208*)(source + kRecordCharactersOffset);
    *(RecW16*)(destination + kRecordCaddiesOffset) = *(RecW16*)(source + kRecordCaddiesOffset);
    *(RecB24*)(destination + kRecordClubSetsOffset) = *(RecB24*)(source + kRecordClubSetsOffset);
    *(RecB6*)(destination + kRecordClubSetsTailOffset) = *(RecB6*)(source + kRecordClubSetsTailOffset);
    *(RecB11*)(destination + kRecordBallsOffset) = *(RecB11*)(source + kRecordBallsOffset);
    *(RecW5H414*)(destination + kRecordSystemSettingsOffset) = *(RecW5H414*)(source + kRecordSystemSettingsOffset);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Replace only the active record inventory/statistics prefix through offset 0x25C.
void SaveData_SetInventoryAndStatisticsPrefix(unsigned char* saveData, unsigned char* source) {
    unsigned char* destination;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        destination = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else
        destination = (unsigned char*)&AsSaveRecordSet(saveData)->slots[AsSaveRecordSet(saveData)->currentSlot];

    *(RecordPrefix0x25C*)destination = *(RecordPrefix0x25C*)source;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw off
// Write one character outfit into the active save record, preserving each original field store.
// Entry contract: saveData is a complete initialized record set; currentSlot is
// 0..3 unless useSharedRecord selects sharedRecord. characterIndex MUST be 0..7:
// the signed index is multiplied by 0x34 without a guard. equipment borrows a
// complete, suitably aligned 0x34-byte CharacterEquipmentTransfer; no retain.
// It may equal the destination exactly (self-copy), or be disjoint from it.
// Partial overlap is not a snapshot copy: early stores may overwrite later
// source fields, so callers must not rely on memmove semantics. The existing
// SaveRewards_SetCharacterFlags caller (save_rewards_item_unlock_flags.cpp) passes a separate stack copy.
// Writes flags, 24 part halfwords, hair and auxiliary bytes; leaves +1 padding.
// No validation, allocation or failure result; invalid inputs can corrupt saves.
void SaveData_SetCharacterEquipmentRecord(unsigned char* saveData, const CharacterEquipmentTransfer* equipment,
                                          int characterIndex) {
    unsigned char* record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else {
        saveData += AsSaveRecordSet(saveData)->currentSlot * sizeof(PlayerRecordView);
        record = saveData + IFF_OFFSET(SaveRecordSetView, slots);
    }
    CharacterEquipmentTransfer* savedEquipment = (CharacterEquipmentTransfer*)(record + characterIndex * 0x34 + 0x2CC);
    savedEquipment->flags = equipment->flags;
    savedEquipment->partIndices[0] = equipment->partIndices[0];
    savedEquipment->partIndices[1] = equipment->partIndices[1];
    savedEquipment->partIndices[2] = equipment->partIndices[2];
    savedEquipment->partIndices[3] = equipment->partIndices[3];
    savedEquipment->partIndices[4] = equipment->partIndices[4];
    savedEquipment->partIndices[5] = equipment->partIndices[5];
    savedEquipment->partIndices[6] = equipment->partIndices[6];
    savedEquipment->partIndices[7] = equipment->partIndices[7];
    savedEquipment->partIndices[8] = equipment->partIndices[8];
    savedEquipment->partIndices[9] = equipment->partIndices[9];
    savedEquipment->partIndices[10] = equipment->partIndices[10];
    savedEquipment->partIndices[11] = equipment->partIndices[11];
    savedEquipment->partIndices[12] = equipment->partIndices[12];
    savedEquipment->partIndices[13] = equipment->partIndices[13];
    savedEquipment->partIndices[14] = equipment->partIndices[14];
    savedEquipment->partIndices[15] = equipment->partIndices[15];
    savedEquipment->partIndices[16] = equipment->partIndices[16];
    savedEquipment->partIndices[17] = equipment->partIndices[17];
    savedEquipment->partIndices[18] = equipment->partIndices[18];
    savedEquipment->partIndices[19] = equipment->partIndices[19];
    savedEquipment->partIndices[20] = equipment->partIndices[20];
    savedEquipment->partIndices[21] = equipment->partIndices[21];
    savedEquipment->partIndices[22] = equipment->partIndices[22];
    savedEquipment->partIndices[23] = equipment->partIndices[23];
    savedEquipment->hairStyleIndex = equipment->hairStyleIndex;
    savedEquipment->auxPartIndex = equipment->auxPartIndex;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Write a caddie flag byte and value word into the active save record.
// Required initialized SaveRecordSetView; currentSlot is 0..3 unless shared
// mode is selected. caddieIndex is 0..7 (PlayerRecordView::caddies[8]), NOT a
// type ID; no index check occurs. Source borrows one aligned 8-byte
// SavedCaddieEquipment: +0 flags, +1..3 padding, +4 unsigned value word.
// Only +0/+4 are copied; destination padding is preserved. The reward producer
// SaveRewards_SetCaddieFlags (save_rewards_item_unlock_flags.cpp) passes a separate stack record.
// Exact self-copy is safe; otherwise require disjoint source/destination:
// the byte store precedes the word load, so this is not a snapshot/memmove.
// No ownership transfer, allocation, validation or failure status.
void SaveData_SetCaddieRecordAt(unsigned char* saveData, const unsigned char* caddieEquipment,
                                unsigned int caddieIndex) {
    unsigned char* record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else {
        saveData += *(unsigned int*)saveData * sizeof(PlayerRecordView);
        record = saveData + 8;
    }
    SavedCaddieEquipment* entry = &((PlayerRecordView*)record)->caddies[caddieIndex];
    const SavedCaddieEquipment* source = (const SavedCaddieEquipment*)caddieEquipment;
    entry->flags = source->flags;
    entry->value = source->value;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Write a club-set flag byte and upgrade level into the active save record.
// Same initialized record/slot contract as SetCaddieRecordAt. clubSetIndex is
// 0..14 (PlayerRecordView::clubSets[15]), NOT the misleading byte-size bound
// kClubSetLookupBound=30 used by a different lookup. No guard is performed.
// Source borrows two readable bytes in SavedClubSetEquipment order: flags +0,
// signed upgradeLevel +1. SaveRewards_SetClubSetFlags (save_rewards_item_unlock_flags.cpp) supplies its
// stack status pair; this setter copies bits without interpreting either byte.
// Exact self-copy or disjoint records only: stores follow source order and no
// overlap snapshot is taken. Writes both bytes; no retained pointer or status.
void SaveData_SetClubSetRecordAt(unsigned char* saveData, const unsigned char* clubSetEquipment,
                                 unsigned int clubSetIndex) {
    unsigned char* record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else {
        saveData += *(unsigned int*)saveData * sizeof(PlayerRecordView);
        record = saveData + 8;
    }
    SavedClubSetEquipment* entry = &((PlayerRecordView*)record)->clubSets[clubSetIndex];
    const SavedClubSetEquipment* source = (const SavedClubSetEquipment*)clubSetEquipment;
    entry->flags = source->flags;
    entry->upgradeLevel = source->upgradeLevel;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Write one saved ball flag byte by inventory index.
void SaveData_SetBallRecordAt(unsigned char* saveData, unsigned char* ballFlags, int ballIndex) {
    unsigned int record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned int)saveData + 0x20A8;
    else
        record =
            (unsigned int)(unsigned char*)&AsSaveRecordSet(saveData)->slots[AsSaveRecordSet(saveData)->currentSlot];
    ((unsigned char*)(record + ballIndex))[0x4CA] = *ballFlags;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Replace the active record mode-data region while preserving its original copy boundaries.
void SaveData_SetModeRecords(unsigned char* saveData, unsigned short* modeRecords) {
    unsigned char* record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else {
        saveData += *(unsigned int*)saveData * sizeof(PlayerRecordView);
        record = saveData + 8;
    }
    struct Halves {
        unsigned short a[0x197];
    };
    *(Halves*)(record + 0x4EC) = *(Halves*)modeRecords;
    struct Bytes {
        unsigned char a[9];
    };
    *(Bytes*)(record + 0x81A) = *(Bytes*)(((ModeRecordTransferView*)modeRecords)->areaOrder);
    *(unsigned short*)(record + 0x824) = ((ModeRecordTransferView*)modeRecords)->rewardCount;
    record[0x826] = ((unsigned char*)modeRecords)[0x33A];
}
#pragma pop

#pragma push
// Reject saved outfits whose nonzero part ids have the wrong item kind or character index.
int SaveData_ValidateCharacterOutfitParts(unsigned char* equipmentRecords) {
    unsigned char* characterRecord = equipmentRecords;
    unsigned char* partCursor;
    int characterIndex = 0;
    do {
        partCursor = characterRecord;
        int partSlot = 0;
        do {
            int typeId = ItemInventory_GetPartIdAt((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory,
                                                   *(short*)(partCursor + 2));
            if (typeId != 0) {
                if ((unsigned int)typeId >> kItemKindShift != kPartItemKind)
                    return 0;
                if ((typeId >> kCharacterIdShift & kCharacterIdMask) != characterIndex)
                    return 0;
            }
            partCursor += 2;
            partSlot++;
        } while (partSlot < kOutfitPartCount);
        characterRecord += 0x34;
        characterIndex++;
    } while (characterIndex < kCharacterCount);
    return 1;
}
#pragma pop

#pragma push
// Resolve a character id and copy its saved outfit flags, part indices, hair and ring.
int SaveData_CopyCharacterEquipmentRecord(unsigned char* saveData, unsigned char* equipment, void* characterKey) {
    unsigned char* saveBytes = saveData;
    unsigned char* resultBytes = equipment;
    int index =
        ItemInventory_FindCharacterIndex((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, characterKey);
    if (index < 0 || (unsigned int)index >= kCharacterLookupBound)
        return 0;
    unsigned char* characterRecordBytes;
    if (AsSaveRecordSet(saveBytes)->useSharedRecord != 0)
        characterRecordBytes = saveBytes + 0x20A8;
    else
        characterRecordBytes =
            (unsigned char*)&AsSaveRecordSet(saveBytes)->slots[AsSaveRecordSet(saveBytes)->currentSlot];
    characterRecordBytes += index * 0x34;
    resultBytes[0] = characterRecordBytes[0x2CC];
    unsigned short v2 = *(unsigned short*)(characterRecordBytes + 0x2D0),
                   v1 = *(unsigned short*)(characterRecordBytes + 0x2CE);
    *(unsigned short*)(resultBytes + 2) = v1;
    *(unsigned short*)(resultBytes + 4) = v2;
    unsigned short v4 = *(unsigned short*)(characterRecordBytes + 0x2D4),
                   v3 = *(unsigned short*)(characterRecordBytes + 0x2D2);
    *(unsigned short*)(resultBytes + 6) = v3;
    *(unsigned short*)(resultBytes + 8) = v4;
    unsigned short v6 = *(unsigned short*)(characterRecordBytes + 0x2D8),
                   v5 = *(unsigned short*)(characterRecordBytes + 0x2D6);
    *(unsigned short*)(resultBytes + 0xA) = v5;
    *(unsigned short*)(resultBytes + 0xC) = v6;
    unsigned short v8 = *(unsigned short*)(characterRecordBytes + 0x2DC),
                   v7 = *(unsigned short*)(characterRecordBytes + 0x2DA);
    *(unsigned short*)(resultBytes + 0xE) = v7;
    *(unsigned short*)(resultBytes + 0x10) = v8;
    unsigned short v10 = *(unsigned short*)(characterRecordBytes + 0x2E0),
                   v9 = *(unsigned short*)(characterRecordBytes + 0x2DE);
    *(unsigned short*)(resultBytes + 0x12) = v9;
    *(unsigned short*)(resultBytes + 0x14) = v10;
    unsigned short v12 = *(unsigned short*)(characterRecordBytes + 0x2E4),
                   v11 = *(unsigned short*)(characterRecordBytes + 0x2E2);
    *(unsigned short*)(resultBytes + 0x16) = v11;
    *(unsigned short*)(resultBytes + 0x18) = v12;
    unsigned short v14 = *(unsigned short*)(characterRecordBytes + 0x2E8),
                   v13 = *(unsigned short*)(characterRecordBytes + 0x2E6);
    *(unsigned short*)(resultBytes + 0x1A) = v13;
    *(unsigned short*)(resultBytes + 0x1C) = v14;
    unsigned short v16 = *(unsigned short*)(characterRecordBytes + 0x2EC),
                   v15 = *(unsigned short*)(characterRecordBytes + 0x2EA);
    *(unsigned short*)(resultBytes + 0x1E) = v15;
    *(unsigned short*)(resultBytes + 0x20) = v16;
    unsigned short v18 = *(unsigned short*)(characterRecordBytes + 0x2F0),
                   v17 = *(unsigned short*)(characterRecordBytes + 0x2EE);
    *(unsigned short*)(resultBytes + 0x22) = v17;
    *(unsigned short*)(resultBytes + 0x24) = v18;
    unsigned short v20 = *(unsigned short*)(characterRecordBytes + 0x2F4),
                   v19 = *(unsigned short*)(characterRecordBytes + 0x2F2);
    *(unsigned short*)(resultBytes + 0x26) = v19;
    *(unsigned short*)(resultBytes + 0x28) = v20;
    unsigned short v22 = *(unsigned short*)(characterRecordBytes + 0x2F8),
                   v21 = *(unsigned short*)(characterRecordBytes + 0x2F6);
    *(unsigned short*)(resultBytes + 0x2A) = v21;
    *(unsigned short*)(resultBytes + 0x2C) = v22;
    unsigned short v24 = *(unsigned short*)(characterRecordBytes + 0x2FC),
                   v23 = *(unsigned short*)(characterRecordBytes + 0x2FA);
    *(unsigned short*)(resultBytes + 0x2E) = v23;
    *(unsigned short*)(resultBytes + 0x30) = v24;
    resultBytes[0x32] = characterRecordBytes[0x2FE];
    resultBytes[0x33] = characterRecordBytes[0x2FF];
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw off
// Copy an outfit by character inventory index, using the existing lookup limit.
int SaveData_CopyCharacterEquipmentRecordAt(unsigned char* saveData, unsigned char* equipment, int characterIndex) {
    if (characterIndex < 0 || (unsigned int)characterIndex >= kCharacterLookupBound)
        return 0;
    unsigned char* record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else {
        saveData += *(unsigned int*)saveData * sizeof(PlayerRecordView);
        record = saveData + 8;
    }
    unsigned char* characterRecordBytes = record + characterIndex * 0x34;
    equipment[0] = characterRecordBytes[0x2CC];
    struct Halves {
        unsigned short a[24];
    };
    *(Halves*)(equipment + 2) = *(Halves*)(characterRecordBytes + 0x2CE);
    equipment[0x32] = characterRecordBytes[0x2FE];
    equipment[0x33] = characterRecordBytes[0x2FF];
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Resolve a caddie id and copy its saved flag byte and value word.
int SaveData_CopyCaddieRecord(unsigned char* saveData, unsigned char* caddieEquipment, void* caddieKey) {
    int index = ItemInventory_FindCaddieIndex((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, caddieKey);
    if (index < 0 || (unsigned int)index >= 0x40)
        return 0;
    unsigned char* record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else
        record = (unsigned char*)&AsSaveRecordSet(saveData)->slots[AsSaveRecordSet(saveData)->currentSlot];
    unsigned char* entry = record + index * 8;
    caddieEquipment[0] = entry[0x46C];
    *(unsigned int*)(caddieEquipment + 4) = *(unsigned int*)(entry + 0x470);
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Resolve a club-set id and copy its saved flag byte and upgrade level.
// The two flag bits are copied unchanged; this accessor does not assign ownership meaning.
int SaveData_CopyClubSetRecord(unsigned char* saveData, unsigned char* clubSetEquipment, void* clubSetKey) {
    int index = ItemInventory_FindClubSetIndex((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, clubSetKey);
    if (index < 0 || (unsigned int)index >= kClubSetLookupBound)
        return 0;
    unsigned char* record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
    else
        record = (unsigned char*)&AsSaveRecordSet(saveData)->slots[AsSaveRecordSet(saveData)->currentSlot];
    unsigned char* entry = record + index * 2;
    clubSetEquipment[0] = entry[0x4AC];
    clubSetEquipment[1] = entry[0x4AD];
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Resolve a ball id and copy its saved flag byte.
int SaveData_CopyBallRecord(unsigned char* saveData, unsigned char* ballFlags, void* ballKey) {
    int index = ItemInventory_FindBallIndex((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, ballKey);
    if (index < 0 || (unsigned int)index >= 11)
        return 0;
    unsigned int record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned int)saveData + 0x20A8;
    else
        record =
            (unsigned int)(unsigned char*)&AsSaveRecordSet(saveData)->slots[AsSaveRecordSet(saveData)->currentSlot];
    unsigned char* entry = (unsigned char*)(record + index);
    *ballFlags = entry[0x4CA];
    return 1;
}
#pragma pop

#pragma push
// Resolve a part ID and test the selected primary/alternate bank of the active record.
int SaveData_TestPartFlag(void* saveData, void* partKey, int useAlternateFlags) {
    int index = ItemInventory_FindPartIndex((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, partKey);
    if (index < 0 || (unsigned int)index >= 0x30E)
        return 0;
    unsigned char* saveBytes = (unsigned char*)saveData;
    unsigned char* record;
    if (useAlternateFlags != 0) {
        if (AsSaveRecordSet(saveBytes)->useSharedRecord != 0)
            record = saveBytes + 0x20A8;
        else
            record = (unsigned char*)&AsSaveRecordSet(saveBytes)->slots[AsSaveRecordSet(saveBytes)->currentSlot];
        return check782(AsPlayerRecord(record)->alternatePartFlags, index);
    }
    if (AsSaveRecordSet(saveBytes)->useSharedRecord != 0)
        record = saveBytes + 0x20A8;
    else
        record = (unsigned char*)&AsSaveRecordSet(saveBytes)->slots[AsSaveRecordSet(saveBytes)->currentSlot];
    return check782(AsPlayerRecord(record)->primaryPartFlags, index);
}
#pragma pop

#pragma push
// Look up caddie equipment and test the selected flag bank in the active record.
int SaveData_TestCaddieItemFlag(void* saveData, void* caddieItemKey, int useAlternateFlags) {
    int index =
        ItemInventory_FindCaddieItemIndex((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, caddieItemKey);
    if (index < 0 || (unsigned int)index >= 0x20)
        return 0;
    unsigned char* saveBytes = (unsigned char*)saveData;
    unsigned char* record;
    if (useAlternateFlags != 0) {
        if (AsSaveRecordSet(saveBytes)->useSharedRecord != 0)
            record = (unsigned char*)&AsSaveRecordSet(saveBytes)->sharedRecord;
        else
            record = (unsigned char*)&AsSaveRecordSet(saveBytes)->slots[AsSaveRecordSet(saveBytes)->currentSlot];
        return checkBits(&AsPlayerRecord(record)->alternateCaddieItemFlags, index);
    }
    if (AsSaveRecordSet(saveBytes)->useSharedRecord != 0)
        record = (unsigned char*)&AsSaveRecordSet(saveBytes)->sharedRecord;
    else
        record = (unsigned char*)&AsSaveRecordSet(saveBytes)->slots[AsSaveRecordSet(saveBytes)->currentSlot];
    return checkBits(&AsPlayerRecord(record)->primaryCaddieItemFlags, index);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return whether the active save record has a nonzero flag for the requested ring.
int SaveData_HasAuxPart(unsigned char* saveData, void* auxPartKey) {
    int index = ItemInventory_FindAuxPartIndex((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory,
                                               auxPartKey); // The auxiliary-part key is supplied by the caller.
    if (index < 0 || index >= 7)
        return 0;
    unsigned int record;
    if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
        record = (unsigned int)saveData + IFF_OFFSET(SaveRecordSetView, sharedRecord);
    else
        record = (unsigned int)saveData + AsSaveRecordSet(saveData)->currentSlot * sizeof(PlayerRecordView) +
                 IFF_OFFSET(SaveRecordSetView, slots);
    // Keep index first in the address expression to preserve the original add operands.
    return *(signed char*)(record + index + IFF_OFFSET(PlayerRecordView, auxPartFlags)) != 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Check consumable availability from equipped slots or spare count, rejecting the unavailable-id flag.
int SaveData_IsConsumableAvailable(unsigned char* saveData, unsigned int itemTypeId, void* checkInventory) {
    int itemIndex;
    int equipped;
    unsigned char* record;
    int result;
    itemIndex = ItemInventory_FindItemIndex((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, itemTypeId);
    if (checkInventory != 0) {

        if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
            record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
        else
            record = saveData + AsSaveRecordSet(saveData)->currentSlot * sizeof(PlayerRecordView) +
                     IFF_OFFSET(SaveRecordSetView, slots);
        equipped = 0;
        if (itemIndex == AsPlayerRecord(record)->equippedItemIndices[0])
            equipped = 1;
        else if (itemIndex == AsPlayerRecord(record)->equippedItemIndices[1])
            equipped = 1;
        else if (itemIndex == AsPlayerRecord(record)->equippedItemIndices[2])
            equipped = 1;
        else if (itemIndex == AsPlayerRecord(record)->equippedItemIndices[3])
            equipped = 1;
        else if (itemIndex == AsPlayerRecord(record)->equippedItemIndices[4])
            equipped = 1;
        else if (itemIndex == AsPlayerRecord(record)->equippedItemIndices[5])
            equipped = 1;
        else if (itemIndex == AsPlayerRecord(record)->equippedItemIndices[6])
            equipped = 1;
        else if (itemIndex == AsPlayerRecord(record)->equippedItemIndices[7])
            equipped = 1;
        result = 0;
        if (equipped || AsPlayerRecord(record)->itemCounts[itemIndex] > 0) {
            if (!(itemTypeId & kUnavailableItemFlag))
                result = 1;
        }
        return result;
    } else {
        return (itemTypeId & kUnavailableItemFlag) == 0;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Expand saved row indices into character/part IDs and the packed hair/base variant.
int AvatarData_ResolveSavedOutfitIds(unsigned char* saveData, unsigned char* resolvedEquipment, int characterIndex) {
    Memory_Clear(resolvedEquipment, 0x94);
    unsigned char* inventory = (unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory;
    if (inventory == 0)
        return 0;
    *(int*)resolvedEquipment = ItemInventory_GetCharacterIdAt(inventory, characterIndex);
    *(int*)(resolvedEquipment + 4) = characterIndex;
    struct Info {
        unsigned char b0;
        unsigned char pad;
        short h[24];
        signed char c;
        unsigned char d;
    };
    typedef IffRecordStorage<IFF_STRUCT::sHairStyle> HairStyleCopy;
    Info equipment;
    HairStyleCopy hairStyleRecord;
    int hasEquipmentRecord;
    unsigned char* entry;
    if (characterIndex < 0 || (unsigned int)characterIndex >= kCharacterLookupBound) {
        hasEquipmentRecord = 0;
    } else {
        unsigned char* record;
        if (AsSaveRecordSet(saveData)->useSharedRecord != 0)
            record = (unsigned char*)&AsSaveRecordSet(saveData)->sharedRecord;
        else
            record = (unsigned char*)&AsSaveRecordSet(saveData)->slots[AsSaveRecordSet(saveData)->currentSlot];
        entry = record + (unsigned int)characterIndex * 0x34;
        equipment.b0 = entry[0x2CC];
        struct Halves {
            unsigned short a[24];
        };
        *(Halves*)equipment.h = *(Halves*)(entry + 0x2CE);
        equipment.c = entry[0x2FE];
        equipment.d = entry[0x2FF];
        hasEquipmentRecord = 1;
    }
    if (hasEquipmentRecord == 0)
        return 0;
    for (int i = 0; i < kOutfitPartCount; i++)
        ((int*)(resolvedEquipment + 8))[i] = ItemInventory_GetPartIdAt(inventory, equipment.h[i]);
    int hairStyleId = ItemInventory_GetHairStyleIdAt(inventory, equipment.c);
    if (hairStyleId == 0)
        return 0;
    hairStyleRecord = *(HairStyleCopy*)ItemInventory_FindHairStyleData(inventory, hairStyleId);
    {
        struct B68 {
            CW_BITS2(unsigned char hi : 5, unsigned char lo : 3)
        };
        ((B68*)(resolvedEquipment + 0x68))->hi = hairStyleRecord.value.colourIndex;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Resolve the saved outfit, build the selection, and refresh primary items if validation fails.
int SaveData_LoadValidatedItemSelection(void* saveData, void* selection, signed char characterIndex) {
    struct Local {
        unsigned char output[20];
        unsigned char pad[0x5C];
        unsigned char temp[20];
        unsigned char tail[0x14];
    } local;
    memset(local.temp, 0, 20);
    int valid;
    if (AvatarData_ResolveSavedOutfitIds((unsigned char*)saveData, local.output, characterIndex) != 0)
        valid = ItemSelection_BuildOutfitFromRecord(selection, local.output);
    else
        valid = 0;
    if (valid == 0)
        return 0;
    if (ItemSelection_Validate(selection) == 0)
        ItemSelection_RefreshPrimaryItems(selection);
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation reset
// Seed shared availability bits with the fixed starter masks and course selections.
extern "C" int SaveData_SeedSharedAvailabilityFlags(unsigned char* saveData) {
    unsigned char character0Flags = saveData[0x2374];
    unsigned char character1Flags = saveData[0x23A8];
    unsigned char character2Flags = saveData[0x23DC];
    character0Flags |= 0x40;
    unsigned char character3Flags = saveData[0x2410];
    character1Flags |= 0x40;
    character2Flags |= 0x40;
    saveData[0x2374] = character0Flags;
    character3Flags |= 0x40;
    saveData[0x23A8] = character1Flags;
    saveData[0x23DC] = character2Flags;
    saveData[0x2410] = character3Flags;
    SharedCourseFlagsView* sharedCourseFlags = (SharedCourseFlagsView*)saveData;
    for (int i = 0; i < kDefaultUnlockedCourseCount; i++)
        sharedCourseFlags->bits.set(i, 1);
    unsigned int* p4 = (unsigned int*)(saveData + 0x2148);
    unsigned int* p0 = (unsigned int*)(saveData + 0x21B8);
    unsigned int v4 = *p4;
    unsigned int v0 = *p0;
    v4 |= 0x1111;
    v0 |= 0x1111;
    v4 |= 0x11110000;
    v0 |= 0x11110000;
    *p4 = v4;
    *p0 = v0;
    return 1;
}
#pragma pop

// Write 25 serialized words in address order; callers already loaded the default.
// T preserves volatile stores for shared/trailing scratch and ordinary slot stores.
template <class T> static inline void SeedSaveBank25(T* bank, unsigned int value) {
    bank[0] = value;
    bank[1] = value;
    bank[2] = value;
    bank[3] = value;
    bank[4] = value;
    bank[5] = value;
    bank[6] = value;
    bank[7] = value;
    bank[8] = value;
    bank[9] = value;
    bank[10] = value;
    bank[11] = value;
    bank[12] = value;
    bank[13] = value;
    bank[14] = value;
    bank[15] = value;
    bank[16] = value;
    bank[17] = value;
    bank[18] = value;
    bank[19] = value;
    bank[20] = value;
    bank[21] = value;
    bank[22] = value;
    bank[23] = value;
    bank[24] = value;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Lazily initialize the save singleton and its record defaults, then ensure IFF data is loaded.
extern "C" void* SaveData_AcquireInitializedInstance() {
    if (((volatile unsigned int&)GetSaveDataSlot()) == 0) {
        if (s_saveDataInitialized == 0) {
            unsigned int sharedTailDefaultWord;
            unsigned int slotTailDefaultWord;
            unsigned int slotZeroWord;
            unsigned int zero;
            unsigned int sharedZeroWord;
            unsigned int slotDefaultWord;
            unsigned char* saveSetStorage;
            unsigned char* slotCursor;
            unsigned int literalZero;
            unsigned int sharedDefaultWord;
            unsigned char* saveSetBase;
            saveSetBase = s_saveData;
            // Phase 1: seed flag/statistic constructors for the four slot records.
            slotCursor = (unsigned char*)&AsSaveRecordSet(saveSetBase)->slots[0];
            slotDefaultWord = s_saveRecordDefaultWord;
            slotZeroWord = s_saveRecordZeroWord;
            slotTailDefaultWord = s_saveRecordDefaultTailWord;
            zero = 0;
            do {
                // Seed the primary part bank; defaults are stored words, not inferred ownership.
                SeedSaveBank25(AsPlayerRecord(slotCursor)->primaryPartFlags, slotDefaultWord);
                AsPlayerRecord(slotCursor)->primaryCaddieItemFlags = zero;
                AsPlayerRecord(slotCursor)->reservedFlags[0] = slotZeroWord;
                AsPlayerRecord(slotCursor)->reservedFlags[1] = slotZeroWord;
                // Seed the alternate part bank independently of the primary bank.
                SeedSaveBank25(AsPlayerRecord(slotCursor)->alternatePartFlags, slotDefaultWord);
                AsPlayerRecord(slotCursor)->alternateCaddieItemFlags = zero;
                AsPlayerRecord(slotCursor)->ownedHairStyles[0] = slotZeroWord;
                AsPlayerRecord(slotCursor)->ownedHairStyles[1] = slotZeroWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.unresolvedFlagWord = zero;
                SeedSaveBank25(AsPlayerRecord(slotCursor)->secondaryBanks.partSizedFlags, slotDefaultWord);
                AsPlayerRecord(slotCursor)->secondaryBanks.sixWordFlags[0] = slotTailDefaultWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.sixWordFlags[1] = slotTailDefaultWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.sixWordFlags[2] = slotTailDefaultWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.sixWordFlags[3] = slotTailDefaultWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.sixWordFlags[4] = slotTailDefaultWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.sixWordFlags[5] = slotTailDefaultWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.unresolvedSmallFlags[0] = zero;
                AsPlayerRecord(slotCursor)->secondaryBanks.unresolvedSmallFlags[1] = zero;
                AsPlayerRecord(slotCursor)->secondaryBanks.unresolvedSmallFlags[2] = zero;
                AsPlayerRecord(slotCursor)->secondaryBanks.unresolvedSmallFlags[3] = zero;
                AsPlayerRecord(slotCursor)->secondaryBanks.checked45BitFlags[0] = slotZeroWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.checked45BitFlags[1] = slotZeroWord;
                AsPlayerRecord(slotCursor)->secondaryBanks.unresolvedTrailingFlag = zero;
                AsPlayerRecord(slotCursor)->unlockedCourses = zero;
                slotCursor += sizeof(PlayerRecordView);
            } while (slotCursor < (unsigned char*)&AsSaveRecordSet(saveSetBase)->sharedRecord);
            // Phase 2: seed the separate shared record using the same stored defaults.
            sharedDefaultWord = s_saveRecordDefaultWord;
            sharedZeroWord = s_saveRecordZeroWord;
            saveSetStorage = s_saveData;
            literalZero = 0;
            SeedSaveBank25(((volatile SaveRecordSetView*)saveSetBase)->sharedRecord.primaryPartFlags,
                           sharedDefaultWord);
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.primaryCaddieItemFlags = literalZero;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.reservedFlags[0] = sharedZeroWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.reservedFlags[1] = sharedZeroWord;
            SeedSaveBank25(((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.alternatePartFlags,
                           sharedDefaultWord);
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.alternateCaddieItemFlags = literalZero;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.ownedHairStyles[0] = sharedZeroWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.ownedHairStyles[1] = sharedZeroWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.unresolvedFlagWord = literalZero;
            SeedSaveBank25(((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.partSizedFlags,
                           sharedDefaultWord);
            sharedTailDefaultWord = s_saveRecordDefaultTailWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.unresolvedSmallFlags[0] =
                literalZero;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.sixWordFlags[0] =
                sharedTailDefaultWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.sixWordFlags[1] =
                sharedTailDefaultWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.sixWordFlags[2] =
                sharedTailDefaultWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.sixWordFlags[3] =
                sharedTailDefaultWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.sixWordFlags[4] =
                sharedTailDefaultWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.sixWordFlags[5] =
                sharedTailDefaultWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.unresolvedSmallFlags[1] =
                literalZero;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.unresolvedSmallFlags[2] =
                literalZero;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.unresolvedSmallFlags[3] =
                literalZero;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.checked45BitFlags[0] =
                sharedZeroWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.checked45BitFlags[1] =
                sharedZeroWord;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.secondaryBanks.unresolvedTrailingFlag =
                literalZero;
            ((volatile SaveRecordSetView*)saveSetStorage)->sharedRecord.unlockedCourses = literalZero;
            // Phase 3: initialize the legacy 0x47C-byte prefix scratch after the record set.
            SeedSaveBank25(
                ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->primaryPartFlags,
                sharedDefaultWord);
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->primaryCaddieItemFlags =
                literalZero;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->reservedFlags[0] =
                sharedZeroWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->reservedFlags[1] =
                sharedZeroWord;
            SeedSaveBank25(
                ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->alternatePartFlags,
                sharedDefaultWord);
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->alternateCaddieItemFlags =
                literalZero;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->ownedHairStyles[0] =
                sharedZeroWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->ownedHairStyles[1] =
                sharedZeroWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.unresolvedFlagWord = literalZero;
            SeedSaveBank25(((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                               ->secondaryBanks.partSizedFlags,
                           sharedDefaultWord);
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.sixWordFlags[0] = sharedTailDefaultWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.sixWordFlags[1] = sharedTailDefaultWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.sixWordFlags[2] = sharedTailDefaultWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.sixWordFlags[3] = sharedTailDefaultWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.sixWordFlags[4] = sharedTailDefaultWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.sixWordFlags[5] = sharedTailDefaultWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.unresolvedSmallFlags[0] = literalZero;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.unresolvedSmallFlags[1] = literalZero;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.unresolvedSmallFlags[2] = literalZero;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.unresolvedSmallFlags[3] = literalZero;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.checked45BitFlags[0] = sharedZeroWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.checked45BitFlags[1] = sharedZeroWord;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))
                ->secondaryBanks.unresolvedTrailingFlag = literalZero;
            ((volatile PlayerRecordView*)(saveSetStorage + kTrailingRecordScratchOffset))->unlockedCourses =
                literalZero;
            // Phase 4: clear the trailing scratch, load IFF once, then initialize playable defaults.
            // Raw storage must include this trailing scratch beyond SaveRecordSetView.
            Memory_Clear(saveSetStorage + kTrailingRecordScratchOffset, kTrailingRecordScratchBytes);
            if (s_saveInventoryInitialized == 0) {
                ItemInventory_LoadIffArchive((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
                s_saveInventoryInitialized = 1;
            }
            *(unsigned int*)s_saveData = 0;
            s_saveData[4] = 0;
            s_saveData[5] = 0;
            s_saveData[6] = 0;
            for (int i = 0; i < kSaveSlotCount; ++i)
                SaveData_InitBlankRecord(s_saveData, i, 0);
            SaveData_InitBlankRecord(s_saveData, 0, 1);
            s_saveDataInitialized = 1;
        }
        ((volatile unsigned int&)GetSaveDataSlot()) = (unsigned int)s_saveData;
    }
    return (void*)GetSaveDataSlot();
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Initialize one save record flag bank and statistics defaults without clearing its other data.
extern "C" void SaveData_InitPlayerRecord(void* record) {
    // Seed the primary part bank; defaults are stored words, not inferred ownership.
    AsPlayerRecord(record)->primaryPartFlags[0] = s_saveRecordDefaultWord;
#pragma opt_propagation reset
    unsigned int zero = 0;
    AsPlayerRecord(record)->primaryPartFlags[1] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[2] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[3] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[4] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[5] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[6] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[7] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[8] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[9] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[10] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[11] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[12] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[13] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[14] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[15] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[16] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[17] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[18] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[19] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[20] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[21] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[22] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[23] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryPartFlags[24] = s_saveRecordDefaultWord;
    AsPlayerRecord(record)->primaryCaddieItemFlags = zero;
    AsPlayerRecord(record)->reservedFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->reservedFlags[1] = s_saveRecordZeroWord;
    // Seed the alternate part bank independently of the primary bank.
    SeedAlternatePartDefaults(record);
    AsPlayerRecord(record)->alternateCaddieItemFlags = zero;
    AsPlayerRecord(record)->ownedHairStyles[0] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->ownedHairStyles[1] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->secondaryBanks.unresolvedFlagWord = zero;
    SeedSecondaryPartSizedDefaults(record);
    SeedSecondarySixWordDefaults(record);
    AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[0] = zero;
    AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[1] = zero;
    AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[2] = zero;
    AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[3] = zero;
    AsPlayerRecord(record)->secondaryBanks.checked45BitFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->secondaryBanks.checked45BitFlags[1] = s_saveRecordZeroWord;
    AsPlayerRecord(record)->secondaryBanks.unresolvedTrailingFlag = zero;
    AsSavePlayerRecord(record)->unlockedCourses = zero;
}
#pragma pop

#pragma push
// Initialize flag defaults, then copy each persisted record region without copying alignment gaps.
void* SaveRecord_CopyCtor(void* destination, void* source) {
    int zero = 0;
    void* src = source;

    void* dst = destination;

    unsigned int* recordWords = (unsigned int*)destination;
    // Seed the primary part bank; defaults are stored words, not inferred ownership.
    SeedPrimaryPartDefaults(recordWords);
    AsPlayerRecord(recordWords)->primaryCaddieItemFlags = zero;
    AsPlayerRecord(recordWords)->reservedFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(recordWords)->reservedFlags[1] = s_saveRecordZeroWord;
    // Seed the alternate part bank independently of the primary bank.
    SeedAlternatePartDefaults(recordWords);
    AsPlayerRecord(recordWords)->alternateCaddieItemFlags = zero;
    AsPlayerRecord(recordWords)->ownedHairStyles[0] = s_saveRecordZeroWord;
    AsPlayerRecord(recordWords)->ownedHairStyles[1] = s_saveRecordZeroWord;
    AsPlayerRecord(recordWords)->secondaryBanks.unresolvedFlagWord = zero;
    SeedSecondaryPartSizedDefaults(recordWords);
    SeedSecondarySixWordDefaults(recordWords);
    AsPlayerRecord(recordWords)->secondaryBanks.unresolvedSmallFlags[0] = zero;
    AsPlayerRecord(recordWords)->secondaryBanks.unresolvedSmallFlags[1] = zero;
    AsPlayerRecord(recordWords)->secondaryBanks.unresolvedSmallFlags[2] = zero;
    AsPlayerRecord(recordWords)->secondaryBanks.unresolvedSmallFlags[3] = zero;
    AsPlayerRecord(recordWords)->unlockedCourses = zero;
    AsPlayerRecord(recordWords)->secondaryBanks.checked45BitFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(recordWords)->secondaryBanks.checked45BitFlags[1] = s_saveRecordZeroWord;
    AsPlayerRecord(recordWords)->secondaryBanks.unresolvedTrailingFlag = zero;
    Memory_Copy(dst, src, 0x25C);
    Memory_Copy(AsPlayerRecord(dst)->courseRecords, AsPlayerRecord(src)->courseRecords, 0x70);
    Memory_Copy(AsPlayerRecord(dst)->characters, AsPlayerRecord(src)->characters, 0x1A0);
    Memory_Copy(AsPlayerRecord(dst)->caddies, AsPlayerRecord(src)->caddies, 0x40);
    Memory_Copy(AsPlayerRecord(dst)->clubSets, AsPlayerRecord(src)->clubSets, 0x1E);
    Memory_Copy(AsPlayerRecord(dst)->balls, AsPlayerRecord(src)->balls, 0x0B);
    Memory_Copy(AsPlayerRecord(dst)->systemSettings, AsPlayerRecord(src)->systemSettings, 0x14);
    Memory_Copy(AsPlayerRecord(dst)->modeRecords, AsPlayerRecord(src)->modeRecords, 0x33C);
    return dst;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation reset
// Seed inventory and secondary-bank defaults in each of the four consecutive slot records.
extern "C" void SaveData_InitSlotRecordDefaults(void* slotRecords) {
    unsigned int zero = 0;
    unsigned int* end = (unsigned int*)((PlayerRecordView*)slotRecords + kSaveSlotCount);
    unsigned int* record = (unsigned int*)slotRecords;
    do {
        // Seed the primary part bank; defaults are stored words, not inferred ownership.
        SeedPrimaryPartDefaults(record);
        AsPlayerRecord(record)->primaryCaddieItemFlags = zero;
        AsPlayerRecord(record)->reservedFlags[0] = s_saveRecordZeroWord;
        AsPlayerRecord(record)->reservedFlags[1] = s_saveRecordZeroWord;
        // Seed the alternate part bank independently of the primary bank.
        SeedAlternatePartDefaults(record);
        AsPlayerRecord(record)->alternateCaddieItemFlags = zero;
        AsPlayerRecord(record)->ownedHairStyles[0] = s_saveRecordZeroWord;
        AsPlayerRecord(record)->ownedHairStyles[1] = s_saveRecordZeroWord;
        AsPlayerRecord(record)->secondaryBanks.unresolvedFlagWord = zero;
        SeedSecondaryPartSizedDefaults(record);
        SeedSecondarySixWordDefaults(record);
        AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[0] = zero;
        AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[1] = zero;
        AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[2] = zero;
        AsPlayerRecord(record)->secondaryBanks.unresolvedSmallFlags[3] = zero;
        AsPlayerRecord(record)->secondaryBanks.checked45BitFlags[0] = s_saveRecordZeroWord;
        AsPlayerRecord(record)->secondaryBanks.checked45BitFlags[1] = s_saveRecordZeroWord;
        AsPlayerRecord(record)->secondaryBanks.unresolvedTrailingFlag = zero;
        AsPlayerRecord(record)->unlockedCourses = zero;
        record = (unsigned int*)((unsigned char*)record + sizeof(PlayerRecordView));
    } while (record < end);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Initialize flag defaults for four slot records, then copy the full four-slot array.
void* SaveSlotArray_CopyCtor(void* destination, void* source) {
    unsigned int* p = (unsigned int*)destination;
    unsigned int zero = 0;
    unsigned int* end = p + kSaveSlotCount * sizeof(PlayerRecordView) / sizeof(unsigned int);
    do {
        // Seed the primary part bank; defaults are stored words, not inferred ownership.
        SeedPrimaryPartDefaults(p);
        AsPlayerRecord(p)->primaryCaddieItemFlags = zero;
        AsPlayerRecord(p)->reservedFlags[0] = s_saveRecordZeroWord;
        AsPlayerRecord(p)->reservedFlags[1] = s_saveRecordZeroWord;
        // Seed the alternate part bank independently of the primary bank.
        SeedAlternatePartDefaults(p);
        AsPlayerRecord(p)->alternateCaddieItemFlags = zero;
        AsPlayerRecord(p)->ownedHairStyles[0] = s_saveRecordZeroWord;
        AsPlayerRecord(p)->ownedHairStyles[1] = s_saveRecordZeroWord;
        AsPlayerRecord(p)->secondaryBanks.unresolvedFlagWord = zero;
        SeedSecondaryPartSizedDefaults(p);
        SeedSecondarySixWordDefaults(p);
        AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[0] = zero;
        AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[1] = zero;
        AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[2] = zero;
        AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[3] = zero;
        AsPlayerRecord(p)->secondaryBanks.checked45BitFlags[0] = s_saveRecordZeroWord;
        AsPlayerRecord(p)->secondaryBanks.checked45BitFlags[1] = s_saveRecordZeroWord;
        AsPlayerRecord(p)->secondaryBanks.unresolvedTrailingFlag = zero;
        AsPlayerRecord(p)->unlockedCourses = zero;
        p += 0x20A;
    } while (p < end);
    Memory_Copy(destination, source, 0x20A0);
    return destination;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation reset
// Initialize defaults, then clear the existing 0x47C-byte record prefix.
extern "C" void* SaveRecordPrefix_ClearCtor(void* record) {
    unsigned int* p = (unsigned int*)record;
    unsigned int zero = 0;
    // Seed the primary part bank; defaults are stored words, not inferred ownership.
    SeedPrimaryPartDefaults(p);
    AsPlayerRecord(p)->primaryCaddieItemFlags = zero;
    AsPlayerRecord(p)->reservedFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->reservedFlags[1] = s_saveRecordZeroWord;
    // Seed the alternate part bank independently of the primary bank.
    SeedAlternatePartDefaults(p);
    AsPlayerRecord(p)->alternateCaddieItemFlags = zero;
    AsPlayerRecord(p)->ownedHairStyles[0] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->ownedHairStyles[1] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->secondaryBanks.unresolvedFlagWord = zero;
    SeedSecondaryPartSizedDefaults(p);
    SeedSecondarySixWordDefaults(p);
    AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[0] = zero;
    AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[1] = zero;
    AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[2] = zero;
    AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[3] = zero;
    AsPlayerRecord(p)->secondaryBanks.checked45BitFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->secondaryBanks.checked45BitFlags[1] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->secondaryBanks.unresolvedTrailingFlag = zero;
    AsPlayerRecord(p)->unlockedCourses = zero;
    Memory_Clear(record, 0x47C);
    return record;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Initialize flag defaults, then copy the existing 0x47C-byte record prefix.
void* SaveRecordPrefix_CopyCtor(void* destination, void* source) {
    unsigned int zero = 0;
    unsigned int* p = (unsigned int*)destination;
    // Seed the primary part bank; defaults are stored words, not inferred ownership.
    SeedPrimaryPartDefaults(p);
    AsPlayerRecord(p)->primaryCaddieItemFlags = zero;
    AsPlayerRecord(p)->reservedFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->reservedFlags[1] = s_saveRecordZeroWord;
    // Seed the alternate part bank independently of the primary bank.
    SeedAlternatePartDefaults(p);
    AsPlayerRecord(p)->alternateCaddieItemFlags = zero;
    AsPlayerRecord(p)->ownedHairStyles[0] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->ownedHairStyles[1] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->secondaryBanks.unresolvedFlagWord = zero;
    SeedSecondaryPartSizedDefaults(p);
    SeedSecondarySixWordDefaults(p);
    AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[0] = zero;
    AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[1] = zero;
    AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[2] = zero;
    AsPlayerRecord(p)->secondaryBanks.unresolvedSmallFlags[3] = zero;
    AsPlayerRecord(p)->secondaryBanks.checked45BitFlags[0] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->secondaryBanks.checked45BitFlags[1] = s_saveRecordZeroWord;
    AsPlayerRecord(p)->secondaryBanks.unresolvedTrailingFlag = zero;
    AsPlayerRecord(p)->unlockedCourses = zero;
    Memory_Copy(destination, source, 0x47C);
    return destination;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Copy the existing 0x47C-byte record prefix; this API does not copy a complete slot.
void* SaveData_CopyRecordPrefix47C(void* destination, void* source) {
    Memory_Copy(destination, source, 0x47C);
    return destination;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return a part-table record by index, skipping its preceding lookup key.
unsigned char* ItemInventory_GetPartDataAt(unsigned char* inventory, int partIndex) {
    unsigned char* partEntry = (unsigned char*)((CItemManager*)inventory)->parts.data;
    if (partIndex < 0 || (unsigned int)partIndex >= ((CItemManager*)inventory)->parts.size)
        return 0;
    while (partIndex != 0) {
        partEntry += 0x1A0;
        partIndex--;
    }
    return partEntry + 4;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Return the embedded IFF inventory from the shared document.
unsigned char* SharedDoc_GetItemInventory() {
    return (unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Set or clear one checked part flag in a 782-bit flag set.
unsigned int* Bitset782_AssignBit(unsigned int* bits, unsigned int index, int value) {
    if (index >= kPartFlagCount) {
        std::out_of_range error(bitsetSetOutOfRangeMessage, outOfRangeExceptionTypeString);
    }
    unsigned int mask = 1u << (index & kFlagBitIndexMask);
    unsigned int word = index >> kFlagWordIndexShift;
    if (value != 0)
        *(bits + word) |= mask;
    else
        *(bits + word) &= ~mask;
    return bits;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Clear one checked part flag in a 782-bit flag set.
unsigned int* Bitset782_ResetBit(unsigned int* bits, unsigned int index) {
    if (index >= kPartFlagCount) {
        std::out_of_range error(bitset782ResetRangeErrorText, outOfRangeExceptionTypeString);
    }
    unsigned int word = index >> 5;
    *(bits + word) &= ~(1u << (index & 31));
    return bits;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_774E8(unsigned int* flags) {
    *flags = 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Set or clear one checked bit in a single-word flag set.
unsigned int* Bitset32_AssignBit(unsigned int* bits, unsigned int index, int value) {
    if (index >= 0x20) {
        std::out_of_range error(s_shopPurchaseBitRangeErrorText, outOfRangeExceptionTypeString);
    }
    unsigned int mask = 1u << index;
    if (value != 0)
        *bits |= mask;
    else
        *bits &= ~mask;
    return bits;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Initialize a two-word bitset to 32 set bits followed by 19 set bits.
void SaveFlags_Init51BitsSet(unsigned int* flagWords) {
    flagWords[0] = -1;
    flagWords[1] = (flagWords[0] << 13) >> 13;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw off
// Initialize all 25 words of a part-sized flag bank from its stored default word.
void SaveFlags_InitPartBank(unsigned int* flagWords) {
    flagWords[0] = s_saveFlagsPartBankDefault;
    flagWords[1] = s_saveFlagsPartBankDefault;
    flagWords[2] = s_saveFlagsPartBankDefault;
    flagWords[3] = s_saveFlagsPartBankDefault;
    flagWords[4] = s_saveFlagsPartBankDefault;
    flagWords[5] = s_saveFlagsPartBankDefault;
    flagWords[6] = s_saveFlagsPartBankDefault;
    flagWords[7] = s_saveFlagsPartBankDefault;
    flagWords[8] = s_saveFlagsPartBankDefault;
    flagWords[9] = s_saveFlagsPartBankDefault;
    flagWords[10] = s_saveFlagsPartBankDefault;
    flagWords[11] = s_saveFlagsPartBankDefault;
    flagWords[12] = s_saveFlagsPartBankDefault;
    flagWords[13] = s_saveFlagsPartBankDefault;
    flagWords[14] = s_saveFlagsPartBankDefault;
    flagWords[15] = s_saveFlagsPartBankDefault;
    flagWords[16] = s_saveFlagsPartBankDefault;
    flagWords[17] = s_saveFlagsPartBankDefault;
    flagWords[18] = s_saveFlagsPartBankDefault;
    flagWords[19] = s_saveFlagsPartBankDefault;
    flagWords[20] = s_saveFlagsPartBankDefault;
    flagWords[21] = s_saveFlagsPartBankDefault;
    flagWords[22] = s_saveFlagsPartBankDefault;
    flagWords[23] = s_saveFlagsPartBankDefault;
    flagWords[24] = s_saveFlagsPartBankDefault;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_776F8(unsigned int* flags) {
    *flags = 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Initialize six flag words from the same stored default.
void SaveFlags_InitSixDefaultWords(unsigned int* flagWords) {
    flagWords[0] = s_saveFlagsSixWordDefault;
    flagWords[1] = s_saveFlagsSixWordDefault;
    flagWords[2] = s_saveFlagsSixWordDefault;
    flagWords[3] = s_saveFlagsSixWordDefault;
    flagWords[4] = s_saveFlagsSixWordDefault;
    flagWords[5] = s_saveFlagsSixWordDefault;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_77738(unsigned int* flags) {
    *flags = 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_77744(unsigned int* flags) {
    *flags = 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Initialize both words of a small flag bank from the same stored default.
void SaveFlags_InitTwoDefaultWords(unsigned int* flagWords) {
    flagWords[0] = s_saveFlagsTwoWordDefault;
    flagWords[1] = s_saveFlagsTwoWordDefault;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Preserve the two ABI input words as a 64-bit pair without changing their values.
unsigned long long SaveRecord_PreserveWordPair(unsigned int highWord, unsigned int lowWord) {
    volatile unsigned int saved2;
    volatile unsigned int saved1;
    saved1 = highWord;
    saved2 = lowWord;
    // The Wii stores highWord:lowWord as one 64-bit big-endian value.
    return ((unsigned long long)highWord << 32) | lowWord;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Set or clear one of 45 bits after checking the bit index.
Ref45* Bitset45Reference_Assign(Ref45* ref, int value) {
    unsigned int i = ref->index;
    unsigned int* words = ref->words;
    if (i >= 0x2D) {
        std::out_of_range error(bitset45AssignRangeErrorText, outOfRangeExceptionTypeString);
    }
    unsigned int word = i >> 5;
    unsigned int mask = 1u << (i & 31);
    if (value != 0)
        *(words + word) |= mask;
    else
        *(words + word) &= ~mask;
    return ref;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Zero the caller-provided flag word; its owning field is not established here.
void SaveFlags_ClearWord_778B0(unsigned int* flags) {
    *flags = 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Clear the course-availability word passed by the new-inventory initializer.
void SaveFlags_ClearCourseAvailabilityWord(unsigned int* flags) {
    *flags = 0;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Set or clear one checked course flag in a 14-bit flag set.
unsigned int* Bitset14_AssignBit(unsigned int* bits, unsigned int index, int value) {
    if (index >= kCourseFlagCount) {
        std::out_of_range error(bitset14SetRangeErrorText, outOfRangeExceptionTypeString);
    }
    unsigned int mask = 1u << index;
    if (value != 0)
        *bits |= mask;
    else
        *bits &= ~mask;
    return bits;
}
#pragma pop

// The save file stores records as the Wii wrote them (big-endian fields).
// Save record layout: +0x00 name code units, +0x20 play time/+0x24 Pang, +0x3C part/caddie/hair flag banks
// (0x3C..0x113), +0x114 owned hair styles, +0x11C item counts, +0x140 secondary banks and statistics
// (0x140..0x217), +0x258 unlocked courses, +0x25C 14 course records, +0x2CC outfit part indices, +0x46C caddie
// values, +0x4E0 stream/buffer volume and control/audio options, +0x4EC 11 Tour area records (0x4A each).
// The packed 0x47C-byte profile ("Save Character Data" to the Wii Remote memory) carries the same fields at packed
// offsets (layout in CFileIoMenu::BuildCurrentPlayerProfile).
}
