// Translation unit 0x803F98EC-0x803F9A2C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
// Revolution media packet assembly; no gameplay tuning knobs.
// API/producer map: CSamplingFilter_DequeueInputSample supplies retained sample
// tails and byte offsets; CMediaSample_GetData/GetSize expose their byte ranges.
// This override copies a requested packet across those tails into output.
// The packet's first native-endian u32 (big-endian on Wii) supplies the NEXT
// packet length, including its own header. This consumer never swaps bytes or
// writes that header: the upstream media-byte producer must provide it.
// completedBytes describes this packet, requestedBytes the next. The initial
// requestedBytes is seeded by CRevolutionSamplingFilter_Ctor from owner+0x94
// (gtl_revolution_sampling_filter_part1.cpp); vfunc_4 restores that seed. vfunc_10 (gtl_revolution_sampling_filter_part2.cpp) exposes the next
// request size to output allocation. Keep the producer/allocator/framing together.
// Use vfunc_9 for framing changes; keep upstream header encoding in agreement.
#pragma optimize_for_size off
#pragma use_lmw_stmw on

struct SamplingBuffer {
    // Existing queue reference is consumed by the caller; prepend retains its own reference.
    int references;
    virtual ~SamplingBuffer();
};

enum SamplingResult { kSamplingOutputReady = 4, kSamplingNeedsInput = 5 };

namespace gtl {
struct CRevolutionSamplingFilter {
    char samplingFilterBase[0x38];
    unsigned int requestedBytes; // +0x38: current output byte count, replaced by the output header.
    unsigned int completedBytes; // +0x3c: byte count of the last completed output.
};
}

extern "C" {
void gtl_CMediaSample_GetData(void*, void*);
void gtl_CMediaSample_GetSize(void*, unsigned int*);
int gtl_CSamplingFilter_DequeueInputSample(gtl::CRevolutionSamplingFilter*, SamplingBuffer**, unsigned int*);
void gtl_CSamplingFilter_RecycleReturnedSample(gtl::CRevolutionSamplingFilter*);
void gtl_CSamplingFilter_PrependInputSample(gtl::CRevolutionSamplingFilter*, SamplingBuffer*, unsigned int);
void* memcpy(void*, const void*, unsigned long);

// Assemble one output sample, requeue a partial input, and read the next byte count from its header.
// output is a required borrowed MEDIA SAMPLE, whose GetData buffer must have
// capacity >= max(self->requestedBytes,sizeof(u32)==4) and a 4-byte-aligned
// GetData address: completion dereferences its first u32. Normal packets require
// requestedBytes>=4 to assemble a complete header. Smaller requests leave some
// or all header bytes untouched: the caller must preinitialize those bytes so
// the resulting first word is a valid next length. A zero-byte request still
// DEQUEUES input (and may requeue it) before reading the preexisting header;
// it requires a queued sample and initialized header, and is not an empty no-op.
// Every produced next length must satisfy the same capacity/header rules.
// Queue entries must have live references and valid
// consumed-byte offsets. Byte counts and header values are trusted, not validated.
// Returns 4 after completion or 5 if the queue empties; a 5 may follow destructive
// input consumption and partial output writes. The local copy cursor is not saved.
int gtl_CRevolutionSamplingFilter_FillOutputSample(gtl::CRevolutionSamplingFilter* self, void* output) {
    unsigned int nextRequestedBytes;
    unsigned int requestedBytes;
    char* outputBytes;
    char* inputBytes;
    unsigned int copyBytes;
    unsigned int remainingBytes;
    unsigned int copiedBytes;
    char* destination;
    gtl_CMediaSample_GetData(output, &outputBytes);
    destination = outputBytes;
    SamplingBuffer* inputSample = 0;
    copiedBytes = 0;
    // Phase 1: drain whole input tails or retain the unused tail at the queue front.
    do {
        remainingBytes = self->requestedBytes - copiedBytes;
        unsigned int consumedBytes;
        if (!gtl_CSamplingFilter_DequeueInputSample(self, &inputSample, &consumedBytes))
            return kSamplingNeedsInput;
        gtl_CMediaSample_GetData(inputSample, &inputBytes);
        unsigned int inputSizeBytes;
        gtl_CMediaSample_GetSize(inputSample, &inputSizeBytes);
        copyBytes = inputSizeBytes - consumedBytes;
        char* inputStart = inputBytes;
        if (remainingBytes >= copyBytes) {
            memcpy(destination, inputStart + consumedBytes, copyBytes);
            gtl_CSamplingFilter_RecycleReturnedSample(self);
        } else {
            memcpy(destination, inputStart + consumedBytes, remainingBytes);
            gtl_CSamplingFilter_PrependInputSample(self, inputSample, consumedBytes + remainingBytes);
            copyBytes = remainingBytes;
        }
        copiedBytes += copyBytes;
        destination += copyBytes;
        // Queue removal transfers its existing reference; partial requeue retained another.
        SamplingBuffer* sampleToRelease = inputSample;
        if (sampleToRelease) {
            if (--sampleToRelease->references == 0) {
                delete sampleToRelease;
            }
            inputSample = 0;
        }
    } while (copiedBytes != (requestedBytes = self->requestedBytes));
    // Phase 2: commit the completed size, then use its first word for the next request.
// The packet header is the file's big-endian word.
    nextRequestedBytes = *(unsigned int*)outputBytes;
    self->completedBytes = requestedBytes;
    self->requestedBytes = nextRequestedBytes;
    return kSamplingOutputReady;
}

} // extern "C"
