// Shop product list builders for items and caddies.
//
// CBuildProduct is the shared builder: it keeps per-category product records and icon texture
// entries. CBuildProductItem and CBuildProductCaddie build the item and caddie lists.

#include <decomp/static_init.h>
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
extern "C" void appFreeUni__FPv(void*);

// Own a completed string only while its enclosing operation can unwind.
struct EH5StringCleanup {
    const unsigned int* words;
    bool active;
    // Guard heap bytes in the completed string representation during unwinding.
    explicit EH5StringCleanup(const void* p) : words((const unsigned int*)p), active(true) {}
    // Transfer string cleanup responsibility to the completed operation.
    void release() {
        active = false;
    }
    // Free heap bytes if the operation did not release this guard.
    ~EH5StringCleanup() {
        if (active && (words[0] & 0x80000000u))
            appFreeUni__FPv((void*)words[2]);
    }
};

#include "game/string.h"

#include "sdk_protos.h"
#include "game/shop_product.h"
#include "iff/IffStruct.h"
#include "classes/CBuildProductCaddie.h"
#include "classes/CBuildProductItem.h"

// Product data model and ownership:
//   +04 provider pointer vector: polymorphic category enumerators, destroyed virtually;
//   +10 snapshotsByCategory: copied 0x2C0 records, freed as raw arrays;
//   +14 sizes: category counts shared by snapshot and texture indexing;
//   +18 texturesByCategory: constructed 0x28 texture objects, destroyed as object arrays.
// Provider growth reserves category slots BEFORE new enumerators are constructed;
// flattening then calls each provider's size/fill into snapshots. Texture loading
// consumes snapshot icon names afterward. Rebuilding invalidates borrowed record/
// texture pointers; freeing providers alone does not free snapshots or textures.
// Product records are UI snapshots, larger than the underlying IFF records.
struct ShopProductListEntry {
    IffItemCommon common;
    char payload[0x2C0 - sizeof(IffItemCommon)];
};
struct ProductTextureEntry {
    void* vtable;
    char storage[0x24];
};
typedef char ProductRecordSize[(sizeof(ShopProductListEntry) == 0x2C0) ? 1 : -1];
typedef char ProductIconOffset[((int)&((ShopProductListEntry*)0)->common.icon == 0x28) ? 1 : -1];
typedef char ProductTextureSize[(sizeof(ProductTextureEntry) == 0x28) ? 1 : -1];
enum ItemProductCategory {
    kClubSets = 0,
    kGolfBalls = 1,
    kEquipmentItems = 2
};
enum CaddieProductCategory {
    kCaddies = 0,
    kCaddieAccessories = 1
};
enum ShopProductSizes {
    kItemCategoryCount = 3,
    kCaddieCategoryCount = 2,
    kProductRecordBytes = 0x2C0,
    kTextureEntryBytes = 0x28
};

// Return existing element storage for placement construction; no allocation.
inline void* operator new(size_t, void* p) {
    return p;
}

