// Colored quad drawing, trail ribbon styles and the transform rotation registry.
//
// Render_DrawColoredPrimitive applies the shared colored-quad render state and draws the
// caller's primitive. The CRotateRegistry methods destroy or clear the registry.

#include <decomp/static_init.h>
#include "classes/CRotateRegistry.h"

extern "C" {

enum {
    kColoredQuadRenderStateCount = 11,
    kColoredQuadVertexFormat = 0xF005,
    kColoredPrimitiveVertexBytes = 16
};
enum TransformOverrideMode {
    kAddTranslation = 0,
    kBlendTranslation = 1,
    kReplaceTranslation = 2
};

extern int Registry_InvokeClearValuesHook(void*);
extern float kFloat_6_2831855;
extern float s_transformRenderUnitTwoPiCache0;
extern float s_transformRenderUnitTwoPiCache1;
extern unsigned int s_coloredQuadRenderStatePairs[22];
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetType7Resource(int);
extern void* GX_GetPrimitiveCount(void*, void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void DrawPrimitiveUP(void*, void*, void*, int);
extern void Vec3_Add(void*, const void*, const void*);
extern void Vec3_Lerp(void*, const void*, const void*, float);
extern void RotateRegistry_dtor(void*, int);
extern void __dl__FPv(void*);

// Packed ribbon style: preserve byte, float and word copy order independently.
struct RibbonStyle {
    unsigned char bytes[8];
    float scalars[2];
    unsigned int words[4];
};
struct RibbonStyleOwner {
    char pad_000[0x2C];
    RibbonStyle style;
};

// Initialize this registry helper's cached full-turn angle.
void TransformRenderUnit_InitTwoPiCache0() {
    s_transformRenderUnitTwoPiCache0 = kFloat_6_2831855;
}

// Initialize the ribbon helper's separate full-turn cache.
void TransformRenderUnit_InitTwoPiCache1() {
    s_transformRenderUnitTwoPiCache1 = kFloat_6_2831855;
}

// Copy the shared state/value pairs to a stack list before submitting them.
void Render_SetColoredQuadState() {
    unsigned int copy[22];
    unsigned int* src = s_coloredQuadRenderStatePairs;
    unsigned int* dst = copy;
    dst -= 1;
    src -= 1;
    unsigned int n = kColoredQuadRenderStateCount;
    do {
        dst[1] = src[1];
        dst[2] = src[2];
        dst += 2;
        src += 2;
    } while (--n);
    Engine_SetRenderStateList(&copy[0], kColoredQuadRenderStateCount);
}

// Apply colored-vertex state and draw the caller's immediate primitive.
void Render_DrawColoredPrimitive(void* primitive, void* vertices, void* primitiveCountInput) {
    struct Block {
        unsigned int w[22];
    };
    Block copy;
    copy = *(Block*)s_coloredQuadRenderStatePairs;
    Engine_SetRenderStateList(&copy.w[0], kColoredQuadRenderStateCount);
    Engine_SetFlexibleVertexFormat(kColoredQuadVertexFormat);
    Engine_SetType7Resource(0);
    void* primitiveCount = GX_GetPrimitiveCount(primitiveCountInput, primitive);
    DrawPrimitiveUP(primitive, primitiveCount, vertices, kColoredPrimitiveVertexBytes);
}

struct TranslationOverride {
    float x, y, z;
    int mode;
    float blendWeight;
    float extraVector[3];
};
struct OverrideNode {
    OverrideNode* left;
    OverrideNode* right;
    OverrideNode* parent;
    unsigned int index;
    TranslationOverride* value;
};
struct OverrideTree {
    unsigned int count;
    unsigned int transformCount;
    void* data;
    OverrideNode head;
};
// Keep the aggregate word-typed: float members change the compiler's bulk copy instructions.
struct TransformEntry {
    unsigned int words[12];
    float* Translation() {
        return (float*)&words[4];
    }
};
typedef char TransformEntry_size[(sizeof(TransformEntry) == 0x30) ? 1 : -1];

// Copy every transform and apply any indexed translation override; reject incompatible arrays.
int SkinTransformOverrides_Apply(OverrideTree* self, TransformEntry* output, TransformEntry* input,
                                 unsigned int transformCount) {
    OverrideNode* node = 0;
    OverrideNode* found = &self->head;
    if (!output || !input)
        return 0;
    unsigned int storedTransformCount = self->transformCount;
    if (storedTransformCount != transformCount)
        return 0;
    if (!self->data)
        return 0;
    TransformEntry* in;
    TransformEntry* out;
    OverrideNode* nil;
    float* outVec;
    unsigned int i;
    i = 0;
    if (storedTransformCount) {
        in = input;
        out = output;
        nil = &self->head;
        while (i < self->transformCount) {
            node = self->head.left;
            found = &self->head;
            while (node) {
                if (node->index >= i) {
                    found = node;
                    node = node->left;
                } else
                    node = node->right;
            }
            if (found == nil || i < found->index)
                found = &self->head;
            outVec = out->Translation();
            const void* inVec =
                in->Translation(); // 0x8025FB14: compute source address before copying, including the blend case.
            *out = *in;
            if (&self->head != found) {
                TranslationOverride* v = found->value;
                if (v) {
                    switch (v->mode) {
                    case kAddTranslation:
                        Vec3_Add(outVec, inVec, v);
                        break;
                    case kBlendTranslation:
                        Vec3_Lerp(outVec, inVec, v, v->blendWeight);
                        break;
                    case kReplaceTranslation:
                        outVec[0] = v->x;
                        outVec[1] = v->y;
                        outVec[2] = v->z;
                        break;
                    }
                }
            }
            ++in;
            ++out;
            ++i;
        }
    }
    return 1;
}

// Copy the packed style without touching the ribbon's geometry or lifetime state.
void TrailRibbon_SetStyle(void* dst, const RibbonStyle* src) {
    RibbonStyle* out = &((RibbonStyleOwner*)dst)->style;
    for (int i = 0; i < 8; i++)
        out->bytes[i] = src->bytes[i];
    for (int i = 0; i < 2; i++)
        out->scalars[i] = src->scalars[i];
    for (int i = 0; i < 4; i++)
        out->words[i] = src->words[i];
}

} // extern "C"

// Destroy the rotation registry, optionally releasing its allocation.
void* CRotateRegistry::Destroy(int doDelete) {
    if (this) {
        RotateRegistry_dtor(this, 0);
        if (doDelete > 0)
            __dl__FPv(this);
    }
    return this;
}

// Forward the virtual clear request to the registry implementation.
int CRotateRegistry::ClearValues() {
    return Registry_InvokeClearValuesHook(this);
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046230C", _ctors_u8025F9F0_0, TransformRenderUnit_InitTwoPiCache0);
STATIC_INIT_ENTRY(".CRT$XCU8046230C", _ctors_u8025F9F0_1, TransformRenderUnit_InitTwoPiCache1);
#pragma pop
