// Translation unit 0x80420498-0x804205A4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void File_SetLastError(int);

extern void* g_padMemAsyncHandle;
extern void* g_padMemCompletionCallback;
extern void* g_padMemIoCallbackData;

enum PadMemCompletionConstants {
    kCompletionReady = 1, kCompletionStateOffset = 4, kAcceptedBytesOffset = 8,
    kDriverStatusBias = 7, kPadMemErrorBase = 0xC738
};

// Translate a PADMEM completion, notify the caller, then release the global operation slot.
// Requires a live g_padMemAsyncHandle; submissions must serialize this single slot.
// Callback arguments: (fileError, requestedBytesOrZero, borrowedHandle, borrowedUserData).
// The byte count comes from submission, not the driver. Callback return is ignored.
void FileIO_OnAsyncResult(void* /*unused*/, int driverStatus) {
    // Publish completion before translating status or entering user code.
    int completionReady = kCompletionReady;
    unsigned int selector = (unsigned int)driverStatus + kDriverStatusBias;
    void* transfer = g_padMemAsyncHandle;
    *(int*)((char*)transfer + kCompletionStateOffset) = completionReady;

    // Status 0 succeeds; -1..-7 map in order. Other values use the -2 error.
    // The individual SDK error meanings have not been established.
    int fileError;
    switch (selector) {
    case 7: fileError = 0; break;
    case 6: fileError = kPadMemErrorBase + 1; break;
    case 5: fileError = kPadMemErrorBase + 2; break;
    case 4: fileError = kPadMemErrorBase + 3; break;
    case 3: fileError = kPadMemErrorBase + 4; break;
    case 2: fileError = kPadMemErrorBase + 5; break;
    case 1: fileError = kPadMemErrorBase + 6; break;
    case 0: fileError = kPadMemErrorBase + 7; break;
    default: fileError = kPadMemErrorBase + 2; break;
    }

    File_SetLastError(fileError);

    if (fileError != 0) {
        *(int*)((char*)transfer + kAcceptedBytesOffset) = 0;
    }

    // Context stays published during notification. A reentrant submission here
    // would be cleared by the cleanup below; the callback must not reuse the slot.
    void (*completionCallback)(int, int, void*, void*) = (void (*)(int, int, void*, void*))g_padMemCompletionCallback;
    if (completionCallback) {
        completionCallback(fileError, *(int*)((char*)transfer + kAcceptedBytesOffset), g_padMemAsyncHandle, g_padMemIoCallbackData);
    }

    // Drop borrowed context only; neither the handle nor user data is freed.
    g_padMemAsyncHandle = 0;
    g_padMemCompletionCallback = 0;
    g_padMemIoCallbackData = 0;
}

} // extern "C"
