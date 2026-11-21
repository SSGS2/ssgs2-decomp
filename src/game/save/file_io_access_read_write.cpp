#include <decomp/static_init.h>
// Translation unit 0x8027624C-0x80276AEC (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/CFileIoAccess.h"
#include "classes/CFileIoAccessRead.h"
#include "classes/CFileIoAccessReadPad.h"
#include "classes/CFileIoAccessWrite.h"

#define AsCFileIoAccess(p) ((CFileIoAccess*)(p))

#define AsCFileIoAccessRead(p) ((CFileIoAccessRead*)(p))

#define AsCFileIoAccessWrite(p) ((CFileIoAccessWrite*)(p))

// Read and write accessors share this asynchronous open/transfer/close sequence.
// Update() always returns 1: it is a scheduler acknowledgement, not completion.
// Poll state == 4 separately; state 3 latches failure on the NEXT update. A null
// resource enters state 3 and therefore reaches state 4 in that same update.
// Resource references belong to the accessor and are released by destroy(). The
// write accessor also owns its buffer; the padded reader borrows its pad buffer.
enum FileAccessState { kAccessOpen = 0, kAccessWaitOpen = 1, kAccessWaitTransfer = 2,
    kAccessFailed = 3, kAccessFinished = 4 };
enum { kFileOpenForWrite = 2, kFileOpenForRead = 3 };

extern "C" {
extern const float kFloat_6_2831855;
extern float s_twoPi_FileIoAccess;
extern float s_twoPi_FileIoAccessRead;
extern float s_twoPi_FileIoAccessWrite;
extern float s_twoPi_FileIoAccessReadPad;
extern void* CFileIoPad_CreateInstance(void*, const char*, int);
extern int CFileIo_GetBuffer(void*);
// Underlying CFileIo save-area status flags.
extern int CFileIo_GetSaveAreaStatusFlags(void*);
extern int CFileIo_GetTransferredBytes(void*);
extern char __vt__13CFileIoAccess[];
extern char __vt__17CFileIoAccessRead[];
extern int CFileIo_ReleaseRef(void*);
// Underlying CFileIo busy byte; do not advance a pending operation.
extern int CFileIo_IsBusy(void*);
extern int CFileIo_CloseHandle(void*);
extern char __vt__20CFileIoAccessReadPad[];
extern char __vt__18CFileIoAccessWrite[];
extern void __dl__FPv(void*);
// Initialize the base file-access angle constant.
void FileIoAccess_InitializeAngleConstant() {
    s_twoPi_FileIoAccess = kFloat_6_2831855;
}

// Return the resource's borrowed buffer address as an ABI word, or zero without a resource.
int CFileIoAccess_GetBuffer(void* p) {
    void* v = ((CFileIoAccess*)p)->resource;
    if (v)
        return CFileIo_GetBuffer(v);
    return 0;
}

// Forward save-area status bits, or zero without a resource; callers test masks 5 and 10.
int CFileIoAccess_GetStorageFullFlags(void* p) {
    void* v = ((CFileIoAccess*)p)->resource;
    if (v)
        return CFileIo_GetSaveAreaStatusFlags(v);
    return 0;
}

// Return the backing transfer's byte count, or zero without a resource.
int CFileIoAccess_GetTransferredBytes(void* p) {
    void* v = ((CFileIoAccess*)p)->resource;
    if (v)
        return CFileIo_GetTransferredBytes(v);
    return 0;
}

#pragma dont_inline on
void CFileIoAccess_Ctor(void* p);
#pragma dont_inline off

struct Vt802764C0 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual int v6();
};
// Create the backing resource through virtual slot 6 and retain it on success.
// The original passes two arguments (path/options, size) to slot 6 in r4/r5.
int CFileIoAccess_CreateResource(void* p) {
    int r = ((Vt802764C0*)p)->v6();
    (*(int*)&AsCFileIoAccessRead(p)->resource) = r;
    return r != 0;
}

