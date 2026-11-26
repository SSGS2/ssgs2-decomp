// CQuadTree: the course's ground and collision grid.
//
// The shared instance holds the course surfaces and their collision triangles
// in a 20-unit X/Z grid, answers ground and object queries at a point or along
// a sweep, and runs the ball's collision simulation. Fields are named at the
// offsets the game uses; unknown spans are padding.
#ifndef CLASSES_CQUADTREE_H
#define CLASSES_CQUADTREE_H

#include "game/types.h"
#include "game/wvector.h"

class CQuadTree {
public:
    void* vtbl;                              // 0x0: original vtable pointer
    int courseData;                          // 0x4
    int groundSurface;                       // 0x8
    int minCellX;                            // 0xc: GetObjList
    int minCellZ;                            // 0x10: GetObjList
    int maxCellX;                            // 0x14: GetObjList
    int maxCellZ;                            // 0x18: GetObjList
    int candidateTriangleIds[2048];          // 0x1c: GetMeshIndex, GetHighestGround, GetLowestGround
    int candidateTriangleCount;              // 0x201c: GetMeshIndex, GetHighestGround, GetLowestGround
    short candidateObjectIds[32];            // 0x2020: BuildProjectedPatchMesh
    unsigned char candidateObjectCount;      // 0x2060: GetObjList, FindNearestSweptBallObjectHit, OverlappedWithObject
    char pad_2061[0x3];                      // 0x2061
    int gridColumns;                         // 0x2064: CreateSurfaceObject, GetMeshList
    int gridRows;                            // 0x2068: CreateSurfaceObject, GetMeshList
    float gridOriginX;                       // 0x206c: CreateSurfaceObject, GetMeshList
    char pad_2070[0x4];                      // 0x2070
    float gridOriginZ;                       // 0x2074: CreateSurfaceObject, GetMeshList
    void* groundRegions;                     // 0x2078
    unsigned int groundRegionCount;          // 0x207c
    unsigned int groundRegionCapacity;       // 0x2080
    char pad_2084[0x4];                      // 0x2084
    float recoveryPositionY;                 // 0x2088: FindBallRecoveryPosition
    char pad_208C[0x4];                      // 0x208c
    float frictionScale;                     // 0x2090
    float bounceScale;                       // 0x2094
    float condition;                         // 0x2098: SetCondition, GetMaxPowerOnGround, GetMinPowerOnGround
    float greenBoxMinX;                      // 0x209c: CalcGreenPlane
    char pad_20A0[0x4];                      // 0x20a0
    float greenBoxMinZ;                      // 0x20a4: CalcGreenPlane
    float greenBoxMaxX;                      // 0x20a8: CalcGreenPlane
    char pad_20AC[0x4];                      // 0x20ac
    float greenBoxMaxZ;                      // 0x20b0: CalcGreenPlane
    float greenCenterX;                      // 0x20b4
    float greenCenterY;                      // 0x20b8
    float greenCenterZ;                      // 0x20bc
    float greenFactor;                       // 0x20c0
    float greenPlaneNormalX;                 // 0x20c4: GetGreenPos
    float greenPlaneNormalY;                 // 0x20c8: GetGreenPos
    float greenPlaneNormalZ;                 // 0x20cc: GetGreenPos
    float greenPlaneConstant;                // 0x20d0: CalcGreenPlane, GetGreenPos
    char pad_20D4[0x4];                      // 0x20d4
    int regionBounceCount;                   // 0x20d8: UpdateAirborneBall, GetBounceCount
    void* vertexScratch;                     // 0x20dc: ClipPolygonToSixPlanes, BuildProjectedPatchMesh
    int vertexScratchCount;                  // 0x20e0: ClipPolygonToSixPlanes, BuildProjectedPatchMesh
    int vertexScratchCapacity;               // 0x20e4: ClipPolygonToSixPlanes, BuildProjectedPatchMesh
    char pad_20E8[0x60];                     // 0x20e8
    void* cells;                             // 0x2148
    char worldBounds[0x1c];                  // 0x214c
    float cupRadius;                         // 0x2168
    char pad_216C[0x14];                     // 0x216c
    float minimumGroundHeight;               // 0x2180: ResolveGroundHeight
    char groundCollisionSkipped;             // 0x2184: SweepBallAgainstTriangles, ResolveGroundHeight
    unsigned char windEnabled;               // 0x2185: ComputeAirborneBallForces
    unsigned char candidateTriangleOverflow; // 0x2186: IntersectBallSweepWithObjectTriangles
    char pad_2187[0x1];                      // 0x2187
    int simulationMode;                      // 0x2188
    char pad_218C[0x18];                     // 0x218c
    int unk_21A4;                            // 0x21a4
    char pad_21A8[0x8];                      // 0x21a8
    void* objects;                           // 0x21b0
    unsigned int objectCount;                // 0x21b4
    unsigned int objectCapacity;             // 0x21b8
    unsigned int effectRandomState;          // 0x21bc: CreateSurfaceObject
    float slopeGravityScale;                 // 0x21c0
    unsigned char lastHitObjectType;         // 0x21c4: FindNearestSweptBallObjectHit
    char pad_21C5[0x1];                      // 0x21c5
    short lastHitObjectIndex;                // 0x21c6: FindNearestSweptBallObjectHit, UpdateAirborneBall
    int lastHitTriangleIndex;                // 0x21c8: FindNearestSweptBallObjectHit
    int lastHitHistoryIndex;                 // 0x21cc: FindNearestSweptBallObjectHit
    char pad_21D0[0x18];                     // 0x21d0
    void* collisionEvents;                   // 0x21e8
    unsigned int collisionEventCount;        // 0x21ec
    unsigned int collisionEventCapacity;     // 0x21f0
    char pad_21F4[0x18];                     // 0x21f4
    int objectSoundEnabled;                  // 0x220c
    void* vertexBuffer;                      // 0x2210
    void* indexBuffer;                       // 0x2214

