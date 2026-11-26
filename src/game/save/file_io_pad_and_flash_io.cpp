// File I/O over the controller-memory pad and the flash save area.
//
// Both devices share CFileIo's handle, status and transfer buffer. This file
// builds their qualified paths, opens files, publishes completion status, and
// tears the objects down. The pad device adds a channel number to its path.
#include <decomp/static_init.h>
#include "classes/CFileIo.h"
#include "classes/CFileIoPad.h"
#include "classes/CFileIoFlash.h"

extern "C" {
enum {
    kSmallStringLengthMask = 0x7f,
    kDevicePathBufferBytes = 0x100,
    kInvalidHandleBias = 0x10000,
    kBiasedInvalidHandle = 0xFFFF,
    kCallbackOpenFlags = 0x60000000
};
static const unsigned int kReadWriteOpenAccess = 0xC0000000u;

extern float kFloat_6_2831855;
extern float s_fileIoTwoPiCache;
extern unsigned int g_fileIoReferenceCountLimit;
extern void* g_fileIoPad;
extern char s_fileIoVtable[];
extern void* g_padFilePathPrefix;
extern char kPadFilePathSeparator[];
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
int FileIO_OpenFile(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                    unsigned int, unsigned int, unsigned int);
void FileIO_GetDefaultDevicePath(int, void*);
void* __nwa__FUl(unsigned int);
extern char s_ownedByteBufferVtable[];
extern char __vt__20CSetCurrentDirectory[];
unsigned long strlen(const char*);

// Publish pad-file open status and clear the singleton's busy flag.
void CFileIoPad::OnOpenComplete(int status) {
    void* g = g_fileIoPad;
    if (g != 0) {
        ((CFileIo*)g)->SetStatusCode(status);
        ((CFileIoFlash*)g_fileIoPad)->ClearBusy();
    }
}

// Close a valid handle, store the invalid sentinel and report success.
int CFileIo::CloseHandle() {
    unsigned int value = (*(unsigned int*)&fileHandle);
    unsigned int check = value + kInvalidHandleBias;
    if (check != kBiasedInvalidHandle) {
        FileIO_CloseFile(value);
    }
    fileHandle = -1;
    return 1;
}

// Initialize file status, handle, reference count, strings, buffer and transfer bookkeeping.
void CFileIo::InitMembers() {
    vtable = (void*)s_fileIoVtable;
    ioBuffer = 0;
    statusCode = 0;
    fileHandle = -1;
    referenceCount = 0;
    busy = 0;
    pathFlags = 0;
    pathLength = 0;
    pathData = 0;
    devicePrefixFlags = 0;
    devicePrefixLength = 0;
    devicePrefixData = 0;
    requestedBufferSize = 0;
    saveAreaStatusFlags = 0;
    transferredBytes = 0;
}

// Initialize this file-I/O module's two-pi cache.
void CFileIo_InitAngleCache(void) {
    s_fileIoTwoPiCache = kFloat_6_2831855;
}

// Store the save-area status flags and report success.
int CFileIo::SetSaveAreaStatusFlags(int saveAreaFlags) {
    saveAreaStatusFlags = saveAreaFlags;
    return 1;
}

// Increment the reference count unless it has reached the configured limit.
int CFileIo::AddRef() {
    if (g_fileIoReferenceCountLimit <= (*(unsigned int*)&referenceCount)) {
        return 0;
    }
    (*(unsigned int*)&referenceCount) = (*(unsigned int*)&referenceCount) + 1;
    return 1;
}

// Return the file operation's busy byte.
int CFileIo::IsBusy() {
    return busy;
}

// Return the allocated I/O buffer as an ABI-sized word.
int CFileIo::GetBuffer() {
    return (*(int*)&ioBuffer);
}

// Return the last file operation's status code.
int CFileIo::GetStatusCode() {
    return statusCode;
}

// Return the cached save-area status flags.
int CFileIo::GetSaveAreaStatusFlags() {
    return saveAreaStatusFlags;
}

// Return the last transfer's completed byte count (shared accessor rename deferred).
int CFileIo::GetTransferredBytes() {
    return transferredBytes;
}

// Always report the device as available.
int CFileIoPad::vfunc_14() {
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
int CFileIo::ReleaseRef() {
    int value = referenceCount;
    if (value != 0) {
        referenceCount = value - 1;
    } else {
        if (this != 0) {
            ((VTest*)this)->m12(1);
        }
    }
    return 1;
}

// Publish transfer status and bytes transferred, then clear the singleton's busy flag.
void CFileIoPad::OnTransferComplete(int status, int bytesTransferred) {
    void* g = g_fileIoPad;
    if (g != 0) {
        ((CFileIo*)g)->SetStatusCode(status);
        ((CFileIo*)g_fileIoPad)->SetTransferredBytes(bytesTransferred);
        ((CFileIoFlash*)g_fileIoPad)->ClearBusy();
    }
}

// Clear the pad-file singleton; the base destructor closes the file and frees
// the owned storage.
CFileIoPad::~CFileIoPad() {
    g_fileIoPad = 0;
}

// Close the file and free owned I/O and string storage before optional deletion.
CFileIo::~CFileIo() {
    unsigned int handle = (*(unsigned int*)&fileHandle);
    vtable = (void*)s_fileIoVtable;
    unsigned int check = handle + kInvalidHandleBias;
    if (check != kBiasedInvalidHandle) {
        FileIO_CloseFile(handle);
    }
    void* buffer = ioBuffer;
    fileHandle = -1;
    if (buffer != 0) {
        __dla__FPv(buffer);
        ioBuffer = 0;
    }
    // The original inlines the release of each member string, including its
    // non-null check on the member address.
    if (&devicePrefixFlags) {
        if (devicePrefixFlags >> 31) {
            appFreeUni__FPv(devicePrefixData);
        }
    }
    if (&pathFlags) {
        if (pathFlags >> 31) {
            appFreeUni__FPv(pathData);
        }
    }
}

// Both persistent path fields use the 12-byte small-string ABI: bit 31 selects
// pointer/length words; inline text starts one byte into the flags word. Local
// allocator bytes are construction bookkeeping, not path characters or modes.
struct StringAllocator {
    char value;
};
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
        if (notHeap)
            count = *(unsigned char*)this & kSmallStringLengthMask;
        else
            count = size;
        extra.value = *(volatile char*)&allocator;
        unsigned len = strlen(g);
        String_ReplaceRange(this, 0, count, g, g + len, &extra);
    }
    // Release only heap storage; inline bytes belong to this object.
    ~LocalString() {
        if (flag >> 31)
            appFreeUni__FPv(ptr);
    }
    // Decode the current length in bytes.
    unsigned length() const {
        unsigned n;
        if (!(flag >> 31))
            n = *(const unsigned char*)this & kSmallStringLengthMask;
        else
            n = size;
        return n;
    }
    // Return borrowed bytes valid until this string changes or is destroyed.
    void* data() {
        void* p;
        if (!(flag >> 31))
            p = (char*)this + 1;
        else
            p = ptr;
        return p;
    }
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
// name is borrowed and copied before return; requestedBufferSize is in bytes.
// Success means path/buffer setup completed, not that a file has been opened.
// Build a pad-qualified path from the prefix, pad digit, separator and filename.
int CFileIoPad::SetPathAndAllocateBuffer(const char* name, int requestedBufferSize) {
    if (name == 0) {
        return 0;
    }
    char* g = (char*)g_padFilePathPrefix;
    StringAllocator allocator0;
    StringAllocator extra0;
    LocalString s(g, allocator0, extra0);
    // Append the ASCII digit for the padIndex field.
    {
        char c = (char)(padIndex + '0');
        unsigned pos;
        unsigned f = s.flag;
        int notHeap = !(f >> 31);
        if (notHeap)
            pos = *(unsigned char*)&s & kSmallStringLengthMask;
        else
            pos = s.size;
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
        if (notHeap)
            pos = *(unsigned char*)&s & kSmallStringLengthMask;
        else
            pos = s.size;
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
        if (notHeap)
            pos = *(unsigned char*)&s & kSmallStringLengthMask;
        else
            pos = s.size;
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
    if (notHeap)
        data = (char*)&s + 1;
    else
        data = s.ptr;
    if (CFileIo::SetPathAndAllocateBuffer((const char*)data, requestedBufferSize) == 0) {
        return 0;
    }
    return 1;
}
#pragma pop

// Locate the first ':' in the path; it ends the device prefix.
inline void FindDeviceColon(int& colon, char*& q, char*& end, char*& begin, unsigned size) {
    if (size != 0) {
        end = begin + size;
        q = begin;
        for (; q < end; ++q)
            if (*q == ':') {
                colon = (int)(q - begin);
                return;
            }
    }
    colon = -1;
}
// Copy fullPath, retain its device prefix through ':', and allocate a transfer buffer.
// fullPath must be NUL-terminated. Returns 1 with a zero-filled owned buffer,
// 0 for missing colon, zero alignment/rounded size, or allocation failure.
// requestedBufferSize is stored unrounded; allocated bytes are rounded upward.
// Failure can leave the new path/prefix installed and the old buffer released.
int CFileIo::SetPathAndAllocateBuffer(const char* fullPath, int requestedBufferSize) {
    unsigned colonPlus;
    StringAllocator pathAllocatorCopy;
    StringAllocator pathAllocator;
    StringAllocator prefixAllocator;
    Memory_Clear(&deviceInfo[0], 8);
    Memory_Clear(&deviceInfo[2], 8);
    Memory_Clear(&deviceInfo[4], 8);
    this->requestedBufferSize = requestedBufferSize;
    // Phase 1: replace the persistent path, then scan its decoded bytes for ':'.
    unsigned count;
    if (!(pathFlags >> 31)) {
        // The length byte is read through a char pointer, as the original does:
        // the load and the mask stay in separate registers.
        count = (unsigned char)((char*)&pathFlags)[0];
        count &= kSmallStringLengthMask;
    } else {
        count = pathLength;
    }
    pathAllocatorCopy.value = *(volatile char*)&pathAllocator;
    unsigned len = (unsigned)strlen(fullPath);
    String_ReplaceRange(&pathFlags, 0, count, fullPath, fullPath + len, &pathAllocatorCopy);
    unsigned pathLengthForScan;
    char* pathBytesForScan;
    if (!(pathFlags >> 31)) {
        pathLengthForScan = *(unsigned char*)&pathFlags & kSmallStringLengthMask;
        pathBytesForScan = (char*)&pathFlags + 1;
    } else {
        pathBytesForScan = (char*)pathData;
        pathLengthForScan = pathLength;
    }
    int colon;
    {
        char* end;
        char* q;
        FindDeviceColon(colon, q, end, pathBytesForScan, pathLengthForScan);
    }
    // Phase 2: copy the prefix including ':'. colon == -1 is recognized by the
    // same unsigned bias comparison used for invalid file handles below.
    int ok;
    unsigned check = (unsigned)colon + kInvalidHandleBias;
    if (check == kBiasedInvalidHandle) {
        ok = 0;
    } else {
        unsigned clampedPrefixLength;
        colonPlus = (unsigned)(colon + 1);
        unsigned oldPrefixLength;
        if (!(devicePrefixFlags >> 31)) {
            oldPrefixLength = (unsigned char)((char*)&devicePrefixFlags)[0];
            oldPrefixLength &= kSmallStringLengthMask;
        } else {
            oldPrefixLength = devicePrefixLength;
        }
        unsigned pathLengthForPrefix;
        char* pathBytesForPrefix;
        if (!(pathFlags >> 31)) {
            pathLengthForPrefix = *(unsigned char*)&pathFlags & kSmallStringLengthMask;
            pathBytesForPrefix = (char*)&pathFlags + 1;
        } else {
            pathBytesForPrefix = (char*)pathData;
            pathLengthForPrefix = pathLength;
        }
        clampedPrefixLength = pathLengthForPrefix;
        unsigned* m = (pathLengthForPrefix < colonPlus) ? &clampedPrefixLength : &colonPlus;
        colonPlus = *m;
        StringAllocator extra8 = prefixAllocator;
        String_ReplaceRange(&devicePrefixFlags, 0, oldPrefixLength, pathBytesForPrefix, pathBytesForPrefix + colonPlus,
                            &extra8);
        ok = 1;
    }
    if (!ok) {
        return 0;
    }
    // Phase 3: discard the old buffer; round bytes to a device alignment multiple.
    void* p = ioBuffer;
    if (p != 0) {
        __dla__FPv(p);
        ioBuffer = 0;
    }
    char* devicePrefixBytes;
    if (!(devicePrefixFlags >> 31)) {
        devicePrefixBytes = (char*)&devicePrefixFlags;
        devicePrefixBytes += 1;
    } else {
        devicePrefixBytes = (char*)devicePrefixData;
    }
    unsigned alignmentBytes = FileIO_GetDeviceTransferAlignment(devicePrefixBytes);
    unsigned rounded;
    if (alignmentBytes == 0) {
        rounded = 0;
    } else {
        unsigned requestedBytes = (*(unsigned int*)&this->requestedBufferSize);
        rounded = alignmentBytes * ((requestedBytes + alignmentBytes - 1) / alignmentBytes);
    }
    if (rounded == 0) {
        return 0;
    }
    void* buf = __nwa__FUl(rounded);
    ioBuffer = buf;
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
        if (f8)
            __dla__FPv(f8);
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
int CFileIo::Open(int completionCallback, void* openDisposition) {
    unsigned oldHandle = *(unsigned*)&fileHandle;
    unsigned check = oldHandle + kInvalidHandleBias;
    if (check != kBiasedInvalidHandle) {
        FileIO_CloseFile(oldHandle);
    }
    fileHandle = -1;
    char* p;
    int notHeapP = !(devicePrefixFlags >> 31);
    if (notHeapP) {
        p = (char*)&devicePrefixFlags + 1;
    } else {
        p = (char*)devicePrefixData;
    }
    // Scope 1 selects the device directory and guarantees restoration.
    Buf1 restoreDirectory;
    FileIO_GetDefaultDevicePath(kDevicePathBufferBytes, restoreDirectory.buf);
    FileIO_SetCurrentDirectory(p);
    if (completionCallback)
        busy = 1;
    void* pathBuffer;
    unsigned pathByteCount;
    int notHeapN = !(pathFlags >> 31);
    if (notHeapN) {
        pathByteCount = *(unsigned char*)&pathFlags & kSmallStringLengthMask;
    } else {
        pathByteCount = pathLength;
    }
    int allocSize = (int)pathByteCount + 3;
    // Scope 2 owns a zero-terminated copy with three extra zero bytes. It lasts
    // only through the backend call, never until the asynchronous callback.
    D2 openPathStorage(allocSize);
    pathBuffer = openPathStorage.f8;
    if (pathBuffer == 0) {
        return 0;
    }
    Memory_Clear(pathBuffer, allocSize);
    int notHeap = !(pathFlags >> 31);
    unsigned srcSize;
    if (notHeap) {
        srcSize = *(unsigned char*)&pathFlags & kSmallStringLengthMask;
    } else {
        srcSize = pathLength;
    }
    char* src;
    if (notHeap) {
        src = (char*)&pathFlags + 1;
    } else {
        src = (char*)pathData;
    }
    Memory_Copy(pathBuffer, src, srcSize);
    int asyncFlags = ((-completionCallback | completionCallback) >> 31) & kCallbackOpenFlags;
    int result = FileIO_OpenFile((unsigned int)(unsigned long)pathBuffer, kReadWriteOpenAccess, 0, 0,
                                 (unsigned int)(unsigned long)openDisposition, (unsigned int)asyncFlags, 0,
                                 (unsigned int)-1, (unsigned int)completionCallback, 0);
    fileHandle = result;
    if ((unsigned int)(result + kInvalidHandleBias) == (unsigned int)kBiasedInvalidHandle) {
        busy = 0;
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
