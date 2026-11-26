#ifndef CLASSES_CSPDSQAR_H
#define CLASSES_CSPDSQAR_H

#include "game/types.h"
#include "classes/CBpSprite.h"

// CSpdSqar: a solid-color square. It holds its own geometry and the BP sprite
// it is drawn into; DrawSquare rebuilds the four vertices of that sprite.
class CSpdSqar {
public:
    CSpdSqar();
    ~CSpdSqar();

    int DrawSquare();

    void* vtbl;         // 0x00
    float rotation;     // 0x04  radians about Z
    unsigned int color; // 0x08  packed RGBA
    float scaleX;       // 0x0C
    float scaleY;       // 0x10
    float positionX;    // 0x14
    float positionY;    // 0x18
    float width;        // 0x1C
    float height;       // 0x20
    float originX;      // 0x24  pivot, in square units
    float originY;      // 0x28
    CBpSprite* sprite;  // 0x2C  borrowed
    int dirty;          // 0x30
};

#define CSPDSQAR_OFFSET(field, offset) \
    typedef char CSpdSqar_offset_##field[((int)&((CSpdSqar*)0)->field == (offset)) ? 1 : -1]
CSPDSQAR_OFFSET(color, 0x08);
CSPDSQAR_OFFSET(sprite, 0x2C);
CSPDSQAR_OFFSET(dirty, 0x30);
#undef CSPDSQAR_OFFSET
typedef char CSpdSqar_size[(sizeof(CSpdSqar) == 0x34) ? 1 : -1];

#endif // CLASSES_CSPDSQAR_H
