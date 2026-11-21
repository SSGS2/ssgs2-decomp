#include "classes/ArchiveStream.h"
// Translation unit 0x802D36D4-0x802D4A1C (5 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern "C" unsigned int strlen(const char *s);
extern "C" void* malloc(unsigned int);
extern "C" void free(void*);
extern "C" int ArchiveContext_CheckLocalFileHeader(void*, unsigned int*, unsigned int*, unsigned int*);
extern "C" int Zlib_InitRawInflate(void*);
extern "C" int ArchiveReader_CreateInputContext(void*, int, int, void*);
extern "C" int ZipStream_OpenArchiveContext(void*);
extern "C" int ArchiveContext_CloseCurrentEntry(void*);
extern "C" int Zlib_Crc32(unsigned int, void*, unsigned int);
extern "C" int Zlib_Inflate(void*, int);
extern "C" int ZipStream_ReadU32LE(void*, unsigned int*);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" unsigned int crc32TableAndUnzipVersion[];
extern "C" int __cntlzw(unsigned int);

enum ZipReadConstants {
    kZipInvalidParameter = -0x66, kZipBadArchive = -0x67,
    kZipOutOfMemory = -0x68, kZipCrcMismatch = -0x69,
    kZipBadPassword = -0x6a, kZipInvalidReader = -0x64,
    kZipInputBufferBytes = 0x4000, kZipLocalHeaderBytes = 0x1e,
    kZipEncryptedFlag = 1, kZipDataDescriptorFlag = 8,
    kZipStored = 0, kZipDeflated = 8, kInflateSyncFlush = 2,
    kInflateStreamEnd = 1, kZipEncryptionHeaderBytes = 12
};

// Directory metadata and the currently opened entry; sizes are bytes.
static const unsigned int kZipLocalHeaderSignature = 0x04034b50;

struct ZipArchiveContext {
    unsigned int archiveStream;
    char pad_04[8];
    unsigned int archivePrefixBytes;
    char pad_10[8];
    unsigned int currentFileValid;
    char pad_1c[0x14];
    unsigned int zipFlags;
    unsigned int compressionMethod;
    unsigned int dosTime;
    unsigned int expectedCrc32;
    unsigned int compressedSize;
    unsigned int uncompressedSize;
    unsigned int fileNameLength;
    char pad_4c[0x2c];
    unsigned int localHeaderOffset;
    void* currentEntry;
};

struct ZipEntryReader {
    unsigned int inputBuffer;
    unsigned int nextInput;
    unsigned int availableInput;
    unsigned int totalInput;
    unsigned int nextOutput;
    unsigned int availableOutput;
    unsigned int totalOutput;
    char pad_1c[8];
    unsigned int allocateCallback;
    unsigned int freeCallback;
    unsigned int allocatorContext;
    unsigned int dataType;
    unsigned int adler32;
    char pad_38[4];
    unsigned int compressedDataOffset;
    unsigned int inflateInitialized;
    unsigned int localExtraFieldOffset;
    unsigned int localExtraFieldSize;
    unsigned int localExtraFieldPosition;
    unsigned int runningCrc32;
    unsigned int expectedCrc32;
    unsigned int compressedBytesRemaining;
    unsigned int outputBytesRemaining;
    unsigned int archiveStream;
    unsigned int compressionMethod;
    unsigned int archivePrefixBytes;
    unsigned char encrypted;
    char pad_6d[3];
    unsigned int cryptoKey0;
    unsigned int cryptoKey1;
    unsigned int cryptoKey2;
    unsigned int encryptionHeaderRemaining;
    unsigned char passwordCheckByte;
};

