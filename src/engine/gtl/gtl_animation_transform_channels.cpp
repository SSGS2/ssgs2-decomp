// Gtl skeletal/model transform animation: sample resource keyframes into local
// poses, then gtl_transform_frustum_stopwatch_input.cpp builds matrices. Resource adapter: gtl_animation_resource_evaluate.cpp; consumers:
// CPetAnimation_EvaluateNamedTransforms and WPuppet bone sampling (wpuppet_model_bone_animation.cpp).
// Mod map: channel target = TransformChannelKind; stored key representation =
// Channel::keyDataType and gtl_Resource_GetDataTypeByteStride (gtl_image_decode_and_tiling.cpp); interpolation
// = s_animationSamplingTables tables, endpoint policy = s_animationEndpointMode. Time is absolute animation
// key time (WPuppet frame), not seconds or a delta. No playback-rate knob here.
// Original ABI/address: gtl_Animation_EvaluateModelTransformChannels, 0x803E33A8.
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

struct ResourceFile {
    u8 unk0[0xc];
    u8* base;
};

struct Chunk {
    u32 unk0;
    s32 count;
    u32 unk8;
    u32 offset;
};

struct TrackRec {
    u8 unk0[6];
    u16 channelCount;
    u16 channelStart;
    u8 reservedA[0xe];
    u16 index;
    u8 unk1A[0x16];
};

struct Channel {
    u8 unk0[2];
    u8 kind;
    u8 reserved3;
    u32 keyDataType;
    u8 unk8[2];
    u16 keyCount;
    u32 keyOffset;
};

struct TempRec {
    f32 scale[4];       // +00, xyz plus fourth lane
    f32 rotation[4];    // +10, Euler xyz or quaternion xyzw per mode
    f32 translation[3];// +20, local/model xyz, NOT world-space coordinates
    u32 mode;
};
typedef char TransformSample_size[(sizeof(TempRec) == 0x30) ? 1 : -1];

// Native PPC big-endian resource records: TrackRec 0x30, Channel 0x10.
// channelStart/count select contiguous Channel records; keyOffset is bytes into
// dk, keyCount must be >=1. type is a resource data tag, not channel kind:
// tags 0,1,2,3,4,6,7,8,9 have strides 8,16,32,16,48,32,48,64,96;
// tag 5 is unusable here (zero stride/null interpolation). No validation exists.
// time offsets by tag: 4,12,16,12,12,0,20,40,60,80;
// value offsets: 0,0,0,0,0,0,16,32,48,64 (s_animationSamplingTables +00/+28).
// Do not infer a C++ float-vector type from stride alone: tangents/key metadata
// share records. Key times must be sorted; selected adjacent times must differ.
enum TransformChannelKind {
    kTranslationZ = 0x12, kTranslationY, kTranslationX, kTranslationBlock,
    kRotationZ, kRotationY, kRotationX, kRotationBlock,
    kScaleZ, kScaleY, kScaleX, kScaleBlock
};
// Matching requires these five literal address words on the stack. They identify
// global query-key scratch for resource tags 6..9, then the search dispatch table;
// named enum values explain the literals without introducing new relocations.
enum TransformSearchAddress {
    kQueryKeyTag6Address = 0x805B1890, kQueryKeyTag7Address = 0x805B1860,
    kQueryKeyTag8Address = 0x805B1820, kQueryKeyTag9Address = 0x805B17C0,
    kKeySearchDispatchAddress = 0x804DA378
};

// The search functions take (first, last, key probe); the original leaves s_animationKeySearchProbes[type] in r5.
typedef void* (*SearchFn)(void* begin, void* end);
typedef void (*ChannelFn)(f32 alpha, void* dst, void* a, void* b);
// First 0x78 bytes of s_animationSamplingTables; remaining bytes are other evaluator tables.
// Search functions use global query-key scratch populated below. Interpolators
// receive alpha in FPR1 and {destination,lowerKey,upperKey} in GPR3..5.
struct TransformDispatchTables {
    u32 timeOffset[10];
    u32 valueOffset[10];
    ChannelFn interpolate[10];
};

