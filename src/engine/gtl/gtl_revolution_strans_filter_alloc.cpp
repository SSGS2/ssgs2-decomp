// Revolution S-transform filter, buffer setup: AllocateBuffers hands back the buffer and its size.

#include "classes/GtlRevolutionSTransFilter.h"

extern "C" {

void* gtl_Alloc(unsigned int size);

enum {
    kTwoBuffersShift = 1,
    kDataOffsetWordByteOffset = 4,
    kPcmDataOffset = 0x20
};

} // extern "C"

// Allocate two adjacent THP PCM work buffers and publish their shared storage.
// buffer/size are required outputs (bytes). The filter owns the allocation;
// callers borrow it until filter cleanup. Nonzero segmentBytes must allow its
// 0x20-byte header plus PCM payload. Allocation failure is unchecked, and repeat
// calls with nonzero size overwrite the previous allocation without freeing it.
// A zero size returns the existing buffer, leaving the filter unchanged. Return
// 1 always; this is not an allocation-success test.
int gtl::CRevolutionSTransFilter::AllocateBuffers(void** buffer, unsigned int* size) {
    *size = segmentBytes << kTwoBuffersShift;
    *buffer = 0;
    if (*size != 0) {
        // One allocation owns both segments.
        allocation = gtl_Alloc(*size);
        buffers[0] = (STransBuffer*)allocation;
        // Header word +4 is the PCM data offset from that segment's base, consumed when decoding.
        *(unsigned int*)((char*)allocation + kDataOffsetWordByteOffset) = kPcmDataOffset;
        buffers[1] = (STransBuffer*)((char*)allocation + segmentBytes);
        *(unsigned int*)((char*)buffers[1] + kDataOffsetWordByteOffset) = kPcmDataOffset;
    }
    *buffer = allocation;
    return 1;
}
