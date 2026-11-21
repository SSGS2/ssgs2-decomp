extern "C" {
extern unsigned char s_quadTreeDtorRegistration[];
extern unsigned char s_quadTreeStorage[];
}
#include "classes/CourseData801C.h"
#include "fp_fused.h"
#include "classes/WPuppet.h"
#include "classes/CGolfDoc.h"
#include "classes/CQuadTree.h"
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCQuadTree(p) ((CQuadTree*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "sdk_protos.h"
#include "game/types.h"
#include "game/golfball.h"
#include "classes/CWind.h"
#include "game/wvector.h"
#include "game/exception.h"


// Course collision and projected-patch implementation. Coordinates are world-space, Y up.
// Mod lookup: simulation cadence -> StepBallSimulation timing map; special-shot
// landing -> HandleBallGroundBounce flag/history map; ground decals/projected
// textures -> BuildProjectedPatchMesh and CourseSurface/CourseTriangle views.
// Physics consumers/caller: golf_rule_round_flow_and_shot_sim.cpp precomputes shot history at
// kGolfSimulationStepSeconds; guide_line_trajectory.cpp samples that history for guide lines.
// Constant/data companions are the named CourseEffects/QuadTree globals declared
// here; shared literals have multiple consumers, so a later mod should introduce
// a dedicated setting instead of retuning a shared 30/60/0.5 indiscriminately.
// the spatial grid indexes X/Z relative to gridOrigin in 20-unit cells. Distances retain
// game-world units; this unit does not establish a real-world conversion. dt/delta are
// caller simulation-time values except the clearance argument noted below; many thresholds use a fixed 1/30 tick.
// Triangle collision planes are [normal.xyz, constant], with n dot position + constant = 0;
// projected-patch clipping planes instead store the threshold for n dot position >= threshold.
// Ground-triangle IDs index groundSurface->triangles; object-triangle IDs are local to
// a surface. Candidate arrays are mutable scratch: each query replaces their contents.
// Pointer-returning material/cache queries borrow course/global storage, never ownership.
// Cast-call macros below preserve legacy hidden struct-return argument order. They are
// ABI adapters; ordinary vector expressions can change CodeWarrior temporary lifetimes.
// CGolfBall::m_contactMaterialIndex stores the contact material index: triangle
// materialIndex is assigned to it before material lookup.
// Binary views for course records that are still opaque in the shared headers.
struct CourseTriangle {
    Vec3f vertices[3];
    Vec3f normal;
    float planeConstant;
    unsigned char twoSided;
    char pad_35[3];
    unsigned int materialFlags;
    int materialIndex;
    unsigned char hitFeatures;
    char pad_41[3];
    void* boneTransform;
};
// Bucket triangle IDs share one allocation across all buckets; bucket zero retains its
// base even when empty. Other triangleIds are slices and must not be freed individually.
struct CourseTriangleBucket {
    int* triangleIds; int reserved04; short count; unsigned char objectCount; char pad_0B[5];
};
struct CourseSurface {
    unsigned char motionType; char pad_01[3];
    unsigned int flags;
    float windRotationScale;
    int reserved0C;
    WPuppet* puppet;
    float transformAxes[9];
    Vec3f position;
    char name[32];
    Vec3f boundsMin, boundsMax;
    char pad_7C[0x28];
    Vec3f partitionCenter;
    CourseTriangle* triangles;
    CourseTriangleBucket* buckets;
    unsigned char singleBucket;
    char pad_B9;
    char motionTags[34];
};
struct CourseMaterial {
    float restitution;
    float friction;
    float lowerLiePercent[3];
    float upperLiePercent[3];
    unsigned int flags;
    unsigned char groundType;
    char collisionName[1];
};
struct GroundRegion { Vec3f boundsMin, boundsMax; int lie; };
struct HolePositionRecord { Vec3f teePosition; Vec3f cupPosition; char pad_18[8]; };
struct CameraTransformView { Vec3f axes[3]; Vec3f position; };
struct BallGroundTriangleView { char pad_00[0x28]; int groundTriangleIndex; };
// Supplemental CQuadTree coordinates currently represented as padding in the public layout.
struct QuadTreeGeometryState {
    char pad_00[0x2084]; Vec3f recoveryPosition;
    char pad_2090[0x20B4-0x2090]; Vec3f groundOverridePosition;
    char pad_20C0[0x20D4-0x20C0]; int groundRegionIndex;
    char pad_20D8[0x214C-0x20D8]; Vec3f launchBoundsMin, launchBoundsMax;
    unsigned char launchBoundsExit; char pad_2165[3]; float cupRadius, cupFloorY;
    int cupContactCounter; Vec3f cupSweepParameters;
    char pad_2180[0x21D0-0x2180]; Vec3f lastHitPosition, lastHitDirection;
};
// Collision rules read the shared-document weather byte at +0x9C6.
struct SharedCollisionRules { char pad_00[0x9C6]; unsigned char weatherCode; };
struct PlayerGroundParameters { char pad_00[0x1C]; Vec3f position; char pad_28[0xAE - 0x28]; unsigned char lieSlot; };

struct SharedStringReference { char* vtable; unsigned int strongCount, weakCount; char* buffer; };
struct MeshReference { unsigned int referenceCount; char* vtable; };
struct MeshPart { unsigned int materialKey, vertexCount; char* vertices; unsigned int reserved; };
struct MeshGroup { unsigned int partCount; MeshPart* parts; };
struct BoneNamePair { char* bone; char* nameRecord; };
struct CourseNamedEntry { char pad_00[20]; char* positions; };
struct MaterialKeyPair { int key, materialIndex; };
struct CourseMaterialSource { char pad_00[0x20]; unsigned int flags, materialIndex; };
struct CourseMaterialTreeStorage { char pad_00[0xAC]; unsigned int count; char* root; char* first; };
struct CourseMaterialTable { char pad_00[0x8C]; CourseMaterialSource* entries; };
// Material-key tree node: two children, tagged parent word, then the key/index pair.
struct CourseMaterialKeyNode { char* left; char* right; unsigned int parentAndColor; int key; int materialIndex; };
// Grid cells likewise slice one shared triangle-ID allocation (base in cell zero).
// Each objectIds array is separate storage. +0x0C caches whether triangle IDs are unique;
// QueryTrianglesAtPoint sets it after checking duplicates, not after sorting the list.
struct GridCellView { int* triangleIds; short* objectIds; short triangleCount; unsigned char objectCount; char pad_0B[5]; };
typedef char MeshPart_size[sizeof(MeshPart) == 16 ? 1 : -1];
typedef char GridCellView_size[sizeof(GridCellView) == 16 ? 1 : -1];

struct CourseMotionPath { char pad_00[0x34]; Vec3f endpoints[2]; float pauseSeconds, travelSeconds; };
struct BallCollisionHistory {
    Vec3f position, velocity;
    char pad_18[8]; char soundName[64];
    unsigned char eventType, collisionType;
    char pad_62[2]; Vec3f normal;
    // +70 stores ball motion state, copied from CGolfBall::m_state
    // by the guide-line sampler (guide_line_trajectory.cpp) and simulation writers (golf_rule_round_flow_and_shot_sim.cpp/com_shot_cpu_rival_ai.cpp).
    int motionState;
};
struct RenderCameraTransforms { char pad_00[0x20]; float view[16]; float projection[16]; };
typedef char CourseTriangle_size[sizeof(CourseTriangle) == 0x48 ? 1 : -1];
typedef char CourseSurface_size[sizeof(CourseSurface) == 0xDC ? 1 : -1];
typedef char CourseTriangleBucket_size[sizeof(CourseTriangleBucket) == 16 ? 1 : -1];
typedef char GroundRegion_size[sizeof(GroundRegion) == 0x1C ? 1 : -1];
typedef char BallCollisionHistory_size[sizeof(BallCollisionHistory) == 0x74 ? 1 : -1];
#define COURSE_OFFSET(type, field) ((int)&((type*)0)->field)
enum {
    kTriangleStride = sizeof(CourseTriangle),
    kSurfaceStride = sizeof(CourseSurface),
    kGroundRegionStride = sizeof(GroundRegion),
    kMinimumBucketPartitionTriangles = 512, kOctantCount = 8,
    kNoGroundTriangle = -1,
    kRaisedGroundRegion = -2,
    kMaxGroundCandidates = 100,
    kMaxGridTriangles = 2048,
    kMotionStatic = 0, kMotionShuttle = 1, kMotionWindRotation = 2,
    kMotionAnimationOnly = 3, kMotionSpecial = 4,
    kSurfaceSkipTriangles = 2,
    kBallStopped = 0, kBallAirborne = 1, kBallSpinDelay = 2,
    kBallRolling = 3, kBallInCup = 4,
    kStoppedSpinDelayMask = 0x01, kSpinDelayEntryClearMask = 0x73,
    kCupHistoryEvent = 3, kNoHistoryCollision = 0,
    kSpecialFlightA = 0x20, kSpecialFlightB = 0x40,
    kFirstLandingSuppressBounce = 0x10,
    kLandingSpecialFlightMask = 0x70,
    kSpinLandingModeMask = 0x03,
    kCupSettleCallThreshold = 60,
    kLaunchBoundsHistorySamples = 60,
    kSpecialFlightHoldSamples = 60,
    kSpinDelayHistorySamples = 30,
    kFloatSignMask = 0x80000000u,
    kCourseFramesPerSecond = 30,
    kWindCycleSeconds = 170,
    kWindOverrideStart = 80, kWindOverrideSoundTime = 169,
    kWindEffectStart = 60, kWindEffectSoundTime = 79,
    kWindUpdatesSuppressed = 0x1000000,
    kGroundTypeTee = 0, kGroundTypeFairway = 1, kGroundTypeGreen = 2,
    kGroundTypeRough = 3, kGroundTypeBunker = 4, kGroundTypeIce = 7,
    kGroundTypeOutside = 9,
    kPatchPlaneCount = 6, kMaxClipInputVertices = 0xFE,
    kPatchCacheEntries = 32, kRayMarchSteps = 15,
    kPatchVertexFormat = 0x11F001, kGpuTriangles = 0x90,
    kTransformWorld = 11, kTransformView = 0, kTransformProjection = 1,
    kPatchVertexCapacity = 256, kPatchIndexCapacity = 384,
    kPatchSourceVertexBytes = 56, kPatchPackedVertexBytes = 24,
    kMaterialKeyMask = 0x7FF, kRecoveryDirectionCount = 12
};

// Named packed entries preserve the original Shift-JIS bytes and padding.
struct CourseObjectStrings {
    char vectorLengthError[20], lengthErrorType[55], centerPointName[5];
    char nodeTag[6], stringFormat[3], standTag[7], floatFormat[3], moveTag[6];
    char collisionBoneFilter[4]; char pad_6D[1]; char soundAliasFormat[8];
    char specialBallCupSound[10], defaultCupSound[15], woodCollisionSound[10];
    char playerActor[7], waterCollisionSound[11], floatPairFormat[6];
    char specialBallBounceSound[10], iceGrassMesh[18], shortGrassMesh[13], longGrassMesh[13];
    char extraTextureFormat[9], poleCollisionSound[10], propertyFilenameFormat[16];
    char firstCharacterName[6], characterNameFormat[7], testLabel[7], stringIntegerFormat[6];
    char treeLengthError[26];
};

// Use the supplied storage for placement construction without allocating.
inline void* operator new(size_t,void* p) { return p; }

extern "C" {
extern CourseObjectStrings courseObjectSoundStrings;
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
extern int s_quadTreeDefaultMaterialIndex;
extern const float kFloat_0_805DF744, kFloat_1_QuadTree, kFloat_30_CourseEffects, kFloat_100_0_CourseEffects, kFloat_20_QuadTree, kFloat_neg1_QuadTree;
extern double kDouble_0_CourseEffects;
void* __nw__FUl(unsigned long);
void __dla__FPv(void*);
void* __nwa__FUl(unsigned);
void* appMallocDummy__FUl(unsigned);
void appFreeUni__FPv(void*);
void* memcpy(void*,const void*,unsigned long);
void* memset(void*,int,unsigned long);
void __as__7WVectorFRC7WVector(void*,const void*);
void Vec3_Zero(void*);
void Matrix_TransformPoint(void*,const void*,const void*);
void __mi__FRC7WVectorRC7WVector(void*,const void*,const void*);
void Vector_CrossProduct(void*,const void*,const void*);
void* Normalize__7WVectorFv(void*);
void __ml__FfRC7WVector(void*,const void*,float);
float __ml__FRC7WVectorRC7WVector(void*,const void*);
void __apl__7WVectorFRC7WVector(void*,const void*);
void Vec3_DivideAssign(void*,float);
void* NameTable_FindByName(void*,const char*);
void String_ScanTokensAfterSubstring(void*,const char*,const char*,...);
void CourseTriangle_Init();
struct fn_801DAE1C_data;
void CourseTriangleBucket_Init(fn_801DAE1C_data*);
void Buffer_dtor(void*);
int WPuppet_FindBoneByName(void*,const char*);
int CString_Compare(void*,const void*);
char* MSLString_LowercaseInPlace(void*);
int WPuppet_CreateMeshByBoneName(void*,const void*,const void*);
unsigned short IPetMesh_GetGroupCount(void*);
void* IPetMesh_GetGroupByIndex(void*,unsigned short);
int CourseMaterialMap_FindInsertionPosition(void*,void*,void*,void*,void*);
void* FxSprayTree_InsertNode(void*,void*,unsigned char,unsigned char,void*);
void __dt__Q23std12length_errorFv();
void __dt__Q33std6detail16char_array_guardFv(void*,int);
void dtor_80046988(void*,int);
}
struct StringGuard {
    char* data; char* flag;
    // Release the untransferred exception string buffer on scope exit.
    ~StringGuard() { if(data) __dla__FPv(data); }
};
struct ErrorBase_u801CCCC0 : std::exception {
    // Install the exception vtable required by this construction phase.
    void SetTable(const void* vt) { *(const void**)this = vt; }
char* text; void* ref;
    // Copy the length-error message and transfer its buffer to a reference-counted owner.
    ErrorBase_u801CCCC0() {
        SetTable(__vt__Q23std11logic_error);
        char flag=0;
        unsigned n=strlen(courseObjectSoundStrings.vectorLengthError)+1;
        char* buffer=(char*)__nwa__FUl(n*2-n);
        text=buffer;
        {
        StringGuard guard;
        guard.data=buffer; guard.flag=&flag;
        char* r=(char*)__nw__FUl(16);
        if(r) { ((SharedStringReference*)r)->strongCount=1; ((SharedStringReference*)r)->weakCount=1; ((SharedStringReference*)r)->vtable=s_sharedCharDeleterVtable; ((SharedStringReference*)r)->buffer=buffer; }
        ref=r; guard.data=0;
        }
        strcpy(text,courseObjectSoundStrings.vectorLengthError);
    }
};
struct LengthError : ErrorBase_u801CCCC0 {
    // Finish the base error by installing the length_error vtable.
    LengthError() : ErrorBase_u801CCCC0() { SetTable(__vt__Q23std12length_error); }
};
struct VectorGuard_u801CCCC0 {
    char* data; unsigned size,capacity; unsigned* ownerCapacity; unsigned offset;
    // Free the allocation still held by the vector-growth guard.
    ~VectorGuard_u801CCCC0() { if(data) { size=0; appFreeUni__FPv(data); } }
};
struct ArrayGuard {
    char* block;
    // Release an array allocation unless successful construction transferred it.
    ~ArrayGuard() { if(block) __dla__FPv(block); }
};
// Release a reference-counted mesh through its deleting destructor when the final reference is dropped.
static inline void releaseResource(char* r) {
    if(r && --((MeshReference*)r)->referenceCount==0) {
        if(r) {
            typedef void (*Dtor)(void*,int);
            ((Dtor*)((MeshReference*)r)->vtable)[2](r,1);
        }
    }
}
// Return a reference to the larger unsigned value, retaining the original tie choice.
static inline const unsigned& larger(const unsigned& a,const unsigned& b) {
    return a<b?b:a;
}
// Compute a triangle AABB and mark which of the eight partitions around the surface center it overlaps.
static inline void octants(char* tri,char* entry,float* lo,int* hit) {
    float* hi=lo+3;
    float a,b,c;
    b=((CourseTriangle*)tri)->vertices[1].x; a=((CourseTriangle*)tri)->vertices[0].x; c=((CourseTriangle*)tri)->vertices[2].x;
    b=a>b?b:a; lo[0]=b>c?c:b;
    b=((CourseTriangle*)tri)->vertices[1].y; a=((CourseTriangle*)tri)->vertices[0].y; c=((CourseTriangle*)tri)->vertices[2].y;
    b=a>b?b:a; lo[1]=b>c?c:b;
    b=((CourseTriangle*)tri)->vertices[1].z; a=((CourseTriangle*)tri)->vertices[0].z; c=((CourseTriangle*)tri)->vertices[2].z;
    b=a>b?b:a; lo[2]=b>c?c:b;
    b=((CourseTriangle*)tri)->vertices[1].x; a=((CourseTriangle*)tri)->vertices[0].x; c=((CourseTriangle*)tri)->vertices[2].x;
    b=a<b?b:a; hi[0]=b<c?c:b;
    b=((CourseTriangle*)tri)->vertices[1].y; a=((CourseTriangle*)tri)->vertices[0].y; c=((CourseTriangle*)tri)->vertices[2].y;
    b=a<b?b:a; hi[1]=b<c?c:b;
    b=((CourseTriangle*)tri)->vertices[1].z; a=((CourseTriangle*)tri)->vertices[0].z; c=((CourseTriangle*)tri)->vertices[2].z;
    b=a<b?b:a; hi[2]=b<c?c:b;
    hit[0]=1; hit[1]=1; hit[2]=1; hit[3]=1;
    hit[4]=1; hit[5]=1; hit[6]=1; hit[7]=1;
    int bit=1;
    float* bound=lo;
    char* centrePointer=entry;
    for(int axis=0;axis<3;axis++,bit<<=1) {
        float centre=((CourseSurface*)centrePointer)->partitionCenter.x;
        if(bound[3]<centre) {
            if(bit==(bit&0)) hit[0]=0;
            if(bit==(bit&1)) hit[1]=0;
            if(bit==(bit&2)) hit[2]=0;
            if(bit==(bit&3)) hit[3]=0;
            if(bit==(bit&4)) hit[4]=0;
            if(bit==(bit&5)) hit[5]=0;
            if(bit==(bit&6)) hit[6]=0;
            if(bit==(bit&7)) hit[7]=0;
        } else if(bound[0]>centre) {
            hit[0]=0;
            if(!(bit&1)) hit[1]=0;
            if(!(bit&2)) hit[2]=0;
            if(!(bit&3)) hit[3]=0;
            if(!(bit&4)) hit[4]=0;
            if(!(bit&5)) hit[5]=0;
            if(!(bit&6)) hit[6]=0;
            if(!(bit&7)) hit[7]=0;
        }
        centrePointer+=4;
        bound++;
    }
}
extern "C" void CQuadTree_CreateSurfaceObject(void* self,short index);

extern "C" {

extern const float kQuadTreeGroundRayLength;
extern const float kFloat_0_8_QuadTree;
extern const float kFloat_0_7_QuadTree;
extern const float kFloat_0_9_QuadTreeGeometry;
extern const float kFloat_1_5_CourseEffects;
extern const float kFloat_neg40_CourseEffects;
extern const float kFloat_150_QuadTree;
extern const float kFloat_1_3_QuadTree;
extern const float kFloat_1_8_QuadTree;
extern const float kFloat_1_13_QuadTree;
extern const float kFloat_34_295296_QuadTree;
extern const float kFloat_0_033333335;
extern const float kFloat_0_3_BallPhysics;
extern const float kFloat_10_QuadTree;
extern const float kFloat_12_8_CourseEffects;
extern const float kFloat_6_4_QuadTree;
extern const float kFloat_0_017453292_QuadTree;
extern void* g_taskManager;
extern const float kFloat_100000_BallPhysics;
extern const float kFloat_0_1_BallPhysics;
extern const float kFloat_neg100000_QuadTree;
extern const double kDouble_0_05_QuadTreeGrid;
extern const float kFloat_0_5_QuadTree;
extern const float kFloat_3_2_QuadTree;
extern const float kFloat_200_QuadTree;
extern const float kFloat_0_03125_QuadTree;
extern const float kFloat_neg0_03125_QuadTree;
extern const char s_grassBounceSoundNames[];
int WPuppet_FindNamedBoneEntry(void*, void*);
void AffineVectors_ScaleTrailingThree(void*, void*);
void Matrix_MoveLastVectorToFront(void*, void*);
int OBB_ContainsPoint(void*, void*);
void AABB_Copy(void*, void*);
void __ami__7WVectorFRC7WVector(void*, void*);
int String_FormatTemporary(char*, int);
extern char s_cameraTransform[];
extern const float kFloat_0_15_QuadTreeGeometry;
extern const float kFloat_50_QuadTree;
extern const float kFloat_0_00001_QuadTree;
extern const float kFloat_neg2_BallPhysics;
extern const float kFloat_neg3_QuadTree;
extern const float kFloat_0_01_quadTree;
extern char s_quadTreeCollisionState[];
extern char emptySimulationSurfaceName;
extern CSharedDoc* g_sharedDoc;
extern int s_quadTreePatchCacheWriteIndex;
extern void* g_quadTreeTriangleQueryCache;
extern void* g_soundManager;

float* CourseData_GetEntryByIndex(void* self, int index);
float CQuadTree_GetLowestGround(void* self, void* arg);
float CQuadTree_SweepBallAgainstTriangles(void* self, void* arg, float* bounds, int*, int);
extern const float kFloat_neg100_QuadTree;
void Plane_Copy(void*, void*);
void Vec3_NegateInPlace(void* value);
void CQuadTree_GetMeshList(void* self, float* point);
// CodeWarrior passes an aggregate by pointer to a copy and returns it through a hidden first pointer.
// These two have the PowerPC-shaped signature (return slot, self, point pointer, ...); the by-value bodies are
// the *ByValue helpers.
Vec3f CQuadTree_FindHighestGroundPointInCandidates(void* self, Vec3f arg, int aFlag, int* bPtr, int cFlag);
#define CQuadTree_FindHighestGroundPointInCandidates(o,s,a,x,y,z) (((void (*)(void*, void*, void*, int, int, int))CQuadTree_FindHighestGroundPointInCandidates)((o),(s),(a),(x),(y),(z)))
int CQuadTree_GetMeshIndex(void* self, void* arg, int value);
int Triangle_ContainsPointXZ(const float* a, const float* b, const float* c, const float* d);
int CQuadTree_IntersectBallSweepWithObjectTriangles(void*, void*, int, void*, float*, int*);
float Plane_EvaluatePointReversed(void* a, void* b);
void* GolfPlayer_GetEntryByIndex(void* self, unsigned char value);
void* GolfBall__Fv();
void* CGolfBall_GetOrAllocatePreDataAt(void*, int);
void* PreData__9CGolfBallFi(int);
void* ItemInventory_GetBallIdAt(void*, void*);
int String_CompareCString(const char*, const char*);
float ShotParameters_GetBaseLaunchSpeed(void*);
void* Vec3_DivideScalar(void*, float, void*);
void* InstanceTable_Get();
void* CWind_GetInstance();
void CWind_GetWindVectorAtPoint(void*, void*, void*, float);
double acos(double value);
void CourseData_ClampPointToActiveRegion(void*, void*, void*, void*);
Vec3f GetGroundPoint__9CQuadTreeF7WVectorbPib(void* self, Vec3f pt, int arg3, int* outIndex, int arg6);
// ABI adapters retain the original struct-return calling conventions of vector and ground functions.
#define Add564(a,b) (((Vec3f (*)(void*, void*))__pl__FRC7WVectorRC7WVector)((a),(b)))
#define SET564(d,e) (((void (*)(void*, const Vec3f&))__as__7WVectorFRC7WVector)((d),(e)))
#define GGP_BV (GetGroundPoint__9CQuadTreeF7WVectorbPib)
#define GROUND564(s,v,a) ((GetGroundPoint__9CQuadTreeF7WVectorbPib)((s), *(Vec3f*)(v), (a), 0, 0))
#define AddR564(a,b) (((Vec3f (*)(const Vec3f&, const Vec3f&))__pl__FRC7WVectorRC7WVector)((a),(b)))
#define Scale564(a,f) (((Vec3f (*)(void*, float))__ml__FfRC7WVector)((a),(f)))
#define GetGroundPoint__9CQuadTreeF7WVectorbPib(o,s,p,a,b,c) (((void (*)(void*, void*, void*, void*, void*, int))GetGroundPoint__9CQuadTreeF7WVectorbPib)((o),(s),(p),(void*)(a),(void*)(b),(c)))
void CQuadTree_GetMeshListAlongSegment(void*, void*, void*);
int Vec3_NearlyEqualWithTolerance(void*, void*, float = 0.0f);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __amu__7WVectorFf(void*, float);
void Matrix_SetAxisRotation(void*, float, int);
float Vec3_DotProduct(void*, void*);
float WVectorLen__FRC7WVector(void*);
void* Object_GetChild();
void* TaskActorTable_FindByName(void*, const char*);
int CourseData_IsPointOutsidePlayableRegion(void*, void*);
int CQuadTree_OverlappedWithObject(void*, void*, float);
void __ml__FRC7WVectorf(void*, void*, float);
float Magnitude__7WVectorCFv(void*);
float Math_SinApprox(float);
float Math_SquareRootApprox(float);
int String_ParseQuotedToken(void*, void*);
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, void*);
void Sound_StopByAlias(void*, void*);
void Sound_PlayByName(void*, void*, int, int, void*, int, float, float, float);
void SoundManager_SetNamedVolumeScale(void*, void*, float);
void __destroy_new_array(void*, void*);
unsigned char GetGroundType__9CQuadTreeF7WVector(void*, void*);
void* FxAttachment_ParseBoneReferences(void*, void*, int, void*, int, int, int);
int CWind_SetOverrideEnabled(void*, int);
int CWind_SetEffectEnabled(void*, int);
void CWind_PlayCannonEndSoundOnce(void*);
void CWind_PlayCannonChargeSoundOnce(void*);
double floor(double);
float CWind_GetWindSpeed(void*);
void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, unsigned int, int);
extern const float kFloat_3_1415927_CourseEffects, kFloat_neg3_1415927_QuadTree;
struct CourseAnimationState {
	char pad0[4]; void* world; char pad8[0x21A0];
	float accum; unsigned int frame; void** items; unsigned int count;
};
struct CourseMotionState {
	unsigned int motionTypeWord, phaseFrames; short surfaceIndex; char padA[2]; float elapsedSeconds;
	signed char soundName[32]; float soundRadius; float angle, speed;
	char pad3C[0x10]; float pause, duration;
};
// Borrow the model associated with an animated surface index.
static inline void* Update1100_effect(CourseAnimationState* s, int index) {
	void* item = s->items[index];
	void* base = AsCourseData801C(s->world)->surfaces; // original course +0xA4
	return *(void**)((char*)base + *(short*)((char*)item + 8) * kSurfaceStride + 0x10);
}
void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
void Model_SetMatrixRecursive(void*, void*, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
void Matrix_ConcatAffine(void*, void*, void*);

struct GridTri564 { float v[3][3]; float n[3]; char pad[4]; unsigned char flag; char pad2[0x48 - 0x35]; };
struct GridCell564 {
	int* list; int pad; short count; char pad2[6];
	// Return the matching triangle-ID position, or count when absent.
	int Find(int v) { int j; for (j = 0; j < count; ++j) if (v == list[j]) break; return j; }
	// Write the next triangle ID; the caller increments count after this store.
	void Add(int v) { list[count] = v; }
};
struct GridBatch564 { char pad[0x108]; int count; };
struct GridWorld564 { char pad[0x10]; GridBatch564* batch; char pad2[0x64 - 0x14]; float minX, y, minZ, maxX, y2, maxZ; char pad3[0xB0 - 0x7C]; GridTri564* tris; };
struct Grid564 {
	char pad[8]; GridWorld564* world; char pad2[0x2064 - 0xC];
	int nx; int nz; float origin[3]; char pad3[0x207C - 0x2078]; int used; char pad4[0x2148 - 0x2080];
	GridCell564* cells; char pad5[0x2180 - 0x214C]; float floor;
};
struct Vec564 {
    float x, y, z;
    // Initialize the three vector components without applying a coordinate transform.
    Vec564(float a, float b, float c) { x = a; y = b; z = c; } };
// Select the smaller scalar with the original comparison/tie behavior.
static inline float min564(float a, float b) { return a > b ? b : a; }
// Select the larger scalar with the original comparison/tie behavior.
static inline float max564(float a, float b) { return a < b ? b : a; }
// Select a or -a relative to the supplied comparison threshold (normally zero).
static inline float abs564(float a, float z) { return a > z ? a : -a; }

void CourseTriangleGrid_Rebuild(void* self, void* arg);


void CQuadTree_UpdateAnimatedObjects(void* self, float delta, int force);

// Copy three vector bit-pattern words, storing Z/Y/X after loading all components.
static inline void Copy3(Arr3* d, void* p) {
	int* s = (int*)p;
	int s0 = s[0], s1 = s[1], s2 = s[2];
	((int*)d)[2] = s2; ((int*)d)[1] = s1; ((int*)d)[0] = s0;
}

// Copy three vector bit-pattern words in X/Y/Z store order.
static inline void Copy3f(int* d, void* p) {
	int* s = (int*)p;
	int s0 = s[0], s1 = s[1], s2 = s[2];
	((int*)d)[0] = s0; ((int*)d)[1] = s1; ((int*)d)[2] = s2;
}

void Triangle_ClosestPointOnEdges(void* out, void* p4, void* p5, void* p6, void* p7, void* p8);

void CQuadTree_GetObjList(void* self, float* bounds);

void CQuadTree_MakeCollisionData(void*, int);
float CQuadTree_FindNearestSweptBallObjectHit(void* self, float* box, float* direction, float* outScale);

// Clamp a signed cell coordinate to the inclusive low/high limits.
static inline int fn_801D00F8_clamp(int v, int lo, int hi) {
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

void CQuadTree_GetMeshListInBounds(void* self, float* bounds);

struct RCBase { int ref; };
struct RC : RCBase { 
virtual ~RC();
};

extern char s_quadTreeUnitScaleVector[];
float CQuadTree_IntersectBallSweepWithPoint(void*, void*, void*, void*);

struct Rec801D9644 { float v[7]; };
struct Box801D9644 { float v[6]; };
float CQuadTree_ResolveGroundHeight(void* self, Vec3f* point, float heightClearance);


float CQuadTree_IntersectBallPathWithCupXZ(void*, float*);
void CQuadTree_HandleBallGroundBounce(void*, void*);
extern const float kFloat_neg0_3_CupCollision;
// Ball history indices are ABI slots; neutral names avoid assigning an unproved shot event.
enum BallHistorySlot {
    kHistorySlot1 = 1, kHistorySpecialFlightB = 2, kHistorySlot3 = 3,
    kHistoryFastGreenContact = 6, kHistorySpinDelayStart = 7,
    kHistoryRollingStart = 8, kHistorySpecialFlightA = 10
};
enum CollisionLayoutMask { kCollisionBoneSkipMask = 0x40000, kMaterialSpecialSurfaceMask = 1 };
// +0x74 is a flag word in the borrowed bone-transform owner; its other fields remain unresolved.
enum { kCollisionBoneFlagsOffset = 0x74 };

struct NormalWord6 { int x,y,z; };
// Copy the ball/selected ground normal as words, flipping two-sided downward normals.
inline void NormalOut6(void* out,void* self,int ix) {
	if(ix<0) {
		ix=CQuadTree_GetMeshIndex(self,GolfBall__Fv(),0);
		if(ix<0) {
			((Vec3f*)out)->x=kFloat_0_805DF744;
			((Vec3f*)out)->y=kFloat_0_805DF744;
			((Vec3f*)out)->z=kFloat_0_805DF744;
		} else {
			int off=ix* kTriangleStride;
			NormalWord6 t=*(NormalWord6*)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface+COURSE_OFFSET(CourseSurface, triangles))+off+COURSE_OFFSET(CourseTriangle, normal));
			if(*(unsigned char*)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface+COURSE_OFFSET(CourseSurface, triangles))+off+COURSE_OFFSET(CourseTriangle, twoSided)) && ((Vec3f*)&t)->y<kFloat_0_805DF744) Vec3_NegateInPlace(&t);
			*(NormalWord6*)out=t;
		}
	} else {
		int off=ix* kTriangleStride;
		NormalWord6 t=*(NormalWord6*)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface+COURSE_OFFSET(CourseSurface, triangles))+off+COURSE_OFFSET(CourseTriangle, normal));
		if(*(unsigned char*)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface+COURSE_OFFSET(CourseSurface, triangles))+off+COURSE_OFFSET(CourseTriangle, twoSided)) && ((Vec3f*)&t)->y<kFloat_0_805DF744) Vec3_NegateInPlace(&t);
		*(NormalWord6*)out=t;
	}
}
// Read the current hole's cup center using the original three-word copy.
inline NormalWord6 CenterValue6() {
	char* p=((char*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	NormalWord6 v=*(NormalWord6*)(((char*)AsCGolfDoc(p)->holeRecords)+AsCGolfDoc(p)->currentHoleIndex*32+12);
	return v;
}
// Copy the current hole's world-space cup position into caller-owned three-word storage.
inline void CopyCupCenterWords(NormalWord6& destination) {
    char* document = (char*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    char* hole = (char*)AsCGolfDoc(document)->holeRecords + AsCGolfDoc(document)->currentHoleIndex * 32;
    *(Arr3*)&destination = *(Arr3*)(hole + 12);
}
// Stop and encode a holed ball, then choose the cup sound for the equipped ball.
inline void Stop6() {
	char* obj=(char*)GolfBall__Fv();
	char* st=(char*)CGolfBall_GetOrAllocatePreDataAt(obj,*(int*)(obj+0x64));
	Vec3_Zero(obj+12); *(int*)(obj+0x18)=0;
	{ int value=1; char* key=obj+0x1989C; char* r=key; unsigned char* kb=(unsigned char*)&key; *(unsigned long*)(obj+0x1989C)=~*(unsigned long*)&value^*(unsigned long*)(obj+0x198A4); for(int byte=0;byte<sizeof(key);++byte) (obj)[byte+0x198A0]=r[byte]^kb[byte]; }
	((BallCollisionHistory*)st)->eventType=3; ((BallCollisionHistory*)st)->collisionType=0;
	char* pl; char* mgr; mgr=(char*)g_sharedDoc; pl=((char*)AsCSharedDoc(mgr)->golfDoc);
	void* found=ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(mgr)->itemInventory[0]),(void*)12);
	if(found==*(void**)(mgr+AsCGolfDoc(pl)->currentPlayer*0x1D4+0x88) ||
		(mgr=(char*)g_sharedDoc, pl=((char*)AsCSharedDoc(mgr)->golfDoc), found=ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(mgr)->itemInventory[0]),(void*)128),
		found==*(void**)(mgr+AsCGolfDoc(pl)->currentPlayer*0x1D4+0x88)))
		strcpy(st+0x20,courseObjectSoundStrings.specialBallCupSound);
	else strcpy(st+0x20,courseObjectSoundStrings.defaultCupSound);
}
void CQuadTree_UpdateBallCupCollision(void* self, float delta);

void CQuadTree_SetObjectSoundEnabled(void* self, int value);

void CQuadTree_PlayObjectSoundFromParsedToken(void* self, void* cursor, int index);

struct fn_801D6928_data {
	void* data;
	int count;
};

void* Array_GetLastElement36(fn_801D6928_data* self);

float Float_Get(float* value);

int CQuadTree_IsSimulationEnabled();

// Compare the caller's absolute curve/spin setting with its threshold, strictly.
static inline int IsAirSettingMagnitudeBelow(float a, float b) { return a < b; }
// Set all three force components to the supplied value.
static inline void SetAirForceComponents(float* out, float z) { out[0] = z; out[1] = z; out[2] = z; }
// Multiply force scalars while retaining the original inline expression boundary.
static inline float ScaleCurveForce(float a, float b) { return a * b; }
// Add a scaled scalar contribution to the vertical force component.
static inline void AddScaledSpinLift(float* out, float a, float b) { FP_FMADDS_ACCUM(out[1], a, b); }
WVector CQuadTree_ComputeAirborneBallForces(void* self);

struct fn_801DAE1C_data {
	int triangleIdsWord;
	int objectIdsWord;
	short triangleCount;
	char objectCount;
	char padb;
	int reserved0C;
};

void CQuadTree_SetSimulationMode(void* self, int value);

void* CQuadTree_FindProjectedPatchCacheEntry(void* unused, float* first, float* last);

void CQuadTree_CalcGreenPlane(void* self);

float CQuadTree_GetGroundSlope(void* self, int index);

void CQuadTree_GetMeshNormalAtPoint(void* out, void* self, void* arg);

int CQuadTree_IsPenetrable(const void* self, int id = 0);

char* CQuadTree_GetBounceSound(void* self, int id = 0);

void CQuadTree_ParseBoneReferences(void* self, void* arg1, int index, void* arg3, void* arg4, void* arg5);

float CQuadTree_GetBounceCoeff(const void* self, int id = 0);

void CQuadTree_SetCondition(void* self);

int CQuadTree_GetObjColNum(void* self);

int CQuadTree_GetBounceCount(void* self);

int CQuadTree_GetGroundFlagBit1AtPoint(void* self, void* arg);

float CQuadTree_GetMaxPowerOnGround(void* self);

float CQuadTree_GetMinPowerOnGround(void* self);

float CQuadTree_GetMaxPowerOnGroundAtPoint(void* self, void* arg);

float CQuadTree_GetMinPowerOnGroundAtPoint(void* self, void* arg);

void CQuadTree_AdjustPointToRegionAndGroundClearance(void* self, void* arg, void* value);

float CQuadTree_GetGreenPos(void* self, void* arg);

extern "C" int QuadTree_IsWaterAtPoint(void* self, void* arg);

extern "C" void* CQuadTree_GetTextureName(void* self, void* arg);

extern "C" void CQuadTree_GetMeshNormal(void* out, void* self, int index);

extern "C" int CQuadTree_TestHighSpeedGroundImpact(void*, void*);
struct NameTable { int p[4]; };
struct V3 { int x, y, z; };

extern float kFloat_0_25_QuadTree;
float* Vec3_Set(float*, float, float, float);
float CGolfBall_GetRadius(void*);
void* GolfDoc_GetCurrent();
void* SharedDoc_GetCurrent();
void* SharedDoc_GetItemInventory();
int Float_IsAbsBelow(void*, float);

struct Words6360 { unsigned int x, y, z; };
// Collision events are 36-byte inline values: byte type, aligned short object index,
// triangle/history indices, then position and velocity bits. No event owns heap payload.
// Vector growth constructs the appended event first because item may alias old storage.
struct fn_801D6360_item {
	unsigned char objectType;
	short objectIndex;
	unsigned int triangleIndex, historyIndex;
	Words6360 positionWords, directionWords;
	// Copy the event tag, indices and both vector bit-pattern records.
	inline fn_801D6360_item(fn_801D6360_item& s)
		: objectType(s.objectType), objectIndex(s.objectIndex), triangleIndex(s.triangleIndex), historyIndex(s.historyIndex), positionWords(s.positionWords), directionWords(s.directionWords) {}
	// Copy the event tag, indices and both vector bit-pattern records.
	inline fn_801D6360_item(const fn_801D6360_item& __restrict s) {
		objectType = s.objectType;
		objectIndex = s.objectIndex;
		triangleIndex = s.triangleIndex;
		historyIndex = s.historyIndex;
		positionWords.x = s.positionWords.x;
		positionWords.y = s.positionWords.y;
		positionWords.z = s.positionWords.z;
		directionWords.x = s.directionWords.x;
		directionWords.y = s.directionWords.y;
		directionWords.z = s.directionWords.z;
	}
};

struct fn_801D6360_vector {
	fn_801D6360_item* data;
	unsigned int size;
	unsigned int capacity;
};

void CollisionEventVector_PushBack(fn_801D6360_vector*, const fn_801D6360_item*);

struct Self5EB0 {
	char pad[0x21C4];
	fn_801D6360_item item;
	fn_801D6360_vector vec;
};

void CQuadTree_ApplyBallMaterialCollision(Self5EB0* self, void* arg, void* dir, float delta);

float CQuadTree_GetHighestGround(void* self, float* point, int* outIndex);

#undef GetGroundPoint__9CQuadTreeF7WVectorbPib
#define GetGroundPoint__9CQuadTreeF7WVectorbPib(o,s,p,a,b,c) (((void (*)(void*, void*, void*, void*, void*, int))GetGroundPoint__9CQuadTreeF7WVectorbPib)((o),(s),(p),(void*)(a),(void*)(b),(c)))

// Select the smaller hit fraction without adding another validity test.
static inline float fn_801D9424_min(float a, float b) {
	if (a > b) return b;
	return a;
}

float Geometry_GetRayTriangleHitFraction(void* unused, void* self, void* a, void* b);

Vec3f CQuadTree_GetRayIntersection(void* self, void* point, void* ray);

#define CELL564(px, py) ((*(GridCell564**)(s + 0x2148))[*((int*)(s + 0x2064)) * (py) + (px)])

#undef CQuadTree_FindHighestGroundPointInCandidates
#define CQuadTree_FindHighestGroundPointInCandidates(o,s,a,x,y,z) (((void (*)(void*, void*, void*, int, int, int))CQuadTree_FindHighestGroundPointInCandidates)((o),(s),(a),(x),(y),(z)))

// Test absolute displacement against the ground-position epsilon.
static inline bool fn_801D7F88_near(float d) {
	float a = (*(int*)&d & 0x80000000) ? -d : d;
	return a < kFloat_0_00001_QuadTree;
}

struct Box7F88 { float v[6]; };
struct Rec7F88 { Box7F88 b; int pad; };
// Test strict X/Z region interior and Y below the region top.
static inline bool fn_801D7F88_in(Box7F88 rec, float* point) {
	return point[0] > rec.v[0] && point[0] < rec.v[3] && point[2] > rec.v[2] && point[2] < rec.v[5] && point[1] < rec.v[4];
}

void CQuadTree_ClipPolygonToSixPlanes(void* self, void* points, int count, int mode);

// Copy the first containing raised-region lie to the global ball and report a hit.
static inline int fn_801D28D4_region(void* self, Vec3f* pp) {
	Vec3f& p = *pp;
	unsigned int count = AsCQuadTree(self)->groundRegionCount;
	for (unsigned int i = 0; i < count; ++i) {
		char* rec = (*(char**)&AsCQuadTree(self)->groundRegions) + i * 0x1c;
		if (p.x > *(float*)rec && p.x < *(float*)(rec + 0xc) && p.z > *(float*)(rec + 8) && p.z < *(float*)(rec + 0x14) && p.y < *(float*)(rec + 0x10)) {
			int v = *(int*)(rec + 0x18);
			((CGolfBall*)GolfBall__Fv())->m_lie = v;
			return 1;
		}
	}
	return 0;
}

#define PLAYER_POS(s) ((void*(*)(void*, int))PreData__9CGolfBallFi)(s, 0)
struct V28w { unsigned int a, b, c; };
struct V28c {
    float x, y, z;
    // Leave the local vector's components uninitialized until explicitly assigned.
    V28c() {}
    // Initialize the local vector from three scalar components.
    V28c(float a, float b, float c) : x(a), y(b), z(c) {}
    // Copy a vector's three component bit patterns without a coordinate transform.
    V28c(const Vec3f& v) { *(V28w*)this = *(const V28w*)&v; } };

// Return a ground triangle normal, flipping a downward two-sided face, or zero for a negative ID.
static inline V28c fn_801D28D4_bound(void* self, int idx) {
	if (idx < 0) return V28c(kFloat_0_805DF744, kFloat_0_805DF744, kFloat_0_805DF744);
	Vec3f b = *(Vec3f*)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + 0xb0) + idx * kTriangleStride + 0x24);
	if (*(unsigned char*)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + 0xb0) + idx * kTriangleStride + 0x34) && b.y < kFloat_0_805DF744) Vec3_NegateInPlace(&b);
	return V28c(b);
}

struct Q28w { char pad[0x2084]; WVector p; };
#define GROUND28(s, p) ((WVector (*)(void*, WVector, int, int, int))GetGroundPoint__9CQuadTreeF7WVectorbPib)(s, p, 0, 0, 0)
#define ROT28(v, m) ((WVector (*)(void*, void*))Matrix_TransformPoint)(v, m)
// Borrow a temporary vector address for the enclosing expression only.
static inline const WVector* addr28(const WVector& v) { return &v; }
void CQuadTree_FindBallRecoveryPosition(void* self, void* arg);

void CQuadTree_ComputeBallGroundForces(float* out, void* self);

float CQuadTree_FindObstructionSweepFraction(void* self, float* box, void* direction);

void Geometry_ClosestRaySegmentParameters(void* out, void* unused, void* a, void* b, void* c, void* d);

extern signed char s_projectedTexturedPatchInitialized;
extern const float kFloat_2_QuadTree;
extern const float kFloat_255_QuadTree;
extern char s_projectedPatchPositions[];
extern char s_quadTreeProjectedPatchIndices[];
extern float kFloat_65535_QuadTree, kFloat_neg65535_QuadTreeBounds;
void CQuadTree_BuildProjectedPatchMesh(void* self, void* boxArg, void* lastArg, void* vertices, int* vertexCount, int vertexCapacity, void* indices, int* indexCount, int indexCapacity, unsigned char includeObjects);
void CQuadTree_DrawIndexedTexturedMesh(void*, void*, int, void*, int, void*);
void Resource_GetOffsetPtr(void*, int, int, void**, int);
void GraphicsBuffer_Flush(void*);
void IndexBuffer_GetOffsetPtr(void*, int, int, void**, int);
void IndexBuffer_Flush(void*);
void UIRender_SetDefaultState(void);
void Engine_SetMaterial(void*);
void SetTransform(int, void*);
void* Camera_GetRenderState(void*);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetVertexStream(int, void*, int, int);
void Engine_SetIndexBuffer(void*, int);
void Engine_SetTextureStageResource(int, int);
void Engine_SetTexturePalette(int, int);
void Engine_SetCachedRenderState(int, int);
void Engine_DrawIndexedPrimitive(int, int, int, int, int, int);
extern void* g_view;
extern const unsigned int kQuadTreePatchMaterial[];
extern float kIdentityMatrix4x4[];

struct RenderVertexAA94 { float x, y, z; unsigned char color[4]; float u, v; };

typedef int (*FnAA94)(void*, void*);
struct ObjAA94 { 
virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3(); virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7(); virtual void s8(); virtual void s9(); virtual void s10(); virtual void s11(); virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15(); virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19(); virtual void s20(); virtual void s21(); virtual void s22(); virtual void s23(); virtual void s24(); virtual void s25(); virtual int s26(void*); virtual int s27(void*);
};
struct MatAA94 { unsigned int w[17]; };
struct GameAA94 { char pad[0xA4]; ObjAA94* res; };

void CQuadTree_DrawProjectedTexturedPatch(void* self, float amount, float alpha, void* a, void* b, void* c, void* d, void* unused, void* extra);

extern void CQuadTree_UpdateBallTriangleCollision(void* self, float dt);
extern signed char s_quadTreeInitialized;
extern float kFloat_12_566371_CourseEffects, kFloat_25_132742_QuadTree, kFloat_62_831856_CourseEffects, kFloat_1_3962634_QuadTree, kFloat_neg10_QuadTree, kFloat_1_0471976_QuadTree, kFloat_0_75_QuadTree;
void CQuadTree_Ctor(void*);
void CQuadTree_dtor(void*);
struct Words2F40 { unsigned int x, y, z; };
struct TriW {
    char pad[0x24]; WVector n; char pad2[0x48-0x30];
    // Borrow this triangle view's normal vector without copying it.
    const WVector& N() { return n; } };
void CQuadTree_UpdateAirborneBall(void* self, float dt);

extern float kFloat_0_4_QuadTree, kFloat_12_QuadTreeGeometry, kFloat_5_QuadTreeGeometry;
extern float kFloat_45_QuadTree, kFloat_80_QuadTree;
float Plane_EvaluatePoint(void*, void*);
extern const float kGroundOverrideHoleRadiusScale;
float Vector_SignedAngleXZ(void*, void*);
// Same holed-ball operation used by the main simulation path; keep its original inline lifetime.
inline void Stop179() {
	char* obj=(char*)GolfBall__Fv();
	char* st=(char*)CGolfBall_GetOrAllocatePreDataAt(obj,*(int*)(obj+0x64));
	Vec3_Zero(obj+12); *(int*)(obj+0x18)=0;
	{ int value=1; char* key=obj+0x1989C; char* r=key; unsigned char* kb=(unsigned char*)&key; *(unsigned long*)(obj+0x1989C)=~*(unsigned long*)&value^*(unsigned long*)(obj+0x198A4); for(int byte=0;byte<sizeof(key);++byte) (obj)[byte+0x198A0]=r[byte]^kb[byte]; }
	((BallCollisionHistory*)st)->eventType=3; ((BallCollisionHistory*)st)->collisionType=0;
	char* pl; char* mgr; mgr=(char*)g_sharedDoc; pl=((char*)AsCSharedDoc(mgr)->golfDoc);
	void* found=ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(mgr)->itemInventory[0]),(void*)12);
	if(found==*(void**)(mgr+AsCGolfDoc(pl)->currentPlayer*0x1D4+0x88) ||
		(mgr=(char*)g_sharedDoc, pl=((char*)AsCSharedDoc(mgr)->golfDoc), found=ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(mgr)->itemInventory[0]),(void*)128),
		found==*(void**)(mgr+AsCGolfDoc(pl)->currentPlayer*0x1D4+0x88)))
		strcpy(st+0x20,courseObjectSoundStrings.specialBallCupSound);
	else strcpy(st+0x20,courseObjectSoundStrings.defaultCupSound);
}
extern void CQuadTree_StepBallSimulation(void* self, float dt);

} // extern "C"

// Create motion state, transformed collision triangles and one/eight triangle buckets for a course surface.
// index must select a valid course surface; a missing puppet is a no-op. The course
// retains the new triangles/buckets and the quadtree owns the new motion object.
extern "C" void CQuadTree_CreateSurfaceObject(void* self,short index) {
    CourseSurface* entry=(CourseSurface*)AsCourseData801C((*(char**)&AsCQuadTree(self)->courseData))->surfaces+index; // original +0xA4
    char* root=(char*)self;
    if(!(char*)entry->puppet) return;
    // Build optional motion state, parse shuttle endpoints, and publish the owned motion pointer.
    if(entry->motionType) {
        char* object=0;
        unsigned lifetime;
        float delay=entry->puppet->animationDuration;
        if(delay>kDouble_0_CourseEffects) lifetime=Runtime_ConvertDoubleToUnsigned(kFloat_30_CourseEffects*delay);
        else lifetime=30;
        switch((int)entry->motionType) {
        case kMotionShuttle: {
            char* p=(char*)__nw__FUl(0x54);
            object=p;
            if(p) { ((CourseMotionState*)p)->soundRadius=kFloat_100_0_CourseEffects; *(unsigned char*)&((CourseMotionState*)p)->soundName[0]=0; }
            float one,zero;
            zero=kFloat_0_805DF744; one=kFloat_1_QuadTree;
            ((CourseMotionState*)p)->pause=zero; ((CourseMotionState*)p)->duration=one;
            __as__7WVectorFRC7WVector(p+0x34,(char*)&entry->position);
            __as__7WVectorFRC7WVector(object+0x40,(char*)&entry->position);
            if(*(signed char*)(entry->motionTags)) {
                char* cursor=entry->motionTags;
                char name[16];
                String_ScanTokensAfterSubstring(&cursor,courseObjectSoundStrings.nodeTag,courseObjectSoundStrings.stringFormat,name);
                String_ScanTokensAfterSubstring(&cursor,courseObjectSoundStrings.standTag,courseObjectSoundStrings.floatFormat,object+0x4C);
                String_ScanTokensAfterSubstring(&cursor,courseObjectSoundStrings.moveTag,courseObjectSoundStrings.floatFormat,object+0x50);
                char* desc=(char*)NameTable_FindByName(*(char**)&AsCQuadTree(root)->courseData,name);
                if(desc) {
                    __as__7WVectorFRC7WVector(object+0x34,((CourseNamedEntry*)desc)->positions);
                    __as__7WVectorFRC7WVector(object+0x40,((CourseNamedEntry*)desc)->positions+12);
                    __as__7WVectorFRC7WVector((char*)&entry->position,((CourseNamedEntry*)desc)->positions);
                }
            }
            break;
        }
        case kMotionWindRotation: {
            char* p=(char*)__nw__FUl(0x3C);
            if(p) { ((CourseMotionState*)p)->soundRadius=kFloat_100_0_CourseEffects; *(unsigned char*)&((CourseMotionState*)p)->soundName[0]=0; }
            double y=entry->boundsMax.y-entry->boundsMin.y;
            double x=entry->boundsMax.x-entry->boundsMin.x;
            double z=entry->boundsMax.z-entry->boundsMin.z;
            y=x<y?y:x; z=y<z?z:y;
            object=p; ((CourseMotionState*)p)->speed=kFloat_1_QuadTree/(float)z;
            break;
        }
        case kMotionSpecial: {
            char* p=(char*)__nw__FUl(0x34);
            if(p) { ((CourseMotionState*)p)->soundRadius=kFloat_100_0_CourseEffects; *(unsigned char*)&((CourseMotionState*)p)->soundName[0]=0; }
            object=p; break;
        }
        case kMotionAnimationOnly: {
            char* p=(char*)__nw__FUl(0x34);
            if(p) { ((CourseMotionState*)p)->soundRadius=kFloat_100_0_CourseEffects; *(unsigned char*)&((CourseMotionState*)p)->soundName[0]=0; }
            object=p; break;
        }
        }
        if(object) {
            while(AsCQuadTree(root)->effectRandomState<lifetime) AsCQuadTree(root)->effectRandomState=AsCQuadTree(root)->effectRandomState*AsCQuadTree(root)->effectRandomState+1;
            AsCQuadTree(root)->effectRandomState%=lifetime;
            ((CourseMotionState*)object)->phaseFrames=AsCQuadTree(root)->effectRandomState;
            *(unsigned char*)&((CourseMotionState*)object)->motionTypeWord=entry->motionType;
            ((CourseMotionState*)object)->surfaceIndex=index;
            if(AsCQuadTree(root)->objectCount<AsCQuadTree(root)->objectCapacity) {
                unsigned size=AsCQuadTree(root)->objectCount+1;
                AsCQuadTree(root)->objectCount=size;
                ((char**)(*(char**)&AsCQuadTree(root)->objects))[size-1]=object;
            } else {
                if(0x3FFFFFFF-AsCQuadTree(root)->objectCapacity<1U) {
                    char storage[sizeof(LengthError)];
                    LengthError* error=new(storage) LengthError;
                    __throw(courseObjectSoundStrings.lengthErrorType,error,(void*)__dt__Q23std12length_errorFv);
                }
                unsigned need=AsCQuadTree(root)->objectCount+1-AsCQuadTree(root)->objectCapacity;
                VectorGuard_u801CCCC0 guard;
                guard.data=0; guard.size=0; guard.capacity=0;
                guard.ownerCapacity=&AsCQuadTree(root)->objectCapacity; guard.offset=0;
                unsigned oldcap=AsCQuadTree(root)->objectCapacity,newcap;
                if(need>0x3FFFFFFF-oldcap) {
                    char storage[sizeof(LengthError)];
                    LengthError* error=new(storage) LengthError;
                    __throw(courseObjectSoundStrings.lengthErrorType,error,(void*)__dt__Q23std12length_errorFv);
                }
                if(oldcap<0x15555555) {
                    unsigned grow=((oldcap+1)*3)/5;
                    newcap=oldcap+larger(grow,need);
                } else if(oldcap<0x2AAAAAAA) {
                    unsigned grow=(oldcap+1)/2;
                    newcap=oldcap+larger(grow,need);
                } else newcap=0x3FFFFFFF;
                if(newcap>0x3FFFFFFF) {
                    char storage[sizeof(LengthError)];
                    LengthError* error=new(storage) LengthError;
                    __throw(courseObjectSoundStrings.lengthErrorType,error,(void*)__dt__Q23std12length_errorFv);
                }
                guard.data=(char*)appMallocDummy__FUl(newcap*4);
                unsigned size=AsCQuadTree(root)->objectCount;
                guard.offset=size;
                ((char**)guard.data)[size+guard.size]=object;
                guard.size++;
                char* old=(*(char**)&AsCQuadTree(root)->objects);
                guard.capacity=newcap;
                int count=((char**)old+AsCQuadTree(root)->objectCount)-(char**)old;
                guard.offset=size-count;
                memcpy(guard.data+guard.offset*4,old,count*4);
                memset(old,0,count*4);
                guard.size+=count;
                old=(*(char**)&AsCQuadTree(root)->objects);
                unsigned cap=AsCQuadTree(root)->objectCapacity;
                AsCQuadTree(root)->objectCount=0;
                (*(char**)&AsCQuadTree(root)->objects)=guard.data; AsCQuadTree(root)->objectCapacity=guard.capacity;
                guard.data=old; guard.capacity=cap;
                AsCQuadTree(root)->objectCount=guard.size; guard.size=0;
            }
        }
    }
    // Motion setup is complete; register eligible surface bounds in the X/Z object grid.
    if(*(int*)&entry->flags==3) return;
    float bounds[6];
    Vec3_Zero(bounds); Vec3_Zero(bounds+3);
    if(entry->motionType!=kMotionShuttle && entry->motionType!=kMotionAnimationOnly) {
        int max=AsCQuadTree(root)->gridColumns-1;
        int x0=(int)((entry->boundsMin.x-AsCQuadTree(root)->gridOriginX)/kFloat_20_QuadTree);
        if(x0<0) x0=0; else if(x0>max) x0=max;
        int x1=(int)((entry->boundsMax.x-AsCQuadTree(root)->gridOriginX)/kFloat_20_QuadTree);
        if(x1<0) x1=0; else if(x1>max) x1=max;
        max=AsCQuadTree(root)->gridRows-1;
        int z0=(int)((entry->boundsMin.z-AsCQuadTree(root)->gridOriginZ)/kFloat_20_QuadTree);
        if(z0<0) z0=0; else if(z0>max) z0=max;
        int z1=(int)((entry->boundsMax.z-AsCQuadTree(root)->gridOriginZ)/kFloat_20_QuadTree);
        if(z1<0) z1=0; else z1 = (max > z1) ? z1 : max;
        for(int z=z0;z<=z1;z++) for(int x=x0;x<=x1;x++) {
            int offset=(x+z*AsCQuadTree(root)->gridColumns)*16;
            char* slot=(*(char**)&AsCQuadTree(root)->cells)+offset;
            int n=((GridCellView*)slot)->objectCount;
            if(n==0) {
                *(char**)&((GridCellView*)((*(char**)&AsCQuadTree(root)->cells)+offset))->objectIds=(char*)__nwa__FUl(8);
                ((GridCellView*)((*(char**)&AsCQuadTree(root)->cells)+offset))->objectIds[0]=index;
                ((GridCellView*)((*(char**)&AsCQuadTree(root)->cells)+offset))->objectCount=1;
            } else {
                int i=0;
                for(;i<n;i++) if(index==((short*)*(char**)&((GridCellView*)slot)->objectIds)[i]) break;
                if(i<n) continue;
                if(n%4==0) {
                    char* tmp=(char*)__nwa__FUl((unsigned char)n*2);
                    slot=(*(char**)&AsCQuadTree(root)->cells)+offset;
                    memcpy(tmp,*(char**)&((GridCellView*)slot)->objectIds,((GridCellView*)slot)->objectCount*2);
                    __dla__FPv(*(char**)&((GridCellView*)((*(char**)&AsCQuadTree(root)->cells)+offset))->objectIds);
                    char* next=(char*)__nwa__FUl((((GridCellView*)((*(char**)&AsCQuadTree(root)->cells)+offset))->objectCount+4)*2);
                    *(char**)&((GridCellView*)((*(char**)&AsCQuadTree(root)->cells)+offset))->objectIds=next;
                    slot=(*(char**)&AsCQuadTree(root)->cells)+offset;
                    memcpy(*(char**)&((GridCellView*)slot)->objectIds,tmp,((GridCellView*)slot)->objectCount*2);
                    __dla__FPv(tmp);
                }
                slot=(*(char**)&AsCQuadTree(root)->cells)+offset;
                ((short*)*(char**)&((GridCellView*)slot)->objectIds)[((GridCellView*)slot)->objectCount]=index;
                slot=(*(char**)&AsCQuadTree(root)->cells)+offset; ((GridCellView*)slot)->objectCount++;
            }
        }
    }
    // Allocate collision triangles, transform mesh vertices to world space and resolve materials.
    if(entry->flags&kSurfaceSkipTriangles) return;
    int triangles=*(int*)&entry->puppet->triangleCount;
    char* block=(char*)__nwa__FUl(triangles* kTriangleStride+16);
    if(block) {
        ArrayGuard guard; guard.block=block;
        block=(char*)__construct_new_array(block,(void*)CourseTriangle_Init,0,0x48,triangles);
        guard.block=0;
    }
    entry->triangles=(CourseTriangle*)block;
    int filter=WPuppet_FindBoneByName((char*)entry->puppet,courseObjectSoundStrings.collisionBoneFilter)!=0;
    char* resource=0;
    char* model=(char*)entry->puppet;
    int triangleOffset=0;
    for(unsigned mesh=0;mesh<((WPuppet*)model)->namedBoneCount;mesh++) {
        char* pair=(*(char**)&((WPuppet*)model)->namedBones)+mesh*8;
        if(!pair) continue;
        char* transform=((BoneNamePair*)pair)->bone;
        if(CString_Compare(((BoneNamePair*)pair)->nameRecord+0x2C,transform+8)) continue;
        if(filter) {
            char* needle=courseObjectSoundStrings.collisionBoneFilter;
            char* name=MSLString_LowercaseInPlace(((BoneNamePair*)pair)->nameRecord+0x2C);
            if(!strstr(name,needle)) continue;
        }
        if(!WPuppet_CreateMeshByBoneName((char*)entry->puppet,&resource,transform+8)) continue;
        int groups=IPetMesh_GetGroupCount(resource);
        for(int group=0;group<groups;group++) {
            // Keep byte indexing of parts: direct parts[part] swaps add operands in this compiler.
            MeshGroup* geometry=(MeshGroup*)IPetMesh_GetGroupByIndex(resource,(unsigned short)group);
            for(unsigned part=0;part<geometry->partCount;part++) {
                for(unsigned v=0;v<((MeshPart*)((char*)geometry->parts + part * sizeof(MeshPart)))->vertexCount;v+=3) {
                    if(entry->motionType==kMotionStatic || entry->motionType==kMotionSpecial) {
                        for(int j=0;j<3;j++) {
                            memcpy((char*)entry->triangles+triangleOffset+j*12,((MeshPart*)((char*)geometry->parts + part * sizeof(MeshPart)))->vertices+(v+j)*12,12);
                            float tmp[3];
                            Matrix_TransformPoint(tmp,(char*)entry->triangles+triangleOffset+j*12,transform+0x78);
                            __as__7WVectorFRC7WVector((char*)entry->triangles+triangleOffset+j*12,tmp);
                        }
                        char* tri=(char*)entry->triangles+triangleOffset;
                        float edge0[3],edge1[3],cross[3],normal[3],negative[3];
                        __mi__FRC7WVectorRC7WVector(edge0,tri,tri+12);
                        __mi__FRC7WVectorRC7WVector(edge1,tri+24,tri+12);
                        Vector_CrossProduct(cross,edge1,edge0);
                        __as__7WVectorFRC7WVector(normal,cross); Normalize__7WVectorFv(normal);
                        __as__7WVectorFRC7WVector((char*)entry->triangles+triangleOffset+0x24,normal);
                        __ml__FfRC7WVector(negative,(char*)entry->triangles+triangleOffset+0x24,kFloat_neg1_QuadTree);
                        float plane=__ml__FRC7WVectorRC7WVector(negative,tri);
                        ((CourseTriangle*)((char*)entry->triangles+triangleOffset))->planeConstant=plane;
                    }
                    *(char**)&((CourseTriangle*)((char*)entry->triangles+triangleOffset))->boneTransform=transform;
                    ((CourseTriangle*)((char*)entry->triangles+triangleOffset))->twoSided=0;
                    {
                        char* key=(char*)&geometry->parts[part];
                        char* map=*(char**)&AsCQuadTree(root)->courseData+0xAC;
                        char* node; unsigned char left,right;
                        char* where=(char*)CourseMaterialMap_FindInsertionPosition(map,key,&node,&left,&right);
                        char* value=node;
                        int keyValue;
                        if(!value || (keyValue=*(int*)&((MeshPart*)key)->materialKey,keyValue > ((MaterialKeyPair*)(value+=12))->key)) {
                            unsigned pairValue[2]; pairValue[0]=((MeshPart*)key)->materialKey; pairValue[1]=s_quadTreeDefaultMaterialIndex;
                            value=(char*)FxSprayTree_InsertNode(map,where,left,right,pairValue)+12;
                        }
                        char* material=*(char**)&((CourseMaterialTable*)*(char**)&AsCQuadTree(root)->courseData)->entries+((MaterialKeyPair*)value)->materialIndex*0x28;
                        *(unsigned int*)&((CourseTriangle*)((char*)entry->triangles+triangleOffset))->materialIndex=((CourseMaterialSource*)material)->materialIndex;
                    }
                    {
                        char* key=(char*)&geometry->parts[part];
                        char* map=*(char**)&AsCQuadTree(root)->courseData+0xAC;
                        char* node; unsigned char left,right;
                        char* where=(char*)CourseMaterialMap_FindInsertionPosition(map,key,&node,&left,&right);
                        char* value=node;
                        int keyValue;
                        if(!value || (keyValue=*(int*)&((MeshPart*)key)->materialKey,keyValue > ((MaterialKeyPair*)(value+=12))->key)) {
                            unsigned pairValue[2]; pairValue[0]=((MeshPart*)key)->materialKey; pairValue[1]=s_quadTreeDefaultMaterialIndex;
                            value=(char*)FxSprayTree_InsertNode(map,where,left,right,pairValue)+12;
                        }
                        char* material=*(char**)&((CourseMaterialTable*)*(char**)&AsCQuadTree(root)->courseData)->entries+((MaterialKeyPair*)value)->materialIndex*0x28;
                        ((CourseTriangle*)((char*)entry->triangles+triangleOffset))->materialFlags=((CourseMaterialSource*)material)->flags;
                    }
                    triangleOffset+=0x48;
                }
            }
        }
        releaseResource(resource);
    }
    // Choose a single full list or eight octants; animated types 1..3 use the single bucket.
    if(*(int*)&entry->puppet->triangleCount<kMinimumBucketPartitionTriangles || (unsigned char)(entry->motionType+255)<=2) {
        block=(char*)__nwa__FUl(0x20);
        if(block) {
            ArrayGuard guard; guard.block=block;
            block=(char*)__construct_new_array(block,(void*)CourseTriangleBucket_Init,(void*)Buffer_dtor,16,1);
            guard.block=0;
        }
        entry->buckets=(CourseTriangleBucket*)block; entry->singleBucket=1;
        char* list=(char*)__nwa__FUl(*(int*)&entry->puppet->triangleCount*4);
        *(char**)&((CourseTriangleBucket*)(char*)entry->buckets)->triangleIds=list;
        ((CourseTriangleBucket*)(char*)entry->buckets)->count=*(int*)&entry->puppet->triangleCount;
        for(int t=0;t<*(int*)&entry->puppet->triangleCount;t++) entry->buckets->triangleIds[t]=t;
        return;
    }
    block=(char*)__nwa__FUl(0x90);
    if(block) {
        ArrayGuard guard; guard.block=block;
        block=(char*)__construct_new_array(block,(void*)CourseTriangleBucket_Init,(void*)Buffer_dtor,16,8);
        guard.block=0;
    }
    entry->buckets=(CourseTriangleBucket*)block; entry->singleBucket=0;
    Vec3_Zero((char*)&entry->partitionCenter);
    int vertexCount=0;
    float vertex[3]={kFloat_0_805DF744,kFloat_0_805DF744,kFloat_0_805DF744};
    model=(char*)entry->puppet;
    for(unsigned mesh=0;mesh<((WPuppet*)model)->namedBoneCount;mesh++) {
        char* pair=(*(char**)&((WPuppet*)model)->namedBones)+mesh*8;
        if(!pair) continue;
        char* transform=((BoneNamePair*)pair)->bone;
        if(CString_Compare(((BoneNamePair*)pair)->nameRecord+0x2C,transform+8)) continue;
        if(!WPuppet_CreateMeshByBoneName((char*)entry->puppet,&resource,transform+8)) continue;
        int groups=IPetMesh_GetGroupCount(resource);
        for(int group=0;group<groups;group++) {
            // Keep byte indexing of parts: direct parts[part] swaps add operands in this compiler.
            MeshGroup* geometry=(MeshGroup*)IPetMesh_GetGroupByIndex(resource,(unsigned short)group);
            for(unsigned part=0;part<geometry->partCount;part++) {
                for(unsigned v=0;v<((MeshPart*)((char*)geometry->parts + part * sizeof(MeshPart)))->vertexCount;v+=3) {
                    for(int j=0;j<3;j++) {
                        memcpy(vertex,((MeshPart*)((char*)geometry->parts + part * sizeof(MeshPart)))->vertices+(v+j)*12,12);
                        float tmp[3];
                        Matrix_TransformPoint(tmp,vertex,transform+0x78);
                        __as__7WVectorFRC7WVector(vertex,tmp);
                        __apl__7WVectorFRC7WVector((char*)&entry->partitionCenter,vertex);
                        vertexCount++;
                    }
                }
            }
        }
        releaseResource(resource);
    }
    // Average world vertices to set the partition center, then count overlaps before allocation.
    Vec3_DivideAssign((char*)&entry->partitionCenter,(float)vertexCount);
    int counts[kOctantCount]={0,0,0,0,0,0,0,0};
    int hit[kOctantCount];
    int t=0;
    do {
        octants((char*)entry->triangles+t* kTriangleStride,(char*)entry,bounds,hit);
        for(int i=0;i<8;i++) if(hit[i]) counts[i]++;
    } while(++t<*(int*)&entry->puppet->triangleCount);
    int total=0;
    for(int i=0;i<8;i++) total+=counts[i];
    char* cursor=(char*)__nwa__FUl(total*4);
    for(int i=0;i<8;i++) {
        if(counts[i]!=0 || i==0) *(char**)&((CourseTriangleBucket*)((char*)entry->buckets+i*16))->triangleIds=cursor;
        else *(char**)&((CourseTriangleBucket*)((char*)entry->buckets+i*16))->triangleIds=0;
        cursor+=counts[i]*4;
    }
    // Each bucket now points at its slice; reset counts and populate IDs in increasing order.
    for(int i=0;i<8;i++) ((CourseTriangleBucket*)((char*)entry->buckets+i*16))->count=0;
    t=0;
    do {
        octants((char*)entry->triangles+t* kTriangleStride,(char*)entry,bounds,hit);
        for(int i=0;i<8;i++) if(hit[i]==1) {
            char* octant=(char*)entry->buckets+i*16;
            ((CourseTriangleBucket*)octant)->triangleIds[((CourseTriangleBucket*)octant)->count]=t;
            ((CourseTriangleBucket*)((char*)entry->buckets+i*16))->count++;
        }
    } while(++t<*(int*)&entry->puppet->triangleCount);
}
// Placement delete leaves caller-owned storage untouched.
inline void operator delete(void*, void*) {}
#pragma auto_inline off

// Rebuild the X/Z triangle grid, deduplicating triangle IDs in every intersected cell.
extern "C" void CourseTriangleGrid_Rebuild(void* self, void* arg) {
	Grid564* s = (Grid564*)self;
	float r[4];
	int used = s->used;
	s->world = (GridWorld564*)arg;
	s->used -= used;
	if (s->cells) {
		if (s->cells->list) {
			__dla__FPv(s->cells->list);
			s->cells->list = 0;
		}
	}
	if (s->cells) { __destroy_new_array(s->cells, Buffer_dtor); s->cells = 0; }
	// Old grid is released; derive dimensions/origin from ground bounds and allocate empty cells.
	s->floor = kFloat_100000_BallPhysics;
	GridWorld564* world = s->world;
	Vec564 pos(world->minX, 0.0f, world->minZ);
	*(WVector*)s->origin = *(WVector*)&pos;
	world = s->world;
	float grid = kFloat_20_QuadTree;
	s->nx = (int)((world->maxX - world->minX) / grid) + 1;
	s->nz = (int)((world->maxZ - world->minZ) / grid) + 1;
	int n = s->nz * s->nx;
	void* p = __nwa__FUl(n * 16 + 16);
	if (p) p = __construct_new_array(p, CourseTriangleBucket_Init, Buffer_dtor, 16, n);
	s->cells = (GridCell564*)p;
	int* counts = (int*)__nwa__FUl(s->nz * s->nx * 4);
	int* heads = (int*)__nwa__FUl(s->nz * s->nx * 4);
	int i, t;
	for (i = 0; i < s->nz * s->nx; ++i) counts[i] = 0;
	float zero = kFloat_0_805DF744;
	grid = kFloat_20_QuadTree;
	for (t = 0; t < s->world->batch->count; ++t) {
		GridTri564* tri = &s->world->tris[t];
		if (tri->flag) {
			if (abs564(tri->n[1], zero) > *(float*)s_quadTreeCollisionState)
				s->floor = min564(min564(s->floor, tri->v[0][1]), min564(tri->v[1][1], tri->v[2][1]));
		} else if (tri->n[1] > *(float*)s_quadTreeCollisionState)
			s->floor = min564(min564(s->floor, tri->v[0][1]), min564(tri->v[1][1], tri->v[2][1]));
		r[0] = min564(tri->v[0][0], min564(tri->v[1][0], tri->v[2][0]));
		r[2] = max564(tri->v[0][0], max564(tri->v[1][0], tri->v[2][0]));
		r[1] = min564(tri->v[0][2], min564(tri->v[1][2], tri->v[2][2]));
		r[3] = max564(tri->v[0][2], max564(tri->v[1][2], tri->v[2][2]));
		r[0] -= s->origin[0]; r[1] -= s->origin[2];
		r[2] -= s->origin[0]; r[3] -= s->origin[2];
		int x0 = (int)(r[0] / grid), x1 = (int)(r[2] / grid);
		int z0 = (int)(r[1] / grid), z1 = (int)(r[3] / grid);
		for (i = 0; i < s->nz * s->nx; ++i) heads[i] = 0;
		for (int z = z0; z <= z1; ++z) {
			for (int x = x0; x <= x1; ++x) {
				if (heads[z * s->nx + x]) break;
				counts[z * s->nx + x]++;
				heads[z * s->nx + x] = 1;
			}
		}
	}
	// First pass counted triangle/cell overlaps; allocate one ID block and divide it into slices.
	int total = 0;
	for (i = 0; i < s->nz * s->nx; ++i) total += counts[i];
	int* cursor = (int*)__nwa__FUl(total * 4);
	for (i = 0; i < s->nz * s->nx; ++i) {
		if (counts[i] != 0 || i == 0) s->cells[i].list = cursor;
		else s->cells[i].list = 0;
		cursor += counts[i];
	}
	for (i = 0; i < s->nz * s->nx; ++i) s->cells[i].count = 0;
	zero = kFloat_0_805DF744;
	grid = kFloat_20_QuadTree;
	for (t = 0; t < s->world->batch->count; ++t) {
		GridTri564* tri = &s->world->tris[t];
		if (tri->flag) {
			if (abs564(tri->n[1], zero) > *(float*)s_quadTreeCollisionState)
				s->floor = min564(min564(s->floor, tri->v[0][1]), min564(tri->v[1][1], tri->v[2][1]));
		} else if (tri->n[1] > *(float*)s_quadTreeCollisionState)
			s->floor = min564(min564(s->floor, tri->v[0][1]), min564(tri->v[1][1], tri->v[2][1]));
		r[0] = min564(tri->v[0][0], min564(tri->v[1][0], tri->v[2][0]));
		r[2] = max564(tri->v[0][0], max564(tri->v[1][0], tri->v[2][0]));
		r[1] = min564(tri->v[0][2], min564(tri->v[1][2], tri->v[2][2]));
		r[3] = max564(tri->v[0][2], max564(tri->v[1][2], tri->v[2][2]));
		r[0] -= s->origin[0]; r[1] -= s->origin[2];
		r[2] -= s->origin[0]; r[3] -= s->origin[2];
		int x0 = (int)(r[0] / grid), x1 = (int)(r[2] / grid);
		int z0 = (int)(r[1] / grid), z1 = (int)(r[3] / grid);
		for (int z = z0; z <= z1; ++z) {
			for (int x = x0; x <= x1; ++x) {
				GridCell564& cell = s->cells[z * s->nx + x];
				if (cell.Find(t) >= cell.count) {
					cell.Add(t);
					s->cells[z * s->nx + x].count++;
				}
			}
		}
	}
	// Second pass filled deduplicated cell lists; release temporary counts and publish the floor.
	if (counts) __dla__FPv(counts);
	if (heads) __dla__FPv(heads);
	s->floor = max564(s->floor, s->world->y) - kFloat_1_QuadTree;
}

// Read the float at a required borrowed address.
extern "C" float Float_Get(float* value) {
	return * value;
}

// Find the highest acceptable candidate plane at X/Z, with optional two-sided inclusion and miss fallback.
// Uses existing candidates without querying. Y is the highest qualifying plane at X/Z,
// regardless of input Y. preservePointOnMiss returns the input and writes -1 on a miss;
// otherwise the sentinel/floor fallback is retained and outTriangleIndex may be untouched.
extern "C" Vec3f (CQuadTree_FindHighestGroundPointInCandidates)(void* self, Vec3f point, int preservePointOnMiss, int* outTriangleIndex, int includeBackFaces) {
    Vec3f tmp = point;
    tmp.y = kFloat_neg100000_QuadTree;
    float negativeOne, minNormalY, zero;
    zero = kFloat_0_805DF744;
    negativeOne = kFloat_neg1_QuadTree;
    minNormalY = kFloat_0_1_BallPhysics;
    char* p = (char*)self;
    int found = 0;
    int i = 0;
    for (; i < AsCQuadTree(self)->candidateTriangleCount; p += 4, ++i) { // 0x801CEEB0: +0x201C
        int idx = *(int*)(p + 0x1C);
        char* obj = *(char**)&AsCQuadTree(self)->groundSurface;
        char* table = *(char**)(obj + 0xB0);
        char* item = table + idx * kTriangleStride;
        if (includeBackFaces == 0) {
            if (*(unsigned char*)(item + 0x34) != 0) continue;
        }
        if (*(unsigned char*)(item + 0x34) == 0) {
            if (*(float*)(item + 0x28) < zero) continue;
        }
        {
            float f0 = *(float*)(item + 0x28);
            f0 = f0 > zero ? f0 : -f0;
            if (f0 < minNormalY) continue;
            int ok = Triangle_ContainsPointXZ((float*)&tmp, (float*)item, (float*)(item + 0xC), (float*)(item + 0x18));
            if (!ok) continue;
            float f1c = negativeOne * FP_FMADDS(tmp.z, *(float*)(item + 0x2C),
                FP_FMADDS(tmp.x, *(float*)(item + 0x24), *(float*)(item + 0x30))) / *(float*)(item + 0x28);
            if (f1c > tmp.y) {
                tmp.y = f1c;
                found = 1;
                if (outTriangleIndex != 0) {
                    *(int*)outTriangleIndex = idx;
                }
            }
        }
    }
    if (found == 0) {
        if (preservePointOnMiss != 0) {
            if (outTriangleIndex != 0) *outTriangleIndex = -1;
            return point;
        } else {
            self = *(void**)&AsCQuadTree(self)->groundSurface;
            void* r = GolfBall__Fv();
            if (((CGolfBall*)r)->m_pos.y < ((CourseSurface*)self)->boundsMin.y) {
                tmp.y = ((CourseSurface*)self)->boundsMin.y;
            }
        }
    }
    return tmp;
}

// Clamp a point to the playable region and ground, retaining the camera's required clearance.
extern "C" void CQuadTree_AdjustPointToRegionAndGroundClearance(void* outPoint, void* quadTree, void* referencePoint) {
	void* result = GolfBall__Fv();
	CourseData_ClampPointToActiveRegion(outPoint, *(void**)&((Vec3f*)quadTree)->y, referencePoint, result);
	const Vec3f& temp = GGP_BV(quadTree, *(Vec3f*)outPoint, 1, 0, 1);
	*(WVector*)outPoint = *(WVector*)((void*)&temp);
	void* manager = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	float y = *(float*)((char*)outPoint + 4);
	float delta = ((Vec3f*)referencePoint)->y - AsCGolfDoc(manager)->cameraGroundClearance;
	delta = y < delta ? delta : y;
	*(float*)((char*)outPoint + 4) = delta +AsCGolfDoc(manager)->cameraGroundClearance;
}

// Collect X/Z-containing candidates, select the closest forward plane, and optionally detect raised ground regions.
// Return a ground triangle ID, -1 for no selection, or -2 for a raised region when
// requested. A sole containing triangle wins without the multi-candidate distance test.
extern "C" int CQuadTree_GetMeshIndex(void* self, void* point, int checkRaisedRegions) {
	float initialBest = kFloat_100000_BallPhysics;
	int result = -1;
	int count = 0;
	int candidates[kMaxGroundCandidates];
	CQuadTree_GetMeshList(self, (float*)point);
	float zero = kFloat_0_805DF744;
	char* cursor = (char*)self;
	int boundsIndex;
	for (int i = 0; i < AsCQuadTree(self)->candidateTriangleCount; cursor += 4, ++i) {
		void* item = *(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles));
		item = (char*)item + AsCQuadTree(cursor)->candidateTriangleIds[0] * kTriangleStride;
		if (((CourseTriangle*)item)->twoSided || !(((CourseTriangle*)item)->normal.y < kFloat_0_805DF744)) {
			if (Triangle_ContainsPointXZ((float*)point, (float*)item, (float*)((char*)&((CourseTriangle*)item)->vertices[1]), (float*)((char*)&((CourseTriangle*)item)->vertices[2]))) {
				candidates[count++] = AsCQuadTree(cursor)->candidateTriangleIds[0];
				if (count >= kMaxGroundCandidates) break;
			}
		}
	}
	if (count == 1) result = candidates[0];
	else {
		float best = initialBest;
		for (int i = 0; i < count; ++i) {
			void* item = *(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles));
			item = (char*)item + candidates[i] * kTriangleStride;
			float distance = Plane_EvaluatePointReversed(point, (char*)&((CourseTriangle*)item)->normal);
			if (distance < kFloat_0_805DF744 && *(unsigned char*)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + candidates[i] * kTriangleStride + 0x34)) distance *= kFloat_neg1_QuadTree;
			if (distance > kFloat_0_805DF744 && distance < best) {
				best = distance;
				result = candidates[i];
			}
		}
	}
	if (checkRaisedRegions) {
		boundsIndex = 0;
		checkRaisedRegions = 0;
		for (; boundsIndex < (*(int*)&AsCQuadTree(self)->groundRegionCount); ++boundsIndex, checkRaisedRegions += 0x1C) {
			char* base2 = (*(char**)&AsCQuadTree(self)->groundRegions); // original +0x2078/+0x207C
			char* range = base2 + checkRaisedRegions;
			struct Bounds { float elems[6]; } bounds;
			bounds = *(Bounds*)range;
			float* elems = bounds.elems;
			if (((float*)point)[0] > elems[0] && ((float*)point)[0] < elems[3] && ((float*)point)[2] > elems[2] && ((float*)point)[2] < elems[5]) {
				float tmp[3];
				CQuadTree_FindHighestGroundPointInCandidates(tmp, self, point, 1, 0, 1);
				if (tmp[1] < elems[4]) { result = -2; break; }
			}
		}
	}
	return result;
}

// Project onto ground, optionally clamp raised regions, and enforce the ground surface's minimum Y.
// Returns a projected copy; optional outIndex is a triangle ID, -1 for floor/miss, -2
// for a raised-region clamp. useFloorOnMiss forces the surface floor for a missing ID.
extern "C" Vec3f (GetGroundPoint__9CQuadTreeF7WVectorbPib)(void* self, Vec3f pointValue, int useFloorOnMiss, int* outIndex, int clampRaisedRegions) {
	float* point = &pointValue.x;
	CQuadTree_GetMeshList(self, point);
	int result = -1;
	WVector temp;
	CQuadTree_FindHighestGroundPointInCandidates(&temp, self, point, (int)useFloorOnMiss, (int)&result, 1);
	*(WVector*)point = temp;
	if (useFloorOnMiss && result == -1)
		point[1] = *(float*)(*(char**)&AsCQuadTree(self)->groundSurface + 0x68);
	if (outIndex)
		*outIndex = result;
	if (clampRaisedRegions) {
		int count = (*(int*)&AsCQuadTree(self)->groundRegionCount); // original +0x207C
		for (int i = 0; i < count; ++i) {
			char* bounds = (*(char**)&AsCQuadTree(self)->groundRegions) + i * 0x1C; // original +0x2078
			if (point[0] > *(float*)bounds && point[0] < *(float*)(bounds + 0xC) &&
			    point[2] > *(float*)(bounds + 8) && point[2] < *(float*)(bounds + 0x14) &&
			    point[1] < *(float*)(bounds + 0x10)) {
				point[1] = *(float*)(bounds + 0x10);
				if (outIndex) *outIndex = -2;
				break;
			}
		}
	}
	if (point[1] < *(float*)(*(char**)&AsCQuadTree(self)->groundSurface + 0x68)) {
		point[1] = *(float*)(*(char**)&AsCQuadTree(self)->groundSurface + 0x68);
		if (outIndex) *outIndex = *outIndex < -1 ? *outIndex : -1;
	}
	return pointValue;
}


// Return the highest nonvertical candidate plane at this X/Z, or preserve Y when no plane qualifies.
extern "C" float CQuadTree_GetHighestGround(void* self, float* point, int* outIndex) {
	CQuadTree_GetMeshList(self, point);
	float neg1 = kFloat_neg1_QuadTree;
	float eps = kFloat_0_1_BallPhysics;
	float zero = kFloat_0_805DF744;
	float best = kFloat_neg100000_QuadTree;
	char* entry = (char*)self;
	int offset;
	int i = 0;
	int index;
	int found = 0;
	best = kFloat_neg100000_QuadTree;
	entry = (char*)self;
	zero = kFloat_0_805DF744;
	found = 0;
	neg1 = kFloat_neg1_QuadTree;
	i = 0;
	eps = kFloat_0_1_BallPhysics;
	while (i < AsCQuadTree(self)->candidateTriangleCount) {
		index = AsCQuadTree(entry)->candidateTriangleIds[0];
		offset = index * kTriangleStride;
		char* obj = *(char**)&AsCQuadTree(self)->groundSurface;
		char* item = *(char**)(obj + 0xB0) + offset;
		float absY = *(float*)(item + 0x28) > zero ? *(float*)(item + 0x28) : -*(float*)(item + 0x28);
		if (!(absY < eps) && (*(unsigned char*)(item + 0x34) || !(*(float*)(item + 0x28) < zero)) &&
		    Triangle_ContainsPointXZ(point, (float*)(item), (float*)(item + 0xC), (float*)(item + 0x18))) {
			found = 1;
			obj = *(char**)&AsCQuadTree(self)->groundSurface;
			item = *(char**)(obj + 0xB0) + offset;
			float num = FP_FMADDS(point[0], *(float*)(item + 0x24), *(float*)(item + 0x30));
			num = FP_FMADDS(point[2], *(float*)(item + 0x2C), num);
			float value = (neg1 * num) / *(float*)(item + 0x28);
			if (value > best) {
				if (outIndex)
					*outIndex = index;
				best = value;
			}
		}
		entry += 4;
		++i;
	}
	if (!found) {
		if (outIndex)
			*outIndex = -1;
		best = point[1];
	}
	return best;
}

// Return the lowest nonvertical candidate plane at this X/Z, or preserve the input Y on a miss.
extern "C" float CQuadTree_GetLowestGround(void* self, void* point) { CQuadTree_GetMeshList(self, (float*)point);
	float negativeOne, minNormalY, zero, lowestY;
	void* candidateCursor;
	int off, candidateIndex, foundGround;
	lowestY = kFloat_100000_BallPhysics;

	candidateCursor = self;
	foundGround = 0;
	candidateIndex = 0;
	zero = kFloat_0_805DF744;
	negativeOne = kFloat_neg1_QuadTree;
	minNormalY = kFloat_0_1_BallPhysics;
	while (candidateIndex < AsCQuadTree(self)->candidateTriangleCount) { int idx = AsCQuadTree(candidateCursor)->candidateTriangleIds[0];
	    void* obj = *(void**)&AsCQuadTree(self)->groundSurface;

		 off = idx * kTriangleStride;
		void* table = *(void**)&((CourseSurface*)obj)->triangles;
		 void* item = (char*)table + off;
		float normalY = ((CourseTriangle*)item)->normal.y;
		normalY = normalY > zero ? normalY : -normalY;
		if (!(normalY < minNormalY)) {

			if (0 != (((CourseTriangle*)item)->twoSided) || !(((CourseTriangle*)item)->normal.y < zero)) {
				if (Triangle_ContainsPointXZ((float*)point, (float*)item, (float*)((char*)&((CourseTriangle*)item)->vertices[1]), (float*)((char*)&((CourseTriangle*)item)->vertices[2]))) { float f4 = *(float*)point;

					foundGround = 1;

					void* obj2 = *(void**)&AsCQuadTree(self)->groundSurface;

					void* table2 = *(void**)&((CourseSurface*)obj2)->triangles; void* item2 = (char*)table2 + off;

					float f1r = FP_FMADDS((((Vec3f*)point)->z), (((CourseTriangle*)item2)->normal.z), FP_FMADDS(f4, (((CourseTriangle*)item2)->normal.x), (((CourseTriangle*)item2)->planeConstant)));

					float val = ((((negativeOne * f1r)))) / (((CourseTriangle*)item2)->normal.y);
					lowestY = lowestY > val ? val : lowestY;
				}
			}
		}
		candidateCursor = ((char*)&AsCQuadTree(candidateCursor)->courseData);
		candidateIndex++;
	}
	if (0 == foundGround) {
		lowestY = ((Vec3f*)point)->y;
	}
	return lowestY;
}

// Collect triangle IDs from the point's clamped X/Z grid cell.
extern "C" void CQuadTree_GetMeshList(void* self, float* point) {
	char* cells;
	int maxRow;
	int row;
	int col;
	int width;
	int maxCol;
	maxRow = AsCQuadTree(self)->gridRows - 1;
	col = (row = (int)((float)(kDouble_0_05_QuadTreeGrid * (double)(point[2] - AsCQuadTree(self)->gridOriginZ))),
	       row = row < 0 ? 0 : (row > maxRow ? maxRow : row),
	       width = AsCQuadTree(self)->gridColumns,
	       maxCol = width - 1,
	       cells = (*(char**)&AsCQuadTree(self)->cells),
	       (int)((float)(kDouble_0_05_QuadTreeGrid * (double)(point[0] - AsCQuadTree(self)->gridOriginX))));
	if (col < 0) col = 0;
	else if (col > maxCol) col = maxCol;
	char* cell = cells + (row * width + col) * 0x10;
	int* list = *(int**)cell;
	// A previously verified unique cell can be copied directly; otherwise check each ID.
	if (*(int*)(cell + 0xC)) {
		AsCQuadTree(self)->candidateTriangleCount = *(short*)(cell + 8);
		char* dest = (char*)self;
		for (int i = 0; i < AsCQuadTree(self)->candidateTriangleCount; ) {
			int value = *list;
			++list;
			++i;
			AsCQuadTree(dest)->candidateTriangleIds[0] = value;
			dest += 4;
		}
		return;
	}
	AsCQuadTree(self)->candidateTriangleCount = 0;
	*(int*)(cell + 0xC) = 1;
	for (int n = 0; n < *(short*)(cell + 8); ++n) {
		char* scan = (char*)self;
		int i = 0;
		for (; i < AsCQuadTree(self)->candidateTriangleCount; ++i) {
			if (*list == AsCQuadTree(scan)->candidateTriangleIds[0]) {
				*(int*)(cell + 0xC) = 0;
				break;
			}
			scan += 4;
		}
		if (i == AsCQuadTree(self)->candidateTriangleCount) {
			*(int*)((char*)self + AsCQuadTree(self)->candidateTriangleCount * 4 + 0x1C) = *list;
			++AsCQuadTree(self)->candidateTriangleCount;
		}
		++list;
	}
}

// Collect unique triangle IDs from grid cells covered by the segment's X/Z bounds.
// start is IN/OUT: subtract gridOrigin in place; delta remains a world displacement.
// The segment mask narrows the X/Z cell rectangle; candidate storage must fit all IDs.
extern "C" void CQuadTree_GetMeshListAlongSegment(void* self, void* start, void* __restrict delta) { int y; int x; int c; int j; int r; int i; char* s;

	float* d;
	int* grid;

	s = (char*)self; float* a;

	a = (float*)start;
	AsCQuadTree(s)->candidateTriangleCount = 0;
	*(WVector*)start -= *(WVector*)(s + 0x206C);
	d = (float*)delta;
	int x0 = fn_801D00F8_clamp((int)((d[0] > kFloat_0_805DF744 ? a[0] : a[0] + d[0]) / kFloat_20_QuadTree), 0, AsCQuadTree(s)->gridColumns - 1);
	int x1 = fn_801D00F8_clamp((int)((d[0] < kFloat_0_805DF744 ? a[0] : a[0] + d[0]) / kFloat_20_QuadTree), 0, AsCQuadTree(s)->gridColumns - 1);
	int y0 = fn_801D00F8_clamp((int)((d[2] > kFloat_0_805DF744 ? a[2] : a[2] + d[2]) / kFloat_20_QuadTree), 0, AsCQuadTree(s)->gridRows - 1);

	int y1 = fn_801D00F8_clamp((int)((d[2] < kFloat_0_805DF744 ? a[2] : a[2] + d[2]) / kFloat_20_QuadTree), 0, AsCQuadTree(s)->gridRows - 1);
	int cols; cols = x1 - x0 + 1;
	int rows = y1 - y0 + 1; grid = (int*)__alloca(rows * 4 * cols);
	float adx = (*(int*)&d[0] & 0x80000000) ? -d[0] : d[0];
	bool flat = adx < kFloat_0_00001_QuadTree;
	if (!(!(!(!(flat))))) {
		for (i = 0; rows > i; ++i) grid[i] = 1;
	} else {
		int* line = (int*)__alloca((1 + cols) * 4);
		for (c = 0; c < cols + 1; ++c) { float slope;
			float distance;

			distance = FP_FMSUBS(kFloat_20_QuadTree, (float)((c + x0)), a[0]);
			slope = d[2] / d[0];
			float height; height = FP_FMADDS(slope, distance, a[2]);
			unsigned int index; index = (int)(height / kFloat_20_QuadTree);
			line[c] = index - y0;
		}
		for (c = 0; c < cols; ++c)
			for (r = 0; rows > r; ++r)
				grid[cols * r + c] = (r >= (line[c] > line[c + 1] ? line[1 + c] : line[c]) && r <= (line[c] < line[c + 1] ? line[c + 1] : line[c])) ? 1 : 0;
	}
	for (y = 0; y < rows; ++y) for (x = 0; x < cols; ++x) {
		if (grid[cols * y + x] == 0) continue;
		for (j = 0; j < CELL564(x0 + x, y0 + y).count; ++j) {
			int count = AsCQuadTree(s)->candidateTriangleCount;
			char* scan = s;
			for (i = 0; i < count; ++i) {
				if (*(int*)(scan + 0x1C) == CELL564(x0 + x, y0 + y).list[j]) break;
				scan = scan + 4;
			}
			if (i == count) {
				*(int*)(s + 0x1C + (4 * count)) = CELL564((x0 + x), y0 + y).list[j];
				++AsCQuadTree(s)->candidateTriangleCount;
			}
		}
	}
}

// Collect unique triangle IDs from every grid cell overlapped by the supplied bounding box.
// bounds is IN/OUT [min.xyz,max.xyz]: both corners lose gridOrigin in place.
// Candidates are deduplicated into fixed scratch storage without a capacity check.
extern "C" void CQuadTree_GetMeshListInBounds(void* self, float* bounds) {
    struct Cell { int* list; int pad; short n; short pad2; int pad3; };
    struct G {
        char headerStorage[0xC];
        int x0, y0, x1, y1;
        int ids[(0x201C - 0x1C) / 4];
        int count;
        char candidateObjectStorage[0x2064 - 0x2020];
        int w, h;
        char bmin[0xC];
        char groundAndCollisionStorage[0x2148 - 0x2078];
        Cell* cells;
    };
    G* g = (G*)self;
    int x0 = 0;
    g->count = x0;
    *(WVector*)bounds -= *(WVector*)g->bmin;
    *(WVector*)((char*)(bounds + 3)) -= *(WVector*)g->bmin;
    int v = (int)(bounds[0] / kFloat_20_QuadTree);
    int max = g->w - 1;
    if (v >= 0) {
        if (v > max) v = max;
        x0 = v;
    }
    g->x0 = x0;
    g->x1 = fn_801D00F8_clamp((int)(bounds[3] / kFloat_20_QuadTree), 0, g->w - 1);
    g->y0 = fn_801D00F8_clamp((int)(bounds[2] / kFloat_20_QuadTree), 0, g->h - 1);
    g->y1 = fn_801D00F8_clamp((int)(bounds[5] / kFloat_20_QuadTree), 0, g->h - 1);
    for (int y = g->y0; y <= g->y1; ++y) {
        for (int x = g->x0; x <= g->x1; ++x) {
            for (int i = 0; i < g->cells[y * g->w + x].n; ++i) {
                int j;
                for (j = 0; j < g->count; ++j) {
                    if (g->ids[j] == g->cells[y * g->w + x].list[i]) break;
                }
                if (j == g->count) {
                    g->ids[g->count] = g->cells[y * g->w + x].list[i];
                    g->count++;
                }
            }
        }
    }
}

// Collect unique surface IDs from grid cells overlapped by the supplied bounding box.
// bounds is IN/OUT [min.xyz,max.xyz]: both corners lose gridOrigin in place.
// Grid IDs are deduplicated; shuttle/animation-only surface IDs are appended afterward.
extern "C" void CQuadTree_GetObjList(void* self, float* bounds) {
	struct Cell { int pad; short* list; short pad8; unsigned char count; char padB[5]; };
	struct G { char pad[12];
	    int x0,y0,x1,y1;
	    char pad1C[0x2020-0x1C];
	    short ids[32];
	    unsigned char count;
	    char pad2061[3];
	    int w, h;
	    char origin[12];
	    char pad2078[0x2148-0x2078];
	    Cell* cells;
	    };
	G* g = (G*)self;
	int j = 0;
	g->count = j;
	*(WVector*)bounds -= *(WVector*)g->origin;
	*(WVector*)((char*)(bounds + 3)) -= *(WVector*)g->origin;
	int v = (int)(bounds[0] / kFloat_20_QuadTree);
	int max = g->w - 1;
	j = (v < 0) ? j : (v > max ? max : v);
	AsCQuadTree(self)->minCellX = j;
	int va = (int)(bounds[3] / kFloat_20_QuadTree);
	int ma = g->w - 1;
	AsCQuadTree(self)->maxCellX = (va < 0) ? 0 : (va > ma ? ma : va);
	int vb = (int)(bounds[2] / kFloat_20_QuadTree);
	int mb = g->h - 1;
	AsCQuadTree(self)->minCellZ = (vb < 0) ? 0 : (vb > mb ? mb : vb);
	int vc = (int)(bounds[5] / kFloat_20_QuadTree);
	int mc = g->h - 1;
	AsCQuadTree(self)->maxCellZ = (vc < 0) ? 0 : (vc > mc ? mc : vc);
	int count;
	for (int y = g->y0; y <= g->y1; ++y) {
		for (int x = g->x0; x <= g->x1; ++x) {
			for (int i = 0; i < g->cells[y * g->w + x].count; ++i) {
				count = g->count;
				int j;
				for (j = 0; j < count; ++j)
					if (g->ids[j] == g->cells[y * g->w + x].list[i]) break;
				if (j == count) {
					g->ids[(unsigned char)count] = g->cells[y * g->w + x].list[i];
					++g->count;
				}
			}
		}
	}
	for (int i = 0; i < AsCQuadTree(self)->objectCount; ++i) {
		void* item = ((*(void***)&AsCQuadTree(self)->objects))[i];
		unsigned char state = *(unsigned char*)item;
		if (state == 1 || state == 3) {
			int count = AsCQuadTree(self)->candidateObjectCount;
			((short*)((char*)self + count * 2))[0x1010] = *(short*)((char*)item + 8);
			++AsCQuadTree(self)->candidateObjectCount;
		}
	}
}

// Return acos of the upward-facing normal's clamped Y; a negative triangle index gives zero.
// Return radians. index < 0 returns zero; nonnegative indices must be valid.
extern "C" float CQuadTree_GetGroundSlope(void* self, int index) {
	if (index < 0)
		return kFloat_0_805DF744;
	void* obj = *(void**)&AsCQuadTree(self)->groundSurface;
	void* table = *(void**)&((CourseSurface*)obj)->triangles;
	int offset = index * kTriangleStride;
	float value[3];
	*(WVector*)value = *(WVector*)((char*)table + offset + 0x24);
	obj = *(void**)&AsCQuadTree(self)->groundSurface;
	table = *(void**)&((CourseSurface*)obj)->triangles;
	char* item = (char*)table + offset;
	if ((unsigned char)item[0x34] != 0 && value[1] < kFloat_0_805DF744)
		Vec3_NegateInPlace(value);
	float y = value[1] < kFloat_neg1_QuadTree ? kFloat_neg1_QuadTree : (value[1] > kFloat_1_QuadTree ? kFloat_1_QuadTree : value[1]);
	float result = (float)acos(y);
	if (result > kFloat_0_805DF744)
		return result;
	return -result;
}

// Copy the triangle normal, flipping downward two-sided normals; a negative index resolves the ball's ground.
extern "C" void CQuadTree_GetMeshNormal(void* out, void* self, int index) {
	int value0[3];
	int value1[3];
	if (index < 0) {
		void* data = GolfBall__Fv();
		int idx = CQuadTree_GetMeshIndex(self, data, 0);
		if (idx < 0) {
			float zero = kFloat_0_805DF744;
			*(float*)out = zero;
			*((float*)out + 1) = zero;
			*((float*)out + 2) = zero;
			return;
		}
		char* base = *(char**)(*(char**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles));
		int t0 = *(int*)(base + idx * kTriangleStride + 0x24);
		int t1 = *(int*)(base + idx * kTriangleStride + 0x28);
		value1[0] = t0;
		value1[1] = t1;
		value1[2] = *(int*)(base + idx * kTriangleStride + 0x2C);
		char* obj2 = *(char**)&AsCQuadTree(self)->groundSurface;
		char* base2 = *(char**)(obj2 + 0xB0);
		char* item2 = base2 + idx * kTriangleStride;
		if ((unsigned char)item2[0x34] != 0) {
			if (*((float*)value1 + 1) < kFloat_0_805DF744) {
				Vec3_NegateInPlace(value1);
			}
		}
		int a = value1[0];
		int b = value1[1];
		*(int*)out = a;
		*((int*)out + 1) = b;
		*((int*)out + 2) = value1[2];
		return;
	}
	char* base = *(char**)(*(char**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles));
	int t0 = *(int*)(base + index * kTriangleStride + 0x24);
	int t1 = *(int*)(base + index * kTriangleStride + 0x28);
	value0[0] = t0;
	value0[1] = t1;
	value0[2] = *(int*)(base + index * kTriangleStride + 0x2C);
	char* obj2 = *(char**)&AsCQuadTree(self)->groundSurface;
	char* base2 = *(char**)(obj2 + 0xB0);
	char* item2 = base2 + index * kTriangleStride;
	if ((unsigned char)item2[0x34] != 0) {
		if (*((float*)value0 + 1) < kFloat_0_805DF744) {
			Vec3_NegateInPlace(value0);
		}
	}
	int a = value0[0];
	int b = value0[1];
	*(int*)out = a;
	*((int*)out + 1) = b;
	*((int*)out + 2) = value0[2];
}

// Find the ground normal at a point, returning a zero vector when no triangle qualifies.
extern "C" void CQuadTree_GetMeshNormalAtPoint(void* out, void* self, void* arg) {
	int index = CQuadTree_GetMeshIndex(self, arg, 0);
	if (index < 0) {
		float z = kFloat_0_805DF744;
		((float*)out)[0] = z;
		((float*)out)[1] = z;
		((float*)out)[2] = z;
		return;
	}
	void* obj = *(void**)&AsCQuadTree(self)->groundSurface;
	void* table = *(void**)&((CourseSurface*)obj)->triangles;
	unsigned char* item = (unsigned char*)table + index * kTriangleStride;
	float value[3];
	*(int*)&value[0] = *(int*)(item + 0x24);
	*(int*)&value[1] = *(int*)(item + 0x28);
	*(int*)&value[2] = *(int*)(item + 0x2C);
	void* obj2 = *(void**)&AsCQuadTree(self)->groundSurface;
	void* table2 = *(void**)&((CourseSurface*)obj2)->triangles;
	unsigned char* item2 = (unsigned char*)table2 + index * kTriangleStride;
	if (item2[0x34] && value[1] < kFloat_0_805DF744)
		Vec3_NegateInPlace(value);
	*(int*)out = *(int*)&value[0];
	*((int*)out + 1) = *(int*)&value[1];
	*((int*)out + 2) = *(int*)&value[2];
}

// Test whether all three projected edge cross products have the same sign, including edge points.
extern "C" int Triangle_ContainsPointXZ(const float* a, const float* b, const float* c, const float* d) {
    struct ProjectedOffset {
        float x, z;
        // Store an XZ edge offset in the original constructor argument order.
        ProjectedOffset(float zValue, float xValue) : x(xValue), z(zValue) {}
    };
    float z3, z2, z1, x3, x2, x1;
    float bx = b[0];
    float az, dx, ax;
    ax = a[0]; az = a[2];
    x1 = bx - ax;
    x2 = c[0] - ax;
    dx = d[0];
    x3 = dx - ax;
    z1 = b[2] - az;
    z2 = c[2] - az;
    z3 = d[2] - az;
    // Constructed XZ offsets retain the six original rounded scratch stores.
    ProjectedOffset v1(z1, x1);
    ProjectedOffset v2(z2, x2);
    ProjectedOffset v3(z3, x3);
    float cross1 = FP_FMSUBS(v1.z, v2.x, v1.x * v2.z);
    float cross2 = FP_FMSUBS(v2.z, v3.x, v2.x * v3.z);
    float cross3 = FP_FMSUBS(v3.z, v1.x, v3.x * v1.z);
    if (cross1 >= kFloat_0_805DF744 && cross2 >= kFloat_0_805DF744 && cross3 >= kFloat_0_805DF744) return 1;
    if (cross1 <= kFloat_0_805DF744 && cross2 <= kFloat_0_805DF744 && cross3 <= kFloat_0_805DF744) return 1;
    return 0;
}

// Change collision simulation mode and clear the cached collision-state word when it changes.
extern "C" void CQuadTree_SetSimulationMode(void* self, int simulationMode) {
	int* field = (int*)(((char*)&AsCQuadTree(self)->simulationMode));
	if (*field != simulationMode) {
		*(int*)(s_quadTreeCollisionState + 0xC) = 0;
		*field = simulationMode;
	}
}

// Sweep the ball against candidate course objects and retain the nearest hit and its surface metadata.
extern "C" float CQuadTree_FindNearestSweptBallObjectHit(void* self, float* box, float* direction, float* outHitPlane) {
    void* scratch;
    char* cursor;
    int best = -1;
    float scale = kFloat_1_QuadTree;
    int outIndex = -1;
    struct Vec0998 {
    float x,y,z;
    // Initialize the three-component sweep-extent scale vector.
    Vec0998(float a,float b,float c):x(a),y(b),z(c){}
        // Compute absolute sweep displacement and scale the unit vector by the global ball radius.
        void calc(float* ballRadiusVector, float* absDir, float* direction) {
            float radius = ((CGolfBall*)GolfBall__Fv())->m_radius;
            float z = direction[2] > kFloat_0_805DF744 ? direction[2] : -direction[2];
            float y = direction[1] > kFloat_0_805DF744 ? direction[1] : -direction[1];
            float x = direction[0] > kFloat_0_805DF744 ? direction[0] : -direction[0];
            absDir[0]=x; absDir[1]=y; absDir[2]=z;
            __ml__FfRC7WVector(ballRadiusVector, this, radius);
        }
    };
    float extent[3];
    float absDir[3];
    float ballRadiusVector[3];
    Vec0998 axis(kFloat_1_QuadTree, kFloat_1_QuadTree, kFloat_1_QuadTree);
    scratch = &axis;
    ((Vec0998*)scratch)->calc(ballRadiusVector, absDir, direction);
    __pl__FRC7WVectorRC7WVector(extent, absDir, ballRadiusVector);
    float expandedBounds[6];
    Vec3_Zero(expandedBounds);
    Vec3_Zero(expandedBounds + 3);
    float tempA[3], tempB[3];
    __mi__FRC7WVectorRC7WVector(tempA, box, extent);
    __as__7WVectorFRC7WVector(expandedBounds, tempA);
    __pl__FRC7WVectorRC7WVector(tempB, box, extent);
    __as__7WVectorFRC7WVector(expandedBounds + 3, tempB);
    struct Bounds0998 { int v[6]; };
    Bounds0998 bounds = *(Bounds0998*)expandedBounds;
    CQuadTree_GetObjList(self, (float*)&bounds);
    struct Tracker0998 {
        int unused[3]; int count; int indices[32];
        // Register a refreshed surface once in simulation scratch; full scratch still allows refresh.
        static int add(char* base, int index) {
            int n = *(int*)(base + 12);
            if ((unsigned int)n >= 32) n = 1;
            else {
                char* scan = base;
                for (int j = 0; j < n ||
                    (((int*)(s_quadTreeCollisionState+16))[n] = index,
                    ++*(int*)(s_quadTreeCollisionState+12), n=1, false); ++j, scan += 4)
                    if (index == *(int*)(scan + 16)) { n=0; break; }
            }
            return n;
        }
    };
    cursor = (char*)self;
    unsigned int i = 0;
    for (; i < AsCQuadTree(self)->candidateObjectCount; cursor += 2, ++i) {
        char* entry = (char*)AsCourseData801C((*(void**)&AsCQuadTree(self)->courseData))->surfaces + *(short*)(cursor + 0x2020) * kSurfaceStride;
        float a[3], b[3];
        __mi__FRC7WVectorRC7WVector(a, entry + 0x64, extent);
        __as__7WVectorFRC7WVector(expandedBounds, a);
        __pl__FRC7WVectorRC7WVector(b, entry + 0x70, extent);
        __as__7WVectorFRC7WVector(expandedBounds + 3, b);
        int inside = 0;
        if (expandedBounds[0] <= box[0] && box[0] <= expandedBounds[3] && expandedBounds[1] <= box[1] && box[1] <= expandedBounds[4] && expandedBounds[2] <= box[2] && box[2] <= expandedBounds[5]) inside = 1;
        if (!inside) continue;
        unsigned char state = *(unsigned char*)entry;
        if ((unsigned char)(state + 255) <= 2) {
            int add = 1;
            if (AsCQuadTree(self)->simulationMode) {
                add = Tracker0998::add(s_quadTreeCollisionState, *(short*)(cursor + 0x2020));
            }
            if (add) CQuadTree_MakeCollisionData(self, *(short*)(cursor + 0x2020));
        }
        if (CQuadTree_IntersectBallSweepWithObjectTriangles(self, direction, *(short*)(cursor + 0x2020), outHitPlane, &scale, &outIndex)) best = *(short*)(cursor + 0x2020);
    }
    if (best >= 0) {
        cursor = (char*)AsCourseData801C((*(void**)&AsCQuadTree(self)->courseData))->surfaces + best * kSurfaceStride;
        if (((CGolfBall*)GolfBall__Fv())->m_flags & 0x10) ++*(unsigned char*)(cursor + 0xB9);
        if (((CGolfBall*)GolfBall__Fv())->m_flags & kSpecialFlightB) {
            if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpecialFlightB] >= 0) {
                box = (float*)((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpecialFlightB];
                if (((CGolfBall*)GolfBall__Fv())->m_preDataCount > (int)box) ++*(unsigned char*)(cursor + 0xB9);
            }
        }
        AsCQuadTree(self)->lastHitObjectIndex = (short)best;
        AsCQuadTree(self)->lastHitTriangleIndex = outIndex;
        AsCQuadTree(self)->lastHitObjectType = *(unsigned char*)cursor;
        AsCQuadTree(self)->lastHitHistoryIndex = ((CGolfBall*)GolfBall__Fv())->m_preDataCount;
        self = *(void**)(*(char**)(cursor + 0xB0) + outIndex * kTriangleStride + 0x3C);
        *(void**)&((CGolfBall*)GolfBall__Fv())->m_contactMaterialIndex = self;
    }
    return scale;
}

// Resolve the point's ground material through the material-key tree; preserve the legacy empty-pool pointer on a miss.
extern "C" void* CQuadTree_GetTextureName(void* self, void* point) {
	int idx = CQuadTree_GetMeshIndex(self, point, 0);
	if (idx < 0) {
		return 0;
	}
	char* obj = *(char**)&AsCQuadTree(self)->groundSurface;
	int off = idx * kTriangleStride;
	char* base = *(char**)(obj + 0xB0);
	char* node = base + off;
	int materialKey = (*(unsigned int*)(node + 0x38)) & kMaterialKeyMask;
	char* treeNode;
	char* obj2 = *(char**)&AsCQuadTree(self)->courseData;
	char* materialTree = obj2 + 0xAC;
	treeNode = *(char**)(obj2 + 0xB0);
	char* lowerBound = materialTree + 4;
	while (treeNode != 0) {
		int val = *(int*)(treeNode + 0xC);
		if (val >= materialKey) {
			lowerBound = treeNode;
			treeNode = *(char**)(treeNode + 0x0);
		} else {
			treeNode = *(char**)(treeNode + 0x4);
		}
	}
	if (lowerBound == materialTree + 4 || materialKey < *(int*)(lowerBound + 0xC)) {
		lowerBound = materialTree + 4;
	}
	if (lowerBound == obj2 + 0xB0) {
		return courseObjectSoundStrings.pad_6D;
	}
	void* insertionNode;
	unsigned char insertLeft;
	unsigned char insertRight;
	int ret = CourseMaterialMap_FindInsertionPosition(materialTree, &materialKey, &insertionNode, &insertLeft, &insertRight);
	char* sptr = (char*)insertionNode;
	// Compare material keys using a strict signed integer order.
	struct Compare { static bool less(const int& value, const int& key) { return value < key; } };
	if (sptr == 0 || Compare::less(*(int*)(sptr += 0xC), materialKey)) {
		int data[2] = {materialKey, s_quadTreeDefaultMaterialIndex};
		sptr = (char*)FxSprayTree_InsertNode(materialTree, (void*)ret, insertLeft, insertRight, data) + 0xC;
	}
	// after: mappedPair = sptr (pointer to +0xc field)
	char* mappedPair = (char*)sptr;
	int idx2 = *(int*)(mappedPair + 0x4);
	char* obj3 = *(char**)&AsCQuadTree(self)->courseData;
	char* arr = *(char**)(obj3 + 0x8C);
	return arr + idx2 * 0x28;
}

// Refresh a surface's world-space collision vertices and plane equations from its named model bones.
extern "C" void CQuadTree_MakeCollisionData(void* self, int index) {
	char* entry = (char*)AsCourseData801C((*(void**)&AsCQuadTree(self)->courseData))->surfaces + index * kSurfaceStride;
	if (!*(void**)&((CourseSurface*)entry)->puppet) return;
	RC* found = 0;
	int dstOffset = 0;
	char* object;
	unsigned int a;
	int c;
	int g;
	unsigned int b;
	unsigned int i;
	char* list;
	int groups;
	list = *(char**)&((CourseSurface*)entry)->puppet;
	for (i = 0; i < *(unsigned int*)(list + 0x140); ++i) {
		char* item = *(char**)(list + 0x13C) + i * 8;
		if (item) {
			object = *(char**)item;
			if (CString_Compare(*(char**)(item + 4) + 0x2C, object + 8) == 0) {
				if (WPuppet_CreateMeshByBoneName(*(void**)&((CourseSurface*)entry)->puppet, &found, object + 8)) {
					groups = IPetMesh_GetGroupCount(found);
					for (g = 0; g < groups; ++g) {
						struct S { int pad; unsigned int n; char* pts; int pad2; };
						struct Grp { unsigned int n; S* s; };
						Grp* group = (Grp*)IPetMesh_GetGroupByIndex(found, g);
												for (a = 0; a < group->n; ++a) {
							for (b = 0; b < group->s[a].n; b += 3) {
								struct T { Vec3f v[3]; Vec3f n; float d; char pad[0x14]; };
								Vec3f nn, tmp, n, e2, e1, s;
								for (c = 0; c < 3; ++c) {
									memcpy(&(*(T**)&((CourseSurface*)entry)->triangles)[dstOffset].v[c], group->s[a].pts + (b + c) * 0xC, 0xC);
									((void (*)(void*, void*, void*))Matrix_TransformPoint)(&tmp, &(*(T**)&((CourseSurface*)entry)->triangles)[dstOffset].v[c], object + 0x78);
									__as__7WVectorFRC7WVector(&(*(T**)&((CourseSurface*)entry)->triangles)[dstOffset].v[c], &tmp);
								}
								T* tri = &(*(T**)&((CourseSurface*)entry)->triangles)[dstOffset];
								__mi__FRC7WVectorRC7WVector(&e1, &tri->v[0], &tri->v[1]);
								__mi__FRC7WVectorRC7WVector(&e2, &tri->v[2], &tri->v[1]);
								Vector_CrossProduct(&n, &e2, &e1);
								__as__7WVectorFRC7WVector(&nn, &n);
								Normalize__7WVectorFv(&nn);
								__as__7WVectorFRC7WVector(&(*(T**)&((CourseSurface*)entry)->triangles)[dstOffset].n, &nn);
								__ml__FfRC7WVector(&s, &(*(T**)&((CourseSurface*)entry)->triangles)[dstOffset].n, kFloat_neg1_QuadTree);
								(*(T**)&((CourseSurface*)entry)->triangles)[dstOffset].d = __ml__FRC7WVectorRC7WVector(&s, tri);
								++dstOffset;
							}
						}
					}
					RC* p = found;
					if (p && --p->ref == 0) {
						delete p;
					}
				}
			}
		}
	}
}

// Advance course motion at 30 Hz, cycle wind, update sound attenuation and apply model animation transforms.
extern "C" void CQuadTree_UpdateAnimatedObjects(void* self, float delta, int forceTick) {
	char worldMatrix[0x30], rotationMatrix[0x30];
	float range[14]; char path[16]; float difference[3], c[3], b[3], a[3];
	CourseAnimationState* animation = (CourseAnimationState*)self;
	unsigned int objectIndex;
	void* surface;
	void** list;
	void* motionPath;
	if (forceTick) ++animation->frame;
	else {
		float tick;
		float accumulated = animation->accum;
		tick = kFloat_0_033333335;
		accumulated += delta;
		animation->accum = accumulated;
		if (accumulated < tick) return;
		float step = delta;
		if (*(unsigned int*)&delta & 0x80000000) step = -step;
		if ((step < kFloat_0_00001_QuadTree) != 0) { step = 0; return; }
		unsigned int n = Runtime_ConvertDoubleToUnsigned(kFloat_30_CourseEffects * animation->accum);
		unsigned int next = animation->frame + n;
		float remaining = animation->accum - kFloat_0_033333335 * n;
		animation->frame = next;
		animation->accum = remaining;
	}
	// The 30 Hz clock is current: update wind override/effect gates on their 170-second cycle.
	void* game = CWind_GetInstance();
	if ((((CWind*)game)->overrideSpeed > kFloat_0_805DF744) != 0) {
		if (!(AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags & kWindUpdatesSuppressed)) {
			CWind_SetOverrideEnabled(CWind_GetInstance(), (animation->frame / kCourseFramesPerSecond % kWindCycleSeconds) >= kWindOverrideStart);
			if (animation->frame / kCourseFramesPerSecond % kWindCycleSeconds >= kWindOverrideSoundTime) { CWind_PlayCannonEndSoundOnce(CWind_GetInstance()); }
			if (!forceTick) {
				CWind_SetEffectEnabled(CWind_GetInstance(), (animation->frame / kCourseFramesPerSecond % kWindCycleSeconds) >= kWindEffectStart);
				if (animation->frame / kCourseFramesPerSecond % kWindCycleSeconds >= kWindEffectSoundTime) { CWind_PlayCannonChargeSoundOnce(CWind_GetInstance()); }
			}
		}
	}
			// Advance each surface at its phase-shifted time; sound attenuation uses world camera distance.
			for (objectIndex = 0; objectIndex < animation->count; ++objectIndex) {
		void** items = animation->items;
		void* motion = items[objectIndex];
		void* base = AsCourseData801C(animation->world)->surfaces; // 0x801D1100: course +0xA4
		surface = (char*)base + ((CourseMotionState*)motion)->surfaceIndex * kSurfaceStride;
		if (!*(int*)&((CourseSurface*)surface)->puppet) continue;
		if (!forceTick && ((CourseMotionState*)motion)->soundName[0]) {
			sprintf(path, courseObjectSoundStrings.soundAliasFormat, objectIndex);
			if (SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, path)) {
				float amount = kFloat_0_805DF744;
				if (AsCQuadTree(self)->objectSoundEnabled) {
					list = animation->items;
					__mi__FRC7WVectorRC7WVector(difference, (char*)&((CourseSurface*)surface)->position, s_cameraTransform + COURSE_OFFSET(CameraTransformView, position));
					amount = kFloat_1_QuadTree - WVectorLen__FRC7WVector(difference) / (kFloat_3_2_QuadTree * *(float*)((char*)list[objectIndex] + 0x30));
					amount = kFloat_0_805DF744 < amount ? amount : kFloat_0_805DF744;
				}
				SoundManager_SetNamedVolumeScale(g_soundManager, path, amount);
			} else *(signed char*)((char*)animation->items[objectIndex] + 0x10) = 0;
		}
		if (*(unsigned char*)surface == 4 && forceTick) continue;
		motion = animation->items[objectIndex];
		((CourseMotionState*)motion)->elapsedSeconds = kFloat_0_033333335 * (float)(animation->frame + ((CourseMotionState*)motion)->phaseFrames);
		int kind = *(unsigned char*)surface;
		switch (kind) {
		case 3: break;
		case 2: {
			list = animation->items;
			CourseMotionState* rotation = (CourseMotionState*)list[objectIndex];
			float v = ((CourseSurface*)surface)->windRotationScale;
			if (v > kFloat_0_805DF744) {
				float speed = CWind_GetWindSpeed(CWind_GetInstance());
				float angle = ((CourseSurface*)surface)->windRotationScale * (kFloat_3_1415927_CourseEffects * *(float*)((char*)list[objectIndex] + 0x0C));
				angle = angle * speed;
				rotation->angle = rotation->speed * angle;
			} else rotation->angle = v * (kFloat_neg3_1415927_QuadTree * rotation->elapsedSeconds);
			Matrix_SetAxisRotation(rotationMatrix, rotation->angle, 2);
			Matrix_ConcatAffine(worldMatrix, rotationMatrix, (char*)((CourseSurface*)surface)->transformAxes);
			Model_SetMatrixRecursive(*(void**)&((CourseSurface*)surface)->puppet, 0, worldMatrix);
			WPuppet_UpdateBoneWorldMatrices(*(void**)&((CourseSurface*)surface)->puppet, 0, 0, kFloat_1_QuadTree);
			continue;
		} case 1: {
			motionPath = animation->items[objectIndex];
			float time, sum, r;
			float rate = ((CourseMotionState*)motionPath)->elapsedSeconds / (((CourseMotionPath*)motionPath)->travelSeconds + ((CourseMotionPath*)motionPath)->pauseSeconds);
			r = floor(rate);
			time = *(float*)((char*)animation->items[objectIndex] + 0x0C);
			sum = ((CourseMotionPath*)motionPath)->travelSeconds + ((CourseMotionPath*)motionPath)->pauseSeconds;
			float p = time - sum * r;
			int idx = (int)(float)floor(time / sum) % 2;
			if (p > ((CourseMotionPath*)motionPath)->travelSeconds) __as__7WVectorFRC7WVector((char*)&((CourseSurface*)surface)->position, (char*)&((CourseMotionPath*)motionPath)->endpoints[1 - idx]);
			else { char* q = (char*)motionPath;
			    q += idx * 12;
			    void* start = q + 0x34;
			    __mi__FRC7WVectorRC7WVector(a, (char*)&((CourseMotionPath*)motionPath)->endpoints[1 - idx], start);
			    __ml__FfRC7WVector(b, a, p / ((CourseMotionPath*)motionPath)->travelSeconds);
			    __pl__FRC7WVectorRC7WVector(c, start, b);
			    __as__7WVectorFRC7WVector((char*)&((CourseSurface*)surface)->position, c);
			    }
		} break;
		}
		// Motion is applied; wrap animation time and dispatch events before rebuilding world bones.
		void* effect = *(void**)&((CourseSurface*)surface)->puppet;
		float duration = ((WPuppet*)effect)->animationDuration; // 0x801D1100: puppet +0x154
		if (duration > kFloat_0_805DF744) {
			float t = floor(*(float*)((char*)animation->items[objectIndex] + 0x0C) / duration);
			float animationLength = ((WPuppet*)*(void**)&((CourseSurface*)surface)->puppet)->animationDuration; // original +0x154
			void** list = animation->items;
			float elapsed = *(float*)((char*)list[objectIndex] + 0x0C) - t * animationLength;
			elapsed = kFloat_0_805DF744 < elapsed ? elapsed : kFloat_0_805DF744;
			if (!forceTick) { void* e = Update1100_effect(animation, objectIndex);
			    range[0] = kFloat_0_805DF744;
			    range[1] = elapsed;
			    AnimationEventParser_ProcessFrameRange(self, e, elapsed - delta, elapsed, range, objectIndex, 0);
			    }
			WPuppet_SetAnimationTimeFromRanges(*(void**)&((CourseSurface*)surface)->puppet, 0, elapsed, 0, 0);
			effect = *(void**)&((CourseSurface*)surface)->puppet;
			Model_SetMatrixRecursive(effect, effect, (char*)((CourseSurface*)surface)->transformAxes);
		} else Model_SetMatrixRecursive(effect, 0, (char*)((CourseSurface*)surface)->transformAxes);
		WPuppet_UpdateBoneWorldMatrices(*(void**)&((CourseSurface*)surface)->puppet, 0, 0, kFloat_1_QuadTree);
	}
}

// Borrow the current hole's cup center in world XYZ coordinates (+Y up).
static inline Vec3f& CurrentSimulationCupPosition() {
    return *(Vec3f*)(((char*)AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->holeRecords) +
        (AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentHoleIndex << 5) + 12);
}
// Advance one ball step through launch bounds, cup-rim checks and airborne/rolling/cup state handlers.
// Required initialized self, current GolfBall and active hole/course records.
// dt is simulation seconds: the golf_rule_round_flow_and_shot_sim.cpp caller creates one history record then
// passes kGolfSimulationStepSeconds (1/30 s). Finite positive dt is the caller's
// contract; no validation occurs here. Velocity is world units/second. This
// function changes global ball state/history and tree collision scratch; no
// return status/rollback, and early completion may skip the remaining phases.
// Editing/test map for a simulation-rate mod (presentation fps is separate):
//   dt -> swept displacement, acceleration integration in UpdateAirborneBall,
//         rolling/cup handlers and rotation; preserve FP expression order.
//   m_shotStage -> nearly-stationary history increments it; moving inside cup
//         also increments and holes ONLY when ++count == 60. Outside cup resets
//         only on that moving branch. Test stationary/moving, inside/outside,
//         counts 59/60/61; replacing equality with >= changes existing behavior.
//   m_preDataCount < 60 -> launch-bounds admission, latched launchBoundsExit
//         0=unclassified, 1=above, 2=below, 3=side. Test samples 59/60/61 and
//         bounds already classified; this threshold is HISTORY, not m_shotStage.
//   UpdateAirborneBall flag B -> hold while sample < event[2]+60; one history
//         sample can still be reserved before returning. Test -1 and +59/+60.
//   state SpinDelay -> exact event[7]+30 equality resumes rolling, records [8].
//         Check +29/+30/+31, including the second spin-delay path in airborne.
// These independent sample/count gates all assume the caller's sampling cadence.
// A future dt change must review the writer and these gates together; changing
// dt alone stretches their simulated durations. No thresholds are changed here.
extern "C" void CQuadTree_StepBallSimulation(void* self, float dt) {
    // Bit copies of the ball XYZ prefix; signedness preserves the existing word loads.
    struct Words179C { unsigned int xBits; int yBits; unsigned int zBits; };

    struct Bounds179C { char pad[0x214C]; float lower[3], upper[3]; };
// Stop179 encodes the holed flag and records the cup event; returns remain explicit below.
    float history[3], radial[3], origin[3], motion[3], predicted[3], zero[3];
    char* previousHistoryRecord;
    char* s=(char*)self;
    char* cur=(char*)GolfBall__Fv();
    *(unsigned char*)&AsCQuadTree(s)->groundCollisionSkipped=0;
    if(((CGolfBall*)cur)->m_state!=kBallSpinDelay) {
        int currentHistoryIndex=((CGolfBall*)cur)->m_preDataCount;
        int previousHistoryIndex, earlierHistoryIndex;
        previousHistoryIndex=currentHistoryIndex-1; earlierHistoryIndex=currentHistoryIndex-2;
        // m_preDataCount is the current record index, not the allocation length.
        // Saturate the two preceding indices at zero. The sign masks implement
        // max(index, 0); these bounded indices never approach INT_MIN.
        previousHistoryIndex &= (-previousHistoryIndex & ~previousHistoryIndex)>>31;
        earlierHistoryIndex &= (-earlierHistoryIndex & ~earlierHistoryIndex)>>31;
        previousHistoryRecord=(char*)((void*(*)(void*,int))PreData__9CGolfBallFi)(cur,previousHistoryIndex);
        void* earlierHistoryRecord=((void*(*)(void*,int))PreData__9CGolfBallFi)(cur,earlierHistoryIndex);
        __mi__FRC7WVectorRC7WVector(history,earlierHistoryRecord,previousHistoryRecord);
        zero[0]=kFloat_0_805DF744; zero[1]=kFloat_0_805DF744; zero[2]=kFloat_0_805DF744;
        if(Vec3_NearlyEqualWithTolerance(history,zero,kFloat_0_00001_QuadTree)) ++((CGolfBall*)cur)->m_shotStage;
        else {
            float offset[3]; Vec3f center=CurrentSimulationCupPosition();
            __mi__FRC7WVectorRC7WVector(offset,cur,&center);
            __as__7WVectorFRC7WVector(history,offset); history[1]=kFloat_0_805DF744;
            if(Magnitude__7WVectorCFv(history)<AsCQuadTree(s)->cupRadius) {
                if(++((CGolfBall*)cur)->m_shotStage==kCupSettleCallThreshold) {
                    Vec3f center2=CurrentSimulationCupPosition();
                    __as__7WVectorFRC7WVector(cur,&center2);
                    float radius=((CGolfBall*)cur)->m_radius;
                    ((CGolfBall*)cur)->m_pos.y=((QuadTreeGeometryState*)s)->cupFloorY+radius;
                    Stop179();
                    return;
                }
            } else ((CGolfBall*)cur)->m_shotStage=0;
        }
    }
    // Handle stationary/cup-floor completion first, then refresh ground type and launch-bound exit.
    Words179C point=*(Words179C*)cur;
    ((CGolfBall*)cur)->m_groundType=GetGroundType__9CQuadTreeF7WVector(self,&point);
    Vec3f center=CurrentSimulationCupPosition();
    __mi__FRC7WVectorRC7WVector(radial,cur,&center); radial[1]=kFloat_0_805DF744;
    if(Magnitude__7WVectorCFv(radial)<AsCQuadTree(s)->cupRadius) {
        float y, radius;
        radius=((CGolfBall*)cur)->m_radius;
        y=((CGolfBall*)cur)->m_pos.y;
        float floor=((QuadTreeGeometryState*)s)->cupFloorY+radius;
        if(y<floor) {
            ((CGolfBall*)cur)->m_pos.y=floor;
            Stop179();
            return;
        }
    }
    if(((CGolfBall*)cur)->m_pos.y<((CourseSurface*)*(char**)&AsCQuadTree(s)->groundSurface)->boundsMin.y) {
        Vec3_Zero(cur + COURSE_OFFSET(CGolfBall, m_velocity)); ((CGolfBall*)cur)->m_recoveryRequired=1; ((CGolfBall*)cur)->m_state=kBallStopped; return;
    }
    if(((CGolfBall*)cur)->m_preDataCount<kLaunchBoundsHistorySamples && ((QuadTreeGeometryState*)s)->launchBoundsExit==0) {
        *(Words179C*)origin=*(Words179C*)cur;
        float boundsSum[3]; __pl__FRC7WVectorRC7WVector(boundsSum,s+0x214C,s+0x2158);
        float sumY=boundsSum[1]; origin[1]=kFloat_0_5_QuadTree*sumY;
        __ml__FRC7WVectorf(motion,cur + COURSE_OFFSET(CGolfBall, m_velocity),dt);
        float fraction, step;
        float epsilon=kFloat_0_00001_QuadTree;
        float margin=kFloat_0_805DF744;
        motion[1]=margin;
        int axis=0;
        // Visit X, Z, then Y. A successful face test exits with its hit fraction.
        for (;; axis = axis==0 ? 2 : (axis==1 ? 3 : 1)) {
            // No earlier face crossing: exhaustion uses the full displacement (fraction = 1).
            if (axis >= 3) {
                fraction = kFloat_1_QuadTree;
                break;
            }
            float magnitude=(((unsigned int*)motion)[axis]&0x80000000)?-motion[axis]:motion[axis];
            int tooSmall=(magnitude<epsilon);
            if(!tooSmall) {
                float* component=&motion[axis]; step=*component;
                float boundary;
                float position;
                if(step>margin) {
                    position=origin[axis];
                    boundary=margin+((Bounds179C*)s)->upper[axis];
                    if(position>boundary || position+step<((Bounds179C*)s)->lower[axis]-margin) {
                        fraction=kFloat_1_QuadTree; break;
                    }
                    boundary=((Bounds179C*)s)->lower[axis]-margin;
                    if(position>boundary) continue;
                    fraction=(boundary-position)/step;
                } else {
                    position=origin[axis];
                    boundary=((Bounds179C*)s)->lower[axis]-margin;
                    if(position<boundary) { fraction=kFloat_1_QuadTree; break; }
                    boundary=margin+((Bounds179C*)s)->upper[axis];
                    if(position+step > boundary) { fraction=kFloat_1_QuadTree; break; }
                    if(position<boundary) continue;
                    fraction=(boundary-position)/step;
                }
                int other;
                for(other=0;other<3;++other) {
                    if(other!=axis) {
                        float hit=FP_FMADDS(fraction, motion[other], origin[other]);
                        if(hit<((float*)&((QuadTreeGeometryState*)s)->launchBoundsMin)[other]-margin || hit>margin+((float*)&((QuadTreeGeometryState*)s)->launchBoundsMax)[other]) break;
                    }
                }
                if(other==3) { position=0; break; }
            }
        }
        if(fraction<kFloat_1_QuadTree) {
            if(((CGolfBall*)cur)->m_pos.y<((QuadTreeGeometryState*)s)->launchBoundsMin.y) ((QuadTreeGeometryState*)s)->launchBoundsExit=2;
            else if(((CGolfBall*)cur)->m_pos.y>((QuadTreeGeometryState*)s)->launchBoundsMax.y) ((QuadTreeGeometryState*)s)->launchBoundsExit=1;
            else ((QuadTreeGeometryState*)s)->launchBoundsExit=3;
        }
    }
    if(!((int(*)(void*,void*))CQuadTree_IsSimulationEnabled)(self,cur) && Magnitude__7WVectorCFv(radial)>AsCQuadTree(s)->cupRadius) {
        Vec3_Zero(cur + COURSE_OFFSET(CGolfBall, m_velocity)); ((CGolfBall*)cur)->m_state=kBallStopped;
        float ground[3]; Words179C copy=*(Words179C*)cur;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(ground,self,(void**)&copy,(void*)1,0,0);
        __as__7WVectorFRC7WVector(cur,ground);
        float radius=((CGolfBall*)cur)->m_radius; ((CGolfBall*)cur)->m_pos.y=((CGolfBall*)cur)->m_pos.y+radius;
        AsCQuadTree(s)->candidateTriangleOverflow=1; return;
    }
    // Measure the ball step against the vertical cup ray; cache ray/segment fractions and distance.
    float hit[3], elevated[3]; Words179C center3; float height[3], up[3], travel[3];
    up[0]=kFloat_0_805DF744; up[1]=kFloat_1_QuadTree; up[2]=kFloat_0_805DF744;
    char* active=(char*)GolfBall__Fv();
    float activeRadius=((CGolfBall*)active)->m_radius;
    float elevatedY=kGroundOverrideHoleRadiusScale*activeRadius;
    height[0]=kFloat_0_805DF744; height[1]=elevatedY; height[2]=kFloat_0_805DF744;
    *(Arr3*)&center3 = *(Arr3*)&CurrentSimulationCupPosition();
    __ml__FRC7WVectorf(travel,cur + COURSE_OFFSET(CGolfBall, m_velocity),dt);
    __mi__FRC7WVectorRC7WVector(elevated,&center3,height);
    Geometry_ClosestRaySegmentParameters(hit,self,elevated,up,cur,travel);
    __as__7WVectorFRC7WVector(s+0x2174,hit);
    float bounceRadius;
    if(((CGolfBall*)cur)->m_eventPreDataIndex[9]<0 && ((CGolfBall*)cur)->m_eventPreDataIndex[kHistoryRollingStart]<0 && AsCQuadTree(s)->simulationMode==0 &&
        (bounceRadius=((CGolfBall*)cur)->m_radius, ((QuadTreeGeometryState*)s)->cupSweepParameters.z<(AsCQuadTree(s)->cupRadius-bounceRadius)) &&
        (!(((CGolfBall*)cur)->m_flags&kSpecialFlightA) || ((CGolfBall*)cur)->m_velocity.y<kFloat_0_805DF744)) {
        ((CGolfBall*)cur)->m_eventPreDataIndex[9]=((CGolfBall*)cur)->m_preDataCount;
        float delta[3], scaled[3]; __ml__FfRC7WVector(scaled,cur + COURSE_OFFSET(CGolfBall, m_velocity),((QuadTreeGeometryState*)s)->cupSweepParameters.y);
        __ml__FRC7WVectorf(delta,scaled,dt);
        __apl__7WVectorFRC7WVector(cur,delta);
        float difference[3]; Vec3f center4=CurrentSimulationCupPosition();
        __mi__FRC7WVectorRC7WVector(difference,&center4,cur);
        __as__7WVectorFRC7WVector(radial,difference); radial[1]=kFloat_0_805DF744;
        float radius=((CGolfBall*)cur)->m_radius;
        float horizontalZ, scale;
        scale=kFloat_0_1_BallPhysics+(kFloat_0_4_QuadTree*Magnitude__7WVectorCFv(radial))/(AsCQuadTree(s)->cupRadius-radius);
        float bounce[3];
        horizontalZ=scale*((CGolfBall*)cur)->m_velocity.z;
        float vertical=((CGolfBall*)cur)->m_velocity.y;
        vertical=vertical>kFloat_0_805DF744?vertical:-vertical;
        bounce[0]=scale*((CGolfBall*)cur)->m_velocity.x;
        bounce[2]=horizontalZ;
        bounce[1]=kFloat_neg2_BallPhysics*vertical;
        __as__7WVectorFRC7WVector(cur + COURSE_OFFSET(CGolfBall, m_velocity),bounce); return;
    }
    float delta[3]; __ml__FRC7WVectorf(delta,cur + COURSE_OFFSET(CGolfBall, m_velocity),dt);
    __pl__FRC7WVectorRC7WVector(predicted,cur,delta);
    float difference[3]; Vec3f center5=CurrentSimulationCupPosition();
    __mi__FRC7WVectorRC7WVector(difference,predicted,&center5);
    __as__7WVectorFRC7WVector(radial,difference); radial[1]=kFloat_0_805DF744;
    float currentRadius=((CGolfBall*)cur)->m_radius;
    if(((QuadTreeGeometryState*)s)->cupSweepParameters.z<AsCQuadTree(s)->cupRadius+currentRadius) {
        if(((CGolfBall*)cur)->m_state==kBallRolling && Magnitude__7WVectorCFv(cur + COURSE_OFFSET(CGolfBall, m_velocity))>kFloat_12_QuadTreeGeometry) {
            float reverse[3]; __ml__FfRC7WVector(reverse,radial,kFloat_neg1_QuadTree);
            float angle=Vector_SignedAngleXZ(reverse,cur + COURSE_OFFSET(CGolfBall, m_velocity));
            angle=angle>kFloat_0_805DF744?angle:-angle;
            if(angle<kFloat_1_0471976_QuadTree) {
                ((CGolfBall*)cur)->m_spin=kFloat_0_805DF744; ((CGolfBall*)cur)->m_curve=kFloat_0_805DF744;
                float speed=kFloat_0_5_QuadTree*Magnitude__7WVectorCFv(cur + COURSE_OFFSET(CGolfBall, m_velocity));
                ((CGolfBall*)cur)->m_velocity.y=speed>kFloat_5_QuadTreeGeometry?kFloat_5_QuadTreeGeometry:speed;
                float bounce[3]; __ml__FRC7WVectorf(bounce,cur + COURSE_OFFSET(CGolfBall, m_velocity),dt);
                __apl__7WVectorFRC7WVector(cur,bounce);
                ((CGolfBall*)cur)->m_state=kBallAirborne; ((BallGroundTriangleView*)cur)->groundTriangleIndex=-1;
                char* state=(char*)CGolfBall_GetOrAllocatePreDataAt(cur,((CGolfBall*)cur)->m_preDataCount);
                ((BallCollisionHistory*)state)->collisionType=2; ((BallCollisionHistory*)state)->eventType=3;
                float up2[3]; up2[0]=kFloat_0_805DF744; up2[1]=kFloat_1_QuadTree; up2[2]=kFloat_0_805DF744;
                __as__7WVectorFRC7WVector(state+0x64,up2);
                strcpy(state+0x20,courseObjectSoundStrings.woodCollisionSound);
                ((CGolfBall*)cur)->m_eventPreDataIndex[5]=((CGolfBall*)cur)->m_preDataCount; return;
            }
        }
        float ground[3]; Words179C copy=*(Words179C*)predicted;
        float radius=((CGolfBall*)cur)->m_radius;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(ground,self,(void**)&copy,0,0,0);
        float predictedY, threshold, epsilon, groundY;
        epsilon=kFloat_0_01_quadTree;
        groundY=ground[1]+radius;
        threshold=epsilon+groundY;
        predictedY=predicted[1];
        if(predictedY>threshold) {
            ((BallGroundTriangleView*)cur)->groundTriangleIndex=-1; ((CGolfBall*)cur)->m_state=kBallAirborne;
        } else if(((QuadTreeGeometryState*)s)->cupSweepParameters.z<AsCQuadTree(s)->cupRadius) {
            ((CGolfBall*)cur)->m_state=kBallInCup;
            if(((CGolfBall*)cur)->m_eventPreDataIndex[4]<0) ((CGolfBall*)cur)->m_eventPreDataIndex[4]=((CGolfBall*)cur)->m_preDataCount;
        }
    } else {
        ((QuadTreeGeometryState*)s)->cupContactCounter=0;
        if(((CGolfBall*)cur)->m_state==kBallInCup) {
            float ground[3]; Words179C copy=*(Words179C*)predicted;
            float radius=((CGolfBall*)cur)->m_radius;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(ground,self,(void**)&copy,0,0,0);
            float predictedY, threshold, epsilon, groundY;
            epsilon=kFloat_0_01_quadTree;
            groundY=ground[1]+radius;
            threshold=epsilon+groundY;
            predictedY=predicted[1];
            if(predictedY>threshold) ((CGolfBall*)cur)->m_state=kBallAirborne;
            else {
                float ground2[3]; Words179C copy2=*(Words179C*)cur;
                float radius2=((CGolfBall*)cur)->m_radius;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(ground2,self,(void**)&copy2,0,0,0);
                ((CGolfBall*)cur)->m_pos.y=ground2[1]+radius2; ((CGolfBall*)cur)->m_state=kBallRolling;
            }
            ((CGolfBall*)cur)->m_eventPreDataIndex[5]=((CGolfBall*)cur)->m_preDataCount;
        }
    }
    // The cup/rim transitions are resolved: dispatch exactly one remaining state handler.
    switch(((CGolfBall*)cur)->m_state) {
    case kBallAirborne: CQuadTree_UpdateAirborneBall(self,dt); break;
    case kBallSpinDelay: {
        char* object=(char*)GolfBall__Fv();
        float spin=kFloat_62_831856_CourseEffects*((CGolfBall*)object)->m_initSpin;
        ((CGolfBall*)object)->m_rotation[1]=FP_FMADDS(spin, dt, ((CGolfBall*)object)->m_rotation[1]);
        if(((CGolfBall*)object)->m_preDataCount==((CGolfBall*)object)->m_eventPreDataIndex[kHistorySpinDelayStart]+kSpinDelayHistorySamples) {
            __apl__7WVectorFRC7WVector(object+12,object+0x4C);
            ((CGolfBall*)object)->m_eventPreDataIndex[kHistoryRollingStart]=((CGolfBall*)object)->m_preDataCount; ((CGolfBall*)object)->m_state=kBallRolling;
            CQuadTree_UpdateBallTriangleCollision(self,dt);
        }
        break;
    }
    case kBallRolling: CQuadTree_UpdateBallTriangleCollision(self,dt); break;
    case kBallInCup: CQuadTree_UpdateBallCupCollision(self,dt); break;
    }
#undef STOP179
#undef CENTER179
}

// Wii simulation gate: always return enabled; the caller retains a disabled-simulation fallback.
extern "C" int CQuadTree_IsSimulationEnabled() {
	return 1;
}

// Selected-club tuning used by the airborne force model (game tuning units).
struct AirborneForceTuningView { char padToCurveScale[0x10]; float curveForceScale, spinLiftScale; };
struct AirborneForceParametersView { char padToTuning[4]; AirborneForceTuningView* tuning; };
// Combine curve, spin, wind, gravity and velocity drag while respecting special-flight event gates.
extern "C" WVector CQuadTree_ComputeAirborneBallForces(void* self) {
    WVector totalForce;
    // Phase 1: curve acts along normalized (-velocity.z, 0, velocity.x).
    // Preserve singleton reads and float operation order;
    // Scales are game tuning values.

    extern float kFloat_0_75_QuadTree, kFloat_99999_QuadTree, kFloat_0_02_QuadTree, kFloat_3_QuadTree, kFloat_0_00008_QuadTree;
    SetAirForceComponents(((float*)&totalForce), kFloat_0_805DF744);
    char* ball = (char*)GolfBall__Fv();
    float absoluteCurveOrSpin;
    if (*(int*)&((CGolfBall*)ball)->m_curve & kFloatSignMask) absoluteCurveOrSpin = -((CGolfBall*)ball)->m_curve;
    else absoluteCurveOrSpin = ((CGolfBall*)ball)->m_curve;
    if (!IsAirSettingMagnitudeBelow(absoluteCurveOrSpin, kFloat_0_00001_QuadTree) && ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpecialFlightA] < 0) {
        float forceContribution[3], horizontalPerpendicular[3];
        float velocityX = ((CGolfBall*)GolfBall__Fv())->m_velocity.x;
        float velocityZ = ((CGolfBall*)GolfBall__Fv())->m_velocity.z;
        float negativeVelocityZ = kFloat_neg1_QuadTree * velocityZ;
        horizontalPerpendicular[2] = velocityX;
        horizontalPerpendicular[1] = kFloat_0_805DF744;
        horizontalPerpendicular[0] = negativeVelocityZ;
        void* parameterTable = InstanceTable_Get();
        float tuningScale = ((AirborneForceParametersView*)parameterTable)->tuning->curveForceScale;
        void* normalizedPerpendicular = Normalize__7WVectorFv(horizontalPerpendicular);
        ball = (char*)GolfBall__Fv();
        float curveOrSpin = ((CGolfBall*)ball)->m_curve;
        __ml__FfRC7WVector(forceContribution, normalizedPerpendicular, ScaleCurveForce(kFloat_0_75_QuadTree * tuningScale, curveOrSpin));
        __apl__7WVectorFRC7WVector(((float*)&totalForce), forceContribution);
    } else {
        ball = (char*)GolfBall__Fv();
        if (*(int*)&((CGolfBall*)ball)->m_curve & kFloatSignMask) absoluteCurveOrSpin = -((CGolfBall*)ball)->m_curve;
        else absoluteCurveOrSpin = ((CGolfBall*)ball)->m_curve;
        if (!IsAirSettingMagnitudeBelow(absoluteCurveOrSpin, kFloat_0_00001_QuadTree) && ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[12] < 0) {
            float forceContribution[3], horizontalPerpendicular[3];
            float velocityX = ((CGolfBall*)GolfBall__Fv())->m_velocity.x;
            float velocityZ = ((CGolfBall*)GolfBall__Fv())->m_velocity.z;
            float negativeVelocityZ = kFloat_neg1_QuadTree * velocityZ;
            horizontalPerpendicular[2] = velocityX;
            horizontalPerpendicular[1] = kFloat_0_805DF744;
            horizontalPerpendicular[0] = negativeVelocityZ;
            void* parameterTable = InstanceTable_Get();
            float tuningScale = ((AirborneForceParametersView*)parameterTable)->tuning->curveForceScale;
            void* normalizedPerpendicular = Normalize__7WVectorFv(horizontalPerpendicular);
            ball = (char*)GolfBall__Fv();
            float curveOrSpin = ((CGolfBall*)ball)->m_curve;
            __ml__FfRC7WVector(forceContribution, normalizedPerpendicular, ScaleCurveForce(kFloat_0_75_QuadTree * tuningScale, curveOrSpin));
            __apl__7WVectorFRC7WVector(((float*)&totalForce), forceContribution);
        }
    }
    // Phase 2: special-flight gates can return the curve contribution alone.
    if ((((CGolfBall*)GolfBall__Fv())->m_flags & kSpecialFlightA) && ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpecialFlightA] < 0) return totalForce;
    if ((((CGolfBall*)GolfBall__Fv())->m_flags & kSpecialFlightB) && ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[12] < 0) return totalForce;
    // Phase 3: wind is scaled by 0.01 for special flight B, otherwise 0.02.
    if (AsCQuadTree(self)->windEnabled) {
        void* sharedContext = g_sharedDoc;
        void* golfDocument = ((void*)AsCSharedDoc(sharedContext)->golfDoc);
        if (!AsCGolfDoc(golfDocument)->powerShotMeterOverride) {
            void* forceBall = GolfBall__Fv();
            if (((CGolfBall*)forceBall)->m_flags & kSpecialFlightB) {
                forceBall = GolfBall__Fv();
                void* wind = CWind_GetInstance();
                float windContribution[3], windVector[3];
                CWind_GetWindVectorAtPoint(windVector, wind, forceBall, kFloat_99999_QuadTree);
                __ml__FfRC7WVector(windContribution, windVector, kFloat_0_01_quadTree);
                __apl__7WVectorFRC7WVector(((float*)&totalForce), windContribution);
            }
            else {
                forceBall = GolfBall__Fv();
                void* wind = CWind_GetInstance();
                float windContribution[3], windVector[3];
                CWind_GetWindVectorAtPoint(windVector, wind, forceBall, kFloat_99999_QuadTree);
                __ml__FfRC7WVector(windContribution, windVector, kFloat_0_02_QuadTree);
                __apl__7WVectorFRC7WVector(((float*)&totalForce), windContribution);
            }
        }
    }
    // Phase 4: subtract mass-scaled gravity, add vertical spin, then subtract speed-scaled drag.
    ball = (char*)GolfBall__Fv();
    float gravity = ((CGolfBall*)ball)->m_mass;
    FP_FNMSUBS_ACCUM(((float*)&totalForce)[1], kFloat_34_295296_QuadTree, gravity);
    ball = (char*)GolfBall__Fv();
    if (*(int*)&((CGolfBall*)ball)->m_spin & kFloatSignMask) absoluteCurveOrSpin = -((CGolfBall*)ball)->m_spin;
    else absoluteCurveOrSpin = ((CGolfBall*)ball)->m_spin;
    if (!IsAirSettingMagnitudeBelow(absoluteCurveOrSpin, kFloat_0_00001_QuadTree)) {
        void* parameterTable = InstanceTable_Get();
        float tuningScale = ((AirborneForceParametersView*)parameterTable)->tuning->spinLiftScale;
        ball = (char*)GolfBall__Fv();
        float curveOrSpin = ((CGolfBall*)ball)->m_spin;
        AddScaledSpinLift(((float*)&totalForce), kFloat_3_QuadTree * tuningScale, curveOrSpin);
    }
    void* forceBall = GolfBall__Fv();
    float* velocity = (float*)((char*)GolfBall__Fv() + 0xC);
    float speed = Magnitude__7WVectorCFv(velocity);
    float forceContribution[3];
    __ml__FfRC7WVector(forceContribution, (char*)forceBall + COURSE_OFFSET(CGolfBall, m_velocity), kFloat_0_00008_QuadTree * speed);
    __ami__7WVectorFRC7WVector(((float*)&totalForce), forceContribution);
    return totalForce;
}

// Search back toward the shot origin for safe ground, trying twelve rotated clearance offsets before falling back.
extern "C" void CQuadTree_FindBallRecoveryPosition(void* self, void* regionBounds) {
    if (((CGolfBall*)GolfBall__Fv())->m_lie == 1) {
        __as__7WVectorFRC7WVector((char*)&((QuadTreeGeometryState*)self)->recoveryPosition, PLAYER_POS(GolfBall__Fv()));
        return;
    }
    void* state2 = GolfBall__Fv();
    Vec3f origin;
    Vec3f nearest;
    Vec3f current;
    Vec3f v;
    __mi__FRC7WVectorRC7WVector(&origin, PLAYER_POS(GolfBall__Fv()), state2);
    origin.y = kFloat_0_805DF744;
    Normalize__7WVectorFv(&origin);
    __amu__7WVectorFf(&origin, kFloat_12_8_CourseEffects);
    Vec3f original = origin;
    Vec3f delta;
    float angleStep = kFloat_30_CourseEffects;
    void* candidate = TaskActorTable_FindByName(g_taskManager ? ((void*(*)(void*))Object_GetChild)(g_taskManager) : 0, courseObjectSoundStrings.playerActor);
    if (candidate) {
        int index = (int)AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer;
        void* info = InstanceTable_Get();
        char* base = (char*)candidate + index * 64;
        __as__7WVectorFRC7WVector(&v, base + 0x128 + *(unsigned char*)((char*)*(void**)((char*)info + 4) + 0xA) * 16);
        Vec3f scaled; __ml__FRC7WVectorf(&scaled, Normalize__7WVectorFv(&original), v.x);
        __as__7WVectorFRC7WVector(&original, &scaled);
    } else {
        Vec3f scaled; __ml__FRC7WVectorf(&scaled, Normalize__7WVectorFv(&original), kFloat_6_4_QuadTree);
        __as__7WVectorFRC7WVector(&original, &scaled);
    }
    __as__7WVectorFRC7WVector(&current, GolfBall__Fv());
    int attempts;
    for (;;) {
        __apl__7WVectorFRC7WVector(&current, &origin);
        { Vec3f point; Vec3f c = current; GetGroundPoint__9CQuadTreeF7WVectorbPib(&point, self, (void**)&c, 0, 0, 0); __as__7WVectorFRC7WVector(&nearest, &point); }
        void* p = PLAYER_POS(GolfBall__Fv());
        Vec3f vec2;
        Vec3f vec; __mi__FRC7WVectorRC7WVector(&vec, GolfBall__Fv(), p);
        float distance = WVectorLen__FRC7WVector(&vec);
        __mi__FRC7WVectorRC7WVector(&vec2, GolfBall__Fv(), &nearest);
        if (WVectorLen__FRC7WVector(&vec2) > distance) break;
        if (CQuadTree_GetMeshIndex(self, &nearest, 0) == -1) break;
        Vec3f rp = nearest;
        if (fn_801D28D4_region(self, &rp)) continue;
        Vec3f bounds; CQuadTree_FindHighestGroundPointInCandidates(&bounds, self, &nearest, 1, 0, 1);
        if (bounds.y > *(float*)((char*)regionBounds + COURSE_OFFSET(GroundRegion, boundsMax.y))) {
            if (CourseData_IsPointOutsidePlayableRegion((*(void**)&AsCQuadTree(self)->courseData), &nearest)) continue;
            const float& ry1 = fn_801D28D4_bound(self, CQuadTree_GetMeshIndex(self, &nearest, 0)).y; float lim1 = *(float*)s_quadTreeCollisionState;
            if (ry1 > lim1) {
                float angle = kFloat_0_805DF744;
                attempts = 0;
                do {
                    Vec3f rot[4]; Matrix_SetAxisRotation(rot, angle, 1);
                    *(WVector*)&nearest = GROUND28(self, *(WVector*)&current + ROT28(&original, rot));
                    const float& ry2 = fn_801D28D4_bound(self, CQuadTree_GetMeshIndex(self, &nearest, 0)).y; float lim2 = *(float*)s_quadTreeCollisionState;
                    if (ry2 < lim2) break;
                    if (((int (*)(void*, WVector, float))CQuadTree_OverlappedWithObject)(self, *(WVector*)&nearest, angle)) break;
                    __mi__FRC7WVectorRC7WVector(&delta, (void*)addr28(GROUND28(self, *(WVector*)&current)), &nearest); Normalize__7WVectorFv(&delta);
                    float absY = delta.y > kFloat_0_805DF744 ? delta.y : -delta.y;
                    float lim = *(float*)(s_quadTreeCollisionState + 8);
                    if (absY > lim) break;
                    ++attempts;
                    FP_FMADDS_ACCUM(angle, kFloat_0_017453292_QuadTree, angleStep);
                } while (attempts < kRecoveryDirectionCount);
                if (attempts == kRecoveryDirectionCount) {
                    // 0x801D28D4: exhausting recovery angles accepts the current position.
                    Q28w* q = (Q28w*)self;
                    q->p = *(WVector*)&current;
                    q->p = GROUND28(self, q->p);
                    AsCQuadTree(self)->recoveryPositionY += ((CGolfBall*)GolfBall__Fv())->m_radius;
                    return;
                }
            }
        }
    }
    __as__7WVectorFRC7WVector((char*)&((QuadTreeGeometryState*)self)->recoveryPosition, PLAYER_POS(GolfBall__Fv()));
}

// Integrate flight, sweep ground and objects, resolve water/recovery and special shots, and record collision effects.
extern "C" void CQuadTree_UpdateAirborneBall(void* self, float dt) {
    // Phase 1: reserve this frame's history and honor the special-flight event hold.
    // dt is caller simulation seconds (StepBallSimulation timing map);
    // contact fractions are dimensionless.
    char* globals=s_quadTreeCollisionState;
    char* s = (char*)self;
    char* obj = (char*)GolfBall__Fv();
    char* historyRecord = (char*)CGolfBall_GetOrAllocatePreDataAt(obj, ((CGolfBall*)obj)->m_preDataCount);
    if ((((CGolfBall*)GolfBall__Fv())->m_flags & kSpecialFlightB)) { if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpecialFlightB] >= 0) {
            int base = ((CGolfBall*)GolfBall__Fv())->m_preDataCount;
            if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpecialFlightB] + kSpecialFlightHoldSamples > base) return;
        } }
    float bounds[6], previousContactNormal[3], movement[3], endpoint[3], normal[3], contact[3], collision[4];
    float ballMass = ((CGolfBall*)obj)->m_mass;
    float velocityIncrement[3]; float acceleration[3];
    WVector netForce = CQuadTree_ComputeAirborneBallForces(self);
    float contactNormalVelocity[3], previousNormalProjection[3], groundedPosition[3], positionForGroundQuery[3], combinedVelocity[3], positionForGroundType[3], endpointForRegionTest[3], regionContactStep[3], horizontal[3], up[3];
    float edgeCenterAtHit[3]; float edgeContactStep[3]; float edgeToCenter[3]; float centerAtHit[3]; float centerContactStep[3]; float groundContactStep[3]; float objectPrimaryVelocity[3]; float objectSecondaryVelocity[3];
    float spinVelocityScaled[3], spinVelocity[3], initialHorizontalVelocity[3];
    // Phase 2: semi-implicit integration: velocity += (netForce / mass) * dt.
    float v, w; Vec3_DivideScalar(acceleration, ballMass, (float*)&netForce); __ml__FRC7WVectorf(velocityIncrement, acceleration, dt);
    __apl__7WVectorFRC7WVector(obj + COURSE_OFFSET(CGolfBall, m_velocity), velocityIncrement);
    if (((CGolfBall*)GolfBall__Fv())->m_flags & kSpecialFlightB) GolfBall__Fv();
    if (((CGolfBall*)obj)->m_lastContactIndex != -1) {
        char* world = *(char**)(4 + s);
        char* grid = *(char**)(world + 0xa4) + ((CGolfBall*)obj)->m_lastContactId * kSurfaceStride;
        char* tri = *(char**)&((CourseSurface*)grid)->triangles + ((CGolfBall*)obj)->m_lastContactIndex * kTriangleStride;
        *(Arr3*)previousContactNormal=*(Arr3*)&((CourseTriangle*)tri)->normal.x;

        __ml__FfRC7WVector(contactNormalVelocity, previousContactNormal, (__ml__FRC7WVectorRC7WVector(obj + COURSE_OFFSET(CGolfBall, m_velocity), previousContactNormal)));
        __ami__7WVectorFRC7WVector(obj + COURSE_OFFSET(CGolfBall, m_velocity), contactNormalVelocity);
        if (((CGolfBall*)obj)->m_velocity.y > kFloat_0_805DF744) { Vec3_Zero(obj + COURSE_OFFSET(CGolfBall, m_velocity)); ((CGolfBall*)obj)->m_state=kBallStopped; }
    }
    float spin=kFloat_12_566371_CourseEffects * ((CGolfBall*)obj)->m_curve;
    ((CGolfBall*)obj)->m_rotation[0]=FP_FMADDS(spin, dt, ((CGolfBall*)obj)->m_rotation[0]);
    if (((CGolfBall*)GolfBall__Fv())->m_flags&3) {
        float spin=kFloat_62_831856_CourseEffects * ((CGolfBall*)obj)->m_initSpin; ((CGolfBall*)obj)->m_rotation[1]=FP_FMADDS(spin, dt, ((CGolfBall*)obj)->m_rotation[1]);
    } else {
        float spin=kFloat_25_132742_QuadTree * ((CGolfBall*)obj)->m_spin; ((CGolfBall*)obj)->m_rotation[1]=FP_FMADDS(spin, dt, ((CGolfBall*)obj)->m_rotation[1]);
    }
    // A reversal against the previous contact normal transitions flight to rolling/spin delay.
    // History +0x64 is a direction/normal vector; it is not a previous position.
    if (((BallGroundTriangleView*)obj)->groundTriangleIndex >= 0) { if (__ml__FRC7WVectorRC7WVector(obj + COURSE_OFFSET(CGolfBall, m_velocity),(char*)CGolfBall_GetOrAllocatePreDataAt(obj, ((CGolfBall*)obj)->m_preDataCount-1)+0x64) < kFloat_0_805DF744) {
            char* prev = (char*)CGolfBall_GetOrAllocatePreDataAt(obj, ((CGolfBall*)obj)->m_preDataCount-1);
            float dot = __ml__FRC7WVectorRC7WVector(obj + COURSE_OFFSET(CGolfBall, m_velocity),(char*)CGolfBall_GetOrAllocatePreDataAt(obj, ((CGolfBall*)obj)->m_preDataCount-1)+0x64);
            __ml__FfRC7WVector(previousNormalProjection,prev+0x64,dot); __ami__7WVectorFRC7WVector(obj + COURSE_OFFSET(CGolfBall, m_velocity),previousNormalProjection);
            if (s_quadTreeInitialized == 0) {
                CQuadTree_Ctor(((char*)(globals+0xa0)));
                __register_global_object(((char*)(globals+0xa0)),(void*)CQuadTree_dtor,
                    ((char*)(globals+0x90)));
                s_quadTreeInitialized=1;
            }
            *(Arr3*)positionForGroundQuery=*(Arr3*)obj;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(groundedPosition,
                ((char*)(globals+0xa0)),
                (void**)positionForGroundQuery,0,0,0);
            __as__7WVectorFRC7WVector(obj,groundedPosition);
            float height=((CGolfBall*)obj)->m_radius; ((CGolfBall*)obj)->m_pos.y = ((CGolfBall*)obj)->m_pos.y + height;
            if (((CGolfBall*)obj)->m_flags&2) {
                char* vec=(char*)((void*(*)(void*,int))PreData__9CGolfBallFi)(GolfBall__Fv(),0);
                *(Arr3*)initialHorizontalVelocity=*(Arr3*)(vec+0xc);
                initialHorizontalVelocity[1]=kFloat_0_805DF744;
                float motion=ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
                __ml__FfRC7WVector(spinVelocityScaled,initialHorizontalVelocity,kFloat_neg40_CourseEffects * ((CGolfBall*)GolfBall__Fv())->m_initSpin);
                Vec3_DivideScalar(spinVelocity,motion,spinVelocityScaled); __as__7WVectorFRC7WVector((char*)GolfBall__Fv()+0x4c,spinVelocity);
                float speed=kFloat_1_5_CourseEffects * ((CGolfBall*)GolfBall__Fv())->m_initSpin; ((CGolfBall*)GolfBall__Fv())->m_spin=speed;
                ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpinDelayStart]=(((CGolfBall*)GolfBall__Fv())->m_preDataCount);
                ((CGolfBall*)GolfBall__Fv())->m_state=kBallSpinDelay;
                Vec3_Zero((char*)GolfBall__Fv()+0xc);
                ((CGolfBall*)GolfBall__Fv())->m_flags&=~0x73;
            } else { ((CGolfBall*)obj)->m_eventPreDataIndex[kHistoryRollingStart]=((CGolfBall*)obj)->m_preDataCount; ((CGolfBall*)obj)->m_state=kBallRolling; }
            char* item=*(char**)(*(char**)((8+s))+0xb0)+((BallGroundTriangleView*)obj)->groundTriangleIndex* kTriangleStride;
            char* name;
            unsigned int id=*(int*)(item+0x3c); if (AsCQuadTree(s)->simulationMode) name=&emptySimulationSurfaceName;
            else name=(char*)((void*(*)(void*,int))CourseData_GetEntryByIndex)(*(void**)(4+s),id)+0x65;
            __pl__FRC7WVectorRC7WVector(combinedVelocity,obj + COURSE_OFFSET(CGolfBall, m_velocity),obj+0x4c); float magnitude=Magnitude__7WVectorCFv(combinedVelocity);
            if (*name && magnitude>kFloat_20_QuadTree) strcpy(historyRecord+0x20,name);
            *(Arr3*)positionForGroundType=*(Arr3*)obj;

            int moving=0;
            if (GetGroundType__9CQuadTreeF7WVector(self,positionForGroundType)==2 && magnitude>kFloat_20_QuadTree) moving=1;
            if (moving) ((CGolfBall*)obj)->m_eventPreDataIndex[kHistoryFastGreenContact]=((CGolfBall*)obj)->m_preDataCount;
            switch (((CGolfBall*)obj)->m_state) {
            case kBallSpinDelay: {
                char* actor=(char*)GolfBall__Fv();
                float spin=kFloat_62_831856_CourseEffects * ((CGolfBall*)actor)->m_initSpin;
                ((CGolfBall*)actor)->m_rotation[1]=FP_FMADDS(spin, dt, ((CGolfBall*)actor)->m_rotation[1]);
                if (((CGolfBall*)actor)->m_preDataCount==((CGolfBall*)actor)->m_eventPreDataIndex[kHistorySpinDelayStart]+kSpinDelayHistorySamples) {
                    __apl__7WVectorFRC7WVector(actor + COURSE_OFFSET(CGolfBall, m_velocity),actor+0x4c);
                    ((CGolfBall*)actor)->m_eventPreDataIndex[kHistoryRollingStart]=((CGolfBall*)actor)->m_preDataCount;
                    ((CGolfBall*)actor)->m_state=kBallRolling;
                    CQuadTree_UpdateBallTriangleCollision(self,dt);
                }
                break;
            }
            case kBallRolling: CQuadTree_UpdateBallTriangleCollision(self,dt); break;
            }
            return;
        } }
    // Phase 3: sweep the integrated displacement; all contact parameters below are fractions.
    __ml__FRC7WVectorf(movement,obj + COURSE_OFFSET(CGolfBall, m_velocity),dt);
    float groundHitFraction=CQuadTree_SweepBallAgainstTriangles(self,obj,movement,(int*)(obj+0x28),0);
    __pl__FRC7WVectorRC7WVector(endpoint,obj,movement);
    Vec3_Zero(bounds); Vec3_Zero(3+bounds);
    int found=(movement[1]<kFloat_0_805DF744);
    if (found) {
        *(Words2F40*)endpointForRegionTest=*(Words2F40*)endpoint;
        for (unsigned int i=0;i<AsCQuadTree(s)->groundRegionCount || (found=0, false);++i) {
            char* box=*(char**)(s+0x2078)+i*0x1c;
            if (endpointForRegionTest[0]>*(float*)box && endpointForRegionTest[0]<*(float*)(box+0xc) && endpointForRegionTest[2]>*(float*)(box+8) && endpointForRegionTest[2]<*(float*)(box+0x14) && endpointForRegionTest[1]<*(float*)(box+0x10)) {
                AABB_Copy(bounds,box); unsigned int id=*(int*)(box+0x18); ((CGolfBall*)GolfBall__Fv())->m_lie=id; found=1; break;
            }
        }
    }
    // A descending endpoint enters a raised region: compare its top-plane fraction with ground contact.
    if (found) {
        float t=(kFloat_neg1_QuadTree * (((CGolfBall*)obj)->m_pos.y-bounds[4]))/movement[1];
        if (t<groundHitFraction) {
            __ml__FfRC7WVector(regionContactStep,movement,t); __apl__7WVectorFRC7WVector(obj,regionContactStep);
            if (((CGolfBall*)obj)->m_eventPreDataIndex[kHistorySlot1]<0) ((CGolfBall*)obj)->m_eventPreDataIndex[kHistorySlot1]=((CGolfBall*)obj)->m_preDataCount;
            ((CGolfBall*)obj)->m_flags&=~0x20;
            float length=Magnitude__7WVectorCFv(obj + COURSE_OFFSET(CGolfBall, m_velocity));
            float clamped;
            float ratio=kFloat_neg1_QuadTree * ((CGolfBall*)obj)->m_velocity.y/length;
            if (ratio<kFloat_0_805DF744) clamped=kFloat_0_805DF744;
            else if (ratio>kFloat_1_QuadTree) clamped=kFloat_1_QuadTree;
            else clamped=ratio;
            float angle=(float)acos(clamped);
            if (!(((CGolfBall*)obj)->m_flags&0x50)) { if (angle<kFloat_1_3962634_QuadTree) { if (((CGolfBall*)obj)->m_velocity.y<kFloat_neg10_QuadTree) {
                        float z=((CGolfBall*)obj)->m_velocity.z; horizontal[0]=((CGolfBall*)obj)->m_velocity.x; horizontal[1]=kFloat_0_805DF744; horizontal[2]=z;
                        if (Magnitude__7WVectorCFv(horizontal)>kFloat_10_QuadTree && (angle>kFloat_1_0471976_QuadTree || (((CGolfBall*)obj)->m_flags&1))) {
                            float one, speed, attenuation;
                            attenuation=kFloat_0_1_BallPhysics;
                            one=kFloat_1_QuadTree; ++AsCQuadTree(s)->regionBounceCount; ((CGolfBall*)obj)->m_lie=0;
                            speed=((CGolfBall*)obj)->m_initSpin;
                            __amu__7WVectorFf(obj + COURSE_OFFSET(CGolfBall, m_velocity), FP_FNMSUBS(attenuation, speed, one));
                            float bounceBase,vertical, bounceSpeed, bounce;
                            float bounceScale;
                            bounceScale=kFloat_0_1_BallPhysics;
                            bounceSpeed=((CGolfBall*)obj)->m_initSpin;
                            bounceBase=kFloat_0_75_QuadTree;
                            vertical=((CGolfBall*)obj)->m_velocity.y;
                            bounce=FP_FNMADDS(bounceScale, bounceSpeed, bounceBase);
                            bounceBase=kFloat_0_805DF744; bounceSpeed=kFloat_1_QuadTree;
                            vertical = vertical * bounce;
                            ((CGolfBall*)obj)->m_velocity.y=vertical;
                            historyRecord[0x60]=4; historyRecord[0x61]=4;
                            up[0]=bounceBase; up[1]=bounceSpeed; up[2]=bounceBase;
                            __as__7WVectorFRC7WVector(historyRecord+0x64,up); strcpy(historyRecord+0x20,courseObjectSoundStrings.waterCollisionSound); return;
                        }
                    } } }
            Vec3_Zero(obj + COURSE_OFFSET(CGolfBall, m_velocity)); ((CGolfBall*)obj)->m_state=kBallStopped; obj[0x58]=1; historyRecord[0x61]=4;
            strcpy(historyRecord+0x20,courseObjectSoundStrings.waterCollisionSound); CQuadTree_FindBallRecoveryPosition(self,bounds); return;
        }
        ((CGolfBall*)GolfBall__Fv())->m_lie=0;
    }
    // Ground wins: choose edge/plane normal, move to contact and record the bounce.
    if (groundHitFraction<kFloat_1_QuadTree) {
        char* tri=*(char**)(*(char**)(8+s)+0xb0)+((BallGroundTriangleView*)obj)->groundTriangleIndex* kTriangleStride;
        if (((CourseTriangle*)tri)->hitFeatures) {
            __ml__FfRC7WVector(edgeContactStep,movement,groundHitFraction); __pl__FRC7WVectorRC7WVector(edgeCenterAtHit,obj,edgeContactStep);
            Triangle_ClosestPointOnEdges(contact,self,edgeCenterAtHit,tri,tri+0xc,tri+0x18);
            __ml__FfRC7WVector(centerContactStep,movement,groundHitFraction); __pl__FRC7WVectorRC7WVector(centerAtHit,obj,centerContactStep);
            __mi__FRC7WVectorRC7WVector(edgeToCenter,centerAtHit,contact); __as__7WVectorFRC7WVector(normal,edgeToCenter); Normalize__7WVectorFv(normal);
        } else {
            *(WVector*)normal = ((TriW*)*(char**)(*(char**)(8+s)+0xb0))[((BallGroundTriangleView*)obj)->groundTriangleIndex].n;
            char* tri2=*(char**)(*(char**)(s+8)+0xb0)+((BallGroundTriangleView*)obj)->groundTriangleIndex* kTriangleStride;
            if (((CourseTriangle*)tri2)->twoSided && __ml__FRC7WVectorRC7WVector(normal,movement)>kFloat_0_805DF744) Vec3_NegateInPlace(normal);
        }
        char* tri2=*(char**)(*(char**)(s+8)+0xb0)+((BallGroundTriangleView*)obj)->groundTriangleIndex* kTriangleStride;
        ((CGolfBall*)obj)->m_contactMaterialIndex=((CourseTriangle*)tri2)->materialIndex;
        __ml__FfRC7WVector(groundContactStep,movement,groundHitFraction); __apl__7WVectorFRC7WVector(obj,groundContactStep); CQuadTree_HandleBallGroundBounce(self,normal);
        char* manager=((char*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        char* player=(char*)GolfPlayer_GetEntryByIndex(manager,AsCGolfDoc(manager)->currentPlayer);
        if (!*(unsigned char*)(player+0xae)) {
            float normalY; float threshold;
            normalY=normal[1]; threshold=*(float*)(globals+4);
            historyRecord[0x61]=normalY>threshold ? 1 : 2;
        }
        else historyRecord[0x61]=3;
        __as__7WVectorFRC7WVector(historyRecord+0x64,normal); historyRecord[0x60]=1; return;
    }
    // No ground contact: test course objects, otherwise consume the full displacement.
    float fraction=CQuadTree_FindNearestSweptBallObjectHit(self,(float*)obj,movement,collision);
    if (fraction<kFloat_1_QuadTree) {
        if (AsCQuadTree(s)->lastHitObjectIndex>=0 && *(unsigned char*)((char*)((void*(*)(void*,int))CourseData_GetEntryByIndex)(*(void**)(4+s),((CGolfBall*)obj)->m_contactMaterialIndex)+0x24)==0xb) {
            char* grid=*(char**)(*(char**)(4+s)+0xa4)+AsCQuadTree(s)->lastHitObjectIndex* kSurfaceStride;
            __apl__7WVectorFRC7WVector(obj,movement); historyRecord[0x60]=5;
            if (sscanf(grid+0xba,courseObjectSoundStrings.floatPairFormat,&v,&w)>=2) {
                float z=((CGolfBall*)obj)->m_velocity.z;
                __ml__FRC7WVectorf(objectPrimaryVelocity,grid+0x20,v+(float)__fabs((double)((CGolfBall*)obj)->m_velocity.y));
                __as__7WVectorFRC7WVector(obj + COURSE_OFFSET(CGolfBall, m_velocity),objectPrimaryVelocity);
                float negativeW=-w; double absZ=__fabs((double)z); float negativeZ=-(float)absZ;
                __ml__FRC7WVectorf(objectSecondaryVelocity,grid+0x2c,(negativeW + negativeZ)); __apl__7WVectorFRC7WVector(obj + COURSE_OFFSET(CGolfBall, m_velocity),objectSecondaryVelocity);
            }
            return;
        }
        CQuadTree_ApplyBallMaterialCollision((Self5EB0*)self,movement,collision,fraction);
        historyRecord[0x61]=2;
        historyRecord[0x60]=2;
        ((CGolfBall*)obj)->m_curve=kFloat_0_805DF744;
        ((CGolfBall*)obj)->m_spin=kFloat_0_805DF744;
        ((CGolfBall*)obj)->m_flags=0;
        return;
    }
    __apl__7WVectorFRC7WVector(obj,movement);
    if (((CGolfBall*)obj)->m_lastContactIndex!=-1) {
        char* grid=*(char**)(*(char**)((((char*)&AsCQuadTree(s)->courseData)))+0xa4)+((CGolfBall*)obj)->m_lastContactId* kSurfaceStride;
        char* tri=*(char**)&((CourseSurface*)grid)->triangles+((CGolfBall*)obj)->m_lastContactIndex* kTriangleStride;
        if (!Triangle_ContainsPointXZ((float*)obj,(float*)tri,(float*)(tri+0xc),(float*)(tri+0x18))) { ((CGolfBall*)obj)->m_lastContactId=-1;
            ((CGolfBall*)obj)->m_lastContactIndex=-1;
            }
    }
}

// Classify a face contact against one oriented edge; near-zero crosses accept grazing/degeneracy.
// Only reached edges initialize their zero vectors. Preserve this short-circuit evaluation order.
// Callers keep independent output arrays: packing them into an aggregate changes MWCC stack placement.
static inline void TestSweepFaceEdge(bool& inside, void* point, void* edgeStart, void* edgeEnd, void* oppositeVertex,
    float* pointCross,
    float* interiorCross,
    float* pointCrossTemporary,
    float* direction,
    float* pointOffset,
    float* interiorCrossTemporary,
    float* directionCopy,
    float* interiorOffset,
    float* zeroPointCross,
    float* zeroInteriorCross) {
    __mi__FRC7WVectorRC7WVector(pointOffset,point,edgeStart);
    __mi__FRC7WVectorRC7WVector(direction,edgeEnd,edgeStart);
    Vector_CrossProduct(pointCrossTemporary,direction,pointOffset);
    __as__7WVectorFRC7WVector(pointCross,pointCrossTemporary);
    __mi__FRC7WVectorRC7WVector(interiorOffset,oppositeVertex,edgeStart);
    __mi__FRC7WVectorRC7WVector(directionCopy,edgeEnd,edgeStart);
    Vector_CrossProduct(interiorCrossTemporary,directionCopy,interiorOffset);
    __as__7WVectorFRC7WVector(interiorCross,interiorCrossTemporary);
    // Preserve zero-vector initialization only on the tested edge paths.
    if (__ml__FRC7WVectorRC7WVector(pointCross,interiorCross) > kFloat_0_805DF744 ||
        (zeroPointCross[0]=kFloat_0_805DF744, zeroPointCross[1]=kFloat_0_805DF744, zeroPointCross[2]=kFloat_0_805DF744,
        Vec3_NearlyEqualWithTolerance(pointCross,zeroPointCross,kFloat_0_00001_QuadTree)) ||
        (zeroInteriorCross[0]=kFloat_0_805DF744, zeroInteriorCross[1]=kFloat_0_805DF744, zeroInteriorCross[2]=kFloat_0_805DF744,
        Vec3_NearlyEqualWithTolerance(interiorCross,zeroInteriorCross,kFloat_0_00001_QuadTree)))
        inside = true;
    else inside = false;
}

// Find the earliest swept-sphere contact against candidate triangle planes, edges and vertices.
// Return a fraction below 1 for a hit, 1 for no earlier hit; optional output ID starts
// at -1. queryMode=0 applies special-flight/cup exclusions; nonzero bypasses those gates.
// startPosition/displacement are borrowed world vectors; triangle hitFeatures is mutable scratch.
// Geometry roadmap (all tests in world space):
// 1. Query an AABB expanded by radius and absolute step; orient two-sided planes toward start.
// 2. Intersect the radius-offset center path with the plane. Motion away is rejected;
//    green penetration clamps to t=0. Crossing includes grazing (signed-distance product <= 0).
// 3. Apply special-flight/cup exclusions before three oriented edge half-space tests.
//    Each cross product is compared with the opposite vertex's cross product: positive
//    dot means the same side; a near-zero cross accepts an edge/degenerate grazing case.
// 4. An inside face wins directly; otherwise sweep against the closest boundary point.
// Only strictly smaller fractions replace best: ties retain the earlier candidate;
// a plane candidate tied with best skips its edge test too. hitFeatures=1 marks boundary hits.
extern "C" float CQuadTree_SweepBallAgainstTriangles(void* self, void* startPosition, float* displacement, int* outTriangleIndex, int queryMode) {
    struct Words { int x,y,z; };
    struct Box { Words lo,hi; };
    struct BoundsStore { Box box; Words lo,hi; } boundStore;
    float queryHalfExtent[3],trianglePlane[4],radiusOffsetStart[3],faceContactPoint[3],cupHorizontalOffset[3],closestEdgePoint[3];
    float absoluteDisplacement[3],radiusExtent[3],queryMinimum[3],queryMaximum[3],radiusOffsetStartCopy[3],normalRadiusOffset[3],faceContactPointCopy[3],faceContactStep[3];
    float centerToCup[3];
    Words reference;
    float centerAtContact[3],centerContactStep[3],radiusOffsetEnd[3];
    // Edge 3 workspace: independent outputs preserve vector-call storage and stack layout.
    float edge3PointCross[3],edge3InteriorCross[3],edge3PointCrossTemporary[3],edge3Direction[3],edge3PointOffset[3],edge3InteriorCrossTemporary[3],edge3DirectionCopy[3],edge3InteriorOffset[3],edge3ZeroPointCross[3],edge3ZeroInteriorCross[3];
    // Edge 2 workspace: independent outputs preserve vector-call storage and stack layout.
    float edge2PointCross[3],edge2InteriorCross[3],edge2PointCrossTemporary[3],edge2Direction[3],edge2PointOffset[3],edge2InteriorCrossTemporary[3],edge2DirectionCopy[3],edge2InteriorOffset[3],edge2ZeroPointCross[3],edge2ZeroInteriorCross[3];
    // Edge 1 workspace: independent outputs preserve vector-call storage and stack layout.
    float edge1PointCross[3],edge1InteriorCross[3],edge1PointCrossTemporary[3],edge1Direction[3],edge1PointOffset[3],edge1InteriorCrossTemporary[3],edge1DirectionCopy[3],edge1InteriorOffset[3],edge1ZeroPointCross[3],edge1ZeroInteriorCross[3];
    float planeHitFraction,best;
    void* base = GolfBall__Fv();
    float radius = ((CGolfBall*)base)->m_radius;
    float absoluteZ = displacement[2]; absoluteZ = absoluteZ > kFloat_0_805DF744 ? absoluteZ : -absoluteZ;
    float absoluteY = displacement[1]; absoluteY = absoluteY > kFloat_0_805DF744 ? absoluteY : -absoluteY;
    float absoluteX = displacement[0]; absoluteX = absoluteX > kFloat_0_805DF744 ? absoluteX : -absoluteX;
    absoluteDisplacement[0]=absoluteX; absoluteDisplacement[1]=absoluteY; absoluteDisplacement[2]=absoluteZ;
    __ml__FfRC7WVector(radiusExtent, s_quadTreeUnitScaleVector, radius);
    __pl__FRC7WVectorRC7WVector(queryHalfExtent, absoluteDisplacement, radiusExtent);
    Vec3_Zero(&boundStore.lo);
    Vec3_Zero(&boundStore.hi);
    __mi__FRC7WVectorRC7WVector(queryMinimum, startPosition, queryHalfExtent);
    __as__7WVectorFRC7WVector(&boundStore.lo, queryMinimum);
    __pl__FRC7WVectorRC7WVector(queryMaximum, startPosition, queryHalfExtent);
    __as__7WVectorFRC7WVector(&boundStore.hi, queryMaximum);
    boundStore.box.lo = boundStore.lo;
    boundStore.box.hi = boundStore.hi;
    // Broad phase uses a scratch bounds copy because the query converts it to grid space.
    CQuadTree_GetMeshListInBounds(self, (float*)&boundStore.box);
    best = kFloat_1_QuadTree;
    if (outTriangleIndex) *outTriangleIndex = -1;
    char* cursor=(char*)self;
    for (int candidateIndex = 0; candidateIndex < AsCQuadTree(self)->candidateTriangleCount; cursor+=4,++candidateIndex) {
        int index = AsCQuadTree(cursor)->candidateTriangleIds[0];
        char* rec = *(char**)(*(char**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + index * kTriangleStride;
        ((CourseTriangle*)rec)->hitFeatures = 0;
        Plane_Copy(trianglePlane, rec + 0x24);
        unsigned char twoSided = ((CourseTriangle*)rec)->twoSided;
        if (twoSided) {
            float d = Plane_EvaluatePointReversed(startPosition, trianglePlane);
            if (d < kFloat_0_805DF744) {
                Vec3_NegateInPlace(trianglePlane);
                trianglePlane[3] = trianglePlane[3] * kFloat_neg1_QuadTree;
            }
        }
        __ml__FfRC7WVector(normalRadiusOffset, trianglePlane, ((CGolfBall*)base)->m_radius);
        __mi__FRC7WVectorRC7WVector(radiusOffsetStartCopy, startPosition, normalRadiusOffset);
        __as__7WVectorFRC7WVector(radiusOffsetStart, radiusOffsetStartCopy);
        void* t = ((void* (*)(void*, void*))CourseData_GetEntryByIndex)((*(void**)&AsCQuadTree(self)->courseData), (void*)(long)((CourseTriangle*)rec)->materialIndex);
        unsigned char materialGroundType = ((unsigned char*)t)[0x24];
        int isGreen = (materialGroundType == 2) ? 1 : 0;
        float normalMovement = __ml__FRC7WVectorRC7WVector(displacement, trianglePlane);
        if (normalMovement > kFloat_0_805DF744) {
            planeHitFraction = kFloat_1_QuadTree;
        } else {
            planeHitFraction = Plane_EvaluatePointReversed(radiusOffsetStart, trianglePlane);
            if (isGreen && planeHitFraction < kFloat_0_805DF744) {
                planeHitFraction = kFloat_0_805DF744;
            } else {
                __pl__FRC7WVectorRC7WVector(radiusOffsetEnd, radiusOffsetStart, displacement);
                float end = Plane_EvaluatePointReversed(radiusOffsetEnd, trianglePlane);
                if (planeHitFraction * end <= kFloat_0_805DF744) {
                    end = end > kFloat_0_805DF744 ? end : -end;
                    float start = planeHitFraction > kFloat_0_805DF744 ? planeHitFraction : -planeHitFraction;
                    planeHitFraction = (planeHitFraction > kFloat_0_805DF744 ? planeHitFraction : -planeHitFraction) / (start + end);
                } else planeHitFraction = kFloat_1_QuadTree;
            }
        }
        if (!(planeHitFraction < best)) continue;
        __ml__FfRC7WVector(faceContactStep,displacement,planeHitFraction);
        __pl__FRC7WVectorRC7WVector(faceContactPointCopy,radiusOffsetStart,faceContactStep);
        __as__7WVectorFRC7WVector(faceContactPoint,faceContactPointCopy);
        if (queryMode == 0) {
            unsigned int flags = *(unsigned int*)&((CGolfBall*)base)->m_flags;
            if ((flags & kSpecialFlightA) && ((CGolfBall*)base)->m_eventPreDataIndex[kHistorySpecialFlightA] < 0) {
                float threshold=*(float*)s_quadTreeCollisionState;
                if (trianglePlane[1] > threshold) {
                    AsCQuadTree(self)->groundCollisionSkipped=1;
                    continue;
                }
            }
            if ((flags & kSpecialFlightB) && ((CGolfBall*)base)->m_eventPreDataIndex[12] < 0) {
                float threshold=*(float*)s_quadTreeCollisionState;
                if (trianglePlane[1] > threshold) {
                    AsCQuadTree(self)->groundCollisionSkipped=1;
                    continue;
                }
            }
            char* manager = ((char*)AsCSharedDoc(g_sharedDoc)->golfDoc);
            char* entry = ((char*)AsCGolfDoc(manager)->holeRecords) + (AsCGolfDoc(manager)->currentHoleIndex << 5);
            struct Pair { unsigned int words[2]; };
            *(Pair*)&reference = *(Pair*)(entry + 0xC);
            reference.z=*(int*)(entry+0x14);
            __ml__FfRC7WVector(centerContactStep,displacement,planeHitFraction);
            __pl__FRC7WVectorRC7WVector(centerAtContact,startPosition,centerContactStep);
            __mi__FRC7WVectorRC7WVector(centerToCup,&reference,centerAtContact);
            __as__7WVectorFRC7WVector(cupHorizontalOffset,centerToCup);
            cupHorizontalOffset[1]=kFloat_0_805DF744;
            if (Magnitude__7WVectorCFv(cupHorizontalOffset) < AsCQuadTree(self)->cupRadius) continue;
        }
        // Check three edge half-spaces. For each edge, compare cross(edge, point-offset)
        // against cross(edge, interior-vertex-offset); positive dot means the same side.
        // Degenerate zero cross products also pass within tolerance. A failed face test
        // falls back to the nearest edge/vertex sweep. The temporary/copy arrays preserve
        // each vector helper's original output storage and evaluation order.
        bool inside;
        {
            TestSweepFaceEdge(inside, faceContactPoint, rec+0xC, rec+0x18, rec,
                    edge1PointCross, edge1InteriorCross, edge1PointCrossTemporary, edge1Direction, edge1PointOffset,
                    edge1InteriorCrossTemporary, edge1DirectionCopy, edge1InteriorOffset, edge1ZeroPointCross, edge1ZeroInteriorCross);
        }
        // 0x801D3A9C: rejected edges share the false assignment below.
        do {
            if (!inside) continue;
            {
                TestSweepFaceEdge(inside, faceContactPoint, rec, rec+0x18, rec+0xC,
                    edge2PointCross, edge2InteriorCross, edge2PointCrossTemporary, edge2Direction, edge2PointOffset,
                    edge2InteriorCrossTemporary, edge2DirectionCopy, edge2InteriorOffset, edge2ZeroPointCross, edge2ZeroInteriorCross);
            }
            if (!inside) continue;
            {
                TestSweepFaceEdge(inside, faceContactPoint, rec, rec+0xC, rec+0x18,
                    edge3PointCross, edge3InteriorCross, edge3PointCrossTemporary, edge3Direction, edge3PointOffset,
                    edge3InteriorCrossTemporary, edge3DirectionCopy, edge3InteriorOffset, edge3ZeroPointCross, edge3ZeroInteriorCross);
            }
            if (!inside) continue;
            inside=true;
            break;
        } while ((inside=false, false));
        if (inside) {
            best=planeHitFraction;
            if (outTriangleIndex) *outTriangleIndex=index;
        } else {
            Triangle_ClosestPointOnEdges(closestEdgePoint,self,faceContactPoint,rec,rec+0xC,rec+0x18);
            float distance=CQuadTree_IntersectBallSweepWithPoint(self,closestEdgePoint,startPosition,displacement);
            if (distance < best) {
                best=distance;
                ((CourseTriangle*)rec)->hitFeatures=1;
                if (outTriangleIndex) *outTriangleIndex=index;
            }
        }
    }
    return best;
}

#pragma opt_common_subs off
// Recompute the green override plane normal and constant from the active player's displacement.
extern "C" void CQuadTree_CalcGreenPlane(void* self) {
	char* base = (char*)self;
	float tol = kFloat_0_00001_QuadTree;
	if (Vec3_NearlyEqualWithTolerance(((char*)&AsCQuadTree(base)->greenBoxMaxX), ((char*)&AsCQuadTree(base)->greenBoxMinX), tol))
		return;
	void* manager = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	void* data = GolfPlayer_GetEntryByIndex(manager, AsCGolfDoc(manager)->currentPlayer);
	float bounds[3];
	float result[3];
	float scaledSlope[3];
	float scale[3];
	float offset[3];
	float dx, dz;
	__mi__FRC7WVectorRC7WVector(bounds, (char*)&((PlayerGroundParameters*)data)->position, base + 0x20B4);
	float zero = kFloat_0_805DF744;
	float one = kFloat_1_QuadTree;
	offset[0] = zero;
	offset[1] = one;
	offset[2] = zero;
	dz = AsCQuadTree(base)->greenBoxMaxZ - AsCQuadTree(base)->greenBoxMinZ;
	dx = AsCQuadTree(base)->greenBoxMaxX - AsCQuadTree(base)->greenBoxMinX;
	scale[1] = zero;
	scale[2] = bounds[2] / dz;
	scale[0] = bounds[0] / dx;

	__ml__FfRC7WVector(scaledSlope, scale, AsCQuadTree(base)->greenFactor);

	__pl__FRC7WVectorRC7WVector(result, scaledSlope, offset);
	__as__7WVectorFRC7WVector(((char*)&AsCQuadTree(base)->greenPlaneNormalX), result);
	(*(WVector*)(base + 0x20C4)).Normalize();

	WVector inverse = kFloat_neg1_QuadTree * *(WVector*)(base + 0x20C4);
	AsCQuadTree(base)->greenPlaneConstant = (inverse * *(WVector*)(base + 0x20B4));
}
#pragma opt_common_subs reset

// Use the green override plane when active; otherwise preserve the point's Y.
extern "C" float CQuadTree_GetGreenPos(void* self, void* point) { int flag = 0;
	struct V3 { unsigned int c, b, a; };

	if (!Vec3_NearlyEqualWithTolerance(((char*)&AsCQuadTree(self)->greenBoxMaxX), ((char*)&AsCQuadTree(self)->greenBoxMinX), 0.00001f)) {
		int b; int c;
		int a; a = ((V3*)point)->a; c = ((V3*)point)->c; b = ((V3*)point)->b;
		V3 copy;
		copy.c = c;
		copy.b = b;
		copy.a = a;
		if (GetGroundType__9CQuadTreeF7WVector(self, &copy) == 2) {
			flag = 1;
		}
	}
	if (flag) {
		float z = AsCQuadTree(self)->greenPlaneNormalZ * ((Vec3f*)point)->z;
		z = AsCQuadTree(self)->greenPlaneNormalX * ((Vec3f*)point)->x + z;
		return kFloat_neg1_QuadTree * (AsCQuadTree(self)->greenPlaneConstant + z) / AsCQuadTree(self)->greenPlaneNormalY;
	}
	return ((Vec3f*)point)->y;
}

// Combine gravity along the slope and material friction. Set stopped state when
// speed < |frictionForce| / mass * (1/30 second), the fixed-tick speed decrement.
extern "C" void CQuadTree_ComputeBallGroundForces(float* out, void* self) {
    int index;
    *(float*)out = kFloat_0_805DF744;
    *(float*)((char*)out + 4) = kFloat_0_805DF744;
    float groundNormal[3];
    *(float*)((char*)out + 8) = kFloat_0_805DF744;
    float downhillForce[3];
    float frictionForce[3];
    float clampedNormalUpCosine; float slopeSine; float factor; float materialFriction; float ballMass;
    void* base = GolfBall__Fv();
    index = ((BallGroundTriangleView*)base)->groundTriangleIndex;
    ((CGolfBall*)GolfBall__Fv())->m_contactMaterialIndex = *(int*)(*(unsigned int*)(*(char**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + index * kTriangleStride + 0x3C);
    int useOverride = 0;
    if (Vec3_NearlyEqualWithTolerance(((char*)&AsCQuadTree(self)->greenBoxMaxX), ((char*)&AsCQuadTree(self)->greenBoxMinX), kFloat_0_00001_QuadTree) == 0) {
        Vec3f v = *(Vec3f*)GolfBall__Fv();
        if (GetGroundType__9CQuadTreeF7WVector(self, &v) == 2) useOverride = 1;
    }
    if (useOverride) __as__7WVectorFRC7WVector(groundNormal, ((char*)&AsCQuadTree(self)->greenPlaneNormalX));
    else {
        int i = ((BallGroundTriangleView*)GolfBall__Fv())->groundTriangleIndex;
        __as__7WVectorFRC7WVector(groundNormal, (char*)(*(unsigned int*)(*(char**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + i * kTriangleStride + 0x24));
        if (groundNormal[1] < kFloat_0_805DF744) {
            void* b2 = GolfBall__Fv();
            int i2 = ((BallGroundTriangleView*)b2)->groundTriangleIndex;
            char* w2 = *(char**)&AsCQuadTree(self)->groundSurface;
            if (*(unsigned char*)(*(unsigned int*)(w2 + 0xB0) + i2 * kTriangleStride + 0x34)) Vec3_NegateInPlace(groundNormal);
        }
    }
    // Project gravity into the surface tangent plane; normal/up dot gives the slope cosine.
    float up[3] = {kFloat_0_805DF744, kFloat_1_QuadTree, kFloat_0_805DF744};
    float normalUpCosine; normalUpCosine = __ml__FRC7WVectorRC7WVector(groundNormal, up);
    if (normalUpCosine < kFloat_neg1_QuadTree) clampedNormalUpCosine = kFloat_neg1_QuadTree;
    else if (normalUpCosine > kFloat_1_QuadTree) clampedNormalUpCosine = kFloat_1_QuadTree;
    else clampedNormalUpCosine = normalUpCosine;
    slopeSine = Math_SinApprox(acos(clampedNormalUpCosine));
    float right[3] = {kFloat_0_805DF744, kFloat_neg1_QuadTree, kFloat_0_805DF744};
    float forward[3];
    __ml__FfRC7WVector(forward, groundNormal, normalUpCosine);
    __pl__FRC7WVectorRC7WVector(downhillForce, right, forward);
    Normalize__7WVectorFv(downhillForce);
    base = GolfBall__Fv();
    float strength = AsCQuadTree(self)->slopeGravityScale * slopeSine;
    float gravityMass = ((CGolfBall*)base)->m_mass;
    __amu__7WVectorFf(downhillForce, kFloat_34_295296_QuadTree * (strength * gravityMass));
    __apl__7WVectorFRC7WVector(out, downhillForce);
    base = GolfBall__Fv();
    __ml__FfRC7WVector(frictionForce, (char*)&((CGolfBall*)base)->m_velocity, kFloat_neg1_QuadTree);
    Normalize__7WVectorFv(frictionForce);
    // Friction opposes normalized velocity and scales with material, weather, normal load and mass.
    base = GolfBall__Fv();
    int mode = ((SharedCollisionRules*)g_sharedDoc)->weatherCode;
    void* arg2 = *(void**)&((CGolfBall*)base)->m_contactMaterialIndex;
    switch (mode) {
    case 2: case 3: {
        float* data = ((float* (*)(void*, void*))CourseData_GetEntryByIndex)((*(void**)&AsCQuadTree(self)->courseData), arg2);
        switch (((CourseMaterial*)data)->groundType) {
        case 0: factor = kFloat_1_3_QuadTree; break;
        case 1: factor = kFloat_1_8_QuadTree; break;
        case 4: factor = kFloat_1_8_QuadTree; break;
        case 3: factor = kFloat_1_5_CourseEffects; break;
        case 2: factor = kFloat_1_13_QuadTree; break;
        case 7: factor = kFloat_1_QuadTree; break;
        default: factor = kFloat_1_8_QuadTree; break;
        }
        break;
    }
    case 0: case 1:
    default: factor = kFloat_1_QuadTree; break;
    }
    base = GolfBall__Fv();
    int lookupIndex;
    void* owner;
    lookupIndex = *(volatile int*)&((CGolfBall*)base)->m_contactMaterialIndex;
    owner = (*(void**)&AsCQuadTree(self)->courseData);
    materialFriction = ((float* (*)(void*, int))CourseData_GetEntryByIndex)(owner, lookupIndex)[1];
    base = GolfBall__Fv();
    float normalFrictionScale = AsCQuadTree(self)->frictionScale * factor;
    float frictionMass = ((CGolfBall*)base)->m_mass;
    normalFrictionScale = normalFrictionScale * materialFriction;
    normalFrictionScale = normalUpCosine * normalFrictionScale;
    __amu__7WVectorFf(frictionForce, kFloat_34_295296_QuadTree * (frictionMass * normalFrictionScale));
    __apl__7WVectorFRC7WVector(out, frictionForce);
    base = GolfBall__Fv();
    ballMass = ((CGolfBall*)base)->m_mass;

    if (Magnitude__7WVectorCFv((char*)GolfBall__Fv() + 0xC) < (Magnitude__7WVectorCFv(frictionForce) / ballMass * kFloat_0_033333335)) ((CGolfBall*)GolfBall__Fv())->m_state = kBallStopped;
}

// Rolling-contact phase helpers retain caller-owned vector storage for CodeWarrior allocation.
#pragma push
#pragma always_inline on
#pragma inline_depth(16)
// Tail of CGolfBall::m_holeInCrypt: encoded word, address-check bytes and per-object XOR mask.
struct BallHoledEncoding_u801CCCC0 {
    unsigned long encodedWord;
    unsigned char addressCheck[4];
    unsigned long xorMask;
};

// Mark the ball holed, preserving both the complemented XOR value and address-dependent checks.
static inline void EncodeBallHoled(char* ball) {
    enum {
        kEncodedWordOffset = COURSE_OFFSET(CGolfBall, m_holeInCrypt),
        kAddressCheckOffset = kEncodedWordOffset + COURSE_OFFSET(BallHoledEncoding_u801CCCC0, addressCheck),
        kXorMaskOffset = kEncodedWordOffset + COURSE_OFFSET(BallHoledEncoding_u801CCCC0, xorMask)
    };
    int value=1;
    char* key=ball+kEncodedWordOffset;
    char* encodedBytes=key;
    unsigned char* addressBytes=(unsigned char*)&key;
    *(unsigned long*)(ball+kEncodedWordOffset)=~*(unsigned long*)&value^*(unsigned long*)(ball+kXorMaskOffset);
    for(int byte=0;byte<sizeof(key);++byte) ball[byte+kAddressCheckOffset]=encodedBytes[byte]^addressBytes[byte];
}

// Bit-preserving XYZ copies; these integer words are never axis-accessed directly.
struct RollingVectorWords_u801CCCC0 { int word0; int word1; int word2; };

// Move backward by the fixed fallback step; request recovery once this fallback velocity is small.
static inline void RecoverRollingBallWithoutGround(char*& ball, float& initialSpeed, float& ballMass,
        float (&reverseVelocity)[3], float (&reverseUnitVelocity)[3], float (&fallbackVelocity)[3], float (&movement)[3]) {
    __ml__FfRC7WVector(reverseVelocity, ball + COURSE_OFFSET(CGolfBall, m_velocity), kFloat_neg1_QuadTree); Vec3_DivideScalar(reverseUnitVelocity, initialSpeed, reverseVelocity);
    __ml__FRC7WVectorf(fallbackVelocity, reverseUnitVelocity, kFloat_0_033333335);
    __as__7WVectorFRC7WVector(ball + COURSE_OFFSET(CGolfBall, m_velocity), fallbackVelocity);
    __ml__FRC7WVectorf(movement, ball + COURSE_OFFSET(CGolfBall, m_velocity), kFloat_0_033333335);
    __apl__7WVectorFRC7WVector(ball, movement);
    ballMass = ((CGolfBall*)ball)->m_mass;
    if (Magnitude__7WVectorCFv(ball + COURSE_OFFSET(CGolfBall, m_velocity)) < kFloat_0_033333335 * (kFloat_1_QuadTree / ballMass)) {
        Vec3_Zero(ball + COURSE_OFFSET(CGolfBall, m_velocity));
        ((CGolfBall*)ball)->m_recoveryRequired = 1;
        ((CGolfBall*)ball)->m_state = kBallStopped;
    }
}

// Test one contact against an oriented triangle edge, accepting either near-zero cross product.
// All vectors are world-space. Scratch is caller-owned to retain conditional zero initialization.
static inline void IsRollingContactInsideEdge(bool& inside, float* contact,
        char* edgeStart, char* edgeEnd, char* interiorVertex,
        float (&pointOffset)[3], float (&direction)[3], float (&pointCrossTemporary)[3],
        float (&pointCross)[3], float (&interiorOffset)[3], float (&directionCopy)[3],
        float (&interiorCrossTemporary)[3], float (&interiorCross)[3],
        float (&zeroPointCross)[3], float (&zeroInteriorCross)[3]) {
    __mi__FRC7WVectorRC7WVector(pointOffset,contact,edgeStart);
    __mi__FRC7WVectorRC7WVector(direction,edgeEnd,edgeStart);
    Vector_CrossProduct(pointCrossTemporary,direction,pointOffset);
    __as__7WVectorFRC7WVector(pointCross,pointCrossTemporary);
    __mi__FRC7WVectorRC7WVector(interiorOffset,interiorVertex,edgeStart);
    __mi__FRC7WVectorRC7WVector(directionCopy,edgeEnd,edgeStart);
    Vector_CrossProduct(interiorCrossTemporary,directionCopy,interiorOffset);
    __as__7WVectorFRC7WVector(interiorCross,interiorCrossTemporary);
    if (__ml__FRC7WVectorRC7WVector(pointCross,interiorCross)>kFloat_0_805DF744 ||
        (zeroPointCross[0]=kFloat_0_805DF744, zeroPointCross[1]=kFloat_0_805DF744, zeroPointCross[2]=kFloat_0_805DF744,
        Vec3_NearlyEqualWithTolerance(pointCross,zeroPointCross,kFloat_0_00001_QuadTree)) ||
        (zeroInteriorCross[0]=kFloat_0_805DF744, zeroInteriorCross[1]=kFloat_0_805DF744, zeroInteriorCross[2]=kFloat_0_805DF744,
        Vec3_NearlyEqualWithTolerance(interiorCross,zeroInteriorCross,kFloat_0_00001_QuadTree)))
        inside=true;
    else inside=false;
}

// Apply the capped cup-attraction impulse after ordinary ground motion has been resolved.
// cupContactCounter counts attraction updates; dt is the caller's simulation step.
static inline void ApplyRollingBallCupAttraction(char*& quadTree, char*& ball, float& dt,
        RollingVectorWords_u801CCCC0& centerFinal, RollingVectorWords_u801CCCC0& center2, float (&diffFinal)[3], float (&cupOffset)[3],
        float (&recovery)[3], float (&cupVelocityIncrement)[3]) {
    // Phase 5: limited cup attraction. The cached sweep distance gates this response;
    // the counter caps repeated attraction updates, not elapsed seconds.
    float finalRadius=((CGolfBall*)ball)->m_radius;
    if ((((QuadTreeGeometryState*)quadTree)->cupSweepParameters.z)>(AsCQuadTree(quadTree)->cupRadius)+finalRadius) return;
    if (!((float)((QuadTreeGeometryState*)quadTree)->cupContactCounter<kFloat_45_QuadTree)) return;
    ++((QuadTreeGeometryState*)quadTree)->cupContactCounter;
    char* player=((char*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    *(Arr3*)&centerFinal=*(Arr3*)(((char*)AsCGolfDoc(player)->holeRecords)+AsCGolfDoc(player)->currentHoleIndex*32+12);
    __mi__FRC7WVectorRC7WVector(diffFinal,ball,&centerFinal);
    __as__7WVectorFRC7WVector(cupOffset,diffFinal);
    cupOffset[1]=kFloat_0_805DF744;
    if (__ml__FRC7WVectorRC7WVector(cupOffset,12+ball)>kFloat_0_805DF744) {
        player=((char*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        *(Arr3*)&center2=*(Arr3*)(((char*)AsCGolfDoc(player)->holeRecords)+AsCGolfDoc(player)->currentHoleIndex*32+12);
        __mi__FRC7WVectorRC7WVector(recovery,&center2,ball);
        recovery[1]=kFloat_0_805DF744;
        Normalize__7WVectorFv(recovery);

        __amu__7WVectorFf(recovery,kFloat_80_QuadTree); __ml__FRC7WVectorf(cupVelocityIncrement,recovery,dt);
        __apl__7WVectorFRC7WVector((((ball + COURSE_OFFSET(CGolfBall, m_velocity)))),cupVelocityIncrement);
    }
}

// Advance rolling motion, resolve stopped/cup state, sweep steep faces, then apply cup attraction.
extern "C" void CQuadTree_UpdateBallTriangleCollision(void* self, float dt) {
    // The stopped phase aliases movementDelta as cupOffset; moving phases use it as displacement.
    float recoveryBounds[6]; float movementDelta[3]; float groundTarget[3]; float contactPlane[4]; float radiusShiftedPosition[3]; float recovery[3];
    float fallbackVelocity[3]; float reverseUnitVelocity[3]; float reverseVelocity[3]; float movement[3]; float velocityIncrement[3]; float groundAcceleration[3]; float groundForce[3];
    float stoppedCupDelta[3]; RollingVectorWords_u801CCCC0 stoppedCupCenterBits; RollingVectorWords_u801CCCC0 stoppedPositionBits;
    float predictedDisplacement[3]; float postObjectHitDisplacement[3]; float unobstructedTarget[3]; RollingVectorWords_u801CCCC0 groundQueryPositionBits;
    float groundTravelDelta[3]; RollingVectorWords_u801CCCC0 segmentOriginBits;
    float radiusAlongPlaneNormal[3]; float planeContactPoint[3]; float toPlaneContact[3]; float movementToFace[3]; float predictedPosition[3]; float previousGroundDelta[3]; float cupDelta[3];
    RollingVectorWords_u801CCCC0 cupCenterBits; RollingVectorWords_u801CCCC0 attractionCupCenterBits;
    float cupVelocityIncrement[3]; float scaledInitialSpinVelocity[3]; float initialSpinVelocity[3]; RollingVectorWords_u801CCCC0 initialVelocityBits;
    float sweepEnd[3];
    float edge3PointCross[3]; float edge3InteriorCross[3]; float edge3PointCrossTemporary[3]; float edge3Direction[3]; float edge3PointOffset[3]; float edge3InteriorCrossTemporary[3]; float edge3DirectionCopy[3]; float edge3InteriorOffset[3]; float edge3ZeroPointCross[3]; float edge3ZeroInteriorCross[3];
    float edge2PointCross[3]; float edge2InteriorCross[3]; float edge2PointCrossTemporary[3]; float edge2Direction[3]; float edge2PointOffset[3]; float edge2InteriorCrossTemporary[3]; float edge2DirectionCopy[3]; float edge2InteriorOffset[3]; float edge2ZeroPointCross[3]; float edge2ZeroInteriorCross[3];
    float edge1PointCross[3]; float edge1InteriorCross[3]; float edge1PointCrossTemporary[3]; float edge1Direction[3]; float edge1PointOffset[3]; float edge1InteriorCrossTemporary[3]; float edge1DirectionCopy[3]; float edge1InteriorOffset[3]; float edge1ZeroPointCross[3]; float edge1ZeroInteriorCross[3];
    float scaledRecoverySpinVelocity[3]; float recoverySpinVelocity[3]; RollingVectorWords_u801CCCC0 recoveryVelocityBits;
    volatile union { float f; unsigned int u; } bits;
    float initialSpeed; float responseScalar;
    char* triangleIdCursor;
    char* quadTree = (char*)self;
    char* ball = (char*)GolfBall__Fv();
    // Phase 1: establish support. A missing triangle takes the legacy reverse/slow/stop path.
    // Vectors here are world-space, Y up; dt is the caller's simulation step.
    ((BallGroundTriangleView*)ball)->groundTriangleIndex = CQuadTree_GetMeshIndex(self, ball, 0);

    initialSpeed = Magnitude__7WVectorCFv(ball + COURSE_OFFSET(CGolfBall, m_velocity));
    if (((BallGroundTriangleView*)ball)->groundTriangleIndex == -1) {

        RecoverRollingBallWithoutGround(ball, initialSpeed, responseScalar, reverseVelocity, reverseUnitVelocity, fallbackVelocity, movement);
        return;
    }
    responseScalar = ((CGolfBall*)ball)->m_mass;

    // Phase 2: update velocity from slope gravity and friction. responseScalar is reused
    // for mass, launch speed and material response in disjoint branches; it has no single unit.
    CQuadTree_ComputeBallGroundForces(groundForce, self); Vec3_DivideScalar(groundAcceleration, responseScalar, groundForce);
    __ml__FRC7WVectorf(velocityIncrement, groundAcceleration, dt);
    __apl__7WVectorFRC7WVector(ball + COURSE_OFFSET(CGolfBall, m_velocity), velocityIncrement);
    // Force integration may stop the ball; handle cup completion, raised-region recovery and spin delay.
    if (((CGolfBall*)ball)->m_state == kBallStopped) {
        float (&cupOffset)[3] = movementDelta;
        char* player = ((char*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        *(Arr3*)&stoppedCupCenterBits = *(Arr3*)(((char*)AsCGolfDoc(player)->holeRecords) + AsCGolfDoc(player)->currentHoleIndex*32 + 12);
        
        __mi__FRC7WVectorRC7WVector(stoppedCupDelta,ball,&stoppedCupCenterBits);
        __as__7WVectorFRC7WVector(cupOffset,stoppedCupDelta);
        cupOffset[1] = kFloat_0_805DF744;
        if (Magnitude__7WVectorCFv(cupOffset) < AsCQuadTree(quadTree)->cupRadius) {
            char* active = (char*)GolfBall__Fv();
            char* record = (char*)CGolfBall_GetOrAllocatePreDataAt(active,((CGolfBall*)active)->m_preDataCount);
            Vec3_Zero((active+12));
            ((CGolfBall*)active)->m_state=kBallStopped;
            EncodeBallHoled(active);
            ((BallCollisionHistory*)record)->eventType=kCupHistoryEvent;
            ((BallCollisionHistory*)record)->collisionType=kNoHistoryCollision;
            char* mgr=(char*)g_sharedDoc;
            player=((char*)AsCSharedDoc(mgr)->golfDoc);
            void* found=ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(mgr)->itemInventory[0]),(void*)12);
            do {
                if (found!=*(void**)(mgr+AsCGolfDoc(player)->currentPlayer*0x1D4+0x88)) {
                    mgr=(char*)g_sharedDoc;
                    player=((char*)AsCSharedDoc(mgr)->golfDoc);
                    found=ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(mgr)->itemInventory[0]),(void*)128);
                    if (found!=*(void**)(mgr+AsCGolfDoc(player)->currentPlayer*0x1D4+0x88)) break;
                }
                strcpy(((BallCollisionHistory*)record)->soundName,courseObjectSoundStrings.specialBallCupSound);
                return;
            } while (0);
            strcpy(((BallCollisionHistory*)record)->soundName,courseObjectSoundStrings.defaultCupSound);
            return;
            return;
        }
        Vec3_Zero(((12+ball)));
        Vec3_Zero(recoveryBounds);
        Vec3_Zero(recoveryBounds + 3);
        *(Arr3*)&stoppedPositionBits=*(Arr3*)ball;
        bool hit;
        // Regions have an open X/Z footprint and only an upper Y plane.
        // Exhausting this search clears hit; a matching region sets it before breaking.
        for (unsigned int j=0;j<AsCQuadTree(quadTree)->groundRegionCount || (hit=false, false);++j) {
            char* box=*(char**)&AsCQuadTree(quadTree)->groundRegions+j*0x1C;
            if (((Vec3f*)&stoppedPositionBits)->x>((GroundRegion*)box)->boundsMin.x) {
                if (((Vec3f*)&stoppedPositionBits)->x<((GroundRegion*)box)->boundsMax.x) {
                    if (((Vec3f*)&stoppedPositionBits)->z>((GroundRegion*)box)->boundsMin.z) {
                        if (((Vec3f*)&stoppedPositionBits)->z<((GroundRegion*)box)->boundsMax.z) {
                            if (((Vec3f*)&stoppedPositionBits)->y<((GroundRegion*)box)->boundsMax.y) {
                                AABB_Copy(recoveryBounds,box);
                                int id=((GroundRegion*)box)->lie;
                                ((CGolfBall*)GolfBall__Fv())->m_lie=id;
                                hit=true;
                                break;
                            }
                        }
                    }
                }
            }
        }
        if (hit) {
            ((CGolfBall*)ball)->m_recoveryRequired=1;
            CQuadTree_FindBallRecoveryPosition(self,recoveryBounds);
            return;
        }
        if (((CGolfBall*)ball)->m_flags&kStoppedSpinDelayMask) {
            char* first=(char*)((void*(*)(void*,int))PreData__9CGolfBallFi)(GolfBall__Fv(),0);
            *(Arr3*)&initialVelocityBits=*(Arr3*)(12+first);
            ((Vec3f*)&initialVelocityBits)->y=kFloat_0_805DF744;
            // responseScalar is launch speed for the division, then delayed-spin magnitude.
            responseScalar=ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
        
            __ml__FfRC7WVector(scaledInitialSpinVelocity,&initialVelocityBits,kFloat_neg40_CourseEffects*((CGolfBall*)GolfBall__Fv())->m_initSpin); Vec3_DivideScalar(initialSpinVelocity,responseScalar,scaledInitialSpinVelocity);
            __as__7WVectorFRC7WVector((char*)GolfBall__Fv()+0x4C,initialSpinVelocity);
            responseScalar=kFloat_1_5_CourseEffects*((CGolfBall*)GolfBall__Fv())->m_initSpin;
            ((CGolfBall*)GolfBall__Fv())->m_spin=responseScalar;
        
            ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpinDelayStart]=(((CGolfBall*)GolfBall__Fv())->m_preDataCount);
            ((CGolfBall*)GolfBall__Fv())->m_state=kBallSpinDelay;
            Vec3_Zero((char*)GolfBall__Fv()+12);
            char* active=(char*)GolfBall__Fv();
            ((CGolfBall*)active)->m_flags&=~kSpinDelayEntryClearMask;
        }
        return;
        return;
    }
    // Still moving: resolve objects, compute a ground-following target and sweep steep triangle faces.
    __ml__FRC7WVectorf(predictedDisplacement,ball + COURSE_OFFSET(CGolfBall, m_velocity),dt);
    __as__7WVectorFRC7WVector(movementDelta,predictedDisplacement);
    float fraction=CQuadTree_FindNearestSweptBallObjectHit(self,(float*)ball,movementDelta,contactPlane);
    if (!(!(fraction < kFloat_1_QuadTree))) {
        CQuadTree_ApplyBallMaterialCollision((Self5EB0*)self,movementDelta,contactPlane,fraction);
        __as__7WVectorFRC7WVector(groundTarget,ball);
        __ml__FRC7WVectorf(postObjectHitDisplacement,(ball + COURSE_OFFSET(CGolfBall, m_velocity)),dt);
        __as__7WVectorFRC7WVector(movementDelta,postObjectHitDisplacement);
    } else {
        __pl__FRC7WVectorRC7WVector(unobstructedTarget,ball,movementDelta);
        __as__7WVectorFRC7WVector(groundTarget,unobstructedTarget);
    }
    *(Arr3*)&groundQueryPositionBits=*(Arr3*)groundTarget;
    groundTarget[1]=CQuadTree_GetHighestGround(self,(float*)&groundQueryPositionBits,(int*)(ball+0x28));
    if (((BallGroundTriangleView*)ball)->groundTriangleIndex==-1) groundTarget[1]=kFloat_neg100000_QuadTree;
    else {
        float ballRadius=((CGolfBall*)ball)->m_radius; groundTarget[1]=(groundTarget[1])+ballRadius;
        // Reuse the matched scalar slot as rolling radius for the rotation update.
        responseScalar=((CGolfBall*)ball)->m_radius;
        __mi__FRC7WVectorRC7WVector(groundTravelDelta,groundTarget,ball);
        ((CGolfBall*)ball)->m_rotation[1]-=kFloat_0_3_BallPhysics*WVectorLen__FRC7WVector(groundTravelDelta)/responseScalar;
    }
    *(Arr3*)&segmentOriginBits=*(Arr3*)ball;
    CQuadTree_GetMeshListAlongSegment(self,&segmentOriginBits,movementDelta);
    float faceHitFraction;float spinResponseScalar;triangleIdCursor=quadTree;
    for (int i=0;i<AsCQuadTree(quadTree)->candidateTriangleCount;triangleIdCursor+=4,++i) { bool inside;
        Plane_Copy(contactPlane,*(char**)&((CourseSurface*)*(char**)&AsCQuadTree(quadTree)->groundSurface)->triangles+AsCQuadTree(triangleIdCursor)->candidateTriangleIds[0]* kTriangleStride+0x24);
        char* tri=*(char**)&((CourseSurface*)*(char**)&AsCQuadTree(quadTree)->groundSurface)->triangles+AsCQuadTree(triangleIdCursor)->candidateTriangleIds[0]* kTriangleStride;
        if (((CourseTriangle*)tri)->twoSided) { if (Plane_EvaluatePoint(contactPlane,ball)<kFloat_0_805DF744) {
                Vec3_NegateInPlace(contactPlane);
                contactPlane[3] = ((contactPlane[3] * kFloat_neg1_QuadTree));
            } }
        tri=*(char**)&((CourseSurface*)*(char**)&AsCQuadTree(quadTree)->groundSurface)->triangles+AsCQuadTree(triangleIdCursor)->candidateTriangleIds[0]* kTriangleStride;
        if (abs564(contactPlane[1],kFloat_0_805DF744)>kFloat_0_5_QuadTree) continue;
        if (__ml__FRC7WVectorRC7WVector(contactPlane,movementDelta)>kFloat_0_805DF744) continue;
        if (((CourseTriangle*)tri)->vertices[0].y<((CGolfBall*)ball)->m_pos.y && ((CourseTriangle*)tri)->vertices[1].y<((CGolfBall*)ball)->m_pos.y && ((CourseTriangle*)tri)->vertices[2].y<((CGolfBall*)ball)->m_pos.y) continue;

        __ml__FfRC7WVector(radiusAlongPlaneNormal,contactPlane,((CGolfBall*)ball)->m_radius);
        __mi__FRC7WVectorRC7WVector(radiusShiftedPosition,ball,radiusAlongPlaneNormal);
        if (__ml__FRC7WVectorRC7WVector(movementDelta,contactPlane)>kFloat_0_805DF744) faceHitFraction=kFloat_1_QuadTree;
        else {
            faceHitFraction=Plane_EvaluatePointReversed(radiusShiftedPosition,contactPlane);

            __pl__FRC7WVectorRC7WVector(sweepEnd,radiusShiftedPosition,movementDelta);
            float signedEndDistance=Plane_EvaluatePointReversed(sweepEnd,contactPlane);
            if (faceHitFraction*signedEndDistance<=kFloat_0_805DF744) {

                faceHitFraction=abs564(faceHitFraction,kFloat_0_805DF744)/((abs564(faceHitFraction,kFloat_0_805DF744))+(abs564(signedEndDistance,kFloat_0_805DF744)));
            } else faceHitFraction=kFloat_1_QuadTree;
        }
        if (!(faceHitFraction<kFloat_1_QuadTree)) continue;
        __ml__FfRC7WVector(toPlaneContact,movementDelta,faceHitFraction);
        __pl__FRC7WVectorRC7WVector(planeContactPoint,radiusShiftedPosition,toPlaneContact);

        IsRollingContactInsideEdge(inside, planeContactPoint, tri+12, tri+24, tri,
            edge1PointOffset, edge1Direction, edge1PointCrossTemporary, edge1PointCross, edge1InteriorOffset, edge1DirectionCopy, edge1InteriorCrossTemporary, edge1InteriorCross, edge1ZeroPointCross, edge1ZeroInteriorCross);
        // Each edge compares point-side and triangle-interior cross products, accepting
        // zero cross products within tolerance. Rejected edges share the false assignment.
        // Keep these tests separate: later zero-vector initializations are conditional.
        do {
            if (!inside) continue;
            IsRollingContactInsideEdge(inside, planeContactPoint, tri, tri+24, tri+12,
                edge2PointOffset, edge2Direction, edge2PointCrossTemporary, edge2PointCross, edge2InteriorOffset, edge2DirectionCopy, edge2InteriorCrossTemporary, edge2InteriorCross, edge2ZeroPointCross, edge2ZeroInteriorCross);
            if (0 == inside) continue;
            IsRollingContactInsideEdge(inside, planeContactPoint, tri, tri+12, tri+24,
                edge3PointOffset, edge3Direction, edge3PointCrossTemporary, edge3PointCross, edge3InteriorOffset, edge3DirectionCopy, edge3InteriorCrossTemporary, edge3InteriorCross, edge3ZeroPointCross, edge3ZeroInteriorCross);
            if (!inside) continue;
            inside=true;
            break;
        } while ((inside=false, false));
        if (!inside) continue;
        ((CGolfBall*)ball)->m_spin=kFloat_0_805DF744;
        ((CGolfBall*)ball)->m_curve=kFloat_0_805DF744;

        __ml__FfRC7WVector(movementToFace,movementDelta,faceHitFraction);
        __apl__7WVectorFRC7WVector(ball,movementToFace);
        float ballRadius=((CGolfBall*)ball)->m_radius; ((CGolfBall*)ball)->m_pos.y=((CGolfBall*)ball)->m_pos.y+ballRadius;
        int contactMaterialIndex=((CourseTriangle*)(*(char**)&((CourseSurface*)*(char**)&AsCQuadTree(quadTree)->groundSurface)->triangles+AsCQuadTree(quadTree+((i*4)))->candidateTriangleIds[0]* kTriangleStride))->materialIndex;
        ((CGolfBall*)ball)->m_contactMaterialIndex=contactMaterialIndex;
        int type=((SharedCollisionRules*)g_sharedDoc)->weatherCode;

        // responseScalar is now the weather restitution multiplier.
        if ((2<=type || (faceHitFraction=0, false)) && !(!(4>type))) {
            switch (((CourseMaterial*)CourseData_GetEntryByIndex(*(char**)&AsCQuadTree(quadTree)->courseData,contactMaterialIndex))->groundType) {
            case kGroundTypeTee: /* Weather-adjusted restitution for this ground type. */ responseScalar=kFloat_0_8_QuadTree; break;
            case kGroundTypeFairway: /* Weather-adjusted restitution for this ground type. */ responseScalar=kFloat_0_8_QuadTree; break;
            case kGroundTypeBunker: /* Weather-adjusted restitution for this ground type. */ responseScalar=kFloat_0_8_QuadTree; break;
            case kGroundTypeRough: /* Weather-adjusted restitution for this ground type. */ responseScalar=kFloat_0_7_QuadTree; break;
            case kGroundTypeGreen: /* Weather-adjusted restitution for this ground type. */ responseScalar=kFloat_0_9_QuadTreeGeometry; break;
            case kGroundTypeIce: /* Weather-adjusted restitution for this ground type. */ responseScalar=kFloat_1_QuadTree; break;
            case 5: case 6:
            default: responseScalar=kFloat_0_8_QuadTree; break;
            }
        } else {
            responseScalar=kFloat_1_QuadTree;
        }
        float* material=((float*(*)(void*,int))CourseData_GetEntryByIndex)(*(char**)&AsCQuadTree(quadTree)->courseData,((CGolfBall*)ball)->m_contactMaterialIndex);
        float materialValue=material[0];
        {float k=kFloat_neg1_QuadTree*responseScalar; __amu__7WVectorFf((ball + COURSE_OFFSET(CGolfBall, m_velocity)),k*materialValue);}
        strcpy((char*)CGolfBall_GetOrAllocatePreDataAt(ball,((CGolfBall*)ball)->m_preDataCount)+0x20,courseObjectSoundStrings.woodCollisionSound);
        __as__7WVectorFRC7WVector((char*)CGolfBall_GetOrAllocatePreDataAt(ball,((CGolfBall*)ball)->m_preDataCount)+0x64,contactPlane);
        return;
    }
    // No steep-face response: choose airborne vs ground-following motion, then apply cup attraction.
    float radius=((CGolfBall*)ball)->m_radius;
    __pl__FRC7WVectorRC7WVector(predictedPosition,ball,movementDelta);
    if (predictedPosition[1] > FP_FMADDS(kFloat_0_5_QuadTree, radius, groundTarget[1])) {
        ((CGolfBall*)ball)->m_spin=kFloat_0_805DF744;
        ((CGolfBall*)ball)->m_curve=kFloat_0_805DF744;
        __apl__7WVectorFRC7WVector(ball,movementDelta);
        ((CGolfBall*)ball)->m_state=kBallAirborne;
        ((BallGroundTriangleView*)ball)->groundTriangleIndex=-1;
        if (((CGolfBall*)ball)->m_eventPreDataIndex[kHistorySlot1]<0) ((CGolfBall*)ball)->m_eventPreDataIndex[kHistorySlot1]=((CGolfBall*)ball)->m_preDataCount;
    } else {
        char* prev=(char*)((void*(*)(void*,int))PreData__9CGolfBallFi)(ball,((CGolfBall*)ball)->m_preDataCount-1);
        // The previous sample was rolling; +70 is its motion-state word.
        if (((BallCollisionHistory*)prev)->motionState==kBallRolling) { float absolute;
            prev=(char*)((void*(*)(void*,int))PreData__9CGolfBallFi)(ball,((CGolfBall*)ball)->m_preDataCount-1);
            __mi__FRC7WVectorRC7WVector(previousGroundDelta,groundTarget,prev);

            __as__7WVectorFRC7WVector(movementDelta,previousGroundDelta);
            float rr=((float (*)(float))Math_SquareRootApprox)(FP_FMADDS(movementDelta[0], movementDelta[0], (movementDelta[2]*movementDelta[2]))); bits.f=rr; double length=rr;

            if (bits.u & kFloatSignMask) absolute=-(float)length; else absolute=(float)length;
            if ((absolute<kFloat_0_00001_QuadTree)!=0) {
                Vec3_Zero((((12+ball))));
                ((CGolfBall*)ball)->m_state=kBallStopped;
                if (((CGolfBall*)ball)->m_flags&kStoppedSpinDelayMask) {
                    char* first=(char*)((void*(*)(void*,int))PreData__9CGolfBallFi)(GolfBall__Fv(),0);
                    *(Arr3*)&recoveryVelocityBits=*(Arr3*)((((first+12))));
                    ((Vec3f*)&recoveryVelocityBits)->y=kFloat_0_805DF744;
                    spinResponseScalar=ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
                    __ml__FfRC7WVector(scaledRecoverySpinVelocity,&recoveryVelocityBits,kFloat_neg40_CourseEffects*((CGolfBall*)GolfBall__Fv())->m_initSpin);
                    Vec3_DivideScalar(recoverySpinVelocity,spinResponseScalar,scaledRecoverySpinVelocity);
                    __as__7WVectorFRC7WVector((char*)GolfBall__Fv()+0x4C,recoverySpinVelocity);
                    spinResponseScalar=kFloat_1_5_CourseEffects*((CGolfBall*)GolfBall__Fv())->m_initSpin;
                    ((CGolfBall*)GolfBall__Fv())->m_spin=spinResponseScalar;

                    ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpinDelayStart]=(((CGolfBall*)GolfBall__Fv())->m_preDataCount);
                    ((CGolfBall*)GolfBall__Fv())->m_state=kBallSpinDelay;
                    Vec3_Zero((char*)GolfBall__Fv()+12);
                    char* active=(char*)GolfBall__Fv();
                    ((CGolfBall*)active)->m_flags&=~kSpinDelayEntryClearMask;
                }
            } else {

                if ((abs564(movementDelta[1],kFloat_0_805DF744))/Magnitude__7WVectorCFv(movementDelta)<(Math_SinApprox(kFloat_1_0471976_QuadTree))) {

                    ((CGolfBall*)ball)->m_velocity.y=(movementDelta[1]/(Math_SquareRootApprox(FP_FMADDS(movementDelta[0], movementDelta[0], movementDelta[2]*movementDelta[2]))))*(Math_SquareRootApprox(FP_FMADDS(((CGolfBall*)ball)->m_velocity.x, ((CGolfBall*)ball)->m_velocity.x, ((CGolfBall*)ball)->m_velocity.z*((CGolfBall*)ball)->m_velocity.z)));
                }
            }
        }
        __as__7WVectorFRC7WVector(ball,groundTarget);
    }
    ApplyRollingBallCupAttraction(quadTree, ball, dt, cupCenterBits, attractionCupCenterBits, cupDelta, movementDelta, recovery, cupVelocityIncrement);
}
#pragma pop

// Choose rain/snow restitution scaling for the current contact material.
// Other weather codes use unity; material IDs retain their verified numeric values.
static inline void SelectGroundBounceRestitution(float& weatherScale, int weatherCode, char* quadTree, char* ball) {
    if (2 <= weatherCode && weatherCode < 4) {
        switch (*((unsigned char*)CourseData_GetEntryByIndex((void*)AsCQuadTree(quadTree)->courseData, ((CGolfBall*)ball)->m_contactMaterialIndex) + 0x24)) {
        case kGroundTypeTee: weatherScale = kFloat_0_8_QuadTree; break;
        case kGroundTypeFairway: weatherScale = kFloat_0_8_QuadTree; break;
        case kGroundTypeBunker: weatherScale = kFloat_0_8_QuadTree; break;
        case kGroundTypeRough: weatherScale = kFloat_0_7_QuadTree; break;
        case kGroundTypeGreen: weatherScale = kFloat_0_9_QuadTreeGeometry; break;
        case kGroundTypeIce: weatherScale = kFloat_1_QuadTree; break;
        case 5: case 6: default: weatherScale = kFloat_0_8_QuadTree; break;
        }
    } else {
        weatherScale = kFloat_1_QuadTree;
    }
}

// Walk the material tree to an exact key or its embedded end marker; no allocation occurs here.
// searchNode/lowerBound are caller scratch, preserving the original register lifetimes.
static inline void FindBounceMaterialKey(char*& searchNode, char*& lowerBound, int materialKey, char* materialMap) {
    while (searchNode) {
        if (((CourseMaterialKeyNode*)searchNode)->key >= materialKey) {
            lowerBound = searchNode;
            searchNode = ((CourseMaterialKeyNode*)searchNode)->left;
        }
        else searchNode = ((CourseMaterialKeyNode*)searchNode)->right;
    }
    if (lowerBound == materialMap + 4 || materialKey < ((CourseMaterialKeyNode*)lowerBound)->key) lowerBound = 4 + materialMap;
}

// Apply ground restitution and special-flight/spin damping, recording landing/bounce sounds and events.
// Required live tree/ball/current history and world-space normal. normal.y is
// compared to collision-state float +0; its slope threshold is separate from
// TestHighSpeedGroundImpact's angle/speed thresholds. Sound selection can insert
// a default material-map node and consume rand() for one of four grass sounds.
// Flag/history editing map (CGolfBall layout in game/golfball.h):
//   event[1] < 0 admits the first-landing gate. It is a shared contact milestone:
//      airborne raised-region and triangle-collision paths also set it. Set it
//      to current sample, clear curve, then evaluate this gate once for the shot.
//   normal.y above threshold: flag 0x10 OR high-speed test suppresses bounce;
//      else flag B (0x40) suppresses it. High-speed test order is significant.
//   other slopes: B plus a recorded event[2] and later sample halves velocity.
//   clear bits 0x10/20/40 AFTER that first-landing decision, leaving low bits.
//   suppressed bounce + low flag bits 0/1: create spin impulse from launch
//      history, set event[7], state=2, zero velocity, clear mask 0x73.
//   suppressed bounce without those bits: update rough lie and stop (state=0).
//   ordinary bounce: reverse normal component; damp normal by restitution and
//      tangent by spin/weather response, then add in the existing float order.
// A/B identify observed bits, not unverified Tomahawk/Cobra/Spike names. Producers
// and event[2]/[10] milestones are in golf_rule_round_flow_and_shot_sim.cpp; airborne force/hold consumers
// are ComputeAirborneBallForces/UpdateAirborneBall here. Test first vs subsequent
// contact, both slope branches, masks 0x10/20/40, and event[2] -1/current/earlier.
extern "C" void CQuadTree_HandleBallGroundBounce(void* self, void* normal) {
    // Phase 1: select the history sound; lookup can insert a default material mapping.
    NameTable grassSoundNames;
    char reversedNormalVelocity[12], tangentVelocity[12];
    // V3 is three integer words carrying Vec3f bits for by-value ABI calls;
    // it does not convert positions/velocities to integer coordinates.
    V3 landingPositionWords;
    unsigned char reboundVelocity[12]; char dampedTangentVelocity[12]; char dampedNormalVelocity[12]; char scaledSpinVelocity[12]; char spinVelocity[12];
    V3 launchVelocityWords;
    char* quadTree = (char*)self;
    char* surfaceSoundName;
    char* ball = (char*)GolfBall__Fv();
    char* historyRecord = (char*)CGolfBall_GetOrAllocatePreDataAt(ball, ((CGolfBall*)ball)->m_preDataCount);
    if (AsCQuadTree(quadTree)->simulationMode) surfaceSoundName = &emptySimulationSurfaceName;
    else surfaceSoundName = (char*)CourseData_GetEntryByIndex((void*)AsCQuadTree(quadTree)->courseData, ((CGolfBall*)ball)->m_contactMaterialIndex) + 0x25;
    if (*surfaceSoundName) {
        char* mgr = (char*)g_sharedDoc;
        char* player = ((char*)AsCSharedDoc(mgr)->golfDoc);
        void* found = ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(mgr)->itemInventory[0]), (void*)12);
        if (found != *(void**)(mgr + AsCGolfDoc(player)->currentPlayer * 0x1D4 + 0x88)) {
            mgr = (char*)g_sharedDoc;
            player = ((char*)AsCSharedDoc(mgr)->golfDoc);
            found = ItemInventory_GetBallIdAt(((char*)&AsCSharedDoc(mgr)->itemInventory[0]), (void*)128);
            if (found == *(void**)(mgr + AsCGolfDoc(player)->currentPlayer * 0x1D4 + 0x88)) {
                strcpy(((BallCollisionHistory*)historyRecord)->soundName, courseObjectSoundStrings.specialBallBounceSound);
            } else {
                char* materialMeshName;
                int groundTriangleIndex = CQuadTree_GetMeshIndex(self, ball, 0);
                if (groundTriangleIndex < 0) {
                    materialMeshName = 0;
                } else { unsigned char insertRight;
                    void* insertionNode;
                    int materialKey = ((CourseSurface*)AsCQuadTree(quadTree)->groundSurface)->triangles[groundTriangleIndex].materialFlags & kMaterialKeyMask;

                    char* courseData = (char*)AsCQuadTree(quadTree)->courseData;
                    char* searchNode = ((CourseMaterialTreeStorage*)courseData)->root;
                    char* materialMap = courseData + 0xAC;
                    char* lowerBoundNode = 4 + materialMap;
                    FindBounceMaterialKey(searchNode, lowerBoundNode, materialKey, materialMap);
                    if (lowerBoundNode == courseData + 0xB0) {
                        materialMeshName = courseObjectSoundStrings.pad_6D;
                    } else {
                        unsigned char insertLeft; unsigned int insertionPosition = CourseMaterialMap_FindInsertionPosition(materialMap, &materialKey, &insertionNode, &insertLeft, &insertRight);
                        char* materialPair = (char*)insertionNode;
                        if (!materialPair || (int)materialKey > *(int*)(materialPair += 0xC)) {
                            int defaultPair[2] = {materialKey, s_quadTreeDefaultMaterialIndex};
                            materialPair = (char*)FxSprayTree_InsertNode(materialMap, (void*)insertionPosition, insertLeft, insertRight, defaultPair) + 0xC;
                        }
                        materialMeshName = (char*)((CourseMaterialTable*)AsCQuadTree(quadTree)->courseData)->entries + *(int*)(materialPair + 4) * 0x28;
                    }
                }

                do {
                    if (materialMeshName) {
                        if (!String_CompareCString(materialMeshName, courseObjectSoundStrings.iceGrassMesh) || !String_CompareCString(materialMeshName, courseObjectSoundStrings.shortGrassMesh) || !String_CompareCString(materialMeshName, courseObjectSoundStrings.longGrassMesh)) {
                            int* grassSoundTable = (int*)s_grassBounceSoundNames;
                            grassSoundNames.p[0] = grassSoundTable[0];
                            grassSoundNames.p[1] = grassSoundTable[1];
                            grassSoundNames.p[2] = grassSoundTable[2];
                            grassSoundNames.p[3] = grassSoundTable[3];
                            strcpy(((BallCollisionHistory*)historyRecord)->soundName, (char*)grassSoundNames.p[rand() % 4]);
                            break;
                        }
                    }
                    strcpy(((BallCollisionHistory*)historyRecord)->soundName, surfaceSoundName);
                } while (0);
            }
        }
    }
    // Phase 2: sound is selected; classify first landing and special-shot gates before applying a response.
    // Representation boundary: ball is the byte cursor for the SAME CGolfBall
    // whose named members appear here. COURSE_OFFSET(m_flags) below is its
    // signed int flags member; keep the raw lvalue/reloads (a direct member
    // substitution changed the verified instruction stream). No separate flag
    // allocation exists.
    // historyRecord is a borrowed BallCollisionHistory; soundName is +20,
    // normal +64, motionState +70. launchHistory below is the same record type;
    // its +0C is velocity float3 copied as V3 bits, not an integer vector.
    // Spin output +4C is the ball's 12-byte landing impulse (header reserved
    // bytes); force consumers use it during spin delay. Repeated GolfBall()
    // calls and the V3 copy shape remain to preserve evaluation and ABI order.
    int allowBounce = 1;
    if (((CGolfBall*)ball)->m_eventPreDataIndex[kHistorySlot1] < 0) {
        ((CGolfBall*)ball)->m_eventPreDataIndex[kHistorySlot1] = ((CGolfBall*)ball)->m_preDataCount;
        ((CGolfBall*)ball)->m_curve = kFloat_0_805DF744;
        if (((Vec3f*)normal)->y > *(float*)s_quadTreeCollisionState) {
            if ((*(int*)(ball + COURSE_OFFSET(CGolfBall, m_flags)) & kFirstLandingSuppressBounce) || CQuadTree_TestHighSpeedGroundImpact(self, normal)) allowBounce = 0;
            else if (*(int*)(ball + COURSE_OFFSET(CGolfBall, m_flags)) & kSpecialFlightB) allowBounce = 0;
        } else if ((*(int*)(ball + COURSE_OFFSET(CGolfBall, m_flags)) & kSpecialFlightB)) {
            if (((CGolfBall*)ball)->m_eventPreDataIndex[kHistorySpecialFlightB] >= 0) {
                if (((CGolfBall*)ball)->m_preDataCount > ((CGolfBall*)ball)->m_eventPreDataIndex[kHistorySpecialFlightB]) {
                    __amu__7WVectorFf(ball + COURSE_OFFSET(CGolfBall, m_velocity), kFloat_0_5_QuadTree);
                }
            }
        }
        *(int*)(ball + COURSE_OFFSET(CGolfBall, m_flags)) &= ~kLandingSpecialFlightMask;
    }
    // Phase 3: special landing stops/spin delay, or ordinary physical restitution.
    if (0 == allowBounce) {
        if (*(int*)(ball + COURSE_OFFSET(CGolfBall, m_flags)) & kSpinLandingModeMask) {
            char* launchHistory = (char*)((void*(*)(void*,int))PreData__9CGolfBallFi)(GolfBall__Fv(), 0);
            // 0x801D5A74: borrow launch record 0 from GolfBall(), not from the tree.
            launchVelocityWords = *(V3*)(launchHistory + 0xC);
            *(float*)&launchVelocityWords.y = kFloat_0_805DF744;
            float launchSpeed = ShotParameters_GetBaseLaunchSpeed(InstanceTable_Get());
            __ml__FfRC7WVector(scaledSpinVelocity, &launchVelocityWords, kFloat_neg40_CourseEffects * ((CGolfBall*)GolfBall__Fv())->m_initSpin);
            Vec3_DivideScalar(spinVelocity, launchSpeed, scaledSpinVelocity);
            __as__7WVectorFRC7WVector((char*)GolfBall__Fv() + 0x4C, spinVelocity);

            ((CGolfBall*)GolfBall__Fv())->m_spin = (kFloat_1_5_CourseEffects * ((CGolfBall*)GolfBall__Fv())->m_initSpin);

            ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpinDelayStart] = (((CGolfBall*)GolfBall__Fv())->m_preDataCount);

            ((CGolfBall*)GolfBall__Fv())->m_state = kBallSpinDelay;
            Vec3_Zero((char*)&((CGolfBall*)GolfBall__Fv())->m_velocity);
            ((CGolfBall*)GolfBall__Fv())->m_flags &= ~kSpinDelayEntryClearMask;
        } else {
            landingPositionWords = *(V3*)&((CGolfBall*)ball)->m_pos;

            char* playerGroundRecord;
            if ((GetGroundType__9CQuadTreeF7WVector(self, &landingPositionWords)) == 3) {
                if (Magnitude__7WVectorCFv(ball + COURSE_OFFSET(CGolfBall, m_velocity)) > kFloat_150_QuadTree || (((CGolfBall*)ball)->m_powerFlags & 0x50)) {
                    playerGroundRecord = (char*)GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
                    ((PlayerGroundParameters*)playerGroundRecord)->lieSlot = 2;
                } else {
                    playerGroundRecord = (char*)GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
                    ((PlayerGroundParameters*)playerGroundRecord)->lieSlot = 1;
                }
            } else {
                playerGroundRecord = (char*)GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
                ((PlayerGroundParameters*)playerGroundRecord)->lieSlot = 0;
            }
            ((CGolfBall*)ball)->m_state = 0;
            Vec3_Zero(ball + COURSE_OFFSET(CGolfBall, m_velocity));
        }
    } else {
        // Ordinary bounce splits velocity into reversed normal and tangent components, each damped separately.
        float nonpositiveSpin = ((CGolfBall*)ball)->m_spin;
        nonpositiveSpin = kFloat_0_805DF744 > nonpositiveSpin ? nonpositiveSpin : kFloat_0_805DF744;
        ((CGolfBall*)ball)->m_spin = nonpositiveSpin;
        float normalVelocityDot = __ml__FRC7WVectorRC7WVector(ball + COURSE_OFFSET(CGolfBall, m_velocity), normal);
        __ml__FfRC7WVector(reversedNormalVelocity, normal, kFloat_neg1_QuadTree * normalVelocityDot);
        __pl__FRC7WVectorRC7WVector(tangentVelocity, ball + COURSE_OFFSET(CGolfBall, m_velocity), reversedNormalVelocity);
        int weatherCode = ((SharedCollisionRules*)g_sharedDoc)->weatherCode;
        float materialWeatherScale;
        float spinDamping = FP_FMADDS(kFloat_0_1_BallPhysics, ((CGolfBall*)ball)->m_initSpin, kFloat_0_3_BallPhysics);
        SelectGroundBounceRestitution(materialWeatherScale, weatherCode, quadTree, ball);
        float restitution = (AsCQuadTree(quadTree)->bounceScale * materialWeatherScale) * ((CourseMaterial*)CourseData_GetEntryByIndex((void*)AsCQuadTree(quadTree)->courseData, ((CGolfBall*)ball)->m_contactMaterialIndex))->restitution;
        __ml__FfRC7WVector(dampedNormalVelocity, reversedNormalVelocity, restitution);
        __ml__FfRC7WVector(dampedTangentVelocity, tangentVelocity, FP_FNMSUBS(spinDamping, (kFloat_1_QuadTree - restitution), kFloat_1_QuadTree));
        __pl__FRC7WVectorRC7WVector(reboundVelocity, dampedTangentVelocity, dampedNormalVelocity);
        __as__7WVectorFRC7WVector(ball + COURSE_OFFSET(CGolfBall, m_velocity), reboundVelocity);
    }
}

// Check material-specific speed and impact-angle thresholds for high-speed landing behavior.
extern "C" int CQuadTree_TestHighSpeedGroundImpact(void* self, void* normal) {
	void* obj = GolfBall__Fv();
	char* p = (char*)obj;
	char** pp = &p;
	float speed = (*(WVector*)(p + 0xc)).Magnitude();

	WVector vec = kFloat_neg1_QuadTree * *(WVector*)(p + 0xc);
	float impactCosine = (vec * *(WVector*)normal) / speed;
	int buf[3];
	struct PositionPair { int words[2]; };
	*(PositionPair*)buf = *(PositionPair*)obj;
	buf[2] = *(int*)((char*)obj + 8);
	int groundType = GetGroundType__9CQuadTreeF7WVector(self, buf);
	if (groundType == 3) {
		if (speed > kFloat_50_QuadTree) {
			float threshold = *(float*)(s_quadTreeCollisionState + 4);
			if (impactCosine > threshold)
				return 1;
		}
	}
	if (groundType == 10) {
		if (speed > kFloat_50_QuadTree) {
			float threshold = *(float*)(s_quadTreeCollisionState + 4);
			if (impactCosine > threshold)
				return 1;
		}
	}
	if (groundType == 12) {
		if (speed > kFloat_50_QuadTree) {
			float threshold = *(float*)(s_quadTreeCollisionState + 4);
			if (threshold != kFloat_0_805DF744)
				return 1;
		}
	}
	return 0;
}

// Apply material-dependent object response and append hit metadata to the collision-event history.
// displacement is the whole step; hitFraction selects contact along it. normal points
// toward the ball. Existing-contact paths require a nonempty collision-event vector.
extern "C" void CQuadTree_ApplyBallMaterialCollision(Self5EB0* self, void* displacement, void* normal, float hitFraction) {
	float cBack[3], tangentialVelocity[3], aZero[3], aStep[3], bStep[3];
	float cC[3], cB[3], cA[3], cStep[3];
	float side[3], axis[3], up[3];
	char* ball = (char*)GolfBall__Fv();
	char* collisionHistory = (char*)CGolfBall_GetOrAllocatePreDataAt(ball, ((CGolfBall*)ball)->m_preDataCount);
	__as__7WVectorFRC7WVector(collisionHistory + 0x64, normal);
	char* name = (char*)CQuadTree_GetBounceSound(self, ((CGolfBall*)ball)->m_contactMaterialIndex);
	if (*(signed char*)name) {
		void* current = *(void**)((char*)SharedDoc_GetCurrent()
			+ *(unsigned char*)((char*)GolfDoc_GetCurrent() + 0x3C1) * 0x1D4 + 0x88);
		if (ItemInventory_GetBallIdAt(SharedDoc_GetItemInventory(), (void*)0xB) == current)
			sprintf(collisionHistory + 0x20, courseObjectSoundStrings.extraTextureFormat, rand() % 4 + 1);
		else
			strcpy(collisionHistory + 0x20, name);
	}
	// Choose pass-through/material-flag response, rolling reversal, or an airborne normal/tangent bounce.
	if (CQuadTree_IsPenetrable(self, ((CGolfBall*)ball)->m_contactMaterialIndex)) {
		float* pz = Vec3_Set(aZero, kFloat_0_805DF744, kFloat_0_805DF744, kFloat_0_805DF744);
		if (Vec3_NearlyEqualWithTolerance(displacement, pz, kFloat_0_00001_QuadTree)) return;
		float len = Magnitude__7WVectorCFv(displacement);
		__ml__FfRC7WVector(aStep, displacement, hitFraction + CGolfBall_GetRadius(ball) / len);
		__apl__7WVectorFRC7WVector(ball, aStep);
		__amu__7WVectorFf(ball + 0xC, CQuadTree_GetBounceCoeff(self, ((CGolfBall*)ball)->m_contactMaterialIndex));
		__as__7WVectorFRC7WVector((char*)&((QuadTreeGeometryState*)self)->lastHitPosition, ball);
		__as__7WVectorFRC7WVector((char*)&((QuadTreeGeometryState*)self)->lastHitDirection, ball + 0xC);
		CollisionEventVector_PushBack(&self->vec, &self->item);
		return;
	}
	if (((CGolfBall*)ball)->m_state == kBallRolling) {
		__ml__FfRC7WVector(bStep, displacement, hitFraction);
		__apl__7WVectorFRC7WVector(ball, bStep);
		__amu__7WVectorFf(ball + 0xC, kFloat_neg1_QuadTree * CQuadTree_GetBounceCoeff(self, ((CGolfBall*)ball)->m_contactMaterialIndex));
		__as__7WVectorFRC7WVector((char*)&((QuadTreeGeometryState*)self)->lastHitPosition, ball);
		__as__7WVectorFRC7WVector((char*)&((QuadTreeGeometryState*)self)->lastHitDirection, ball + 0xC);
		CollisionEventVector_PushBack(&self->vec, &self->item);
		return;
	}
	if (((CGolfBall*)ball)->m_lastContactIndex == -1) {
		float d = __ml__FRC7WVectorRC7WVector(ball + 0xC, normal);
		__ml__FfRC7WVector(cBack, normal, kFloat_neg1_QuadTree * d);
		__pl__FRC7WVectorRC7WVector(tangentialVelocity, ball + 0xC, cBack);
		float k = FP_FMADDS(kFloat_0_1_BallPhysics, ((CGolfBall*)ball)->m_initSpin, kFloat_0_3_BallPhysics);
		__ml__FfRC7WVector(cA, cBack, CQuadTree_GetBounceCoeff(self, ((CGolfBall*)ball)->m_contactMaterialIndex));
		__ml__FfRC7WVector(cB, tangentialVelocity, FP_FNMSUBS(k, (kFloat_1_QuadTree - CQuadTree_GetBounceCoeff(self, ((CGolfBall*)ball)->m_contactMaterialIndex)), kFloat_1_QuadTree));
		__pl__FRC7WVectorRC7WVector(cC, cB, cA);
		__as__7WVectorFRC7WVector(ball + 0xC, cC);
		if (((CGolfBall*)ball)->m_flags & 0x10) __amu__7WVectorFf(ball + 0xC, kFloat_0_25_QuadTree);
		if (((CGolfBall*)ball)->m_flags & kSpecialFlightB) {
			if (((CGolfBall*)ball)->m_eventPreDataIndex[kHistorySpecialFlightB] >= 0 && ((CGolfBall*)ball)->m_preDataCount > ((CGolfBall*)ball)->m_eventPreDataIndex[kHistorySpecialFlightB])
				__amu__7WVectorFf(ball + 0xC, kFloat_0_25_QuadTree);
		}
		__ml__FfRC7WVector(cStep, displacement, hitFraction);
		__apl__7WVectorFRC7WVector(ball, cStep);
		__as__7WVectorFRC7WVector((char*)&((QuadTreeGeometryState*)self)->lastHitPosition, ball);
		__as__7WVectorFRC7WVector((char*)&((QuadTreeGeometryState*)self)->lastHitDirection, ball + 0xC);
		CollisionEventVector_PushBack(&self->vec, &self->item);
	} else {
		((CGolfBall*)ball)->m_lastContactId = *(short*)((char*)Array_GetLastElement36((fn_801D6928_data*)&self->vec) + 2);
		((CGolfBall*)ball)->m_lastContactIndex = *(int*)((char*)Array_GetLastElement36((fn_801D6928_data*)&self->vec) + 4);
		int ids = ((CGolfBall*)ball)->m_lastContactId;
		int id = ((CGolfBall*)ball)->m_lastContactIndex;
		char* obj = *(char**)&AsCQuadTree(self)->courseData;
		char* grid = *(char**)(obj + 0xA4) + ids * kSurfaceStride;
		char* item = *(char**)(grid + 0xB0) + id * kTriangleStride;
		if (Float_IsAbsBelow(item + 0x28, kFloat_0_00001_QuadTree)) {
			Vec3_Zero(ball + 0xC);
			((CGolfBall*)ball)->m_state = kBallStopped;
		} else {
			((CGolfBall*)ball)->m_pos.y = kFloat_neg1_QuadTree * FP_FMADDS(((CGolfBall*)ball)->m_pos.z, ((CourseTriangle*)item)->normal.z,
				FP_FMADDS(((CGolfBall*)ball)->m_pos.x, ((CourseTriangle*)item)->normal.x, ((CourseTriangle*)item)->planeConstant)) / ((CourseTriangle*)item)->normal.y;
			((CGolfBall*)ball)->m_pos.y += CGolfBall_GetRadius(ball);
		}
	}
	// After recording/resolving contact, use a fixed gravity tick to decide whether contact persists.
	float* pu = Vec3_Set(up, kFloat_0_805DF744, kFloat_34_295296_QuadTree, kFloat_0_805DF744);
	__ml__FfRC7WVector(axis, pu, kFloat_0_033333335);
	__mi__FRC7WVectorRC7WVector(side, ball + 0xC, axis);
	if (__ml__FRC7WVectorRC7WVector(side, normal) < kFloat_0_805DF744) {
		if (*(float*)((char*)normal + 4) > Float_Get((float*)s_quadTreeCollisionState)) {
			Vec3_Zero(ball + 0xC);
			((CGolfBall*)ball)->m_state = kBallStopped;
			return;
		}
		if (((CGolfBall*)ball)->m_velocity.y > kFloat_0_805DF744) Vec3_NegateInPlace(ball + 0xC);
		((CGolfBall*)ball)->m_lastContactId = *(short*)((char*)Array_GetLastElement36((fn_801D6928_data*)&self->vec) + 2);
		((CGolfBall*)ball)->m_lastContactIndex = *(int*)((char*)Array_GetLastElement36((fn_801D6928_data*)&self->vec) + 4);
	}
}

extern "C" {
#pragma push
#pragma opt_unroll_loops off
#pragma inline_depth(16)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(20000)
#pragma use_lmw_stmw on
}
extern "C" {
struct Alloc6360 {
	// Placement-copy one collision event into already allocated vector storage.
	inline void construct(fn_801D6360_item* p, const fn_801D6360_item& value) const {
		new (p) fn_801D6360_item(value);
	}
	// Placement-copy one collision event into already allocated vector storage.
	inline void construct(fn_801D6360_item* p, fn_801D6360_item& value) const {
		new (p) fn_801D6360_item(value);
	}
};

extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct Buffer6360 {
	void* data;
	void* allocator;
	// Guard an exception-message allocation until a shared owner takes it.
	inline Buffer6360(void* p, void* a) : data(p), allocator(a) {}
	// Free an exception-message allocation that was not transferred.
	inline ~Buffer6360() { if (data) __dla__FPv(data); }
};
struct ErrorString6360 {
	char* message;
	void* owner;
	inline ErrorString6360(const char* const& text);
};
}
namespace std {
struct logic_error : exception {
	ErrorString6360 value;
	inline logic_error(const char* text);

    ~logic_error();
};
}
extern "C" {
struct Owner6360 {
	void* vtable;
	int references;
	int owners;
	void* data;
	// Create the shared-character owner for the supplied message allocation.
	inline Owner6360(void* p) {
		references = 1;
		owners = 1;
		vtable = s_sharedCharDeleterVtable;
		data = p;
	}
};
}
// Allocate/copy the message and transfer it to its shared-character owner.
inline ErrorString6360::ErrorString6360(const char* const& text) {
	Alloc6360 allocator = {};
	unsigned int n = strlen(text) + 1;
	char* p = (char*)__nwa__FUl(n * 2 - n);
	message = p;
	{
		Buffer6360 buffer(p, &allocator);
		owner = new Owner6360(p);
		buffer.data = 0;
	}
	strcpy(message, text);
}
// Initialize the error message while installing the logic_error vtable.
inline std::logic_error::logic_error(const char* text)
	: value((*(void**)this = __vt__Q23std11logic_error, text)) {}
namespace std {
struct length_error : logic_error {
	// Construct the logic_error base, then select the length_error vtable.
	inline length_error(const char* text) : logic_error(text) {
		*(void**)this = __vt__Q23std12length_error;
	}
	~length_error();
};
}
extern "C" {
// Subtract the transferred/destroyed element count from the retained size word.
static inline void reduce6360(unsigned int& value, const unsigned int& count) {
	value -= count;
}
struct Storage6360 {
	fn_801D6360_item* data;
	unsigned int size, capacity;
	unsigned int* allocator;
	// Old elements before the insertion point that still need to be moved.
	unsigned int prefix;
	// Initialize an empty growth buffer borrowing the original capacity/allocator word.
	inline Storage6360(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), prefix(0) {}
	// Free whichever allocation remains in the growth guard after a swap or unwind.
	inline ~Storage6360() {
		if (data) {
			unsigned int count = size;
			Alloc6360 allocator = {};
			reduce6360(size, count);
			appFreeUni__FPv(data);
		}
	}
};

// The length checks use the same shared-string exception payload as the STL.
#define LENGTH_ERROR_6360() \
	{ Alloc6360 allocator = {}; throw std::length_error(courseObjectSoundStrings.vectorLengthError); }


// Copy count appended events into guarded storage and advance its constructed size.
static inline void fill6360(Storage6360& storage,
	const fn_801D6360_item* item, unsigned int count) {
	Alloc6360 allocator = {};
	fn_801D6360_item* dst = storage.data + storage.prefix + storage.size;
	while (count != 0) {
		allocator.construct((dst), *(item));
		--count;
		++dst;
		++storage.size;
	}
}

// Copy old events backward into the new prefix, tracking constructed entries for cleanup.
static inline void move6360(Storage6360& storage, fn_801D6360_item* begin,
	fn_801D6360_item* src) {
	Alloc6360 allocator = {};
	fn_801D6360_item* dst = storage.data + storage.prefix;
	while ((unsigned int)src > (unsigned int)begin) {
		--dst;
		--src;
		allocator.construct((dst), *(src));
		--storage.prefix;
		++storage.size;
	}
}

// Select the larger growth amount by reference.
static inline const unsigned int& max6360(const unsigned int& a, const unsigned int& b) {
	return a < b ? b : a;
}

// Compute bounded vector growth in element counts, throwing on size overflow.
static inline unsigned int nextCapacity6360(unsigned int oldCapacity, unsigned int needed) {
	if (needed > 0x071C71C7U - oldCapacity) LENGTH_ERROR_6360();
	if (oldCapacity < 0x025ED097U) {
		unsigned int n = oldCapacity + 1;
		unsigned int growth = (n * 4 - n) / 5;
		return oldCapacity + max6360(growth, needed);
	} else if (oldCapacity < 0x04BDA12EU) {
		unsigned int growth = (oldCapacity + 1) >> 1;
		return oldCapacity + max6360(growth, needed);
	} else return 0x071C71C7;
}

// Append into a new event allocation, transfer old elements and swap storage ownership.
static inline void grow6360(fn_801D6360_vector* self, const fn_801D6360_item* item) {
	Alloc6360 allocator = {};
	Alloc6360 maxAllocator = {};
	if (0x071C71C7U - self->capacity < 1U) LENGTH_ERROR_6360();
	unsigned int needed = self->size + 1 - self->capacity;
	Storage6360 storage(&self->capacity);
	unsigned int newCapacity = nextCapacity6360(self->capacity, needed);
	Alloc6360 allocationAllocator = {};
	if (newCapacity > 0x071C71C7U) LENGTH_ERROR_6360();
	storage.data = (fn_801D6360_item*)appMallocDummy__FUl(newCapacity * 0x24);
	storage.capacity = newCapacity;
	storage.prefix = self->size;
	// Construct the appended value before moving the old range: item may alias it.
	fill6360(storage, item, 1);
	move6360(storage, self->data, self->data + self->size);
	// All new events are constructed; swap owned storage so the guard frees only the former allocation.
	unsigned int newSize = storage.size;
	fn_801D6360_item* newData = storage.data;
	fn_801D6360_item* oldData = self->data;
	self->size = 0;
	unsigned int oldCapacity = self->capacity;
	newCapacity = storage.capacity;
	storage.data = oldData;
	Alloc6360 swapAllocator = {};
	self->capacity = newCapacity;
	storage.capacity = oldCapacity;
	self->data = newData;
	self->size = newSize;
	storage.size = 0;
}

// Copy one event into owned storage, growing first when no spare slot remains.
void CollisionEventVector_PushBack(fn_801D6360_vector* self, const fn_801D6360_item* item) {
	Alloc6360 allocator = {};
	if (self->size < self->capacity) {
		fn_801D6360_item* dst = self->data + self->size;
		allocator.construct((dst), *(item));
		self->size++;
	} else {
		grow6360(self, item);
	}
}

#undef LENGTH_ERROR_6360
#pragma pop
}

// Return the final 36-byte collision-event record; callers must supply a nonempty array.
extern "C" void* Array_GetLastElement36(fn_801D6928_data* self) {
	return (char*)self->data + (self->count - 1) * 0x24;
}

// Reserved Wii hook with no work in this build.
extern "C" void CQuadTree_ReservedWiiHook() {}

// Project a point onto each triangle edge, clamp to the segments and return the nearest result.
extern "C" void Triangle_ClosestPointOnEdges(void* out, void* unused, void* point, void* vertex0, void* vertex1, void* vertex2) {
	Arr3 v0, v1, v2;
	Arr3 t0, t1, t2;
	Arr3 e, d, b, c, a;
	Arr3 j, i, g, h, f;
	Arr3 o, n, l, m, k;
	__mi__FRC7WVectorRC7WVector(&a, point, vertex0);
	__mi__FRC7WVectorRC7WVector(&b, vertex1, vertex0);
	c = *(Arr3*)(Normalize__7WVectorFv(&b));
	__mi__FRC7WVectorRC7WVector(&d, vertex0, vertex1);
	float len = WVectorLen__FRC7WVector(&d);
	float dot = __ml__FRC7WVectorRC7WVector(&c, &a);
	if (dot < kFloat_0_805DF744) Copy3(&v0, vertex0);
	else if (dot > len) Copy3(&v0, vertex1);
	else { __ml__FfRC7WVector(&e, &c, dot); __as__7WVectorFRC7WVector(&c, &e); __pl__FRC7WVectorRC7WVector(&v0, vertex0, &c); }

	__mi__FRC7WVectorRC7WVector(&f, point, vertex1);
	__mi__FRC7WVectorRC7WVector(&g, vertex2, vertex1);
	h = *(Arr3*)(Normalize__7WVectorFv(&g));
	__mi__FRC7WVectorRC7WVector(&i, vertex1, vertex2);
	len = WVectorLen__FRC7WVector(&i);
	dot = __ml__FRC7WVectorRC7WVector(&h, &f);
	if (dot < kFloat_0_805DF744) Copy3(&v1, vertex1);
	else if (dot > len) Copy3(&v1, vertex2);
	else { __ml__FfRC7WVector(&j, &h, dot); __as__7WVectorFRC7WVector(&h, &j); __pl__FRC7WVectorRC7WVector(&v1, vertex1, &h); }

	__mi__FRC7WVectorRC7WVector(&k, point, vertex2);
	__mi__FRC7WVectorRC7WVector(&l, vertex0, vertex2);
	m = *(Arr3*)(Normalize__7WVectorFv(&l));
	__mi__FRC7WVectorRC7WVector(&n, vertex2, vertex0);
	len = WVectorLen__FRC7WVector(&n);
	dot = __ml__FRC7WVectorRC7WVector(&m, &k);
	if (dot < kFloat_0_805DF744) Copy3(&v2, vertex2);
	else if (dot > len) Copy3(&v2, vertex0);
	else { __ml__FfRC7WVector(&o, &m, dot); __as__7WVectorFRC7WVector(&m, &o); __pl__FRC7WVectorRC7WVector(&v2, vertex2, &m); }

	__mi__FRC7WVectorRC7WVector(&t0, &v0, point); float x = WVectorLen__FRC7WVector(&t0);
	__mi__FRC7WVectorRC7WVector(&t1, &v1, point); float y = WVectorLen__FRC7WVector(&t1);
	__mi__FRC7WVectorRC7WVector(&t2, &v2, point); float z = WVectorLen__FRC7WVector(&t2);
	if (x < y) { if (x < z) *(Arr3*)out = v0; else *(Arr3*)out = v2; }
	else { if (y < z) *(Arr3*)out = v1; else *(Arr3*)out = v2; }
}

// Solve |movingOrigin + t*displacement - fixedTarget|^2 = ballRadius^2 for the earliest accepted root.
// Inputs are borrowed world vectors. Return 1 on a miss; a root at 1 is excluded.
// The first root accepts zero, but the fallback root must be strictly positive.
// The code negates displacement so targetFromOrigin + t*reverseDisplacement gives the same squared distance.
// A zero-length displacement follows the existing division behavior; no validation is added.
extern "C" float CQuadTree_IntersectBallSweepWithPoint(void* unused, void* fixedTarget, void* movingOrigin, void* displacement) {

	float roots[2];
	float neg = kFloat_neg1_QuadTree;
	WVector reverseDisplacement = neg * *(WVector*)displacement;
	WVector targetFromOrigin = *(WVector*)fixedTarget - *(WVector*)movingOrigin;
	float displacementLengthSquared = (reverseDisplacement * reverseDisplacement);
	float targetDisplacementDot = (targetFromOrigin * reverseDisplacement);
	void* g = GolfBall__Fv();
	float radiusA = ((CGolfBall*)g)->m_radius;
	g = GolfBall__Fv();
	float radiusB = ((CGolfBall*)g)->m_radius;
	float targetDistanceSquared = (targetFromOrigin * targetFromOrigin);
	float scaledQuadraticConstant = FP_FNMSUBS(radiusB, radiusA, targetDistanceSquared);
	scaledQuadraticConstant = displacementLengthSquared * scaledQuadraticConstant;
	float disc = FP_FMSUBS(targetDisplacementDot, targetDisplacementDot, scaledQuadraticConstant);
	if (disc < kFloat_0_805DF744)
		return kFloat_1_QuadTree;
	float sqrtDiscriminant = Math_SquareRootApprox(disc);
	roots[0] = FP_FMSUBS(kFloat_neg1_QuadTree, targetDisplacementDot, sqrtDiscriminant) / displacementLengthSquared;
	float sqrtDiscriminantAgain = Math_SquareRootApprox(disc);
	roots[1] = FP_FMADDS(kFloat_neg1_QuadTree, targetDisplacementDot, sqrtDiscriminantAgain) / displacementLengthSquared;
	if (roots[0] < kFloat_0_805DF744) {
		if (roots[1] > kFloat_0_805DF744 && roots[1] < kFloat_1_QuadTree)
			return roots[1];
	} else {
		if (roots[0] < kFloat_1_QuadTree)
			return roots[0];
	}
	return kFloat_1_QuadTree;
}

// Resolve cup-floor, pole and rim contacts, update bounce events, and encode a holed ball when it crosses the floor.
extern "C" void CQuadTree_UpdateBallCupCollision(void* self, float delta) {
	typedef NormalWord6 V;
	V radial, move;
	struct Times { volatile float fraction; float boundaryHit; volatile float hit; } times;
	V normal, boundary, displacement;
	char* geom=(char*)GolfBall__Fv();
	char* state=(char*)CGolfBall_GetOrAllocatePreDataAt(geom,((CGolfBall*)geom)->m_preDataCount);
	((CGolfBall*)geom)->m_velocity.y=FP_FNMSUBS(kFloat_34_295296_QuadTree, delta, ((CGolfBall*)geom)->m_velocity.y);
	{ V t; __ml__FRC7WVectorf(&t,geom + COURSE_OFFSET(CGolfBall, m_velocity),delta); __as__7WVectorFRC7WVector(&move,&t); }
	float geomRadius=((CGolfBall*)geom)->m_radius;
	float height=(((CGolfBall*)geom)->m_pos.y-geomRadius)-((QuadTreeGeometryState*)self)->cupFloorY;
	float heightAfter=height+((Vec3f*)&move)->y;
	if(height*heightAfter<kFloat_0_805DF744 && ((CGolfBall*)geom)->m_velocity.y<kFloat_0_805DF744) times.fraction=height/(height-heightAfter);
	else times.fraction=kFloat_1_QuadTree;
	__amu__7WVectorFf(&move,times.fraction);
	times.hit=CQuadTree_SweepBallAgainstTriangles(self,geom,(float*)&move,(int*)(geom+0x28),0);
	if(((BallGroundTriangleView*)geom)->groundTriangleIndex>=0) ((CGolfBall*)geom)->m_contactMaterialIndex=((CourseTriangle*)(*(char**)&((CourseSurface*)(*(char**)&AsCQuadTree(self)->groundSurface))->triangles+((BallGroundTriangleView*)geom)->groundTriangleIndex* kTriangleStride))->materialIndex;
	{ V t,center; CopyCupCenterWords(center); __mi__FRC7WVectorRC7WVector(&t,geom,&center); __as__7WVectorFRC7WVector(&radial,&t); }
	((Vec3f*)&radial)->y=kFloat_0_805DF744;
	float radius=((CGolfBall*)geom)->m_radius;
	volatile float after,before;
	before=Magnitude__7WVectorCFv(&radial)-(AsCQuadTree(self)->cupRadius-radius);
	{ V u,t,center; CopyCupCenterWords(center); __pl__FRC7WVectorRC7WVector(&t,geom,&move); __mi__FRC7WVectorRC7WVector(&u,&t,&center); __as__7WVectorFRC7WVector(&radial,&u); }
	((Vec3f*)&radial)->y=kFloat_0_805DF744;
	radius=((CGolfBall*)geom)->m_radius;
	after=Magnitude__7WVectorCFv(&radial)-(AsCQuadTree(self)->cupRadius-radius);
	// Compare the floor-limited step with ground and horizontal cup boundaries; preserve branch ordering.
	if(((BallCollisionHistory*)((void*(*)(void*,int))PreData__9CGolfBallFi)(geom,((CGolfBall*)geom)->m_preDataCount-1))->motionState==kBallAirborne && before>kFloat_0_805DF744) {
		float cmpRadius=((CGolfBall*)geom)->m_radius; if(after>cmpRadius) {
			V t; __ml__FfRC7WVector(&t,&move,times.hit); __apl__7WVectorFRC7WVector(geom,&t);
			if(times.hit<kFloat_1_QuadTree) { V n; NormalOut6(&n,self,((BallGroundTriangleView*)geom)->groundTriangleIndex); CQuadTree_HandleBallGroundBounce(self,&n); }
		} else {
			__apl__7WVectorFRC7WVector(geom,&move);
			float penetration=after;
			if(penetration>kFloat_0_805DF744) {
				float r=((CGolfBall*)geom)->m_radius; V ground; V point=*(V*)geom;
				float edge=Math_SquareRootApprox(FP_FMSUBS(r, r, (r-penetration)*(r-penetration)));
				GetGroundPoint__9CQuadTreeF7WVectorbPib(&ground,self,(void**)&point,0,0,0);
				float y=((Vec3f*)&ground)->y+edge;
				{ V center; CopyCupCenterWords(center); __mi__FRC7WVectorRC7WVector(&boundary,geom,&center); }
				((Vec3f*)&boundary)->y=kFloat_0_805DF744;
				Normalize__7WVectorFv(&boundary); __amu__7WVectorFf(&boundary,AsCQuadTree(self)->cupRadius);
				{ V center; CopyCupCenterWords(center); __apl__7WVectorFRC7WVector(&boundary,&center); }
				{ V t; __mi__FRC7WVectorRC7WVector(&t,geom,&boundary); __as__7WVectorFRC7WVector(&normal,Normalize__7WVectorFv(&t)); }
				if(((CGolfBall*)geom)->m_pos.y<y) {
					((CGolfBall*)geom)->m_flags&=~0x73; ((CGolfBall*)geom)->m_spin=kFloat_0_805DF744; ((CGolfBall*)geom)->m_curve=kFloat_0_805DF744; ((CGolfBall*)geom)->m_pos.y=y;
					float dot=__ml__FRC7WVectorRC7WVector(geom + COURSE_OFFSET(CGolfBall, m_velocity),&normal); dot=dot>kFloat_0_805DF744?dot:-dot;
					V t; __ml__FfRC7WVector(&t,&normal,kFloat_1_3_QuadTree*dot); __apl__7WVectorFRC7WVector(geom + COURSE_OFFSET(CGolfBall, m_velocity),&t);
					state[0x61]=0; state[0x60]=3; __as__7WVectorFRC7WVector(state+0x64,&normal); strcpy(state+0x20,courseObjectSoundStrings.poleCollisionSound);
					return;
				}
			}
		}
	} else if(after<kFloat_0_805DF744) {
		__apl__7WVectorFRC7WVector(geom,&move);
		if(height*heightAfter<kFloat_0_805DF744 && ((CGolfBall*)geom)->m_velocity.y<kFloat_0_805DF744) Stop6();
	} else {
		V input=move;
		times.boundaryHit=((float(*)(void*,float*))CQuadTree_IntersectBallPathWithCupXZ)(self,(float*)&input);
		float boundaryTime=times.boundaryHit;
		float collisionTime=times.hit;
		if(boundaryTime<collisionTime) {
			V t; __ml__FfRC7WVector(&t,&move,collisionTime); __apl__7WVectorFRC7WVector(geom,&t);
			if(times.hit<kFloat_1_QuadTree) { V n; NormalOut6(&n,self,((BallGroundTriangleView*)geom)->groundTriangleIndex); CQuadTree_HandleBallGroundBounce(self,&n); }
		} else if(boundaryTime<kFloat_1_QuadTree) {
			((CGolfBall*)geom)->m_flags&=~0x73;
			V t; __ml__FfRC7WVector(&t,&move,times.boundaryHit); __apl__7WVectorFRC7WVector(geom,&t);
			{ V center; CopyCupCenterWords(center); __mi__FRC7WVectorRC7WVector(&displacement,&center,geom); }
			((Vec3f*)&displacement)->y=kFloat_0_805DF744; Normalize__7WVectorFv(&displacement);
			float speed=Magnitude__7WVectorCFv(geom + COURSE_OFFSET(CGolfBall, m_velocity));
			V bounce; __ml__FfRC7WVector(&bounce,&displacement,kFloat_0_3_BallPhysics*speed); __as__7WVectorFRC7WVector(geom + COURSE_OFFSET(CGolfBall, m_velocity),&bounce);
			state[0x61]=0; state[0x60]=3; __as__7WVectorFRC7WVector(state+0x64,&displacement); strcpy(state+0x20,courseObjectSoundStrings.poleCollisionSound);
		} else __apl__7WVectorFRC7WVector(geom,&move);
	}
	{ V t,center; CopyCupCenterWords(center); __mi__FRC7WVectorRC7WVector(&t,&center,geom); __as__7WVectorFRC7WVector(&radial,&t); }
	((Vec3f*)&radial)->y=kFloat_0_805DF744;
	// After movement, correct rim penetration by height and history state, or clamp against the cup wall.
	float distance=Magnitude__7WVectorCFv(&radial);
	float finalRadius=((CGolfBall*)geom)->m_radius;
	if(distance>AsCQuadTree(self)->cupRadius-finalRadius) {
		if(((CGolfBall*)geom)->m_eventPreDataIndex[kHistoryRollingStart]<0) {
			float oldY=((CGolfBall*)geom)->m_pos.y; V saved=CenterValue6();
			if(oldY<((Vec3f*)&saved)->y) {
				if(times.fraction<kFloat_1_QuadTree) { Stop6(); return; }
				V t, origin=saved, scaled;
				__ml__FfRC7WVector(&scaled,Normalize__7WVectorFv(&radial),AsCQuadTree(self)->cupRadius-finalRadius);
				__mi__FRC7WVectorRC7WVector(&t,&origin,&scaled); __as__7WVectorFRC7WVector(geom,&t); ((CGolfBall*)geom)->m_pos.y=oldY;
				__amu__7WVectorFf(geom + COURSE_OFFSET(CGolfBall, m_velocity),kFloat_neg0_3_CupCollision);
				((CGolfBall*)geom)->m_velocity.y=((CGolfBall*)geom)->m_velocity.y*kFloat_neg1_QuadTree;
				((CGolfBall*)geom)->m_flags&=~0x73;
			} else {
				V origin=saved;
				if(oldY<((Vec3f*)&origin)->y+finalRadius) {
					if(distance>AsCQuadTree(self)->cupRadius) {
						V ground, point=*(V*)geom; finalRadius=((CGolfBall*)geom)->m_radius;
						GetGroundPoint__9CQuadTreeF7WVectorbPib(&ground,self,(void**)&point,0,0,0);
						float y=((Vec3f*)&ground)->y+finalRadius;
						if(((CGolfBall*)geom)->m_pos.y<y) {
							((CGolfBall*)geom)->m_flags&=~0x70; ((CGolfBall*)geom)->m_pos.y=y;
							int idx=CQuadTree_GetMeshIndex(self,geom,0);
							float* mat=((float*(*)(void*,int))CourseData_GetEntryByIndex)((*(char**)&AsCQuadTree(self)->courseData),((CourseTriangle*)(*(char**)&((CourseSurface*)(*(char**)&AsCQuadTree(self)->groundSurface))->triangles+idx* kTriangleStride))->materialIndex);
							float factor=kFloat_neg1_QuadTree*mat[0];
							float velocity=((CGolfBall*)geom)->m_velocity.y;
							float zero=kFloat_0_805DF744,one=kFloat_1_QuadTree;
							((CGolfBall*)geom)->m_velocity.y=velocity*factor;
							state[0x61]=1; state[0x60]=1;
							float up[3]; up[0]=zero; up[1]=one; up[2]=zero; __as__7WVectorFRC7WVector(state+0x64,up);
							idx=CQuadTree_GetMeshIndex(self,geom,0);
							int material=((CourseTriangle*)(*(char**)&((CourseSurface*)(*(char**)&AsCQuadTree(self)->groundSurface))->triangles+idx* kTriangleStride))->materialIndex;
							char* name;
							if(AsCQuadTree(self)->simulationMode) name=&emptySimulationSurfaceName;
							else name=(char*)((void*(*)(void*,int))CourseData_GetEntryByIndex)((*(char**)&AsCQuadTree(self)->courseData),material)+0x25;
							if(*(signed char*)name) strcpy(state+0x20,name);
						}
					} else {
						V ground, point=*(V*)geom;
						float d=AsCQuadTree(self)->cupRadius-distance;
						float edge=Math_SquareRootApprox(FP_FMSUBS(finalRadius, finalRadius, d*d));
						GetGroundPoint__9CQuadTreeF7WVectorbPib(&ground,self,(void**)&point,0,0,0);
						float y=((Vec3f*)&ground)->y+edge;
						if(((CGolfBall*)geom)->m_pos.y<y) { ((CGolfBall*)geom)->m_pos.y=y;
						    __amu__7WVectorFf(geom + COURSE_OFFSET(CGolfBall, m_velocity),kFloat_neg0_3_CupCollision);
						    ((CGolfBall*)geom)->m_flags&=~0x73;
						    }
					}
				}
			}
		} else {
			float oldY=((CGolfBall*)geom)->m_pos.y; V t,center,scaled; CopyCupCenterWords(center);
			__ml__FfRC7WVector(&scaled,Normalize__7WVectorFv(&radial),AsCQuadTree(self)->cupRadius-finalRadius);
			__mi__FRC7WVectorRC7WVector(&t,&center,&scaled); __as__7WVectorFRC7WVector(geom,&t); ((CGolfBall*)geom)->m_pos.y=oldY;
			void* dir=Normalize__7WVectorFv(&radial); float speed=Magnitude__7WVectorCFv(geom + COURSE_OFFSET(CGolfBall, m_velocity));
			V bounce; __ml__FfRC7WVector(&bounce,dir,kFloat_0_3_BallPhysics*speed); __as__7WVectorFRC7WVector(geom + COURSE_OFFSET(CGolfBall, m_velocity),&bounce);
		}
	}
}

#pragma opt_propagation off
// Solve the horizontal ball path against the cup radius and return the first usable crossing fraction.
// displacement is IN/OUT: Y is set to zero. This is horizontal movement, not a point.
// The original root-selection branches are retained, including the contradictory
// negative-second-root test; do not repair them in a readability-only change.
extern "C" float CQuadTree_IntersectBallPathWithCupXZ(void* self, float* displacement) {
	void* game = g_sharedDoc;
	void* obj = ((void*)AsCSharedDoc(game)->golfDoc);
	unsigned char index = AsCGolfDoc(obj)->currentHoleIndex;
	void* entry = (char*)AsCGolfDoc(obj)->holeRecords + index * 0x20;
	float t[2];
	float basis[3];
	int tmpEntry[3];
	*(Arr3*)tmpEntry = *(Arr3*)((char*)entry + COURSE_OFFSET(HolePositionRecord, cupPosition));
	__mi__FRC7WVectorRC7WVector(basis, GolfBall__Fv(), tmpEntry);
	basis[1] = kFloat_0_805DF744;
	displacement[1] = kFloat_0_805DF744;
	float a = (*(WVector*)displacement * *(WVector*)displacement);
	float b = (*(WVector*)displacement * *(WVector*)basis);
	float v1 = ((CGolfBall*)GolfBall__Fv())->m_radius;
	float v2 = ((CGolfBall*)GolfBall__Fv())->m_radius;
	float c = (*(WVector*)basis * *(WVector*)basis);
	float x = AsCQuadTree(self)->cupRadius;
	float disc = FP_FMSUBS(b, b, a * FP_FNMSUBS((x - v2), (x - v1), c));
	if (disc < kFloat_0_805DF744) return kFloat_1_QuadTree;
	float root = Math_SquareRootApprox(disc);
	t[0] = FP_FMSUBS(kFloat_neg1_QuadTree, b, root) / a;
	root = Math_SquareRootApprox(disc);
	t[1] = FP_FMADDS(kFloat_neg1_QuadTree, b, root) / a;
	if (t[1] < kFloat_0_805DF744) {
		if (t[1] > kFloat_0_805DF744 && t[1] < kFloat_1_QuadTree) return t[1];
	} else {
		if (t[0] < kFloat_1_QuadTree) return t[0];
	}
	return kFloat_1_QuadTree;
}
#pragma opt_propagation reset

// Select the weather and ground-type adjustment applied to material lie percentages.
extern "C" void CQuadTree_SetCondition(void* self) {
	switch (((SharedCollisionRules*)g_sharedDoc)->weatherCode) {
	case 0:
	case 1:
		AsCQuadTree(self)->condition = kFloat_0_805DF744;
		break;
	case 2:
	case 3:
		switch (*((unsigned char*)GolfBall__Fv() + 0x59)) {
		case 0:
			AsCQuadTree(self)->condition = kFloat_neg2_BallPhysics;
			break;
		case 1:
			AsCQuadTree(self)->condition = kFloat_neg2_BallPhysics;
			break;
		case 4:
			AsCQuadTree(self)->condition = kFloat_neg3_QuadTree;
			break;
		case 3:
			AsCQuadTree(self)->condition = kFloat_neg2_BallPhysics;
			break;
		case 2:
			AsCQuadTree(self)->condition = kFloat_0_805DF744;
			break;
		default:
			AsCQuadTree(self)->condition = kFloat_neg3_QuadTree;
			break;
		}
		break;
	}
}

// Read restitution from the indexed course material record.
extern "C" float CQuadTree_GetBounceCoeff(const void* self, int materialIndex) {
	void* arg = *(void**)((const char*)self + 4);
	return *CourseData_GetEntryByIndex(arg, materialIndex);
}

#pragma opt_strength_reduction off
// Project a point to ground and return its material type, with tee and raised-region special cases.
extern "C" unsigned char GetGroundType__9CQuadTreeF7WVector(void* self, void* point) {
	char* game = (char*)g_sharedDoc;
	// A zero flag permits the tee-position special case before ordinary triangle/material lookup.
	if (AsCGolfDoc(AsCSharedDoc(game)->golfDoc)->unk_3CC == 0) {
		char* obj = ((char*)AsCSharedDoc(game)->golfDoc);
		Vec3f e = *(Vec3f*)(((char*)AsCGolfDoc(obj)->holeRecords) + AsCGolfDoc(obj)->currentHoleIndex * 0x20);
		if (fn_801D7F88_near(*(float*)point - e.x)) {
			char* obj2 = ((char*)AsCSharedDoc(game)->golfDoc);
			Vec3f e2 = *(Vec3f*)(((char*)AsCGolfDoc(obj2)->holeRecords) + AsCGolfDoc(obj2)->currentHoleIndex * 0x20);
			if (fn_801D7F88_near(((Vec3f*)point)->z - e2.z)) {
				return 0;
			}
		}
	}
	CQuadTree_GetMeshList(self, (float*)point);
	WVector tmp;
	int stack10;
	CQuadTree_FindHighestGroundPointInCandidates(&tmp, self, point, 1, (int)&stack10, 1);
	*(WVector*)point = tmp;
	int r9 = stack10;
	if (r9 < 0) return 9;
	int i;
	for (i = 0; i < (*(int*)&AsCQuadTree(self)->groundRegionCount); ++i) {
		Box7F88 rec = (*(Rec7F88**)&AsCQuadTree(self)->groundRegions)[i].b;
		if (*(float*)point > rec.v[0] && *(float*)point < rec.v[3] &&
		    ((Vec3f*)point)->z > rec.v[2] && ((Vec3f*)point)->z < rec.v[5] &&
		    ((Vec3f*)point)->y < rec.v[4]) {
			return 9;
		}
	}
	void* o = *(void**)&AsCQuadTree(self)->groundSurface;
	int idx = r9;
	void* table2 = *(void**)&((CourseSurface*)o)->triangles;
	void* item = (char*)table2 + idx * kTriangleStride;
	void* item2 = *(void**)&((CourseTriangle*)item)->materialIndex;
	void* r = ((void* (*)(void*, void*))CourseData_GetEntryByIndex)((*(void**)&AsCQuadTree(self)->courseData), item2);
	return ((CourseMaterial*)r)->groundType;
}
#pragma opt_strength_reduction reset

// Read bit 1 of the point's ground material flags; missing ground returns one.
extern "C" int CQuadTree_GetGroundFlagBit1AtPoint(void* self, void* arg) {
	int index = CQuadTree_GetMeshIndex(self, arg, 0);
	if (index < 0)
		return 1;
	void* obj = *(void**)&AsCQuadTree(self)->groundSurface;
	void* table = *(void**)&((CourseSurface*)obj)->triangles;
	void* item = *(void**)((char*)table + index * kTriangleStride + 0x3C);
	float* res = ((float* (*)(void*, void*))CourseData_GetEntryByIndex)((*(void**)&AsCQuadTree(self)->courseData), item);
	return (((unsigned int)((CourseMaterial*)res)->flags) >> 1) & 1;
}

// Read bit 0 of the indexed material flags through the original volatile course-data load.
extern "C" int CQuadTree_IsPenetrable(const void* self, int materialIndex) {
	void* arg = *(void* volatile*)((const char*)self + 4);
	return ((unsigned int)*(unsigned int*)((char*)CourseData_GetEntryByIndex(arg, materialIndex) + 0x20)) & 1;
}

// Return the upper lie fraction: (material percentage + adjustment) / 100 for the player's lie slot.
extern "C" float CQuadTree_GetMaxPowerOnGround(void* self) {
	void* manager = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	void* data = GolfPlayer_GetEntryByIndex(manager, AsCGolfDoc(manager)->currentPlayer);

	void* item = *(void**)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + (((BallGroundTriangleView*)GolfBall__Fv())->groundTriangleIndex) * kTriangleStride + 0x3C);
	float* values = ((float* (*)(void*, void*))CourseData_GetEntryByIndex)((*(void**)&AsCQuadTree(self)->courseData), item);
	unsigned char slot = ((PlayerGroundParameters*)data)->lieSlot;
	float* selected = (float*)((char*)values + (slot * 4));
	float result = selected[5];
	result = AsCQuadTree(self)->condition + result;
	return kFloat_0_01_quadTree * result;
}

// Return the lower lie fraction: (material percentage + adjustment) / 100 for the player's lie slot.
extern "C" float CQuadTree_GetMinPowerOnGround(void* self) {
	void* manager = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	void* data = GolfPlayer_GetEntryByIndex(manager, AsCGolfDoc(manager)->currentPlayer);
	void* item = *(void**)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + (((BallGroundTriangleView*)GolfBall__Fv())->groundTriangleIndex) * kTriangleStride + 0x3C);
	float* values = ((float* (*)(void*, void*))CourseData_GetEntryByIndex)((*(void**)&AsCQuadTree(self)->courseData), item);
	unsigned char slot = ((PlayerGroundParameters*)data)->lieSlot;
	float* selected = (float*)((char*)values + slot * 4);
	float result = selected[2];
	result = AsCQuadTree(self)->condition + result;
	return kFloat_0_01_quadTree * result;
}

// Return the adjusted upper lie fraction at a point; requires a valid ground triangle and lie slot.
extern "C" float CQuadTree_GetMaxPowerOnGroundAtPoint(void* self, void* point) {
	void* manager = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	void* data = GolfPlayer_GetEntryByIndex(manager, AsCGolfDoc(manager)->currentPlayer);

	void* item = *(void**)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + (CQuadTree_GetMeshIndex(self, point, 0)) * kTriangleStride + 0x3C);
	float* values = ((float* (*)(void*, void*))CourseData_GetEntryByIndex)((*(void**)&AsCQuadTree(self)->courseData), item);
	unsigned char slot = ((PlayerGroundParameters*)data)->lieSlot;
	float* selected = (float*)((char*)values + ((slot * 4)));
	float result = selected[5];
	result = AsCQuadTree(self)->condition + result;
	return kFloat_0_01_quadTree * result;
}

// Return the adjusted lower lie fraction at a point; requires a valid ground triangle and lie slot.
extern "C" float CQuadTree_GetMinPowerOnGroundAtPoint(void* self, void* point) {
	void* manager = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
	void* data = GolfPlayer_GetEntryByIndex(manager, AsCGolfDoc(manager)->currentPlayer);

	void* item = *(void**)((char*)*(void**)((char*)*(void**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + (CQuadTree_GetMeshIndex(self, point, 0)) * kTriangleStride + 0x3C);
	float* values = ((float* (*)(void*, void*))CourseData_GetEntryByIndex)((*(void**)&AsCQuadTree(self)->courseData), item);
	unsigned char slot = ((PlayerGroundParameters*)data)->lieSlot;
	float* selected = (slot + values);
	float result = selected[2];
	result = AsCQuadTree(self)->condition + result;
	return kFloat_0_01_quadTree * result;
}

// Return the material collision sound name, or the shared empty name in simulation mode.
extern "C" char* CQuadTree_GetBounceSound(void* self, int materialIndex) {
	if (AsCQuadTree(self)->simulationMode) {
		return &emptySimulationSurfaceName;
	}
	return (char*)CourseData_GetEntryByIndex((*(void**)&AsCQuadTree(self)->courseData), materialIndex) + 0x25;
}

// Test candidate object boxes and optional character-bone bounds, expanded by the requested scale.
extern "C" int CQuadTree_OverlappedWithObject(void* self, void* point, float scale) {
	float box1[6];
	Vec3_Zero(box1);
	Vec3_Zero(box1 + 3);
	float zero, ten, one;
	one = kFloat_1_QuadTree;
	zero = kFloat_0_805DF744;
	ten = kFloat_10_QuadTree;
	char* cell;
	for (unsigned char i = 0; i < AsCQuadTree(self)->candidateObjectCount; ++i) {
		cell = (char*)self + (unsigned int)i * 2;
		int index = *(short*)(cell + 0x2020);
		char* room = (char*)AsCourseData801C((*(void**)&AsCQuadTree(self)->courseData))->surfaces + index * kSurfaceStride;
		if ((*(unsigned int*)(room + 4) & 1) != 0) continue;
		void* node = *(void**)(room + 0x10);
		char* shape;
		char** found;
		int n = 0;
		found = (char**)WPuppet_FindNamedBoneEntry(node, courseObjectSoundStrings.firstCharacterName);
		if (found != 0) {
			while (found != 0) {
				float matrix[12];
				Matrix_MoveLastVectorToFront(matrix, found[0] + 0x78);
				V28c d, c, b, a;
				V28c ones(one, one, one);
				shape = found[1];
				__ml__FfRC7WVector(&a, &ones, scale);
				__mi__FRC7WVectorRC7WVector(&b, shape + 0xC, shape);
				__ml__FfRC7WVector(&c, &b, kFloat_0_5_QuadTree);
				__pl__FRC7WVectorRC7WVector(&d, &c, &a);
				AffineVectors_ScaleTrailingThree(matrix, &d);
				V28c e;
				((void (*)(void*, void*, void*))Matrix_TransformPoint)(&e, found[1] + 0x18, found[0] + 0x78);
				*(WVector*)matrix = *(WVector*)&e;
				if (OBB_ContainsPoint(matrix, point)) return 1;
				++n;
				found = (char**)WPuppet_FindNamedBoneEntry(*(void**)((char*)AsCourseData801C((*(void**)&AsCQuadTree(self)->courseData))->surfaces + *(short*)(cell + 0x2020) * kSurfaceStride + 0x10), (char*)((int (*)(char*, ...))String_FormatTemporary)(courseObjectSoundStrings.characterNameFormat, n));
			}
		} else {
			room = (char*)AsCourseData801C((*(void**)&AsCQuadTree(self)->courseData))->surfaces + *(short*)(cell + 0x2020) * kSurfaceStride;
			float y = ((Vec3f*)point)->y;
			if (y > *(float*)(room + 0x8C) || y < *(float*)(room + 0x80) - ten) continue;
			AABB_Copy(box1, room + 0x7C);
			V28c a;
			V28c v1(one, zero, one);
			__ml__FfRC7WVector(&a, &v1, scale);
			*(WVector*)box1 -= *(WVector*)&a;
			V28c b;
			V28c v2(one, zero, one);
			__ml__FfRC7WVector(&b, &v2, scale);
			*(WVector*)(box1 + 3) += *(WVector*)&b;
			if (*(float*)point > box1[0] && *(float*)point < box1[3] && ((Vec3f*)point)->z > box1[2] && ((Vec3f*)point)->z < box1[5]) return 1;
		}
	}
	return 0;
}

// Compute closest approach parameters for a ray and segment, preserving the original degenerate-case branches.
// Inputs describe rayOrigin+s*rayDirection (s >= 0) and segmentOrigin+t*segmentDirection (0 <= t <= 1).
// out is [s,t,world distance]; both direction inputs are displacements, not endpoint positions. Degenerate divisions
// follow the original branches; no new zero-length validation is introduced.
extern "C" void Geometry_ClosestRaySegmentParameters(void* out, void* unused, void* rayOrigin, void* rayDirection, void* segmentOrigin, void* segmentDirection) { float originDifference[3]; float f1;
	(void)unused;

	float closestDifference[3];
	float rayClosestPoint[3];
	float scaledRayDirection[3];
	float segmentClosestPoint[3];
	float scaledSegmentDirection[3];
	float directionDot; float rayOriginDot; float rayLengthSquared; float segmentOriginDot; float f31; float f4;
    // Before clamping: A=rayLengthSquared, B=directionDot, C=f31=segment length squared,
    // D=rayOriginDot, E=segmentOriginDot; determinant=A*C-B*B.
    // rayNumerator/segmentNumerator are parameter numerators; f4/f31 become their denominators after clipping.
    // f31 is deliberately reused: giving it one geometric name would misdescribe its later role.
	float determinant;
	float rayNumerator; float segmentNumerator;
	float f0; float f3; float f2;
	float s; float t;
	__mi__FRC7WVectorRC7WVector(originDifference, rayOrigin, segmentOrigin);
	rayLengthSquared = __ml__FRC7WVectorRC7WVector(rayDirection, rayDirection);
	directionDot = __ml__FRC7WVectorRC7WVector(rayDirection, segmentDirection);
	f31 = __ml__FRC7WVectorRC7WVector(segmentDirection, segmentDirection);
	rayOriginDot = __ml__FRC7WVectorRC7WVector(rayDirection, originDifference);
	segmentOriginDot = __ml__FRC7WVectorRC7WVector(segmentDirection, originDifference);
	f1 = __ml__FRC7WVectorRC7WVector(originDifference, originDifference);
	(void)f1;
	f4 = FP_FMSUBS(rayLengthSquared, f31, (directionDot * directionDot));
	{
		determinant = f4;

		if ((*(int*)&determinant) & 0x80000000) f1 = -f4; else f1 = f4;
	}
	int nearlyParallel = f1 < kFloat_0_00001_QuadTree;
	if (nearlyParallel) {
		f2 = kFloat_0_805DF744;
		f0 = segmentOriginDot;
		rayNumerator = kFloat_0_805DF744;
		segmentNumerator = segmentOriginDot;
	} else {
		f2 = FP_FMSUBS(directionDot, segmentOriginDot, f31 * rayOriginDot);
		f0 = FP_FMSUBS(rayLengthSquared, segmentOriginDot, directionDot * rayOriginDot);
		rayNumerator = f2;
		segmentNumerator = f0;
	}
	// Clamp the ray parameter at its origin, then the segment parameter at either endpoint.
	if (rayNumerator < kFloat_0_805DF744) {
		f2 = kFloat_0_805DF744;
		rayNumerator = kFloat_0_805DF744;
		segmentNumerator = segmentOriginDot;
		f0 = segmentOriginDot;
	} else {
		f31 = f4;
	}
	f1 = segmentNumerator;
	f3 = kFloat_0_805DF744;
	if (f1 < f3) {
		f2 = -rayOriginDot;
		segmentNumerator = f3;
		f0 = f3;
		// Keep the original comparison direction, including its unordered case.
		if (f2 < f3) {
			rayNumerator = f3;
			f2 = f3;
		} else {
			f4 = rayLengthSquared;
			rayNumerator = f2;
		}
	} else {
		if (f1 > f31) {
			 segmentNumerator = f31;
			f1 = -rayOriginDot;
			f0 = f31;
			f2 = f1 + directionDot;
			if (f2 < f3) {
				rayNumerator = f3;
				f2 = f3;
			} else {
				rayNumerator = f2;
				f4 = rayLengthSquared;
			}
		}
	}
	f3 = (*(int*)&rayNumerator & 0x80000000) ? -rayNumerator : rayNumerator;
	__typeof__(kFloat_0_00001_QuadTree > f3) savedLbl_805DF780 = (f3 < kFloat_0_00001_QuadTree);
	if ((savedLbl_805DF780)) {
		s = kFloat_0_805DF744;
	} else {
		f1 = rayNumerator;
		s = f1 / f4;
	}
	f0 = segmentNumerator;
	f2 = (*(int*)&segmentNumerator & 0x80000000) ? -segmentNumerator : segmentNumerator;
	unsigned int segmentNumeratorNearZero = (f2 < kFloat_0_00001_QuadTree);
	if (segmentNumeratorNearZero) {
		t = kFloat_0_805DF744;
	} else {
		f0 = segmentNumerator;
		t = f0 / f31;
	}
	// Reconstruct the two closest world positions; publish parameters and Euclidean distance.
	__ml__FfRC7WVector(scaledSegmentDirection, segmentDirection, t);
	__pl__FRC7WVectorRC7WVector(segmentClosestPoint, segmentOrigin, scaledSegmentDirection);
	__ml__FfRC7WVector(scaledRayDirection, rayDirection, s);
	__pl__FRC7WVectorRC7WVector(rayClosestPoint, rayOrigin, scaledRayDirection);
	__mi__FRC7WVectorRC7WVector(closestDifference, rayClosestPoint, segmentClosestPoint);
	f1 = Magnitude__7WVectorCFv(closestDifference);
	*(float*)out = s;
	*((float*)out + 1) = t;
	*((float*)out + 2) = f1;
}

// Sweep candidate objects and ground triangles for the nearest obstruction fraction.
extern "C" float CQuadTree_FindObstructionSweepFraction(void* self, float* box, void* direction) { float fraction;
	float vertices[3][3];
	short indices[100];
	float inverse[3], transform[3];
	float upper[3], lower[3];
	float hit[4];
	float extent[3];
	float plane[4];
	float absdir[3];
	float scaledAxis[3];
	float axis[3];
	float minTmp[3], maxTmp[3], lowerTmp[3], upperTmp[3];
	struct Words8A54 { int x; int y; int z; };
	Words8A54 start;
	float point[3], movement[3], end[3];
	float best;
	int result;

	char* s = (char*)self;
	Vec3_Zero(transform);
	Vec3_Zero(inverse);
	result = -1;
	best = kFloat_1_QuadTree;
	float* axisPtr=axis;
	axisPtr[0] = kFloat_1_QuadTree;
	axisPtr[1] = kFloat_1_QuadTree;
	axisPtr[2] = kFloat_1_QuadTree;
	float radius = ((CGolfBall*)GolfBall__Fv())->m_radius;
	float dz = *(float*)((char*)direction + 8);
	dz = dz > kFloat_0_805DF744 ? dz : -dz;
	float dy = *(float*)((char*)direction + 4);
	dy = dy > kFloat_0_805DF744 ? dy : -dy;
	float dx = *(float*)direction;
	dx = dx > kFloat_0_805DF744 ? dx : -dx;
	absdir[0]=dx; absdir[1]=dy; absdir[2]=dz;
	__ml__FfRC7WVector(scaledAxis, axisPtr, radius);
	__pl__FRC7WVectorRC7WVector(extent, absdir, scaledAxis);
	__mi__FRC7WVectorRC7WVector(minTmp, box, extent);
	__as__7WVectorFRC7WVector(transform, minTmp);
	__pl__FRC7WVectorRC7WVector(maxTmp, box, extent);
	__as__7WVectorFRC7WVector(inverse, maxTmp);
	__ami__7WVectorFRC7WVector(transform, s + 0x206c);
	__ami__7WVectorFRC7WVector(inverse, s + 0x206c);
	int value; int limit;
	(value = (int)(transform[0]/kFloat_20_QuadTree), limit = AsCQuadTree(s)->gridColumns-1,
	value = value<0 ? 0 : value>limit ? limit : value,
	AsCQuadTree(s)->minCellX=value,
	value=(int)(inverse[0]/kFloat_20_QuadTree), limit=AsCQuadTree(s)->gridColumns-1,
	value = value<0 ? 0 : value>limit ? limit : value,
	AsCQuadTree(s)->maxCellX=value,
	value=(int)(transform[2]/kFloat_20_QuadTree), limit=AsCQuadTree(s)->gridRows-1,
	value = value<0 ? 0 : value>limit ? limit : value,
	AsCQuadTree(s)->minCellZ=value,
	value=(int)(inverse[2]/kFloat_20_QuadTree), limit=AsCQuadTree(s)->gridRows-1,
	value = value<0 ? 0 : value>limit ? limit : value,
	AsCQuadTree(s)->maxCellZ=value);
	Vec3_Zero(lower);
	Vec3_Zero(upper);
	// Visit grid object IDs before dynamic objects. Preserve the legacy n==unique skip;
	// with an initially empty list this loop does not discover a new unique grid object.
	int unique=0;
	int id;
	for (int iz=AsCQuadTree(s)->minCellZ; iz<=AsCQuadTree(s)->maxCellZ; ++iz) for (int ix=AsCQuadTree(s)->minCellX; ix<=AsCQuadTree(s)->maxCellX; ++ix) {
		char* cell=(*(char**)&AsCQuadTree(s)->cells)+(ix+iz*AsCQuadTree(s)->gridColumns)*16;
		for(int j=0;j<*(unsigned char*)(cell+0xA);++j) {
			id=((short*)*(char**)&((GridCellView*)cell)->objectIds)[j];
			int n;
			for(n=0;n<unique;++n) if(indices[n]==id) break;
			if(n==unique) continue;
			char* room=*(char**)&AsCourseData801C(*(char**)&AsCQuadTree(s)->courseData)->surfaces+id* kSurfaceStride;
			__mi__FRC7WVectorRC7WVector(lowerTmp,room+0x64,extent);
			__as__7WVectorFRC7WVector(lower,lowerTmp);
			__pl__FRC7WVectorRC7WVector(upperTmp,room+0x70,extent);
			__as__7WVectorFRC7WVector(upper,upperTmp);
			int inside=0;
			if(lower[0]<=box[0] && box[0]<=upper[0] && lower[1]<=box[1] && box[1]<=upper[1] && lower[2]<=box[2] && box[2]<=upper[2]) inside=1;
			if(inside) CQuadTree_IntersectBallSweepWithObjectTriangles(self,direction,id,hit,&best,&result);
			indices[unique++]=(short)id;
		}
	}
	for(unsigned int i=0;i<AsCQuadTree(s)->objectCount;++i) {
		char* ob=((char**)(*(char**)&AsCQuadTree(s)->objects))[i];
		switch(*(unsigned char*)ob) {
		case 1: case 3: {
			int id=*(short*)(ob+8); CQuadTree_MakeCollisionData(self,id); // original r4 = lha 0x8(r3) at 0x801D8E68 (the _p226 copy took the previous loop's id)
			CQuadTree_IntersectBallSweepWithObjectTriangles(self,direction,id,hit,&best,&result);
			break;
		}
		}
	}
	Copy3f((int*)&start, box);
	CQuadTree_GetMeshListAlongSegment(self,&start,direction);
	float zero=kFloat_0_805DF744;

	char* cursor=s;
	for(id=0;id<AsCQuadTree(s)->candidateTriangleCount;cursor+=4,++id) {
		int offset=AsCQuadTree(cursor)->candidateTriangleIds[0]* kTriangleStride;
		float* vertex=vertices[0];
		for(int j=0,vo=0;j<3;++j,vertex+=3,vo+=12)
			__as__7WVectorFRC7WVector(vertex,*(char**)&((CourseSurface*)*(char**)&AsCQuadTree(s)->groundSurface)->triangles+(offset+vo));
		Plane_Copy(plane,*(char**)&((CourseSurface*)*(char**)&AsCQuadTree(s)->groundSurface)->triangles+offset+0x24);
		if(*(unsigned char*)(*(char**)&((CourseSurface*)*(char**)&AsCQuadTree(s)->groundSurface)->triangles+offset+0x34) && Plane_EvaluatePointReversed(GolfBall__Fv(),plane)<zero) {
			Vec3_NegateInPlace(plane);
			plane[3]*=(kFloat_neg1_QuadTree);
		}
		if(__ml__FRC7WVectorRC7WVector(direction,plane)>zero) fraction=kFloat_1_QuadTree;
		else {
			fraction=Plane_EvaluatePointReversed(box,plane);
			if(fraction<zero) fraction=zero;
			else {
				__pl__FRC7WVectorRC7WVector(end,box,direction);
				float d=Plane_EvaluatePointReversed(end,plane);
				if(fraction*d<=zero) {
					float ad=d>zero?d:-d;
					float af=fraction>zero?fraction:-fraction;
					fraction=(fraction>zero?fraction:-fraction)/(af+ad);
				} else fraction=kFloat_1_QuadTree;
			}
		}
		if(fraction<best) {
			__ml__FfRC7WVector(movement,direction,fraction);
			__pl__FRC7WVectorRC7WVector(point,box,movement);
			if(Triangle_ContainsPointXZ(point,vertices[0],vertices[1],vertices[2])) best=fraction;
		}
	}
	return best;
}

// Count collision-event records with a nonnegative object index.
extern "C" int CQuadTree_GetObjColNum(void* self) {
    void* end;
    void* records;
	unsigned int result; records = (void*)AsCQuadTree(self)->collisionEvents; end = (char*)records + (*(int*)&AsCQuadTree(self)->collisionEventCount) * 0x24;

	 result = 0;
	while (records != end) {
		if (*(short*)((char*)records + 2) >= 0)
			++result;
		records = (char*)records + sizeof(fn_801D6360_item);
	}
	return result;
}

// Inclusive X/Y/Z bounds in the original test order; an unordered comparison is outside.
static inline int WaterVolumeContains(const char* table, int offset, const float* point, float x, float y) {
    int inside = 0;
    const GroundRegion* region = (const GroundRegion*)(table + offset);
    if (((const GroundRegion*)(table + offset))->boundsMin.x <= x)
        if (x <= region->boundsMax.x)
            if (region->boundsMin.y <= y)
                if (y <= region->boundsMax.y)
                    if (region->boundsMin.z <= point[2])
                        if (point[2] <= region->boundsMax.z) inside = 1;
    return inside;
}

// Test raised-region volumes at their midpoint height and compare against the highest ground.
// arg is IN/OUT: each region replaces its Y with that region midpoint, including on
// a miss. Return 1 when a containing region top is above the sampled ground, else 0.
// Required borrowed tree and writable world-space Vec3 point. Query refreshes
// candidate IDs ONCE, before the region loop; regions are 0x1C-byte GroundRegion
// records, NOT triangle records. Each trial writes Y=(minY+maxY)*0.5 in that
// operation order, even when containment fails. Empty regions leave Y unchanged;
// miss leaves the LAST midpoint; hit leaves its successful midpoint, not ground Y.
// Bounds are inclusive; top>highestGround is strict. Highest-ground scratch is
// four floats for the callee's vector-return ABI. No ownership is transferred.
extern "C" int QuadTree_IsWaterAtPoint(void* self, void* point) {
    char* regionBytes;
    float midpointWeight;
    int regionOffset;
    unsigned int regionIndex;
    float pointX;
    char* table;
    unsigned int containsPoint;
    float midpointY;
    float heightSum;
    float highestGroundY;
	CQuadTree_GetMeshList(self, (float*)point);
	float groundSample[4];
	midpointWeight = kFloat_0_5_QuadTree;
	regionIndex = 0;
	regionOffset = 0;
	while (regionIndex < AsCQuadTree(self)->groundRegionCount) { pointX = *(float*)point;
		table = (*(char**)&AsCQuadTree(self)->groundRegions);
		  regionBytes = table + regionOffset;

		heightSum = ((GroundRegion*)regionBytes)->boundsMin.y;
		midpointY = ((GroundRegion*)regionBytes)->boundsMax.y;
		 heightSum = heightSum + midpointY;
		__typeof__(midpointWeight * heightSum) roundedMidpoint = ((((midpointWeight * heightSum)))); midpointY = roundedMidpoint;
		((Vec3f*)point)->y = midpointY;

		containsPoint = WaterVolumeContains(table, regionOffset, (const float*)point, pointX, midpointY);
		if (containsPoint) {
			CQuadTree_FindHighestGroundPointInCandidates(groundSample, self, point, 1, 0, 1);
			highestGroundY = groundSample[1];

			if ((((GroundRegion*)regionBytes)->boundsMax.y) > highestGroundY) {
				return 1;
			}
		}
		regionOffset += sizeof(GroundRegion);
		regionIndex++;
	}
	return 0;
}

// Return unit brightness; callers multiply scene RGB by this factor (CBall refresh and player puppet lighting).
extern "C" float CQuadTree_DefaultLightingFactor() {
	return kFloat_1_QuadTree;
}

// March up to fifteen normalized ray steps, refining the first ground crossing against candidate triangles.
// Both inputs are IN/OUT: normalize/scale ray to one march step and advance point
// for each missed step. Return a projected ground point after refinement or exhaustion.
extern "C" Vec3f CQuadTree_GetRayIntersection(void* self, void* point, void* ray) {
	int step;
	(*(WVector*)ray).Normalize();
	*(WVector*)ray *= kQuadTreeGroundRayLength;
	float one, closest;
	step = 1;
	closest = kFloat_1_QuadTree;
	one = closest;
	do {
		if (GGP_BV(self, Add564(point, ray), 1, 0, 0).y > Add564(point, ray).y) {
			((void (*)(void*, Vec3f, void*))CQuadTree_GetMeshListAlongSegment)(self, *(Vec3f*)point, ray);
			if (AsCQuadTree(self)->candidateTriangleCount > 0) {
				char* p = (char*)self;
				for (int i = 0; i < AsCQuadTree(self)->candidateTriangleCount; ++i) {
					float d = Geometry_GetRayTriangleHitFraction(self, point, ray, *(char**)(*(char**)&AsCQuadTree(self)->groundSurface + COURSE_OFFSET(CourseSurface, triangles)) + AsCQuadTree(p)->candidateTriangleIds[0] * kTriangleStride);
					closest = (d > closest) ? closest : d;
					p = 4 + p;
				}
			}
			if (closest < one) break;
		}
		*(WVector*)point += *(WVector*)ray;
	} while (step++ < kRayMarchSteps);
	return GGP_BV(self, AddR564(*(Vec3f*)point, Scale564(ray, closest)), 0, 0, 0);
}

#pragma global_optimizer off
// Compute a ray/triangle hit fraction with the original independent barycentric tests; return one on rejection.
// unused is an ignored receiver ABI word. rayOrigin/rayStep are required
// borrowed float3s; triangle begins with three world-space Vec3f vertices.
// rayStep is a displacement, not necessarily normalized: returned fraction
// scales that displacement. No input/output storage is modified or retained.
// determinant epsilon is 0.00001; near-parallel/degenerate or either independent
// barycentric component outside [0,1] returns 1. There is deliberately NO
// U+V<=1 or nonnegative-fraction test. Accepted result is min(fraction,1).
// Do not substitute a conventional ray/triangle algorithm: negative fractions
// and the independent U/V tests are existing behavior used by ground marching.
// Named float3 result arrays preserve WVector's hidden-return/copy sequence;
// both scaled-origin-cross calls must remain, as must float precision/order.
extern "C" float Geometry_GetRayTriangleHitFraction(void* unused, void* rayOrigin, void* rayStep, void* triangle) {
	float edgeToVertex1[3], edgeToVertex2[3], directionCrossEdge2[3], originFromVertex0[3], originCrossEdge1[3], edge1Result[3], edge2Result[3], directionCrossResult[3], originOffsetResult[3], scaledDirectionCross[3], originCrossResult[3], scaledOriginCrossForV[3], scaledOriginCrossForFraction[3];
	__mi__FRC7WVectorRC7WVector(edge1Result, (char*)&((CourseTriangle*)triangle)->vertices[1], triangle);
	__as__7WVectorFRC7WVector(edgeToVertex1, edge1Result);
	__mi__FRC7WVectorRC7WVector(edge2Result, (char*)&((CourseTriangle*)triangle)->vertices[2], triangle);
	__as__7WVectorFRC7WVector(edgeToVertex2, edge2Result);
	Vector_CrossProduct(directionCrossResult, rayStep, edgeToVertex2);
	__as__7WVectorFRC7WVector(directionCrossEdge2, directionCrossResult);
	float determinant = __ml__FRC7WVectorRC7WVector(directionCrossEdge2, edgeToVertex1);
	float absoluteDeterminant = (*(int*)&determinant & kFloatSignMask) ? -determinant : determinant;
	bool parallelOrDegenerate = absoluteDeterminant < kFloat_0_00001_QuadTree;
	if (parallelOrDegenerate) return kFloat_1_QuadTree;
	float inverseDeterminant = kFloat_1_QuadTree / determinant;
	__mi__FRC7WVectorRC7WVector(originOffsetResult, rayOrigin, triangle);
	__as__7WVectorFRC7WVector(originFromVertex0, originOffsetResult);
	__ml__FfRC7WVector(scaledDirectionCross, directionCrossEdge2, inverseDeterminant);
	float barycentricU = __ml__FRC7WVectorRC7WVector(scaledDirectionCross, originFromVertex0);
	if (barycentricU < kFloat_0_805DF744 || barycentricU > kFloat_1_QuadTree) return kFloat_1_QuadTree;
	Vector_CrossProduct(originCrossResult, originFromVertex0, edgeToVertex1);
	__as__7WVectorFRC7WVector(originCrossEdge1, originCrossResult);
	__ml__FfRC7WVector(scaledOriginCrossForV, originCrossEdge1, inverseDeterminant);
	float barycentricV = __ml__FRC7WVectorRC7WVector(scaledOriginCrossForV, rayStep);
	if (barycentricV < kFloat_0_805DF744 || barycentricV > kFloat_1_QuadTree) return kFloat_1_QuadTree;
	__ml__FfRC7WVector(scaledOriginCrossForFraction, originCrossEdge1, inverseDeterminant);
	float hitFraction = __ml__FRC7WVectorRC7WVector(scaledOriginCrossForFraction, edgeToVertex2);
	float noHitFraction = kFloat_1_QuadTree;
	return fn_801D9424_min(hitFraction, noHitFraction);
}
#pragma global_optimizer reset

// Add collision events spaced more than three history frames apart to the raised-region bounce count.
extern "C" int CQuadTree_GetBounceCount(void* self) {
    int result;
    void* end;
    void* records;
    int next;
	int value; result = AsCQuadTree(self)->regionBounceCount; value = -1; records = (void*)AsCQuadTree(self)->collisionEvents;

	end = (char*)records + (*(int*)&AsCQuadTree(self)->collisionEventCount) * 0x24;

	while (records != end) {
		next = *(int*)((char*)records + 8);
		if (next - value > 3) {
			value = next;
			result += 1;
		}
		records = (char*)records + sizeof(fn_801D6360_item);
	}
	return result;
}

// Resolve ground clearance with vertical sweeps, raised-region clamping and the configured minimum height.
// point is IN/OUT during raised-region/sweep adjustment. heightClearance is a world-space offset,
// added above the surface. Return at least minimumGroundHeight + heightClearance; this final clamp
// is returned without necessarily writing that clamped value back to point->y.
extern "C" float CQuadTree_ResolveGroundHeight(void* self, Vec3f* point, float heightClearance) {
	for (unsigned char i = 0; i < AsCQuadTree(self)->groundRegionCount; ++i) {
		Box801D9644 r = *(Box801D9644*)&((Rec801D9644*)(*(void**)&AsCQuadTree(self)->groundRegions))[i];
		if (point->x > r.v[0] && point->x < r.v[3] && point->z > r.v[2] && point->z < r.v[5] && point->y < r.v[4]) {
			point->y = r.v[4];
			break;
		}
	}
	if (*(unsigned char*)&AsCQuadTree(self)->groundCollisionSkipped) {
		void* g = GolfBall__Fv();
		if (((CGolfBall*)g)->m_preDataCount > 0) {
			Vec3f p = *point;
			return heightClearance + CQuadTree_GetLowestGround(self, &p);
		}
	}
	float t[2];
	float a[3];
	a[0] = kFloat_0_805DF744; a[1] = kFloat_100_0_CourseEffects; a[2] = kFloat_0_805DF744;
	t[0] = CQuadTree_SweepBallAgainstTriangles(self, point, a, 0, 1);
	float v[3];
	float m[3];
	float b[3];
	b[0] = kFloat_0_805DF744; b[1] = kFloat_neg100_QuadTree; b[2] = kFloat_0_805DF744;
	m[0] = kFloat_0_805DF744; m[1] = kFloat_100_0_CourseEffects; m[2] = kFloat_0_805DF744;
	__pl__FRC7WVectorRC7WVector(v, point, m);
	t[1] = CQuadTree_SweepBallAgainstTriangles(self, v, b, 0, 1);
	int idx = !(t[0] < kFloat_1_QuadTree - t[1]);
	if (t[idx] < kFloat_1_QuadTree) {
		if (idx == 0) {
			float c[3];
			c[0] = kFloat_0_805DF744; c[1] = kFloat_neg100_QuadTree; c[2] = kFloat_0_805DF744;
			t[1] = CQuadTree_SweepBallAgainstTriangles(self, point, c, 0, 1);
			if (t[1] < kFloat_1_QuadTree) { float y = FP_FNMSUBS(kFloat_100_0_CourseEffects, t[1], point->y); point->y = y + heightClearance; }
		} else {
			{ float y = FP_FMADDS_ADD_FIRST(point->y, kFloat_100_0_CourseEffects, (kFloat_1_QuadTree - t[1])); point->y = y + heightClearance; }
		}
	} else {
		float c[3];
		c[0] = kFloat_0_805DF744; c[1] = kFloat_neg100_QuadTree; c[2] = kFloat_0_805DF744;
		t[1] = CQuadTree_SweepBallAgainstTriangles(self, point, c, 0, 1);
		if (t[1] < kFloat_1_QuadTree) { float y = FP_FNMSUBS(kFloat_100_0_CourseEffects, t[1], point->y); point->y = y + heightClearance; }
	}
	float floor = AsCQuadTree(self)->minimumGroundHeight + heightClearance;
	float y = point->y;
	return y < floor ? floor : y;
}

// Store the per-quadtree gate used by distance-based object sound attenuation.
extern "C" void CQuadTree_SetObjectSoundEnabled(void* self, int value) {
	AsCQuadTree(self)->objectSoundEnabled = value;
}

// Parse an animation sound event, restart its per-object alias and apply distance attenuation.
extern "C" void CQuadTree_PlayObjectSoundFromParsedToken(void* self, void* cursor, int index) {
	char filename[0x88];
	char path[16];

	int count;
	sprintf(path, courseObjectSoundStrings.soundAliasFormat);
	*(void**)cursor = (char*)String_ParseQuotedToken(*(void**)cursor, filename);
	count = 0;
	sscanf(filename, courseObjectSoundStrings.stringIntegerFormat, (char*)((*(void***)&AsCQuadTree(self)->objects))[index] + 0x10, &count);
	if (SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, path)) Sound_StopByAlias(g_soundManager, path);
	float amount = kFloat_0_805DF744;
	if (AsCQuadTree(self)->objectSoundEnabled) {
		char* item = (char*)((*(void***)&AsCQuadTree(self)->objects))[index];
		WVector tmp = *(WVector*)((char*)AsCourseData801C((*(void**)&AsCQuadTree(self)->courseData))->surfaces + *(short*)(item + 8) * kSurfaceStride + 0x38) - *(WVector*)((char*)s_cameraTransform + COURSE_OFFSET(CameraTransformView, position));
		void** arr = (*(void***)&AsCQuadTree(self)->objects);
		float len = WVectorLen(tmp);
		amount = kFloat_1_QuadTree - len / (kFloat_3_2_QuadTree * *(float*)((char*)arr[index] + 0x30));
		if (kFloat_0_805DF744 < amount) {
		} else
			amount = kFloat_0_805DF744;
	}
	Sound_PlayByName(g_soundManager, (char*)((*(void***)&AsCQuadTree(self)->objects))[index] + 0x10, 0, count != 0, path, 0, kFloat_0_5_QuadTree, kFloat_200_QuadTree, amount);
	SoundManager_SetNamedVolumeScale(g_soundManager, path, amount);
}

// Forward an animation attachment event to the associated surface model's bone-reference parser.
extern "C" void CQuadTree_ParseBoneReferences(void* self, void* arg1, int index, void* arg3, void* arg4, void* arg5) {
	void* array = AsCQuadTree(self)->objects;
	void* item = *(void**)((char*)array + index * 4);
	void* manager = AsCourseData801C((*(void**)&AsCQuadTree(self)->courseData))->surfaces;
	short id = *(short*)((char*)item + 8);
	void* record = (char*)manager + id * kSurfaceStride;
	void* value = *(void**)&((CourseSurface*)record)->puppet;
	FxAttachment_ParseBoneReferences(self, arg1, index, value, 0, 0, 0);
}

// Recursively clip a polygon against six patch planes and append the surviving vertices to bounded scratch storage.
// Capacity invariant for editing this routine and BuildProjectedPatchMesh:
// - Initial input is a convex ring with count >= 3 and storage for count+1 Vec3s.
//   Recursion can produce degenerate rings; the leaf discards counts <= 2.
// - One plane adds at most one vertex. The mesh builder starts with triangles:
//   after six planes count <= 3+6 = 9, so a closed ring needs at most 10 slots.
// - The local status/distance arrays and recursive output each have 256 slots.
//   The >254 guard alone is NOT sufficient for arbitrary nonconvex input.
// - On leaf entry 0 <= vertexScratchCount <= vertexScratchCapacity must hold;
//   the leaf appends min(count, capacity-countSoFar) vertices, possibly truncating.
// points is writable and needs an extra Vec3 slot: points[count] is overwritten with
// points[0]. planeIndex starts at zero; >254 input vertices are rejected. Survivors
// append to self vertex scratch; counts may be truncated when capacity is exhausted.
extern "C" void CQuadTree_ClipPolygonToSixPlanes(void* self, void* points, int count, int planeIndex) { char status[256]; float diffs[256]; int outCount = 0; int outOffset = 0;
	if (count > kMaxClipInputVertices) return;
	if (planeIndex == kPatchPlaneCount) {
		if (count <= 2) return;
		int baseIndex = AsCQuadTree(self)->vertexScratchCount;
		int capacity = AsCQuadTree(self)->vertexScratchCapacity;
		if (baseIndex + count >= capacity) count = capacity - baseIndex;
		char* p = (char*)points;
		char* src = (char*)AsCQuadTree(self)->vertexScratch + baseIndex * 0xc;
		for (int i = 0; count > i; ) {
			__as__7WVectorFRC7WVector(src, p);
			i++;
			p += 0xc;
			src += 0xc;
		}
		AsCQuadTree(self)->vertexScratchCount += count;
		return;
	}
	// Classify signed distance n dot p - plane[3]: 0 kept, 1 discarded, 2 within tolerance.
	float outsideTolerance = kFloat_neg0_03125_QuadTree; float insideTolerance = kFloat_0_03125_QuadTree;
	char* record = (char*)self + (planeIndex * 16) + 0x20e8;

	char* pPoints = (char*)points; float* diffPtr = diffs;
	 char* statusPtr = status;
	int i = 0;

	while (i < count) {
		float d = Vec3_DotProduct(pPoints, record);
		float f0 = *(float*)(record + 0xc);
		f0 = d - f0;
		if (f0 > insideTolerance) statusPtr[0] = outOffset;
		else if (f0 < outsideTolerance) statusPtr[0] = 1;
		else statusPtr[0] = 2;
		diffPtr[0] = f0;
		++statusPtr;
		i += 1;
		diffPtr++;
		pPoints += 0xc;
	}
	// Close the input ring using the required extra vertex slot; interpolate every crossing edge.
	status[count] = status[0];
	diffs[count] = diffs[0];
	pPoints = (char*)points;
	__as__7WVectorFRC7WVector(pPoints + count * 0xc, points);

	pPoints = (char*)points;
	statusPtr = status;
	char outBuf[0xC00]; diffPtr = diffs;
	char* outPtr = outBuf;
	int j = 0;
	while (j < count) {
		unsigned char st = (unsigned char)statusPtr[0];
		if (0 == st || st == 2) {
			__as__7WVectorFRC7WVector((((outBuf + outOffset))), pPoints);
			outCount++;
			outOffset += 0xc;
		}
		unsigned char st2 = (unsigned char)statusPtr[0];
		if (2 != st2) {
			unsigned char next = (unsigned char)statusPtr[1];
			if (2 != next) { if (st2 != next) { float t = diffPtr[0] / (diffPtr[0] - diffPtr[1]); float vec2[3];
				float vec[3];
				float vec4[3];

				__mi__FRC7WVectorRC7WVector(vec, pPoints + 0xc, pPoints);
				__as__7WVectorFRC7WVector(vec2, vec); float vec3[3]; __ml__FRC7WVectorf(vec3, vec2, t);

				 __pl__FRC7WVectorRC7WVector(vec4, pPoints, vec3);

				__as__7WVectorFRC7WVector(((outOffset + outBuf)), vec4); outCount = (((1 + outCount)));
				outOffset += 0xc;
			} }
		}
		++statusPtr;
		diffPtr += 1;
		j += 1;
		pPoints += 0xc;
	}
				CQuadTree_ClipPolygonToSixPlanes(self, outBuf, outCount, (1 + planeIndex));
}

// Search the 32-entry patch ring for matching corner and projection-direction vectors.
// Starts at the next write slot and excludes the immediately preceding slot; returns borrowed storage.
extern "C" void* CQuadTree_FindProjectedPatchCacheEntry(void* unused, float* corners, float* projectionDirection) {
	unsigned char* record;
	int end;
	int index = s_quadTreePatchCacheWriteIndex;
	end = index - 1;
	if (end < 0)
		end = 31;
	while (index != end) {
		record = (unsigned char*)g_quadTreeTriangleQueryCache + index * 0xF48;
		if (record[0] && Vec3_NearlyEqualWithTolerance(record + 4, corners, kFloat_0_15_QuadTreeGeometry) &&
		    Vec3_NearlyEqualWithTolerance(record + 0x10, corners + 3, kFloat_0_15_QuadTreeGeometry) &&
		    Vec3_NearlyEqualWithTolerance(record + 0x1C, corners + 6, kFloat_0_15_QuadTreeGeometry) &&
		    Vec3_NearlyEqualWithTolerance(record + 0x28, corners + 9, kFloat_0_15_QuadTreeGeometry) &&
		    Vec3_NearlyEqualWithTolerance(record + 0x34, projectionDirection, kFloat_0_15_QuadTreeGeometry))
			return (void*)record;
		if (++index == 32)
			index = 0;
	}
	return 0;
}

// Clip ground/object triangles to the patch volume, triangulate surviving polygons and cache the mesh.
// Capacity invariant: each convex clipped triangle yields <=9 vertices and <=21
// fan indices. Before accepting a fan, remaining index storage must cover the
// ENTIRE fan: the loop's test uses its starting count, not a running count.
// Total results must fit both caller storage and the fixed cache (256 vertices,
// 384 indices); a cache hit requires caller capacities >= the cached counts.
// Changes to plane count must update the 3+planes and 3*(vertices-2) bounds above.
// corners holds four world-space Vec3 values in perimeter order; projectionDirection
// is borrowed. vertices/indices and both count outputs are required caller storage;
// indices are 16-bit. Capacities count ELEMENTS, not bytes. Callers must also fit the
// fixed 256-vertex/384-index cache. A cache hit copies stored counts without checking
// capacities; the cache key ignores includeObjects. Each triangle fan tests its starting
// index count rather than each appended triangle; callers must provide sufficient storage.
extern "C" void CQuadTree_BuildProjectedPatchMesh(void* self, void* corners, void* projectionDirection, void* vertices, int* vertexCount, int vertexCapacity, void* indices, int* indexCount, int indexCapacity, unsigned char includeObjects) {
	int out;
	struct W { unsigned int x; unsigned int y; unsigned int z; };
	struct B { W lo; W hi; };
	float tri[4][3]; float objectTri[4][3];
	struct { Vec3f lo; Vec3f hi; } bounds;
	B boundsCopy, objectBounds;
	Vec3f center; Vec3f projected; Vec3f neg; Vec3f axisA; Vec3f axisB; Vec3f negative;
	Vec3f view; Vec3f centerCopy; Vec3f edgeA; Vec3f cross; Vec3f edgeB; Vec3f edgeC;
	float planeA[4]; float planeAN[4]; float planeB[4]; float planeBN[4]; float planeC[4]; float planeCN[4];
	Vec3f marginLo; Vec3f marginHi; Vec3f normalCopy; Vec3f objectNormalCopy; Vec3f initialHi; Vec3f initialLo;
	float radius; float halfA; float halfB; float dotA; float dotB; float dotC;
	float* box=(float*)corners; float* last=(float*)projectionDirection;
	char* s=(char*)self;
	Vec3_Zero(&bounds.lo); Vec3_Zero(&bounds.hi);
	out=0;
	char* cached=(char*)CQuadTree_FindProjectedPatchCacheEntry(self,box,last);
	if(cached) {
		memcpy(vertices,cached+0x44,*(int*)(cached+0x40)*12);
		memcpy(indices,cached+0xC48,*(int*)(cached+0xC44)*2);
		*vertexCount=*(int*)(cached+0x40); *indexCount=*(int*)(cached+0xC44); return;
	}
	// Cache missed: average the four corners and project the center onto ground.
	Vec3_Zero(&center);
	float* point=box;
	for(int i=0;i<4;i++,point+=3) __apl__7WVectorFRC7WVector(&center,point);
	__amu__7WVectorFf(&center,kFloat_0_25_QuadTree);
	*(W*)&centerCopy=*(W*)&center;
	GetGroundPoint__9CQuadTreeF7WVectorbPib(&view,self,(void**)&centerCopy,0,0,0);
	__as__7WVectorFRC7WVector(&projected,&view);
	float nx=-last[0],ny=-last[1],nz=-last[2];
	neg.x=nx; neg.y=ny; neg.z=nz;
	Normalize__7WVectorFv(&neg);
	__mi__FRC7WVectorRC7WVector(&edgeA,box,((3+box)));
	__as__7WVectorFRC7WVector(&axisA,&edgeA); Normalize__7WVectorFv(&axisA);
	Vector_CrossProduct(&cross,&neg,&axisA);
	__as__7WVectorFRC7WVector(&axisB,&cross); Normalize__7WVectorFv(&axisB);
	__mi__FRC7WVectorRC7WVector(&edgeB,((box+3)),box);
	halfA=kFloat_0_5_QuadTree*Magnitude__7WVectorCFv(&edgeB);
	__mi__FRC7WVectorRC7WVector(&edgeC,box+6,(3+box));
	halfB=kFloat_0_5_QuadTree*Magnitude__7WVectorCFv(&edgeC);
	if(halfA>halfB) radius=halfA; else radius=halfB;
	// Construct opposite plane pairs about projected center along axisA, axisB and -projection.
	// Ideal half-space: n dot p >= plane[3]; each pair bounds one local slab.
	// The clipper also retains the tolerance band down to plane[3] - 0.03125.
	dotA=Vec3_DotProduct(&axisA,&projected);
	planeA[0]=axisA.x; planeA[1]=axisA.y; planeA[2]=axisA.z; planeA[3]=dotA-halfA;
	Plane_Copy(s+0x20E8,planeA);
	negative.x=-axisA.x; negative.y=-axisA.y; negative.z=-axisA.z;
	planeAN[0]=negative.x; planeAN[1]=negative.y; planeAN[2]=negative.z; planeAN[3]=-dotA-halfA;
	Plane_Copy(s+0x20F8,planeAN);
	dotB=Vec3_DotProduct(&axisB,&projected);
	planeB[0]=axisB.x; planeB[1]=axisB.y; planeB[2]=axisB.z; planeB[3]=dotB-halfB;
	Plane_Copy(s+0x2108,planeB);
	negative.x=-axisB.x; negative.y=-axisB.y; negative.z=-axisB.z;
	planeBN[0]=negative.x; planeBN[1]=negative.y; planeBN[2]=negative.z; planeBN[3]=-dotB-halfB;
	Plane_Copy(s+0x2118,planeBN);
	dotC=Vec3_DotProduct(&neg,&projected);
	planeC[0]=neg.x; planeC[1]=neg.y; planeC[2]=neg.z; planeC[3]=dotC-radius;
	Plane_Copy(s+0x2128,planeC);
	negative.x=-neg.x; negative.y=-neg.y; negative.z=-neg.z;
	planeCN[0]=negative.x; planeCN[1]=negative.y; planeCN[2]=negative.z; planeCN[3]=-dotC-radius;
	Plane_Copy(s+0x2138,planeCN);
	// Build a world AABB, expand vertically by radius and all axes by one, then collect ground IDs.
	initialLo.x=kFloat_65535_QuadTree; initialLo.y=kFloat_65535_QuadTree; initialLo.z=kFloat_65535_QuadTree;
	__as__7WVectorFRC7WVector(&bounds.lo,&initialLo);
	initialHi.x=kFloat_neg65535_QuadTreeBounds; initialHi.y=kFloat_neg65535_QuadTreeBounds; initialHi.z=kFloat_neg65535_QuadTreeBounds;
	__as__7WVectorFRC7WVector(&bounds.hi,&initialHi);
	point=box;
	for(int i=0;4>i;point+=3,i++) {
		if(point[0]>bounds.hi.x) bounds.hi.x=point[0];
		if(point[0]<bounds.lo.x) bounds.lo.x=point[0];
		if(point[1]>bounds.hi.y) bounds.hi.y=point[1];
		if(point[1]<bounds.lo.y) bounds.lo.y=point[1];
		if(point[2]>bounds.hi.z) bounds.hi.z=point[2];
		if(point[2]<bounds.lo.z) bounds.lo.z=point[2];
	}
	bounds.lo.y = bounds.lo.y - radius; bounds.hi.y = (((bounds.hi.y + radius)));
	marginLo.x=kFloat_1_QuadTree; marginLo.y=kFloat_1_QuadTree; marginLo.z=kFloat_1_QuadTree;
	__ami__7WVectorFRC7WVector(&bounds.lo,&marginLo);
	marginHi.x=kFloat_1_QuadTree; marginHi.y=kFloat_1_QuadTree; marginHi.z=kFloat_1_QuadTree;
	__apl__7WVectorFRC7WVector(&bounds.hi,&marginHi);
	boundsCopy=*(B*)&bounds;
	CQuadTree_GetMeshListInBounds(self,(float*)&boundsCopy);
	AsCQuadTree(s)->vertexScratchCount=0; AsCQuadTree(s)->vertexScratch=vertices; dotC=kFloat_0_01_quadTree;
	AsCQuadTree(s)->vertexScratchCapacity=vertexCapacity;
	// Clip eligible ground triangles into caller vertex scratch; fan each surviving polygon into indices.
	char* idptr=s;
	for(int i=0;i<AsCQuadTree(s)->candidateTriangleCount;idptr+=4,i++) {
		unsigned int prev=AsCQuadTree(s)->vertexScratchCount;
		*(W*)&normalCopy=*(W*)&neg;
		// idptr advances four bytes per candidate; the shifted view deliberately
		// reads candidateTriangleIds[i] without changing the scratch base s.
		char* world=(char*)AsCQuadTree(s)->groundSurface;
		char* p=(char*)((CourseSurface*)world)->triangles+AsCQuadTree(idptr)->candidateTriangleIds[0]* kTriangleStride;
		if(((CourseTriangle*)p)->twoSided || !(Vec3_DotProduct(&((CourseTriangle*)p)->normal,&normalCopy)<dotC)) {
			memcpy(tri,((CourseTriangle*)p)->vertices,sizeof(((CourseTriangle*)p)->vertices)); CQuadTree_ClipPolygonToSixPlanes(self,tri,3,0);
		}
		int diff=AsCQuadTree(s)->vertexScratchCount-prev;
		short* dst=(short*)indices+out;
		int written=0;
		for(int t=0;t<diff-2;t++) {
			if((out+3)>indexCapacity) break;
			unsigned int base=(prev+written);
			dst[0]=prev; dst[1]=(1+base); dst[2]=(base+2); written = written + 1; dst += 3;
		}
			out = ((3*written) + out);
	}
	// Optionally append object triangles using the same world clip volume and fan representation.
	if(includeObjects) {
		objectBounds=*(B*)&bounds; CQuadTree_GetObjList(self,(float*)&objectBounds);
		char* objectId=s;
		dotC=kFloat_0_01_quadTree;
		for(int i=0;i<AsCQuadTree(s)->candidateObjectCount;objectId+=2,i++) { int offset=0;
			// objectId advances two bytes per signed-short candidate ID. CourseData owns
			// surfaces; each surface references its puppet and triangle storage.
			char* owner=(char*)AsCQuadTree(s)->courseData;
			char* item=(char*)((CourseData801C*)owner)->surfaces+AsCQuadTree(objectId)->candidateObjectIds[0]* kSurfaceStride;
			int count=(int)((CourseSurface*)item)->puppet->triangleCount;

			for(int j=0;count>j;offset+=kTriangleStride,j++) {
				int prev=AsCQuadTree(s)->vertexScratchCount;
				*(W*)&objectNormalCopy=*(W*)&neg;
				char* p=(char*)((CourseSurface*)item)->triangles+offset;
				if(((CourseTriangle*)p)->twoSided || !(Vec3_DotProduct(&((CourseTriangle*)p)->normal,&objectNormalCopy)<dotC)) {
					memcpy(objectTri,((CourseTriangle*)p)->vertices,sizeof(((CourseTriangle*)p)->vertices)); CQuadTree_ClipPolygonToSixPlanes(self,objectTri,3,0);
				}

				short* dst=(short*)indices+out; int diff=AsCQuadTree(s)->vertexScratchCount-prev;
				int limit=(out+3); int written=0;
				for(int t=0;t<diff-2;t++) {
					if(limit>indexCapacity) break;
					unsigned int base=prev+written;
					dst[0]=prev; dst[1]=1+base; written = written + 1; dst[2]=((base+2)); dst += 3;
				}
				out+=3*written;
			}
		}
	}
	// Publish counts and copy the finished mesh into the next fixed-size cache slot.
	struct Cache { unsigned char valid; unsigned char pad[3]; Vec3f box[4]; Vec3f last; int vc; Vec3f verts[256]; int ic; short idx[384]; };
	int vc=AsCQuadTree(s)->vertexScratchCount;
	Cache* record=&((Cache*)g_quadTreeTriangleQueryCache)[s_quadTreePatchCacheWriteIndex++];
	if(s_quadTreePatchCacheWriteIndex==kPatchCacheEntries) s_quadTreePatchCacheWriteIndex=0;
	record->valid=1;
	__as__7WVectorFRC7WVector(&record->box[0],box);
	__as__7WVectorFRC7WVector(&record->box[2],box+6);
	__as__7WVectorFRC7WVector(&record->box[1],(box+3));
	__as__7WVectorFRC7WVector(&record->box[3],9+box);
	__as__7WVectorFRC7WVector(&record->last,last);
	record->vc=vc; memcpy(record->verts,vertices,((12*vc))); record->ic=out; memcpy(record->idx,indices,(out*2));
	*vertexCount=AsCQuadTree(s)->vertexScratchCount; *indexCount=out;
}

// Build a projected patch, compute texture coordinates and height-faded alpha, then draw it.
// worldCenter +/- worldHalfAxisU +/- worldHalfAxisV form four world corners. Half-axis
// lengths must be nonzero. amount scales UV span; alpha scales opacity. material is
// borrowed; includeObjectsAbi retains the original integer/pointer-shaped call ABI.
extern "C" void CQuadTree_DrawProjectedTexturedPatch(void* self, float amount, float alpha, void* material, void* worldCenter, void* worldHalfAxisU, void* worldHalfAxisV, void* unused, void* includeObjectsAbi) {
	if (!s_projectedTexturedPatchInitialized) s_projectedTexturedPatchInitialized = 1;
	float half;
	int outCount = 0, count = 0;
	char vEC[12], vE0[12], vD4[12], vC8[12], vBC[12], vB0[12], vA4[12], v98[12];
	char v8C[12], v80[12], v74[12], v68[12], v5C[12], v50[12], v44[12], v38[12];
	char temp2[12], temp[12];
	__as__7WVectorFRC7WVector(vB0, worldHalfAxisU); Normalize__7WVectorFv(vB0);
	__as__7WVectorFRC7WVector(vA4, worldHalfAxisV); Normalize__7WVectorFv(vA4);
	Vector_CrossProduct(v98, vB0, vA4);
	__as__7WVectorFRC7WVector(vBC, v98); Normalize__7WVectorFv(vBC);
	__mi__FRC7WVectorRC7WVector(v80, worldCenter, worldHalfAxisU); __mi__FRC7WVectorRC7WVector(v8C, v80, worldHalfAxisV); __as__7WVectorFRC7WVector(vC8, v8C);
	__pl__FRC7WVectorRC7WVector(v68, worldCenter, worldHalfAxisU); __mi__FRC7WVectorRC7WVector(v74, v68, worldHalfAxisV); __as__7WVectorFRC7WVector(vD4, v74);
	__pl__FRC7WVectorRC7WVector(v50, worldCenter, worldHalfAxisU); __pl__FRC7WVectorRC7WVector(v5C, v50, worldHalfAxisV); __as__7WVectorFRC7WVector(vE0, v5C);
	__mi__FRC7WVectorRC7WVector(v38, worldCenter, worldHalfAxisU); __pl__FRC7WVectorRC7WVector(v44, v38, worldHalfAxisV); __as__7WVectorFRC7WVector(vEC, v44);
	unsigned char vertices[0x3800];
	((void (*)(void*,void*,void*,void*,int*,int,void*,int*,int,void*))CQuadTree_BuildProjectedPatchMesh)(self, vC8, vBC, s_projectedPatchPositions, &count, kPatchVertexCapacity, s_quadTreeProjectedPatchIndices, &outCount, kPatchIndexCapacity, includeObjectsAbi);
	// UVs use dot products along normalized half-axes: U=0.5+scaleU*offsetU,
	// V=0.5-scaleV*offsetV. Alpha fades linearly with absolute world-Y separation,
	// reaching zero at half the sum of the two half-axis lengths; RGB remains white.
	float lengthC = Magnitude__7WVectorCFv(worldHalfAxisU);
	float lengthD = Magnitude__7WVectorCFv(worldHalfAxisV);
	float midpoint = kFloat_0_5_QuadTree;
	half = midpoint * (lengthC + lengthD);
	float scaleC = amount / (kFloat_2_QuadTree * lengthC);
	float scaleD = amount / (kFloat_2_QuadTree * lengthD);
	float colorScale = kFloat_255_QuadTree;
	float* vertex = (float*)s_projectedPatchPositions;
	float* item = (float*)vertices;
	for (int i = 0; i < count; vertex += 3, ++i, item += 14) {
		item[10] = vertex[0]; item[11] = vertex[1]; item[12] = vertex[2];
		__mi__FRC7WVectorRC7WVector(temp2, vertex, worldCenter); float dist = Vec3_DotProduct(vB0, temp2);
		item[6] = scaleC * dist + midpoint;
		__mi__FRC7WVectorRC7WVector(temp, vertex, worldCenter); dist = Vec3_DotProduct(vA4, temp);
		float uv = midpoint - scaleD * dist;
		float delta = ((float*)worldCenter)[1] - vertex[1];
		item[7] = uv;
		float absDelta = (float)__fabs((double)delta);
		float* clip = half < absDelta ? &half : &absDelta;
		int byte = (int)(colorScale * ((alpha * (half - *clip)) / half));
		((unsigned int*)item)[4] = (byte << 24) | 0x00ffffff;
	}
	((void (*)(void*,void*,int,void*,int,void*))CQuadTree_DrawIndexedTexturedMesh)(self, vertices, count, s_quadTreeProjectedPatchIndices, outCount, material);
}

// Pack patch vertices/colors/UVs into GPU buffers, bind the material textures and draw indexed triangles.
// Input vertex stride 0x38: ARGB word at +0x10, UV at +0x18, XYZ at +0x28.
// Output stride 0x18: XYZ, RGBA bytes, UV. Indices are borrowed u16 values and count
// is a multiple of three. The supplied GPU buffers must already have enough capacity.
extern "C" void CQuadTree_DrawIndexedTexturedMesh(void* self, void* vertices, int vertexCount, void* indices, int indexCount, void* material) {
	void* vertexBuffer;
	void* indexBuffer;
	MatAA94 mat;
	Resource_GetOffsetPtr(AsCQuadTree(self)->vertexBuffer, 0, 0, &vertexBuffer, 0);
	int offset = 0;
	char* src = (char*)vertices;
	for (int i = 0; i < vertexCount; ++i) {
		*(float*)((char*)vertexBuffer + offset) = *(float*)(src + 0x28);
		*(float*)((char*)vertexBuffer + offset + 4) = *(float*)(src + 0x2C);
		*(float*)((char*)vertexBuffer + offset + 8) = *(float*)(src + 0x30);
		unsigned int c0 = *(unsigned int*)(src + 0x10);
		*((unsigned char*)vertexBuffer + offset + 0xE) = (((unsigned char*)&c0)[3]);
		unsigned int c1 = *(unsigned int*)(src + 0x10);
		*((unsigned char*)vertexBuffer + offset + 0xD) = (((unsigned char*)&c1)[2]);
		unsigned int c2 = *(unsigned int*)(src + 0x10);
		*((unsigned char*)vertexBuffer + offset + 0xC) = (((unsigned char*)&c2)[1]);
		unsigned int c3 = *(unsigned int*)(src + 0x10);
		*((unsigned char*)vertexBuffer + offset + 0xF) = (((unsigned char*)&c3)[0]);
		*(float*)((char*)vertexBuffer + offset + 0x10) = *(float*)(src + 0x18);
		*(float*)((char*)vertexBuffer + offset + 0x14) = *(float*)(src + 0x1C);
		src += kPatchSourceVertexBytes;
		offset += kPatchPackedVertexBytes;
	}
	// Vertices are packed; flush them and copy the borrowed u16 triangle indices.
	GraphicsBuffer_Flush(AsCQuadTree(self)->vertexBuffer);
	IndexBuffer_GetOffsetPtr(AsCQuadTree(self)->indexBuffer, 0, 0, &indexBuffer, 0);
	for (int i = 0; i < indexCount; ++i) ((unsigned short*)indexBuffer)[i] = ((unsigned short*)indices)[i];
	IndexBuffer_Flush(AsCQuadTree(self)->indexBuffer);
	// Draw world vertices with identity world transform and camera view/projection matrices.
	UIRender_SetDefaultState();
	mat = *(MatAA94*)kQuadTreePatchMaterial;
	Engine_SetMaterial(&mat);
	SetTransform(kTransformWorld, kIdentityMatrix4x4);
	void* state = Camera_GetRenderState(g_view);
	SetTransform(kTransformProjection, (char*)((RenderCameraTransforms*)state)->projection);
	SetTransform(kTransformView, (char*)((RenderCameraTransforms*)state)->view);
	Engine_SetFlexibleVertexFormat(kPatchVertexFormat);
	Engine_SetVertexStream(0, AsCQuadTree(self)->vertexBuffer, 0, kPatchPackedVertexBytes);
	Engine_SetIndexBuffer(AsCQuadTree(self)->indexBuffer, 0);
	ObjAA94* res = ((GameAA94*)g_view)->res;
	Engine_SetTextureStageResource(0, res->s26(material));
	res = ((GameAA94*)g_view)->res;
	Engine_SetTexturePalette(0, res->s27(material));
	Engine_SetCachedRenderState(0, 0);
	Engine_DrawIndexedPrimitive(kGpuTriangles, 0, 0, indexCount, 0, indexCount / 3);
}

// Initialize the triangle/object lists and counts of an empty 16-byte grid bucket.
extern "C" void CourseTriangleBucket_Init(fn_801DAE1C_data* self) {
	self->triangleCount = 0;
	self->triangleIdsWord = 0;
	self->objectCount = 0;
	self->objectIdsWord = 0;
	self->reserved0C = 0;
}

// Merge the surface buckets crossed by a ball sweep, then test transformed triangles and retain the closest hit.
// Uses the global ball position/radius with a world displacement. bestFraction is
// IN/OUT; return 1 only when a closer hit updates it and the required plane/index
// outputs. Candidate IDs/hitFeatures are overwritten. Bucket IDs must be sorted
// for the two-bucket merge. Single-bucket/same-octant paths require 0 <= n <= 2048:
// n > 2048 sets overflow and skips copying, but leaves count = n; the later loop
// still uses that uncapped count and stale IDs. This is not safe truncation.
// The two-bucket merge stops at 2048 entries and caps each remaining tail copy.
extern "C" int CQuadTree_IntersectBallSweepWithObjectTriangles(void* self, void* displacement, int surfaceIndex, void* outHitPlane, float* bestFraction, int* outTriangleIndex) {
    extern float kFloat_3_QuadTree, s_quadTreeSurfaceReferenceValue;
    float vertices[3][3], start[3], point[3], copied[3], plane[4];
    float transformed[3], difference[3], shifted[3], scaledNormal[3], sum[3], movement[3], closest[3], end[3];
    float p3[3], q3[3], cross3[3], b3[3], a3[3], cross23[3], d3[3], c3[3], zero3[3], zero23[3];
    float p2[3], q2[3], cross2[3], b2[3], a2[3], cross22[3], d2[3], c2[3], zero2[3], zero22[3];
    float p1[3], q1[3], cross1[3], b1[3], a1[3], cross21[3], d1[3], c1[3], zero1[3], zero21[3];
    float sign;
    struct Self { char pad[0x1C]; int ids[0x800]; int count; };
    Self* state = (Self*)self;
    int current;
    char* cursor;
    int vertexOffset;
    float* vertex;
    int offset;
    char* grid;
    int i, j, id, found, special, skip, below, above;
    char* owner = (char*)(*(void**)&AsCQuadTree(self)->courseData);
    grid = (char*)*(void**)(owner + 0xA4) + surfaceIndex * kSurfaceStride;
    found = 0;
    if (((CourseSurface*)grid)->singleBucket) {
        char* bucket = *(char**)&((CourseSurface*)grid)->buckets;
        int n = *(short*)(bucket + 8);
        state->count = n;
        if (n > kMaxGridTriangles) AsCQuadTree(self)->candidateTriangleOverflow = 1;
        else memcpy(((char*)&AsCQuadTree(self)->candidateTriangleIds), **(void***)&((CourseSurface*)grid)->buckets, n * 4);
    } else {
        float* origin = (float*)GolfBall__Fv();
        unsigned char bits = 0;
        if (origin[0] > ((CourseSurface*)grid)->partitionCenter.x) bits |= 1;
        if (origin[1] > ((CourseSurface*)grid)->partitionCenter.y) bits |= 2;
        if (origin[2] > ((CourseSurface*)grid)->partitionCenter.z) bits |= 4;
        int cell = bits;

        __pl__FRC7WVectorRC7WVector(transformed, GolfBall__Fv(), displacement);
        bits = 0;
        if (transformed[0] > ((CourseSurface*)grid)->partitionCenter.x) bits |= 1;
        if (transformed[1] > ((CourseSurface*)grid)->partitionCenter.y) bits |= 2;
        if (transformed[2] > ((CourseSurface*)grid)->partitionCenter.z) bits |= 4;
        int endCell = bits;
        if (cell == endCell) {
            char* bucket = *(char**)&((CourseSurface*)grid)->buckets + cell * 0x10;
            int n = *(short*)(bucket + 8); state->count = n;
            if (n > kMaxGridTriangles) AsCQuadTree(self)->candidateTriangleOverflow = 1;
            else memcpy(((char*)&AsCQuadTree(self)->candidateTriangleIds), *(void**)(*(char**)&((CourseSurface*)grid)->buckets + cell * 0x10), n * 4);
        } else {
            int bOff = 0;
            int endOffset, cellOffset;
            cellOffset = cell * 0x10;
            state->count = bOff;
            endOffset = endCell * 0x10;
            int a, b, aOff;
            b = 0; a = 0; aOff = 0;
            while (1) {
                char* buckets = *(char**)&((CourseSurface*)grid)->buckets;
                char* ca = (char*)(cellOffset + (int)buckets);
                if (!*(short*)(ca + 8)) break;
                char* cb = (char*)(endOffset + (int)buckets);
                if (!*(short*)(cb + 8)) break;
                int va, vb;
                vb = *(int*)((char*)*(void**)cb + bOff);
                va = *(int*)((char*)*(void**)ca + aOff);
                if (va < vb) { ++a; aOff += 4; *(int*)((char*)self + state->count * 4 + 0x1C) = va; }
                else if (vb < va) { ++b; bOff += 4; *(int*)((char*)self + state->count * 4 + 0x1C) = vb; }
                else { ++a; aOff += 4; ++b; bOff += 4; *(int*)((char*)self + state->count * 4 + 0x1C) = va; }
                int n = ++state->count;
                if (n >= kMaxGridTriangles) { AsCQuadTree(self)->candidateTriangleOverflow = 1; break; }
                buckets = *(char**)&((CourseSurface*)grid)->buckets;
                int na;
                char* tailBucket = (char*)(cellOffset + (int)buckets);
                na = *(short*)(tailBucket + 8);
                if (a == na) {
                    int nb = *(short*)((char*)((int)buckets + endOffset) + 8);
                    if (b != nb) {
                        int remaining = nb - b;
                        if (remaining > kMaxGridTriangles - n) remaining = kMaxGridTriangles - n;
                        void* destination = &state->ids[n];
                        char* source = (char*)((int)*(void**)(buckets + endOffset) + b * 4);
                        memcpy(destination, source, remaining * 4);
                        state->count += remaining;
                    }
                    break;
                } else if (b == *(short*)((char*)(endOffset + (int)buckets) + 8)) {
                    int remaining = na - a;
                    if (remaining > kMaxGridTriangles - n) remaining = kMaxGridTriangles - n;
                    void* destination = &state->ids[n];
                    char* source = (char*)((int)*(void**)tailBucket + a * 4);
                    memcpy(destination, source, remaining * 4);
                    state->count += remaining;
                    break;
                }
            }
            if (state->count == kMaxGridTriangles) AsCQuadTree(self)->candidateTriangleOverflow = 1;
        }
    }
    // Merged candidates are ready: compute the swept vertical band, then reject outlying vertices.
    float lo, hi, fraction;
    if (((float*)displacement)[1] > kFloat_0_805DF744) {
        fraction = ((CGolfBall*)GolfBall__Fv())->m_radius;
        lo = ((float*)GolfBall__Fv())[1] - fraction;
        fraction = ((CGolfBall*)GolfBall__Fv())->m_radius;
        float y = ((float*)GolfBall__Fv())[1];
        hi = ((float*)displacement)[1] + (y + fraction);
    } else {
        fraction = ((CGolfBall*)GolfBall__Fv())->m_radius;
        lo = ((float*)displacement)[1] + (((float*)GolfBall__Fv())[1] - fraction);
        fraction = ((CGolfBall*)GolfBall__Fv())->m_radius;
        float y = ((float*)GolfBall__Fv())[1];
        hi = y + fraction;
    }
    float zeroValue, negativeOne, nearDistance, epsilon;
    nearDistance = kFloat_3_QuadTree;
    epsilon = kFloat_0_00001_QuadTree;
    zeroValue = kFloat_0_805DF744;
    negativeOne = kFloat_neg1_QuadTree;
    cursor = (char*)self;
    for(i=0;i<state->count;cursor+=4,i++) {
        if (surfaceIndex == *(short*)((char*)GolfBall__Fv() + 0xB4) && i == *(int*)((char*)GolfBall__Fv() + 0xB8)) continue;
        id=*(int*)(cursor+0x1C);
        offset = id * kTriangleStride;
        char* item=*(char**)&((CourseSurface*)grid)->triangles+offset;
        if (*(unsigned int*)(*(char**)&((CourseTriangle*)item)->boneTransform+kCollisionBoneFlagsOffset) & kCollisionBoneSkipMask) continue;
        float* material = ((float* (*)(void*, void*))CourseData_GetEntryByIndex)(*(void**)&AsCQuadTree(self)->courseData, *(void**)&((CourseTriangle*)item)->materialIndex);
        if (((CourseMaterial*)material)->flags & kMaterialSpecialSurfaceMask) {
            skip = 1; special = 0;
            if ((*(unsigned int*)&((CGolfBall*)GolfBall__Fv())->m_flags & 0x10) ||
                ((*(unsigned int*)&((CGolfBall*)GolfBall__Fv())->m_flags & kSpecialFlightA) && ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySpecialFlightA] < 0)) special = 1;
            if (!special) {
                sign = *((float* (*)(void*, void*))CourseData_GetEntryByIndex)(*(void**)&AsCQuadTree(self)->courseData, *(void**)(*(char**)&((CourseSurface*)grid)->triangles+offset+0x3C)) - s_quadTreeSurfaceReferenceValue;
                float value = sign;
                if (*(unsigned int*)&sign & 0x80000000) value = -value;
                if (!(value < epsilon)) skip = 0;
            }
            if (skip) continue;
            if (*(unsigned int*)&((CGolfBall*)GolfBall__Fv())->m_flags & kSpecialFlightB) continue;
        }

        above = 0;
        vertex = vertices[0];
        below = 0;
        j = 0;
        *(unsigned char*)(*(char**)&((CourseSurface*)grid)->triangles+offset+0x40) = above;
        vertexOffset = 0;
        for (;j<3;++j,vertexOffset+=12,vertex+=3) {
            __as__7WVectorFRC7WVector(vertex, *(char**)&((CourseSurface*)grid)->triangles+(vertexOffset+offset));
            if (vertex[1] > hi) ++above;
            else if (vertex[1] < lo) ++below;
            if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySlot1] < 0 && ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySlot3] < 0) {

                __mi__FRC7WVectorRC7WVector(difference, GolfBall__Fv(), vertex);
                if (WVectorLen__FRC7WVector(difference) < nearDistance) {
                    current = ((CGolfBall*)GolfBall__Fv())->m_preDataCount;
                    ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[kHistorySlot3] = current;
                }
            }
        }
        if (below == 3 || above == 3) continue;

        Plane_Copy(plane, *(char**)&((CourseSurface*)grid)->triangles+offset+0x24);
        if (*(unsigned char*)(*(char**)&((CourseSurface*)grid)->triangles+offset+0x34) && Plane_EvaluatePointReversed(GolfBall__Fv(), plane) < zeroValue) {
            Vec3_NegateInPlace(plane);
            plane[3] *= negativeOne;
        }

        float radius = ((CGolfBall*)GolfBall__Fv())->m_radius;
        __ml__FfRC7WVector(scaledNormal, plane, radius);
        __mi__FRC7WVectorRC7WVector(shifted, GolfBall__Fv(), scaledNormal);
        __as__7WVectorFRC7WVector(start, shifted);
        if (__ml__FRC7WVectorRC7WVector(displacement, plane) > zeroValue) fraction = kFloat_1_QuadTree;
        else {
            fraction = Plane_EvaluatePointReversed(start, plane);

            __pl__FRC7WVectorRC7WVector(end, start, displacement);
            float endDistance = Plane_EvaluatePointReversed(end, plane);
            if (fraction * endDistance <= zeroValue) {
                float absEnd = endDistance > zeroValue ? endDistance : -endDistance;
                float absStart = fraction > zeroValue ? fraction : -fraction;
                fraction = (fraction > zeroValue ? fraction : -fraction) / (absStart + absEnd);
            } else fraction = kFloat_1_QuadTree;
        }

        __ml__FfRC7WVector(movement, displacement, fraction);
        __pl__FRC7WVectorRC7WVector(sum, start, movement);
        __as__7WVectorFRC7WVector(point, sum);
        if (!(fraction < *bestFraction)) continue;
        // Face contact must pass all three edge tests; otherwise sweep against the nearest triangle edge.
        bool inside;
        {
            __mi__FRC7WVectorRC7WVector(a1, point, vertices[1]);
            __mi__FRC7WVectorRC7WVector(b1, vertices[2], vertices[1]);
            Vector_CrossProduct(cross1, b1, a1);
            __as__7WVectorFRC7WVector(p1, cross1);
            __mi__FRC7WVectorRC7WVector(c1, vertices[0], vertices[1]);
            __mi__FRC7WVectorRC7WVector(d1, vertices[2], vertices[1]);
            Vector_CrossProduct(cross21, d1, c1);
            __as__7WVectorFRC7WVector(q1, cross21);
            // Preserve zero-vector initialization only on the tested edge paths.
            if (__ml__FRC7WVectorRC7WVector(p1,q1) > zeroValue ||
                (zero1[0]=zeroValue, zero1[1]=zeroValue, zero1[2]=zeroValue,
                Vec3_NearlyEqualWithTolerance(p1,zero1,kFloat_0_00001_QuadTree)) ||
                (zero21[0]=zeroValue, zero21[1]=zeroValue, zero21[2]=zeroValue,
                Vec3_NearlyEqualWithTolerance(q1,zero21,kFloat_0_00001_QuadTree)))
                inside = true;
            else inside = false;

        }
        // 0x801DAE38: rejected edges share the false assignment below.
        do {
            if (!inside) continue;
            {
                __mi__FRC7WVectorRC7WVector(a2, point, vertices[0]);
                __mi__FRC7WVectorRC7WVector(b2, vertices[2], vertices[0]);
                Vector_CrossProduct(cross2, b2, a2);
                __as__7WVectorFRC7WVector(p2, cross2);
                __mi__FRC7WVectorRC7WVector(c2, vertices[1], vertices[0]);
                __mi__FRC7WVectorRC7WVector(d2, vertices[2], vertices[0]);
                Vector_CrossProduct(cross22, d2, c2);
                __as__7WVectorFRC7WVector(q2, cross22);
                // Preserve zero-vector initialization only on the tested edge paths.
                if (__ml__FRC7WVectorRC7WVector(p2,q2) > zeroValue ||
                    (zero2[0]=zeroValue, zero2[1]=zeroValue, zero2[2]=zeroValue,
                    Vec3_NearlyEqualWithTolerance(p2,zero2,kFloat_0_00001_QuadTree)) ||
                    (zero22[0]=zeroValue, zero22[1]=zeroValue, zero22[2]=zeroValue,
                    Vec3_NearlyEqualWithTolerance(q2,zero22,kFloat_0_00001_QuadTree)))
                    inside = true;
                else inside = false;

            }
            if (!inside) continue;
            {
                __mi__FRC7WVectorRC7WVector(a3, point, vertices[0]);
                __mi__FRC7WVectorRC7WVector(b3, vertices[1], vertices[0]);
                Vector_CrossProduct(cross3, b3, a3);
                __as__7WVectorFRC7WVector(p3, cross3);
                __mi__FRC7WVectorRC7WVector(c3, vertices[2], vertices[0]);
                __mi__FRC7WVectorRC7WVector(d3, vertices[1], vertices[0]);
                Vector_CrossProduct(cross23, d3, c3);
                __as__7WVectorFRC7WVector(q3, cross23);
                // Preserve zero-vector initialization only on the tested edge paths.
                if (__ml__FRC7WVectorRC7WVector(p3,q3) > zeroValue ||
                    (zero3[0]=zeroValue, zero3[1]=zeroValue, zero3[2]=zeroValue,
                    Vec3_NearlyEqualWithTolerance(p3,zero3,kFloat_0_00001_QuadTree)) ||
                    (zero23[0]=zeroValue, zero23[1]=zeroValue, zero23[2]=zeroValue,
                    Vec3_NearlyEqualWithTolerance(q3,zero23,kFloat_0_00001_QuadTree)))
                    inside = true;
                else inside = false;

            }
            if (!inside) continue;
            inside = true;
            break;
        } while ((inside = false, false));
        if (inside) {
            *bestFraction = fraction;
            found = 1;
            Plane_Copy(outHitPlane, *(char**)&((CourseSurface*)grid)->triangles+offset+0x24);
            *outTriangleIndex = id;
            char* triangle = *(char**)&((CourseSurface*)grid)->triangles+offset;
            if (*(unsigned char*)(triangle+0x34) && Plane_EvaluatePointReversed(GolfBall__Fv(),triangle+0x24) < zeroValue) {
                Vec3_NegateInPlace(outHitPlane);
                ((float*)outHitPlane)[3] *= negativeOne;
            }
        } else {

            Triangle_ClosestPointOnEdges(closest, self, point, vertices[0], vertices[1], vertices[2]);
            __as__7WVectorFRC7WVector(copied, closest);
            float distance = CQuadTree_IntersectBallSweepWithPoint(self, copied, GolfBall__Fv(), displacement);
            if (!(distance < *bestFraction)) continue;
            *(unsigned char*)(*(char**)&((CourseSurface*)grid)->triangles+offset+0x40) = 1;
            found = 1;
            *bestFraction = distance;
            Plane_Copy(outHitPlane, *(char**)&((CourseSurface*)grid)->triangles+offset+0x24);
            *outTriangleIndex = id;
            char* triangle = *(char**)&((CourseSurface*)grid)->triangles+offset;
            if (*(unsigned char*)(triangle+0x34) && Plane_EvaluatePointReversed(GolfBall__Fv(),triangle+0x24) < zeroValue) {
                Vec3_NegateInPlace(outHitPlane);
                ((float*)outHitPlane)[3] *= negativeOne;
            }
        }
    }
    return found;
}

#undef CQuadTree_FindHighestGroundPointInCandidates
#undef Add564
#undef SET564
#undef GROUND564
#undef AddR564
#undef Scale564
#undef GetGroundPoint__9CQuadTreeF7WVectorbPib
#undef LENGTH_ERROR_6360
#undef CELL564
#undef PLAYER_POS
#undef GROUND28
#undef ROT28
#undef CENTER179
#undef CTR179
#undef STOP179
