#ifndef CLASSES_CCOMSTROKESCORE_H
#define CLASSES_CCOMSTROKESCORE_H

#include "game/types.h"

struct CComStrokeScore {
    void* volatile data;
    void* vtable;
    // Archive adapter borrows its data and performs no resource release.
    ~CComStrokeScore() {}
    void* Destroy(int shouldDelete);
};

#endif // CLASSES_CCOMSTROKESCORE_H
