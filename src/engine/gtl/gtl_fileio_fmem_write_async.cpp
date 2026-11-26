// The FMEM file device's asynchronous write: starts a NAND write through the shared command
// block and holds the write callback that finishes the request.

#include "classes/GtlFileIoDeviceFMEM.h"
#include "classes/FmemFileState.h"
#include "types.h"

extern "C" {

void File_SetLastError(int x);
void* FileIO_Alloc(u32 size);
void FileIO_Free(void* p);
void NANDSetUserData(void* a, void* b);
int NANDWriteAsync(void* a, void* b, u32 c, void* d, void* e);
void* memcpy(void* dst, const void* src, u32 n);

struct gtl_CCallBackWriteFile {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void vfunc_2();
    void* fileState;          // 0x4: borrowed FMEM file state, must survive completion
    void* completionCallback; // 0x8: completion function address (shared field name retained)
};

extern char __vt__Q23gtl18CCallBackWriteFile[];

volatile u32 s_fmemAsyncOperation[3];
char s_nandAsyncCommandBlock[256];
// Shared async operation words: [0] owned callback, [1] owned staging buffer, [2] byte count.
// The dispatcher forwards [1]/[2] to completion. One block serves all FMEM async requests;
// the caller must serialize submissions until completion has released these allocations.

enum FmemWriteConstants {
    kInvalidFileHandle = -1,
    kInvalidHandleError = 6,
    kWriteSubmitError = 0x1D,
    kWriteCallbackBytes = 0xC,
    kOperationPending = 0,
    kOperationFailed = 2,
    kCallbackWord = 0,
    kStagingBufferWord = 1,
    kByteCountWord = 2
};

#pragma use_lmw_stmw on

} // extern "C"

// Queue a NAND write from a private copy; return 1 if accepted, 0 on invalid handle/submit failure.
// srcBuf is borrowed only during this call. device is the open FMEM file state and must outlive
// completion, which calls completionCallback(error, bytesWritten, device, device->callbackArgument).
// unusedArgument is ignored. The existing path assumes its allocations succeed.
int gtl::CFileIoDeviceFMEM::WriteAsync(void* device, void* srcBuf, u32 size, int unusedArgument,
                                       int completionCallback) {
    (void)unusedArgument;
    if ((u32)device == (u32)kInvalidFileHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    // Phase 1: mark pending and attach the file/callback to a newly owned completion object.
    (*(u32*)&AsFmemFileState(device)->operationState) = kOperationPending;
    gtl_CCallBackWriteFile* callback = (gtl_CCallBackWriteFile*)FileIO_Alloc(kWriteCallbackBytes);
    if (callback != 0) {
        *(void**)callback = (void*)__vt__Q23gtl18CCallBackWriteFile;
        callback->fileState = device;
        callback->completionCallback = (void*)(u32)completionCallback;
    }
    // Phase 2: copy all source bytes before submission so the caller may reuse srcBuf on return.
    s_fmemAsyncOperation[kCallbackWord] = (u32)callback;
    s_fmemAsyncOperation[kByteCountWord] = size;
    s_fmemAsyncOperation[kStagingBufferWord] = (u32)FileIO_Alloc(s_fmemAsyncOperation[kByteCountWord]);
    memcpy((void*)s_fmemAsyncOperation[kStagingBufferWord], srcBuf, s_fmemAsyncOperation[kByteCountWord]);
    NANDSetUserData(s_nandAsyncCommandBlock, (void*)s_fmemAsyncOperation);
    // Phase 3: transfer cleanup responsibility to completion only if NAND accepts the request.
    int nandResult =
        NANDWriteAsync(device, (void*)s_fmemAsyncOperation[kStagingBufferWord], s_fmemAsyncOperation[kByteCountWord],
                       (void*)gtl::CFileIoDeviceFMEM::DispatchNandCallback, s_nandAsyncCommandBlock);
    if (nandResult != 0) {
        File_SetLastError(kWriteSubmitError);
        FileIO_Free((void*)s_fmemAsyncOperation[kStagingBufferWord]);
        s_fmemAsyncOperation[kStagingBufferWord] = 0;
        callback->vfunc_1();
        (*(u32*)&AsFmemFileState(device)->operationState) = kOperationFailed;
        return 0;
    }
    File_SetLastError(0);
    return 1;
}

#pragma use_lmw_stmw off
