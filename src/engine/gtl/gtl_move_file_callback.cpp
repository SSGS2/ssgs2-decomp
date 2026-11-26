// Move-file completion callback: translates the NAND result into a file error and notifies the client,
// then releases the callback object and returns the allocator callback's result.

#include "sdk_protos.h"
#include "classes/gtl_CCallBackMoveFileWithProgress.h"

extern "C" {
void File_SetLastError(int);
void* FileIO_InvokeAllocatorCallback3(void*);

// NAND result magnitudes 1..15 map to file errors 0xC351..0xC35F, except
// -7, which is deliberately unhandled. MAXDEPTH (-16) also takes the default.
enum MoveCompletionStatus {
    kNandOk = 0,
    kNandAccess = -1,
    kNandAllocFailed = -2,
    kNandBusy = -3,
    kNandCorrupt = -4,
    kNandEccCritical = -5,
    kNandExists = -6,
    kNandInvalid = -8,
    kNandMaxBlocks = -9,
    kNandMaxFds = -10,
    kNandMaxFiles = -11,
    kNandNoEntry = -12,
    kNandNotEmpty = -13,
    kNandOpenFd = -14,
    kNandAuthentication = -15,
    kNandUnknown = -64,
    kNandFatal = -128
};
enum MoveCompletionConstants {
    kFileNandErrorBase = 0xC350,
    kFileUnknownError = 0xC360,
    kFileFatalError = 0xC361,
    kNonDeletingTeardown = -1,
    kTeardownTableIndex = 2
};

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
} // extern "C"

// Translate move completion status, notify the client with zero progress, then dispose self.
// The object is consumed. Its callback must be non-null and must not retain pointers
// to the four stack-local word pairs. Their precise progress meanings are not known.
// Return the configured allocator callback 3's result after virtual non-deleting teardown.
void* gtl::CCallBackMoveFileWithProgress::OnNandResult(int status) {
    // Phase 1: preserve volatile zero-word staging and translate the NAND status.
    volatile int zeroWord0 = 0, zeroWord1 = 0, zeroWord2 = 0, zeroWord3 = 0, zeroWord4 = 0, zeroWord5 = 0,
                 zeroWord6 = 0, zeroWord7 = 0;
    int fileError;
    switch (status) {
    case kNandOk:
        fileError = 0;
        break;
    case kNandAccess:
        fileError = kFileNandErrorBase + 1;
        break;
    case kNandAllocFailed:
        fileError = kFileNandErrorBase + 2;
        break;
    case kNandBusy:
        fileError = kFileNandErrorBase + 3;
        break;
    case kNandCorrupt:
        fileError = kFileNandErrorBase + 4;
        break;
    case kNandEccCritical:
        fileError = kFileNandErrorBase + 5;
        break;
    case kNandExists:
        fileError = kFileNandErrorBase + 6;
        break;
    case kNandInvalid:
        fileError = kFileNandErrorBase + 8;
        break;
    case kNandMaxBlocks:
        fileError = kFileNandErrorBase + 9;
        break;
    case kNandMaxFds:
        fileError = kFileNandErrorBase + 10;
        break;
    case kNandMaxFiles:
        fileError = kFileNandErrorBase + 11;
        break;
    case kNandNoEntry:
        fileError = kFileNandErrorBase + 12;
        break;
    case kNandNotEmpty:
        fileError = kFileNandErrorBase + 13;
        break;
    case kNandOpenFd:
        fileError = kFileNandErrorBase + 14;
        break;
    case kNandUnknown:
        fileError = kFileUnknownError;
        break;
    case kNandAuthentication:
        fileError = kFileNandErrorBase + 15;
        break;
    case kNandFatal:
        fileError = kFileFatalError;
        break;
    default:
        fileError = kFileUnknownError;
        break;
    }

    // Phase 2: publish last error before invoking the client with four zero pairs.
    File_SetLastError(fileError);

    int progressPair0[2] = {zeroWord6, zeroWord7};
    int progressPair1[2] = {zeroWord4, zeroWord5};
    int progressPair2[2] = {zeroWord2, zeroWord3};
    int progressPair3[2] = {zeroWord0, zeroWord1};

    void (**clientCallback)(int*, int*, int*, int*, int, int, int, int, int) =
        (void (**)(int*, int*, int*, int*, int, int, int, int, int))&callback;
    (*clientCallback)((int*)progressPair0, (int*)progressPair1, (int*)progressPair2, (int*)progressPair3, 0, fileError,
                      0, 0, 0);

    // Phase 3: teardown through vtable byte offset +8, then configured disposal.
    // Callback 3 is externally supplied; preserve it rather than substituting FileIO_Free.
    ((void (**)(void*, int))(*(void***)this))[kTeardownTableIndex](this, kNonDeletingTeardown);

    return FileIO_InvokeAllocatorCallback3(this);
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw off
