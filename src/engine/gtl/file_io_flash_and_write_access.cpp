// Flash-memory file I/O: the CFileIoFlash singleton and its write-access helpers, CFileIoAccessWrite
// and CFileIoAccessWritePad. They delegate to the pad-file code, start operations with completion
// callbacks, and record the status and clear the busy flag when each completion arrives.

#include <decomp/static_init.h>
#include "classes/CFileIoAccessWrite.h"
#include "classes/CFileIoAccessWritePad.h"
#include "classes/CFileIoFlash.h"
#include "classes/CFileIoPad.h"
#include "classes/CFileIo.h"
#include "classes/CFlashMemoryDebug.h"
#include <decomp/cw_bitfields.h>

extern "C" {
enum FlashStatus {
    kFlashStatusC361 = 0xC361,
    kFlashStatusC354 = 0xC354,
    kFlashStatusC353 = 0xC353
};

struct FlashDebugOptions {
    char reserved[0xA];
    unsigned char synchronous;
};

} // extern "C"

// Create a pad-file instance through this write-access object's owner.
void* CFileIoAccessWritePad::CreatePad(int a, int b) {
    return CFileIoPad::CreateInstance(owner, a, b);
}

// Delegate the flash device availability query to the common pad-file hook.
int CFileIoFlash::vfunc_14() {
    return CFileIoPad::vfunc_14();
}

// Delegate this flash operation with its default zero option.
int CFileIoFlash::QueryDiskFreeSpaceDefault() {
    return ((CFileIo*)this)->QueryDiskFreeSpace(0);
}

// Return false for this unsupported flash capability.
int CFileIoFlash::vfunc_9() {
    return 0;
}

extern "C" {

extern const float kFloat_6_2831855;
extern float s_flashFileTwoPiCache;
extern float s_padWriteTwoPiCache;
extern float s_flashWriteTwoPiCache;

void destroy__18CFileIoAccessWriteFi(void*, int);
void __dl__FPv(void*);
int FileIo_ReadAlignedBuffer(void*, void*);

} // extern "C"

