// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CROUNDDOL3D_H
#define CLASSES_CROUNDDOL3D_H

#include "game/types.h"
#include "game/wvector.h"

class CRoundDol3D {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* dolphinModel;                        // 0x4: 1 use in CRoundDol3D_dtor
    void* dolphinStandModel;                        // 0x8: 1 use in CRoundDol3D_dtor
    void* backgroundModel;                        // 0xc: 1 use in CRoundDol3D_dtor
    void* capsuleModel;                       // 0x10: 1 use in CRoundDol3D_dtor
    unsigned int activeModel; // 0x14: CRoundDol3D_SetAnimationByIndex selects the model passed to AnimationEventParser_ProcessFrameRange
    float rotationAngle; // 0x18: CRoundDol3D_DecelerateRotation passes this angle to Matrix_SetAxisRotation
    char pad_01C[0x80];
    WVector cameraTarget; // 0x9C: CRoundDol3D_vfunc_24 copies a WVector here
    unsigned int cameraTargetOverrideEnabled; // 0xA8: CRoundDol3D_SetCameraTargetOverrideEnabled stores its value argument here
    char pad_0AC[0x4];
    float rotationSpeed; // 0xB0: CRoundDol3D_UpdateRotationFromPadMotion / CRoundDol3D_DecelerateRotation integrate and decay rotation speed
    unsigned int unk_B4; // 0xB4: CRoundDol3D_ResetRotation / CRoundDol3D_UpdateRotationFromPadMotion reset this word
    char pad_0B8[0x14];
    unsigned int speedLevel; // 0xCC: CRoundDol3D_UpdateRotationFromPadMotion selects levels 1..7 by rotation speed
    void* cameraMotionPlayer;                       // 0xd0: 2 uses in CRoundDol3D_dtor
    char motionSamples[0x180]; // 0xD4: CRoundDol3D_UpdateRotationFromPadMotion fills/clamps motion samples, 0xC bytes each
    unsigned int motionSampleCount; // 0x254: CRoundDol3D_UpdateRotationFromPadMotion receives and caps the input sample count
    char previousFirstMotionSample[0xC]; // 0x258: CRoundDol3D_ResampleMotion caches input[0] for the next interpolation window
    unsigned int state; // 0x264: CRoundDol3D_SetAnimationByIndex sets 1; CRoundDol3D_GetState reads it
    float animationTime; // 0x268: current animation time; initialized from the range start and advanced each update
    float previousAnimationTime; // 0x26C: CRoundDol3D_UpdateAnimationAndCamera +0x160 saves current time; +0x1E4 passes it to AnimationEventParser_ProcessFrameRange
    unsigned int animationIndex; // 0x270: CRoundDol3D_SetAnimationByIndex stores the animation selection argument
    float animationEnd; // 0x274: CRoundDol3D_SetAnimationByIndex takes the animation range value at +8
    unsigned int animation; // 0x278: CRoundDol3D_SetAnimationByIndex stores the WPuppet_FindAnimationRangeByName lookup result
    char effectBox[1]; // 0x27C: CRoundDol3D_DrawModelsAndEffects / vfunc_25 pass this address; anchor only, extent unknown
    char pad_27D[0x37F];
    void* sprayAttachmentTreeRoot;                      // 0x5fc: 1 use in CRoundDol3D_dtor
    char pad_600[0x8];                  // 0x600
    void* sequenceAttachmentTreeRoot;                      // 0x608: 1 use in CRoundDol3D_dtor
    char pad_60C[0x4];                  // 0x60c
    void* sharedFile;                      // 0x610: 2 uses in CRoundDol3D_dtor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x34, 0x5f8, 0x604
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CROUNDDOL3D_OFF(f, o) typedef char CRoundDol3D_off_##f[((int)&((CRoundDol3D*)0)->f == (o)) ? 1 : -1]
CROUNDDOL3D_OFF(dolphinModel, 0x4);
CROUNDDOL3D_OFF(dolphinStandModel, 0x8);
CROUNDDOL3D_OFF(backgroundModel, 0xc);
CROUNDDOL3D_OFF(capsuleModel, 0x10);
CROUNDDOL3D_OFF(cameraMotionPlayer, 0xd0);
CROUNDDOL3D_OFF(sprayAttachmentTreeRoot, 0x5fc);
CROUNDDOL3D_OFF(sequenceAttachmentTreeRoot, 0x608);
CROUNDDOL3D_OFF(sharedFile, 0x610);
CROUNDDOL3D_OFF(activeModel, 0x14);
CROUNDDOL3D_OFF(rotationAngle, 0x18);
CROUNDDOL3D_OFF(cameraTarget, 0x9C);
CROUNDDOL3D_OFF(cameraTargetOverrideEnabled, 0xA8);
CROUNDDOL3D_OFF(rotationSpeed, 0xB0);
CROUNDDOL3D_OFF(unk_B4, 0xB4);
CROUNDDOL3D_OFF(speedLevel, 0xCC);
CROUNDDOL3D_OFF(motionSamples, 0xD4);
CROUNDDOL3D_OFF(motionSampleCount, 0x254);
CROUNDDOL3D_OFF(previousFirstMotionSample, 0x258);
CROUNDDOL3D_OFF(state, 0x264);
CROUNDDOL3D_OFF(animationTime, 0x268);
CROUNDDOL3D_OFF(animationIndex, 0x270);
CROUNDDOL3D_OFF(animationEnd, 0x274);
CROUNDDOL3D_OFF(animation, 0x278);
CROUNDDOL3D_OFF(effectBox, 0x27C);
CROUNDDOL3D_OFF(vtbl, 0x0);
CROUNDDOL3D_OFF(previousAnimationTime, 0x26C);
#undef CROUNDDOL3D_OFF

#endif  // CLASSES_CROUNDDOL3D_H