// Open an entry, allocate its input buffer, and seed optional ZipCrypto password keys.
int ArchiveContext_OpenCurrentEntryWithPassword(ZipArchiveContext* archive, const signed char* password) {
    if (archive == 0) return kZipInvalidParameter;
    if (archive->currentFileValid == 0) return kZipInvalidParameter;
    if (archive->currentEntry != 0) ArchiveContext_CloseCurrentEntry(archive);
    unsigned int localHeaderVariableBytes;
    unsigned int extraFieldOffset;
    unsigned int extraFieldBytes;
    if (ArchiveContext_CheckLocalFileHeader(archive, &localHeaderVariableBytes, &extraFieldOffset, &extraFieldBytes) != 0) return kZipBadArchive;
    ZipEntryReader* entry = (ZipEntryReader*)malloc(0x84);
    if (entry == 0) return kZipOutOfMemory;
    entry->inputBuffer = (unsigned int)malloc(kZipInputBufferBytes);
    entry->localExtraFieldOffset = extraFieldOffset;
    entry->localExtraFieldSize = extraFieldBytes;
    entry->localExtraFieldPosition = 0;
    if (entry->inputBuffer == 0) {
        if (entry != 0) free(entry);
        return kZipOutOfMemory;
    }
    entry->inflateInitialized = 0;
    unsigned int methodLeadingZeros = (unsigned int)__cntlzw(archive->compressionMethod);
    entry->expectedCrc32 = archive->expectedCrc32;
    entry->runningCrc32 = 0;
    unsigned int storedMethod = methodLeadingZeros >> 5;
    entry->compressionMethod = archive->compressionMethod;
    entry->archiveStream = archive->archiveStream;
    entry->archivePrefixBytes = archive->archivePrefixBytes;
    entry->totalOutput = 0;
    // 0x802D3EF0: initialize inflate state only for compressed entries.
    if (!storedMethod) {
        entry->allocateCallback = 0;
        entry->freeCallback = 0;
        entry->allocatorContext = 0;
        if (Zlib_InitRawInflate(&entry->nextInput) == 0) entry->inflateInitialized = 1;
    }
    entry->compressedBytesRemaining = archive->compressedSize;
    entry->outputBytesRemaining = archive->uncompressedSize;
    entry->encrypted = archive->zipFlags & kZipEncryptedFlag;
    if (archive->zipFlags & kZipDataDescriptorFlag) {
        entry->passwordCheckByte = (unsigned char)((archive->dosTime >> 8) & 0xff);
    } else {
        entry->passwordCheckByte = (unsigned char)(archive->expectedCrc32 >> 24);
    }
    entry->encryptionHeaderRemaining = entry->encrypted ? kZipEncryptionHeaderBytes : 0;
    entry->cryptoKey0 = 0x12345678;
    entry->cryptoKey1 = 0x23456789;
    entry->cryptoKey2 = 0x34567890;
    while (password != 0 && *password != 0) {
        signed char passwordByte = *password;
        password++;
        unsigned int key0Index = entry->cryptoKey0 ^ passwordByte;
        entry->cryptoKey0 = (entry->cryptoKey0 >> 8) ^ crc32TableAndUnzipVersion[(key0Index & 0xff)];
        entry->cryptoKey1 = (entry->cryptoKey1 + (entry->cryptoKey0 & 0xff)) * 0x08088405u + 1;
        unsigned int key2Index = entry->cryptoKey2 ^ (entry->cryptoKey1 >> 24);
        entry->cryptoKey2 = (entry->cryptoKey2 >> 8) ^ crc32TableAndUnzipVersion[key2Index & 0xff];
    }
    entry->compressedDataOffset = localHeaderVariableBytes + archive->localHeaderOffset + kZipLocalHeaderBytes;
    entry->availableInput = 0; // 0x802D3EF0: existing ZipEntryReader layout.
    archive->currentEntry = entry;
    return 0;
}
struct ArchiveReaderHandle {
    int archiveHandle;
    int sourceHandle;
    char pad_008[0x138];
    char currentDirectory[2]; // Only the initial separator and terminator are written here.
};

struct InflateBlockState {
    int mode;
    void* codes;
    char pad_08[4];
    void* codeLengths;
    char pad_10[4];
    void* decodeTable;
    char pad_18[4];
    unsigned int bitCount;
    unsigned int bitBuffer;
    void* huffmanWorkspace;
    void* window;
    char pad_2c[4];
    void* readCursor;
    void* writeCursor;
    int (*calculateChecksum)(int, int, int);
    unsigned int checksum;
};

