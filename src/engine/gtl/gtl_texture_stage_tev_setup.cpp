// Color operation identifiers describe the equations emitted below.
static const int kColorMultiply = 0;
static const int kColorAdd = 2;
static const int kColorKeepPrevious = 16;
static const int kColorSelectArgument1 = 17;
static const int kColorSelectArgument2 = 18;
static const int kColorBlendConstant = 19;
static const int kColorBlendTextureAlpha = 21;
static const int kColorInterpolate = 22;
static const int kArgumentPrevious0 = 0;
static const int kArgumentPrevious1 = 1;
static const int kArgumentTexture = 2;
static const int kArgumentConstant = 3;
static const int kArgumentTextureAlpha = 0x22;
// Per-stage constant (konst) RGBA colors are read at self + (stage + 25) * 4 + 0x78, i.e. one 4-byte
// RGBA entry per stage starting at byte 0xDC of TextureStageState (the 25 and 0x78 are kept split because
// the compiled address arithmetic depends on it).
static const int kKonstColorBaseWord = 25;
static const int kKonstColorByteOffset = 0x78;
// PREVIOUS0/PREVIOUS1 (0 and 1) are both treated as the running TEV result (GX_CC_CPREV) by every switch below;
// the game's distinction between them is not established.

#include "revolution/GX/GXTypes.h"

// Texture/palette handles and fixed-size stage records share this state block.
struct TextureStageOperation {
    int colorOperation;
    int colorArgument1;
    int colorArgument2;
    char reserved_0C[0x2C];
    int colorArgument0;
    char reserved_3C[0x28];
};
struct TextureStageState {
    void* textures[8];
    void* palettes[8];
    char reserved_040[0xB4];
    int extraLightingStage;
    char reserved_0F8[0x28];
    TextureStageOperation stages[8];
};
// Declaration ABI: each 12-byte record names a stream and byte offset, a storage
// type (not a byte count), then method/usage/index bytes. An explicit sentinel
// ends the array; element count excludes it. No implicit count or bounds check.
struct VertexElement {
    unsigned short stream;
    unsigned short offset;
    unsigned int type;
    unsigned char method;
    unsigned char usage;
    unsigned char usageIndex;
    unsigned char reserved;
};
static const int kVertexFloat1 = 0;
static const int kVertexFloat2 = 1;
static const int kVertexFloat3 = 2;
static const int kVertexFloat4 = 3;
static const int kVertexPackedColor = 4;
// Type 5 occupies four bytes in both size consumers. BuildDeclaration's
// 0x800000 branch nevertheless advances by ONE byte while emitting this type.
// Keep the neutral encoding name: the one-byte branch is not a byte4 layout.
static const int kVertexStorageType5 = 5;
static const int kVertexShort2 = 6;
static const int kVertexShort4 = 7;
static const int kVertexSpecialByte = 0xFE;
static const int kVertexUnused = 0x11;
// Declaration usage IDs follow the position/weights/indices/normal/UV/color
// categories; they are unrelated to GX attribute numbers.
enum VertexUsage {
    kUsagePosition = 0,
    kUsageBlendWeights = 1,
    kUsageBlendIndices = 2,
    kUsageNormal = 3,
    kUsageTexCoord = 5,
    kUsageColor = 10,
    kUsageEnd = 0xFF
};
enum VertexFormatBits {
    kFormatPosition = 0x1,
    kFormatPositionVariant5 = 0x5,
    kFormatNormal = 0x400,
    kFormatPackedColor = 0xF000,
    kFormatTexCoordVariant1 = 0x10000,
    kFormatTexCoordVariant2 = 0x20000,
    kFormatFourByteIndices = 0x400000,
    kFormatOneByteAdvanceType5 = 0x800000
};
static const unsigned int kTextureStageCount = 8;
static const int kVertexConversionRuleCount = 0x41;
static const unsigned int kVertexTypeUnused = 0xFFFFFFFFu;
static const unsigned int kTextureFormatPairBias = 0x40000000u;
static const unsigned int kTextureFormatPairFirst = 0x30;
static const unsigned int kTextureFormatPairSecond = 0x32;
static const unsigned int kPlatformFormatBias = 0x80000000u;
static const unsigned int kTextureFormatIndexed = 9;
static const unsigned int kPaletteFormatSplit = 6;

