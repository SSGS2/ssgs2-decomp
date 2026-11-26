#ifndef CLASSES_GTL_CCALLBACKGETFILESIZE_H
#define CLASSES_GTL_CCALLBACKGETFILESIZE_H

#include "types.h"

// Request state that the size callback writes its result into. Layout is defined
// where the size completion is implemented.
struct GetFileSizeRequest;

namespace gtl {

// Completion callback for a file-size query. It publishes the size to the request
// state, invokes the optional completion function, and owns its own storage.
class CCallBackGetFileSize {
public:
    // Optionally free the object. deleteFlag > 0 frees it. Returns the original address,
    // which may already be freed. A null object is accepted.
    void* Destroy(int deleteFlag);

    // Publish the size result, invoke the completion function, and destroy this object.
    void* OnSizeResult(int completionStatus, void* sizeResultBuffer);

    void* vtbl;                       // 0x00
    GetFileSizeRequest* requestState; // 0x04: borrowed request state
    void* completionCallback;         // 0x08
};

} // namespace gtl

#define GTL_CCALLBACKGETFILESIZE_OFF(f, o) \
    typedef char gtl_CCallBackGetFileSize_off_##f[((int)&((gtl::CCallBackGetFileSize*)0)->f == (o)) ? 1 : -1]
GTL_CCALLBACKGETFILESIZE_OFF(vtbl, 0x0);
GTL_CCALLBACKGETFILESIZE_OFF(requestState, 0x4);
GTL_CCALLBACKGETFILESIZE_OFF(completionCallback, 0x8);
#undef GTL_CCALLBACKGETFILESIZE_OFF

#endif // CLASSES_GTL_CCALLBACKGETFILESIZE_H
