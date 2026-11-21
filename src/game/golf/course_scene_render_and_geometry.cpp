#include <decomp/static_init.h>
// Course scene drawing, course-geometry ownership, weather and Score Card setup.
// CGolfBg_CreateCourseObjects builds beaches/rivers/lava/boundary grids/generated
// texture surfaces from CourseData; CSceneManager_RenderCourseScene submits their
// opaque, shadow, effects and transparent passes. Score Card hooks own their form.
// Shared layouts: CSceneManager, CScore, CourseData801C, CGolfDoc, CSharedDoc,
// WVector; scalar pool: u801E8048_data.cpp. Private views below name binary offsets.
// Modder lookup:
// - Models/course loading: CGolfBg_CreateCourseObjects dispatches CourseGeometryKind
//   records through CourseData/course-entry iterators. It constructs surfaces,
//   not model filenames; ResourceManager/CGolfBg's upstream load supplies the data.
//   Preserve entry-kind dispatch, pool insertion order and destructor ownership.
// - Widescreen draw context: RenderCourseScene is a CONSUMER of the supplied
//   camera and global g_view. It sets clip distances, not aspect or resolution.
//   Main aspect lives at kViewPerspectiveAspectRatio in u80065F78_data.cpp; startup display,
//   viewport/scissor and the separate loading projection are in engine_startup_and_ani_sprite.cpp.
// - Clip planes: kSceneClipNear=3, kSceneClipFar=100000 scene-distance units,
//   defined in u801E8048_data.cpp; increasing near or reducing far hides geometry.
// - Camera/object fading: kSceneProximityFadeDistanceSquared=800^2, capped radius
//   48, kSceneDistanceFadeBandSquared=25600 and record.maximumDistanceSquared
//   share squared-distance math. Increasing the band begins the far fade sooner
//   in squared distance; it is not a linear 25600-unit fade distance.
// - Texture/shadow passes: CupPreview_CaptureModel and ShadowManager_RenderShadowTextures
//   run before main drawing; their own units control texture dimensions/formats.
//   Local state/texture-stage presets are renderer protocol arrays, not image data.
// - Labels/markers: RenderCourseObjectNames projects a world anchor, centers it
//   with font width in viewport units, then draws; marker visibility uses round
//   flags. A widescreen mod must retain world->view->project order and avoid
//   applying a layout-space offset to the world coordinates.
// Safe tuning ranges are not established here. Preserve camera-pass order,
// persistent material opacity, descending transparency sort, transient-node vs
// scene-object ownership, pool cursor locks and exception cleanup. No frame-rate
// clock or controller mapping is set in this unit.
// Keep nested phase helpers inline; their caller-owned scratch preserves the original code.
#pragma inline_max_size(100000)
#pragma inline_max_total_size(100000)
enum { kScoreActorInterfaceVtableOffset = 0x3C };
// CGolfRule_EnterState (golf_rule_round_flow_and_shot_sim.cpp) defines these document-state bits.
// Keep camera sorting and marker suppression as separate policies over the same states.
enum SceneGameplayState {
    kSceneStatePlayerPreview = 0x2, kSceneStateAim = 0x8,
    kSceneStateBallFlight = 0x80, kSceneStateCameraTransition = 0x200,
    kSceneStatePrepareNextShot = 0x800, kSceneStateDelayedTransition = 0x1000,
    kSceneStateHoleResults = 0x2000, kSceneStateCameraReturn = 0x40000,
    kSceneStateFightResult = 0x80000, kSceneStateHoleOverview = 0x200000,
    kSceneStateStorySequence = 0x1000000, kSceneStateWaitPlayerAndCaddie = 0x10000000
};
enum SceneStatePolicy {
    kCameraPositionSortStates = kSceneStatePlayerPreview | kSceneStateBallFlight |
        kSceneStateCameraTransition | kSceneStateCameraReturn | kSceneStateHoleOverview | kSceneStateStorySequence,
    kAdditionalCameraPositionSortStates = kSceneStateDelayedTransition | kSceneStateFightResult,
    kMarkerHiddenStates = kSceneStateCameraTransition | kSceneStatePrepareNextShot |
        kSceneStateDelayedTransition | kSceneStateHoleResults | kSceneStateWaitPlayerAndCaddie
};
// Byte offsets into courseGlowEffectStrings (a string pool): each selects the resource name passed to a weather
// effect constructor (0x77 is an empty name).
// Offset of the course-entry vector inside the course data object.
enum { kCourseEntryVectorOffset = 0x14 };
enum WeatherResourceOffsets {
    kEmptyWeatherResource = 0x77,
    kSnowParticleResource = 0x78,
    kAmbientSnowResource = 0x86,
    kSpecialWeatherResource = 0x99
};

#include "classes/CSceneManager.h"
#include "classes/CScore.h"
#include "classes/CourseData801C.h"
// Borrow the CSceneManager layout view; no ownership is transferred.
static inline CSceneManager* AsCSceneManager(void* object) { return (CSceneManager*)object; }
// Borrow the CScore layout view; no ownership is transferred.
static inline CScore* AsCScore(void* object) { return (CScore*)object; }
#include "game/string.h"
// Compare iterator positions without changing the original template call boundary.
template <class T> inline bool same_pos(const T& a, const T& b) { return a == b; }
#include "game/wvector.h"

#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
// Borrow the CSharedDoc layout view; no ownership is transferred.
static inline CSharedDoc* AsCSharedDoc(void* object) { return (CSharedDoc*)object; }
// Borrow the CGolfDoc layout view; no ownership is transferred.
static inline CGolfDoc* AsCGolfDoc(void* object) { return (CGolfDoc*)object; }

struct CleanupClass;
struct CourseSceneConstructionView;
struct Fn801EB1FCObject;
struct Fn801EB238Item;
struct Fn801EB238Object;
struct Fn801EB248Object;
struct Fn801EB82CObject;
struct Fn801EBList;
struct Fn801EC088Object;
struct ScoreConstructorView;
struct ScoreFormOwnerView;
struct Fn801EC280Object;
struct List;
struct SList;
struct SortKey;

