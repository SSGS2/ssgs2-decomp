// GTL model animation: compose node matrices and orient billboard nodes toward
// the camera. Entry: gtl_Animation_EvaluateModelMatrices; local/world evaluators
// are in gtl_animation_node_evaluate.cpp/gtl_animation_node_evaluation.cpp, transform consumers in gtl_transform_frustum_stopwatch_input.cpp. This is model
// pose evaluation, not the UI keyframe clock or golf simulation frame rate.
// Mod lookup:
// - Move/scale/rotate a model node: GtlPose below and BuildNodeMatrix's mode 5/6.
// - Change a track target: GtlTrackTarget below; preserve 0x10-byte record copies.
// - Change key format: dataType, stride table in gtl_image_decode_and_tiling.cpp and time/value/interp
//   tables s_animationSamplingTables plus search table s_animationKeySearchFunctions must agree.
// - Change camera-facing behavior: billboard cases 0..3 in the final two phases.
// Resource chunks are trusted native-endian Wii records: 'mdl' node hierarchy,
// 'trns' base matrices (numeric tag 0x736E7274), 'bill' camera-facing entries.
// Chunk offsets are relative to file; track keyOffset is relative to keyData.
// Billboard mode: 0 camera-position direction with fixed Y axis; 1 same
// direction with scale/translation-preserving alignment; 2 camera forward with
// fixed Y axis; 3 camera forward with preserving alignment. Unknown modes leave
// the composed basis unchanged. The root influences the camera-position vector;
// it is not concatenated into every node by this function.
// This entry initializes anim.tracks/keyData/time to zero and never installs
// tracks. Thus its normal path uses supplied poses or stored matrices; the
// inlined track evaluator documents the common format used by sibling APIs.
// Translation unit 0x803E14D0-0x803E251C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "sdk_protos.h"

struct GtlChunk {
    u32 tag;
    u32 count;
    u32 reserved08;
    u32 offset;
};

// 0x10-byte track: type +2 must be a supported GtlTrackTarget (0x12..0x1D);
// it selects a pose destination. dataType +4 is a table
// ordinal 0..9, EXCLUDING 5 (zero stride and no initialized search cache).
// keyCount +A must be >=1; keyOffset +C addresses sorted key records. All used
// table entries and the complete key range must exist; no validation is done.
struct GtlAnimTrack {
    u8 unk00[2];
    u8 type;
    u8 unk03;
    u32 dataType;
    u8 unk08[2];
    u16 keyCount;
    u32 keyOffset;
};

// 0x30-byte node: [trackBase,trackBase+trackCount) indexes the track array;
// parent is a node ordinal or 0xFFFF (root); sampleIndex selects a pose/matrix
// slot, not necessarily this node's ordinal. billIndex selects a 0x18-byte bill
// record whose first u32 is mode 0..3. Parent links must form an acyclic tree.
struct GtlAnimNode {
    u8 unk00[6];
    u16 trackCount;
    u16 trackBase;
    u16 parent;
    u8 unk0C[0x0C];
    u16 sampleIndex;
    u8 unk1A[2];
    u16 billIndex;
    u8 unk1E[0x12];
};

// 0x30-byte LOCAL transform; named fields agree with the matrix builder.
// Evidence: GtlNodeTransform / gtl_Transform_BuildNodeMatrix in gtl_transform_frustum_stopwatch_input.cpp.
struct GtlPose {
    float scale[4];    // +00: XYZ scale (dimensionless); +C padding/lane
    float rot[4];    // +10: XYZ Euler radians for mode 5; XYZW quaternion for 6
    float translation[3];  // +20: XYZ translation in model-authoring units
    u32 transformMode;      // +2C: transform mode, not an axis-order permutation
};

