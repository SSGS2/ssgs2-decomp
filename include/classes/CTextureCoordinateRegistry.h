#ifndef CLASSES_CTEXTURECOORDINATEREGISTRY_H
#define CLASSES_CTEXTURECOORDINATEREGISTRY_H

#include "game/types.h"

// The data layout of CTextureCoordinateRegistry has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CTextureCoordinateRegistry {
public:
    int DeleteAllValues();

    static void* Create(void* arg);
    static int vfunc_1(void* arg);

    ~CTextureCoordinateRegistry();
};

#endif // CLASSES_CTEXTURECOORDINATEREGISTRY_H
