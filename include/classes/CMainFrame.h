#ifndef CLASSES_CMAINFRAME_H
#define CLASSES_CMAINFRAME_H

#include "game/types.h"

// The main frame: the simulated clock and the hook dispatch that runs the project's update.
// The fields are the base part shared with the project object; the project adds its own after them.
class CMainFrame {
public:
    // Default main-frame hooks; this base implementation has no work.
    void vfunc_5();
    void vfunc_3();

    // Snapshot the virtual value and reset the elapsed time.
    void ResetElapsedTimeAndCaptureValue();
    // Advance the simulated clock and run the hook dispatch; returns applicationResult.
    void* UpdateVirtualHooks(unsigned int flags);

    // Base initialization and cleanup, reached through the constructor and destructor call sites. Both return this.
    CMainFrame* Init();
    CMainFrame* Destroy(int freeFlag);

    void* vtbl;                // 0x00
    void* applicationResult;   // 0x04  nonzero after an exit request
    float elapsedMilliseconds; // 0x08  simulated milliseconds since the last reset
    int initialValue;          // 0x0C
};

#define CMAINFRAME_OFFSET(field, offset) \
    typedef char CMainFrame_offset_##field[((int)&((CMainFrame*)0)->field == (offset)) ? 1 : -1]
CMAINFRAME_OFFSET(applicationResult, 0x04);
CMAINFRAME_OFFSET(elapsedMilliseconds, 0x08);
CMAINFRAME_OFFSET(initialValue, 0x0C);
#undef CMAINFRAME_OFFSET

#endif // CLASSES_CMAINFRAME_H
