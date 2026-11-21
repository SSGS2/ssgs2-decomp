#include <decomp/ct_member.h>
typedef unsigned long new_size_t;
struct FmemPathStringPool {
    char wildcardPathSuffix[3], pathSeparator[2], devicePrefix[6];
    char vectorLengthError[20], lengthErrorType[55];
    char vectorIndexOutOfRange[30], outOfRangeType[55];
    char stringReserveLengthError[35], stringOutOfRange[27], stringLengthError[27];
};
typedef char FmemPathStringPoolSize[(sizeof(FmemPathStringPool) == 0x104) ? 1 : -1];
// Retain byte-based addresses: direct typed pool pointers change CodeWarrior's hoisting.
enum FmemPathStringOffsets {
    kFmemPathSeparator = (int)&((FmemPathStringPool*)0)->pathSeparator,
    kFmemVectorLengthError = (int)&((FmemPathStringPool*)0)->vectorLengthError,
    kFmemLengthErrorType = (int)&((FmemPathStringPool*)0)->lengthErrorType,
    kFmemVectorIndexOutOfRange = (int)&((FmemPathStringPool*)0)->vectorIndexOutOfRange,
    kFmemOutOfRangeType = (int)&((FmemPathStringPool*)0)->outOfRangeType
};

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
void __dt__Q23std12out_of_rangeFv(void*, int);
extern const char filePathUtilityStrings[];
}
}

// Translation unit 0x8041B6A4-0x8041C290 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

extern "C" {

void* FileIO_Alloc(unsigned int);
void FileIO_Free(void*);
// Legacy shared spelling: this callback wrapper is used to release asynchronous storage.
void FileIO_InvokeAllocatorCallback3(void*);
void File_SetLastError(int);
char* strcpy(char*, const char*);
unsigned int strlen(const char*);
char* strchr(const char*, int);
char* strrchr(const char*, int);
void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __dla__FPv(void*);
int NANDReadDir(const char*, void*, unsigned int*);
int NANDReadDirAsync(const char*, void*, unsigned int*, void*, void*);
void NANDSetUserData(void*, void*);
void gtl_CFileIoDeviceFMEM_DispatchNandCallback();
void FileIOString_ReplaceRangeWithAllocator(void*, unsigned int, unsigned int, const char*, const char*, void*);
void FileIOString_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, void*);
void FileIOString_dtor(void*, int);
void FileIOString_dtor_8041F6F4(void*, int);
void dtor_8041C294(void*, int);
void FindHandleNames_dtor_8041C514(void*, int);
void FileIOCallback_dtor(void*, int);
extern char __vt__Q23gtl15CFindHandleFMEM[];
extern char __vt__Q23gtl16CFindHandleFMEMi[];
extern char __vt__Q23gtl22CCallBackFindFirstFile[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std12out_of_range[];
extern char s_sharedCharDeleterVtable[], filePathUtilityStrings[], s_nandAsyncCommandBlock[];
struct Link3Async { void* callback; void* buffer; unsigned int size; };
extern Link3Async s_fmemAsyncOperation;

// FindFirstFile's output uses the file layer's 0x140-byte find-data record.
struct FmemFindData {
    unsigned int attributes;
    char metadata[0x28];
    char name[0x114];
};
typedef char FmemFindDataSize[(sizeof(FmemFindData) == 0x140) ? 1 : -1];
typedef char FmemFindNameOffset[((int)&((FmemFindData*)0)->name == 0x2C) ? 1 : -1];
enum FmemFindConstants {
    kFindDataDirectory = 0x10,
    kNandDirectoryNameBytes = 13, // Maximum 12-character name plus terminator.
    kNandBufferAlignment = 32,
    kFindFirstOperationBytes = 0xC4,
    kFileErrorNotFound = 2,
    kNandErrorBase = 50000,
    kFileErrorUnknownNand = 50016,
    kFileErrorNandFatal = 50017
};

// Completion receives status, transient find-data, the search handle and borrowed user context.
// gtl_file_io_nand_dir_listing.cpp passes an uninitialized find-data record on failure: inspect status first.
typedef void* (*FmemFindCompletion)(void* status, FmemFindData*, void* handle, void* context);

struct Link3SmallString {
    unsigned int flags, length;
    char* data;
    // Read the character count from the active inline/heap representation.
    unsigned int size() const { return flags >> 31 ? length : *(const unsigned char*)this & 0x7f; }
    // Borrow the active character storage until this string changes or is destroyed.
    const char* text() const { return flags >> 31 ? data : (const char*)this + 1; }
};
struct Link3FindHandle {
    void** vtable;
    Link3SmallString pattern;
    void* names;
    unsigned int bytes, capacity, offset, count, next;
};
struct Link3Callback {
    void** vtable;
    Link3FindHandle* handle;
    Link3SmallString path;
    FmemFindCompletion callback;
    void* context;
};
struct Link3CharGuard {
    char* data;
    // Release a message array if exception construction has not transferred it.
    ~Link3CharGuard() { if (data) __dla__FPv(data); }
};
struct Link3ErrorControl { void* table; int strong, weak; char* text; };
// Construct a foreign exception object in caller-provided aligned storage.
void* operator new(new_size_t, void* p) { return p; }

} // extern "C"

