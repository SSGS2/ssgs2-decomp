#ifndef CLASSES_CCREATURE_H
#define CLASSES_CCREATURE_H

#include "game/types.h"

struct CreaturePointRange;
class CCreature {
public:
    void* vtable;                  // 0x0
    void* secondaryVtable;         // 0x4
    int flags;                     // 0x8
    float movementBoundsOrigin[3]; // 0xC: ntNpc_BlackSparrow_UpdateGridTargetAndMove: used as target-position origin
    float movementBoundsMax[3];    // 0x18: ntNpc_PolaShip_Ctor: Vec3_Zero
    int puppet;                    // 0x24
    float transform[12]; // 0x28: ntNpc_BlackSparrow_UpdateGridTargetAndMove: Matrix_CopyAffine; translation at +0x4C
    float animationTime; // 0x58
    char pad_05C[0x4];   // 0x5c
    int animationRange;  // 0x60
    float targetPosition
        [3];        // 0x64: ntNpc_BlackSparrow_UpdateGridTargetAndMove: target minus translation, normalized movement
    float turnRate; // 0x70
    float movementSpeed;            // 0x74
    float bankAngle;                // 0x78: ntNpc_BlackSparrow_Draw: Matrix_SetAxisRotation axis 2
    float maxBankAngle;             // 0x7c
    char headingBankParameter[0x4]; // 0x80
    float heading;                  // 0x84
    float cellSize;                 // 0x88
    int gridXCount;                 // 0x8C: ntNpc_BlackSparrow_UpdateGridTargetAndMove: grid index modulo and X bounds
    int gridYCount;                 // 0x90: ntNpc_BlackSparrow_UpdateGridTargetAndMove: Y bounds
    int gridZCount;                 // 0x94: ntNpc_BlackSparrow_UpdateGridTargetAndMove: grid stride and Z bounds
    float gridStepX;  // 0x98: ntNpc_BlackSparrow_UpdateGridTargetAndMove: X cell spacing and Random_RangeFloat bound
    float gridStepY;  // 0x9C: ntNpc_BlackSparrow_UpdateGridTargetAndMove: Y cell spacing and Random_RangeFloat bound
    float gridStepZ;  // 0xA0: ntNpc_BlackSparrow_UpdateGridTargetAndMove: Z cell spacing and Random_RangeFloat bound
    int historyCount; // 0xA4: ntNpc_BlackSparrow_UpdateGridTargetAndMove: history search and shift bound
    int history;      // 0xa8
    unsigned char opacity;         // 0xAC: ntNpc_BlackSparrow_UpdateAnimationAndMovement: interpolated keyframe value
    unsigned char animationActive; // 0xAD: ntNpc_BlackSparrow_UpdateAnimationAndMovement: gates animation update
    char pad_0AE[0x2];
    float ballSpawnOffsetDistance; // 0xb0
    int opacityKeyframeCount;      // 0xb4
    void* opacityKeyframeNext;     // 0xb8
    void* opacityKeyframePrevious; // 0xbc
    int targetPoints;              // 0xc0
    int targetPointCount;          // 0xc4
    int targetPointCapacity;       // 0xc8
    unsigned char unk_CC;          // 0xcc
    char pad_0CD[0x3];             // 0xcd
    int pathPoints;                // 0xd0
    int pathPointCount;            // 0xd4
    int pathPointCapacity;         // 0xd8
    unsigned char pathCursor;      // 0xDC: ntNpc_BlackSparrow_UpdateGridTargetAndMove: indexes 12-byte path positions
    char pad_0DD[0x3c];
    unsigned char recordPath; // 0x119: ntNpc_BlackSparrow_UpdateGridTargetAndMove: appends target position when set
    unsigned char movementEnabled;       // 0x11a
    char pad_11B[0x1];                   // 0x11b
    int unk_11C;                         // 0x11c
    int unk_120;                         // 0x120
    float unk_124;                       // 0x124
    float unk_128;                       // 0x128
    void* sequenceEffectHandles;         // 0x12c
    void* sequenceEffectCount;           // 0x130
    unsigned int sequenceEffectCapacity; // 0x134
    unsigned char unk_138;               // 0x138
    char pad_139[0x3];                   // 0x139
    unsigned int unk_13C;                // 0x13c
    unsigned int unk_140;                // 0x140
    unsigned int unk_144;                // 0x144
    float unk_148;                       // 0x148
    unsigned int unk_14C;                // 0x14c

    int vfunc_26();

