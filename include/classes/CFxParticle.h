// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXPARTICLE_H
#define CLASSES_CFXPARTICLE_H

#include "game/types.h"

class CFxParticle {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x8];                  // 0x4
    float positionY;                        // 0xc: 1 use in CFxSpray_Ctor
    float positionZ;                       // 0x10: 1 use in CFxSpray_Ctor
    char pad_014[0x54];                 // 0x14
    unsigned int listNodeCount;                // 0x68: 2 uses in CFxParticle_ctor, CFxParticle_dtor
    void* listTail;                       // 0x6c: 1 use in CFxParticle_ctor
    void* listHead;                       // 0x70: 2 uses in CFxParticle_ctor, CFxParticle_dtor
    char pad_074[0x4];                  // 0x74
    void* velocityY;                       // 0x78: 1 use in CFxSpray_Ctor
    unsigned int velocityZ;                // 0x7c: 1 use in CFxSpray_Ctor
    float forceScale;                       // 0x80: 2 uses in CFxParticle_ctor, CFxSpray_Ctor
    void* angularVelocityXBits;                       // 0x84: 1 use in CFxSpray_Ctor
    char pad_088[0x34];                 // 0x88
    unsigned int spriteResource;                // 0xbc: 1 use in CFxParticle_ctor
    void* petBody;                       // 0xc0: 3 uses in CFxParticle_dtor, CFxParticle_ctor
    unsigned int positionHistory;                // 0xc4: 2 uses in CFxParticle_ctor, CFxParticle_dtor
    unsigned int positionHistoryReadIndex;                // 0xc8: 1 use in CFxParticle_ctor
    unsigned int positionHistoryWriteIndex;                // 0xcc: 1 use in CFxParticle_ctor
    unsigned int positionHistoryCapacity;                // 0xd0: 1 use in CFxParticle_ctor
    unsigned int ribbonHistory;                // 0xd4: 2 uses in CFxParticle_ctor, CFxParticle_dtor
    unsigned int ribbonHistoryReadIndex;                // 0xd8: 1 use in CFxParticle_ctor
    unsigned int ribbonHistoryWriteIndex;                // 0xdc: 1 use in CFxParticle_ctor
    unsigned int ribbonHistoryCapacity;                // 0xe0: 1 use in CFxParticle_ctor
    char pad_0E4[0x4];                  // 0xe4
    float opacityScale;                       // 0xe8: 1 use in CFxParticle_ctor
    char pad_0EC[0x170];                // 0xec
    unsigned int velocityKeyCount;               // 0x25c: 1 use in CFxSpray_Ctor
    void* velocityKeyTail;                      // 0x260: 1 use in CFxSpray_Ctor
    void* velocityKeyHead;                      // 0x264: 1 use in CFxSpray_Ctor
    char pad_268[0x30];                 // 0x268
    unsigned int rotationKeyCount;               // 0x298: 1 use in CFxSpray_Ctor
    void* rotationKeyTail;                      // 0x29c: 1 use in CFxSpray_Ctor
    void* rotationKeyHead;                      // 0x2a0: 1 use in CFxSpray_Ctor
    unsigned int unk_2A4;               // 0x2a4: 1 use in CFxSpray_Ctor
    void* unk_2A8;                      // 0x2a8: 1 use in CFxSpray_Ctor
    void* unk_2AC;                      // 0x2ac: 1 use in CFxSpray_Ctor
    char pad_2B0[0x4];                  // 0x2b0
    void* unk_2B4;                      // 0x2b4: 1 use in CFxSpray_Ctor
    void* unk_2B8;                      // 0x2b8: 1 use in CFxSpray_Ctor
    unsigned int unk_2BC;               // 0x2bc: 1 use in CFxSpray_Ctor
    void* unk_2C0;                      // 0x2c0: 1 use in CFxSpray_Ctor
    void* unk_2C4;                      // 0x2c4: 1 use in CFxSpray_Ctor
    unsigned int unk_2C8;               // 0x2c8: 1 use in CFxSpray_Ctor
    void* unk_2CC;                      // 0x2cc: 1 use in CFxSpray_Ctor
    void* unk_2D0;                      // 0x2d0: 1 use in CFxSpray_Ctor
    unsigned int generationKeyCount;               // 0x2d4: 1 use in CFxSpray_Ctor
    void* generationKeyTail;                      // 0x2d8: 1 use in CFxSpray_Ctor
    void* generationKeyHead;                      // 0x2dc: 1 use in CFxSpray_Ctor
    unsigned int gravityPointKeyCount;               // 0x2e0: 1 use in CFxSpray_Ctor
    void* gravityPointKeyTail;                      // 0x2e4: 1 use in CFxSpray_Ctor
    void* gravityPointKeyHead;                      // 0x2e8: 1 use in CFxSpray_Ctor
    unsigned int fadeKeyCount;               // 0x2ec: 1 use in CFxSpray_Ctor
    void* fadeKeyTail;                      // 0x2f0: 1 use in CFxSpray_Ctor
    void* fadeKeyHead;                      // 0x2f4: 1 use in CFxSpray_Ctor
    char pad_2F8[0xc];                  // 0x2f8
    unsigned int sizeKeyCount;               // 0x304: 1 use in CFxSpray_Ctor
    void* sizeKeyTail;                      // 0x308: 1 use in CFxSpray_Ctor
    void* sizeKeyHead;                      // 0x30c: 1 use in CFxSpray_Ctor
    unsigned int frameKeyCount;               // 0x310: 1 use in CFxSpray_Ctor
    void* frameKeyTail;                      // 0x314: 1 use in CFxSpray_Ctor
    void* frameKeyHead;                      // 0x318: 1 use in CFxSpray_Ctor
    unsigned int quakeKeyCount;               // 0x31c: 1 use in CFxSpray_Ctor
    void* quakeKeyTail;                      // 0x320: 1 use in CFxSpray_Ctor
    void* quakeKeyHead;                      // 0x324: 1 use in CFxSpray_Ctor
    unsigned int flashKeyCount;               // 0x328: 1 use in CFxSpray_Ctor
    void* flashKeyTail;                      // 0x32c: 1 use in CFxSpray_Ctor
    void* flashKeyHead;                      // 0x330: 1 use in CFxSpray_Ctor
    char pad_334[0x8];                  // 0x334
    unsigned int bubbleKeyCount;               // 0x33c: 1 use in CFxSpray_Ctor
    void* bubbleKeyTail;                      // 0x340: 1 use in CFxSpray_Ctor
    void* bubbleKeyHead;                      // 0x344: 1 use in CFxSpray_Ctor
    unsigned int unk_348;               // 0x348: 1 use in CFxSpray_Ctor
    void* unk_34C;                      // 0x34c: 1 use in CFxSpray_Ctor
    void* unk_350;                      // 0x350: 1 use in CFxSpray_Ctor
    unsigned int soundKeyCount;               // 0x354: 1 use in CFxSpray_Ctor
    void* soundKeyTail;                      // 0x358: 1 use in CFxSpray_Ctor
    void* soundKeyHead;                      // 0x35c: 1 use in CFxSpray_Ctor
    char pad_360[0xc];                  // 0x360
    unsigned char unk_36C;              // 0x36c: 2 uses in CFxSpray_Ctor
    char pad_36D[0x3];                  // 0x36d
    void* unk_370;                      // 0x370: 1 use in CFxSpray_Ctor
    unsigned int unk_374;               // 0x374: 1 use in CFxSpray_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x14, 0x24, 0x6c, 0x80, 0xc4, 0xd4, 0x1a8, 0x1b4, 0x260, 0x29c, 0x2a8, 0x2b4, 0x2c0, 0x2cc, 0x2d8, 0x2e4, 0x2f0, 0x308, 0x314, 0x320, 0x32c, 0x340, 0x34c, 0x358
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXPARTICLE_OFF(f, o) typedef char CFxParticle_off_##f[((int)&((CFxParticle*)0)->f == (o)) ? 1 : -1]
CFXPARTICLE_OFF(positionY, 0xc);
CFXPARTICLE_OFF(positionZ, 0x10);
CFXPARTICLE_OFF(listNodeCount, 0x68);
CFXPARTICLE_OFF(listTail, 0x6c);
CFXPARTICLE_OFF(listHead, 0x70);
CFXPARTICLE_OFF(velocityY, 0x78);
CFXPARTICLE_OFF(velocityZ, 0x7c);
CFXPARTICLE_OFF(forceScale, 0x80);
CFXPARTICLE_OFF(angularVelocityXBits, 0x84);
CFXPARTICLE_OFF(spriteResource, 0xbc);
CFXPARTICLE_OFF(petBody, 0xc0);
CFXPARTICLE_OFF(positionHistory, 0xc4);
CFXPARTICLE_OFF(positionHistoryReadIndex, 0xc8);
CFXPARTICLE_OFF(positionHistoryWriteIndex, 0xcc);
CFXPARTICLE_OFF(positionHistoryCapacity, 0xd0);
CFXPARTICLE_OFF(ribbonHistory, 0xd4);
CFXPARTICLE_OFF(ribbonHistoryReadIndex, 0xd8);
CFXPARTICLE_OFF(ribbonHistoryWriteIndex, 0xdc);
CFXPARTICLE_OFF(ribbonHistoryCapacity, 0xe0);
CFXPARTICLE_OFF(opacityScale, 0xe8);
CFXPARTICLE_OFF(velocityKeyCount, 0x25c);
CFXPARTICLE_OFF(velocityKeyTail, 0x260);
CFXPARTICLE_OFF(velocityKeyHead, 0x264);
CFXPARTICLE_OFF(rotationKeyCount, 0x298);
CFXPARTICLE_OFF(rotationKeyTail, 0x29c);
CFXPARTICLE_OFF(rotationKeyHead, 0x2a0);
CFXPARTICLE_OFF(unk_2A4, 0x2a4);
CFXPARTICLE_OFF(unk_2A8, 0x2a8);
CFXPARTICLE_OFF(unk_2AC, 0x2ac);
CFXPARTICLE_OFF(unk_2B4, 0x2b4);
CFXPARTICLE_OFF(unk_2B8, 0x2b8);
CFXPARTICLE_OFF(unk_2BC, 0x2bc);
CFXPARTICLE_OFF(unk_2C0, 0x2c0);
CFXPARTICLE_OFF(unk_2C4, 0x2c4);
CFXPARTICLE_OFF(unk_2C8, 0x2c8);
CFXPARTICLE_OFF(unk_2CC, 0x2cc);
CFXPARTICLE_OFF(unk_2D0, 0x2d0);
CFXPARTICLE_OFF(generationKeyCount, 0x2d4);
CFXPARTICLE_OFF(generationKeyTail, 0x2d8);
CFXPARTICLE_OFF(generationKeyHead, 0x2dc);
CFXPARTICLE_OFF(gravityPointKeyCount, 0x2e0);
CFXPARTICLE_OFF(gravityPointKeyTail, 0x2e4);
CFXPARTICLE_OFF(gravityPointKeyHead, 0x2e8);
CFXPARTICLE_OFF(fadeKeyCount, 0x2ec);
CFXPARTICLE_OFF(fadeKeyTail, 0x2f0);
CFXPARTICLE_OFF(fadeKeyHead, 0x2f4);
CFXPARTICLE_OFF(sizeKeyCount, 0x304);
CFXPARTICLE_OFF(sizeKeyTail, 0x308);
CFXPARTICLE_OFF(sizeKeyHead, 0x30c);
CFXPARTICLE_OFF(frameKeyCount, 0x310);
CFXPARTICLE_OFF(frameKeyTail, 0x314);
CFXPARTICLE_OFF(frameKeyHead, 0x318);
CFXPARTICLE_OFF(quakeKeyCount, 0x31c);
CFXPARTICLE_OFF(quakeKeyTail, 0x320);
CFXPARTICLE_OFF(quakeKeyHead, 0x324);
CFXPARTICLE_OFF(flashKeyCount, 0x328);
CFXPARTICLE_OFF(flashKeyTail, 0x32c);
CFXPARTICLE_OFF(flashKeyHead, 0x330);
CFXPARTICLE_OFF(bubbleKeyCount, 0x33c);
CFXPARTICLE_OFF(bubbleKeyTail, 0x340);
CFXPARTICLE_OFF(bubbleKeyHead, 0x344);
CFXPARTICLE_OFF(unk_348, 0x348);
CFXPARTICLE_OFF(unk_34C, 0x34c);
CFXPARTICLE_OFF(unk_350, 0x350);
CFXPARTICLE_OFF(soundKeyCount, 0x354);
CFXPARTICLE_OFF(soundKeyTail, 0x358);
CFXPARTICLE_OFF(soundKeyHead, 0x35c);
CFXPARTICLE_OFF(unk_36C, 0x36c);
CFXPARTICLE_OFF(unk_370, 0x370);
CFXPARTICLE_OFF(unk_374, 0x374);
#undef CFXPARTICLE_OFF

#endif  // CLASSES_CFXPARTICLE_H
