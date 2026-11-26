// GX command FIFO writers and TEV combiner setup: the helpers that push vertex data, indices
// and combiner inputs into the GX write port.
//
// Each Render_Write* function emits one vertex layout, in the attribute order the active GX
// vertex descriptor expects. The Render_Configure* and Engine_Configure* functions set the
// color and alpha combiner inputs of the TEV stages.

#include "sdk_protos.h"

// GX command FIFO: each volatile write emits one attribute component or index.
enum {
    kGXFifoAddress = 0xCC008000
};

enum TevAlphaInput {
    kAlphaPrevious = 0,
    kAlphaRegister0 = 1,
    kAlphaRegister1 = 2,
    kAlphaRegister2 = 3,
    kAlphaTexture = 4,
    kAlphaRaster = 5,
    kAlphaKonst = 6,
    kAlphaZero = 7
};
enum TevColorInput {
    kColorPrevious = 0,
    kColorPreviousAlpha = 1,
    kColorRegister0 = 2,
    kColorRegister0Alpha = 3,
    kColorRegister1 = 4,
    kColorRegister1Alpha = 5,
    kColorRegister2 = 6,
    kColorRegister2Alpha = 7,
    kColorTexture = 8,
    kColorTextureAlpha = 9,
    kColorRaster = 10,
    kColorRasterAlpha = 11,
    kColorOne = 12,
    kColorHalf = 13,
    kColorKonst = 14,
    kColorZero = 15
};
enum TevOperation {
    kTevAdd = 0,
    kTevSubtract = 1
};
enum TevBias {
    kTevBiasZero = 0
};
enum TevScale {
    kTevScaleOne = 0,
    kTevScaleTwo = 1
};
enum TevOutput {
    kTevPrevious = 0,
    kTevRegister0 = 1,
    kTevRegister1 = 2,
    kTevRegister2 = 3
};

enum TevKonstSelector {
    kKonstAlphaOne = 0,
    kKonstColor0Rgb = 12,
    kKonstColor1Rgb = 13,
    kKonstColor2Rgb = 14,
    kKonstColor0Alpha = 28,
    kKonstColor1Alpha = 29
};
enum CachedAlphaArgument {
    kCachedPrevious0 = 0,
    kCachedPrevious1 = 1,
    kCachedTexture = 2,
    kCachedStageConstant = 3
};
enum CachedAlphaOperation {
    kAlphaMultiply = 0,
    kAlphaAdd = 2,
    kAlphaPassRasterOrPrevious = 16,
    kAlphaSelectFirst = 17,
    kAlphaSelectSecond = 18,
    kAlphaBlendMaterial = 19,
    kAlphaMultiplyAddStage = 20
};

// FIFO contracts: values is borrowed and contains count complete records. The
// active GX vertex descriptor must consume exactly the attributes emitted here;
// index helpers repeat an index for separate attributes, not separate vertices.
// Packed RGBA occupies one word (byte order R,G,B,A); it is never a float.
// Position/UV spaces come from the caller; these writers perform no conversion.
// Record strides in 32-bit words:
//   Position=3, PositionColor=4, PositionTex1=5, PositionTex1Stride24=6,
//   PositionColorTex1=6, PositionTex2=7, PositionColorTex2=8,
//   PositionTex3=9, PositionColorTex3=10, PositionColorTex4=12.
// Volatile accesses and optimization pragmas preserve the hardware write order.

// Cached arguments 0 and 1 both select previous alpha in this translator;
// 2 selects texture alpha, and 3 selects the stage constant only in operation 20.
// A stage occupies 100 bytes. constantColors[0] supplies the material blend;
// constantColors[index] supplies operation 20's optional constant input.

struct AlphaStageState {
    int operation, firstArgument, secondArgument;
    char padding[0x24];
    int addendArgument;
    char tail[0x30];
};
struct AlphaCombinerState {
    char prefix[0xdc];
    unsigned char constantColors[8][4];
    char padding[0x30];
    AlphaStageState stages[8];
};
typedef char AlphaStageStrideCheck[sizeof(AlphaStageState) == 100 ? 1 : -1];
typedef char AlphaStageOffsetCheck[(int)&((AlphaCombinerState*)0)->stages == 0x12c ? 1 : -1];

