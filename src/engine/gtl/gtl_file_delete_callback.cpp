// Translation unit 0x8041EEE4-0x8041EF58 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {

void *FileIO_InvokeAllocatorCallback3(void *p);

struct gtl_CCallBackDeleteFile {
    virtual void vfunc_0(int);
    virtual void vfunc_1();
    virtual void *vfunc_2(int);
    void *completionFunction; // +4: optional completion function, called with (code, context).
    int callbackContext;   // +8: opaque user context word, not interpreted here.
};

enum { kTeardownWithoutDelete = -1 };

// Notify the optional delete-file completion function and dispose of this callback object.
// self must be live; completionCode is passed unchanged (including nonzero errors).
// Return the allocator hook's opaque result, not the completion status. Do not
// access self after teardown; callback 3 receives its storage for disposal here.
void *gtl_CCallBackDeleteFile_OnDeleteResult(gtl_CCallBackDeleteFile *self, int completionCode) {
    int callbackCode = 0;
    if (completionCode != 0) {
        callbackCode = completionCode;
    }
    if (self->completionFunction) {
        ((void (*)(int, int))self->completionFunction)(callbackCode, self->callbackContext);
    }
    // Negative destructor argument retains storage for the separate allocator hook.
    self->vfunc_0(kTeardownWithoutDelete);
    return (void *)FileIO_InvokeAllocatorCallback3(self);
}

} // extern "C"
