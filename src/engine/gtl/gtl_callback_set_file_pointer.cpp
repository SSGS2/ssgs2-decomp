// Translation unit 0x8041F08C-0x8041F0CC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {
void FileIO_Free(void *p);

struct gtl_CCallBackSetFilePointer {
    virtual void vfunc_0(int);
};

// End the seek callback's lifetime and free its storage only for a positive deleteFlag.
// Null is accepted; return the original address, which is unusable after freeing.
// A nonpositive flag leaves storage for the completion path's allocator hook.
void *gtl_CCallBackSetFilePointer_dtor(gtl_CCallBackSetFilePointer *self, int deleteFlag) {
    if (self) {
        if (deleteFlag > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

} // extern "C"
