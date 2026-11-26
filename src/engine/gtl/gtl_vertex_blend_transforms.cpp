// gtl model rendering: prepare a mesh's transform palette and vertex-blend state.
// Mod/API lookup: ApplyVertexBlendTransforms chooses weighted groups, indexed
// matrices, or one cached rigid transform, in that priority order. GRMesh selects
// ranges; GRContext supplies borrowed node/bind/pose/view tables. Math composition
// and row-vector convention: gtl_vector_matrix_math.cpp (Mtx44_Concat3/4, TransformVec4).
// SetTransform (gx_renderer_state_and_efb_copy.cpp) copies sixteen floats into the engine cache, so stack
// temporaries need survive only the call. Palette slots start at 11.
// Shared scratch s_vertexBlendMatrixScratch / s_vertexBlendWeightScratch holds matrices/weights per group;
// calls must be serialized with other users of that scratch and render state.
// No time or gameplay-distance tuning knob; this controls model deformation.

typedef float f32;

typedef unsigned int u32;

typedef unsigned short u16;

// 64 bytes: four consecutive float rows; row-vector composition, translation row
// at m[3]. No implicit transposition or projection/divide-by-W in this operation.
struct GRMtx44 {
    f32 m[4][4];
};

struct GRNode {
    char unk0[0x18];
    u16 mtxIndex; // 0x18
    char unk1A[2];
    u16 typeIndex; // 0x1C
    char reserved1E[0x12];
};

struct GRBlendGroup {
    u32 count;
    u32 start;
};

struct GRWeight {
    u32 node;
    f32 weight;
};

// Types 0..3 share the same view-scale/translation path below. Their individual
// asset labels are unresolved; do not invent four distinct billboard algorithms.
struct GRTypeEntry {
    int type;
    char unk4[0x14];
};

struct GRMesh {
    char unk0[0x14];
    u16 matStart; // 0x14
    u16 matCount; // 0x16
    char unk18[2];
    u16 blendCount; // 0x1A
    u16 blendStart; // 0x1C
};

struct GRContext {
    GRNode* nodes;              // 0x00
    GRMtx44* invBind;           // 0x04
    GRMtx44* world;             // 0x08
    GRMtx44* view;              // 0x0C
    u16* matIndices;            // 0x10
    GRBlendGroup* groups;       // 0x14
    GRWeight* weights;          // 0x18
    u16* groupIndices;          // 0x1C
    GRTypeEntry* types;         // 0x20
    int nodeIndex;              // 0x24
    GRMesh* mesh;               // 0x28
    u32 state9;                 // 0x2C
    int cachedBlend;            // 0x30
    u32 cachedState9;           // 0x34
    int rigidTransformUploaded; // 0x38: rigid result already uploaded for current context
    char unk3C[4];
    GRMtx44 result; // 0x40
};

