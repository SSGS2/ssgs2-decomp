// Translation unit 0x80413AAC-0x80413AFC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/DvdDeviceFileState.h"
extern "C" {

void File_SetLastError(int error);

enum DvdQueryError { kDvdQuerySuccess = 0, kDvdQueryInvalidHandle = 6 };

// Read the DVD handle's stored query word; reject the invalid-handle sentinel first.
// The caller-facing meaning of this word needs confirmation before naming the shared field.
// file is borrowed and must be valid except for sentinel -1; null is not checked.
// Return 0 on invalid handle (last-error 6), otherwise return the raw +0x40 word
// and clear last-error. Thus a zero return alone cannot distinguish failure from data.
unsigned int gtl_CFileIoDeviceDVD_vfunc_31(void* self, void* file) {
    (void)self;
    if (file == (void*)-1) {
        File_SetLastError(kDvdQueryInvalidHandle);
        return 0;
    }
    File_SetLastError(kDvdQuerySuccess);
    return AsDvdDeviceFileState(file)->unk_40;
}

} // extern "C"
