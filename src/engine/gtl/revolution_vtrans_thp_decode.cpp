// Translation unit 0x803FA2E4-0x803FA3F0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

namespace gtl {
struct CRevolutionVTransFilter {
    char filterBase[0x3c];
    // Caller-provided planar output/work addresses, not offsets into the input.
    // Plane dimensions and workspace sizing are established outside this method.
    unsigned int yPlane;
    unsigned int uPlane;
    unsigned int vPlane;
    unsigned int decodeWork;
};
}

extern "C" {
void LCEnable();
void gtl_CMediaSample_GetData(void*, unsigned int*);
int THPVideoDecode(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
void OSReport(const char*, ...);
extern char videoDecodeErrorStrings[];

struct VideoDecodeErrorPool {
    char decoderResult[0x21];
    char inputAddress[0xB];
    char yPlaneAddress[0xB];
    char uPlaneAddress[0xB];
    char vPlaneAddress[0xB];
    char workAddress[0xB];
    char separator[1];
};
enum { kNoInputSample = 2, kFrameDecoded = 4, kDecodeFailed = 7 };

// Decode one THP frame into planar YUV output while the locked cache is enabled.
// input is a borrowed compressed sample; the third argument is only tested for
// zero here (its wider interface meaning is unresolved). No reference is retained.
// Returns 2 for absent input/argument, 4 for success, 7 for a decoder error.
int gtl_CRevolutionVTransFilter_DecodeInputSample(gtl::CRevolutionVTransFilter* self, void* input, int inputArgumentPresent)
{
    if (!input || !inputArgumentPresent) return kNoInputSample;
    // Phase 1: decode into the preconfigured Y/U/V planes with locked cache on.
    LCEnable();
    unsigned int data;
    gtl_CMediaSample_GetData(input, &data);
    int result = THPVideoDecode(data, self->yPlane, self->uPlane, self->vPlane, self->decodeWork);
    LCDisable();
    // Phase 2: report decoder status and every address after disabling the cache.
    if (result) {
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->decoderResult, result);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->inputAddress, data);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->yPlaneAddress, self->yPlane);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->uPlaneAddress, self->uPlane);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->vPlaneAddress, self->vPlane);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->workAddress, self->decodeWork);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->separator);
        return kDecodeFailed;
    }
    return kFrameDecoded;
}

} // extern "C"
