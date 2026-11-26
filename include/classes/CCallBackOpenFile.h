// gtl::CCallBackOpenFile: completion callback for an asynchronous file open.
// Same layout as FmemOperationCallback (vtable, file state, user payload).
#ifndef CLASSES_CCALLBACKOPENFILE_H
#define CLASSES_CCALLBACKOPENFILE_H

namespace gtl {

class CCallBackOpenFile {
public:
    void* vtbl;          // 0x0
    int argument;        // 0x4: file state
    int callbackPayload; // 0x8: user completion function

    void* Destroy(int deleteFlag);
    void* OnOpenComplete(int resultCode, void* completionData);
};

}

#endif
