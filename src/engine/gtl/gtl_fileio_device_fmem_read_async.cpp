#include "types.h"
#include "classes/FmemFileState.h"

extern "C" {
void File_SetLastError(int x);
void* FileIO_Alloc(u32 size);
void NANDSetUserData(void*, void*);
int NANDReadAsync(void* a, u32 b, u32 c, void* d, void* e);
void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void);

extern char __vt__Q23gtl17CCallBackReadFile[];

u32 s_fmemAsyncOperation[3];
char s_nandAsyncCommandBlock[256];

struct gtl_CCallBackReadFile {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void vfunc_2();
    virtual void vfunc_3();
    void* fileState;   // 0x4
    void* completionCallback;    // 0x8
};

#pragma use_lmw_stmw on

enum {
    kInvalidFileHandle = 0xFFFFFFFFu,
    kFileErrorInvalidHandle = 6,
    kFileErrorRead = 0x1e,
    kFileErrorNone = 0,
    kOperationPending = 0,
    kOperationFailed = 2,
    kReadCallbackBytes = 0xc
};

// Submit a NAND read into caller-owned storage; return 1 if queued, 0 on immediate failure.
// device is an open NAND file state; destinationAddress/size are an address and byte count.
// Both must remain valid through completion. cbArg is the optional user callback address,
// not its context (the latter comes from FmemFileState::callbackArgument). p4/p5 are ignored.
// Uses the shared command block: the caller must serialize operations that reuse it.
int gtl_CFileIoDeviceFMEM_ReadAsync(void* self, void* device, u32 destinationAddress, u32 size, int p4, int p5, void* cbArg) {
    gtl_CCallBackReadFile* cb;
    (void)self;
    (void)p4;
    (void)p5;
    if ((u32)device == kInvalidFileHandle) {
        File_SetLastError(kFileErrorInvalidHandle);
        return 0;
    }
    // Build the completion object; its lifetime transfers to the NAND dispatcher on acceptance.
    // Allocation failure is not handled separately by the existing implementation.
    AsFmemFileState(device)->operationState = kOperationPending;
    cb = (gtl_CCallBackReadFile*)FileIO_Alloc(kReadCallbackBytes);
    if (cb != 0) {
        *(void**)cb = __vt__Q23gtl17CCallBackReadFile;
        cb->fileState = device;
        cb->completionCallback = cbArg;
    }
    // Dispatcher payload: callback object, optional argument 0, optional argument 1.
    s_fmemAsyncOperation[0] = (u32)cb;
    s_fmemAsyncOperation[1] = 0;
    s_fmemAsyncOperation[2] = 0;
    NANDSetUserData(s_nandAsyncCommandBlock, &s_fmemAsyncOperation);
    if (NANDReadAsync(device, destinationAddress, size, (void*)gtl_CFileIoDeviceFMEM_DispatchNandCallback, s_nandAsyncCommandBlock) != 0) {
        // Rejected submission: release the completion object and mark the file failed.
        File_SetLastError(kFileErrorRead);
        {
            void* vt = *(void**)cb;
            ((void (**)(void*))vt)[3](cb);
        }
        AsFmemFileState(device)->operationState = kOperationFailed;
        return 0;
    }
    File_SetLastError(kFileErrorNone);
    return 1;
}

#pragma use_lmw_stmw off
} // extern "C"
