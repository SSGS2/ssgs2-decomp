#include <decomp/ct_member.h>
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
// Inventory lookup receivers below are the shared document's inline subobject at +0xA04.
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char shopStrings[];
}
}

// Translation unit 0x802CAA54-0x802CBF2C (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

// Byte layout of the shop data used here. Each character's selected outfit is a
// 0x34-byte row: the 24 signed part indices start at row +2, and the signed
// auxiliary index is at row +0x33. -1 means no selection, not inventory index 0.
enum ShopOutfitLayout {
    kSelectedCharacterOffset = 0x44,
    kPartSelectionBitsOffset = 0x54,
    kOutfitRowStride = 0x34,
    kFirstPartIndexOffset = 0x2e6,
    kAuxPartIndexOffset = 0x317,
    kShopCategoryOffset = 0x840,
    kProductsCategoryOffset = 0x874,
    kPartSlotsPerOutfit = 24,
    kPartSelectionBitCount = 0x30e,
    kNoInventoryIndex = -1,
    kPartTypeOffset = 0x198
};
// Category marker map (layout element ID = marker +0x10):
// 0 -> cleared and never enabled here; 1 -> partType 0; 2 -> partType 1;
// 3 -> partType 4; 4 -> every other partType. No highlight elements are touched.
enum OutfitCategoryMarker {
    kPartType0Marker = 1, kPartType1Marker = 2,
    kPartType4Marker = 3, kOtherPartTypeMarker = 4, kCategoryMarkerCount = 5
};
static const u32 kMaximumVectorElements = 0x3fffffffU; // Four bytes per item ID.
static const u32 kThreeFifthsGrowthLimit = 0x15555555U;
static const u32 kHalfGrowthLimit = 0x2aaaaaaaU;
enum ShopExceptionStringOffset {
    kOutOfRangeTypeNameOffset = 0xf4,
    kVectorLengthMessageOffset = 0x139,
    kLengthErrorTypeNameOffset = 0x14d
};

extern void* __nwa__FUl(u32);
extern u32 ItemInventory_GetPartIdAt(void*, int);
extern u32 ItemInventory_GetAuxPartIdAt(void*, int);
extern int CPangya2Shop_IsItemOwned(void*, u32);
extern void* appMallocDummy__FUl(u32);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(u32);
extern void* __dla__FPv(void*);
extern unsigned strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* memcpy(void*, const void*, unsigned);
extern void* memset(void*, int, unsigned);
extern void CProductsCategory_SetMarkerVisible(void*, int, int);
extern void* ItemInventory_FindPartData(void*, u32);
extern void* g_sharedDoc;
extern char bitsetTestOutOfRangeMessage[];

struct VectorU32;
int CPangya2Shop_CollectUnownedSelectedOutfitItems(void* self, VectorU32* out);
int CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(void* self);

extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std12length_error[];
extern char shopStrings[];
extern "C" void __dt__Q23std9exceptionFv(void*);
extern void __dt__Q23std12out_of_rangeFv();
extern void __dt__Q23std12length_errorFv();
extern void __throw(const char*, void*, void*) __attribute__((noreturn));

struct Ref {
    void* vtable;
    int count;
    int controlWord08; // Control-block counter at +8.
    char* data;
    // Create the shared exception-message control block and take ownership of p.
    Ref(char* p) { count = 1; controlWord08 = 1; vtable = s_sharedCharDeleterVtable; data = p; }
};

struct BufferOwner {
    char* data;
    char* allocator;
    // Release an untransferred char[] if control-block construction failed.
    ~BufferOwner() { if (data) __dla__FPv(data); }
};

struct ErrorBase_u802CAA54 {
    void* vtable;
    char* data;
    Ref* ref;
    // Establish the logic_error base before attaching an owned message.
    ErrorBase_u802CAA54() : vtable(__vt__Q23std11logic_error) {}
    // Run the exception base cleanup during destruction or unwinding.
    ~ErrorBase_u802CAA54() { __dt__Q23std9exceptionFv(this); }
};

} // extern "C"

