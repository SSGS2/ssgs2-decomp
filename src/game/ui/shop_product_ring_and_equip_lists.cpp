// Shop product lists for rings and equipment items.
//
// The ring list is rebuilt from AuxPart.iff and the equipment item list from Item.iff, each
// keeping an owned copy of every accepted record. Both also copy the common IFF header.

#include <decomp/static_init.h>
#include "classes/CSharedDoc.h"
#include "classes/CItemManager.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#define AsCItemManager(p) ((CItemManager*)(p))
// The shared header reserves this embedded CItemManager as bytes; give it its established table type here.
struct SharedShopInventoryView {
    char sharedState[IFF_OFFSET(CSharedDoc, itemInventory)];
    CItemManager itemInventory;
};
// Shared ownership block used by the vector exception's dynamically copied text.
struct SharedErrorTextOwner {
    void* vtable;
    int strongReferences;
    int weakReferences;
    char* text;
};
#include "iff/IffStruct.h"
// Constants retain the original integer types and encoded values.
static const unsigned kMaxPointerElements = 0x3FFFFFFFU;
static const unsigned kPointerGrowthThreeFifthsLimit = 0x15555555U;
static const unsigned kPointerGrowthHalfLimit = 0x2AAAAAAAU;
static const unsigned kLengthErrorTypeNameOffset = 0x14;

#include "classes/PointerCollectionState.h"
#include "sdk_protos.h"
#include "game/shop_product.h"
#include "classes/CShopProductRing.h"
#include "classes/CShopProductInterface.h"

