#include <decomp/offset_of.h>
typedef unsigned long new_size_t;
// Course scene loader: geometry, placement, regions, weather, effects and rivals.
// Mod-question index (all enter through CourseModel_LoadFromFile):
// - Geometry variants: CourseGeometryVariant, header baseMeshCount/extraMeshCount0/1
//   in ModelResource801BD2B8.h and CourseInputMeshReadView::variant. Allocation
//   and filtering must agree; skipped records still consume their full file bytes.
// - Placement/scale: CourseInputMeshReadView translation/rotationDegrees/uniformScale
//   -> modelMatrix -> puppet world matrices AND world collision triangles. Root
//   bone compensation is part of both fallback and selected-mesh paths.
// - Weather/model names: SelectCourseMeshSuffix, CourseStrings cloud/rain/night
//   suffixes; consumers are resource paths, not the geometry group selector.
// - Lighting/fog: ReadCourseSceneMarker, ApplyCourseEnvironment and
//   CopyCourseAlternateEnvironmentLight; scene colors also multiply CGolfDoc::bright.
//   Fog stored on self is engine distance*.32, with extended distances another *3.
// - Water/bounds/camera regions: ReadCourseBoundsRecord/ReadCoursePolygonRegions,
//   CourseRegionKind and CourseStrings tags. Later matching tags override earlier.
// - Effects/shadows: SetupCourseEffectsAndShadows, CourseBoundsKind and the effect
//   tag/name formats; attachment transforms live on self, not the loader frame.
// - Party archives/balloons/rivals: LoadCourseSceneArchiveAndBalloons,
//   ReadCourseRivalCandidates, AppendCourseEndpointCandidates. These use separate
//   streams after ReleaseMainCourseInput. WPolySoup_ReleaseResources (course_data_and_polysoup_cleanup.cpp)
//   is the course-owned resource teardown boundary; see failure contract at entry.
// Constant/data companions: courseModelResourceStrings and u801BD08C_data.cpp.
// Packed string-pool view; byte extents come from the globals research.
struct CourseStrings {
    char padding0[90];
    char openErrorFormat[46];
    char openErrorTitle[17];
    char versionError[40];
    char versionErrorTitle[23];
    char lightingFormat[9];
    char soundFormat[9];
    char propertyFileFormat[16];
    char outlineTag[8];
    char inBoundsTag[3];
    char outOfBoundsTag[3];
    char autoTag[5];
    char beachTag[6];
    char riverTag[6];
    char seaTag[4];
    char waveTag[5];
    char npcTag[4];
    char cameraTag[7];
    char lavaTag[5];
    char centerBone[7];
    char windTag[6];
    char floatFormat[3];
    char soundTag[6];
    char cloudSuffix[7];
    char rainSuffix[6];
    char empty[1];
    char nightSuffix[3];
    char icePeople1[14];
    char icePeople2[14];
    char icePeople3[14];
    char iceUndersea[14];
    char petExtension[5];
    char effectTag[4];
    char effectFormat[12];
    char excludedSequence[17];
    char sprayExtension[4];
    char sequenceExtension[4];
    char backgroundShadow[3];
    char courseFileFormat[13];
    char rankMarker[5];
    char environmentExtension[5];
    char geometryExtension[6];
    char rivalExtension[6];
    char holeNameFormat[15];
    char startName[4];
    char compressedSuffix[9];
    char decompressedSuffix[4];
    char balloonExtension[6];
};
enum {
    kCourseStringsOpenErrorFormatOffset = (int)DECOMP_OFFSETOF(CourseStrings, openErrorFormat),
    kCourseStringsOpenErrorTitleOffset = (int)DECOMP_OFFSETOF(CourseStrings, openErrorTitle),
    kCourseStringsVersionErrorOffset = (int)DECOMP_OFFSETOF(CourseStrings, versionError),
    kCourseStringsVersionErrorTitleOffset = (int)DECOMP_OFFSETOF(CourseStrings, versionErrorTitle),
    kCourseStringsLightingFormatOffset = (int)DECOMP_OFFSETOF(CourseStrings, lightingFormat),
    kCourseStringsSoundFormatOffset = (int)DECOMP_OFFSETOF(CourseStrings, soundFormat),
    kCourseStringsPropertyFileFormatOffset = (int)DECOMP_OFFSETOF(CourseStrings, propertyFileFormat),
    kCourseStringsOutlineTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, outlineTag),
    kCourseStringsInBoundsTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, inBoundsTag),
    kCourseStringsOutOfBoundsTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, outOfBoundsTag),
    kCourseStringsAutoTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, autoTag),
    kCourseStringsBeachTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, beachTag),
    kCourseStringsRiverTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, riverTag),
    kCourseStringsSeaTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, seaTag),
    kCourseStringsWaveTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, waveTag),
    kCourseStringsNpcTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, npcTag),
    kCourseStringsCameraTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, cameraTag),
    kCourseStringsLavaTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, lavaTag),
    kCourseStringsCenterBoneOffset = (int)DECOMP_OFFSETOF(CourseStrings, centerBone),
    kCourseStringsWindTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, windTag),
    kCourseStringsFloatFormatOffset = (int)DECOMP_OFFSETOF(CourseStrings, floatFormat),
    kCourseStringsSoundTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, soundTag),
    kCourseStringsCloudSuffixOffset = (int)DECOMP_OFFSETOF(CourseStrings, cloudSuffix),
    kCourseStringsRainSuffixOffset = (int)DECOMP_OFFSETOF(CourseStrings, rainSuffix),
    kCourseStringsEmptyOffset = (int)DECOMP_OFFSETOF(CourseStrings, empty),
    kCourseStringsNightSuffixOffset = (int)DECOMP_OFFSETOF(CourseStrings, nightSuffix),
    kCourseStringsIcePeople1Offset = (int)DECOMP_OFFSETOF(CourseStrings, icePeople1),
    kCourseStringsIcePeople2Offset = (int)DECOMP_OFFSETOF(CourseStrings, icePeople2),
    kCourseStringsIcePeople3Offset = (int)DECOMP_OFFSETOF(CourseStrings, icePeople3),
    kCourseStringsIceUnderseaOffset = (int)DECOMP_OFFSETOF(CourseStrings, iceUndersea),
    kCourseStringsPetExtensionOffset = (int)DECOMP_OFFSETOF(CourseStrings, petExtension),
    kCourseStringsEffectTagOffset = (int)DECOMP_OFFSETOF(CourseStrings, effectTag),
    kCourseStringsEffectFormatOffset = (int)DECOMP_OFFSETOF(CourseStrings, effectFormat),
    kCourseStringsExcludedSequenceOffset = (int)DECOMP_OFFSETOF(CourseStrings, excludedSequence),
    kCourseStringsSprayExtensionOffset = (int)DECOMP_OFFSETOF(CourseStrings, sprayExtension),
    kCourseStringsSequenceExtensionOffset = (int)DECOMP_OFFSETOF(CourseStrings, sequenceExtension),
    kCourseStringsBackgroundShadowOffset = (int)DECOMP_OFFSETOF(CourseStrings, backgroundShadow),
    kCourseStringsCourseFileFormatOffset = (int)DECOMP_OFFSETOF(CourseStrings, courseFileFormat),
    kCourseStringsRankMarkerOffset = (int)DECOMP_OFFSETOF(CourseStrings, rankMarker),
    kCourseStringsEnvironmentExtensionOffset = (int)DECOMP_OFFSETOF(CourseStrings, environmentExtension),
    kCourseStringsGeometryExtensionOffset = (int)DECOMP_OFFSETOF(CourseStrings, geometryExtension),
    kCourseStringsRivalExtensionOffset = (int)DECOMP_OFFSETOF(CourseStrings, rivalExtension),
    kCourseStringsHoleNameFormatOffset = (int)DECOMP_OFFSETOF(CourseStrings, holeNameFormat),
    kCourseStringsStartNameOffset = (int)DECOMP_OFFSETOF(CourseStrings, startName),
    kCourseStringsCompressedSuffixOffset = (int)DECOMP_OFFSETOF(CourseStrings, compressedSuffix),
    kCourseStringsDecompressedSuffixOffset = (int)DECOMP_OFFSETOF(CourseStrings, decompressedSuffix),
    kCourseStringsBalloonExtensionOffset = (int)DECOMP_OFFSETOF(CourseStrings, balloonExtension),
};
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
// CourseModel_LoadFromFile: shared-document mode word at original +0xAD8.
#include "classes/ModelResource801BD2B8.h"
#define AsModelResource801BD2B8(p) ((ModelResource801BD2B8*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/string.h"
#include "game/wvector.h"

#include "game/types.h"
#include "classes/CNodeContainer.h"

#define AsCNodeContainer(p) ((CNodeContainer*)(p))
// Translation unit 0x801BD08C-0x801BF838 (7 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Runtime mesh record built from the course file (0xDC-byte stride).
struct CourseMeshRecord {
    unsigned char collisionKind, flags; char padding02[6];
    float windScale, originDistance; void* puppet; float matrix[12];
    char modelName[32]; float bounds[6], expandedBounds[6];
    float center[3], radius, centerCopy[3];
    void* triangles; volatile int triangleCount;
    unsigned char state, tagPresent; char tag[32]; char tailPadding[2];
};
// File-read view of scratch+0x610, valid only until Matrix_CopyAffine writes
// over +0x668..+0x697. This is a format reference, not a replacement stack object:
// runtime processing reuses those bytes for a matrix after consuming TRS.
struct CourseInputMeshReadView {
    unsigned char surfaceKindAndFlags;
    char unresolved01[7];
    char modelName[32];                 // +08 -> scratch+618
    float bounds[6];                    // +28 -> scratch+638
    char boundsWorkArea[0x18];          // +40 -> scratch+650, recomputed at runtime
    float translation[3];              // +58 -> scratch+668 (later affine matrix)
    float rotationDegrees[3];           // +64 -> scratch+674
    float uniformScale;                // +70 -> scratch+680
    char unresolved74[0x14];
    int variant;                       // +88 -> scratch+698, survives matrix copy
    char tag[32];                      // +8C -> scratch+69C
};
typedef char CourseInputMeshReadView_size[(sizeof(CourseInputMeshReadView) == 0xAC) ? 1 : -1];
// Other transient records, expressed as frame ranges so overlapping views are
// explicit: region [310,330) is the owning CNodeContainer; [330,348) is its
// incoming file header (name plus count at 340). Sound records [558,5B4) and
// [5B4,610) hold parsed name/timing plus transformed bounds before list copying.
// Effect scratch [418,480) describes attachments only during the bounds loop;
// its matrix/vector pointers refer to persistent self storage, never scratch.
// Loader section/lifetime map (not a declaration of fully recovered C++ types):
// 1. Open stream/header -> owned camera, marker and bounds arrays.
// 2. Read camera/lighting/region sections -> runtime transforms and region copies.
// 3. Read fallback mesh and selected mesh records -> puppets, bounds, triangles;
//    classify bone tags, collect model/bitmap resources and attached effects.
// 4. Release main stream and its temporary vectors/strings, keeping runtime data.
// 5. Open independent environment/light/rival streams -> fog/lights/AI candidates.
// 6. Load mode-specific archives/balloons, then destroy remaining path temporaries.
// Scratch live ranges: +48 holder borrows &stream at +4C until main-stream cleanup;
// +198 temporary region name dies before the +310 region is populated/appended;
// +288 collision vector lives from fallback setup through main-stream cleanup;
// +248 bitmap vector and +258/+264 strings live only through mesh/resource setup.
// +610 input record is overwritten per mesh. Its +668 region begins as translation
// but later receives a copied 0x30-byte affine matrix: offset names describe the
// file-read phase, not permanent field types. +480 is the working affine matrix.
// +220/+22C path strings outlive the main stream. +20/+10/+00 are separate holders
// for later streams, each destroyed after its own resource pointer is cleared.
// Never retain pointers into scratch after the corresponding copy/append call.
// Fixed scratch-frame regions preserve the loader's original aliasing and lifetimes.
enum CourseScratchOffsets {
    kScratchInputMesh = 0x610,
    kScratchInputModelName = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, modelName),
    kScratchInputBounds = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, bounds),
    kScratchExpandedBounds = 0x650,
    kScratchInputTranslation = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, translation),
    kScratchInputRotationX = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, rotationDegrees[0]),
    kScratchInputRotationY = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, rotationDegrees[1]),
    kScratchInputRotationZ = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, rotationDegrees[2]),
    kScratchInputScale = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, uniformScale),
    kScratchInputVariant = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, variant),
    kScratchInputTag = kScratchInputMesh + (int)DECOMP_OFFSETOF(CourseInputMeshReadView, tag),
    kScratchModelMatrix = 0x480,
    kScratchRotationRadiansX = 0x27C,
    kScratchRotationRadiansY = 0x280,
    kScratchRotationRadiansZ = 0x284,
    kScratchCollisionData = 0x288,
    kScratchWeatherSuffix = 0x264,
    kScratchModelPath = 0x258,
    kScratchBitmapNames = 0x248,
    kScratchRegion = 0x310,
    kScratchRegionHeader = 0x330,
    kScratchRegionPointCount = 0x340,
    kScratchEnvironmentPath = 0x220,
    kScratchCoursePath = 0x22C,
    kScratchEffectAttachment = 0x418,
    kScratchEffectName = 0x440,
};
// Named views of the fixed loader frame. These are POD storage, not constructed
// strings/vectors: existing init/dtor calls continue to delimit each owner's life.
// inputMesh.translation overlaps an affine matrix after TRS has been consumed.
struct CourseLoadFrameView {
    char staging000[0x48];
    unsigned int mainStreamHolder;
    void* mainStream;
    int resourceCapability;
    char staging054[0x174 - 0x54];
    float fallbackRootBone[3], scaledFallbackRootBone[3];
    unsigned int regionPointBits[3];
    unsigned int regionName[3]; // MSL string; explicitly constructed/destroyed per region.
    char staging1A4[0x220 - 0x1A4];
    unsigned int environmentPath[3];
    unsigned int coursePath[3];
    float effectRotationRadians[3];
    char staging244[4];
    unsigned int bitmapNames[4];
    unsigned int modelPath[3];
    unsigned int weatherSuffix[3];
    float scaledRootBone[3];
    float rotationRadians[3];
    unsigned int collisionNodes[3];
    float serializedRegionPoint[3], runtimeRegionPoint[3];
    char staging2AC[0x310 - 0x2AC];
    unsigned int regionObject[8]; // Explicit CNodeContainer lifetime, not automatic C++ ownership.
    struct RegionFileHeader {
        char name[16]; int pointCount; unsigned int uninterpreted14;
    } regionHeader;
    char staging348[0x480 - 0x348];
    union {
        float modelMatrix[12];
        struct { float basis[9]; float translation[3]; } modelTransform;
    };
    char staging4B0[0x610 - 0x4B0];
    CourseInputMeshReadView inputMesh;
    char staging6BC[0x6C8 - 0x6BC];
};
union CourseLoadWorkspace {
    unsigned char bytes[0x6C8];
    CourseLoadFrameView frame;
    // Borrow legacy ABI byte storage; never transfer ownership from this frame.
    operator unsigned char*() { return bytes; }
};
typedef char CourseLoadWorkspace_size[(sizeof(CourseLoadWorkspace) == 0x6C8) ? 1 : -1];
typedef char CourseRegionFileHeader_size[(sizeof(CourseLoadFrameView::RegionFileHeader) == 0x18) ? 1 : -1];
typedef char CourseLoadFrame_rootOffset[((int)DECOMP_OFFSETOF(CourseLoadFrameView, fallbackRootBone) == 0x174) ? 1 : -1];
typedef char CourseLoadFrame_regionOffset[((int)DECOMP_OFFSETOF(CourseLoadFrameView, regionHeader) == 0x330) ? 1 : -1];
typedef char CourseLoadFrame_translationOffset[((int)DECOMP_OFFSETOF(CourseLoadFrameView, modelTransform.translation) == 0x4A4) ? 1 : -1];
typedef char CourseLoadFrame_inputMeshOffset[((int)DECOMP_OFFSETOF(CourseLoadFrameView, inputMesh) == 0x610) ? 1 : -1];