extern "C" {

int TextureStage_HasActiveEntryAtOrBeyondLimit(void* p, unsigned int limit);
int TextureStage_GetAlphaCombinerBaseIndex(void* p, int index);
int TextureStage_UsesSplitIndexedPalette(void* p, int index, int stage);
int TextureStage_UsesPairedTextureFormat(void* p, int index);

void GXSetTevKColorSel(int, int);
void GXSetTevKAlphaSel(int, int);
void GXSetTevColorS10(int, const unsigned short*);
void GXSetTevKColor(int, const unsigned char*);

extern const unsigned short kTevInitialRegister0Red;
extern const unsigned short kTevInitialRegister0Green;
extern const unsigned short kTevInitialRegister0Blue;
extern const unsigned short kTevInitialRegister0Alpha;
extern const unsigned char kTevKonstColor0Red;
extern const unsigned char kTevKColor0Green;
extern const unsigned char kTevKColor0Blue;
extern const unsigned char kTevKonstColor0Alpha;
extern const unsigned char kTevKonstColor1Red;
extern const unsigned char kTevKColor1Green;
extern const unsigned char kByte_0_EngineMaterialBlue;
extern const unsigned char kRenderColorAlpha182;
extern const unsigned char kTevKonstColor2Red;
extern const unsigned char kTevKColor2Green;
extern const unsigned char kTevKColor2Blue;
extern const unsigned char kTevKonstColor2Alpha;

void Render_WriteTexCoord2f(float x, float y);
void Render_WriteFifo3f(float x, float y, float z);

#pragma optimization_level 1
// Emit one 16-bit FIFO index for each input element.
void Render_WriteFifoU16Array(volatile unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    for (unsigned int i = 0; i < limit; ++i) {
        *(volatile unsigned short*)kGXFifoAddress = *p++;
    }
}
#pragma optimization_level reset

#pragma optimization_level 1
// Emit each input index twice, for two independently indexed vertex attributes.
void Render_WriteFifoU16Indices2(volatile unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    for (unsigned int i = 0; i < limit; ++i) {
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        p++;
    }
}

// Emit each input index twice; this entry point preserves the alternate loop form.
void Render_WriteFifoU16Indices2_Alternate(volatile unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    for (unsigned int i = 0; i < limit; ++i) {
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        p++;
    }
}
#pragma optimization_level reset

// Emit packed XYZ and one UV pair from five-float vertices.
void Render_WriteVerticesPositionTex1(float* values, unsigned int count) {
    volatile unsigned int limit = count;
    float* p = values;
    unsigned int i = 0;
    while (i < limit) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        Render_WriteTexCoord2f(p[3], p[4]);
        p += 5;
        ++i;
    }
}

// Emit XYZ and UV from six-word vertices, skipping the fourth word.
void Render_WriteVerticesPositionTex1Stride24(float* values, unsigned int count) {
    volatile unsigned int limit = count;
    float* p = values;
    unsigned int i = 0;
    while (i < limit) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        Render_WriteTexCoord2f(p[4], p[5]);
        p += 6;
        ++i;
    }
}

// Emit packed XYZ and one UV pair through the alternate entry point.
void Render_WritePositionTexCoordVertices(float* values, unsigned int count) {
    volatile unsigned int limit = count;
    float* p = values;
    unsigned int i = 0;
    while (i < limit) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        Render_WriteTexCoord2f(p[3], p[4]);
        p += 5;
        ++i;
    }
}

#pragma optimization_level 0
// Emit each input index five times for five indexed vertex attributes.
void Render_WriteFifoU16Indices5(unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    unsigned int i = 0;
    while (i < limit) {
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        ++p;
        ++i;
    }
}

// Emit XYZ, packed RGBA, and one UV pair from six-word vertices.
void Render_WriteVerticesPositionColorTex1(float* values, unsigned int count) {
    float* p = values;
    unsigned int i = 0;
    while (i < count) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        *(volatile unsigned int*)kGXFifoAddress = *(unsigned int*)(p + 3);
        Render_WriteTexCoord2f(p[4], p[5]);
        p += 6;
        ++i;
    }
}

#pragma optimization_level 0
// Emit two copies of each index while preserving the original volatile loop ordering.
void Render_WriteFifoU16Indices2_Ordered(volatile unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    unsigned int i = 0;
    while (i < limit) {
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        ++i;
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        ++p;
    }
}

// Emit two copies of each index through the non-volatile-input entry point.
void Render_WriteFifoU16Indices2_OrderedAlternate(unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    unsigned int i = 0;
    while (i < limit) {
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        ++i;
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        ++p;
    }
}
#pragma optimization_level reset

// Emit three copies of each index for three indexed vertex attributes.
void Render_WriteFifoU16Indices3(unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    unsigned int i = 0;
    while (i < limit) {
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        ++i;
        *(volatile unsigned short*)kGXFifoAddress = p[0];
        ++p;
    }
}
#pragma optimization_level reset

#pragma optimization_level 0
// Emit one floating-point UV pair to the GX FIFO.
void Render_WriteTexCoord2f(float x, float y) {
    *(volatile float*)kGXFifoAddress = x;
    *(volatile float*)kGXFifoAddress = y;
}
#pragma optimization_level reset

#pragma optimization_level 0
// Emit one floating-point XYZ position to the GX FIFO.
void Render_WriteFifo3f(float x, float y, float z) {
    *(volatile float*)kGXFifoAddress = x;
    *(volatile float*)kGXFifoAddress = y;
    *(volatile float*)kGXFifoAddress = z;
}
#pragma optimization_level reset

