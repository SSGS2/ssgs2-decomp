// HIO file I/O device: the Flush method, which flushes the stdio stream behind a handle.
// It reports success or failure and does not close the handle.

#include "types.h"
#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceHIO.h"

extern "C" {

int File_SetLastError(int x);
int fflush(void* p);

enum {
    kInvalidHandleError = 6
};

} // extern "C"

// Flush the FILE* stored at the start of a borrowed HIO handle.
// Handle -1 returns -1 and sets error 6. Otherwise return 1 if fflush succeeds,
// 0 if it fails; neither case updates the file layer's last error. Null handles
// are not accepted, the device is not used, and flushing does not close/free the handle.
int gtl::CFileIoDeviceHIO::Flush(void* fileHandle) {
    if (fileHandle == (void*)-1) {
        File_SetLastError(kInvalidHandleError);
        return -1;
    }
    int flushResult = fflush(*(void**)fileHandle);
    return (flushResult == 0) ? 1 : 0;
}
