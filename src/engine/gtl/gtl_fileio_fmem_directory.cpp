extern "C" void* FileIOString_dtor(void*, int);

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm an exceptional-exit owner; the pointed-to storage remains borrowed.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm cleanup after normal-path disposal.
    void release() { active = false; }
    // Destroy the string subobject on an exceptional exit when still armed.
    ~EH5Cleanup() { if (active) destroy(object, -1); }
};

#include "types.h"
#define char_array_guard CharArrayGuard_u8041C598
#include "game/stdexcept_local.h"
#undef char_array_guard
#include "sdk_protos.h"


// The shared declaration marks __throw nothrow. Dispatch through a volatile
// function pointer so enclosing owners retain their original unwind coverage.
// Build and throw an owning std::out_of_range using the supplied message/type tag.
static void EH5RaiseOutOfRange(const char* text, const char* typeName) {
    struct Error { void* table; char* message; void* control; } error;
    error.table = __vt__Q23std11logic_error;
    std::detail::default_delete_chars deleter;
    unsigned int length = strlen(text) + 1;
    error.message = new char[length];
    {
        std::detail::CharArrayGuard_u8041C598 guard(error.message, deleter);
        error.control = new std::detail::message_control(error.message);
        guard.release();
    }
    strcpy(error.message, text);
    error.table = __vt__Q23std12out_of_range;
    typedef void (*ThrowFunction)(const char*, void*, void*);
    ThrowFunction volatile dispatch = __throw;
    dispatch(typeName, &error, (void*)std::__dt__Q23std12out_of_rangeFv);
}

