#ifndef CLASSES_CCOIN_H
#define CLASSES_CCOIN_H

#include "game/types.h"
#include "game/wvector.h"
#include "classes/CParticleBase.h"

class CCoin : public CParticleBase {
public:
    // Build the effect at a world position with room for count coins and the fixed spark set.
    CCoin(int count, const WVector& position);
    // Release the coin models, the spark records, the shared sprite and both arrays.
    // A delete flag above zero also frees the object.
    void* Destroy(int deleting);
    // Show or hide the coins; shows also (re)attach their shadows.
    void SetEnabled(unsigned char enable);
    // Draw enabled coin models and their live spark sprites.
    void DrawCoinsAndSprites();
    // Recreate the spark grid, reset the fade and sound state, and launch and enable the effect.
    void RestartCoinEffect();
    // Create missing coin models, then launch the coins and spark sprites from the effect position.
    void InitializeLaunchAndSparks();
    // Integrate and bounce the coins, fade the sparks, and stop them on expiry.
    void UpdateCoinsAndSprites(float delta);

    void* vtbl;                 // 0x0: original vtable pointer
    char pad_004[0x10];         // 0x4
    int bitmapId;               // 0x14
    unsigned char enabled;      // 0x18
    char pad_019[0x3];          // 0x19
    void* sprite;               // 0x1c
    void* sparkSprite;          // 0x20
    char pad_024[0xc];          // 0x24
    void* vertex0;              // 0x30
    void* vertex1;              // 0x34
    void* vertex2;              // 0x38
    void* vertex3;              // 0x3c
    void* vertexListTerminator; // 0x40
    char pad_044[0xe0];         // 0x44
    void* coinEntries;          // 0x124
    void* spriteEntries;        // 0x128
    int coinCount;              // 0x12c
    int spriteCount;            // 0x130
    WVector position;           // 0x134
    float fadeDuration;         // 0x140
    float opacity;              // 0x144
    int bounceSoundCount;       // 0x148
};

#define CCOIN_OFF(f, o) typedef char CCoin_off_##f[((int)&((CCoin*)0)->f == (o)) ? 1 : -1]
CCOIN_OFF(bitmapId, 0x14);
CCOIN_OFF(enabled, 0x18);
CCOIN_OFF(sprite, 0x1c);
CCOIN_OFF(sparkSprite, 0x20);
CCOIN_OFF(vertex0, 0x30);
CCOIN_OFF(vertex1, 0x34);
CCOIN_OFF(vertex2, 0x38);
CCOIN_OFF(vertex3, 0x3c);
CCOIN_OFF(vertexListTerminator, 0x40);
CCOIN_OFF(coinEntries, 0x124);
CCOIN_OFF(spriteEntries, 0x128);
CCOIN_OFF(coinCount, 0x12c);
CCOIN_OFF(spriteCount, 0x130);
CCOIN_OFF(fadeDuration, 0x140);
CCOIN_OFF(opacity, 0x144);
CCOIN_OFF(bounceSoundCount, 0x148);
#undef CCOIN_OFF

#endif // CLASSES_CCOIN_H
