#include <decomp/static_init.h>
enum { kHairRecordCopyBias = 0x10, kHairConfigurationPairedWords = 0x46 };
// Base product prefix shared by these shop categories; ids use the original pointer-valued C ABI.
struct ShopProductListView {
    void* vtable;
    void* saveData;
    void** records;
    unsigned int recordCount;
};
struct CaddieItemProductView {
    ShopProductListView products;
    unsigned int capacity;
    unsigned int caddieTypeId;
};
enum ShopItemIdBits {
    kItemSerialMask = 0x03FFFFFF,
    kCaddieItemOwnerShift = 21,
    kCaddieItemOwnerMask = 0x1F,
    kHairStyleCount = 0x33,
    kOwnershipWordShift = 5,
    kOwnershipBitMask = 31,
    kHairInitiallyUnlockedShift = 5,
    kHairInitiallyUnlockedMask = 7
};
enum SavedItemFlagExtraction {
    kEquippedFlagToSignBit = 25, // byte bit 6: equipped/current item
    kOwnedFlagToSignBit = 24,    // byte bit 7: owned/unlocked item
    kTwoHighBits = 0xC0000000
};
// Preserve the two-step shift/mask and signed shift: CodeWarrior emits different code for a direct bool mask.
#include "classes/SavePlayerRecord.h" // CShopProductHair_CheckUnlockAndOwnership: slot ownership bits at +0x114
#include "classes/CSharedDoc.h"
#include "classes/ShopProductRecord.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
// Translation unit 0x80237E24-0x80238E9C (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct ObjCtorVt {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void init(void* arg);
};

#include "game/shop_product.h"
#include "classes/CShopProductHair.h"

#define AsCShopProductHair(p) ((CShopProductHair*)(p))

