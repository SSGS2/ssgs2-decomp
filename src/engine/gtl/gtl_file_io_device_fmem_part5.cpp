// Translation unit 0x8041D730-0x8041D76C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

extern "C" {
int NANDGetCurrentDir(void *);

enum { kNandCurrentDirectoryBufferBytes = 0x40 };

// Copy the current NAND directory into a borrowed buffer of at least 64 bytes.
// Returns 1 only for SDK success; returns 0 for a short buffer or SDK failure.
// A short buffer is untouched. No null check or file-layer last-error update occurs.
int gtl_CFileIoDeviceFMEM_GetCurrentDirectory(void *self, void *directoryBuffer, u32 bufferBytes) {
    if (bufferBytes < kNandCurrentDirectoryBufferBytes) {
        return 0;
    }
    int nandStatus = NANDGetCurrentDir(directoryBuffer);
    return (nandStatus == 0);
}
} // extern "C"
