// Translation unit 0x804196A4-0x8041988C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {

void File_SetLastError(int x);
int NANDRead(void *a, void *b, void *c);

// Error names retain numeric NAND status until the individual SDK meanings are established.
enum NandReadErrors {
    kInvalidHandleError = 6, kNandErrorBase = 0xC350,
    kNandStatusMinus64 = 0xC360, kNandStatusMinus128 = 0xC361
};

// Read synchronously into borrowed destination; byteCountWord carries a count, not a pointer.
// Returns 1 on any nonnegative NAND result (including short/zero reads), 0 on error.
// bytesRead may be null; on failure it is left unchanged. Only handle -1 is rejected.
// self is unused; the caller supplies valid NAND handle/buffer/alignment for the SDK.
int gtl_CFileIoDeviceFMEM_Read(void *self, void *fileHandle, void *destination, void *byteCountWord, int *bytesRead) {
    if (fileHandle == (void *)-1) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    int nandResult = NANDRead(fileHandle, destination, byteCountWord);
    if (nandResult < 0) {
        // Translate status to the file layer; -7 and other unlisted values use the -64 code.
        // Preserve the original switch, including its unreachable zero case.
        int fileError;
        switch (nandResult) {
            case 0: fileError = 0; break;
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
            case -64: fileError = kNandStatusMinus64; break;
            case -15: fileError = kNandErrorBase + 15; break;
            case -128: fileError = kNandStatusMinus128; break;
            default: fileError = kNandStatusMinus64; break;
        }
        File_SetLastError(fileError);
        return 0;
    }
    // Publish transferred bytes only after success; a short read is still successful.
    if (bytesRead) {
        *bytesRead = nandResult;
    }
    File_SetLastError(0);
    return 1;
}

} // extern "C"
