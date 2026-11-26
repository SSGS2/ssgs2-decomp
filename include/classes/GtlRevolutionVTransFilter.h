// gtl::CRevolutionVTransFilter: decodes THP video into planar YUV 4:2:0 frames held in one owned allocation.
#ifndef CLASSES_GTLREVOLUTIONVTRANSFILTER_H
#define CLASSES_GTLREVOLUTIONVTRANSFILTER_H

#include "classes/gtl_CTransformFilter.h"

namespace gtl {

class CRevolutionVTransFilter : public CTransformFilter {
public:
    // Set up the THP video transform for a borrowed output sample; returns this.
    CRevolutionVTransFilter* Init(void* outputSample);
    // Free the decoder and frame allocations and the base; frees this when deleting is positive. Returns this.
    void* Destroy(int deleting);

    // Allocate decoder scratch and one frame holding the header and the Y, U, V planes.
    int AllocateFrameBuffers(unsigned char** outFrameBuffer, unsigned char** outFrameSize);
    // Decode one compressed sample into the planes.
    int DecodeInputSample(void* input, int inputArgumentPresent);

    unsigned int frameHeaderSize; // 0x1C
    unsigned int yPlaneOffset;    // 0x20
    unsigned short yPlaneWidth;   // 0x24
    unsigned short yPlaneHeight;  // 0x26
    unsigned int uPlaneOffset;    // 0x28
    unsigned short uPlaneWidth;   // 0x2C
    unsigned short uPlaneHeight;  // 0x2E
    unsigned int vPlaneOffset;    // 0x30
    unsigned short vPlaneWidth;   // 0x34
    unsigned short vPlaneHeight;  // 0x36
    char pad_038[0x4];            // 0x38
    unsigned char* yPlane;        // 0x3C: inside frameBuffer
    unsigned char* uPlane;        // 0x40: inside frameBuffer
    unsigned char* vPlane;        // 0x44: inside frameBuffer
    void* decoderWorkBuffer;      // 0x48
    void* frameBuffer;            // 0x4C
};

#define GTLREVOLUTIONVTRANSFILTER_OFF(f, o) \
    typedef char GtlRevolutionVTransFilter_off_##f[((int)&((CRevolutionVTransFilter*)0)->f == (o)) ? 1 : -1]
GTLREVOLUTIONVTRANSFILTER_OFF(frameHeaderSize, 0x1C);
GTLREVOLUTIONVTRANSFILTER_OFF(yPlaneWidth, 0x24);
GTLREVOLUTIONVTRANSFILTER_OFF(uPlaneOffset, 0x28);
GTLREVOLUTIONVTRANSFILTER_OFF(yPlane, 0x3C);
GTLREVOLUTIONVTRANSFILTER_OFF(frameBuffer, 0x4C);
#undef GTLREVOLUTIONVTRANSFILTER_OFF
typedef char GtlRevolutionVTransFilter_size[(sizeof(CRevolutionVTransFilter) == 0x50) ? 1 : -1];

} // namespace gtl

#endif // CLASSES_GTLREVOLUTIONVTRANSFILTER_H