enum CourseObjectOffsets {
    kCourseBaseMeshCountOffset = (int)DECOMP_OFFSETOF(ModelResource801BD2B8, baseMeshCount),
    kCourseSelectedMeshCountOffset = (int)DECOMP_OFFSETOF(ModelResource801BD2B8, selectedMeshCount),
    kCourseFallbackSurfaceOffset = (int)DECOMP_OFFSETOF(ModelResource801BD2B8, fallbackSurface),
    kCourseEntryContainerOffset = (int)DECOMP_OFFSETOF(ModelResource801BD2B8, entryContainer),
    kCourseFogStartOffset = (int)DECOMP_OFFSETOF(ModelResource801BD2B8, fogStart),
    kCourseFogEndOffset = (int)DECOMP_OFFSETOF(ModelResource801BD2B8, fogEnd),
};
// Weather values returned by the shared document.
static const int kCourseWeatherCloudy = 1;
static const int kCourseWeatherRainy = 2;
static const int kCourseWeatherVariantEnd = 3;
// Geometry groups are numeric file groups, not weather or day/night choices.
// 0 allocates base only; 1/2 allocate base plus one extra group; other values
// allocate both groups. Filtering skips group 1 only in mode 2 and vice versa.
// No geographic/game-mode names for these groups are established by this unit.
enum CourseGeometryVariant { kBaseMeshesOnly=0, kExtraMeshGroup0=1, kExtraMeshGroup1=2 };
enum CourseSceneMarkerKind { kMarkerPositionOnly=0, kMarkerLighting=1,
    kMarkerUninterpreted=2, kMarkerSetNight=3 };

enum CourseRegionKind {
    kRegionOutline = 1, kRegionInBounds = 2, kRegionOutOfBounds = 3,
    kRegionAuto = 4, kRegionBeach = 5, kRegionCamera = 6,
    kRegionRiver = 7, kRegionSea = 8, kRegionWave = 9,
    kRegionNpc = 10, kRegionLava = 11
};
enum CourseBoundsKind { kSoundBounds = 5, kEffectBounds = 6 };
enum CoursePreloadFlags {
    kPreloadSurfaceFlag08 = 1, kPreloadLargeOrFlag10 = 2, kPreloadCollision = 4
};

// Bounds embedded in the collision object returned by PuppetMesh_CreatePartSamplesAndBounds.
struct CourseCollisionNodeView { char pad_000[0x3C]; float bounds[6]; };

struct CourseMaterialCursorView { char pad_000[0x3C]; int resourceId; };
struct CourseBitmapResourceView { char pad_000[0x1C]; NString name; };

struct CourseFileInfo { void* vtable; void* byteLength; };
enum {
    kMeshMatrixOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, matrix),
    kMeshNameOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, modelName),
    kMeshBoundsOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, bounds),
    kMeshBoundsMaxOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, bounds[3]),
    kMeshExpandedBoundsOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, expandedBounds),
    kMeshCenterOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, center),
    kMeshCenterCopyOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, centerCopy),
    kMeshTrianglesOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, triangles),
    kMeshOriginDistanceOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, originDistance),
    kMeshPuppetOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, puppet),
    kMeshRadiusOffset = (int)DECOMP_OFFSETOF(CourseMeshRecord, radius),
    // char[32] at runtime mesh +0xBA, copied from the file record's tag (+0x8C). For Driving Contest booster gates it is two
    // decimal floats ("500 500" strongest sand gate, "5 15" weakest) that scale the mesh's basis vectors 1 and 2.
    kMeshTagOffset = 0xba,
    kCourseFormatVersion = 0x71, kCourseSurfaceKindMask = 7,
       kCourseSurfaceFlagsMask = 0xF8, kMaximumAttachedEffects = 256,
       kCourseIdCategory = 0x28000000, kCourseModelRecordBytes = 0xDC,
       kCourseBoundsRecordBytes = 0x5C, kCourseCameraRecordBytes = 0x58,
       kCourseMarkerRecordBytes = 0x3C, kCourseTriangleBytes = 0x48,
       kCourseBitmapRecordBytes = 0x28 };
typedef char CourseMeshRecord_size[(sizeof(CourseMeshRecord) == kCourseModelRecordBytes) ? 1 : -1];


#pragma dont_inline on
// Return the resource stream's byte length stored after its vtable.
void* CourseFile_GetByteLength(void* p) {
    return ((CourseFileInfo*)p)->byteLength;
}

#pragma dont_inline reset
void __dl__FPv(void*);

// Optionally release this allocation; the object has no teardown work here.
void* dtor_801BD204(void* p, int deleteFlag) {
    if (p != 0 && deleteFlag > 0) {
        __dl__FPv(p);
    }
    return p;
}

int CString_Compare(void*, void*);

// Compare mesh names, then the original asymmetric flag/kind tie-breaks used by qsort.
int CourseMesh_CompareNameFlagsAndKind(void* p1, void* p2) {
    int result = CString_Compare(((CourseMeshRecord*)p1)->modelName, ((CourseMeshRecord*)p2)->modelName);
    if (result != 0) {
        return result;
    }
    if (((CourseMeshRecord*)p1)->flags < ((CourseMeshRecord*)p2)->flags) {
        return -1;
    }
    return ((CourseMeshRecord*)p1)->collisionKind > ((CourseMeshRecord*)p2)->collisionKind;
}

void Vec3_Zero(void*);
void appFreeUni__FPv(void*);
void String_ReserveCapacity(void*, unsigned int);

struct Str;
struct Alloc { char c __attribute__((aligned(4))); };
Str* String_ReplaceRange(Str*, unsigned int, unsigned int, const char*, const char*, const Alloc&);

extern char __vt__14CNodeContainer[];
extern float kFloat_1_polygon;

// Clear mesh bounds/centers, runtime pointers, flags, and names; initialize the wind scale to one.
void* CourseMesh_InitRecord(void* p) {
    char* self = (char*)p;
    Vec3_Zero(self + 0x64);
    Vec3_Zero(self + 0x70);
    Vec3_Zero(self + 0x7c);
    Vec3_Zero(self + 0x88);
    float f0 = kFloat_1_polygon;
    ((CourseMeshRecord*)self)->triangleCount = 0;
    ((CourseMeshRecord*)self)->triangles = 0;
    ((CourseMeshRecord*)self)->puppet = 0;
    ((CourseMeshRecord*)self)->windScale = f0;
    *(int*)(self + 0x4) = 0;
    self[0x2] = 0;
    self[0xb9] = 0;
    self[0x44] = 0;
    self[kMeshTagOffset] = 0;
    return self;
}

