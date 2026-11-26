// CGeneTex3D: the vertex-buffer primitive that textured effects draw through.
//
// It owns a CPU-side vertex array and the original vtable. The vtable pointer
// sits at 0x4, after the per-owner vertex count, so it is an explicit member
// rather than a compiler-managed one; the six drawing methods are the vtable's
// slots. Every effect that embeds the primitive installs the same vtable.
#ifndef CLASSES_CGENETEX3D_H
#define CLASSES_CGENETEX3D_H

#include "game/types.h"

class CGeneTex3D {
public:
    void SelectVertexFormat();
    int AllocVertexBuffer(int count);
    void SetVertexColorARGB(int index, unsigned char alpha, unsigned char red, unsigned char green, unsigned char blue);
    void SetVertexPosition(int index, void* destination);
    void SetTexCoord(int index, float u, float v);
    void DrawVertices(int primitive, int count);

    unsigned char vertexCount; // 0x00
    char pad_01[3];
    void* vtbl;     // 0x04  the original vtable, an explicit member
    void* vertices; // 0x08  CPU-side vertices, stride 0x18
};

#define CGENETEX3D_OFF(f, o) typedef char CGeneTex3D_off_##f[((int)&((CGeneTex3D*)0)->f == (o)) ? 1 : -1]
CGENETEX3D_OFF(vertexCount, 0x00);
CGENETEX3D_OFF(vtbl, 0x04);
CGENETEX3D_OFF(vertices, 0x08);
#undef CGENETEX3D_OFF
typedef char CGeneTex3D_size[(sizeof(CGeneTex3D) == 0x0C) ? 1 : -1];

#endif // CLASSES_CGENETEX3D_H
