// SpringGridVertex: layout view; methods in project_base_and_grid_surface.cpp (GridSurface_StepSpringHeights integrates velocity and y; Grid_RecomputeNormalsAndNormalTexcoords computes normals and texture coordinates).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_GRIDCELL801C9600_H
#define CLASSES_GRIDCELL801C9600_H

class SpringGridVertex {
public:
    float x; // 0x0
    float y; // 0x4
    float z; // 0x8
    float normalX; // 0xC
    float normalY; // 0x10
    float normalZ; // 0x14
    float acceleration; // 0x18
    float velocity; // 0x1C
    float texU; // 0x20
    float texV; // 0x24
    unsigned char active; // 0x28
};

#define OFF(f, o) typedef char GridCell801C9600_off_##f[((int)&((SpringGridVertex*)0)->f == (o)) ? 1 : -1]
OFF(x, 0x0);
OFF(y, 0x4);
OFF(z, 0x8);
OFF(normalX, 0xC);
OFF(normalY, 0x10);
OFF(normalZ, 0x14);
OFF(acceleration, 0x18);
OFF(velocity, 0x1C);
OFF(texU, 0x20);
OFF(texV, 0x24);
OFF(active, 0x28);
#undef OFF
typedef char GridCell801C9600_size[(sizeof(SpringGridVertex) == 0x2C) ? 1 : -1];

#define AsGridCell801C9600(p) ((SpringGridVertex*)(p))

#endif
