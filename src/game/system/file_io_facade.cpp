// File I/O facade: sends reads, writes, seeks, deletes, moves and directory changes to the device
// registered for each path prefix, and manages file handles and the file-I/O lock.

#include <decomp/local_class.h>
// File I/O facade: handle/device dispatch, aligned read/write storage, asynchronous
// completion cleanup, device-prefix routing, and path normalization. No gameplay
// tuning constants. API lookup:
// - ReadFile/WriteFile: byte counts and buffer ownership; FileHandle.h is the
//   transfer-state schema. Direct-buffer bit bypasses allocation, not lifetime.
// - SeekFile/GetFileSize: backend results, NOT uniformly boolean status; see
//   each contract. gtl_CFile wrappers in gtl_cfile_io.cpp preserve the result-word ABI.
// - FileIO_PollAsyncTransferResult: despite its legacy name, the completion branch also
//   frees an async WRITE allocation when no read copy-back is pending.
// - GetDeviceTransferAlignment/DeleteFile/QueryDiskFreeSpace/MoveFile*: registered
//   device prefix selects backend. Path strings are borrowed NUL-terminated text.
// - SplitDeviceAndNormalizePath/SetCurrentDirectory: slash/dot normalization and
//   shared current directory; capacities include the terminator. FileIoPathStrings
//   below supplies device names and separators; this is not a filesystem parser.
#include "classes/FileRegistryTree.h"
#include "classes/FileRegistryNode.h"
#include "classes/FileHandle.h"
#include "sdk_protos.h"