namespace std {
struct exception {
    void* table;
    // Install the base exception table before the derived error table.
    exception() { table = __vt__Q23std11logic_error; }
    ~exception();
};
struct length_error : exception {
    char* message;
    void* control;
    length_error(const char* text);
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
struct out_of_range : exception {
    char* message;
    void* control;
    out_of_range(const char* text);
    ~out_of_range();
    DECOMP_CT_MEMBER(out_of_range)
};
}

// Allocate an exception message and its shared ownership record with unwind cleanup.
static void Link3MakeMessage(char*& message, void*& control, const char* text) {
    message = (char*)__nwa__FUl(strlen(text) + 1);
    Link3CharGuard guard = { message };
    Link3ErrorControl* owner = (Link3ErrorControl*)__nw__FUl(16);
    if (owner) {
        owner->strong = owner->weak = 1;
        owner->table = s_sharedCharDeleterVtable;
        owner->text = message;
    }
    control = owner;
    guard.data = 0;
    strcpy(message, text);
}
// Construct an owned length-error message and its derived exception table.
std::length_error::length_error(const char* text) {
    Link3MakeMessage(message, control, text);
    table = __vt__Q23std12length_error;
}
// Construct an owned bounds-error message and its derived exception table.
std::out_of_range::out_of_range(const char* text) {
    Link3MakeMessage(message, control, text);
    table = __vt__Q23std12out_of_range;
}

extern "C" {

// These guards are armed only while constructing the corresponding foreign
// object. After construction, the original function retains the handle even if
// a later operation throws.
struct Link3HandleConstruction {
    Link3FindHandle* handle;
    bool asynchronous;
    // Unwind a partially constructed handle using its selected allocator path.
    ~Link3HandleConstruction() {
        if (!handle) return;
        if (asynchronous) {
            FindHandleNames_dtor_8041C514(&handle->names, -1);
            FileIOString_dtor_8041F6F4(&handle->pattern, -1);
        } else {
            dtor_8041C294(&handle->names, -1);
            FileIOString_dtor(&handle->pattern, -1);
        }
        FileIO_Free(handle);
    }
};
struct Link3CallbackConstruction {
    Link3Callback* object;
    // Unwind a completion object until its path and base are fully constructed.
    ~Link3CallbackConstruction() {
        if (!object) return;
        FileIOString_dtor_8041F6F4(&object->path, -1);
        FileIOCallback_dtor(object, 0);
        FileIO_Free(object);
    }
};
struct Link3Name {
    Link3SmallString value;
    // Start with an empty inline directory-entry name.
    Link3Name() { value.flags = value.length = 0; value.data = 0; }
    // Release the temporary entry name's heap representation if present.
    ~Link3Name() { FileIOString_dtor(&value, -1); }
};
struct Link3ByteStorage {
    char* data;
    unsigned int size, capacity;
    unsigned int* owner;
    unsigned int offset;
    // Start a replacement buffer that borrows the owner's capacity word.
    Link3ByteStorage(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Release untransferred storage, or the old buffer after a successful swap.
    ~Link3ByteStorage() { if (data) { size = 0; FileIO_Free(data); } }
};
// Free a search handle's names and pattern through the synchronous or asynchronous allocator path.
static void Link3FreeHandle(Link3FindHandle* handle, bool asynchronous) {
    if (!handle) return;
    if (handle->names) {
        handle->bytes = 0;
        if (asynchronous) FileIO_InvokeAllocatorCallback3(handle->names);
        else FileIO_Free(handle->names);
    }
    if (handle->pattern.flags >> 31) {
        if (asynchronous) FileIO_InvokeAllocatorCallback3(handle->pattern.data);
        else FileIO_Free(handle->pattern.data);
    }
    FileIO_Free(handle);
}
// Choose geometric buffer growth, throwing on unsigned capacity overflow.
static unsigned int Link3Grow(unsigned int capacity, unsigned int needed) {
    if (needed > ~capacity) {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(filePathUtilityStrings + kFmemVectorLengthError);
        exception_runtime::__throw(exception_runtime::filePathUtilityStrings + kFmemLengthErrorType, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
    if (capacity < 0x55555555U) {
        unsigned int growth = (3 * (capacity + 1)) / 5;
        return capacity + (growth < needed ? needed : growth);
    }
    if (capacity < 0xaaaaaaaaU) {
        unsigned int growth = (capacity + 1) / 2;
        return capacity + (growth < needed ? needed : growth);
    }
    return 0xffffffffU;
}
// Resize the directory-name buffer, preserving its existing bytes and zeroing the extension.
static void Link3ResizeNames(Link3FindHandle* handle, unsigned int size) {
    // Phase 1: reuse capacity when possible; reject impossible growth before allocation.
    unsigned int oldSize = handle->bytes;
    if (size <= oldSize) { if (size < oldSize) handle->bytes = size; return; }
    unsigned int added = size - oldSize;
    if (added <= handle->capacity && oldSize <= handle->capacity - added) {
        memset((char*)handle->names + oldSize, 0, added);
        handle->bytes += added;
        return;
    }
    unsigned int needed = oldSize + added - handle->capacity;
    if (needed > ~handle->capacity) {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(filePathUtilityStrings + kFmemVectorLengthError);
        exception_runtime::__throw(exception_runtime::filePathUtilityStrings + kFmemLengthErrorType, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
    // Phase 2: construct the new suffix, move the prefix, and swap storage ownership.
    Link3ByteStorage storage(&handle->capacity);
    unsigned int capacity = Link3Grow(*storage.owner, needed);
    storage.data = (char*)FileIO_Alloc(capacity);
    storage.capacity = capacity;
    storage.offset = handle->bytes;
    memset(storage.data + storage.offset + storage.size, 0, added);
    char* old = (char*)handle->names;
    unsigned int bytes = handle->bytes;
    storage.size += added;
    storage.offset -= bytes;
    memcpy(storage.data + storage.offset, old, bytes);
    memset(old, 0, bytes);
    handle->bytes = 0;
    storage.size += bytes;
    unsigned int oldCapacity = handle->capacity;
    handle->capacity = storage.capacity;
    storage.capacity = oldCapacity;
    handle->names = storage.data;
    storage.data = old;
    handle->bytes = storage.size;
    storage.size = 0;
}

// Find the first matching NAND entry, or allocate and submit an asynchronous directory search.
// With callback == 0, result is required and filled before return. With a callback,
// result is unused: return means submission succeeded, not that a match was found.
// Completion receives separate transient output; context must outlive completion.
// A successful handle is owned by the caller/file layer; (void*)-1 means failure.
// Precondition: only one FMEM async operation may be outstanding across the device.
// s_fmemAsyncOperation and s_nandAsyncCommandBlock are shared with other operations;
// this function neither checks occupancy nor supports reentrant submission.
void* gtl_CFileIoDeviceFMEM_FindFirstFile(void*, const char* path, FmemFindData* result, FmemFindCompletion callback, void* context) {
    // Phase 1: split a device-prefixed path into directory and wildcard pattern.
    char directory[0x104];
    strcpy(directory, path);
    const char* firstSlash = strchr(directory, '/');
    char* lastSlash = strrchr(directory, '/');
    const char* search = firstSlash == lastSlash ? filePathUtilityStrings + kFmemPathSeparator : firstSlash;
    *lastSlash = 0;
    const char* pattern = lastSlash + 1;
    // Both string replace helpers ignore incoming r8 (allocator tag). The
    // original uninitialized tag bytes therefore have no observable effect.
    char allocatorTag = 0;
    bool asynchronous = callback != 0;
    Link3FindHandle* handle = (Link3FindHandle*)FileIO_Alloc(0x28);
    if (handle) {
        handle->vtable = (void**)(asynchronous ? __vt__Q23gtl16CFindHandleFMEMi : __vt__Q23gtl15CFindHandleFMEM);
        handle->pattern.flags = handle->pattern.length = 0;
        handle->pattern.data = 0;
        handle->names = 0;
        handle->bytes = handle->capacity = handle->offset = handle->count = handle->next = 0;
        Link3HandleConstruction construction = { handle, asynchronous };
        unsigned int n = strlen(pattern);
        if (asynchronous) FileIOString_ReplaceRange(&handle->pattern, 0, handle->pattern.size(), pattern, pattern+n, &allocatorTag);
        else FileIOString_ReplaceRangeWithAllocator(&handle->pattern, 0, handle->pattern.size(), pattern, pattern+n, &allocatorTag);
        construction.handle = 0;
    }
    if (!asynchronous) {
        // Phase 2a: count entries, reserve aligned storage, then find the first match.
        if (NANDReadDir(search, 0, &handle->count) != 0 || !handle->count) {
            File_SetLastError(kFileErrorNotFound);
            Link3FreeHandle(handle, false);
            return (void*)-1;
        }
        int bytes = (int)(handle->count * kNandDirectoryNameBytes);
        int remainder = bytes % kNandBufferAlignment;
        unsigned int aligned = (unsigned int)bytes + (remainder ? kNandBufferAlignment-remainder : 0);
        Link3ResizeNames(handle, aligned);
        NANDReadDir(search, handle->names, &handle->count);
        Link3Name name;
        bool found = false;
        for (unsigned int i = 0; i < handle->count; ++i) {
            if (handle->offset >= handle->bytes) {
                union { double alignment; unsigned char bytes[sizeof(std::out_of_range)]; } storage;
                std::out_of_range* error = (std::out_of_range*)storage.bytes;
                error->__ct(filePathUtilityStrings + kFmemVectorIndexOutOfRange);
                exception_runtime::__throw(exception_runtime::filePathUtilityStrings + kFmemOutOfRangeType, error,
                    (void*)exception_runtime::__dt__Q23std12out_of_rangeFv);
            }
            const char* entry = (char*)handle->names + handle->offset;
            unsigned int n = strlen(entry);
            FileIOString_ReplaceRangeWithAllocator(&name.value, 0, name.value.size(), entry, entry+n, &allocatorTag);
            handle->offset += name.value.size() + 1;
            if (((int (*)(void*, const char*))handle->vtable[2])(handle, name.value.text())) {
                handle->next = i + 1;
                found = true;
                break;
            }
        }
        if (!found) { Link3FreeHandle(handle, false); return (void*)-1; }
        memset(result, 0, sizeof(FmemFindData));
        strcpy(((FmemFindData*)result)->name, name.value.text());
        unsigned int count = 0;
        if (NANDReadDir(((FmemFindData*)result)->name, 0, &count) == 0)
            ((FmemFindData*)result)->attributes |= kFindDataDirectory;
    } else {
        // Phase 2b: construct the continuation; it retains copies of path and callback.
        Link3Callback* completion = (Link3Callback*)FileIO_Alloc(0x1c);
        if (completion) {
            completion->vtable = (void**)__vt__Q23gtl22CCallBackFindFirstFile;
            completion->handle = handle;
            completion->path.flags = completion->path.length = 0;
            completion->path.data = 0;
            completion->callback = callback;
            completion->context = context;
            Link3CallbackConstruction construction = { completion };
            unsigned int n = strlen(search);
            FileIOString_ReplaceRange(&completion->path, 0, completion->path.size(), search, search+n, &allocatorTag);
            construction.object = 0;
        }
        // Publish the sole pending operation before handing its context to NAND.
        // On accepted submission the dispatch/continuation chain owns these allocations.
        s_fmemAsyncOperation.callback = completion;
        s_fmemAsyncOperation.size = kFindFirstOperationBytes;
        s_fmemAsyncOperation.buffer = FileIO_Alloc(s_fmemAsyncOperation.size);
        NANDSetUserData(s_nandAsyncCommandBlock, &s_fmemAsyncOperation);
        int error = NANDReadDirAsync(search, 0, &handle->count, (void*)gtl_CFileIoDeviceFMEM_DispatchNandCallback, s_nandAsyncCommandBlock);
        int translated;
        if (error == 0) translated = 0;
        else if (error <= -1 && error >= -6) translated = kNandErrorBase-error;
        else if (error <= -8 && error >= -15) translated = kNandErrorBase-error;
        else if (error == -128) translated = kFileErrorNandFatal;
        else translated = kFileErrorUnknownNand;
        File_SetLastError(translated);
        if (error) {
            // Rejected submission transfers no ownership: retire staging, callback and handle.
            FileIO_Free(s_fmemAsyncOperation.buffer);
            s_fmemAsyncOperation.buffer = 0;
            ((void (*)(void*))completion->vtable[3])(completion);
            Link3FreeHandle(handle, true);
            return (void*)-1;
        }
    }
    File_SetLastError(0);
    return handle;
}

} // extern "C"
