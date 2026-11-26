#ifndef CLASSES_CSPARROW_H
#define CLASSES_CSPARROW_H

#include "game/types.h"

class CSparrow {
public:
    void* vtbl;                // 0x0: original vtable pointer
    void* secondaryVtable;     // 0x4
    char pad_008[0x1c];        // 0x8
    void* model;               // 0x24
    char pad_028[0x30];        // 0x28
    float animationFrame;      // 0x58
    char pad_05C[0x51];        // 0x5c
    unsigned char drawEnabled; // 0xad
    char pad_0AE[0x6E];        // 0xae
    int maxInstanceCount;      // 0x11c
    int instanceCount;         // 0x120
    void* instanceMatrices;    // 0x124

    ~CSparrow();

    // Scale the active flock size by ball speed and randomize instance headings around the spawn center.
    void InitFlockInstances();

    // Initialize the sparrow's common creature base and instance storage; returns the sparrow.
    void* Init(void* arg, int count);

    // Draw the shared sparrow puppet once for each active instance transform.
    void Draw();

    // Adjust the secondary-interface pointer back to its owner for destruction.
    void DtorThunk4(int deleteFlag);
};

#define CSPARROW_OFF(f, o) typedef char CSparrow_off_##f[((int)&((CSparrow*)0)->f == (o)) ? 1 : -1]
CSPARROW_OFF(secondaryVtable, 0x4);
CSPARROW_OFF(model, 0x24);
CSPARROW_OFF(animationFrame, 0x58);
CSPARROW_OFF(drawEnabled, 0xad);
CSPARROW_OFF(instanceCount, 0x120);
CSPARROW_OFF(instanceMatrices, 0x124);
#undef CSPARROW_OFF

#endif // CLASSES_CSPARROW_H
