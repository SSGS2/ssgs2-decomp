// GTL model animation: evaluate a node and its ancestors into world matrices,
// sample transform channels and optionally face billboards toward a direction.
// Mod index: channel destination -> AnimationChannel below; endpoint policy ->
// s_animationEndpointMode; key encoding -> dataType-indexed tables s_animationSamplingTables/804DA378;
// pose interpretation -> GtlPose and gtl_Transform_BuildNodeMatrix (gtl_transform_frustum_stopwatch_input.cpp).
// The same sampling globals/tables are used by gtl_animation_transform_channels.cpp: edits affect both.
// No frame-rate conversion here: time/key times and the wrap interval use the
// animation's own common timebase, whose seconds/ticks conversion is not known.

#include "types.h"
#include "sdk_protos.h"

// Native 16-byte channel record. type chooses the destination; dataType is a
// distinct encoding/table index [0,9] with nonnull stride/search/interp entries.
// keyCount must be >=1; keyOffset is bytes from keyData, not a key ordinal.
// Key times must be sorted; distinct bracketing times avoid division by zero.
// dataType remains an opaque format ID, not a channel or image-format enum:
// ID       0   1   2   3   4  5   6   7   8   9
// stride   8  16  32  16  48  0  32  48  64  96 bytes (gtl_image_decode_and_tiling.cpp).
// ID 5 is unusable here (zero stride and uninitialized search-cache slot).
// timeOffset/valueOffset locate fields inside each encoding; search/interp tables
// select its implementation. Preserve the ID mapping and all four tables together
// when adding an encoding; assigning an invented semantic type would be unsafe.
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
    u16 parent;
    u8 unk0C[0x0C];
    u16 sampleIndex;
    u8 unk1A[2];
    u16 billboard;
    u8 unk1E[0x12];
};

// 0x30-byte transform payload passed directly to gtl_transform_frustum_stopwatch_input.cpp's GtlNodeTransform.
// Scale occupies +00; translation occupies +20; rotation is the separate +10 lane.
// XYZ are model-local; scale is dimensionless, translation uses model units.
// +0C/+1C are fourth-lane storage copied by vector channels. A 16-byte translation
// channel also copies +2C, but order is saved/restored around the track loop.
struct GtlPose {
    float scale[4];       // scale XYZ + fourth word
    float rot[4];         // rotation payload, interpreted according to transformMode
    float translation[3]; // translation XYZ
    u32 transformMode;    // transform mode, not an Euler-axis permutation
};

enum AnimationChannel {
    kTranslationZ = 0x12,
    kTranslationY = 0x13,
    kTranslationX = 0x14,
    kTranslationVector = 0x15,
    kRotationZ = 0x16,
    kRotationY = 0x17,
    kRotationX = 0x18,
    kRotationVector = 0x19,
    kScaleZ = 0x1A,
    kScaleY = 0x1B,
    kScaleX = 0x1C,
    kScaleVector = 0x1D
};
enum AnimationModes {
    kNoParent = 0xFFFF,
    kClampKeys = 0,
    kWrapKeys = 1,
    kQuaternionTransform = 6
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
    u8 unk0C[4];
    u8* billboards;
    u8 unk14[0x10];
    GtlAnimTrack* tracks;
    u8* keyData;
    GtlPose* samples;
    u32 sampleCount;
    float time;
    u8* rootToCameraDirection;
    u8 unk3C[4];
    u8* direction;
    float localMtx[16];
};

