// CourseData801C: layout view from course_data_geometry_and_surfaces.cpp (0x801C3100-0x801C482C); provisional type name.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_COURSEDATA801C_H
#define CLASSES_COURSEDATA801C_H

struct CourseData801C {
    char pad_000[0x14];
    void* regions;   // 0x14: CourseData_IsPointOutsidePlayableRegion traverses 0x20-byte polygon records
    int regionCount; // 0x18: CourseData_IsPointOutsidePlayableRegion computes end of region array
    char pad_01c[0x58];
    int nameCount;       // 0x74: CourseData_FindNameRecord / CourseData_FindNamedRecord search 0x58-byte records
    int namedPointCount; // 0x78: CourseData_FindNamedPoint searches 0x3C-byte records
    int boundsCount;     // 0x7c: CourseData_FindBoundsContainingPoint tests 0x5C-byte bounding records
    char pad_080[0x8];
    int surfaceCount; // 0x88: CourseData_FindSurfaceByName searches 0xDC-byte records
    char pad_08c[0x4];
    int resourceCount; // 0x90: CourseData_ReleaseResources releases resource at record+0x20
    void* resources;   // 0x94: CourseData_ReleaseResources traverses 0x28-byte records
    void* names;       // 0x98: CourseData_FindNameRecord / CourseData_FindNamedRecord compare record names
    void* bounds;      // 0x9c: CourseData_FindBoundsContainingPoint reads min/max XYZ
    void* namedPoints; // 0xa0: CourseData_FindNamedPoint compares names at record+1
    void*
        surfaces; // 0xa4: CourseData_FindSurfaceByName / CourseSurfaceProperties_GetParticleWindScale use 0xDC-byte stride
    void* fallbackSurface; // 0xa8: CourseSurfaceProperties_GetParticleWindScale uses for index 0xFFFF
    char pad_0ac[0x14];
    void*
        entryContainer; // 0xc0: CourseData_GetEntryByIndex / CourseData_FindEntryByName forward embedded list at +0x14
    char pad_0c4[0x14];
    void* firstWindActorSpray;    // 0xd8: CourseData_UpdateWindActors visits this slot with a 0x48-byte row stride
    void* firstWindActorSequence; // 0xdc: CourseData_UpdateWindActors applies wind through CFxSequence_SetWindVector
    char pad_0e0[0x47f4];
    int windActorCount;       // 0x48d4: CourseData_UpdateWindActors updates actor slots with wind vectors
    unsigned char regionMode; // 0x48d8: CourseData_ClampPointToActiveRegion selects polygon type 6 or 1
    char pad_48d9[0x33];
    int generatedCount;  // 0x490c: CourseData_ApplyModelNodeTags increments after linking generated node
    void* generatedHead; // 0x4910: CourseData_ApplyModelNodeTags inserts allocated node
    char pad_4914[0x1c];
    void** resourceObjects;           // 0x4930: CourseData_ReleaseResources releases pointed-to objects
    unsigned int resourceObjectCount; // 0x4934: CourseData_ReleaseResources traverses and subtracts released count
};

#define OFF(f, o) typedef char CourseData801C_off_##f[((int)&((CourseData801C*)0)->f == (o)) ? 1 : -1]
OFF(regions, 0x14);
OFF(regionCount, 0x18);
OFF(nameCount, 0x74);
OFF(namedPointCount, 0x78);
OFF(boundsCount, 0x7c);
OFF(surfaceCount, 0x88);
OFF(resourceCount, 0x90);
OFF(resources, 0x94);
OFF(names, 0x98);
OFF(bounds, 0x9c);
OFF(namedPoints, 0xa0);
OFF(surfaces, 0xa4);
OFF(fallbackSurface, 0xa8);
OFF(entryContainer, 0xc0);
OFF(firstWindActorSpray, 0xd8);
OFF(firstWindActorSequence, 0xdc);
OFF(windActorCount, 0x48d4);
OFF(regionMode, 0x48d8);
OFF(generatedCount, 0x490c);
OFF(generatedHead, 0x4910);
OFF(resourceObjects, 0x4930);
OFF(resourceObjectCount, 0x4934);
#undef OFF
#define AsCourseData801C(p) ((CourseData801C*)(p))

#endif
