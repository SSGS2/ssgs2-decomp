// Streamed audio cancellation: scheduler load-task cancel hook; normal read and
// completion delivery are in gtl_dvd_file_stream_read_part1.cpp. No gameplay tuning constants.
// API map: clear task receiver first, query stream cancellation capability,
// choose asynchronous cancellation when supported, otherwise cancel synchronously.
#include "classes/SoundStreamSource.h"

// Borrowed file-stream vtable view. Slot offsets include the two ABI header words.
struct AudioStreamCancellationView {
    // Vtable: +0x28 is a capability, not a readiness/busy test; +0x48 cancel calls DvdFileStream::DVDCancel; +0x4C
    // cancelAsync(callback, userData).
    virtual void reservedSlot08();
    virtual void reservedSlot0C();
    virtual void reservedSlot10();
    virtual void reservedSlot14();
    virtual void reservedSlot18();
    virtual void reservedSlot1C();
    virtual void reservedSlot20();
    virtual void reservedSlot24();
    virtual int canCancelAsync(); // +0x28: capability, not a readiness/busy test.
    virtual void reservedSlot2C();
    virtual void reservedSlot30();
    virtual void reservedSlot34();
    virtual void reservedSlot38();
    virtual void reservedSlot3C();
    virtual void reservedSlot40();
    virtual void reservedSlot44();
    virtual void cancel();              // +0x48: DvdFileStream calls DVDCancel.
    virtual void cancelAsync(int, int); // +0x4C: callback, userData (both null here).
    virtual void reservedSlot50();
    virtual int canCancel();
    // +0x54: cancellation capability, not a busy flag.
};

extern "C" {

extern AudioStreamCancellationView* g_activeAudioStream;

} // extern "C"

// Suppress this task's completion callback, then cancel the active file stream if supported.
// Borrows the task and the global stream; neither is freed. Cancellation return values are ignored.
// The receiver remains suppressed even when cancellation is unsupported. Null callback/userData
// requests no cancel notification. This hook does not wait for async completion or close the stream;
// the task/stream manager owns that lifecycle. Serialize with stream reads.
void gtl::CSoundStreamSource::StrmDataLoadTask::Cancel() {
    callback = 0;
    if (g_activeAudioStream && g_activeAudioStream->canCancel()) {
        if (g_activeAudioStream->canCancelAsync()) {
            g_activeAudioStream->cancelAsync(0, 0);
        } else {
            g_activeAudioStream->cancel();
        }
    }
}
