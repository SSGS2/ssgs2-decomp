// Translation unit 0x8042001C-0x80420154 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/PadMemDevice.h"
#include "sdk_protos.h"

extern "C" {

void File_SetLastError(int code, void* p);
int PadMemoryDriver_Read(void* dev, void* a2, unsigned short a3, unsigned short a5, int a4);

enum { kPadTransferLimit = 0xF30, kInvalidHandleError = 6, kPadDeviceErrorBase = 0xC738 };

// Submit a PAD-memory read, map driver status to last-error, and report the capped byte count on success.
// handle is only checked against -1; the device comes from self, not handle.
// buffer is borrowed writable storage. bytesRead, when non-null, points to an int
// and is left untouched on failure. Returns 1 on success, 0 on error.
// requestedBytes/deviceOffset are truncated to 16 bits at the driver boundary;
// their valid ranges and the driver's offset unit are not enforced here.
int gtl_CFileIoDevicePADMEM_ReadBytes(void* self, void* handle, void* buffer, int requestedBytes, void* bytesRead, int deviceOffset) {
    void* checkedHandle = handle;
    if (checkedHandle == (void*)-1) {
        File_SetLastError(kInvalidHandleError, buffer);
        return 0;
    }
    // Submit synchronously (final argument zero), then translate the error domain.
    int driverStatus = PadMemoryDriver_Read(AsPadMemDevice(self)->device, buffer, (unsigned short)requestedBytes, (unsigned short)deviceOffset, 0);
    int fileError;
    switch (driverStatus) {
    case 0: fileError = 0; break;
    case -1: fileError = kPadDeviceErrorBase + 1; break;
    case -2: fileError = kPadDeviceErrorBase + 2; break;
    case -3: fileError = kPadDeviceErrorBase + 3; break;
    case -4: fileError = kPadDeviceErrorBase + 4; break;
    case -5: fileError = kPadDeviceErrorBase + 5; break;
    case -6: fileError = kPadDeviceErrorBase + 6; break;
    case -7: fileError = kPadDeviceErrorBase + 7; break;
    default: fileError = kPadDeviceErrorBase + 2; break;
    }
    File_SetLastError(fileError, 0);
    if (driverStatus != 0) return 0;
    // Only successful reads publish a count; this is not a driver-reported size.
    if (bytesRead != 0) {
        // The driver receives the original 16-bit length; only the reported count is capped here.
        *(int*)bytesRead = (unsigned int)requestedBytes > (unsigned int)kPadTransferLimit ? kPadTransferLimit : requestedBytes;
    }
    File_SetLastError(0, 0);
    return 1;
}

} // extern "C"
