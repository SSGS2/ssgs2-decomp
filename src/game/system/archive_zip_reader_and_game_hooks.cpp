// ZIP archive reader and resource table support: opens archives and reads their entries,
// loads resource index tables, validates and compares flash save blocks, and builds the
// shop product records. It also holds a few small game hooks for input blocking and queries.

#include <decomp/static_init.h>
#include "classes/CBuildProduct.h"
#include "classes/ArchiveEntryInfo.h"
#include "classes/ArchiveHandle.h"
#include "classes/ArchiveContext.h"
#include "classes/ArchiveCursor.h"
#include "classes/ArchiveStream.h"
#include "classes/ResourceTableHeader.h"
#include "classes/ResourceTableEntry.h"
#undef AsArchiveContext
#undef AsArchiveCursor
#undef AsArchiveEntryInfo
// View borrowed context storage; preserve its base address and ownership.
static inline ArchiveContext* AsArchiveContext(void* p) {
    return (ArchiveContext*)p;
}
// View the ZIP cursor without changing the active-entry lifetime.
static inline ArchiveCursor* AsArchiveCursor(void* p) {
    return (ArchiveCursor*)p;
}
// View caller-owned metadata output storage.
static inline ArchiveEntryInfo* AsArchiveEntryInfo(void* p) {
    return (ArchiveEntryInfo*)p;
}

#include "game/shop_product.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CSaveFormFlash.h"
#include "classes/CSaveForm.h"

// Archive results follow the ZIP wrapper's status ABI, rather than NAND error codes.
enum ArchiveResult {
    kArchiveSuccess = 0,
    kArchiveMoreData = 0x600,
    kArchiveBadLocalHeader = 0x700,
    kArchiveTruncatedRead = 0x800,
    kArchiveCorruptEntry = 0x1000,
    kArchiveInvalidArgument = 0x10000,
    kArchiveWrongHandleKind = 0x80000,
    kArchiveInflateFailure = 0x05000000
};
// All three accepted API mode codes take the same memory-read path here. The
// public meanings of the codes are unresolved; do not infer different sinks.
enum ArchiveReadMode {
    kArchiveReadMode1 = 1,
    kArchiveReadMode2 = 2,
    kArchiveReadMode3 = 3
};
// Existing pool at 0x8048B5C4: four separator/parent/separator spellings and UT.
enum ArchivePathStringOffset {
    kParentBackslashBackslash = 0x1f6,
    kParentBackslashSlash = 0x1fb,
    kParentSlashSlash = 0x200,
    kParentSlashBackslash = 0x205,
    kZipExtendedTimestampTag = 0x20a,
    kParentPatternBytes = 4,
    kZipExtraFieldHeaderBytes = 4
};
enum {
    kArchiveReaderHandleTag = 1,
    kZipCentralHeaderBytes = 0x2e,
    kFlashPlayerCount = 4,
    kFlashSaveBytes = 4 * sizeof(SavePlayerRecord),
    kFileReadOnly = 0x1,
    kFileHidden = 0x2,
    kFileSystem = 0x4,
    kFileDirectory = 0x10,
    kFileArchive = 0x20,
    kFileTimeUnixEpochLow = 0xD53E8000u,
    kFileTimeUnixEpochHigh = 0x019DB1DE
};

// Only the word excluded from comparison and the outfit range are interpreted here.
struct FlashPlayerRecordView {
    char beforeComparisonWord[0x20];
    int comparisonIgnoredWord;
    char beforeOutfitParts[0x2CC - 0x24];
    char outfitParts[sizeof(SavePlayerRecord) - 0x2CC];
};
struct FlashSaveBlockView {
    FlashPlayerRecordView players[kFlashPlayerCount];
};
// The ZIP library returns decoded native words, rather than an on-disk packed header.
struct ZipCentralEntryFields {
    unsigned int versionMadeBy;
    char beforeSizes[0x18 - 4];
    unsigned int compressedSize, uncompressedSize;
    char beforeAttributes[0x34 - 0x20];
    unsigned int externalAttributes;
};
typedef char ZipCentralEntryFields_attributes[((int)&((ZipCentralEntryFields*)0)->externalAttributes == 0x34) ? 1 : -1];
typedef char FlashPlayerRecordView_size[(sizeof(FlashPlayerRecordView) == sizeof(SavePlayerRecord)) ? 1 : -1];

