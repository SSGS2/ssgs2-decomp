#include <decomp/static_init.h>
#include <decomp/cw_string_guard.h>
#include "classes/MWStringStorage.h"
#include "classes/FmemOperationCallback.h"
#include "classes/FmemFileState.h"
// String storage is 12 bytes: heap = {flag/capacity, length, pointer}; inline =
// {high-bit tag + 7-bit length, 11 bytes including the terminator}. Capacity
// counts allocated bytes, so logical length must remain strictly below capacity.
// The views overlap: copy characters before publishing a new representation.
struct FileIOInlineString { unsigned char taggedLength; char data[11]; };
struct FileIOVectorStorage { void* data; unsigned int count; unsigned int capacity; };
struct PadMemoryHandle { void* vtable; int word1; int word2; };
static const unsigned int kInlineStringCapacity = 11;
static const unsigned int kMaximumStringLength = 0x7ffffffeU;
static const unsigned int kStringCapacityMask = 0x7fffffffU;
static const unsigned int kHeapStringFlag = 0x80000000U;
static const unsigned int kInlineLengthMask = 0x7f;
static const unsigned int kStringAllocationAlignment = 16;
static const unsigned int kPathBufferBytes = 0x104;
static const unsigned int kPadMemoryNameLimit = 0x20;
static const int kFileNameTooLong = 0x6f;
static const int kFileOpenSucceeded = 1;
static const int kFileOpenFailed = 2;
static const unsigned int kLengthErrorTypeOffset = 0x1f;
static const unsigned int kOutOfRangeTypeOffset = 0x74;
static const unsigned int kReserveLengthErrorOffset = 0xab;
static const unsigned int kStringRangeErrorOffset = 0xce;
static const unsigned int kStringLengthErrorOffset = 0xe9;

