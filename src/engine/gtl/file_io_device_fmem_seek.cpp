#include "classes/FmemFileState.h"
#include "types.h"

extern "C" {

void File_SetLastError(int x);
void* FileIO_Alloc(u32 size);
void NANDSetUserData(void* a, void* b);
int NANDSeekAsync(void* a, u32 b, int c, void* d, void* e);
void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void);

extern char __vt__Q23gtl23CCallBackSetFilePointer[];

struct gtl_CCallBackSetFilePointer {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void vfunc_2();
    void* fileState;   // 0x4: borrowed open-file state until completion.
    void* completionCallback;    // 0x8: callback address, carried as a raw word.
};

// Shared single-operation context: callback object, unused buffer, unused size.
// Other FMEM operations reuse these globals: callers must serialize requests.
volatile u32 s_fmemAsyncOperation[3];
char s_nandAsyncCommandBlock[256];

enum SeekContract {
    kInvalidFileWord = 0xFFFFFFFFu, kInvalidHandleError = 6,
    kSeekSubmissionError = 0x19, kSeekFromStart = 0,
    kSeekFromCurrent = 1, kSeekFromEnd = 2, kOperationPending = 0,
    kOperationFailed = 2, kSeekCallbackBytes = 0xC
};

#pragma use_lmw_stmw on

// Submit a NAND seek; return 1 if accepted, 0 for an invalid handle or submission failure.
// fileHandle is borrowed until completion; byteOffset is passed as a raw 32-bit
// displacement. origin: 1=current, 2=end, everything else=start. self/unused are ignored.
// Completion owns the allocated callback after acceptance; rejection invokes its cleanup slot.
// Allocation failure is not handled here: the existing rejection path dereferences null.
int gtl_CFileIoDeviceFMEM_SeekAsync(void* self, void* fileHandle, u32 byteOffset, u32 origin, int unused, int callbackAddress) {
    (void)self;
    (void)unused;
    if ((u32)fileHandle == kInvalidFileWord) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    // Phase 1: mark pending, translate the origin, and create completion context.
    *(u32*)&AsFmemFileState(fileHandle)->operationState = kOperationPending;
    gtl_CCallBackSetFilePointer* completion;
    int nandOrigin = kSeekFromStart;
    if (origin == kSeekFromCurrent) {
        nandOrigin = kSeekFromCurrent;
    } else if (origin == kSeekFromEnd) {
        nandOrigin = kSeekFromEnd;
    }
    completion = (gtl_CCallBackSetFilePointer*)FileIO_Alloc(kSeekCallbackBytes);
    if (completion != 0) {
        *(void**)completion = (void*)__vt__Q23gtl23CCallBackSetFilePointer;
        completion->fileState = fileHandle;
        completion->completionCallback = (void*)(u32)callbackAddress;
    }
    // Phase 2: publish the callback before submitting the shared command block.
    s_fmemAsyncOperation[0] = (u32)completion;
    s_fmemAsyncOperation[1] = 0;
    s_fmemAsyncOperation[2] = 0;
    NANDSetUserData(s_nandAsyncCommandBlock, (void*)s_fmemAsyncOperation);
    int submissionStatus = NANDSeekAsync(fileHandle, byteOffset, nandOrigin, (void*)gtl_CFileIoDeviceFMEM_DispatchNandCallback, s_nandAsyncCommandBlock);
    // Phase 3: rejected requests clean up synchronously; accepted ones finish via NAND callback.
    if (submissionStatus != 0) {
        File_SetLastError(kSeekSubmissionError);
        completion->vfunc_1();
        *(u32*)&AsFmemFileState(fileHandle)->operationState = kOperationFailed;
        return 0;
    }
    File_SetLastError(0);
    return 1;
}

#pragma use_lmw_stmw off

} // extern "C"
