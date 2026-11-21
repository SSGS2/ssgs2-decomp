#include <decomp/static_init.h>
#include "classes/CSharedDoc.h"
#include "classes/SavePlayerRecord.h"
#include "iff/IffStruct.h"

// Save storage contains four normal records, one temporary record, and a packed summary.
enum SaveStorageLayout {
    kSaveSlotCount = 4, kSaveRecordBytes = sizeof(SavePlayerRecord),
    kSaveRecordsOffset = 8, kTemporaryRecordOffset = 0x20A8,
    kPlayerSummaryOffset = 0x28D0, kPlayerSummaryBytes = 0x47C,
    kInitialPang = 3000, kInventoryPartCount = 782,
    kDefaultByteTableEntry = 8, kRecordWordCount = kSaveRecordBytes / 4,
    kCharacterCount = 8, kNoEquipmentIndex = -1,
    kPartBankWordCount = 25, kTailBankWordCount = 6,
    kHairStyleCount = 51, kAvailabilityBitCount = 45, kCourseInventoryCount = 14,
    kDefaultPartFlagShift = 5, kDefaultPartFlagMask = 7
};
// Inventory prefix shared by normal records and the packed summary.
// Legacy field names below are provisional.
// Offset map verified against SaveData_TestPartFlag/TestCaddieItemFlag in save_data_records_and_defaults.cpp:
// +0x03C primary part flags; +0x0A0 primary caddie-item flags; +0x0A4 reserved
// flag words (not established as hair flags); +0x0AC alternate part flags;
// +0x110 alternate caddie-item flags; +0x114 owned hairstyles.
// The name at +0 is UTF-16; its first code unit doubles as the empty-slot test.
struct SaveInventoryPrefix {
    unsigned short playerName[16];
    // +0x20 is elapsed play time in 30 Hz save ticks (load-menu display divides by 30).
    unsigned int playTicks, unk_24, pang; // +0x24's rank meaning is unconfirmed.
    signed char characterIndex, clubSetIndex, ballIndex, caddieIndex;
    signed char caddieItemIndex;
    signed char equippedItemIndices[8];
    char alignmentPadding[3];
    unsigned int primaryPartFlags[kPartBankWordCount];
    unsigned int primaryCaddieItemFlags, reservedFlagWords[2], alternatePartFlags[kPartBankWordCount];
    unsigned int alternateCaddieItemFlags, ownedHairStyles[2];
};
typedef char SaveInventoryPrefix_size[(sizeof(SaveInventoryPrefix) == 0x11C) ? 1 : -1];
enum SaveInventoryOffsets {
    kPrimaryPartFlagsOffset = (int)&((SaveInventoryPrefix*)0)->primaryPartFlags,
    kPrimaryCaddieItemFlagsOffset = (int)&((SaveInventoryPrefix*)0)->primaryCaddieItemFlags,
    kReservedFlagWordsOffset = (int)&((SaveInventoryPrefix*)0)->reservedFlagWords,
    kAlternatePartFlagsOffset = (int)&((SaveInventoryPrefix*)0)->alternatePartFlags,
    kAlternateCaddieItemFlagsOffset = (int)&((SaveInventoryPrefix*)0)->alternateCaddieItemFlags,
    kOwnedHairStylesOffset = (int)&((SaveInventoryPrefix*)0)->ownedHairStyles,
    kUnlockedCoursesOffset = (int)&((SavePlayerRecord*)0)->unlockedCourses
};
// Identifier domains for mods (these domains are not interchangeable):
// * part flags: index into ItemInventory_GetPartDataAt, [0,782), not an IFF type ID;
// * caddie-item flags: 32-bit table-index bank; defaults set indices 0,4,...,28;
// * hairstyle flags: 51 usable bits, but no display-name mapping is established here;
// * course availability: 14 course-inventory indices; starter indices are 0,2,5;
// * quantities: 13 halfwords, distinct from the adjacent seven auxiliary flag bytes;
// * +0x144/+0x1A8/+0x1D0: initializer shape is known; gameplay ID domains remain unresolved.
// Starter policy lives in GrantStarterPartIndices / InitDefaultInventoryRecord.
// The Seed* helpers reproduce overwritten constructor stores and are not unlock policy.

