// File-memory directory listing: continues a NAND directory listing after its first query. It resizes
// the name buffer for the entries found, then queues the next read with a completion callback.

extern "C" void* FileIOCallback_dtor(void*, int);

#include "classes/GtlFileIoDeviceFMEM.h"
#include "classes/gtl_CCallBackFindFirstFile.h"
#include "types.h"
#define char_array_guard CharArrayGuard_u8041E2C8
#include "game/stdexcept_local.h"
#undef char_array_guard
#include "sdk_protos.h"

extern "C" {
void* FileIO_Alloc(unsigned int n);
void FileIO_Free(void* p);
void* FileIO_InvokeAllocatorCallback2(unsigned int n);
void FileIO_InvokeAllocatorCallback3(void* p);
// This unit uses FileIO_InvokeAllocatorCallback2 for a fresh allocation and FileIO_InvokeAllocatorCallback3
// for disposal. Their declarations elsewhere conflict, so shared renames
// identify the callback slots without asserting a universal signature.
void FileIOString_ReplaceRange(void* errorBuffer, int a, int b, const char* c, const char* d, void* e);
void NANDSetUserData(void* a, void* b);
void NANDReadDirAsync(const char* a, void* b, void* c, void* d, void* e);
extern char filePathUtilityStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char __vt__Q23gtl17CCallBackFileList[];
void __dt__Q23std12length_errorFv(void*);
void* memset(void*, int, size_t);
void* memcpy(void*, const void*, size_t);
char* strcpy(char*, const char*);
size_t strlen(const char*);

// Directory names occupy 13-byte NAND slots; DMA buffers use 32-byte alignment.
enum DirectoryListConstants {
    kNandDirectoryNameBytes = 13,
    kNandBufferAlignment = 32,
    kInlinePathLengthMask = 0x7F
};
struct DirectoryListBuffer {
    char prefix[0x10];
    void* data;
    unsigned int size;
    unsigned int capacity;
    char reserved[4];
    unsigned int entryCount;
};
// directory borrows this container; it owns data through allocator callback slots
// 2 (allocate) and 3 (free). size/capacity are bytes, entryCount is NAND names.
// Resizing preserves old bytes and zeroes newly exposed bytes; callback objects
// do not take ownership of directory itself.
struct DirectoryListOperation {
    void* callback;
    void* owner;
    int resultBytes;
    char commandBlock[1]; // Opaque NAND command storage continues after this prefix.
};
struct DirectoryListCallback {
    void* vtable;
    void* directory;
    unsigned int pathFlags;
    unsigned int pathLength;
    void* heapPath;
    void* (*completion)(void*, void*, void*, void*);
    void* userArgument;
};
struct InlineDirectoryPath {
    unsigned char flags;
    char data[11];
};
typedef char DirectoryListCallbackSize[sizeof(DirectoryListCallback) == 0x1C ? 1 : -1];

} // extern "C"

