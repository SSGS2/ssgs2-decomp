// Gtl model material binding: select a draw entry's material and four texture
// stages before generic model primitives are submitted. Material conversion is
// gtl_GenericMaterial_GetDescription; renderer state IDs/TEV consumers are mapped
// in gx_renderer_state_and_efb_copy.cpp/gx_renderer_state_and_resources.cpp/gtl_texture_stage_tev_setup.cpp/gx_fifo_vertex_and_tev_helpers.cpp. No scene ownership changes here.
// Mod map: material colors/lighting = flags + description; texture bindings =
// entry.texStage, texA images/texB palettes; UV animation = entry.xformIndex,
// xforms and optional matrixIndex/table148; TEV = combinerFlags and colorCombiner/alphaCombiner.
// State templates s_defaultMaterialRenderStateTemplate/EF0 are shared read-only ID/value lists copied
// before values are replaced. Keep their IDs and list lengths together.
// Translation unit 0x803DDA9C-0x803DDE94 (1 function).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct GenericRendererEntry {
    char unk0[8];
    unsigned int materialIndex;
    unsigned char texStage[4];
    unsigned char xformIndex[4];
    unsigned char matrixIndex[4];
    char unk18[4];
};

struct GenericMaterialSlot {
    char unk0[0x60];
    unsigned int flags; // +60, packed signed material-source selectors below
    char unk64[0xC];
    float uv[4][2];
    unsigned int stageConstantWords[4]; // +90, per-stage constant, cached state IDs 25..28
    unsigned int combinerFlags[4]; // +A0, bits 24/25 admit color/alpha combiner upload
    unsigned char colorCombiner[4][4]; // +B0, {color op,arg0,arg1,arg2}, one row/stage
    unsigned char alphaCombiner[4][4]; // +C0, {alpha op,arg0,arg1,arg2}, one row/stage
    char unkD0[0x20];
};

struct GenericRendererXform {
    float v[12];
};

struct Words8 {
    unsigned int w[8];
};

struct Words16 {
    unsigned int w[16];
};

struct GenericRenderer {
    GenericRendererEntry* entries;
    GenericMaterialSlot* materials;
    GenericRendererXform* xforms;
    void** texA;
    void** texB;
    char* matrixTable;
    char unk18[4];
    unsigned short index;
    char reserved1e[2];
    unsigned int cachedTransformSelectors[4];
    unsigned int cachedTextureSelectors[4];
    int dirty;
    float mtx[4][16];
    GenericRendererXform* xforms144;
    unsigned char* table148;
};

