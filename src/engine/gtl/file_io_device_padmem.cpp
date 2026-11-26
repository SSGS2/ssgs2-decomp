// PAD memory file device: reports whether a file handle's storage word is nonzero. It clears the
// flag output first, returns 1 for a valid handle, and sets the file layer's last error for an
// invalid handle.

#include "types.h"
#include "classes/GtlFileIoDevicePADMEM.h"

extern "C" void File_SetLastError(int x);

enum {
    kInvalidPadMemHandle = -1,
    kInvalidHandleError = 6
};

// Report whether the handle's entire +4 storage word is nonzero; return 1 for a valid handle.
// hasNonzeroStorageWord must be writable: it is cleared even before rejecting handle -1.
// Error 6 accompanies an invalid handle; success leaves last error unchanged.
// This tests all string-header bits, including inline text/length, not just heap
// ownership bit 31. The wider purpose of this virtual slot remains unresolved.
int gtl::CFileIoDevicePADMEM::vfunc_30(void* handle, int* hasNonzeroStorageWord) {
    *hasNonzeroStorageWord = 0;
    if ((u32)handle == (u32)kInvalidPadMemHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    if (((gtl::CFileIoDevicePADMEM*)handle)->path.flags != 0) { // 0x80420B9C: tests the entire +4 word
        *hasNonzeroStorageWord = 1;
    }
    return 1;
}
