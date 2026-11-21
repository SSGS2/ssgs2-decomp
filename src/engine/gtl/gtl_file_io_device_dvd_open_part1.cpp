// Translation unit 0x80412858-0x80412938 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

struct DVDFile {
    char dvdFileInfo[0x3C]; // SDK DVDFileInfo, initialized by DVDFastOpen.
    int entryNumber;
    int deviceState[7]; // Zeroed wrapper state; individual roles are not established here.
};

enum FileError { kFileOk = 0, kFileNotFound = 2, kFileOpenFailed = 0x6E };
enum { kPathBufferSize = 0x110, kInvalidEntryNumber = -1 };
typedef char DVDFile_size_check[(sizeof(DVDFile) == 0x5C) ? 1 : -1];

extern "C" {
int DVDConvertPathToEntrynum(char*);
int DVDFastOpen(int, DVDFile*);
DVDFile* FileIO_Alloc(int);
void FileIO_Free(DVDFile*);
void File_SetLastError(int);
}

// Resolve the slash-prefixed DVD path, allocate its wrapper, and open the SDK file.
// Failures return the device API's (DVDFile*)-1 sentinel and set its last error.
// path must be NUL-terminated, shorter than kPathBufferSize, and contain '/'.
// Text before the first slash is discarded. self is unused. The returned wrapper
// belongs to the caller's device-close path; allocation failure has no local handler.
extern "C" DVDFile* gtl_CFileIoDeviceDVD_OpenFile(void* self, const char* path) {
    // Resolve the device-relative path before allocating any wrapper storage.
    char buffer[kPathBufferSize];
    strcpy(buffer, path);
    char* slash = strchr(buffer, '/');
    DVDFile* file;
    int index = DVDConvertPathToEntrynum(slash);
    if (index == kInvalidEntryNumber) {
        File_SetLastError(kFileNotFound);
        return (DVDFile*)-1;
    }
    // Initialize the wrapper only; DVDFastOpen fills the embedded SDK DVDFileInfo.
    file = FileIO_Alloc(sizeof(DVDFile));
    if (file != 0) {
        file->entryNumber = kInvalidEntryNumber;
        file->deviceState[0] = 0;
        file->deviceState[1] = 0;
        file->deviceState[2] = 0;
        file->deviceState[3] = 0;
        file->deviceState[4] = 0;
        file->deviceState[5] = 0;
        file->deviceState[6] = 0;
    }
    // On SDK failure, release the wrapper and publish the device-level error.
    if (DVDFastOpen(index, file) == 0) {
        if (file != 0)
            FileIO_Free(file);
        File_SetLastError(kFileOpenFailed);
        return (DVDFile*)-1;
    }
    // Publish the resolved entry only after a successful open.
    file->entryNumber = index;
    File_SetLastError(kFileOk);
    return file;
}