// Destroy the write-access base and optionally delete the pad-write object.
void* CFileIoAccessWritePad::Destroy(int flag) {
    if (this != 0) {
        destroy__18CFileIoAccessWriteFi(this, 0);
        if (flag > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}

// Read the aligned buffer synchronously in debug mode or install the transfer callback.
int CFileIoFlash::ReadAlignedBuffer() {
    if (((FlashDebugOptions*)CFlashMemoryDebug::GetInstance())->synchronous != 0) {
        return FileIo_ReadAlignedBuffer(this, 0);
    } else {
        return FileIo_ReadAlignedBuffer(this, (void*)CFileIoFlash::OnReadWriteComplete);
    }
}

// Run the delegated operation with an optional status-completion callback.
void CFileIoFlash::DispatchDeletePath() {
    if (((FlashDebugOptions*)CFlashMemoryDebug::GetInstance())->synchronous != 0) {
        ((CFileIo*)this)->DeletePath(0);
    } else {
        ((CFileIo*)this)->DeletePath((int)CFileIoFlash::CompleteWithStatus);
    }
}

// Open a file with synchronous debug handling or the flash-open completion callback.
int CFileIoFlash::Open(void* openMode) {
    if (((FlashDebugOptions*)CFlashMemoryDebug::GetInstance())->synchronous != 0) {
        return ((CFileIo*)this)->Open(0, openMode);
    } else {
        return ((CFileIo*)this)->Open((int)CFileIoFlash::OnOpenComplete, openMode);
    }
}

// Write the padded buffer with optional transfer completion notification.
int CFileIoFlash::StartPaddedWrite(void* sourceBuffer) {
    if (((FlashDebugOptions*)CFlashMemoryDebug::GetInstance())->synchronous != 0) {
        return ((CFileIo*)this)->WritePaddedBuffer(sourceBuffer, 0);
    } else {
        return ((CFileIo*)this)->WritePaddedBuffer(sourceBuffer, (int)CFileIoFlash::OnReadWriteComplete);
    }
}

// Check the save area with optional completion notification.
void CFileIoFlash::DispatchSaveAreaCheck(void* saveArea, void* checkOptions) {
    if (((FlashDebugOptions*)CFlashMemoryDebug::GetInstance())->synchronous != 0) {
        CheckSaveArea(saveArea, checkOptions, 0);
    } else {
        CheckSaveArea(saveArea, checkOptions, (int)CFileIoFlash::OnSaveAreaCheckComplete);
    }
}

// Move sourcePath to destinationPath with optional busy-flag completion notification.
int CFileIoFlash::DispatchQualifiedPathMove(void* destinationPath, void* sourcePath) {
    if (((FlashDebugOptions*)CFlashMemoryDebug::GetInstance())->synchronous != 0) {
        return MoveQualifiedPath(destinationPath, (const char*)sourcePath, 0);
    } else {
        return MoveQualifiedPath(destinationPath, (const char*)sourcePath,
                                 (int)CFileIoFlash::OnQualifiedPathMoveComplete);
    }
}

extern "C" {

void* __nwa__FUl(void*);
void Memory_Copy(void*, void*, void*);

void* __nw__FUl(unsigned int);
extern char __vt__21CFileIoAccessWritePad[];
struct CFileIoAccessWritePadView {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void destroy(int);
};
inline CFileIoAccessWritePad::CFileIoAccessWritePad(void* o) {
    ((CFileIoAccessWrite*)this)->Init();
    *(void**)this = (void*)__vt__21CFileIoAccessWritePad;
    owner = o;
}
} // extern "C"

// Allocate pad-write access, initialize its copied buffer, and delete it on failure.
void* CFileIoAccessWritePad::Create(void* owner, void* openOptions, void* byteCount, void* sourceData) {
    CFileIoAccessWritePad* p = new CFileIoAccessWritePad(owner);
    if (p == 0) {
        return 0;
    }
    if (((CFileIoAccessWrite*)p)->InitializeBuffer(openOptions, byteCount, sourceData) == 0) {
        if (p != 0) {
            ((CFileIoAccessWritePadView*)p)->destroy(1);
        }
        return 0;
    }
    return p;
}

// Initialize the flash-file helper's two-pi cache.
void CFileIoFlash::InitAngleCache() {
    s_flashFileTwoPiCache = kFloat_6_2831855;
}

// Initialize the pad-write helper's two-pi cache.
void CFileIoAccessWritePad::InitAngleCache() {
    s_padWriteTwoPiCache = kFloat_6_2831855;
}

// Initialize the flash-write helper's two-pi cache.
void CFileIoAccessWrite::InitAngleCache() {
    s_flashWriteTwoPiCache = kFloat_6_2831855;
}

extern "C" {

extern void* g_fileIoFlash;
// The base teardown takes the delete flag directly; an explicit ~CFileIo()
// call passes -1 instead of the non-deleting 0.
void __dt__7CFileIoFv(void*, int);
extern "C" void __ct__13CFileIoAccessFv(void*);
extern char __vt__18CFileIoAccessWrite[];

} // extern "C"

// Initialize the access base, write state and empty copied-data buffer.
void* CFileIoAccessWrite::Init() {
    __ct__13CFileIoAccessFv(this);
    state = 0;
    *(void**)this = (void*)__vt__18CFileIoAccessWrite;
    (*(int*)&buffer) = 0;
    return this;
}

extern "C" {

extern char __vt__12CFileIoFlash[];
struct Manager {
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual void f3();
    virtual void f4();
    virtual void f5();
    virtual void f6();
    virtual void f7();
    virtual void f8();
    virtual void f9();
    virtual void f10();
    virtual void f11();
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
        ((CFileIo*)this)->InitMembers();
        *(void**)this = (void*)__vt__12CFileIoFlash;
    }
};

extern "C" void appFreeUni__FPv(void*);
struct SmallString {
    union {
        unsigned int mode;
        struct {
            CW_BITS2(unsigned char big : 1, unsigned char len : 7)
        } b;
    };
    unsigned int size;
    char* heap;
    SmallString() {
        *(volatile unsigned int*)&mode = 0;
        *(volatile unsigned int*)&size = 0;
        *(char* volatile*)&heap = 0;
    }
    ~SmallString() {
        if (mode >> 31)
            appFreeUni__FPv(heap);
    }
};
struct Alloc80277280 {
    volatile unsigned char c;
    char pad[3];
};
extern "C" void String_ReplaceRange(SmallString*, int, unsigned int, const char*, const char*, Alloc80277280*);
extern "C" void appFreeUni__FPv(void*);
extern "C" unsigned int strlen(const char*);
extern "C" char* g_flashFilePathPrefix;
} // extern "C"

// Acquire the existing singleton or create and validate a new flash-file object.
void* CFileIoFlash::AcquireInstance() {
    if (g_fileIoFlash != 0) {
        if ((void*)((CFileIo*)g_fileIoFlash)->AddRef() != 0)
            return g_fileIoFlash;
        return 0;
    }
    CFileIoFlashConstructorView* p = new CFileIoFlashConstructorView();
    g_fileIoFlash = p;
    if (p == 0)
        return 0;
    Manager* mp = (Manager*)p;
    if (mp->is_ready() == 0) {
        if (mp != 0)
            mp->destroy(1);
        return 0;
    }
    return mp;
}

// Acquire the existing singleton or create one using the supplied initialization arguments.
void* CFileIoFlash::CreateInstance(void* a, void* b) {
    if (g_fileIoFlash != 0) {
        if ((void*)((CFileIo*)g_fileIoFlash)->AddRef() != 0)
            return g_fileIoFlash;
        return 0;
    }
    CFileIoFlashConstructorView* p = new CFileIoFlashConstructorView();
    g_fileIoFlash = p;
    if (p == 0)
        return 0;
    Manager* mp = (Manager*)p;
    if (mp->create(a, b) == 0) {
        if (mp != 0)
            mp->destroy(1);
        return 0;
    }
    return mp;
}

#pragma push
#pragma use_lmw_stmw on
// Prefix the requested path and pass the resulting string to common file setup.
int CFileIoFlash::SetPathAndAllocateBuffer(const char* path, void* openOptions) {
    Alloc80277280 x0;
    Alloc80277280 x;
    Alloc80277280 y0;
    Alloc80277280 y;
    SmallString qualifiedPath;
    unsigned int appendPosition = qualifiedPath.b.len;
    x.c = x0.c;
    String_ReplaceRange(&qualifiedPath, 0, appendPosition, g_flashFilePathPrefix,
                        g_flashFilePathPrefix + strlen(g_flashFilePathPrefix), &x);
    appendPosition = !(qualifiedPath.mode >> 31) ? qualifiedPath.b.len : qualifiedPath.size;
    y.c = y0.c;
    String_ReplaceRange(&qualifiedPath, appendPosition, 0, path, path + strlen(path), &y);
    const char* pathText = !(qualifiedPath.mode >> 31) ? (const char*)&qualifiedPath.mode + 1 : qualifiedPath.heap;
    if (((CFileIo*)this)->SetPathAndAllocateBuffer(pathText, (int)openOptions) == 0)
        return 0;
    return 1;
}
#pragma pop

// Publish open status and clear busy when the completion condition is satisfied.
void CFileIoFlash::OnOpenComplete(int status, int completionFlag) {
    if (status == 0 || completionFlag != 0) {
        if (g_fileIoFlash != 0) {
            ((CFileIo*)g_fileIoFlash)->SetStatusCode(status);
            ((CFileIoFlash*)g_fileIoFlash)->ClearBusy();
        }
    }
}

// Clear the flash singleton's busy flag after a qualified-path move.
int CFileIoFlash::OnQualifiedPathMoveComplete() {
    void* p = g_fileIoFlash;
    if (p == 0) {
        return 0;
    }
    ((CFileIoFlash*)p)->ClearBusy();
    return 0;
}

// Publish completion status and clear busy on the live flash singleton.
void CFileIoFlash::CompleteWithStatus(int status) {
    if (g_fileIoFlash != 0) {
        ((CFileIo*)g_fileIoFlash)->SetStatusCode(status);
        ((CFileIoFlash*)g_fileIoFlash)->ClearBusy();
    }
}

// Publish transfer status and completed byte count, then clear busy.
void CFileIoFlash::OnReadWriteComplete(void* status, int transferredBytes) {
    if (g_fileIoFlash != 0) {
        ((CFileIo*)g_fileIoFlash)->SetStatusCode((int)status);
        ((CFileIo*)g_fileIoFlash)->SetTransferredBytes(transferredBytes);
        ((CFileIoFlash*)g_fileIoFlash)->ClearBusy();
    }
}

// Publish status and save-area flags, then clear busy.
void CFileIoFlash::OnSaveAreaCheckComplete(void* status, int saveAreaFlags) {
    if (g_fileIoFlash != 0) {
        ((CFileIo*)g_fileIoFlash)->SetStatusCode((int)status);
        ((CFileIo*)g_fileIoFlash)->SetSaveAreaStatusFlags(saveAreaFlags);
        ((CFileIoFlash*)g_fileIoFlash)->ClearBusy();
    }
}

// Clear the flash singleton, destroy the file-I/O base and optionally delete the object.
void* CFileIoFlash::Destroy(int flag) {
    if (this != 0) {
        g_fileIoFlash = 0;
        __dt__7CFileIoFv(this, 0);
        if (flag > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}

// Report whether the cached file status is nonzero.
int CFileIoFlash::HasNonzeroStatus() {
    return statusCode != 0;
}

// Test for status 0xC361; the meaning of that status is not named here.
int CFileIoFlash::IsStatusC361() {
    return ((CFileIo*)this)->GetStatusCode() == kFlashStatusC361;
}

// Test for status 0xC354; the meaning of that status is not named here.
int CFileIoFlash::IsStatusC354() {
    return ((CFileIo*)this)->GetStatusCode() == kFlashStatusC354;
}

// Test for status 0xC353; the meaning of that status is not named here.
int CFileIoFlash::IsStatusC353() {
    return ((CFileIo*)this)->GetStatusCode() == kFlashStatusC353;
}

// Replace the owned buffer with a copy of the requested data, then initialize base access.
int CFileIoAccessWrite::InitializeBuffer(void* openOptions, void* byteCount, void* sourceData) {
    if (byteCount == 0) {
        return 0;
    }
    if (sourceData == 0) {
        return 0;
    }
    if (buffer != 0) {
        __dl__FPv(buffer);
        buffer = 0;
    }
    buffer = __nwa__FUl(byteCount);
    if (buffer == 0) {
        return 0;
    }
    Memory_Copy(buffer, sourceData, byteCount);
    return ((CFileIoAccess*)this)->CreateResource(openOptions, byteCount);
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623A4", _ctors_u80276AEC_0, CFileIoFlash::InitAngleCache);
STATIC_INIT_ENTRY(".CRT$XCU804623A4", _ctors_u80276AEC_1, CFileIoAccessWritePad::InitAngleCache);
STATIC_INIT_ENTRY(".CRT$XCU804623A4", _ctors_u80276AEC_2, CFileIoAccessWrite::InitAngleCache);
#pragma pop
