// CourseSurface801C: layout view from CourseSurfaceProperties_GetParticleOpacity / CourseSurfaceProperties_GetParticleWindScale / CourseData_FindSurfaceByName.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_COURSESURFACE801C_H
#define CLASSES_COURSESURFACE801C_H

struct CourseSurface801C {
    char pad_000[0x2];
    unsigned char particleOpacity; // 0x2: CourseSurfaceProperties_GetParticleOpacity returns this byte
    char pad_003[0x5];
    float particleWindScale; // 0x8: CourseSurfaceProperties_GetParticleWindScale returns this scalar
    char pad_00c[0x38];
    char name; // 0x44: CourseData_FindSurfaceByName compares string at this address
};

#define OFF(f, o) typedef char CourseSurface801C_off_##f[((int)&((CourseSurface801C*)0)->f == (o)) ? 1 : -1]
OFF(particleOpacity, 0x2);
OFF(particleWindScale, 0x8);
OFF(name, 0x44);
#undef OFF
#define AsCourseSurface801C(p) ((CourseSurface801C*)(p))

#endif