// Typed view of the 0x25C-byte prefix initialized here. Names without a proven
// gameplay role deliberately retain their byte offset. These are serialized
// banks: a word bit index is not a byte-table index or an inventory item ID.
struct SaveRecordSeedView {
    char identityAndEquipment[0x3C];
    unsigned int primaryPartFlags[kPartBankWordCount];                 // +03C, 782 usable bits
    unsigned int primaryCaddieItemFlags;               // +0A0
    unsigned int reservedFlagsA4[2];                   // +0A4, role unresolved
    unsigned int alternatePartFlags[kPartBankWordCount];               // +0AC, 782 usable bits
    unsigned int alternateCaddieItemFlags;             // +110
    unsigned int ownedHairStyles[2];                   // +114, 51 usable bits
    unsigned short itemQuantities[13];                // +11C; keep halfword units
    unsigned char auxiliaryFlags136[7], padding13D[3];
    unsigned int flags140, opaquePartBank144[kPartBankWordCount];      // +144 uses the part-bank initializer
    unsigned int opaqueDefaultBank1A8[kTailBankWordCount];              // +1A8; six serialized words
    unsigned int flags1C0, flags1C4, flags1C8, flags1CC;
    unsigned int availability1D0[2];                  // 45 usable bits; IDs unresolved
    unsigned int flags1D8, opaqueWords1DC[10];            // individual word meanings unresolved
    float scalars204[3];                              // +204/+208/+20C, units unresolved
    unsigned int word210;
    float scalar214;                                  // units unresolved
    unsigned char defaultBytes218[64];                // each byte initially 8, NOT a bitset
    unsigned int unlockedCourses;                    // +258, 14 course inventory bits
};
typedef char SaveRecordSeedView_size[sizeof(SaveRecordSeedView) == 0x25C ? 1 : -1];
typedef char SaveRecordSeedView_table_offset[((int)&((SaveRecordSeedView*)0)->defaultBytes218 == 0x218) ? 1 : -1];
enum SaveSeedOffsets {
    kSeedPrimaryPartFlagsOffset = (int)&((SaveRecordSeedView*)0)->primaryPartFlags,
    kSeedPrimaryCaddieItemFlagsOffset = (int)&((SaveRecordSeedView*)0)->primaryCaddieItemFlags,
    kSeedReservedFlagsA4Offset = (int)&((SaveRecordSeedView*)0)->reservedFlagsA4,
    kSeedAlternatePartFlagsOffset = (int)&((SaveRecordSeedView*)0)->alternatePartFlags,
    kSeedAlternateCaddieItemFlagsOffset = (int)&((SaveRecordSeedView*)0)->alternateCaddieItemFlags,
    kSeedOwnedHairStylesOffset = (int)&((SaveRecordSeedView*)0)->ownedHairStyles,
    kSeedItemQuantitiesOffset = (int)&((SaveRecordSeedView*)0)->itemQuantities,
    kSeedAuxiliaryFlags136Offset = (int)&((SaveRecordSeedView*)0)->auxiliaryFlags136,
    kSeedFlags140Offset = (int)&((SaveRecordSeedView*)0)->flags140,
    kSeedOpaquePartBank144Offset = (int)&((SaveRecordSeedView*)0)->opaquePartBank144,
    kSeedOpaqueDefaultBank1A8Offset = (int)&((SaveRecordSeedView*)0)->opaqueDefaultBank1A8,
    kSeedFlags1C0Offset = (int)&((SaveRecordSeedView*)0)->flags1C0,
    kSeedFlags1C4Offset = (int)&((SaveRecordSeedView*)0)->flags1C4,
    kSeedFlags1C8Offset = (int)&((SaveRecordSeedView*)0)->flags1C8,
    kSeedFlags1CCOffset = (int)&((SaveRecordSeedView*)0)->flags1CC,
    kSeedAvailability1D0Offset = (int)&((SaveRecordSeedView*)0)->availability1D0,
    kSeedFlags1D8Offset = (int)&((SaveRecordSeedView*)0)->flags1D8,
    kSeedOpaqueWords1DCOffset = (int)&((SaveRecordSeedView*)0)->opaqueWords1DC,
    kSeedScalars204Offset = (int)&((SaveRecordSeedView*)0)->scalars204,
    kSeedWord210Offset = (int)&((SaveRecordSeedView*)0)->word210,
    kSeedScalar214Offset = (int)&((SaveRecordSeedView*)0)->scalar214,
    kSeedDefaultBytes218Offset = (int)&((SaveRecordSeedView*)0)->defaultBytes218,
    kSeedUnlockedCoursesOffset = (int)&((SaveRecordSeedView*)0)->unlockedCourses
};
// Banks with unresolved IDs are safe to preserve/copy at these widths. Changing
// their policy requires tracing the consuming accessor first; zero/default-word
// constructors alone do not establish a statistic, unlock category or enum range.

// Record-to-summary map (byte offsets; sizes are unchanged):
// record 0x000 -> packed 0x000, 0x25C bytes: name, inventory, statistics, course bits
// record 0x25C: 0x70 course-record bytes omitted
// record 0x2CC -> packed 0x25C, 0x1A0 bytes: eight 0x34-byte character outfits
// record 0x46C -> packed 0x3FC, 0x40 bytes: eight caddie records
// record 0x4AC -> packed 0x43C, 0x1E bytes: fifteen club-set records
// record 0x4CA -> packed 0x45A, 0x0B bytes: eleven ball records
// record 0x4D5: three alignment bytes are not copied (packed +0x465 also untouched)
// record 0x4D8 -> packed 0x468, 0x14 bytes: settings
// record 0x4EC: 0x33C mode/progress bytes omitted.
// The full record is 0x828 bytes; the packed summary is 0x47C bytes.
struct SaveRecordSections {
    char inventory[0x25C], slotOnlyData[0x70], characterOutfits[0x1A0];
    char caddieRecords[0x40], clubSetRecords[0x1E], ballRecords[0xB], padding[3];
    char settings[0x14], progress[0x33C];
};
struct PackedProfileSections {
    char inventory[0x25C], characterOutfits[0x1A0], caddieRecords[0x40];
    char clubSetRecords[0x1E], ballRecords[0xB], padding[3], settings[0x14];
};
typedef char SaveRecordSections_size[(sizeof(SaveRecordSections) == 0x828) ? 1 : -1];
typedef char PackedProfileSections_size[(sizeof(PackedProfileSections) == 0x47C) ? 1 : -1];

// Translation unit 0x800719F4-0x80073060 (15 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "sdk_protos.h"

