// Translation unit 0x80419394-0x804194C4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/FmemFileState.h"
#include "types.h"
#include "sdk_protos.h"

extern "C" {
extern void* memset(void*, int, unsigned int);
void File_SetLastError(int x);
void FileIO_Free(void* p);
void* FileIO_Alloc(unsigned int size);
int NANDCreate(void* p, int a, int b);
int NANDOpen(void* p, void* q, unsigned char flags);

enum FmemOpenConstants {
    kPathBytes = 0x104, kNandRead = 1, kNandWrite = 2,
    kCreateMode2 = 2, kCreateMode4 = 4, // Other mode semantics are unresolved.
    kOwnerReadWrite = 0x30, kNoAttributes = 0, kNandExists = -6,
    kCreateFailed = 0x52, kOpenFailed = 0x6e,
    kFileStateBytes = 0x98, kNandFileInfoBytes = 0x8c
};
static const unsigned int kRequestedRead = 0x80000000U;
static const unsigned int kRequestedWrite = 0x40000000U;

// Open the NAND path after the device prefix, creating it first in modes 2 or 4.
// devicePath must fit in kPathBytes with a terminator and contain '/'; the first
// slash begins the NAND path. No extra validation or truncation recovery occurs.
// Return an owned FmemFileState address as an int, or -1 with last-error set.
// On success the caller must close/release that handle; self/arg4 are unused.
int gtl_CFileIoDeviceFMEM_OpenFile(void* self, const char* devicePath, int flags, int /*unused*/, int createModeOrHandle) {
    // Phase 1: strip the device prefix and translate access bits to NAND flags.
    char pathBuffer[kPathBytes];
    strncpy(pathBuffer, devicePath, kPathBytes);
    char* nandPath = strchr(pathBuffer, '/');
    unsigned char nandAccess = 0;
    if (flags & kRequestedRead) nandAccess |= kNandRead;
    if (flags & kRequestedWrite) nandAccess |= kNandWrite;
    // Phase 2: optional create accepts an existing file without truncating it.
    if (createModeOrHandle == kCreateMode2 || createModeOrHandle == kCreateMode4) {
        int createResult = NANDCreate(nandPath, kOwnerReadWrite, kNoAttributes);
        if (createResult != 0 && createResult != kNandExists) {
            File_SetLastError(kCreateFailed);
            return -1;
        }
    }
    // Phase 3: reuse the incoming mode slot for the owned handle. Clear the SDK
    // prefix and +0x8c/+0x94; +0x90 is left for later operations to populate.
    // Allocation failure still reaches NANDOpen, preserving existing behavior.
    createModeOrHandle = (int)FileIO_Alloc(kFileStateBytes);
    if (createModeOrHandle) {
        AsFmemFileState(createModeOrHandle)->operationState = 0;
        AsFmemFileState(createModeOrHandle)->callbackArgument = 0;
        memset((void*)createModeOrHandle, 0, kNandFileInfoBytes);
    }
    // Phase 4: transfer the opened handle to the caller, or release it on failure.
    int openResult = NANDOpen(nandPath, (void*)createModeOrHandle, (unsigned char)nandAccess);
    if (openResult != 0) {
        File_SetLastError(kOpenFailed);
        if (createModeOrHandle) FileIO_Free((void*)createModeOrHandle);
        return -1;
    }
    File_SetLastError(0);
    return createModeOrHandle;
}

} // extern "C"
