#include <decomp/local_class.h>
#include <decomp/ct_member.h>
// Shop caddie/caddie-item product lists: filter inventory IFF tables into owned
// copies, expose counts/common headers/full records, then release their storage.
// API/mod lookup:
// - Which products appear? CShopProductCaddie_LoadFromInventory /
//   CShopProductCaddieItem_LoadFromInventory filter inventory rows; virtual accepts
//   slot 6 receives each borrowed row and the caller's filterContext unchanged.
// - Record fields: iff/IffStruct.h (sCaddie, sCadItem, sClubSet, IffItemCommon);
//   CItemManager.h supplies caddies/caddieItems tables; CSharedDoc.h the inventory.
// - Selection/copy: *_CopyRecord copies the whole IFF record; *_CopyCommonRecord copies
//   just the common header via virtual slot 2; *_GetCount returns records.
// - Lifetime/reset: *_DeleteAll clears owned rows but keeps capacity;
//   CShopBaseC/D destructors also free the pointer array. Product interface/base
//   dispatch tables s_shopBaseCVtable/s_shopBaseDVtable remain external companions.
// This is product presentation/storage, not purchase-price or inventory mutation.
#include "classes/CSharedDoc.h"
#include "classes/CItemManager.h"
#include "iff/IffStruct.h"

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char comThinkVectorErrorStrings[];
}
}

