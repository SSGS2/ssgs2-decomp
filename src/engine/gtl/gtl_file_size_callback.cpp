#include <decomp/local_class.h>
#include "types.h"
#include "sdk_protos.h"

extern "C" {
void *memcpy(void *dst, const void *src, size_t n);
void *FileIO_InvokeAllocatorCallback3(void *p);
void File_SetLastError(int x);

struct gtl_CCallBackGetFileSize_Inner {
    char unk[0x8c];
    unsigned int status;   /* 0x8c */
    unsigned int size;     /* 0x90 */
    unsigned int completionUserWord;    /* 0x94 */
};

struct gtl_CCallBackGetFileSize {
    virtual void vfunc_0(int);
    gtl_CCallBackGetFileSize_Inner *requestState; /* 0x04 */
    void *completionCallback;                              /* 0x08 */
};

enum GetFileSizeCompletion { kSizeQuerySucceeded = 1, kSizeQueryFailed = 2 };

// Publish an asynchronous size result, invoke the optional callback, and destroy this wrapper.
// sizeResultBuffer must contain a readable u32 even on failure: memcpy precedes its
// null check. The buffer and self are consumed via the file-I/O allocator callback.
// requestState is borrowed request state. cb(status, sizeWords, request, extra) receives
// a stack {high=0, low=fileSizeBytes} pair valid only during that synchronous call.
// Negative completionStatus marks failure without changing the request's old size;
// success clears last error. Return value is the allocator callback's opaque result.
void *gtl_CCallBackGetFileSize_OnSizeResult(gtl_CCallBackGetFileSize *self, int completionStatus, void *sizeResultBuffer) {
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
        self->requestState->status = kSizeQuerySucceeded;
        self->requestState->size = fileSizeBytes;
    } else {
        self->requestState->status = kSizeQueryFailed;
    }

    if (self->completionCallback) {
        typedef void (*CompletionCallback)(int, void *, void *, unsigned int);
        ((CompletionCallback)self->completionCallback)(completionStatus, callbackSizeWords, self->requestState, self->requestState->completionUserWord);
    }

    // Phase 3: virtual cleanup uses -1, followed by the allocator's release hook.
    struct CallbackVirtualView { 
    virtual void m(int) DECOMP_LOCAL_VIRTUAL;
    };
    ((CallbackVirtualView*)self)->m(-1);
    return FileIO_InvokeAllocatorCallback3(self);
}
} // extern "C"