// Reverse component numbering is serialized format policy, not axis swapping.
// ORIGINAL channel-kind numbering (proven by the six jump tables jumptable_804DA3A0/3D0/400/430/460/490): kinds
// 0x12-0x14 scale X,Y,Z, 0x15 scale block, 0x16-0x18 rotation X,Y,Z, 0x19 rotation block, 0x1A-0x1C translation
// X,Y,Z, 0x1D translation block. The Wii enum below keeps the decompiler's code-block order (translation first); its
// values must NOT be renumbered: the case labels build the switch while the table bytes are anchored data.
enum GtlTrackTarget {
    kTranslationZ = 0x12, kTranslationY = 0x13, kTranslationX = 0x14,
    kTranslationRecord = 0x15,
    kRotationZ = 0x16, kRotationY = 0x17, kRotationX = 0x18,
    kRotationRecord = 0x19,
    kScaleZ = 0x1A, kScaleY = 0x1B, kScaleX = 0x1C, kScaleRecord = 0x1D
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
    void* bill;
    void* ikChunkElements[4];
    GtlAnimTrack* tracks;
    u8* keyData;
    GtlPose* samples;
    u32 sampleCount;
    float time;
    float* rootToCameraDirection;
    float* cameraPosition;
    void* inverseView;
};

// A pose is 0x30 bytes of scale/rotation/translation/mode, never a matrix.
// Evaluated/stored matrices are 0x40 bytes: four float4 basis/translation rows.
// The raw byte cursors below address these distinct arrays by sampleIndex.
struct GtlEvaluatedMatrix {
    float right[4], up[4], forward[4], translation[4];
};
typedef char GtlPoseSizeCheck[sizeof(GtlPose) == 0x30 ? 1 : -1];
typedef char GtlMatrixSizeCheck[sizeof(GtlEvaluatedMatrix) == 0x40 ? 1 : -1];
struct GtlAnimLocal {
    GtlAnimation anim;
    float localMatrix[16];
};

extern "C" {

void* gtl_ResourceFile_FindChunk(void* file, void* data, u32 tag);
s32 gtl_ResourceFile_HasChunk(void* file, void* data, u32 tag);
void* gtl_ResourceFile_GetChunkElement(void* file, void* data, u32 tag, u32 index);
u32 gtl_Resource_GetDataTypeByteStride(u32 dataType);
void* gtl_Culling_GetInverseViewMatrix(void);
void Vec3_Subtract(void* dst, const void* a, const void* b);
void Vec3_Normalize(void* v, void* v2);
void Vector_CopyComponents(void* dst, const void* src);
// ORIGINAL arities: Vec4_Normalize(v, input) is called in place (the Wii does mr r3,rX / mr r4,rX), and
// gtl_Billboard_RotateDirection takes the 0x18-byte billboard entry in r5 (+4 clamp enable, +8/+C yaw bounds,
// +10/+14 pitch bounds). The Wii call text below does not name those values: the registers still hold them.
void Vec4_Normalize(void* v);
void Vec3_Cross(void* dst, const void* a, const void* b);
void gtl_Billboard_RotateDirection(void* dst, const void* dir);
void gtl_Billboard_BuildYAxisBasis(void* dst, const void* dir);
void Matrix_AlignForwardPreservingScaleAndTranslationWrapper(void* dst, void* src, void* dir, s32 flag);
void gtl_Transform_BuildNodeMatrix(void* out, const void* pose);
void gtl_Transform_QuaternionFromEuler(float* out, float a, float b, float c);
void Mtx44_Copy(void* dst, const void* src);
void PSMTX44Concat(void* a, void* b, void* ab);
s32 gtl_Animation_EvaluateNodeLocalMatrix(void* anim, u32 nodeIndex, void* out);
void gtl_Animation_EvaluateNodeWorldMatrix(void* anim, u32 nodeIndex, void* out, u32* flags);
void* memcpy(void* dst, const void* src, u32 n);
void* memset(void* dst, s32 value, u32 n);

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
extern s32 s_animationEndpointMode;
extern s8 s_animationKeySearchProbesInitialized;
extern float s_animationSegmentDuration;
extern float s_animationSegmentStartTime;
extern float s_animationSampleTime;
extern float s_animationSegmentEndTime;
extern float kFloat_0_GtlModel;
extern float kAnimationWrapInterval;
extern float kUnitYAxisVec4[4];

}