// Transform a point and direction by a borrowed row-major 3x4 matrix, without normalization.
// out0 = M * (v, 1) and out1 = M * (w, 0); coordinate spaces are caller-defined.
// Outputs each hold three floats. Keep them separate from matrix/input storage:
// the point output is written before w is read.
// The original sums each row as (m[2]*z + m[0]*x) + (m[3] + m[1]*y) for the point and m[2]*z + (m[0]*x + m[1]*y)
// for the direction (paired-single code at 0x8042F6B4).
extern const float kProjectiveSkinDirectionW;
extern "C" void _savegpr_27(void);
extern "C" void _restgpr_27(void);
// clang-format off
#ifdef __MWERKS__
asm void Mtx34_TransformPointAndDirection(float* out0, float* out1, const float* v, const float* w, const float* m) {
    nofralloc
    stwu r1, -48(r1)
    mflr r0
    stw r0, 52(r1)
    stfd f31, 32(r1)
    psq_st f31, 40(r1), 0, 0
    addi r11, r1, 32
    bl _savegpr_27
    mr r31, r7
    mr r29, r5
    psq_l f0, 0(r29), 0, 0
    mr r27, r3
    psq_l f2, 0(r31), 0, 0
    mr r30, r6
    psq_l f8, 16(r31), 0, 0
    mr r28, r4
    ps_mul f4, f2, f0
    psq_l f1, 8(r29), 1, 0
    psq_l f3, 8(r31), 0, 0
    ps_mul f10, f8, f0
    psq_l f9, 24(r31), 0, 0
    ps_madd f5, f3, f1, f4
    ps_madd f11, f9, f1, f10
    lfs f31, kProjectiveSkinDirectionW
    ps_sum0 f6, f5, f6, f5
    ps_sum0 f12, f11, f12, f11
    psq_st f6, 0(r27), 1, 0
    psq_l f2, 32(r31), 0, 0
    psq_l f3, 40(r31), 0, 0
    ps_mul f4, f2, f0
    psq_st f12, 4(r27), 1, 0
    ps_madd f5, f3, f1, f4
    psq_l f0, 0(r31), 0, 0
    ps_sum0 f6, f5, f6, f5
    psq_st f6, 8(r27), 1, 0
    psq_l f6, 0(r30), 0, 0
    psq_l f2, 16(r31), 0, 0
    ps_mul f8, f0, f6
    psq_l f4, 32(r31), 0, 0
    ps_mul f10, f2, f6
    psq_l f7, 8(r30), 1, 0
    ps_mul f12, f4, f6
    psq_l f1, 8(r31), 0, 0
    ps_sum0 f8, f8, f8, f8
    psq_l f3, 24(r31), 0, 0
    ps_sum0 f10, f10, f10, f10
    psq_l f5, 40(r31), 0, 0
    ps_sum0 f12, f12, f12, f12
    ps_madd f9, f1, f7, f8
    ps_madd f11, f3, f7, f10
    ps_madd f13, f5, f7, f12
    psq_st f9, 0(r28), 1, 0
    psq_st f11, 4(r28), 1, 0
    psq_st f13, 8(r28), 1, 0
    psq_l f31, 40(r1), 0, 0
    lfd f31, 32(r1)
    addi r11, r1, 32
    bl _restgpr_27
    lwz r0, 52(r1)
    mtlr r0
    addi r1, r1, 48
    blr
}
#else
// clang-format on
void Mtx34_TransformPointAndDirection(float* out0, float* out1, const float* v, const float* w, const float* m) {
    float x = v[0], y = v[1], z = v[2];
    for (int i = 0; i < 3; ++i) {
        const float* r = m + i * 4;
        float a = r[0] * x;
        float b = r[1] * y;
        out0[i] = (r[2] * z + a) + (r[3] + b);
    }
    // Directions use only the linear 3x3 part; translation affects the point above.
    x = w[0];
    y = w[1];
    z = w[2];
    for (int i = 0; i < 3; ++i) {
        const float* r = m + i * 4;
        float a = r[0] * x;
        float b = r[1] * y;
        out1[i] = r[2] * z + (a + b);
    }
}
#endif

