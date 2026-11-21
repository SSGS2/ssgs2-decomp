// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGOLFCADDIE_H
#define CLASSES_CGOLFCADDIE_H

#include "game/types.h"

class CGolfCaddie {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;                       // 0x18: 2 uses in CGolfCaddie_Ctor, CGolfCaddie_dtor
    void* activeCaddie;                       // 0x1c: 24 uses in CGolfCaddie_OnProcess, CGolfCaddie_OnInit
    int caddieEntries;                         // 0x20: 4 uses in CGolfCaddie_Ctor, CGolfCaddie_dtor, CGolfCaddie_OnInit
    unsigned int caddieEntryCount;                // 0x24: 4 uses in CGolfCaddie_dtor, CGolfCaddie_Ctor, CGolfCaddie_OnInit
    int caddieEntryCapacity;                         // 0x28: 1 use in CGolfCaddie_Ctor
    char pad_02C[0x4];                  // 0x2c
    int animationState;                         // 0x30: 14 uses in CGolfCaddie_OnProcess
    void* runDustEffect;                       // 0x34: 1 use in CGolfCaddie_OnProcess
    float position[3];                  // 0x38: 0x8012A730 adds texture offset; CGolfCaddie_OnProcess anchors run dust here
    float movementDelay;                       // 0x44: 6 uses in CGolfCaddie_OnProcess, CGolfCaddie_OnInit
    float yaw;                       // 0x48: 4 uses in CGolfCaddie_OnProcess, CGolfCaddie_OnInit
    float movementTarget[3];            // 0x4c: CGolfCaddie_OnProcess subtracts position, then passes this vector to CPetBody_StartMovement
    char pad_058[0x1];                  // 0x58
    unsigned char caddieVisible;               // 0x59: 2 uses in CGolfCaddie_Ctor, CGolfCaddie_OnInit
    char pad_05A[0x2];                  // 0x5a
    float idleAnimationTimer;                       // 0x5c: 3 uses in CGolfCaddie_OnProcess
    char pad_060[0x4];                  // 0x60
    bool reactionModeOneEnabled;                        // 0x64: 2 uses in CGolfCaddie_OnInit, CGolfCaddie_HandleMsg
    char pad_065[0xb];                  // 0x65
    int effectEntryKey;                         // 0x70: 2 uses in CGolfCaddie_Ctor, CGolfCaddie_OnDestroy
    float cachedPlacementPosition[3];   // +0x74: saved preview placement
    float cachedPlacementYaw;          // +0x80: saved preview yaw (radians)
    void* texture3D;                       // 0x84: 6 uses in CGolfCaddie_OnProcess, CGolfCaddie_OnDestroy, CGolfCaddie_Ctor
    float affineMatrices[4][12];       // +0x88..+0x147: four affine matrices
    char reserved148[0x28];            // +0x148..+0x16f: unresolved
    unsigned int commentaryTimestampMs; // +0x170: commentary clock in milliseconds
    void* soundBankNames;                      // 0x174: 1 use in CGolfCaddie_Ctor
    void* soundBankNameCount;                      // 0x178: 1 use in CGolfCaddie_Ctor
    void* soundBankNameCapacity;                      // 0x17c: 1 use in CGolfCaddie_Ctor
    void* conteCaddies;                      // 0x180: 9 uses in CGolfCaddie_OnProcess, CGolfCaddie_dtor, CGolfCaddie_Ctor...
    void* storyConteCaddieOverride;                      // 0x184: 4 uses in CGolfCaddie_Ctor, CGolfCaddie_dtor, CGolfCaddie_OnDestroy...
    void* pendingModelLoadCount;                      // 0x188: 2 uses in CGolfCaddie_Ctor
    void* pendingModelLoadTreeRoot;                      // 0x18c: 3 uses in CGolfCaddie_Ctor
    char pad_190[0x4];                  // 0x190
    void* pendingModelLoadIterator;                      // 0x194: 2 uses in CGolfCaddie_Ctor
    void* petPreload;                      // 0x198: 4 uses in CGolfCaddie_dtor, CGolfCaddie_Ctor, CGolfCaddie_OnDestroy
    int petPreloadPlayerIndex;                        // 0x19c: 1 use in CGolfCaddie_Ctor
    void* petPreloadActive;                      // 0x1a0: 1 use in CGolfCaddie_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x20, 0x44, 0x74, 0x88, 0x174, 0x184, 0x188, 0x18c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGOLFCADDIE_OFF(f, o) typedef char CGolfCaddie_off_##f[((int)&((CGolfCaddie*)0)->f == (o)) ? 1 : -1]
typedef char CGolfCaddie_size_check[(sizeof(CGolfCaddie) == 0x1a4) ? 1 : -1];
CGOLFCADDIE_OFF(secondaryVtable, 0x18);
CGOLFCADDIE_OFF(activeCaddie, 0x1c);
CGOLFCADDIE_OFF(caddieEntries, 0x20);
CGOLFCADDIE_OFF(caddieEntryCount, 0x24);
CGOLFCADDIE_OFF(caddieEntryCapacity, 0x28);
CGOLFCADDIE_OFF(animationState, 0x30);
CGOLFCADDIE_OFF(runDustEffect, 0x34);
CGOLFCADDIE_OFF(position, 0x38);
CGOLFCADDIE_OFF(movementTarget, 0x4c);
CGOLFCADDIE_OFF(movementDelay, 0x44);
CGOLFCADDIE_OFF(yaw, 0x48);
CGOLFCADDIE_OFF(caddieVisible, 0x59);
CGOLFCADDIE_OFF(idleAnimationTimer, 0x5c);
CGOLFCADDIE_OFF(reactionModeOneEnabled, 0x64);
CGOLFCADDIE_OFF(effectEntryKey, 0x70);
CGOLFCADDIE_OFF(cachedPlacementPosition, 0x74);
CGOLFCADDIE_OFF(cachedPlacementYaw, 0x80);
CGOLFCADDIE_OFF(texture3D, 0x84);
CGOLFCADDIE_OFF(affineMatrices, 0x88);
CGOLFCADDIE_OFF(reserved148, 0x148);
CGOLFCADDIE_OFF(commentaryTimestampMs, 0x170);
CGOLFCADDIE_OFF(soundBankNames, 0x174);
CGOLFCADDIE_OFF(soundBankNameCount, 0x178);
CGOLFCADDIE_OFF(soundBankNameCapacity, 0x17c);
CGOLFCADDIE_OFF(conteCaddies, 0x180);
CGOLFCADDIE_OFF(storyConteCaddieOverride, 0x184);
CGOLFCADDIE_OFF(pendingModelLoadCount, 0x188);
CGOLFCADDIE_OFF(pendingModelLoadTreeRoot, 0x18c);
CGOLFCADDIE_OFF(pendingModelLoadIterator, 0x194);
CGOLFCADDIE_OFF(petPreload, 0x198);
CGOLFCADDIE_OFF(petPreloadPlayerIndex, 0x19c);
CGOLFCADDIE_OFF(petPreloadActive, 0x1a0);
#undef CGOLFCADDIE_OFF

#endif  // CLASSES_CGOLFCADDIE_H
