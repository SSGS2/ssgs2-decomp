// Translation unit 0x804190A4-0x804190EC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

extern "C" {

int File_SetLastError(int x);
int fflush(void *p);

enum { kInvalidHandleError = 6 };

// Flush the FILE* stored at the start of a borrowed HIO handle.
// Handle -1 returns -1 and sets error 6. Otherwise return 1 if fflush succeeds,
// 0 if it fails; neither case updates the file layer's last error. Null handles
// are not accepted, self is unused, and flushing does not close/free the handle.
int gtl_CFileIoDeviceHIO_Flush(void *self, void *fileHandle) {
    if (fileHandle == (void *)-1) {
        File_SetLastError(kInvalidHandleError);
        return -1;
    }
    int flushResult = fflush(*(void **)fileHandle);
    return (flushResult == 0) ? 1 : 0;
}

} // extern "C"
