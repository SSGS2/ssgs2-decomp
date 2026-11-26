#ifndef CLASSES_CFXSPRAY_H
#define CLASSES_CFXSPRAY_H

#include "game/types.h"
#include "game/wvector.h"

class CFxBox;

// Trail types are defined in the .cpp inside per-section namespaces; they stay opaque here.
namespace ns3 {
struct Stream;
struct Mat;
}
namespace ns4 {
struct RibbonHistory;
}

class CFxSpray {
public:
    // Allocate a spray owned by effectOwner and load its script. A null result
    // means loading failed and the unfinished spray has been deleted.
    static CFxSpray* CreateFromScript(CFxBox* effectOwner, const char* scriptName, int bypassCachedConfig,
                                      int replaceCachedConfig);

    // Load this spray's script by name, optionally copying a cached configuration
    // instead of parsing it again. Returns 0 if the script cannot be opened or parsed.
    int LoadByName(const char* name, int useCachedConfig, int replace);

    void* vtbl;                        // 0x0: original vtable pointer
    unsigned char clearOnPartialClear; // 0x4: CFxSpray_Reset (0x80111388) accesses this member
    char pad_005[0x2F];
    unsigned char viewCullingEnabled; // 0x34: CFxSpray_Reset (0x80111388) accesses this member
    char pad_035[0x43];
    void* effectOwner;          // 0x78
    unsigned int particleCount; // 0x7c: CFxSpray_Reset (0x80111388) accesses this member
    char pad_080[0x4];
    char* particleListHead;      // 0x84: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int parentSequence; // 0x88: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char drawEnabled;   // 0x8c: CFxSpray_Reset (0x80111388) accesses this member
    char pad_08D[0x3];
    float elapsedTime; // 0x90: CFxSpray_Reset (0x80111388) accesses this member
    char pad_094[0x18];
    unsigned char flashOpacity; // 0xac: CFxSpray_Reset (0x80111388) accesses this member
    char pad_0AD[0x2F];
    unsigned int positionIsWorldSpace; // 0xdc: CFxSpray_Reset (0x80111388) accesses this member
    char pad_0E0[0x90];
    unsigned char particlesFollowEmitter; // 0x170: CFxSpray_Reset (0x80111388) accesses this member
    char pad_171[0x1];
    unsigned char billboardEnabled;      // 0x172: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char attachedToParent;      // 0x173: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int parentRotationAxisMask; // 0x174: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int spriteResource;         // 0x178: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char petResourceName;       // 0x17c: CFxSpray_Reset (0x80111388) accesses this member
    char pad_17D[0x27];
    unsigned char petLightingEnabled; // 0x1a4: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char sphericalEmission;  // 0x1a5: CFxSpray_Reset (0x80111388) accesses this member
    char pad_1A6[0x1A];
    unsigned char regenerateAtSamePosition; // 0x1c0: CFxSpray_Reset (0x80111388) accesses this member
    char pad_1C1[0x3];
    float cameraOffset;       // 0x1c4: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int minLifetime; // 0x1c8: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int maxLifetime; // 0x1cc: CFxSpray_Reset (0x80111388) accesses this member
    float opacityMultiplier;  // 0x1d0: CFxSpray_Reset (0x80111388) accesses this member
    float volumeCoreScaleX;   // 0x1d4: CFxSpray_Reset (0x80111388) accesses this member
    float volumeCoreScaleY;   // 0x1d8: CFxSpray_Reset (0x80111388) accesses this member
    float friction;           // 0x1dc: CFxSpray_Reset (0x80111388) accesses this member
    float groundFriction;     // 0x1e0: CFxSpray_Reset (0x80111388) accesses this member
    char pad_1E4[0xC];
    float groundHeightOffset;             // 0x1f0: CFxSpray_Reset (0x80111388) accesses this member
    float groundRestitution;              // 0x1f4: CFxSpray_Reset (0x80111388) accesses this member
    float groundCollisionRadius;          // 0x1f8: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int spriteBlendMode;         // 0x1fc: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int trailBlendMode;          // 0x200: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int colorModulationEnabled;  // 0x204: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int lightingColor;           // 0x208: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int minInitialFrame;         // 0x20c: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int maxInitialFrame;         // 0x210: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int trailSegmentCount;       // 0x214: CFxSpray_Reset (0x80111388) accesses this member
    float trailSampleInterval;            // 0x218: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char trailEndAlpha;          // 0x21c: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char trailEndRed;            // 0x21d: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char trailEndGreen;          // 0x21e: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char trailEndBlue;           // 0x21f: CFxSpray_Reset (0x80111388) accesses this member
    void* resourceId;                     // 0x220
    float trailWidthScale;                // 0x224: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char trailMonolithEnabled;   // 0x228: CFxSpray_Reset (0x80111388) accesses this member
    unsigned char trailCameraSideEnabled; // 0x229: CFxSpray_Reset (0x80111388) accesses this member
    char pad_22A[0x1A];
    float minWindScale;                       // 0x244: CFxSpray_Reset (0x80111388) accesses this member
    float maxWindScale;                       // 0x248: CFxSpray_Reset (0x80111388) accesses this member
    WVector windVector;                       // 0x24C: set by SetWindVector
    unsigned int externalOrientationAxisMask; // 0x258: CFxSpray_Reset (0x80111388) accesses this member
    char pad_25C[0x3C];
    unsigned int angularSpeedTrackCount; // 0x298: CFxSpray_Reset (0x80111388) accesses this member
    char pad_29C[0x4];
    char* angularSpeedTrackHead;             // 0x2a0: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int velocityRotationTrackCount; // 0x2a4: CFxSpray_Reset (0x80111388) accesses this member
    char pad_2A8[0x4];
    char* velocityRotationTrackHead;      // 0x2ac: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int particleOrbitTrackCount; // 0x2b0: CFxSpray_Reset (0x80111388) accesses this member
    char pad_2B4[0x4];
    char* particleOrbitTrackHead;        // 0x2b8: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int emitterOrbitTrackCount; // 0x2bc: CFxSpray_Reset (0x80111388) accesses this member
    char pad_2C0[0x4];
    char* emitterOrbitTrackHead; // 0x2c4: CFxSpray_Reset (0x80111388) accesses this member
    char pad_2C8[0xC];
    unsigned int spawnTrackCount; // 0x2d4: CFxSpray_Reset (0x80111388) accesses this member
    char pad_2D8[0x4];
    char* spawnTrackHead;                // 0x2dc: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int gravityPointTrackCount; // 0x2e0: CFxSpray_Reset (0x80111388) accesses this member
    char pad_2E4[0x4];
    char* gravityPointTrackHead;  // 0x2e8: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int colorTrackCount; // 0x2ec: CFxSpray_Reset (0x80111388) accesses this member
    char pad_2F0[0x4];
    char* colorTrackHead;        // 0x2f4: CFxSpray_Reset (0x80111388) accesses this member
    float minInitialSize;        // 0x2f8: CFxSpray_Reset (0x80111388) accesses this member
    float maxInitialSize;        // 0x2fc: CFxSpray_Reset (0x80111388) accesses this member
    float spriteAspectRatio;     // 0x300: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int sizeTrackCount; // 0x304: CFxSpray_Reset (0x80111388) accesses this member
    char pad_308[0x4];
    char* sizeTrackHead;                // 0x30c: CFxSpray_Reset (0x80111388) accesses this member
    unsigned int spriteFrameTrackCount; // 0x310: CFxSpray_Reset (0x80111388) accesses this member
    char pad_314[0x4];
    char* spriteFrameTrackHead; // 0x318: CFxSpray_Reset (0x80111388) accesses this member
    char pad_31C[0x18];
    float minDrawDistance; // 0x334: CFxSpray_Reset (0x80111388) accesses this member
    float maxDrawDistance; // 0x338: CFxSpray_Reset (0x80111388) accesses this member
    char pad_33C[0x24];
    unsigned char circularEmission; // 0x360: CFxSpray_Reset (0x80111388) accesses this member
    char pad_361[0x7];
    float emissionAngle; // 0x368: CFxSpray_Reset (0x80111388) accesses this member

