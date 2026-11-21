// Translation unit 0x804190EC-0x80419394 (25 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

extern "C" {
void *NANDGetUserData(void *);
void FileIO_Free(void *);
void FileIOString_ReplaceRangeWithAllocator(void *, u32, u32, const char *, const char *, void *);
void NANDGetCurrentDir(void *);
void NANDInit(void);
void *memset(void *, int, size_t);
extern char gtlFileIoDeviceVtable[];
extern char __vt__Q23gtl17CFileIoDeviceFMEM[];
extern char pathSeparatorString[2];
// HIO slots below are fixed-result stubs: self is ignored. The operation names
// and the wider meaning of each result are unresolved; preserve the slot numbers.
// Return the fixed zero result for HIO slot 11.
int gtl_CFileIoDeviceHIO_vfunc_11(void *self);
// Return the fixed zero result for HIO slot 12.
int gtl_CFileIoDeviceHIO_DeleteFile(void *self);
// Return the fixed zero result for HIO slot 13.
int gtl_CFileIoDeviceHIO_vfunc_13(void *self);
// Return the fixed zero result for HIO slot 14.
int gtl_CFileIoDeviceHIO_vfunc_14(void *self);
// Return the fixed zero result for HIO slot 15.
int gtl_CFileIoDeviceHIO_vfunc_15(void *self);
// Return the fixed zero result for HIO slot 16.
int gtl_CFileIoDeviceHIO_vfunc_16(void *self);
// Return the fixed zero result for HIO slot 17.
int gtl_CFileIoDeviceHIO_MoveFileToDirectoryAsync(void *self);
// Return the fixed zero result for HIO slot 18.
int gtl_CFileIoDeviceHIO_vfunc_18(void *self);
// Return the fixed -1 result for HIO slot 19.
int gtl_CFileIoDeviceHIO_vfunc_19(void *self);
// Return the fixed zero result for HIO slot 20.
int gtl_CFileIoDeviceHIO_vfunc_20(void *self);
// Return the fixed zero result for HIO slot 21.
int gtl_CFileIoDeviceHIO_vfunc_21(void *self);
// Return the fixed zero result for HIO slot 22.
int gtl_CFileIoDeviceHIO_vfunc_22(void *self);
// Return the fixed zero result for HIO slot 23.
int gtl_CFileIoDeviceHIO_vfunc_23(void *self);
// Return the fixed 1 result for HIO slot 24.
int gtl_CFileIoDeviceHIO_vfunc_24(void *self);
// Return the fixed -1 result for HIO slot 25.
int gtl_CFileIoDeviceHIO_vfunc_25(void *self);
// Return the fixed zero result for HIO slot 26.
int gtl_CFileIoDeviceHIO_vfunc_26(void *self);
// Return the fixed zero result for HIO slot 27.
int gtl_CFileIoDeviceHIO_vfunc_27(void *self);
// Return the fixed zero result for HIO slot 28.
int gtl_CFileIoDeviceHIO_vfunc_28(void *self);
// Return the fixed zero result for HIO slot 29.
int gtl_CFileIoDeviceHIO_vfunc_29(void *self);
// Return the fixed zero result for HIO slot 30.
int gtl_CFileIoDeviceHIO_vfunc_30(void *self);
// Return the fixed zero result for HIO slot 31.
int gtl_CFileIoDeviceHIO_vfunc_31(void *self);
// Report the FMEM device's fixed available result; self is ignored.
int gtl_CFileIoDeviceFMEM_IsAvailable(void *self);

struct Obj80419194 {
    virtual void v0();
    virtual void v1();
    virtual void m(void *nandResult, void *operationWord1, void *operationWord2);
};

struct gtl_CFileIoDeviceFMEM {
    void *vtable;       /* 0x00 */
    u32 pathStringStorageWord;      /* 0x04 */
    u32 pathStringHeapLength;      /* 0x08 */
    void *pathStringHeapData;     /* 0x0C */
    u32 directoryStringStorageWord;      /* 0x10 */
    u32 directoryStringHeapLength;      /* 0x14 */
    void *directoryStringHeapData;     /* 0x18 */
};
// +0x04 and +0x10 each begin a 12-byte string representation. The flag word's
// top bit selects owned heap text (the corresponding StringHeapLength and
// StringHeapData fields); otherwise
// byte 0 low seven bits hold length and inline text starts at byte 1. The first
// string starts as the path separator; the second caches the NAND current directory.
enum { kInlineStringLengthMask = 0x7f, kNandDirectoryBufferBytes = 0x40 };

// Forward a NAND completion result and its stored operation words to callback slot 2.
// commandBlock must retain non-null user data {callback, word1, word2} until dispatch.
// nandResult is the SDK status carried in the first argument register, not a device.
// No ownership is released here; the invoked callback handles operation cleanup.
void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void *nandResult, void *commandBlock);

