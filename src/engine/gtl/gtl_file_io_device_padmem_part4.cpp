// Translation unit 0x80420B58-0x80420B9C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" void File_SetLastError(int x);

extern "C" {

enum PadMemHandleStatus { kPadMemNoError = 0, kPadMemInvalidHandle = 6 };
const unsigned int kInvalidPadMemHandle = 0xFFFFFFFFu;

// Report success for every handle word except the invalid sentinel; no I/O or release.
// Returns 0/error 6 for the sentinel, 1/error 0 otherwise (including zero).
// The corresponding DVD/FMEM slot closes a file; PADMEM only checks the sentinel.
int gtl_CFileIoDevicePADMEM_IsHandleValid(void* self, unsigned int handleWord) {
    (void)self;
    if (handleWord == kInvalidPadMemHandle) {
        File_SetLastError(kPadMemInvalidHandle);
        return 0;
    }
    File_SetLastError(kPadMemNoError);
    return 1;
}

} // extern "C"
