// Context8012F978: partial layout for methods in the address-named translation unit.
// Field comments identify the function providing layout evidence.
#ifndef CLASSES_CONTEXT8012F978_H
#define CLASSES_CONTEXT8012F978_H

class Context8012F978 {
public:
    char pad_000[0x1C];
    void* animationHolder; // 0x1C: CGolfCaddie_SetLookAtTarget
    void** modelHolders; // 0x20: CGolfCaddie_ActivatePlayerModel
    unsigned int modelHolderCount; // 0x24: CGolfCaddie_ActivatePlayerModel
    char pad_028[0x8];
    int animationState; // 0x30: CGolfCaddie_UpdateMovement
    int effectControlId; // 0x34: CGolfCaddie_UpdateMovement
    float position[3]; // 0x38: CGolfCaddie_UpdateMovement copies holder position, CGolfCaddie_PlayShotReaction subtracts ball position
    char pad_044[0x8];
    float targetPosition[3]; // 0x4c: CGolfCaddie_UpdateMovement computes targetPosition minus position
    unsigned char animationActive; // 0x58: CGolfCaddie_PlayShotReaction
    char pad_059[0x17];
    void* fxInterpreterKey; // 0x70: CGolfCaddie_ActivatePlayerModel
    char pad_074[0x10C];
    void* playerCaddieBindings; // 0x180: CGolfCaddie_ActivatePlayerModel
    int storyModelMode; // 0x184: CGolfCaddie_PollPendingModelLoads
    int pendingTextCount; // 0x188: CGolfCaddie_PollPendingModelLoads
    void* pendingTextTreeRoot; // 0x18C: CGolfCaddie_PollPendingModelLoads
    char pad_190[0x4];
    void* pendingTextIterator; // 0x194: CGolfCaddie_PollPendingModelLoads
    void* pendingOperation; // 0x198: CGolfCaddie_UpdatePendingOperation
    int expectedPlayer; // 0x19C: CGolfCaddie_UpdatePendingOperation
    int operationActive; // 0x1A0: CGolfCaddie_UpdatePendingOperation
};

#define OFF(f, o) typedef char Context8012F978_off_##f[((int)&((Context8012F978*)0)->f == (o)) ? 1 : -1]
OFF(animationHolder, 0x1C);
OFF(modelHolders, 0x20);
OFF(modelHolderCount, 0x24);
OFF(animationState, 0x30);
OFF(effectControlId, 0x34);
OFF(position, 0x38);
OFF(targetPosition, 0x4C);
OFF(animationActive, 0x58);
OFF(fxInterpreterKey, 0x70);
OFF(playerCaddieBindings, 0x180);
OFF(storyModelMode, 0x184);
OFF(pendingTextCount, 0x188);
OFF(pendingTextTreeRoot, 0x18C);
OFF(pendingTextIterator, 0x194);
OFF(pendingOperation, 0x198);
OFF(expectedPlayer, 0x19C);
OFF(operationActive, 0x1A0);
#undef OFF

#endif
