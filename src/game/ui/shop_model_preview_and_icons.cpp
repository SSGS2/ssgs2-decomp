// Shop model previews and product icon textures for the shop.
//
// The CModelOperation classes (character, caddie and item variants) show the selected product
// as a 3D model. CBuildProductChara builds the character product lists and their icon texture
// groups from the character's texture archive.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include <decomp/ct_member.h>
// Shop model previews and character/caddie icon loading (frame rate, model, texture).
// Entry points are CModelOperationItem::UpdateRotationAndExhibition/2/3 and CBuildProductChara::Load*;
// provider schemas live in game/shop_product.h and classes/CBuildProductCharaFields.h.
// Mod lookup:
// - Automatic item yaw: kItemPreviewAutoYawStepStorage in the data section,
//   binary32 0.034906585 (~2 degrees in radians) PER UPDATE CALL, not per second.
//   Pointer rotation is a separate branch; the update object's delta is unchanged.
// - Preview framing/viewport: ModelOperationBase_GetDefaultViewport and
//   ApplyCameraPreset in model_operation_exhibition_preview.cpp; no resolution/aspect constant is set here.
// - Club set / ball model path: CModelOperationItem::OpenResource uses
//   itemPreviewResourcePaths (+0/+kBallPreviewResource); isClubSet chooses the path.
// - Fashion archive: LoadProductTextures indexes resourceNames[0..7]; icon paths
//   come from each IFF record's icon field and the suffix in fashionArchiveStrings.
// - Caddie/accessory icons: CBuildProductCaddie::LoadIconGroups calls ShopCaddieList_BuildProducts
//   (shop_product_list_builder.cpp), despite its retained Chara ABI name. Its argument is a CADDIE index.
//   Its two-group storage must not be used as the five-group fashion storage.
// Preserve: asynchronous archive lifetime, record/texture parallel indices, first
// group cache policy, pointer-shaped IDs, float operation order and virtual ABI.
// All self pointers require constructed objects except nullable deleting dtors.
// Rendering/resource methods run through the existing UI update sequence; no locks
// or rollback are added. Dimensions/viewport units belong to the base preview API.
#include "iff/IffStruct.h"
// Builder storage/ownership contract (shared field aliases are retained in the header):
// +04 products_*: owned vector of eight provider pointers; count/capacity count pointers.
// +10 recordArrays_*: owned pointer table of flattened fixed-stride IFF record copies.
// +14 recordCounts_*: pointer to signed counts, NOT a scalar count. _pp is a legacy
// cast view used for address arithmetic; _u32 holds the same 32-bit pointer bits.
// +18 resourceArrays_*: owned parallel CBpTex arrays; record g/i maps to texture g/i.
// CBuildProductCaddie::LoadIconGroups uses two groups; LoadProductTextures uses five. Their table
// allocations have different sizes, so do not interchange these loading paths.
// Rebuild policy: all groups after group 0 are discarded every load. First group 0 is
// discarded only when flag != 0 AND textureEntriesBuilt == 0; otherwise existing data is reused.
// textureEntriesBuilt becomes 1 after texture entry setup; archive polling is a separate operation.
// Resource lookup results are borrowed and become invalid when their group is rebuilt.
enum CharacterProductStorage {
    kProductRecordStride = sizeof(IFF_STRUCT::sBall),
    kTextureEntryStride = 0x28,
    kAccessoryRecordGroup = 4,
    kProductCategoryCount = 8,
    kEmbeddedSaveDataOffset = 0x24
};
enum CharacterProductCategory {
    kAvatarCategory = 0,
    kTopCategory = 1,
    kBottomCategory = 2,
    kShoesCategory = 3,
    kAccessoriesCategory = 4,
    kHairCategory = 5,
    kRingsCategory = 6,
    kCostumeSetsCategory = 7
};
enum {
    kFlattenedProductGroupCount = 5,
    kSingleProviderGroupCount = 4
};
// Provider/category index -> flattened record group -> texture/display index:
// 0 avatar       -> 0 -> same index
// 1 top          -> 1 -> same index
// 2 bottom       -> 2 -> same index
// 3 shoes        -> 3 -> same index
// 4 accessories  -> 4 -> [0, accessoryCount)
// 5 hair         -> 4 -> [accessoryCount, accessoryCount+hairCount)
// 6 rings        -> 4 -> [accessoryCount+hairCount, totalGroup4Count)
// 7 costume sets -> extraRecords/extraRecordCount, no parallel texture group
// A provider read takes a PROVIDER-LOCAL index, not the concatenated display index.
// FindResourceByTypeId searches groups 1..4; avatar group 0 is deliberately absent.
// Tables at +10/+14/+18 share group numbers, but hold records/counts/textures.
// Byte addressing uses group*4, recordIndex*kProductRecordStride, or
// recordIndex*kTextureEntryStride respectively; those offsets are not interchangeable.
enum {
    kProductTableEntryBytes = sizeof(void*),
    kFirstNonAvatarGroup = 1
};

enum CharacterPartFilters {
    kTopPartMask = 1,
    kBottomPartMask = 2,
    kShoePartMask = 0x10,
    kAccessoryPartMask = 0x2C
};
// US character names; archive offsets still select the original internal resource names.
enum CharacterFashionArchives {
    kScoutFashionArchive = 0x4B,
    kHanaFashionArchive = 0x59,
    kUncleBobFashionArchive = 0x67,
    kCeciliaFashionArchive = 0x75,
    kMaxFashionArchive = 0x83,
    kKoohFashionArchive = 0x91,
    kArinFashionArchive = 0x9F,
    kKazFashionArchive = 0xAD
};
// Eight product categories: avatar, top, bottom, shoes, accessories, hair, rings, costume sets.
// Record group 4 concatenates accessories, hair and rings; costume sets are stored separately.
enum ItemPreviewTextOffsets {
    kBallPreviewResource = 0x16
};

enum CharacterIconTextOffsets {
    kEmptyIconPrefix = 0x67,
    kIconResourceSuffix = 0x68
};

enum FashionTextOffsets {
    kEmptyTexturePrefix = 0xBB,
    kTextureFileExtension = 0xBC,
    kVectorLengthErrorType = 0x14
};

#include "classes/CBuildProductCharaFields.h"
#include "classes/CBuildProductChara.h"
// CBuildProductCaddie::LoadIconGroups (0x802D7340):
// resourceArrays at +0x18 holds pointer entries;
// the +4 store clears its second entry, now written as resourceArrays_pp[1].
#include "classes/CModelOperationCaddie.h"
#include "classes/CModelOperationItem.h"
#include "classes/CSharedDoc.h"
#include "classes/ShopProductRecord.h"

#define AsCBuildProductCharaFields(p) ((CBuildProductCharaFields*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#define AsShopProductRecord(p) ((ShopProductRecord*)(p))

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char fashionArchiveStrings[];
}
}

#include "game/singletons.h"
#include "sdk_protos.h"
#include "game/shop_product.h"
#include "game/exception.h"
#include "classes/CBuildProductCaddie.h"
#include "classes/CModelOperation.h"
#include "classes/CAsyncExhibitionPet.h"
#include "classes/CPackTextureResource.h"
#include "classes/CBpTex.h"