extern "C" {

void gtl_GenericMaterial_GetDescription(void* desc, void* mat);
void Engine_SetMaterial(void* desc);
void Engine_SetRenderStateList(void* list, int count);
void Vec3_MultiplyComponents(void* out, void* a, void* b);
void gtl_Transform_BuildTextureMatrix(void* out, void* in);
void PSMTX44Concat(void* a, void* b, void* ab);
void SetTransform(int type, void* mtx);
void Engine_SetTextureStageResource(int stage, void* p);
void Engine_SetTexturePalette(int stage, void* p);
void Engine_SetCachedRenderState(int state, unsigned int value);
void Engine_SetTextureStageStateList(int stage, void* list, int count);
extern unsigned int s_defaultMaterialRenderStateTemplate[8];
extern unsigned int s_defaultTextureStageStateTemplate[16];

// Required self and all selected arrays are borrowed, native PPC32 views. index
// is a draw-entry index, NOT materialIndex; 0xFFFF is a no-op. All other indices
// must fit entries and its material/texture/transform lookup arrays (unchecked).
// Reusing cached self->index is also a no-op: changing material data in place
// requires invalidating that cache before this call. Renderer thread only.
// Side effects: global material/lighting/TEV states, retained engine image/palette
// bindings, texture matrices 2..5; self index/stage caches, dirty and mtx updated.
// No status/rollback; publication of index precedes stage work. Resource setters
// retain bindings, but the source lookup arrays stay owned by the model context.
// A missing transform selector 0xFF leaves the old matrix; a texture selector
// 0xFF updates the cache but does NOT unbind a prior texture/palette. Missing
// texA/texB arrays bind null only for a changed, non-0xFF selector.
void gtl_GenericRenderer_ApplyMaterial(unsigned int index, GenericRenderer* self) {
    float vec[3];
    Words8 list;
    GenericRendererXform xf;
    float mtxTmp[16];
    Words16 stageList;
    unsigned char desc[0x50];

    if (index == 0xffff) {
        return;
    }
    if (index != self->index) {
        GenericRendererEntry* entry;
        GenericMaterialSlot* mat;
        entry = self->entries + index;
        mat = self->materials + entry->materialIndex;
        gtl_GenericMaterial_GetDescription(desc, mat);
        Engine_SetMaterial(desc);

        // Template IDs {18,17,19,20}: material/specular/ambient color source and
        // lighting property 20 (its downstream equation is still unestablished).
        // Each extraction is sign_extend_3bits(flags >> shift) >> 1, retaining
        // the ignored low bit and negative encodings; do not replace with &3.
        list = *(Words8*)s_defaultMaterialRenderStateTemplate;
        list.w[1] = (int)((mat->flags >> 6) << 29) >> 30;
        list.w[3] = (int)((mat->flags >> 8) << 29) >> 30;
        list.w[5] = (int)((mat->flags >> 10) << 29) >> 30;
        list.w[7] = (int)((mat->flags >> 12) << 29) >> 30;
        Engine_SetRenderStateList(list.w, 4);

        self->index = index;
        float* uv;
        float* mtxOut;
        float* mtxIn;
        unsigned int* combinerFlags;
        unsigned char* colorCombiner;
        unsigned char* alphaCombiner;
        int i;
        unsigned int* stageConstantWords;
        stageConstantWords = mat->stageConstantWords;
        uv = &mat->uv[0][0];
        combinerFlags = stageConstantWords + 4;
        colorCombiner = (unsigned char*)(stageConstantWords + 8);
        alphaCombiner = (unsigned char*)(stageConstantWords + 12);
        mtxOut = &self->mtx[0][0];
        mtxIn = mtxOut;
        // Four stage rows, not four renderer objects. mtxOut/mtxIn alias the
        // same 16-float row. Concat writes first = first * optional matrix;
        // texture coordinates are row vectors (gtl_vector_matrix_math.cpp). uv advances two
        // floats, though MultiplyComponents reads three: preserve that third
        // lane read and its cross-row/default-state behavior.
        for (i = 0; i < 4; i++) {
            unsigned char t = entry->xformIndex[i];
            if (t != 0xff) {
                GenericRendererXform* src = self->xforms + t;
                Vec3_MultiplyComponents(vec, src, uv);
                xf = *src;
                xf.v[0] = vec[0];
                xf.v[1] = vec[1];
                xf.v[2] = vec[2];
                gtl_Transform_BuildTextureMatrix(mtxOut, &xf);

                int m = entry->matrixIndex[i];
                if (m != 0xff) {
                    unsigned char sel = self->table148[m * 8];
                    if (sel != 0xff) {
                        if (self->matrixTable != 0) {
                            PSMTX44Concat(mtxOut, mtxIn, self->matrixTable + sel * 0x40);
                        } else {
                            gtl_Transform_BuildTextureMatrix(mtxTmp, self->xforms144 + sel);
                            PSMTX44Concat(mtxOut, mtxIn, mtxTmp);
                        }
                    }
                }
                SetTransform(i + 2, mtxIn);
            }

            self->cachedTransformSelectors[i] = entry->xformIndex[i];
            unsigned char u = entry->texStage[i];
            if (u != self->cachedTextureSelectors[i]) {
                void* res = 0;
                void* tex = 0;
                if (u != 0xff) {
                    if (self->texA != 0) {
                        res = self->texA[u];
                    }
                    if (self->texB != 0) {
                        tex = self->texB[u];
                    }
                    self->dirty = 1;
                    Engine_SetTextureStageResource(i, res);
                    Engine_SetTexturePalette(i, tex);
                }
            }
            self->cachedTextureSelectors[i] = entry->texStage[i];

            unsigned int stateA = *combinerFlags;
            if (((int)((stateA << 7) & 0xC0000000) >> 31) || ((int)((stateA << 6) & 0xC0000000) >> 31)) {
                // IDs {0,14,1,2,3,15,4,5}: color op/arg0/arg1/arg2,
                // then alpha op/arg0/arg1/arg2. Disabled flags leave old TEV
                // state untouched. Values use ENGINE enums, not GX op codes.
                stageList = *(Words16*)s_defaultTextureStageStateTemplate;
                stageList.w[1] = colorCombiner[0];
                stageList.w[3] = colorCombiner[1];
                stageList.w[5] = colorCombiner[2];
                stageList.w[7] = colorCombiner[3];
                stageList.w[9] = alphaCombiner[0];
                stageList.w[11] = alphaCombiner[1];
                stageList.w[13] = alphaCombiner[2];
                stageList.w[15] = alphaCombiner[3];
                Engine_SetCachedRenderState(i + 0x19, *stageConstantWords);
                Engine_SetTextureStageStateList(i, stageList.w, 8);
            }
            mtxOut += 16;
            mtxIn += 16;
            combinerFlags++;
            colorCombiner += 4;
            alphaCombiner += 4;
            stageConstantWords++;
            uv += 2;
        }
    }
}

} // extern "C"
