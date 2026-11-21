#include "classes/FmemFileState.h"
#include "classes/FmemOperationCallback.h"
#include "classes/GtlCreateFileCallback.h"
// View the existing callback's captured arguments without retaining it.
static inline GtlCreateFileCallback* AsGtlCreateFileCallback(void* object) { return (GtlCreateFileCallback*)object; }
// Translation unit 0x8041F420-0x8041F5F4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

extern "C" {
extern char __vt__Q23gtl17CCallBackOpenFile[];

// Slot 2 is the deleting destructor in these callback vtables.
class DeletingCallback { public: 
virtual void destroy(short);
};
struct NandCallbackContext {
    void* callback;
    void* owner;
    int headerBytes;
    unsigned char commandBlock[1];
};
// context is an allocated 12-byte dispatch header followed by the NAND command block.
// The header links the operation callback to the allocation containing that block.
// FmemOperationCallback's generic argument/userArgument slots hold file/completion
// pointers specifically for OpenFile; they have different meanings for other callbacks.
enum { kNandAlreadyExists = -6, kFileOperationComplete = 2,
       kAsyncOpenSubmitError = 0x6e, kCallbackHeaderBytes = 0xc };

void FileIO_Free(void *p);
void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void *self, void *createResult);
void *FileIO_InvokeAllocatorCallback2(u32 n);
void *FileIO_InvokeAllocatorCallback3(void *p);
void File_SetLastError(int x);
void NANDSetUserData(void *a, void *b);
int NANDOpenAsync(void *a, void *b, int c, void *d, void *e);

#pragma use_lmw_stmw on
// After create succeeds (or the file exists), submit its asynchronous NAND open and release this callback.
// Consumes self; on create failure it also frees context. Successful create requires
// non-null context. file remains caller-owned; path belongs to self until cleanup.
// Completion is (rawResult, phase, file, file->callbackArgument): phase 0 reports
// create failure/open submission here; the open callback later reports phase 1.
// User callbacks must leave self/file alive for the remaining accesses and cleanup.
// Allocation failures are not handled. Keep existing cleanup behavior on each branch.
void gtl_CCallBackCreateFile_OnCreateResult(void *self, int createResult, void *context) {
    void *callbackObject = self;
    int savedCreateResult = createResult;
    void *savedContext = context;

    if (createResult != 0 && createResult != kNandAlreadyExists) {
        // No open will be submitted. Finish the file state, notify phase 0 and consume self.
        // This path does not explicitly free path; do not fold it into the cleanup below.
        if (context != 0) {
            FileIO_InvokeAllocatorCallback3(context);
        }
        {
            void *fileState = AsGtlCreateFileCallback(self)->file;
            AsFmemFileState(fileState)->operationState = kFileOperationComplete;
            void *completionCallback = AsGtlCreateFileCallback(self)->completionCallback;
            if (completionCallback != 0) {
                void *callbackFile = AsGtlCreateFileCallback(self)->file;
                int userArgument = AsFmemFileState(callbackFile)->callbackArgument;
                ((void (*)(int, int, void *, int))completionCallback)(savedCreateResult, 0, callbackFile, userArgument);
            }
            ((DeletingCallback*)self)->destroy(-1);
            FileIO_InvokeAllocatorCallback3(self);
        }
        return;
    }

    // Capture the file and completion in the callback that will survive this create callback.
    void *file = AsGtlCreateFileCallback(self)->file;
    void *completion = AsGtlCreateFileCallback(self)->completionCallback;

    void *openCallback = FileIO_InvokeAllocatorCallback2(sizeof(FmemOperationCallback));
    if (openCallback != 0) {
        try {
        void *vtable = (void *)__vt__Q23gtl17CCallBackOpenFile;
        AsFmemOperationCallback(openCallback)->vtable = vtable;
        *(void**)&AsFmemOperationCallback(openCallback)->argument = file;
        *(void**)&AsFmemOperationCallback(openCallback)->callbackPayload = completion;
        } catch (...) {
            throw;
        }
    }

    // Reuse the create context for the open request and register its dispatch header.
    ((NandCallbackContext*)context)->callback = openCallback;
    void *commandBlock = ((NandCallbackContext*)context)->commandBlock;
    ((NandCallbackContext*)context)->owner = context;
    ((NandCallbackContext*)context)->headerBytes = kCallbackHeaderBytes;
    NANDSetUserData(commandBlock, context);

    void *path = AsGtlCreateFileCallback(self)->path;
    void *openFile = AsGtlCreateFileCallback(self)->file;
    unsigned char accessMode = AsGtlCreateFileCallback(self)->accessMode;
    void *dispatchCallback = (void *)gtl_CFileIoDeviceFMEM_DispatchNandCallback;
    int result = NANDOpenAsync(path, openFile, accessMode, dispatchCallback, commandBlock);

    // Submission failure destroys the newly created operation callback immediately.
    // context is not freed here; preserve that behavior rather than adding a new owner.
    if (result != 0) {
        File_SetLastError(kAsyncOpenSubmitError);
        void *fileState = AsGtlCreateFileCallback(self)->file;
        AsFmemFileState(fileState)->operationState = kFileOperationComplete;
        ((DeletingCallback*)openCallback)->destroy(-1);
        FileIO_InvokeAllocatorCallback3(openCallback);
    } else {
        File_SetLastError(0);
    }

    // Report submission, including zero on success, before releasing path and self.
    void *completionCallback = AsGtlCreateFileCallback(self)->completionCallback;
    if (completionCallback != 0) {
        void *fileState = AsGtlCreateFileCallback(self)->file;
        int userArgument = AsFmemFileState(fileState)->callbackArgument;
        ((void (*)(int, int, void *, int))completionCallback)(result, 0, fileState, userArgument);
    }

    // The pending open retains file/context/openCallback, not this path or create callback.
    void *ownedPath = AsGtlCreateFileCallback(self)->path;
    if (ownedPath != 0) {
        FileIO_InvokeAllocatorCallback3(ownedPath);
        AsGtlCreateFileCallback(self)->path = 0;
    }

    ((DeletingCallback*)self)->destroy(-1);
    FileIO_InvokeAllocatorCallback3(self);
}
#pragma use_lmw_stmw reset

} // extern "C"
