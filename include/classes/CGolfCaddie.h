// CGolfCaddie: the caddie that walks the course with a player.
//
// It keeps one entry per player, loads caddie and pet models asynchronously,
// binds the loaded puppet to the scene and its shadow, reacts to the shot in
// flight, and drives the caddie's walk, look-at and commentary. The class is
// declared without virtual functions on purpose: its vtable is still taken
// from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CGOLFCADDIE_H
#define CLASSES_CGOLFCADDIE_H

#include "game/types.h"

struct CaddieMessage;

class CGolfCaddie {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* secondaryVtable;              // 0x18
    void* activeCaddie;                 // 0x1c: the caddie entry currently in use
    void** caddieEntries;               // 0x20: caddie entry slots, by player index
    unsigned int caddieEntryCount;      // 0x24
    int caddieEntryCapacity;            // 0x28
    char pad_02C[0x4];                  // 0x2c
    int animationState;                 // 0x30
    int runDustEffect;                  // 0x34: control key of the running caddie's dust
    float position[3];                  // 0x38: world position
    float movementDelay;                // 0x44
    float yaw;                          // 0x48
    float movementTarget[3];            // 0x4c: where the caddie is walking to
    unsigned char movementBlocked;      // 0x58
    unsigned char caddieVisible;        // 0x59
    char pad_05A[0x2];                  // 0x5a
    float idleAnimationTimer;           // 0x5c
    int reactionValue;                  // 0x60: absolute value above one picks a reaction comment
    bool reactionModeOneEnabled;        // 0x64
    char pad_065[0x3];                  // 0x65
    float aimYaw;                       // 0x68: aim direction about the Y axis
    float windCommentTimer;             // 0x6c: seconds spent in the camera's wind region
    int effectEntryKey;                 // 0x70
    float cachedPlacementPosition[3];   // 0x74: saved preview placement
    float cachedPlacementYaw;           // 0x80: saved preview yaw in radians
    void* texture3D;                    // 0x84
    float affineMatrices[4][12];        // 0x88: four 3x4 affine matrices
    unsigned char currentAction;        // 0x148: last comment action latched, 0 when free
    unsigned char actionFlags[36];      // 0x149: one busy flag per comment action
    unsigned char aimCommentIssued;     // 0x16d: one aim comment per aim
    char pad_16E[0x2];                  // 0x16e
    unsigned int commentaryTimestampMs; // 0x170: commentary clock in milliseconds
    void* soundBankNames;               // 0x174
    void* soundBankNameCount;           // 0x178
    void* soundBankNameCapacity;        // 0x17c
    void* conteCaddies;                 // 0x180
    void* storyConteCaddieOverride;     // 0x184
    void* pendingModelLoadCount;        // 0x188
    void* pendingModelLoadTreeRoot;     // 0x18c
    char pad_190[0x4];                  // 0x190
    void* pendingModelLoadIterator;     // 0x194
    void* petPreload;                   // 0x198
    int petPreloadPlayerIndex;          // 0x19c
    int petPreloadActive;               // 0x1a0: an asynchronous preload is running

    void SetLookAtTarget(void* target, unsigned char enabled);
    void PlayShotReaction();
    void UpdateMovement();
    void ActivatePlayerModel(int playerIndex, float yawRadians, void* positionOrBone);

    int PollPendingModelLoads();
    void RebuildModelLoadQueue();
    int UpdatePendingOperation();

    void UpdateBoneRelativeRenderParams(void* puppet);

    int ParseAnimationMetadata(void* caddieEntry);
    int CanPlayAnimation(void* caddieEntry, const char* animationName);
    void ApplyPendingPetState(float delta);
    void OnProcess(float delta);
    void UpdateSituationalCommentary(float delta);
    void TriggerCommentary(int actionIndex, int force, unsigned char variantNumber);
    void HandleMsg(const CaddieMessage* message);

    void Draw();

    void OnInit();
    void OnReload();
    void* FindExistingEntry(unsigned int key, unsigned int value);
    void* CreateCaddieEntry(unsigned int caddieTypeId, unsigned int instanceId, int reuseExisting, int createPetBody);
    void CreatePetBodyForEntry(void* entry, void* lookup);

    static void* GetRTTI();
    static void vfunc_1Thunk18(void* p);

    static void OnPreLoadInit(void* ptr);
    void OnDestroy();
    void OnLoad();

    CGolfCaddie();
    ~CGolfCaddie();
};

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
CGOLFCADDIE_OFF(movementBlocked, 0x58);
CGOLFCADDIE_OFF(caddieVisible, 0x59);
CGOLFCADDIE_OFF(idleAnimationTimer, 0x5c);
CGOLFCADDIE_OFF(reactionValue, 0x60);
CGOLFCADDIE_OFF(reactionModeOneEnabled, 0x64);
CGOLFCADDIE_OFF(aimYaw, 0x68);
CGOLFCADDIE_OFF(windCommentTimer, 0x6c);
CGOLFCADDIE_OFF(effectEntryKey, 0x70);
CGOLFCADDIE_OFF(cachedPlacementPosition, 0x74);
CGOLFCADDIE_OFF(cachedPlacementYaw, 0x80);
CGOLFCADDIE_OFF(texture3D, 0x84);
CGOLFCADDIE_OFF(affineMatrices, 0x88);
CGOLFCADDIE_OFF(currentAction, 0x148);
CGOLFCADDIE_OFF(actionFlags, 0x149);
CGOLFCADDIE_OFF(aimCommentIssued, 0x16d);
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

#endif // CLASSES_CGOLFCADDIE_H
