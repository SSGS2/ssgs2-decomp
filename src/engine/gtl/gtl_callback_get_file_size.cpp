// Destructor for the file-size callback (gtl::CCallBackGetFileSize). It frees the callback object
// through the file-I/O allocator when the delete flag is positive; otherwise the storage is kept.

#include "types.h"
#include "classes/gtl_CCallBackGetFileSize.h"

extern "C" {
extern void FileIO_Free(void* p);
} // extern "C"

// Destroy the file-size callback, optionally releasing its allocation.
// self may be null. A positive deleteFlag transfers storage to FileIO_Free;
// zero/negative retains storage. Return the original address, even after freeing
// it: callers must not dereference that return value when storage was released.
void* gtl::CCallBackGetFileSize::Destroy(int deleteFlag) {
    if (this != 0 && deleteFlag > 0) {
        FileIO_Free(this);
    }
    return this;
}