// Configure four GX TEV stages, using raster color/alpha in the last stage when requested.
// arg is tested only for nonzero. For each stage, GX computes D + (1-C)*A + C*B
// (or D - that blend), followed by scale/clamp. Tn denotes that stage's texture:
//   0: RGB=reg0+T0*K0; alpha=reg0.a-T0.a*K0.a (unclamped).
//   1: RGB=2*(prev+T1*K1); alpha=prev.a-T1.a*K1.a (unclamped).
//   2: RGB=clamp(prev+T2); alpha=clamp(prev.a+T2.a).
//   3: RGB=clamp(rasterOrZero+lerp(prev.a,prev.rgb,K2.rgb)); alpha=rasterOrZero.a.
// This installs combiner inputs/registers, not texture bindings or stage count.
void Render_ConfigureFourStageTevCombiner(void* volatile arg) {
    GXSetTevColorIn(0, kColorZero, kColorTexture, kColorKonst, kColorRegister0);
    GXSetTevColorOp(0, kTevAdd, kTevBiasZero, kTevScaleOne, false, kTevPrevious);
    GXSetTevAlphaIn(0, kAlphaZero, kAlphaTexture, kAlphaKonst, kAlphaRegister0);
    GXSetTevAlphaOp(0, kTevSubtract, kTevBiasZero, kTevScaleOne, false, kTevPrevious);
    GXSetTevKColorSel(0, kKonstColor0Rgb);
    GXSetTevKAlphaSel(0, kKonstColor0Alpha);
    GXSetTevColorIn(1, kColorZero, kColorTexture, kColorKonst, kColorPrevious);
    GXSetTevColorOp(1, kTevAdd, kTevBiasZero, kTevScaleTwo, false, kTevPrevious);
    GXSetTevAlphaIn(1, kAlphaZero, kAlphaTexture, kAlphaKonst, kAlphaPrevious);
    GXSetTevAlphaOp(1, kTevSubtract, kTevBiasZero, kTevScaleOne, false, kTevPrevious);
    GXSetTevKColorSel(1, kKonstColor1Rgb);
    GXSetTevKAlphaSel(1, kKonstColor1Alpha);
    GXSetTevColorIn(2, kColorZero, kColorTexture, kColorOne, kColorPrevious);
    GXSetTevColorOp(2, kTevAdd, kTevBiasZero, kTevScaleOne, true, kTevPrevious);
    GXSetTevAlphaIn(2, kAlphaTexture, kAlphaZero, kAlphaZero, kAlphaPrevious);
    GXSetTevAlphaOp(2, kTevAdd, kTevBiasZero, kTevScaleOne, true, kTevPrevious);
    GXTevAlphaArg finalAlpha;
    GXTevColorArg rasterAddition;
    rasterAddition = kColorZero;
    finalAlpha = kAlphaZero;
    if (arg) {
        rasterAddition = kColorRaster;
        finalAlpha = kAlphaRaster;
    }
    GXSetTevColorIn(3, kColorPreviousAlpha, kColorPrevious, kColorKonst, rasterAddition);
    GXSetTevColorOp(3, kTevAdd, kTevBiasZero, kTevScaleOne, true, kTevPrevious);
    GXSetTevAlphaIn(3, kAlphaZero, kAlphaZero, kAlphaZero, finalAlpha);
    GXSetTevAlphaOp(3, kTevAdd, kTevBiasZero, kTevScaleOne, true, kTevPrevious);
    GXSetTevKColorSel(3, kKonstColor2Rgb);
    // Publish the signed TEV register and three konst colors after configuring stages.
    volatile unsigned short rgba0[4] = {kTevInitialRegister0Red, kTevInitialRegister0Green, kTevInitialRegister0Blue,
                                        kTevInitialRegister0Alpha};
    volatile unsigned short rgba1[4] = {rgba0[0], rgba0[1], rgba0[2], rgba0[3]};
    unsigned short rgba[4] = {rgba1[0], rgba1[1], rgba1[2], rgba1[3]};
    GXSetTevColorS10(1, rgba);
    unsigned char color0[4] = {kTevKonstColor0Red, kTevKColor0Green, kTevKColor0Blue, kTevKonstColor0Alpha};
    volatile unsigned char color1[4] = {kTevKonstColor0Red, kTevKColor0Green, kTevKColor0Blue, kTevKonstColor0Alpha};
    volatile unsigned char color2[4] = {kTevKonstColor0Red, kTevKColor0Green, kTevKColor0Blue, kTevKonstColor0Alpha};
    GXSetTevKColor(0, color0);
    unsigned char color3[4] = {kTevKonstColor1Red, kTevKColor1Green, kByte_0_EngineMaterialBlue, kRenderColorAlpha182};
    volatile unsigned char color4[4] = {kTevKonstColor1Red, kTevKColor1Green, kByte_0_EngineMaterialBlue,
                                        kRenderColorAlpha182};
    volatile unsigned char color5[4] = {kTevKonstColor1Red, kTevKColor1Green, kByte_0_EngineMaterialBlue,
                                        kRenderColorAlpha182};
    GXSetTevKColor(1, color3);
    unsigned char color6[4] = {kTevKonstColor2Red, kTevKColor2Green, kTevKColor2Blue, kTevKonstColor2Alpha};
    volatile unsigned char color7[4] = {kTevKonstColor2Red, kTevKColor2Green, kTevKColor2Blue, kTevKonstColor2Alpha};
    volatile unsigned char color8[4] = {kTevKonstColor2Red, kTevKColor2Green, kTevKColor2Blue, kTevKonstColor2Alpha};
    GXSetTevKColor(2, color6);
}

