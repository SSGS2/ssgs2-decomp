#ifndef CLASSES_CB2DPLAYERHANDLE_H
#define CLASSES_CB2DPLAYERHANDLE_H

#include "game/types.h"

// The data layout of CB2dPlayerHandle has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CB2dPlayerHandle {
public:
    static void RequestPendingScript(void* object);
};

#endif // CLASSES_CB2DPLAYERHANDLE_H
