// Golf course background actor: scheduled resource/reload commands, terrain-grounded
// tee/cup positions, wind/sky particles, and the closed camera flyover route.
// Entry/mod lookup:
// - Camera geometry: CGolfBg_HandleMsg/kBuildCameraPath, kCameraGroundClearance,
//   kCameraEndExtension and kCameraTurnWidth below; all are world distances.
// - Reload: RequestCourseReload starts, FinalizeCourseReload polls/initializes,
//   and BackgroundCommand 12 writes completion to caller storage. Keep saved
//   hole order, party-control restart and FX re-enable together at completion.
// - Terrain placement: SetCurrentHoleStartOnGround/SetCurrentHoleCupOnGround;
//   GetGroundPoint in course_collision_and_ball_physics.cpp defines floor fallback and triangle-ID output.
// - Sky/wind: command IDs 1/6/10; CPvsFxBox owns loaded effect resources. Wind
//   lifetime uses 1+9/windSpeed; zero wind is not checked in that command.
// Shared layouts: CGolfBg/CGolfDoc/CSharedDoc/CPvsFxBox; asset names and paths
// come from golfBackgroundStrings (.rodata 0x8046A888 in symbols.txt). Receivers/globals must
// be initialized and actor dispatch serialized; commands borrow their payloads.
// The due-time clock is milliseconds, route coordinates are world XYZ (+Y up),
// and route generation consumes two rand() choices for end-turn side. This unit
// has no presentation frame-rate, resolution or aspect-ratio setting.
#include "classes/CGolfBg.h"
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/string.h"
#include "game/wvector.h"
#include "classes/CPvsFxBox.h"
#include "iff/IffStruct.h"

// Translation unit 0x8012484C-0x80125CAC (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "sdk_protos.h"

