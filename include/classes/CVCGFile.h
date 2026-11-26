#ifndef CLASSES_CVCGFILE_H
#define CLASSES_CVCGFILE_H

#include "game/types.h"

// The data layout of CVCGFile has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CVCGFile {
public:
    CVCGFile();
    ~CVCGFile();
};

#endif // CLASSES_CVCGFILE_H
