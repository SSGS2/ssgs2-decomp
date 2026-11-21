#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Translation unit 0x80280150-0x80280D18 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/CSaveFormBanner.h"

#define AsCSaveFormBanner(p) ((CSaveFormBanner*)(p))

enum SaveFormConstants {
    kMd5DigestBytes = 16,
    kApplicationRecordBytes = 0x47C,
    kBannerRecordBytes = 0x72A0
};

// The application record embeds outfit data after its 0x25C-byte prefix.
// Word 0x20 varies independently of the record's meaningful comparison data.
struct ApplicationSaveRecord {
    char prefix[0x20];
    int comparisonIgnoredWord;
    char remainingPrefix[0x238];
    char outfitData[kApplicationRecordBytes - 0x25C];
};
typedef char ApplicationRecordSize[(sizeof(ApplicationSaveRecord) == kApplicationRecordBytes) ? 1 : -1];
typedef char ApplicationOutfitOffset[((int)&((ApplicationSaveRecord*)0)->outfitData == 0x25C) ? 1 : -1];
typedef char ApplicationIgnoredWordOffset[((int)&((ApplicationSaveRecord*)0)->comparisonIgnoredWord == 0x20) ? 1 : -1];

extern "C" {
extern float kFloat_6_2831855;
extern float s_flashReadAccessTau;
extern float s_flashWriteAccessTau;
extern float s_saveFormTau;
extern float s_bannerSaveFormTau;
extern float s_applicationSaveFormTau;

void* CFileIoFlash_CreateInstance(void*, void*);
void* CFileIoAccessWrite_Ctor(void*);
int CFileIoAccessWrite_InitializeBuffer(void*, void*, void*, void*);
void* destroy__18CFileIoAccessWriteFi(void*, int);
void* CSaveForm_dtor(void*, int);
void destroy__17CFileIoAccessReadFi(void*, int);
int CFileIoAccess_GetBuffer(void*);
int SaveForm_FindValidRecord(void*, int);
int SaveForm_GetRecordSize(void*);
int SaveForm_CompareRecordBytes(void*, signed char*, signed char*);
int GameMD5_Verify(void*, unsigned int, void*);
extern unsigned char s_saveFormBaseVtable[0x100];
void __dl__FPv(void*);
void* __nw__FUl(unsigned int);
int SaveRecordPrefix_ClearCtor(void*);

// Partial virtual-call adapter; method order preserves the original ABI slots.
struct FileIoMenuCalls {
// dt stays in f1 on the Wii; the per-frame tick takes it.
    virtual void slot00(); virtual void slot04(); virtual void slot08();
    virtual void slot0C(); virtual void slot10(); virtual void slot14();
    virtual void slot18(); virtual void slot1C(); virtual void slot20();
    virtual int isReady();
    virtual void slot30(); virtual void slot34(); virtual void slot38();
    virtual void slot3C(); virtual void slot40(); virtual void slot44();
    virtual void slot48(); virtual void slot4C(); virtual void slot50();
    virtual void slot54(); virtual int processReadyMenu();
    char baseStorage[0x14];
    void* fileAccess;
    char menuStorage[0xC];
    void* saveForm;
};

// Run the ready-menu callback when ready; otherwise return the original idle result of one.
int CFileIoMenu_ProcessWhenReady(FileIoMenuCalls* p) {
    if (p->isReady())
        return p->processReadyMenu();
    return 1;
}

extern unsigned char __vt__23CFileIoAccessWriteFlash[];
struct FlashWriteAccessStorage {
    void* vtable;
    char padding[0x10];
    FlashWriteAccessStorage() { CFileIoAccessWrite_Ctor(this); vtable = __vt__23CFileIoAccessWriteFlash; }
};
// Create flash write access and destroy it on buffer-initialization failure.
extern "C" void* CFileIoAccessWriteFlash_Create(void* a, void* b, void* c) {
    FlashWriteAccessStorage* p = new FlashWriteAccessStorage;
    if (!p) return 0;
    if (!CFileIoAccessWrite_InitializeBuffer(p, a, b, c)) {
        if (p) {
            struct DestroyCaller { 
            virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void b() DECOMP_LOCAL_VIRTUAL; virtual void c() DECOMP_LOCAL_VIRTUAL; virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((DestroyCaller*)p)->destroy(1);
        }
        return 0;
    }
    return p;
}

// Compare application records after clearing the one word excluded from meaningful equality.
extern "C" int CSaveFormApplication_CompareRecords(void* saveForm, void* leftRecord, void* rightRecord) {
    if (!leftRecord || !rightRecord) return 0;
    unsigned int recordBytes = (unsigned int)SaveForm_GetRecordSize(saveForm);
    if (recordBytes != kApplicationRecordBytes) return 0;
    char first[kApplicationRecordBytes];
    char second[kApplicationRecordBytes];
    SaveRecordPrefix_ClearCtor(first);
    Memory_Copy(first, leftRecord, recordBytes);
    ((ApplicationSaveRecord*)first)->comparisonIgnoredWord = 0;
    SaveRecordPrefix_ClearCtor(second);
    Memory_Copy(second, rightRecord, recordBytes);
    ((ApplicationSaveRecord*)second)->comparisonIgnoredWord = 0;
    return SaveForm_CompareRecordBytes(saveForm, (signed char*)first, (signed char*)second);
}

// Find the first valid record in the file-access buffer using this menu's save form.
int CFileIoMenu_FindValidSaveRecord(FileIoMenuCalls* p) {
    if (!p->fileAccess || !p->saveForm)
        return 0;
    int bufferAddress = CFileIoAccess_GetBuffer(p->fileAccess);
    return SaveForm_FindValidRecord(p->saveForm, bufferAddress);
}

// Bounds-check a record and verify its optional trailing MD5 digest.
int SaveForm_ValidateRecord(void* p, void* base, unsigned int index) {
    char* result;
    unsigned int recordCount = AsCSaveFormBanner(p)->recordCount;
    if (recordCount <= index) {
        result = 0;
    } else if (!base) {
        result = 0;
    } else {
        unsigned char hasMd5 = AsCSaveFormBanner(p)->hasMd5;
        int mask = (-(int)hasMd5 | hasMd5) >> 31;
        result = (char*)base + index * ((mask & kMd5DigestBytes) + AsCSaveFormBanner(p)->recordSize);
    }
    if (!result)
        return 0;
    if (!AsCSaveFormBanner(p)->hasMd5)
        return 1;
    return GameMD5_Verify(result, AsCSaveFormBanner(p)->recordSize, result + AsCSaveFormBanner(p)->recordSize);
}

// Compare exactly recordSize bytes and return boolean equality.
int SaveForm_CompareRecordBytes(void* saveForm, signed char* leftRecord, signed char* rightRecord) {
    unsigned int byteCount = AsCSaveFormBanner(saveForm)->recordSize;
    for (unsigned int i = 0; i < byteCount; ++i) {
        if (leftRecord[i] != rightRecord[i])
            return 0;
    }
    return 1;
}

// Return the indexed record using a stride that includes the optional MD5 trailer.
void* SaveForm_GetRecord(void* p, void* base, unsigned int index) {
    unsigned int recordCount = AsCSaveFormBanner(p)->recordCount;
    if (recordCount <= index)
        return 0;
    if (base == 0)
        return 0;
    int mask = (-((int)AsCSaveFormBanner(p)->hasMd5) | AsCSaveFormBanner(p)->hasMd5) >> 31;
    unsigned int recordSize = AsCSaveFormBanner(p)->recordSize;
    return (char*)base + (recordSize + (mask & kMd5DigestBytes)) * index;
}

// Destroy flash read access and optionally free its storage.
void* CFileIoAccessReadFlash_dtor(void* p, int deleting) {
    if (p) {
        destroy__17CFileIoAccessReadFi(p, 0);
        if (deleting > 0)
            __dl__FPv(p);
    }
    return p;
}

// Destroy flash write access and optionally free its storage.
void* CFileIoAccessWriteFlash_dtor(void* p, int deleting) {
    if (p) {
        destroy__18CFileIoAccessWriteFi(p, 0);
        if (deleting > 0)
            __dl__FPv(p);
    }
    return p;
}

// Destroy the base form and optionally free banner-form storage.
void* CSaveFormBanner_dtor(void* p, int deleting) {
    if (p) {
        CSaveForm_dtor(p, 0);
        if (deleting > 0)
            __dl__FPv(p);
    }
    return p;
}

// Destroy the base form and optionally free application-form storage.
void* CSaveFormApplication_dtor(void* p, int deleting) {
    if (p) {
        CSaveForm_dtor(p, 0);
        if (deleting > 0)
            __dl__FPv(p);
    }
    return p;
}

struct SaveFormFields {
    void* vtable;
    int recordSize;
    int recordCount;
    unsigned char hasMd5;
};
void CSaveForm_Ctor(SaveFormFields* p, int recordSizeArg, int recordCountArg, int hasMd5);

extern unsigned char __vt__20CSaveFormApplication[];
struct ApplicationFormCalls {
    virtual void destroy(int);
    virtual void slot0C();
    virtual int validateConfiguration();
};
struct ApplicationFormStorage : SaveFormFields {
    ApplicationFormStorage(void* a, void* b) {
        CSaveForm_Ctor(this, kApplicationRecordBytes, (int)a, (int)b);
        vtable = __vt__20CSaveFormApplication;
    }
};
#pragma opt_lifetimes off
// Create a 0x47C-byte application-record form and reject invalid configuration.
extern "C" void* CSaveFormApplication_Create(void* a, void* b) {
    ApplicationFormStorage* p = new ApplicationFormStorage(a, b);
    if (!p) return 0;
    ApplicationFormCalls* vp = (ApplicationFormCalls*)p;
    if (!vp->validateConfiguration()) {
        if (p) vp->destroy(1);
        return 0;
    }
    return p;
}
#pragma opt_lifetimes reset

extern unsigned char __vt__15CSaveFormBanner[];
extern unsigned char __vt__22CFileIoAccessReadFlash[];
extern void CFileIoAccessRead_Ctor(void*);
struct FlashReadAccessCalls {
    virtual void a(); virtual void b(); virtual void c();
    virtual void destroy(int);
    virtual int initialize(void*, void*);
};
struct FlashReadAccessStorage {
    void* vtable;
    char pad[0xC];
    FlashReadAccessStorage() { CFileIoAccessRead_Ctor(this); vtable = __vt__22CFileIoAccessReadFlash; }
};
// Create flash read access and destroy it on initialization failure.
extern "C" void* CFileIoAccessReadFlash_Create(void* a, void* b) {
    FlashReadAccessStorage* p = new FlashReadAccessStorage;
    if (!p) return 0;
    if (!((FlashReadAccessCalls*)p)->initialize(a, b)) {
        if (p) ((FlashReadAccessCalls*)p)->destroy(1);
        return 0;
    }
    return p;
}
extern int s_applicationSaveBufferAlignment; // 4: required base-buffer address alignment
extern void* SaveForm_GetRecord(void*, void*, unsigned int);
extern int SaveData_ValidateCharacterOutfitParts(void*);
extern int GameMD5_Verify(void*, unsigned int, void*);
#pragma inline_depth(0)
// Verify the indexed record and buffer alignment, then validate its embedded outfit data.
extern "C" int CSaveFormApplication_ValidateOutfitRecord(void* saveForm, int bufferAddress, void* recordIndex) {
    if (!SaveForm_ValidateRecord(saveForm, (void*)bufferAddress, (unsigned int)recordIndex))
        return 0;
    if (bufferAddress % s_applicationSaveBufferAlignment)
        return 0;
    void* record = SaveForm_GetRecord(saveForm, (void*)bufferAddress, (unsigned int)recordIndex);
    return SaveData_ValidateCharacterOutfitParts(((ApplicationSaveRecord*)record)->outfitData) != 0;
}
struct BannerFormCalls {
    virtual void destroy(int);
    virtual void slot04();
    virtual int validateConfiguration();
};
struct BannerFormStorage {
    void* vtable;
    int recordSize;
    int recordCount;
    unsigned char hasMd5;
    BannerFormStorage() { CSaveForm_Ctor((SaveFormFields*)this, kBannerRecordBytes, 1, 0); vtable = __vt__15CSaveFormBanner; }
};
#pragma inline_depth(1)
// Create the single 0x72A0-byte banner form and reject invalid configuration.
extern "C" void* CSaveFormBanner_Create() {
    BannerFormStorage* p = new BannerFormStorage;
    if (!p)
        return 0;
    BannerFormCalls* q = (BannerFormCalls*)p;
    if (!q->validateConfiguration()) {
        if (p)
            q->destroy(1);
        return 0;
    }
    return p;
}
#pragma inline_depth(0)

// Initialize base form record size/count and the optional-MD5 flag.
void CSaveForm_Ctor(SaveFormFields* p, int recordSizeArg, int recordCountArg, int hasMd5) {
    p->recordSize = recordSizeArg;
    p->vtable = &s_saveFormBaseVtable;
    p->recordCount = recordCountArg;
    p->hasMd5 = hasMd5;
}

// Return record count times payload size plus any sixteen-byte MD5 trailer.
int SaveForm_GetStorageSize(SaveFormFields* p) {
    unsigned char hasMd5 = p->hasMd5;
    int recordStrideBytes = p->recordSize;
    recordStrideBytes += ((-(int)hasMd5 | hasMd5) >> 31) & kMd5DigestBytes;
    return recordStrideBytes * p->recordCount;
}

// Free form storage only for a positive deleting-destructor argument.
void* CSaveForm_dtor(void* p, int deletingFlag) {
    if (p && deletingFlag > 0)
        __dl__FPv(p);
    return p;
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void FlashReadAccess_InitTauConstant() {
    s_flashReadAccessTau = kFloat_6_2831855;
}

// Create the flash I/O backend using the final two factory arguments.
void* CFileIoAccessReadFlash_CreateFlash(void* a, void* b, void* c) {
    return CFileIoFlash_CreateInstance(b, c);
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void FlashWriteAccess_InitTauConstant() {
    s_flashWriteAccessTau = kFloat_6_2831855;
}

// Create the flash I/O backend using the final two factory arguments.
void* CFileIoAccessWriteFlash_createFlash(void* a, void* b, void* c) {
    return CFileIoFlash_CreateInstance(b, c);
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void SaveForm_InitTauConstant() {
    s_saveFormTau = kFloat_6_2831855;
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void BannerSaveForm_InitTauConstant() {
    s_bannerSaveFormTau = kFloat_6_2831855;
}

// Return the payload size, excluding the digest trailer.
int SaveForm_GetRecordSize(void* p) {
    return (int)AsCSaveFormBanner(p)->recordSize;
}

struct SaveFormCalls {
    virtual void slot08();
    virtual int validateRecord(void*, unsigned int);
    virtual void slot10();
    virtual int compareRecord(void*, void*);
    unsigned int recordSize;
    unsigned int recordCount;
    unsigned char hasMd5;
};

// Scan the stored records and return the first whose virtual validator accepts it.
int SaveForm_FindValidRecord(void* p, int baseArg) {
    unsigned int i = 0;
    unsigned int recordCount = AsCSaveFormBanner(p)->recordCount;
    for (; i < recordCount; ++i) {
        if (!((SaveFormCalls*)p)->validateRecord((void*)baseArg, i))
            continue;
        unsigned int n = AsCSaveFormBanner(p)->recordCount;
        if (n <= i) continue;
        if (!baseArg) continue;
        unsigned int recordSize = AsCSaveFormBanner(p)->recordSize;
        unsigned char hasMd5 = AsCSaveFormBanner(p)->hasMd5;
        int mask = (-(int)hasMd5 | hasMd5) >> 31;
        recordSize += mask & kMd5DigestBytes;
        void* item = (char*)baseArg + i * recordSize;
        if (item != 0) return (int)item;
    }
    return 0;
}

#pragma use_lmw_stmw on
// Validate every input record and compare it against the supplied reference within the capacity bound.
int SaveForm_VerifyAllRecordCopies(SaveFormCalls* self, void* input, void* referenceRecord, unsigned int capacity) {
    unsigned int recordCount = self->recordCount;
    int mask = (-(int)self->hasMd5 | self->hasMd5) >> 31;
    unsigned int recordSize = (mask & kMd5DigestBytes) + self->recordSize;
    if (recordCount * recordSize > capacity) return 0;
    for (unsigned int i = 0; i < recordCount; ++i) {
        int ok;
        if (input == 0 || referenceRecord == 0) {
            ok = 0;
        } else if (self->validateRecord(input, i) == 0) {
            ok = 0;
        } else {
            void* item;
            if (self->recordCount <= i) {
                item = 0;
            } else if (input == 0) {
                item = 0;
            } else {
                unsigned char hasExtra = self->hasMd5;
                int imask = (-(int)hasExtra | hasExtra) >> 31;
                item = (char*)input + i * ((imask & kMd5DigestBytes) + self->recordSize);
            }
            if (item != 0) {
                ok = self->compareRecord(item, referenceRecord);
            } else {
                ok = 0;
            }
        }
        if (ok == 0) return 0;
    }
    return 1;
}

extern int GameMD5_Compute(void*, void*, unsigned int);
#pragma opt_lifetimes off
// Clear storage, compute an optional digest, and replicate one payload/digest into every record.
extern "C" int SaveForm_ReplicateWithDigest(SaveFormCalls* self, void* base, void* data, unsigned int size) {
#pragma use_lmw_stmw on
    if (self->recordSize != size) return 0;
    if (!base || !data) return 0;
    unsigned int clearCount;
    int clearFlag;
    clearFlag = self->hasMd5;
    clearCount = self->recordCount;
    int mask0 = (-(int)clearFlag | clearFlag) >> 31;
    unsigned int recordSize = (mask0 & kMd5DigestBytes) + self->recordSize;
    Memory_Clear(base, clearCount * recordSize);
    char extra[kMd5DigestBytes];
    if (self->hasMd5) {
        if (!GameMD5_Compute(extra, data, size)) return 0;
    }
    void* item;
    int i;
    int recordCount = (int)self->recordCount;
    for (i = 0; i < recordCount; ++i) {
        if (self->recordCount <= (unsigned int)i) {
            item = 0;
        } else if (!base) {
            item = 0;
        } else {
            unsigned char hasMd5 = self->hasMd5;
            int imask = (-(int)hasMd5 | hasMd5) >> 31;
            item = (char*)base + i * ((imask & kMd5DigestBytes) + self->recordSize);
        }
        if (!item) return 0;
        Memory_Copy(item, data, size);
        if (self->hasMd5) Memory_Copy((char*)item + size, extra, kMd5DigestBytes);
    }
    return 1;
}
#pragma opt_lifetimes reset

// Accept banner-form configuration unconditionally.
int CSaveFormBanner_vfunc_2() {
    return 1;
}

// Initialize this module's stored two-pi constant from the shared scalar pool.
void ApplicationSaveForm_InitTauConstant() {
    s_applicationSaveFormTau = kFloat_6_2831855;
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623D0", _ctors_u80280150_0, FlashReadAccess_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU804623D0", _ctors_u80280150_1, FlashWriteAccess_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU804623D0", _ctors_u80280150_2, SaveForm_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU804623D0", _ctors_u80280150_3, BannerSaveForm_InitTauConstant);
STATIC_INIT_ENTRY(".CRT$XCU804623D0", _ctors_u80280150_4, ApplicationSaveForm_InitTauConstant);
#pragma pop
