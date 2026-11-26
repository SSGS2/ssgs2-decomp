// CourseResource801C: layout view from CourseData_ReleaseResources.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_COURSERESOURCE801C_H
#define CLASSES_COURSERESOURCE801C_H

struct CourseResource801C {
    char pad_000[0x20];
    void* resourceId; // 0x20: CourseData_ReleaseResources passes to ResourceManager_ReleaseId and clears
};

#define OFF(f, o) typedef char CourseResource801C_off_##f[((int)&((CourseResource801C*)0)->f == (o)) ? 1 : -1]
OFF(resourceId, 0x20);
#undef OFF
#define AsCourseResource801C(p) ((CourseResource801C*)(p))

#endif
