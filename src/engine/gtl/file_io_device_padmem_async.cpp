#include "sdk_protos.h"
#include "classes/PadMemDevice.h"

struct Unk804206E4 {
    int handleWord; // +0x0: untouched by submission and completion in these units.
    int completionReady; // +0x4: zero while pending, one in CompleteAsyncOperation.
    int acceptedTransferSize; // +0x8: requested bytes; completion replaces with zero on error.
};

// Status values are named by the external API result until their SDK meanings are established.
enum PadMemoryError {
    kPadMemoryNoError = 0, kPadMemoryInvalidHandle = 6,
    kPadMemoryStatusMinus1 = 0xC739, kPadMemoryStatusMinus2 = 0xC73A,
    kPadMemoryStatusMinus3 = 0xC73B, kPadMemoryStatusMinus4 = 0xC73C,
    kPadMemoryStatusMinus5 = 0xC73D, kPadMemoryStatusMinus6 = 0xC73E,
    kPadMemoryStatusMinus7 = 0xC73F
};
enum { kPadMemoryStatusBias = 7, kPadMemoryInvalidHandleWord = -1 };

extern Unk804206E4* g_padMemAsyncHandle;
extern int (*g_padMemCompletionCallback)(int, int, Unk804206E4*, void*);
extern void* g_padMemIoCallbackData;

extern "C" void File_SetLastError(int x);
extern "C" int PadMemory_TransferBlock(void* a, void* b, unsigned short c, int d, int e);
extern "C" int PadMemoryIo_CompleteAsyncOperation(int, int);

extern "C" {

#pragma opt_lifetimes off
#pragma opt_common_subs off
// Submit asynchronous PAD memory I/O, publish completion state, and translate submission status.
// Borrows the handle/buffer until completion; only the sentinel -1 is checked (not null).
// One global operation slot: callers must serialize submissions. Callback data is borrowed.
// Returns 1 for accepted submission, 0 for an immediate error; completion is reported separately.
// Completion calls (translatedError, acceptedBytesOrZero, handle, callbackData), then clears global context.
// The driver gets only the low 16 bits of size, but the full requested size is published on success.
int gtl_CFileIoDevicePADMEM_BeginAsyncTransfer(void* self, void* fileHandle, void* buffer, unsigned int size, int /*unused*/, int completionCallback, void* callbackData) {
    if (fileHandle == (void*)kPadMemoryInvalidHandleWord) {
        File_SetLastError(kPadMemoryInvalidHandle);
        return 0;
    }

    // Publish callback context before entering the driver; it may be consulted by completion.
    Unk804206E4* handle = (Unk804206E4*)fileHandle;
    handle->completionReady = 0;
    handle->acceptedTransferSize = 0;
    g_padMemAsyncHandle = handle;
    g_padMemCompletionCallback = (int (*)(int, int, Unk804206E4*, void*))completionCallback;
    g_padMemIoCallbackData = callbackData;

    int status = PadMemory_TransferBlock(AsPadMemDevice(self)->device, buffer, (unsigned short)size, 0, (int)PadMemoryIo_CompleteAsyncOperation);

    // Translate driver status (-7..0) to the file layer's error domain.
    int submissionStatus = status;
    int translatedError;
    switch (submissionStatus + kPadMemoryStatusBias) {
        case 7: translatedError = kPadMemoryNoError; break;
        case 6: translatedError = kPadMemoryStatusMinus1; break;
        case 5: translatedError = kPadMemoryStatusMinus2; break;
        case 4: translatedError = kPadMemoryStatusMinus3; break;
        case 3: translatedError = kPadMemoryStatusMinus4; break;
        case 2: translatedError = kPadMemoryStatusMinus5; break;
        case 1: translatedError = kPadMemoryStatusMinus6; break;
        case 0: translatedError = kPadMemoryStatusMinus7; break;
        default: translatedError = kPadMemoryStatusMinus2; break;
    }
    File_SetLastError(translatedError);

    // Rejected submissions leave the published global context in place; there is no rollback here.
    if (submissionStatus != 0) {
        return 0;
    }

    // Accepted submission records requested bytes, not a measured completed transfer size.
    handle->acceptedTransferSize = (int)size;
    File_SetLastError(kPadMemoryNoError);
    return 1;
}

#pragma opt_lifetimes reset
#pragma opt_common_subs reset

} // extern "C"
