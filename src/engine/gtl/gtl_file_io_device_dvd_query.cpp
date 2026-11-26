// DVD device: the query of the status word stored in a DVD file's state. An invalid handle is
// rejected and sets the last error.

#include "classes/DvdDeviceFileState.h"
#include "classes/GtlFileIoDeviceDVD.h"

extern "C" {

void File_SetLastError(int error);

enum DvdQueryError {
    kDvdQuerySuccess = 0,
    kDvdQueryInvalidHandle = 6
};

} // extern "C"

// Read the DVD handle's stored query word; reject the invalid-handle sentinel first.
// The caller-facing meaning of this word needs confirmation before naming the shared field.
// file is borrowed and must be valid except for sentinel -1; null is not checked.
// Return 0 on invalid handle (last-error 6), otherwise return the raw +0x40 word
// and clear last-error. Thus a zero return alone cannot distinguish failure from data.
unsigned int gtl::CFileIoDeviceDVD::vfunc_31(void* file) {
    if (file == (void*)-1) {
        File_SetLastError(kDvdQueryInvalidHandle);
        return 0;
    }
    File_SetLastError(kDvdQuerySuccess);
    return AsDvdDeviceFileState(file)->unk_40;
}
