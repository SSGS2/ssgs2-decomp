#ifndef CLASSES_MODELRESOURCE801BD2B8_H
#define CLASSES_MODELRESOURCE801BD2B8_H

#include "game/types.h"

class ModelResource801BD2B8 {
public:
    char pad_000[4];                // 0x0: unmodelled bytes
    char pad_004[0x5b];             // 0x4
    char filenameTerminator;        // 0x5f: CourseModel_LoadFromFile original object access
    char pad_060[0x4];              // 0x60
    int formatTag;                  // 0x64: CourseModel_LoadFromFile original object access
    int baseMeshCount;              // 0x68: CourseModel_LoadFromFile original object access
    int extraMeshCount0;            // 0x6c: CourseModel_LoadFromFile original object access
    int extraMeshCount1;            // 0x70: CourseModel_LoadFromFile original object access
    int cameraCount;                // 0x74: CourseModel_LoadFromFile original object access
    int sceneMarkerCount;           // 0x78: CourseModel_LoadFromFile original object access
    int boundsCount;                // 0x7c: CourseModel_LoadFromFile original object access
    int bitmapEntryCount;           // 0x80: CourseModel_LoadFromFile original object access
    int regionDefinitionCount;      // 0x84: CourseModel_LoadFromFile original object access
    int selectedMeshCount;          // 0x88: CourseModel_LoadFromFile original object access
    void* bitmapEntries;            // 0x8c: CourseModel_LoadFromFile original object access
    char pad_090[0x8];              // 0x90
    void* cameras;                  // 0x98: CourseModel_LoadFromFile original object access
    void* bounds;                   // 0x9c: CourseModel_LoadFromFile original object access
    void* sceneMarkers;             // 0xa0: CourseModel_LoadFromFile original object access
    void* meshes;                   // 0xa4: CourseModel_LoadFromFile original object access
    void* fallbackSurface;          // 0xa8: CourseModel_LoadFromFile original object access
    char pad_0AC[0x14];             // 0xac
    void* entryContainer;           // 0xc0: CourseModel_LoadFromFile original object access
    char pad_0C4[0x1];              // 0xc4
    char resourceCapabilityLatched; // 0xc5: CourseModel_LoadFromFile original object access
    char pad_0C6[0x480e];           // 0xc6
    int attachedEffectCount;        // 0x48d4: CourseModel_LoadFromFile original object access
    char regionMode;                // 0x48d8: CourseModel_LoadFromFile original object access
    char pad_48D9[0x53];            // 0x48d9
    char surfaceBinaryLoadFailed;   // 0x492c: CourseModel_LoadFromFile original object access
    char pad_492D[0xf];             // 0x492d
    float fogStart;                 // 0x493c: CourseModel_LoadFromFile original object access
    float extendedFogStart;         // 0x4940: CourseModel_LoadFromFile stores three times fogStart
    float fogEnd;                   // 0x4944: CourseModel_LoadFromFile original object access
    float extendedFogEnd;           // 0x4948: CourseModel_LoadFromFile stores three times fogEnd
};

#define MODELRESOURCE801BD2B8_OFF(f, o) \
    typedef char ModelResource801BD2B8_off_##f[((int)&((ModelResource801BD2B8*)0)->f == (o)) ? 1 : -1]
MODELRESOURCE801BD2B8_OFF(filenameTerminator, 0x5f);
MODELRESOURCE801BD2B8_OFF(formatTag, 0x64);
MODELRESOURCE801BD2B8_OFF(baseMeshCount, 0x68);
MODELRESOURCE801BD2B8_OFF(extraMeshCount0, 0x6c);
MODELRESOURCE801BD2B8_OFF(extraMeshCount1, 0x70);
MODELRESOURCE801BD2B8_OFF(cameraCount, 0x74);
MODELRESOURCE801BD2B8_OFF(sceneMarkerCount, 0x78);
MODELRESOURCE801BD2B8_OFF(boundsCount, 0x7c);
MODELRESOURCE801BD2B8_OFF(bitmapEntryCount, 0x80);
MODELRESOURCE801BD2B8_OFF(regionDefinitionCount, 0x84);
MODELRESOURCE801BD2B8_OFF(selectedMeshCount, 0x88);
MODELRESOURCE801BD2B8_OFF(bitmapEntries, 0x8c);
MODELRESOURCE801BD2B8_OFF(cameras, 0x98);
MODELRESOURCE801BD2B8_OFF(bounds, 0x9c);
MODELRESOURCE801BD2B8_OFF(sceneMarkers, 0xa0);
MODELRESOURCE801BD2B8_OFF(meshes, 0xa4);
MODELRESOURCE801BD2B8_OFF(fallbackSurface, 0xa8);
MODELRESOURCE801BD2B8_OFF(entryContainer, 0xc0);
MODELRESOURCE801BD2B8_OFF(resourceCapabilityLatched, 0xc5);
MODELRESOURCE801BD2B8_OFF(attachedEffectCount, 0x48d4);
MODELRESOURCE801BD2B8_OFF(regionMode, 0x48d8);
MODELRESOURCE801BD2B8_OFF(surfaceBinaryLoadFailed, 0x492c);
MODELRESOURCE801BD2B8_OFF(fogStart, 0x493c);
MODELRESOURCE801BD2B8_OFF(extendedFogStart, 0x4940);
MODELRESOURCE801BD2B8_OFF(fogEnd, 0x4944);
MODELRESOURCE801BD2B8_OFF(extendedFogEnd, 0x4948);
#undef MODELRESOURCE801BD2B8_OFF

#endif // CLASSES_MODELRESOURCE801BD2B8_H
