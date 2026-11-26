// DvdFileStream::Seek and DvdFileStream::Cancel: moves the stream position within its length and
// cancels a pending DVD command on the stream's file info.

#include "classes/GtlDvdFileStream.h"

extern "C" {

void* gtl_DvdFilePosition_SeekClamped(void*, int, int);
void* DVDCancel(void*);

} // extern "C"

// Update the embedded {length, position} pair using byte offset and origin.
// Origin 0 resets to start, 2 selects end, other values use current position.
// The helper clamps unsigned addition to length, so negative underflow can land
// at EOF. Its implementation returns void: this legacy void* declaration does
// not provide a meaningful return value. The stream remains caller-owned.
void* gtl::nw4r::ut::DvdFileStream::Seek(int offset, int origin) {
    return gtl_DvdFilePosition_SeekClamped(&length, offset, origin);
}

// Cancel the embedded DVD file-info command and forward DVDCancel's raw result.
// The stream/file-info storage is borrowed; cancellation does not free it. The
// legacy void* return carries the SDK result word, not an owned object pointer.
void* gtl::nw4r::ut::DvdFileStream::Cancel() {
    return DVDCancel(fileInfo);
}
