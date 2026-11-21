#include "sdk_protos.h"
#include "classes/PadMemDevice.h"

struct PadMemTransferState {
    int handleWord;
    int completionState;
    int requestedBytes;
};

// One global in-flight request: these borrowed objects must survive completion.
// Submitting another request replaces this registration; no per-request queue is built here.
extern PadMemTransferState* g_padMemAsyncHandle;
extern int (*g_padMemCompletionCallback)(int, int, PadMemTransferState*, void*);
extern void* g_padMemIoCallbackData;

extern "C" void File_SetLastError(int x);
extern "C" void FileIO_OnAsyncResult(void*, int);
// Submit the PADMEM transfer; a zero result means the asynchronous request was accepted.
extern "C" int PadMemoryDriver_Read(void*, void*, unsigned int, unsigned int, void*);

extern "C" {

#pragma opt_lifetimes off
#pragma opt_common_subs off
enum {
    kInvalidHandleError = 6,
    kMinimumPadMemResult = -7,
    kPadMemErrorBase = 0xC738,
    kPadMemFallbackError = 0xC73A
};

// Register the completion callback, submit a PADMEM transfer, and translate submission errors.
// Returns 1 for accepted submission, 0 for failure; completion occurs separately.
// handle must reference writable transfer state (only -1 is rejected). buffer and
// callbackData remain borrowed. The driver sees the low 16 bits of length/offset,
// but requestedBytes records the full byteCount, with no clamp or validation.
int gtl_CFileIoDevicePADMEM_ReadAsync(void* self, void* handle, void* buffer, unsigned int byteCount, unsigned int deviceOffset, int /*unused*/, int callbackAddress, void* callbackData) {
    if (handle == (void*)-1) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }

    // Phase 1: install completion state before entering the driver.
    PadMemTransferState* transfer = (PadMemTransferState*)handle;
    transfer->completionState = 0;
    transfer->requestedBytes = 0;
    g_padMemAsyncHandle = transfer;
    g_padMemCompletionCallback = (int (*)(int, int, PadMemTransferState*, void*))callbackAddress;
    g_padMemIoCallbackData = callbackData;

    // Phase 2: submit with the same device-relative offset as the synchronous read.
    int status = PadMemoryDriver_Read(AsPadMemDevice(self)->device, buffer, (unsigned short)byteCount, (unsigned short)deviceOffset, (void*)FileIO_OnAsyncResult);

    int submissionResult = status;
    int fileError;
    // Preserve the original indexed switch: individual PADMEM error meanings are unresolved.
    switch (submissionResult - kMinimumPadMemResult) {
        case 7: fileError = 0; break;
        case 6: fileError = kPadMemErrorBase + 1; break;
        case 5: fileError = kPadMemErrorBase + 2; break;
        case 4: fileError = kPadMemErrorBase + 3; break;
        case 3: fileError = kPadMemErrorBase + 4; break;
        case 2: fileError = kPadMemErrorBase + 5; break;
        case 1: fileError = kPadMemErrorBase + 6; break;
        case 0: fileError = kPadMemErrorBase + 7; break;
        default: fileError = kPadMemFallbackError; break;
    }
    File_SetLastError(fileError);

    if (submissionResult != 0) {
        return 0;
    }

    // Phase 3: publish the requested count only after acceptance. Registration
    // remains installed on failure too; this routine does not restore earlier globals.
    transfer->requestedBytes = (int)byteCount;
    File_SetLastError(0);
    return 1;
}

#pragma opt_lifetimes reset
#pragma opt_common_subs reset

} // extern "C"