extern "C" {

extern float kFloat_6_2831855;
extern float s_itemProductFullTurn;
extern float s_caddieProductFullTurn;
extern char __vt__17CBuildProductItem[];
extern char __vt__19CBuildProductCaddie[];

void SaveData_SetCurrentRecord(void*, void*);
void __ct__13CBuildProductFiUc(void*, int, unsigned char);
void __dt__11ProductBaseFv(void*, int);
void __dl__FPv(void*);
void __dla__FPv(void*);
void String_ReserveCapacity(void*, unsigned int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);

void* String_ReplaceRange(void*, int, int, const char*, const char*, unsigned char*);
void* __nwa__FUl(unsigned int);
void __destroy_new_array(void*, void*);
void String_ConstructConcatCStr(void*, void*, void*);
void appFreeUni__FPv(void*);
extern "C" void __ct__6CBpTexFv(void*);
extern "C" void __dt__6CBpTexFv(void*);
struct ProductRecordStrings {
    char vectorLengthError[20];   // 0x0
    char lengthErrorType[55];     // 0x14
    char emptyPrefix[1];          // 0x4B
    char iconTextureExtension[5]; // 0x4C: ".gsr", appended to product icon names.
};
extern ProductRecordStrings characterRecordStrings;

// ABI wrapper for the game string; the second word is heap length, not capacity.
struct Fn802D6A98String {
    unsigned int flags;
    unsigned int length;
    char* data;
    // Release string-owned heap bytes using the game string destructor.
    ~Fn802D6A98String() {
        ((NString*)(this))->~NString();
    }
};
struct Fn802D6A98Arg {
    volatile unsigned int flags;
    unsigned int length;
    char* data;
};
struct ItemProductBuilderView;
int ShopItemList_BuildProducts(void*);
int BuildProductChara_ClearProductEntries(void*);
struct ProductTextureCacheView {
    char padding00[0x10];
    void** arrays;
    int* sizes;
    void** texturesByCategory;
    char padding1C[0x2D70 - 0x1C];
    unsigned char textureEntriesBuilt;
};

// Construct prefix + string with cleanup if reserve or insertion throws.
// out is uninitialized result storage; prefix/arg are borrowed byte strings.
// Successful construction transfers heap ownership to out; exceptions free it.
// Allocator-token bytes preserve the original ABI and are not character data.
void* String_ConcatCStringAndString(Fn802D6A98String* out, const char* prefix, Fn802D6A98Arg* arg) {
    out->flags = 0;
    out->length = 0;
    out->data = 0;
    EH5StringCleanup eh5Cleanup39(out);
    unsigned int prefixLength = strlen(prefix);
    unsigned int length;
    const char* data;
    if ((arg->flags >> 31) == 0) {
        data = (const char*)arg + 1;
        length = ((volatile unsigned char*)arg)[0] & 0x7f;
    } else {
        data = arg->data;
        length = arg->length;
    }
    String_ReserveCapacity(out, prefixLength + length);
    struct {
        volatile unsigned char second;
        char pad1[3];
        volatile unsigned char sourceSecond;
        char pad2[3];
        volatile unsigned char first;
        char pad3[3];
        volatile unsigned char sourceFirst;
    } chars;
    chars.first = chars.sourceFirst;
    out =
        (Fn802D6A98String*)String_ReplaceRange(out, 0, 0, prefix, prefix + prefixLength, (unsigned char*)&chars.first);
    chars.second = chars.sourceSecond;
    void* built = String_ReplaceRange(out, prefixLength, 0, data, data + length, (unsigned char*)&chars.second);
    eh5Cleanup39.release();
    return built;
}

// Build icon texture paths and create one texture entry for each product snapshot.
// Return 1 if cached or constructed, 0 if sizes is absent after rebuilding products.
// A nonzero result does not poll asynchronous texture readiness; it marks cache setup.
// The load path is iconName + ".gsr" (empty prefix at pooled offset 0x4B).
int ShopItemList_BuildProductTextures(ProductTextureCacheView* self) {
    if (self->textureEntriesBuilt)
        return 1;
    ShopItemList_BuildProducts(self);
    if (!self->sizes)
        return 0;
    // Phase 1: rebuild snapshots, then destroy stale constructed texture arrays.
    if (self->texturesByCategory) {
        for (int i = 0; i < kItemCategoryCount; ++i) {
            if (self->texturesByCategory[i]) {
                __destroy_new_array(self->texturesByCategory[i], (void*)__dt__6CBpTexFv);
                self->texturesByCategory[i] = 0;
            }
        }
    } else {
        self->texturesByCategory = (void**)__nwa__FUl(12);
        self->texturesByCategory[0] = 0;
        self->texturesByCategory[1] = 0;
        self->texturesByCategory[2] = 0;
    }
    // Phase 2: construct a texture array per category (including its 0x10-byte
    // runtime array header), then load each texture from its snapshot's icon name.
    for (int category = 0; category < kItemCategoryCount; ++category) {
        int count = self->sizes[category];
        void* records = __nwa__FUl(count * kTextureEntryBytes + 0x10);
        if (records)
            records = __construct_new_array(records, (void*)__ct__6CBpTexFv, (void*)__dt__6CBpTexFv, kTextureEntryBytes,
                                            count);
        self->texturesByCategory[category] = records;
        for (int i = 0; i < count; ++i) {
            ProductTextureEntry* record = (ProductTextureEntry*)self->texturesByCategory[category] + i;
            char* source = ((ShopProductListEntry*)self->arrays[category])[i].common.icon;
            struct StringCleanup {
                Fn802D6A98String* value;
                // Guard an already constructed temporary string.
                StringCleanup(Fn802D6A98String* p) : value(p) {}
                // Destroy the temporary string on normal or exception exit.
                ~StringCleanup() {
                    value->~Fn802D6A98String();
                }
            };
            // Path construction: snapshot icon -> owned iconName -> empty-prefix
            // temporary -> texturePath with .gsr suffix. Guards own intermediate
            // heap strings; the texture loader must consume/copy the borrowed path.
            Fn802D6A98String iconName;
            iconName.flags = 0;
            iconName.length = 0;
            iconName.data = 0;
            unsigned int length = strlen(source);
            unsigned char allocator = 0;
            String_ReserveCapacity(&iconName, length);
            String_ReplaceRange(&iconName, 0, 0, source, source + length, &allocator);
            unsigned int pathStorage[3];
            Fn802D6A98String* texturePath = (Fn802D6A98String*)pathStorage;
            {
                unsigned int prefixedStorage[3];
                Fn802D6A98String* prefixedName = (Fn802D6A98String*)prefixedStorage;
                String_ConcatCStringAndString(prefixedName, characterRecordStrings.emptyPrefix,
                                              (Fn802D6A98Arg*)&iconName);
                StringCleanup cleanupB(prefixedName);
                String_ConstructConcatCStr(texturePath, prefixedName, characterRecordStrings.iconTextureExtension);
            }
            StringCleanup cleanupC(texturePath);
            const char* path = (texturePath->flags >> 31) == 0 ? (const char*)texturePath + 1 : texturePath->data;
            typedef void (*LoadTexture)(void*, const char*);
            ((LoadTexture*)record->vtable)[3](record, path);
            String_ReplaceRange(&iconName, 0, -1, 0, 0, &allocator);
            String_ReplaceRange(texturePath, 0, -1, 0, 0, &allocator);
        }
    }
    self->textureEntriesBuilt = 1;
    return 1;
}

struct ProductProviderListView {
    char padding00[4];
    void** entries;
    unsigned int count;
};

struct ItemProductBuilderView {
    char padding00[4];
    void** providersByCategory; // Owned category providers; product snapshots live in arrays below.
    unsigned int count;
    unsigned int capacity;
    void** arrays;
    unsigned int* sizes;
    char padding18[8];
    unsigned char mode;
    // Borrow one provider; caller ensures index is below count.
    void* at(unsigned int index) {
        return providersByCategory[index];
    }
};
// Control block of the shared char-string the length_error message is held in.
struct SharedCharControlBlock {
    void* vtable;
    int refs1;
    int refs2;
    char* data;
};
void* __nw__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* __ct__19CShopProductClubSetFPvPv(void*, void*, unsigned char);
void* __ct__16CShopProductBallFPvPv(void*, void*, unsigned char);
void* __ct__21CShopProductEquipItemFPvPv(void*, void*, unsigned char);
void appFreeUni__FPv(void*);
int PtrArray_FreeRecords(void*);
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
extern char s_sharedCharDeleterVtable[];
extern ProductRecordStrings characterRecordStrings;
void __dt__Q23std12length_errorFv(void*);
void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);

