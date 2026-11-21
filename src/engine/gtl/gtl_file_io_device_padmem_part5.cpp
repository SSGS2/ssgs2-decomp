// Translation unit 0x80420BF4-0x80420C30 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" void File_SetLastError(int x);

extern "C" {

struct PadMemoryHandle {
    void* vtable;
    int completionReady;  // +4: 0 pending, 1 completion-ready; not inspected here.
    int acceptedTransferBytes; // +8: accepted transfer byte count, zeroed on completion error.
};
enum { kInvalidHandle = 0xFFFFFFFFu, kInvalidHandleError = 6 };

// Return the PAD-memory handle's operation result, reporting an invalid sentinel handle.
// handle borrows live PadMemoryHandle storage; only the all-bits-set sentinel
// is checked. It sets last-error 6 and returns 0, also a possible valid result.
// This neither waits nor clears the result; callers must establish completion.
// Async submitters retain the FULL requested byte count here although the driver
// receives its low 16 bits; this is not the translated PAD error code. Open sets 0.
int gtl_CFileIoDevicePADMEM_GetTransferBytes(void* self, void* handle) {
    (void)self;
    if ((u32)handle == kInvalidHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    return ((PadMemoryHandle*)handle)->acceptedTransferBytes;
}

} // extern "C"
