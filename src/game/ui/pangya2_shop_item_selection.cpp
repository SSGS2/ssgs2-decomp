#include "classes/CSharedDoc.h"
// Borrow the shared-document layout without adjusting or owning its address.
static inline CSharedDoc* AsCSharedDoc(void* p) { return (CSharedDoc*)p; }
#include "classes/CPangya2Shop.h"
#include "classes/CShopProductsTab.h"
#include "iff/IffStruct.h"
// Borrow the shop layout without changing the underlying object or its lifetime.
static inline CPangya2Shop* AsCPangya2Shop(void* p) { return (CPangya2Shop*)p; }

// Translation unit 0x802C9D94-0x802CAA54 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"

// Construct an exception in caller-provided storage; no allocation or ownership transfer.
inline void* operator new(size_t, void* p) { return p; }

typedef CPangya2Shop Obj;

// Category selects the product builder, not a UI element ID. Input action 0x14
// opens item details; -1 lets InputManager choose the input device.
enum { kShopCharacters = 0, kShopItems = 1, kShopCaddies = 2,
       kShowItemInfoAction = 0x14, kAnyInputDevice = -1, kNoSelection = -1,
       kEquipmentSlotCount = 24, kPartIndexCount = 0x30e,
       kPartBitWordCount = 25, kPartBitsPerWord = 32, kPartBitWordShift = 5,
       kClzZeroToBooleanShift = 5,
       kPointerBytes = 4, kMaximumScanItems = 0x3fffffffU,
       kScanGrowthThreeFifthsLimit = 0x15555555U, kScanGrowthHalfLimit = 0x2aaaaaaaU };

// Layout view for fields hidden in the shared shop header padding.
// +0x54: bit i gates scanning equipment part index i (782 logical bits in 25 words).
// +0x2e4: per-character selections, 0x34-byte stride. -1 means no selection.
// Each ordinary index is signed 16-bit; the auxiliary index is signed 8-bit.
// The auxiliary selection has an ownership check but no eligibility-bit check here.
struct ShopPartSelection {
    short reserved;
    short partIndices[kEquipmentSlotCount];
    char reservedTail;
    signed char auxiliaryIndex;
};
struct ShopPartScanView {
    char prefix[0x54];
    unsigned int eligibleParts[kPartBitWordCount];
    char beforeSelections[0x2e4 - 0xb8];
    ShopPartSelection selections[1]; // Indexed by the current character; stride is 0x34.
};
typedef char SelectionStrideCheck[sizeof(ShopPartSelection) == 0x34 ? 1 : -1];

// Stack payload shared by GetSelectedProductInfo and SetDisplayedData:
// +0 price in Pang; +4/+6/+8 signed Power/Control/Spin, then four reserved bytes;
// +0x0e and +0x36 contain 40-byte message IDs (not necessarily NUL-terminated).
// Keep the three clear spans: the ten-byte stat region is NOT ten stat entries.
struct ItemInfoBuffer {
    int pangPrice;       // +0x00: price in Pang.
    char statBonusesAndReserved[0xa];   // +0x04: three short stats and four reserved bytes.
    char nameMessageId[0x28];  // +0x0e: localized name message ID.
    char descriptionMessageId[0x28];  // +0x36: localized description message ID.
};

