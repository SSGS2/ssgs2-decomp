#ifndef CLASSES_CCAMERAOWNER_H
#define CLASSES_CCAMERAOWNER_H

#include "game/types.h"

class CCameraOwner {
public:
    ~CCameraOwner();

    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x18]; // 0x4
    void* modelPreview; // 0x1c
};

#define CCAMERAOWNER_OFF(f, o) typedef char CCameraOwner_off_##f[((int)&((CCameraOwner*)0)->f == (o)) ? 1 : -1]
CCAMERAOWNER_OFF(modelPreview, 0x1c);
#undef CCAMERAOWNER_OFF

#endif // CLASSES_CCAMERAOWNER_H
