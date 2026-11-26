#ifndef CLASSES_CANOTHERHAND_H
#define CLASSES_CANOTHERHAND_H

#include "game/types.h"

// Owns a thread that runs one callback under an embedded mutex. The caller
// supplies the callback receiver and its two arguments.
class CAnotherHand {
public:
    CAnotherHand();
    ~CAnotherHand();

    void Lock();
    void Unlock();

    // Store the callback inputs, start a suspended thread on the owned stack and resume it.
    // Returns 1 if the thread was created, 0 if creation failed.
    int StartCallbackThread(void* receiver, void* argument, void* value, void* unused, unsigned int priorityDelta);

    // Thread entry point. It is static because the thread system calls it with the owner as its only argument.
    static int CallbackThreadEntry(void* self);

    void* vtbl;                       // 0x0: original vtable pointer
    void* thread;                     // 0x4: OS thread handle, zero when there is none
    void* callbackReceiver;           // 0x8: virtual object the callback is made on
    void* callbackArgument;           // 0xc: first callback argument
    void* callbackValue;              // 0x10: second callback argument, an int in disguise
    unsigned int threadPriorityDelta; // 0x14: added to the base priority 0x18 when the thread runs
    char osThread[0x330];             // 0x18
    void* threadStack;                // 0x348: 64 KiB of stack for the callback thread
};

#define CANOTHERHAND_OFF(f, o) typedef char CAnotherHand_off_##f[((int)&((CAnotherHand*)0)->f == (o)) ? 1 : -1]
CANOTHERHAND_OFF(thread, 0x4);
CANOTHERHAND_OFF(threadStack, 0x348);
CANOTHERHAND_OFF(callbackReceiver, 0x8);
CANOTHERHAND_OFF(callbackArgument, 0xc);
CANOTHERHAND_OFF(callbackValue, 0x10);
CANOTHERHAND_OFF(threadPriorityDelta, 0x14);
#undef CANOTHERHAND_OFF

#endif // CLASSES_CANOTHERHAND_H
