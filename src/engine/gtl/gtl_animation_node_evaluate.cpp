// Model animation: evaluate one node's LOCAL matrix at Animation::time, or blend
// one float key. No hierarchy/world transform composition occurs here.
// Mod lookup:
// - channel edits: NodeTrackTag -> GtlPose byte lane below; serialized key layout
//   uses dataType, s_animationSamplingTables time/value offsets and interpolation table, plus
//   the stride table in gtl_Resource_GetDataTypeByteStride (gtl_image_decode_and_tiling.cpp).
// - playback time: self->time is in the resource keys' time units, not converted
//   seconds/frames here. Keep the producer and key times on the same scale.
// - endpoint policy: s_animationEndpointMode=0 clamps; 1 blends last -> first across a
//   synthetic interval of kAnimationWrapInterval (1 key-time unit). Other values have no
//   safe endpoint path: they pass null keys to interpolation. Not a free enum.
// - interpolation timing globals/caches: see the map beside their declarations;
//   this evaluator shares scratch with gtl_animation_model_matrices.cpp/gtl_animation_node_evaluation.cpp/gtl_animation_transform_channels.cpp/gtl_texture_animation_matrices.cpp.
// - matrix layout/mode: gtl_Transform_BuildNodeMatrix (gtl_transform_frustum_stopwatch_input.cpp), pose below.
//   Keep sample mode, channel tags and interpolation record width together.

#include "types.h"
#include "sdk_protos.h"

struct GtlAnimTrack {
    u8 unk00[2];
    u8 type;
    u8 unk03;
    u32 dataType;
    u8 unk08[2];
    u16 keyCount;
    u32 keyOffset;
};

struct GtlAnimNode {
    u8 unk00[6];
    u16 trackCount;
    u16 trackBase;
    u8 reserved0A[0x0E];
    u16 sampleIndex;
    u8 unk1A[0x16];
};

// 0x30-byte native pose, four-byte alignment. The matrix builder in gtl_transform_frustum_stopwatch_input.cpp
// proves the first lane is scale, and +20 is translation, despite older
// spellings pos/euler. Rotation is Euler XYZ (radians) until order=6 converts it
// to a quaternion. Translation is in the model's local length units; scale is
// dimensionless. Fourth scale word is padding; quaternion uses all four rot words.
struct GtlPose {
    float scale[4]; // +00
    float rot[4];
    float translation[3]; // +20
    u32 transformMode;    // +2C; mode 5 builds scale/Euler/translation, 6 quaternion transform.
};

// Tags are resource ordinals, not byte offsets or dataType ordinals. The 16-byte
// translation copy includes order; the evaluator saves/restores order afterwards.
enum NodeTrackTag {
    kTranslationZ = 0x12,
    kTranslationY = 0x13,
    kTranslationX = 0x14,
    kTranslationAndOrder = 0x15,
    kRotationZ = 0x16,
    kRotationY = 0x17,
    kRotationX = 0x18,
    kRotationXYZW = 0x19,
    kScaleZ = 0x1A,
    kScaleY = 0x1B,
    kScaleX = 0x1C,
    kScaleXYZW = 0x1D
};

typedef void (*GtlInterpFn)(void* dst, void* a, void* b, float alpha);
typedef u8* (*GtlFindKeyFn)(void* first, void* end, void* cache);

struct GtlAnimTables {
    u32 timeOffset[10];
    u32 valueOffset[10];
    GtlInterpFn interp[10];
};

struct GtlAnimation {
    GtlAnimNode* nodes;
    u32 nodeCount;
    u8* matrices;
    u8 unk0C[0x18];
    GtlAnimTrack* tracks;
    u8* keyData;
    GtlPose* samples;
    u32 sampleCount;
    float time;
};

