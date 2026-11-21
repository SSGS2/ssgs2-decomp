// Translation unit 0x804206E4-0x804207D4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct Unk804206E4 {
    int unk0; // +0: handle word, untouched here; interpretation unresolved.
    int completionReady; // +4: completion-ready flag (0 on submission, 1 on completion).
    int requestedBytesOrZero; // +8: requested byte count on acceptance, cleared on error.
};

Unk804206E4* g_padMemAsyncHandle;
int (*g_padMemCompletionCallback)(int, int, Unk804206E4*, void*);
void* g_padMemIoCallbackData;

// PADMEM status meanings are unresolved; names identify the original numeric status.
enum {
    kCompletionReady = 1, kPadMemStatusBias = 7,
    kPadMemSuccess = 0,
    kPadMemStatusMinus1 = 0xC739, kPadMemStatusMinus2 = 0xC73A,
    kPadMemStatusMinus3 = 0xC73B, kPadMemStatusMinus4 = 0xC73C,
    kPadMemStatusMinus5 = 0xC73D, kPadMemStatusMinus6 = 0xC73E,
    kPadMemStatusMinus7 = 0xC73F
};

// Publish completion, translate PADMEM status, invoke the callback and clear its context.
// Requires one outstanding operation and a live, non-null g_padMemAsyncHandle.
// Callback receives (fileError, requestedBytesOrZero, borrowedHandle, borrowedData).
// Returns its result when installed, otherwise the translated error; does not set
// File_SetLastError. Context is cleared AFTER callback: submissions must be serialized
// and the callback must not install another operation in the same global slot.
int PadMemoryIo_CompleteAsyncOperation(int, int resultCode) {
    int completionResult;
    Unk804206E4* completedHandle = g_padMemAsyncHandle;
    completedHandle->completionReady = kCompletionReady;
    if (resultCode != 0) {
        completedHandle->requestedBytesOrZero = 0;
    }

    // Bias [-7,0] to switch indices [0,7]; all other statuses map to the -2 error.
    unsigned int statusIndex = (unsigned int)(resultCode + kPadMemStatusBias);
    switch (statusIndex) {
        case 7: completionResult = kPadMemSuccess; break;
        case 6: completionResult = kPadMemStatusMinus1; break;
        case 5: completionResult = kPadMemStatusMinus2; break;
        case 4: completionResult = kPadMemStatusMinus3; break;
        case 3: completionResult = kPadMemStatusMinus4; break;
        case 2: completionResult = kPadMemStatusMinus5; break;
        case 1: completionResult = kPadMemStatusMinus6; break;
        case 0: completionResult = kPadMemStatusMinus7; break;
        default: completionResult = kPadMemStatusMinus2; break;
    }

    if (g_padMemCompletionCallback != 0) {
        int requestedBytesOrZero = completedHandle->requestedBytesOrZero;
        completionResult = g_padMemCompletionCallback(completionResult, requestedBytesOrZero, g_padMemAsyncHandle, g_padMemIoCallbackData);
    }

    g_padMemAsyncHandle = 0;
    g_padMemCompletionCallback = 0;
    g_padMemIoCallbackData = 0;

    return completionResult;
}

} // extern "C"
