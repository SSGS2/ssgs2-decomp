#ifndef CLASSES_CBGM_H
#define CLASSES_CBGM_H

#include "game/types.h"

// The data layout of CBgm has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CBgm {
public:
    static int collection_data_vfunc_7();
    static int collection_data_vfunc_6();
    static int collection_data_vfunc_5();
    static int collection_data_vfunc_4();
    static int collection_data_vfunc_3();
    static int collection_data_vfunc_2();
    static int collection_data_vfunc_1();
};

#endif // CLASSES_CBGM_H
