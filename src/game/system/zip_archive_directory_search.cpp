// Translation unit 0x802D1CE4-0x802D2714 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

extern "C" {

enum ZipDirectoryConstants {
    kZipReadError = -1,
    kZipBadArchive = -103,
    kMaxZipCommentLength = 0xffff,
    kZipSearchStep = 0x400,
    kZipSignatureLength = 4,
    kZipSearchBufferSize = kZipSearchStep + kZipSignatureLength,
    kZipSignatureP = 0x50, kZipSignatureK = 0x4b,
    kZipEndSignatureLow = 0x05, kZipEndSignatureHigh = 0x06
};
void* malloc(unsigned int);
void free(void*);
void* memcpy(void*, const void*, unsigned int);
int ZipStream_ReadU32LE(void*, unsigned int*);
int ZipArchive_ReadCentralDirectoryEntry(void*, void*, void*, int, int, int, int, int, int);
void File_CloseHandle(void*);
void __dl__FPv(void*);

// Two views of the SAME stream object, not two independent streams:
// +00/+08 first flag byte, +04 file handle, +0C archive base, +10 ownership,
// +14 data, +18 size, +1C position in both views. ZipArchiveInput additionally
// exposes +20 as unk_20; its meaning is not established by this unit.
// Thus the search helper updates OpenArchiveContext's position through the cast;
// it never accesses the separate +20 field. data addresses the in-memory buffer;
// its ownership is not decoded by these views. size/position/base are byte offsets
// and all views require this 32-bit layout.
struct ZipSearchStream {
    unsigned char streamFlags[0x8];
    unsigned char bufferFlags[8];
    char ownsFileHandle[4];
    void* data;
    unsigned int size;
    unsigned int position;
};
int ZipStream_FindEndOfCentralDirectoryLegacy(ZipSearchStream*);

struct ZipArchiveInput {
    unsigned char streamFlags[4];
    unsigned int fileHandle;
    unsigned char bufferFlags[4];
    unsigned int archiveBaseOffset;
    unsigned char ownsFileHandle[4];
    void* data;
    unsigned int size;
    unsigned int position;
    unsigned int unk_20;
};

struct ZipArchiveContext {
    unsigned int stream;
    struct { unsigned int entryCount, commentLength; } globalInfo;
    unsigned int bytesBeforeArchive;
    unsigned int currentEntryIndex;
    unsigned int currentEntryOffset;
    unsigned int currentEntryValid;
    unsigned int endOfDirectoryOffset;
    unsigned int centralDirectorySize;
    unsigned int centralDirectoryOffset;
    unsigned int currentEntryInfo[20];
    unsigned int localHeaderOffset;
    unsigned int activeReadState;
};

struct ZipDirectoryReadState {
    unsigned int sig;
    ZipArchiveContext info;
};

// ZipStream_OpenArchiveContext (0x802D1E38): current entry info and local-header offset.
// The stream flag bytes retain their legacy short-read test; ownership is at 0x10.
typedef char U802D1E38Info_off_arr[((int)&((ZipArchiveContext*)0)->currentEntryInfo == 0x28) ? 1 : -1];
typedef char U802D1E38Info_off_f30[((int)&((ZipArchiveContext*)0)->localHeaderOffset == 0x78) ? 1 : -1];

// Preserve the legacy short-read policy; neither flag has a confirmed standalone meaning.
static inline int ZipLegacyShortReadStatus(ZipArchiveInput* stream) {
    int shortReadIsError;
    if (stream->streamFlags[0] && stream->bufferFlags[0]) shortReadIsError = 1;
    else shortReadIsError = 0;
    return shortReadIsError ? kZipReadError : 0;
}

#pragma opt_lifetimes off
// Stack staging order is reversed from file order; each u16 keeps its low/high byte pair.
// This is scratch storage, not the packed on-disk ZIP header.
struct ZipEndRecordScratch {
    unsigned char commentLength[2];
    unsigned char totalEntries[2];
    unsigned char entriesOnDisk[2];
    unsigned char directoryDisk[2];
    unsigned char disk[2];
    unsigned char unused[2];
    unsigned int signature;
};
typedef char ZipEndRecordScratchSize[sizeof(ZipEndRecordScratch) == 16 ? 1 : -1];

// Parse the end record, reject split archives, and initialize the first central-directory entry.
// Ownership: null input returns null. A nonnull input is consumed on either
// outcome: parse failure closes its file handle only if ownsFileHandle[0], then
// deletes the stream; success retains it in context->stream for archive cleanup.
// Callers must not close/delete the supplied stream after this call. The backing
// data is not freed here. Success returns a malloc-owned context; failure null.
// Legacy allocation behavior is preserved: *context is assigned before its null
// check, so this routine requires context allocation to succeed.
//
// End-record grammar, little endian, in read order:
// u32 signature; u16 disk; u16 directory disk; u16 entries on disk; u16 total entries;
// u32 directory size; u32 directory offset; u16 comment length.
// globalInfo.entryCount receives entries on disk; it must equal totalEntries,
// so accepted single-disk archives have the same count in both words. Do not reorder.
// Each expanded u16 block reads low byte then high byte, advancing position per
// byte; it latches -1 only when BOTH flag bytes are set on a short read. Other
// flag combinations preserve legacy behavior (byteValue may be uninitialized).
// These are not general checked readers and must not be 'fixed'.
void* ZipStream_OpenArchiveContext(ZipArchiveInput* self) {
    // Phase 1: find the end record and position the stream at its signature.
    if (!self) return 0;
    int diskNumber, directoryDiskNumber, totalEntries;
    int bytesRead;
    int error = 0;
    ZipEndRecordScratch header;
    ZipArchiveContext archive = {};
    header.signature = 0;
    unsigned int directoryEnd = (unsigned int)ZipStream_FindEndOfCentralDirectoryLegacy((ZipSearchStream*)self);
    if (directoryEnd == (unsigned int)kZipReadError) error = kZipReadError;
    if (!error) self->position = directoryEnd;
    if (!error) {
        if (ZipStream_ReadU32LE(self, &header.signature)) error = kZipReadError;
    }

    // Phase 2: decode the five u16 fields around the two u32 directory fields.
    // u16 #1: disk number, staging bytes +8/+9 (low/high).
    diskNumber = 0;
    if (!error) {
        bytesRead = 1;
        if (self->position + 1 > self->size) bytesRead = self->size - self->position;
        memcpy(&header.disk[0], (char*)self->data + self->position, bytesRead);
        self->position += bytesRead;
        unsigned int byteValue;
        unsigned int wordValue;
        int readError;
        if (bytesRead == 1) { byteValue = header.disk[0]; readError = 0; }
        else { readError = ZipLegacyShortReadStatus(self); }
        wordValue = byteValue;
        if (readError == 0) {
            bytesRead = 1;
            if (self->position + 1 > self->size) bytesRead = self->size - self->position;
            memcpy(&header.disk[1], (char*)self->data + self->position, bytesRead);
            self->position += bytesRead;
            if (bytesRead == 1) { byteValue = header.disk[1]; readError = 0; }
            else { readError = ZipLegacyShortReadStatus(self); }
        }
        wordValue += byteValue << 8;
        diskNumber = (int)(wordValue & -(readError == 0));
        if (readError != 0) error = kZipReadError;
    }

    // u16 #2: central-directory disk number, staging bytes +6/+7.
    directoryDiskNumber = 0;
    if (!error) {
        bytesRead = 1;
        if (self->position + 1 > self->size) bytesRead = self->size - self->position;
        memcpy(&header.directoryDisk[0], (char*)self->data + self->position, bytesRead);
        self->position += bytesRead;
        unsigned int byteValue;
        unsigned int wordValue;
        int readError;
        if (bytesRead == 1) { byteValue = header.directoryDisk[0]; readError = 0; }
        else { readError = ZipLegacyShortReadStatus(self); }
        wordValue = byteValue;
        if (readError == 0) {
            bytesRead = 1;
            if (self->position + 1 > self->size) bytesRead = self->size - self->position;
            memcpy(&header.directoryDisk[1], (char*)self->data + self->position, bytesRead);
            self->position += bytesRead;
            if (bytesRead == 1) { byteValue = header.directoryDisk[1]; readError = 0; }
            else { readError = ZipLegacyShortReadStatus(self); }
        }
        wordValue += byteValue << 8;
        directoryDiskNumber = (int)(wordValue & -(readError == 0));
        if (readError != 0) error = kZipReadError;
    }

    // u16 #3: entries on this disk (stored in globalInfo.entryCount), staging bytes +4/+5.
    if (!error) {
        bytesRead = 1;
        if (self->position + 1 > self->size) bytesRead = self->size - self->position;
        memcpy(&header.entriesOnDisk[0], (char*)self->data + self->position, bytesRead);
        self->position += bytesRead;
        unsigned int byteValue;
        unsigned int wordValue;
        int readError;
        if (bytesRead == 1) { byteValue = header.entriesOnDisk[0]; readError = 0; }
        else { readError = ZipLegacyShortReadStatus(self); }
        wordValue = byteValue;
        if (readError == 0) {
            bytesRead = 1;
            if (self->position + 1 > self->size) bytesRead = self->size - self->position;
            memcpy(&header.entriesOnDisk[1], (char*)self->data + self->position, bytesRead);
            self->position += bytesRead;
            if (bytesRead == 1) { byteValue = header.entriesOnDisk[1]; readError = 0; }
            else { readError = ZipLegacyShortReadStatus(self); }
        }
        wordValue += byteValue << 8;
        if (readError == 0) archive.globalInfo.entryCount = wordValue;
        else archive.globalInfo.entryCount = 0;
        if (readError != 0) error = kZipReadError;
    }

    // u16 #4: total entries across all disks, staging bytes +2/+3.
    totalEntries = 0;
    if (!error) {
        bytesRead = 1;
        if (self->position + 1 > self->size) bytesRead = self->size - self->position;
        memcpy(&header.totalEntries[0], (char*)self->data + self->position, bytesRead);
        self->position += bytesRead;
        unsigned int byteValue;
        unsigned int wordValue;
        int readError;
        if (bytesRead == 1) { byteValue = header.totalEntries[0]; readError = 0; }
        else { readError = ZipLegacyShortReadStatus(self); }
        wordValue = byteValue;
        if (readError == 0) {
            bytesRead = 1;
            if (self->position + 1 > self->size) bytesRead = self->size - self->position;
            memcpy(&header.totalEntries[1], (char*)self->data + self->position, bytesRead);
            self->position += bytesRead;
            if (bytesRead == 1) { byteValue = header.totalEntries[1]; readError = 0; }
            else { readError = ZipLegacyShortReadStatus(self); }
        }
        wordValue += byteValue << 8;
        totalEntries = (int)(wordValue & -(readError == 0));
        if (readError != 0) error = kZipReadError;
    }

    if (!error) {
        if (totalEntries != archive.globalInfo.entryCount || directoryDiskNumber != 0 || diskNumber != 0) error = kZipBadArchive;
    }
    if (!error) {
        if (ZipStream_ReadU32LE(self, &archive.centralDirectorySize)) error = kZipReadError;
    }
    if (!error) {
        if (ZipStream_ReadU32LE(self, &archive.centralDirectoryOffset)) error = kZipReadError;
    }

    // u16 #5: archive-comment byte length, staging bytes +0/+1.
    if (!error) {
        bytesRead = 1;
        if (self->position + 1 > self->size) bytesRead = self->size - self->position;
        memcpy(&header.commentLength[0], (char*)self->data + self->position, bytesRead);
        self->position += bytesRead;
        unsigned int byteValue;
        unsigned int wordValue;
        int readError;
        if (bytesRead == 1) { byteValue = header.commentLength[0]; readError = 0; }
        else { readError = ZipLegacyShortReadStatus(self); }
        wordValue = byteValue;
        if (readError == 0) {
            bytesRead = 1;
            if (self->position + 1 > self->size) bytesRead = self->size - self->position;
            memcpy(&header.commentLength[1], (char*)self->data + self->position, bytesRead);
            self->position += bytesRead;
            if (bytesRead == 1) { byteValue = header.commentLength[1]; readError = 0; }
            else { readError = ZipLegacyShortReadStatus(self); }
        }
        wordValue += byteValue << 8;
        if (readError == 0) archive.globalInfo.commentLength = wordValue;
        else archive.globalInfo.commentLength = 0;
        if (readError != 0) error = kZipReadError;
    }

    if (!error) {
        if (directoryEnd + self->archiveBaseOffset < archive.centralDirectoryOffset + archive.centralDirectorySize) {
            if (!error) error = kZipBadArchive;
        }
    }

    // Phase 3: on any parse/consistency error, consume the stream and return null.
    if (error != 0) {
        if (self) {
            if (self->ownsFileHandle[0]) File_CloseHandle((void*)self->fileHandle);
            __dl__FPv(self);
        }
        return 0;
    }

    // Phase 4: retain the stream, account for any prefix before ZIP data, and
    // initialize first-entry validity. Failure of that entry read does not make
    // opening fail; currentEntryValid records its result in the returned context.
    archive.stream = (unsigned int)self;
    archive.bytesBeforeArchive = directoryEnd + self->archiveBaseOffset - (archive.centralDirectoryOffset + archive.centralDirectorySize);
    archive.endOfDirectoryOffset = directoryEnd;
    archive.activeReadState = 0;
    self->archiveBaseOffset = 0;

    ZipArchiveContext* context = (ZipArchiveContext*)malloc(sizeof(ZipArchiveContext));
    *context = archive;
    if (context) {
        context->currentEntryOffset = context->centralDirectoryOffset;
        context->currentEntryIndex = 0;
        context->currentEntryValid = (ZipArchive_ReadCentralDirectoryEntry(context, context->currentEntryInfo, &context->localHeaderOffset,
                               0, 0, 0, 0, 0, 0) == 0);
    }
    return context;
}
#pragma opt_lifetimes reset

// Search backward for the ZIP end-of-central-directory signature with a legacy exit test.
// A first-chunk miss (-1) exits immediately; only a match at offset zero permits another chunk.
// Borrow self/data; return a byte offset or -1, and leave position changed. The
// 0x404-byte temporary is owned/freed here. Search chunks overlap by four bytes.
// This is legacy behavior, not a full implementation of ZIP's backward search.
// The descending loop also probes signatureIndex == -1 on a complete-chunk miss;
// that before-buffer access is preserved here, not endorsed as a safe scan bound.
int ZipStream_FindEndOfCentralDirectoryLegacy(ZipSearchStream* self) {
    unsigned int streamSize = self->size;
    unsigned int searchLimit = kMaxZipCommentLength;
    self->position = streamSize;
    if (streamSize < kMaxZipCommentLength) {
        searchLimit = streamSize;
    }
    void* searchBuffer = malloc(kZipSearchBufferSize);
    if (searchBuffer == 0) {
        return -1;
    }
    int foundOffset = -1;
    unsigned int backwardDistance = kZipSignatureLength;
    while (backwardDistance < searchLimit) {
        backwardDistance += kZipSearchStep;
        if (backwardDistance > searchLimit) {
            backwardDistance = searchLimit;
        }
        unsigned int chunkStart = streamSize - backwardDistance;
        unsigned int chunkSize = kZipSearchBufferSize;
        unsigned int availableBytes = streamSize - chunkStart;
        if (availableBytes <= kZipSearchBufferSize) {
            chunkSize = availableBytes;
        }
        self->position = chunkStart;
        unsigned int readSize = chunkSize;
        unsigned int endPos = chunkStart + chunkSize;
        if (endPos > self->size) {
            readSize = self->size - chunkStart;
        }
        memcpy(searchBuffer, (char*)self->data + chunkStart, readSize);
        unsigned int completeChunk = readSize / chunkSize;
        self->position = self->position + readSize;
        if (completeChunk == 1) {
            int signatureIndex = (int)chunkSize - 3;
            if (signatureIndex >= 0) {
                unsigned int positionsLeft = (unsigned int)(signatureIndex + 1);
                for (; positionsLeft > 0; positionsLeft--) {
                    signatureIndex--;
                    unsigned char* bytes = (unsigned char*)searchBuffer;
                    if (bytes[signatureIndex] == kZipSignatureP) {
                        unsigned char* signature = bytes + signatureIndex;
                        if (signature[1] == kZipSignatureK && signature[2] == kZipEndSignatureLow && signature[3] == kZipEndSignatureHigh) {
                            foundOffset = (int)chunkStart + signatureIndex;
                            break;
                        }
                    }
                }
            }
        }
        // Preserve the legacy exit test: a failed search (-1) also exits here.
        if (foundOffset != 0) {
            break;
        }
    }
    if (searchBuffer != 0) {
        free(searchBuffer);
    }
    return foundOffset;
}
} // extern "C"
