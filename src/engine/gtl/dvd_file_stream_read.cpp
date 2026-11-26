// DvdFileStream::ReadAligned: reads from the DVD at the stream's position, rounding the size up to
// 32-byte blocks, then moves the position forward by the bytes read.

#include "classes/GtlDvdFileStream.h"

extern "C" {

int DVDReadPrio(void*, void*, int, int, int);
int gtl_DvdFilePosition_AdvanceClamped(unsigned int*, unsigned int);

enum {
    kDvdReadAlignmentMask = 0x1F
}; // DVD transfers use 32-byte alignment.

} // extern "C"

// Read at the current DVD byte position and advance it only after a positive result.
// Borrows dst; caller supplies DVD-compatible alignment and enough writable
// space for the rounded remaining length, which can exceed the logical remainder.
// Returns the SDK byte count/error unchanged. Assumes position <= file length;
// unsigned addition/rounding is preserved, with no overflow or alignment validation.
int gtl::nw4r::ut::DvdFileStream::ReadAligned(void* dst, unsigned int requestedLength) {
    // Compare rounded byte offsets, not block indices. Leave the length unchanged
    // unless the request's rounded end exceeds the file's rounded end.
    unsigned int roundedRequestedEnd =
        (position + requestedLength + kDvdReadAlignmentMask) & ~(unsigned int)kDvdReadAlignmentMask;
    unsigned int roundedFileLength = (length + kDvdReadAlignmentMask) & ~(unsigned int)kDvdReadAlignmentMask;
    unsigned int size;
    if (roundedRequestedEnd > roundedFileLength)
        size = (length - position + kDvdReadAlignmentMask) & ~(unsigned int)kDvdReadAlignmentMask;
    else
        size = requestedLength;
    int result = DVDReadPrio(fileInfo, dst, size, position, priority);
    if (result > 0) {
        // The helper receives the adjacent (file length, position) pair and clamps
        // position to the logical length even if the read includes DVD padding.
        unsigned int* cursor = &length;
        gtl_DvdFilePosition_AdvanceClamped(cursor, result);
    }
    return result;
}
