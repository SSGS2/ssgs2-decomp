// WPuppet, the animated model object: loading a model file, its bone hierarchy, bone
// visibility and bounds, skin matrices, animation playback and root motion.
// Also covers the collision boxes and named bones, and child puppets that share a parent's model.

#include <decomp/static_init.h>
// WPuppet model, bone, animation and resource ownership (unit 0x8004FC28).
// Entry map for model/texture/animation mods (types: WPuppet.h, wvector.h;
// strings/scalars: the data section and puppetPreloadStrings below):
// - Model loading / shared cache: WPuppet_Share publishes borrowed
//   resource metadata; WPuppet_RefreshMatrixBuffers checks shared-name ownership
//   before replacing bone matrices. Search HasSharedPuppetResourceName for the
//   exact byte-key lookup; changing a basename also changes sharing, not just UI.
// - Bone visibility: WPuppet_ShowBoneSelection / HideBoneSelection select one
//   name or the Recursive keyword; WPuppet_ParseIndexedBoneName handles the
//   separately defined indexed-name grammar. It is NOT a general path parser.
// - Bounds / collision: WPuppet_AccumulateTransformedBounds traverses mesh/skin
//   matrices; PuppetBoundsView holds min/max/center/radius in model/world paths.
// - Material/texture animation: search layerResource, layerMatrices and
//   CTextureCoordinateRegistry::Create; group/transform chunk counts must agree
//   with the allocations and fixed binary record layouts below.
// - Animation events: PuppetFrameChunk/PuppetFrameRecord describe keyed records;
//   PuppetMotionRangeView and animationRanges select ranges and blending.
// - Root-motion timing: GetInverseRootMotionOffset consumes 30-Hz key frames;
//   GetRootMotionDelta documents the preserved double-30 boundary formula,
//   unchecked range/weight prerequisites and destructive sampling side effects.
// - Pose/allocation: UpdateSkinFromAnimationSource exposes GetAmount's float
//   dataflow; AllocateSkinMatrices documents destructive replacement/empty success.
// - Chunk dispatch: LoadFromChunks documents the six-word ABI, overloaded
//   size/name word, loader order, and unconditional BONE reload.
// - Child lifetime: PuppetInstanceNode lists borrow/share model resources; the
//   release paths distinguish child links, metadata, bone/skin/layer buffers.
//   Do not free a lookup result independently of the resource that owns it.
// GPU camera/viewport policy is outside this unit. Layouts below describe Wii
// runtime records / native big-endian chunk fields, not portable serialized pointers.
#include "MSL/vector.h"
#include "MSL/list.h"
// Placeholder element types for the containers whose deleting destructors this unit keeps out of line:
// the destructor depends only on whether the element is a scalar (pointer) or a record.
struct MSLRecord {
    unsigned int words[1];
};
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
typedef std::vector<MSLRecord, app::app_allocator<MSLRecord> > MSLRecordVector;
typedef std::list<void*, app::app_allocator<void*> > MSLPointerList;
#include "classes/MWStringStorage.h"
#include "game/types.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/WPuppet.h"
#include "classes/ResourceStringNode.h"
#include <decomp/cw_bitfields.h>
#include "classes/CPetAnimation.h"
#include "classes/CPetTexManager.h"
#include "classes/CRotateRegistry.h"
#include "classes/CTextureCoordinateRegistry.h"
#include "classes/CPreloadSeqSync.h"
#include "classes/CPreloadFileSync.h"
#include "classes/CPreloadTextureSync.h"
#include "classes/CPreloadSprSync.h"
#include "classes/GtlIGenericModelingResource.h"

// Entry points of gtl::IGenericModelingResource members that are called through function-pointer casts with
// the original argument lists. Each is the member symbol under its mangled name; `this` is the first argument.
extern "C" {
void EvaluateTransforms__Q23gtl24IGenericModelingResourceCFPvPvPvPvUiPUi();
void SetResource__Q23gtl24IGenericModelingResourceFiiP12GtlRefHeader();
void GetResource__Q23gtl24IGenericModelingResourceFiiPP12GtlRefHeader();
void QueueDraw__Q23gtl24IGenericModelingResourceFPvPv();
void EvaluateMaterialAnimation__Q23gtl24IGenericModelingResourceCFPv();
}

// Local layouts for the binary model/chunk formats used here.
struct PuppetBoneView {
    char pad_000[8];
    char name[0x24];
    float initialMatrix[12];
    char pad_05C[8];
    void* next;
    unsigned int index;
    unsigned char flags;
    char pad_06D[0xB];
    float worldMatrix[12];
    float skinMatrix[12];
};
struct PuppetCameraRenderState {
    char pad_000[0x1C];
    unsigned int dirtyMatrices;
    float viewMatrix[16];
    float projectionMatrix[16];
};
struct PuppetArchiveView {
    char pad_000[0xC];
    void* chunkTable;
};
struct PuppetChunkHeader {
    char tag[4];
    unsigned int byteSize;
    unsigned int dataOffset;
    unsigned short entryCount;
    unsigned short formatFlags;
};
struct PuppetMotionRangeView {
    float startTime, endTime, blendEndTime;
    int blendMode;
    char pad_010[8];
    const char* name;
    void* blendSourceName;
    char pad_020[0x10];
    unsigned short animationIndex;
    char pad_032[2];
};
struct PuppetInstanceNode {
    void* value;
    unsigned int payload[2];
    void* prev;
    void* next;
};
struct PuppetMaterialView {
    char pad_000[0x2C];
    float opacity;
    char pad_030[0xC0];
};
struct PuppetBoundsView {
    Vec3f minimum, maximum, center;
    float radius;
    char* command;
    char name[0x100];
};
struct PuppetMetadataView {
    const char* name;
    void* root;
    float duration;
    unsigned int triangleCount;
    unsigned int keyRecordCount;
    void* keyRecords;
    Vec3f minimum, maximum;
    void* namedBones;
    int namedBoneCount;
    void* animationRanges;
};
typedef char PuppetBoneView_size[(sizeof(PuppetBoneView) == 0xD8) ? 1 : -1];
typedef char PuppetMaterialView_size[(sizeof(PuppetMaterialView) == 0xF0) ? 1 : -1];
typedef char PuppetBoundsView_size[(sizeof(PuppetBoundsView) == 0x12C) ? 1 : -1];

// Small-string bytes overlap the heap words; keep both representations explicit.
struct PuppetInlineStringBytes {
    unsigned char lengthAndFlags;
    char text[11];
};
union PuppetStringKeyStorage {
    MWStringStorage heap;
    PuppetInlineStringBytes small;
};
struct PuppetNameNodeView {
    void* left;
    void* right;
    void* taggedParent;
    PuppetStringKeyStorage key;
    unsigned int useCount;
};
struct PuppetMotionSuppressionRange {
    char pad_000[8];
    float startFrame, endFrame;
};
struct PuppetStringValuePair {
    MWStringStorage key;
    unsigned int value;
};
struct PuppetKeyRecord {
    unsigned int key;
    unsigned int flags;
    char* text;
};
struct PuppetFrameChunk {
    char tag[4];
    unsigned int byteSize;
    unsigned int format;
    unsigned int recordsOffset;
    unsigned short rangeEnd, rangeStart, recordCount;
};
struct PuppetFrameRecord {
    unsigned int textOffset;
    unsigned int format;
    unsigned int key;
    unsigned short textLength;
    char pad_00E[4];
    unsigned char flags;
    char pad_013;
};
struct PuppetModelLayerView {
    unsigned short groupCount, firstGroup;
    char pad_004[6];
    unsigned short parentBone;
    char pad_00C[0xC];
    unsigned short transformIndex;
    char pad_01A[0x16];
};
struct PuppetPrimitiveView {
    char pad_000[4];
    unsigned short materialIndex;
    char pad_006[8];
    unsigned short triangleCount, indexHeader;
    char pad_012[0xE];
};
struct PuppetNamedBoneView {
    void* bone;
    void* bounds;
};
struct PuppetMeshGroupView {
    unsigned int firstPrimitive, primitiveCount;
};
struct PuppetMaterialTextures {
    char pad_000[0xC];
    unsigned char textureIds[4];
    char pad_010[0xC];
};
struct PuppetBoneNameRecord {
    unsigned int textOffset;
    unsigned short formatFlags, textLength;
};
struct PuppetCollisionRecord {
    unsigned int textOffset;
    char pad_004[8];
    Vec3f minimum, maximum;
    char pad_024[4];
    unsigned short textLength, boneIndex;
    char pad_02C[4];
};
struct PuppetSharedStringOwner {
    void* vtable;
    int strongReferences, weakReferences;
    char* text;
};
enum PuppetRecordSizes {
    kBoneBytes = sizeof(PuppetBoneView),
    kMaterialBytes = sizeof(PuppetMaterialView),
    kModelLayerBytes = sizeof(PuppetModelLayerView),
    kMatrix4Bytes = 0x40
};

// Confirmed entries in the existing read-only string pool; lengths include terminators.
#define PUPPET_OFFSET(type, member) ((int)&((type*)0)->member)

// Fail compilation if a named binary-format field no longer describes the original layout.
#define PUPPET_FIELD(type, member, offset) \
    typedef char type##_off_##member[(PUPPET_OFFSET(type, member) == (offset)) ? 1 : -1]
PUPPET_FIELD(PuppetBoneView, name, 0x8);
PUPPET_FIELD(PuppetBoneView, initialMatrix, 0x2C);
PUPPET_FIELD(PuppetBoneView, next, 0x64);
PUPPET_FIELD(PuppetBoneView, index, 0x68);
PUPPET_FIELD(PuppetBoneView, flags, 0x6C);
PUPPET_FIELD(PuppetBoneView, worldMatrix, 0x78);
PUPPET_FIELD(PuppetBoneView, skinMatrix, 0xA8);
PUPPET_FIELD(PuppetCameraRenderState, dirtyMatrices, 0x1C);
PUPPET_FIELD(PuppetCameraRenderState, viewMatrix, 0x20);
PUPPET_FIELD(PuppetCameraRenderState, projectionMatrix, 0x60);
PUPPET_FIELD(PuppetArchiveView, chunkTable, 0xC);
PUPPET_FIELD(PuppetChunkHeader, byteSize, 0x4);
PUPPET_FIELD(PuppetChunkHeader, dataOffset, 0x8);
PUPPET_FIELD(PuppetChunkHeader, entryCount, 0xC);
PUPPET_FIELD(PuppetMotionRangeView, startTime, 0x0);
PUPPET_FIELD(PuppetMotionRangeView, endTime, 0x4);
PUPPET_FIELD(PuppetMotionRangeView, blendEndTime, 0x8);
PUPPET_FIELD(PuppetMotionRangeView, blendMode, 0xC);
PUPPET_FIELD(PuppetMotionRangeView, name, 0x18);
PUPPET_FIELD(PuppetMotionRangeView, blendSourceName, 0x1C);
PUPPET_FIELD(PuppetMotionRangeView, animationIndex, 0x30);
PUPPET_FIELD(PuppetInstanceNode, value, 0x0);
PUPPET_FIELD(PuppetInstanceNode, prev, 0xC);
PUPPET_FIELD(PuppetInstanceNode, next, 0x10);
PUPPET_FIELD(PuppetMaterialView, opacity, 0x2C);
PUPPET_FIELD(PuppetBoundsView, minimum, 0x0);
PUPPET_FIELD(PuppetBoundsView, maximum, 0xC);
PUPPET_FIELD(PuppetBoundsView, center, 0x18);
PUPPET_FIELD(PuppetBoundsView, radius, 0x24);
PUPPET_FIELD(PuppetBoundsView, command, 0x28);
PUPPET_FIELD(PuppetBoundsView, name, 0x2C);
PUPPET_FIELD(PuppetMetadataView, name, 0x0);
PUPPET_FIELD(PuppetMetadataView, root, 0x4);
PUPPET_FIELD(PuppetMetadataView, duration, 0x8);
PUPPET_FIELD(PuppetMetadataView, triangleCount, 0xC);
PUPPET_FIELD(PuppetMetadataView, keyRecordCount, 0x10);
PUPPET_FIELD(PuppetMetadataView, keyRecords, 0x14);
PUPPET_FIELD(PuppetMetadataView, minimum, 0x18);
PUPPET_FIELD(PuppetMetadataView, maximum, 0x24);
PUPPET_FIELD(PuppetMetadataView, namedBones, 0x30);
PUPPET_FIELD(PuppetMetadataView, namedBoneCount, 0x34);
PUPPET_FIELD(PuppetMetadataView, animationRanges, 0x38);
PUPPET_FIELD(PuppetNameNodeView, key, 0xC);
PUPPET_FIELD(PuppetNameNodeView, useCount, 0x18);
PUPPET_FIELD(PuppetKeyRecord, key, 0x0);
PUPPET_FIELD(PuppetKeyRecord, flags, 0x4);
PUPPET_FIELD(PuppetKeyRecord, text, 0x8);
PUPPET_FIELD(PuppetFrameChunk, recordsOffset, 0xC);
PUPPET_FIELD(PuppetFrameChunk, rangeEnd, 0x10);
PUPPET_FIELD(PuppetFrameChunk, rangeStart, 0x12);
PUPPET_FIELD(PuppetFrameChunk, recordCount, 0x14);
PUPPET_FIELD(PuppetFrameRecord, textOffset, 0x0);
PUPPET_FIELD(PuppetFrameRecord, key, 0x8);
PUPPET_FIELD(PuppetFrameRecord, textLength, 0xC);
PUPPET_FIELD(PuppetFrameRecord, flags, 0x12);
PUPPET_FIELD(PuppetModelLayerView, groupCount, 0x0);
PUPPET_FIELD(PuppetModelLayerView, firstGroup, 0x2);
PUPPET_FIELD(PuppetModelLayerView, parentBone, 0xA);
PUPPET_FIELD(PuppetModelLayerView, transformIndex, 0x18);
PUPPET_FIELD(PuppetPrimitiveView, materialIndex, 0x4);
PUPPET_FIELD(PuppetPrimitiveView, triangleCount, 0xE);
PUPPET_FIELD(PuppetPrimitiveView, indexHeader, 0x10);
PUPPET_FIELD(PuppetNamedBoneView, bone, 0x0);
PUPPET_FIELD(PuppetNamedBoneView, bounds, 0x4);
PUPPET_FIELD(PuppetMeshGroupView, firstPrimitive, 0x0);
PUPPET_FIELD(PuppetMeshGroupView, primitiveCount, 0x4);
PUPPET_FIELD(PuppetMaterialTextures, textureIds, 0xC);
PUPPET_FIELD(PuppetBoneNameRecord, textOffset, 0x0);
PUPPET_FIELD(PuppetBoneNameRecord, textLength, 0x6);
PUPPET_FIELD(PuppetCollisionRecord, minimum, 0xC);
PUPPET_FIELD(PuppetCollisionRecord, maximum, 0x18);
PUPPET_FIELD(PuppetCollisionRecord, textLength, 0x28);
PUPPET_FIELD(PuppetCollisionRecord, boneIndex, 0x2A);
PUPPET_FIELD(PuppetSharedStringOwner, strongReferences, 0x4);
PUPPET_FIELD(PuppetSharedStringOwner, weakReferences, 0x8);
PUPPET_FIELD(PuppetSharedStringOwner, text, 0xC);
#undef PUPPET_FIELD
typedef char PuppetMaterialTextures_size[(sizeof(PuppetMaterialTextures) == 0x1C) ? 1 : -1];
typedef char PuppetBoneNameRecord_size[(sizeof(PuppetBoneNameRecord) == 8) ? 1 : -1];
typedef char PuppetCollisionRecord_size[(sizeof(PuppetCollisionRecord) == 0x30) ? 1 : -1];

struct PuppetPreloadStrings {
    char vectorLengthError[20];     // +0x0 "vector length error"
    char lengthErrorType[55];       // +0x14 "!std::exception!!std::logic_error!!std::length_error!!"
    char boundBone[6];              // +0x4B "bound"
    char centerBone[7];             // +0x51 "center"
    char emptyPath[1];              // +0x58
    char stringFormat[3];           // +0x59 "%s"
    char animationTag[5];           // +0x5C "ANIM"
    char meshTag[5];                // +0x61 "MESH"
    char boneTag[5];                // +0x66 "BONE"
    char frameTag[5];               // +0x6B "FRAM"
    char motionTag[5];              // +0x70 "MOTI"
    char collisionTag[5];           // +0x75 "COLL"
    char collisionCommandFormat[6]; // +0x7A "%s %s"
    char externalFilesTag[5];       // +0x80 "EXTF"
    char spriteSuffix[5];           // +0x85 ".spr"
    char compareRangeError[26];     // +0x8A "string compare: pos > len"
    char outOfRangeType[55];        // +0xA4 "!std::exception!!std::logic_error!!std::out_of_range!!"
    char sequenceSuffix[5];         // +0xDB ".seq"
    char textureSuffix[5];          // +0xE0 ".gsr"
    char fileSuffix[5];             // +0xE5 ".gar"
    char defaultBodyPattern[7];     // +0xEA "d_def."
    char defaultHeadPattern[7];     // +0xF1 "h_def."
    char substringRangeError[27];   // +0xF8 "basic_string: out_of_range"
    char recursiveKeyword[10];      // +0x113 "Recursive"
    char treeLengthError[26];       // +0x11D "tree::insert length error"
    char reserveLengthError[35];    // +0x137 "basic_string::reserve length_error"
    char stringLengthError[27];     // +0x15A "basic_string: length_error"
    char padding[7];                // +0x175
};
typedef char PuppetPreloadStrings_size[(sizeof(PuppetPreloadStrings) == 0x17C) ? 1 : -1];

// Numeric format tags passed to the model resource API.
enum PuppetChunkId {
    kChunkBoneTransforms = 0x6674,
    kChunkSkinTransforms = 0x736E7274,
    kChunkModelLayers = 0x6C646D,
    kChunkMeshGroups = 0x68736D,
    kChunkPrimitives = 0x6D7270,
    kChunkTextureNames = 0x6E786574,
    kChunkStringData = 0x646273,
    kChunkIndexData = 0x646269,
    kChunkIndexHeaders = 0x686269,
    kChunkBindMatrices = 0x78746D6F,
    kChunkBounds = 0x76646272,
    kChunkMaterialLayers = 0x647074,
    kChunkMaterialTransforms = 0x7074,
    kChunkMaterialRecords = 0x6472746D,
    kChunkMaterialMetadata = 0x6C72746D
};
enum PuppetLoadFlags {
    kLoadTextures = 0x1,
    kLoadAnimation = 0x4,
    kLoadModel = 0x8,
    kLoadKeys = 0x20,
    kLoadMotionRanges = 0x40,
    kLoadCollision = 0x80,
    kLoadExternalResources = 0x100,
    kLoadModelArchive = 0x2000,
    kContainerLoadMode = 0x82,
    kCloneModelLoadMode = 0x101B,
    kAnimationSourceFlags = 0x166
};
enum PuppetRenderConstants {
    kViewMatrixDirty = 1,
    kProjectionMatrixDirty = 2,
    kViewTransform = 0,
    kProjectionTransform = 1,
    kPuppetLightSlot = 7,
    kSceneLightSlot = 3,
    kRenderStateCount = 16,
    kTextureStateCount = 6,
    kPuppetRenderStateValueIndex = 12
};
enum PuppetTextureConstants {
    kTextureIdCount = 0x100,
    kNoTextureId = 0xFF,
    kMaterialTextureSlots = 4,
    kTextureResourceKind = 3,
    kTexturePaletteResourceKind = 5
};
enum PuppetStringConstants {
    kStringHeapFlagBit = 31,
    kStringInlineLengthMask = 0x7F
};

// The duplicate STL/exception adapters and explicit value copies preserve CodeWarrior's
// instruction scheduling, cleanup references, and allocator lifetimes. Keep them in place
// when changing model behavior; they are matching constraints rather than game rules.

// Placement construction uses already allocated vector/list storage.
inline void* operator new(size_t, void* p) {
    return p;
}
extern "C" {

extern void* WPuppet_ctor(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
int WPuppet_RegisterResourceNameUse(void*);

extern char* strcpy(char*, const char*);
extern void appFreeUni__FPv(void*);
struct B3CCVector;
struct B3CCElem;
extern void PuppetMaterialVector_InsertIteratorRange(B3CCVector*, B3CCElem*, B3CCElem**, B3CCElem**);
extern void __destroy_new_array(void*, void*);
extern void WPuppetBone_Init(void*);
extern void WPuppetBone_dtor(void*, int);
extern void WPuppetBone_CopyState(void*, void*);
extern void Vec3_Zero(void*);
extern void AABB_Copy(void*, void*);
extern void appFreeUni__FPv(void*);
extern "C" int GetChunkElementCount__Q23gtl20IGenericResourceFileFi(void*, int);
extern void __destroy_new_array(void*, void*);

extern void* __nwa__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);

extern int WPuppet_LinkBonesFromModelResource(void*, void*, int, void*);
extern void WPuppet_Share(void*, void*);
extern const PuppetPreloadStrings puppetPreloadStrings;
extern DATA_SECTION(".data") void* __vt__Q23std11logic_error;
extern DATA_SECTION(".data") void* s_sharedCharDeleterVtable;
extern DATA_SECTION(".data") void* __vt__Q23std12length_error;
extern void* __dt__Q23std12length_errorFv(void*, int);
extern void __dla__FPv(void*);
struct Copy502StringGuard {
    char* text;
    char* allocator;
    // Release a message array until the exception's shared owner takes it.
    ~Copy502StringGuard() {
        if (text)
            __dla__FPv(text);
    }
};
struct Copy502Record {
    unsigned int words[60];
};
struct Copy502Bone {
    char data[kBoneBytes];
    // Initialize one bone in the clone's owned array.
    Copy502Bone() {
        WPuppetBone_Init(this);
    }
    // Destroy bone contents without freeing this array element individually.
    ~Copy502Bone() {
        WPuppetBone_dtor(this, -1);
    }
};
struct Copy502ErrorBase {
    void* vtable;
    char* text;
    void* inner;
    // Copy a borrowed error message into shared-owned array storage.
    Copy502ErrorBase(const char* message) {
        vtable = &__vt__Q23std11logic_error;
        volatile char baseAllocator __attribute__((aligned(4))) = 0;
        char stringAllocator __attribute__((aligned(4))) = 0;
        unsigned int messageSize = strlen(message) + 1;
        char* buffer = (char*)__nwa__FUl(messageSize * 2 - messageSize);
        text = buffer;
        {
            Copy502StringGuard guard = {buffer, &stringAllocator};
            void* p = __nw__FUl(16);
            if (p) {
                ((PuppetSharedStringOwner*)p)->strongReferences = 1;
                ((PuppetSharedStringOwner*)p)->weakReferences = 1;
                ((PuppetSharedStringOwner*)p)->vtable = &s_sharedCharDeleterVtable;
                ((PuppetSharedStringOwner*)p)->text = buffer;
            }
            inner = p;
            guard.text = 0;
        }
        strcpy(text, message);
    }
    // Empty ABI base cleanup; the runtime's supplied exception destructor releases ownership.
    ~Copy502ErrorBase() {}
};
struct Copy502Error : Copy502ErrorBase {
    // Finish the length_error payload and transfer it to the exception runtime.
    Copy502Error(const char* message) : Copy502ErrorBase(message) {
        vtable = &__vt__Q23std12length_error;
        __throw((const char*)(puppetPreloadStrings.lengthErrorType), this, __dt__Q23std12length_errorFv);
    }
};
struct Copy502Object {
    char data[0x23c];
    // Construct the clone's initially empty puppet storage.
    Copy502Object() {
        WPuppet_ctor(this);
    }
};
struct Copy502Vector {
    void* data;
    unsigned int size;
    unsigned int capacity;
    // Start an empty temporary owner for material-record storage.
    Copy502Vector() : data(0), size(0), capacity(0) {}
    // Free the temporary's backing storage without recursively deleting copied records.
    ~Copy502Vector() {
        if (data) {
            unsigned int n = *(volatile unsigned int*)&size;
            volatile char allocator __attribute__((aligned(4))) = 0;
            size = n - n;
            appFreeUni__FPv(data);
        }
    }
};

#pragma opt_common_subs off
// Clone the model instance, retaining shared resources and copying owned materials and bones.
// Ownership map: return a newly allocated WPuppet. modelResource is cloned when
// animation/load mode requires it, otherwise retained; animation and nameIndexMap
// are retained separately. boneMatrices is shared by resource-name registration;
// skinMatrices starts null. materialMetadata/keyRange and hierarchyContext are
// copied aliases. materials are copied as records (not recursively deep-copied),
// modelLayerVisibilityWords gets a new byte buffer, bones gets new constructed records.
// ApplyResourceMetadata copies the resource name, aliases keyRecords/ranges,
// creates a new namedBones table remapped to destination bones, and borrows bounds.
void* WPuppet_MakeClone(void* src) {
    int offset;
    int i;
    // Phase 1: establish the new object and retain/clone its shared dependencies.
    Copy502Object* dst = new Copy502Object;
    WPuppet_RegisterResourceNameUse(src);
    unsigned int inherited;
    if (!AsWPuppet(src)->modelResource)
        inherited = 0;
    else {
        unsigned int result = 0;
        if ((AsWPuppet(src)->animation && ((CPetAnimation*)(AsWPuppet(src)->animation))->GetEntryCount() > 0) ||
            AsWPuppet(src)->loadFlags == kCloneModelLoadMode)
            ((gtl::IGenericModelingResource*)(AsWPuppet(src)->modelResource))
                ->Clone(1, (gtl::IGenericModelingResource**)&result);
        else {
            result = AsWPuppet(src)->modelResourceWord;
            ++*(int*)result;
        }
        inherited = result;
    }
    AsWPuppet(dst)->modelResourceWords = (unsigned int*)inherited;
    {
        void* p = AsWPuppet(src)->animation;
        if (p) {
            AsWPuppet(dst)->animation = p;
            ++*(int*)p;
        }
    }
    void* p = AsWPuppet(src)->nameIndexMap;
    if (p) {
        AsWPuppet(dst)->nameIndexMap = p;
        ++*(int*)p;
    }
    AsWPuppet(dst)->boneMatrices = AsWPuppet(src)->boneMatrices;
    AsWPuppet(dst)->skinMatrices = 0;
    AsWPuppet(dst)->materialMetadata = AsWPuppet(src)->materialMetadata;
    AsWPuppet(dst)->keyRange = AsWPuppet(src)->keyRange;
    volatile char clearAllocator __attribute__((aligned(4))) = 0;
    unsigned int oldCount = *(volatile unsigned int*)&AsWPuppet(dst)->materialCount;
    AsWPuppet(dst)->materialCount = oldCount - oldCount;

    // Phase 2: reserve material records using a temporary owner, then copy the range.
    int count = AsWPuppet(src)->materialCount;
    unsigned int cap = AsWPuppet(dst)->materialCapacity;
    if (count > cap) {
        Copy502Vector temp;
        volatile char tempAllocator __attribute__((aligned(4))) = 0;
        if ((unsigned int)count > 0x1111111) {
            const char* message = puppetPreloadStrings.vectorLengthError;
            Copy502Error error(message);
        }
        temp.data = appMallocDummy__FUl(count * kMaterialBytes);
        temp.capacity = count;
        Copy502Record* to;
        Copy502Record* end;
        Copy502Record* from;
        end = (Copy502Record*)AsWPuppet(dst)->materials + AsWPuppet(dst)->materialCount;
        from = (Copy502Record*)AsWPuppet(dst)->materials;
        to = (Copy502Record*)temp.data + temp.size;
        volatile char copyAllocator __attribute__((aligned(4))) = 0;
        while (from < end) {
            if (to) {
                try {
                    *to = *from;
                } catch (...) {
                    throw;
                }
            }
            ++from;
            ++to;
            ++temp.size;
        }
        *(volatile unsigned int*)&AsWPuppet(dst)->materialCount = 0;
        unsigned int newcap = temp.capacity;
        void* fresh = temp.data;
        unsigned int oldcap = AsWPuppet(dst)->materialCapacity;
        unsigned int newsize = temp.size;
        AsWPuppet(dst)->materialCapacity = newcap;
        void* old = AsWPuppet(dst)->materials;
        volatile char swapAllocator __attribute__((aligned(4))) = 0;
        AsWPuppet(dst)->materials = fresh;
        unsigned int oldsize = *(volatile unsigned int*)&AsWPuppet(dst)->materialCount;
        temp.capacity = oldcap;
        temp.data = old;
        AsWPuppet(dst)->materialCount = newsize;
        temp.size = oldsize;
    }
    unsigned int rangeCount = AsWPuppet(src)->materialCount;
    void* position = AsWPuppet(dst)->materials;
    void* begin = AsWPuppet(src)->materials;
    void* end = (char*)begin + rangeCount * kMaterialBytes;
    void* volatile endIterator = end;
    void* volatile beginIterator = begin;
    void* volatile positionIterator = position;
    void* volatile positionCopy = position;
    volatile char iteratorAllocator __attribute__((aligned(4))) = 0;
    char allocator __attribute__((aligned(4))) = 0;
    void* rangeEnd = end;
    void* rangeBegin = begin;
    ((void (*)(void*, void*, void*, void*, void*))PuppetMaterialVector_InsertIteratorRange)(
        (char*)&AsWPuppet(dst)->materials, position, &rangeBegin, &rangeEnd, &allocator);
    // Phase 3: copy owned visibility-word/bone arrays; rebuild the destination bone links.
    if (AsWPuppet(src)->modelLayerVisibilityWords && AsWPuppet(src)->modelResource) {
        int bytes =
            GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(src)->modelResource, kChunkModelLayers) * 4;
        void* copy = __nwa__FUl(bytes);
        AsWPuppet(dst)->modelLayerVisibilityWords = copy;
        Memory_Copy(copy, AsWPuppet(src)->modelLayerVisibilityWords, bytes);
    }
    if (AsWPuppet(dst)->bones) {
        __destroy_new_array(AsWPuppet(dst)->bones, WPuppetBone_dtor);
        AsWPuppet(dst)->bones = 0;
    }
    count = AsWPuppet(src)->boneCount;
    if (count > 0) {
        *(Copy502Bone**)&AsWPuppet(dst)->bones = new Copy502Bone[count];
        offset = 0;
        for (i = 0; i < (count = AsWPuppet(src)->boneCount); ++i) {
            WPuppetBone_CopyState((char*)AsWPuppet(dst)->bones + offset, (char*)AsWPuppet(src)->bones + offset);
            offset += kBoneBytes;
        }
    }
    AsWPuppet(dst)->boneCount = count;
    AsWPuppet(dst)->boneRoot =
        (void*)WPuppet_LinkBonesFromModelResource(dst, AsWPuppet(dst)->bones, count, AsWPuppet(dst)->modelResource);
    // Phase 4: ABI payload for PuppetMetadataView, in byte order:
    // name +00, root +04, duration +08, triangleCount +0c,
    // keyRecordCount +10, keyRecords +14, minimum +18, maximum +24,
    // namedBones +30, namedBoneCount +34, animationRanges +38.
    // Pointer members refer to source metadata; applying them does not transfer ownership.
    struct {
        void* name;
        int root;
        float duration;
        int triangleCount;
        int keyRecordCount;
        int keyRecords;
        unsigned int minimum[3];
        unsigned int maximum[3];
        int namedBones, namedBoneCount, animationRanges;
    } setup;
    Vec3_Zero(setup.minimum);
    Vec3_Zero(setup.maximum);
    *(char*)&AsWPuppet(dst)->hasResourceName = 1;
    setup.name = AsWPuppet(src)->resourceName;
    setup.keyRecords = *(int*)&AsWPuppet(src)->keyRecords;
    setup.triangleCount = *(int*)&AsWPuppet(src)->triangleCount;
    AABB_Copy(setup.minimum, (char*)&AsWPuppet(src)->boundsMin);
    setup.root = *(int*)&AsWPuppet(src)->boneRoot;
    setup.namedBones = *(int*)&AsWPuppet(src)->namedBones;
    setup.namedBoneCount = *(int*)&AsWPuppet(src)->namedBoneCount;
    *(int*)&AsWPuppet(dst)->boundObjects = *(int*)&AsWPuppet(src)->boundObjects;
    setup.animationRanges = *(int*)&AsWPuppet(src)->animationRanges;
    setup.keyRecordCount = AsWPuppet(src)->keyRecordCount;
    setup.duration = AsWPuppet(src)->animationDuration;
    AsWPuppet(dst)->loadFlags = AsWPuppet(src)->loadFlags;
    *(int*)&AsWPuppet(dst)->hierarchyContext = *(int*)&AsWPuppet(src)->hierarchyContext;
    WPuppet_Share(dst, &setup);
    AsWPuppet(dst)->boneSortKey = AsWPuppet(src)->boneSortKey;
    return dst;
}
#pragma opt_common_subs reset

void WPuppetBone_SetBasicMatrix(void*, void*);
void* appMallocDummy__FUl(unsigned int);
int sprintf(char*, const char*, ...);
void* WPuppetBone_FindBone(void*, const char*, int);
int WPuppet_ResolveDefaultBone(void* arg0);

struct Fn53834Node {
    Fn53834Node* prev;
    Fn53834Node* next;
    void* data;
};
struct Fn53834Guard {
    void* owner;
    void* node;
    // Free a child-list node unless its insertion transferred ownership.
    ~Fn53834Guard() {
        if (node)
            appFreeUni__FPv(node);
    }
};
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_loop_invariants off
// Publish an allocated child node before a list position; payload ownership stays borrowed.
#define SplicePuppetChildNodeBefore(self, position, alloc)                        \
    {                                                                             \
        ((WPuppetListNode*)((WPuppetListNode*)(position))->prev)->next = (alloc); \
        ((WPuppetListNode*)(alloc))->prev = ((WPuppetListNode*)(position))->prev; \
        ((WPuppetListNode*)(position))->prev = (alloc);                           \
        ((WPuppetListNode*)(alloc))->next = (position);                           \
        ++AsWPuppet(self)->childCount;                                            \
    }

// Copy child bone matrices to this model, then insert the borrowed child by nondecreasing sort key.
void WPuppet_InsertChildBySortKey(void* self, void* child, void* unusedA, void* unusedB) {
    (void)unusedA;
    (void)unusedB;
    // Both model chains must have equal length and matching bone order.
    // The OR condition intentionally requires both cursors valid on every iteration;
    // strcmp is observational only: its result does not validate or align the chains.
    // Matrices flow FROM child bones TO this parent; child-list nodes own no puppet.
    void* sourceBone;
    void* destinationBone = AsWPuppet(self)->boneRoot;
    sourceBone = AsWPuppet(child)->boneRoot;
    while (destinationBone || sourceBone) {
        strcmp((char*)destinationBone + 8, (char*)sourceBone + 8);
        WPuppetBone_SetBasicMatrix(destinationBone, (char*)&((PuppetBoneView*)sourceBone)->initialMatrix);
        destinationBone = *(void**)&((PuppetBoneView*)destinationBone)->next;
        sourceBone = *(void**)&((PuppetBoneView*)sourceBone)->next;
    }
    // Find the first child whose key is >= the incoming key (ties insert before).
    void* volatile cursor;
    void* volatile initial;
    cursor = initial = AsWPuppet(self)->childHead;
    while (cursor != (char*)&AsWPuppet(self)->childTail) {
        if (AsWPuppet(((WPuppetListNode*)cursor)->value)->boneSortKey >= AsWPuppet(child)->boneSortKey)
            break;
        cursor = ((WPuppetListNode*)cursor)->next;
    }
    void* insertionPosition = cursor;
    void* sentinel = (char*)&AsWPuppet(self)->childTail;
    if (insertionPosition != sentinel) {
        volatile bool flag __attribute__((aligned(4))) = false;
        // Allocate a node; volatile mirrors retain the existing cleanup expansion.
        void* alloc = appMallocDummy__FUl(12);
        void* volatile allocB = alloc;
        void* volatile allocA = alloc;
        Fn53834Guard guard;
        guard.owner = (char*)&AsWPuppet(self)->childTail;
        guard.node = alloc;
        void* slot = (char*)alloc + 8;
        if (slot) {
            try {
                *(void**)slot = child;
            } catch (...) {
                throw;
            }
        }
        guard.node = 0;
        SplicePuppetChildNodeBefore(self, insertionPosition, alloc);
    } else {
        volatile bool flag __attribute__((aligned(4))) = false;
        // Allocate a node; volatile mirrors retain the existing cleanup expansion.
        void* alloc = appMallocDummy__FUl(12);
        void* volatile allocB = alloc;
        void* volatile allocA = alloc;
        Fn53834Guard guard;
        guard.owner = (char*)&AsWPuppet(self)->childTail;
        guard.node = alloc;
        void* slot = (char*)alloc + 8;
        if (slot) {
            try {
                *(void**)slot = child;
            } catch (...) {
                throw;
            }
        }
        guard.node = 0;
        SplicePuppetChildNodeBefore(self, sentinel, alloc);
    }
}
#undef SplicePuppetChildNodeBefore
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset

int CString_Compare(const char* arg0, const char* arg1);
void WPuppet_SetBoneVisibilityRecursive(void* arg0, int arg1, int arg2, const char* arg3);
extern const PuppetPreloadStrings puppetPreloadStrings;
extern float kFloat_0_puppet;
int List_GetByteByKey(void* arg0, int arg1, unsigned char* arg2);
void WPuppetBone_SetAlpha(void* arg0, int arg1, int arg2);
extern void* g_resourceManager;

extern int WPuppet_RefreshMatrixBuffers(void*);
extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern int WPuppet_AllocateSkinMatrices(void*);
extern void ResourcePath_NormalizeImageExtension(void*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const char*, const char*);
extern const char s_puppetResourceNameRefCounts[];
extern void appFreeUni__FPv(void*);

#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma opt_propagation off
// Return the shorter length by reference; reference lifetime preserves the original inlined string comparison.
static inline const unsigned int& PuppetCommonPrefixLength(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Return one when the word is zero, and zero otherwise.
static inline int PuppetWordIsZero(int value) {
    return value == 0;
}
// Order unequal-length string prefixes after their common bytes compare equal.
static inline int PuppetCompareKeyLengths(unsigned int a, unsigned int b) {
    if (a < b)
        return -1;
    if (a == b)
        return 0;
    return 1;
}
// Does the process-wide metadata tree contain this puppet's exact resource name?
// self is required; hasResourceName==0 returns false without touching the tree.
// Own a temporary copied key, borrow tree links/strings, release only that copy.
// Root pointer is stored at header+4; that storage ADDRESS is also end(), never
// a dereferenceable key node. header is s_puppetResourceNameRefCounts.
// Ordering is memcmp(common prefix), then unsigned length; case is significant.
// This is a membership query, not an increment of the node's useCount.
static inline bool HasSharedPuppetResourceName(void* self) {
    // Phase 1: own a temporary copy of the resource-name key (inline or heap).
    if (!AsWPuppet(self)->hasResourceName)
        return false;
    // Keep a local query type: removing it renumbers later local cleanup-class
    // symbols, which are part of this unit's emitted exception/destructor ABI.
    // Inline bytes overlay these heap words through PuppetInlineStringBytes.
    struct PuppetResourceQueryKey {
        unsigned int flags, heapLength;
        char* heapData;
    } query = {};
    unsigned int length;
    const char* name;
    name = (const char*)AsWPuppet(self)->resourceName;
    length = strlen(name);
    String_ReserveCapacity(&query, length);
    struct F54Alloc {
        volatile char value;
        char pad[3];
    } uninit;
    char local = uninit.value;
    __typeof__(length) sourceLengthCopy = length;
    ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(&query, 0, 0, name,
                                                                                      name + sourceLengthCopy, &local);
    int heap = (query.flags >> 31);
    __typeof__(heap) heapTagCopy = heap;
    int size = query.heapLength;
    unsigned int isHeap = PuppetWordIsZero(PuppetWordIsZero(heapTagCopy));
    unsigned int queryInlineLength = ((PuppetInlineStringBytes*)&query)->lengthAndFlags & 0x7f;
    const char* data = query.heapData;
    void* searchNode = *(void**)(s_puppetResourceNameRefCounts + 4);
    void* candidate = (void*)(4 + s_puppetResourceNameRefCounts);
    // Phase 2: lower_bound in a string-keyed tree; candidate starts as the end sentinel.
    // Compare common bytes first, then lengths. Temporary length copies preserve
    // the reference-returning minimum helper's lifetimes in the original code.
    while (searchNode) {
        unsigned int queryLength;
        const void* queryText;
        if (isHeap == 0) {
            queryLength = queryInlineLength;
            queryText = ((PuppetInlineStringBytes*)&query)->text;
        } else {
            queryText = data;
            __typeof__(size) heapLengthCopy = size;
            queryLength = heapLengthCopy;
        }
        unsigned int nodeStoredLength = !(((PuppetNameNodeView*)searchNode)->key.heap.flags >> 31)
                                            ? (((PuppetNameNodeView*)searchNode)->key.small.lengthAndFlags & 0x7f)
                                            : ((PuppetNameNodeView*)searchNode)->key.heap.length;
        const void* nodeKey;
        unsigned int nodeKeyLength;
        if (!(((PuppetNameNodeView*)searchNode)->key.heap.flags >> 31)) {
            nodeKey = ((PuppetNameNodeView*)searchNode)->key.small.text;
            nodeKeyLength = ((PuppetNameNodeView*)searchNode)->key.small.lengthAndFlags & 0x7f;
        } else {
            nodeKey = ((PuppetNameNodeView*)searchNode)->key.heap.data;
            nodeKeyLength = ((PuppetNameNodeView*)searchNode)->key.heap.length;
        }
        unsigned int nodeLengthCopy = nodeKeyLength;
        unsigned int nodeLength = PuppetCommonPrefixLength(nodeStoredLength, nodeLengthCopy);
        unsigned int queryLengthForCompare = queryLength;
        int keyOrder =
            memcmp((void*)nodeKey, (void*)queryText, PuppetCommonPrefixLength(queryLengthForCompare, nodeLength));
        if (keyOrder == 0)
            keyOrder = PuppetCompareKeyLengths(nodeLength, queryLength);
        if (keyOrder >= 0) {
            candidate = searchNode;
            searchNode = ((PuppetNameNodeView*)searchNode)->left;
        } else
            searchNode = ((PuppetNameNodeView*)searchNode)->right;
    }
    // Phase 3: reject a greater lower_bound key; only equality proves shared ownership.
    if (candidate != (void*)(4 + s_puppetResourceNameRefCounts)) {
        unsigned int queryLength;
        const void* candidateText;
        unsigned int candidateLength;
        if (!(((PuppetNameNodeView*)candidate)->key.heap.flags >> 31)) {
            candidateText = ((PuppetNameNodeView*)candidate)->key.small.text;
            candidateLength = ((PuppetNameNodeView*)candidate)->key.small.lengthAndFlags & 0x7f;
        } else {
            candidateText = ((PuppetNameNodeView*)candidate)->key.heap.data;
            candidateLength = ((PuppetNameNodeView*)candidate)->key.heap.length;
        }
        __typeof__(candidateLength) candidateLengthCopy = candidateLength;
        __typeof__(queryInlineLength) inlineLengthCopy = queryInlineLength;
        unsigned int queryStoredLength = isHeap == 0 ? inlineLengthCopy : query.heapLength;
        const void* queryKey;
        if (isHeap == 0) {
            queryLength = queryInlineLength;
            queryKey = ((PuppetInlineStringBytes*)&query)->text;
        } else {
            queryKey = query.heapData;
            queryLength = query.heapLength;
        }
        unsigned int queryLengthCopy = queryLength;
        unsigned int compareLength = PuppetCommonPrefixLength(queryStoredLength, queryLengthCopy);
        int keyOrder =
            memcmp((void*)queryKey, (void*)candidateText, PuppetCommonPrefixLength(candidateLengthCopy, compareLength));
        if (0 == keyOrder)
            keyOrder = PuppetCompareKeyLengths(compareLength, candidateLength);
        if (keyOrder < 0)
            candidate = (void*)(4 + s_puppetResourceNameRefCounts);
    }
    if (heap)
        appFreeUni__FPv(query.heapData);
    return candidate != (void*)(4 + s_puppetResourceNameRefCounts);
}
// Delete bone matrices only when no shared resource-name entry owns their lifetime.
static inline int ReleaseUnsharedPuppetBoneMatrices(void* self) {
    if (HasSharedPuppetResourceName(self))
        return 0;
    void* old = AsWPuppet(self)->boneMatrices;
    if (old) {
        __dla__FPv(old);
        AsWPuppet(self)->boneMatrices = 0;
    }
    return 1;
}

#pragma use_lmw_stmw on
// Replace unshared bone matrices and evaluate the model resource's default transforms.
int WPuppet_RefreshMatrixBuffers(void* self) {
    if (self) {
        if (ReleaseUnsharedPuppetBoneMatrices(self)) {
            void* resource = AsWPuppet(self)->modelResource;
            if (!resource && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
                void* parent =
                    AsWPuppet(self)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
                if (parent)
                    resource = AsWPuppet(parent)->modelResource;
            }
            int count =
                resource ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(resource, kChunkBoneTransforms) : 0;
            if (count) {
                AsWPuppet(self)->boneMatrices = __nwa__FUl(count * 0x30);
                resource = AsWPuppet(self)->modelResource;
                if (!resource && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
                    void* parent =
                        AsWPuppet(self)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
                    if (parent)
                        resource = AsWPuppet(parent)->modelResource;
                }
                if (resource)
                    ((gtl::IGenericModelingResource*)resource)
                        ->EvaluateTransformAnimation(AsWPuppet(self)->boneMatrices, (void*)(long)count);
            }
        }
        WPuppet_AllocateSkinMatrices(self);
        if (!*(int*)&AsWPuppet(self)->animation && AsWPuppet(self)->loadFlags != 0x82)
            ReleaseUnsharedPuppetBoneMatrices(self);
    }
    return 1;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma opt_propagation reset

// Show the named bone, or all descendants when the name is absent or the Recursive keyword is used.
void WPuppet_ShowBoneSelection(void* self, const char* boneName) {
    if (boneName && CString_Compare(boneName, puppetPreloadStrings.recursiveKeyword)) {
        WPuppet_SetBoneVisibilityRecursive(self, 0, 0, boneName);
    } else {
        WPuppet_SetBoneVisibilityRecursive(self, 1, 0, 0);
    }
}

// Hide the named bone, or all descendants when the name is absent or the Recursive keyword is used.
void WPuppet_HideBoneSelection(void* self, const char* boneName) {
    if (boneName && CString_Compare(boneName, puppetPreloadStrings.recursiveKeyword)) {
        WPuppet_SetBoneVisibilityRecursive(self, 0, 1, boneName);
    } else {
        WPuppet_SetBoneVisibilityRecursive(self, 1, 1, 0);
    }
}

// Find the first exact byte substring at or after the requested position; return -1 when absent.
int String_FindSubstring(void* string, const char* needle, unsigned int startPosition, unsigned int needleLength) {
    const char* data;
    unsigned int length;
    if (!(AsMWStringStorage(string)->flags >> kStringHeapFlagBit)) {
        data = ((PuppetStringKeyStorage*)string)->small.text;
        length = ((PuppetStringKeyStorage*)string)->small.lengthAndFlags & kStringInlineLengthMask;
    } else {
        data = AsMWStringStorage(string)->data;
        length = AsMWStringStorage(string)->length;
    }
    if (startPosition <= length) {
        unsigned int remaining = length - startPosition;
        const char* end = needle + needleLength;
        const char* candidate = data + startPosition;
        while (remaining >= needleLength) {
            const char* text = candidate;
            const char* key = needle;
            // Structured break/flag and inline-predicate forms change this counted loop's code.
            for (; key < end; ++key, ++text) {
                if ((signed char)*key != (signed char)*text)
                    goto mismatch;
            }
            return candidate - data;
        mismatch:
            ++candidate;
            --remaining;
        }
    }
    return -1;
}

extern int WPuppet_ReleaseNamedBoneData(void* self);

// Find the bounds-name entry selected by an offset in the confirmed string pool.
static inline char* FindNamedBoneByKeyOffset(void* self, int keyOffset) {
    unsigned int count = AsWPuppet(self)->namedBoneCount;
    unsigned int off = 0;
    unsigned int i = 0;
    const char* key = (const char*)&puppetPreloadStrings + keyOffset;
    for (; i < count; off += 8, ++i) {
        char* record = *(char**)&AsWPuppet(self)->namedBones + off;
        if (record && ((PuppetNamedBoneView*)record)->bounds &&
            strcmp(key, ((PuppetBoundsView*)((PuppetNamedBoneView*)record)->bounds)->name) == 0)
            return record;
    }
    return 0;
}
// Apply shared model metadata, resolve named bones, and choose bound or center as the bounds anchor.
void WPuppet_Share(void* self, void* src) {
    int i;
    unsigned int off;
    strcpy(AsWPuppet(self)->resourceName, ((PuppetMetadataView*)src)->name);
    *(unsigned int*)&AsWPuppet(self)->keyRecords = *(unsigned int*)&((PuppetMetadataView*)src)->keyRecords;
    *(unsigned int*)&AsWPuppet(self)->keyRecordCount = *(unsigned int*)&((PuppetMetadataView*)src)->keyRecordCount;
    AsWPuppet(self)->triangleCount = *(unsigned int*)&((PuppetMetadataView*)src)->triangleCount;
    *(unsigned int*)&AsWPuppet(self)->animationRanges = *(unsigned int*)&((PuppetMetadataView*)src)->animationRanges;
    AABB_Copy((char*)&AsWPuppet(self)->boundsMin, (char*)&((PuppetMetadataView*)src)->minimum);
    AsWPuppet(self)->animationDuration = ((PuppetMetadataView*)src)->duration;
    WPuppet_ReleaseNamedBoneData(self);
    int count = *(int*)&((PuppetMetadataView*)src)->namedBoneCount;
    AsWPuppet(self)->namedBones = 0;
    if (count > 0) {
        unsigned int bytes = count * 8;
        void* data = __nwa__FUl(bytes);
        AsWPuppet(self)->namedBones = data;
        Memory_Clear(data, bytes);
    }
    *(int*)&AsWPuppet(self)->namedBoneCount = count;
    for (i = 0, off = 0; i < count; off += 8, ++i) {
        char* entry = (char*)(*(unsigned int*)&((PuppetMetadataView*)src)->namedBones + off);
        if (entry) {
            char** table = *(char***)&AsWPuppet(self)->namedBones;
            if (table) {
                char** slot = (char**)((char*)table + off);
                slot[1] = *(char**)(entry + 4);
                void* name = *(void**)entry;
                slot[0] = name ? (char*)WPuppetBone_FindBone(AsWPuppet(self)->boneRoot, (char*)name + 8, 0) : 0;
            }
        }
    }
    void* found = FindNamedBoneByKeyOffset(self, PUPPET_OFFSET(PuppetPreloadStrings, boundBone));
    if (!found)
        found = FindNamedBoneByKeyOffset(self, PUPPET_OFFSET(PuppetPreloadStrings, centerBone));
    if (found) {
        AsWPuppet(self)->boundsBone = ((PuppetNamedBoneView*)found)->bone;
        AsWPuppet(self)->boundsData = ((PuppetNamedBoneView*)found)->bounds;
    }
    void* name = AsWPuppet(self)->boundsBone;
    if (name)
        AsWPuppet(self)->boundsBone = WPuppetBone_FindBone(AsWPuppet(self)->boneRoot, ((PuppetBoneView*)name)->name, 0);
}
// Reserved virtual hook; the base puppet performs no work.
void WPuppet_vfunc_3() {}
// Reserved virtual hook; the base puppet performs no work.
void WPuppet_vfunc_6() {}
extern void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
extern void Resource_Release(void*);
extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void* ResourceHandle_dtor(void*, int);
extern char* strcpy(char*, const char*);
extern void WPuppet_LoadFromChunks(void*, void*, void*, int, void*, int);
struct PathHandle {
    void** value;
    // Release the resource held in the borrowed slot and clear that slot.
    ~PathHandle() {
        void* p = *value;
        if (p) {
            if (p) {
                Resource_Release(p);
                *value = 0;
            }
        }
    }
};
#pragma use_lmw_stmw on
struct WPFile_ {
    virtual void a0();
    virtual void a1();
    virtual void rd(void*, unsigned int);
};
// Read a model file, remember its basename and directory, and dispatch the requested chunk loaders.
int WPuppet_LoadPET(void* self, const char* name, int loadMode, int loadFlags) {
    void* value = Resource_OpenResolvedPath(AsWPuppet(self)->resourceManager, name, -1, 0, 0, 0);
    PathHandle handle = {&value};
    if (!value)
        return 1;
    char path[0x200];
    char tail[0x200];
    strcpy(path, name);
    char* slash = strrchr(path, '/');
    slash = slash ? slash : path;
    strcpy(slash, puppetPreloadStrings.emptyPath);
    slash = strrchr((char*)name, '/');
    strcpy(tail, slash ? slash + 1 : name);
    strcpy(AsWPuppet(self)->resourceName, tail);
    void* buffer = __nwa__FUl(*(unsigned int*)((char*)value + 4));
    unsigned int size = *(unsigned int*)((char*)value + 4);
    ((WPFile_*)value)->rd(buffer, size);
    if (value) {
        Resource_Release(value);
        value = 0;
    }
    char* pathArg;
    pathArg = (!path[0]) ? 0 : path;
    WPuppet_LoadFromChunks(self, buffer, (void*)size, loadMode, pathArg, loadFlags);
    if (buffer)
        __dla__FPv(buffer);
    if ((loadFlags & 8) && !AsWPuppet(self)->modelResource)
        return 1;
    return 0;
}
#pragma use_lmw_stmw reset
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const char*, const char*);

extern void* __dt__Q23std12out_of_rangeFv(void*, int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void* __vt__Q23std11logic_error;
extern void* s_sharedCharDeleterVtable;
struct D408Exc {
    void* vt;
    char* text;
    unsigned int* rep;
};
extern DATA_SECTION(".data") void* __vt__Q23std12out_of_range;
// Select the substring length without extending beyond the source string.
static inline const unsigned int& d408min(const unsigned int& a, const unsigned int& b) {
    const unsigned int* p = &b;
    if (a < b)
        p = &a;
    return *p;
}
#pragma use_lmw_stmw on
// Construct an inline or heap string from a checked substring, throwing for an invalid start position.
void String_ConstructSubstring(void* self, void* input, unsigned int offset, unsigned int maxCount) {
    unsigned int tag = *(unsigned int*)input;
    ((unsigned int*)self)[0] = 0;
    ((unsigned int*)self)[1] = 0;
    ((unsigned int*)self)[2] = 0;
    unsigned int length;
    if ((tag >> 31) == 0) {
        length = *(unsigned char*)input & 0x7f;
    } else {
        length = AsMWStringStorage(input)->length;
    }
    unsigned int reserve;
    if (offset <= length) {
        const unsigned int initial = length - offset;
        reserve = d408min(initial, maxCount);
    } else {
        reserve = 0;
    }
    String_ReserveCapacity(self, reserve);
    unsigned int count = maxCount;
    void* source;
    unsigned int sourceLength;
    if ((*(unsigned int*)input >> 31) == 0) {
        source = (char*)input + 1;
        sourceLength = *(unsigned char*)input & 0x7f;
    } else {
        source = *(void**)&AsMWStringStorage(input)->data;
        sourceLength = AsMWStringStorage(input)->length;
    }
    if (offset > sourceLength) {
        const char* message = puppetPreloadStrings.substringRangeError;
        char alloc __attribute__((aligned(4))) = 0;
        D408Exc e;
        D408Exc* mp = &e;
        mp->vt = &__vt__Q23std11logic_error;
        unsigned int len = strlen(message);
        char* text = (char*)__nwa__FUl((len + 1) * 2 - (len + 1));
        mp->text = text;
        {
            Copy502StringGuard guard = {text, &alloc};
            unsigned int* rep = (unsigned int*)__nw__FUl(0x10);
            if (rep) {
                rep[1] = 1;
                rep[2] = 1;
                rep[0] = (unsigned int)&s_sharedCharDeleterVtable;
                rep[3] = (unsigned int)text;
            }
            mp->rep = rep;
            guard.text = 0;
        }
        strcpy(mp->text, message);
        mp->vt = &__vt__Q23std12out_of_range;
        __throw((const char*)(puppetPreloadStrings.outOfRangeType), mp, (void*)__dt__Q23std12out_of_rangeFv);
    }
    source = (char*)source + offset;
    const unsigned int remaining = sourceLength - offset;
    count = d408min(remaining, count);
    volatile char stackByte;
    char local = stackByte;
    ((void (*)(void*, int, int, const char*, const char*, const char*))String_ReplaceRange)(
        self, 0, 0, (const char*)source, (const char*)source + count, &local);
}
#pragma use_lmw_stmw reset
// Enable the per-puppet light parameters used by the rendering entry points.
void WPuppet_SetRenderParamsEnabled(void* self, unsigned int enabled) {
    *(unsigned int*)&AsWPuppet(self)->renderParamsEnabled = enabled;
}
extern void gtl_GenericRenderer_SetCullingMode(void*);
extern void gtl_GenericRenderer_SetProjectionMatrix(void*);
extern void gtl_GenericRenderer_SetViewMatrix(void*);
extern void Engine_GetLightEnabled(int, void*);
extern void Engine_SetLightEnabled(int, void*);
extern void Engine_SetLight(int, void*);
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Camera_Apply(void*, float, float);
extern void* Camera_GetRenderState(void*);
extern void SetTransform(int, void*);
extern void gtl_GenericRenderer_SetCullingMode(void*);
extern bool Copy_IndexedMatricesOrIdentity(void*, void*, unsigned int);
struct Fn50CD8Pair {
    unsigned int a, b;
};
struct Fn50CD8Pairs {
    Fn50CD8Pair p[16];
};
struct Fn50CD8Mtx {
    unsigned int w[12];
};
struct Fn50CD8Table {
    Fn50CD8Pairs pairs;
    Fn50CD8Mtx m1;
    Fn50CD8Mtx m2;
};
extern Fn50CD8Table s_puppetCameraRenderDefaults;
extern const unsigned int kFloatBits_neg1_5_PuppetTextureStage;
// Resolve a material-animation resource and prepare indexed matrices for rendering.
static inline int fn_80050CD8_ready(void* self, void* node) {
    if (!node)
        return 0;
    void* other = AsWPuppet(self)->layerController;
    if (!other)
        return 0;
    if (!AsWPuppet(self)->layerResource) {
        if (AsWPuppet(self)->modelResource) {
            if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource,
                                                                     kChunkMaterialLayers))
                AsWPuppet(self)->layerResource = AsWPuppet(self)->modelResource;
        } else if (AsWPuppet(self)->loadFlags == kContainerLoadMode) {
            void* sentinel = (char*)&AsWPuppet(self)->childTail;
            for (void* n = AsWPuppet(self)->childHead; n != sentinel; n = ((WPuppetListNode*)n)->next) {
                if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(
                        AsWPuppet(((WPuppetListNode*)n)->value)->modelResource, kChunkMaterialLayers)) {
                    AsWPuppet(self)->layerResource = AsWPuppet(((WPuppetListNode*)n)->value)->modelResource;
                    break;
                }
            }
        }
    }
    void* res = AsWPuppet(self)->layerResource;
    int ok;
    if (res)
        ok = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(res, kChunkMaterialTransforms);
    else
        ok = 0;
    if (!ok)
        return 0;
    if (!Copy_IndexedMatricesOrIdentity(other, node, ok))
        return 0;
    return 1;
}
// Apply camera matrices and puppet render state, draw children and the local model, then restore light state.
void WPuppet_RenderWithCamera(void* self, float delta, void* camera, void* arg2, void* arg3, void* arg4, void* arg5) {
    Fn50CD8Table* defaults = &s_puppetCameraRenderDefaults;
    gtl_GenericRenderer_SetCullingMode(AsWPuppet(self)->rendererOption);
    int savedSceneLight = 1;
    Engine_SetLightEnabled(kPuppetLightSlot, AsWPuppet(self)->renderParamsEnabled);
    if (AsWPuppet(self)->renderParamsEnabled) {
        Engine_SetLight(kPuppetLightSlot, (char*)&AsWPuppet(self)->renderParams);
        Engine_GetLightEnabled(kSceneLightSlot, &savedSceneLight);
        Engine_SetLightEnabled(kSceneLightSlot, 0);
    }
    Camera_Apply(camera, 0.0f, 0.0f);
    void* cameraState = Camera_GetRenderState(camera);
    if (*(unsigned int*)&((PuppetCameraRenderState*)cameraState)->dirtyMatrices & kProjectionMatrixDirty) {
        SetTransform(kProjectionTransform, (char*)&((PuppetCameraRenderState*)cameraState)->projectionMatrix);
        gtl_GenericRenderer_SetProjectionMatrix((char*)&((PuppetCameraRenderState*)cameraState)->projectionMatrix);
    }
    if (*(unsigned int*)&((PuppetCameraRenderState*)cameraState)->dirtyMatrices & kViewMatrixDirty) {
        SetTransform(kViewTransform, (char*)&((PuppetCameraRenderState*)cameraState)->viewMatrix);
        gtl_GenericRenderer_SetViewMatrix((char*)&((PuppetCameraRenderState*)cameraState)->viewMatrix);
    }
    Fn50CD8Pairs renderStates = defaults->pairs;
    renderStates.p[kPuppetRenderStateValueIndex].b = AsWPuppet(self)->renderStateValue;
    Fn50CD8Mtx* textureStateDefaults = &defaults->m1;
    Fn50CD8Mtx* textureParameterDefaults = &defaults->m2;
    Fn50CD8Mtx textureStates = *textureStateDefaults;
    Fn50CD8Mtx textureParameters = *textureParameterDefaults;
    textureParameters.w[7] = kFloatBits_neg1_5_PuppetTextureStage;
    Engine_SetRenderStateList(&renderStates, kRenderStateCount);
    Engine_SetTextureStageStateList(0, &textureStates, kTextureStateCount);
    Engine_SetTextureStageParameterList(0, &textureParameters, kTextureStateCount);
    void* layerMatrices = AsWPuppet(self)->layerMatrices;
    int layerMatrixAddress = 0;
    void* inheritedSkinMatrices = AsWPuppet(self)->skinMatrices;
    if (fn_80050CD8_ready(self, layerMatrices))
        layerMatrixAddress = *(int*)&AsWPuppet(self)->layerMatrices;
    void* sentinel = (char*)&AsWPuppet(self)->childTail;
    for (void* childNode = AsWPuppet(self)->childHead; sentinel != childNode;
         childNode = ((WPuppetListNode*)childNode)->next) {
        void* child = ((WPuppetListNode*)childNode)->value;
        void** vtable = *(void***)child;
        ((void (*)(void*, float, void*, void*, void*, void*, void*, void*, int))vtable[10])(
            child, delta, camera, arg2, arg3, arg4, arg5, inheritedSkinMatrices, layerMatrixAddress);
    }
    if (AsWPuppet(self)->modelResource) {
        void** vtable = *(void***)self;
        ((void (*)(void*, float, void*, void*, void*, void*, void*, void*, int))vtable[10])(
            self, delta, camera, arg2, arg3, arg4, arg5, 0, layerMatrixAddress);
    }
    Engine_SetLightEnabled(kPuppetLightSlot, 0);
    if (AsWPuppet(self)->renderParamsEnabled)
        Engine_SetLightEnabled(kSceneLightSlot, (void*)savedSceneLight);
    gtl_GenericRenderer_SetCullingMode(AsWPuppet(self)->rendererOption);
}
// Return the count-and-sentinel child-list storage used by list helpers.
char* WPuppet_GetChildList(void* self) {
    return (char*)&AsWPuppet(self)->childCount;
}
// Reserved puppet hook with no implementation.
void WPuppet_ReservedHook() {}
// Return the evaluated skin-matrix buffer.
void* WPuppet_GetSkinMatrices(void* self) {
    return AsWPuppet(self)->skinMatrices;
}
// Store the renderer option applied around camera-driven puppet rendering.
int WPuppet_SetRendererOption(void* self, unsigned int rendererOption) {
    *(unsigned int*)&AsWPuppet(self)->rendererOption = rendererOption;
    return 1;
}
// Set the switch selecting externally supplied skin matrices instead of animation evaluation.
int WPuppet_SetAnimationDisabled(void* self, unsigned char disabled) {
    AsWPuppet(self)->animationDisabled = disabled;
    return 1;
}

// Copy the complete per-puppet light/render parameter block without changing its enabled switch.
void WPuppet_SetRenderParams(void* self, const Params50BD4* parameters) {
    AsWPuppet(self)->renderParams = *parameters;
}

extern const float kFloat_255_PuppetOpacity;
// Set bone opacity and normalized material opacity, then propagate to every child.
void WPuppet_SetOpacityRecursive(void* self, unsigned int opacity) {
    if (AsWPuppet(self)->bones) {
        int i = 0;
        int offset = 0;
        while (i < AsWPuppet(self)->boneCount) {
            WPuppetBone_SetAlpha((char*)AsWPuppet(self)->bones + offset, opacity, 0);
            offset += kBoneBytes;
            ++i;
        }
    }
    if (AsWPuppet(self)->modelResource) {
        void* item = AsWPuppet(self)->materials;
        while (item != (char*)AsWPuppet(self)->materials + *(int*)&AsWPuppet(self)->materialCount * kMaterialBytes) {
            *(float*)&((PuppetMaterialView*)item)->opacity = ((float)opacity) / kFloat_255_PuppetOpacity;
            item = (char*)item + kMaterialBytes;
        }
    }
    void* end = (char*)&AsWPuppet(self)->childTail;
    void* node = AsWPuppet(self)->childHead;
    while (node != end) {
        WPuppet_SetOpacityRecursive(((WPuppetListNode*)node)->value, opacity);
        node = ((WPuppetListNode*)node)->next;
    }
}

int WPuppet_OverrideSkinMatrices(void* arg0, void* arg1, void* arg2);
struct Fn51320Guard {
    void* owner;
    void* node;
    // Free a hierarchy-list node whose insertion did not commit.
    ~Fn51320Guard() {
        if (node)
            appFreeUni__FPv(node);
    }
};
#pragma opt_common_subs off
// Normalize spaces in a bone name, cache successful lookups, or inherit the first child's default bone.
void* WPuppet_FindOrCacheBone(void* self, void* boneName) {
    void* inheritedBone;
    // One shared exit keeps the inherited-bone branches at their original positions.
    do {
        char normalizedName[0x100];
        if (boneName != 0) {
            sprintf(normalizedName, (const char*)(puppetPreloadStrings.stringFormat), boneName);
            char* character = normalizedName;
            char value;
            while ((value = *character) != 0) {
                if (value == ' ')
                    *character = '_';
                ++character;
            }
        } else {
            void* cached = AsWPuppet(self)->defaultBone;
            if (!cached) {
                // Preserve the three unrolled child levels before the recursive fallback.
                void* child;
                if (AsWPuppet(self)->childCount == 0)
                    child = 0;
                else {
                    child = AsWPuppet(self)->childHead;
                    child = ((WPuppetListNode*)child)->value;
                }
                if (child != 0) {
                    inheritedBone = AsWPuppet(child)->defaultBone;
                    if (inheritedBone)
                        break;
                    if (AsWPuppet(child)->childCount == 0)
                        child = 0;
                    else {
                        void* head = AsWPuppet(child)->childHead;
                        child = ((WPuppetListNode*)head)->value;
                    }
                    if (child != 0) {
                        inheritedBone = AsWPuppet(child)->defaultBone;
                        if (inheritedBone)
                            break;
                        if (AsWPuppet(child)->childCount == 0)
                            child = 0;
                        else {
                            void* head = AsWPuppet(child)->childHead;
                            child = ((WPuppetListNode*)head)->value;
                        }
                        if (child != 0) {
                            inheritedBone = AsWPuppet(child)->defaultBone;
                            if (inheritedBone)
                                break;
                            void* nextChild;
                            if (AsWPuppet(child)->childCount == 0)
                                nextChild = 0;
                            else {
                                void* head = AsWPuppet(child)->childHead;
                                nextChild = ((WPuppetListNode*)head)->value;
                            }
                            if (nextChild != 0)
                                return (void*)(long)WPuppet_ResolveDefaultBone(nextChild);
                            return 0;
                        } else
                            return 0;
                    } else
                        return 0;
                } else
                    return 0;
            }
            return cached;
        }
        void* sentinel = (char*)&AsWPuppet(self)->cachedBoneTail;
        void* volatile node;
        void* volatile initial;
        void* volatile allocationB;
        void* volatile allocationA;
        node = initial = AsWPuppet(self)->cachedBoneHead;
        while (node != sentinel) {
            if (strcmp(((PuppetBoneView*)((WPuppetListNode*)node)->value)->name, normalizedName) == 0) {
                return ((WPuppetListNode*)node)->value;
            }
            node = ((WPuppetListNode*)node)->next;
        }
        void* insertionSentinel;
        void* found = WPuppetBone_FindBone(AsWPuppet(self)->boneRoot, normalizedName, 0);
        if (found) {
            volatile bool flag = false;
            insertionSentinel = (char*)&AsWPuppet(self)->cachedBoneTail;
            void* alloc = appMallocDummy__FUl(sizeof(WPuppetListNode));
            allocationA = allocationB = alloc;
            Fn51320Guard guard;
            guard.owner = (char*)&AsWPuppet(self)->cachedBoneTail;
            guard.node = alloc;
            void* valueSlot = (char*)&((WPuppetListNode*)alloc)->value;
            if (valueSlot) {
                try {
                    *(void**)valueSlot = found;
                } catch (...) {
                    throw;
                }
            }
            guard.node = 0;
            ((WPuppetListNode*)((WPuppetListNode*)insertionSentinel)->prev)->next = alloc;
            ((WPuppetListNode*)alloc)->prev = ((WPuppetListNode*)insertionSentinel)->prev;
            ((WPuppetListNode*)insertionSentinel)->prev = alloc;
            ((WPuppetListNode*)alloc)->next = insertionSentinel;
            ++AsWPuppet(self)->cachedBoneCount;
        }
        return found;
    } while (0);
    return inheritedBone;
}
void* WPuppet_FindOrCacheBone(void* arg0, void* arg1);
#pragma opt_common_subs reset
int WPuppet_ApplyNamedMaterialLayerAnimation(void* arg0, void* arg1, void* arg2, unsigned int arg3, float arg4);
void* String_ParseQuotedToken(void* arg0, void* arg1);
int String_ParseDecimalInt(const char* arg0);
int WPuppet_ParseMaterialLayerAnimationCommand(void* arg0, void* arg1, void* arg2);

void* ResourceManager_LoadBitmap(void*, void*, int, int, int, int);
void __dla__FPv(void* arg0);
void __dl__FPv(void* arg0);
void WPuppetBone_ConcatMatrixAlongChain(void* arg0, void* arg1, void* arg2, void* arg3);
void WPuppet_UpdateBoundsFromResourceBox(void* arg0, int arg1, int arg2);
void __mi__FRC7WVectorRC7WVector(void* arg0, void* arg1, void* arg2);
float WVectorLen__FRC7WVector(void* arg0);

void WPuppet_GetInverseRootMotionOffset(void* arg0, void* arg1, float arg2);
void Matrix_ExpandAffineTo4x4(void* arg0, void* arg1);
extern const char kIdentityMatrix4x4[];
void Vec3_Zero(void* arg0);
int BoneControllerMap_EraseByIndex(void* arg0, void* arg1);
// Forward the skin-matrix buffer and its matrix count (encoded in a pointer-typed ABI word).
int WPuppet_OverrideSkinMatricesForwarder(void* self, void* skinSource, void* matrixCount) {
    return WPuppet_OverrideSkinMatrices(self, skinSource, matrixCount);
}
int BoneControllerTree_ContainsKey(void* arg0, void* arg1);
int LayerControllerMap_EraseAndDeleteByKey(void* arg0, void* arg1);
void PuppetChildList_EraseMatchingValue(void* arg0, void* arg1, void* arg2);
void Matrix_CopyAffine(void* arg0, void* arg1);
void appFreeUni__FPv(void* arg0);
extern "C" void* GetChunkElement__Q23gtl20IGenericResourceFileFii(void* arg0, const char* arg1, int arg2);
void Matrix_ExtractAffineFrom4x4(void* arg0, void* arg1);
void* memset(void*, int, unsigned int);

extern "C" int EvaluateTransforms__Q23gtl25IGenericAnimationResourceCFPvPvf(void*, void*, void*, float);

extern float kFloat_0_puppet;
extern void* g_puppetMatrixScratch;

struct WInstanceLock {
    void* lock;
    // Acquire the borrowed instance mutex for this scope.
    WInstanceLock(void* l) {
        lock = l;
        OSLockMutex((OSMutex*)lock);
    }
    // Release this scope's mutex acquisition.
    ~WInstanceLock() {
        OSUnlockMutex((OSMutex*)lock);
    }
};
typedef Vec3f fn_800516C4_vec;
typedef int (*fn_800516C4_fp)(void*, void*, void*, const void*, void*, void*, int);
#define fn_800516C4_apply ((fn_800516C4_fp)EvaluateTransforms__Q23gtl24IGenericModelingResourceCFPvPvPvPvUiPUi)
// Advance the mutex-protected instance cursor and return its current payload.
inline void* GetNextInstancePayload(void* self) {
    WPuppetInstanceList* list = &AsWPuppet(self)->animationInstances;
    void* lock = list->mutex;
    OSLockMutex((OSMutex*)lock);
    void* current = list->cursor;
    if (current) {
        void* next = ((PuppetInstanceNode*)current)->next;
        void* node = ((PuppetInstanceNode*)current)->value;
        if (next == list->first)
            next = 0;
        list->cursor = next;
        OSUnlockMutex((OSMutex*)lock);
        return node;
    }
    OSUnlockMutex((OSMutex*)lock);
    return 0;
}
// Find a named collision/bone record in the puppet's metadata table.
inline char* FindNamedBoneByName(void* self, const char* key) {
    unsigned int off = 0;
    unsigned int i = 0;
    unsigned int count = AsWPuppet(self)->namedBoneCount;
    for (; i < count; off += 8, ++i) {
        char* record = *(char**)&AsWPuppet(self)->namedBones + off;
        if (record && ((PuppetNamedBoneView*)record)->bounds &&
            strcmp(key, ((PuppetBoundsView*)((PuppetNamedBoneView*)record)->bounds)->name) == 0)
            return record;
    }
    return 0;
}
// Return the first child payload or null when the child list is empty.
inline void* GetFirstChildPayload(void* self) {
    return AsWPuppet(self)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
}
// Resolve the local model or the first child's model in container load mode.
inline void* GetModelResourceOrChildResource(void* self) {
    void* r = AsWPuppet(self)->modelResource;
    if (!r && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* child = GetFirstChildPayload(self);
        if (child)
            r = AsWPuppet(child)->modelResource;
    }
    return r;
}
// Root-motion sampling (30-Hz animation key frames, NOT seconds).
// out is three writable floats; self is required. Output is center translation at
// zero minus translation at frame, with identity as the modeling root: resource
// model coordinates, before the puppet/world transform (npc_pola_penguin.cpp transforms the
// resulting delta as a direction). No distance unit scaling happens here.
// Missing animation/model/bone buffer/global scratch, nonpositive frame, or an
// inclusive suppression interval returns zero. On the positive sampling path,
// metadata MUST contain a center bone, its model layer/transform index must be
// valid, and the selected animation must be evaluable. These are unchecked asset
// prerequisites, not recoverable failures; evaluator return values are ignored.
// Side effects: consumes the shared instance-list cursor; overwrites boneMatrices
// and g_puppetMatrixScratch at frame, then at ZERO. Does not restore the old pose.
// Only cursor operations are mutex-protected. Callers must serialize sampling,
// rendering and other users of these buffers; the locks do not protect the pose.
// Re-evaluate the desired pose before using boneMatrices after this query.
void WPuppet_GetInverseRootMotionOffset(void* out, void* self, float frame) {
    if (!AsWPuppet(self)->animation) {
        *(float*)out = 0.0f;
        *(float*)((char*)out + 4) = 0.0f;
        *(float*)((char*)out + 8) = 0.0f;
        return;
    }
    void* suppressedRange;
    {
        WInstanceLock outer((char*)AsWPuppet(self)->animationInstances.mutex);
        AsWPuppet(self)->animationInstances.cursor = AsWPuppet(self)->animationInstances.first;
        suppressedRange = GetNextInstancePayload(self);
    }
    while (suppressedRange) {
        if (((PuppetMotionSuppressionRange*)suppressedRange)->startFrame <= frame &&
            frame <= *(float*)&((PuppetMotionSuppressionRange*)suppressedRange)->endFrame) {
            *(float*)out = 0.0f;
            *(float*)((char*)out + 4) = 0.0f;
            *(float*)((char*)out + 8) = 0.0f;
            return;
        }
        suppressedRange = GetNextInstancePayload(self);
    }
    void* resource = AsWPuppet(self)->modelResource;
    if (!resource) {
        *(float*)out = 0.0f;
        *(float*)((char*)out + 4) = 0.0f;
        *(float*)((char*)out + 8) = 0.0f;
        return;
    }
    if (frame > 0.0f) {
        char* record =
            FindNamedBoneByName(self, (char*)&puppetPreloadStrings + PUPPET_OFFSET(PuppetPreloadStrings, centerBone));
        void* modelLayer = GetChunkElement__Q23gtl20IGenericResourceFileFii(
            resource, (const char*)kChunkModelLayers, ((PuppetBoneView*)((PuppetNamedBoneView*)record)->bone)->index);
        void* animationResource = ((CPetAnimation*)(AsWPuppet(self)->animation))->GetSelectedResource();
        void* boneMatrices = AsWPuppet(self)->boneMatrices;
        if (!boneMatrices) {
            *(float*)out = 0.0f;
            *(float*)((char*)out + 4) = 0.0f;
            *(float*)((char*)out + 8) = 0.0f;
            return;
        }
        void* skinScratch = g_puppetMatrixScratch;
        if (!skinScratch) {
            *(float*)out = 0.0f;
            *(float*)((char*)out + 4) = 0.0f;
            *(float*)((char*)out + 8) = 0.0f;
            return;
        }
        float* centerTransform =
            (float*)((char*)skinScratch +
                     (*(unsigned short*)&((PuppetModelLayerView*)modelLayer)->transformIndex << 6));
        unsigned int boneTransformCount =
            GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource, kChunkBoneTransforms);
        void* sk = GetModelResourceOrChildResource(self);
        unsigned int effectiveBoneCount =
            sk ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(sk, kChunkBoneTransforms) : 0;
        EvaluateTransforms__Q23gtl25IGenericAnimationResourceCFPvPvf(animationResource, boneMatrices,
                                                                     (void*)effectiveBoneCount, frame);
        void* s1 = GetModelResourceOrChildResource(self);
        fn_800516C4_apply(AsWPuppet(self)->modelResource, skinScratch,
                          s1 ? (void*)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(s1, kChunkSkinTransforms)
                             : 0,
                          kIdentityMatrix4x4, boneMatrices, (void*)boneTransformCount, 0);
        fn_800516C4_vec currentTranslation;
        float ax = centerTransform[12], ay = centerTransform[13], az = centerTransform[14];
        currentTranslation.x = ax;
        currentTranslation.y = ay;
        currentTranslation.z = az;
        EvaluateTransforms__Q23gtl25IGenericAnimationResourceCFPvPvf(animationResource, boneMatrices,
                                                                     (void*)effectiveBoneCount, 0.0f);
        void* s2 = GetModelResourceOrChildResource(self);
        fn_800516C4_apply(AsWPuppet(self)->modelResource, skinScratch,
                          s2 ? (void*)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(s2, kChunkSkinTransforms)
                             : 0,
                          kIdentityMatrix4x4, boneMatrices, (void*)boneTransformCount, 0);
        fn_800516C4_vec zeroTranslation;
        float bx = centerTransform[12], by = centerTransform[13], bz = centerTransform[14];
        zeroTranslation.x = bx;
        zeroTranslation.y = by;
        zeroTranslation.z = bz;
        __mi__FRC7WVectorRC7WVector(out, &zeroTranslation, &currentTranslation);
        return;
    }
    *(float*)out = 0.0f;
    *(float*)((char*)out + 4) = 0.0f;
    *(float*)((char*)out + 8) = 0.0f;
}

extern void* __nw__FUl(unsigned int);
extern void* IPetMesh_Ctor(void*, void*, void*);
extern void IPetMesh_LoadGroups(void*, int);

struct Object53D38 {
    char data[0x18];
    // Forward the original two construction arguments to the pet-mesh initializer.
    Object53D38(void* a, void* b) {
        IPetMesh_Ctor(this, a, b);
    }
};

// Resolve the default bone through the first-child chain, preserving the original unrolled lookup.
inline void* lookup53D38(void* node) {
    void* p;
    void* outer = AsWPuppet(node)->defaultBone;
    if (!outer) {
        void* c4 = AsWPuppet(node)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(node)->childHead)->value;
        if (c4) {
            p = AsWPuppet(c4)->defaultBone;
            if (!p) {
                void* c3 = AsWPuppet(c4)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(c4)->childHead)->value;
                if (c3) {
                    p = AsWPuppet(c3)->defaultBone;
                    if (!p) {
                        void* c2 =
                            AsWPuppet(c3)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(c3)->childHead)->value;
                        if (c2) {
                            p = AsWPuppet(c2)->defaultBone;
                            if (!p) {
                                void* c1 = AsWPuppet(c2)->childCount == 0
                                               ? 0
                                               : ((WPuppetListNode*)AsWPuppet(c2)->childHead)->value;
                                if (c1)
                                    p = (void*)WPuppet_ResolveDefaultBone(c1);
                                else
                                    p = 0;
                            }
                        } else
                            p = 0;
                    }
                } else
                    p = 0;
            }
        } else
            p = 0;
    } else
        p = outer;
    return p;
}
// Validate a bone's model-layer index against the selected resource chunk count.
inline int checked53D38(void* self, int record, int kind) {
    unsigned int n;
    if (!record)
        record = -1;
    else {
        n = *(unsigned int*)&((PuppetBoneView*)record)->index;
        if (n >=
            (unsigned int)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource, kind))
            record = -1;
        else
            record = *(int*)&((PuppetBoneView*)record)->index;
    }
    return record;
}
// Resolve a named or default bone and return its checked model-layer index.
inline int recordIndex53D38(void* self, const char* name, int kind) {
    int record = (int)lookup53D38(self);
    if (name)
        record = (int)WPuppetBone_FindBone(AsWPuppet(self)->boneRoot, name, 0);
    return checked53D38(self, record, kind);
}
// Reject model layers with no mesh groups and return a usable layer index or -1.
inline unsigned int index53D38(void* self, const char* name, int kind = kChunkModelLayers) {
    unsigned int result;
    int index = recordIndex53D38(self, name, kind);
    unsigned short* tag = (unsigned short*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
        AsWPuppet(self)->modelResource, (const char*)kind, index);
    result = -1;
    if (*tag)
        result = index;
    return result;
}
#pragma use_lmw_stmw on
// Search child models for the first usable layer matching the requested bone name.
inline void search53D38(int kind, const char* name, void* sentinel, unsigned int& result, void*& selected,
                        unsigned int node) {
    while (node != (unsigned int)sentinel) {
        result = index53D38(*(void**)((char*)node + 8), name, kind);
        if (result != -1) {
            selected = *(void**)((char*)node + 8);
            break;
        }
        node = *(unsigned int*)((char*)node + 4);
    }
}
// Find a model layer in this puppet or its children and construct a mesh wrapper for that layer.
int WPuppet_CreateMeshByBoneName(void* self, void** out, const char* name) {
    void* object;
    void* sentinel;
    void* selected;
    *out = 0;
    unsigned int result = index53D38(self, name);
    selected = 0;
    if (result == -1) {
        sentinel = (char*)&AsWPuppet(self)->childTail;
        search53D38(kChunkModelLayers, name, sentinel, result, selected, *(unsigned int*)&AsWPuppet(self)->childHead);
    } else
        selected = self;
    if (!selected)
        return 0;
    object = new Object53D38(AsWPuppet(selected)->modelResource, AsWPuppet(self)->nameIndexMap);
    *out = object;
    IPetMesh_LoadGroups(object, result);
    return 1;
}
#pragma use_lmw_stmw reset

struct TexManagerResource {
    char unk[0x14];
    // Initialize the texture-manager resource wrapper.
    TexManagerResource() {
        ((CPetTexManager*)this)->__ct();
    }
};

#pragma use_lmw_stmw on
// Load each distinct texture referenced by model materials and register its model-resource handles.
int WPuppet_LoadMaterialTextures(void* self, void* chunkData, void* chunkSize, void* loadMode, void* resourceDirectory,
                                 unsigned int* outCount) {
    if (!AsWPuppet(self)->modelResource)
        return 0;
    void* old = AsWPuppet(self)->nameIndexMap;
    if (old) {
        void** vt = *(void***)((char*)old + 4);
        ((void (*)(void*, int))vt[2])(old, 1);
        AsWPuppet(self)->nameIndexMap = 0;
    }
    if (!GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource, kChunkTextureNames))
        return 0;
    TexManagerResource* state = new TexManagerResource;
    AsWPuppet(self)->nameIndexMap = state;
    unsigned int layerCount = (unsigned int)(long)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(
        AsWPuppet(self)->modelResource, kChunkModelLayers);
    void* strings = GetChunkElement__Q23gtl20IGenericResourceFileFii(AsWPuppet(self)->modelResource,
                                                                     (const char*)kChunkStringData, 0);
    unsigned char seen[kTextureIdCount];
    unsigned int used = 0;
    memset(seen, kNoTextureId, sizeof(seen));
    unsigned char* seenBegin = seen;
    for (unsigned int layer = 0; layer < layerCount; ++layer) {
        void* modelLayer = GetChunkElement__Q23gtl20IGenericResourceFileFii(AsWPuppet(self)->modelResource,
                                                                            (const char*)kChunkModelLayers, layer);
        for (int groupIndex = 0; groupIndex < ((PuppetModelLayerView*)modelLayer)->groupCount; ++groupIndex) {
            void* meshGroup = GetChunkElement__Q23gtl20IGenericResourceFileFii(
                AsWPuppet(self)->modelResource, (const char*)kChunkMeshGroups,
                ((PuppetModelLayerView*)modelLayer)->firstGroup + groupIndex);
            for (unsigned int primitiveIndex = 0; primitiveIndex < ((PuppetMeshGroupView*)meshGroup)->primitiveCount;
                 ++primitiveIndex) {
                void* primitive = GetChunkElement__Q23gtl20IGenericResourceFileFii(
                    AsWPuppet(self)->modelResource, (const char*)kChunkPrimitives,
                    ((PuppetMeshGroupView*)meshGroup)->firstPrimitive + primitiveIndex);
                unsigned char* seenEnd = seenBegin + used;
                for (int textureSlot = 0; textureSlot < kMaterialTextureSlots; ++textureSlot) {
                    unsigned char textureId = ((PuppetMaterialTextures*)AsWPuppet(self)
                                                   ->materialMetadata)[((PuppetPrimitiveView*)primitive)->materialIndex]
                                                  .textureIds[textureSlot];
                    if (textureId == kNoTextureId)
                        continue;
                    int alreadySeen = 0;
                    for (unsigned int i = 0; i < used; ++i) {
                        if (seenBegin[i] == textureId) {
                            alreadySeen = 1;
                            break;
                        }
                    }
                    if (!alreadySeen) {
                        void* key = GetChunkElement__Q23gtl20IGenericResourceFileFii(
                            AsWPuppet(self)->modelResource, (const char*)kChunkTextureNames, textureId);
                        void* texture = ((void* (*)(void*, void*, int, int, int))ResourceManager_LoadBitmap)(
                            g_resourceManager, (char*)strings + *(int*)((char*)key + 4), 0, 0, 0);
                        if (texture) {
                            void* manager = *(void**)g_resourceManager;
                            void** vt = *(void***)manager;
                            int imageHandle = ((int (*)(void*, void*))vt[0x70 / 4])(manager, texture);
                            vt = *(void***)manager;
                            int paletteHandle = ((int (*)(void*, void*))vt[0x74 / 4])(manager, texture);
                            ((int (*)(void*, int, int,
                                      int))SetResource__Q23gtl24IGenericModelingResourceFiiP12GtlRefHeader)(
                                AsWPuppet(self)->modelResource, kTextureResourceKind, textureId, imageHandle);
                            ((int (*)(void*, int, int,
                                      int))SetResource__Q23gtl24IGenericModelingResourceFiiP12GtlRefHeader)(
                                AsWPuppet(self)->modelResource, kTexturePaletteResourceKind, textureId, paletteHandle);
                            ((CPetTexManager*)AsWPuppet(self)->nameIndexMap)
                                ->AppendTextureEntry((int)texture, textureId,
                                                     ((PuppetPrimitiveView*)primitive)->materialIndex);
                        }
                        *seenEnd++ = textureId;
                        used++;
                    }
                }
            }
        }
    }
    *outCount = used;
    return 0;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Unlink and free owned instance-list nodes under the mutex, release storage, and optionally delete the list.
void* WPuppetOwnedInstances_dtor(void* self, int deleting) {
    if (self) {
        {
            WInstanceLock guard((void*)((int)self + 0x20));
            if (((WPuppetInstanceList*)self)->ownedHead) {
                void* head;
                void* lock = (char*)((WPuppetInstanceList*)self)->mutex;
                void* node;
                while ((node = ((WPuppetInstanceList*)self)->ownedHead) != 0) {
                    OSLockMutex((OSMutex*)lock);
                    void* next = *(void**)&((PuppetInstanceNode*)node)->next;
                    void* prev = *(void**)&((PuppetInstanceNode*)node)->prev;
                    *(void**)&((PuppetInstanceNode*)next)->prev = prev;
                    next = *(void**)&((PuppetInstanceNode*)node)->next;
                    prev = *(void**)&((PuppetInstanceNode*)node)->prev;
                    *(void**)&((PuppetInstanceNode*)prev)->next = next;
                    head = *(void**)&((PuppetInstanceNode*)node)->next;
                    if (head == node)
                        head = 0;
                    OSUnlockMutex((OSMutex*)lock);
                    ((WPuppetInstanceList*)self)->ownedHead = head;
                    appFreeUni__FPv(node);
                    node = ((WPuppetInstanceList*)self)->ownedHead;
                }
            }
            void* item = ((WPuppetInstanceList*)self)->storage;
            if (item) {
                appFreeUni__FPv(item);
                ((WPuppetInstanceList*)self)->storage = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Unlink and free owned animation-instance nodes under the mutex, release storage, and optionally delete the list.
void* WPuppetAnimationInstances_dtor(void* self, int deleting) {
    if (self) {
        {
            WInstanceLock guard((void*)((int)self + 0x20));
            if (((WPuppetInstanceList*)self)->ownedHead) {
                void* head;
                void* lock = (char*)((WPuppetInstanceList*)self)->mutex;
                void* node;
                while ((node = ((WPuppetInstanceList*)self)->ownedHead) != 0) {
                    OSLockMutex((OSMutex*)lock);
                    void* next = *(void**)&((PuppetInstanceNode*)node)->next;
                    void* prev = *(void**)&((PuppetInstanceNode*)node)->prev;
                    *(void**)&((PuppetInstanceNode*)next)->prev = prev;
                    next = *(void**)&((PuppetInstanceNode*)node)->next;
                    prev = *(void**)&((PuppetInstanceNode*)node)->prev;
                    *(void**)&((PuppetInstanceNode*)prev)->next = next;
                    head = *(void**)&((PuppetInstanceNode*)node)->next;
                    if (head == node)
                        head = 0;
                    OSUnlockMutex((OSMutex*)lock);
                    ((WPuppetInstanceList*)self)->ownedHead = head;
                    appFreeUni__FPv(node);
                    node = ((WPuppetInstanceList*)self)->ownedHead;
                }
            }
            void* item = ((WPuppetInstanceList*)self)->storage;
            if (item) {
                appFreeUni__FPv(item);
                ((WPuppetInstanceList*)self)->storage = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Use each model layer's transform index to copy evaluated skin matrices into its bone.
void WPuppet_CopySkinMatricesToBones(void* self, void* skinMatrices) {
    if (!skinMatrices)
        return;
    void* data = AsWPuppet(self)->modelResource;
    if (!data && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* node = *(void**)&AsWPuppet(self)->childCount;
        // 0x8004FC28 unit: guard the existing first-child lookup scope.
        if (node) {
            void* child = ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
            if (child)
                data = AsWPuppet(child)->modelResource;
        }
    skip_child2:;
    }
    if (!data)
        return;
    void* entries = GetChunkElement__Q23gtl20IGenericResourceFileFii(data, (const char*)kChunkModelLayers, 0);
    void* element = AsWPuppet(self)->bones;
    int i = 0;
    while (i < AsWPuppet(self)->boneCount) {
        unsigned int index = *(unsigned int*)&((PuppetBoneView*)element)->index;
        unsigned short entry = *(unsigned short*)((char*)entries + index * 0x30 + 0x18);
        char matrix[0x30];
        Matrix_ExtractAffineFrom4x4((char*)skinMatrices + (entry << 6), matrix);
        Matrix_CopyAffine((char*)&((PuppetBoneView*)element)->skinMatrix, matrix);
        element = (char*)element + kBoneBytes;
        ++i;
    }
}
#pragma use_lmw_stmw reset
// Concatenate bone matrices along the hierarchy using the puppet world matrix.
void WPuppet_UpdateBoneWorldMatrices(void* self, void* transformInput, void* transformOutput) {
    void* p = AsWPuppet(self)->boneRoot;
    if (!p)
        return;
    WPuppetBone_ConcatMatrixAlongChain(p, (char*)&AsWPuppet(self)->worldMatrix, transformInput, transformOutput);
}

// Return root-motion offset only when the requested named animation range exists.
void WPuppet_GetInverseRootMotionOffsetForRange(void* result, void* self, float frame, void* rangeName) {
    void* array = AsWPuppet(self)->animationRanges;
    void** end = 0;
    void* found = 0;
    void** item = 0;
    if (array) {
        item = *(void***)array;
        end = item + *(unsigned int*)((char*)array + 4);
        while (item != end) {
            found = *item;
            if (found) {
                if (strcmp(*(const char**)&((PuppetMotionRangeView*)found)->name, (const char*)rangeName) == 0)
                    break;
            }
            ++item;
        }
        if (item == end)
            found = 0;
    }
    if (!found) {
        float z = kFloat_0_puppet;
        *(float*)result = z;
        *((float*)result + 1) = z;
        *((float*)result + 2) = z;
    } else {
        WPuppet_GetInverseRootMotionOffset(result, self, frame);
    }
}

// Queue the local model with its world matrix, materials, visibility words, and optional inherited matrices.
void WPuppet_QueueModelDraw(void* arg0, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6,
                            void* arg7, void* arg8, void* arg9) {
    if (AsWPuppet(arg0)->modelResource) {
        char matrix[0x40];
        char pad[0x10];
        Matrix_ExpandAffineTo4x4((char*)&AsWPuppet(arg0)->worldMatrix, matrix);
        void* params[8] = {};
        arg6 = arg6 ? arg6 : AsWPuppet(arg0)->skinMatrices;
        params[0] = arg6;
        params[2] = AsWPuppet(arg0)->materials;
        params[4] = arg7;
        params[5] = AsWPuppet(arg0)->modelLayerVisibilityWords;
        pad[0] = 0;
        ((void (*)(void*, void*, void*, int))QueueDraw__Q23gtl24IGenericModelingResourceFPvPv)(
            AsWPuppet(arg0)->modelResource, arg2 ? matrix : (void*)kIdentityMatrix4x4, params, 0);
    }
}

// Replace the bone override controller with one sized for the effective model's transform count.
int WPuppet_RebuildBoneController(void* self) {
    if (*(int*)&AsWPuppet(self)->boneController) {
        if (*(int*)&AsWPuppet(self)->boneController) {
            void* obj = (void*)*(int*)&AsWPuppet(self)->boneController;
            void* vt = *(void**)obj;
            void* fn = *(void**)((char*)vt + 8);
            ((void (*)(void*, int))fn)(obj, 1);
        }
        AsWPuppet(self)->boneController = 0;
    }
    void* p = AsWPuppet(self)->modelResource;
    if (!p && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* q;
        if (AsWPuppet(self)->childCount == 0)
            q = 0;
        else {
            q = AsWPuppet(self)->childHead;
            q = *(void**)((char*)q + 8);
        }
        if (q)
            p = AsWPuppet(q)->modelResource;
    }
    if (p) {
        p = (void*)(long)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, kChunkBoneTransforms);
    } else {
        p = 0;
    }
    if (!p)
        return 0;
    int id = (int)CRotateRegistry::Create(p);
    *(int*)&AsWPuppet(self)->boneController = id;
    unsigned int a = (unsigned int)-id;
    unsigned int b = (unsigned int)id;
    return (int)((a | b) >> 31);
}

// Copy the supplied world matrix to this puppet and every descendant.
void WPuppet_SetWorldMatrixRecursive(void* self, void* worldMatrix) {
    Matrix_CopyAffine((char*)&AsWPuppet(self)->worldMatrix, worldMatrix);
    void* n0 = AsWPuppet(self)->childHead;
    void* base0 = (char*)&AsWPuppet(self)->childTail;
    while (n0 != base0) {
        void* o0 = ((WPuppetListNode*)n0)->value;
        Matrix_CopyAffine((char*)&AsWPuppet(o0)->worldMatrix, worldMatrix);
        void* n1 = AsWPuppet(o0)->childHead;
        void* base1 = (char*)&AsWPuppet(o0)->childTail;
        while (n1 != base1) {
            void* o1 = ((WPuppetListNode*)n1)->value;
            Matrix_CopyAffine((char*)&AsWPuppet(o1)->worldMatrix, worldMatrix);
            void* n2 = AsWPuppet(o1)->childHead;
            void* base2 = (char*)&AsWPuppet(o1)->childTail;
            while (base2 != n2) {
                void* o2 = ((WPuppetListNode*)n2)->value;
                Matrix_CopyAffine((char*)&AsWPuppet(o2)->worldMatrix, worldMatrix);
                void* n3 = AsWPuppet(o2)->childHead;
                void* base3 = (char*)&AsWPuppet(o2)->childTail;
                while (base3 != n3) {
                    WPuppet_SetWorldMatrixRecursive(((WPuppetListNode*)n3)->value, worldMatrix);
                    n3 = ((WPuppetListNode*)n3)->next;
                }
                n2 = ((WPuppetListNode*)n2)->next;
            }
            n1 = ((WPuppetListNode*)n1)->next;
        }
        n0 = ((WPuppetListNode*)n0)->next;
    }
}

int SkinTransformOverrides_Apply(void* arg0, void* arg1, void* arg2, void* arg3);
// Resolve the effective model for animation evaluation in container mode.
inline void* GetEffectiveModelResource(void* arg0) {
    void* parent = AsWPuppet(arg0)->modelResource;
    if (!parent && AsWPuppet(arg0)->loadFlags == kContainerLoadMode) {
        void* child = !*(void**)&AsWPuppet(arg0)->childCount ? 0 : *(void**)(*(char**)&AsWPuppet(arg0)->childHead + 8);
        if (child)
            parent = AsWPuppet(child)->modelResource;
    }
    return parent;
}
#pragma use_lmw_stmw on
// Evaluate a borrowed source animation into this puppet's existing buffers:
// animation channels -> 0x30-byte bone poses -> overrides -> 0x40-byte skin
// matrices (identity root) -> bone records. GetAmount returns absolute key-frame
// time, not a time delta (pet_animation.cpp); no playback clock is advanced here.
// Disabled/invalid source, missing selected resource, or either missing output
// buffer leaves this puppet untouched. Without an effective model, evaluation
// is skipped but existing skin matrices are still copied to bones. Evaluation
// statuses are ignored. Buffers must cover the effective resource's counts and
// indexed tracks; serialize with drawing/root-motion sampling/override mutation.
void WPuppet_UpdateSkinFromAnimationSource(void* self, void* animationSource) {
    if (AsWPuppet(self)->animationDisabled)
        return;
    if (!animationSource || !(*(unsigned int*)&AsWPuppet(animationSource)->loadFlags & kAnimationSourceFlags) ||
        !AsWPuppet(animationSource)->animation)
        return;
    void* group;
    void* resolved = ((CPetAnimation*)(AsWPuppet(animationSource)->animation))->GetSelectedResource();
    if (!resolved)
        return;

    void* parent = GetEffectiveModelResource(self);
    void* array = AsWPuppet(self)->boneMatrices;
    if (!array)
        return;
    void* object = AsWPuppet(self)->skinMatrices;
    if (!object)
        return;

    if (parent) {
        void* p = GetEffectiveModelResource(self);
        unsigned int boneTransformCount =
            p ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, kChunkBoneTransforms) : 0;
        float animationFrame = ((CPetAnimation*)(AsWPuppet(animationSource)->animation))->GetAmount();
        EvaluateTransforms__Q23gtl25IGenericAnimationResourceCFPvPvf(resolved, array, (void*)boneTransformCount,
                                                                     animationFrame);

        group = AsWPuppet(self)->boneController;
        if (group && *(void**)((char*)group + 8)) {
            p = GetEffectiveModelResource(self);
            SkinTransformOverrides_Apply(
                group, array, array,
                p ? (void*)(long)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, kChunkBoneTransforms) : 0);
        }

        p = GetEffectiveModelResource(self);
        unsigned int skinTransformCount =
            p ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, kChunkSkinTransforms) : 0;
        ((int (*)(void*, void*, void*, void*, void*, void*,
                  int))EvaluateTransforms__Q23gtl24IGenericModelingResourceCFPvPvPvPvUiPUi)(
            parent, object, (void*)skinTransformCount, (void*)kIdentityMatrix4x4, array,
            (void*)(long)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(parent, kChunkBoneTransforms), 0);
    }
    WPuppet_CopySkinMatricesToBones(self, object);
}
#pragma use_lmw_stmw on
// Apply a world matrix throughout the hierarchy and optionally evaluate animation from another puppet.
void Model_SetMatrixRecursive(void* self, void* animationSource, void* worldMatrix) {
    Matrix_CopyAffine((char*)&AsWPuppet(self)->worldMatrix, worldMatrix);
    void* n0 = AsWPuppet(self)->childHead;
    void* end0 = (char*)&AsWPuppet(self)->childTail;
    for (; n0 != end0; n0 = ((WPuppetListNode*)n0)->next) {
        void* o0 = ((WPuppetListNode*)n0)->value;
        Matrix_CopyAffine((char*)&AsWPuppet(o0)->worldMatrix, worldMatrix);
        void* end1 = (char*)&AsWPuppet(o0)->childTail;
        void* n1 = AsWPuppet(o0)->childHead;
        for (; end1 != n1; n1 = ((WPuppetListNode*)n1)->next) {
            void* o1 = ((WPuppetListNode*)n1)->value;
            Matrix_CopyAffine((char*)&AsWPuppet(o1)->worldMatrix, worldMatrix);
            void* n2 = AsWPuppet(o1)->childHead;
            void* end2 = (char*)&AsWPuppet(o1)->childTail;
            for (; end2 != n2; n2 = ((WPuppetListNode*)n2)->next) {
                void* o2 = ((WPuppetListNode*)n2)->value;
                Matrix_CopyAffine((char*)&AsWPuppet(o2)->worldMatrix, worldMatrix);
                void* end3 = (char*)&AsWPuppet(o2)->childTail;
                void* n3 = AsWPuppet(o2)->childHead;
                for (; n3 != end3; n3 = ((WPuppetListNode*)n3)->next)
                    WPuppet_SetWorldMatrixRecursive(((WPuppetListNode*)n3)->value, worldMatrix);
            }
        }
    }
    if (animationSource) {
        if (*(unsigned int*)&AsWPuppet(animationSource)->loadFlags & kAnimationSourceFlags)
            WPuppet_UpdateSkinFromAnimationSource(self, animationSource);
    }
}

// Borrow the model used by skin allocation; container mode consults only its
// first child, never searches later children. No retain or resource mutation.
// Expansion preserves query-local lifetimes and CodeWarrior's local-class
// destructor symbol IDs; an inline function or do/while scope changes those IDs.
// Use only as standalone statements in this function (never as an if/else body).
// Arguments here are simple locals / the stable self pointer.
#define RESOLVE_SKIN_ALLOCATION_MODEL(model, self)                         \
    model = AsWPuppet(self)->modelResource;                                \
    if (!model && AsWPuppet(self)->loadFlags == kContainerLoadMode) {      \
        void* child;                                                       \
        if (AsWPuppet(self)->childCount == 0)                              \
            child = 0;                                                     \
        else                                                               \
            child = ((WPuppetListNode*)AsWPuppet(self)->childHead)->value; \
        if (child)                                                         \
            model = AsWPuppet(child)->modelResource;                       \
    }
#pragma opt_propagation off
// Destructively replace owned skinMatrices; __dla is its release operation.
// Resolution at EVERY query: own model first, else first child's model only in
// kContainerLoadMode. The hierarchy/resources must remain stable across queries.
// Return/state map after deleting the old skin array:
//   zero skin count (including no model): 1, skinMatrices remains null;
//   missing effective model / zero bone count / failed temporary allocation:
//       0, skinMatrices remains null;
//   otherwise: publish new skin allocation, evaluate, return evaluator != 0.
// A failed skin allocation is passed to the evaluator; failure has no rollback
// and may leave a published/partially written array. Allocation is assumed valid
// by this trusted-resource path. Existing boneMatrices are borrowed and reused;
// otherwise a temporary boneCount*0x30 pose array is initialized from defaults,
// then freed after evaluation. skinCount*0x40 bytes hold the resulting matrices.
// Counts remain unsigned integers until legacy ABI casts at evaluator calls.
// The modeling adapter in gtl_generic_modeling_resource_part1.cpp is declared void and does not establish a
// success result; this int-call view consumes r3 anyway. Preserve that
// unresolved result convention rather than promise reliable allocation status.
int WPuppet_AllocateSkinMatrices(void* self) {
    if (AsWPuppet(self)->skinMatrices) {
        __dla__FPv(AsWPuppet(self)->skinMatrices);
        AsWPuppet(self)->skinMatrices = 0;
    }
    void* parent;
    RESOLVE_SKIN_ALLOCATION_MODEL(parent, self);
    unsigned int skinTransformCount;
    if (parent)
        skinTransformCount = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(parent, kChunkSkinTransforms);
    else
        skinTransformCount = 0;
    if (!skinTransformCount)
        return 1;

    void* model;
    RESOLVE_SKIN_ALLOCATION_MODEL(model, self);
    if (!model)
        return 0;

    void* found;
    RESOLVE_SKIN_ALLOCATION_MODEL(found, self);
    unsigned int boneTransformCount;
    if (found)
        boneTransformCount = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(found, kChunkBoneTransforms);
    else
        boneTransformCount = 0;
    if (!boneTransformCount)
        return 0;

    void* extra = AsWPuppet(self)->boneMatrices;
    void* allocated = 0;
    if (!extra) {
        void* fresh = __nwa__FUl(boneTransformCount * 0x30);
        allocated = fresh;
        extra = allocated;
        if (!fresh)
            return 0;

        void* second;
        RESOLVE_SKIN_ALLOCATION_MODEL(second, self);
        unsigned int defaultPoseCount;
        if (second)
            defaultPoseCount = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(second, kChunkBoneTransforms);
        else
            defaultPoseCount = 0;
        ((gtl::IGenericModelingResource*)model)->EvaluateTransformAnimation(extra, (void*)defaultPoseCount);
    }

    void* data = __nwa__FUl(skinTransformCount << 6);
    int ok = 0;
    AsWPuppet(self)->skinMatrices = data;
    void* skinModel;
    RESOLVE_SKIN_ALLOCATION_MODEL(skinModel, self);
    unsigned int evaluationSkinCount;
    if (skinModel)
        evaluationSkinCount = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(skinModel, kChunkSkinTransforms);
    else
        evaluationSkinCount = 0;
    int result = ((int (*)(void*, void*, void*, const char*, void*, void*,
                           int))EvaluateTransforms__Q23gtl24IGenericModelingResourceCFPvPvPvPvUiPUi)(
        model, AsWPuppet(self)->skinMatrices, (void*)evaluationSkinCount, kIdentityMatrix4x4, extra,
        (void*)boneTransformCount, 0);
    if (result)
        ok = 1;
    if (allocated)
        __dla__FPv(allocated);
    return ok;
}
#undef RESOLVE_SKIN_ALLOCATION_MODEL
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __vt__Q23std11logic_error;
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void* memcpy(void*, const void*, unsigned int);

extern DATA_SECTION(".data") void* __vt__Q23std12out_of_range;
extern void* __vt__Q23std12length_error;
extern void* s_sharedCharDeleterVtable;
extern void* __dt__Q23std12length_errorFv(void*, int);
struct F58Str {
    unsigned int hdr;
    unsigned int size;
    char* data;
    // Start an empty inline string for named-resource path processing.
    F58Str() : hdr(0), size(0), data(0) {}
    // Free string storage only when the high tag bit selects heap ownership.
    ~F58Str() {
        if (hdr >> 31)
            appFreeUni__FPv(data);
    }
};
struct F58Guard {
    char* text;
    char* allocator;
    // Free an exception message array until shared ownership is established.
    ~F58Guard() {
        if (text)
            __dla__FPv(text);
    }
};
struct F58Exc {
    void* vt;
    char* text;
    unsigned int* rep;
};
struct F58Vec {
    void** data;
    unsigned int size;
    unsigned int capacity;
    void* alloc;
    unsigned int extra;
    // Start a temporary vector owner with a borrowed allocator/capacity reference.
    F58Vec(void* a) : data(0), size(0), capacity(0), alloc(a), extra(0) {}
    // Free pointer storage only; ownership of moved requests stays in the destination.
    ~F58Vec() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Return the lesser value by reference for the preloader's string/vector operations.
template <class T> static inline const T& f58min(const T& a, const T& b) {
    const T* p = &b;
    if (a < b)
        p = &a;
    return *p;
}
// Select the larger capacity increment while retaining the original reference-based inline shape.
static inline const unsigned int& f58max(const unsigned int& a, const unsigned int& b) {
    const unsigned int* p = &a;
    if (a < b)
        p = &b;
    return *p;
}
// Construct and throw the vector length exception using the runtime's shared-message representation.
static inline void f58_length_error() {
    F58Exc e;
    F58Exc* mp = &e;
    const char* message = (const char*)puppetPreloadStrings.vectorLengthError;
    mp->vt = &__vt__Q23std11logic_error;
    char alloc __attribute__((aligned(4))) = 0;
    unsigned int len = strlen(message);
    char* text = (char*)__nwa__FUl((len + 1) * 2 - (len + 1));
    mp->text = text;
    {
        F58Guard guard = {text, &alloc};
        unsigned int* rep = (unsigned int*)__nw__FUl(0x10);
        if (rep) {
            rep[1] = 1;
            rep[2] = 1;
            rep[0] = (unsigned int)&s_sharedCharDeleterVtable;
            rep[3] = (unsigned int)text;
        }
        mp->rep = rep;
        guard.text = 0;
    }
    strcpy(mp->text, message);
    mp->vt = &__vt__Q23std12length_error;
    __throw((const char*)(puppetPreloadStrings.lengthErrorType), mp, (void*)__dt__Q23std12length_errorFv);
}
// Compare a checked suffix position with the requested extension.
static inline int f58_compare(F58Str& s, unsigned int pos, const char* str) {
    unsigned int n1;
    unsigned int slen;
    slen = strlen(str);
    n1 = 4;
    const char* begin;
    unsigned int len;
    if (!(s.hdr >> 31)) {
        begin = (const char*)&s + 1;
        len = *(unsigned char*)&s & 0x7f;
    } else {
        begin = s.data;
        len = s.size;
    }
    if (pos > len) {
        F58Exc e;
        F58Exc* mp = &e;
        const char* message = puppetPreloadStrings.compareRangeError;
        char alloc __attribute__((aligned(4))) = 0;
        mp->vt = &__vt__Q23std11logic_error;
        unsigned int mlen = strlen(message);
        char* text = (char*)__nwa__FUl((mlen + 1) * 2 - (mlen + 1));
        mp->text = text;
        {
            F58Guard guard = {text, &alloc};
            unsigned int* rep = (unsigned int*)__nw__FUl(0x10);
            if (rep) {
                rep[1] = 1;
                rep[2] = 1;
                rep[0] = (unsigned int)&s_sharedCharDeleterVtable;
                rep[3] = (unsigned int)text;
            }
            mp->rep = rep;
            guard.text = 0;
        }
        strcpy(mp->text, message);
        mp->vt = &__vt__Q23std12out_of_range;
        __throw((const char*)(puppetPreloadStrings.outOfRangeType), mp, (void*)__dt__Q23std12out_of_rangeFv);
    }
    unsigned int rem = len - pos;
    unsigned int rlen = f58min(n1, rem);
    const char* first = begin + pos;
    int r = memcmp((void*)first, (void*)str, f58min(slen, rlen));
    if (r == 0) {
        if (rlen < slen)
            return -1;
        if (rlen == slen)
            return 0;
        return 1;
    }
    return r;
}

// Choose the synchronous preloader by the resource suffix and retain its object in the preload vector.
int WPuppet_PreloadNamedResource(void* self, char* name) {
    // Return 0 for no preload vector or no suffix. Return 1 means a suffixed
    // request was processed, even if allocation/loading produced no handle.
    // fileSuffix is intentionally EXCLUDED: equality skips generic file creation.
    // Other unrecognized suffixes request the generic preloader using ORIGINAL name,
    // while policy compares a normalized temporary path.
    if (!AsWPuppet(self)->namedResources)
        return 0;
    // Phase 1: normalize a temporary lookup key; keep the original name for acquisition.
    F58Str path;
    unsigned int length = strlen(name);
    String_ReserveCapacity(&path, length);
    volatile char stackByte __attribute__((aligned(4)));
    char local __attribute__((aligned(4))) = stackByte;
    ((void (*)(void*, int, int, const char*, const char*, const char*))String_ReplaceRange)(&path, 0, 0, name,
                                                                                            name + length, &local);
    ResourcePath_NormalizeImageExtension(&path);
    unsigned int pos = (unsigned int)-1;
    const char* begin;
    unsigned int len;
    if (!(path.hdr >> 31)) {
        begin = (const char*)&path + 1;
        len = *(unsigned char*)&path & 0x7f;
    } else {
        begin = path.data;
        len = path.size;
    }
    unsigned int dot;
    if (len) {
        unsigned int last = len - 1;
        if (pos > last)
            pos = last;
        const char* p = begin + pos;
        for (;;) {
            if (*p == '.') {
                dot = p - begin;
                goto dot_found;
            }
            if (p <= begin)
                break;
            --p;
        }
    }
    dot = (unsigned int)-1;
dot_found: // Keep the result join: structured scans change register allocation.
    if (dot == (unsigned int)-1)
        return 0;
    // Phase 2: suffix policy (fileSuffix comparison intentionally uses !=).
    int acquiredPreloader = 0;
    const char* suffix;
    suffix = puppetPreloadStrings.spriteSuffix;
    if (f58_compare(path, dot, suffix) == 0)
        acquiredPreloader = (int)CPreloadSprSync::Create((const char*)name);
    else if (suffix = puppetPreloadStrings.sequenceSuffix, f58_compare(path, dot, suffix) == 0)
        acquiredPreloader = (int)CPreloadSeqSync::Create(name);
    else if (suffix = puppetPreloadStrings.textureSuffix, f58_compare(path, dot, suffix) == 0)
        acquiredPreloader = (int)CPreloadTextureSync::Create(name);
    else if (suffix = puppetPreloadStrings.fileSuffix, f58_compare(path, dot, suffix) != 0)
        acquiredPreloader = (int)CPreloadFileSync::Create(name);
    // Phase 3: append only an acquired handle, growing pointer storage transactionally.
    if (acquiredPreloader) {
        struct F58Vec2 {
            void** data;
            unsigned int size;
            unsigned int capacity;
        };
        F58Vec2* vec = (F58Vec2*)AsWPuppet(self)->namedResources;
        if (vec->size < vec->capacity) {
            ++vec->size;
            vec->data[vec->size - 1] = (void*)acquiredPreloader;
        } else {
            if (0x3FFFFFFFu - vec->capacity < 1u)
                f58_length_error();
            F58Vec tmp(&vec->capacity);
            unsigned int needed = vec->size + 1 - vec->capacity;
            unsigned int capacity = vec->capacity;
            if (needed > 0x3FFFFFFFu - capacity)
                f58_length_error();
            unsigned int newCap;
            if (capacity < 0x15555555u)
                newCap = capacity + f58max((capacity + 1) * 3 / 5, needed);
            else if (capacity < 0x2AAAAAAAu)
                newCap = capacity + f58max((capacity + 1) / 2, needed);
            else
                newCap = 0x3FFFFFFF;
            if (newCap > 0x3FFFFFFFu)
                f58_length_error();
            tmp.data = (void**)appMallocDummy__FUl(newCap * 4);
            tmp.extra = vec->size;
            (tmp.data + tmp.extra)[tmp.size++] = (void*)acquiredPreloader;
            void** old = vec->data;
            unsigned int after = (unsigned int)((old + vec->size) - old);
            tmp.extra = tmp.extra - after;
            tmp.capacity = newCap;
            unsigned int bytes = after * 4;
            memcpy(tmp.data + tmp.extra, old, bytes);
            memset(old, 0, bytes);
            vec->size = 0;
            tmp.size += after;
            unsigned int oldCapacity = vec->capacity;
            vec->capacity = tmp.capacity;
            tmp.capacity = oldCapacity;
            void** oldData = vec->data;
            vec->data = tmp.data;
            tmp.data = oldData;
            vec->size = tmp.size;
            tmp.size = 0;
        }
    }
    // Success means the suffixed request was processed, including an empty acquisition.
    return 1;
}
// Fill the caller's bone-transform buffer from this model or its first-child model.
int WPuppet_EvaluateModelTransformAnimation(void* self, void* destination) {
    if (!destination)
        return 0;
    void* first = AsWPuppet(self)->modelResource;
    void* second = first;
    if (!first && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* obj;
        if (AsWPuppet(self)->childCount == 0)
            obj = 0;
        else
            obj = ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
        if (obj)
            second = AsWPuppet(obj)->modelResource;
    }
    if (!second)
        return 0;
    if (!first && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* obj;
        if (AsWPuppet(self)->childCount == 0)
            obj = 0;
        else
            obj = ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
        if (obj)
            first = AsWPuppet(obj)->modelResource;
    }
    int named = first ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(first, kChunkBoneTransforms) : 0;
    if (((gtl::IGenericModelingResource*)second)->EvaluateTransformAnimation(destination, (void*)named))
        return 1;
    return 0;
}

void* Matrix_TransformPoint(void* arg0, void* arg1, void* arg2);
void __as__7WVectorFRC7WVector(void* arg0, void* arg1);
void __pl__FRC7WVectorRC7WVector(void* arg0, void* arg1, void* arg2);
void __ml__FRC7WVectorf(void* arg0, void* arg1, float arg2);
void __mi__FRC7WVectorRC7WVector(void* arg0, void* arg1, void* arg2);
float Magnitude__7WVectorCFv(void* arg0);
void AABB_Copy(void* arg0, void* arg1);
void Matrix_ExpandAffineTo4x4(void* arg0, void* arg1);
void Vec3_CopySourceToDestination(void* arg0, void* arg1);
float Vec3_Dot(void* arg0, const char* arg1);
void Mtx44_TransformPointPerspective(void* arg0, void* arg1, void* arg2);
extern float kFloat_30_805DD7C8;
extern float kFloat_0_5_805DD7CC;
extern const float kFloat_0_78125_Bounds;
extern const char kUnitXAxisVector[];
extern const char kUnitYAxis[];
extern const char kUnitZAxis_Bounds[];
// Copy explicit bounds or derive model bounds and radius from the resource bounding-volume chunk.
void WPuppet_UpdateBoundsFromResourceBox(void* selfArg, int explicitBounds, int unused) {
    unsigned char* self = (unsigned char*)selfArg;
    void* boundsAxisY;
    void* boundsAxisZ;
    unsigned char* boundsChunk;
    float matrix[16];
    float bounds[6];
    float vector[3];
    float negativeExtent[3];
    float positiveExtent[3];
    float b[3];
    float a[3];
    float c[3];
    float d[3];
    if (explicitBounds) {
        AABB_Copy(self + 0x10C, (void*)explicitBounds);
        __pl__FRC7WVectorRC7WVector(a, self + 0x10C, self + 0x118);
        __ml__FRC7WVectorf(b, a, kFloat_0_5_805DD7CC);
        __as__7WVectorFRC7WVector(self + 0x124, b);
        __mi__FRC7WVectorRC7WVector(c, self + 0x124, self + 0x10C);
        AsWPuppet(self)->boundsRadius = Magnitude__7WVectorCFv(c);
        return;
    }
    void* entry = AsWPuppet(self)->modelResource;
    if (entry == 0 && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* obj;
        if (AsWPuppet(self)->childCount == 0)
            obj = 0;
        else
            obj = *(void**)((unsigned char*)AsWPuppet(self)->childHead + 8);
        if (obj)
            entry = AsWPuppet(obj)->modelResource;
    }
    if (entry == 0)
        return;
    boundsChunk = (unsigned char*)GetChunkElement__Q23gtl20IGenericResourceFileFii(entry, (const char*)kChunkBounds, 0);
    vector[0] = *(float*)(boundsChunk + 0x30);
    vector[1] = *(float*)(boundsChunk + 0x34);
    vector[2] = *(float*)(boundsChunk + 0x38);
    Matrix_ExpandAffineTo4x4(self + 0x50, matrix);
    Mtx44_TransformPointPerspective(vector, vector, matrix);
    Vec3_CopySourceToDestination(vector, self + 0x124);
    boundsAxisY = boundsChunk + 0x10;
    boundsAxisZ = boundsChunk + 0x20;
    float axisExtentX = Vec3_Dot(boundsAxisY, kUnitXAxisVector);
    float t = Vec3_Dot(boundsChunk, kUnitXAxisVector);
    axisExtentX = t + axisExtentX;
    t = Vec3_Dot(boundsAxisZ, kUnitXAxisVector);
    axisExtentX = t + axisExtentX;
    float halfExtentX = kFloat_0_78125_Bounds * (kFloat_0_5_805DD7CC * axisExtentX);
    float axisExtentY = Vec3_Dot(boundsAxisY, kUnitYAxis);
    t = Vec3_Dot(boundsChunk, kUnitYAxis);
    axisExtentY = t + axisExtentY;
    t = Vec3_Dot(boundsAxisZ, kUnitYAxis);
    axisExtentY = t + axisExtentY;
    float halfExtentY = kFloat_0_78125_Bounds * (kFloat_0_5_805DD7CC * axisExtentY);
    float axisExtentZ = Vec3_Dot(boundsAxisY, kUnitZAxis_Bounds);
    t = Vec3_Dot(boundsChunk, kUnitZAxis_Bounds);
    axisExtentZ = t + axisExtentZ;
    t = Vec3_Dot(boundsAxisZ, kUnitZAxis_Bounds);
    axisExtentZ = t + axisExtentZ;
    float halfExtentZ = kFloat_0_78125_Bounds * (kFloat_0_5_805DD7CC * axisExtentZ);
    positiveExtent[0] = halfExtentX;
    positiveExtent[1] = halfExtentY;
    positiveExtent[2] = halfExtentZ;
    negativeExtent[0] = -halfExtentX;
    negativeExtent[1] = -halfExtentY;
    negativeExtent[2] = -halfExtentZ;
    __as__7WVectorFRC7WVector(bounds, negativeExtent);
    __as__7WVectorFRC7WVector(&bounds[3], positiveExtent);
    AABB_Copy(self + 0x10C, bounds);
    __mi__FRC7WVectorRC7WVector(d, self + 0x124, self + 0x10C);
    AsWPuppet(self)->boundsRadius = Magnitude__7WVectorCFv(d);
}

extern float kFloat_1_Puppet;
void WPuppet_GetRootMotionDelta(void* result, float arg1, void* arg2, float arg3, void* arg4);
void __pl__FRC7WVectorRC7WVector(void* arg0, void* arg1, void* arg2);
void __ml__FRC7WVectorf(void* arg0, void* arg1, float arg2);

// Compute inverse-root displacement(endFrame) - displacement(startFrame), in
// resource model-coordinate distance units; see GetInverseRootMotionOffset for
// its destructive zero-pose sampling / serialization contract. out needs 3 floats.
// Inputs are absolute 30-Hz key frames (npc_pola_penguin.cpp converts seconds before calling;
// pet_body_animation_and_movement.cpp passes integer frame samples). Equal endpoints return zero immediately.
// A nonnull rangeName MUST resolve in animationRanges. Missing names are currently
// dereferenced, not rejected; use the ForRange query for a checked single sample.
// MOTI start/end/blendEnd are seconds (LoadMotionRangesFromChunk). Preserve this
// exact legacy arithmetic, including its SECOND factor of 30:
//   roundedDuration = trunc((endSeconds-startSeconds)*30 + 0.5)
//   boundary = min(endFrame, 30*roundedDuration)
// This boundary is not simply endSeconds*30; a consistent physical dimension for
// the extra scaling is unestablished. It is not a separate frame-rate setting.
// Past boundary, weight = 30*((endFrame-30*endSeconds)/(blendEndSeconds-endSeconds)).
// Weight is UNCLAMPED; denominator must be nonzero for a finite value (calculated
// even for stop/unknown modes). The loader initially sets blendEnd==end; callers
// entering this branch must arrange valid metadata or inherit that division.
// Mode 1 stops; mode 2 samples the referenced named range and the unnamed base,
// weighting them by weight and 1-weight. Names must resolve and recursive range
// references must terminate; no cycle/depth guard. Crossing a strict boundary
// evaluates the later interval FIRST, then the earlier interval, then sums them.
// Reverse intervals are accepted but are not normalized/reordered by this code.
#define kRootMotionStop 1
#define kRootMotionBlend 2
void WPuppet_GetRootMotionDelta(void* result, float startFrame, void* self, float endFrame, void* rangeName) {
    if (startFrame == endFrame) {
        float z = kFloat_0_puppet;
        *(float*)result = z;
        *((float*)result + 1) = z;
        *((float*)result + 2) = z;
        return;
    }
    if (rangeName) {
        void** end;
        void* found;
        void* array = AsWPuppet(self)->animationRanges;
        if (!array) {
            found = 0;
        } else {
            void** item = *(void***)array;
            end = item + *(unsigned int*)((char*)array + 4);
            while (item != end || (found = 0, false)) {
                found = *item;
                if (found && strcmp(*(const char**)&((PuppetMotionRangeView*)found)->name, (const char*)rangeName) == 0)
                    break;
                ++item;
            }
        }
        float roundedDurationFrames =
            (((PuppetMotionRangeView*)found)->endTime - ((PuppetMotionRangeView*)found)->startTime) *
                kFloat_30_805DD7C8 +
            kFloat_0_5_805DD7CC;
        int durationFrames = (int)roundedDurationFrames;
        float boundary = kFloat_30_805DD7C8 * (float)durationFrames;
        if (boundary > endFrame)
            boundary = endFrame;
        else
            boundary = kFloat_30_805DD7C8 * (float)durationFrames;
        if (endFrame > boundary && startFrame < boundary) {
            char earlierDelta[12], laterDelta[12];
            WPuppet_GetRootMotionDelta(laterDelta, boundary, self, endFrame, rangeName);
            WPuppet_GetRootMotionDelta(earlierDelta, startFrame, self, boundary, rangeName);
            __pl__FRC7WVectorRC7WVector(result, earlierDelta, laterDelta);
            return;
        }
        if (endFrame > boundary && startFrame >= boundary) {
            float blendWeight =
                kFloat_30_805DD7C8 *
                ((endFrame - kFloat_30_805DD7C8 * ((PuppetMotionRangeView*)found)->endTime) /
                 (((PuppetMotionRangeView*)found)->blendEndTime - ((PuppetMotionRangeView*)found)->endTime));
            float rangeEndSeconds = ((PuppetMotionRangeView*)found)->endTime;
            int type = *(int*)&((PuppetMotionRangeView*)found)->blendMode;
            switch (type) {
            case kRootMotionBlend: {
                char weightedBase[12], baseDelta[12], weightedBlend[12], blendDelta[12];
                WPuppet_GetRootMotionDelta(blendDelta, startFrame - kFloat_30_805DD7C8 * rangeEndSeconds, self,
                                           endFrame - kFloat_30_805DD7C8 * rangeEndSeconds,
                                           *(void**)&((PuppetMotionRangeView*)found)->blendSourceName);
                __ml__FRC7WVectorf(weightedBlend, blendDelta, blendWeight);
                WPuppet_GetRootMotionDelta(
                    baseDelta, startFrame + kFloat_30_805DD7C8 * ((PuppetMotionRangeView*)found)->startTime, self,
                    endFrame + kFloat_30_805DD7C8 * ((PuppetMotionRangeView*)found)->startTime, 0);
                __ml__FRC7WVectorf(weightedBase, baseDelta, kFloat_1_Puppet - blendWeight);
                __pl__FRC7WVectorRC7WVector(result, weightedBase, weightedBlend);
                return;
            }
            case kRootMotionStop: {
                float z = kFloat_0_puppet;
                *(float*)result = z;
                *((float*)result + 1) = z;
                *((float*)result + 2) = z;
                return;
            }
            }
        }
    }
    char endOffset[12], startOffset[12];
    WPuppet_GetInverseRootMotionOffset(startOffset, self, startFrame);
    WPuppet_GetInverseRootMotionOffset(endOffset, self, endFrame);
    __mi__FRC7WVectorRC7WVector(result, endOffset, startOffset);
}
#undef kRootMotionStop
#undef kRootMotionBlend
// Transform the selected bounds by its anchor bone, update center and radius, and visit child puppets.
void WPuppet_UpdateBoundsFromBone(void* self) {
    if (AsWPuppet(self)->boundsBone) {
        // Keep the original scratch-buffer sizes and copies for CodeWarrior's stack layout.
        char minimum[0x18], maximum[12], transformedMinimum[0x18], transformedMaximum[12], center[12], endpointSum[12],
            centerToMaximum[12];
        Matrix_TransformPoint(transformedMinimum, AsWPuppet(self)->boundsData,
                              ((PuppetBoneView*)AsWPuppet(self)->boundsBone)->worldMatrix);
        *(WVector*)minimum = *(WVector*)transformedMinimum;
        Matrix_TransformPoint(transformedMaximum, &((PuppetBoundsView*)AsWPuppet(self)->boundsData)->maximum,
                              ((PuppetBoneView*)AsWPuppet(self)->boundsBone)->worldMatrix);
        *(WVector*)maximum = *(WVector*)transformedMaximum;
        __pl__FRC7WVectorRC7WVector(endpointSum, minimum, maximum);
        __ml__FRC7WVectorf(center, endpointSum, kFloat_0_5_805DD7CC);
        AsWPuppet(self)->boundsCenter = *(WVector*)center;
        __mi__FRC7WVectorRC7WVector(centerToMaximum, maximum, (char*)&AsWPuppet(self)->boundsCenter);
        AsWPuppet(self)->boundsRadius = (*(WVector*)centerToMaximum).Magnitude();
    }
    struct MethodOwner {
        void invoke() {}
    };
    typedef void (MethodOwner::*Method)(void);
    union MethodData {
        Method method;
        int words[3];
    } method;
    extern MethodData s_puppetUpdateBoundsFromBoneMethod;
    method.method = s_puppetUpdateBoundsFromBoneMethod.method;
    union MethodData local;
    local.method = method.method;
    void* end = (char*)&AsWPuppet(self)->childTail;
    for (void* node = AsWPuppet(self)->childHead; node != end; node = ((WPuppetListNode*)node)->next) {
        void* obj = ((WPuppetListNode*)node)->value;
        (((MethodOwner*)obj)->*local.method)();
    }
}

// Free every node in a count-and-sentinel list, then optionally delete the list object.
void* WPuppetChildList_dtor(MSLPointerList* self, int deleting) {
    return ::std::__list_deleting_dtor(self, deleting);
}

#pragma use_lmw_stmw on
// Erase each consecutive child-list run matching the supplied value and update the list count.
void PuppetChildList_EraseMatchingValue(void* arg0, void* arg1, void* arg2) {
    void* end = (char*)arg0 + 4;
    void* node = *(void**)((char*)arg0 + 8);
    while (node != end) {
        void* key = *(void**)((char*)arg1 + 4);
        key = *(void**)key;
        if (!(*(void**)((char*)node + 8) == key)) {
            node = *(void**)((char*)node + 4);
        } else {
            void* last = *(void**)((char*)node + 4);
            while (last != end) {
                if (*(void**)((char*)last + 8) != key)
                    break;
                last = *(void**)((char*)last + 4);
                if (last == end)
                    break;
            }
            if (node != last) {
                void* data = *(void**)last;
                void* prev = *(void**)((char*)data + 4);
                void* first = *(void**)node;
                *(void**)((char*)first + 4) = prev;
                first = *(void**)node;
                prev = *(void**)((char*)data + 4);
                *(void**)prev = first;
                while (node != last) {
                    void* cur = node;
                    node = *(void**)((char*)node + 4);
                    appFreeUni__FPv(cur);
                    *(int*)arg0 = *(int*)arg0 - 1;
                }
            }
            node = last;
            if (node == end)
                return;
            node = *(void**)((char*)node + 4);
        }
    }
}

// Optionally refresh bone matrices, update model bounds, and set the radius to half the bounds diagonal.
void WPuppet_UpdateBound(void* self, int refreshBones) {
    extern float kFloat_1_Puppet;
    if (refreshBones) {
        void* p = AsWPuppet(self)->boneRoot;
        if (p)
            ((void (*)(void*, float, void*, int, int))WPuppetBone_ConcatMatrixAlongChain)(
                p, kFloat_1_Puppet, (char*)&AsWPuppet(self)->worldMatrix, 0, 0);
    }
    WPuppet_UpdateBoundsFromResourceBox(self, 0, 0);

    WVector tmp = AsWPuppet(self)->boundsMax - AsWPuppet(self)->boundsMin;
    extern float kFloat_0_5_805DD7CC;
    AsWPuppet(self)->boundsRadius = kFloat_0_5_805DD7CC * WVectorLen(tmp);
}

// Find the bone/bounds pair whose stored collision name matches the requested name.
void* WPuppet_FindNamedBoneEntry(void* self, void* boneName) {
    unsigned int offset;
    unsigned int i;
    unsigned int count;
    i = 0;
    offset = 0;
    count = AsWPuppet(self)->namedBoneCount;
    while (i < count) {
        char* entry = *(char**)&AsWPuppet(self)->namedBones + offset;
        if (entry && ((PuppetNamedBoneView*)entry)->bounds &&
            !strcmp((const char*)boneName, ((PuppetBoundsView*)((PuppetNamedBoneView*)entry)->bounds)->name))
            return entry;
        offset += 8;
        ++i;
    }
    return 0;
}

// Evaluate the effective model's transforms into caller-owned buffers and return success as zero or one.
int WPuppet_EvaluateTransforms(void* self, void* skinMatrices, void* skinCount, void* worldMatrix, void* boneMatrices,
                               void* boneCount) {
    void* p = AsWPuppet(self)->modelResource;
    if (!p && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* q;
        if (AsWPuppet(self)->childCount == 0)
            q = 0;
        else {
            q = AsWPuppet(self)->childHead;
            q = *(void**)((char*)q + 8);
        }
        if (q)
            p = AsWPuppet(q)->modelResource;
    }
    if (!p)
        return 0;
    int result = ((int (*)(void*, void*, void*, void*, void*, void*,
                           int))EvaluateTransforms__Q23gtl24IGenericModelingResourceCFPvPvPvPvUiPUi)(
        p, skinMatrices, skinCount, worldMatrix, boneMatrices, boneCount, 0);
    unsigned int a = (unsigned int)-result;
    unsigned int b = (unsigned int)result;
    return (int)((a | b) >> 31);
}

// Initialize both endpoints of an axis-aligned bounding box to zero.
void* PuppetBounds_ZeroEndpoints(void* bounds) {
    Vec3_Zero(bounds);
    Vec3_Zero((char*)&((PuppetBoundsView*)bounds)->maximum);
    return bounds;
}

// Erase the bone-controller override keyed by the supplied index.
int WPuppet_EraseBoneControllerEntry(void* self, int index) {
    void* p = AsWPuppet(self)->boneController;
    if (!p)
        return 0;
    return BoneControllerMap_EraseByIndex(p, &index);
}

// Clear the per-puppet value copied into the render-state list.
void WPuppet_ClearRenderStateValue(void* self) {
    *(int*)&AsWPuppet(self)->renderStateValue = 0;
}
// Initialize the key, flags, and owned string pointer of a key record.
void PuppetKeyRecord_Clear(void* record) {
    *(int*)record = 0;
    *(int*)&((PuppetKeyRecord*)record)->flags = 0;
    *(int*)&((PuppetKeyRecord*)record)->text = 0;
}
// Return an animation entry's name length, or zero when no animation table is loaded.
int WPuppet_GetAnimationNameSize(void* self, unsigned short idx) {
    void* p = AsWPuppet(self)->animation;
    if (!p)
        return 0;
    return ((CPetAnimation*)p)->GetEntryNameSize(idx);
}
// The caller passes the index as a full word; a member call would truncate it to 16 bits first.
extern "C" int CopyEntryName__13CPetAnimationFPcUsUi(void* self, char* destination, unsigned int index,
                                                     unsigned int size);
// Copy an indexed animation name into the caller's buffer.
int WPuppet_CopyAnimationName(void* self, void* destination, unsigned int index, unsigned int size) {
    void* p = AsWPuppet(self)->animation;
    if (!p)
        return 0;
    return CopyEntryName__13CPetAnimationFPcUsUi(p, (char*)destination, index, size);
}
// Evaluate a named animation's transforms at the requested time.
int WPuppet_EvaluateAnimationTransforms(void* self, void* destination, int index, const char* name, float time) {
    void* p = AsWPuppet(self)->animation;
    if (!p)
        return 0;
    return ((CPetAnimation*)p)->EvaluateNamedTransforms((int)destination, index, time, name);
}
// Fetch the scalar animation value associated with a name.
int WPuppet_GetAnimationValueByName(void* self, float* destination, const char* name) {
    void* p = AsWPuppet(self)->animation;
    if (!p)
        return 0;
    return ((CPetAnimation*)p)->GetValueByName(destination, name);
}

extern void* __nwa__FUl(unsigned int);

extern void __as__7WVectorFRC7WVector(void*, void*);
extern void AABB_Copy(void*, void*);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void __ml__FRC7WVectorf(void*, void*, float);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float WVectorLen__FRC7WVector(void*);

extern void __dl__FPv(void*);
extern void __dla__FPv(void*);

extern float kFloat_0_001_ObjectBounds;
extern float kFloat_0_5_805DD7CC;

struct CollisionBoneBoundsEntry {
    void* object;
    void* data;
};
struct Fn80056814Object {
    char unk0[0x28];
    void* text;
    char name[0x100];
};
typedef Vec3f CollisionVec3;
struct CollisionMinimumStaging {
    float x, y, z, pad;
};
struct Fn80056814Obj {
    char first[12];
    char second[12];
    char sphereAndName[0x12C - 0x18];
    // Initialize both bounds vectors to zero before their later population.
    Fn80056814Obj() {
        Vec3_Zero(first);
        Vec3_Zero(second);
    }
    // Allocate array storage through the game's array allocator.
    static void* operator new[](size_t n) {
        return __nwa__FUl(n);
    }
};
// Loader alias map (all offsets relative to WPuppet):
// +134 boundsBone / +138 boundsData select one borrowed entry from +13C namedBones.
// +13C entries are {bone pointer, bounds pointer}, stride 8; +140 is their count.
// +164 boneRoot owns the searchable bone chain. +22C boundObjects owns the
// 0x12C-stride bounds array; each entry's data points into that array.
// Bounds +00/+0C/+18/+24/+28/+2C = min/max/center/radius/owned command/name.
// Fn80056814Obj constructs the same allocation as PuppetBoundsView; its two
// initial vectors are the bounds min/max, not a second collision representation.
struct CollisionLoadView {
    char pad_000[0x134];
    void* boundsBone;
    void* boundsData;
    CollisionBoneBoundsEntry* namedBones;
    unsigned int namedBoneCount;
    char pad_144[0x164 - 0x144];
    char* boneRoot;
    char pad_168[0x22C - 0x168];
    char* boundObjects;
};
// COLL command offsets are chunk-relative; lengths are the packed 16-bit fields.
// Cursor views below advance offsets by four bytes and lengths by two bytes.
struct CollisionCommandReadView {
    unsigned int nameOffset;
    unsigned int commandOffsets[2];
    Vec3f minimum, maximum;
    unsigned short commandLengths[2];
    unsigned short nameLength, boneIndex;
    char unresolved2C[4];
};
enum {
    kCollisionCommandStagingBytes = 0x200,
    kCollisionCommandCount = 2
};
typedef char
    CollisionCommandReadView_size[(sizeof(CollisionCommandReadView) == sizeof(PuppetCollisionRecord)) ? 1 : -1];

// Copy exactly three decoded float words, retaining the game's vector-copy ABI.
// The minimum staging record has a trailing alignment lane; only XYZ is copied.
static inline void CopyDecodedCollisionVector(CollisionVec3* output, const void* decodedXYZ) {
    *(WVector*)output = *(const WVector*)decodedXYZ;
}

// Join nontrivial command strings; the bounds record owns the new allocation.
static inline void AllocateCollisionCommand(CollisionBoneBoundsEntry*& namedBone, char (&firstCommand)[0x400]) {
    if (strlen(firstCommand) > 1 || strlen(firstCommand + 0x200) > 1) {
        unsigned int secondCommandLength = strlen(firstCommand + 0x200);
        unsigned int firstCommandLength = strlen(firstCommand);
        // One separator and the terminating NUL accompany the two spans.
        unsigned int commandBytes = firstCommandLength + secondCommandLength + 2;
        unsigned int allocationBytes = 2 * commandBytes;
        *(void**)&((PuppetBoundsView*)namedBone->data)->command = __nwa__FUl(allocationBytes - commandBytes);
        sprintf(*(char**)&((PuppetBoundsView*)namedBone->data)->command,
                ((const char*)&puppetPreloadStrings + PUPPET_OFFSET(PuppetPreloadStrings, collisionCommandFormat)),
                firstCommand, firstCommand + 0x200);
    }
}

// Load COLL boxes and names, enforce nonzero extents, bind them to bones, and select the default bounds anchor.
void WPuppet_LoadCollisionChunks(void* selfArg, void* data, unsigned int dataSize) {
    // Phase 1: locate COLL before allocating tables. Producer/caller contract:
    // every visited header is complete; byteSize is nonzero and stays in dataSize;
    // COLL dataOffset + entryCount*0x30 stays in that chunk; count is nonnegative
    // in signed 16-bit range. Span offsets are relative to COLL, not its payload.
    // All name/command spans stay inside the chunk. For ordinary unterminated
    // file spans, lengths must be <512 so zero-filled staging retains a NUL;
    // the decoded name must additionally fit the 256-byte runtime name field.
    // This routine performs none of those validations; extending the producer
    // must preserve them or introduce a separately accepted checked reader.
    struct {
        CollisionVec3 minimum;
        CollisionVec3 maximum;
    } bounds;
    CollisionVec3& minimum = bounds.minimum;
    CollisionVec3& maximum = bounds.maximum;
    int chunkOffset = 0;
    float halfScale;
    char firstCommand[0x400];

    CollisionMinimumStaging minimumPosition;
    char collisionName[0x200];
    CollisionVec3 maximumPosition;
    unsigned char center[12];

    CollisionLoadView* self = (CollisionLoadView*)selfArg;
    char* collisionChunk;
    const char* collisionTag = ((const char*)&puppetPreloadStrings + PUPPET_OFFSET(PuppetPreloadStrings, collisionTag));

    // Exhaustion assigns null to collisionChunk and terminates the search.
    for (; (chunkOffset < dataSize) || (collisionChunk = 0);
         chunkOffset += ((PuppetChunkHeader*)collisionChunk)->byteSize) {
        int chunkOffsetCopy = chunkOffset;
        collisionChunk = (char*)data + chunkOffsetCopy;
        if (memcmp((void*)collisionTag, collisionChunk, 4) == 0)
            break;
    }
    if (collisionChunk == 0)
        return;

    // Phase 2: allocate one {bone,bounds} entry and one owned bounds record per COLL item.
    Vec3_Zero(&minimum.x);
    Vec3_Zero(&maximum);

    char* collisionRecord;
    short collisionCount;
    collisionRecord = collisionChunk + ((PuppetChunkHeader*)collisionChunk)->dataOffset;
    collisionCount = *(short*)&((PuppetChunkHeader*)collisionChunk)->entryCount; // signed format count
    if (0 == collisionCount)
        return;

    // Preserve the signed 16-bit format count through each allocation/publication.
    short allocationCount = collisionCount;
    self->namedBones =
        (CollisionBoneBoundsEntry*)__nwa__FUl((unsigned int)allocationCount * sizeof(CollisionBoneBoundsEntry));
    short tableCount = collisionCount;
    CollisionBoneBoundsEntry* namedBonesCopy = self->namedBones;
    Memory_Clear(namedBonesCopy, (unsigned int)tableCount * sizeof(CollisionBoneBoundsEntry));

    short boundsCount = collisionCount;
    self->boundObjects = (char*)new Fn80056814Obj[boundsCount];
    short publishedCount = collisionCount;
    Memory_Clear(self->boundObjects, (unsigned int)collisionCount * sizeof(PuppetBoundsView));
    self->namedBoneCount = publishedCount;

    float epsilon;
    epsilon = kFloat_0_001_ObjectBounds;
    halfScale = kFloat_0_5_805DD7CC;

    // Phase 3: decode bone-local boxes/names, bind bones by index/name, enforce
    // epsilon extents, and derive the center/radius from the source endpoints.
    for (unsigned int collisionIndex = 0; collisionIndex < (unsigned int)collisionCount;
         collisionRecord += sizeof(PuppetCollisionRecord), ++collisionIndex) {
        float minimumX;
        float maximumX;
        unsigned int entryByteOffset = collisionIndex * 8;
        unsigned int lookupByteOffset = entryByteOffset;
        CollisionBoneBoundsEntry* namedBone = (CollisionBoneBoundsEntry*)((char*)self->namedBones + lookupByteOffset);
        CollisionVec3* minimumSource = (CollisionVec3*)&((PuppetCollisionRecord*)collisionRecord)->minimum;
        char* boundsStorage = self->boundObjects;

        namedBone->data = boundsStorage + collisionIndex * sizeof(PuppetBoundsView);
        float minimumY;
        float minimumZ;
        minimumZ = minimumSource->z;
        minimumY = minimumSource->y;
        minimumX = minimumSource->x;
        minimumPosition.x = minimumX;
        minimumPosition.y = minimumY;
        minimumPosition.z = minimumZ;
        CopyDecodedCollisionVector(&minimum, &minimumPosition);
        CollisionVec3* maximumSource = (CollisionVec3*)&((PuppetCollisionRecord*)collisionRecord)->maximum;
        float maximumZ;
        float sourceMaximumY = maximumSource->y;
        float maximumY;
        maximumZ = maximumSource->z;
        maximumY = sourceMaximumY;
        maximumX = maximumSource->x;
        maximumPosition.x = maximumX;
        maximumPosition.z = maximumZ;
        maximumPosition.y = maximumY;
        CopyDecodedCollisionVector(&maximum, &maximumPosition);

        Memory_Clear(collisionName, 0x200);
        Memory_Copy(collisionName, (char*)collisionChunk + ((PuppetCollisionRecord*)collisionRecord)->textOffset,
                    ((PuppetCollisionRecord*)collisionRecord)->textLength);

        char* boneChain = self->boneRoot;
        int boneIndex = ((PuppetCollisionRecord*)collisionRecord)->boneIndex;
        char* boneName;
        /* Find the bone name for boneIndex; exhaustion leaves boneName null. */
        for (char* node = boneChain; (node) || (boneName = 0, false); node = *(char**)&((PuppetBoneView*)node)->next) {
            if (boneIndex == ((PuppetBoneView*)node)->index) {
                boneName = ((PuppetBoneView*)node)->name;
                break;
            }
        }
        namedBone->object = boneName ? WPuppetBone_FindBone(boneChain, boneName, 0) : 0;

        AABB_Copy(namedBone->data, &minimum.x);

        if (((float*)namedBone->data)[3] - ((float*)namedBone->data)[0] < epsilon)
            ((float*)namedBone->data)[3] = ((float*)namedBone->data)[3] + epsilon;
        if (((float*)namedBone->data)[4] - ((float*)namedBone->data)[1] < epsilon)
            ((float*)namedBone->data)[4] = ((float*)namedBone->data)[4] + epsilon;
        if (((float*)namedBone->data)[5] - ((float*)namedBone->data)[2] < epsilon)
            ((float*)namedBone->data)[5] = ((float*)namedBone->data)[5] + epsilon;

        *(int*)&((PuppetBoundsView*)namedBone->data)->command = 0;
        WVector endpointSum = *(WVector*)(&minimum.x) + *(WVector*)&maximum;
        __ml__FRC7WVectorf(center, &endpointSum, kFloat_0_5_805DD7CC);
        __typeof__(namedBone->data) boundsData1 = namedBone->data;
        __typeof__(namedBone->data) boundsData2 = namedBone->data;
        __typeof__(namedBone->data) boundsData3 = boundsData2;
        __typeof__(namedBone->data) boundsData4 = namedBone->data;
        __typeof__(boundsData3) boundsData5 = namedBone->data;
        __typeof__(namedBone->data) boundsData6 = namedBone->data;
        __typeof__(namedBone->data) boundsData7 = boundsData4;
        __typeof__(boundsData7) boundsData8 = boundsData1;
        __typeof__(boundsData8) boundsData9 = boundsData6;
        *(WVector*)((char*)&((PuppetBoundsView*)boundsData9)->center) = *(WVector*)center;
        unsigned char diagonal[12];
        __mi__FRC7WVectorRC7WVector(diagonal, &maximum, &minimum.x);

        *(float*)&((PuppetBoundsView*)boundsData5)->radius = halfScale * (WVectorLen(*(WVector*)diagonal));

        strcpy((char*)&((PuppetBoundsView*)namedBone->data)->name, collisionName);

        // Two packed command spans become one owned formatted command string.
        char* commandDestination = firstCommand;
        char* lengthCursor = collisionRecord;
        char* offsetCursor = collisionRecord;
        for (int commandIndex = 0; kCollisionCommandCount > commandIndex; ++commandIndex) {
            Memory_Clear(commandDestination, kCollisionCommandStagingBytes);
            unsigned short commandLength = ((CollisionCommandReadView*)lengthCursor)->commandLengths[0];
            unsigned int commandOffset = ((CollisionCommandReadView*)offsetCursor)->commandOffsets[0];
            unsigned short copyLength = commandLength;
            Memory_Copy(commandDestination, (char*)collisionChunk + commandOffset, copyLength);
            char* nextLengthCursor = lengthCursor + 2;
            commandDestination += kCollisionCommandStagingBytes;
            lengthCursor = nextLengthCursor;
            offsetCursor = offsetCursor + 4;
        }
        AllocateCollisionCommand(namedBone, firstCommand);
    }

    // Phase 4: choose the preferred bounds anchor, falling back from bound to center.
    CollisionBoneBoundsEntry* anchorEntry;
    unsigned int namedBoneCount = self->namedBoneCount;
    {
        const char* collisionName =
            ((const char*)&puppetPreloadStrings + PUPPET_OFFSET(PuppetPreloadStrings, boundBone));
        /* Find the named collision entry for the bound anchor. */
        for (unsigned int commandIndex = 0; (namedBoneCount > commandIndex) || (anchorEntry = 0, false);
             ++commandIndex) {
            __typeof__(8 * commandIndex) anchorValue1 = commandIndex * 8;
            anchorEntry = (CollisionBoneBoundsEntry*)((char*)self->namedBones + anchorValue1);
            if (anchorEntry) {
                if (anchorEntry->data) {
                    if (!strcmp((const char*)collisionName, (char*)&((PuppetBoundsView*)anchorEntry->data)->name))
                        break;
                }
            }
        }
        if (anchorEntry) {
            void* anchorBone = anchorEntry->object;
            __typeof__(anchorEntry->data) anchorValue2 = anchorEntry->data;
            self->boundsBone = anchorBone;
            __typeof__(anchorEntry->data) anchorValue3 = anchorEntry->data;
            __typeof__(anchorEntry->data) anchorValue4 = anchorEntry->data;
            __typeof__(anchorValue3) anchorValue5 = anchorValue4;
            __typeof__(anchorEntry->data) anchorValue6 = anchorEntry->data;
            __typeof__(anchorEntry->data) anchorValue7 = anchorValue2;
            __typeof__(anchorValue7) anchorValue8 = anchorValue6;
            __typeof__(anchorEntry->data) anchorValue9 = anchorValue5;
            __typeof__(anchorValue8) anchorValue10 = anchorValue9;
            self->boundsData = anchorValue10;
            return;
        }
    }
    {
        const char* collisionName =
            ((const char*)&puppetPreloadStrings + PUPPET_OFFSET(PuppetPreloadStrings, centerBone));
        /* Fall back to the named collision entry for the center anchor. */
        for (unsigned int commandIndex = 0; (commandIndex < namedBoneCount) || (anchorEntry = 0, false);
             ++commandIndex) {
            __typeof__(self->namedBones) anchorValue11 = self->namedBones;
            anchorEntry = (CollisionBoneBoundsEntry*)((char*)anchorValue11 + 8 * commandIndex);
            if (anchorEntry) {
                if (anchorEntry->data) {
                    if (!strcmp((const char*)collisionName, (char*)&((PuppetBoundsView*)anchorEntry->data)->name))
                        break;
                }
            }
        }
        if (anchorEntry) {
            self->boundsBone = anchorEntry->object;
            self->boundsData = anchorEntry->data;
        }
    }
}

#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma opt_propagation off
// Free unshared collision command strings and bounds objects, then discard the named-bone lookup table.
int WPuppet_ReleaseNamedBoneData(void* self) {
    unsigned int count = AsWPuppet(self)->namedBoneCount;
    int found = HasSharedPuppetResourceName(self);
    if (!found) {
        for (unsigned int i = 0; i < count; ++i) {
            char* entry = (char*)AsWPuppet(self)->namedBones + i * 8;
            if (entry && *(void**)(entry + 4) && *(void**)((char*)*(void**)(entry + 4) + 0x28)) {
                __dla__FPv(*(void**)((char*)*(void**)(entry + 4) + 0x28));
                *(void**)((char*)*(void**)(entry + 4) + 0x28) = 0;
            }
        }
        void* allocated = AsWPuppet(self)->boundObjects;
        if (allocated) {
            __dla__FPv((char*)allocated - 0x10);
            AsWPuppet(self)->boundObjects = 0;
        }
    }
    if (AsWPuppet(self)->namedBones) {
        __dla__FPv(AsWPuppet(self)->namedBones);
        AsWPuppet(self)->namedBones = 0;
    }
    AsWPuppet(self)->namedBoneCount = 0;
    return 1;
}
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma opt_propagation reset

// Query the selected buffer kind on this model or the effective first-child model.
int WPuppet_GetModelBufferCount(void* self, int selector) {
    void* p = AsWPuppet(self)->modelResource;
    if (!p && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* q;
        if (AsWPuppet(self)->childCount == 0)
            q = 0;
        else {
            q = AsWPuppet(self)->childHead;
            q = ((WPuppetListNode*)q)->value;
        }
        if (q)
            p = AsWPuppet(q)->modelResource;
    }
    if (p)
        return ((gtl::IGenericModelingResource*)p)->GetBufferCount(selector);
    return 0;
}
// Read an indexed model resource, returning failure when no effective model exists.
int WPuppet_GetModelResource(void* self, int selector, int index, void* out) {
    void* p = AsWPuppet(self)->modelResource;
    if (!p && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* q;
        if (AsWPuppet(self)->childCount == 0)
            q = 0;
        else {
            q = AsWPuppet(self)->childHead;
            q = ((WPuppetListNode*)q)->value;
        }
        if (q)
            p = AsWPuppet(q)->modelResource;
    }
    if (p)
        return ((int (*)(void*, int, int, void*))GetResource__Q23gtl24IGenericModelingResourceFiiPP12GtlRefHeader)(
            p, selector, index, out);
    return 1;
}
// Replace an indexed resource on the effective model.
int WPuppet_SetModelResource(void* self, int selector, int index, int resource) {
    void* q;
    void* p = AsWPuppet(self)->modelResource;
    if (!p && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        if (AsWPuppet(self)->childCount == 0)
            q = 0;
        else {
            q = AsWPuppet(self)->childHead;
            q = ((WPuppetListNode*)q)->value;
        }
        if (q)
            p = AsWPuppet(q)->modelResource;
    }
    if (p)
        return ((gtl::IGenericModelingResource*)p)->SetResource(selector, index, (GtlRefHeader*)(long)resource);
    return 1;
}

// Test whether the bone override controller contains the requested index.
int WPuppet_HasBoneControllerEntry(void* self, int index) {
    void* p = AsWPuppet(self)->boneController;
    if (!p)
        return 0;
    return BoneControllerTree_ContainsKey(p, &index);
}
// Erase and delete the material-layer controller entry for an index.
int WPuppet_EraseLayerControllerEntry(void* self, int index) {
    void* p = AsWPuppet(self)->layerController;
    if (!p)
        return 0;
    return LayerControllerMap_EraseAndDeleteByKey(p, &index);
}
// Remove child-list entries matching the supplied child value through the list erasure helper.
void WPuppet_EraseMatchingChild(void* self, int childValue) {
    struct {
        int a;
        int b;
        int c;
        void* d;
        int e;
        void* f;
    } s;
    s.a = childValue;
    int t = s.c;
    s.d = &s.a;
    s.e = t;
    s.f = &s.a;
    (void)s.b;
    (void)s.d;
    (void)s.f;
    PuppetChildList_EraseMatchingValue((char*)&AsWPuppet(self)->childCount, &s.e, &s.a);
}
// Return the effective model's bone-transform count.
int WPuppet_GetFtCount(void* self) {
    void* p = AsWPuppet(self)->modelResource;
    if (!p && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* t;
        if (AsWPuppet(self)->childCount == 0) {
            t = 0;
        } else {
            void* q = AsWPuppet(self)->childHead;
            t = *(void**)((char*)q + 8);
        }
        if (t)
            p = AsWPuppet(t)->modelResource;
    }
    if (p)
        return GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, kChunkBoneTransforms);
    return 0;
}
// Return the effective model's skin-transform count.
int WPuppet_GetSnrtCount(void* self) {
    void* p = AsWPuppet(self)->modelResource;
    if (!p && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* t;
        if (AsWPuppet(self)->childCount == 0) {
            t = 0;
        } else {
            void* q = AsWPuppet(self)->childHead;
            t = *(void**)((char*)q + 8);
        }
        if (t)
            p = AsWPuppet(t)->modelResource;
    }
    if (p)
        return GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, kChunkSkinTransforms);
    return 0;
}
// Release a key record's owned text buffer and optionally delete the record.
void* PuppetKeyRecord_dtor(void* record, int deleting) {
    if (record) {
        void* p = ((PuppetKeyRecord*)record)->text;
        if (p) {
            __dla__FPv(p);
            ((PuppetKeyRecord*)record)->text = 0;
        }
        if (deleting > 0)
            __dl__FPv(record);
    }
    return record;
}
void appFreeUni__FPv(void* arg0);
void Resource_Release(void* arg0);
void* __nwa__FUl(unsigned int);
int WPuppetBone_FindBoneByTail(void* arg0, const char* arg1);
int BoneControllerMap_SetVectorEntry(void* arg0, void* arg1, void* arg2);
void WPuppetBone_Init(void*);
void WPuppetBone_SetName(void*, void*);

int WPuppet_ParseIndexedBoneName(void*, void*, void*, int*);
int WPuppet_LinkBonesFromModelResource(void*, void*, int, void*);

void __destroy_new_array(void*, void*);
void PuppetKeyRecord_Clear(void*);
void* PuppetKeyRecord_dtor(void*, int);
void WPuppetBone_Init(void*);
void WPuppetBone_dtor(void*, int);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);

// Load indexed bone names from BONE records, extract model sort keys, and build the bone hierarchy.
int WPuppet_LoadBonesFromChunks(void* self, void* data, unsigned int size) {
    unsigned int chunkOffset = 0;
    const char* boneTag = puppetPreloadStrings.boneTag;
    void* boneChunk;
    /* Search at found: stop on a match or exhausted input. */
    while ((chunkOffset < size) || (boneChunk = 0, false)) {
        boneChunk = (char*)data + chunkOffset;
        if (!memcmp((void*)boneTag, boneChunk, 4))
            break;
        chunkOffset += *(unsigned int*)((char*)boneChunk + 4);
    }
    if (!boneChunk)
        return 0;
    void* oldBones = AsWPuppet(self)->bones;
    int count = *(unsigned short*)&((PuppetChunkHeader*)boneChunk)->entryCount;
    void* boneRecord = (char*)boneChunk + *(unsigned int*)((char*)boneChunk + 8);
    AsWPuppet(self)->boneCount = count;
    if (oldBones) {
        __destroy_new_array(oldBones, WPuppetBone_dtor);
        AsWPuppet(self)->bones = 0;
    }
    int boneCount = AsWPuppet(self)->boneCount;
    int constructedArray = 0;
    void* newBones = __nwa__FUl(boneCount * kBoneBytes + 0x10);
    if (newBones) {
        constructedArray = 1;
        newBones = __construct_new_array(newBones, WPuppetBone_Init, WPuppetBone_dtor, kBoneBytes, boneCount);
    }
    AsWPuppet(self)->bones = newBones;
    char buffer[0x100];
    char temporaryBone[kBoneBytes];
    int sortKey;
    WPuppetBone_Init(temporaryBone);
    int boneIndex = 0;
    int boneByteOffset = 0;
    while (boneIndex < AsWPuppet(self)->boneCount) {
        void* nameData = (char*)boneChunk + ((PuppetBoneNameRecord*)boneRecord)->textOffset;
        Memory_Clear(buffer, 0x100);
        Memory_Copy(buffer, nameData, ((PuppetBoneNameRecord*)boneRecord)->textLength);
        WPuppet_ParseIndexedBoneName(self, buffer, buffer, &sortKey);
        if (sortKey > 0)
            AsWPuppet(self)->boneSortKey = sortKey;
        WPuppetBone_SetName((char*)AsWPuppet(self)->bones + boneByteOffset, buffer);
        ((PuppetBoneView*)((char*)AsWPuppet(self)->bones + boneByteOffset))->index = boneIndex;
        boneRecord = (char*)boneRecord + sizeof(PuppetBoneNameRecord);
        boneByteOffset += kBoneBytes;
        ++boneIndex;
    }
    int result = WPuppet_LinkBonesFromModelResource(self, AsWPuppet(self)->bones, AsWPuppet(self)->boneCount,
                                                    AsWPuppet(self)->modelResource);
    WPuppetBone_dtor(temporaryBone, -1);
    return result;
}

extern void BoneNode_SetNextChain(void*, void*, int);
extern void BoneNode_SetChildAndParent(void*, void*);
extern void Mtx44_InvertLegacyCofactors(void*, int, void*);
struct Mtx55CF8 {
    unsigned int w[16];
};
#pragma use_lmw_stmw on
// Link the contiguous bone chain and model parents, then apply inverse bind matrices to each bone.
int WPuppet_LinkBonesFromModelResource(void* self, void* bones, int boneCount, void* modelResource) {
    if (!bones || boneCount <= 0)
        return 0;
    {
        void* previousBone = bones;
        void* nextBone = (char*)bones + kBoneBytes;
        for (int i = 1; i < boneCount; ++i) {
            BoneNode_SetNextChain(previousBone, nextBone, 1);
            previousBone = nextBone;
            nextBone = (char*)nextBone + kBoneBytes;
        }
    }
    if (!modelResource)
        return (int)bones;
    void* bone = bones;
    for (int i = 0; i < boneCount; ++i) {
        void* modelLayer =
            GetChunkElement__Q23gtl20IGenericResourceFileFii(modelResource, (const char*)kChunkModelLayers, i);
        void* parentCandidate;
        unsigned short parentIndex = *(unsigned short*)&((PuppetModelLayerView*)modelLayer)->parentBone;
        if (parentIndex == 0xFFFF) {
            AsWPuppet(self)->defaultBone = bone;
        } else {
            parentCandidate = bones;
            for (int j = 0; j < boneCount; ++j) {
                if (parentIndex == *(int*)&((PuppetBoneView*)parentCandidate)->index) {
                    BoneNode_SetChildAndParent(bone, (char*)bones + j * kBoneBytes);
                    break;
                }
                parentCandidate = (char*)parentCandidate + kBoneBytes;
            }
        }
        Mtx55CF8 inverseBindMatrix = *(Mtx55CF8*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
            modelResource, (const char*)kChunkBindMatrices, i);
        float initialMatrix[12];
        Mtx44_InvertLegacyCofactors(&inverseBindMatrix, 0, &inverseBindMatrix);
        Matrix_ExtractAffineFrom4x4(&inverseBindMatrix, initialMatrix);
        WPuppetBone_SetBasicMatrix(bone, initialMatrix);
        bone = (char*)bone + kBoneBytes;
    }
    return (int)bones;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Load FRAM key bounds and records, allocating a terminated string for every record.
void WPuppet_LoadKeyRecordsFromChunk(void* self, void* data, unsigned int size) {
    int i;
    const char* frameTag = puppetPreloadStrings.frameTag;
    unsigned int chunkOffset = 0;
    void* frameChunk;
    /* Search at found: stop on a match or exhausted input. */
    while ((chunkOffset < size) || (frameChunk = 0, false)) {
        frameChunk = (char*)data + chunkOffset;
        if (memcmp((void*)frameTag, frameChunk, 4) != 0) {
            chunkOffset += *(unsigned int*)((char*)frameChunk + 4);
        } else
            break;
    }
    if (!frameChunk)
        return;
    {
        void* allocation = AsWPuppet(self)->keyRange;
        if (allocation) {
            __dl__FPv(allocation);
            AsWPuppet(self)->keyRange = 0;
        }
        allocation = AsWPuppet(self)->keyRecords;
        if (allocation) {
            __destroy_new_array(allocation, PuppetKeyRecord_dtor);
            AsWPuppet(self)->keyRecords = 0;
        }
        if (!*(unsigned short*)&((PuppetFrameChunk*)frameChunk)->rangeEnd)
            return;
        allocation = __nw__FUl(4);
        if (allocation) {
            *(unsigned short*)allocation = 0;
            *(unsigned short*)((char*)allocation + 2) = 0;
        }
        AsWPuppet(self)->keyRange = allocation;
        *(unsigned short*)((char*)allocation + 2) = *(unsigned short*)&((PuppetFrameChunk*)frameChunk)->rangeEnd;
        *(unsigned short*)AsWPuppet(self)->keyRange = *(unsigned short*)&((PuppetFrameChunk*)frameChunk)->rangeStart;
        int keyCount = *(unsigned short*)&((PuppetFrameChunk*)frameChunk)->recordCount;
        AsWPuppet(self)->keyRecordCount = keyCount;
        void* newKeys = __nwa__FUl(keyCount * 12 + 0x10);
        if (newKeys)
            newKeys = __construct_new_array(newKeys, PuppetKeyRecord_Clear, PuppetKeyRecord_dtor, 12, keyCount);
        AsWPuppet(self)->keyRecords = newKeys;
        i = 0;
        unsigned int keyByteOffset = 0;
        void* frameRecord = (char*)frameChunk + *(unsigned int*)&((PuppetFrameChunk*)frameChunk)->recordsOffset;
        for (; i < AsWPuppet(self)->keyRecordCount; ++i) {
            void* keyRecord = (char*)AsWPuppet(self)->keyRecords + keyByteOffset;
            *(unsigned int*)((char*)keyRecord + 4) = *(unsigned char*)&((PuppetFrameRecord*)frameRecord)->flags;
            *(unsigned int*)keyRecord = *(unsigned int*)((char*)frameRecord + 8);
            void* keyText = __nwa__FUl(*(unsigned short*)&((PuppetFrameRecord*)frameRecord)->textLength + 1);
            *(void**)((char*)keyRecord + 8) = keyText;
            Memory_Clear(keyText, *(unsigned short*)&((PuppetFrameRecord*)frameRecord)->textLength + 1);
            Memory_Copy(*(void**)((char*)keyRecord + 8), (char*)frameChunk + *(unsigned int*)frameRecord,
                        *(unsigned short*)&((PuppetFrameRecord*)frameRecord)->textLength);
            frameRecord = (char*)frameRecord + sizeof(PuppetFrameRecord);
            keyByteOffset += 0xC;
        }
    }
done:;
}
#pragma use_lmw_stmw reset
extern float kFloat_30_805DD7C8;
extern unsigned int g_puppetTransformBufferCapacity;
extern void* g_puppetMatrixScratch;
extern int gtl_ResourceFile_GetChunkElementCount(void*, void*, int);
extern void* gtl_ResourceFile_GetChunkElement(void*, void*, int, int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const char*, const char*);
extern const char s_puppetResourceNameRefCounts[];
extern void String_ReserveCapacity(void*, unsigned int);
extern void WPuppet_LoadModelFromArchive(void*, void*, void*);
// Return the shorter string length by reference.
static inline const unsigned int& StrMin(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Return the shared name tree's root-word address, which doubles as its end sentinel.
static inline void* mapEnd56D() {
    return (void*)(s_puppetResourceNameRefCounts + 4);
}
// Compare an inline or heap string key with a supplied byte span.
static inline int StrCmp56D(const void* s, unsigned int len, const void* data) {
    unsigned int sz;
    const char* d;
    unsigned int sz2;
    if (!(*(unsigned int*)s >> 31))
        sz = *(unsigned char*)s & 0x7F;
    else
        sz = ((const MWStringStorage*)s)->length;
    if (!(*(unsigned int*)s >> 31)) {
        d = (const char*)s + 1;
        sz2 = *(unsigned char*)s & 0x7F;
    } else {
        d = ((const MWStringStorage*)s)->data;
        sz2 = ((const MWStringStorage*)s)->length;
    }
    unsigned int n = StrMin(sz, sz2);
    unsigned int m = StrMin(len, n);
    int cmp = memcmp((void*)d, (void*)data, (int)m);
    if (!cmp) {
        if (n < len)
            cmp = -1;
        else if (n == len)
            cmp = 0;
        else
            cmp = 1;
    }
    return cmp;
}
struct VRef56D {
    virtual void v0();
    virtual void v1();
    virtual void release(int);
};
struct RefObj56D {
    int count;
    VRef56D obj;
};
#pragma opt_loop_invariants off
// Count nondegenerate triangles, replace the model resource, and prepare its transform buffers.
void WPuppet_LoadModelFromArchive(void* self, void* archive, void* name) {
    // Phase 1: count sliding index triples whose three indices differ; this is
    // a strip-style walk (advance by one index), not independent triangle triples.
    unsigned int layerCount = (unsigned int)(long)gtl_ResourceFile_GetChunkElementCount(
        archive, *(void**)((char*)archive + PUPPET_OFFSET(PuppetArchiveView, chunkTable)), kChunkModelLayers);
    void* indexData = gtl_ResourceFile_GetChunkElement(
        archive, *(void**)((char*)archive + PUPPET_OFFSET(PuppetArchiveView, chunkTable)), kChunkIndexData, 0);
    AsWPuppet(self)->triangleCount = 0;
    for (unsigned int i = 0; i < layerCount; ++i) {
        void* modelLayer = gtl_ResourceFile_GetChunkElement(
            archive, *(void**)((char*)archive + PUPPET_OFFSET(PuppetArchiveView, chunkTable)), kChunkModelLayers, i);
        for (int j = 0; j < *(unsigned short*)modelLayer; ++j) {
            void* meshGroup = gtl_ResourceFile_GetChunkElement(
                archive, *(void**)((char*)archive + PUPPET_OFFSET(PuppetArchiveView, chunkTable)), kChunkMeshGroups,
                *(unsigned short*)((char*)modelLayer + 2) + j);
            for (unsigned int k = 0; k < *(unsigned int*)((char*)meshGroup + 4); ++k) {
                void* primitive = gtl_ResourceFile_GetChunkElement(
                    archive, *(void**)((char*)archive + PUPPET_OFFSET(PuppetArchiveView, chunkTable)), kChunkPrimitives,
                    *(unsigned int*)meshGroup + k);
                unsigned int n = *(unsigned short*)&((PuppetPrimitiveView*)primitive)->indexHeader;
                void* indexHeader = gtl_ResourceFile_GetChunkElement(
                    archive, *(void**)((char*)archive + PUPPET_OFFSET(PuppetArchiveView, chunkTable)),
                    kChunkIndexHeaders, n);
                unsigned int indexByteOffset = *(unsigned int*)((char*)indexHeader + 4);
                unsigned short* indexCursor = (unsigned short*)((char*)indexData + indexByteOffset);
                for (int m = 0; m < *(unsigned short*)&((PuppetPrimitiveView*)primitive)->triangleCount; ++m) {
                    unsigned short secondVertex = indexCursor[1];
                    unsigned short firstVertex = indexCursor[0];
                    if (secondVertex != firstVertex) {
                        unsigned short thirdVertex = indexCursor[2];
                        if (thirdVertex != firstVertex && secondVertex != thirdVertex)
                            ++AsWPuppet(self)->triangleCount;
                    }
                    indexCursor++;
                }
            }
        }
    }

    // Phase 2: release the old retained resource, then construct its replacement.
    unsigned int query[7] = {0, 0, 0, 0, 0, 0, 0};
    void* old = *(void**)((char*)self + PUPPET_OFFSET(WPuppet, modelResource));
    if (old) {
        if (--*(unsigned int*)old == 0) {
            if (old)
                ((void (*)(void*, short))(*(void***)((char*)old + 4))[2])(old, 1);
        }
        AsWPuppet(self)->modelResource = 0;
    }
    gtl::IGenericModelingResource::CreateFromMemory(archive, (int)(long)name, &AsWPuppet(self)->modelResource, query);
    // Phase 3: query name registration before replacing bone-matrix storage.
    // Registered names share that storage; unregistered names allocate it locally.
    int valid;
    if (!AsWPuppet(self)->hasResourceName) {
        valid = 0;
    } else {
        char* begin = AsWPuppet(self)->resourceName;
        unsigned int str[3] = {0, 0, 0};
        volatile int stackByte;
        unsigned int length = (unsigned int)strlen(begin);
        String_ReserveCapacity(str, length);
        char local = (char)stackByte;
        ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(str, 0, 0, begin,
                                                                                          begin + length, &local);
        void* node = *(void**)mapEnd56D();
        void* cand = mapEnd56D();
        unsigned int isLong = str[0] >> 31;
        unsigned int qbyte = *(unsigned char*)str & 0x7F;
        unsigned int qsize = str[1];
        void* qptr = (void*)str[2];
        int isShort = (isLong == 0);
        bool heap = (isShort == 0);
        while (node) {
            unsigned int qlen;
            const void* qdata;
            if (!heap) {
                qlen = qbyte;
                qdata = (char*)str + 1;
            } else {
                qdata = qptr;
                qlen = qsize;
            }
            int cmp = StrCmp56D((char*)node + PUPPET_OFFSET(ResourceStringNode, stringFlags), qlen, qdata);
            if (cmp < 0)
                node = *(void**)((char*)node + 4);
            else {
                cand = node;
                node = *(void**)node;
            }
        }
        if (cand != mapEnd56D()) {
            unsigned int nl;
            const void* nd;
            if (!(*(unsigned int*)((char*)cand + PUPPET_OFFSET(ResourceStringNode, stringFlags)) >> 31)) {
                nd = ((PuppetNameNodeView*)cand)->key.small.text;
                nl = *(unsigned char*)((char*)cand + PUPPET_OFFSET(ResourceStringNode, stringFlags)) & 0x7F;
            } else {
                nd = *(void**)((char*)cand + PUPPET_OFFSET(ResourceStringNode, heapData));
                nl = *(unsigned int*)((char*)cand + PUPPET_OFFSET(ResourceStringNode, heapLength));
            }
            if (StrCmp56D(str, nl, nd) < 0)
                cand = mapEnd56D();
        }
        if (isLong)
            appFreeUni__FPv((void*)str[2]);
        valid = mapEnd56D() != cand;
    }
    if (valid) {
        valid = 0;
    } else {
        void* oldBoneMatrices = AsWPuppet(self)->boneMatrices;
        if (oldBoneMatrices) {
            __dla__FPv(oldBoneMatrices);
            AsWPuppet(self)->boneMatrices = 0;
        }
        valid = 1;
    }
    if (valid) {
        void* effectiveModel = *(void**)((char*)self + PUPPET_OFFSET(WPuppet, modelResource));
        if (!effectiveModel && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
            void* parent = AsWPuppet(self)->childCount == 0 ? 0 : *(void**)((char*)AsWPuppet(self)->childHead + 8);
            if (parent)
                effectiveModel = AsWPuppet(parent)->modelResource;
        }
        int count = effectiveModel
                        ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(effectiveModel, kChunkBoneTransforms)
                        : 0;
        if (count) {
            void* out = __nwa__FUl(count * 0x30);
            AsWPuppet(self)->boneMatrices = out;
            void* src = *(void**)((char*)self + PUPPET_OFFSET(WPuppet, modelResource));
            if (!src && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
                void* parent = AsWPuppet(self)->childCount == 0 ? 0 : *(void**)((char*)AsWPuppet(self)->childHead + 8);
                if (parent)
                    src = AsWPuppet(parent)->modelResource;
            }
            if (src)
                ((gtl::IGenericModelingResource*)src)
                    ->EvaluateTransformAnimation(AsWPuppet(self)->boneMatrices, (void*)(long)count);
        }
    }
    WPuppet_AllocateSkinMatrices(self);
    void* effectiveModel = *(void**)((char*)self + PUPPET_OFFSET(WPuppet, modelResource));
    if (!effectiveModel && AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* parent = AsWPuppet(self)->childCount == 0 ? 0 : *(void**)((char*)AsWPuppet(self)->childHead + 8);
        if (parent)
            effectiveModel = AsWPuppet(parent)->modelResource;
    }
    int n =
        effectiveModel ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(effectiveModel, kChunkSkinTransforms) : 0;
    if (g_puppetTransformBufferCapacity < (unsigned int)n) {
        if (g_puppetMatrixScratch) {
            __dla__FPv(g_puppetMatrixScratch);
            g_puppetMatrixScratch = 0;
        }
        g_puppetTransformBufferCapacity = 0;
        void* skinScratch = __nwa__FUl(n << 6);
        g_puppetMatrixScratch = skinScratch;
        if (skinScratch)
            g_puppetTransformBufferCapacity = n;
    }
}
struct D2A4 {
    void* p;
    volatile int n;
};
// Clear a vector count, release its allocation, and optionally delete its wrapper.
void* dtor_8005D2A4(void* self, int deleting) {
    if (self) {
        D2A4* s = (D2A4*)self;
        void* p = s->p;
        if (p) {
            int t = s->n;
            s->n = t - t;
            appFreeUni__FPv(p);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
// Release and clear the resource referenced by a scoped handle, then optionally delete the handle.
void* ResourceHandle_dtor(void* self, int deleting) {
    if (self) {
        void* p = **(void***)self;
        if (p) {
            if (p) {
                Resource_Release(p);
                **(void***)self = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
// Replace spaces with underscores and search the bone hierarchy for a matching name.
int WPuppet_FindBoneByName(void* self, void* boneName) {
    char buffer[0x100];
    strncpy(buffer, (const char*)boneName, 0x100);
    for (signed char* p = (signed char*)buffer; *p != 0; ++p) {
        if (*p == ' ')
            *p = '_';
    }
    void* q = AsWPuppet(self)->boneRoot;
    if (q)
        return WPuppetBone_FindBoneByTail(q, buffer);
    return 0;
}
// Release a heap-backed string and optionally delete its storage wrapper.
void* StringObj_dtor(void* self, int deleting) {
    if (self) {
        if (self) {
            if ((*(unsigned int*)self >> 31) != 0)
                appFreeUni__FPv(*(void**)((char*)self + 8));
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
// Set a bone-controller vector override for the supplied key.
int WPuppet_SetBoneControllerVector(void* self, int boneIndex, float* vector) {
    int a1 = boneIndex;
    void* p = AsWPuppet(self)->boneController;
    if (!p)
        return 0;
    struct {
        float a, b, c;
        volatile int d;
        float e;
    } s;
    s.d = 3;
    s.e = kFloat_0_puppet;
    s.a = vector[0];
    s.b = vector[1];
    s.c = vector[2];
    s.d = 0;
    return BoneControllerMap_SetVectorEntry(p, &a1, &s.a);
}
#pragma use_lmw_stmw on
// Walk size-prefixed archive chunks and return the first matching four-byte tag.
void* Chunk_FindByTag(unsigned char* data, unsigned int dataSize, void* tag) {
    unsigned char* p;
    unsigned int offset = 0;
    while (offset < dataSize) {
        p = data + offset;
        if (!memcmp(tag, p, 4))
            return p;
        offset += *(unsigned int*)(p + 4);
    }
    return 0;
}
#pragma use_lmw_stmw reset
// Clear a vector count, release its allocation, and optionally delete the vector.
void* WPuppetMaterialVector_dtor(void* self, int deleting) {
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    void* p = *(void**)self;
                    if (p) {
                        int t = *(volatile int*)((char*)self + 4);
                        *(volatile int*)((char*)self + 4) = t - t;
                        appFreeUni__FPv(p);
                    }
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
// Return the model-layer count from this puppet or its first child.
// self is required. A local resource wins; otherwise consult ONLY the first
// child, not siblings or deeper descendants. childCount is a scalar count,
// childHead is a WPuppetListNode with borrowed puppet at +8. A nonempty list
// must have a valid head; its child's resource must satisfy the resource API.
// No child (or a null first-child value) returns 0. No mutation/ownership change.
int WPuppet_GetModelCount(void* self) {
    void* modelResource = AsWPuppet(self)->modelResource;
    int modelCount = 0;
    if (modelResource) {
        modelCount = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(modelResource, kChunkModelLayers);
    } else {
        unsigned int childCount = AsWPuppet(self)->childCount;
        void* firstChild;
        if (childCount == 0) {
            firstChild = 0;
        } else
            firstChild = AsWPuppet(self)->childHead, firstChild = ((WPuppetListNode*)firstChild)->value;
        if (firstChild) {
            void* childModelResource = AsWPuppet(firstChild)->modelResource;
            modelCount = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(childModelResource, kChunkModelLayers);
        }
    }
    return modelCount;
}
// Grow the shared skin-transform scratch buffer to hold the requested matrix count.
int WPuppet_EnsureTransformBufferCapacity(unsigned int requiredMatrices) {
    unsigned int g0 = g_puppetTransformBufferCapacity;
    if (g0 >= requiredMatrices) {
        return 1;
    }
    void* g1 = g_puppetMatrixScratch;
    if (g1) {
        __dla__FPv(g1);
        g_puppetMatrixScratch = 0;
    }
    g_puppetTransformBufferCapacity = 0;
    void* p = __nwa__FUl(requiredMatrices << 6);
    g_puppetMatrixScratch = p;
    if (!p)
        return 0;
    g_puppetTransformBufferCapacity = requiredMatrices;
    return 1;
}

// Find a named animation-range record in the pointer vector.
void* WPuppet_FindAnimationRangeByName(void* self, void* rangeName) {
    void* base = AsWPuppet(self)->animationRanges;
    if (!base)
        return 0;
    void* arr = *(void**)base;
    unsigned int count = *(unsigned int*)((char*)base + 4);
    void* end = (char*)arr + (count << 2);
    void* e = 0;
    for (void* cur = arr; cur != end; cur = (char*)cur + 4) {
        e = *(void**)cur;
        if (e && !strcmp(*(const char**)&((PuppetMotionRangeView*)e)->name, (const char*)rangeName))
            return e;
    }
    return 0;
}

// Return this puppet's or its first child's material metadata and optionally its material count.
void* WPuppet_GetMaterialMetadata(void* self, void* outCount) {
    if (outCount)
        *(void**)outCount = 0;
    void* r31 = AsWPuppet(self)->materialMetadata;
    void* r3;
    if (!r31) {
        void* r5;
        if (AsWPuppet(self)->childCount == 0) {
            r5 = 0;
        } else {
            void* t = AsWPuppet(self)->childHead;
            r5 = *(void**)((char*)t + 8);
        }
        if (r5 == 0)
            return 0;
        r3 = AsWPuppet(r5)->modelResource;
        r31 = AsWPuppet(r5)->materialMetadata;
    } else {
        r3 = AsWPuppet(self)->modelResource;
    }
    if (outCount) {
        void* res = (void*)(long)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(r3, kChunkMaterialMetadata);
        *(void**)outCount = res;
    }
    return r31;
}

#pragma scheduling off
// Copy the shared context pointer to this puppet and every descendant.
// Borrow context (nullable) without retaining/releasing it. Requires a live,
// acyclic hierarchy, valid nonnull child values, and no concurrent list mutation.
// List schema: each node is {previous +0,next +4,value +8}; end is the embedded
// sentinel at &parent.childTail, never null. Empty head equals that sentinel.
// Traversal is preorder: self, children, grandchildren, great-grandchildren;
// at the fourth child level recurse with exactly the same context, then advance
// that level's saved node->next. Every write is hierarchyContext; list links and
// ownership stay unchanged. The expanded levels preserve original stack/call
// shape; each depth has its own cursor, end sentinel, and saved node for advance.
void WPuppet_SetHierarchyContextRecursive(void* self, void* context) {
    void* borrowedContext = context;
    void* childCursor;
    void* grandchildCursor;
    void* greatGrandchildCursor;
    void* recursiveChildCursor;
    char* recursiveChildEnd;
    char* greatGrandchildEnd;
    char* grandchildEnd;
    char* childEnd;
    childEnd = (char*)&AsWPuppet(self)->childTail;
    childCursor = AsWPuppet(self)->childHead;
    AsWPuppet(self)->hierarchyContext = borrowedContext;
    while (childCursor != (void*)childEnd) {
        void* childNode = childCursor;
        void* child = ((WPuppetListNode*)childNode)->value;
        AsWPuppet(child)->hierarchyContext = borrowedContext;
        grandchildEnd = (char*)&AsWPuppet(child)->childTail;
        grandchildCursor = AsWPuppet(child)->childHead;
        while (grandchildCursor != (void*)grandchildEnd) {
            void* grandchildNode = grandchildCursor;
            void* grandchild = ((WPuppetListNode*)grandchildNode)->value;
            AsWPuppet(grandchild)->hierarchyContext = borrowedContext;
            greatGrandchildEnd = (char*)&AsWPuppet(grandchild)->childTail;
            greatGrandchildCursor = AsWPuppet(grandchild)->childHead;
            while (greatGrandchildCursor != (void*)greatGrandchildEnd) {
                void* greatGrandchildNode = greatGrandchildCursor;
                void* greatGrandchild = ((WPuppetListNode*)greatGrandchildNode)->value;
                AsWPuppet(greatGrandchild)->hierarchyContext = borrowedContext;
                recursiveChildEnd = (char*)&AsWPuppet(greatGrandchild)->childTail;
                recursiveChildCursor = AsWPuppet(greatGrandchild)->childHead;
                while (recursiveChildCursor != (void*)recursiveChildEnd) {
                    void* recursiveChildNode = recursiveChildCursor;
                    void* recursiveChild = ((WPuppetListNode*)recursiveChildNode)->value;
                    WPuppet_SetHierarchyContextRecursive(recursiveChild, borrowedContext);
                    recursiveChildCursor = ((WPuppetListNode*)recursiveChildNode)->next;
                }
                greatGrandchildCursor = ((WPuppetListNode*)greatGrandchildNode)->next;
            }
            grandchildCursor = ((WPuppetListNode*)grandchildNode)->next;
        }
        childCursor = ((WPuppetListNode*)childNode)->next;
    }
}
#pragma scheduling on
// Find the time-containing range and set its animation index and local time in 30-fps frames.
int WPuppet_SetAnimationTimeFromRanges(void* self, int animationTrack, float f1) {
    if (animationTrack != 0)
        return 0;
    void* r5 = AsWPuppet(self)->animation;
    if (!r5)
        return 0;
    void* base = AsWPuppet(self)->animationRanges;
    if (!base)
        return 0;
    void** arr = *(void***)base;
    void** end = arr + *(unsigned int*)((char*)base + 4);
    for (void** p = arr; p != end; ++p) {
        void* e = *p;
        if (*(float*)e <= f1) {
            if (f1 <= *(float*)((char*)e + 4)) {
                f1 = f1 - *(float*)e;
                unsigned short u = *(unsigned short*)&((PuppetMotionRangeView*)e)->animationIndex;
                ((CPetAnimation*)r5)->SetModeAndAmount(u, kFloat_30_805DD7C8 * f1);
                break;
            }
        }
    }
    return 0;
}

// Find the contiguous key-record index range intersecting the requested key interval; wildcard keys remain included.
int WPuppet_FindKeyRange(void* self, int firstKey, int lastKey, int* firstIndex, int* lastIndex) {
    int keyByteOffset;
    int count = AsWPuppet(self)->keyRecordCount;
    int recordIndex = 0;
    keyByteOffset = 0;
    for (; recordIndex < count; ++recordIndex) {
        int* keyWords = *(int**)&AsWPuppet(self)->keyRecords;
        int key = *(int*)((char*)keyWords + keyByteOffset);
        if ((unsigned int)key != 0xFFFFFFFFu) {
            if (firstKey <= key)
                break;
        }
        keyByteOffset += 0xc;
    }
    if (recordIndex < count) {
        int* keyWords = *(int**)&AsWPuppet(self)->keyRecords;
        int firstMatchingKey = *(int*)((char*)keyWords + recordIndex * 0xc);
        if (firstMatchingKey <= lastKey) {
            *firstIndex = recordIndex;
            *lastIndex = recordIndex;
            while (*lastIndex + 1 < AsWPuppet(self)->keyRecordCount) {
                int currentIndex = *lastIndex;
                int* keyWords = *(int**)&AsWPuppet(self)->keyRecords;
                char* currentRecord = (char*)keyWords + currentIndex * 0xc;
                int nextKey = *(int*)(currentRecord + 0xc);
                if ((unsigned int)nextKey != 0xFFFFFFFFu) {
                    if (lastKey < nextKey)
                        break;
                }
                *lastIndex = *lastIndex + 1;
            }
            return 1;
        }
    }
    return 0;
}

// Return the local default bone or recursively inherit the first child's default bone.
int WPuppet_ResolveDefaultBone(void* self) {
    void* p = AsWPuppet(self)->defaultBone;
    if (!p) {
        void* n = AsWPuppet(self)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
        if (n) {
            void* q = AsWPuppet(n)->defaultBone;
            if (q)
                return (int)q;
            n = AsWPuppet(n)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(n)->childHead)->value;
            if (n) {
                q = AsWPuppet(n)->defaultBone;
                if (q)
                    return (int)q;
                n = AsWPuppet(n)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(n)->childHead)->value;
                if (n) {
                    q = AsWPuppet(n)->defaultBone;
                    if (q)
                        return (int)q;
                    q = AsWPuppet(n)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(n)->childHead)->value;
                    if (q)
                        return WPuppet_ResolveDefaultBone(q);
                    else
                        return 0;
                } else
                    return 0;
            } else
                return 0;
        } else
            return 0;
    }
    return (int)p;
}

// Resolve a texture key and assign the loaded bitmap's paired handles throughout the model hierarchy.
void WPuppet_ApplyNamedResourcePairRecursive(void* self, int bitmapResource, int textureKey, int unused) {
    void* base;
    if (bitmapResource > 0) {
        void* node = (char*)&AsWPuppet(self)->childTail;
        void* end = AsWPuppet(self)->childHead;
        while (end != node) {
            WPuppet_ApplyNamedResourcePairRecursive(((WPuppetListNode*)end)->value, bitmapResource, textureKey, unused);
            end = ((WPuppetListNode*)end)->next;
        }
        if (AsWPuppet(self)->modelResource) {
            unsigned char byte;
            if (List_GetByteByKey(AsWPuppet(self)->nameIndexMap, textureKey, &byte)) {
                void* manager = *(void**)g_resourceManager;
                base = manager;
                end = (void*)(long)(((int (*)(void*, int))(*(void**)((char*)*(void**)base + 0x70)))(base,
                                                                                                    bitmapResource));
                base = (void*)(long)(((int (*)(void*, int))(*(void**)((char*)*(void**)base + 0x74)))(base,
                                                                                                     bitmapResource));
                ((void (*)(void*, int, int, int))SetResource__Q23gtl24IGenericModelingResourceFiiP12GtlRefHeader)(
                    AsWPuppet(self)->modelResource, 3, byte, (int)(long)end);
                ((void (*)(void*, int, int, int))SetResource__Q23gtl24IGenericModelingResourceFiiP12GtlRefHeader)(
                    AsWPuppet(self)->modelResource, 5, byte, (int)(long)base);
            }
        }
    }
}

char* strcpy(char*, const char*);
void WPuppet_LoadFromChunks(void*, void*, void*, int, void*, int);
#pragma use_lmw_stmw on
// Remember the supplied resource name and directory, then load model chunks with the requested flags.
int WPuppet_LoadNamedResourceFromChunks(void* self, const char* resourceName, void* data, void* dataSize, int loadMode,
                                        int loadFlags) {
    char path[0x200];
    char tail[0x200];
    strcpy(path, resourceName);
    char* slash = strrchr(path, '/');
    slash = slash ? slash : path;
    strcpy(slash, puppetPreloadStrings.emptyPath);
    char* s = strrchr((char*)resourceName, '/');
    char* tailSrc;
    if (s)
        tailSrc = s + 1;
    else
        tailSrc = (char*)resourceName;
    strcpy(tail, tailSrc);
    strcpy(AsWPuppet(self)->resourceName, tail);
    int exists = (loadMode != 0);
    void* p = path;
    p = (path[0] == 0) ? 0 : p;
    WPuppet_LoadFromChunks(self, data, dataSize, exists, p, loadFlags);
    if ((loadFlags & 8) && !AsWPuppet(self)->modelResource)
        return 1;
    return 0;
}
#pragma use_lmw_stmw reset

// Unlink and free cached-bone list nodes, updating the count, and optionally delete the list; bones are borrowed.
void* WPuppetCachedBoneList_dtor(MSLPointerList* self, int deleting) {
    return ::std::__list_deleting_dtor(self, deleting);
}

// Resolve a named or default bone and return its flags byte.
int WPuppet_GetBoneFlags(void* self, int boneName) {
    void* p;
    if (boneName != 0)
        p = WPuppet_FindOrCacheBone(self, (void*)(long)boneName);
    else {
        void* r4 = AsWPuppet(self)->defaultBone;
        if (!r4) {
            void* c4 = AsWPuppet(self)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
            if (c4) {
                p = AsWPuppet(c4)->defaultBone;
                if (!p) {
                    void* c3 = AsWPuppet(c4)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(c4)->childHead)->value;
                    if (c3) {
                        p = AsWPuppet(c3)->defaultBone;
                        if (!p) {
                            void* c2 = AsWPuppet(c3)->childCount == 0
                                           ? 0
                                           : ((WPuppetListNode*)AsWPuppet(c3)->childHead)->value;
                            if (c2) {
                                p = AsWPuppet(c2)->defaultBone;
                                if (!p) {
                                    void* c1 = AsWPuppet(c2)->childCount == 0
                                                   ? 0
                                                   : ((WPuppetListNode*)AsWPuppet(c2)->childHead)->value;
                                    if (c1) {
                                        p = (void*)WPuppet_ResolveDefaultBone(c1);
                                    } else {
                                        p = 0;
                                    }
                                }
                            } else {
                                p = 0;
                            }
                        }
                    } else {
                        p = 0;
                    }
                }
            } else {
                p = 0;
            }
        } else {
            p = r4;
        }
    }
    return *(unsigned char*)&((PuppetBoneView*)p)->flags;
}

// Apply externally supplied skin transforms when animation is disabled, then copy results into bones.
// matrixCount is a count of 64-byte matrices encoded in a pointer-typed ABI word.
int WPuppet_OverrideSkinMatrices(void* self, void* skinSource, void* matrixCount) {

    if ((AsWPuppet(self)->animationDisabled) == 0)
        return 1;
    if (!skinSource || !matrixCount) {
        AsWPuppet(self)->animationDisabled = 0;
        return 1;
    }
    void* r31 = AsWPuppet(self)->skinMatrices;
    if (r31 == 0)
        return 0;

    void* r5 = AsWPuppet(self)->modelResource;
    if (0 == r5) {
        if (AsWPuppet(self)->loadFlags == kContainerLoadMode) {
            void* r3;
            if (AsWPuppet(self)->childCount == 0) {
                r3 = 0;
            } else {
                r3 = AsWPuppet(self)->childHead;
                r3 = ((WPuppetListNode*)r3)->value;
            }
            if (0 != r3)
                r5 = AsWPuppet(r3)->modelResource;
        }
    }
    void* r3result;
    if (0 != r5) {
        r3result = (void*)(long)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(r5, kChunkSkinTransforms);
    } else {
        r3result = 0;
    }
    // Require the skin-transform chunk count to agree with the allocated matrix count.
    if ((unsigned int)matrixCount != (unsigned int)r3result) {
        return 0;
    }
    // r30
    int idx = (unsigned int)matrixCount;
    __typeof__(idx) idxCopy = idx;
    __typeof__(idxCopy) idxCopyCopy = idxCopy;
    __typeof__(idxCopyCopy) idxCopyCopyCopy = idxCopyCopy;
    __typeof__(idxCopyCopyCopy) idxCopyCopyCopyCopy = idxCopyCopyCopy;
    __typeof__(idxCopyCopyCopyCopy) idxCopyCopyCopyCopyCopy = idxCopyCopyCopyCopy;
    __typeof__(idxCopyCopyCopyCopyCopy) idxCopyCopyCopyCopyCopyCopy = idxCopyCopyCopyCopyCopy;
    __typeof__(idxCopyCopyCopyCopyCopyCopy) idxCopyCopyCopyCopyCopyCopyCopy = idxCopyCopyCopyCopyCopyCopy;
    Memory_Copy(r31, skinSource, idxCopyCopyCopyCopyCopyCopyCopy << 6);
    void* p = AsWPuppet(self)->skinMatrices;
    if (p != 0) {
        WPuppet_CopySkinMatricesToBones(self, p);
    }
    return 1;
}

// Parse three quoted tokens: animation name, integer sample amount, layer index.
// commandCursor borrows a writable char* slot; *commandCursor must be nonnull
// NUL-terminated text. Each token must fit 63 characters BEFORE parsing: the
// length checks follow an unbounded copy. The cursor is committed after EVERY
// token, even on a later failure; no rollback. Null cursor/source returns 0
// without consuming text; empty/invalid tokens return 0 after consuming them.
// String_ParseQuotedToken (club_trail_and_ring_effects.cpp) accepts an unterminated final quote at NUL;
// decimal syntax follows String_ParseDecimalInt, with no validation here.
// Accepted sample integer is 0..INT_MAX; it becomes float without dividing by 30.
// Its domain is animation key-time ticks, not MOTI seconds:
// GetValueByName (pet_animation.cpp) reads the scene chunk's float at +8; only value>sample
// is evaluated. EvaluateTextureTransformMatrices (gtl_texture_animation_matrices.cpp) compares that sample
// directly with DK key float timestamps. A seconds-per-tick rate for independently
// authored material resources is not established; preserve the resource's tick
// scale. layer is zero-based, < kChunkMaterialTransforms count in layerResource
// (checked downstream), and must also fit the source animation's matrix array.
// 0 can mean valid-but-not-yet-applicable as well as failure; nonzero is the
// controller map insertion result. A failed apply may still cache layerResource.
int WPuppet_ParseMaterialLayerAnimationCommand(void* self, void* commandCursor, void* animationSource) {
    char buf3[0x40];
    if (commandCursor == 0 || 0 == animationSource)
        return 0;

    char buf1[0x40];
    *(void**)commandCursor = (void*)(long)String_ParseQuotedToken(*(void**)commandCursor, buf1);
    int l1 = strlen(buf1);
    if (l1 <= 0 || (unsigned int)l1 >= 0x40)
        return 0;
    char buf2[0x40];
    *(void**)commandCursor = (void*)(long)String_ParseQuotedToken(*(void**)commandCursor, buf2);
    int l2 = strlen(buf2);
    if (0 >= l2 || (unsigned int)l2 >= 0x40)
        return 0;
    unsigned int h1 = String_ParseDecimalInt(buf2);
    *(void**)commandCursor = (void*)(long)String_ParseQuotedToken(*(void**)commandCursor, buf3);
    int l3 = strlen(buf3);
    if (l3 <= 0 || (unsigned int)l3 >= 0x40)
        return 0;
    // Identity aliases, not five conversions or samples. Keeping these five
    // local declarations also preserves CodeWarrior's generated StringCleanup
    // destructor symbol IDs later in this unit (the acceptance gate checks ABI).
    unsigned int sampleBitsAlias1 = h1;
    unsigned int sampleBitsAlias2 = sampleBitsAlias1;
    unsigned int sampleBitsAlias3 = sampleBitsAlias2;
    unsigned int sampleBitsAlias4 = sampleBitsAlias3;
    unsigned int sampleBitsAlias5 = sampleBitsAlias4;
    unsigned int h2 = String_ParseDecimalInt(buf3);

    float frame = (float)(int)sampleBitsAlias5;
    return WPuppet_ApplyNamedMaterialLayerAnimation(self, animationSource, buf1, h2, frame);
}

void WPuppet_GetPivot(void* result, void* node, int path);
void* WPuppet_FindOrCacheBone(void* arg0, void* arg1);
int WPuppet_ResolveDefaultBone(void* arg0);
// Resolve a named or default bone and copy the translation from its initial matrix.
// result is required writable float[3] (12 bytes); node is a live puppet.
// path==0 follows defaultBone down FIRST children only (expanded four levels,
// then ResolveDefaultBone); nonzero path is the Wii address of a borrowed bone
// NUL-terminated name (at most 255 characters), passed to FindOrCacheBone's
// fixed 256-byte normalization buffer. Named lookup can update the puppet's cache.
// Required on BOTH paths: lookup succeeds and returns a valid PuppetBoneView.
// A null result is dereferenced unconditionally; there is no zero-vector/no-hit
// fallback and no failure status. Successful output copies initialMatrix[9..11]
// bit-for-bit, in the stored initial bone-transform space, with no world transform.
// No bone ownership transfers; result must not overwrite live lookup structures.
void WPuppet_GetPivot(void* result, void* node, int path) {
    void* found;
    if (path != 0)
        found = WPuppet_FindOrCacheBone(node, (void*)(long)path);
    else {
        void* r4 = AsWPuppet(node)->defaultBone;
        if (!r4) {
            void* c4 = AsWPuppet(node)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(node)->childHead)->value;
            if (c4) {
                found = AsWPuppet(c4)->defaultBone;
                if (!found) {
                    void* c3 = AsWPuppet(c4)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(c4)->childHead)->value;
                    if (c3) {
                        found = AsWPuppet(c3)->defaultBone;
                        if (!found) {
                            void* c2 = AsWPuppet(c3)->childCount == 0
                                           ? 0
                                           : ((WPuppetListNode*)AsWPuppet(c3)->childHead)->value;
                            if (c2) {
                                found = AsWPuppet(c2)->defaultBone;
                                if (!found) {
                                    void* c1 = AsWPuppet(c2)->childCount == 0
                                                   ? 0
                                                   : ((WPuppetListNode*)AsWPuppet(c2)->childHead)->value;
                                    if (c1)
                                        found = (void*)(long)WPuppet_ResolveDefaultBone(c1);
                                    else
                                        found = 0;
                                }
                            } else
                                found = 0;
                        }
                    } else
                        found = 0;
                }
            } else
                found = 0;
        } else
            found = r4;
    }
    struct Words3 {
        unsigned int w[3];
    };
    *(Words3*)result = *(Words3*)((char*)&((PuppetBoneView*)found)->initialMatrix[9]);
}

void WPuppetBone_SetRenderFlag40000(void* arg0, int arg1, int arg2);
#pragma use_lmw_stmw on
// Always visit child puppets; change the selected bone's draw mask on each puppet.
// includeDescendants additionally follows local model-parent bone relationships.
void WPuppet_SetBoneVisibilityRecursive(void* self, int includeDescendants, int hidden, const char* boneName) {
    for (void* node = AsWPuppet(self)->childHead; (char*)&AsWPuppet(self)->childTail != node;
         node = *(void**)((char*)node + 4)) {
        WPuppet_SetBoneVisibilityRecursive(*(void**)((char*)node + 8), includeDescendants, hidden, boneName);
    }
    if (!*(int*)&AsWPuppet(self)->modelLayerVisibilityWords)
        return;
    void* found;
    unsigned int r25;
    if (!boneName) {
        found = AsWPuppet(self)->defaultBone;
        if (!found) {
            void* c1 = AsWPuppet(self)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
            if (c1) {
                found = AsWPuppet(c1)->defaultBone;
                if (!found) {
                    void* c2 = AsWPuppet(c1)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(c1)->childHead)->value;
                    if (c2) {
                        found = AsWPuppet(c2)->defaultBone;
                        if (!found) {
                            void* c3 = AsWPuppet(c2)->childCount == 0
                                           ? 0
                                           : ((WPuppetListNode*)AsWPuppet(c2)->childHead)->value;
                            if (c3) {
                                found = AsWPuppet(c3)->defaultBone;
                                if (!found) {
                                    void* c4 = AsWPuppet(c3)->childCount == 0
                                                   ? 0
                                                   : ((WPuppetListNode*)AsWPuppet(c3)->childHead)->value;
                                    if (c4)
                                        found = (void*)(long)WPuppet_ResolveDefaultBone(c4);
                                    else
                                        found = 0;
                                }
                            } else
                                found = 0;
                        }
                    } else
                        found = 0;
                }
            } else
                found = 0;
        }
        r25 = *(int*)&((PuppetBoneView*)found)->index;
    } else {
        void* hit = AsWPuppet(self)->defaultBone;
        if (!hit) {
            void* c1 = AsWPuppet(self)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
            if (c1) {
                hit = AsWPuppet(c1)->defaultBone;
                if (!hit) {
                    void* c2 = AsWPuppet(c1)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(c1)->childHead)->value;
                    if (c2) {
                        hit = AsWPuppet(c2)->defaultBone;
                        if (!hit) {
                            void* c3 = AsWPuppet(c2)->childCount == 0
                                           ? 0
                                           : ((WPuppetListNode*)AsWPuppet(c2)->childHead)->value;
                            if (c3) {
                                hit = AsWPuppet(c3)->defaultBone;
                                if (!hit) {
                                    void* c4 = AsWPuppet(c3)->childCount == 0
                                                   ? 0
                                                   : ((WPuppetListNode*)AsWPuppet(c3)->childHead)->value;
                                    if (c4)
                                        hit = (void*)(long)WPuppet_ResolveDefaultBone(c4);
                                    else
                                        hit = 0;
                                }
                            } else
                                hit = 0;
                        }
                    } else
                        hit = 0;
                }
            } else
                hit = 0;
        }
        found = hit;
        if (boneName)
            found = WPuppetBone_FindBone(AsWPuppet(self)->boneRoot, boneName, 0);
        if (!found) {
            r25 = -1;
        } else {
            unsigned int idx2 = *(unsigned int*)&((PuppetBoneView*)found)->index;
            unsigned int count =
                GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource, kChunkModelLayers);
            if (idx2 >= count)
                r25 = -1;
            else
                r25 = *(int*)&((PuppetBoneView*)found)->index;
        }
    }
    if (r25 == -1)
        return;
    unsigned int flag = (hidden == 0) ? 1 : 0;
    *(int*)(*(char**)&AsWPuppet(self)->modelLayerVisibilityWords + r25 * 4) = flag;
    WPuppetBone_SetRenderFlag40000(*(char**)&AsWPuppet(self)->bones + r25 * kBoneBytes, 0, hidden);
    if (!includeDescendants)
        return;
    unsigned int i;
    int offset;
    unsigned int count2 =
        GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource, kChunkModelLayers);
    void* entries = GetChunkElement__Q23gtl20IGenericResourceFileFii(AsWPuppet(self)->modelResource,
                                                                     (const char*)kChunkModelLayers, 0);
    unsigned short idx16 = (unsigned short)r25;
    offset = 0;
    for (i = 0; i < count2; ++i) {
        if (idx16 == *(unsigned short*)&((PuppetModelLayerView*)entries)->parentBone) {
            WPuppetBone_SetRenderFlag40000(*(char**)&AsWPuppet(self)->bones + offset, 0, hidden);
            WPuppet_SetBoneVisibilityRecursive(self, 1, hidden, *(char**)&AsWPuppet(self)->bones + offset + 8);
        }
        entries = (char*)entries + kModelLayerBytes;
        offset += kBoneBytes;
    }
}
#pragma use_lmw_stmw reset

extern void Matrix_SetIdentityAffine(void*);
extern float kFloat_1_Puppet;
extern float kFloat_0_puppet;
extern float kFloat_0_5_805DD7CC;
extern void WPuppet_SetWorldMatrixRecursive(void*, void*);
extern void WPuppet_AccumulateTransformedBounds(void*, void*, void*, void*, void*, void*);
extern void WPuppet_UpdateSkinFromAnimationSource(void*, void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float Magnitude__7WVectorCFv(void*);

#pragma opt_common_subs off
#pragma opt_loop_invariants off
// Reset the model-space hierarchy before a bounds sample. Inline function splits
// changed register allocation; this scoped macro keeps the original loop lifetimes.
// The two literal booleans retain comparison operand order at the sample sites.
#define RESET_ANIMATION_BOUNDS_TRANSFORMS(self, matrix, rootSentinelFirst, deepSentinelFirst)               \
    do {                                                                                                    \
        Matrix_CopyAffine((char*)&AsWPuppet(self)->worldMatrix, matrix);                                    \
        for (void* childNode = AsWPuppet(self)->childHead;                                                  \
             (rootSentinelFirst ? (char*)&AsWPuppet(self)->childTail != childNode                           \
                                : childNode != (char*)&AsWPuppet(self)->childTail);                         \
             childNode = ((WPuppetListNode*)childNode)->next) {                                             \
            void* child = ((WPuppetListNode*)childNode)->value;                                             \
            Matrix_CopyAffine((char*)&AsWPuppet(child)->worldMatrix, matrix);                               \
            for (void* grandchildNode = AsWPuppet(child)->childHead;                                        \
                 grandchildNode != (char*)&AsWPuppet(child)->childTail;                                     \
                 grandchildNode = ((WPuppetListNode*)grandchildNode)->next) {                               \
                void* grandchild = ((WPuppetListNode*)grandchildNode)->value;                               \
                Matrix_CopyAffine((char*)&AsWPuppet(grandchild)->worldMatrix, matrix);                      \
                for (void* greatGrandchildNode = AsWPuppet(grandchild)->childHead;                          \
                     greatGrandchildNode != (char*)&AsWPuppet(grandchild)->childTail;                       \
                     greatGrandchildNode = ((WPuppetListNode*)greatGrandchildNode)->next) {                 \
                    void* greatGrandchild = ((WPuppetListNode*)greatGrandchildNode)->value;                 \
                    Matrix_CopyAffine((char*)&AsWPuppet(greatGrandchild)->worldMatrix, matrix);             \
                    for (void* deeperNode = AsWPuppet(greatGrandchild)->childHead;                          \
                         (deepSentinelFirst ? (char*)&AsWPuppet(greatGrandchild)->childTail != deeperNode   \
                                            : deeperNode != (char*)&AsWPuppet(greatGrandchild)->childTail); \
                         deeperNode = ((WPuppetListNode*)deeperNode)->next)                                 \
                        WPuppet_SetWorldMatrixRecursive(((WPuppetListNode*)deeperNode)->value, matrix);     \
                }                                                                                           \
            }                                                                                               \
        }                                                                                                   \
    } while (0)

// Update the selected skin/bone pose and add its vertices to model-space bounds.
static inline void AccumulateAnimationPoseVertices(void* self, void* owner, float* minimumCorner,
                                                   float* maximumCorner) {
    if (owner != 0 && (*(unsigned int*)&AsWPuppet(owner)->loadFlags & kAnimationSourceFlags) != 0)
        WPuppet_UpdateSkinFromAnimationSource(self, owner);
    if (AsWPuppet(self)->boneRoot != 0)
        ((void (*)(void*, float, void*, int, int))WPuppetBone_ConcatMatrixAlongChain)(
            AsWPuppet(self)->boneRoot, kFloat_1_Puppet, (char*)&AsWPuppet(self)->worldMatrix, 0, 0);
    WPuppet_AccumulateTransformedBounds(self, AsWPuppet(self)->skinMatrices, AsWPuppet(self)->boneRoot, minimumCorner,
                                        maximumCorner, 0);
}

// Merge sampled bounds into the puppet, or overwrite a borrowed caller bounds record.
// The puppet path also publishes center/radius; the caller path writes only min/max.
static inline void PublishSampledAnimationBounds(void*& self, void*& bounds, float (&minimumCorner)[3],
                                                 float (&maximumCorner)[3], float& existingComponent,
                                                 float& sampledComponent, float (&boundsCenter)[3],
                                                 float (&cornerSum)[3], float (&radiusVector)[3]) {
    // Phase 4: merge into the puppet's existing box, or overwrite the caller's box.
    if (!bounds) {
        existingComponent = AsWPuppet(self)->boundsMin.x;
        sampledComponent = minimumCorner[0];
        minimumCorner[0] = (existingComponent > sampledComponent) ? sampledComponent : existingComponent;
        sampledComponent = maximumCorner[0];
        existingComponent = AsWPuppet(self)->boundsMax.x;
        maximumCorner[0] = (existingComponent < sampledComponent) ? sampledComponent : existingComponent;
        sampledComponent = minimumCorner[1];
        existingComponent = AsWPuppet(self)->boundsMin.y;
        minimumCorner[1] = (existingComponent > sampledComponent) ? sampledComponent : existingComponent;
        sampledComponent = maximumCorner[1];
        existingComponent = AsWPuppet(self)->boundsMax.y;
        maximumCorner[1] = (existingComponent < sampledComponent) ? sampledComponent : existingComponent;
        sampledComponent = minimumCorner[2];
        existingComponent = AsWPuppet(self)->boundsMin.z;
        minimumCorner[2] = (existingComponent > sampledComponent) ? sampledComponent : existingComponent;
        existingComponent = AsWPuppet(self)->boundsMax.z;
        sampledComponent = maximumCorner[2];
        maximumCorner[2] = (existingComponent < sampledComponent) ? sampledComponent : existingComponent;
        __as__7WVectorFRC7WVector((char*)&AsWPuppet(self)->boundsMin, minimumCorner);
        __as__7WVectorFRC7WVector((char*)&AsWPuppet(self)->boundsMax, maximumCorner);
        __pl__FRC7WVectorRC7WVector(cornerSum, minimumCorner, maximumCorner);
        __ml__FRC7WVectorf(boundsCenter, cornerSum, kFloat_0_5_805DD7CC);

        __as__7WVectorFRC7WVector((char*)&AsWPuppet(self)->boundsCenter, boundsCenter);
        __mi__FRC7WVectorRC7WVector(radiusVector, (char*)&AsWPuppet(self)->boundsCenter, minimumCorner);
        AsWPuppet(self)->boundsRadius = Magnitude__7WVectorCFv(radiusVector);
    } else {
        __as__7WVectorFRC7WVector(bounds, minimumCorner);
        __as__7WVectorFRC7WVector((char*)&((PuppetBoundsView*)bounds)->maximum, maximumCorner);
    }
}

// Sample range endpoints and the default pose, accumulating transformed vertices into model or caller bounds.
void WPuppet_AccumulateAnimationBounds(void* self, int resetFlag, void* selected, int value, void* filter,
                                       void* bounds) {
    float minimumCorner[3];
    float matrix[12];
    // Phase 1: initialize an empty accumulator and select the animation owner.
    // Seed minimum/maximum corners with opposite finite sentinels (+/-99999).
    // Identity world transforms make the sampled geometry relative to the model.
    minimumCorner[0] = 99999.0f;
    float maximumCorner[3];
    minimumCorner[1] = 99999.0f;
    minimumCorner[2] = 99999.0f;
    maximumCorner[0] = -99999.0f;
    maximumCorner[1] = -99999.0f;
    maximumCorner[2] = -99999.0f;
    if (resetFlag != 0) {
        Vec3_Zero((char*)&AsWPuppet(self)->boundsMin);
        Vec3_Zero((char*)&AsWPuppet(self)->boundsMax);
    }
    if (0 != bounds) {
        float zero[3];
        zero[0] = 0.0f;
        zero[1] = 0.0f;
        zero[2] = 0.0f;
        ((void* (*)(void*, void*))__as__7WVectorFRC7WVector)(
            bounds,
            ((void* (*)(void*, void*))__as__7WVectorFRC7WVector)((char*)&((PuppetBoundsView*)bounds)->maximum, zero));
    }

    Matrix_SetIdentityAffine(matrix);
    void* owner = selected;
    void* animationRanges;
    if (0 != owner) {
        animationRanges = AsWPuppet(owner)->animationRanges;
    } else {
        owner = self;
        animationRanges = AsWPuppet(self)->animationRanges;
    }
    // Phase 2: sample each selected range's first endpoint, and its last endpoint
    // when distinct. Reset child world matrices before each skin/bone evaluation.
    if (animationRanges) {
        char* rangeCursor = *(char**)animationRanges;
        while (rangeCursor != *(char**)animationRanges + (*(unsigned int*)((char*)animationRanges + 4) << 2)) {
            PuppetMotionRangeView* rangeRecord = *(PuppetMotionRangeView**)rangeCursor;
            if (0 != rangeRecord) {
                if (0 == filter ||
                    strcmp(*(const char**)&((PuppetMotionRangeView*)rangeRecord)->name, (const char*)filter) == 0) {
                    ((int (*)(void*, int, float, int, int))WPuppet_SetAnimationTimeFromRanges)(
                        owner, 0, ((PuppetMotionRangeView*)rangeRecord)->startTime, value, 0);
                    RESET_ANIMATION_BOUNDS_TRANSFORMS(self, matrix, 1, 0);
                    AccumulateAnimationPoseVertices(self, owner, minimumCorner, maximumCorner);
                    if (((PuppetMotionRangeView*)rangeRecord)->startTime !=
                        ((PuppetMotionRangeView*)rangeRecord)->endTime) {
                        ((int (*)(void*, int, float, int, int))WPuppet_SetAnimationTimeFromRanges)(
                            owner, 0, ((PuppetMotionRangeView*)rangeRecord)->endTime, value, 0);
                        RESET_ANIMATION_BOUNDS_TRANSFORMS(self, matrix, 0, 1);
                        AccumulateAnimationPoseVertices(self, owner, minimumCorner, maximumCorner);
                        if (filter != 0)
                            break;
                    }
                }
            }
            rangeCursor = 4 + rangeCursor;
        }
    }
    // Phase 3: without a name filter, also include the default frame-zero pose.
    if (filter == 0) {
        ((int (*)(void*, int, float, int, int))WPuppet_SetAnimationTimeFromRanges)(owner, 0, kFloat_0_puppet, value, 0);
        RESET_ANIMATION_BOUNDS_TRANSFORMS(self, matrix, 0, 0);
        AccumulateAnimationPoseVertices(self, owner, minimumCorner, maximumCorner);
    }
    // Phase 4: publish either instance bounds or the caller's endpoints.
    float existingComponent;
    float sampledComponent;
    float boundsCenter[3];
    float cornerSum[3];
    float radiusVector[3];
    PublishSampledAnimationBounds(self, bounds, minimumCorner, maximumCorner, existingComponent, sampledComponent,
                                  boundsCenter, cornerSum, radiusVector);
}
#undef RESET_ANIMATION_BOUNDS_TRANSFORMS
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset

// Find a string tree's insertion parent and report the predecessor and link direction.
void* StringTree_FindInsertionPosition(void* self, void* input, void** out, unsigned char* flag1,
                                       unsigned char* flag2) {
    unsigned int bRef, aRef, b2Ref;
    unsigned int a, b, b2;
    const void *ap, *bp;
    void* parent = (char*)self + 4;
    void* cur;
    *out = 0;
    cur = *(void**)((char*)self + 4);
    *flag1 = 1;
    *flag2 = 1;
    while (cur) {
        parent = cur;
        unsigned int tag = *(unsigned int*)&((ResourceStringNode*)cur)->stringFlags;
        if ((tag >> 31) == 0) {
            ap = ((PuppetNameNodeView*)cur)->key.small.text;
            a = *(unsigned char*)&((ResourceStringNode*)cur)->stringFlags & 0x7f;
        } else {
            ap = *(void**)&((ResourceStringNode*)cur)->heapData;
            a = *(unsigned int*)&((ResourceStringNode*)cur)->heapLength;
        }
        tag = *(unsigned int*)input;
        aRef = a;
        if ((tag >> 31) == 0) {
            b = *(unsigned char*)input & 0x7f;
        } else {
            b = *(unsigned int*)((char*)input + 4);
        }
        tag = *(unsigned int*)input;
        bRef = b;
        if ((tag >> 31) == 0) {
            bp = (char*)input + 1;
            b2 = *(unsigned char*)input & 0x7f;
        } else {
            bp = *(void**)((char*)input + 8);
            b2 = *(unsigned int*)((char*)input + 4);
        }
        b2Ref = b2;
        const unsigned int mRef = PuppetCommonPrefixLength(bRef, b2Ref);
        int cmp = memcmp((void*)bp, (void*)ap, (int)PuppetCommonPrefixLength(aRef, mRef));
        if (!cmp) {
            if (mRef < a) {
                cmp = -1;
            } else if (mRef == a) {
                cmp = 0;
            } else {
                cmp = 1;
            }
        }
        if (cmp < 0) {
            cur = *(void**)cur;
            *flag1 = 1;
        } else {
            *out = cur;
            cur = *(void**)((char*)cur + 4);
            *flag1 = 0;
            *flag2 = 0;
        }
    }
    return parent;
}

extern const char* MSLString_LowercaseInPlace(void* arg0);
extern void PSMTX44Identity(void* arg0);
extern void String_ReserveCapacity(void* arg0, unsigned int arg1);
extern void String_ReplaceRange(void* arg0, int, int, const char*, const char*, const char*, const char*);
extern int String_FindSubstring(void* arg0, const char* text, unsigned int, unsigned int);
extern int BoneControllerMap_SetIndexedPayload(void* arg0, int* arg1, void* arg2);
extern float kFloat_0_25_Puppet, kFloat_0_75_Puppet, kPuppetHeadTextureMatrixValue24;

// Initialize default body or head texture-coordinate matrices from the resource basename pattern.
int WPuppet_InitializeDefaultTextureCoordinates(void* self) {
    if (!AsWPuppet(self)->layerController)
        return 0;
    const char* name = MSLString_LowercaseInPlace(AsWPuppet(self)->resourceName);
    char pad[4] = {};
    struct {
        unsigned int len;
        char buf[4];
        unsigned int cap;
    } s = {};
    char scratch[0x40];
    unsigned int n = strlen(name);
    String_ReserveCapacity(&s, n);
    char c = *(char*)((char*)&s - 8);
    ((void (*)(void*, int, int, const char*, const char*, const char*))String_ReplaceRange)(&s, 0, 0, name, name + n,
                                                                                            &c);
    struct StringCleanup {
        void* object;
        // Free only heap-backed temporary string storage.
        ~StringCleanup() {
            unsigned int* words = (unsigned int*)object;
            if (words[0] >> 31)
                appFreeUni__FPv(*(char**)(words + 2));
        }
    } cleanup = {&s};
    int mode = 0;
    PSMTX44Identity(scratch);
    const char* key = puppetPreloadStrings.defaultBodyPattern;
    unsigned int len = strlen(key);
    unsigned int pos = String_FindSubstring(&s, key, 0, len);
    if (pos != 0xFFFFFFFFu) {
        *(volatile float*)((char*)&scratch + 0x20) = kFloat_0_puppet;
        *(volatile float*)((char*)&scratch + 0x24) = kFloat_0_25_Puppet;
    } else {
        key = puppetPreloadStrings.defaultHeadPattern;
        len = strlen(key);
        pos = String_FindSubstring(&s, key, 0, len);
        if (pos != 0xFFFFFFFFu) {
            mode = 1;
            *(volatile float*)((char*)&scratch + 0x20) = kFloat_0_75_Puppet;
            *(volatile float*)((char*)&scratch + 0x24) = kPuppetHeadTextureMatrixValue24;
        } else {
            return 0;
        }
    }
    BoneControllerMap_SetIndexedPayload(AsWPuppet(self)->layerController, &mode, &scratch);

    return 1;
}

extern void* appMallocDummy__FUl(unsigned int);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const char*, const char*);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void* s_sharedCharDeleterVtable;
extern void* __vt__Q23std12length_error;
extern void* __dt__Q23std12length_errorFv(void*, int);
extern void RBTree_RebalanceAfterInsert(void*, void*);
extern void Engine_SetLightEnabled(int, void*);
extern void Engine_SetLight(int, void*);
extern void Engine_SetRenderStateList(void*, int);
extern bool Copy_IndexedMatricesOrIdentity(void*, void*, unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
struct Fn800510DCBlock {
    unsigned int d[0x19];
    unsigned int v;
    unsigned int e[6];
};
extern const Fn800510DCBlock s_puppetAlternateRenderDefaults;
extern const PuppetPreloadStrings puppetPreloadStrings;

struct Fn5D838Guard {
    void* owner;
    void* node;
    // Free the pending node until its insertion commits.
    ~Fn5D838Guard() {
        if (node)
            appFreeUni__FPv(node);
    }
};
struct Fn5D838Ref {
    int vt;
    int c1;
    int c2;
    char* s;
};
struct Fn5D838Holder {
    char* p;
    char* flag;
    // Guard the message array while borrowing the allocator marker.
    Fn5D838Holder(char* b, char* f) {
        p = b;
        flag = f;
    }
    // Free a message array whose shared owner was not installed.
    ~Fn5D838Holder() {
        if (p)
            __dla__FPv(p);
    }
};
struct Fn5D838ErrBase {
    void* vt;
    char* buf;
    Fn5D838Ref* ref;
    // Copy the length-error message and construct its shared deleter owner.
    Fn5D838ErrBase(const char* msg0) {
        vt = (void*)&__vt__Q23std11logic_error;
        char stringAllocator __attribute__((aligned(4)));
        volatile char stringAllocator2 __attribute__((aligned(4))) = 0;
        stringAllocator = 0;
        unsigned n = strlen(msg0) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            Fn5D838Holder h(buffer, &stringAllocator);
            Fn5D838Ref* r = (Fn5D838Ref*)__nw__FUl(16);
            if (r) {
                int table = (int)&s_sharedCharDeleterVtable;
                r->c1 = 1;
                r->c2 = 1;
                r->vt = table;
                r->s = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg0);
    }
    // Empty local ABI cleanup; runtime exception destruction handles the shared payload.
    ~Fn5D838ErrBase() {}
};
struct Fn5D838Err : Fn5D838ErrBase {
    // Install the derived vtable and throw the completed length-error payload.
    Fn5D838Err(const char* msg) : Fn5D838ErrBase(msg) {
        vt = (void*)&__vt__Q23std12length_error;
        __throw((const char*)(puppetPreloadStrings.lengthErrorType), this, (void*)__dt__Q23std12length_errorFv);
    }
    // Empty local cleanup, retaining the original exception construction contract.
    ~Fn5D838Err() {}
};
// Allocate and copy the heap representation of a string key.
static inline void Fn5D838Append(void* pair, const unsigned int* source) {
    char local __attribute__((aligned(4)));
    volatile char uninit __attribute__((aligned(4)));
    char allocator = uninit;
    const char* begin = (const char*)source[2];
    const char* end = begin + source[1];
    local = allocator;
    ((void (*)(void*, int, int, const char*, const char*, const char*))String_ReplaceRange)(pair, 0, 0, begin, end,
                                                                                            &local);
}
// Copy an inline string directly or duplicate heap text, then copy its mapped value.
static inline void Fn5D838Copy(void* pair, const unsigned int* source) {
    try {
        unsigned int first = source[0];
        if (!(first >> 31)) {
            *(unsigned int*)pair = first;
            unsigned int second = source[1];
            unsigned int third = source[2];
            *(unsigned int*)((char*)pair + 4) = second;
            *(unsigned int*)((char*)pair + 8) = third;
        } else {
            *(unsigned int*)pair = 0;
            unsigned int length = source[1];
            *(unsigned int*)((char*)pair + 4) = 0;
            *(unsigned int*)((char*)pair + 8) = 0;
            String_ReserveCapacity(pair, length);
            Fn5D838Append(pair, source);
        }
        *(unsigned int*)&((PuppetStringValuePair*)pair)->value = source[3];
    } catch (...) {
        extern void __throw(const char*, void*, void*) __attribute__((noreturn));
        __throw(0, 0, 0);
    }
}
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_loop_invariants off
// Copy a string/value pair into a new tree node, link it, and rebalance the tree.
void* NamedObjectMap_InsertNode(void* tree, void* parent, int insertLeft, int updateFirst, void* stringValuePair) {
    if (*(unsigned int*)tree == 0xFFFFFFFFu) {
        const char* message = puppetPreloadStrings.treeLengthError;
        Fn5D838Err error(message);
    }
    volatile bool flag __attribute__((aligned(4))) = false;
    void* node = appMallocDummy__FUl(0x1C);
    void* volatile nodeA;
    void* volatile nodeB;
    nodeB = node;
    nodeA = node;
    Fn5D838Guard guard;
    guard.owner = (char*)tree + 4;
    guard.node = node;
    void* pair = (char*)&((ResourceStringNode*)node)->stringFlags;
    if (pair) {
        Fn5D838Copy(pair, (const unsigned int*)stringValuePair);
    }
    void* n = guard.node;
    guard.node = 0;
    *(void**)((char*)n + 4) = 0;
    *(void**)n = 0;
    void** parentLink = (void**)((char*)n + 8);
    if (parentLink) {
        try {
            *parentLink = parent;
        } catch (...) {
            throw;
        }
    }
    if (insertLeft != 0) {
        *(void**)parent = n;
    } else {
        *(void**)((char*)parent + 4) = n;
    }
    unsigned int count = *(unsigned int*)tree;
    *(unsigned int*)tree = count + 1;
    RBTree_RebalanceAfterInsert(n, *(void**)((char*)tree + 4));
    if (updateFirst != 0) {
        *(void**)((char*)tree + 8) = n;
    }
    return n;
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset

struct Fn800510DCVObj {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8(float, void*, void*, void*, void*, void*, void*, int);
};
// Apply the alternate render-state list and draw children and the local model with evaluated layer matrices.
void WPuppet_DrawChildrenAndModel(void* self, void* camera, float frameTime, void* arg3, void* arg4, void* arg5,
                                  void* arg6) {
    Engine_SetLightEnabled(kPuppetLightSlot, AsWPuppet(self)->renderParamsEnabled);
    if (AsWPuppet(self)->renderParamsEnabled) {
        Engine_SetLight(kPuppetLightSlot, (char*)&AsWPuppet(self)->renderParams);
    }

    Fn800510DCBlock renderStates = s_puppetAlternateRenderDefaults;
    renderStates.v = AsWPuppet(self)->renderStateValue;
    Engine_SetRenderStateList(&renderStates, kRenderStateCount);
    void* layerController;
    void* layerMatricesOrChildNode;
    void* inheritedSkinMatrices;
    int layerMatrixAddress;
    layerMatricesOrChildNode = AsWPuppet(self)->layerMatrices;
    layerMatrixAddress = 0;
    inheritedSkinMatrices = AsWPuppet(self)->skinMatrices;
    int hasLayerMatrices;
    if (!layerMatricesOrChildNode) {
        hasLayerMatrices = 0;
    } else {
        layerController = AsWPuppet(self)->layerController;
        if (!layerController) {
            hasLayerMatrices = 0;
        } else {
            if (AsWPuppet(self)->layerResource == 0) {
                if (AsWPuppet(self)->modelResource) {
                    if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource,
                                                                             kChunkMaterialLayers)) {
                        AsWPuppet(self)->layerResource = AsWPuppet(self)->modelResource;
                    }
                } else if (AsWPuppet(self)->loadFlags == kContainerLoadMode) {
                    void* sentinel = (char*)&AsWPuppet(self)->childTail;
                    void* childNode = AsWPuppet(self)->childHead;
                    while (childNode != sentinel) {
                        if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(
                                AsWPuppet(((WPuppetListNode*)childNode)->value)->modelResource, kChunkMaterialLayers)) {
                            AsWPuppet(self)->layerResource =
                                AsWPuppet(((WPuppetListNode*)childNode)->value)->modelResource;
                            break;
                        }
                        childNode = ((WPuppetListNode*)childNode)->next;
                    }
                }
            }
            int layerCount;
            if (AsWPuppet(self)->layerResource) {
                layerCount = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->layerResource,
                                                                                  kChunkMaterialTransforms);
            } else {
                layerCount = 0;
            }
            if (layerCount == 0) {
                hasLayerMatrices = 0;
            } else {
                hasLayerMatrices =
                    Copy_IndexedMatricesOrIdentity(layerController, layerMatricesOrChildNode, layerCount);
            }
        }
    }
    if (hasLayerMatrices) {
        layerMatrixAddress = *(int*)&AsWPuppet(self)->layerMatrices;
    }
    void* sentinel = (char*)&AsWPuppet(self)->childTail;
    for (layerMatricesOrChildNode = AsWPuppet(self)->childHead; layerMatricesOrChildNode != sentinel;
         layerMatricesOrChildNode = ((WPuppetListNode*)layerMatricesOrChildNode)->next) {
        (*(Fn800510DCVObj**)((char*)layerMatricesOrChildNode + 8))
            ->v8(frameTime, camera, arg3, arg4, arg5, arg6, inheritedSkinMatrices, layerMatrixAddress);
    }
    if (AsWPuppet(self)->modelResource) {
        ((Fn800510DCVObj*)self)->v8(frameTime, camera, arg3, arg4, arg5, arg6, 0, layerMatrixAddress);
    }
    Engine_SetLightEnabled(kPuppetLightSlot, 0);
}

extern DATA_SECTION(".data") void* s_cLocaleData;
extern void __dl__FPv(void*);
extern void* __nwa__FUl(unsigned int);
extern int WPuppet_InitializeDefaultTextureCoordinates(void*);
struct F2368VBase {
    virtual void destroy(int);
};
struct
    F2368VObj { // ORIGINAL: slot 0x24 (v7) takes the child in r4; the Wii calls write v7() without it, the register still holds it
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
};
// Cache the local or child model that supplies the requested material-animation chunk.
static inline void f2368_ensure(void* self, int tag) {
    if (AsWPuppet(self)->layerResource)
        return;
    void* p = AsWPuppet(self)->modelResource;
    if (p) {
        if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, tag))
            AsWPuppet(self)->layerResource = AsWPuppet(self)->modelResource;
    } else if (AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* end = (char*)&AsWPuppet(self)->childTail;
        void* n = AsWPuppet(self)->childHead;
        for (; n != end; n = ((WPuppetListNode*)n)->next) {
            if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(
                    AsWPuppet(((WPuppetListNode*)n)->value)->modelResource, tag)) {
                AsWPuppet(self)->layerResource = AsWPuppet(((WPuppetListNode*)n)->value)->modelResource;
                break;
            }
        }
    }
}
// Resolve the outgoing child's material-animation resource during the layer-buffer reset phase.
static inline void f2368_ensureA(void* self, int tag) {
    if (AsWPuppet(self)->layerResource)
        return;
    void* p = AsWPuppet(self)->modelResource;
    if (p) {
        if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, tag))
            AsWPuppet(self)->layerResource = AsWPuppet(self)->modelResource;
    } else if (AsWPuppet(self)->loadFlags == kContainerLoadMode) {
        void* n = AsWPuppet(self)->childHead;
        void* end = (char*)&AsWPuppet(self)->childTail;
        for (; n != end; n = ((WPuppetListNode*)n)->next) {
            if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(
                    AsWPuppet(((WPuppetListNode*)n)->value)->modelResource, tag)) {
                AsWPuppet(self)->layerResource = AsWPuppet(((WPuppetListNode*)n)->value)->modelResource;
                break;
            }
        }
    }
}
// Replace a child model and rebuild its shared material-animation buffers and controller.
void WPuppet_ReplaceChildAndLayerAnimation(void* self, void* child, void* arg2, void* arg3, void* arg4, void* arg5,
                                           void* arg6) {
    void* p;
    void* array;
    unsigned int count;
    void* alloc;
    if (child) {
        (*(F2368VObj*)self).v7();
        f2368_ensureA(child, kChunkMaterialLayers);
        if (AsWPuppet(child)->layerResource) {
            p = AsWPuppet(self)->layerMatrices;
            if (p) {
                __dla__FPv(p);
                AsWPuppet(self)->layerMatrices = 0;
            }
            p = AsWPuppet(self)->layerScratch;
            if (p) {
                __dl__FPv(p);
                AsWPuppet(self)->layerScratch = 0;
            }
            p = AsWPuppet(self)->layerController;
            if (p) {
                if (p)
                    ((F2368VBase*)p)->destroy(1);
                AsWPuppet(self)->layerController = 0;
            }
            AsWPuppet(self)->layerResource = 0;
        }
    }
    if (!arg2)
        return;
    WPuppet_InsertChildBySortKey(self, arg2, arg3, arg4);
    f2368_ensure(arg2, kChunkMaterialLayers);
    if (!AsWPuppet(arg2)->layerResource)
        return;
    p = AsWPuppet(self)->layerMatrices;
    if (p) {
        __dla__FPv(p);
        AsWPuppet(self)->layerMatrices = 0;
    }
    p = AsWPuppet(self)->layerScratch;
    if (p) {
        __dl__FPv(p);
        AsWPuppet(self)->layerScratch = 0;
    }
    f2368_ensure(self, kChunkMaterialLayers);
    p = AsWPuppet(self)->layerResource;
    count = p ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, kChunkMaterialTransforms) : 0;
    if (!count)
        return;
    array = __nwa__FUl(count << 6);
    AsWPuppet(self)->layerMatrices = array;
    if (!array)
        return;
    if (array) {
        p = AsWPuppet(self)->layerResource;
        f2368_ensure(self, kChunkMaterialLayers);
        p = AsWPuppet(self)->layerResource;
        int ok = p ? (((int (*)(void*, void*, int))EvaluateMaterialAnimation__Q23gtl24IGenericModelingResourceCFPv)(
                          p, array, count) != 0)
                   : 0;
        if (!ok)
            return;
    }
    alloc = __nw__FUl(0x40);
    AsWPuppet(self)->layerScratch = alloc;
    if (alloc == 0)
        return;
    p = AsWPuppet(self)->layerController;
    if (p) {
        if (p)
            ((F2368VBase*)p)->destroy(1);
        AsWPuppet(self)->layerController = 0;
    }
    f2368_ensure(self, kChunkMaterialLayers);
    p = AsWPuppet(self)->layerResource;

    void* resource = p ? (void*)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(p, kChunkMaterialTransforms) : 0;
    if (!resource)
        return;
    (void)arg5;
    AsWPuppet(self)->layerController = CTextureCoordinateRegistry::Create(resource);
    WPuppet_InitializeDefaultTextureCoordinates(self);
    (void)arg6;
}

#pragma opt_lifetimes off
// Allocate material-layer matrices, evaluate their defaults, and create the texture-coordinate controller.
int WPuppet_InitializeTextureCoordinateAnimation(WPuppet* self) {
    int count;
    void* elements;
    void* node;
    void* obj;
    void* res;
    void* sentinel;
    if (self->layerMatrices) {
        __dla__FPv(self->layerMatrices);
        self->layerMatrices = 0;
    }
    if (self->layerScratch) {
        __dl__FPv(self->layerScratch);
        self->layerScratch = 0;
    }
    if (!AsWPuppet(self)->layerResource) {
        if (self->modelResource) {
            if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(self->modelResource, kChunkMaterialLayers))
                self->layerResource = self->modelResource;
        } else if (self->loadFlags == kContainerLoadMode) {
            sentinel = (char*)&AsWPuppet(self)->childTail;
            for (node = self->childHead; node != sentinel; node = *(void**)((char*)node + 4)) {
                obj = *(void**)((char*)node + 8);
                res = AsWPuppet(obj)->modelResource;
                if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(res, kChunkMaterialLayers)) {
                    self->layerResource = AsWPuppet(obj)->modelResource;
                    break;
                }
            }
        }
    }
    count = self->layerResource
                ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(self->layerResource, kChunkMaterialTransforms)
                : 0;
    if (!count)
        return 0;
    self->layerMatrices = __nwa__FUl(count << 6);
    if (!self->layerMatrices)
        return 0;
    elements = self->layerMatrices;
    if (!self->layerResource) {
        if (self->modelResource) {
            if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(self->modelResource, kChunkMaterialLayers))
                self->layerResource = self->modelResource;
        } else if (self->loadFlags == kContainerLoadMode) {
            sentinel = (char*)&AsWPuppet(self)->childTail;
            for (node = self->childHead; node != sentinel; node = *(void**)((char*)node + 4)) {
                obj = *(void**)((char*)node + 8);
                res = AsWPuppet(obj)->modelResource;
                if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(res, kChunkMaterialLayers)) {
                    self->layerResource = AsWPuppet(obj)->modelResource;
                    break;
                }
            }
        }
    }
    if (!self->layerResource)
        return 0;
    if (!(((int (*)(void*, void*, int))EvaluateMaterialAnimation__Q23gtl24IGenericModelingResourceCFPv)(
              self->layerResource, elements, count) != 0))
        return 0;
    self->layerScratch = __nw__FUl(0x40);
    if (!self->layerScratch)
        return 0;
    if (self->layerController) {
        void** vt = *(void***)self->layerController;
        ((void (*)(void*, int))vt[2])(self->layerController, 1);
        self->layerController = 0;
    }
    if (!self->layerResource) {
        if (self->modelResource) {
            if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(self->modelResource, kChunkMaterialLayers))
                self->layerResource = self->modelResource;
        } else if (self->loadFlags == kContainerLoadMode) {
            sentinel = (char*)&AsWPuppet(self)->childTail;
            for (node = self->childHead; node != sentinel; node = *(void**)((char*)node + 4)) {
                obj = *(void**)((char*)node + 8);
                res = AsWPuppet(obj)->modelResource;
                if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(res, kChunkMaterialLayers)) {
                    self->layerResource = AsWPuppet(obj)->modelResource;
                    break;
                }
            }
        }
    }
    void* resource;
    if (!self->layerResource || !(resource = (void*)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(
                                      self->layerResource, kChunkMaterialTransforms)))
        return 0;
    self->layerController = CTextureCoordinateRegistry::Create(resource);
    WPuppet_InitializeDefaultTextureCoordinates(self);
    return 1;
}
#pragma opt_lifetimes reset

struct A5C4Str {
    unsigned int hdr;
    unsigned int size;
    void* data;
};
struct A5C4Uninit {
    volatile char value;
    char pad[3];
};
struct A5C4Alloc {
    char value;
    char pad[3];
};

// Parse an indexed bone prefix and recognize it only when its suffix equals the
// resource name with its LAST extension removed (CString_Compare semantics).
// Required input: NUL-terminated narrow name shorter than 0x80 bytes. Recognized
// form is <one byte>_<digit-starting token>_<resource basename>. The first byte
// is unconstrained; only the token's FIRST byte is checked with locale digit bit 8.
// String_ParseDecimalInt interprets the whole token; do not promise all-digits
// validation. A recognized candidate MUST have the second '_' and token length
// <0x40; neither condition is checked before copying into suffix[0x40].
// Required output capacity: at least strlen(input)+1 bytes (0x80 suffices under
// the input contract). No capacity parameter exists. result is required and may
// not overlap output/input/self; output must not overlap self's resource name.
// In-place input/output is safe because input is
// copied locally first. This routine does not own any caller buffer.
// Always writes *result=-1 first. An unrecognized prefix copies the entire input
// to output; a candidate copies text after the second '_' even if basename
// comparison fails. Only a matching basename returns 1 and writes the parsed
// index; all other supported paths return 0. Malformed candidates are outside
// the contract, not a safe failure path. Temporary string allocations may throw.
int WPuppet_ParseIndexedBoneName(void* self, void* input, void* output, int* result) {
    char name[0x80];
    *result = -1;
    strncpy(name, (const char*)input, 0x80);
    unsigned int ok = 0;
    if (name[1] == '_') {
        int bad;
        int c = (signed char)name[2];
        bad = 0;
        if (c < 0 || c >= 0x100)
            bad = 1;
        ok = (bad != 0)
                 ? ok
                 : *(unsigned short*)((char*)*(void**)((char*)*(void**)((char*)&s_cLocaleData + 0x38) + 8) + c * 2) & 8;
    }
    if (ok == 0) {
        strcpy((char*)output, name);
        return 0;
    }
    char suffix[0x40];
    const char* p = name + 2;
    Memory_Clear(suffix, 0x40);
    char* q = suffix;
    while (*p != '_' && *p)
        *q++ = *p++;
    int parsedIndex = String_ParseDecimalInt(suffix);
    strcpy((char*)output, p + 1);
    unsigned int len;
    char* base = (char*)AsWPuppet(self)->resourceName;
    A5C4Str s = {0, 0, 0};
    A5C4Str b;
    A5C4Uninit uninit;
    A5C4Alloc alloc;
    A5C4Uninit uninit2;
    A5C4Alloc alloc2;
    len = strlen(base);
    String_ReserveCapacity(&s, len);
    alloc.value = uninit.value;
    ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(&s, 0, 0, base, base + len,
                                                                                      &alloc.value);
    struct StringCleanup {
        void* object;
        // Free only heap-backed temporary string storage.
        ~StringCleanup() {
            if (object) {
                unsigned int* words = (unsigned int*)object;
                if (words[0] >> 31)
                    appFreeUni__FPv(*(char**)(words + 2));
            }
        }
    } cleanupS = {&s};
    const char* sd;
    unsigned int sl;
    unsigned int idx = -1;
    if (!(s.hdr >> 31)) {
        sd = (char*)&s + 1;
        sl = *(unsigned char*)&s & 0x7f;
    } else {
        sd = (const char*)s.data;
        sl = s.size;
    }
    unsigned int dot;
    if (sl != 0) {
        if (sl - 1 < -1U)
            idx = sl - 1;
        const char* cur = sd + idx;
        for (;;) {
            if (*cur == '.') {
                dot = cur - sd;
                goto found_dot;
            }
            if (cur <= sd)
                break;
            --cur;
        }
    }
    dot = -1;
found_dot: // Keep the result join: structured scans change register allocation.
    String_ConstructSubstring(&b, &s, 0, dot);
    StringCleanup cleanupB = {&b};
    if (!(s.hdr >> 31) && !(b.hdr >> 31)) {
        s = b;
    } else {
        unsigned int r5 = (s.hdr >> 31) ? s.size : (*(unsigned char*)&s & 0x7f);
        const char* bd;
        unsigned int bl;
        if (!(b.hdr >> 31)) {
            bd = (char*)&b + 1;
            bl = *(unsigned char*)&b & 0x7f;
        } else {
            bd = (const char*)b.data;
            bl = b.size;
        }
        alloc2.value = uninit2.value;
        ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(&s, 0, r5, bd, bd + bl,
                                                                                          &alloc2.value);
    }
    cleanupB.object = 0;
    if (b.hdr >> 31)
        appFreeUni__FPv(b.data);
    const char* aData;
    if (s.hdr >> 31) {
        aData = (const char*)s.data;
    } else {
        aData = (char*)&s + 1;
    }
    if (CString_Compare((const char*)output, aData) != 0) {
        cleanupS.object = 0;
        if (s.hdr >> 31)
            appFreeUni__FPv(s.data);
        return 0;
    }
    *result = parsedIndex;
    cleanupS.object = 0;
    if (s.hdr >> 31)
        appFreeUni__FPv(s.data);
    return 1;
}

void* VertexBuffer_GetDescription(void*, void*);
void* Resource_GetOffsetPtr(void*, int, void*, unsigned int*, int);
void GraphicsBuffer_Flush(void*);
void* FlexibleVertexFormat_GetPositionAddress(void*, void*);

struct W57Ref {
    CW_BITS2(unsigned cnt : 16, unsigned rest : 16)
};
extern void CubeResourceStorage_dtor(void*, int);
struct W57Handle {
    W57Ref* p;
    // Initialize an empty retained graphics-resource handle.
    W57Handle() : p(0) {}
    // Release one 16-bit reference, destroying the resource at zero, and clear the handle.
    void reset() {
        W57Ref* q = p;
        if (q) {
            if (--q->cnt == 0)
                CubeResourceStorage_dtor(q, 1);
        }
        p = 0;
    }
};
// Four numeric words written by VertexBuffer_GetDescription (gtl_graphics_texture_buffer_resources.cpp).
// They are not pointers: word 2 is byte count, word 3 a flexible-vertex-format code.
struct BoundsVertexDescription {
    unsigned int formatSentinel, resourceKind, byteCount, vertexFormat;
};
// Bounds-only projections of the SAME resources as the opening schemas:
// Bone: next/index/worldMatrix at +64/+68/+78; the full bone stride is irrelevant
// here because traversal follows next rather than pointer arithmetic.
struct BoundsBoneView {
    char pad[0x64];
    BoundsBoneView* next;
    unsigned int index;
    char pad2[0xc];
    float worldMatrix[12];
};
// Model layer: identical 0x30 stride, group fields +0/+2 and transform index +18.
struct BoundsLayerView {
    unsigned short groupCount;
    unsigned short firstGroup;
    char pad[0x14];
    unsigned short transformIndex;
    char pad2[0x16];
};
// This path advances mesh groups by TWELVE bytes, unlike the opening eight-byte
// PuppetMeshGroupView projection used in element-by-element loader queries.
// Keep this distinct until the resource's group stride is reconciled. Bounds
// narrows primitiveCountWord to its low 16 bits; the loader uses the whole word.
struct BoundsMeshGroupView {
    unsigned int firstPrimitive;
    unsigned int primitiveCountWord;
    unsigned int unknown08;
};
// Same 0x20 primitive stride as PuppetPrimitiveView; additional fields consumed
// here are vertexCount +8, vertexStrideBytes +A, vertexBufferIndex +12.
struct BoundsPrimitiveView {
    char pad[8];
    unsigned short vertexCount;
    unsigned short vertexStrideBytes;
    char pad2[6];
    unsigned short vertexBufferIndex;
    char pad3[0xc];
};
typedef char BoundsLayer_stride[(sizeof(BoundsLayerView) == sizeof(PuppetModelLayerView)) ? 1 : -1];
typedef char BoundsPrimitive_stride[(sizeof(BoundsPrimitiveView) == sizeof(PuppetPrimitiveView)) ? 1 : -1];
typedef char BoundsMeshGroup_stride[(sizeof(BoundsMeshGroupView) == 12) ? 1 : -1];
typedef char BoundsDescription_size[(sizeof(BoundsVertexDescription) == 16) ? 1 : -1];
typedef char BoundsBone_matrixOffset
    [(PUPPET_OFFSET(BoundsBoneView, worldMatrix) == PUPPET_OFFSET(PuppetBoneView, worldMatrix)) ? 1 : -1];
typedef char BoundsLayer_transformOffset
    [(PUPPET_OFFSET(BoundsLayerView, transformIndex) == PUPPET_OFFSET(PuppetModelLayerView, transformIndex)) ? 1 : -1];
typedef char BoundsPrimitive_bufferOffset[(PUPPET_OFFSET(BoundsPrimitiveView, vertexBufferIndex) == 0x12) ? 1 : -1];

// Choose the smaller bounds coordinate.
template <class T> inline const T& w57min(const T& a, const T& b) {
    return b < a ? b : a;
}
// Choose the larger bounds coordinate.
template <class T> inline const T& w57max(const T& a, const T& b) {
    return a < b ? b : a;
}

#pragma use_lmw_stmw on
// Expand caller-initialized three-float min/max accumulators; empty geometry
// leaves them unchanged. Borrow transform (0x40-byte matrices), a linked bone
// chain, and optional extraArg (0x30-byte affine). Every bone/layer/group/buffer
// index and vertex span must be valid. Resource/query/mapping statuses are
// ignored; there is no invalid-buffer recovery or bounds checking here.
// For each vertex: copy XYZ -> perspective transform by layer skin matrix ->
// affine bone worldMatrix -> optional extra affine -> compare with min/max.
// AccumulateAnimationPoseVertices prepares an identity-root hierarchy for model
// bounds. Other callers must initialize bones/skin for their desired output
// space; this function applies no yards or meters scaling and no axis remapping.
// Recursion deliberately passes the SAME matrices, bone chain, accumulators and
// extra matrix to every child puppet's model. Children must share compatible
// bone/layer indices. Hierarchy must be acyclic/stable; buffers must not be
// mutated concurrently (mapping/flush is not a mutex). A retained buffer handle
// lives for one primitive, is flushed after reading, then released via reset.
void WPuppet_AccumulateTransformedBounds(void* self, void* transform, void* nodes, void* minArg, void* maxArg,
                                         void* extraArg) {
    float* minOut = (float*)minArg;
    float* maxOut = (float*)maxArg;
    float* extra = (float*)extraArg;
    if (AsWPuppet(self)->modelResource) {
        int gcount;
        BoundsLayerView* layer;
        BoundsMeshGroupView* meshGroups;
        GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource, kChunkModelLayers);
        layer = (BoundsLayerView*)GetChunkElement__Q23gtl20IGenericResourceFileFii(AsWPuppet(self)->modelResource,
                                                                                   (const char*)kChunkModelLayers, 0);
        meshGroups = (BoundsMeshGroupView*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
            AsWPuppet(self)->modelResource, (const char*)kChunkMeshGroups, 0);
        BoundsPrimitiveView* records = (BoundsPrimitiveView*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
            AsWPuppet(self)->modelResource, (const char*)kChunkPrimitives, 0);
        W57Handle handle;
        for (BoundsBoneView* child = (BoundsBoneView*)nodes; child; child = child->next) {
            BoundsLayerView* record = layer + child->index;
            gcount = record->groupCount;
            for (int g = 0; gcount > g; ++g) {
                BoundsMeshGroupView* group = record->firstGroup + meshGroups + g;
                unsigned int primitiveCountWord = group->primitiveCountWord;
                int count = (unsigned short)primitiveCountWord;
                for (int i = 0; count > i; ++i) {
                    BoundsPrimitiveView* entry = records + group->firstPrimitive + i;
                    ((int (*)(void*, int, int, void*))GetResource__Q23gtl24IGenericModelingResourceFiiPP12GtlRefHeader)(
                        AsWPuppet(self)->modelResource, 0, entry->vertexBufferIndex, &handle.p);
                    float box[3];
                    BoundsVertexDescription description;
                    VertexBuffer_GetDescription(handle.p, &description);
                    void* base = 0;
                    // Third word is passed as a legacy query argument; the
                    // implementation ignores it and returns storage+4+offset.
                    Resource_GetOffsetPtr(handle.p, 0, (void*)description.byteCount, (unsigned int*)&base, 0);
                    unsigned int jcount = entry->vertexCount;
                    for (unsigned int j = 0; j < jcount; ++j) {
                        void* v = FlexibleVertexFormat_GetPositionAddress((char*)base + entry->vertexStrideBytes * j,
                                                                          (void*)description.vertexFormat);
                        float point[3];
                        memcpy(point, v, 12);
                        Mtx44_TransformPointPerspective(point, point, (char*)transform + record->transformIndex * 0x40);
                        Vec3_CopySourceToDestination(point, box);
                        float t[3];
                        Matrix_TransformPoint(t, box, child->worldMatrix);
                        __as__7WVectorFRC7WVector(box, t);
                        if (extra) {
                            float t2[3];
                            Matrix_TransformPoint(t2, box, extra);
                            __as__7WVectorFRC7WVector(box, t2);
                        }
                        minOut[0] = w57min(minOut[0], box[0]);
                        maxOut[0] = w57max(maxOut[0], box[0]);
                        minOut[1] = w57min(minOut[1], box[1]);
                        maxOut[1] = w57max(maxOut[1], box[1]);
                        minOut[2] = w57min(minOut[2], box[2]);
                        maxOut[2] = w57max(maxOut[2], box[2]);
                    }
                    __typeof__(handle.p) handlePCopy = handle.p;
                    GraphicsBuffer_Flush(handlePCopy);
                    handle.reset();
                }
            }
        }
    }
    void* sentinel = (char*)&AsWPuppet(self)->childTail;
    for (void* node = AsWPuppet(self)->childHead; sentinel != node; node = ((WPuppetListNode*)node)->next)
        WPuppet_AccumulateTransformedBounds(((WPuppetListNode*)node)->value, transform, nodes, minOut, maxOut, extra);
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs off
#pragma opt_propagation off
// Subtract using the original inline arithmetic shape used when clearing vector counts.
static inline unsigned int SubtractReleasedCount(unsigned int a, unsigned int b) {
    return a - b;
}
struct Key57388 {
    unsigned int tag;
    unsigned int size;
    char* data;
    // Copy a borrowed key into the original small-string representation.
    Key57388(const char* input) : tag(0), size(0), data(0) {
        const char* name = input;
        unsigned int len = strlen(name);
        String_ReserveCapacity(this, len);
        volatile char uninit __attribute__((aligned(4)));
        char local __attribute__((aligned(4))) = uninit;
        ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(this, 0, 0, name, name + len,
                                                                                          &local);
    }
    // Intentionally empty: surrounding code performs the string-storage cleanup.
    ~Key57388() {}
};
#pragma opt_common_subs off
struct Key57388Second {
    unsigned int tag;
    unsigned int size;
    char* data;
    // Copy the second lookup key while retaining its distinct allocator lifetimes.
    Key57388Second(const char* input) : tag(0), size(0), data(0) {
        const char* name = input;
        unsigned int len = strlen(name);
        String_ReserveCapacity(this, len);
        char local __attribute__((aligned(4)));
        volatile char uninit __attribute__((aligned(4)));
        local = uninit;
        ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(this, 0, 0, name, name + len,
                                                                                          &local);
    }
    // Intentionally empty: surrounding code performs the string-storage cleanup.
    ~Key57388Second() {}
};
struct Vec57388 {
    void** data;
    unsigned int size;
    unsigned int capacity;
    // Clear the count and free pointer storage without destroying pointed-to values.
    ~Vec57388() {
        ((MSLRecordVector*)this)->~MSLRecordVector();
    }
};
// Delete an allocated pointer vector and clear its owning slot.
static inline void DeletePuppetPointerVector(Vec57388** slot) {
    if (*slot) {
        delete *slot;
        *slot = 0;
    }
}
// Compare iterator values using the original inline equality helper.
template <class T> static inline bool PuppetIteratorValuesEqual(const T& a, const T& b) {
    return a == b;
}
struct MotionOwnedBuffers {
    char pad[0x18];
    void* a;
    void* b;
};
struct PuppetPointerIterator {
    void** value;
};
struct PuppetTrivialAllocation {};
struct PuppetDeletingDispatch {
    virtual void destroy(int);
};
struct PuppetResourceReferenceWord {
    unsigned int refs;
};
struct PuppetResourceReleaseDispatch : PuppetResourceReferenceWord {
    virtual void destroy(int);
};
// Return the shorter key length by reference.
static inline const unsigned int& ShorterResourceNameLength(const unsigned int& a, const unsigned int& b) {
    const unsigned int* result = &b;
    if (a < b)
        result = &a;
    return *result;
}
// Order equal-prefix keys by their lengths.
static inline int CompareResourceNameLengths(unsigned int a, unsigned int b) {
    if (a < b)
        return -1;
    if (a == b)
        return 0;
    return 1;
}
// Compare a stored inline/heap key with a byte span, retaining comparison intermediates.
static inline int CompareResourceNameSpan(const void* storedName, unsigned int queryLength, const void* queryBytes,
                                          unsigned int& storedLength, unsigned int& queryLengthCopy,
                                          unsigned int& storageLengthCopy, unsigned int& comparisonLength) {
    queryLengthCopy = queryLength;
    storedLength = !(*(unsigned int*)storedName >> 31) ? (*(unsigned char*)storedName & 0x7F)
                                                       : ((const MWStringStorage*)storedName)->length;
    const char* storedBytes;
    unsigned int storageLength;
    if (!(*(unsigned int*)storedName >> 31)) {
        storedBytes = (const char*)storedName + 1;
        storageLength = *(unsigned char*)storedName & 0x7F;
    } else {
        storedBytes = ((const MWStringStorage*)storedName)->data;
        storageLength = ((const MWStringStorage*)storedName)->length;
    }
    storageLengthCopy = storageLength;
    comparisonLength = ShorterResourceNameLength(storedLength, storageLengthCopy);
    int ordering =
        memcmp((void*)storedBytes, (void*)queryBytes, ShorterResourceNameLength(queryLengthCopy, comparisonLength));
    if (ordering == 0) {
        if (comparisonLength < queryLength)
            return -1;
        if (comparisonLength == queryLength)
            return 0;
        return 1;
    }
    return ordering;
}
// Return one when a resource-name word is zero, and zero otherwise.
static inline int ResourceNameIsZero(int value) {
    return value == 0;
}
// Convert the string's high-bit tag to the original integer heap-storage switch.
static inline int ResourceNameUsesHeap(unsigned int tag) {
    return ResourceNameIsZero(ResourceNameIsZero(tag >> 31));
}
// Compare a stored key against a query whose inline/heap representation was already decoded.
static inline int CompareDecodedResourceName(const void* storedName, int usesHeap, unsigned int inlineLength,
                                             unsigned int queryLength, const void* queryBytes,
                                             unsigned int& storedLength, unsigned int& queryLengthCopy,
                                             unsigned int& storageLengthCopy, unsigned int& comparisonLength) {
    queryLengthCopy = queryLength;
    const char* storedBytes;
    unsigned int storageLength;
    storedLength = usesHeap == 0 ? inlineLength : ((const MWStringStorage*)storedName)->length;
    if (!usesHeap) {
        storedBytes = (const char*)storedName + 1;
        storageLength = inlineLength;
    } else {
        storedBytes = ((const MWStringStorage*)storedName)->data;
        storageLength = ((const MWStringStorage*)storedName)->length;
    }
    storageLengthCopy = storageLength;
    comparisonLength = ShorterResourceNameLength(storedLength, storageLengthCopy);
    int ordering =
        memcmp((void*)storedBytes, (void*)queryBytes, ShorterResourceNameLength(queryLengthCopy, comparisonLength));
    if (ordering == 0) {
        if (comparisonLength < queryLength)
            return -1;
        if (comparisonLength == queryLength)
            return 0;
        return 1;
    }
    return ordering;
}
// Find the shared resource-name tree entry that exactly matches a temporary string.
static inline void* FindNameBeforeMotionRelease(void* query) {
    unsigned int queryComparisonLength, queryStorageLength, registeredLengthCopy, queryLength;
    unsigned int nodeLength, queryLengthCopy, nodeStorageLength, nodeComparisonLength;
    unsigned int queryUsesHeap;
    void* candidate;
    unsigned int queryInlineLength;
    int queryHeapTag;
    char* queryHeapBytes;
    int queryHeapLength;
    int querySpanLength;
    void* node;
    queryHeapTag = *(unsigned int*)query >> 31;
    node = *(void**)(s_puppetResourceNameRefCounts + 4);
    queryHeapBytes = *(char**)((char*)query + 8);
    queryHeapLength = *(unsigned int*)((char*)query + 4);
    candidate = (void*)(s_puppetResourceNameRefCounts + 4);
    queryUsesHeap = ResourceNameIsZero(ResourceNameIsZero(queryHeapTag));
    queryInlineLength = *(unsigned char*)query & 0x7F;
    while (node) {
        const char* queryBytes;
        if (!queryUsesHeap) {
            querySpanLength = queryInlineLength;
            queryBytes = (const char*)query + 1;
        } else {
            queryBytes = queryHeapBytes;
            querySpanLength = queryHeapLength;
        }
        if (CompareResourceNameSpan((char*)&((ResourceStringNode*)node)->stringFlags, querySpanLength, queryBytes,
                                    nodeLength, queryLengthCopy, nodeStorageLength, nodeComparisonLength) >= 0) {
            candidate = node;
            node = *(void**)node;
        } else {
            node = *(void**)((char*)node + 4);
        }
    }
    if (candidate != (void*)(s_puppetResourceNameRefCounts + 4)) {
        unsigned int registeredTag = *(unsigned int*)&((ResourceStringNode*)candidate)->stringFlags;
        unsigned int registeredLength;
        const char* registeredBytes;
        if (!(registeredTag >> 31)) {
            registeredBytes = (const char*)candidate + 0xD;
            registeredLength = *(unsigned char*)&((ResourceStringNode*)candidate)->stringFlags & 0x7F;
        } else {
            registeredBytes = *(const char**)&((ResourceStringNode*)candidate)->heapData;
            registeredLength = *(unsigned int*)&((ResourceStringNode*)candidate)->heapLength;
        }
        int ordering =
            CompareDecodedResourceName(query, queryUsesHeap, queryInlineLength, registeredLength, registeredBytes,
                                       queryLength, registeredLengthCopy, queryStorageLength, queryComparisonLength);
        if (ordering < 0)
            candidate = (void*)(s_puppetResourceNameRefCounts + 4);
    }
    if (queryHeapTag)
        appFreeUni__FPv(*(void**)((char*)query + 8));
    return candidate;
}
// Repeat the exact-name lookup before deciding whether bone matrices are shared.
static inline void* FindNameBeforeBoneMatrixRelease(void* query) {
    unsigned int nodeComparisonLength, nodeStorageLength, queryLengthCopy, nodeLength;
    unsigned int queryLength, registeredLengthCopy, queryStorageLength, queryComparisonLength;
    unsigned int queryUsesHeap;
    void* candidate;
    unsigned int queryInlineLength;
    int queryHeapTag;
    char* queryHeapBytes;
    int queryHeapLength;
    int querySpanLength;
    void* node;
    queryHeapTag = *(unsigned int*)query >> 31;
    node = *(void**)(s_puppetResourceNameRefCounts + 4);
    queryHeapBytes = *(char**)((char*)query + 8);
    queryHeapLength = *(unsigned int*)((char*)query + 4);
    candidate = (void*)(s_puppetResourceNameRefCounts + 4);
    queryUsesHeap = ResourceNameIsZero(ResourceNameIsZero(queryHeapTag));
    queryInlineLength = *(unsigned char*)query & 0x7F;
    while (node) {
        const char* queryBytes;
        if (!queryUsesHeap) {
            querySpanLength = queryInlineLength;
            queryBytes = (const char*)query + 1;
        } else {
            queryBytes = queryHeapBytes;
            querySpanLength = queryHeapLength;
        }
        if (CompareResourceNameSpan((char*)&((ResourceStringNode*)node)->stringFlags, querySpanLength, queryBytes,
                                    nodeLength, queryLengthCopy, nodeStorageLength, nodeComparisonLength) >= 0) {
            candidate = node;
            node = *(void**)node;
        } else {
            node = *(void**)((char*)node + 4);
        }
    }
    if (candidate != (void*)(s_puppetResourceNameRefCounts + 4)) {
        unsigned int registeredTag = *(unsigned int*)&((ResourceStringNode*)candidate)->stringFlags;
        unsigned int registeredLength;
        const char* registeredBytes;
        if (!(registeredTag >> 31)) {
            registeredBytes = (const char*)candidate + 0xD;
            registeredLength = *(unsigned char*)&((ResourceStringNode*)candidate)->stringFlags & 0x7F;
        } else {
            registeredBytes = *(const char**)&((ResourceStringNode*)candidate)->heapData;
            registeredLength = *(unsigned int*)&((ResourceStringNode*)candidate)->heapLength;
        }
        int ordering =
            CompareDecodedResourceName(query, queryUsesHeap, queryInlineLength, registeredLength, registeredBytes,
                                       queryLength, registeredLengthCopy, queryStorageLength, queryComparisonLength);
        if (ordering < 0)
            candidate = (void*)(s_puppetResourceNameRefCounts + 4);
    }
    if (queryHeapTag)
        appFreeUni__FPv(*(void**)((char*)query + 8));
    return candidate;
}
// Drop one intrusive resource reference, delete on zero, and clear the owner's pointer.
static inline void ReleasePuppetResourceReference(void** slot) {
    void* v = *slot;
    if (v) {
        if (--*(unsigned int*)v == 0) {
            if (v)
                ((PuppetResourceReleaseDispatch*)v)->destroy(1);
        }
        *slot = 0;
    }
}
#pragma use_lmw_stmw on
#pragma opt_propagation on
// Release model references and owned buffers, keeping metadata that is shared by registered resource names.
void WPuppet_ReleaseResources(void* input) {
    // Phase 1: release retained model/animation/name resources and per-instance arrays.
    char* self = (char*)input;
    ReleasePuppetResourceReference((void**)((char*)&AsWPuppet(self)->modelResource));
    ReleasePuppetResourceReference((void**)((char*)&AsWPuppet(self)->animation));
    ReleasePuppetResourceReference((void**)((char*)&AsWPuppet(self)->nameIndexMap));
    if (AsWPuppet(self)->bones) {
        __destroy_new_array(AsWPuppet(self)->bones, (void*)WPuppetBone_dtor);
        AsWPuppet(self)->bones = 0;
    }
    if (AsWPuppet(self)->modelLayerVisibilityWords) {
        __dla__FPv(AsWPuppet(self)->modelLayerVisibilityWords);
        AsWPuppet(self)->modelLayerVisibilityWords = 0;
    }
    {
        volatile unsigned int* p = (volatile unsigned int*)((char*)&AsWPuppet(self)->materialCount);
        unsigned int n = *p;
        *p = SubtractReleasedCount(n, n);
    }
    WPuppet_ReleaseNamedBoneData(self);
    // Phase 2: preserve metadata governed by a registered basename; only the
    // unregistered branch frees motion records, shared matrices and key buffers.
    bool motionMetadataIsShared;
    if (!AsWPuppet(self)->hasResourceName) {
        motionMetadataIsShared = false;
    } else {
        void* registeredName;
        {
            Key57388 key((const char*)AsWPuppet(self)->resourceName);
            registeredName = FindNameBeforeMotionRelease(&key);
        }
        motionMetadataIsShared = (void*)((char*)s_puppetResourceNameRefCounts + 4) != registeredName;
    }
    if (!motionMetadataIsShared) {
        if (*(Vec57388**)&AsWPuppet(self)->animationRanges) {
            for (PuppetPointerIterator it = {(*(Vec57388**)&AsWPuppet(self)->animationRanges)->data};
                 it.value != (*(Vec57388**)&AsWPuppet(self)->animationRanges)->data +
                                 (*(Vec57388**)&AsWPuppet(self)->animationRanges)->size;
                 ++it.value) {
                if (*it.value) {
                    if (((MotionOwnedBuffers*)*it.value)->a) {
                        __dla__FPv(((MotionOwnedBuffers*)*it.value)->a);
                        ((MotionOwnedBuffers*)*it.value)->a = 0;
                    }
                    if (((MotionOwnedBuffers*)*it.value)->b) {
                        __dla__FPv(((MotionOwnedBuffers*)*it.value)->b);
                        ((MotionOwnedBuffers*)*it.value)->b = 0;
                    }
                }
            }
            if (*(volatile unsigned int*)&(*(Vec57388**)&AsWPuppet(self)->animationRanges)->size &&
                (*(Vec57388**)&AsWPuppet(self)->animationRanges)->data[0])
                __dla__FPv((*(Vec57388**)&AsWPuppet(self)->animationRanges)->data[0]);
            {
                unsigned int n = *(volatile unsigned int*)&(*(Vec57388**)&AsWPuppet(self)->animationRanges)->size;
                (*(Vec57388**)&AsWPuppet(self)->animationRanges)->size = SubtractReleasedCount(n, n);
            }
            DeletePuppetPointerVector((Vec57388**)((char*)&AsWPuppet(self)->animationRanges));
        }
        bool boneMatricesAreShared;
        if (!AsWPuppet(self)->hasResourceName) {
            boneMatricesAreShared = false;
        } else {
            void* registeredMatrixName;
            {
                Key57388Second key((const char*)AsWPuppet(self)->resourceName);
                registeredMatrixName = FindNameBeforeBoneMatrixRelease(&key);
            }
            boneMatricesAreShared = (void*)((char*)s_puppetResourceNameRefCounts + 4) != registeredMatrixName;
        }
        if (!boneMatricesAreShared) {
            if (AsWPuppet(self)->boneMatrices) {
                __dla__FPv(AsWPuppet(self)->boneMatrices);
                AsWPuppet(self)->boneMatrices = 0;
            }
        }
        if (AsWPuppet(self)->materialMetadata) {
            __dla__FPv(AsWPuppet(self)->materialMetadata);
            AsWPuppet(self)->materialMetadata = 0;
        }
        if (AsWPuppet(self)->keyRange) {
            if (AsWPuppet(self)->keyRange)
                delete (PuppetTrivialAllocation*)AsWPuppet(self)->keyRange;
            AsWPuppet(self)->keyRange = 0;
        }
        if (AsWPuppet(self)->keyRecords) {
            __destroy_new_array(AsWPuppet(self)->keyRecords, (void*)PuppetKeyRecord_dtor);
            AsWPuppet(self)->keyRecords = 0;
        }
    }
    // Phase 3: release instance-only skin/controller/preload/layer resources.
    if (AsWPuppet(self)->skinMatrices) {
        __dla__FPv(AsWPuppet(self)->skinMatrices);
        AsWPuppet(self)->skinMatrices = 0;
    }
    if (AsWPuppet(self)->boneController) {
        if (AsWPuppet(self)->boneController)
            ((PuppetDeletingDispatch*)AsWPuppet(self)->boneController)->destroy(1);
        AsWPuppet(self)->boneController = 0;
    }
    if (*(Vec57388**)&AsWPuppet(self)->namedResources) {
        void** end;
        for (PuppetPointerIterator it = {(*(Vec57388**)&AsWPuppet(self)->namedResources)->data};
             (end = (*(Vec57388**)&AsWPuppet(self)->namedResources)->data +
                    (*(Vec57388**)&AsWPuppet(self)->namedResources)->size,
             !(end == it.value));
             ++it.value) {
            if (*it.value) {
                if (*it.value)
                    ((PuppetDeletingDispatch*)*it.value)->destroy(1);
                *it.value = 0;
            }
        }
        (*(Vec57388**)&AsWPuppet(self)->namedResources)->size =
            SubtractReleasedCount((*(Vec57388**)&AsWPuppet(self)->namedResources)->size,
                                  (*(Vec57388**)&AsWPuppet(self)->namedResources)->size);
        DeletePuppetPointerVector((Vec57388**)((char*)&AsWPuppet(self)->namedResources));
    }
    if (AsWPuppet(self)->layerMatrices) {
        __dla__FPv(AsWPuppet(self)->layerMatrices);
        AsWPuppet(self)->layerMatrices = 0;
    }
    if (AsWPuppet(self)->layerScratch) {
        __dl__FPv(AsWPuppet(self)->layerScratch);
        AsWPuppet(self)->layerScratch = 0;
    }
    if (AsWPuppet(self)->layerController) {
        if (AsWPuppet(self)->layerController)
            ((PuppetDeletingDispatch*)AsWPuppet(self)->layerController)->destroy(1);
        AsWPuppet(self)->layerController = 0;
    }
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
extern void RBTree_RebalanceAfterErase(void*, void*);
extern void BaseObject_dtor(void*, int);
#pragma opt_common_subs off
// Select the common byte count for destructor-time name comparisons.
static inline const unsigned int& minFC28(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
#pragma opt_propagation off
// Preserve the destructor's inline integer zero-test sequence.
static inline int notFC28(int value) {
    return value == 0;
}
// Order equal-prefix resource names by length.
static inline int sizesFC28(unsigned int a, unsigned int b) {
    if (a < b)
        return -1;
    if (a == b)
        return 0;
    return 1;
}
// Advance to the in-order successor of a string-tree node.
static inline void* nextFC28(void* node) {
    void* right = *(void**)((char*)node + 4);
    if (right) {
        void* left;
        while ((left = *(void**)right) != 0)
            right = left;
        node = right;
    } else {
        void* parent;
        while (node != *(void**)(parent = (void*)(*(unsigned int*)((char*)node + 8) & ~1u)))
            node = parent;
        node = parent;
    }
    return node;
}
// Compare an inline/heap tree key with a caller-supplied byte span.
static inline int cmpFC28(const void* s, unsigned int len, const void* data) {
    unsigned int lenCopy = len;
    unsigned int sz = !(*(unsigned int*)s >> 31) ? (*(unsigned char*)s & 0x7F) : ((const MWStringStorage*)s)->length;
    const char* d;
    unsigned int sz2;
    if (!(*(unsigned int*)s >> 31)) {
        d = (const char*)s + 1;
        sz2 = *(unsigned char*)s & 0x7F;
    } else {
        d = ((const MWStringStorage*)s)->data;
        sz2 = ((const MWStringStorage*)s)->length;
    }
    unsigned int n = minFC28(sz, (unsigned int)sz2);
    int cmp = memcmp((void*)d, (void*)data, minFC28(lenCopy, n));
    if (cmp == 0) {
        if (n < len)
            return -1;
        if (n == len)
            return 0;
        return 1;
    }
    return cmp;
}
#pragma opt_loop_invariants off
// Free list nodes and material storage; child/cache payloads remain borrowed.
#define ReleasePuppetChildAndCacheStorage(self)                                                    \
    {                                                                                              \
        if ((char*)&AsWPuppet(self)->childCount)                                                   \
            if ((char*)&AsWPuppet(self)->childCount)                                               \
                if ((char*)&AsWPuppet(self)->childCount)                                           \
                    if ((char*)&AsWPuppet(self)->childCount) {                                     \
                        void* list = (char*)&AsWPuppet(self)->childTail;                           \
                        void* n = AsWPuppet(self)->childHead;                                      \
                        if (n != list) {                                                           \
                            void* nValue = n;                                                      \
                            void* tail = *(void**)list;                                            \
                            void* nValue2 = n;                                                     \
                            *(void**)((char*)*(void**)nValue + 4) = *(void**)((char*)tail + 4);    \
                            *(void**)(*(void**)((char*)tail + 4)) = *(void**)nValue2;              \
                            while (n != list) {                                                    \
                                void* nValue3 = n;                                                 \
                                void* cur = nValue3;                                               \
                                n = *(void**)((char*)n + 4);                                       \
                                appFreeUni__FPv(cur);                                              \
                                --*(unsigned int*)&AsWPuppet(self)->childCount;                    \
                            }                                                                      \
                        }                                                                          \
                    }                                                                              \
        if ((char*)&AsWPuppet(self)->materials)                                                    \
            if ((char*)&AsWPuppet(self)->materials)                                                \
                if ((char*)&AsWPuppet(self)->materials)                                            \
                    if ((char*)&AsWPuppet(self)->materials) {                                      \
                        if (AsWPuppet(self)->materials) {                                          \
                            AsWPuppet(self)->materialCount =                                       \
                                *(volatile unsigned int*)&AsWPuppet(self)->materialCount -         \
                                *(volatile unsigned int*)&AsWPuppet(self)->materialCount;          \
                            appFreeUni__FPv(AsWPuppet(self)->materials);                           \
                        }                                                                          \
                    }                                                                              \
        if ((char*)&AsWPuppet(self)->cachedBoneCount)                                              \
            if ((char*)&AsWPuppet(self)->cachedBoneCount)                                          \
                if ((char*)&AsWPuppet(self)->cachedBoneCount)                                      \
                    if ((char*)&AsWPuppet(self)->cachedBoneCount) {                                \
                        void* list = (char*)&AsWPuppet(self)->cachedBoneTail;                      \
                        void* n = AsWPuppet(self)->cachedBoneHead;                                 \
                        if (n != list) {                                                           \
                            void* tail = *(void**)list;                                            \
                            __typeof__(n) nCopy = n;                                               \
                            __typeof__(nCopy) nCopyCopy = nCopy;                                   \
                            *(void**)((char*)*(void**)nCopyCopy + 4) = *(void**)((char*)tail + 4); \
                            *(void**)(*(void**)((char*)tail + 4)) = *(void**)n;                    \
                            while (n != list) {                                                    \
                                void* cur = n;                                                     \
                                n = *(void**)((char*)n + 4);                                       \
                                appFreeUni__FPv(cur);                                              \
                                --*(unsigned int*)&AsWPuppet(self)->cachedBoneCount;               \
                            }                                                                      \
                        }                                                                          \
                    }                                                                              \
    }

// Release resources and name registration, then destroy owned containers and optionally this puppet.
void* WPuppet_dtor(void* self, int deleting) {
    char local[4];
    // Phase 1: release resources while the name registry still describes sharing.
    if (self) {
        extern char __vt__7WPuppet[];
        *(void**)self = __vt__7WPuppet;
        WPuppet_ReleaseResources((char*)self);
        // Phase 2: find the exact basename key, decrement its extra-use count or
        // erase the tree node when that count is already zero. Do not invert this test.
        if (AsWPuppet(self)->hasResourceName) {
            if (AsWPuppet(self)->resourceName) {
                volatile char uninit;
                struct String {
                    unsigned int size;
                    char local[4];
                    unsigned int capacity;
                } query = {};
                const char* name = (const char*)AsWPuppet(self)->resourceName;
                unsigned int length = strlen(name);

                String_ReserveCapacity(&query, length);
                local[0] = uninit;
                unsigned int lengthValue = length;
                ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(
                    &query, 0, 0, name, name + lengthValue, &local[0]);
                int heap;
                void* node;
                const char* data;
                int size;
                void* found;
                unsigned int isHeap;
                unsigned int inlineLen;
                unsigned int len;
                heap = (*(unsigned int*)&query >> 31);
                int heapValue = heap;
                node = *(void**)(s_puppetResourceNameRefCounts + 4);
                data = *(const char**)((char*)&query + 8);
                size = *(unsigned int*)((char*)&query + 4);
                found = (void*)(4 + s_puppetResourceNameRefCounts);
                isHeap = notFC28(notFC28(heapValue));
                inlineLen = *(unsigned char*)&query & 0x7f;
                while (node) {
                    const void* text;
                    if (isHeap == 0) {
                        len = inlineLen;
                        text = (char*)&query + 1;
                    } else {
                        text = data;
                        __typeof__(size) sizeCopy = size;
                        __typeof__(sizeCopy) sizeCopyCopy = sizeCopy;
                        len = sizeCopyCopy;
                    }
                    if (cmpFC28((char*)&((ResourceStringNode*)node)->stringFlags, len, text) >= 0) {
                        found = node;
                        node = *(void**)node;
                    } else
                        node = *(void**)((char*)node + 4);
                }
                if (found != (void*)(s_puppetResourceNameRefCounts + 4)) {
                    unsigned int sz2;
                    const void* text;
                    unsigned int len;
                    if (!(*(unsigned int*)&((ResourceStringNode*)found)->stringFlags >> 31)) {
                        text = ((PuppetNameNodeView*)found)->key.small.text;
                        len = *(unsigned char*)&((ResourceStringNode*)found)->stringFlags & 0x7f;
                    } else {
                        text = *(void**)&((ResourceStringNode*)found)->heapData;
                        len = *(unsigned int*)&((ResourceStringNode*)found)->heapLength;
                    }
                    __typeof__(len) lenCopy = len;
                    __typeof__(inlineLen) inlineLenCopy = inlineLen;
                    unsigned int sz = isHeap == 0 ? inlineLenCopy : *(unsigned int*)((char*)&query + 4);
                    const void* d;

                    if (isHeap == 0) {
                        sz2 = inlineLen;
                        d = (char*)&query + 1;
                    } else {
                        d = *(void**)((char*)&query + 8);
                        sz2 = *(unsigned int*)((char*)&query + 4);
                    }
                    unsigned int n = minFC28(sz, (unsigned int)sz2);
                    int cmp = memcmp((void*)d, (void*)text, minFC28((lenCopy), n));
                    if (0 == cmp) {
                        if (n < len)
                            cmp = -1;
                        else if (n == len)
                            cmp = 0;
                        else
                            cmp = 1;
                    }
                    if (cmp < 0)
                        found = (void*)(4 + s_puppetResourceNameRefCounts);
                }
                if (heap)
                    appFreeUni__FPv(*(void**)((char*)&query + 8));
                if ((void*)(s_puppetResourceNameRefCounts + 4) != found) {

                    if ((*(unsigned int*)&((ResourceStringNode*)found)->value))
                        --*(unsigned int*)&((ResourceStringNode*)found)->value;
                    else {
                        if (found == *(void**)(s_puppetResourceNameRefCounts + 8)) {
                            *(void**)(s_puppetResourceNameRefCounts + 8) = nextFC28(found);
                        }
                        RBTree_RebalanceAfterErase(found, *(void**)(4 + s_puppetResourceNameRefCounts));
                        if ((char*)&((ResourceStringNode*)found)->stringFlags)
                            if ((char*)&((ResourceStringNode*)found)->stringFlags)
                                if (*(unsigned int*)&((ResourceStringNode*)found)->stringFlags >> 31)
                                    appFreeUni__FPv(*(void**)&((ResourceStringNode*)found)->heapData);
                        appFreeUni__FPv(found);
                        --*(unsigned int*)s_puppetResourceNameRefCounts;
                    }
                }
            }
        }
        // Phase 3: free child/cache list NODES and material storage. These loops do
        // not delete child puppets or cached bone payloads; their pointers are borrowed.
        ReleasePuppetChildAndCacheStorage(self);
        // Phase 4: drain instance allocation rings under their recursive mutexes,
        // then release their storage arrays. Object memory is freed only if deleting>0.
        if ((char*)&AsWPuppet(self)->animationInstances) {
            WInstanceLock outer((char*)&AsWPuppet(self)->animationInstances.mutex);
            if (AsWPuppet(self)->animationInstances.ownedHead) {
                void* nxt;
                void* lock80 = (char*)&AsWPuppet(self)->animationInstances.mutex;
                void* node;
                while ((node = AsWPuppet(self)->animationInstances.ownedHead) != 0) {
                    OSLockMutex((OSMutex*)lock80);
                    ((PuppetInstanceNode*)((PuppetInstanceNode*)node)->next)->prev = ((PuppetInstanceNode*)node)->prev;
                    ((PuppetInstanceNode*)((PuppetInstanceNode*)node)->prev)->next = ((PuppetInstanceNode*)node)->next;
                    nxt = *(void**)&((PuppetInstanceNode*)node)->next;
                    if (nxt == node)
                        nxt = 0;
                    OSUnlockMutex((OSMutex*)lock80);
                    AsWPuppet(self)->animationInstances.ownedHead = nxt;
                    appFreeUni__FPv(node);
                }
            }
            if (AsWPuppet(self)->animationInstances.storage) {
                appFreeUni__FPv(AsWPuppet(self)->animationInstances.storage);
                AsWPuppet(self)->animationInstances.storage = 0;
            }
        }
        if ((char*)&AsWPuppet(self)->ownedInstances) {
            WInstanceLock outer((char*)&AsWPuppet(self)->ownedInstances.mutex);
            if (AsWPuppet(self)->ownedInstances.ownedHead) {
                void* nxt;
                void* lock18 = (char*)&AsWPuppet(self)->ownedInstances.mutex;
                void* node;
                while ((node = AsWPuppet(self)->ownedInstances.ownedHead) != 0) {
                    OSLockMutex((OSMutex*)lock18);
                    ((PuppetInstanceNode*)((PuppetInstanceNode*)node)->next)->prev = ((PuppetInstanceNode*)node)->prev;
                    ((PuppetInstanceNode*)((PuppetInstanceNode*)node)->prev)->next = ((PuppetInstanceNode*)node)->next;
                    nxt = *(void**)&((PuppetInstanceNode*)node)->next;
                    if (nxt == node)
                        nxt = 0;
                    OSUnlockMutex((OSMutex*)lock18);
                    AsWPuppet(self)->ownedInstances.ownedHead = nxt;
                    appFreeUni__FPv(node);
                }
            }
            if (AsWPuppet(self)->ownedInstances.storage) {
                appFreeUni__FPv(AsWPuppet(self)->ownedInstances.storage);
                AsWPuppet(self)->ownedInstances.storage = 0;
            }
        }
        if (self)
            BaseObject_dtor(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma opt_propagation reset

extern const float kFloat_0_32_PetMesh;
extern void IPetMesh_ScaleResource(void*, float);
extern void* gtl_ResourceFile_GetChunkElement(void*, void*, int, int);
extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void PuppetMaterialVector_InsertDirectRange(void*, void*, void*, void*, void*);
struct Load55474Query {
    unsigned int w[7];
};
extern const Load55474Query s_puppetModelCreateOptions;
struct Load55474Material {
    unsigned int a, b, c;
    unsigned char d[12];
    unsigned int e;
};
// MESH boundary schema (native Wii big-endian words, 4-byte aligned). Offsets
// below are RELATIVE, not relocated pointers. Find55474 returns the chunk base:
//   chunk+8 -> descriptor offset from chunk; descriptor+0 -> archive-name word
//   passed verbatim to CreateFromMemory; descriptor+4 -> resource offset FROM
//   CHUNK (not descriptor). The resource's +C is its chunk-table runtime view.
// GetChunkElement returns borrowed records in that resource. Relationship map:
// kChunkModelLayers {u16 groupCount,+2 u16 firstGroup} -> kChunkMeshGroups
// {u32 firstPrimitive,count} -> kChunkPrimitives {+E u16 candidateTriangleCount,
// +10 u16 indexHeader} -> index header
// {reserved word,+4 u32 byteOffset} (8-byte stride) -> u16 index stream.
// Each candidate uses indices[m..m+2]; repeated indices make it degenerate.
// No nested bounds/version validation: require every referenced span in length,
// nonzero traversed chunk sizes, and candidateCount+2 indices per primitive.
struct PuppetMeshDescriptor {
    unsigned int archiveNameWord, resourceOffset;
};
struct PuppetIndexHeader {
    unsigned int reserved, indexByteOffset;
};
struct Load55474Record {
    unsigned int w[60];
};
struct Load55474Vector {
    void* data;
    unsigned int size, capacity;
    // Start an empty material-buffer owner during model loading.
    Load55474Vector() : data(0), size(0), capacity(0) {}
    // Release owned record storage after reserve failure or a successful swap.
    ~Load55474Vector() {
        if (data) {
            unsigned int n = *(volatile unsigned int*)&size;
            volatile char allocator = 0;
            size = n - n;
            appFreeUni__FPv(data);
        }
    }
};
// Locate the MESH chunk in the size-prefixed input archive.
static inline void* Find55474(const char* name, unsigned int length) {
    unsigned int offset = 0;
    const char* signature = puppetPreloadStrings.meshTag;
    while (offset < length) {
        void* block = (char*)name + offset;
        if (memcmp((void*)signature, block, 4) == 0)
            return block;
        offset += *(unsigned int*)((char*)block + 4);
    }
    return 0;
}

#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma opt_unroll_loops off
#pragma opt_loop_count off
// Load the MESH model, count triangles, allocate material records, and copy material metadata and visibility masks.
// Despite const char* name, the archive buffer must be WRITABLE: ScaleResource
// changes embedded geometry by kFloat_0_32_PetMesh before resource creation.
// The caller must keep it live for any resource borrowing its records. Missing
// MESH returns without changes; malformed nested offsets and allocation/resource
// failures have no complete rollback path. This is a trusted-asset loader.
void WPuppet_LoadModelAndMaterialsFromChunks(void* puppet, const char* name, unsigned int length) {
    // Phase 1: locate MESH, scale its archive geometry, and count nondegenerate
    // sliding index triples. name is the borrowed chunk buffer, not a C-string path.
    void* self = puppet;
    void* block = Find55474(name, length);
    if (0 == block)
        return;
    int base = ((PuppetChunkHeader*)block)->dataOffset;
    int baseValue = base;
    void* archive = (char*)block + baseValue;
    void* records = (char*)block + ((PuppetMeshDescriptor*)archive)->resourceOffset;
    void* archiveName = (void*)((PuppetMeshDescriptor*)archive)->archiveNameWord;
    IPetMesh_ScaleResource(records, kFloat_0_32_PetMesh);
    int layerCount = gtl_ResourceFile_GetChunkElementCount(records, *(void**)&((PuppetArchiveView*)records)->chunkTable,
                                                           kChunkModelLayers);
    void* info = gtl_ResourceFile_GetChunkElement(records, *(void**)&((PuppetArchiveView*)records)->chunkTable,
                                                  kChunkIndexData, 0);
    AsWPuppet(self)->triangleCount = 0;
    void* groups = gtl_ResourceFile_GetChunkElement(records, *(void**)&((PuppetArchiveView*)records)->chunkTable,
                                                    kChunkIndexHeaders, 0);
    for (unsigned int i = 0; i < (unsigned int)layerCount; ++i) {
        void* layer = gtl_ResourceFile_GetChunkElement(records, *(void**)&((PuppetArchiveView*)records)->chunkTable,
                                                       kChunkModelLayers, i);
        for (int j = 0; j < ((PuppetModelLayerView*)layer)->groupCount; ++j) {
            void* group =
                gtl_ResourceFile_GetChunkElement(records, *(void**)&((PuppetArchiveView*)records)->chunkTable,
                                                 kChunkMeshGroups, ((PuppetModelLayerView*)layer)->firstGroup + j);
            for (unsigned int k = 0; k < ((PuppetMeshGroupView*)group)->primitiveCount; ++k) {
                void* item = gtl_ResourceFile_GetChunkElement(
                    records, *(void**)&((PuppetArchiveView*)records)->chunkTable, kChunkPrimitives,
                    ((PuppetMeshGroupView*)group)->firstPrimitive + k);
                if (info) {
                    void* ids = (char*)groups + *(unsigned short*)&((PuppetPrimitiveView*)item)->indexHeader * 8;
                    unsigned short* indices =
                        (unsigned short*)((char*)info + ((PuppetIndexHeader*)ids)->indexByteOffset);
                    for (int m = 0; m < *(unsigned short*)&((PuppetPrimitiveView*)item)->triangleCount; ++m) {
                        unsigned short b = indices[1];
                        unsigned short a = indices[0];
                        if (a != b) {
                            unsigned short c = indices[2];
                            if ((unsigned short)a != c && (unsigned short)b != (unsigned short)c)
                                ++AsWPuppet(self)->triangleCount;
                        }
                        indices = 1 + indices;
                    }
                } else {
                    AsWPuppet(self)->triangleCount += *(unsigned short*)&((PuppetPrimitiveView*)item)->triangleCount;
                }
            }
        }
    }
    Load55474Query query = s_puppetModelCreateOptions;
    void* old = AsWPuppet(self)->modelResource;
    if (old) {
        if (--*(unsigned int*)old == 0) {
            if (old)
                ((void (*)(void*, short))(*(void***)((char*)old + 4))[2])(old, 1);
        }
        AsWPuppet(self)->modelResource = 0;
    }
    gtl::IGenericModelingResource::CreateFromMemory(records, (int)(long)archiveName,
                                                    (void**)&AsWPuppet(self)->modelResource, &query);
    void* resource = AsWPuppet(self)->modelResource;
    if (0 == resource) {
        if (AsWPuppet(self)->loadFlags == kContainerLoadMode) {
            void* parent = AsWPuppet(self)->childCount == 0 ? 0 : ((WPuppetListNode*)AsWPuppet(self)->childHead)->value;
            if (parent)
                resource = AsWPuppet(parent)->modelResource;
        }
    }
    unsigned int n =
        resource ? GetChunkElementCount__Q23gtl20IGenericResourceFileFi(resource, kChunkSkinTransforms) : 0;
    if (g_puppetTransformBufferCapacity < n) {
        if (g_puppetMatrixScratch) {
            __dla__FPv(g_puppetMatrixScratch);
            g_puppetMatrixScratch = 0;
        }
        g_puppetTransformBufferCapacity = 0;
        __typeof__(n) nCopy = n;
        __typeof__(nCopy) nCopyCopy = nCopy;
        __typeof__(nCopyCopy) nCopyCopyCopy = nCopyCopy;
        void* a = __nwa__FUl(nCopyCopyCopy << 6);
        __typeof__(a) aCopy = a;
        __typeof__(aCopy) aCopyCopy = aCopy;
        __typeof__(aCopyCopy) aCopyCopyCopy = aCopyCopy;
        g_puppetMatrixScratch = aCopyCopyCopy;
        if (a)
            g_puppetTransformBufferCapacity = n;
    }
    // Phase 2: reset material size, reserve record storage with an unwind guard,
    // then copy material records from the new model resource into that storage.
    volatile char clearAllocator = 0;
    int oldSize = AsWPuppet(self)->materialCount;
    int oldSizeValue = oldSize;
    AsWPuppet(self)->materialCount -= oldSizeValue;
    unsigned int size =
        GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource, kChunkMaterialRecords);
    void* source = GetChunkElement__Q23gtl20IGenericResourceFileFii(AsWPuppet(self)->modelResource,
                                                                    (const char*)kChunkMaterialRecords, 0);
    if (size > AsWPuppet(self)->materialCapacity) {
        Load55474Vector temp;
        volatile char alloc = 0;
        if (size > 0x1111111) {
            char sa = 0;
            const char* message = puppetPreloadStrings.vectorLengthError;

            struct {
                void* vtable;
                char* text;
                void* inner;
            } error;
            error.vtable = &__vt__Q23std11logic_error;

            volatile char ea = 0;
            int len = strlen(message) + 1;
            int product = len * 2;
            int lenValue2 = len;
            char* text = (char*)__nwa__FUl(product - lenValue2);
            error.text = text;
            {
                Copy502StringGuard guard = {text, &sa};
                void* inner = __nw__FUl(16);
                if (inner) {
                    *(int*)((char*)inner + 4) = 1;
                    *(int*)((char*)inner + 8) = 1;
                    *(void**)inner = &s_sharedCharDeleterVtable;
                    *(void**)((char*)inner + 12) = text;
                }
                error.inner = inner;
                guard.text = 0;
            }
            __typeof__(error.text) errorTextCopy = error.text;
            __typeof__(error.text) errorTextCopy2 = error.text;
            __typeof__(error.text) errorTextCopy2Copy = errorTextCopy2;
            __typeof__(errorTextCopy) errorTextCopy2CopyCopy = errorTextCopy2Copy;
            strcpy(errorTextCopy2CopyCopy, message);
            error.vtable = &__vt__Q23std12length_error;
            __throw((const char*)(puppetPreloadStrings.lengthErrorType), &error, __dt__Q23std12length_errorFv);
        }
        __typeof__(size) sizeCopy = size;
        __typeof__(sizeCopy) sizeCopyCopy = sizeCopy;
        temp.data = appMallocDummy__FUl(sizeCopyCopy * kMaterialBytes);
        __typeof__(size) sizeCopy2 = size;
        __typeof__(sizeCopy2) sizeCopy2Copy = sizeCopy2;
        Copy502Record* from = (Copy502Record*)AsWPuppet(self)->materials;
        temp.capacity = sizeCopy2Copy;
        Copy502Record* end = from + AsWPuppet(self)->materialCount;
        __typeof__(temp.size) tempSizeCopy = temp.size;
        volatile char ca = 0;
        Copy502Record* to = (Copy502Record*)temp.data + tempSizeCopy;
        while (from < end) {
            if (to) {
                try {
                    *to = *from;
                } catch (...) {
                    throw;
                }
            }
            ++from;
            ++temp.size;
            ++to;
        }
        __typeof__(temp.data) tempDataCopy = temp.data;
        *(volatile unsigned int*)&AsWPuppet(self)->materialCount = 0;
        void* old = AsWPuppet(self)->materials;
        void* fresh = tempDataCopy;
        int cap = AsWPuppet(self)->materialCapacity;

        unsigned int os = *(volatile unsigned int*)&AsWPuppet(self)->materialCount;

        temp.data = old;
        volatile char swapAllocator = 0;
        AsWPuppet(self)->materialCapacity = (temp.capacity);
        temp.capacity = cap;
        AsWPuppet(self)->materials = fresh;
        AsWPuppet(self)->materialCount = (temp.size);
        unsigned int osValue = os;
        temp.size = osValue;
    }
    void* position = AsWPuppet(self)->materials;
    void* volatile iterator2 = position;
    void* volatile iterator1 = position;
    volatile char iteratorAllocator = 0;

    __typeof__(size) sizeCopy3 = size;
    __typeof__(sizeCopy3) sizeCopy3Copy = sizeCopy3;
    char dummy = 0;
    PuppetMaterialVector_InsertDirectRange((char*)&AsWPuppet(self)->materials, position, source,
                                           (char*)source + sizeCopy3Copy * kMaterialBytes, &dummy);
    // Phase 3: replace the separate 0x1c-byte material metadata array and its masks.
    size = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource, kChunkMaterialMetadata);
    void* dst = AsWPuppet(self)->materialMetadata;
    if (dst) {
        __dla__FPv(dst);
        AsWPuppet(self)->materialMetadata = 0;
    }
    __typeof__(size) sizeCopy4 = size;
    __typeof__(sizeCopy4) sizeCopy4Copy = sizeCopy4;
    dst = __nwa__FUl(sizeCopy4Copy * 0x1C);
    AsWPuppet(self)->materialMetadata = dst;
    Load55474Material* src = (Load55474Material*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
        AsWPuppet(self)->modelResource, (const char*)kChunkMaterialMetadata, 0);
    for (unsigned int i = 0; size > i; i++) {
        Load55474Material* out = ((Load55474Material*)AsWPuppet(self)->materialMetadata) + i;
        out->a = src->a;
        out->b = src->b;
        out->c = src->c;
        out->d[0] = src->d[0];
        out->d[1] = src->d[1];
        out->d[2] = src->d[2];
        __typeof__(src->d[5]) srcDCopy = src->d[5];
        out->d[3] = src->d[3];
        out->d[4] = src->d[4];
        __typeof__(srcDCopy) srcDCopy2 = src->d[5];
        out->d[5] = srcDCopy2;
        __typeof__(src->d[6]) srcDCopy3 = src->d[6];
        __typeof__(srcDCopy3) srcDCopy4 = src->d[6];
        out->d[6] = srcDCopy4;
        out->d[7] = src->d[7];
        out->d[8] = src->d[8];
        out->d[9] = src->d[9];
        out->d[10] = src->d[10];
        out->d[11] = src->d[11];
        out->e = src->e;
        src += 1;
    }
    dst = AsWPuppet(self)->modelLayerVisibilityWords;
    if (dst) {
        __dla__FPv(dst);
        AsWPuppet(self)->modelLayerVisibilityWords = 0;
    }
    __typeof__(layerCount) layerCountCopy = layerCount;
    __typeof__(layerCountCopy * 4) layerCountCopyCopy = layerCountCopy * 4;
    __typeof__(layerCountCopyCopy) layerCountCopyCopy2 = 4 * layerCountCopy;
    __typeof__(layerCountCopyCopy2) layerCountCopyCopy3 = layerCountCopy * 4;
    __typeof__(4 * layerCountCopy) layerCountCopyCopy4 = 4 * layerCountCopy;
    __typeof__(layerCountCopyCopy4) layerCountCopyCopy3Copy = layerCountCopyCopy3;
    dst = __nwa__FUl(layerCountCopyCopy3Copy);
    AsWPuppet(self)->modelLayerVisibilityWords = dst;
    for (unsigned int i = 0; i < (unsigned int)layerCount; i++)
        ((unsigned int*)AsWPuppet(self)->modelLayerVisibilityWords)[i] = 1;
}
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma opt_unroll_loops reset
#pragma opt_loop_count reset

extern int s_puppetResourceInitialRefCount;
extern void* NamedObjectMap_InsertNode(void*, void*, int, int, void*);
extern void* StringTree_FindInsertionPosition(void*, void*, void**, unsigned char*, unsigned char*);
#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma opt_propagation off
// Select the shorter byte count for shared-name registration comparisons.
static inline const unsigned int& f74_min(const unsigned int& a, const unsigned int& b) {
    const unsigned int* result = &b;
    if (a < b)
        result = &a;
    return *result;
}
// Read the inline-length byte or the heap-length word of a string.
static inline unsigned int f74_len(const void* s) {
    unsigned int result;
    if (!(*(const unsigned int*)s >> 31)) {
        result = *(const unsigned char*)s & 0x7f;
    } else {
        result = *(const unsigned int*)((const char*)s + 4);
    }
    return result;
}
// Return the inline bytes or heap-data pointer of a string.
static inline const void* f74_data(const void* s) {
    const void* result;
    if (!(*(const unsigned int*)s >> 31)) {
        result = (const char*)s + 1;
    } else {
        result = *(void* const*)((const char*)s + 8);
    }
    return result;
}
// Compare a stored string with a supplied text span.
static inline int f74_cmp(const void* a, const void* bdata, unsigned int blen) {
    unsigned int l1 = f74_len(a);
    unsigned int l2 = f74_len(a);
    const void* d = f74_data(a);
    unsigned int t = f74_min(l1, l2);
    int cmp = memcmp((void*)d, (void*)bdata, (int)f74_min(blen, t));
    if (cmp == 0) {
        if (t < blen)
            return -1;
        if (t == blen)
            return 0;
        return 1;
    }
    return cmp;
}
// Compare a second registration key with the supplied text span.
static inline int f74_cmpB(const void* a, const void* bdata, unsigned int blen) {
    unsigned int l1 = f74_len(a);
    unsigned int l2 = f74_len(a);
    const void* d = f74_data(a);
    unsigned int t = f74_min(l1, l2);
    int cmp = memcmp((void*)d, (void*)bdata, (int)f74_min(blen, t));
    if (cmp == 0) {
        if (t < blen)
            return -1;
        if (t == blen)
            return 0;
        return 1;
    }
    return cmp;
}
// Compare a query against a decoded tree key while preserving its original inlined loads.
static inline int f74_cmpC(const void* a, const void* bdata, unsigned int blen) {
    unsigned int blength = blen;
    unsigned int l1;
    if (!(*(const unsigned int*)a >> 31))
        l1 = *(const unsigned char*)a & 0x7f;
    else
        l1 = *(const unsigned int*)((const char*)a + 4);
    unsigned int l1b = l1;
    unsigned int l2;
    const void* d;
    if (!(*(const unsigned int*)a >> 31)) {
        d = (const char*)a + 1;
        l2 = *(const unsigned char*)a & 0x7f;
    } else {
        d = *(void* const*)((const char*)a + 8);
        l2 = *(const unsigned int*)((const char*)a + 4);
    }
    unsigned int l2b = l2;
    unsigned int t = f74_min(l1b, l2b);
    int cmp = memcmp((void*)d, (void*)bdata, (int)f74_min(blen, t));
    if (cmp == 0) {
        if (t < blength)
            return -1;
        if (t == blength)
            return 0;
        return 1;
    }
    return cmp;
}
// Compare the second query against a decoded tree key.
static inline int f74_cmpD(const void* a, const void* bdata, unsigned int blen) {
    unsigned int blength = blen;
    unsigned int l1;
    if (!(*(const unsigned int*)a >> 31))
        l1 = *(const unsigned char*)a & 0x7f;
    else
        l1 = *(const unsigned int*)((const char*)a + 4);
    unsigned int l1b = l1;
    unsigned int l2;
    const void* d;
    if (!(*(const unsigned int*)a >> 31)) {
        d = (const char*)a + 1;
        l2 = *(const unsigned char*)a & 0x7f;
    } else {
        d = *(void* const*)((const char*)a + 8);
        l2 = *(const unsigned int*)((const char*)a + 4);
    }
    unsigned int l2b = l2;
    unsigned int t = f74_min(l1b, l2b);
    int cmp = memcmp((void*)d, (void*)bdata, (int)f74_min(blen, t));
    if (cmp == 0) {
        if (t < blength)
            return -1;
        if (t == blength)
            return 0;
        return 1;
    }
    return cmp;
}
// Construct the temporary resource-basename string used as a tree key.
static inline void f74_make(unsigned int* s, const char* name) {
    s[0] = 0;
    s[1] = 0;
    s[2] = 0;
    unsigned int length = strlen(name);
    String_ReserveCapacity(s, length);
    char local __attribute__((aligned(4)));
    volatile char c __attribute__((aligned(4)));
    local = c;
    ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(s, 0, 0, name, name + length,
                                                                                      &local);
}
// Copy a registration string without sharing a heap allocation.
static inline void f74_copy(unsigned int* d, const unsigned int* s) {
    unsigned int* dest = d;
    if (!(s[0] >> 31)) {
        dest[0] = s[0];
        dest[1] = s[1];
        dest[2] = s[2];
    } else {
        dest[0] = 0;
        dest[1] = 0;
        dest[2] = 0;
        String_ReserveCapacity(dest, s[1]);
        char local __attribute__((aligned(4)));
        volatile char c __attribute__((aligned(4)));
        local = c;
        ((void (*)(void*, int, int, const char*, const char*, char*))String_ReplaceRange)(
            dest, 0, 0, (const char*)s[2], (const char*)s[2] + s[1], &local);
    }
}
struct F74Str {
    unsigned int hdr;
    unsigned int len;
    unsigned int data;
};
struct F74Pair {
    unsigned int hdr;
    unsigned int len;
    unsigned int data;
    unsigned int value;
};
struct F74Dummy {
    volatile char value;
    char pad[3];
} __attribute__((aligned(4)));
struct F74Flags {
    unsigned char first, second;
    char pad[2];
} __attribute__((aligned(4)));
#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma opt_propagation off

// Compare a borrowed tree payload to an owned query key; both remain live in the caller.
static inline int CompareResourceNamePair(F74Str& key, char*& treePayload, unsigned int& queryLength,
                                          unsigned int& storedLength, unsigned int& storedSize,
                                          unsigned int& comparisonBytes) {
    unsigned int queryDecodedLength;
    const void* queryText;
    if (!(key.hdr >> 31)) {
        queryText = (char*)&key + 1;
        queryDecodedLength = *(unsigned char*)&key & 0x7f;
    } else {
        queryText = *(void**)((char*)&key + 8);
        queryDecodedLength = key.len;
    }
    queryLength = queryDecodedLength;
    unsigned int headerLength;
    if (!(*(unsigned int*)treePayload >> 31))
        headerLength = *(unsigned char*)treePayload & 0x7f;
    else
        headerLength = *(unsigned int*)(treePayload + 4);
    storedLength = headerLength;
    unsigned int decodedLength;
    const void* storedText;
    if (!(*(unsigned int*)treePayload >> 31)) {
        storedText = treePayload + 1;
        decodedLength = *(unsigned char*)treePayload & 0x7f;
    } else {
        storedText = *(void**)(treePayload + 8);
        decodedLength = *(unsigned int*)(treePayload + 4);
    }
    storedSize = decodedLength;
    comparisonBytes = f74_min(storedLength, storedSize);
    int order = memcmp((void*)storedText, (void*)queryText, f74_min(queryLength, comparisonBytes));
    if (order == 0) {
        if (comparisonBytes < queryDecodedLength)
            order = -1;
        else if (comparisonBytes == queryDecodedLength)
            order = 0;
        else
            order = 1;
    }

    return order;
}

// Find or insert a resource-name treePayload while retaining the original temporary-string lifetimes.
// key and insertionPair own independent string copies; StringCleanup releases both
// after NamedObjectMap_InsertNode copies the treePayload. treePayload borrows the tree's payload.
#define FIND_OR_INSERT_RESOURCE_NAME(key, insertionPair, candidate, queryLength, storedLength, storedSize,           \
                                     comparisonBytes)                                                                \
    do {                                                                                                             \
        if (candidate) {                                                                                             \
            treePayload = (char*)&((ResourceStringNode*)candidate)->stringFlags;                                     \
            if (CompareResourceNamePair(key, treePayload, queryLength, storedLength, storedSize, comparisonBytes) >= \
                0)                                                                                                   \
                break;                                                                                               \
        }                                                                                                            \
        {                                                                                                            \
            f74_copy((unsigned int*)&insertionPair, (const unsigned int*)&key);                                      \
            StringCleanup pairCleanup = {&insertionPair};                                                            \
            insertionPair.value = s_puppetResourceInitialRefCount;                                                   \
            treePayload = (char*)NamedObjectMap_InsertNode((void*)s_puppetResourceNameRefCounts, insertionParent,    \
                                                           insertionFlags[0], insertionFlags[1], &insertionPair) +   \
                          0xC;                                                                                       \
        }                                                                                                            \
    } while (0)

// Insert or retain the resource basename in the shared metadata reference-count tree.
int WPuppet_RegisterResourceNameUse(void* self) {
    // Phase 1: build a temporary owned key and find its lower_bound tree candidate.
    struct StringCleanup {
        void* object;
        // Free only queryUsesHeap-backed temporary string storage.
        ~StringCleanup() {
            unsigned int* words = (unsigned int*)object;
            if (words[0] >> 31)
                appFreeUni__FPv(*(void**)(words + 2));
        }
    };
    unsigned int decodedLength;
    void* searchNode;
    char* resourceName;
    const void* queryHeapData;
    unsigned int queryHeapLength;
    int queryUsesHeap;
    unsigned int queryInlineLength;
    void* lowerBoundNode;
    int queryUsesHeapNormalized;
    F74Str lookupKey;
    F74Str firstUseKey;
    F74Str additionalUseKey;
    F74Pair firstUseInsertionPair;
    F74Pair additionalUseInsertionPair;
    unsigned int candidateComparisonBytes, lookupDecodedLength, candidateKeyLength, lookupStoredLength;
    unsigned int nodeStoredLength, queryDecodedLength, nodeDecodedLength, nodeComparisonBytes;
    unsigned int firstComparisonBytes, firstDecodedLength, firstQueryLength, firstStoredLength;
    void* firstUseCandidate;
    unsigned int additionalComparisonBytes, additionalDecodedLength, additionalQueryLength, additionalStoredLength;
    void* additionalUseCandidate;
    resourceName = AsWPuppet(self)->resourceName;
    if (resourceName == 0)
        return 0;
    f74_make((unsigned int*)&lookupKey, resourceName);
    queryUsesHeap = lookupKey.hdr >> 31;
    int heapFlagCopy = queryUsesHeap;
    searchNode = *(void**)(s_puppetResourceNameRefCounts + 4);
    queryHeapData = (const void*)lookupKey.data;
    queryHeapLength = lookupKey.len;
    queryUsesHeapNormalized = PuppetWordIsZero(PuppetWordIsZero(heapFlagCopy));
    lowerBoundNode = (void*)(4 + s_puppetResourceNameRefCounts);
    queryInlineLength = *(unsigned char*)&lookupKey & 0x7f;
    while (searchNode) {
        const void* text;
        if (queryUsesHeapNormalized == 0) {
            decodedLength = queryInlineLength;
            text = (char*)&lookupKey + 1;
        } else {
            text = queryHeapData;
            decodedLength = queryHeapLength;
        }
        queryDecodedLength = decodedLength;
        nodeStoredLength = !(*(unsigned int*)&((ResourceStringNode*)searchNode)->stringFlags >> 31)
                               ? (*(unsigned char*)&((ResourceStringNode*)searchNode)->stringFlags & 0x7f)
                               : *(unsigned int*)&((ResourceStringNode*)searchNode)->heapLength;
        const void* d;
        unsigned int sz2;
        if (!(*(unsigned int*)&((ResourceStringNode*)searchNode)->stringFlags >> 31)) {
            d = ((PuppetNameNodeView*)searchNode)->key.small.text;
            sz2 = *(unsigned char*)&((ResourceStringNode*)searchNode)->stringFlags & 0x7f;
        } else {
            d = *(void**)&((ResourceStringNode*)searchNode)->heapData;
            sz2 = *(unsigned int*)&((ResourceStringNode*)searchNode)->heapLength;
        }
        nodeDecodedLength = sz2;
        nodeComparisonBytes = f74_min(nodeStoredLength, nodeDecodedLength);
        int cmp = memcmp((void*)d, (void*)text, f74_min(queryDecodedLength, nodeComparisonBytes));
        if (cmp == 0) {
            if (nodeComparisonBytes < decodedLength)
                cmp = -1;
            else if (nodeComparisonBytes == decodedLength)
                cmp = 0;
            else
                cmp = 1;
        }
        if (cmp >= 0) {
            lowerBoundNode = searchNode;
            searchNode = *(void**)searchNode;
        } else
            searchNode = *(void**)((char*)searchNode + 4);
    }
    if (lowerBoundNode != (void*)(4 + s_puppetResourceNameRefCounts)) {
        unsigned int sz2;
        const void* text;
        unsigned int decodedLength;
        if (!(*(unsigned int*)&((ResourceStringNode*)lowerBoundNode)->stringFlags >> 31)) {
            text = ((PuppetNameNodeView*)lowerBoundNode)->key.small.text;
            decodedLength = *(unsigned char*)&((ResourceStringNode*)lowerBoundNode)->stringFlags & 0x7f;
        } else {
            text = *(void**)&((ResourceStringNode*)lowerBoundNode)->heapData;
            decodedLength = *(unsigned int*)&((ResourceStringNode*)lowerBoundNode)->heapLength;
        }
        candidateKeyLength = decodedLength;
        __typeof__(queryInlineLength) inlineLenCopy = queryInlineLength;
        lookupStoredLength = queryUsesHeapNormalized == 0 ? inlineLenCopy : lookupKey.len;
        const void* d;
        if (queryUsesHeapNormalized == 0) {
            sz2 = queryInlineLength;
            d = (char*)&lookupKey + 1;
        } else {
            d = (const void*)lookupKey.data;
            sz2 = lookupKey.len;
        }
        lookupDecodedLength = sz2;
        candidateComparisonBytes = f74_min(lookupStoredLength, lookupDecodedLength);
        int cmp = memcmp((void*)d, (void*)text, f74_min(candidateKeyLength, candidateComparisonBytes));
        if (cmp == 0) {
            if (candidateComparisonBytes < decodedLength)
                cmp = -1;
            else if (candidateComparisonBytes == decodedLength)
                cmp = 0;
            else
                cmp = 1;
        }
        if (cmp < 0)
            lowerBoundNode = (void*)(4 + s_puppetResourceNameRefCounts);
    }
    // The lookup key has served its purpose; branches below own independent keys.
    if (queryUsesHeap)
        appFreeUni__FPv(*(void**)((char*)&lookupKey + 8));
    // Phase 2: first use stores zero; later registrations increment that counter.
    // Each branch destroys its query/treePayload copies after the tree has copied them.
    if ((void*)(4 + s_puppetResourceNameRefCounts) == lowerBoundNode) {
        f74_make((unsigned int*)&firstUseKey, resourceName);
        StringCleanup keyCleanup = {&firstUseKey};
        unsigned char insertionFlags[2] __attribute__((aligned(4)));
        void* insertionParent =
            StringTree_FindInsertionPosition((void*)s_puppetResourceNameRefCounts, &firstUseKey, &firstUseCandidate,
                                             &insertionFlags[0], &insertionFlags[1]);
        char* treePayload;
        // 0x8005A774: Keep the existing resourceName entry or insert it before updating its counter.
        FIND_OR_INSERT_RESOURCE_NAME(firstUseKey, firstUseInsertionPair, firstUseCandidate, firstQueryLength,
                                     firstStoredLength, firstDecodedLength, firstComparisonBytes);
        *(int*)(treePayload + 0xC) = 0;
    } else {
        f74_make((unsigned int*)&additionalUseKey, resourceName);
        StringCleanup keyCleanup = {&additionalUseKey};
        unsigned char insertionFlags[2] __attribute__((aligned(4)));
        void* insertionParent =
            StringTree_FindInsertionPosition((void*)s_puppetResourceNameRefCounts, &additionalUseKey,
                                             &additionalUseCandidate, &insertionFlags[0], &insertionFlags[1]);
        char* treePayload;
        // 0x8005A774: Keep the existing resourceName entry or insert it before updating its counter.
        FIND_OR_INSERT_RESOURCE_NAME(additionalUseKey, additionalUseInsertionPair, additionalUseCandidate,
                                     additionalQueryLength, additionalStoredLength, additionalDecodedLength,
                                     additionalComparisonBytes);
        *(int*)(treePayload + 0xC) = *(int*)(treePayload + 0xC) + 1;
    }
    AsWPuppet(self)->hasResourceName = 1;
    return 1;
}
#undef FIND_OR_INSERT_RESOURCE_NAME
#pragma opt_common_subs reset
#pragma opt_loop_invariants reset
#pragma opt_propagation reset

struct NamedResourceVector {
    void** data;
    unsigned int size;
    unsigned int capacity;
};
struct NamedResourceRecord {
    unsigned int offset;
    unsigned short length;
    unsigned short unk;
};
extern void appFreeUni__FPv(void*);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern int WPuppet_PreloadNamedResource(void*, char*);
// MOTI width/height are legacy field names for first/last frames, divided by 30
// below to produce seconds, not image dimensions.
struct MotionRangeEntry {
    unsigned int nameOffset;
    unsigned int dataOffset;
    unsigned int nameBytesOffset;
    unsigned int firstFrame;
    unsigned int lastFrame;
    unsigned short rangeCode, blendSourceRangeIndex, animationIndex, nameLength;
};
// Runtime 0x34-byte range: +0/+4/+8 seconds (start/end/blendEnd); +C blendMode
// starts zero via Memory_Clear. +18 owns rangeName, +1C owns copied source name.
// +24/+28 preserve MOTI words without interpreting them; +2C rangeCode is copied
// but has no established consumer; +2E indexes the SAME MOTI array for the source
// name (must be <count); +30 selects CPetAnimation's entry in SetAnimationTimeFromRanges.
// PuppetMotionRangeView above is this same allocation, not another range format.
struct Fn80056084Record {
    float startSeconds, endSeconds, blendEndSeconds;
    char pad0[0xc];
    void* rangeName;
    void* blendSourceName;
    char pad1[0x4];
    unsigned int nameOffset;
    unsigned int dataOffset;
    unsigned short rangeCode, blendSourceRangeIndex, animationIndex;
};
struct MotionRangeVector {
    void** data;
    unsigned int size;
    unsigned int capacity;
    // Initialize an empty motion-range pointer vector.
    MotionRangeVector() : data(0), size(0), capacity(0) {}
    // Free the pointer array while leaving pointed-to range objects untouched.
    ~MotionRangeVector() {
        if (data) {
            unsigned int n = size;
            size = n - n;
            appFreeUni__FPv(data);
        }
    }
};

extern float kFloat_30_805DD7C8;

struct Fn56084Temp {
    void* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int extra;
};
extern void __throw(const char*, void*, void*) __attribute__((noreturn));
// Throw a length exception when the motion-range pointer vector would overflow.
static inline void ThrowMotionRangeLengthError() {
    Fn56084Temp msg;
    Fn56084Temp* mp = &msg;
    mp->data = (void*)&__vt__Q23std11logic_error;
    char alloc __attribute__((aligned(4))) = 0;
    const char* message = (const char*)puppetPreloadStrings.vectorLengthError;
    unsigned int len = strlen(message);
    char* text = (char*)__nwa__FUl((len + 1) * 2 - (len + 1));
    mp->size = (unsigned int)text;
    {
        Copy502StringGuard guard;
        guard.text = text;
        guard.allocator = &alloc;
        unsigned int* rep = (unsigned int*)__nw__FUl(0x10);
        if (rep) {
            rep[1] = 1;
            rep[2] = 1;
            rep[0] = (unsigned int)&s_sharedCharDeleterVtable;
            rep[3] = (unsigned int)text;
        }
        mp->capacity = (unsigned int)rep;
        guard.text = 0;
    }
    strcpy((char*)mp->size, message);
    mp->data = (void*)&__vt__Q23std12length_error;
    __throw((const char*)(puppetPreloadStrings.lengthErrorType), mp, (void*)__dt__Q23std12length_errorFv);
}
// Select the larger required capacity increment for the motion-range vector.
template <class T> static inline const T& fn56084max(const T& a, const T& b) {
    return a < b ? b : a;
}
// Locate the MOTI animation-range chunk.
static inline void* FindMotionChunk(const char* data, unsigned int length) {
    unsigned int offset = 0;
    const char* signature = puppetPreloadStrings.motionTag;
    while (offset < length) {
        char* block = (char*)data + offset;
        if (memcmp((void*)signature, block, 4) == 0)
            return block;
        offset += *(unsigned int*)(block + 4);
    }
    return 0;
}

#pragma opt_common_subs off
// Load MOTI ranges, convert frame endpoints to seconds, resolve referenced names, and return the owned range vector.
// archive borrows size bytes; chunk/data/name offsets are MOTI-base-relative,
// entry stride 0x1C; all referenced spans must fit. No malformed-input checks.
// Ownership transfers to WPuppet_LoadFromChunks::animationRanges.
// WPuppet_ReleaseResources (also called by WPuppet_dtor) in its unshared-motion
// branch releases each record's two __nwa strings via __dla,
// then data[0] (the ONE contiguous record array) via __dla, then clears size and
// DeletePuppetPointerVector frees appMallocDummy pointer storage + wrapper.
// Do not delete records separately or use the vector destructor alone: it frees
// only its pointer storage. Shared metadata is released only by the last owner.
// Empty/absent MOTI returns an allocated empty vector; wrapper/record/string
// allocations must succeed (wrapper is dereferenced even after a null result).
void* WPuppet_LoadMotionRangesFromChunk(void* unused, void* archive, unsigned int size) {
    // Phase 1: allocate the vector wrapper and locate MOTI; absent/empty chunks
    // return an empty owner. Factory allocation is assumed to succeed by this code.
    (void)unused;
    MotionRangeVector* result = (MotionRangeVector*)__nw__FUl(12);
    if (result) {
        result->data = 0;
        result->size = 0;
        result->capacity = 0;
    }
    unsigned int zero = *(volatile unsigned int*)&result->size;
    result->size = zero - zero;
    char* entry = (char*)FindMotionChunk((const char*)archive, size);
    if (!entry)
        return result;
    unsigned short count = *(unsigned short*)(entry + 0xC);
    if (!count)
        return result;
    char* source = entry + *(unsigned int*)(entry + 8);
    // Phase 2: create a contiguous record array; each record owns its copied
    // data/name strings. Endpoints convert from 30-Hz frames to seconds.
    Fn80056084Record* records = (Fn80056084Record*)__nwa__FUl(count * 0x34);
    Memory_Clear(records, count * 0x34);
    MotionRangeEntry* in = (MotionRangeEntry*)source;
    Fn80056084Record* out = records;
    for (int i = 0; i < count; ++i) {
        out->startSeconds = ((float)in->firstFrame) / 30.0f;
        float h = ((float)in->lastFrame) / 30.0f;
        out->blendEndSeconds = h;
        out->endSeconds = h;
        out->nameOffset = in->nameOffset;
        out->dataOffset = in->dataOffset;
        out->rangeCode = in->rangeCode;
        out->blendSourceRangeIndex = in->blendSourceRangeIndex;
        out->animationIndex = in->animationIndex;
        out->rangeName = __nwa__FUl(in->nameLength + 1);
        Memory_Clear(out->rangeName, in->nameLength + 1);
        Memory_Copy(out->rangeName, entry + in->nameBytesOffset, in->nameLength);
        ++in;
        ++out;
    }
    // Phase 3: resolve each record's referenced-name index after all names exist.
    out = records;
    for (int i = 0; i < count; ++i) {
        Fn80056084Record* ref = (Fn80056084Record*)((char*)records + out->blendSourceRangeIndex * 0x34);
        unsigned int len = strlen((char*)ref->rangeName);
        out->blendSourceName = __nwa__FUl(len + 1);
        strcpy((char*)out->blendSourceName, (char*)ref->rangeName);
        ((char*)out->blendSourceName)[len] = 0;
        ++out;
    }
    if (count > result->capacity) {
        MotionRangeVector tmp;
        if (count > 0x3FFFFFFFu)
            ThrowMotionRangeLengthError();
        tmp.data = (void**)appMallocDummy__FUl(count * 4);
        void** begin = result->data;
        unsigned int n = (unsigned int)((begin + result->size) - begin);
        tmp.capacity = count;
        unsigned int bytes = n * 4;
        memcpy(tmp.data + tmp.size, begin, bytes);
        memset(begin, 0, bytes);
        result->size = 0;
        tmp.size = tmp.size + n;
        unsigned int oldCapacity = result->capacity;
        result->capacity = tmp.capacity;
        void** oldData = result->data;
        result->data = tmp.data;
        unsigned int oldSize = result->size;
        tmp.capacity = oldCapacity;
        tmp.data = oldData;
        result->size = tmp.size;
        tmp.size = oldSize;
    }
    // Phase 4: publish pointers into the shared contiguous record allocation.
    // Cleanup must free each string, the first record's array base, then this vector.
    char* item = (char*)records;
    for (int i = 0; i < count; ++i) {
        if (result->capacity > result->size) {
            ++result->size;
            (result->data + result->size)[-1] = item;
        } else {
            unsigned int capacity = result->capacity;
            if (0x3FFFFFFFu - capacity < 1u)
                ThrowMotionRangeLengthError();
            MotionRangeVector tmp;
            unsigned int needed = result->size + 1 - capacity;
            if (needed > 0x3FFFFFFFu - capacity)
                ThrowMotionRangeLengthError();
            unsigned int newCap;
            if (capacity < 0x15555555u)
                newCap = capacity + fn56084max((capacity + 1) * 3 / 5, needed);
            else if (capacity < 0x2AAAAAAAu)
                newCap = capacity + fn56084max((capacity + 1) / 2, needed);
            else
                newCap = 0x3FFFFFFF;
            if (newCap > 0x3FFFFFFFu)
                ThrowMotionRangeLengthError();
            unsigned int n = result->size;
            tmp.data = (void**)appMallocDummy__FUl(newCap * 4);
            tmp.data[n] = item;
            unsigned int after = (unsigned int)((result->data + result->size) - result->data);
            memcpy(tmp.data + (result->size - after), result->data, after * 4);
            memset(result->data, 0, after * 4);
            void** oldData = result->data;
            unsigned int oldCapacity = result->capacity;
            unsigned int newSize = after + 1;
            tmp.capacity = newCap;
            result->size = 0;
            result->data = tmp.data;
            result->capacity = tmp.capacity;
            result->size = newSize;
            tmp.data = oldData;
            tmp.capacity = oldCapacity;
            tmp.size = 0;
        }
        item += 0x34;
    }
    return result;
}
#pragma opt_common_subs reset
#pragma opt_common_subs off
#pragma opt_propagation off
struct NamedResourceVectorTemp {
    void** data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* alloc;
    unsigned int extra;
    // Guard external-resource vector storage while borrowing the destination capacity.
    NamedResourceVectorTemp(unsigned int* a) : data(0), size(0), capacity(0), alloc(a), extra(0) {}
    // Free storage only; moved preloader pointers belong to the destination vector.
    ~NamedResourceVectorTemp() {
        if (data) {
            unsigned int n = size;
            size = n - n;
            appFreeUni__FPv(data);
        }
    }
};

extern void __throw(const char*, void*, void*) __attribute__((noreturn));

// Throw a length exception when the external-resource pointer vector would overflow.
static inline void ThrowNamedResourceLengthError() {
    struct Message {
        void* data;
        unsigned int size;
        unsigned int capacity;
    } msg;
    Message* mp = &msg;
    mp->data = (void*)&__vt__Q23std11logic_error;
    char alloc __attribute__((aligned(4))) = 0;
    const char* message = (const char*)puppetPreloadStrings.vectorLengthError;
    unsigned int len = strlen(message);
    char* text = (char*)__nwa__FUl((len + 1) * 2 - (len + 1));
    mp->size = (unsigned int)text;
    {
        Copy502StringGuard guard;
        guard.text = text;
        guard.allocator = &alloc;
        unsigned int* rep = (unsigned int*)__nw__FUl(0x10);
        if (rep) {
            rep[1] = 1;
            rep[2] = 1;
            rep[0] = (unsigned int)&s_sharedCharDeleterVtable;
            rep[3] = (unsigned int)text;
        }
        mp->capacity = (unsigned int)rep;
        guard.text = 0;
    }
    strcpy((char*)mp->size, message);
    mp->data = (void*)&__vt__Q23std12length_error;
    __throw((const char*)(puppetPreloadStrings.lengthErrorType), mp, (void*)__dt__Q23std12length_errorFv);
}
struct Fn585Item {
    virtual void destroy(int);
};
// Select the larger external-resource vector capacity increment.
static inline const unsigned int& fn585max(const unsigned int& a, const unsigned int& b) {
    const unsigned int* result = &a;
    if (a < b)
        result = &b;
    return *result;
}
// Return the end iterator of the external-resource pointer vector.
static inline void** fn585end(NamedResourceVector* v) {
    return v->data + v->size;
}
// Fill a pointer span with the supplied retained resource value.
static inline void fn585fill(void** out, unsigned int n, void* const& value) {
    for (; n; --n)
        *out++ = value;
}
// Transfer pointer slots into temporary vector storage and clear their previous owners.
static inline void fn585move(NamedResourceVectorTemp& tmp, NamedResourceVector* vec, void** from, void** end) {
    unsigned int used = (int)((char*)end - (char*)from) / 4;
    tmp.extra -= used;
    unsigned int bytes = used * 4;
    memcpy(tmp.data + tmp.extra, from, bytes);
    memset(from, 0, bytes);
    vec->size = 0;
    tmp.size += used;
}

// Load EXTF resource names and retain the corresponding synchronous preloader objects.
// archive is borrowed, native-endian packed chunk data. Requires complete aligned
// headers, nonzero byteSize steps, and valid record/name offsets within the EXTF
// chunk; size bounds the search only, not every read. No format validation is added.
// Return 0 if EXTF is absent (old resources unchanged) or vector allocation is null;
// return 1 once the chunk is processed, even if an individual preload fails.
int WPuppet_LoadNamedResourcesFromChunk(void* self, const char* archive, unsigned int size) {
    char* entry;
    const char* magic = puppetPreloadStrings.externalFilesTag;
    unsigned int offset = 0;
    // Phase 1: search chunk headers {tag +0, byteSize +4, dataOffset +8,
    // entryCount u16 +c, formatFlags u16 +e}; offsets below are relative to entry.
    while ((offset < size) || (entry = 0, false)) {
        char* p = (char*)archive + offset;
        if (memcmp(magic, p, 4) == 0) {
            entry = p;
            break;
        }
        offset += ((PuppetChunkHeader*)p)->byteSize;
    }
    if (!entry)
        return 0;
    // Phase 2: after finding EXTF, destroy old request objects, buffer and wrapper.
    // This replacement is destructive before new loading; it has no rollback.
    NamedResourceVector* vec = *(NamedResourceVector**)&AsWPuppet(self)->namedResources;
    if (vec) {
        void* empty = 0;
        for (void** p = vec->data; fn585end(*(NamedResourceVector**)&AsWPuppet(self)->namedResources) != p; ++p) {
            if (*p) {
                Fn585Item* item = (Fn585Item*)*p;
                if (item)
                    item->destroy(1);
                *p = empty;
            }
        }
        (*(NamedResourceVector**)&AsWPuppet(self)->namedResources)->size -=
            (*(NamedResourceVector**)&AsWPuppet(self)->namedResources)->size;
        NamedResourceVector* dead = *(NamedResourceVector**)&AsWPuppet(self)->namedResources;
        if (dead) {
            if (dead) {
                if (dead) {
                    if (dead) {
                        if (dead) {
                            if (dead) {
                                if (dead->data) {
                                    dead->size -= dead->size;
                                    appFreeUni__FPv(dead->data);
                                }
                            }
                        }
                    }
                }
                __dl__FPv(dead);
            }
            AsWPuppet(self)->namedResources = 0;
        }
    }
    if (!((PuppetChunkHeader*)entry)->entryCount)
        return 1;
    vec = (NamedResourceVector*)__nw__FUl(12);
    if (vec) {
        vec->data = 0;
        vec->size = 0;
        vec->capacity = 0;
    }
    *(NamedResourceVector**)&AsWPuppet(self)->namedResources = vec;
    if (!vec)
        return 0;
    void* zero = 0;
    unsigned int count = ((PuppetChunkHeader*)entry)->entryCount;
    unsigned int oldSize = vec->size;
    // Phase 3: resize to count NULL slots, not just capacity. Growth uses a guarded
    // buffer transfer/clear/swap; after commit tmp frees only the old slot array.
    // The later preload helper APPENDS requests beyond these initial null slots.
    do {
        if (count > oldSize) {
            unsigned int add = (unsigned int)count - oldSize;
            unsigned int capacity = vec->capacity;
            if (add > capacity || oldSize > capacity - add) {
                unsigned int required = (vec->size + add) - vec->capacity;
                if (required > 0x3FFFFFFFu - vec->capacity)
                    ThrowNamedResourceLengthError();
            } else {
                fn585fill(vec->data + oldSize, add, zero);
                vec->size += add;
                break;
            }
            {
                unsigned int needed = (vec->size + add) - vec->capacity;
                NamedResourceVectorTemp tmp(&vec->capacity);
                unsigned int cap = vec->capacity;
                if (needed > 0x3FFFFFFFu - cap)
                    ThrowNamedResourceLengthError();
                unsigned int newCap;
                if (cap < 0x15555555u) {
                    unsigned int grown = (cap + 1) * 3 / 5;
                    unsigned int best = fn585max(grown, needed);
                    newCap = cap + best;
                } else if (cap < 0x2AAAAAAAu) {
                    unsigned int grown = (cap + 1) / 2;
                    unsigned int best = fn585max(grown, needed);
                    newCap = cap + best;
                } else {
                    newCap = 0x3FFFFFFFu;
                }
                if (newCap > 0x3FFFFFFFu)
                    ThrowNamedResourceLengthError();
                tmp.data = (void**)appMallocDummy__FUl(newCap * 4);
                tmp.capacity = newCap;
                tmp.extra = vec->size;
                fn585fill(tmp.data + vec->size + tmp.size, add, zero);
                tmp.size = tmp.size + add;
                fn585move(tmp, vec, vec->data, vec->data + vec->size);
                unsigned int oldCapacity = vec->capacity;
                vec->capacity = tmp.capacity;
                tmp.capacity = oldCapacity;
                void** oldData = vec->data;
                vec->data = tmp.data;
                tmp.data = oldData;
                vec->size = tmp.size;
                tmp.size = 0;
            }
        } else if ((unsigned int)count < oldSize) {
            unsigned int diff = oldSize - (unsigned int)count;
            vec->size = oldSize - diff;
        }
    } while (0);

    // Phase 4: copy each length-delimited name to a temporary terminated string.
    // PreloadNamedResource copies/consumes the name during the call; free it afterward.
    NamedResourceRecord* rec = (NamedResourceRecord*)(entry + ((PuppetChunkHeader*)entry)->dataOffset);
    char terminator = 0;
    for (int i = 0; i < ((PuppetChunkHeader*)entry)->entryCount; ++i) {
        if (rec->length) {
            char* name = (char*)__nwa__FUl(rec->length + 1);
            memcpy(name, entry + rec->offset, rec->length);
            name[rec->length] = terminator;
            WPuppet_PreloadNamedResource(self, name);
            if (name)
                __dla__FPv(name);
        }
        ++rec;
    }
    return 1;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset

struct B3CCRef {
    int vt;
    int c1;
    int c2;
    char* s;
};
struct B3CCHolder {
    char* p;
    char* flag;
    // Guard the material-vector error message with a borrowed allocator marker.
    B3CCHolder(char* b, char* f) {
        p = b;
        flag = f;
    }
    // Release the message array unless its ownership was transferred.
    ~B3CCHolder() {
        if (p)
            __dla__FPv(p);
    }
};
struct B3CCErrBase {
    void* vt;
    char* buf;
    B3CCRef* ref;
    // Copy a material-vector overflow message into a shared exception owner.
    B3CCErrBase(const char* msg0) {
        vt = (void*)&__vt__Q23std11logic_error;
        char done __attribute__((aligned(4))) = 0;
        const char* msg = msg0;
        unsigned n = strlen(msg) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            B3CCHolder h(buffer, &done);
            B3CCRef* r = (B3CCRef*)__nw__FUl(16);
            if (r) {
                int table = (int)&s_sharedCharDeleterVtable;
                r->c1 = 1;
                r->c2 = 1;
                r->vt = table;
                r->s = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
    // Empty local cleanup; the exception runtime uses the supplied shared-payload destructor.
    ~B3CCErrBase() {}
};
struct B3CCErr2 : B3CCErrBase {
    // Install the length_error vtable and throw the constructed payload.
    B3CCErr2(const char* msg) : B3CCErrBase(msg) {
        vt = (void*)&__vt__Q23std12length_error;
        __throw((const char*)(puppetPreloadStrings.lengthErrorType), this, (void*)__dt__Q23std12length_errorFv);
    }
    // Empty local cleanup, preserving the original exception ABI shape.
    ~B3CCErr2() {}
};
struct B3CCCP {
    char v[2];
};
struct B3CCIP {
    int a, b;
};
struct B3CCElem {
    float a[17];
    char bytesA[12];
    B3CCIP q0, q1;
    int r0;
    char bytesB[12];
    B3CCIP w0;
    B3CCIP w1;
    B3CCIP w2;
    B3CCIP w3;
    B3CCIP w4;
    B3CCIP w5;
    B3CCIP w6;
    B3CCIP w7;
    B3CCIP w8;
    B3CCIP w9;
    B3CCIP w10;
    B3CCIP w11;
    B3CCIP w12;
    int x0;
    B3CCIP y0, y1;
    int z0;
};
struct B3CCVector {
    B3CCElem* data;
    unsigned int size;
    unsigned int capacity;
};
extern void __throw(const char*, void*, void*) __attribute__((noreturn));
// Throw the material vector length exception before an overflowing insertion.
static inline void fn_8005B3CC_throw() {
    const char* m;
    m = puppetPreloadStrings.vectorLengthError;
    B3CCErr2 e(m);
}
// Copy overlapping material records from the end toward the insertion point.
static inline B3CCElem* B3CCcopyBack(B3CCElem* src, B3CCElem* low, B3CCElem* dst) {
    while (src > low) {
        --src;
        --dst;
        *dst = *src;
    }
    return dst;
}
// Return the number of material records between two iterators.
static inline int fn8005B3CCdist(B3CCElem* first, B3CCElem* last) {
    return last - first;
}
// Select the larger material-vector capacity increment.
template <class T> static inline const T& fn8005B3CCmax(const T& a, const T& b) {
    const T* r;
    r = &a;
    if (a < b)
        r = &b;
    return *r;
}

// Insert an iterator range of material records into a vector, growing its storage when necessary.
void PuppetMaterialVector_InsertIteratorRange(B3CCVector* vecPtr, B3CCElem* position, B3CCElem** firstArg,
                                              B3CCElem** lastArg) {
    // Phase 1: measure the borrowed source range; position must be an iterator
    // into vecPtr. Records are copied by value; pointed-to resources are not retained.
    B3CCElem** firstP;
    B3CCVector* v;

    B3CCElem* posArg;
    B3CCElem** lastP;
    v = vecPtr;
    posArg = position;
    firstP = firstArg;
    lastP = lastArg;
    int count;
    count = fn8005B3CCdist(*firstP, *lastP);
    if (count == 0)
        return;
    B3CCElem* pos = posArg;

    unsigned int cap0 = v->capacity;
    if (count > cap0 || v->size > cap0 - count) {
        unsigned int cap = v->capacity;
        if (v->size + count - cap0 > 0x1111111u - cap)
            fn_8005B3CC_throw();
        unsigned int unused;
        if (cap < 0x5B05B0u) {
            unused = (cap + 1) * 3 / 5;
        } else if (cap < 0xB60B60u) {
            unused = (cap + 1) >> 1;
        }
    } else {
        unsigned int size = v->size;
        // Phase 2a: with spare capacity, shift the suffix backward before filling the gap.
        B3CCElem* src;
        B3CCElem* low;
        B3CCElem* fEnd;
        int shift;
        B3CCElem* dst;
        B3CCElem* fSrc;

        B3CCElem* fDst;
        dst = v->data + size;
        int countValue3 = count;
        v->size = size + count;
        shift = countValue3 * sizeof(B3CCElem);
        unsigned int after = dst - pos;
        dst = (B3CCElem*)((char*)dst + shift);
        if (after) {
            src = (B3CCElem*)((char*)dst - shift);
            low = dst - (count + after);
            while (src > low)
                *--dst = *--src;
        }
        fSrc = *firstP;
        fDst = (B3CCElem*)((char*)dst - shift);
        fEnd = *lastP;
        for (; fSrc < fEnd; ++fSrc, ++fDst)
            *fDst = *fSrc;
        return;
    }

    // Phase 2b: grow storage, copy insertion/prefix/suffix, free the old buffer,
    // then publish the new size. The iterator argument objects themselves are borrowed.
    B3CCElem* old = v->data;
    int index = pos - old;
    unsigned int need = v->size + count - v->capacity;
    unsigned int cap2 = v->capacity;
    if (need > 0x1111111u - cap2)
        fn_8005B3CC_throw();
    unsigned int newCap;
    unsigned int grownB;
    unsigned int grownA;
    int g;
    if (cap2 < 0x5B05B0u) {
        g = ((1 + cap2)) * 3 / 5;
        int gValue = g;
        grownA = gValue;
        newCap = cap2 + fn8005B3CCmax(grownA, need);
    } else if (cap2 < 0xB60B60u) {
        g = ((cap2 + 1)) >> 1;
        grownB = g;
        newCap = cap2 + fn8005B3CCmax(grownB, need);
    } else {
        newCap = 0x1111111u;
    }
    if (newCap > 0x1111111u)
        fn_8005B3CC_throw();
    B3CCElem* dst;
    B3CCElem* srcEnd;
    B3CCElem* src;
    B3CCElem* fresh = (B3CCElem*)appMallocDummy__FUl(newCap * kMaterialBytes);

    v->data = fresh;
    v->capacity = newCap;
    src = *firstP;
    srcEnd = *lastP;
    unsigned int offset = index * sizeof(B3CCElem);
    dst = (B3CCElem*)((char*)fresh + offset);
    for (; src < srcEnd; ++src, ++dst)
        *dst = *src;
    if (old) {
        B3CCElem* head;
        B3CCElem* out;
        B3CCElem* out2;
        B3CCElem* mid;
        B3CCElem* end;
        out = v->data;
        head = old;
        mid = (B3CCElem*)((char*)old + offset);
        for (; head < mid; ++head, ++out)
            *out = *head;
        out2 = out + count;
        end = (old + v->size);
        for (; mid < end; ++mid, ++out2)
            *out2 = *mid;
        appFreeUni__FPv(old);
    }
    int countValue4 = count;
    v->size += countValue4;
}

// Insert a direct range of material records into a vector, preserving existing prefix and suffix records.
void PuppetMaterialVector_InsertDirectRange(void* vecPtr, void* position, void* firstArg, void* lastArg, void* unused) {
    // Phase 1: adapt raw iterators to material records and measure the source range.
    // This is the direct-pointer form of InsertIteratorRange's ownership contract.
    unsigned int g;
    B3CCElem* first;
    B3CCVector* v;
    B3CCElem* pos;
    B3CCElem* last;
    int count;
    (void)unused;
    first = (B3CCElem*)firstArg;
    v = (B3CCVector*)vecPtr;
    pos = (B3CCElem*)position;
    last = (B3CCElem*)lastArg;
    count = last - first;
    if (0 == count)
        return;

    // Phase 2: either shift/fill in the existing allocation, or allocate and copy
    // insertion/prefix/suffix before releasing old storage and updating size.
    unsigned int cap0 = v->capacity;
    if (count > cap0 || cap0 - count < v->size) {
        unsigned int cap = v->capacity;
        unsigned int unused2;
        if (v->size + count - cap0 > 0x1111111u - cap)
            fn_8005B3CC_throw();
        if (cap < 0x5B05B0u) {
            unused2 = (1 + cap) * 3 / 5;
        } else if (cap < 0xB60B60u) {
            unused2 = (cap + 1) >> 1;
        }
    } else {
        B3CCElem* src;
        B3CCElem* low;
        int shift;
        B3CCElem* dst;
        B3CCElem* fDst;
        unsigned int size = v->size;
        dst = size + v->data;
        v->size = count + size;
        shift = count * sizeof(B3CCElem);
        unsigned int after = dst - pos;
        dst = (B3CCElem*)((char*)dst + shift);
        if (after) {
            low = dst - (count + after);
            src = (B3CCElem*)((char*)dst - shift);
            while (src > low)
                *--dst = *--src;
        }
        fDst = (B3CCElem*)((char*)dst - shift);
        for (; first < last; ++first, ++fDst)
            *fDst = *first;
        return;
    }

    B3CCElem* old = v->data;
    unsigned int index = pos - old;
    unsigned int need = v->size + count - v->capacity;
    unsigned int cap2 = v->capacity;
    if (need > 0x1111111u - cap2)
        fn_8005B3CC_throw();
    unsigned int newCap;
    unsigned int grownB;
    unsigned int grownA;
    if (cap2 < 0x5B05B0u) {
        g = ((cap2 + 1)) * 3 / 5;
        grownA = g;
        newCap = cap2 + fn8005B3CCmax(grownA, need);
    } else if (cap2 < 0xB60B60u) {
        g = (cap2 + 1) >> 1;
        unsigned int gValue2 = g;
        grownB = gValue2;
        newCap = cap2 + fn8005B3CCmax(grownB, need);
    } else {
        newCap = 0x1111111u;
    }
    if (newCap > 0x1111111u)
        fn_8005B3CC_throw();
    B3CCElem* dst;
    B3CCElem* fresh = (B3CCElem*)appMallocDummy__FUl(newCap * kMaterialBytes);
    v->data = fresh;
    v->capacity = newCap;
    unsigned int offset = index * sizeof(B3CCElem);
    dst = (B3CCElem*)((char*)fresh + offset);
    for (; first < last; ++first, ++dst)
        *dst = *first;
    if (old) {
        B3CCElem* head;
        B3CCElem* out;
        B3CCElem* out2;
        B3CCElem* mid;
        B3CCElem* end;
        out = v->data;
        head = old;
        mid = (B3CCElem*)((char*)old + index * sizeof(B3CCElem));
        for (; head < mid; ++head, ++out)
            *out = *head;
        out2 = out + count;
        end = old + v->size;
        for (; mid < end; ++mid, ++out2)
            *out2 = *mid;
        appFreeUni__FPv(old);
    }
    v->size += count;
}

// Evaluate a named material matrix after its threshold and store it in the indexed layer controller.
int WPuppet_ApplyNamedMaterialLayerAnimation(void* self, void* animationSource, void* animationName,
                                             unsigned int layerIndex, float threshold) {
    if (!animationSource || !animationName)
        return 0;
    void* resource = AsWPuppet(self)->layerScratch;
    if (!resource)
        return 0;
    int state;
    float value;
    if (AsWPuppet(animationSource)->animation)
        state =
            ((CPetAnimation*)AsWPuppet(animationSource)->animation)->GetValueByName(&value, (const char*)animationName);
    else
        state = 0;
    if (!state)
        return 0;
    if (value <= threshold)
        return 0;
    if (!AsWPuppet(self)->layerResource) {
        if (AsWPuppet(self)->modelResource) {
            if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AsWPuppet(self)->modelResource,
                                                                     kChunkMaterialLayers))
                AsWPuppet(self)->layerResource = AsWPuppet(self)->modelResource;
        } else if (AsWPuppet(self)->loadFlags == kContainerLoadMode) {
            void* sentinel = (char*)&AsWPuppet(self)->childTail;
            for (void* n = AsWPuppet(self)->childHead; n != sentinel; n = ((WPuppetListNode*)n)->next) {
                if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(
                        AsWPuppet(((WPuppetListNode*)n)->value)->modelResource, kChunkMaterialLayers)) {
                    AsWPuppet(self)->layerResource = AsWPuppet(((WPuppetListNode*)n)->value)->modelResource;
                    break;
                }
            }
        }
    }
    void* found = AsWPuppet(self)->layerResource;
    unsigned int layers;
    if (found)
        layers = (unsigned int)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(found, kChunkMaterialTransforms);
    else
        layers = 0;
    unsigned int* arg3p = &layerIndex;
    if (layers <= *arg3p)
        return 0;
    int ok;
    if (AsWPuppet(animationSource)->animation)
        ok = ((CPetAnimation*)(AsWPuppet(animationSource)->animation))
                 ->EvaluateNamedMaterialMatrix(
                     (PetMaterialMatrix*)resource, 1, threshold, (const char*)animationName,
                     *arg3p); /* 0x80059CC8-0x80059CF8: r7 = layerIndex (loaded for the compare above, still live) */
    else
        ok = 0;
    if (!ok)
        return 0;
    void* object = AsWPuppet(self)->layerController;
    if (!object)
        return 0;
    int result = BoneControllerMap_SetIndexedPayload(object, (int*)&layerIndex, resource);
    return result;
}

struct OldVt54F90 {
    char pad[4];
    virtual void release(int);
};
// Locate ANIM in trusted chunk bytes; each chunk's native-endian +4 size must
// be positive and fit the remaining span. There is no malformed-data validator.
static inline void* find54F90(const char* data, unsigned int size) {
    char* block;
    const char* sig = puppetPreloadStrings.animationTag;
    unsigned int offset = 0;
    while (offset < size) {
        block = (char*)data + offset;
        if (memcmp((void*)sig, block, 4) == 0)
            return block;
        offset += *(unsigned int*)(block + 4);
    }
    return 0;
}
// Six GPR arguments, as supplied by LoadResourceFile and LoadNamedResourceFromChunks:
// self, borrowed data, sizeOrArchiveName, textureLoadMode, resourceDirectory, flags.
// This dispatcher has NO return status: wrappers ignore r3 and inspect the model.
// The former unused seventh/eighth parameters and int return obscured that ABI.
// sizeOrArchiveName is a tagged-by-flags word: archive mode 0x2000 passes it as
// the archive name to CreateFromMemory; chunk loaders interpret its numeric value
// as a byte length. Even archive-only flags still reach the unconditional BONE
// scan below: the word is forwarded unchanged to both interpretations, with no
// validation. A safe general archive-only calling contract is not established
// by these wrappers; do not infer that flags isolate the two modes.
// data is writable for MESH scaling.
// Operation/state order (no transaction/rollback; callee statuses are ignored):
//  1 publish flags; optional archive, then MESH/model/materials;
//  2 ANIM: absent -> duration=0 but KEEP old animation; present -> delete old,
//    allocate/construct replacement, load entries, publish returned duration;
//  3 MOTI replaces range pointer; KEYS; textures use mode/directory and local count;
//  4 ALWAYS delete boneRoot, clear it, reload BONE (even with flags==0);
//  5 optional collision, then EXTF. Missing BONE returns null after old root loss.
// Allocation/nested-offset failures are unchecked. Resources may borrow data;
// callers must obey each loader's lifetime rules. Serialize against draw/update.
void WPuppet_LoadFromChunks(void* self, void* data, void* sizeOrArchiveName, int textureLoadMode,
                            void* resourceDirectory, int flags) {
    AsWPuppet(self)->loadFlags = flags;
    if (flags & kLoadModelArchive)
        WPuppet_LoadModelFromArchive(self, data, sizeOrArchiveName);
    if (flags & kLoadModel) {
        // Only the first three words are consumed by this loader's definition.
        // Keep the historical extra call operands (including the boneRoot read).
        ((void (*)(void*, void*, void*, void*, int, int))WPuppet_LoadModelAndMaterialsFromChunks)(
            self, data, sizeOrArchiveName, AsWPuppet(self)->boneRoot, 0, flags == kCloneModelLoadMode);
    }
    if (flags & kLoadAnimation) {
        void* block = find54F90((const char*)data, (unsigned int)sizeOrArchiveName);
        float value;
        if (!block) {
            value = kFloat_0_puppet;
        } else {
            void* old = AsWPuppet(self)->animation;
            if (old) {
                if (old) {
                    ((OldVt54F90*)old)->release(1);
                }
                AsWPuppet(self)->animation = 0;
            }
            void* obj = (void*)__nw__FUl(0x18);
            if (obj)
                ((CPetAnimation*)obj)->__ct();
            AsWPuppet(self)->animation = obj;
            value = ((CPetAnimation*)obj)->LoadEntriesFromChunk((PetAnimationChunk*)block);
        }
        AsWPuppet(self)->animationDuration = value;
    }
    if (flags & kLoadMotionRanges) {
        AsWPuppet(self)->animationRanges =
            // Loader consumes three GPR words; this legacy FPR operand is unused.
            ((void* (*)(void*, void*, void*, float))WPuppet_LoadMotionRangesFromChunk)(self, data, sizeOrArchiveName,
                                                                                       kFloat_0_puppet);
    }
    if (flags & kLoadKeys)
        WPuppet_LoadKeyRecordsFromChunk(self, data, (unsigned int)sizeOrArchiveName);
    unsigned int count = 0;
    if (flags & kLoadTextures) {
        WPuppet_LoadMaterialTextures(self, data, sizeOrArchiveName, (void*)(long)textureLoadMode,
                                     resourceDirectory ? resourceDirectory : 0, &count);
    }
    if (AsWPuppet(self)->boneRoot) {
        WPuppetBone_dtor(AsWPuppet(self)->boneRoot, 1);
        AsWPuppet(self)->boneRoot = 0;
    }
    // BONE returns a pointer encoded in its declared int result. The fourth GPR
    // operand is not read by that definition; do not infer flag-gated bone reload.
    AsWPuppet(self)->boneRoot =
        ((void* (*)(void*, void*, void*, int))WPuppet_LoadBonesFromChunks)(self, data, sizeOrArchiveName, flags);
    if (flags & kLoadCollision)
        WPuppet_LoadCollisionChunks(self, data, (unsigned int)sizeOrArchiveName);
    if (flags & kLoadExternalResources) {
        // Underlying loader consumes the numeric byte length; result is ignored.
        ((void (*)(void*, void*, void*))WPuppet_LoadNamedResourcesFromChunk)(self, data, sizeOrArchiveName);
    }
}

#undef ReleasePuppetChildAndCacheStorage
} // extern "C"
