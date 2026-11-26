// CPetBody: a character's animated body.
//
// The class owns a puppet (the model and its animation state), the body's world
// transform, a small flight-movement controller, a smooth head look-at, lighting
// parameters and the script callbacks the body answers. Pets, caddies and the
// scripted actors of the game all use it.
//
// It is declared without virtual functions on purpose: its vtable is still taken
// from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CPETBODY_H
#define CLASSES_CPETBODY_H

#include "game/types.h"
#include "game/wvector.h"

// A named animation range inside a puppet's animation table. Only the fields the
// body reads are named; the name and repeat name are borrowed pointers.
struct PetAnimationRange {
    float startTime;   // 0x00
    float endTime;     // 0x04
    float maximumTime; // 0x08
    char pad_00C[0xC];
    void* name;       // 0x18
    void* repeatName; // 0x1C
};

// A node of the callback ring that script commands are dispatched through.
struct PetCallbackRingNode;

// An attachment registered by script: the animation it belongs to, its name, the
// model resource it created and its current state.
struct PetAttachment {
    char animation[0x40]; // 0x00
    char name[0x40];      // 0x40
    void* resource;       // 0x80
    char state[0x10];     // 0x84
};

class CPetBody {
public:
    // Playback and pose.
    void ResetPlaybackAndLookAt();
    void SetNameString(const char* text);
    void SetPosition(const void* sourcePosition);
    void SetRotation(const void* sourceEulerRadians);
    void SetYaw(float value);
    void ApplyRotationMatrix(void* rotationMatrix);
    void SetUniformScale(float value);

    // Animation.
    void UpdateAnimation(float dt);
    float ComputeAnimationStep(float dt);
    void PlayAnimationByName(float startOffset, void* animationName, int useRootMotion, int preserveRootMotion);
    char* SetNextAnimation(const char* source, int useRootMotion);
    void ApplyAnimationToPuppet();
    const char* GetCurrentAnimationName();
    char* GetNextAnimationName();
    void* GetAnimationFinished();

    // Flight movement.
    void StartMovement(void* destination);
    void UpdateMovement(float delta);
    void EnableFlightMovement();
    float* GetMovementDirectionPtr();

    // Base transform. The third basis vector is the body's forward direction.
    WVector GetNegatedBaseForward();

    // Head look-at.
    void UpdateLookAt(float delta);
    void SetLookAtTarget(void* target, int turnBody);

    // Lighting.
    void SetupLighting();

    // Script commands.
    void DispatchCommandCallbacks(char** commandText);
    void ParseAndRegisterAttachment(char** inout);
    void ParseMaterialLayerAnimationCommand(void* commandText);
    void ParseBoneReferences(void* commandText, void* effectContext);
    void PlaySoundCommand(void* commandText);
    void PlaySound(const char* text);

    // Model loading and access.
    int RequestModelLoad(void* modelName);
    int PollModelLoad();
    void* GetPuppet();
    void* GetPuppetBoundsMin();
    void SetOpacity(unsigned char opacity);

    // Per-frame update and drawing.
    void Update(float dt);
    void Draw(void* useViewDraw, void* drawEnabled);

    // Hiding.
    void Hide();
    void Show();

    // Layout. The callback-ring node type is defined with the ring helpers in the
    // file that allocates it.
    void* vtbl;                                      // 0x00
    void* puppet;                                    // 0x04  the loaded WPuppet, or null
    int shadowMode;                                  // 0x08
    PetAnimationRange* currentAnimation;             // 0x0C
    float animationTime;                             // 0x10
    float baseMatrixBasis[9];                        // 0x14  body frame without animation offsets
    unsigned int baseMatrixPositionX;                // 0x38
    unsigned int baseMatrixPositionY;                // 0x3C
    unsigned int baseMatrixPositionZ;                // 0x40
    float matrixBasis[9];                            // 0x44  frame handed to the puppet
    Arr3 matrixPosition;                             // 0x68
    float rotationX;                                 // 0x74  Euler angles in radians
    float rotationY;                                 // 0x78
    float rotationZ;                                 // 0x7C
    float positionX;                                 // 0x80
    float positionY;                                 // 0x84
    float positionZ;                                 // 0x88
    float movementDirection[3];                      // 0x8C  current displacement step
    float uniformScale;                              // 0x98
    int movementState;                               // 0x9C
    unsigned char flightMovementEnabled;             // 0xA0
    char pad_0A1[3];                                 // 0xA1
    float movementTarget[3];                         // 0xA4
    float movementStart[3];                          // 0xB0
    float minimumHeight;                             // 0xBC
    float verticalVelocity;                          // 0xC0
    float movementPitchRadians;                      // 0xC4
    unsigned int flags;                              // 0xC8
    PetCallbackRingNode* instanceHead;               // 0xCC  command callback ring
    PetCallbackRingNode* instanceIterator;           // 0xD0
    PetCallbackRingNode* instanceNodeAllocationHead; // 0xD4
    PetCallbackRingNode* instanceFreeNodeHead;       // 0xD8  recycled node ring
    int unk_DC;                                      // 0xDC
    int instanceHashMask;                            // 0xE0
    int instanceHashBucketCount;                     // 0xE4
    void** instanceHashBuckets;                      // 0xE8
    char instanceMutex[0x18];                        // 0xEC
    void* renderParameters;                          // 0x104  array of render parameter slots
    int renderParameterCount;                        // 0x108
    int unk_10C;                                     // 0x10C
    unsigned char renderParametersEnabled;           // 0x110
    unsigned char defaultRenderParametersEnabled;    // 0x111
    char pad_112[2];                                 // 0x112
    int lightingMode;                                // 0x114
    float lightDirectionX;                           // 0x118
    float lightDirectionY;                           // 0x11C
    float lightDirectionZ;                           // 0x120
    int lightColor;                                  // 0x124
    int ambientLightColor;                           // 0x128
    int unk_12C;                                     // 0x12C
    int nameStringFlags;                             // 0x130  short string or heap pointer
    int nameStringLength;                            // 0x134
    int nameStringData;                              // 0x138
    float rootMotionDeltaX;                          // 0x13C  sampled root displacement
    float rootMotionDeltaY;                          // 0x140
    float rootMotionDeltaZ;                          // 0x144
    char pad_148[0xC];                               // 0x148
    float animationSpeed;                            // 0x154
    float rootMotionScale;                           // 0x158
    unsigned char queuedAnimationName[32];           // 0x15C
    unsigned char queuedAnimationUsesRootMotion;     // 0x17C
    char pad_17D[3];                                 // 0x17D
    unsigned int attachmentCount;                    // 0x180
    void* attachmentTail;                            // 0x184  sentinel of the attachment list
    void* attachmentHead;                            // 0x188
    void* followTarget;                              // 0x18C
    unsigned char turnToTarget;                      // 0x190
    char pad_191[3];                                 // 0x191
    float lookAtPitch;                               // 0x194
    float lookAtYaw;                                 // 0x198
    float targetPitch;                               // 0x19C
    float targetYaw;                                 // 0x1A0
    int animationFinished;                           // 0x1A4
    char unk_1A8;                                    // 0x1A8
    char pad_1A9[3];                                 // 0x1A9
    int resourcePending;                             // 0x1AC
    int unk_1B0;                                     // 0x1B0

