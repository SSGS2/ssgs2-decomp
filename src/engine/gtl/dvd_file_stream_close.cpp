// DvdFileStream::Close: closes the DVD file handle that the stream owns and clears its open flag.

#include "classes/GtlDvdFileStream.h"

extern "C" {

void DVDClose(void*);

} // extern "C"

// Close an owned DVD handle once; borrowed handles stay open.
// This clears isOpen only for an owned, open handle; it
// neither frees the stream nor reports DVDClose errors. A borrowed stream keeps its
// isOpen flag. The fileInfo passed to DVD is embedded, rather than allocated here.
void gtl::nw4r::ut::DvdFileStream::Close() {
    if (ownsFile && isOpen) {
        DVDClose(fileInfo);
        isOpen = 0;
    }
}