extern "C" {

// Point vectors store data, logical size, and allocated capacity in that order.
struct Vec3Array { void* data; unsigned int count; unsigned int capacity; };
struct BackgroundMessage {
    void* sender;
    unsigned int type;
    union { unsigned int word; int integer; void* object; const char* text; } payload;
    char reserved[0x10];
    unsigned int dueTimeMilliseconds;
};
struct HolePositions { WVector start; WVector cup; char reserved[8]; };
struct RoundHoleOrder { char reserved[0x82A]; unsigned char holes[18]; };
// This hole byte and the ground-query context are not exposed by the shared views yet.
struct CourseHoleSelection { char reserved[0x3B7]; unsigned char holeNumber; };
struct BallGroundQuery { char reserved[0x28]; int context; };
// Command contract (payload storage is borrowed for this synchronous dispatch):
// 1 sky: no payload; 2 particle color: packed word; 3 reload current hole: none;
// 4 camera route: writable Vec3Array*, at least two source aim points required;
// 5 surface hierarchy: C string; 6 wind particles: none (nonzero wind speed assumed);
// 8 rank course / 9 current hole: none; 10 wind actors: none;
// 11 request reload: integer mode; 12 poll reload: required writable void**,
// receives the integer completion result encoded as a pointer (0 or 1), not an
// owned object; 0x2712 debug: required NUL-terminated byte string, at most 255
// bytes before NUL (debugText[256], unbounded strcpy; no truncation/status).
// Commands 4 and 12 write caller storage;
// other commands only read their payload. No payload is retained by this switch.
// Future dueTimeMilliseconds invokes receiver slot10 and returns without dispatch.
enum BackgroundCommand {
    kReloadSky = 1, kSetParticleColor = 2, kReloadCurrentHole = 3,
    kBuildCameraPath = 4, kSetSurfaceContext = 5, kInitializeWind = 6,
    kLoadRankCourse = 8, kLoadCurrentHole = 9, kUpdateWindActors = 10,
    kRequestCourseReload = 11, kPollCourseReload = 12, kCopyDebugText = 0x2712
};
enum {
    kDartsParty = 1, kDriveParty = 2, kBalloonParty = 3, kFullRoundHoleCount = 18,
    kHoleCopyGroups = 3, kHolesPerCopyGroup = 6, kWindParticleCount = 50,
    // World-distance units, +Y up; no conversion to yards or screen pixels.
    // Clearance lifts grounded controls; extension lengthens the two end turns;
    // width offsets the far turn laterally before its 250-unit extension.
    // The final sidePoint uses a cross product of that extension: its length
    // follows extension, not width. Midpoints use the same lifted endpoints.
    // Increase clearance for height, extension for longer turns, width for more
    // lateral separation at the far end. Safe tuning ranges are not established.
    kCameraGroundClearance = 100, kCameraEndExtension = 250, kCameraTurnWidth = 30
};
typedef char BackgroundMessage_payload_offset[(IFF_OFFSET(BackgroundMessage, payload) == 8) ? 1 : -1];
typedef char BackgroundMessage_time_offset[(IFF_OFFSET(BackgroundMessage, dueTimeMilliseconds) == 0x1C) ? 1 : -1];
typedef char HolePositions_size[(sizeof(HolePositions) == 32) ? 1 : -1];
typedef char RoundHoleOrder_holes_offset[(IFF_OFFSET(RoundHoleOrder, holes) == 0x82A) ? 1 : -1];

// Clear a vector of trivial 3D points; retain the original volatile count access.
void Vec3Vector_Clear(void* self) {
    volatile int* p = (volatile int*)&((Vec3Array*)self)->count;
    union { int a; unsigned int b; } value;
    value.a = *p;
    *p = value.a - value.b;
}

// Return the data pointer of a non-virtual vector.
void* Vector_GetData(void* self) {
    return ((Vec3Array*)self)->data;
}

// Return the unchecked point at the requested index in a 3D-point vector.
void* Vec3Vector_GetPointAt(void* self, int index) {
    return (char*)((Vec3Array*)self)->data + index * sizeof(WVector);
}

// Return the embedded aim-path vector unchanged.
void* AimPath_GetPointVector(void* self) { return self; }

// Set the packed color used by the course effect particles.
void CPvsFxBox_SetParticleColor(void* self, unsigned int value) {
    ((CPvsFxBox*)self)->particleColor = value;
}

// Return allocated point capacity; this accessor does not report logical point count.
unsigned int Vec3Vector_GetCapacity(void* self) {
    return ((Vec3Array*)self)->capacity;
}

// Return the last point; callers must provide a nonempty vector.
void* Vec3Vector_GetBack(void* self) {
    return (char*)((Vec3Array*)self)->data + (((Vec3Array*)self)->count - 1) * sizeof(WVector);
}

extern void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
extern const char golfBackgroundStrings[];
struct GolfBackgroundStringPool {
    char ambientActor[8]; // +0x0: Ambient
    char reserved8[0x10];
    char courseBinaryFormat[13]; // +0x18: %s_%02d.gbin
    char reserved25[0x22];
    char golfRuleActor[9]; // +0x47: GolfRule
    char reserved50[0x13];
    char sunRecord[4]; // +0x63: sun
    char reserved67[0x59];
    char namedCourseBinaryFormat[8]; // +0xC0: %s.gbin
    char reservedC8[0xC];
    char stringFormat[3]; // +0xD4: %s
    char rankCourseBinaryFormat[13]; // +0xD7: %s_rank.gbin
    char effectActor[7]; // +0xE4: Effect
    char caddieActor[7]; // +0xEB: Caddie
    char gridActor[5]; // +0xF2: Grid
    char cameraActor[7]; // +0xF7: Camera
    char rankSuffix[5]; // +0xFE: rank
    char dartsSystem[12]; // +0x103: DartsSystem
    char driveSystem[12]; // +0x10F: DriveSystem
    char balloonSystem[14]; // +0x11B: BalloonSystem
};

// Send camera message 2 to the camera actor.
void GolfBg_DispatchCameraMessage2(void* arg) {
    IActor_DispatchMessage(arg, arg, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->cameraActor, 2, 0, 0, 0, 0);
}

extern void* g_sharedDoc;
extern void* g_fx;
extern void* g_taskManager;
extern void* g_golfCupPreview;
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void* GolfBall__Fv();
extern int CourseModel_IsPendingFilenameEmpty(void*);
extern void CGolfBg_InitializeCurrentHole(void*);
extern void CFx_SetUpdateEnabled(void*, int);
extern void* Object_GetChild(void*);
extern void* TaskActorTable_FindByName(void*, const char*);
extern void CupPreview_SetPosition(void*, void*, void*);
extern void CGolfDoc_SetCurrentHole(void*, int);
extern void* SharedStringState_GetSecondString(int);
extern int String_FormatTemporary(const char*, ...);
extern int CourseModel_MatchesFilename(void*, int);
extern int CourseModel_SetFileAndLoad(void*, int);
extern int String_ParseDecimalInt(const char*);
extern void SoundManager_StopAndClearChannels(void*);
extern void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);
extern void CFx_SetUpdateEnabled(void*, int);
extern void CFx_ClearControlsAndResetBox(void*);
extern void* g_soundManager;
extern unsigned char g_courseLoadFailed;
extern unsigned int g_currentTimeMilliseconds;
extern unsigned char g_courseResourceListsEnabled;
extern unsigned char CGolfDoc_GetCurrentHoleNumber(void*);
extern const char* SharedDoc_GetSelectedCourseMapName();
extern void* TaskManager_GetCurrentChild();
extern int CourseModel_LoadFromFile(void*, const char*);
extern void* CWind_GetInstance();
extern float CWind_GetWindSpeed(void*);
extern void CPvsFxBox_InitWindParticles(void*, int, float);
extern void CPvsFxBox_LoadSkyResources(void*, const char*, float);
extern void CPvsFxBox_LoadVariantBitmap(void*, const char*);
extern void CPvsFxBox_RebuildSceneEffects(void*);
extern void CPvsFxBox_ReloadCourseEffects(void*);
extern void CPvsFxBox_RebuildAuxiliaryEffect(void*);
extern void CourseData_SetSurfaceHierarchyContext(void*, const char*);
extern void CourseData_ReleaseResources(void*, int);
extern void CourseData_BuildQuadTree(void*);
extern void CShadowManager_CopyNamedRecordSettings(void*, const char*, void*);
extern void CPvsFxBox_ReleaseParticleResource(void*);
extern void CourseData_UpdateWindActors(void*);
extern int CGolfBg_FinalizeCourseReload(void*);
extern void CGolfBg_ReloadCurrentHoleResources(void*);
extern void CGolfBg_RequestCourseReload(void*, int);
extern void* CGolfBg_BuildCourseRankListPath(void*, void*);
extern void* Storage_ClearThreeWords(void*);
extern void* MWString_GetCString(void*);
extern void* CPackedFile_Create(void*);
extern void* CPackedFile_PollLoad(void*);
extern void Vec3Vector_AppendCopy(void*, void*);
extern void CGolfDoc_GetCurrentHoleStartPosition(void*);
extern void* __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void* __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void* __ml__FRC7WVectorf(void*, void*, float);
extern void* Vector_CrossProduct(void*, void*, void*);
extern void* Normalize__7WVectorFv(void*);
extern void* __amu__7WVectorFf(void*, float);
extern void* Vec3_Set(void*, float, float, float);
extern int rand();
extern void WVector_DefaultConstructNoOp(void*);
extern int Vec3Vector_GetPointCount(void*);
extern int Vec3Vector_ReservePointCapacity(void*, int);
extern void* RenderPacket_GetInstance();
extern void* RenderPacket_GetShadowData(void*);
extern void* ShadowManager_GetCurrent();
extern void* GolfDoc_GetCurrent();

