typedef unsigned long new_size_t;
// Course surface sidecar (.sbin) loading and barycentric projection.
// Mod lookup: triangle weights -> Triangle_ComputeBarycentricCoordinates;
// variant selection -> SurfaceVariantPolicy; sidecar framing -> loader contract;
// ownership/failure -> table below. Shared views: CourseData801C, WPuppet,
// CSharedDoc and IFF_STRUCT::sCourse. Extension bytes: courseModelResourceStrings.
// The loader's missing group decoding is a correctness boundary: this source
// can explain the current behavior but cannot support a new positive-count asset.
#include "classes/CourseData801C.h"
#define AsCourseData801C(p) ((CourseData801C*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"
#include "classes/WPuppet.h"
#include "iff/IffStruct.h"
// Translation unit 0x801C482C-0x801C54BC (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// Editing guide: Triangle_ComputeBarycentricCoordinates is a complete geometric
// routine. LoadSurfaceBinary/ReadSurfaceGroups preserve an incomplete parser:
// the latter has no generated-node constructor call and cannot safely consume
// positive group counts. Layout documentation below describes current storage,
// not a recovered implementation of the missing on-disk group payload.

extern "C" {

enum { kCourseVariantMask = 0xF, kGeneratedNodeIndexMask = 0x7FF, kSeekRelative = 1 };
enum SurfaceBinarySizes { kSerializedWordBytes = 4, kSerializedPairBytes = 8,
    kSerializedTripleBytes = 12, kArrayCookieBytes = 16,
    kSurfaceSidecarPathBytes = 0x40 };
enum SurfaceVariantPolicy { kSkipBothVariantBlocks = 0, kReadFirstVariantBlock = 1, kReadSecondVariantBlock = 2 };
struct SurfaceModelRecord {
    char pad_000[0x10];
    WPuppet* puppet;
    char pad_014[0xC8];
};
typedef char SurfaceModelRecord_size[(sizeof(SurfaceModelRecord) == 0xDC) ? 1 : -1];
struct OwnedArray { int count; void* data; };
struct CourseModelResourceStrings {
    char precedingEntries[0x244];
    char surfaceBinaryExtension[6]; // ".sbin"
};
enum { kSurfaceBinaryExtensionOffset = (int)&((CourseModelResourceStrings*)0)->surfaceBinaryExtension };


// Initialize an empty owned record array.
void CourseSurfaceGroup_InitEmpty(void* self) {
    OwnedArray* array = (OwnedArray*)self;
    array->count = 0;
    array->data = 0;
}

void WPuppet_SetHierarchyContextRecursive(void*, void*);
void __mi__FRC7WVectorRC7WVector(float*, const float*, const float*);
void Vector_CrossProduct(float*, const float*, const float*);
void __as__7WVectorFRC7WVector(float*, const float*);
extern float kFloat_0_PetBody;

// Propagate the hierarchy context to every loaded surface model and the fallback model.
void CourseData_SetSurfaceHierarchyContext(void* self, void* hierarchyContext) {
    int offset = 0;
    int i = 0;
    while (i < AsCourseData801C(self)->surfaceCount) {
        SurfaceModelRecord* records = (SurfaceModelRecord*)AsCourseData801C(self)->surfaces;
        void* object = ((SurfaceModelRecord*)((char*)records + offset))->puppet;
        if (object != 0) {
            WPuppet_SetHierarchyContextRecursive(object, hierarchyContext);
        }
        offset += sizeof(SurfaceModelRecord);
        ++i;
    }
    void* object = ((SurfaceModelRecord*)AsCourseData801C(self)->fallbackSurface)->puppet;
    WPuppet_SetHierarchyContextRecursive(object, hierarchyContext);
}

// Project onto the triangle normal's dominant plane, then solve for the B/C barycentric weights.
// All inputs must use the same coordinate space. Triangle must be nondegenerate;
// no zero-determinant check/clamp is performed. Output A weight is 1-B-C; weights
// can lie outside [0,1] when point projects outside the triangle. No ownership transfer.
void Triangle_ComputeBarycentricCoordinates(float* weightB, float* weightC, const float* point, const float* vertexA,
                 const float* vertexB, const float* vertexC) {
    // The projected array is {pointU, pointV, edgeBU, edgeBV, edgeCU, edgeCV}.
    struct ProjectedTriangle { float pointU, pointV, edgeBU, edgeBV, edgeCU, edgeCV; };
    struct { float projectedPointU; float projectedPointV; float edgeAC[3]; float edgeAB[3]; float normal[3]; float normalCopy[3]; ProjectedTriangle projected; } scratch;
    __mi__FRC7WVectorRC7WVector(scratch.edgeAC, vertexC, vertexA);
    __mi__FRC7WVectorRC7WVector(scratch.edgeAB, vertexB, vertexA);
    Vector_CrossProduct(scratch.normal, scratch.edgeAB, scratch.edgeAC);
    __as__7WVectorFRC7WVector(scratch.normalCopy, scratch.normal);
    float absNormalX = scratch.normalCopy[0]; absNormalX = (absNormalX > kFloat_0_PetBody) ? absNormalX : -absNormalX;
    float absNormalY = scratch.normalCopy[1]; absNormalY = (absNormalY > kFloat_0_PetBody) ? absNormalY : -absNormalY;
    float absNormalZ = scratch.normalCopy[2]; absNormalZ = (absNormalZ > kFloat_0_PetBody) ? absNormalZ : -absNormalZ;
    if (absNormalX >= absNormalY && absNormalX >= absNormalZ) {
        // Drop X and solve in the Z/Y plane.
        float originU = vertexA[2];

        float originV = vertexA[1];
        float edgeBV = vertexB[1];

        float edgeCU = vertexC[2];

        edgeBV = edgeBV - originV;

        float edgeCV = vertexC[1]; edgeCU = edgeCU - originU;
        edgeCV -= originV;
        float pointU = point[2]; scratch.projectedPointU = pointU;
        float pointV = point[1]; scratch.projectedPointV = pointV;
        float pointDeltaU = pointU - originU; scratch.projected.pointU = pointDeltaU;
        scratch.projected.pointV = (pointV - originV);
        float edgeBU = (vertexB[2]) - originU; scratch.projected.edgeBU = edgeBU;
        scratch.projected.edgeBV = edgeBV;
        scratch.projected.edgeCU = edgeCU;
        scratch.projected.edgeCV = edgeCV;
    } else if (absNormalY >= absNormalX && absNormalY >= absNormalZ) {
        // Drop Y and solve in the X/Z plane.
        float edgeCU = vertexC[0];
        float originU = vertexA[0];

        float pointU = point[0];

        float originV = vertexA[2];
        float pointDeltaU = pointU - originU;
        float edgeBV = vertexB[2];

        float edgeCV = vertexC[2];
        float pointV = point[2];

        scratch.projectedPointU = pointU; edgeBV = edgeBV - originV;
        edgeCU = edgeCU - originU;
        edgeCV -= originV;
        scratch.projectedPointV = pointV;
        scratch.projected.pointU = pointDeltaU;
        float pointDeltaV = pointV - originV; scratch.projected.pointV = pointDeltaV;
        scratch.projected.edgeBU = ((vertexB[0]) - originU);
        scratch.projected.edgeBV = edgeBV;
        scratch.projected.edgeCU = edgeCU;
        scratch.projected.edgeCV = edgeCV;
    } else {
        // Drop Z and solve in the X/Y plane.
        float edgeBV = vertexB[1];
        float originU = vertexA[0];

        float pointU = point[0];
        float edgeBU = (vertexB[0]) - originU;
        float pointV = point[1];

        float originV = vertexA[1]; float pointDeltaU = pointU - originU;

        edgeBV = edgeBV - originV;
        float edgeCU = vertexC[0];
        scratch.projectedPointU = pointU; float edgeCV = vertexC[1];
        scratch.projectedPointV = pointV; edgeCU = edgeCU - originU;
        edgeCV -= originV;
        scratch.projected.pointU = pointDeltaU;
        float pointDeltaV = pointV - originV; scratch.projected.pointV = pointDeltaV;
        scratch.projected.edgeBU = edgeBU;
        scratch.projected.edgeBV = edgeBV;
        scratch.projected.edgeCU = edgeCU;
        scratch.projected.edgeCV = edgeCV;
    }
    // solve the 2x2 system
    float weightBNumerator = scratch.projected.edgeCV * scratch.projected.pointU - scratch.projected.edgeCU * scratch.projected.pointV;
    float determinant = scratch.projected.edgeBU * scratch.projected.edgeCV - scratch.projected.edgeCU * scratch.projected.edgeBV;
    float weightCNumerator = scratch.projected.edgeBU * scratch.projected.pointV - scratch.projected.pointU * scratch.projected.edgeBV;
    *weightB = weightBNumerator / determinant;
    *weightC = weightCNumerator / determinant;
}

extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void __destroy_new_array(void*, void* (*)(void*, int));

// Release one group's raw-data allocation; optionally delete that group object.
// The array destructor below invokes this callback once per group.
void* CourseSurfaceGroup_Destroy(void* self, int shouldDelete) {
    if (self != 0) {
        OwnedArray* array = (OwnedArray*)self;
        if (array->data != 0) {
            __dla__FPv(array->data);
            array->data = 0;
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Destroy an array of owned records, invoking each record's cleanup callback.
void* CourseSurfaceGroupArray_Destroy(void* self, int shouldDelete) {
    if (self != 0) {
        OwnedArray* array = (OwnedArray*)self;
        if (array->data != 0) {
            __destroy_new_array(array->data, CourseSurfaceGroup_Destroy);
            array->data = 0;
        }
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

struct Stream {
    virtual void v0();
    virtual void v1();
    virtual void read(void*, int);
    virtual void v3();
    virtual void v4();
    virtual void skip(int, int);
};
// Partial view of one node in the chain that starts at the fallback model's boneRoot->firstChild
// (walked through `next`). Serialized node fingerprints use the id and comparison tag; links and packed index follow.
struct ChainNode {
    int id;
    ChainNode* firstChild;
    char pad_008[0x28];
    int comparisonTag;
    char pad_034[0x30];
    ChainNode* next;
    char pad_068[0x38];
    unsigned int packedIndex;
};
// Two owned group arrays: common block at +0/+4; course-variant-selected block
// at +8/+12. Variant 1 reads the first optional block, variant 2 the second.
// Surface category names are unresolved. count/data own COMMON groups;
// count2/data2 own the SELECTED optional groups; neither array is a triangle mapping.
struct SurfaceGroupArrays { int commonGroupCount; void* commonGroups; int selectedGroupCount; void* selectedGroups; };
// This is exactly LocalArray[2] at the loader's call boundary:
//   SurfaceGroupArrays offset   LocalArray index   role
//   +00/+04        result[0]          common groups (always considered)
//   +08/+0C        result[1]          selected optional groups
// Each data allocation holds count-sized {s32 count, owned raw-data pointer}
// group descriptors, not ChainNode objects. The selected variant allocation currently
// uses COMMON count while its loop uses SELECTED count; retain that discrepancy.
// Variant policy (low courseFlags nibble; unknown values have no switch branch):
//   0/no course: skip optional A, skip optional B
//   1:           read A's count, skip B after the selected-group loop
//   2:           skip A, read B's count
// The common block is length-prefixed too, but its payload is not reconstructed.
struct CourseDataView {
    char pad_000[0x10];
    void* resourceManager;
    char pad_014[0xC];
    char modelPath[0x88];
    SurfaceModelRecord* holder;
    char rest[0x4874];
    int listCount;
    void* listData;
    ChainNode* listHead;
};
// CourseDataView is a partial view of the SAME CourseData801C, not a separate allocation.
// +10 resource manager, +20 model path, +A8 fallback model (holder);
// +4920/+4924 generated-index count/owned u32 array, +4928 generated-chain head.
// `rest` covers live unmodeled fields. In particular it must not be zeroed as pad.
typedef char CourseDataView_fallback_offset[((int)&((CourseDataView*)0)->holder == 0xA8) ? 1 : -1];
typedef char CourseDataView_indices_offset[((int)&((CourseDataView*)0)->listCount == 0x4920) ? 1 : -1];
typedef char SurfaceGroupPair_size[(sizeof(SurfaceGroupArrays) == 16) ? 1 : -1];
extern void* __nwa__FUl(unsigned int);
extern void* __construct_new_array(void*, void*, void*, unsigned int, unsigned int);
extern CSharedDoc* g_sharedDoc;

extern void* strrchr(void*, int);
extern void* strncpy(void*, const void*, int); /* (dest, source, count): strncpy-like, reads r3-r5 */
extern void* strcat(void*, const char*);
extern int Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
extern void Resource_Release(void*);
extern unsigned int strlen(const char*);
extern void __destroy_new_array(void*, void* (*)(void*, int));
extern char courseModelResourceStrings[];
extern int CourseData_ReadSurfaceTriangleMappings(void*, void*, int, void*, void*, void*);
struct RawTriple;
extern void CourseData_ReadSurfaceGroups(CourseDataView*, SurfaceGroupArrays*, Stream*,
    ChainNode**, RawTriple*, void*);


struct InputFile {
    virtual void unknown0();
    virtual void unknown1();
    virtual void read(void*, int);
};
struct FileScope {
    InputFile** ptr;
    // Track the local file slot so explicit release can null it before scope exit.
    FileScope(InputFile** p) : ptr(p) {}
    // Release the opened file on every return; this does not clean raw temporary arrays.
    ~FileScope() {
        if (*ptr) {
            if (*ptr) { Resource_Release(*ptr); *ptr = 0; }
        }
    }
};
struct LocalArray {
    int count;
    void* data;
    // Initialize an empty owned surface-group array.
    LocalArray() : count(0), data(0) {}
    // Destroy each group's raw-data array, then its array allocation/cookie.
    ~LocalArray() {
        if (data) {
            __destroy_new_array(data, CourseSurfaceGroup_Destroy);
            data = 0;
        }
    }
};
// Serialized 12-byte record read verbatim. a/b/c are three native-endian 32-bit
// words supplied to downstream parsing; their individual roles remain unresolved.
// Keep this distinct from the separate 8-byte raw record array named pairs below.
struct RawTriple {
    int a, b, c; // Opaque serialized words 0, 1, 2; no geometric/vector meaning established.
    // Retain the nontrivial array-destruction ABI/cookie without per-record cleanup.
    ~RawTriple() {}
    // Allocate array storage including the compiler's cookie.
    static void* operator new[](new_size_t n) { return __nwa__FUl(n); }
    // Release the original array allocation, not its post-cookie element pointer.
    static void operator delete[](void* p) { __dla__FPv(p); }
};
typedef char SerializedTriple_size[(sizeof(RawTriple) == kSerializedTripleBytes) ? 1 : -1];
// One surface-group descriptor {a, b}: b is an owned raw-data array freed by the destructor. (The tag name
// is part of the mangled symbol and cannot be changed without altering the object file.)
struct Elem_u801C482C {
    int a;   // opaque word initialized to zero; no per-record schema established
    void* ownedRawData; // owned raw-record array
    // Initialize an empty group descriptor and its owned raw-data pointer.
    Elem_u801C482C() { a = 0; ownedRawData = 0; }
    // Free the group's raw data, without interpreting record contents.
    ~Elem_u801C482C() { if (ownedRawData) { __dla__FPv(ownedRawData); ownedRawData = 0; } }
    // Allocate group-array storage with its compiler-managed cookie.
    static void* operator new[](new_size_t n) { return __nwa__FUl(n); }
    // Release group-array allocation storage after element destruction.
    static void operator delete[](void* p) { __dla__FPv(p); }
};

// Call the preserved six-register ABI of the partial group reader with borrowed lookup tables.
// All six ABI arguments are now explicit. The partial reader still consumes only the
// first three; making the borrowed tables visible does not recover their payload schema.
static inline void CallPartialSurfaceGroupReader(CourseDataView* owner, LocalArray* result,
    InputFile* file, ChainNode** borrowedNodes, RawTriple* borrowedTriples, void* borrowedPairs) {
    CourseData_ReadSurfaceGroups(owner, (SurfaceGroupArrays*)result, (Stream*)file,
        borrowedNodes, borrowedTriples, borrowedPairs);
}
// Preserve the missing constructor's observable placeholder, including nulling the append cursor.
// This is NOT node creation: after one append, another append would dereference null.
// Required cursor names an existing borrowed tail. It loses its next link and
// becomes null; no allocation, ownership transfer or group payload read occurs.
static inline void AppendUnconstructedSurfaceNode(ChainNode*& appendCursor) {
    appendCursor->next = 0;
    appendCursor = 0;
}

#pragma use_lmw_stmw on
// PARTIAL loader: validate the .sbin prefix and invoke the unreconstructed surface-group path.
// Format boundary (Wii native-endian words, length values are bytes):
//   known prefix      fingerprints and opaque lookup arrays described below
//   common block      s32 byteLength; if positive, s32 groupCount; payload UNKNOWN
//   optional block A  s32 byteLength; skip byteLength or read selected groupCount
//   optional block B  s32 byteLength; skip byteLength or read selected groupCount
//   triangle mappings downstream reader owns this schema, not decoded in this unit
// The partial reader does not advance over selected group payloads. Consequently
// later block positions cannot be inferred for positive counts from this source.
// Do not add reads, node allocation, count validation or cleanup while editing names/layouts:
// those would repair/change this baseline rather than clarify its implemented behavior.
// Serialized order (read verbatim in Wii native byte order, with no swapping here):
// s32 boneCount; boneCount*{s32 id,s32 comparisonTag}; s32 tripleCount;
// s32 pairCount; tripleCount*12 raw bytes; pairCount*8 raw bytes;
// then surface-group blocks and triangle mappings consumed by the two readers.
// Return 0 for invalid path/open/fingerprint or both record counts zero, 1 after parsing.
// FileScope releases the file on every return; raw nodes/triples/pairs have explicit
// cleanup ONLY on the successful path. Fingerprint/count failures after nodes allocation
// leak nodes in this existing implementation. LocalArray cleans result groups at exit.
// Temporary-table contract at the two downstream calls:
// nodes: sibling-order ChainNode* table, count entries, borrowed existing model nodes;
// triples: tripleCount native 3-word opaque records, owned temporary new[];
// pairs: pairCount native 2-word opaque records, owned temporary raw allocation;
// result: two owned group arrays, destroyed before this function returns.
// Call preconditions: self/resourceManager/fallbackSurface/puppet/boneRoot and
// shared document must be valid; the bone sibling chain must be finite. Input
// stream must provide every requested byte, positive counts must fit allocation
// arithmetic, and reads/skips must succeed: the stream methods return no checked
// status here. Truncated/malformed files have no defined recovery contract.
// Source-grounded return/ownership matrix ("failure" is not transactional):
//   no extension/path too long/open fails -> 0; no temporary arrays allocated
//   node count or fingerprint mismatch   -> 0; file closes, node table leaks
//   tripleCount == pairCount == 0        -> 0; file closes, node table leaks
//   exception after allocating tables   -> unwinds file/group guards only;
//                                           raw table allocations can leak
//   full normal path                    -> 1; tables/groups/file released;
//                                           generated index allocation stays in self
// Repeated successful loads overwrite self->listData without releasing its old
// allocation here. Caller must arrange previous generated-state cleanup first.
// The parseable subset has no positive common/selected group count AND no
// unread selected payload; common blocks with nonpositive length are safest.
// Optional unselected blocks are skipped by length. Even in that subset the
// downstream triangle-mapping reader, not this return value, determines usability.
// No returned surface-group pointer outlives result. Persistent generated links
// and index storage are published through self by the reader (currently incomplete).
int CourseData_LoadSurfaceBinary(CourseDataView* self) {
    // Phase 1: replace model extension with .sbin within the fixed 64-byte path buffer.
    char path[kSurfaceSidecarPathBytes] = {0};
    void* end = strrchr(self->modelPath, '.');
    if (!end) return 0;
    int length = (int)strlen(courseModelResourceStrings + kSurfaceBinaryExtensionOffset);
    char* start = self->modelPath;
    int distance = (char*)end - start;
    int remaining = (kSurfaceSidecarPathBytes - 1) - length;
    if (distance >= remaining) return 0;

    strncpy(path, start, distance); /* 0x801C4B8C-0x801C4BAC: r4 = self+0x20, r5 = end - start (ours passed no source or count) */
    strcat(path, courseModelResourceStrings + kSurfaceBinaryExtensionOffset);
    InputFile* file = (InputFile*)Resource_OpenResolvedPath(self->resourceManager, path, -1, 0, 0, 0);
    FileScope scope(&file);
    int tripleCount, pairCount, serializedNodeId, serializedNodeTag;
    if (!file) return 0;

    // Phase 2: index existing sibling bones and verify serialized fingerprints in order.
    ChainNode* head = (ChainNode*)((SurfaceModelRecord*)AsCourseData801C(self)->fallbackSurface)->puppet->boneRoot; // CourseData_LoadSurfaceBinary: puppet +0x164
    ChainNode* first = head->firstChild;
    ChainNode* node = first;
    int count = 0;
    for (ChainNode* nextNode = node; nextNode; nextNode = nextNode->next) ++count;
    ChainNode** nodes = (ChainNode**)__nwa__FUl(count * sizeof(ChainNode*));
    int index = 0;
    for (node = first; node;) { nodes[index] = node; node = node->next; ++index; }

    file->read(&index, kSerializedWordBytes);
    if (index != count) return 0; // FileScope closes; raw nodes is not guarded.
    for (index = 0; index < count; ++index) {
        file->read(&serializedNodeId, kSerializedWordBytes);
        file->read(&serializedNodeTag, kSerializedWordBytes);
        if (serializedNodeId != nodes[index]->id || serializedNodeTag != nodes[index]->comparisonTag) return 0; // nodes leaks, same as count mismatch.
    }
    // Phase 3: read opaque lookup records. Their counts are trusted, not validated as sizes.
    pairCount = 0; tripleCount = 0;
    file->read(&tripleCount, kSerializedWordBytes);
    file->read(&pairCount, kSerializedWordBytes);
    RawTriple* triples;
    if (tripleCount > 0) {
        triples = new RawTriple[tripleCount];
        file->read(triples, tripleCount * kSerializedTripleBytes);
    } else triples = 0;
    void* pairs;
    if (pairCount > 0) {
        pairs = __nwa__FUl(pairCount * kSerializedPairBytes);
        file->read(pairs, pairCount * kSerializedPairBytes);
    } else pairs = 0;
    if (tripleCount == 0 && pairCount == 0) return 0; // Existing early failure still owns raw nodes.
    // Phase 4: parse common/variant groups, then mappings, using borrowed temporary tables.
    // PPC r3..r8 carry owner, result, file, nodes, triples, pairs in that order.
    // Only r3..r5 are consumed by the partial reader. The remaining borrowed tables
    // are exposed in its signature for research, but their group-record relationships
    // and the missing node construction must be recovered together before extending it.
    LocalArray result[2];
    CallPartialSurfaceGroupReader(self, result, file, nodes, triples, pairs);
    CourseData_ReadSurfaceTriangleMappings(self, file, count, nodes, triples, &result);
    // Phase 5: explicit successful-path cleanup; RawTriple's array cookie is 16 bytes.
    if (triples) { if (triples) __dla__FPv((char*)triples - kArrayCookieBytes); }
    if (pairs) __dla__FPv(pairs);
    __dla__FPv(nodes);
    int maximum = 0;
    for (; first; first = first->next) {
        if (maximum < first->id) maximum = first->id;
    }
    if (file) { Resource_Release(file); file = 0; }
    return 1;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// PARTIAL parser: read group counts/skip policy and publish an empty generated chain.
// This is a framing operation with incomplete payload handling, not a working
// node factory. Inputs owner/arrays/stream and the shared document are required;
// arrays must be two empty owned group-array descriptors. Borrowed lookup-table
// arguments are UNUSED here; no schema can be inferred from them here.
// Generated state publication always writes listHead=0, listCount=0 and allocates
// zero-byte listData on a normal return, since no path can make head nonnull.
// It may also truncate the existing bone chain before failing. No rollback.
// Failure table for appends, independent of optional-block framing:
//   0 appends      -> no tail mutation; proceed to empty-state publication
//   1 append       -> requires old tail; clears its next, cursor becomes null
//   2+ appends     -> second append dereferences null (across either group loop)
// Thus group payload recovery requires an intentional behavior repair, including
// the constructor and count mismatch; a new name/helper cannot supply that data.
// Current branch behavior (NOT the intended complete file format):
// common count <=0: no appends; =1: clear old tail->next, set head null; >1: null dereference
// selected count >0: allocate COMMON count descriptors; same null-append limitation
// Even the first append requires a nonnull existing tail; a common append leaves it null for the selected block.
// variant 0: skip A,B; variant 1: select A, later skip B; variant 2: skip A, select B
// other variants: consume no optional headers. No positive-count payload is decoded.
// Partial reconstruction: common-block node construction is missing. The loop sets
// link->next=null and link=null; a second iteration dereferences null. The variant
// loop has the same problem and allocates count elements while iterating count2.
// Do not treat positive group counts as a working parser or add node fields by guess.
// stream/owner are borrowed; arrays owns allocations; owner receives the index array.
// Modification boundary: size/offset names and skip-policy documentation can be
// changed independently. Implementing group payloads requires the missing ChainNode
// allocation/initialization, table arguments, and actual serialized group format;
// assigning a new ChainNode here without those contracts would invent behavior.
void CourseData_ReadSurfaceGroups(CourseDataView* self, SurfaceGroupArrays* arrays, Stream* stream,
    ChainNode** /*borrowedNodes*/, RawTriple* /*borrowedTriples*/, void* /*borrowedPairs*/) {
    ChainNode* link = (ChainNode*)self->holder->puppet->boneRoot; // CourseData_ReadSurfaceGroups: puppet +0x164
    link = link->firstChild;
    while (link != 0 && link->next != 0) link = link->next;

    // Phase 1: common block length/count; generated ChainNode construction remains unresolved.
    int blockLength;
    stream->read(&blockLength, kSerializedWordBytes);
    ChainNode* head = 0;
    if (blockLength > 0) {
        stream->read(&arrays->commonGroupCount, kSerializedWordBytes);
        if (arrays->commonGroupCount > 0) {
            int count = arrays->commonGroupCount;
            arrays->commonGroups = new Elem_u801C482C[count];
        }
        if (arrays->commonGroupCount > 0) {
            for (int i = 0; i < arrays->commonGroupCount; ++i) {
                // Existing decompilation leaves generated-node construction unresolved here.
                AppendUnconstructedSurfaceNode(link);
                if (i == 0) head = link;
            }
        }
    }
    // Course variant = low courseFlags nibble of the selected course (0 when no course is selected).
    IFF_STRUCT::sCourse* courseRecord = (IFF_STRUCT::sCourse*)AsCSharedDoc(g_sharedDoc)->selectedCourseData;
    unsigned char courseVariant = (courseRecord == 0) ? 0 : (courseRecord->courseFlags & kCourseVariantMask);
    // Phase 2: optional blocks are byte-length-prefixed. Variant controls read vs skip.
    switch (courseVariant) {
    case kSkipBothVariantBlocks:
        stream->read(&blockLength, 4);
        stream->skip(blockLength, kSeekRelative);
        stream->read(&blockLength, 4);
        stream->skip(blockLength, kSeekRelative);
        break;
    case kReadFirstVariantBlock:
        stream->read(&blockLength, 4);
        if (blockLength > 0) stream->read(&arrays->selectedGroupCount, kSerializedWordBytes); // CourseData_ReadSurfaceGroups: second array count +8
        break;
    case kReadSecondVariantBlock:
        stream->read(&blockLength, 4);
        stream->skip(blockLength, kSeekRelative);
        stream->read(&blockLength, 4);
        if (blockLength > 0) stream->read(&arrays->selectedGroupCount, 4);
        break;
    }
    // Phase 3: variant group's node-construction placeholder; see contract above.
    if (arrays->selectedGroupCount > 0) {
        int count = arrays->commonGroupCount;
        arrays->selectedGroups = new Elem_u801C482C[count];
        for (int i = 0; i < arrays->selectedGroupCount; ++i) {
            // Preserve the existing unresolved node-construction path, including its null assignments.
            AppendUnconstructedSurfaceNode(link);
        }
    }
    courseRecord = (IFF_STRUCT::sCourse*)AsCSharedDoc(g_sharedDoc)->selectedCourseData;
    courseVariant = (courseRecord == 0) ? 0 : (courseRecord->courseFlags & kCourseVariantMask);
    if (courseVariant == kReadFirstVariantBlock) {
        stream->read(&blockLength, 4);
        stream->skip(blockLength, kSeekRelative);
    }
    // Phase 4: publish generated-node chain and its low-11-bit indices (not pointers).
    self->listHead = head;
    self->listCount = 0;
    for (ChainNode* p = head; p; p = p->next) ++self->listCount;
    self->listData = __nwa__FUl(self->listCount * kSerializedWordBytes);
    int index = 0;
    for (ChainNode* p = self->listHead; p; p = p->next) ((unsigned int*)self->listData)[index++] = p->packedIndex & kGeneratedNodeIndexMask;
}
#pragma use_lmw_stmw reset

} // extern "C"
