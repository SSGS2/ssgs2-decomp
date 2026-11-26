// Host file device, GetFileSize: returns the length in bytes of an open host file. It finds the
// length by seeking to the end of the stream, then seeks back to the saved position. The high
// word output is set to zero when it is given. An invalid handle returns -1 and sets the last error.

#include "types.h"
#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceHIO.h"

extern "C" {

void File_SetLastError(int x);
void fseek(void* stream, void* byteOffset, int origin);

struct HostFilePositionView {
    char fileStatePrefix[0x18];
    void* position; // Preserve the original 32-bit ABI representation.
};
struct HostFileHandle {
    HostFilePositionView* stream;
};
enum {
    kSeekFromStart = 0,
    kSeekFromEnd = 2,
    kInvalidFileHandleError = 6
};

} // extern "C"

// rejected. Return the low 32-bit byte length as an ABI word, or all bits set on
// invalid handle. sizeHigh is optional and remains untouched on that error.
// Neither seek status is checked; this reports the stream's stored position.
void* gtl::CFileIoDeviceHIO::GetFileSize(HostFileHandle* file, int* sizeHigh) {
    if (file == (HostFileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return (void*)-1;
    }
    HostFilePositionView* stream = file->stream;
    void* savedPosition = stream->position;
    // Query through the FILE position field, then restore the borrowed stream.
    fseek(stream, 0, kSeekFromEnd);
    stream = file->stream;
    void* fileLength = stream->position;
    fseek(stream, savedPosition, kSeekFromStart);
    if (sizeHigh != 0) {
        *sizeHigh = 0;
    }
    return fileLength;
}