struct InflateInternalState {
    char pad_000[0x14];
    InflateBlockState* node;
};

struct ZipEntryCleanupView {
    void* inputBuffer;
    char pad_04[0x1c];
    InflateInternalState* inflateState;
    char pad_24[4];
    void (*freeCallback)(void*, void*);
    void* allocatorContext;
    char pad_30[4];
    unsigned int adler32;
    char pad_38[8];
    int inflateInitialized;
    char pad_44[0xc];
    unsigned int runningCrc32;
    unsigned int expectedCrc32;
    char pad_58[4];
    int outputBytesRemaining;
};

#pragma use_lmw_stmw on
// Reader-status codes returned by ArchiveContext_InitializeReader (0 = success).
// The 0x01000000 and 0x200 values are returned as-is to callers; their exact origin
// (already-open reader / failed to open the archive context) is inferred from the control flow.
enum ArchiveReaderInitStatus {
    kReaderInitAlreadyOpen = 0x01000000, // reader already has an archive handle, or its source handle is not -1
    kReaderInitOpenFailed = 0x200        // ZipStream_OpenArchiveContext returned no handle
};

// currentDirectory sits at offset 0x140 in ArchiveReaderHandle (8 + 0x138), so for a string of
// length n, reader + n + 0x13f is &currentDirectory[n - 1] (last character) and +0x140 / +0x141
// are the slots for an appended separator and terminator. The original code addresses it this way.
// Create the backing archive reader after normalizing the initial directory separator.
// reader: handle block; the archive/source handles must still be unset (0 / -1).
// inputArg, sizeArg, flagsArg: forwarded unchanged to ArchiveReader_CreateInputContext.
// Returns 0 on success, kReaderInit* on failure, or the context creator's own value if it failed.
int ArchiveContext_InitializeReader(ArchiveReaderHandle* reader, void* inputArg, int sizeArg, int flagsArg) {
    if (reader->archiveHandle != 0 || reader->sourceHandle != -1) {
        return kReaderInitAlreadyOpen;
    }
    // Start the current directory as "/".
    reader->currentDirectory[0] = '/';
    reader->currentDirectory[1] = 0;
    unsigned int dirLength = strlen(reader->currentDirectory);
    // Last character of the directory string (offset 0x13f + length from the reader base,
    // the directory string starts at +0x140 in the original layout).
    char* dirEnd = (char*)(dirLength + (unsigned int)reader);
    signed char lastChar = (signed char)dirEnd[0x13f];
    // Ensure the directory ends in a path separator (backslash or '/'); append '/' if not.
    if (lastChar != 0x5c && lastChar != '/') {
        dirEnd[0x140] = '/';
        dirEnd[0x141] = 0;
    }
    int createdOutput;
    int context = ArchiveReader_CreateInputContext(inputArg, sizeArg, flagsArg, &createdOutput);
    if (context == 0) {
        return createdOutput; // creation failed: the creator's own status is passed through
    }
    int archiveHandle = ZipStream_OpenArchiveContext((void*)context);
    reader->archiveHandle = archiveHandle;
    if (archiveHandle == 0) {
        return kReaderInitOpenFailed;
    }
    return 0;
}