#pragma use_lmw_stmw on
#pragma optimization_level 4
#pragma global_optimizer off
#pragma use_frame_pointer off
#pragma optimize_for_size on
// Translate one cached alpha-stage operation into GX TEV inputs and optional preparation stages.
// index is a valid cached stage index (0..7). Supported operations are 0, 2,
// and 16..20; other operations/argument codes leave inputs uninitialized.
// GX alpha equation: clamp(D + (1-C)*A + C*B), with unit scale and zero bias.
// Operation 19 blends second -> first using material alpha; operation 20 adds
// blendArgument + first*second (its name must not be read as a conventional lerp).
// Valid-input contract: index addresses an existing stage and each cached argument
// belongs to the supported set below. Unsupported operations/arguments leave some
// local inputs uninitialized; the final GX call still occurs. There is no fallback.
// Equation guide (P=previous, T=texture or prepared reg2, K=selected konst alpha):
//   selectFirst/Second: selected*K; multiply: first*second; add: first+second;
//   blendMaterial: second*(1-material.a)+first*material.a;
//   pass: index==0 ? raster : P; multiplyAddStage: addend+first*second.
// Only multiplyAddStage reads raw texture alpha even when reg2 was prepared.
// All final equations clamp to [0,1]. Selection operations retain the active K selector.
void Engine_ConfigureTextureStageAlphaCombiner(unsigned char* obj, int index) {
    int alphaB;
    if (!TextureStage_HasActiveEntryAtOrBeyondLimit(obj, index))
        return;

    volatile int baseStage;
    volatile int tevStage;
    volatile unsigned int hasPreparedAlpha;
    volatile int alphaA;
    volatile int alphaC;
    volatile int alphaD;
    volatile int operation;
    volatile int bias;
    volatile int scale;
    volatile int outputRegister;
    unsigned char materialColor[4];
    unsigned char stageColor[4];
    volatile unsigned char clamp;
    // When an unsupported operation/argument skips the input assignments, the original still programs GX from the
    // stale stack words it holds.
    int firstStage = TextureStage_GetAlphaCombinerBaseIndex(obj, index);
    baseStage = firstStage;
    hasPreparedAlpha = 0;
    // Some formats reserve two stages and place prepared texture alpha in reg2.
    // tevStage then names the stage that applies the requested cached operation.
    if (TextureStage_UsesSplitIndexedPalette(obj, index, tevStage = firstStage)) {
        volatile int s1;
        volatile int s0;
        int a1 = tevStage + 1;
        int a0 = tevStage;
        s0 = a0;
        s1 = a1;
        GXSetTevKAlphaSel(a0, kKonstAlphaOne);
        GXSetTevAlphaIn(s0, kAlphaZero, kAlphaKonst, kAlphaTexture, kAlphaZero);
        GXSetTevAlphaOp(s0, kTevAdd, kTevBiasZero, kTevScaleOne, 1, kTevRegister2);
        GXSetTevAlphaIn(s1, kAlphaZero, kAlphaZero, kAlphaZero, kAlphaRegister2);
        GXSetTevAlphaOp(s1, kTevAdd, kTevBiasZero, kTevScaleOne, 1, kTevRegister2);
        int a9 = baseStage;
        hasPreparedAlpha = 1;
        tevStage = a9 + 2;
    } else if (TextureStage_UsesPairedTextureFormat(obj, index)) {
        volatile int s1;
        volatile int s0;
        int a1 = tevStage + 1;
        int a0 = tevStage;
        s0 = a0;
        s1 = a1;
        GXSetTevAlphaIn(a0, kAlphaZero, kAlphaZero, kAlphaZero, kAlphaZero);
        GXSetTevAlphaOp(s0, kTevAdd, kTevBiasZero, kTevScaleOne, 1, kTevRegister0);
        GXSetTevAlphaIn(s1, kAlphaZero, kAlphaZero, kAlphaZero, kAlphaTexture);
        GXSetTevAlphaOp(s1, kTevAdd, kTevBiasZero, kTevScaleOne, 1, kTevRegister2);
        int a9 = baseStage;
        hasPreparedAlpha = 1;
        tevStage = a9 + 2;
    }

    // Select the four inputs; preparation above changes texture selection only
    // in the cases that explicitly consult hasPreparedAlpha.
    switch (((AlphaCombinerState*)obj)->stages[index].operation) {
    case kAlphaSelectFirst:
        // A=0, B=first, C=K, D=0: first*K. Texture input may use prepared reg2.
        switch (((AlphaCombinerState*)obj)->stages[index].firstArgument) {
        case kCachedTexture:
            if (hasPreparedAlpha)
                alphaB = kAlphaRegister2;
            else
                alphaB = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaB = kAlphaPrevious;
            break;
        }
        alphaA = kAlphaZero;
        alphaD = kAlphaZero;
        alphaC = kAlphaKonst;
        operation = kTevAdd;
        bias = kTevBiasZero;
        scale = kTevScaleOne;
        clamp = 1;
        outputRegister = kTevPrevious;
        break;
    case kAlphaSelectSecond:
        // A=0, B=second, C=K, D=0: second*K. Same preparation rule as select-first.
        switch (((AlphaCombinerState*)obj)->stages[index].secondArgument) {
        case kCachedTexture:
            if (hasPreparedAlpha) {
                alphaB = kAlphaRegister2;
                break;
            }
            alphaB = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaB = kAlphaPrevious;
            break;
        }
        alphaA = kAlphaZero;
        alphaD = kAlphaZero;
        alphaC = kAlphaKonst;
        operation = kTevAdd;
        bias = kTevBiasZero;
        scale = kTevScaleOne;
        clamp = 1;
        outputRegister = kTevPrevious;
        break;
    case kAlphaMultiply:
        // A=D=0: B*C, resolving either texture argument through prepared reg2.
        switch (((AlphaCombinerState*)obj)->stages[index].firstArgument) {
        case kCachedTexture:
            if (hasPreparedAlpha)
                alphaB = kAlphaRegister2;
            else
                alphaB = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaB = kAlphaPrevious;
            break;
        }
        switch (((AlphaCombinerState*)obj)->stages[index].secondArgument) {
        case kCachedTexture:
            if (hasPreparedAlpha)
                alphaC = kAlphaRegister2;
            else
                alphaC = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaC = kAlphaPrevious;
            break;
        }
        alphaA = kAlphaZero;
        alphaD = kAlphaZero;
        operation = kTevAdd;
        bias = kTevBiasZero;
        scale = kTevScaleOne;
        clamp = 1;
        outputRegister = kTevPrevious;
        break;
    case kAlphaAdd:
        // B=C=0: D+A. Both texture arguments may resolve through prepared reg2.
        switch (((AlphaCombinerState*)obj)->stages[index].firstArgument) {
        case kCachedTexture:
            if (hasPreparedAlpha)
                alphaD = kAlphaRegister2;
            else
                alphaD = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaD = kAlphaPrevious;
            break;
        }
        switch (((AlphaCombinerState*)obj)->stages[index].secondArgument) {
        case kCachedTexture:
            if (hasPreparedAlpha)
                alphaA = kAlphaRegister2;
            else
                alphaA = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaA = kAlphaPrevious;
            break;
        }
        alphaC = kAlphaZero;
        alphaB = kAlphaZero;
        operation = kTevAdd;
        bias = kTevBiasZero;
        scale = kTevScaleOne;
        clamp = 1;
        outputRegister = kTevPrevious;
        break;
    case kAlphaBlendMaterial: {
        // A=second, B=first, C=material.a, D=0. Upload material color to konst 0.
        switch (((AlphaCombinerState*)obj)->stages[index].firstArgument) {
        case kCachedTexture:
            if (hasPreparedAlpha)
                alphaB = kAlphaRegister2;
            else
                alphaB = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaB = kAlphaPrevious;
            break;
        }
        switch (((AlphaCombinerState*)obj)->stages[index].secondArgument) {
        case kCachedTexture:
            if (hasPreparedAlpha)
                alphaA = kAlphaRegister2;
            else
                alphaA = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaA = kAlphaPrevious;
            break;
        }
        GXSetTevKAlphaSel(tevStage, kKonstColor0Alpha);
        unsigned char* volatile buf = materialColor;
        materialColor[0] = ((AlphaCombinerState*)obj)->constantColors[0][0];
        materialColor[1] = ((AlphaCombinerState*)obj)->constantColors[0][1];
        materialColor[2] = ((AlphaCombinerState*)obj)->constantColors[0][2];
        materialColor[3] = ((AlphaCombinerState*)obj)->constantColors[0][3];
        GXSetTevKColor(0, materialColor);
        alphaC = kAlphaKonst;
        alphaD = kAlphaZero;
        operation = kTevAdd;
        bias = kTevBiasZero;
        scale = kTevScaleOne;
        clamp = 1;
        outputRegister = kTevPrevious;
        break;
    }
    case kAlphaPassRasterOrPrevious:
        // A=B=C=0: D passes raster alpha for the first cached stage, otherwise previous.
        alphaA = kAlphaZero;
        alphaB = kAlphaZero;
        alphaC = kAlphaZero;
        if (index == 0)
            alphaD = kAlphaRaster;
        else
            alphaD = kAlphaPrevious;
        operation = kTevAdd;
        bias = kTevBiasZero;
        scale = kTevScaleOne;
        clamp = 1;
        outputRegister = kTevPrevious;
        break;
    case kAlphaMultiplyAddStage: {
        // A=0: D+B*C. These switches intentionally bypass hasPreparedAlpha.
        switch (((AlphaCombinerState*)obj)->stages[index].addendArgument) {
        case kCachedTexture:
            alphaD = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaD = kAlphaPrevious;
            break;
        }
        switch (((AlphaCombinerState*)obj)->stages[index].firstArgument) {
        case kCachedTexture:
            alphaB = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaB = kAlphaPrevious;
            break;
        }
        switch (((AlphaCombinerState*)obj)->stages[index].secondArgument) {
        case kCachedTexture:
            alphaC = kAlphaTexture;
            break;
        case kCachedPrevious0:
        case kCachedPrevious1:
            alphaC = kAlphaPrevious;
            break;
        case kCachedStageConstant: {
            GXSetTevKAlphaSel(tevStage, kKonstColor0Alpha);
            unsigned char* volatile buf = stageColor;
            // This split address equals constantColors[index]: (index+25)*4 + 0x78 = 0xDC+index*4.
            // Retain the split expression because this function deliberately compiles at optimization level 0.
            unsigned char* src = obj + (index + 25) * 4;
            stageColor[0] = src[0x78];
            stageColor[1] = src[0x79];
            stageColor[2] = src[0x7a];
            stageColor[3] = src[0x7b];
            GXSetTevKColor(0, stageColor);
            alphaC = kAlphaKonst;
            break;
        }
        }
        alphaA = kAlphaZero;
        operation = kTevAdd;
        bias = kTevBiasZero;
        scale = kTevScaleOne;
        clamp = 1;
        outputRegister = kTevPrevious;
        break;
    }
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    default:
        break;
    }
    // Commit exactly one final operation after any format preparation stages.
    GXSetTevAlphaIn(tevStage, alphaA, alphaB, alphaC, alphaD);
    GXSetTevAlphaOp(tevStage, operation, bias, scale, clamp, outputRegister);
}
#pragma optimization_level reset
#pragma global_optimizer reset

