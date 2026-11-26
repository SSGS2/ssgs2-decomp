// Revolution split filter: the constructor and SplitInput, which divide one input packet
// into two output samples that share its two payload regions.

#include "classes/GtlMediaSample.h"
#include "classes/gtl_CRevolutionSplitFilter.h"
#include "classes/GtlMediaFormat.h"

enum {
    kSplitOutputReady = 4,
    kSplitPacketPrefixBytes = 8
};
// Native-endian packet: opaque 8-byte prefix, then two byte lengths at +8/+C.
// Payload begins at 8 + 4 * format.splitPacketHeaderWordCount, NOT sizeof(SplitPacket).
// A valid format must include the length words in that header and the packet must
// contain both payload ranges; neither the header nor the buffer size is checked here.
struct SplitPacket {
    char prefix[8];
    int primaryBytes, secondaryBytes;
};

extern "C" {

extern void __vt__Q23gtl22CRevolutionSplitFilter();

// Share the packet's two payload regions with the output samples without copying.
// inputSample and both outputs must be valid. Outputs borrow consecutive byte ranges
// in the input buffer; its storage must survive their use. Getter/setter errors are
// ignored and the method always returns 4, the base Process method's ready result.
} // extern "C"

// Construct the base splitter and install the Revolution-specific virtual table.
// outputFormat is borrowed by both output sample wrappers and must outlive them.
// baseArgument is forwarded unchanged; the current gtl::IFilter::Init ignores it.
// The factory that creates this object leaves baseArgument unset.
gtl::CRevolutionSplitFilter::CRevolutionSplitFilter(void* outputFormat, void* baseArgument) {
    Init(outputFormat, baseArgument);
    void* vtable = (void*)__vt__Q23gtl22CRevolutionSplitFilter;
    this->vtable = vtable;
}

int gtl::CRevolutionSplitFilter::SplitInput(void* inputSample, void* primaryOutput, void* secondaryOutput) {
    int sourceAddress;
    int headerWordCount;
    char* payload;
    GtlMediaFormat format;
    ((gtl::CMediaSample*)inputSample)->GetData((int*)&sourceAddress);
    SplitPacket* packet = (SplitPacket*)sourceAddress;
    ((gtl::CMediaSample*)inputSample)->CopyFormat(&format);
    // Locate the first payload using the format's word count, including length words.
    headerWordCount = format.splitPacketHeaderWordCount; // Number of words between the prefix and payload.
    payload = (char*)packet + kSplitPacketPrefixBytes + headerWordCount * sizeof(int);
    // Publish two views into the same packet; no ownership transfer or allocation.
    // The two payload lengths are the file's big-endian words.
    ((gtl::CMediaSample*)primaryOutput)->SetBuffer((int)payload, packet->primaryBytes);
    ((gtl::CMediaSample*)secondaryOutput)->SetBuffer((int)(payload + packet->primaryBytes), packet->secondaryBytes);
    return kSplitOutputReady;
}