#pragma dont_inline on
// Construct a reader with no resource, no latched failure, and an open request pending.
void* CFileIoAccessRead_Ctor(void* p) {
    CFileIoAccess_Ctor(p);
    AsCFileIoAccessRead(p)->state = kAccessOpen;
    *(void**)p = (void*)__vt__17CFileIoAccessRead;
    return p;
}
#pragma dont_inline off

// Return only the local failure latch, without querying the backing resource.
unsigned char CFileIoAccess_GetFlag(void* p) {
    return AsCFileIoAccess(p)->failureLatched;
}

#pragma dont_inline on
// Return the borrowed backing-resource address as a word; ownership stays with the accessor.
int CFileIoAccess_GetResource(void* p) {
    return (*(int*)&((CFileIoAccess*)p)->resource);
}
#pragma dont_inline off

#pragma dont_inline on
// Store the low byte in the failure latch but return the original full-width value.
int CFileIoAccess_SetFailureFlag(char* p, int v) {
    AsCFileIoAccess(p)->failureLatched = (char)v; // 0x80276560: +0x8
    return v;
}
#pragma dont_inline off

// Initialize the read-access angle constant.
void FileIoAccessRead_InitializeAngleConstant() {
    s_twoPi_FileIoAccessRead = kFloat_6_2831855;
}

// Report completion of the read-access state machine.
int Object_IsStateFour(void* p) {
    int v = AsCFileIoAccessRead(p)->state;
    return v == kAccessFinished;
}

// Read access supports this capability unconditionally.
int CFileIoAccessRead_vfunc_2() {
    return 1;
}

// Initialize the padded-read angle constant.
void FileIoAccessReadPad_InitializeAngleConstant() {
    s_twoPi_FileIoAccessReadPad = kFloat_6_2831855;
}

} // extern "C"

// Create a file resource using the reader's borrowed pad buffer and the given path/mode.
extern "C" void* createPad__20CFileIoAccessReadPadFv(CFileIoAccessReadPad* self, const char* path, int mode) {
    return CFileIoPad_CreateInstance(self->buffer, path, mode);
}

extern "C" {

// Initialize the write-access angle constant.
void FileIoAccessWrite_InitializeAngleConstant() {
    s_twoPi_FileIoAccessWrite = kFloat_6_2831855;
}

// Report terminal state, including failed operations after their latch update.
int CFileIoAccessWrite_IsFinished(void* p) {
    int v = AsCFileIoAccessWrite(p)->state;
    return v == kAccessFinished;
}

// Write access supports this capability unconditionally.
int CFileIoAccessWrite_vfunc_2() {
    return 1;
}

struct Vt80276358 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual int v9();
};

// Query the backing resource's slot-9 readiness predicate, treating null as false.
int CFileIoAccess_QueryResourceStatusSlot9(void* self) {
    int isReady = 0;
    void* resource = ((CFileIoAccess*)self)->resource;
    if (resource) {
        if (((Vt80276358*)resource)->v9() != 0)
            isReady = 1;
    }
    return isReady;
}

struct Vt802763C4 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual int v11();
};

// Query the backing resource's slot-11 readiness predicate, treating null as false.
int CFileIoAccess_QueryResourcePredicate11(void* self) {
    int isReady = 0;
    void* resource = ((CFileIoAccess*)self)->resource;
    if (resource) {
        if (((Vt802763C4*)resource)->v11() != 0)
            isReady = 1;
    }
    return isReady;
}

struct Vt80276508 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual int v6(int, int);
};

// Create the backing resource with default arguments through virtual slot 6.
int CFileIoAccess_CreateDefaultResource(void* p) {
    int r = ((Vt80276508*)p)->v6(0, 0);
    (*(int*)&AsCFileIoAccessRead(p)->resource) = r;
    return r != 0;
}

struct Vt80276450 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual int v8();
};

// Query the backing resource's slot-8 readiness predicate, treating null as false.
int CFileIoAccess_QueryResourcePredicate8(void* self) {
    int isReady = 0;
    void* resource = ((CFileIoAccess*)self)->resource;
    if (resource) {
        if (((Vt80276450*)resource)->v8() != 0)
            isReady = 1;
    }
    return isReady;
}

} // extern "C"

