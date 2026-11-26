// Destroy for the save-area check callback, gtl::CCallBackRVLCheckSaveArea. The object
// is freed only when the delete flag is positive; it has no payload to clean up.

#include "types.h"
#include "classes/gtl_CCallBackRVLCheckSaveArea.h"

extern "C" {
void FileIO_Free(void* p);
}

// Optionally release the save-area callback wrapper; there is no payload cleanup.
// Null is accepted. deleteFlag > 0 frees the object; zero/negative values preserve
// storage. Return the original address, which may already have been freed.
void* gtl::CCallBackRVLCheckSaveArea::Destroy(int deleteFlag) {
    if (this != 0) {
        if (deleteFlag > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}
