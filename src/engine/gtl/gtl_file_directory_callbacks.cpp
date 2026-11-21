#pragma push
// Translation unit 0x8041E134-0x8041E1E8 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {
void FileIO_Free(void *p);
void FileIO_InvokeAllocatorCallback3(void *p);

enum { kCompletionFunctionOffset = 4, kUserContextOffset = 8,
       kTeardownVtableByteOffset = 8, kTeardownWithoutDelete = -1 };

struct DirectoryCallbackVirtuals {
    virtual void teardown(int);
};

// Finish the create-directory callback's lifetime; positive deleteFlag also frees its storage.
// Return the original address (possibly freed); null is accepted and nonpositive flags retain storage.
void *gtl_CCallBackCreateDirectory_dtor(void *self, int deleteFlag);

// Forward directory-removal completion to the optional user callback, then dispose of self.
// self is a live callback object: +4 holds the function, +8 its opaque context word.
// User arguments are (completionCode, context); zero is passed through as zero,
// and every nonzero code is preserved. The allocator hook controls final disposal.
void gtl_CCallBackRemoveDirectory_Complete(void *self, int completionCode);

} // extern "C"

#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
void gtl_CCallBackRemoveDirectory_Complete(void *self, int completionCode) {
    int callbackCode = 0;
    if (completionCode != 0) {
        callbackCode = completionCode;
    }
    // Notify before teardown so the callback object is still alive during the call.
    void *completionFunction = *(void **)((char *)self + kCompletionFunctionOffset);
    if (completionFunction) {
        typedef void (*CompletionFunction)(int, int);
        ((CompletionFunction)completionFunction)(callbackCode, *(int *)((char *)self + kUserContextOffset));
    }
    // End the object lifetime without FileIO_Free, then pass storage to callback 3.
    ((DirectoryCallbackVirtuals*)self)->teardown(kTeardownWithoutDelete);
    FileIO_InvokeAllocatorCallback3(self);
}
#pragma pop

#pragma push
void *gtl_CCallBackCreateDirectory_dtor(void *self, int deleteFlag) {
    if (self != 0) {
        if (deleteFlag > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}
#pragma pop

}
