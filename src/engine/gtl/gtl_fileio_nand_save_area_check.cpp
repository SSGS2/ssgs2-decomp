// The FMEM file device's save-area check: asks NAND whether there is room for a number of
// blocks and files, synchronously or through a callback, and maps the answer to a result code.
// It also holds the release routine for the move-with-progress callback.

#include "classes/GtlFileIoDeviceFMEM.h"
#include "sdk_protos.h"
#include "classes/FmemOperationCallback.h"
#include "classes/gtl_CCallBackMoveFileWithProgress.h"

extern "C" {
void* FileIO_Alloc(u32 size);
void FileIO_Free(void* p);
int NANDCheck(u32 neededBlocks, u32 neededFiles, u32* answer);
void NANDSetUserData(void* commandBlock, void* userData, void* resultBuffer);
int NANDCheckAsync(u32 neededBlocks, u32 neededFiles, void* answer, void* callback, void* commandBlock);

extern char __vt__Q23gtl25CCallBackRVLCheckSaveArea[0x14];

// For this callback specialization, argument stores the caller's callback address.
typedef FmemOperationCallback SaveAreaCallback;
struct FmemAsyncOperation {
    SaveAreaCallback* callback;
    u32* buffer;
    u32 size;
};
FmemAsyncOperation s_fmemAsyncOperation;
// NAND error meanings follow include/revolution/NAND/nand.h; wrapper codes retain the 0xC350 base.
enum SaveAreaResult {
    kSaveAreaSuccess = 0,
    kSaveAreaAccessDenied = 0xC351,
    kSaveAreaAllocationFailed = 0xC352,
    kSaveAreaBusy = 0xC353,
    kSaveAreaCorrupt = 0xC354,
    kSaveAreaEccCritical = 0xC355,
    kSaveAreaAlreadyExists = 0xC356,
    kSaveAreaInvalid = 0xC358,
    kSaveAreaMaxBlocks = 0xC359,
    kSaveAreaMaxFileDescriptors = 0xC35A,
    kSaveAreaMaxFiles = 0xC35B,
    kSaveAreaNoSuchFile = 0xC35C,
    kSaveAreaNotEmpty = 0xC35D,
    kSaveAreaOpenFile = 0xC35E,
    kSaveAreaAuthentication = 0xC35F,
    kSaveAreaUnknown = 0xC360,
    kSaveAreaFatal = 0xC361,
};
enum NandSaveAreaResult {
    kNandAccess = -1,
    kNandAllocationFailed = -2,
    kNandBusy = -3,
    kNandCorrupt = -4,
    kNandEccCritical = -5,
    kNandExists = -6,
    kNandInvalid = -8,
    kNandMaxBlocks = -9,
    kNandMaxDescriptors = -10,
    kNandMaxFiles = -11,
    kNandNoSuchFile = -12,
    kNandNotEmpty = -13,
    kNandOpenFile = -14,
    kNandAuthentication = -15,
    kNandUnknown = -64,
    kNandFatal = -128
};
static const int kNandClusterByteShift = 14;
static const u32 kSaveAreaNotChecked = 0xFFFFFFFFu;
char s_nandAsyncCommandBlock[256];

} // extern "C"

// Release a callback allocation when invoked as a deleting destructor.
void* gtl::CCallBackMoveFileWithProgress::Destroy(int deleting) {
    if (this != 0 && deleting > 0) {
        FileIO_Free(this);
    }
    return this;
}