extern "C" {
extern void File_SetLastError(int x);
extern void *memset(void*, int, unsigned int);
extern void FileIO_Free(void *p);
extern void FileIOString_ReplaceRangeWithAllocator(void *a, unsigned int b, unsigned int c, const char *d, const char *e, void *f);
extern void *NANDReadDir(char *a, int b, void *c);
extern void __throw(const char *a, void *b, void *c);
extern char filePathUtilityStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern void __dt__Q23std12out_of_rangeFv(void *p);

struct FmemDirectorySearchHandle {
    int vtable;
    char pad[0x0C];
    void *packedNames;                    // Borrowed packed NUL-terminated names.
    unsigned packedNameByteCount;                // Extent of packedNames in bytes.
    char pad2[0x04];
    unsigned nextNameByteOffset;                 // Next name's byte offset in packedNames.
    unsigned packedNameCount;               // Number of packed names.
    unsigned nameIndex;           // Next logical name index after acceptance.
};
// This is the directory search handle. +0 is its vtable;
// +0x10/+0x14 are packed-name bytes
// and byte extent, +0x1C is a byte cursor, +0x20 name count, +0x24 next index.
// Names are NUL-terminated and must fit wholly within that extent: the code
// checks the cursor but strlen does not bound the terminator search.
// Directory enumeration writes a fixed-size result with its NAND path at +0x2C.
struct FmemDirectoryEntry {
    int flags;
    char metadata[0x28];
    char path[0x114];
};
typedef char FmemDirectoryEntry_size[sizeof(FmemDirectoryEntry) == 0x140 ? 1 : -1];
enum DirectorySearchResult { kDirectoryEntryFlag = 0x10, kInvalidDirectoryHandle = 6,
                            kInlineNameLengthMask = 0x7F,
                            kNameRangeMessageOffset = 0x56, kRangeTypeNameOffset = 0x74 };

// Decode the byte length of the handle routine's inline/heap temporary string.
static inline void DirectoryNameLength(unsigned* storage, unsigned& length) {
    if (storage[0] >> 31) length = storage[1];
    else length = *(unsigned char*)&storage[0] & kInlineNameLengthMask;
}

// Borrow the temporary string's NUL-terminated data until replacement or cleanup.
static inline void DirectoryNameData(unsigned* storage, char*& data) {
    if (storage[0] >> 31) data = (char*)storage[2];
    else data = (char*)&storage[0] + 1;
}

#pragma use_lmw_stmw on
// Advance through packed names until the handle's matcher accepts one, then fill its directory entry.
// directoryHandle must be a valid borrowed handle or (void*)-1; null is not
// checked. outEntry needs 0x140 writable bytes and a name that fits its path.
// Returns 1 for an entry, 0 for invalid handle/exhaustion; exhaustion leaves the
// previous last-error value. Range overrun throws. Output owns no allocations.
// Rejected names consume byte positions even on exhaustion, but nameIndex is
// committed only after acceptance. Reusing an exhausted handle can thus throw.
int gtl_CFileIoDeviceFMEM_ReadNextDirectoryEntry(void *unused, void *directoryHandle, void *outEntry) {
    if (directoryHandle == (void*)-1) {
        File_SetLastError(kInvalidDirectoryHandle);
        return 0;
    }
    FmemDirectorySearchHandle *search = (FmemDirectorySearchHandle*)directoryHandle;
    int entryFound = 0;
    unsigned nameIndex;
    unsigned nameStorage[3];
    char allocatorToken = 0;
    // Stateless allocator representation: only its address is meaningful to
    // the string API. The original byte copies are retained for ABI matching;
    // this uninitialized byte is not an enumeration option or a name character.
    char allocatorSource;
    (void)allocatorSource;

    // Phase 1: own a 12-byte temporary string. Bit 31 of word 0 selects heap
    // length/data in words 1/2; otherwise its first byte is the inline length
    // and text starts at byte 1 (native big-endian layout).
    // The metadata aliases below describe the out_of_range construction path;
    // their unused aliases emit no normal-path runtime setup.
    int zeroWord = 0;
    char *errorStrings = filePathUtilityStrings;
    char *logicErrorVtable = __vt__Q23std11logic_error;
    char *charDeleterVtable = s_sharedCharDeleterVtable;
    char *rangeErrorVtable = __vt__Q23std12out_of_range;
    nameStorage[0] = zeroWord;
    nameStorage[1] = zeroWord;
    nameIndex = search->nameIndex;
    nameStorage[2] = zeroWord;
    EH5Cleanup eh5Cleanup36(nameStorage, (void (*)(void*, int))FileIOString_dtor);
    allocatorToken = (char)allocatorSource;
    void *dtor = (void*)__dt__Q23std12out_of_rangeFv;
    (void)charDeleterVtable; (void)dtor; (void)logicErrorVtable; (void)rangeErrorVtable; (void)errorStrings;

    // Phase 2: copy each candidate before advancing the byte cursor and testing
    // the handle's pattern matcher (vtable +8). Its input is borrowed temporary text.
    while (nameIndex < search->packedNameCount) {
        unsigned bytePosition = search->nextNameByteOffset;
        unsigned packedByteCount = search->packedNameByteCount;
        if (bytePosition >= packedByteCount) {
            EH5RaiseOutOfRange(errorStrings + kNameRangeMessageOffset, errorStrings + kRangeTypeNameOffset);
        }
        char *packedName = (char*)search->packedNames + bytePosition;
        unsigned previousLength;
        DirectoryNameLength(nameStorage, previousLength);
        allocatorToken = (char)allocatorSource;
        unsigned candidateByteLength = strlen(packedName);
        FileIOString_ReplaceRangeWithAllocator(&nameStorage[0], 0, previousLength, packedName, packedName + candidateByteLength, &allocatorToken);

        unsigned nameLength;
        DirectoryNameLength(nameStorage, nameLength);
        unsigned nameEndOffset = nameLength + search->nextNameByteOffset;
        search->nextNameByteOffset = nameEndOffset + 1;

        char *candidateName;
        DirectoryNameData(nameStorage, candidateName);

        void **vt = *(void***)search;
        typedef int (*Fn)(void*, char*);
        int accepted = ((Fn)vt[2])(search, candidateName);
        if (accepted != 0) {
            entryFound = 1;
            search->nameIndex = nameIndex + 1;
            break;
        } else {
            nameIndex++;
        }
    }

    if (!entryFound) {
        if (nameStorage[0] >> 31) { eh5Cleanup36.release(); FileIO_Free((void*)nameStorage[2]); }
        return 0;
    }

    // Phase 3: flatten the accepted name into a zeroed entry. NANDReadDir is
    // used only as a directory probe: success sets bit 0x10; other errors are
    // ignored and last-error is cleared because enumeration itself succeeded.
    char *acceptedName;
    DirectoryNameData(nameStorage, acceptedName);
    memset(outEntry, 0, sizeof(FmemDirectoryEntry));
    strcpy(((FmemDirectoryEntry*)outEntry)->path, acceptedName);
    unsigned childCount = 0;
    int nandResult = (int)NANDReadDir(((FmemDirectoryEntry*)outEntry)->path, 0, &childCount);
    if (nandResult == 0) {
        ((FmemDirectoryEntry*)outEntry)->flags |= kDirectoryEntryFlag;
    }
    File_SetLastError(0);
    if (nameStorage[0] >> 31) { eh5Cleanup36.release(); FileIO_Free((void*)nameStorage[2]); }
    return 1;
}
#pragma use_lmw_stmw reset

} // extern "C"
