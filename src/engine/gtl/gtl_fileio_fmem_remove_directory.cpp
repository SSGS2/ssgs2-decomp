// Translation unit 0x8041CCF0-0x8041D0DC (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

extern "C" {
void File_SetLastError(int x);
void *FileIO_Alloc(unsigned int size);
int NANDDelete(char *nandPath);
int NANDDeleteAsync(char *nandPath, void *a, void *b);
void NANDSetUserData(void*, void*);
void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void *self, void *arg);

struct NandDeleteCallbackInterface {
    virtual void Reserved0();
    virtual void Release();
    virtual void Complete();
    virtual void Reserved3();
};

extern char __vt__Q23gtl24CCallBackRemoveDirectory[];
extern char s_fmemAsyncOperation[];
extern char s_nandAsyncCommandBlock[];

// Translation policy: success 0 -> 0; ordinary -n -> 0xC350+n (n=1..15, except 7).
// Unknown -7/-64/unlisted -> 0xC360; fatal -128 -> 0xC361.
// NAND results -1..-15 map to 0xC351..0xC35F, except -7 (unmapped -> unknown).
// -64/unlisted results map to 0xC360; -128 maps to 0xC361; zero is success.
enum NandDeleteResult {
    kNandSuccess = 0, kNandAccess = -1, kNandAlloc = -2,
    kNandBusy = -3, kNandCorrupt = -4, kNandEccCritical = -5,
    kNandExists = -6, kNandUnmapped = -7, kNandInvalid = -8, kNandMaxBlocks = -9,
    kNandMaxDescriptors = -10, kNandMaxFiles = -11, kNandNoExists = -12, kNandNotEmpty = -13,
    kNandOpenFiles = -14, kNandAuthentication = -15,
    kNandUnknown = -64, kNandFatal = -128
};
enum FileDeleteError {
    kDeleteOk = 0, kDeleteAccess = 0xC351, kDeleteAlloc = 0xC352,
    kDeleteBusy = 0xC353, kDeleteCorrupt = 0xC354, kDeleteEccCritical = 0xC355,
    kDeleteExists = 0xC356, kDeleteInvalid = 0xC358, kDeleteMaxBlocks = 0xC359,
    kDeleteMaxDescriptors = 0xC35A, kDeleteNoExists = 0xC35C, kDeleteNotEmpty = 0xC35D,
    kDeleteOpenFiles = 0xC35E, kDeleteAuthentication = 0xC35F,
    kDeleteUnknown = 0xC360, kDeleteFatal = 0xC361,
    kDeleteMaxFiles = 0xC35B
};
struct RemoveDirectoryCallback {
    void* vtable;
    int completionCallback;
    void* callbackContext;
};
struct FmemAsyncDeleteState {
    void* callback;
    // DispatchNandCallback forwards these two optional arguments to the callback.
    void* callbackArgument0;
    void* callbackArgument1;
};

// Translate the NAND status for both synchronous deletion and asynchronous submission.
// The comparison shape preserves code generation; the enums define the error policy.
// Equivalent flat mapping (nandResult -> lastError), for auditing the nested tests below:
//    0 -> 0 (ok)            -1 -> 0xC351   -2 -> 0xC352   -3 -> 0xC353   -4 -> 0xC354
//   -5 -> 0xC355            -6 -> 0xC356   -8 -> 0xC358   -9 -> 0xC359  -10 -> 0xC35A
//  -11 -> 0xC35B  -12 -> 0xC35C  -13 -> 0xC35D  -14 -> 0xC35E  -15 -> 0xC35F
//  -128 -> 0xC361 (fatal); -7, -64 and any other value -> 0xC360 (unknown).
// Block layout: the top-level tests split the range at -9, then at -3 / -15; labels below set the result.
static inline void TranslateNandDeleteResult(int nandResult, int& lastError) {
    do {
        if (nandResult != kNandMaxBlocks) {
            if (nandResult < kNandMaxBlocks) {
                if (nandResult == kNandAuthentication) goto L_Authentication;
                if (nandResult < kNandAuthentication) {
                    if (nandResult == kNandUnknown) goto L_Unknown;
                    // Original 0x8041CCF0: both bounds share the default result.
                    if (nandResult >= kNandUnknown) goto L_DefaultError;
                    if (nandResult != kNandFatal) goto L_DefaultError;
                    goto L_Fatal;
                } else {
                    if (nandResult == kNandNoExists) goto L_NoExists;
                    if (nandResult < kNandNoExists) {
                        if (nandResult < kNandNotEmpty) goto L_OpenFiles;
                        goto L_NotEmpty;
                    } else {
                        if (nandResult < kNandMaxDescriptors) goto L_MaxFiles;
                        goto L_MaxDescriptors;
                    }
                }
            } else {
                if (nandResult == kNandBusy) goto L_Busy;
                if (nandResult < kNandBusy) {
                    if (nandResult == kNandExists) goto L_Exists;
                    if (nandResult < kNandExists) {
                        if (nandResult < kNandUnmapped) goto L_Invalid;
                        goto L_DefaultError;
                    } else {
                        if (nandResult < kNandCorrupt) goto L_EccCritical;
                        goto L_Corrupt;
                    }
                } else {
                    if (nandResult != kNandSuccess) {
                        if (nandResult >= kNandSuccess) goto L_DefaultError;
                        if (nandResult < kNandAccess) goto L_Alloc;
                        goto L_Access;
                    }
                }

            }
            lastError = kDeleteOk;
            break;
        L_Access:
            lastError = kDeleteAccess;
            break;
        L_Alloc:
            lastError = kDeleteAlloc;
            break;
        L_Busy:
            lastError = kDeleteBusy;
            break;
        L_Corrupt:
            lastError = kDeleteCorrupt;
            break;
        L_EccCritical:
            lastError = kDeleteEccCritical;
            break;
        L_Exists:
            lastError = kDeleteExists;
            break;
        L_Invalid:
            lastError = kDeleteInvalid;
            break;
        }
        lastError = kDeleteMaxBlocks;
        break;
    L_MaxDescriptors:
        lastError = kDeleteMaxDescriptors;
        break;
    L_MaxFiles:
        lastError = kDeleteMaxFiles;
        break;
    L_NoExists:
        lastError = kDeleteNoExists;
        break;
    L_NotEmpty:
        lastError = kDeleteNotEmpty;
        break;
    L_OpenFiles:
        lastError = kDeleteOpenFiles;
        break;
    L_Unknown:
        lastError = kDeleteUnknown;
        break;
    L_Authentication:
        lastError = kDeleteAuthentication;
        break;
    L_Fatal:
        lastError = kDeleteFatal;
        break;
    L_DefaultError:
        lastError = kDeleteUnknown;
        break;
    } while (0);
}

