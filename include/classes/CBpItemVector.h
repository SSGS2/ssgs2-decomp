#ifndef CLASSES_CBPITEMVECTOR_H
#define CLASSES_CBPITEMVECTOR_H

#include "game/types.h"

// The data layout of CBpItemVector has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CBpItemVector {
public:
    int SetItemAtSlot(int itemType, int requestedPlayerSlot, int retainedItemArgument);
};

#endif // CLASSES_CBPITEMVECTOR_H