#pragma push
#pragma opt_propagation on
#pragma opt_common_subs on
#pragma opt_lifetimes on
#include "game/exception.h"
extern "C" {
unsigned long strlen(const char*) __attribute__((nothrow));
char* strcpy(char*, const char*) __attribute__((nothrow));
void __dla__FPv(void*) __attribute__((nothrow));
void __throw(const char*, void*, void*) __attribute__((noreturn, nothrow));
}
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std11logic_error[];
extern "C" int __vt__Q23std12length_error[];
namespace std {
namespace detail {
struct default_delete_chars {
    char unused;
    // Initialize the empty array-deleter policy.
    default_delete_chars() : unused(0) {}
    // Construct the policy without initializing its compiler-visible scratch byte.
    default_delete_chars(int) {}
    // Release the character array owned by an exception message.
    void operator()(char* p) const { delete[] p; }
};
struct CharArrayGuard_u8041F634 {
    char* ptr;
    default_delete_chars& deleter;
    // Borrow the deleter and guard p until ownership reaches the control block.
    CharArrayGuard_u8041F634(char* p, default_delete_chars& d) : ptr(p), deleter(d) {}
    // Free the buffer if control-block construction did not take ownership.
    ~CharArrayGuard_u8041F634() { if (ptr) deleter(ptr); }
    // Disarm cleanup after successful ownership transfer.
    void release() { ptr = 0; }
};
struct message_control {
    const void* table;
    int useCount;
    int weakCount;
    char* ptr;
    // Take ownership of the message array with one strong and one weak reference.
    message_control(char* p) : useCount(1), weakCount(1) { table = s_sharedCharDeleterVtable; ptr = p; }
};
}
class logic_error : public exception {
public:
#pragma push
#pragma opt_propagation off
    // Copy a terminated message and attach shared ownership for exception copies.
    logic_error(const char* text) {
        detail::default_delete_chars deleter;
        unsigned int length = strlen(text) + 1;
        message = static_cast<char*>(operator new[]((length << 1) - length));
        {
            char* buffer = message;
            detail::CharArrayGuard_u8041F634 guard(buffer, deleter);
            control = new detail::message_control(buffer);
            guard.release();
        }
        strcpy(message, text);
    }
#pragma pop
    // Copy the diagnostic, then publish this exception's address to the throw adapter.
    // Address references use the Wii's 32-bit integer ABI; they are never lengths.
    logic_error(const char* text, unsigned long& publishedErrorAddress, unsigned long& messageBufferAddress, unsigned int& diagnosticAddress) {
        detail::default_delete_chars deleter(0);
        volatile detail::default_delete_chars original;
        deleter.unused = 0;
        unsigned int length = strlen((const char*)diagnosticAddress) + 1;
        message = static_cast<char*>(operator new[]((length << 1) - length));
        {
            messageBufferAddress = (unsigned int)message;
            detail::CharArrayGuard_u8041F634 guard((char*)messageBufferAddress, deleter);
            control = new detail::message_control((char*)messageBufferAddress);
            guard.release();
        }
        strcpy(message, (const char*)diagnosticAddress);
        publishedErrorAddress = (unsigned long)this;
    }
    // Copy diagnosticAddress's text into owned storage and publish this exception.
    // diagnosticAddress/publishedErrorAddress carry address bits, not string lengths.
    logic_error(unsigned int& diagnosticAddress, unsigned int& publishedErrorAddress, detail::default_delete_chars& deleter, char*& messageBufferAddress) {
        unsigned int length = strlen((const char*)diagnosticAddress) + 1;
        message = static_cast<char*>(operator new[]((length << 1) - length));
        {
            messageBufferAddress = message;
            detail::CharArrayGuard_u8041F634 guard(messageBufferAddress, deleter);
            control = new detail::message_control(messageBufferAddress);
            guard.release();
        }
        strcpy(message, (const char*)diagnosticAddress);
        publishedErrorAddress = (unsigned int)this;
    }
    virtual ~logic_error();
private:
    char* message;
    detail::message_control* control;
};
extern "C" void __dt__Q23std12out_of_rangeFv(void*);
extern "C" void __dt__Q23std12length_errorFv(void*);
class out_of_range : public logic_error {
public:
    // Construct and throw a range error through the caller's scratch slots.
    out_of_range(unsigned int& diagnosticAddress, unsigned int& publishedErrorAddress, const char* typeName, detail::default_delete_chars& deleter, char*& messageBufferAddress) : logic_error(diagnosticAddress, publishedErrorAddress, deleter, messageBufferAddress) {
        __throw(typeName, (void*)publishedErrorAddress, (void*)__dt__Q23std12out_of_rangeFv);
    }
    // Construct and throw a range error with a copied message.
    out_of_range(const char* text, const char* typeName) : logic_error(text) {
        __throw(typeName, this, (void*)__dt__Q23std12out_of_rangeFv);
    }
    virtual ~out_of_range();
};
class length_error : public logic_error {
public:
    // Construct and throw a length error through the caller's scratch slots.
    length_error(unsigned int& diagnosticAddress, unsigned int& publishedErrorAddress, const char* typeName, detail::default_delete_chars& deleter, char*& messageBufferAddress) : logic_error(diagnosticAddress, publishedErrorAddress, deleter, messageBufferAddress) {
        __throw(typeName, (void*)publishedErrorAddress, (void*)__dt__Q23std12length_errorFv);
    }
    // Construct and throw a length error with a copied message.
    length_error(const char* text, const char* typeName) : logic_error(text) {
        __throw(typeName, this, (void*)__dt__Q23std12length_errorFv);
    }
    // Throw a length error after publishing its address to the throw adapter.
    length_error(const char* text, const char* typeName, unsigned long& publishedErrorAddress, unsigned long& messageBufferAddress, unsigned int& diagnosticAddress) : logic_error(text, publishedErrorAddress, messageBufferAddress, diagnosticAddress) {
        __throw(typeName, (void*)publishedErrorAddress, (void*)__dt__Q23std12length_errorFv);
    }
    virtual ~length_error();
};
}
#pragma pop
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>


