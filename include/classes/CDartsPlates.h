#ifndef CLASSES_CDARTSPLATES_H
#define CLASSES_CDARTSPLATES_H

#include "game/types.h"

// CDartsPlates: the plate handles and the balloon model above them.
//
// CDartsSystem owns the instance; the destructor releases the plate handles,
// the texture and the balloon's scene binding.
class CDartsPlates {
public:
    ~CDartsPlates();
    void Draw();

    void* vtbl;                 // 0x0
    char pad_004[0xc];          // 0x4
    float heading;              // 0x10
    void* plates;               // 0x14
    unsigned int plateCount;    // 0x18
    unsigned int plateCapacity; // 0x1c: OnPreLoadInit zeroes the trailing word of plates/count/capacity vector
    void* texture;              // 0x20
    void* balloonModel;         // 0x24
    int renderSuppressed;       // 0x28
};

#define CDARTSPLATES_OFF(f, o) typedef char CDartsPlates_off_##f[((int)&((CDartsPlates*)0)->f == (o)) ? 1 : -1]
CDARTSPLATES_OFF(heading, 0x10);
CDARTSPLATES_OFF(plates, 0x14);
CDARTSPLATES_OFF(plateCount, 0x18);
CDARTSPLATES_OFF(plateCapacity, 0x1c);
CDARTSPLATES_OFF(texture, 0x20);
CDARTSPLATES_OFF(balloonModel, 0x24);
CDARTSPLATES_OFF(renderSuppressed, 0x28);
#undef CDARTSPLATES_OFF
typedef char CDartsPlates_size[(sizeof(CDartsPlates) == 0x2c) ? 1 : -1];

#endif // CLASSES_CDARTSPLATES_H
