#ifndef CLASSES_CROTATEREGISTRY_H
#define CLASSES_CROTATEREGISTRY_H

#include "game/types.h"

// The data layout of CRotateRegistry has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CRotateRegistry {
public:
    static void* Create(void* arg);

    static int DeleteAllValues(void* tree);

    void* Destroy(int doDelete);
    int ClearValues();
};

#endif // CLASSES_CROTATEREGISTRY_H
