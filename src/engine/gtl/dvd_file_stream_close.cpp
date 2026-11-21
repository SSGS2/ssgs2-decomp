// Translation unit 0x80412370-0x804123C0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "classes/GtlDvdFileStream.h"

extern "C" {

void DVDClose(void*);

struct DvdFileStream {
    // Offset map: open flag +0x04, embedded DVDFileInfo +0x28, close policy +0x6e.
	unsigned char vtableStorage[4];
	unsigned char isOpen;
	unsigned char streamState[0x23];
	unsigned char dvdFileInfo[0x40];
	unsigned char readState[6];
	unsigned char closeFileOnClose;
};

// Close an owned DVD handle once; borrowed handles stay open.
// self must be valid. This clears isOpen only for an owned, open handle; it
// neither frees self nor reports DVDClose errors. A borrowed stream keeps its
// isOpen flag. The fileInfo passed to DVD is embedded, rather than allocated here.
void gtl_nw4r_ut_DvdFileStream_Close(DvdFileStream* self) {
	if (self->closeFileOnClose && self->isOpen) {
		DVDClose(((GtlDvdFileStream*)self)->fileInfo);
		self->isOpen = 0;
	}
}

} // extern "C"
