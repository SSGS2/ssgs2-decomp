#include "classes/GtlDvdFileStream.h"
// Borrow the stream layout without transferring ownership or changing its representation.
static inline GtlDvdFileStream* AsGtlDvdFileStream(void* object) {
    return (GtlDvdFileStream*)object;
}
// Translation unit 0x8041253C-0x80412578 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

int DVDReadPrio(void*, void*, int, int, int);

enum { kDvdReadAlignment = 32 };

// Read synchronously at the current byte position and return the DVD API's result unchanged.
// Caller provides a valid stream (position <= length) and an aligned destination large
// enough for the rounded final extent. Both are borrowed only during this call.
// This primitive leaves position unchanged; only requests beyond the rounded limit are resized.
int gtl_nw4r_ut_DvdFileStream_ReadAtPosition(void* self, void* destination, unsigned int requestedBytes) {
    unsigned int readOffset;
    unsigned int limit;
    unsigned int roundedEnd;
    unsigned int roundedLimit;
    unsigned int readLength;
    void* priority;
    readOffset = AsGtlDvdFileStream(self)->position;
    limit = AsGtlDvdFileStream(self)->length;
    roundedEnd = (readOffset + requestedBytes + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    roundedLimit = (limit + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    readLength = requestedBytes;
    if (roundedEnd > roundedLimit) {
        readLength = (limit - readOffset + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    }
    priority = (void*)AsGtlDvdFileStream(self)->priority; // Raw priority word; no pointer ownership.
    return DVDReadPrio(AsGtlDvdFileStream(self)->fileInfo, destination, (int)readLength, (int)readOffset, (int)priority);
}

} // extern "C"
