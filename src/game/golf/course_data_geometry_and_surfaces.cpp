// Course geometry queries: point tests and clamping against the playable region, lookups of
// course entries, names, bounds, surfaces and points by name or index, and surface particle
// settings. CourseData_UpdateWindActors updates the course's wind actors.

#pragma push
#include "classes/CFxSequence.h"
#include "classes/WPuppet.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/CourseBounds801C.h"
#include "classes/CourseSurface801C.h"
#include "classes/CourseResource801C.h"
#include "classes/CourseRegion801C.h"
#include "classes/MWStringStorage.h"
#include "classes/CourseData801C.h"
#include "classes/CQuadTree.h"
#define AsCQuadTree(p) ((CQuadTree*)(p))
#include "classes/Polygon801C.h"
#include "game/types.h"
#include "game/wvector.h"
#include "iff/IffStruct.h"
// Course geometry, surface lookups, model tags and wind-linked effects.
// Mod lookup: boundary selection -> CourseData_ClampPointToActiveRegion/region types;
// model metadata -> ApplyModelNodeTags grammar below; particle tuning -> surface
// property accessors and UpdateWindActors. Scalars: the data section; layouts:
// CourseData801C, Polygon801C and Course*801C headers. Lookup pointers are borrowed.

#include "sdk_protos.h"
#include "classes/CWind.h"
#include "classes/CRival.h"
#include "classes/CFx.h"
#include "classes/CFxSpray.h"

// The public surface/point views describe prefixes; file-record strides include trailing data.
enum CourseRecordConstants {
    kFallbackSurfaceIndex = 0xFFFF,
    kOpaqueParticle = 0xFF,
    kSurfaceRecordBytes = 0xDC,
    kNameRecordBytes = 0x58,
    kNamedPointRecordBytes = 0x3C,
    kBoundsRecordBytes = 0x5C,
    kRegionRecordBytes = 0x20,
    kCourseEntryBytes = 0x88,
    kPrimaryBoundaryRegion = 1,
    kExclusionRegion = 2,
    kAdditionalBoundaryRegion = 3,
    kAlternateBoundaryRegion = 6,
    kSkipAttachedEffect = 0x40000,
    kWindActorStride = 0x48,
    kWindActorSlotsOffset = 0xD8
};

// Lookup results borrow storage from CourseData; releasing/reloading the course
// invalidates them. Most name lookups return null on a miss. FindSurfaceByName
// uses fallbackSurface only for a null/empty query, not an unmatched nonempty one.
// Surface indices must be in range or exactly 0xFFFF; other indices are unchecked.
// Polygon/point operations use world XYZ, with geometric tests projected to XZ.
// Region types: 1/3 require containment, 2 excludes containment; clamp selects
// the first type 1 (regionMode=0) or type 6 (regionMode!=0), ignoring type 3.
enum CourseTagOffset {
    kSoundParameters = 0xE1,
    kNpcTag = 0x122,
    kCenterTag = 0x132,
    kWindParameter = 0x139,
    kFloatFormat = 0x13F,
    kSoundTag = 0x142,
    kEffectTag = 0x196,
    kSprayExtension = 0x1B7,
    kSequenceExtension = 0x1BB,
    kNightOnlyTag = 0x20B,
    kDayOnlyTag = 0x216,
    kNoCollisionTag = 0x21F,
    kIntegerFormat = 0x226,
    kNoFogTag = 0x229,
    kBlendTag = 0x230,
    kAboveTag = 0x237,
    kEffectParameters = 0x23B
};
struct CourseEntryContainer {
    char padding00[0x14];
    char entries[1]; // Address of the embedded named-object table.
};
struct CourseModelContext {
    unsigned int reserved;
    int collisionDisabled;
    float windScale;
    unsigned int reserved0C;
    WPuppet* puppet;
};

// Two pointer slots per wind actor, separated from the next pair by a 0x48-byte stride.
// This view describes only the slots, not the complete actor allocation.
struct CourseWindActorSlots {
    void* spray;
    void* sequence;
};
// Only these descriptor fields are established by this caller; all others are
// initialized by FxAttachment_Init. Both pointers borrow model/bone storage.
struct CourseFxAttachmentView {
    char prefix[8];
    void* worldTransform;
    char reserved0C[0x0C];
    void* localPosition;
};
typedef char CourseFxTransform_offset[IFF_OFFSET(CourseFxAttachmentView, worldTransform) == 8 ? 1 : -1];
typedef char CourseFxPosition_offset[IFF_OFFSET(CourseFxAttachmentView, localPosition) == 0x18 ? 1 : -1];

// CFxSpray's shared header leaves its emitter-position region opaque.
struct CourseSprayPositionView {
    char effectState[0xB8];
    WVector worldPosition;
};
struct CourseBoneFlagsView {
    char boneState[0x74];
    unsigned int flags;
};
typedef char CourseSprayPosition_offset[IFF_OFFSET(CourseSprayPositionView, worldPosition) == 0xB8 ? 1 : -1];
typedef char CourseBoneFlags_offset[IFF_OFFSET(CourseBoneFlagsView, flags) == 0x74 ? 1 : -1];

extern "C" void appFreeUni__FPv(void*);

struct GenGuard {
    void* head;
    void* n;
    // Leave both slots uninitialized; callers assign directly or use init().
    // Destructor requires n initialized. Unwind before that assignment is unsupported.
    // head records the destination list address; this constructor acquires nothing.
    GenGuard() {}
    // Free an uncommitted node if constructing its payload throws.
    ~GenGuard() {
        if (n)
            appFreeUni__FPv(n);
    }
    // Adopt a node until the caller clears n after successful construction.
    void init(void* p, void** h) {
        head = (void*)h;
        n = p;
    }
};

