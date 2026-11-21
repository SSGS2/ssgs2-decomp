// Translation unit 0x8041988C-0x80419A74 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {

void File_SetLastError(int x);
int NANDWrite(void *a, void *b, void *c);

// Preserve the sparse NAND-result map; names identify numeric codes without
// claiming unresolved SDK error meanings. -7 is intentionally not a case.
enum NandFileError {
    kFileSuccess = 0, kInvalidHandleError = 6, kNandErrorBase = 0xC350,
    kNandErrorMinus64 = 0xC360, kNandErrorMinus128 = 0xC361
};

// Write through NAND and optionally report the nonnegative byte count returned by it.
// handle/buffer are borrowed; byteCount retains the existing pointer-shaped ABI type.
// Returns 1 for any nonnegative result (including a short write), 0 on failure.
// bytesWrittenOut may be null and remains untouched on failure; only -1 is rejected.
int gtl_CFileIoDeviceFMEM_Write(void *self, void *handle, void *buffer, void *byteCount, int *bytesWrittenOut) {
    if (handle == (void *)-1) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    // Phase 1: submit the borrowed buffer, then translate negative driver results.
    int bytesWritten = NANDWrite(handle, buffer, byteCount);
    if (bytesWritten < 0) {
        int fileError;
        switch (bytesWritten) {
            case 0: fileError = kFileSuccess; break;
            case -1: fileError = kNandErrorBase + 1; break;
            case -2: fileError = kNandErrorBase + 2; break;
            case -3: fileError = kNandErrorBase + 3; break;
            case -4: fileError = kNandErrorBase + 4; break;
            case -5: fileError = kNandErrorBase + 5; break;
            case -6: fileError = kNandErrorBase + 6; break;
            case -8: fileError = kNandErrorBase + 8; break;
            case -9: fileError = kNandErrorBase + 9; break;
            case -10: fileError = kNandErrorBase + 10; break;
            case -11: fileError = kNandErrorBase + 11; break;
            case -12: fileError = kNandErrorBase + 12; break;
            case -13: fileError = kNandErrorBase + 13; break;
            case -14: fileError = kNandErrorBase + 14; break;
            case -64: fileError = kNandErrorMinus64; break;
            case -15: fileError = kNandErrorBase + 15; break;
            case -128: fileError = kNandErrorMinus128; break;
            default: fileError = kNandErrorMinus64; break;
        }
        File_SetLastError(fileError);
        return 0;
    }
    // Phase 2: publish the actual count only after the driver succeeds.
    if (bytesWrittenOut) {
        *bytesWrittenOut = bytesWritten;
    }
    File_SetLastError(kFileSuccess);
    return 1;
}

} // extern "C"