#pragma dont_inline on
// Release the reader's resource reference; positive flag also frees this accessor.
void* CFileIoAccessRead::destroy(int flag) {
    if (this) {
        ((CFileIoAccess*)this)->destroy(0);
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}
#pragma dont_inline off

// Release the reader base without freeing its borrowed pad buffer; optionally free self.
void* CFileIoAccessReadPad::destroy(int flag) {
    if (this) {
        ((CFileIoAccessRead*)this)->destroy(0);
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}

#pragma dont_inline on
// Drop the retained resource reference, clear it, and optionally free this accessor.
void* CFileIoAccess::destroy(int flag) {
    if (this != 0) {
        void* res = resource;
        vtbl = (void*)__vt__13CFileIoAccess;
        if (res != 0) {
            CFileIo_ReleaseRef(res);
            resource = 0;
        }
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}
#pragma dont_inline off

// Free the owned write buffer, release the backing resource, and optionally free self.
void* CFileIoAccessWrite::destroy(int flag) {
    if (this != 0) {
        void* buf = buffer;
        vtbl = (void*)__vt__18CFileIoAccessWrite;
        if (buf != 0) {
            __dl__FPv(buf);
            buffer = 0;
        }
        ((CFileIoAccess*)this)->destroy(0);
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}

extern "C" {

struct Vt802762D0 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual int v7();
};

// Report a resource slot-7 error or a local failure latch; this does not test completion.
int CFileIoAccess_HasFailure(void* p) {
    int flag = 1;
    int found = 0;
    void* self = p;
    void* v = AsCFileIoAccess(self)->resource; // 0x802762D0: +0x4
    if (v) {
        if (((Vt802762D0*)v)->v7() != 0)
            found = 1;
    }
    if (found == 0) {
        if (AsCFileIoAccess(self)->failureLatched == 0)
            flag = 0;
    }
    return flag;
}

struct Vt804B46E8 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void m3(int);
    virtual int v4(void*, void*);
};

struct PaddedReadAccessStorage {
    void* vtable;
    int f4;
    unsigned char f8;
    char pad[3];
    int fC;
    void* f10;
    // Keep the caller's pad buffer as a borrowed member after constructing the reader.
    PaddedReadAccessStorage(void* a) {
        CFileIoAccessRead_Ctor(this);
        vtable = (void*)__vt__20CFileIoAccessReadPad;
        f10 = a;
    }
};

// Allocate a read accessor for a caller-supplied pad buffer; destroy it if opening fails.
void* CFileIoAccessReadPad_Create(void* buffer, void* path, void* mode) {
    void* padBuffer = buffer;
    void* filePath = path;
    void* openMode = mode;
    PaddedReadAccessStorage* access = new PaddedReadAccessStorage(padBuffer);
    if (access == 0) {
        return 0;
    }
    if (!((Vt804B46E8*)access)->v4(filePath, openMode)) {
        if (access) ((Vt804B46E8*)access)->m3(1);
        return 0;
    }
    return access;
}

struct FileReadResourceInterface {
    virtual int open(int);
    virtual int startRead();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual int hasError();
};

// Advance open, read, and close polling; latch failure before entering the finished state.
// A successful close request immediately selects finished; it is not polled again here.
int CFileIoAccessRead_Update(void* access) {
    void* self = access;
    void* resource = (void*)CFileIoAccess_GetResource(self);
    if (resource == 0) {
        AsCFileIoAccessRead(self)->state = kAccessFailed;
    }
    int state = AsCFileIoAccessRead(self)->state;
    switch (state) {
        case kAccessOpen:
            if (((FileReadResourceInterface*)resource)->open(kFileOpenForRead) == 0) {
                AsCFileIoAccessRead(self)->state = kAccessFailed;
            } else {
                AsCFileIoAccessRead(self)->state = kAccessWaitOpen;
            }
            break;
        case kAccessWaitOpen:
            // The completed open must be error-free before the transfer is submitted.
            if (CFileIo_IsBusy(resource) == 0) {
                if (((FileReadResourceInterface*)resource)->hasError() != 0) {
                    AsCFileIoAccessRead(self)->state = kAccessFailed;
                } else {
                    if (((FileReadResourceInterface*)resource)->startRead() == 0) {
                        AsCFileIoAccessRead(self)->state = kAccessFailed;
                    } else {
                        AsCFileIoAccessRead(self)->state = kAccessWaitTransfer;
                    }
                }
            }
            break;
        case kAccessWaitTransfer:
            // Wait for transfer activity to cease, then submit close.
            if (CFileIo_IsBusy(resource) == 0) {
                if (CFileIo_CloseHandle(resource) == 0) {
                    AsCFileIoAccessRead(self)->state = kAccessFailed;
                } else {
                    AsCFileIoAccessRead(self)->state = kAccessFinished;
                }
            }
            break;
        case kAccessFailed:
            CFileIoAccess_SetFailureFlag((char*)self, 1);
            AsCFileIoAccessRead(self)->state = kAccessFinished;
            break;
    }
    return 1;
}

struct FileWriteResourceInterface {
    virtual int open(int);
    virtual int m1();
    virtual int startWrite(int);
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual int hasError();
};

// Advance open, write, and close polling; latch failure before entering the finished state.
// The buffer remains accessor-owned through close; callers observe state/latch separately.
int CFileIoAccessWrite_Update(void* access) {
    void* self = access;
    void* resource = (void*)CFileIoAccess_GetResource(self);
    if (resource == 0) {
        AsCFileIoAccessWrite(self)->state = kAccessFailed;
    }
    int state = AsCFileIoAccessWrite(self)->state;
    switch (state) {
        case kAccessOpen:
            if (((FileWriteResourceInterface*)resource)->open(kFileOpenForWrite) == 0) {
                AsCFileIoAccessWrite(self)->state = kAccessFailed;
            } else {
                AsCFileIoAccessWrite(self)->state = kAccessWaitOpen;
            }
            break;
        case kAccessWaitOpen:
            if (CFileIo_IsBusy(resource) == 0) {
                if (((FileWriteResourceInterface*)resource)->hasError() != 0) {
                    AsCFileIoAccessWrite(self)->state = kAccessFailed;
                } else {
                    int writeBuffer = (*(int*)&AsCFileIoAccessWrite(self)->buffer);
                    if (((FileWriteResourceInterface*)resource)->startWrite(writeBuffer) == 0) {
                        AsCFileIoAccessWrite(self)->state = kAccessFailed;
                    } else {
                        AsCFileIoAccessWrite(self)->state = kAccessWaitTransfer;
                    }
                }
            }
            break;
        case kAccessWaitTransfer:
            if (CFileIo_IsBusy(resource) == 0) {
                if (CFileIo_CloseHandle(resource) == 0) {
                    AsCFileIoAccessWrite(self)->state = kAccessFailed;
                } else {
                    AsCFileIoAccessWrite(self)->state = kAccessFinished;
                }
            }
            break;
        case kAccessFailed:
            CFileIoAccess_SetFailureFlag((char*)self, 1);
            AsCFileIoAccessWrite(self)->state = kAccessFinished;
            break;
    }
    return 1;
}

#pragma dont_inline on
// Initialize the base accessor without creating a file resource.
void CFileIoAccess_Ctor(void* p) {
    (*(int*)&AsCFileIoAccess(p)->resource) = 0;
    *(void**)p = (void*)__vt__13CFileIoAccess;
    AsCFileIoAccess(p)->failureLatched = 0;
}
#pragma dont_inline off
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462394", _ctors_u8027624C_0, FileIoAccess_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462394", _ctors_u8027624C_1, FileIoAccessRead_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462394", _ctors_u8027624C_2, FileIoAccessReadPad_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80462394", _ctors_u8027624C_3, FileIoAccessWrite_InitializeAngleConstant);
#pragma pop
