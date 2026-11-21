// Translation unit 0x803F9A9C-0x803F9C24 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/gtl_CSourceFilter.h"
extern "C" {

struct CRevolutionSourceFilter {
    gtl_CSourceFilter base;
    unsigned int submittedReadBytes;
};

struct DVDFileInfo {
    char commandBlock[0x30];
    unsigned int startAddress;
    unsigned int length;
};

void gtl_CSourceFilter_Ctor(void*, void*, unsigned int);
int gtl_CSourceFilter_IsReadPending(void*);
void gtl_CMediaSample_GetData(void*, unsigned int*);
void gtl_CSourceFilter_GetReadOffset(void*, unsigned int*);
void gtl_CSourceFilter_MarkReadPending(void*);
void gtl_CSourceFilter_ClearReadPending(void*);
int DVDGetCommandBlockStatus(void*);
int DVDReadAsyncPrio(DVDFileInfo*, void*, int, int, void*, int);
void __vt__Q23gtl23CRevolutionSourceFilter();
typedef char RevolutionSource_bytesReadOffset[((int)&((CRevolutionSourceFilter*)0)->submittedReadBytes == 0x2C) ? 1 : -1];
typedef char DvdInfo_lengthOffset[((int)&((DVDFileInfo*)0)->length == 0x34) ? 1 : -1];

enum { kDvdAlignment = 32, kDvdPriority = 2, kReadPending = 1, kReadComplete = 4, kReadError = 7 };

// Initialize the DVD source filter and clear the last submitted byte count.
void* gtl_CRevolutionSourceFilter_Ctor(void* self, void* source, unsigned int callback) {
    gtl_CSourceFilter_Ctor(self, source, callback);
    ((CRevolutionSourceFilter*)self)->submittedReadBytes = 0;
    ((gtl_CSourceFilter*)self)->vtable = (void*)__vt__Q23gtl23CRevolutionSourceFilter;
    return self;
}

// Report the byte count of the most recently submitted DVD read.
// outSubmittedBytes must be writable; this is a submission length, not hardware progress.
int gtl_CRevolutionSourceFilter_GetSubmittedReadBytes(CRevolutionSourceFilter* self, unsigned int* outSubmittedBytes) {
    *outSubmittedBytes = self->submittedReadBytes;
    return 1;
}

// Submit one sample read, or poll the pending read until the DVD status becomes zero.
// Returns 1=pending, 4=complete, 7=bad arguments/submission failure. requestedBytes is
// input-only; actual submitted length is exposed by vfunc_10. file/sample are borrowed
// and must remain valid through completion. The destination must have room for the
// 32-byte rounded request, not just *requestedBytes, and satisfy DVD buffer alignment.
// Require request/rounded size to fit a nonnegative int and readOffset <= file length;
// these arithmetic preconditions are unchecked here.
int gtl_CRevolutionSourceFilter_SubmitOrPollDvdRead(CRevolutionSourceFilter* self, DVDFileInfo* file,
                                        void* sample, unsigned int* requestedBytes) {
    if (!file || !sample || !requestedBytes)
        return kReadError;
    if (!gtl_CSourceFilter_IsReadPending(self)) {
        // Obtain the sample's storage and round the requested transfer up to 32 bytes.
        unsigned int dataAddress;
        gtl_CMediaSample_GetData(sample, &dataAddress);
        int requested = (int)*requestedBytes;
        int remainder = requested % kDvdAlignment;
        unsigned int transferBytes = requested + (kDvdAlignment - remainder);
        if (remainder == 0)
            transferBytes = requested;
        unsigned int readOffset;
        gtl_CSourceFilter_GetReadOffset(self, &readOffset);
        // Clamp at EOF after rounding: the final transfer length may be unaligned.
        unsigned int available = file->length - readOffset;
        if (available < transferBytes)
            transferBytes = available;
        if (!DVDReadAsyncPrio(file, (void*)dataAddress, (int)transferBytes, (int)readOffset, 0, kDvdPriority))
            return kReadError;
        self->submittedReadBytes = transferBytes;
        gtl_CSourceFilter_MarkReadPending(self);
    }
    // A nonzero status, including hardware error states, remains pending here.
    // Polling uses this call's file, so callers must keep the same handle while pending.
    int readComplete = 0;
    if (!DVDGetCommandBlockStatus(file))
        readComplete = 1;
    int result = kReadPending;
    if (readComplete) {
        gtl_CSourceFilter_ClearReadPending(self);
        result = kReadComplete;
    }
    return result;
}

} // extern "C"
