// CSceneManager: the scene's object pools and draw lists.
//
// The four pools own allocation blocks of scene objects; the lists gather
// records for one frame. The class is declared without virtual functions on
// purpose: its vtable is taken from the original game data, so `vtbl` is
// spelled out as a member.
#ifndef CLASSES_CSCENEMANAGER_H
#define CLASSES_CSCENEMANAGER_H

#include "game/types.h"

class CSceneManager {
public:
    void* vtbl;                               // 0x0: original vtable pointer
    char pad_004[0xc];                        // 0x4
    int firstBeachNode;                       // 0x10
    int beachIterator;                        // 0x14
    void* beachAllocationBlocks;              // 0x18
    int freeBeachNodes;                       // 0x1c
    int beachAllocationBatchSize;             // 0x20
    int beachBucketMask;                      // 0x24
    int beachBucketCount;                     // 0x28
    void* beachBuckets;                       // 0x2c
    char beachPoolMutex[0x18];                // 0x30
    int firstRiverNode;                       // 0x48
    int riverIterator;                        // 0x4c
    void* riverAllocationBlocks;              // 0x50
    int freeRiverNodes;                       // 0x54
    int riverAllocationBatchSize;             // 0x58
    int riverBucketMask;                      // 0x5c
    int riverBucketCount;                     // 0x60
    void* riverBuckets;                       // 0x64
    char riverPoolMutex[0x18];                // 0x68
    int firstGeneratedTextureNode;            // 0x80
    int generatedTextureIterator;             // 0x84
    void* generatedTextureAllocationBlocks;   // 0x88
    int freeGeneratedTextureNodes;            // 0x8c
    int generatedTextureAllocationBatchSize;  // 0x90
    int generatedTextureBucketMask;           // 0x94
    int generatedTextureBucketCount;          // 0x98
    void* generatedTextureBuckets;            // 0x9c
    char generatedTexturePoolMutex[0x18];     // 0xa0
    int firstBoundaryNode;                    // 0xb8
    int boundaryIterator;                     // 0xbc
    void* boundaryAllocationBlocks;           // 0xc0
    int freeBoundaryNodes;                    // 0xc4
    int boundaryAllocationBatchSize;          // 0xc8
    int boundaryBucketMask;                   // 0xcc
    int boundaryBucketCount;                  // 0xd0
    void* boundaryBuckets;                    // 0xd4
    char boundaryPoolMutex[0x18];             // 0xd8
    void* generatedSurface;                   // 0xf0
    void* marker;                             // 0xf4
    void* weatherEffect;                      // 0xf8
    void* secondaryWeatherEffect;             // 0xfc
    unsigned char sceneViewOptionEnabled;     // 0x100
    unsigned char unk_101;                    // 0x101
    unsigned char alternateRenderSelection;   // 0x102
    char pad_103[0x1];                        // 0x103
    int renderRecordCount;                    // 0x104
    void* lastRenderRecord;                   // 0x108
    void* firstRenderRecord;                  // 0x10c
    int overlayDrawableCount;                 // 0x110
    void* lastOverlayDrawable;                // 0x114
    void* firstOverlayDrawable;               // 0x118
    int deferredDrawableCount;                // 0x11c
    void* lastDeferredDrawable;               // 0x120
    void* firstDeferredDrawable;              // 0x124
    int transparentObjectCount;               // 0x128
    void* lastTransparentObject;              // 0x12c
    void* firstTransparentObject;             // 0x130
    char pad_134[0x4];                        // 0x134
    int debugPointCount;                      // 0x138
    void* lastDebugPoint;                     // 0x13c
    void* firstDebugPoint;                    // 0x140
    char pad_144[0x1];                        // 0x144
    unsigned char objectNamesVisible;         // 0x145
    char pad_146[0x2];                        // 0x146
    int renderEnabled;                        // 0x148
    int unk_14C;                              // 0x14c
    int debugNameFont;                        // 0x150
    char pad_154[0xc];                        // 0x154
    unsigned char cameraProximityFadeEnabled; // 0x160
    unsigned char effectRenderPassActive;     // 0x161
    char pad_162[0x2];                        // 0x162
    int previousCameraMode;                   // 0x164
    int debugDrawFlags;                       // 0x168

    static void RemoveUnretainedRenderRecords(void* object);

    void RenderCourseScene(void* camera);

    // Course scene ownership: the objects built from the course data, and the
    // pass that releases them.
    void CreateCourseObjects();
    void ClearOwnedSceneObjects();

    CSceneManager();
    ~CSceneManager();
};

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

#endif // CLASSES_CSCENEMANAGER_H
