#include <decomp/static_init.h>
#include "classes/CFileIoAccessWrite.h"
#include "classes/CFileIoAccessWritePad.h"
#include "classes/CFileIoFlash.h"
#include "classes/CFileIo.h"
#include <decomp/cw_bitfields.h>

#define AsCFileIoFlash(p) ((CFileIoFlash*)(p))

#define AsCFileIoAccessWrite(p) ((CFileIoAccessWrite*)(p))
// Translation unit 0x80276AEC-0x80277528 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
enum FlashStatus { kFlashStatusC361 = 0xC361, kFlashStatusC354 = 0xC354, kFlashStatusC353 = 0xC353 };
int CFileIoPad_vfunc_14();
// Delegate the flash device availability query to the common device hook.
int CFileIoFlash_vfunc_14() { return CFileIoPad_vfunc_14(); }

void* CFileIoPad_CreateInstance(void*, int, int);
struct FlashDebugOptions { char reserved[0xA]; unsigned char synchronous; };
// Create a pad-file instance through this write-access object's owner.
void* CFileIoAccessWritePad_CreatePad(CFileIoAccessWritePad* self, int a, int b) {
    return CFileIoPad_CreateInstance(self->owner, a, b);
}

int CFileIo_QueryDiskFreeSpace(void*, int);
// Delegate this flash operation with its default zero option.
int CFileIoFlash_QueryDiskFreeSpaceDefault(void* p) {
    return CFileIo_QueryDiskFreeSpace(p, 0);
}

// Return false for this unsupported flash capability.
int CFileIoFlash_vfunc_9() {
    return 0;
}

extern const float kFloat_6_2831855;
extern float s_flashFileTwoPiCache;
extern float s_padWriteTwoPiCache;
extern float s_flashWriteTwoPiCache;

int CFileIo_GetStatusCode(void*);
void destroy__18CFileIoAccessWriteFi(void*, int);
void __dl__FPv(void*);
void* CFlashMemoryDebug_GetInstance(void*);
void FileIo_ReadAlignedBuffer(void*, void*);
void CFileIo_DeletePath(void*, void*);
void CFileIo_Open(void*, void*, void*);
void CFileIo_WritePaddedBuffer(void*, void*, void*);
void CFileIoFlash_CheckSaveArea(void*, void*, void*, void*);
void CFileIoFlash_OnReadWriteComplete(void*, int);
void CFileIoFlash_OnOpenComplete(int, int);
void CFileIoFlash_OnSaveAreaCheckComplete(void*, int);
void CFileIoFlash_OnReadWriteComplete(void*, int);
void CFileIoFlash_CompleteWithStatus(int);
void CFileIoFlash_Open(void*, void*);
void CFileIoFlash_StartPaddedWrite(void*, void*);
void CFileIoFlash_DispatchSaveAreaCheck(void*, void*, void*);