#pragma use_lmw_stmw on
// Deliver a failed directory query, or allocate an aligned name buffer and queue the listing callback.
// error is the first query's status (null means success). On error, completion
// runs synchronously; its stack errorBuffer is transient and uninitialized here.
// operation and self are then destroyed/freed. On success, operation survives
// into NANDReadDirAsync with a newly allocated continuation; self is retired.
// Completion/path copies belong to the continuation; directory/userArgument stay borrowed.
void gtl::CCallBackFindFirstFile::ContinueListing(void* error, void* operation, int resultBytes) {
    // Phase 1: failure consumes the operation immediately; no async read is queued.
    if (error != 0) {
        char errorBuffer[0x140];
        (void)errorBuffer;
        this->completion(error, errorBuffer, this->directory, this->userArgument);
        if (operation != 0) {
            FileIO_InvokeAllocatorCallback3(operation);
        }
        ((void (*)(gtl::CCallBackFindFirstFile*, int))(*(void***)this)[2])(this, -1);
        FileIO_InvokeAllocatorCallback3(this);
        return;
    }

    void* directory = this->directory;
    unsigned int entryCount = ((DirectoryListBuffer*)directory)->entryCount;
    // Phase 2: grow/shrink the name buffer for entryCount 13-byte slots.
    // The rotate sequence emulates SIGNED remainder by 32 in unsigned words:
    // with bit 31 clear, remainder is nameBytes&31; with bit 31 set, a nonzero
    // remainder is (nameBytes&31)-32 modulo 2^32. Thus large counts do not follow
    // ordinary unsigned align-up behavior. Preserve both overflow and this edge.
    {
        unsigned int nameBytes = entryCount * kNandDirectoryNameBytes;
        unsigned int signBit = nameBytes >> 31;
        unsigned int shiftedBytes = nameBytes << 27;
        unsigned int adjustedBytes = shiftedBytes - signBit;
        unsigned int rotatedBytes = (adjustedBytes << 5) | (adjustedBytes >> 27);
        unsigned int alignmentRemainder = rotatedBytes + signBit;
        unsigned int alignedBytes;
        if (alignmentRemainder == 0) {
            alignedBytes = nameBytes;
        } else {
            alignedBytes = nameBytes + (kNandBufferAlignment - alignmentRemainder);
        }
        // size is the exposed range, capacity the allocation. Growth stages a
        // new allocation, copies/clears the old range, then publishes the header.
        unsigned int size = ((DirectoryListBuffer*)directory)->size;
        if (alignedBytes > size) {
            unsigned int capacity = ((DirectoryListBuffer*)directory)->capacity;
            unsigned int additionalBytes = alignedBytes - size;
            if (additionalBytes > capacity || size > (capacity - additionalBytes)) {
                unsigned int previousCapacity = ((DirectoryListBuffer*)directory)->capacity;
                if ((size + additionalBytes) - capacity > (unsigned int)(-1 - previousCapacity)) {
                    std::length_error(filePathUtilityStrings + 0xB, filePathUtilityStrings + 0x1F);
                }
                unsigned int capacityShortfall = size + additionalBytes - capacity;
                unsigned int newCapacity;
                if (previousCapacity < 0x55555555u) {
                    unsigned int growthBase = previousCapacity + 1;
                    unsigned int growth = (growthBase * 4 - growthBase) / 5;
                    if (growth < capacityShortfall)
                        growth = capacityShortfall;
                    newCapacity = previousCapacity + growth;
                } else if (previousCapacity < 0xAAAAAAAAu) { // 0xAAAAAAAA
                    unsigned int halfGrowth = (previousCapacity + 1) >> 1;
                    if (halfGrowth < capacityShortfall)
                        halfGrowth = capacityShortfall;
                    newCapacity = previousCapacity + halfGrowth;
                } else {
                    newCapacity = (unsigned int)-1;
                }
                void* newBuffer = FileIO_InvokeAllocatorCallback2(newCapacity);
                unsigned int previousSize = ((DirectoryListBuffer*)directory)->size;
                // Zero the added range before moving the previous names into the new buffer.
                memset((char*)newBuffer + previousSize, 0, additionalBytes);
                void* oldData = ((DirectoryListBuffer*)directory)->data;
                unsigned int oldSize = ((DirectoryListBuffer*)directory)->size;
                unsigned int bytesToCopy = oldData ? oldSize : 0;
                memcpy(newBuffer, oldData, bytesToCopy);
                if (oldData)
                    memset(oldData, 0, bytesToCopy);
                ((DirectoryListBuffer*)directory)->size = 0;
                unsigned int newSize = previousSize + additionalBytes;
                ((DirectoryListBuffer*)directory)->capacity = newCapacity;
                void* oldBuffer = ((DirectoryListBuffer*)directory)->data;
                ((DirectoryListBuffer*)directory)->data = newBuffer;
                ((DirectoryListBuffer*)directory)->size = newSize;
                if (oldBuffer)
                    FileIO_InvokeAllocatorCallback3(oldBuffer);
            } else {
                memset((char*)((DirectoryListBuffer*)directory)->data + ((DirectoryListBuffer*)directory)->size, 0,
                       additionalBytes);
                ((DirectoryListBuffer*)directory)->size += additionalBytes;
            }
        } else if (alignedBytes < size) {
            ((DirectoryListBuffer*)directory)->size = size - (size - alignedBytes);
        }
    }

    // Phase 3: borrow the existing path, then deep-copy it into a fresh listing
    // continuation before the current callback is destroyed. Bit 31 selects heap
    // bytes; otherwise the inline bytes begin one byte after pathFlags.
    unsigned int pathFlags = this->pathFlags;
    void* userArgument = this->userArgument;
    void* pathData;
    if ((pathFlags >> 31) == 0) {
        pathData = ((InlineDirectoryPath*)&this->pathFlags)->data;
    } else {
        pathData = this->heapPath;
    }
    void* directoryArgument = this->directory;
    void* nextCallback = FileIO_Alloc(sizeof(DirectoryListCallback));
    if (nextCallback != 0) {
        ((DirectoryListCallback*)nextCallback)->vtable = (void*)__vt__Q23gtl17CCallBackFileList;
        ((DirectoryListCallback*)nextCallback)->directory = directoryArgument;
        ((DirectoryListCallback*)nextCallback)->pathFlags = 0;
        ((DirectoryListCallback*)nextCallback)->pathLength = 0;
        ((DirectoryListCallback*)nextCallback)->heapPath = 0;
        ((DirectoryListCallback*)nextCallback)->completion = this->completion;
        ((DirectoryListCallback*)nextCallback)->userArgument = userArgument;
        unsigned int previousPathLength;
        // The new pathFlags is zero: the generic string decoder's inline branch
        // is always selected here. Retain its expression for matching codegen.
        if ((0 >> 31) == 0) {
            previousPathLength = ((unsigned char)*(unsigned char*)&((DirectoryListCallback*)nextCallback)->pathFlags) &
                                 kInlinePathLengthMask;
        } else {
            previousPathLength = ((DirectoryListCallback*)nextCallback)->pathLength;
        }
        size_t pathLength = strlen((const char*)pathData);
        char allocatorTag;
        try {
            FileIOString_ReplaceRange(&((DirectoryListCallback*)nextCallback)->pathFlags, 0, previousPathLength,
                                      (const char*)pathData, (const char*)pathData + pathLength, &allocatorTag);
        } catch (...) {
            FileIOCallback_dtor(nextCallback, -1);
            FileIO_InvokeAllocatorCallback3(nextCallback);
            throw;
        }
    }
    // Phase 4: transfer the continuation to the operation, associate the NAND
    // command block with that operation, and submit the second directory query.
    // Success requires a live operation; this routine adds no allocation checks.
    ((DirectoryListOperation*)operation)->callback = nextCallback;
    void* commandBlock = ((DirectoryListOperation*)operation)->commandBlock;
    ((DirectoryListOperation*)operation)->owner = operation;
    ((DirectoryListOperation*)operation)->resultBytes = resultBytes;
    NANDSetUserData(commandBlock, operation);
    void* directoryForRead = this->directory;
    void* path;
    if ((pathFlags >> 31) == 0) {
        path = ((InlineDirectoryPath*)&this->pathFlags)->data;
    } else {
        path = this->heapPath;
    }
    NANDReadDirAsync((const char*)path, ((DirectoryListBuffer*)directoryForRead)->data,
                     &((DirectoryListBuffer*)directoryForRead)->entryCount,
                     (void*)gtl::CFileIoDeviceFMEM::DispatchNandCallback, commandBlock);
    // Virtual destructor slot 2 receives -1 (nondeleting cleanup), followed by
    // the allocator's explicit free. Async completion uses nextCallback, not self.
    ((void (*)(gtl::CCallBackFindFirstFile*, int))(*(void***)this)[2])(this, -1);
    FileIO_InvokeAllocatorCallback3(this);
}
#pragma use_lmw_stmw reset
