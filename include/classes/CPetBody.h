// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CPETBODY_H
#define CLASSES_CPETBODY_H

#include "game/types.h"

class CPetBody {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* puppet;                        // 0x4: 26 uses in CPetBody_Draw, CPetBody_Ctor, CPetBody_Update...
    int shadowMode;                          // 0x8: 1 use in CPetBody_Ctor
    void* currentAnimation;                        // 0xc: 3 uses in CPetBody_Draw, CPetBody_ParseBoneReferences, CPetBody_ParseAndRegisterAttachment
    float animationTime;                       // 0x10: 2 uses in CPetBody_Draw
    float baseMatrixBasis[9]; // 0x14: CPetBody_UpdateLookAt (0x801BAA20), Matrix_CopyAffine
    unsigned int baseMatrixPositionX; // 0x38: CPetBody_SetYaw
    unsigned int baseMatrixPositionY; // 0x3c: CPetBody_SetYaw
    unsigned int baseMatrixPositionZ; // 0x40: CPetBody_SetYaw
    float matrixBasis[9]; // 0x44: CPetBody_SetRotation (0x801BA23C), Matrix_CopyAffine
    Arr3 matrixPosition;                        // 0x68: 2 uses in CPetBody_Draw
    float rotationX; // 0x74: CPetBody_SetRotation, CPetBody_UpdateLookAt
    float rotationY; // 0x78: CPetBody_SetRotation, CPetBody_UpdateLookAt
    float rotationZ; // 0x7c: CPetBody_SetRotation, CPetBody_UpdateLookAt
    float positionX; // 0x80: CPetBody_SetupLighting
    float positionY; // 0x84: CPetBody_UpdateMovement integrates verticalVelocity into height
    float positionZ; // 0x88: CPetBody_SetupLighting
    float movementDirection[3]; // 0x8c: CPetBody_UpdateMovement cross product with the up vector
    float uniformScale;                       // 0x98: 4 uses in CPetBody_Ctor, CPetBody_Draw
    unsigned int movementState; // 0x9c: CPetBody_StartMovement
    unsigned char flightMovementEnabled;               // 0xa0: 1 use in CPetBody_vfunc_16
    char pad_0A1[0x3]; // 0xa1
    float movementTarget[3]; // 0xa4: CPetBody_StartMovement copies destination; CPetBody_UpdateMovement computes distance
    float movementStart[3]; // 0xb0: CPetBody_StartMovement copies initial position
    float minimumHeight; // 0xbc: CPetBody_StartMovement
    float verticalVelocity; // 0xc0: CPetBody_StartMovement
    float movementPitchRadians; // 0xc4: CPetBody_StartMovement
    unsigned int flags;                // 0xc8: 14 uses in CPetBody_Draw, CPetBody_Update, CPetBody_Ctor...
    int instanceHead;                         // 0xcc: 2 uses in CPetBody_Ctor, CPetBody_DispatchCommandCallbacks
    int instanceIterator;                         // 0xd0: 2 uses in CPetBody_Ctor, CPetBody_DispatchCommandCallbacks
    int instanceNodeAllocationHead;                         // 0xd4: 1 use in CPetBody_Ctor
    int instanceFreeNodeHead;                         // 0xd8: 1 use in CPetBody_Ctor
    int unk_DC;                         // 0xdc: 1 use in CPetBody_Ctor
    int instanceHashMask;                         // 0xe0: 1 use in CPetBody_Ctor
    int instanceHashBucketCount;                         // 0xe4: 4 uses in CPetBody_Ctor
    int instanceHashBuckets;                         // 0xe8: 4 uses in CPetBody_Ctor
    char instanceMutex[0x18]; // 0xec: CPetBody_DispatchCommandCallbacks / NextInstance lock and unlock
    void* renderParameters;                      // 0x104: 3 uses in CPetBody_Draw, CPetBody_Ctor
    int renderParameterCount;                        // 0x108: 2 uses in CPetBody_Ctor, CPetBody_Draw
    int unk_10C;                        // 0x10c: 1 use in CPetBody_Ctor
    unsigned char renderParametersEnabled;              // 0x110: 1 use in CPetBody_Draw
    unsigned char defaultRenderParametersEnabled;              // 0x111: 1 use in CPetBody_Draw
    char pad_112[0x2];                  // 0x112
    int lightingMode;                        // 0x114: 1 use in CPetBody_Ctor
    float lightDirectionX;                      // 0x118: 1 use in CPetBody_Ctor
    float lightDirectionY;                      // 0x11c: 1 use in CPetBody_Ctor
    float lightDirectionZ;                      // 0x120: 1 use in CPetBody_Ctor
    int lightColor;                        // 0x124: 1 use in CPetBody_Ctor
    int ambientLightColor;                        // 0x128: 1 use in CPetBody_Ctor
    int unk_12C;                        // 0x12c: 1 use in CPetBody_Ctor
    int nameStringFlags;                        // 0x130: 1 use in CPetBody_Ctor
    int nameStringLength;                        // 0x134: 1 use in CPetBody_Ctor
    int nameStringData;                        // 0x138: 1 use in CPetBody_Ctor
    float rootMotionDeltaX; // 0x13c: CPetBody_ComputeAnimationStep vector accumulation
    float rootMotionDeltaY; // 0x140: same vector, existing rootMotionDeltaZ at 0x144
    float rootMotionDeltaZ; // 0x144: CPetBody_ComputeAnimationStep
    char pad_148[0xc]; // 0x148
    float animationSpeed;                      // 0x154: 1 use in CPetBody_Update
    float rootMotionScale;                      // 0x158: 1 use in CPetBody_Update
    unsigned char queuedAnimationName[32]; // 0x15c: CPetBody_ResetPlaybackAndLookAt, CPetBody_UpdateAnimation
    unsigned char queuedAnimationUsesRootMotion; // 0x17c: CPetBody_UpdateAnimation, CPetBody_SetNextAnimation
    char pad_17D[0x3]; // 0x17d
    unsigned int attachmentCount;               // 0x180: 2 uses in CPetBody_Ctor, CPetBody_ParseAndRegisterAttachment
    void* attachmentTail;                      // 0x184: 1 use in CPetBody_Ctor
    void* attachmentHead;                      // 0x188: 2 uses in CPetBody_Ctor, CPetBody_ParseAndRegisterAttachment
    void* followTarget; // 0x18c: CPetBody_ResetPlaybackAndLookAt, CPetBody_UpdateLookAt, CPetBody_SetLookAtTarget
    unsigned char turnToTarget; // 0x190: CPetBody_UpdateLookAt, CPetBody_SetLookAtTarget
    char pad_191[0x3]; // 0x191
    float lookAtPitch;                      // 0x194: 2 uses in CPetBody_Draw
    float lookAtYaw;                      // 0x198: 2 uses in CPetBody_Draw
    float targetPitch; // 0x19c: CPetBody_UpdateLookAt
    float targetYaw; // 0x1a0: CPetBody_UpdateLookAt
    int animationFinished; // 0x1a4: CPetBody_UpdateAnimation, CPetBody_PlayAnimationByName, CPetBody_GetAnimationFinished
    char unk_1A8;                       // 0x1a8: 1 use in CPetBody_Ctor
    char pad_1A9[0x3]; // 0x1a9
    int resourcePending; // 0x1ac: CPetBody_RequestModelLoad, CPetBody_PollModelLoad
    int unk_1B0;                        // 0x1b0: 1 use in CPetBody_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x14, 0x38, 0x44, 0x68, 0x80, 0x8c, 0xcc, 0xec, 0x104, 0x114, 0x130, 0x13c, 0x180, 0x184
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CPETBODY_OFF(f, o) typedef char CPetBody_off_##f[((int)&((CPetBody*)0)->f == (o)) ? 1 : -1]
CPETBODY_OFF(puppet, 0x4);
CPETBODY_OFF(shadowMode, 0x8);
CPETBODY_OFF(currentAnimation, 0xc);
CPETBODY_OFF(animationTime, 0x10);
CPETBODY_OFF(matrixPosition, 0x68);
CPETBODY_OFF(uniformScale, 0x98);
CPETBODY_OFF(flightMovementEnabled, 0xa0);
CPETBODY_OFF(flags, 0xc8);
CPETBODY_OFF(instanceHead, 0xcc);
CPETBODY_OFF(instanceIterator, 0xd0);
CPETBODY_OFF(instanceNodeAllocationHead, 0xd4);
CPETBODY_OFF(instanceFreeNodeHead, 0xd8);
CPETBODY_OFF(unk_DC, 0xdc);
CPETBODY_OFF(instanceHashMask, 0xe0);
CPETBODY_OFF(instanceHashBucketCount, 0xe4);
CPETBODY_OFF(instanceHashBuckets, 0xe8);
CPETBODY_OFF(renderParameters, 0x104);
CPETBODY_OFF(renderParameterCount, 0x108);
CPETBODY_OFF(unk_10C, 0x10c);
CPETBODY_OFF(renderParametersEnabled, 0x110);
CPETBODY_OFF(defaultRenderParametersEnabled, 0x111);
CPETBODY_OFF(lightingMode, 0x114);
CPETBODY_OFF(lightDirectionX, 0x118);
CPETBODY_OFF(lightDirectionY, 0x11c);
CPETBODY_OFF(lightDirectionZ, 0x120);
CPETBODY_OFF(lightColor, 0x124);
CPETBODY_OFF(ambientLightColor, 0x128);
CPETBODY_OFF(unk_12C, 0x12c);
CPETBODY_OFF(nameStringFlags, 0x130);
CPETBODY_OFF(nameStringLength, 0x134);
CPETBODY_OFF(nameStringData, 0x138);
CPETBODY_OFF(animationSpeed, 0x154);
CPETBODY_OFF(rootMotionScale, 0x158);
CPETBODY_OFF(attachmentCount, 0x180);
CPETBODY_OFF(attachmentTail, 0x184);
CPETBODY_OFF(attachmentHead, 0x188);
CPETBODY_OFF(lookAtPitch, 0x194);
CPETBODY_OFF(lookAtYaw, 0x198);
CPETBODY_OFF(unk_1A8, 0x1a8);
CPETBODY_OFF(unk_1B0, 0x1b0);
CPETBODY_OFF(baseMatrixPositionX, 0x38);
CPETBODY_OFF(baseMatrixPositionY, 0x3c);
CPETBODY_OFF(baseMatrixPositionZ, 0x40);
CPETBODY_OFF(rotationX, 0x74);
CPETBODY_OFF(rotationY, 0x78);
CPETBODY_OFF(rotationZ, 0x7c);
CPETBODY_OFF(positionX, 0x80);
CPETBODY_OFF(positionZ, 0x88);
CPETBODY_OFF(movementState, 0x9c);
CPETBODY_OFF(minimumHeight, 0xbc);
CPETBODY_OFF(verticalVelocity, 0xc0);
CPETBODY_OFF(movementPitchRadians, 0xc4);
CPETBODY_OFF(rootMotionDeltaZ, 0x144);
CPETBODY_OFF(queuedAnimationName, 0x15c);
CPETBODY_OFF(queuedAnimationUsesRootMotion, 0x17c);
CPETBODY_OFF(followTarget, 0x18c);
CPETBODY_OFF(turnToTarget, 0x190);
CPETBODY_OFF(targetPitch, 0x19c);
CPETBODY_OFF(targetYaw, 0x1a0);
CPETBODY_OFF(animationFinished, 0x1a4);
CPETBODY_OFF(resourcePending, 0x1ac);
CPETBODY_OFF(baseMatrixBasis, 0x14);
CPETBODY_OFF(matrixBasis, 0x44);
CPETBODY_OFF(positionY, 0x84);
CPETBODY_OFF(movementDirection, 0x8c);
CPETBODY_OFF(movementTarget, 0xa4);
CPETBODY_OFF(movementStart, 0xb0);
CPETBODY_OFF(instanceMutex, 0xec);
CPETBODY_OFF(rootMotionDeltaX, 0x13c);
CPETBODY_OFF(rootMotionDeltaY, 0x140);
#undef CPETBODY_OFF

#endif  // CLASSES_CPETBODY_H
