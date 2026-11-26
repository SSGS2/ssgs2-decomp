// GtlFileIoDeviceDVD::SubmitSeekWithCallback: submits a DVD seek and records the caller's completion
// callback once the seek has been accepted.

#include "classes/DvdDeviceFileState.h"
#include "classes/GtlFileIoDeviceDVD.h"
extern "C" {

struct DVDContext {
    // Borrowed file-handle extension; +0x50 is the shifted SDK priority,
    // +0x54/+0x58 hold the completion function address and opaque argument.
    char fileStateStorage[0x50]; // File object prefix, including callbackContext at +0x2C.
    int seekPriority;
    int callback;
    int userData;
};

void File_SetLastError(int);
int DVDSeek(void*, int, void*, int);

enum DvdSeekConstants {
    kInvalidFileHandle = -1,
    kDefaultSeekPriorityArgument = -1,
    kDefaultSeekPriorityValue = 0xA0,
    kSeekPriorityShift = 6,
    kFileInvalidHandle = 6,
    kFileSeekSubmissionFailed = 0x19,
    kFileOk = 0
};

} // extern "C"

// Submit a DVD seek, recording its completion callback only after successful submission.
// file is a live DVD context address or the invalid sentinel; offset is bytes.
// priorityArgument uses its upper bits (>>6); -1 selects 0xA0, hence SDK priority 2.
// Return 1 means accepted, not completed; 0 sets last error to 6 or 0x19.
// callback receives (fileError, resultOrZero, context, userData). The caller keeps
// the file context alive until completion. unused is ignored.
int gtl::CFileIoDeviceDVD::SubmitSeekWithCallback(unsigned int file, int offset, int unused,
                                                  unsigned int priorityArgument, int callback, int userData) {
    if (file == (unsigned int)kInvalidFileHandle) {
        File_SetLastError(kFileInvalidHandle);
        return 0;
    }
    if (priorityArgument == (unsigned int)kDefaultSeekPriorityArgument)
        priorityArgument = kDefaultSeekPriorityValue;

    // Clear the prior completion state before submission; rejection leaves it cleared.
    DVDContext* context = (DVDContext*)file;
    context->seekPriority = priorityArgument >> kSeekPriorityShift;
    context->callback = 0;
    context->userData = 0;
    AsDvdDeviceFileState(file)->callbackContext = (void*)file;
    if (!DVDSeek((void*)file, offset, (void*)&gtl::CFileIoDeviceDVD::OnSeekComplete,
                 priorityArgument >> kSeekPriorityShift)) {
        File_SetLastError(kFileSeekSubmissionFailed);
        return 0;
    }
    context->callback = callback;
    context->userData = userData;
    File_SetLastError(kFileOk);
    return 1;
}
