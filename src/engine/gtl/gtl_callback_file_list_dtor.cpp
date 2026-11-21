// Translation unit 0x8041E85C-0x8041E8CC (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {
void FileIO_Free(void *p);
void FileIO_InvokeAllocatorCallback3(void *p);
enum { kHeapStringFlagShift = 31 };

struct gtl_CCallBackFileList {
    virtual void vfunc_0(int);
    void *callbackContext; /* 0x04; opaque to destruction */
    u32 stringFlags;       /* 0x08; top bit selects heap storage */
    u32 stringLength;      /* 0x0C */
    void *heapText;        /* 0x10 */
};
// The string starts at +8: flags/length/heap-pointer occupy +8/+C/+10.
// When the high flag bit is clear the same region is inline string storage;
// stringLength and heapText must not then be interpreted as separate fields.

// Release a callback-list allocation through the file-I/O allocator.
void dtor_8041E85C(void *p) {
    FileIO_Free(p);
}

// Release the callback's heap-backed string and optionally the callback itself.
// Null is accepted; deleting > 0 releases self. Returns the original address,
// possibly dangling. String storage uses allocator callback 3; self uses FileIO_Free.
void *gtl_CCallBackFileList_dtor(gtl_CCallBackFileList *self, int deleting) {
    if (self) {
        // Preserve the original embedded-string address guard.
        if (&self->stringFlags) {
            if (self->stringFlags >> kHeapStringFlagShift) {
                FileIO_InvokeAllocatorCallback3(self->heapText);
            }
        }
        if (deleting > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

} // extern "C"
