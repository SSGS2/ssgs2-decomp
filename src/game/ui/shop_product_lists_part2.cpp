// A pointer vector's maximum element count is its byte limit divided by four.
enum ShopPointerVectorLimit { kMaxProductPointers = 0x3FFFFFFF,
    kThreeFifthsGrowthLimit = 0x15555555, kHalfGrowthLimit = 0x2AAAAAAA };
enum ShopIffTableOffset { kPartTableOffset = 0x2C, kHairTableOffset = 0xA4 };
#include "classes/CSharedDoc.h"
#include "iff/IffStruct.h"
template <class T> inline bool same_pos(const T& a, const T& b) { return a == b; }
// Translation unit 0x80232BFC-0x80234124 (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/ShopProductListState.h"

extern "C" {

extern void *g_sharedDoc;
extern char comThinkVectorErrorStrings[];
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern "C" void *appMallocDummy__FUl(unsigned int);
extern "C" void appFreeUni__FPv(void*);
extern "C" void *__nw__FUl(unsigned int);
extern "C" void *memcpy(void*, const void*, unsigned int);
extern "C" void *memset(void*, int, unsigned int);
extern "C" void __dl__FPv(void*);

// Copy representation of IFF_STRUCT::sHairStyle; words[0..22] are IffItemCommon.
typedef IFF_STRUCT::sHairStyle HairStyleCopyRecord;
struct HairProductList {
    virtual void slot08();
    virtual int clearProducts();
    virtual void slot10();
    virtual void slot14();
    virtual void slot18();
    virtual void slot1c();
    virtual int acceptsRecord(HairStyleCopyRecord *, int);
    void *context;
    HairStyleCopyRecord **items;
    unsigned int count, capacity;
};
static inline void throwLengthError();

// Copy representation of IFF_STRUCT::sPart; retain assignment granularity for exact code.
typedef IFF_STRUCT::sPart PartCopyRecord;
struct PartProductList {
    virtual void slot08();
    virtual int clearProducts();
    virtual void slot10();
    virtual void slot14();
    virtual void slot18();
    virtual void slot1c();
    virtual int acceptsRecord(PartCopyRecord *, int);
    void *context;
    PartCopyRecord **items;
    unsigned int count;
    unsigned int capacity;
};

extern "C" void *__nwa__FUl(unsigned int);
extern "C" void __dla__FPv(void *);
extern "C" void __dt__Q23std12length_errorFv(void *);

// Construct and throw the vector-capacity length error.
static inline void throwLengthError() {
    void *msg[3];
    void *tmp[2];
    unsigned char flag = 0;
    char *data;
    void *obj;
    msg[0] = __vt__Q23std11logic_error;
    data = (char *)__nwa__FUl(strlen(comThinkVectorErrorStrings) + 1);
    msg[1] = data;
    tmp[0] = data;
    tmp[1] = &flag;
    obj = __nw__FUl(0x10);
    if (obj) {
        ((unsigned int *)obj)[1] = 1;
        ((unsigned int *)obj)[2] = 1;
        ((void **)obj)[0] = s_sharedCharDeleterVtable;
        ((void **)obj)[3] = data;
    }
    msg[2] = obj;
    tmp[0] = 0;
    strcpy((char *)msg[1], comThinkVectorErrorStrings);
    msg[0] = __vt__Q23std12length_error;
    __throw(comThinkVectorErrorStrings + 0x14, msg, (void *)__dt__Q23std12length_errorFv);
}

// The temporary owns the new allocation until the insertion can be committed.
struct F04Buffer {
    PartCopyRecord **items;
    unsigned int count, capacity;
    unsigned int *capacityPtr;
    unsigned int offset;
    F04Buffer(unsigned int *p) : items(0), count(0), capacity(0), capacityPtr(p), offset(0) {}
    ~F04Buffer() {
        if (items) {
            count = 0;
            appFreeUni__FPv(items);
        }
    }
};

// Return the larger capacity increment by reference.
static inline const unsigned int &f04Max(const unsigned int &a, const unsigned int &b) {
    return a < b ? b : a;
}

struct F04ErrorBase {
    void *vtable;
    char *data;
    void *owner;
    F04ErrorBase() : vtable(__vt__Q23std11logic_error) {}
    ~F04ErrorBase() {}
};
struct F04CharGuard {
    char *data;
    unsigned char *flag;
    F04CharGuard(char *p, unsigned char *f) : data(p), flag(f) {}
    ~F04CharGuard() { if (data) __dla__FPv(data); }
};
namespace std {
struct length_error : F04ErrorBase {
    length_error(const char *source) {
        struct Flag { unsigned char value; char padding[3]; } flag;
        flag.value = 0;
        unsigned int size = strlen(source) + 1;
        char *text = (char *)__nwa__FUl((char *)(size * 2) - (char *)size);
        data = text;
        {
            F04CharGuard guard(text, &flag.value);
            void *block = __nw__FUl(0x10);
            if (block) {
                ((unsigned int *)block)[1] = 1;
                ((unsigned int *)block)[2] = 1;
                ((void **)block)[0] = s_sharedCharDeleterVtable;
                ((void **)block)[3] = text;
            }
            owner = block;
            guard.data = 0;
        }
        strcpy(data, source);
        vtable = __vt__Q23std12length_error;
        __throw(comThinkVectorErrorStrings + 0x14, this, (void *)__dt__Q23std12length_errorFv);
    }
    ~length_error() {}
};
}
// Throw the vector length error using its original constructor/lifetime ordering.
static inline void f04ThrowLengthError(const char *source) {
    std::length_error error(source);
}

#pragma use_lmw_stmw on
struct A98Buffer {
    HairStyleCopyRecord **items;
    unsigned int count, capacity;
    unsigned int *capacityPtr;
    unsigned int offset;
    A98Buffer(unsigned int *p) : items(0), count(0), capacity(0), capacityPtr(p), offset(0) {}
    ~A98Buffer() {
        if (items) {
            count = 0;
            appFreeUni__FPv(items);
        }
    }
};

// Filter the hair-style IFF table and append owned copies to the shop list.
int CShopProductHair_BuildProductList(HairProductList *self, int filterArgument) {
    unsigned int growthThree, growthHalf, needed;
    if (!self->clearProducts()) return 0;
    CSharedDoc *sharedDoc = (CSharedDoc*)g_sharedDoc;
    HairStyleTableEntry *entry = ((IffTable<IFF_STRUCT::sHairStyle>*)(sharedDoc->itemInventory + kHairTableOffset))->data;
    const char *message = comThinkVectorErrorStrings;
    while (!same_pos(((IffTable<IFF_STRUCT::sHairStyle>*)(sharedDoc->itemInventory + kHairTableOffset))->data + ((IffTable<IFF_STRUCT::sHairStyle>*)(sharedDoc->itemInventory + kHairTableOffset))->size, entry)) {
        HairStyleCopyRecord *record = &entry->record;
        if (self->acceptsRecord(record, filterArgument)) {
            HairStyleCopyRecord *copy = (HairStyleCopyRecord *)__nw__FUl(sizeof(IFF_STRUCT::sHairStyle));
            if (copy == 0) return 0;
            *copy = *record;
            unsigned int count = self->count;
            unsigned int available = self->capacity;
            if (count < available) {
                self->count = count + 1;
                (self->items + self->count)[-1] = copy;
            } else {
                unsigned int newCapacity;
                if (kMaxProductPointers - self->capacity < 1) f04ThrowLengthError(message);
                A98Buffer buffer(&self->capacity);
                needed = self->count + 1 - self->capacity;
                unsigned int capacity = self->capacity;
                if (needed > kMaxProductPointers - capacity) f04ThrowLengthError(message);
                if (capacity < kThreeFifthsGrowthLimit) {
                    growthThree = (3 * (capacity + 1)) / 5;
                    newCapacity = capacity + f04Max(growthThree, needed);
                } else if (capacity < kHalfGrowthLimit) {
                    growthHalf = (capacity + 1) / 2;
                    newCapacity = capacity + f04Max(growthHalf, needed);
                } else newCapacity = kMaxProductPointers;
                if (newCapacity > kMaxProductPointers) f04ThrowLengthError(message);
                HairStyleCopyRecord **fresh = (HairStyleCopyRecord **)appMallocDummy__FUl(newCapacity * 4);
                buffer.offset = self->count;
                buffer.capacity = newCapacity;
                (fresh + buffer.offset)[buffer.count] = copy;
                buffer.items = fresh;
                ++buffer.count;
                HairStyleCopyRecord **old = self->items;
                HairStyleCopyRecord **end = self->items + self->count;
                int prefix = ((char *)end - (char *)old) / 4;
                buffer.offset -= prefix;
                memcpy(buffer.items + buffer.offset, old, prefix * 4);
                memset(old, 0, prefix * 4);
                buffer.count += prefix;
                self->count = 0;
                old = self->items;
                unsigned int oldCapacity = self->capacity;
                HairStyleCopyRecord **newItems = buffer.items;
                buffer.items = old;
                self->capacity = buffer.capacity;
                buffer.capacity = oldCapacity;
                self->items = newItems;
                self->count = buffer.count;
                buffer.count = 0;
            }
        }
        ++entry;
    }
    return 1;
}

// Filter the part IFF table and append owned copies to the shop list.
int CShopProductParts_RebuildFromPartTable(PartProductList *self, int filterArgument) {
    if (!self->clearProducts()) return 0;
    CSharedDoc *sharedDoc = (CSharedDoc*)g_sharedDoc;
    PartTableEntry *entry = ((IffTable<IFF_STRUCT::sPart>*)(sharedDoc->itemInventory + kPartTableOffset))->data;
    const char *message = comThinkVectorErrorStrings;
    while (((IffTable<IFF_STRUCT::sPart>*)(sharedDoc->itemInventory + kPartTableOffset))->data + ((IffTable<IFF_STRUCT::sPart>*)(sharedDoc->itemInventory + kPartTableOffset))->size != entry) {
        PartCopyRecord *record = &entry->record;
        if (self->acceptsRecord(record, filterArgument)) {
            PartCopyRecord *copy = (PartCopyRecord *)__nw__FUl(sizeof(IFF_STRUCT::sPart));
            if (copy == 0) return 0;
            *copy = *record;
            unsigned int count = self->count;
            unsigned int available = self->capacity;
            if (count < available) {
                self->count = count + 1;
                (self->items + self->count)[-1] = copy;
            } else {
                unsigned int newCapacity;
                if (kMaxProductPointers - self->capacity < 1) f04ThrowLengthError(message);
                F04Buffer buffer(&self->capacity);
                unsigned int needed = self->count + 1 - self->capacity;
                unsigned int capacity = self->capacity;
                if (needed > kMaxProductPointers - capacity) f04ThrowLengthError(message);
                if (capacity < kThreeFifthsGrowthLimit) {
                    unsigned int growth = (3 * (capacity + 1)) / 5;
                    newCapacity = capacity + f04Max(growth, needed);
                } else if (capacity < kHalfGrowthLimit) {
                    unsigned int growth = (capacity + 1) / 2;
                    newCapacity = capacity + f04Max(growth, needed);
                } else newCapacity = kMaxProductPointers;
                if (newCapacity > kMaxProductPointers) f04ThrowLengthError(message);
                {
                    PartCopyRecord **fresh = (PartCopyRecord **)appMallocDummy__FUl(newCapacity * 4);
                    buffer.offset = self->count;
                    buffer.capacity = newCapacity;
                    (fresh + buffer.offset)[buffer.count] = copy;
                    buffer.items = fresh;
                    ++buffer.count;
                    PartCopyRecord **old = self->items;
                    PartCopyRecord **end = self->items + self->count;
                    int prefix = ((char *)end - (char *)old) / 4;
                    buffer.offset -= prefix;
                    memcpy(buffer.items + buffer.offset, old, prefix * 4);
                    memset(old, 0, prefix * 4);
                    buffer.count += prefix;
                    self->count = 0;
                    old = self->items;
                    unsigned int oldCapacity = self->capacity;
                    PartCopyRecord **newItems = buffer.items;
                    buffer.items = old;
                    self->capacity = buffer.capacity;
                    buffer.capacity = oldCapacity;
                    self->items = newItems;
                    self->count = buffer.count;
                    buffer.count = 0;
                }
            }
        }
        ++entry;
    }
    return 1;
}
#pragma use_lmw_stmw off

// Return the hair-list default false result.
int ShopHairList_DefaultFalse() { return 0; }

// Return the shop product list count through its signed ABI.
int PtrArray_GetCountB(void *p) { return *(const int*)&AsShopProductListState(p)->itemCount; }

// Return the part-list default false result.
int ShopPartList_DefaultFalse() { return 0; }

extern char shopHairProductListVtable[]; // shopHairProductListVtable
extern char shopPartProductListVtable[]; // shopPartProductListVtable
extern "C" void __dl__FPv(void*);

// Initialize an empty hair-product pointer list with its borrowed context.
void ShopHairPointerArray_Initialize(void *self, void *context) {
    AsShopProductListState(self)->items = 0;
    *(void **)self = shopHairProductListVtable; // shopHairProductListVtable
    AsShopProductListState(self)->itemCount = 0;
    AsShopProductListState(self)->itemCapacity = 0;
    AsShopProductListState(self)->context = context;
}

// Initialize an empty part-product pointer list with its borrowed context.
void __ct__10CShopBaseBFPvPv(void *self, void *context) {
    AsShopProductListState(self)->items = 0;
    *(void **)self = shopPartProductListVtable; // shopPartProductListVtable
    AsShopProductListState(self)->itemCount = 0;
    AsShopProductListState(self)->itemCapacity = 0;
    AsShopProductListState(self)->context = context;
}

// Return the product list count through its signed ABI.
int CShopProductCaddie_GetCount(void *p) { return *(const int*)&AsShopProductListState(p)->itemCount; }

// Copy representation of the 0x5C-byte IffItemCommon header.
typedef IffItemCommon CommonItemCopyRecord;

struct ProductRecordInterface {
    virtual void slot08();
    virtual void slot0c();
    virtual int get(CommonItemCopyRecord *, unsigned int);
};

// Fetch a full part record through the virtual getter and copy only its common IFF header.
int CShopProductParts_CopyCommonRecord(ProductRecordInterface *object, CommonItemCopyRecord *out, unsigned int index) {
    if (out == 0) return 0;
    struct { CommonItemCopyRecord record; unsigned char padding[0x260]; } result;
    if (!object->get(&result.record, index)) return 0;
    *out = result.record;
    return 1;
}

// Fetch a full hair record through the virtual getter and copy only its common IFF header.
int CShopProductHair_vfunc_4(ProductRecordInterface *object, CommonItemCopyRecord *out, unsigned int index) {
    if (out == 0) return 0;
    struct { CommonItemCopyRecord record; unsigned char padding[0x260]; } result;
    typedef int (*GetRecord)(ProductRecordInterface*, CommonItemCopyRecord*, unsigned int);
    GetRecord getRecord = reinterpret_cast<GetRecord>((*reinterpret_cast<void***>(object))[4]);
    if (!getRecord(object, &result.record, index)) return 0;
    *out = result.record;
    return 1;
}

extern "C" void appFreeUni__FPv(void*);
extern "C" void CShopProductInterface_dtor(void*, int);

// Delete each owned record and clear the product count while retaining vector storage.
int CShopProductCaddie_DeleteAll(void *self) {
    void **cursor = AsShopProductListState(self)->items;
    unsigned int count;
    // 0x80232BFC unit: reload the end before deleting each entry.
    while (!same_pos((void **)((count = AsShopProductListState(self)->itemCount) * 4 + (char *)AsShopProductListState(self)->items), cursor))
    {
        void *record = *cursor;
        if (record != 0) {
            __dl__FPv(record);
            *cursor = 0;
        }
        cursor = cursor + 1;
    }
    AsShopProductListState(self)->itemCount = (unsigned)count - count;
    return 1;
}

#pragma opt_common_subs off
// Clear hair products through the virtual hook, release vector storage and optionally delete self.
int ShopHairProductList_dtor(void *self, int deleting) {
    if (self != 0) {
        *(void **)self = shopHairProductListVtable; // shopHairProductListVtable
        ((HairProductList *)self)->clearProducts();
        char *vectorStorage = (char *)self + 8;
        if (vectorStorage != 0 && vectorStorage != 0 && vectorStorage != 0 && vectorStorage != 0 && vectorStorage != 0 && AsShopProductListState(self)->items != 0) {
            *(int*)&AsShopProductListState(self)->itemCount = AsShopProductListState(self)->itemCount - *(int*)&AsShopProductListState(self)->itemCount;
            appFreeUni__FPv(AsShopProductListState(self)->items);
        }
        CShopProductInterface_dtor(self, 0);
        if (deleting > 0) __dl__FPv(self);
    }
    return (int)self;
}

#pragma opt_common_subs reset
#pragma opt_common_subs off
// Clear part products through the virtual hook, release vector storage and optionally delete self.
int __dt__10CShopBaseBFv(void *self, int deleting) {
    if (self != 0) {
        *(void **)self = shopPartProductListVtable; // shopPartProductListVtable
        ((HairProductList *)self)->clearProducts();
        char *vectorStorage = (char *)self + 8;
        if (vectorStorage != 0 && vectorStorage != 0 && vectorStorage != 0 && vectorStorage != 0 && vectorStorage != 0 && AsShopProductListState(self)->items != 0) {
            *(int*)&AsShopProductListState(self)->itemCount = AsShopProductListState(self)->itemCount - *(int*)&AsShopProductListState(self)->itemCount;
            appFreeUni__FPv(AsShopProductListState(self)->items);
        }
        CShopProductInterface_dtor(self, 0);
        if (deleting > 0) __dl__FPv(self);
    }
    return (int)self;
}
#pragma opt_common_subs reset

// Copy a bounds-checked hair-style IFF record into caller storage.
int CShopProductHair_CopyRecord(void *self, void *outRecord, unsigned int productIndex) {
    if (outRecord == 0) return 0;
    if (AsShopProductListState(self)->itemCount <= productIndex) return 0;
    void **records = AsShopProductListState(self)->items;
    void *record = records[productIndex];
    if (record == 0) return 0;
    Memory_Copy(outRecord, record, sizeof(IFF_STRUCT::sHairStyle));
    return 1;
}

// Copy a bounds-checked caddie IFF record into caller storage.
int CShopProductCaddie_CopyRecord(void *self, void *outRecord, unsigned int productIndex) {
    if (outRecord == 0) return 0;
    if (AsShopProductListState(self)->itemCount <= productIndex) return 0;
    void **records = AsShopProductListState(self)->items;
    void *record = records[productIndex];
    if (record == 0) return 0;
    Memory_Copy(outRecord, record, sizeof(IFF_STRUCT::sCaddie));
    return 1;
}

// Delete owned product records and clear the count, reloading bounds after each deletion.
int PtrArray_DeleteAllAndClearB(void *self) {
    void **cursor = AsShopProductListState(self)->items;
    unsigned int count;
    void **base;
    // 0x80232BFC unit: reload both list bounds for each deletion.
    while ((count = AsShopProductListState(self)->itemCount,
        base = AsShopProductListState(self)->items,
        !same_pos((char *)base + count * 4, (char *)cursor)))
    {
        void *record = *cursor;
        if (record != (void *)0) {
            __dl__FPv(record);
            *cursor = (void *)0;
        }
        cursor = cursor + 1;
    }
    AsShopProductListState(self)->itemCount = (unsigned)count - count;
    return 1;
}

} // extern "C"
