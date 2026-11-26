#ifndef CLASSES_CPARTICLEBASE_H
#define CLASSES_CPARTICLEBASE_H

#include "game/types.h"

// The data layout of CParticleBase has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CParticleBase {
public:
    ~CParticleBase();
};

#endif // CLASSES_CPARTICLEBASE_H
