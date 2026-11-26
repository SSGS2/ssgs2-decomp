// W3dSpr: a textured sprite quad. It owns an optional bitmap, a world position, a screen
// rectangle relative to a pivot, and four vertices whose colour and UVs are set here.
#ifndef CLASSES_W3DSPR_H
#define CLASSES_W3DSPR_H
#include "game/types.h"
#include "game/wvector.h"
#include "classes/W3dSprVertex.h"

class W3dSpr {
public:
    // Initialise the sprite in storage the caller owns; returns the sprite.
    void* Init();
    // Release an owned bitmap, then the base object; frees the storage when flags is positive.
    void* Destroy(int flags);

    int LoadSprite(void* name, void* loadOptions);
    void SetColor(unsigned int value);
    void SetRect(float width, float height, float pivotX, float pivotY);
    void SetPos(void* source);
    int AttachTexture(unsigned int value);
    void* GetBitmap();

    void* vtbl;               // 0x0
    char unk_4[0xc];          // 0x4
    void* resourceManager;    // 0x10
    void* bitmap;             // 0x14
    float left;               // 0x18
    float right;              // 0x1c
    float top;                // 0x20
    float bottom;             // 0x24
    WVector position;         // 0x28
    W3dSprVertex vertices[4]; // 0x34
    W3dSprVertex* vertex0;    // 0x114
    W3dSprVertex* vertex1;    // 0x118
    W3dSprVertex* vertex2;    // 0x11c
    W3dSprVertex* vertex3;    // 0x120
    int vertexListTerminator; // 0x124
    unsigned char ownsBitmap; // 0x128
};

#define W3DSPR_OFF(f, o) typedef char W3dSpr_off_##f[((int)&((W3dSpr*)0)->f == (o)) ? 1 : -1]
W3DSPR_OFF(vtbl, 0x0);
W3DSPR_OFF(unk_4, 0x4);
W3DSPR_OFF(resourceManager, 0x10);
W3DSPR_OFF(bitmap, 0x14);
W3DSPR_OFF(left, 0x18);
W3DSPR_OFF(right, 0x1c);
W3DSPR_OFF(top, 0x20);
W3DSPR_OFF(bottom, 0x24);
W3DSPR_OFF(position, 0x28);
W3DSPR_OFF(vertices, 0x34);
W3DSPR_OFF(vertex0, 0x114);
W3DSPR_OFF(vertex1, 0x118);
W3DSPR_OFF(vertex2, 0x11c);
W3DSPR_OFF(vertex3, 0x120);
W3DSPR_OFF(vertexListTerminator, 0x124);
W3DSPR_OFF(ownsBitmap, 0x128);
#undef W3DSPR_OFF
#endif
