// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSCENEMANAGER_H
#define CLASSES_CSCENEMANAGER_H

#include "game/types.h"

class CSceneManager {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc];                  // 0x4
    int firstBeachNode;                         // 0x10: 1 use in CSceneManager_Ctor
    int beachIterator;                         // 0x14: 1 use in CSceneManager_Ctor
    void* beachAllocationBlocks;                       // 0x18: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    int freeBeachNodes;                         // 0x1c: 1 use in CSceneManager_Ctor
    int beachAllocationBatchSize;                         // 0x20: 1 use in CSceneManager_Ctor
    int beachBucketMask;                         // 0x24: 1 use in CSceneManager_Ctor
    int beachBucketCount;                         // 0x28: 4 uses in CSceneManager_Ctor
    void* beachBuckets;                       // 0x2c: 6 uses in CSceneManager_Ctor, CSceneManager_dtor
    char beachPoolMutex[0x18];                 // 0x30
    int firstRiverNode;                         // 0x48: 1 use in CSceneManager_Ctor
    int riverIterator;                         // 0x4c: 1 use in CSceneManager_Ctor
    void* riverAllocationBlocks;                       // 0x50: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    int freeRiverNodes;                         // 0x54: 1 use in CSceneManager_Ctor
    int riverAllocationBatchSize;                         // 0x58: 1 use in CSceneManager_Ctor
    int riverBucketMask;                         // 0x5c: 1 use in CSceneManager_Ctor
    int riverBucketCount;                         // 0x60: 4 uses in CSceneManager_Ctor
    void* riverBuckets;                       // 0x64: 6 uses in CSceneManager_Ctor, CSceneManager_dtor
    char riverPoolMutex[0x18];                 // 0x68
    int firstGeneratedTextureNode;                         // 0x80: 1 use in CSceneManager_Ctor
    int generatedTextureIterator;                         // 0x84: 1 use in CSceneManager_Ctor
    void* generatedTextureAllocationBlocks;                       // 0x88: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    int freeGeneratedTextureNodes;                         // 0x8c: 1 use in CSceneManager_Ctor
    int generatedTextureAllocationBatchSize;                         // 0x90: 1 use in CSceneManager_Ctor
    int generatedTextureBucketMask;                         // 0x94: 1 use in CSceneManager_Ctor
    int generatedTextureBucketCount;                         // 0x98: 4 uses in CSceneManager_Ctor
    void* generatedTextureBuckets;                       // 0x9c: 6 uses in CSceneManager_Ctor, CSceneManager_dtor
    char generatedTexturePoolMutex[0x18];                 // 0xa0
    int firstBoundaryNode;                         // 0xb8: 1 use in CSceneManager_Ctor
    int boundaryIterator;                         // 0xbc: 1 use in CSceneManager_Ctor
    void* boundaryAllocationBlocks;                       // 0xc0: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    int freeBoundaryNodes;                         // 0xc4: 1 use in CSceneManager_Ctor
    int boundaryAllocationBatchSize;                         // 0xc8: 1 use in CSceneManager_Ctor
    int boundaryBucketMask;                         // 0xcc: 1 use in CSceneManager_Ctor
    int boundaryBucketCount;                         // 0xd0: 4 uses in CSceneManager_Ctor
    void* boundaryBuckets;                       // 0xd4: 6 uses in CSceneManager_Ctor, CSceneManager_dtor
    char boundaryPoolMutex[0x18];                 // 0xd8
    int generatedSurface;                         // 0xf0: 1 use in CSceneManager_Ctor
    void* marker;                       // 0xf4: 4 uses in CSceneManager_Ctor, CSceneManager_dtor
    void* weatherEffect;                       // 0xf8: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    void* secondaryWeatherEffect;                       // 0xfc: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    unsigned char sceneViewOptionEnabled;              // 0x100: 1 use in CSceneManager_Ctor
    unsigned char unk_101;              // 0x101: 1 use in CSceneManager_Ctor
    unsigned char alternateRenderSelection;              // 0x102: 2 uses in CSceneManager_Ctor
    char pad_103[0x1];                  // 0x103
    int renderRecordCount;                        // 0x104: 1 use in CSceneManager_Ctor
    void* lastRenderRecord;                      // 0x108: 1 use in CSceneManager_Ctor
    void* firstRenderRecord;                      // 0x10c: 4 uses in CSceneManager_dtor, CSceneManager_Ctor
    int overlayDrawableCount;                        // 0x110: 1 use in CSceneManager_Ctor
    void* lastOverlayDrawable;                      // 0x114: 1 use in CSceneManager_Ctor
    void* firstOverlayDrawable;                      // 0x118: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    int deferredDrawableCount;                        // 0x11c: 1 use in CSceneManager_Ctor
    void* lastDeferredDrawable;                      // 0x120: 1 use in CSceneManager_Ctor
    void* firstDeferredDrawable;                      // 0x124: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    int transparentObjectCount;                        // 0x128: 1 use in CSceneManager_Ctor
    void* lastTransparentObject;                      // 0x12c: 1 use in CSceneManager_Ctor
    void* firstTransparentObject;                      // 0x130: 3 uses in CSceneManager_dtor, CSceneManager_Ctor
    char pad_134[0x4];                  // 0x134
    int debugPointCount;                        // 0x138: 1 use in CSceneManager_Ctor
    void* lastDebugPoint;                      // 0x13c: 1 use in CSceneManager_Ctor
    void* firstDebugPoint;                      // 0x140: 2 uses in CSceneManager_Ctor, CSceneManager_dtor
    char pad_144[0x1];                  // 0x144
    unsigned char objectNamesVisible;              // 0x145: 1 use in CSceneManager_Ctor
    char pad_146[0x2];                  // 0x146
    int renderEnabled;                        // 0x148: 1 use in CSceneManager_Ctor
    int unk_14C;                        // 0x14c: 1 use in CSceneManager_Ctor
    int debugNameFont;                        // 0x150: 2 uses in CSceneManager_Ctor
    char pad_154[0xc];                  // 0x154
    unsigned char cameraProximityFadeEnabled;              // 0x160: 1 use in CSceneManager_Ctor
    unsigned char effectRenderPassActive;              // 0x161: 1 use in CSceneManager_Ctor
    char pad_162[0x2];                  // 0x162
    int previousCameraMode;                        // 0x164: 1 use in CSceneManager_Ctor
    int debugDrawFlags;                        // 0x168: 1 use in CSceneManager_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x10, 0x30, 0x48, 0x68, 0x80, 0xa0, 0xb8, 0xd8, 0x104, 0x108, 0x110, 0x114, 0x11c, 0x120, 0x128, 0x12c, 0x138, 0x13c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSCENEMANAGER_OFF(f, o) typedef char CSceneManager_off_##f[((int)&((CSceneManager*)0)->f == (o)) ? 1 : -1]
