// W3dSpr: methods in w3d_sprite_and_billboard.cpp. Layout view; no virtual declarations.
#ifndef CLASSES_W3DSPR_H
#define CLASSES_W3DSPR_H
#include "game/types.h"
class W3dSpr {
public:
    void* vtbl; // 0x0: W3dSpr_ctor/dtor install __vt__6W3dSpr.
    char unk_4[0xc];
    void* resourceManager; // 0x10: 0x8021E928 LoadBitmap; dtor ReleaseId.
    void* bitmap; // 0x14: 0x8021E928 load result; W3dSpr_GetBitmap returns it.
    float left; // 0x18: 0x8021E980 stores -f3.
    float right; // 0x1c: 0x8021E980 stores f1-f3.
    float top; // 0x20: 0x8021E980 stores -f4.
    float bottom; // 0x24: 0x8021E980 stores f2-f4.
    char position[0xc]; // 0x28: 0x8021E9A4 WVector assignment.
    char vertices[0xe0]; // 0x34: ctor builds four 0x38-byte vertices.
    void* vertex0; // 0x114: ctor and 0x8021E904.
    void* vertex1; // 0x118
    void* vertex2; // 0x11c
    void* vertex3; // 0x120
    int vertexListTerminator; // 0x124: ctor writes zero after the four pointers.
    unsigned char ownsBitmap; // 0x128: load sets 1; dtor gates ReleaseId.
};
#define W3DSPR_OFF(f,o) typedef char W3dSpr_off_##f[((int)&((W3dSpr*)0)->f == (o)) ? 1 : -1]
W3DSPR_OFF(vtbl,0x0);
W3DSPR_OFF(unk_4,0x4);
W3DSPR_OFF(resourceManager,0x10);
W3DSPR_OFF(bitmap,0x14);
W3DSPR_OFF(left,0x18);
W3DSPR_OFF(right,0x1c);
W3DSPR_OFF(top,0x20);
W3DSPR_OFF(bottom,0x24);
W3DSPR_OFF(position,0x28);
W3DSPR_OFF(vertices,0x34);
W3DSPR_OFF(vertex0,0x114);
W3DSPR_OFF(vertex1,0x118);
W3DSPR_OFF(vertex2,0x11c);
W3DSPR_OFF(vertex3,0x120);
W3DSPR_OFF(vertexListTerminator,0x124);
W3DSPR_OFF(ownsBitmap,0x128);
#undef W3DSPR_OFF
#endif
