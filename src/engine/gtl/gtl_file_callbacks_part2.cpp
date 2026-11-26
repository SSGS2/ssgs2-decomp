// Callback objects for file reads and writes (gtl::CCallBackReadFile and
// gtl::CCallBackWriteFile): their destroy and completion steps, which keep the last result.

#include "types.h"
#include <decomp/local_class.h>
#include "classes/FmemFileState.h"
#include "classes/gtl_CCallBackReadFile.h"
#include "classes/gtl_CCallBackWriteFile.h"

extern "C" {
void FileIO_Free(void* p);
void* FileIO_InvokeAllocatorCallback3(u32 allocationWord); // Here, callback 3 releases this pointer word.
void File_SetLastError(int x);

// Virtual slot 0 of a callback object (the destructor-like entry), called through a raw pointer.
struct CallbackVirtualView {
    virtual void teardown(int) DECOMP_LOCAL_VIRTUAL;
};

enum WriteCompletionState {
    kWriteCompleted = 1,
    kWriteFailed = 2
};
} // extern "C"

// Publish a write result, notify the caller, then release completion storage and this wrapper.
// result is a nonnegative byte count or a negative error; completionStorage may be null.
// self->fileState is borrowed. cb receives (error, byteCount, fileState, fileState->callbackArgument).
// On failure the previous operationResult and last error are retained; return is the release hook's value.
void* gtl::CCallBackWriteFile::OnWriteComplete(int result, void* completionStorage) {
    int callbackError = 0;
    int callbackByteCount = 0;
    // Publish completion before invoking user code; failures report zero callback bytes.
    if (result >= 0) {
        File_SetLastError(0);
        AsFmemFileState(fileState)->operationState = kWriteCompleted;
        AsFmemFileState(fileState)->operationResult = result;
        callbackByteCount = result;
    } else {
        AsFmemFileState(fileState)->operationState = kWriteFailed;
        callbackError = result;
    }
    if (completionCallback) {
        typedef void (*Cb)(int, int, void*, int);
        ((Cb)completionCallback)(callbackError, callbackByteCount, fileState,
                                 AsFmemFileState(fileState)->callbackArgument);
    }
    // The negative destructor flag preserves wrapper storage for callback 3 to release.
    if (completionStorage) {
        FileIO_InvokeAllocatorCallback3((u32)completionStorage);
    }
    ((CallbackVirtualView*)this)->teardown(-1);
    return FileIO_InvokeAllocatorCallback3((u32)this);
}

// Optionally free a read-callback wrapper; flags > 0 deletes storage, otherwise return it unchanged.
void* gtl::CCallBackReadFile::Destroy(int flags) {
    if (this) {
        if (flags > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}
