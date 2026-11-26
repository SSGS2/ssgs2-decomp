#ifndef CLASSES_CEVENTMODELSC_MDL_MOT_H
#define CLASSES_CEVENTMODELSC_MDL_MOT_H

#include "game/types.h"

namespace ns0 {
struct ModelMotionCommand;
}

// A model motion command: queues the named motion on the actor's model queue.
class CEventModelSC_MDL_MOT {
public:
    // Queue the motion and return the command. A named initializer rather than a
    // constructor because callers construct it over borrowed command storage.
    void* Init(ns0::ModelMotionCommand* input);

    // True once the queue tied to this command has processed its record.
    unsigned int IsMotionQueueIdle();

    // Free the command when the deleting flag is set, then return its address.
    void* Destroy(int flag);

    void* vtbl;      // 0x0: original vtable pointer
    void* modelSlot; // 0x4
};

#define CEVENTMODELSC_MDL_MOT_OFF(f, o) \
    typedef char CEventModelSC_MDL_MOT_off_##f[((int)&((CEventModelSC_MDL_MOT*)0)->f == (o)) ? 1 : -1]
CEVENTMODELSC_MDL_MOT_OFF(modelSlot, 0x4);
#undef CEVENTMODELSC_MDL_MOT_OFF

#endif // CLASSES_CEVENTMODELSC_MDL_MOT_H