#pragma push
#pragma optimization_level 0
// Transform a point with perspective division and its normal without translation; keep paired-single rounding.
// m supplies 16 floats, v/w supply XYZ, and out0/out1 receive XYZ. The point
// divides by the fourth transformed component (which must be nonzero); the
// direction uses kProjectiveSkinDirectionW and is neither divided nor normalized.
// This applies the same linear matrix to the direction, not an inverse transpose.
// Spaces are caller-defined. The existing paired-single implementation is kept intact.
// C arithmetic specification, using column-major m[column*4 + row]:
//   component(i, x,y,z,h) = fma(z,m[8+i], x*m[i])
//                         + fma(h,m[12+i], y*m[4+i]);  i=0..3.
// Each multiply, fused multiply-add, and final lane sum rounds separately to float.
//   pointW = component(3,v.x,v.y,v.z,1);
//   reciprocalW = 1.0f / pointW;
//   out0[i] = component(i,v.x,v.y,v.z,1) * reciprocalW; i=0..2.
//   out1[i] = component(i,w.x,w.y,w.z,kProjectiveSkinDirectionW); i=0..2.
// Ordinary C reassociation can change fused rounding and the compiled result.
// Lane map after matrix loads (pairs separated by commas):
//   c0=(m0,m4), c1=(m8,m12) -> X; c2=(m1,m5), c3=(m9,m13) -> Y;
//   c4=(m2,m6), c5=(m10,m14) -> Z; c6=(m3,m7), c7=(m11,m15) -> W.
// a holds (x,y), b holds (z,homogeneousW). Point writes finish before direction
// loads; overlapping point output with direction input changes the result.
void Skin_TransformPositionAndNormalProjective(float* out0, float* out1, const float* v, const float* w,
                                               const float* m) {
    extern const float kProjectiveSkinDirectionW;
    register const float* matrix = m;
    register const float* point = v;
    register const float* direction = w;
    register float* point_out = out0;
    register float* direction_out = out1;
    register float zero = kProjectiveSkinDirectionW;
    register __vec2x32float__ a, b, c0, c1, c2, c3, c4, c5, c6, c7;
    register __vec2x32float__ p, q, sum, result;
    // Preserve the two independently rounded lane sums and the fused products.
    asm {    // Phase 1: collect matrix coefficients into the X/Y/Z/W lane pairs above.
        psq_l a, 0(matrix), 0, 0
        psq_l b, 16(matrix), 0, 0
        psq_l p, 8(matrix), 0, 0
        psq_l q, 24(matrix), 0, 0
        ps_merge00 c0, a, b
        ps_merge11 c2, a, b
        psq_l a, 32(matrix), 0, 0
        psq_l b, 48(matrix), 0, 0
        ps_merge00 c4, p, q
        ps_merge11 c6, p, q
        ps_merge00 c1, a, b
        ps_merge11 c3, a, b
        psq_l p, 40(matrix), 0, 0
        psq_l q, 56(matrix), 0, 0
        psq_l a, 0(point), 0, 0
        ps_merge00 c5, p, q
        psq_l b, 8(point), 1, 0
        ps_merge11 c7, p, q
             // Phase 2: accumulate the point with homogeneous W=1.
        ps_mul p, a, c4
        ps_madd q, b, c5, p
        ps_mul c5, a, c0
        ps_mul p, a, c6
        ps_sum0 sum, q, q, q
        ps_madd c5, b, c1, c5
        ps_madd q, b, c7, p
        ps_merge11 p, b, b
        ps_sum0 c7, c5, c5, c5
        ps_sum1 result, q, sum, q
        ps_mul c5, a, c2
        ps_sum0 q, result, result, result
        ps_madd c5, b, c3, c5
        ps_merge11 q, q, q
        ps_sum1 c7, c5, c7, c5
        ps_div q, p, q
        ps_mul result, result, q
        ps_mul c7, c7, q
        psq_st result, 8(point_out), 1, 0
        psq_st c7, 0(point_out), 0, 0
             // Phase 3: substitute the direction's homogeneous component; no perspective divide.
        psq_l b, 8(direction), 1, 0
        psq_l p, 40(matrix), 0, 0
        psq_l q, 56(matrix), 0, 0
        ps_merge00 b, b, zero
        psq_l a, 0(direction), 0, 0
        ps_merge00 c5, p, q
        ps_merge11 c7, p, q
        ps_mul p, a, c0
        ps_mul c0, a, c2
        ps_madd q, b, c1, p
        ps_madd c1, b, c3, c0
        ps_mul p, a, c4
        ps_sum0 sum, q, q, q
        ps_mul c0, a, c6
        ps_madd q, b, c5, p
        ps_sum1 result, c1, sum, c1
        ps_madd c1, b, c7, c0
        ps_sum0 c2, q, q, q
        psq_st result, 0(direction_out), 0, 0
        ps_sum1 result, c1, c2, c1
        psq_st result, 8(direction_out), 1, 0
    }
}
#pragma pop
#pragma dont_inline on
#pragma optimization_level 0
// Emit each input index six times for six indexed vertex attributes.
void Render_WriteFifoU16Indices6(volatile unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    unsigned int i = 0;
    while (i < limit) {
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        ++p;
        ++i;
    }
}
#pragma optimization_level reset

