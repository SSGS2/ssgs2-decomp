// bitset functions (SaveRewards_SetPartFlags, SaveRewards_SetCaddieItemFlags). Equivalent to building this unit with -ipa file -inline noauto.
#pragma ipa file
#pragma auto_inline off
#include "iff/IffStruct.h"

#include "game/stdexcept_local.h"
#include "game/singletons.h"
#include "classes/CPangyaCommonDialog.h"
#include "classes/CSharedDoc.h"
#include "classes/ItemRequestState80300EC8.h"
#define __throw SDKHeaderThrow
#define strlen SDKHeaderStrlen
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
#undef strlen
#undef __throw

extern "C" {
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
}
// Allocate through the game's scalar allocator.
inline void* operator new(size_t size) {
    return __nw__FUl(size);
}
void operator delete(void*) throw();

enum IffRewardKind {
    kRewardCharacter = 1,
    kRewardPart = 2,
    kRewardClub = 3,
    kRewardClubSet = 4,
    kRewardBall = 5,
    kRewardConsumable = 6,
    kRewardCaddie = 7,
    kRewardCaddieItem = 8,
    kRewardSetItem = 9,
    kRewardCourse = 10,
    kRewardUnused = 0
};
enum RewardLimits {
    kCharacterCount = 8,
    kCaddieCount = 8,
    kClubSetCount = 15,
    kBallCount = 11,
    kCourseCount = 14,
    kPartCount = 782,
    kCaddieItemBits = 32,
    kSetItemBits = 161,
    kRewardTextBits = 3,
    kOtherNotificationBits = 45,
    kRequestMaxCapacity = 0x15555555,
    kRequestSmallGrowthLimit = 0x071C71C7,
    kRequestMediumGrowthLimit = 0x0E38E38E
};
enum RewardStringOffsets {
    kRewardPopupSound = 0x4B,
    kRangeErrorType = 0x5C
};

// This module copies only the first 0x25C bytes of a full SavePlayerRecord.
// Reward policy here is defined by saved bits, without assuming shop semantics:
//   secondaryFlag == 0: test primary; if absent set primary + its update bit.
//   secondaryFlag != 0: test secondary; if absent set BOTH + the update bit.
// An already-set requested bit returns before writing anything, including the
// update bit. Thus a primary-only grant never upgrades to the secondary state.
// QueueRequestIfNeeded uses that SAME requested-bit test to suppress duplicate
// popups (item_request_state.cpp). Choose 0 to require only primary, nonzero to require the
// stronger two-bit state. Which game design reward deserves that stronger state
// is not established by these callers; do not label it purchased/owned/unlocked.
// Character/caddie/club/ball byte masks: primary 0x80, secondary 0x40.
// Part/caddie-item bitsets instead select separate primary/secondary word arrays.
// Indices passed here are inventory indices, never raw IFF type IDs.
struct CharacterRewardRecord {
    unsigned short words[26];
};
struct CaddieRewardRecord {
    unsigned int words[2];
};
struct ClubSetRewardRecord {
    unsigned char flags, mode;
};
struct RewardSaveRecordView {
    char prefix[0x2CC];
    CharacterRewardRecord characters[kCharacterCount]; // 0x2CC, stride 0x34
    CaddieRewardRecord caddies[kCaddieCount];          // 0x46C, stride 8
    ClubSetRewardRecord clubSets[kClubSetCount];       // 0x4AC, stride 2
    unsigned char ballFlags[kBallCount];               // 0x4CA
};
#define AsRewardSaveRecord(p) ((RewardSaveRecordView*)(p))
typedef char RewardSave_charactersOffset[((int)&((RewardSaveRecordView*)0)->characters == 0x2CC) ? 1 : -1];
typedef char RewardSave_caddiesOffset[((int)&((RewardSaveRecordView*)0)->caddies == 0x46C) ? 1 : -1];
typedef char RewardSave_clubSetsOffset[((int)&((RewardSaveRecordView*)0)->clubSets == 0x4AC) ? 1 : -1];
typedef char RewardSave_ballFlagsOffset[((int)&((RewardSaveRecordView*)0)->ballFlags == 0x4CA) ? 1 : -1];

extern "C" {

extern signed char s_saveDataInitialized;
extern CSharedDoc* g_sharedDoc;
extern char s_saveData[];

void SaveData_Ctor(void*);
void appFreeUni__FPv(void*);
void* appMallocDummy__FUl(unsigned int);
void __dl__FPv(void*);
int ItemInventory_GetCourseIdAt(void*, int);
int ItemInventory_GetCharacterIdAt(void*, int);
int ItemInventory_GetCaddieIdAt(void*, int);
int ItemInventory_GetCaddieItemIdAt(void*, int);
int ItemInventory_GetSetItemIdAt(void*, int);
void* ItemInventory_FindSetItemData(void*, int);
int ItemInventory_FindPartIndex(void*, int);
int ItemInventory_GetPartIdAt(void*, int);
int ItemInventory_GetClubSetIdAt(void*, int);
int ItemInventory_GetBallIdAt(void*, int);
void ItemRequestState_QueueRequestIfNeeded(void*, void*, int, unsigned int, int);
void* SaveData_GetInstance();
int SaveData_TestAllSetItemPartFlags(void*, void*, int);
void SaveRewards_SetSetMemberPartFlags(void*, int, int);
void* SaveData_GetCurrentRecord(void*);
void SaveRewards_CopyPrefix(void*, void*);
void* Bitset161Reference_Assign(void*, int);
void* SetItemBitReference_GetStorage(void*, void*);
void SaveData_SetInventoryAndStatisticsPrefix(void*, void*);
void SaveData_SetClubSetRecordAt(void*, void*, unsigned int);
void SaveData_SetBallRecordAt(void*, void*, unsigned int);
extern char bitset45AssignRangeErrorText[];
extern char tourOpenFactorStrings[];
// Separate compiler literals for the set-item, character/caddie, club, ball and notification bitsets.
// Each contains "index out of range of bitset::set"; keep the individual storage symbols.
extern char bitsetSetItemSetRangeErrorText[];
extern const float kFloat_0_5_RewardPopupSound;
extern const float kFloat_200_RewardPopupSound;
extern const float kFloat_1_RewardPopupSound;
void SaveRewards_SetPartFlags(void*, unsigned short, short);
void SaveRewards_SetCostumeSetFlags(void*, int, int);
void SaveRewards_SetClubSetFlags(void*, unsigned char, short);
void SaveRewards_SetBallFlags(void*, unsigned int, unsigned int);
void SaveRewards_SetCharacterFlags(void*, unsigned int, short);
void SaveRewards_SetCaddieFlags(void*, unsigned char, short);
void SaveRewards_SetCaddieItemFlags(void*, unsigned char, short);
void SaveRewards_UnlockCourse(void*, unsigned char);
void __ct__14CPopItemDialogFPvPv(void*, unsigned int, unsigned int);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
void* __nw__FUl(unsigned int);
// 12-byte request: popup context word, full IFF type ID (kind in high six bits),
// then 16-bit inventory index and requested reward policy. Values are copied;
// this unit does not retain or release the popup context word.
struct QueuedItemReward {
    unsigned int popupContext, typeId;
    unsigned short inventoryIndex, secondaryFlag;
};
unsigned int ItemRequestVector_GetSize(void*);
QueuedItemReward* Array_GetElement(void*, int);
void WString_ReserveCapacity(void*, unsigned int);
void WString_ReplaceRange(void*, int, unsigned int, unsigned int, unsigned int, unsigned char*);
void* __ct__19CPangyaCommonDialogFPvi(void*, int, int);
void* __nw__FUl(unsigned int);
extern const float kRewardDialogZeroUpdateStep;

struct RewardTextEntry;
struct RewardTextStorage {
    unsigned char header[0x14];
    RewardTextEntry* data;
    unsigned int size;
};

void SaveData_SetCharacterEquipmentRecord(void*, const void*, unsigned int);

void SaveData_SetCaddieRecordAt(void*, const void*, unsigned char);

// Wii MSB-first signed one-bit fields read as -1 when set; preserve their signedness.
struct RewardStatusPair {
    CW_BITS3(signed char primary : 1, signed char secondary : 1, unsigned char rest : 6) unsigned char mode;
    unsigned short h;
    unsigned int w;
};
struct SavePrefixFloat {
    float v;
    // Preserve uninitialized storage until a saved prefix is copied in.
    SavePrefixFloat() {}
    // Copy this saved float with the original scalar assignment order.
    SavePrefixFloat& operator=(const SavePrefixFloat& o) {
        v = o.v;
        return *this;
    }
    // Copy-construct this saved float without changing its precision.
    SavePrefixFloat(const SavePrefixFloat& o) : v(o.v) {}
};
// Keep the split members and nontrivial float wrapper: copy order affects matching code.
struct SaveRewardPrefix {
    // +00: h[0] is the slot-used name code unit (SaveData_IsSlotUsed); the rest is copied opaquely here.
    unsigned short h[16];
    // +20..+28: copied save words; consumer meanings have not been established.
    unsigned int w20, w24, w28;
    // +2C..+2F: selected character/club-set/ball/caddie inventory indices.
    // SavePlayerRecord exposes these as signed bytes (-1 can mean no caddie); keep this copy view unsigned.
    // +30 is unresolved and is not another established inventory selector.
    unsigned char characterIndex, clubSetIndex, ballIndex, caddieIndex, b30;
    unsigned char b31[8];
    unsigned int partPrimaryFlags[25];
    unsigned int caddieItemPrimaryFlags;
    unsigned int wA4[2];
    unsigned int partSecondaryFlags[25];
    unsigned int caddieItemSecondaryFlags;
    // +114: owned hair-style bits (CShopProductHair); retain the split copy members.
    unsigned int ownedHairStyleFlags[2];
    unsigned short h11c[12];
    unsigned short h134;
    unsigned char b136, b137, b138, b139, b13a, b13b, b13c;
    unsigned int characterUpdateFlags;
    unsigned int partUpdateFlags[25];
    unsigned int setItemUpdateFlags[6];
    unsigned int clubSetUpdateFlags, ballUpdateFlags, caddieUpdateFlags, caddieItemUpdateFlags;
    unsigned int otherNotificationFlags[2];
    unsigned int textNotificationFlags, w1dc, w1e0, w1e4, w1e8, w1ec, w1f0, w1f4, w1f8, w1fc, w200;
    float f204, f208, f20c;
    unsigned int w210;
    SavePrefixFloat f214;
    unsigned char b218[64];
    unsigned int unlockedCourses;
};
struct SaveBitReference {
    unsigned int* word;
    unsigned int bit;
};
// Build a word-array/bit-index reference without testing or changing the bit.
inline SaveBitReference MakeSaveBitReference(unsigned int* word, unsigned int bit) {
    SaveBitReference result;
    result.word = word;
    result.bit = bit;
    return result;
}

extern char bitsetCharacterCaddieSetRangeErrorText[];
extern char bitsetTestRangeErrorText[];
extern char bitset14SetRangeErrorText[];
extern char bitsetTestRangeError[];
// Return caller-supplied placement storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}
}

