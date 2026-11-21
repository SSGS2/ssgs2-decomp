// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// The original vtable (lbl_804AE270) and typeinfo stay data symbols.
#ifndef CLASSES_CGOLFPLAYER_H
#define CLASSES_CGOLFPLAYER_H

#include "game/types.h"

// RTTI base class: CPuppetScript (not declared here)
class CGolfPlayer {
public:
    void* vtbl;  // offset 0: original vtable pointer (lbl_804AE270)

    // Field layout from the constructor CGolfPlayer_Ctor (golf_player_lifecycle.cpp; zero-inits and sets the fields below, allocation
    // size 0x3E8) and from the offsets used by CGolfPlayer_OnPreLoadInit..8 (golf_player_actor.cpp, golf_player_actor.cpp, golf_player_lifecycle.cpp).
    // Names only where behaviour justifies them; everything else is unk_<offset>.
    char pad_004[0x14];          // 0x04: base-class data (NativeBase80201D18 ctor)
    void* sub_vtbl;              // 0x18: secondary vtable (ctor stores __vt__11CGolfPlayer + 0x3c)
    int attachedSprayIds, attachedSprayCount, attachedSprayCapacity, attachedSequenceIds, attachedSequenceCount, attachedSequenceCapacity;  // 0x1C: zeroed by ctor
    void* clubPuppet;                // 0x34: polymorphic object, vtable slot 5 called (golf_player_actor.cpp UpdateObjectTransforms)
    char pad_038[4];             // 0x38
    void* playerModels;                // 0x3C: array of 0x2A0-byte objects (vfunc_6, UpdateObjectTransforms)
    void* animationStates;       // 0x40: array of 0xB4-byte animation entries (vfunc_6/7)
    float swingReach; // 0x44: CGolfPlayer_HandleMsg
    void* trailStartBoneEntry;                // 0x48: (vfunc_6 channel setup)
    void* trailEndBoneEntry;                // 0x4C: (vfunc_6 channel setup)
    void* tailEffects[4];        // 0x50: per-channel resources (vfunc_6, indexed by trailEnabled)
    unsigned char trailEnabled[4];   // 0x60: per-channel enable bytes (vfunc_4 clears, vfunc_6 tests)
    char pad_064[1];             // 0x64
    char shotAnimationPrefix[0x40];          // 0x65: C string passed to the formatter in vfunc_6 (up to 0xA5)
    unsigned char swingAnimationName;        // 0xA5: cleared by vfunc_4
    char pad_0A6[0x1C4];         // 0xA6
    unsigned char translucencyEnabled;       // 0x26A: toggled in vfunc_6
    char pad_26B[1];             // 0x26B
    int lightType;                 // 0x26C: ctor 0; vfunc_2 sets 2
    Vec3f lightDirection;               // 0x270: 12-byte copy target (vfunc_4); ctor zeroes the three floats
    int lightColor;                 // 0x27C: ctor 0
    void* ambientLightColor;               // 0x280: ctor 0; vfunc_4 stores a lookup result
    int unk_284;                 // 0x284: ctor 0x00FFFFFF
    Vec3f attachmentDirection;               // 0x288: vector passed by address (UpdateObjectTransforms)
    char pad_294[0x18];          // 0x294
    Vec3f attachmentPosition; // 0x2AC: CGolfPlayer_LockClubAttachment transforms attachment point and adds scaled direction
    unsigned char attachmentPositionLocked;       // 0x2B8: ctor 0
    char pad_2B9[3];             // 0x2B9
    float distanceToHole;               // 0x2BC: compared with a constant in vfunc_6
    int cleanupListCount;                 // 0x2C0: ctor 0
    void* cleanupListPrev;               // 0x2C4: ctor: points to self + 0x2C4
    void* cleanupListNext;               // 0x2C8: ctor: points to self + 0x2C4
    unsigned char animationCompletionNotifyPending;       // 0x2CC: cleared in vfunc_6
    unsigned char unk_2CD;       // 0x2CD: ctor 0
    char pad_2CE[2];             // 0x2CE
    float unk_2D0;               // 0x2D0: ctor 0.0
    int requestedPowerShotMode;                 // 0x2D4: event result (vfunc_6)
    int unk_2D8, unk_2DC;        // 0x2D8: ctor 0
    unsigned char singlePowerShotReady, doublePowerShotDisabled;  // 0x2E0: ctor 0
    char pad_2E2[2];             // 0x2E2
    int unk_2E4;                 // 0x2E4: ctor 0
    char slots_2E8[0xC0];        // 0x2E8: in-place array (ctor)
    void* playerStates;          // 0x3A8: array of 0x64-byte state records, allocated in vfunc_4 (100 entries)
    int readyForQueuedAnimation;                 // 0x3AC: ctor 1
    unsigned char playerIndexOffset; // 0x3B0: entry index offset (vfunc_7)
    char pad_3B1[3];             // 0x3B1
    unsigned int animationDisabled; // 0x3B4: vfunc_4 clears (CGolfPlayer_SetObjectFlag stores here)
    int processingSuspended, avatarPreloadPending;        // 0x3B8: ctor 0
    int preloadPlayerIndex;      // 0x3C0: ctor -1
    void* avatarPreloader;       // 0x3C4: polymorphic object (vfunc_6 update/done)
    unsigned int avatarPreloadKind; // 0x3c8: CGolfPlayer_HandleMsg
    void* loadedSoundNames;               // 0x3CC: vector begin (size 0x3D0, capacity 0x3D4; vfunc_3)
    unsigned int loadedSoundNameCount, loadedSoundNameCapacity;  // 0x3D0
    void* pendingModelLoadCount;               // 0x3D8: ctor 0
    void* pendingModelLoadRoot;               // 0x3DC: ctor 0 (list head)
    char pad_3E0[4];             // 0x3E0
    void* firstPendingModelLoad;               // 0x3E4: ctor: points to self + 0x3DC