// Check the completed CRC, release inflate allocations, and detach the current entry.
int ArchiveContext_CloseCurrentEntry(void* archiveArg) {
    void* archive = archiveArg;
    ZipEntryCleanupView* entry;
    int status;
    if (archive == 0) {
        return kZipInvalidParameter;
    }
    status = 0;
    entry = (ZipEntryCleanupView*)((ZipArchiveContext*)archive)->currentEntry; // 0x802D4770: current entry.
    if (entry == 0) {
        return kZipInvalidParameter;
    }
    if (entry->outputBytesRemaining == 0) {
        if (entry->runningCrc32 != entry->expectedCrc32) {
            status = kZipCrcMismatch;
        }
    }
    if (entry->inputBuffer != 0) {
        free(entry->inputBuffer);
        entry->inputBuffer = 0;
    }
    entry->inputBuffer = 0;
    if (entry->inflateInitialized != 0 && (char*)entry + 4) {
        if (entry->inflateState != 0) {
            if (entry->freeCallback != 0) {
                InflateInternalState* inflate = entry->inflateState;
                InflateBlockState* block = inflate->node;
                if (block != 0) {
                    unsigned int blockMode = block->mode;
                    if (blockMode - 4 <= 1) {
                        entry->freeCallback(entry->allocatorContext, block->codeLengths);
                    }
                    if (block->mode == 6) {
                        entry->freeCallback(entry->allocatorContext, block->codes);
                    }
                    block->mode = 0;
                    block->bitCount = 0;
                    block->bitBuffer = 0;
                    void* window = block->window;
                    block->writeCursor = window;
                    block->readCursor = window;
                    if (block->calculateChecksum != 0) {
                        int callbackResult = block->calculateChecksum(0, 0, 0);
                        block->checksum = callbackResult;
                        entry->adler32 = callbackResult;
                    }
                    entry->freeCallback(entry->allocatorContext, block->window);
                    entry->freeCallback(entry->allocatorContext, block->huffmanWorkspace);
                    entry->freeCallback(entry->allocatorContext, block);
                }
                entry->freeCallback(entry->allocatorContext, entry->inflateState);
                entry->inflateState = 0;
            }
        }
    }
    entry->inflateInitialized = 0;
    if (entry != 0) {
        free(entry);
    }
    ((ZipArchiveContext*)archive)->currentEntry = 0;
    return status;
}

