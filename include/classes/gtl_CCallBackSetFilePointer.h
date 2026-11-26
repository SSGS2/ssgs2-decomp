#ifndef CLASSES_GTL_CCALLBACKSETFILEPOINTER_H
#define CLASSES_GTL_CCALLBACKSETFILEPOINTER_H

#include "types.h"

namespace gtl {

// Completion callback for a file seek. It records the seek result in the borrowed
// file state, invokes the optional completion function, and owns its own storage.
class CCallBackSetFilePointer {
public:
    // End the object's lifetime. deleteFlag > 0 also frees it. Null is accepted.
    // Returns the original address, which may already be freed.
    void* Destroy(int deleteFlag);

    // Record the seek result (arg >= 0 is the new position, negative is an error), notify
    // the completion function, then release this object through allocator callback 3.
    void* Complete(int arg);

    void* vtbl;               // 0x00
    void* fileState;          // 0x04: borrowed open-file state until completion
    void* completionCallback; // 0x08
};

} // namespace gtl

#define GTL_CCALLBACKSETFILEPOINTER_OFF(f, o) \
    typedef char gtl_CCallBackSetFilePointer_off_##f[((int)&((gtl::CCallBackSetFilePointer*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKSETFILEPOINTER_OFF(vtbl, 0x0);
GTL_CCALLBACKSETFILEPOINTER_OFF(fileState, 0x4);
GTL_CCALLBACKSETFILEPOINTER_OFF(completionCallback, 0x8);
#undef GTL_CCALLBACKSETFILEPOINTER_OFF

#endif // CLASSES_GTL_CCALLBACKSETFILEPOINTER_H
