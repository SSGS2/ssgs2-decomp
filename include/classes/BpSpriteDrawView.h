// BpSpriteDrawView: pooled sprite layout used by BpCtrl_CreateDrawObject and draw/bounds methods.
// Layout view only; no methods, vtable or RTTI are emitted.
#ifndef CLASSES_BPDRAWOBJECT800BC91C_H
#define CLASSES_BPDRAWOBJECT800BC91C_H

class BpSpriteDrawView {
public:
    char pad_0[0xC];
    int drawRectCallback; // 0xC: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    int drawQuadCallback; // 0x10: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_14[0x8];
    int image; // 0x1C: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_20[0x14];
    int sourceX; // 0x34: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_38[0x14];
    int sourceY; // 0x4C: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_50[0x14];
    int sourceWidth; // 0x64: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_68[0x14];
    int sourceHeight; // 0x7C: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_80[0x2C];
    int positionX; // 0xAC: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_B0[0x8];
    char* formatProperty; // 0xB8: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_BC[0x8];
    int positionY; // 0xC4: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_C8[0x14];
    int scaleX; // 0xDC: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_E0[0x14];
    int scaleY; // 0xF4: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_F8[0x14];
    int opacity; // 0x10C: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_110[0x8];
    char* blendProperty; // 0x118: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_11C[0x20];
    int tileColumns; // 0x13C: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_140[0x8];
    char* tileFlipProperty; // 0x148: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_14C[0x8];
    int tileRows; // 0x154: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_158[0x14];
    int pivotX; // 0x16C: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_170[0x14];
    int pivotY; // 0x184: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_188[0x14];
    int rotation; // 0x19C: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
    char pad_1A0[0x14];
    int tintColor; // 0x1B4: BpSprite_DrawTilesUnrotated, BpSprite_DrawTilesRotated, BpSprite_ComputeUnrotatedRect / BpSprite_ComputeRotatedQuad
};

#define OFF(f, o) typedef char BpDrawObject800BC91C_off_##f[((int)&((BpSpriteDrawView*)0)->f == (o)) ? 1 : -1]
OFF(drawRectCallback, 0xC);
OFF(drawQuadCallback, 0x10);
OFF(image, 0x1C);
OFF(sourceX, 0x34);
OFF(sourceY, 0x4C);
OFF(sourceWidth, 0x64);
OFF(sourceHeight, 0x7C);
OFF(positionX, 0xAC);
OFF(formatProperty, 0xB8);
OFF(positionY, 0xC4);
OFF(scaleX, 0xDC);
OFF(scaleY, 0xF4);
OFF(opacity, 0x10C);
OFF(blendProperty, 0x118);
OFF(tileColumns, 0x13C);
OFF(tileFlipProperty, 0x148);
OFF(tileRows, 0x154);
OFF(pivotX, 0x16C);
OFF(pivotY, 0x184);
OFF(rotation, 0x19C);
OFF(tintColor, 0x1B4);
#undef OFF

#endif
