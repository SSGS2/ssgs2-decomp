#include "types.h"

extern "C" {
void FileIO_Free(void *p);
void *FileIO_InvokeAllocatorCallback3(void *p);

struct gtl_CCallBackDeleteFile {
    virtual void vfunc_0(int);
};

// Optionally release the delete-file callback through FileIO; return its old address.
// deleteStorage > 0 frees self; zero/negative values retain storage. Null is accepted.
void *gtl_CCallBackDeleteFile_dtor(gtl_CCallBackDeleteFile *self, int deleteStorage) {
    if (self) {
        if (deleteStorage > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

namespace gtl {
struct CCallBackDiskFreeSpace {
// Slot 0x8 is the destructor-like entry; the original calls it with -1 in r4.
    virtual void vfunc_0(int); /* slot 0x8: the destructor-like entry the original calls with -1 */
    virtual void vfunc_1();
    virtual void vfunc_2(int);
    void *cb;     // +0x04: not accessed here; its role is unresolved.
    void *completionCallback; // +0x08: optional completion function, not an allocation.
    int completionUserDataWord;   // +0x0C: opaque word passed as the completion function's last argument.
};
}

enum DiskSpaceCompletionConstants {
    kSecondDiskSpaceResultValue = 0x01000000, // Fixed middle result; role/unit unresolved.
    kCompletionStatusZero = 0,      // Meaning beyond the emitted status word is unresolved.
    kDestroyWithoutFree = -1
};

// Report fixed disk-space result words to the optional callback, then dispose this request.
// Completion arguments are (0, &first, &second, &third, fieldC), with 64-bit
// values {0,0x1000000,0}; capacity roles/units are unresolved. Pointers expire on return.
// Valid self is required; this destroys it, then passes it to allocator callback 3.
// The returned value is callback 3's result, not an owned or retained self pointer.
void *gtl_CCallBackDiskFreeSpace_CompleteAndDispose(gtl::CCallBackDiskFreeSpace *self) {
    // Phase 1: retain the six original volatile stack-word stores. This outer
    // group is not passed to the callback; the call uses the 64-bit locals below.
    volatile int firstHighWord = 0;
    volatile int firstLowWord = 0;
    volatile int secondHighWord = 0;
    volatile int secondLowWord = kSecondDiskSpaceResultValue;
    volatile int thirdHighWord = 0;
    volatile int thirdLowWord = 0;
    // Phase 2: callback borrows three independently addressed 64-bit stack values.
    if (self->completionCallback) {
        volatile unsigned long long first = 0;
        volatile unsigned long long second = kSecondDiskSpaceResultValue;
        volatile unsigned long long third = 0;
        ((void (*)(int, void *, void *, void *, int))self->completionCallback)(kCompletionStatusZero, (void *)&first, (void *)&second, (void *)&third, self->completionUserDataWord);
    }
    // Phase 3: vtable slot +0x08 tears down without freeing; callback 3 disposes
    // storage using the configured allocator. It must not be replaced by FileIO_Free.
    self->vfunc_0(kDestroyWithoutFree);
    return (void *)FileIO_InvokeAllocatorCallback3(self);
}

} // extern "C"