// Apply this node's tracks in array order to an existing local pose. All inputs
// required; cur is written, others borrowed. t/key times share authored time
// units (no conversion here; seconds vs ticks is not established by this API).
// Clamp at endpoints for s_animationEndpointMode==0; mode 1 blends last->first across the
// kAnimationWrapInterval interval. Interior alpha=(t-t0)/(t1-t0); keys must have distinct
// adjacent times. Exact hits copy values, avoiding interpolation/rounding.
// Search callbacks receive [secondKey,end) plus global per-format scratch;
// interpolation callbacks receive WHOLE key records, not value-only pointers.
// Globals s_animationSegmentDuration/StartTime/SampleTime/EndTime publish interval/delta time for interpolators.
// These scratch globals make evaluation non-reentrant and unsynchronized.
// A 16-byte translation record overwrites order transiently; restore it after
// all tracks. Mode 6 then converts sampled rot's XYZ radians to a quaternion
// with argument order Z,X,Y. Do not normalize/reorder arithmetic.
static inline void EvalNodeTracks(GtlAnimTables* tables, GtlPose* cur, GtlAnimTrack* tracks, u8* keyData, GtlAnimNode* node, float t)
{
    s32 order = cur->transformMode;
    u32 j;

    for (j = 0; j < node->trackCount; j++) {
        GtlAnimTrack* track = tracks + (node->trackBase + j);
        void* dst = 0;
        u32 size = 0;
        switch (track->type) {
        case kTranslationZ: dst = &cur->translation[2]; size = 4; break;
        case kTranslationY: dst = &cur->translation[1]; size = 4; break;
        case kTranslationX: dst = &cur->translation[0]; size = 4; break;
        case kTranslationRecord: dst = cur->translation; size = 0x10; break;
        case kRotationZ: dst = &cur->rot[2]; size = 4; break;
        case kRotationY: dst = &cur->rot[1]; size = 4; break;
        case kRotationX: dst = &cur->rot[0]; size = 4; break;
        case kRotationRecord: dst = cur->rot; size = 0x10; break;
        case kScaleZ: dst = &cur->scale[2]; size = 4; break;
        case kScaleY: dst = &cur->scale[1]; size = 4; break;
        case kScaleX: dst = &cur->scale[0]; size = 4; break;
        case kScaleRecord: dst = cur->scale; size = 0x10; break;
        }

        u8* keys = keyData + track->keyOffset;
        u32 stride = gtl_Resource_GetDataTypeByteStride(track->dataType);
        u32 tOff = tables->timeOffset[track->dataType];
        bool key0Ge = *(float*)(keys + tOff) >= t;
        u8* lastKey;
        bool afterLast;

        if ((track->keyCount == 1 || key0Ge) && s_animationEndpointMode == 0) {
            Memory_Copy(dst, keys + tables->valueOffset[track->dataType], size);
            continue;
        }

        lastKey = keys + stride * (track->keyCount - 1);
        afterLast = t >= *(float*)(lastKey + tOff);
        if (afterLast && s_animationEndpointMode == 0) {
            Memory_Copy(dst, lastKey + tables->valueOffset[track->dataType], size);
            continue;
        }

        float t0 = kFloat_0_GtlModel;
        float t1 = kFloat_0_GtlModel;
        float alpha = kFloat_0_GtlModel;
        u8* a = 0;
        u8* b = 0;

        if (key0Ge || afterLast) {
            b = keys;
            a = lastKey;
            if (s_animationEndpointMode == 1) {
                if (key0Ge) {
                    t1 = *(float*)(keys + tOff);
                    t0 = t1 - kAnimationWrapInterval;
                    alpha = (t - t0) / kAnimationWrapInterval;
                } else {
                    t0 = *(float*)(lastKey + tOff);
                    alpha = (t - t0) / kAnimationWrapInterval;
                    t1 = t0 + kAnimationWrapInterval;
                }
            }
        } else {
            u8* cache;
            u8* found;
            u8* prev;
            u32 idx;

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

            cache = (u8*)s_animationKeySearchProbes[track->dataType];
            *(float*)(cache + tOff) = t;

            found = s_animationKeySearchFunctions[track->dataType](keys + stride, keys + stride + stride * (track->keyCount - 1), cache);
            if (found == 0) {
                found = lastKey;
            }
            t1 = *(float*)(found + tOff);
            if (t == t1) {
                Memory_Copy(dst, found + tables->valueOffset[track->dataType], size);
                continue;
            }

            idx = (u32)(found - keys) / stride;
            prev = keys + stride * (idx - 1);
            t0 = *(float*)(prev + tOff);
            alpha = (t - t0) / (t1 - t0);
            a = prev;
            b = found;
        }

        s_animationSegmentStartTime = t0;
        s_animationSampleTime = t;
        s_animationSegmentDuration = t1 - t0;
        s_animationSegmentEndTime = t1;
        tables->interp[track->dataType](dst, a, b, alpha);
    }

    cur->transformMode = order;
    if (order == 6) {
        gtl_Transform_QuaternionFromEuler(cur->rot, cur->rot[2], cur->rot[0], cur->rot[1]);
    }
}

