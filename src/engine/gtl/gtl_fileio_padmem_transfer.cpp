// Translation unit 0x80420154-0x80420288 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/PadMemDevice.h"
#include "sdk_protos.h"

extern "C" void File_SetLastError(int x);
extern "C" int PadMemory_TransferBlock(void* device, void* buffer, unsigned short byteCount, int, int);

extern "C" {

enum PadTransferConstants {
    kInvalidHandle = -1, kInvalidHandleError = 6, kMaxTransferBytes = 0xF30,
    kResultSwitchBias = 7, kTransferOk = 0,
    kPadError1 = 0xC739, kPadError2 = 0xC73A, kPadError3 = 0xC73B,
    kPadError4 = 0xC73C, kPadError5 = 0xC73D, kPadError6 = 0xC73E, kPadError7 = 0xC73F
};

// Transfer at most one PAD-memory block and report the byte count only on driver success.
// fileHandle is checked only against -1; the actual transfer uses self->device.
// buffer is borrowed. Returns 1 on driver result 0, otherwise 0 and a translated last error.
// transferredBytes may be null and is left untouched on failure. Large requests are
// truncated to one 0xF30-byte block; the caller must loop to transfer the remainder.
int gtl_CFileIoDevicePADMEM_TransferOneBlock(void* self, unsigned int fileHandle, void* buffer, unsigned int requestedBytes, int* transferredBytes) {
    if (fileHandle == (unsigned int)kInvalidHandle) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }
    // Phase 1: clamp the byte count before narrowing to the driver's 16-bit length.
    unsigned int transferBytes = kMaxTransferBytes;
    if (requestedBytes <= kMaxTransferBytes) {
        transferBytes = requestedBytes;
    }
    void* device = AsPadMemDevice(self)->device;
    int driverResult = PadMemory_TransferBlock(device, buffer, (unsigned short)transferBytes, 0, 0);
    int operationResult = driverResult;
    // Phase 2: translate results -1..-7 to 0xC739..0xC73F; other failures use error 2.
    // Adding seven is a switch-index transformation, not a change to the driver result.
    int code;
    switch (driverResult + kResultSwitchBias) {
        case 7: code = kTransferOk; break;
        case 6: code = kPadError1; break;
        case 5: code = kPadError2; break;
        case 4: code = kPadError3; break;
        case 3: code = kPadError4; break;
        case 2: code = kPadError5; break;
        case 1: code = kPadError6; break;
        case 0: code = kPadError7; break;
        default: code = kPadError2; break;
    }
    File_SetLastError(code);
    if (operationResult != kTransferOk) {
        return 0;
    }
    // Phase 3: publish the clamped count only after successful completion.
    if (transferredBytes != 0) {
        *transferredBytes = (int)transferBytes;
    }
    File_SetLastError(kTransferOk);
    return 1;
}

} // extern "C"
