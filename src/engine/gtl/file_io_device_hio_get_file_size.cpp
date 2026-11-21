// Translation unit 0x80418F58-0x80418FEC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

extern "C" {

void File_SetLastError(int x);
void fseek(void *stream, void *byteOffset, int origin);

struct HostFilePositionView {
    char fileStatePrefix[0x18];
    void* position; // Preserve the original 32-bit ABI representation.
};
struct HostFileHandle { HostFilePositionView* stream; };
enum { kSeekFromStart = 0, kSeekFromEnd = 2, kInvalidFileHandleError = 6 };

// rejected. Return the low 32-bit byte length as an ABI word, or all bits set on
// invalid handle. sizeHigh is optional and remains untouched on that error.
// Neither seek status is checked; this reports the stream's stored position.
void *CFileIoDeviceHIO_GetFileSize(void *self, HostFileHandle *file, int *sizeHigh) {
    if (file == (HostFileHandle *)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return (void *)-1;
    }
    HostFilePositionView *stream = file->stream;
    void *savedPosition = stream->position;
    // Query through the FILE position field, then restore the borrowed stream.
    fseek(stream, 0, kSeekFromEnd);
    stream = file->stream;
    void *fileLength = stream->position;
    fseek(stream, savedPosition, kSeekFromStart);
    if (sizeHigh != 0) {
        *sizeHigh = 0;
    }
    return fileLength;
}

} // extern "C"
