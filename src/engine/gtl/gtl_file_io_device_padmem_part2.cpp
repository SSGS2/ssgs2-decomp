// PADMEM file I/O device: the QueryHandleFileInfo method, which fills a file-info record
// with the stored last-write metadata words.

#include "classes/GtlFileIoDevicePADMEM.h"
#include "sdk_protos.h"

extern "C" void File_SetLastError(int x);
extern "C" int PadMemory_QueryStoredMetadata(void* p, void* out);
extern "C" void* memset(void* dest, int value, unsigned int size);

// BY_HANDLE_FILE_INFORMATION-compatible result: only the last-write slots are filled here.
// The device returns two opaque metadata words; no time epoch or tick unit is established.
// File-size fields are elsewhere (+0x24 in the FMEM implementation).
struct PadMemoryFileStat {
    char metadata[0x14];
    unsigned int lastWriteTimeFirstWord;
    unsigned int lastWriteTimeSecondWord;
    char remainingMetadata[0x18];
};
enum PadMemoryFileError {
    kFileSuccess = 0,
    kInvalidHandle = 6,
    kInvalidParameter = 0x57,
    kInvalidFileHandle = 0xFFFFFFFFu,
    kPadResultBias = 7,
    kPadMemoryError1 = 0xC739,
    kPadMemoryError2 = 0xC73A,
    kPadMemoryError3 = 0xC73B,
    kPadMemoryError4 = 0xC73C,
    kPadMemoryError5 = 0xC73D,
    kPadMemoryError6 = 0xC73E,
    kPadMemoryError7 = 0xC73F
};

#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_strength_reduction off

// Query PAD metadata into file-info last-write slots; return handled status, with failures in last-error.
// output borrows a writable 0x34-byte record. Invalid output/handle returns 0;
// any attempted query returns 1, including failures (record then remains zeroed).
// The handle is checked only for the invalid sentinel; the device field selects the query target.
int gtl::CFileIoDevicePADMEM::QueryHandleFileInfo(int handle, void* output) {
    unsigned int metadataWords[2];
    int queryResult;
    unsigned int dispatchIndex;
    int fileError;
    int wordMask;
    int querySucceeded;
    unsigned int firstOutputWord;
    unsigned int secondOutputWord;
    if (output == 0) {
        File_SetLastError(kInvalidParameter);
        return 0;
    }
    if ((unsigned int)handle == kInvalidFileHandle) {
        File_SetLastError(kInvalidHandle);
        return 0;
    }
    // Start with a fully zeroed result, then translate PAD status 0/-1..-7 to file errors.
    memset(output, 0, sizeof(PadMemoryFileStat));
    queryResult = PadMemory_QueryStoredMetadata(controllerIndex, metadataWords);
    dispatchIndex = (unsigned int)queryResult + kPadResultBias;
    switch (dispatchIndex) {
    case 7:
        fileError = kFileSuccess;
        break;
    case 6:
        fileError = kPadMemoryError1;
        break;
    case 5:
        fileError = kPadMemoryError2;
        break;
    case 4:
        fileError = kPadMemoryError3;
        break;
    case 3:
        fileError = kPadMemoryError4;
        break;
    case 2:
        fileError = kPadMemoryError5;
        break;
    case 1:
        fileError = kPadMemoryError6;
        break;
    case 0:
        fileError = kPadMemoryError7;
        break;
    default:
        fileError = kPadMemoryError2;
        break;
    }
    File_SetLastError(fileError);
    // Reverse the two metadata words into file-info order only when the query succeeds.
    if (queryResult != 0) {
        querySucceeded = 0;
    } else {
        // Identity mask preserves both complete 32-bit words; no timestamp conversion is performed.
        wordMask = -1;
        firstOutputWord = metadataWords[1];
        firstOutputWord = firstOutputWord & wordMask;
        ((PadMemoryFileStat*)output)->lastWriteTimeSecondWord = 0;
        ((PadMemoryFileStat*)output)->lastWriteTimeFirstWord = firstOutputWord;
        secondOutputWord = metadataWords[0];
        secondOutputWord = secondOutputWord & wordMask;
        ((PadMemoryFileStat*)output)->lastWriteTimeSecondWord = secondOutputWord;
        querySucceeded = 1;
    }
    if (querySucceeded != 0) {
        File_SetLastError(kFileSuccess);
    }
    return 1;
}

#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
