#ifndef CLASSES_CBALL_H
#define CLASSES_CBALL_H

#include "game/types.h"

struct BallMessage;

// The visible golf ball actor. It owns a cache of loaded ball puppets (one per
// equipped ball ID), draws the puppet and its ground shadow, and keeps the
// visual transform in step with the simulated ball.
class CBall {
public:
    CBall();

    // Actor hooks.
    void OnPreLoadInit();
    void OnLoad();
    void OnReload();
    void OnInit();
    void OnProcess();
    void OnDisplay();
    void Display();
    void OnDestroy();
    void* GetRTTI();

    // Factory and type registration, called through the actor registry.
    static void* Create();
    static void RegisterType();
    void HandleMsg(void* msg);

    // Effects and presentation.
    void AttachFx(int kind);
    void DetachFx(int kind);
    void TransformBall();
    void DrawGroundShadowAdjustedThis();

    // Named Destroy rather than ~CBall: the real destructor does not hold the code.
    void* Destroy(int del);

    void* vtbl;                       // 0x0: original vtable pointer
    char pad_004[0x4];                // 0x4
    int unk_8;                        // 0x8: constructor initializes to 1
    char pad_00C[0xC];                // 0xc
    void* secondaryVtable;            // 0x18: vtable of the secondary interface
    int puppetEntryCount;             // 0x1c
    void* puppetListSentinel;         // 0x20: previous link of the cache sentinel
    void* firstPuppetEntry;           // 0x24: next link of the cache sentinel
    void* puppet;                     // 0x28
    void* bitmap;                     // 0x2c: shadow texture
    float matrixBasis[9];             // 0x30: affine matrix of the ball model
    float position[3];                // 0x54: world position
    float rotationAxis[3];            // 0x60: rolling axis
    unsigned char lightUpdatePending; // 0x6c
    char pad_06D[0x3];                // 0x6d
    float groundSinkFactor;           // 0x70
    int sinkStartFrame;               // 0x74
    int lightingMode;                 // 0x78
    float lightDirection[3];          // 0x7c
    void* lightColor;                 // 0x88
    void* ambientLightColor;          // 0x8c
    unsigned int unk_90;              // 0x90: constructor initializes to 0x00FFFFFF
    unsigned char visible;            // 0x94
    char pad_095[0x3];                // 0x95
    int previousPlaybackFrame;        // 0x98
    void* vertexStreams[4];           // 0x9c
    void* unk_AC;                     // 0xac
    char vertexData[0xE0];            // 0xb0: four 0x38-byte vertices
    int effectMessageValue;           // 0x190
};

#define CBALL_OFF(f, o) typedef char CBall_off_##f[((int)&((CBall*)0)->f == (o)) ? 1 : -1]
CBALL_OFF(puppetEntryCount, 0x1c);
CBALL_OFF(firstPuppetEntry, 0x24);
CBALL_OFF(puppet, 0x28);
CBALL_OFF(lightUpdatePending, 0x6c);
CBALL_OFF(groundSinkFactor, 0x70);
CBALL_OFF(sinkStartFrame, 0x74);
CBALL_OFF(lightColor, 0x88);
CBALL_OFF(ambientLightColor, 0x8c);
CBALL_OFF(visible, 0x94);
CBALL_OFF(previousPlaybackFrame, 0x98);
CBALL_OFF(effectMessageValue, 0x190);
CBALL_OFF(unk_8, 0x8);
CBALL_OFF(secondaryVtable, 0x18);
CBALL_OFF(puppetListSentinel, 0x20);
CBALL_OFF(bitmap, 0x2c);
CBALL_OFF(matrixBasis, 0x30);
CBALL_OFF(position, 0x54);
CBALL_OFF(rotationAxis, 0x60);
CBALL_OFF(lightingMode, 0x78);
CBALL_OFF(lightDirection, 0x7c);
CBALL_OFF(unk_90, 0x90);
CBALL_OFF(vertexStreams, 0x9c);
CBALL_OFF(unk_AC, 0xac);
CBALL_OFF(vertexData, 0xb0);
#undef CBALL_OFF

#endif // CLASSES_CBALL_H