// Release the node container's entry storage and owned name, optionally freeing the container.
void* CNodeContainer_dtor(void* p, int deleteFlag) {
    if (p) {
        unsigned* size = &AsCNodeContainer(p)->entryCount;
        int oldSize = *size;
        AsCNodeContainer(p)->vtbl = (void*)__vt__14CNodeContainer;
        unsigned emptySize = *(unsigned*)size - oldSize;
        *size = emptySize;
        if ((int)p + 0x14 != 0) {
            if ((int)p + 0x14 != 0) {
                if ((int)p + 0x14 != 0) {
                    if ((int)p + 0x14 != 0) {
                    void* resource = (*(void**)&AsCNodeContainer(p)->entries);
                    if (resource) {
                        AsCNodeContainer(p)->entryCount = *size - emptySize;
                        appFreeUni__FPv(resource);
                    }
                    }
                }
            }
        }
        if ((int)p + 4 != 0) {

            if ((AsCNodeContainer(p)->nameStringFlags) >> 31) {
                appFreeUni__FPv(AsCNodeContainer(p)->nameStringData);
            }
        }
        if (deleteFlag > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

struct CopyStringData { unsigned flags, count; const char* data; };
struct CopyObject { void* vtable; CopyStringData str; unsigned tail[4]; };
// Copy the name with the original inline/owned string behavior and initialize an empty entry vector.
void* CNodeContainer_CtorFromName(void* self, void* other) {
    char* s = (char*)self;
    char* o = (char*)other;
    CopyObject* obj = (CopyObject*)self;
    CopyStringData* src = (CopyStringData*)other;
    unsigned flags = src->flags;
    bool owned = flags >> 31;
    obj->vtable = __vt__14CNodeContainer;
    if (!owned) {
        const char* data = src->data;
        unsigned count = src->count;
        obj->str.flags = flags;
        obj->str.count = count;
        obj->str.data = data;
    } else {
        unsigned count = src->count;
        CopyStringData* dst = &obj->str;
        dst->flags = 0;
        dst->count = 0;
        dst->data = 0;
        String_ReserveCapacity(dst, count);
        Alloc a;
        register char allocator = a.c;
        const char* first = src->data;
        const char* last = first + src->count;
        Alloc copy;
        copy.c = allocator;
        String_ReplaceRange((Str*)&obj->str, 0, 0, first, last, copy);
    }
    AsCNodeContainer(s)->metadataWord = 0;
    AsCNodeContainer(s)->entries = 0;
    AsCNodeContainer(s)->entryCount = 0;
    AsCNodeContainer(s)->entryCapacity = 0;
    return self;
}

// Resource and parser helpers used by the long setup routine below.
extern void* g_resourceManager;
extern void* g_view;
extern char courseModelResourceStrings[];
extern unsigned int s_sceneLighting[7];
extern unsigned int kIdentityMatrix4x4[];
extern float kFloat_neg1_CourseModel, kCourseSoundDefaultParameter60, kFloat_0_PetBody;
extern float kFloat_1_polygon, kFloat_0_5_CourseSoundPosition;
void* Pointer_GetFirstPointer(void*);
void WPolySoup_ReleaseResources(void*);
void strncpy(void*, const void*, int);
int SharedDoc_GetSelectedCourseVariant(void);
void* Object_GetWord10(void*);
void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
void ResourceHolder_SetResourceAddress(void*, void*);
char* String_FormatTemporary(...);
void MessageBox_Report(void*, void*, void*, void*);
void ResourceHandle_dtor(void*, int);
void* __nwa__FUl(unsigned int);
void* __construct_new_array(void*, void*, void*, int, int);
// Initialize the named scene-marker string.
int CourseRecord_ClearVectorWords(...);
int CourseNamedPoint_dtor(...);
// Zero the two vectors in a bounds record.
int CourseRecord_ZeroBoundingVectors(...);
// No-op default constructor for a camera record.
int CourseCameraRecord_DefaultConstructNoOp(...);
void* CourseVector_Copy3(void*, void*);
void Matrix_BuildCameraTransform(void*, void*, void*);
void Matrix_CopyAffine(void*, void*);
void* MWString_AssignCString(void*, void*);
char* MWString_GetCString(void*);
int sscanf(...);
void __as__7WVectorFRC7WVector(void*, void*);
void GolfDoc_SetDaytime(void*, int); // Clear/set the course-model byte flag; its bit meanings are unresolved.
void* GolfDoc_GetCurrent(void);
unsigned int SharedDoc_GetCourseId(void);
int SharedDoc_GetItemInventory(void);
int ItemInventory_FindCourseData(int, int);
int CGolfDoc_GetCurrentHoleNumber(void*);
int CGolfDoc_LoadParameterXml(void*, void*, unsigned char);
char* SharedDoc_GetSelectedCourseMapName(void);
int CString_FindSubstringForwarder(void*, void*);
int Object_SetIntegerProperty10(void*, int);
int WVector_DefaultConstructNoOp(void*);
void __amu__7WVectorFf(void*, float);
int Normalize__7WVectorFv(void*);
int FourVectorState_Reset(void*);
int CourseIntVector_InitEmpty(void*);
int ResourceManager_LoadByExtension(void*, void*, int, int, int);
int WMatrix_DefaultConstructNoOp(void*);
int Matrix_SetIdentityAffine(void*);
void Matrix3x3_ScaleAssign(void*, float);
int WPuppet_GetPivot(void*, void*, int);
void __ml__FRC7WVectorf(void*, void*, float);
int __ami__7WVectorFRC7WVector(void*, void*);
int Matrix_AppendEulerRotation(void*, void*);
int __apl__7WVectorFRC7WVector(void*, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, void*, int, float);
int PuppetMesh_CreatePartSamplesAndBounds(void*, int);
int CourseIntVector_Append(void*, void*);
int AABB_Copy(void*, void*);
int __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FfRC7WVector(void*, void*, float);
int __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
float Magnitude__7WVectorCFv(void*);
int Object_SetWord14(void*, int);
int Model_SetMatrixRecursive(void*, int, void*);
int WPuppet_UpdateBound(void*, int);
int WPuppet_GetTriangleCount(void*);
int CourseTriangle_Init(...);
int WPuppet_GetRootBone(void*);
void Option_GetInstance(void);
int CourseObject_DefaultFalseQuery(void);
int WPuppet_CreateMeshByBoneName(void*, void*, int);
int IPetMesh_GetGroupCount(void*);
int IPetMesh_GetGroupByIndex(void*, unsigned short);
int PuppetBone_GetWorldMatrix(void*);
int Matrix_TransformPoint(void*, void*, void*);
int Vector_CrossProduct(void*, void*, void*);
float __ml__FRC7WVectorRC7WVector(void*, void*);
int WPuppet_FindNamedBoneEntryForward(void*, void*);
int BorrowedNameRef_Set(void*, void*);
int String_ScanTokensAfterSubstring(...);
int CourseModelPointerVector_Append(void*, void*);
// Return the shared document's current weather kind.
int CourseResource_GetSelectedVariant(void);
// Return the golf document's daytime flag.
int CGolfDoc_IsDaytime(void*);
int CourseModelString_InsertString(void*, int, void*);
int CourseBounds_CopySixWords(void*, void*);
int WPuppet_ComputeMeshBounds(void*, void*, void*);
int NameRegistry_FindByName(void*, void*);
int CourseNameRegistry_Insert(void*, void*, void*, int);
int CourseObject_GetEmbeddedData(void*);
void* SceneManager_GetCurrent(void);
int ResourcePreloadQueue_AddRequest(void*, int, void*, float, int, float, void*, void*, int, void*);
int Storage_ClearThreeWords(void*);
int MWString_AppendCString(void*, void*);
int MWString_FindCString(void*, void*, int);
int CourseModelTreeIterator_GetNodeAdapter(void*);
int IntStringTree_GetOrInsertValue(void*, void*);
int CourseModelPointerIterator_PostIncrement(void*, int);
int CourseModelPointerVector_End(void*);
// Compare pointer iterators for inequality.
int CourseModelPointerIterator_NotEqual(void*, void*);
int CourseModelPointerIterator_Get(void*);
// Clear a bitmap-name record's two trailing words.
int CourseResourcePair_Clear(...);
int CourseModelIterator_EmptyCleanup(void*);
int CourseModelTreeIterator_FromFirstNode(void*, void*);
int CourseModelTreeIterator_Copy(void*, void*);
int CourseModelTreeIterator_GetPayload(void*);
int ResourceManager_LoadBitmap(void*, void*, int, int, int);
int CourseData_FindEntryByName(void*, void*);
int CourseMaterialMap_GetOrInsertValue(void*, void* = 0);
int CourseModelTreeIterator_PostIncrement(void*, void*, int);
int CourseModelByteBuffer_ClearSize(void*);
int CourseObject_ReleaseReference(void*);
int qsort(void*, int, int, void*);
int CQuadTree_GetInstance(void);
int CQuadTree_ClearObjects(int, int);
int CFx_GetInstance(void);
int CFx_CreateAttachedEffect(int, void*, void*, int, int, int);
int CString_FindLastCharForwarder(void*, int);
int FxSpray_NoOpLifecycleHook(void*, void*);
int CFxSequence_InvokeSprayLifecycleHooks(void*, void*);
int DefaultQuery_ReturnZero(void);
int CourseData_LoadSurfaceBinary(void*);
int ShadowManager_GetCurrent(void);
int CShadowManager_SetSelectionValue(int, float, void*, void*);
int Resource_Release(void*);
int SharedDoc_GetCourseResourceName(void);
int strstr(const char*, void*);
int CourseResource_BuildVariantName(void*, int, int);
void MWString_ConstructCString(void*, void*);
int MWString_CopyConstruct(void*, void*);
int ScriptString_GetCharacterData(void*);
int CHashPath_FindPath(void*);
int RenderPacket_GetInstance(void);
int TransformRecord_CopyMatrix4x4(int, void*);
int CBalloonSystem_DestroyAllQueuedBalloons(void);
int CRival_GetInstance(void);
int CRival_GetShotPositionThinker(void*);
int CComThinkShotPos_ClearCachedData(int);
int String_ReplaceRangeCountAdapter(void*, int, int, void*, int);
int String_InitEmptyStorage(void*);
int CComThinkShotPos_AddCandidate(int, void*);
int CGolfDoc_GetCurrentHoleCupPosition(void*);
int CGolfDoc_GetCurrentHoleStartPosition(void*);
int SharedDoc_GetCurrent(void);
int Engine_GetLightEnabled(int, void*);
int CourseRenderPacket_LoadSBINArchive(int, void*, unsigned char, int);
int CBalloonSystem_Spawn(void*, int);
int Color_ScaleRGB(unsigned int, float);
int Bounds_BuildCenterAndHalfExtentAxes(void*, void*);
char* strcpy(char*, const char*);
void dtor_801C15D8(void*, int);
void IntStringTree_dtor(void*, int);
void dtor_801C2A88(void*, int);
void ByteVector_dtor(void*, int);
void dtor_801C315C(void*, int);
void* appMallocUni__FUl(int);
int ShadowManager_IsCreated(void);
int FxAttachment_Init(void*);
int Engine_SetLightEnabled(int, int);
int Engine_GetRenderState(int, void*);
int CourseRenderStates_ApplyPair(void*, int);
int String_AssignWithInlineFastPath(void*, void*);
int CourseObjectName_MatchesFilterPattern(void*, void*);
int String_CompareCString(void*, void*);
int CourseData_ApplyModelNodeTags(void*, void*);
int CourseModelAllocator_EmptyCleanup(void*);
int CourseMaterialMap_InitEmpty(void*);
int CourseModelTreeIterator_GetNode(void*);
int CourseModelTreeIterator_Assign(void*, void*);
int CourseModelObjectIterator_GetFirst(void*);
int CourseModelObjectIterator_GetNext(void*);
int CourseObjectList_Dtor(void*, int);
int ObjectList_FindById(void*, int);
int CourseModelByteBuffer_GetElement(void*, int);
int CourseModelRecordVector_InitEmpty(void*);
int CourseModelRecordVector_ResizeZeroFilled(void*, int);
int Array_GetElement88(void*, int);
int RenderPacket_CopyLightDescription(int, void*);
int gtl_IGenericEnvironmentResource_CreateFromMemory(void*, int, void*);
int gtl_IGenericEnvironmentResource_Apply(void*, int);
int gtl_IGenericEnvironmentResource_GetLightCount(void*, void*);
int gtl_IGenericEnvironmentResource_CopyLights(void*, int, int);
int gtl_GenericLight_GetDescription(void*, int);
void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);
unsigned int strlen(const char*);
// No-op default constructor for a course sound-emitter record.
void CourseSoundEmitter_DefaultConstructNoOp(void*);
// Prepend a copied sound-emitter record to the course's owned list.
void CourseNodeList_PrependRecord(void*, void*);
void TripleRecordVector_Append(void*, void*);
void CNodeContainer_Append(void*, void*);

struct Header {
 virtual void a(); virtual void b(); virtual void c(); virtual void d();
 virtual void e(); virtual void f(); virtual void g();
 virtual void query(int, int, void*);
};
struct Model {
 virtual void a(); virtual void b(); virtual void c();
 virtual void setup(void*, int, int);
};
struct Input {
    virtual void slot0(); virtual void slot1();
    virtual void read(void*, int);
};
// Request this byte count through the stream; no returned byte count or short-read check is available here.
// This boundary does no endian conversion: multibyte integer/float fields below
// are interpreted in native Wii big-endian order, with binary32 floats. The read
// interface must supply those bytes in full; this is not a validating parser.
static inline void readData(void* stream, void* dst, int size) {
    ((Input*)stream)->read(dst, size);
}

int CourseModelByteBuffer_Init(void*); int CourseModelByteBuffer_ResizeZeroFilled(void*, int);
extern char kCourseNpcFallbackMarker[1];
int Matrix_MoveLastVectorToFront(void*, void*); int AffineVectors_ScaleTrailingThree(void*, void*);
struct ArrayA { char data[0xdc];
 // Initialize one owned runtime mesh record.
 ArrayA() { CourseMesh_InitRecord(this); }
 // Allocate the array, including the compiler-managed array header.
 static void* operator new[](new_size_t n) { return __nwa__FUl(n); }
};
struct ArrayB { char data[0x3c];
 // Clear one scene-marker record before file data is read.
 ArrayB() { CourseRecord_ClearVectorWords(this); }
 // Destroy the scene marker and its owned name string.
 ~ArrayB() { CourseNamedPoint_dtor(this, -1); }
 // Allocate the array, including the compiler-managed array header.
 static void* operator new[](new_size_t n) { return __nwa__FUl(n); }
};
struct ArrayC { char data[0x5c];
 // Zero one bounds record before reading its file fields.
 ArrayC() { CourseRecord_ZeroBoundingVectors(this); }
 // Allocate the array, including the compiler-managed array header.
 static void* operator new[](new_size_t n) { return __nwa__FUl(n); }
};
struct ArrayD { char data[0x58];
 // Preserve the camera record default-construction hook.
 ArrayD() { CourseCameraRecord_DefaultConstructNoOp(this); }
 // Allocate the array, including the compiler-managed array header.
 static void* operator new[](new_size_t n) { return __nwa__FUl(n); }
};
struct ArrayE { char data[0x48];
 // Initialize one world-space collision triangle record.
 ArrayE() { CourseTriangle_Init(this); }
 // Allocate the array, including the compiler-managed array header.
 static void* operator new[](new_size_t n) { return __nwa__FUl(n); }
};
struct ArrayF { char data[0x28];
 // Clear one model/bitmap resource pair.
 ArrayF() { CourseResourcePair_Clear(this); }
 // Allocate the array, including the compiler-managed array header.
 static void* operator new[](new_size_t n) { return __nwa__FUl(n); }
};
struct MainRecord {
    char data[0xdc];
    // Initialize the fallback runtime mesh record.
    MainRecord() { CourseMesh_InitRecord(this); }
};
// Semantic allocation aliases preserve existing constructor/destructor symbols.
// Counts in the file header choose element counts; sizeof each storage type is its runtime stride.
typedef ArrayA CourseMeshStorage;
typedef ArrayB CourseSceneMarkerStorage;
typedef ArrayC CourseBoundsStorage;
typedef ArrayD CourseCameraStorage;
typedef ArrayE CourseTriangleStorage;
typedef ArrayF CourseResourcePairStorage;
typedef char CourseMeshStorage_stride[(sizeof(CourseMeshStorage) == kCourseModelRecordBytes) ? 1 : -1];
typedef char CourseMarkerStorage_stride[(sizeof(CourseSceneMarkerStorage) == kCourseMarkerRecordBytes) ? 1 : -1];
typedef char CourseBoundsStorage_stride[(sizeof(CourseBoundsStorage) == kCourseBoundsRecordBytes) ? 1 : -1];
typedef char CourseCameraStorage_stride[(sizeof(CourseCameraStorage) == kCourseCameraRecordBytes) ? 1 : -1];
typedef char CourseTriangleStorage_stride[(sizeof(CourseTriangleStorage) == kCourseTriangleBytes) ? 1 : -1];
typedef char CourseResourcePairStorage_stride[(sizeof(CourseResourcePairStorage) == kCourseBitmapRecordBytes) ? 1 : -1];

// File bounds are tightly serialized: u8 kind, char tag[64], XYZ minimum, XYZ maximum.
// Runtime alignment inserts three bytes before the two XYZ vectors; those bytes are not read.
struct CourseBoundsReadView {
    unsigned char kind;
    char tag[64];
    char alignment41[3];
    Vec3f minimum;
    Vec3f maximum;
};
// Mesh group -> 16-byte batches -> consecutive triples of local-space XYZ vertices.
// The two unmodeled batch words are retained without assigning an index-buffer meaning.
struct CourseVertexBatchView {
    unsigned int unresolved00;
    unsigned int vertexCount;
    Vec3f* vertices;
    unsigned int unresolved0C;
};
struct CourseMeshGroupView { unsigned int batchCount; CourseVertexBatchView* batches; };
struct CourseTriangleView {
    WVector corners[3];
    WVector normal;
    float planeConstant;
    unsigned char flags;
    char unresolved35[0x13];
};
// Borrow a triangle at a byte offset; buffer ownership stays with the fallback mesh.
static inline CourseTriangleView* CourseTriangleAt(void*& meshRecord, unsigned int& byteOffset) {
    return (CourseTriangleView*)((char*)((CourseMeshRecord*)meshRecord)->triangles + byteOffset);
}
// Address a batch through its byte stride; its vertexCount and vertices fields retain element units.
static inline CourseVertexBatchView* CourseVertexBatchAt(CourseMeshGroupView* group, unsigned int byteOffset) {
    return (CourseVertexBatchView*)((char*)group->batches + byteOffset);
}
typedef char CourseBoundsReadView_size[sizeof(CourseBoundsReadView)==0x5C ? 1 : -1];
typedef char CourseVertexBatchView_size[sizeof(CourseVertexBatchView)==0x10 ? 1 : -1];
typedef char CourseTriangleView_size[sizeof(CourseTriangleView)==0x48 ? 1 : -1];

struct Bits { unsigned x,y,z; };
typedef Vec3f Vec;
// Load per-hole property XML from IFF or the map-name fallback.
// Borrows owner/frame storage; appended records are copied before local cleanup.
static inline void LoadCoursePropertyParameters(char*& s, int& i) {
    // Resolve course properties from IFF, falling back to the map property XML.
    unsigned int flag; flag = 0;
    i = SharedDoc_GetCourseId();
    if ((unsigned char)i > 0x7f && (unsigned char)SharedDoc_GetCourseId() != 0xfd) flag = 1;
    if (flag) i = ((unsigned char)SharedDoc_GetCourseId() - 0x80);
    else i = (unsigned char)SharedDoc_GetCourseId();

    IFF_STRUCT::sCourse* selected = (IFF_STRUCT::sCourse*)ItemInventory_FindCourseData((SharedDoc_GetItemInventory()), kCourseIdCategory | i);
    if (!(!(selected && 0 != selected->propertyFile[0]))) {
        void* golfDocForPropertyFile = (void*)GolfDoc_GetCurrent();

        CGolfDoc_LoadParameterXml(AsModelResource801BD2B8(s)->entryContainer, selected->propertyFile, ((unsigned char)(CGolfDoc_GetCurrentHoleNumber(golfDocForPropertyFile) - 1)));
    } else {
        void* golfDocForDefaultProperties = (void*)GolfDoc_GetCurrent();
        unsigned char c = (unsigned char)(CGolfDoc_GetCurrentHoleNumber(golfDocForDefaultProperties) - 1);
        char* name = String_FormatTemporary(courseModelResourceStrings + kCourseStringsPropertyFileFormatOffset, SharedDoc_GetSelectedCourseMapName());
        CGolfDoc_LoadParameterXml(AsModelResource801BD2B8(s)->entryContainer, name, c);
    }
}

// Read, classify and copy world-space polygon regions.
// Borrows owner/frame storage; appended records are copied before local cleanup.
static inline void ReadCoursePolygonRegions(char*& s, CourseLoadWorkspace& scratch, void*& stream, int& i, int& j) {
    // Load named polygons and classify playable, water, camera, and hazard regions.
    AsModelResource801BD2B8(s)->regionMode = 0;
    for (i = 0; i < AsModelResource801BD2B8(s)->regionDefinitionCount; ++i) {
        // File header: 16-byte C-string field, signed point count, then one uninterpreted word.
        // The format must terminate the name within its field; this reader adds no terminator.
        readData(stream, &scratch.frame.regionHeader, sizeof(CourseLoadFrameView::RegionFileHeader));
        MWString_ConstructCString(scratch.frame.regionName, scratch.frame.regionHeader.name);
        CNodeContainer_CtorFromName(scratch + kScratchRegion, scratch.frame.regionName);
        ((NString*)(scratch.frame.regionName))->~NString();
        // Tests intentionally accumulate in this order: later tags overwrite
        // earlier kinds. Only camera/lava are mutually exclusive via else-if.
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsOutlineTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionOutline);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsInBoundsTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionInBounds);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsOutOfBoundsTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionOutOfBounds);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsAutoTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionAuto);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsBeachTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionBeach);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsRiverTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionRiver);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsSeaTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionSea);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsWaveTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionWave);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsNpcTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionNpc);
        if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsCameraTagOffset)) {
            AsModelResource801BD2B8(s)->regionMode = 1;
            Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionCamera);
        } else if (CString_FindSubstringForwarder(scratch.frame.regionHeader.name, courseModelResourceStrings + kCourseStringsLavaTagOffset)) Object_SetIntegerProperty10(scratch + kScratchRegion, kRegionLava);
        for (j = 0; j < scratch.frame.regionHeader.pointCount; ++j) {
            WVector_DefaultConstructNoOp(scratch.frame.runtimeRegionPoint);
            readData(stream, scratch.frame.serializedRegionPoint, 0xc);
            *(Vec*)(scratch.frame.runtimeRegionPoint) = *(Vec*)(scratch.frame.serializedRegionPoint);
            *(Bits*)(scratch.frame.regionPointBits) = *(Bits*)(scratch.frame.runtimeRegionPoint);
            TripleRecordVector_Append(scratch + kScratchRegion, scratch.frame.regionPointBits);
        }
        CNodeContainer_Append(s + 0x14, scratch + kScratchRegion);
        CNodeContainer_dtor(scratch + kScratchRegion, -1);
    }
}

