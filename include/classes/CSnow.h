// CSnow: the snow weather effect.
//
// It holds one animated sprite texture, the UV bounds of its first frame, the
// shared allocation of snow particles, and its retained graphics buffers.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CSNOW_H
#define CLASSES_CSNOW_H

#include "game/types.h"

class CSnow {
public:
    // Release the sprite, particle storage and graphics buffers.
    ~CSnow();

    // Create the sprite and initialize the UV bounds and render context. This is
    // the original constructor's body kept as a named method; the compiler's
    // own constructor code changes the store order.
    CSnow* Init(int count, const char* resourceName, int context, float scale);

    void RebuildParticleBuffers(int requestedCount);
    void Draw();
    void UpdateParticles(float deltaTime);

    struct GeneratedGeometry {
        unsigned char vertexCount; // 0x58
        char pad_59[3];
        void* vtbl;     // 0x5c
        void* vertices; // 0x60
    };

    void* vtbl;                 // 0x00
    float fallSpeedMultiplier;  // 0x04
    char pad_008[0x18];         // 0x08
    void* particles;            // 0x20
    void* animatedSprite;       // 0x24
    void* spriteTexture;        // 0x28
    float uMin;                 // 0x2c
    float vMin;                 // 0x30
    float uMax;                 // 0x34
    float vMax;                 // 0x38
    float vSpan;                // 0x3c
    float uSpan;                // 0x40
    int particleCount;          // 0x44
    void* effectContext;        // 0x48
    void* vertexBuffer;         // 0x4c
    void* indexBuffer;          // 0x50
    void* auxiliaryBuffer;      // 0x54
    GeneratedGeometry geometry; // 0x58
};

#define CSNOW_OFF(f, o) typedef char CSnow_off_##f[((int)&((CSnow*)0)->f == (o)) ? 1 : -1]
CSNOW_OFF(fallSpeedMultiplier, 0x4);
CSNOW_OFF(particles, 0x20);
CSNOW_OFF(animatedSprite, 0x24);
CSNOW_OFF(spriteTexture, 0x28);
CSNOW_OFF(uMin, 0x2c);
CSNOW_OFF(uSpan, 0x40);
CSNOW_OFF(particleCount, 0x44);
CSNOW_OFF(effectContext, 0x48);
CSNOW_OFF(vertexBuffer, 0x4c);
CSNOW_OFF(geometry, 0x58);
#undef CSNOW_OFF

typedef char CSnow_size[(sizeof(CSnow) == 0x64) ? 1 : -1];
typedef char CSnow_geometry_size[(sizeof(CSnow::GeneratedGeometry) == 0xc) ? 1 : -1];

#endif // CLASSES_CSNOW_H