extern void* SceneManager_GetCurrent();
extern void CGolfBg_CreateCourseObjects(void*);

#pragma inline_depth(0)
struct EventReceiver {
    virtual void slot0(); virtual void slot1(); virtual void slot2();
    virtual void slot3(); virtual void slot4(); virtual void slot5();
    virtual void slot6(); virtual void slot7(); virtual void slot8();
    virtual void slot9(); virtual void slot10();
};
struct PendingTask { 
virtual ~PendingTask();
};
// Dispatch scheduled course-loading, camera-path, particle, and reload commands.
void CGolfBg_HandleMsg(register void* actor, const void* message) { char debugText[256]; 

    // actor and message are borrowed for the dispatch; each switch arm is terminal.
    struct PathPoint { float x, y, z; } pathPoint, start, farTurnCenter, firstControlPoint, previousPoint, extensionPoint, sidePoint;
    char archivePathStorage[12];
    if (((const BackgroundMessage*)message)->dueTimeMilliseconds > g_currentTimeMilliseconds) {
        ((EventReceiver*)actor)->slot10();
        return;
    }
    unsigned int type = ((const BackgroundMessage*)message)->type;
    switch (type) {
    case kLoadCurrentHole: {
        unsigned int holeNumber = (unsigned char)CGolfDoc_GetCurrentHoleNumber(GolfDoc_GetCurrent());
        const char* courseFilename = (const char*)String_FormatTemporary(((const GolfBackgroundStringPool*)golfBackgroundStrings)->courseBinaryFormat, SharedDoc_GetSelectedCourseMapName(), (int)holeNumber);
        void* golfDocument = GolfDoc_GetCurrent();
        if (!CourseModel_LoadFromFile(AsCGolfDoc(golfDocument)->courseData, courseFilename)) {
            g_courseLoadFailed = 1;
            break;
        }
        CGolfBg_InitializeCurrentHole(actor);
        void* owner = TaskManager_GetCurrentChild();
        ((EventReceiver*)owner)->slot9();
        IActor_DispatchMessage(actor, actor, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->golfRuleActor, 9, 0, 0, 0, 0);
        break;
    }
    case kInitializeWind: {
        float windSpeed = CWind_GetWindSpeed(CWind_GetInstance());
        float particleLifetime=1.0f + 9.0f / windSpeed;
        void* golfDocument = GolfDoc_GetCurrent();
        CPvsFxBox_InitWindParticles(AsCGolfDoc(golfDocument)->courseEffectBox, kWindParticleCount, particleLifetime);
        break;
    }
    // Fit the entire byte string plus NUL in debugText[256]; overflow is unchecked.
    case kCopyDebugText: strcpy(debugText, ((const BackgroundMessage*)message)->payload.text); break;
    case kReloadSky: {
        const char* value = SharedDoc_GetSelectedCourseMapName();
        // Field-derived byte addresses preserve CodeWarrior's shared pool-base register.
        const char* skyFilename = (const char*)String_FormatTemporary(golfBackgroundStrings + IFF_OFFSET(GolfBackgroundStringPool, stringFormat), value);
        void* golfDocument = GolfDoc_GetCurrent();
        CPvsFxBox_LoadSkyResources(AsCGolfDoc(golfDocument)->courseEffectBox, skyFilename, 0.0f);
        value = SharedDoc_GetSelectedCourseMapName();
        skyFilename = (const char*)String_FormatTemporary(golfBackgroundStrings + IFF_OFFSET(GolfBackgroundStringPool, stringFormat), value);
        golfDocument = GolfDoc_GetCurrent();
        CPvsFxBox_LoadVariantBitmap(AsCGolfDoc(golfDocument)->courseEffectBox, skyFilename);
        golfDocument = GolfDoc_GetCurrent(); CPvsFxBox_RebuildSceneEffects(AsCGolfDoc(golfDocument)->courseEffectBox);
        golfDocument = GolfDoc_GetCurrent(); CPvsFxBox_ReloadCourseEffects(AsCGolfDoc(golfDocument)->courseEffectBox);
        golfDocument = GolfDoc_GetCurrent(); CPvsFxBox_RebuildAuxiliaryEffect(AsCGolfDoc(golfDocument)->courseEffectBox);
        break;
    }
    case kSetParticleColor: {
        void* golfDocument = GolfDoc_GetCurrent();
        CPvsFxBox_SetParticleColor(AsCGolfDoc(golfDocument)->courseEffectBox, ((const BackgroundMessage*)message)->payload.word);
        break;
    }
    case kReloadCurrentHole: CGolfBg_ReloadCurrentHoleResources(actor); break;
    case kBuildCameraPath: {
        // World-space route, Y up. Aim source is borrowed; output list owns copied points.
        // Input must contain >=2 points and distinct endpoint directions for normalization.
        // Output descriptor AND backing allocation must be disjoint from the aim
        // vector and its points. The source is read after output mutation below.
        // Ground clearance/extension/width are world-distance units, not screen pixels.
        // Phase 1: prepare output storage, lift start/first point and seed controls.
        // Build an elevated closed camera route: outbound controls, randomized end turn,
        // reverse controls, and randomized start turn. M(A,B) below means (A+B)*0.5.
        // Route sketch (P0..P[n-1] are aim controls, S is the grounded tee):
        //   M(startExtension,S), S, M(S,P0), P0, ... lifted P[n-2]
        //   -> farTurnCenter -> farExtension -> farSide -> P[n-2]
        //   -> reverse controls through P0 -> startSide -> startExtension -> M(startExtension,S).
        // Every arrow includes its midpoint; the first/last midpoint closes the route.
        // P[n-1] determines the far turn direction/center and is not appended directly.
        void* golfDocument = GolfDoc_GetCurrent();
        void* selected = AimPath_GetPointVector(AsCGolfDoc(golfDocument)->aimPath);
        void* list;
        list = ((const BackgroundMessage*)message)->payload.object;
        void* aimPoints = selected; // Borrowed source controls; list owns the output copies.
        WVector_DefaultConstructNoOp(&pathPoint); WVector_DefaultConstructNoOp(&start); WVector_DefaultConstructNoOp(&farTurnCenter);
        WVector_DefaultConstructNoOp(&firstControlPoint); WVector_DefaultConstructNoOp(&previousPoint); WVector_DefaultConstructNoOp(&extensionPoint); WVector_DefaultConstructNoOp(&sidePoint);
        int returnPointCount = (Vec3Vector_GetPointCount(aimPoints) - 3) * 2;
        int routePointCount = (Vec3Vector_GetPointCount(aimPoints) - 2) * 2 + returnPointCount + 0x11;
        unsigned int requiredCapacity = routePointCount;
        // These branches differ: ReservePointCapacity (course_entry_vectors_and_doc_accessors.cpp) preserves
        // points/count and invalidates the old data pointer; Clear sets count=0
        // while retaining allocation/capacity. For a replacement route callers
        // must supply count=0 on the growth branch. No rollback covers later
        // append failure. Do not make reserve clear as a readability change.
        if (Vec3Vector_GetCapacity(list) < (unsigned int)requiredCapacity) Vec3Vector_ReservePointCapacity(list, requiredCapacity); else Vec3Vector_Clear(list);

        PathPoint origin; CGolfDoc_GetCurrentHoleStartPosition(&origin); __as__7WVectorFRC7WVector(&start, &origin);
        {
            PathPoint groundPoint;
            PathPoint groundInput = start;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint, CQuadTree_GetInstance(), &groundInput, 0, 0, 0);
            float y=groundPoint.y;
            start.y = ((y + (float)kCameraGroundClearance));
        }
        __as__7WVectorFRC7WVector(&firstControlPoint, Vector_GetData(aimPoints));
        {
            PathPoint groundPoint;
            PathPoint groundInput = firstControlPoint;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint, CQuadTree_GetInstance(), &groundInput, 1, 0, 1);
            float y=groundPoint.y;
            firstControlPoint.y = (((float)kCameraGroundClearance + y));
        }
        {
            PathPoint combinedPosition,scaledOffset,directionDelta;
            __mi__FRC7WVectorRC7WVector(&directionDelta,&start,&firstControlPoint);
            __ml__FRC7WVectorf(&scaledOffset,Normalize__7WVectorFv(&directionDelta),(float)kCameraEndExtension);
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&start,&scaledOffset);
            __as__7WVectorFRC7WVector(&extensionPoint,&combinedPosition);
        }
        {
            PathPoint scaledOffset;
            PathPoint combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&extensionPoint,&start);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        Vec3Vector_AppendCopy(list,&start);
        {
            PathPoint scaledOffset,combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&start,&firstControlPoint);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        Vec3Vector_AppendCopy(list,&firstControlPoint);
        // Phase 2: outbound controls; every control is preceded by a lifted midpoint.
        int i;
        for (i=1; i < Vec3Vector_GetPointCount(aimPoints)-1; ++i) {
            void* next = Vec3Vector_GetPointAt(aimPoints,i);
            void* last = Vec3Vector_GetPointAt(aimPoints,i-1);
            {
                PathPoint scaledOffset,combinedPosition;
                __pl__FRC7WVectorRC7WVector(&combinedPosition,last,next);
                __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
                __as__7WVectorFRC7WVector(&pathPoint,&scaledOffset);
            }
            {
                PathPoint groundPoint,groundInput=pathPoint;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint,CQuadTree_GetInstance(),&groundInput,1,0,1);
                pathPoint.y=(((float)kCameraGroundClearance+groundPoint.y));
            }
            Vec3Vector_AppendCopy(list,&pathPoint);
            __as__7WVectorFRC7WVector(&pathPoint,Vec3Vector_GetPointAt(aimPoints,i));
            {
                PathPoint groundPoint;
                PathPoint groundInput=pathPoint;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint,CQuadTree_GetInstance(),&groundInput,1,0,1);
                pathPoint.y=(float)kCameraGroundClearance+groundPoint.y;
            }
            Vec3Vector_AppendCopy(list,&pathPoint);
        }
        // Phase 3: construct the far-end turn and its smoothing midpoints.
        // Turn around the last control point with a randomly chosen lateral offset.
        float direction = rand()%2 ? 1.0f : -1.0f;
        { void* last=Vec3Vector_GetPointAt(aimPoints,i-1); void* final=Vec3Vector_GetBack(aimPoints);
        PathPoint perpendicular,verticalAxis,directionDelta;
        __mi__FRC7WVectorRC7WVector(&directionDelta,final,last);
        Vector_CrossProduct(&perpendicular,Vec3_Set(&verticalAxis,0.0f,direction,0.0f),&directionDelta);
        __as__7WVectorFRC7WVector(&sidePoint,&perpendicular);
    }
        Normalize__7WVectorFv(&sidePoint); __amu__7WVectorFf(&sidePoint,(float)kCameraTurnWidth);
        {
            PathPoint combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,Vec3Vector_GetBack(aimPoints),&sidePoint);
            __as__7WVectorFRC7WVector(&farTurnCenter,&combinedPosition);
        }
        {
            PathPoint groundPoint;
            PathPoint groundInput=farTurnCenter;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint,CQuadTree_GetInstance(),&groundInput,0,0,0);
            float y=groundPoint.y;
            farTurnCenter.y=((float)kCameraGroundClearance+y);
        }
        __as__7WVectorFRC7WVector(&previousPoint,Vec3Vector_GetPointAt(aimPoints,i-1));
        {
            PathPoint groundPoint;
            PathPoint groundInput=previousPoint;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint,CQuadTree_GetInstance(),&groundInput,1,0,1);
            float y=groundPoint.y;
            previousPoint.y=(((float)kCameraGroundClearance+y));
        }
        {
            PathPoint scaledOffset;
            PathPoint directionDelta;
            __mi__FRC7WVectorRC7WVector(&directionDelta,&farTurnCenter,&previousPoint);
            __ml__FRC7WVectorf(&scaledOffset,Normalize__7WVectorFv(&directionDelta),(float)kCameraEndExtension);
            __as__7WVectorFRC7WVector(&pathPoint,&scaledOffset);
        }
        { PathPoint combinedPosition; __pl__FRC7WVectorRC7WVector(&combinedPosition,&farTurnCenter,&pathPoint); __as__7WVectorFRC7WVector(&extensionPoint,&combinedPosition); }
        {
            PathPoint combinedPosition,perpendicular,verticalAxis;
            Vector_CrossProduct(&perpendicular,Vec3_Set(&verticalAxis,0.0f,-direction,0.0f),&pathPoint);
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&farTurnCenter,&perpendicular);
            __as__7WVectorFRC7WVector(&sidePoint,&combinedPosition);
        }
        {
            PathPoint scaledOffset,combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&previousPoint,&farTurnCenter);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        Vec3Vector_AppendCopy(list,&farTurnCenter);
        {
            PathPoint scaledOffset,combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&farTurnCenter,&extensionPoint);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        Vec3Vector_AppendCopy(list,&extensionPoint);
        {
            PathPoint scaledOffset;
            PathPoint combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&extensionPoint,&sidePoint);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        Vec3Vector_AppendCopy(list,&sidePoint);
        {
            PathPoint scaledOffset;
            PathPoint combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&sidePoint,&previousPoint);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        Vec3Vector_AppendCopy(list,&previousPoint);
        // Phase 4: return controls, lifted onto the same terrain clearance.
        // Return along the control points in reverse order.
        for (int j=Vec3Vector_GetPointCount(aimPoints)-3; j>=0; --j) {
            void* current=Vec3Vector_GetPointAt(aimPoints,j);
            int savedJ = j+1;
            void* next=Vec3Vector_GetPointAt(aimPoints,savedJ);
            {
                PathPoint scaledOffset;
                PathPoint combinedPosition;
                __pl__FRC7WVectorRC7WVector(&combinedPosition,next,current);
                __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
                __as__7WVectorFRC7WVector(&pathPoint,&scaledOffset);
            }
            {
                PathPoint groundPoint,groundInput=pathPoint;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint,CQuadTree_GetInstance(),&groundInput,1,0,1);
                pathPoint.y=groundPoint.y+(float)kCameraGroundClearance;
            }
            Vec3Vector_AppendCopy(list,&pathPoint);
            __as__7WVectorFRC7WVector(&pathPoint,Vec3Vector_GetPointAt(aimPoints,j));
            {
                PathPoint groundPoint,groundInput=pathPoint;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint,CQuadTree_GetInstance(),&groundInput,1,0,1);
                float savedY = groundPoint.y;
                pathPoint.y=savedY+(float)kCameraGroundClearance;
            }
            Vec3Vector_AppendCopy(list,&pathPoint);
        }
        // Phase 5: randomized start-end turn, ending with the midpoint back toward start.
        {
            PathPoint scaledOffset,directionDelta;
            __mi__FRC7WVectorRC7WVector(&directionDelta,&start,&firstControlPoint);
            __ml__FRC7WVectorf(&scaledOffset,Normalize__7WVectorFv(&directionDelta),(float)kCameraEndExtension);
            __as__7WVectorFRC7WVector(&pathPoint,&scaledOffset);
        }
        { PathPoint combinedPosition; __pl__FRC7WVectorRC7WVector(&combinedPosition,&start,&pathPoint); __as__7WVectorFRC7WVector(&extensionPoint,&combinedPosition); }
        {
            float turnDirection=rand()%2 ? 1.0f : -1.0f;
            PathPoint combinedPosition;
            PathPoint perpendicular;
            PathPoint verticalAxis;
            float savedDirection = turnDirection;
            Vector_CrossProduct(&perpendicular,Vec3_Set(&verticalAxis,0.0f,savedDirection,0.0f),&pathPoint);
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&start,&perpendicular);
            __as__7WVectorFRC7WVector(&sidePoint,&combinedPosition);
        }
        {
            PathPoint scaledOffset,combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&firstControlPoint,&sidePoint);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        Vec3Vector_AppendCopy(list,&sidePoint);
        {
            PathPoint scaledOffset,combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&sidePoint,&extensionPoint);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        Vec3Vector_AppendCopy(list,&extensionPoint);
        {
            PathPoint scaledOffset,combinedPosition;
            __pl__FRC7WVectorRC7WVector(&combinedPosition,&extensionPoint,&start);
            __ml__FRC7WVectorf(&scaledOffset,&combinedPosition,0.5f);
            Vec3Vector_AppendCopy(list,&scaledOffset);
        }
        break;
    }
    case kSetSurfaceContext: { void* golfDocument=GolfDoc_GetCurrent(); CourseData_SetSurfaceHierarchyContext(AsCGolfDoc(golfDocument)->courseData,((const BackgroundMessage*)message)->payload.text); break; }
    case kLoadRankCourse: {
        void* task = 0;
        if (g_courseResourceListsEnabled) {
            Storage_ClearThreeWords(archivePathStorage);
            try {
            if (CGolfBg_BuildCourseRankListPath(actor, archivePathStorage)) {
                void* queue = MWString_GetCString(archivePathStorage);
                task = CPackedFile_Create(queue);
                if (task) while (!CPackedFile_PollLoad(task)) {}
            }
            } catch (...) {
                ((NString*)(archivePathStorage))->~NString();
                throw;
            }
            ((NString*)(archivePathStorage))->~NString();
        }
        unsigned int name = String_FormatTemporary(((const GolfBackgroundStringPool*)golfBackgroundStrings)->rankCourseBinaryFormat, SharedDoc_GetSelectedCourseMapName());
        void* golfDocument = GolfDoc_GetCurrent();
        if (!CourseModel_LoadFromFile(AsCGolfDoc(golfDocument)->courseData, (const char*)name)) {
            g_courseLoadFailed = 1;
            break;
        }
        golfDocument = GolfDoc_GetCurrent(); CourseData_ReleaseResources(AsCGolfDoc(golfDocument)->courseData, 0);
        golfDocument = GolfDoc_GetCurrent(); CourseData_BuildQuadTree(AsCGolfDoc(golfDocument)->courseData);
        GolfBg_DispatchCameraMessage2(actor);
        void* b = RenderPacket_GetShadowData(RenderPacket_GetInstance());
        CShadowManager_CopyNamedRecordSettings(ShadowManager_GetCurrent(), ((const GolfBackgroundStringPool*)golfBackgroundStrings)->sunRecord, b);
        const char* value=SharedDoc_GetSelectedCourseMapName();
        golfDocument = GolfDoc_GetCurrent(); CPvsFxBox_LoadSkyResources(AsCGolfDoc(golfDocument)->courseEffectBox, value, 0.0f);
        golfDocument = GolfDoc_GetCurrent(); CPvsFxBox_ReleaseParticleResource(AsCGolfDoc(golfDocument)->courseEffectBox);
        golfDocument = GolfDoc_GetCurrent(); CPvsFxBox_RebuildAuxiliaryEffect(AsCGolfDoc(golfDocument)->courseEffectBox);
        golfDocument = GolfDoc_GetCurrent(); CPvsFxBox_RebuildSceneEffects(AsCGolfDoc(golfDocument)->courseEffectBox);
        golfDocument = GolfDoc_GetCurrent(); CPvsFxBox_ReloadCourseEffects(AsCGolfDoc(golfDocument)->courseEffectBox);
        if (task) delete (PendingTask*)task;
        CGolfBg_CreateCourseObjects(SceneManager_GetCurrent());
        IActor_DispatchMessage(actor, actor, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->effectActor, 15, 0, 0, 0, 0);
        IActor_DispatchMessage(actor, actor, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->caddieActor, 14, 0, 0, 0, 0);
        break;
    }
    case kUpdateWindActors: {
        void* golfDocument = GolfDoc_GetCurrent();
        if (AsCGolfDoc(golfDocument)->courseData) { golfDocument=GolfDoc_GetCurrent(); CourseData_UpdateWindActors(AsCGolfDoc(golfDocument)->courseData); }
        break;
    }
    case kRequestCourseReload: CGolfBg_RequestCourseReload(actor, ((const BackgroundMessage*)message)->payload.integer); break;
    case kPollCourseReload: *(void**)((const BackgroundMessage*)message)->payload.object = (void*)(unsigned long)CGolfBg_FinalizeCourseReload(actor); break;
    default: break;
    }
}
#pragma inline_depth(8)

