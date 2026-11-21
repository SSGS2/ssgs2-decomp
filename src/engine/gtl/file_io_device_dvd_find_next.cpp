// Translation unit 0x804130C8-0x80413360 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>

// 12-byte string overlay: the top flag bit selects owned heap data; otherwise
// text starts at byte +1 and its 7-bit length is in the first byte's low bits.
// rest also overlaps inline text; it is not an independent capacity field.
struct StringRep { CW_BITS2( unsigned int isLong : 1, unsigned int rest : 31)
 unsigned int heapLength; char* data; };
// The borrowed search handle embeds a DVD directory cursor followed by its pattern.
struct DVDFile { char directoryHandleStorage[0x0C]; StringRep path; };
// Caller-owned result: flags at +0, filename at +0x2C, total size 0x140 bytes.
struct FindData { unsigned int flags; char metadataStorage[0x28]; char name[0x114]; };
// Opaque argument view only; the actual returned directory-entry view is below.
struct Entry { char bytes[0x14]; };
enum { kInvalidSearchHandle = 0xFFFFFFFF, kInvalidHandleError = 6,
       kEntryFound = 1, kDirectoryAttribute = 0x10,
       kShortStringLengthShift = 24, kShortStringLengthMask = 0x7f,
       kDvdDirectoryEntryRead = 1, kNoSearchError = 0 };
extern "C" {
void FileIO_Free(void*);
void* memset(void*, int, unsigned int);
int File_SetLastError(int);
int DVDReadDir(DVDFile*, Entry*);
void FileIOString_ReplaceRangeWithAllocator(StringRep*, unsigned int, unsigned int, const char*, const char*, void*);
void FileIOString_FitCapacityForLength(StringRep*, unsigned int);
void* FileIOString_dtor(StringRep*, int);
extern char fileSearchExtensionlessDotSuffix[];

// DVDDirEntry as filled by DVDReadDir: name at +8 is borrowed from the DVD API.
struct DirEntry804130C8 { unsigned int entryNum; int isDir; char* name; };

// Advance a DVD search cursor until its stored wildcard pattern matches an entry.
// search is the FindFirstFile handle; outEntry must be writable for sizeof(FindData).
// Return 1 fills outEntry; 0 leaves it untouched. Error 6 means handle -1; exhaustion
// also returns 0 but clears the error. Null handles/outputs are not validated.
// This borrows search, consumes cursor entries and frees only temporary name storage.
int gtl_CFileIoDeviceDVD_FindNextFile(void* unusedDevice, DVDFile* search, FindData* outEntry) {
    if ((unsigned int)search == kInvalidSearchHandle) { File_SetLastError(kInvalidHandleError); return 0; }
    int found = 0;
    DirEntry804130C8 directoryEntry;
    // Phase 1: advance first, then copy this name into an owned temporary string.
    while (DVDReadDir(search, (Entry*)&directoryEntry) == kDvdDirectoryEntryRead) {
        StringRep candidateName;
        *(unsigned int*)&candidateName = 0; candidateName.heapLength = 0; candidateName.data = 0;
        const char* name = directoryEntry.name;
        unsigned int sourceNameLength = strlen(name);
        FileIOString_FitCapacityForLength(&candidateName, sourceNameLength);
        char copyAllocatorTag = 0;
        FileIOString_ReplaceRangeWithAllocator(&candidateName, 0, 0, name, name + sourceNameLength, &copyAllocatorTag);
        // Extensionless entries get the shared dot suffix for matching only.
        if (!strchr(name, '.')) {
            unsigned int nameLength = candidateName.isLong ? candidateName.heapLength : (candidateName.rest >> kShortStringLengthShift) & kShortStringLengthMask;
            char suffixAllocatorTag = 0;
            FileIOString_ReplaceRangeWithAllocator(&candidateName, nameLength, 0, fileSearchExtensionlessDotSuffix, fileSearchExtensionlessDotSuffix + strlen(fileSearchExtensionlessDotSuffix), &suffixAllocatorTag);
        }
        int candidateUsesHeap = candidateName.isLong;
        char* patternCursor = search->path.isLong ? search->path.data : (char*)&search->path + 1;
        char* nameCursor = candidateUsesHeap ? candidateName.data : (char*)&candidateName + 1;
        // Phase 2: case-sensitive byte matching, without backtracking. '?' consumes
        // one byte. A run of '*' scans to the next literal pattern byte (even '?').
        // The loop cannot enter after either string ends: a trailing '*' is not
        // consumed when the name is already empty. Preserve this limited behavior.
        while (*patternCursor != 0 && *nameCursor != 0) {
            if (*patternCursor == *nameCursor || *patternCursor == '?') {
                ++patternCursor; ++nameCursor;
            } else if (*patternCursor != '*') {
                break;
            }
            if (*patternCursor == '*') {
                do { ++patternCursor; } while (*patternCursor == '*');
                char nextPatternByte = *patternCursor;
                while (*nameCursor != nextPatternByte && *nameCursor != 0) ++nameCursor;
            }
        }
        int match = (*patternCursor == 0 && *nameCursor == 0);
        // No temporary name ownership escapes the iteration, including on a match.
        if (candidateUsesHeap) FileIO_Free(candidateName.data);
        if (match) { found = 1; break; }
    }
    // Phase 3: publish the original DVD name, without the temporary dot suffix.
    // strcpy assumes the API's name fits the 0x114-byte result field.
    if (!found) { File_SetLastError(kNoSearchError); return 0; }
    memset(outEntry, 0, sizeof(FindData));
    strcpy(outEntry->name, directoryEntry.name);
    outEntry->flags |= kEntryFound;
    if (directoryEntry.isDir == 1) outEntry->flags |= kDirectoryAttribute;
    File_SetLastError(kNoSearchError);
    return 1;
}
}
