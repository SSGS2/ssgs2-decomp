// GTL DVD device seek: forwards a seek on a DVD file handle to the SDK with a fixed
// position mode, and records the result in the file layer's last error.

#include "classes/GtlFileIoDeviceDVD.h"

extern "C" {

void File_SetLastError(int error);
int fn_8034A774(unsigned int file, unsigned int offset, int mode);

enum {
    kInvalidDvdFile = 0xFFFFFFFFu,
    kDvdPositionMode2 = 2,
    kInvalidHandleError = 6,
    kSeekError = 25,
    kNoError = 0
};

} // extern "C"

// Forward a DVD position request with mode 2, clear the optional output and map errors.
// The device is unused; file is a borrowed handle word. The callee is not defined in
// current source, so mode 2's "from end" label remains unverified. offset is passed
// unchanged. clearedOutput receives only zero, not bytes read or a new position;
// it is untouched for an invalid handle, but cleared before a callee failure.
// Returns 0 on success, -1 on failure; sets the file layer's last-error either way.
int gtl::CFileIoDeviceDVD::Seek(unsigned int file, unsigned int offset, int* clearedOutput) {
    if (file == kInvalidDvdFile) {
        File_SetLastError(kInvalidHandleError);
        return -1;
    }

    if (clearedOutput != 0) {
        *clearedOutput = 0;
    }

    int result = fn_8034A774(file, offset, kDvdPositionMode2);
    if (result < 0) {
        File_SetLastError(kSeekError);
        return -1;
    }

    File_SetLastError(kNoError);
    return 0;
}
