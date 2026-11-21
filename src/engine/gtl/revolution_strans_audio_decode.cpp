// Translation unit 0x803F9F60-0x803FA01C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

namespace gtl {
struct STransBuffer {
    unsigned int decoderResult; // Decoder return word, not the output sample's byte size.
    int dataOffset;    // Bytes relative to this header; allocation setup writes 0x20.
};

struct CRevolutionSTransFilter {
    char baseFilterState[0x1c];
    unsigned int outputSegmentBytes; // Published bytes per segment, including its 0x20-byte header.
    void* allocation; // Both alternating buffers occupy this allocation.
    STransBuffer* buffers[2];
    signed char currentBuffer; // Must be 0 or 1; XOR 1 rotates the two segment aliases.
};
}

extern "C" {

void gtl_CMediaSample_GetData(void*, void**);
unsigned int THPAudioDecode(void*, void*, int);
void gtl_CMediaSample_SetBuffer(void*, void*, unsigned int);

enum AudioTransformResult { kMissingSample = 2, kSampleDecoded = 4 };
// The decoder's third-argument mode is fixed at zero; its SDK meaning is not inferred here.
enum AudioDecodeConstants { kThpDecodeMode = 0, kAlternateBufferBit = 1 };

// Decode THP audio into the next PCM buffer, publish it, then alternate buffers.
// Return 2 for a missing sample without advancing; otherwise publish and return 4.
// Requires initialized buffers and a valid THP payload. No decoder failure is tested.
// input/output are borrowed; the output aliases filter-owned memory, reused two
// successful calls later. The output includes its header, not just the PCM payload.
int gtl_CRevolutionSTransFilter_DecodeAudioSample(gtl::CRevolutionSTransFilter* self, void* inputSample, void* outputSample) {
    if (inputSample == 0 || outputSample == 0) {
        return kMissingSample;
    }
    // Decode to PCM after the selected header; keep the decoder return in its first word.
    void* encodedData;
    gtl_CMediaSample_GetData(inputSample, &encodedData);
    gtl::STransBuffer* decodeBuffer = self->buffers[self->currentBuffer];
    decodeBuffer->decoderResult = THPAudioDecode((char*)decodeBuffer + decodeBuffer->dataOffset, encodedData, kThpDecodeMode);
    // Publish before toggling, so output and decode refer to the same segment.
    gtl_CMediaSample_SetBuffer(outputSample, self->buffers[self->currentBuffer], self->outputSegmentBytes);
    self->currentBuffer ^= kAlternateBufferBit;
    return kSampleDecoded;
}

} // extern "C"
