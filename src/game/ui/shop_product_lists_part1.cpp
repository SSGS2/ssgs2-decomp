#include "classes/CSharedDoc.h"
#include "classes/CItemManager.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#define AsCItemManager(p) ((CItemManager*)(p))
// The shared header reserves this embedded CItemManager as bytes; give it its established table type here.
struct SharedShopInventoryView {
    char sharedState[IFF_OFFSET(CSharedDoc, itemInventory)];
    CItemManager itemInventory;
};
// Constants retain the original integer types and encoded values.
// Pointer arrays own copied IFF records. DeleteAll deletes records and resets count, retaining
// the pointer allocation; deleting destructors additionally free that allocation and the object.
// Rebuild clears first and appends accepted copies in table order. Failed record allocation
// returns 0 with the accepted prefix retained; success returns 1, including an empty list.
// Pointer-array growth assumes allocation succeeds; length overflow throws length_error.
// Dispatch contracts: slot 3 clears owned records (0 means failure); slot 8
// accepts/rejects a borrowed IFF record with the caller's filter context.
// Filter context is borrowed and passed unchanged to virtual slot 8. Growth copies pointers
// and clears old slots; it never deep-copies records a second time.
static const unsigned kMaxPointerElements = 0x3FFFFFFFU;
static const unsigned kPointerGrowthThreeFifthsLimit = 0x15555555U;
static const unsigned kPointerGrowthHalfLimit = 0x2AAAAAAAU;
static const unsigned kLengthErrorTypeNameOffset = 0x14;
// Call ABI (borrowed pointers, nonzero = success/acceptance):
// slot 3: int clearRecords(void* receiver)
// slot 4: int copySelectedRecord(void* receiver, void* output, unsigned int index)
// slot 8: int acceptRecord(void* receiver, void* record, void* context)
// Casts retain their spelling: new typedefs/enumerators change CodeWarrior's
// generated cleanup-symbol numbering and hence exception relocation identities.

#include "iff/IffStruct.h"
#include "classes/PtrArray8023.h"
// Named container fields preserve original +0x00/+0x04/+0x08/+0x0C/+0x10 accesses.
#include "classes/CShopProductSetItem.h"
#include "classes/CShopProductList.h"

#define AsCShopProductList(p) ((CShopProductList*)(p))

#define AsCShopProductSetItem(p) ((CShopProductSetItem*)(p))
// Compare iterator addresses without dereferencing their borrowed record pointers.
template <class T> inline bool same_pos(const T& a, const T& b) { return a == b; }
// Translation unit 0x8023163C-0x80232BFC (15 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern void Memory_Copy(void*, void*, unsigned int);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void CShopProductInterface_dtor(void*, int);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nw__FUl(unsigned long);
extern int strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void appFreeUni__FPv(void*);
extern int __nwa__FUl(unsigned int);
extern CSharedDoc* g_sharedDoc;
extern char comThinkVectorErrorStrings[];
extern void* __vt__Q23std11logic_error[];
extern void* s_sharedCharDeleterVtable[];
extern void* __vt__Q23std12length_error[];
extern void __throw(void*, void*, void*);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*);
extern char comThinkVectorErrorStrings[];
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nw__FUl(unsigned long);
extern int strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void appFreeUni__FPv(void*);

struct SharedErrorTextOwner {
    void* vtable;
    int strongReferences, weakReferences;
    void* text;
};

