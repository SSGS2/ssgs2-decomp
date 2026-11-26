// PAD-memory device open: calls the device's open routine, then runs an optional completion
// callback straight away when the open succeeds.

#include <decomp/local_class.h>
#include "classes/GtlFileIoDevicePADMEM.h"
#include "sdk_protos.h"

struct Unk804206E4 {
    int unk0;
    int completionReady;      // +0x4: completion-ready flag (zero pending, one complete).
    int requestedBytesOrZero; // +0x8: transfer size for read/write; not initialized by this open wrapper.
};

extern Unk804206E4* g_padMemAsyncHandle;
extern int (*g_padMemCompletionCallback)(int, int, Unk804206E4*, void*);
extern void* g_padMemIoCallbackData;

extern "C" void File_SetLastError(int x);

#pragma opt_lifetimes off
#pragma opt_common_subs off
// Open through virtual slot 2, then invoke the optional completion callback synchronously.
// path/data are borrowed; callbackAddress receives (0, 0, openedHandle, data).
// Returns the underlying open result (-1 on failure); ownership follows that open API.
// One global context slot: serialize calls. Success clears it AFTER the callback;
// failure leaves callback/data installed. Reentrant callback submissions would be cleared too.
void* gtl::CFileIoDevicePADMEM::OpenWithCallback(const char* path, int /*flags*/, int /*r6*/, int /*mode*/, int /*r8*/,
                                                 int /*r9*/, int callbackAddress, void* data) {
    // Publish context before calling the device's open implementation.
    g_padMemAsyncHandle = 0;
    g_padMemCompletionCallback = (int (*)(int, int, Unk804206E4*, void*))callbackAddress;
    g_padMemIoCallbackData = data;

    // Only path is forwarded to open; the other incoming integer arguments are ignored here.
    struct V4 {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual void* v2(const char*) DECOMP_LOCAL_VIRTUAL;
    };
    Unk804206E4* openedHandle = (Unk804206E4*)(((V4*)this)->v2(path));

    // Successful open is already complete when the user callback runs.
    if (openedHandle != (Unk804206E4*)-1) {
        g_padMemAsyncHandle = openedHandle;
        File_SetLastError(0);
        g_padMemAsyncHandle->completionReady = 1;
        if (g_padMemCompletionCallback != 0) {
            g_padMemCompletionCallback(0, 0, g_padMemAsyncHandle, g_padMemIoCallbackData);
        }
        g_padMemAsyncHandle = 0;
        g_padMemCompletionCallback = 0;
        g_padMemIoCallbackData = 0;
    }

    return openedHandle;
}

#pragma opt_lifetimes reset
#pragma opt_common_subs reset
