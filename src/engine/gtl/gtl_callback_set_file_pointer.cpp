// Destructor for the set-file-pointer (seek) callback (gtl::CCallBackSetFilePointer). It frees the
// callback object through the file-I/O allocator when the delete flag is positive.

#include "types.h"
#include "classes/gtl_CCallBackSetFilePointer.h"

extern "C" {
void FileIO_Free(void* p);
} // extern "C"

// End the seek callback's lifetime and free its storage only for a positive deleteFlag.
// Null is accepted; return the original address, which is unusable after freeing.
// A nonpositive flag leaves storage for the completion path's allocator hook.
void* gtl::CCallBackSetFilePointer::Destroy(int deleteFlag) {
    if (this) {
        if (deleteFlag > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}