extern "C" {

void gtl_Transform_BuildNodeMatrix(void* out, const void* pose);
void gtl_Transform_QuaternionFromEuler(float* out, float a, float b, float c);
u32 gtl_Resource_GetDataTypeByteStride(u32 dataType);
void Mtx44_Copy(void* dst, const void* src);

// Native tables indexed by dataType [0,10); no runtime validation. Tag 5 has
// stride zero and no cache initialization here; it is not a usable animated key
// type. Each used type needs valid time/value offsets, stride and callbacks.
extern GtlAnimTables s_animationSamplingTables;
extern GtlFindKeyFn s_animationKeySearchFunctions[10];
// Per-type search probe pointers. Lazy initialization uses the following BSS
// records; tag 9 points to s_animationKeyScratchType9 immediately after this pointer array.
// Search writes requested time at cache+timeOffset, then calls FindKey on the
// range excluding the first key. This is mutable shared scratch, not owned keys.
extern void* s_animationKeySearchProbes[10];
extern u8 s_animationKeyScratchType9[8];
extern u8 s_animationKeyScratchType1[0x10];
extern u8 s_animationKeyScratchType2[0x20];
extern u8 s_animationKeyScratchType3[0x10];
extern u8 s_animationKeyScratchType4[0x30];
extern u8 s_animationKeyScratchType6[0x20];
extern u8 s_animationKeyScratchType7[0x30];
extern u8 s_animationKeyScratchType8[0x40];
extern u8 s_animationKeyScratchType0[8];
extern s32 s_animationEndpointMode;              // endpoint mode: 0 clamp, 1 synthetic last-to-first blend.
extern s8 s_animationKeySearchProbesInitialized; // cache-pointer table initialized, published without locking.
extern float s_animationSegmentDuration;         // interval duration t1-t0.
extern float s_animationSegmentStartTime;        // left-key time t0.
extern float s_animationSampleTime;     // requested time t (Bezier consumers, gtl_animation_bezier_interpolate.cpp).
extern float s_animationSegmentEndTime; // right-key time t1 (same consumers).
extern float kFloat_0_GtlModel;
extern float kAnimationWrapInterval;

extern float kFloat_1_GtlModel;
// Blend required readable float keys into required dst; exact key aliasing is OK
// because both loads precede the store. alpha is dimensionless; [0,1] interpolates,
// outside extrapolates without clamping. Always returns 1. Preserve expression
// order, single precision and compiler fusion; kFloat_1_GtlModel is shared 1.0f.
s32 gtl_Animation_InterpolateLinearFloat(float* dst, const float* a, const float* b, float alpha) {
    float inv = kFloat_1_GtlModel - alpha;
    *dst = inv * *a + alpha * *b;
    return 1;
}

// Required: self, nodes[nodeIndex] (nodeIndex<nodeCount), writable nonaliasing
// 64-byte matrix out, and valid referenced sample/matrix/track/key extents.
// All are borrowed; no allocation/retention. Animated nodes require samples!=0
// and sampleIndex<sampleCount. Tracks need keyCount>=1, known NodeTrackTag,
// usable dataType, ascending key times and nonzero interpolated interval spans.
// Fallback when tracks OR keyData are absent: samples with sampleIndex<=sampleCount
// go to BuildNodeMatrix; otherwise copy matrix[sampleIndex]. The <= is original:
// equality is safe only with an extra allocated sample; ordinary arrays require <.
// nodes==0 enters that branch but still dereferences nodes: null is not supported.
// With all animation pointers present, sampleIndex>=sampleCount leaves base
// uninitialized before BuildNodeMatrix. No identity fallback/validation is implied.
// Modes 0..4 (and unsupported modes) in the matrix builder can leave out untouched;
// use resource mode 5/6 for a defined result. Always returns 1, not validity status.
// Serial animation-thread use only: key lookup and interpolation mutate shared
// cache/timing globals; reentrant/concurrent evaluation is not supported.
s32 gtl_Animation_EvaluateNodeLocalMatrix(GtlAnimation* self, u32 nodeIndex, void* out) {
    GtlPose base;
    GtlPose cur;
    const u32* valueOffsets;
    u32 trackBase;
    s32 order;
    u8* keyData;
    GtlAnimTrack* tracks;
    GtlAnimNode* node;
    GtlPose* samples;
    GtlAnimTables* tables = &s_animationSamplingTables;

    // Phase 1: select stored pose/matrix fallback when animation data is absent.
    if (self->nodes == 0 || self->tracks == 0 || self->keyData == 0) {
        if (self->samples != 0 && self->nodes[nodeIndex].sampleIndex <= self->sampleCount) {
            gtl_Transform_BuildNodeMatrix(out, self->samples + self->nodes[nodeIndex].sampleIndex);
        } else {
            Mtx44_Copy(out, self->matrices + (self->nodes[nodeIndex].sampleIndex << 6));
        }
        return 1;
    }

    // Phase 2: seed the pose, then overwrite only the channels present in tracks.
    if (self->nodes[nodeIndex].sampleIndex < self->sampleCount) {
        Memory_Copy(&base, self->samples + self->nodes[nodeIndex].sampleIndex, sizeof(GtlPose));

        if (self->nodes != 0 && (tracks = self->tracks) != 0 && (keyData = self->keyData) != 0 &&
            (samples = self->samples) != 0 && nodeIndex < self->nodeCount) {
            node = self->nodes + nodeIndex;
            float t = self->time;
            GtlPose* src = samples + node->sampleIndex;

            if (node->trackCount != 0) {
                trackBase = node->trackBase;
                Memory_Copy(&cur, src, sizeof(GtlPose));
                valueOffsets = tables->valueOffset;
                order = cur.transformMode;
                GtlInterpFn* interps = tables->interp;

                for (u32 i = 0; i < node->trackCount; i++) {
                    GtlAnimTrack* track = tracks + (trackBase + i);

                    void* dst = 0;
                    u32 size = 0;
                    switch (track->type) {
                    case kTranslationZ:
                        dst = &cur.translation[2];
                        size = 4;
                        break;
                    case kTranslationY:
                        dst = &cur.translation[1];
                        size = 4;
                        break;
                    case kTranslationX:
                        dst = &cur.translation[0];
                        size = 4;
                        break;
                    case kTranslationAndOrder:
                        dst = cur.translation;
                        size = 0x10;
                        break;
                    case kRotationZ:
                        dst = &cur.rot[2];
                        size = 4;
                        break;
                    case kRotationY:
                        dst = &cur.rot[1];
                        size = 4;
                        break;
                    case kRotationX:
                        dst = &cur.rot[0];
                        size = 4;
                        break;
                    case kRotationXYZW:
                        dst = cur.rot;
                        size = 0x10;
                        break;
                    case kScaleZ:
                        dst = &cur.scale[2];
                        size = 4;
                        break;
                    case kScaleY:
                        dst = &cur.scale[1];
                        size = 4;
                        break;
                    case kScaleX:
                        dst = &cur.scale[0];
                        size = 4;
                        break;
                    case kScaleXYZW:
                        dst = cur.scale;
                        size = 0x10;
                        break;
                    }

                    u8* keys = keyData + track->keyOffset;
                    u32 stride = gtl_Resource_GetDataTypeByteStride(track->dataType);
                    u32 tOff = tables->timeOffset[track->dataType];
                    bool key0Ge = *(float*)(keys + tOff) >= t;

                    if ((track->keyCount == 1 || key0Ge) && s_animationEndpointMode == 0) {
                        Memory_Copy(dst, keys + valueOffsets[(s32)track->dataType], size);
                        continue;
                    }

                    // Phase 3: clamp endpoints or choose the two keys and alpha.
                    u8* lastKey = keys + stride * (track->keyCount - 1);
                    bool afterLast = t >= *(float*)(lastKey + tOff);

                    if (afterLast && s_animationEndpointMode == 0) {
                        Memory_Copy(dst, lastKey + valueOffsets[(s32)track->dataType], size);
                        continue;
                    }

                    float t0 = kFloat_0_GtlModel;
                    float t1 = t0;
                    float alpha = t0;
                    u8* b = 0;
                    u8* a = 0;

                    if (key0Ge || afterLast) {
                        if (s_animationEndpointMode == 1) {
                            b = keys;
                            a = lastKey;
                            if (key0Ge) {
                                t1 = *(float*)(keys + tOff);
                                alpha = kAnimationWrapInterval;
                                t0 = t1 - alpha;
                                alpha = (t - t0) / alpha;
                            } else if (afterLast) {
                                t0 = *(float*)(lastKey + tOff);
                                alpha = kAnimationWrapInterval;
                                t1 = t0 + alpha;
                                alpha = (t - t0) / alpha;
                            }
                        }
                    } else {
                        if (s_animationKeySearchProbesInitialized == 0) {
                            s_animationKeySearchProbes[0] = s_animationKeyScratchType0;
                            s_animationKeySearchProbes[1] = s_animationKeyScratchType1;
                            s_animationKeySearchProbes[9] = (void*)((u32)&s_animationKeySearchProbes[0] + 0x28);
                            s_animationKeySearchProbes[2] = s_animationKeyScratchType2;
                            s_animationKeySearchProbes[3] = s_animationKeyScratchType3;
                            s_animationKeySearchProbes[4] = s_animationKeyScratchType4;
                            s_animationKeySearchProbes[6] = s_animationKeyScratchType6;
                            s_animationKeySearchProbes[7] = s_animationKeyScratchType7;
                            s_animationKeySearchProbes[8] = s_animationKeyScratchType8;
                            s_animationKeySearchProbesInitialized = 1;
                        }

                        u8* cache = (u8*)s_animationKeySearchProbes[track->dataType];
                        *(float*)(cache + tOff) = t;

                        u8* found = s_animationKeySearchFunctions[track->dataType](
                            keys + stride, keys + stride + stride * (track->keyCount - 1), cache);
                        if (found == 0) {
                            found = keys + stride * (track->keyCount - 1);
                        }
                        t1 = *(float*)(found + tOff);
                        if (t == t1) {
                            Memory_Copy(dst, found + valueOffsets[(s32)track->dataType], size);
                            continue;
                        }

                        u32 idx = (u32)(found - keys) / stride;
                        u8* prev = keys + stride * (idx - 1);
                        t0 = *(float*)(prev + tOff);
                        alpha = (t - t0) / (t1 - t0);
                        a = prev;
                        b = found;
                    }

                    // Phase 4: publish timing for interpolation callbacks (including
                    // Bezier), then write the selected pose lane. Endpoint copies
                    // above bypass these globals; their previous values remain.
                    s_animationSegmentStartTime = t0;
                    s_animationSampleTime = t;
                    s_animationSegmentDuration = t1 - t0;
                    s_animationSegmentEndTime = t1;
                    interps[(s32)track->dataType](dst, a, b, alpha);
                }

                // Phase 5: restore mode, convert Euler when required, build local matrix.
                cur.transformMode = order;
                switch (order) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    break;
                case 6:
                    gtl_Transform_QuaternionFromEuler(cur.rot, cur.rot[2], cur.rot[0], cur.rot[1]);
                    break;
                }
                Memory_Copy(&base, &cur, sizeof(GtlPose));
            }
        }
    }

    gtl_Transform_BuildNodeMatrix(out, &base);
    return 1;
}

} // extern "C"
