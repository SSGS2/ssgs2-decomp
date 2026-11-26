// Reference-counted exception text control block, matching the runtime ABI.
struct ShopErrorTextOwner {
    void* vtable;
    int owners;
    int references;
    void* data;
};

// Compare container cursors without changing their iterator representation.
template <class T> inline bool same_pos(const T& lhs, const T& rhs) {
    return lhs == rhs;
}

#include "sdk_protos.h"
#include "classes/ShopProductRecord.h"
#include "classes/CShopProductInterface.h"
#include "classes/CShopProductBall.h"
#include "classes/CShopProductClubSet.h"
#include "classes/CShopProductEquipItem.h"

// Four-byte pointer arrays cap element count before byte-size overflow; growth uses 3/5 then 1/2.
enum ProductVectorLimits {
    kMaxProductPointerCount = 0x3FFFFFFF,
    kThreeFifthsGrowthLimit = 0x15555555,
    kHalfGrowthLimit = 0x2AAAAAAA
};

extern "C" {

extern void s_shopBaseAVtable();
extern void s_shopBaseEVtable();
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);

struct CopyVTable {
    void* entries[4];
    int (*copy)(void*, void*);
};
struct CopyObject {
    CopyVTable* vtable;
};

struct CopyDispatch {
    virtual ~CopyDispatch();
    virtual void method0C();
    virtual int copy(void*);
};

// Common-header output schema (both common-header copy functions):
// [00,28) name/message-id bytes; [28,50) icon-name bytes; [50,54) type ID;
// [54,58) price bits; byte 58 level flags; byte 59 shop flags; [5A,5C) unresolved flags.
// dest borrows at least 0x5C writable bytes. Return 0 for null dest or failed indexed copy,
// 1 after writing the header. The large local is the virtual callee's full-record scratch buffer.
// Byte loads are intentionally expanded/volatile to preserve the original copy schedule.

