#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceFMEM.h"

extern "C" {
extern char s_fmemAsyncOperation[];
extern char s_nandAsyncCommandBlock[];
extern char __vt__Q23gtl22CCallBackDiskFreeSpace[];
void File_SetLastError(int);
int NAND_QueryFileSystemStats(void*, void*);
int NANDChangeDir(void*);
void* FileIO_Alloc(int);
void NANDSetUserData(void*, void*);
int NAND_QueryFileSystemStatsAsync(void*, void*, void*, void*);
int FileIoDevice_AccumulateDirectoryAllocationSize(void*, void*, void*);
void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void*, void*);

enum DiskSpaceQueryConstants {
    kDeviceCapacityBytes = 0x01000000, // Fixed 16 MiB budget for the device's cached directory.
    kRequestDirectoryFreeBytes = 0x1,
    kRequestDeviceCapacity = 0x10,
    kRequestNandStatWord = 0x100,
    kFileNandErrorBase = 0xC350,
    kFileNandUnknownError = 0xC360,
    kFileNandFatalError = 0xC361,
    kDiskSpaceCallbackBytes = 0x10
};
// NAND status -N maps to 0xC350+N for N=1..6,8..15. -64 and unknown
// statuses map to 0xC360; -128 maps to 0xC361. Keep each SDK call's mapping
// beside its failure path. A successful query finally clears the last error.

