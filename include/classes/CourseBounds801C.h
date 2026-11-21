// CourseBounds801C: layout view from CourseData_FindBoundsContainingPoint (0x801C4598).
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_COURSEBOUNDS801C_H
#define CLASSES_COURSEBOUNDS801C_H

struct CourseBounds801C {
    char pad_000[0x44];
    float minX; // 0x44: CourseData_FindBoundsContainingPoint lower X comparison
    float minY; // 0x48: CourseData_FindBoundsContainingPoint lower Y comparison
    float minZ; // 0x4c: CourseData_FindBoundsContainingPoint lower Z comparison
    float maxX; // 0x50: CourseData_FindBoundsContainingPoint upper X comparison
    float maxY; // 0x54: CourseData_FindBoundsContainingPoint upper Y comparison
    float maxZ; // 0x58: CourseData_FindBoundsContainingPoint upper Z comparison
};

#define OFF(f, o) typedef char CourseBounds801C_off_##f[((int)&((CourseBounds801C*)0)->f == (o)) ? 1 : -1]
OFF(minX, 0x44);
OFF(minY, 0x48);
OFF(minZ, 0x4c);
OFF(maxX, 0x50);
OFF(maxY, 0x54);
OFF(maxZ, 0x58);
#undef OFF
#define AsCourseBounds801C(p) ((CourseBounds801C*)(p))

#endif

