// GTL FMEM device current-directory query: copies the NAND current directory into a
// caller buffer, which must be at least 64 bytes.

#include "types.h"
#include "classes/GtlFileIoDeviceFMEM.h"

extern "C" int NANDGetCurrentDir(void*);

// Copy the current NAND directory into a borrowed buffer of at least 64 bytes.
// Returns 1 only for SDK success; returns 0 for a short buffer or SDK failure.
// A short buffer is untouched. No null check or file-layer last-error update occurs.
int gtl::CFileIoDeviceFMEM::GetCurrentDirectory(void* directoryBuffer, u32 bufferBytes) {
    enum {
        kNandCurrentDirectoryBufferBytes = 0x40
    };
    if (bufferBytes < kNandCurrentDirectoryBufferBytes) {
        return 0;
    }
    int nandStatus = NANDGetCurrentDir(directoryBuffer);
    return (nandStatus == 0);
}
