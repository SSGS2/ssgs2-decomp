// Destructor for the file-list callback (gtl::CCallBackFileList). It frees the heap-backed string
// through allocator callback 3 when its flag bit is set, then frees the object itself.
// Inline string storage needs no release.

#include "types.h"
#include "classes/gtl_CCallBackFileList.h"

extern "C" {
void FileIO_Free(void* p);
void FileIO_InvokeAllocatorCallback3(void* p);
enum {
    kHeapStringFlagShift = 31
};

// Release a callback-list allocation through the file-I/O allocator.
void dtor_8041E85C(void* p) {
    FileIO_Free(p);
}

} // extern "C"

// The string starts at +8: flags/length/heap-pointer occupy +8/+C/+10.
// When the high flag bit is clear the same region is inline string storage;
// stringLength and heapText must not then be interpreted as separate fields.

// Release the callback's heap-backed string and optionally the callback itself.
// Null is accepted; deleting > 0 releases the object. Returns the original address,
// possibly dangling. String storage uses allocator callback 3; the object uses FileIO_Free.
void* gtl::CCallBackFileList::Destroy(int deleting) {
    if (this) {
        // Preserve the original embedded-string address guard.
        if (&stringFlags) {
            if (stringFlags >> kHeapStringFlagShift) {
                FileIO_InvokeAllocatorCallback3(heapText);
            }
        }
        if (deleting > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}