extern "C" {

void gtl_Transform_BuildNodeMatrix(void* out, const void* pose);
void gtl_Transform_QuaternionFromEuler(float* out, float a, float b, float c);
void Vec4_Normalize(void* v, void* v2);
void Vec3_Cross(void* dst, void* a, void* b);
u32 gtl_Resource_GetDataTypeByteStride(u32 dataType);
void Mtx44_Copy(void* dst, const void* src);
s32 gtl_Animation_EvaluateNodeLocalMatrix(GtlAnimation* self, u32 nodeIndex, void* out);
s32 gtl_Animation_EvaluateNodeWorldMatrix(GtlAnimation* self, u32 nodeIndex, void* out, u32* flags);
// The default direction for types 0/1 is self->rootToCameraDirection (the root-to-camera vector) in r4; every type takes the
// billboard entry in r5.
void gtl_Billboard_RotateDirection(float* out, const float* dir = 0);
void gtl_Billboard_BuildYAxisBasis(float* out, const float* in);
void Matrix_AlignForwardPreservingScaleAndTranslationWrapper(void* dst, const void* src, const void* dir, s32 flag);
void PSMTX44Concat(void* dst, const void* a, const void* b);

extern GtlAnimTables s_animationSamplingTables;
extern GtlFindKeyFn s_animationKeySearchFunctions[10];
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
// Global endpoint mode: 0 copies first/last values; 1 interpolates last -> first
// across kAnimationWrapInterval (1.0 in the same timebase) outside the range. Other values are unsupported: outside
// the key span the interpolator receives null endpoints. This is not modulo time.
extern s32 s_animationEndpointMode;
extern s8 s_animationKeySearchProbesInitialized;
extern float s_animationSegmentDuration;  // duration t1-t0, published before interpolation
extern float s_animationSegmentStartTime; // selected lower endpoint time t0
extern float s_animationSampleTime;       // requested sample time t
extern float s_animationSegmentEndTime;   // selected upper endpoint time t1
extern float kFloat_0_GtlModel;
extern float kAnimationWrapInterval;
extern float kUnitYAxisVec4[4];

// Replace basis rows using the shared Y row, normalized XZ direction and cross
// product; retain other matrix words. Direction must have nonzero XZ magnitude.
static inline void BillboardBuildYAxisBasisInline(float* m, const float* dir) {
    float zero = kFloat_0_GtlModel;
    m[4] = kUnitYAxisVec4[0];
    m[5] = kUnitYAxisVec4[1];
    m[6] = kUnitYAxisVec4[2];
    m[7] = kUnitYAxisVec4[3];
    m[8] = dir[0];
    m[9] = zero;
    m[10] = dir[2];
    m[11] = zero;
    Vec4_Normalize(m + 8, m + 8);
    Vec3_Cross(m, m + 4, m + 8);
}

// Required live self/nodes and valid nodeIndex; parent chains must be acyclic,
// every parent index valid, every sampleIndex backed by a 64-byte matrix slot
// and a u32 flag. out/flags must not alias inputs or each other. They are borrowed;
// no allocation/retention occurs. flags!=0 reuses the cached world matrix without
// inspecting time: caller must clear the ENTIRE cache after time/pose/direction
// changes. Each computed ancestor/target slot gets flag=1; other slots survive.
// Returns 1 even for cache hits; no recoverable error status or rollback exists.
// Missing track/key arrays selects static poses/matrices, not a validation error.
// The static path accepts sampleIndex<=sampleCount while animated uses <; do not
// rely on this boundary as an array bounds check. An animated out-of-range sample
// can pass uninitialized pose storage to the builder; valid assets must avoid it.
// Local sampling is duplicated for root/child to preserve numeric evaluation and
// compiler lifetimes. Child world = the existing concat(local,parentWorld) call;
// Billboard records are 0x18 bytes: type 0/1 use default direction, 2/3 use
// direction+20; even types replace Y-axis basis, odd types align preserving scale
// and translation. rootToCameraDirection plus billboards gate this pass. Shared search scratch
// and interpolation globals make evaluation non-reentrant; serialize calls.
s32 gtl_Animation_EvaluateNodeWorldMatrix(GtlAnimation* self, u32 nodeIndex, void* out, u32* flags) {
    u32 nodeOff = nodeIndex * 0x30;
    GtlAnimNode* node = (GtlAnimNode*)((u8*)self->nodes + nodeOff);
    GtlAnimTables* tables = &s_animationSamplingTables;
    GtlAnimNode* pnode;
    GtlPose cur;
    GtlPose base;
    GtlPose cur2;
    GtlPose base2;
    float dirN[3];
    float dirP[3];

    // Phase 1 (root): reuse cache or sample a local transform directly to out.
    if (node->parent == kNoParent) {
        if (flags[node->sampleIndex] != 0) {
            return 1;
        }
        u8* dst = (u8*)out + (node->sampleIndex << 6);

        if (self->nodes == 0 || self->tracks == 0 || self->keyData == 0) {
            GtlAnimNode* nodeA = (GtlAnimNode*)((u8*)self->nodes + nodeOff);
            if (self->samples != 0 && nodeA->sampleIndex <= self->sampleCount) {
                gtl_Transform_BuildNodeMatrix(dst, self->samples + nodeA->sampleIndex);
            } else {
                Mtx44_Copy(dst, self->matrices + (nodeA->sampleIndex << 6));
            }
        } else {
            GtlAnimNode* nodeA = (GtlAnimNode*)((u8*)self->nodes + nodeOff);
            if (nodeA->sampleIndex < self->sampleCount) {
                Memory_Copy(&base, self->samples + nodeA->sampleIndex, sizeof(GtlPose));

                if (self->nodes != 0 && self->tracks != 0 && self->keyData != 0 && self->samples != 0 &&
                    nodeIndex < self->nodeCount && nodeA->trackCount != 0) {
                    float t = self->time;
                    u32 trackBase = nodeA->trackBase;
                    Memory_Copy(&cur, self->samples + nodeA->sampleIndex, sizeof(GtlPose));
                    u32* valueOffset = tables->valueOffset;
                    GtlInterpFn* interpBase = tables->interp;
                    u32* timeOffset = tables->timeOffset;
                    s32 order = cur.transformMode;

                    for (u32 i = 0; i < nodeA->trackCount; i++) {
                        GtlAnimTrack* track = self->tracks + (trackBase + i);

                        u32 size = 0;
                        void* slot = 0;
                        switch (track->type) {
                        case kTranslationZ:
                            slot = &cur.translation[2];
                            size = 4;
                            break;
                        case kTranslationY:
                            slot = &cur.translation[1];
                            size = 4;
                            break;
                        case kTranslationX:
                            slot = &cur.translation[0];
                            size = 4;
                            break;
                        case kTranslationVector:
                            slot = cur.translation;
                            size = 0x10;
                            break;
                        case kRotationZ:
                            slot = &cur.rot[2];
                            size = 4;
                            break;
                        case kRotationY:
                            slot = &cur.rot[1];
                            size = 4;
                            break;
                        case kRotationX:
                            slot = &cur.rot[0];
                            size = 4;
                            break;
                        case kRotationVector:
                            slot = cur.rot;
                            size = 0x10;
                            break;
                        case kScaleZ:
                            slot = &cur.scale[2];
                            size = 4;
                            break;
                        case kScaleY:
                            slot = &cur.scale[1];
                            size = 4;
                            break;
                        case kScaleX:
                            slot = &cur.scale[0];
                            size = 4;
                            break;
                        case kScaleVector:
                            slot = cur.scale;
                            size = 0x10;
                            break;
                        }

                        u8* keys = self->keyData + track->keyOffset;
                        u32 stride = gtl_Resource_GetDataTypeByteStride(track->dataType);
                        u32 tOff = timeOffset[track->dataType];
                        bool key0Ge = *(float*)(keys + tOff) >= t;

                        if ((track->keyCount == 1 || key0Ge) && s_animationEndpointMode == kClampKeys) {
                            Memory_Copy(slot, keys + valueOffset[track->dataType], size);
                            continue;
                        }

                        u8* lastKey = keys + stride * (track->keyCount - 1);
                        bool afterLast = t >= *(float*)(lastKey + tOff);

                        if (afterLast && s_animationEndpointMode == kClampKeys) {
                            Memory_Copy(slot, lastKey + valueOffset[track->dataType], size);
                            continue;
                        }

                        float t0 = kFloat_0_GtlModel;
                        float t1 = t0;
                        float alpha = t0;
                        u8* a = 0;
                        u8* b = 0;

                        if (key0Ge || afterLast) {
                            if (s_animationEndpointMode == kWrapKeys) {
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
                                s_animationKeySearchProbes[9] = s_animationKeyScratchType9;
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
                                found = lastKey;
                            }
                            t1 = *(float*)(found + tOff);
                            if (t == t1) {
                                Memory_Copy(slot, found + valueOffset[track->dataType], size);
                                continue;
                            }

                            u32 idx = (u32)(found - keys) / stride;
                            u8* prev = keys + stride * (idx - 1);
                            t0 = *(float*)(prev + tOff);
                            alpha = (t - t0) / (t1 - t0);
                            a = prev;
                            b = found;
                        }

                        s_animationSegmentStartTime = t0;
                        s_animationSampleTime = t;
                        s_animationSegmentDuration = t1 - t0;
                        s_animationSegmentEndTime = t1;
                        interpBase[track->dataType](slot, a, b, alpha);
                    }

                    cur.transformMode = order;
                    if (order == kQuaternionTransform) {
                        gtl_Transform_QuaternionFromEuler(cur.rot, cur.rot[2], cur.rot[0], cur.rot[1]);
                    }
                    Memory_Copy(&base, &cur, sizeof(GtlPose));
                }
            }
            gtl_Transform_BuildNodeMatrix(dst, &base);
        }
    } else {
        // Phase 2 (child): compute parent/ancestors before sampling this node.
        pnode = self->nodes + node->parent;
        u32 ps = pnode->sampleIndex;

        if (pnode->parent == kNoParent) {
            if (flags[ps] == 0) {
                gtl_Animation_EvaluateNodeLocalMatrix(self, node->parent, (u8*)out + (ps << 6));

                if (self->rootToCameraDirection != 0 && self->billboards != 0) {
                    float* m = (float*)((u8*)out + (ps << 6));
                    s32 bbType = *(s32*)(self->billboards + pnode->billboard * 0x18);
                    switch (bbType) {
                    case 0:
                        gtl_Billboard_RotateDirection(dirP);
                        gtl_Billboard_BuildYAxisBasis(m, dirP);
                        break;
                    case 1:
                        gtl_Billboard_RotateDirection(dirP);
                        Matrix_AlignForwardPreservingScaleAndTranslationWrapper(m, m, dirP, 1);
                        break;
                    case 2:
                        gtl_Billboard_RotateDirection(dirP, (float*)(self->direction + 0x20));
                        gtl_Billboard_BuildYAxisBasis(m, dirP);
                        break;
                    case 3:
                        gtl_Billboard_RotateDirection(dirP, (float*)(self->direction + 0x20));
                        Matrix_AlignForwardPreservingScaleAndTranslationWrapper(m, m, dirP, 1);
                        break;
                    }
                }
                flags[ps] = 1;
            }
        } else {
            GtlAnimNode* gnode = self->nodes + pnode->parent;
            gtl_Animation_EvaluateNodeWorldMatrix(self, pnode->parent, out, flags);

            if (flags[ps] == 0) {
                gtl_Animation_EvaluateNodeLocalMatrix(self, node->parent, self->localMtx);
                PSMTX44Concat((u8*)out + (ps << 6), self->localMtx, (u8*)out + (gnode->sampleIndex << 6));

                if (self->rootToCameraDirection != 0 && self->billboards != 0) {
                    float* m = (float*)((u8*)out + (ps << 6));
                    s32 bbType = *(s32*)(self->billboards + pnode->billboard * 0x18);
                    switch (bbType) {
                    case 0:
                        gtl_Billboard_RotateDirection(dirP);
                        gtl_Billboard_BuildYAxisBasis(m, dirP);
                        break;
                    case 1:
                        gtl_Billboard_RotateDirection(dirP);
                        Matrix_AlignForwardPreservingScaleAndTranslationWrapper(m, m, dirP, 1);
                        break;
                    case 2:
                        gtl_Billboard_RotateDirection(dirP, (float*)(self->direction + 0x20));
                        gtl_Billboard_BuildYAxisBasis(m, dirP);
                        break;
                    case 3:
                        gtl_Billboard_RotateDirection(dirP, (float*)(self->direction + 0x20));
                        Matrix_AlignForwardPreservingScaleAndTranslationWrapper(m, m, dirP, 1);
                        break;
                    }
                }
                flags[ps] = 1;
            }
        }

        if (flags[node->sampleIndex] != 0) {
            return 1;
        }

        if (self->nodes == 0 || self->tracks == 0 || self->keyData == 0) {
            GtlAnimNode* nodeB = (GtlAnimNode*)((u8*)self->nodes + nodeOff);
            if (self->samples != 0 && nodeB->sampleIndex <= self->sampleCount) {
                gtl_Transform_BuildNodeMatrix(self->localMtx, self->samples + nodeB->sampleIndex);
            } else {
                Mtx44_Copy(self->localMtx, self->matrices + (nodeB->sampleIndex << 6));
            }
        } else {
            GtlAnimNode* nodeB = (GtlAnimNode*)((u8*)self->nodes + nodeOff);
            if (nodeB->sampleIndex < self->sampleCount) {
                Memory_Copy(&base2, self->samples + nodeB->sampleIndex, sizeof(GtlPose));

                if (self->nodes != 0 && self->tracks != 0 && self->keyData != 0 && self->samples != 0 &&
                    nodeIndex < self->nodeCount && nodeB->trackCount != 0) {
                    float t = self->time;
                    u32 trackBase = nodeB->trackBase;
                    Memory_Copy(&cur2, self->samples + nodeB->sampleIndex, sizeof(GtlPose));
                    u32* valueOffset = tables->valueOffset;
                    GtlInterpFn* interpBase = tables->interp;
                    u32* timeOffset = tables->timeOffset;
                    s32 order = cur2.transformMode;

                    for (u32 i = 0; i < nodeB->trackCount; i++) {
                        GtlAnimTrack* track = self->tracks + (trackBase + i);

                        u32 size = 0;
                        void* slot = 0;
                        switch (track->type) {
                        case kTranslationZ:
                            slot = &cur2.translation[2];
                            size = 4;
                            break;
                        case kTranslationY:
                            slot = &cur2.translation[1];
                            size = 4;
                            break;
                        case kTranslationX:
                            slot = &cur2.translation[0];
                            size = 4;
                            break;
                        case kTranslationVector:
                            slot = cur2.translation;
                            size = 0x10;
                            break;
                        case kRotationZ:
                            slot = &cur2.rot[2];
                            size = 4;
                            break;
                        case kRotationY:
                            slot = &cur2.rot[1];
                            size = 4;
                            break;
                        case kRotationX:
                            slot = &cur2.rot[0];
                            size = 4;
                            break;
                        case kRotationVector:
                            slot = cur2.rot;
                            size = 0x10;
                            break;
                        case kScaleZ:
                            slot = &cur2.scale[2];
                            size = 4;
                            break;
                        case kScaleY:
                            slot = &cur2.scale[1];
                            size = 4;
                            break;
                        case kScaleX:
                            slot = &cur2.scale[0];
                            size = 4;
                            break;
                        case kScaleVector:
                            slot = cur2.scale;
                            size = 0x10;
                            break;
                        }

                        u8* keys = self->keyData + track->keyOffset;
                        u32 stride = gtl_Resource_GetDataTypeByteStride(track->dataType);
                        u32 tOff = timeOffset[track->dataType];
                        bool key0Ge = *(float*)(keys + tOff) >= t;

                        if ((track->keyCount == 1 || key0Ge) && s_animationEndpointMode == kClampKeys) {
                            Memory_Copy(slot, keys + valueOffset[track->dataType], size);
                            continue;
                        }

                        u8* lastKey = keys + stride * (track->keyCount - 1);
                        bool afterLast = t >= *(float*)(lastKey + tOff);

                        if (afterLast && s_animationEndpointMode == kClampKeys) {
                            Memory_Copy(slot, lastKey + valueOffset[track->dataType], size);
                            continue;
                        }

                        float t0 = kFloat_0_GtlModel;
                        float t1 = t0;
                        float alpha = t0;
                        u8* a = 0;
                        u8* b = 0;

                        if (key0Ge || afterLast) {
                            if (s_animationEndpointMode == kWrapKeys) {
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
                                s_animationKeySearchProbes[9] = s_animationKeyScratchType9;
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
                                found = lastKey;
                            }
                            t1 = *(float*)(found + tOff);
                            if (t == t1) {
                                Memory_Copy(slot, found + valueOffset[track->dataType], size);
                                continue;
                            }

                            u32 idx = (u32)(found - keys) / stride;
                            u8* prev = keys + stride * (idx - 1);
                            t0 = *(float*)(prev + tOff);
                            alpha = (t - t0) / (t1 - t0);
                            a = prev;
                            b = found;
                        }

                        s_animationSegmentStartTime = t0;
                        s_animationSampleTime = t;
                        s_animationSegmentDuration = t1 - t0;
                        s_animationSegmentEndTime = t1;
                        interpBase[track->dataType](slot, a, b, alpha);
                    }

                    cur2.transformMode = order;
                    if (order == kQuaternionTransform) {
                        gtl_Transform_QuaternionFromEuler(cur2.rot, cur2.rot[2], cur2.rot[0], cur2.rot[1]);
                    }
                    Memory_Copy(&base2, &cur2, sizeof(GtlPose));
                }
                gtl_Transform_BuildNodeMatrix(self->localMtx, &base2);
            } else {
                gtl_Transform_BuildNodeMatrix(self->localMtx, &base2);
            }
        }
        PSMTX44Concat((u8*)out + (node->sampleIndex << 6), self->localMtx, (u8*)out + (ps << 6));
    }

    // Phase 3: adjust target billboard once, then publish its cache flag.
    if (self->rootToCameraDirection != 0 && self->billboards != 0) {
        float* m = (float*)((u8*)out + (node->sampleIndex << 6));
        s32 bbType = *(s32*)(self->billboards + node->billboard * 0x18);
        switch (bbType) {
        case 0:
            gtl_Billboard_RotateDirection(dirN);
            BillboardBuildYAxisBasisInline(m, dirN);
            break;
        case 1:
            gtl_Billboard_RotateDirection(dirN);
            Matrix_AlignForwardPreservingScaleAndTranslationWrapper(m, m, dirN, 1);
            break;
        case 2:
            gtl_Billboard_RotateDirection(dirN, (float*)(self->direction + 0x20));
            BillboardBuildYAxisBasisInline(m, dirN);
            break;
        case 3:
            gtl_Billboard_RotateDirection(dirN, (float*)(self->direction + 0x20));
            Matrix_AlignForwardPreservingScaleAndTranslationWrapper(m, m, dirN, 1);
            break;
        }
    }
    flags[node->sampleIndex] = 1;
    return 1;
}

} // extern "C"