extern "C" {

extern float kFloat_6_2831855;
extern float s_saveDataRuntimeTau;
extern unsigned int s_saveRecordDefaultWord;
extern unsigned int s_saveRecordZeroWord;
extern unsigned int s_saveRecordDefaultTailWord;

struct PointerArray {
	void** data;
	unsigned int index;
};

struct FourBytes {
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
};

// Return the first cursor-slot pointer in the array.
void* CursorSlotArray_Begin(void* arg) {
	return *static_cast<void**>(arg);
}

// Copy a borrowed cursor-slot iterator; neither pointer owns or retains an element.
void CursorSlotIterator_Copy(void** destination, void** source) {
	*destination = *source;
}

// Return the first input-device-slot pointer in the array.
void* InputDeviceSlotArray_Begin(void* arg) {
	return *static_cast<void**>(arg);
}

// Copy an input-device-slot iterator.
void InputDeviceSlotIterator_Copy(void** destination, void** source) {
	*destination = *source;
}

// Return a borrowed writable slot at the unchecked current index; storage must remain alive.
void** PointerArray_GetCurrentSlot(PointerArray* arg) {
	return &arg->data[arg->index];
}

// Return the selected pointer slot for this duplicate iterator specialization.
void** PointerArray_GetCurrentSlot_1A30(PointerArray* arg) {
	return &arg->data[arg->index];
}

// Copy four color channels in their existing byte order.
void ColorRGBA_Copy(FourBytes* dest, const FourBytes* src) {
	dest->byte0 = src->byte0;
	dest->byte1 = src->byte1;
	dest->byte2 = src->byte2;
	dest->byte3 = src->byte3;
}

// Initialize the save-data unit's runtime full-circle constant.
void InitSaveDataRuntimeTau() {
	s_saveDataRuntimeTau = kFloat_6_2831855;
}

void ItemInventory_LoadIffArchive(void*);
extern unsigned int s_saveInventoryInitialized;
extern void* g_sharedDoc;
void SaveData_InitBlankRecord(void*, int, int);

// Reload the serialized part default for each word; volatile store order is part of the ABI.
static inline void SeedPartBankWords(volatile unsigned int* bankWords) {
    bankWords[0] = s_saveRecordDefaultWord;
    bankWords[1] = s_saveRecordDefaultWord;
    bankWords[2] = s_saveRecordDefaultWord;
    bankWords[3] = s_saveRecordDefaultWord;
    bankWords[4] = s_saveRecordDefaultWord;
    bankWords[5] = s_saveRecordDefaultWord;
    bankWords[6] = s_saveRecordDefaultWord;
    bankWords[7] = s_saveRecordDefaultWord;
    bankWords[8] = s_saveRecordDefaultWord;
    bankWords[9] = s_saveRecordDefaultWord;
    bankWords[10] = s_saveRecordDefaultWord;
    bankWords[11] = s_saveRecordDefaultWord;
    bankWords[12] = s_saveRecordDefaultWord;
    bankWords[13] = s_saveRecordDefaultWord;
    bankWords[14] = s_saveRecordDefaultWord;
    bankWords[15] = s_saveRecordDefaultWord;
    bankWords[16] = s_saveRecordDefaultWord;
    bankWords[17] = s_saveRecordDefaultWord;
    bankWords[18] = s_saveRecordDefaultWord;
    bankWords[19] = s_saveRecordDefaultWord;
    bankWords[20] = s_saveRecordDefaultWord;
    bankWords[21] = s_saveRecordDefaultWord;
    bankWords[22] = s_saveRecordDefaultWord;
    bankWords[23] = s_saveRecordDefaultWord;
    bankWords[24] = s_saveRecordDefaultWord;
}

// Reload each of the six tail defaults without caching the shared default word.
static inline void SeedTailBankWords(volatile unsigned int* bankWords) {
    bankWords[0] = s_saveRecordDefaultTailWord;
    bankWords[1] = s_saveRecordDefaultTailWord;
    bankWords[2] = s_saveRecordDefaultTailWord;
    bankWords[3] = s_saveRecordDefaultTailWord;
    bankWords[4] = s_saveRecordDefaultTailWord;
    bankWords[5] = s_saveRecordDefaultTailWord;
}

// Seed one normal slot's serialized flag banks before starter-record construction.
// Borrow record storage; preserve global default reloads and the observed write order.
static inline void SeedNormalRecordFlagWords(volatile unsigned int* recordWords, unsigned int zero, int zeroDefault) {
    SeedPartBankWords(recordWords + kSeedPrimaryPartFlagsOffset / 4);
        recordWords[kSeedReservedFlagsA4Offset / 4 + 0] = zeroDefault;
        recordWords[kSeedPrimaryCaddieItemFlagsOffset / 4] = zero;
    SeedPartBankWords(recordWords + kSeedAlternatePartFlagsOffset / 4);
        recordWords[kSeedReservedFlagsA4Offset / 4 + 1] = zeroDefault;
        recordWords[kSeedAlternateCaddieItemFlagsOffset / 4] = zero;
        recordWords[kSeedFlags140Offset / 4] = zero;
        recordWords[kSeedOwnedHairStylesOffset / 4 + 0] = zeroDefault;
    SeedPartBankWords(recordWords + kSeedOpaquePartBank144Offset / 4);
    SeedTailBankWords(recordWords + kSeedOpaqueDefaultBank1A8Offset / 4);

        recordWords[kSeedOwnedHairStylesOffset / 4 + 1] = zeroDefault;
        recordWords[kSeedFlags1C4Offset / 4] = zero;
        recordWords[kSeedFlags1C0Offset / 4] = zero;
        recordWords[kSeedFlags1CCOffset / 4] = zero;
        recordWords[kSeedFlags1C8Offset / 4] = zero;
        recordWords[kSeedFlags1D8Offset / 4] = zero;
        recordWords[kSeedAvailability1D0Offset / 4 + 0] = zeroDefault;
        recordWords[kSeedAvailability1D0Offset / 4 + 1] = zeroDefault;
        recordWords[kSeedUnlockedCoursesOffset / 4] = zero;
}

// Seed temporary-record flag banks before InitBlankRecord supplies starter inventory.
// Borrow the full save-word buffer; global default words are reloaded in store order.
static inline void SeedTemporaryRecordFlagWords(volatile unsigned int* saveWords, unsigned int zero, int zeroDefault) {
	// Phase 2: seed equivalent defaults in the temporary/shared record at +0x20A8.
    SeedPartBankWords(saveWords + (kTemporaryRecordOffset + kSeedPrimaryPartFlagsOffset) / 4);
    saveWords[(kTemporaryRecordOffset + kSeedPrimaryCaddieItemFlagsOffset) / 4] = zero;
	saveWords[(kTemporaryRecordOffset + kSeedReservedFlagsA4Offset) / 4 + 0] = s_saveRecordZeroWord,
	saveWords[(kTemporaryRecordOffset + kSeedReservedFlagsA4Offset) / 4 + 1] = s_saveRecordZeroWord;
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 0] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 1] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 2] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 3] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 4] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 5] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 6] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 7] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 8] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 9] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 10] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 11] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 12] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 13] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 14] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 15] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 16] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 17] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 18] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 19] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 20] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 21] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 22] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 23] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternatePartFlagsOffset) / 4 + 24] = s_saveRecordDefaultWord,
	saveWords[(kTemporaryRecordOffset + kSeedAlternateCaddieItemFlagsOffset) / 4] = 0,
	saveWords[(kTemporaryRecordOffset + kSeedOwnedHairStylesOffset) / 4 + 0] = s_saveRecordZeroWord,
	saveWords[(kTemporaryRecordOffset + kSeedOwnedHairStylesOffset) / 4 + 1] = s_saveRecordZeroWord,
	saveWords[(kTemporaryRecordOffset + kSeedFlags140Offset) / 4] = 0,
    SeedPartBankWords(saveWords + (kTemporaryRecordOffset + kSeedOpaquePartBank144Offset) / 4);
    SeedTailBankWords(saveWords + (kTemporaryRecordOffset + kSeedOpaqueDefaultBank1A8Offset) / 4);
	for (unsigned int off = (kTemporaryRecordOffset + kSeedFlags1C0Offset); off <= (kTemporaryRecordOffset + kSeedFlags1CCOffset); off += 4) saveWords[off / 4] = 0;
	for (unsigned int off = (kTemporaryRecordOffset + kSeedAvailability1D0Offset); off <= (kTemporaryRecordOffset + kSeedAvailability1D0Offset + 4); off += 4) saveWords[off / 4] = zeroDefault;
	saveWords[(kTemporaryRecordOffset + kSeedFlags1D8Offset) / 4] = 0; saveWords[(kTemporaryRecordOffset + kSeedUnlockedCoursesOffset) / 4] = 0;
}

