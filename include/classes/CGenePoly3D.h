#ifndef CLASSES_CGENEPOLY3D_H
#define CLASSES_CGENEPOLY3D_H

#include "game/types.h"

struct PolygonAllocationView;
struct PolygonColorView;
struct PolygonPositionView;
class CGenePoly3D {
public:
    void* vtbl;        // 0x0: original vtable pointer
    char pad_004[0x4]; // 0x4
    void* vertices;    // 0x8

    static unsigned int ReplaceVertexStorage(PolygonAllocationView* p, unsigned int n);
    void vfunc_5();
    static void SetVertexColorARGB(PolygonColorView* p, unsigned char index, unsigned char alpha, unsigned char red,
                                   unsigned char green, unsigned char blue);
    static void CopyVertexPosition(PolygonPositionView* p, unsigned char index, void* output);
    void DrawStoredVertices(unsigned int primitive, unsigned int count, void* vertices);
    static void SelectVertexFormat();

    ~CGenePoly3D();
};

#define CGENEPOLY3D_OFF(f, o) typedef char CGenePoly3D_off_##f[((int)&((CGenePoly3D*)0)->f == (o)) ? 1 : -1]
CGENEPOLY3D_OFF(vertices, 0x8);
#undef CGENEPOLY3D_OFF

#endif // CLASSES_CGENEPOLY3D_H
