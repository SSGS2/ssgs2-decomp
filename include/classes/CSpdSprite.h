#ifndef CLASSES_CSPDSPRITE_H
#define CLASSES_CSPDSPRITE_H

#include "game/types.h"
#include "classes/CBpSprite.h"

// CSpdSprite: a textured quad. It holds its position, scale, pivot, texture
// coordinates and flip mode, and RebuildVertices writes the four transformed
// vertices into the BP sprite it is drawn with.
class CSpdSprite {
public:
    CSpdSprite();
    ~CSpdSprite();

    int RebuildVertices();

    void* vtbl;                  // 0x00
    float rotation;              // 0x04  radians about Z
    volatile unsigned int color; // 0x08  packed RGBA; the original reads it volatile
    float scaleX;                // 0x0C
    float scaleY;                // 0x10
    float positionX;             // 0x14
    float positionY;             // 0x18
    float vertexX;               // 0x1C  right edge of the quad, in sprite units
    float vertexY;               // 0x20  bottom edge of the quad, in sprite units
    float originX;               // 0x24  pivot
    float originY;               // 0x28
    CBpSprite* sprite;           // 0x2C  borrowed
    int dirty;                   // 0x30
    void* source;                // 0x34  texture binding, borrowed
    float u0, v0;                // 0x38
    float u1, v1;                // 0x40
    char pad_48[8];              // 0x48
    int flipMode;                // 0x50  kFlipVertical, kFlipHorizontal or kFlipBoth
};

#define CSPDSPRITE_OFFSET(field, offset) \
    typedef char CSpdSprite_offset_##field[((int)&((CSpdSprite*)0)->field == (offset)) ? 1 : -1]
CSPDSPRITE_OFFSET(color, 0x08);
CSPDSPRITE_OFFSET(sprite, 0x2C);
CSPDSPRITE_OFFSET(source, 0x34);
CSPDSPRITE_OFFSET(flipMode, 0x50);
#undef CSPDSPRITE_OFFSET
typedef char CSpdSprite_size[(sizeof(CSpdSprite) == 0x54) ? 1 : -1];

#endif // CLASSES_CSPDSPRITE_H
