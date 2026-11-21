// Translation unit 0x80420B9C-0x80420BF4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "classes/PadMemDevice.h"

extern "C" void File_SetLastError(int x);

extern "C" {

enum { kInvalidPadMemHandle = -1, kInvalidHandleError = 6 };

// Report whether the handle's entire +4 storage word is nonzero; return 1 for a valid handle.
// hasNonzeroStorageWord must be writable: it is cleared even before rejecting handle -1.
// Error 6 accompanies an invalid handle; success leaves last error unchanged.
// This tests all string-header bits, including inline text/length, not just heap
// ownership bit 31. The wider purpose of this virtual slot remains unresolved.
int gtl_CFileIoDevicePADMEM_vfunc_30(void* self, void* handle, int* hasNonzeroStorageWord) {
    (void)self;
    *hasNonzeroStorageWord = 0;
    if ((u32)handle == (u32)kInvalidPadMemHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    if (AsPadMemDevice(handle)->nameStorageWord != 0) { // 0x80420B9C: tests the entire +4 word
        *hasNonzeroStorageWord = 1;
    }
    return 1;
}

} // extern "C"