// Build a reference-counted error string, transfer its buffer to the owner, then throw.
// Keep this exception builder in the caller: factoring it into a function changes the
// CodeWarrior cleanup helpers and unwind relocations, even when declared inline.
#define THROW_VECTOR_LENGTH_ERROR() do { \
    struct ErrObj { void* vtable; char* msg; void* owner; void* pad; }; \
    ErrObj errorObject; \
    errorObject.vtable = __vt__Q23std11logic_error; \
    char cleanupChar = 0; \
    char* errorText = comThinkVectorErrorStrings; \
    int errorLength = strlen(errorText) + 1; \
    char* errorCopy = (char*)__nwa__FUl((errorLength << 1) - errorLength); \
    errorObject.msg = errorCopy; \
    /* Free the error string unless construction transferred its allocation. */ \
    struct TextCleanup { char* data; ~TextCleanup() { if (data) __dla__FPv(data); } }; \
    TextCleanup textCleanup = { errorCopy }; \
    char* volatile cleanupGuard[2]; \
    cleanupGuard[0] = errorCopy; \
    cleanupGuard[1] = (char*)&cleanupChar; \
    (void)cleanupGuard; \
    int cleanupFlag = 0; \
    void* errorOwner = __nw__FUl(0x10); \
    if (errorOwner != 0) { \
        ((SharedErrorTextOwner*)errorOwner)->strongReferences = 1; \
        ((SharedErrorTextOwner*)errorOwner)->weakReferences = 1; \
        ((SharedErrorTextOwner*)errorOwner)->vtable = s_sharedCharDeleterVtable; \
        ((SharedErrorTextOwner*)errorOwner)->text = errorCopy; \
    } \
    errorObject.owner = errorOwner; \
    textCleanup.data = 0; \
    cleanupGuard[0] = 0; \
    if (cleanupChar != 0) __dla__FPv(cleanupGuard[0]); \
    strcpy(errorObject.msg, errorText); \
    errorObject.vtable = __vt__Q23std12length_error; \
    __throw(errorText + kLengthErrorTypeNameOffset, &errorObject, (void*)__dt__Q23std12length_errorFv); \
} while (0)

// Copy one pair from the four contiguous 40-byte character resource names.
static inline void CopyCharacterResourceNamePair(IFF_STRUCT::sChar* recordCopy, unsigned char* sourceBytes, unsigned byteOffset) {
    unsigned char first = sourceBytes[IFF_OFFSET(IFF_STRUCT::sChar, model) + byteOffset];
    unsigned char second = sourceBytes[IFF_OFFSET(IFF_STRUCT::sChar, model) + 1 + byteOffset];
    ((unsigned char*)recordCopy->model)[byteOffset] = first;
    ((unsigned char*)recordCopy->model)[byteOffset + 1] = second;
}

