#include "classes/CItemManager.h"

// Lookup contract: returned records are borrowed from CItemManager's arrays.
// A lookup releases its optional mutex before returning; it does not pin a record.
// Reload/destruction invalidates those addresses. Pointer-typed typeId arguments
// carry a 32-bit integer ID, never an address to dereference. Searches are linear.
// Description lookup is the exception to optional locking: it performs no lock.
// Destructor deleteFlag > 0 frees the wrapper as well as its owned allocation.
// IFF type ids encode the table in their upper six bits; part ids also encode character and body slot.
enum ItemIdFields {
    kBuiltInCourseCount = 3, kAuxPartNoStatBonus = 99, kStatCount = 5,
    kCaddieItemOwnerShift = 21, kCaddieItemOwnerMask = 0x1F, kCaddieTypeIdBase = 0x1C000000,
    kItemKindShift = 26, kPartCharacterShift = 18, kPartTypeIdBase = 0x08000000,
    kDefaultPartFlag = 0x400, kPartSlotStride = 0x2000, kOutfitSlotCount = 24,
    kItemCharacter = 1,
    kItemPart = 2,
    kItemClub = 3,
    kItemClubSet = 4,
    kItemBall = 5,
    kItemConsumable = 6,
    kItemCaddie = 7,
    kItemCaddieItem = 8,
    kItemSet = 9,
    kItemCourse = 10,
    kItemHairStyle = 15,
    kItemAuxPart = 28
};