extern "C" {

void CSceneManager_RenderCourseScene(void* self,void* camera);
void SceneRenderList_SortByDistanceDescending(SList* self, SortKey* key);
void SceneRenderList_MergeByDistanceDescending(List* a, List* b, char* key);
void SceneRenderList_InitAngleConstant();
void SceneRenderList_SortBySignedPayload(SList* self, char* key);
void SceneRenderList_MergeBySignedPayload(List* a, List* b, char* key);
void ScenePuppetList_SortByCameraProjectionDescending(SList* self, SortKey* key);
void ScenePuppetList_MergeByCameraProjectionDescending(List* a, List* b, void* key);
void CSceneManager_ClearOwnedSceneObjects(CleanupClass* arg);
void CGolfBg_CreateCourseObjects(CourseSceneConstructionView* self);
Fn801EB1FCObject* CBeach_Ctor(Fn801EB1FCObject* self);
Fn801EB238Item* Array_GetItemByIndex(Fn801EB238Object* self, int index);
void* CourseData_GetBoundaryRecordCount(Fn801EB248Object* self);
void SceneBeachList_AppendObject(Fn801EBList* self, void** value);
void SceneRiverList_AppendObject(Fn801EBList* self, void** value);
void SceneGeneratedTextureList_AppendObject(Fn801EBList* self, void** value);
void CourseEntryIterator_Advance(Fn801EB82CObject* self);
void SceneBoundaryList_AppendObject(Fn801EBList* self, void** value);
void CourseWeather_ConfigureEffects(void* object, unsigned char mode, int enabled);
Fn801EC088Object* CScore_Create();
ScoreConstructorView* CScore_Ctor(ScoreConstructorView* self);
void CScore_OnDestroy(ScoreFormOwnerView* self);
void CScore_OnLoad();
void CScore_OnReload(Fn801EC280Object* first);

struct Link { Link* prev; Link* next; };
// Copy a world-space bounding sphere by value, preserving its center and radius.
struct Sphere {
    Vec3f v; float r;
    // Reserve caller-owned scratch; the opacity helper fills it before reading it.
    Sphere() {}
    // Copy world-space center and radius from an existing bounding sphere.
    Sphere(const Sphere& s):v(s.v),r(s.r) {}
};
// Render list links precede the record payload. Distances are squared world-space distances.
struct SceneRenderRecord {
    Link links;
    int kind;
    char padding0C[4];
    float drawScale;
    void* puppet;
    void* customDrawable;
    char displayName[12];
    char padding28[0x14];
    unsigned int flags;
    unsigned char opacity;
    unsigned char visible;
    char padding42[6];
    unsigned int renderParameter;
    float maximumDistanceSquared;
};
struct CameraTransformView { Vec3f right, up, forward, position; };
struct SceneDrawableLink { Link links; int kind; void* drawable; };
struct TransparentSceneLink { Link links; void* puppet; float drawScale; unsigned int parameters[3]; };
struct SceneDebugPointLink { Link links; Vec3f position; unsigned int color; };
struct SceneSortState { char precedingFields[0x134]; unsigned char kindSortPending; };
struct SceneHoleRecord { Vec3f tee, cup; unsigned char par; char remainingFields[7]; };
struct SceneSurfaceBounds { char precedingFields[0x64]; Vec3f minimum, maximum; };
struct SceneCameraMatrix { char precedingFields[0x14]; float worldToView[12]; };
struct SceneQuadTreeDebug {
    char precedingFields[0x20B4]; Vec3f selectedPoint;
    char precedingWaterLevel[0xC0]; float waterLevel;
};
struct PuppetOpacityView { char precedingFields[0x6C]; unsigned char opacity; };
struct ScenePuppetView {
    char precedingFields[0x108];
    int geometryCount;
    Vec3f boundsMin;
    Vec3f boundsMax;
    Sphere boundingSphere;
    char precedingOpacity[0x30];
    PuppetOpacityView* opacityState;
};
struct SharedWeatherView { char precedingFields[0x9C6]; unsigned char selectedWeather; };
// Renderer command 26 reads cached submission state at +0x4844;
// see RenderCourseScene for the provider, producer and distinct ==1/==2 consumers.
enum SceneResourceProtocol { kSceneResourceQuery1A = 0x1A, kSceneResourceMode2 = 2 };

// Pair counts in the linked renderer presets; neither count is a texture dimension.
enum ScenePresetCounts {
    kSceneRenderStatePairCount = 15,
    kSceneTextureStageStatePairCount = 6,
    kSceneTextureStageParameterPairCount = 5
};
enum SceneDebugDiskConstants {
    kSceneDebugCircleSegments = 128,
    kSceneDebugDiskFillColor = 0x66000000, kSceneDebugDiskRimColor = 0xFF000000,
    kSceneDebugProjectedDrawFlags = 0x300000 // Existing renderer protocol; individual bits unresolved.
};
enum ScenePoolIndex { kSceneBeachPool, kSceneRiverPool, kSceneTexturePool, kSceneBoundaryPool };
enum SceneOpacity { kOpaque = 255, kNearlyOpaque = 254, kCameraInsideOpacity = 63 };
// These names describe dispatch behavior visible here, not inferred gameplay object classes.
enum SceneRecordKind {
    kRecordShadowBoundary = 0, kRecordAlternateExcluded = 2, kRecordCountedPuppet3 = 3,
    kRecordInheritedOpacity = 4, kRecordSkipped = 5, kRecordDeferredDrawable = 6
};
enum CourseGeometryKind {
    kEntryProjectedQuads2 = 2, kEntryProjectedQuads3 = 3, kEntryBeach = 5,
    kEntryRiver = 7, kEntryGeneratedPolygon = 8, kEntryTextureStrip = 9, kEntryLava = 11,
    kBoundaryGroundGrid = 2
};
enum SceneRecordAndDebugBits {
    kDisableCameraFade = 1, kForceTransparentPass = 0x100,
    kDrawWaterPlane = 0x10, kDrawRaisedWaterPlane = 0x20
};
enum CourseWeather { kClearWeather = 0, kCloudyWeather = 1, kRainWeather = 2, kSnowWeather = 3, kSpecialWeatherEffect = 4 };
enum WeatherParticleCounts {
    kPetalParticleCount = 500, kLightSnowCount = 1024, kHeavySnowCount = 2048,
    kAmbientSnowCount = 4096, kSpecialParticleCount = 512
};
// These local course labels describe observed weather policy, not unverified display names.
// A numeric-to-display-name table is not established by this unit; keep the byte IDs visible.
enum WeatherCourseId {
    kWeatherCourse4 = 4, kWeatherCourse5 = 5, kWeatherCourse6 = 6,
    kWeatherCourse7 = 7, kRainSubstitutesSnowCourse8 = 8, kPersistentLightSnowCourse9 = 9,
    kWeatherPetalCourse11 = 11, kWeatherSilicaCourse12 = 12, kScaledAmbientSnowCourse13 = 13,
    kWeatherCourseByteMask = 0xFF,
    kWeatherPair11WrapBias = 0xF5, kWeatherPair8WrapBias = 0xF8
};

struct P30 { unsigned w[30]; };
struct P12 { unsigned w[12]; };
struct P10 { unsigned w[10]; };
struct Vertex { char data[16]; unsigned color; char rest[36]; };
extern void *g_sharedDoc, *g_golfCupPreview, *g_shadowManager, *g_view;
extern void **g_resourceManager;
extern signed char s_renderPacketInitialized;
extern int g_sceneRenderedGeometryCount,g_sceneCourseSurfaceCount,g_sceneRenderedPuppetCount,g_drawSceneDebugGeometry;
extern char s_cameraTransform[],s_renderPacket[],s_sceneRenderPacketDestructorRegistration[],s_sceneProjectedEffectVertexPointers[];
extern Vertex s_projectedEffectVertex0[4];
extern P30 kSceneRenderStatePreset;
extern P12 kSceneTextureStageStatePreset;
extern P10 kSceneTextureStageParameterPreset;
extern float kFloat_1_805DF9F8,kFloat_0_805DF9FC,kSceneClipNear,kSceneClipFar;
extern float kSceneProximityFadeDistanceSquared,kSceneProximityFadeRadiusLimit,kFloat_0_25_Scene,kSceneOpacityScale,kSceneDistanceFadeBandSquared,kSceneDebugVerticalLineHeight;
extern float kSceneDebugPointCrossScale,kSceneDropCircleRadiusInset,kSceneCircleFullTurnRadians,kSceneCircleSegmentReciprocal128,kFloat_192_SceneDebug,kSceneHalfScale;
void* memset(void*,int,unsigned);
void WView_ForwardRendererParameters(void*,int);
void appFreeUni__FPv(void*);
void* appMallocDummy__FUl(unsigned);
void WPuppet_SetOpacityRecursive(void*,int);
void CupPreview_CaptureModel(void*,void*);
void ShadowManager_RenderShadowTextures(void*,void*);
void CPvsFxBox_DrawEnabledEffects(void*,void*);
void CupPreview_DrawModel(void*,void*);
void Camera_SetClip(void*,float,float,int);
Vec3f* GolfBall__Fv();
float Math_SquareRootApprox(float);
float Math_SinApprox(float);
float Math_CosApprox(float);
void __pl__FRC7WVectorRC7WVector(void*,const void*,const void*);
void __mi__FRC7WVectorRC7WVector(void*,const void*,const void*);
void __ml__FfRC7WVector(void*,const void*,float);
void __as__7WVectorFRC7WVector(void*,const void*);
float Vec3_CalculateLengthSquared(void*);
float Magnitude__7WVectorCFv(void*);
void Engine_SetRenderStateList(void*,int);
void Engine_SetTextureStageStateList(int,void*,int);
void Engine_SetTextureStageParameterList(int,void*,int);
void Matrix_TransformPoint(void*,const void*,const void*);
void WView_ProjectViewPoint(void*,void*,void*,int);
void WView_DrawLine(void*,void*,unsigned,void*,unsigned,int);
int WView_DefaultCapabilityTrueC(void*,void*);
void ShadowManager_DrawProjectedModel(void*,void*);
void RenderPacket_Init(void*);
void RenderPacket_Destroy();
void RenderPacket_DrawBatches(void*,void*);
void CMarker_Draw(void*);
void* CQuadTree_GetInstance();
void CQuadTree_ReservedWiiHook(void*);
void TextureMesh_DrawLayers(void*);
void GroundGrid_DrawLitSurface(void*);
void CourseEffects_EmptyHook(void*,void*);
void* CourseData_FindSurfaceByName(void*,int);
void WFont_Print(void*,void*,void*,int,int,float,float);
// Return the active golf document through the shared-document singleton.
static inline void* currentGolfDocument() { return (void*)AsCSharedDoc(g_sharedDoc)->golfDoc; /* +0xb90 */ }
struct VObject {
    virtual void slot8();
    virtual void slotC();
    virtual void slot10();
    virtual void slot14();
    virtual void draw(void*,int,bool,bool,bool,float);
}
;
struct VCamera {
    virtual void slot8();
    virtual void slotC();
    virtual void flushQueue(int);
    virtual void slot14();
    virtual void drawScenePrimitive(void*,int,int,int);
}
;
struct VService {
// WGeneric3DR_DispatchCommand command ABI (+0x24): command, value word, output. Command 26 interprets VALUE as
// int*, ignores output and returns null.
    virtual void slot8();
    virtual void slotC();
    virtual void slot10();
    virtual void slot14();
    virtual void slot18();
    virtual void slot1C();
    virtual void slot20();
    // WGeneric3DR_DispatchCommand command ABI (+0x24): command, value word, output.
    // Command 26 interprets VALUE as int*, ignores output and returns null.
    // This local pointer prototype preserves that 32-bit ABI at the call site.
    virtual void query(int,int*,int);
}
;
struct VFont {
    virtual void slot8();
    virtual void slotC();
    virtual void slot10();
    virtual float width(void*,void*);
    virtual void slot18();
    virtual void slot1C();
    virtual void slot20();
    virtual void finish(void*);
}
;
// Dispatch one of the three scene-object render/update virtual hooks without changing ABI slots.
static inline void dispatchSceneObjectHook(void* p,int s) {
    if (s==8) ((VObject*)p)->slot8();
    else if (s==0x10) ((VObject*)p)->slot10();
    else ((VObject*)p)->slot14();
}
// Select the camera/render pass through its original virtual interface.
// WView slot +0x10 = WView_FlushRenderQueue, NOT a persistent camera mode.
// phase1_class_tables -> renderer command 11 -> WGeneric3D_Flush (wgeneric3d_render_queue_and_soft_keyboard.cpp):
// always ResetRenderState and zero vertexBufferBytesUsed; submissionState==1
// discards both entry counts. Otherwise an empty queue returns after reset;
// nonempty work runs ExecuteEarlyRenderPass, then either DrawTaggedQuadBatches
// (bit 4) or queued replay followed by ExecuteLateRenderPass. Both counts clear.
// flags 0: sorted single pass; 1: sorted two passes; 2: reverse single pass;
// 4: tagged quad batches instead of ordinary replay/late pass. These calls can
// draw, reset cached states and consume queues; they do not save/restore state.
// ResetRenderState's writes (wgeneric3dr_renderer_core.cpp): cached settings/texture indices/render
// flags=0; blend/vertex-format/draw-mode caches=-1; fog/color-ramp=0; pass range
// [0,1]; two packed defaults=0x1004; tail draw words/bytes=0. Those packed bits'
// individual meanings are unresolved, but their reset values are established.
// Replay executes entries with their captured resourceOverride, clearing that
// override after each execution; entry flags are also mutated by the 1/2 paths.
// Flag 4 additionally uploads s_generic3DQuadRenderDefaults (wgeneric3d_render_queue_and_soft_keyboard.cpp), camera
// view/projection and identity world/texture transforms, clears type-7 resource
// and selects the quad vertex format, then changes texture/palette/blend per
// batch. See DrawTaggedQuadBatches there for the full state-list/payload schema;
// no incoming GPU state snapshot is restored by this adapter or by the flush.
// Flags occupy the pointer-shaped command value word, with no payload to retain.
static inline void flushSceneViewQueue(void* p,int flags) { ((VCamera*)p)->flushQueue(flags); }
// Draw a puppet using its record scale and the common scene view.
static inline void drawScenePuppet(void* p,float f) {
    bool zero=false;
    ((VObject*)p)->draw(g_view,1,zero,zero,zero,f);
}
// Draw the supplied immediate-mode primitive through the camera interface.
static inline void drawScenePrimitive(void* cam,void* p,int flags) {
    ((VCamera*)cam)->drawScenePrimitive(p,0x10,flags,0);
}
// Unlink and free a temporary render list, decrementing its stored count for every node.
static inline void clearTransientRenderList(Link* sentinel,unsigned int& count) {
    Link* it; Link* end;
    end=sentinel; it=end->next;
    if(it!=end) {
        Link* last=end->prev;
        it->prev->next=last->next; last->next->prev=it->prev;
        while(it!=end) { Link* old=it; it=it->next; appFreeUni__FPv(old); --count; }
    }
}
// Sort the scene render records using a copied world-space reference position.
static inline void sortSceneRecordsByDistanceCopied(void* self,Vec3f v) {
    Vec3f copy=v;
    SceneRenderList_SortByDistanceDescending((SList*)((unsigned)&AsCSceneManager(self)->renderRecordCount),(SortKey*)&copy);
}
// Keep the original volatile vector-copy boundary before distance sorting.
static inline void sortSceneRecordsByDistance(void* self,volatile Vec3f v) {
    sortSceneRecordsByDistanceCopied(self,*(const Vec3f*)&v);
}
struct Flag { unsigned char v; };
// Run the signed payload sort using a copied zero key, preserving its volatile ABI boundary.
static inline void sortSceneRecordsByKindCopied(void* self,volatile Flag f) {
    Flag copy=*(const Flag*)&f;
    SceneRenderList_SortBySignedPayload((SList*)(&AsCSceneManager(self)->renderRecordCount),(char*)&copy.v);
}
// Build the zero-valued sort key used by the original scene pass.
static inline Flag makeZeroSortKey() { Flag f; f.v=0; return f; }
// Apply the payload sort to the scene render list after a distance-order change.
static inline void sortSceneRecordsByKind(void* self) { Flag a=makeZeroSortKey(); sortSceneRecordsByKindCopied(self,a); }
struct NodeGuard_u801E8048 {
    char flag; void* allocation; void* copy; void* head; void* n;
    // Initialize the allocator tag; init() arms the node pointer before a throwing operation.
    NodeGuard_u801E8048(): flag(0) {}
    // Free an unlinked node on unwind; setting n to zero commits ownership to the render list.
    ~NodeGuard_u801E8048() { if(n) appFreeUni__FPv(n); }
    // Record the newly allocated node and destination sentinel for exception cleanup.
    void init(void* p,void* h) { allocation=p; copy=p; head=h; n=p; }
};
// Append a freshly allocated node immediately before the circular-list sentinel.
static inline void append(Link* end,Link* n) {
    end->prev->next=n; n->prev=end->prev; end->prev=n; n->next=end;
}
struct WInstanceLock {
    OSMutex* mutex;
    // Acquire one recursive OS-mutex level for this scope.
    WInstanceLock(void* p):mutex((OSMutex*)p) { OSLockMutex(mutex); }
    // Release one recursive OS-mutex level, including on exception exit.
    ~WInstanceLock() { OSUnlockMutex(mutex); }
};

extern float kFloat_6_2831855;
extern float s_sceneRenderListAngleConstant;

// Ring contract shared by all three sort/merge policies:
// sentinel = address of the owner's prev link; empty means prev==next==sentinel,
// count==0. Every live link satisfies next->prev==self and prev->next==self.
// Split creates two disjoint rings whose counts sum to the input count. Merge
// transfers complete runs, closing the source gap before attaching destination
// endpoints; source finishes empty. Comparator policy never changes this protocol.
// Distance orders null puppets first (including ties between nulls); numeric and
// projection comparisons use strict ordering and leave equal keys in their input order.
// SNode/SList and ListNode/List are ABI views of the same link/count prefix.
struct ListNode {
    ListNode* prev;
    ListNode* next;
    void* value;
};

struct List {
    int count;
    ListNode* prev;
    ListNode* next;
};

// Move source's closed run [first,last] before destination, preserving the emitted cursor-update order.
// Preconditions: both rings are valid/disjoint; runEnd follows last. Payloads are never copied.
// The source gap is closed before destination links are attached; caller transfers runLength counts.
static inline void SpliceSceneMergeRun(ListNode* destination, ListNode* first, ListNode* last,
    ListNode*& sourceCursor, ListNode* runEnd) {
    first->prev->next = last->next;
    last->next->prev = first->prev;
    destination->prev->next = first;
    first->prev = destination->prev;
    sourceCursor = runEnd;
    destination->prev = last;
    last->next = destination;
}

// Empty source's sentinel ring, append its remaining nodes before destination, and transfer its count.
// first/last are caller snapshots: resetting source links must not erase the range endpoints.
static inline void AppendSceneMergeRemainder(List* destinationList, List* sourceList,
    ListNode* destination, ListNode* first, ListNode* last) {
    sourceList->next = (ListNode*)&sourceList->prev;
    sourceList->prev = (ListNode*)&sourceList->prev;
    destination->prev->next = first;
    first->prev = destination->prev;
    destination->prev = last;
    last->next = destination;
    destinationList->count += sourceList->count;
    sourceList->count = 0;
}

struct SortItem {
    char pad[0x124];
};

extern float __ml__FRC7WVectorRC7WVector(void*, void*);
extern void SceneList_Clear(void*);
extern void Vec3_Subtract(void*, void*, void*);
extern float Vec3_LengthSquared(void*);
extern void GeneratedSurface_Destroy(void*, int);
extern void GroundGrid_Dtor(void*, int);
extern void CMarker_ReleaseRenderResources(void*);

struct SortKey { int words[3]; };

struct SNode {
    SNode* prev;
    SNode* next;
    char* val;
};

struct SList {
    int count;
    SNode* prev;
    SNode* next;
    // Create an empty ring whose two link words themselves form the sentinel.
    SList() {
        count = 0;
        prev = next = (SNode*)&prev;
    }
    // Free remaining nodes only; payload objects belong to their enclosing scene owner.
    ~SList() {
        SNode* end = (SNode*)&prev;
        SNode* it = next;
        if (it != end) {
            SNode* last = prev;
            it->prev->next = last->next;
            last->next->prev = it->prev;
            while (it != end) {
                SNode* cur = it;
                it = it->next;
                appFreeUni__FPv(cur);
                --count;
            }
        }
    }
};

struct SortRecord {
    char pad[0x124];
};

// Put null puppets first; otherwise prefer the puppet farther from the reference point.
static inline bool SNode_swapNeeded(SNode* node, SNode* other, SortKey* key) {
    if (((SceneRenderRecord*)other)->puppet == 0)
        return true;
    if (((SceneRenderRecord*)node)->puppet == 0)
        return false;
    unsigned int tmp[3];
    Vec3_Subtract(tmp, &((ScenePuppetView*)((SceneRenderRecord*)other)->puppet)->boundingSphere.v, key);
    float av = Vec3_LengthSquared(tmp);
    Vec3_Subtract(tmp, &((ScenePuppetView*)((SceneRenderRecord*)node)->puppet)->boundingSphere.v, key);
    float bv = Vec3_LengthSquared(tmp);
    return av > bv;
}

// Compare puppet positions by projection onto the camera direction, farthest first.
static inline bool SList_less(char* l, char* r, SortKey* key) {
    return (*(WVector*)r * *(WVector*)key) > (*(WVector*)l * *(WVector*)key);
}

// Use separate dot-product temporaries to preserve the original comparison and register lifetimes.
static inline bool List_dotGreater(void* r, void* l, void* key) {
    float lv = (*(WVector*)l * *(WVector*)key);
    float rv = (*(WVector*)r * *(WVector*)key);
    return rv > lv;
}

struct DNode {
    void* value;
    char padding04[8];
    DNode* prev;
    DNode* next;
};
// Cleanup/iteration view of the four 0x38-byte ownership pools described at Fn801EBList below.
// activeRing and reusableRing anchor the active and free rings; iterationCursor tracks traversal.
// lookupEntryCount/array at +0x18/+0x1c describe the indexed lookup; lock begins at +0x20. Payload destruction runs
// outside the per-pop lock, then nodes are recycled and lookup entries cleared under the lock.
// Cross-view aliases (offsets within one pool):
//   +00 DList.activeRing = Fn801EBList.activeRing = active ring anchor
//   +04 DList.iterationCursor = append view's padding04 = traversal cursor
//   +08 DList.padding08 = Fn801EBList.slabRing = slab-allocation ring
//   +0c DList.reusableRing = Fn801EBList.reusableRing = reusable-node ring anchor
//   +10 DList.padding10 = Fn801EBList.nodesPerSlab = nodes per allocation (not live size)
// All rings are null or circular without sentinels. DNode/Fn801EBNode agree:
// +0c prev, +10 next. Ordinary Link/SNode are DIFFERENT prefixes (+0/+4 links).
// Beach rendering starts at active.prev and walks prev, stopping after active;
// other pools start at active and walk next, stopping before returning to active.
// DList_NextObject follows next during destruction. Thus render traversal order can
// differ from destruction order; never normalize the link direction by spelling.
struct DList {
    DNode* activeRing;
    DNode* iterationCursor;
    char padding08[4];
    DNode* reusableRing;
    char padding10[8];
    int lookupEntryCount;
    void** array;
    char lock[4];
    char padding24[0x14];
};
// Return the active-ring anchor; this is neither a sentinel nor necessarily the first draw node.
static inline DNode*& PoolActiveRing(DList* pool) { return pool->activeRing; }
// Return the traversal cursor, whose null value means iteration is complete.
static inline DNode*& PoolIterationCursor(DList* pool) { return pool->iterationCursor; }
// Read and advance a typed pool cursor while locked; drawing occurs after the lock is released.
// Beaches include the anchor then stop in reverse order; other pools stop on wrapping forward.
static inline void* nextSceneListObject(DList* pool,bool reverse) {
    OSMutex* mutex=(OSMutex*)&pool->lock;
    OSLockMutex(mutex);
    DNode* n=PoolIterationCursor(pool); void* obj;
    if(n) {
        obj=n->value;
        if(reverse) PoolIterationCursor(pool)=n==PoolActiveRing(pool)?0:n->prev;
        else { DNode* next=n->next; PoolIterationCursor(pool)=next==PoolActiveRing(pool)?0:next; }
        OSUnlockMutex(mutex);
    } else { OSUnlockMutex(mutex); obj=0; }
    return obj;
}
struct Instance { 
virtual ~Instance();
};
struct CleanupClass {
    char pad[0x10];
    DList lists[4];
    void* owned;
    void* last;
};

// Return the cursor object and advance to the next active node under the list mutex.
static inline void* DList_NextObject(DList* l, void* mtx) {
    OSLockMutex((OSMutex*)mtx);
    DNode* n = l->iterationCursor;
    if (n) {
        DNode* nx = n->next;
        void* v = n->value;
        l->iterationCursor = (nx == l->activeRing) ? 0 : nx;
        OSUnlockMutex((OSMutex*)mtx);
        return v;
    }
    OSUnlockMutex((OSMutex*)mtx);
    return 0;
}

// Return the active nodes to the reusable circular pool without freeing their allocation.
static inline void DList_clear(DList* l) {
    if (l->activeRing && l->reusableRing) {
        l->activeRing->prev->next = l->reusableRing->next;
        l->reusableRing->next->prev = l->activeRing->prev;
        l->reusableRing->next = l->activeRing;
        l->activeRing->prev = l->reusableRing;
        l->activeRing = 0;
    } else if (l->activeRing) {
        char* mtx = (char*)&l->lock;
        DNode* nx;
        DNode* n;
        while (l->activeRing) {
            n = l->activeRing;
            OSLockMutex((OSMutex*)mtx);
            n->next->prev = n->prev;
            n->prev->next = n->next;
            nx = n->next;
            if (nx == n) nx = 0;
            OSUnlockMutex((OSMutex*)mtx);
            n = l->activeRing;
            DNode* last = l->reusableRing;
            OSLockMutex((OSMutex*)mtx);
            if (!last) {
                n->prev = n;
                n->next = n;
                OSUnlockMutex((OSMutex*)mtx);
            } else {
                n->next = last;
                n->prev = last->prev;
                last->prev = n;
                n->prev->next = n;
                OSUnlockMutex((OSMutex*)mtx);
                n = last;
            }
            l->reusableRing = n;
            l->activeRing = nx;
        }
    }
}

// Return active nodes to the reusable pool with the original base-object cleanup schedule.
static inline void DList_clear2(DList* l) {
    if (l->activeRing && l->reusableRing) {
        l->activeRing->prev->next = l->reusableRing->next;
        l->reusableRing->next->prev = l->activeRing->prev;
        l->reusableRing->next = l->activeRing;
        l->activeRing->prev = l->reusableRing;
        l->activeRing = 0;
    } else if (l->activeRing) {
        DNode* n;
        DNode* last;
        DNode* nx;
        char* mtx;
        mtx = (char*)&l->lock;
        while (l->activeRing) {
            n = l->activeRing;
            OSLockMutex((OSMutex*)mtx);
            n->next->prev = n->prev;
            n->prev->next = n->next;
            nx = n->next;
            if (nx == n) nx = 0;
            OSUnlockMutex((OSMutex*)mtx);
            n = l->activeRing;
            last = l->reusableRing;
            OSLockMutex((OSMutex*)mtx);
            if (!last) {
                n->prev = n;
                n->next = n;
                OSUnlockMutex((OSMutex*)mtx);
            } else {
                n->next = last;
                n->prev = last->prev;
                last->prev = n;
                n->prev->next = n;
                OSUnlockMutex((OSMutex*)mtx);
                n = last;
            }
            l->reusableRing = n;
            l->activeRing = nx;
        }
    }
}

// Clear the list's indexed object lookup array after object destruction.
static inline void DList_zero(DList* l) {
    for (int i = 0; i < l->lookupEntryCount; i++) l->array[i] = 0;
}

// Destroy a polymorphic owned scene object through its original base destructor.
static inline void DList_releaseInstance(void* p) {
    delete (Instance*)p;
}

void __ct__18NativeBase80201D18Fv(void* self);
void InputManager_Init(void* self);
void InputManager_SetCursorDrawOptions(void* object, int a, int b, int c, int index);
void InputManager_SetCursorColorOverride(void* object, int a, int b, int c, int d, int index);

// Constructor-only base fields coexist with the semantic score fields in CScore.h.
struct ScoreConstructorView {
	void* vtable;
	char padding04[4];
	int actorType;
	char padding0C[0x0C];
	void* actorInterfaceVtable; // secondary actor-interface vtable; borrowed static table
	int field1C; // overlays CScore::bonusOverlayEnabled
	float field20; // overlays CScore::elapsedTime
	int field24; // overlays CScore::transitionState
	int field28; // constructor flag set to 1; meaning unresolved
	float field2C; // overlays CScore::transitionProgress
	char padding30[8];
	int bonusOverlay; // owned CScore::bonusOverlay; lifecycle contract beside CScore_Ctor below
	int field3C; // overlays CScore::result
	int field40; // overlays CScore::scoreMenuActive
	int field44; // overlays CScore::currentPlayerId
	int field48; // state word cleared by constructor; role unresolved
	int field4C; // state word cleared by constructor; role unresolved
	char padding50[4];
	int field54; // overlays CScore::menusInitialized
	char padding58[4];
	int field5C; // overlays the owned CScore::scoreForm pointer word
	int field60; // overlays the owned CScore::rankView pointer word
};

extern char __vt__6CScore[];
extern const float kScoreInitialTime;

// Secondary actor-interface vtable shares storage with the constructor's raw +0x18 word.
static inline void*& ScoreActorInterfaceVtable(ScoreConstructorView* score) { return score->actorInterfaceVtable; }

struct ScoreFormOwnerView { char padding00[0x5C]; void* scoreForm; void* rankView; };
struct ScoreOwned { 
virtual void release(int);
};
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_scoreInputManagerDestructorRegistration[];
void InputManager_Dtor();

struct Fn801EC280Args { int values[8]; };
struct Fn801EC280Object { void** vtable; };
// Virtual actor command hook at slot +0x2C; commands 3 and 6 initialize the score actor.
class V2Cs {
    public: 
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C(Fn801EC280Args*);
}
;

struct Fn801EB248Object {
	char padding00[0x7C];
	void* field7C;
};

struct Fn801EB238Object {
	char padding00[0x9C];
	char* items;
};

// Native Wii course boundary record (same 0x5C-byte view as CourseBoundsRecord
// in project_g_app_and_devices.cpp). GroundGrid_ConfigureSurfaceBounds (course_water_surface_rendering.cpp) consumes +0x44
// as six floats {minXYZ,maxXYZ}, and +1 as NUL-terminated property text containing
// *type/*rot/*noripple. Text must terminate before the bounds; no length is
// supplied or checked. This record is not a point vector: four world corners
// on max.y are generated by the callee. Neither interior pointer is retained.
struct Fn801EB238Item {
	unsigned char kind;
	char properties[0x43];
	Vec3f minimum, maximum;
};
typedef char SceneBoundaryRecord_size[sizeof(Fn801EB238Item) == 0x5C ? 1 : -1];
typedef char SceneBoundaryRecord_bounds[((int)&((Fn801EB238Item*)0)->minimum == 0x44) ? 1 : -1];

struct Fn801EB82CObject {
	void* field00;
};

struct Fn801EB1FCObject {
	void* vtable;
};

extern char __vt__6CBeach[];

struct Fn801EBLock {
	// Lock the mutex represented by this address; no separate mutex allocation is owned.
	Fn801EBLock() { OSLockMutex((OSMutex*)this); }
	// Release the matching recursive lock acquisition.
	~Fn801EBLock() { OSUnlockMutex((OSMutex*)this); }
};
extern void* appMallocUni__FUl(unsigned int size);

// Pool layout (0x38 bytes): activeRing +0 = active ring; +4 = iteration cursor;
// slabRing +8 = slab-header ring; reusableRing +0xC = free ring; nodesPerSlab +0x10 = allocation size;
// +0x1C is an indexed object lookup used by cleanup; mutex starts at +0x20.
// Each slab has nodesPerSlab 0x18-byte nodes: node 0 tracks the allocation, nodes 1..nodesPerSlab-1 are usable.
// Thus nodesPerSlab must exceed 1. Null ring means empty; nonempty rings have no sentinel node.
// The four append implementations retain their different compiler schedules but share one protocol:
// reserve/register slab -> populate free ring -> detach free node -> initialize -> splice active.
// Nested locks acquire the same recursive OS mutex; each helper balances its own acquisition.
// *value is copied, not the wrapper pointer. Scene cleanup deletes the payloads before recycling
// nodes; clearing a ring alone does not delete payloads or free the backing slabs.
// Append entry contract for all four implementations: initialized pool with
// nodesPerSlab>=2, required readable value[1], and a uniquely owned payload
// suitable for that pool's cleanup (beach/river/texture: virtual deleting dtor;
// boundary: GroundGrid_Dtor). Do not append the same owner twice. On return the
// scene owns *value; the caller's pointer word is NOT cleared or retained.
// Publication is the final activeRing store, after detached-node initialization.
// Slab allocation must return usable storage or throw: null is dereferenced,
// never reported as an append failure. On allocation throw before publication,
// WInstanceLock unwinds, activeRing is unchanged, and payload ownership remains
// with the caller; these append functions do not delete it. CGolfBg_CreateCourseObjects
// holds raw new results, so that exceptional path has no local payload guard.
// CSceneManager_ClearOwnedSceneObjects releases published payloads, recycles
// active nodes and clears lookups, retaining slab storage for later reuse.
// Ring direction follows DNode: +0xC is the preceding link, +0x10 the following link.
struct Fn801EBList {
	void* activeRing;
	char padding04[4];
	void* slabRing;
	void* reusableRing;
	int nodesPerSlab;
	char padding14[0x0C];
	char mutexStorage[0x18];
};
// Return the allocation size in nodes, including the slab-header node (not live object count).
static inline int PoolNodesPerSlab(Fn801EBList* pool) { return pool->nodesPerSlab; }
struct Fn801EBNode {
	void* value;
	unsigned int renderState;
    unsigned char renderFlags;
    char padding09[3];
	Fn801EBNode* prev;
	Fn801EBNode* next;
	char padding14[4];
};

// Make a detached node into a one-node ring; caller holds the recursive pool mutex.
static inline void InitializeScenePoolRing(Fn801EBNode* node) {
    node->prev = node;
    node->next = node;
}
// Insert a detached node immediately before an existing anchor, preserving the anchor.
static inline void SpliceScenePoolNodeBefore(Fn801EBNode* node, Fn801EBNode* anchor) {
    node->next = anchor;
    node->prev = anchor->prev;
    anchor->prev = node;
    node->prev->next = node;
}
// Disconnect a node; caller handles singleton-to-null conversion and publishes the new head.
static inline void UnlinkScenePoolNode(Fn801EBNode* node) {
    node->next->prev = node->prev;
    node->prev->next = node->next;
}

struct Fn801EC088Object {
	char padding00[0x64];
	// Construct the complete score actor in the 0x64-byte allocation.
	Fn801EC088Object() { CScore_Ctor(reinterpret_cast<ScoreConstructorView*>(this)); }
};

extern void* CourseEntryIterator_DefaultConstruct(void* self);
extern void* GolfDoc_GetCurrent();
extern unsigned int CourseEntryVector_Begin(void* self);
extern void* CourseEntryIterator_CopyConstruct(void* a, void* b, int c);
extern void CourseEntryIterator_Assign(void* a, void* b);
extern void* CourseEntryIterator_GetEntry(void* self);
extern int CourseEntry_GetKind(void* self);
extern void* __nw__FUl(unsigned int size);
extern void GeneratedSurface_Init(void* self);
extern void* CourseEntry_GetPointVector(void*);
extern void Vec3Vector_CopyCtor(void* self, void* arg);
extern void GeneratedSurface_BuildFromPolygon(void* self, void* arg);
extern void dtor_801247CC(void* self, int flags);
extern void MWString_CopyConstructForNameLookup(void* self, void* arg);
extern void* MWString_GetCString(void* self);
extern void GroundStrip_BuildGeometry(void* a, void* b, unsigned int c, int d);

extern void CRiver_Ctor(void* self);
extern void ntRiver_Lava_Ctor(void* self);
extern unsigned int SharedDoc_GetCourseId();
extern void GroundStripOwner_Init(void* self);
// Tests course IDs 8 and 13; this caller uses the result for ambient weather.
extern int SharedDoc_IsCourse8Or13();
extern void ntPetal_Ctor(void*, int, const char*, int);
extern void ntSilicaParticle_Ctor(void*, int, const char*, int);
extern void CSnow_Ctor(void*, int, const char*, int, float);
extern void CRain_ctor(void*);
extern int rand();
extern void CShadowManager_SetSelectionFlags(void*, int);
extern void __dl__FPv(void*);
extern const char courseGlowEffectStrings[];
extern char __vt__6CCloud[];
extern float kWeatherPetalEffectScale, kWeatherSnowConstructorScale, kWeatherCourse13SnowCountScale;
extern float kWeatherAmbientSnowConstructorScale, kWeatherAmbientSnowEffectScale, kWeatherSpecialEffectScale;

// Weather owner aliases agree with CSceneManager.weatherEffect and secondaryWeatherEffect.
// Both slots are owned, released through virtual slot 0 with
// deleting flag 1. This policy creates only +f8; +fc's producer is unresolved.
typedef CSceneManager CourseWeatherOwner;
struct Fn801EBA30Effect {
    void** vtable;
    float field04;
    char padding08[0x5C];
};
struct VEffect {
    virtual void release(int);
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void update(int, float);
};

// Access the owner's primary effect slot; this slot can hold petals, silica, snow, cloud or rain.
static inline void*& OwnedCourseWeatherEffect(CourseWeatherOwner* owner) { return owner->weatherEffect; }
// Access the second owned effect slot; this unit releases it but does not establish its producer.
static inline void*& OwnedSecondaryWeatherEffect(CourseWeatherOwner* owner) { return owner->secondaryWeatherEffect; }
// Access the constructor-specific scale word; its visual units are not established here.
static inline float& CourseWeatherEffectScale(void* effect) { return ((Fn801EBA30Effect*)effect)->field04; }

// Release a deleting-virtual effect and clear the owning slot; callers choose when recreation is needed.
static inline void ReleaseOwnedWeatherEffect(void*& ownedEffect) {
    Fn801EBA30Effect* weatherEffect = (Fn801EBA30Effect*)ownedEffect;
    if (weatherEffect != 0) {
        if (weatherEffect != 0) ((VEffect*)weatherEffect)->release(1);
        ownedEffect = 0;
    }
}

// Allocation-view structs for the weather/scenery objects, each named after the class it constructs and sized to it:
// PetalEffectAlloc=ntPetal, SilicaParticleEffectAlloc=ntSilicaParticle, SnowEffectAlloc=CSnow, CloudEffectAlloc=CCloud, RainEffectAlloc=CRain.
// (They are allocation views, not weather IDs.) field04 is forwarded
// effect scale where written; its precise visual units are not established here.
struct PetalEffectAlloc {
    void** vtable;
    float field04;
    char pad[0x24];
    // Construct the petal particle effect in owned storage.
    PetalEffectAlloc(int a, const char* s, int c) { ntPetal_Ctor(this, a, s, c); }
};
struct SilicaParticleEffectAlloc {
    void** vtable;
    float field04;
    char pad[0x24];
    // Construct the silica particle effect in owned storage.
    SilicaParticleEffectAlloc(int a, const char* s, int c) { ntSilicaParticle_Ctor(this, a, s, c); }
};
struct SnowEffectAlloc {
    void** vtable;
    float field04;
    char pad[0x5C];
    // Construct the snow effect, forwarding particle count, resource and scale.
    SnowEffectAlloc(int a, const char* s, int c, float f) { CSnow_Ctor(this, a, s, c, f); }
};
struct CloudEffectAlloc {
    void** vtable;
    float field04;
    // Initialize the cloud effect with unit scale and its original vtable.
    CloudEffectAlloc() { field04 = kFloat_1_805DF9F8; vtable = (void**)__vt__6CCloud; }
};
struct RainEffectAlloc {
    char pad[0x28];
    // Construct the rain effect in owned storage.
    RainEffectAlloc() { CRain_ctor(this); }
};

extern void DrawScene_NoOpHook(void* self, int a);
extern void GroundGrid_Ctor(void* self);
extern void GroundGrid_ConfigureSurfaceBounds(void* self, void* a, void* b);
extern void CMarker_CreateRenderResources(void* self);
extern void* CourseResource_GetSelectedVariant();
extern void BuildGroundProjectedPointQuads(void* self, void* arg);
extern unsigned int CourseEntryVector_End(void* self);
extern int CourseEntryIterator_NotEqual(void* a, void* b);

// Course-scene construction view: list10 beaches, list48 rivers/lava, list80 texture strips,
// listB8 boundary grids; fieldF0 owns the single generated polygon surface, fieldF4 the marker.
// Construction view of the course scene ownership pools.
struct CourseSceneConstructionView {
	char padding00[0x10];
	Fn801EBList beachPool;
	Fn801EBList riverAndLavaPool;
	Fn801EBList generatedTexturePool;
	Fn801EBList boundaryPool;
	void* generatedSurface;
	void* marker;
};

// Borrow the beach ownership pool embedded in the course scene.
static inline Fn801EBList& CourseSceneBeachPool(CourseSceneConstructionView* scene) { return scene->beachPool; }
// Borrow the shared river/lava ownership pool.
static inline Fn801EBList& CourseSceneRiverPool(CourseSceneConstructionView* scene) { return scene->riverAndLavaPool; }
// Borrow the generated-texture-strip ownership pool.
static inline Fn801EBList& CourseSceneTexturePool(CourseSceneConstructionView* scene) { return scene->generatedTexturePool; }
// Borrow the boundary-grid ownership pool.
static inline Fn801EBList& CourseSceneBoundaryPool(CourseSceneConstructionView* scene) { return scene->boundaryPool; }

// Destroy a copied point-vector temporary without deleting its stack wrapper.
struct Dtor47CC { char pad[0xC]; ~Dtor47CC(){ dtor_801247CC(this, -1); } };
// Destroy a copied entry-name string without deleting its stack wrapper.
struct Dtor463FC { char pad[0xC]; ~Dtor463FC(){ ((NString*)(this))->~NString(); } };

// Construct an owned generated polygon surface.
struct GeneratedSurfaceAlloc { char pad[0x3C]; GeneratedSurfaceAlloc() { GeneratedSurface_Init(this); } };
// Construct an owned beach strip.
struct BeachAlloc { char pad[0x54]; BeachAlloc() { CBeach_Ctor(reinterpret_cast<Fn801EB1FCObject*>(this)); } };
// Construct an owned generated-texture strip.
struct GeneTex3DAlloc { char pad[0x54]; GeneTex3DAlloc() { GroundStripOwner_Init(this); } };
// Construct an owned river surface.
struct RiverAlloc { char pad[0x6C]; RiverAlloc() { CRiver_Ctor(this); } };
// Construct an owned lava surface.
struct LavaRiverAlloc { char pad[0x6C]; LavaRiverAlloc() { ntRiver_Lava_Ctor(this); } };
// Construct an owned boundary ground grid.
struct N190 { char pad[0x190]; N190() { GroundGrid_Ctor(this); } };
struct VObj { 
virtual void v0(); virtual void m(void*, unsigned int);
};

#pragma opt_common_subs off
// Draw overlay hooks, then deferred custom drawables, and establish the transparent-pass state.
// The caller retains its iteration/drawable scratch variables to preserve their ABI lifetimes.
static inline void RenderCourseOverlaysAndDeferred(void* self, void* camera, Link*& renderLink, void*& drawable) {
    flushSceneViewQueue(camera,1);
    for (renderLink=(*(Link**)&AsCSceneManager(self)->firstOverlayDrawable);(Link*)(&AsCSceneManager(self)->lastOverlayDrawable) != renderLink;renderLink=renderLink->next) {
        drawable=((SceneDrawableLink*)renderLink)->drawable;
        if (drawable) dispatchSceneObjectHook(drawable,8);
    }
    flushSceneViewQueue(camera,0);
    AsCSceneManager(self)->effectRenderPassActive=1;
    for (renderLink=(*(Link**)&AsCSceneManager(self)->firstDeferredDrawable);!same_pos(renderLink, (Link*)(&AsCSceneManager(self)->lastDeferredDrawable));renderLink=renderLink->next) {
        drawable=((SceneDrawableLink*)renderLink)->drawable;
        if (drawable) dispatchSceneObjectHook(drawable,8);
    }
    drawable=AsCGolfDoc(currentGolfDocument())->courseEffectBox;
    if(drawable) CourseEffects_EmptyHook(drawable,camera);
    flushSceneViewQueue(camera,4);
    AsCSceneManager(self)->effectRenderPassActive=0;
}

// Allocate/link copied transparent draw parameters; the puppet remains borrowed from scene ownership.
struct SceneTransparentPayload { void* puppet; float drawScale; unsigned renderParameter, reserved0, reserved1; };
// The caller retains the cleanup guard; setting guard.n to zero commits list ownership.
#pragma push
#pragma always_inline on
// Append copied parameters to this frame's transparent queue and commit the caller's node guard.
static inline void AppendCourseTransparentPuppet(void* self, const SceneTransparentPayload& data, NodeGuard_u801E8048& guard) {
    Link* queueSentinel=(Link*)(&AsCSceneManager(self)->lastTransparentObject);
    Link* newNode=(Link*)appMallocDummy__FUl(0x1c);
    guard.init(newNode,queueSentinel);
    if(&((TransparentSceneLink*)newNode)->puppet) {
        try {
            *(SceneTransparentPayload*)&((TransparentSceneLink*)newNode)->puppet=data;
        } catch(...) {
            appFreeUni__FPv(newNode);
            throw;
        }
    }
    append(queueSentinel,newNode);
    ++(*(unsigned*)&AsCSceneManager(self)->transparentObjectCount);
    guard.n=0;
}

// Select inherited/forced opacity or calculate near/far distance fades; distances are squared world units.
static inline void UpdateCourseRecordOpacity(
    void* self, Link*& renderLink, void*& drawable, Vec3f& cameraToObject, Vec3f& cameraDelta, Sphere& boundsSphere) {
    // 3c. Select inherited/forced opacity, or compute near and far fades.
    // The computed byte is also pushed recursively into puppet materials.
    if (((SceneRenderRecord*)renderLink)->kind==kRecordInheritedOpacity) ((SceneRenderRecord*)renderLink)->opacity=((ScenePuppetView*)((SceneRenderRecord*)renderLink)->puppet)->opacityState->opacity;
    else if (((unsigned)AsCGolfDoc(currentGolfDocument())->gameStateFlags)&kSceneStateStorySequence) ((SceneRenderRecord*)renderLink)->opacity=kOpaque;
    else if(((SceneRenderRecord*)renderLink)->puppet) {

        __mi__FRC7WVectorRC7WVector(&cameraDelta,(&((CameraTransformView*)s_cameraTransform)->position),(&((ScenePuppetView*)((SceneRenderRecord*)renderLink)->puppet)->boundingSphere.v));
        __as__7WVectorFRC7WVector(&cameraToObject,&cameraDelta);
        float distanceSquared=Vec3_CalculateLengthSquared(&cameraToObject);
        int opacity;
        if(distanceSquared>kSceneProximityFadeDistanceSquared) ((SceneRenderRecord*)renderLink)->opacity=kOpaque;
        else {
            if ((((SceneRenderRecord*)renderLink)->flags&kDisableCameraFade) || !AsCSceneManager(self)->cameraProximityFadeEnabled) opacity=kOpaque;
            else {
                drawable=((SceneRenderRecord*)renderLink)->puppet;
                float fadeRadius=kSceneProximityFadeRadiusLimit;
                *(Arr3*)&boundsSphere.v=*(Arr3*)&((ScenePuppetView*)drawable)->boundingSphere.v; boundsSphere.r=((ScenePuppetView*)drawable)->boundingSphere.r;
                float objectRadius=boundsSphere.r;
                fadeRadius=objectRadius>fadeRadius?fadeRadius:objectRadius;
                // Near fade subtracts the capped fade radius squared from center distance squared.
                // Inside the radius use opacity 63; otherwise clamp sqrt(outsideRadiusSquared)/fadeRadius.
                float outsideRadiusSquared=distanceSquared-fadeRadius*fadeRadius;
                if(outsideRadiusSquared<=kFloat_0_805DF9FC) opacity=kCameraInsideOpacity;
                else if(outsideRadiusSquared>fadeRadius*fadeRadius) opacity=kOpaque;
                else {
                    float distanceRatio=Math_SquareRootApprox(outsideRadiusSquared)/fadeRadius,clampedRatio=kFloat_0_25_Scene;
                    clampedRatio=distanceRatio<clampedRatio?clampedRatio:(distanceRatio>kFloat_1_805DF9F8?kFloat_1_805DF9F8:distanceRatio);
                    opacity=(int)(kSceneOpacityScale*clampedRatio);
                }
            }
            ((SceneRenderRecord*)renderLink)->opacity=opacity;
        }
        // Fade linearly in squared distance across the band below the maximum distance.
        if(((SceneRenderRecord*)renderLink)->opacity==kOpaque) {
            float maximumDistanceSquared=((SceneRenderRecord*)renderLink)->maximumDistanceSquared;
            if(maximumDistanceSquared>kFloat_0_805DF9FC && distanceSquared>=maximumDistanceSquared-kSceneDistanceFadeBandSquared)
                ((SceneRenderRecord*)renderLink)->opacity=(int)(kSceneOpacityScale*(maximumDistanceSquared-distanceSquared)/kSceneDistanceFadeBandSquared);
            else if (((SceneRenderRecord*)renderLink)->flags&kForceTransparentPass) ((SceneRenderRecord*)renderLink)->opacity=kNearlyOpaque;
        }
    } else ((SceneRenderRecord*)renderLink)->opacity=kOpaque;
}

// Persist material opacity and draw the puppet when the current opaque-pass policy allows it.
static inline void RenderCourseRecordPuppet(void* self, Link*& renderLink, void*& drawable) {
    drawable=((SceneRenderRecord*)renderLink)->puppet;
    unsigned char alpha=((SceneRenderRecord*)renderLink)->opacity;
    if(alpha!=((ScenePuppetView*)drawable)->opacityState->opacity) WPuppet_SetOpacityRecursive(drawable,alpha);
    // 3e. Normal selection draws opaque non-boundaries now; alternate
    // selection draws every kind except 2 here, even if alpha is reduced.
    if(!AsCSceneManager(self)->alternateRenderSelection) {
        if (((SceneRenderRecord*)renderLink)->opacity==kOpaque && ((SceneRenderRecord*)renderLink)->kind!=kRecordShadowBoundary) drawScenePuppet(((SceneRenderRecord*)renderLink)->puppet,((SceneRenderRecord*)renderLink)->drawScale);
    }
    else if (((SceneRenderRecord*)renderLink)->kind!=kRecordAlternateExcluded) drawScenePuppet(((SceneRenderRecord*)renderLink)->puppet,((SceneRenderRecord*)renderLink)->drawScale);
}

// Draw projected model shadows and initialize/draw their render packet at a boundary record.
static inline void RenderCourseShadowBoundary(void* camera, Link*& renderLink) {
    if(((SceneRenderRecord*)renderLink)->kind==kRecordShadowBoundary && g_shadowManager) {
        ShadowManager_DrawProjectedModel(g_shadowManager,camera);
        if(!Sing_805DC7B0()) {
            if(!s_renderPacketInitialized) {
                RenderPacket_Init(s_renderPacket);
                __register_global_object(s_renderPacket,(void*)RenderPacket_Destroy,s_sceneRenderPacketDestructorRegistration);
                s_renderPacketInitialized=1;
            }
            Sing_805DC7B0()=s_renderPacket;
        }
        RenderPacket_DrawBatches(Sing_805DC7B0(),camera);
    }
}

// Draw allowed markers and active weather slots, then select the course-surface camera state.
static inline void RenderCourseMarkerAndWeather(void* self, void* camera) {
    if (!(((unsigned)AsCGolfDoc(currentGolfDocument())->gameStateFlags)&kMarkerHiddenStates) && AsCSceneManager(self)->marker) CMarker_Draw(AsCSceneManager(self)->marker);
    if(AsCSceneManager(self)->weatherEffect) dispatchSceneObjectHook(AsCSceneManager(self)->weatherEffect,0x10);
    if(AsCSceneManager(self)->secondaryWeatherEffect) dispatchSceneObjectHook(AsCSceneManager(self)->secondaryWeatherEffect,0x10);
    if(AsCSceneManager(self)->weatherEffect||AsCSceneManager(self)->secondaryWeatherEffect) flushSceneViewQueue(camera,0);
}

// Draw the selected quadtree point and invoke its reserved debug hook.
static inline void RenderCourseQuadTreeDebug(
    Vec3f& verticalOffset, Vec3f& selectedPointForOffset, Vec3f& selectedPointForLine, Vec3f& lineEndWorld) {
    if(g_drawSceneDebugGeometry) {
        verticalOffset.x=kFloat_0_805DF9FC; verticalOffset.y=kSceneDebugVerticalLineHeight; verticalOffset.z=kFloat_0_805DF9FC;
        *(Arr3*)&selectedPointForOffset=*(Arr3*)&((SceneQuadTreeDebug*)CQuadTree_GetInstance())->selectedPoint; *(Arr3*)&selectedPointForLine=*(Arr3*)&((SceneQuadTreeDebug*)CQuadTree_GetInstance())->selectedPoint;
        __pl__FRC7WVectorRC7WVector(&lineEndWorld,&selectedPointForOffset,&verticalOffset);
        WView_DrawLine(g_view,&selectedPointForLine,0xffff0000,&lineEndWorld,0xffff0000,0);
        CQuadTree_ReservedWiiHook(CQuadTree_GetInstance());
    }
}

// Draw a debug water plane; raised selects the diagnostic plane 192 world units higher.
// cornerIndex/cornerWorld/cornerView are borrowed caller scratch for the original frame layout.
static inline void RenderCourseDebugWaterPlane(void* self, void* camera, bool raised,
    int& cornerIndex, Vec3f& cornerWorld, Vec3f& cornerView) {
    if((*(unsigned*)&AsCSceneManager(self)->debugDrawFlags)&(raised ? kDrawRaisedWaterPlane : kDrawWaterPlane)) {
        void* surfaceBounds=CourseData_FindSurfaceByName(AsCGolfDoc(currentGolfDocument())->courseData,0);
        for(cornerIndex=0;cornerIndex<4;++cornerIndex) {
            cornerWorld.x=(cornerIndex&1)?((SceneSurfaceBounds*)surfaceBounds)->maximum.x:((SceneSurfaceBounds*)surfaceBounds)->minimum.x;
            cornerWorld.z=(cornerIndex&2)?((SceneSurfaceBounds*)surfaceBounds)->maximum.z:((SceneSurfaceBounds*)surfaceBounds)->minimum.z;
            if (raised) cornerWorld.y=kFloat_192_SceneDebug+((SceneQuadTreeDebug*)CQuadTree_GetInstance())->waterLevel;
            else cornerWorld.y=((SceneQuadTreeDebug*)CQuadTree_GetInstance())->waterLevel;
            Matrix_TransformPoint(&cornerView,&cornerWorld,((SceneCameraMatrix*)camera)->worldToView);
            WView_ProjectViewPoint(camera,&s_projectedEffectVertex0[cornerIndex],&cornerView,0);
            s_projectedEffectVertex0[cornerIndex].color=0x808080ff;
        }
        drawScenePrimitive(camera,s_sceneProjectedEffectVertexPointers,0);
    }
}

// Draw two camera-facing diagonals at every world-space drop debug point.
static inline void RenderCourseDropPointCrosses(
    void* self, void* camera, Link*& renderLink, Vec3f& positiveDiagonal, Vec3f& positiveOffset,
    Vec3f& positiveLineEnd, Vec3f& positiveDiagonalCopy, Vec3f& positiveOffsetCopy, Vec3f& positiveLineStart,
    Vec3f& negativeDiagonal, Vec3f& negativeOffset, Vec3f& negativeLineEnd, Vec3f& negativeDiagonalCopy,
    Vec3f& negativeOffsetCopy, Vec3f& negativeLineStart) {
    float scale=kSceneDebugPointCrossScale;
    for (renderLink=(*(Link**)&AsCSceneManager(self)->firstDebugPoint);renderLink!=(Link*)(&AsCSceneManager(self)->lastDebugPoint);renderLink=renderLink->next) {

        __pl__FRC7WVectorRC7WVector(&positiveDiagonal,s_cameraTransform,(&((CameraTransformView*)s_cameraTransform)->up));
        __ml__FfRC7WVector(&positiveOffset,&positiveDiagonal,scale);
        __pl__FRC7WVectorRC7WVector(&positiveLineEnd,(&((SceneDebugPointLink*)renderLink)->position),&positiveOffset);
        __pl__FRC7WVectorRC7WVector(&positiveDiagonalCopy,s_cameraTransform,(&((CameraTransformView*)s_cameraTransform)->up));
        __ml__FfRC7WVector(&positiveOffsetCopy,&positiveDiagonalCopy,scale);
        __mi__FRC7WVectorRC7WVector(&positiveLineStart,(&((SceneDebugPointLink*)renderLink)->position),&positiveOffsetCopy);
        WView_DrawLine(camera,&positiveLineStart,((SceneDebugPointLink*)renderLink)->color,&positiveLineEnd,((SceneDebugPointLink*)renderLink)->color,0x300000);

        __mi__FRC7WVectorRC7WVector(&negativeDiagonal,s_cameraTransform,(&((CameraTransformView*)s_cameraTransform)->up));
        __ml__FfRC7WVector(&negativeOffset,&negativeDiagonal,scale);
        __pl__FRC7WVectorRC7WVector(&negativeLineEnd,(&((SceneDebugPointLink*)renderLink)->position),&negativeOffset);
        __mi__FRC7WVectorRC7WVector(&negativeDiagonalCopy,s_cameraTransform,(&((CameraTransformView*)s_cameraTransform)->up));
        __ml__FfRC7WVector(&negativeOffsetCopy,&negativeDiagonalCopy,scale);
        __mi__FRC7WVectorRC7WVector(&negativeLineStart,(&((SceneDebugPointLink*)renderLink)->position),&negativeOffsetCopy);
        WView_DrawLine(camera,&negativeLineStart,((SceneDebugPointLink*)renderLink)->color,&negativeLineEnd,((SceneDebugPointLink*)renderLink)->color,0x300000);
    }
}

// Project puppet bounds and center each debug label using its font width in viewport units.
static inline void RenderCourseObjectNames(
    void* self, void* camera, Link*& renderLink, void*& drawable, float* projectedPosition, Vec3f& boundsSum,
    Vec3f& boundsMidpoint, Vec3f& labelAnchorWorld, Vec3f& cameraToLabel, Vec3f& labelAnchorView) {
    if(AsCSceneManager(self)->objectNamesVisible) {
        for (renderLink=(*(Link**)&AsCSceneManager(self)->firstRenderRecord);renderLink!=(Link*)(&AsCSceneManager(self)->lastRenderRecord);renderLink=renderLink->next) {
            drawable=((SceneRenderRecord*)renderLink)->puppet;
            if(!drawable) continue;

            __pl__FRC7WVectorRC7WVector(&boundsSum,(&((ScenePuppetView*)drawable)->boundsMax),(&((ScenePuppetView*)drawable)->boundsMin));
            __ml__FfRC7WVector(&boundsMidpoint,&boundsSum,kSceneHalfScale);
            __as__7WVectorFRC7WVector(&labelAnchorWorld,&boundsMidpoint);
            labelAnchorWorld.y=((ScenePuppetView*)((SceneRenderRecord*)renderLink)->puppet)->boundsMax.y;
            __mi__FRC7WVectorRC7WVector(&cameraToLabel,&labelAnchorWorld,(&((CameraTransformView*)s_cameraTransform)->position));
            if (Magnitude__7WVectorCFv(&cameraToLabel)>kFloat_192_SceneDebug && WView_DefaultCapabilityTrueC(camera,(&((ScenePuppetView*)((SceneRenderRecord*)renderLink)->puppet)->boundsMin))) continue;
            Matrix_TransformPoint(&labelAnchorView,&labelAnchorWorld,((SceneCameraMatrix*)camera)->worldToView);
            WView_ProjectViewPoint(camera,projectedPosition,&labelAnchorView,0);
            drawable=(*(void**)&AsCSceneManager(self)->debugNameFont);
            float labelWidthPixels=((VFont*)drawable)->width(camera,((SceneRenderRecord*)renderLink)->displayName);
            WFont_Print((*(void**)&AsCSceneManager(self)->debugNameFont),camera,((SceneRenderRecord*)renderLink)->displayName,0,-1,projectedPosition[0]-kSceneHalfScale*labelWidthPixels,projectedPosition[1]);
        }
        drawable=(*(void**)&AsCSceneManager(self)->debugNameFont);
        ((VFont*)drawable)->finish(g_view);
    }
}

// Sort queued puppets by camera-forward projection and draw them after deferred overlays.
static inline void RenderCourseTransparentPuppets(void* self, Link*& renderLink, Vec3f& cameraForwardSortKey) {
    // Sort transparency by world position dot camera-forward (descending), not radial distance.
    if(AsCSceneManager(self)->transparentObjectCount && !AsCSceneManager(self)->alternateRenderSelection) {
        *(Arr3*)&cameraForwardSortKey=*(Arr3*)&((CameraTransformView*)s_cameraTransform)->forward;
        ScenePuppetList_SortByCameraProjectionDescending((SList*)(&AsCSceneManager(self)->transparentObjectCount),(SortKey*)&cameraForwardSortKey);
        for (renderLink=(*(Link**)&AsCSceneManager(self)->firstTransparentObject);renderLink!=(Link*)(&AsCSceneManager(self)->lastTransparentObject);renderLink=renderLink->next) {
            drawScenePuppet(((TransparentSceneLink*)renderLink)->puppet,((TransparentSceneLink*)renderLink)->drawScale);
            flushSceneViewQueue(g_view,2);
        }
    }
}

#pragma pop

// Borrow the selected hole's world-space cup position; hole index must already be valid.
static inline Vec3f& CurrentSceneCupWorld() {
    return ((SceneHoleRecord*)AsCGolfDoc(currentGolfDocument())->holeRecords +
        AsCGolfDoc(currentGolfDocument())->currentHoleIndex)->cup;
}

// Render the course scene: visibility/distance fading, opaque objects, shadows/effects, depth-sorted transparency and debug overlays.
// self/camera are borrowed. Transient nodes own only copied pointers and draw parameters;
// scene ownership pools keep the objects alive through all passes. Distances/limits are SQUARED
// world-space distances; opacity is 0..255. Camera basis/positions are world-space; debug geometry
// passes through worldToView then projection before immediate drawing. Queue
// flush flags 0/1/2/4 are mapped at flushSceneViewQueue above and in wgeneric3d_render_queue_and_soft_keyboard.cpp;
// these are renderer replay policies, not camera transforms or opaque bundles.
// Branch policy after visibility/kind and maximum-distance rejection:
// custom drawable -> slot8 hook, no puppet opacity/culling path
// ordinary puppet -> inherited/forced/computed opacity, recursively persisted in puppet materials
// alpha<255       -> append borrowed puppet+parameters to transparent queue
// normal selection-> draw opaque non-boundaries now; alternate -> draw all except kind 2 now
// shadow boundary -> projected-shadow/packet draw after its record, if shadow manager exists
// Flush policy: flags 0 for surfaces; flags 1 overlays; flags 0 + effectRenderPassActive=1
// for deferred draws; flags 4 + flag=0 before transparency; g_view flags 2 after each transparent
// puppet. Finally forward renderer parameter word 0. Opacity mutations intentionally persist;
// the function does not save/restore every render-state bundle to its incoming values.
void CSceneManager_RenderCourseScene(void* self,void* camera) {
    // Scratch remains in these caller scopes so inlined passes preserve the Wii frame layout.
    // The render pass helpers borrow references and never retain them.
    Link* renderLink;
    if(!AsCSceneManager(self)->renderEnabled) return;
    // Phase 1: prepare target/counters, capture cup and shadow textures, clear last frame's queues.
    int enabled=0;
    if(AsCSceneManager(self)->sceneViewOptionEnabled && ((unsigned)AsCGolfDoc(currentGolfDocument())->gameStateFlags)!=kSceneStateHoleOverview) enabled=1;
    WView_ForwardRendererParameters(g_view,enabled);
    g_sceneRenderedGeometryCount=0;
    g_sceneRenderedPuppetCount=0;
    g_sceneCourseSurfaceCount=AsCourseData801C(AsCGolfDoc(currentGolfDocument())->courseData)->surfaceCount;
    int rendererSubmissionState;
    struct Basis { float matrix[12]; int gap; int tail; char reserved[0x28]; } basis; memset(basis.matrix,0,0x30);
    basis.tail=0;
    basis.matrix[8]=kFloat_1_805DF9F8;
    basis.matrix[4]=kFloat_1_805DF9F8;
    basis.matrix[0]=kFloat_1_805DF9F8;
    rendererSubmissionState=0;
    if(g_golfCupPreview) CupPreview_CaptureModel(g_golfCupPreview,camera);
    if(g_shadowManager) ShadowManager_RenderShadowTextures(g_shadowManager,camera);
    clearTransientRenderList((Link*)&AsCSceneManager(self)->lastTransparentObject,*(unsigned*)&AsCSceneManager(self)->transparentObjectCount);
    clearTransientRenderList((Link*)&AsCSceneManager(self)->lastDeferredDrawable,*(unsigned*)&AsCSceneManager(self)->deferredDrawableCount);
    void* service=*g_resourceManager;
    // Query the renderer's cached submission-state word (+0x4844), NOT
    // texture readiness: CProjectG_CreateWindowProvider (project_g_app_and_devices.cpp) publishes
    // this service at *g_resourceManager. WGeneric3DR_DispatchCommand (wgeneric3dr_immediate_renderer.cpp),
    // command 26, synchronously writes that word through argument 2; argument 3
    // is unused and the null return is not failure. WGeneric3D_AllocateVertexBuffer
    // (wgeneric3d_render_queue_and_soft_keyboard.cpp) sets it to 0=idle; its SubmitPrimitive/QueueTaggedPrimitive/Flush
    // suppress work at 1=pending. WGeneric3DR_ActivateRenderSubmission (wgeneric3dr_renderer_core.cpp)
    // promotes 1->2=active then invokes the reset hook; CompleteRenderSubmission
    // invokes the flush hook then changes 2->0, leaving 1 unchanged. That source's
    // state views alias this same +0x4844 (WGeneric3D.h calls it submissionState).
    // This caller's ==2 active branch merely walks to the record sentinel: no draws,
    // writes or callbacks, and it does not gate the later render passes. Do not
    // reinterpret 2 as "loading" or replace these distinct equality tests.
    ((VService*)service)->query(kSceneResourceQuery1A,&rendererSubmissionState,0);
    if (rendererSubmissionState==kSceneResourceMode2) {
        Link* renderLink=(*(Link**)&AsCSceneManager(self)->firstRenderRecord);
        while (renderLink!=(Link*)(&AsCSceneManager(self)->lastRenderRecord)) renderLink=renderLink->next;
    }
    // Keep these four by-value sort calls here: extracting them reverses their volatile copy slots.
    // Phase 2: update persistent record order when kind/camera state requires it.
    if(((SceneSortState*)self)->kindSortPending) {
        sortSceneRecordsByKind(self);
        ((SceneSortState*)self)->kindSortPending=0;
    }
    unsigned golfStateFlags=((unsigned)AsCGolfDoc(currentGolfDocument())->gameStateFlags);
    if(golfStateFlags==kSceneStatePlayerPreview) {
        if(AsCSceneManager(self)->previousCameraMode) {
            AsCSceneManager(self)->previousCameraMode=0;
            sortSceneRecordsByDistance(self,*GolfBall__Fv());
            sortSceneRecordsByKind(self);
        }
    } else if(golfStateFlags==kSceneStateAim) {
        if(AsCGolfDoc(currentGolfDocument())->cameraMode) {
            sortSceneRecordsByDistance(self,((CameraTransformView*)s_cameraTransform)->position);
            sortSceneRecordsByKind(self);
        } else if(AsCSceneManager(self)->previousCameraMode) {
            sortSceneRecordsByDistance(self,*GolfBall__Fv());
            sortSceneRecordsByKind(self);
        }
        AsCSceneManager(self)->previousCameraMode=AsCGolfDoc(currentGolfDocument())->cameraMode;
    } else {
        AsCSceneManager(self)->previousCameraMode=-1;
        golfStateFlags=((unsigned)AsCGolfDoc(currentGolfDocument())->gameStateFlags);
        if((golfStateFlags&kCameraPositionSortStates) || (golfStateFlags&kAdditionalCameraPositionSortStates)) {
            sortSceneRecordsByDistance(self,((CameraTransformView*)s_cameraTransform)->position);
            sortSceneRecordsByKind(self);
        }
    }
    void* drawable=AsCGolfDoc(currentGolfDocument())->courseEffectBox;
    if(drawable) CPvsFxBox_DrawEnabledEffects(drawable,camera);
    if(g_golfCupPreview) CupPreview_DrawModel(g_golfCupPreview,camera);
    Camera_SetClip(camera,kSceneClipNear,kSceneClipFar,0);
    P10 presetB;
    P12 presetA;
    P30 preset=kSceneRenderStatePreset;
    presetA=kSceneTextureStageStatePreset;
    presetB=kSceneTextureStageParameterPreset;
    float projectedPosition[14];
    Engine_SetRenderStateList(&preset,kSceneRenderStatePairCount);
    Engine_SetTextureStageStateList(0,&presetA,kSceneTextureStageStatePairCount);
    Engine_SetTextureStageParameterList(0,&presetB,kSceneTextureStageParameterPairCount);
    // Phase 3: visit visible records; dispatch custom draws, cull puppets, compute alpha,
    // collect transparency, and draw ordinary opaque puppets with the shadow boundary hook.
    Vec3f cameraToObject;
    for (renderLink=(*(Link**)&AsCSceneManager(self)->firstRenderRecord);renderLink!=(Link*)&AsCSceneManager(self)->lastRenderRecord;renderLink=renderLink->next) {
        // 3a. Visibility/kind gate: deferred records copy only kind+drawable into
        // a transient node; custom drawable hooks bypass puppet culling/fades.
        if(!((SceneRenderRecord*)renderLink)->visible) continue;
        int objectKind=((SceneRenderRecord*)renderLink)->kind;
        if(objectKind==kRecordSkipped) continue;
        if(objectKind==kRecordDeferredDrawable) {
            volatile int data[2];
            data[0]=objectKind;
            data[1]=(*(int*)&((SceneRenderRecord*)renderLink)->customDrawable);
            NodeGuard_u801E8048 guard;
            Link* end=(Link*)(&AsCSceneManager(self)->lastDeferredDrawable);
            Link* mem=(Link*)appMallocDummy__FUl(0x10);
            guard.init(mem,end);
            if(&((SceneDrawableLink*)mem)->kind) {
                try {
                    ((SceneDrawableLink*)mem)->kind=data[0];
                    ((SceneDrawableLink*)mem)->drawable=(void*)data[1];
                } catch(...) {
                    appFreeUni__FPv(mem);
                    throw;
                }
            }
            append(end,mem);
            ++(*(unsigned*)&AsCSceneManager(self)->deferredDrawableCount);
            guard.n=0;
            continue;
        }
        drawable=((SceneRenderRecord*)renderLink)->customDrawable;
        if(drawable) dispatchSceneObjectHook(drawable,8);
        else {
            drawable=((SceneRenderRecord*)renderLink)->puppet;
            if(!drawable) continue;
            // 3b. Reject beyond maximum squared distance before updating opacity
            // or queuing transparency. A nonpositive maximum disables this test.
            if (!(((unsigned)AsCGolfDoc(currentGolfDocument())->gameStateFlags)&kSceneStateStorySequence) && ((SceneRenderRecord*)renderLink)->maximumDistanceSquared>kFloat_0_805DF9FC) {
                Vec3f v;
                __mi__FRC7WVectorRC7WVector(&v,(&((CameraTransformView*)s_cameraTransform)->position),(&((ScenePuppetView*)drawable)->boundingSphere.v));
                __as__7WVectorFRC7WVector(&cameraToObject,&v);
                float d=Vec3_CalculateLengthSquared(&cameraToObject);
                if (((SceneRenderRecord*)renderLink)->maximumDistanceSquared<=d) continue;
            }
            objectKind=((SceneRenderRecord*)renderLink)->kind;
            if (objectKind==kRecordShadowBoundary || (unsigned)(objectKind-kRecordAlternateExcluded)<=1) {
                ++g_sceneRenderedPuppetCount;
                g_sceneRenderedGeometryCount+=((ScenePuppetView*)((SceneRenderRecord*)renderLink)->puppet)->geometryCount;
            }
            {
                Vec3f cameraDelta;
                Sphere boundsSphere;
                UpdateCourseRecordOpacity(self, renderLink, drawable, cameraToObject, cameraDelta, boundsSphere);
            }
            if(((SceneRenderRecord*)renderLink)->opacity<kOpaque) {
                // 3d. Retain only a borrowed puppet pointer plus copied parameters
                // in this frame's transparent queue; scene pools own the puppet.
                SceneTransparentPayload data;
                data.drawScale=((SceneRenderRecord*)renderLink)->drawScale;
                data.puppet=((SceneRenderRecord*)renderLink)->puppet;
                data.renderParameter=((SceneRenderRecord*)renderLink)->renderParameter;
                data.reserved0=0;
                data.reserved1=0;
                NodeGuard_u801E8048 guard;
                AppendCourseTransparentPuppet(self, data, guard);
            }
            RenderCourseRecordPuppet(self, renderLink, drawable);
        }
        RenderCourseShadowBoundary(camera, renderLink);
    }
    // Phase 4: marker/weather and owned course surfaces (beach, river, boundaries, texture strips).
    RenderCourseMarkerAndWeather(self, camera);
    {
        Vec3f verticalOffset;
        Vec3f selectedPointForOffset,selectedPointForLine,lineEndWorld;
        RenderCourseQuadTreeDebug(verticalOffset, selectedPointForOffset,selectedPointForLine,lineEndWorld);
    }
    if((*(void**)&AsCSceneManager(self)->generatedSurface)) TextureMesh_DrawLayers((*(void**)&AsCSceneManager(self)->generatedSurface));
    void* object;
    // Keep the cursor guards in these scopes to preserve exception-cleanup frame slots.
    // Four pooled traversals, in render order: beach (reverse), river/lava,
    // boundary grid, generated texture (forward). Each cursor read is locked;
    // drawing happens outside that lock. Offsets are owner-relative pool fields.
    {
        WInstanceLock guard((&((CleanupClass*)self)->lists[kSceneBeachPool].lock));
        drawable=PoolActiveRing(&((CleanupClass*)self)->lists[kSceneBeachPool]);
        (*(void**)&(&((CleanupClass*)self)->lists[kSceneBeachPool])->iterationCursor)=drawable?((Fn801EBNode*)drawable)->prev:0;
        object=nextSceneListObject(&((CleanupClass*)self)->lists[kSceneBeachPool],true); }
    while(object) { dispatchSceneObjectHook(object,0x10); object=nextSceneListObject(&((CleanupClass*)self)->lists[kSceneBeachPool],true); }
    {
        WInstanceLock guard((&((CleanupClass*)self)->lists[kSceneRiverPool].lock));
        (*(void**)&(&((CleanupClass*)self)->lists[kSceneRiverPool])->iterationCursor)=(*(void**)&(&((CleanupClass*)self)->lists[kSceneRiverPool])->activeRing);
        object=nextSceneListObject(&((CleanupClass*)self)->lists[kSceneRiverPool],false);
    }
    while(object) { dispatchSceneObjectHook(object,0x14); object=nextSceneListObject(&((CleanupClass*)self)->lists[kSceneRiverPool],false); }
    {
        WInstanceLock guard((&((CleanupClass*)self)->lists[kSceneBoundaryPool].lock));
        (*(void**)&(&((CleanupClass*)self)->lists[kSceneBoundaryPool])->iterationCursor)=(*(void**)&(&((CleanupClass*)self)->lists[kSceneBoundaryPool])->activeRing);
        object=nextSceneListObject(&((CleanupClass*)self)->lists[kSceneBoundaryPool],false);
    }
    while(object) { GroundGrid_DrawLitSurface(object); object=nextSceneListObject(&((CleanupClass*)self)->lists[kSceneBoundaryPool],false); }
    {
        WInstanceLock guard((&((CleanupClass*)self)->lists[kSceneTexturePool].lock));
        (*(void**)&(&((CleanupClass*)self)->lists[kSceneTexturePool])->iterationCursor)=(*(void**)&(&((CleanupClass*)self)->lists[kSceneTexturePool])->activeRing);
        object=nextSceneListObject(&((CleanupClass*)self)->lists[kSceneTexturePool],false);
    }
    while(object) { dispatchSceneObjectHook(object,0x10); object=nextSceneListObject(&((CleanupClass*)self)->lists[kSceneTexturePool],false); }
    // Phase 5: overlay and deferred custom draws precede the transparent puppet pass.
    RenderCourseOverlaysAndDeferred(self, camera, renderLink, drawable);
    {
        Vec3f cameraForwardSortKey;
        RenderCourseTransparentPuppets(self, renderLink, cameraForwardSortKey);
    }
    // Phase 6: debug crosses/drop disk, water planes, and centered object-name labels.
    // The disk remains here because moving its nested vector/vertex scratch changes the Wii frame.
    // Drop disk points live in world XZ at cup Y; each is transformed to view then projected.
    if(((unsigned)AsCGolfDoc(currentGolfDocument())->gameStateFlags)&kSceneStateCameraTransition) {
        {
            Vec3f positiveDiagonal,positiveOffset,positiveLineEnd,positiveDiagonalCopy,positiveOffsetCopy,positiveLineStart;
            Vec3f negativeDiagonal,negativeOffset,negativeLineEnd,negativeDiagonalCopy,negativeOffsetCopy,negativeLineStart;
            RenderCourseDropPointCrosses(
                self, camera, renderLink,
                positiveDiagonal,positiveOffset,positiveLineEnd,positiveDiagonalCopy,positiveOffsetCopy,positiveLineStart,
                negativeDiagonal,negativeOffset,negativeLineEnd,negativeDiagonalCopy,negativeOffsetCopy,negativeLineStart);
        }
        if(AsCGolfDoc(currentGolfDocument())->dropRequest) {
            // 6a. Radius is the ball-to-cup XZ distance minus the existing inset.
            drawable=((SceneHoleRecord*)AsCGolfDoc(currentGolfDocument())->holeRecords+AsCGolfDoc(currentGolfDocument())->currentHoleIndex);
            Vec3f cupCenterWorld=((SceneHoleRecord*)drawable)->cup,ballFromCupXZ;
            __mi__FRC7WVectorRC7WVector(&ballFromCupXZ,GolfBall__Fv(),&cupCenterWorld);
            ballFromCupXZ.y=kFloat_0_805DF9FC;
            float radius=Magnitude__7WVectorCFv(&ballFromCupXZ)-kSceneDropCircleRadiusInset;
            // 6b. Project the shared triangle center and set translucent fill / black rim colors.
            Vertex fillVertices[3]; void* fillVertexPointers[4]={&fillVertices[0],&fillVertices[1],&fillVertices[2],0};
            fillVertices[0].color=fillVertices[1].color=fillVertices[2].color=kSceneDebugDiskFillColor;
            Vec3f cupCenterForProjection=CurrentSceneCupWorld(),cupCenterView;
            Matrix_TransformPoint(&cupCenterView,&cupCenterForProjection,((SceneCameraMatrix*)camera)->worldToView);
            WView_ProjectViewPoint(camera,&fillVertices[2],&cupCenterView,0);
            s_projectedEffectVertex0[0].color=kSceneDebugDiskRimColor;
            s_projectedEffectVertex0[1].color=kSceneDebugDiskRimColor;
            s_projectedEffectVertex0[2].color=kSceneDebugDiskRimColor;
            s_projectedEffectVertex0[3].color=kSceneDebugDiskRimColor;
            // 6c. Each segment draws a filled triangle, then a one-world-unit-wide rim quad.
            for(int segment=0;segment<kSceneDebugCircleSegments;++segment) {
                for(int vertexIndex=0;vertexIndex<2;++vertexIndex) {
                    int angleIndex=(segment+(vertexIndex&1))%kSceneDebugCircleSegments;
                    float sinAngle=Math_SinApprox((kSceneCircleFullTurnRadians*(float)angleIndex)*kSceneCircleSegmentReciprocal128);
                    Vec3f direction;
                    direction.x=Math_CosApprox((kSceneCircleFullTurnRadians*(float)angleIndex)*kSceneCircleSegmentReciprocal128);
                    direction.y=kFloat_0_805DF9FC;
                    direction.z=sinAngle;
                    Vec3f segmentCupWorld=CurrentSceneCupWorld(),radialOffsetWorld,segmentPointWorld,segmentPointView;
                    __ml__FfRC7WVector(&radialOffsetWorld,&direction,radius);
                    __pl__FRC7WVectorRC7WVector(&segmentPointWorld,&segmentCupWorld,&radialOffsetWorld);
                    Matrix_TransformPoint(&segmentPointView,&segmentPointWorld,((SceneCameraMatrix*)camera)->worldToView);
                    WView_ProjectViewPoint(camera,&fillVertices[vertexIndex],&segmentPointView,0);
                }
                drawScenePrimitive(camera,fillVertexPointers,kSceneDebugProjectedDrawFlags);
                // Vertex bit 0 chooses segment endpoint; bit 1 chooses inner/outer radius.
                for(int vertexIndex=0;vertexIndex<4;++vertexIndex) {
                    int angleIndex=(segment+(vertexIndex&1))%kSceneDebugCircleSegments;
                    float sinAngle=Math_SinApprox((kSceneCircleFullTurnRadians*(float)angleIndex)*kSceneCircleSegmentReciprocal128);
                    Vec3f direction;
                    direction.x=Math_CosApprox((kSceneCircleFullTurnRadians*(float)angleIndex)*kSceneCircleSegmentReciprocal128);
                    direction.y=kFloat_0_805DF9FC;
                    direction.z=sinAngle;
                    float rimRadius=(vertexIndex&2)?kFloat_1_805DF9F8+radius:radius;
                    Vec3f segmentCupWorld=CurrentSceneCupWorld(),radialOffsetWorld,segmentPointWorld,segmentPointView;
                    __ml__FfRC7WVector(&radialOffsetWorld,&direction,rimRadius);
                    __pl__FRC7WVectorRC7WVector(&segmentPointWorld,&segmentCupWorld,&radialOffsetWorld);
                    Matrix_TransformPoint(&segmentPointView,&segmentPointWorld,((SceneCameraMatrix*)camera)->worldToView);
                    WView_ProjectViewPoint(camera,&s_projectedEffectVertex0[vertexIndex],&segmentPointView,0);
                }
                drawScenePrimitive(camera,s_sceneProjectedEffectVertexPointers,kSceneDebugProjectedDrawFlags);
            }
        }
    }
    {
        int cornerIndex;
        Vec3f cornerWorld,cornerView;
        RenderCourseDebugWaterPlane(self, camera, false, cornerIndex, cornerWorld,cornerView);
    }
    {
        int cornerIndex;
        Vec3f cornerWorld,cornerView;
        RenderCourseDebugWaterPlane(self, camera, true, cornerIndex, cornerWorld,cornerView);
    }
    {
        Vec3f boundsSum,boundsMidpoint,labelAnchorWorld,cameraToLabel,labelAnchorView;
        RenderCourseObjectNames(
            self, camera, renderLink, drawable, projectedPosition,
            boundsSum,boundsMidpoint,labelAnchorWorld,cameraToLabel,labelAnchorView);
    }
    // Phase 7: forward renderer parameter word 0 after all scene/debug passes.
    WView_ForwardRendererParameters(g_view,0);
}

#pragma opt_common_subs reset
// Sort render records by decreasing squared distance from the supplied world position, putting null puppets first.
void SceneRenderList_SortByDistanceDescending(SList* self, SortKey* key) {
    // Base cases: 0/1 nodes need no work; reorder a two-node ring directly.
    switch (self->count) {
    case 0:
    case 1:
        break;
    case 2: {
        SNode* node = self->next;
        SNode* other = node->next;
        if (SNode_swapNeeded(node, other, key)) {
            SNode* t = node->prev;
            node->prev = node->next;
            node->next = t;
            t = other->prev;
            other->prev = other->next;
            other->next = t;
            t = self->prev;
            self->prev = self->next;
            self->next = t;
        }
        break;
    }
    default: {
        unsigned half = (unsigned)self->count >> 1;
        unsigned n = half;
        SNode* node = self->next;
        while (n != 0) {
            node = node->next;
            --n;
        }

        // Split the second half into a temporary sentinel ring before recursive sorting.
        SList temporary;
        SNode* sentinel = (SNode*)&temporary.prev;
        int rest = self->count - half;
        if (rest != 0) {
            if (&temporary != self) {
                self->count -= rest;
                temporary.count += rest;
            }
            SNode* last = self->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            temporary.prev->next = node;
            node->prev = temporary.prev;
            temporary.prev = last;
            last->next = sentinel;
        }
        // Recurse on disjoint rings, then splice-merge without copying payloads.
        SortKey left = *key;
        SceneRenderList_SortByDistanceDescending(self, &left);
        SortKey right = *key;
        SceneRenderList_SortByDistanceDescending(&temporary, &right);
        SortKey mergeKey = *key;
        SceneRenderList_MergeByDistanceDescending((List*)self, (List*)&temporary, (char*)&mergeKey);
    }
    }
}

#pragma use_lmw_stmw on
// Merge two render-record lists using squared distance from the supplied world position.
void SceneRenderList_MergeByDistanceDescending(List* destinationList, List* sourceList, char* sortKey) {
    if (destinationList == sourceList)
        return;
    ListNode* runEnd;
    ListNode* destination = destinationList->next;
    ListNode* const destinationEnd = (ListNode*)&destinationList->prev;
    ListNode* sourceCursor = sourceList->next;
    ListNode* const sourceEnd = (ListNode*)&sourceList->prev;
    // Splice ordered runs from source into destination and maintain both live node counts.
    while (destination != destinationEnd && sourceCursor != sourceEnd) {
        if (SNode_swapNeeded((SNode*)destination, (SNode*)sourceCursor, (SortKey*)sortKey)) {
            int runLength = 1;
            for (runEnd = sourceCursor->next; runEnd != sourceEnd; runEnd = runEnd->next) {
                if (!SNode_swapNeeded((SNode*)destination, (SNode*)runEnd, (SortKey*)sortKey))
                    break;
                ++runLength;
            }
            ListNode* runLast = runEnd->prev;
            ListNode* runFirst = sourceCursor;
            SpliceSceneMergeRun(destination, runFirst, runLast, sourceCursor, runEnd);
            sourceList->count -= runLength;
            destinationList->count += runLength;
        }
        destination = destination->next;
    }
    // Append the remaining source suffix and restore source to an empty sentinel ring.
    if (sourceCursor != sourceEnd && sourceList->count != 0) {
        ListNode* runLast;
        ListNode* runFirst = sourceList->next;
        runLast = sourceList->prev;
        AppendSceneMergeRemainder(destinationList, sourceList, destination, runFirst, runLast);
    }
}
#pragma use_lmw_stmw reset

// Initialize the scene angle conversion constant at startup.
void SceneRenderList_InitAngleConstant() {
    s_sceneRenderListAngleConstant = kFloat_6_2831855;
}

// Stable-sort list payloads by increasing signed integer value; the key parameter is unused.
void SceneRenderList_SortBySignedPayload(SList* self, char* key) {
    // Base cases: 0/1 nodes need no work; reorder a two-node ring directly.
    switch (self->count) {
    case 0:
    case 1:
        break;
    case 2: {
        SNode* n = self->next;
        SNode* nx = n->next;
        if ((int)nx->val < (int)n->val) {
            SNode* p = n->prev;
            n->prev = nx;
            n->next = p;
            SNode* q = nx->prev;
            nx->prev = nx->next;
            nx->next = q;
            SNode* t = self->prev;
            self->prev = self->next;
            self->next = t;
        }
        break;
    }
    default: {
        unsigned half = (unsigned)self->count >> 1;
        unsigned n = half;
        SNode* node = self->next;
        while (n != 0) {
            node = node->next;
            --n;
        }

        // Split the second half into a temporary sentinel ring before recursive sorting.
        SList temporary;
        int rest = self->count - half;
        SNode* sentinel = (SNode*)&temporary.prev;
        if (rest != 0) {
            if (&temporary != self) {
                self->count -= rest;
                temporary.count += rest;
            }
            SNode* last = self->prev;
            node->prev->next = last->next;
            last->next->prev = node->prev;
            temporary.prev->next = node;
            node->prev = temporary.prev;
            temporary.prev = last;
            last->next = sentinel;
        }
        char copy[4];
        copy[0] = *key;
        SceneRenderList_SortBySignedPayload(self, copy);
        char copy2[4];
        copy2[0] = *key;
        SceneRenderList_SortBySignedPayload(&temporary, copy2);
        char copy3[4];
        copy3[0] = *key;
        SceneRenderList_MergeBySignedPayload((List*)self, (List*)&temporary, copy3);
    }
    }
}

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_common_subs off
// Merge two payload lists in increasing signed integer order; the key parameter is unused.
void SceneRenderList_MergeBySignedPayload(List* destinationList, List* sourceList, char* sortKey) {
    if (destinationList == sourceList)
        return;
    ListNode* runEnd;
    ListNode* destinationEnd;
    ListNode* destination = destinationList->next;
    destinationEnd = (ListNode*)&destinationList->prev;
    ListNode* sourceCursor = sourceList->next;
    ListNode* const sourceEnd = (ListNode*)&sourceList->prev;
    // Splice ordered runs from source into destination and maintain both live node counts.
    while (destination != destinationEnd && sourceCursor != sourceEnd) {
        int destinationValue = (int)destination->value;
        if ((int)sourceCursor->value < destinationValue) {
            int runLength = 1;
            for (runEnd = sourceCursor->next; runEnd != sourceEnd; runEnd = runEnd->next) {
                if ((int)runEnd->value >= destinationValue)
                    break;
                ++runLength;
            }
            ListNode* runLast = runEnd->prev;
            SpliceSceneMergeRun(destination, sourceCursor, runLast, sourceCursor, runEnd);
            sourceList->count -= runLength;
            destinationList->count += runLength;
        }
        destination = destination->next;
    }
    // Append the remaining source suffix and restore source to an empty sentinel ring.
    if (sourceCursor != sourceEnd && sourceList->count != 0) {
        ListNode* runLast;
        ListNode* runFirst = sourceList->next;
        runLast = sourceList->prev;
        AppendSceneMergeRemainder(destinationList, sourceList, destination, runFirst, runLast);
    }
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_propagation off
// Stable-sort transparent puppets by decreasing projection onto the supplied camera direction.
void ScenePuppetList_SortByCameraProjectionDescending(SList* self, SortKey* key) {
    SNode* sent;
    int half;
    SNode* node;
    unsigned n;
    // Base cases: 0/1 nodes need no work; reorder a two-node ring directly.
    switch (self->count) {
    case 0:
    case 1:
        break;
    case 2: {
        SNode* a = self->next;
        SNode* b = a->next;
        if (SList_less((char*)&((ScenePuppetView*)a->val)->boundingSphere.v, (char*)&((ScenePuppetView*)b->val)->boundingSphere.v, key)) {
            SNode* t = a->prev;
            a->prev = a->next;
            a->next = t;
            t = b->prev;
            b->prev = b->next;
            b->next = t;
            t = self->prev;
            self->prev = self->next;
            self->next = t;
        }
        break;
    }
    default: {
    half = (unsigned)self->count >> 1;
    node = self->next;
    n = half;
    while (n != 0) {
        node = node->next;
        --n;
    }

    SList temporary;
    half = self->count - half;
    sent = (SNode*)&temporary.prev;
    if (half != 0) {
        if (&temporary != self) {
            self->count -= half;
            temporary.count += half;
        }
        SNode* last = self->prev;
        node->prev->next = last->next;
        last->next->prev = node->prev;
        temporary.prev->next = node;
        node->prev = temporary.prev;
        temporary.prev = last;
        last->next = sent;
    }
    SortKey left = *key;
    ScenePuppetList_SortByCameraProjectionDescending(self, &left);
    SortKey right = *key;
    ScenePuppetList_SortByCameraProjectionDescending(&temporary, &right);
    SortKey mergeKey = *key;
    ScenePuppetList_MergeByCameraProjectionDescending((List*)self, (List*)&temporary, &mergeKey);
    }
    }
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Merge transparent puppet lists in decreasing camera-direction projection order.
void ScenePuppetList_MergeByCameraProjectionDescending(List* destinationList, List* sourceList, void* sortKey) {
    if (destinationList == sourceList)
        return;
    ListNode* runEnd;
    ListNode* destination;
    ListNode* destinationEnd;
    destination = destinationList->next;
    destinationEnd = (ListNode*)&destinationList->prev;
    ListNode* sourceCursor = sourceList->next;
    ListNode* const sourceEnd = (ListNode*)&sourceList->prev;
    // Splice ordered runs from source into destination and maintain both live node counts.
    while (destination != destinationEnd && sourceCursor != sourceEnd) {
        if (List_dotGreater(&((ScenePuppetView*)sourceCursor->value)->boundingSphere.v, &((ScenePuppetView*)destination->value)->boundingSphere.v, sortKey)) {
            runEnd = sourceCursor->next;
            int runLength = 1;
            while (runEnd != sourceEnd) {
                if (!List_dotGreater(&((ScenePuppetView*)runEnd->value)->boundingSphere.v, &((ScenePuppetView*)destination->value)->boundingSphere.v, sortKey))
                    break;
                ++runLength;
                runEnd = runEnd->next;
            }
            ListNode* runLast = runEnd->prev;
            ListNode* runFirst = sourceCursor;
            SpliceSceneMergeRun(destination, runFirst, runLast, sourceCursor, runEnd);
            sourceList->count -= runLength;
            destinationList->count += runLength;
        }
        destination = destination->next;
    }
    // Append the remaining source suffix and restore source to an empty sentinel ring.
    if (sourceCursor != sourceEnd && sourceList->count != 0) {
        ListNode* runLast;
        ListNode* runFirst = sourceList->next;
        runLast = sourceList->prev;
        AppendSceneMergeRemainder(destinationList, sourceList, destination, runFirst, runLast);
    }
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_propagation off
// Release owned scene objects under each list mutex, empty the lookup arrays and release marker resources.
void CSceneManager_ClearOwnedSceneObjects(CleanupClass* arg) {
    // Destroy the singleton generated surface, then each pool's payloads. For each pool:
    // fetch/advance under lock -> destroy unlocked -> recycle nodes and zero lookup under lock.
    // Pool slabs remain allocated for reuse; the final marker call releases render resources only.
    CleanupClass* self;
    if (arg->owned) {
        GeneratedSurface_Destroy(arg->owned, 1);
        arg->owned = 0;
    }
    self = arg;
    {
        DList* l;
        void* v;
        char* mtx;
        {
            WInstanceLock lock(&self->lists[3].lock);
            l = &self->lists[3];
            mtx = (char*)&l->lock;
            l->iterationCursor = l->activeRing;
            v = DList_NextObject(l, mtx);
        }
        if (v) {
            mtx = (char*)&l->lock;
            while (v) {
                if (v) GroundGrid_Dtor(v, 1);
                v = DList_NextObject(l, mtx);
            }
        }
        {
            WInstanceLock lock(&l->lock);
            DList_clear(l);
            DList_zero(l);
        }
    }
    {
        DList* l;
        char* mtx;
        void* v;
        {
            WInstanceLock lock(&self->lists[0].lock);
            l = &self->lists[0];
            mtx = (char*)&l->lock;
            l->iterationCursor = l->activeRing;
            v = DList_NextObject(l, mtx);
        }
        if (v) {
            mtx = (char*)&l->lock;
            while (v) {
                if (v) DList_releaseInstance(v);
                v = DList_NextObject(l, mtx);
            }
        }
        {
            WInstanceLock lock(&l->lock);
            DList_clear2(l);
            DList_zero(l);
        }
    }
    {
        DList* l;
        char* mtx;
        void* v;
        {
            WInstanceLock lock(&self->lists[1].lock);
            l = &self->lists[1];
            mtx = (char*)&l->lock;
            l->iterationCursor = l->activeRing;
            v = DList_NextObject(l, mtx);
        }
        if (v) {
            mtx = (char*)&l->lock;
            while (v) {
                if (v) DList_releaseInstance(v);
                v = DList_NextObject(l, mtx);
            }
        }
        {
            WInstanceLock lock(&l->lock);
            DList_clear2(l);
            DList_zero(l);
        }
    }
    {
        DList* l;
        char* mtx;
        void* v;
        {
            WInstanceLock lock(&self->lists[2].lock);
            l = &self->lists[2];
            mtx = (char*)&l->lock;
            l->iterationCursor = l->activeRing;
            v = DList_NextObject(l, mtx);
        }
        if (v) {
            mtx = (char*)&l->lock;
            while (v) {
                if (v) DList_releaseInstance(v);
                v = DList_NextObject(l, mtx);
            }
        }
        {
            WInstanceLock lock(&l->lock);
            DList_clear2(l);
            DList_zero(l);
        }
    }
    if (self->last) CMarker_ReleaseRenderResources(self->last);
}
#pragma pop

// Build course geometry/effects from typed course entries, add boundary markers, and configure weather.
// Replaces the scene-owned objects; entry point/name temporaries are copied and destroyed locally.
// CourseGeometryKind maps only kinds established by constructors/calls below. Unknown kinds skip.
// Required loaded course/document/marker and serialized scene access. Destructive
// rebuild, with no rollback/status result: ClearOwnedSceneObjects runs FIRST.
// Ownership/publication/failure map (source entry vectors remain borrowed):
// - Generated polygon: new -> publish generatedSurface -> copy points -> build.
//   A later throw leaves the published partial surface owned by the scene; clear
//   releases it with GeneratedSurface_Destroy. A second polygon overwrites this
//   singleton without releasing the first, so valid input needs at most one.
// - Beach, river, lava, texture strip: new -> local raw pointer -> configure ->
//   append into beach/river/river/texture pool. Before successful append no scene
//   owner exists; configure/copy/append throws can leak the completed object.
//   Afterwards ClearOwnedSceneObjects deletes the pool payload via its virtual
//   destructor, then recycles nodes/lookup arrays (retaining slab allocations).
// - Projected quads: mutate the existing marker directly, no new owner/commit.
// - Boundary grids: new -> configure -> append boundary pool; same unadopted
//   failure window, then GroundGrid_Dtor on clear. Marker render resources are
//   rebuilt last; clear releases those resources, not the marker object itself.
// - Weather: configured only after all geometry; see ConfigureEffects below for
//   both effect slots and selectedWeather's separate publication point.
// Local point/name copies unwind automatically; the raw completed objects above
// have no guards. A throwing new-expression cleans its own failed construction,
// not earlier objects. Null allocations/setup failures are not handled here and
// may be dereferenced; successful allocations/services are a caller requirement.
// No concurrent renderer may observe the partially rebuilt scene. Preserve these
// existing failure windows in readability work; adding recovery is a separate mod.
void CGolfBg_CreateCourseObjects(CourseSceneConstructionView* self) {
	// Phase 1: clear previous ownership and iterate the course entry vector at courseData+0x14.
	CSceneManager_ClearOwnedSceneObjects((CleanupClass*)self);
	struct Iter { void* p; } iterObj;
	void* beachObject;
	VObj* riverObject;
	VObj* lavaObject;
	void* generatedTextureObject;
	void* boundaryObject;
	void* iteratorCopy;
	unsigned int firstCourseEntry;
	unsigned int endCourseEntry;
	CourseEntryIterator_DefaultConstruct(&iterObj);

	{
		void* courseEntries = GolfDoc_GetCurrent();
		courseEntries = AsCGolfDoc(courseEntries)->courseData; // CGolfDoc +0x8
		courseEntries = static_cast<char*>(courseEntries) + kCourseEntryVectorOffset;
		firstCourseEntry = CourseEntryVector_Begin(courseEntries);
		void* ret = CourseEntryIterator_CopyConstruct(&iteratorCopy, &firstCourseEntry, 0);
		CourseEntryIterator_Assign(&iterObj, ret);
	}

	for (; (endCourseEntry = CourseEntryVector_End(static_cast<char*>(AsCGolfDoc(GolfDoc_GetCurrent())->courseData) + kCourseEntryVectorOffset), CourseEntryIterator_NotEqual(&iterObj, &endCourseEntry)); CourseEntryIterator_Advance(reinterpret_cast<Fn801EB82CObject*>(&iterObj))) {

		int entryKind = CourseEntry_GetKind(CourseEntryIterator_GetEntry(&iterObj));
		// Phase 2: construct each recognized entry and adopt it into the corresponding owner/list.
		if (entryKind == kEntryGeneratedPolygon) {
			void* p = new GeneratedSurfaceAlloc;
			(*(void**)&AsCSceneManager(self)->generatedSurface) = p;
			void* polygonEntry = (void*)CourseEntryIterator_GetEntry(&iterObj);
			void* entryPoints = reinterpret_cast<void*>(CourseEntry_GetPointVector(polygonEntry));
			Dtor47CC polygonPoints;
			Vec3Vector_CopyCtor(&polygonPoints, entryPoints);
			GeneratedSurface_BuildFromPolygon((*(void**)&AsCSceneManager(self)->generatedSurface), &polygonPoints);
		} else if (CourseEntry_GetKind(CourseEntryIterator_GetEntry(&iterObj)) == kEntryBeach) {
			void* p = new BeachAlloc;
			beachObject = p;
			{
			void* beachEntry = (void*)CourseEntryIterator_GetEntry(&iterObj);
			void* entryPoints = reinterpret_cast<void*>(CourseEntry_GetPointVector(beachEntry));
			Dtor47CC beachPoints;
			Vec3Vector_CopyCtor(&beachPoints, entryPoints);
			Dtor463FC beachName;
			MWString_CopyConstructForNameLookup(&beachName, CourseEntryIterator_GetEntry(&iterObj));
			unsigned int entryNameAddress = reinterpret_cast<unsigned int>(MWString_GetCString(&beachName));
			// Final argument is BUFFER COUNT, not a mode enum. beach_wavelet_ground_strip.cpp allocates
			// one vertex buffer per layer, each 2*pointCount vertices (48 bytes
			// per path point). CBeach_UpdateWaves (ball_actor_and_ambient_sound.cpp) updates layer state;
			// CWavelet_DrawLayers (wind_indicator_wavelet_cloud_overlay.cpp) draws the corresponding buffers;
			// generated texture strips below request two. Point input needs >=2
			// points and nonzero configured width. The name address is forwarded
			// to virtual +0x14 for parameter lookup (not a numeric row count).
			// Keep buffer/layer counts equal and <=255 for byte-indexed consumers;
			// neither 2 nor 3 selects a separate geometry algorithm here.
			GroundStrip_BuildGeometry(beachObject, &beachPoints, entryNameAddress, 0x3);
			}
			SceneBeachList_AppendObject(&CourseSceneBeachPool(self), &beachObject);
		} else if (CourseEntry_GetKind(CourseEntryIterator_GetEntry(&iterObj)) == kEntryRiver) {
			riverObject = reinterpret_cast<VObj*>(new RiverAlloc);
			{
			Dtor463FC riverName;
			MWString_CopyConstructForNameLookup(&riverName, CourseEntryIterator_GetEntry(&iterObj));
			unsigned int entryNameAddress = reinterpret_cast<unsigned int>(MWString_GetCString(&riverName));
			void* riverEntry = (void*)CourseEntryIterator_GetEntry(&iterObj);
			void* entryPoints = reinterpret_cast<void*>(CourseEntry_GetPointVector(riverEntry));
			riverObject->m(entryPoints, entryNameAddress);
			}
			SceneRiverList_AppendObject(&CourseSceneRiverPool(self), reinterpret_cast<void**>(&riverObject));
		} else if (CourseEntry_GetKind(CourseEntryIterator_GetEntry(&iterObj)) == kEntryLava) {
			lavaObject = reinterpret_cast<VObj*>(new LavaRiverAlloc);
			{
			Dtor463FC lavaName;
			MWString_CopyConstructForNameLookup(&lavaName, CourseEntryIterator_GetEntry(&iterObj));
			unsigned int entryNameAddress = reinterpret_cast<unsigned int>(MWString_GetCString(&lavaName));
			void* lavaEntry = (void*)CourseEntryIterator_GetEntry(&iterObj);
			void* entryPoints = reinterpret_cast<void*>(CourseEntry_GetPointVector(lavaEntry));
			lavaObject->m(entryPoints, entryNameAddress);
			}
			SceneRiverList_AppendObject(&CourseSceneRiverPool(self), reinterpret_cast<void**>(&lavaObject));
		} else if (CourseEntry_GetKind(CourseEntryIterator_GetEntry(&iterObj)) == kEntryTextureStrip) {
			void* p = new GeneTex3DAlloc;
			generatedTextureObject = p;
			{
			void* textureStripEntry = (void*)CourseEntryIterator_GetEntry(&iterObj);
			void* entryPoints = reinterpret_cast<void*>(CourseEntry_GetPointVector(textureStripEntry));
			Dtor47CC textureStripPoints;
			Vec3Vector_CopyCtor(&textureStripPoints, entryPoints);
			Dtor463FC textureStripName;
			MWString_CopyConstructForNameLookup(&textureStripName, CourseEntryIterator_GetEntry(&iterObj));
			unsigned int entryNameAddress = reinterpret_cast<unsigned int>(MWString_GetCString(&textureStripName));
			GroundStrip_BuildGeometry(generatedTextureObject, &textureStripPoints, entryNameAddress, 0x2);
			}
			SceneGeneratedTextureList_AppendObject(&CourseSceneTexturePool(self), &generatedTextureObject);
		} else {
			if (CourseEntry_GetKind(CourseEntryIterator_GetEntry(&iterObj)) == kEntryProjectedQuads3 || CourseEntry_GetKind(CourseEntryIterator_GetEntry(&iterObj)) == kEntryProjectedQuads2) {
				void* projectedQuadEntry = (void*)CourseEntryIterator_GetEntry(&iterObj);
				void* entryPoints = reinterpret_cast<void*>(CourseEntry_GetPointVector(projectedQuadEntry));
				BuildGroundProjectedPointQuads(AsCSceneManager(self)->marker, entryPoints);
			}
		}

	}

	// Phase 3: turn kind-2 boundary records into owned ground grids, using bounds at +0x44.
	Fn801EB238Item* item;
	void* p;
	int boundaryIndex;
	for (boundaryIndex = 0; boundaryIndex < reinterpret_cast<int>(CourseData_GetBoundaryRecordCount(reinterpret_cast<Fn801EB248Object*>(AsCGolfDoc(GolfDoc_GetCurrent())->courseData))); ++boundaryIndex) {
		void* courseData = GolfDoc_GetCurrent();
		courseData = AsCGolfDoc(courseData)->courseData; // CGolfDoc +0x8
		item = Array_GetItemByIndex(reinterpret_cast<Fn801EB238Object*>(courseData), boundaryIndex);
		unsigned char boundaryKind = item->kind;
		if (boundaryKind == kBoundaryGroundGrid) {
			p = new N190;
			boundaryObject = p;
			GroundGrid_ConfigureSurfaceBounds(p, &item->minimum, item->properties);
			SceneBoundaryList_AppendObject(&CourseSceneBoundaryPool(self), &boundaryObject);
		}
	}

	// Phase 4: finalize marker resources and configure weather from the selected course variant.
	void* marker = AsCSceneManager(self)->marker;
	CMarker_CreateRenderResources(marker);
	void* variantWord = CourseResource_GetSelectedVariant();
	unsigned char weatherCode = static_cast<unsigned char>(reinterpret_cast<unsigned int>(variantWord));
	CourseWeather_ConfigureEffects(self, weatherCode, 0x1);
	DrawScene_NoOpHook(self, 0x1);
}

#pragma dont_inline on
// Initialize the generated-texture base and install the beach vtable.
Fn801EB1FCObject* CBeach_Ctor(Fn801EB1FCObject* self) {
	GroundStripOwner_Init(self);
	self->vtable = __vt__6CBeach;
	return self;
}

// Return an indexed 0x5C-byte course-boundary record; the caller must supply a valid index.
Fn801EB238Item* Array_GetItemByIndex(Fn801EB238Object* self, int index) {
	return reinterpret_cast<Fn801EB238Item*>(self->items) + index;
}

// Return the course boundary-record count carried in the original pointer-valued ABI.
void* CourseData_GetBoundaryRecordCount(Fn801EB248Object* self) {
	return self->field7C;
}
#pragma dont_inline reset

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Add a beach object to the scene's pooled circular list while holding its mutex.
// Transfer/publication/failure contract: Fn801EBList above. No success status.
void SceneBeachList_AppendObject(Fn801EBList* self, void** value) {
	WInstanceLock appendLock(&self->mutexStorage[0]);
	Fn801EBNode* slabNode;
	char* slabMutex;
	Fn801EBNode* slabRingAnchor;
	int slabNodeIndex;
	Fn801EBNode* reusableNodeOrActiveAnchor;
	{
		WInstanceLock reserveLock(&self->mutexStorage[0]);
		// Phase 1: reserve and register a slab only when the reusable ring is empty.
		// Reserve: register slab node 0; put nodes 1..nodesPerSlab-1 into the free ring.
		if (!self->reusableRing) {
			slabNode = static_cast<Fn801EBNode*>(appMallocUni__FUl(PoolNodesPerSlab(self) * sizeof(Fn801EBNode)));
			slabMutex = &self->mutexStorage[0];
			slabRingAnchor = static_cast<Fn801EBNode*>(self->slabRing);
			OSLockMutex((OSMutex*)slabMutex);
			if (!slabRingAnchor) {
				InitializeScenePoolRing(slabNode);
				OSUnlockMutex((OSMutex*)slabMutex);
				slabRingAnchor = slabNode;
			} else {
				SpliceScenePoolNodeBefore(slabNode, slabRingAnchor);
				OSUnlockMutex((OSMutex*)slabMutex);
			}
			self->slabRing = slabRingAnchor;
			slabNodeIndex = 1;
			if (PoolNodesPerSlab(self) > 1) {
				char* freeRingMutex = &self->mutexStorage[0];
				++slabNode;
				while (slabNodeIndex < PoolNodesPerSlab(self)) {
					{
						// Insert this slab node into the free ring, creating the ring if empty.
						reusableNodeOrActiveAnchor = static_cast<Fn801EBNode*>(self->reusableRing);
						OSLockMutex((OSMutex*)freeRingMutex);
						if (!reusableNodeOrActiveAnchor) {
							InitializeScenePoolRing(slabNode);
							OSUnlockMutex((OSMutex*)freeRingMutex);
							reusableNodeOrActiveAnchor = slabNode;
						} else {
							SpliceScenePoolNodeBefore(slabNode, reusableNodeOrActiveAnchor);
							OSUnlockMutex((OSMutex*)freeRingMutex);
						}
						self->reusableRing = reusableNodeOrActiveAnchor;
					}
					++slabNode;
					++slabNodeIndex;
				}
			}
		}
		// Detach one reusable node; a singleton leaves the reusable ring empty.
		reusableNodeOrActiveAnchor = static_cast<Fn801EBNode*>(self->reusableRing);
		char* detachMutex = &self->mutexStorage[0];
		OSLockMutex((OSMutex*)detachMutex);
		UnlinkScenePoolNode(reusableNodeOrActiveAnchor);
		Fn801EBNode* next = reusableNodeOrActiveAnchor->next;
		if (next == reusableNodeOrActiveAnchor) next = 0;
		OSUnlockMutex((OSMutex*)detachMutex);
		self->reusableRing = next;
	}
	// Phase 2: initialize detached payload, then splice it into the active ring.
	// value is reused below for the active-head address; its wrapper is not retained.
	reusableNodeOrActiveAnchor->value = *value;
	reusableNodeOrActiveAnchor->renderFlags = 0;
	char* activeRingMutex = &self->mutexStorage[0];
	*(int*)&reusableNodeOrActiveAnchor->renderState = 0;
	value = static_cast<void**>(self->activeRing);
	OSLockMutex((OSMutex*)activeRingMutex);
	if (value == 0) {
		InitializeScenePoolRing(reusableNodeOrActiveAnchor);
		OSUnlockMutex((OSMutex*)activeRingMutex);
	} else {
		SpliceScenePoolNodeBefore(reusableNodeOrActiveAnchor, (Fn801EBNode*)value);
		OSUnlockMutex((OSMutex*)activeRingMutex);
		reusableNodeOrActiveAnchor = (Fn801EBNode*)value;
	}
	self->activeRing = reusableNodeOrActiveAnchor;
}

// Add a river or lava object to the scene's pooled circular list while holding its mutex.
// Transfer/publication/failure contract: Fn801EBList above. No success status.
void SceneRiverList_AppendObject(Fn801EBList* self, void** value) {
	WInstanceLock appendLock(&self->mutexStorage[0]);
	Fn801EBNode* slabNode;
	char* slabMutex;
	Fn801EBNode* slabRingAnchor;
	int slabNodeIndex;
	Fn801EBNode* reusableNodeOrActiveAnchor;
	{
		WInstanceLock reserveLock(&self->mutexStorage[0]);
		// Phase 1: reserve and register a slab only when the reusable ring is empty.
		// Reserve: register slab node 0; put nodes 1..nodesPerSlab-1 into the free ring.
		if (!self->reusableRing) {
			slabNode = static_cast<Fn801EBNode*>(appMallocUni__FUl(PoolNodesPerSlab(self) * sizeof(Fn801EBNode)));
			slabMutex = &self->mutexStorage[0];
			slabRingAnchor = static_cast<Fn801EBNode*>(self->slabRing);
			OSLockMutex((OSMutex*)slabMutex);
			if (!slabRingAnchor) {
				InitializeScenePoolRing(slabNode);
				OSUnlockMutex((OSMutex*)slabMutex);
				slabRingAnchor = slabNode;
			} else {
				SpliceScenePoolNodeBefore(slabNode, slabRingAnchor);
				OSUnlockMutex((OSMutex*)slabMutex);
			}
			self->slabRing = slabRingAnchor;
			slabNodeIndex = 1;
			if (PoolNodesPerSlab(self) > 1) {
				char* freeRingMutex = &self->mutexStorage[0];
				++slabNode;
				while (slabNodeIndex < PoolNodesPerSlab(self)) {
					{
						// Insert this slab node into the free ring, creating the ring if empty.
						reusableNodeOrActiveAnchor = static_cast<Fn801EBNode*>(self->reusableRing);
						OSLockMutex((OSMutex*)freeRingMutex);
						if (!reusableNodeOrActiveAnchor) {
							InitializeScenePoolRing(slabNode);
							OSUnlockMutex((OSMutex*)freeRingMutex);
							reusableNodeOrActiveAnchor = slabNode;
						} else {
							SpliceScenePoolNodeBefore(slabNode, reusableNodeOrActiveAnchor);
							OSUnlockMutex((OSMutex*)freeRingMutex);
						}
						self->reusableRing = reusableNodeOrActiveAnchor;
					}
					++slabNode;
					++slabNodeIndex;
				}
			}
		}
		// Detach one reusable node; a singleton leaves the reusable ring empty.
		reusableNodeOrActiveAnchor = static_cast<Fn801EBNode*>(self->reusableRing);
		char* detachMutex = &self->mutexStorage[0];
		OSLockMutex((OSMutex*)detachMutex);
		UnlinkScenePoolNode(reusableNodeOrActiveAnchor);
		Fn801EBNode* next = reusableNodeOrActiveAnchor->next;
		if (next == reusableNodeOrActiveAnchor) next = 0;
		OSUnlockMutex((OSMutex*)detachMutex);
		self->reusableRing = next;
	}
	// Phase 2: initialize detached payload, then splice it into the active ring.
	// value is reused below for the active-head address; its wrapper is not retained.
	reusableNodeOrActiveAnchor->value = *value;
	reusableNodeOrActiveAnchor->renderFlags = 0;
	char* activeRingMutex = &self->mutexStorage[0];
	*(int*)&reusableNodeOrActiveAnchor->renderState = 0;
	value = static_cast<void**>(self->activeRing);
	OSLockMutex((OSMutex*)activeRingMutex);
	if (value == 0) {
		InitializeScenePoolRing(reusableNodeOrActiveAnchor);
		OSUnlockMutex((OSMutex*)activeRingMutex);
	} else {
		SpliceScenePoolNodeBefore(reusableNodeOrActiveAnchor, (Fn801EBNode*)value);
		OSUnlockMutex((OSMutex*)activeRingMutex);
		reusableNodeOrActiveAnchor = (Fn801EBNode*)value;
	}
	self->activeRing = reusableNodeOrActiveAnchor;
}

// Add a generated-texture object to the scene's pooled circular list while holding its mutex.
// Transfer/publication/failure contract: Fn801EBList above. No success status.
void SceneGeneratedTextureList_AppendObject(Fn801EBList* self, void** value) {
	Fn801EBNode* slabNode;
	int slabNodeIndex;
	Fn801EBNode* reusableNodeOrActiveAnchor;
	const WInstanceLock& appendLock = WInstanceLock(&self->mutexStorage[0]);
	{
		const WInstanceLock& reserveLock = WInstanceLock(&self->mutexStorage[0]);
		// Phase 1: reserve and register a slab only when the reusable ring is empty.
		// Reserve: register slab node 0; put nodes 1..nodesPerSlab-1 into the free ring.
		if (!self->reusableRing) {
			slabNode = static_cast<Fn801EBNode*>(appMallocUni__FUl(PoolNodesPerSlab(self) * sizeof(Fn801EBNode)));
			char* slabMutex = &self->mutexStorage[0];
			Fn801EBNode* slabRingAnchor = static_cast<Fn801EBNode*>(self->slabRing);
			OSLockMutex((OSMutex*)slabMutex);
			if (!slabRingAnchor) {
				InitializeScenePoolRing(slabNode);
				OSUnlockMutex((OSMutex*)slabMutex);
				slabRingAnchor = slabNode;
			} else {
				SpliceScenePoolNodeBefore(slabNode, slabRingAnchor);
				OSUnlockMutex((OSMutex*)slabMutex);
			}
			self->slabRing = slabRingAnchor;
			slabNodeIndex = 1;
			if (PoolNodesPerSlab(self) > 1) {
				char* freeRingMutex = &self->mutexStorage[0];
				++slabNode;
				while (slabNodeIndex < PoolNodesPerSlab(self)) {
					{
						// Insert this slab node into the free ring, creating the ring if empty.
						reusableNodeOrActiveAnchor = static_cast<Fn801EBNode*>(self->reusableRing);
						OSLockMutex((OSMutex*)freeRingMutex);
						if (!reusableNodeOrActiveAnchor) {
							InitializeScenePoolRing(slabNode);
							OSUnlockMutex((OSMutex*)freeRingMutex);
							reusableNodeOrActiveAnchor = slabNode;
						} else {
							SpliceScenePoolNodeBefore(slabNode, reusableNodeOrActiveAnchor);
							OSUnlockMutex((OSMutex*)freeRingMutex);
						}
						self->reusableRing = reusableNodeOrActiveAnchor;
					}
					++slabNode;
					++slabNodeIndex;
				}
			}
		}
		// Detach one reusable node; a singleton leaves the reusable ring empty.
		reusableNodeOrActiveAnchor = static_cast<Fn801EBNode*>(self->reusableRing);
		char* detachMutex = &self->mutexStorage[0];
		OSLockMutex((OSMutex*)detachMutex);
		UnlinkScenePoolNode(reusableNodeOrActiveAnchor);
		Fn801EBNode* next = reusableNodeOrActiveAnchor->next;
		if (next == reusableNodeOrActiveAnchor) next = 0;
		OSUnlockMutex((OSMutex*)detachMutex);
		self->reusableRing = next;
	} // reserveLock destroyed here
	// Phase 2: initialize detached payload, then splice it into the active ring.
	// value is reused below for the active-head address; its wrapper is not retained.
	reusableNodeOrActiveAnchor->value = *value;
	reusableNodeOrActiveAnchor->renderFlags = 0;
	char* activeRingMutex = &self->mutexStorage[0];
	*(int*)&reusableNodeOrActiveAnchor->renderState = 0;
	value = static_cast<void**>(self->activeRing);
	OSLockMutex((OSMutex*)activeRingMutex);
	if (value == 0) {
		InitializeScenePoolRing(reusableNodeOrActiveAnchor);
		OSUnlockMutex((OSMutex*)activeRingMutex);
	} else {
		SpliceScenePoolNodeBefore(reusableNodeOrActiveAnchor, (Fn801EBNode*)value);
		OSUnlockMutex((OSMutex*)activeRingMutex);
		reusableNodeOrActiveAnchor = (Fn801EBNode*)value;
	}
	self->activeRing = reusableNodeOrActiveAnchor;
}

// Advance the course-entry iterator by one 0x20-byte entry.
void CourseEntryIterator_Advance(Fn801EB82CObject* self) {
	self->field00 = static_cast<char*>(self->field00) + 0x20;
}

// Add a course boundary object to the scene's pooled circular list while holding its mutex.
// Transfer/publication/failure contract: Fn801EBList above. No success status.
void SceneBoundaryList_AppendObject(Fn801EBList* self, void** value) {
	Fn801EBNode* slabNode;
	char* slabMutex;
	Fn801EBNode* slabRingAnchor;
	int slabNodeIndex;
	Fn801EBNode* reusableNodeOrActiveAnchor;
	const WInstanceLock& appendLock = WInstanceLock(&self->mutexStorage[0]);
	{
		const WInstanceLock& reserveLock = WInstanceLock(&self->mutexStorage[0]);
		// Phase 1: reserve and register a slab only when the reusable ring is empty.
		// Reserve: register slab node 0; put nodes 1..nodesPerSlab-1 into the free ring.
		if (!self->reusableRing) {
			slabNode = static_cast<Fn801EBNode*>(appMallocUni__FUl(PoolNodesPerSlab(self) * sizeof(Fn801EBNode)));
			slabMutex = &self->mutexStorage[0];
			slabRingAnchor = static_cast<Fn801EBNode*>(self->slabRing);
			OSLockMutex((OSMutex*)slabMutex);
			if (!slabRingAnchor) {
				InitializeScenePoolRing(slabNode);
				OSUnlockMutex((OSMutex*)slabMutex);
				slabRingAnchor = slabNode;
			} else {
				SpliceScenePoolNodeBefore(slabNode, slabRingAnchor);
				OSUnlockMutex((OSMutex*)slabMutex);
			}
			self->slabRing = slabRingAnchor;
			slabNodeIndex = 1;
			if (PoolNodesPerSlab(self) > 1) {
				char* freeRingMutex = &self->mutexStorage[0];
				++slabNode;
				while (slabNodeIndex < PoolNodesPerSlab(self)) {
					{
						// Insert this slab node into the free ring, creating the ring if empty.
						reusableNodeOrActiveAnchor = static_cast<Fn801EBNode*>(self->reusableRing);
						OSLockMutex((OSMutex*)freeRingMutex);
						if (!reusableNodeOrActiveAnchor) {
							InitializeScenePoolRing(slabNode);
							OSUnlockMutex((OSMutex*)freeRingMutex);
							reusableNodeOrActiveAnchor = slabNode;
						} else {
							SpliceScenePoolNodeBefore(slabNode, reusableNodeOrActiveAnchor);
							OSUnlockMutex((OSMutex*)freeRingMutex);
						}
						self->reusableRing = reusableNodeOrActiveAnchor;
					}
					++slabNode;
					++slabNodeIndex;
				}
			}
		}
		// Detach one reusable node; a singleton leaves the reusable ring empty.
		reusableNodeOrActiveAnchor = static_cast<Fn801EBNode*>(self->reusableRing);
		char* detachMutex = &self->mutexStorage[0];
		OSLockMutex((OSMutex*)detachMutex);
		UnlinkScenePoolNode(reusableNodeOrActiveAnchor);
		Fn801EBNode* next = reusableNodeOrActiveAnchor->next;
		if (next == reusableNodeOrActiveAnchor) next = 0;
		OSUnlockMutex((OSMutex*)detachMutex);
		self->reusableRing = next;
	}
	// Phase 2: initialize detached payload, then splice it into the active ring.
	// value is reused below for the active-head address; its wrapper is not retained.
	reusableNodeOrActiveAnchor->value = *value;
	reusableNodeOrActiveAnchor->renderFlags = 0;
	char* activeRingMutex = &self->mutexStorage[0];
	*(int*)&reusableNodeOrActiveAnchor->renderState = 0;
	value = static_cast<void**>(self->activeRing);
	OSLockMutex((OSMutex*)activeRingMutex);
	if (value == 0) {
		InitializeScenePoolRing(reusableNodeOrActiveAnchor);
		OSUnlockMutex((OSMutex*)activeRingMutex);
	} else {
		SpliceScenePoolNodeBefore(reusableNodeOrActiveAnchor, (Fn801EBNode*)value);
		OSUnlockMutex((OSMutex*)activeRingMutex);
		reusableNodeOrActiveAnchor = (Fn801EBNode*)value;
	}
	self->activeRing = reusableNodeOrActiveAnchor;
}
#pragma pop

// Select shadows for clear/cloud/rain/snow: courses 4..8 and 13 disable selection flags.
// Special weather 4 never calls this helper and leaves the shadow setting untouched.
static inline void ApplyCourseWeatherShadowPolicy(unsigned int courseId, int& useCourseShadow) {
        switch ((unsigned char)courseId) {
        case kWeatherCourse4: case kWeatherCourse5: case kWeatherCourse6:
        case kWeatherCourse7: case kRainSubstitutesSnowCourse8: case kScaledAmbientSnowCourse13:
            useCourseShadow = 0;
            break;
        }
        if (useCourseShadow == 0) CShadowManager_SetSelectionFlags(g_shadowManager, 0);
        else CShadowManager_SetSelectionFlags(g_shadowManager, 1);
}

// Replace weather particles/clouds as needed and choose the course's shadow settings.
// Failure/publication contract shared with CreateCourseObjects: recreation first
// destroys and nulls primary +F8 and secondary +FC via ReleaseOwnedWeatherEffect
// (virtual deleting destructor). Every new Petal/Silica/Snow/Cloud/Rain instance
// is published in primary only after its constructor returns, then scale/count
// setup may still run. A throwing constructor leaves that slot null; later throws
// leave the published instance owned there. No local rollback or failure return.
// Ambient allocations precede the selectedWeather write; switch allocations follow
// it, so failure can leave old selectedWeather with new ambience, or new weather
// with an absent/partial effect. Secondary is released but never acquired here.
// Null returns are not checked before scale/update uses. All factories must
// succeed for normal use; completed scene geometry is retained on weather failure.
void CourseWeather_ConfigureEffects(void* object, unsigned char weather, int recreateEffects) {
    // Borrow object; it retains effect allocations. recreateEffects!=0 releases
    // BOTH slots first. Otherwise course branches may reuse +f8's existing type.
    // Primary-slot compatibility by course byte (display names unverified):
    // 11/12 outside rain -> petal/silica; 9 -> persistent light snow;
    // 8/13 outside rain/snow -> ambient snow (13 scales particle count).
    // Rain on 8 uses heavy snow; 8/9 update snow counts in rain/snow modes.
    // Clear/cloud/rain/snow disable course shadows on 4..8 and 13, enable on others;
    // mode 4 leaves shadows untouched. +fc is only released here, never allocated.
    // Primary-slot class contracts:
    // 11 -> ntPetal; 12 -> ntSilicaParticle (outside rain); 9 -> CSnow (all modes).
    // 8 -> CSnow (ambient or rain substitute); 13 -> CSnow ambience with scaled count.
    // Other courses -> CCloud / CRain / CSnow according to requested weather.
    // A nonnull primary slot is reused without a runtime type check. In rain/snow,
    // only course bytes 8/9 receive the snow-count virtual update. Recreation releases
    // both slots before changing class; the secondary slot's creator is unknown here.
    // The masked-add range tests below deliberately compare the LOW BYTE only.
    // Phase 1: optional destructive recreation, then course-specific ambient effects.
    CourseWeatherOwner* self = (CourseWeatherOwner*)object;
    unsigned int courseId = SharedDoc_GetCourseId();
    if (recreateEffects != 0) {
        ReleaseOwnedWeatherEffect(OwnedCourseWeatherEffect(self));
        ReleaseOwnedWeatherEffect(OwnedSecondaryWeatherEffect(self));
    }
    if (((courseId + kWeatherPair11WrapBias) & kWeatherCourseByteMask) <= 1 && weather != kRainWeather) {
        if (!OwnedCourseWeatherEffect(self)) {
            if ((unsigned char)courseId == kWeatherPetalCourse11) {
                OwnedCourseWeatherEffect(self) = new PetalEffectAlloc(kPetalParticleCount, (char*)courseGlowEffectStrings + kEmptyWeatherResource, 0);
            } else if ((unsigned char)courseId == kWeatherSilicaCourse12) {
                OwnedCourseWeatherEffect(self) = new SilicaParticleEffectAlloc(kPetalParticleCount, (char*)courseGlowEffectStrings + kEmptyWeatherResource, 0);
            }
            CourseWeatherEffectScale(OwnedCourseWeatherEffect(self)) = kWeatherPetalEffectScale;
        }
    } else {
        ReleaseOwnedWeatherEffect(OwnedSecondaryWeatherEffect(self));
    }
    if ((unsigned char)courseId == kPersistentLightSnowCourse9) {
        if (!OwnedCourseWeatherEffect(self)) {
            SnowEffectAlloc* p = new SnowEffectAlloc(kLightSnowCount, (char*)courseGlowEffectStrings + kSnowParticleResource, 0, kWeatherSnowConstructorScale);
            OwnedCourseWeatherEffect(self) = p;
            CourseWeatherEffectScale(p) = kFloat_0_25_Scene;
        }
    } else if (SharedDoc_IsCourse8Or13() != 0 && weather != kRainWeather && weather != kSnowWeather) {
        if (!OwnedCourseWeatherEffect(self)) {
            int particleCount = kAmbientSnowCount;
            if ((SharedDoc_GetCourseId() & kWeatherCourseByteMask) == kScaledAmbientSnowCourse13) {
                particleCount = (int)(kWeatherCourse13SnowCountScale * (float)particleCount);
            }
            SnowEffectAlloc* p = new SnowEffectAlloc(particleCount, (char*)courseGlowEffectStrings + kAmbientSnowResource, 0, kWeatherAmbientSnowConstructorScale);
            OwnedCourseWeatherEffect(self) = p;
            CourseWeatherEffectScale(p) = kWeatherAmbientSnowEffectScale;
        }
    } else {
        // Only this branch short-circuits unchanged weather. Ambient-course
        // branches above continue to the weather switch even without recreation.
        if (weather == ((SharedWeatherView*)g_sharedDoc)->selectedWeather && recreateEffects == 0) return;
        ReleaseOwnedWeatherEffect(OwnedCourseWeatherEffect(self));
    }
    // Phase 2: commit selected weather, create/update its particles and select shadow behavior.
    // Courses 8/9 substitute snow for rain; 11/12 use petal/silica ambience outside rain.
    ((SharedWeatherView*)g_sharedDoc)->selectedWeather = weather;
    switch (weather) {
    case kClearWeather: { // Keep course ambience, refresh course-9 particle count, apply shadows.
            if ((unsigned char)courseId == kPersistentLightSnowCourse9) {
                int r = rand() % kSpecialParticleCount + kSpecialParticleCount;
                VEffect* weatherEffect = (VEffect*)OwnedCourseWeatherEffect(self);
                weatherEffect->update(r, kSceneHalfScale);
            }
            int useCourseShadow = 1;
            ApplyCourseWeatherShadowPolicy(courseId, useCourseShadow);
            break;
        }
    case kCloudyWeather: { // Ensure a primary effect/cloud, refresh course 9, apply shadows.
            if (!OwnedCourseWeatherEffect(self)) {
                OwnedCourseWeatherEffect(self) = new CloudEffectAlloc();
            }
            if ((unsigned char)courseId == kPersistentLightSnowCourse9) {
                int r = rand() % kSpecialParticleCount + kSpecialParticleCount;
                VEffect* weatherEffect = (VEffect*)OwnedCourseWeatherEffect(self);
                weatherEffect->update(r, kSceneHalfScale);
            }
            int useCourseShadow = 1;
            ApplyCourseWeatherShadowPolicy(courseId, useCourseShadow);
            break;
        }
    case kRainWeather: { // Use snow on course 8 or rain elsewhere; update snow counts/shadows.
            if (!OwnedCourseWeatherEffect(self)) {
                if ((unsigned char)courseId == kRainSubstitutesSnowCourse8) {
                    SnowEffectAlloc* p = new SnowEffectAlloc(kHeavySnowCount, (char*)courseGlowEffectStrings + kSnowParticleResource, 0, kWeatherSnowConstructorScale);
                    OwnedCourseWeatherEffect(self) = p;
                    CourseWeatherEffectScale(p) = kFloat_0_25_Scene;
                } else {
                    OwnedCourseWeatherEffect(self) = new RainEffectAlloc();
                }
            }
            if (((courseId + kWeatherPair8WrapBias) & kWeatherCourseByteMask) <= 1) {
                VEffect* weatherEffect = (VEffect*)OwnedCourseWeatherEffect(self);
                weatherEffect->update(kHeavySnowCount, kFloat_1_805DF9F8);
            }
            int useCourseShadow = 1;
            ApplyCourseWeatherShadowPolicy(courseId, useCourseShadow);
            break;
        }
    case kSnowWeather: { // Ensure/update snow and apply the course shadow policy.
            if (!OwnedCourseWeatherEffect(self)) {
                SnowEffectAlloc* p = new SnowEffectAlloc(kHeavySnowCount, (char*)courseGlowEffectStrings + kSnowParticleResource, 0, kWeatherSnowConstructorScale);
                OwnedCourseWeatherEffect(self) = p;
                CourseWeatherEffectScale(p) = kFloat_0_25_Scene;
            } else if (((courseId + kWeatherPair8WrapBias) & kWeatherCourseByteMask) <= 1) {
                VEffect* weatherEffect = (VEffect*)OwnedCourseWeatherEffect(self);
                weatherEffect->update(kHeavySnowCount, kFloat_1_805DF9F8);
            }
            int useCourseShadow = 1;
            ApplyCourseWeatherShadowPolicy(courseId, useCourseShadow);
            break;
        }
    case kSpecialWeatherEffect: { // Ensure special particles; leave the shadow selection unchanged.
            if (!OwnedCourseWeatherEffect(self)) {
                SnowEffectAlloc* p = new SnowEffectAlloc(kSpecialParticleCount, (char*)courseGlowEffectStrings + kSpecialWeatherResource, 2, kWeatherSnowConstructorScale);
                OwnedCourseWeatherEffect(self) = p;
                CourseWeatherEffectScale(p) = kWeatherSpecialEffectScale;
            }
            break;
        }
    default:
        break;
    }
}

// Allocate and construct the score UI actor.
Fn801EC088Object* CScore_Create() {
	return new Fn801EC088Object;
}

// Initialize score/bonus transitions, menu state and owned score/rank forms.
// Fresh self only, returns self after native-base initialization. +28=1 and
// +48/+4C=0 are constructor-only words in recovered score operations; preserve
// them, with no invented tuning effect. CScore_OnProcess (score_actor_and_generated_surfaces.cpp) lazily creates
// owned CBonusInfo at +38; HandleMsg command 0 deletes/replaces it when enabling
// bonus display. OnDestroy here frees only rankView/scoreForm, NOT bonusOverlay:
// clearing/closing the bonus workflow is a separate lifetime responsibility.
ScoreConstructorView* CScore_Ctor(ScoreConstructorView* self) {
	__ct__18NativeBase80201D18Fv(self);
	int zero = 0;
	void* base = __vt__6CScore;
	float zeroTime = kScoreInitialTime;
	void* actorInterfaceVtable = (char*)base + kScoreActorInterfaceVtableOffset;
	int one = 1;
	int actorType = 5;
	self->vtable = base;
	ScoreActorInterfaceVtable(self) = actorInterfaceVtable; // secondary actor interface at +0x18; not owned
	AsCScore(self)->bonusOverlayEnabled = zero;
	AsCScore(self)->elapsedTime = zeroTime;
	AsCScore(self)->transitionState = zero;
	self->field28 = one; // +0x28 constructor flag; consumers not established here
	AsCScore(self)->transitionProgress = zeroTime;
	*(int*)&AsCScore(self)->bonusOverlay = zero;
	AsCScore(self)->result = zero;
	AsCScore(self)->scoreMenuActive = zero;
	AsCScore(self)->currentPlayerId = zero;
	self->field48 = zero; // +0x48 state word; meaning unresolved
	self->field4C = zero; // +0x4C state word; meaning unresolved
	*(int*)&AsCScore(self)->scoreForm = zero;
	*(int*)&AsCScore(self)->rankView = zero;
	AsCScore(self)->menusInitialized = zero;
	self->actorType = actorType;
	return self;
}

// Release score/rank forms and reset the four input cursors' draw and color overrides.
void CScore_OnDestroy(ScoreFormOwnerView* self) {
	{
		void* p = AsCScore(self)->rankView;
		if (p) {
			if (p) {
				((ScoreOwned*)p)->release(1);
			}
			AsCScore(self)->rankView = 0;
		}
	}
	{
		void* p = AsCScore(self)->scoreForm;
		if (p) {
			if (p) {
				((ScoreOwned*)p)->release(1);
			}
			AsCScore(self)->scoreForm = 0;
		}
	}
	if (!Sing_805DC5C8()) {
		if (!s_inputManagerInitialized) {
			InputManager_Init(s_gameInputManager);
			__register_global_object(s_gameInputManager, reinterpret_cast<void*>(InputManager_Dtor), s_scoreInputManagerDestructorRegistration);
			s_inputManagerInitialized = 1;
		}
		Sing_805DC5C8() = s_gameInputManager;
	}
	void* manager = Sing_805DC5C8();
	int index = 0;
	do {
		InputManager_SetCursorDrawOptions(manager, 0, 0, 0, index);
		InputManager_SetCursorColorOverride(manager, 0, 0xD6, 0x48, 0x5C, index);
		++index;
	} while (index < 4);
}

// The score actor has no extra load-time work.
void CScore_OnLoad() {}

// Send zero-filled setup commands 3 and 6 through the actor initialization hook.
void CScore_OnReload(Fn801EC280Object* first) {
	Fn801EC280Args args1;
	args1.values[0] = 0;
	args1.values[1] = 3;
	args1.values[2] = 0;
	args1.values[3] = 0;
	args1.values[4] = 0;
	args1.values[5] = 0;
	args1.values[6] = 0;
	args1.values[7] = 0;
	if (first) {
		((V2Cs*)first)->v2C(&args1);
	}
	Fn801EC280Args args2;
	args2.values[0] = 0;
	args2.values[1] = 6;
	args2.values[2] = 0;
	args2.values[3] = 0;
	args2.values[4] = 0;
	args2.values[5] = 0;
	args2.values[6] = 0;
	args2.values[7] = 0;
	if (first) {
		((V2Cs*)first)->v2C(&args2);
	}
}

} // extern "C"

#undef AT
#undef ATI
#undef PTR
#undef VC

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621AC", _ctors_u801E8048_0, SceneRenderList_InitAngleConstant);
#pragma pop