extern "C" {

extern void* g_rival;
extern void* EntryList_GetEntryOrFallback(void* self, int index);
extern void* NamedObjectTable_FindOrFallback(void* self, const char* name);
extern void String_ReplaceRange(void* a, void* b, void* c, int d, int e, int f, unsigned char* out);
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern void* CQuadTree_GetInstance(void);
extern void CourseTriangleGrid_Rebuild(void*, void*);
extern int String_CompareCString(const char*, const char*);
extern int CString_Compare(const char*, const char*);
extern char* MSLString_LowercaseInPlace(const char*);
extern int Polygon_ContainsPointXZ(void*, void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void Vector_CrossProduct(void*, void*, void*);
extern float __ml__FRC7WVectorRC7WVector(void*, void*);
extern float Magnitude__7WVectorCFv(void*);
extern void Normalize__7WVectorFv(void*);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern double acos(double);
extern float Math_SinApprox(float);
extern void Polygon_ClampPointAlongRayXZ(void*, void*, void*, void*, void*);
extern void MWString_CopyConstructForNameLookup(void*, void*);
extern "C" void GetWindVector__5CWindFv(void* result, const void* self);
extern "C" void GetWindVectorAtPoint__5CWindFRC7WVectorf(void* result, const void* self, const void* point,
                                                         float maxBaseMagnitude);
extern void ResourceManager_ReleaseId(void*, void*);
extern void ResourceManager_ReleaseObject(void*, void*, int);
extern void* g_resourceManager;
extern void* g_fx;
extern float kFloat_99999_CourseEffectWindQuery;
extern void* g_sharedDoc;
extern float kFloat_neg1_CourseModel;
extern float kCourseSoundDefaultParameter60;
extern float kFloat_0_PetBody;
extern float kFloat_1_polygon;
extern float kFloat_0_5_CourseSoundPosition;
extern void WPuppet_SetBoneVisibilityRecursive(void*, bool, bool, void*);
extern void String_ScanTokensAfterSubstring(void*, const char*, const char*, ...);
extern void Matrix_MoveLastVectorToFront(void*, void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void AffineVectors_ScaleTrailingThree(void*, void*);
extern void* Matrix_TransformPoint(void*, void*, void*);
extern void Matrix_CopyAffineElements(void*, void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* FxAttachment_Init(void*);
extern char courseModelResourceStrings[];
extern void WPuppet_ClearRenderStateValue(void*);
extern void WPuppet_HideBoneSelection(void*, void*);
extern void appFreeUni__FPv(void*);

// Return an indexed fixed-size course entry without bounds checking.
void* Array_GetElement88(void** self, int index);

// Initialize an empty tagged narrow string.
void String_InitEmptyStorage(void* self);

// Read a surface's particle wind scale, using the fallback surface for the sentinel index.
float CourseSurfaceProperties_GetParticleWindScale(void* self, int index);

// Adapt a source pointer/count into the half-open source range for string replacement.
// In C terms: String_ReplaceRange(str, pos, n, src, src + srcLen, <local tag byte>, forwardedArg7);
// stringStorage and erasePosition/eraseCount are forwarded unchanged (the latter
// two retain pointer-shaped word types). Source addresses/counts are byte units.
// Args 6/8/9 are unused; arg 7 is forwarded as the trailing byte-pointer argument,
// whose meaning is unresolved. The volatile byte preserves allocator-tag ABI setup.
// No allocation ownership is transferred by this adapter itself.
void String_ReplaceRangeCountAdapter(void* stringStorage, void* erasePosition, void* eraseCount, int sourceBeginAddress,
                                     int sourceByteCount, int unusedArg6, int forwardedArg7, int unusedArg8,
                                     unsigned char unusedArg9);

// Release a tagged string's heap storage and optionally delete the string object.
void* dtor_801C315C(void* self, int deleting);

// Read particle opacity, treating the fallback index as fully opaque.
unsigned char CourseSurfaceProperties_GetParticleOpacity(void* self, int index);

// Return the active rival singleton.

// Look up a course entry through the embedded table's indexed fallback lookup.
void* CourseData_GetEntryByIndex(void* self, int index);

// Look up a course entry through the embedded table's named fallback lookup.
void* CourseData_FindEntryByName(void* self, const char* name);

// Rebuild the course triangle grid and register every surface with the spatial tree.
void CourseData_BuildQuadTree(void* self);

// Search fixed-size named course records using the original comparison routine.
void* CourseData_FindNamedRecord(void* self, const char* name);

// Find a polygon-region record by its tagged string name.
void* NameTable_FindByName(void* self, const char* name);

#pragma opt_lifetimes off
// Update global effects and each course spray/sequence actor from the current wind.
void CourseData_UpdateWindActors(void* self);
#pragma opt_lifetimes reset

// Release course handles and objects; free the handle array but retain object-vector capacity.
void CourseData_ReleaseResources(void* self);

// Find a course name record by exact string comparison.
void* CourseData_FindNameRecord(void* self, const char* name);

// Find a named point, skipping the record's leading flag byte.
void* CourseData_FindNamedPoint(void* self, const char* name);

// Find a named surface, using the default surface for a missing or empty name.
void* CourseData_FindSurfaceByName(void* self, const char* name);

// Use quadrant winding to test a point against a polygon projected onto the XZ plane.
// Returns 1 only for a total winding of +/-4 quarter turns; fewer than three
// vertices returns 0. This does not define a separate point-on-edge policy.
int Polygon_ContainsPointXZ(void* self, void* polygon, void* pointPosition);

// Clamp a point's distance along a reference ray to the polygon boundary in XZ.
// output receives world XYZ. Requires at least two vertices and a nonzero XZ
// ray; the selected edge must give a nonzero sine-rule denominator. The result's
// Y is rayOrigin.Y, even if pointPosition.Y differs. No validation is added here.
void Polygon_ClampPointAlongRayXZ(void* output, void* self, void* polygon, void* pointPosition, void* rayOrigin);
#pragma push
#pragma use_lmw_stmw on
// Find the first bounds record whose name contains the requested substring.
void* CourseData_FindBoundsByNameSubstring(void* self, const char* name);
#pragma pop

// Find the first axis-aligned bounds containing XYZ, including boundary faces.
void* CourseData_FindBoundsContainingPoint(void* self, float* point);

// Reject points outside required regions or inside exclusion regions, then consult the spatial tree.
int CourseData_IsPointOutsidePlayableRegion(void* self, void* arg);

// Clamp a point to the first boundary polygon selected by the course's region mode.
void CourseData_ClampPointToActiveRegion(void* out, void* self, int* xyz, void* arg);

#pragma push
#pragma opt_loop_invariants off
// A generated sound list node owns this 0x5C-byte payload after its two links:
// 32-byte sound name, three opaque sound parameters,
// and a 3x4 affine transform in translation-first engine order. parameters[0]
// starts at zero; the tag scanner writes [2] then [1]. Their units are unresolved.
struct GeneratedSoundPayload {
    unsigned char soundName[0x20];
    float parameters[3];
    float transform[12];
};
struct GenValue {
    // Copy a generated sound payload into node storage without changing copy widths.
    GenValue(GeneratedSoundPayload& payload) {
        char* dst = (char*)this;
        unsigned char b, a;
        b = payload.soundName[0];
        a = payload.soundName[1];
        dst[0] = b;
        dst[1] = a;
        b = payload.soundName[2];
        a = payload.soundName[3];
        dst[2] = b;
        dst[3] = a;
        b = payload.soundName[4];
        a = payload.soundName[5];
        dst[4] = b;
        dst[5] = a;
        b = payload.soundName[6];
        a = payload.soundName[7];
        dst[6] = b;
        dst[7] = a;
        b = payload.soundName[8];
        a = payload.soundName[9];
        dst[8] = b;
        dst[9] = a;
        b = payload.soundName[10];
        a = payload.soundName[11];
        dst[10] = b;
        dst[11] = a;
        b = payload.soundName[12];
        a = payload.soundName[13];
        dst[12] = b;
        dst[13] = a;
        b = payload.soundName[14];
        a = payload.soundName[15];
        dst[14] = b;
        dst[15] = a;
        b = payload.soundName[16];
        a = payload.soundName[17];
        dst[16] = b;
        dst[17] = a;
        b = payload.soundName[18];
        a = payload.soundName[19];
        dst[18] = b;
        dst[19] = a;
        b = payload.soundName[20];
        a = payload.soundName[21];
        dst[20] = b;
        dst[21] = a;
        b = payload.soundName[22];
        a = payload.soundName[23];
        dst[22] = b;
        dst[23] = a;
        b = payload.soundName[24];
        a = payload.soundName[25];
        dst[24] = b;
        dst[25] = a;
        b = payload.soundName[26];
        a = payload.soundName[27];
        dst[26] = b;
        dst[27] = a;
        b = payload.soundName[28];
        a = payload.soundName[29];
        dst[28] = b;
        dst[29] = a;
        b = payload.soundName[30];
        a = payload.soundName[31];
        dst[30] = b;
        dst[31] = a;
        *(float*)(dst + 0x20) = payload.parameters[0];
        *(float*)(dst + 0x24) = payload.parameters[1];
        *(float*)(dst + 0x28) = payload.parameters[2];
        Matrix_CopyAffineElements(dst + 0x2C, payload.transform);
    }
};
// Use caller-provided node storage for placement construction.
inline void* operator new(size_t, void* p) {
    return p;
}
// Placement cleanup does not own storage; GenGuard frees an uncommitted node on unwind.
inline void operator delete(void*, void*) {}
// Interpret model tags for day/night visibility, wind, collision, sound nodes, and attached effects.
// Per named bone (item+0 = bone resource, item+4 = node whose +0x2C is the tag/name string and
// +0x28 the free-form parameter text `name`), at most ONE of these primary tags is handled,
// in this order (tag strings are kXxxTag offsets into courseModelResourceStrings):
//   kNightOnlyTag : acts only when isDaytime != 0  -> WPuppet_SetBoneVisibilityRecursive(bone,1,1)
//   kDayOnlyTag   : acts only when isDaytime == 0  -> same visibility call
//   kCenterTag    : parse wind scale + collision-disabled flag from `name`; no-fog substring clears
//                   the puppet render-state value (the blend-substring search result is discarded)
//   kNpcTag       : nothing further
//   kSoundTag     : build a GeneratedSoundPayload and append a heap node to generatedHead
//   other         : if the lowercased tag contains kAboveTag, hide that bone selection
// Independently of the above, any bone whose parameter text contains kEffectTag attaches an
// effect (only for .spr / .seq resources and bones without kSkipAttachedEffect).
// To change one policy edit only its branch; the effect phase at the end is separate.
void CourseData_ApplyModelNodeTags(void* self, void* context);

#pragma pop

} // extern "C"

#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
void* Array_GetElement88(void** self, int index) {
    return (char*)*self + index * kCourseEntryBytes;
}
#pragma pop

#pragma push
void* CRival::GetInstance() {
    return g_rival;
}
#pragma pop

#pragma push
void String_ReplaceRangeCountAdapter(void* stringStorage, void* erasePosition, void* eraseCount, int sourceBeginAddress,
                                     int sourceByteCount, int unusedArg6, int forwardedArg7, int unusedArg8,
                                     unsigned char unusedArg9) {
    struct Tag {
        unsigned char byte;
        char pad[3];
    };
    Tag local;
    volatile Tag tag;
    local.byte = tag.byte;
    String_ReplaceRange(stringStorage, erasePosition, eraseCount, sourceBeginAddress,
                        sourceBeginAddress + sourceByteCount, (int)&local, (unsigned char*)forwardedArg7);
}
#pragma pop

#pragma push
void String_InitEmptyStorage(void* self) {
    (*(int*)&AsMWStringStorage(self)->flags) = 0;
    (*(int*)&AsMWStringStorage(self)->length) = 0;
    (*(int*)&AsMWStringStorage(self)->data) = 0;
}
#pragma pop

#pragma push
void* dtor_801C315C(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            if (((AsMWStringStorage(self)->flags >> 31) != 0))
                appFreeUni__FPv((*(void**)&AsMWStringStorage(self)->data));
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma opt_loop_invariants off
// Apply model metadata in named-bone order. self/context/puppet and every
// named-bone metadata pointer are required; array entries are 8-byte pairs:
// +0 live bone, +4 metadata (bounds +0/+C, local point +18, command text ptr +28,
// inline classification text +2C). A null live bone is tolerated only where
// explicitly tested (visibility/effect); sound needs its +78 world transform.
// Tag grammar, verified against courseModelResourceStrings (0x8046FE9C):
// - exact classification "night_only"/"day_only": hide recursively in opposite
//   daytime mode; exclusive with center/NPC/sound/abv classification handling.
// - exact "center": scan command text for "*wind" + %f and "*nocol" + %d;
//   write context.windScale/collisionDisabled. "*nofog" clears puppet render
//   state; "*blend" is searched but its result is intentionally ignored.
// - exact "npc": bypass sound/abv branch; no NPC allocation occurs here.
// - classification containing "sound": command "%s %f %f" writes name,
//   parameter[2], parameter[1] (defaults 60,-1; parameter[0]=0). Physical units
//   remain unproven. The transform encodes bone bounds/placement in world space.
// - otherwise lowercase classification IN PLACE and hide selection on "abv"
//   substring. This mutation is observable by later consumers.
// - independently, nonnull command text containing "*fx" is scanned by
//   "%s %s %s" into command scratch, resource name, unused token. Only .spr/.seq
//   extensions (case-sensitive, suffix length>=4 including dot) and live bones
//   without flag 0x40000 dispatch CFx::CreateAttachedEffect.
// sscanf results are unchecked and %s has no width: valid trusted text must fit
// destinations (effect tokens 32 bytes, command scratch 8 bytes). Missing/invalid
// tokens are not a supported recovery path; no status is returned. Sound names
// are copied into a generated owned node and committed to self's list; a guard
// frees an uncommitted node if payload construction throws. Earlier nodes/state
// survive later failures. Attached effects are managed by g_fx, not this list;
// the descriptor is stack-only, but its matrix/position pointers borrow bones,
// so effect teardown must precede releasing the puppet. No resource preload or
// successful creation is promised by the ignored call result.
void CourseData_ApplyModelNodeTags(void* self, void* context) {
    int offset;
    char* str;
    char* item;
    int count;
    GeneratedSoundPayload payload;
    char defaultAttachment[0x28];
    char effectAttachment[0x28];
    char effectResourceName[0x20];
    char unusedEffectToken[0x20];
    float half[3];
    float t1[3];
    float t2[3];
    int value[2];
    count = 0;
    offset = 0;
    char* cont;
    while ((unsigned int)count < ((WPuppet*)(cont = (char*)((CourseModelContext*)context)->puppet))->namedBoneCount) {
        // Decode each bone's primary tag. Day/night visibility and center options
        // update existing model state; NPC tags bypass sound/above handling.
        item = (char*)((WPuppet*)cont)->namedBones + offset;
        if (item) {
            str = courseModelResourceStrings;
            char* name = *(char**)((char*)*(void**)(item + 4) + 0x28);
            if (String_CompareCString((char*)*(void**)(item + 4) + 0x2C, str + kNightOnlyTag) == 0) {
                void* manager = g_sharedDoc;
                manager = (void*)((CSharedDoc*)manager)->golfDoc;
                if (((CGolfDoc*)manager)->isDaytime) {
                    if (*(void**)item)
                        WPuppet_SetBoneVisibilityRecursive(((CourseModelContext*)context)->puppet, 1, 1,
                                                           (char*)*(void**)item + 8);
                }
            } else if (String_CompareCString((char*)*(void**)(item + 4) + 0x2C, str + kDayOnlyTag) == 0) {
                void* manager = g_sharedDoc;
                manager = (void*)((CSharedDoc*)manager)->golfDoc;
                if (!((CGolfDoc*)manager)->isDaytime) {
                    if (*(void**)item)
                        WPuppet_SetBoneVisibilityRecursive(((CourseModelContext*)context)->puppet, 1, 1,
                                                           (char*)*(void**)item + 8);
                }
            } else if (String_CompareCString((char*)*(void**)(item + 4) + 0x2C, str + kCenterTag) == 0) {
                void* localName = name;
                String_ScanTokensAfterSubstring(&localName, str + kWindParameter, str + kFloatFormat,
                                                &((CourseModelContext*)context)->windScale);
                String_ScanTokensAfterSubstring(&localName, str + kNoCollisionTag, str + kIntegerFormat,
                                                &((CourseModelContext*)context)->collisionDisabled);
                if (name != 0) {
                    if (strstr(name, str + kNoFogTag) != 0)
                        WPuppet_ClearRenderStateValue(((CourseModelContext*)context)->puppet);
                    strstr(name, courseModelResourceStrings + kBlendTag);
                }
            } else if (String_CompareCString((char*)*(void**)(item + 4) + 0x2C, str + kNpcTag) != 0) {
                char* tag = (char*)*(void**)(item + 4) + 0x2C;
                if (strstr(tag, str + kSoundTag) != 0) {
                    // Sound nodes append owned records after computing a transform
                    // from the bone bounds/placement. Guard ownership ends at linking.
                    payload.parameters[1] = kFloat_neg1_CourseModel;
                    payload.parameters[2] = kCourseSoundDefaultParameter60;
                    payload.parameters[0] = kFloat_0_PetBody;
                    sscanf(name, str + kSoundParameters, payload.soundName, &payload.parameters[2],
                           &payload.parameters[1]);
                    // Local bounding half-extents scale the affine axes; the
                    // local reference point is transformed to world translation.
                    Matrix_MoveLastVectorToFront(payload.transform, (char*)*(void**)item + 0x78);
                    __mi__FRC7WVectorRC7WVector(t1, (char*)*(void**)(item + 4) + 0xC, *(void**)(item + 4));
                    __ml__FfRC7WVector(half, t1, kFloat_0_5_CourseSoundPosition);
                    AffineVectors_ScaleTrailingThree(payload.transform, half);
                    Matrix_TransformPoint(t2, (char*)*(void**)(item + 4) + 0x18, (char*)*(void**)item + 0x78);
                    __as__7WVectorFRC7WVector(payload.transform, t2);
                    void** list = (void**)(str = (char*)&AsCourseData801C(self)->generatedHead);
                    GenGuard guard0;
                    volatile bool flag __attribute__((aligned(4))) = false;
                    char* node = (char*)appMallocDummy__FUl(0x64);
                    char* volatile allocA;
                    char* volatile allocB;
                    allocB = node;
                    allocA = node;
                    guard0.head = (char*)&AsCourseData801C(self)->generatedHead;
                    guard0.n = node;
                    char* dst = node + 8;
                    new (dst) GenValue(payload);
                    char* n2 = (char*)guard0.n;
                    guard0.n = 0;
                    // Commit ownership: +0/+4 are intrusive links, +8 is the
                    // copied payload. list is the embedded sentinel link address;
                    // generatedCount changes only after the node is linked.
                    void* head = *list;
                    *(void**)((char*)head + 4) = n2;
                    *(void**)n2 = *list;
                    *list = n2;
                    *(void**)(n2 + 4) = list;
                    AsCourseData801C(self)->generatedCount = AsCourseData801C(self)->generatedCount + 1;
                } else {
                    str += kAboveTag;
                    if (strstr(MSLString_LowercaseInPlace(tag), str) != 0) {
                        WPuppet_HideBoneSelection(((CourseModelContext*)context)->puppet,
                                                  (char*)*(void**)(item + 4) + 0x2C);
                    }
                }
            }
            if (name != 0) {
                str = courseModelResourceStrings;
                name = strstr(name, str + kEffectTag);
                if (name != 0) {
                    // Phase 2 for this bone: attachment tags are independent of
                    // day/night/center/sound handling above. The parsed command
                    // words and third string token are not otherwise used here.
                    // The effect system receives a temporary descriptor and
                    // borrows the bone world matrix/local position it points to.
                    FxAttachment_Init(defaultAttachment);
                    ((int*)effectResourceName)[0] = 0;
                    ((int*)effectResourceName)[1] = 0;
                    ((int*)effectResourceName)[2] = 0;
                    ((int*)effectResourceName)[3] = 0;
                    ((int*)effectResourceName)[4] = 0;
                    ((int*)effectResourceName)[5] = 0;
                    ((int*)effectResourceName)[6] = 0;
                    ((int*)effectResourceName)[7] = 0;
                    ((int*)unusedEffectToken)[0] = 0;
                    ((int*)unusedEffectToken)[1] = 0;
                    ((int*)unusedEffectToken)[2] = 0;
                    ((int*)unusedEffectToken)[3] = 0;
                    ((int*)unusedEffectToken)[4] = 0;
                    ((int*)unusedEffectToken)[5] = 0;
                    ((int*)unusedEffectToken)[6] = 0;
                    ((int*)unusedEffectToken)[7] = 0;
                    sscanf(name, str + kEffectParameters, value, effectResourceName, unusedEffectToken);
                    void* resource = *(void**)item;
                    if (resource != 0 && (((CourseBoneFlagsView*)resource)->flags & kSkipAttachedEffect) == 0) {
                        name = strrchr(effectResourceName, '.');
                        if (name != 0 && strlen(name) >= 4) {
                            if (String_CompareCString(name + 1, str + kSprayExtension) == 0 ||
                                String_CompareCString(name + 1, str + kSequenceExtension) == 0) {
                                FxAttachment_Init(effectAttachment);
                                ((CourseFxAttachmentView*)effectAttachment)->worldTransform =
                                    (char*)*(void**)item + 0x78;
                                ((CourseFxAttachmentView*)effectAttachment)->localPosition =
                                    (char*)*(void**)(item + 4) + 0x18;
                                String_CompareCString(name + 1, courseModelResourceStrings + kSprayExtension);
                                int zero = 0;
                                ((CFx*)g_fx)
                                    ->CreateAttachedEffect((Vec2_8010*)(effectAttachment), effectResourceName, zero, 0,
                                                           zero);
                            }
                        }
                    }
                }
            }
        }
        offset += 8;
        ++count;
    }
}
#pragma pop

#pragma push
void CourseData_BuildQuadTree(void* self) {
    void* list = CQuadTree_GetInstance();
    (*(void**)&AsCQuadTree(list)->courseData) = self;
    list = CQuadTree_GetInstance();
    CourseTriangleGrid_Rebuild(list, AsCourseData801C(self)->fallbackSurface);
    for (int i = 0; (short)i < AsCourseData801C(self)->surfaceCount; ++i) {
        list = CQuadTree_GetInstance();
        ((CQuadTree*)list)->CreateSurfaceObject((short)i);
    }
}
#pragma pop

#pragma push
void* CourseData_GetEntryByIndex(void* self, int index) {
    return EntryList_GetEntryOrFallback(((CourseEntryContainer*)AsCourseData801C(self)->entryContainer)->entries,
                                        index);
}
#pragma pop

#pragma push
void* CourseData_FindEntryByName(void* self, const char* name) {
    return NamedObjectTable_FindOrFallback(((CourseEntryContainer*)AsCourseData801C(self)->entryContainer)->entries,
                                           name);
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
void CourseData_ReleaseResources(void* self) {
    // Resource records own handles; their array allocation begins 0x10 bytes
    // before the first record. Drop handles before freeing that array.
    {
        if (AsCourseData801C(self)->resources) {
            int i = 0;
            int offset = 0;

            while (i < AsCourseData801C(self)->resourceCount) {
                if (g_resourceManager) {
                    void* resource =
                        AsCourseResource801C((char*)AsCourseData801C(self)->resources + offset)->resourceId;
                    if (resource) {
                        ResourceManager_ReleaseId(g_resourceManager, resource);
                        AsCourseResource801C((char*)AsCourseData801C(self)->resources + offset)->resourceId =
                            (void*)(0);
                    }
                }
                offset += 0x28;
                ++i;
            }
            if (AsCourseData801C(self)->resources) {
                if (AsCourseData801C(self)->resources) {
                    __dla__FPv((char*)AsCourseData801C(self)->resources - 0x10);
                }
            }
            AsCourseData801C(self)->resources = 0;
        }
    }
    AsCourseData801C(self)->resourceCount = 0;
    // Release model instances and clear logical size, leaving resourceObjects allocated.
    {
        unsigned int i2 = 0;
        unsigned int offset2 = 0;
        while (i2 < AsCourseData801C(self)->resourceObjectCount) {
            if (g_resourceManager) {
                void* resource = *(void**)((char*)AsCourseData801C(self)->resourceObjects + offset2);
                if (resource) {
                    ResourceManager_ReleaseObject(g_resourceManager, resource, 0);
                    *(void**)((char*)AsCourseData801C(self)->resourceObjects + offset2) = 0;
                }
            }
            i2 = 1 + i2;
            offset2 += 4;
        }
        {

            AsCourseData801C(self)->resourceObjectCount = (AsCourseData801C(self)->resourceObjectCount) - i2;
        }
    }
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
int Polygon_ContainsPointXZ(void* self, void* polygon, void* pointPosition) {
    float* point = (float*)pointPosition;
    int vertexOffset;
    int vertexCount;
    int windingQuarterTurns;
    int vertexIndex;
    vertexCount = AsPolygon801C(polygon)->vertexCount;
    if (vertexCount < 3)
        return 0;
    windingQuarterTurns = 0;
    vertexIndex = 0;
    vertexOffset = 0;
    for (; vertexIndex < vertexCount; ++vertexIndex, vertexOffset += 12) {
        float edgeStart[3];
        float edgeEnd[3];
        *(WVector*)edgeStart = *(WVector*)((char*)AsPolygon801C(polygon)->vertices + vertexOffset);
        int nextVertexIndex = (vertexIndex + 1) % vertexCount;
        *(WVector*)edgeEnd = *(WVector*)((char*)AsPolygon801C(polygon)->vertices + nextVertexIndex * 12);
        int startQuadrant =
            (edgeStart[0] > point[0]) ? ((edgeStart[2] > point[2]) ? 1 : 4) : ((edgeStart[2] > point[2]) ? 2 : 3);
        int endQuadrant =
            (edgeEnd[0] > point[0]) ? ((edgeEnd[2] > point[2]) ? 1 : 4) : ((edgeEnd[2] > point[2]) ? 2 : 3);
        // Wrap a three-quarter jump; resolve a half-turn with its X crossing.
        int quadrantChange = endQuadrant - startQuadrant;
        switch (quadrantChange) {
        case 2:
        case -2: {
            float crossingX =
                edgeEnd[0] - ((edgeEnd[2] - point[2]) * (edgeStart[0] - edgeEnd[0])) / (edgeStart[2] - edgeEnd[2]);
            if (crossingX > point[0])
                quadrantChange = -quadrantChange;
            break;
        }
        case 3:
            quadrantChange = -1;
            break;
        case -3:
            quadrantChange = 1;
            break;
        }
        windingQuarterTurns += quadrantChange;
    }
    int sign = windingQuarterTurns >> 31;
    return ((sign ^ windingQuarterTurns) - sign) == 4;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
int CourseData_IsPointOutsidePlayableRegion(void* self, void* arg) {
    char* entry = (char*)AsCourseData801C(self)->regions;
    while (entry != (char*)AsCourseData801C(self)->regions + AsCourseData801C(self)->regionCount * kRegionRecordBytes) {
        int type = AsCourseRegion801C(entry)->type;
        if (type == kAdditionalBoundaryRegion || type == kPrimaryBoundaryRegion) {
            if (!Polygon_ContainsPointXZ(self, (char*)&AsCourseRegion801C(entry)->polygon, arg))
                return 1;
        } else if (type == kExclusionRegion) {
            if (Polygon_ContainsPointXZ(self, (char*)&AsCourseRegion801C(entry)->polygon, arg))
                return 1;
        }
        entry += kRegionRecordBytes;
    }
    void* list = CQuadTree_GetInstance();
    return ((CQuadTree*)list)->GetGroundFlagBit1AtPoint(arg);
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
void CourseData_ClampPointToActiveRegion(void* out, void* self, int* xyz, void* arg) {
    int first = xyz[0];
    int second = xyz[1];
    int third = xyz[2];
    *(int*)out = first;
    *((int*)out + 1) = second;
    *((int*)out + 2) = third;
    char* entry = (char*)AsCourseData801C(self)->regions;
    char* end = entry + AsCourseData801C(self)->regionCount * kRegionRecordBytes;
    for (; entry != end; entry += kRegionRecordBytes) {
        int type = AsCourseRegion801C(entry)->type;
        if ((AsCourseData801C(self)->regionMode && type == kAlternateBoundaryRegion) ||
            (!AsCourseData801C(self)->regionMode && type == kPrimaryBoundaryRegion)) {
            char* params = (char*)&AsCourseRegion801C(entry)->polygon;
            if (!Polygon_ContainsPointXZ(self, params, xyz)) {
                WVector local;
                Polygon_ClampPointAlongRayXZ(&local, self, params, xyz, arg);
                *(WVector*)out = local;
            }
            break;
        }
    }
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
void Polygon_ClampPointAlongRayXZ(void* output, void* self, void* polygon, void* pointPosition, void* rayOrigin) {
    // output/pointPosition/rayOrigin require three floats; polygon requires at
    // least two readable vertices even when vertexCount<=0 (orientation loads
    // precede loops). self is unused. output must not alias rayOrigin; in-place
    // pointPosition is supported because the requested ray is saved first.
    // No-hit (including collinear straddles): retain horizontal distance, then
    // normalize/rescale/add origin anyway. Thus output is reconstructed XYZ at
    // rayOrigin.Y, not an untouched copy; rounding can differ from input.
    // Strict opposite cross signs exclude an edge aligned with the ray. Zero
    // ray/zero vertex rays, coincident first rays, and zero sine denominator
    // are not rejected. They follow Normalize/division's numeric behavior;
    // do not add fallback/epsilon or promise finite output for degeneracies.
    // Angles passed to acos/Math_SinApprox are radians, distances world units.
    int vertexCount = AsPolygon801C(polygon)->vertexCount;
    float* clampedPosition = (float*)output;
    ((int*)output)[0] = ((int*)pointPosition)[0];
    ((int*)output)[1] = ((int*)pointPosition)[1];
    ((int*)output)[2] = ((int*)pointPosition)[2];

    // Geometry: all work is in the XZ plane relative to rayOrigin. The point's ray R and each
    // polygon edge (start ray S, end ray E) are compared: an edge is "hit" when S and E both
    // lie in front of R (dot >= 0) and R falls between them (sign of Y of cross(R,S) and
    // cross(R,E) differ in the way matching the polygon winding). With
    //   a = angle(S,R), b = angle(R,E), |S| = ds, |E| = de
    // the distance from the origin to the edge along R follows from the sine rule:
    //   boundary = de*ds*sin(a+b) / (de*sin(b) + ds*sin(a))
    // and the result distance is min(|R|, boundary). The two loops below are the same
    // computation for the two winding signs (cross Y of the first two vertices > 0, else the
    // other); they differ in the cross-sign test direction. NOTE: only the first loop zeroes
    // the start ray's Y before use, as the original code does.
    // Work in rays relative to rayOrigin, discarding the requested height.
    WVector result = *(WVector*)pointPosition - *(WVector*)rayOrigin;
    *(WVector*)clampedPosition = result;
    clampedPosition[1] = kFloat_0_PetBody;
    float clampedDistance = (*(WVector*)clampedPosition).Magnitude();

    char* vertices = (char*)AsPolygon801C(polygon)->vertices;
    float orientationCross[3];
    WVector secondVertexRay = *(WVector*)(vertices + 12) - *(WVector*)rayOrigin;
    WVector firstVertexRay = *(WVector*)vertices - *(WVector*)rayOrigin;
    Vector_CrossProduct(orientationCross, &firstVertexRay, &secondVertexRay);

    float zero = kFloat_0_PetBody;
    // The first two rays choose the winding branch. Both branches find the
    // first forward edge straddling the requested ray, with opposite sign tests.
    if (orientationCross[1] > zero) {
        for (int i = 0; i < vertexCount; ++i) {
            float av[3], edgeStartRay[3];
            __mi__FRC7WVectorRC7WVector(av, vertices + i * 12, rayOrigin);
            *(WVector*)edgeStartRay = *(WVector*)av;
            edgeStartRay[1] = zero;
            if ((*(WVector*)edgeStartRay * *(WVector*)clampedPosition) < zero)
                continue;

            float t1[3], startCross[3];
            Vector_CrossProduct(t1, clampedPosition, edgeStartRay);
            *(WVector*)startCross = *(WVector*)t1;

            int nextIdx = (i + 1) % vertexCount;
            float next[3], edgeEndRay[3];
            __mi__FRC7WVectorRC7WVector(next, vertices + nextIdx * 12, rayOrigin);
            *(WVector*)edgeEndRay = *(WVector*)next;
            edgeEndRay[1] = zero;
            if ((*(WVector*)edgeEndRay * *(WVector*)clampedPosition) < zero)
                continue;

            float t2[3], endCross[3];
            Vector_CrossProduct(t2, clampedPosition, edgeEndRay);
            *(WVector*)endCross = *(WVector*)t2;

            if (startCross[1] < zero && endCross[1] > zero) {
                float pointDirection[3], endDirection[3], startDirection[3];
                pointDirection[0] = clampedPosition[0];
                pointDirection[1] = zero;
                pointDirection[2] = clampedPosition[2];
                endDirection[0] = edgeEndRay[0];
                endDirection[1] = zero;
                endDirection[2] = edgeEndRay[2];
                startDirection[0] = edgeStartRay[0];
                startDirection[1] = zero;
                startDirection[2] = edgeStartRay[2];

                float startDistance = (*(WVector*)startDirection).Magnitude();
                float endDistance = (*(WVector*)endDirection).Magnitude();
                (*(WVector*)endDirection).Normalize();
                (*(WVector*)pointDirection).Normalize();
                (*(WVector*)startDirection).Normalize();

                float cosA = (*(WVector*)startDirection * *(WVector*)pointDirection);
                float clamped;
                if (cosA < kFloat_neg1_CourseModel)
                    clamped = kFloat_neg1_CourseModel;
                else if (cosA > kFloat_1_polygon)
                    clamped = kFloat_1_polygon;
                else
                    clamped = cosA;
                float angles[2];
                angles[0] = (float)acos(clamped);

                float cosB = (*(WVector*)endDirection * *(WVector*)pointDirection);
                if (cosB < kFloat_neg1_CourseModel)
                    clamped = kFloat_neg1_CourseModel;
                else if (cosB > kFloat_1_polygon)
                    clamped = kFloat_1_polygon;
                else
                    clamped = cosB;
                angles[1] = (float)acos(clamped);

                // Ray/edge intersection distance by the sine rule; shorten only.
                float endSineProduct = endDistance * Math_SinApprox(angles[1]);
                float sineRuleDenominator = startDistance * Math_SinApprox(angles[0]) + endSineProduct;
                float boundaryDistance =
                    (endDistance * startDistance * Math_SinApprox(angles[0] + angles[1])) / sineRuleDenominator;
                clampedDistance = (clampedDistance > boundaryDistance) ? boundaryDistance : clampedDistance;
                break;
            }
        }
    } else {
        for (int i = 0; i < vertexCount; ++i) {
            float av[3], edgeStartRay[3];
            __mi__FRC7WVectorRC7WVector(av, vertices + i * 12, rayOrigin);
            *(WVector*)edgeStartRay = *(WVector*)av;
            if ((*(WVector*)edgeStartRay * *(WVector*)clampedPosition) < zero)
                continue;

            float t1[3], startCross[3];
            Vector_CrossProduct(t1, clampedPosition, edgeStartRay);
            *(WVector*)startCross = *(WVector*)t1;

            int nextIdx = (i + 1) % vertexCount;
            float next[3], edgeEndRay[3];
            __mi__FRC7WVectorRC7WVector(next, vertices + nextIdx * 12, rayOrigin);
            *(WVector*)edgeEndRay = *(WVector*)next;
            edgeEndRay[1] = zero;
            if ((*(WVector*)edgeEndRay * *(WVector*)clampedPosition) < zero)
                continue;

            float t2[3], endCross[3];
            Vector_CrossProduct(t2, clampedPosition, edgeEndRay);
            *(WVector*)endCross = *(WVector*)t2;

            if (startCross[1] > zero && endCross[1] < zero) {
                float pointDirection[3], endDirection[3], startDirection[3];
                pointDirection[0] = clampedPosition[0];
                pointDirection[1] = zero;
                pointDirection[2] = clampedPosition[2];
                endDirection[0] = edgeEndRay[0];
                endDirection[1] = zero;
                endDirection[2] = edgeEndRay[2];
                startDirection[0] = edgeStartRay[0];
                startDirection[1] = zero;
                startDirection[2] = edgeStartRay[2];

                float startDistance = (*(WVector*)startDirection).Magnitude();
                float endDistance = (*(WVector*)endDirection).Magnitude();
                (*(WVector*)endDirection).Normalize();
                (*(WVector*)startDirection).Normalize();
                (*(WVector*)pointDirection).Normalize();

                float cosA = (*(WVector*)startDirection * *(WVector*)pointDirection);
                float clamped;
                if (cosA < kFloat_neg1_CourseModel)
                    clamped = kFloat_neg1_CourseModel;
                else if (cosA > kFloat_1_polygon)
                    clamped = kFloat_1_polygon;
                else
                    clamped = cosA;
                float angles[2];
                angles[0] = (float)acos(clamped);

                float cosB = (*(WVector*)endDirection * *(WVector*)pointDirection);
                if (cosB < kFloat_neg1_CourseModel)
                    clamped = kFloat_neg1_CourseModel;
                else if (cosB > kFloat_1_polygon)
                    clamped = kFloat_1_polygon;
                else
                    clamped = cosB;
                angles[1] = (float)acos(clamped);

                float endSineProduct = endDistance * Math_SinApprox(angles[1]);
                float sineRuleDenominator = startDistance * Math_SinApprox(angles[0]) + endSineProduct;
                float boundaryDistance =
                    (endDistance * startDistance * Math_SinApprox(angles[0] + angles[1])) / sineRuleDenominator;
                clampedDistance = (clampedDistance > boundaryDistance) ? boundaryDistance : clampedDistance;
                break;
            }
        }
    }

    // Restore world position from the horizontal ray and its shortened distance.
    (*(WVector*)clampedPosition).Normalize();

    WVector scaled = clampedDistance * *(WVector*)clampedPosition;

    WVector finalValue = scaled + *(WVector*)rayOrigin;
    *(WVector*)output = finalValue;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
void* CourseData_FindNameRecord(void* self, const char* name) {
    if (name == 0 || *name == 0)
        return 0;
    int offset;
    int i;
    i = 0;
    offset = 0;
    for (; i < AsCourseData801C(self)->nameCount; ++i) {
        if (CString_Compare((char*)AsCourseData801C(self)->names + offset, name) == 0)
            return (char*)AsCourseData801C(self)->names + i * kNameRecordBytes;
        offset += kNameRecordBytes;
    }
    return 0;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
void* CourseData_FindNamedPoint(void* self, const char* name) {
    if (name == 0 || *name == 0)
        return 0;
    int offset;
    int i;
    i = 0;
    offset = 0;
    for (; i < AsCourseData801C(self)->namedPointCount; ++i) {
        if (CString_Compare((char*)AsCourseData801C(self)->namedPoints + offset + 1, name) == 0)
            return (char*)AsCourseData801C(self)->namedPoints + i * kNamedPointRecordBytes;
        offset += kNamedPointRecordBytes;
    }
    return 0;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma use_lmw_stmw on
void* CourseData_FindBoundsByNameSubstring(void* self, const char* name) {
    if (name == 0 || (signed char)*name == 0)
        return 0;
    int offset;
    int count = AsCourseData801C(self)->boundsCount;
    char* entries;
    int i = 0;
    offset = 0;
    for (; i < count; ++i) {
        entries = (char*)AsCourseData801C(self)->bounds;
        if (strstr(entries + offset + 1, name) != 0)
            return entries + i * kBoundsRecordBytes;
        offset += kBoundsRecordBytes;
    }
    return 0;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
void* CourseData_FindSurfaceByName(void* self, const char* name) {
    if (name == 0 || *name == 0)
        return AsCourseData801C(self)->fallbackSurface;
    int offset;
    int i;
    i = 0;
    offset = 0;
    for (; i < AsCourseData801C(self)->surfaceCount; ++i) {
        if (CString_Compare(&AsCourseSurface801C((char*)AsCourseData801C(self)->surfaces + offset)->name, name) == 0)
            return (char*)AsCourseData801C(self)->surfaces + i * kSurfaceRecordBytes;
        offset += kSurfaceRecordBytes;
    }
    return 0;
}
#pragma pop

#pragma push
void* NameTable_FindByName(void* self, const char* name) {
    if (name == 0 || *name == 0)
        return 0;
    char* entry = (char*)AsCourseData801C(self)->regions;
    while (entry != (char*)AsCourseData801C(self)->regions + AsCourseData801C(self)->regionCount * kRegionRecordBytes) {
        char local[16];
        MWString_CopyConstructForNameLookup(local, entry);
        unsigned int flags = *(unsigned int*)local;
        char* str;
        if (flags >> 31 == 0)
            str = local + 1;
        else
            str = AsMWStringStorage(local)->data;
        int cmp = CString_Compare(str, name);
        unsigned int flags2 = *(unsigned int*)local;
        int found = (cmp == 0);
        if (flags2 >> 31)
            appFreeUni__FPv((void*)AsMWStringStorage(local)->data);
        if (found)
            return entry;
        entry += kRegionRecordBytes;
    }
    return 0;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
void* CourseData_FindBoundsContainingPoint(void* self, float* point) {
    int count;
    int offset;
    char* entries;
    int found;
    count = AsCourseData801C(self)->boundsCount;
    int i = 0;
    offset = 0;
    for (; i < count; ++i) {
        entries = (char*)AsCourseData801C(self)->bounds;
        found = 0;
        if (AsCourseBounds801C(entries + offset)->minX <= point[0] &&
            point[0] <= AsCourseBounds801C(entries + offset)->maxX &&
            AsCourseBounds801C(entries + offset)->minY <= point[1] &&
            point[1] <= AsCourseBounds801C(entries + offset)->maxY &&
            AsCourseBounds801C(entries + offset)->minZ <= point[2] &&
            point[2] <= AsCourseBounds801C(entries + offset)->maxZ)
            found = 1;
        if (found != 0)
            return entries + i * kBoundsRecordBytes;
        offset += kBoundsRecordBytes;
    }
    return 0;
}
#pragma pop

#pragma push
void* CourseData_FindNamedRecord(void* self, const char* name) {
    int offset = 0;
    for (int i = 0; i < AsCourseData801C(self)->nameCount; ++i) {
        char* entry = (char*)AsCourseData801C(self)->names + offset;
        if (String_CompareCString(entry, name) == 0)
            return (char*)AsCourseData801C(self)->names + i * kNameRecordBytes;
        offset += kNameRecordBytes;
    }
    return 0;
}
#pragma pop

#pragma push
unsigned char CourseSurfaceProperties_GetParticleOpacity(void* self, int index) {
    if (index == kFallbackSurfaceIndex)
        return kOpaqueParticle;
    return AsCourseSurface801C((char*)AsCourseData801C(self)->surfaces + index * kSurfaceRecordBytes)->particleOpacity;
}
#pragma pop

#pragma push
float CourseSurfaceProperties_GetParticleWindScale(void* self, int index) {
    if (index == kFallbackSurfaceIndex)
        return AsCourseSurface801C(AsCourseData801C(self)->fallbackSurface)->particleWindScale;
    return AsCourseSurface801C((char*)AsCourseData801C(self)->surfaces + index * kSurfaceRecordBytes)
        ->particleWindScale;
}
#pragma pop

#pragma push
#pragma opt_lifetimes off
void CourseData_UpdateWindActors(void* self) {
    // Broadcast the global wind first; actor-specific queries below use world positions.
    CWind* windInstance = CWind::GetInstance();
    int dest[3];
    int local[3];
    GetWindVector__5CWindFv(local, windInstance);
    dest[0] = local[0];
    dest[1] = local[1];
    dest[2] = local[2];
    ((CFx*)g_fx)->SetWindVector(dest);
    char* rowCursor = (char*)self;
    void* actor;
    int i = 0;
    // Slot pairs are at self+0xD8+i*0x48. rowCursor retains the enclosing-base
    // address; the slots view below applies the offset exactly once.
    for (; i < AsCourseData801C(self)->windActorCount; rowCursor += kWindActorStride, ++i) {
        actor = ((CourseWindActorSlots*)(rowCursor + kWindActorSlotsOffset))->spray;
        if (actor) {
            CWind* v = CWind::GetInstance();
            char result[12];
            GetWindVectorAtPoint__5CWindFRC7WVectorf(result, v, &((CourseSprayPositionView*)actor)->worldPosition,
                                                     kFloat_99999_CourseEffectWindQuery);
            ((CFxSpray*)(((CourseWindActorSlots*)(rowCursor + kWindActorSlotsOffset))->spray))->SetWindVector(result);
        }
        actor = ((CourseWindActorSlots*)(rowCursor + kWindActorSlotsOffset))->sequence;
        if (actor) {
            CWind* v = CWind::GetInstance();
            char result[12];
            GetWindVectorAtPoint__5CWindFRC7WVectorf(result, v, &((CFxSequence*)actor)->worldPosition,
                                                     kFloat_99999_CourseEffectWindQuery);
            ((CFxSequence*)(((CourseWindActorSlots*)(rowCursor + kWindActorSlotsOffset))->sequence))
                ->SetWindVector(result);
        }
    }
}
#pragma pop
}