// Rebuild the avatar list from Character.iff, allocating a complete copy of each accepted character.
int CShopProductAvatar_RebuildFromCharacterTable(void* self, void* filterContext) {
    // Clear owned records via virtual slot 3 before scanning the borrowed inventory table.
    if (!((int(*)(void*))(*(void***)self)[3])(self)) return 0;
    char* records = (char*)(&((SharedShopInventoryView*)g_sharedDoc)->itemInventory)->characters.data;
    char* end;
    for (; end = (char*)(&((SharedShopInventoryView*)g_sharedDoc)->itemInventory)->characters.data +
                        (&((SharedShopInventoryView*)g_sharedDoc)->itemInventory)->characters.size * sizeof(CharacterTableEntry), !(end == records);
         records += sizeof(CharacterTableEntry)) {
        IFF_STRUCT::sChar* sourceRecord = &((CharacterTableEntry*)records)->record;
        if (!((int(*)(void*, void*, void*))(*(void***)self)[8])(self, sourceRecord, filterContext)) continue;
        IFF_STRUCT::sChar* recordCopy = (IFF_STRUCT::sChar*)__nw__FUl(sizeof(IFF_STRUCT::sChar));
        if (!recordCopy) return 0;
        // Copy phase: common prefix, four resource names, then typed fields.
        // Unknown IFF bytes are retained; accepting a record never normalizes it.
        struct WordBlock { unsigned w[0x17]; };
        *(WordBlock*)&recordCopy->common = *(WordBlock*)sourceRecord;
        unsigned char* sourceBytes = (unsigned char*)sourceRecord;
// Copy the four 40-byte resource names in byte pairs to retain instruction order.
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 0);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 2);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 4);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 6);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 8);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 10);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 12);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 14);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 16);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 18);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 20);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 22);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 24);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 26);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 28);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 30);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 32);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 34);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 36);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 38);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 40);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 42);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 44);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 46);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 48);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 50);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 52);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 54);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 56);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 58);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 60);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 62);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 64);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 66);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 68);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 70);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 72);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 74);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 76);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 78);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 80);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 82);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 84);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 86);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 88);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 90);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 92);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 94);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 96);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 98);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 100);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 102);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 104);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 106);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 108);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 110);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 112);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 114);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 116);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 118);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 120);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 122);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 124);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 126);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 128);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 130);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 132);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 134);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 136);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 138);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 140);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 142);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 144);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 146);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 148);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 150);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 152);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 154);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 156);
        CopyCharacterResourceNamePair(recordCopy, sourceBytes, 158);
        recordCopy->stats[0] = sourceRecord->stats[0];
        recordCopy->stats[1] = sourceRecord->stats[1];
        recordCopy->stats[2] = sourceRecord->stats[2];
        recordCopy->stats[3] = sourceRecord->stats[3];
        recordCopy->stats[4] = sourceRecord->stats[4];
        recordCopy->scale = sourceRecord->scale;
        recordCopy->defaultPartCount = sourceRecord->defaultPartCount;
        recordCopy->optionalPartCount = sourceRecord->optionalPartCount;
        recordCopy->baseStats[0] = sourceRecord->baseStats[0];
        recordCopy->baseStats[1] = sourceRecord->baseStats[1];
        recordCopy->baseStats[2] = sourceRecord->baseStats[2];
        recordCopy->baseStats[3] = sourceRecord->baseStats[3];
        recordCopy->baseStats[4] = sourceRecord->baseStats[4];
        recordCopy->unk_11B = sourceRecord->unk_11B;
        // Append phase: publish the owned copy; growth below moves pointer
        // words only and retains already accepted records on later failure.
        unsigned recordCount = (*(unsigned*)&AsPtrArray8023(self)->count);
        unsigned pointerCapacity = AsPtrArray8023(self)->capacity;
        if (recordCount < pointerCapacity) {
            unsigned n = recordCount + 1;
            (*(unsigned*)&AsPtrArray8023(self)->count) = n;
            *(void**)((*(char**)&AsPtrArray8023(self)->data) + n * 4 - 4) = recordCopy;
            continue;
        }
        // Pointer storage is full: compute bounded growth before allocating a replacement array.
        if (kMaxPointerElements - pointerCapacity < 1) {
            THROW_VECTOR_LENGTH_ERROR();
        }
        unsigned additionalCapacity = recordCount + 1 - pointerCapacity;
        unsigned* volatile tv[5];
        tv[0] = 0;
        tv[1] = 0;
        tv[2] = 0;
        tv[3] = &AsPtrArray8023(self)->capacity;
        tv[4] = 0;
        (void)tv;
        if (additionalCapacity > kMaxPointerElements - pointerCapacity) {
            THROW_VECTOR_LENGTH_ERROR();
        }
        unsigned newCapacity;
        if (pointerCapacity < kPointerGrowthThreeFifthsLimit) {
            unsigned growth = (3U * (pointerCapacity + 1)) / 5U;
            unsigned* chosen = (growth < additionalCapacity) ? &additionalCapacity : &growth;
            newCapacity = pointerCapacity + *chosen;
        } else if (pointerCapacity < kPointerGrowthHalfLimit) {
            unsigned growth = (pointerCapacity + 1) / 2;
            unsigned* chosen = (growth < additionalCapacity) ? &additionalCapacity : &growth;
            newCapacity = pointerCapacity + *chosen;
        } else {
            newCapacity = kMaxPointerElements;
        }
        if (newCapacity > kMaxPointerElements) {
            THROW_VECTOR_LENGTH_ERROR();
        }
        // Allocate, append the new record, move existing pointers, and publish the new storage.
        void** newPointers = (void**)appMallocDummy__FUl(newCapacity * 4);
        newPointers[recordCount] = recordCopy;
        void** recordPointers = (*(void***)&AsPtrArray8023(self)->data);
        memcpy(newPointers, recordPointers, recordCount * 4);
        memset(recordPointers, 0, recordCount * 4);
        void** oldPointers = recordPointers;
        (*(void***)&AsPtrArray8023(self)->data) = newPointers;
        (*(unsigned*)&AsPtrArray8023(self)->count) = recordCount + 1;
        AsPtrArray8023(self)->capacity = newCapacity;
        if (oldPointers != 0) appFreeUni__FPv(oldPointers);
    }
    return 1;
}