// Translation unit 0x8018ED64-0x80190474 (24 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void ItemInventory_LockMutex(void*);
void ItemInventory_UnlockMutex(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

struct OwnedObject {
    void* object;
    void* resetState;
};

// Reset the auxiliary state, release the owned allocation, and optionally delete this wrapper.
void* OwnedObject_dtor(OwnedObject* self, int deleteFlag) {
    if (self != 0) {
        self->resetState = 0;
        if (self != 0 && self->object != 0) {
            self->resetState = 0;
            appFreeUni__FPv(self->object);
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Reset the auxiliary state, release the owned allocation, and optionally delete this wrapper.
void* dtor_80190400(OwnedObject* self, int deleteFlag) {
    if (self != 0) {
        self->resetState = 0;
        if (self != 0 && self->object != 0) {
            self->resetState = 0;
            appFreeUni__FPv(self->object);
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindCharacterData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        CharacterTableEntry* entry = (CharacterTableEntry*)list->characters.data;
        CharacterTableEntry* end = entry + list->characters.size;
        // The inventory mutex occupies the storage at +4.
        // 0x8018ED64: preserve the canonical end assignment on exhaustion.
        while (entry != end || (entry = end, false)) {
            if ((unsigned int)typeId == entry->key) break;
            entry++;
        }
        if (entry != end)
            result = (char*)&entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        CharacterTableEntry* entry = (CharacterTableEntry*)list->characters.data;
        CharacterTableEntry* end = entry + list->characters.size;
        while (entry != end || (entry = end, false)) {
            if ((unsigned int)typeId == entry->key) break;
            entry++;
        }
        if (entry != end)
            result = (char*)&entry->record;
    }
    return result;
}

// Return the first matching entry in [entry, end), or end; no locking here.
static inline PartTableEntry* FindPartEntryByOpaqueId(PartTableEntry* entry, PartTableEntry* end, void* typeId) {
    for (; entry != end; ++entry) {
        if ((unsigned int)typeId == entry->key)
            return entry;
    }
    return end;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindPartData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        PartTableEntry* entry = (PartTableEntry*)list->parts.data;
        PartTableEntry* end = entry + list->parts.size;
        entry = FindPartEntryByOpaqueId(entry, end, typeId);
        if (entry != end)
            result = (char*)&entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        PartTableEntry* entry = (PartTableEntry*)list->parts.data;
        PartTableEntry* end = entry + list->parts.size;
        entry = FindPartEntryByOpaqueId(entry, end, typeId);
        if (entry != end)
            result = (char*)&entry->record;
    }
    return result;
}

// Return the first matching entry in [entry, end), or end; no locking here.
static inline ClubTableEntry* FindClubEntryByOpaqueId(ClubTableEntry* entry, ClubTableEntry* end, void* typeId) {
    for (; entry != end; ++entry) {
        if ((unsigned int)typeId == entry->key)
            return entry;
    }
    return end;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindClubData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        ClubTableEntry* entry = (ClubTableEntry*)list->clubs.data;
        ClubTableEntry* end = entry + list->clubs.size;
        entry = FindClubEntryByOpaqueId(entry, end, typeId);
        if (entry != end)
            result = (char*)&entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        ClubTableEntry* entry = (ClubTableEntry*)list->clubs.data;
        ClubTableEntry* end = entry + list->clubs.size;
        entry = FindClubEntryByOpaqueId(entry, end, typeId);
        if (entry != end)
            result = (char*)&entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindClubSetData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        ClubSetTableEntry* entry = list->clubSets.data;
        ClubSetTableEntry* end = entry + list->clubSets.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        ClubSetTableEntry* entry = list->clubSets.data;
        ClubSetTableEntry* end = entry + list->clubSets.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindBallData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        BallTableEntry* entry = list->balls.data;
        BallTableEntry* end = entry + list->balls.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        BallTableEntry* entry = list->balls.data;
        BallTableEntry* end = entry + list->balls.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindItemData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        ItemTableEntry* entry = list->items.data;
        ItemTableEntry* end = entry + list->items.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        ItemTableEntry* entry = list->items.data;
        ItemTableEntry* end = entry + list->items.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindCaddieData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        CaddieTableEntry* entry = list->caddies.data;
        CaddieTableEntry* end = entry + list->caddies.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        CaddieTableEntry* entry = list->caddies.data;
        CaddieTableEntry* end = entry + list->caddies.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindCaddieItemData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        CaddieItemTableEntry* entry = list->caddieItems.data;
        CaddieItemTableEntry* end = entry + list->caddieItems.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        CaddieItemTableEntry* entry = list->caddieItems.data;
        CaddieItemTableEntry* end = entry + list->caddieItems.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindSetItemData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        SetItemTableEntry* entry = list->setItems.data;
        SetItemTableEntry* end = entry + list->setItems.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        SetItemTableEntry* entry = list->setItems.data;
        SetItemTableEntry* end = entry + list->setItems.size;
        while (entry != end) {
            if ((unsigned int)typeId == entry->key)
                break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindCourseData(CItemManager* list, void* typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        CourseTableEntry* entry = list->courses.data;
        CourseTableEntry* end = entry + list->courses.size;
        while (entry != end || (entry = end, false)) {
            if ((unsigned int)typeId == entry->key) break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        CourseTableEntry* entry = list->courses.data;
        CourseTableEntry* end = entry + list->courses.size;
        while (entry != end || (entry = end, false)) {
            if ((unsigned int)typeId == entry->key) break;
            ++entry;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindHairStyleData(CItemManager* list, unsigned int typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        HairStyleTableEntry* entry = list->hairStyles.data;
        HairStyleTableEntry* end = entry + list->hairStyles.size;
        while (entry != end || (entry = end, false)) {
            if (typeId != entry->key) {
                ++entry;
                continue;
            }
            break;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        HairStyleTableEntry* entry = list->hairStyles.data;
        HairStyleTableEntry* end = entry + list->hairStyles.size;
        while (entry != end || (entry = end, false)) {
            if (typeId != entry->key) {
                ++entry;
                continue;
            }
            break;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Look up an IFF record by type id; return null when absent, locking only when enabled.
void* ItemInventory_FindAuxPartData(CItemManager* list, unsigned int typeId) {
    void* result = 0;
    if (list->lockEnabled) {
        ItemInventory_LockMutex(list->inventoryMutexStorage);
        AuxPartTableEntry* entry = list->auxParts.data;
        AuxPartTableEntry* end = entry + list->auxParts.size;
        while (entry != end || (entry = end, false)) {
            if (typeId != entry->key) {
                ++entry;
                continue;
            }
            break;
        }
        if (entry != end)
            result = &entry->record;
        ItemInventory_UnlockMutex(list->inventoryMutexStorage);
    } else {
        AuxPartTableEntry* entry = list->auxParts.data;
        AuxPartTableEntry* end = entry + list->auxParts.size;
        while (entry != end || (entry = end, false)) {
            if (typeId != entry->key) {
                ++entry;
                continue;
            }
            break;
        }
        if (entry != end)
            result = &entry->record;
    }
    return result;
}

// Reset the auxiliary state, release the owned allocation, and optionally delete this wrapper.
void* dtor_80190230(OwnedObject* self, int deleteFlag) {
    if (self != 0) {
        self->resetState = 0;
        if (self != 0 && self->object != 0) {
            self->resetState = 0;
            appFreeUni__FPv(self->object);
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Reset the auxiliary state, release the owned allocation, and optionally delete this wrapper.
void* dtor_801902A4(OwnedObject* self, int deleteFlag) {
    if (self != 0) {
        self->resetState = 0;
        if (self != 0 && self->object != 0) {
            self->resetState = 0;
            appFreeUni__FPv(self->object);
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Reset the auxiliary state, release the owned allocation, and optionally delete this wrapper.
void* dtor_80190318(OwnedObject* self, int deleteFlag) {
    if (self != 0) {
        self->resetState = 0;
        if (self != 0 && self->object != 0) {
            self->resetState = 0;
            appFreeUni__FPv(self->object);
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Return the borrowed description-record address through the int ABI, or zero.
// The record begins with sDesc::name: borrowed NUL-terminated message-key text.
// Callers consume that text, rather than treating the return as a numeric key.
int ItemInventory_FindDescriptionMessageKeyText(CItemManager* list, void* typeId) {
    DescTableEntry* entry = (DescTableEntry*)list->descriptions.data;
    DescTableEntry* end = entry + list->descriptions.size;
    while (entry != end || (entry = end, false)) {
        if ((unsigned int)typeId == entry->key) break;
        entry++;
    }
    if (entry == end)
        return 0;
    return (int)((char*)&entry->record);
}

// Dispatch by item kind; a missing hairstyle deliberately falls through to the auxiliary-part table.
void* ItemInventory_FindDataWithHairStyleFallback(void* self, unsigned int type) {
    switch (type >> kItemKindShift) {
        case kItemCharacter: return ItemInventory_FindCharacterData((CItemManager*)self, (void*)type);
        case kItemPart: return ItemInventory_FindPartData((CItemManager*)self, (void*)type);
        case kItemClub: return ItemInventory_FindClubData((CItemManager*)self, (void*)type);
        case kItemClubSet: return ItemInventory_FindClubSetData((CItemManager*)self, (void*)type);
        case kItemBall: return ItemInventory_FindBallData((CItemManager*)self, (void*)type);
        case kItemConsumable: return ItemInventory_FindItemData((CItemManager*)self, (void*)type);
        case kItemCaddie: return ItemInventory_FindCaddieData((CItemManager*)self, (void*)type);
        case kItemCaddieItem: return ItemInventory_FindCaddieItemData((CItemManager*)self, (void*)type);
        case kItemSet: return ItemInventory_FindSetItemData((CItemManager*)self, (void*)type);
        case kItemCourse: return ItemInventory_FindCourseData((CItemManager*)self, (void*)type);
        case kItemHairStyle: {
            void* result = ItemInventory_FindHairStyleData((CItemManager*)self, type);
            if (result) return result;
        }
        case kItemAuxPart: return ItemInventory_FindAuxPartData((CItemManager*)self, type);
        default: return 0;
    }
}

#pragma use_lmw_stmw on
// Return the first matching entry in [entry, end), or end; no locking here.
static inline CharacterTableEntry* FindDefaultCharacterEntry(CharacterTableEntry* entry, CharacterTableEntry* end, unsigned int typeId) {
    for (; entry != end; ++entry) {
        if (typeId == (unsigned int)entry->key) return entry;
    }
    return end;
}

// out must hold 24 IDs; defaultPartCount is trusted to fit. Missing parts become 0.
// Returns 1 for a found character, 0 for null out or a missing character (out untouched).
// Build the character's default body-part ids, validate each record, and zero the remaining outfit slots.
int ItemInventory_GetDefaultPartIds(CItemManager* list, unsigned int typeId, unsigned int* out) {
    if (out != 0) {
        IFF_STRUCT::sChar* found = 0;
        if (list->lockEnabled) {
            ItemInventory_LockMutex(list->inventoryMutexStorage);
            CharacterTableEntry* entry = (CharacterTableEntry*)list->characters.data;
            CharacterTableEntry* end = entry + list->characters.size;
            entry = FindDefaultCharacterEntry(entry, end, typeId);
            if (entry != end) found = &entry->record;
            ItemInventory_UnlockMutex(list->inventoryMutexStorage);
        } else {
            CharacterTableEntry* entry = (CharacterTableEntry*)list->characters.data;
            CharacterTableEntry* end = entry + list->characters.size;
            entry = FindDefaultCharacterEntry(entry, end, typeId);
            if (entry != end) found = &entry->record;
        }
        if (found != 0) {
            int count = found->defaultPartCount;
            unsigned int* dest = out;
            int i = 0;
            // Synthesize one default ID per body slot, then validate each ID.
            unsigned int base = (typeId << kPartCharacterShift) | kPartTypeIdBase;
            unsigned int bits = 0;
            for (; i < count; ++i) {
                unsigned int value = base | bits | kDefaultPartFlag;
                IFF_STRUCT::sPart* second = 0;
                if (list->lockEnabled) {
                    ItemInventory_LockMutex(list->inventoryMutexStorage);
                    PartTableEntry* entry = (PartTableEntry*)list->parts.data;
                    PartTableEntry* end = entry + list->parts.size;
                    entry = FindPartEntryByOpaqueId(entry, end, (void*)value);
                    if (entry != end) second = &entry->record;
                    ItemInventory_UnlockMutex(list->inventoryMutexStorage);
                } else {
                    PartTableEntry* entry = (PartTableEntry*)list->parts.data;
                    PartTableEntry* end = entry + list->parts.size;
                    entry = FindPartEntryByOpaqueId(entry, end, (void*)value);
                    if (entry != end) second = &entry->record;
                }
                if (second != 0) *dest = value;
                else *dest = 0;
                bits += kPartSlotStride;
                ++dest;
            }
            // Clear the unused tail only after all default body slots are written.
            for (; i < kOutfitSlotCount; ++i) out[i] = 0;
            return 1;
        }
    }
    return 0;
}
#pragma use_lmw_stmw reset

// Return the item's Pang price, or -1 when the type id has no record (including hairstyle fallback).
int ItemInventory_GetPricePang(void* self, unsigned int type) {
    IffItemCommon* record = (IffItemCommon*)ItemInventory_FindDataWithHairStyleFallback(self, type);
    if (record != 0)
        return record->price;
    return -1;
}

// Dispatch the type id to its IFF table and return the matching record, or null.
void* ItemInventory_FindDataById(void* self, unsigned int type) {
    switch (type >> kItemKindShift) {
        case kItemCharacter: return ItemInventory_FindCharacterData((CItemManager*)self, (void*)type);
        case kItemPart: return ItemInventory_FindPartData((CItemManager*)self, (void*)type);
        case kItemClub: return ItemInventory_FindClubData((CItemManager*)self, (void*)type);
        case kItemClubSet: return ItemInventory_FindClubSetData((CItemManager*)self, (void*)type);
        case kItemBall: return ItemInventory_FindBallData((CItemManager*)self, (void*)type);
        case kItemConsumable: return ItemInventory_FindItemData((CItemManager*)self, (void*)type);
        case kItemCaddie: return ItemInventory_FindCaddieData((CItemManager*)self, (void*)type);
        case kItemCaddieItem: return ItemInventory_FindCaddieItemData((CItemManager*)self, (void*)type);
        case kItemSet: return ItemInventory_FindSetItemData((CItemManager*)self, (void*)type);
        case kItemCourse: return ItemInventory_FindCourseData((CItemManager*)self, (void*)type);
        case kItemHairStyle: return ItemInventory_FindHairStyleData((CItemManager*)self, type);
        case kItemAuxPart: return ItemInventory_FindAuxPartData((CItemManager*)self, type);
        default: return 0;
    }
}

// Return the course-table size minus three reserved entries, with optional locking.
int ItemInventory_GetArchiveCourseCount(CItemManager* inventory) {
    if (inventory->lockEnabled != 0) {
        ItemInventory_LockMutex(inventory->inventoryMutexStorage);
        int result = (int)inventory->courses.size - kBuiltInCourseCount;
        ItemInventory_UnlockMutex(inventory->inventoryMutexStorage);
        return result;
    }
    return (int)inventory->courses.size - kBuiltInCourseCount;
}

// Return the first matching entry in [entry, end), or end; no locking here.
static inline CharacterTableEntry* FindCharacterEntryById(CharacterTableEntry* entry, CharacterTableEntry* end, unsigned int typeId) {
    for (; entry != end; ++entry) {
        if (typeId == entry->key)
            return entry;
    }
    return end;
}

// Return the first matching entry in [entry, end), or end; no locking here.
static inline PartTableEntry* FindPartEntryById(PartTableEntry* entry, PartTableEntry* end, unsigned int typeId) {
    for (; entry != end; ++entry) {
        if (typeId == entry->key)
            return entry;
    }
    return end;
}

// Return the first matching entry in [entry, end), or end; no locking here.
static inline AuxPartTableEntry* FindAuxPartEntryById(AuxPartTableEntry* entry, AuxPartTableEntry* end, unsigned int typeId) {
    for (; entry != end; ++entry) {
        if (typeId == entry->key)
            return entry;
    }
    return end;
}

// Return the first matching entry in [entry, end), or end; no locking here.
static inline CaddieTableEntry* FindCaddieEntryById(CaddieTableEntry* entry, CaddieTableEntry* end, unsigned int typeId) {
    for (; entry != end; ++entry) {
        if (typeId == entry->key)
            return entry;
    }
    return end;
}

#pragma use_lmw_stmw on
// statIndex must index the record's five stat lanes; no general range check exists.
// Optional arrays: outfitPartIds[24], auxPartIds[5], trainingBonuses[5].
// Byte-sized accumulators wrap after each addition; the result is not saturated.
// Auxiliary value 99 contributes nothing; caddie eligibility compares its level to playerRank.
// Sum base, outfit, ring, training and rank-eligible caddie bonuses for one stat; byte truncation is intentional.
unsigned char ItemInventory_SumAttributeContributions(CItemManager* self, int statIndex, unsigned int typeId,
                          unsigned int* outfitPartIds, unsigned int* auxPartIds,
short* trainingBonuses, unsigned int caddieOrItemId, unsigned int playerRank) {
    IFF_STRUCT::sPart* partRecord;
    IFF_STRUCT::sAuxPart* auxPartRecord;
    unsigned int equippedTypeId;
    unsigned char equipmentStat;
    IFF_STRUCT::sChar* characterRecord = 0;
    if (self->lockEnabled) {
        ItemInventory_LockMutex(self->inventoryMutexStorage);
        CharacterTableEntry* entry = self->characters.data;
        CharacterTableEntry* end = entry + self->characters.size;
        entry = FindCharacterEntryById(entry, end, typeId);
        if (entry != end)
            characterRecord = &entry->record;
        ItemInventory_UnlockMutex(self->inventoryMutexStorage);
    } else {
        CharacterTableEntry* entry = self->characters.data;
        CharacterTableEntry* end = entry + self->characters.size;
        entry = FindCharacterEntryById(entry, end, typeId);
        if (entry != end)
            characterRecord = &entry->record;
    }
    if (characterRecord != 0)
        equipmentStat = characterRecord->baseStats[statIndex];
    else
        equipmentStat = 0;

    // Phase 1: add each found outfit record to the character base stat.
    if (outfitPartIds != 0) {
        IFF_STRUCT::sPart* outfitRecords[kOutfitSlotCount];
        int i;
        for (i = 0; i < kOutfitSlotCount; i++) {
            equippedTypeId = outfitPartIds[i];
            if (equippedTypeId == 0) {
                outfitRecords[i] = 0;
            } else {
                partRecord = 0;
                if (self->lockEnabled) {
                    ItemInventory_LockMutex(self->inventoryMutexStorage);
                    PartTableEntry* entry = self->parts.data;
                    PartTableEntry* end = entry + self->parts.size;
                    entry = FindPartEntryById(entry, end, equippedTypeId);
                    if (entry != end)
                        partRecord = (IFF_STRUCT::sPart*)((char*)&entry->record);
                    ItemInventory_UnlockMutex(self->inventoryMutexStorage);
                } else {
                    PartTableEntry* entry = self->parts.data;
                    PartTableEntry* end = entry + self->parts.size;
                    entry = FindPartEntryById(entry, end, equippedTypeId);
                    if (entry != end)
                        partRecord = (IFF_STRUCT::sPart*)((char*)&entry->record);
                }
                outfitRecords[i] = partRecord;
            }
            if (outfitRecords[i] != 0)
                equipmentStat = equipmentStat + (short)outfitRecords[i]->stats[statIndex];
        }
    }

    // Phase 2: add auxiliary-part bonuses, skipping the no-bonus marker.
    if (auxPartIds != 0 && statIndex < kStatCount) {
        IFF_STRUCT::sAuxPart* auxPartRecords[kStatCount];
        int i;
        for (i = 0; i < kStatCount; i++) {
            equippedTypeId = auxPartIds[i];
            if (equippedTypeId == 0) {
                auxPartRecords[i] = 0;
            } else {
                auxPartRecord = 0;
                if (self->lockEnabled) {
                    ItemInventory_LockMutex(self->inventoryMutexStorage);
                    AuxPartTableEntry* entry = self->auxParts.data;
                    AuxPartTableEntry* end = entry + self->auxParts.size;
                    entry = FindAuxPartEntryById(entry, end, equippedTypeId);
                    if (entry != end)
                        auxPartRecord = (IFF_STRUCT::sAuxPart*)((char*)&entry->record);
                    ItemInventory_UnlockMutex(self->inventoryMutexStorage);
                } else {
                    AuxPartTableEntry* entry = self->auxParts.data;
                    AuxPartTableEntry* end = entry + self->auxParts.size;
                    entry = FindAuxPartEntryById(entry, end, equippedTypeId);
                    if (entry != end)
                        auxPartRecord = (IFF_STRUCT::sAuxPart*)((char*)&entry->record);
                }
                auxPartRecords[i] = auxPartRecord;
            }
            if (auxPartRecords[i] != 0) {
                int auxBonus = auxPartRecords[i]->stats[statIndex];
                if (auxBonus != kAuxPartNoStatBonus)
                    equipmentStat = equipmentStat + auxBonus;
            }
        }
    }

    // Phase 3: truncate training separately and resolve the caddie or its owner ID.
    unsigned char trainingBonus = 0;
    if (trainingBonuses != 0)
        trainingBonus = trainingBonuses[statIndex];

    IFF_STRUCT::sCaddie* caddieRecord;
    if ((caddieOrItemId >> kItemKindShift) == kItemCaddieItem) {
        caddieRecord = 0;
        if (self->lockEnabled) {
            ItemInventory_LockMutex(self->inventoryMutexStorage);
            CaddieTableEntry* entry = self->caddies.data;
            CaddieTableEntry* end = entry + self->caddies.size;
            entry = FindCaddieEntryById(entry, end, ((caddieOrItemId >> kCaddieItemOwnerShift) & kCaddieItemOwnerMask) | kCaddieTypeIdBase);
            if (entry != end)
                caddieRecord = (IFF_STRUCT::sCaddie*)((char*)&entry->record);
            ItemInventory_UnlockMutex(self->inventoryMutexStorage);
        } else {
            CaddieTableEntry* entry = self->caddies.data;
            CaddieTableEntry* end = entry + self->caddies.size;
            entry = FindCaddieEntryById(entry, end, ((caddieOrItemId >> kCaddieItemOwnerShift) & kCaddieItemOwnerMask) | kCaddieTypeIdBase);
            if (entry != end)
                caddieRecord = (IFF_STRUCT::sCaddie*)((char*)&entry->record);
        }
    } else {
        caddieRecord = 0;
        if (self->lockEnabled) {
            ItemInventory_LockMutex(self->inventoryMutexStorage);
            CaddieTableEntry* entry = self->caddies.data;
            CaddieTableEntry* end = entry + self->caddies.size;
            entry = FindCaddieEntryById(entry, end, caddieOrItemId);
            if (entry != end)
                caddieRecord = (IFF_STRUCT::sCaddie*)((char*)&entry->record);
            ItemInventory_UnlockMutex(self->inventoryMutexStorage);
        } else {
            CaddieTableEntry* entry = self->caddies.data;
            CaddieTableEntry* end = entry + self->caddies.size;
            entry = FindCaddieEntryById(entry, end, caddieOrItemId);
            if (entry != end)
                caddieRecord = (IFF_STRUCT::sCaddie*)((char*)&entry->record);
        }
    }

    // Phase 4: apply the rank gate, then combine the byte-sized contributions.
    unsigned char caddieBonus;
    if (caddieRecord != 0 && ((IffLevelBits*)&caddieRecord->common.levelFlags)->level <= playerRank)
        caddieBonus = (short)caddieRecord->stats[statIndex];
    else
        caddieBonus = 0;
    int total = caddieBonus;
    total += equipmentStat;
    return trainingBonus + total;
}
#pragma use_lmw_stmw reset
} // extern "C"