extern "C" {

extern char s_shopRingRecordListVtable[];
extern char __vt__18CShopProductAvatar[];
extern float kFloat_6_2831855;
extern float s_shopRingFullCircleAngle;

// This shop callback always reports false; the virtual operation is not established here.
int fn_802373B4() {
    return 0;
}

struct ShopRecordCountView {
    unsigned char baseStorage[0xC];
    int value;
};

// Return the number of ring records currently in the product list.
int CShopProductRing::GetCount(ShopRecordCountView* arg) {
    return arg->value;
}

// This shop callback always reports false; the virtual operation is not established here.
int fn_80237DB0() {
    return 0;
}

struct RingProductListState {
    void* vtable;
    void* value;
    void** records;
    int recordCount;
    int recordCapacity;
};

// Product list layout: vtable +0, borrowed filter/context +4, pointer array +8,
// size +0xC, capacity +0x10. Each accepted IFF record is a separately owned new
// allocation; the pointer array uses appMallocDummy/appFreeUni. DeleteAll frees
// records but retains capacity. The destructor additionally frees the array.
// A failed rebuild can leave a partially populated list; there is no rollback.

// Initialize a ring product list with its filter/context and empty pointer storage.
void ShopRingRecordList_Initialize(RingProductListState* list, void* value) {
    list->records = 0;
    list->vtable = s_shopRingRecordListVtable;
    list->recordCount = 0;
    list->recordCapacity = 0;
    list->value = value;
}

// Initialize the shop unit's cached full-circle angle.
void ShopRing_InitializeAngleCache() {
    s_shopRingFullCircleAngle = kFloat_6_2831855;
}

void __dl__FPv(void*);
void __dla__FPv(void*);
void appFreeUni__FPv(void*);
void* __nw__FUl(unsigned long);
void* appMallocDummy__FUl(unsigned long);
void* __nwa__FUl(unsigned long);
void* memcpy(void*, const void*, unsigned long);
void* memset(void*, int, unsigned long);
extern CSharedDoc* g_sharedDoc;
extern char comThinkVectorErrorStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
void __dt__Q23std12length_errorFv();

struct ShopRecordProvider {
    virtual void unusedSlot0();
    virtual void unusedSlot1();
    virtual int get(void*);
};

struct ShopBytePair {
    unsigned char a;
    unsigned char b;
};

struct ShopCommonTextBytes {
    unsigned char name[40], icon[40];
};
struct ShopCommonRecordBuffer {
    union {
        IffItemCommon common;
        ShopCommonTextBytes text; // Unsigned byte view preserves the fixed-array copy ABI.
        unsigned char bytes[sizeof(IffItemCommon)];
    };
    unsigned char selectedRecordTail[0x25C];
};

// get() fills caller-owned scratch with the selected concrete record. These two
// common-header routines below copy only the 0x5C-byte header into out; tail bytes stay
// untouched. Success is 1; null out or failed get returns 0 without writing out.

// Copy the selected equipment item's common IFF header, including type ID and Pang price.
int CShopProductEquipItem_CopyCommonRecord(ShopRecordProvider* self, ShopCommonRecordBuffer* out) {
    if (out == 0)
        return 0;
    ShopCommonRecordBuffer data;
    if (self->get(&data) == 0)
        return 0;
    unsigned char a;
    unsigned char b;
    // +0x00..0x27: message-id name.
    b = data.text.name[0];
    a = data.text.name[1];
    out->text.name[0] = b;
    out->text.name[1] = a;
    b = data.text.name[2];
    a = data.text.name[3];
    out->text.name[2] = b;
    out->text.name[3] = a;
    a = data.text.name[4];
    b = data.text.name[5];
    out->text.name[4] = a;
    out->text.name[5] = b;
    a = data.text.name[6];
    b = data.text.name[7];
    out->text.name[6] = a;
    out->text.name[7] = b;
    a = data.text.name[8];
    b = data.text.name[9];
    out->text.name[8] = a;
    out->text.name[9] = b;
    a = data.text.name[10];
    b = data.text.name[11];
    out->text.name[10] = a;
    out->text.name[11] = b;
    a = data.text.name[12];
    b = data.text.name[13];
    out->text.name[12] = a;
    out->text.name[13] = b;
    a = data.text.name[14];
    b = data.text.name[15];
    out->text.name[14] = a;
    out->text.name[15] = b;
    a = data.text.name[16];
    b = data.text.name[17];
    out->text.name[16] = a;
    out->text.name[17] = b;
    a = data.text.name[18];
    b = data.text.name[19];
    out->text.name[18] = a;
    out->text.name[19] = b;
    a = data.text.name[20];
    b = data.text.name[21];
    out->text.name[20] = a;
    out->text.name[21] = b;
    a = data.text.name[22];
    b = data.text.name[23];
    out->text.name[22] = a;
    out->text.name[23] = b;
    a = data.text.name[24];
    b = data.text.name[25];
    out->text.name[24] = a;
    out->text.name[25] = b;
    a = data.text.name[26];
    b = data.text.name[27];
    out->text.name[26] = a;
    out->text.name[27] = b;
    a = data.text.name[28];
    b = data.text.name[29];
    out->text.name[28] = a;
    out->text.name[29] = b;
    a = data.text.name[30];
    b = data.text.name[31];
    out->text.name[30] = a;
    out->text.name[31] = b;
    a = data.text.name[32];
    b = data.text.name[33];
    out->text.name[32] = a;
    out->text.name[33] = b;
    a = data.text.name[34];
    b = data.text.name[35];
    out->text.name[34] = a;
    out->text.name[35] = b;
    a = data.text.name[36];
    b = data.text.name[37];
    out->text.name[36] = a;
    out->text.name[37] = b;
    a = data.text.name[38];
    b = data.text.name[39];
    out->text.name[38] = a;
    out->text.name[39] = b;
    // +0x28..0x4F: icon resource name.
    a = data.text.icon[0];
    b = data.text.icon[1];
    out->text.icon[0] = a;
    out->text.icon[1] = b;
    a = data.text.icon[2];
    b = data.text.icon[3];
    out->text.icon[2] = a;
    out->text.icon[3] = b;
    a = data.text.icon[4];
    b = data.text.icon[5];
    out->text.icon[4] = a;
    out->text.icon[5] = b;
    a = data.text.icon[6];
    b = data.text.icon[7];
    out->text.icon[6] = a;
    out->text.icon[7] = b;
    a = data.text.icon[8];
    b = data.text.icon[9];
    out->text.icon[8] = a;
    out->text.icon[9] = b;
    a = data.text.icon[10];
    b = data.text.icon[11];
    out->text.icon[10] = a;
    out->text.icon[11] = b;
    a = data.text.icon[12];
    b = data.text.icon[13];
    out->text.icon[12] = a;
    out->text.icon[13] = b;
    a = data.text.icon[14];
    b = data.text.icon[15];
    out->text.icon[14] = a;
    out->text.icon[15] = b;
    a = data.text.icon[16];
    b = data.text.icon[17];
    out->text.icon[16] = a;
    out->text.icon[17] = b;
    a = data.text.icon[18];
    b = data.text.icon[19];
    out->text.icon[18] = a;
    out->text.icon[19] = b;
    a = data.text.icon[20];
    b = data.text.icon[21];
    out->text.icon[20] = a;
    out->text.icon[21] = b;
    a = data.text.icon[22];
    b = data.text.icon[23];
    out->text.icon[22] = a;
    out->text.icon[23] = b;
    a = data.text.icon[24];
    b = data.text.icon[25];
    out->text.icon[24] = a;
    out->text.icon[25] = b;
    a = data.text.icon[26];
    b = data.text.icon[27];
    out->text.icon[26] = a;
    out->text.icon[27] = b;
    a = data.text.icon[28];
    b = data.text.icon[29];
    out->text.icon[28] = a;
    out->text.icon[29] = b;
    a = data.text.icon[30];
    b = data.text.icon[31];
    out->text.icon[30] = a;
    out->text.icon[31] = b;
    a = data.text.icon[32];
    b = data.text.icon[33];
    out->text.icon[32] = a;
    out->text.icon[33] = b;
    a = data.text.icon[34];
    b = data.text.icon[35];
    out->text.icon[34] = a;
    out->text.icon[35] = b;
    a = data.text.icon[36];
    b = data.text.icon[37];
    out->text.icon[36] = a;
    out->text.icon[37] = b;
    a = data.text.icon[38];
    b = data.text.icon[39];
    out->text.icon[38] = a;
    out->text.icon[39] = b;
    // +0x50..0x5B: key, price, level/shop flags, unresolved final halfword.
    out->common.typeId = data.common.typeId;
    out->common.price = data.common.price;
    out->common.levelFlags = data.common.levelFlags;
    out->common.shopFlags = data.common.shopFlags;
    out->common.commonFlagWord = data.common.commonFlagWord;
    return 1;
}

// Copy the selected ring's common IFF header, including level and shop flags.
int CShopProductRing::CopyCommonRecord(ShopCommonRecordBuffer* out) {
    ShopRecordProvider* self = (ShopRecordProvider*)this;
    if (out == 0)
        return 0;
    ShopCommonRecordBuffer data;
    if (self->get(&data) == 0)
        return 0;
    unsigned char a;
    unsigned char b;
    // +0x00..0x27: message-id name.
    b = data.text.name[0];
    a = data.text.name[1];
    out->text.name[0] = b;
    out->text.name[1] = a;
    b = data.text.name[2];
    a = data.text.name[3];
    out->text.name[2] = b;
    out->text.name[3] = a;
    a = data.text.name[4];
    b = data.text.name[5];
    out->text.name[4] = a;
    out->text.name[5] = b;
    a = data.text.name[6];
    b = data.text.name[7];
    out->text.name[6] = a;
    out->text.name[7] = b;
    a = data.text.name[8];
    b = data.text.name[9];
    out->text.name[8] = a;
    out->text.name[9] = b;
    a = data.text.name[10];
    b = data.text.name[11];
    out->text.name[10] = a;
    out->text.name[11] = b;
    a = data.text.name[12];
    b = data.text.name[13];
    out->text.name[12] = a;
    out->text.name[13] = b;
    a = data.text.name[14];
    b = data.text.name[15];
    out->text.name[14] = a;
    out->text.name[15] = b;
    a = data.text.name[16];
    b = data.text.name[17];
    out->text.name[16] = a;
    out->text.name[17] = b;
    a = data.text.name[18];
    b = data.text.name[19];
    out->text.name[18] = a;
    out->text.name[19] = b;
    a = data.text.name[20];
    b = data.text.name[21];
    out->text.name[20] = a;
    out->text.name[21] = b;
    a = data.text.name[22];
    b = data.text.name[23];
    out->text.name[22] = a;
    out->text.name[23] = b;
    a = data.text.name[24];
    b = data.text.name[25];
    out->text.name[24] = a;
    out->text.name[25] = b;
    a = data.text.name[26];
    b = data.text.name[27];
    out->text.name[26] = a;
    out->text.name[27] = b;
    a = data.text.name[28];
    b = data.text.name[29];
    out->text.name[28] = a;
    out->text.name[29] = b;
    a = data.text.name[30];
    b = data.text.name[31];
    out->text.name[30] = a;
    out->text.name[31] = b;
    a = data.text.name[32];
    b = data.text.name[33];
    out->text.name[32] = a;
    out->text.name[33] = b;
    a = data.text.name[34];
    b = data.text.name[35];
    out->text.name[34] = a;
    out->text.name[35] = b;
    a = data.text.name[36];
    b = data.text.name[37];
    out->text.name[36] = a;
    out->text.name[37] = b;
    a = data.text.name[38];
    b = data.text.name[39];
    out->text.name[38] = a;
    out->text.name[39] = b;
    // +0x28..0x4F: icon resource name.
    a = data.text.icon[0];
    b = data.text.icon[1];
    out->text.icon[0] = a;
    out->text.icon[1] = b;
    a = data.text.icon[2];
    b = data.text.icon[3];
    out->text.icon[2] = a;
    out->text.icon[3] = b;
    a = data.text.icon[4];
    b = data.text.icon[5];
    out->text.icon[4] = a;
    out->text.icon[5] = b;
    a = data.text.icon[6];
    b = data.text.icon[7];
    out->text.icon[6] = a;
    out->text.icon[7] = b;
    a = data.text.icon[8];
    b = data.text.icon[9];
    out->text.icon[8] = a;
    out->text.icon[9] = b;
    a = data.text.icon[10];
    b = data.text.icon[11];
    out->text.icon[10] = a;
    out->text.icon[11] = b;
    a = data.text.icon[12];
    b = data.text.icon[13];
    out->text.icon[12] = a;
    out->text.icon[13] = b;
    a = data.text.icon[14];
    b = data.text.icon[15];
    out->text.icon[14] = a;
    out->text.icon[15] = b;
    a = data.text.icon[16];
    b = data.text.icon[17];
    out->text.icon[16] = a;
    out->text.icon[17] = b;
    a = data.text.icon[18];
    b = data.text.icon[19];
    out->text.icon[18] = a;
    out->text.icon[19] = b;
    a = data.text.icon[20];
    b = data.text.icon[21];
    out->text.icon[20] = a;
    out->text.icon[21] = b;
    a = data.text.icon[22];
    b = data.text.icon[23];
    out->text.icon[22] = a;
    out->text.icon[23] = b;
    a = data.text.icon[24];
    b = data.text.icon[25];
    out->text.icon[24] = a;
    out->text.icon[25] = b;
    a = data.text.icon[26];
    b = data.text.icon[27];
    out->text.icon[26] = a;
    out->text.icon[27] = b;
    a = data.text.icon[28];
    b = data.text.icon[29];
    out->text.icon[28] = a;
    out->text.icon[29] = b;
    a = data.text.icon[30];
    b = data.text.icon[31];
    out->text.icon[30] = a;
    out->text.icon[31] = b;
    a = data.text.icon[32];
    b = data.text.icon[33];
    out->text.icon[32] = a;
    out->text.icon[33] = b;
    a = data.text.icon[34];
    b = data.text.icon[35];
    out->text.icon[34] = a;
    out->text.icon[35] = b;
    a = data.text.icon[36];
    b = data.text.icon[37];
    out->text.icon[36] = a;
    out->text.icon[37] = b;
    a = data.text.icon[38];
    b = data.text.icon[39];
    out->text.icon[38] = a;
    out->text.icon[39] = b;
    // +0x50..0x5B: key, price, level/shop flags, unresolved final halfword.
    out->common.typeId = data.common.typeId;
    out->common.price = data.common.price;
    out->common.levelFlags = data.common.levelFlags;
    out->common.shopFlags = data.common.shopFlags;
    out->common.commonFlagWord = data.common.commonFlagWord;
    return 1;
}

struct RingProductRecords {
    unsigned char baseStorage[8];
    void** entries;
    unsigned int count;
};

// Copy a ring record by index; reject null output, invalid indices, and null entries.
int CShopProductRing::CopyRecord(RingProductRecords* records, void* dst, unsigned int index) {
    if (dst == 0)
        return 0;
    if (records->count <= index)
        return 0;
    void* entry = records->entries[index];
    if (entry == 0)
        return 0;
    Memory_Copy(dst, entry, sizeof(IFF_STRUCT::sAuxPart));
    return 1;
}

struct RingProductRecordsForClear {
    unsigned char baseStorage[8];
    void** entries;
    volatile unsigned int count;
};

// Delete every owned ring record and empty the list, retaining its pointer capacity.
int CShopProductRing::DeleteAll(RingProductRecordsForClear* records) {
    void** entry = records->entries;
    while (records->entries + records->count != entry) {
        if (*entry != 0) {
            __dl__FPv(*entry);
            *entry = 0;
        }
        ++entry;
    }
    register unsigned int c = records->count;
    records->count = c - c;
    return 1;
}

struct RingProductVtable {
    unsigned char baseStorage[0xC];
    int (*clear)(void*);
};

struct RingProductOwnedRecords {
    RingProductVtable* vtable;
    unsigned char filterStorage[4];
    void** entries;
    volatile unsigned int count;
};

// Clear ring records, free pointer storage, and optionally delete the list object.
void* ShopRingRecordList_dtor(RingProductOwnedRecords* obj, int should_delete) {
    if (obj != 0) {
        RingProductVtable* vt = (RingProductVtable*)s_shopRingRecordListVtable;
        obj->vtable = vt;
        vt->clear(obj);
        if (obj->entries != 0) {
            obj->count = obj->count - obj->count;
            appFreeUni__FPv(obj->entries);
        }
        ((CShopProductInterface*)obj)->Destroy(0);
        if (should_delete > 0)
            __dl__FPv(obj);
    }
    return obj;
}

} // extern "C"