// This shop callback always reports false; the virtual operation is not established here.
int CShopProductAvatar_ReturnFalse() { return 0; }

// Return the number of products in the shop list.
int CShopProductList_GetCount(void* self) { return AsCShopProductList(self)->productCount; }

// Copy the selected set-item common header, including its type ID, Pang price, and shop restrictions.
// out must hold the common IFF header. Virtual slot 4 copies the selected record into
// 0x2B8 bytes of scratch; this wrapper returns 0 for null output or failed record copy.
// Only the common header is written, not the bundled-item payload; output remains caller-owned.
int CShopProductSetItem_CopySelectedCommonHeader(void* self, void* out, unsigned int index) {
    if (!out) return 0;
    // The suffix vfunc_4 names this wrapper; the call below is slot 4 of
    // the runtime receiver, whose concrete dispatch table is not proved here.
    // Do not install this wrapper in that same slot: it would recurse.
    // Phase 1: fetch the runtime receiver's selected full record.
    unsigned char temp[0x2B8];
    if (((int(*)(void*, void*, unsigned int))(*(void***)self)[4])(self, temp, index) == 0) return 0;
    // Phase 2: copy opaque header bytes 0..87 verbatim. Expanded stores
    // preserve instruction order; the field meanings are not all established.
    unsigned char _a0 = temp[0], _b0 = temp[1];
    ((unsigned char*)out)[0] = _a0;
    ((unsigned char*)out)[1] = _b0;
    ((unsigned char*)out)[2] = temp[2];
    ((unsigned char*)out)[3] = temp[3];
    ((unsigned char*)out)[4] = temp[4];
    ((unsigned char*)out)[5] = temp[5];
    ((unsigned char*)out)[6] = temp[6];
    ((unsigned char*)out)[7] = temp[7];
    ((unsigned char*)out)[8] = temp[8];
    ((unsigned char*)out)[9] = temp[9];
    ((unsigned char*)out)[10] = temp[10];
    ((unsigned char*)out)[11] = temp[11];
    ((unsigned char*)out)[12] = temp[12];
    ((unsigned char*)out)[13] = temp[13];
    ((unsigned char*)out)[14] = temp[14];
    ((unsigned char*)out)[15] = temp[15];
    ((unsigned char*)out)[16] = temp[16];
    ((unsigned char*)out)[17] = temp[17];
    ((unsigned char*)out)[18] = temp[18];
    ((unsigned char*)out)[19] = temp[19];
    ((unsigned char*)out)[20] = temp[20];
    ((unsigned char*)out)[21] = temp[21];
    ((unsigned char*)out)[22] = temp[22];
    ((unsigned char*)out)[23] = temp[23];
    ((unsigned char*)out)[24] = temp[24];
    ((unsigned char*)out)[25] = temp[25];
    ((unsigned char*)out)[26] = temp[26];
    ((unsigned char*)out)[27] = temp[27];
    ((unsigned char*)out)[28] = temp[28];
    ((unsigned char*)out)[29] = temp[29];
    ((unsigned char*)out)[30] = temp[30];
    ((unsigned char*)out)[31] = temp[31];
    ((unsigned char*)out)[32] = temp[32];
    ((unsigned char*)out)[33] = temp[33];
    ((unsigned char*)out)[34] = temp[34];
    ((unsigned char*)out)[35] = temp[35];
    ((unsigned char*)out)[36] = temp[36];
    ((unsigned char*)out)[37] = temp[37];
    ((unsigned char*)out)[38] = temp[38];
    ((unsigned char*)out)[39] = temp[39];
    ((unsigned char*)out)[40] = temp[40];
    ((unsigned char*)out)[41] = temp[41];
    ((unsigned char*)out)[42] = temp[42];
    ((unsigned char*)out)[43] = temp[43];
    ((unsigned char*)out)[44] = temp[44];
    ((unsigned char*)out)[45] = temp[45];
    ((unsigned char*)out)[46] = temp[46];
    ((unsigned char*)out)[47] = temp[47];
    ((unsigned char*)out)[48] = temp[48];
    ((unsigned char*)out)[49] = temp[49];
    ((unsigned char*)out)[50] = temp[50];
    ((unsigned char*)out)[51] = temp[51];
    ((unsigned char*)out)[52] = temp[52];
    ((unsigned char*)out)[53] = temp[53];
    ((unsigned char*)out)[54] = temp[54];
    ((unsigned char*)out)[55] = temp[55];
    ((unsigned char*)out)[56] = temp[56];
    ((unsigned char*)out)[57] = temp[57];
    ((unsigned char*)out)[58] = temp[58];
    ((unsigned char*)out)[59] = temp[59];
    ((unsigned char*)out)[60] = temp[60];
    ((unsigned char*)out)[61] = temp[61];
    ((unsigned char*)out)[62] = temp[62];
    ((unsigned char*)out)[63] = temp[63];
    ((unsigned char*)out)[64] = temp[64];
    ((unsigned char*)out)[65] = temp[65];
    ((unsigned char*)out)[66] = temp[66];
    ((unsigned char*)out)[67] = temp[67];
    ((unsigned char*)out)[68] = temp[68];
    ((unsigned char*)out)[69] = temp[69];
    ((unsigned char*)out)[70] = temp[70];
    ((unsigned char*)out)[71] = temp[71];
    ((unsigned char*)out)[72] = temp[72];
    ((unsigned char*)out)[73] = temp[73];
    ((unsigned char*)out)[74] = temp[74];
    ((unsigned char*)out)[75] = temp[75];
    ((unsigned char*)out)[76] = temp[76];
    ((unsigned char*)out)[77] = temp[77];
    ((unsigned char*)out)[78] = temp[78];
    ((unsigned char*)out)[79] = temp[79];
    ((unsigned char*)out)[80] = temp[80];
    ((unsigned char*)out)[81] = temp[81];
    ((unsigned char*)out)[82] = temp[82];
    ((unsigned char*)out)[83] = temp[83];
    ((unsigned char*)out)[84] = temp[84];
    ((unsigned char*)out)[85] = temp[85];
    ((unsigned char*)out)[86] = temp[86];
    ((unsigned char*)out)[87] = temp[87];
    // Phase 3: finish the typed type-ID/price/restriction fields and report success.
    ((IFF_STRUCT::sSetItem*)out)->common.typeId = ((IFF_STRUCT::sSetItem*)temp)->common.typeId;
    (*(unsigned*)&((IFF_STRUCT::sSetItem*)out)->common.price) = (*(unsigned*)&((IFF_STRUCT::sSetItem*)temp)->common.price);
    ((IFF_STRUCT::sSetItem*)out)->common.levelFlags = temp[0x58];
    ((IFF_STRUCT::sSetItem*)out)->common.shopFlags = temp[0x59];
    ((IFF_STRUCT::sSetItem*)out)->common.commonFlagWord = ((IFF_STRUCT::sSetItem*)temp)->common.commonFlagWord;
    return 1;
}

