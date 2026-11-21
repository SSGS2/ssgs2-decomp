// Translation unit 0x8041A0DC-0x8041A13C (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
void FileIO_Free(void *p);

enum CallbackDestroyConstants { kDeletingDestructorTableIndex = 2, kDeleteStorage = 1 };

// Optionally release callback storage through FileIO and return its original address.
// Null is accepted; deleteStorage > 0 frees self, so the return may be dangling.
void *FileIOCallback_dtor(void *self, int deleteStorage) {
    if (self) {
        if (deleteStorage > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

// Destroy and release a non-null callback using its deleting-destructor dispatch slot.
// The table's two metadata words precede slot zero at byte +0x08 (index 2).
// Consumes the callback; null is a no-op and no reference is retained.
void gtl_ICallBack_Destroy(void *callback) {
    if (callback) {
        (*(void (***)(void *, int))callback)[kDeletingDestructorTableIndex](callback, kDeleteStorage);
    }
}
} // extern "C"
