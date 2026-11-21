// Translation unit 0x80412578-0x804125F8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/GtlDvdFileStream.h"
extern "C" {

enum { kDvdReadAlignment = 32 };

typedef void (*DVDCallback)(int);
extern int DVDReadAsyncPrio(void* fileInfo, void* dst, int length, int offset,
                        DVDCallback callback, int priority);
extern void gtl_DvdFileStream_OnReadComplete(int result);

// Submit an asynchronous DVD read, clamping only requests past the rounded EOF.
// All lengths/positions are bytes. The caller supplies DVD-compatible alignment
// and enough buffer storage for padding through rounded EOF; size is otherwise
// passed unchanged. Requires position <= length and no unsigned-addition overflow.
// callback/userData are stored 32-bit ABI words. Stream and destination must stay
// alive through completion; ownership does not transfer. Return 1 means submission
// was accepted, not completed. Position is unchanged and busy stays set on failure.
int gtl_nw4r_ut_DvdFileStream_ReadAlignedAsync(GtlDvdFileStream* self, void* buffer,
                                      unsigned int size, unsigned int callback,
                                      unsigned int userData) {
    unsigned int streamLength;
    unsigned int streamPosition;
    streamPosition = self->position;
    streamLength = self->length;
    // Phase 1: retain completion words before marking the stream busy.
    self->readCallback = callback;
    self->readUserData = userData;
    unsigned int end = streamPosition + size;
    unsigned int roundedEnd = (end + kDvdReadAlignment - 1) & ~(kDvdReadAlignment - 1);
    unsigned int roundedLength = (streamLength + kDvdReadAlignment - 1) & ~(kDvdReadAlignment - 1);
    self->busy = 1; // 0x80412578: original busy byte +0x6C.
    // Phase 2: compare rounded bounds; only the clamped branch rounds size itself.
    if (roundedEnd > roundedLength) {
        size = (streamLength - streamPosition + kDvdReadAlignment - 1) & ~(kDvdReadAlignment - 1);
    }
    // Phase 3: submit using the captured byte offset; this entry performs no advance.
    int result = DVDReadAsyncPrio(self->fileInfo, buffer, size, streamPosition,
                              gtl_DvdFileStream_OnReadComplete, self->priority);
    return result != 0;
}

} // extern "C"