// Preserve packed-summary constructor stores before its whole-block clear; these
// writes are scaffolding, not defaults that survive SaveData_Ctor.
static inline void SeedSummaryFlagWords(volatile unsigned int* saveWords, int zeroDefault) {
	// Phase 3: seed packed-summary words at +0x28D0 before its final whole-block clear.
    SeedPartBankWords(saveWords + (kPlayerSummaryOffset + kSeedPrimaryPartFlagsOffset) / 4); saveWords[(kPlayerSummaryOffset + kSeedPrimaryCaddieItemFlagsOffset) / 4] = 0;
	saveWords[(kPlayerSummaryOffset + kSeedReservedFlagsA4Offset) / 4 + 0] = s_saveRecordZeroWord,
	saveWords[(kPlayerSummaryOffset + kSeedReservedFlagsA4Offset) / 4 + 1] = s_saveRecordZeroWord;
    SeedPartBankWords(saveWords + (kPlayerSummaryOffset + kSeedAlternatePartFlagsOffset) / 4); saveWords[(kPlayerSummaryOffset + kSeedAlternateCaddieItemFlagsOffset) / 4] = 0;
	for (unsigned int off = (kPlayerSummaryOffset + kSeedOwnedHairStylesOffset); off <= (kPlayerSummaryOffset + kSeedOwnedHairStylesOffset + 4); off += 4) saveWords[off / 4] = zeroDefault;
	saveWords[(kPlayerSummaryOffset + kSeedFlags140Offset) / 4] = 0;
    SeedPartBankWords(saveWords + (kPlayerSummaryOffset + kSeedOpaquePartBank144Offset) / 4);
    SeedTailBankWords(saveWords + (kPlayerSummaryOffset + kSeedOpaqueDefaultBank1A8Offset) / 4);
	for (unsigned int off = (kPlayerSummaryOffset + kSeedFlags1C0Offset); off <= (kPlayerSummaryOffset + kSeedFlags1CCOffset); off += 4) saveWords[off / 4] = 0;
    saveWords[(kPlayerSummaryOffset + kSeedAvailability1D0Offset) / 4 + 0] = zeroDefault;
    saveWords[(kPlayerSummaryOffset + kSeedAvailability1D0Offset) / 4 + 1] = zeroDefault;
    saveWords[(kPlayerSummaryOffset + kSeedFlags1D8Offset) / 4] = 0;
    saveWords[(kPlayerSummaryOffset + kSeedUnlockedCoursesOffset) / 4] = 0;
}