extern "C" {
// Set a character reward flag, optionally its secondary flag, and mark the character update bit.
void SaveRewards_SetCharacterFlags(void* self, unsigned int index, short secondaryFlag);

struct ClubSetRewardStatus {
    CW_BITS3(signed char primary : 1, signed char secondary : 1, unsigned char rest : 6) unsigned char mode;
};
struct ClubSetRewardWrite {
    ClubSetRewardStatus pair;
    unsigned short pad;
};
extern char bitsetClubSetRewardSetRangeErrorText[];
void SaveData_SetClubSetRecordAt(void*, void*, unsigned int);
// Set club-set reward flags and its update bit, skipping an already-set requested flag.
void SaveRewards_SetClubSetFlags(void* self, unsigned char index, short secondaryFlag);

// Set caddie reward flags and its update bit, skipping an already-set requested flag.
void SaveRewards_SetCaddieFlags(void* self, unsigned char index, short secondaryFlag);

// Set ball reward flags and its update bit, skipping an already-set requested flag.
void SaveRewards_SetBallFlags(void* self, unsigned int index, unsigned int secondaryFlag);

extern char bitsetTestOutOfRangeMessage[];

// Check a set-member part flag, throwing the original range error if the index exceeds 781.
inline int TestSetPartBit(const SaveBitReference& r) {
    unsigned int bit = r.bit;
    unsigned int* word = r.word;
    if (bit >= kPartCount) {
        throw std::out_of_range(bitsetTestOutOfRangeMessage);
    }
    return (word[bit >> 5] & (1u << (bit & 31))) != 0;
}

// Return true only if every part in an IFF set has the requested primary/secondary flag.
int SaveData_TestAllSetItemPartFlags(void* requestState, void* arg2, int secondaryFlag);

// Check an individual part flag with the original throwing range check.
inline int TestPartBit(const SaveBitReference& r) {
    unsigned int* word = r.word;
    unsigned int bit = r.bit;
    if (bit >= kPartCount) {
        throw std::out_of_range(bitsetTestOutOfRangeMessage);
    }
    return (word[bit >> 5] & (1u << (bit & 31))) != 0;
}

// Check a part flag using the update path range-error construction.
inline int TestPartBitForUpdate(const SaveBitReference& r) {
    unsigned int* word = r.word;
    unsigned int bit = r.bit;
    if (bit >= kPartCount) {
        std::out_of_range error(bitsetTestOutOfRangeMessage, tourOpenFactorStrings + kRangeErrorType);
    }
    return (word[bit >> 5] & (1u << (bit & 31))) != 0;
}

extern char bitsetSetOutOfRangeMessage[];

// Set one checked part flag in the copied save prefix.
inline void SetPartBit(const SaveBitReference& r) {
    if (r.bit >= kPartCount) {
        std::out_of_range error(bitsetSetOutOfRangeMessage, tourOpenFactorStrings + kRangeErrorType);
    }
    r.word[r.bit >> 5] |= 1u << (r.bit & 31);
}

// Set a part update bit and return the current save singleton pointer word.
inline int SetPartNotificationBit(const SaveBitReference& r) {
    if (r.bit >= kPartCount) {
        std::out_of_range error(bitsetSetOutOfRangeMessage, tourOpenFactorStrings + kRangeErrorType);
    }
    unsigned int* w = r.word;
    unsigned int off = r.bit >> 5;
    unsigned int b = r.bit & 31;
    unsigned int mask = 1u << b;
    unsigned int old = w[off];
    unsigned int nw = old | mask;
    w[off] = nw;
    return ((int&)GetSaveDataSlot());
}

extern char s_shopPurchaseBitRangeErrorText[];

// Check a bit in the single-word caddie-item flags.
inline int TestCaddieItemBit(const SaveBitReference& r) {
    unsigned int* word = r.word;
    unsigned int bit = r.bit;
    if (bit >= kCaddieItemBits) {
        std::out_of_range error(bitsetTestRangeError, tourOpenFactorStrings + kRangeErrorType);
    }
    return (word[0] & (1u << bit)) != 0;
}

// Set a checked caddie-item flag.
inline void SetCaddieItemBit(const SaveBitReference& r) {
    if (r.bit >= kCaddieItemBits) {
        std::out_of_range error(s_shopPurchaseBitRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    r.word[0] |= 1u << r.bit;
}

// Set a caddie-item update bit and return the current save singleton pointer word.
inline int SetCaddieItemNotificationBit(const SaveBitReference& r) {
    if (r.bit >= kCaddieItemBits) {
        std::out_of_range error(s_shopPurchaseBitRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    r.word[0] |= 1u << r.bit;
    return ((int&)GetSaveDataSlot());
}

// Select the primary or secondary caddie-item flags before testing the bit.
inline int HasCaddieItemFlag(const SaveRewardPrefix& first, unsigned int index, int flag) {
    return TestCaddieItemBit(flag ? MakeSaveBitReference((unsigned int*)&first.caddieItemSecondaryFlags, index)
                                  : MakeSaveBitReference((unsigned int*)&first.caddieItemPrimaryFlags, index));
}

// Set primary/secondary caddie-item flags and the corresponding update bit.
void SaveRewards_SetCaddieItemFlags(void* requestState, unsigned char index, short secondaryFlag);

// Select the primary or secondary part flags before testing the bit.
inline int HasPartFlag(SaveRewardPrefix& first, unsigned int index, int flag) {
    return TestPartBitForUpdate(flag ? MakeSaveBitReference(&first.partSecondaryFlags[0], index)
                                     : MakeSaveBitReference(&first.partPrimaryFlags[0], index));
}

// Set primary/secondary outfit-part flags and the corresponding update bit.
void SaveRewards_SetPartFlags(void* self, unsigned short index, short secondaryFlag);

// Check a set-member part flag before applying that member reward.
inline int TestSetPartBitForUpdate(const SaveBitReference& r) {
    unsigned int* word = r.word;
    unsigned int bit = r.bit;
    if (bit >= kPartCount) {
        throw std::out_of_range(bitsetTestOutOfRangeMessage);
    }
    return (word[bit >> 5] & (1u << (bit & 31))) != 0;
}

// Set a part flag while granting a costume set.
inline void SetSetPartBit(const SaveBitReference& r) {
    if (r.bit >= kPartCount) {
        throw std::out_of_range(bitsetSetOutOfRangeMessage);
    }
    r.word[r.bit >> 5] |= 1u << (r.bit & 31);
}

// Mark a changed set-member part and return the current save singleton pointer word.
inline int SetSetPartNotificationBit(const SaveBitReference& r) {
    if (r.bit >= kPartCount) {
        throw std::out_of_range(bitsetSetOutOfRangeMessage);
    }
    r.word[r.bit >> 5] |= 1u << (r.bit & 31);
    return ((int&)GetSaveDataSlot());
}

// Apply part reward flags to each missing member of an IFF costume set.
void SaveRewards_SetSetMemberPartFlags(void* requestState, int index, int secondaryFlag);

// Test an outfit part inventory index against the requested primary/secondary bitset.
int SaveRewards_TestPartFlag(void* requestState, unsigned int index, int secondaryFlag);

// Check a primary/secondary caddie-item bit with the original range check.
inline int TestCaddieItemFlagBit(const SaveBitReference& r) {
    unsigned int* word = r.word;
    unsigned int bit = r.bit;
    if (bit >= kCaddieItemBits) {
        std::out_of_range error(bitsetTestRangeError, tourOpenFactorStrings + kRangeErrorType);
    }
    return (*word & (1u << bit)) != 0;
}

// Test a caddie-item inventory index against the requested primary/secondary bitset.
int SaveRewards_TestCaddieItemFlag(void* requestState, unsigned int index, int secondaryFlag);

// Set one of the 45 additional notification flags in the save prefix.
void SaveRewards_SetOtherNotificationBit(void* self, unsigned int index);
// 20-byte text request: kind, 12-byte MSL wide string, notification index.
// flags high bit selects owned heap data; otherwise flags/length/data hold
// inline bytes. length counts 16-bit code units on the heap path. kind 0 marks
// one of the three saved text notifications; other kinds are not marked here.
struct RewardTextEntry {
    unsigned int kind, flags, length, data, notificationIndex;
};
struct RewardDialogInterface {
    virtual ~RewardDialogInterface();
    virtual void unused();
    virtual int update(float);
};
union StringAllocatorByte {
    unsigned int alignment;
    unsigned char value;
};
// Owning copy of a queued reward text; retain the type name used by EH relocations.
struct A54String {
    unsigned int kind, flags, length, data, notificationIndex;
    // Release the temporary reward text's heap buffer if selected by the tag.
    ~A54String() {
        if (flags >> 31)
            appFreeUni__FPv((void*)data);
    }
};
struct RewardDialogAllocation {
    char bytes[0x44];
    // Construct the common dialog in its allocated storage.
    RewardDialogAllocation(int a, int b) {
        __ct__19CPangyaCommonDialogFPvi(this, a, b);
    }
};

// Show queued text dialogs in order; return complete once no queued or active dialog remains.
int ItemRequestDialogs_ProcessQueue(void* self);

extern char bitsetTestRangeErrorText_804B8178[];
extern char bitsetNotificationSetRangeErrorText[];

// Build the reference used by the text-notification path.
inline SaveBitReference MakeNotificationBitReference(unsigned int* word, unsigned int bit) {
    SaveBitReference result;
    result.word = word;
    result.bit = bit;
    return result;
}

// Test one of the three text-notification flags.
inline int TestTextNotificationBit(const SaveBitReference& r) {
    unsigned int bit = r.bit;
    unsigned int* word = r.word;
    if (bit >= kRewardTextBits) {
        std::out_of_range error(bitsetTestRangeErrorText_804B8178, tourOpenFactorStrings + kRangeErrorType);
    }
    return (*word & (1u << bit)) != 0;
}

// Mark notification bits for queued kind-zero text rewards without repeating prior notifications.
void ItemRequestState_MarkTextNotifications(RewardTextStorage* self);

// Apply queued rewards by the six-bit IFF item kind; entries carry inventory indices, not type IDs.
// This pass borrows the vector and does NOT erase requests or show popups.
// ProcessPopupQueue consumes entries later, so applying must precede popup
// draining. Grant setters skip an already-set requested bit, making repeated
// application idempotent for that saved state. Club/consumable/unused kinds do
// nothing here. There is no all-or-nothing transaction across queued rewards.
void ItemRequestState_ApplyPendingRewards(void* self);

#pragma push
#pragma use_lmw_stmw on
// Read a character primary/secondary reward flag from its saved equipment record.
// index must name a valid character slot. Returns 0 or -1 (signed one-bit value),
// NOT necessarily 1. Copying all 26 halfwords preserves the original record copy;
// only its first BYTE is queried: bit 7 primary, bit 6 secondary. The shifts move
// the selected bit into the sign position before arithmetic sign extension.
int SaveRewards_TestCharacterFlag(void* requestState, int index, int secondaryFlag);
#pragma pop

struct ItemPopupAllocation {
    char data[0x30];
    // Construct an item popup from its type/context words in allocated storage.
    ItemPopupAllocation(unsigned int a, unsigned int b) {
        __ct__14CPopItemDialogFPvPv(this, a, b);
    }
};
struct ItemPopupRequest {
    unsigned int a, b, c;
};
inline int ItemPopupDistance(ItemPopupRequest* first, ItemPopupRequest* last);
struct ItemPopupInterface {
    virtual ~ItemPopupInterface();
    virtual int update();
};
extern void* g_soundManager;

// Show item popups in order, play the reward sound and retire completed popups.
// The front request is removed when its popup is CREATED, before completion.
// Save changes must already have been applied by ApplyPendingRewards. Return 1
// only on a call that finds no pending request and no active popup at entry;
// destroying the last active popup here still returns 0 until the next call.
int ItemRequestState_ProcessPopupQueue(void* self);

// Compute the number of 12-byte requests between two vector positions.
inline int ItemPopupDistance(ItemPopupRequest* first, ItemPopupRequest* last) {
    return ((int)last - (int)first) / 12;
}

// Read a ball primary/secondary reward flag from its saved flag byte.
// As with character queries, the selected saved bit is sign-extended to 0 or -1.
int SaveRewards_TestBallFlag(void* requestState, int index, int secondaryFlag);

// Test whether a course inventory index is unlocked.
bool SaveRewards_IsCourseUnlocked(void* arg1, unsigned int index);

// Read a club-set primary/secondary reward flag, retaining both original byte loads.
int SaveRewards_TestClubSetFlag(void* requestState, int index, int secondaryFlag);

// Grant every missing part in a set and mark the set-item update bit.
void SaveRewards_SetCostumeSetFlags(void* requestState, int index, int secondaryFlag);

// Lazily construct and return the save-data singleton.
void* SaveData_GetInstance();

// Release an owned reward-text buffer and optionally free its entry.
void* dtor_803019E8(void* self, int freeFlag);

// Clear a vector count, release its buffer and optionally free the vector.
void* dtor_8030B914(void* self, int freeFlag);

// Clear and release a word-vector buffer, optionally freeing the vector.
void* dtor_8030B984(void* self, int freeFlag);

// Translate a valid inventory index to an IFF type ID and execute or queue its reward request.
// Resolve a valid character inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestCharacterByInventoryIndex(void* requestState, void* popupContext, int index,
                                                       int secondaryFlag);

// Translate a valid inventory index to an IFF type ID and execute or queue its reward request.
// Resolve a valid caddie inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestCaddieByInventoryIndex(void* requestState, void* popupContext, int index,
                                                    int secondaryFlag);

// Translate a valid inventory index to an IFF type ID and execute or queue its reward request.
// Resolve a valid caddieitem inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestCaddieItemByInventoryIndex(void* requestState, void* popupContext, int index,
                                                        int secondaryFlag);

// Translate a valid inventory index to an IFF type ID and execute or queue its reward request.
// Resolve a valid setitem inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestSetItemByInventoryIndex(void* requestState, void* popupContext, int index,
                                                     int secondaryFlag);

// Translate a valid inventory index to an IFF type ID and execute or queue its reward request.
// Resolve a valid part inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestPartByInventoryIndex(void* requestState, void* popupContext, int index, int secondaryFlag);

// Translate a valid inventory index to an IFF type ID and execute or queue its reward request.
// Resolve a valid clubset inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestClubSetByInventoryIndex(void* requestState, void* popupContext, int index,
                                                     int secondaryFlag);

