#ifndef CLASSES_CCALLBACK_H
#define CLASSES_CCALLBACK_H

#include "game/types.h"

class CCallback {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int (*callback)(void*, void*, int); // 0x4
    void* callbackContext;              // 0x8

    int InvokeCallback(int arg);
    int SetCallbackAndContext(int a, int b);

    CCallback();
};

#define CCALLBACK_OFF(f, o) typedef char CCallback_off_##f[((int)&((CCallback*)0)->f == (o)) ? 1 : -1]
CCALLBACK_OFF(vtbl, 0x0);
CCALLBACK_OFF(callback, 0x4); // CCallback_InvokeCallback (0x802932C0): invokes (self, callbackContext, arg).
CCALLBACK_OFF(callbackContext, 0x8);
#undef CCALLBACK_OFF

#endif // CLASSES_CCALLBACK_H
