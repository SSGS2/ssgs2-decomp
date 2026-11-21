#include "types.h"
#include "classes/FmemFileState.h"

extern "C" {

void File_SetLastError(int x);

enum FmemQueryConstants { kInvalidFmemHandle = 0xFFFFFFFFu, kFileInvalidHandleError = 6 };

// Report whether the file's operation-state word is nonzero; return query success.
// fileHandle is borrowed; outState must be writable even for an invalid handle.
// Invalid handle: write 0, set error 6, return 0. Valid handle: write 0/1 and return 1;
// success leaves last error unchanged. The nonzero state's exact meaning is unresolved.
int gtl_CFileIoDeviceFMEM_QueryHasOperationState(void* self, void* fileHandle, int* outState) {
    (void)self;
    *outState = 0;
    if ((u32)fileHandle == kInvalidFmemHandle) {
        File_SetLastError(kFileInvalidHandleError);
        return 0;
    }
    if ((u32)AsFmemFileState(fileHandle)->operationState != 0) {
        *outState = 1;
    }
    return 1;
}

} // extern "C"
