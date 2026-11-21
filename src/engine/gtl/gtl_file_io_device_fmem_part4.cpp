// Translation unit 0x8041D6EC-0x8041D728 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/FmemFileState.h"
#include "types.h"

extern "C" {

void File_SetLastError(int x);

enum { kInvalidHandle = 0xFFFFFFFFu, kInvalidHandleError = 6 };

// Return the FMEM file handle's stored operation result without waiting or consuming it.
// handle borrows FmemFileState storage. Only the all-bits-set sentinel is checked;
// that error sets last-error 6 and returns 0, which is also a possible valid result.
// Poll operationState separately before relying on the result: open does not
// initialize it, and failed async seeks leave the previous result unchanged.
int gtl_CFileIoDeviceFMEM_GetOperationResult(void* self, void* handle) {
    (void)self;
    if ((u32)handle == kInvalidHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    return AsFmemFileState(handle)->operationResult;
}

} // extern "C"
