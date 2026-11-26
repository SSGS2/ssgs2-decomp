// Texture animation: sample authored tracks into transform parameters, then
// build 64-byte texture matrices for the material renderer (gtl_generic_renderer_material.cpp).
// Modding index: track target IDs below choose transform fields; key times and
// type-specific key layouts choose interpolation. gtl_Transform_BuildTextureMatrix
// (gtl_transform_frustum_stopwatch_input.cpp) interprets the resulting 0x30-byte scale/rotation/translation/mode
// record. Shared samplers gtl_animation_node_evaluate.cpp/gtl_animation_model_matrices.cpp use the same tables and scratch.

#include "types.h"
#include "sdk_protos.h"
#include "MSL/stddef.h"

// Big-endian Wii directory entry, 16 bytes: tag/count/element byte size/
// offset. Chunk offsets below are added to the RESOURCE OBJECT, not its +C
// directory pointer. dk-relative track offsets have a different base.
typedef struct GtlChunk {
    u32 tag;
    u32 count;
    u32 elementStrideBytes;
    u32 offset;
} GtlChunk;

// pt (0x7074): eight-byte range. matrix is output slot (0xFF skips); start/count
// select contiguous vcf records. dpt (0x647074) supplies 0x30 bytes per matrix.
typedef struct GtlTexRange {
    u8 matrix;
    u8 unk1[3];
    u16 count;
    u16 start;
} GtlTexRange;

// vcf (0x766366): 16-byte track, not an individual key. transformTarget +2 selects target,
// keyDataType +4 selects key encoding/stride/tables (0..9, see valid search types below),
// count +A is key count (>0), offset +C is BYTE offset into dk (0x646B) data.
typedef struct GtlTexKey {
    u8 unk0[2];
    u8 transformTarget;
    u8 reserved3;
    u32 keyDataType;
    u8 unk8[2];
    u16 count;
    u32 offset;
} GtlTexKey;

typedef struct GtlResourceFile {
    u8 unk0[0xC];
    void* unkC;
} GtlResourceFile;

typedef struct GtlTexMatrix {
    u32 w[12];
} GtlTexMatrix;

// The search functions take (first, last, key probe); the original leaves the probe in r5.
typedef void* (*GtlTexSearchFn)(void* first, void* end);
typedef void (*GtlTexInterpFn)(void* dst, void* a, void* b, f32 weight, f32 lo, f32 hi);

