#include "types.h"
#include "sdk_protos.h"

extern "C" {

void File_SetLastError(int x);
void *fwrite(void *a, int b, unsigned int c, void *d);

enum HostWriteConstants { kInvalidHandleError = 6, kByteElementSize = 1 };

// handle must point to a stream pointer; only -1 is rejected (null is dereferenced).
// The legacy fwrite declaration represents its byte-count return as void*.
// Return 0 for sentinel -1 from fwrite, 1 for every other result, including short writes.
// bytesWrittenOut is optional and unchanged on failure. Success does not clear last-error.
int gtl_CFileIoDeviceHIO_Write(void *self, void **handle, void *buffer, unsigned int byteCount, unsigned int *bytesWrittenOut) {
    if (handle == (void **)-1) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    void *writtenCountWord = fwrite(buffer, kByteElementSize, byteCount, *handle);
    if (writtenCountWord == (void *)-1) {
        return 0;
    }
    if (bytesWrittenOut != 0) {
        *bytesWrittenOut = (unsigned int)writtenCountWord;
    }
    return 1;
}

} // extern "C"