// Compare iterator positions without changing their pointer representation.
template <class T> inline bool SameProductIterator(const T& a, const T& b) { return a == b; }
// Translation unit 0x80234124-0x802356DC (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern char s_shopBaseCVtable[];
extern char s_shopBaseDVtable[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char comThinkVectorErrorStrings[];
extern void Memory_Copy(void*, const void*, unsigned int);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void CShopProductInterface_dtor(void*, int);
extern CSharedDoc* g_sharedDoc;
extern void* __nw__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __throw(void*, void*, void*);
extern void __dt__Q23std12length_errorFv(void*);
extern unsigned int strlen(const char*);
extern char* strcpy(char*, const char*);

// Pointer-vector allocation limits (32-bit byte size / 4-byte pointer).
enum ProductVectorLimits { kProductMaxEntries = 0x3FFFFFFF,
    kProductFastGrowthLimit = 0x15555555, kProductHalfGrowthLimit = 0x2AAAAAAA };
enum ProductStorageConstants { kProductPointerBytes = sizeof(void*), kLengthErrorTagOffset = 0x14,
    kCommonNameOffset = IFF_OFFSET(IffItemCommon, name), kCommonIconOffset = IFF_OFFSET(IffItemCommon, icon) };
// Product-list contract: self owns a {data, size, capacity} pointer vector at +8
// and each separately allocated IFF record it contains. Counts are records.
// Constructor context and filterContext are borrowed; VecObj.context is at +4.
// Loading first calls virtual
// slot 1, then appends accepted rows. The base C/D vtables map this slot to
// DeleteAll; it also runs during destruction. clearOwnedRows is destructive.
// Empty/fully rejected tables still return 1 after that hook succeeds.
// Loading is incremental: it first clears the prior contents, then appends each
// accepted copy. Return 0 on a failed clear hook or null row allocation; earlier
// rows appended by this load remain owned. Capacity errors throw length_error.
// Loader outline: clear -> filter borrowed row -> allocate/copy accepted row ->
// append pointer (fast or guarded grow/transfer/commit). CaddieItem and Caddie
// differ only in inventory table, record width and end-comparison operand order.
// Keep their partial-load failure and row ownership policy synchronized.
// Required inputs: self is a fully constructed C/D product list with valid virtual
// slots 1 and 6, 0 <= size <= capacity, and capacity writable pointer slots (or an
// empty null allocation). g_sharedDoc and its itemInventory must be live; each IFF
// table must have contiguous size entries. Neither self nor inventory is null-
// checked. filterContext may be null only if the filter override accepts it.
// All inventory rows/context are borrowed. Filters must not invalidate inventory
// storage during traversal or reenter mutation of this list; the end is reloaded.
// Growth failure is NOT transactional: ownedRow is allocated before length checks
// and pointer-array allocation, without a row deletion guard. A thrown length_error
// or allocator exception can leak that unappended row. F18VecBuffer frees only its
// pending pointer ARRAY (or the old array after commit), never row objects. Prior
// committed rows remain owned; no rollback restores the list cleared at entry.
// appMallocDummy returning null is unchecked before stores; do not promise a clean
// 0 result for pointer-array failure. Only a null row allocation returns 0 here.
// Allocation/growth and destruction are intentionally expanded at their call
// sites: factoring the append path into one inline helper changes register
// allocation and exception tables under this compiler. Follow the phases below.
// CopyRecord requires a destination large enough for its full IFF type; CopyCommonRecord
// copies only IffItemCommon. Both return 0 on invalid selection, 1 after copying.
// DeleteAll frees records and keeps pointer capacity; destruction also frees data.
// Word-copy view of IFF_STRUCT::sCadItem; keep the original copy granularity and trailing flags.
struct Record88 {
    unsigned int commonWords[sizeof(IffItemCommon) / sizeof(unsigned int)];
    char model[40];
    short trailingFlags;
    unsigned short trailingValue;
    // Copy the complete caddie-item record from borrowed source storage.
    Record88(const void* src) { *this = *static_cast<const Record88*>(src); }
};

struct VecObj {
    void** vtable;
    void* context; // Borrowed context at +4; never freed here.
    void** begin;
    unsigned int size;
    unsigned int capacity;
};


// Word-copy view of IFF_STRUCT::sCaddie; the six shorts are five stat bonuses and record padding.
struct Record90 {
    unsigned int commonWords[sizeof(IffItemCommon) / sizeof(unsigned int)];
    char model[40];
    unsigned short statsAndPadding[6];
    // Copy the complete caddie record from borrowed source storage.
    Record90(const void* src) { *this = *static_cast<const Record90*>(src); }
};

// Aliases keep constructor/exception symbols unchanged while giving loaders product roles.
typedef Record88 CaddieItemProductRecordView;
typedef Record90 CaddieProductRecordView;

struct PtrVector { void** begin; void** end; void** capacity; };

extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct F18ErrorBase {
    void* table;
    // Initialize the runtime logic-error vtable.
    F18ErrorBase() { table = __vt__Q23std11logic_error; }
    // Destroy the exception base through its runtime destructor.
    ~F18ErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
struct F18Allocation {
    char* pointer;
    char* allocator;
    // Guard error-text array ownership until the shared control block takes it.
    F18Allocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Guard error-text array ownership until the shared control block takes it.
    ~F18Allocation() { if (pointer) __dla__FPv(pointer); }
};
struct F18ControlBase {
    void* table;
    unsigned int strong, weak;
    // Initialize the shared text deleter and strong/weak counts to one.
    F18ControlBase() { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; }
};
struct F18Control : F18ControlBase {
    char* pointer;
    // Store the owned error-text array in its shared control block.
    F18Control(char* p) : pointer(p) {}
};
namespace std {
struct length_error : F18ErrorBase {
    char* text;
    void* control;
    // Allocate the standard vector-length message and shared deleter.
    length_error() {
        const char* message = comThinkVectorErrorStrings;
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        unsigned int n = strlen(message) + 1;
        text = static_cast<char*>(__nwa__FUl(2 * n - n));
        {
            char* p = text;
            F18Allocation allocation(p, &allocator.value);
            control = new F18Control(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}

// Select the larger required/growth count by reference.
inline const unsigned int& F18Max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

struct F18VecBuffer {
    void* data;
    volatile unsigned int size;
    unsigned int capacity;
    unsigned int* owner;
    volatile unsigned int offset;
    // Initialize an empty growth guard borrowing the destination capacity counter.
    F18VecBuffer(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free the pending or swapped-out allocation still owned by this guard.
    ~F18VecBuffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};

struct F18Vec {
    void* vtable;
    void* context;
    void** data;
    unsigned int size;
    unsigned int capacity;
};

struct F18Interface {
// Slot 1 is DeleteAll (nonzero = success).
    virtual void slot0();
    virtual int clearOwnedRows(); // Slot 1: DeleteAll; nonzero means success.
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual int accepts(void*, void*);
    void* context;
    void** data;
    unsigned int size;
    unsigned int capacity;
};

typedef F18Interface CaddieProductListInterface;

// Clear owned product rows through destructive vtable slot 1; retain pointer capacity.
static inline int ClearCaddieProductRows(void* products) {
    return static_cast<CaddieProductListInterface*>(products)->clearOwnedRows();
}

#pragma push
#pragma use_lmw_stmw on
// filterContext passes unchanged to virtual slot 6; nonzero accepts a borrowed input row.
// Filter the caddie-item IFF table into owned record copies; preserve partial-load failure behavior.
int CShopProductCaddieItem_LoadFromInventory(void* self, void* filterContext) {
    // Phase 0: discard owned rows through DeleteAll, retaining pointer capacity.
    if (ClearCaddieProductRows(self) == 0)
        return 0;
    CaddieProductListInterface* products = static_cast<CaddieProductListInterface*>(self);
    CSharedDoc* shared = g_sharedDoc;
    IffTableEntry<IFF_STRUCT::sCadItem>* entry = ((CItemManager*)shared->itemInventory)->caddieItems.data;
    while (entry != ((CItemManager*)shared->itemInventory)->caddieItems.data +
                        ((CItemManager*)shared->itemInventory)->caddieItems.size) {
        void* sourceRecord = &entry->record;
        // Phase 1: filter borrowed inventory rows; only accepted rows are copied.
        if (static_cast<CaddieProductListInterface*>(self)->accepts(sourceRecord, filterContext) != 0) {
            CaddieItemProductRecordView* ownedRow = static_cast<CaddieItemProductRecordView*>(__nw__FUl(sizeof(CaddieItemProductRecordView)));
            if (ownedRow == 0)
                return 0;
            *ownedRow = *reinterpret_cast<const CaddieItemProductRecordView*>(sourceRecord);
            // Phase 2a: append directly if the current pointer array has a free slot.
            unsigned int rowCount = products->size;
            if (rowCount < products->capacity) {
                void** pointers;
                unsigned int nextRowCount;
                nextRowCount = rowCount + 1;
                pointers = products->data;
                *(volatile unsigned int*)&products->size = nextRowCount;
                (pointers + nextRowCount)[-1] = ownedRow;
            } else {
                // Phase 2b: validate pointer-count limits before allocating a replacement.
                if (kProductMaxEntries - products->capacity < 1) {
                    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(exception_runtime::comThinkVectorErrorStrings + kLengthErrorTagOffset, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                unsigned int additionalSlots = 1 + products->size - products->capacity;
                // Guard the new pointer array until copied entries are committed.
                F18VecBuffer replacement(&products->capacity);
                unsigned int newCapacity = products->capacity;
                if (additionalSlots > kProductMaxEntries - newCapacity) {
                    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(exception_runtime::comThinkVectorErrorStrings + kLengthErrorTagOffset, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                // Grow by max(3*(capacity+1)/5,needed), then by half at large
                // capacities, then saturate at the representable pointer limit.
                if (newCapacity < kProductFastGrowthLimit) {
                    newCapacity += F18Max((3 * (newCapacity + 1)) / 5, additionalSlots);
                } else if (newCapacity < kProductHalfGrowthLimit) {
                    newCapacity += F18Max((newCapacity + 1) / 2, additionalSlots);
                } else {
                    newCapacity = kProductMaxEntries;
                }
                if (newCapacity > kProductMaxEntries) {
                    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(exception_runtime::comThinkVectorErrorStrings + kLengthErrorTagOffset, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                void** newPointers = static_cast<void**>(appMallocDummy__FUl(newCapacity * kProductPointerBytes));
                unsigned int oldRowCount = products->size;
                replacement.offset = oldRowCount;
                replacement.data = newPointers;
                (newPointers + oldRowCount)[replacement.size++] = ownedRow;
                void** oldPointers = products->data;
                replacement.capacity = newCapacity;
                int pointersToMove = (reinterpret_cast<char*>(oldPointers + products->size) - reinterpret_cast<char*>(oldPointers)) / kProductPointerBytes;
                int insertionStart = oldRowCount - pointersToMove;
                replacement.offset = insertionStart;
                unsigned int pointerBytes = pointersToMove * kProductPointerBytes;
                // Phase 3: transfer existing pointers into the replacement,
                // after placing the new row at oldRowCount. No row is copied again.
                memcpy(newPointers + insertionStart, oldPointers, pointerBytes);
                memset(oldPointers, 0, pointerBytes);
                unsigned int zero = 0;
                unsigned int newSize = replacement.size + pointersToMove;
                replacement.size = newSize;
                oldPointers = products->data;
                void** committedPointers = static_cast<void**>(replacement.data);
                unsigned int oldPointerCapacity = products->capacity;
                products->size = zero;
                replacement.data = oldPointers;
                products->capacity = replacement.capacity;
                replacement.capacity = oldPointerCapacity;
                // Phase 4: commit data/size/capacity. The guard now owns only
                // the cleared old pointer allocation and releases it at scope exit.
                products->data = committedPointers;
                products->size = newSize;
                replacement.size = zero;
            }
        }
        ++entry;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// filterContext passes unchanged to virtual slot 6; nonzero accepts a borrowed input row.
// Filter the caddie IFF table into owned record copies; reload the table end after each product.
int CShopProductCaddie_LoadFromInventory(void* self, void* filterContext) {
    // Phase 0: discard owned rows through DeleteAll, retaining pointer capacity.
    if (static_cast<CaddieProductListInterface*>(self)->clearOwnedRows() == 0)
        return 0;
    CaddieProductListInterface* products = static_cast<CaddieProductListInterface*>(self);
    CSharedDoc* shared = g_sharedDoc;
    IffTableEntry<IFF_STRUCT::sCaddie>* entry = ((CItemManager*)shared->itemInventory)->caddies.data;
    while (((CItemManager*)shared->itemInventory)->caddies.data +
                        ((CItemManager*)shared->itemInventory)->caddies.size != entry) {
        void* sourceRecord = &entry->record;
        // Phase 1: filter borrowed inventory rows; only accepted rows are copied.
        if (static_cast<CaddieProductListInterface*>(self)->accepts(sourceRecord, filterContext) != 0) {
            CaddieProductRecordView* ownedRow = static_cast<CaddieProductRecordView*>(__nw__FUl(sizeof(CaddieProductRecordView)));
            if (ownedRow == 0)
                return 0;
            *ownedRow = *reinterpret_cast<const CaddieProductRecordView*>(sourceRecord);
            // Phase 2a: append directly if the current pointer array has a free slot.
            unsigned int rowCount = products->size;
            if (rowCount < products->capacity) {
                unsigned int nextRowCount = rowCount + 1;
                void** pointers = products->data;
                *(volatile unsigned int*)&products->size = nextRowCount;
                (pointers + nextRowCount)[-1] = ownedRow;
            } else {
                // Phase 2b: validate pointer-count limits before allocating a replacement.
                if (kProductMaxEntries - products->capacity < 1) {
                    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(exception_runtime::comThinkVectorErrorStrings + kLengthErrorTagOffset, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                unsigned int additionalSlots = 1 + products->size - products->capacity;
                // Guard the new pointer array until copied entries are committed.
                F18VecBuffer replacement(&products->capacity);
                unsigned int newCapacity = products->capacity;
                if (additionalSlots > kProductMaxEntries - newCapacity) {
                    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(exception_runtime::comThinkVectorErrorStrings + kLengthErrorTagOffset, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                // Same 3/5, 1/2, saturation growth policy as the caddie-item path.
                if (newCapacity < kProductFastGrowthLimit) {
                    newCapacity += F18Max((3 * (newCapacity + 1)) / 5, additionalSlots);
                } else if (newCapacity < kProductHalfGrowthLimit) {
                    newCapacity += F18Max((newCapacity + 1) / 2, additionalSlots);
                } else {
                    newCapacity = kProductMaxEntries;
                }
                if (newCapacity > kProductMaxEntries) {
                    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(exception_runtime::comThinkVectorErrorStrings + kLengthErrorTagOffset, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                void** newPointers = static_cast<void**>(appMallocDummy__FUl(newCapacity * kProductPointerBytes));
                unsigned int oldRowCount = products->size;
                replacement.offset = oldRowCount;
                replacement.data = newPointers;
                (newPointers + oldRowCount)[replacement.size++] = ownedRow;
                void** oldPointers = products->data;
                replacement.capacity = newCapacity;
                int pointersToMove = (reinterpret_cast<char*>(oldPointers + products->size) - reinterpret_cast<char*>(oldPointers)) / kProductPointerBytes;
                int insertionStart = oldRowCount - pointersToMove;
                replacement.offset = insertionStart;
                unsigned int pointerBytes = pointersToMove * kProductPointerBytes;
                // Phase 3: transfer existing pointers; owned rows keep their addresses.
                memcpy(newPointers + insertionStart, oldPointers, pointerBytes);
                memset(oldPointers, 0, pointerBytes);
                unsigned int zero = 0;
                unsigned int newSize = replacement.size + pointersToMove;
                replacement.size = newSize;
                oldPointers = products->data;
                void** committedPointers = static_cast<void**>(replacement.data);
                unsigned int oldPointerCapacity = products->capacity;
                products->size = zero;
                replacement.data = oldPointers;
                products->capacity = replacement.capacity;
                replacement.capacity = oldPointerCapacity;
                // Phase 4: publish the replacement; the guard frees the cleared old array.
                products->data = committedPointers;
                products->size = newSize;
                replacement.size = zero;
            }
        }
        ++entry;
    }
    return 1;
}
#pragma pop

int CShopProductCaddie_CopyCommonRecord(void* arg0, void* arg1, unsigned int index);

// Delete owned club-set rows and clear size while retaining pointer-array capacity.
int CShopProductClubSet_DeleteAll(void* arg0) {
    void* object = arg0;
    void** item = ((VecObj*)object)->begin;
    unsigned int count = 0;
    // Original product DeleteAll: reload the count before each end check.
    while ((count = ((VecObj*)object)->size),
           !SameProductIterator(((VecObj*)object)->begin + count, item)) {
            if (*item != 0) {
                __dl__FPv(*item);
                *item = 0;
            }
            ++item;
    }
    ((VecObj*)object)->size -= count;
    return 1;
}

#pragma opt_propagation off
#pragma opt_common_subs off
// Delete product records through slot 1, release vector storage, and optionally delete the product list.
void* __dt__10CShopBaseCFv(void* object, int deleting) {
    if (object != 0) {
        void** vtable = reinterpret_cast<void**>(s_shopBaseCVtable);
        *reinterpret_cast<void**>(object) = vtable;
        // Phase 1: virtual DeleteAll releases row allocations, preserving vector capacity.
        ClearCaddieProductRows(object);
        // Phase 2: release the +8 embedded vector's pointer array. Repeated
        // address guards are the original member-destructor expansion, not five vectors.
        if (static_cast<char*>(object) + 8 != 0 && static_cast<char*>(object) + 8 != 0 && static_cast<char*>(object) + 8 != 0 && static_cast<char*>(object) + 8 != 0 && static_cast<char*>(object) + 8 != 0 &&
            ((VecObj*)object)->begin != 0) {
            unsigned int* cp = &((VecObj*)object)->size;
            unsigned int count = *cp;
            *cp -= count;
            appFreeUni__FPv(((VecObj*)object)->begin);
        }
        // Phase 3: release the interface base, then optionally the wrapper itself.
        CShopProductInterface_dtor(object, 0);
        if (deleting > 0)
            __dl__FPv(object);
    }
    return object;
}

// Delete product records through slot 1, release vector storage, and optionally delete the product list.
void* __dt__10CShopBaseDFv(void* object, int deleting) {
    if (object != 0) {
        void** vtable = reinterpret_cast<void**>(s_shopBaseDVtable);
        *reinterpret_cast<void**>(object) = vtable;
        // Same row -> pointer array -> base/wrapper ownership order as BaseC.
        static_cast<CaddieProductListInterface*>(object)->clearOwnedRows();
        if (static_cast<char*>(object) + 8 != 0 && static_cast<char*>(object) + 8 != 0 && static_cast<char*>(object) + 8 != 0 && static_cast<char*>(object) + 8 != 0 && static_cast<char*>(object) + 8 != 0 &&
            ((VecObj*)object)->begin != 0) {
            unsigned int* cp = &((VecObj*)object)->size;
            unsigned int count = *cp;
            *cp -= count;
            appFreeUni__FPv(((VecObj*)object)->begin);
        }
        CShopProductInterface_dtor(object, 0);
        if (deleting > 0)
            __dl__FPv(object);
    }
    return object;
}

#pragma opt_common_subs reset
#pragma opt_propagation reset
// Copy one caddie-item IFF record to the caller after checking destination, index and stored pointer.
int CShopProductCaddieItem_CopyRecord(void* self, void* destination, unsigned int productIndex) {
    if (destination == 0)
        return 0;
    unsigned int count = ((VecObj*)self)->size;
    if (count <= productIndex)
        return 0;
    void* source = reinterpret_cast<void**>(((VecObj*)self)->begin)[productIndex];
    if (source == 0)
        return 0;
    Memory_Copy(destination, source, sizeof(IFF_STRUCT::sCadItem));
    return 1;
}

// Copy one club-set IFF record to the caller after checking destination, index and stored pointer.
int CShopProductClubSet_CopyRecord(void* self, void* destination, unsigned int productIndex) {
    if (destination == 0)
        return 0;
    unsigned int count = ((VecObj*)self)->size;
    if (count <= productIndex)
        return 0;
    void* source = reinterpret_cast<void**>(((VecObj*)self)->begin)[productIndex];
    if (source == 0)
        return 0;
    Memory_Copy(destination, source, sizeof(IFF_STRUCT::sClubSet));
    return 1;
}

#pragma use_lmw_stmw on
// Delete all owned record copies and clear the size while retaining the vector allocation.
int CShopProductCaddieItem_DeleteAll(void* self) {
    void* object = self;
    void** item = ((VecObj*)object)->begin;
    unsigned int count = 0;
    // Original product DeleteAll: reload the count before each end check.
    while ((count = ((VecObj*)object)->size),
           !SameProductIterator(((VecObj*)object)->begin + count, item)) {
            if (*item != 0) {
                __dl__FPv(*item);
                *item = 0;
            }
            ++item;
    }
    ((VecObj*)object)->size -= count;
    return 1;
}
#pragma use_lmw_stmw off

// This product-interface hook is unsupported and returns zero.
int CShopProduct_DefaultUnsupportedHookA() {
    return 0;
}

// Return the number of caddie-item products currently stored.
int CShopProductCaddieItem_GetCount(void* self) {
    return (int)((VecObj*)self)->size;
}

// This product-interface hook is unsupported and returns zero.
int CShopProduct_DefaultUnsupportedHookB() {
    return 0;
}

// Return the product count through the existing pointer-sized ABI.
void* CShopProductClubSet_GetCount(void* self) {
    return (void*)((VecObj*)self)->size;
}

// Initialize an empty product pointer vector and store its caller-supplied context.
void __ct__10CShopBaseCFPv(void* self, void* context) {
    (*(void**)&((VecObj*)self)->vtable) = &s_shopBaseCVtable;
    (*(unsigned int*)&((VecObj*)self)->begin) = 0;
    ((VecObj*)self)->size = 0;
    ((VecObj*)self)->capacity = 0;
    ((VecObj*)self)->context = context;
}

// Initialize an empty product pointer vector and store its caller-supplied context.
void __ct__10CShopBaseDFPv(void* self, void* context) {
    (*(void**)&((VecObj*)self)->vtable) = &s_shopBaseDVtable;
    (*(unsigned int*)&((VecObj*)self)->begin) = 0;
    ((VecObj*)self)->size = 0;
    ((VecObj*)self)->capacity = 0;
    ((VecObj*)self)->context = context;
}

struct ShopDataInterface {
    virtual void slot0();
    virtual void slot1();
    virtual int get(void*);
};

#pragma opt_propagation off
// Fetch a selected product and copy its common IFF header; an inline pair-copy helper changes instruction scheduling.
// Requires sizeof(IffItemCommon) writable bytes. get() writes only into local
// scratch; destination remains untouched on its failure. Returns 1 after copying
// the complete common header, including bytes not semantically decoded here.
int CShopProductCaddie_CopyCommonRecord(void* self, void* destination, unsigned int index) {
    // Phase 1: validate output and fetch the full row into borrowed local scratch.
    struct IndexedShopDataInterface {
        virtual void slot0() DECOMP_LOCAL_VIRTUAL;
        virtual void slot1() DECOMP_LOCAL_VIRTUAL;
        virtual int get(void*, unsigned int) DECOMP_LOCAL_VIRTUAL;
    };
    if (destination == 0)
        return 0;
    union { unsigned char bytes[0x2B8]; IffItemCommon common; } recordBuffer;
    unsigned char* data = recordBuffer.bytes;
    // Fetch the full row into scratch; copy only the common header after success.
    if (static_cast<IndexedShopDataInterface*>(self)->get(data, index) == 0)
        return 0;
    // Phase 2: copy the common header as a record, preserving its complete name/icon
    // arrays (including unused bytes), identifiers, price, flags and trailing metadata.
    // Byte pairs intentionally load both source bytes before storing either one;
    // memcpy/assignment or an inline pair-copy helper changes the current object.
    // Name[40]:
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 0]; second = data[kCommonNameOffset + 1]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 0] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 1] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 2]; second = data[kCommonNameOffset + 3]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 2] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 3] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 4]; second = data[kCommonNameOffset + 5]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 4] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 5] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 6]; second = data[kCommonNameOffset + 7]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 6] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 7] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 8]; second = data[kCommonNameOffset + 9]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 8] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 9] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 10]; second = data[kCommonNameOffset + 11]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 10] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 11] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 12]; second = data[kCommonNameOffset + 13]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 12] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 13] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 14]; second = data[kCommonNameOffset + 15]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 14] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 15] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 16]; second = data[kCommonNameOffset + 17]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 16] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 17] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 18]; second = data[kCommonNameOffset + 19]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 18] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 19] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 20]; second = data[kCommonNameOffset + 21]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 20] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 21] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 22]; second = data[kCommonNameOffset + 23]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 22] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 23] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 24]; second = data[kCommonNameOffset + 25]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 24] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 25] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 26]; second = data[kCommonNameOffset + 27]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 26] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 27] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 28]; second = data[kCommonNameOffset + 29]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 28] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 29] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 30]; second = data[kCommonNameOffset + 31]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 30] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 31] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 32]; second = data[kCommonNameOffset + 33]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 32] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 33] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 34]; second = data[kCommonNameOffset + 35]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 34] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 35] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 36]; second = data[kCommonNameOffset + 37]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 36] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 37] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 38]; second = data[kCommonNameOffset + 39]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 38] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 39] = second; }
    // Copy the complete 40-byte icon field; pairs retain the original byte schedule.
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 0]; second = data[kCommonIconOffset + 1]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 0] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 1] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 2]; second = data[kCommonIconOffset + 3]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 2] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 3] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 4]; second = data[kCommonIconOffset + 5]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 4] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 5] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 6]; second = data[kCommonIconOffset + 7]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 6] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 7] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 8]; second = data[kCommonIconOffset + 9]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 8] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 9] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 10]; second = data[kCommonIconOffset + 11]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 10] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 11] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 12]; second = data[kCommonIconOffset + 13]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 12] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 13] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 14]; second = data[kCommonIconOffset + 15]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 14] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 15] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 16]; second = data[kCommonIconOffset + 17]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 16] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 17] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 18]; second = data[kCommonIconOffset + 19]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 18] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 19] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 20]; second = data[kCommonIconOffset + 21]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 20] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 21] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 22]; second = data[kCommonIconOffset + 23]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 22] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 23] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 24]; second = data[kCommonIconOffset + 25]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 24] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 25] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 26]; second = data[kCommonIconOffset + 27]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 26] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 27] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 28]; second = data[kCommonIconOffset + 29]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 28] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 29] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 30]; second = data[kCommonIconOffset + 31]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 30] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 31] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 32]; second = data[kCommonIconOffset + 33]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 32] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 33] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 34]; second = data[kCommonIconOffset + 35]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 34] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 35] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 36]; second = data[kCommonIconOffset + 37]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 36] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 37] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 38]; second = data[kCommonIconOffset + 39]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 38] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 39] = second; }
    (*( unsigned int*)&((IffItemCommon*)destination)->typeId) = (*( unsigned int*)&recordBuffer.common.typeId);
    (*( unsigned int*)&((IffItemCommon*)destination)->price) = (*( unsigned int*)&recordBuffer.common.price);
    (*( unsigned char*)&((IffItemCommon*)destination)->levelFlags) = recordBuffer.common.levelFlags;
    (*( unsigned char*)&((IffItemCommon*)destination)->shopFlags) = recordBuffer.common.shopFlags;
    // Preserve the trailing common-item metadata word; its individual bits are not decoded here.
    (*( unsigned short*)&((IffItemCommon*)destination)->commonFlagWord) = (*( unsigned short*)&recordBuffer.common.commonFlagWord);
    return 1;
}
#pragma opt_propagation reset