// Async state layout (all three are statics in the game's data, not on the stack):
//   s_fmemAsyncOperation   = { callback wrapper*, argument0 = 0, argument1 = 0 }   (FmemAsyncDeleteState)
//   s_nandAsyncCommandBlock = NAND SDK command block; NANDSetUserData points it at s_fmemAsyncOperation.
//   wrapper (RemoveDirectoryCallback) = { vtable, caller's completionCallback, caller's callbackContext }
// When NANDDeleteAsync finishes, the SDK calls gtl_CFileIoDeviceFMEM_DispatchNandCallback, which reads
// the user data and invokes the wrapper. Whether NANDDeleteAsync can complete before returning is not
// established here; the wrapper is only released by this function on a rejected submission (return 0).
// The path buffer is on the stack and NANDDeleteAsync is given a pointer into it, so the SDK must
// copy the path during submission (this code relies on that; it is not verified here).
// Delete a NAND path synchronously or submit an asynchronous delete with callback ownership.
// path must contain '/' within its first 0x104 bytes; only the suffix starting there reaches NAND.
// strncpy does not guarantee termination for longer paths. self is unused.
// completionCallback == 0 blocks and returns the delete result; otherwise return 1 means queued.
// The optional callback/context are retained in an allocated wrapper, released on rejection or
// by completion processing. This shared async command block must not be reused while pending.
int gtl_CFileIoDeviceFMEM_RemoveDirectory(void *self, const char *path, int completionCallback, void *callbackContext) {
    enum { kPathBufferSize = 0x104 };
    char pathBuffer[kPathBufferSize];
    // Strip the file-layer device prefix without allocating a persistent path string.
    strncpy(pathBuffer, path, kPathBufferSize);
    char *nandPath = strchr(pathBuffer, '/');
    if (completionCallback == 0) {
        int nandResult = NANDDelete(nandPath);
        int lastError;
        TranslateNandDeleteResult(nandResult, lastError);
        File_SetLastError(lastError);
        if (nandResult != kNandSuccess) return 0;
        return 1;
    } else {
        // Publish the callback wrapper before submission; a rejected request releases it below.
        // Allocation failure still publishes null and submits; rejection would then
        // dereference null in Release. Successful allocation is an existing precondition.
        RemoveDirectoryCallback* callback = (RemoveDirectoryCallback*)FileIO_Alloc(sizeof(RemoveDirectoryCallback));
        if (callback) {
            callback->vtable = __vt__Q23gtl24CCallBackRemoveDirectory;
            callback->completionCallback = completionCallback;
            callback->callbackContext = callbackContext;
        }
        // Publish [wrapper, argument0=0, argument1=0] at byte offsets [0,4,8].
        // Wrapper stores callback/context words; it does not own the path buffer.
        // Preserve byte-based stores: a volatile typed view folds the first address into stwu.
        volatile char* operation = s_fmemAsyncOperation;
        *(void**)operation = callback;
        *(int*)(operation + (int)&((FmemAsyncDeleteState*)0)->callbackArgument0) = 0;
        *(int*)(operation + (int)&((FmemAsyncDeleteState*)0)->callbackArgument1) = 0;
        NANDSetUserData(s_nandAsyncCommandBlock, &s_fmemAsyncOperation);
        int nandResult = NANDDeleteAsync(nandPath, (void *)gtl_CFileIoDeviceFMEM_DispatchNandCallback, s_nandAsyncCommandBlock);
        int lastError;
        TranslateNandDeleteResult(nandResult, lastError);
        // Submission status is reported now; actual deletion finishes through the dispatcher.
        File_SetLastError(lastError);
        if (nandResult != kNandSuccess) {
            ((NandDeleteCallbackInterface*)callback)->Release();
            return 0;
        }
        return 1;
    }
}
// Compile-time guards for private ABI views used above.
#define LOCAL_FIELD(t, f, o) typedef char local_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD(RemoveDirectoryCallback, callbackContext, 8);
LOCAL_FIELD(FmemAsyncDeleteState, callbackArgument0, 4);
LOCAL_FIELD(FmemAsyncDeleteState, callbackArgument1, 8);
#undef LOCAL_FIELD

} // extern "C"