// Apply the primary environment, scale fog distances and release its stream holder.
// Owns parsed resource/raw bytes only within this phase; endStream is cleared before holder cleanup.
static inline void ApplyCourseEnvironment(void* self, char*& s, CourseLoadWorkspace& scratch, void*& endStream, int& i) {
    if (endStream) {
        for (i = 0; i < 8; ++i) Engine_SetLightEnabled(i, 0);
        void* resource = appMallocUni__FUl((int)CourseFile_GetByteLength(endStream));

        readData(endStream, resource, ((int)CourseFile_GetByteLength(endStream)));
        gtl_IGenericEnvironmentResource_CreateFromMemory(resource, (int)CourseFile_GetByteLength(endStream), scratch + 0x1c);
        gtl_IGenericEnvironmentResource_Apply(*(void**)(scratch + 0x1c), 0);
        Engine_GetRenderState(0x17, s + kCourseFogStartOffset);
        Engine_GetRenderState(0x18, s + kCourseFogEndOffset);
        float first = AsModelResource801BD2B8(s)->fogStart;
        float factor = 0.32f;

        float a = ((first * factor));
        float b = (AsModelResource801BD2B8(s)->fogEnd) * factor;
        AsModelResource801BD2B8(s)->fogStart = a;
        AsModelResource801BD2B8(s)->extendedFogStart = a * 3.0f; // original +0x493c + 4
        AsModelResource801BD2B8(s)->fogEnd = b;
        AsModelResource801BD2B8(s)->extendedFogEnd = b * 3.0f; // original +0x4944 + 4
        CourseRenderStates_ApplyPair(self, 0);
        Engine_GetLightEnabled(7, scratch + 0x18);
        CourseObject_ReleaseReference(*(void**)(scratch + 0x1c));
        appFreeUni__FPv(resource);
        if (endStream) Resource_Release(endStream);
        endStream = 0;
    }
    ResourceHandle_dtor(scratch + 0x20, -1);
}

// Append copied cup/start rival candidates using world-space endpoint positions.
// Candidate storage at +378 remains live for the caller to destroy after archive loading.
static inline void AppendCourseEndpointCandidates(CourseLoadWorkspace& scratch, int& records) {
    String_InitEmptyStorage(scratch + 0x378);
    void* golfDocForHoleName = (void*)GolfDoc_GetCurrent();
    MWString_AssignCString(scratch + 0x378, String_FormatTemporary(courseModelResourceStrings + kCourseStringsHoleNameFormatOffset, (unsigned char)CGolfDoc_GetCurrentHoleNumber(golfDocForHoleName)));
    CGolfDoc_GetCurrentHoleCupPosition(scratch + 0x60);
    *(WVector*)(scratch + 0x384) = *(WVector*)(scratch + 0x60);
    *(float*)(scratch + 0x390) = 2.0f;
    *(int*)(scratch + 0x398) = 0;
    *(int*)(scratch + 0x394) = -1;
    *(int*)(scratch + 0x39c) = 0;
    *(int*)(scratch + 0x3a0) = 0;
    *(int*)(scratch + 0x3a4) = 0;
    *(int*)(scratch + 0x3a8) = -1;
    CComThinkShotPos_AddCandidate(records, scratch + 0x378);
    MWString_AssignCString(scratch + 0x378, courseModelResourceStrings + kCourseStringsStartNameOffset);
    CGolfDoc_GetCurrentHoleStartPosition(scratch + 0x54);
    *(WVector*)(scratch + 0x384) = *(WVector*)(scratch + 0x54);
    *(int*)(scratch + 0x394) = 1000;
    *(float*)(scratch + 0x390) = 2.0f;
    *(int*)(scratch + 0x398) = 0;
    *(int*)(scratch + 0x39c) = 0;
    *(int*)(scratch + 0x3a0) = 0;
    *(int*)(scratch + 0x3a4) = 0;
    *(int*)(scratch + 0x3a8) = 1000;
    CComThinkShotPos_AddCandidate(records, scratch + 0x378);
}

// Release main-course input and all temporary collision/bitmap/name owners.
// Runtime mesh/resource records already hold their copies; later streams have separate holders.
static inline void ReleaseMainCourseInput(CourseLoadWorkspace& scratch, void*& stream) {
    // End main-stream phase: runtime records now retain their copies/resources;
    // free the collision/bitmap vectors and path strings before later file phases.
    if (stream) Resource_Release(stream);
    stream = 0;
    IntStringTree_dtor(scratch + kScratchBitmapNames, -1);
    ((NString*)(scratch.frame.modelPath))->~NString();
    ((NString*)(scratch.frame.weatherSuffix))->~NString();
    dtor_801C15D8(scratch + kScratchCollisionData, -1);
    ResourceHandle_dtor(scratch + 0x48, -1);
}

// Read one camera and convert its world-space position/target to an affine transform.
static inline void ReadCourseCameraRecord(char*& item, CourseLoadWorkspace& scratch, void*& stream, void*& second) {
    readData(stream, item, 0x20);
    readData(stream, scratch + 0x2d0, 0xc);
    readData(stream, scratch + 0x2c4, 0xc);
    second = CourseVector_Copy3(scratch + 0x1a4, scratch + 0x2c4);
    void* first = CourseVector_Copy3(scratch + 0x1b0, scratch + 0x2d0);
    Matrix_BuildCameraTransform(scratch + 0x348, first, second);
    Matrix_CopyAffine(item + 0x20, scratch + 0x348);
    readData(stream, item + 0x50, 4);
    readData(stream, item + 0x54, 4);
}

// Read one scene marker, applying only established lighting/daytime marker actions.
static inline void ReadCourseSceneMarker(char*& item, CourseLoadWorkspace& scratch, void*& stream) {
    readData(stream, item, 1);
    readData(stream, (1 + item), 0x20);
    readData(stream, scratch + 0x2b8, 0xc);
    *(float*)(item + 0x24) = *(float*)(scratch + 0x2b8);
    *(float*)(item + 0x28) = *(float*)(scratch + 0x2bc);
    *(float*)(item + 0x2c) = *(float*)(scratch + 0x2c0);
    if (*(unsigned char*)item) {
        readData(stream, scratch + 0x4b0, 0x40);
        MWString_AssignCString(item + 0x30, scratch + 0x4b0);
    }
    switch (*(unsigned char*)item) {
    case kMarkerLighting: { // Override the scene direction and three colors.
        char* text = MWString_GetCString(item + 0x30); sscanf(text, courseModelResourceStrings + kCourseStringsLightingFormatOffset,
                    (s_sceneLighting + 4), 5 + s_sceneLighting, 6 + s_sceneLighting);
        *(WVector*)(s_sceneLighting + 1) = *(WVector*)(item + 0x24);
        break;
    }
    case kMarkerPositionOnly: // Retain position without scene-lighting side effects.
    case kMarkerUninterpreted: break; // No lighting action for this marker kind.
    case kMarkerSetNight: /* Set the golf document to nighttime. */ GolfDoc_SetDaytime((void*)GolfDoc_GetCurrent(), 0); break;
    }
}

// Read one aligned world-space bounds record and copy an optional sound emitter.
static inline void ReadCourseBoundsRecord(CourseBoundsReadView*& item, CourseLoadWorkspace& scratch, void*& stream, char*& s) {
    readData(stream, item, 1);
    readData(stream, item->tag, 0x40);
    readData(stream, scratch + 0x2ac, 0xc);
    item->minimum.x = ((Vec3f*)(scratch + 0x2ac))->x;
    item->minimum.y = ((Vec3f*)(scratch + 0x2ac))->y;
    item->minimum.z = ((Vec3f*)(scratch + 0x2ac))->z;
    readData(stream, scratch + 0x2ac, 0xc);
    item->maximum.x = ((Vec3f*)(scratch + 0x2ac))->x;
    item->maximum.y = ((Vec3f*)(scratch + 0x2ac))->y;
    item->maximum.z = ((Vec3f*)(scratch + 0x2ac))->z;
    if (item->kind == kSoundBounds) {
        CourseSoundEmitter_DefaultConstructNoOp(scratch + 0x5b4);
        float fminus = -1.0f; *(float*)(scratch + 0x5d8) = fminus;
        *(float*)(scratch + 0x5dc) = (60.0f);
        *(float*)(scratch + 0x5d4) = (0.0f);
        sscanf(item->tag, courseModelResourceStrings + kCourseStringsSoundFormatOffset, scratch + 0x5b4,
                    scratch + 0x5dc, scratch + 0x5d8);
        Bounds_BuildCenterAndHalfExtentAxes(scratch + 0x5e0, &item->minimum);
        CourseNodeList_PrependRecord(s + 0x490c, scratch + 0x5b4);
    }
}

// Compute a world-space triangle unit normal and plane d=-dot(n,v0).
// Keeps the original cross-product order and degenerate Normalize behavior.
static inline void FinalizeCourseTriangle(char*& s, CourseLoadWorkspace& scratch, unsigned int& triangleOffset, CourseTriangleView*& triangle) {
    __mi__FRC7WVectorRC7WVector(scratch + 0x108, &triangle->corners[0], &triangle->corners[1]);
    __mi__FRC7WVectorRC7WVector(scratch + 0x114, &triangle->corners[2], &triangle->corners[1]);
    Vector_CrossProduct(scratch + 0x120, scratch + 0x114, scratch + 0x108);
    CourseTriangleAt(AsModelResource801BD2B8(s)->fallbackSurface, triangleOffset)->normal = *(WVector*)(scratch + 0x120);
    (CourseTriangleAt(AsModelResource801BD2B8(s)->fallbackSurface, triangleOffset)->normal).Normalize();
    __ml__FfRC7WVector(scratch + 0xfc, &CourseTriangleAt(AsModelResource801BD2B8(s)->fallbackSurface, triangleOffset)->normal, -1.0f);

    float planeConstant = ((*(WVector*)(scratch + 0xfc) * triangle->corners[0]));
    CourseTriangleAt(AsModelResource801BD2B8(s)->fallbackSurface, triangleOffset)->planeConstant = planeConstant;
    CourseTriangleAt(AsModelResource801BD2B8(s)->fallbackSurface, triangleOffset)->flags = 0;
}

