// Translation unit 0x8041E25C-0x8041E2C8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "classes/CallBackFindFirstFileState.h"

extern "C" {
void FileIO_Free(void *p);
void FileIO_InvokeAllocatorCallback3(void *p);
enum { kHeapStringFlagShift = 31 };

// Destroy the owned long-string storage, then optionally release the callback object.
// Null is accepted; deleting > 0 frees self through FileIO_Free. The returned address
// is the original self and may be dangling. Callback 3 releases the string allocation;
// it is distinct from the callback used to release the object itself.
void *gtl_CCallBackFindFirstFile_dtor(void *self, int deleting) {
    if (self != 0) {
        // Retain the original embedded-string address guard (+8 is stringFlags).
        if ((u32)self + 8) {
            // High bit means heap-backed; clear means inline storage, not a pointer.
            if (((CallBackFindFirstFileState*)self)->stringFlags >> kHeapStringFlagShift) {
                FileIO_InvokeAllocatorCallback3(((CallBackFindFirstFileState*)self)->stringData);
            }
        }
        if (deleting > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

} // extern "C"
