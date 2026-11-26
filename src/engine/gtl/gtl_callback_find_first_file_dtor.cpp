// Destructor for the find-first-file callback (gtl::CCallBackFindFirstFile). It releases the heap
// path string through allocator callback 3 when its flag bit is set, then frees the object if asked.

#include "types.h"
#include "classes/gtl_CCallBackFindFirstFile.h"

extern "C" {
void FileIO_Free(void* p);
void FileIO_InvokeAllocatorCallback3(void* p);
enum {
    kHeapStringFlagShift = 31
};
} // extern "C"

// Destroy the owned long-string storage, then optionally release the callback object.
// Null is accepted; deleting > 0 frees the object through FileIO_Free. The returned address
// is the original this and may be dangling. Callback 3 releases the string allocation;
// it is distinct from the callback used to release the object itself.
void* gtl::CCallBackFindFirstFile::Destroy(int deleting) {
    if (this != 0) {
        // Retain the original embedded-string address guard (+8 is pathFlags).
        if ((u32)this + 8) {
            // High bit means heap-backed; clear means inline storage, not a pointer.
            if (pathFlags >> kHeapStringFlagShift) {
                FileIO_InvokeAllocatorCallback3(heapPath);
            }
        }
        if (deleting > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}
