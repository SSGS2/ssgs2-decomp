#ifndef CLASSES_CCOMRANDOMUSE_H
#define CLASSES_CCOMRANDOMUSE_H

#include "game/types.h"

// The data layout of CComRandomUse has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CComRandomUse {
public:
    float AdjustValue(int index, int category, float requestedValue);
    int AreTablesLoaded();
    int RequestShotTableLoad(void* arg);
    int RequestPuttTableLoad(void* arg);
    int RequestNoTargetTableLoad(void* arg);
    int RequestSpecialTableLoad(void* arg);

    CComRandomUse();
    ~CComRandomUse();
};

#endif // CLASSES_CCOMRANDOMUSE_H
