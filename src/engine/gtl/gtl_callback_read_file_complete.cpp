// Translation unit 0x8041F2E8-0x8041F3A8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "classes/FmemFileState.h"

extern "C" {
void File_SetLastError(int x);
void *FileIO_InvokeAllocatorCallback3(u32 allocationAddress);

struct gtl_CCallBackReadFile {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void vfunc_2();
    void *fileState;   // 0x4: borrowed FmemFileState, valid through user completion
    void *completionCallback;    // 0x8: optional completion(error, byteCount, fileState, userArgument)
};

enum ReadCompletionState { kReadCompleted = 1, kReadFailed = 2 };
enum { kFileSuccess = 0, kNondeletingDestructor = -1 };

// Publish a NAND read result, notify the user and consume this callback wrapper.
// result is a nonnegative byte count or a negative error. Before notification,
// failure leaves operationResult/last error untouched and reports zero bytes.
// self and self->fileState must be nonnull; fileState is borrowed, self is released here. The return
// value belongs to allocator callback 3, not to the read or the user callback.
void *gtl_CCallBackReadFile_CompleteRead(gtl_CCallBackReadFile *self, int result) {
    int callbackError = 0;
    int callbackByteCount = 0;
    // Phase 1: make the file-state result visible before invoking user code.
    if (result >= 0) {
        File_SetLastError(kFileSuccess);
        AsFmemFileState(self->fileState)->operationState = kReadCompleted;
        AsFmemFileState(self->fileState)->operationResult = result;
        callbackByteCount = result;
    } else {
        AsFmemFileState(self->fileState)->operationState = kReadFailed;
        callbackError = result;
    }
    // Phase 2: the optional completion observes state 1/2 and the stored user word.
    if (self->completionCallback) {
        typedef void (*ReadCompletion)(int, int, void *, int);
        ((ReadCompletion)self->completionCallback)(callbackError, callbackByteCount, self->fileState, AsFmemFileState(self->fileState)->callbackArgument);
    }
    // Phase 3: nondeleting virtual cleanup, then explicit release through slot 3.
    // The user completion must leave self alive for this cleanup.
    self->vfunc_0(kNondeletingDestructor);
    return FileIO_InvokeAllocatorCallback3((u32)self);
}

} // extern "C"
