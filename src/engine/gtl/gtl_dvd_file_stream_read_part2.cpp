// DvdFileStream::ReadAtPosition: a synchronous DVD read at the current byte position. The
// position is left unchanged, and only requests past the rounded limit are resized.

#include "classes/GtlDvdFileStream.h"

extern "C" {

int DVDReadPrio(void*, void*, int, int, int);

enum {
    kDvdReadAlignment = 32
};

} // extern "C"

// Read synchronously at the current byte position and return the DVD API's result unchanged.
// Caller provides a valid stream (position <= length) and an aligned destination large
// enough for the rounded final extent. Both are borrowed only during this call.
// This primitive leaves position unchanged; only requests beyond the rounded limit are resized.
int gtl::nw4r::ut::DvdFileStream::ReadAtPosition(void* destination, unsigned int requestedBytes) {
    unsigned int readOffset;
    unsigned int limit;
    unsigned int roundedEnd;
    unsigned int roundedLimit;
    unsigned int readLength;
    void* priorityWord;
    readOffset = position;
    limit = length;
    roundedEnd = (readOffset + requestedBytes + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    roundedLimit = (limit + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    readLength = requestedBytes;
    if (roundedEnd > roundedLimit) {
        readLength = (limit - readOffset + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    }
    priorityWord = (void*)priority; // Raw priority word; no pointer ownership.
    return DVDReadPrio(fileInfo, destination, (int)readLength, (int)readOffset, (int)priorityWord);
}
