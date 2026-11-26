// DvdFileStream::SubmitReadAsync: an async DVD read that is clamped to the padded file
// extent and advances the stream position once the read is accepted.

#include "classes/GtlDvdFileStream.h"

extern "C" {

int DVDReadAsyncPrio(void*, void*, int, int, int, int);
void gtl_DvdFilePosition_AdvanceClamped(void*, int);

enum {
    kDvdReadAlignment = 32
};

} // extern "C"

// Submit a read limited by the padded file extent, advancing position only on acceptance.
// size/position/length are bytes. Destination is borrowed until completion and must
// meet DVD alignment/capacity requirements (the final read can include padding to 32 bytes).
// Caller supplies a valid stream with position <= length; this wrapper performs no busy check.
// Returns boolean submission status, not bytes read. Completion calls (result, stream, userData);
// rejection clears busy but leaves those words installed. Accepted position advances immediately.
int gtl::nw4r::ut::DvdFileStream::SubmitReadAsync(void* destination, unsigned int size, int callback, int userData,
                                                  int /*unused*/) {
    // Phase 1: limit the request against the rounded file extent, preserving the original size otherwise.
    unsigned int requestedEnd = (position + size + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    unsigned int fileLimit = (length + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    if (requestedEnd > fileLimit) {
        size = (length - position + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    }

    // Phase 2: publish completion context and busy state before entering the DVD API.
    // The second extent check repeats the clamp above and is kept as written.
    unsigned int alignedEnd = (position + size + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    unsigned int alignedLimit = (length + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    readCallback = callback;
    readUserData = userData;
    busy = 1;
    if (alignedEnd > alignedLimit) {
        size = (length - position + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1);
    }
    int result = DVDReadAsyncPrio(fileInfo, destination, size, position, (int)OnReadComplete, priority);
    // Phase 3: accepted requests advance the logical position now; rejection only clears busy.
    int success = result != 0;
    if (success) {
        gtl_DvdFilePosition_AdvanceClamped(&length, size);
    } else {
        busy = 0;
    }
    return success;
}
