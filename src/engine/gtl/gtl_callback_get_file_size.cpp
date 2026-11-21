#include "types.h"

extern "C" {
extern void FileIO_Free(void *p);

// Destroy the file-size callback, optionally releasing its allocation.
// self may be null. A positive deleteFlag transfers storage to FileIO_Free;
// zero/negative retains storage. Return the original address, even after freeing
// it: callers must not dereference that return value when storage was released.
void *gtl_CCallBackGetFileSize_dtor(void *self, int deleteFlag) {
    if (self != 0 && deleteFlag > 0) {
        FileIO_Free(self);
    }
    return self;
}
} // extern "C"
