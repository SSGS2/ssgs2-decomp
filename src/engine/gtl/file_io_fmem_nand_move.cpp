// NAND file device (FMEM), asynchronous file move: queues a NAND move of a file into the directory
// named by the destination path. An immediate NAND status is translated into a file-layer error
// code. Completion is handled separately by DispatchNandCallback.

#include "classes/GtlFileIoDeviceFMEM.h"
#include "sdk_protos.h"

extern "C" {

void* memcpy(void*, const void*, unsigned int);

void NANDSetUserData(void*, void*);
int NAND_MoveAsync(char* a, char* b, void* c, void* d);
void* FileIO_Alloc(unsigned int size);
void File_SetLastError(void* x);

extern char __vt__Q23gtl29CCallBackMoveFileWithProgress[];
extern char nandMoveCurrentDirectoryText;
extern char s_fmemAsyncOperation[];
extern char s_nandAsyncCommandBlock[];

#pragma use_lmw_stmw on

enum NandMoveConstants {
    kNandPathBytes = 0x104,
    kMoveCallbackBytes = 8,
    kPathSlash = '/'
};
// Meanings follow NANDResult in revolution/NAND/nand.h; values are file-layer error codes.
enum NandMoveError {
    kNandMoveOk = 0,
    kNandMoveAccessDenied = 0xC351,
    kNandMoveAllocationFailed = 0xC352,
    kNandMoveBusy = 0xC353,
    kNandMoveCorrupt = 0xC354,
    kNandMoveEccCritical = 0xC355,
    kNandMoveAlreadyExists = 0xC356,
    kNandMoveInvalid = 0xC358,
    kNandMoveMaxBlocks = 0xC359,
    kNandMoveMaxDescriptors = 0xC35A,
    kNandMoveMaxFiles = 0xC35B,
    kNandMoveNotFound = 0xC35C,
    kNandMoveDirectoryNotEmpty = 0xC35D,
    kNandMoveFileOpen = 0xC35E,
    kNandMoveAuthentication = 0xC35F,
    kNandMoveUnknown = 0xC360,
    kNandMoveFatal = 0xC361
};

// Queue a NAND move into the destination's directory and translate immediate submission errors.
// Both borrowed paths must contain '/'; the prefix before the first slash is discarded.
// A trailing slash denotes a directory; otherwise the final component is stripped, not used to rename.
// Callers serialize the single global command/context and require callback allocation to succeed.
// Returns 1 when queued, 0 on immediate failure; successful queuing transfers wrapper cleanup to dispatch.
int gtl::CFileIoDeviceFMEM::MoveFileToDirectoryAsync(const char* sourcePath, const char* destinationPath,
                                                     int callbackArgument) {
    char destinationDirectory[kNandPathBytes];

    // Extract the NAND-relative source and destination directory without changing existing truncation.
    char* nandSource = strchr(sourcePath, kPathSlash);
    char* nandDestination = strchr(destinationPath, kPathSlash);
    char* lastSlash = strrchr(nandDestination, kPathSlash);

    char* destinationLeaf;
    if (strlen(lastSlash + 1) == 0) {
        destinationLeaf = 0;
    } else {
        destinationLeaf = lastSlash + 1;
    }

    if (destinationLeaf == 0) {
        // strncpy retains its original behavior: a full-size input need not be NUL-terminated.
        strncpy(destinationDirectory, nandDestination, kNandPathBytes);
    } else if (nandDestination == destinationLeaf) {
        // Retained legacy fallback; ordinary slash-containing strings cannot reach this comparison.
        strncpy(destinationDirectory, &nandMoveCurrentDirectoryText, kNandPathBytes);
    } else {
        size_t destinationLength = strlen(nandDestination);
        size_t leafLength = strlen(destinationLeaf);
        size_t directoryLength = destinationLength - (leafLength + 1);
        if (directoryLength > kNandPathBytes - 1) {
            directoryLength = kNandPathBytes - 1;
        }
        memcpy(destinationDirectory, nandDestination, (unsigned int)directoryLength);
        destinationDirectory[directoryLength] = 0;
    }

    // Allocate the two-word callback wrapper and publish it before submitting the shared command.
    void* callback = FileIO_Alloc(kMoveCallbackBytes);
    if (callback != 0) {
        *(void**)callback = __vt__Q23gtl29CCallBackMoveFileWithProgress;
        *((int*)callback + 1) = callbackArgument;
    }

    volatile char* operation = s_fmemAsyncOperation;
    *(void**)operation = callback;
    *(int*)(operation + 4) = 0; // Remaining two context words start at zero; roles unresolved here.
    *(int*)(operation + 8) = 0;
    NANDSetUserData(s_nandAsyncCommandBlock, &s_fmemAsyncOperation);

    int submissionStatus = NAND_MoveAsync(nandSource, destinationDirectory,
                                          (void*)gtl::CFileIoDeviceFMEM::DispatchNandCallback, s_nandAsyncCommandBlock);
    int savedStatus = submissionStatus;

    // Map only immediate NAND status; later completion is handled by DispatchNandCallback.
    int translatedError;
    switch (submissionStatus) {
    case 0:
        translatedError = kNandMoveOk;
        break;
    case -1:
        translatedError = kNandMoveAccessDenied;
        break;
    case -2:
        translatedError = kNandMoveAllocationFailed;
        break;
    case -3:
        translatedError = kNandMoveBusy;
        break;
    case -4:
        translatedError = kNandMoveCorrupt;
        break;
    case -5:
        translatedError = kNandMoveEccCritical;
        break;
    case -6:
        translatedError = kNandMoveAlreadyExists;
        break;
    case -8:
        translatedError = kNandMoveInvalid;
        break;
    case -9:
        translatedError = kNandMoveMaxBlocks;
        break;
    case -10:
        translatedError = kNandMoveMaxDescriptors;
        break;
    case -11:
        translatedError = kNandMoveMaxFiles;
        break;
    case -12:
        translatedError = kNandMoveNotFound;
        break;
    case -13:
        translatedError = kNandMoveDirectoryNotEmpty;
        break;
    case -14:
        translatedError = kNandMoveFileOpen;
        break;
    case -64:
        translatedError = kNandMoveUnknown;
        break;
    case -15:
        translatedError = kNandMoveAuthentication;
        break;
    case -128:
        translatedError = kNandMoveFatal;
        break;
    default:
        translatedError = kNandMoveUnknown;
        break;
    }
    File_SetLastError((void*)translatedError);

    // An immediate rejection owns cleanup locally; the global context is not reset here.
    if (savedStatus != 0) {
        ((void (**)(void*))(*(void**)callback))[3](callback); // Vtable +0x0c releases the wrapper.
        return 0;
    }
    return 1;
}

#pragma use_lmw_stmw off

} // extern "C"
