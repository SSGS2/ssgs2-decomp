// File-memory device: creates a NAND directory, either synchronously or as an asynchronous request
// with a completion callback. Also maps NAND status codes to file error codes.

#include "classes/GtlFileIoDeviceFMEM.h"
#include "types.h"
#include "sdk_protos.h"

#include "classes/FmemOperationCallback.h"
extern "C" {
extern void File_SetLastError(int x);
extern void* FileIO_Alloc(unsigned int size);
extern void FileIO_Free(void* p);
extern int NANDCreateDir(void* a, int b, int c);
extern int NANDCreateDirAsync(void* a, int b, int c, void* d, void* e);
extern void NANDSetUserData(void*, void*);

extern char __vt__Q23gtl24CCallBackCreateDirectory[];
extern char s_fmemAsyncOperation[];
extern char s_nandAsyncCommandBlock[];

struct CB {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
};

enum DirectoryCreateConstants {
    kPathBufferBytes = 0x104,
    kOwnerReadWrite = 0x30,
    kNoAttributes = 0,
    kCallbackBytes = 0xC,
    kCallbackDestroySlot = 3
};

// SDK NAND status -> file-layer last-error code. -7 has no explicit mapping;
// unhandled statuses (including -16/MAXDEPTH) use kFileUnknown.
enum DirectoryCreateStatus {
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
enum DirectoryCreateError {
    kFileOk = 0,
    kFileAccess = 0xC351,
    kFileAllocFailed = 0xC352,
    kFileBusy = 0xC353,
    kFileCorrupt = 0xC354,
    kFileEccCritical = 0xC355,
    kFileExists = 0xC356,
    kFileInvalid = 0xC358,
    kFileMaxBlocks = 0xC359,
    kFileMaxFds = 0xC35A,
    kFileMaxFiles = 0xC35B,
    kFileNoEntry = 0xC35C,
    kFileNotEmpty = 0xC35D,
    kFileOpenFd = 0xC35E,
    kFileAuthentication = 0xC35F,
    kFileUnknown = 0xC360,
    kFileFatal = 0xC361
};

} // extern "C"

// Create a NAND directory synchronously, or submit it with a completion callback.
// path must contain '/' and fit, including its terminator, in kPathBufferBytes;
// only the suffix beginning at the FIRST slash is passed to NAND (e.g. a device prefix is skipped).
// completionAddress == 0 selects synchronous execution; otherwise it is the Wii
// address of void completion(int rawNandResult, int userArgument), NOT a boolean flag.
// Return 1 for sync success / async submission, 0 for failure; last error describes
// the immediate NAND result. Async completion owns the allocated callback wrapper.
// Async calls require the global command block/operation record to be idle.
// Callback allocation failure is not converted into a clean failure return:
// a null wrapper is still submitted (or dereferenced during rejection cleanup).
int gtl::CFileIoDeviceFMEM::CreateDirectory(const char* path, int completionAddress, int userArgument) {
    char pathBuffer[kPathBufferBytes];
    strncpy(pathBuffer, path, kPathBufferBytes);
    char* nandPath = strchr(pathBuffer, '/');
    if (completionAddress == 0) {
        // Complete the operation before translating the raw NAND status.
        int nandResult = NANDCreateDir(nandPath, kOwnerReadWrite, kNoAttributes);
        int savedResult = nandResult;
        int fileError;
        switch (nandResult) {
        case kNandOk:
            fileError = kFileOk;
            break;
        case kNandAccess:
            fileError = kFileAccess;
            break;
        case kNandAllocFailed:
            fileError = kFileAllocFailed;
            break;
        case kNandBusy:
            fileError = kFileBusy;
            break;
        case kNandCorrupt:
            fileError = kFileCorrupt;
            break;
        case kNandEccCritical:
            fileError = kFileEccCritical;
            break;
        case kNandExists:
            fileError = kFileExists;
            break;
        case kNandInvalid:
            fileError = kFileInvalid;
            break;
        case kNandMaxBlocks:
            fileError = kFileMaxBlocks;
            break;
        case kNandMaxFds:
            fileError = kFileMaxFds;
            break;
        case kNandMaxFiles:
            fileError = kFileMaxFiles;
            break;
        case kNandNoEntry:
            fileError = kFileNoEntry;
            break;
        case kNandNotEmpty:
            fileError = kFileNotEmpty;
            break;
        case kNandOpenFd:
            fileError = kFileOpenFd;
            break;
        case kNandAuthentication:
            fileError = kFileAuthentication;
            break;
        case kNandUnknown:
            fileError = kFileUnknown;
            break;
        case kNandFatal:
            fileError = kFileFatal;
            break;
        default:
            fileError = kFileUnknown;
            break;
        }
        File_SetLastError(fileError);
        if (savedResult == 0) {
            return 1;
        }
        return 0;
    } else {
        // Publish the callback and its arguments before submitting the shared command block.
        void* callback = FileIO_Alloc(kCallbackBytes);
        if (callback != 0) {
            AsFmemOperationCallback(callback)->vtable = (void*)__vt__Q23gtl24CCallBackCreateDirectory;
            AsFmemOperationCallback(callback)->argument = completionAddress;
            AsFmemOperationCallback(callback)->callbackPayload = userArgument;
        }
        *(void**)s_fmemAsyncOperation = callback;
        *(int*)(s_fmemAsyncOperation + 4) = 0;
        *(int*)(s_fmemAsyncOperation + 8) = 0;
        NANDSetUserData(s_nandAsyncCommandBlock, &s_fmemAsyncOperation);
        int nandResult =
            NANDCreateDirAsync(nandPath, kOwnerReadWrite, kNoAttributes,
                               (void*)gtl::CFileIoDeviceFMEM::DispatchNandCallback, s_nandAsyncCommandBlock);
        int savedResult = nandResult;
        int fileError;
        // Preserve the same translation as the synchronous path; completion uses raw status.
        switch (nandResult) {
        case kNandOk:
            fileError = kFileOk;
            break;
        case kNandAccess:
            fileError = kFileAccess;
            break;
        case kNandAllocFailed:
            fileError = kFileAllocFailed;
            break;
        case kNandBusy:
            fileError = kFileBusy;
            break;
        case kNandCorrupt:
            fileError = kFileCorrupt;
            break;
        case kNandEccCritical:
            fileError = kFileEccCritical;
            break;
        case kNandExists:
            fileError = kFileExists;
            break;
        case kNandInvalid:
            fileError = kFileInvalid;
            break;
        case kNandMaxBlocks:
            fileError = kFileMaxBlocks;
            break;
        case kNandMaxFds:
            fileError = kFileMaxFds;
            break;
        case kNandMaxFiles:
            fileError = kFileMaxFiles;
            break;
        case kNandNoEntry:
            fileError = kFileNoEntry;
            break;
        case kNandNotEmpty:
            fileError = kFileNotEmpty;
            break;
        case kNandOpenFd:
            fileError = kFileOpenFd;
            break;
        case kNandAuthentication:
            fileError = kFileAuthentication;
            break;
        case kNandUnknown:
            fileError = kFileUnknown;
            break;
        case kNandFatal:
            fileError = kFileFatal;
            break;
        default:
            fileError = kFileUnknown;
            break;
        }
        File_SetLastError(fileError);
        if (savedResult != 0) {
            // Submission failed: destroy the wrapper here; no completion will reclaim it.
            typedef void (*VFn)(void*);
            VFn destroyCallback = (VFn)(*(void***)callback)[kCallbackDestroySlot];
            destroyCallback(callback);
            return 0;
        }
        return 1;
    }
}

extern "C" {} // extern "C"
