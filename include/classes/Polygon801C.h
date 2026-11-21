// Polygon801C: layout view from Polygon_ContainsPointXZ / Polygon_ClampPointAlongRayXZ.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_POLYGON801C_H
#define CLASSES_POLYGON801C_H

struct Polygon801C {
    void* vertices; // 0x0: Polygon_ContainsPointXZ loads 12-byte XYZ vertices
    int vertexCount; // 0x4: Polygon_ContainsPointXZ rejects fewer than three vertices
};

#define OFF(f, o) typedef char Polygon801C_off_##f[((int)&((Polygon801C*)0)->f == (o)) ? 1 : -1]
OFF(vertices, 0x0);
OFF(vertexCount, 0x4);
#undef OFF
#define AsPolygon801C(p) ((Polygon801C*)(p))

#endif