    void SetPathPointsAndOrient(CreaturePointRange* range, float value);
    void UpdateAnimationTimeAndOpacity(float delta);
    void UpdateWanderMovement(float dt);
    void Draw();

    // Whether a point lies outside the camera-facing dot threshold.
    int IsOutsideCameraFacingThreshold(void* src, float threshold);

    void* LoadPuppet(int arg);
    void* LoadPuppetResource(int resourceId, int queuePreload);
    void InitRandomGridCell(const void* worldBounds, unsigned int flags);
    // Randomize heading and place the creature on the ground at its ball-relative offset.
    void SpawnAtBallOffset();
    void AttachBoneMetadataEffect(void* boneEntry);

    int ParseBoneReferences(int parser, int argument);
    void PlaySoundFromParsedToken(int parser);

    void vfunc_36Thunk4(int forwarded1, int forwarded2);
    void vfunc_37Thunk4(int forwarded1);
    void DtorThunk4(int deleteFlag);

    void SetKind(unsigned int value);
    void SetAnimationActive(unsigned char value);
    void SetBallSpawnOffsetDistance(float value);

    void vfunc_25();
    void vfunc_4();

    ~CCreature();
};

#define CCREATURE_OFF(f, o) typedef char CCreature_off_##f[((int)&((CCreature*)0)->f == (o)) ? 1 : -1]
CCREATURE_OFF(vtable, 0x0);
CCREATURE_OFF(secondaryVtable, 0x4);
CCREATURE_OFF(flags, 0x8);
CCREATURE_OFF(puppet, 0x24);
CCREATURE_OFF(animationTime, 0x58);
CCREATURE_OFF(animationRange, 0x60);
CCREATURE_OFF(turnRate, 0x70);
CCREATURE_OFF(movementSpeed, 0x74);
CCREATURE_OFF(maxBankAngle, 0x7c);
CCREATURE_OFF(heading, 0x84);
CCREATURE_OFF(cellSize, 0x88);
CCREATURE_OFF(history, 0xa8);
CCREATURE_OFF(ballSpawnOffsetDistance, 0xb0);
CCREATURE_OFF(opacityKeyframeCount, 0xb4);
CCREATURE_OFF(opacityKeyframeNext, 0xb8);
CCREATURE_OFF(opacityKeyframePrevious, 0xbc);
CCREATURE_OFF(targetPoints, 0xc0);
CCREATURE_OFF(targetPointCount, 0xc4);
CCREATURE_OFF(targetPointCapacity, 0xc8);
CCREATURE_OFF(unk_CC, 0xcc);
CCREATURE_OFF(pathPoints, 0xd0);
CCREATURE_OFF(pathPointCount, 0xd4);
CCREATURE_OFF(pathPointCapacity, 0xd8);
CCREATURE_OFF(movementEnabled, 0x11a);
CCREATURE_OFF(unk_11C, 0x11c);
CCREATURE_OFF(unk_120, 0x120);
CCREATURE_OFF(unk_124, 0x124);
CCREATURE_OFF(unk_128, 0x128);
CCREATURE_OFF(sequenceEffectHandles, 0x12c);
CCREATURE_OFF(sequenceEffectCount, 0x130);
CCREATURE_OFF(sequenceEffectCapacity, 0x134);
CCREATURE_OFF(unk_138, 0x138);
CCREATURE_OFF(unk_13C, 0x13c);
CCREATURE_OFF(unk_140, 0x140);
CCREATURE_OFF(unk_144, 0x144);
CCREATURE_OFF(unk_148, 0x148);
CCREATURE_OFF(unk_14C, 0x14c);
CCREATURE_OFF(movementBoundsOrigin, 0xC);
CCREATURE_OFF(movementBoundsMax, 0x18);
CCREATURE_OFF(transform, 0x28);
CCREATURE_OFF(targetPosition, 0x64);
CCREATURE_OFF(bankAngle, 0x78);
CCREATURE_OFF(gridXCount, 0x8C);
CCREATURE_OFF(gridYCount, 0x90);
CCREATURE_OFF(gridZCount, 0x94);
CCREATURE_OFF(gridStepX, 0x98);
CCREATURE_OFF(gridStepY, 0x9C);
CCREATURE_OFF(gridStepZ, 0xA0);
CCREATURE_OFF(historyCount, 0xA4);
CCREATURE_OFF(opacity, 0xAC);
CCREATURE_OFF(animationActive, 0xAD);
CCREATURE_OFF(pathCursor, 0xDC);
CCREATURE_OFF(recordPath, 0x119);
#undef CCREATURE_OFF

#endif // CLASSES_CCREATURE_H