#pragma push
#pragma use_lmw_stmw on
// Query optional space outputs synchronously, or submit a shared NAND callback operation.
// Each nonnull output addresses two u32 words {high=0, low=value}; directoryFree
// is capacity minus recursive allocation (clamped), deviceCapacity is fixed, and
// nandStat receives the SDK's first result word (its units are unresolved here).
// callback=0 selects synchronous writes. Otherwise output pointers select request
// bits only; callback/userData are retained in a heap wrapper, not invoked here.
// Returns 1 for synchronous success or accepted submission, 0 for immediate failure.
// Async callers must serialize the shared command/context and ensure allocation
// succeeds. The existing SDK call uses stack result words; their async lifetime
// is unresolved and deliberately preserved rather than silently repaired.
int gtl_CFileIoDeviceFMEM_QueryDiskSpace(void* self, void* /*unused*/, void* directoryFree, void* deviceCapacity, void* nandStat, int callback, int userData) {
    if (callback == 0) {
        // Phase 1: optionally query the raw NAND stat; later outputs remain untouched on failure.
        if (nandStat != 0) {
            int nandSecondWord = 0;
            int nandFirstWord = 0;
            int nandStatus = NAND_QueryFileSystemStats(&nandFirstWord, &nandSecondWord);
            int fileError;
            switch (nandStatus) {
            case 0: fileError = 0; break;
            case -1: fileError = kFileNandErrorBase + 1; break;
            case -2: fileError = kFileNandErrorBase + 2; break;
            case -3: fileError = kFileNandErrorBase + 3; break;
            case -4: fileError = kFileNandErrorBase + 4; break;
            case -5: fileError = kFileNandErrorBase + 5; break;
            case -6: fileError = kFileNandErrorBase + 6; break;
            case -8: fileError = kFileNandErrorBase + 8; break;
            case -9: fileError = kFileNandErrorBase + 9; break;
            case -10: fileError = kFileNandErrorBase + 10; break;
            case -11: fileError = kFileNandErrorBase + 11; break;
            case -12: fileError = kFileNandErrorBase + 12; break;
            case -13: fileError = kFileNandErrorBase + 13; break;
            case -14: fileError = kFileNandErrorBase + 14; break;
            case -64: fileError = kFileNandUnknownError; break;
            case -15: fileError = kFileNandErrorBase + 15; break;
            case -128: fileError = kFileNandFatalError; break;
            default: fileError = kFileNandUnknownError; break;
            }
            File_SetLastError(fileError);
            if (nandStatus != 0) {
                return 0;
            }
            *(int*)nandStat = 0;
            *(int*)((char*)nandStat + 4) = nandFirstWord;
        }
        // Phase 2: publish the fixed device budget; this can precede a later directory failure.
        if (deviceCapacity != 0) {
            *(int*)deviceCapacity = 0;
            *(int*)((char*)deviceCapacity + 4) = kDeviceCapacityBytes;
        }
        // Phase 3: switch to the cached directory, accumulate allocation bytes, then publish free space.
        // NANDChangeDir changes process-wide NAND state; this method does not restore it.
        if (directoryFree != 0) {
            void* directoryPath;
            if ((AsGtlFileIoDeviceFMEM(self)->cachedCurrentDirectory.flags >> 31) == 0) {
                directoryPath = (char*)&AsGtlFileIoDeviceFMEM(self)->cachedCurrentDirectory + 1; // Short-string data begins one byte after its flag word (+0x11).
            } else {
                directoryPath = AsGtlFileIoDeviceFMEM(self)->cachedCurrentDirectory.data;
            }
            int changeDirStatus = NANDChangeDir(directoryPath);
            int changeDirError;
            switch (changeDirStatus) {
            case 0: changeDirError = 0; break;
            case -1: changeDirError = kFileNandErrorBase + 1; break;
            case -2: changeDirError = kFileNandErrorBase + 2; break;
            case -3: changeDirError = kFileNandErrorBase + 3; break;
            case -4: changeDirError = kFileNandErrorBase + 4; break;
            case -5: changeDirError = kFileNandErrorBase + 5; break;
            case -6: changeDirError = kFileNandErrorBase + 6; break;
            case -8: changeDirError = kFileNandErrorBase + 8; break;
            case -9: changeDirError = kFileNandErrorBase + 9; break;
            case -10: changeDirError = kFileNandErrorBase + 10; break;
            case -11: changeDirError = kFileNandErrorBase + 11; break;
            case -12: changeDirError = kFileNandErrorBase + 12; break;
            case -13: changeDirError = kFileNandErrorBase + 13; break;
            case -14: changeDirError = kFileNandErrorBase + 14; break;
            case -64: changeDirError = kFileNandUnknownError; break;
            case -15: changeDirError = kFileNandErrorBase + 15; break;
            case -128: changeDirError = kFileNandFatalError; break;
            default: changeDirError = kFileNandUnknownError; break;
            }
            File_SetLastError(changeDirError);
            int directoryQuerySucceeded;
            if (changeDirStatus != 0) {
                directoryQuerySucceeded = 0;
            } else {
                unsigned int allocatedBytes[2] = { 0, 0 };
                void* allocationRootPath;
                if ((AsGtlFileIoDeviceFMEM(self)->cachedCurrentDirectory.flags >> 31) == 0) {
                    allocationRootPath = (char*)&AsGtlFileIoDeviceFMEM(self)->cachedCurrentDirectory + 1;
                } else {
                    allocationRootPath = AsGtlFileIoDeviceFMEM(self)->cachedCurrentDirectory.data;
                }
                directoryQuerySucceeded = FileIoDevice_AccumulateDirectoryAllocationSize(self, allocationRootPath, allocatedBytes);
                // allocatedBytes is {high, low}; the calculation uses only the low word.
                if (directoryQuerySucceeded != 0) {
                    if (allocatedBytes[1] > (unsigned int)kDeviceCapacityBytes) {
                        allocatedBytes[1] = (unsigned int)kDeviceCapacityBytes;
                    }
                    *(int*)directoryFree = 0;
                    *(int*)((char*)directoryFree + 4) = (int)((unsigned int)kDeviceCapacityBytes - allocatedBytes[1]);
                }
            }
            if (directoryQuerySucceeded == 0) {
                return 0;
            }
        }
    } else {

        // Phase 4: retain request bits and completion arguments in the owned callback wrapper.
        int requestMask = 0;
        if (directoryFree != 0) requestMask |= kRequestDirectoryFreeBytes;
        if (deviceCapacity != 0) requestMask |= kRequestDeviceCapacity;
        if (nandStat != 0) requestMask |= kRequestNandStatWord;
        void* callbackWrapper = FileIO_Alloc(kDiskSpaceCallbackBytes);
        if (callbackWrapper != 0) {
            *(void**)callbackWrapper = __vt__Q23gtl22CCallBackDiskFreeSpace;
            *((int*)callbackWrapper + 1) = requestMask;
            *((int*)callbackWrapper + 2) = callback;
            *((int*)callbackWrapper + 3) = userData;
        }
        // Phase 5: publish {callback, word1=0, word2=0}, then submit the shared command.
        char* operation = s_fmemAsyncOperation;
        *(void**)operation = callbackWrapper;
        *(int*)(operation + 4) = 0;
        *(int*)(operation + 8) = 0;
        NANDSetUserData(s_nandAsyncCommandBlock, &s_fmemAsyncOperation);
        int asyncFirstWord = 0;
        int asyncSecondWord = 0;
        int submissionStatus = NAND_QueryFileSystemStatsAsync(&asyncFirstWord, &asyncSecondWord, (void*)gtl_CFileIoDeviceFMEM_DispatchNandCallback, s_nandAsyncCommandBlock);
        int submissionError;
        switch (submissionStatus) {
        case 0: submissionError = 0; break;
        case -1: submissionError = kFileNandErrorBase + 1; break;
        case -2: submissionError = kFileNandErrorBase + 2; break;
        case -3: submissionError = kFileNandErrorBase + 3; break;
        case -4: submissionError = kFileNandErrorBase + 4; break;
        case -5: submissionError = kFileNandErrorBase + 5; break;
        case -6: submissionError = kFileNandErrorBase + 6; break;
        case -8: submissionError = kFileNandErrorBase + 8; break;
        case -9: submissionError = kFileNandErrorBase + 9; break;
        case -10: submissionError = kFileNandErrorBase + 10; break;
        case -11: submissionError = kFileNandErrorBase + 11; break;
        case -12: submissionError = kFileNandErrorBase + 12; break;
        case -13: submissionError = kFileNandErrorBase + 13; break;
        case -14: submissionError = kFileNandErrorBase + 14; break;
        case -64: submissionError = kFileNandUnknownError; break;
        case -15: submissionError = kFileNandErrorBase + 15; break;
        case -128: submissionError = kFileNandFatalError; break;
        default: submissionError = kFileNandUnknownError; break;
        }
        File_SetLastError(submissionError);
        // A rejected submission destroys the wrapper locally; accepted work owns it via dispatch.
        if (submissionStatus != 0) {
            void** callbackVtable = *(void***)callbackWrapper;
            ((void(*)(void*))callbackVtable[3])(callbackWrapper);
            return 0;
        }

    }
    File_SetLastError(0);
    return 1;
}
#pragma pop

} // extern "C"
