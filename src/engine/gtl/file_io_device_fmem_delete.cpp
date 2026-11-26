// GtlFileIoDeviceFMEM::DeleteFile: deletes a file from NAND memory, either at once or asynchronously,
// and maps the NAND result codes to file error codes.

#include "classes/GtlFileIoDeviceFMEM.h"
#include "classes/FmemOperationCallback.h"
#include "types.h"

extern "C" {

void File_SetLastError(int x);
char* strncpy(char* dst, const char* src, unsigned int n);
char* strchr(const char* s, int c);
int NANDDelete(const char*);
int NANDDeleteAsync(void* a, void* b, void* c);
void NANDSetUserData(void*, void*);
void* FileIO_Alloc(unsigned int size);

extern char __vt__Q23gtl19CCallBackDeleteFile[];
extern char s_fmemAsyncOperation[];
extern char s_nandAsyncCommandBlock[];

// Result identities follow revolution/NAND/nand.h; the file layer has a distinct error domain.
enum NandDeleteResult {
    kNandOk = 0,
    kNandAccess = -1,
    kNandAlloc = -2,
    kNandBusy = -3,
    kNandCorrupt = -4,
    kNandEccCritical = -5,
    kNandExists = -6,
    kNandInvalid = -8,
    kNandMaxBlocks = -9,
    kNandMaxDescriptors = -10,
    kNandMaxFiles = -11,
    kNandNoExists = -12,
    kNandNotEmpty = -13,
    kNandOpenFiles = -14,
    kNandAuthentication = -15,
    kNandUnknown = -64,
    kNandFatal = -128
};
enum FileDeleteError {
    kDeleteOk = 0,
    kDeleteAccess = 0xC351,
    kDeleteAlloc = 0xC352,
    kDeleteBusy = 0xC353,
    kDeleteCorrupt = 0xC354,
    kDeleteEccCritical = 0xC355,
    kDeleteExists = 0xC356,
    kDeleteInvalid = 0xC358,
    kDeleteMaxBlocks = 0xC359,
    kDeleteMaxDescriptors = 0xC35A,
    kDeleteMaxFiles = 0xC35B,
    kDeleteNoExists = 0xC35C,
    kDeleteNotEmpty = 0xC35D,
    kDeleteOpenFiles = 0xC35E,
    kDeleteAuthentication = 0xC35F,
    kDeleteUnknown = 0xC360,
    kDeleteFatal = 0xC361
};
enum {
    kDeletePathBytes = 0x104,
    kDeleteCallbackBytes = 0xC,
    kCallbackReleaseSlot = 3
};

} // extern "C"

