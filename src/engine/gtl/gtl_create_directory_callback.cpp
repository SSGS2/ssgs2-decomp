// Completion for the create-directory callback, gtl::CCallBackCreateDirectory. It reports
// the result code, then tears the object down through its destructor virtual.

#include "types.h"
#include "classes/gtl_CCallBackCreateDirectory.h"

extern "C" {
void FileIO_InvokeAllocatorCallback3(void* p);
}

enum DirectoryCallbackLayout {
    kDestroyWithoutFreeing = -1
};

// Views the object's destructor virtual (vtable slot 2) for the teardown call.
struct DirectoryCallbackVirtuals {
    virtual void teardown(int);
};

// Forward directory-creation status to the optional user callback, then dispose of self.
// code is forwarded unchanged (0 remains success); the second callback argument is
// the stored opaque word at +8. The object is consumed even without a callback.
// Teardown uses the destructor virtual first, then configured allocator callback 3;
// that callback's shared contract is unresolved, so do not replace it with FileIO_Free.
void gtl::CCallBackCreateDirectory::CompleteAndDestroy(int code) {
    int completionCode = 0;
    if (code != 0) {
        completionCode = code;
    }
    if (completion) {
        typedef void (*DirectoryCompletion)(int, int);
        ((DirectoryCompletion)completion)(completionCode, context);
    }
    // The callback has returned; perform teardown before handing storage to the allocator.
    ((DirectoryCallbackVirtuals*)this)->teardown(kDestroyWithoutFreeing);
    FileIO_InvokeAllocatorCallback3(this);
}
