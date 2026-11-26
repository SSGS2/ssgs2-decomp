#ifndef CLASSES_CBPSPRITE_H
#define CLASSES_CBPSPRITE_H

#include "game/types.h"

// One vertex as the sprite stores it: position, color bytes and texture
// coordinates, all as raw bit patterns.
struct SpriteVertexBits {
    int positionX, positionY, positionZ;
    unsigned char color[4];
    int texCoordU, texCoordV;
};

// A 2D sprite: a texture binding and up to four vertices, drawn as one strip.
class CBpSprite {
public:
    ~CBpSprite();
    int Draw();
    int BeginVertices(int textureBindingAddress);
    int ActivateAndClearVertexCount();
    int AppendVertex(void* vertex); // caller's record: x, y, z floats, RGBA bytes, u, v floats

    void* vtbl;                   // 0x00
    void* textureBinding;         // 0x04  CBpTex, borrowed
    int active;                   // 0x08
    int vertexCount;              // 0x0C
    SpriteVertexBits vertices[4]; // 0x10
    int drawStateOverride;        // 0x70
};

typedef char CBpSprite_size[(sizeof(CBpSprite) == 0x74) ? 1 : -1];

#endif // CLASSES_CBPSPRITE_H