    float ParseScaledFloat(const char* value);

    void Reset();

    void SetWindVector(const void* source);

    int ParseScriptLine(const char* input);

    void ApplyEmitterOrbitTrack(float frame, float scale);
    void ApplyParentOrbitTrack(float frame, float scale);

    int EmitScheduledParticles(float emitterMilliseconds);

    void DrawParticlesAndBounds(void* renderer);

    int Update(float deltaTime);

    void DrawPositionTrail(void* renderer, ns3::Stream* stream, const void* position);

    void AppendTrailSample(ns3::Stream* particle, const ns3::Mat* worldPosition, float sampleTime);

    void DrawRibbonTrail(void* renderer, ns4::RibbonHistory* history, Arr3* currentWorldPosition);

    void SpawnChildSpray(void* source, void* entry);

    int InitializeParticle(int reusePosition, void* out, void* coreBounds, int initializing, float speedPercent,
                           float spreadAngle);

    CFxSpray(void* effectOwner);

    ~CFxSpray();
};

#define CFXSPRAY_OFF(f, o) typedef char CFxSpray_off_##f[((int)&((CFxSpray*)0)->f == (o)) ? 1 : -1]
CFXSPRAY_OFF(vtbl, 0x0);
CFXSPRAY_OFF(clearOnPartialClear, 0x4);
CFXSPRAY_OFF(viewCullingEnabled, 0x34);
CFXSPRAY_OFF(effectOwner, 0x78);
CFXSPRAY_OFF(particleCount, 0x7c);
CFXSPRAY_OFF(particleListHead, 0x84);
CFXSPRAY_OFF(parentSequence, 0x88);
CFXSPRAY_OFF(drawEnabled, 0x8c);
CFXSPRAY_OFF(elapsedTime, 0x90);
CFXSPRAY_OFF(flashOpacity, 0xac);
CFXSPRAY_OFF(positionIsWorldSpace, 0xdc);
CFXSPRAY_OFF(particlesFollowEmitter, 0x170);
CFXSPRAY_OFF(billboardEnabled, 0x172);
CFXSPRAY_OFF(attachedToParent, 0x173);
CFXSPRAY_OFF(parentRotationAxisMask, 0x174);
CFXSPRAY_OFF(spriteResource, 0x178);
CFXSPRAY_OFF(petResourceName, 0x17c);
CFXSPRAY_OFF(petLightingEnabled, 0x1a4);
CFXSPRAY_OFF(sphericalEmission, 0x1a5);
CFXSPRAY_OFF(regenerateAtSamePosition, 0x1c0);
CFXSPRAY_OFF(cameraOffset, 0x1c4);
CFXSPRAY_OFF(minLifetime, 0x1c8);
CFXSPRAY_OFF(maxLifetime, 0x1cc);
CFXSPRAY_OFF(opacityMultiplier, 0x1d0);
CFXSPRAY_OFF(volumeCoreScaleX, 0x1d4);
CFXSPRAY_OFF(volumeCoreScaleY, 0x1d8);
CFXSPRAY_OFF(friction, 0x1dc);
CFXSPRAY_OFF(groundFriction, 0x1e0);
CFXSPRAY_OFF(groundHeightOffset, 0x1f0);
CFXSPRAY_OFF(groundRestitution, 0x1f4);
CFXSPRAY_OFF(groundCollisionRadius, 0x1f8);
CFXSPRAY_OFF(spriteBlendMode, 0x1fc);
CFXSPRAY_OFF(trailBlendMode, 0x200);
CFXSPRAY_OFF(colorModulationEnabled, 0x204);
CFXSPRAY_OFF(lightingColor, 0x208);
CFXSPRAY_OFF(minInitialFrame, 0x20c);
CFXSPRAY_OFF(maxInitialFrame, 0x210);
CFXSPRAY_OFF(trailSegmentCount, 0x214);
CFXSPRAY_OFF(trailSampleInterval, 0x218);
CFXSPRAY_OFF(trailEndAlpha, 0x21c);
CFXSPRAY_OFF(trailEndRed, 0x21d);
CFXSPRAY_OFF(trailEndGreen, 0x21e);
CFXSPRAY_OFF(trailEndBlue, 0x21f);
CFXSPRAY_OFF(resourceId, 0x220);
CFXSPRAY_OFF(trailWidthScale, 0x224);
CFXSPRAY_OFF(trailMonolithEnabled, 0x228);
CFXSPRAY_OFF(trailCameraSideEnabled, 0x229);
CFXSPRAY_OFF(minWindScale, 0x244);
CFXSPRAY_OFF(maxWindScale, 0x248);
CFXSPRAY_OFF(windVector, 0x24C);
CFXSPRAY_OFF(externalOrientationAxisMask, 0x258);
CFXSPRAY_OFF(angularSpeedTrackCount, 0x298);
CFXSPRAY_OFF(angularSpeedTrackHead, 0x2a0);
CFXSPRAY_OFF(velocityRotationTrackCount, 0x2a4);
CFXSPRAY_OFF(velocityRotationTrackHead, 0x2ac);
CFXSPRAY_OFF(particleOrbitTrackCount, 0x2b0);
CFXSPRAY_OFF(particleOrbitTrackHead, 0x2b8);
CFXSPRAY_OFF(emitterOrbitTrackCount, 0x2bc);
CFXSPRAY_OFF(emitterOrbitTrackHead, 0x2c4);
CFXSPRAY_OFF(spawnTrackCount, 0x2d4);
CFXSPRAY_OFF(spawnTrackHead, 0x2dc);
CFXSPRAY_OFF(gravityPointTrackCount, 0x2e0);
CFXSPRAY_OFF(gravityPointTrackHead, 0x2e8);
CFXSPRAY_OFF(colorTrackCount, 0x2ec);
CFXSPRAY_OFF(colorTrackHead, 0x2f4);
CFXSPRAY_OFF(minInitialSize, 0x2f8);
CFXSPRAY_OFF(maxInitialSize, 0x2fc);
CFXSPRAY_OFF(spriteAspectRatio, 0x300);
CFXSPRAY_OFF(sizeTrackCount, 0x304);
CFXSPRAY_OFF(sizeTrackHead, 0x30c);
CFXSPRAY_OFF(spriteFrameTrackCount, 0x310);
CFXSPRAY_OFF(spriteFrameTrackHead, 0x318);
CFXSPRAY_OFF(minDrawDistance, 0x334);
CFXSPRAY_OFF(maxDrawDistance, 0x338);
CFXSPRAY_OFF(circularEmission, 0x360);
CFXSPRAY_OFF(emissionAngle, 0x368);
#undef CFXSPRAY_OFF

#endif // CLASSES_CFXSPRAY_H
