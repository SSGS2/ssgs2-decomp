// Translation unit 0x8041F0CC-0x8041F1CC (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "classes/FmemFileState.h"

extern "C" {
void FileIO_Free(void *p);
void *FileIO_InvokeAllocatorCallback3(u32 allocationAddress);
void File_SetLastError(int x);

struct gtl_CCallBackWriteFile {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void vfunc_2();
};

// Destruct the write callback; positive flags also free its storage. Return the original address.
void *gtl_CCallBackWriteFile_dtor(gtl_CCallBackWriteFile *self, int flags) {
    if (self) {
        if (flags > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

struct gtl_CCallBackSetFilePointer {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void vfunc_2();
    void *fileState;   // 0x4
    void *completionCallback;    // 0x8
};

enum { kFileErrorNone = 0, kOperationComplete = 1, kOperationFailed = 2 };

// Record async seek completion, notify the optional user callback, then release this callback.
// arg >= 0 is the resulting file position in bytes; negative arg is the NAND error.
// User callback arguments: (negative error or 0, position or 0, borrowed file state, user context).
// On failure operationResult is left unchanged and last-error is not set here.
// The return value is forwarded from the final allocator callback, not the seek result.
void *gtl_CCallBackSetFilePointer_Complete(gtl_CCallBackSetFilePointer *self, int arg) {
    int completionError = 0;
    int completedPosition = 0;
    if (arg >= 0) {
        File_SetLastError(kFileErrorNone);
        AsFmemFileState(self->fileState)->operationState = kOperationComplete;
        AsFmemFileState(self->fileState)->operationResult = arg;
        completedPosition = arg;
    } else {
        AsFmemFileState(self->fileState)->operationState = kOperationFailed;
        completionError = arg;
    }
    if (self->completionCallback) {
        typedef void (*CompletionCallback)(int, int, void *, int);
        ((CompletionCallback)self->completionCallback)(completionError, completedPosition, self->fileState, AsFmemFileState(self->fileState)->callbackArgument);
    }
    // Destruct without the positive delete flag, then release through the file-I/O hook.
    self->vfunc_0(-1);
    return FileIO_InvokeAllocatorCallback3((u32)self);
}

} // extern "C"
