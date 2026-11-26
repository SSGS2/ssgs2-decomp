#ifndef CLASSES_CAPPFONTRESOURCE_H
#define CLASSES_CAPPFONTRESOURCE_H

#include "game/types.h"

struct CAppFontResource {
    void* vtable;
    int reserved04;
    void* backingResource;
    void** storage;
    int count;
    int capacity;

    CAppFontResource();
    ~CAppFontResource();
};

#endif // CLASSES_CAPPFONTRESOURCE_H
