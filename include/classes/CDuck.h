#ifndef CLASSES_CDUCK_H
#define CLASSES_CDUCK_H

#include "game/types.h"

class CDuck {
public:
    void* vtbl;             // 0x0: original vtable pointer
    void* secondaryVtable;  // 0x4
    char pad_008[0x1c];     // 0x8
    void* puppet;           // 0x24
    char pad_028[0xf4];     // 0x28
    int instanceCount;      // 0x11c
    void* instanceMatrices; // 0x120
    int linkCount;          // 0x124
    void* listPrevious;     // 0x128
    void* listNext;         // 0x12c

    // Initialize CDuck state and owned resources (the original constructor; not a C++ constructor).
    void* Init(const char* arg, int count);

    // Cull and draw each duck puppet using its corresponding follower transform.
    void DrawInstances();

    // Move each duck toward its predecessor with a clamped turn rate and minimum separation.
    void UpdateFlockFormation(float deltaTime);

    // Place the flock leader and followers at fixed intervals behind it.
    void InitFlockFormation(const void* src, unsigned int flags);

    // Load one puppet per duck instance into the owned list and select the first puppet.
    void LoadPuppetInstances(void* arg);

    // Release CDuck resources and optionally delete the object.
    void* Dtor(int doDelete);

    // Adjust the secondary-interface pointer back to its owner for destruction.
    void DtorThunk4(int deleteFlag);
};

#define CDUCK_OFF(f, o) typedef char CDuck_off_##f[((int)&((CDuck*)0)->f == (o)) ? 1 : -1]
CDUCK_OFF(secondaryVtable, 0x4);
CDUCK_OFF(puppet, 0x24);
CDUCK_OFF(instanceCount, 0x11c);
CDUCK_OFF(instanceMatrices, 0x120);
CDUCK_OFF(linkCount, 0x124);
CDUCK_OFF(listPrevious, 0x128);
CDUCK_OFF(listNext, 0x12c);
#undef CDUCK_OFF

#endif // CLASSES_CDUCK_H
