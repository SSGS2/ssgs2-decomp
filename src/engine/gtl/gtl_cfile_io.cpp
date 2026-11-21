// Translation unit 0x8043EFB4-0x8043F960 (12 functions; boundaries chosen from call/vtable evidence over decomp-toolkit's units).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
#include <decomp/cw_string_guard.h>

struct StringAllocatorTag {
    u8 v;
};

struct gtl_CFileBase {
    s32 handle;     /* 0x00 */
};

/* the vptr sits at 0x04 after the base; virtual slot n is at 0x04 + 4 * n */
struct gtl_CFile : gtl_CFileBase {
    virtual void v0();
    virtual void v1(const char *, u32);
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    s32 opened;     /* 0x08 */
    u32 pathFlags;       /* 0x0C: SSO flags/capacity, or first byte with heap bit and inline length */
    u32 pathLength;      /* 0x10: heap string length (inline characters overlap this word) */
    u32 pathData;      /* 0x14: heap pointer bits (inline characters overlap this word) */
};

enum FileOpenOptions {
    kAccessMask = 3, kReadOnly = 0, kWriteOnly = 1, kReadWrite = 2,
    kShareMask = 0x70, kDenyReadWrite = 0x10, kDenyWrite = 0x20,
    kDenyRead = 0x30, kAllowReadWrite = 0x40,
    kCreate = 0x1000, kKeepExisting = 0x2000, kIgnoredOpenOption = 0x8000,
    kNoBuffering = 0x10000, kWriteThrough = 0x20000,
    kRandomAccess = 0x40000, kSequentialAccess = 0x80000
};
enum NativeFileOptions {
    kNativeShareNone = 0, kNativeShareRead = 1, kNativeShareWrite = 2,
    kNativeShareReadWrite = 3, kCreateAlways = 2, kOpenExisting = 3, kOpenAlways = 4,
    kNormalAttributes = 0x80, kNativeNoBuffering = 0x20000000,
    kNativeRandomAccess = 0x10000000, kNativeSequentialAccess = 0x08000000
};
static const u32 kNativeReadAccess = 0x80000000;
static const u32 kNativeWriteAccess = 0x40000000;
static const u32 kNativeReadWriteAccess = 0xC0000000;
static const u32 kNativeWriteThrough = 0x80000000;
enum StringLimits { kInlineStringCapacity = 11, kStringAllocationAlignment = 16,
                    kMaximumStringLength = 0x7ffffffe };
struct StringExceptionStrings {
    char rangeError[0x1b];
    char rangeErrorType[0x37];
    char lengthError[0x1b];
    char lengthErrorType[0x37];
};

