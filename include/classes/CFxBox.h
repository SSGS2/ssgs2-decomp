// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CFXBOX_H
#define CLASSES_CFXBOX_H

#include "game/types.h"

class CFxBox {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    float nominalStepMilliseconds;                        // 0x4: 1 use in CFxBox_Ctor
    float timeScale;                        // 0x8: 1 use in CFxBox_Ctor
    char unk_C;                         // 0xc: 1 use in CFxBox_Ctor
    char pad_00D[0x103];                // 0xd
    char pause;                       // 0x110: 1 use in CFxBox_Ctor
    char pad_111[0x3];                  // 0x111
    int renderFlags;                        // 0x114: 1 use in CFxBox_Ctor
    char clampSimulationStep;                       // 0x118: 1 use in CFxBox_Ctor
    char pad_119[0x3];                  // 0x119
    int unk_11C;                        // 0x11c: 1 use in CFxBox_Ctor
    int unk_120;                        // 0x120: 1 use in CFxBox_Ctor
    char debugBoundsEnabled;                       // 0x124: 1 use in CFxBox_Ctor
    char pad_125[0x3];                  // 0x125
    int sprayTemplateCacheCount;                        // 0x128: 1 use in CFxBox_Ctor
    int sprayTemplateCacheRoot;                        // 0x12c: 1 use in CFxBox_Ctor
    void* sprayTemplateCacheFirst;                      // 0x130: 1 use in CFxBox_Ctor
    int sequenceTemplateCacheCount;                        // 0x134: 1 use in CFxBox_Ctor
    int sequenceTemplateCacheRoot;                        // 0x138: 1 use in CFxBox_Ctor
    void* sequenceTemplateCacheFirst;                      // 0x13c: 1 use in CFxBox_Ctor
    int resourceManager;                        // 0x140: 1 use in CFxBox_Ctor
    int spriteCacheCount;                        // 0x144: 1 use in CFxBox_Ctor
    int spriteCacheRoot;                        // 0x148: 1 use in CFxBox_Ctor
    char pad_14C[0x4];                  // 0x14c
    void* spriteCacheFirst;                      // 0x150: 1 use in CFxBox_Ctor
    int sprayCount;                        // 0x154: 1 use in CFxBox_Ctor
    int sprayTreeRoot;                        // 0x158: 1 use in CFxBox_Ctor
    void* sprayTreeFirst;                      // 0x15c: 1 use in CFxBox_Ctor
    int sequenceCount;                        // 0x160: 1 use in CFxBox_Ctor
    int sequenceTreeRoot;                        // 0x164: 1 use in CFxBox_Ctor
    void* sequenceTreeFirst;                      // 0x168: 1 use in CFxBox_Ctor
    int cameraTransformAddress;                        // 0x16c: 1 use in CFxBox_Ctor
    void* unk_170;                      // 0x170: 1 use in CFxBox_Ctor
    void* unk_174;                      // 0x174: 1 use in CFxBox_Ctor
    void* unk_178;                      // 0x178: 1 use in CFxBox_Ctor
    void* unk_17C;                      // 0x17c: 1 use in CFxBox_Ctor
    int unk_180;                        // 0x180: 1 use in CFxBox_Ctor
    char pad_184[0x10];                 // 0x184
    int unk_194;                        // 0x194: 1 use in CFxBox_Ctor
    char pad_198[0x4];                  // 0x198
    float unk_19C;                      // 0x19c: 1 use in CFxBox_Ctor
    float unk_1A0;                      // 0x1a0: 1 use in CFxBox_Ctor
    char pad_1A4[0x28];                 // 0x1a4
    int unk_1CC;                        // 0x1cc: 1 use in CFxBox_Ctor
    char pad_1D0[0x4];                  // 0x1d0
    float unk_1D4;                      // 0x1d4: 1 use in CFxBox_Ctor
    float unk_1D8;                      // 0x1d8: 1 use in CFxBox_Ctor
    char pad_1DC[0x28];                 // 0x1dc
    int unk_204;                        // 0x204: 1 use in CFxBox_Ctor
    char pad_208[0x4];                  // 0x208
    float unk_20C;                      // 0x20c: 1 use in CFxBox_Ctor
    float unk_210;                      // 0x210: 1 use in CFxBox_Ctor
    char pad_214[0x28];                 // 0x214
    int unk_23C;                        // 0x23c: 1 use in CFxBox_Ctor
    char pad_240[0x4];                  // 0x240
    float unk_244;                      // 0x244: 1 use in CFxBox_Ctor
    float unk_248;                      // 0x248: 1 use in CFxBox_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x128, 0x12c, 0x134, 0x138, 0x144, 0x148, 0x154, 0x158, 0x160, 0x164, 0x184, 0x1bc, 0x1f4, 0x22c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CFXBOX_OFF(f, o) typedef char CFxBox_off_##f[((int)&((CFxBox*)0)->f == (o)) ? 1 : -1]
CFXBOX_OFF(nominalStepMilliseconds, 0x4);
CFXBOX_OFF(timeScale, 0x8);
CFXBOX_OFF(unk_C, 0xc);
CFXBOX_OFF(pause, 0x110);
CFXBOX_OFF(renderFlags, 0x114);
CFXBOX_OFF(clampSimulationStep, 0x118);
CFXBOX_OFF(unk_11C, 0x11c);
CFXBOX_OFF(unk_120, 0x120);
CFXBOX_OFF(debugBoundsEnabled, 0x124);
CFXBOX_OFF(sprayTemplateCacheCount, 0x128);
CFXBOX_OFF(sprayTemplateCacheRoot, 0x12c);
CFXBOX_OFF(sprayTemplateCacheFirst, 0x130);
CFXBOX_OFF(sequenceTemplateCacheCount, 0x134);
CFXBOX_OFF(sequenceTemplateCacheRoot, 0x138);
CFXBOX_OFF(sequenceTemplateCacheFirst, 0x13c);
CFXBOX_OFF(resourceManager, 0x140);
CFXBOX_OFF(spriteCacheCount, 0x144);
CFXBOX_OFF(spriteCacheRoot, 0x148);
CFXBOX_OFF(spriteCacheFirst, 0x150);
CFXBOX_OFF(sprayCount, 0x154);
CFXBOX_OFF(sprayTreeRoot, 0x158);
CFXBOX_OFF(sprayTreeFirst, 0x15c);
CFXBOX_OFF(sequenceCount, 0x160);
CFXBOX_OFF(sequenceTreeRoot, 0x164);
CFXBOX_OFF(sequenceTreeFirst, 0x168);
CFXBOX_OFF(cameraTransformAddress, 0x16c);
CFXBOX_OFF(unk_170, 0x170);
CFXBOX_OFF(unk_174, 0x174);
CFXBOX_OFF(unk_178, 0x178);
CFXBOX_OFF(unk_17C, 0x17c);
CFXBOX_OFF(unk_180, 0x180);
CFXBOX_OFF(unk_194, 0x194);
CFXBOX_OFF(unk_19C, 0x19c);
CFXBOX_OFF(unk_1A0, 0x1a0);
CFXBOX_OFF(unk_1CC, 0x1cc);
CFXBOX_OFF(unk_1D4, 0x1d4);
CFXBOX_OFF(unk_1D8, 0x1d8);
CFXBOX_OFF(unk_204, 0x204);
CFXBOX_OFF(unk_20C, 0x20c);
CFXBOX_OFF(unk_210, 0x210);
CFXBOX_OFF(unk_23C, 0x23c);
CFXBOX_OFF(unk_244, 0x244);
CFXBOX_OFF(unk_248, 0x248);
#undef CFXBOX_OFF

#endif  // CLASSES_CFXBOX_H
