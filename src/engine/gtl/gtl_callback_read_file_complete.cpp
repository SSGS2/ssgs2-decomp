// Completion of a NAND file read (gtl::CCallBackReadFile::CompleteRead): it publishes the result,
// notifies the user callback and releases the read callback wrapper.

#include "types.h"
#include <decomp/local_class.h>
#include "classes/FmemFileState.h"
#include "classes/gtl_CCallBackReadFile.h"

extern "C" {
void File_SetLastError(int x);
void* FileIO_InvokeAllocatorCallback3(u32 allocationAddress);

// Virtual slot 0 of a callback object (the destructor-like entry), called through a raw pointer.
struct CallbackVirtualView {
    virtual void teardown(int) DECOMP_LOCAL_VIRTUAL;
};

enum ReadCompletionState {
    kReadCompleted = 1,
    kReadFailed = 2
};
enum {
    kFileSuccess = 0,
    kNondeletingDestructor = -1
};
} // extern "C"

// Publish a NAND read result, notify the user and consume this callback wrapper.
// result is a nonnegative byte count or a negative error. Before notification,
// failure leaves operationResult/last error untouched and reports zero bytes.
// self and self->fileState must be nonnull; fileState is borrowed, self is released here. The return
// value belongs to allocator callback 3, not to the read or the user callback.
void* gtl::CCallBackReadFile::CompleteRead(int result) {
    int callbackError = 0;
    int callbackByteCount = 0;
    // Phase 1: make the file-state result visible before invoking user code.
    if (result >= 0) {
        File_SetLastError(kFileSuccess);
        AsFmemFileState(fileState)->operationState = kReadCompleted;
        AsFmemFileState(fileState)->operationResult = result;
        callbackByteCount = result;
    } else {
        AsFmemFileState(fileState)->operationState = kReadFailed;
        callbackError = result;
    }
    // Phase 2: the optional completion observes state 1/2 and the stored user word.
    if (completionCallback) {
        typedef void (*ReadCompletion)(int, int, void*, int);
        ((ReadCompletion)completionCallback)(callbackError, callbackByteCount, fileState,
                                             AsFmemFileState(fileState)->callbackArgument);
    }
    // Phase 3: nondeleting virtual cleanup, then explicit release through slot 3.
    // The user completion must leave self alive for this cleanup.
    ((CallbackVirtualView*)this)->teardown(kNondeletingDestructor);
    return FileIO_InvokeAllocatorCallback3((u32)this);
}