#pragma optimization_level 0
#pragma use_frame_pointer on
// Emit XYZ, packed RGBA, and two UV pairs from eight-word vertices.
void Render_WriteVerticesPositionColorTex2(float* values, volatile unsigned int count) {
    float* p = values;
    unsigned int i = 0;
    while (i < count) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        unsigned int tmp = *(unsigned int*)(p + 3);
        *(volatile unsigned int*)kGXFifoAddress = tmp;
        Render_WriteTexCoord2f(p[4], p[5]);
        Render_WriteTexCoord2f(p[6], p[7]);
        p += 8;
        ++i;
    }
}
#pragma use_frame_pointer reset
#pragma optimization_level reset
#pragma dont_inline off

#pragma dont_inline on
#pragma optimization_level 0
struct VertexUVs {
    float xyz[3];
    struct Pair {
        float x, y;
    } uv[3];
};
// Emit XYZ and two UV pairs from seven-float vertices.
void Render_WriteVerticesPositionTex2(float* values, unsigned int count) {
    float* p = values;
    unsigned int i = 0;
    while (i < count) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        for (int j = 0; j < 2; ++j)
            Render_WriteTexCoord2f(((VertexUVs*)p)->uv[j].x, ((VertexUVs*)p)->uv[j].y);
        p += 7;
        ++i;
    }
}
#pragma optimization_level reset
#pragma dont_inline off

