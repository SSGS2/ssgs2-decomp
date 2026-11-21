// Translation unit 0x8041DEAC-0x8041DEEC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {
void FileIO_Free(void *p);

// Optionally release the save-area callback wrapper; there is no payload cleanup.
// Null self is accepted. deleteFlag > 0 frees self; zero/negative values preserve
// storage. Return the original address, which may already have been freed.
void *gtl_CCallBackRVLCheckSaveArea_dtor(void *self, int deleteFlag) {
    if (self != 0) {
        if (deleteFlag > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

} // extern "C"