extern "C" {

extern GRMtx44 s_vertexBlendMatrixScratch[];
extern f32 s_vertexBlendWeightScratch[];
void PSMTX44Concat(GRMtx44* out, GRMtx44* a, GRMtx44* b);
void Mtx44_WeightedSum(GRMtx44* out, GRMtx44* mats, f32* weights, u32 count);
void Mtx44_Concat3(GRMtx44* out, GRMtx44* a, GRMtx44* b, GRMtx44* c);
void Mtx44_Concat4(GRMtx44* out, GRMtx44* a, GRMtx44* b, GRMtx44* c, GRMtx44* d);
void SetTransform(int slot, GRMtx44* m);
f32 Vec4_Length(f32* v);
void Vec4_Scale(f32* dst, f32* src, f32 s);
void Mtx44_TransformVec4(f32* dst, f32* src, GRMtx44* m);
void Engine_SetCachedRenderState(int state, int value);
enum MeshTransformPolicy {
    kFirstMeshTransformSlot = 11,
    kIndexedMatrixBlendState = 0x22,
    kMeshForwardedRenderState = 9
};
// self is an unused ABI receiver. c, mesh, selected node and all selected table
// entries are required/live. Counts and indices are trusted, not clamped: weight
// groups must fit shared scratch, palette ranges must fit engine transform slots,
// and mat/type/node indices must address their respective arrays. Inputs must not
// alias c->result or shared scratch. No allocation, ownership transfer or status.
// Writes result/rigidTransformUploaded (rigid path), shared scratch (weighted path), engine matrices
// and render states, cachedBlend/cachedState9. No input or render failure recovery.
//
// Composition is explicit left-associated row-vector multiplication:
// weighted: inverseBind[node]*world[node] per weight, sum in original weight order,
// then (currentNodeWorld*sum)*view; indexed: ((currentNodeWorld*inverseBind[index])
// *world[index])*view; rigid: currentNodeWorld*view. Keep this order even if an
// alternative skinning convention seems more familiar. invBind/world/view are the
// caller's bind/pose/final-space inputs; this routine does no unit/axis conversion.
// The exact upstream frame of currentNodeWorld relative to each pose is not
// established here; a mod changing spaces must trace its context producer first.
// For types 0..3 the rigid variant scales each world basis row by the corresponding
// view row's Vec4 length, then transforms the translation row through view. Other
// nonnull type values leave result untouched yet still upload it and set rigidTransformUploaded.
// Caller must supply a valid prior result for such values; no fallback is built.
//
// rigidTransformUploaded suppresses only the rigid upload: caller must clear it after node/pose/view
// changes. Weighted/indexed paths always upload. State 0x22 is set to 1 only for
// the indexed-matrix path, 0 for weighted or rigid; it is not a universal "skinning
// enabled" flag. State 9 is forwarded unchanged from c->state9. Both cached values
// must describe the actual engine state on entry; external state changes require
// invalidating/synchronizing this context. No previous render state is restored.
void gtl_GenericRenderer_ApplyVertexBlendTransforms(void* self, GRContext* c) {
    GRNode* node = &c->nodes[c->nodeIndex];
    GRMesh* mesh = c->mesh;
    int enable = 0;
    GRMtx44* mtx = &c->world[node->mtxIndex];
    GRMtx44 tmp;

    if (mesh->blendCount != 0) {
        u32 i;
        for (i = 0; i < mesh->blendCount; i++) {
            GRMtx44* outs = s_vertexBlendMatrixScratch;
            f32* ws = s_vertexBlendWeightScratch;
            GRBlendGroup* group = &c->groups[c->groupIndices[mesh->blendStart + i]];
            u32 j;
            for (j = 0; j < group->count; j++) {
                GRWeight* w = &c->weights[group->start + j];
                PSMTX44Concat(outs, &c->invBind[w->node], &c->world[c->nodes[w->node].mtxIndex]);
                outs++;
                *ws = w->weight;
                ws++;
            }
            Mtx44_WeightedSum(&tmp, s_vertexBlendMatrixScratch, s_vertexBlendWeightScratch, group->count);
            Mtx44_Concat3(&tmp, mtx, &tmp, c->view);
            SetTransform(i + kFirstMeshTransformSlot, &tmp);
        }
    } else if (mesh->matCount != 0) {
        u32 i;
        GRMtx44* out = &c->result;
        for (i = 0; i < mesh->matCount; i++) {
            u32 idx = c->matIndices[mesh->matStart + i];
            Mtx44_Concat4(out, mtx, &c->invBind[idx], &c->world[c->nodes[idx].mtxIndex], c->view);
            SetTransform(i + kFirstMeshTransformSlot, out);
        }
        enable = 1;
    } else if (c->rigidTransformUploaded == 0) {
        GRMtx44* out = &c->result;
        if (c->types != 0) {
            switch (c->types[node->typeIndex].type) {
            case 0:
            case 1:
            case 2:
            case 3:
                Vec4_Scale(out->m[0], mtx->m[0], Vec4_Length(c->view->m[0]));
                Vec4_Scale(out->m[1], mtx->m[1], Vec4_Length(c->view->m[1]));
                Vec4_Scale(out->m[2], mtx->m[2], Vec4_Length(c->view->m[2]));
                Mtx44_TransformVec4(out->m[3], mtx->m[3], c->view);
                break;
            }
        } else {
            PSMTX44Concat(out, mtx, c->view);
        }
        SetTransform(kFirstMeshTransformSlot, out);
        c->rigidTransformUploaded = 1;
    }

    if (c->cachedBlend != enable) {
        Engine_SetCachedRenderState(kIndexedMatrixBlendState, enable);
        c->cachedBlend = enable;
    }
    if (c->cachedState9 != c->state9) {
        Engine_SetCachedRenderState(kMeshForwardedRenderState, c->state9);
        c->cachedState9 = c->state9;
    }
}

} // extern "C"