// Project the supplied tee position onto terrain using the golf ball ground context.
// unused is an unused receiver; input requires three world-space floats (+Y up).
// The selected hole record must exist. Writes start unconditionally and updates
// GolfBall+0x28 with the triangle ID, or -1 on a miss. GetGroundPoint (course_collision_and_ball_physics.cpp)
// always returns a vector: with flags 0,0 a miss uses its low-Y sentinel/floor
// fallback, then clamps Y to groundSurface.boundsMin.y. X/Z remain the input.
// A miss therefore publishes the floor result; it does not preserve the old tee,
// return failure, or leave result uninitialized. No raised-region clamp is used.
void CGolfDoc_SetCurrentHoleStartOnGround(void* unused, const void* input) {
    WVector result;
    unsigned int groundInput[3];
    groundInput[0] = ((const unsigned int*)input)[0];
    groundInput[1] = ((const unsigned int*)input)[1];
    groundInput[2] = ((const unsigned int*)input)[2];
    void* golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char holeIndex = AsCGolfDoc(golfDocument)->currentHoleIndex;
    void* holePositions = (HolePositions*)AsCGolfDoc(golfDocument)->holeRecords + holeIndex;
    void* object = GolfBall__Fv();
    void* a = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&result, a, groundInput, 0, (int)&((BallGroundQuery*)object)->context, 0);
    ((HolePositions*)holePositions)->start = result;
}