extern "C" {

struct FilePathString {
    union {
        u32 flag;
        struct {
            CW_BITS2(
                u32 heapbit : 1,
                u32 capacity : 31)
        };
        struct {
            CW_BITS2(
                u8 byteheap : 1,
                u8 smallsize : 7)
        };
    };
    u32 length;
    char *ptr;
};

struct SharedMessageControl {
    void *vt;
    u32 c1;
    u32 c2;
    char *s;
};

extern char __vt__Q23gtl5CFile[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12out_of_range[];
extern char __vt__Q23std12length_error[];
extern char s_sharedCharDeleterVtable[];
extern char stringRangeAndLengthExceptionStrings_8043EFB4[];
extern u8 s_fileIoStringAllocatedTerminator;
extern u8 s_fileIoStringInPlaceTerminator;
void __dt__Q23std12out_of_rangeFv(void);
void __dt__Q23std12length_errorFv(void);
void *__nwa__FUl(u32 size);
void *FileIO_Alloc(u32 size);
void FileIO_Free(void *p);
void __dla__FPv(void *p);
void __dl__FPv(void *p);
void *__nw__FUl(u32 size);
void *memcpy(void *dst, const void *src, size_t n);

// Replace a bounded string range, growing aligned storage and protecting overlapping insertions.
FilePathString *FileIO_String_ReplaceRange(FilePathString *self, u32 position, u32 removeCount, const char *first, const char *last, StringAllocatorTag *extra);
s32 FileIO_OpenFile(const char *path, u32 r4, u32 r5, u32 r6, u32 r7, u32 r8, u32 r9, u32 r10, u32 stack8, u32 stackC);
s32 FileIO_ReadFile(s32 handle, void *buf, u32 length, s32 *out, s32 a, s32 b, u32 c, u32 d);
s32 FileIO_CloseFile(s32 handle);
s32 FileIO_WriteFile(s32 handle, s32 a, s32 b, void *out, s32 c, s32 d, s32 e);
s32 FileIO_SeekFile(s32 handle, s32 a, s32 *out, s32 b, s32 c, s32 d, s32 e);
s32 FileIO_GetFileSize(s32 handle, s32 *out, s32 a, s32 b);
s32 FileIO_DispatchDeviceHandleOperation10(s32 handle);
void *File_GetLastError(void);
void FileError_GetUnknownMessage(void);

struct Holder_u8043EFB4 {
    char *p;
    char *flag;
    Holder_u8043EFB4(char *b, char *f) {
        p = b;
        flag = f;
    }
    ~Holder_u8043EFB4() {
        if (p) {
            __dla__FPv(p);
        }
    }
};

struct LogicErrorMessage {
    void *vt;
    char *buf;
    SharedMessageControl *ref;
    LogicErrorMessage(const char *msg) {
        vt = __vt__Q23std11logic_error;
        char done __attribute__((aligned(4))) = 0;
        const char *m = msg;
        u32 n = strlen(m) + 1;
        char *b = (char *)__nwa__FUl(n * 2 - n);
        buf = b;
        {
            Holder_u8043EFB4 h(b, &done);
            SharedMessageControl *r = (SharedMessageControl *)__nw__FUl(16);
            if (r) {
                r->c1 = 1;
                r->c2 = 1;
                r->vt = s_sharedCharDeleterVtable;
                r->s = b;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, m);
    }
};

struct StringRangeError : LogicErrorMessage {
    StringRangeError(const char *msg) : LogicErrorMessage(msg) {
        vt = __vt__Q23std12out_of_range;
        __throw(((StringExceptionStrings*)stringRangeAndLengthExceptionStrings_8043EFB4)->rangeErrorType, this, (void *)__dt__Q23std12out_of_rangeFv);
    }
};

struct StringLengthError : LogicErrorMessage {
    StringLengthError(const char *msg) : LogicErrorMessage(msg) {
        vt = __vt__Q23std12length_error;
        __throw(((StringExceptionStrings*)stringRangeAndLengthExceptionStrings_8043EFB4)->lengthErrorType, this, (void *)__dt__Q23std12length_errorFv);
    }
};

// Initialize an unopened file and its empty small-string path.
void gtl_CFile_Ctor(gtl_CFile *self) {
    ((void **)self)[1] = __vt__Q23gtl5CFile;
    self->pathFlags = 0;
    self->pathLength = 0;
    self->pathData = 0;
    self->handle = -1;
    self->opened = 0;
}

#pragma opt_lifetimes off
} // extern C
struct CFilePath {
    char storage[12];
    ~CFilePath();
};
struct CFileCtor {
    char prefix[12];
    CFilePath name;
    CFileCtor(const char*, u32);
};
// Initialize path storage and open the supplied file using the requested flags.
CFileCtor::CFileCtor(const char *path, u32 flags) {
    gtl_CFile* self = (gtl_CFile*)this;
    ((void **)self)[1] = __vt__Q23gtl5CFile;
    self->pathFlags = 0;
    self->pathLength = 0;
    self->pathData = 0;
    self->handle = -1;
    self->v1(path, flags);
}
extern "C" {
#pragma opt_lifetimes reset

// Store the path, translate access/share/create/cache options, and open the underlying handle.
s32 gtl_CFile_Open(gtl_CFile *self, const char *path, u32 flags) {
    StringAllocatorTag clearAllocator;
    volatile StringAllocatorTag clearAllocatorSource;
    volatile StringAllocatorTag copyAllocatorSource;
    StringAllocatorTag copyAllocator;
    u32 access;
    s32 share;
    u32 creation;
    u32 attributes;
    s32 oldPathLength;
    s32 handle;

    flags &= ~kIgnoredOpenOption;
    self->opened = 0;
    self->handle = -1;
    clearAllocator.v = clearAllocatorSource.v;
    FileIO_String_ReplaceRange((FilePathString *)&self->pathFlags, 0, -1, 0, 0, &clearAllocator);
    if ((self->pathFlags >> 0x1F) == 0) {
        oldPathLength = *(u8 *)&self->pathFlags & 0x7F;
    } else {
        oldPathLength = self->pathLength;
    }
    copyAllocator.v = copyAllocatorSource.v;
    FileIO_String_ReplaceRange((FilePathString *)&self->pathFlags, 0, oldPathLength, path, path + strlen(path), &copyAllocator);

    access = 0;
    switch (flags & kAccessMask) {
    case kReadOnly:
        access = kNativeReadAccess;
        break;
    case kWriteOnly:
        access = kNativeWriteAccess;
        break;
    case kReadWrite:
        access = kNativeReadWriteAccess;
        break;
    }

    switch (flags & kShareMask) {
    default:
    case kDenyReadWrite:
    case 0x0:
        share = kNativeShareNone;
        break;
    case kDenyWrite:
        share = kNativeShareRead;
        break;
    case kDenyRead:
        share = kNativeShareWrite;
        break;
    case kAllowReadWrite:
        share = kNativeShareReadWrite;
        break;
    }

    if (flags & kCreate) {
        if (flags & kKeepExisting) {
            creation = kOpenAlways;
        } else {
            creation = kCreateAlways;
        }
    } else {
        creation = kOpenExisting;
    }

    attributes = kNormalAttributes;
    if (flags & kNoBuffering) {
        attributes |= kNativeNoBuffering;
    }
    if (flags & kWriteThrough) {
        attributes |= kNativeWriteThrough;
    }
    if (flags & kRandomAccess) {
        attributes |= kNativeRandomAccess;
    }
    if (flags & kSequentialAccess) {
        attributes |= kNativeSequentialAccess;
    }

    handle = FileIO_OpenFile(path, access, share, 0, creation, attributes, 0, -1, 0, 0);
    if ((u32)(handle + 0x10000) == 0xFFFF) {
        FileError_GetUnknownMessage();
        return 0;
    }
    self->handle = handle;
    self->opened = 1;
    return 1;
}

// Read bytes, query failure information if needed, and return the transferred byte count.
s32 gtl_CFile_Read(gtl_CFile *self, void *buffer, u32 length) {
    s32 bytesRead;
    if (length == 0) {
        return 0;
    }
    s32 readResult = FileIO_ReadFile(self->handle, buffer, length, &bytesRead, 0, -1, 0, 0);
    if (readResult == 0) {
        File_GetLastError();
    }
    return bytesRead;
}

// Write the supplied byte range and query failure information on an unsuccessful write.
void gtl_CFile_WriteBuffer(gtl_CFile *self, s32 bufferAddress, s32 bytesToWrite) {
    u32 bytesWritten;
    if (bytesToWrite != 0) {
        if (FileIO_WriteFile(self->handle, bufferAddress, bytesToWrite, &bytesWritten, -1, 0, 0) == 0) {
            File_GetLastError();
        }
    }
}

#pragma opt_lifetimes off
// Seek the handle and return the native result words without changing their original ABI.
s64 gtl_CFile_Seek64(gtl_CFile *self, s32 unused, s32 highOffset, s32 lowOffset, s32 origin) {
    union { s64 value; struct { s32 high; s32 low; } parts; } result;
    result.parts.high = highOffset;
    result.parts.low = lowOffset;
    s32 seekResult = FileIO_SeekFile(self->handle, lowOffset, &result.parts.high, origin, -1, 0, 0);
    result.parts.low = seekResult;
    if ((u32)(seekResult + 0x10000) == 0xFFFF) {
        if (File_GetLastError() != 0) {
            File_GetLastError();
        }
    }
    return result.value;
}
#pragma opt_lifetimes reset

// Dispatch device handle operation 10 and query failure information if it fails.
void gtl_CFile_vfunc_5(gtl_CFile *self) {
    s32 handle = self->handle;
    if ((u32)(handle + 0x10000) != 0xFFFF) {
        if (FileIO_DispatchDeviceHandleOperation10(handle) == 0) {
            File_GetLastError();
        }
    }
}

// Query the native file-size result and fetch error information on failure.
s32 gtl_CFile_GetFileSize(const gtl_CFile *self) {
    struct { s32 high; s32 low; } result;
    s32 sizeResult = FileIO_GetFileSize(self->handle, &result.high, 0, 0);
    result.low = sizeResult;
    if ((u32)(sizeResult + 0x10000) == 0xFFFF) {
        if (File_GetLastError() != 0) {
            File_GetLastError();
        }
    }
    return result.low;
}

// Close the handle, mark the file unopened, and clear its stored path.
void gtl_CFile_Close(gtl_CFile *self) {
    s32 failed = 0;
    if ((u32)(self->handle + 0x10000) != 0xFFFF) {
        failed = FileIO_CloseFile(self->handle) == 0;
    }
    self->handle = -1;
    self->opened = 0;
    StringAllocatorTag sp1C;
    volatile StringAllocatorTag sp8;
    sp1C.v = sp8.v;
    FileIO_String_ReplaceRange((FilePathString *)&self->pathFlags, 0, -1, 0, 0, &sp1C);
    if (failed) {
        File_GetLastError();
    }
}

// Close an owned open handle, release heap path storage, and optionally free the object.
gtl_CFile *gtl_CFile_dtor(gtl_CFile *self, s32 flags) {
    if (self) {
        ((void **)self)[1] = __vt__Q23gtl5CFile;
        if ((u32)(self->handle + 0x10000) != 0xFFFF && self->opened != 0) {
            self->v6();
        }
        if (&self->pathFlags) {
            if (self->pathFlags >> 31) {
                FileIO_Free((void *)self->pathData);
            }
        }
        if (flags > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

// Release heap storage for a file path and optionally delete the string object.
FilePathString *__dt__9CFilePathFv(FilePathString *self, s32 flags) {
    if (self) {
        if (self->heapbit) {
            FileIO_Free(self->ptr);
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// Grow aligned storage or edit in place, preserving any overlapping source range.
FilePathString *FileIO_String_ReplaceRange(FilePathString *self, u32 position, u32 removeCount, const char *first, const char *last, StringAllocatorTag *extra) {
    u32 tailLength;
    u32 usesHeap;
    u32 newLength;
    char *data;
    u32 length;
    u32 capacity;
    u32 removeLength;
    u32 insertLength;
    char *newData;
    volatile StringAllocatorTag c;
    StringAllocatorTag copy;

    usesHeap = self->heapbit;
    if (usesHeap == 0) {
        length = self->smallsize;
        data = (char *)self + 1;
        capacity = kInlineStringCapacity;
    } else {
        data = self->ptr;
        length = self->length;
        capacity = self->capacity;
    }
    if (position > length) {
        const char *rangeErrorText = stringRangeAndLengthExceptionStrings_8043EFB4;
        StringRangeError e(rangeErrorText);
    }
    u32 availableLength = length - position;
    const u32 *removeLimit = &removeCount;
    if (availableLength < removeCount) {
        removeLimit = &availableLength;
    }
    removeLength = *removeLimit;
    insertLength = last - first;
    if (insertLength > kMaximumStringLength || length - removeLength > kMaximumStringLength - insertLength) {
        const char *lengthErrorText = ((StringExceptionStrings*)stringRangeAndLengthExceptionStrings_8043EFB4)->lengthError;
        StringLengthError e(lengthErrorText);
    }
    newLength = insertLength + (length - removeLength);
    tailLength = length - (position + removeLength);
    if (newLength >= capacity) {
        capacity = (capacity + kStringAllocationAlignment - 1) & ~(kStringAllocationAlignment - 1);
        while (capacity < newLength + 1) {
            capacity = ((capacity * 2) + kStringAllocationAlignment - 1) & ~(kStringAllocationAlignment - 1);
        }
        newData = (char *)FileIO_Alloc(capacity);
        if (position) {
            memcpy(newData, data, position);
        }
        char *insertDestination = newData + position;
        memmove(insertDestination, first, last - first);
        if (tailLength) {
            memcpy(insertDestination + insertLength, data + position + removeLength, tailLength);
        }
        newData[newLength] = s_fileIoStringAllocatedTerminator;
        if (usesHeap) {
            FileIO_Free(data);
        } else {
            self->flag |= 0x80000000;
        }
        self->ptr = newData;
        self->length = newLength;
        self->capacity = capacity;
    } else {
        struct Temp : FilePathString {
            ~Temp() {
                if (flag >> 31) {
                    FileIO_Free(ptr);
                }
            }
        };
        Temp overlapCopy;
        overlapCopy.flag = 0;
        overlapCopy.length = 0;
        overlapCopy.ptr = 0;
        if (tailLength) {
            if (data + position + insertLength < last) {
                if (last <= data + length) {
                    u32 overlapLength = !(overlapCopy.flag >> 31) ? (*(u8 *)&overlapCopy & 0x7F) : 0;
                    copy.v = c.v;
                    FileIO_String_ReplaceRange(&overlapCopy, 0, overlapLength, first, last, &copy);
                    first = (overlapCopy.flag >> 31) ? overlapCopy.ptr : (char *)&overlapCopy + 1;
                }
            }
        }
        if (tailLength) {
            memmove(data + position + insertLength, data + position + removeLength, tailLength);
        }
        memmove(data + position, first, insertLength);
        data[newLength] = s_fileIoStringInPlaceTerminator;
        if (usesHeap == 0) {
            self->smallsize = newLength;
        } else {
            self->length = newLength;
        }
    }
    return self;
}
#pragma pop

} // extern "C"
