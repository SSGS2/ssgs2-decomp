// Revolution media sampling filter: assembles one output sample from queued input packets.
// CRevolutionSamplingFilter::FillOutputSample copies queued bytes into the output and reads
// the next request size from the packet header.

#include "classes/GtlMediaSample.h"
#include "classes/gtl_CRevolutionSamplingFilter.h"
#include "sdk_protos.h"
// Revolution media packet assembly. The packet's first big-endian u32 gives the length of the NEXT
// packet, header included. This consumer does not swap bytes or write that header; the upstream
// producer provides it. completedBytes is this packet's size, requestedBytes the next one.
#pragma optimize_for_size off
#pragma use_lmw_stmw on

struct SamplingBuffer {
    // Existing queue reference is consumed by the caller; prepend retains its own reference.
    int references;
    virtual ~SamplingBuffer();
};

enum SamplingResult {
    kSamplingOutputReady = 4,
    kSamplingNeedsInput = 5
};

extern "C" {
void* memcpy(void*, const void*, unsigned long);
} // extern "C"

// Assemble one output sample from queued input and read the next byte count from its header.
// output is a borrowed media sample whose data buffer holds at least max(requestedBytes, 4) bytes
// and is 4-byte aligned, because the first u32 is read after completion. Requests smaller than 4
// bytes leave header bytes untouched, so the caller must preinitialize them. A zero-byte request
// still dequeues input before reading the existing header. Queue entries and header values are
// trusted, not validated.
// Returns 4 (kSamplingOutputReady) after completion or 5 (kSamplingNeedsInput) if the queue
// empties; a 5 may follow input consumption and partial output writes. The copy cursor is not saved.
int gtl::CRevolutionSamplingFilter::FillOutputSample(void* output) {
    unsigned int nextRequestedBytes;
    unsigned int requestedBytes;
    char* outputBytes;
    char* inputBytes;
    unsigned int copyBytes;
    unsigned int remainingBytes;
    unsigned int copiedBytes;
    char* destination;
    ((gtl::CMediaSample*)output)->GetData((int*)&outputBytes);
    destination = outputBytes;
    SamplingBuffer* inputSample = 0;
    copiedBytes = 0;
    // Drain whole input tails, or requeue the unused tail at the queue front.
    do {
        remainingBytes = this->requestedBytes - copiedBytes;
        unsigned int consumedBytes;
        if (!DequeueInputSample(&inputSample, &consumedBytes))
            return kSamplingNeedsInput;
        ((gtl::CMediaSample*)inputSample)->GetData((int*)&inputBytes);
        unsigned int inputSizeBytes;
        ((gtl::CMediaSample*)inputSample)->GetSize((int*)&inputSizeBytes);
        copyBytes = inputSizeBytes - consumedBytes;
        char* inputStart = inputBytes;
        if (remainingBytes >= copyBytes) {
            memcpy(destination, inputStart + consumedBytes, copyBytes);
            RecycleReturnedSample();
        } else {
            memcpy(destination, inputStart + consumedBytes, remainingBytes);
            PrependInputSample((unsigned int*)inputSample, consumedBytes + remainingBytes);
            copyBytes = remainingBytes;
        }
        copiedBytes += copyBytes;
        destination += copyBytes;
        // Queue removal transfers its existing reference; a partial requeue retained another.
        SamplingBuffer* sampleToRelease = inputSample;
        if (sampleToRelease) {
            if (--sampleToRelease->references == 0) {
                delete sampleToRelease;
            }
            inputSample = 0;
        }
    } while (copiedBytes != (requestedBytes = this->requestedBytes));
    // Commit the completed size, then take the next request from the packet header.
    nextRequestedBytes = *(unsigned int*)outputBytes;
    completedBytes = requestedBytes;
    this->requestedBytes = nextRequestedBytes;
    return kSamplingOutputReady;
}
