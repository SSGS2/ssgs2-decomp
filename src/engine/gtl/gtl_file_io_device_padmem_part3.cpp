// PADMEM file I/O device: the QueryStoredFileInfo method, which fills a file-info record
// with the stored title and the metadata words.

#include "classes/GtlFileIoDevicePADMEM.h"
#include "sdk_protos.h"

extern "C" void File_SetLastError(int x);
extern "C" int PadMemory_QueryStoredTitle(void* p, int* out);
extern "C" int PadMemory_QueryStoredMetadata(void* p, void* out);
extern "C" void* memset(void* dest, int value, unsigned int size);
extern "C" void* memcpy(void* dest, const void* src, unsigned int size);
extern "C" size_t strlen(const char* s);

// The PADMEM device fills the same 0x140-byte file-info record as other devices.
// +0x14 receives metadata[1], +0x18 receives metadata[0]; no time unit is established.
// +0x2C is a NUL-terminated title, relying on the zeroed record for termination.
struct PadMemoryFileInfo {
    unsigned char prefix[0x14];
    unsigned int metadataWord1;
    unsigned int metadataWord0;
    unsigned char reserved[0x10];
    char title[0x114];
};
enum {
    kFileErrorNone = 0,
    kFileErrorInvalidInfo = 2,
    kPadErrorMinus1 = 0xC739,
    kPadErrorMinus2 = 0xC73A,
    kPadErrorMinus3 = 0xC73B,
    kPadErrorMinus4 = 0xC73C,
    kPadErrorMinus5 = 0xC73D,
    kPadErrorMinus6 = 0xC73E,
    kPadErrorMinus7 = 0xC73F,
    kPadResultBias = 7
};

#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_strength_reduction off

// Read the stored title and two metadata words; translate either device failure to a file error.
// output borrows at least 0x140 writable bytes. The driver title is borrowed, never freed.
// Precondition: a successful title query supplies a string shorter than 0x114 bytes;
// the copy has no length clamp. Return 1 on success, -1 on any failure (last-error 2).
// A failed metadata query leaves the title present; callers must honor the return value.
int gtl::CFileIoDevicePADMEM::QueryStoredFileInfo(int /*unused*/, void* output) {
    if (output == 0) {
        File_SetLastError(kFileErrorInvalidInfo);
        return -1;
    }

    // Phase 1: clear the complete record and obtain the borrowed title.
    memset(output, 0, sizeof(PadMemoryFileInfo));

    void* storedTitle = 0;
    int titleResult = PadMemory_QueryStoredTitle(controllerIndex, (int*)&storedTitle);

    int resultIndex = titleResult + kPadResultBias;
    int fileError;
    switch (resultIndex) {
    case kPadResultBias:
        fileError = kFileErrorNone;
        break;
    case 6:
        fileError = kPadErrorMinus1;
        break;
    case 5:
        fileError = kPadErrorMinus2;
        break;
    case 4:
        fileError = kPadErrorMinus3;
        break;
    case 3:
        fileError = kPadErrorMinus4;
        break;
    case 2:
        fileError = kPadErrorMinus5;
        break;
    case 1:
        fileError = kPadErrorMinus6;
        break;
    case 0:
        fileError = kPadErrorMinus7;
        break;
    default:
        fileError = kPadErrorMinus2;
        break;
    }
    File_SetLastError(fileError);

    if (titleResult != 0) {
        File_SetLastError(kFileErrorInvalidInfo);
        return -1;
    }

    const char* title = (const char*)storedTitle;
    size_t titleLength = strlen(title);
    if (titleLength != 0) {
        memcpy(((PadMemoryFileInfo*)output)->title, title, (unsigned int)titleLength);
    }

    // Phase 2: query the opaque metadata pair, translating the same 0/-1..-7 statuses.
    // The translated error is overwritten with invalid-info on either query failure.
    unsigned int metadata[2];
    int metadataResult = PadMemory_QueryStoredMetadata(controllerIndex, metadata);

    resultIndex = metadataResult + kPadResultBias;
    switch (resultIndex) {
    case kPadResultBias:
        fileError = kFileErrorNone;
        break;
    case 6:
        fileError = kPadErrorMinus1;
        break;
    case 5:
        fileError = kPadErrorMinus2;
        break;
    case 4:
        fileError = kPadErrorMinus3;
        break;
    case 3:
        fileError = kPadErrorMinus4;
        break;
    case 2:
        fileError = kPadErrorMinus5;
        break;
    case 1:
        fileError = kPadErrorMinus6;
        break;
    case 0:
        fileError = kPadErrorMinus7;
        break;
    default:
        fileError = kPadErrorMinus2;
        break;
    }
    File_SetLastError(fileError);

    int metadataAvailable;
    if (metadataResult != 0) {
        metadataAvailable = 0;
    } else {
        // Phase 3: reverse the word order. The identity masks and intermediate zero
        // store preserve the matched instruction sequence; they perform no conversion.
        int mask = -1;
        unsigned int secondWord = metadata[1];
        unsigned int firstWord = 0;
        metadataAvailable = 1;
        secondWord = secondWord & (unsigned int)mask;
        ((PadMemoryFileInfo*)output)->metadataWord0 = firstWord;
        ((PadMemoryFileInfo*)output)->metadataWord1 = secondWord;
        firstWord = metadata[0];
        firstWord = firstWord & (unsigned int)mask;
        ((PadMemoryFileInfo*)output)->metadataWord0 = firstWord;
    }

    if (metadataAvailable == 0) {
        File_SetLastError(kFileErrorInvalidInfo);
        return -1;
    } else {
        File_SetLastError(kFileErrorNone);
        return 1;
    }
}

#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
