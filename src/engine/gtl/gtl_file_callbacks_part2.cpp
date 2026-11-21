// Translation unit 0x8041F1CC-0x8041F2E8 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "classes/FmemFileState.h"

extern "C" {
void FileIO_Free(void *p);
void *FileIO_InvokeAllocatorCallback3(u32 allocationWord); // Here, callback 3 releases this pointer word.
void File_SetLastError(int x);

struct gtl_CCallBackReadFile {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void vfunc_2();
};

// Optionally free a read-callback wrapper; flags > 0 deletes storage, otherwise return it unchanged.
void *gtl_CCallBackReadFile_dtor(gtl_CCallBackReadFile *self, int flags) {
    if (self) {
        if (flags > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

struct gtl_CCallBackWriteFile {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void vfunc_2();
    void *fileState;   // 0x4
    void *completionCallback;    // 0x8
};

enum WriteCompletionState { kWriteCompleted = 1, kWriteFailed = 2 };

// Publish a write result, notify the caller, then release completion storage and this wrapper.
// result is a nonnegative byte count or a negative error; completionStorage may be null.
// self->fileState is borrowed. cb receives (error, byteCount, fileState, fileState->callbackArgument).
// On failure the previous operationResult and last error are retained; return is the release hook's value.
void *gtl_CCallBackWriteFile_OnWriteComplete(gtl_CCallBackWriteFile *self, int result, void *completionStorage) {
    int callbackError = 0;
    int callbackByteCount = 0;
    // Publish completion before invoking user code; failures report zero callback bytes.
    if (result >= 0) {
        File_SetLastError(0);
        AsFmemFileState(self->fileState)->operationState = kWriteCompleted;
        AsFmemFileState(self->fileState)->operationResult = result;
        callbackByteCount = result;
    } else {
        AsFmemFileState(self->fileState)->operationState = kWriteFailed;
        callbackError = result;
    }
    if (self->completionCallback) {
        typedef void (*Cb)(int, int, void *, int);
        ((Cb)self->completionCallback)(callbackError, callbackByteCount, self->fileState, AsFmemFileState(self->fileState)->callbackArgument);
    }
    // The negative destructor flag preserves wrapper storage for callback 3 to release.
    if (completionStorage) {
        FileIO_InvokeAllocatorCallback3((u32)completionStorage);
    }
    self->vfunc_0(-1);
    return FileIO_InvokeAllocatorCallback3((u32)self);
}

} // extern "C"