extern "C" {
extern void* memcpy(void*, const void*, size_t);
extern void* memset(void*, int, size_t);

extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_80516C10[];

extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern void* __register_global_object(void*, void*, void*);
extern int Input_IsActionPressed(void*, int, int);

extern int CShopProductsTab_GetSelectedProductIndex(void*);
extern int ShopProductTable_GetRecordAt(void*, int, int);
extern int CBuildProduct_GetTextureEntryAt(void*, int, int);
extern void CItemInfo_Show(void*);
extern void CItemInfo_SetDisplayedData(void*, int*);
extern int CShopProductsTab_IsSelectedProductOwned(void*);
extern void CItemInfo_SetNotBuyGroupEnabled(void*, int);
extern void CItemInfo_SetItemTexture(void*, int);
extern void CPangya2Shop_UpdateControlScriptSelection(Obj*);
extern int CPangya2Shop_GetSelectedProductInfo(Obj*, int*);
extern void* g_sharedDoc;
extern void* ItemInventory_GetPartIdAt(void*, int);
extern void* ItemInventory_GetAuxPartIdAt(void*, int);
extern int CPangya2Shop_IsItemOwned(void*, void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned long);
extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void __dt__Q23std12out_of_rangeFv();
extern void __dt__Q23std12length_errorFv();
struct ShopExceptionStrings {
    char beforeOutOfRangeType[0xf4];
    char outOfRangeType[55];
    char beforeLengthMessage[14];
    char vectorLengthError[20];
    char lengthErrorType[55];
};
extern ShopExceptionStrings shopStrings;
extern char bitsetTestOutOfRangeMessage[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std12length_error[];

struct ScanRef {
    void* vtable;
    int strong;
    int weak;
    char* data;
};

// Exception text guard owns only its new[] buffer until the reference block takes it.
struct ScanBuffer {
    char* data;
    char* allocator;
    // Release text on construction failure; a cleared data pointer disarms cleanup.
    ~ScanBuffer() { if (data) __dla__FPv(data); }
};

struct ScanAllocator { char value; char padding[3]; };

struct ScanErrorBase {
    void* vtable;
    // Install the logic_error base vtable before constructing its shared message.
    ScanErrorBase() { vtable = __vt__Q23std11logic_error; }
    // Destroy the exception base without freeing the enclosing exception storage.
    ~ScanErrorBase() { __dt__Q23std9exceptionFv(this, 0); }
};

struct ScanError : ScanErrorBase {
    char* data;
    ScanRef* ref;
    // Copy a borrowed NUL-terminated message into reference-counted owned text.
    ScanError(const char* message) {
        ScanAllocator allocator;
        allocator.value = 0;
        unsigned int length = strlen(message) + 1;
        char* text = (char*)__nwa__FUl(length * 2 - length);
        data = text;
        {
            ScanBuffer buffer;
            buffer.data = text;
            buffer.allocator = &allocator.value;
            ScanRef* control = (ScanRef*)__nw__FUl(16);
            if (control) {
                control->strong = 1;
                control->weak = 1;
                control->vtable = s_sharedCharDeleterVtable;
                control->data = text;
            }
            ref = control;
            buffer.data = 0;
        }
        strcpy(data, message);
    }
};

// Build and throw a 12-byte error object with the supplied derived RTTI/destructor.
// Caller-scope construction preserves CodeWarrior exception cleanup boundaries.
#define SCAN_THROW(message, table, type, destructor) \
    { \
        struct { unsigned int words[3]; } storage; \
        ScanError* error = new (&storage) ScanError(message); \
        error->vtable = table; \
        __throw(type, error, (void*)destructor); \
    }

#define SCAN_LENGTH_ERROR() SCAN_THROW(shopStrings.vectorLengthError, __vt__Q23std12length_error, shopStrings.lengthErrorType, __dt__Q23std12length_errorFv)

// Own the array of item-ID words. The void* element type carries numeric type IDs,
// not dereferenceable item objects (GetPartIdAt returns the inventory entry key).
// No sentinel: data[0..size) is live, capacity counts four-byte slots; destruction
// frees only the array and never treats its ID words as allocations.
struct ScanVector {
    void** data;
    unsigned int size;
    unsigned int capacity;
    // Return the number of stored IDs, not bytes or allocated slots.
    unsigned int count() const { return size; }
    // Drop n entries from the logical end; caller guarantees n <= size.
    void erase(unsigned int n) { size -= n; }
    // Start empty without acquiring storage.
    ScanVector() { data = 0; size = 0; capacity = 0; }
    // Forget entries and release only the owned pointer-array allocation.
    ~ScanVector() {
        if (data) {
            erase(count());
            appFreeUni__FPv(data);
        }
    }
};

// During growth, own the replacement array; after the swap, own the previous array.
// IDs are copied as values. start marks the destination prefix still being copied.
struct ScanGrowth {
    void** data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* oldCapacity;
    unsigned int start;
    // Free whichever array the transaction currently owns, including during unwinding.
    ~ScanGrowth() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

// Borrow the larger growth increment (retaining the original reference-return ABI).
static inline const unsigned int& scan_max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Borrow the ordinary-part eligibility words in the shop's embedded bitset.
static inline unsigned int* scan_bits(void* self) {
    return ((ShopPartScanView*)self)->eligibleParts;
}

// Choose capacity in pointer slots: add ~60%, then ~50%, then saturate at the byte limit.
// extra is the additional capacity required; caller checks overflow and maximum size.
static inline unsigned int scan_next(unsigned int capacity, const unsigned int& extra) {
    if (capacity < kScanGrowthThreeFifthsLimit) {
        unsigned int increment = ((capacity + 1) * 3) / 5;
        return capacity + scan_max(increment, extra);
    }
    if (capacity < kScanGrowthHalfLimit) {
        unsigned int increment = (capacity + 1) / 2;
        return capacity + scan_max(increment, extra);
    }
    return kMaximumScanItems;
}

// Kept in caller scope: an inline-function version changes 73 instruction bytes in this compiler.
// Grow into temporary storage, transfer the old items, then swap ownership for exception-safe cleanup.
// value is an item-ID word. indexed selects the existing byte-indexed write expression;
// it does not select a different container or transfer item ownership.
// Phase order on growth: validate -> allocate/append -> copy/clear -> swap -> free old array.
#define SCAN_PUSH(v, value, indexed) \
    if ((v).capacity > (v).size) { \
        if (indexed) { \
            unsigned int size = ++(v).size; \
            *(void**)((unsigned int)(v).data + (size - 1) * kPointerBytes) = value; \
        } else { \
            unsigned int size = ++(v).size; \
            *((v).data + size - 1) = value; \
        } \
    } else { \
        if (kMaximumScanItems - (v).capacity < 1U) { SCAN_LENGTH_ERROR(); } \
        ScanGrowth growth; \
        growth.data = 0; \
        growth.size = 0; \
        growth.capacity = 0; \
        growth.oldCapacity = &(v).capacity; \
        growth.start = 0; \
        unsigned int extra = (v).size + 1 - (v).capacity; \
        unsigned int cap = (v).capacity; \
        if (extra > kMaximumScanItems - cap) { SCAN_LENGTH_ERROR(); } \
        unsigned int next = scan_next(cap, extra); \
        if (next > kMaximumScanItems) { SCAN_LENGTH_ERROR(); } \
        void** data = (void**)appMallocDummy__FUl(next * kPointerBytes); \
        growth.start = (v).size; \
        growth.data = data; \
        *(void**)((unsigned int)(data + (v).size) + growth.size * kPointerBytes) = value; \
        growth.capacity = next; \
        void** old = (v).data; \
        ++growth.size; \
        int count = ((int)((unsigned int)(old + (v).size) - (unsigned int)old)) / kPointerBytes; \
        growth.start -= count; \
        unsigned int bytes = count * kPointerBytes; \
        memcpy(data + growth.start, old, bytes); \
        memset(old, 0, bytes); \
        unsigned int size = growth.size + count; \
        void** previous = (v).data; \
        (v).data = growth.data; \
        unsigned int previousCapacity = (v).capacity; \
        (v).capacity = growth.capacity; \
        growth.capacity = previousCapacity; \
        growth.data = previous; \
        (v).size = size; \
        growth.size = 0; \
    }

#pragma push
#pragma use_lmw_stmw on
// Collect eligible, unowned equipment for the current character and report whether any exists.
// self borrows a live shop with a valid currentCharaIndex. Return 1 if any ordinary
// or auxiliary selection is unowned, otherwise 0. Ordinary indices other than -1
// must be in [0,782) or throw out_of_range; vector growth can throw length_error.
// Inventory IDs are copied as values; the temporary ID array is always released.
int CPangya2Shop_HasUnownedSelectedParts(void* self) {
    ScanVector found;
    int offset = (int)AsCPangya2Shop(self)->currentCharaIndex * (int)sizeof(ShopPartSelection);
    // Keep the byte cursor for CodeWarrior scheduling; field-derived offsets describe each access.
    char* selectionCursor = (char*)self + offset;

    // Phase 1: scan all 24 ordinary slots; a match must pass both bitset and ownership tests.
    int slot = 0;
    do {
        int index = *(short*)(selectionCursor + IFF_OFFSET(ShopPartScanView, selections[0].partIndices));
        if (index != kNoSelection) {
            struct BitReference { unsigned int* data; unsigned int index; };
            BitReference reference;
            reference.data = scan_bits(self);
            reference.index = index;
            BitReference copy = reference;
            if ((unsigned int)index >= (unsigned int)kPartIndexCount) {
                SCAN_THROW(bitsetTestOutOfRangeMessage, __vt__Q23std12out_of_range, shopStrings.outOfRangeType, __dt__Q23std12out_of_rangeFv);
            }
            int eligible = 0;
            if ((copy.data[(unsigned int)index >> kPartBitWordShift] & (1u << (index & (kPartBitsPerWord - 1)))) &&
                CPangya2Shop_IsItemOwned(self, ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, index)) == 0) {
                eligible = 1;
            }
            if (eligible) {
                void* itemId = ItemInventory_GetPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, index);
                SCAN_PUSH(found, itemId, 0);
            }
        }
        ++slot;
        selectionCursor += sizeof(short);
    } while (slot < kEquipmentSlotCount);

    // Phase 2: append an unowned auxiliary selection, then reduce the collection to a Boolean.
    int index = *((signed char*)self + offset + IFF_OFFSET(ShopPartScanView, selections[0].auxiliaryIndex));
    if (index != kNoSelection) {
        if (!CPangya2Shop_IsItemOwned(self, ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, index))) {
            void* itemId = ItemInventory_GetAuxPartIdAt(AsCSharedDoc(g_sharedDoc)->itemInventory, index);
            SCAN_PUSH(found, itemId, 1);
        }
    }
    if (found.size != 0) return 1;
    return 0;
}
#pragma pop