extern "C" {

extern float kFloat_6_2831855;
extern float s_shopBallAngleConstant;
extern float s_shopCaddieAngleConstant;
extern float s_shopCaddieItemAngleConstant;
extern float s_shopClubSetAngleConstant;
extern float s_shopEquipItemAngleConstant;
extern float s_shopAvatarAngleConstant;
extern float s_shopHairAngleConstant;

struct Fn80238B40Arg {
    unsigned int vtableWord;
    void* saveData; // Save-data object.
};

struct Fn80238B40Arg2 {
    char padding[0x50];
    void* typeId; // Numeric item type ID carried in a pointer-sized ABI word.
};

int SaveData_IsConsumableAvailable(void*, void*, int);
void __dt__10CShopBaseAFv(void*, int);
void __dt__10CShopBaseBFv(void*, int);
void __dt__10CShopBaseCFv(void*, int);
void __dt__10CShopBaseDFv(void*, int);
void __dt__10CShopBaseEFv(void*, int);
void ShopHairProductList_dtor(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*);
void __dt__Q33std6detail16char_array_guardFv(void*, int);
void __ct__10CShopBaseAFPvPv(void*, void*, void*);
void __ct__10CShopBaseBFPvPv(void*, void*, void*);
void __ct__10CShopBaseDFPv(void*, void*);
void __ct__10CShopBaseEFPv(void*, void*);
void __ct__10CShopBaseCFPv(void*, void*);
void ShopHairPointerArray_Initialize(void*, void*);
int SaveData_CopyBallRecord(void*, void*, void*);
int SaveData_CopyCaddieRecord(void*, void*, void*);
int SaveData_CopyClubSetRecord(void*, void*, void*);
int SaveData_TestCaddieItemFlag(void*, void*, int);
int ItemInventory_FindCharacterIndex(void*, void*);
int ItemInventory_FindHairStyleIndex(void*, void*);
void* SaveData_GetCurrentRecord(void*);
int SaveData_CopyCharacterEquipmentRecord(void*, unsigned char*, void*);
void* ItemInventory_GetCharacterIdAt(void*, void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
void* __nw__FUl(unsigned int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* __nwa__FUl(unsigned int);
void __throw(const char*, void*, void (*)(void*)) __attribute__((noreturn));
void __dt__Q23std12out_of_rangeFv(void*);
extern void* g_sharedDoc;
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12out_of_range[];
extern char shopHairOwnershipBoundsError[], kShopHairOutOfRangeExceptionType[];
extern char __vt__16CShopProductBall[0x24];
extern char __vt__19CShopProductClubSet[0x24];
extern char __vt__21CShopProductEquipItem[0x24];
extern char __vt__18CShopProductCaddie[0x24];
extern char __vt__22CShopProductCaddieItem[0x24];
extern char s_shopProductInterfaceVtable[0x20];
extern char __vt__16CShopProductHair[0x24];

struct Fn80237E24Vtable {
    char padding[0x1C];
    void (*init)(void*, void*);
};

struct Fn80238854Vtable {
    char padding[0x1C];
    void (*init)(void*, void*);
};

// Destroy category resources and free the product only for a positive deleting-destructor flag.
void* CShopProductHair_dtor(void* self, int shouldDelete) {
    if (self) {
        ShopHairProductList_dtor(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

class InterfaceVirtual { public: 
virtual void v08(); virtual void v0C(int);
};
// Restore the interface vtable, dispatch its cleanup hook and optionally free the object.
void* CShopProductInterface_dtor(void* self, int shouldDelete) {
    if (self) {
        *(void**)self = &s_shopProductInterfaceVtable;
        ((InterfaceVirtual*)self)->v0C(shouldDelete);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Copy the full character record for an inventory index; null output, empty lists or missing ids fail.
int CharacterProducts_CopyRecordForInventoryCharacter(void* self, void* characterRecordOut, void* characterIndex) {
    if (characterRecordOut == 0)
        return 0;
    if ((void*)((ShopProductListView*)self)->recordCount == 0)
        return 0;
    if (((ShopProductListView*)self)->saveData == 0)
        return 0;
    void* characterTypeId = ItemInventory_GetCharacterIdAt(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), characterIndex);
    void** entries = ((ShopProductListView*)self)->records;
    void** end;
    for (void** entry = entries; end = entries + ((ShopProductListView*)self)->recordCount, !(end == entry); ++entry) {
        void* source = *entry;
        if (characterTypeId == (void*)((ShopProductRecord*)source)->typeId) { // 0x80237E24: product +0x50
            struct Pair { unsigned char b0; unsigned char b1; };
            struct P40 { Pair e[0x14]; };
            struct UObj {
                unsigned int commonWords[0x16];
                unsigned int commonTailWord;
                P40 model;
                P40 hairTexture;
                P40 baseTexture;
                P40 faceTexture;
                unsigned int power, control, accuracy, spin, curve;
                float modelScale;
                unsigned char defaultPartCount, reservedCharacterByte, basePower, baseControl, baseAccuracy, baseSpin, baseCurve, reservedTail;
            };
            *(UObj*)characterRecordOut = *(UObj*)source;
            return 1;
        }
    }
    return 0;
}

class V1C8854 {
    public: 
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C(void*);
}
;
struct CShopBaseC {
    CShopBaseC(void*);
    ~CShopBaseC();
    void** vtable;
    void v1C(void* p) { ((ObjCtorVt*)this)->init(p); }
};
struct CShopProductCaddieItem : CShopBaseC {
    char pad[0x10]; void* caddieTypeId;
    CShopProductCaddieItem(void* a, void* b, void* c);
};
// Remember the caddie type ID in the original pointer-sized ABI slot before initializing its products.
CShopProductCaddieItem::CShopProductCaddieItem(void* caddieTypeIdValue, void* b, void* c) : CShopBaseC(b) {
    vtable = (void**)__vt__22CShopProductCaddieItem;
    caddieTypeId = caddieTypeIdValue;
    v1C(c);
}

// Test the saved owned flag, or the equipped flag when testEquipped is set; missing save records fail.
int CShopProductBall_vfunc_6(void* self, void* productRecord, int testEquipped) {
    volatile unsigned char savedFlags;
    if (SaveData_CopyBallRecord(((ShopProductListView*)self)->saveData, (unsigned char*)&savedFlags, (void*)((ShopProductRecord*)productRecord)->typeId) == 0)
        return 0;
    if (testEquipped) {
        unsigned int flagInSignBit = (unsigned int)savedFlags << kEquippedFlagToSignBit;
        flagInSignBit &= kTwoHighBits;
        return ((int)flagInSignBit >> 31) != 0;
    } else {
        unsigned int flagInSignBit = (unsigned int)savedFlags << kOwnedFlagToSignBit;
        flagInSignBit &= kTwoHighBits;
        return ((int)flagInSignBit >> 31) != 0;
    }
}
// Test the saved owned flag, or the equipped flag when testEquipped is set; missing save records fail.
int CShopProductCaddie_IsOwnedOrEquipped(void* self, void* productRecord, int testEquipped) {
    volatile unsigned char savedFlags[8];
    if (SaveData_CopyCaddieRecord(((ShopProductListView*)self)->saveData, (unsigned char*)savedFlags, (void*)((ShopProductRecord*)productRecord)->typeId) == 0)
        return 0;
    if (testEquipped) {
        unsigned int flagInSignBit = (unsigned int)savedFlags[0] << kEquippedFlagToSignBit;
        flagInSignBit &= kTwoHighBits;
        return ((int)flagInSignBit >> 31) != 0;
    } else {
        unsigned int flagInSignBit = (unsigned int)savedFlags[0] << kOwnedFlagToSignBit;
        flagInSignBit &= kTwoHighBits;
        return ((int)flagInSignBit >> 31) != 0;
    }
}

// Test the saved owned flag, or the equipped flag when testEquipped is set; missing save records fail.
int CShopProductClubSet_vfunc_6(void* self, void* productRecord, int testEquipped) {
    volatile unsigned char savedFlags[2];
    if (SaveData_CopyClubSetRecord(((ShopProductListView*)self)->saveData, (unsigned char*)savedFlags, (void*)((ShopProductRecord*)productRecord)->typeId) == 0)
        return 0;
    if (testEquipped) {
        unsigned int flagInSignBit = (unsigned int)savedFlags[0] << kEquippedFlagToSignBit;
        flagInSignBit &= kTwoHighBits;
        return ((int)flagInSignBit >> 31) != 0;
    } else {
        unsigned int flagInSignBit = (unsigned int)savedFlags[0] << kOwnedFlagToSignBit;
        flagInSignBit &= kTwoHighBits;
        return ((int)flagInSignBit >> 31) != 0;
    }
}

class HairVt {
    public: 
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C(void*);
}
;
// Construct the hair product and copy its 71-word configuration before category initialization.
void* CShopProductHair_Ctor(void* self, void* source, void* baseArg, void* initArg) {
    unsigned int* destinationWords;
    unsigned int* sourceWords;
    int i;
    ShopHairPointerArray_Initialize(self, baseArg);
    *(void**)self = &__vt__16CShopProductHair;
    destinationWords = (unsigned int*)((char*)self + kHairRecordCopyBias);
    sourceWords = (unsigned int*)((char*)source - 4);
    unsigned int* end = sourceWords + kHairConfigurationPairedWords;
    for (; sourceWords != end; ) {
        unsigned int firstWord = sourceWords[1];
        unsigned int secondWord = sourceWords[2];
        sourceWords += 2;
        destinationWords[1] = firstWord;
        destinationWords[2] = secondWord;
        destinationWords += 2;
    }
    destinationWords[1] = sourceWords[1];
    ((HairVt*)self)->v1C(initArg);
    return self;
}

class V1C7E24 {
    public: 
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C(void*);
}
;
struct CShopBaseA {
    CShopBaseA(void*, void*);
    ~CShopBaseA();
    void** vtable;
    void v1C(void* p) { ((ObjCtorVt*)this)->init(p); }
};
struct CShopProductBall : CShopBaseA {
    CShopProductBall(void* a, void* b);
};
// Initialize the ball category base and invoke its product-list initialization hook.
CShopProductBall::CShopProductBall(void* a, void* b) : CShopBaseA(a, b) { vtable = (void**)__vt__16CShopProductBall; v1C(b); }

class V1C8708 {
    public: 
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C(void*);
}
;
struct CShopBaseB {
    CShopBaseB(void*, void*);
    ~CShopBaseB();
    void** vtable;
    void v1C(void* p) { ((ObjCtorVt*)this)->init(p); }
};
struct CShopProductCaddie : CShopBaseB {
    CShopProductCaddie(void* a, void* b);
};
// Initialize the caddie category base and invoke its product-list initialization hook.
CShopProductCaddie::CShopProductCaddie(void* a, void* b) : CShopBaseB(a, b) {
    vtable = (void**)__vt__18CShopProductCaddie;
    v1C(b);
}

struct CShopBaseD {
    CShopBaseD(void*);
    ~CShopBaseD();
    void** vtable;
    void v1C(void* p) { ((ObjCtorVt*)this)->init(p); }
};
struct CShopProductClubSet : CShopBaseD {
    CShopProductClubSet(void* a, void* b);
};
// Initialize the club-set provider and populate its category products.
CShopProductClubSet::CShopProductClubSet(void* a, void* b) : CShopBaseD(a) {
    vtable = (void**)__vt__19CShopProductClubSet;
    v1C(b);
}

struct CShopBaseE {
    CShopBaseE(void*);
    ~CShopBaseE();
    void** vtable;
    void v1C(void* p) { ((ObjCtorVt*)this)->init(p); }
};
struct CShopProductEquipItem : CShopBaseE {
    CShopProductEquipItem(void* a, void* b);
};
// Initialize the equipment provider and populate its category products.
CShopProductEquipItem::CShopProductEquipItem(void* a, void* b) : CShopBaseE(a) {
    vtable = (void**)__vt__21CShopProductEquipItem;
    v1C(b);
}


// Require both the saved item flag and an item id belonging to this caddie.
int CShopProductCaddieItem_IsAvailableForCaddie(void* self, void* productRecord, int testEquipped) {
    int available = SaveData_TestCaddieItemFlag(((ShopProductListView*)self)->saveData, (void*)((ShopProductRecord*)productRecord)->typeId, testEquipped);
    int result = 0;
    if (available &&
        ((((ShopProductRecord*)productRecord)->typeId >> kCaddieItemOwnerShift) & kCaddieItemOwnerMask) == (((CaddieItemProductView*)self)->caddieTypeId & kItemSerialMask))
        result = 1;
    return result;
}

} // extern "C"

CShopProductAvatar::~CShopProductAvatar() {}


extern "C" {

// Destroy category resources and free the product only for a positive deleting-destructor flag.
void* CShopProductBall_dtor(void* self, int shouldDelete) {
    if (self) {
        __dt__10CShopBaseAFv(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy category resources and free the product only for a positive deleting-destructor flag.
void* CShopProductCaddie_dtor(void* self, int shouldDelete) {
    if (self) {
        __dt__10CShopBaseBFv(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy category resources and free the product only for a positive deleting-destructor flag.
void* CShopProductCaddieItem_dtor(void* self, int shouldDelete) {
    if (self) {
        __dt__10CShopBaseCFv(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy category resources and free the product only for a positive deleting-destructor flag.
void* CShopProductClubSet_dtor(void* self, int shouldDelete) {
    if (self) {
        __dt__10CShopBaseDFv(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy category resources and free the product only for a positive deleting-destructor flag.
void* CShopProductEquipItem_dtor(void* self, int shouldDelete) {
    if (self) {
        __dt__10CShopBaseEFv(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Delegate equipment-item availability to the save-data flag query.
int CShopProductEquipItem_vfunc_6(Fn80238B40Arg* product, Fn80238B40Arg2* productRecord, int testEquipped) {
    return SaveData_IsConsumableAvailable(product->saveData, productRecord->typeId, testEquipped);
}

// Report that this product interface is ready.
int CShopProductInterface_vfunc_1() {
    return 1;
}

// Test the saved owned flag, or the equipped flag when testEquipped is set; missing save records fail.
// An equipped avatar must also match the active saved character index.
int CShopProductAvatar_IsOwnedOrEquipped(void* self, void* productRecord, int testEquipped) {
    volatile unsigned char savedFlags[0x38];
    if (testEquipped) {
        int index = ItemInventory_FindCharacterIndex(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), (void*)((ShopProductRecord*)productRecord)->typeId);
        signed char current = AsSavePlayerRecord(SaveData_GetCurrentRecord(((ShopProductListView*)self)->saveData))->characterIndex;
        if (current != index)
            return 0;
    }
    if (SaveData_CopyCharacterEquipmentRecord(((ShopProductListView*)self)->saveData, (unsigned char*)savedFlags, (void*)((ShopProductRecord*)productRecord)->typeId) == 0)
        return 0;
    if (testEquipped) {
        unsigned int flagInSignBit = (unsigned int)savedFlags[0] << kEquippedFlagToSignBit;
        flagInSignBit &= kTwoHighBits;
        return ((int)flagInSignBit >> 31) != 0;
    } else {
        unsigned int flagInSignBit = (unsigned int)savedFlags[0] << kOwnedFlagToSignBit;
        flagInSignBit &= kTwoHighBits;
        return ((int)flagInSignBit >> 31) != 0;
    }
}

#pragma use_lmw_stmw on
struct CShopProductHairExceptionBuffer {
    void* table;
    char* text;
    void* owner;
    ~CShopProductHairExceptionBuffer() { __dt__Q23std9exceptionFv(this); }
};
struct CShopProductHairExceptionGuard {
    void* text;
    void* allocator;
    ~CShopProductHairExceptionGuard() { __dt__Q33std6detail16char_array_guardFv(this, 0); }
};
struct CShopProductHairExceptionOwner {
    void* table;
    int a;
    int b;
    char* text;
    CShopProductHairExceptionOwner(char* p) {
        a = 1;
        b = 1;
        table = &s_sharedCharDeleterVtable;
        text = p;
    }
};
// Offer initially unlocked hairstyles for this character; optionally require the save-slot ownership bit.
int CShopProductHair_CheckUnlockAndOwnership(void* self, void* hairStyleRecord, int testOwned) {
    int result = 0;
    unsigned char shopFlags = ((IFF_STRUCT::sHairStyle*)hairStyleRecord)->common.shopFlags;
    unsigned int characterTypeId = *(volatile unsigned int*)&AsCShopProductHair(self)->characterTypeId; // 0x80238C4C: +0x64
    if ((shopFlags >> kHairInitiallyUnlockedShift) & kHairInitiallyUnlockedMask) {
        unsigned char index = ((IFF_STRUCT::sHairStyle*)hairStyleRecord)->characterIndex;
        if ((characterTypeId & kItemSerialMask) == index)
            result = 1;
    }
    if (testOwned) {
        unsigned int index = ItemInventory_FindHairStyleIndex(((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory), (void*)((ShopProductRecord*)hairStyleRecord)->typeId);
        void* data = SaveData_GetCurrentRecord(AsCShopProductHair(self)->saveData);
        if (index >= kHairStyleCount) {
            CShopProductHairExceptionBuffer buffer;
            CShopProductHairExceptionBuffer* bp = &buffer;
            const char* messageText = shopHairOwnershipBoundsError;
            *(volatile void**)&bp->table = &__vt__Q23std11logic_error;
            unsigned char empty = 0;
            unsigned int len = strlen(messageText) + 1;
            char* message = (char*)__nwa__FUl(len * 2 - len);
            bp->text = message;
            CShopProductHairExceptionGuard guard = { message, &empty };
            bp->owner = new CShopProductHairExceptionOwner(message);
            guard.text = 0;
            strcpy(bp->text, shopHairOwnershipBoundsError);
            bp->table = &__vt__Q23std12out_of_range;
            __throw(kShopHairOutOfRangeExceptionType, bp, __dt__Q23std12out_of_rangeFv);
        }
        unsigned int offset = (index >> kOwnershipWordShift) * sizeof(unsigned int);
        unsigned int shift = index & kOwnershipBitMask;
                unsigned int ownedStylesWord = AsSavePlayerRecord(data)->ownedHairStyles[offset / sizeof(unsigned int)];
        unsigned int mask = 1u << shift;
        unsigned int present = mask & ownedStylesWord;
        unsigned int isOwned = present != 0;
        return (result & isOwned) != 0;
    }
    return result;
}
#pragma use_lmw_stmw reset

// Initialize this category's angle conversion constant during static startup.
void ShopBall_InitAngleConstant() {
    s_shopBallAngleConstant = kFloat_6_2831855;
}

// Initialize this category's angle conversion constant during static startup.
void ShopCaddie_InitAngleConstant() {
    s_shopCaddieAngleConstant = kFloat_6_2831855;
}

// Initialize this category's angle conversion constant during static startup.
void ShopCaddieItem_InitAngleConstant() {
    s_shopCaddieItemAngleConstant = kFloat_6_2831855;
}

// Initialize this category's angle conversion constant during static startup.
void ShopClubSet_InitAngleConstant() {
    s_shopClubSetAngleConstant = kFloat_6_2831855;
}

// Initialize this category's angle conversion constant during static startup.
void ShopEquipItem_InitAngleConstant() {
    s_shopEquipItemAngleConstant = kFloat_6_2831855;
}

// Initialize this category's angle conversion constant during static startup.
void ShopAvatar_InitAngleConstant() {
    s_shopAvatarAngleConstant = kFloat_6_2831855;
}

// Initialize this category's angle conversion constant during static startup.
void ShopHair_InitAngleConstant() {
    s_shopHairAngleConstant = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462250", _ctors_u80237E24_0, ShopBall_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462250", _ctors_u80237E24_1, ShopCaddie_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462250", _ctors_u80237E24_2, ShopCaddieItem_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462250", _ctors_u80237E24_3, ShopClubSet_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462250", _ctors_u80237E24_4, ShopEquipItem_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462250", _ctors_u80237E24_5, ShopAvatar_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462250", _ctors_u80237E24_6, ShopHair_InitAngleConstant);
#pragma pop
