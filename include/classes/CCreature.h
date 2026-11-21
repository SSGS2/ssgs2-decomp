// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCREATURE_H
#define CLASSES_CCREATURE_H

#include "game/types.h"

class CCreature {
public:
    void* vtable;                        // 0x0: 4 uses in ntNpc_PolaShip_Ctor, ntPenguin_Duck_Ctor, ntNpc_PolaPenguin2_Ctor
    void* secondaryVtable;                        // 0x4: 7 uses in ntNpc_PolaShip_Ctor, CDisposable_Ctor, CDuck_Ctor...
    int flags;                          // 0x8: 10 uses in CDisposable_Ctor, CDuck_Ctor, CUfo_Ctor...
    float movementBoundsOrigin[3]; // 0xC: ntNpc_BlackSparrow_UpdateGridTargetAndMove: used as target-position origin
    float movementBoundsMax[3]; // 0x18: ntNpc_PolaShip_Ctor: Vec3_Zero
    int puppet;                         // 0x24: 10 uses in ntNpc_PolaPenguin2_Ctor, CDisposable_Ctor, CDuck_Ctor...
    float transform[12]; // 0x28: ntNpc_BlackSparrow_UpdateGridTargetAndMove: Matrix_CopyAffine; translation at +0x4C
    float animationTime;                       // 0x58: 10 uses in ntNpc_PolaPenguin2_Ctor, CDuck_Ctor, CUfo_Ctor...
    char pad_05C[0x4];                  // 0x5c
    int animationRange;                         // 0x60: 10 uses in ntNpc_PolaPenguin2_Ctor, CDuck_Ctor, CUfo_Ctor...
    float targetPosition[3]; // 0x64: ntNpc_BlackSparrow_UpdateGridTargetAndMove: target minus translation, normalized movement
    float turnRate;                       // 0x70: 4 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    float movementSpeed;                       // 0x74: 6 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    float bankAngle; // 0x78: ntNpc_BlackSparrow_Draw: Matrix_SetAxisRotation axis 2
    float maxBankAngle;                       // 0x7c: 4 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    char pad_080[0x4];                  // 0x80
    float heading;                       // 0x84: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    float cellSize;                       // 0x88: 4 uses in CDuck_Ctor, ntPenguin_Duck_Ctor, ntNpc_PolaPenguin1_Ctor...
    int gridXCount; // 0x8C: ntNpc_BlackSparrow_UpdateGridTargetAndMove: grid index modulo and X bounds
    int gridYCount; // 0x90: ntNpc_BlackSparrow_UpdateGridTargetAndMove: Y bounds
    int gridZCount; // 0x94: ntNpc_BlackSparrow_UpdateGridTargetAndMove: grid stride and Z bounds
    float gridStepX; // 0x98: ntNpc_BlackSparrow_UpdateGridTargetAndMove: X cell spacing and Random_RangeFloat bound
    float gridStepY; // 0x9C: ntNpc_BlackSparrow_UpdateGridTargetAndMove: Y cell spacing and Random_RangeFloat bound
    float gridStepZ; // 0xA0: ntNpc_BlackSparrow_UpdateGridTargetAndMove: Z cell spacing and Random_RangeFloat bound
    int historyCount; // 0xA4: ntNpc_BlackSparrow_UpdateGridTargetAndMove: history search and shift bound
    int history;                         // 0xa8: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    unsigned char opacity; // 0xAC: ntNpc_BlackSparrow_UpdateAnimationAndMovement: interpolated keyframe value
    unsigned char animationActive; // 0xAD: ntNpc_BlackSparrow_UpdateAnimationAndMovement: gates animation update
    char pad_0AE[0x2];
    float ballSpawnOffsetDistance;                       // 0xb0: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    int opacityKeyframeCount;                         // 0xb4: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    void* opacityKeyframeNext;                       // 0xb8: 8 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    void* opacityKeyframePrevious;                       // 0xbc: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    int targetPoints;                         // 0xc0: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    int targetPointCount;                         // 0xc4: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    int targetPointCapacity;                         // 0xc8: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    unsigned char unk_CC;               // 0xcc: 2 uses in CUfo_Ctor, ntPenguin_Ctor
    char pad_0CD[0x3];                  // 0xcd
    int pathPoints;                         // 0xd0: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    int pathPointCount;                         // 0xd4: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    int pathPointCapacity;                         // 0xd8: 9 uses in CDuck_Ctor, CUfo_Ctor, ntPenguin_Ctor...
    unsigned char pathCursor; // 0xDC: ntNpc_BlackSparrow_UpdateGridTargetAndMove: indexes 12-byte path positions
    char pad_0DD[0x3c];
    unsigned char recordPath; // 0x119: ntNpc_BlackSparrow_UpdateGridTargetAndMove: appends target position when set
    unsigned char movementEnabled;              // 0x11a: 3 uses in ntPenguin_Duck_Ctor, ntNpc_BlackSparrow_Ctor, ntNpc_PolaPenguin2_Ctor
    char pad_11B[0x1];                  // 0x11b
    int unk_11C;                        // 0x11c: 9 uses in ntPenguin_Duck_Ctor, CDisposable_Ctor, CDuck_Ctor...
    int unk_120;                        // 0x120: 9 uses in CDuck_Ctor, ntPenguin_Duck_Ctor, CUfo_Ctor... (CONFLICT sizes [1, 4])
    float unk_124;                      // 0x124: 6 uses in CDuck_Ctor, ntPenguin_Duck_Ctor, ntNpc_Dragon_Ctor...
    float unk_128;                      // 0x128: 7 uses in ntPenguin_Duck_Ctor, CDisposable_Ctor, CDuck_Ctor...
    void* sequenceEffectHandles;                      // 0x12c: 7 uses in CDuck_Ctor, ntPenguin_Duck_Ctor, ntNpc_Dragon_Ctor...
    void* sequenceEffectCount;                      // 0x130: 6 uses in ntPenguin_Duck_Ctor, CDisposable_Ctor, ntNpc_Dragon_Ctor...
    unsigned int sequenceEffectCapacity;               // 0x134: 2 uses in ntNpc_Dragon_Ctor, ntNpc_PolaShip_Ctor
    unsigned char unk_138;              // 0x138: 2 uses in ntNpc_Dragon_Ctor, ntNpc_PolaShip_Ctor (CONFLICT sizes [1, 4])
    char pad_139[0x3];                  // 0x139
    unsigned int unk_13C;               // 0x13c: 2 uses in ntNpc_Dragon_Ctor, ntNpc_PolaShip_Ctor
    unsigned int unk_140;               // 0x140: 1 use in ntNpc_PolaShip_Ctor
    unsigned int unk_144;               // 0x144: 1 use in ntNpc_PolaShip_Ctor
    float unk_148;                      // 0x148: 1 use in ntNpc_PolaShip_Ctor
    unsigned int unk_14C;               // 0x14c: 1 use in ntNpc_PolaShip_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x0, 0x4, 0xc, 0x18, 0x24, 0x58, 0x60, 0xb8, 0x120, 0x124, 0x128, 0x12c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CCREATURE_H