// Destroy the write-access base and optionally delete the pad-write object.
void* CFileIoAccessWritePad_dtor(void* self, int flag) {
    if (self != 0) {
        destroy__18CFileIoAccessWriteFi(self, 0);
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Read the aligned buffer synchronously in debug mode or install the transfer callback.
void CFileIoFlash_ReadAlignedBuffer(void* self) {
    if (((FlashDebugOptions*)CFlashMemoryDebug_GetInstance(self))->synchronous != 0) {
        FileIo_ReadAlignedBuffer(self, 0);
    } else {
        FileIo_ReadAlignedBuffer(self, (void*)CFileIoFlash_OnReadWriteComplete);
    }
}

// Run the delegated operation with an optional status-completion callback.
void CFileIoFlash_DispatchDeletePath(void* self) {
    if (((FlashDebugOptions*)CFlashMemoryDebug_GetInstance(self))->synchronous != 0) {
        CFileIo_DeletePath(self, 0);
    } else {
        CFileIo_DeletePath(self, (void*)CFileIoFlash_CompleteWithStatus);
    }
}

// Open a file with synchronous debug handling or the flash-open completion callback.
void CFileIoFlash_Open(void* self, void* openMode) {
    if (((FlashDebugOptions*)CFlashMemoryDebug_GetInstance(self))->synchronous != 0) {
        CFileIo_Open(self, 0, openMode);
    } else {
        CFileIo_Open(self, (void*)CFileIoFlash_OnOpenComplete, openMode);
    }
}

// Write the padded buffer with optional transfer completion notification.
void CFileIoFlash_StartPaddedWrite(void* self, void* sourceBuffer) {
    if (((FlashDebugOptions*)CFlashMemoryDebug_GetInstance(self))->synchronous != 0) {
        CFileIo_WritePaddedBuffer(self, sourceBuffer, 0);
    } else {
        CFileIo_WritePaddedBuffer(self, sourceBuffer, (void*)CFileIoFlash_OnReadWriteComplete);
    }
}

// Check the save area with optional completion notification.
void CFileIoFlash_DispatchSaveAreaCheck(void* self, void* saveArea, void* checkOptions) {
    if (((FlashDebugOptions*)CFlashMemoryDebug_GetInstance(self))->synchronous != 0) {
        CFileIoFlash_CheckSaveArea(self, saveArea, checkOptions, 0);
    } else {
        CFileIoFlash_CheckSaveArea(self, saveArea, checkOptions, (void*)CFileIoFlash_OnSaveAreaCheckComplete);
    }
}

void CFileIoFlash_MoveQualifiedPath(void*, void*, void*, void*);
int CFileIoFlash_OnQualifiedPathMoveComplete();
// Move sourcePath to destinationPath with optional busy-flag completion notification.
void CFileIoFlash_DispatchQualifiedPathMove(void* self, void* destinationPath, void* sourcePath) {
    if (((FlashDebugOptions*)CFlashMemoryDebug_GetInstance(self))->synchronous != 0) {
        CFileIoFlash_MoveQualifiedPath(self, destinationPath, sourcePath, 0);
    } else {
        CFileIoFlash_MoveQualifiedPath(self, destinationPath, sourcePath, (void*)CFileIoFlash_OnQualifiedPathMoveComplete);
    }
}

void* __nwa__FUl(void*);
void Memory_Copy(void*, void*, void*);
int CFileIoAccess_CreateResource(void*, void*, void*);
int CFileIoAccessWrite_InitializeBuffer(void* self, void* openOptions, void* byteCount, void* sourceData);

void* __nw__FUl(unsigned int);
void* CFileIoAccessWrite_Ctor(void*);
extern char __vt__21CFileIoAccessWritePad[];
struct Type80276BFCDispatch {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void destroy(int);
};
inline CFileIoAccessWritePad::CFileIoAccessWritePad(void* o) {
    CFileIoAccessWrite_Ctor(this);
    *(void**)this = (void*)__vt__21CFileIoAccessWritePad;
    owner = o;
}
// Allocate pad-write access, initialize its copied buffer, and delete it on failure.
void* CFileIoAccessWritePad_Create(void* owner, void* openOptions, void* byteCount, void* sourceData) {
    CFileIoAccessWritePad* p = new CFileIoAccessWritePad(owner);
    if (p == 0) {
        return 0;
    }
    if (CFileIoAccessWrite_InitializeBuffer(p, openOptions, byteCount, sourceData) == 0) {
        if (p != 0) {
            ((Type80276BFCDispatch*)p)->destroy(1);
        }
        return 0;
    }
    return p;
}

// Initialize the flash-file helper's two-pi cache.
void CFileIoFlash_InitAngleCache() {
    s_flashFileTwoPiCache = kFloat_6_2831855;
}

// Initialize the pad-write helper's two-pi cache.
void CFileIoAccessWritePad_InitAngleCache() {
    s_padWriteTwoPiCache = kFloat_6_2831855;
}

// Initialize the flash-write helper's two-pi cache.
void CFileIoAccessWrite_InitAngleCache() {
    s_flashWriteTwoPiCache = kFloat_6_2831855;
}

// Report whether the cached file status is nonzero.
int CFileIoFlash_HasNonzeroStatus(void* p) {
    return ((CFileIo*)p)->statusCode != 0;
}

// Test for status 0xC361; its higher-level error meaning is unresolved.
int CFileIoFlash_vfunc_10(void* p) {
    return CFileIo_GetStatusCode(p) == kFlashStatusC361;
}

// Test for status 0xC354; its higher-level error meaning is unresolved.
int CFileIoFlash_vfunc_11(void* p) {
    return CFileIo_GetStatusCode(p) == kFlashStatusC354;
}

// Test for status 0xC353; its higher-level error meaning is unresolved.
int CFileIoFlash_vfunc_8(void* p) {
    return CFileIo_GetStatusCode(p) == kFlashStatusC353;
}

extern void* g_fileIoFlash;
void CFileIoFlash_ClearBusy(void*);
int CFileIo_SetStatusCode(void*, int);
void CFileIo_SetTransferredBytes(void*, int);
void CFileIo_SetSaveAreaStatusFlags(void*, int);
void CFileIo_dtor(void*, int);
void CFileIoAccess_Ctor(void*);
void CFileIoFlash_OnReadWriteComplete(void*, int);
extern char __vt__18CFileIoAccessWrite[];
// Clear the live flash singleton's busy flag after a qualified-path move.
int CFileIoFlash_OnQualifiedPathMoveComplete() {
    void* p = g_fileIoFlash;
    if (p == 0) {
        return 0;
    }
    CFileIoFlash_ClearBusy(p);
    return 0;
}

// Publish completion status and clear busy on the live flash singleton.
void CFileIoFlash_CompleteWithStatus(int status) {
    if (g_fileIoFlash != 0) {
        CFileIo_SetStatusCode(g_fileIoFlash, status);
        CFileIoFlash_ClearBusy(g_fileIoFlash);
    }
}

// Publish transfer status and completed byte count, then clear busy.
void CFileIoFlash_OnReadWriteComplete(void* status, int transferredBytes) {
    if (g_fileIoFlash != 0) {
        CFileIo_SetStatusCode(g_fileIoFlash, (int)status);
        CFileIo_SetTransferredBytes(g_fileIoFlash, transferredBytes);
        CFileIoFlash_ClearBusy(g_fileIoFlash);
    }
}

// Publish status and save-area flags, then clear busy.
void CFileIoFlash_OnSaveAreaCheckComplete(void* status, int saveAreaFlags) {
    if (g_fileIoFlash != 0) {
        CFileIo_SetStatusCode(g_fileIoFlash, (int)status);
        CFileIo_SetSaveAreaStatusFlags(g_fileIoFlash, saveAreaFlags);
        CFileIoFlash_ClearBusy(g_fileIoFlash);
    }
}

// Clear the flash singleton, destroy the file-I/O base and optionally delete the object.
void* CFileIoFlash_dtor(void* p, int flag) {
    if (p != 0) {
        g_fileIoFlash = 0;
        CFileIo_dtor(p, 0);
        if (flag > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// Initialize the access base, write state and empty copied-data buffer.
void* CFileIoAccessWrite_Ctor(void* p) {
    CFileIoAccess_Ctor(p);
    AsCFileIoAccessWrite(p)->state = 0;
    *(void**)p = (void*)__vt__18CFileIoAccessWrite;
    (*(int*)&AsCFileIoAccessWrite(p)->buffer) = 0;
    return p;
}

void* CFileIo_AddRef(void*);
void* CFileIo_InitMembers(void*);
extern char __vt__12CFileIoFlash[];
struct Manager {
    virtual void f0(); virtual void f1(); virtual void f2(); virtual void f3();
    virtual void f4(); virtual void f5(); virtual void f6(); virtual void f7();
    virtual void f8(); virtual void f9(); virtual void f10(); virtual void f11();
    virtual void destroy(int);
    virtual void* create(void*, void*);
    virtual void* is_ready();
    char reserved[0x54];
};

// Construction view of CFileIoFlash; canonical members: classes/CFileIoFlash.h.
// Both factories allocate 0x58 bytes; the shared header only describes +0..+0x0b.
struct CFileIoFlashConstructorView {
    char storage[0x58];
    CFileIoFlashConstructorView() {
        CFileIo_InitMembers(this);
        *(void**)this = (void*)__vt__12CFileIoFlash;
    }
};

// Acquire the existing singleton or create and validate a new flash-file object.
void* CFileIoFlash_AcquireInstance() {
    if (g_fileIoFlash != 0) {
        if (CFileIo_AddRef(g_fileIoFlash) != 0) return g_fileIoFlash;
        return 0;
    }
    CFileIoFlashConstructorView* p = new CFileIoFlashConstructorView();
    g_fileIoFlash = p;
    if (p == 0) return 0;
    Manager* mp = (Manager*)p;
    if (mp->is_ready() == 0) {
        if (mp != 0) mp->destroy(1);
        return 0;
    }
    return mp;
}

// Acquire the existing singleton or create one using the supplied initialization arguments.
void* CFileIoFlash_CreateInstance(void* a, void* b) {
    if (g_fileIoFlash != 0) {
        if (CFileIo_AddRef(g_fileIoFlash) != 0) return g_fileIoFlash;
        return 0;
    }
    CFileIoFlashConstructorView* p = new CFileIoFlashConstructorView();
    g_fileIoFlash = p;
    if (p == 0) return 0;
    Manager* mp = (Manager*)p;
    if (mp->create(a, b) == 0) {
        if (mp != 0) mp->destroy(1);
        return 0;
    }
    return mp;
}

extern "C" void appFreeUni__FPv(void*);
struct SmallString {
    union {
        unsigned int mode;
        struct { CW_BITS2( unsigned char big : 1, unsigned char len : 7) } b;
    };
    unsigned int size;
    char* heap;
    SmallString() { *(volatile unsigned int*)&mode = 0; *(volatile unsigned int*)&size = 0; *(char* volatile*)&heap = 0; }
    ~SmallString() {
        if (mode >> 31) appFreeUni__FPv(heap);
    }
};
struct Alloc80277280 { volatile unsigned char c; char pad[3]; };
extern "C" void String_ReplaceRange(SmallString*, int, unsigned int, const char*, const char*, Alloc80277280*);
extern "C" void appFreeUni__FPv(void*);
extern "C" int CFileIo_SetPathAndAllocateBuffer(void*, const char*, void*);
extern "C" unsigned int strlen(const char*);
extern "C" char* g_flashFilePathPrefix;
#pragma push
#pragma use_lmw_stmw on
// Prefix the requested path and pass the resulting string to common file setup.
int CFileIoFlash_SetPathAndAllocateBuffer(void* self, const char* path, void* openOptions) {
    Alloc80277280 x0;
    Alloc80277280 x;
    Alloc80277280 y0;
    Alloc80277280 y;
    SmallString qualifiedPath;
    unsigned int appendPosition = qualifiedPath.b.len;
    x.c = x0.c;
    String_ReplaceRange(&qualifiedPath, 0, appendPosition, g_flashFilePathPrefix, g_flashFilePathPrefix + strlen(g_flashFilePathPrefix), &x);
    appendPosition = !(qualifiedPath.mode >> 31) ? qualifiedPath.b.len : qualifiedPath.size;
    y.c = y0.c;
    String_ReplaceRange(&qualifiedPath, appendPosition, 0, path, path + strlen(path), &y);
    const char* pathText = !(qualifiedPath.mode >> 31) ? (const char*)&qualifiedPath.mode + 1 : qualifiedPath.heap;
    if (CFileIo_SetPathAndAllocateBuffer(self, pathText, openOptions) == 0) return 0;
    return 1;
}
#pragma pop

// Publish open status and clear busy when the completion condition is satisfied.
void CFileIoFlash_OnOpenComplete(int status, int completionFlag) {
    if (status == 0 || completionFlag != 0) {
        if (g_fileIoFlash != 0) {
            CFileIo_SetStatusCode(g_fileIoFlash, status);
            CFileIoFlash_ClearBusy(g_fileIoFlash);
        }
    }
}

// Replace the owned buffer with a copy of the requested data, then initialize base access.
int CFileIoAccessWrite_InitializeBuffer(void* self, void* openOptions, void* byteCount, void* sourceData) {
    if (byteCount == 0) {
        return 0;
    }
    if (sourceData == 0) {
        return 0;
    }
    if (AsCFileIoAccessWrite(self)->buffer != 0) {
        __dl__FPv(AsCFileIoAccessWrite(self)->buffer);
        AsCFileIoAccessWrite(self)->buffer = 0;
    }
    AsCFileIoAccessWrite(self)->buffer = __nwa__FUl(byteCount);
    if (AsCFileIoAccessWrite(self)->buffer == 0) {
        return 0;
    }
    Memory_Copy(AsCFileIoAccessWrite(self)->buffer, sourceData, byteCount);
    return CFileIoAccess_CreateResource(self, openOptions, byteCount);
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623A4", _ctors_u80276AEC_0, CFileIoFlash_InitAngleCache);
STATIC_INIT_ENTRY(".CRT$XCU804623A4", _ctors_u80276AEC_1, CFileIoAccessWritePad_InitAngleCache);
STATIC_INIT_ENTRY(".CRT$XCU804623A4", _ctors_u80276AEC_2, CFileIoAccessWrite_InitAngleCache);
#pragma pop