    static void InitDefaults(void* ptr);

    CPetBody(const char* modelCommand, int mode);
    ~CPetBody();
};

#define CPETBODY_OFF(f, o) typedef char CPetBody_off_##f[((int)&((CPetBody*)0)->f == (o)) ? 1 : -1]
CPETBODY_OFF(vtbl, 0x0);
CPETBODY_OFF(puppet, 0x4);
CPETBODY_OFF(currentAnimation, 0xc);
CPETBODY_OFF(animationTime, 0x10);
CPETBODY_OFF(baseMatrixBasis, 0x14);
CPETBODY_OFF(baseMatrixPositionX, 0x38);
CPETBODY_OFF(matrixBasis, 0x44);
CPETBODY_OFF(matrixPosition, 0x68);
CPETBODY_OFF(rotationX, 0x74);
CPETBODY_OFF(positionX, 0x80);
CPETBODY_OFF(movementDirection, 0x8c);
CPETBODY_OFF(uniformScale, 0x98);
CPETBODY_OFF(movementState, 0x9c);
CPETBODY_OFF(flightMovementEnabled, 0xa0);
CPETBODY_OFF(movementTarget, 0xa4);
CPETBODY_OFF(movementStart, 0xb0);
CPETBODY_OFF(minimumHeight, 0xbc);
CPETBODY_OFF(verticalVelocity, 0xc0);
CPETBODY_OFF(movementPitchRadians, 0xc4);
CPETBODY_OFF(flags, 0xc8);
CPETBODY_OFF(instanceHead, 0xcc);
CPETBODY_OFF(instanceMutex, 0xec);
CPETBODY_OFF(renderParameters, 0x104);
CPETBODY_OFF(renderParameterCount, 0x108);
CPETBODY_OFF(renderParametersEnabled, 0x110);
CPETBODY_OFF(lightingMode, 0x114);
CPETBODY_OFF(lightDirectionX, 0x118);
CPETBODY_OFF(lightColor, 0x124);
CPETBODY_OFF(ambientLightColor, 0x128);
CPETBODY_OFF(nameStringFlags, 0x130);
CPETBODY_OFF(nameStringData, 0x138);
CPETBODY_OFF(rootMotionDeltaX, 0x13c);
CPETBODY_OFF(animationSpeed, 0x154);
CPETBODY_OFF(rootMotionScale, 0x158);
CPETBODY_OFF(queuedAnimationName, 0x15c);
CPETBODY_OFF(queuedAnimationUsesRootMotion, 0x17c);
CPETBODY_OFF(attachmentCount, 0x180);
CPETBODY_OFF(attachmentTail, 0x184);
CPETBODY_OFF(attachmentHead, 0x188);
CPETBODY_OFF(followTarget, 0x18c);
CPETBODY_OFF(turnToTarget, 0x190);
CPETBODY_OFF(lookAtPitch, 0x194);
CPETBODY_OFF(targetPitch, 0x19c);
CPETBODY_OFF(animationFinished, 0x1a4);
CPETBODY_OFF(resourcePending, 0x1ac);
#undef CPETBODY_OFF

typedef char PetAnimationRange_size[(sizeof(PetAnimationRange) == 0x20) ? 1 : -1];
typedef char PetAttachment_size[(sizeof(PetAttachment) == 0x94) ? 1 : -1];

#endif // CLASSES_CPETBODY_H
