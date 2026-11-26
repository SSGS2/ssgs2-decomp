// HIO and FMEM file I/O devices: a run of stub virtual slots on CFileIoDeviceHIO, the NAND
// completion callback dispatch, and the FMEM Init and Destroy methods that own its path strings.

#include "types.h"
#include "sdk_protos.h"
#include "classes/GtlFileIoDeviceFMEM.h"
#include "classes/GtlFileIoDeviceHIO.h"

extern "C" {
void* NANDGetUserData(void*);
void FileIO_Free(void*);
void FileIOString_ReplaceRangeWithAllocator(void*, u32, u32, const char*, const char*, void*);
void NANDGetCurrentDir(void*);
void NANDInit(void);
void* memset(void*, int, size_t);
extern char gtlFileIoDeviceVtable[];
extern char __vt__Q23gtl17CFileIoDeviceFMEM[];
extern char pathSeparatorString[2];

struct Obj80419194 {
    virtual void v0();
    virtual void v1();
    virtual void m(void* nandResult, void* operationWord1, void* operationWord2);
};

// +0x04 and +0x10 each begin a 12-byte string representation. The flag word's
// top bit selects owned heap text (the corresponding StringHeapLength and
// StringHeapData fields); otherwise
// byte 0 low seven bits hold length and inline text starts at byte 1. The first
// string starts as the path separator; the second caches the NAND current directory.
enum {
    kInlineStringLengthMask = 0x7f,
    kNandDirectoryBufferBytes = 0x40
};

// Forward a NAND completion result and its stored operation words to callback slot 2.
// commandBlock must retain non-null user data {callback, word1, word2} until dispatch.
// nandResult is the SDK status carried in the first argument register, not a device.
// No ownership is released here; the invoked callback handles operation cleanup.

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma use_lmw_stmw on
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

} // extern "C"

// Function definitions follow the original address order.

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_11() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::DeleteFile() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_13() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_14() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_15() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_16() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::MoveFileToDirectoryAsync() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_18() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_19() {
    return -1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_20() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_21() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_22() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_23() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_24() {
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_25() {
    return -1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_26() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_27() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_28() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_29() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_30() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
int gtl::CFileIoDeviceHIO::vfunc_31() {
    return 0;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
void gtl::CFileIoDeviceFMEM::DispatchNandCallback(void* nandResult, void* commandBlock) {
    struct T {
        void* completionCallback;
        void* operationWord1;
        void* operationWord2;
    };
    T* callbackContext = (T*)NANDGetUserData(commandBlock);
    ((Obj80419194*)callbackContext->completionCallback)
        ->m(nandResult, callbackContext->operationWord1, callbackContext->operationWord2);
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma use_lmw_stmw on
// Initialize path strings, initialize NAND, and cache its current directory; return this.
// The caller provides writable storage. NAND return codes are ignored here.
// Volatile accesses and allocator-token byte copies preserve the matched construction.
void* gtl::CFileIoDeviceFMEM::Init() {
    volatile gtl::CFileIoDeviceFMEM* device;
    int zeroOrHeaderByte;
    u32 currentLength;
    // Bytes 0..15 reproduce allocator tokens; 16..79 hold the directory C string.
    volatile char scratch[0x50];
    size_t separatorLength;
    size_t directoryLength;
    device = this;
    // Construct the base path string and replace its empty contents with the separator.
    zeroOrHeaderByte = 0;
    device->pathString.flags = zeroOrHeaderByte;
    device->pathString.length = zeroOrHeaderByte;
    device->pathString.data = (char*)zeroOrHeaderByte;
    zeroOrHeaderByte = ((volatile unsigned char*)device)[4];
    device->vtbl = (void*)gtlFileIoDeviceVtable;
    scratch[12] = scratch[8];
    currentLength = zeroOrHeaderByte & kInlineStringLengthMask;
    separatorLength = strlen(pathSeparatorString);
    FileIOString_ReplaceRangeWithAllocator((void*)&device->pathString, 0, currentLength, pathSeparatorString,
                                           pathSeparatorString + separatorLength, (void*)&scratch[12]);

    // Establish the derived vtable and an empty directory string before the SDK query.
    device->cachedCurrentDirectory.flags = 0;
    device->vtbl = (void*)__vt__Q23gtl17CFileIoDeviceFMEM;
    device->cachedCurrentDirectory.length = 0;
    device->cachedCurrentDirectory.data = (char*)0;

    NANDInit();

    memset((void*)&scratch[16], 0, kNandDirectoryBufferBytes);
    NANDGetCurrentDir((void*)&scratch[16]);

    // Replace the directory string's current contents using its active representation.
    u32 directoryStringFlags = device->cachedCurrentDirectory.flags;
    if (!(directoryStringFlags >> 31)) {
        unsigned char inlineHeader = ((volatile unsigned char*)device)[0x10];
        currentLength = inlineHeader & kInlineStringLengthMask;
    } else {
        currentLength = device->cachedCurrentDirectory.length;
    }
    scratch[0] = scratch[4];

    directoryLength = strlen((const char*)&scratch[16]);
    FileIOString_ReplaceRangeWithAllocator((void*)&device->cachedCurrentDirectory, 0, currentLength,
                                           (const char*)&scratch[16], (const char*)&scratch[16] + directoryLength,
                                           (void*)&scratch[0]);

    return (void*)device;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
// Release the two owned string buffers, then free the device when flags > 0.
// Inline text is part of this. The returned pointer may already have been freed;
// with flags <= 0 the released string representations are not reset for reuse.
void* gtl::CFileIoDeviceFMEM::Destroy(int flags) {
    void* self = this;
    if (self) {
        // Destroy the cached directory string before the base device's path string.
        if ((unsigned int)self + 0x10) {
            if (cachedCurrentDirectory.flags >> 31) {
                FileIO_Free(cachedCurrentDirectory.data);
            }
        }
        if (self) {
            if ((unsigned int)self + 0x4) {
                if (pathString.flags >> 31) {
                    FileIO_Free(pathString.data);
                }
            }
        }
        if (flags > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
// Report the FMEM device as available.
int gtl::CFileIoDeviceFMEM::IsAvailable() {
    return 1;
}
#pragma pop
