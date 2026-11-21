// Translation unit 0x8041F3A8-0x8041F420 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {
void FileIO_Free(void *p);
extern char __vt__Q23gtl19CCallBackCreateFile[];

// Release the create callback's owned path, then optionally free the callback itself.
// The captured file/completion callback are borrowed and untouched. Null is accepted;
// deleting > 0 frees self. The ABI returns self even when that address has been freed.
void *gtl_CCallBackCreateFile_dtor(void *self, int deleting) {
    if (self) {
        void *ownedPath = *(void **)((char *)self + 8);
        *(void **)self = (void *)__vt__Q23gtl19CCallBackCreateFile;
        if (ownedPath) {
            FileIO_Free(ownedPath);
            *(void **)((char *)self + 8) = 0;
        }
        if (deleting > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

} // extern "C"
