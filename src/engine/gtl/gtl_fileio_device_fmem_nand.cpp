#include "types.h"
#include "sdk_protos.h"
#include "classes/FmemFileState.h"
#include "classes/FmemOperationCallback.h"

extern "C" {
extern void* memset(void*, int, unsigned int);
extern void* memcpy(void*, const void*, unsigned int);

extern void File_SetLastError(int x);
extern void* FileIO_Alloc(unsigned int size);
extern void FileIO_Free(void* p);
extern char* strncpy(char* dst, const char* devicePath, size_t n);
extern char* strchr(const char* s, int c);
extern size_t strlen(const char* s);

extern void NANDSetUserData(void* a, void* b);
extern int NANDCreateAsync(void* a, int b, int c, void* d, void* e);
extern int NANDOpenAsync(void* a, void* b, int c, void* d, void* e);

extern void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void* self, void* arg);

extern char __vt__Q23gtl19CCallBackCreateFile[];
extern char __vt__Q23gtl17CCallBackOpenFile[];
// Shared in-flight NAND request: callback, optional create workspace, and its size.
struct FmemAsyncOperation {
    void* callback;
    void* workspace;
    int workspaceBytes;
};
struct FmemCreateCallback {
    FmemOperationCallback base;
    unsigned char accessMode;
    char padding[3];
    int completionArgument;
};
extern FmemAsyncOperation s_fmemAsyncOperation;
enum {
    kPathCapacity = 0x104,
    kCreateMode = 2,
    kNandReadAccess = 1,
    kNandWriteAccess = 2,
    kNandFilePermissions = 0x30,
    kNandCommandBytes = 0xC4,
    kNandAlreadyExists = -6,
    kCreateSubmissionError = 0x52,
    kOpenSubmissionError = 0x6E,
    kNoError = 0
};
static const unsigned int kReadFlag = 0x80000000u;
static const unsigned int kWriteFlag = 0x40000000u;
// The NAND library command block (opaque storage) shared by every request.
extern char s_nandAsyncCommandBlock[];
// Slot 3 of a callback object's vtable is its "release/destroy" entry, called when a submission is rejected.
enum { kCallbackReleaseSlot = 3 };

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Open or create a NAND file asynchronously, publishing callback storage and cleaning up rejected submissions.
int gtl_CFileIoDeviceFMEM_CreateOrOpenFileAsync(void* self, const char* devicePath, int flags, int /*unused*/, int mode, int /*unused*/, int /*unused*/, int callbackArgument) {
    // NOTE: the three locals below are deliberately reused for several unrelated values (see the comments at each
    // reuse); separate locals change register allocation and break the match.
    // Phase 1: copy the path and translate the open flags into NAND access bits.
    // Phase 2: allocate the file state.  Phase 3: build the create/open callback and submit the NAND request.
    // Returns the new file state, or -1 (with File_SetLastError) when the NAND call rejects the submission.
    int flagsOrCallbackAddress = flags;
    int modeOrCallbackAddress = mode;
    int completionArgumentOrPathBytes = callbackArgument;

    char path[kPathCapacity];
    strncpy(path, devicePath, kPathCapacity);
    char* nandPath = strchr(path, '/');

    unsigned char accessMode = 0;
    if (flagsOrCallbackAddress & kReadFlag) accessMode |= kNandReadAccess;
    if (flagsOrCallbackAddress & kWriteFlag) accessMode |= kNandWriteAccess;

    void* fileState = FileIO_Alloc(sizeof(FmemFileState));
    if (fileState) {
        AsFmemFileState(fileState)->operationState = 0; // 0x8041D0E4: allocated 0x98-byte FMEM file state
        AsFmemFileState(fileState)->callbackArgument = 0;
        memset(fileState, 0, sizeof(AsFmemFileState(fileState)->pad_000));
    }
    int initialState = 0;
    AsFmemFileState(fileState)->operationState = initialState;

    if (modeOrCallbackAddress == kCreateMode) {
        void* callback = FileIO_Alloc(sizeof(FmemCreateCallback));
        // Create path (mode == kCreateMode): the callback also owns a heap copy of the path for the later open.
        // Reuse the dispatch scratch as the callback address; separate locals change register allocation.
        modeOrCallbackAddress = (int)callback;
        flagsOrCallbackAddress = modeOrCallbackAddress;
        if (modeOrCallbackAddress) {
            AsFmemOperationCallback(callback)->vtable = (void*)__vt__Q23gtl19CCallBackCreateFile;
            *(void**)&AsFmemOperationCallback(callback)->argument = fileState;
            AsFmemOperationCallback(callback)->callbackPayload = initialState;
            ((FmemCreateCallback*)callback)->accessMode = accessMode;
            ((FmemCreateCallback*)callback)->completionArgument = completionArgumentOrPathBytes;
            int len = (int)strlen(nandPath);
            // The completion argument has been saved; reuse its scratch for the path byte count.
            completionArgumentOrPathBytes = len;
            void* pathCopy = FileIO_Alloc(len + 1);
            *(void**)&AsFmemOperationCallback(callback)->callbackPayload = pathCopy;
            memcpy(pathCopy, nandPath, completionArgumentOrPathBytes + 1);
        }

        FmemAsyncOperation* operation = &s_fmemAsyncOperation;
        operation->callback = (void*)modeOrCallbackAddress;
        operation->workspaceBytes = kNandCommandBytes;
        int workspaceBytes = operation->workspaceBytes;
        operation->workspace = FileIO_Alloc(workspaceBytes);
        NANDSetUserData(s_nandAsyncCommandBlock, (void*)operation);

        int result = NANDCreateAsync(nandPath, kNandFilePermissions, 0, (void*)gtl_CFileIoDeviceFMEM_DispatchNandCallback, s_nandAsyncCommandBlock);
        if (result != 0 && result != kNandAlreadyExists) {
            File_SetLastError(kCreateSubmissionError);
            void* workspace = operation->workspace;
            FileIO_Free(workspace);
            operation->workspace = 0;
            void** callbackVtable = *(void***)modeOrCallbackAddress;
            ((void(*)(void*))callbackVtable[kCallbackReleaseSlot])((void*)modeOrCallbackAddress);
            if (fileState) FileIO_Free(fileState);
            return -1;
        }
        File_SetLastError(kNoError);
        return (int)fileState;
    } else {
        void* callback = FileIO_Alloc(sizeof(FmemOperationCallback));
        modeOrCallbackAddress = (int)callback;
        if (modeOrCallbackAddress) {
            // The empty try/catch only reproduces the compiler's exception-table entry for this store.
            try {
                AsFmemOperationCallback(callback)->vtable = (void*)__vt__Q23gtl17CCallBackOpenFile;
            } catch (...) {
            }
            *(void**)&AsFmemOperationCallback(callback)->argument = fileState;
            AsFmemOperationCallback(callback)->callbackPayload = completionArgumentOrPathBytes;
        }

        FmemAsyncOperation* operation = &s_fmemAsyncOperation;
        operation->callback = callback;
        operation->workspace = 0;
        operation->workspaceBytes = 0;
        NANDSetUserData(s_nandAsyncCommandBlock, (void*)operation);

        int result = NANDOpenAsync(nandPath, fileState, accessMode, (void*)gtl_CFileIoDeviceFMEM_DispatchNandCallback, s_nandAsyncCommandBlock);
        if (result != 0) {
            File_SetLastError(kOpenSubmissionError);
            void** callbackVtable = *(void***)modeOrCallbackAddress;
            ((void(*)(void*))callbackVtable[kCallbackReleaseSlot])((void*)modeOrCallbackAddress);
            if (fileState) FileIO_Free(fileState);
            return -1;
        }
        File_SetLastError(kNoError);
        return (int)fileState;
    }
    // Unreachable (both branches return); the (void) uses below only keep the unused parameters referenced.
    (void)self;
    (void)devicePath;
    (void)flags;
    (void)mode;
    (void)callbackArgument;
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset
} // extern "C"