extern "C" void FileIO_Free(void* p);
extern "C" void* FmemAsync_OperationStateDtor(void* p, int n);
extern "C" void* FileIO_InvokeAllocatorCallback3(void* p);
extern "C" void* FileIO_Alloc(size_t n);
extern "C" void File_SetLastError(int x);
extern "C" void fn_803CB6FC(const char* s);
extern "C" void __dl__FPv(void* p);
extern "C" void* FileIO_InvokeAllocatorCallback2(size_t n);
extern "C" void* memcpy(void* dst, const void* src, size_t n);
extern "C" char filePathUtilityStrings[];
extern "C" char s_fileIOHeapReplaceTerminator;
extern "C" char s_fileIOInlineReplaceTerminator;

extern "C" int s_fmemOperationDtorRegistration[64];
extern "C" int s_fmemAsyncOperation[3];
extern "C" int __vt__Q23gtl13CHandlePADMEM[4];
extern "C" int __vt__Q23std12length_error[];
extern "C" void __dt__Q23std12length_errorFv(void*);

extern "C" {

// Initialize and register the shared asynchronous-operation storage.
void* FmemAsync_InitOperationState() {
    s_fmemAsyncOperation[0] = 0;
    s_fmemAsyncOperation[1] = 0;
    s_fmemAsyncOperation[2] = 0;
    return __register_global_object(s_fmemAsyncOperation, (void*)FmemAsync_OperationStateDtor, &s_fmemOperationDtorRegistration);
}

// Optionally free the operation-state object; return its original address.
void* FmemAsync_OperationStateDtor(void* p, int n) {
    if (p != 0 && n > 0) {
        FileIO_Free(p);
    }
    return p;
}

// Release a heap string buffer and optionally delete its owner.
void* FileIOString_dtor_8041F6F4(void* p, int n) {
    if (p != 0) {
        if (AsMWStringStorage(p)->flags >> 31) {
            FileIO_InvokeAllocatorCallback3(*(void**)&AsMWStringStorage(p)->data);
        }
        if (n > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// Clear and release vector storage with the FileIO_InvokeAllocatorCallback3 release routine.
void* dtor_8041FD78(void* p, int n) {
    if (p != 0) {
        ((FileIOVectorStorage*)p)->count = 0;
        if (p != 0) {
            if (((FileIOVectorStorage*)p)->data != 0) {
                ((FileIOVectorStorage*)p)->count = 0;
                FileIO_InvokeAllocatorCallback3(((FileIOVectorStorage*)p)->data);
            }
        }
        if (n > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// Clear and release vector storage with the FileIO_Free release routine.
void* dtor_8041FDEC(void* p, int n) {
    if (p != 0) {
        ((FileIOVectorStorage*)p)->count = 0;
        if (p != 0) {
            if (((FileIOVectorStorage*)p)->data != 0) {
                ((FileIOVectorStorage*)p)->count = 0;
                FileIO_Free(((FileIOVectorStorage*)p)->data);
            }
        }
        if (n > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// Create a PADMEM handle and submit the filename following the first slash.
// Requires a terminated path shorter than kPathBufferBytes and containing '/'.
// The suffix may contain at most 32 characters. On that error, return -1 and set
// last-error to 0x6f; the already allocated handle is not released on this path.
// Allocation failure is not checked before submission; successful submission can
// therefore return null. The callee must consume/copy the stack-backed suffix.
void* gtl_CFileIoDevicePADMEM_OpenHandle(void* self, const char* path) {
    char buf[kPathBufferBytes];
    strncpy(buf, path, kPathBufferBytes);
    char* slash = strchr(buf, '/');
    void* h = FileIO_Alloc(sizeof(PadMemoryHandle));
    if (h != 0) {
        ((PadMemoryHandle*)h)->vtable = __vt__Q23gtl13CHandlePADMEM;
        ((PadMemoryHandle*)h)->word1 = 0;
        ((PadMemoryHandle*)h)->word2 = 0;
    }
    size_t len = strlen(slash + 1);
    if (len > kPadMemoryNameLimit) {
        File_SetLastError(kFileNameTooLong);
        return (void*)-1;
    } else {
        fn_803CB6FC(slash + 1);
        File_SetLastError(0);
        return h;
    }
}

// Publish open success/failure, invoke the user callback, and release callback storage.
// resultCode == 0 means success. self and non-null completionData are consumed;
// fileState is borrowed and survives this callback. In this callback variant,
// callbackPayload holds a FUNCTION address; callbackArgument in fileState is
// the user context (file +0x94).
// Invocation is synchronous and precedes all releases: the user callback must
// leave self/completionData alive for the cleanup below. Vtable slot 2 destroys
// self with -1 (without freeing storage); allocator callback 3 frees it afterward.
void* gtl_CCallBackOpenFile_OnOpenComplete(void* self, int resultCode, void* completionData) {
    int openResult = resultCode;
    void* completionStorage = completionData;
    if (resultCode == 0) {
        File_SetLastError(0);
        void* fileState = *(void**)&AsFmemOperationCallback(self)->argument;
        AsFmemFileState(fileState)->operationState = kFileOpenSucceeded;
    } else {
        void* fileState = *(void**)&AsFmemOperationCallback(self)->argument;
        AsFmemFileState(fileState)->operationState = kFileOpenFailed;
    }
    if (*(void**)&AsFmemOperationCallback(self)->callbackPayload != 0) {
        void* fileState = *(void**)&AsFmemOperationCallback(self)->argument;
        int userContext = AsFmemFileState(fileState)->callbackArgument;
        ((void(*)(int, int, void*, int))*(void**)&AsFmemOperationCallback(self)->callbackPayload)(openResult, 1, fileState, userContext);
    }
    if (completionStorage != 0) {
        FileIO_InvokeAllocatorCallback3(completionStorage);
    }
    void** vt = *(void***)self;
    ((void(*)(void*, int))vt[2])(self, -1);
    return FileIO_InvokeAllocatorCallback3(self);
}

#pragma push
#pragma use_lmw_stmw on
extern "C" void* __nwa__FUl(unsigned long);
extern "C" void* __nw__FUl(unsigned long);
extern "C" void __dla__FPv(void*);

#pragma push
#pragma use_lmw_stmw on
 #pragma opt_propagation on
 #pragma opt_common_subs on
 #pragma opt_lifetimes off
// Resize string capacity, preserving content and switching between inline and heap storage.
void FileIOString_Reserve(void* selfArg, size_t requestedCapacity) {
    struct Bits { CW_BITS2( unsigned int heap:1, unsigned int cap:31) };
    struct ByteBits { CW_BITS2( unsigned char heap:1, unsigned char len:7) };
    size_t newCapacity;
    register void* self = selfArg;
    // One compiler scratch slot spans two non-overlapping phases: a character
    // capacity request before allocation, a destination address afterward.
    register size_t capacityOrBufferAddress = requestedCapacity;
    register size_t heap;
    register unsigned int currentLength;
    char* previousData;
    unsigned int usesHeapStorage;
    // Validate and choose capacity without truncating the current string.
    if (capacityOrBufferAddress > kMaximumStringLength) {
        currentLength = (size_t)(filePathUtilityStrings + kReserveLengthErrorOffset);
        std::length_error error((const char*)currentLength, filePathUtilityStrings + kLengthErrorTypeOffset, newCapacity, heap, currentLength);
    }
    unsigned int storageFlags = AsMWStringStorage(self)->flags;
    heap = storageFlags >> 31;
    size_t currentCapacity;
    if (!heap) {
        currentLength = ((FileIOInlineString*)self)->taggedLength & kInlineLengthMask;
        currentCapacity = kInlineStringCapacity;
    } else {
        currentLength = AsMWStringStorage(self)->length;
        currentCapacity = storageFlags & kStringCapacityMask;
    }
    if (capacityOrBufferAddress < currentLength) {
        capacityOrBufferAddress = currentLength;
    }
    newCapacity = kInlineStringCapacity;
    if (capacityOrBufferAddress + 1 > kInlineStringCapacity) {
        newCapacity = (capacityOrBufferAddress + kStringAllocationAlignment) & ~((size_t)0xF);
    }
    if (newCapacity != currentCapacity) {
        // Select inline bytes or allocate a heap destination. From this point,
        // capacityOrBufferAddress is an address, never a character count.
        // Shrink allocation failure leaves the original string intact; growth throws.
        if (newCapacity == kInlineStringCapacity) {
            previousData = AsMWStringStorage(self)->data;
            capacityOrBufferAddress = (size_t)(((FileIOInlineString*)self)->data);
            usesHeapStorage = 0;
        } else {
            if (newCapacity > currentCapacity) {
                capacityOrBufferAddress = (size_t)FileIO_InvokeAllocatorCallback2(newCapacity);
            } else {
                try {
                    capacityOrBufferAddress = (size_t)FileIO_InvokeAllocatorCallback2(newCapacity);
                } catch (...) {
                    return;
                }
            }
            usesHeapStorage = 1;
            if (heap) {
                previousData = AsMWStringStorage(self)->data;
            } else {
                previousData = ((FileIOInlineString*)self)->data;
            }
        }
        // Preserve bytes and terminator before releasing the old heap allocation.
        memmove((char*)capacityOrBufferAddress, previousData, currentLength);
        ((char*)capacityOrBufferAddress)[currentLength] = 0;
        if (heap) {
            FileIO_InvokeAllocatorCallback3(previousData);
        }
        // Commit the overlapping tag/length/capacity fields only after the copy.
        ((Bits*)self)->heap = usesHeapStorage;
        if (!usesHeapStorage) {
            ((ByteBits*)self)->len = currentLength;
        } else {
            *(void**)&AsMWStringStorage(self)->data = (void*)capacityOrBufferAddress;
            AsMWStringStorage(self)->length = (unsigned int)currentLength;
            ((Bits*)self)->cap = newCapacity;
        }
    }
}
#pragma pop

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes on
struct StringAllocator { char unused; };
// Replace a string slice, preserving aliased input through a temporary when necessary.
// Replace min(count, length-pos) characters with [srcFirst, srcLast); return self.
// Source is borrowed and may alias this string. Invalid pos/length throws; the
// allocator owns replacement storage, while the local string owns any alias copy.
void* FileIOString_ReplaceRange(void* self, size_t pos, size_t count, const void* srcFirst, const void* srcLast, const StringAllocator& allocator) {
    struct Bits { CW_BITS2( unsigned int heap:1, unsigned int cap:31) };
    struct ByteBits { CW_BITS2( unsigned char heap:1, unsigned char len:7) };
    int isHeap;
    register unsigned int suffixLength;
    register unsigned int newLength;
    register char* characters;
    register size_t currentLength;
    register size_t capacityBytes;
    register size_t erasedLength;
    register size_t sourceLength;
    register char* replacementBuffer;
    size_t availableLength;
    unsigned int storageFlags = AsMWStringStorage(self)->flags;
    isHeap = (int)(storageFlags >> 31);
    if (!isHeap) {
        characters = ((FileIOInlineString*)self)->data;
        capacityBytes = kInlineStringCapacity;
        currentLength = ((FileIOInlineString*)self)->taggedLength & kInlineLengthMask;
    } else {
        characters = AsMWStringStorage(self)->data;
        capacityBytes = storageFlags & kStringCapacityMask;
        currentLength = AsMWStringStorage(self)->length;
    }
    // Check the slice, clamp erase length, and compute the final size and suffix.
    if (pos > currentLength) {
        suffixLength = (unsigned int)(filePathUtilityStrings + kStringRangeErrorOffset);
        std::detail::default_delete_chars deleter;
        std::out_of_range error(suffixLength, newLength, filePathUtilityStrings + kOutOfRangeTypeOffset, deleter, replacementBuffer);
    }
    availableLength = currentLength - pos;
    const size_t& clampedEraseLength = availableLength < count ? availableLength : count;
    sourceLength = (char*)srcLast - (char*)srcFirst;
    erasedLength = clampedEraseLength;
    if (sourceLength > kMaximumStringLength || currentLength - erasedLength > kMaximumStringLength - sourceLength) {
        newLength = (unsigned int)(filePathUtilityStrings + kStringLengthErrorOffset);
        std::detail::default_delete_chars deleter;
        std::length_error error(newLength, suffixLength, filePathUtilityStrings + kLengthErrorTypeOffset, deleter, replacementBuffer);
    }
    newLength = sourceLength + (currentLength - erasedLength);
    suffixLength = currentLength - (pos + erasedLength);
    if (newLength >= capacityBytes) {
        // Build prefix / insertion / suffix in new storage, then replace ownership.
        capacityBytes = (capacityBytes + 15) & ~((size_t)0xF);
        while (capacityBytes < newLength + 1) {
            capacityBytes = ((capacityBytes << 1) + 15) & ~((size_t)0xF);
        }
        replacementBuffer = (char*)FileIO_InvokeAllocatorCallback2(capacityBytes);
        if (pos != 0) {
            memcpy(replacementBuffer, characters, pos);
        }
        char* insertPtr = replacementBuffer + pos;
        memmove(insertPtr, srcFirst, (char*)srcLast - (char*)srcFirst);
        if (suffixLength != 0) {
            memcpy(insertPtr + sourceLength, characters + pos + erasedLength, suffixLength);
        }
        replacementBuffer[newLength] = s_fileIOHeapReplaceTerminator;
        if (isHeap) {
            FileIO_InvokeAllocatorCallback3(characters);
        } else {
            AsMWStringStorage(self)->flags |= kHeapStringFlag;
        }
        *(void**)&AsMWStringStorage(self)->data = replacementBuffer;
        AsMWStringStorage(self)->length = (unsigned int)newLength;
        ((Bits*)self)->cap = capacityBytes;
    } else {
        // Reuse storage; save an overlapping source before shifting the suffix.
        const void* srcData = srcFirst;
        // Release only an owned heap alias-copy when this scope exits.
        struct LocalStr { unsigned int storageFlags; unsigned int len; void* data; ~LocalStr() { if (storageFlags >> 31) FileIO_InvokeAllocatorCallback3(data); } } local = {0, 0, 0};
        if (suffixLength != 0) {
            char* suffixDestination = characters + pos + sourceLength;
            const void* srcEnd = srcLast;
            if (!(suffixDestination >= srcEnd) && !(srcEnd > characters + currentLength)) {
                size_t aliasCopyLength = !(local.storageFlags >> 31) ? (*(unsigned char*)&local & kInlineLengthMask) : local.len;
                StringAllocator copy;
                volatile StringAllocator original;
                copy.unused = original.unused;
                FileIOString_ReplaceRange(&local, 0, aliasCopyLength, srcFirst, srcLast, copy);
                if (local.storageFlags >> 31) {
                    srcData = local.data;
                } else {
                    srcData = (char*)&local + 1;
                }
            }
        }
        if (suffixLength != 0) {
            char* insertionPoint = characters + pos;
            memmove(insertionPoint + sourceLength, insertionPoint + erasedLength, suffixLength);
        }
        memmove(characters + pos, srcData, sourceLength);
        characters[newLength] = s_fileIOInlineReplaceTerminator;
        if (!isHeap) {
            ((ByteBits*)self)->len = newLength;
        } else {
            AsMWStringStorage(self)->length = (unsigned int)newLength;
        }
        (void)local;
    }
    return self;
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046271C", _ctors_u8041F634_0, (void (*)())FmemAsync_InitOperationState);
#pragma pop
