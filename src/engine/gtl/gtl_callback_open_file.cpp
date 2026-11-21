// Translation unit 0x8041F5F4-0x8041F634 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {
extern void FileIO_Free(void *p);

// End the open-file callback's lifetime and free its storage only for a positive deleteFlag.
// Null is accepted; return the original address, which may already have been freed.
// Nonpositive flags let the completion path dispose of storage separately.
void *gtl_CCallBackOpenFile_dtor(void *self, int deleteFlag) {
    if (self != 0 && deleteFlag > 0) {
        FileIO_Free(self);
    }
    return self;
}

} // extern "C"