// Read, decrypt, and inflate up to the requested bytes; report end-of-entry through done.
int ArchiveEntry_ReadAndInflate(void* archive, void* output, unsigned int requested, unsigned char* done) {
    // Returns bytes produced (>= 0) or a kZip* / zlib error code; *done = 1 when the entry is fully read.
    // archive: ZipArchiveContext with an open entry; output/requested: destination buffer and capacity.
    int result = 0;
    unsigned int total = 0;
    ZipEntryReader* entry;
    if (done) *done = 0;
    if (!archive) return kZipInvalidParameter;
    entry = (ZipEntryReader*)((ZipArchiveContext*)archive)->currentEntry;
    if (!entry) return kZipInvalidParameter;
    if (!entry->inputBuffer) return kZipInvalidReader;
    if (!requested) return 0;
    entry->nextOutput = (unsigned int)output;
    entry->availableOutput = requested;
    if (requested > entry->outputBytesRemaining) entry->availableOutput = entry->outputBytesRemaining;
    while (entry->availableOutput) {
        // Phase 1: refill the input buffer (up to 16 KiB) from the archive stream when it is empty.
        if (!entry->availableInput && entry->compressedBytesRemaining) {
        unsigned int availableBytes;
        unsigned int chunkBytes = entry->compressedBytesRemaining < kZipInputBufferBytes ? entry->compressedBytesRemaining : kZipInputBufferBytes;
        if (!chunkBytes) { if (done) *done = 1; return 0; }
        AsArchiveStream(entry->archiveStream)->position = entry->compressedDataOffset + entry->archivePrefixBytes;
        availableBytes = chunkBytes;
        unsigned int position = AsArchiveStream(entry->archiveStream)->position;
        unsigned int bufferBytes = AsArchiveStream(entry->archiveStream)->bufferSize;
        if (position + chunkBytes > bufferBytes) availableBytes = bufferBytes - position;
        void* base = AsArchiveStream(entry->archiveStream)->buffer;
        memcpy((void*)entry->inputBuffer, (char*)base + position, availableBytes);
        AsArchiveStream(entry->archiveStream)->position = position + availableBytes;
        if (availableBytes / chunkBytes != 1) return -1;
        entry->compressedDataOffset += chunkBytes;
        entry->compressedBytesRemaining -= chunkBytes;
        entry->nextInput = entry->inputBuffer;
        entry->availableInput = chunkBytes;
        // Phase 1b: ZipCrypto-decrypt the new chunk in place, updating the three keys per byte.
        if (entry->encrypted) {
            unsigned char* buf = (unsigned char*)entry->nextInput;
            unsigned int i;
            for (i = 0; i < chunkBytes; i++) {
                unsigned int keyLow = entry->cryptoKey2 & 0xffff;
                signed char keyStreamByte = (signed char)(((keyLow | 2) * ((keyLow | 2) ^ 1)) >> 8);
                signed char plainByte = (signed char)buf[i] ^ keyStreamByte;
                unsigned int key0 = entry->cryptoKey0;
                entry->cryptoKey0 = (key0 >> 8) ^ crc32TableAndUnzipVersion[(key0 ^ plainByte) & 0xff];
                entry->cryptoKey1 = (entry->cryptoKey1 + (entry->cryptoKey0 & 0xff)) * 0x08088405 + 1;
                entry->cryptoKey2 = (entry->cryptoKey2 >> 8) ^ crc32TableAndUnzipVersion[(entry->cryptoKey2 ^ (entry->cryptoKey1 >> 24)) & 0xff];
                buf[i] = (unsigned char)plainByte;
            }
        }
        }
        // Phase 2: consume the 12-byte ZipCrypto header (possibly spread over several chunks).
        // The header counts against outputBytesRemaining (it is part of the entry's data size);
        // its last byte must equal passwordCheckByte once the whole header is consumed.
        unsigned int headerBytesToSkip=entry->availableInput > entry->encryptionHeaderRemaining ? entry->encryptionHeaderRemaining : entry->availableInput;
        if (headerBytesToSkip) {
            signed char ch=((signed char*)entry->nextInput)[headerBytesToSkip-1];
            entry->outputBytesRemaining-=headerBytesToSkip;
            entry->availableInput-=headerBytesToSkip;
            entry->nextInput+=headerBytesToSkip;
            entry->encryptionHeaderRemaining-=headerBytesToSkip;
            if (!entry->encryptionHeaderRemaining && ch != (signed char)entry->passwordCheckByte) return kZipBadPassword;
        }
        // Phase 3a: stored entry - copy min(input, output space) bytes and update the CRC.
        if (!entry->compressionMethod) {
            unsigned int count=entry->availableInput < entry->availableOutput ? entry->availableInput : entry->availableOutput;
            if (count) {
                unsigned int copyIndex;
                for (copyIndex=0; copyIndex<count; copyIndex++) ((unsigned char*)entry->nextOutput)[copyIndex]=((unsigned char*)entry->nextInput)[copyIndex];
            }
            entry->runningCrc32=Zlib_Crc32(entry->runningCrc32,(void*)entry->nextOutput,count);
            total+=count;
            entry->outputBytesRemaining-=count;
            entry->availableInput-=count;
            entry->availableOutput-=count;
            entry->nextOutput+=count;
            entry->nextInput+=count;
            entry->totalOutput+=count;
            if (!entry->outputBytesRemaining && done) *done=1;
        } else {
            // Phase 3b: deflated entry - inflate; bytes produced = growth of totalOutput.
            unsigned int previousOutputBytes=entry->totalOutput;
            unsigned int outputStart=entry->nextOutput;
            result=Zlib_Inflate(&entry->nextInput,kInflateSyncFlush);
            unsigned int count=entry->totalOutput-previousOutputBytes;
            entry->runningCrc32=Zlib_Crc32(entry->runningCrc32,(void*)outputStart,count);
            total+=count; entry->outputBytesRemaining-=count;
            if (result==kInflateStreamEnd || !entry->outputBytesRemaining) { if(done)*done=1; return total; }
            if (result) break;
        }
        if (!entry->availableOutput) break;
    }
    return result ? result : total;
}

struct ZipBufferedStream {
    unsigned char hasBackingFile;
    char pad_01[7];
    unsigned char fileError;
    char pad_09[0xb];
    unsigned char* buffer;
    unsigned int bufferSize;
    unsigned int position;
};

static inline bool StreamBad(ZipBufferedStream* s) {
    return s->hasBackingFile != 0 && s->fileError != 0;
}

