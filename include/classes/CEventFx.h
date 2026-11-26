#ifndef CLASSES_CEVENTFX_H
#define CLASSES_CEVENTFX_H

#include "game/types.h"

// A spray or sequence effect attached to a model, with its resource name and
// the transform it was created from.
class CEventFx {
public:
    // Create the attached effect and keep its name and position inputs.
    void* Init(const char* name, void* attachment, void* positionOffset, void* basePosition, void* bypassResourceCache);

    // Free the effect when the deleting flag is set, then return its address.
    void* Destroy(int flag);

    void* vtbl;                       // 0x0: original vtable pointer
    unsigned int nameLength;          // 0x4
    void* nameData;                   // 0x8
    void* spray;                      // 0xc
    void* sequence;                   // 0x10
    unsigned int bypassResourceCache; // 0x14
    void* positionOffset;             // 0x18
    float position[3];                // 0x1c
    float basePosition[3];            // 0x28
    void* texture;                    // 0x34
    void* eventVtable;                // 0x38
};

#define CEVENTFX_OFF(f, o) typedef char CEventFx_off_##f[((int)&((CEventFx*)0)->f == (o)) ? 1 : -1]
CEVENTFX_OFF(nameLength, 0x4);
CEVENTFX_OFF(nameData, 0x8);
CEVENTFX_OFF(spray, 0xc);
CEVENTFX_OFF(sequence, 0x10);
CEVENTFX_OFF(bypassResourceCache, 0x14);
CEVENTFX_OFF(positionOffset, 0x18);
CEVENTFX_OFF(position, 0x1c);
CEVENTFX_OFF(basePosition, 0x28);
CEVENTFX_OFF(texture, 0x34);
CEVENTFX_OFF(eventVtable, 0x38);
#undef CEVENTFX_OFF

#endif // CLASSES_CEVENTFX_H