struct Vec3 { unsigned int x, y, z; };

// Ground a world position and notify the grid and golf-rule actors.
void CGolfBg_GroundAndDispatchAmbientPosition(void* self, void* input) {
    unsigned char result[12];
    Vec3 groundInput = *(Vec3*)input;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(result, CQuadTree_GetInstance(), &groundInput, 0, 0, 0);
    *(WVector*)input = *(WVector*)result;
    IActor_DispatchMessage(self, self, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->gridActor, 1, (int)input, 0, 0, 0);
    IActor_DispatchMessage(self, self, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->golfRuleActor, 2, (int)input, 0, 0, 0);
}

struct ScreenControl {
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual void v3(); virtual void v4(int);
};

// Finish a loaded course, reset party controls, and restore a temporarily saved round.
int CGolfBg_FinalizeCourseReload(void* self) {
    // courseReloadSettled is used as a completion latch here: 1 returns success immediately.
    // A nonempty pending filename means loading still runs; only then return 0.
    if (AsCGolfBg(self)->courseReloadSettled)
        return 1;
    if (!CourseModel_IsPendingFilenameEmpty(AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData))
        return 0;
    CGolfBg_InitializeCurrentHole(self);
    if (g_fx)
        CFx_SetUpdateEnabled(g_fx, 1);
    int mode = (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode);
    switch (mode) {
    case kDartsParty: {
        void* screen = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        void* control = TaskActorTable_FindByName(screen, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->dartsSystem);
        ((ScreenControl*)control)->v4(0);
        break;
    }
    case kDriveParty: {
        void* screen = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        void* control = TaskActorTable_FindByName(screen, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->driveSystem);
        ((ScreenControl*)control)->v4(0);
        break;
    }
    case kBalloonParty: {
        void* screen = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        void* control = TaskActorTable_FindByName(screen, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->balloonSystem);
        ((ScreenControl*)control)->v4(0);
        break;
    }
    }
    unsigned char holeIndex = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentHoleIndex;
    void* holePositions = (HolePositions*)AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->holeRecords + holeIndex;
    Vec3 groundInput = *(Vec3*)&((HolePositions*)holePositions)->cup;
    CupPreview_SetPosition(g_golfCupPreview, &groundInput, (void*)(unsigned long)groundInput.x);
    // Once resources are ready, restore all 18 saved order bytes, count and hole index.
    // Clear the restore marker only after selecting the saved hole.
    if (AsCGolfBg(self)->savedHoleCount) {
        for (int i = 0; i < kHoleCopyGroups; i++) {
            unsigned char base = i * kHolesPerCopyGroup;
            ((RoundHoleOrder*)g_sharedDoc)->holes[base + 0] = AsCGolfBg(self)->savedHoleOrder[base + 0];
            ((RoundHoleOrder*)g_sharedDoc)->holes[base + 1] = AsCGolfBg(self)->savedHoleOrder[base + 1];
            ((RoundHoleOrder*)g_sharedDoc)->holes[base + 2] = AsCGolfBg(self)->savedHoleOrder[base + 2];
            ((RoundHoleOrder*)g_sharedDoc)->holes[base + 3] = AsCGolfBg(self)->savedHoleOrder[base + 3];
            ((RoundHoleOrder*)g_sharedDoc)->holes[base + 4] = AsCGolfBg(self)->savedHoleOrder[base + 4];
            ((RoundHoleOrder*)g_sharedDoc)->holes[base + 5] = AsCGolfBg(self)->savedHoleOrder[base + 5];
        }
        unsigned char flag = AsCGolfBg(self)->savedHoleCount;
        AsCSharedDoc(g_sharedDoc)->roundHoleCount = flag;
        CGolfDoc_SetCurrentHole(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), AsCGolfBg(self)->savedHoleIndex);
        AsCGolfBg(self)->savedHoleCount = 0;
        AsCGolfBg(self)->savedHoleIndex = 0;
    }
    AsCGolfBg(self)->courseReloadSettled = 1;
    return 1;
}