extern "C" {

GtlChunk* gtl_ResourceFile_FindChunk(void* res, void* base, u32 tag, ...);
u8* gtl_ResourceFile_GetChunkElement(void* res, void* base, u32 tag, u32 index);
void Vec4_Copy(void* dst, const void* src);
u32 gtl_Resource_GetDataTypeByteStride(u32 type);
void gtl_Transform_BuildTextureMatrix(void* dst, void* src);

extern u8 s_animationSamplingTables[];
extern u8 s_animationKeyScratchType1[];
extern u8 s_animationKeyScratchType2[];
extern void* s_animationKeySearchProbes[];
extern u8 s_animationKeyScratchType0[];
extern s32 s_animationEndpointMode;
extern u8 s_animationKeySearchProbesInitialized;
extern f32 s_animationSegmentDuration;
extern f32 s_animationSegmentStartTime;
extern f32 s_animationSampleTime;
extern f32 s_animationSegmentEndTime;
extern const f32 kFloat_0_GtlModel;
extern const f32 kAnimationWrapInterval;

// Required output matrices and valid resource. First ABI argument is unused.
// Output slots are range.matrix*64 bytes; skipped/missing-data slots stay as-is.
// Time is authored key-time units, with NO seconds/frame-rate conversion here;
// double input is narrowed once to float before every comparison/calculation.
// Caller must supply times in the same scale as dk's floats. Out-of-range mode
// 0 clamps endpoints; mode 1 interpolates last->first across a one-key-time-unit
// seam (kAnimationWrapInterval=1). Other global mode values have no established contract.
// Return 0 if pt/dpt absent (no output writes), else 1 even if all slots skipped.
// No bounds/type/zero-count validation or allocation; borrowed resource and
// output arrays must cover all selected records/indices and must not alias.
// Global search keys and interpolation scalars are overwritten: serialize with
// every other gtl animation evaluator. Not reentrant or safe for parallel use.
s32 gtl_Animation_EvaluateTextureTransformMatrices(void* unused, u8* matrices, GtlResourceFile* resource,
                                                   f64 authoredTime) {
    u8* animationTables = s_animationSamplingTables;
    f32 t;
    GtlChunk* pt;
    GtlChunk* dpt;
    u8* ranges;
    u8* mats;
    u8* vcf;
    u8* dk;
    u8* searchKeyType3;
    u8* searchKeyType4;
    u8* searchKeyType6;
    u8* searchKeyType7;
    u8* searchKeyType8;
    u8* searchKeyType9;
    GtlTexSearchFn* searchFns;
    u32 i;
    u32 off;

    // Preserve the first lookup (mtrl plus original variadic extra word), even
    // though this code does not use its result. Required tags are tested next.
    gtl_ResourceFile_FindChunk(resource, resource->unkC, 0x6C72746D, 0x6C720000);
    dpt = gtl_ResourceFile_FindChunk(resource, resource->unkC, 0x647074);
    pt = gtl_ResourceFile_FindChunk(resource, resource->unkC, 0x7074);
    if (dpt == NULL || pt == NULL) {
        return 0;
    }
    ranges = (pt != NULL) ? (u8*)resource + pt->offset : NULL;
    mats = (dpt != NULL) ? (u8*)resource + dpt->offset : NULL;
    vcf = gtl_ResourceFile_GetChunkElement(resource, resource->unkC, 0x766366, 0);
    dk = gtl_ResourceFile_GetChunkElement(resource, resource->unkC, 0x646B, 0);
    // Wii address bindings: mutable search-key records for types 3,4,6,7,8,9.
    // Keep absolute-address expressions here to preserve the original accesses;
    // corresponding symbols are s_animationKeyScratchType3/18B0/1890/1860/1820/17C0.
    // The 10-entry lower-bound table at s_animationKeySearchFunctions has callable entries ONLY
    // for types 0,1,6,7,8,9; types 2..5 cannot take the interior-search branch.
    searchKeyType3 = (u8*)0x805B18E0;
    searchKeyType4 = (u8*)0x805B18B0;
    searchKeyType6 = (u8*)0x805B1890;
    searchKeyType7 = (u8*)0x805B1860;
    searchKeyType8 = (u8*)0x805B1820;
    searchKeyType9 = (u8*)0x805B17C0;
    searchFns = (GtlTexSearchFn*)0x804DA378;

    // s_animationSamplingTables schema (10 entries each): +00 time byte offsets, +28
    // value byte offsets, +50 interpolation function pointers, +78 defaults.
    // Type -> stride/time/value: 0=8/4/0, 1=16/12/0, 6=32/20/16,
    // 7=48/40/32, 8=64/60/48, 9=96/80/64. Search functions in gtl_bitmap_font_and_anim_keys.cpp
    // compare float time and return the first key >= the mutable search key.
    t = (f32)authoredTime;
    off = 0;
    for (i = 0; i < pt->count; i++, off += 8) {
        GtlTexRange* range = (GtlTexRange*)(ranges + off);
        GtlTexMatrix m;
        GtlTexMatrix w;
        u8* out;
        u32 start;
        u32 j;

        if (range->matrix == 0xFF) {
            continue;
        }
        out = matrices + (range->matrix << 6);
        if (mats == NULL || vcf == NULL || dk == NULL) {
            continue;
        }
        if (mats == NULL) {
            // Retain the fallback even though the guard above skips null mats.
            Vec4_Copy(&m.w[0], animationTables + 0x78);
            Vec4_Copy(&m.w[4], animationTables + 0x78 + 0x10);
            Vec4_Copy(&m.w[8], animationTables + 0x78 + 0x20);
        } else {
            m = *(GtlTexMatrix*)(mats + range->matrix * 0x30);
        }

        if (mats != NULL && vcf != NULL && dk != NULL && range->count != 0) {
            w = m;
            start = range->start;
            for (j = 0; j < range->count; j++) {
                GtlTexKey* key = (GtlTexKey*)(vcf + ((start + j) << 4));
                u8* dst = NULL;
                u32 size = 0;
                u32 comp;
                u32 stride;
                u32 timeOff;
                u32 n;
                u8* base;
                u8* last;
                u8* a;
                u8* b;
                f32 wt;
                f32 lo;
                f32 hi;
                s32 geFirst;
                s32 geLast;

                // Transform-record targets: 85 first 16 bytes (scale/padding),
                // 86 rotation quaternion, 8A/8B translation U/V, 8C word +28,
                // 8D full final 16 bytes including mode +2C. Unsupported type
                // leaves dst null/size zero; valid assets must avoid that path.
                switch (key->transformTarget) {
                case 0x85:
                    dst = (u8*)&w.w[0];
                    size = 0x10;
                    break;
                    // The original jump table jumptable_804DA4C0 (index = target - 0x85) sends 0x89 (not 0x86) to the rotation record.
                case 0x86:
                    dst = (u8*)&w.w[4];
                    size = 0x10;
                    break;
                case 0x8A:
                    dst = (u8*)&w.w[8];
                    size = 4;
                    break;
                case 0x8B:
                    dst = (u8*)&w.w[9];
                    size = 4;
                    break;
                case 0x8C:
                    dst = (u8*)&w.w[10];
                    size = 4;
                    break;
                case 0x8D:
                    dst = (u8*)&w.w[8];
                    size = 0x10;
                    break;
                }

                comp = key->keyDataType;
                stride = gtl_Resource_GetDataTypeByteStride(comp);
                base = dk + key->offset;
                timeOff = ((u32*)animationTables)[comp];
                n = key->count;
                geFirst = (*(f32*)(base + timeOff) >= t);

                if ((n == 1 || geFirst) && s_animationEndpointMode == 0) {
                    Memory_Copy(dst, base + ((u32*)(animationTables + 0x28))[comp], size);
                    continue;
                }
                last = base + stride * (n - 1);
                geLast = (t >= *(f32*)(last + timeOff));
                if (geLast && s_animationEndpointMode == 0) {
                    Memory_Copy(dst, last + ((u32*)(animationTables + 0x28))[comp], size);
                    continue;
                }

                hi = kFloat_0_GtlModel;
                lo = kFloat_0_GtlModel;
                wt = kFloat_0_GtlModel;
                a = NULL;
                b = NULL;
                if (geFirst || geLast) {
                    if (s_animationEndpointMode == 1) {
                        b = base;
                        a = last;
                        if (geFirst) {
                            hi = *(f32*)(base + timeOff);
                            lo = hi - kAnimationWrapInterval;
                            wt = (t - lo) / kAnimationWrapInterval;
                        } else {
                            lo = *(f32*)(last + timeOff);
                            hi = lo + kAnimationWrapInterval;
                            wt = (t - lo) / kAnimationWrapInterval;
                        }
                    }
                } else {
                    if (s_animationKeySearchProbesInitialized == 0) {
                        s_animationKeySearchProbes[0] = s_animationKeyScratchType0;
                        s_animationKeySearchProbes[1] = s_animationKeyScratchType1;
                        s_animationKeySearchProbes[2] = s_animationKeyScratchType2;
                        s_animationKeySearchProbes[3] = searchKeyType3;
                        s_animationKeySearchProbes[4] = searchKeyType4;
                        s_animationKeySearchProbes[6] = searchKeyType6;
                        s_animationKeySearchProbes[7] = searchKeyType7;
                        s_animationKeySearchProbes[8] = searchKeyType8;
                        s_animationKeySearchProbes[9] = searchKeyType9;
                        s_animationKeySearchProbesInitialized = 1;
                    }
                    *(f32*)((u8*)s_animationKeySearchProbes[comp] + timeOff) = t;
                    b = (u8*)searchFns[comp](base + stride, base + stride * n);
                    if (b == NULL) {
                        b = base + stride * (n - 1);
                    }
                    hi = *(f32*)(b + timeOff);
                    if (t == hi) {
                        Memory_Copy(dst, b + ((u32*)(animationTables + 0x28))[comp], size);
                        continue;
                    }
                    a = base + (((u32)(b - base) / stride) - 1) * stride;
                    lo = *(f32*)(a + timeOff);
                    wt = (t - lo) / (hi - lo);
                }

                // Shared interpolation context: +314 lower time, +318 sample
                // time, +310 interval length, +31C upper time. Interpolators
                // may read these globals as well as the explicit arguments.
                s_animationSegmentStartTime = lo;
                s_animationSampleTime = t;
                s_animationSegmentDuration = hi - lo;
                s_animationSegmentEndTime = hi;
                ((GtlTexInterpFn*)(animationTables + 0x50))[comp](dst, a, b, wt, lo, hi);
            }
            // The original (0x803E4158..0x803E41B8) keeps the record's own mode word (+0x2C, saved before the key loop) and
            // copies only words 0..10 back. Target 0x8D copies 16 bytes into w[8..11], whose last word is the key time.
            m = w;
        }
        gtl_Transform_BuildTextureMatrix(out, &m);
    }
    return 1;
}

} // extern "C"
