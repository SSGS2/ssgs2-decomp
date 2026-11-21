#include <decomp/cw_bitfields.h>
// gtl mesh vertex-stream binding. Modding lookup: mesh buffer/decl indices choose
// the resources; effectType chooses built-in render path; material depth +0xD4
// is negated/scaled by 1/512 and passed to the upload call with index 6.
// The upload's actual source range and engine consumption are unresolved; see below.
// Graphics upload/creation: gtl_generic_model_graphics_load.cpp and resource APIs; generic_modeling_resource_description.cpp only queries
// counts. Material state setup: gtl_generic_renderer_material.cpp; resource material records: below.
// No draw is issued here. This prepares persistent renderer state for a caller.
// Translation unit 0x803DDE94-0x803DE3E0 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

typedef unsigned int u32;

typedef unsigned short u16;

typedef float f32;

struct BVS_Vec3 { f32 x, y, z; };

enum MeshBindingValue {
    kNoBuiltinEffect = 0, kBuiltinWithoutExtraUpload = 1,
    kBuiltinDepthProjection = 2, kBuiltinAdditiveStreams = 3,
    kBuiltinTextureProjection = 6,
    kDepthProjectionFloatSlot = 6, kTextureProjectionSettingsSlot = 9
};

struct BVS_VBDesc {
    u32 unk0;
    u32 unk4;
    u32 size;
    u32 fvf;
};

struct BVS_RefRes {
    CW_BITS2(
        u32 rc : 16,
        u32 rest : 16)
};

struct BVS_Mesh {
    u16 effectType;   // 0x0
    u16 type7Id;      // 0x2
    u16 materialIdx;  // 0x4
    char reserved6[0x4];
    u16 stride;       // 0xa
    char unkC[0x6];
    u16 bufferIdx;    // 0x12
    char unk14[0x4];
    u16 declIdx;      // 0x18
};

struct BVS_Material {
    char unk0[0xd4];
    f32 depth;        // +D4 authored scalar; this path computes -(value)*(1/512).
    char reservedD8[0x8];
    char settings[0x10]; // +E0 four native float words; see upload contract below.
};
// Resource material stride is 0xF0. The two upload calls below have a separate
// ABI problem: gtl_builtin_effect_resources.cpp's FloatArray_CopyIndexedRange uses the supplied
// index on BOTH arrays and copies 4*n floats. It does not copy n floats from
// source[0] to destination[index]. For effect2, (6,&projectionDepth,1) reads
// source floats 6..9 beyond the one-float local. For effect6, (9,settings,1)
// reads source floats 9..12, material bytes +104..+113, beyond this record.
// Thus +E0..+EF is NOT proven to be the tint reaching the engine in this code.
// The engine destination is s_textureProjectionSettings+80+9*4 (gtl_builtin_effect_resources.cpp),
// whereas SetupTintCombiner in gtl_texture_projection_input_device.cpp reads RGBA at +80..+8F. Its normalized
// color conversion cannot establish names for these four material words.
// Preserve these calls/bytes; reconciling intended payload/tint needs a separate
// comparison against the original, not guessed RGBA field names.
typedef char BVS_Material_size[(sizeof(BVS_Material) == 0xF0) ? 1 : -1];

struct BVS_Renderer {
    BVS_Mesh* mesh;       // 0x0
    char unk4[0x4];
    void** baseVertexBuffers;       // 0x8: base vertex buffers, indexed by mesh.bufferIdx
    void** additiveVertexBuffers;      // 0xc: optional additive position/normal streams, same index
    void** mergedVertexBuffers;      // 0x10: optional writable CPU-sum output buffers, same index
    void** additiveStreamDeclarations;       // 0x14: two-stream declarations indexed by bufferIdx
    void** singleStreamDeclarations;       // 0x18: single-stream declarations indexed by declIdx
    BVS_Material* mats;   // 0x1c
};

