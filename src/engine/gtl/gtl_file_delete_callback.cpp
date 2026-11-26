// Completion for the file delete callback (gtl::CCallBackDeleteFile). It records the result
// code and then hands the callback's storage to the disposal step.

#include "types.h"
#include <decomp/local_class.h>
#include "classes/gtl_CCallBackDeleteFile.h"

extern "C" {

void* FileIO_InvokeAllocatorCallback3(void* p);

// Virtual slot 0 of a callback object (the destructor-like entry), called through a raw pointer.
struct CallbackVirtualView {
    virtual void teardown(int) DECOMP_LOCAL_VIRTUAL;
};

enum {
    kTeardownWithoutDelete = -1
};

} // extern "C"

// Notify the optional delete-file completion function and dispose of this callback object.
// self must be live; completionCode is passed unchanged (including nonzero errors).
// Return the allocator hook's opaque result, not the completion status. Do not
// access self after teardown; callback 3 receives its storage for disposal here.
void* gtl::CCallBackDeleteFile::OnDeleteResult(int completionCode) {
    int callbackCode = 0;
    if (completionCode != 0) {
        callbackCode = completionCode;
    }
    if (completionFunction) {
        ((void (*)(int, int))completionFunction)(callbackCode, callbackContext);
    }
    // Negative destructor argument retains storage for the separate allocator hook.
    ((CallbackVirtualView*)this)->teardown(kTeardownWithoutDelete);
    return (void*)FileIO_InvokeAllocatorCallback3(this);
}
