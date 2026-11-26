// gtl::nw4r::ut::DvdFileStream: a byte stream over one DVD file entry.
// The field offsets below are the ones the methods in src/engine/gtl/ use.
#ifndef CLASSES_GTLDVDFILESTREAM_H
#define CLASSES_GTLDVDFILESTREAM_H

namespace gtl {
namespace nw4r {
namespace ut {

class DvdFileStream {
public:
    // Open a DVD entry and reset the clamped read position. Returns this even if the open fails;
    // check isOpen for success.
    void* Init(void* entryNumber);
    // Close an owned stream through its virtual close method, then optionally free it.
    void* Destroy(int deleteFlag);
    // Close an owned, open handle. Borrowed handles stay open.
    void Close();

    // Reads at the current position; ReadAligned advances it.
    int ReadAligned(void* dst, unsigned int length);
    int ReadAtPosition(void* destination, unsigned int requestedBytes);
    // Asynchronous reads. Completion calls the stored callback with (result, stream, userData).
    int SubmitReadAsync(void* destination, unsigned int size, int callback, int userData, int unused);
    int ReadAlignedAsync(void* buffer, unsigned int size, unsigned int callback, unsigned int userData);

    // Position and cancellation.
    void* Seek(int offset, int origin);
    void* Cancel();
    bool CancelAsync(void* callback, void* userData);

    // Virtual slots. Their meanings are not all established.
    void vfunc_5(void*);
    int vfunc_6(void*);
    int vfunc_8();
    int vfunc_9();
    int vfunc_10();
    int vfunc_11();
    int vfunc_12();
    int vfunc_13();
    int GetLength();
    unsigned char IsBusy();
    void* GetPosition();
    int vfunc_18();
    int vfunc_19();

    static void* GetRTTI();
    static void InitializeTypeInfoAndConstants();
    static void InitializeBaseTypeInfoAndConstants();
    static void InitializeRootTypeInfoAndConstants();
    // SDK completion callbacks; the SDK passes the embedded fileInfo block back.
    static void OnReadComplete(void* result, void* fileInfo);
    static void OnCancelComplete(void* result, void* fileInfo);

    void* vtable;         // 0x00
    unsigned char isOpen; // 0x04
    char pad_005[3];
    void* readResult;            // 0x08
    int readCallback;            // 0x0C: completion callback word
    int readUserData;            // 0x10: completion user data word
    unsigned int length;         // 0x14: file length in bytes
    unsigned int position;       // 0x18: current byte position
    void* cancelCallback;        // 0x1C
    void* cancelUserData;        // 0x20
    unsigned char cancelPending; // 0x24
    char pad_025[3];
    char fileInfo[0x3C];          // 0x28: embedded SDK file-info block
    DvdFileStream* callbackOwner; // 0x64: the stream itself, found by SDK completions
    int priority;                 // 0x68: DVD read priority
    unsigned char busy;           // 0x6C: set while an async read is pending
    unsigned char closeOnDestroy; // 0x6D
    unsigned char ownsFile;       // 0x6E
};

#define GTLDVDFILESTREAM_OFF(f, o) \
    typedef char GtlDvdFileStream_off_##f[((int)&((gtl::nw4r::ut::DvdFileStream*)0)->f == (o)) ? 1 : -1]
GTLDVDFILESTREAM_OFF(vtable, 0x00);
GTLDVDFILESTREAM_OFF(isOpen, 0x04);
GTLDVDFILESTREAM_OFF(readCallback, 0x0C);
GTLDVDFILESTREAM_OFF(readUserData, 0x10);
GTLDVDFILESTREAM_OFF(length, 0x14);
GTLDVDFILESTREAM_OFF(position, 0x18);
GTLDVDFILESTREAM_OFF(cancelCallback, 0x1C);
GTLDVDFILESTREAM_OFF(cancelUserData, 0x20);
GTLDVDFILESTREAM_OFF(cancelPending, 0x24);
GTLDVDFILESTREAM_OFF(fileInfo, 0x28);
GTLDVDFILESTREAM_OFF(callbackOwner, 0x64);
GTLDVDFILESTREAM_OFF(priority, 0x68);
GTLDVDFILESTREAM_OFF(busy, 0x6C);
GTLDVDFILESTREAM_OFF(closeOnDestroy, 0x6D);
GTLDVDFILESTREAM_OFF(ownsFile, 0x6E);
#undef GTLDVDFILESTREAM_OFF

} // namespace ut
} // namespace nw4r
} // namespace gtl

#endif