extern "C" {

extern u8 s_animationSamplingTables[0x1B8];
extern SearchFn s_animationKeySearchFunctions[10];
extern u32 s_animationKeySearchProbes[10];
extern u8 s_animationKeyScratchType4[0x30];
extern u8 s_animationKeyScratchType3[0x10];
extern u8 s_animationKeyScratchType2[0x20];
extern u8 s_animationKeyScratchType1[0x10];
extern s32 s_animationEndpointMode;
extern s8 s_animationKeySearchProbesInitialized;
extern f32 s_animationSegmentDuration;
extern f32 s_animationSegmentStartTime;
extern f32 s_animationSampleTime;
extern f32 s_animationSegmentEndTime;
extern u8 s_animationKeyScratchType0[8];
extern f32 kAnimationWrapInterval;
extern f32 kFloat_0_GtlModel;

Chunk* gtl_ResourceFile_FindChunk(ResourceFile* res, u8* base, u32 tag);
void* gtl_ResourceFile_GetChunkElement(ResourceFile* res, u8* base, u32 tag, u32 index);
u32 gtl_Resource_GetDataTypeByteStride(u32 type);
void gtl_Transform_QuaternionFromEuler(void* out, f32 x, f32 y, f32 z);

// Required res borrows a live native resource/chunk tree: snrt (root track count),
// ldm (track descriptors), ft (default poses), vcf (channels), dk (key bytes).
// Chunk payload offsets here are relative to res; res->base is the lookup input.
// Missing ft returns 0 without touching out. snrt/ldm are dereferenced unchecked.
// count limits visited tracks, NOT output capacity: each visited rec.index <
// ft.count writes out[index], so out must contain at least ft.count*0x30 bytes
// (or cover every such selected index). Other output slots remain untouched.
// Require ldm to cover min(count,snrt.count), valid channel/key ranges, and out
// disjoint from resource data and global scratch. Zero count still needs chunks.
// Returns 1 after traversal, even when no channel was evaluated; no allocation.
// self is an unused ABI slot. Endpoint policy 0 copies first/last; 1 interpolates
// a one-key-time-unit last-to-first seam. Other policies are unsupported, unchecked.
// s_animationKeySearchProbes holds per-type query-key pointers, s_animationKeySearchProbesInitialized is lazy-init latch;
// s_animationSegmentStartTime/318/310/31C publish start/sample/duration/end for interpolators.
// All animation evaluators sharing these globals must run serialized, non-reentrant.
int gtl_Animation_EvaluateModelTransformChannels(void* self, f32 t, void* out, u32 count, ResourceFile* res)
{
    // Byte-based view of TransformDispatchTables: direct typed field access
    // changes MWCC's indirect-call instruction count and cannot be proven by
    // the gate. Offsets +00/+28/+50 are the three arrays in the schema above.
    u8* dispatchBytes = s_animationSamplingTables;
    Chunk* ldm;
    Chunk* ft;
    Chunk* root;
    Channel* channels;
    u8* ldmData;
    u8* ftData;
    u8* keyData;
    u32 ldmCount;
    u32 ftCount;
    u32 recOff;
    TempRec tmp;
    u32 i;

    root = gtl_ResourceFile_FindChunk(res, res->base, 0x736e7274);
    ldm = gtl_ResourceFile_FindChunk(res, res->base, 0x006c646d);
    ft = gtl_ResourceFile_FindChunk(res, res->base, 0x6674);
    if (ft == 0) {
        return 0;
    }
    // These retained lookups/volatile spills reproduce the original call and stack
    // sequence, including chunks unused by the local channel loop. Do not remove
    // them as dead work: that changes calls and the equivalence proof.
    void* volatile sink[33] = {0};
    sink[0] = (u8*)res + ldm->offset;
    sink[1] = (void*)(u32)ldm->count;
    sink[2] = gtl_ResourceFile_GetChunkElement(res, res->base, 0x736e7274, 0);
    sink[3] = gtl_ResourceFile_GetChunkElement(res, res->base, 0x727363, 0);
    sink[5] = gtl_ResourceFile_GetChunkElement(res, res->base, 0x726b69, 0);
    sink[6] = gtl_ResourceFile_GetChunkElement(res, res->base, 0x006a6b69, 0);
    sink[7] = gtl_ResourceFile_GetChunkElement(res, res->base, 0x656b69, 0);
    sink[8] = gtl_ResourceFile_GetChunkElement(res, res->base, 0x696a6b69, 0);
    sink[9] = gtl_ResourceFile_GetChunkElement(res, res->base, 0x766366, 0);
    keyData = (u8*)gtl_ResourceFile_GetChunkElement(res, res->base, 0x646b, 0);
    sink[10] = keyData;
    ftData = (u8*)res + ft->offset;
    sink[11] = ftData;
    ftCount = ft->count;
    sink[12] = (void*)ftCount;
    u32 volatile searchAddresses[5] = {kQueryKeyTag6Address, kQueryKeyTag7Address, kQueryKeyTag8Address, kQueryKeyTag9Address, kKeySearchDispatchAddress};
    ldmData = (u8*)sink[0];
    ldmCount = (u32)sink[1];
    channels = (Channel*)sink[9];

    recOff = 0;
    volatile f32 tSpill = t;
    for (i = 0; i < (u32)root->count; i++) {
        TrackRec* rec;
        u32 idx;
        u8* outRec;
        s32 mode;
        u32 k;

        if (i >= count) {
            break;
        }
        rec = (TrackRec*)(ldmData + recOff);
        idx = rec->index;
        outRec = (u8*)out + idx * 0x30;
        if (idx < ftCount) {
            Memory_Copy(outRec, ftData + idx * 0x30, 0x30);
            if (ldmData != 0 && channels != 0 && keyData != 0 && ftData != 0 && i < ldmCount) {
                f32 t2 = tSpill;
                TrackRec* rec2 = (TrackRec*)(ldmData + recOff);
                if (rec2->channelCount != 0) {
                    Memory_Copy(&tmp, ftData + rec->index * 0x30, 0x30);
                    mode = tmp.mode;
                    u32 start = rec2->channelStart;
                    for (k = 0; k < rec2->channelCount; k++) {
                        Channel* ch = channels + start + k;
                        u32 size = 0;
                        void* dst = 0;
                        u32 stride;
                        u8* keys;
                        u32 timeOff;
                        f32 first;
                        f32 lastTime;
                        f32 alpha;
                        f32 upperTime;
                        f32 lowerTime;
                        void* lowerKey = 0;
                        u8* upperKey = 0;
                        u8* lastKey;

                        switch (ch->kind) {
                        case kTranslationZ:
                            dst = (u8*)&tmp + 0x28;
                            size = 4;
                            break;
                        case kTranslationY:
                            dst = (u8*)&tmp + 0x24;
                            size = 4;
                            break;
                        case kTranslationX:
                            dst = (u8*)&tmp + 0x20;
                            size = 4;
                            break;
                        case kTranslationBlock:
                            dst = (u8*)&tmp + 0x20;
                            size = 0x10;
                            break;
                        case kRotationZ:
                            dst = (u8*)&tmp + 0x18;
                            size = 4;
                            break;
                        case kRotationY:
                            dst = (u8*)&tmp + 0x14;
                            size = 4;
                            break;
                        case kRotationX:
                            dst = (u8*)&tmp + 0x10;
                            size = 4;
                            break;
                        case kRotationBlock:
                            dst = (u8*)&tmp + 0x10;
                            size = 0x10;
                            break;
                        case kScaleZ:
                            dst = (u8*)&tmp + 0x8;
                            size = 4;
                            break;
                        case kScaleY:
                            dst = (u8*)&tmp + 0x4;
                            size = 4;
                            break;
                        case kScaleX:
                            dst = (u8*)&tmp;
                            size = 4;
                            break;
                        case kScaleBlock:
                            dst = (u8*)&tmp;
                            size = 0x10;
                            break;
                        }

                        keys = keyData + ch->keyOffset;
                        stride = gtl_Resource_GetDataTypeByteStride(ch->keyDataType);
                        timeOff = ((u32*)dispatchBytes)[ch->keyDataType];
                        first = *(f32*)(keys + timeOff);
                        u32 firstGe = (first >= t);

                        if (ch->keyCount == 1 || firstGe) {
                            if (s_animationEndpointMode == 0) {
                                Memory_Copy(dst, keys + ((u32*)(dispatchBytes + 0x28))[ch->keyDataType], size);
                                do { } while (0);
                                continue;
                            }
                        }

                        lastKey = keys + stride * (ch->keyCount - 1);
                        lastTime = *(f32*)(lastKey + timeOff);
                        u32 lastGe = (t >= lastTime);
                        if (lastGe) {
                            if (s_animationEndpointMode == 0) {
                                Memory_Copy(dst, lastKey + ((u32*)(dispatchBytes + 0x28))[ch->keyDataType], size);
                                do { } while (0);
                                continue;
                            }
                        }

                        alpha = kFloat_0_GtlModel;
                        upperTime = kFloat_0_GtlModel;
                        lowerTime = kFloat_0_GtlModel;
                        if (firstGe || lastGe) {
                            if (s_animationEndpointMode == 1) {
                                upperKey = keys;
                                lowerKey = lastKey;
                                if (firstGe) {
                                    upperTime = first;
                                    lowerTime = first - kAnimationWrapInterval;
                                    alpha = (t - lowerTime) / kAnimationWrapInterval;
                                } else if (lastGe) {
                                    lowerTime = lastTime;
                                    upperTime = lastTime + kAnimationWrapInterval;
                                    alpha = (t - lowerTime) / kAnimationWrapInterval;
                                }
                            }
                        } else {
                            u8* begin;
                            void* found;
                            u8* seg;

                            if (s_animationKeySearchProbesInitialized == 0) {
                                s_animationKeySearchProbes[0] = (u32)s_animationKeyScratchType0;
                                s_animationKeySearchProbes[1] = (u32)s_animationKeyScratchType1;
                                s_animationKeySearchProbes[2] = (u32)s_animationKeyScratchType2;
                                s_animationKeySearchProbes[3] = (u32)s_animationKeyScratchType3;
                                s_animationKeySearchProbes[4] = (u32)s_animationKeyScratchType4;
                                s_animationKeySearchProbes[6] = searchAddresses[0];
                                s_animationKeySearchProbes[7] = searchAddresses[1];
                                s_animationKeySearchProbes[8] = searchAddresses[2];
                                s_animationKeySearchProbes[9] = searchAddresses[3];
                                s_animationKeySearchProbesInitialized = 1;
                            }

                            begin = keys + stride;
                            *(f32*)(s_animationKeySearchProbes[ch->keyDataType] + timeOff) = t;
                            found = ((SearchFn*)searchAddresses[4])[ch->keyDataType](begin, begin + stride * (ch->keyCount - 1));
                            seg = (u8*)found;
                            if (found == 0) {
                                seg = lastKey;
                            }
                            upperTime = *(f32*)(seg + timeOff);
                            if (t == upperTime) {
                                Memory_Copy(dst, seg + ((u32*)(dispatchBytes + 0x28))[ch->keyDataType], size);
                                do { } while (0);
                                continue;
                            }
                            {
                                u32 j = (u32)(seg - keys) / stride;
                                u8* prev = keys + (j - 1) * stride;

                                lowerTime = *(f32*)(prev + timeOff);
                                alpha = (t - lowerTime) / (upperTime - lowerTime);
                                lowerKey = prev;
                                upperKey = seg;
                            }
                        }

                        s_animationSegmentStartTime = lowerTime;
                        s_animationSampleTime = t;
                        s_animationSegmentDuration = upperTime - lowerTime;
                        s_animationSegmentEndTime = upperTime;
                        ((ChannelFn*)(dispatchBytes + 0x50))[ch->keyDataType](alpha, dst, lowerKey, upperKey);
                    }
                    // TranslationBlock copies 16 bytes including the mode word;
                    // restore the default's mode before publishing the pose.
                    tmp.mode = mode;
                    if (mode == 6) {
                        // Euler input is radians, argument order z,x,y; conversion
                        // writes quaternion xyzw into the same four rotation lanes.
                        gtl_Transform_QuaternionFromEuler(&tmp.rotation[0], tmp.rotation[2], tmp.rotation[0], tmp.rotation[1]);
                    }
                    Memory_Copy(outRec, &tmp, 0x30);
                }
            }
        }
        recOff += 0x30;
    }
    return 1;
}

} // extern "C"
