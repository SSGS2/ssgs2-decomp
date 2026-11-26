// Revolution VTrans video filter: CRevolutionVTransFilter::DecodeInputSample decodes one THP
// video frame into planar Y, U and V planes and reports decoder errors.

#include "classes/GtlMediaSample.h"
#include "classes/GtlRevolutionVTransFilter.h"
#include "sdk_protos.h"

extern "C" {
void LCEnable();
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
enum {
    kNoInputSample = 2,
    kFrameDecoded = 4,
    kDecodeFailed = 7
};

} // extern "C"

// Decode one THP frame into planar YUV output while the locked cache is enabled.
// input is a borrowed compressed sample; the second argument is only tested for
// zero here (its wider interface meaning is not inferred). The filter keeps no reference to it.
// Returns 2 for absent input/argument, 4 for success, 7 for a decoder error.
int gtl::CRevolutionVTransFilter::DecodeInputSample(void* input, int inputArgumentPresent) {
    if (!input || !inputArgumentPresent)
        return kNoInputSample;
    // Phase 1: decode into the preconfigured Y/U/V planes with locked cache on.
    LCEnable();
    unsigned int data;
    ((CMediaSample*)input)->GetData((int*)&data);
    int result = THPVideoDecode(data, (unsigned int)yPlane, (unsigned int)uPlane, (unsigned int)vPlane,
                                (unsigned int)decoderWorkBuffer);
    LCDisable();
    // Phase 2: report decoder status and every address after disabling the cache.
    if (result) {
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->decoderResult, result);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->inputAddress, data);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->yPlaneAddress, yPlane);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->uPlaneAddress, uPlane);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->vPlaneAddress, vPlane);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->workAddress, decoderWorkBuffer);
        OSReport(((VideoDecodeErrorPool*)videoDecodeErrorStrings)->separator);
        return kDecodeFailed;
    }
    return kFrameDecoded;
}