// Translate a valid inventory index to an IFF type ID and execute or queue its reward request.
// Resolve a valid ball inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestBallByInventoryIndex(void* requestState, void* popupContext, int index, int secondaryFlag);

// Translate a valid inventory index to an IFF type ID and execute or queue its reward request.
// Resolve a valid course inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestCourseByInventoryIndex(void* requestState, void* popupContext, int index);

// Return the number of pending item requests in a vector.
unsigned int ItemRequestVector_GetSize(void* self);

// Return the indexed 12-byte pending item request without a bounds check.
QueuedItemReward* Array_GetElement(void* self, int index);

// Layout map: a=borrowed popup context, b=IFF type ID, c=inventory index, d=secondary policy.
// Scalar widths and signed policy remain unchanged.
struct PendingItemRequest {
    unsigned int popupContext, typeId;
    unsigned short inventoryIndex;
    short secondaryFlag;
    // Copy the request's context, type ID, inventory index and policy in ABI order.
    PendingItemRequest& operator=(const PendingItemRequest& o) {
        unsigned int popupContext = o.popupContext, typeId = o.typeId;
        unsigned short inventoryIndex = o.inventoryIndex;
        short secondaryFlag = o.secondaryFlag;
        this->popupContext = popupContext;
        this->typeId = typeId;
        this->inventoryIndex = inventoryIndex;
        this->secondaryFlag = secondaryFlag;
        return *this;
    }
};
struct PendingItemRequestVector {
    PendingItemRequest* data;
    unsigned int size, capacity;
};
struct PendingItemRequestWords {
    unsigned int a, b, c;
    // Copy the same 12-byte request as three words, preserving EH copy semantics.
    PendingItemRequestWords(const PendingItemRequestWords& o) {
        try {
            unsigned int x = o.a, y = o.b;
            a = x;
            b = y;
            c = o.c;
        } catch (...) {
            throw;
        }
    }
};
// Return the larger growth candidate by reference, preserving the original selection order.
inline const unsigned int& LargerRequestCapacity(const unsigned int& growthCandidate,
                                                 const unsigned int& additionalSlots) {
    const unsigned int* r = &growthCandidate;
    if (growthCandidate < additionalSlots)
        r = &additionalSlots;
    return *r;
}
// RAII buffer for pending item requests; retain the type name used by EH relocations.
struct B35CTemp {
    PendingItemRequest* data;
    unsigned int size, cap;
    unsigned int* capref;
    unsigned int remaining;
    // Start an empty temporary allocation for request-vector growth.
    B35CTemp(unsigned int* ref) : data(0), size(0), cap(0), capref(ref), remaining(0) {}
    // Free backing storage; request words themselves own no allocations here.
    ~B35CTemp() {
        if (data) {
            volatile char allocator __attribute__((aligned(4))) = 0;
            size = size - size;
            appFreeUni__FPv(data);
        }
    }
};
#pragma push
#pragma use_lmw_stmw on
// Append a request, growing its buffer with the original guarded allocation/copy sequence.
void ItemRequestVector_AppendCopy(PendingItemRequestVector* self, PendingItemRequest* value);
#pragma pop

