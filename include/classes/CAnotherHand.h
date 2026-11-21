// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CANOTHERHAND_H
#define CLASSES_CANOTHERHAND_H

#include "game/types.h"

class CAnotherHand {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* thread;                        // 0x4: 3 uses in CAnotherHand_dtor, CAnotherHand_Ctor
    void* callbackReceiver;            // 0x8: CAnotherHand_StartCallbackThread -> CAnotherHand_CallbackThreadEntry virtual receiver
    void* callbackArgument;            // 0xc: CAnotherHand_CallbackThreadEntry first callback argument
    void* callbackValue;               // 0x10: CAnotherHand_StartCallbackThread third input, dispatched as int
    unsigned int threadPriorityDelta;                // 0x14: CAnotherHand_CallbackThreadEntry compares OSGetThreadPriority result minus 0x18
    char pad_018[0x330];                // 0x18
    void* threadStack;                      // 0x348: 4 uses in CAnotherHand_dtor, CAnotherHand_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x330
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CANOTHERHAND_OFF(f, o) typedef char CAnotherHand_off_##f[((int)&((CAnotherHand*)0)->f == (o)) ? 1 : -1]
CANOTHERHAND_OFF(thread, 0x4);
CANOTHERHAND_OFF(threadStack, 0x348);
CANOTHERHAND_OFF(callbackReceiver, 0x8);
CANOTHERHAND_OFF(callbackArgument, 0xc);
CANOTHERHAND_OFF(callbackValue, 0x10);
CANOTHERHAND_OFF(threadPriorityDelta, 0x14);
#undef CANOTHERHAND_OFF

#endif  // CLASSES_CANOTHERHAND_H
