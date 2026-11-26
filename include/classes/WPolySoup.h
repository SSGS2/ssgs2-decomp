// WPolySoup: partial layout used by the methods that reference it.
// Non-virtual layout view; unknown meanings retain their offset names.
#ifndef CLASSES_WPOLYSOUP_H
#define CLASSES_WPOLYSOUP_H

#include "game/types.h"

class WPolySoup {
public:
    char pad_000[0x10];           // 0x0
    unsigned int unk_10;          // 0x10: WPolySoup_Ctor
    unsigned int regions;         // 0x14: WPolySoup_Ctor
    unsigned int regionCount;     // 0x18: WPolySoup_Ctor
    unsigned int regionCapacity;  // 0x1c: WPolySoup_Ctor
    unsigned char unk_20;         // 0x20: WPolySoup_Ctor
    char pad_021[0x5F];           // 0x21
    int resourceEntryCount;       // 0x80: WPolySoup_ReleaseResources bounds the 0x28-byte resource-entry release loop
    char pad_084[0x4];            // 0x84
    unsigned int surfaceCount;    // 0x88: WPolySoup_Ctor
    unsigned int resourceEntries; // 0x8c: WPolySoup_Ctor
    unsigned int resourceCount;   // 0x90: WPolySoup_Ctor
    unsigned int resources;       // 0x94: WPolySoup_Ctor
    unsigned int cameras;         // 0x98: WPolySoup_Ctor
    unsigned int bounds;          // 0x9c: WPolySoup_Ctor
    unsigned int namedPoints;     // 0xa0: WPolySoup_Ctor
    unsigned int surfaces;        // 0xa4: WPolySoup_Ctor
    unsigned int fallbackSurface; // 0xa8: WPolySoup_Ctor
    unsigned int effectTreeCount; // 0xac: WPolySoup_Ctor
    unsigned int effectTreeRoot;  // 0xb0: WPolySoup_Ctor
    void* effectTreeFirst;        // 0xb4: WPolySoup_Ctor
    float unk_B8;                 // 0xb8: WPolySoup_Ctor
    float unk_BC;                 // 0xbc: WPolySoup_Ctor
    void* entryContainer;         // 0xc0: WPolySoup_Ctor
    unsigned char unk_C4;         // 0xc4: WPolySoup_Ctor
    unsigned char resourceCapabilityLatched; // 0xc5: WPolySoup_Ctor
    char pad_0C6[0x2];                       // 0xc6
    void* staticPetGroupVtable;              // 0xc8: WPolySoup_Ctor
    void* staticPetCount;                    // 0xcc: WPolySoup_Ctor
    void* staticPetEntries;                  // 0xd0: WPolySoup_Ctor
    char windActors[0x4800];                 // 0xd4: WPolySoup_ReleaseResources clears this region
    int windActorCount;                      // 0x48d4: WPolySoup_ReleaseResources resets after clearing windActors
    char pad_48D8[0x4];                      // 0x48d8
    unsigned int auxiliaryNodeCount;         // 0x48dc: WPolySoup_Ctor
    void* auxiliaryNodeHead;                 // 0x48e0: WPolySoup_Ctor
    void* auxiliaryNodeTail;                 // 0x48e4: WPolySoup_Ctor
    unsigned int auxiliaryVector1Data;       // 0x48e8: WPolySoup_Ctor
    unsigned int auxiliaryVector1Count;      // 0x48ec: WPolySoup_Ctor
    unsigned int auxiliaryVector1Capacity;   // 0x48f0: WPolySoup_Ctor
    unsigned int auxiliaryVector2Data;       // 0x48f4: WPolySoup_Ctor
    unsigned int auxiliaryVector2Count;      // 0x48f8: WPolySoup_Ctor
    unsigned int auxiliaryVector2Capacity;   // 0x48fc: WPolySoup_Ctor
    unsigned int ambientSoundCount;          // 0x4900: WPolySoup_Ctor
    void* ambientSoundHead;                  // 0x4904: WPolySoup_Ctor
    void* ambientSoundTail;                  // 0x4908: WPolySoup_Ctor
    unsigned int generatedCount;             // 0x490c: WPolySoup_Ctor
    void* generatedHead;                     // 0x4910: WPolySoup_Ctor
    void* generatedTail;                     // 0x4914: WPolySoup_Ctor
    char pad_4918[0x4];                      // 0x4918
    float unk_491C;                          // 0x491c: WPolySoup_Ctor
    unsigned int resourceIdCount;            // 0x4920: WPolySoup_Ctor
    unsigned int resourceIds;                // 0x4924: WPolySoup_Ctor
    unsigned int resourceIdCapacity;         // 0x4928: WPolySoup_Ctor
    unsigned char surfaceBinaryLoadFailed;   // 0x492c: WPolySoup_Ctor
    char pad_492D[0x3];                      // 0x492d
    unsigned int resourceObjects;            // 0x4930: WPolySoup_Ctor
    unsigned int resourceObjectCount;        // 0x4934: WPolySoup_Ctor
    unsigned int resourceObjectCapacity;     // 0x4938: WPolySoup_Ctor
    float defaultFogStart;                   // 0x493c: WPolySoup_Ctor
    float alternateFogStart;                 // 0x4940: WPolySoup_Ctor
    float defaultFogEnd;                     // 0x4944: WPolySoup_Ctor
    float alternateFogEnd;                   // 0x4948: WPolySoup_Ctor
    char pad_494C[0x334];                    // 0x494c
    unsigned int pendingFilenameFlags;       // 0x4c80: WPolySoup_Ctor
    unsigned int pendingFilenameLength;      // 0x4c84: WPolySoup_Ctor
    unsigned int pendingFilenameData;        // 0x4c88: WPolySoup_Ctor
};