#pragma opt_common_subs off
// Copy fixed 40-byte name and icon fields without adding NULs or decoding text.
// Two volatile byte loads precede each pair of stores; preserve this schedule.
static inline void CopyProductNameAndIcon(volatile const unsigned char* source, unsigned char* dest) {
    // Phase 1: copy the 40-byte name/message-id field, with no decoding or termination changes.
    unsigned char b0 = source[0x0];
    unsigned char b1 = source[0x1];
    dest[0x0] = b0;
    dest[0x1] = b1;
    unsigned char b2 = source[0x2];
    unsigned char b3 = source[0x3];
    dest[0x2] = b2;
    dest[0x3] = b3;
    unsigned char b4 = source[0x4];
    unsigned char b5 = source[0x5];
    dest[0x4] = b4;
    dest[0x5] = b5;
    unsigned char b6 = source[0x6];
    unsigned char b7 = source[0x7];
    dest[0x6] = b6;
    dest[0x7] = b7;
    unsigned char b8 = source[0x8];
    unsigned char b9 = source[0x9];
    dest[0x8] = b8;
    dest[0x9] = b9;
    unsigned char ba = source[0xa];
    unsigned char bb = source[0xb];
    dest[0xa] = ba;
    dest[0xb] = bb;
    unsigned char bc = source[0xc];
    unsigned char bd = source[0xd];
    dest[0xc] = bc;
    dest[0xd] = bd;
    unsigned char be = source[0xe];
    unsigned char bf = source[0xf];
    dest[0xe] = be;
    dest[0xf] = bf;
    unsigned char b10 = source[0x10];
    unsigned char b11 = source[0x11];
    dest[0x10] = b10;
    dest[0x11] = b11;
    unsigned char b12 = source[0x12];
    unsigned char b13 = source[0x13];
    dest[0x12] = b12;
    dest[0x13] = b13;
    unsigned char b14 = source[0x14];
    unsigned char b15 = source[0x15];
    dest[0x14] = b14;
    dest[0x15] = b15;
    unsigned char b16 = source[0x16];
    unsigned char b17 = source[0x17];
    dest[0x16] = b16;
    dest[0x17] = b17;
    unsigned char b18 = source[0x18];
    unsigned char b19 = source[0x19];
    dest[0x18] = b18;
    dest[0x19] = b19;
    unsigned char b1a = source[0x1a];
    unsigned char b1b = source[0x1b];
    dest[0x1a] = b1a;
    dest[0x1b] = b1b;
    unsigned char b1c = source[0x1c];
    unsigned char b1d = source[0x1d];
    dest[0x1c] = b1c;
    dest[0x1d] = b1d;
    unsigned char b1e = source[0x1e];
    unsigned char b1f = source[0x1f];
    dest[0x1e] = b1e;
    dest[0x1f] = b1f;
    unsigned char b20 = source[0x20];
    unsigned char b21 = source[0x21];
    dest[0x20] = b20;
    dest[0x21] = b21;
    unsigned char b22 = source[0x22];
    unsigned char b23 = source[0x23];
    dest[0x22] = b22;
    dest[0x23] = b23;
    unsigned char b24 = source[0x24];
    unsigned char b25 = source[0x25];
    dest[0x24] = b24;
    dest[0x25] = b25;
    unsigned char b26 = source[0x26];
    unsigned char b27 = source[0x27];
    dest[0x26] = b26;
    dest[0x27] = b27;
    // Phase 2: copy the 40-byte icon resource name.
    unsigned char b28 = source[0x28];
    unsigned char b29 = source[0x29];
    dest[0x28] = b28;
    dest[0x29] = b29;
    unsigned char b2a = source[0x2a];
    unsigned char b2b = source[0x2b];
    dest[0x2a] = b2a;
    dest[0x2b] = b2b;
    unsigned char b2c = source[0x2c];
    unsigned char b2d = source[0x2d];
    dest[0x2c] = b2c;
    dest[0x2d] = b2d;
    unsigned char b2e = source[0x2e];
    unsigned char b2f = source[0x2f];
    dest[0x2e] = b2e;
    dest[0x2f] = b2f;
    unsigned char b30 = source[0x30];
    unsigned char b31 = source[0x31];
    dest[0x30] = b30;
    dest[0x31] = b31;
    unsigned char b32 = source[0x32];
    unsigned char b33 = source[0x33];
    dest[0x32] = b32;
    dest[0x33] = b33;
    unsigned char b34 = source[0x34];
    unsigned char b35 = source[0x35];
    dest[0x34] = b34;
    dest[0x35] = b35;
    unsigned char b36 = source[0x36];
    unsigned char b37 = source[0x37];
    dest[0x36] = b36;
    dest[0x37] = b37;
    unsigned char b38 = source[0x38];
    unsigned char b39 = source[0x39];
    dest[0x38] = b38;
    dest[0x39] = b39;
    unsigned char b3a = source[0x3a];
    unsigned char b3b = source[0x3b];
    dest[0x3a] = b3a;
    dest[0x3b] = b3b;
    unsigned char b3c = source[0x3c];
    unsigned char b3d = source[0x3d];
    dest[0x3c] = b3c;
    dest[0x3d] = b3d;
    unsigned char b3e = source[0x3e];
    unsigned char b3f = source[0x3f];
    dest[0x3e] = b3e;
    dest[0x3f] = b3f;
    unsigned char b40 = source[0x40];
    unsigned char b41 = source[0x41];
    dest[0x40] = b40;
    dest[0x41] = b41;
    unsigned char b42 = source[0x42];
    unsigned char b43 = source[0x43];
    dest[0x42] = b42;
    dest[0x43] = b43;
    unsigned char b44 = source[0x44];
    unsigned char b45 = source[0x45];
    dest[0x44] = b44;
    dest[0x45] = b45;
    unsigned char b46 = source[0x46];
    unsigned char b47 = source[0x47];
    dest[0x46] = b46;
    dest[0x47] = b47;
    unsigned char b48 = source[0x48];
    unsigned char b49 = source[0x49];
    dest[0x48] = b48;
    dest[0x49] = b49;
    unsigned char b4a = source[0x4a];
    unsigned char b4b = source[0x4b];
    dest[0x4a] = b4a;
    dest[0x4b] = b4b;
    unsigned char b4c = source[0x4c];
    unsigned char b4d = source[0x4d];
    dest[0x4c] = b4c;
    dest[0x4d] = b4d;
    unsigned char b4e = source[0x4e];
    unsigned char b4f = source[0x4f];
    dest[0x4e] = b4e;
    dest[0x4f] = b4f;
}

} // extern "C"

