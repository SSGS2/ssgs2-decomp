// Object801752FC. Outer receiver constructed at 0x801752FC; embedded CPvsFxBox starts at 0x208.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_OBJECT801752FC_H
#define CLASSES_OBJECT801752FC_H

class Object801752FC {
public:
    char pad_000[0x8];
    void* listNext; // 0x8: ScreenBallPreview_dtor traverses the list from self + 8
    int currentBallResource; // 0xC: ScreenBallPreview_Ctor initializes this outer receiver field
    void* ballRenderer; // 0x10: ScreenBallPreview_Ctor allocates CGIBallRenderer; ScreenBallPreview_dtor destroys it
    void* secondaryBallRenderer; // 0x14: ScreenBallPreview_Ctor initializes this outer receiver field
    void* ballRingRenderer; // 0x18: ScreenBallPreview_Ctor initializes this outer receiver field
    float screenCenterX; // 0x1C: ScreenBallPreview_Ctor initializes this outer receiver field
    float screenCenterY; // 0x20: ScreenBallPreview_Ctor initializes this outer receiver field
    char pad_024[0x40];
    float ballLightType; // 0x64: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballLightDiffuseRed; // 0x68: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballLightDiffuseGreen; // 0x6C: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballLightDiffuseBlue; // 0x70: ScreenBallPreview_Ctor initializes this outer receiver field
    char pad_074[0x14];
    float ballLightAmbientRed; // 0x88: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballLightAmbientGreen; // 0x8C: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballLightAmbientBlue; // 0x90: ScreenBallPreview_Ctor initializes this outer receiver field
    char pad_094[0xC];
    int ballLightPositionZ; // 0xA0: ScreenBallPreview_Draw
    float ballLightDirectionX; // 0xA4: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballLightDirectionY; // 0xA8: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballLightDirectionZ; // 0xAC: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballLightRange; // 0xB0: ScreenBallPreview_Draw
    float ballLightFalloff; // 0xB4: ScreenBallPreview_Draw
    float ballLightConstantAttenuation; // 0xB8: ScreenBallPreview_Draw
    float ballLightLinearAttenuation; // 0xBC: ScreenBallPreview_Draw
    float ballLightQuadraticAttenuation; // 0xC0: ScreenBallPreview_Draw
    float ballLightInnerConeAngle; // 0xC4: ScreenBallPreview_Draw
    float ballLightOuterConeAngle; // 0xC8: ScreenBallPreview_Draw
    char pad_0CC[0x2C];
    float ballViewTranslationZ; // 0xF8: ScreenBallPreview_Ctor initializes this outer receiver field
    int ballViewportX; // 0xFC: ScreenBallPreview_Ctor initializes this outer receiver field
    int ballViewportY; // 0x100: ScreenBallPreview_Ctor initializes this outer receiver field
    int ballViewportWidth; // 0x104: ScreenBallPreview_Ctor initializes this outer receiver field
    int ballViewportHeight; // 0x108: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballViewportMinDepth; // 0x10C: ScreenBallPreview_Ctor initializes this outer receiver field
    float ballViewportMaxDepth; // 0x110: ScreenBallPreview_Ctor initializes this outer receiver field
    char pad_114[0x2C];
    float strokePointViewTranslationZ; // 0x140: ScreenBallPreview_Ctor initializes this outer receiver field
    int strokePointViewportX; // 0x144: ScreenBallPreview_Ctor initializes this outer receiver field
    int strokePointViewportY; // 0x148: ScreenBallPreview_Ctor initializes this outer receiver field
    int strokePointViewportWidth; // 0x14C: ScreenBallPreview_Ctor initializes this outer receiver field
    int strokePointViewportHeight; // 0x150: ScreenBallPreview_Ctor initializes this outer receiver field
    float strokePointViewportMinDepth; // 0x154: ScreenBallPreview_Ctor initializes this outer receiver field
    float strokePointViewportMaxDepth; // 0x158: ScreenBallPreview_Ctor initializes this outer receiver field
    char pad_15C[0x24];
    float ringViewTranslationX; // 0x180: ScreenBallPreview_Ctor initializes this outer receiver field
    char pad_184[0x4];
    float ringViewTranslationZ; // 0x188: ScreenBallPreview_Ctor initializes this outer receiver field
    int ringViewportX; // 0x18C: ScreenBallPreview_Ctor initializes this outer receiver field
    int ringViewportY; // 0x190: ScreenBallPreview_Ctor initializes this outer receiver field
    int ringViewportWidth; // 0x194: ScreenBallPreview_Ctor initializes this outer receiver field
    int ringViewportHeight; // 0x198: ScreenBallPreview_Ctor initializes this outer receiver field
    float ringViewportMinDepth; // 0x19C: ScreenBallPreview_Ctor initializes this outer receiver field
    float ringViewportMaxDepth; // 0x1A0: ScreenBallPreview_Ctor initializes this outer receiver field
    float ringYaw; // 0x1A4: ScreenBallPreview_Ctor initializes this outer receiver field
    void* ringBitmapA; // 0x1A8: ScreenBallPreview_Ctor ResourceManager_LoadBitmap; ScreenBallPreview_dtor ResourceManager_ReleaseId
    void* ringBitmapB; // 0x1AC: ScreenBallPreview_Ctor ResourceManager_LoadBitmap; ScreenBallPreview_dtor ResourceManager_ReleaseId
    float rotationYaw; // 0x1B0: ScreenBallPreview_Update wraps at two pi and passes to Mtx44_SetRotationY
    float rotationPitch; // 0x1B4: ScreenBallPreview_Update negates, wraps at two pi, and passes to Mtx44_SetRotationX
    char pad_1B8[0x48];
    float unk_200; // 0x200: ScreenBallPreview_Ctor initializes this outer receiver field
    float rotationPhase; // 0x204: ScreenBallPreview_Ctor initializes this outer receiver field
    char pad_208[0x4];
    float unk_20C; // 0x20C: ScreenBallPreview_Ctor initializes this outer receiver field
    char pad_210[0x378];
    void* sprayAttachmentTreeRoot; // 0x588: ScreenBallPreview_dtor
    char pad_58C[0x8];
    void* sequenceAttachmentTreeRoot; // 0x594: ScreenBallPreview_dtor
    char pad_598[0x4];
    void* unk_59C; // 0x59C: ScreenBallPreview_Ctor initializes this outer receiver field
    void* unk_5A0; // 0x5A0: ScreenBallPreview_Ctor initializes this outer receiver field
    void* unk_5A4; // 0x5A4: ScreenBallPreview_Ctor initializes this outer receiver field
    void* unk_5A8; // 0x5A8: ScreenBallPreview_Ctor initializes this outer receiver field
    int unk_5AC; // 0x5AC: ScreenBallPreview_Ctor initializes this outer receiver field
};

