#ifndef CLASSES_CRAIN_H
#define CLASSES_CRAIN_H

#include "game/types.h"

struct Obj801DC054;
class CRain {
public:
    void* vtbl;         // 0x0: original vtable pointer
    char pad_004[0x1c]; // 0x4
    void* vertexBuffer; // 0x20

    void vfunc_4();

    void DrawLineSegments();
    static void UpdateDropVertices(Obj801DC054* object, float timeStep);
    void UpdateDropDirection();

    ~CRain();
    CRain();
};

#define CRAIN_OFF(f, o) typedef char CRain_off_##f[((int)&((CRain*)0)->f == (o)) ? 1 : -1]
CRAIN_OFF(vertexBuffer, 0x20);
#undef CRAIN_OFF

#endif // CLASSES_CRAIN_H
