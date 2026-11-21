// CourseRegion801C: layout view from CourseData_IsPointOutsidePlayableRegion / CourseData_ClampPointToActiveRegion.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_COURSEREGION801C_H
#define CLASSES_COURSEREGION801C_H
#include "classes/Polygon801C.h"

struct CourseRegion801C {
    char pad_000[0x10];
    int type; // 0x10: CourseData_IsPointOutsidePlayableRegion selects inside/outside tests for types 1/2/3
    Polygon801C polygon; // 0x14: Polygon_ContainsPointXZ receives this embedded polygon
};

#define OFF(f, o) typedef char CourseRegion801C_off_##f[((int)&((CourseRegion801C*)0)->f == (o)) ? 1 : -1]
OFF(type, 0x10);
OFF(polygon, 0x14);
#undef OFF
#define AsCourseRegion801C(p) ((CourseRegion801C*)(p))

#endif

