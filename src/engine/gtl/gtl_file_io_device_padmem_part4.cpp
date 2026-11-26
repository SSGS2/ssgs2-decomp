// PADMEM file I/O device: the IsHandleValid method, which accepts every handle except the
// invalid sentinel value.

#include "sdk_protos.h"
#include "classes/GtlFileIoDevicePADMEM.h"

extern "C" void File_SetLastError(int x);

enum PadMemHandleStatus {
    kPadMemNoError = 0,
    kPadMemInvalidHandle = 6
};
const unsigned int kInvalidPadMemHandle = 0xFFFFFFFFu;

// Report success for every handle word except the invalid sentinel; no I/O or release.
// Returns 0/error 6 for the sentinel, 1/error 0 otherwise (including zero).
// The corresponding DVD/FMEM slot closes a file; PADMEM only checks the sentinel.
int gtl::CFileIoDevicePADMEM::IsHandleValid(unsigned int handleWord) {
    if (handleWord == kInvalidPadMemHandle) {
        File_SetLastError(kPadMemInvalidHandle);
        return 0;
    }
    File_SetLastError(kPadMemNoError);
    return 1;
}
