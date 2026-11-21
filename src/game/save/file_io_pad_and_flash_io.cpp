#include <decomp/static_init.h>
#include "classes/CFileIo.h"
#include "classes/CFileIoPad.h"

#define AsCFileIoPad(p) ((CFileIoPad*)(p))

#define AsCFileIo(p) ((CFileIo*)(p))
// Translation unit 0x80275074-0x80275A14 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
enum {
    kPathStringOffset = (int)&((CFileIo*)0)->pathFlags,
    kPathTextOffset = kPathStringOffset + 1,
    kDevicePrefixStringOffset = (int)&((CFileIo*)0)->devicePrefixFlags,
    kDevicePrefixTextOffset = kDevicePrefixStringOffset + 1,
    kSmallStringLengthMask = 0x7f, kDevicePathBufferBytes = 0x100,
    kInvalidHandleBias = 0x10000, kBiasedInvalidHandle = 0xFFFF,
    kCallbackOpenFlags = 0x60000000
};
static const unsigned int kReadWriteOpenAccess = 0xC0000000u;
struct FileIoTransferWork {
    char prefix[0x38];
    unsigned int deviceInfoWords0[2], deviceInfoWords1[2], deviceInfoWords2[2];
};
 
extern float kFloat_6_2831855;
extern float s_fileIoTwoPiCache;
extern unsigned int g_fileIoReferenceCountLimit;
extern void* g_fileIoPad;
extern char s_fileIoVtable[];
extern void* g_padFilePathPrefix;
extern char kPadFilePathSeparator[];
extern int CFileIo_SetPathAndAllocateBuffer(void*, void*, int);
void CFileIo_SetStatusCode(void*, int);
void CFileIoFlash_ClearBusy(void*);
void CFileIo_SetTransferredBytes(void*, int);
void* CFileIo_dtor(void*, int);
void __dl__FPv(void*);
void FileIO_CloseFile(int);
void appFreeUni__FPv(void*);
void __dla__FPv(void*);
void String_ReplaceRange(void*, unsigned, unsigned, const char*, const char*, void*);
void Memory_Clear(void*, int);
void Memory_Copy(void*, void*, unsigned);
// Select the registered device's normalized directory; shared rename is deferred.
void FileIO_SetCurrentDirectory(void*);
unsigned FileIO_GetDeviceTransferAlignment(void*);
int FileIO_OpenFile(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
void FileIO_GetDefaultDevicePath(int, void*);
void* __nwa__FUl(unsigned int);
extern char s_ownedByteBufferVtable[];
extern char __vt__20CSetCurrentDirectory[];
unsigned long strlen(const char*);

// Publish pad-file open status and clear the singleton's busy flag.
void CFileIoPad_OnOpenComplete(int status) {
    void* g = g_fileIoPad;
    if (g != 0) {
        CFileIo_SetStatusCode(g, status);
        CFileIoFlash_ClearBusy(g_fileIoPad);
    }
}

// Close a valid handle, store the invalid sentinel and report success.
int CFileIo_CloseHandle(void* self) {
    unsigned int value = (*(unsigned int*)&((CFileIo*)self)->fileHandle);
    unsigned int check = value + kInvalidHandleBias;
    if (check != kBiasedInvalidHandle) {
        FileIO_CloseFile(value);
    }
    ((CFileIo*)self)->fileHandle = -1;
    return 1;
}

// Initialize file status, handle, reference count, strings, buffer and transfer bookkeeping.
void CFileIo_InitMembers(void* self) {
    AsCFileIo(self)->vtable = (void*)s_fileIoVtable;
    (*(int*)&AsCFileIo(self)->ioBuffer) = 0;
    AsCFileIo(self)->statusCode = 0;
    AsCFileIo(self)->fileHandle = -1;
    AsCFileIo(self)->referenceCount = 0;
    AsCFileIo(self)->busy = 0;
    (*(int*)((char*)self + kPathStringOffset)) = 0;
    (*(int*)&AsCFileIo(self)->pathLength) = 0;
    (*(int*)&AsCFileIo(self)->pathData) = 0;
    (*(int*)((char*)self + kDevicePrefixStringOffset)) = 0;
    (*(int*)&AsCFileIo(self)->devicePrefixLength) = 0;
    AsCFileIo(self)->devicePrefixData = 0;
    AsCFileIo(self)->requestedBufferSize = 0;
    AsCFileIo(self)->saveAreaStatusFlags = 0;
    AsCFileIo(self)->transferredBytes = 0;
}

// Initialize this file-I/O module's two-pi cache.
void CFileIo_InitAngleCache(void) {
    s_fileIoTwoPiCache = kFloat_6_2831855;
}

// Store the save-area status flags and report success.
int CFileIo_SetSaveAreaStatusFlags(void* self, int saveAreaFlags) {
    ((CFileIo*)self)->saveAreaStatusFlags = saveAreaFlags;
    return 1;
}

// Increment the reference count unless it has reached the configured limit.
int CFileIo_AddRef(void* self) {
    if (g_fileIoReferenceCountLimit <= (*(unsigned int*)&((CFileIo*)self)->referenceCount)) {
        return 0;
    }
    (*(unsigned int*)&((CFileIo*)self)->referenceCount) = (*(unsigned int*)&((CFileIo*)self)->referenceCount) + 1;
    return 1;
}

// Return the file operation's busy byte.
unsigned char CFileIo_IsBusy(void* self) {
    return AsCFileIo(self)->busy;
}

// Return the allocated I/O buffer as an ABI-sized word.
int CFileIo_GetBuffer(void* self) {
    return (*(int*)&((CFileIo*)self)->ioBuffer);
}

// Return the last file operation's status code.
int CFileIo_GetStatusCode(void* self) {
    return ((CFileIo*)self)->statusCode;
}

// Return the cached save-area status flags.
int CFileIo_GetSaveAreaStatusFlags(void* self) {
    return ((CFileIo*)self)->saveAreaStatusFlags;
}

// Return the last transfer's completed byte count (shared accessor rename deferred).
int CFileIo_GetTransferredBytes(void* self) {
    return ((CFileIo*)self)->transferredBytes;
}

// Report that the pad-file device is available.
int CFileIoPad_vfunc_14(void) {
    return 1;
}

struct VTest {
    virtual void m0(int);
    virtual void m1(int);
    virtual void m2(int);
    virtual void m3(int);
    virtual void m4(int);
    virtual void m5(int);
    virtual void m6(int);
    virtual void m7(int);
    virtual void m8(int);
    virtual void m9(int);
    virtual void m10(int);
    virtual void m11(int);
    virtual void m12(int);
    virtual void m13(int);
    virtual void m14(int);
};

// Release one reference or invoke virtual deletion when the count was already zero.
int CFileIo_ReleaseRef(void* self) {
    int value = ((CFileIo*)self)->referenceCount;
    if (value != 0) {
        ((CFileIo*)self)->referenceCount = value - 1;
    } else {
        if (self != 0) {
            ((VTest*)self)->m12(1);
        }
    }
    return 1;
}

// Publish transfer status and bytes transferred, then clear the singleton's busy flag.
void CFileIoPad_OnTransferComplete(int status, int transferredBytes) {
    void* g = g_fileIoPad;
    if (g != 0) {
        CFileIo_SetStatusCode(g, status);
        CFileIo_SetTransferredBytes(g_fileIoPad, transferredBytes);
        CFileIoFlash_ClearBusy(g_fileIoPad);
    }
}

// Clear the pad-file singleton, destroy its base and optionally delete the object.
void* CFileIoPad_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        g_fileIoPad = 0;
        CFileIo_dtor(self, 0);
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Close the file and free owned I/O and string storage before optional deletion.
void* CFileIo_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        unsigned int v = (*(unsigned int*)&AsCFileIo(self)->fileHandle);
        AsCFileIo(self)->vtable = (void*)s_fileIoVtable;
        unsigned int check = v + kInvalidHandleBias;
        if (check != kBiasedInvalidHandle) {
            FileIO_CloseFile(v);
        }
        void* p = AsCFileIo(self)->ioBuffer;
        AsCFileIo(self)->fileHandle = -1;
        if (p != 0) {
            __dla__FPv(p);
            AsCFileIo(self)->ioBuffer = 0;
        }
        if ((unsigned int)((char*)self + kDevicePrefixStringOffset)) {
            if (AsCFileIo(self)->devicePrefixFlags >> 31) {
                appFreeUni__FPv((*(void**)&AsCFileIo(self)->devicePrefixData));
            }
        }
        if ((unsigned int)((char*)self + kPathStringOffset)) {
            if (AsCFileIo(self)->pathFlags >> 31) {
                appFreeUni__FPv(AsCFileIo(self)->pathData);
            }
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Both persistent path fields use the 12-byte small-string ABI: bit 31 selects
// pointer/length words; inline text starts one byte into the flags word. Local
// allocator bytes are construction bookkeeping, not path characters or modes.
struct StringAllocator { char value; };
struct LocalString {
    unsigned flag;
    unsigned size;
    char* ptr;
    // Copy a borrowed C string into an initially empty ABI string.
    LocalString(const char* input, StringAllocator& allocator, StringAllocator& extra) {
        unsigned count;
        const char* g = input;
        *(volatile unsigned*)&flag = 0;
        *(volatile unsigned*)&size = 0;
        *(volatile char**)&ptr = 0;
        unsigned f = flag;
        int notHeap = !(f >> 31);
        if (notHeap) count = *(unsigned char*)this & kSmallStringLengthMask;
        else count = size;
        extra.value = *(volatile char*)&allocator;
        unsigned len = strlen(g);
        String_ReplaceRange(this, 0, count, g, g + len, &extra);
    }
    // Release only heap storage; inline bytes belong to this object.
    ~LocalString() { if (flag >> 31) appFreeUni__FPv(ptr); }
    // Decode the current length in bytes.
    unsigned length() const {
        unsigned n;
        if (!(flag >> 31)) n = *(const unsigned char*)this & kSmallStringLengthMask;
        else n = size;
        return n;
    }
    // Return borrowed bytes valid until this string changes or is destroyed.
    void* data() {
        void* p;
        if (!(flag >> 31)) p = (char*)this + 1;
        else p = ptr;
        return p;
    }
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
// name is borrowed and copied before return; requestedBufferSize is in bytes.
// Success means path/buffer setup completed, not that a file has been opened.
// Build a pad-qualified path from the prefix, pad digit, separator and filename.
int CFileIoPad_SetPathAndAllocateBuffer(void* self, const char* name, int requestedBufferSize) {
    if (name == 0) {
        return 0;
    }
    char* g = (char*)g_padFilePathPrefix;
    StringAllocator allocator0;
    StringAllocator extra0;
    LocalString s(g, allocator0, extra0);
    // Append the ASCII digit for the padIndex field.
    {
        char c = (char)(AsCFileIoPad(self)->padIndex + '0');
        unsigned pos;
        unsigned f = s.flag;
        int notHeap = !(f >> 31);
        if (notHeap) pos = *(unsigned char*)&s & kSmallStringLengthMask;
        else pos = s.size;
        StringAllocator extra;
        StringAllocator allocator;
        extra.value = *(volatile char*)&allocator;
        String_ReplaceRange(&s, pos, 0, &c, &c + 1, &extra);
    }
    // append kPadFilePathSeparator
    {
        unsigned pos;
        char* str = kPadFilePathSeparator;
        unsigned f = s.flag;
        int notHeap = !(f >> 31);
        if (notHeap) pos = *(unsigned char*)&s & kSmallStringLengthMask;
        else pos = s.size;
        StringAllocator allocator;
        StringAllocator extra;
        extra.value = *(volatile char*)&allocator;
        unsigned len = strlen(str);
        String_ReplaceRange(&s, pos, 0, str, str + len, &extra);
    }
    // append name
    {
        char* str = (char*)name;
        unsigned pos;
        unsigned f = s.flag;
        int notHeap = !(f >> 31);
        if (notHeap) pos = *(unsigned char*)&s & kSmallStringLengthMask;
        else pos = s.size;
        StringAllocator allocator;
        StringAllocator extra;
        extra.value = *(volatile char*)&allocator;
        unsigned len = strlen(str);
        String_ReplaceRange(&s, pos, 0, str, str + len, &extra);
    }
    // Delegate the assembled path; the base setup copies it before s is destroyed.
    void* data;
    unsigned f = s.flag;
    int notHeap = !(f >> 31);
    if (notHeap) data = (char*)&s + 1;
    else data = s.ptr;
    if (CFileIo_SetPathAndAllocateBuffer(self, data, requestedBufferSize) == 0) {
        return 0;
    }
    return 1;
}
#pragma pop

// Original CFileIo_SetPathAndAllocateBuffer: locate the device-prefix separator.
inline void FindDeviceColon(int& colon, char*& q, char*& end, char*& begin, unsigned size) {
    if (size != 0) {
        end = begin + size;
        q = begin;
        for (; q < end; ++q)
            if (*q == ':') { colon = (int)(q - begin); return; }
    }
    colon = -1;
}
// Copy fullPath, retain its device prefix through ':', and allocate a transfer buffer.
// fullPath must be NUL-terminated. Returns 1 with a zero-filled owned buffer,
// 0 for missing colon, zero alignment/rounded size, or allocation failure.
// requestedBufferSize is stored unrounded; allocated bytes are rounded upward.
// Failure can leave the new path/prefix installed and the old buffer released.
int CFileIo_SetPathAndAllocateBuffer(void* selfIn, void* fullPath, int requestedBufferSize) {
    unsigned colonPlus;
    char* self = (char*)selfIn;
    StringAllocator pathAllocatorCopy;
    StringAllocator pathAllocator;
    StringAllocator prefixAllocator;
    Memory_Clear(((FileIoTransferWork*)self)->deviceInfoWords0, 8);
    Memory_Clear(((FileIoTransferWork*)self)->deviceInfoWords1, 8);
    Memory_Clear(((FileIoTransferWork*)self)->deviceInfoWords2, 8);
    AsCFileIo(self)->requestedBufferSize = requestedBufferSize;
    // Phase 1: replace the persistent path, then scan its decoded bytes for ':'.
    unsigned count;
    if (!(AsCFileIo(self)->pathFlags >> 31)) {
        count = (unsigned char)self[kPathStringOffset];
        count &= kSmallStringLengthMask;
    } else {
        count = AsCFileIo(self)->pathLength;
    }
    pathAllocatorCopy.value = *(volatile char*)&pathAllocator;
    unsigned len = (unsigned)strlen((const char*)fullPath);
    String_ReplaceRange(((char*)self + kPathStringOffset), 0, count, (const char*)fullPath, (const char*)fullPath + len, &pathAllocatorCopy);
    unsigned pathLengthForScan;
    char* pathBytesForScan;
    if (!(AsCFileIo(self)->pathFlags >> 31)) {
        pathLengthForScan = (unsigned char)self[kPathStringOffset] & kSmallStringLengthMask;
        pathBytesForScan = self + kPathTextOffset;
    } else {
        pathBytesForScan = (*(char**)&AsCFileIo(self)->pathData);
        pathLengthForScan = AsCFileIo(self)->pathLength;
    }
    int colon;
    {
        char* end;
        char* q;
        FindDeviceColon(colon, q, end, pathBytesForScan, pathLengthForScan);
    }
    // Phase 2: copy the prefix including ':'. colon==-1 is recognized by the
    // same unsigned bias comparison used for invalid file handles below.
    int ok;
    unsigned check = (unsigned)colon + kInvalidHandleBias;
    if (check == kBiasedInvalidHandle) {
        ok = 0;
    } else {
        unsigned clampedPrefixLength;
        colonPlus = (unsigned)(colon + 1);
        unsigned oldPrefixLength;
        if (!(AsCFileIo(self)->devicePrefixFlags >> 31)) {
            oldPrefixLength = (unsigned char)self[kDevicePrefixStringOffset];
            oldPrefixLength &= kSmallStringLengthMask;
        } else {
            oldPrefixLength = AsCFileIo(self)->devicePrefixLength;
        }
        unsigned pathLengthForPrefix;
        char* pathBytesForPrefix;
        if (!(AsCFileIo(self)->pathFlags >> 31)) {
            pathLengthForPrefix = (unsigned char)self[kPathStringOffset] & kSmallStringLengthMask;
            pathBytesForPrefix = self + kPathTextOffset;
        } else {
            pathBytesForPrefix = (*(char**)&AsCFileIo(self)->pathData);
            pathLengthForPrefix = AsCFileIo(self)->pathLength;
        }
        clampedPrefixLength = pathLengthForPrefix;
        unsigned* m = (pathLengthForPrefix < colonPlus) ? &clampedPrefixLength : &colonPlus;
        colonPlus = *m;
        StringAllocator extra8 = prefixAllocator;
        String_ReplaceRange(((char*)self + kDevicePrefixStringOffset), 0, oldPrefixLength, pathBytesForPrefix, pathBytesForPrefix + colonPlus, &extra8);
        ok = 1;
    }
    if (!ok) {
        return 0;
    }
    // Phase 3: discard the old buffer; round bytes to a device alignment multiple.
    void* p = AsCFileIo(self)->ioBuffer;
    if (p != 0) {
        __dla__FPv(p);
        AsCFileIo(self)->ioBuffer = 0;
    }
    char* devicePrefixBytes;
    if (!(AsCFileIo(self)->devicePrefixFlags >> 31)) {
        devicePrefixBytes = self;
        devicePrefixBytes += kDevicePrefixTextOffset;
    } else {
        devicePrefixBytes = (*(char**)&AsCFileIo(self)->devicePrefixData);
    }
    unsigned alignmentBytes = FileIO_GetDeviceTransferAlignment(devicePrefixBytes);
    unsigned rounded;
    if (alignmentBytes == 0) {
        rounded = 0;
    } else {
        unsigned requestedBytes = (*(unsigned int*)&AsCFileIo(self)->requestedBufferSize);
        rounded = alignmentBytes * ((requestedBytes + alignmentBytes - 1) / alignmentBytes);
    }
    if (rounded == 0) {
        return 0;
    }
    void* buf = __nwa__FUl(rounded);
    AsCFileIo(self)->ioBuffer = buf;
    if (buf == 0) {
        return 0;
    }
    Memory_Clear(buf, (int)rounded);
    return 1;
}

// Scope owner snapshots the device directory; destruction restores it on every
// return/unwind. This is separate from the temporary filename's storage owner.
// Retain Buf1/D2 type spelling for outlined destructor symbol correspondence.
struct Buf1 {
    void* vt;
    char buf[0x100];
    // Initialize the saved-directory buffer and restore-operation vtable.
    Buf1() : vt(__vt__20CSetCurrentDirectory), buf() {}
    // Restore the directory captured by GetDefaultDevicePath in the caller.
    ~Buf1() {
        vt = __vt__20CSetCurrentDirectory;
        FileIO_SetCurrentDirectory(buf);
    }
};
// The DOL's out-of-line Buf1 destructor (0x80276170, __dt__4Buf1Fv(this, flag)) sits in slot 2: it restores the
// vtable, restores the saved directory (buffer at +4), and frees for a positive flag.

// Temporary filename buffer owner; frees its array at scope exit.
struct D2 {
    void* vt;
    int size;
    void* f8;
    // Allocate the requested temporary filename bytes, if positive.
    D2(int n) : vt(s_ownedByteBufferVtable), size(n), f8(0) {
        if (n > 0) {
            f8 = __nwa__FUl((unsigned)n);
        }
    }
    // Free the copied filename after open has consumed it.
    ~D2() {
        vt = s_ownedByteBufferVtable;
        if (f8) __dla__FPv(f8);
        f8 = 0;
    }
};

#pragma push
#pragma use_lmw_stmw on
// Open the copied path; return 1 for an accepted handle, 0 on immediate failure.
// completionCallback is a callback ADDRESS encoded as int, zero for synchronous;
// openDisposition is forwarded as a word, not dereferenced. A nonzero callback
// selects 0x60000000 flags and sets busy before open. Busy clears on immediate
// open failure or via OnOpenComplete; allocation failure leaves it set as before.
// 0xC0000000 requests read/write access. The backend must consume/copy the path
// during FileIO_OpenFile: the temporary filename is freed even for async opens.
// Handle validity uses (handle+0x10000)!=0xFFFF, an unsigned spelling of != -1.
int CFileIo_Open(void* object, int completionCallback, void* openDisposition) {
    char* self = (char*)object;
    unsigned oldHandle = *(unsigned*)&AsCFileIo(self)->fileHandle;
    unsigned check = oldHandle + kInvalidHandleBias;
    if (check != kBiasedInvalidHandle) {
        FileIO_CloseFile(oldHandle);
    }
    ((CFileIo*)self)->fileHandle = -1;
    char* p;
    int notHeapP = !(AsCFileIo(self)->devicePrefixFlags >> 31);
    if (notHeapP) {
        p = self + kDevicePrefixTextOffset;
    } else {
        p = *(char**)&AsCFileIo(self)->devicePrefixData;
    }
    // Scope 1 selects the device directory and guarantees restoration.
    Buf1 restoreDirectory;
    FileIO_GetDefaultDevicePath(kDevicePathBufferBytes, restoreDirectory.buf);
    FileIO_SetCurrentDirectory(p);
    if (completionCallback) AsCFileIo(self)->busy = 1;
    void* pathBuffer;
    unsigned pathLength;
    int notHeapN = !(AsCFileIo(self)->pathFlags >> 31);
    if (notHeapN) {
        pathLength = (unsigned char)self[kPathStringOffset] & kSmallStringLengthMask;
    } else {
        pathLength = AsCFileIo(self)->pathLength;
    }
    int allocSize = (int)pathLength + 3;
    // Scope 2 owns a zero-terminated copy with three extra zero bytes. It lasts
    // only through the backend call, never until the asynchronous callback.
    D2 openPathStorage(allocSize);
    pathBuffer = openPathStorage.f8;
    if (pathBuffer == 0) {
        return 0;
    }
    Memory_Clear(pathBuffer, allocSize);
    int notHeap = !(AsCFileIo(self)->pathFlags >> 31);
    unsigned srcSize;
    if (notHeap) {
        srcSize = (unsigned char)self[kPathStringOffset] & kSmallStringLengthMask;
    } else {
        srcSize = AsCFileIo(self)->pathLength;
    }
    char* src;
    if (notHeap) {
        src = self + kPathTextOffset;
    } else {
        src = *(char**)&AsCFileIo(self)->pathData;
    }
    Memory_Copy(pathBuffer, src, srcSize);
    int asyncFlags = ((-completionCallback | completionCallback) >> 31) & kCallbackOpenFlags;
    int result = FileIO_OpenFile((unsigned int)(unsigned long)pathBuffer, kReadWriteOpenAccess, 0, 0, (unsigned int)(unsigned long)openDisposition, (unsigned int)asyncFlags, 0, (unsigned int)-1, (unsigned int)completionCallback, 0);
    ((CFileIo*)self)->fileHandle = result;
    if ((unsigned int)(result + kInvalidHandleBias) == (unsigned int)kBiasedInvalidHandle) {
        AsCFileIo(self)->busy = 0;
        return 0;
    }
    return 1;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462390", _ctors_u80275074_0, CFileIo_InitAngleCache);
#pragma pop