    void CreateSurfaceObject(short index);
    static void AdjustPointToRegionAndGroundClearance(void* outPoint, void* quadTree, void* referencePoint);
    float GetHighestGround(float* point, int* outIndex);
    int GetMeshIndex(void* point, int checkRaisedRegions);
    float GetLowestGround(void* point);
    void GetMeshList(float* point);
    void GetMeshListAlongSegment(void* start, void* __restrict delta);
    void GetMeshListInBounds(float* bounds);
    void GetObjList(float* bounds);
    static void GetMeshNormal(void* out, void* self, int index);
    static void GetMeshNormalAtPoint(void* out, void* self, void* arg);
    void* GetTextureName(void* point);
    void MakeCollisionData(int index);
    void FindBallRecoveryPosition(void* regionBounds);
    void UpdateAnimatedObjects(float delta, int forceTick);
    void UpdateAirborneBall(float dt);
    void StepBallSimulation(float dt);
    float SweepBallAgainstTriangles(void* startPosition, float* displacement, int* outTriangleIndex, int queryMode);
    float FindNearestSweptBallObjectHit(float* box, float* direction, float* outHitPlane);
    float GetGreenPos(void* point);
    void CalcGreenPlane();
    float GetGroundSlope(int index);
    static void ComputeBallGroundForces(float* out, void* self);
    WVector ComputeAirborneBallForces();
    void UpdateBallTriangleCollision(float dt);
    void HandleBallGroundBounce(void* normal);
    void ApplyBallMaterialCollision(void* displacement, void* normal, float hitFraction);
    int TestHighSpeedGroundImpact(void* normal);
    static float IntersectBallSweepWithPoint(void* unused, void* fixedTarget, void* movingOrigin, void* displacement);
    void UpdateBallCupCollision(float delta);
    void SetCondition();
    void SetSimulationMode(int mode);
    int GetGroundFlagBit1AtPoint(void* arg);
    int IsPenetrable(int materialIndex) const;
    float GetMaxPowerOnGround();
    float GetMinPowerOnGround();
    float GetMaxPowerOnGroundAtPoint(void* point);
    float GetMinPowerOnGroundAtPoint(void* point);
    char* GetBounceSound(int materialIndex);
    float FindObstructionSweepFraction(float* box, void* direction);
    int GetObjColNum();
    // Callers pass a point or an object; the argument is not examined.
    float DefaultLightingFactor(void* context);
    Vec3f GetRayIntersection(void* point, void* ray);
    float GetBounceCoeff(int materialIndex) const;
    void ReservedWiiHook();
    int GetBounceCount();
    float ResolveGroundHeight(Vec3f* point, float heightClearance);
    void PlayObjectSoundFromParsedToken(void* cursor, int index);
    void ParseBoneReferences(void* arg1, int index, void* arg3, void* arg4, void* arg5);
    void ClipPolygonToSixPlanes(void* points, int count, int planeIndex);
    static void* FindProjectedPatchCacheEntry(void* unused, float* corners, float* projectionDirection);
    void DrawProjectedTexturedPatch(float amount, float alpha, void* material, void* worldCenter, void* worldHalfAxisU,
                                    void* worldHalfAxisV, void* unused, void* includeObjectsAbi);
    int IntersectBallSweepWithObjectTriangles(void* displacement, int surfaceIndex, void* outHitPlane,
                                              float* bestFraction, int* outTriangleIndex);

    void FreeBuffers();
    // The collision-mesh count that callers pass is not used.
    void ClearObjects(int collisionMeshCount);

    void InitGroundOverrideBounds();

    CQuadTree();
    ~CQuadTree();

    void ResetPerShotSimulationState();
};

#define CQUADTREE_OFF(f, o) typedef char off_##f[((int)&((CQuadTree*)0)->f == (o)) ? 1 : -1]
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

#endif // CLASSES_CQUADTREE_H