extern "C" {

extern const unsigned int kVertexFormatCategoryMasks[5];
extern const unsigned int kVertexElementTerminatorComparison[4];
extern const unsigned int kVertexDeclarationEnd[3];
extern const unsigned int kVertexDeclarationConversionRules[130];

int TexturePalette_GetDescription(void*, unsigned int*);
int TextureStage_ClassifyTextureSlot(void*, unsigned int);
int VertexDeclaration_GetElementCount(void*);
int TextureFormat_IsPaletted(unsigned int);
void TextureResource_GetLevelDescription(void*, void*, unsigned int*);
int TexturePalette_ClassifySlot(void*, unsigned int);
int TextureStage_UsesSplitIndexedPalette(void*, unsigned int);
int GXSetTevSwapMode(unsigned int, unsigned int, unsigned int);
void GXSetTevColor(unsigned int, const unsigned char*);
int GXSetTevKColorSel(unsigned int, unsigned int);
void GXSetTevKColor(unsigned int, const unsigned char*);
void GXSetTevColorIn(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
void GXSetTevColorOp(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern const unsigned char kTevSplitColor0Red;
extern const unsigned char kTevSplitColor0Green;
extern const unsigned char kTevSplitColor0Blue;
extern const unsigned char kTevSplitColor0Alpha;
extern const unsigned char kTevSplitColor1Red;
extern const unsigned char kTevSplitColor1Green;
extern const unsigned char kTevSplitColor1Blue;
extern const unsigned char kTevSplitColor1Alpha;
void Texture_GetLevelDescription(void*, void*, void*);
void Texture_GetKind4LevelDescription(void*, void*, void*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);

#pragma push
// Optimize the scan without level 4's eight-way unrolling. The input reads
// stay volatile and in index order; the private threshold needs no spill.
#pragma optimization_level 2
#pragma global_optimizer on
// Report whether any active texture slot has an index at least equal to the limit.
int TextureStage_HasActiveEntryAtOrBeyondLimit(volatile unsigned int* arg, unsigned int limit) {
    volatile unsigned int* values = arg;
    unsigned int i = 0;
    for (; i < kTextureStageCount; i++) {
        if (values[i] != 0 && i >= limit) {
            return 1;
        }
    }
    return 0;
}
#pragma pop

#pragma global_optimizer off
#pragma optimization_level 1
#pragma use_frame_pointer on
#pragma use_lmw_stmw on
// Count GX texture maps consumed by preceding slots, stopping at the first empty slot.
int TextureStage_CountTextureMapSlotsBeforeIndex(void* volatile values, volatile unsigned int count) {
    register unsigned int total = 0;
    register unsigned int i = 0;
    for (; i < count;) {
        register int value = TextureStage_ClassifyTextureSlot(values, i);
        if (value == 0)
            break;
        total += value;
        i++;
    }
    return total;
}
#pragma global_optimizer reset
#pragma use_frame_pointer reset
#pragma use_lmw_stmw reset

void TextureResource_GetLevelDescription(void* level, void* description, unsigned int* resource);

#pragma global_optimizer off
#pragma optimization_level 1
#pragma use_lmw_stmw on
// Recognize either texture format that consumes a pair of GX texture maps.
int TextureStage_UsesPairedTextureFormat(unsigned int* values, unsigned int index) {
    unsigned int value = values[index];
    if (value == 0)
        return 0;
    volatile unsigned int result[9];
    TextureResource_GetLevelDescription((void*)0, (void*)result, (unsigned int*)value);
    if (result[0] + kTextureFormatPairBias == kTextureFormatPairFirst ||
        result[0] + kTextureFormatPairBias == kTextureFormatPairSecond)
        return 1;
    return 0;
}
#pragma use_lmw_stmw reset
#pragma optimization_level reset
#pragma global_optimizer reset

#pragma push
#pragma optimization_level 4
#pragma global_optimizer off
#pragma peephole on
#pragma scheduling on
#pragma optimize_for_size on
#pragma use_frame_pointer on
#pragma use_lmw_stmw on
// Count preceding TEV stages, including lighting and split-palette expansion.
int TextureStage_GetAlphaCombinerBaseIndex(void* stageState, volatile unsigned int count) {
    register int total = 0;
    total++;
    if (((TextureStageState*)stageState)->extraLightingStage != 0)
        total++;
    for (unsigned int i = 0; i < count; i++) {
        if (TextureStage_UsesSplitIndexedPalette(stageState, i))
            total += 3;
        else
            total += 1;
    }
    return total;
}
#pragma pop

typedef struct {
    char pad0[0x120];
    int kind;
    int u124;
    int u128;
    char pad12C[0x2C];
    int u158;
} Entry;

#pragma use_lmw_stmw on
#pragma optimization_level 4
#pragma opt_common_subs off
#pragma optimize_for_size on
// Translate one stored color operation into GX TEV inputs, registers and optional preparatory stages.
// index selects one of eight stored operations; callers supply supported argument codes.
// GX ADD evaluates D + A*(1-C) + B*C componentwise, then bias/scale/clamp.
// PREV carries the preceding logical operation. C0/C1 are split-color constants;
// C2 carries reconstructed texture color. Ordinary operations emit at baseStage;
// split/paired preparation consumes baseStage and baseStage+1, then emits at +2.
// This routine configures RGB only; alpha, texture bindings and stage count are set elsewhere.
void TextureStage_ConfigureTevColorCombiner(register unsigned char* self, register int index) {
    // Phases: locate the physical base stage, reconstruct special texture formats,
    // select the operation inputs, then program the final RGB stage.
    int colorInputB;
    int colorInputC;
    volatile int firstStage;
    volatile int currentStage;
    volatile unsigned int hasPreparedColor;
    volatile int colorInputA;
    volatile int colorInputD;
    volatile int operation;
    volatile int bias;
    volatile int scale;
    volatile int outputRegister;
    unsigned char multiplyKonstRgba[4];
    unsigned char modulateKonstRgba[4];
    unsigned char weightKonstRgba[4];
    unsigned char argument2KonstRgba[4];
    unsigned char splitColor1[4];
    unsigned char splitColor0[4];
    unsigned char splitColors[8];
    volatile unsigned char clamp;

    if (!TextureStage_HasActiveEntryAtOrBeyondLimit((volatile unsigned int*)self, index))
        return;

    // When an unsupported operation/argument skips the input assignments, the original still programs GX from the
    // stale stack words it holds.
    int baseStage = TextureStage_GetAlphaCombinerBaseIndex(self, index);
    firstStage = baseStage;
    hasPreparedColor = 0;
    currentStage = baseStage;
    // Recognize an indexed texture with the palette format that needs three TEV stages.
    if (TextureStage_UsesSplitIndexedPalette(self, index)) {
        volatile int secondPreparationStage;
        volatile int firstPreparationStage;
        int secondStageIndex = currentStage + 1;
        int firstStageIndex = currentStage;
        firstPreparationStage = firstStageIndex;
        secondPreparationStage = secondStageIndex;
        GXSetTevSwapMode(secondStageIndex, 0, 1);
        splitColors[0] = kTevSplitColor0Red;
        splitColors[1] = kTevSplitColor0Green;
        splitColors[2] = kTevSplitColor0Blue;
        splitColors[3] = kTevSplitColor0Alpha;
        splitColors[4] = kTevSplitColor1Red;
        splitColors[5] = kTevSplitColor1Green;
        splitColors[6] = kTevSplitColor1Blue;
        splitColors[7] = kTevSplitColor1Alpha;
        unsigned char* volatile buf0 = splitColor0;
        splitColor0[0] = splitColors[0];
        splitColor0[1] = splitColors[1];
        splitColor0[2] = splitColors[2];
        splitColor0[3] = splitColors[3];
        splitColor1[0] = kTevSplitColor1Red;
        splitColor1[1] = kTevSplitColor1Green;
        splitColor1[2] = kTevSplitColor1Blue;
        splitColor1[3] = kTevSplitColor1Alpha;
        GXSetTevColor(1, splitColor0);
        unsigned char* volatile buf1 = splitColor1;
        GXSetTevColor(2, splitColor1);
        // C2 = texture(first map)*C0; next map adds swizzled texture*C1, clamped.
        GXSetTevColorIn(firstPreparationStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
        GXSetTevColorOp(firstPreparationStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0, GX_TEVREG2);
        GXSetTevColorIn(secondPreparationStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C1, GX_CC_C2);
        GXSetTevColorOp(secondPreparationStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVREG2);
        int preparationBaseStage = firstStage;
        hasPreparedColor = 1;
        currentStage = preparationBaseStage + 2;
    } else if (TextureStage_UsesPairedTextureFormat((unsigned int*)self, index)) {
        volatile int secondPreparationStage;
        volatile int firstPreparationStage;
        int secondStageIndex = currentStage + 1;
        int firstStageIndex = currentStage;
        firstPreparationStage = firstStageIndex;
        secondPreparationStage = secondStageIndex;
        // C0 = first map unchanged; second stage copies C0 into clamped C2.
        // The paired texture binding/alpha setup is outside this RGB routine.
        GXSetTevColorIn(firstPreparationStage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
        GXSetTevColorOp(firstPreparationStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0, GX_TEVREG0);
        GXSetTevColorIn(secondPreparationStage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
        GXSetTevColorOp(secondPreparationStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVREG2);
        int preparationBaseStage = firstStage;
        hasPreparedColor = 1;
        currentStage = preparationBaseStage + 2;
    }

    GXSetTevSwapMode(currentStage, 0, 0);

    switch (((TextureStageState*)self)->stages[index].colorOperation) {
    // PREV = argument1.
    case kColorSelectArgument1:
        switch (((TextureStageState*)self)->stages[index].colorArgument1) {
        case kArgumentTexture:
            colorInputB = hasPreparedColor ? GX_CC_C2 : GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputB = GX_CC_CPREV;
            break;
        }
        colorInputA = GX_CC_ZERO;
        colorInputC = GX_CC_ONE;
        colorInputD = GX_CC_ZERO;
        operation = GX_TEV_ADD;
        bias = GX_TB_ZERO;
        scale = GX_CS_SCALE_1;
        clamp = 1;
        outputRegister = GX_TEVPREV;
        break;
    // PREV = argument2.
    case kColorSelectArgument2:
        switch (((TextureStageState*)self)->stages[index].colorArgument2) {
        case kArgumentTexture:
            colorInputB = hasPreparedColor ? GX_CC_C2 : GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputB = GX_CC_CPREV;
            break;
        }
        colorInputA = GX_CC_ZERO;
        colorInputC = GX_CC_ONE;
        colorInputD = GX_CC_ZERO;
        operation = GX_TEV_ADD;
        bias = GX_TB_ZERO;
        scale = GX_CS_SCALE_1;
        clamp = 1;
        outputRegister = GX_TEVPREV;
        break;
    // PREV = argument1 * argument2.
    case kColorMultiply:
        switch (((TextureStageState*)self)->stages[index].colorArgument1) {
        case kArgumentTexture:
            colorInputB = hasPreparedColor ? GX_CC_C2 : GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputB = GX_CC_CPREV;
            break;
        }
        switch (((TextureStageState*)self)->stages[index].colorArgument2) {
        case kArgumentTexture:
            colorInputC = hasPreparedColor ? GX_CC_C2 : GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputC = GX_CC_CPREV;
            break;
        case kArgumentConstant: {
            GXSetTevKColorSel(currentStage, GX_TEV_KCSEL_K0);
            unsigned char* volatile buf = multiplyKonstRgba;
            unsigned char* src = self + (index + kKonstColorBaseWord) * 4;
            multiplyKonstRgba[0] = src[kKonstColorByteOffset + 0];
            multiplyKonstRgba[1] = src[kKonstColorByteOffset + 1];
            multiplyKonstRgba[2] = src[kKonstColorByteOffset + 2];
            multiplyKonstRgba[3] = src[kKonstColorByteOffset + 3];
            GXSetTevKColor(0, multiplyKonstRgba);
            colorInputC = GX_CC_KONST;
            break;
        }
        }
        colorInputA = GX_CC_ZERO;
        colorInputD = GX_CC_ZERO;
        operation = GX_TEV_ADD;
        bias = GX_TB_ZERO;
        scale = GX_CS_SCALE_1;
        clamp = 1;
        outputRegister = GX_TEVPREV;
        break;
    // PREV = argument1 + argument2.
    case kColorAdd:
        switch (((TextureStageState*)self)->stages[index].colorArgument1) {
        case kArgumentTexture:
            colorInputD = hasPreparedColor ? GX_CC_C2 : GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputD = GX_CC_CPREV;
            break;
        }
        switch (((TextureStageState*)self)->stages[index].colorArgument2) {
        case kArgumentTexture:
            colorInputA = hasPreparedColor ? GX_CC_C2 : GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputA = GX_CC_CPREV;
            break;
        case 22:
            break;
        }
        operation = GX_TEV_ADD;
        colorInputB = GX_CC_ZERO;
        colorInputC = GX_CC_ZERO;
        bias = GX_TB_ZERO;
        scale = GX_CS_SCALE_1;
        clamp = 1;
        outputRegister = GX_TEVPREV;
        break;
    // PREV = argument2*(1-K0) + argument1*K0; K0 is RGBA at self+0xDC.
    case kColorBlendConstant:
        switch (((TextureStageState*)self)->stages[index].colorArgument1) {
        case kArgumentTexture:
            colorInputB = hasPreparedColor ? GX_CC_C2 : GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputB = GX_CC_CPREV;
            break;
        }
        switch (((TextureStageState*)self)->stages[index].colorArgument2) {
        case kArgumentTexture:
            colorInputA = hasPreparedColor ? GX_CC_C2 : GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputA = GX_CC_CPREV;
            break;
        }
        GXSetTevKColorSel(currentStage, GX_TEV_KCSEL_K0);
        {
            unsigned char* volatile buf = modulateKonstRgba;
            modulateKonstRgba[0] = self[0xDC];
            modulateKonstRgba[1] = self[0xDD];
            modulateKonstRgba[2] = self[0xDE];
            modulateKonstRgba[3] = self[0xDF];
            GXSetTevKColor(0, modulateKonstRgba);
        }
        colorInputC = GX_CC_KONST;
        colorInputD = GX_CC_ZERO;
        operation = GX_TEV_ADD;
        bias = GX_TB_ZERO;
        scale = GX_CS_SCALE_1;
        clamp = 1;
        outputRegister = GX_TEVPREV;
        break;
    // PREV is unchanged.
    case kColorKeepPrevious:
        colorInputA = GX_CC_ZERO;
        colorInputB = GX_CC_ZERO;
        colorInputC = GX_CC_ZERO;
        colorInputD = GX_CC_CPREV;
        operation = GX_TEV_ADD;
        bias = GX_TB_ZERO;
        scale = GX_CS_SCALE_1;
        clamp = 1;
        outputRegister = GX_TEVPREV;
        break;
    // PREV = argument2*(1-argument0) + argument1*argument0.
    // Texture arguments here read TEXC directly, even after preparation. The constant
    // argument2 branch sets C (the weight) to KONST; it leaves A unassigned.
    // Fully assigned combinations for THIS opcode:
    //   argument0 (weight): texture, previous0/1, constant, texture-alpha;
    //   argument1 (B): texture or previous0/1;
    //   argument2 (A): texture or previous0/1.
    // Do not select constant for argument2: it overwrites the weight and supplies
    // no A. Other unlisted argument values also leave a GX input unassigned.
    case kColorInterpolate:
        switch (((TextureStageState*)self)->stages[index].colorArgument0) {
        case kArgumentTexture:
            colorInputC = GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputC = GX_CC_CPREV;
            break;
        case kArgumentConstant: {
            GXSetTevKColorSel(currentStage, GX_TEV_KCSEL_K0);
            unsigned char* volatile buf = weightKonstRgba;
            unsigned char* src = self + (index + kKonstColorBaseWord) * 4;
            weightKonstRgba[0] = src[kKonstColorByteOffset + 0];
            weightKonstRgba[1] = src[kKonstColorByteOffset + 1];
            weightKonstRgba[2] = src[kKonstColorByteOffset + 2];
            weightKonstRgba[3] = src[kKonstColorByteOffset + 3];
            GXSetTevKColor(0, weightKonstRgba);
            colorInputC = GX_CC_KONST;
            break;
        }
        case kArgumentTextureAlpha:
            colorInputC = GX_CC_TEXA;
            break;
        }
        switch (((TextureStageState*)self)->stages[index].colorArgument1) {
        case kArgumentTexture:
            colorInputB = GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputB = GX_CC_CPREV;
            break;
        }
        switch (((TextureStageState*)self)->stages[index].colorArgument2) {
        case kArgumentTexture:
            colorInputA = GX_CC_TEXC;
            break;
        case kArgumentPrevious0:
        case kArgumentPrevious1:
            colorInputA = GX_CC_CPREV;
            break;
        case kArgumentConstant: {
            // Note: this lerp branch stores the constant into colorInputC (not A); kept as is to match the original build.
            GXSetTevKColorSel(currentStage, GX_TEV_KCSEL_K0);
            unsigned char* volatile buf = argument2KonstRgba;
            unsigned char* src = self + (index + kKonstColorBaseWord) * 4;
            argument2KonstRgba[0] = src[kKonstColorByteOffset + 0];
            argument2KonstRgba[1] = src[kKonstColorByteOffset + 1];
            argument2KonstRgba[2] = src[kKonstColorByteOffset + 2];
            argument2KonstRgba[3] = src[kKonstColorByteOffset + 3];
            GXSetTevKColor(0, argument2KonstRgba);
            colorInputC = GX_CC_KONST;
            break;
        }
        }
        colorInputD = GX_CC_ZERO;
        operation = GX_TEV_ADD;
        bias = GX_TB_ZERO;
        scale = GX_CS_SCALE_1;
        clamp = 1;
        outputRegister = GX_TEVPREV;
        break;
    // PREV = PREV*(1-textureAlpha) + textureRGB*textureAlpha.
    case kColorBlendTextureAlpha:
        colorInputA = GX_CC_CPREV;
        colorInputB = GX_CC_TEXC;
        colorInputC = GX_CC_TEXA;
        colorInputD = GX_CC_ZERO;
        operation = GX_TEV_ADD;
        bias = GX_TB_ZERO;
        scale = GX_CS_SCALE_1;
        clamp = 1;
        outputRegister = GX_TEVPREV;
        break;
    }

    GXSetTevColorIn(currentStage, colorInputA, colorInputB, colorInputC, colorInputD);
    GXSetTevColorOp(currentStage, operation, bias, scale, clamp, outputRegister);
}
#pragma optimization_level reset
#pragma global_optimizer reset
#pragma use_lmw_stmw reset
#pragma use_frame_pointer reset
#pragma optimize_for_size reset

#pragma push
#pragma optimization_level 0
#pragma global_optimizer off
#pragma peephole on
#pragma scheduling on
#pragma optimize_for_size on
#pragma use_lmw_stmw on
// Return the final vertex element offset plus its encoded storage size.
// Requires at least one element followed by a valid sentinel, with the last element
// at the greatest byte offset. Types 0..7 store 4/8/12/16/4/4/4/8 bytes.
// Unsupported types add zero; this computes the final extent, not a sum of sizes.
int VertexDeclaration_GetStride(register void* declaration) {
    register unsigned int lastElementIndex = VertexDeclaration_GetElementCount(declaration) - 1;
    register char* lastElement;
    register unsigned int elementBytes = 0;
    lastElement = (char*)declaration + lastElementIndex * 12;
    switch (((VertexElement*)lastElement)->type) {
    case kVertexFloat1:
        elementBytes = 4;
        break;
    case kVertexFloat2:
        elementBytes = 8;
        break;
    case kVertexFloat3:
        elementBytes = 12;
        break;
    case kVertexFloat4:
        elementBytes = 16;
        break;
    case kVertexPackedColor:
        elementBytes = 4;
        break;
    case kVertexStorageType5:
        elementBytes = 4;
        break;
    case kVertexShort2:
        elementBytes = 4;
        break;
    case kVertexShort4:
        elementBytes = 8;
        break;
    }
    return ((VertexElement*)lastElement)->offset + elementBytes;
}
#pragma pop

typedef VertexElement Key;

// Scan 12-byte vertex elements until the declaration terminator is found.
int VertexDeclaration_GetElementCount(void* volatile entries) {
    unsigned int i = 0;
    unsigned int found = 0;
    for (;;) {
        Key key = *(volatile Key*)kVertexElementTerminatorComparison;
        unsigned char* e = (unsigned char*)entries + i * 12;
        if (*(volatile unsigned short*)e == key.stream && *(volatile unsigned short*)(e + 2) == key.offset &&
            (*(volatile unsigned int*)(e + 4) == key.type || *(volatile unsigned int*)(e + 4) == 0xFFFFFFFFu) &&
            *(volatile unsigned char*)(e + 8) == key.method && *(volatile unsigned char*)(e + 9) == key.usage &&
            *(volatile unsigned char*)(e + 10) == key.usageIndex) {
            found = 1;
        } else {
            found = 0;
        }
        // The rodata constant holds big-endian word values (0x00FF0000, 0x11); model-file declarations are swapped at load
        // and carry the native layout {stream 0xFF, offset 0, type 0x11 or -1}.
        if (found)
            break;
        i++;
    }
    return i;
}

typedef struct {
    unsigned char code;
    unsigned char usageIndex;
    unsigned char type;
    unsigned char pad;
    unsigned int value;
} TblEnt;

#pragma global_optimizer off
#pragma use_frame_pointer on
#pragma use_lmw_stmw on
// Validate a declaration against the conversion table and combine its vertex-format flags.
int VertexDeclaration_TryConvertToFormatFlags(unsigned char* entries, unsigned int* out) {
    // Phases: copy conversion rules, find each contiguous stream-zero element
    // by offset, consume rules in increasing order, then publish aggregate flags.
    // Return 0 on unsupported order/type/gap; out is zeroed before validation.
    volatile unsigned int last;
    volatile unsigned int aggregate;
    volatile unsigned int extra;
    volatile unsigned int total;
    volatile unsigned int search;
    volatile int idx;
    volatile unsigned int value;
    volatile unsigned int data;
    unsigned int size;
    int type;
    volatile unsigned char positionSeen;
    volatile unsigned char weightsSeen;
    volatile unsigned char indicesSeen;
    unsigned char* entry;
    TblEnt* t;
    unsigned int table[0x42][2];
    unsigned int i;

    entry = (unsigned char*)(kVertexDeclarationConversionRules - 1);
    t = (TblEnt*)table;
    for (i = 0; i < kVertexConversionRuleCount; i++) {
        ((volatile unsigned int*)t)[1] = ((unsigned int*)entry)[1];
        ((volatile unsigned int*)t)[2] = ((unsigned int*)entry)[2];
        entry += 8;
        t = (TblEnt*)((unsigned char*)t + 8);
    }
    table[0][0] = (unsigned int)VertexDeclaration_GetElementCount(entries);
    data = 0;
    last = 0xFFFFFFFF;
    aggregate = 0;
    extra = 0;
    positionSeen = 0;
    weightsSeen = 0;
    indicesSeen = 0;
    *out = 0;
    total = 0;
    for (; total < table[0][0]; ++total) {
        search = 0;
        entry = 0;
        for (; search < table[0][0]; ++search) {
            entry = entries + search * 12;
            if (((VertexElement*)entry)->offset == data && ((VertexElement*)entry)->stream == 0)
                break;
            if (search == table[0][0] - 1)
                return 0;
        }
        for (idx = last + 1; idx < kVertexConversionRuleCount; ++idx) {
            t = (TblEnt*)((unsigned int*)table + 1) + idx;
            if (t->code == ((VertexElement*)entry)->usage && t->usageIndex == ((VertexElement*)entry)->usageIndex &&
                t->type == ((VertexElement*)entry)->type) {
                switch (t->code) {
                // Rules are consumed in table order, never reused. Weights
                // require an earlier position and may occur once; indices may
                // occur once and require position unless using type 0xFE.
                case kUsagePosition:
                    positionSeen = 1;
                    break;
                case kUsageBlendWeights:
                    if (positionSeen == 0 || weightsSeen != 0)
                        return 0;
                    weightsSeen = 1;
                    break;
                case kUsageBlendIndices:
                    if ((positionSeen == 0 && t->type != kVertexSpecialByte) || indicesSeen != 0)
                        return 0;
                    indicesSeen = 1;
                    break;
                }
                value = 0;
                // UV rules replace the extra word; other rules OR their flags.
                if (t->code == kUsageTexCoord)
                    extra = t->value;
                else
                    value = t->value;
                type = t->type;
                size = 0;
                aggregate |= value;
                last = idx;
                switch (type) {
                case kVertexFloat1:
                    size = 4;
                    break;
                case kVertexFloat2:
                    size = 8;
                    break;
                case kVertexFloat3:
                    size = 0xC;
                    break;
                case kVertexFloat4:
                    size = 0x10;
                    break;
                case kVertexPackedColor:
                    size = 4;
                    break;
                case kVertexStorageType5:
                    size = 4;
                    break;
                case kVertexShort2:
                    size = 4;
                    break;
                case kVertexShort4:
                    size = 8;
                    break;
                case kVertexSpecialByte:
                    size = 1;
                    break;
                }
                data += size;
                break;
            } else if (t->code == kUsageEnd && t->type == kVertexUnused) {
                return 0;
            }
        }
    }
    *out = aggregate | extra;
    return 1;
}
#pragma global_optimizer reset
#pragma use_frame_pointer reset
#pragma use_lmw_stmw reset

typedef struct {
    unsigned int v[5];
} MaskTable;

#pragma push
#pragma global_optimizer off
#pragma optimization_level 0
#pragma peephole on
#pragma scheduling on
#pragma optimize_for_size on
#pragma use_frame_pointer on
#pragma use_lmw_stmw on
void* __alloca(unsigned int);
// Expand supported vertex-format bits into elements, then append the terminator.
int VertexFormat_BuildDeclaration(register unsigned int mask, register void* entries) {
    // Phases: expand each recognized category mask into a 12-byte element,
    // advance byte offset and usage counters, then append the explicit sentinel.
    // Caller owns enough output storage; success is 1 even for an empty mask.
    register unsigned int n;
    register unsigned int data;
    register unsigned char* e;
    volatile unsigned int i;
    volatile unsigned int tmp;
    unsigned char* volatile last;
    Key extra;
    MaskTable masks = *(MaskTable*)kVertexFormatCategoryMasks;
    __alloca(0);
    volatile unsigned int counts[14] = {0};

    n = 0;
    data = 0;
    for (i = 0; i < 5; i++) {
        e = (unsigned char*)entries + n * 12;
        switch (mask & masks.v[i]) {
        // Expansion map (one selected value per category mask):
        // 1/5 -> position float3; 0x400 -> normal float3; 0xF000 -> color;
        // 0x10000/0x20000 -> UV float2. Both index variants emit type 5,
        // but the first advances one byte and the second advances four.
        // Therefore the one-byte variant cannot be assumed to round-trip
        // through GetStride/TryConvertToFormatFlags for following elements.
        case kFormatOneByteAdvanceType5:
            ((VertexElement*)e)->stream = 0;
            ((VertexElement*)e)->offset = (unsigned short)data;
            data += 1;
            ((VertexElement*)e)->type = kVertexStorageType5;
            ((VertexElement*)e)->method = 0;
            ((VertexElement*)e)->usage = kUsageBlendIndices;
            ((VertexElement*)e)->usageIndex = (unsigned char)counts[kUsageBlendIndices]++;
            break;
        case kFormatPosition:
        case kFormatPositionVariant5:
            ((VertexElement*)e)->stream = 0;
            ((VertexElement*)e)->offset = (unsigned short)data;
            data += 12;
            ((VertexElement*)e)->type = kVertexFloat3;
            ((VertexElement*)e)->method = 0;
            ((VertexElement*)e)->usage = kUsagePosition;
            ((VertexElement*)e)->usageIndex = (unsigned char)counts[kUsagePosition]++;
            break;
        case kFormatFourByteIndices:
            ((VertexElement*)e)->stream = 0;
            ((VertexElement*)e)->offset = (unsigned short)data;
            data += 4;
            ((VertexElement*)e)->type = kVertexStorageType5;
            ((VertexElement*)e)->method = 0;
            ((VertexElement*)e)->usage = kUsageBlendIndices;
            ((VertexElement*)e)->usageIndex = (unsigned char)counts[kUsageBlendIndices]++;
            break;
        case kFormatNormal:
            ((VertexElement*)e)->stream = 0;
            ((VertexElement*)e)->offset = (unsigned short)data;
            data += 12;
            ((VertexElement*)e)->type = kVertexFloat3;
            ((VertexElement*)e)->method = 0;
            ((VertexElement*)e)->usage = kUsageNormal;
            ((VertexElement*)e)->usageIndex = (unsigned char)counts[kUsageNormal]++;
            break;
        case kFormatPackedColor:
            ((VertexElement*)e)->stream = 0;
            ((VertexElement*)e)->offset = (unsigned short)data;
            data += 4;
            ((VertexElement*)e)->type = kVertexPackedColor;
            ((VertexElement*)e)->method = 0;
            ((VertexElement*)e)->usage = kUsageColor;
            ((VertexElement*)e)->usageIndex = (unsigned char)counts[kUsageColor]++;
            break;
        case kFormatTexCoordVariant1:
        case kFormatTexCoordVariant2:
            ((VertexElement*)e)->stream = 0;
            ((VertexElement*)e)->offset = (unsigned short)data;
            data += 8;
            ((VertexElement*)e)->type = kVertexFloat2;
            ((VertexElement*)e)->method = 0;
            ((VertexElement*)e)->usage = kUsageTexCoord;
            tmp = ((unsigned int)(mask & masks.v[i])) >> 16;
            ((VertexElement*)e)->usageIndex = (unsigned char)counts[kUsageTexCoord]++;
            break;
        default:
            continue;
        }
        n++;
    }
    *(unsigned int*)&extra = kVertexDeclarationEnd[0];
    *((unsigned int*)&extra + 1) = kVertexDeclarationEnd[1];
    *((unsigned int*)&extra + 2) = kVertexDeclarationEnd[2];
    ((Key*)entries)[n].stream = extra.stream;
    ((Key*)entries)[n].offset = extra.offset;
    ((Key*)entries)[n].type = extra.type;
    ((Key*)entries)[n].method = extra.method;
    ((Key*)entries)[n].usage = extra.usage;
    ((Key*)entries)[n].usageIndex = extra.usageIndex;
    ((Key*)entries)[n].reserved = extra.reserved;
    last = (unsigned char*)entries + n * 12;
    return 1;
}
#pragma pop

// Return the number of GX texture maps required for this texture and its palette.
int TextureStage_ClassifyTextureSlot(void* values, unsigned int index) {
    void* v = values;
    unsigned int i = index;
    unsigned int* value = (unsigned int*)((unsigned int*)v)[i];
    if (value == 0) {
        return 0;
    }
    volatile unsigned int result[9];
    TextureResource_GetLevelDescription(0, (void*)result, value);
    int ret = 1;
    if (TextureFormat_IsPaletted(result[0]) != 0) {
        ret = TexturePalette_ClassifySlot(v, i);
    } else if ((result[0] + kTextureFormatPairBias) == kTextureFormatPairFirst ||
               (result[0] + kTextureFormatPairBias) == kTextureFormatPairSecond) {
        ret = 2;
    }
    return ret;
}

#pragma global_optimizer off
// Report whether this indexed texture needs two palette maps and RGB reconstruction.
int TextureStage_UsesSplitIndexedPalette(void* values, unsigned int index) {
    void* v = values;
    unsigned int i = index;
    unsigned int* value = (unsigned int*)((unsigned int*)v)[i];
    if (value == 0) {
        return 0;
    }
    volatile unsigned int result[9];
    TextureResource_GetLevelDescription(0, (void*)result, value);
    if ((result[0] + kPlatformFormatBias) != kTextureFormatIndexed) {
        return 0;
    }
    void* second = ((TextureStageState*)v)->palettes[i];
    if (second == 0) {
        return 0;
    }
    volatile unsigned int result2[5];
    TexturePalette_GetDescription(second, (unsigned int*)result2);
    if ((result2[0] + kPlatformFormatBias) != kPaletteFormatSplit) {
        return 0;
    }
    return 1;
}
#pragma global_optimizer reset

#pragma global_optimizer off
#pragma optimization_level 1
#pragma use_frame_pointer on
#pragma use_lmw_stmw on
// Sum the palette map slots required by the requested texture stages.
int TexturePalette_CountRequiredSlots(void* volatile values, volatile unsigned int count) {
    int total = 0;
    for (unsigned int i = 0; i < count; i++) {
        total += TexturePalette_ClassifySlot(values, i);
    }
    return total;
}
#pragma global_optimizer reset
#pragma use_frame_pointer reset
#pragma use_lmw_stmw reset

#pragma global_optimizer off
// Return zero for no palette, two for split format, and one for other palettes.
int TexturePalette_ClassifySlot(void* values, unsigned int index) {
    void* value = ((TextureStageState*)values)->palettes[index];
    if (value == 0) {
        return 0;
    }
    unsigned int result[5];
    TexturePalette_GetDescription(value, result);
    if ((result[0] + kPlatformFormatBias) != kPaletteFormatSplit) {
        return 1;
    }
    return 2;
}
#pragma global_optimizer reset

#pragma global_optimizer off
#pragma use_frame_pointer on
#pragma use_lmw_stmw on
// Dispatch the resource descriptor query by the handle kind encoded in bits 13 through 15.
void TextureResource_GetLevelDescription(void* level, void* description, unsigned int* resource) {
    {
        unsigned int resourceKind;
        unsigned int handleBits = *resource;
        resourceKind = (handleBits >> 13) & 0x7;
        switch (resourceKind) {
        case 3:
            Texture_GetLevelDescription(resource, level, description);
            break;
        case 4:
            Texture_GetKind4LevelDescription(resource, level, description);
            break;
        default:
            break;
        }
    }
}
#pragma global_optimizer reset
#pragma use_frame_pointer reset
#pragma use_lmw_stmw reset

} // extern "C"
