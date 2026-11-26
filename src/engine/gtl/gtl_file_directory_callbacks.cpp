// Directory callbacks for GTL file I/O: the completion of a remove-directory request and the
// teardown of a create-directory request.
//
// The remove-directory completion passes its result to the user callback and then releases the
// callback. The create-directory teardown frees the callback storage when asked to.

#pragma push

#include "types.h"
#include "classes/gtl_CCallBackRemoveDirectory.h"
#include "classes/gtl_CCallBackCreateDirectory.h"

extern "C" {
void FileIO_Free(void* p);
void FileIO_InvokeAllocatorCallback3(void* p);

enum {
    kTeardownWithoutDelete = -1
};

// Views the object's destructor virtual (vtable slot 2) for the teardown call.
struct DirectoryCallbackVirtuals {
    virtual void teardown(int);
};

} // extern "C"

#pragma pop

// Definitions follow the original text-address order.
#pragma push
// Forward directory-removal completion to the optional user callback, then dispose of self.
// User arguments are (completionCode, context); zero is passed through as zero,
// and every nonzero code is preserved. The allocator hook controls final disposal.
void gtl::CCallBackRemoveDirectory::Complete(int completionCode) {
    int callbackCode = 0;
    if (completionCode != 0) {
        callbackCode = completionCode;
    }
    // Notify before teardown so the callback object is still alive during the call.
    if (completion) {
        typedef void (*CompletionFunction)(int, int);
        ((CompletionFunction)completion)(callbackCode, context);
    }
    // End the object lifetime without FileIO_Free, then pass storage to callback 3.
    ((DirectoryCallbackVirtuals*)this)->teardown(kTeardownWithoutDelete);
    FileIO_InvokeAllocatorCallback3(this);
}
#pragma pop

#pragma push
// Finish the create-directory callback's lifetime; positive deleteFlag also frees its storage.
// Return the original address (possibly freed); null is accepted and nonpositive flags retain storage.
void* gtl::CCallBackCreateDirectory::Destroy(int deleteFlag) {
    if (this != 0) {
        if (deleteFlag > 0) {
            FileIO_Free(this);
        }
    }
    return this;
}
#pragma pop