extern "C" {

extern float kFloat_6_2831855;
extern float s_archiveEntryFullTurn;
extern float s_resourceTableFullTurn;
extern float s_saveFormFlashFullTurn;
extern float s_buildProductFullTurn;

void __dl__FPv(void*);
int ArchiveContext_ReadEntry(void*, int, void*, unsigned int, unsigned int);
int ArchiveContext_GetEntryInfo(void*, int, void*);
void appFreeUni__FPv(void*);
void* SaveForm_ValidateRecord(void*, int, void*);
void* SaveForm_GetRecord(void*, int, void*);
int SaveData_ValidateCharacterOutfitParts(void*);
void* SharedFile_Create(const char*, int);
void* CSharedFilePtr_GetLoadedBuffer(void*);
int strcmp(void*, void*);
void* strstr(void*, void*);
void strncpy(void*, void*, int);
void* __nw__FUl(unsigned int);
void ResourceTable_RelocateSharedBuffer(void*);
void ArchiveContext_CloseCurrentEntry(void*);
int ZipArchive_ReadCentralDirectoryEntry(void*, void*, void*, void*, int, int, int, int, int);
void ArchiveContext_OpenCurrentEntryWithPassword(void*, void*);
int ArchiveEntry_ReadAndInflate(void*, void*, unsigned int, unsigned char*);
int ArchiveContext_CheckLocalFileHeader(void*, void*, void*, void*);
void File_CloseHandle(void*);
void free(void*);
void* __nwa__FUl(unsigned int);
void SaveData_Ctor(void*);
void __destroy_new_array(void*, void*);
extern "C" void __dt__6CBpTexFv(void*);
extern int s_flashSaveRecordAlignment;
extern const char s_zipResourceIndexFilename[];
extern char inflateDiagnosticStrings[];
extern char __vt__14CSaveFormFlash[];
extern char __vt__13CBuildProduct[];
extern unsigned int g_archiveLastResult;
void SaveData_InitSlotRecordDefaults(void*);
int SaveForm_GetRecordSize(void*);
int SaveForm_CompareRecordBytes(void*, void*, void*);
void Memory_Copy(void*, void*, int);
void Memory_Clear(void*, int);
int ArchiveContext_InitializeReader(void*, void*, void*, void*);
void __dla__FPv(void*);
void* memcpy(void*, const void*, unsigned int);
char* strcpy(char*, const char*);
// Validate the archive handle, then read or resume an entry and publish the wrapper status.
// destination borrows requestedBytes writable bytes. See ArchiveContext_ReadEntry
// for mode/resume/status semantics; this wrapper also stores the status globally.
int Archive_ReadEntry(void* archive, int entryIndex, void* destination, unsigned int requestedBytes, int mode) {
    if (archive == 0) {
        g_archiveLastResult = kArchiveInvalidArgument;
        return kArchiveInvalidArgument;
    } else if (AsArchiveHandle(archive)->tag != kArchiveReaderHandleTag) {
        g_archiveLastResult = kArchiveWrongHandleKind;
        return kArchiveWrongHandleKind;
    } else {
        return (g_archiveLastResult = (unsigned int)ArchiveContext_ReadEntry(
                    AsArchiveHandle(archive)->context, entryIndex, destination, requestedBytes, mode));
    }
}

// Clear the output prefix and fetch metadata (index -1 requests the entry count).
int Archive_GetEntryInfo(void* arg, int idx, void* out) {
    AsArchiveEntryInfo(out)->index = 0;
    AsArchiveEntryInfo(out)->path[0] = 0;
    AsArchiveEntryInfo(out)->uncompressedSize = 0;
    if (arg == 0) {
        g_archiveLastResult = kArchiveInvalidArgument;
        return kArchiveInvalidArgument;
    } else if (AsArchiveHandle(arg)->tag != kArchiveReaderHandleTag) {
        g_archiveLastResult = kArchiveWrongHandleKind;
        return kArchiveWrongHandleKind;
    } else {
        return (g_archiveLastResult =
                    (unsigned int)ArchiveContext_GetEntryInfo(AsArchiveHandle(arg)->context, idx, out));
    }
}

} // extern "C"

// Release vector backing storage; pointed-to products must already be destroyed.
template <> std::vector<CShopProductInterface*>::~vector() {
    void* ptr = this;
    if (ptr != 0) {
        if (ptr != 0) {
            if (ptr != 0) {
                if (ptr != 0) {
                    void* first = *(void**)ptr;
                    if (first != 0) {
                        int _t = (*(volatile int*)&AsShopProductVector(ptr)->size);
                        (*(int*)&AsShopProductVector(ptr)->size) = _t - _t;
                        appFreeUni__FPv(first);
                    }
                }
            }
        }
    }
}