extern "C" {
enum {
    kInvalidFileHandleError = 6,
    kPathCapacity = 0x104,
    kInlineStringLengthMask = 0x7F,
    kRegistryParentColorBit = 1,
    kDeviceQueryAlignmentSlot = 0x68 / 4,
    kDeviceDeletePathSlot = 0x38 / 4,
    kDeviceMovePathSlot = 0x44 / 4,
    kDeviceMoveWithProgressSlot = 0x4C / 4,
    kDeviceDiskFreeSpaceSlot = 0x50 / 4,
    kDeviceAsyncWriteSlot = 0x74 / 4,
    kDeviceWriteSlot = 0x1C / 4,
    kDeviceSetPathSlot = 0x88 / 4
};
static const unsigned int kFileAsync = 0x40000000u;
static const unsigned int kFileDirectBuffer = 0x20000000u;
// Handle API contract: -1 is the invalid sentinel tested here; a null handle
// is not validated. Each operation has its own return contract. Read/Write
// forward backend status separately from completed-byte outputs. GetFileSize
// returns the device's size result; SeekFile forwards its backend's result word
// and optional high word. Do not infer boolean success from all nonzero returns.
// The file-I/O mutex supports recursive acquisition. Guard holds one level;
// manual registry searches acquire/release a second level inside that guard.
// Registry string flags: bit31=heap string, otherwise low 7 bits of its first
// byte=inline length and characters start one byte after stringFlags.
// Tree header: registry->first begins traversal; &registry->root is the sentinel.
// Successor is right-child/leftmost, otherwise climb masked parent links until
// the current node is its parent's left child. Bit 0 of parentAndColor is color.
// The repeated searches below borrow registry nodes/devices and never free them.
struct FileIoAutoLockLayout {
    OSMutex* mutex;
    void* vtable;
};
// The singleton's mutex prefix is followed by registry state and the shared default-device path.
struct FileIoGlobalStorage {
    char mutex[0x18];
    char reserved18[0x0C];
    FileRegistryTree devices;
    char defaultDevicePath[kPathCapacity];
};
typedef char FileIoDefaultPathOffset[((int)&((FileIoGlobalStorage*)0)->defaultDevicePath == 0x30) ? 1 : -1];
extern OSMutex s_fileIoMutex;
extern char s_fileIoAutoLockMutexVTable[];
void* __dl__FPv(void*);
// Release one recursive mutex level and optionally free the guard allocation.
inline void* __dt__14CAutoLockMutexFv(void* self, int flags) {
    ((FileIoAutoLockLayout*)self)->vtable = (void*)s_fileIoAutoLockMutexVTable;
    OSUnlockMutex(((FileIoAutoLockLayout*)self)->mutex);
    if (flags > 0)
        __dl__FPv(self);
    return self;
}
struct CAutoLockMutex {
    OSMutex* mutex;
    void* vtable;
    // Acquire one mutex level for this guard's lifetime.
    CAutoLockMutex(OSMutex* m) {
        vtable = (void*)s_fileIoAutoLockMutexVTable;
        mutex = m;
        OSLockMutex(m);
    }
    // Release the guard's own mutex level without deleting stack storage.
    ~CAutoLockMutex() {
        __dt__14CAutoLockMutexFv(this, 0);
    }
};
extern FileRegistryTree s_fileRegistryTree;
void File_SetLastError(int);
void FileIO_ResolveFullPath(void*, int, void*, int);
void* memcpy(void*, const void*, unsigned);
void* memset(void*, int, unsigned);
extern void (*g_fileIoFreeCallback)(void*);
extern void* (*g_fileIoAllocate)(unsigned);
extern char s_fileIoAutoLockMutexVTable[];
// Invoke the backend's handle operation at vtable offset 0x28 (slot 10) while holding the file-I/O mutex.
int FileIO_DispatchDeviceHandleOperation10(FileHandle* file) {
    CAutoLockMutex guard(&s_fileIoMutex);
    if (file == (FileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return 0;
    }
    void* device = file->device;
    struct HandleOperation30Device {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual void v2() DECOMP_LOCAL_VIRTUAL;
        virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL;
        virtual void v5() DECOMP_LOCAL_VIRTUAL;
        virtual void v6() DECOMP_LOCAL_VIRTUAL;
        virtual void v7() DECOMP_LOCAL_VIRTUAL;
        virtual void v8() DECOMP_LOCAL_VIRTUAL;
        virtual void v9() DECOMP_LOCAL_VIRTUAL;
        virtual void* InvokeHandleOperation(void*) DECOMP_LOCAL_VIRTUAL;
    };
    void* result = ((HandleOperation30Device*)device)->InvokeHandleOperation(file->handle);
    return (int)result;
}

#pragma push
#pragma use_lmw_stmw on
// Forward a size query and its output arguments to the handle's device.
// Required live file or -1; null is not supported. sizeArg1..3 are forwarded
// unchanged, with nullability/meaning defined by the device. Returns the size
// result word, or -1/error 6 for invalid file (outputs untouched). DVD's method
// in gtl_file_io_device_dvd_size.cpp returns the byte length and optionally clears the high word.
int FileIO_GetFileSize(FileHandle* file, void* sizeArg1, void* sizeArg2, void* sizeArg3) {
    CAutoLockMutex guard(&s_fileIoMutex);
    if (file == (FileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return -1;
    }
    void* device = file->device;
    struct FileSizeDevice {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual void v2() DECOMP_LOCAL_VIRTUAL;
        virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL;
        virtual void v5() DECOMP_LOCAL_VIRTUAL;
        virtual void v6() DECOMP_LOCAL_VIRTUAL;
        virtual void* GetSize(void*, void*, void*, void*) DECOMP_LOCAL_VIRTUAL;
    };
    void* result = ((FileSizeDevice*)device)->GetSize(file->handle, sizeArg1, sizeArg2, sizeArg3);
    return (int)result;
}
#pragma pop

// Release the underlying device handle, then free the file-handle wrapper.
void* File_CloseHandle(FileHandle* file) {
    CAutoLockMutex guard(&s_fileIoMutex);
    if (file == (FileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return 0;
    }
    void* device = file->device;
    struct HandleReleaseDevice {
        virtual void a0() DECOMP_LOCAL_VIRTUAL;
        virtual void a1() DECOMP_LOCAL_VIRTUAL;
        virtual void a2() DECOMP_LOCAL_VIRTUAL;
        virtual void a3() DECOMP_LOCAL_VIRTUAL;
        virtual void a4() DECOMP_LOCAL_VIRTUAL;
        virtual void a5() DECOMP_LOCAL_VIRTUAL;
        virtual void a6() DECOMP_LOCAL_VIRTUAL;
        virtual void a7() DECOMP_LOCAL_VIRTUAL;
        virtual void a8() DECOMP_LOCAL_VIRTUAL;
        virtual void a9() DECOMP_LOCAL_VIRTUAL;
        virtual void a10() DECOMP_LOCAL_VIRTUAL;
        virtual void a11() DECOMP_LOCAL_VIRTUAL;
        virtual void a12() DECOMP_LOCAL_VIRTUAL;
        virtual void a13() DECOMP_LOCAL_VIRTUAL;
        virtual void a14() DECOMP_LOCAL_VIRTUAL;
        virtual void a15() DECOMP_LOCAL_VIRTUAL;
        virtual void a16() DECOMP_LOCAL_VIRTUAL;
        virtual void a17() DECOMP_LOCAL_VIRTUAL;
        virtual void a18() DECOMP_LOCAL_VIRTUAL;
        virtual void a19() DECOMP_LOCAL_VIRTUAL;
        virtual void a20() DECOMP_LOCAL_VIRTUAL;
        virtual void* ReleaseHandle(void*) DECOMP_LOCAL_VIRTUAL;
    };
    void* result = ((HandleReleaseDevice*)device)->ReleaseHandle(file->handle);
    if (file != 0) {
        g_fileIoFreeCallback(file);
    }
    return result;
}

#pragma push
#pragma use_lmw_stmw on
// Dispatch a seek through the synchronous or asynchronous device entry selected by the handle flags.
// Required live file or -1. offset is the signed low 32-bit BYTE displacement.
// Sync outHighResultWord is the high-result-word pointer (optional for the
// DVD/HIO devices), not a guaranteed new-position output; gtl::CFile::Seek64
// combines it with this return word. Invalid file returns -1/error 6 unchanged.
// origin domain for HIO/FMEM: 0=start, 1=current, 2=end (other values map to start).
// DVD sync ignores origin and invokes its position callee with fixed mode 2;
// that callee's interpretation is unresolved (gtl_file_io_device_dvd_seek.cpp). Returns are backend
// dependent: HIO returns fseek status, DVD returns 0/-1, not an absolute position.
// Async ignores outHighResultWord entirely and forwards origin/options unmodified.
// DVD (file_io_device_dvd_seek.cpp) ignores origin; option1=-1 chooses priority 0xA0>>6==2,
// other priority words are shifted >>6; option2=callback address, option3=user
// data. FMEM (file_io_device_fmem_seek.cpp) uses origin, ignores option1, takes option2 as callback;
// no common option enum exists across devices. HIO async is unsupported.
// Async return is submission status, not completion: file/context/callback data
// must stay live until that backend reports completion. No buffer is allocated here.
int FileIO_SeekFile(FileHandle* file, int offset, int* outHighResultWord, int origin, int asyncOption1,
                    int asyncOption2, int asyncOption3) {
    CAutoLockMutex guard(&s_fileIoMutex);
    if (file == (FileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return -1;
    }
    if (file->flags & kFileAsync) {
        void* device = file->device;
        struct AsyncSeekDevice {
            virtual void d0() DECOMP_LOCAL_VIRTUAL;
            virtual void d1() DECOMP_LOCAL_VIRTUAL;
            virtual void d2() DECOMP_LOCAL_VIRTUAL;
            virtual void d3() DECOMP_LOCAL_VIRTUAL;
            virtual void d4() DECOMP_LOCAL_VIRTUAL;
            virtual void d5() DECOMP_LOCAL_VIRTUAL;
            virtual void d6() DECOMP_LOCAL_VIRTUAL;
            virtual void d7() DECOMP_LOCAL_VIRTUAL;
            virtual void d8() DECOMP_LOCAL_VIRTUAL;
            virtual void d9() DECOMP_LOCAL_VIRTUAL;
            virtual void d10() DECOMP_LOCAL_VIRTUAL;
            virtual void d11() DECOMP_LOCAL_VIRTUAL;
            virtual void d12() DECOMP_LOCAL_VIRTUAL;
            virtual void d13() DECOMP_LOCAL_VIRTUAL;
            virtual void d14() DECOMP_LOCAL_VIRTUAL;
            virtual void d15() DECOMP_LOCAL_VIRTUAL;
            virtual void d16() DECOMP_LOCAL_VIRTUAL;
            virtual void d17() DECOMP_LOCAL_VIRTUAL;
            virtual void d18() DECOMP_LOCAL_VIRTUAL;
            virtual void d19() DECOMP_LOCAL_VIRTUAL;
            virtual void d20() DECOMP_LOCAL_VIRTUAL;
            virtual void d21() DECOMP_LOCAL_VIRTUAL;
            virtual void d22() DECOMP_LOCAL_VIRTUAL;
            virtual void d23() DECOMP_LOCAL_VIRTUAL;
            virtual void d24() DECOMP_LOCAL_VIRTUAL;
            virtual void d25() DECOMP_LOCAL_VIRTUAL;
            virtual void d26() DECOMP_LOCAL_VIRTUAL;
            virtual void d27() DECOMP_LOCAL_VIRTUAL;
            virtual int SeekAsync(int, int, int, int, int, int) DECOMP_LOCAL_VIRTUAL;
        };
        int result = ((AsyncSeekDevice*)device)
                         ->SeekAsync((int)file->handle, offset, origin, asyncOption1, asyncOption2, asyncOption3);
        return result;
    } else {
        void* device = file->device;
        struct SeekDevice {
            virtual void d0() DECOMP_LOCAL_VIRTUAL;
            virtual void d1() DECOMP_LOCAL_VIRTUAL;
            virtual void d2() DECOMP_LOCAL_VIRTUAL;
            virtual void d3() DECOMP_LOCAL_VIRTUAL;
            virtual void d4() DECOMP_LOCAL_VIRTUAL;
            virtual void d5() DECOMP_LOCAL_VIRTUAL;
            virtual int Seek(int, int, int*, int) DECOMP_LOCAL_VIRTUAL;
        };
        int result = ((SeekDevice*)device)->Seek((int)file->handle, offset, outHighResultWord, origin);
        return result;
    }
}
#pragma pop

#pragma push
#pragma opt_lifetimes off
// Run the device's pre-close hook, close the file, and free its handle wrapper.
int FileIO_CloseFile(FileHandle* file) {
    CAutoLockMutex guard1(&s_fileIoMutex);
    if (file == (FileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return -1;
    }
    {
        CAutoLockMutex guard2(&s_fileIoMutex);
        if (file == (FileHandle*)-1) {
            File_SetLastError(kInvalidFileHandleError);
        } else {
            void* device = file->device;
            struct BeforeCloseDevice {
                virtual void v0() DECOMP_LOCAL_VIRTUAL;
                virtual void v1() DECOMP_LOCAL_VIRTUAL;
                virtual void v2() DECOMP_LOCAL_VIRTUAL;
                virtual void v3() DECOMP_LOCAL_VIRTUAL;
                virtual void v4() DECOMP_LOCAL_VIRTUAL;
                virtual void v5() DECOMP_LOCAL_VIRTUAL;
                virtual void v6() DECOMP_LOCAL_VIRTUAL;
                virtual void v7() DECOMP_LOCAL_VIRTUAL;
                virtual void v8() DECOMP_LOCAL_VIRTUAL;
                virtual void BeforeClose(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((BeforeCloseDevice*)device)->BeforeClose((int)file->handle);
        }
    }
    void* device = file->device;
    struct CloseDevice {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual void v2() DECOMP_LOCAL_VIRTUAL;
        virtual int Close(int) DECOMP_LOCAL_VIRTUAL;
    };
    int result = ((CloseDevice*)device)->Close((int)file->handle);
    if (file != 0) {
        g_fileIoFreeCallback(file);
    }
    return result;
}
#pragma pop

// Poll an asynchronous transfer, copy completed buffered read data, and release owned transfer storage.
// completionState is a required writable int; outDeviceResult is optional.
// Nonzero completion copies requestedBytes (not the backend's short-read count)
// to the borrowed readCopyDestination, then clears copy-back state and frees owned storage.
// Common cleanup for reads AND writes: WriteFile installs allocatedBytes and
// transferBuffer but no readCopyDestination. A write caller must start from idle state
// with readCopyDestination/requestedBytes==0, then use this poll after submission until
// the device sets completionState. Do not free/directly reuse transfer storage
// based on the poll RETURN: only *completionState enables the release branch.
// Early invalid/nonasync returns leave both outputs untouched. A zero completion
// state retains the allocation even on error; nonzero triggers cleanup regardless
// of the returned status. The backend must write completionState reliably.
int FileIO_PollAsyncTransferResult(FileHandle* file, int* completionState, int* outDeviceResult) {
    CAutoLockMutex guard(&s_fileIoMutex);
    if (file == (FileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return 0;
    }
    if (!(file->flags & kFileAsync)) {
        return 0;
    }
    int result = 0;
    if (outDeviceResult != 0) {
        void* device = file->device;
        struct AsyncResultDevice {
            virtual void d0() DECOMP_LOCAL_VIRTUAL;
            virtual void d1() DECOMP_LOCAL_VIRTUAL;
            virtual void d2() DECOMP_LOCAL_VIRTUAL;
            virtual void d3() DECOMP_LOCAL_VIRTUAL;
            virtual void d4() DECOMP_LOCAL_VIRTUAL;
            virtual void d5() DECOMP_LOCAL_VIRTUAL;
            virtual void d6() DECOMP_LOCAL_VIRTUAL;
            virtual void d7() DECOMP_LOCAL_VIRTUAL;
            virtual void d8() DECOMP_LOCAL_VIRTUAL;
            virtual void d9() DECOMP_LOCAL_VIRTUAL;
            virtual void d10() DECOMP_LOCAL_VIRTUAL;
            virtual void d11() DECOMP_LOCAL_VIRTUAL;
            virtual void d12() DECOMP_LOCAL_VIRTUAL;
            virtual void d13() DECOMP_LOCAL_VIRTUAL;
            virtual void d14() DECOMP_LOCAL_VIRTUAL;
            virtual void d15() DECOMP_LOCAL_VIRTUAL;
            virtual void d16() DECOMP_LOCAL_VIRTUAL;
            virtual void d17() DECOMP_LOCAL_VIRTUAL;
            virtual void d18() DECOMP_LOCAL_VIRTUAL;
            virtual void d19() DECOMP_LOCAL_VIRTUAL;
            virtual void d20() DECOMP_LOCAL_VIRTUAL;
            virtual void d21() DECOMP_LOCAL_VIRTUAL;
            virtual void d22() DECOMP_LOCAL_VIRTUAL;
            virtual void d23() DECOMP_LOCAL_VIRTUAL;
            virtual void d24() DECOMP_LOCAL_VIRTUAL;
            virtual void d25() DECOMP_LOCAL_VIRTUAL;
            virtual void d26() DECOMP_LOCAL_VIRTUAL;
            virtual void d27() DECOMP_LOCAL_VIRTUAL;
            virtual void d28() DECOMP_LOCAL_VIRTUAL;
            virtual void d29() DECOMP_LOCAL_VIRTUAL;
            virtual void d30() DECOMP_LOCAL_VIRTUAL;
            virtual int GetAsyncResult(int) DECOMP_LOCAL_VIRTUAL;
        };
        result = ((AsyncResultDevice*)device)->GetAsyncResult((int)file->handle);
        *outDeviceResult = result;
    }
    {
        void* device = file->device;
        struct AsyncPollDevice {
            virtual void d0() DECOMP_LOCAL_VIRTUAL;
            virtual void d1() DECOMP_LOCAL_VIRTUAL;
            virtual void d2() DECOMP_LOCAL_VIRTUAL;
            virtual void d3() DECOMP_LOCAL_VIRTUAL;
            virtual void d4() DECOMP_LOCAL_VIRTUAL;
            virtual void d5() DECOMP_LOCAL_VIRTUAL;
            virtual void d6() DECOMP_LOCAL_VIRTUAL;
            virtual void d7() DECOMP_LOCAL_VIRTUAL;
            virtual void d8() DECOMP_LOCAL_VIRTUAL;
            virtual void d9() DECOMP_LOCAL_VIRTUAL;
            virtual void d10() DECOMP_LOCAL_VIRTUAL;
            virtual void d11() DECOMP_LOCAL_VIRTUAL;
            virtual void d12() DECOMP_LOCAL_VIRTUAL;
            virtual void d13() DECOMP_LOCAL_VIRTUAL;
            virtual void d14() DECOMP_LOCAL_VIRTUAL;
            virtual void d15() DECOMP_LOCAL_VIRTUAL;
            virtual void d16() DECOMP_LOCAL_VIRTUAL;
            virtual void d17() DECOMP_LOCAL_VIRTUAL;
            virtual void d18() DECOMP_LOCAL_VIRTUAL;
            virtual void d19() DECOMP_LOCAL_VIRTUAL;
            virtual void d20() DECOMP_LOCAL_VIRTUAL;
            virtual void d21() DECOMP_LOCAL_VIRTUAL;
            virtual void d22() DECOMP_LOCAL_VIRTUAL;
            virtual void d23() DECOMP_LOCAL_VIRTUAL;
            virtual void d24() DECOMP_LOCAL_VIRTUAL;
            virtual void d25() DECOMP_LOCAL_VIRTUAL;
            virtual void d26() DECOMP_LOCAL_VIRTUAL;
            virtual void d27() DECOMP_LOCAL_VIRTUAL;
            virtual void d28() DECOMP_LOCAL_VIRTUAL;
            virtual void d29() DECOMP_LOCAL_VIRTUAL;
            virtual int PollRead(int, int*) DECOMP_LOCAL_VIRTUAL;
        };
        result = ((AsyncPollDevice*)device)->PollRead((int)file->handle, completionState);
    }
    if (*(int*)completionState != 0) {
        void* readCopyDestination = file->readCopyDestination;
        if (readCopyDestination != 0) {
            void* transferBuffer = file->transferBuffer;
            memcpy((void*)readCopyDestination, (void*)transferBuffer, file->requestedBytes);
            file->readCopyDestination = 0;
            file->requestedBytes = 0;
        }
        if (file->allocatedBytes != 0) {
            g_fileIoFreeCallback((void*)file->transferBuffer);
            file->transferBuffer = 0;
            file->allocatedBytes = 0;
        }
    }
    return result;
}

// Find the device whose registered prefix matches the path and query its transfer alignment.
void* FileIO_GetDeviceTransferAlignment(void* path) {
    CAutoLockMutex guard(&s_fileIoMutex);
    OSLockMutex(&s_fileIoMutex);
    FileRegistryTree* registry = &s_fileRegistryTree;
    void* node = registry->first;
    void* end = (void*)&registry->root;
    // Original 0x804162B4: exhaustion unlocks and clears; a match breaks out.
    while (node != end || (OSUnlockMutex(&s_fileIoMutex), node = 0, false)) {
        unsigned int flags = ((FileRegistryNode*)node)->stringFlags;
        const char* name;
        size_t len;
        if (!(flags >> 31)) {
            len = *(unsigned char*)&((FileRegistryNode*)node)->stringFlags & kInlineStringLengthMask;
            name = (const char*)&((FileRegistryNode*)node)->stringFlags + 1;
        } else {
            len = ((FileRegistryNode*)node)->stringLength;
            name = ((FileRegistryNode*)node)->stringData;
        }
        int cmp = strncmp((const char*)path, name, len);
        if (cmp == 0) {
            node = ((FileRegistryNode*)node)->file;
            OSUnlockMutex(&s_fileIoMutex);
            break;
        }
        void* child = ((FileRegistryNode*)node)->right;
        if (child != 0) {
            while (((FileRegistryNode*)child)->left != 0)
                child = ((FileRegistryNode*)child)->left;
            node = child;
        } else {
            while (1) {
                unsigned int v = ((FileRegistryNode*)node)->parentAndColor;
                void* cand = (void*)(v & ~(unsigned)kRegistryParentColorBit);
                if (node != ((FileRegistryNode*)cand)->left) {
                    node = cand;
                    continue;
                }
                node = cand;
                break;
            }
        }
    }
    if (node != 0) {
        void* obj = node;
        typedef void* (*Fn)(void*);
        // Original 0x804162B4: after the inner unlock (0x8041635C found / 0x804163B8 not found)
        // only the guard's unlock runs (0x8041640C found / 0x804163D8 not found); no third OSUnlockMutex.
        void* r = ((Fn)(*(void***)obj)[kDeviceQueryAlignmentSlot])(obj);
        return r;
    } else {
        return 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Resolve a path and delete it through its registered device, forwarding callback and user data.
void* FileIO_DeleteFile(void* path, void* callback, void* callbackArgument) {
    void* const savedCallback = callback;
    void* const savedCallbackArgument = callbackArgument;
    void* const mutex = (void*)&s_fileIoMutex;
    CAutoLockMutex guard((OSMutex*)mutex);
    char resolvedPath[kPathCapacity];
    FileIO_ResolveFullPath(path, kPathCapacity, resolvedPath, 0);
    OSLockMutex((OSMutex*)mutex);
    FileRegistryTree* registry = &s_fileRegistryTree;
    void* node = registry->first;
    void* const sentinel = (void*)&registry->root;
    // Original 0x804164D8: retain the distinct found/exhausted unlock paths.
    while (node != sentinel || (OSUnlockMutex((OSMutex*)mutex), node = 0, false)) {
        unsigned int flags = ((FileRegistryNode*)node)->stringFlags;
        const char* name;
        size_t len;
        if (!(flags >> 31)) {
            len = *(unsigned char*)&((FileRegistryNode*)node)->stringFlags & kInlineStringLengthMask;
            name = (const char*)&((FileRegistryNode*)node)->stringFlags + 1;
        } else {
            len = ((FileRegistryNode*)node)->stringLength;
            name = ((FileRegistryNode*)node)->stringData;
        }
        int cmp = strncmp((const char*)resolvedPath, name, len);
        if (cmp == 0) {
            node = ((FileRegistryNode*)node)->file;
            OSUnlockMutex((OSMutex*)mutex);
            break;
        }
        void* child = ((FileRegistryNode*)node)->right;
        if (child != 0) {
            while (((FileRegistryNode*)child)->left != 0)
                child = ((FileRegistryNode*)child)->left;
            node = child;
        } else {
            while (1) {
                unsigned int v = ((FileRegistryNode*)node)->parentAndColor;
                void* cand = (void*)(v & ~(unsigned)kRegistryParentColorBit);
                if (node != ((FileRegistryNode*)cand)->left) {
                    node = cand;
                    continue;
                }
                node = cand;
                break;
            }
        }
    }
    if (node != 0) {
        void* obj = node;
        typedef void* (*Fn)(void*, void*, void*, void*);
        // Original 0x804164D8: after the inner unlock (0x80416590 found / 0x804165EC not found)
        // only the guard's unlock runs (0x8041664C found / 0x8041660C not found); no third OSUnlockMutex.
        void* r = ((Fn)(*(void***)obj)[kDeviceDeletePathSlot])(obj, resolvedPath, savedCallback, savedCallbackArgument);
        return r;
    } else {
        return 0;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Query disk free-space values through the path's device, forwarding callback and user data.
void* FileIO_QueryDiskFreeSpace(void* path, void* availableBytes, void* totalBytes, void* totalFreeBytes,
                                void* callback, void* callbackArgument) {
    void* const mutex = (void*)&s_fileIoMutex;
    CAutoLockMutex guard((OSMutex*)mutex);
    void* const savedPath = path;
    void* const savedAvailableBytes = availableBytes;
    void* const savedTotalBytes = totalBytes;
    void* const savedTotalFreeBytes = totalFreeBytes;
    void* const savedCallback = callback;
    void* const savedCallbackArgument = callbackArgument;
    char resolvedPath[kPathCapacity];
    FileIO_ResolveFullPath(savedPath, kPathCapacity, resolvedPath, 0);
    OSLockMutex((OSMutex*)mutex);
    FileRegistryTree* registry = &s_fileRegistryTree;
    void* node = registry->first;
    void* const sentinel = (void*)&registry->root;
    // Original unit 0x80415A88: exhaust the registry or stop at the device prefix.
    while (node != sentinel || (OSUnlockMutex((OSMutex*)mutex), node = 0, false)) {
        unsigned int flags = ((FileRegistryNode*)node)->stringFlags;
        const char* name;
        size_t len;
        if (!(flags >> 31)) {
            len = *(unsigned char*)&((FileRegistryNode*)node)->stringFlags & kInlineStringLengthMask;
            name = (const char*)&((FileRegistryNode*)node)->stringFlags + 1;
        } else {
            len = ((FileRegistryNode*)node)->stringLength;
            name = ((FileRegistryNode*)node)->stringData;
        }
        int cmp = strncmp(resolvedPath, name, len);
        if (cmp == 0) {
            node = ((FileRegistryNode*)node)->file;
            OSUnlockMutex((OSMutex*)mutex);
            break;
        }
        void* child = ((FileRegistryNode*)node)->right;
        if (child != 0) {
            while (((FileRegistryNode*)child)->left != 0)
                child = ((FileRegistryNode*)child)->left;
            node = child;
        } else {
            while (1) {
                unsigned int v = ((FileRegistryNode*)node)->parentAndColor;
                void* cand = (void*)(v & ~(unsigned)kRegistryParentColorBit);
                if (node != ((FileRegistryNode*)cand)->left) {
                    node = cand;
                    continue;
                }
                node = cand;
                break;
            }
        }
    }
    if (node != 0) {
        void* obj = node;
        typedef void* (*Fn)(void*, void*, void*, void*, void*, void*, void*);
        void* r =
            ((Fn)(*(void***)obj)[kDeviceDiskFreeSpaceSlot])(obj, resolvedPath, savedAvailableBytes, savedTotalBytes,
                                                            savedTotalFreeBytes, savedCallback, savedCallbackArgument);
        return r;
    } else {
        return 0;
    }
}
#pragma pop

// Resolve source and destination paths and move through the destination's registered device.
void* FileIO_MoveFile(void* sourcePath, void* destinationPath) {
    void* const mutex = (void*)&s_fileIoMutex;
    CAutoLockMutex guard((OSMutex*)mutex);
    void* const savedSourcePath = sourcePath;
    void* const savedDestinationPath = destinationPath;
    char resolvedSource[kPathCapacity];
    FileIO_ResolveFullPath(savedSourcePath, kPathCapacity, resolvedSource, 0);
    char resolvedDestination[kPathCapacity];
    FileIO_ResolveFullPath(savedDestinationPath, kPathCapacity, resolvedDestination, 0);
    OSLockMutex((OSMutex*)mutex);
    FileRegistryTree* registry = &s_fileRegistryTree;
    void* node = registry->first;
    void* end = (void*)&registry->root;
    // Original 0x80416668: exhaustion performs the inner unlock and clears node.
    while (node != end || (OSUnlockMutex((OSMutex*)mutex), node = 0, false)) {
        unsigned int flags = ((FileRegistryNode*)node)->stringFlags;
        const char* name;
        size_t len;
        if (!(flags >> 31)) {
            len = *(unsigned char*)&((FileRegistryNode*)node)->stringFlags & kInlineStringLengthMask;
            name = (const char*)&((FileRegistryNode*)node)->stringFlags + 1;
        } else {
            len = ((FileRegistryNode*)node)->stringLength;
            name = ((FileRegistryNode*)node)->stringData;
        }
        int cmp = strncmp(resolvedDestination, name, len);
        if (cmp == 0) {
            node = ((FileRegistryNode*)node)->file;
            OSUnlockMutex((OSMutex*)mutex);
            break;
        }
        void* child = ((FileRegistryNode*)node)->right;
        if (child != 0) {
            while (((FileRegistryNode*)child)->left != 0)
                child = ((FileRegistryNode*)child)->left;
            node = child;
        } else {
            while (1) {
                unsigned int v = ((FileRegistryNode*)node)->parentAndColor;
                void* cand = (void*)(v & ~(unsigned)kRegistryParentColorBit);
                if (node != ((FileRegistryNode*)cand)->left) {
                    node = cand;
                    continue;
                }
                node = cand;
                break;
            }
        }
    }
    if (node != 0) {
        void* obj = node;
        typedef void* (*Fn)(void*, void*, void*);
        void* r = ((Fn)(*(void***)obj)[kDeviceMovePathSlot])(obj, resolvedSource, resolvedDestination);
        return r;
    } else {
        return 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Move resolved paths through the destination device with progress data and backend-specific options.
void* FileIO_MoveFileWithCallback(void* sourcePath, void* destinationPath, int progressArgument, int backendOption1,
                                  int backendOption2) {
    volatile void* lockMutex = &s_fileIoMutex;
    volatile void* lockVtable = (void*)s_fileIoAutoLockMutexVTable;
    (void)lockMutex;
    (void)lockVtable;
    OSLockMutex(&s_fileIoMutex);
    char resolvedSource[kPathCapacity];
    char resolvedDestination[kPathCapacity];
    FileIO_ResolveFullPath(sourcePath, kPathCapacity, resolvedSource, 0);
    FileIO_ResolveFullPath(destinationPath, kPathCapacity, resolvedDestination, 0);
    OSLockMutex(&s_fileIoMutex);
    FileRegistryTree* registry = &s_fileRegistryTree;
    void* node = registry->first;
    void* end = (void*)&registry->root;
    // Original 0x80416814: keep the inner unlock on both search exits.
    while (node != end || (OSUnlockMutex(&s_fileIoMutex), node = 0, false)) {
        unsigned int flags = ((FileRegistryNode*)node)->stringFlags;
        const char* name;
        size_t len;
        if (!(flags >> 31)) {
            len = *(unsigned char*)&((FileRegistryNode*)node)->stringFlags & kInlineStringLengthMask;
            name = (const char*)&((FileRegistryNode*)node)->stringFlags + 1;
        } else {
            len = ((FileRegistryNode*)node)->stringLength;
            name = ((FileRegistryNode*)node)->stringData;
        }
        int cmp = strncmp(resolvedDestination, name, len);
        if (cmp == 0) {
            node = ((FileRegistryNode*)node)->file;
            OSUnlockMutex(&s_fileIoMutex);
            break;
        }
        void* child = ((FileRegistryNode*)node)->right;
        if (child != 0) {
            while (((FileRegistryNode*)child)->left != 0)
                child = ((FileRegistryNode*)child)->left;
            node = child;
        } else {
            while (1) {
                unsigned int v = ((FileRegistryNode*)node)->parentAndColor;
                void* cand = (void*)(v & ~(unsigned)kRegistryParentColorBit);
                if (node != ((FileRegistryNode*)cand)->left) {
                    node = cand;
                    continue;
                }
                node = cand;
                break;
            }
        }
    }
    if (node != 0) {
        void* obj = node;
        typedef void* (*Fn)(void*, void*, void*, int, int, int);
        void* r = ((Fn)(*(void***)obj)[kDeviceMoveWithProgressSlot])(obj, resolvedSource, resolvedDestination,
                                                                     progressArgument, backendOption1, backendOption2);
        OSUnlockMutex(&s_fileIoMutex);
        return r;
    } else {
        OSUnlockMutex(&s_fileIoMutex);
        return 0;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Prepare an aligned transfer buffer, dispatch a write, and release synchronous buffer storage.
// source is borrowed. Direct mode forwards it; buffered mode owns a rounded-size
// allocation but initializes only byteCount bytes before submitting transferBytes.
// The backend must supply a power-of-two alignment for the rounding expression.
// Async keeps the allocated buffer in file; sync frees it and writes outWrittenBytes
// if supplied. context/asyncOption* are opaque backend arguments, not decoded flags.
// Required idle live file (or -1), source[byteCount], byteCount>=0, successful
// allocation if buffered, and rounding that does not overflow. Only one request
// may occupy file's transfer fields; a second submission would overwrite them.
// Direct async source must meet backend alignment and stay live/unchanged until
// completion; buffered source may be reused after the synchronous memcpy here.
// Keep file and any backend-retained context/callback data live until completion.
// Buffered async storage remains file-owned even on a rejected submission; this
// function has no rejection cleanup. FileIO_PollAsyncTransferResult releases it ONLY when
// the backend reports completion, with no stale readCopyDestination installed.
// CloseFile/CloseHandle free the wrapper, not this allocation: finish/release
// the transfer before closing. outWrittenBytes is untouched on all async paths.
void* FileIO_WriteFile(FileHandle* file, void* source, int byteCount, int* outWrittenBytes, void* context,
                       void* asyncOption1, void* asyncOption2) {
    // Phase 1: acquire the lock manually and prepare borrowed/owned transfer storage.
    volatile void* sp14 = (void*)s_fileIoAutoLockMutexVTable;
    volatile void* sp10 = &s_fileIoMutex;
    (void)sp14;
    (void)sp10;
    OSLockMutex(&s_fileIoMutex);
    if (file == (FileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        OSUnlockMutex(&s_fileIoMutex);
        return 0;
    }
    unsigned int flags = file->flags;
    file->allocatedBytes = 0;
    int requestedBytes = byteCount;
    file->transferBuffer = source;
    unsigned int transferBytes = (unsigned int)requestedBytes;
    if (!(flags & kFileDirectBuffer)) {
        void* device = file->device;
        typedef unsigned int (*QueryAlignmentFunction)(void*);
        unsigned int alignment = ((QueryAlignmentFunction)(*(void***)device)[kDeviceQueryAlignmentSlot])(device);
        transferBytes = (requestedBytes + alignment - 1) & ~(alignment - 1);
        file->allocatedBytes = transferBytes;
        void* transferBuffer = g_fileIoAllocate(transferBytes);
        file->transferBuffer = transferBuffer;
        memcpy(transferBuffer, source, (size_t)requestedBytes);
    }
    // Asynchronous writes retain any allocated buffer until the device completes.
    if (file->flags & kFileAsync) {
        void* device = file->device;
        typedef void* (*AsyncWriteFunction)(void*, void*, void*, unsigned int, void*, void*, void*);
        void* deviceResult = ((AsyncWriteFunction)(*(void***)device)[kDeviceAsyncWriteSlot])(
            device, file->handle, file->transferBuffer, transferBytes, context, asyncOption1, asyncOption2);
        void* result = deviceResult;
        OSUnlockMutex(&s_fileIoMutex);
        return result;
    } else {
        int writtenBytes = 0;
        void* device = file->device;
        typedef void* (*WriteFunction)(void*, void*, void*, unsigned int, void*);
        void* deviceResult = ((WriteFunction)(*(void***)device)[kDeviceWriteSlot])(
            device, file->handle, file->transferBuffer, transferBytes, &writtenBytes);
        void* result = deviceResult;
        if (!(file->flags & kFileDirectBuffer)) {
            g_fileIoFreeCallback(file->transferBuffer);
            file->transferBuffer = 0;
            file->allocatedBytes = 0;
        }
        if (outWrittenBytes != 0) {
            *outWrittenBytes = writtenBytes;
        }
        OSUnlockMutex(&s_fileIoMutex);
        return result;
    }
}

#pragma pop

// Packed path strings; member extents include NUL terminators.
struct FileIoPathStrings {
    char dvdDevice[5], hostDevice[6], fileMemoryDevice[6];
    char physicalMemory0Device[7], physicalMemory1Device[7], physicalMemory2Device[7], physicalMemory3Device[7];
    char separator[2], parentDirectory[3], currentDirectory[2];
    char treeInsertError[26], lengthErrorType[55], stringReserveError[35];
    char stringRangeError[27], rangeErrorType[55], stringLengthError[27];
    unsigned char padding[3];
};
extern FileIoPathStrings fileIoPathStrings;
typedef char FileIoPath_separatorOffset[((int)&((FileIoPathStrings*)0)->separator == 0x2D) ? 1 : -1];

#pragma push
#pragma use_lmw_stmw on
// Split a device prefix and normalize slash-separated components, folding dot and parent segments.
// Both capacities include NUL space. Returns 1 on success, 0 on a checked size
// failure. Outputs must already contain valid empty/current strings: no prefix
// leaves devicePrefix unchanged and relative paths append to normalizedPath.
// At root, a slash-terminated '..' preserves the remaining suffix literally;
// keep that branch when changing normalization rather than imposing new rules.
int FileIO_SplitDeviceAndNormalizePath(const char* path, char* devicePrefix, int prefixCapacity, char* normalizedPath,
                                       int pathCapacity) {
    OSLockMutex(&s_fileIoMutex);
    const char* suffix = path;
    char* colon = strchr(suffix, ':');
    int devicePrefixBytes = 0;
    if (colon) {
        devicePrefixBytes = (int)(colon - suffix) + 1;
        if ((unsigned)prefixCapacity <= (unsigned)devicePrefixBytes) {
            OSUnlockMutex(&s_fileIoMutex);
            return 0;
        }
        memcpy(devicePrefix, suffix, (size_t)devicePrefixBytes);
        (devicePrefix)[devicePrefixBytes] = 0;
        suffix = colon + 1;
    }
    int rootBytes = (int)strlen(fileIoPathStrings.separator);
    if ((unsigned)pathCapacity <= (unsigned)rootBytes) {
        OSUnlockMutex(&s_fileIoMutex);
        return 0;
    }
    char component[kPathCapacity];
    const char* cursor = suffix;
    if (cursor[0] == '/') {
        strcpy(normalizedPath, fileIoPathStrings.separator);
        cursor += rootBytes;
    }
    while (strlen(cursor) != 0) {
        char* slash = strchr(cursor, '/');
        int componentBytes;
        if (slash) {
            componentBytes = (int)(slash - cursor);
            if ((unsigned)componentBytes >= (unsigned)kPathCapacity) {
                OSUnlockMutex(&s_fileIoMutex);
                return 0;
            }
            memcpy(component, cursor, (size_t)componentBytes);
            component[componentBytes] = 0;
            if (strcmp(component, fileIoPathStrings.parentDirectory) == 0) {
                if (strcmp(normalizedPath, fileIoPathStrings.separator) == 0) {
                    int remainingBytes = (int)strlen(cursor);
                    strcat(normalizedPath, cursor);
                    cursor += remainingBytes;
                    continue;
                }
            }
            cursor = slash + 1;
        } else {
            int remainingBytes = (int)strlen(cursor);
            if ((unsigned)remainingBytes >= (unsigned)kPathCapacity) {
                OSUnlockMutex(&s_fileIoMutex);
                return 0;
            }
            strcpy(component, cursor);
            cursor += remainingBytes;
        }
        // .L_80416DE4
        if (component[0] == 0)
            continue;
        if (strcmp(component, fileIoPathStrings.currentDirectory) != 0) {
            if (strcmp(component, fileIoPathStrings.parentDirectory) == 0) {
                char* lastSlash = strrchr(normalizedPath, '/');
                if (lastSlash && lastSlash != normalizedPath) {
                    *lastSlash = 0;
                } else {
                    strcpy(normalizedPath, fileIoPathStrings.separator);
                }
                continue;
            } else {
                int pathBytes = (int)strlen(normalizedPath);
                if (pathBytes != 0) {
                    char* pathEnd = normalizedPath + pathBytes;
                    if (*(pathEnd - 1) != '/') {
                        strcat(normalizedPath, fileIoPathStrings.separator);
                        pathBytes = (int)strlen(normalizedPath);
                    }
                } else {
                    strcat(normalizedPath, fileIoPathStrings.separator);
                    pathBytes = (int)strlen(normalizedPath);
                }
                int componentBytes = (int)strlen(component);
                if ((unsigned)pathCapacity <= (unsigned)(pathBytes + componentBytes)) {
                    OSUnlockMutex(&s_fileIoMutex);
                    return 0;
                }
                strcat(normalizedPath, component);
            }
        }
    }
    OSUnlockMutex(&s_fileIoMutex);
    return 1;
}
#pragma pop

// Resolve a registered device and pass it the normalized path when it is nonempty.
void* FileIO_SetCurrentDirectory(const char* path) {
    // Phase 1: guard holds lock level 1 while normalization updates the shared
    // default-device prefix and the local normalized suffix.
    CAutoLockMutex guard(&s_fileIoMutex);
    char normalizedPath[kPathCapacity];
    memset(normalizedPath, 0, kPathCapacity);
    int normalized = FileIO_SplitDeviceAndNormalizePath(path, ((FileIoGlobalStorage*)&s_fileIoMutex)->defaultDevicePath,
                                                        kPathCapacity, normalizedPath, kPathCapacity);
    if (normalized == 0) {
        return 0;
    }
    // Phase 2: acquire registry lock level 2. Each search exit releases it:
    // match -> body unlock then break; exhaustion -> condition unlock then false.
    // Every function return then destroys guard, releasing the remaining level 1.
    OSLockMutex(&s_fileIoMutex);
    FileRegistryTree* registry = &s_fileRegistryTree;
    void* node = registry->first;
    void* end = (void*)&registry->root;
    void* found = 0;
    // The right operand runs ONLY at the sentinel; it is the exhaustion exit,
    // not a retry condition. Keep its unlock separate from the guard's unlock.
    while (node != end || (OSUnlockMutex(&s_fileIoMutex), found = 0, false)) {
        unsigned int flags = ((FileRegistryNode*)node)->stringFlags;
        size_t len;
        const char* name;
        if (!(flags >> 31)) {
            len = *(unsigned char*)&((FileRegistryNode*)node)->stringFlags & kInlineStringLengthMask;
            name = (const char*)&((FileRegistryNode*)node)->stringFlags + 1;
        } else {
            len = ((FileRegistryNode*)node)->stringLength;
            name = ((FileRegistryNode*)node)->stringData;
        }
        // Original 0x80416FA8: addi r3, r30, 0x30 with r30 = s_fileIoMutex (set at 0x80416F04-0x80416F08),
        // i.e. compare the device prefix FileIO_SplitDeviceAndNormalizePath wrote, not the local path buffer.
        int cmp = strncmp(((FileIoGlobalStorage*)&s_fileIoMutex)->defaultDevicePath, name, len);
        if (cmp == 0) {
            found = ((FileRegistryNode*)node)->file;
            OSUnlockMutex(&s_fileIoMutex);
            break;
        }
        void* child = ((FileRegistryNode*)node)->right;
        if (child != 0) {
            while (((FileRegistryNode*)child)->left != 0)
                child = ((FileRegistryNode*)child)->left;
            node = child;
        } else {
            while (1) {
                unsigned int v = ((FileRegistryNode*)node)->parentAndColor;
                void* cand = (void*)(v & ~(unsigned)kRegistryParentColorBit);
                if (node != ((FileRegistryNode*)cand)->left) {
                    node = cand;
                    continue;
                }
                node = cand;
                break;
            }
        }
    }
    if (found == 0) {
        return 0;
    }
    // Phase 3: registry search is unlocked; the outer guard still protects the
    // selected borrowed device and shared path during the backend call.
    size_t pathBytes = strlen(normalizedPath);
    if (pathBytes != 0) {
        void** vt = *(void***)found;
        typedef void (*Fn)(void*, const char*, size_t);
        ((Fn)vt[kDeviceSetPathSlot])(found, normalizedPath, pathBytes);
    }
    return (void*)1;
}

#pragma push
#pragma use_lmw_stmw on
// Prepare an aligned read, retain asynchronous copy-back state, and clean up synchronous buffers.
// destination is borrowed writable storage for byteCount bytes. Buffered reads
// allocate a rounded transfer size and retain destination/requestedBytes in file.
// Direct reads use caller storage; alignment/lifetime are the caller/backend contract.
// Sync status 1 copies exactly byteCount, even if readBytes reports fewer bytes;
// outReadBytes is optional. Async returns backend status without setting that output.
void* FileIO_ReadFile(FileHandle* file, void* destination, int byteCount, int* outReadBytes, void* context,
                      void* asyncOption1, void* asyncOption2, void* asyncOption3) {
    // Phase 1: initialize transfer ownership. transferSizeWord is an integer byte
    // count in the legacy pointer representation; it never holds the result now.
    CAutoLockMutex guard(&s_fileIoMutex);
    if (file == (FileHandle*)-1) {
        File_SetLastError(kInvalidFileHandleError);
        return 0;
    }
    unsigned int flags = file->flags;
    file->readCopyDestination = 0;
    void* transferSizeWord = (void*)(size_t)byteCount;
    file->requestedBytes = 0;
    file->allocatedBytes = 0;
    file->transferBuffer = destination;
    if (!(flags & kFileDirectBuffer)) {
        file->readCopyDestination = destination;
        file->requestedBytes = (unsigned int)byteCount;
        void* device = file->device;
        struct TransferAlignmentDevice {
            virtual void v0() DECOMP_LOCAL_VIRTUAL;
            virtual void v1() DECOMP_LOCAL_VIRTUAL;
            virtual void v2() DECOMP_LOCAL_VIRTUAL;
            virtual void v3() DECOMP_LOCAL_VIRTUAL;
            virtual void v4() DECOMP_LOCAL_VIRTUAL;
            virtual void v5() DECOMP_LOCAL_VIRTUAL;
            virtual void v6() DECOMP_LOCAL_VIRTUAL;
            virtual void v7() DECOMP_LOCAL_VIRTUAL;
            virtual void v8() DECOMP_LOCAL_VIRTUAL;
            virtual void v9() DECOMP_LOCAL_VIRTUAL;
            virtual void v10() DECOMP_LOCAL_VIRTUAL;
            virtual void v11() DECOMP_LOCAL_VIRTUAL;
            virtual void v12() DECOMP_LOCAL_VIRTUAL;
            virtual void v13() DECOMP_LOCAL_VIRTUAL;
            virtual void v14() DECOMP_LOCAL_VIRTUAL;
            virtual void v15() DECOMP_LOCAL_VIRTUAL;
            virtual void v16() DECOMP_LOCAL_VIRTUAL;
            virtual void v17() DECOMP_LOCAL_VIRTUAL;
            virtual void v18() DECOMP_LOCAL_VIRTUAL;
            virtual void v19() DECOMP_LOCAL_VIRTUAL;
            virtual void v20() DECOMP_LOCAL_VIRTUAL;
            virtual void v21() DECOMP_LOCAL_VIRTUAL;
            virtual void v22() DECOMP_LOCAL_VIRTUAL;
            virtual void v23() DECOMP_LOCAL_VIRTUAL;
            virtual unsigned int GetTransferAlignment() DECOMP_LOCAL_VIRTUAL;
        };
        unsigned int alignment = ((TransferAlignmentDevice*)device)->GetTransferAlignment();
        unsigned int transferBytes = (byteCount + alignment - 1) & ~(alignment - 1);
        transferSizeWord = (void*)(size_t)transferBytes;
        file->allocatedBytes = transferBytes;
        void* transferBuffer = g_fileIoAllocate(transferBytes);
        file->transferBuffer = transferBuffer;
    }
    // Phase 2: dispatch the aligned size; backend result and byte count are distinct.
    unsigned int transferBytes = (unsigned int)(size_t)transferSizeWord;
    if (file->flags & kFileAsync) {
        // This existing rejection occurs after allocation. It leaves file's
        // owned buffer/copy-back fields installed; this function does not free it.
        if (asyncOption2 != 0 && !(file->flags & kFileDirectBuffer)) {
            return 0;
        }
        void* device = file->device;
        struct AsyncReadDevice {
            virtual void v0() DECOMP_LOCAL_VIRTUAL;
            virtual void v1() DECOMP_LOCAL_VIRTUAL;
            virtual void v2() DECOMP_LOCAL_VIRTUAL;
            virtual void v3() DECOMP_LOCAL_VIRTUAL;
            virtual void v4() DECOMP_LOCAL_VIRTUAL;
            virtual void v5() DECOMP_LOCAL_VIRTUAL;
            virtual void v6() DECOMP_LOCAL_VIRTUAL;
            virtual void v7() DECOMP_LOCAL_VIRTUAL;
            virtual void v8() DECOMP_LOCAL_VIRTUAL;
            virtual void v9() DECOMP_LOCAL_VIRTUAL;
            virtual void v10() DECOMP_LOCAL_VIRTUAL;
            virtual void v11() DECOMP_LOCAL_VIRTUAL;
            virtual void v12() DECOMP_LOCAL_VIRTUAL;
            virtual void v13() DECOMP_LOCAL_VIRTUAL;
            virtual void v14() DECOMP_LOCAL_VIRTUAL;
            virtual void v15() DECOMP_LOCAL_VIRTUAL;
            virtual void v16() DECOMP_LOCAL_VIRTUAL;
            virtual void v17() DECOMP_LOCAL_VIRTUAL;
            virtual void v18() DECOMP_LOCAL_VIRTUAL;
            virtual void v19() DECOMP_LOCAL_VIRTUAL;
            virtual void v20() DECOMP_LOCAL_VIRTUAL;
            virtual void v21() DECOMP_LOCAL_VIRTUAL;
            virtual void v22() DECOMP_LOCAL_VIRTUAL;
            virtual void v23() DECOMP_LOCAL_VIRTUAL;
            virtual void v24() DECOMP_LOCAL_VIRTUAL;
            virtual void v25() DECOMP_LOCAL_VIRTUAL;
            virtual void* ReadAsync(void*, void*, unsigned int, void*, void*, void*, void*) DECOMP_LOCAL_VIRTUAL;
        };
        void* deviceResult = ((AsyncReadDevice*)device)
                                 ->ReadAsync(file->handle, file->transferBuffer, transferBytes, context, asyncOption1,
                                             asyncOption2, asyncOption3);
        return deviceResult;
    } else {
        int readBytes = 0;
        void* device = file->device;
        struct ReadDevice {
            virtual void v0() DECOMP_LOCAL_VIRTUAL;
            virtual void v1() DECOMP_LOCAL_VIRTUAL;
            virtual void v2() DECOMP_LOCAL_VIRTUAL;
            virtual void v3() DECOMP_LOCAL_VIRTUAL;
            virtual void* Read(void*, void*, unsigned int, int*, void*) DECOMP_LOCAL_VIRTUAL;
        };
        void* deviceResult =
            ((ReadDevice*)device)->Read(file->handle, file->transferBuffer, transferBytes, &readBytes, context);
        // Phase 3 (sync only): copy successful data, release temporary storage
        // on either status, clear ownership fields and publish the count.
        // Async ownership instead remains in file for completion handling.
        if (!(file->flags & kFileDirectBuffer)) {
            if ((int)(size_t)deviceResult == 1) {
                memcpy(destination, file->transferBuffer, (size_t)byteCount);
            }
            g_fileIoFreeCallback(file->transferBuffer);
            file->transferBuffer = 0;
            file->allocatedBytes = 0;
            file->readCopyDestination = 0;
            file->requestedBytes = 0;
        }
        if (outReadBytes != 0) {
            *outReadBytes = readBytes;
        }
        return deviceResult;
    }
}
#pragma pop

} // extern "C"
