// FMEM/NAND file length queries, 0x80419C8C..0x8041A0DC.
// Synchronous results return bytes; asynchronous requests use the shared operation
// and NAND command block, completed by DispatchNandCallback (gtl_file_io_device_hio_fmem.cpp).
// Modding/API lookup: TranslateNandLengthError maps SDK status to File_SetLastError;
// GetFileSize below owns submission and immediate-rejection cleanup. Callback
// layout is classes/FmemOperationCallback.h. No gameplay tuning constants here.

#include "classes/FmemOperationCallback.h"
#include "types.h"

// Global single-operation record: +0 callback, +4 owned length buffer, +8 byte count.
// DispatchNandCallback passes the buffer to the callback, which releases it.
struct FmemLengthOperation { void* callback; void* resultBuffer; int resultBytes; };
typedef char FmemLengthOperationSizeCheck[sizeof(FmemLengthOperation)==12 ? 1 : -1];
static const int kFmemResultBufferOffset=(int)&((FmemLengthOperation*)0)->resultBuffer;
// Destruction is virtual slot one in the operation callback interface.
struct FileLengthCallbackInterface { 
virtual void reserved0(); virtual void destroy(); virtual void reserved2(); virtual void reserved3();
};
static const unsigned int kInvalidFileHandle = 0xFFFFFFFFU;
static const int kFileLengthQueryFailed = -1;

// Borrow the one pending operation record; the byte anchor preserves its ABI address.
static inline FmemLengthOperation& LengthOperationAt(const volatile void* address) {
    return *(FmemLengthOperation*)address;
}
// Access the owned buffer slot while preserving the rejection path's address lifetime.
static inline void*& LengthBufferSlot(char* address) {
    return *(void**)(address + kFmemResultBufferOffset);
}

extern "C" {
void File_SetLastError(int x);
void* FileIO_Alloc(unsigned int size);
void FileIO_Free(void* p);
int NANDGetLength(int a, void* b);
int NANDGetLengthAsync(int a, void* b, void* c, void* d);
void NANDSetUserData(void* a, void* b);
void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void* self, void* arg);

extern char __vt__Q23gtl20CCallBackGetFileSize[];
extern char s_fmemAsyncOperation[];
extern char s_nandAsyncCommandBlock[];

// Negative Wii NAND SDK results. -7 is reserved, NAND_RESULT_INVALID is -8,
// and NAND_RESULT_OPENFD is -14. Keep the hole at -7: treating INVALID as -7
// shifts every category through OPENFD and misdescribes the game's error map.
// Authoritative numeric domain: include/revolution/NAND/nand.h::NANDResult.
// NAND_RESULT_MAXDEPTH (-16) exists but is unlisted here and maps to Unknown.
enum NandLengthStatus {
    kNandAccess = -1,
    kNandAllocation = -2,
    kNandBusy = -3,
    kNandCorrupt = -4,
    kNandEccCritical = -5,
    kNandExists = -6,
    kNandReserved7 = -7,
    kNandInvalid = -8,
    kNandMaxBlocks = -9,
    kNandMaxFds = -10,
    kNandMaxFiles = -11,
    kNandNoEntry = -12,
    kNandNotEmpty = -13,
    kNandOpenFds = -14,
    kNandAuthentication = -15,
    kNandUnknown = -64,
    kNandFatal = -128,
};
// Game-side error codes (50001..50017) stored with File_SetLastError for each NAND code above;
// 6 is the invalid-handle error. The compare ladders below are a hand-matched form of one table lookup:
// NAND result -> FileLengthError (anything unlisted falls to the unknown/default code).
enum FileLengthError {
    kFileSuccess = 0,
    kFileInvalidHandle = 6,
    kFileAccess = 50001,
    kFileAllocation = 50002,
    kFileBusy = 50003,
    kFileCorrupt = 50004,
    kFileEccCritical = 50005,
    kFileExists = 50006,
    kFileInvalid = 50008,
    kFileMaxBlocks = 50009,
    kFileMaxFds = 50010,
    kFileMaxFiles = 50011,
    kFileNoEntry = 50012,
    kFileNotEmpty = 50013,
    kFileOpenFds = 50014,
    kFileAuthentication = 50015,
    kFileUnknown = 50016,
    kFileFatal = 50017,
};