// Initialize four normal records, the temporary/shared record, and the packed summary.
// self is caller-owned writable save storage through +0x2D4C; return the same address.
// Byte +5 selects the shared record; word +0 selects the current normal slot.
// Constructor seeds are grouped by typed serialized bank below. They are
// scaffolding: InitBlankRecord later supplies each slot's actual starter inventory;
// the summary seed is completely overwritten by Memory_Clear in phase 4.
// Preserve global default reloads and volatile store order, even for overwritten data.
void* SaveData_Ctor(void* self) {
    unsigned int zero = 0;
    int slotIndex;
    int zeroDefault = s_saveRecordZeroWord;
    int tailDefault = s_saveRecordDefaultTailWord;

	int wordDefault = s_saveRecordDefaultWord;

	volatile unsigned int* saveWords = reinterpret_cast<volatile unsigned int*>(self);
	// Phase 1: seed the flag/statistic words of all four normal records.
	// Each named bank is record-relative; array indices count 32-bit words.
	// Preserve volatile stores and their order, including writes overwritten later.
	for (int recordIndex = 0; recordIndex < kSaveSlotCount; ++recordIndex) {
		volatile unsigned int* recordWords = reinterpret_cast<volatile unsigned int*>(reinterpret_cast<char*>(self) + kSaveRecordsOffset + recordIndex * kSaveRecordBytes);

        SeedNormalRecordFlagWords(recordWords, zero, zeroDefault);
	}
	// Phase 2: seed the temporary record's serialized flag banks.
	SeedTemporaryRecordFlagWords(saveWords, zero, zeroDefault);
	// Phase 3: preserve the overwritten summary seed stores.
	SeedSummaryFlagWords(saveWords, zeroDefault);
	// Phase 4: clear the packed summary and ensure inventory metadata is available.
	Memory_Clear(reinterpret_cast<char*>(self) + kPlayerSummaryOffset, kPlayerSummaryBytes);
	if (!s_saveInventoryInitialized) { ItemInventory_LoadIffArchive((char*)static_cast<CSharedDoc*>(g_sharedDoc)->itemInventory); s_saveInventoryInitialized = 1; }
	// Phase 5: reset selection flags and build starter records for all slots and shared mode.
	saveWords[0] = 0;
	reinterpret_cast<volatile unsigned char*>(self)[4] = 0;
	reinterpret_cast<volatile unsigned char*>(self)[5] = 0;
	reinterpret_cast<volatile unsigned char*>(self)[6] = 0;
	for (slotIndex = 0; slotIndex < kSaveSlotCount; ++slotIndex) SaveData_InitBlankRecord(self, slotIndex, 0);
	SaveData_InitBlankRecord(self, 0, 1);
	return self;
}

// Return the first normal save record immediately after the eight-byte save-data header.
void* SaveData_GetFirstRecordStorage(void* arg) {
	return static_cast<char*>(arg) + kSaveRecordsOffset;
}

union SummaryRecordSnapshot {
    unsigned int words[kRecordWordCount];
    SaveRecordSections fields;
};
// Preserve the original temporary-record construction; every initialized byte is
// overwritten by the inventory section copy. This helper does not define starter
// inventory policy and does not initialize padding outside the copied sections.
static inline void InitializeSummarySnapshotScaffolding(SummaryRecordSnapshot& recordCopy) {
	// Phase 2: preserve constructor scaffolding (NOT summary policy defaults).
	// All bytes written here lie in inventory[0x25C] and are overwritten by the
	// first section copy in phase 3. Edit starter values in InitDefaultInventoryRecord,
	// not in this snapshot-construction sequence.
	// Volatile accesses force the same default loads/stores as the original constructor.
    SeedPartBankWords(reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->primaryPartFlags);
	reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->primaryCaddieItemFlags = 0;
	reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->reservedFlagsA4[0] = s_saveRecordZeroWord;
	reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->reservedFlagsA4[1] = s_saveRecordZeroWord;
    SeedPartBankWords(reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->alternatePartFlags);
	reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->alternateCaddieItemFlags = 0;
	reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->ownedHairStyles[0] = s_saveRecordZeroWord;
	reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->ownedHairStyles[1] = s_saveRecordZeroWord;
	reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->flags140 = 0;
    SeedPartBankWords(reinterpret_cast<volatile SaveRecordSeedView*>(recordCopy.words)->opaquePartBank144);
	unsigned int tailDefault = *reinterpret_cast<volatile unsigned int*>(&s_saveRecordDefaultTailWord);
	// Remaining default writes are ordinary accesses after the volatile tail-word load.
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->opaqueDefaultBank1A8[0] = tailDefault;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->opaqueDefaultBank1A8[1] = tailDefault;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->opaqueDefaultBank1A8[2] = tailDefault;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->opaqueDefaultBank1A8[3] = tailDefault;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->opaqueDefaultBank1A8[4] = tailDefault;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->opaqueDefaultBank1A8[5] = tailDefault;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->flags1C0 = 0;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->flags1C4 = 0;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->flags1C8 = 0;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->flags1CC = 0;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->availability1D0[0] = s_saveRecordZeroWord;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->availability1D0[1] = s_saveRecordZeroWord;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->flags1D8 = 0;
	reinterpret_cast<SaveRecordSeedView*>(recordCopy.words)->unlockedCourses = 0;
}