#pragma dont_inline on
#pragma optimization_level 0
// Emit packed XYZ positions from three-float vertices.
void Render_WritePositions3f(float* values, volatile unsigned int count) {
    float* p = values;
    unsigned int i = 0;
    while (i < count) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        p += 3;
        ++i;
    }
}
#pragma optimization_level reset
#pragma dont_inline off

#pragma optimization_level 0
// Emit four copies of each index, retaining the volatile counter update order.
void Render_WriteFifoU16Indices4(unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    volatile unsigned int i = 0;
    while (i < limit) {
        *(volatile unsigned short*)kGXFifoAddress = *p;
        ++i;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p++;
    }
}
#pragma optimization_level reset

#pragma optimization_level 0
#pragma optimization_level 0
#pragma use_frame_pointer on
// Emit three copies of each index through the alternate entry point.
void Render_WriteFifoU16Indices3_Alternate(unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    unsigned short* p = values;
    unsigned int i = 0;
    while (i < limit) {
        *(volatile unsigned short*)kGXFifoAddress = *p;
        ++i;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p++;
    }
}
#pragma use_frame_pointer reset
#pragma optimization_level reset

#pragma optimization_level 0
#pragma use_frame_pointer on
// Emit XYZ and packed RGBA from four-word vertices.
void Render_WriteVerticesPositionColor(float* values, volatile unsigned int count) {
    float* p = values;
    unsigned int i = 0;
    while (i < count) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        unsigned int tmp = *(unsigned int*)(p + 3);
        *(volatile unsigned int*)kGXFifoAddress = tmp;
        p += 4;
        ++i;
    }
}
#pragma use_frame_pointer reset
#pragma optimization_level reset

#pragma optimization_level 0
// Emit four copies of each index through the alternate entry point.
void Render_WriteFifoU16Indices4_Alternate(unsigned short* values, unsigned int count) {
    volatile unsigned int limit = count;
    volatile unsigned short* p = values;
    unsigned int i = 0;
    while (i < limit) {
        *(volatile unsigned short*)kGXFifoAddress = *p;
        ++i;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p;
        *(volatile unsigned short*)kGXFifoAddress = *p++;
    }
}
#pragma optimization_level reset

#pragma dont_inline on
#pragma optimization_level 0
// Emit XYZ and three UV pairs from nine-float vertices.
void Render_WriteVerticesPositionTex3(float* values, unsigned int count) {
    float* p = values;
    unsigned int i = 0;
    while (i < count) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        for (int j = 0; j < 3; ++j)
            Render_WriteTexCoord2f(((VertexUVs*)p)->uv[j].x, ((VertexUVs*)p)->uv[j].y);
        p += 9;
        ++i;
    }
}
#pragma optimization_level reset

#pragma global_optimizer off
#pragma optimization_level 0
#pragma use_frame_pointer on
#pragma use_lmw_stmw on
// Emit XYZ, packed RGBA, and three UV pairs from ten-word vertices.
void Render_WriteVerticesPositionColorTex3(float* values, unsigned int count) {
    volatile unsigned int limit = count;
    float* p = values;
    unsigned int i = 0;
    while (i < limit) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        *(volatile unsigned int*)kGXFifoAddress = *(unsigned int*)(p + 3);
        Render_WriteTexCoord2f(p[4], p[5]);
        Render_WriteTexCoord2f(p[6], p[7]);
        Render_WriteTexCoord2f(p[8], p[9]);
        p += 10;
        ++i;
    }
}
#pragma global_optimizer reset
#pragma optimization_level reset
#pragma use_frame_pointer reset
#pragma use_lmw_stmw reset

#pragma optimization_level 0
#pragma use_frame_pointer on
// Emit XYZ, packed RGBA, and four UV pairs from twelve-word vertices.
void Render_WriteVerticesPositionColorTex4(float* values, volatile unsigned int count) {
    float* p = values;
    unsigned int i = 0;
    while (i < count) {
        Render_WriteFifo3f(p[0], p[1], p[2]);
        unsigned int tmp = (*(volatile unsigned int*)kGXFifoAddress = *(unsigned int*)(p + 3));
        Render_WriteTexCoord2f(p[4], p[5]);
        Render_WriteTexCoord2f(p[6], p[7]);
        Render_WriteTexCoord2f(p[8], p[9]);
        Render_WriteTexCoord2f(p[10], p[11]);
        p += 12;
        ++i;
    }
}
#pragma use_frame_pointer reset
#pragma optimization_level reset
#pragma dont_inline off

} // extern "C"
