// Revolution source filter: reads samples from a DVD file. It submits a read, polls it until it
// completes, and reports how many bytes were submitted.

#include "classes/GtlMediaSample.h"
#include "classes/gtl_CRevolutionSourceFilter.h"

struct DVDFileInfo {
    char commandBlock[0x30];
    unsigned int startAddress;
    unsigned int length;
};

extern "C" {

int DVDGetCommandBlockStatus(void*);
int DVDReadAsyncPrio(DVDFileInfo*, void*, int, int, void*, int);
void __vt__Q23gtl23CRevolutionSourceFilter();
typedef char DvdInfo_lengthOffset[((int)&((DVDFileInfo*)0)->length == 0x34) ? 1 : -1];

enum {
    kDvdAlignment = 32,
    kDvdPriority = 2,
    kReadPending = 1,
    kReadComplete = 4,
    kReadError = 7
};

} // extern "C"

namespace gtl {

// Initialize the DVD source filter and clear the last submitted byte count.
void* CRevolutionSourceFilter::Init(void* source, unsigned int callback) {
    CSourceFilter::Init((const SourceData*)source, (void*)callback);
    submittedReadBytes = 0;
    vtable = (void*)__vt__Q23gtl23CRevolutionSourceFilter;
    return this;
}

// Submit one sample read, or poll the pending read until the DVD status becomes zero.
// Returns 1=pending, 4=complete, 7=bad arguments/submission failure. requestedBytes is
// input-only; actual submitted length is exposed by the submitted-bytes getter. file/sample are borrowed
// and must remain valid through completion. The destination must have room for the
// 32-byte rounded request, not just *requestedBytes, and satisfy DVD buffer alignment.
// Require request/rounded size to fit a nonnegative int and readOffset <= file length;
// these arithmetic preconditions are unchecked here.
int CRevolutionSourceFilter::SubmitOrPollDvdRead(DVDFileInfo* file, void* sample, unsigned int* requestedBytes) {
    if (!file || !sample || !requestedBytes)
        return kReadError;
    if (!IsReadPending()) {
        // Obtain the sample's storage and round the requested transfer up to 32 bytes.
        unsigned int dataAddress;
        ((CMediaSample*)sample)->GetData((int*)&dataAddress);
        int requested = (int)*requestedBytes;
        int remainder = requested % kDvdAlignment;
        unsigned int transferBytes = requested + (kDvdAlignment - remainder);
        if (remainder == 0)
            transferBytes = requested;
        unsigned int readOffset;
        GetReadOffset(&readOffset);
        // Clamp at EOF after rounding: the final transfer length may be unaligned.
        unsigned int available = file->length - readOffset;
        if (available < transferBytes)
            transferBytes = available;
        if (!DVDReadAsyncPrio(file, (void*)dataAddress, (int)transferBytes, (int)readOffset, 0, kDvdPriority))
            return kReadError;
        submittedReadBytes = transferBytes;
        MarkReadPending();
    }
    // A nonzero status, including hardware error states, remains pending here.
    // Polling uses this call's file, so callers must keep the same handle while pending.
    int readComplete = 0;
    if (!DVDGetCommandBlockStatus(file))
        readComplete = 1;
    int result = kReadPending;
    if (readComplete) {
        ClearReadPending();
        result = kReadComplete;
    }
    return result;
}

// Report the byte count of the most recently submitted DVD read.
// outSubmittedBytes must be writable; this is a submission length, not hardware progress.
int CRevolutionSourceFilter::GetSubmittedReadBytes(unsigned int* outSubmittedBytes) {
    *outSubmittedBytes = submittedReadBytes;
    return 1;
}

} // namespace gtl
