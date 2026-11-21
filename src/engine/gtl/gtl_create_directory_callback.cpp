// Translation unit 0x8041E1E8-0x8041E25C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

namespace gtl {
struct CCallBackCreateDirectory {
    // ABI destructor slot (+0x08), despite its legacy name; negative flag retains storage.
    virtual void v2(int);
};
}

extern "C" {
void FileIO_InvokeAllocatorCallback3(void *p);

enum DirectoryCallbackLayout {
    kCompletionFunctionOffset = 4, kUserArgumentOffset = 8,
    kDestroyWithoutFreeing = -1
};

// Forward directory-creation status to the optional user callback, then dispose of self.
// code is forwarded unchanged (0 remains success); the second callback argument is
// the stored opaque word at +8. self must be valid and is consumed even without a callback.
// Teardown uses the destructor virtual first, then configured allocator callback 3;
// that callback's shared contract is unresolved, so do not replace it with FileIO_Free.
void gtl_CCallBackCreateDirectory_CompleteAndDestroy(void *self, int code) {
    int completionCode = 0;
    if (code != 0) {
        completionCode = code;
    }
    void *userCallback = *(void **)((char *)self + kCompletionFunctionOffset);
    if (userCallback) {
        typedef void (*DirectoryCompletion)(int, int);
        ((DirectoryCompletion)userCallback)(completionCode, *(int *)((char *)self + kUserArgumentOffset));
    }
    // The callback has returned; perform teardown before handing storage to the allocator.
    ((gtl::CCallBackCreateDirectory *)self)->v2(kDestroyWithoutFreeing);
    FileIO_InvokeAllocatorCallback3(self);
}

} // extern "C"