extern "C" {

// Reserved main-loop entry notification; no side effects.
void GameHook_OnMainLoopEnter() {}
// Reserved frame-start notification; no side effects.
void GameHook_OnFrameBegin() {}
// Reserved initialization notification; no side effects.
void GameHook_OnInitializationComplete() {}
// Reserved render-completion notification; no side effects.
void GameHook_OnFrameRenderComplete() {}
// Reserved shutdown notification; no side effects.
void GameHook_OnShutdown() {}
// Leave input unblocked at this hook.
int GameHook_ShouldBlockInput() {
    return 0;
}
// Return false for the unresolved reserved query.
int GameHook_ReservedFalse() {
    return 0;
}

// Initialize this source group\'s angle conversion constant.
void ArchiveEntry_InitFullTurnConstant() {
    s_archiveEntryFullTurn = kFloat_6_2831855;
}

// Initialize the resource-table source group\'s angle conversion constant.
void ResourceTable_InitFullTurnConstant() {
    s_resourceTableFullTurn = kFloat_6_2831855;
}

// Initialize the save-form source group\'s angle conversion constant.
void SaveFormFlash_InitFullTurnConstant() {
    s_saveFormFlashFullTurn = kFloat_6_2831855;
}

// Initialize the build-product source group\'s angle conversion constant.
void BuildProduct_InitFullTurnConstant() {
    s_buildProductFullTurn = kFloat_6_2831855;
}

// Destroy the flash save form and optionally free its allocation.
CSaveFormFlash::~CSaveFormFlash() {
    ((CSaveForm*)this)->Destroy(0);
}

#pragma use_lmw_stmw on
// Load/relocate the resource index; write the named pointer array through out and return its count.
int ResourceTable_FindEntryByName(void* arg, void* name, void* out) {
    if (name == 0)
        return 0;
    if (*(void**)arg == 0)
        *(void**)arg = SharedFile_Create(s_zipResourceIndexFilename, 0);
    void* info;
    while ((info = CSharedFilePtr_GetLoadedBuffer(*(void**)arg)) == 0) {
    }
    ResourceTable_RelocateSharedBuffer(arg);
    info = CSharedFilePtr_GetLoadedBuffer(*(void**)arg);
    int count = AsResourceTableHeader(info)->entryCount;
    void* entry = (void*)(AsResourceTableHeader(info) + 1);
    for (int i = 0; i < count; ++i, entry = (void*)(AsResourceTableEntry(entry) + 1)) {
        if (!strcmp(AsResourceTableEntry(entry)->name, name)) {
            *(void**)out = AsResourceTableEntry(entry)->items;
            return AsResourceTableEntry(entry)->count;
        }
    }
    return 0;
}
#pragma use_lmw_stmw reset

// Validate the save block, its alignment, and all four players\' outfit parts.
int CSaveFormFlash::ValidateOutfitRecords(int size, void* out) {
    if (!SaveForm_ValidateRecord(this, size, out))
        return 0;
    int alignment = s_flashSaveRecordAlignment;
    if (size % alignment != 0)
        return 0;
    void* obj = SaveForm_GetRecord(this, size, out);
    for (int i = 0; i < kFlashPlayerCount; ++i, obj = (FlashPlayerRecordView*)obj + 1)
        if (!SaveData_ValidateCharacterOutfitParts(((FlashPlayerRecordView*)obj)->outfitParts))
            return 0;
    return 1;
}

struct Loader {
    void* vtable;
    void* unk[3];
};
class Y5 {
public:
    virtual void v08(int);
    virtual void v0C();
    virtual int v10();
};
struct Fn802D56FCNew : Loader {
    // Construct the four-player save-form base and install the flash implementation.
    Fn802D56FCNew(int recordCount, int hasMd5) {
        ((CSaveForm*)this)->__ct(kFlashSaveBytes, recordCount, hasMd5);
        vtable = (void*)__vt__14CSaveFormFlash;
    }
};
// Construct and initialize a four-player flash save form; destroy it if initialization fails.
CSaveFormFlash* CSaveFormFlash::Create(int recordCount, int hasMd5) {
    Loader* loader = new Fn802D56FCNew(recordCount, hasMd5);
    if (!loader)
        return 0;
    if (!((Y5*)loader)->v10()) {
        if (loader)
            ((Y5*)loader)->v08(1);
        return 0;
    }
    return (CSaveFormFlash*)loader;
}

// Initialize a ZIP reader and wrap it in a tagged archive handle.
void* Archive_Open(void* a, void* b, void* c) {
    void* context = __nw__FUl(0x244);
    if (context != 0) {
        (*(unsigned int*)&AsArchiveContext(context)->reader) = 0;
        AsArchiveContext(context)->openIndex = -1;
        AsArchiveContext(context)->cachedIndex = -1;
        (*(unsigned int*)&AsArchiveContext(context)->password) = 0;
        (*(unsigned int*)&AsArchiveContext(context)->ownedAuxiliaryBuffer) =
            0; // owned auxiliary array, released alongside the password buffer
    }
    int result = ArchiveContext_InitializeReader(context, a, b, c);
    g_archiveLastResult = (unsigned int)result;
    if (result != 0) {
        if (context != 0) {
            void* p = AsArchiveContext(context)->password;
            if (p != 0)
                __dla__FPv(p);
            (*(unsigned int*)&AsArchiveContext(context)->password) = 0;
            p = AsArchiveContext(context)->ownedAuxiliaryBuffer;
            if (p != 0)
                __dla__FPv(p);
            (*(unsigned int*)&AsArchiveContext(context)->ownedAuxiliaryBuffer) = 0;
            __dl__FPv(context);
        }
        return 0;
    }
    void* handle = __nw__FUl(8);
    AsArchiveHandle(handle)->tag = kArchiveReaderHandleTag;
    AsArchiveHandle(handle)->context = context;
    return handle;
}

#pragma use_lmw_stmw on
// Convert buffer-relative names, item arrays and item pointers to addresses exactly once.
void ResourceTable_RelocateSharedBuffer(void* self) {
    void* p = *(void**)self;
    int initialized = 0;
    if (p != 0) {
        void* info = CSharedFilePtr_GetLoadedBuffer(p);
        if (info != 0) {
            info = CSharedFilePtr_GetLoadedBuffer(*(void**)self);
            initialized = (AsResourceTableHeader(info)->relocated != 0);
        }
    }
    if (initialized)
        return;
    int off;
    void* entry;
    int i;
    void* base = CSharedFilePtr_GetLoadedBuffer(*(void**)self);
    // The resource index (zipinfo.bin) is big-endian on disc.
    void* info = CSharedFilePtr_GetLoadedBuffer(*(void**)self);
    entry = (void*)(AsResourceTableHeader(info) + 1);
    i = 0;
    off = 0;
    while (i < AsResourceTableHeader(info)->entryCount) {
        int old0 = (*(int*)&AsResourceTableEntry(entry)->name);
        (*(int*)&AsResourceTableEntry(entry)->name) = (int)((char*)base + old0);
        int old8 = (*(int*)&AsResourceTableEntry(entry)->items);
        (*(int*)&AsResourceTableEntry(entry)->items) = (int)((char*)base + old8);
        int j = 0;
        off = 0;
        while (j < AsResourceTableEntry(entry)->count) {
            void* arr = AsResourceTableEntry(entry)->items;
            ++j;
            int slotval = *(int*)((char*)arr + off);
            *(int*)((char*)arr + off) = (int)((char*)base + slotval);
            off += 4;
        }
        ++i;
        entry = (void*)(AsResourceTableEntry(entry) + 1);
    }
    AsResourceTableHeader(info)->relocated = 1;
}
#pragma use_lmw_stmw reset

// Compare four-player save blocks after clearing each player\'s excluded comparison word.
int CSaveFormFlash::CompareRecords(void* src, void* dst) {
    if (src == 0 || dst == 0)
        return 0;
    int sz = SaveForm_GetRecordSize(this);
    if ((unsigned int)sz != (unsigned int)kFlashSaveBytes)
        return 0;
    char buf1[kFlashSaveBytes];
    char buf2[kFlashSaveBytes];
    SaveData_InitSlotRecordDefaults(buf1);
    Memory_Copy(buf1, src, sz);
    int zero = 0;
    ((FlashSaveBlockView*)buf1)->players[0].comparisonIgnoredWord = zero;
    ((FlashSaveBlockView*)buf1)->players[1].comparisonIgnoredWord = zero;
    ((FlashSaveBlockView*)buf1)->players[2].comparisonIgnoredWord = zero;
    ((FlashSaveBlockView*)buf1)->players[3].comparisonIgnoredWord = zero;
    SaveData_InitSlotRecordDefaults(buf2);
    Memory_Copy(buf2, dst, sz);
    ((FlashSaveBlockView*)buf2)->players[0].comparisonIgnoredWord = zero;
    ((FlashSaveBlockView*)buf2)->players[1].comparisonIgnoredWord = zero;
    ((FlashSaveBlockView*)buf2)->players[2].comparisonIgnoredWord = zero;
    ((FlashSaveBlockView*)buf2)->players[3].comparisonIgnoredWord = zero;
    return (int)SaveForm_CompareRecordBytes(this, buf1, buf2);
}

// Close the active entry and stream, then release reader buffers and the tagged handle.
int Archive_Close(void* handle) {
    if (handle == 0) {
        g_archiveLastResult = kArchiveInvalidArgument;
        return kArchiveInvalidArgument;
    }
    if (AsArchiveHandle(handle)->tag != kArchiveReaderHandleTag) {
        g_archiveLastResult = kArchiveWrongHandleKind;
        return kArchiveWrongHandleKind;
    }
    void* context = AsArchiveHandle(handle)->context;
    int entryState = AsArchiveContext(context)->openIndex;
    if (entryState != -1) {
        ArchiveContext_CloseCurrentEntry(AsArchiveContext(context)->reader);
    }
    AsArchiveContext(context)->openIndex = -1;
    void* reader = AsArchiveContext(context)->reader;
    if (reader != 0) {
        if (reader != 0) {
            entryState = AsArchiveCursor(reader)->entryReadState;
            if (entryState != 0) {
                ArchiveContext_CloseCurrentEntry(reader);
            }
            void* stream = AsArchiveCursor(reader)->stream;
            if (stream != 0) {
                if (AsArchiveStream(stream)->closeFileOnDestroy != 0) {
                    File_CloseHandle(AsArchiveStream(stream)->fileHandle);
                }
                __dl__FPv(stream);
            }
            if (reader != 0) {
                free(reader);
            }
        }
    }
    entryState = 0;
    AsArchiveContext(context)->reader = 0;
    g_archiveLastResult = 0;
    if (context != 0) {
        void* p = AsArchiveContext(context)->password;
        if (p != 0)
            __dla__FPv(p);
        AsArchiveContext(context)->password = 0;
        p = AsArchiveContext(context)->ownedAuxiliaryBuffer;
        if (p != 0)
            __dla__FPv(p);
        AsArchiveContext(context)->ownedAuxiliaryBuffer = 0;
        __dl__FPv(context);
    }
    __dl__FPv(handle);
    return (int)g_archiveLastResult;
}

} // extern "C"

