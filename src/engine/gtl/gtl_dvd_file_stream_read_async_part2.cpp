// DvdFileStream::ReadAlignedAsync: an async DVD read at the stream's current position that
// clamps only requests past the rounded end of file. The position does not change.

#include "classes/GtlDvdFileStream.h"

extern "C" {

enum {
    kDvdReadAlignment = 32
};

typedef void (*DVDCallback)(int);
extern int DVDReadAsyncPrio(void* fileInfo, void* dst, int length, int offset, DVDCallback callback, int priority);

} // extern "C"

// Submit an asynchronous DVD read, clamping only requests past the rounded EOF.
// All lengths/positions are bytes. The caller supplies DVD-compatible alignment
// and enough buffer storage for padding through rounded EOF; size is otherwise
// passed unchanged. Requires position <= length and no unsigned-addition overflow.
// callback/userData are stored 32-bit ABI words. Stream and destination must stay
// alive through completion; ownership does not transfer. Return 1 means submission
// was accepted, not completed. Position is unchanged and busy stays set on failure.
int gtl::nw4r::ut::DvdFileStream::ReadAlignedAsync(void* buffer, unsigned int size, unsigned int callback,
                                                   unsigned int userData) {
    unsigned int streamLength;
    unsigned int streamPosition;
    streamPosition = position;
    streamLength = length;
    // Phase 1: retain completion words before marking the stream busy.
    readCallback = callback;
    readUserData = userData;
    unsigned int end = streamPosition + size;
    unsigned int roundedEnd = (end + kDvdReadAlignment - 1) & ~(kDvdReadAlignment - 1);
    unsigned int roundedLength = (streamLength + kDvdReadAlignment - 1) & ~(kDvdReadAlignment - 1);
    busy = 1;
    // Phase 2: compare rounded bounds; only the clamped branch rounds size itself.
    if (roundedEnd > roundedLength) {
        size = (streamLength - streamPosition + kDvdReadAlignment - 1) & ~(kDvdReadAlignment - 1);
    }
    // Phase 3: submit using the captured byte offset; this entry performs no advance.
    int result = DVDReadAsyncPrio(fileInfo, buffer, size, streamPosition, (DVDCallback)OnReadComplete, priority);
    return result != 0;
}
