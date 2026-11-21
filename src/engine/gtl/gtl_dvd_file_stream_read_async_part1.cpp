#include "classes/GtlDvdFileStream.h"
// Translation unit 0x80412448-0x8041253C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

int DVDReadAsyncPrio(void*, void*, int, int, int, int);
void gtl_DvdFileStream_OnReadComplete(int, void*);
void gtl_DvdFilePosition_AdvanceClamped(void*, int);

enum { kDvdReadAlignment = 32 };

// Submit a read limited by the padded file extent, advancing position only on acceptance.
// size/position/length are bytes. Destination is borrowed until completion and must
// meet DVD alignment/capacity requirements (the final read can include padding to 32 bytes).
// Caller supplies a valid stream with position <= length; this wrapper performs no busy check.
// Returns boolean submission status, not bytes read. Completion calls (result, stream, userData);
// rejection clears busy but leaves those words installed. Accepted position advances immediately.
int gtl_nw4r_ut_DvdFileStream_SubmitReadAsync(GtlDvdFileStream* self, void* destination, unsigned int size,
                                       int callback, int userData, int /*unused*/) {
    // Phase 1: limit the request against the rounded file extent, preserving the original size otherwise.
    unsigned int requestedEnd = (self->position + size + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    unsigned int fileLimit = (self->length + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    if (requestedEnd > fileLimit) {
        size = (self->length - self->position + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    }

    // Phase 2: publish completion context and busy state before entering the DVD API.
    // The second extent check is intentionally retained, even though it repeats the clamp.
    unsigned int alignedEnd = (self->position + size + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    unsigned int alignedLimit = (self->length + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1u);
    self->readCallback = callback;
    self->readUserData = userData;
    self->busy = 1;
    if (alignedEnd > alignedLimit) {
        size = (self->length - self->position + (kDvdReadAlignment - 1)) & ~(kDvdReadAlignment - 1);
    }
    int result = DVDReadAsyncPrio(self->fileInfo, destination,
                              size, self->position, (int)gtl_DvdFileStream_OnReadComplete, self->priority);
    // Phase 3: accepted requests advance the logical position now; rejection only clears busy.
    int success = result != 0;
    if (success) {
        gtl_DvdFilePosition_AdvanceClamped(&self->length, size);
    } else {
        self->busy = 0;
    }
    return success;
}

} // extern "C"