extern "C" {

void VertexBuffer_GetDescription(void* vb, BVS_VBDesc* desc);
int Resource_GetOffsetPtr(void* res, int a, int b, void* out, int c);
u32 FlexibleVertexFormat_GetStride(u32 fvf);
u32 VertexFormat_GetElementOffset(u32 fvf, int element);
void Memory_Copy(void* dst, const void* src, u32 n);
void Vec3_Add(BVS_Vec3* out, const BVS_Vec3* a, const BVS_Vec3* b);
void GraphicsBuffer_Flush(void* buf);
void Engine_SetVertexDeclaration(void* decl);
void GraphicsResource_AcquireBuiltinEffect(int id, BVS_RefRes** out);
void GraphicsResource_AcquireCachedType7(int id, BVS_RefRes** out);
void Engine_SetType6Resource(BVS_RefRes* res);
void Engine_SetType7Resource(BVS_RefRes* res);
void Engine_SetVertexStream(int stream, void* vb, int offset, u32 stride);
void CubeResourceStorage_dtor(void* p, int flags);
void BuiltinEffect_CopyProjectionFloatRangeThunk(int id, f32* v, int n);
void TextureProjection_CopySettingsFloatRangeThunk(int id, void* v, int n);
extern const f32 kMaterialDepthToProjectionScale;
// Drop the temporary acquired reference; Engine_SetType*Resource retains its
// own reference. Zero count invokes deleting destructor, never called on null.
static inline void BVS_Release(BVS_RefRes* resource) {
    if (--resource->rc == 0) CubeResourceStorage_dtor(resource, 1);
}
// Required mesh/base-buffer/declaration/material arrays with valid mesh indices.
// All resources are borrowed from self and must outlive submission/draw. CPU
// merge requires compatible vertex counts/formats and writable distinct output
// storage: position/normal are 3 floats; optional element 5 copies 8 base bytes.
// Vectors are added in their stored mesh space without normalization/transform.
// Return 0 = additive stream bound on GPU (effect 3, additiveStreamDeclarations, streams 0+1).
// Return 1 = base or CPU-merged buffer bound (mesh effect, singleStreamDeclarations, stream 1 clear).
// These are path indicators, not failure statuses. Mapping/acquire results are
// not checked. No renderer state is restored; successful return does not draw.
// CPU path writes summed positions/normals then flushes additive, output, base
// in that order. Neither source buffer is deliberately modified here.
// effectType: 0 unbind effect, 1 no extra upload, 2 upload -depth/512 to built-in
// float slot 6, 6 pass material.settings to texture-projection slot 9 (see
// indexed-source ABI caveat above). Other
// nonzero IDs are forwarded to the built-in resource table without extra upload;
// type7Id==0 clears type 7, otherwise it indexes the cached type-7 resource table.
// Depth is loaded from the selected authored material record, not computed from
// vertex Z here. The proved local relation is projectionDepth=-depth/512 in
// binary32, negation then multiply; increasing depth makes this local scalar
// more negative. No meters/yards/pixels or safe range can be assigned: the
// upstream authoring unit and, because of the indexed-source issue, its actual
// engine consumption are unestablished. kMaterialDepthToProjectionScale is the
// external constant symbol for 1/512; do not reinterpret it as a pixel size.
int gtl_GenericRenderer_BindVertexStreams(BVS_Renderer* self)
{
    void* vb2;
    BVS_Mesh* mesh;
    void* vb;
    mesh = self->mesh;
    vb = self->baseVertexBuffers[mesh->bufferIdx];
    BVS_VBDesc descA;
    VertexBuffer_GetDescription(vb, &descA);
    if (self->additiveVertexBuffers != 0) {
        vb2 = self->additiveVertexBuffers[mesh->bufferIdx];
        if (vb2 != 0) {
            if (self->mergedVertexBuffers != 0 && self->mergedVertexBuffers[mesh->bufferIdx] != 0) {
                BVS_VBDesc descB;
                BVS_VBDesc descC;
                char* p2;
                char* p1;
                char* p3;
                char* dPos;
                char* sPos2;
                char* dNrm;
                char* sNrm1;
                char* sNrm2;
                u32 off3, off1, off2, count, i;
                u32 o0, o3, o5, stride3;
                u32 b0, b3, stride2;
                u32 a0, a3, a5, stride1;
                void* vb3;

                VertexBuffer_GetDescription(vb2, &descB);
                Resource_GetOffsetPtr(vb2, 0, 0, &p2, 0);
                count = descA.size / FlexibleVertexFormat_GetStride(descA.fvf);
                Resource_GetOffsetPtr(vb, 0, 0, &p1, 0);
                vb3 = self->mergedVertexBuffers[mesh->bufferIdx];
                VertexBuffer_GetDescription(vb3, &descC);
                Resource_GetOffsetPtr(vb3, 0, 0, &p3, 0);
                o0 = VertexFormat_GetElementOffset(descC.fvf, 0);
                o3 = VertexFormat_GetElementOffset(descC.fvf, 3);
                o5 = VertexFormat_GetElementOffset(descC.fvf, 5);
                stride3 = FlexibleVertexFormat_GetStride(descC.fvf);
                b0 = VertexFormat_GetElementOffset(descB.fvf, 0);
                b3 = VertexFormat_GetElementOffset(descB.fvf, 3);
                // Preserve the unused format query and its call ordering.
                VertexFormat_GetElementOffset(descB.fvf, 5);
                stride2 = FlexibleVertexFormat_GetStride(descB.fvf);
                a0 = VertexFormat_GetElementOffset(descA.fvf, 0);
                a3 = VertexFormat_GetElementOffset(descA.fvf, 3);
                a5 = VertexFormat_GetElementOffset(descA.fvf, 5);
                stride1 = FlexibleVertexFormat_GetStride(descA.fvf);
                for (i = 0, off3 = 0, off1 = 0, off2 = 0; i < count;
                     i++, off3 += stride3, off1 += stride1, off2 += stride2) {
                    BVS_Vec3 pa, pb, psum, na, nb, nsum;
                    dPos = p3 + (o0 + off3);
                    sPos2 = p2 + (b0 + off2);
                    dNrm = p3 + (o3 + off3);
                    sNrm1 = p1 + (a3 + off1);
                    sNrm2 = p2 + (b3 + off2);
                    Memory_Copy(&pa, p1 + (a0 + off1), 12);
                    Memory_Copy(&pb, sPos2, 12);
                    Memory_Copy(&na, sNrm1, 12);
                    Memory_Copy(&nb, sNrm2, 12);
                    Vec3_Add(&psum, &pa, &pb);
                    Vec3_Add(&nsum, &na, &nb);
                    Memory_Copy(dPos, &psum, 12);
                    Memory_Copy(dNrm, &nsum, 12);
                    if (o5 != 0xFFFFFFFF && a5 != 0xFFFFFFFF) {
                        Memory_Copy(p3 + (o5 + off3), p1 + (a5 + off1), 8);
                    }
                }
                GraphicsBuffer_Flush(vb2);
                GraphicsBuffer_Flush(vb3);
                GraphicsBuffer_Flush(vb);
                vb = vb3;
            } else {
                BVS_VBDesc descD;
                BVS_RefRes* eff;
                VertexBuffer_GetDescription(vb2, &descD);
                Engine_SetVertexDeclaration(self->additiveStreamDeclarations[mesh->bufferIdx]);
                GraphicsResource_AcquireBuiltinEffect(kBuiltinAdditiveStreams, &eff);
                Engine_SetType6Resource(eff);
                BVS_Release(eff);
                Engine_SetType7Resource(0);
                Engine_SetVertexStream(0, vb, 0, mesh->stride);
                Engine_SetVertexStream(1, vb2, 0, FlexibleVertexFormat_GetStride(descD.fvf));
                return 0;
            }
        }
    }
    {
        BVS_Mesh* m;
        BVS_RefRes* eff;
        BVS_RefRes* t7;
        Engine_SetVertexDeclaration(self->singleStreamDeclarations[mesh->declIdx]);
        m = self->mesh;
        eff = 0;
        if (m->effectType != 0) GraphicsResource_AcquireBuiltinEffect(m->effectType, &eff);
        Engine_SetType6Resource(eff);
        t7 = 0;
        if (m->type7Id != 0) GraphicsResource_AcquireCachedType7(m->type7Id, &t7);
        Engine_SetType7Resource(t7);
        switch (m->effectType) {
        case kNoBuiltinEffect:
        case kBuiltinWithoutExtraUpload: break;
        case kBuiltinDepthProjection: {
            f32 depth;
            depth = -self->mats[m->materialIdx].depth;
            f32 projectionDepth = depth * kMaterialDepthToProjectionScale;
            BuiltinEffect_CopyProjectionFloatRangeThunk(kDepthProjectionFloatSlot, &projectionDepth, 1);
            break;
        }
        case kBuiltinTextureProjection:
            TextureProjection_CopySettingsFloatRangeThunk(kTextureProjectionSettingsSlot, self->mats[m->materialIdx].settings, 1);
            break;
        }
        if (eff != 0) {
            BVS_Release(eff);
            eff = 0;
        }
        if (t7 != 0) {
            BVS_Release(t7);
            t7 = 0;
        }
        Engine_SetVertexStream(0, vb, 0, mesh->stride);
        Engine_SetVertexStream(1, 0, 0, 0);
        return 1;
    }
}

} // extern "C"