// Copy the alternate environment's first light description, then release its temporary owners.
// parsed/lightCount are caller slots because the runtime writes through their addresses.
static inline void CopyCourseAlternateEnvironmentLight(CourseLoadWorkspace& scratch, void*& endStream, int& size, void*& parsed, int& lightCount) {
    gtl_IGenericEnvironmentResource_CreateFromMemory((void*)CourseModelByteBuffer_GetElement(scratch + 0x214, 0), size, &parsed);
    lightCount = 0;
    gtl_IGenericEnvironmentResource_GetLightCount(parsed, &lightCount);
    CourseModelRecordVector_InitEmpty(scratch + 0x208);
    CourseModelRecordVector_ResizeZeroFilled(scratch + 0x208, lightCount);
    gtl_IGenericEnvironmentResource_CopyLights(parsed, Array_GetElement88(scratch + 0x208, 0), lightCount);
    gtl_GenericLight_GetDescription(scratch + 0x4f0, Array_GetElement88(scratch + 0x208, 0));
    RenderPacket_CopyLightDescription(RenderPacket_GetInstance(), scratch + 0x4f0);
    if (parsed) { CourseObject_ReleaseReference(parsed); parsed = 0; }
    dtor_801C2A88(scratch + 0x208, -1);
    if (endStream) Resource_Release(endStream);
    endStream = 0;
    ByteVector_dtor(scratch + 0x214, -1);
}

// Read/copy rival candidates and release the independent rival-file stream holder.
// Every candidate name is destroyed only after AddCandidate has copied it.
static inline void ReadCourseRivalCandidates(void*& resource, int& records, CourseLoadWorkspace& scratch, int& i) {
    if (resource) {
        readData(resource, scratch + 0x1f8, 0x10);
        for (i = 0; i < *(int*)(scratch + 0x200); ++i) {
            readData(resource, scratch + 0x3e0, 0x34);
            String_InitEmptyStorage(scratch + 0x3ac);
            MWString_AssignCString(scratch + 0x3ac, scratch + 0x3e0);
            *(Vec*)(scratch + 0x3b8) = *(Vec*)(scratch + 0x400);
            *(float*)(scratch + 0x3c4) = *(float*)(scratch + 0x40c);
            *(int*)(scratch + 0x3c8) = *(int*)(scratch + 0x410);
            *(int*)(scratch + 0x3cc) = 0;
            *(int*)(scratch + 0x3d0) = 0;
            *(int*)(scratch + 0x3d4) = 0;
            *(int*)(scratch + 0x3d8) = 0;
            *(int*)(scratch + 0x3dc) = i;
            CComThinkShotPos_AddCandidate(records, scratch + 0x3ac);
            dtor_801C315C(scratch + 0x3ac, -1);
        }
        if (resource) Resource_Release(resource);
        resource = 0;
    }
    ResourceHandle_dtor(scratch + 0x00, -1);
}

// Select the weather/daytime suffix in an existing empty string; caller owns it.
// Keep this dispatch local: CodeWarrior's branch polarity is part of the byte identity.
static inline void SelectCourseMeshSuffix(CourseLoadWorkspace& scratch, int& weatherKind) {
    // Select the mesh-name weather suffix before appending the day/night suffix.
    // Retain this dispatch tree: structured alternatives change branch order or polarity.
    do {
        if (weatherKind == kCourseWeatherCloudy) goto cloudyModelSuffix;
        if (kCourseWeatherCloudy <= weatherKind) {
            if (weatherKind >= kCourseWeatherVariantEnd) goto defaultModelSuffix;
            goto rainyModelSuffix;
        }
        goto defaultModelSuffix;
    cloudyModelSuffix:
        MWString_AssignCString(scratch.frame.weatherSuffix, courseModelResourceStrings + kCourseStringsCloudSuffixOffset);
        break;
    rainyModelSuffix:
        MWString_AssignCString(scratch.frame.weatherSuffix, courseModelResourceStrings + kCourseStringsRainSuffixOffset);
        break;
    defaultModelSuffix:
        MWString_AssignCString(scratch.frame.weatherSuffix, courseModelResourceStrings + kCourseStringsEmptyOffset);
        break;
    } while (0);
    void* golfDocForWeather = (void*)GolfDoc_GetCurrent();
    if (!CGolfDoc_IsDaytime(golfDocForWeather)) MWString_AppendCString(scratch.frame.weatherSuffix, courseModelResourceStrings + kCourseStringsNightSuffixOffset);

}

#pragma inline_depth(32)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)

