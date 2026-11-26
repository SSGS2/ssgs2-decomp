// GTL FMEM device operation-state query: reports whether a file handle's operation-state
// word is nonzero.

#include "types.h"
#include "classes/FmemFileState.h"
#include "classes/GtlFileIoDeviceFMEM.h"

extern "C" {

void File_SetLastError(int x);

enum FmemQueryConstants {
    kInvalidFmemHandle = 0xFFFFFFFFu,
    kFileInvalidHandleError = 6
};
}

// Report whether the file's operation-state word is nonzero; return query success.
// fileHandle is borrowed; outState must be writable even for an invalid handle.
// Invalid handle: write 0, set error 6, return 0. Valid handle: write 0/1 and return 1;
// success leaves last error unchanged. The meaning of a nonzero state is not identified.
int gtl::CFileIoDeviceFMEM::QueryHasOperationState(void* fileHandle, int* outState) {
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
