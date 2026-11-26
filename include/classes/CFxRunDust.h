#ifndef CLASSES_CFXRUNDUST_H
#define CLASSES_CFXRUNDUST_H

#include "game/types.h"

// The data layout of CFxRunDust has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CFxRunDust {
public:
    int UpdateGroundWaterSequences();

    ~CFxRunDust();
};

#endif // CLASSES_CFXRUNDUST_H