// This shop callback always reports false; the virtual operation is not established here.
int CShopProductSetItem_ReturnFalse() { return 0; }

extern char s_shopSetItemPointerArrayVtable[];
extern char s_shopPartsPointerArrayVtable[];

// Return the number of pointers currently in this shop record container.
int PtrArray_GetCount(void* self) { return AsPtrArray8023(self)->count; }

// Initialize the set-item record pointer array and retain its caller-supplied filter parameter.
void ShopSetItemPointerArray_Initialize(void* self, int param) {
    (*(int*)&AsPtrArray8023(self)->data) = 0;
    AsPtrArray8023(self)->vtable = (void*)s_shopSetItemPointerArrayVtable;
    AsPtrArray8023(self)->count = 0;
    (*(int*)&AsPtrArray8023(self)->capacity) = 0;
    AsPtrArray8023(self)->parameter = param;
}

// Initialize the parts record pointer array and retain its caller-supplied filter parameter.
void ShopPartsPointerArray_Initialize(void* self, int param) {
    (*(int*)&AsPtrArray8023(self)->data) = 0;
    AsPtrArray8023(self)->vtable = (void*)s_shopPartsPointerArrayVtable;
    AsPtrArray8023(self)->count = 0;
    (*(int*)&AsPtrArray8023(self)->capacity) = 0;
    AsPtrArray8023(self)->parameter = param;
}