    // vtable slots (current symbol; implement as a method with its real signature):
    // slot 0: fn_8014B118 (medium)  void vfunc_0();
    // slot 1: fn_8014AAC8 (medium)  void vfunc_1();
    // slot 2: fn_8013B24C (medium)  void vfunc_2();
    // slot 3: fn_8013B73C (medium)  void vfunc_3();
    // slot 4: fn_8013C5CC (medium)  void vfunc_4();
    // slot 5: fn_8013C288 (medium)  void vfunc_5();
    // slot 6: fn_8013C708 (medium)  void vfunc_6();
    // slot 7: fn_8013D3A8 (medium)  void vfunc_7();
    // slot 8: fn_8013AC04 (medium)  void vfunc_8();
    // slot 9: fn_8013EFB0 (medium)  void vfunc_9();
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGOLFPLAYER_OFF(f, o) typedef char CGolfPlayer_off_##f[((int)&((CGolfPlayer*)0)->f == (o)) ? 1 : -1]
typedef char CGolfPlayer_size_check[(sizeof(CGolfPlayer) == 0x3E8) ? 1 : -1];
CGOLFPLAYER_OFF(sub_vtbl, 0x18);
CGOLFPLAYER_OFF(playerModels, 0x3C);
CGOLFPLAYER_OFF(animationStates, 0x40);
CGOLFPLAYER_OFF(tailEffects, 0x50);
CGOLFPLAYER_OFF(trailEnabled, 0x60);
CGOLFPLAYER_OFF(shotAnimationPrefix, 0x65);
CGOLFPLAYER_OFF(lightType, 0x26C);
CGOLFPLAYER_OFF(lightDirection, 0x270);
CGOLFPLAYER_OFF(attachmentDirection, 0x288);
CGOLFPLAYER_OFF(distanceToHole, 0x2BC);
CGOLFPLAYER_OFF(requestedPowerShotMode, 0x2D4);
CGOLFPLAYER_OFF(slots_2E8, 0x2E8);
CGOLFPLAYER_OFF(playerStates, 0x3A8);
CGOLFPLAYER_OFF(animationDisabled, 0x3B4);
CGOLFPLAYER_OFF(avatarPreloadPending, 0x3BC);
CGOLFPLAYER_OFF(avatarPreloader, 0x3C4);
CGOLFPLAYER_OFF(loadedSoundNames, 0x3CC);
CGOLFPLAYER_OFF(pendingModelLoadCount, 0x3D8);
CGOLFPLAYER_OFF(pendingModelLoadRoot, 0x3DC);
CGOLFPLAYER_OFF(firstPendingModelLoad, 0x3E4);
CGOLFPLAYER_OFF(swingReach, 0x44);
CGOLFPLAYER_OFF(avatarPreloadKind, 0x3c8);
CGOLFPLAYER_OFF(attachmentPosition, 0x2AC);
#undef CGOLFPLAYER_OFF

#endif  // CLASSES_CGOLFPLAYER_H