// Attach effects from bounds tags, register collision capacity and initialize optional shadow state.
// Effects borrow transforms in course-owned storage; input parsing is complete.
static inline void SetupCourseEffectsAndShadows(void* self, char*& courseBytes, CourseLoadWorkspace& scratch, unsigned int& collisionMeshCount, int& recordIndex) {
    // Phase 6: register collision capacity, attached effects and optional shadows.
    // Each accepted effect retains pointers to self's transform/offset storage.
    CQuadTree_ClearObjects(CQuadTree_GetInstance(), collisionMeshCount);
    AsModelResource801BD2B8(courseBytes)->attachedEffectCount = 0;
    for (recordIndex = 0; recordIndex < AsModelResource801BD2B8(courseBytes)->boundsCount; ++recordIndex) {
        char* item = (char*)(AsModelResource801BD2B8(courseBytes)->bounds) + recordIndex * kCourseBoundsRecordBytes;
        if ((unsigned char)*item == kEffectBounds) {
            FxAttachment_Init(scratch + kScratchEffectAttachment);
            WVector_DefaultConstructNoOp(scratch + 0x238);
            BorrowedNameRef_Set(scratch + 0x28, item + 1);
            String_ScanTokensAfterSubstring(scratch + 0x28, courseModelResourceStrings + kCourseStringsEffectTagOffset, courseModelResourceStrings + kCourseStringsEffectFormatOffset,
                        scratch + kScratchEffectName, scratch + 0x238, scratch + 0x23c, scratch + 0x240);
            if (CString_Compare(scratch + kScratchEffectName, courseModelResourceStrings + kCourseStringsExcludedSequenceOffset)) {
                *(void**)((courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0xd4) = item;
                Matrix_SetIdentityAffine((courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0xe0);
                Matrix_AppendEulerRotation((courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0xe0, scratch + 0x238);
                __pl__FRC7WVectorRC7WVector(scratch + 0x6c, item + 0x44, item + 0x50);
                __ml__FRC7WVectorf(scratch + 0x78, scratch + 0x6c, 0.5f);
                *(WVector*)((courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0x104) = *(WVector*)(scratch + 0x78);
                Vec3_Zero((courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0x110);
                *(void**)(scratch + 0x420) = (courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0xe0;
                *(void**)(scratch + 0x430) = (courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0x110;
                void* effect = (void*)CFx_CreateAttachedEffect(CFx_GetInstance(), scratch + kScratchEffectAttachment, scratch + kScratchEffectName, 0, 0, 0);
                if (effect) {
                    int suffix = CString_FindLastCharForwarder(scratch + kScratchEffectName, '.');
                    if (suffix == 0 || strlen((char*)suffix) < 4) continue;
                    if (String_CompareCString((char*)suffix + 1, courseModelResourceStrings + kCourseStringsSprayExtensionOffset) == 0) {
                        FxSpray_NoOpLifecycleHook(effect, s_sceneLighting);
                        *(void**)((courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0xd8) = effect;
                    } else if (String_CompareCString((char*)suffix + 1, courseModelResourceStrings + kCourseStringsSequenceExtensionOffset) == 0) {
                        CFxSequence_InvokeSprayLifecycleHooks(effect, s_sceneLighting);
                        *(void**)((courseBytes + AsModelResource801BD2B8(courseBytes)->attachedEffectCount * 0x48) + 0xdc) = effect;
                    } else continue;
                    if (++AsModelResource801BD2B8(courseBytes)->attachedEffectCount >= kMaximumAttachedEffects) break;
                }
            }
        }
    }
    AsModelResource801BD2B8(courseBytes)->surfaceBinaryLoadFailed = 0; // Surface-binary load failure flag.
    Option_GetInstance();
    if (DefaultQuery_ReturnZero()) AsModelResource801BD2B8(courseBytes)->surfaceBinaryLoadFailed = CourseData_LoadSurfaceBinary(self) == 0;
    if (ShadowManager_IsCreated()) ((int (*)(int, void*, void*, float))CShadowManager_SetSelectionValue)(ShadowManager_GetCurrent(), courseModelResourceStrings + kCourseStringsBackgroundShadowOffset,
                                   *(void**)((char*)(*(void**)(courseBytes + kCourseFallbackSurfaceOffset)) + 0x10), *(float*)((char*)(*(void**)(courseBytes + kCourseFallbackSurfaceOffset)) + 0x68));
}



// Load the mode-specific scene archive and optional balloon records, consuming independent streams.
// Borrows the two live path strings; all streams opened here are released here.
static inline void LoadCourseSceneArchiveAndBalloons(void* self, CourseLoadWorkspace& scratch, int& found, int& geometryVariant, char*& environmentPath, int& recordIndex) {
    // Phase 10: load the scene archive and optional party balloons. Only the
    // two long-lived path strings and cup/start candidate remain to destroy.
    if ((int)((CSharedDoc*)SharedDoc_GetCurrent())->partyGameMode == 2) { if (found == 0) {
        MWString_CopyConstruct(scratch + 0x1e8, scratch.frame.environmentPath);
        MWString_AppendCString(scratch + 0x1e8, courseModelResourceStrings + kCourseStringsCompressedSuffixOffset);
        void* compressedCoursePath = (void*)ScriptString_GetCharacterData(scratch + 0x1e8);
        if (CHashPath_FindPath(compressedCoursePath))
            MWString_AppendCString(scratch.frame.environmentPath, courseModelResourceStrings + kCourseStringsDecompressedSuffixOffset);
        ((NString*)(scratch + 0x1e8))->~NString();
    } }
    if ((unsigned char)SharedDoc_GetCourseId() != 7) {
        environmentPath = MWString_GetCString(scratch.frame.environmentPath);
        CourseRenderPacket_LoadSBINArchive(RenderPacket_GetInstance(), environmentPath, (unsigned char)geometryVariant, -1);
    }
    TransformRecord_CopyMatrix4x4(RenderPacket_GetInstance(), kIdentityMatrix4x4);
    CBalloonSystem_DestroyAllQueuedBalloons();
    if (0 == found) {
        String_AssignWithInlineFastPath(scratch.frame.environmentPath, scratch.frame.coursePath);
        int key2 = MWString_FindCString(scratch.frame.environmentPath, courseModelResourceStrings + kCourseStringsGeometryExtensionOffset, 0);

        String_ReplaceRangeCountAdapter(scratch.frame.environmentPath, key2, ((int)strlen(courseModelResourceStrings + kCourseStringsGeometryExtensionOffset)), courseModelResourceStrings + kCourseStringsBalloonExtensionOffset, ((int)strlen(courseModelResourceStrings + kCourseStringsBalloonExtensionOffset)));
        if ((int)((CSharedDoc*)SharedDoc_GetCurrent())->partyGameMode == 3) {
            char* tag2 = MWString_GetCString(scratch.frame.environmentPath);
            void* resource = (void*)Resource_OpenResolvedPath(Object_GetWord10(self), tag2, -1, 0, 0, 0);
            ResourceHolder_SetResourceAddress(scratch + 0x04, &resource);
            if (resource) {
                readData(resource, scratch + 0x1d8, 0x10);
                for (recordIndex = 0; recordIndex < *(int*)(scratch + 0x1e0); ++recordIndex) {
                    readData(resource, scratch + 0x1c8, 0x10);
                    *(Vec*)(scratch + 0x1bc) = *(Vec*)(scratch + 0x1c8);
                    CBalloonSystem_Spawn(scratch + 0x1bc, *(int*)(scratch + 0x1d4));
                }
                if (resource) Resource_Release(resource);
                resource = 0;
            }
            ResourceHandle_dtor(scratch + 0x04, -1);
        }
    }
}


// Load course geometry, regions, collisions, effects, environment, rival targets, and party-mode data.
// fileName is borrowed; self receives owned arrays/resources. Returns 0 for open
// or version failure after old resources have already been released; no rollback.
// A successful load returns 1. Stream reads assume valid complete course records.
// Serialized-section contract (sizes are FILE bytes, not runtime struct strides):
//   header 0x28 -> self+0x60: unused word, tag0x71, three mesh group counts,
//     camera/marker/bounds/bitmap/region counts (schema ModelResource801BD2B8.h).
//   cameraCount * 0x40: name[32], position XYZ[12], target XYZ[12], two words[8].
//   sceneMarkerCount: kind[1]+name[32]+position[12], plus text[64] iff kind!=0.
//   boundsCount * 0x59: kind[1]+tag[64]+minXYZ[12]+maxXYZ[12].
//   regionDefinitionCount: name[16]+pointCount[4]+unused[4], then pointCount*12.
//   one fallback mesh[0xAC], then sum(all three mesh group counts)*0xAC records;
//     CourseInputMeshReadView gives their offsets. Runtime mesh stride is 0xDC.
// All counts must be nonnegative, products/sums must fit the 32-bit allocations,
// and the stream must contain every section, including unselected mesh variants.
// The number of successfully loaded, unfiltered meshes must not exceed
// selectedMeshCount: meshWriteOffset advances only on successful resource load,
// with no capacity check. In particular geometryVariant0 allocates base count
// but adds no extra filtering here; the supplied file/resources must respect
// that capacity. bitmapEntryCount is later rebuilt from collected material names,
// not a further serialized mesh section. Do not size input from runtime strides.
// Fixed strings used by CString/sscanf require an in-field NUL and valid format;
// the loader adds none to file fields. Derived puppet vertex batches must contain
// multiples of three vertices and fit WPuppet_GetTriangleCount's allocation.
// No per-section byte budget/count cap/short-read check enforces these preconditions.
// Endianness: readData copies native Wii big-endian integers and binary32 floats;
// its own resource API; rival/balloon records are raw 0x10 headers followed by
// 0x34/0x10 records respectively, with the same trusted-count convention.
// Published-resource failure contract: arrays are stored on self immediately,
// puppets/triangles/effects are published as their phases finish. There is no
// completed/transaction bit and no catch that undoes those publications. Later
// exceptions escape, leaving partial new contents and already changed lighting,
// collision/global effect state; callers must not render/use self as a loaded
// course. WPolySoup_ReleaseResources is the normal owner cleanup boundary, but
// this loader does not prove it safe for every partially initialized array or
// unpublished local resource. Scratch is POD with manual cleanup calls: those
// calls only run when reached, not automatically on unwind. Do not promise leak-
// free recovery/retry for an exceptional late failure without auditing that state.
int CourseModel_LoadFromFile(void* self, const char* fileName) {
    // Phase 1: latch capabilities, discard previous contents, validate header,
    // and allocate the selected runtime arrays. Both failure returns are here.
    unsigned int collisionMeshCount;
    int regionPointIndex;
    int boundsCount;
    char* courseBytes = (char*)self;

    int recordIndex;
    CourseLoadWorkspace scratch;
    void* resourceManager = (void*)Pointer_GetFirstPointer(g_resourceManager);
    ((Header*)resourceManager)->query(0x1f, 2, scratch + 0x50);
    if (scratch.frame.resourceCapability != 0) AsModelResource801BD2B8(courseBytes)->resourceCapabilityLatched = 1; // Latch resource-manager capability query (0x1F, 2).
    WPolySoup_ReleaseResources(self);
    strncpy(courseBytes + 0x20, fileName, 0x3f);
    AsModelResource801BD2B8(courseBytes)->filenameTerminator = 0;
    int geometryVariant; geometryVariant = SharedDoc_GetSelectedCourseVariant();
#define stream (scratch.frame.mainStream)
    stream = (void*)Resource_OpenResolvedPath(Object_GetWord10(self), fileName, -1, 0, 0, 0);
    ResourceHolder_SetResourceAddress(scratch + 0x48, &scratch.frame.mainStream);
    if (stream == 0) {
        char* base = courseModelResourceStrings;
        char* msg = String_FormatTemporary(base + kCourseStringsOpenErrorFormatOffset, fileName);
        MessageBox_Report(0, msg, base + kCourseStringsOpenErrorTitleOffset, 0);
        ResourceHandle_dtor(scratch + 0x48, -1);
        return 0;
    }
    readData(stream, courseBytes + 0x60, 0x28);
    if (AsModelResource801BD2B8(courseBytes)->formatTag != kCourseFormatVersion) {
        char* base = courseModelResourceStrings;
        MessageBox_Report(0, base + kCourseStringsVersionErrorOffset, base + kCourseStringsVersionErrorTitleOffset, 0);
        ResourceHandle_dtor(scratch + 0x48, -1);
        return 0;
    }
    // Choose array capacity from geometry groups (independent of weather suffixes).
    // All file records are still consumed; incompatible groups are skipped below.
    switch ((unsigned char)geometryVariant) {
    case kBaseMeshesOnly: AsModelResource801BD2B8(courseBytes)->selectedMeshCount = AsModelResource801BD2B8(courseBytes)->baseMeshCount; break;
    case kExtraMeshGroup0: AsModelResource801BD2B8(courseBytes)->selectedMeshCount = AsModelResource801BD2B8(courseBytes)->baseMeshCount + AsModelResource801BD2B8(courseBytes)->extraMeshCount0; break;
    case kExtraMeshGroup1: AsModelResource801BD2B8(courseBytes)->selectedMeshCount = AsModelResource801BD2B8(courseBytes)->baseMeshCount + AsModelResource801BD2B8(courseBytes)->extraMeshCount1; break;
    default: AsModelResource801BD2B8(courseBytes)->selectedMeshCount = AsModelResource801BD2B8(courseBytes)->baseMeshCount + AsModelResource801BD2B8(courseBytes)->extraMeshCount0 + AsModelResource801BD2B8(courseBytes)->extraMeshCount1; break;
    }
    unsigned int meshCount; meshCount = *(int*)(courseBytes + kCourseSelectedMeshCountOffset);
    CourseMeshStorage* meshRecords = new CourseMeshStorage[meshCount];
    AsModelResource801BD2B8(courseBytes)->meshes = meshRecords;
    int markerCount; markerCount = AsModelResource801BD2B8(courseBytes)->sceneMarkerCount;
    CourseSceneMarkerStorage* sceneMarkers = new CourseSceneMarkerStorage[markerCount];
    AsModelResource801BD2B8(courseBytes)->sceneMarkers = sceneMarkers;
     boundsCount = AsModelResource801BD2B8(courseBytes)->boundsCount;
    CourseBoundsStorage* boundsRecords = new CourseBoundsStorage[boundsCount];
    AsModelResource801BD2B8(courseBytes)->bounds = boundsRecords;
    int cameraCount; cameraCount = AsModelResource801BD2B8(courseBytes)->cameraCount;
    CourseCameraStorage* cameraRecords = new CourseCameraStorage[cameraCount];
    AsModelResource801BD2B8(courseBytes)->cameras = cameraRecords;
    MainRecord* fallbackSurfaceStorage = new MainRecord;
    AsModelResource801BD2B8(courseBytes)->fallbackSurface = fallbackSurfaceStorage;
    // Phase 2: consume cameras, scene markers, bounds and polygon regions.
    // Read named camera positions and targets, then build their affine transforms.
    for (recordIndex = 0; recordIndex < AsModelResource801BD2B8(courseBytes)->cameraCount; ++recordIndex) {
        char* item = (char*)(AsModelResource801BD2B8(courseBytes)->cameras) + recordIndex * kCourseCameraRecordBytes;
        void* second;
        ReadCourseCameraRecord(item, scratch, stream, second);
    }
    // Initialize scene lighting; marker records may override direction and colors.
    s_sceneLighting[0] = 2;
    s_sceneLighting[4] = 0x00ffffff;
    s_sceneLighting[5] = 0x00808080;
    s_sceneLighting[6] = 0x00ffffff;
    for (recordIndex = 0; recordIndex < AsModelResource801BD2B8(courseBytes)->sceneMarkerCount; ++recordIndex) {
        char* item = (char*)(AsModelResource801BD2B8(courseBytes)->sceneMarkers) + recordIndex * kCourseMarkerRecordBytes;
        ReadCourseSceneMarker(item, scratch, stream);
    }
    void* golfDocForBrightness = (void*)GolfDoc_GetCurrent();
    s_sceneLighting[4] = (unsigned int)Color_ScaleRGB(s_sceneLighting[4], AsCGolfDoc(golfDocForBrightness)->bright);
    golfDocForBrightness = (void*)GolfDoc_GetCurrent();
    s_sceneLighting[5] = (unsigned int)Color_ScaleRGB(s_sceneLighting[5], AsCGolfDoc(golfDocForBrightness)->bright);
    golfDocForBrightness = (void*)GolfDoc_GetCurrent();
    s_sceneLighting[6] = (unsigned int)Color_ScaleRGB(s_sceneLighting[6], AsCGolfDoc(golfDocForBrightness)->bright);
    {

    // Load typed bounds and collect sound/effect regions from their tags.
    // Each pair of 12-byte reads is file XYZ -> scratch vector -> aligned runtime bounds.
    // Sound tags parse into a copied emitter; its oriented axes derive from these same min/max values.
    for (recordIndex = 0; recordIndex < AsModelResource801BD2B8(courseBytes)->boundsCount; ++recordIndex) {
        CourseBoundsReadView* item = (CourseBoundsReadView*)((char*)AsModelResource801BD2B8(courseBytes)->bounds + recordIndex * kCourseBoundsRecordBytes);
        ReadCourseBoundsRecord(item, scratch, stream, courseBytes);
    }
    }
    LoadCoursePropertyParameters(courseBytes, recordIndex);
    ReadCoursePolygonRegions(courseBytes, scratch, stream, recordIndex, regionPointIndex);
    if (2 == s_sceneLighting[0]) {

        *(WVector*)(1 + s_sceneLighting) *= -1.0f;
        (*(WVector*)(1 + s_sceneLighting)).Normalize();
    }
    // Phase 3: build the fallback surface, puppet, bounds and world triangles.
    // Transform order: uniform scale -> subtract scaled root-bone vector from
    // translation -> append Euler rotation (degrees converted to radians) ->
    // add file translation. +668 is no longer a translation once matrix-copied.
    FourVectorState_Reset(scratch + kScratchInputMesh);
    readData(stream, scratch + kScratchInputMesh, 0xac);
    CourseIntVector_InitEmpty(scratch + kScratchCollisionData);
    void* fallbackPuppet = (void*)ResourceManager_LoadByExtension(g_resourceManager, scratch.frame.inputMesh.modelName, 0, 0, 0);
    WMatrix_DefaultConstructNoOp(scratch.frame.modelMatrix);
    WVector_DefaultConstructNoOp(scratch + kScratchRotationRadiansX);
    WVector_DefaultConstructNoOp(scratch.frame.scaledRootBone);
    float halfTurnDegrees = 180.0f;
    float fallbackPiRadians = 3.14159274f;
    scratch.frame.rotationRadians[0] = fallbackPiRadians * scratch.frame.inputMesh.rotationDegrees[0] / halfTurnDegrees;
    scratch.frame.rotationRadians[1] = fallbackPiRadians * scratch.frame.inputMesh.rotationDegrees[1] / halfTurnDegrees;
    scratch.frame.rotationRadians[2] = fallbackPiRadians * scratch.frame.inputMesh.rotationDegrees[2] / halfTurnDegrees;
    Matrix_SetIdentityAffine(scratch.frame.modelMatrix);
    Matrix3x3_ScaleAssign(scratch.frame.modelMatrix, scratch.frame.inputMesh.uniformScale);
    WPuppet_GetPivot(scratch.frame.fallbackRootBone, fallbackPuppet, 0);
    __ml__FRC7WVectorf(scratch.frame.scaledFallbackRootBone, scratch.frame.fallbackRootBone, scratch.frame.inputMesh.uniformScale);
    *(WVector*)(scratch.frame.scaledRootBone) = *(WVector*)(scratch.frame.scaledFallbackRootBone);
    *(WVector*)(scratch.frame.modelTransform.translation) -= *(WVector*)(scratch.frame.scaledRootBone);
    Matrix_AppendEulerRotation(scratch.frame.modelMatrix, scratch + kScratchRotationRadiansX);
    *(WVector*)(scratch.frame.modelTransform.translation) += *(WVector*)(scratch + kScratchInputTranslation);
    Model_SetMatrixRecursive(fallbackPuppet, 0, scratch.frame.modelMatrix);
    WPuppet_UpdateBoneWorldMatrices(fallbackPuppet, g_view, 1, 1.0f);
    void* fallbackCollisionNode = (void*)PuppetMesh_CreatePartSamplesAndBounds(fallbackPuppet, 1);
    *(void**)(scratch + 0x44) = fallbackCollisionNode;
    CourseIntVector_Append(scratch + kScratchCollisionData, scratch + 0x44);
    AABB_Copy(scratch.frame.inputMesh.bounds, (char*)*(void**)(scratch + 0x44) + 0x3c);
    Matrix_CopyAffine(scratch + kScratchInputTranslation, scratch.frame.modelMatrix);
    *(unsigned char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) = *(unsigned char*)(scratch + kScratchInputMesh) & 7;
    *((unsigned char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + 1) = *(unsigned char*)(scratch + kScratchInputMesh) & 0xf8;
    AABB_Copy((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshBoundsOffset, scratch.frame.inputMesh.bounds);
    Matrix_CopyAffine((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshMatrixOffset, scratch + kScratchInputTranslation);
    __pl__FRC7WVectorRC7WVector(scratch + 0x15c, (char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshBoundsMaxOffset, (char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshBoundsOffset);
    __ml__FfRC7WVector(scratch + 0x168, scratch + 0x15c, 0.5f);
    *(WVector*)((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshCenterCopyOffset) = *(WVector*)(scratch + 0x168);
    __pl__FRC7WVectorRC7WVector(scratch + 0x144, (char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshBoundsMaxOffset, (char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshBoundsOffset);
    __ml__FfRC7WVector(scratch + 0x150, scratch + 0x144, 0.5f);
    *(WVector*)((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshCenterOffset) = *(WVector*)(scratch + 0x150);
    __mi__FRC7WVectorRC7WVector(scratch + 0x138, (char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshBoundsMaxOffset, (char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshBoundsOffset);
    float width = WVectorLen(*(WVector*)(scratch + 0x138));
    *(float*)((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshRadiusOffset) = (((0.5f * width)));

    *(float*)((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshOriginDistanceOffset) = ((*(WVector*)(scratch + kScratchInputTranslation)).Magnitude());
    strncpy((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshNameOffset, scratch.frame.inputMesh.modelName, 0x1f);
    *(void**)((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshPuppetOffset) = fallbackPuppet;
    Object_SetWord14((*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset)), 0x1000000);
    Model_SetMatrixRecursive((*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset)), 0, (char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshMatrixOffset);
    WPuppet_UpdateBound((*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset)), 1);
    ((Model*)*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset))->setup(s_sceneLighting, 0, 0);
    meshCount = WPuppet_GetTriangleCount((*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset)));
    CourseTriangleStorage* collisionTriangles = new CourseTriangleStorage[meshCount];
    *(void**)((char*)(AsModelResource801BD2B8(courseBytes)->fallbackSurface) + kMeshTrianglesOffset) = collisionTriangles;
    void* geometryNode = (void*)WPuppet_GetRootBone(*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset));
    Option_GetInstance();
    CourseObject_DefaultFalseQuery();
    *(int*)(scratch + 0x40) = 0;
    WPuppet_CreateMeshByBoneName(*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset), scratch + 0x40, 0);
    int meshGroupCount = IPetMesh_GetGroupCount(*(void**)(scratch + 0x40));
    // Expand mesh-group vertices into world-space triangles with normalized planes.
    // Loop hierarchy: group -> 16-byte vertex batch -> consecutive triangle -> corner.
    // A batch exposes vertex count at +4 and xyz-array pointer at +8; the count
    // must be a multiple of three. No index lookup occurs in this expansion.
    // Each 0x48-byte output record stores world xyz at +0/+C/+18, unit normal
    // n=(v2-v1)x(v0-v1) at +24, plane d=-dot(n,v0) at +30, flag 0 at +34.
    // Input vertices are model-local; PuppetBone_GetWorldMatrix supplies local->world.
    // Degenerate triangles retain the existing Normalize behavior (no extra guard).
    // triangleOffset is bytes into runtime output; batchIndex/firstCornerIndex/cornerIndex are element indices.
    unsigned int triangleOffset = 0;
    for (recordIndex = 0; recordIndex < (unsigned short)meshGroupCount; ++recordIndex) {
        CourseMeshGroupView* group = (CourseMeshGroupView*)IPetMesh_GetGroupByIndex(*(void**)(scratch + 0x40), (unsigned short)recordIndex);
        for (unsigned int batchIndex = 0; batchIndex < group->batchCount; ++batchIndex) {
            for (unsigned int firstCornerIndex = 0; firstCornerIndex < CourseVertexBatchAt(group, batchIndex * sizeof(CourseVertexBatchView))->vertexCount; firstCornerIndex += 3) {
                for (int cornerIndex = 0; 3 > cornerIndex; ++cornerIndex) {
                    memcpy(&CourseTriangleAt(AsModelResource801BD2B8(courseBytes)->fallbackSurface, triangleOffset)->corners[cornerIndex],
                        (char*)CourseVertexBatchAt(group, batchIndex * sizeof(CourseVertexBatchView))->vertices + (cornerIndex + firstCornerIndex) * 12, 12);
                    void* transform = (void*)PuppetBone_GetWorldMatrix(geometryNode);
                    Matrix_TransformPoint(scratch + 0x12c, &CourseTriangleAt(AsModelResource801BD2B8(courseBytes)->fallbackSurface, triangleOffset)->corners[cornerIndex], transform);
                    *(WVector*)(&CourseTriangleAt(AsModelResource801BD2B8(courseBytes)->fallbackSurface, triangleOffset)->corners[cornerIndex]) = *(WVector*)(scratch + 0x12c);
                }
                // Retain this direct local view: constructing it through the helper changes register lifetimes.
                CourseTriangleView* triangle = (CourseTriangleView*)((char*)
                    ((CourseMeshRecord*)AsModelResource801BD2B8(courseBytes)->fallbackSurface)->triangles + triangleOffset);
                FinalizeCourseTriangle(courseBytes, scratch, triangleOffset, triangle);
                triangleOffset += sizeof(CourseTriangleView);
            }
        }
    }
    void* windTagBone = (void*)WPuppet_FindNamedBoneEntryForward(*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset), courseModelResourceStrings + kCourseStringsCenterBoneOffset);
    if (windTagBone) {
        void* text = *(void**)(*(char**)((char*)windTagBone + 4) + 0x28);
        if (text) {
            BorrowedNameRef_Set(scratch + 0x3c, text);
            String_ScanTokensAfterSubstring(scratch + 0x3c, courseModelResourceStrings + kCourseStringsWindTagOffset, courseModelResourceStrings + kCourseStringsFloatFormatOffset, (char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + 8);
        }
    }
    if (WPuppet_FindNamedBoneEntryForward(*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshPuppetOffset), courseModelResourceStrings + kCourseStringsNpcTagOffset))
        CourseModelPointerVector_Append(courseBytes + 0x48e8, kCourseNpcFallbackMarker);
    {

    for (unsigned int child = 0; child < *(unsigned int*)(*(char**)((char*)*(void**)(courseBytes + kCourseFallbackSurfaceOffset) + 0x10) + 0x140); ++child) {
        char* entry = *(char**)(*(char**)((char*)*(void**)(courseBytes + kCourseFallbackSurfaceOffset) + 0x10) + 0x13c) + (child * 8);
        char* item = *(char**)(((4 + entry)));
        if (CString_FindSubstringForwarder(item + 0x2c, courseModelResourceStrings + kCourseStringsSoundTagOffset)) {
            void* text = *(void**)(*(char**)((((entry + 4)))) + 0x28);
            CourseSoundEmitter_DefaultConstructNoOp(scratch + 0x558);
            *(float*)(scratch + 0x57c) = (-1.0f);
            *(float*)(scratch + 0x580) = (60.0f);
            *(float*)(scratch + 0x578) = (0.0f);
            sscanf(text, courseModelResourceStrings + kCourseStringsSoundFormatOffset, scratch + 0x558, scratch + 0x580, scratch + 0x57c);
            Matrix_MoveLastVectorToFront(scratch + 0x584, (void*)PuppetBone_GetWorldMatrix(*(void**)entry));
            __mi__FRC7WVectorRC7WVector(scratch + 0xe4, *(char**)(4 + entry) + 0xc, *(void**)(4 + entry));
            __ml__FfRC7WVector(scratch + 0xf0, scratch + 0xe4, 0.5f);
            AffineVectors_ScaleTrailingThree(scratch + 0x584, scratch + 0xf0);
            void* transform = (void*)PuppetBone_GetWorldMatrix(*(void**)entry);
            Matrix_TransformPoint(scratch + 0xd8, *(char**)(4 + entry) + 0x18, transform);
            *(WVector*)(scratch + 0x584) = *(WVector*)(scratch + 0xd8);
            CourseNodeList_PrependRecord(courseBytes + 0x4900, scratch + 0x558);
        }
    }
    }
    ((int (*)(void*, int, void*, float, float, int, void*, void*, int, void*))ResourcePreloadQueue_AddRequest)(SceneManager_GetCurrent(), 0, *(void**)((char*)*(void**)(courseBytes + kCourseFallbackSurfaceOffset) + 0x10), 1.0f, 0.0f, 0, (char*)(*(void**)(courseBytes + kCourseFallbackSurfaceOffset)) + 0x44, (void*)0, 3, (void*)0);
    // Phase 4: choose resource-name policy, then load variant-filtered meshes.
    // Weather policy is separate from selected geometry variant: 1 adds cloud,
    // 2 adds rain, all other weather values add nothing; append night afterward.
    // On a missing weather-specific resource, retry the unmodified file name.
    int fileMeshCount = AsModelResource801BD2B8(courseBytes)->extraMeshCount0 + (AsModelResource801BD2B8(courseBytes)->extraMeshCount1 + *(int*)(courseBytes + kCourseBaseMeshCountOffset));
    collisionMeshCount = 0;
    Storage_ClearThreeWords(scratch.frame.weatherSuffix);
    Storage_ClearThreeWords(scratch.frame.modelPath);
    int weatherKind = (unsigned char)CourseResource_GetSelectedVariant();
    SelectCourseMeshSuffix(scratch, weatherKind);
    float one = 1.0f; float expandedBoundsHeight = 10.0f; float largePreloadRadius = 128.0f; float half = 0.5f;
    float piRadians = 3.14159274f;
    int meshWriteOffset = 0;
    for (recordIndex = 0; recordIndex < fileMeshCount; ++recordIndex) {
        readData(stream, scratch + kScratchInputMesh, 0xac);
        MWString_AssignCString(scratch.frame.modelPath, scratch.frame.inputMesh.modelName);
        void* matchesNameFilter = (void*)CourseObjectName_MatchesFilterPattern(self, scratch.frame.inputMesh.modelName);
        int meshVariantGroup = scratch.frame.inputMesh.variant;
        if (meshVariantGroup == kExtraMeshGroup0 && (unsigned char)geometryVariant == kExtraMeshGroup1) continue;
        if (kExtraMeshGroup1 == meshVariantGroup && (unsigned char)geometryVariant == kExtraMeshGroup0) continue;
        // Preserve the original four-name rule exactly. These ice models are
        // passed load option 1; other names get 0. The callee's option semantics
        // are not established here, so this is an exception list, not a claim
        // about visibility/collision/animation. Both suffix and fallback loads
        // use the same option. Keep the comparison/short-circuit order.
        unsigned int loadOption = 1;
        loadOption = (String_CompareCString(scratch.frame.inputMesh.modelName, courseModelResourceStrings + kCourseStringsIcePeople1Offset) ||
            String_CompareCString(scratch.frame.inputMesh.modelName, courseModelResourceStrings + kCourseStringsIcePeople2Offset) ||
            String_CompareCString(scratch.frame.inputMesh.modelName, courseModelResourceStrings + kCourseStringsIcePeople3Offset) ||
            String_CompareCString(scratch.frame.inputMesh.modelName, courseModelResourceStrings + kCourseStringsIceUnderseaOffset)) ? 0 : loadOption;

        CourseModelString_InsertString(scratch.frame.modelPath, (MWString_FindCString(scratch.frame.modelPath, courseModelResourceStrings + kCourseStringsPetExtensionOffset, 0)), scratch.frame.weatherSuffix);
        unsigned char raw = *(unsigned char*)(scratch + kScratchInputMesh);
        int active = (((7 & raw))) != 0;
        char* resolved = MWString_GetCString(scratch.frame.modelPath);
        void* resource = (void*)ResourceManager_LoadByExtension(g_resourceManager, resolved, loadOption, active, 0);
        if (0 == resource) {
            MWString_AssignCString(scratch.frame.modelPath, scratch.frame.inputMesh.modelName);
            active = (*(unsigned char*)(scratch + kScratchInputMesh) & 7) != 0;
            resolved = MWString_GetCString(scratch.frame.modelPath);
            resource = (void*)ResourceManager_LoadByExtension(g_resourceManager, resolved, loadOption, active, 0);
        }
        if (resource) {
            float halfTurnDegrees = 180.0f;
            scratch.frame.rotationRadians[0] = piRadians * scratch.frame.inputMesh.rotationDegrees[0] / halfTurnDegrees;
            scratch.frame.rotationRadians[1] = piRadians * scratch.frame.inputMesh.rotationDegrees[1] / halfTurnDegrees;
            scratch.frame.rotationRadians[2] = piRadians * scratch.frame.inputMesh.rotationDegrees[2] / halfTurnDegrees;
            Matrix_SetIdentityAffine(scratch.frame.modelMatrix);
            Matrix3x3_ScaleAssign(scratch.frame.modelMatrix, scratch.frame.inputMesh.uniformScale);
            WPuppet_GetPivot(scratch + 0xc0, resource, 0);
            __ml__FRC7WVectorf(scratch + 0xcc, scratch + 0xc0, scratch.frame.inputMesh.uniformScale);
            *(WVector*)(scratch.frame.scaledRootBone) = *(WVector*)(scratch + 0xcc);
            *(WVector*)(scratch.frame.modelTransform.translation) -= *(WVector*)(scratch.frame.scaledRootBone);
            Matrix_AppendEulerRotation(scratch.frame.modelMatrix, scratch + kScratchRotationRadiansX);
            *(WVector*)(scratch.frame.modelTransform.translation) += *(WVector*)(scratch + kScratchInputTranslation);
            Model_SetMatrixRecursive(resource, 0, scratch.frame.modelMatrix);
            WPuppet_UpdateBoneWorldMatrices(resource, g_view, 1, 1.0f);
            void* node = (void*)PuppetMesh_CreatePartSamplesAndBounds(resource, 0);
            CourseIntVector_Append(scratch + kScratchCollisionData, &node);
            AABB_Copy(scratch.frame.inputMesh.bounds, ((CourseCollisionNodeView*)node)->bounds);
            Matrix_CopyAffine(scratch + kScratchInputTranslation, scratch.frame.modelMatrix);

            if ((((CourseCollisionNodeView*)node)->bounds[4] - ((CourseCollisionNodeView*)node)->bounds[1]) < expandedBoundsHeight) {
                AABB_Copy(scratch + kScratchExpandedBounds, ((CourseCollisionNodeView*)node)->bounds);
            } else {
                CourseBounds_CopySixWords(scratch + 0x2f8, ((CourseCollisionNodeView*)node)->bounds);
                *(float*)(scratch + 0x308) = one + *(float*)(scratch + 0x2fc);
                WPuppet_ComputeMeshBounds(scratch + 0x2e0, resource, scratch + 0x2f8);
                AABB_Copy(scratch + kScratchExpandedBounds, scratch + 0x2e0);
                *(float*)(scratch + 0x660) = expandedBoundsHeight + *(float*)(scratch + 0x654);
            }
            // Commit one successfully loaded mesh. Rejected variant records and
            // failed loads consume input but never advance meshWriteOffset.
            char* runtimeMeshBytes = (char*)AsModelResource801BD2B8(courseBytes)->meshes + meshWriteOffset;
            *(unsigned char*)runtimeMeshBytes = *(unsigned char*)(scratch + kScratchInputMesh) & 7;
            meshWriteOffset += kCourseModelRecordBytes;
            runtimeMeshBytes[1] = *(unsigned char*)(scratch + kScratchInputMesh) & 0xf8;
            AABB_Copy(runtimeMeshBytes + kMeshBoundsOffset, scratch.frame.inputMesh.bounds);
            AABB_Copy(runtimeMeshBytes + kMeshExpandedBoundsOffset, scratch + kScratchExpandedBounds);
            Matrix_CopyAffine(runtimeMeshBytes + kMeshMatrixOffset, scratch + kScratchInputTranslation);
            __pl__FRC7WVectorRC7WVector(scratch + 0xa8, runtimeMeshBytes + kMeshBoundsMaxOffset, runtimeMeshBytes + kMeshBoundsOffset);
            __ml__FfRC7WVector(scratch + 0xb4, scratch + 0xa8, 0.5f);
            *(WVector*)(runtimeMeshBytes + kMeshCenterCopyOffset) = *(WVector*)(scratch + 0xb4);
            __pl__FRC7WVectorRC7WVector(scratch + 0x90, runtimeMeshBytes + kMeshBoundsMaxOffset, runtimeMeshBytes + kMeshBoundsOffset);
            __ml__FfRC7WVector(scratch + 0x9c, scratch + 0x90, 0.5f);
            *(WVector*)(runtimeMeshBytes + kMeshCenterOffset) = *(WVector*)(scratch + 0x9c);
            __mi__FRC7WVectorRC7WVector(scratch + 0x84, runtimeMeshBytes + kMeshBoundsMaxOffset, runtimeMeshBytes + kMeshBoundsOffset);

            *(float*)(runtimeMeshBytes + kMeshRadiusOffset) = half * (WVectorLen(*(WVector*)(scratch + 0x84)));
            *(float*)(runtimeMeshBytes + kMeshOriginDistanceOffset) = (*(WVector*)(scratch + kScratchInputTranslation)).Magnitude();
            strncpy(runtimeMeshBytes + kMeshNameOffset, MWString_GetCString(scratch.frame.modelPath), 0x1f);
            strncpy(runtimeMeshBytes + kMeshTagOffset, scratch.frame.inputMesh.tag, 0x1f);
            runtimeMeshBytes[0x63] = 0; runtimeMeshBytes[0xd9] = 0;
            *(void**)(runtimeMeshBytes + kMeshPuppetOffset) = resource;
            if (*(unsigned char*)runtimeMeshBytes) collisionMeshCount = (short)(1 + collisionMeshCount);
            void* object = *(void**)(runtimeMeshBytes + kMeshPuppetOffset);
            if (0 == object) {
                if (!NameRegistry_FindByName((char*)*(void**)(courseBytes + kCourseEntryContainerOffset) + 0x60, runtimeMeshBytes + kMeshNameOffset)) {
                    unsigned int len = strlen(runtimeMeshBytes + kMeshNameOffset);
                    char* copy = (char*)__nwa__FUl((len + 1));
                    strcpy(copy, runtimeMeshBytes + kMeshNameOffset);
                    CourseNameRegistry_Insert((char*)*(void**)(courseBytes + kCourseEntryContainerOffset) + 0x60, &copy, copy, 0);
                }
            } else {
                Model_SetMatrixRecursive((*(void**)(runtimeMeshBytes + kMeshPuppetOffset)), 0, runtimeMeshBytes + kMeshMatrixOffset);
                WPuppet_UpdateBound((*(void**)(runtimeMeshBytes + kMeshPuppetOffset)), 1);
                ((Model*)*(void**)(runtimeMeshBytes + kMeshPuppetOffset))->setup(s_sceneLighting, 0, 0);
                CourseData_ApplyModelNodeTags(self, runtimeMeshBytes);
                int flags = 0;
                if (*(unsigned char*)runtimeMeshBytes) flags |= kPreloadCollision;
                if ((runtimeMeshBytes[1] & 0x10) || *(float*)((char*)CourseObject_GetEmbeddedData((*(void**)(runtimeMeshBytes + kMeshPuppetOffset))) + 0x0c) > largePreloadRadius) flags |= kPreloadLargeOrFlag10;
                if (runtimeMeshBytes[1] & 8) flags |= kPreloadSurfaceFlag08;
                float stateDistance = matchesNameFilter ? 640.0f : 0.0f;
                int category = 3;
                if (*(unsigned char*)runtimeMeshBytes == 0) category = 2;
                ((int (*)(void*, int, void*, float, float, int, void*, void*, int, void*))ResourcePreloadQueue_AddRequest)(SceneManager_GetCurrent(), category, (*(void**)(runtimeMeshBytes + kMeshPuppetOffset)), *(float*)(runtimeMeshBytes + kMeshOriginDistanceOffset),
                            stateDistance, 0, runtimeMeshBytes + kMeshNameOffset, runtimeMeshBytes + kMeshMatrixOffset, flags, runtimeMeshBytes + kMeshExpandedBoundsOffset);
            }
        }
    }
    // Phase 5: collect bitmap names, load resources, then bind triangle materials.
    CourseModelAllocator_EmptyCleanup(scratch + 0x34);
    CourseMaterialMap_InitEmpty(scratch + kScratchBitmapNames);
    void* token = (void*)CourseModelTreeIterator_GetNode(scratch + kScratchCollisionData);
    CourseModelTreeIterator_Assign(scratch + 0x34, &token);
    while ((token = (void*)CourseModelPointerVector_End(scratch + kScratchCollisionData), CourseModelPointerIterator_NotEqual(scratch + 0x34, &token))) {
        void* entry = *(void**)CourseModelTreeIterator_GetNodeAdapter(scratch + 0x34);
        void* cursor = (void*)CourseModelObjectIterator_GetFirst((char*)entry + 4);
        while (cursor) {
            int id = ((CourseMaterialCursorView*)cursor)->resourceId;
            *(int*)(scratch + 0x30) = id;
            void* resource = (void*)ObjectList_FindById(g_resourceManager, id);
            void* nameSlot = (void*)IntStringTree_GetOrInsertValue(scratch + kScratchBitmapNames, scratch + 0x30);
            MWString_AssignCString(nameSlot, &((CourseBitmapResourceView*)resource)->name);
            cursor = (void*)CourseModelObjectIterator_GetNext((char*)entry + 4);
        }
        CourseModelPointerIterator_PostIncrement(scratch + 0x34, 0);
    }
    int itemCount = CourseModelPointerIterator_Get(scratch + kScratchBitmapNames);
    AsModelResource801BD2B8(courseBytes)->bitmapEntryCount = itemCount;
    CourseResourcePairStorage* bitmapRecords = new CourseResourcePairStorage[itemCount];
    AsModelResource801BD2B8(courseBytes)->bitmapEntries = bitmapRecords;
    CourseModelIterator_EmptyCleanup(scratch + 0x2c);
    CourseModelTreeIterator_FromFirstNode(scratch + 0x0c, scratch + kScratchBitmapNames);
    CourseModelTreeIterator_Copy(scratch + 0x2c, scratch + 0x0c);
    unsigned int rowOffset = 0;
    for (recordIndex = 0; recordIndex < AsModelResource801BD2B8(courseBytes)->bitmapEntryCount; ) {
        memset(((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + rowOffset), 0, 0x20);
        void* name = (void*)CourseModelTreeIterator_GetPayload(scratch + 0x2c);
        char* text = (char*)MWString_GetCString((char*)name + 4);
        strncpy(((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + rowOffset), text, 0x1f);
        void* modelObj = (void*)ResourceManager_LoadBitmap(Object_GetWord10(self), ((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + rowOffset), 0, 0, 0);
        *(void**)(((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + rowOffset) + 0x20) = modelObj;
        *(void**)(((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + rowOffset) + 0x24) = (void*)CourseData_FindEntryByName(self, ((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + rowOffset));
        *(int*)CourseMaterialMap_GetOrInsertValue(courseBytes + 0xac, ((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + rowOffset) + 0x20) = recordIndex;

        recordIndex = (1 + recordIndex);
        rowOffset += 0x28;
        CourseModelTreeIterator_PostIncrement(scratch + 0x08, scratch + 0x2c, 0);
    }
    triangleOffset = 0;
    for (recordIndex = 0; recordIndex < (unsigned short)meshGroupCount; ++recordIndex) {
        char* tree = (char*)IPetMesh_GetGroupByIndex(*(void**)(scratch + 0x40), (unsigned short)recordIndex);
        for (unsigned int k = 0; k < *(unsigned int*)tree; ++k) {
            for (unsigned int m = 0; m < *(unsigned int*)(*(char**)(4 + tree) + k * 0x10 + 4); m += 3) {
                int map = *(int*)CourseMaterialMap_GetOrInsertValue(courseBytes + 0xac, *(char**)((tree + 4)) + k * 0x10);
                *(int*)((char*)*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshTrianglesOffset) + triangleOffset + 0x3c) = *(int*)((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + map * 0x28 + 0x24);
                map = *(int*)CourseMaterialMap_GetOrInsertValue(courseBytes + 0xac, *(char**)((tree + 4)) + k * 0x10);
                *(int*)((char*)*(void**)((char*)AsModelResource801BD2B8(courseBytes)->fallbackSurface + kMeshTrianglesOffset) + triangleOffset + 0x38) = *(int*)((char*)AsModelResource801BD2B8(courseBytes)->bitmapEntries + map * 0x28 + 0x20);
                triangleOffset += 0x48;
            }
        }
    }
    // Triangle positions/materials now persist in self. Release temporary
    // collision objects and the borrowed mesh-group handle before sorting rows.
    token = (void*)CourseModelTreeIterator_GetNode(scratch + kScratchCollisionData);
    CourseModelTreeIterator_Assign(scratch + 0x34, &token);
    while ((token = (void*)CourseModelPointerVector_End(scratch + kScratchCollisionData), CourseModelPointerIterator_NotEqual(scratch + 0x34, &token))) {
        void* node = (void*)CourseModelTreeIterator_GetNodeAdapter(scratch + 0x34);
        CourseObjectList_Dtor(*(void**)node, 1);
        CourseModelPointerIterator_PostIncrement(scratch + 0x34, 0);
    }
    CourseModelByteBuffer_ClearSize(scratch + kScratchCollisionData);
    CourseObject_ReleaseReference(*(void**)(scratch + 0x40));
    if (*(int*)(courseBytes + kCourseSelectedMeshCountOffset) > 1) qsort(AsModelResource801BD2B8(courseBytes)->meshes, *(int*)(courseBytes + kCourseSelectedMeshCountOffset), 0xdc, CourseMesh_CompareNameFlagsAndKind);
    SetupCourseEffectsAndShadows(self, courseBytes, scratch, collisionMeshCount, recordIndex);
    ReleaseMainCourseInput(scratch, stream);
    // Phase 7: independent environment stream; apply render states and fog.
    // Its raw buffer and parsed resource both die before the stream holder.
    void* golfDocForResourceName = (void*)GolfDoc_GetCurrent();

    void* nameIndex = (void*)String_FormatTemporary(courseModelResourceStrings + kCourseStringsCourseFileFormatOffset, SharedDoc_GetCourseResourceName(), ((unsigned char)CGolfDoc_GetCurrentHoleNumber(golfDocForResourceName)));
    MWString_ConstructCString(scratch.frame.coursePath, nameIndex);
    Storage_ClearThreeWords(scratch.frame.environmentPath);
    int found = strstr(fileName, courseModelResourceStrings + kCourseStringsRankMarkerOffset) != 0;
    CourseResource_BuildVariantName(scratch.frame.environmentPath, found, 0);
    MWString_AppendCString(scratch.frame.environmentPath, courseModelResourceStrings + kCourseStringsEnvironmentExtensionOffset);
    char* environmentPath = MWString_GetCString(scratch.frame.environmentPath);
    void* endStream = (void*)Resource_OpenResolvedPath(Object_GetWord10(self), environmentPath, -1, 0, 0, 0);
    ResourceHolder_SetResourceAddress(scratch + 0x20, &endStream);
    ApplyCourseEnvironment(self, courseBytes, scratch, endStream, recordIndex);
    // Phase 8: alternate environment stream; copy its first light description
    // to the render packet, then release parsed resource, records and byte buffer.
    CourseResource_BuildVariantName(scratch.frame.environmentPath, found, 1);
    MWString_AppendCString(scratch.frame.environmentPath, courseModelResourceStrings + kCourseStringsEnvironmentExtensionOffset);
    environmentPath = MWString_GetCString(scratch.frame.environmentPath);
    endStream = (void*)Resource_OpenResolvedPath(Object_GetWord10(self), environmentPath, -1, 0, 0, 0);
    ResourceHolder_SetResourceAddress(scratch + 0x10, &endStream);
    if (endStream) {
        int size = (int)CourseFile_GetByteLength(endStream);
        CourseModelByteBuffer_Init(scratch + 0x214);
        CourseModelByteBuffer_ResizeZeroFilled(scratch + 0x214, size);
        void* buffer = (void*)CourseModelByteBuffer_GetElement(scratch + 0x214, 0);
        readData(endStream, buffer, size);
        void* parsed = 0;
        int lightCount;
        CopyCourseAlternateEnvironmentLight(scratch, endStream, size, parsed, lightCount);
    }
    ResourceHandle_dtor(scratch + 0x10, -1);
    // Phase 9: load rival target points, then append cup/start candidates.
    // AddCandidate copies each temporary name/position; destroy it after copying.
    String_AssignWithInlineFastPath(scratch.frame.environmentPath, scratch.frame.coursePath);
    unsigned int key = MWString_FindCString(scratch.frame.environmentPath, courseModelResourceStrings + kCourseStringsGeometryExtensionOffset, 0);
    void* rivalManager = (void*)CRival_GetInstance();
    int records = CRival_GetShotPositionThinker(rivalManager);
    CComThinkShotPos_ClearCachedData(records); // r3 = CRival_GetShotPositionThinker's result at 0x801BF324-0x801BF32C (CComThinkShotPos_ClearCachedData uses r3 as its object)
    int suffixSize = strlen(courseModelResourceStrings + kCourseStringsRivalExtensionOffset);

    String_ReplaceRangeCountAdapter(scratch.frame.environmentPath, key, (strlen(courseModelResourceStrings + kCourseStringsGeometryExtensionOffset)), courseModelResourceStrings + kCourseStringsRivalExtensionOffset, suffixSize);
    char* tag = MWString_GetCString(scratch.frame.environmentPath);
    void* resource = Resource_OpenResolvedPath(Object_GetWord10(self), tag, -1, 0, 0, 0);
    ResourceHolder_SetResourceAddress(scratch + 0x00, &resource);
    ReadCourseRivalCandidates(resource, records, scratch, recordIndex);
    AppendCourseEndpointCandidates(scratch, records);
    CourseResource_BuildVariantName(scratch.frame.environmentPath, found, 0);
    LoadCourseSceneArchiveAndBalloons(self, scratch, found, geometryVariant, environmentPath, recordIndex);
    dtor_801C315C(scratch + 0x378, -1);
    ((NString*)(scratch.frame.environmentPath))->~NString();
    ((NString*)(scratch.frame.coursePath))->~NString();
#undef stream
    return 1;
}

#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset
} // extern "C"