CSCENEMANAGER_OFF(firstBeachNode, 0x10);
CSCENEMANAGER_OFF(beachIterator, 0x14);
CSCENEMANAGER_OFF(beachAllocationBlocks, 0x18);
CSCENEMANAGER_OFF(freeBeachNodes, 0x1c);
CSCENEMANAGER_OFF(beachAllocationBatchSize, 0x20);
CSCENEMANAGER_OFF(beachBucketMask, 0x24);
CSCENEMANAGER_OFF(beachBucketCount, 0x28);
CSCENEMANAGER_OFF(beachBuckets, 0x2c);
CSCENEMANAGER_OFF(firstRiverNode, 0x48);
CSCENEMANAGER_OFF(riverIterator, 0x4c);
CSCENEMANAGER_OFF(riverAllocationBlocks, 0x50);
CSCENEMANAGER_OFF(freeRiverNodes, 0x54);
CSCENEMANAGER_OFF(riverAllocationBatchSize, 0x58);
CSCENEMANAGER_OFF(riverBucketMask, 0x5c);
CSCENEMANAGER_OFF(riverBucketCount, 0x60);
CSCENEMANAGER_OFF(riverBuckets, 0x64);
CSCENEMANAGER_OFF(firstGeneratedTextureNode, 0x80);
CSCENEMANAGER_OFF(generatedTextureIterator, 0x84);
CSCENEMANAGER_OFF(generatedTextureAllocationBlocks, 0x88);
CSCENEMANAGER_OFF(freeGeneratedTextureNodes, 0x8c);
CSCENEMANAGER_OFF(generatedTextureAllocationBatchSize, 0x90);
CSCENEMANAGER_OFF(generatedTextureBucketMask, 0x94);
CSCENEMANAGER_OFF(generatedTextureBucketCount, 0x98);
CSCENEMANAGER_OFF(generatedTextureBuckets, 0x9c);
CSCENEMANAGER_OFF(firstBoundaryNode, 0xb8);
CSCENEMANAGER_OFF(boundaryIterator, 0xbc);
CSCENEMANAGER_OFF(boundaryAllocationBlocks, 0xc0);
CSCENEMANAGER_OFF(freeBoundaryNodes, 0xc4);
CSCENEMANAGER_OFF(boundaryAllocationBatchSize, 0xc8);
CSCENEMANAGER_OFF(boundaryBucketMask, 0xcc);
CSCENEMANAGER_OFF(boundaryBucketCount, 0xd0);
CSCENEMANAGER_OFF(boundaryBuckets, 0xd4);
CSCENEMANAGER_OFF(generatedSurface, 0xf0);
CSCENEMANAGER_OFF(marker, 0xf4);
CSCENEMANAGER_OFF(weatherEffect, 0xf8);
CSCENEMANAGER_OFF(secondaryWeatherEffect, 0xfc);
CSCENEMANAGER_OFF(sceneViewOptionEnabled, 0x100);
CSCENEMANAGER_OFF(unk_101, 0x101);
CSCENEMANAGER_OFF(alternateRenderSelection, 0x102);
CSCENEMANAGER_OFF(renderRecordCount, 0x104);
CSCENEMANAGER_OFF(lastRenderRecord, 0x108);
CSCENEMANAGER_OFF(firstRenderRecord, 0x10c);
CSCENEMANAGER_OFF(overlayDrawableCount, 0x110);
CSCENEMANAGER_OFF(lastOverlayDrawable, 0x114);
CSCENEMANAGER_OFF(firstOverlayDrawable, 0x118);
CSCENEMANAGER_OFF(deferredDrawableCount, 0x11c);
CSCENEMANAGER_OFF(lastDeferredDrawable, 0x120);
CSCENEMANAGER_OFF(firstDeferredDrawable, 0x124);
CSCENEMANAGER_OFF(transparentObjectCount, 0x128);
CSCENEMANAGER_OFF(lastTransparentObject, 0x12c);
CSCENEMANAGER_OFF(firstTransparentObject, 0x130);
CSCENEMANAGER_OFF(debugPointCount, 0x138);
CSCENEMANAGER_OFF(lastDebugPoint, 0x13c);
CSCENEMANAGER_OFF(firstDebugPoint, 0x140);
CSCENEMANAGER_OFF(objectNamesVisible, 0x145);
CSCENEMANAGER_OFF(renderEnabled, 0x148);
CSCENEMANAGER_OFF(unk_14C, 0x14c);
CSCENEMANAGER_OFF(debugNameFont, 0x150);
CSCENEMANAGER_OFF(cameraProximityFadeEnabled, 0x160);
CSCENEMANAGER_OFF(effectRenderPassActive, 0x161);
CSCENEMANAGER_OFF(previousCameraMode, 0x164);
CSCENEMANAGER_OFF(debugDrawFlags, 0x168);
#undef CSCENEMANAGER_OFF

#endif  // CLASSES_CSCENEMANAGER_H
