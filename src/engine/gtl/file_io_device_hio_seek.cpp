// Host file device, SeekFile: moves the position of an open host file. Origin 1 is relative to the
// current position, 2 to the end, and any other value to the start. The high word output is cleared
// on success. A failed seek or an invalid handle returns -1 and sets the file layer's last error.

#include "types.h"
#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceHIO.h"

extern "C" {

void File_SetLastError(int x);
int fseek(void* stream, int byteOffset, unsigned int origin);

enum {
    kSeekFromStart = 0,
    kSeekFromCurrent = 1,
    kSeekFromEnd = 2,
    kInvalidHandleError = 6,
    kSeekFailedError = 0x19
};

} // extern "C"

// file's first word is the stream; only the -1 handle sentinel is checked.
// byteOffset carries a signed 32-bit byte displacement in a pointer-typed ABI word.
// origin 1 = current, 2 = end, all other values = start. highWord is optional:
// success clears it; errors leave it untouched. The return is not a queried position.
int gtl::CFileIoDeviceHIO::SeekFile(void** file, void* byteOffset, unsigned int* highWord, unsigned int origin) {
    if (file == (void**)-1) {
        File_SetLastError(kInvalidHandleError);
        return -1;
    }
    unsigned int seekOrigin = kSeekFromStart;
    if (origin == kSeekFromCurrent) {
        seekOrigin = kSeekFromCurrent;
    }
    if (origin == kSeekFromEnd) {
        seekOrigin = kSeekFromEnd;
    }
    int seekStatus = fseek(*file, (int)byteOffset, seekOrigin);
    if (seekStatus < 0) {
        File_SetLastError(kSeekFailedError);
        return -1;
    }
    if (highWord != 0) {
        *highWord = 0;
    }
    return seekStatus;
}