namespace std {
struct out_of_range : ErrorBase_u802CAA54 {
    ~out_of_range();
    // Copy a borrowed message into shared char[] storage for an index error.
    out_of_range(const char* message) {
        char* buffer;
        unsigned int messageBytes;
        const char* text;
        text = message;
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        messageBytes = strlen(text) + 1;
        buffer = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        data = buffer;
        {
            BufferOwner owner;
            owner.data = buffer;
            owner.allocator = &allocator.value;
            ref = new Ref(buffer);
            owner.data = 0;
        }
        strcpy(data, text);
        vtable = __vt__Q23std12out_of_range;
    }
};
struct length_error : ErrorBase_u802CAA54 {
    ~length_error();
    // Copy a borrowed message into shared char[] storage for a size error.
    length_error(const char* message) {
        char* buffer;
        unsigned int messageBytes;
        const char* text;
        text = message;
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        messageBytes = strlen(text) + 1;
        buffer = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        data = buffer;
        {
            BufferOwner owner;
            owner.data = buffer;
            owner.allocator = &allocator.value;
            ref = new Ref(buffer);
            owner.data = 0;
        }
        strcpy(data, text);
        vtable = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}

// Throw an index exception with its own copy of message.
inline void throwOutOfRange(const char* message) {
    throw std::out_of_range(message);
}

// Construct and throw the length exception using this unit's runtime type string.
inline void throwLengthError(const char* message) {
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(message);
        exception_runtime::__throw(exception_runtime::shopStrings + kLengthErrorTypeNameOffset, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

// Borrow the larger operand; callers must keep both operands alive through use.
template <typename T>
inline const T& maxRef(const T& a, const T& b) { return a < b ? b : a; }

// Temporary owner for push_back growth. data owns appMallocDummy storage until
// commit swaps in the old allocation; its destructor then releases that old block.
// position is the prefix destination cursor, in elements. copy() also accounts
// for the already-written appended item and zeros the moved source words.
struct Grow {
    u32* data;
    u32 size;
    u32 capacity;
    u32* allocator;
    u32 position;
    // Begin an empty temporary using a borrowed allocator token (not dereferenced here).
    Grow(u32* a) : data(0), size(0), capacity(0), allocator(a), position(0) {}
    // Free whichever allocation the temporary currently owns.
    ~Grow() { if (data) { size = 0; appFreeUni__FPv(data); } }
    // Move [first,last) into the prefix and count it plus the pending appended item.
    void copy(u32* first, u32* last) {
        u32 count = last - first;
        ++size;
        position -= count;
        memcpy(data + position, first, count * 4);
        memset(first, 0, count * 4);
        size += count;
    }
};

struct VectorU32 {
    // Owns an appMallocDummy block of capacity u32s; size counts initialized IDs.
    // No item payload is owned. Destruction frees only the backing allocation.
    u32* data;
    u32 size;
    u32 capacity;
    // Release the item-ID array, without touching inventory records.
    ~VectorU32() { if (data) { size = 0; appFreeUni__FPv(data); } }
    // Append an ID, reallocating if needed; overflow throws std::length_error.
    void push_back(u32 itemId) {
        u32 currentCapacity = capacity;
        u32 currentSize = size;
        if (currentSize < currentCapacity) {
            ++size;
            data[size - 1] = itemId;
        } else {
            // Choose a growth step while keeping four-byte allocation sizes representable.
            if (kMaximumVectorElements - capacity < 1) {
                throwLengthError(shopStrings + kVectorLengthMessageOffset);
            }
            u32* allocator = &capacity;
            u32 extra = size + 1 - capacity;
            Grow grown(allocator);
            u32 cap = capacity;
            if (extra > kMaximumVectorElements - cap) {
                throwLengthError(shopStrings + kVectorLengthMessageOffset);
            }
            u32 halfStep;
            u32 step;
            if (cap < kThreeFifthsGrowthLimit) {
                step = ((cap + 1) * 3) / 5;
                cap += maxRef(step, extra);
            } else if (cap < kHalfGrowthLimit) {
                halfStep = (cap + 1) / 2;
                cap += maxRef(halfStep, extra);
            } else cap = kMaximumVectorElements;
            if (cap > kMaximumVectorElements) {
                throwLengthError(shopStrings + kVectorLengthMessageOffset);
            }
            // Stage the new tail item, then move the old prefix into the new allocation.
            grown.data = (u32*)appMallocDummy__FUl(cap * 4);
            grown.capacity = cap;
            u32 offset = size;
            grown.position = offset;
            grown.data[offset + grown.size] = itemId;
            grown.copy(data, data + size);
            u32 newSize;
            u32 oldCapacity;
            u32* replacement;
            u32* previous;
            // Commit ownership by swapping allocations; grown now frees the old block.
            previous = data;
            replacement = grown.data;
            newSize = grown.size;
            oldCapacity = capacity;
            size = 0;
            grown.data = previous;
            capacity = grown.capacity;
            grown.capacity = oldCapacity;
            data = replacement;
            size = newSize;
            grown.size = 0;
        }
    }
};

struct BitRef {
    u32* words;
    u32 index;
};

extern "C" {

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Rebuild category markers for selected, enabled, unowned outfit items.
// Clear all markers first; return 0 outside shop category 0, otherwise return 1
// even if no items were found. Inventory and part records are borrowed and must exist.
int CPangya2Shop_UpdateUnownedSelectedOutfitCategoryMarkers(void* self) {
    u8* shopBytes = (u8*)self;
    // Reset the marker set even when the current shop category is unsupported.
    for (int i = 0; i < kCategoryMarkerCount; ++i)
        CProductsCategory_SetMarkerVisible(*(void**)(shopBytes + kProductsCategoryOffset), i, 0);
    if (*(u32*)(shopBytes + kShopCategoryOffset)) return 0;

    VectorU32 found = { 0, 0, 0 };
    int characterIndex = (signed char)shopBytes[kSelectedCharacterOffset];
    int outfitOffset = characterIndex * kOutfitRowStride;
    u8* outfitBytes = shopBytes + outfitOffset;
    // Collect enabled part selections; an invalid bit index throws before ownership lookup.
    for (int i = 0; i < kPartSlotsPerOutfit; ++i) {
        int inventoryIndex = *(short*)(outfitBytes + kFirstPartIndexOffset + i * 2);
        if (inventoryIndex != kNoInventoryIndex) {
            BitRef selection = { (u32*)(shopBytes + kPartSelectionBitsOffset), (u32)inventoryIndex };
            BitRef probe = selection;
            if ((u32)inventoryIndex >= kPartSelectionBitCount) {
                std::out_of_range error(bitsetTestOutOfRangeMessage);
                __throw(shopStrings + kOutOfRangeTypeNameOffset, &error, (void*)__dt__Q23std12out_of_rangeFv);
            }
            int includeUnownedPart = 0;
            // Bitset words hold 32 selections each, with bit 0 at the low end.
            if (probe.words[probe.index >> 5] & (1u << (probe.index & 31)))
                if (!CPangya2Shop_IsItemOwned(self, ItemInventory_GetPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex)))
                    includeUnownedPart = 1;
            if (includeUnownedPart) {
                u32 itemId = ItemInventory_GetPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex);
                found.push_back(itemId);
            }
        }
    }
    // Auxiliary selection has no part-bitset gate; append it after the normal parts.
    int inventoryIndex = (signed char)shopBytes[outfitOffset + kAuxPartIndexOffset];
    if (inventoryIndex != kNoInventoryIndex && !CPangya2Shop_IsItemOwned(self, ItemInventory_GetAuxPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex))) {
        u32 itemId = ItemInventory_GetAuxPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex);
        found.push_back(itemId);
    }
    // Flatten part types to marker indices. Multiple items may enable the same marker.
    for (u32 i = 0; i < found.size; ++i) {
        u32 item = found.data[i];
        u8 partType = *((u8*)ItemInventory_FindPartData(((CSharedDoc*)g_sharedDoc)->itemInventory, item) + kPartTypeOffset);
        switch (partType) {
        case 0: CProductsCategory_SetMarkerVisible(*(void**)(shopBytes + kProductsCategoryOffset), kPartType0Marker, 1); break;
        case 1: CProductsCategory_SetMarkerVisible(*(void**)(shopBytes + kProductsCategoryOffset), kPartType1Marker, 1); break;
        case 4: CProductsCategory_SetMarkerVisible(*(void**)(shopBytes + kProductsCategoryOffset), kPartType4Marker, 1); break;
        default: CProductsCategory_SetMarkerVisible(*(void**)(shopBytes + kProductsCategoryOffset), kOtherPartTypeMarker, 1); break;
        }
    }
    return 1;
}
#pragma opt_propagation reset
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Append selected, enabled, unowned outfit IDs to caller-owned out; do not clear it.
// Return whether the final vector is nonempty, including any preexisting entries.
// self/out and the selected character row must be valid; invalid bit indices throw.
int CPangya2Shop_CollectUnownedSelectedOutfitItems(void* self, VectorU32* out) {
    u8* shopBytes = (u8*)self;
    int outfitOffset = (signed char)shopBytes[kSelectedCharacterOffset] * kOutfitRowStride;
    u8* partCursor = shopBytes + outfitOffset;
    // Read one signed short per slot; the cursor advances while the row offset stays fixed.
    for (int i = 0; i < kPartSlotsPerOutfit; ++i) {
        int inventoryIndex = *(short*)(partCursor + kFirstPartIndexOffset);
        if (inventoryIndex != kNoInventoryIndex) {
            BitRef ref = { (u32*)(shopBytes + kPartSelectionBitsOffset), (u32)inventoryIndex };
            BitRef probe = ref;
            int includeUnownedPart = 0;
            if ((u32)inventoryIndex >= kPartSelectionBitCount) {
                std::out_of_range error(bitsetTestOutOfRangeMessage);
                __throw(shopStrings + kOutOfRangeTypeNameOffset, &error, (void*)__dt__Q23std12out_of_rangeFv);
            }
            if (probe.words[probe.index >> 5] & (1u << (probe.index & 31))) {
                includeUnownedPart = (!CPangya2Shop_IsItemOwned(self, ItemInventory_GetPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex))) ? 1 : includeUnownedPart;
            }
            if (includeUnownedPart) {
                u32 itemId = ItemInventory_GetPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex);
                out->push_back(itemId);
            }
        }
        partCursor += 2;
    }
    // Append the auxiliary selection last; it bypasses the normal part-enable bitset.
    int inventoryIndex = (signed char)shopBytes[outfitOffset + kAuxPartIndexOffset];
    if (inventoryIndex != kNoInventoryIndex && !CPangya2Shop_IsItemOwned(self, ItemInventory_GetAuxPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex))) {
        u32 itemId = ItemInventory_GetAuxPartIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, inventoryIndex);
        out->push_back(itemId);
    }
    return out->size != 0;
}
#pragma opt_propagation reset
#pragma pop

} // extern "C"