extern "C" {

#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Count required NAND clusters for the file sizes, then check capacity synchronously or submit a callback.
int gtl_FileIO_CheckSaveArea(int unused, u32* words, u32 wordCount, u32* checkResultFlags, int callbackAddress,
                             int userArgument) {
    u32 checkedWordCount;
    u32 requiredBlocks;
    int savedCallbackAddress;
    int savedUserArgument;
    SaveAreaCallback* callback;
    requiredBlocks = 0;
    checkedWordCount = wordCount;
    savedCallbackAddress = callbackAddress;
    savedUserArgument = userArgument;
    (void)unused;
    if (checkedWordCount > 0) {
        u32 fourWordGroups = checkedWordCount >> 2;
        if (fourWordGroups != 0) {
            do {
                u32 word;
                word = *words++;
                requiredBlocks += (((-(s32)word) | word) >> 31) + (word >> kNandClusterByteShift);
                word = *words++;
                requiredBlocks += (((-(s32)word) | word) >> 31) + (word >> kNandClusterByteShift);
                word = *words++;
                requiredBlocks += (((-(s32)word) | word) >> 31) + (word >> kNandClusterByteShift);
                word = *words++;
                requiredBlocks += (((-(s32)word) | word) >> 31) + (word >> kNandClusterByteShift);
            } while (--fourWordGroups);
        }
        u32 remainingWords = checkedWordCount & 3;
        if (remainingWords != 0) {
            do {
                u32 word = *words++;
                requiredBlocks += (((-(s32)word) | word) >> 31) + (word >> kNandClusterByteShift);
            } while (--remainingWords);
        }
    }

    if (savedCallbackAddress == 0) {
        *checkResultFlags = kSaveAreaNotChecked;
        int nandResult = NANDCheck(requiredBlocks, checkedWordCount, checkResultFlags);
        if (nandResult == 0)
            return 0;
        switch (nandResult) {
        case 0:
            return 0;
        case kNandAccess:
            return kSaveAreaAccessDenied;
        case kNandAllocationFailed:
            return kSaveAreaAllocationFailed;
        case kNandBusy:
            return kSaveAreaBusy;
        case kNandCorrupt:
            return kSaveAreaCorrupt;
        case kNandEccCritical:
            return kSaveAreaEccCritical;
        case kNandExists:
            return kSaveAreaAlreadyExists;
        case kNandInvalid:
            return kSaveAreaInvalid;
        case kNandMaxBlocks:
            return kSaveAreaMaxBlocks;
        case kNandMaxDescriptors:
            return kSaveAreaMaxFileDescriptors;
        case kNandMaxFiles:
            return kSaveAreaMaxFiles;
        case kNandNoSuchFile:
            return kSaveAreaNoSuchFile;
        case kNandNotEmpty:
            return kSaveAreaNotEmpty;
        case kNandOpenFile:
            return kSaveAreaOpenFile;
        case kNandAuthentication:
            return kSaveAreaAuthentication;
        case kNandUnknown:
            return kSaveAreaUnknown;
        case kNandFatal:
            return kSaveAreaFatal;
        default:
            return kSaveAreaUnknown;
        }
    } else {
        int nandResult;
        callback = (SaveAreaCallback*)FileIO_Alloc(sizeof(SaveAreaCallback));
        if (callback != 0) {
            callback->vtable = __vt__Q23gtl25CCallBackRVLCheckSaveArea;
            callback->argument = (u32)savedCallbackAddress;
            callback->callbackPayload = (u32)savedUserArgument;
        }
        s_fmemAsyncOperation.callback = callback;
        s_fmemAsyncOperation.size = sizeof(u32);
        s_fmemAsyncOperation.buffer = (u32*)FileIO_Alloc(s_fmemAsyncOperation.size);
        *(u32*)s_fmemAsyncOperation.buffer = kSaveAreaNotChecked;
        NANDSetUserData(s_nandAsyncCommandBlock, &s_fmemAsyncOperation, (void*)s_fmemAsyncOperation.buffer);
        nandResult = NANDCheckAsync(requiredBlocks, checkedWordCount, (void*)s_fmemAsyncOperation.buffer,
                                    (void*)gtl::CFileIoDeviceFMEM::DispatchNandCallback, s_nandAsyncCommandBlock);
        if (nandResult == 0) {
            return 0;
        }
        FileIO_Free((void*)s_fmemAsyncOperation.buffer);
        s_fmemAsyncOperation.buffer = 0;
        ((void (**)(void*))(*(void**)callback))[3](callback);
        switch (nandResult) {
        case 0:
            return 0;
        case kNandAccess:
            return kSaveAreaAccessDenied;
        case kNandAllocationFailed:
            return kSaveAreaAllocationFailed;
        case kNandBusy:
            return kSaveAreaBusy;
        case kNandCorrupt:
            return kSaveAreaCorrupt;
        case kNandEccCritical:
            return kSaveAreaEccCritical;
        case kNandExists:
            return kSaveAreaAlreadyExists;
        case kNandInvalid:
            return kSaveAreaInvalid;
        case kNandMaxBlocks:
            return kSaveAreaMaxBlocks;
        case kNandMaxDescriptors:
            return kSaveAreaMaxFileDescriptors;
        case kNandMaxFiles:
            return kSaveAreaMaxFiles;
        case kNandNoSuchFile:
            return kSaveAreaNoSuchFile;
        case kNandNotEmpty:
            return kSaveAreaNotEmpty;
        case kNandOpenFile:
            return kSaveAreaOpenFile;
        case kNandAuthentication:
            return kSaveAreaAuthentication;
        case kNandUnknown:
            return kSaveAreaUnknown;
        case kNandFatal:
            return kSaveAreaFatal;
        default:
            return kSaveAreaUnknown;
        }
    }
    return 0;
}
#pragma use_lmw_stmw off
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
} // extern "C"