#define WPOLYSOUP_OFF(f, o) typedef char WPolySoup_off_##f[((int)&((WPolySoup*)0)->f == (o)) ? 1 : -1]
WPOLYSOUP_OFF(unk_10, 0x10);
WPOLYSOUP_OFF(regions, 0x14);
WPOLYSOUP_OFF(regionCount, 0x18);
WPOLYSOUP_OFF(regionCapacity, 0x1c);
WPOLYSOUP_OFF(unk_20, 0x20);
WPOLYSOUP_OFF(resourceEntryCount, 0x80);
WPOLYSOUP_OFF(surfaceCount, 0x88);
WPOLYSOUP_OFF(resourceEntries, 0x8c);
WPOLYSOUP_OFF(resourceCount, 0x90);
WPOLYSOUP_OFF(resources, 0x94);
WPOLYSOUP_OFF(cameras, 0x98);
WPOLYSOUP_OFF(bounds, 0x9c);
WPOLYSOUP_OFF(namedPoints, 0xa0);
WPOLYSOUP_OFF(surfaces, 0xa4);
WPOLYSOUP_OFF(fallbackSurface, 0xa8);
WPOLYSOUP_OFF(effectTreeCount, 0xac);
WPOLYSOUP_OFF(effectTreeRoot, 0xb0);
WPOLYSOUP_OFF(effectTreeFirst, 0xb4);
WPOLYSOUP_OFF(unk_B8, 0xb8);
WPOLYSOUP_OFF(unk_BC, 0xbc);
WPOLYSOUP_OFF(entryContainer, 0xc0);
WPOLYSOUP_OFF(unk_C4, 0xc4);
WPOLYSOUP_OFF(resourceCapabilityLatched, 0xc5);
WPOLYSOUP_OFF(staticPetGroupVtable, 0xc8);
WPOLYSOUP_OFF(staticPetCount, 0xcc);
WPOLYSOUP_OFF(staticPetEntries, 0xd0);
WPOLYSOUP_OFF(windActors, 0xd4);
WPOLYSOUP_OFF(windActorCount, 0x48d4);
WPOLYSOUP_OFF(auxiliaryNodeCount, 0x48dc);
WPOLYSOUP_OFF(auxiliaryNodeHead, 0x48e0);
WPOLYSOUP_OFF(auxiliaryNodeTail, 0x48e4);
WPOLYSOUP_OFF(auxiliaryVector1Data, 0x48e8);
WPOLYSOUP_OFF(auxiliaryVector1Count, 0x48ec);
WPOLYSOUP_OFF(auxiliaryVector1Capacity, 0x48f0);
WPOLYSOUP_OFF(auxiliaryVector2Data, 0x48f4);
WPOLYSOUP_OFF(auxiliaryVector2Count, 0x48f8);
WPOLYSOUP_OFF(auxiliaryVector2Capacity, 0x48fc);
WPOLYSOUP_OFF(ambientSoundCount, 0x4900);
WPOLYSOUP_OFF(ambientSoundHead, 0x4904);
WPOLYSOUP_OFF(ambientSoundTail, 0x4908);
WPOLYSOUP_OFF(generatedCount, 0x490c);
WPOLYSOUP_OFF(generatedHead, 0x4910);
WPOLYSOUP_OFF(generatedTail, 0x4914);
WPOLYSOUP_OFF(unk_491C, 0x491c);
WPOLYSOUP_OFF(resourceIdCount, 0x4920);
WPOLYSOUP_OFF(resourceIds, 0x4924);
WPOLYSOUP_OFF(resourceIdCapacity, 0x4928);
WPOLYSOUP_OFF(surfaceBinaryLoadFailed, 0x492c);
WPOLYSOUP_OFF(resourceObjects, 0x4930);
WPOLYSOUP_OFF(resourceObjectCount, 0x4934);
WPOLYSOUP_OFF(resourceObjectCapacity, 0x4938);
WPOLYSOUP_OFF(defaultFogStart, 0x493c);
WPOLYSOUP_OFF(alternateFogStart, 0x4940);
WPOLYSOUP_OFF(defaultFogEnd, 0x4944);
WPOLYSOUP_OFF(alternateFogEnd, 0x4948);
WPOLYSOUP_OFF(pendingFilenameFlags, 0x4c80);
WPOLYSOUP_OFF(pendingFilenameLength, 0x4c84);
WPOLYSOUP_OFF(pendingFilenameData, 0x4c88);
#undef WPOLYSOUP_OFF

#endif // CLASSES_WPOLYSOUP_H
