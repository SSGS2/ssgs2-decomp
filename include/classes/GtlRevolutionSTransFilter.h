// gtl::CRevolutionSTransFilter: decodes THP audio into two alternating PCM segments that share one allocation.
#ifndef CLASSES_GTLREVOLUTIONSTRANSFILTER_H
#define CLASSES_GTLREVOLUTIONSTRANSFILTER_H

#include "classes/gtl_CTransformFilter.h"

namespace gtl {

// Header of one PCM segment, followed by the decoded samples.
struct STransBuffer {
    unsigned int decoderResult; // 0x0: return word of the decoder
    int dataOffset;             // 0x4: bytes from this header to the PCM data
};

class CRevolutionSTransFilter : public CTransformFilter {
public:
    // Set up the filter for its output sample; returns this.
    CRevolutionSTransFilter* Init(void* outputSample);
    // Free the segment allocation and the base; frees this when deleting is positive. Returns this.
    void* Destroy(int deleting);

    // Allocate both segments for segmentBytes each and return the allocation.
    int AllocateBuffers(void** buffer, unsigned int* size);
    // Decode an input sample into the next segment and publish it.
    int DecodeAudioSample(void* inputSample, void* outputSample);

    unsigned int segmentBytes; // 0x1C: bytes per segment, including its header
    void* allocation;          // 0x20: both segments live in this block
    STransBuffer* buffers[2];  // 0x24, 0x28: the two segments
    signed char currentBuffer; // 0x2C: 0 or 1, toggled after each decode
};

#define GTLREVOLUTIONSTRANSFILTER_OFF(f, o) \
    typedef char GtlRevolutionSTransFilter_off_##f[((int)&((CRevolutionSTransFilter*)0)->f == (o)) ? 1 : -1]
GTLREVOLUTIONSTRANSFILTER_OFF(segmentBytes, 0x1C);
GTLREVOLUTIONSTRANSFILTER_OFF(allocation, 0x20);
GTLREVOLUTIONSTRANSFILTER_OFF(buffers, 0x24);
GTLREVOLUTIONSTRANSFILTER_OFF(currentBuffer, 0x2C);
#undef GTLREVOLUTIONSTRANSFILTER_OFF
typedef char GtlRevolutionSTransFilter_size[(sizeof(CRevolutionSTransFilter) == 0x30) ? 1 : -1];

} // namespace gtl

#endif