// Initialize the character-record product base, then forward its selection/filter flag.
CShopProductAvatar::CShopProductAvatar(void* value, unsigned char flag) : CShopProduct<IFF_STRUCT::sChar>(value) {
    slot1C(flag);
}

extern "C" {

// Borrow whichever capacity/growth argument is greater (neither is copied).
static inline const unsigned& VectorMax(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}

// Throw the original vector length_error when pointer-array growth would overflow.
static inline void VectorThrowLengthError() {
    char* errorText = comThinkVectorErrorStrings;
    struct LengthErrorObject {
        void* vtable;
        void* data;
        void* rep;
    } errorObject;
    errorObject.vtable = __vt__Q23std11logic_error;
    volatile char cleanupChar = 0;
    unsigned errorLength = strlen(errorText) + 1;
    char* errorCopy = (char*)__nwa__FUl((errorLength << 1) - errorLength);
    errorObject.data = errorCopy;
    struct Cleanup {
        char* p;
        volatile char* pp;
    } cleanup;
    cleanup.p = errorCopy;
    cleanup.pp = &cleanupChar;
    SharedErrorTextOwner* errorOwner = (SharedErrorTextOwner*)__nw__FUl(sizeof(SharedErrorTextOwner));
    if (errorOwner != 0) {
        errorOwner->strongReferences = 1;
        errorOwner->weakReferences = 1;
        errorOwner->vtable = s_sharedCharDeleterVtable;
        errorOwner->text = errorCopy;
    }
    errorObject.rep = errorOwner;
    cleanup.p = 0;
    if (cleanupChar != 0)
        __dla__FPv(cleanup.p);
    strcpy(errorCopy, errorText);
    errorObject.vtable = __vt__Q23std12length_error;
    __throw(comThinkVectorErrorStrings + kLengthErrorTypeNameOffset, &errorObject, (void*)__dt__Q23std12length_errorFv);
}

#pragma push
#pragma use_lmw_stmw on
struct Copy92 {
    unsigned w[23];
};

struct RingInventoryTableView {
    char pad[0xAB4];
    void* volatile begin;
    volatile unsigned int count;
};

struct TargetArrayGuard {
    char* p;
    char& flag;
    // Protect a newly allocated error string until its shared owner is constructed.
    TargetArrayGuard(char* value, char& f) : p(value), flag(f) {}
    // Release untransferred text on unwind; clearing p transfers ownership.
    ~TargetArrayGuard() {
        if (p != 0)
            __dla__FPv(p);
    }
};

extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct TargetLengthBase {
    void* vtable;
    char* data;
    void* rep;
    // Delegate exception-base teardown to the original runtime implementation.
    ~TargetLengthBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};

} // extern "C"