#pragma opt_propagation off
#pragma opt_common_subs off
// Fetch a selected caddie item and copy its common IFF header; explicit pairs preserve this hook's scheduling.
// Same scratch-first, destination-unchanged-on-failure contract as the caddie hook.
int CShopProductCaddieItem_CopyCommonRecord(void* self, void* destination, unsigned int index) {
    struct IndexedShopDataInterface {
        virtual void slot0() DECOMP_LOCAL_VIRTUAL;
        virtual void slot1() DECOMP_LOCAL_VIRTUAL;
        virtual int get(void*, unsigned int) DECOMP_LOCAL_VIRTUAL;
    };
    if (destination == 0)
        return 0;
    union { unsigned char bytes[0x2B8]; IffItemCommon common; } recordBuffer;
    unsigned char* data = recordBuffer.bytes;
    // Fetch the full row into scratch; copy only the common header after success.
    if (static_cast<IndexedShopDataInterface*>(self)->get(data, index) == 0)
        return 0;
    // Copy the complete 40-byte name field, including unused trailing bytes.
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 0]; second = data[kCommonNameOffset + 1]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 0] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 1] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 2]; second = data[kCommonNameOffset + 3]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 2] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 3] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 4]; second = data[kCommonNameOffset + 5]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 4] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 5] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 6]; second = data[kCommonNameOffset + 7]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 6] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 7] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 8]; second = data[kCommonNameOffset + 9]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 8] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 9] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 10]; second = data[kCommonNameOffset + 11]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 10] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 11] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 12]; second = data[kCommonNameOffset + 13]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 12] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 13] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 14]; second = data[kCommonNameOffset + 15]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 14] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 15] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 16]; second = data[kCommonNameOffset + 17]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 16] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 17] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 18]; second = data[kCommonNameOffset + 19]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 18] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 19] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 20]; second = data[kCommonNameOffset + 21]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 20] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 21] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 22]; second = data[kCommonNameOffset + 23]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 22] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 23] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 24]; second = data[kCommonNameOffset + 25]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 24] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 25] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 26]; second = data[kCommonNameOffset + 27]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 26] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 27] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 28]; second = data[kCommonNameOffset + 29]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 28] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 29] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 30]; second = data[kCommonNameOffset + 31]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 30] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 31] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 32]; second = data[kCommonNameOffset + 33]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 32] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 33] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 34]; second = data[kCommonNameOffset + 35]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 34] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 35] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 36]; second = data[kCommonNameOffset + 37]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 36] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 37] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonNameOffset + 38]; second = data[kCommonNameOffset + 39]; static_cast<unsigned char*>(destination)[kCommonNameOffset + 38] = first; static_cast<unsigned char*>(destination)[kCommonNameOffset + 39] = second; }
    // Copy the complete 40-byte icon field; pairs retain the original byte schedule.
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 0]; second = data[kCommonIconOffset + 1]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 0] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 1] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 2]; second = data[kCommonIconOffset + 3]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 2] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 3] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 4]; second = data[kCommonIconOffset + 5]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 4] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 5] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 6]; second = data[kCommonIconOffset + 7]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 6] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 7] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 8]; second = data[kCommonIconOffset + 9]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 8] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 9] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 10]; second = data[kCommonIconOffset + 11]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 10] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 11] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 12]; second = data[kCommonIconOffset + 13]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 12] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 13] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 14]; second = data[kCommonIconOffset + 15]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 14] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 15] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 16]; second = data[kCommonIconOffset + 17]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 16] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 17] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 18]; second = data[kCommonIconOffset + 19]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 18] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 19] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 20]; second = data[kCommonIconOffset + 21]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 20] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 21] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 22]; second = data[kCommonIconOffset + 23]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 22] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 23] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 24]; second = data[kCommonIconOffset + 25]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 24] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 25] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 26]; second = data[kCommonIconOffset + 27]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 26] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 27] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 28]; second = data[kCommonIconOffset + 29]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 28] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 29] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 30]; second = data[kCommonIconOffset + 31]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 30] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 31] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 32]; second = data[kCommonIconOffset + 33]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 32] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 33] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 34]; second = data[kCommonIconOffset + 35]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 34] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 35] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 36]; second = data[kCommonIconOffset + 37]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 36] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 37] = second; }
    { unsigned char second; unsigned char first; first = data[kCommonIconOffset + 38]; second = data[kCommonIconOffset + 39]; static_cast<unsigned char*>(destination)[kCommonIconOffset + 38] = first; static_cast<unsigned char*>(destination)[kCommonIconOffset + 39] = second; }
    (*( unsigned int*)&((IffItemCommon*)destination)->typeId) = (*( unsigned int*)&recordBuffer.common.typeId);
    (*( unsigned int*)&((IffItemCommon*)destination)->price) = (*( unsigned int*)&recordBuffer.common.price);
    (*( unsigned char*)&((IffItemCommon*)destination)->levelFlags) = recordBuffer.common.levelFlags;
    (*( unsigned char*)&((IffItemCommon*)destination)->shopFlags) = recordBuffer.common.shopFlags;
    // Preserve the trailing common-item metadata word; its individual bits are not decoded here.
    (*( unsigned short*)&((IffItemCommon*)destination)->commonFlagWord) = (*( unsigned short*)&recordBuffer.common.commonFlagWord);
    return 1;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
} // extern "C"
