// Translation unit 0x804125F8-0x80412608 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/GtlDvdFileStream.h"

extern "C" {

void* gtl_DvdFilePosition_SeekClamped(void*, int, int);
void* DVDCancel(void*);

// Update the embedded {length, position} pair using byte offset and origin.
// Origin 0 resets to start, 2 selects end, other values use current position.
// The helper clamps unsigned addition to length, so negative underflow can land
// at EOF. Its implementation returns void: this legacy void* declaration does
// not provide a meaningful return value. The stream remains caller-owned.
void* gtl_nw4r_ut_DvdFileStream_Seek(void* self, int offset, int origin) {
	return gtl_DvdFilePosition_SeekClamped(&((GtlDvdFileStream*)self)->length, offset, origin); // 0x804125F8: +0x14
}

// Cancel the embedded DVD file-info command and forward DVDCancel's raw result.
// The stream/file-info storage is borrowed; cancellation does not free it. The
// legacy void* return carries the SDK result word, not an owned object pointer.
void* gtl_nw4r_ut_DvdFileStream_Cancel(void* self) {
	return DVDCancel(((GtlDvdFileStream*)self)->fileInfo); // 0x80412600: +0x28
}

} // extern "C"
