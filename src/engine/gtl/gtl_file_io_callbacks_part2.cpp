// GTL file I/O callbacks for writes and file-pointer seeks.
//
// Each callback records its result, notifies the optional user callback, and then releases itself.

#include "types.h"
#include <decomp/local_class.h>
#include "classes/FmemFileState.h"
#include "classes/gtl_CCallBackWriteFile.h"
#include "classes/gtl_CCallBackSetFilePointer.h"

extern "C" {
void FileIO_Free(void* p);
void* FileIO_InvokeAllocatorCallback3(u32 allocationAddress);
void File_SetLastError(int x);

// Virtual slot 0 of a callback object (the destructor-like entry), called through a raw pointer.
struct CallbackVirtualView {
    virtual void teardown(int) DECOMP_LOCAL_VIRTUAL;
};

enum {
    kFileErrorNone = 0,
    kOperationComplete = 1,
    kOperationFailed = 2
};
} // extern "C"

// Record async seek completion, notify the optional user callback, then release this callback.
// arg >= 0 is the resulting file position in bytes; negative arg is the NAND error.
// User callback arguments: (negative error or 0, position or 0, borrowed file state, user context).
// On failure operationResult is left unchanged and last-error is not set here.
// The return value is forwarded from the final allocator callback, not the seek result.
void* gtl::CCallBackSetFilePointer::Complete(int arg) {
    int completionError = 0;
    int completedPosition = 0;
    if (arg >= 0) {
        File_SetLastError(kFileErrorNone);
        AsFmemFileState(fileState)->operationState = kOperationComplete;
        AsFmemFileState(fileState)->operationResult = arg;
        completedPosition = arg;
    } else {
        AsFmemFileState(fileState)->operationState = kOperationFailed;
        completionError = arg;
    }
    if (completionCallback) {
        typedef void (*CompletionCallback)(int, int, void*, int);
        ((CompletionCallback)completionCallback)(completionError, completedPosition, fileState,
                                                 AsFmemFileState(fileState)->callbackArgument);
    }
    // Destruct without the positive delete flag, then release through the file-I/O hook.
    ((CallbackVirtualView*)this)->teardown(-1);
    return FileIO_InvokeAllocatorCallback3((u32)this);
}

// Destruct the write callback; positive flags also free its storage. Return the original address.
void* gtl::CCallBackWriteFile::Destroy(int flags) {
    if (this) {
        if (flags > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}
