#ifndef CLASSES_GTL_CCALLBACKREMOVEDIRECTORY_H
#define CLASSES_GTL_CCALLBACKREMOVEDIRECTORY_H

#include "types.h"

namespace gtl {

// Completion callback for a directory removal. It holds the caller's completion
// function and one opaque context word, and owns its own storage.
class CCallBackRemoveDirectory {
public:
    // Optionally free the object. shouldDelete > 0 frees it. Returns the original
    // address, which may already be freed.
    void* Destroy(int shouldDelete);

    // Report the completion code to the completion function (a zero code stays zero),
    // then end the object's lifetime without freeing it and pass its storage to allocator callback 3.
    void Complete(int completionCode);

    void* vtbl;       // 0x00
    void* completion; // 0x04; called as (code, context)
    int context;      // 0x08
};

} // namespace gtl

#define GTL_CCALLBACKREMOVEDIRECTORY_OFF(f, o) \
    typedef char gtl_CCallBackRemoveDirectory_off_##f[((int)&((gtl::CCallBackRemoveDirectory*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKREMOVEDIRECTORY_OFF(vtbl, 0x0);
GTL_CCALLBACKREMOVEDIRECTORY_OFF(completion, 0x4);
GTL_CCALLBACKREMOVEDIRECTORY_OFF(context, 0x8);
#undef GTL_CCALLBACKREMOVEDIRECTORY_OFF

#endif // CLASSES_GTL_CCALLBACKREMOVEDIRECTORY_H