// Release the two owned string buffers, then free the device when flags > 0.
// Inline text is part of self. The returned pointer may already have been freed;
// with flags <= 0 the released string representations are not reset for reuse.
void *gtl_CFileIoDeviceFMEM_dtor(void *self, int flags);

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma use_lmw_stmw on
// Initialize path strings, initialize NAND, and cache its current directory; return self.
// self is caller-provided writable storage. NAND return codes are ignored here.
// Volatile accesses and allocator-token byte copies preserve the matched construction.
void *gtl_CFileIoDeviceFMEM_Ctor(volatile gtl_CFileIoDeviceFMEM *self);
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
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_11(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_DeleteFile(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_13(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_14(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_15(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_16(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_MoveFileToDirectoryAsync(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_18(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_19(void *self) { return -1; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_20(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_21(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_22(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_23(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_24(void *self) { return 1; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_25(void *self) { return -1; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_26(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_27(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_28(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_29(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_30(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceHIO_vfunc_31(void *self) { return 0; }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
void gtl_CFileIoDeviceFMEM_DispatchNandCallback(void *nandResult, void *commandBlock) {
    struct T { void *completionCallback; void *operationWord1; void *operationWord2; };
    T *callbackContext = (T *)NANDGetUserData(commandBlock);
    ((Obj80419194 *)callbackContext->completionCallback)->m(nandResult, callbackContext->operationWord1, callbackContext->operationWord2);
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma use_lmw_stmw on
extern "C" {
void *gtl_CFileIoDeviceFMEM_Ctor(volatile gtl_CFileIoDeviceFMEM *self) {
    volatile gtl_CFileIoDeviceFMEM *device;
    int zeroOrHeaderByte;
    u32 currentLength;
    // Bytes 0..15 reproduce allocator tokens; 16..79 hold the directory C string.
    volatile char scratch[0x50];
    size_t separatorLength;
    size_t directoryLength;
    device = self;
    // Construct the base path string and replace its empty contents with the separator.
    zeroOrHeaderByte = 0;
    device->pathStringStorageWord = zeroOrHeaderByte;
    device->pathStringHeapLength = zeroOrHeaderByte;
    device->pathStringHeapData = (void *)zeroOrHeaderByte;
    zeroOrHeaderByte = ((volatile unsigned char *)device)[4];
    device->vtable = (void *)gtlFileIoDeviceVtable;
    scratch[12] = scratch[8];
    currentLength = zeroOrHeaderByte & kInlineStringLengthMask;
    separatorLength = strlen(pathSeparatorString);
    FileIOString_ReplaceRangeWithAllocator((void *)&device->pathStringStorageWord, 0, currentLength, pathSeparatorString,
                  pathSeparatorString + separatorLength, (void *)&scratch[12]);

    // Establish the derived vtable and an empty directory string before the SDK query.
    device->directoryStringStorageWord = 0;
    device->vtable = (void *)__vt__Q23gtl17CFileIoDeviceFMEM;
    device->directoryStringHeapLength = 0;
    device->directoryStringHeapData = (void *)0;

    NANDInit();

    memset((void *)&scratch[16], 0, kNandDirectoryBufferBytes);
    NANDGetCurrentDir((void *)&scratch[16]);

    // Replace the directory string's current contents using its active representation.
    u32 directoryStringFlags = device->directoryStringStorageWord;
    if (!(directoryStringFlags >> 31)) {
        unsigned char inlineHeader = ((volatile unsigned char *)device)[0x10];
        currentLength = inlineHeader & kInlineStringLengthMask;
    } else {
        currentLength = device->directoryStringHeapLength;
    }
    scratch[0] = scratch[4];

    directoryLength = strlen((const char *)&scratch[16]);
    FileIOString_ReplaceRangeWithAllocator((void *)&device->directoryStringStorageWord, 0, currentLength, (const char *)&scratch[16], (const char *)&scratch[16] + directoryLength, (void *)&scratch[0]);

    return (void *)device;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
void *gtl_CFileIoDeviceFMEM_dtor(void *self, int flags) {
    if (self) {
        // Destroy the cached directory string before the base device's path string.
        if ((unsigned int)self + 0x10) {
            if (((gtl_CFileIoDeviceFMEM*)self)->directoryStringStorageWord >> 31) {
                FileIO_Free(((gtl_CFileIoDeviceFMEM*)self)->directoryStringHeapData);
            }
        }
        if (self) {
            if ((unsigned int)self + 0x4) {
                if (((gtl_CFileIoDeviceFMEM*)self)->pathStringStorageWord >> 31) {
                    FileIO_Free(((gtl_CFileIoDeviceFMEM*)self)->pathStringHeapData);
                }
            }
        }
        if (flags > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
extern "C" {
int gtl_CFileIoDeviceFMEM_IsAvailable(void *self) { return 1; }
}
#pragma pop
