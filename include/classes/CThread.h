// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CTHREAD_H
#define CLASSES_CTHREAD_H

#include "game/types.h"

class CThread {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int thread;                          // 0x4: 1 use in CThread_IsFinished
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CTHREAD_OFF(f, o) typedef char CThread_off_##f[((int)&((CThread*)0)->f == (o)) ? 1 : -1]
CTHREAD_OFF(thread, 0x4);
#undef CTHREAD_OFF

#endif  // CLASSES_CTHREAD_H