// Write the translated immediate NAND error; sync and async use this same mapping.
// lastError is caller-owned scratch; unlisted codes, including -7, map to kFileUnknown.
#pragma push
#pragma always_inline on
static inline void TranslateNandLengthError(int nandResult, int& lastError) {
    // Ordered comparisons retain the binary layout; labels below are the mapping table.
    if (nandResult != kNandMaxBlocks) {
        if (nandResult < kNandMaxBlocks) {
            if (nandResult == kNandAuthentication) goto errorAuthentication;
            if (nandResult < kNandAuthentication) {
                if (nandResult == kNandUnknown) goto errorUnknown;
                // Original 0x80419C8C: both bounds share the default result.
                if (nandResult < kNandUnknown) {
                    if (nandResult != kNandFatal) goto errorDefault;
                    goto errorFatal;
                }
                goto errorDefault;
            } else {
                if (nandResult == kNandNoEntry) goto errorNoEntry;
                if (nandResult < kNandNoEntry) {
                    if (nandResult < kNandNotEmpty) goto errorOpenFds;
                    goto errorNotEmpty;
                } else {
                    if (nandResult < kNandMaxFds) goto errorMaxFiles;
                    goto errorMaxFds;
                }
            }
        } else {
            if (nandResult == kNandBusy) goto errorBusy;
            if (nandResult < kNandBusy) {
                if (nandResult == kNandExists) goto errorExists;
                if (nandResult < kNandExists) {
                    if (nandResult < kNandReserved7) goto errorInvalid;
                    goto errorDefault;
                } else {
                    if (nandResult < kNandCorrupt) goto errorEccCritical;
                    goto errorCorrupt;
                }
            } else {
                if (nandResult != 0) {
                    if (nandResult >= 0) goto errorDefault;
                    if (nandResult < kNandAccess) goto errorAllocation;
                    goto errorAccess;
                }
            }

        }
        lastError = kFileSuccess;
        goto translated;
    errorAccess:
        lastError = kFileAccess;
        goto translated;
    errorAllocation:
        lastError = kFileAllocation;
        goto translated;
    errorBusy:
        lastError = kFileBusy;
        goto translated;
    errorCorrupt:
        lastError = kFileCorrupt;
        goto translated;
    errorEccCritical:
        lastError = kFileEccCritical;
        goto translated;
    errorExists:
        lastError = kFileExists;
        goto translated;
    errorInvalid:
        lastError = kFileInvalid;
        goto translated;
    }
    lastError = kFileMaxBlocks;
    goto translated;
errorMaxFds:
    lastError = kFileMaxFds;
    goto translated;
errorMaxFiles:
    lastError = kFileMaxFiles;
    goto translated;
errorNoEntry:
    lastError = kFileNoEntry;
    goto translated;
errorNotEmpty:
    lastError = kFileNotEmpty;
    goto translated;
errorOpenFds:
    lastError = kFileOpenFds;
    goto translated;
errorUnknown:
    lastError = kFileUnknown;
    goto translated;
errorAuthentication:
    lastError = kFileAuthentication;
    goto translated;
errorFatal:
    lastError = kFileFatal;
    goto translated;
errorDefault:
    lastError = kFileUnknown;
translated:
    ;
}
#pragma pop

// Return a synchronous byte length, or submit an asynchronous length query.
// fileHandle is a borrowed FmemFileState/NAND file-info address; self is unused.
// completionAddress == 0: return length or -1; optional outSyncWord becomes zero
// only on success. Otherwise return 0 for accepted submission, -1 for rejection,
// and leave outSyncWord untouched. Last-error describes the immediate NAND result.
// completionAddress is void callback(int nandResult, void* lengthWords,
// void* fileState, unsigned int context), with context read from fileState +0x94.
// lengthWords is a borrowed two-word {0, length} array valid during that call.
// Keep fileHandle alive until completion. Require the shared async record idle
// on entry and prevent overlapping async operations until completion;
// completion frees the allocated length buffer and callback wrapper. Allocation
// failure is unchecked here, as in the existing implementation.
int gtl_CFileIoDeviceFMEM_GetFileSize(void* self, unsigned int fileHandle, int* outSyncWord, int completionAddress) {
    if (fileHandle == kInvalidFileHandle) {
        File_SetLastError(kFileInvalidHandle);
        return kFileLengthQueryFailed;
    }
    int fileLength = 0;
    if (completionAddress == 0) {
        // Synchronous phase 1: fetch a byte length into caller-independent storage.
        int nandResult = NANDGetLength(fileHandle, &fileLength);
        int operationResult = nandResult;
        int lastError;
        TranslateNandLengthError(nandResult, lastError);
        // Synchronous phase 2: publish translated error before inspecting raw success.
        // Unlisted results (including reserved -7) map to kFileUnknown.
        File_SetLastError(lastError);
        if (operationResult < 0) {
            return kFileLengthQueryFailed;
        }
        if (outSyncWord) {
            *outSyncWord = 0;
        }
        File_SetLastError(kFileSuccess);
        return fileLength;
    } else {
        // Async phase 1: retain file/completion in the wrapper and allocate one word.
        void* callback = FileIO_Alloc(sizeof(FmemOperationCallback));
        if (callback) {
            AsFmemOperationCallback(callback)->vtable = __vt__Q23gtl20CCallBackGetFileSize;
            AsFmemOperationCallback(callback)->argument = fileHandle;
            AsFmemOperationCallback(callback)->callbackPayload = completionAddress;
        }
        volatile char* operation = s_fmemAsyncOperation;
        LengthOperationAt(operation).callback = callback;
        LengthOperationAt(operation).resultBytes = sizeof(int);
        LengthOperationAt(operation).resultBuffer = FileIO_Alloc(LengthOperationAt(operation).resultBytes);
        *(int*)LengthOperationAt(operation).resultBuffer = 0;
        // Async phase 2: publish the shared record, then submit the NAND operation.
        NANDSetUserData(s_nandAsyncCommandBlock, (void*)operation);
        int nandResult = NANDGetLengthAsync(fileHandle, LengthOperationAt(s_fmemAsyncOperation).resultBuffer, (void*)gtl_CFileIoDeviceFMEM_DispatchNandCallback, s_nandAsyncCommandBlock);
        int operationResult = nandResult;
        int lastError;
        TranslateNandLengthError(nandResult, lastError);
        File_SetLastError(lastError);
        // Async phase 3: rejection releases both allocations; acceptance transfers
        // cleanup to the callback. The file-info object remains caller-owned.
        if (operationResult != 0) {
            // Rejection keeps the result-buffer slot anchored through the free call.
            char* failedOperation = s_fmemAsyncOperation;
            void* resultBuffer = LengthBufferSlot(failedOperation);
            FileIO_Free(resultBuffer);
            LengthBufferSlot(failedOperation) = 0;
            ((FileLengthCallbackInterface*)callback)->destroy();
            return kFileLengthQueryFailed;
        }
        File_SetLastError(kFileSuccess);
        return 0;
    }
}
} // extern "C"