// Rebuild and return a borrowed packed summary inside self at +0x28D0.
// A subsequent rebuild overwrites it. Current slot must be in [0,4); no bounds checks.
// Source is shared +0x20A8 when byte +5 is set, otherwise slots[currentSlot].
void* SaveData_BuildCurrentPlayerSummary(void* self) {
	char* saveStorage = static_cast<char*>(self);
	char* sourceRecord;
	// Phase 1: choose the source record without changing current-slot/shared-mode state.
	if (static_cast<unsigned char>(saveStorage[5]) != 0) {
		sourceRecord = saveStorage + kTemporaryRecordOffset;
	} else {
		sourceRecord = saveStorage + (*reinterpret_cast<unsigned int*>(saveStorage) * kSaveRecordBytes) + kSaveRecordsOffset;
	}
	SummaryRecordSnapshot recordCopy;
	// Phase 2: preserve overwritten constructor scaffolding before copying real data.
	InitializeSummarySnapshotScaffolding(recordCopy);
	// Phase 3: copy every logical record section to the stack snapshot; padding is skipped.
	Memory_Copy(recordCopy.fields.inventory, sourceRecord, 0x25C);
	Memory_Copy(recordCopy.fields.slotOnlyData, sourceRecord + 0x25C, 0x70);
	Memory_Copy(recordCopy.fields.characterOutfits, sourceRecord + 0x2CC, 0x1A0);
	Memory_Copy(recordCopy.fields.caddieRecords, sourceRecord + 0x46C, 0x40);
	Memory_Copy(recordCopy.fields.clubSetRecords, sourceRecord + 0x4AC, 0x1E);
	Memory_Copy(recordCopy.fields.ballRecords, sourceRecord + 0x4CA, 0xB);
	Memory_Copy(recordCopy.fields.settings, sourceRecord + 0x4D8, 0x14);
	Memory_Copy(recordCopy.fields.progress, sourceRecord + 0x4EC, 0x33C);
	// Phase 4: flatten the six summary sections; slot-only/progress bytes stay out.
	if (saveStorage + kPlayerSummaryOffset != 0) {
		Memory_Copy(saveStorage + kPlayerSummaryOffset, recordCopy.fields.inventory, 0x25C);
		Memory_Copy(saveStorage + 0x2B2C, recordCopy.fields.characterOutfits, 0x1A0);
		Memory_Copy(saveStorage + 0x2CCC, recordCopy.fields.caddieRecords, 0x40);
		Memory_Copy(saveStorage + 0x2D0C, recordCopy.fields.clubSetRecords, 0x1E);
		Memory_Copy(saveStorage + 0x2D2A, recordCopy.fields.ballRecords, 0xB);
		Memory_Copy(saveStorage + 0x2D38, recordCopy.fields.settings, 0x14);
	}
	return saveStorage + kPlayerSummaryOffset;
}

// Copy the original 0x828-word block into storage after the save-data header.
// Borrow both buffers; destination must hold header + four 0x828-byte records.
// This does not copy the temporary record or packed summary and retains no pointers.
void SaveData_CopyRecordBlock(void* dest, const void* src) {
	// Preserve the observed word count: this helper copies 0x20A0 bytes, not one 0x828-byte slot.
	struct Copy { unsigned int words[0x828]; };
	*reinterpret_cast<Copy*>(static_cast<char*>(dest) + 8) = *static_cast<const Copy*>(src);
}

// Pack borrowed record sections into caller-owned 0x47C-byte storage using the map above.
// Returns 0 for a null source/destination, 1 after copying; omitted/padding bytes are untouched.
// The unused first argument is the save-data receiver. No allocations or retained pointers.
int SaveData_PackProfileSections(void*, void* dest, const void* src) {
	if (dest == 0 || src == 0) return 0;
	Memory_Copy(static_cast<PackedProfileSections*>(dest)->inventory, static_cast<const SaveRecordSections*>(src)->inventory, 0x25C);
	Memory_Copy(static_cast<PackedProfileSections*>(dest)->characterOutfits, static_cast<const SaveRecordSections*>(src)->characterOutfits, 0x1A0);
	Memory_Copy(static_cast<PackedProfileSections*>(dest)->caddieRecords, static_cast<const SaveRecordSections*>(src)->caddieRecords, 0x40);
	Memory_Copy(static_cast<PackedProfileSections*>(dest)->clubSetRecords, static_cast<const SaveRecordSections*>(src)->clubSetRecords, 0x1E);
	Memory_Copy(static_cast<PackedProfileSections*>(dest)->ballRecords, static_cast<const SaveRecordSections*>(src)->ballRecords, 0xB);
	Memory_Copy(static_cast<PackedProfileSections*>(dest)->settings, static_cast<const SaveRecordSections*>(src)->settings, 0x14);
	return 1;
}

// Restore a packed summary to caller-owned 0x828-byte record storage using the map above.
// Returns 0 for null source/destination, otherwise 1; preserves course records, padding,
// and trailing progress. The unused first argument is the save-data receiver.
int SaveData_UnpackProfileSections(void*, void* dest, const void* src) {
	if (dest == 0 || src == 0) return 0;
	Memory_Copy(static_cast<SaveRecordSections*>(dest)->inventory, static_cast<const PackedProfileSections*>(src)->inventory, 0x25C);
	Memory_Copy(static_cast<SaveRecordSections*>(dest)->characterOutfits, static_cast<const PackedProfileSections*>(src)->characterOutfits, 0x1A0);
	Memory_Copy(static_cast<SaveRecordSections*>(dest)->caddieRecords, static_cast<const PackedProfileSections*>(src)->caddieRecords, 0x40);
	Memory_Copy(static_cast<SaveRecordSections*>(dest)->clubSetRecords, static_cast<const PackedProfileSections*>(src)->clubSetRecords, 0x1E);
	Memory_Copy(static_cast<SaveRecordSections*>(dest)->ballRecords, static_cast<const PackedProfileSections*>(src)->ballRecords, 0xB);
	Memory_Copy(static_cast<SaveRecordSections*>(dest)->settings, static_cast<const PackedProfileSections*>(src)->settings, 0x14);
	return 1;
}

extern void* memset(void*, int, unsigned int);
void* SharedDoc_GetItemInventory(); void* ItemInventory_GetPartDataAt(void*, int); void Bitset782_AssignBit(void*, int, int); void Bitset782_ResetBit(void*, int);
// The six address-named helpers below clear one flag word each; their separate calls are intentional.
void SaveFlags_ClearWord_774E8(void*); void Bitset32_AssignBit(void*, int, int); void SaveFlags_Init51BitsSet(void*); void SaveFlags_InitPartBank(void*);
void SaveFlags_ClearWord_776F8(void*); void SaveFlags_InitSixDefaultWords(void*); void SaveFlags_ClearWord_77738(void*); void SaveFlags_ClearWord_77744(void*);
void SaveFlags_InitTwoDefaultWords(void*); unsigned long long SaveRecord_PreserveWordPair(void*, int); void Bitset45Reference_Assign(unsigned long long*, int); void SaveFlags_ClearWord_778B0(void*);
void SaveFlags_ClearCourseAvailabilityWord(void*); void Bitset14_AssignBit(void*, int, int); extern const float kFloat_0_SaveRecordDefaults;

