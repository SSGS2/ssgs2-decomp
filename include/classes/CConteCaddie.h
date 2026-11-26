#ifndef CLASSES_CCONTECADDIE_H
#define CLASSES_CCONTECADDIE_H

#include "game/types.h"

// The data layout of CConteCaddie has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
struct RefHolder;
class CConteCaddie {
public:
    CConteCaddie();

    static void DestroyOwnedActor(RefHolder* p);
};

#endif // CLASSES_CCONTECADDIE_H
