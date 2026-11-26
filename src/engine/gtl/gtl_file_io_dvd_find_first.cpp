// DVD file device: starts a directory search with a wildcard pattern (FindFirst).
// It opens the directory, finds the first entry whose name matches, and returns a search handle.

#include "classes/GtlFileIoDeviceDVD.h"
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>

// 12-byte string ABI: long form owns data and uses lengthOrInlineBytes as length;
// short form stores up to the representation's inline limit starting at byte +1.
// rest's top seven bits carry short length. Do not treat inline bytes as a pointer.
struct StringRep {
    CW_BITS2(unsigned int isLong : 1, unsigned int rest : 31)
    unsigned int lengthOrInlineBytes;
    char* data;
};
// A successful search transfers this allocation (DVD iterator + owned pattern)
// to the caller as an integer handle. out contains a copy, not a pointer into it.
struct DVDFile {
    char directoryHandle[0x0C];
    StringRep searchPattern;
};
struct FindData {
    unsigned int flags;
    char reserved[0x28];
    char name[0x114];
};
struct Entry {
    char bytes[0x14];
};

enum {
    kFindResultBaseFlags = 1,
    kFindDirectory = 0x10,
    kPathNotFound = 2,
    kDirectoryOpenFailed = 0x6e,
    kSearchPathBytes = 0x110,
    kInlineLengthMask = 0x7F
};

extern "C" {
void* memset(void*, int, unsigned int);
void* FileIO_Alloc(unsigned int);
void FileIO_Free(void*);
int File_SetLastError(int);
int DVDConvertPathToEntrynum(char*);
int DVD_FastOpenDirectory(int, DVDFile*);
int DVDReadDir(DVDFile*, Entry*);
void FileIOString_ReplaceRangeWithAllocator(StringRep*, unsigned int, unsigned int, const char*, const char*, void*);
void FileIOString_FitCapacityForLength(StringRep*, unsigned int);
void* FileIOString_dtor(StringRep*, int);
extern char pathAndStringExceptionStrings[];

// DVDDirEntry as filled by DVDReadDir (DVDReadDir): the name is a pointer at +8.
struct DirEntry {
    unsigned int entryNum;
    int isDir;
    char* name;
};

// Free the owned long pattern and search allocation, including partial construction.
// This helper does not call a DVD close operation.
static void FreeSearch(DVDFile* file) {
    if (file) {
        if (file->searchPattern.isLong)
            FileIO_Free(file->searchPattern.data);
        FileIO_Free(file);
    }
}

} // extern "C"

// Open a directory search and return its first wildcard match, or -1 with the last error set.
// path/out must be nonnull. path must be NUL-terminated, contain '/', and fit in
// kSearchPathBytes; no validation or bounded copy is performed here. The prefix
// before the first '/' is ignored. A returned handle owns the search allocation;
// -1 with last error 0 means no match (including DVDReadDir stopping without 1).
// On failure out is untouched. Allocation failure has no separate recovery path.
int gtl::CFileIoDeviceDVD::FindFirst(const char* path, FindData* out) {
    // Split the borrowed input into a directory path and a copied search pattern.
    char pathBuffer[kSearchPathBytes];
    strcpy(pathBuffer, path);
    char* first = strchr(pathBuffer, '/');
    char* last = strrchr(pathBuffer, '/');
    // The directory handed to DVDConvertPathToEntrynum is the text from the first '/' (or "/" when
    // the only '/' is the last one); the pattern is what follows the last '/'.
    char* directory = first;
    if (first == last)
        directory = pathAndStringExceptionStrings;
    char* pattern = last + 1;
    *last = 0;
    DVDFile* file = (DVDFile*)FileIO_Alloc(sizeof(DVDFile));
    if (file) {
        file->searchPattern.isLong = 0;
        file->searchPattern.rest = 0;
        file->searchPattern.lengthOrInlineBytes = 0;
        file->searchPattern.data = 0;
        unsigned int currentStringLength = file->searchPattern.isLong
                                               ? file->searchPattern.lengthOrInlineBytes
                                               : ((file->searchPattern.rest >> 24) & kInlineLengthMask);
        char allocatorTag = 0;
        FileIOString_ReplaceRangeWithAllocator(&file->searchPattern, 0, currentStringLength, pattern,
                                               pattern + strlen(pattern), &allocatorTag);
    }
    // Resolve/open the directory; failures free the pattern and handle allocation.
    int index = DVDConvertPathToEntrynum(directory);
    if (index == -1) {
        FreeSearch(file);
        File_SetLastError(kPathNotFound);
        return -1;
    }
    if (!DVD_FastOpenDirectory(index, file)) {
        FreeSearch(file);
        File_SetLastError(kDirectoryOpenFailed);
        return -1;
    }
    int found = 0;
    DirEntry entry;
    // Read an entry, construct a temporary match string, then test the pattern.
    // Dotless names gain a trailing '.' ONLY for matching; the output keeps the original name.
    while (DVDReadDir(file, (Entry*)&entry) == 1) {
        StringRep query;
        *(unsigned int*)&query = 0;
        query.lengthOrInlineBytes = 0;
        query.data = 0;
        const char* name = entry.name;
        unsigned int nameLength = strlen(name);
        FileIOString_FitCapacityForLength(&query, nameLength);
        char allocatorTag = 0;
        FileIOString_ReplaceRangeWithAllocator(&query, 0, 0, name, name + nameLength, &allocatorTag);
        if (!strchr(name, '.')) {
            // The MSL inline length is bits 24..30 of word 0 (memory byte 0 of the big-endian word).
            unsigned int currentStringLength =
                query.isLong ? query.lengthOrInlineBytes : (query.rest >> 24) & kInlineLengthMask;
            char appendAllocatorTag = 0;
            FileIOString_ReplaceRangeWithAllocator(&query, currentStringLength, 0, ".", "." + 1, &appendAllocatorTag);
        }
        int queryIsLong = query.isLong;
        char* patternCursor = file->searchPattern.isLong ? file->searchPattern.data : (char*)&file->searchPattern + 1;
        char* nameCursor = queryIsLong ? query.data : (char*)&query + 1;
        // Case-sensitive, single-pass matcher: '?' consumes one character. Runs
        // of '*' skip to the FIRST literal next pattern byte (or end for trailing '*').
        // There is no backtracking; '*?' does not implement a general glob here.
        // A leading '*' also requires a nonempty candidate to enter this loop.
        while (*patternCursor != 0 && *nameCursor != 0) {
            if (*patternCursor == *nameCursor || *patternCursor == '?') {
                ++patternCursor;
                ++nameCursor;
            } else if (*patternCursor != '*') {
                break;
            }
            if (*patternCursor == '*') {
                do {
                    ++patternCursor;
                } while (*patternCursor == '*');
                char nextPatternChar = *patternCursor;
                while (*nameCursor != nextPatternChar && *nameCursor != 0)
                    ++nameCursor;
            }
        }
        int match = (*patternCursor == 0 && *nameCursor == 0);
        // Temporary candidate storage never escapes this iteration.
        if (queryIsLong)
            FileIO_Free(query.data);
        if (match) {
            found = 1;
            break;
        }
    }
    if (!found) {
        FreeSearch(file);
        File_SetLastError(0);
        return -1;
    }
    // Publish the original entry name and transfer the live iterator to the caller.
    memset(out, 0, sizeof(FindData));
    strcpy(out->name, entry.name);
    out->flags |= kFindResultBaseFlags; // Output name at +0x2C.
    if (entry.isDir == 1)
        out->flags |= kFindDirectory;
    File_SetLastError(0);
    return (int)file;
}