// Return the bitset storage pointer while preserving the iterator helper stack stores.
void* SetItemBitReference_GetStorage(void* arg1, void* arg2);

// Read a caddie primary/secondary reward flag, retaining the original two-word volatile copy.
int SaveRewards_TestCaddieFlag(void* requestState, int index, int secondaryFlag);

// Destroy queued reward texts from last to first, then release their vector buffer.
void* dtor_8030B858(void* self, int freeFlag);

// Copy the 0x25C-byte save prefix with its original field-wise assignment order.
void SaveRewards_CopyPrefix(void* destination, void* source);

#pragma push
#pragma use_lmw_stmw on
// Assign a checked bit in the 161-bit set-item update mask.
inline void AssignSetItemBit(unsigned int count, unsigned int* bits, int value) {
    if (count >= kSetItemBits) {
        std::out_of_range error(bitsetSetItemSetRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    unsigned int mask = 1u << (count & 31);
    unsigned int word = count >> 5;
    if (value)
        bits[word] |= mask;
    else
        bits[word] &= ~mask;
}
// Set or clear a checked set-item bit and return the reference.
void* Bitset161Reference_Assign(void* self, int value);
#pragma pop

// Unlock a course inventory index if its save bit is not already set.
void SaveRewards_UnlockCourse(void* self, unsigned char index);

} // extern "C"

// Game entry points in original address order.
extern "C" {

int ItemRequestState_ProcessPopupQueue(void* self) {
    int result = 0;
    if (AsItemRequestState80300EC8(self)->dispatchEnabled == 0)
        return 0;
    if (AsItemRequestState80300EC8(self)->primaryProcessor == 0) {
        if (AsItemRequestState80300EC8(self)->pendingRequestCount != 0) {
            ItemPopupRequest words = *(ItemPopupRequest*)AsItemRequestState80300EC8(self)->pendingRequests;
            void* object = new ItemPopupAllocation(words.b, words.a);
            AsItemRequestState80300EC8(self)->primaryProcessor = object;
            Sound_PlayByName(g_soundManager, tourOpenFactorStrings + kRewardPopupSound, 0, 0, 0, 0,
                             kFloat_0_5_RewardPopupSound, kFloat_200_RewardPopupSound, kFloat_1_RewardPopupSound);
            ItemPopupRequest* array = (ItemPopupRequest*)AsItemRequestState80300EC8(self)->pendingRequests;
            ItemPopupRequest* end = array + AsItemRequestState80300EC8(self)->pendingRequestCount;
            memmove(array, array + 1, (ItemPopupDistance(array, end) - 1) * sizeof(ItemPopupRequest));
            --AsItemRequestState80300EC8(self)->pendingRequestCount;
        } else {
            result = 1;
        }
    }
    ItemPopupInterface* obj = (ItemPopupInterface*)AsItemRequestState80300EC8(self)->primaryProcessor;
    if (obj) {
        if (obj->update()) {
            obj = (ItemPopupInterface*)AsItemRequestState80300EC8(self)->primaryProcessor;
            if (obj) {
                delete obj;
                AsItemRequestState80300EC8(self)->primaryProcessor = 0;
            }
        }
    }
    return result;
}

// Apply queued inventory rewards to their saved primary/secondary flags.
void ItemRequestState_ApplyPendingRewards(void* self) {
    unsigned int i = 0;
    while (i < ItemRequestVector_GetSize(&AsItemRequestState80300EC8(self)->pendingRequests)) {
        QueuedItemReward entry = *Array_GetElement(&AsItemRequestState80300EC8(self)->pendingRequests, i);
        unsigned int kind = entry.typeId >> 26;
        switch (kind) {
        case kRewardPart:
            SaveRewards_SetPartFlags(self, entry.inventoryIndex, (short)entry.secondaryFlag);
            break;
        case kRewardSetItem:
            SaveRewards_SetCostumeSetFlags(self, entry.inventoryIndex, (short)entry.secondaryFlag);
            break;
        case kRewardClubSet:
            SaveRewards_SetClubSetFlags(self, (unsigned char)entry.inventoryIndex, (short)entry.secondaryFlag);
            break;
        case kRewardBall:
            SaveRewards_SetBallFlags(self, (unsigned char)entry.inventoryIndex, (short)entry.secondaryFlag);
            break;
        case kRewardCharacter:
            SaveRewards_SetCharacterFlags(self, (unsigned char)entry.inventoryIndex, (short)entry.secondaryFlag);
            break;
        case kRewardCaddie:
            SaveRewards_SetCaddieFlags(self, (unsigned char)entry.inventoryIndex, (short)entry.secondaryFlag);
            break;
        case kRewardCaddieItem:
            SaveRewards_SetCaddieItemFlags(self, (unsigned char)entry.inventoryIndex, (short)entry.secondaryFlag);
            break;
        case kRewardCourse:

            SaveRewards_UnlockCourse(self, (unsigned char)entry.inventoryIndex);
            break;
        case kRewardUnused:
        case kRewardClub:
        case kRewardConsumable:
            break;
        }
        ++i;
    }
}

// Release the heap-backed wide string at +4 and optionally delete its wrapper.
void* dtor_803019E8(void* self, int freeFlag) {
    if (self != 0) {
        int fieldAddress = (int)self + 4;
        if (fieldAddress != 0 && (((unsigned int*)self)[1] >> 31))
            appFreeUni__FPv(((void**)self)[3]);
        if (freeFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Display one queued text notification and retire a dialog after its update completes.
int ItemRequestDialogs_ProcessQueue(void* self) {
    // Phase 1: honor the dispatch gate and create a text dialog only when idle.
    bool result = false;
    if (AsItemRequestState80300EC8(self)->dispatchEnabled == 0)
        return 0;
    if (AsItemRequestState80300EC8(self)->secondaryProcessor == 0) {
        if (AsItemRequestState80300EC8(self)->textEntryCount != 0) {
            RewardTextEntry* entry = (RewardTextEntry*)AsItemRequestState80300EC8(self)->textEntries;
            A54String name;
            name.kind = entry->kind;
            if (!(entry->flags >> 31)) {
                unsigned int flags = entry->flags, length = entry->length;
                name.flags = flags;
                name.length = length;
                name.data = entry->data;
            } else {
                name.flags = 0;
                name.length = 0;
                name.data = 0;
                WString_ReserveCapacity(&name.flags, entry->length);
                StringAllocatorByte ch;
                volatile StringAllocatorByte dummy;
                ch.value = dummy.value;
                WString_ReplaceRange(&name.flags, 0, 0, entry->data, entry->data + entry->length * 2, &ch.value);
            }
            name.notificationIndex = entry->notificationIndex;
            void* object = new RewardDialogAllocation(3, 10);
            AsItemRequestState80300EC8(self)->secondaryProcessor = object;
            ((CPangyaCommonDialog*)object)
                ->SetText(!(name.flags >> 31) ? (void*)((char*)&name.flags + 2) : (void*)name.data);
            // Phase 2: compact the queue, preserving each wide string's inline/heap ownership.
            RewardTextEntry* base = (RewardTextEntry*)AsItemRequestState80300EC8(self)->textEntries;
            int count = AsItemRequestState80300EC8(self)->textEntryCount;
            RewardTextEntry* end = base + count;
            if ((end - base) - 1) {
                RewardTextEntry* dst = base;
                for (RewardTextEntry* src = base + 1; src < end; ++src, ++dst) {
                    dst->kind = src->kind;
                    if (!(dst->flags >> 31) && !(src->flags >> 31)) {
                        unsigned int flags = src->flags, length = src->length;
                        dst->flags = flags;
                        dst->length = length;
                        dst->data = src->data;
                    } else {
                        unsigned int length = (dst->flags >> 31) ? dst->length : (*(unsigned char*)&dst->flags & 127);
                        unsigned int data, size;
                        if (!(src->flags >> 31)) {
                            data = (unsigned int)((char*)src + 6);
                            size = *(unsigned char*)&src->flags & 127;
                        } else {
                            data = src->data;
                            size = src->length;
                        }
                        volatile StringAllocatorByte dummy;
                        StringAllocatorByte ch;
                        ch.value = dummy.value;
                        WString_ReplaceRange((char*)dst + 4, 0, length, data, data + size * 2, &ch.value);
                    }
                    dst->notificationIndex = src->notificationIndex;
                }
            }
            --AsItemRequestState80300EC8(self)->textEntryCount;
            RewardTextEntry* last = (RewardTextEntry*)AsItemRequestState80300EC8(self)->textEntries +
                                    AsItemRequestState80300EC8(self)->textEntryCount;
            if (last && (char*)last + 4 && (last->flags >> 31))
                appFreeUni__FPv((void*)last->data);
        } else
            result = 1;
    }
    // Phase 3: tick the active dialog and release it after it reports completion.
    RewardDialogInterface* object = (RewardDialogInterface*)AsItemRequestState80300EC8(self)->secondaryProcessor;
    if (object && object->update(kRewardDialogZeroUpdateStep)) {
        object = (RewardDialogInterface*)AsItemRequestState80300EC8(self)->secondaryProcessor;
        if (object) {
            delete object;
            AsItemRequestState80300EC8(self)->secondaryProcessor = 0;
        }
    }
    return result;
}

// Mark each queued text notification in the saved notification bitset.
void ItemRequestState_MarkTextNotifications(RewardTextStorage* self) {
    unsigned int count = 0;
    unsigned int offset = 0;
    if (self->size)
        while (count < self->size) {
            A54String name;
            RewardTextEntry* entry = (RewardTextEntry*)((char*)self->data + offset);

            name.kind = entry->kind;
            if (!(entry->flags >> 31)) {
                unsigned int length = entry->length;

                unsigned int flags = entry->flags;
                name.flags = flags;
                name.length = length;
                name.data = entry->data;
            } else {
                StringAllocatorByte ch;
                volatile StringAllocatorByte dummy;
                name.flags = 0;
                name.length = 0;
                name.data = 0;

                WString_ReserveCapacity(&name.flags, entry->length);
                ch.value = dummy.value;
                WString_ReplaceRange(&name.flags, 0, 0, entry->data, entry->data + entry->length * 2, &ch.value);
            }
            int index = entry->notificationIndex;
            name.notificationIndex = index;
            switch (name.kind)
            case 0: {
                if (0 == ((int&)GetSaveDataSlot())) {
                    if (!s_saveDataInitialized) {
                        SaveData_Ctor(s_saveData);
                        s_saveDataInitialized = 1;
                    }
                    ((int&)GetSaveDataSlot()) = (int)s_saveData;
                }

                SaveRewardPrefix first =
                    *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
                const SaveBitReference& testRef = MakeNotificationBitReference(&first.textNotificationFlags, index);
                if (!TestTextNotificationBit(testRef)) {
                    unsigned int instance = ((int&)GetSaveDataSlot());
                    if (0 == ((int&)GetSaveDataSlot())) {
                        if (s_saveDataInitialized == 0) {
                            SaveData_Ctor(s_saveData);
                            s_saveDataInitialized = 1;
                        }
                        ((int&)GetSaveDataSlot()) = (int)s_saveData;
                    }

                    SaveRewardPrefix second = *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)(
                        (void*)((int&)GetSaveDataSlot()));
                    SaveBitReference ref = MakeNotificationBitReference(&second.textNotificationFlags, index);
                    SaveBitReference r = ref;
                    if (r.bit >= 3) {
                        std::out_of_range error(bitsetNotificationSetRangeErrorText,
                                                tourOpenFactorStrings + kRangeErrorType);
                    }

                    *r.word |= 1u << r.bit;
                    if (!instance) {
                        if (!s_saveDataInitialized) {
                            SaveData_Ctor(s_saveData);
                            s_saveDataInitialized = 1;
                        }
                        ((int&)GetSaveDataSlot()) = (int)s_saveData;
                    }
                    SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &second);
                }
            }
                ++count;
            offset += 0x14;
        }
}

// Set the requested character policy and its update bit unless that policy is already present.
void SaveRewards_SetCharacterFlags(void* self, unsigned int index, short secondaryFlag) {
    // Acquire save storage, test the requested policy, then publish changed flags and update bits.
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    struct Record {
        unsigned short h[26];
    };
    Record second;
    unsigned short* entry =
        AsRewardSaveRecord(((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
            ->characters[index]
            .words;
    unsigned short first[26] = {entry[0],  entry[1],  entry[2],  entry[3],  entry[4],  entry[5],  entry[6],
                                entry[7],  entry[8],  entry[9],  entry[10], entry[11], entry[12], entry[13],
                                entry[14], entry[15], entry[16], entry[17], entry[18], entry[19], entry[20],
                                entry[21], entry[22], entry[23], entry[24], entry[25]};
    if (secondaryFlag ? ((RewardStatusPair*)first)->secondary : ((RewardStatusPair*)first)->primary)
        return;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    second =
        *(Record*)&AsRewardSaveRecord(((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
             ->characters[index];
    ((RewardStatusPair*)&second)->primary = 1;
    if (secondaryFlag)
        ((RewardStatusPair*)&second)->secondary = 1;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetCharacterEquipmentRecord((void*)((int&)GetSaveDataSlot()), &second, index);
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix copy =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    SaveBitReference r = MakeSaveBitReference(&copy.characterUpdateFlags, index);
    if (r.bit >= kCharacterCount) {
        std::out_of_range error(bitsetCharacterCaddieSetRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    *r.word |= 1u << r.bit;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &copy);
    (void)self;
}

// Set the requested caddie policy and its update bit unless already present.
void SaveRewards_SetCaddieFlags(void* self, unsigned char index, short secondaryFlag) {
    // Acquire save storage, test the requested policy, then publish changed flags and update bits.
    (void)self;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    RewardStatusPair pair = *(RewardStatusPair*)&AsRewardSaveRecord(
                                 ((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
                                 ->caddies[index];
    if (secondaryFlag ? pair.secondary : pair.primary)
        return;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    RewardStatusPair value = *(RewardStatusPair*)&AsRewardSaveRecord(
                                  ((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
                                  ->caddies[index];
    value.primary = 1;
    if (secondaryFlag)
        value.secondary = 1;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetCaddieRecordAt((void*)((int&)GetSaveDataSlot()), &value, index);
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix copy =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    SaveBitReference ref;
    ref.word = &copy.caddieUpdateFlags;
    ref.bit = index;
    SaveBitReference r = ref;
    if (r.bit >= kCaddieCount) {
        std::out_of_range error(bitsetCharacterCaddieSetRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    *r.word |= 1u << r.bit;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &copy);
}

// Set primary/secondary caddie-item bits and mark the changed inventory index.
void SaveRewards_SetCaddieItemFlags(void* requestState, unsigned char index, short secondaryFlag) {
    // Acquire save storage, test the requested policy, then publish changed flags and update bits.
    (void)requestState;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    if (HasCaddieItemFlag(SaveRewardPrefix(*(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)(
                              (void*)((int&)GetSaveDataSlot()))),
                          index, secondaryFlag))
        return;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix copy =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    SetCaddieItemBit(MakeSaveBitReference(&copy.caddieItemPrimaryFlags, index));
    if (secondaryFlag)
        SetCaddieItemBit(MakeSaveBitReference(&copy.caddieItemSecondaryFlags, index));
    int instance = SetCaddieItemNotificationBit(MakeSaveBitReference(&copy.caddieItemUpdateFlags, index));
    if (instance == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &copy);
}

// Record set-item update state, then grant flags to its member parts.
void SaveRewards_SetCostumeSetFlags(void* requestState, int index, int secondaryFlag) {
    if (SaveData_TestAllSetItemPartFlags(requestState, (void*)(unsigned char)index, secondaryFlag) == 0) {
        SaveRewards_SetSetMemberPartFlags(requestState, index, secondaryFlag);
        void* handle = SaveData_GetCurrentRecord(SaveData_GetInstance());
        char local[0x270];
        SaveRewards_CopyPrefix(local, handle);
        struct {
            void* data;
            int index;
        } result;
        result.data = (void*)SetItemBitReference_GetStorage(local + 0x1a8, (void*)index);
        result.index = index;
        Bitset161Reference_Assign(&result, 1);
        void* h = SaveData_GetInstance();
        SaveData_SetInventoryAndStatisticsPrefix(h, local);
    }
}

// Resolve every set member to a part index and grant its requested reward flags.
void SaveRewards_SetSetMemberPartFlags(void* requestState, int index, int secondaryFlag) {
    (void)requestState;
    void* selection = (void*)ItemInventory_GetSetItemIdAt(g_sharedDoc->itemInventory, index);
    IFF_STRUCT::sSetItem* setItem =
        (IFF_STRUCT::sSetItem*)ItemInventory_FindSetItemData(g_sharedDoc->itemInventory, (int)selection);
    unsigned char count = 0;
    while (count < setItem->itemCount) {
        unsigned short bit =
            (unsigned short)ItemInventory_FindPartIndex(g_sharedDoc->itemInventory, (int)setItem->itemIds[count]);
        if (((int&)GetSaveDataSlot()) == 0) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            ((int&)GetSaveDataSlot()) = (int)s_saveData;
        }
        SaveRewardPrefix first =
            *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
        if (!TestSetPartBitForUpdate(secondaryFlag ? MakeSaveBitReference(&first.partSecondaryFlags[0], bit)
                                                   : MakeSaveBitReference(&first.partPrimaryFlags[0], bit))) {
            if (((int&)GetSaveDataSlot()) == 0) {
                if (!s_saveDataInitialized) {
                    SaveData_Ctor(s_saveData);
                    s_saveDataInitialized = 1;
                }
                ((int&)GetSaveDataSlot()) = (int)s_saveData;
            }
            SaveRewardPrefix copy =
                *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
            SetSetPartBit(MakeSaveBitReference(&copy.partPrimaryFlags[0], bit));
            if (secondaryFlag)
                SetSetPartBit(MakeSaveBitReference(&copy.partSecondaryFlags[0], bit));
            int instance = SetSetPartNotificationBit(MakeSaveBitReference(&copy.partUpdateFlags[0], bit));
            if (instance == 0) {
                if (!s_saveDataInitialized) {
                    SaveData_Ctor(s_saveData);
                    s_saveDataInitialized = 1;
                }
                ((int&)GetSaveDataSlot()) = (int)s_saveData;
            }
            SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &copy);
        }
        count++;
    }
}

// Set primary/secondary part bits and mark the changed inventory index.
void SaveRewards_SetPartFlags(void* self, unsigned short index, short secondaryFlag) {
    // Acquire save storage, test the requested policy, then publish changed flags and update bits.
    (void)self;
    SaveRewardPrefix copy;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix first =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    if (HasPartFlag(first, index, secondaryFlag))
        return;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    copy = *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    SetPartBit(MakeSaveBitReference(&copy.partPrimaryFlags[0], index));
    if (secondaryFlag)
        SetPartBit(MakeSaveBitReference(&copy.partSecondaryFlags[0], index));
    int instance = SetPartNotificationBit(MakeSaveBitReference(&copy.partUpdateFlags[0], index));
    if (instance == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &copy);
}

// Set the requested club-set policy and its update bit unless already present.
void SaveRewards_SetClubSetFlags(void* self, unsigned char index, short secondaryFlag) {
    // Acquire save storage, test the requested policy, then publish changed flags and update bits.
    (void)self;
    ClubSetRewardWrite value;
    ClubSetRewardWrite pair;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    pair.pair = *(ClubSetRewardStatus*)&AsRewardSaveRecord(
                     ((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
                     ->clubSets[index];
    if (secondaryFlag ? pair.pair.secondary : pair.pair.primary)
        return;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    value.pair = *(ClubSetRewardStatus*)&AsRewardSaveRecord(
                      ((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
                      ->clubSets[index];
    value.pair.primary = 1;
    if (secondaryFlag)
        value.pair.secondary = 1;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetClubSetRecordAt((void*)((int&)GetSaveDataSlot()), &value.pair, index);
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix copy =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    SaveBitReference r = MakeSaveBitReference(&copy.clubSetUpdateFlags, index);
    if (r.bit >= kClubSetCount) {
        std::out_of_range error(bitsetClubSetRewardSetRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    unsigned int* w = r.word;
    unsigned int b = r.bit;
    unsigned int mask = 1u << b;
    unsigned int old = w[0];
    unsigned int nw = old | mask;
    w[0] = nw;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &copy);
}

// Set the requested ball policy and its update bit unless already present.
void SaveRewards_SetBallFlags(void* self, unsigned int index, unsigned int secondaryFlag) {
    // Acquire save storage, test the requested policy, then publish changed flags and update bits.
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    struct Bits {
        CW_BITS3(signed char primary : 1, signed char secondary : 1, unsigned char rest : 6)
    };
    struct BitsValue {
        Bits pair;
        unsigned short pad;
    };
    Bits probe =
        *(Bits*)&AsRewardSaveRecord(((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
             ->ballFlags[index];
    if (secondaryFlag ? probe.secondary : probe.primary)
        return;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    BitsValue value;
    value.pair =
        *(Bits*)&AsRewardSaveRecord(((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
             ->ballFlags[index];
    value.pair.primary = 1;
    if (secondaryFlag)
        value.pair.secondary = 1;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetBallRecordAt((void*)((int&)GetSaveDataSlot()), &value, index);
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix copy =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    SaveBitReference r = MakeSaveBitReference(&copy.ballUpdateFlags, index);
    if (r.bit >= kBallCount) {
        extern char bitsetBallRewardSetRangeErrorText[];
        std::out_of_range error(bitsetBallRewardSetRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    unsigned int* w = r.word;
    unsigned int b = r.bit;
    unsigned int mask = 1u << b;
    unsigned int old = w[0];
    unsigned int nw = old | mask;
    w[0] = nw;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &copy);
}

// Set the saved course bit after checking its inventory index.
void SaveRewards_UnlockCourse(void* self, unsigned char index) {
    // Acquire save storage, test the requested policy, then publish changed flags and update bits.
    (void)self;
    SaveRewardPrefix second;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix first =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    SaveBitReference r1 = MakeSaveBitReference(&first.unlockedCourses, index);
    if (r1.bit >= kCourseCount) {
        std::out_of_range error(bitsetTestRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    if ((*r1.word & (1u << r1.bit)) == 0) {
        if (((int&)GetSaveDataSlot()) == 0) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            ((int&)GetSaveDataSlot()) = (int)s_saveData;
        }
        second = *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
        SaveBitReference r2 = MakeSaveBitReference(&second.unlockedCourses, index);
        if (r2.bit >= kCourseCount) {
            std::out_of_range error(bitset14SetRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
        }
        *r2.word |= 1u << r2.bit;
        if (((int&)GetSaveDataSlot()) == 0) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            ((int&)GetSaveDataSlot()) = (int)s_saveData;
        }
        SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &second);
    }
}

// Set one saved notification bit after checking its index.
void SaveRewards_SetOtherNotificationBit(void* self, unsigned int index) {
    // Acquire save storage, test the requested policy, then publish changed flags and update bits.
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix copy =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    const SaveBitReference& ref = MakeSaveBitReference(copy.otherNotificationFlags, index);
    SaveBitReference r = ref;
    if (r.bit >= kOtherNotificationBits) {
        std::out_of_range error(bitset45AssignRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }
    unsigned int* words = r.word;
    unsigned int offset = index >> 5;
    unsigned int bit = index & 31;
    unsigned int mask = 1u << bit;
    words[offset] = words[offset] | mask;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveData_SetInventoryAndStatisticsPrefix((void*)((int&)GetSaveDataSlot()), &copy);
}

#pragma push
#pragma use_lmw_stmw on
// Test the requested character reward policy without changing saved state.
int SaveRewards_TestCharacterFlag(void* requestState, int index, int secondaryFlag) {
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    struct Record {
        unsigned short v0, v1, v2, v3, v4, v5, v6, v7, v8;
        unsigned short v9, v10, v11, v12, v13, v14, v15, v16, v17;
        unsigned short v18, v19, v20, v21, v22, v23, v24, v25;
    };
    Record values =
        *(Record*)&AsRewardSaveRecord(((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
             ->characters[index];
    if (secondaryFlag)
        return (int)((*(unsigned char*)&values << 25) & 0xC0000000) >> 31;
    return (int)((*(unsigned char*)&values << 24) & 0xC0000000) >> 31;
}
#pragma pop

// Test the requested caddie reward policy without changing saved state.
int SaveRewards_TestCaddieFlag(void* requestState, int index, int secondaryFlag) {
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    unsigned int* value =
        AsRewardSaveRecord(((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
            ->caddies[(unsigned char)index]
            .words;
    volatile unsigned int words[2];
    words[0] = value[0];
    words[1] = value[1];
    if (secondaryFlag)
        return ((*(volatile unsigned char*)words) & 0x60) << 25 >> 31;
    return ((*(volatile unsigned char*)words) & 0xC0) << 24 >> 31;
}

// Test the selected primary or secondary caddie-item bit.
int SaveRewards_TestCaddieItemFlag(void* requestState, unsigned int index, int secondaryFlag) {
    (void)requestState;
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix copy =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    return TestCaddieItemFlagBit(secondaryFlag ? MakeSaveBitReference(&copy.caddieItemSecondaryFlags, index)
                                               : MakeSaveBitReference(&copy.caddieItemPrimaryFlags, index));
}

// Test the selected primary or secondary part bit.
int SaveRewards_TestPartFlag(void* requestState, unsigned int index, int secondaryFlag) {
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    (void)requestState;
    SaveRewardPrefix copy =
        *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    return TestPartBit(secondaryFlag ? MakeSaveBitReference(&copy.partSecondaryFlags[0], index)
                                     : MakeSaveBitReference(&copy.partPrimaryFlags[0], index));
}

// Return whether every member part of the selected set has the requested policy.
int SaveData_TestAllSetItemPartFlags(void* requestState, void* arg2, int secondaryFlag) {
    void* selection = (void*)ItemInventory_GetSetItemIdAt(g_sharedDoc->itemInventory, (int)arg2);
    int result = 1;
    IFF_STRUCT::sSetItem* table =
        (IFF_STRUCT::sSetItem*)ItemInventory_FindSetItemData(g_sharedDoc->itemInventory, (int)selection);
    (void)requestState;
    for (unsigned char i = 0; i < table->itemCount; ++i) {
        unsigned int value = ((IFF_STRUCT::sSetItem*)table)->itemIds[i];
        unsigned short index = (unsigned short)ItemInventory_FindPartIndex(g_sharedDoc->itemInventory, (int)value);
        if (((int&)GetSaveDataSlot()) == 0) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            ((int&)GetSaveDataSlot()) = (int)s_saveData;
        }
        SaveRewardPrefix copy =
            *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
        if (TestSetPartBit(secondaryFlag ? MakeSaveBitReference(&copy.partSecondaryFlags[0], index)
                                         : MakeSaveBitReference(&copy.partPrimaryFlags[0], index)) == 0)
            result = 0;
    }
    return result;
}

// Test the requested club-set reward policy without changing saved state.
int SaveRewards_TestClubSetFlag(void* requestState, int index, int secondaryFlag) {
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    int savedIndex = index;
    void* base = ((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    unsigned char* entry = (unsigned char*)&AsRewardSaveRecord(base)->clubSets[(unsigned char)savedIndex];
    unsigned char b0 = entry[0];
    unsigned char b1 = entry[1];
    volatile struct {
        unsigned char a, b;
    } values;
    values.a = b0;
    values.b = b1;
    (void)values;
    if (secondaryFlag)
        return (b0 & 0x60) << 25 >> 31;
    return (b0 & 0xC0) << 24 >> 31;
}

// Test the requested ball reward policy without changing saved state.
int SaveRewards_TestBallFlag(void* requestState, int index, int secondaryFlag) {
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    unsigned char value =
        AsRewardSaveRecord(((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot())))
            ->ballFlags[index];
    if (secondaryFlag)
        return (int)(value & 0x60) << 25 >> 31;
    return (int)(value & 0xC0) << 24 >> 31;
}

// Test the saved unlock bit for a bounds-checked course index.
bool SaveRewards_IsCourseUnlocked(void* arg1, unsigned int index) {
    if (((int&)GetSaveDataSlot()) == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    SaveRewardPrefix e;
    e = *(SaveRewardPrefix*)((void* (*)(void*))SaveData_GetCurrentRecord)((void*)((int&)GetSaveDataSlot()));
    const SaveBitReference& reference = MakeSaveBitReference(&e.unlockedCourses, index);
    SaveBitReference checked = reference;
    (void)arg1;
    if (kCourseCount <= index) {
        std::out_of_range error(bitsetTestRangeErrorText, tourOpenFactorStrings + kRangeErrorType);
    }

    return ((*checked.word) & (1u << index)) != 0;
}

// Resolve a valid character inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestCharacterByInventoryIndex(void* requestState, void* popupContext, int index,
                                                       int secondaryFlag) {
    if (index != -1) {
        CSharedDoc* sharedDoc = g_sharedDoc;
        int typeId = ItemInventory_GetCharacterIdAt(sharedDoc->itemInventory, index);
        ItemRequestState_QueueRequestIfNeeded(requestState, popupContext, typeId, (unsigned short)index, secondaryFlag);
    }
}

// Resolve a valid caddie inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestCaddieByInventoryIndex(void* requestState, void* popupContext, int index,
                                                    int secondaryFlag) {
    if (index != -1) {
        CSharedDoc* sharedDoc = g_sharedDoc;
        int typeId = ItemInventory_GetCaddieIdAt(sharedDoc->itemInventory, index);
        ItemRequestState_QueueRequestIfNeeded(requestState, popupContext, typeId, (unsigned short)index, secondaryFlag);
    }
}

// Resolve a valid caddieitem inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestCaddieItemByInventoryIndex(void* requestState, void* popupContext, int index,
                                                        int secondaryFlag) {
    if (index != -1) {
        CSharedDoc* sharedDoc = g_sharedDoc;
        int typeId = ItemInventory_GetCaddieItemIdAt(sharedDoc->itemInventory, index);
        ItemRequestState_QueueRequestIfNeeded(requestState, popupContext, typeId, (unsigned short)index, secondaryFlag);
    }
}

// Resolve a valid setitem inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestSetItemByInventoryIndex(void* requestState, void* popupContext, int index,
                                                     int secondaryFlag) {
    if (index != -1) {
        CSharedDoc* sharedDoc = g_sharedDoc;
        int typeId = ItemInventory_GetSetItemIdAt(sharedDoc->itemInventory, index);
        ItemRequestState_QueueRequestIfNeeded(requestState, popupContext, typeId, (unsigned short)index, secondaryFlag);
    }
}

// Resolve a valid part inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestPartByInventoryIndex(void* requestState, void* popupContext, int index,
                                                  int secondaryFlag) {
    if (index != -1) {
        CSharedDoc* sharedDoc = g_sharedDoc;
        int typeId = ItemInventory_GetPartIdAt(sharedDoc->itemInventory, index);
        ItemRequestState_QueueRequestIfNeeded(requestState, popupContext, typeId, (unsigned short)index, secondaryFlag);
    }
}

// Resolve a valid clubset inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestClubSetByInventoryIndex(void* requestState, void* popupContext, int index,
                                                     int secondaryFlag) {
    if (index != -1) {
        CSharedDoc* sharedDoc = g_sharedDoc;
        int typeId = ItemInventory_GetClubSetIdAt(sharedDoc->itemInventory, index);
        ItemRequestState_QueueRequestIfNeeded(requestState, popupContext, typeId, (unsigned short)index, secondaryFlag);
    }
}

// Resolve a valid ball inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestBallByInventoryIndex(void* requestState, void* popupContext, int index,
                                                  int secondaryFlag) {
    if (index != -1) {
        CSharedDoc* sharedDoc = g_sharedDoc;
        int typeId = ItemInventory_GetBallIdAt(sharedDoc->itemInventory, index);
        ItemRequestState_QueueRequestIfNeeded(requestState, popupContext, typeId, (unsigned short)index, secondaryFlag);
    }
}

// Resolve a valid course inventory index to its IFF type ID and queue the reward.
void ItemRequestState_RequestCourseByInventoryIndex(void* requestState, void* popupContext, int index) {
    if (index != -1) {
        CSharedDoc* sharedDoc = g_sharedDoc;
        int typeId = ItemInventory_GetCourseIdAt(sharedDoc->itemInventory, index);
        ItemRequestState_QueueRequestIfNeeded(requestState, popupContext, typeId, (unsigned short)index, 1);
    }
}

// Return the pending request count from the vector header.
unsigned int ItemRequestVector_GetSize(void* self) {
    return *(unsigned int*)((char*)self + 4);
}

#pragma push
#pragma use_lmw_stmw on
// Copy one borrowed request into the pending queue, growing storage transactionally.
void ItemRequestVector_AppendCopy(PendingItemRequestVector* self, PendingItemRequest* value) {
    // Fast path: copy popup context, IFF type, inventory selector, and reward policy.
    volatile char allocator __attribute__((aligned(4))) = 0;
    if (self->size < self->capacity) {
        unsigned int size = self->size + 1;
        PendingItemRequest* data = self->data;
        self->size = size;
        unsigned int popupContext = value->popupContext, typeId = value->typeId;
        unsigned short inventoryIndex = value->inventoryIndex;
        short secondaryFlag = value->secondaryFlag;
        PendingItemRequest* out = data + (size - 1);
        out->popupContext = popupContext;
        out->typeId = typeId;
        out->inventoryIndex = inventoryIndex;
        out->secondaryFlag = secondaryFlag;
        return;
    }
    volatile char allocator2 __attribute__((aligned(4))) = 0;
    volatile char allocator3 __attribute__((aligned(4))) = 0;
    if (kRequestMaxCapacity - self->capacity < 1) {
        throw std::length_error(tourOpenFactorStrings);
    }
    // Growth transaction: construct the new tail before reading an aliased old entry.
    B35CTemp replacement(&self->capacity);
    unsigned int additionalSlots = self->size + 1 - self->capacity;
    unsigned int oldCapacity = self->capacity;
    if (additionalSlots > kRequestMaxCapacity - oldCapacity) {
        throw std::length_error(tourOpenFactorStrings);
    }
    unsigned int newCapacity;
    if (oldCapacity < kRequestSmallGrowthLimit) {
        unsigned int grow = 3 * (oldCapacity + 1) / 5;
        newCapacity = oldCapacity + LargerRequestCapacity(grow, additionalSlots);
    } else if (oldCapacity < kRequestMediumGrowthLimit) {
        unsigned int grow = (oldCapacity + 1) >> 1;
        newCapacity = oldCapacity + LargerRequestCapacity(grow, additionalSlots);
    } else {
        newCapacity = kRequestMaxCapacity;
    }
    volatile char allocator4 __attribute__((aligned(4))) = 0;
    if (newCapacity > kRequestMaxCapacity) {
        throw std::length_error(tourOpenFactorStrings);
    }
    replacement.data = (PendingItemRequest*)appMallocDummy__FUl(newCapacity * sizeof(PendingItemRequest));
    unsigned int oldCount = self->size;
    PendingItemRequest* destination = replacement.data + oldCount;
    PendingItemRequest* slot = destination + replacement.size;
    *slot = *value;
    replacement.remaining = oldCount;
    replacement.cap = newCapacity;
    volatile char allocator5 __attribute__((aligned(4))) = 0;
    replacement.size = replacement.size + 1;
    PendingItemRequest* source = self->data + self->size;
    PendingItemRequest* begin = self->data;
    volatile char allocator6 __attribute__((aligned(4))) = 0;
    // Move the old prefix backward; remaining counts uncommitted leading entries.
    while (source > begin) {
        --destination;
        --source;
        new (destination) PendingItemRequestWords(*(PendingItemRequestWords*)source);
        replacement.remaining = replacement.remaining - 1;
        replacement.size = replacement.size + 1;
    }
    // Commit only after all copies succeed; temp then frees the retired array.
    PendingItemRequest* oldData = self->data;
    PendingItemRequest* newData = replacement.data;
    unsigned int newCount = replacement.size;
    self->size = 0;
    unsigned int retiredCapacity = self->capacity;
    unsigned int replacementCapacity = replacement.cap;
    replacement.data = oldData;
    volatile char allocator7 __attribute__((aligned(4))) = 0;
    self->capacity = replacementCapacity;
    replacement.cap = retiredCapacity;
    self->data = newData;
    self->size = newCount;
    replacement.size = 0;
}
#pragma pop

// Borrow an indexed request from the caller-validated vector range.
QueuedItemReward* Array_GetElement(void* self, int index) {
    return (QueuedItemReward*)(*(unsigned int**)self) + index;
}

// Release each text entry's heap string and the entry array, then optionally delete its wrapper.
void* dtor_8030B858(void* self, int freeFlag) {
    if (self != 0) {
        void** data = (void**)self;
        if (data[0] != 0) {
            volatile int* countField = (volatile int*)self + 1;
            char* entry;
            int count = *countField;
            entry = (char*)data[0] + count * 0x14;
            *countField = count - count;
            while (0 != count) {
                entry -= 0x14;
                if (entry != 0 && entry + 4 != 0 && (((unsigned int*)(entry + 4))[0] >> 31))
                    appFreeUni__FPv(*(void**)(entry + 0xC));
                count = count - 1;
            }
            appFreeUni__FPv(data[0]);
        }
        if (0 < freeFlag)
            __dl__FPv(self);
    }
    return self;
}

// Clear and release a counted scalar buffer, then optionally delete its wrapper.
void* dtor_8030B914(void* self, int freeFlag) {
    if (self != 0) {
        void* resource = *(void**)self;
        if (resource != 0) {
            volatile int* field = (volatile int*)((char*)self + 4);
            int value = *field;
            *field = value - value;
            appFreeUni__FPv(resource);
        }
        if (freeFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Reset the count and release the scalar buffer, then optionally delete its wrapper.
void* dtor_8030B984(void* self, int freeFlag) {
    if (self != 0) {
        *(int*)((char*)self + 4) = 0;
        if (self != 0 && *(void**)self != 0) {
            *(int*)((char*)self + 4) = 0;
            appFreeUni__FPv(*(void**)self);
        }
        if (freeFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Lazily initialize and return the shared save-data instance.
void* SaveData_GetInstance() {
    if (((int&)GetSaveDataSlot()) == 0) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((int&)GetSaveDataSlot()) = (int)s_saveData;
    }
    return (void*)((int&)GetSaveDataSlot());
}

// Copy the saved inventory/statistics prefix in the recovered member order.
void SaveRewards_CopyPrefix(void* destination, void* source) {
    *(SaveRewardPrefix*)destination = *(SaveRewardPrefix*)source;
}

// Return the borrowed bit-reference storage while preserving iterator stack writes.
void* SetItemBitReference_GetStorage(void* setItemBits, void* arg2) {
    void* volatile local2;
    void* volatile local1;
    local1 = setItemBits;
    local2 = arg2;
    return setItemBits;
}

#pragma push
#pragma use_lmw_stmw on
// Assign the indexed set-item bit and return the original reference.
void* Bitset161Reference_Assign(void* self, int value) {
    AssignSetItemBit(*(unsigned int*)((char*)self + 4), *(unsigned int**)self, value);
    return self;
}
#pragma pop
} // extern "C"
