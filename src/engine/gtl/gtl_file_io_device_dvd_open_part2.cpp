// Translation unit 0x8041364C-0x80413768 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

struct DVDFile {
    char sdkFileInfo[0x3C];
    int entryNumber;
    // Same request tail as classes/ChunkedDvdReadRequest.h.
    int bytesRead;
    int requestedLength;
    int nextBuffer;
    int nextOffset;
    int priority;
    int completionArg;
    int completionArg2;
};

enum { kFileSuccess = 0, kFileNotFound = 2, kInvalidEntryNumber = -1,
       kDevicePathCapacity = 0x100 };
typedef char DVDFile_size_check[sizeof(DVDFile) == 0x5C ? 1 : -1];

extern "C" {
int DVDConvertPathToEntrynum(char*);
int DVDFastOpen(int, DVDFile*);
DVDFile* FileIO_Alloc(int);
void FileIO_Free(DVDFile*);
void File_SetLastError(int);

// Open the DVD path after its device prefix, then invoke completion synchronously.
// path is borrowed, NUL-terminated, includes '/', and must fit in 256 bytes with NUL.
// self and unused3..7 are ignored; no access/open-mode validation is done here.
// Returns an owned handle to be closed/freed by the caller, or (DVDFile*)-1 on error.
// The optional callback runs only on success, before return, with (0,0,file,context).
// Allocation is assumed to succeed: DVDFastOpen is still called if allocation is null.
DVDFile* gtl_CFileIoDeviceDVD_OpenWithSynchronousCallback(void* self, const char* path,
                                      int unused3, int unused4, int unused5, int unused6, int unused7,
                                      void (*callback)(int, int, DVDFile*, void*), void* context) {
    // Resolve the slash-prefixed device path to a DVD directory entry before allocating.
    char pathCopy[kDevicePathCapacity];
    strcpy(pathCopy, path);
    char* dvdPath = strchr(pathCopy, '/');
    DVDFile* file;
    int entryNumber;
    entryNumber = DVDConvertPathToEntrynum(dvdPath);
    if (entryNumber == kInvalidEntryNumber) {
        File_SetLastError(kFileNotFound);
        return (DVDFile*)-1;
    }
    // Start an empty request tail. The SDK file-info prefix is initialized by FastOpen.
    file = FileIO_Alloc(sizeof(DVDFile));
    if (file != 0) {
        file->entryNumber = kInvalidEntryNumber;
        file->bytesRead = 0;
        file->requestedLength = 0;
        file->nextBuffer = 0;
        file->nextOffset = 0;
        file->priority = 0;
        file->completionArg = 0;
        file->completionArg2 = 0;
    }
    // On open failure release the wrapper; both lookup/open failures report not found.
    if (DVDFastOpen(entryNumber, file) == 0) {
        if (file != 0) FileIO_Free(file);
        File_SetLastError(kFileNotFound);
        return (DVDFile*)-1;
    }
    // Publish the opened entry and notify synchronously while the handle is valid.
    file->entryNumber = entryNumber;
    File_SetLastError(kFileSuccess);
    if (callback != 0)
        callback(0, 0, file, context);
    return file;
}
}