static inline int ReadU8(ZipBufferedStream* s, unsigned char* value) {
    int n = 1;
    int pos = s->position;
    if (pos + 1 > s->bufferSize) {
        n = s->bufferSize - pos;
    }
    unsigned char b;
    memcpy(&b, s->buffer + pos, n);
    s->position += n;
    if (n == 1) {
        *value = b;
        return 0;
    }
    return -StreamBad(s);
}

static inline int ReadU16(ZipBufferedStream* s, unsigned int* out) {
    unsigned char byte;
    int e = ReadU8(s, &byte);
    unsigned int v = byte;
    if (e == 0) {
        e = ReadU8(s, &byte);
    }
    v += (unsigned int)byte << 8;
    *out = (e == 0) ? v : 0;
    return e;
}

// Validate the local ZIP header against directory metadata and locate its extra field.
int ArchiveContext_CheckLocalFileHeader(void* mgrArg, unsigned int* outVariableBytes, unsigned int* outExtraFieldOffset, unsigned int* outExtraFieldBytes) {
    ZipArchiveContext* mgr = (ZipArchiveContext*)mgrArg;
    int status = 0;
    unsigned int magic;
    unsigned int local;
    unsigned int flags;
    unsigned int fileNameBytes;
    unsigned int extraFieldBytes;

    *outVariableBytes = 0;
    *outExtraFieldOffset = 0;
    *outExtraFieldBytes = 0;
    ((ZipBufferedStream*)mgr->archiveStream)->position = mgr->localHeaderOffset + mgr->archivePrefixBytes;
    if (ZipStream_ReadU32LE((ZipBufferedStream*)mgr->archiveStream, &magic) != 0) {
        status = -1;
    } else if (magic != kZipLocalHeaderSignature) {
        status = kZipBadArchive;
    }

    if (ReadU16((ZipBufferedStream*)mgr->archiveStream, &local) != 0) status = -1;
    if (ReadU16((ZipBufferedStream*)mgr->archiveStream, &flags) != 0) status = -1;
    if (ReadU16((ZipBufferedStream*)mgr->archiveStream, &local) != 0) {
        status = -1;
    } else if (status == 0 && local != mgr->compressionMethod) {
        status = kZipBadArchive;
    }
    if (status == 0 && mgr->compressionMethod != kZipStored && mgr->compressionMethod != kZipDeflated) status = kZipBadArchive;

    if (ZipStream_ReadU32LE((ZipBufferedStream*)mgr->archiveStream, &local) != 0) status = -1;
    if (ZipStream_ReadU32LE((ZipBufferedStream*)mgr->archiveStream, &local) != 0) {
        status = -1;
    } else if (status == 0 && local != mgr->expectedCrc32 && !(flags & kZipDataDescriptorFlag)) {
        status = kZipBadArchive;
    }

    if (ZipStream_ReadU32LE((ZipBufferedStream*)mgr->archiveStream, &local) != 0) {
        status = -1;
    } else if (status == 0 && local != mgr->compressedSize && !(flags & kZipDataDescriptorFlag)) {
        status = kZipBadArchive;
    }

    if (ZipStream_ReadU32LE((ZipBufferedStream*)mgr->archiveStream, &local) != 0) {
        status = -1;
    } else if (status == 0 && local != mgr->uncompressedSize && !(flags & kZipDataDescriptorFlag)) {
        status = kZipBadArchive;
    }

    if (ReadU16((ZipBufferedStream*)mgr->archiveStream, &fileNameBytes) != 0) {
        status = -1;
    } else if (status == 0 && fileNameBytes != mgr->fileNameLength) {
        status = kZipBadArchive;
    }
    *outVariableBytes = *outVariableBytes + fileNameBytes;

    if (ReadU16((ZipBufferedStream*)mgr->archiveStream, &extraFieldBytes) != 0) status = -1;

    *outExtraFieldOffset = fileNameBytes + mgr->localHeaderOffset + kZipLocalHeaderBytes;
    *outExtraFieldBytes = extraFieldBytes;
    *outVariableBytes = *outVariableBytes + extraFieldBytes;
    return status;
}

} // extern "C"
