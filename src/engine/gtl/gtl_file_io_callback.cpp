// Shared destruction for GTL file I/O callbacks: a null-safe release helper and ICallBack::Destroy,
// which runs the callback's deleting destructor.
//
// A positive delete flag also frees the callback storage through the file I/O allocator.

#include "classes/gtl_ICallBack.h"

extern "C" {
void FileIO_Free(void* p);

enum CallbackDestroyConstants {
    kDeletingDestructorTableIndex = 2,
    kDeleteStorage = 1
};

// Optionally release callback storage through FileIO and return its original address.
// Null is accepted; deleteStorage > 0 frees self, so the return may be dangling.
void* FileIOCallback_dtor(void* self, int deleteStorage) {
    if (self) {
        if (deleteStorage > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

} // extern "C"

// Destroy and release a non-null callback using its deleting-destructor dispatch slot.
// The table's two metadata words precede slot zero at byte +0x08 (index 2).
// Consumes the callback; null is a no-op and no reference is retained.
void gtl::ICallBack::Destroy() {
    if (this) {
        (*(void (***)(void*, int))this)[kDeletingDestructorTableIndex](this, kDeleteStorage);
    }
}
