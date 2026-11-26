#ifndef CLASSES_GTL_CCALLBACKCREATEDIRECTORY_H
#define CLASSES_GTL_CCALLBACKCREATEDIRECTORY_H

#include "types.h"

namespace gtl {

// Completion callback for a directory creation. It holds the caller's completion
// function and one opaque context word, and owns its own storage.
class CCallBackCreateDirectory {
public:
    // Optionally free the object. deleteFlag > 0 frees it. Returns the original
    // address, which may already be freed.
    void* Destroy(int deleteFlag);

    // Report the status to the completion function (zero stays zero), then tear the
    // object down without freeing it and pass its storage to allocator callback 3.
    void CompleteAndDestroy(int code);

    void* vtbl;       // 0x00
    void* completion; // 0x04; called as (code, context)
    int context;      // 0x08
};

} // namespace gtl

#define GTL_CCALLBACKCREATEDIRECTORY_OFF(f, o) \
    typedef char gtl_CCallBackCreateDirectory_off_##f[((int)&((gtl::CCallBackCreateDirectory*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKCREATEDIRECTORY_OFF(vtbl, 0x0);
GTL_CCALLBACKCREATEDIRECTORY_OFF(completion, 0x4);
GTL_CCALLBACKCREATEDIRECTORY_OFF(context, 0x8);
#undef GTL_CCALLBACKCREATEDIRECTORY_OFF

#endif // CLASSES_GTL_CCALLBACKCREATEDIRECTORY_H