extern "C" {

// Required file/root/out: loaded native resource, 16-float root transform and
// output array of 0x40-byte matrices indexed by sampleIndex. 'trns' must exist;
// every traversed node/sample/parent/bill index must be valid for its chunk.
// poses may be null for stored matrices; otherwise supply every accessed sample.
// The legacy fallback tests sample<=poseCount (inclusive): callers must avoid an
// accessed index==poseCount unless poses[poseCount] is a readable sentinel
// GtlPose (another full 0x30 bytes). poseCount is the strict track-path count
// but an INCLUSIVE bound in the fallback: do not change <= to <.
// For each accessed sample, out/flags/stored matrices still need that slot
// within trns.count; allocating a sentinel pose alone does not extend them.
// flagsIn may be null; otherwise contains trns.count u32 values. Nonzero means
// output already computed and must be preserved. Flags are copied to stack,
// updated locally and never written back. No retained caller pointers or status;
// malformed input is unchecked rather than reported with partial-success codes.
// Phases: load chunks/camera -> ensure ancestors -> local pose/base matrix ->
// compose parent using recovered concat ABI -> billboard -> mark output ready.
// Matrices use basis vectors at +0/+10/+20, translation at +30; camera inverse
// supplies position at +30 and forward at +20. Model/root/camera coordinates
// must be compatible; this function performs no unit or handedness conversion.
void gtl_Animation_EvaluateModelMatrices(void* file, void* out, void* root, GtlPose* poses, u32 poseCount, u32* flagsIn)
{
    GtlAnimTables* tables = (GtlAnimTables*)&s_animationSamplingTables;
    u32 i;
    GtlPose childBasePose;
    GtlPose childSampledPose;
    GtlPose rootBasePose;
    GtlPose rootSampledPose;
    float rootToCameraDirection[3];
    float cameraPosition[3];
    float parentBillboardDirection[3];
    float nodeBillboardDirection[3];
    float* inverseView;
    float* inverseViewPosition;

    float* cameraForward;
    float* nodeMatrixWords;
    void* bill;
    GtlChunk* transformsChunk;
    GtlChunk* nodesChunk;

    u32 animZero = 0;
    float zero;
    float fixedUpW;
    float fixedUpZ;
    float fixedUpY;
    float fixedUpX;

    transformsChunk = (GtlChunk*)gtl_ResourceFile_FindChunk(file, *(void**)((u8*)file + 0xC), 0x736E7274);
    u32* flags = (u32*)__alloca(transformsChunk->count * 4);
    if (flagsIn != 0) {
        memcpy(flags, flagsIn, transformsChunk->count * 4);
    } else {
        memset(flags, 0, transformsChunk->count * 4);
    }

    inverseView = (float*)gtl_Culling_GetInverseViewMatrix();
    inverseViewPosition = inverseView + 12;
    Vec3_Subtract(rootToCameraDirection, inverseViewPosition, (float*)root + 12);
    Vec3_Normalize(rootToCameraDirection, rootToCameraDirection);
    Vector_CopyComponents(cameraPosition, inverseViewPosition);

    nodesChunk = (GtlChunk*)gtl_ResourceFile_FindChunk(file, *(void**)((u8*)file + 0xC), 0x6C646D);
    bill = 0;
    if (gtl_ResourceFile_HasChunk(file, *(void**)((u8*)file + 0xC), 0x6C6C6962)) {
        GtlChunk* billChunk = (GtlChunk*)gtl_ResourceFile_FindChunk(file, *(void**)((u8*)file + 0xC), 0x6C6C6962);
        if (billChunk->count != 0) {
            bill = (u8*)file + billChunk->offset;
        }
    }

    // This entry leaves tracks/keyData null: choose supplied pose (including
    // the inclusive sentinel) or stored matrix. The track branch below is
    // the shared evaluator expansion, with separate root/child pose scratch.
    GtlAnimLocal evaluationStorage = {};
    GtlAnimation& anim = evaluationStorage.anim;
    float (&localMatrix)[16] = evaluationStorage.localMatrix;
    anim.nodes = (GtlAnimNode*)((u8*)file + nodesChunk->offset);
    anim.nodeCount = nodesChunk->count;
    anim.matrices = (u8*)file + transformsChunk->offset;
    anim.bill = bill;
    anim.ikChunkElements[0] = gtl_ResourceFile_GetChunkElement(file, *(void**)((u8*)file + 0xC), 0x726B69, animZero);
    anim.ikChunkElements[1] = gtl_ResourceFile_GetChunkElement(file, *(void**)((u8*)file + 0xC), 0x6A6B69, animZero);
    anim.ikChunkElements[2] = gtl_ResourceFile_GetChunkElement(file, *(void**)((u8*)file + 0xC), 0x656B69, animZero);
    anim.ikChunkElements[3] = gtl_ResourceFile_GetChunkElement(file, *(void**)((u8*)file + 0xC), 0x696A6B69, animZero);
    anim.rootToCameraDirection = rootToCameraDirection;
    anim.cameraPosition = cameraPosition;
    anim.inverseView = inverseView;
    anim.samples = poses;
    anim.sampleCount = poseCount;

    GtlAnimNode* nodes = anim.nodes;
    GtlAnimTrack* tracks = anim.tracks;
    u8* keyData = anim.keyData;

    cameraForward = (float*)((u8*)inverseView + 0x20);
    fixedUpX = kUnitYAxisVec4[0];
    fixedUpY = kUnitYAxisVec4[1];
    fixedUpZ = kUnitYAxisVec4[2];
    fixedUpW = kUnitYAxisVec4[3];
    zero = kFloat_0_GtlModel;

    for (i = 0; i < transformsChunk->count; i++) {
        GtlAnimNode* node = nodes + i;
        u32 parent = node->parent;
        u32 sample = node->sampleIndex;
        bool compute = false;

        if (parent == 0xFFFF) {
            if (flags[sample] == 0) {
                u8* dst = (u8*)out + (sample << 6);

                if (nodes != 0 && tracks != 0 && keyData != 0) {
                    if (sample < poseCount) {
                        Memory_Copy(&rootBasePose, poses + sample, 0x30);
                        if (poses != 0 && i < anim.nodeCount && node->trackCount != 0) {
                            Memory_Copy(&rootSampledPose, poses + sample, 0x30);
                            EvalNodeTracks(tables, &rootSampledPose, tracks, keyData, node, anim.time);
                            Memory_Copy(&rootBasePose, &rootSampledPose, 0x30);
                        }
                    }
                    gtl_Transform_BuildNodeMatrix(dst, &rootBasePose);
                } else if (poses != 0 && sample <= poseCount) {
                    gtl_Transform_BuildNodeMatrix(dst, poses + sample);
                } else {
                    Mtx44_Copy(dst, anim.matrices + (sample << 6));
                }
                compute = true;
            }
        } else {
            GtlAnimNode* parentNode = nodes + parent;
            u32 grandparentIndex = parentNode->parent;
            u32 parentSample = parentNode->sampleIndex;
            u8* parentMatrix = (u8*)out + (parentSample << 6);
            bool parentDone = false;

            if (grandparentIndex == 0xFFFF) {
                if (flags[parentSample] == 0) {
                    gtl_Animation_EvaluateNodeLocalMatrix(&anim, parent, parentMatrix);
                    parentDone = true;
                }
            } else {
                GtlAnimNode* grandparentNode = nodes + grandparentIndex;
                gtl_Animation_EvaluateNodeWorldMatrix(&anim, grandparentIndex, out, flags);
                if (flags[parentSample] == 0) {
                    gtl_Animation_EvaluateNodeLocalMatrix(&anim, parent, localMatrix);
                    PSMTX44Concat(parentMatrix, localMatrix, (u8*)out + (grandparentNode->sampleIndex << 6));
                    parentDone = true;
                }
            }

            if (parentDone) {
                if (anim.bill != 0) {
                    u8* entry = (u8*)anim.bill + parentNode->billIndex * 0x18;
                    switch (*(u32*)entry) {
                    case 0:
                        gtl_Billboard_RotateDirection(parentBillboardDirection, rootToCameraDirection);
                        gtl_Billboard_BuildYAxisBasis(parentMatrix, parentBillboardDirection);
                        break;
                    case 1:
                        gtl_Billboard_RotateDirection(parentBillboardDirection, rootToCameraDirection);
                        Matrix_AlignForwardPreservingScaleAndTranslationWrapper(parentMatrix, parentMatrix, parentBillboardDirection, 1);
                        break;
                    case 2:
                        gtl_Billboard_RotateDirection(parentBillboardDirection, cameraForward);
                        gtl_Billboard_BuildYAxisBasis(parentMatrix, parentBillboardDirection);
                        break;
                    case 3:
                        gtl_Billboard_RotateDirection(parentBillboardDirection, cameraForward);
                        Matrix_AlignForwardPreservingScaleAndTranslationWrapper(parentMatrix, parentMatrix, parentBillboardDirection, 1);
                        break;
                    }
                }
                flags[parentSample] = 1;
            }

            if (flags[sample] == 0) {
                u8* childMatrix = (u8*)out + (sample << 6);

                if (nodes != 0 && tracks != 0 && keyData != 0) {
                    if (sample < poseCount) {
                        Memory_Copy(&childBasePose, poses + sample, 0x30);
                        if (poses != 0 && i < anim.nodeCount && node->trackCount != 0) {
                            Memory_Copy(&childSampledPose, poses + sample, 0x30);
                            EvalNodeTracks(tables, &childSampledPose, tracks, keyData, node, anim.time);
                            Memory_Copy(&childBasePose, &childSampledPose, 0x30);
                        }
                    }
                    gtl_Transform_BuildNodeMatrix(localMatrix, &childBasePose);
                } else if (poses != 0 && sample <= poseCount) {
                    gtl_Transform_BuildNodeMatrix(localMatrix, poses + sample);
                } else {
                    Mtx44_Copy(localMatrix, anim.matrices + (sample << 6));
                }
                PSMTX44Concat(childMatrix, localMatrix, (u8*)out + (parentSample << 6));
                compute = true;
            }
        }

        if (compute) {
            if (anim.bill != 0) {
                nodeMatrixWords = (float*)((u8*)out + (sample << 6));
                u8* entry = (u8*)anim.bill + node->billIndex * 0x18;
                switch (*(u32*)entry) {
                case 0:
                    gtl_Billboard_RotateDirection(nodeBillboardDirection, rootToCameraDirection);
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->up[0] = fixedUpX;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->up[1] = fixedUpY;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->up[2] = fixedUpZ;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->up[3] = fixedUpW;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->forward[0] = nodeBillboardDirection[0];
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->forward[1] = zero;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->forward[2] = nodeBillboardDirection[2];
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->forward[3] = zero;
                    Vec4_Normalize(nodeMatrixWords + 8);
                    Vec3_Cross(nodeMatrixWords, nodeMatrixWords + 4, nodeMatrixWords + 8);
                    break;
                case 1:
                    gtl_Billboard_RotateDirection(nodeBillboardDirection, rootToCameraDirection);
                    Matrix_AlignForwardPreservingScaleAndTranslationWrapper(nodeMatrixWords, nodeMatrixWords, nodeBillboardDirection, 1);
                    break;
                case 2:
                    gtl_Billboard_RotateDirection(nodeBillboardDirection, cameraForward);
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->up[0] = fixedUpX;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->up[1] = fixedUpY;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->up[2] = fixedUpZ;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->up[3] = fixedUpW;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->forward[0] = nodeBillboardDirection[0];
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->forward[1] = zero;
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->forward[2] = nodeBillboardDirection[2];
                    ((GtlEvaluatedMatrix*)nodeMatrixWords)->forward[3] = zero;
                    Vec4_Normalize(nodeMatrixWords + 8);
                    Vec3_Cross(nodeMatrixWords, nodeMatrixWords + 4, nodeMatrixWords + 8);
                    break;
                case 3:
                    gtl_Billboard_RotateDirection(nodeBillboardDirection, cameraForward);
                    Matrix_AlignForwardPreservingScaleAndTranslationWrapper(nodeMatrixWords, nodeMatrixWords, nodeBillboardDirection, 1);
                    break;
                }
            }
            flags[sample] = 1;
        }
    }
}

}
