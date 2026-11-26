#ifndef CLASSES_CBUILDPRODUCTCADDIE_H
#define CLASSES_CBUILDPRODUCTCADDIE_H

#include "game/types.h"

// The data layout of CBuildProductCaddie has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CBuildProductCaddie {
public:
    int LoadIconGroups(int caddieIndex);

    ~CBuildProductCaddie();
    CBuildProductCaddie(int count, unsigned char flag);
};

#endif // CLASSES_CBUILDPRODUCTCADDIE_H