// Delete a NAND file now, or queue deletion when completionCallback is nonzero.
// path is borrowed, must fit (including NUL) in 0x104 bytes and contain '/'; only
// its suffix from the first slash reaches NAND. No validation or forced NUL is added.
// Returns 1 for synchronous success or accepted async submission, 0 for failure.
// Async completion calls completionCallback(result, callbackContext).
// All FMEM async operations share one command/operation block: callers must serialize them.
int gtl::CFileIoDeviceFMEM::DeleteFile(char* path, int completionCallback, int callbackContext) {
    // Phase 1: strip the device prefix using a stack copy valid through the submission call.
    char pathBuffer[kDeletePathBytes];
    strncpy(pathBuffer, path, kDeletePathBytes);
    char* nandPath = strchr(pathBuffer, '/');

    if (completionCallback == 0) {
        // Phase 2a: complete synchronously and translate the SDK result to the file error domain.
        int nandResult = NANDDelete(nandPath);
        int lastError;
        switch ((int)nandResult) {
        case kNandOk:
            lastError = kDeleteOk;
            break;
        case kNandAccess:
            lastError = kDeleteAccess;
            break;
        case kNandAlloc:
            lastError = kDeleteAlloc;
            break;
        case kNandBusy:
            lastError = kDeleteBusy;
            break;
        case kNandCorrupt:
            lastError = kDeleteCorrupt;
            break;
        case kNandEccCritical:
            lastError = kDeleteEccCritical;
            break;
        case kNandExists:
            lastError = kDeleteExists;
            break;
        case kNandInvalid:
            lastError = kDeleteInvalid;
            break;
        case kNandMaxBlocks:
            lastError = kDeleteMaxBlocks;
            break;
        case kNandMaxDescriptors:
            lastError = kDeleteMaxDescriptors;
            break;
        case kNandMaxFiles:
            lastError = kDeleteMaxFiles;
            break;
        case kNandNoExists:
            lastError = kDeleteNoExists;
            break;
        case kNandNotEmpty:
            lastError = kDeleteNotEmpty;
            break;
        case kNandOpenFiles:
            lastError = kDeleteOpenFiles;
            break;
        case kNandUnknown:
            lastError = kDeleteUnknown;
            break;
        case kNandAuthentication:
            lastError = kDeleteAuthentication;
            break;
        case kNandFatal:
            lastError = kDeleteFatal;
            break;
        default:
            lastError = kDeleteUnknown;
            break;
        }
        File_SetLastError(lastError);
        if (nandResult == kNandOk) {
            return 1;
        }
        return 0;
    } else {
        // Phase 2b: allocate the callback object and publish {callback, no buffer, zero bytes}.
        // argument holds completionCallback's function address; callbackPayload holds callbackContext.
        void* callback = FileIO_Alloc(kDeleteCallbackBytes);
        if (callback) {
            AsFmemOperationCallback(callback)->vtable = __vt__Q23gtl19CCallBackDeleteFile;
            AsFmemOperationCallback(callback)->argument = completionCallback;
            AsFmemOperationCallback(callback)->callbackPayload = callbackContext;
        }
        volatile char* operation = s_fmemAsyncOperation;
        *(void**)operation = callback;
        *(int*)(operation + 4) = 0;
        *(int*)(operation + 8) = 0;
        NANDSetUserData(s_nandAsyncCommandBlock, &s_fmemAsyncOperation);
        // Phase 3: translate submission status with the same mapping as synchronous deletion.
        // -7 and -16 (and other unlisted results) deliberately map to kDeleteUnknown.
        int nandResult =
            NANDDeleteAsync(nandPath, (void*)gtl::CFileIoDeviceFMEM::DispatchNandCallback, s_nandAsyncCommandBlock);
        int submissionResult = nandResult;
        int lastError;
        switch (nandResult) {
        case kNandOk:
            lastError = kDeleteOk;
            break;
        case kNandAccess:
            lastError = kDeleteAccess;
            break;
        case kNandAlloc:
            lastError = kDeleteAlloc;
            break;
        case kNandBusy:
            lastError = kDeleteBusy;
            break;
        case kNandCorrupt:
            lastError = kDeleteCorrupt;
            break;
        case kNandEccCritical:
            lastError = kDeleteEccCritical;
            break;
        case kNandExists:
            lastError = kDeleteExists;
            break;
        case kNandInvalid:
            lastError = kDeleteInvalid;
            break;
        case kNandMaxBlocks:
            lastError = kDeleteMaxBlocks;
            break;
        case kNandMaxDescriptors:
            lastError = kDeleteMaxDescriptors;
            break;
        case kNandMaxFiles:
            lastError = kDeleteMaxFiles;
            break;
        case kNandNoExists:
            lastError = kDeleteNoExists;
            break;
        case kNandNotEmpty:
            lastError = kDeleteNotEmpty;
            break;
        case kNandOpenFiles:
            lastError = kDeleteOpenFiles;
            break;
        case kNandUnknown:
            lastError = kDeleteUnknown;
            break;
        case kNandAuthentication:
            lastError = kDeleteAuthentication;
            break;
        case kNandFatal:
            lastError = kDeleteFatal;
            break;
        default:
            lastError = kDeleteUnknown;
            break;
        }
        File_SetLastError(lastError);
        // Phase 4: failed submission releases the callback locally; accepted work transfers
        // it to DispatchNandCallback. Allocation failure is not safely handled by this code.
        if (submissionResult != kNandOk) {
            ((void (**)(void*))(AsFmemOperationCallback(callback)->vtable))[kCallbackReleaseSlot](callback);
            return 0;
        }
        return 1;
    }
}
