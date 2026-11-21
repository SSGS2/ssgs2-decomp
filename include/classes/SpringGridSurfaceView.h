// SpringGridSurfaceView: layout view; methods in project_base_and_grid_surface.cpp (GroundGrid_InitializeTwoByTwo records g_currentTimeMilliseconds; GridSurface_StepSpringHeights caps surface height).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_GRIDSURFACE801C9600_H
#define CLASSES_GRIDSURFACE801C9600_H

class SpringGridSurfaceView {
public:
    char pad_000[0x4];
    int itemCount; // 0x4
    int groupCount; // 0x8
    void** groups; // 0xC
    void** vectors; // 0x10
    char pad_014[0x4];
    int updatedAt; // 0x18
    char pad_01C[0x10];
    float height; // 0x2C
};

#define OFF(f, o) typedef char GridSurface801C9600_off_##f[((int)&((SpringGridSurfaceView*)0)->f == (o)) ? 1 : -1]
OFF(itemCount, 0x4);
OFF(groupCount, 0x8);
OFF(groups, 0xC);
OFF(vectors, 0x10);
OFF(updatedAt, 0x18);
OFF(height, 0x2C);
#undef OFF

#define AsGridSurface801C9600(p) ((SpringGridSurfaceView*)(p))

#endif