// On the item-info action, populate the selected product dialog and mark it open.
// obj and its selected tab/builders/dialog are borrowed live shop objects. Return
// 1 once the dialog is opened, 0 for absent input or rejected selection. A failed
// product-info lookup still opens the pane and supplies its selected texture.
int CPangya2Shop_OpenSelectedItemInfo(Obj* obj) {
    // Phase 1: acquire the input singleton, then require a selection and action 0x14.
    if (Sing_805DC5C8() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_80516C10);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = s_gameInputManager;
    }

    void* inputManager = Sing_805DC5C8();

    if (obj->selectedListIndex == kNoSelection) {
        return 0;
    }
    if (!Input_IsActionPressed(inputManager, kShowItemInfoAction, kAnyInputDevice)) {
        return 0;
    }

    // Phase 2: reject absent caddie records, the list-0 type-ID-zero entry, and empty tabs.
    if (obj->categoryIndex == kShopCaddies) {
        if (obj->selectedListIndex == kNoSelection) {
            return 0;
        }
        void* selectedTab = ((void**)obj->productLists)[obj->selectedListIndex];
        int selectedProduct = CShopProductsTab_GetSelectedProductIndex(selectedTab);
        int category = obj->categoryIndex;
        void* selectedRecord = 0;
        switch (category) {
            case kShopCharacters:
                selectedRecord = (void*)(long)ShopProductTable_GetRecordAt(obj->charaProductBuilder, obj->selectedListIndex, selectedProduct);
                break;
            case kShopItems:
                selectedRecord = (void*)(long)ShopProductTable_GetRecordAt(obj->itemProductBuilder, obj->selectedListIndex, selectedProduct);
                break;
            case kShopCaddies:
                selectedRecord = (void*)(long)ShopProductTable_GetRecordAt(obj->caddieProductBuilder, obj->selectedListIndex, selectedProduct);
                break;
            default:
                break;
        }
        if (selectedRecord == 0) {
            return 0;
        }
        if (obj->selectedListIndex == 0) {
            if (((IffItemCommon*)selectedRecord)->typeId == 0) {
                return 0;
            }
        }
    }

    {
        void* selectedTab = ((void**)obj->productLists)[obj->selectedListIndex];
        if (((CShopProductsTab*)selectedTab)->productCount == 0) {
            return 0;
        }
    }

    // Phase 3: show the pane, copy stack payload to its controls, and gate the unowned group.
    CItemInfo_Show(obj->itemInfo);

    ItemInfoBuffer itemInfoData;
    itemInfoData.pangPrice = 0;
    Memory_Clear(itemInfoData.statBonusesAndReserved, 0xa);
    Memory_Clear(itemInfoData.nameMessageId, 0x28);
    Memory_Clear(itemInfoData.descriptionMessageId, 0x28);

    if (CPangya2Shop_GetSelectedProductInfo(obj, &itemInfoData.pangPrice)) {
        CItemInfo_SetDisplayedData(obj->itemInfo, &itemInfoData.pangPrice);
        void* selectedTab = ((void**)obj->productLists)[obj->selectedListIndex];
        int ownershipResult = CShopProductsTab_IsSelectedProductOwned(selectedTab);
        unsigned int leadingZeroBits = (unsigned int)__cntlzw(ownershipResult);
        int notOwned = (int)(leadingZeroBits >> kClzZeroToBooleanShift); // 32 >> 5 iff result == 0.
        CItemInfo_SetNotBuyGroupEnabled(obj->itemInfo, notOwned);
    }

    // Phase 4: choose the matching builder texture, then mark the pane open and refresh controls.
    {
        int textureEntry = 0;
        void* selectedTab = ((void**)obj->productLists)[obj->selectedListIndex];
        int selectedProduct = CShopProductsTab_GetSelectedProductIndex(selectedTab);
        int category = obj->categoryIndex;
        switch (category) {
            case kShopCharacters:
                textureEntry = CBuildProduct_GetTextureEntryAt(obj->charaProductBuilder, obj->selectedListIndex, selectedProduct);
                break;
            case kShopItems:
                textureEntry = CBuildProduct_GetTextureEntryAt(obj->itemProductBuilder, obj->selectedListIndex, selectedProduct);
                break;
            case kShopCaddies:
                textureEntry = CBuildProduct_GetTextureEntryAt(obj->caddieProductBuilder, obj->selectedListIndex, selectedProduct);
                break;
            default:
                break;
        }
        CItemInfo_SetItemTexture(obj->itemInfo, textureEntry);
    }

    obj->itemInfoOpen = 1;
    CPangya2Shop_UpdateControlScriptSelection(obj);
    return 1;
}

} // extern "C"
