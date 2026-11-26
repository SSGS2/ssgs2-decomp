#ifndef CLASSES_CTHREAD_H
#define CLASSES_CTHREAD_H

#include "game/types.h"

// Owner of one OS thread handle. The thread itself lives in the owning object;
// this class only tracks its handle.
class CThread {
public:
    int IsFinished();
    void CancelIfRunning();

    void* vtbl; // 0x0: original vtable pointer
    int thread; // 0x4: OS thread handle, zero when there is none
};

#define CTHREAD_OFF(f, o) typedef char CThread_off_##f[((int)&((CThread*)0)->f == (o)) ? 1 : -1]
CTHREAD_OFF(thread, 0x4);
#undef CTHREAD_OFF

#endif // CLASSES_CTHREAD_H
