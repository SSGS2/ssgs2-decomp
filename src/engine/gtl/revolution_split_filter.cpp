#include "classes/gtl_CSplitFilter.h"
#include "classes/GtlMediaFormat.h"

enum { kSplitOutputReady = 4, kSplitPacketPrefixBytes = 8 };
// Native-endian packet: opaque 8-byte prefix, then two byte lengths at +8/+C.
// Payload begins at 8 + 4 * format.splitPacketHeaderWordCount, NOT sizeof(SplitPacket).
// A valid format must include the length words in that header and the packet must
// contain both payload ranges; neither the header nor the buffer size is checked here.
struct SplitPacket { char prefix[8]; int primaryBytes, secondaryBytes; };

// Translation unit 0x803F9C84-0x803F9D60 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {



extern void __vt__Q23gtl22CRevolutionSplitFilter();
extern void* gtl_CSplitFilter_Ctor(void*, void*, void*);
extern int gtl_CMediaSample_GetData(void*, int*);
extern int gtl_CMediaSample_CopyFormat(void*, void*);
extern int gtl_CMediaSample_SetBuffer(void*, int, int);

// Construct the base splitter and install the Revolution-specific virtual table.
// outputFormat is borrowed by both output sample wrappers and must outlive them.
// baseArgument is forwarded unchanged; the current gtl_IFilter_Ctor ignores it.
void gtl_CRevolutionSplitFilter_Ctor(gtl_CSplitFilter* self, void* outputFormat, void* baseArgument) {
	gtl_CSplitFilter_Ctor(self, outputFormat, baseArgument);
	void* vtable = (void*)__vt__Q23gtl22CRevolutionSplitFilter;
	self->vtable = vtable;
}

// Share the packet's two payload regions with the output samples without copying.
// inputSample and both outputs must be valid. Outputs borrow consecutive byte ranges
// in the input buffer; its storage must survive their use. Getter/setter errors are
// ignored and the method always returns 4, the base Process method's ready result.
int gtl_CRevolutionSplitFilter_SplitInput(void* self, void* inputSample, void* primaryOutput, void* secondaryOutput) {
	int sourceAddress;
	int headerWordCount;
	char* payload;
	GtlMediaFormat format;
	gtl_CMediaSample_GetData(inputSample, &sourceAddress);
	SplitPacket* packet = (SplitPacket*)sourceAddress;
	gtl_CMediaSample_CopyFormat(inputSample, &format);
	// Locate the first payload using the format's word count, including length words.
	headerWordCount = format.splitPacketHeaderWordCount; // Number of words between the prefix and payload.
	payload = (char*)packet + kSplitPacketPrefixBytes + headerWordCount * sizeof(int);
	// Publish two views into the same packet; no ownership transfer or allocation.
// The two payload lengths are the file's big-endian words.
	gtl_CMediaSample_SetBuffer(primaryOutput, (int)payload, packet->primaryBytes);
	gtl_CMediaSample_SetBuffer(secondaryOutput, (int)(payload + packet->primaryBytes), packet->secondaryBytes);
	return kSplitOutputReady;
}

} // extern "C"