struct ClubSetProviderInit {
    char data[0x14];
    // Construct the owned club-set provider with its borrowed save owner.
    ClubSetProviderInit(void* owner, unsigned char mode) {
        __ct__19CShopProductClubSetFPvPv(this, owner, mode);
    }
};
struct BallProviderInit {
    char data[0x14];
    // Construct the owned ball provider with its borrowed save owner.
    BallProviderInit(void* owner, unsigned char mode) {
        __ct__16CShopProductBallFPvPv(this, owner, mode);
    }
};
struct EquipItemProviderInit {
    char data[0x14];
    // Construct the owned equipment provider with its borrowed save owner.
    EquipItemProviderInit(void* owner, unsigned char mode) {
        __ct__21CShopProductEquipItemFPvPv(this, owner, mode);
    }
};
// Virtual interface of a category provider: size() = number of products, fill(dest, i) = write product i into the
// 0x2C0-byte snapshot record at dest. (Slots 0 and 1 are not used here.)
struct ProductProviderView {
    virtual void slot0();
    virtual void slot1();
    virtual void fill(void*, int);
    virtual int size();
};
// Scratch transaction for growing a provider-pointer vector: new storage is staged here, swapped in, and the old
// block is freed on scope exit (hand-expanded std::vector growth).
struct ProviderVectorGrowth {
    void** data;
    unsigned int count, capacity;
    unsigned int* allocator;
    unsigned int offset;
    // Stage replacement provider-vector storage before committing ownership.
    ProviderVectorGrowth(unsigned int* p) : data(0), count(0), capacity(0), allocator(p), offset(0) {}
    // Free the allocation retained by the staging transaction.
    ~ProviderVectorGrowth() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
};
} // extern "C"
namespace std {
struct exception {
    void* vtable;
};
struct logic_error : exception {
    char* text;
    void* control;
    // Leave exception storage for the derived constructor to initialize.
    logic_error() {}
    // Copy exception text into storage owned by a shared deleter.
    logic_error(const char* const& message, int) {
        struct BufferGuard {
            char* data;
            char* flag;
            // Delete the message allocation if shared-deleter construction unwinds.
            ~BufferGuard() {
                if (data)
                    __dla__FPv(data);
            }
        };
        vtable = __vt__Q23std11logic_error;
        struct {
            char value;
            char padding[3];
        } flag;
        flag.value = 0;
        unsigned int n = strlen(message) + 1;
        char* allocated = (char*)__nwa__FUl(2 * n - n);
        text = allocated;
        {
            BufferGuard guard;
            guard.data = allocated;
            guard.flag = &flag.value;
            SharedCharControlBlock* node = (SharedCharControlBlock*)__nw__FUl(0x10);
            if (node) {
                node->refs1 = 1;
                node->refs2 = 1;
                node->vtable = s_sharedCharDeleterVtable;
                node->data = allocated;
            }
            control = node;
            guard.data = 0;
        }
        strcpy(text, message);
    }
    ~logic_error();
};
struct length_error : logic_error {
    // Build and retag the length exception with guarded message allocation.
    length_error(const char* message) {
        struct BufferGuard {
            char* data;
            char* flag;
            // Delete the message allocation if shared-deleter construction unwinds.
            ~BufferGuard() {
                if (data)
                    __dla__FPv(data);
            }
        };
        vtable = __vt__Q23std11logic_error;
        struct {
            char value;
            char padding[3];
        } flag;
        flag.value = 0;
        unsigned int n = strlen(message) + 1;
        char* allocated = (char*)__nwa__FUl(2 * n - n);
        text = allocated;
        {
            BufferGuard guard;
            guard.data = allocated;
            guard.flag = &flag.value;
            SharedCharControlBlock* node = (SharedCharControlBlock*)__nw__FUl(0x10);
            if (node) {
                node->refs1 = 1;
                node->refs2 = 1;
                node->vtable = s_sharedCharDeleterVtable;
                node->data = allocated;
            }
            control = node;
            guard.data = 0;
        }
        strcpy(text, message);
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
    // Retag the completed logic_error base as a length_error.
    length_error(const char* const& message, int) : logic_error(message, 0) {
        vtable = __vt__Q23std12length_error;
    }
};
}
extern "C" {

// Choose the original 60%, then 50%, growth policy for product-provider pointers.
inline unsigned int RecommendItemProviderCapacity(const unsigned int& currentCapacity, const unsigned int& requested) {
    unsigned int capacity = currentCapacity;
    if (requested > 0x3fffffffu - capacity) {
        throw std::length_error(characterRecordStrings.vectorLengthError, 0);
    }
    if (capacity < 0x15555555u) {
        unsigned int growth = (capacity + 1) * 3 / 5;
        const unsigned int* larger = &growth;
        if (growth < requested)
            larger = &requested;
        return capacity + *larger;
    } else if (capacity < 0x2aaaaaaau) {
        unsigned int growth = (capacity + 1) / 2;
        const unsigned int* larger = &growth;
        if (growth < requested)
            larger = &requested;
        return capacity + *larger;
    }
    return 0x3fffffffu;
}

#pragma push
#pragma use_lmw_stmw on
// Rebuild club-set, ball, and equipment categories and their product snapshot arrays.
// Return 0 only if provider clearing reports failure, else 1 after flattening.
// The builder owns newly allocated providers and snapshots; size/fill are virtual
// calls on fully constructed category providers, never on the reserved null slots.
int ShopItemList_BuildProducts(void* ptr) {
    unsigned int count;
    unsigned int add;
    ItemProductBuilderView* self;
    self = (ItemProductBuilderView*)ptr;
    if (!BuildProductChara_ClearProductEntries(self))
        return 0;
    PtrArray_FreeRecords(self);
    // Phase 1: provider count is cleared and old snapshots freed; reserve three slots.
    count = self->count;
    if (count < kItemCategoryCount) {
        add = kItemCategoryCount - count;
        // Original 0x802D6194: share the resize completion without reordering growth.
        do {
            if (add > self->capacity || count > self->capacity - add) {
                RecommendItemProviderCapacity(self->capacity, self->count + add - self->capacity);
            } else {
                memset((char*)self->providersByCategory + count * 4, 0, add * 4);
                self->count += add;
                break;
            }
            {
                unsigned int requested = self->count + add - self->capacity;
                ProviderVectorGrowth fresh(&self->capacity);
                unsigned int newCapacity = RecommendItemProviderCapacity(self->capacity, requested);
                if (newCapacity > 0x3fffffffu) {
                    throw std::length_error(characterRecordStrings.vectorLengthError, 0);
                }
                fresh.data = (void**)appMallocDummy__FUl(newCapacity * 4);
                unsigned int oldCount = self->count;
                fresh.capacity = newCapacity;
                fresh.offset = oldCount;
                memset(fresh.data + oldCount + fresh.count, 0, add * 4);
                void** old = self->providersByCategory;
                int moved = ((char*)(old + self->count) - (char*)old) / 4;
                fresh.count += add;
                fresh.offset -= moved;
                memcpy(fresh.data + fresh.offset, old, moved * 4);
                memset(old, 0, moved * 4);
                fresh.count += moved;
                old = self->providersByCategory;
                void** replacement = fresh.data;
                self->count = 0;
                unsigned int oldCapacity = self->capacity;
                fresh.data = old;
                self->capacity = fresh.capacity;
                fresh.capacity = oldCapacity;
                self->providersByCategory = replacement;
                self->count = fresh.count;
                fresh.count = 0;
            }
        } while (0);
    } else if (count > kItemCategoryCount)
        self->count -= count - kItemCategoryCount;
    // Phase 2: construct all category providers before reading any size/fill callback.
    void* owner = ((CBuildProduct*)self)->object24;
    self->providersByCategory[kClubSets] = new ClubSetProviderInit(owner, self->mode);
    self->providersByCategory[kGolfBalls] = new BallProviderInit(owner, self->mode);
    self->providersByCategory[kEquipmentItems] = new EquipItemProviderInit(owner, self->mode);
    // Phase 3: allocate by-value snapshot arrays and flatten each provider in order.
    self->arrays = (void**)__nwa__FUl(12);
    self->arrays[0] = 0;
    self->arrays[1] = 0;
    self->arrays[2] = 0;
    for (int i = 0; i < kItemCategoryCount; ++i) {
        self->sizes[i] = ((ProductProviderView*)self->at(i))->size();
        self->arrays[i] = (void*)__nwa__FUl(self->sizes[i] * kProductRecordBytes);
        for (int j = 0; j < (int)self->sizes[i]; ++j)
            ((ProductProviderView*)self->at(i))->fill((char*)self->arrays[i] + j * kProductRecordBytes, j);
    }
    return 1;
}
#pragma pop

struct OwnedProductProvider {
    // Fallback destruction hook; concrete providers supply allocation cleanup.
    virtual void destroy(int) {}
};

struct ProductTextureGroupsView {
    char padding00[0x14];
    int* categoryCounts;
    int** values;
    int count;
};

struct ProductRecordGroupsView {
    char padding00[0x10];
    int** values;
    int* categoryCounts;
    char padding18[4];
    int count;
};

struct ProductCategoryCountsView {
    char padding00[0x14];
    int* entries;
    char padding18[4];
    int count;
};

struct ProductSaveContextView {
    char padding00[0x24];
    char saveRecord[0x2D70 - 0x24];
    unsigned char textureEntriesBuilt;
};

// Report that the product builder is available.
int CBuildProduct::vfunc_1() {
    return 1;
}
// Initialize the item-product unit's angle constant.
void InitItemProductAngleConstant() {
    s_itemProductFullTurn = kFloat_6_2831855;
}
// Read a category count using the original inclusive upper-bound check.
// Caller contract for all category accessors below: 0 <= category < count.
// Existing check rejects only category > count, so category == count and negatives
// still index storage and are not validated here. -1 reports only that checked case.
int CBuildProduct::GetProductCount(int category) {
    ProductCategoryCountsView* self = (ProductCategoryCountsView*)this;
    if (self->count < category)
        return -1;
    return self->entries[category];
}
// Select the builder's save record and invalidate its texture-entry setup flag.
int CBuildProduct::SelectSaveRecord(void* src) {
    ProductSaveContextView* self = (ProductSaveContextView*)this;
    SaveData_SetCurrentRecord(self->saveRecord, src);
    self->textureEntriesBuilt = 0;
    return 1;
}
// Construct the common product builder and install the item-builder vtable.
CBuildProductItem::CBuildProductItem(int count, unsigned char flag) {
    __ct__13CBuildProductFiUc(this, count, flag);
    *(void**)this = (void*)__vt__17CBuildProductItem;
}
// Initialize the caddie-product unit's angle constant.
void InitCaddieProductAngleConstant() {
    s_caddieProductFullTurn = kFloat_6_2831855;
}
// Destroy the item builder and optionally free its allocation.
CBuildProductItem::~CBuildProductItem() {
    __dt__11ProductBaseFv(this, 0);
}
// Destroy the caddie builder and optionally free its allocation.
CBuildProductCaddie::~CBuildProductCaddie() {
    __dt__11ProductBaseFv(this, 0);
}
// Construct the common product builder and install the caddie-builder vtable.
CBuildProductCaddie::CBuildProductCaddie(int count, unsigned char flag) {
    __ct__13CBuildProductFiUc(this, count, flag);
    *(void**)this = (void*)__vt__19CBuildProductCaddie;
}
// Return a nonempty category's texture entries; retain the original bound check.
// Borrow the category texture array until rebuild/destruction; valid category
// required as above. Null means a checked rejection or empty category.
int* CBuildProduct::GetProductDrawEntries(int category) {
    ProductTextureGroupsView* self = (ProductTextureGroupsView*)this;
    if (self->count < category)
        return 0;
    int entryCount = (self->count < category) ? -1 : self->categoryCounts[category];
    if (entryCount == 0)
        return 0;
    return self->values[category];
}
// Find a texture entry by category and index using the original bounds and null checks.
// Requires valid category and entryIndex >= 0. Rejects index >= category count,
// but a negative index steps BEFORE storage. Returned entry is borrowed.
void* CBuildProduct::GetTextureEntryAt(int category, int entryIndex) {
    int entryCount;
    void* categoryEntries;
    if (count < category) {
        categoryEntries = 0;
    } else {
        entryCount = (count < category) ? -1 : indices[category];
        categoryEntries = entryCount ? ((void**)textureBindingsBySlot)[category] : 0;
    }
    if (categoryEntries == 0)
        return 0;
    entryCount = (count < category) ? -1 : indices[category];
    if (entryCount <= entryIndex)
        return 0;
    return (char*)((void**)textureBindingsBySlot)[category] + entryIndex * kTextureEntryBytes;
}
// Return a nonempty category's product records; retain the original bound check.
// Borrow the category snapshot array until rebuild/destruction; valid category
// required as above. Null means a checked rejection or empty category.
int* CBuildProduct::GetProductRecords(int category) {
    ProductRecordGroupsView* self = (ProductRecordGroupsView*)this;
    if (self->count < category)
        return 0;
    int entryCount = (self->count < category) ? -1 : self->categoryCounts[category];
    if (entryCount == 0)
        return 0;
    return self->values[category];
}
struct ProductRecordLookupView {
    char padding00[0x10];
    int** records;
    int* values;
    char padding18[4];
    int count;
};
// Find a product snapshot by category and index using the original bounds and null checks.
// Requires valid category and productIndex >= 0. Rejects index >= category count,
// but has no negative-index check. Returned snapshot is borrowed until rebuild.
void* ShopProductTable_GetRecordAt(ProductRecordLookupView* self, int category, int productIndex) {
    int productCount;
    void* categoryRecords;
    if (self->count < category) {
        categoryRecords = 0;
    } else {
        productCount = (self->count < category) ? -1 : self->values[category];
        categoryRecords = productCount ? self->records[category] : 0;
    }
    if (categoryRecords == 0)
        return 0;
    productCount = (self->count < category) ? -1 : self->values[category];
    if (productCount <= productIndex)
        return 0;
    return (char*)self->records[category] + productIndex * kProductRecordBytes;
}
// Release each category's raw record array and then the category-pointer array.
int PtrArray_FreeRecords(void* ptr) {
    struct ProductRecordArrayOwnerView {
        char padding00[0x10];
        void** records;
        char padding14[0x08];
        int count;
    }* self = (struct ProductRecordArrayOwnerView*)ptr;
    if (self->records == 0)
        return 0;
    for (int i = self->count - 1; i >= 0; --i) {
        if (self->records[i] != 0) {
            __dla__FPv(self->records[i]);
            self->records[i] = 0;
        }
    }
    if (self->records != 0) {
        __dla__FPv(self->records);
        self->records = 0;
    }
    return 1;
}
#pragma dont_inline on
// Destroy all owned product providers and clear the logical provider count.
int BuildProductChara_ClearProductEntries(void* ptr) {
    ProductProviderListView* self = (ProductProviderListView*)ptr;
    void** entry = self->entries;
    for (; self->count + self->entries != entry; ++entry) {
        if (*entry != 0) {
            if (*entry != 0) {
                ((OwnedProductProvider*)*entry)->destroy(1);
            }
            *entry = 0;
        }
    }
    self->count = self->count - self->count;
    return 1;
}

#pragma dont_inline off

struct CaddieProductBuilderView {
    char padding00[4];
    void** providersByCategory; // Owned caddie/accessory providers, not product snapshots.
    unsigned int count;
    unsigned int capacity;
    void** productSnapshots;
    int* sizes;
    char padding18[8];
    unsigned char mode;
    char padding21[0x2D70 - 0x21];
    unsigned char textureEntriesBuilt;
    // Borrow a provider at a caller-validated index; do not transfer ownership.
    // Keep byte addressing here: typed indexing swaps the indexed-load operands.
    void* at(unsigned int index) {
        return *(void**)((char*)providersByCategory + index * sizeof(void*));
    }
};
void* __ct__18CShopProductCaddieFPvPv(void*, void*, unsigned char);
void* __ct__22CShopProductCaddieItemFPvPvPv(void*, void*, void*, unsigned char);
void* ItemInventory_GetCaddieIdAt(void*, void*);
extern CSharedDoc* g_sharedDoc;
struct CaddieRecordStrings {
    char vectorLengthError[20]; // 0x0
    char lengthErrorType[55];   // 0x14
    char noCaddieIcon[10];      // 0x4B
    char noCaddieName[18];      // 0x55
};
extern CaddieRecordStrings caddieRecordStrings;

struct Init6BF8A {
    char data[0x14];
    // Construct the owned caddie provider with its borrowed save owner.
    Init6BF8A(void* owner, unsigned char mode) {
        __ct__18CShopProductCaddieFPvPv(this, owner, mode);
    }
};
struct Init6BF8B {
    char data[0x18];
    // Resolve the caddie inventory entry, then construct its accessory provider.
    Init6BF8B(void* arg, void* owner, CaddieProductBuilderView* self) {
        void* res = ItemInventory_GetCaddieIdAt(g_sharedDoc->itemInventory, arg);
        __ct__22CShopProductCaddieItemFPvPvPv(this, res, owner, self->mode);
    }
};
struct Record6BF8 {
    virtual void slot0();
    virtual void slot1();
    virtual void fill(void*, int);
    virtual int size();
};
struct Value6BF8 {
    union {
        unsigned int words[kProductRecordBytes / 4];
        ShopProductListEntry record;
    };
};
// Select the larger growth request by reference.
inline const unsigned int& MaxCaddieProviderGrowth(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Choose caddie-provider capacity while checking the pointer-vector allocation limit.
inline unsigned int RecommendCaddieProviderCapacity(const unsigned int& currentCapacity,
                                                    const unsigned int& requested) {
    unsigned int capacity = currentCapacity;
    if (requested > 0x3fffffffu - capacity)
        throw std::length_error(caddieRecordStrings.vectorLengthError, 0);
    if (capacity < 0x15555555u) {
        unsigned int growth = (capacity + 1) * 3 / 5;
        return capacity + MaxCaddieProviderGrowth(growth, requested);
    } else if (capacity < 0x2aaaaaaau) {
        unsigned int growth = (capacity + 1) / 2;
        return capacity + MaxCaddieProviderGrowth(growth, requested);
    }
    return 0x3fffffffu;
}

// Move provider pointers into replacement storage and clear the old slots.
inline void MoveCaddieProviders(ProviderVectorGrowth& fresh, void** first, void** last) {
    int moved = ((char*)last - (char*)first) / 4;
    fresh.offset -= moved;
    memcpy(fresh.data + fresh.offset, first, moved * 4);
    memset(first, 0, moved * 4);
    fresh.count += moved;
}
// Initialize new provider slots and transfer the existing providers.
inline void AppendCaddieProviderSlots(ProviderVectorGrowth& fresh, CaddieProductBuilderView* self, unsigned int add,
                                      unsigned int capacity) {
    unsigned int oldCount = self->count;
    fresh.capacity = capacity;
    fresh.offset = oldCount;
    memset(fresh.data + oldCount + fresh.count, 0, add * 4);
    fresh.count += add;
    MoveCaddieProviders(fresh, self->providersByCategory, self->providersByCategory + self->count);
}
// Allocate replacement provider storage and swap it into the caddie builder.
inline void GrowCaddieProviders(CaddieProductBuilderView* self, unsigned int add) {
    ProviderVectorGrowth fresh(&self->capacity);
    unsigned int requested = self->count + add - self->capacity;
    unsigned int newCapacity = RecommendCaddieProviderCapacity(self->capacity, requested);
    if (newCapacity > 0x3fffffffu)
        throw std::length_error(caddieRecordStrings.vectorLengthError, 0);
    fresh.data = (void**)appMallocDummy__FUl(newCapacity * 4);
    AppendCaddieProviderSlots(fresh, self, add, newCapacity);
    void** old = self->providersByCategory;
    void** replacement = fresh.data;
    self->count = 0;
    unsigned int oldCapacity = self->capacity;
    fresh.data = old;
    self->capacity = fresh.capacity;
    fresh.capacity = oldCapacity;
    self->providersByCategory = replacement;
    self->count = fresh.count;
    fresh.count = 0;
}
#pragma push
#pragma use_lmw_stmw on
// Build caddie/accessory categories and append the special no-caddie record.
// The last caddie snapshot is a zero-type "no caddie" choice. Accessories always
// rebuild; an existing caddie snapshot array is retained unless mode is nonzero
// and textureEntriesBuilt is false. This selective reuse is intentional current behavior.
int ShopCaddieList_BuildProducts(CaddieProductBuilderView* self, void* arg) {
    if (!BuildProductChara_ClearProductEntries(self))
        return 0;
    unsigned int count = self->count;
    if (count < kCaddieCategoryCount) {
        unsigned int add = kCaddieCategoryCount - count;
        // Original 0x802D6BF8: grow or fill the available capacity.
        do {
            if (add > self->capacity || count > self->capacity - add) {
                RecommendCaddieProviderCapacity(self->capacity,
                                                *(volatile unsigned int*)&self->count + add - self->capacity);
            } else {
                memset(self->providersByCategory + count, 0, add * 4);
                self->count += add;
                break;
            }
            GrowCaddieProviders(self, add);
        } while (0);
    } else if (count > kCaddieCategoryCount)
        self->count -= count - kCaddieCategoryCount;
    void* owner = ((CBuildProduct*)self)->object24;
    self->providersByCategory[kCaddies] = new Init6BF8A(owner, self->mode);
    self->providersByCategory[kCaddieAccessories] = new Init6BF8B(arg, owner, self);

    // Phase 2: provider slots are populated; invalidate only the required snapshots.
    if (self->productSnapshots) {
        if (self->productSnapshots[1]) {
            __dla__FPv(self->productSnapshots[1]);
            self->productSnapshots[1] = 0;
        }
        if (self->mode && !self->textureEntriesBuilt && self->productSnapshots[0]) {
            __dla__FPv(self->productSnapshots[0]);
            self->productSnapshots[0] = 0;
        }
    } else {
        self->productSnapshots = (void**)__nwa__FUl(8);
        self->productSnapshots[0] = 0;
        self->productSnapshots[1] = 0;
    }
    // Phase 3: flatten missing arrays, appending the no-caddie choice only to category 0.
    for (int category = 0; category < kCaddieCategoryCount; ++category) {
        if (self->productSnapshots[category])
            continue;
        self->sizes[category] = ((Record6BF8*)self->at(category))->size();
        if (category == kCaddies)
            ++self->sizes[category];
        self->productSnapshots[category] = __nwa__FUl(self->sizes[category] * kProductRecordBytes);
        for (int i = 0; i < self->sizes[category]; ++i) {
            if (category == kCaddies && i == self->sizes[category] - 1) {
                Value6BF8 value = {};
                value.record.common.typeId = 0;
                strncpy(value.record.common.icon, caddieRecordStrings.noCaddieIcon, 10);
                strncpy(value.record.common.name, caddieRecordStrings.noCaddieName, 20);
                ((Value6BF8*)self->productSnapshots[category])[i] = value;
            } else
                ((Record6BF8*)self->at(category))
                    ->fill((char*)self->productSnapshots[category] + i * kProductRecordBytes, i);
        }
    }
    return 1;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046250C", _ctors_u802D5D78_0, InitItemProductAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046250C", _ctors_u802D5D78_1, InitCaddieProductAngleConstant);
#pragma pop
