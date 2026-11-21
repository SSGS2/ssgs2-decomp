// Translation unit 0x80419A74-0x80419C8C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

extern "C" {

struct NANDFileInfo;
void File_SetLastError(int errorCode);
int NANDSeek(NANDFileInfo* fileInfo, int byteOffset, int origin);

enum SeekOrigin { kSeekFromStart = 0, kSeekFromCurrent = 1, kSeekFromEnd = 2 };
enum SeekFailure { kSeekFailed = -1, kInvalidFileHandle = 6 };
enum NandSeekResult {
    kNandSuccess = 0, kNandAccess = -1, kNandAlloc = -2,
    kNandBusy = -3, kNandCorrupt = -4, kNandEccCritical = -5,
    kNandExists = -6, kNandUnmapped = -7, kNandInvalid = -8,
    kNandMaxBlocks = -9, kNandMaxDescriptors = -10, kNandMaxFiles = -11,
    kNandNoExists = -12, kNandNotEmpty = -13, kNandOpenFiles = -14,
    kNandAuthentication = -15, kNandUnknown = -64, kNandFatal = -128
};
enum FileSeekError {
    kSeekOk = 0, kSeekAccess = 0xC351, kSeekAlloc = 0xC352,
    kSeekBusy = 0xC353, kSeekCorrupt = 0xC354, kSeekEccCritical = 0xC355,
    kSeekExists = 0xC356, kSeekInvalid = 0xC358, kSeekMaxBlocks = 0xC359,
    kSeekMaxDescriptors = 0xC35A, kSeekMaxFiles = 0xC35B,
    kSeekNoExists = 0xC35C, kSeekNotEmpty = 0xC35D, kSeekOpenFiles = 0xC35E,
    kSeekAuthentication = 0xC35F, kSeekUnknown = 0xC360, kSeekFatal = 0xC361
};

// Seek an open NAND file; return its new position or -1 and clear the optional high word on success.
// fileInfo is borrowed and must be a valid open handle; only the -1 sentinel is checked.
// byteOffset is signed bytes. origin 0/1/2 selects start/current/end; other values use start.
// positionHigh is an output high word, never an input offset; failures leave it untouched.
// Nonnegative NAND results are positions, not status codes. Final success clears last error.
int gtl_CFileIoDeviceFMEM_Seek(void* self, NANDFileInfo* fileInfo, int byteOffset, unsigned int* positionHigh, unsigned int origin) {
    if (fileInfo == (NANDFileInfo*)-1) {
        File_SetLastError(kInvalidFileHandle);
        return kSeekFailed;
    }
    // Translate origin, then issue the synchronous seek without changing handle ownership.
    int nandOrigin = kSeekFromStart;
    if (origin == kSeekFromCurrent) nandOrigin = kSeekFromCurrent;
    if (origin == kSeekFromEnd) nandOrigin = kSeekFromEnd;
    int nandResult = NANDSeek(fileInfo, byteOffset, nandOrigin);
    int seekResult = nandResult;
    int lastError;
    // Translate negative NAND statuses to the file API's 50000+ error namespace.
    // -7 (unmapped), unrecognized negatives and positive positions initially map to
    // kSeekUnknown; the success epilogue below clears that temporary error for positions.
    // Keep this branch tree/label arrangement to preserve the compiled instructions.
    do {
        if (nandResult != kNandMaxBlocks) {
            if (nandResult < kNandMaxBlocks) {
                if (nandResult == kNandAuthentication) goto L_Authentication;
                if (nandResult < kNandAuthentication) {
                    if (nandResult == kNandUnknown) goto L_Unknown;
                    if (nandResult < kNandUnknown && nandResult == kNandFatal) goto L_Fatal;
                    goto L_DefaultError;
                } else {
                    if (nandResult == kNandNoExists) goto L_NoExists;
                    if (nandResult < kNandNoExists) {
                        if (nandResult >= kNandNotEmpty) goto L_NotEmpty;
                        goto L_OpenFiles;
                    } else {
                        if (nandResult >= kNandMaxDescriptors) goto L_MaxDescriptors;
                        goto L_MaxFiles;
                    }
                }
            } else {
                if (nandResult == kNandBusy) goto L_Busy;
                if (nandResult < kNandBusy) {
                    if (nandResult == kNandExists) goto L_Exists;
                    if (nandResult < kNandExists) {
                        if (nandResult >= kNandUnmapped) goto L_DefaultError;
                        goto L_Invalid;
                    } else {
                        if (nandResult >= kNandCorrupt) goto L_Corrupt;
                        goto L_EccCritical;
                    }
                } else {
                    if (nandResult != kNandSuccess) {
                        if (nandResult >= kNandSuccess) goto L_DefaultError;
                        if (nandResult >= kNandAccess) goto L_Access;
                        goto L_Alloc;
                    }
                }
            }
            lastError = kSeekOk;
            break;
        L_Access:
            lastError = kSeekAccess;
            break;
        L_Alloc:
            lastError = kSeekAlloc;
            break;
        L_Busy:
            lastError = kSeekBusy;
            break;
        L_Corrupt:
            lastError = kSeekCorrupt;
            break;
        L_EccCritical:
            lastError = kSeekEccCritical;
            break;
        L_Exists:
            lastError = kSeekExists;
            break;
        L_Invalid:
            lastError = kSeekInvalid;
            break;
        }
        lastError = kSeekMaxBlocks;
        break;
    L_MaxDescriptors:
        lastError = kSeekMaxDescriptors;
        break;
    L_MaxFiles:
        lastError = kSeekMaxFiles;
        break;
    L_NoExists:
        lastError = kSeekNoExists;
        break;
    L_NotEmpty:
        lastError = kSeekNotEmpty;
        break;
    L_OpenFiles:
        lastError = kSeekOpenFiles;
        break;
    L_Unknown:
        lastError = kSeekUnknown;
        break;
    L_Authentication:
        lastError = kSeekAuthentication;
        break;
    L_Fatal:
        lastError = kSeekFatal;
        break;
    L_DefaultError:
        lastError = kSeekUnknown;
        break;
    } while (0);
    // Publish the mapped error first; any nonnegative position is then a successful seek.
    File_SetLastError(lastError);
    if (seekResult < 0) {
        return kSeekFailed;
    }
    if (positionHigh) {
        *positionHigh = 0;
    }
    File_SetLastError(kSeekOk);
    return seekResult;
}

} // extern "C"
