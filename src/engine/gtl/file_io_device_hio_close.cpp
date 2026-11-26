// Host file device, CloseFile: closes the host stream held by a handle from OpenFile and frees
// that handle. The invalid handle returns 0 and sets the file layer's last error.

#include "types.h"
#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceHIO.h"

extern "C" {

void FileIO_Free(void* p);
int File_SetLastError(int x);
int fclose(void* p);

enum {
    kInvalidHostHandleError = 6
};

} // extern "C"

// handle is returned by OpenFile: (void*)-1 is the failure sentinel; otherwise
// it must be a valid non-null wrapper whose first word is the stream pointer.
// The device is not used. Returns 1 only when fclose returns zero. The sentinel returns 0
// and sets last-error to 6; the ordinary path does not explicitly update last-error.
int gtl::CFileIoDeviceHIO::CloseFile(void* handle) {
    if (handle == (void*)-1) {
        File_SetLastError(kInvalidHostHandleError);
        return 0;
    }
    int closeResult = fclose(*(void**)handle);
    int succeeded = (closeResult == 0) ? 1 : 0;
    if (handle != 0) {
        FileIO_Free(handle);
    }
    return succeeded;
}