// Grant the fixed starter PART TABLE indices (not packed IFF type IDs) to one bank.
// Item display names are not established here; resolve them through part metadata.
static inline void GrantStarterPartIndices(void* partFlags) {
  Bitset782_AssignBit(partFlags, 0x24, 1);
  Bitset782_AssignBit(partFlags, 0x31, 1);
  Bitset782_AssignBit(partFlags, 0x44, 1);
  Bitset782_AssignBit(partFlags, 0x66, 1);
  Bitset782_AssignBit(partFlags, 0x77, 1);
  Bitset782_AssignBit(partFlags, 0xA3, 1);
  Bitset782_AssignBit(partFlags, 0xC3, 1);
  Bitset782_AssignBit(partFlags, 0xE1, 1);
  Bitset782_AssignBit(partFlags, 0xEE, 1);
  Bitset782_AssignBit(partFlags, 0x125, 1);
  Bitset782_AssignBit(partFlags, 0x137, 1);
  Bitset782_AssignBit(partFlags, 0x15A, 1);
  Bitset782_AssignBit(partFlags, 0x16E, 1);
  Bitset782_AssignBit(partFlags, 0x197, 1);
  Bitset782_AssignBit(partFlags, 0x1B3, 1);
  Bitset782_AssignBit(partFlags, 0x1C6, 1);
  Bitset782_AssignBit(partFlags, 0x1F5, 1);
  Bitset782_AssignBit(partFlags, 0x203, 1);
  Bitset782_AssignBit(partFlags, 0x23C, 1);
  Bitset782_AssignBit(partFlags, 0x220, 1);
  Bitset782_AssignBit(partFlags, 0x250, 1);
  Bitset782_AssignBit(partFlags, 0x275, 1);
  Bitset782_AssignBit(partFlags, 0x28D, 1);
  Bitset782_AssignBit(partFlags, 0x29E, 1);
  Bitset782_AssignBit(partFlags, 0x2AA, 1);
  Bitset782_AssignBit(partFlags, 0x2BC, 1);
  Bitset782_AssignBit(partFlags, 0x2EE, 1);
  Bitset782_AssignBit(partFlags, 0x304, 1);
}

