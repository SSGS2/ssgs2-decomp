#ifndef CLASSES_GTL_CCALLBACKDELETEFILE_H
#define CLASSES_GTL_CCALLBACKDELETEFILE_H

#include "types.h"

namespace gtl {

// Completion callback for a file delete. It holds the caller's completion function and
// one opaque context word, and owns its own storage.
class CCallBackDeleteFile {
public:
    // Optionally free the object. deleteStorage > 0 frees it. Returns the original
    // address, which may already be freed.
    void* Destroy(int deleteStorage);

    // Pass the completion code to the completion function, end the object's lifetime
    // without freeing it, and pass its storage to allocator callback 3.
    void* OnDeleteResult(int completionCode);

    void* vtbl;               // 0x00
    void* completionFunction; // 0x04: called as (code, context)
    int callbackContext;      // 0x08
};

} // namespace gtl

#define GTL_CCALLBACKDELETEFILE_OFF(f, o) \
    typedef char gtl_CCallBackDeleteFile_off_##f[((int)&((gtl::CCallBackDeleteFile*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKDELETEFILE_OFF(vtbl, 0x0);
GTL_CCALLBACKDELETEFILE_OFF(completionFunction, 0x4);
GTL_CCALLBACKDELETEFILE_OFF(callbackContext, 0x8);
#undef GTL_CCALLBACKDELETEFILE_OFF

#endif // CLASSES_GTL_CCALLBACKDELETEFILE_H
