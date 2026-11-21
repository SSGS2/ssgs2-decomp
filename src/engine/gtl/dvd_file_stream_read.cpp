// Translation unit 0x804123C0-0x80412448 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

int DVDReadPrio(void*, void*, int, int, int);
int gtl_DvdFilePosition_AdvanceClamped(unsigned int*, unsigned int);

struct DvdFileStream {
    char pad00[0x14];
    unsigned int length; // +0x14: file length in bytes; first word of the position helper's view.
    unsigned int position; // +0x18: current byte position, not the start of the file.
    char pad1C[0x0C];
    char fileInfo[0x40];
    int priority;
};

enum { kDvdReadAlignmentMask = 0x1F }; // DVD transfers use 32-byte alignment.

// Read at the current DVD byte position and advance it only after a positive result.
// Borrows self/dst; caller supplies DVD-compatible alignment and enough writable
// space for the rounded remaining length, which can exceed the logical remainder.
// Returns the SDK byte count/error unchanged. Assumes position <= file length;
// unsigned addition/rounding is preserved, with no overflow or alignment validation.
int gtl_nw4r_ut_DvdFileStream_ReadAligned(DvdFileStream* self, void* dst, unsigned int length) {
    // Compare rounded byte offsets, not block indices. Leave length unchanged
    // unless the request's rounded end exceeds the file's rounded end.
    unsigned int roundedRequestedEnd = (self->position + length + kDvdReadAlignmentMask) & ~(unsigned int)kDvdReadAlignmentMask;
    unsigned int roundedFileLength = (self->length + kDvdReadAlignmentMask) & ~(unsigned int)kDvdReadAlignmentMask;
    unsigned int size;
    if (roundedRequestedEnd > roundedFileLength)
        size = (self->length - self->position + kDvdReadAlignmentMask) & ~(unsigned int)kDvdReadAlignmentMask;
    else
        size = length;
    int result = DVDReadPrio(self->fileInfo, dst, size, self->position, self->priority);
    if (result > 0) {
        // The helper receives the adjacent (file length, position) pair and clamps
        // position to the logical length even if the read includes DVD padding.
        unsigned int* cursor = &self->length;
        gtl_DvdFilePosition_AdvanceClamped(cursor, result);
    }
    return result;
}

} // extern "C"
