// Revolution STrans audio decoding: CRevolutionSTransFilter::DecodeAudioSample decodes THP
// audio into the next of two alternating PCM segments.

#include "classes/GtlMediaSample.h"
#include "classes/GtlRevolutionSTransFilter.h"

extern "C" {

unsigned int THPAudioDecode(void*, void*, int);

enum AudioTransformResult {
    kMissingSample = 2,
    kSampleDecoded = 4
};
// The decoder's third-argument mode is fixed at zero; its SDK meaning is not inferred here.
enum AudioDecodeConstants {
    kThpDecodeMode = 0,
    kAlternateBufferBit = 1
};

} // extern "C"

// Decode THP audio into the next PCM segment, publish it, then alternate segments.
// Return 2 for a missing sample without advancing; otherwise publish and return 4.
// Requires initialized segments and a valid THP payload. No decoder failure is tested.
// input/output are borrowed; the output aliases filter-owned memory, reused two
// successful calls later. The output includes its header, not just the PCM payload.
int gtl::CRevolutionSTransFilter::DecodeAudioSample(void* inputSample, void* outputSample) {
    if (inputSample == 0 || outputSample == 0) {
        return kMissingSample;
    }
    // Decode to PCM after the selected header; keep the decoder return in its first word.
    void* encodedData;
    ((CMediaSample*)inputSample)->GetData((int*)&encodedData);
    STransBuffer* decodeBuffer = buffers[currentBuffer];
    decodeBuffer->decoderResult =
        THPAudioDecode((char*)decodeBuffer + decodeBuffer->dataOffset, encodedData, kThpDecodeMode);
    // Publish before toggling, so output and decode refer to the same segment.
    ((CMediaSample*)outputSample)->SetBuffer((int)buffers[currentBuffer], segmentBytes);
    currentBuffer ^= kAlternateBufferBit;
    return kSampleDecoded;
}
