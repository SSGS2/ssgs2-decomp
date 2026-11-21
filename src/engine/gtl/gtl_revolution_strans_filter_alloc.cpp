#include "classes/GtlRevolutionSTransFilter.h"
// View existing filter storage without allocation, ownership changes, or checks.
inline GtlRevolutionSTransFilter* AsGtlRevolutionSTransFilter(void* filter) {
    return static_cast<GtlRevolutionSTransFilter*>(filter);
}
// Translation unit 0x803F9ED8-0x803F9F60 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* gtl_Alloc(unsigned int size);

enum { kTwoBuffersShift = 1, kDataOffsetWordByteOffset = 4, kPcmDataOffset = 0x20 };

// Allocate two adjacent THP PCM work buffers and publish their shared storage.
// buffer/size are required outputs (bytes). The filter owns the allocation;
// callers borrow it until filter cleanup. Nonzero segmentSize must allow its
// 0x20-byte header plus PCM payload. Allocation failure is unchecked, and repeat
// calls with nonzero size overwrite the previous allocation without freeing it.
// A zero size returns the existing buffer, leaving the filter unchanged. Return
// 1 always; this is not an allocation-success test.
int gtl_CRevolutionSTransFilter_AllocateBuffers(void* self, void** buffer, unsigned int* size) {
    *size = AsGtlRevolutionSTransFilter(self)->segmentSize << kTwoBuffersShift;
    *buffer = 0;
    if (*size != 0) {
        // One allocation owns both segments; +0x24/+0x28 are interior aliases.
        AsGtlRevolutionSTransFilter(self)->buffer = gtl_Alloc(*size);
        AsGtlRevolutionSTransFilter(self)->firstSegment = AsGtlRevolutionSTransFilter(self)->buffer;
        // Header word +0 is filled by THPAudioDecode; +4 is the PCM data offset
        // from that segment's base, consumed by vfunc_10 in revolution_strans_audio_decode.cpp.
        *(unsigned int*)((char*)AsGtlRevolutionSTransFilter(self)->buffer + kDataOffsetWordByteOffset) = kPcmDataOffset;
        AsGtlRevolutionSTransFilter(self)->secondSegment = (char*)AsGtlRevolutionSTransFilter(self)->buffer + AsGtlRevolutionSTransFilter(self)->segmentSize;
        *(unsigned int*)((char*)AsGtlRevolutionSTransFilter(self)->secondSegment + kDataOffsetWordByteOffset) = kPcmDataOffset;
    }
    *buffer = AsGtlRevolutionSTransFilter(self)->buffer;
    return 1;
}

} // extern "C"