// Set starter inventory/statistic fields in a borrowed writable 0x828-byte record.
// Caller must already have loaded inventory metadata; no pointers are retained.
// This is not a whole-record clear: name tail, course records, outfits, settings
// and mode/progress data are initialized by SaveData_InitBlankRecord instead.
void SaveData_InitDefaultInventoryRecord(void* record) {
  char* recordBytes = static_cast<char*>(record);
  // Phase 1: mark the UTF-16 name empty and initialize counters/equipped indices.
  *reinterpret_cast<unsigned short*>(recordBytes) = 0;
  reinterpret_cast<SaveInventoryPrefix*>(recordBytes)->playTicks = 0;
  reinterpret_cast<SaveInventoryPrefix*>(recordBytes)->unk_24 = 0;
  reinterpret_cast<SaveInventoryPrefix*>(recordBytes)->pang = kInitialPang;
  AsSavePlayerRecord(recordBytes)->characterIndex = 0;
  AsSavePlayerRecord(recordBytes)->clubSetIndex = 0;
  AsSavePlayerRecord(recordBytes)->ballIndex = 0;
  AsSavePlayerRecord(recordBytes)->caddieIndex = 0;
  reinterpret_cast<SaveInventoryPrefix*>(recordBytes)->caddieItemIndex = static_cast<char>(kNoEquipmentIndex);
  memset(reinterpret_cast<SaveInventoryPrefix*>(recordBytes)->equippedItemIndices, kNoEquipmentIndex, kCharacterCount);
  // Phase 2: seed the primary part bank from the three shop-flag bits 5..7.
  // Their individual meanings are unresolved; any nonzero combination enables the bit.
  for (int partIndex = 0; partIndex < kInventoryPartCount; ++partIndex) {
    void* part = ItemInventory_GetPartDataAt(SharedDoc_GetItemInventory(), partIndex);
    if (((static_cast<IFF_STRUCT::sPart*>(part)->common.shopFlags >> kDefaultPartFlagShift) & kDefaultPartFlagMask) != 0) {
      Bitset782_AssignBit(recordBytes + kPrimaryPartFlagsOffset, partIndex, 1);
    } else {
      Bitset782_ResetBit(recordBytes + kPrimaryPartFlagsOffset, partIndex);
    }
  }
  // Phase 3: initialize the paired flag banks and hairstyle bits.
  // Bits 0,4,...,28 are caddie-item inventory indices, not character IDs.
  SaveFlags_ClearWord_774E8(recordBytes + kPrimaryCaddieItemFlagsOffset);
  SaveFlags_ClearWord_774E8(recordBytes + kAlternateCaddieItemFlagsOffset);
  Bitset32_AssignBit(recordBytes + kPrimaryCaddieItemFlagsOffset, 0, 1);
  Bitset32_AssignBit(recordBytes + kPrimaryCaddieItemFlagsOffset, 4, 1);
  Bitset32_AssignBit(recordBytes + kPrimaryCaddieItemFlagsOffset, 8, 1);
  Bitset32_AssignBit(recordBytes + kPrimaryCaddieItemFlagsOffset, 0xC, 1);
  Bitset32_AssignBit(recordBytes + kPrimaryCaddieItemFlagsOffset, 0x10, 1);
  Bitset32_AssignBit(recordBytes + kPrimaryCaddieItemFlagsOffset, 0x14, 1);
  Bitset32_AssignBit(recordBytes + kPrimaryCaddieItemFlagsOffset, 0x18, 1);
  Bitset32_AssignBit(recordBytes + kPrimaryCaddieItemFlagsOffset, 0x1C, 1);
  Bitset32_AssignBit(recordBytes + kAlternateCaddieItemFlagsOffset, 0, 1);
  Bitset32_AssignBit(recordBytes + kAlternateCaddieItemFlagsOffset, 4, 1);
  Bitset32_AssignBit(recordBytes + kAlternateCaddieItemFlagsOffset, 8, 1);
  Bitset32_AssignBit(recordBytes + kAlternateCaddieItemFlagsOffset, 0xC, 1);
  Bitset32_AssignBit(recordBytes + kAlternateCaddieItemFlagsOffset, 0x10, 1);
  Bitset32_AssignBit(recordBytes + kAlternateCaddieItemFlagsOffset, 0x14, 1);
  Bitset32_AssignBit(recordBytes + kAlternateCaddieItemFlagsOffset, 0x18, 1);
  Bitset32_AssignBit(recordBytes + kAlternateCaddieItemFlagsOffset, 0x1C, 1);
  SaveFlags_Init51BitsSet(recordBytes + kReservedFlagWordsOffset);
  SaveFlags_InitPartBank(recordBytes + kAlternatePartFlagsOffset);
  // Preserve this second clear: the alternate caddie-item bank ends empty.
  SaveFlags_ClearWord_774E8(recordBytes + kAlternateCaddieItemFlagsOffset);
  SaveFlags_Init51BitsSet(recordBytes + kOwnedHairStylesOffset);
  // Phase 4: force the same 28 starter-part inventory indices in both part banks.
  // These are table indices, not packed IFF type IDs; both lists must stay in sync.
  GrantStarterPartIndices(recordBytes + kPrimaryPartFlagsOffset);
  GrantStarterPartIndices(recordBytes + kAlternatePartFlagsOffset);
  // Phase 5: reset item quantities (+0x11C, 13 halfwords), seven auxiliary flags
  // (+0x136), and statistic/availability banks through +0x214. Unnamed offsets
  // describe serialized layout; their game-specific interpretation is unresolved.
  memset(recordBytes + kSeedItemQuantitiesOffset, 0, 0x1A);
  memset(recordBytes + kSeedAuxiliaryFlags136Offset, 0, 7);
  SaveFlags_ClearWord_776F8(recordBytes + kSeedFlags140Offset);
  SaveFlags_InitPartBank(recordBytes + kSeedOpaquePartBank144Offset);
  SaveFlags_InitSixDefaultWords(recordBytes + kSeedOpaqueDefaultBank1A8Offset);
  SaveFlags_ClearWord_77738(recordBytes + kSeedFlags1C0Offset);
  SaveFlags_ClearWord_77744(recordBytes + kSeedFlags1C4Offset);
  SaveFlags_ClearWord_776F8(recordBytes + kSeedFlags1C8Offset);
  SaveFlags_ClearWord_774E8(recordBytes + kSeedFlags1CCOffset);
  SaveFlags_InitTwoDefaultWords(recordBytes + kSeedAvailability1D0Offset);
  // Set bit indices 0,1,2,27..35,37..39 of the 45-bit field at +0x1D0.
  // Each r3:r4 pair carries the bit reference consumed by Assign; do not merge calls.
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 1);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 2);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x1B);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x1C);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x1D);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x1E);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x1F);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x20);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x21);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x22);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x23);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x25);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x26);
    Bitset45Reference_Assign(&pair, 1);
  }
  {
    unsigned long long pair = SaveRecord_PreserveWordPair(recordBytes + kSeedAvailability1D0Offset, 0x27);
    Bitset45Reference_Assign(&pair, 1);
  }
  SaveFlags_ClearWord_778B0(recordBytes + kSeedFlags1D8Offset);
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 0 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 1 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 2 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 3 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 4 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 5 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 6 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 7 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 8 * 4) = 0;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedOpaqueWords1DCOffset + 9 * 4) = 0;
  *reinterpret_cast<float*>(recordBytes + kSeedScalars204Offset + 0 * 4) = kFloat_0_SaveRecordDefaults;
  *reinterpret_cast<float*>(recordBytes + kSeedScalars204Offset + 1 * 4) = kFloat_0_SaveRecordDefaults;
  *reinterpret_cast<float*>(recordBytes + kSeedScalars204Offset + 2 * 4) = kFloat_0_SaveRecordDefaults;
  *reinterpret_cast<unsigned int*>(recordBytes + kSeedWord210Offset) = 0;
  *reinterpret_cast<float*>(recordBytes + kSeedScalar214Offset) = kFloat_0_SaveRecordDefaults;
  // The 64-byte table starts at byte value 8; its downstream meaning is unresolved here.
  memset(recordBytes + kSeedDefaultBytes218Offset, kDefaultByteTableEntry, sizeof(((SaveRecordSeedView*)0)->defaultBytes218));
  // Phase 6: course bits use inventory indices; only indices 0, 2 and 5 start set.
  SaveFlags_ClearCourseAvailabilityWord(recordBytes + kUnlockedCoursesOffset);
  Bitset14_AssignBit(recordBytes + kUnlockedCoursesOffset, 0, 1);
  Bitset14_AssignBit(recordBytes + kUnlockedCoursesOffset, 2, 1);
  Bitset14_AssignBit(recordBytes + kUnlockedCoursesOffset, 5, 1);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462004", _ctors_u800719F4_0, InitSaveDataRuntimeTau);
#pragma pop