extern "C" {

extern float kFloat_6_2831855;
extern float s_characterProductAngleConstant;
extern float s_caddieModelOperationAngleConstant;
extern float s_itemModelOperationAngleConstant;
extern const char itemPreviewResourcePaths[]; // Packed club-set and ball preview resource paths.
extern unsigned char __vt__21CModelOperationCaddie[];
extern unsigned char __vt__19CModelOperationItem[];
extern unsigned char __vt__18CBuildProductChara[];

extern void ModelOperationBase_Ctor(void*);
extern void __ct__13CBuildProductFiUc(void*, int, unsigned char);
extern void __dt__18ModelOperationBaseFv(void*, int);
extern void* __nwa__FUl(unsigned int);
extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern void __dt__11ProductBaseFv(void*, int);
extern int ModelOperationBase_GetDefaultViewport(void*, void*);
extern int ModelOperationBase_ReleaseExhibition(void*);
extern int ModelOperationBase_ApplyCameraPreset(void*, int);
extern int ModelOperationBase_UpdatePointerRotation(void*);
extern void* ItemInventory_FindSetItemData(void*, void*);
extern void* g_sharedDoc;
extern int ModelOperationBase_OffsetRotation(void*, void*);
extern int ModelOperationBase_IsInteractiveRotationEnabled(void*);
extern float kItemPreviewCameraDistance;
extern float kFloat_0_ModelOperationItem;
extern float kItemPreviewAutoYawStepStorage;

extern void ShopCaddieList_BuildProducts(void*, int);
extern void String_ConcatCStringAndString(void*, const char*, void*);
extern void String_ConstructConcatCStr(void*, void*, void*);
extern void* ResourceTable_FindEntryByName(void*, void*, void*);
extern void* __ct__18CShopProductAvatarFPvUc(void*, void*, unsigned char);
extern int CharacterProducts_CopyRecordForInventoryCharacter(void*, void*, int);
extern void* __ct__17CShopProductPartsFPCUiUiPvPv(void*, void*, int, void*, unsigned char);
extern void* __ct__16CShopProductHairFPvPvPv(void*, void*, void*, unsigned char);
extern void* __ct__19CShopProductSetItemFPvPvPv(void*, void*, void*, unsigned char);
extern void* __ct__16CShopProductRingFPvPv(void*, void*, unsigned char);
extern int BuildProductChara_ClearProductEntries(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void Allocation_Dtor(void*);
extern char s_characterResourceTableDestructorRegistration[12];
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern const char fashionArchiveStrings[];
extern const char* MSLString_LowercaseInPlace(const char*);
extern void appFreeUni__FPv(void*);
extern "C" void __dt__6CBpTexFv(void*, int);
extern void __destroy_new_array(void*, void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern char caddieRecordStrings[];
extern void String_ReserveCapacity(void*, unsigned int);
extern void* String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, void*);
#pragma push
#pragma use_lmw_stmw on
struct Allocator73 {
    unsigned char value;
    // Preserve the allocator tag's original default-construction behavior.
    Allocator73() {}
};
struct String73 {
    unsigned int words[3];
    // Leave this temporary uninitialized until the string ABI constructs its value.
    String73() {}
    // Copy a C string, reporting its length through the caller's scratch word.
    String73(const char* text, unsigned int& length, const Allocator73& allocator = Allocator73()) {
        words[0] = 0;
        words[1] = 0;
        words[2] = 0;
        length = strlen(text);
        String_ReserveCapacity(this, length);
        ((void* (*)(String73*, unsigned int, unsigned int, const char*, const char*, Allocator73))String_ReplaceRange)(
            this, 0, 0, text, text + length, allocator);
    }
    // Release heap text only when the high storage-mode bit is set.
    ~String73() {
        if (words[0] >> 31)
            appFreeUni__FPv((void*)words[2]);
    }
    // Borrow NUL-terminated text from inline or heap storage.
    const char* c_str() const {
        const char* text;
        if ((words[0] >> 31) == 0)
            text = (const char*)this + 1;
        else
            text = (const char*)words[2];
        return text;
    }
    // Replace a range through the original string/allocator calling convention.
    void replace(unsigned int pos, unsigned int size, const char* begin, const char* end) {
        Allocator73 allocator;
        ((void* (*)(String73*, unsigned int, unsigned int, const char*, const char*, Allocator73))String_ReplaceRange)(
            this, pos, size, begin, end, allocator);
    }
};
struct Item73 {
    char data[0x28];
    // Initialize one owned CBpTex icon entry.
    Item73() {
        ((CBpTex*)this)->__ct();
    }
    // Destroy one CBpTex entry while the array allocator retains its storage.
    ~Item73() {
        __dt__6CBpTexFv(this, -1);
    }
    // Allocate the original array size, including compiler-managed array metadata.
    static void* operator new[](size_t size) {
        return __nwa__FUl(size);
    }
};
#pragma opt_loop_invariants off
// Build caddie/accessory icon entries, retaining cached caddie entries when allowed.
// Entry point: CBuildProductCaddie::LoadIconGroups.
// caddieIndex is forwarded as the accessory provider selection, not an avatar index.
// Bounds: 0<=caddieIndex<CItemManager::caddies.size (table at inventory +0x68),
// NOT the character archive's 0..7 range. ItemInventory_GetCaddieIdAt in
// item_inventory_tables.cpp returns zero for negative/out-of-range indices; that zero goes to
// CShopProductCaddieItem through shop_product_list_builder.cpp, without a local failure check here.
// Do not promise an invalid index will return 0 from THIS loader. A displayed
// no-caddie choice is the appended group-0 record, not an archive/category index.
// Group 0=caddies (including the appended no-caddie record), group 1=accessories;
// use 0<=recordIndex<recordCounts[group], with category/group strictly 0..1.
// Record strides are 0x2C0; CBpTex entries 0x28. Name input must be terminated
// within its IFF icon field. The setter consumes/copies each temporary path
// before its String73 destructors run; no path pointer can be retained.
// Requires the two-group builder layout; return 0 only for a missing count table,
// 1 after texture entry setup (not GPU readiness). Partial rebuilds remain on failure.
// New texture arrays are owned by the builder; temporary path strings are borrowed
// only for the virtual set-path call. Replacing a group invalidates its old entries.
int CBuildProductCaddie::LoadIconGroups(int caddieIndex) {
    // Phase 1: build the two record groups and reject a missing count-table pointer.
    ShopCaddieList_BuildProducts(this, caddieIndex);
    if (AsCBuildProductCharaFields(this)->recordCountsAddress_s32 == 0) {
        return 0;
    }
    // Phase 2: invalidate group 1; retain group 0 unless first-load policy requires reset.
    void* resourceGroups = AsCBuildProductCharaFields(this)->resourceArrays_ptr;
    if (resourceGroups != 0) {
        void* previousIcons = ((void**)resourceGroups)[1];
        if (previousIcons != 0) {
            __destroy_new_array(previousIcons, (void*)__dt__6CBpTexFv);
            AsCBuildProductCharaFields(this)->resourceArrays_pp[1] = 0;
        }
        if (AsCBuildProductCharaFields(this)->flag != 0) {
            if (AsCBuildProductCharaFields(this)->textureEntriesBuilt == 0) {
                void* previousCaddieIcons = *AsCBuildProductCharaFields(this)->resourceArrays_pp;
                if (previousCaddieIcons != 0) {
                    __destroy_new_array(previousCaddieIcons, (void*)__dt__6CBpTexFv);
                    *AsCBuildProductCharaFields(this)->resourceArrays_pp = 0;
                }
            }
        }
    } else {
        void* p = __nwa__FUl(0x8);
        AsCBuildProductCharaFields(this)->resourceArrays_ptr = p;
        ((void**)p)[0] = 0;
        AsCBuildProductCharaFields(this)->resourceArrays_pp[1] = 0;
    }

    // Phase 3: allocate absent texture arrays and resolve each record's icon path.
    void* iconName;
    unsigned int iconNameLength;
    int groupPointerOffset;
    int recordByteOffset;
    int textureByteOffset;
    int recordGroup;
    int recordIndex;
    recordGroup = 0;
    groupPointerOffset = 0;
    for (; recordGroup < 2; recordGroup++) {
        void** textureGroups = AsCBuildProductCharaFields(this)->resourceArrays_pp;
        if (*(void**)((char*)textureGroups + groupPointerOffset) == 0) {
            void** recordCounts = AsCBuildProductCharaFields(this)->recordCounts_pp;
            int count = *(int*)((char*)recordCounts + groupPointerOffset);
            void* textureEntries = new Item73[count];
            recordIndex = 0;
            void** installedTextureGroups = AsCBuildProductCharaFields(this)->resourceArrays_pp;
            *(void**)((char*)installedTextureGroups + groupPointerOffset) = textureEntries;
            void** installedRecordCounts = AsCBuildProductCharaFields(this)->recordCounts_pp;
            int groupRecordCount = *(int*)((char*)installedRecordCounts + groupPointerOffset);
            if (groupRecordCount > 0) {
                recordByteOffset = 0;
                textureByteOffset = 0;
                for (; recordIndex < *(int*)(groupPointerOffset + AsCBuildProductCharaFields(this)->recordCounts_u32);
                     recordIndex++) {
                    unsigned int recordOrTextureBase = AsCBuildProductCharaFields(this)->recordArrays_u32;
                    recordOrTextureBase = *(unsigned int*)(groupPointerOffset + recordOrTextureBase);
                    iconName = ((IffItemCommon*)((char*)recordOrTextureBase + recordByteOffset))->icon;
                    {
                        String73 input((const char*)iconName, iconNameLength);
                        String73 result = ((String73 (*)(const String73&, const char*))String_ConstructConcatCStr)(
                            ((String73 (*)(const char*, const String73&))String_ConcatCStringAndString)(
                                caddieRecordStrings + kEmptyIconPrefix, input),
                            caddieRecordStrings + kIconResourceSuffix);
                        unsigned int textureTableAddress = AsCBuildProductCharaFields(this)->resourceArrays_u32;
                        recordOrTextureBase = *(unsigned int*)(groupPointerOffset + textureTableAddress);
                        void* textureEntry = (char*)recordOrTextureBase + textureByteOffset;
                        struct Vt {
                            virtual void a() DECOMP_LOCAL_VIRTUAL;
                            virtual void set(const char*) DECOMP_LOCAL_VIRTUAL;
                        };
                        ((Vt*)textureEntry)->set(result.c_str());
                        textureByteOffset += kTextureEntryStride;
                        input.replace(0, -1, 0, 0);
                        result.replace(0, -1, 0, 0);
                    }
                    recordByteOffset += kProductRecordStride;
                }
            }
        }
        groupPointerOffset += 4;
    }
    if (AsCBuildProductCharaFields(this)->textureEntriesBuilt == 0) {
        AsCBuildProductCharaFields(this)->textureEntriesBuilt = 1;
    }
    return 1;
}
#pragma pop

// Initialize the character-product angle conversion constant at startup.
void CBuildProductChara::InitAngleConstant() {
    s_characterProductAngleConstant = kFloat_6_2831855;
}

// Initialize the caddie-preview angle conversion constant at startup.
void CModelOperationCaddie_InitAngleConstant() {
    s_caddieModelOperationAngleConstant = kFloat_6_2831855;
}

// Initialize the item-preview angle conversion constant at startup.
void CModelOperationItem_InitAngleConstant() {
    s_itemModelOperationAngleConstant = kFloat_6_2831855;
}

// Advance the owned character texture archive; requires a previously opened archive.
// Returns 1 after polling, not an independent texture readiness status.
int CBuildProductChara::UpdateTextureArchiveLoad() {
    ((CPackTextureResource*)(textureArchive_ptr))->PollLoad();
    return 1;
}

// Select the preview resource for a club set or ball.
// The chosen packed-pool path is borrowed by the base open operation; returns 1.
// Preserve the isClubSet branch and path offset when replacing preview models.
int CModelOperationItem::OpenResource() {
    if (isClubSet) {
        ((CModelOperation*)this)->OpenResource(itemPreviewResourcePaths);
    } else {
        ((CModelOperation*)this)->OpenResource(itemPreviewResourcePaths + kBallPreviewResource);
    }
    return 1;
}

// Initialize the caddie model operation with no preview and no selected inventory index.
CModelOperationCaddie::CModelOperationCaddie() {
    ModelOperationBase_Ctor(this);
    vtbl = __vt__21CModelOperationCaddie;
    (*(int*)&asyncPet) = 0;
    inventoryIndex = -1;
}

// Initialize the item model operation and remember whether it displays club sets.
// self is required caller-owned storage; base construction supplies preview
// lifetime/cleanup (ModelOperationBase_Ctor, model_operation_chara_caddie.cpp). Accepted selector is
// 0=ball, 1=club set, as CPangya2Shop::Init's two ItemPreviewStorage calls show.
// It is narrowed to a byte WITHOUT Boolean normalization: 256 would become 0,
// while any nonzero retained byte selects club sets in OpenResource. Other ints
// are outside this API's two-choice input contract, even if a branch accepts them.
CModelOperationItem::CModelOperationItem(int isClubSet) {
    ModelOperationBase_Ctor(this);
    exhibitionPet = 0;
    vtbl = __vt__19CModelOperationItem;
    this->isClubSet = (unsigned char)isClubSet;
}

// Detach the item preview, destroy the base operation and optionally free the allocation.
CModelOperationItem::~CModelOperationItem() {
    exhibitionPet = 0;
    __dt__18ModelOperationBaseFv(this, 0);
}

// Initialize archive/provider ownership and allocate the eight borrowed archive-name slots.
// Constructor admission/ownership contract:
// - count is the base's parallel OUTPUT-GROUP slot count, not eight providers,
//   a character ID, or a product count. The character builder's supported caller
//   CPangya2Shop::Init (shop_products_tab_and_top_menu.cpp, kShopCharacterBuilderMode) passes 5. Keep count=5
//   with this class's hard-coded five record/texture groups. The common base
//   accepts a nonnegative allocation-sized count but does not validate it against
//   this derived class. Passing 2/3 belongs to the caddie/item builders instead.
// - flag is a stored byte, 0..255, forwarded unchanged to the category providers.
//   Shop supplies characterEntryMode; this file tests zero versus nonzero for
//   invalidated group-0 rebuilding (see the storage contract above). It does not
//   normalize the byte or define a new gameplay enum; provider constructors
//   __ct__18CShopProductAvatarFPvUc and the CShopProduct{Parts,Hair,Ring,SetItem} constructors
//   own filtering policy.
// - The base constructor CBuildProduct::CBuildProduct (archive_zip_reader_and_game_hooks.cpp) allocates/zeros
//   count indices and constructs its OWN embedded SaveData at +24. No external
//   save pointer is retained by this constructor. The base later owns category
//   providers, record tables and texture arrays; __dt__11ProductBaseFv releases
//   those. CBuildProductChara::Destroy first releases this derived class's archive,
//   baseProduct provider, owned 8-pointer name TABLE and extra set-record array.
//   Name strings stored in that table are borrowed pool data, never deleted.
// Required fresh 0x2D88-byte receiver. Allocation/constructor exceptions propagate;
// partially constructed state is not a usable builder. No load/readiness is
// promised on return; LoadProductTextures/BuildProducts provide those phases.
CBuildProductChara::CBuildProductChara(int count, unsigned char flag) {
    __ct__13CBuildProductFiUc(this, count, flag);
    void* self = this;
    AsCBuildProductCharaFields(self)->texture_s32 = 0;
    AsCBuildProductCharaFields(self)->vtbl = __vt__18CBuildProductChara;
    void* p = __nwa__FUl(0x20);
    AsCBuildProductCharaFields(self)->resourceNames_ptr = p;
    AsCBuildProductCharaFields(self)->baseProduct_s32 = 0;
    AsCBuildProductCharaFields(self)->extraRecords_s32 = 0;
    AsCBuildProductCharaFields(self)->extraRecordCount_s32 = -1;
}

// Replace the exhibition with a pet-style preview using the default viewport.
// Return 0 on viewport/release/create failure, 1 when created; a failed create
// leaves exhibitionPet/updateObject null after releasing the previous preview.
// The base owns destruction; updateObject aliases the stored exhibition, not a
// second allocation. kItemPreviewCameraDistance=10 becomes cameraDistance through
// CAsyncExhibitionPet::Create -> virtual slot 4 ->
// CAsyncExhibition_ReplaceExhibition (async_exhibition_avatar_preview.cpp). It is preview-scene camera distance, not
// degrees, seconds or screen pixels; conversion to physical meters is unknown.
int CModelOperationItem::CreateItemPreviewPet() {
    char buf[16];
    if (ModelOperationBase_GetDefaultViewport(this, buf) == 0) {
        return 0;
    }
    if (ModelOperationBase_ReleaseExhibition(this) == 0) {
        return 0;
    }
    void* p = CAsyncExhibitionPet::Create(buf, 1, kItemPreviewCameraDistance);
    (*(void**)&exhibitionPet) = p;
    updateObject = p;
    return p != 0;
}

// Apply the default camera preset before displaying the requested item selection.
// arg is a required selection for success and is borrowed by the selection API.
// Return 0 for null selection/preview or preset failure, otherwise forward setter
// status. No rollback of a successfully applied camera preset on setter failure.
int CModelOperationItem::SelectPreviewItem(void* arg) {
    if (exhibitionPet == 0) {
        return 0;
    }
    if (arg == 0) {
        return 0;
    }
    if (ModelOperationBase_ApplyCameraPreset(this, 0) != 0) {
        return ((CAsyncExhibitionPet*)((*(void**)&exhibitionPet)))->SetPetSelection(arg);
    }
    return 0;
}

// Find the icon texture matching an item id across non-avatar record groups.
// itemTypeId carries a 32-bit numeric ID in a void*; it is never dereferenced.
// Returns a borrowed CBpTex entry, or null for zero/not found. Caller must have
// built matching record/texture tables first; this does not test texture readiness.
// Rebuilding that group invalidates the returned address.
void* CBuildProductChara::FindResourceByTypeId(void* itemTypeId) {
    if (itemTypeId == 0) {
        return 0;
    }
    register int recordGroup = kFirstNonAvatarGroup;
    register int groupPointerOffset = kProductTableEntryBytes;
    register int recordByteOffset = 0;
    register int recordIndex = 0;
    for (; recordGroup < kFlattenedProductGroupCount; recordGroup++, groupPointerOffset += kProductTableEntryBytes) {
        int* recordTableWordView = recordCounts_s32ptr;
        int count = *(int*)((char*)recordTableWordView + groupPointerOffset);
        recordByteOffset = 0;
        recordIndex = 0;
        for (; recordIndex < count; recordIndex++, recordByteOffset += kProductRecordStride) {
            recordTableWordView = recordArrays_s32ptr;
            int recordGroupAddress = *(int*)((char*)recordTableWordView + groupPointerOffset);
            char* record = (char*)(recordGroupAddress + recordByteOffset);
            int recordTypeId = (int)AsShopProductRecord(record)->typeId;
            if ((unsigned int)itemTypeId == (unsigned int)recordTypeId) {
                void** textureGroups = resourceArrays_pp;
                int textureGroupOffset = recordGroup << 2;
                int textureEntryOffset = recordIndex * kTextureEntryStride;
                void* textureGroup = *(void**)((char*)textureGroups + textureGroupOffset);
                return (void*)((char*)textureGroup + textureEntryOffset);
            }
        }
    }
    return 0;
}

// Rotate the preview by pointer input or an automatic yaw, then tick its update object.
// Return 0 with no changes when updateObject is null; otherwise return 1 even
// when rotation helpers report failure. frameDelta is a finite nonnegative
// SECONDS step, forwarded unchanged to virtual slot 1. No clamp/max is imposed;
// large steps can cross animation events/loops, zero still advances auto yaw.
// Slot 1 is CAsyncExhibitionPet_UpdateAndInstallPendingPet (pet_preload_async.cpp), forwarding the delta to
// CAsyncExhibition_UpdateLoadingStateAndIndicator (async_exhibition_and_pet_format.cpp) and then
// CExhibition::UpdateLoadState. Loading/readiness can gate the model timeline.
// CExhibition_UpdateAnimationAndEffects (exhibition_model_preview.cpp) adds it to animationTime
// and subtracts it from animationDelay. These are seconds: MOTI range loader
// WPuppet_LoadMotionRangesFromChunk (wpuppet_model_bone_animation.cpp) divides 30-Hz frame endpoints
// by 30; AnimationEventParser_ProcessFrameRange (actor_animation_commands_and_attachments.cpp) converts seconds
// back to frames. The same preview update multiplies dt by 1000 for CFxBox's
// millisecond clock and opacity. Preserve these conversions; yaw is radians/call.
// Auto yaw remains 0.034906585 radians/call regardless of frameDelta. A frame rate
// mod must consider this separately from the model's animation/camera timeline.
// Preserve {zero,yaw,zero}, rotation-before-tick ordering and the shared receiver.
int CModelOperationItem::UpdateRotationAndExhibition(float frameDelta) {
    if ((*(int*)&this->updateObject) == 0) {
        return 0;
    }
    if (ModelOperationBase_IsInteractiveRotationEnabled(this) != 0) {
        ModelOperationBase_UpdatePointerRotation(this);
    } else {
        float rotationOffset[3];
        rotationOffset[0] = kFloat_0_ModelOperationItem;
        rotationOffset[1] = kItemPreviewAutoYawStepStorage;
        rotationOffset[2] = kFloat_0_ModelOperationItem;
        ModelOperationBase_OffsetRotation(this, rotationOffset);
    }
    void* updateObject = this->updateObject;
    struct Vt {
        virtual void a() DECOMP_LOCAL_VIRTUAL;
        virtual void d(float) DECOMP_LOCAL_VIRTUAL;
    };
    ((Vt*)updateObject)->d(frameDelta);
    return 1;
}

// Find the first costume set containing the requested item id and return that set id.
// Both itemTypeId and the return value are numeric IDs encoded as pointer bits;
// null means no containing set. Inventory lookup and itemCount are trusted.
// The copied set record supplies the set ID; inventory supplies its member list.
// No pointer into extraRecords escapes: the return is a copied 32-bit type ID,
// usable as a value after BuildProducts replaces/frees that array or the builder
// is destroyed. It carries no release operation. Rebuilding can change WHICH
// set contains the item, so re-query for current membership; the old ID is not
// a guarantee that the set is still offered. CPangya2Shop's callers (shop_product_selection_and_purchase.cpp,
// pangya2_shop_purchase.cpp) feed the value to ItemInventory_FindSetItemData, not to a destructor.
// During this synchronous search the builder's extraRecords and inventory must
// stay stable. Every copied set ID must resolve to an inventory sSetItem, whose
// itemCount is in 0..8 for its itemIds[8]; there is no null/length check here.
void* CBuildProductChara::FindSetContainingItem(void* itemTypeId) {
    int recordByteOffset = 0;
    int setIndex = 0;
    while (setIndex < extraRecordCount_s32) {
        IFF_STRUCT::sSetItem* memberWindow = (IFF_STRUCT::sSetItem*)ItemInventory_FindSetItemData(
            AsCSharedDoc(g_sharedDoc)->itemInventory,
            (void*)((IffItemCommon*)((char*)extraRecords_ptr + recordByteOffset))->typeId);
        int itemCount = memberWindow->itemCount;
        for (int itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
            // Sliding view: only itemIds[0] is read after the first iteration.
            // Advancing by one word exposes itemIds[itemIndex] at that offset;
            // the shifted address is not a second complete sSetItem record.
            if (itemTypeId == (void*)memberWindow->itemIds[0]) {
                return (void*)((IffItemCommon*)((char*)extraRecords_ptr + setIndex * kProductRecordStride))->typeId;
            }
            memberWindow = (IFF_STRUCT::sSetItem*)((char*)memberWindow + sizeof(unsigned int));
        }
        recordByteOffset += kProductRecordStride;
        setIndex++;
    }
    return 0;
}

// Release archive, product, name-array and set-record allocations before base destruction.
// Nullable self; flag>0 deletes the wrapper after owned resources. Return is the
// incoming address, potentially freed. Borrowed icon/record addresses expire here.
void* CBuildProductChara::Destroy(int flag) {
    if (this != 0) {
        void* textureArchive = textureArchive_ptr;
        vtbl = __vt__18CBuildProductChara;
        if (textureArchive != 0) {
            if (textureArchive != 0) {
                struct Vt {
                    virtual void c(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((Vt*)textureArchive)->c(1);
            }
            textureArchive_ptr = 0;
        }
        void* characterProvider = baseProduct_ptr;
        if (characterProvider != 0) {
            if (characterProvider != 0) {
                struct Vt {
                    virtual void c(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((Vt*)characterProvider)->c(1);
            }
            baseProduct_ptr = 0;
        }
        void* archiveNameTable = resourceNames_ptr;
        if (archiveNameTable != 0) {
            __dla__FPv(archiveNameTable);
            resourceNames_ptr = 0;
        }
        void* setRecords = extraRecords_ptr;
        if (setRecords != 0) {
            __dla__FPv(setRecords);
            extraRecords_ptr = 0;
        }
        __dt__11ProductBaseFv(this, 0);
        if (flag > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}

#pragma push
#pragma use_lmw_stmw on
struct Allocator81 {
    unsigned char value;
    // Preserve the allocator tag's original empty construction.
    Allocator81() {}
};

struct String81 {
    unsigned int words[3];
    // Reserve temporary string storage for construction by the string ABI.
    String81() {}
    // Copy a C string and return its length in the caller's scratch word.
    String81(const char* text, unsigned int& length, const Allocator81& allocator = Allocator81()) {
        words[0] = 0;
        words[1] = 0;
        words[2] = 0;
        length = strlen(text);
        String_ReserveCapacity(this, length);
        ((void* (*)(String81*, unsigned int, unsigned int, const char*, const char*, Allocator81))String_ReplaceRange)(
            this, 0, 0, text, text + length, allocator);
    }
    // Free only a heap-backed string; short strings live inside this object.
    ~String81() {
        if (words[0] >> 31)
            appFreeUni__FPv((void*)words[2]);
    }
    // Borrow NUL-terminated text from inline or heap storage.
    const char* c_str() const {
        const char* text;
        if ((words[0] >> 31) == 0) {
            text = (const char*)this + 1;
        } else {
            text = (const char*)words[2];
        }
        return text;
    }
    // Return character count, selecting the short-string or heap length representation.
    unsigned int size() const {
        if ((words[0] >> 31) == 0)
            return *(unsigned char*)this & 0x7F;
        return words[1];
    }
    // Replace a range through the original string/allocator calling convention.
    void replace(unsigned int pos, unsigned int size, const char* begin, const char* end) {
        replaceImpl(pos, size, begin, end);
    }
    // Forward replacement with this specialization's allocator tag.
    void replaceImpl(unsigned int pos, unsigned int size, const char* begin, const char* end) {
        Allocator81 allocator;
        ((void* (*)(String81*, unsigned int, unsigned int, const char*, const char*, Allocator81))String_ReplaceRange)(
            this, pos, size, begin, end, allocator);
    }
    // Replace the whole string and expose the old length through the scratch reference.
    void assign(const char* text, unsigned int& length) {
        assignWithAllocator(text, length, Allocator81());
    }
    // Assign C text through the allocator-aware string ABI.
    void assignWithAllocator(const char* text, unsigned int& length, const Allocator81& allocator) {
        length = size();
        ((void* (*)(String81*, unsigned int, unsigned int, const char*, const char*, Allocator81))String_ReplaceRange)(
            this, 0, length, text, text + strlen(text), allocator);
    }
};

struct Resource81 {
    char data[0x20];
    // Construct an owned archive loader from borrowed path and metadata views.
    Resource81(void* path, void* textureNames, void* textureCountWord) {
        ((CPackTextureResource*)this)->Init((const char*)path, (void**)textureNames, (unsigned int)textureCountWord);
    }
};

struct Item81 {
    char data[0x28];
    // Initialize one icon texture entry.
    Item81() {
        ((CBpTex*)this)->__ct();
    }
    // Destroy an entry without deleting its enclosing new[] allocation.
    ~Item81() {
        __dt__6CBpTexFv(this, -1);
    }
    // Allocate the original array size, including compiler-managed array metadata.
    static void* operator new[](size_t size) {
        return __nwa__FUl(size);
    }
};

#pragma opt_loop_invariants off
// Select the character fashion archive, rebuild product records and create lowercase icon texture paths.
// characterIndex is 0..7 (Ken..Kaz), unrelated to the eight CATEGORY indices.
// Returns 0 when product rebuilding fails, 1 after texture entries are set up;
// 1 does not guarantee the archive's asynchronous loading has completed.
// characterIndex must be 0..7 for the archive table AND accepted by the inventory
// provider. It is indexed before provider validation. Return 1 means entries were
// created, not that asynchronous archive I/O has completed; poll via UpdateTextureArchiveLoad.
// The combined valid range is 0<=characterIndex<min(8, inventory.characters.size);
// characters is CItemManager's table at +0x20. GetCharacterIdAt (item_inventory_tables.cpp) returns
// zero for an invalid inventory ordinal; the archive index is unchecked here.
// self owns the archive, provider instances, record copies and texture arrays.
// This rebuild is not transactional: failure may leave already-replaced resources.
// characterIndex denotes an INVENTORY ordinal, not a type ID or category index;
// accepted inventory bounds belong to CharacterProducts_CopyRecordForInventoryCharacter
// (shop_product_categories.cpp). This entry indexes the 8-name archive table BEFORE that validation.
// Categories 0..7 map to groups 0..4 as documented above; record/texture indices
// must be less than that group's signed count (never a provider-local index).
// Archive lifetime: self->textureArchive_ptr owns Resource81 until replacement/destruction.
// It borrows process-lifetime archive names and ResourceTable metadata while
// asynchronous load proceeds; keep those alive across UpdateTextureArchiveLoad polls. Do not
// destroy the pack loader merely because textureEntriesBuilt==1: textureEntriesBuilt means entries set up,
// not I/O complete. All group lookup results expire on the next rebuild.
int CBuildProductChara::LoadProductTextures(int characterIndex) {
    // Phase 1: select the character archive; names are borrowed from the packed string pool.
    char* archiveNames = (char*)fashionArchiveStrings;
    (resourceNames_strings)[0] = archiveNames + kScoutFashionArchive;
    (resourceNames_strings)[1] = archiveNames + kHanaFashionArchive;
    (resourceNames_strings)[2] = archiveNames + kUncleBobFashionArchive;
    (resourceNames_strings)[3] = archiveNames + kCeciliaFashionArchive;
    (resourceNames_strings)[4] = archiveNames + kMaxFashionArchive;
    (resourceNames_strings)[5] = archiveNames + kKoohFashionArchive;
    (resourceNames_strings)[6] = archiveNames + kArinFashionArchive;
    (resourceNames_strings)[7] = archiveNames + kKazFashionArchive;

    void* previousArchive = textureArchive_ptr;
    if (previousArchive != 0) {
        struct Vt {
            virtual ~Vt() DECOMP_LOCAL_VIRTUAL;
        };
        delete (Vt*)previousArchive;
        textureArchive_ptr = 0;
    }

    // Phase 2: locate the archive metadata and start its asynchronous load.
    unsigned int textureNamesAddress = 0;
    if (Sing_805DC620() == 0) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     &s_characterResourceTableDestructorRegistration);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }

    void* archiveNameTable = resourceNames_ptr;
    unsigned int characterNameOffset = (unsigned int)characterIndex << 2;
    void* archiveName = *(void**)((char*)archiveNameTable + characterNameOffset);
    void* textureCountWord = ResourceTable_FindEntryByName(Sing_805DC620(), archiveName, &textureNamesAddress);

    void* textureArchive = new Resource81(*(void**)((char*)resourceNames_ptr + characterNameOffset),
                                          (void*)textureNamesAddress, textureCountWord);
    textureArchive_ptr = textureArchive;
    ((CPackTextureResource*)textureArchive)->PollLoad();

    // Phase 3: rebuild providers and record copies before making parallel icon arrays.
    // On success all five record groups/counts exist; their order follows the map
    // beside CharacterProductCategory. Only now can parallel textures be allocated.
    int productsBuilt = BuildProducts(characterIndex);
    if (productsBuilt == 0)
        return 0;

    // Phase 4: free non-avatar icons; group 0 obeys the shared first-load cache policy.
    if (resourceArrays_ptr != 0) {
        int releaseGroupOffset;
        int remainingTextureGroups = 4;
        releaseGroupOffset = 0x10;
        do {
            void* textureGroups = resourceArrays_ptr;
            void* previousGroup = *(void**)((char*)textureGroups + releaseGroupOffset);
            if (previousGroup != 0) {
                __destroy_new_array(previousGroup, (void*)__dt__6CBpTexFv);
                void* reloadedTextureGroups = resourceArrays_ptr;
                *(void**)((char*)reloadedTextureGroups + releaseGroupOffset) = 0;
            }
            remainingTextureGroups--;
            releaseGroupOffset -= 4;
        } while (remainingTextureGroups > 0);

        if (flag) {
            if (!textureEntriesBuilt) {
                void* textureGroups = resourceArrays_ptr;
                void* previousFirstGroup = *(void**)textureGroups;
                if (previousFirstGroup != 0) {
                    __destroy_new_array(previousFirstGroup, (void*)__dt__6CBpTexFv);
                    void* reloadedTextureGroups = resourceArrays_ptr;
                    *(void**)reloadedTextureGroups = 0;
                }
            }
        }
    } else {
        void* newTextureGroups = __nwa__FUl(0x14);
        resourceArrays_ptr = newTextureGroups;
        *(void**)newTextureGroups = 0;
    }

    void* textureGroups = resourceArrays_ptr;
    ((void**)textureGroups)[1] = 0;
    textureGroups = resourceArrays_ptr;
    ((void**)textureGroups)[2] = 0;
    textureGroups = resourceArrays_ptr;
    ((void**)textureGroups)[3] = 0;
    textureGroups = resourceArrays_ptr;
    ((void**)textureGroups)[4] = 0;

    void* iconName;
    // Phase 5: fill missing groups with lowercase icon paths; counts match the record groups.
    // For each (group,index), read the record's icon name, build/lowercase its
    // path, and pass it to that texture entry. The setter receives a temporary
    // string and must consume/copy it before the two replace calls below.
    unsigned int resultLength;
    // Reused for the icon-name length, then the lowercase path address.
    // Keeping one scratch word preserves the original compiler lifetimes.
    unsigned int iconPathScratchWord;
    int groupPointerOffset;
    int recordByteOffset;
    int textureByteOffset;
    int recordGroup;
    int index;
    recordGroup = 0;
    groupPointerOffset = 0;
    for (; recordGroup < kFlattenedProductGroupCount; recordGroup++, groupPointerOffset += 4) {
        void* textureGroups = resourceArrays_ptr;
        if (*(void**)((char*)textureGroups + groupPointerOffset) != 0)
            continue;

        void* recordCounts = recordCounts_ptr;
        int count = *(int*)((char*)recordCounts + groupPointerOffset);
        void* textureEntries = new Item81[count];
        index = 0;
        void* installedTextureGroups = resourceArrays_ptr;
        *(void**)((char*)installedTextureGroups + groupPointerOffset) = textureEntries;
        void* installedRecordCounts = recordCounts_ptr;
        if (*(int*)((char*)installedRecordCounts + groupPointerOffset) <= 0)
            continue;

        recordByteOffset = 0;
        textureByteOffset = 0;
        for (; index < *(int*)(groupPointerOffset + recordCounts_u32); index++) {
            unsigned int recordOrTextureBase = recordArrays_u32;
            recordOrTextureBase = *(unsigned int*)(groupPointerOffset + recordOrTextureBase);
            iconName = ((IffItemCommon*)((char*)recordOrTextureBase + recordByteOffset))->icon;
            {
                String81 input((const char*)iconName, iconPathScratchWord);

                String81 result = ((String81 (*)(const String81&, const char*))String_ConstructConcatCStr)(
                    ((String81 (*)(const char*, const String81&))String_ConcatCStringAndString)(
                        fashionArchiveStrings + kEmptyTexturePrefix, input),
                    fashionArchiveStrings + kTextureFileExtension);
                iconPathScratchWord = (unsigned int)MSLString_LowercaseInPlace(result.c_str());
                result.assign((const char*)iconPathScratchWord, resultLength);
                unsigned int textureGroupTableAddress = resourceArrays_u32;
                recordOrTextureBase = *(unsigned int*)(groupPointerOffset + textureGroupTableAddress);
                void* textureEntry = (char*)recordOrTextureBase + textureByteOffset;
                struct Vt {
                    virtual void textureGroups() DECOMP_LOCAL_VIRTUAL;
                    virtual void set(const char*) DECOMP_LOCAL_VIRTUAL;
                };
                ((Vt*)textureEntry)->set(result.c_str());

                textureByteOffset += kTextureEntryStride;
                input.replace(0, -1, 0, 0);
                result.replace(0, -1, 0, 0);
            }
            recordByteOffset += kProductRecordStride;
        }
    }

    if (!textureEntriesBuilt) {
        textureEntriesBuilt = 1;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_loop_invariants off
extern unsigned char __vt__Q23std11logic_error[];
extern unsigned char s_sharedCharDeleterVtable[];
extern unsigned char __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv(void*, int);

struct Ref77 {
    void* vtable;
    int refs;
    int owners;
    char* text;
    // Adopt the exception message in a shared deleter record.
    Ref77(char* p) {
        refs = 1;
        owners = 1;
        vtable = s_sharedCharDeleterVtable;
        text = p;
    }
};
struct Guard77 {
    char* p;
    void* allocator;
    // Own the exception text until the shared reference record takes responsibility.
    Guard77(char* data, void* a) : p(data), allocator(a) {}
    // Release a message allocation that was not transferred.
    ~Guard77() {
        if (p)
            __dla__FPv(p);
    }
};
struct Allocator77 {
    unsigned char value;
    char pad[3];
};
struct ErrorBase77 : std::exception {
    // Install the exception ABI vtable without altering its storage layout.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    char* text;
    Ref77* ref;
    // Copy the exception message and create its shared ownership record.
    ErrorBase77(const char* const& s) {
        SetTable(__vt__Q23std11logic_error);
        Allocator77 allocator;
        allocator.value = 0;
        unsigned int n = strlen(s) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        text = buffer;
        {
            Guard77 guard(buffer, &allocator);
            ref = new Ref77(buffer);
            guard.p = 0;
        }
        strcpy(text, s);
    }
};
namespace std {
struct length_error : ErrorBase77 {
    // Construct the owned message and specialize the exception vtable.
    length_error(const char* const& s) : ErrorBase77(s) {
        SetTable(__vt__Q23std12length_error);
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
// Return the larger capacity increment by reference.
inline const unsigned int& maximum77(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Apply the original pointer-vector growth rule and throw on allocator overflow.
inline unsigned int recommend77(unsigned int capacity, unsigned int extra) {
    if (extra > 0x3FFFFFFF - capacity) {
        {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct(fashionArchiveStrings);
            exception_runtime::__throw(exception_runtime::fashionArchiveStrings + kVectorLengthErrorType, error,
                                       (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
    }
    if (capacity < 0x15555555) {
        unsigned int amount = ((capacity + 1) * 4 - (capacity + 1)) / 5;
        return capacity + maximum77(amount, extra);
    } else if (capacity < 0x2AAAAAAA) {
        unsigned int amount = (capacity + 1) / 2;
        return capacity + maximum77(amount, extra);
    }
    return 0x3FFFFFFF;
}
struct CapacityLimit77 {
    // Reject a pointer-vector capacity above the allocator's element limit.
    CapacityLimit77(unsigned int capacity) {
        if (capacity > 0x3FFFFFFF) {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct(fashionArchiveStrings);
            exception_runtime::__throw(exception_runtime::fashionArchiveStrings + kVectorLengthErrorType, error,
                                       (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
    }
};
struct Growth77 {
    void** data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* sourceCapacity;
    unsigned int offset;
    // Allocate a temporary pointer-vector buffer according to the growth rule.
    Growth77(const unsigned int& extra, unsigned int* c) {
        unsigned int zero = 0;
        data = (void**)zero;
        size = zero;
        capacity = zero;
        sourceCapacity = c;
        offset = zero;
        unsigned int allocatedCapacity = recommend77(*c, extra);
        CapacityLimit77 limit(allocatedCapacity);
        data = (void**)appMallocDummy__FUl(allocatedCapacity * 4);
        capacity = allocatedCapacity;
    }
    // Free the buffer left here after exchanging ownership with the live vector.
    ~Growth77() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
struct CapacityCheck77 {
    struct Check {
        // Evaluate capacity growth solely to preserve its overflow exception.
        Check(unsigned int capacity, unsigned int extra) {
            recommend77(capacity, extra);
        }
    };
    // Validate the proposed capacity increase before changing live storage.
    CapacityCheck77(unsigned int capacity, unsigned int extra) {
        Check check(capacity, extra);
    }
};
// Typed category-provider ABI. Receiver is a constructed provider owned by the
// builder. The prefix slots are +08 destroy/delete and +0C (unused here).
// +10 copies ONE category record into required nonaliasing 0x2C0 slot storage;
// index must be 0<=index<recordCount(). Output is a value copy, not an owner.
// The category determines payload size; unused slot bytes need not be initialized.
// +14 returns the provider-local record count, required nonnegative for allocation.
// Group 4 rebases flattened index to accessory i / hair i-A / ring i-A-H;
// costume-set records are separate. No virtual here takes a texture entry index.
struct ProductRecordProvider {
    virtual void destroy(int);
    virtual void unused();
    virtual void copyRecord(void*, int);
    virtual int recordCount();
};
// Allocation-sized wrappers retain the original constructor/destructor exception paths.
struct AvatarProductProvider {
    char data[0x14];
    // Construct an avatar provider in the original 0x14-byte allocation.
    AvatarProductProvider(void* p, unsigned char b) {
        __ct__18CShopProductAvatarFPvUc(this, p, b);
    }
};
struct PartsProductProvider {
    char data[0x134];
    // Construct a parts provider filtered by the requested part mask.
    PartsProductProvider(void* h, int type, void* p, unsigned char b) {
        __ct__17CShopProductPartsFPCUiUiPvPv(this, h, type, p, b);
    }
};
struct HairProductProvider {
    char data[0x130];
    // Construct a hair provider using the selected character record.
    HairProductProvider(void* h, void* p, unsigned char b) {
        __ct__16CShopProductHairFPvPvPv(this, h, p, b);
    }
};
struct RingProductProvider {
    char data[0x14];
    // Construct the ring provider from the embedded save-data view.
    RingProductProvider(void* p, unsigned char b) {
        __ct__16CShopProductRingFPvPv(this, p, b);
    }
};
struct SetProductProvider {
    char data[0x130];
    // Construct the selected character's costume-set provider.
    SetProductProvider(void* h, void* p, unsigned char b) {
        __ct__19CShopProductSetItemFPvPvPv(this, h, p, b);
    }
};

#pragma opt_common_subs off
// Build eight category providers, copy the selected character and flatten category records into display groups.
// characterIndex is an inventory ORDINAL: 0<=index<CItemManager::characters.size,
// table at inventory+0x20 (ItemInventory_GetCharacterIdAt, item_inventory_tables.cpp).
// It is not a type ID, category or fixed 0..7 archive selector. Lookup maps invalid
// indices to typeId 0; CopyRecord searches the provider for that ID and returns 0
// if absent (no guarantee for malformed data containing ID 0). Require a loaded
// inventory and a valid ordinal for a usable character; this function does not
// validate it before clearing the old providers. The fashion archive path's
// separate eight-character mapping must also support any newly added character.
// return 0 after a clear/provider/lookup failure, retaining prior mutations. Return
// 1 publishes five parallel record groups plus separate costume-set records.
// Record copies/providers are owned; inventory records used to populate them are
// borrowed. Preserve provider-local indices when flattening accessories/hair/rings.
int CBuildProductChara::BuildProducts(int characterIndex) {
    unsigned int missingCategoryCount;
    int recordByteOffset;
    int innerOffset;
    int lastOffset;
    int groupPointerOffset;
    int remainingGroups;
    int subtotal;
    void* saveData;
    int categoryPointerOffset;
    int recordGroup;
    char* groupPointerOffsetBytes;
    int groupRecordIndex;
    int recordIndex;
    int lastIndex;
    // Phase 1: discard old providers and obtain the selected character record.
    if (!BuildProductChara_ClearProductEntries(this))
        return 0;

    void* oldCharacterProvider = baseProduct_ptr;
    if (oldCharacterProvider != 0) {
        if (oldCharacterProvider != 0)
            ((ProductRecordProvider*)oldCharacterProvider)->destroy(1);
        baseProduct_ptr = 0;
    }

    saveData = (char*)this + kEmbeddedSaveDataOffset;
    void* characterProvider = new AvatarProductProvider(saveData, flag);
    baseProduct_ptr = characterProvider;
    if (characterProvider == 0)
        return 0;

    char characterRecordStorage[sizeof(IFF_STRUCT::sChar) + sizeof(unsigned int)];
    if (!CharacterProducts_CopyRecordForInventoryCharacter(characterProvider, characterRecordStorage, characterIndex))
        return 0;

    // Phase 2: resize the provider-pointer vector to exactly eight entries.
    // Growth ownership/publication ledger (Growth77 destructor frees ONLY its
    // buffer via appFreeUni, never providers): in-place growth zeroes new slots
    // before publishing count. Allocating growth first validates capacity, then
    // temporary owns the new buffer; live this retains old buffer/count/capacity.
    // A capacity/allocation throw leaves that live vector unchanged, but phase 1
    // has ALREADY cleared providers and published baseProduct. No whole-build
    // rollback exists. appMallocDummy must yield usable storage or throw/abort.
    // Once allocated, the tail is zeroed, old pointer words copied and old slots
    // zeroed. memcpy/memset and the following word assignments do not throw.
    // The swap publishes capacity/buffer/count; Growth77 then owns/frees the OLD
    // buffer with size=0. It never frees the newly published buffer. Phase 3
    // installs each provider separately, so a later constructor throw leaves
    // earlier providers owned in this for ClearProductEntries/destruction.
    unsigned int oldCount = productCount;
    if (oldCount < kProductCategoryCount) {
        unsigned int cap = productCapacity;
        missingCategoryCount = kProductCategoryCount - oldCount;
        // Original 0x802D7340 unit: preserve the growth path before common setup.
        do {
            if (missingCategoryCount > cap || oldCount > cap - missingCategoryCount) {
                CapacityCheck77 check(productCapacity, productCount + missingCategoryCount - cap);
            } else {
                memset((char*)products_ptr + oldCount * 4, 0, missingCategoryCount * 4);
                productCount += missingCategoryCount;
                break;
            }
            {
                int oldSize;
                void** oldBuf;
                unsigned int extra = productCount + missingCategoryCount - productCapacity;
                Growth77 growth(extra, &productCapacity);
                growth.offset = productCount;
                memset(growth.data + growth.offset + growth.size, 0, missingCategoryCount * 4);
                oldBuf = products_pp;
                void** oldEnd = oldBuf + productCount;
                growth.size += missingCategoryCount;
                oldSize = (int)((char*)oldEnd - (char*)oldBuf) / 4;
                growth.offset -= oldSize;
                unsigned int oldBytes = oldSize * 4;
                memcpy(growth.data + growth.offset, oldBuf, oldBytes);
                memset(oldBuf, 0, oldBytes);
                growth.size += oldSize;
                void** newBuf = growth.data;
                unsigned int oldCapacity = productCapacity;
                productCount = 0;
                growth.data = products_pp;
                productCapacity = growth.capacity;
                growth.capacity = oldCapacity;
                products_pp = newBuf;
                productCount = growth.size;
                growth.size = 0;
            }
        } while (0);
    } else if (oldCount > kProductCategoryCount) {
        productCount -= oldCount - kProductCategoryCount;
    }

    // Phase 3: install providers in protocol order: avatar/top/bottom/shoes/accessory/hair/ring/set.
    void* categoryProviders;
    void* newProvider;

    newProvider = new AvatarProductProvider(saveData, flag);
    categoryProviders = products_ptr;
    ((void**)categoryProviders)[kAvatarCategory] = newProvider;

    newProvider = new PartsProductProvider(characterRecordStorage, kTopPartMask, saveData, flag);
    categoryProviders = products_ptr;
    ((void**)categoryProviders)[kTopCategory] = newProvider;

    newProvider = new PartsProductProvider(characterRecordStorage, kBottomPartMask, saveData, flag);
    categoryProviders = products_ptr;
    ((void**)categoryProviders)[kBottomCategory] = newProvider;

    newProvider = new PartsProductProvider(characterRecordStorage, kShoePartMask, saveData, flag);
    categoryProviders = products_ptr;
    ((void**)categoryProviders)[kShoesCategory] = newProvider;

    newProvider = new PartsProductProvider(characterRecordStorage, kAccessoryPartMask, saveData, flag);
    categoryProviders = products_ptr;
    ((void**)categoryProviders)[kAccessoriesCategory] = newProvider;

    newProvider = new HairProductProvider(characterRecordStorage, saveData, flag);
    categoryProviders = products_ptr;
    ((void**)categoryProviders)[kHairCategory] = newProvider;

    newProvider = new RingProductProvider(saveData, flag);
    categoryProviders = products_ptr;
    ((void**)categoryProviders)[kRingsCategory] = newProvider;

    newProvider = new SetProductProvider(characterRecordStorage, saveData, flag);
    categoryProviders = products_ptr;
    ((void**)categoryProviders)[kCostumeSetsCategory] = newProvider;

    // Phase 4: release previous flattened records, retaining avatar records under cache policy.
    if (recordArrays_ptr != 0) {
        remainingGroups = 4;
        groupPointerOffset = 0x10;
        do {
            void* recordGroups = recordArrays_ptr;
            void* previousGroup = *(void**)((char*)recordGroups + groupPointerOffset);
            if (previousGroup != 0) {
                __dla__FPv(previousGroup);
                void* reloadedRecordGroups = recordArrays_ptr;
                *(void**)((char*)reloadedRecordGroups + groupPointerOffset) = 0;
            }
            remainingGroups--;
            groupPointerOffset -= 4;
        } while (remainingGroups > 0);

        if (flag != 0) {
            if (textureEntriesBuilt == 0) {
                void* recordGroups = recordArrays_ptr;
                void* previousFirstGroup = *(void**)recordGroups;
                if (previousFirstGroup != 0) {
                    __dla__FPv(previousFirstGroup);
                    void* reloadedRecordGroups = recordArrays_ptr;
                    *(void**)reloadedRecordGroups = 0;
                }
            }
        }
    } else {
        void* newRecordGroups = __nwa__FUl(0x14);
        recordArrays_ptr = newRecordGroups;
        *(void**)newRecordGroups = 0;
    }

    void* previous = extraRecords_ptr;
    if (previous != 0) {
        __dla__FPv(previous);
        extraRecords_ptr = 0;
    }

    void* recordGroups = recordArrays_ptr;
    ((void**)recordGroups)[1] = 0;
    recordGroups = recordArrays_ptr;
    ((void**)recordGroups)[2] = 0;
    recordGroups = recordArrays_ptr;
    ((void**)recordGroups)[3] = 0;
    recordGroups = recordArrays_ptr;
    ((void**)recordGroups)[4] = 0;

    // Phase 5: groups 0..3 copy one provider each; existing group 0 may remain cached.
    categoryPointerOffset = 0;
    recordGroup = 0;
    // Three independent cursors: recordGroup is a logical output group;
    // categoryPointerOffset is provider-table bytes; groupPointerOffsetBytes
    // is OUTPUT table bytes encoded as char* (never dereference it alone).
    // They advance together only for the one-provider groups 0..3. Phase 6
    // switches to provider-local indices while filling combined output group 4.
    groupPointerOffsetBytes = 0;
    for (; recordGroup < kSingleProviderGroupCount;
         recordGroup++, categoryPointerOffset += 4, groupPointerOffsetBytes += 4) {
        void* destinationRecordGroups = recordArrays_ptr;
        if (*(void**)((char*)destinationRecordGroups + (unsigned int)groupPointerOffsetBytes) == 0) {
            void* providerTable = products_ptr;
            void* provider = *(void**)((char*)providerTable + categoryPointerOffset);
            int count = ((ProductRecordProvider*)provider)->recordCount();

            void* recordCounts = recordCounts_ptr;
            *(int*)((char*)recordCounts + (unsigned int)groupPointerOffsetBytes) = count;
            void* recordBuffer = __nwa__FUl(*(int*)((char*)recordCounts_ptr + (unsigned int)groupPointerOffsetBytes) *
                                            kProductRecordStride);
            void* installedRecordGroups = recordArrays_ptr;
            *(void**)((char*)installedRecordGroups + (unsigned int)groupPointerOffsetBytes) = recordBuffer;

            groupRecordIndex = 0;
            innerOffset = 0;
            for (; groupRecordIndex < *(int*)(groupPointerOffsetBytes + (unsigned int)recordCounts_ptr);
                 groupRecordIndex++) {
                void* sourceProviders = products_ptr;
                void* sourceProvider = *(void**)((char*)sourceProviders + categoryPointerOffset);
                void* base = *(void**)(groupPointerOffsetBytes + (unsigned int)recordArrays_ptr);
                ((ProductRecordProvider*)sourceProvider)->copyRecord((char*)base + innerOffset, groupRecordIndex);
                innerOffset += kProductRecordStride;
            }
        }
    }

    void* providerTable = products_ptr;
    // Phase 6: group 4 concatenates accessories, hair, rings in that order.
    void* accessoryProvider = ((void**)providerTable)[kAccessoriesCategory]; // index 4
    int accessoryCount = ((ProductRecordProvider*)accessoryProvider)->recordCount();

    providerTable = products_ptr;
    void* hairProvider = ((void**)providerTable)[kHairCategory]; // index 5
    int hairCount = ((ProductRecordProvider*)hairProvider)->recordCount();

    providerTable = products_ptr;
    void* ringProvider = ((void**)providerTable)[kRingsCategory]; // index 6
    int ringCount = ((ProductRecordProvider*)ringProvider)->recordCount();

    subtotal = accessoryCount + hairCount;
    void* recordCounts = recordCounts_ptr;
    ((int*)recordCounts)[kAccessoryRecordGroup] = ringCount + subtotal;
    void* accessoryRecords = __nwa__FUl(recordCounts_s32ptr[kAccessoryRecordGroup] * kProductRecordStride);
    void* destinationRecordGroups = recordArrays_ptr;
    ((void**)destinationRecordGroups)[kAccessoryRecordGroup] = accessoryRecords;

    recordIndex = 0;
    recordByteOffset = 0;
    for (; recordIndex < recordCounts_s32ptr[kAccessoryRecordGroup]; recordIndex++) {
        if (recordIndex < accessoryCount) {
            void* categoryProviders = products_ptr;
            void* sourceProvider = ((void**)categoryProviders)[kAccessoriesCategory];
            void* destinationRecord = (char*)((void**)recordArrays_ptr)[kAccessoryRecordGroup] + recordByteOffset;
            ((ProductRecordProvider*)sourceProvider)->copyRecord(destinationRecord, recordIndex);
        } else if (recordIndex < subtotal) {
            void* categoryProviders = products_ptr;
            void* sourceProvider = ((void**)categoryProviders)[kHairCategory];
            void* destinationRecord = (char*)((void**)recordArrays_ptr)[kAccessoryRecordGroup] + recordByteOffset;
            ((ProductRecordProvider*)sourceProvider)->copyRecord(destinationRecord, recordIndex - accessoryCount);
        } else {
            void* categoryProviders = products_ptr;
            void* sourceProvider = ((void**)categoryProviders)[kRingsCategory];
            void* destinationRecord = (char*)((void**)recordArrays_ptr)[kAccessoryRecordGroup] + recordByteOffset;
            ((ProductRecordProvider*)sourceProvider)
                ->copyRecord(destinationRecord, recordIndex - accessoryCount - hairCount);
        }
        recordByteOffset += kProductRecordStride;
    }

    providerTable = products_ptr;
    // Phase 7: copy costume sets separately; these do not participate in icon groups 0..4.
    void* setProvider = ((void**)providerTable)[kCostumeSetsCategory];
    int setCount = ((ProductRecordProvider*)setProvider)->recordCount();
    extraRecordCount_u32 = setCount;

    void* setRecords = __nwa__FUl(setCount * kProductRecordStride);
    extraRecords_ptr = setRecords;

    lastIndex = 0;
    lastOffset = 0;
    for (; lastIndex < extraRecordCount_s32; lastIndex++) {
        void* categoryProviders = products_ptr;
        void* sourceProvider = ((void**)categoryProviders)[kCostumeSetsCategory];
        void* destinationRecord = (char*)extraRecords_ptr + lastOffset;
        ((ProductRecordProvider*)sourceProvider)->copyRecord(destinationRecord, lastIndex);
        lastOffset += kProductRecordStride;
    }

    return 1;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462514", _ctors_u802D7340_0, CBuildProductChara::InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462514", _ctors_u802D7340_1, CModelOperationCaddie_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462514", _ctors_u802D7340_2, CModelOperationItem_InitAngleConstant);
#pragma pop
