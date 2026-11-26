// Reads ZIP central-directory entries from an in-memory archive: the fixed header fields, the
// file name, extra field and comment (copied into caller buffers), and the local-header offset.

#pragma use_lmw_stmw on
extern "C" {
int ZipStream_ReadU32LE(void*, unsigned*);
void* memcpy(void*, const void*, unsigned);

struct ZipMemoryReader {
    unsigned char flags[0x14];
    unsigned char* buffer;
    unsigned bufferBytes;
    unsigned position;
};
struct ZipDirectoryCursor {
    ZipMemoryReader* reader;
    char otherCursorState04[8];
    unsigned offset;
    char otherCursorState10[4];
    unsigned base;
};
struct ZipDosDate {
    unsigned second, minute, hour, day, month, year;
};
struct ZipEntryInfo {
    unsigned versionMadeBy, versionNeeded, flags, compressionMethod, dosTimestamp, crc32, compressedBytes,
        uncompressedBytes;
    unsigned fileNameBytes, extraFieldBytes, commentBytes, startDisk, internalAttributes, externalAttributes;
    ZipDosDate date;
};

// The byte expansion preserves the short-read value lifetime required by CodeWarrior.
#define READ_BYTE(B, V)                               \
    {                                                 \
        int count = 1;                                \
        unsigned pos = r->position;                   \
        if (pos + 1 > r->bufferBytes)                 \
            count = r->bufferBytes - pos;             \
        memcpy(&(B), r->buffer + pos, count);         \
        r->position += count;                         \
        if (count == 1) {                             \
            (V) = (B);                                \
            err = 0;                                  \
        } else {                                      \
            unsigned failed;                          \
            if (r->flags[0] != 0 && r->flags[8] != 0) \
                failed = 1;                           \
            else                                      \
                failed = 0;                           \
            err = -(unsigned)(failed != 0);           \
        }                                             \
    }
inline int ReadShort(ZipMemoryReader* r, unsigned* output) {
    unsigned char first;
    unsigned char second;
    unsigned low;
    int value;
    int err;
    READ_BYTE(first, value);
    low = value;
    if (err == 0) {
        READ_BYTE(second, value);
    }
    low += value << 8;
    if (err == 0)
        *output = low;
    else
        *output = 0;
    return err;
}
// Copy one variable-length ZIP field using the existing unsigned length clamp.
// This assumes position <= bufferBytes and a non-wrapping position + length.
// The caller's direct skips do not enforce those bounds; subtraction can underflow.
inline void ReadField(ZipMemoryReader* r, unsigned char* destination, unsigned length, int& status) {
    unsigned actual = length;
    unsigned position = r->position;
    if (position + length > r->bufferBytes)
        actual = r->bufferBytes - position;
    memcpy(destination, r->buffer + position, actual);
    unsigned items = actual / length;
    r->position += actual;
    if (items != 1)
        status = -1;
}
enum {
    kZipReadError = -1,
    kZipInvalidCursor = -0x66,
    kZipBadSignature = -0x67,
    kCentralDirectorySignature = 0x02014B50,
    kDosYearBase = 1980,
    kDosDayMask = 31,
    kDosMonthMask = 15,
    kDosMinuteMask = 63
};

// Decode a central-directory header and optionally copy its name, extra field, comment, and local-header offset.
int ZipArchive_ReadCentralDirectoryEntry(ZipDirectoryCursor* source, ZipEntryInfo* out, unsigned* outLocalHeaderOffset,
                                         unsigned char* name, unsigned nameCapacity, unsigned char* extra,
                                         unsigned extraCapacity, unsigned char* comment, unsigned commentCapacity) {
    register int status = 0;
    unsigned localHeaderOffset;
    unsigned signature;
    ZipEntryInfo info;
    if (source == 0)
        return kZipInvalidCursor;
    source->reader->position = source->base + source->offset;
    if (ZipStream_ReadU32LE(source->reader, &signature) != 0)
        status = kZipReadError;
    else if (signature != kCentralDirectorySignature)
        status = kZipBadSignature;

    if (ReadShort(source->reader, &info.versionMadeBy) != 0)
        status = kZipReadError;
    if (ReadShort(source->reader, &info.versionNeeded) != 0)
        status = kZipReadError;
    if (ReadShort(source->reader, &info.flags) != 0)
        status = kZipReadError;
    if (ReadShort(source->reader, &info.compressionMethod) != 0)
        status = kZipReadError;
    if (ZipStream_ReadU32LE(source->reader, &info.dosTimestamp) != 0)
        status = kZipReadError;
    info.date.day = (info.dosTimestamp >> 16) & kDosDayMask;
    info.date.month = ((info.dosTimestamp >> 21) & kDosMonthMask) - 1;
    info.date.year = (info.dosTimestamp >> 25) + kDosYearBase;
    info.date.hour = (info.dosTimestamp >> 11) & 31;
    info.date.minute = (info.dosTimestamp >> 5) & kDosMinuteMask;
    info.date.second = (info.dosTimestamp & 31) * 2;
    if (ZipStream_ReadU32LE(source->reader, &info.crc32) != 0)
        status = kZipReadError;
    if (ZipStream_ReadU32LE(source->reader, &info.compressedBytes) != 0)
        status = kZipReadError;
    if (ZipStream_ReadU32LE(source->reader, &info.uncompressedBytes) != 0)
        status = kZipReadError;
    if (ReadShort(source->reader, &info.fileNameBytes) != 0)
        status = kZipReadError;
    if (ReadShort(source->reader, &info.extraFieldBytes) != 0)
        status = kZipReadError;
    if (ReadShort(source->reader, &info.commentBytes) != 0)
        status = kZipReadError;
    if (ReadShort(source->reader, &info.startDisk) != 0)
        status = kZipReadError;
    if (ReadShort(source->reader, &info.internalAttributes) != 0)
        status = kZipReadError;
    if (ZipStream_ReadU32LE(source->reader, &info.externalAttributes) != 0)
        status = kZipReadError;
    if (ZipStream_ReadU32LE(source->reader, &localHeaderOffset) != 0)
        status = kZipReadError;

    unsigned bytesToSkip = info.fileNameBytes;
    if (status == 0 && name != 0) {
        unsigned count;
        if (info.fileNameBytes < nameCapacity) {
            name[info.fileNameBytes] = 0;
            count = info.fileNameBytes;
        } else
            count = nameCapacity;
        if (info.fileNameBytes != 0 && nameCapacity != 0) {
            ReadField(source->reader, name, count, status);
        }
        bytesToSkip -= count;
    }
    if (status == 0 && extra != 0) {
        unsigned count = extraCapacity;
        if (info.extraFieldBytes < extraCapacity)
            count = info.extraFieldBytes;
        if (bytesToSkip != 0) {
            source->reader->position += bytesToSkip;
            bytesToSkip = 0;
        }
        if (info.extraFieldBytes != 0 && extraCapacity != 0) {
            ReadField(source->reader, extra, count, status);
        }
        bytesToSkip += info.extraFieldBytes - count;
    } else
        bytesToSkip += info.extraFieldBytes;
    if (status == 0 && comment != 0) {
        unsigned count;
        if (info.commentBytes < commentCapacity) {
            comment[info.commentBytes] = 0;
            count = info.commentBytes;
        } else
            count = commentCapacity;
        if (bytesToSkip != 0)
            source->reader->position += bytesToSkip;
        if (info.commentBytes != 0 && commentCapacity != 0) {
            ReadField(source->reader, comment, count, status);
        }
    }
    if (status == 0 && out != 0) {
        out->versionMadeBy = info.versionMadeBy;
        out->versionNeeded = info.versionNeeded;
        out->flags = info.flags;
        out->compressionMethod = info.compressionMethod;
        out->dosTimestamp = info.dosTimestamp;
        out->crc32 = info.crc32;
        out->compressedBytes = info.compressedBytes;
        out->uncompressedBytes = info.uncompressedBytes;
        out->fileNameBytes = info.fileNameBytes;
        out->extraFieldBytes = info.extraFieldBytes;
        out->commentBytes = info.commentBytes;
        out->startDisk = info.startDisk;
        out->internalAttributes = info.internalAttributes;
        out->externalAttributes = info.externalAttributes;
        unsigned minute = info.date.minute;
        unsigned second = info.date.second;
        out->date.second = second;
        out->date.minute = minute;
        unsigned day = info.date.day;
        unsigned hour = info.date.hour;
        out->date.hour = hour;
        out->date.day = day;
        unsigned year = info.date.year;
        unsigned month = info.date.month;
        out->date.month = month;
        out->date.year = year;
    }
    if (status == 0 && outLocalHeaderOffset != 0)
        *outLocalHeaderOffset = localHeaderOffset;
    return status;
}
}
