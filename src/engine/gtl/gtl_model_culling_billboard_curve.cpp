// Translation unit 0x803DE6BC-0x803E02C0 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// Generic model rendering, frustum culling, billboard direction and animation
// interpolation. Mod lookup: Model_DrawWithFrustumCulling chooses active model
// items; Culling_SetEnabled selects whole-model (1) or per-item (2) bounds;
// Culling_SetProjectionMatrix/SetViewMatrix rebuild planes; Billboard_RotateDirection
// applies angular limits; Curve_FindBezierParameter inverts a key-time curve.
// Related data: s_rendererDefaults renderer defaults; g_genericRendererContext shared culling state;
// kPerspectiveFovTangentNumerator..20E8 scalar pool (roles/values below). Resource channel evaluator:
// gtl_animation_transform_channels.cpp; plane construction/bounds math: gtl_transform_frustum_stopwatch_input.cpp.
// Renderer/culling state is global: serialize setters, draw and interpolation.

extern "C" {

extern const float kUnitYAxisVec4[4];
extern const float kPerspectiveFovTangentNumerator;
extern const float kPerspectiveFovAngleScale;
extern const float kProjectionDepthSignThreshold;
extern const float kFloat_0_GtlModel;
extern float kFloat_1_GtlModel;
extern float kFloat_neg1_GtlModel;
extern float kCurveRelativeSpacingTolerance;
extern float kCurveMinimumSpacingTolerance;
extern float kBezierLinearThirdReference;
extern float kBezierCubicCoefficient;
extern float kBezierMidpointScale;
extern const float kBezierMidpointHalf;
extern float kBezierRootCoefficient4;
extern float kBezierRootDenominatorScale;
// Constant roles (not independent tuning knobs): B0=1 and B8=2 projection
// extraction; BC=0 depth-sign split; C0=0, C4=1, C8=-1 shared with billboards
// and curves; CC=.01 relative curve tolerance; D0=.0001 absolute tolerance;
// D4=1/3 linearity reference; D8=3 cubic coefficient; DC=1/8 midpoint weight;
// E0=.5 subdivision/degenerate root; E4=4 discriminant; E8=2 root denominator.
// Split a constant at its consumer for a future mod: changing shared 0/1/-1
// would also change projection, normalization, angle clamps and root equations.

extern float Math_SquareRootApprox(float);

extern float kUnitYAxis[3];
extern void Vec3_Normalize(float* out, float* in);
extern void Vec4_Normalize(void* dst, const void* src);
extern void Mtx44_Copy(void* dst, const void* src);
extern void Mtx44_InvertLegacyCofactors(void* dst, int det, const void* src);
extern void PSMTX44Transpose(void* dst, const void* src);
extern void Mtx44_TransformVec4Forward(float* dst, const float* v, const float* m);
extern void gtl_Frustum_BuildPerspectivePlanes(void* state, float a, float b, float c, float d);
extern float Vec3_Dot(float* a, float* b);
extern void Vec3_Cross(float* out, float* a, float* b);
extern void* Vec2_Hermite(void* dst, const void* p0, const void* startTangent, const void* endPoint, const void* t1, float t);
extern void Mtx44_SetRotationY(float* m, float angle);
extern void Mtx44_SetRotationAxisAngle(float* m, float* axis, float angle);
extern void PSMTX44Concat(float* a, float* b, float* out);
extern double acos(double x);
extern double atan(double x);

extern void* Object_GetField8(unsigned int);
extern void Engine_GetRenderState(int, void*);
extern void Engine_SetCachedRenderState(int, int);
extern void __dl__FPv(void*);
extern void gtl_GenericResource_Free(void*);extern float gtl_Bounds_ProjectBoxRadius(float* matrix, float* plane, void* extents);
extern float gtl_Plane_EvaluatePoint(float* plane, float* point);
extern void* g_genericRendererContext;
extern char s_rendererDefaults[];
extern char s_defaultTextureStageStateTemplate[];
extern const struct RenderOverrides s_defaultItemRenderStateOverrides;
extern int VertexBuffer_GetDescription(void* buffer, unsigned int* desc);
extern int gtl_GenericRenderer_GetVertexBlendMode(void* declaration);
extern void gtl_GenericRenderer_ApplyVertexBlendTransforms(unsigned short self, void* context);
extern void gtl_GenericRenderer_BindVertexStreams(void* render);
extern void gtl_GenericRenderer_ApplyMaterial(unsigned int index, void* state);
extern int Engine_SetIndexBuffer(void* indexBuffer, int baseVertex);
extern int DrawPrimitiveFromBoundBuffers(int primitive, int firstVertex, void* count);
extern int Engine_DrawIndexedPrimitive(void*, int, int, void*, int, int);
extern const float kIdentityMatrix4x4[16];

typedef struct BillboardLimits {
    char unk0[4];
    int limitsEnabled;
    float maxYawRadians;
    float minYawRadians;
    float maxPitchRadians;
    float minPitchRadians;
} BillboardLimits;

typedef struct DrawModel {
    char unk0[0x8];
    unsigned int modelWrapper;
} DrawModel;
// DrawModel +8 is the parent wrapper; Object_GetField8 borrows its parsed model.
// Native Wii model header: +10 signed model word, +14 entry count, entries +20.
// ModelEntry stride 16: tag, count/value, opaque word, resource-relative offset.
// All offsets/indices are trusted; loading/validation must happen before draw.

typedef struct ModelHeader {
    char unk0[0x10];
    int unk10;
    unsigned int entryCount;
} ModelHeader;

typedef struct ModelEntry {
    unsigned int tag;
    unsigned int value;
    unsigned int opaqueChunkWord;
    unsigned int offset;
} ModelEntry;

typedef struct DrawTarget {
    unsigned int firstOverrideData;
    unsigned int firstOverrideEnableFlags;
    char unk8[0x4];
    unsigned int secondOverrideData;
    unsigned int secondOverrideEnable;
    char unk14[0x4];
    unsigned int fallbackVertexBufferTable;
    char unk1C[0x8];
    unsigned int vertexDeclarationTable;
    char unk28[8];
    void** indexBufferTable;
    char reserved34[8];
    unsigned int optionalOverrideData;
    unsigned int optionalOverrideEnable;
} DrawTarget;

typedef struct DrawParams {
    void* transforms;
    unsigned int renderOverride;
    void* materialData;
    void* textureData;
    unsigned int optionalStateWord;
    unsigned int* itemEnableWords;
    unsigned int forwardedRenderWord0;
    unsigned int forwardedRenderWord1;
} DrawParams;
// DrawTarget: +0/+4 first pointer+enable; +C/+10 second pointer+enable;
// +18 fallback render word (a table of vertex buffers; the queued renderer indexes it);
// +24 forwarded render word (a table of vertex declarations); +30 table of index
// buffers (DrawQueuedPrimitives); +3C/+40 optional word+enable.
// DrawParams (nullable overrides): +0 transforms ('trns'), +8 material data,
// +C texture data; +4 nonzero render override; +10 optional state word;
// +14 item-enable u32[count] (zero disables); +18/+1C forwarded render words.
// These are borrowed pointers/opaque renderer settings, not ownership handles.

typedef struct DrawItem {
    unsigned short meshRangeCount;   // mesh range count
    unsigned short firstMeshRangeIndex;   // first mesh range (index into the mesh-range table)
    char unk4[0x8];
    unsigned short reservedC;
    char unkE[0x4];
    unsigned short boundsIndex;
    unsigned short renderStateOverrideSet;  // render-state override set (FFFF = defaults)
    char reserved16[0x4];
    unsigned char itemFlags;
    char reserved1B[0x15];
} DrawItem;

typedef struct DrawBounds {
    float matrix[16];
    void* radiusModeWord;
    char reserved44[0xC];
} DrawBounds;

// Mesh range (12 bytes): a run of primitives of one item.
typedef struct MeshRange {
    unsigned int firstPrimitive;
    unsigned int primitiveCount;
    unsigned int reserved8;
} MeshRange;

// Primitive (0x20 bytes): one draw call of a model item.
typedef struct PrimitiveEntry {
    char reserved0[4];
    unsigned short material;           // 4: index into the material table
    char reserved6[2];
    unsigned short indexedDrawArgument;               // 8: indexed draw argument
    char reservedA[2];
    unsigned short primitiveType;               // C: GX primitive type (first draw argument)
    unsigned short vertexCount;               // E: primitive/vertex count (last draw argument)
    unsigned short indexBuffer;        // 10: index into the index-buffer table; FFFF = not indexed
    unsigned short vertexBuffer;       // 12: index into the vertex-buffer table
    char reserved14[4];
    unsigned short vertexDeclaration;  // 18: index into the vertex-declaration table
    char reserved1A[6];
} PrimitiveEntry;

// Material table entry (0x1C bytes); only the override-set index is read here.
typedef struct MaterialEntry {
    char reserved0[0x18];
    unsigned int overrideSet;          // 18: render-state override set, 0xFFFFFFFF = the item's set
} MaterialEntry;

// Render-state override set (0x20 bytes): value for each guarded render-state slot.
// A nonzero `disabled` word leaves every slot untouched.
typedef struct RenderOverrides {
    unsigned int disabled;
    unsigned int slot12;
    unsigned int slot13;
    unsigned int slot14;
    unsigned int slot6;
    unsigned int slot3;
    unsigned int slot5;
    unsigned int slot0;
} RenderOverrides;

typedef struct DrawFrame {
    unsigned int count;
    int unk154;
    DrawItem* ptr158;
    MeshRange* ptr15C;
    void* ptr160;
    PrimitiveEntry* ptr164;
    void* ptr168;
    void* ptr16C;
    void* ptr170;
    void* ptr174;
    void* ptr178;
    void* ptr17C;
    void* ptr180;
    DrawBounds* ptr184;
    DrawBounds* ptr188;
    void* ptr18C;
    void* ptr190;
    RenderOverrides* ptr194;
    void* ptr198;
    void* ptr19C;
    void* ptr1A0;
    void* ptr1A4;
    void* ptr1A8;
} DrawFrame;
// Frame is a local DIRECTORY of borrowed chunk payloads, not a serialized header.
// ptr158 etc retain original member-address spellings; their suffixes are NOT
// offsets in this compact struct. Schema established by the switch below:
// count/ptr158 = mdl item count/DrawItem[ count ]; ptr15C=msh, ptr160=smsh,
// ptr164=prm, ptr168=mtrl, ptr16C=mtrd, ptr170=bidx, ptr174=texc,
// ptr178=trns, ptr17C=omtx, ptr180=nsf, ptr184=whole-model rbv bounds,
// ptr188=bvpm bounds array, ptr18C=tpd, ptr190=tp, ptr194=ors,
// ptr198=vw, ptr19C=envl, ptr1A0=eidx, ptr1A4=bill (only nonempty), ptr1A8=srpt.
// Chunk names describe identifiers, not a guessed full payload schema; unexamined
// payloads are forwarded to GenericRenderer_DrawQueuedPrimitives unchanged.
// DrawItem stride 0x30: +0/+C active counts, +12 u16 bounds index (FFFF=none),
// +1A bit0 default enable. Bounds stride 0x50: matrix[16], radius-mode word,
// padding. The legacy void* field 'extents' is an INTEGER selector in a pointer
// slot: gtl_transform_frustum_stopwatch_input.cpp's ProjectBoxRadius sums projected axes only for mode 0 and
// returns zero otherwise. It is not a dereferenced extent allocation.

struct RenderStateSaver {
    unsigned int saved;
    unsigned int unk4;
    unsigned int data[8];
    // Restore state 9 after the queued renderer's temporary changes.
    ~RenderStateSaver() { Engine_SetCachedRenderState(9, saved); }
};

typedef struct FrustumState {
    char unk0[0x60];
    float planes[6][4];
    char reservedC0[0xC0];
    int mode180;
} FrustumState;
// DD300: +0 six view-space float4 planes; +60 six transformed planes;
// +C0 transposed view, +100 inverse view, +140 view matrix, +180 culling mode.
// Plane equation dot(normal,point)+d; boundary distance <= -boxRadius is culled.

struct Words84 { unsigned int w[84]; };
struct Words16 { unsigned int w[16]; };
struct Words8 { unsigned int w[8]; };
struct Words32 { unsigned int w[32]; };
struct DrawDefaults {
    char prefix[0x290];
    Words84 state;
    Words32 geometry;
    Words8 render;
};

// A model item participates when either of its two active-count words is nonzero.
static inline int ItemActive(DrawItem* it) {
    return it->meshRangeCount != 0 || it->reservedC != 0;
}

// Test all six shared planes against a transformed bound. Inputs borrow a 16-float
// model-to-world matrix and bound. Concatenation is dst-first (gtl_vector_matrix_math.cpp):
// local bound matrix * model-to-world in row-vector order; translation is [12..14].
// The first three matrix rows include the box axes/scales. Reject
// on the first separating/tangent plane, otherwise accept. No output ownership.
static inline int IsBoundsInside(float* m, DrawBounds* b) {
    DrawBounds t;
    float* pt;
    unsigned int offset;
    float* plane;
    unsigned int p;
    PSMTX44Concat(t.matrix, b->matrix, m);
    t.radiusModeWord = b->radiusModeWord;
    pt = &t.matrix[12];
    offset = 0;
    for (p = 0; p < 6; p++) {
        plane = (float*)((char*)g_genericRendererContext + offset + 0x60);
        float radius = gtl_Bounds_ProjectBoxRadius(t.matrix, plane, t.radiusModeWord);
        float dist = gtl_Plane_EvaluatePoint(plane, pt);
        if (dist <= -radius) {
            return 0;
        }
        offset += 0x10;
    }
    return 1;
}

// Missing matrix or bounds opts out of culling; it is not an empty/invisible box.
static inline int IsBoundsCulled(float* m, DrawBounds* b) {
    if (m == 0) {
        return 0;
    }
    if (b == 0) {
        return 0;
    }
    return !IsBoundsInside(m, b);
}

float gtl_Curve_FindBezierParameter(float x0, float x1, float x2, float x3, float t);

void gtl_Billboard_RotateDirection(float* arg0, const float* arg1, const BillboardLimits* arg2);

void gtl_Model_DrawWithFrustumCulling(const DrawTarget* arg0, float* arg1, const DrawParams* arg2, const DrawModel* arg3);

void* RenderStateSlot12Guard_dtor(void* self, int flags);

void* RenderStateSlot13Guard_dtor(void* self, int flags);

void* RenderStateSlot14Guard_dtor(void* self, int flags);

void* RenderStateSlot6Guard_dtor(void* self, int flags);

void* RenderStateSlot3Guard_dtor(void* self, int flags);

void* dtor_803DEFF8(void* self, int flags);

void* RenderStateSlot9Guard_dtor(void* self, int flags);

void gtl_Culling_SetEnabled(int enabled);

void* dtor_803DF06C(void* self, int flags);

void* GenericResourceRef_dtor(void* self, int flags);

int gtl_Animation_InterpolateHermiteFloat(float* out, const float* a, const float* b, float t);

void* gtl_Culling_GetInverseViewMatrix();

void gtl_Billboard_BuildYAxisBasis(float* out, const float* in);

void gtl_Culling_SetViewMatrix(const void* viewMatrix);

void gtl_Culling_SetProjectionMatrix(const float* p);

// Saves one render-state slot, applies a temporary value and restores the saved
// value when the guard leaves scope (the compiler's inline destructor; the
// out-of-line copies are the RenderStateSlotNGuard_dtor functions below).
// `disabled` != 0 leaves the slot untouched and skips the restore.
template <int Slot>
struct RenderStateGuard {
    unsigned int saved;
    unsigned int applied;
    unsigned int disabled;
    RenderStateGuard(unsigned int value, unsigned int off) {
        disabled = off;
        if (off == 0) {
            Engine_GetRenderState(Slot, this);
            applied = value;
            if (value != saved) {
                Engine_SetCachedRenderState(Slot, value);
            }
        }
    }
    ~RenderStateGuard() {
        if (disabled == 0) {
            if (applied != saved) {
                Engine_SetCachedRenderState(Slot, saved);
            }
        }
    }
};


// Per-draw packet (drawState2 of Model_DrawWithFrustumCulling; the vertex-blend
// routine's GRContext): the words this routine writes.
struct QueuedPacket {
    char unk0[0x24];
    unsigned int itemIndex;       // 24
    PrimitiveEntry* primitive;    // 28: the primitive being drawn
    int vertexBlendMode;          // 2C: gtl_GenericRenderer_GetVertexBlendMode of its declaration
};

#pragma push
#pragma use_lmw_stmw on
// Draw every primitive of model item `index`. frame is the chunk directory, state the
// material/texture override words (state[0] = material table), packet the per-draw
// packet and render the stream-binding block (render[0] = current primitive,
// [2] vertex buffers, [6] vertex declarations). Render-state overrides come from
// the item's override set, then (per primitive) from its material's set; each is held
// by a RenderStateGuard so the previous values return after the draw. flags != 0
// leaves slot 5 alone.
void gtl_GenericRenderer_DrawQueuedPrimitives(unsigned int index, const DrawTarget* target, DrawFrame* frame, void** state, QueuedPacket* packet, void** render, unsigned int flags) {
    const RenderOverrides* itemSet;
    DrawItem* item = frame->ptr158 + index;
    unsigned short blendSelf = 0xFFFF;

    if (item->renderStateOverrideSet == 0xFFFF) {
        itemSet = &s_defaultItemRenderStateOverrides;
    } else {
        itemSet = frame->ptr194 + item->renderStateOverrideSet;
    }
    RenderStateGuard<12> itemGuard12(itemSet->slot12, itemSet->disabled);
    RenderStateGuard<13> itemGuard13(itemSet->slot13, itemSet->disabled);
    RenderStateGuard<14> itemGuard14(itemSet->slot14, itemSet->disabled);
    RenderStateGuard<6> itemGuard6(itemSet->slot6, itemSet->disabled);
    RenderStateGuard<3> itemGuard3(itemSet->slot3, itemSet->disabled);
    RenderStateGuard<5> itemGuard5(itemSet->slot5, itemSet->disabled | flags);
    RenderStateGuard<0> itemGuard0(itemSet->slot0, itemSet->disabled);

    for (unsigned int r = 0; r < item->meshRangeCount; r++) {
        MeshRange* range = frame->ptr15C + item->firstMeshRangeIndex + r;
        for (unsigned int p = 0; p < range->primitiveCount; p++) {
            PrimitiveEntry* prim = frame->ptr164 + range->firstPrimitive + p;
            unsigned int descriptor[4];
            packet->primitive = prim;
            VertexBuffer_GetDescription(((void**)render[2])[prim->vertexBuffer], descriptor);
            packet->vertexBlendMode = gtl_GenericRenderer_GetVertexBlendMode(((void**)render[6])[prim->vertexDeclaration]);
            gtl_GenericRenderer_ApplyVertexBlendTransforms(blendSelf, packet);
            render[0] = prim;
            gtl_GenericRenderer_BindVertexStreams(render);
            gtl_GenericRenderer_ApplyMaterial(prim->material, state);

            MaterialEntry* material = (MaterialEntry*)state[0] + prim->material;
            const RenderOverrides* materialSet;
            if (material->overrideSet == 0xFFFFFFFF) {
                materialSet = itemSet;
            } else {
                materialSet = frame->ptr194 + material->overrideSet;
            }
            RenderStateGuard<12> guard12(materialSet->slot12, materialSet->disabled);
            RenderStateGuard<13> guard13(materialSet->slot13, materialSet->disabled);
            RenderStateGuard<14> guard14(materialSet->slot14, materialSet->disabled);
            RenderStateGuard<6> guard6(materialSet->slot6, materialSet->disabled);
            RenderStateGuard<3> guard3(materialSet->slot3, materialSet->disabled);
            RenderStateGuard<5> guard5(materialSet->slot5, materialSet->disabled | flags);
            RenderStateGuard<0> guard0(materialSet->slot0, materialSet->disabled);

            if (prim->indexBuffer == 0xFFFF) {
                DrawPrimitiveFromBoundBuffers(prim->primitiveType, 0, (void*)prim->vertexCount);
            } else {
                Engine_SetIndexBuffer(target->indexBufferTable[prim->indexBuffer], 0);
                Engine_DrawIndexedPrimitive((void*)prim->primitiveType, 0, 0, (void*)prim->indexedDrawArgument, 0, prim->vertexCount);
            }
        }
    }
}
#pragma pop

// Guard-destructor ABI for the next seven functions: self nullable, return the
// same address; words +0/+4 saved/applied state, +8 suppress-restore flag.
// Restore only when not suppressed and values differ; flags>0 frees self.
// Restore render-state slot 12 after its temporary override.
void* RenderStateSlot12Guard_dtor(void* self, int flags) {
    if (self != 0) {
        if (*(int*)((char*)self + 8) == 0) {
            unsigned int a = *(unsigned int*)self;
            unsigned int b = *(unsigned int*)((char*)self + 4);
            if (b != a) {
                Engine_SetCachedRenderState(0xc, a);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Restore slot 13 under the guard contract above, then optionally free the guard.
void* RenderStateSlot13Guard_dtor(void* self, int flags) {
    if (self != 0) {
        if (*(int*)((char*)self + 8) == 0) {
            unsigned int a = *(unsigned int*)self;
            unsigned int b = *(unsigned int*)((char*)self + 4);
            if (b != a) {
                Engine_SetCachedRenderState(0xd, a);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Restore slot 14 under the guard contract above, then optionally free the guard.
void* RenderStateSlot14Guard_dtor(void* self, int flags) {
    if (self != 0) {
        if (*(int*)((char*)self + 8) == 0) {
            unsigned int a = *(unsigned int*)self;
            unsigned int b = *(unsigned int*)((char*)self + 4);
            if (b != a) {
                Engine_SetCachedRenderState(0xe, a);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Restore slot 6 under the guard contract above, then optionally free the guard.
void* RenderStateSlot6Guard_dtor(void* self, int flags) {
    if (self != 0) {
        if (*(int*)((char*)self + 8) == 0) {
            unsigned int a = *(unsigned int*)self;
            unsigned int b = *(unsigned int*)((char*)self + 4);
            if (b != a) {
                Engine_SetCachedRenderState(6, a);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Restore slot 3 under the guard contract above, then optionally free the guard.
void* RenderStateSlot3Guard_dtor(void* self, int flags) {
    if (self != 0) {
        if (*(int*)((char*)self + 8) == 0) {
            unsigned int a = *(unsigned int*)self;
            unsigned int b = *(unsigned int*)((char*)self + 4);
            if (b != a) {
                Engine_SetCachedRenderState(3, a);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Restore slot 5 under the guard contract above, then optionally free the guard.
void* dtor_803DEFF8(void* self, int flags) {
    if (self != 0) {
        if (*(int*)((char*)self + 8) == 0) {
            unsigned int a = *(unsigned int*)self;
            unsigned int b = *(unsigned int*)((char*)self + 4);
            if (b != a) {
                Engine_SetCachedRenderState(5, a);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Restore slot 0 under the guard contract above, then optionally free the guard.
void* dtor_803DF06C(void* self, int flags) {
    if (self != 0) {
        if (*(int*)((char*)self + 8) == 0) {
            unsigned int a = *(unsigned int*)self;
            unsigned int b = *(unsigned int*)((char*)self + 4);
            if (b != a) {
                Engine_SetCachedRenderState(0, a);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Restore slot 9 unconditionally for nonnull self, then optionally free the guard.
void* RenderStateSlot9Guard_dtor(void* self, int flags) {
    if (self != 0) {
        Engine_SetCachedRenderState(9, *(int*)self);
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// Draw every enabled active item of a loaded model. Required target/model and
// initialized DD300/default tables; matrix may be null to bypass bounds testing,
// params may be null. Caller owns all pointers for the synchronous call; renderer
// must not retain these stack packets. No validation/status/rollback is provided.
// Resource must contain all chunks referenced by its items; bounds indices must
// fit bvpm, and params->unk14, when present, must contain count u32 enables.
// Phase map: (1) borrow chunk directory, (2) apply nullable overrides, (3) whole
// model test in mode 1, (4) construct packets from defaults, (5) active/enable/
// per-item bounds admission in mode 2, then queue/draw each admitted item.
// Render packet map: drawState[0..5] material/texture/target overrides, [6] current
// state2, [17..80] four identity matrices, [81..83] tpd/tp/target flags. Geometry
// packet drawState2[0..8] = items,omtx,trns,matrix,bidx,envl,vw,eidx,bill;
// [9] item index and [14]=0 per submission; remaining words retain defaults.
// obj.data[2..7] carries target/param render overrides and material data. Only
// render-state 9 is saved/restored locally (including unwinding); state2 is read
// but not restored here. The lower renderer owns any further state transitions.
// Missing model bytes produce zero items; a culled model returns before state9
// acquisition. No model/resource references are released by this draw wrapper.
void gtl_Model_DrawWithFrustumCulling(const DrawTarget* arg0, float* arg1, const DrawParams* arg2, const DrawModel* arg3) {
    DrawFrame frame;
    void* drawState[84];
    unsigned int drawState2[32];
    char* tbl;
    char* base;
    ModelHeader* hdr;
    unsigned int flag2;
    DrawItem* item;
    DrawBounds* bounds;
    unsigned int i;
    ModelEntry* entry;
    const unsigned int* src;

    // Phase 1: empty directory first, then resolve each resource-relative payload.
    tbl = s_rendererDefaults;
    base = (char*)Object_GetField8(arg3->modelWrapper);
    frame.count = 0;
    frame.unk154 = -1;
    frame.ptr158 = 0;
    frame.ptr15C = 0;
    frame.ptr160 = 0;
    frame.ptr164 = 0;
    frame.ptr168 = 0;
    frame.ptr16C = 0;
    frame.ptr170 = 0;
    frame.ptr174 = 0;
    frame.ptr178 = 0;
    frame.ptr17C = 0;
    frame.ptr180 = 0;
    frame.ptr184 = 0;
    frame.ptr188 = 0;
    frame.ptr18C = 0;
    frame.ptr190 = 0;
    frame.ptr194 = 0;
    frame.ptr198 = 0;
    frame.ptr19C = 0;
    frame.ptr1A0 = 0;
    frame.ptr1A4 = 0;
    frame.ptr1A8 = 0;
    if (base != 0) {
        hdr = (ModelHeader*)base;
        frame.unk154 = hdr->unk10;
        entry = (ModelEntry*)(base + 0x20);
        for (i = 0; i < hdr->entryCount; entry++, i++) {
            switch (entry->tag) {
            case 0x6C646D:
                frame.count = entry->value;
                frame.ptr158 = (DrawItem*)(base + entry->offset);
                break;
            case 0x68736D:
                frame.ptr15C = (MeshRange*)(base + entry->offset);
                break;
            case 0x68736D73:
                frame.ptr160 = base + entry->offset;
                break;
            case 0x6D7270:
                frame.ptr164 = (PrimitiveEntry*)(base + entry->offset);
                break;
            case 0x6C72746D:
                frame.ptr168 = base + entry->offset;
                break;
            case 0x6472746D:
                frame.ptr16C = base + entry->offset;
                break;
            case 0x78646962:
                frame.ptr170 = base + entry->offset;
                break;
            case 0x63786574:
                frame.ptr174 = base + entry->offset;
                break;
            case 0x736E7274:
                frame.ptr178 = base + entry->offset;
                break;
            case 0x78746D6F:
                frame.ptr17C = base + entry->offset;
                break;
            case 0x66736E:
                frame.ptr180 = base + entry->offset;
                break;
            case 0x76646272:
                frame.ptr184 = (DrawBounds*)(base + entry->offset);
                break;
            case 0x6D707662:
                frame.ptr188 = (DrawBounds*)(base + entry->offset);
                break;
            case 0x7074:
                frame.ptr190 = base + entry->offset;
                break;
            case 0x647074:
                frame.ptr18C = base + entry->offset;
                break;
            case 0x73726F:
                frame.ptr194 = (RenderOverrides*)(base + entry->offset);
                break;
            case 0x7776:
                frame.ptr198 = base + entry->offset;
                break;
            case 0x6C766E65:
                frame.ptr19C = base + entry->offset;
                break;
            case 0x78646965:
                frame.ptr1A0 = base + entry->offset;
                break;
            case 0x6C6C6962:
                if ((int)entry->value != 0) {
                    frame.ptr1A4 = base + entry->offset;
                }
                break;
            case 0x70747273:
                frame.ptr1A8 = base + entry->offset;
                break;
            }
        }
    }

    // Phase 2: overrides replace payload pointers without copying or retaining.
    if (arg2 != 0) {
        if (arg2->materialData != 0) {
            frame.ptr16C = arg2->materialData;
        }
        if (arg2->textureData != 0) {
            frame.ptr174 = arg2->textureData;
        }
        if (arg2->transforms != 0) {
            frame.ptr178 = arg2->transforms;
        }
    }

    // Phase 3: mode 1 rejects the whole model, mode 2 defers to each item.
    if (((FrustumState*)g_genericRendererContext)->mode180 == 1 && IsBoundsCulled(arg1, frame.ptr184)) {
        return;
    }

    // Phase 4: copy opaque defaults, then fill only the documented packet words.
// The default render state is reached as s_rendererDefaults + 0x290, which lies inside s_defaultTextureStageStateTemplate
// (0x80498EF0..0x804993F0).
    const DrawDefaults* defaults = (const DrawDefaults*)tbl;
    *(Words84*)drawState = defaults->state;
// Word 7 of the default table is {u16 index = 0xFFFF, u16 unk1E = 0}, the GenericRenderer cached draw-entry index
// (0 means the first draw entry already applied).
    drawState[0] = (void*)frame.ptr168;
    drawState[1] = (void*)frame.ptr16C;
    drawState[2] = (void*)frame.ptr174;
    drawState[3] = (arg0->firstOverrideEnableFlags != 0) ? (void*)arg0->firstOverrideData : 0;
    drawState[4] = (arg0->secondOverrideEnable != 0) ? (void*)arg0->secondOverrideData : 0;
    drawState[5] = (arg2 != 0 && arg2->optionalStateWord != 0) ? (void*)arg2->optionalStateWord : 0;
    unsigned int endFlags = arg0->firstOverrideEnableFlags;
    *(Words16*)&drawState[17] = *(const Words16*)kIdentityMatrix4x4;
    *(Words16*)&drawState[33] = *(const Words16*)kIdentityMatrix4x4;
    *(Words16*)&drawState[49] = *(const Words16*)kIdentityMatrix4x4;
    *(Words16*)&drawState[65] = *(const Words16*)kIdentityMatrix4x4;
    drawState[81] = (void*)frame.ptr18C;
    drawState[82] = (void*)frame.ptr190;
    drawState[83] = (void*)endFlags;

    Engine_GetRenderState(2, &drawState[6]);

    *(Words32*)drawState2 = defaults->geometry;
    drawState2[0] = (unsigned int)frame.ptr158;
    drawState2[1] = (unsigned int)frame.ptr17C;
    drawState2[2] = (unsigned int)frame.ptr178;
    drawState2[3] = (unsigned int)arg1;
    drawState2[4] = (unsigned int)frame.ptr170;
    drawState2[5] = (unsigned int)frame.ptr19C;
    drawState2[6] = (unsigned int)frame.ptr198;
    drawState2[7] = (unsigned int)frame.ptr1A0;
    drawState2[8] = (unsigned int)frame.ptr1A4;

    RenderStateSaver obj;
    *(Words8*)obj.data = defaults->render;
    obj.data[2] = (arg2 != 0 && arg2->renderOverride != 0) ? arg2->renderOverride : arg0->fallbackVertexBufferTable;
    obj.data[3] = (arg2 != 0) ? arg2->forwardedRenderWord0 : 0;
    obj.data[4] = (arg0->optionalOverrideEnable != 0) ? arg0->optionalOverrideData : 0;
    obj.data[5] = (arg2 != 0) ? arg2->forwardedRenderWord1 : 0;
    obj.data[6] = arg0->vertexDeclarationTable;
    obj.data[7] = (unsigned int)frame.ptr16C;

    Engine_GetRenderState(9, &obj);

    // Phase 5: independent active, enable and optional bounds gates precede draw.
    for (unsigned int j = 0; j < frame.count; j++) {
        item = frame.ptr158 + j;
        if (!ItemActive(item)) {
            continue;
        }
        flag2 = (arg2 != 0 && arg2->itemEnableWords != 0) ? arg2->itemEnableWords[j] : (item->itemFlags & 1);
        if (flag2 == 0) {
            continue;
        }
        if (frame.ptr188 != 0 && item->boundsIndex != 0xFFFF) {
            bounds = frame.ptr188 + item->boundsIndex;
            if (((FrustumState*)g_genericRendererContext)->mode180 == 2 && IsBoundsCulled(arg1, bounds)) {
                continue;
            }
        }
        drawState2[9] = j;
        drawState2[14] = 0;
        gtl_GenericRenderer_DrawQueuedPrimitives(j, arg0, &frame, drawState, (QueuedPacket*)drawState2, (void**)obj.data, 0);
    }
}
#pragma pop

// Rebuild perspective planes from a REQUIRED 16-float projection and the last
// view. This is coefficient extraction, not a general matrix decomposition:
// p[0]=horizontal scale, p[5]=vertical cotangent scale, p[10]=depth coefficient,
// p[14]=depth translation. fovY=2*atan(1/p[5]) radians, aspect=p[5]/p[0].
// Let d=p[10], z=p[14]. For d<0 ratio=(d-1)/(1+d), else (1+d)/(d-1);
// near=(z-z*ratio)/ratio, far=near*ratio, preserving this float order exactly.
// These are the distances ACTUALLY passed to BuildPerspectivePlanes, not a
// substitute textbook near/far formula. Its near/far normals face -Z/+Z;
// positive distances enclose negative view Z (see gtl_transform_frustum_stopwatch_input.cpp). Do not infer
// a compatible clip-depth convention from one coefficient or reverse the signs.
// Preconditions: finite perspective coefficients, nonzero p[0]/p[5], d!=+/-1,
// nonzero finite ratio, meaningful positive near<far and nondegenerate view.
// Orthographic, infinite-far, zero/NaN/singular inputs have no rejection path:
// division/atan/normalization propagate invalid values into shared planes.
// Publishes six view planes and six transformed planes (+60); no matrix is
// retained. SetViewMatrix also refreshes them, so either setter uses the latest
// other input. World/view distances share caller units; no yard conversion.
void gtl_Culling_SetProjectionMatrix(const float* p) {
    struct PerspectivePlanes { float fov, aspect, nearZ, farZ; };
    volatile PerspectivePlanes planes;
    float aspect;
    float fov;
    float depth;
    float scale;
    float angle;
    angle = (float)atan(kPerspectiveFovTangentNumerator / p[5]);
    aspect = p[5] / p[0];
    planes.aspect = aspect;
    depth = p[10];
    scale = kPerspectiveFovAngleScale;
    fov = scale * angle;
    planes.fov = fov;
    float ratio;
    if (depth < kProjectionDepthSignThreshold) {
        float tmp;
        float one;
        one = kPerspectiveFovTangentNumerator;
        tmp = p[14];
        float a = depth - one;
        float b = one + depth;
        ratio = a / b;
        float v = tmp - tmp * ratio;
        planes.nearZ = v / ratio;
    } else {
        float tmp;
        float one;
        one = kPerspectiveFovTangentNumerator;
        tmp = p[14];
        float a = one + depth;
        float b = depth - one;
        ratio = a / b;
        float v = tmp - tmp * ratio;
        planes.nearZ = v / ratio;
    }
    float vv = planes.nearZ;
    gtl_Frustum_BuildPerspectivePlanes(g_genericRendererContext, planes.fov, planes.aspect, vv, (planes.farZ = vv * ratio));
    unsigned int r31 = 0;
    unsigned int r30off = 0;
    for (; r31 < 6; ) {
        Mtx44_TransformVec4Forward((float*)((char*)g_genericRendererContext + r30off + 0x60),
                                   (float*)((char*)g_genericRendererContext + r30off),
                                   (float*)((char*)g_genericRendererContext + 0xc0));
        Vec3_Normalize((float*)((char*)g_genericRendererContext + r30off + 0x60),
                       (float*)((char*)g_genericRendererContext + r30off + 0x60));
        r31 = r31 + 1;
        r30off = r30off + 0x10;
    }
}

// Copy the required invertible 16-float view, store inverse/transpose and refresh
// six transformed planes from the last projection. Caller storage is borrowed
// only for this call. Singular input is not rejected and has no status result.
void gtl_Culling_SetViewMatrix(const void* viewMatrix) {
    Mtx44_Copy((char*)g_genericRendererContext + 0x140, viewMatrix);
    unsigned int r31counter = 0;
    char* r30 = (char*)g_genericRendererContext + 0x140;
    Mtx44_InvertLegacyCofactors((char*)g_genericRendererContext + 0x100, 0, r30);
    PSMTX44Transpose((char*)g_genericRendererContext + 0xc0, r30);
    r31counter = 0;
    unsigned int r31 = 0;
    unsigned int r30off = 0;
    for (; r31 < 6; ) {
        Mtx44_TransformVec4Forward((float*)((char*)g_genericRendererContext + r30off + 0x60),
                                  (float*)((char*)g_genericRendererContext + r30off),
                                  (float*)((char*)g_genericRendererContext + 0xc0));
        Vec3_Normalize((float*)((char*)g_genericRendererContext + r30off + 0x60),
                       (float*)((char*)g_genericRendererContext + r30off + 0x60));
        r31 = r31 + 1;
        r30off = r30off + 0x10;
    }
}

// Set culling policy: 1 whole-model bounds, 2 per-item bounds; other words skip
// both tests. Requires initialized global state; does not rebuild any planes.
void gtl_Culling_SetEnabled(int enabled) {
    ((FrustumState*)g_genericRendererContext)->mode180 = enabled;
}

// Borrow shared inverse-view float[16], overwritten by the next view setter;
// caller must not free it. No copy, retain or readiness check is performed.
void* gtl_Culling_GetInverseViewMatrix() {
    return (void*)((char*)g_genericRendererContext + 0x100);
}

// Clear wrapper word +4, free its generic resource at +0 if present, and
// optionally delete wrapper storage (flags>0). Null accepted; returns self.
void* GenericResourceRef_dtor(void* self, int flags) {
    if (self != 0) {
        int zero = 0;
        *(int*)((char*)self + 4) = zero;
        if (self != 0) {
            if (*(void**)self != 0) {
                *(int*)((char*)self + 4) = zero;
                gtl_GenericResource_Free(*(void**)self);
            }
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma opt_lifetimes off
// Build a Y-axis billboard basis in out[0..11]: unit Y at +4, normalized input
// XZ direction at +8, cross-product right vector at +0. Required nonzero XZ
// input[3], output capacity 12 floats; keep distinct storage to avoid overwrites.
// The last word of the first vector is untouched; no translation is produced.
// Lifetime optimization stays disabled to preserve the vector-copy/spill shape.
void gtl_Billboard_BuildYAxisBasis(float* out, const float* in) {
    float z;
    float l3;
    float l2;
    float l1;
    float l0;
    float i0;
    float i2;
    i2 = in[2];
    i0 = in[0];
    z = kFloat_0_GtlModel;
    l0 = kUnitYAxisVec4[0];
    l1 = kUnitYAxisVec4[1];
    l2 = kUnitYAxisVec4[2];
    l3 = kUnitYAxisVec4[3];
    float* r31 = out + 4;
    float* r30 = out + 8;
    r31[0] = l0;
    r31[1] = l1;
    r31[2] = l2;
    r31[3] = l3;
    r30[0] = i0;
    r30[1] = z;
    r30[2] = i2;
    r30[3] = z;
    Vec4_Normalize(r30, r30);
    Vec3_Cross(out, r31, r30);
}
#pragma pop

// Clamp rounding drift to [-1,1] before acos; return pitch magnitude in radians.
static inline float BillboardPitchAngle(float cosine) {
    if (cosine > kFloat_1_GtlModel) {
        cosine = kFloat_1_GtlModel;
    } else if (cosine < kFloat_neg1_GtlModel) {
        cosine = kFloat_neg1_GtlModel;
    }
    return (float)acos((double)cosine);
}

// Clamp a unit direction to billboard yaw/pitch limits in radians. Required
// out[3], in[3], limits: +4 enables clamping; +8/+C positive/negative yaw bounds,
// +10/+14 pitch bounds. Input XZ length must be nonzero for normalization.
// Disabled or already-in-range copies input; otherwise rebuilds a unit direction
// from rotations about Y and a cross-product axis. No allocation or status result.
void gtl_Billboard_RotateDirection(float* arg0, const float* arg1, const BillboardLimits* arg2) {
    float tmp[3];
    float axis[3];
    float mtxA[16];
    float mtxB[16];
    float mtxC[16];
    float f;
    float angleY;
    float angleX;
    int flag;

    if (arg2->limitsEnabled == 0) {
        float z;
        float y;
        float x;
        x = arg1[0];
        y = arg1[1];
        z = arg1[2];
        arg0[0] = x;
        arg0[1] = y;
        arg0[2] = z;
        return;
    }

    struct Raw3 { unsigned int v[3]; };
    *(Raw3*)tmp = *(const Raw3*)arg1;
    tmp[1] = kFloat_0_GtlModel;
    Vec3_Normalize(tmp, tmp);

    f = tmp[2];
    if (f > kFloat_1_GtlModel) {
        f = kFloat_1_GtlModel;
    } else if (f < kFloat_neg1_GtlModel) {
        f = kFloat_neg1_GtlModel;
    }
    angleY = (float)acos((double)f);

    f = Vec3_Dot(tmp, (float*)arg1);
    angleX = BillboardPitchAngle(f);

    flag = 0;
    if (arg1[0] >= kFloat_0_GtlModel) {
        if (arg2->maxYawRadians < angleY) {
            angleY = arg2->maxYawRadians;
            flag = 1;
        }
    } else {
        if (-angleY < arg2->minYawRadians) {
            angleY = arg2->minYawRadians;
            flag = 1;
        } else {
            angleY *= kFloat_neg1_GtlModel;
        }
    }

    if (arg1[1] >= kFloat_0_GtlModel) {
        if (-angleX < arg2->minPitchRadians) {
            angleX = arg2->minPitchRadians;
            flag = 1;
        } else {
            angleX *= kFloat_neg1_GtlModel;
        }
    } else {
        if (arg2->maxPitchRadians < angleX) {
            angleX = arg2->maxPitchRadians;
            flag = 1;
        }
    }

    if (flag == 0) {
        float z;
        float y;
        float x;
        x = arg1[0];
        y = arg1[1];
        z = arg1[2];
        arg0[0] = x;
        arg0[1] = y;
        arg0[2] = z;
        return;
    }

    Mtx44_SetRotationY(mtxA, angleY);
    tmp[0] = mtxA[8];
    tmp[1] = mtxA[9];
    tmp[2] = mtxA[10];
    Vec3_Cross(axis, kUnitYAxis, tmp);
    Mtx44_SetRotationAxisAngle(mtxB, axis, angleX);
    PSMTX44Concat(mtxC, mtxA, mtxB);
    arg0[0] = mtxC[8];
    arg0[1] = mtxC[9];
    arg0[2] = mtxC[10];
}

#pragma push
#pragma opt_lifetimes off
// Find the normalized Bezier parameter for target t in a scalar key-coordinate
// curve (x0..x3); t is a target COORDINATE, not an initial parameter estimate.
// Intended domain: finite monotone increasing controls/endpoints, x0<x3, target
// in [x0,x3]. Output is a dimensionless parameter, nominally [0,1], but NO clamp
// or failure status guarantees it for arbitrary/degenerate inputs.
// Endpoints return 0/1 exactly. tolerance=max(abs(x3-x0)*.01,.0001), in the same
// key-coordinate units; 1/3 is the dimensionless linearity reference.
// Termination: at most EIGHT de Casteljau subdivisions. The existing loop breaks
// when abs(span)>tolerance OR abs((x2-x1)/span-1/3)>.01; preserve those comparison
// directions, even though this is not a general convergence/residual criterion.
// Each selected half narrows [lo,hi], then a quadratic approximation supplies r.
// Zero quadratic term uses linear root, or .5 if also constant; negative
// discriminant is clamped to 0. Square root uses Math_SquareRootApprox.
// No iterative refinement follows; out-of-range targets/flat spans/nonmonotone
// curves may return out-of-range/NaN parameters. Increasing tolerances changes
// subdivision admission, not a proven error bound. Keep all FP order/precision
// and the disabled lifetime optimization: no algebraic simplification here.
float gtl_Curve_FindBezierParameter(float x0, float x1, float x2, float x3, float t) {
    float tol;
    float lo;
    float hi;
    float mid;
    float tmp;
    float dt;
    float b1;
    float b2;
    float disc;
    float root;
    float r;
    int i;

    if (t == x0) {
        return kFloat_0_GtlModel;
    }
    if (t == x3) {
        return kFloat_1_GtlModel;
    }

    tol = (float)__fabs((double)(x3 - x0)) * kCurveRelativeSpacingTolerance;
    if (tol < kCurveMinimumSpacingTolerance) {
        tol = kCurveMinimumSpacingTolerance;
    }

    lo = kFloat_0_GtlModel;
    hi = kFloat_1_GtlModel;

    for (i = 8; i != 0; i--) {
        float span = x3 - x0;
        if ((float)__fabs((double)span) > tol) {
            break;
        }
        if ((float)__fabs((double)(((x2 - x1) / span) - kBezierLinearThirdReference)) > kCurveRelativeSpacingTolerance) {
            break;
        }

        float midpointSum = x3 + (kBezierCubicCoefficient * x2 + (kBezierCubicCoefficient * x1 + x0));
        mid = midpointSum * kBezierMidpointScale;
        if (t < mid) {
            float tmp2 = kBezierMidpointHalf * (x1 + x2);
            x1 = kBezierMidpointHalf * (x0 + x1);
            hi = kBezierMidpointHalf * (lo + hi);
            x3 = mid;
            x2 = kBezierMidpointHalf * (x1 + tmp2);
        } else {
            float tmp2 = kBezierMidpointHalf * (x1 + x2);
            x2 = kBezierMidpointHalf * (x2 + x3);
            lo = kBezierMidpointHalf * (lo + hi);
            x0 = mid;
            x1 = kBezierMidpointHalf * (tmp2 + x2);
        }
    }

    dt = x0 - t;
    b1 = kBezierCubicCoefficient * (x1 - x0);
    b2 = (x3 - x0) - b1;
    if (kFloat_0_GtlModel == b2) {
        if (kFloat_0_GtlModel == b1) {
            r = kBezierMidpointHalf;
        } else {
            r = -dt / b1;
        }
    } else {
        float p4 = kBezierRootCoefficient4 * b2;
        disc = (b1 * b1) - (p4 * dt);
        if (disc < kFloat_0_GtlModel) {
            disc = kFloat_0_GtlModel;
        }
        root = Math_SquareRootApprox(disc);
        if ((b2 + b1) < kFloat_0_GtlModel) {
            root = -root;
        }
        r = (-b1 + root) / (kBezierRootDenominatorScale * b2);
    }
    return (r * (hi - lo)) + lo;
}
#pragma pop

#pragma push
#pragma opt_lifetimes off
// Interpolate one float channel with Hermite tangents. Required out[1], a[3],
// b[2]: a[0] start, a[2] start tangent, b[0] end, b[1] end tangent. t is the
// dimensionless segment parameter (normally [0,1]; no clamp). Stages float2
// values with zero second components, writes out[0], returns 1 without failure.
int gtl_Animation_InterpolateHermiteFloat(float* out, const float* a, const float* b, float t) {
    float result[2];
    float buf0[2];
    float buf1[2];
    float buf2[2];
    float buf3[2];

    float b4;
    float b0 = b[0];
    b4 = b[1];
    float a2 = a[2];
    float a0 = a[0];

    *(unsigned int*)&buf0[0] = 0;
    *(unsigned int*)&buf1[0] = 0;
    *(unsigned int*)&buf2[0] = 0;
    *(unsigned int*)&buf3[0] = 0;

    *(unsigned int*)&buf0[1] = 0;
    buf0[0] = a0;
    *(unsigned int*)&buf1[1] = 0;
    buf1[0] = a2;
    *(unsigned int*)&buf2[1] = 0;
    buf2[0] = b0;
    *(unsigned int*)&buf3[1] = 0;
    buf3[0] = b4;

    Vec2_Hermite(result, buf0, buf1, buf2, buf3, t);
    out[0] = result[0];
    return 1;
}
#pragma pop

} // extern "C"
