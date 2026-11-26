// Asynchronous file-size query: the callback object that stores the size result in its
// request, calls the optional completion callback and then destroys itself.

#include <decomp/local_class.h>
#include "types.h"
#include "sdk_protos.h"
#include "classes/gtl_CCallBackGetFileSize.h"

extern "C" {
void* memcpy(void* dst, const void* src, size_t n);
void* FileIO_InvokeAllocatorCallback3(void* p);
void File_SetLastError(int x);

// Virtual slot 0 of a callback object (the destructor-like entry), called through a raw pointer.
struct CallbackVirtualView {
    virtual void teardown(int) DECOMP_LOCAL_VIRTUAL;
};
} // extern "C"

// Request state that the size query writes its result into.
struct GetFileSizeRequest {
    char unk[0x8c];
    unsigned int status;             // 0x8c
    unsigned int size;               // 0x90
    unsigned int completionUserWord; // 0x94
};

enum GetFileSizeCompletion {
    kSizeQuerySucceeded = 1,
    kSizeQueryFailed = 2
};

// Publish an asynchronous size result, invoke the optional callback, and destroy this wrapper.
// sizeResultBuffer must contain a readable u32 even on failure: memcpy precedes its
// null check. The buffer and self are consumed via the file-I/O allocator callback.
// requestState is borrowed request state. cb(status, sizeWords, request, extra) receives
// a stack {high=0, low=fileSizeBytes} pair valid only during that synchronous call.
// Negative completionStatus marks failure without changing the request's old size;
// success clears last error. Return value is the allocator callback's opaque result.
void* gtl::CCallBackGetFileSize::OnSizeResult(int completionStatus, void* sizeResultBuffer) {
    // Phase 1: extract the size before releasing its temporary result allocation.
    unsigned int fileSizeBytes = 0;
    memcpy(&fileSizeBytes, sizeResultBuffer, 4);
    if (sizeResultBuffer) {
        FileIO_InvokeAllocatorCallback3(sizeResultBuffer);
    }

    unsigned int callbackSizeWords[2];
    callbackSizeWords[0] = 0;
    callbackSizeWords[1] = fileSizeBytes;

    // Phase 2: publish completion before notifying the caller.
    if (completionStatus >= 0) {
        File_SetLastError(0);
        requestState->status = kSizeQuerySucceeded;
        requestState->size = fileSizeBytes;
    } else {
        requestState->status = kSizeQueryFailed;
    }

    if (completionCallback) {
        typedef void (*CompletionCallback)(int, void*, void*, unsigned int);
        ((CompletionCallback)completionCallback)(completionStatus, callbackSizeWords, requestState,
                                                 requestState->completionUserWord);
    }

    // Phase 3: virtual cleanup uses -1, followed by the allocator's release hook.
    ((CallbackVirtualView*)this)->teardown(-1);
    return FileIO_InvokeAllocatorCallback3(this);
}
