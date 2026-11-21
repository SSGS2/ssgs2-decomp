#include "sdk_protos.h"

extern "C" {
void* memcpy(void*, const void*, unsigned int);
int NANDMove(char*, char*);
void File_SetLastError(void* x);
extern char nandMoveCurrentDirectoryText;
}

enum NandMoveConstants {
    kPathSlash = '/', kNandPathBytes = 0x104,
    kFileNandErrorBase = 0xC350, kFileNandUnknownError = 0xC360,
    kFileNandFatalError = 0xC361
};

#pragma use_lmw_stmw on

// Move a NAND source into the destination directory; return 1 on success, 0 on error.
// Both borrowed paths must contain '/'; prefixes before the first slash are ignored.
// A trailing slash keeps the full destination; otherwise its final leaf is stripped.
// The leaf is never passed as a new filename. No path validation is performed.
extern "C" int gtl_CFileIoDeviceFMEM_MoveToDirectory(void* /*self*/, const char* sourcePath, const char* destinationPath) {
    char destinationDirectory[kNandPathBytes];

    // Phase 1: select the NAND path slices and isolate the destination's last component.
    register char* destinationLeaf;
    char* nandSource = strchr(sourcePath, kPathSlash);
    char* nandDestination = strchr(destinationPath, kPathSlash);
    char* lastSlash = strrchr(nandDestination, kPathSlash);

    if (strlen(lastSlash + 1) == 0) {
        destinationLeaf = 0;
    } else {
        destinationLeaf = lastSlash + 1;
    }

    // Phase 2: form the directory. Full-size strncpy does not guarantee a terminator.
    if (destinationLeaf == 0) {
        strncpy(destinationDirectory, nandDestination, kNandPathBytes);
    } else if (nandDestination == destinationLeaf) {
        // Preserved fallback: ordinary slash-containing strings do not reach this equality.
        strncpy(destinationDirectory, &nandMoveCurrentDirectoryText, kNandPathBytes);
    } else {
        size_t destinationLength = strlen(nandDestination);
        size_t leafLength = strlen(destinationLeaf);
        size_t directoryLength = destinationLength - (leafLength + 1);
        if (directoryLength > kNandPathBytes - 1) {
            directoryLength = kNandPathBytes - 1;
        }
        memcpy(destinationDirectory, nandDestination, (unsigned int)directoryLength);
        destinationDirectory[directoryLength] = 0;
    }

    // Phase 3: perform the move and map NAND errors to the file layer's error range.
    int nandStatus = NANDMove(nandSource, destinationDirectory);
    int savedStatus = nandStatus;

    int fileError;
    switch (nandStatus) {
        case 0: fileError = 0; break;
        case -1: fileError = kFileNandErrorBase + 1; break;
        case -2: fileError = kFileNandErrorBase + 2; break;
        case -3: fileError = kFileNandErrorBase + 3; break;
        case -4: fileError = kFileNandErrorBase + 4; break;
        case -5: fileError = kFileNandErrorBase + 5; break;
        case -6: fileError = kFileNandErrorBase + 6; break;
        case -8: fileError = kFileNandErrorBase + 8; break;
        case -9: fileError = kFileNandErrorBase + 9; break;
        case -10: fileError = kFileNandErrorBase + 10; break;
        case -11: fileError = kFileNandErrorBase + 11; break;
        case -12: fileError = kFileNandErrorBase + 12; break;
        case -13: fileError = kFileNandErrorBase + 13; break;
        case -14: fileError = kFileNandErrorBase + 14; break;
        case -15: fileError = kFileNandErrorBase + 15; break;
        case -64: fileError = kFileNandUnknownError; break;
        case -128: fileError = kFileNandFatalError; break;
        default: fileError = kFileNandUnknownError; break;
    }
    File_SetLastError((void*)fileError);

    return savedStatus == 0 ? 1 : 0;
}

#pragma use_lmw_stmw off
