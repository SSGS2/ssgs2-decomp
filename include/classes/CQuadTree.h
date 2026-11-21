// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CQUADTREE_H
#define CLASSES_CQUADTREE_H

#include "game/types.h"

class CQuadTree {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int courseData;                          // 0x4: 1 use in CQuadTree_Ctor
    int groundSurface;                          // 0x8: 1 use in CQuadTree_Ctor
    int minCellX; // 0xc: CQuadTree_GetObjList
    int minCellZ; // 0x10: CQuadTree_GetObjList
    int maxCellX; // 0x14: CQuadTree_GetObjList
    int maxCellZ; // 0x18: CQuadTree_GetObjList
    int candidateTriangleIds[2048]; // 0x1c: CQuadTree_GetMeshIndex, CQuadTree_GetHighestGround, CQuadTree_GetLowestGround
    int candidateTriangleCount; // 0x201c: CQuadTree_GetMeshIndex, CQuadTree_GetHighestGround, CQuadTree_GetLowestGround
    short candidateObjectIds[32]; // 0x2020: CQuadTree_BuildProjectedPatchMesh
    unsigned char candidateObjectCount; // 0x2060: CQuadTree_GetObjList, CQuadTree_FindNearestSweptBallObjectHit, CQuadTree_OverlappedWithObject
    char pad_2061[0x3]; // 0x2061
    int gridColumns; // 0x2064: CQuadTree_CreateSurfaceObject, CQuadTree_GetMeshList
    int gridRows; // 0x2068: CQuadTree_CreateSurfaceObject, CQuadTree_GetMeshList
    float gridOriginX; // 0x206c: CQuadTree_CreateSurfaceObject, CQuadTree_GetMeshList
    char pad_2070[0x4]; // 0x2070
    float gridOriginZ; // 0x2074: CQuadTree_CreateSurfaceObject, CQuadTree_GetMeshList
    int groundRegions;                       // 0x2078: 2 uses in CQuadTree_Ctor
    unsigned int groundRegionCount;              // 0x207c: 3 uses in CQuadTree_Ctor
    unsigned int groundRegionCapacity;              // 0x2080: 2 uses in CQuadTree_Ctor
    char pad_2084[0x4]; // 0x2084
    float recoveryPositionY; // 0x2088: CQuadTree_FindBallRecoveryPosition
    char pad_208C[0x4]; // 0x208c
    float frictionScale;                     // 0x2090: 1 use in CQuadTree_Ctor
    float bounceScale;                     // 0x2094: 1 use in CQuadTree_Ctor
    float condition; // 0x2098: CQuadTree_SetCondition, CQuadTree_GetMaxPowerOnGround, CQuadTree_GetMinPowerOnGround
    float greenBoxMinX; // 0x209c: CQuadTree_CalcGreenPlane
    char pad_20A0[0x4]; // 0x20a0
    float greenBoxMinZ; // 0x20a4: CQuadTree_CalcGreenPlane
    float greenBoxMaxX; // 0x20a8: CQuadTree_CalcGreenPlane
    char pad_20AC[0x4]; // 0x20ac
    float greenBoxMaxZ; // 0x20b0: CQuadTree_CalcGreenPlane
    float greenCenterX; // 0x20b4
    float greenCenterY; // 0x20b8
    float greenCenterZ; // 0x20bc
    float greenFactor;                     // 0x20c0: 1 use in CQuadTree_Ctor
    float greenPlaneNormalX; // 0x20c4: CQuadTree_GetGreenPos
    float greenPlaneNormalY; // 0x20c8: CQuadTree_GetGreenPos
    float greenPlaneNormalZ; // 0x20cc: CQuadTree_GetGreenPos
    float greenPlaneConstant; // 0x20d0: CQuadTree_CalcGreenPlane, CQuadTree_GetGreenPos
    char pad_20D4[0x4]; // 0x20d4
    int regionBounceCount; // 0x20d8: CQuadTree_UpdateAirborneBall, CQuadTree_GetBounceCount
    void* vertexScratch; // 0x20dc: CQuadTree_ClipPolygonToSixPlanes, CQuadTree_BuildProjectedPatchMesh
    int vertexScratchCount; // 0x20e0: CQuadTree_ClipPolygonToSixPlanes, CQuadTree_BuildProjectedPatchMesh
    int vertexScratchCapacity; // 0x20e4: CQuadTree_ClipPolygonToSixPlanes, CQuadTree_BuildProjectedPatchMesh
    char pad_20E8[0x60]; // 0x20e8
    int cells;                       // 0x2148: 1 use in CQuadTree_Ctor
    char pad_214C[0x1c];                // 0x214c
    float cupRadius;                     // 0x2168: 1 use in CQuadTree_Ctor
    char pad_216C[0x14]; // 0x216c
    float minimumGroundHeight; // 0x2180: CQuadTree_ResolveGroundHeight
    char groundCollisionSkipped; // 0x2184: CQuadTree_SweepBallAgainstTriangles, CQuadTree_ResolveGroundHeight
    unsigned char windEnabled; // 0x2185: CQuadTree_ComputeAirborneBallForces
    unsigned char candidateTriangleOverflow; // 0x2186: CQuadTree_IntersectBallSweepWithObjectTriangles
    char pad_2187[0x1]; // 0x2187
    int simulationMode;                       // 0x2188: 1 use in CQuadTree_Ctor
    char pad_218C[0x18];                // 0x218c
    int unk_21A4;                       // 0x21a4: 1 use in CQuadTree_Ctor
    char pad_21A8[0x8];                 // 0x21a8
    void* objects;                     // 0x21b0: 2 uses in CQuadTree_Ctor
    unsigned int objectCount;              // 0x21b4: 3 uses in CQuadTree_Ctor
    unsigned int objectCapacity;              // 0x21b8: 2 uses in CQuadTree_Ctor
    unsigned int effectRandomState; // 0x21bc: CQuadTree_CreateSurfaceObject
    float slopeGravityScale;                     // 0x21c0: 1 use in CQuadTree_Ctor
    unsigned char lastHitObjectType; // 0x21c4: CQuadTree_FindNearestSweptBallObjectHit
    char pad_21C5[0x1]; // 0x21c5
    short lastHitObjectIndex; // 0x21c6: CQuadTree_FindNearestSweptBallObjectHit, CQuadTree_UpdateAirborneBall
    int lastHitTriangleIndex; // 0x21c8: CQuadTree_FindNearestSweptBallObjectHit
    int lastHitHistoryIndex; // 0x21cc: CQuadTree_FindNearestSweptBallObjectHit
    char pad_21D0[0x18]; // 0x21d0
    int collisionEvents;                       // 0x21e8: 2 uses in CQuadTree_Ctor
    unsigned int collisionEventCount;              // 0x21ec: 3 uses in CQuadTree_Ctor
    unsigned int collisionEventCapacity;              // 0x21f0: 2 uses in CQuadTree_Ctor
    char pad_21F4[0x18];                // 0x21f4
    int objectSoundEnabled;                       // 0x220c: 1 use in CQuadTree_Ctor
    void* vertexBuffer;                     // 0x2210: 3 uses in CQuadTree_Ctor
    void* indexBuffer;                     // 0x2214: 3 uses in CQuadTree_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x2078, 0x209c, 0x20a8, 0x214c, 0x2158, 0x21b0, 0x21e8, 0x2210, 0x2214
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CQUADTREE_OFF(f, o) typedef char CQuadTree_off_##f[((int)&((CQuadTree*)0)->f == (o)) ? 1 : -1]
CQUADTREE_OFF(courseData, 0x4);
CQUADTREE_OFF(groundSurface, 0x8);
CQUADTREE_OFF(groundRegions, 0x2078);
CQUADTREE_OFF(groundRegionCount, 0x207c);
CQUADTREE_OFF(groundRegionCapacity, 0x2080);
CQUADTREE_OFF(frictionScale, 0x2090);
CQUADTREE_OFF(bounceScale, 0x2094);
CQUADTREE_OFF(greenFactor, 0x20c0);
CQUADTREE_OFF(cells, 0x2148);
CQUADTREE_OFF(cupRadius, 0x2168);
CQUADTREE_OFF(simulationMode, 0x2188);
CQUADTREE_OFF(unk_21A4, 0x21a4);
CQUADTREE_OFF(objects, 0x21b0);
CQUADTREE_OFF(objectCount, 0x21b4);
CQUADTREE_OFF(objectCapacity, 0x21b8);
CQUADTREE_OFF(slopeGravityScale, 0x21c0);
CQUADTREE_OFF(collisionEvents, 0x21e8);
CQUADTREE_OFF(collisionEventCount, 0x21ec);
CQUADTREE_OFF(collisionEventCapacity, 0x21f0);
CQUADTREE_OFF(objectSoundEnabled, 0x220c);
CQUADTREE_OFF(vertexBuffer, 0x2210);
CQUADTREE_OFF(indexBuffer, 0x2214);
CQUADTREE_OFF(minCellX, 0xc);
CQUADTREE_OFF(minCellZ, 0x10);
CQUADTREE_OFF(maxCellX, 0x14);
CQUADTREE_OFF(maxCellZ, 0x18);
CQUADTREE_OFF(candidateTriangleIds, 0x1c);
CQUADTREE_OFF(candidateTriangleCount, 0x201c);
CQUADTREE_OFF(candidateObjectIds, 0x2020);
CQUADTREE_OFF(candidateObjectCount, 0x2060);
CQUADTREE_OFF(gridColumns, 0x2064);
CQUADTREE_OFF(gridRows, 0x2068);
CQUADTREE_OFF(gridOriginX, 0x206c);
CQUADTREE_OFF(gridOriginZ, 0x2074);
CQUADTREE_OFF(recoveryPositionY, 0x2088);
CQUADTREE_OFF(condition, 0x2098);
CQUADTREE_OFF(greenBoxMinX, 0x209c);
CQUADTREE_OFF(greenBoxMinZ, 0x20a4);
CQUADTREE_OFF(greenBoxMaxX, 0x20a8);
CQUADTREE_OFF(greenBoxMaxZ, 0x20b0);
CQUADTREE_OFF(greenPlaneNormalX, 0x20c4);
CQUADTREE_OFF(greenPlaneNormalY, 0x20c8);
CQUADTREE_OFF(greenPlaneNormalZ, 0x20cc);
CQUADTREE_OFF(greenPlaneConstant, 0x20d0);
CQUADTREE_OFF(regionBounceCount, 0x20d8);
CQUADTREE_OFF(vertexScratch, 0x20dc);
CQUADTREE_OFF(vertexScratchCount, 0x20e0);
CQUADTREE_OFF(vertexScratchCapacity, 0x20e4);
CQUADTREE_OFF(minimumGroundHeight, 0x2180);
CQUADTREE_OFF(groundCollisionSkipped, 0x2184);
CQUADTREE_OFF(windEnabled, 0x2185);
CQUADTREE_OFF(candidateTriangleOverflow, 0x2186);
CQUADTREE_OFF(effectRandomState, 0x21bc);
CQUADTREE_OFF(lastHitObjectType, 0x21c4);
CQUADTREE_OFF(lastHitObjectIndex, 0x21c6);
CQUADTREE_OFF(lastHitTriangleIndex, 0x21c8);
CQUADTREE_OFF(lastHitHistoryIndex, 0x21cc);
#undef CQUADTREE_OFF

#endif  // CLASSES_CQUADTREE_H