// Copy the selected club-set common IFF header; preserve bytewise name/icon loads.
int CShopProductClubSet::CopyCommonHeader(unsigned char* dest, unsigned int index) {
    if (dest == 0)
        return 0;
    struct Temp {
        volatile unsigned char nameAndIcon[0x50];
        unsigned int typeId;
        unsigned int pricePangBits;
        unsigned char levelFlags;
        unsigned char shopFlags;
        unsigned short commonFlags;
        unsigned char padding[0x2bc - 0x5c];
    } temp;
    typedef int (*CopyByIndex)(CopyObject*, void*, unsigned int);
    CopyObject* source = (CopyObject*)this;
    if (((CopyByIndex)source->vtable->copy)(source, &temp, index) == 0)
        return 0;
    CopyProductNameAndIcon(temp.nameAndIcon, dest);
    // Phase 3: copy scalar header fields, including the final common-header flag word.
    ((ShopProductRecord*)dest)->typeId = temp.typeId;
    *(unsigned int*)&((IFF_STRUCT::sClubSet*)dest)->common.price = temp.pricePangBits;
    ((IFF_STRUCT::sClubSet*)dest)->common.levelFlags = temp.levelFlags;
    ((IFF_STRUCT::sClubSet*)dest)->common.shopFlags = temp.shopFlags;
    ((IFF_STRUCT::sClubSet*)dest)->common.commonFlagWord = temp.commonFlags;
    return 1;
}

