// Directory size on a file device: sums the allocation size of every file under a directory tree,
// recursing into subdirectories. Each file size is rounded up to 0x4000-byte blocks and added into
// a two-word total. The inline helpers read and borrow the file layer's small path strings.

extern "C" void* FileIOString_dtor(void*, int);

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Register exceptional cleanup for a constructed local string.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm before the explicit normal-path free.
    void release() {
        active = false;
    }
    // Destroy the string on unwinding unless normal cleanup already ran.
    ~EH5Cleanup() {
        if (active)
            destroy(object, -1);
    }
};

#include "types.h"
#include "sdk_protos.h"

extern "C" {
enum DirectoryAllocationConstants {
    kDirectoryAttribute = 0x10,
    kAllocationUnitBytes = 0x4000,
    kInlinePathLengthMask = 0x7f,
    kOpenReadAccess = 0x80000000u,
    kOpenExclusiveSharing = 0,
    kOpenExistingDisposition = 3,
    kOpenNormalAttributes = 0x80
};
enum DirectoryDeviceSlots {
    kDeviceOpen = 0x10 / 4,
    kDeviceClose = 0x14 / 4,
    kDeviceQuerySize = 0x24 / 4,
    kDeviceFindFirst = 0x54 / 4,
    kDeviceFindNext = 0x58 / 4,
    kDeviceCloseFind = 0x5c / 4
};
extern char filePathUtilityStrings[0x104];
void FileIOString_FitCapacityForLength(void* a, u32 b);
void FileIOString_ReplaceRangeWithAllocator(void* a, u32 b, u32 c, const char* d, const char* e, void* f);
void FileIO_Free(void* p);

// Read the active length of the 12-byte file-layer string; flags is its already loaded first word.
inline u32 FileIOPathLength(const char* string, u32 flags) {
    return flags >> 31 ? *(const u32*)(string + 4) : *(const unsigned char*)string & kInlinePathLengthMask;
}

// Borrow active string bytes; the string/cleanup guard retains ownership.
inline char* FileIOPathBytes(char* string, u32 flags) {
    return flags >> 31 ? *(char**)(string + 8) : string + 1;
}

// Device contract observed here (vtable byte offsets are named by DirectoryDeviceSlots):
// open(self,path,access,sharing,disposition,attributes) -> opaque handle. The flag
// values mirror GENERIC_READ / no sharing / OPEN_EXISTING / FILE_ATTRIBUTE_NORMAL;
// the actual device implements their interpretation. No invalid-file-handle check is made.
// query(self,handle,&outWord,0,0) -> returnWord. Both are used as unsigned words,
// despite the recovered pointer-shaped return type. Their high/low or physical/logical
// meaning is not established by this caller; do not give them a 64-bit interpretation.
// Confirmed backend example: synchronous FMEM_vfunc_7 (gtl_fileio_fmem_get_file_size.cpp) returns
// NAND byte length and writes zero to outWord on success. Thus totals[1] accumulates
// rounded byte lengths there; totals[0] normally changes only on recursive overflow.
// close is passed the handle plus two arithmetic scratch values, whose device meaning
// is unresolved. FindFirst returns -1 on failure; FindNext returns zero at the end.
//
// Counter update contract, with R(n) the signed-remainder rounding below:
// file: totals[0] += R(query outWord); totals[1] += R(query returnWord), independently.
// directory: totals[1] += child[1], or on overflow totals[0]++ and
// totals[1] = child[1] - (UINT_MAX - oldTotals[1]); child[0] is discarded.
// The overflow branch differs by one from ordinary unsigned wrap; preserve it.

// Recursively accumulate allocation-size words for entries beneath a directory.
// baseDirectory is a borrowed base path; allocationTotals points to TWO existing unsigned 32-bit totals
// and is accumulated into, not initialized. This unit does not establish their
// device-specific interpretation: query slot 0x24 writes one and returns the other.
// Do not treat them as a conventional high/low 64-bit value: the file branch
// rounds/adds them independently, while recursion uses only child word 1 plus carry.
// Returns 1 even when find-first returns -1 (missing/empty/inaccessible conflated).
int FileIoDevice_AccumulateDirectoryAllocationSize(void* self, const char* baseDirectory, void* allocationTotals) {
    // Phase 1: build the owned search-pattern string (12-byte small-string ABI).
    // High bit of word 0 selects heap storage at +8; otherwise text is at +1.
    char entryPathString[12];
    *(u32*)(entryPathString + 0) = 0;
    *(u32*)(entryPathString + 4) = 0;
    *(u32*)(entryPathString + 8) = 0;

    strlen(baseDirectory);
    size_t len = strlen(baseDirectory);
    FileIOString_FitCapacityForLength(entryPathString, (u32)len);

    unsigned char patternAllocator = *(unsigned char*)(entryPathString + 0);
    FileIOString_ReplaceRangeWithAllocator(entryPathString, 0, 0, baseDirectory, baseDirectory + len,
                                           (void*)&patternAllocator);
    EH5Cleanup searchPatternCleanup(entryPathString, (void (*)(void*, int))FileIOString_dtor);

    u32 flags = *(u32*)entryPathString;
    u32 pathLength = FileIOPathLength(entryPathString, flags);

    char* pathTokens = filePathUtilityStrings;
    size_t patternLength = strlen(pathTokens);
    unsigned char patternSuffixAllocator = *(unsigned char*)(entryPathString + 4);
    (void)patternSuffixAllocator;
    patternSuffixAllocator = 0;
    FileIOString_ReplaceRangeWithAllocator(entryPathString, pathLength, 0, pathTokens, pathTokens + patternLength,
                                           (void*)&patternSuffixAllocator);

    char* ptr;
    flags = *(u32*)entryPathString;
    ptr = FileIOPathBytes(entryPathString, flags);
    (void)ptr;

    // Device vtable byte slots: 0x54 find-first, 0x58 find-next, 0x5c close-find.
    // Entry +0 is attributes (0x10 directory); +0x2c is the borrowed entry name.
    char directoryEntry[0x140];
    *(u32*)directoryEntry = 0;
    void** vt = *(void***)self;
    typedef void* (*FindFirstEntry)(void*, char*, void*, int, int);
    void* findResult = ((FindFirstEntry)vt[kDeviceFindFirst])(self, ptr, (void*)directoryEntry, 0, 0);

    // addis r0,r3,1 / cmplwi r0,0xffff recognizes the -1 handle.
    if (findResult == (void*)-1) {
        flags = *(u32*)entryPathString;
        if (flags >> 31) {
            {
                searchPatternCleanup.release();
                FileIO_Free(*(void**)(entryPathString + 8));
            }
        }
        return 1;
    }

    char* entrySeparator = pathTokens + 3;
    u32 enumerationHandle = (u32)(unsigned long)findResult;
    int hasNextEntry = 0;

    // Phase 2: rebuild base/separator/name for each entry; recurse for directories,
    // otherwise open the file and accumulate the two size-query results.
    do {
        flags = *(u32*)entryPathString;
        u32 oldPathLength = FileIOPathLength(entryPathString, flags);

        unsigned char basePathAllocator = *(unsigned char*)(entryPathString + 0); // sp2C
        (void)basePathAllocator;
        basePathAllocator = 0;
        FileIOString_ReplaceRangeWithAllocator(entryPathString, 0, oldPathLength, baseDirectory,
                                               baseDirectory + strlen(baseDirectory), (void*)&basePathAllocator);

        flags = *(u32*)entryPathString;
        u32 directoryPathLength = FileIOPathLength(entryPathString, flags);

        unsigned char separatorAllocator = *(unsigned char*)(entryPathString + 4);
        (void)separatorAllocator;
        separatorAllocator = 0;
        FileIOString_ReplaceRangeWithAllocator(entryPathString, directoryPathLength, 0, entrySeparator,
                                               entrySeparator + strlen(entrySeparator), (void*)&separatorAllocator);

        flags = *(u32*)entryPathString;
        u32 pathWithSeparatorLength = FileIOPathLength(entryPathString, flags);

        // The directory entry written by vtable slot 0x54/0x58 is 0x140 bytes; its file name is at +0x2c.
        char* entryName = directoryEntry + 0x2c;
        unsigned char entryNameAllocator = *(unsigned char*)(entryPathString + 0); // sp1C
        (void)entryNameAllocator;
        entryNameAllocator = 0;
        FileIOString_ReplaceRangeWithAllocator(entryPathString, pathWithSeparatorLength, 0, entryName,
                                               entryName + strlen(entryName), (void*)&entryNameAllocator);

        if (*(u32*)directoryEntry & kDirectoryAttribute) {
            flags = *(u32*)entryPathString;
            char* recursivePathBytes = FileIOPathBytes(entryPathString, flags);

            // Existing recursive merge: ignore child word 0; propagate only
            // overflow of child word 1. Preserve its unusual subtraction exactly.
            u32 recursiveSize[2] = {0, 0};
            if (FileIoDevice_AccumulateDirectoryAllocationSize(self, recursivePathBytes, recursiveSize) != 0) {
                unsigned int previousChildTotal = *((u32*)((char*)allocationTotals + 4));
                if (recursiveSize[1] > (unsigned int)(-1 - previousChildTotal)) {
                    *(u32*)allocationTotals = *(u32*)allocationTotals + 1;
                    *((u32*)((char*)allocationTotals + 4)) = recursiveSize[1] - (-1 - previousChildTotal);
                } else {
                    *((u32*)((char*)allocationTotals + 4)) = previousChildTotal + recursiveSize[1];
                }
            }
        } else {
            char* devicePrefix = pathTokens + 5;
            char deviceFilePathString[12];
            *(u32*)(deviceFilePathString + 0) = 0;
            *(u32*)(deviceFilePathString + 4) = 0;
            *(u32*)(deviceFilePathString + 8) = 0;
            size_t tlen = strlen(devicePrefix);
            FileIOString_FitCapacityForLength(deviceFilePathString, (u32)tlen);

            unsigned char prefixAllocator = *(unsigned char*)(deviceFilePathString + 4);
            (void)prefixAllocator;
            prefixAllocator = 0;
            FileIOString_ReplaceRangeWithAllocator(deviceFilePathString, 0, 0, devicePrefix, devicePrefix + tlen,
                                                   (void*)&prefixAllocator);
            EH5Cleanup openPathCleanup(deviceFilePathString, (void (*)(void*, int))FileIOString_dtor);

            u32 openPathFlags = *(u32*)deviceFilePathString;
            u32 prefixLength = FileIOPathLength(deviceFilePathString, openPathFlags);

            flags = *(u32*)entryPathString;
            char* fullEntryPathBytes;
            u32 fullEntryPathLength;
            if (flags >> 31) {
                fullEntryPathBytes = *(char**)(entryPathString + 8);
                fullEntryPathLength = *(u32*)(entryPathString + 4);
            } else {
                fullEntryPathBytes = entryPathString + 1;
                fullEntryPathLength = *(unsigned char*)entryPathString & 0x7f;
            } // lbz 0x5c(r1) at 0x8041AE9C

            unsigned char openPathAllocator = 0;
            FileIOString_ReplaceRangeWithAllocator(deviceFilePathString, prefixLength, 0, fullEntryPathBytes,
                                                   fullEntryPathBytes + fullEntryPathLength, (void*)&openPathAllocator);

            char* openPathBytes;
            if (*(u32*)deviceFilePathString >> 31)
                openPathBytes = *(char**)(deviceFilePathString + 8);
            else
                openPathBytes = deviceFilePathString + 1;

            // Open/query/close is synchronous here; temporary path storage is released afterward.
            typedef void* (*OpenEntryFile)(void*, char*, unsigned int, int, int, int);
            vt = *(void***)self;
            void* fileHandle =
                ((OpenEntryFile)vt[kDeviceOpen])(self, openPathBytes, kOpenReadAccess, kOpenExclusiveSharing,
                                                 kOpenExistingDisposition, kOpenNormalAttributes);

            u32 sizeOutputWord = 0;
            typedef void* (*QueryFileSizeWords)(void*, void*, void*, int, int);
            vt = *(void***)self;
            void* sizeReturnWord =
                ((QueryFileSizeWords)vt[kDeviceQuerySize])(self, fileHandle, (void*)&sizeOutputWord, 0, 0);

            // Phase 3: round each query word up to a 0x4000-byte allocation unit.
            // For nonnegative n the rotate/subtract idiom gives n % 16384;
            // sign extraction preserves signed-remainder behavior for high-bit n.
            // The rotations are compiler ABI detail; replacing with unsigned %
            // would change behavior for words with bit 31 set.
            u32 roundedOutputWord = sizeOutputWord;
            u32 outputSignBit = roundedOutputWord >> 31;
            u32 shiftedOutputWord = (roundedOutputWord << 18) - outputSignBit;
            u32 outputRemainder = ((shiftedOutputWord << 14) | (shiftedOutputWord >> 18)) + outputSignBit;
            if (outputRemainder != 0) {
                roundedOutputWord = roundedOutputWord + (kAllocationUnitBytes - outputRemainder);
            }
            u32 returnSignBit = (u32)(unsigned long)sizeReturnWord >> 31;
            u32 shiftedReturnWord = ((u32)(unsigned long)sizeReturnWord << 18) - returnSignBit;
            sizeOutputWord = roundedOutputWord;
            u32 returnRemainder = ((shiftedReturnWord << 14) | (shiftedReturnWord >> 18)) + returnSignBit;
            u32 roundedReturnWord;
            if (returnRemainder == 0) {
                roundedReturnWord = (u32)(unsigned long)sizeReturnWord;
            } else {
                roundedReturnWord = (u32)(unsigned long)sizeReturnWord + (kAllocationUnitBytes - returnRemainder);
            }

            u32 previousReturnTotal = *((u32*)((char*)allocationTotals + 4));
            u32 newOutputTotal = *(u32*)allocationTotals + roundedOutputWord;
            *(u32*)allocationTotals = newOutputTotal;
            *((u32*)((char*)allocationTotals + 4)) = previousReturnTotal + roundedReturnWord;

            // Close each file after querying, including when a size is zero.
            // Extra register arguments preserve the device's existing call ABI.
            typedef void (*CloseEntryFile)(void*, void*, u32, u32);
            vt = *(void***)self;
            ((CloseEntryFile)vt[kDeviceClose])(self, fileHandle, previousReturnTotal, newOutputTotal);

            flags = *(u32*)deviceFilePathString;
            if (flags >> 31) {
                {
                    openPathCleanup.release();
                    FileIO_Free(*(void**)(deviceFilePathString + 8));
                }
            }
        }

        typedef int (*FindNextEntry)(void*, void*, void*, int, int);
        vt = *(void***)self;
        hasNextEntry =
            ((FindNextEntry)vt[kDeviceFindNext])(self, (void*)enumerationHandle, (void*)directoryEntry, 0, 0);
    } while (hasNextEntry != 0);

    // Phase 4: close successful enumeration once, then free the search string.
    // There is no separate enumeration/file RAII owner on exceptional exits.
    typedef void (*CloseEnumeration)(void*, void*);
    vt = *(void***)self;
    ((CloseEnumeration)vt[kDeviceCloseFind])(self, (void*)enumerationHandle);

    flags = *(u32*)entryPathString;
    if (flags >> 31) {
        {
            searchPatternCleanup.release();
            FileIO_Free(*(void**)(entryPathString + 8));
        }
    }
    return 1;
}
} // extern "C"
