// Callback objects for file deletion and free disk space (gtl::CCallBackDeleteFile and
// gtl::CCallBackDiskFreeSpace). Their destroy and completion steps free or dispose the object.

#include "types.h"
#include <decomp/local_class.h>
#include "classes/gtl_CCallBackDeleteFile.h"
#include "classes/gtl_CCallBackDiskFreeSpace.h"

extern "C" {
void FileIO_Free(void* p);
void* FileIO_InvokeAllocatorCallback3(void* p);

// Virtual slot 0 of a callback object (the destructor-like entry), called through a raw pointer.
struct CallbackVirtualView {
    virtual void teardown(int) DECOMP_LOCAL_VIRTUAL;
};

} // extern "C"

enum DiskSpaceCompletionConstants {
    kSecondDiskSpaceResultValue = 0x01000000, // Fixed middle result word.
    kCompletionStatusZero = 0,                // Status word passed to the completion function.
    kDestroyWithoutFree = -1
};

// Optionally release the delete-file callback through FileIO; return its old address.
// deleteStorage > 0 frees self; zero/negative values retain storage. Null is accepted.
void* gtl::CCallBackDeleteFile::Destroy(int deleteStorage) {
    if (this) {
        if (deleteStorage > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}

// Report fixed disk-space result words to the optional callback, then dispose this request.
// Completion arguments are (0, &first, &second, &third, fieldC), with 64-bit
// values {0,0x1000000,0}. Pointers expire on return.
// Valid self is required; this destroys it, then passes it to allocator callback 3.
// The returned value is callback 3's result, not an owned or retained self pointer.
void* gtl::CCallBackDiskFreeSpace::CompleteAndDispose() {
    // Phase 1: retain the six original volatile stack-word stores. This outer
    // group is not passed to the callback; the call uses the 64-bit locals below.
    volatile int firstHighWord = 0;
    volatile int firstLowWord = 0;
    volatile int secondHighWord = 0;
    volatile int secondLowWord = kSecondDiskSpaceResultValue;
    volatile int thirdHighWord = 0;
    volatile int thirdLowWord = 0;
    // Phase 2: callback borrows three independently addressed 64-bit stack values.
    if (completionCallback) {
        volatile unsigned long long first = 0;
        volatile unsigned long long second = kSecondDiskSpaceResultValue;
        volatile unsigned long long third = 0;
        ((void (*)(int, void*, void*, void*, int))completionCallback)(
            kCompletionStatusZero, (void*)&first, (void*)&second, (void*)&third, completionUserDataWord);
    }
    // Phase 3: vtable slot +0x08 tears down without freeing; callback 3 disposes
    // storage using the configured allocator. It must not be replaced by FileIO_Free.
    ((CallbackVirtualView*)this)->teardown(kDestroyWithoutFree);
    return (void*)FileIO_InvokeAllocatorCallback3(this);
}