// Project the supplied cup position onto terrain and save it for the current hole.
void CGolfDoc_SetCurrentHoleCupOnGround(void* unused, const void* input) {
    WVector result;
    unsigned int groundInput[3];
    groundInput[0] = ((const unsigned int*)input)[0];
    groundInput[1] = ((const unsigned int*)input)[1];
    groundInput[2] = ((const unsigned int*)input)[2];
    void* golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned char holeIndex = AsCGolfDoc(golfDocument)->currentHoleIndex;
    void* holePositions = (HolePositions*)AsCGolfDoc(golfDocument)->holeRecords + holeIndex;
    void* a = CQuadTree_GetInstance();
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&result, a, groundInput, 0, 0, 0);
    ((HolePositions*)holePositions)->cup = result;
}

// Load a requested course, temporarily replacing the round order for named course variants.
// mode is a SharedStringState pair selector, not a course ID or party mode:
// 0 = primary pair, 1 = secondary pair (GetSecondString, story_event_script_shared_state.cpp). A nonempty
// second string selects the named-course format; its suffix must be _rank or a
// valid one-based hole number. 3 explicitly requests selected map/current hole
// and bypasses string lookup. Other integers also fall back because that getter
// returns null outside 0/1; no additional mode meaning is established.
// Filename equality leaves loading untouched. Changed named variants save all
// 18 holes/count/index for FinalizeCourseReload to restore. Failed load latches
// g_courseLoadFailed, leaving settled=1; success clears settled until completion.
// No input or temporary filename pointer transfers ownership.
void CGolfBg_RequestCourseReload(void* self, int mode) { int name; 
    AsCGolfBg(self)->courseReloadSettled = 1;
    AsCGolfBg(self)->savedHoleIndex = 0;
    AsCGolfBg(self)->savedHoleCount = 0;

    // Phase 1: select a named variant only when mode != 3 and its second string exists.
    // Preserve the short-circuit predicate: the lookup must not run for mode 3.
    // savedHoleCount is both the old count and the later restore-present marker.
    if (!(!(mode != 3 && SharedStringState_GetSecondString(mode)))) {
        name = String_FormatTemporary(((const GolfBackgroundStringPool*)golfBackgroundStrings)->namedCourseBinaryFormat, SharedStringState_GetSecondString(mode));
        void* golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        AsCGolfBg(self)->savedHoleCount = AsCSharedDoc(g_sharedDoc)->roundHoleCount;
        AsCGolfBg(self)->savedHoleIndex = AsCGolfDoc(golfDocument)->currentHoleIndex;
        for (int j = 0; j < kHoleCopyGroups; ++j) {
            unsigned char i = kHolesPerCopyGroup * j;
            AsCGolfBg(self)->savedHoleOrder[i + 0] = ((RoundHoleOrder*)g_sharedDoc)->holes[i];
            AsCGolfBg(self)->savedHoleOrder[i + 1] = ((RoundHoleOrder*)g_sharedDoc)->holes[i + 1];
            AsCGolfBg(self)->savedHoleOrder[i + 2] = ((RoundHoleOrder*)g_sharedDoc)->holes[i + 2];
            AsCGolfBg(self)->savedHoleOrder[i + 3] = ((RoundHoleOrder*)g_sharedDoc)->holes[i + 3];
            AsCGolfBg(self)->savedHoleOrder[i + 4] = ((RoundHoleOrder*)g_sharedDoc)->holes[i + 4];
            AsCGolfBg(self)->savedHoleOrder[i + 5] = ((RoundHoleOrder*)g_sharedDoc)->holes[i + 5];
        }
    } else {
        void* state = g_sharedDoc;
        void* arg = AsCSharedDoc(state)->selectedCourseData;
        name = String_FormatTemporary(((const GolfBackgroundStringPool*)golfBackgroundStrings)->courseBinaryFormat, arg ? ((IFF_STRUCT::sCourse*)arg)->mapName : 0,
                          ((CourseHoleSelection*)AsCSharedDoc(state)->golfDoc)->holeNumber);
    }
    // Phase 2: only a changed filename installs the temporary 1..18 order and starts loading.
    // Three six-byte copy groups below cover the entire round order (18 bytes).
    if (!CourseModel_MatchesFilename(AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData, name)) {
        if (AsCGolfBg(self)->savedHoleCount) {
            for (int j = 0; j < kHoleCopyGroups; ++j) {
                unsigned char i = j * kHolesPerCopyGroup;
                ((RoundHoleOrder*)g_sharedDoc)->holes[i] = (i + 1);
                ((RoundHoleOrder*)g_sharedDoc)->holes[i + 1] = i + 2;
                ((RoundHoleOrder*)g_sharedDoc)->holes[i + 2] = i + 3;
                ((RoundHoleOrder*)g_sharedDoc)->holes[i + 3] = i + 4;
                ((RoundHoleOrder*)g_sharedDoc)->holes[i + 4] = i + 5;
                ((RoundHoleOrder*)g_sharedDoc)->holes[i + 5] = (i + 6);
            }
            AsCSharedDoc(g_sharedDoc)->roundHoleCount = kFullRoundHoleCount;
            // Named variants must contain an underscore. Suffix rank selects hole index 0;
            // a decimal suffix is converted from one-based file naming to zero-based hole index.
            const char* key = strrchr((const char*)SharedStringState_GetSecondString(mode), '_') + 1;
            if (strcmp(key, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->rankSuffix))
                CGolfDoc_SetCurrentHole(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)(String_ParseDecimalInt(key) - 1));
            else
                CGolfDoc_SetCurrentHole(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
        }
        IActor_DispatchMessage(self, self, ((const GolfBackgroundStringPool*)golfBackgroundStrings)->ambientActor, 2, 0, 0, 0, 0);
        SoundManager_StopAndClearChannels(g_soundManager);
        if (g_fx) {
            CFx_SetUpdateEnabled(g_fx, 0);
            CFx_ClearControlsAndResetBox(g_fx);
        }
        if (!CourseModel_SetFileAndLoad(AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->courseData, name))
            g_courseLoadFailed = 1;
        else
            AsCGolfBg(self)->courseReloadSettled = 0;
    }
}

} // extern "C"