namespace std {
struct length_error : TargetLengthBase {
    length_error(const char*);
    ~length_error();
};

// Copy the borrowed message and install the original shared-string exception representation.
inline length_error::length_error(const char* text) {
    vtable = __vt__Q23std11logic_error;
    char flag = 0;
    unsigned length = strlen(text) + 1;
    char* copy = (char*)__nwa__FUl((length << 1) - length);
    data = copy;
    {
        TargetArrayGuard guard(copy, flag);
        SharedErrorTextOwner* owner = (SharedErrorTextOwner*)__nw__FUl(sizeof(SharedErrorTextOwner));
        if (owner) {
            owner->strongReferences = 1;
            owner->weakReferences = 1;
            owner->vtable = s_sharedCharDeleterVtable;
            owner->text = copy;
        }
        rep = owner;
        guard.p = 0;
    }
    strcpy(data, text);
    vtable = __vt__Q23std12length_error;
}
}

// Use supplied exception storage without allocating or taking ownership of it.
inline void* operator new(size_t, void* p) {
    return p;
}

extern "C" {
// Construct and throw the original vector length_error with its cleanup representation.
static inline void TargetThrowLengthError() {
    struct Storage {
        void* words[3];
    } storage;
    std::length_error* error = new (&storage) std::length_error(comThinkVectorErrorStrings);
    __throw(comThinkVectorErrorStrings + kLengthErrorTypeNameOffset, error, (void*)__dt__Q23std12length_errorFv);
}

struct TargetVectorBuffer {
    void** data;
    unsigned size;
    unsigned capacity;
    unsigned* sourceCapacity;
    unsigned offset;
    // Start an empty temporary pointer-array owner; record payloads are not owned here.
    TargetVectorBuffer(unsigned* a) : data(0), size(0), capacity(0), sourceCapacity(a), offset(0) {}
    // Free only the pointer allocation (possibly the old array after a swap).
    ~TargetVectorBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

struct TargetCollection {
    virtual void slot08();
    virtual int clear();
    virtual void slot10();
    virtual void slot14();
    virtual void slot18();
    virtual void slot1C();
    virtual int accepts(void*, void*);
};

struct EquipItemInventoryTableView {
    char pad[0xA60];
    void* volatile begin;
    volatile unsigned count;
};

// Rebuild policy (also used by the ring variant below): clear -> filter -> clone -> append.
// A successful append transfers recordCopy to the list. Spare capacity takes the
// short branch; all remaining code in that iteration implements the same append
// using a replacement POINTER array. It never clones existing record payloads.
// Allocation failure returns 0 after earlier accepted entries have committed.
// Rebuild the filtered equipment-item list from Item.iff, owning a copy of each accepted record.
int CShopProductEquipItem_RebuildFilteredItemList(TargetCollection* self, void* filterContext) {
    // Phase 1: clear old payloads; scan keyed IFF entries, skipping the four-byte key.
    if (!self->clear())
        return 0;
    CSharedDoc* mgr = g_sharedDoc;
    for (char* records = (char*)(&((volatile SharedShopInventoryView*)mgr)->itemInventory)->items.data;
         (&((volatile SharedShopInventoryView*)mgr)->itemInventory)->items.size * sizeof(ItemTableEntry) +
             (char*)(&((volatile SharedShopInventoryView*)mgr)->itemInventory)->items.data !=
         records;
         records += sizeof(ItemTableEntry)) {
        char* sourceRecord = records + 4;
        if (!self->accepts(sourceRecord, filterContext))
            continue;
        void* recordCopy = __nw__FUl(sizeof(IFF_STRUCT::sItem));
        if (!recordCopy)
            return 0;
        struct Copy60 {
            unsigned w[23];
            short a;
            short b;
        };
        *(Copy60*)recordCopy = *(Copy60*)sourceRecord;
        unsigned recordCount = AsPointerCollectionState(self)->size;
        unsigned pointerCapacity = AsPointerCollectionState(self)->capacity;
        if (pointerCapacity > recordCount) {
            unsigned next = recordCount + 1;
            void** recordPointers = AsPointerCollectionState(self)->data;
            AsPointerCollectionState(self)->size = next;
            recordPointers[next - 1] = recordCopy;
            continue;
        }
        if (kMaxPointerElements - pointerCapacity < 1) {
            TargetThrowLengthError();
        }
        // Phase 2: no free slot; choose checked growth (3/5, then 1/2, then the maximum).
        unsigned additionalCapacity =
            AsPointerCollectionState(self)->size + 1 - AsPointerCollectionState(self)->capacity;
        TargetVectorBuffer buffer(&AsPointerCollectionState(self)->capacity); // 0x80236EFC.
        pointerCapacity = AsPointerCollectionState(self)->capacity;
        if (additionalCapacity > kMaxPointerElements - pointerCapacity) {
            TargetThrowLengthError();
        }
        unsigned newCapacity;
        if (pointerCapacity < kPointerGrowthThreeFifthsLimit) {
            unsigned growth = ((pointerCapacity + 1) * 3U) / 5U;
            const unsigned& m = VectorMax(growth, additionalCapacity);
            newCapacity = pointerCapacity + m;
        } else if (pointerCapacity < kPointerGrowthHalfLimit) {
            unsigned growth = (pointerCapacity + 1) / 2;
            const unsigned& m = VectorMax(growth, additionalCapacity);
            newCapacity = pointerCapacity + m;
        } else {
            newCapacity = kMaxPointerElements;
        }
        if (newCapacity > kMaxPointerElements) {
            TargetThrowLengthError();
        }
        // Phase 3: place the new payload, move old pointers, then swap array ownership.
        // Zeroing old slots transfers pointers without deleting their records.
        void** newPointers = (void**)appMallocDummy__FUl(newCapacity * 4);
        buffer.offset = AsPointerCollectionState(self)->size;
        (newPointers + buffer.offset)[buffer.size] = recordCopy;
        buffer.capacity = newCapacity;
        void** recordPointers = AsPointerCollectionState(self)->data;
        void** end = recordPointers + AsPointerCollectionState(self)->size;
        buffer.data = newPointers;
        ++buffer.size;
        int count = end - recordPointers;
        buffer.offset -= count;
        memcpy(newPointers + buffer.offset, recordPointers, count * 4);
        memset(recordPointers, 0, count * 4);
        buffer.size += count;
        void** oldPointers = AsPointerCollectionState(self)->data;
        void** replacement = buffer.data;
        unsigned oldCap = AsPointerCollectionState(self)->capacity;
        AsPointerCollectionState(self)->size = 0;
        buffer.data = oldPointers;
        AsPointerCollectionState(self)->capacity = buffer.capacity;
        buffer.capacity = oldCap;
        AsPointerCollectionState(self)->data = replacement;
        AsPointerCollectionState(self)->size = buffer.size;
        buffer.size = 0;
    }
    return 1;
}

struct TargetErrorStorage {
    void* vtable;
    char* data;
    void* rep;
};

// Build a vector length_error in caller storage and transfer it to the exception runtime.
static inline void TargetThrowLengthError2(TargetErrorStorage& error, char* flag) {
    const char* text = comThinkVectorErrorStrings;
    error.vtable = __vt__Q23std11logic_error;
    flag[0] = 0;
    unsigned length = strlen(text) + 1;
    char* copy = (char*)__nwa__FUl((length << 1) - length);
    error.data = copy;
    {
        TargetArrayGuard guard(copy, flag[0]);
        SharedErrorTextOwner* owner = (SharedErrorTextOwner*)__nw__FUl(sizeof(SharedErrorTextOwner));
        if (owner) {
            owner->strongReferences = 1;
            owner->weakReferences = 1;
            owner->vtable = s_sharedCharDeleterVtable;
            owner->text = copy;
        }
        error.rep = owner;
        guard.p = 0;
    }
    strcpy(error.data, text);
    error.vtable = __vt__Q23std12length_error;
    __throw(text + kLengthErrorTypeNameOffset, &error, (void*)__dt__Q23std12length_errorFv);
}

// Reload the volatile ring-table bounds and return the byte address after the last keyed entry.
static inline char* TargetRecordsEnd(RingInventoryTableView* mgr) {
    unsigned count = (&((volatile SharedShopInventoryView*)mgr)->itemInventory)->auxParts.size;
    char* begin = (char*)(&((volatile SharedShopInventoryView*)mgr)->itemInventory)->auxParts.data;
    return begin + count * sizeof(AuxPartTableEntry);
}

// Copy a complete AuxPart ring record, retaining byte, halfword, and word load/store widths.
static inline void TargetCopyRecord70(void* dstp, const void* srcp) {
    *(Copy92*)dstp = *(Copy92*)srcp;
    unsigned char* o = (unsigned char*)dstp;
    const unsigned char* s = (const unsigned char*)srcp;
    o[0x5C] = s[0x5C];
    o[0x5D] = s[0x5D];
    o[0x5E] = s[0x5E];
    o[0x5F] = s[0x5F];
    o[0x60] = s[0x60];
    o[0x61] = s[0x61];
    o[0x62] = s[0x62];
    o[0x63] = s[0x63];
    o[0x64] = s[0x64];
    o[0x65] = s[0x65];
    *(short*)(o + 0x66) = *(short*)(s + 0x66);
    *(unsigned short*)(o + 0x68) = *(unsigned short*)(s + 0x68);
    *(unsigned short*)(o + 0x6A) = *(unsigned short*)(s + 0x6A);
    *(unsigned short*)(o + 0x6C) = *(unsigned short*)(s + 0x6C);
    o[0x6E] = s[0x6E];
    o[0x6F] = s[0x6F];
}

#pragma opt_lifetimes off
// Rebuild the filtered ring list from AuxPart.iff, growing pointer storage as necessary.
int CShopProductRing::RebuildFilteredAuxParts(void* filterContext) {
    TargetCollection* self = (TargetCollection*)this;
    TargetErrorStorage error1, error2, error3;
    unsigned growthLarge, growthSmall, additionalCapacity;
    char flag1[4], flag2[4], flag3[4];
    TargetErrorStorage* perr3;
    TargetErrorStorage* perr1;
    char* records;
    char* sourceRecord;
    void* recordCopy;
    unsigned pointerCapacity;
    unsigned newCapacity;
    // Phase 1: clear owned records and filter AuxPart entries using the borrowed context.
    if (!self->clear())
        return 0;
    perr3 = &error3;
    perr1 = &error1;
    RingInventoryTableView* mgr = (RingInventoryTableView*)g_sharedDoc;
    for (records = (char*)(&((volatile SharedShopInventoryView*)mgr)->itemInventory)->auxParts.data;
         TargetRecordsEnd(mgr) != records; records += sizeof(AuxPartTableEntry)) {
        sourceRecord = records + 4;
        if (!self->accepts(sourceRecord, filterContext))
            continue;
        recordCopy = __nw__FUl(sizeof(IFF_STRUCT::sAuxPart));
        if (!recordCopy)
            return 0;
        TargetCopyRecord70(recordCopy, sourceRecord);
        unsigned recordCount = AsPointerCollectionState(self)->size;
        pointerCapacity = AsPointerCollectionState(self)->capacity;
        if (recordCount < pointerCapacity) {
            unsigned next = recordCount + 1;
            void** recordPointers = AsPointerCollectionState(self)->data;
            AsPointerCollectionState(self)->size = next;
            recordPointers[next - 1] = recordCopy;
            continue;
        }
        if (kMaxPointerElements - pointerCapacity < 1) {
            TargetThrowLengthError2(*perr1, flag1);
        }
        // Phase 2: choose checked pointer capacity; record payload is already allocated.
        additionalCapacity = AsPointerCollectionState(self)->size + 1 - AsPointerCollectionState(self)->capacity;
        TargetVectorBuffer buffer(&AsPointerCollectionState(self)->capacity); // 0x80237888.
        pointerCapacity = AsPointerCollectionState(self)->capacity;
        if (additionalCapacity > kMaxPointerElements - pointerCapacity) {
            TargetThrowLengthError2(*perr3, flag3);
        }
        if (pointerCapacity < kPointerGrowthThreeFifthsLimit) {
            growthLarge = ((pointerCapacity + 1) * 3U) / 5U;
            const unsigned& m = VectorMax(growthLarge, additionalCapacity);
            newCapacity = pointerCapacity + m;
        } else if (pointerCapacity < kPointerGrowthHalfLimit) {
            growthSmall = (pointerCapacity + 1) / 2;
            const unsigned& m = VectorMax(growthSmall, additionalCapacity);
            newCapacity = pointerCapacity + m;
        } else {
            newCapacity = kMaxPointerElements;
        }
        if (newCapacity > kMaxPointerElements) {
            TargetThrowLengthError2(error2, flag2);
        }
        // Phase 3: fill a replacement array and swap; buffer frees the former array only.
        void** newPointers = (void**)appMallocDummy__FUl(newCapacity * 4);
        buffer.offset = AsPointerCollectionState(self)->size;
        (newPointers + buffer.offset)[buffer.size] = recordCopy;
        buffer.capacity = newCapacity;
        void** recordPointers = AsPointerCollectionState(self)->data;
        void** end = recordPointers + AsPointerCollectionState(self)->size;
        buffer.data = newPointers;
        ++buffer.size;
        int count = end - recordPointers;
        buffer.offset -= count;
        memcpy(newPointers + buffer.offset, recordPointers, count * 4);
        memset(recordPointers, 0, count * 4);
        buffer.size += count;
        void** oldPointers = AsPointerCollectionState(self)->data;
        void** replacement = buffer.data;
        unsigned oldCap = AsPointerCollectionState(self)->capacity;
        AsPointerCollectionState(self)->size = 0;
        buffer.data = oldPointers;
        AsPointerCollectionState(self)->capacity = buffer.capacity;
        buffer.capacity = oldCap;
        AsPointerCollectionState(self)->data = replacement;
        AsPointerCollectionState(self)->size = buffer.size;
        buffer.size = 0;
    }
    return 1;
}

#pragma opt_lifetimes reset

#undef THROW_VECTOR_LENGTH_ERROR
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046224C", _ctors_u80236BF4_0, ShopRing_InitializeAngleCache);
#pragma pop