#define OFF(f, o) typedef char Object801752FC_off_##f[((int)&((Object801752FC*)0)->f == (o)) ? 1 : -1]
OFF(listNext, 0x8);
OFF(currentBallResource, 0xC);
OFF(ballRenderer, 0x10);
OFF(secondaryBallRenderer, 0x14);
OFF(ballRingRenderer, 0x18);
OFF(screenCenterX, 0x1C);
OFF(screenCenterY, 0x20);
OFF(ballLightType, 0x64);
OFF(ballLightDiffuseRed, 0x68);
OFF(ballLightDiffuseGreen, 0x6C);
OFF(ballLightDiffuseBlue, 0x70);
OFF(ballLightAmbientRed, 0x88);
OFF(ballLightAmbientGreen, 0x8C);
OFF(ballLightAmbientBlue, 0x90);
OFF(ballLightPositionZ, 0xA0);
OFF(ballLightDirectionX, 0xA4);
OFF(ballLightDirectionY, 0xA8);
OFF(ballLightDirectionZ, 0xAC);
OFF(ballLightRange, 0xB0);
OFF(ballLightFalloff, 0xB4);
OFF(ballLightConstantAttenuation, 0xB8);
OFF(ballLightLinearAttenuation, 0xBC);
OFF(ballLightQuadraticAttenuation, 0xC0);
OFF(ballLightInnerConeAngle, 0xC4);
OFF(ballLightOuterConeAngle, 0xC8);
OFF(ballViewTranslationZ, 0xF8);
OFF(ballViewportX, 0xFC);
OFF(ballViewportY, 0x100);
OFF(ballViewportWidth, 0x104);
OFF(ballViewportHeight, 0x108);
OFF(ballViewportMinDepth, 0x10C);
OFF(ballViewportMaxDepth, 0x110);
OFF(strokePointViewTranslationZ, 0x140);
OFF(strokePointViewportX, 0x144);
OFF(strokePointViewportY, 0x148);
OFF(strokePointViewportWidth, 0x14C);
OFF(strokePointViewportHeight, 0x150);
OFF(strokePointViewportMinDepth, 0x154);
OFF(strokePointViewportMaxDepth, 0x158);
OFF(ringViewTranslationX, 0x180);
OFF(ringViewTranslationZ, 0x188);
OFF(ringViewportX, 0x18C);
OFF(ringViewportY, 0x190);
OFF(ringViewportWidth, 0x194);
OFF(ringViewportHeight, 0x198);
OFF(ringViewportMinDepth, 0x19C);
OFF(ringViewportMaxDepth, 0x1A0);
OFF(ringYaw, 0x1A4);
OFF(ringBitmapA, 0x1A8);
OFF(ringBitmapB, 0x1AC);
OFF(rotationYaw, 0x1B0);
OFF(rotationPitch, 0x1B4);
OFF(unk_200, 0x200);
OFF(rotationPhase, 0x204);
OFF(unk_20C, 0x20C);
OFF(sprayAttachmentTreeRoot, 0x588);
OFF(sequenceAttachmentTreeRoot, 0x594);
OFF(unk_59C, 0x59C);
OFF(unk_5A0, 0x5A0);
OFF(unk_5A4, 0x5A4);
OFF(unk_5A8, 0x5A8);
OFF(unk_5AC, 0x5AC);
#undef OFF
#define AsObject801752FC(p) ((Object801752FC*)(p))

#endif
