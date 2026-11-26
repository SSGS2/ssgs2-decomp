// Destructor for the open-file callback (gtl::CCallBackOpenFile). It frees the callback object
// through the file-I/O allocator when the delete flag is positive; otherwise the storage is kept.

#include "types.h"
#include "classes/CCallBackOpenFile.h"

extern "C" {
extern void FileIO_Free(void* p);
}

namespace gtl {

// End the open-file callback's lifetime and free its storage only for a positive deleteFlag.
// Null is accepted; return the original address, which may already have been freed.
// Nonpositive flags let the completion path dispose of storage separately.
void* CCallBackOpenFile::Destroy(int deleteFlag) {
    if (this != 0 && deleteFlag > 0) {
        FileIO_Free(this);
    }
    return this;
}

} // namespace gtl