// Allocate per-slot product indices and initialize the embedded save-data state.
CBuildProduct::CBuildProduct(int count, unsigned char flag) {
    field10 = 0;
    indices = (int*)__nwa__FUl((unsigned int)count * 4);
    textureBindingsBySlot = 0;
    this->count = count;
    this->flag = flag;
    SaveData_Ctor(object24);
    ready = 0;
    for (int i = 0; i < count; ++i) {
        indices[i] = 0;
    }
}

extern "C" {

#pragma use_lmw_stmw on
#pragma opt_propagation on
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Destroy shop products, per-slot arrays and vector storage in the original ownership order.
void* __dt__11ProductBaseFv(void* self, int shouldDelete) {
    void** finish;
    int off2;
    int cnt;
    int off;
    void** begin;
    unsigned int count;
    if (self != 0) {
        *(void**)((char*)self + 0) = (void*)__vt__13CBuildProduct;
        // Release slot indices, then delete each owned product before vector storage.
        if ((*(void**)&AsCBuildProduct(self)->indices) != 0) {
            __dla__FPv((*(void**)&AsCBuildProduct(self)->indices));
            (*(void**)&AsCBuildProduct(self)->indices) = 0;
        }
        off = (*(int*)&AsCBuildProduct(self)->products.data);
        int zero = 0;
        while ((count = AsCBuildProduct(self)->products.size, begin = (*(void***)&AsCBuildProduct(self)->products.data),
                finish = begin + count, !(finish == (void**)off))) {
            if (*(void**)off != 0) {
                void* obj = *(void**)off;
                if (obj != 0) {
                    ((Y5*)obj)->v08(1);
                }
                *(void**)off = (void*)zero;
            }
            off += 4;
        }
        {
            char* end = (char*)begin + count;
            AsCBuildProduct(self)->products.size -= end - (char*)begin;
        }
        // textureBindingsBySlot is a count-slot table of owned CBpTex arrays: CBpTex::~CBpTex
        // releases each element's shared/direct texture references (bp_sprite_pool_and_texture_binding.cpp).
        // Delete elements before deleting the outer pointer table.
        if (AsCBuildProduct(self)->textureBindingsBySlot != 0) {
            int zero = 0;
            cnt = AsCBuildProduct(self)->count - 1;
            off = cnt << 2;
            for (; cnt >= 0; off -= 4, --cnt) {
                void* obj = *(void**)((char*)AsCBuildProduct(self)->textureBindingsBySlot + off);
                if (obj != 0) {
                    __destroy_new_array(obj, (void*)__dt__6CBpTexFv);
                    *(void**)((char*)AsCBuildProduct(self)->textureBindingsBySlot + off) = (void*)zero;
                }
            }
            if (AsCBuildProduct(self)->textureBindingsBySlot != 0) {
                __dla__FPv(AsCBuildProduct(self)->textureBindingsBySlot);
                AsCBuildProduct(self)->textureBindingsBySlot = 0;
            }
        }
        // field10 is a separate count-slot table of owned raw arrays. Their element
        // type is unresolved: cleanup supplies no element destructor. Neither table
        // length is products.size; both use the constructor's count.
        if (AsCBuildProduct(self)->field10 != 0) {
            int zero2 = 0;
            cnt = AsCBuildProduct(self)->count - 1;
            off2 = cnt << 2;
            for (; cnt >= 0; off2 -= 4, --cnt) {
                void* obj = *(void**)((char*)AsCBuildProduct(self)->field10 + off2);
                if (obj != 0) {
                    __dla__FPv(obj);
                    *(void**)((char*)AsCBuildProduct(self)->field10 + off2) = (void*)zero2;
                }
            }
            if (AsCBuildProduct(self)->field10 != 0) {
                __dla__FPv(AsCBuildProduct(self)->field10);
                AsCBuildProduct(self)->field10 = 0;
            }
        }
        if ((char*)self + 4 != 0) {
            if ((char*)self + 4 != 0) {
                if ((char*)self + 4 != 0) {
                    if ((char*)self + 4 != 0) {
                        if ((char*)self + 4 != 0) {
                            void* arr4 = (*(void**)&AsCBuildProduct(self)->products.data);
                            if (arr4 != 0) {
                                char* begin = (*(char**)&AsCBuildProduct(self)->products.data);
                                char* end = begin + (*(int*)&AsCBuildProduct(self)->products.size);
                                (*(int*)&AsCBuildProduct(self)->products.size) -= end - begin;
                                appFreeUni__FPv(arr4);
                            }
                        }
                    }
                }
            }
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_propagation off
// Seek to the requested ZIP entry, inflate it, and translate end/error/more-data results.
// Requires an initialized reader, a valid nonnegative entryIndex, a writable
// destination of requestedBytes bytes, and a central directory the reader can
// advance through. Codes 1/2/3 are accepted identically; all others return 0x10000.
// A repeated openIndex continues that entry; switching entries closes the old
// stream and restarts the requested one. This returns STATUS, never byte count:
// finished => 0 (takes precedence over readResult), positive read => 0x600,
// -0x6a => 0x1000, other unfinished nonpositive reads => 0x05000000.
// Nonpositive reads close/reset openIndex, even when finished is set; a positive
// final read leaves the entry open. Buffers remain caller-owned; no rollback.
int ArchiveContext_ReadEntry(void* self, int entryIndex, void* destination, unsigned int requestedBytes,
                             unsigned int mode) {
    if (mode != kArchiveReadMode3 && mode != kArchiveReadMode2 && mode != kArchiveReadMode1) {
        return kArchiveInvalidArgument;
    }
    if (entryIndex != AsArchiveContext(self)->openIndex) {
        // Reposition only on an entry change; rewind the directory for earlier indices.
        if (AsArchiveContext(self)->openIndex != -1) {
            ArchiveContext_CloseCurrentEntry(AsArchiveContext(self)->reader);
        }
        void* obj = AsArchiveContext(self)->reader;
        AsArchiveContext(self)->openIndex = -1;
        if (entryIndex >= (*(int*)&AsArchiveCursor(obj)->entryCount)) {
            return kArchiveInvalidArgument;
        }
        if (entryIndex < AsArchiveCursor(obj)->entryIndex && obj != 0) {
            (*(int*)&AsArchiveCursor(obj)->entryOffset) = AsArchiveCursor(obj)->firstEntryOffset;
            AsArchiveCursor(obj)->entryIndex = 0;
            int readResult = ZipArchive_ReadCentralDirectoryEntry(
                obj, AsArchiveCursor(obj)->entryHeader, AsArchiveCursor(obj)->localHeaderOffset, 0, 0, 0, 0, 0, 0);
            AsArchiveCursor(obj)->entryReady = (readResult == 0);
        }
        void* readerCursor;
        while (AsArchiveCursor(readerCursor = AsArchiveContext(self)->reader)->entryIndex < entryIndex) {
            if (readerCursor != 0 && AsArchiveCursor(readerCursor)->entryReady != 0 &&
                (unsigned int)(AsArchiveCursor(readerCursor)->entryIndex + 1) !=
                    AsArchiveCursor(readerCursor)->entryCount) {
                AsArchiveCursor(readerCursor)->entryOffset +=
                    AsArchiveCursor(readerCursor)->fileNameLength + AsArchiveCursor(readerCursor)->extraFieldLength +
                    AsArchiveCursor(readerCursor)->fileCommentLength + kZipCentralHeaderBytes;
                AsArchiveCursor(readerCursor)->entryIndex = AsArchiveCursor(readerCursor)->entryIndex + 1;
                int readResult = ZipArchive_ReadCentralDirectoryEntry(
                    readerCursor, AsArchiveCursor(readerCursor)->entryHeader,
                    AsArchiveCursor(readerCursor)->localHeaderOffset, 0, 0, 0, 0, 0, 0);
                AsArchiveCursor(readerCursor)->entryReady = (readResult == 0);
            }
        }
        ArchiveContext_OpenCurrentEntryWithPassword(readerCursor, AsArchiveContext(self)->password);
        AsArchiveContext(self)->openIndex = entryIndex;
    }
    // Read one chunk, release an exhausted/error stream, then translate status.
    unsigned char finished;
    int readResult =
        ArchiveEntry_ReadAndInflate(AsArchiveContext(self)->reader, destination, requestedBytes, &finished);
    if (readResult <= 0) {
        ArchiveContext_CloseCurrentEntry(AsArchiveContext(self)->reader);
        AsArchiveContext(self)->openIndex = -1;
    }
    if (finished != 0) {
        return 0;
    }
    if (readResult > 0) {
        return kArchiveMoreData;
    }
    if (readResult == -0x6a) {
        return kArchiveCorruptEntry;
    }
    return kArchiveInflateFailure;
}
#pragma use_lmw_stmw reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

#pragma use_lmw_stmw on
// Advance to the stored path suffix using the ZIP wrapper's fixed search order.
// Both arguments borrow caller storage; only pathSuffix changes, never the bytes.
static inline void SkipArchivePathPrefixes(char*& pathSuffix, char (&normalizedPath)[0x104]) {
    pathSuffix = normalizedPath;
    for (;;) {
        if (*pathSuffix != 0 && pathSuffix[1] == ':') {
            pathSuffix += 2;
            continue;
        }
        signed char pathCharacter = (signed char)*pathSuffix;
        if (pathCharacter == '\\') {
            ++pathSuffix;
            continue;
        }
        if (pathCharacter == '/') {
            ++pathSuffix;
            continue;
        }
        char* found = (char*)strstr(pathSuffix, inflateDiagnosticStrings + kParentBackslashBackslash);
        if (found) {
            pathSuffix = found + kParentPatternBytes;
            continue;
        }
        found = (char*)strstr(pathSuffix, inflateDiagnosticStrings + kParentBackslashSlash);
        if (found) {
            pathSuffix = found + kParentPatternBytes;
            continue;
        }
        found = (char*)strstr(pathSuffix, inflateDiagnosticStrings + kParentSlashSlash);
        if (found) {
            pathSuffix = found + kParentPatternBytes;
            continue;
        }
        found = (char*)strstr(pathSuffix, inflateDiagnosticStrings + kParentSlashBackslash);
        if (found) {
            pathSuffix = found + kParentPatternBytes;
            continue;
        }
        break;
    }
}

// Read/cache ZIP metadata, strip path prefixes, and mark timestamp presence with epoch placeholders.
// out is caller-owned ArchiveEntryInfo storage. Index -1 returns entry count in
// out->index; any metadata query closes the current data read, including a cache hit.
// Supports the existing trusted ZIP subset: bounded terminated names and valid
// extra fields. This is neither a general path canonicalizer nor a time decoder.
int ArchiveContext_GetEntryInfo(void* self, int entryIndex, void* out) {
    unsigned int extraFieldBytes;
    unsigned int localHeaderBytes;
    unsigned int extraFieldOffset;
    unsigned char tag[4];
    unsigned int timestampScratch[16];
    char centralHeader[0x50];
    char entryName[0x104];
    char normalizedPath[0x104];
    void* obj;
    int zero;
    char* pathSuffix;
    void* initialReaderCursor;
    // Phase 1: validate against this context's directory cursor before closing
    // an open data stream. Negative indices other than the count query are invalid.
    if (entryIndex < -1 || (initialReaderCursor = AsArchiveContext(self)->reader,
                            entryIndex >= (int)AsArchiveCursor(initialReaderCursor)->entryCount))
        return kArchiveInvalidArgument;

    if (AsArchiveContext(self)->openIndex != -1) {
        ArchiveContext_CloseCurrentEntry(initialReaderCursor);
    }
    AsArchiveContext(self)->openIndex = -1;

    if (entryIndex == AsArchiveContext(self)->cachedIndex && entryIndex != -1) {
        memcpy(out, AsArchiveContext(self)->cachedInfo, sizeof(ArchiveEntryInfo));
        return 0;
    }

    // Phase 2: answer the count-only query without seeking or allocating.
    if (entryIndex == -1) {
        void* obj = AsArchiveContext(self)->reader;
        (*(int*)&AsArchiveEntryInfo(out)->index) = (*(int*)&AsArchiveCursor(obj)->entryCount);
        AsArchiveEntryInfo(out)->path[0] = 0;
        (*(int*)&AsArchiveEntryInfo(out)->attributes) = 0;
        (*(int*)&AsArchiveEntryInfo(out)->accessTimeLow) = 0;
        (*(int*)&AsArchiveEntryInfo(out)->accessTimeHigh) = 0;
        (*(int*)&AsArchiveEntryInfo(out)->creationTimeLow) = 0;
        (*(int*)&AsArchiveEntryInfo(out)->creationTimeHigh) = 0;
        (*(int*)&AsArchiveEntryInfo(out)->modificationTimeLow) = 0;
        (*(int*)&AsArchiveEntryInfo(out)->modificationTimeHigh) = 0;
        (*(int*)&AsArchiveEntryInfo(out)->compressedSize) = 0;
        (*(int*)&AsArchiveEntryInfo(out)->uncompressedSize) = 0;
        return 0;
    }

    // Phase 3: locate the central-directory record and load its local-header extra bytes.
    obj = AsArchiveContext(self)->reader;
    if (entryIndex < AsArchiveCursor(obj)->entryIndex && obj != 0) {
        (*(int*)&AsArchiveCursor(obj)->entryOffset) = AsArchiveCursor(obj)->firstEntryOffset;
        AsArchiveCursor(obj)->entryIndex = 0;
        int directoryReadStatus = ZipArchive_ReadCentralDirectoryEntry(
            obj, AsArchiveCursor(obj)->entryHeader, AsArchiveCursor(obj)->localHeaderOffset, 0, 0, 0, 0, 0, 0);
        AsArchiveCursor(obj)->entryReady = (directoryReadStatus == 0);
    }

    void* directoryCursor;
    zero = 0;
    while (AsArchiveCursor(directoryCursor = AsArchiveContext(self)->reader)->entryIndex < entryIndex) {
        if (directoryCursor != 0 && AsArchiveCursor(directoryCursor)->entryReady != 0 &&
            (unsigned int)(AsArchiveCursor(directoryCursor)->entryIndex + 1) !=
                AsArchiveCursor(directoryCursor)->entryCount) {
            int total =
                AsArchiveCursor(directoryCursor)->fileNameLength + AsArchiveCursor(directoryCursor)->extraFieldLength;
            int commentBytes = AsArchiveCursor(directoryCursor)->fileCommentLength;
            total = total + commentBytes;
            total = total + (*(int*)&AsArchiveCursor(directoryCursor)->entryOffset);
            (*(int*)&AsArchiveCursor(directoryCursor)->entryOffset) = total + kZipCentralHeaderBytes;
            AsArchiveCursor(directoryCursor)->entryIndex = AsArchiveCursor(directoryCursor)->entryIndex + 1;
            int directoryReadStatus = ZipArchive_ReadCentralDirectoryEntry(
                directoryCursor, AsArchiveCursor(directoryCursor)->entryHeader,
                AsArchiveCursor(directoryCursor)->localHeaderOffset, 0, 0, 0, 0, 0, zero);
            AsArchiveCursor(directoryCursor)->entryReady = (directoryReadStatus == 0);
        }
    }

    ZipArchive_ReadCentralDirectoryEntry(directoryCursor, centralHeader, 0, entryName, 0x104, 0, 0, 0, 0);

    int localHeaderStatus = ArchiveContext_CheckLocalFileHeader(AsArchiveContext(self)->reader, &localHeaderBytes,
                                                                &extraFieldOffset, &extraFieldBytes);
    if (localHeaderStatus != 0)
        return kArchiveBadLocalHeader;

    void* localHeaderCursor = AsArchiveContext(self)->reader;
    AsArchiveStream(AsArchiveCursor(localHeaderCursor)->stream)->position = extraFieldOffset;
    void* extraField = __nwa__FUl(extraFieldBytes);
    void* archiveStream = *(void**)AsArchiveContext(self)->reader;
    unsigned int bytesToRead = extraFieldBytes;
    unsigned int readPosition = AsArchiveStream(archiveStream)->position;
    unsigned int bufferBytes = AsArchiveStream(archiveStream)->bufferSize;
    if (readPosition + bytesToRead > bufferBytes)
        bytesToRead = bufferBytes - readPosition;

    memcpy(extraField, (char*)AsArchiveStream(archiveStream)->buffer + readPosition, bytesToRead);
    AsArchiveStream(archiveStream)->position += bytesToRead;
    if (extraFieldBytes != bytesToRead) {
        __dla__FPv(extraField);
        return kArchiveTruncatedRead;
    }

    void* resolvedEntryCursor = AsArchiveContext(self)->reader;
    AsArchiveEntryInfo(out)->index = (*(unsigned int*)&AsArchiveCursor(resolvedEntryCursor)->entryIndex);
    strcpy(normalizedPath, entryName);

    // Phase 4: strip drive prefixes and leading separators, then discard everything up
    // to and including a separator + ".." + separator sequence. Search order is
    // back/back, back/slash, slash/slash, slash/back (not earliest-match order).
    // Components after the final such sequence remain as stored, including their
    // separator spelling; a leading "../" without a preceding separator survives.
    SkipArchivePathPrefixes(pathSuffix, normalizedPath);

    strncpy(AsArchiveEntryInfo(out)->path, pathSuffix, 0x104);

    // Phase 5: translate native creator attributes; DOS-family creators use the low bits.
    unsigned int versionMadeBy = ((ZipCentralEntryFields*)centralHeader)->versionMadeBy;
    unsigned int externalAttributes = ((ZipCentralEntryFields*)centralHeader)->externalAttributes;
    unsigned int isHidden = 0;
    unsigned int isSystem = 0;
    unsigned int isArchive = 1;
    unsigned int isReadOnly = (externalAttributes >> 23) & 1;
    unsigned int isDirectory = (externalAttributes >> 30) & 1;
    int creatorOs = (int)(versionMadeBy >> 8);
    isReadOnly = isReadOnly ^ 1;
    if (creatorOs == 0 || creatorOs == 7 || creatorOs == 0xb || creatorOs == 0xe) {
        isReadOnly = externalAttributes & 1;
        isHidden = (externalAttributes >> 1) & 1;
        isSystem = (externalAttributes >> 2) & 1;
        isDirectory = (externalAttributes >> 4) & 1;
        isArchive = (externalAttributes >> 5) & 1;
    }
    AsArchiveEntryInfo(out)->attributes = 0;
    if (isDirectory) {
        unsigned int t = AsArchiveEntryInfo(out)->attributes;
        t |= kFileDirectory;
        AsArchiveEntryInfo(out)->attributes = t;
    }
    if (isArchive) {
        unsigned int t = AsArchiveEntryInfo(out)->attributes;
        t |= kFileArchive;
        AsArchiveEntryInfo(out)->attributes = t;
    }
    if (isHidden) {
        unsigned int t = AsArchiveEntryInfo(out)->attributes;
        t |= kFileHidden;
        AsArchiveEntryInfo(out)->attributes = t;
    }
    if (isReadOnly) {
        unsigned int t = AsArchiveEntryInfo(out)->attributes;
        t |= kFileReadOnly;
        AsArchiveEntryInfo(out)->attributes = t;
    }
    if (isSystem) {
        unsigned int t = AsArchiveEntryInfo(out)->attributes;
        t |= kFileSystem;
        AsArchiveEntryInfo(out)->attributes = t;
    }

    AsArchiveEntryInfo(out)->compressedSize = ((ZipCentralEntryFields*)centralHeader)->compressedSize;
    AsArchiveEntryInfo(out)->uncompressedSize = ((ZipCentralEntryFields*)centralHeader)->uncompressedSize;

    // Phase 6: missing times stay zero. UT (ZIP ID 0x5455, bytes 'U','T') only supplies
    // Timestamp values are never read. Only the LOW length byte is used when
    // skipping fields, and payload bounds are not checked; keep this limitation
    // in mind before accepting other ZIP producers or untrusted archives.
    Memory_Clear(&timestampScratch[6], 8);
    Memory_Clear(&timestampScratch[14], 8);
    AsArchiveEntryInfo(out)->accessTimeLow = timestampScratch[14];
    AsArchiveEntryInfo(out)->accessTimeHigh = timestampScratch[15];
    AsArchiveEntryInfo(out)->creationTimeLow = timestampScratch[14];
    AsArchiveEntryInfo(out)->creationTimeHigh = timestampScratch[15];
    AsArchiveEntryInfo(out)->modificationTimeLow = timestampScratch[14];
    AsArchiveEntryInfo(out)->modificationTimeHigh = timestampScratch[15];

    unsigned int extraFieldLength = extraFieldBytes;
    unsigned int extraFieldCursor = 0;
    while (extraFieldCursor + kZipExtraFieldHeaderBytes < extraFieldLength) {
        tag[0] = ((unsigned char*)extraField)[extraFieldCursor];
        tag[1] = ((unsigned char*)extraField)[extraFieldCursor + 1];
        tag[2] = 0;
        unsigned char fieldPayloadBytes = ((unsigned char*)extraField)[extraFieldCursor + 2];
        int directoryReadStatus = strcmp(tag, inflateDiagnosticStrings + kZipExtendedTimestampTag);
        if (directoryReadStatus != 0) {
            extraFieldCursor = fieldPayloadBytes + extraFieldCursor + kZipExtraFieldHeaderBytes;
            continue;
        }
        unsigned char timestampFlags = ((unsigned char*)extraField)[extraFieldCursor + 4];
        unsigned int hasModificationTime = timestampFlags & 1;
        unsigned int hasAccessTime = (timestampFlags >> 1) & 1;
        unsigned int hasCreationTime = (timestampFlags >> 2) & 1;
        // Preserve the original epoch placeholder; this routine does not decode timestamp bytes.
        if (hasModificationTime) {
            timestampScratch[4] = kFileTimeUnixEpochLow;
            timestampScratch[5] = kFileTimeUnixEpochHigh;
            timestampScratch[12] = kFileTimeUnixEpochLow;
            timestampScratch[13] = kFileTimeUnixEpochHigh;
            AsArchiveEntryInfo(out)->modificationTimeLow = kFileTimeUnixEpochLow;
            AsArchiveEntryInfo(out)->modificationTimeHigh = kFileTimeUnixEpochHigh;
        }
        if (hasAccessTime) {
            timestampScratch[2] = kFileTimeUnixEpochLow;
            timestampScratch[3] = kFileTimeUnixEpochHigh;
            timestampScratch[10] = kFileTimeUnixEpochLow;
            timestampScratch[11] = kFileTimeUnixEpochHigh;
            AsArchiveEntryInfo(out)->accessTimeLow = kFileTimeUnixEpochLow;
            AsArchiveEntryInfo(out)->accessTimeHigh = kFileTimeUnixEpochHigh;
        }
        if (hasCreationTime) {
            timestampScratch[0] = kFileTimeUnixEpochLow;
            timestampScratch[1] = kFileTimeUnixEpochHigh;
            timestampScratch[8] = kFileTimeUnixEpochLow;
            timestampScratch[9] = kFileTimeUnixEpochHigh;
            AsArchiveEntryInfo(out)->creationTimeLow = kFileTimeUnixEpochLow;
            AsArchiveEntryInfo(out)->creationTimeHigh = kFileTimeUnixEpochHigh;
        }
        break;
    }

    // Phase 7: commit the complete result to the single-entry cache, then release scratch.
    if (extraField != 0)
        __dla__FPv(extraField);
    memcpy(AsArchiveContext(self)->cachedInfo, out, sizeof(ArchiveEntryInfo));
    AsArchiveContext(self)->cachedIndex = entryIndex;
    return 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624FC", _ctors_u802D4A1C_0, ArchiveEntry_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU804624FC", _ctors_u802D4A1C_1, ResourceTable_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU804624FC", _ctors_u802D4A1C_2, SaveFormFlash_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU804624FC", _ctors_u802D4A1C_3, BuildProduct_InitFullTurnConstant);
#pragma pop