// Copy a complete SetItem.iff record, including up to eight bundled item IDs and the item count.
// dst is caller-owned storage for the complete record. Return 0 for null dst, an
// out-of-range index or a null slot; otherwise copy the record and return 1.
int CShopProductSetItem_CopyRecord(void* self, void* dst, unsigned idx) {
    if (dst == 0) return 0;
    if (AsCShopProductSetItem(self)->itemRecordCount <= idx) return 0;
    int* arr = (*(int**)&AsCShopProductSetItem(self)->itemRecords);
    int val = arr[idx];
    if (val == 0) return 0;
    Memory_Copy(dst, (void*)val, sizeof(IFF_STRUCT::sSetItem));
    return 1;
}

// Copy a complete Part.iff record, including textures, stats, and the body-slot hide mask.
// dst is caller-owned storage for the complete record. Return 0 for null dst, an
// out-of-range index or a null slot; otherwise copy the record and return 1.
int CShopProductParts_CopyRecord(void* self, void* dst, unsigned idx) {
    if (dst == 0) return 0;
    if ((*(unsigned*)&AsPtrArray8023(self)->count) <= idx) return 0;
    int* arr = (*(int**)&AsPtrArray8023(self)->data);
    int val = arr[idx];
    if (val == 0) return 0;
    Memory_Copy(dst, (void*)val, sizeof(IFF_STRUCT::sPart));
    return 1;
}

// Delete all owned product objects, reloading the list bounds after each deletion.
int CShopProductList_DeleteAll(void* self) {
    int count = 0;
    int* p = (*(int**)&AsCShopProductList(self)->products);
    int zero = 0;
    // 0x8023163C unit: reload the array bounds after each deletion.
    while (!same_pos((*(int**)&AsCShopProductList(self)->products) +
        (count = AsCShopProductList(self)->productCount), p)) {
        if (*p != 0) {
            __dl__FPv((void*)*p);
            *p = zero;
        }
        p++;
    }
    AsCShopProductList(self)->productCount = count - (*(unsigned int*)&AsCShopProductList(self)->productCount);
    return 1;
}

// Delete all owned part records and reset the count without shrinking pointer storage.
int CShopProductParts_DeleteAll(void* self) {
    int count = 0;
    int* p = (*(int**)&AsPtrArray8023(self)->data);
    int zero = 0;
    // 0x8023163C unit: reload the array bounds after each deletion.
    while (!same_pos((*(int**)&AsPtrArray8023(self)->data) +
        (count = AsPtrArray8023(self)->count), p)) {
        if (*p != 0) {
            __dl__FPv((void*)*p);
            *p = zero;
        }
        ++p;
    }
    AsPtrArray8023(self)->count = count - count;
    return 1;
}

