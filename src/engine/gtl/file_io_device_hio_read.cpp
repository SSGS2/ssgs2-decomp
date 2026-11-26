// Host file device, ReadBytes: reads up to a given number of bytes from an open host file. A short
// read still succeeds, and the bytes read are reported when a count output is given. A failed read
// or an invalid handle returns 0 and sets the file layer's last error.

#include "types.h"
#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceHIO.h"

extern "C" {

void File_SetLastError(int x);
void* fread(void* buffer, int elementBytes, unsigned int elementCount, void* stream);

enum {
    kInvalidHandleError = 6,
    kReadFailedError = 0x1e,
    kByteElementSize = 1
};

} // extern "C"

// bytesRead is optional and is written only on success. A short/zero read succeeds.
// The pointer-typed fread result preserves the recovered 32-bit ABI: it is a
// byte count, with all bits set treated as failure by this wrapper.
int gtl::CFileIoDeviceHIO::ReadBytes(void** file, void* buffer, unsigned int byteCount, unsigned int* bytesRead) {
    if (file == (void**)-1) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    void* readCount = fread(buffer, kByteElementSize, byteCount, *file);
    if (readCount == (void*)-1) {
        File_SetLastError(kReadFailedError);
        return 0;
    }
    if (bytesRead != 0) {
        *bytesRead = (unsigned int)readCount;
    }
    return 1;
}
