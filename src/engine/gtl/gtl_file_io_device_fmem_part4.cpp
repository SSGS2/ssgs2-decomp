// GTL FMEM device operation result: returns the result stored in an FMEM file handle,
// without waiting for the operation to finish.

#include "classes/FmemFileState.h"
#include "types.h"
#include "classes/GtlFileIoDeviceFMEM.h"

extern "C" void File_SetLastError(int x);

// Return the FMEM file handle's stored operation result without waiting or consuming it.
// handle borrows FmemFileState storage. Only the all-bits-set sentinel is checked;
// that error sets last-error 6 and returns 0, which is also a possible valid result.
// Poll operationState separately before relying on the result: open does not
// initialize it, and failed async seeks leave the previous result unchanged.
int gtl::CFileIoDeviceFMEM::GetOperationResult(void* handle) {
    enum {
        kInvalidHandle = 0xFFFFFFFFu,
        kInvalidHandleError = 6
    };
    if ((u32)handle == kInvalidHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    return AsFmemFileState(handle)->operationResult;
}
