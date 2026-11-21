// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CGIBALLRINGRENDERER_H
#define CLASSES_CGIBALLRINGRENDERER_H

#include "game/types.h"

class CGIBallRingRenderer {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x8];                  // 0x4
    int unk_C;                          // 0xc: 1 use in ScreenBallPreview_Ctor
    void* primaryBallRenderer;                       // 0x10: 1 use in ScreenBallPreview_Ctor
    void* secondaryBallRenderer;                       // 0x14: 1 use in ScreenBallPreview_Ctor
    void* ballRingRenderer;                       // 0x18: 1 use in ScreenBallPreview_Ctor
    float liePenaltyPercent;                       // 0x1c: 2 uses in ScreenBallPreview_Ctor, CGIBallRingRenderer_Draw
    int segmentCount;                         // 0x20: 6 uses in CGIBallRingRenderer_Draw, ScreenBallPreview_Ctor
    int ringTexture;                         // 0x24: 2 uses in CGIBallRingRenderer_Draw
    int percentageTexture;                         // 0x28: 2 uses in CGIBallRingRenderer_Draw
    char pad_02C[0x10];                 // 0x2c
    float vertex0U;                       // 0x3c: 2 uses in CGIBallRingRenderer_Draw
    float vertex0V;                       // 0x40: 2 uses in CGIBallRingRenderer_Draw
    char pad_044[0x10];                 // 0x44
    float vertex1U;                       // 0x54: 2 uses in CGIBallRingRenderer_Draw
    float vertex1V;                       // 0x58: 2 uses in CGIBallRingRenderer_Draw
    float vertex2X;                       // 0x5c: 2 uses in CGIBallRingRenderer_Draw
    char pad_060[0x4];                  // 0x60
    float vertex2Z;                       // 0x64: 3 uses in CGIBallRingRenderer_Draw, ScreenBallPreview_Ctor
    float vertex2Color;                       // 0x68: 1 use in ScreenBallPreview_Ctor
    float vertex2U;                       // 0x6c: 3 uses in CGIBallRingRenderer_Draw, ScreenBallPreview_Ctor
    float vertex2V;                       // 0x70: 3 uses in CGIBallRingRenderer_Draw, ScreenBallPreview_Ctor
    float vertex3X;                       // 0x74: 2 uses in CGIBallRingRenderer_Draw
    char pad_078[0x4];                  // 0x78
    float vertex3Z;                       // 0x7c: 2 uses in CGIBallRingRenderer_Draw
    char pad_080[0x4];                  // 0x80
    float vertex3U;                       // 0x84: 2 uses in CGIBallRingRenderer_Draw
    float vertex3V;                       // 0x88: 3 uses in CGIBallRingRenderer_Draw, ScreenBallPreview_Ctor
    float unk_8C;                       // 0x8c: 1 use in ScreenBallPreview_Ctor
    float unk_90;                       // 0x90: 1 use in ScreenBallPreview_Ctor
    char pad_094[0x10];                 // 0x94
    float unk_A4;                       // 0xa4: 1 use in ScreenBallPreview_Ctor
    float unk_A8;                       // 0xa8: 1 use in ScreenBallPreview_Ctor
    float unk_AC;                       // 0xac: 1 use in ScreenBallPreview_Ctor
    char pad_0B0[0x48];                 // 0xb0
    float unk_F8;                       // 0xf8: 1 use in ScreenBallPreview_Ctor
    int unk_FC;                         // 0xfc: 1 use in ScreenBallPreview_Ctor
    int unk_100;                        // 0x100: 1 use in ScreenBallPreview_Ctor
    int unk_104;                        // 0x104: 1 use in ScreenBallPreview_Ctor
    int unk_108;                        // 0x108: 1 use in ScreenBallPreview_Ctor
    float unk_10C;                      // 0x10c: 1 use in ScreenBallPreview_Ctor
    float unk_110;                      // 0x110: 1 use in ScreenBallPreview_Ctor
    char pad_114[0x2c];                 // 0x114
    float unk_140;                      // 0x140: 1 use in ScreenBallPreview_Ctor
    int unk_144;                        // 0x144: 1 use in ScreenBallPreview_Ctor
    int unk_148;                        // 0x148: 1 use in ScreenBallPreview_Ctor
    int unk_14C;                        // 0x14c: 1 use in ScreenBallPreview_Ctor
    int unk_150;                        // 0x150: 1 use in ScreenBallPreview_Ctor
    float unk_154;                      // 0x154: 1 use in ScreenBallPreview_Ctor
    float unk_158;                      // 0x158: 1 use in ScreenBallPreview_Ctor
    char pad_15C[0x24];                 // 0x15c
    float unk_180;                      // 0x180: 1 use in ScreenBallPreview_Ctor
    char pad_184[0x4];                  // 0x184
    float unk_188;                      // 0x188: 1 use in ScreenBallPreview_Ctor
    int unk_18C;                        // 0x18c: 1 use in ScreenBallPreview_Ctor
    int unk_190;                        // 0x190: 1 use in ScreenBallPreview_Ctor
    int unk_194;                        // 0x194: 1 use in ScreenBallPreview_Ctor
    int unk_198;                        // 0x198: 1 use in ScreenBallPreview_Ctor
    float unk_19C;                      // 0x19c: 1 use in ScreenBallPreview_Ctor
    float unk_1A0;                      // 0x1a0: 1 use in ScreenBallPreview_Ctor
    float unk_1A4;                      // 0x1a4: 1 use in ScreenBallPreview_Ctor
    void* ringTextureResource;                      // 0x1a8: 3 uses in ScreenBallPreview_Ctor
    void* percentageTextureResource;                      // 0x1ac: 3 uses in ScreenBallPreview_Ctor
    char pad_1B0[0x50];                 // 0x1b0
    float unk_200;                      // 0x200: 1 use in ScreenBallPreview_Ctor
    float unk_204;                      // 0x204: 1 use in ScreenBallPreview_Ctor
    char pad_208[0x4];                  // 0x208
    float unk_20C;                      // 0x20c: 1 use in ScreenBallPreview_Ctor
    char pad_210[0x38c];                // 0x210
    void* unk_59C;                      // 0x59c: 1 use in ScreenBallPreview_Ctor
    void* unk_5A0;                      // 0x5a0: 1 use in ScreenBallPreview_Ctor
    void* unk_5A4;                      // 0x5a4: 1 use in ScreenBallPreview_Ctor
    void* unk_5A8;                      // 0x5a8: 1 use in ScreenBallPreview_Ctor
    int unk_5AC;                        // 0x5ac: 1 use in ScreenBallPreview_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x28, 0x2c, 0x34, 0x58, 0x64, 0xa4, 0xcc, 0x114, 0x15c, 0x1b0, 0x1b8, 0x1f8, 0x208, 0x5b0, 0x5e8, 0x620, 0x658
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CGIBALLRINGRENDERER_OFF(f, o) typedef char CGIBallRingRenderer_off_##f[((int)&((CGIBallRingRenderer*)0)->f == (o)) ? 1 : -1]
CGIBALLRINGRENDERER_OFF(unk_C, 0xc);
CGIBALLRINGRENDERER_OFF(primaryBallRenderer, 0x10);
CGIBALLRINGRENDERER_OFF(secondaryBallRenderer, 0x14);
CGIBALLRINGRENDERER_OFF(ballRingRenderer, 0x18);
CGIBALLRINGRENDERER_OFF(liePenaltyPercent, 0x1c);
CGIBALLRINGRENDERER_OFF(segmentCount, 0x20);
CGIBALLRINGRENDERER_OFF(ringTexture, 0x24);
CGIBALLRINGRENDERER_OFF(percentageTexture, 0x28);
CGIBALLRINGRENDERER_OFF(vertex0U, 0x3c);
CGIBALLRINGRENDERER_OFF(vertex0V, 0x40);
CGIBALLRINGRENDERER_OFF(vertex1U, 0x54);
CGIBALLRINGRENDERER_OFF(vertex1V, 0x58);
CGIBALLRINGRENDERER_OFF(vertex2X, 0x5c);
CGIBALLRINGRENDERER_OFF(vertex2Z, 0x64);
CGIBALLRINGRENDERER_OFF(vertex2Color, 0x68);
CGIBALLRINGRENDERER_OFF(vertex2U, 0x6c);
CGIBALLRINGRENDERER_OFF(vertex2V, 0x70);
CGIBALLRINGRENDERER_OFF(vertex3X, 0x74);
CGIBALLRINGRENDERER_OFF(vertex3Z, 0x7c);
CGIBALLRINGRENDERER_OFF(vertex3U, 0x84);
CGIBALLRINGRENDERER_OFF(vertex3V, 0x88);
CGIBALLRINGRENDERER_OFF(unk_8C, 0x8c);
CGIBALLRINGRENDERER_OFF(unk_90, 0x90);
CGIBALLRINGRENDERER_OFF(unk_A4, 0xa4);
CGIBALLRINGRENDERER_OFF(unk_A8, 0xa8);
CGIBALLRINGRENDERER_OFF(unk_AC, 0xac);
CGIBALLRINGRENDERER_OFF(unk_F8, 0xf8);
CGIBALLRINGRENDERER_OFF(unk_FC, 0xfc);
CGIBALLRINGRENDERER_OFF(unk_100, 0x100);
CGIBALLRINGRENDERER_OFF(unk_104, 0x104);
CGIBALLRINGRENDERER_OFF(unk_108, 0x108);
CGIBALLRINGRENDERER_OFF(unk_10C, 0x10c);
CGIBALLRINGRENDERER_OFF(unk_110, 0x110);
CGIBALLRINGRENDERER_OFF(unk_140, 0x140);
CGIBALLRINGRENDERER_OFF(unk_144, 0x144);
CGIBALLRINGRENDERER_OFF(unk_148, 0x148);
CGIBALLRINGRENDERER_OFF(unk_14C, 0x14c);
CGIBALLRINGRENDERER_OFF(unk_150, 0x150);
CGIBALLRINGRENDERER_OFF(unk_154, 0x154);
CGIBALLRINGRENDERER_OFF(unk_158, 0x158);
CGIBALLRINGRENDERER_OFF(unk_180, 0x180);
CGIBALLRINGRENDERER_OFF(unk_188, 0x188);
CGIBALLRINGRENDERER_OFF(unk_18C, 0x18c);
CGIBALLRINGRENDERER_OFF(unk_190, 0x190);
CGIBALLRINGRENDERER_OFF(unk_194, 0x194);
CGIBALLRINGRENDERER_OFF(unk_198, 0x198);
CGIBALLRINGRENDERER_OFF(unk_19C, 0x19c);
CGIBALLRINGRENDERER_OFF(unk_1A0, 0x1a0);
CGIBALLRINGRENDERER_OFF(unk_1A4, 0x1a4);
CGIBALLRINGRENDERER_OFF(ringTextureResource, 0x1a8);
CGIBALLRINGRENDERER_OFF(percentageTextureResource, 0x1ac);
CGIBALLRINGRENDERER_OFF(unk_200, 0x200);
CGIBALLRINGRENDERER_OFF(unk_204, 0x204);
CGIBALLRINGRENDERER_OFF(unk_20C, 0x20c);
CGIBALLRINGRENDERER_OFF(unk_59C, 0x59c);
CGIBALLRINGRENDERER_OFF(unk_5A0, 0x5a0);
CGIBALLRINGRENDERER_OFF(unk_5A4, 0x5a4);
CGIBALLRINGRENDERER_OFF(unk_5A8, 0x5a8);
CGIBALLRINGRENDERER_OFF(unk_5AC, 0x5ac);
#undef CGIBALLRINGRENDERER_OFF

#endif  // CLASSES_CGIBALLRINGRENDERER_H