// Clear set-item records, free pointer storage, and optionally delete the container.
void* ShopSetItemPointerArray_dtor(void* self, int flags) {
    if (self != 0) {
        *(void**)self = s_shopSetItemPointerArrayVtable;
        ((void(*)(void*))((void**)s_shopSetItemPointerArrayVtable)[3])(self);
        // Repeated +8 checks are expanded member-destructor guards. The
        // volatile subtraction preserves the observed count loads; allocation
        // release follows the virtual payload clear above.
        if ((char*)self + 8 != 0 && (char*)self + 8 != 0 && (char*)self + 8 != 0 &&
            (char*)self + 8 != 0 && (char*)self + 8 != 0 && AsPtrArray8023(self)->data != 0) {
            (*(volatile int*)&AsPtrArray8023(self)->count) = (*(volatile int*)&AsPtrArray8023(self)->count) - (*(volatile int*)&AsPtrArray8023(self)->count);
            appFreeUni__FPv(AsPtrArray8023(self)->data);
        }
        CShopProductInterface_dtor(self, 0);
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

// Clear part records, free pointer storage, and optionally delete the container.
void* ShopPartsPointerArray_dtor(void* self, int flags) {
    if (self != 0) {
        *(void**)self = s_shopPartsPointerArrayVtable;
        ((void(*)(void*))((void**)s_shopPartsPointerArrayVtable)[3])(self);
        // Repeated +8 checks are expanded member-destructor guards. The
        // volatile subtraction preserves the observed count loads; allocation
        // release follows the virtual payload clear above.
        if ((char*)self + 8 != 0 && (char*)self + 8 != 0 && (char*)self + 8 != 0 &&
            (char*)self + 8 != 0 && (char*)self + 8 != 0 && AsPtrArray8023(self)->data != 0) {
            (*(volatile int*)&AsPtrArray8023(self)->count) = (*(volatile int*)&AsPtrArray8023(self)->count) - (*(volatile int*)&AsPtrArray8023(self)->count);
            appFreeUni__FPv(AsPtrArray8023(self)->data);
        }
        CShopProductInterface_dtor(self, 0);
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

#pragma use_lmw_stmw on
// Rebuild the filtered set-item list from SetItem.iff and own a copy of each accepted bundle.
int CShopProductSetItem_RebuildFilteredRecords(void* self, void* filterContext) {
    // Clear owned records via virtual slot 3 before scanning the borrowed inventory table.
    if (!((int(*)(void*))(*(void***)self)[3])(self)) return 0;
    CSharedDoc* sharedDoc = g_sharedDoc;
    char* records = (char*)(&((SharedShopInventoryView*)sharedDoc)->itemInventory)->setItems.data;
    for (; records != (char*)(&((SharedShopInventoryView*)sharedDoc)->itemInventory)->setItems.data +
                        (&((SharedShopInventoryView*)sharedDoc)->itemInventory)->setItems.size * sizeof(SetItemTableEntry);
         records += sizeof(SetItemTableEntry)) {
        char* sourceRecord = records + 4;
        if (!((int(*)(void*, void*, void*))(*(void***)self)[8])(self, sourceRecord, filterContext)) continue;
        void* recordCopy = __nw__FUl(sizeof(IFF_STRUCT::sSetItem));
        if (!recordCopy) return 0;
        // Preserve the compiler's original double/word/byte record-copy representation.
        struct Rec {
            double d[11];
            unsigned u;
            unsigned e[8];
            unsigned char a;
            unsigned char b;
            unsigned short c;
        };
        *(Rec*)recordCopy = *(Rec*)sourceRecord;
        // Append phase: publish the owned copy; growth below moves pointer
        // words only and retains already accepted records on later failure.
        unsigned recordCount = AsCShopProductSetItem(self)->itemRecordCount;
        unsigned pointerCapacity = AsCShopProductSetItem(self)->itemRecordCapacity;
        if (pointerCapacity > recordCount) {
            unsigned newSize = recordCount + 1;
            AsCShopProductSetItem(self)->itemRecordCount = newSize;
            ((void**)AsCShopProductSetItem(self)->itemRecords)[newSize - 1] = recordCopy;
            continue;
        }
        // Pointer storage is full: compute bounded growth before allocating a replacement array.
        if (kMaxPointerElements - pointerCapacity < 1) {
            THROW_VECTOR_LENGTH_ERROR();
        }
        unsigned additionalCapacity = AsCShopProductSetItem(self)->itemRecordCount + 1 - AsCShopProductSetItem(self)->itemRecordCapacity;
        struct ReallocGuard {
            void** start;
            unsigned added;
            unsigned pointerCapacity;
            unsigned* pcap;
            unsigned oldsize;
        };
        volatile ReallocGuard guard;
        guard.start = 0;
        guard.added = 0;
        guard.pointerCapacity = 0;
        guard.oldsize = 0;
        unsigned cap2 = AsCShopProductSetItem(self)->itemRecordCapacity;
        guard.pcap = (&AsCShopProductSetItem(self)->itemRecordCapacity);
        if (additionalCapacity > kMaxPointerElements - cap2) {
            THROW_VECTOR_LENGTH_ERROR();
        }
        unsigned newCapacity;
        if (cap2 < kPointerGrowthThreeFifthsLimit) {
            unsigned growth = (3U * (cap2 + 1)) / 5U;
            unsigned* chosen = (growth < additionalCapacity) ? &additionalCapacity : &growth;
            newCapacity = cap2 + *chosen;
        } else if (cap2 < kPointerGrowthHalfLimit) {
            unsigned growth = (cap2 + 1) / 2;
            unsigned* chosen = (growth < additionalCapacity) ? &additionalCapacity : &growth;
            newCapacity = cap2 + *chosen;
        } else {
            newCapacity = kMaxPointerElements;
        }
        if (newCapacity > kMaxPointerElements) {
            THROW_VECTOR_LENGTH_ERROR();
        }
        // Allocate, append the new record, move existing pointers, and publish the new storage.
        void** newPointers = (void**)appMallocDummy__FUl(newCapacity * 4);
        unsigned pos = guard.added;
        unsigned count = AsCShopProductSetItem(self)->itemRecordCount;
        guard.oldsize = count;
        guard.pointerCapacity = newCapacity;
        *(void**)((char*)(newPointers + count) + pos * 4) = recordCopy;
        guard.start = newPointers;
        guard.added = pos + 1;
        void** recordPointers = (*(void***)&AsCShopProductSetItem(self)->itemRecords);
        int total = (int)((char*)(recordPointers + AsCShopProductSetItem(self)->itemRecordCount) - (char*)recordPointers) / 4;
        unsigned after = count - total;
        guard.oldsize = after;
        memcpy((char*)newPointers + after * 4, recordPointers, total * 4);
        memset(recordPointers, 0, total * 4);
        unsigned added = guard.added;
        void** oldBegin = (*(void***)&AsCShopProductSetItem(self)->itemRecords);
        unsigned newSize = added + total;
        void** newBegin = guard.start;
        guard.added = newSize;
        unsigned oldCap = AsCShopProductSetItem(self)->itemRecordCapacity;
        unsigned newCap2 = guard.pointerCapacity;
        AsCShopProductSetItem(self)->itemRecordCount = 0;
        guard.start = oldBegin;
        AsCShopProductSetItem(self)->itemRecordCapacity = newCap2;
        guard.pointerCapacity = oldCap;
        (*(void***)&AsCShopProductSetItem(self)->itemRecords) = newBegin;
        AsCShopProductSetItem(self)->itemRecordCount = newSize;
        guard.added = 0;
        if (oldBegin != 0) {
            guard.added = 0;
            appFreeUni__FPv(oldBegin);
        }
    }
    return 1;
}

#pragma use_lmw_stmw reset

#undef THROW_VECTOR_LENGTH_ERROR

} // extern "C"

