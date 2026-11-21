// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCALLBACK_H
#define CLASSES_CCALLBACK_H

#include "game/types.h"

class CCallback {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int (*callback)(void*, void*, int);                  // 0x4
    void* callbackContext;                        // 0x8: 1 use in CCallback_InvokeCallback

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCALLBACK_OFF(f, o) typedef char CCallback_off_##f[((int)&((CCallback*)0)->f == (o)) ? 1 : -1]
CCALLBACK_OFF(vtbl, 0x0);
CCALLBACK_OFF(callback, 0x4); // CCallback_InvokeCallback (0x802932C0): invokes (self, callbackContext, arg).
CCALLBACK_OFF(callbackContext, 0x8);
#undef CCALLBACK_OFF

#endif  // CLASSES_CCALLBACK_H