extern "C" {
#pragma opt_common_subs reset
#pragma opt_common_subs off
// Copy the selected ball common IFF header; preserve bytewise name/icon loads.
int CShopProductBall_CopyCommonRecord(CopyObject* source, unsigned char* dest, unsigned int index) {
    if (dest == 0)
        return 0;
    struct Temp {
        volatile unsigned char nameAndIcon[0x50];
        unsigned int typeId;
        unsigned int pricePangBits;
        unsigned char levelFlags;
        unsigned char shopFlags;
        unsigned short commonFlags;
        unsigned char padding[0x2c0 - 0x5c];
    } temp;
    typedef int (*CopyByIndex)(CopyObject*, void*, unsigned int);
    if (((CopyByIndex)source->vtable->copy)(source, &temp, index) == 0)
        return 0;
    CopyProductNameAndIcon(temp.nameAndIcon, dest);
    // Phase 3: copy scalar common-header fields.
    ((ShopProductRecord*)dest)->typeId = temp.typeId; // CShopProductBall_CopyCommonRecord: +0x50
    *(unsigned int*)&((IFF_STRUCT::sBall*)dest)->common.price = temp.pricePangBits;
    ((IFF_STRUCT::sBall*)dest)->common.levelFlags = temp.levelFlags;
    ((IFF_STRUCT::sBall*)dest)->common.shopFlags = temp.shopFlags;
    ((IFF_STRUCT::sBall*)dest)->common.commonFlagWord = temp.commonFlags; // Preserve the final common-header flag word.
    return 1;
}

#pragma opt_common_subs reset
// The list owns each allocated record and its pointer array. DeleteAll frees records only;
// the base destructor subsequently frees the array. Count-minus-volatile-count stores zero,
// and repeated member-address guards below are retained compiler-shape scaffolding.
struct PtrArray {
    unsigned int interfaceWords[2];
    void** data;
    unsigned int count;
};

// Copy a valid indexed ball record into caller storage.
int CShopProductBall_CopyRecord(PtrArray* array, void* dest, unsigned int index) {
    if (dest == 0)
        return 0;
    if (array->count <= index)
        return 0;
    void* entry = array->data[index];
    if (entry == 0)
        return 0;
    Memory_Copy(dest, entry, sizeof(IFF_STRUCT::sBall));
    return 1;
}

// Copy a valid indexed consumable record into caller storage.
int CShopProductEquipItem_CopyRecord(PtrArray* array, void* dest, unsigned int index) {
    if (dest == 0)
        return 0;
    if (array->count <= index)
        return 0;
    void* entry = array->data[index];
    if (entry == 0)
        return 0;
    Memory_Copy(dest, entry, sizeof(IFF_STRUCT::sItem));
    return 1;
}

// Delete every owned ball record and clear the logical pointer count.
int CShopProductBall_DeleteAll(PtrArray* array) {
    void** cursor = array->data;
    while (!same_pos(array->data + array->count, cursor)) {
        if (*cursor != 0) {
            __dl__FPv(*cursor);
            *cursor = 0;
        }
        ++cursor;
    }
    unsigned int count = array->count;
    array->count = count - *(volatile unsigned int*)&count;
    return 1;
}

// Delete every owned consumable record and clear the logical pointer count.
int CShopProductEquipItem_DeleteAll(PtrArray* array) {
    void* zero = 0;
    void** cursor = array->data;
    while (!same_pos(array->data + array->count, cursor)) {
        if (*cursor != 0) {
            __dl__FPv(*cursor);
            *cursor = zero;
        }
        ++cursor;
    }
    unsigned int count = array->count;
    array->count = count - *(volatile unsigned int*)&count;
    return 1;
}

struct VTable {
    void* entries[3];
    void (*destroy)(void*);
};
struct DtorDispatch {
    virtual ~DtorDispatch();
    virtual void destroy();
};
struct DtorObject {
    VTable* vtable;
    unsigned int productFilter;
    void* data;
    unsigned int count;
};

// Destroy owned products, release pointer storage and optionally free the shop base object.
void* __dt__10CShopBaseAFv(DtorObject* self, int deallocate) {
    if (self != 0) {
        self->vtable = (VTable*)&s_shopBaseAVtable;
        ((DtorDispatch*)self)->destroy();
        if (&self->data != 0 && &self->data != 0 && &self->data != 0 && &self->data != 0 && &self->data != 0 &&
            self->data != 0) {
            unsigned int count = self->count;
            self->count = count - *(volatile unsigned int*)&count;
            appFreeUni__FPv(self->data);
        }
        ((CShopProductInterface*)self)->Destroy(0);
        if (deallocate > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy owned products, release pointer storage and optionally free the shop base object.
void* __dt__10CShopBaseEFv(DtorObject* self, int deallocate) {
    if (self != 0) {
        self->vtable = (VTable*)&s_shopBaseEVtable;
        ((DtorDispatch*)self)->destroy();
        if (&self->data != 0 && &self->data != 0 && &self->data != 0 && &self->data != 0 && &self->data != 0 &&
            self->data != 0) {
            unsigned int count = self->count;
            self->count = count - *(volatile unsigned int*)&count;
            appFreeUni__FPv(self->data);
        }
        ((CShopProductInterface*)self)->Destroy(0);
        if (deallocate > 0)
            __dl__FPv(self);
    }
    return self;
}

// Return false for this unsupported shop-product interface operation.
int CShopBaseA_ReturnFalse() {
    return 0;
}

// Return the number of product pointers in the ball list.
int CShopProductBall_GetCount(int* self) {
    return self[3];
}

// Return false for this unsupported shop-product interface operation.
int CShopBaseE_ReturnFalse() {
    return 0;
}

struct ManagerVTable802356DC {
    void* e0;
    void* e4;
    void* e8;
    int (*check)(void*);
    void* e10;
    void* e14;
    void* e18;
    void* e1c;
    int (*insert)(void*, void*, int);
};

// Shared-document overlay: the IFF tables begin inside itemInventory at 0xA04.
struct ShopInventoryTables {
    ManagerVTable802356DC* vtable;
    char pad04[0xA48 - 4];
    unsigned char* clubSetTable;
    unsigned int clubSetCount;
    unsigned int clubSetCapacity;
    unsigned char* ballTable;
    unsigned int ballCount;
};
extern ShopInventoryTables* g_sharedDoc;

// Aggregate copy view of sClubSet; the split word pairs retain the original copy schedule.
struct ClubSetCopyRecord {
    struct Prefix {
        unsigned int commonHeaderWords[23];
        unsigned int power;
        unsigned int control;
        unsigned int accuracy;
        unsigned int spin;
        unsigned int curve;
        struct WordPair {
            unsigned int a, b;
        } woodAndIronIds, wedgeAndPutterIds;
    } prefix;
    struct HalfPair {
        unsigned short a, b;
    } powerAndControlSlots, accuracyAndSpinSlots;
    unsigned short curveSlots;
    unsigned short recordFlags;
};

// Virtual-call view of a product list: check is slot 0x0C and the acceptance predicate is slot 0x20.
// The shop product classes declare no virtual functions, so the list code calls through this view.
struct ProductListVirtualView {
    virtual ~ProductListVirtualView();
    virtual int check();
    virtual void method10();
    virtual void method14();
    virtual void method18();
    virtual void method1c();
    virtual int insert(void*, int);
};

extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void* memset(void*, int, unsigned int);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void __dt__Q23std12length_errorFv(void*);
extern "C" void __dla__FPv(void*);
extern char comThinkVectorErrorStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct ThrowObj802356DC {
    void* vtable;
    void* data;
    void* ref;
};
struct ThrowGuard802356DC {
    unsigned* a;
    unsigned* b;
};

#define THROW_VECTOR_LENGTH_ERROR()                                                                         \
    do {                                                                                                    \
        char* errorText = comThinkVectorErrorStrings;                                                       \
        ThrowObj802356DC errorObject;                                                                       \
        ThrowGuard802356DC guard;                                                                           \
        char emptyChar = 0;                                                                                 \
        errorObject.vtable = __vt__Q23std11logic_error;                                                     \
        unsigned errorLength = strlen(errorText) + 1;                                                       \
        char* errorCopy = (char*)__nwa__FUl((errorLength << 1) - errorLength);                              \
        errorObject.data = errorCopy;                                                                       \
        guard.a = (unsigned*)errorCopy;                                                                     \
        guard.b = (unsigned*)&emptyChar;                                                                    \
        void* errorOwner = __nw__FUl(0x10);                                                                 \
        if (errorOwner != 0) {                                                                              \
            ((ShopErrorTextOwner*)errorOwner)->owners = 1;                                                  \
            ((ShopErrorTextOwner*)errorOwner)->references = 1;                                              \
            *(void**)errorOwner = s_sharedCharDeleterVtable;                                                \
            ((ShopErrorTextOwner*)errorOwner)->data = errorCopy;                                            \
        }                                                                                                   \
        errorObject.ref = errorOwner;                                                                       \
        guard.a = 0;                                                                                        \
        char* stale = 0;                                                                                    \
        if (stale != 0) {                                                                                   \
            __dla__FPv(stale);                                                                              \
        }                                                                                                   \
        strcpy((char*)errorObject.data, errorText);                                                         \
        errorObject.vtable = __vt__Q23std12length_error;                                                    \
        __throw(comThinkVectorErrorStrings + 0x14, &errorObject, (void (*)())__dt__Q23std12length_errorFv); \
    } while (0)

template <class T>
// Return a borrowed reference to the larger argument, choosing a on equality.
const T& max_ref(const T& a, const T& b) {
    return (a < b) ? b : a;
}

// Aggregate copy view of sBall; unknown trailing words keep their recorded offsets.
struct BallCopyRecord {
    unsigned int commonHeaderWords[23];
    char effectFiles[7 * 40];
    char effectBones[7 * 40];
    char model[40];
    unsigned int f2b4;
    unsigned int f2b8;
    short f2bc;
    unsigned short f2be;
};

struct PointerVectorGuard {
    ClubSetCopyRecord** data;
    unsigned size;
    unsigned capacity;
    unsigned* allocator;
    unsigned tail;
    // Begin a pending pointer-vector owner with no allocation and the caller's allocator token.
    PointerVectorGuard(unsigned* a) : data(0), size(0), capacity(0), allocator(a), tail(0) {}
    // Free pointer storage only; this guard never deletes pointed-to product records.
    ~PointerVectorGuard() {
        if (data != 0) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct TextArrayGuard {
    char* data;
    char* allocator;
    // Release an exception-text array unless its allocation has been transferred to a control block.
    ~TextArrayGuard() {
        if (data != 0)
            __dla__FPv(data);
    }
};

} // extern "C"

namespace std {
struct exception {
    void* vtable;
};

struct logic_error : exception {
    char* data;
    void* ref;
    // Allocate exception text and its shared owner; the derived constructor copies the message.
    logic_error(const char* text) {
        vtable = __vt__Q23std11logic_error;
        char allocator = 0;
        unsigned length = strlen(text) + 1;
        char* copy = (char*)__nwa__FUl((length << 1) - length);
        data = copy;
        TextArrayGuard guard;
        guard.data = copy;
        guard.allocator = &allocator;
        void* owner = __nw__FUl(0x10);
        if (owner != 0) {
            ((ShopErrorTextOwner*)owner)->owners = 1;
            ((ShopErrorTextOwner*)owner)->references = 1;
            *(void**)owner = s_sharedCharDeleterVtable;
            (*(char**)&((ShopErrorTextOwner*)owner)->data) = copy;
        }
        ref = owner;
        guard.data = 0;
    }
    ~logic_error();
};

struct length_error : logic_error {
    // Copy the length-error text into its owned buffer and install the derived vtable.
    length_error(const char* text) : logic_error(text) {
        strcpy(data, text);
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
};
}

extern "C" {

// A pending vector owns its allocation until the completed vector is swapped in.
#define THROW_TARGET_LENGTH_ERROR() throw std::length_error("vector length error")

} // extern "C"

#pragma push
#pragma use_lmw_stmw on
// Filter the shared club-set table and append owned copies of accepted IFF records.
// filter is passed unchanged to the acceptance predicate at vtable slot 0x20, which decides
// acceptance; this function performs the allocation. Returns 0 for failed check or record
// allocation, 1 after scanning. Existing products remain; failure may leave earlier accepted
// records appended. Source records are borrowed, never freed.
int CShopProductClubSet::CollectAcceptedRecords(register int filter) {
    ProductListVirtualView* list = (ProductListVirtualView*)this;
    // Phase 1: validate the receiver and visit keyed inventory records (4-byte key + 0x8C payload).
    if (list->check() == 0)
        return 0;
    ShopInventoryTables* inventory = g_sharedDoc;
    unsigned int record = (unsigned int)inventory->clubSetTable;
    while ((unsigned int)(inventory->clubSetTable + inventory->clubSetCount * 0x90) != record) {
        ClubSetCopyRecord* source = (ClubSetCopyRecord*)(record + 4);
        if (list->insert(source, filter) != 0) {
            // Phase 2: allocate/copy an independent accepted record, preserving halfword loads.
            ClubSetCopyRecord* product = (ClubSetCopyRecord*)__nw__FUl(0x8c);
            if (product == 0)
                return 0;
            product->prefix = source->prefix;
            volatile ClubSetCopyRecord::HalfPair* halves = &source->powerAndControlSlots;
            unsigned short h80 = halves->a;
            unsigned short h82 = halves->b;
            product->powerAndControlSlots.a = h80;
            product->powerAndControlSlots.b = h82;
            halves = &source->accuracyAndSpinSlots;
            unsigned short h84 = halves->a;
            unsigned short h86 = halves->b;
            product->accuracyAndSpinSlots.a = h84;
            product->accuracyAndSpinSlots.b = h86;
            product->curveSlots = source->curveSlots;
            product->recordFlags = source->recordFlags;
            // Phase 3: append directly if possible; otherwise build replacement pointer storage.
            unsigned int currentSize = size;
            if (currentSize < capacity) {
                size = currentSize + 1;
                ((ClubSetCopyRecord**)begin)[size - 1] = product;
            } else {
                if (kMaxProductPointerCount - capacity < 1) {
                    THROW_TARGET_LENGTH_ERROR();
                }
                unsigned int additionalSlots = size + 1 - capacity;
                PointerVectorGuard pending(&capacity);
                unsigned int currentCapacity = capacity;
                if (additionalSlots > kMaxProductPointerCount - currentCapacity) {
                    THROW_TARGET_LENGTH_ERROR();
                }
                unsigned int newCapacity;
                if (currentCapacity < kThreeFifthsGrowthLimit) {
                    unsigned int grow = (currentCapacity + 1) * 3 / 5;
                    const unsigned int& chosen = (grow < additionalSlots) ? additionalSlots : grow;
                    newCapacity = currentCapacity + chosen;
                } else if (currentCapacity < kHalfGrowthLimit) {
                    unsigned int grow = (currentCapacity + 1) / 2;
                    const unsigned int& chosen = (grow < additionalSlots) ? additionalSlots : grow;
                    newCapacity = currentCapacity + chosen;
                } else {
                    newCapacity = kMaxProductPointerCount;
                }
                if (newCapacity > kMaxProductPointerCount) {
                    THROW_TARGET_LENGTH_ERROR();
                }
                ClubSetCopyRecord** allocatedData = (ClubSetCopyRecord**)appMallocDummy__FUl(newCapacity * 4);
                pending.tail = size;
                pending.capacity = newCapacity;
                (allocatedData + pending.tail)[pending.size] = product;
                pending.data = allocatedData;
                ++pending.size;
                // Transfer pointers, not records. The pending guard will free only the old array.
                ClubSetCopyRecord** old = (ClubSetCopyRecord**)begin;
                int count = ((char*)(old + size) - (char*)old) / 4;
                pending.tail -= count;
                memcpy(pending.data + pending.tail, old, count * 4);
                memset(old, 0, count * 4);
                pending.size += count;
                size = 0;
                ClubSetCopyRecord** oldData = (ClubSetCopyRecord**)begin;
                ClubSetCopyRecord** newData = pending.data;
                pending.data = oldData;
                unsigned oldCapacity = capacity;
                capacity = pending.capacity;
                pending.capacity = oldCapacity;
                begin = (void**)newData;
                size = pending.size;
                pending.size = 0;
            }
        }
        record += 0x90;
    }
    return 1;
}
#pragma pop

#define THROW_RECORD_LENGTH_ERROR()                     \
    do {                                                \
        throw std::length_error("vector length error"); \
    } while (0)

#pragma push
#pragma use_lmw_stmw on
// Filter the shared ball table and append owned copies of accepted IFF records.
// Same filter, return and partial-append contract as the club-set builder.
int CShopProductBall::AppendFilteredRecords(register int filter) {
    // Phase 1: validate and scan keyed ball records (4-byte key + 0x2C0 payload).
    ProductListVirtualView* list = (ProductListVirtualView*)this;
    if (list->check() == 0)
        return 0;
    ShopInventoryTables* inventory = g_sharedDoc;
    unsigned int record = (unsigned int)inventory->ballTable;
    while ((unsigned int)(inventory->ballTable + inventory->ballCount * 0x2C4) != record) {
        BallCopyRecord* source = (BallCopyRecord*)(record + 4);
        if (list->insert(source, filter) != 0) {
            // Phase 2: copy an accepted borrowed payload into a newly owned ball record.
            BallCopyRecord* product = (BallCopyRecord*)__nw__FUl(0x2C0);
            if (product == 0)
                return 0;
            *product = *source;
            // Phase 3: append, growing only the pointer array when capacity is exhausted.
            unsigned int currentSize = size;
            if (currentSize < capacity) {
                size = currentSize + 1;
                ((ClubSetCopyRecord**)begin)[size - 1] = (ClubSetCopyRecord*)product;
            } else {
                unsigned int threeFifthsGrowth;
                unsigned int growHalf;
                unsigned int additionalSlots;
                if (kMaxProductPointerCount - capacity < 1) {
                    THROW_RECORD_LENGTH_ERROR();
                }
                additionalSlots = size + 1 - capacity;
                PointerVectorGuard pending(&capacity);
                unsigned int currentCapacity = capacity;
                if (additionalSlots > kMaxProductPointerCount - currentCapacity) {
                    THROW_RECORD_LENGTH_ERROR();
                }
                unsigned int newCapacity;
                if (currentCapacity < kThreeFifthsGrowthLimit) {
                    threeFifthsGrowth = (currentCapacity + 1) * 3 / 5;
                    const unsigned int& chosen =
                        (threeFifthsGrowth < additionalSlots) ? additionalSlots : threeFifthsGrowth;
                    newCapacity = currentCapacity + chosen;
                } else if (currentCapacity < kHalfGrowthLimit) {
                    growHalf = (currentCapacity + 1) / 2;
                    const unsigned int& chosen = (growHalf < additionalSlots) ? additionalSlots : growHalf;
                    newCapacity = currentCapacity + chosen;
                } else {
                    newCapacity = kMaxProductPointerCount;
                }
                if (newCapacity > kMaxProductPointerCount) {
                    THROW_RECORD_LENGTH_ERROR();
                }
                ClubSetCopyRecord** allocatedData = (ClubSetCopyRecord**)appMallocDummy__FUl(newCapacity * 4);
                pending.tail = size;
                pending.capacity = newCapacity;
                (allocatedData + pending.tail)[pending.size] = (ClubSetCopyRecord*)product;
                pending.data = allocatedData;
                ++pending.size;
                // Transfer record pointers and swap arrays; pending owns the previous array after swap.
                ClubSetCopyRecord** old = (ClubSetCopyRecord**)begin;
                int count = ((char*)(old + size) - (char*)old) / 4;
                pending.tail -= count;
                memcpy(pending.data + pending.tail, old, count * 4);
                memset(old, 0, count * 4);
                pending.size += count;
                size = 0;
                ClubSetCopyRecord** oldData = (ClubSetCopyRecord**)begin;
                ClubSetCopyRecord** newData = pending.data;
                pending.data = oldData;
                unsigned oldCapacity = capacity;
                capacity = pending.capacity;
                pending.capacity = oldCapacity;
                begin = (void**)newData;
                size = pending.size;
                pending.size = 0;
            }
        }
        record += 0x2C4;
    }
    return 1;
}
#pragma pop

// Return the product count, stored as a pointer-sized value at +0xC. It is not an address.
int* CShopProductEquipItem::GetCount() {
    return (int*)size;
}

extern "C" {

// Initialize an empty product-pointer list with its filter and base-A vtable.
void __ct__10CShopBaseAFPvPv(unsigned int* self, unsigned int value) {
    self[2] = 0;
    self[0] = (unsigned int)&s_shopBaseAVtable;
    self[3] = 0;
    self[4] = 0;
    self[1] = value;
}

// Initialize an empty product-pointer list with its filter and base-E vtable.
void __ct__10CShopBaseEFPv(unsigned int* self, unsigned int value) {
    self[2] = 0;
    self[0] = (unsigned int)&s_shopBaseEVtable;
    self[3] = 0;
    self[4] = 0;
    self[1] = value;
}

} // extern "C"
