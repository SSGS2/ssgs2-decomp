#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include <decomp/ct_member.h>
typedef unsigned long new_size_t;
#include "classes/CourseData801C.h"
#include "fp_fused.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CSharedDoc.h"
#include "game/golfball.h"
#define AsCQuadTree(p) ((CQuadTree*)(p))
#include "classes/CQuadTree.h"
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/WView.h"
#include "classes/InputPointerState.h"
#include "classes/CTask.h"
#define AsWView(p) ((WView*)(p))
// Prefix of the course object string pool used by bounds lookup and vector exceptions.
struct CourseObjectStringPrefix {
    char vectorLengthError[20];
    char lengthErrorType[55];
    char centerPointName[5];
};
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const CourseObjectStringPrefix courseObjectSoundStrings;
}
}

#include "game/wvector.h"
#include "game/chatmsg_match.h"
// Translation unit 0x801CAB90-0x801CC2C0 (20 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Resource names and pointer actions share one packed byte pool.
struct GolfTaskStringPool {
    char empty[1];
    char permanentSoundBank[13];
    char menuSoundBank[8];
    char titleTask[11];
    char titleEntry[11];
    char pauseAction[6]; // Japanese action name meaning stop/pause, variant 2.
    char debugInfoAction[11];
    char inputProvider[12];
    char keyboardDevice[9];
    char mouseDevice[6];
    char windowProvider[7];
    char windowCreationFailure[375];
};
// Course bounds are 0x5C-byte records; named points store XYZ at +0x24.
struct CourseBoundsRecord {
    unsigned char type;
    char reserved[0x43];
    unsigned int boundsWords[6];
};
struct CourseNamedPoint { char prefix[0x24]; unsigned int positionWords[3]; };
struct HolePositionRecord {
    char prefix[0x0C];
    unsigned int positionWords[3];
    char suffix[8];
};
// Supplemental fields that the shared CQuadTree header still represents as padding.
struct QuadTreeGroundState {
    char prefix[0x20B4];
    unsigned int overridePositionWords[3];
    char reserved20C0[0x14];
    int groundRegionIndex;
    char reserved20D8[0x8C];
    unsigned char resetFlag; // Reset here; full gameplay meaning still needs research.
    char reserved2165[3];
    float cupRadius;
    float recoveryHeight;
    unsigned int resetCounter;
    char reserved2174[0x34];
    float objectAnimationFraction;
    int objectAnimationTick;
};
struct ActorTypeDescriptor { const char* name; ActorTypeDescriptor* base; };
// These views must retain their binary layout when edited.
typedef char CourseBoundsRecord_size[(sizeof(CourseBoundsRecord) == 0x5C) ? 1 : -1];
typedef char HolePositionRecord_size[(sizeof(HolePositionRecord) == 0x20) ? 1 : -1];
typedef char QuadTreeGroundState_recoveryHeight[((int)&((QuadTreeGroundState*)0)->recoveryHeight == 0x216C) ? 1 : -1];
typedef char QuadTreeGroundState_animationTick[((int)&((QuadTreeGroundState*)0)->objectAnimationTick == 0x21AC) ? 1 : -1];
typedef char GolfTaskStringPool_window[((int)&((GolfTaskStringPool*)0)->windowProvider == 0x58) ? 1 : -1];
enum { kProjectWindowReady = 4, kGroundOverrideBoundsType = 7,
       kFloatSignMask = 0x80000000u, kWindowErrorIcon = 0x10,
       kDiagnosticDurationMs = 1000, kWhiteRgb = 0x00FFFFFF,
       kDiagnosticTextColor = 0xFFFF0000u, kDeviceListMutexOffset = 0x88 };

extern "C" {

extern void Rect_GetScreenBounds(void*, void*, void*, void*, void*);
extern int Screen_GetDimensionByAxis(int);
extern void Rect_Offset(void*, int, int);
extern void* Generic3DPool_CreateFromNamedSource(void*, int, const char*, int);
extern void MessageBox_Report(void*, const char*, void*, int);
extern void Vec3_Zero(void*);
extern void AABB_Copy(void*, void*);
extern void* CourseData_FindNamedPoint(void*, const char*);
// GetGroundPoint has the PowerPC-shaped signature (return slot, self, pointer to the point, ...); the by-value
// calls go through a helper.
extern WVector GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, WVector, bool, int*, bool);
#define GetGroundPoint__9CQuadTreeF7WVectorbPib_BV GetGroundPoint__9CQuadTreeF7WVectorbPib
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void* GolfBall__Fv(void);
extern void* g_legacyWindowContext;
extern void* g_resourceManager;
extern unsigned int g_currentTimeMilliseconds;
extern CSharedDoc* g_sharedDoc;
extern float kGroundOverrideHoleRadiusScale;
extern const float kFloat_0_805DF744;
extern char kProjectWindowDefaultRectangleWords[]; // Four default window rectangle words, copied before querying screen bounds.
extern const CourseObjectStringPrefix courseObjectSoundStrings;
extern GolfTaskStringPool golfTaskStrings;
extern unsigned int s_quadTreePerShotResetWord; // Shared collision/recovery word cleared on ground and per-shot resets.

extern InputPointerState* g_inputPointer;
extern CTask* g_taskManager;
extern void* g_mouseCursor;
extern void* g_fx;
extern void* g_soundManager;
extern WView* g_view;
extern unsigned int g_applicationUpdateCount;
extern void* g_pointerDeviceAdapter;
extern void* g_bindingDeviceAdapter;
extern ActorTypeDescriptor s_golfTaskTypeInfo;
extern unsigned char g_debugPauseEnabled;
extern unsigned int g_timeScaleCheckedTask;
extern unsigned int g_drawSceneDebugGeometry;
extern unsigned char g_courseLoadFailed;
extern unsigned char s_lastMouseInputEnabled;
extern const float kDefaultProjectTimeScale;
extern float kFloat_0_Scene;
extern float s_projectTimeScaleFloat_1;
extern const float kProjectTimeScaleCompareEpsilon;
extern unsigned char g_mouseInputEnabled;
extern void Pointer_Update(void*);
extern int GolfTask_NoOpStatus(); // Wii stub returns zero: restore the default time scale.
extern int Pointer_GetRisingActionBits(void*, const char*, int);
extern void* Object_GetChild(void*);
extern void* CTask_ProcessPendingTransition(void*);
extern int DiskDialog_ShouldBlockUpdate(); // Wii disk-dialog block-update stub returns zero.
extern void CMouseCursor_Update(void*, float);
extern void CTask_UpdateCurrentTaskWithFadeGates(void*, float); // Update the current task through fade/loading gates.
extern void SoundManager_UpdatePlayback(void*, void*, float);
extern void Audio_UpdateStreamVolumeFade(float); // Advance the master-volume fade tween.
extern void Application_RequestExit(void*); // Set the application exit-request word.

struct ProjectUpdateState { char prefix[0x64]; float timeScale; };
// Poll mapped input, apply debug pause/time scaling, and update tasks, effects and audio.
void CProjectG_Update(ProjectUpdateState* self, float delta) {
	// The pointer abstraction owns hardware sampling; pause/debug actions are mapped names.
	Pointer_Update(g_inputPointer);
	if (GolfTask_NoOpStatus() == 0)
		self->timeScale = kDefaultProjectTimeScale;
	if (Pointer_GetRisingActionBits(g_inputPointer, golfTaskStrings.pauseAction, 1))
		g_debugPauseEnabled = (unsigned char)(0 == g_debugPauseEnabled);
	if (g_debugPauseEnabled)
		self->timeScale = kFloat_0_Scene;
	// Time-scale gate in plain C: if (fabsf(timeScale - 1.0f) >= epsilon) { the scale is
	// non-default; unless the active child task's type chain includes the golf task type,
	// reset timeScale to the default }. So only the golf task may run at a non-default speed.
	// (The sign-bit test below is an inlined fabsf through float/integer aliasing; the
	// loop walks the task's type-descriptor `base` chain, a hand-written is-a check.)
	float difference = self->timeScale - s_projectTimeScaleFloat_1;
	unsigned int bits;
	*(float*)&bits = difference;
	if (bits & kFloatSignMask) difference = -difference;
	int isDefaultScale = difference < kProjectTimeScaleCompareEpsilon;
	if (!isDefaultScale) {
		void* object = g_taskManager ? Object_GetChild(g_taskManager) : 0;
		g_timeScaleCheckedTask = (unsigned int)object;
		int isGolfTask = 0;
		if (object) {
			struct TaskTypeInfoInterface { 
			virtual void* Get() DECOMP_LOCAL_VIRTUAL;
			};
			ActorTypeDescriptor* node = (ActorTypeDescriptor*)((TaskTypeInfoInterface*)object)->Get();
			ActorTypeDescriptor* head = &s_golfTaskTypeInfo;
			for (ActorTypeDescriptor* n = node; n; n = n->base)
				if (n == head) { isGolfTask = 1; break; }
		}
		if (!isGolfTask) self->timeScale = kDefaultProjectTimeScale;
	}
	// Keep processing transitions and audio even when gameplay time is paused.
	float scaledDelta = delta * self->timeScale;
	CTask_ProcessPendingTransition(g_taskManager);
	if (!DiskDialog_ShouldBlockUpdate()) {
		CMouseCursor_Update(g_mouseCursor, scaledDelta);
		CTask_UpdateCurrentTaskWithFadeGates(g_taskManager, scaledDelta);
		struct FxUpdateInterface { 
		virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual void v1() DECOMP_LOCAL_VIRTUAL; virtual void v2() DECOMP_LOCAL_VIRTUAL; virtual void v3() DECOMP_LOCAL_VIRTUAL; virtual void v4() DECOMP_LOCAL_VIRTUAL; virtual void Update(float) DECOMP_LOCAL_VIRTUAL;
		};
		((FxUpdateInterface*)g_fx)->Update(scaledDelta);
	}
	SoundManager_UpdatePlayback(g_soundManager, g_view, scaledDelta);
	Audio_UpdateStreamVolumeFade(scaledDelta);
	if (Pointer_GetRisingActionBits(g_inputPointer, golfTaskStrings.debugInfoAction, 1))
		g_drawSceneDebugGeometry = !g_drawSceneDebugGeometry;
	if (g_courseLoadFailed) Application_RequestExit(self);
	++g_applicationUpdateCount;
	unsigned char old = g_mouseInputEnabled;
	if (s_lastMouseInputEnabled != old) {
		s_lastMouseInputEnabled = old;
		struct PointerWindowInterface { 
		virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual void v1() DECOMP_LOCAL_VIRTUAL; virtual void v2() DECOMP_LOCAL_VIRTUAL; virtual void v3() DECOMP_LOCAL_VIRTUAL; virtual void Update(void*) DECOMP_LOCAL_VIRTUAL;
		};
		((PointerWindowInterface*)g_pointerDeviceAdapter)->Update(g_legacyWindowContext);
	}
}

struct ProjectWindowState { char prefix[0xA0]; unsigned int flags; char padA4[0x0C]; void* object; void* list; };
struct DeviceRegistrationEntry { void* context; void* object; };
struct DeviceRegistrationList { DeviceRegistrationEntry entries[16]; unsigned int count; };
struct DeviceRegistrationLink { void* reserved; void* owner; };

// Create the window provider, register its interface and report initialization failure.
int CProjectG_CreateWindowProvider(ProjectWindowState* self) {
	int rect[20];
	int* src = (int*)kProjectWindowDefaultRectangleWords;
	rect[0] = *src++;
	rect[2] = *src++;
	rect[1] = *src++;
	rect[3] = *src++;
	for (int i = 4; 20 > i; ++i) rect[i] = 0;

	Rect_GetScreenBounds(rect, 0, 0, 0, 0); int x = Screen_GetDimensionByAxis(0) - (rect[2] - rect[0]);
	int y = Screen_GetDimensionByAxis(1) - (rect[3] - rect[1]);
	__typeof__(self->object) savedObject = self->object; Rect_Offset(rect, (x + (int)((unsigned int)x >> 31)) >> 1, (y + (int)((unsigned int)y >> 31)) >> 1);
	void* obj = Generic3DPool_CreateFromNamedSource(savedObject, 0, golfTaskStrings.windowProvider, 0);
	if (obj == 0) {
		MessageBox_Report(g_legacyWindowContext, golfTaskStrings.windowCreationFailure, ((rect+4)), kWindowErrorIcon);
		return 0;
	}
	self->flags = kProjectWindowReady | self->flags;
	void* entry = ((void* (*)(void*))(*(void***)obj)[5])(obj);
	if (entry) {
		void* list = self->list; __typeof__(self->list) savedList = self->list;
		((DeviceRegistrationList*)list)->entries[((DeviceRegistrationList*)list)->count].context = g_legacyWindowContext;
		((DeviceRegistrationLink*)entry)->owner = savedList;
		((DeviceRegistrationList*)list)->entries[((DeviceRegistrationList*)list)->count].object = entry;
		((DeviceRegistrationList*)list)->count = ((DeviceRegistrationList*)list)->count + 1;
	}
	*(void**)g_resourceManager = obj;
	return 1;
}

struct QuadTreeBoundsPrefix { char prefix[0x209C]; };
struct GroundPositionWords {
    unsigned int x, y, z;
    GroundPositionWords(const unsigned int* p) : x(p[0]), y(p[1]), z(p[2]) {}
};
// Load override bounds, choose the named center or hole position, and reset ground simulation state.
#pragma push
#pragma opt_common_subs off
void CQuadTree_InitGroundOverrideBounds(CQuadTree* self) {
	Vec3_Zero(&AsCQuadTree(self)->greenBoxMinX); // original +0x209C
	Vec3_Zero(&AsCQuadTree(self)->greenBoxMaxX); // original +0x20A8
	CourseData801C* course = (CourseData801C*)self->courseData;
	int count = course->boundsCount;
	int i;
	for (i=0; i<count; ++i) {
		void* manager = course;
		CourseBoundsRecord* bounds = (CourseBoundsRecord*)((CourseData801C*)manager)->bounds;
		if (bounds[i].type == kGroundOverrideBoundsType) {
			AABB_Copy(&AsCQuadTree(self)->greenBoxMinX, bounds[i].boundsWords); // original +0x209C
			break;
		}
	}
	void* found = CourseData_FindNamedPoint(((CourseData801C*)AsCQuadTree(self)->courseData), courseObjectSoundStrings.centerPointName);
	// GetGroundPoint returns a WVector through a hidden pointer: the original passes a stack
	// temporary as r3 (0x801CC170 addi r3,r1,0x44 / 0x801CC194 addi r3,r1,0x2c) and then
	// copies it into +0x20B4. It never writes this+0x44 or this+0x2C.
	if (found) {
		*(WVector*)((QuadTreeGroundState*)self)->overridePositionWords =
            GetGroundPoint__9CQuadTreeF7WVectorbPib_BV(self, *(WVector*)((CourseNamedPoint*)found)->positionWords, false, 0, false);
	} else {
		// g_sharedDoc holds the object pointer itself: 0x801CC18C lwz r6,g_sharedDoc then
		// 0x801CC19C lwz r9,0xb90(r6) (no extra dereference).
		void* mgr = ((CGolfDoc*)g_sharedDoc->golfDoc);
		unsigned int idx = AsCGolfDoc(mgr)->currentHoleIndex;
		int* v = (int*)((HolePositionRecord*)AsCGolfDoc(mgr)->holeRecords)[idx].positionWords;
		GroundPositionWords p((const unsigned int*)v);
		*(WVector*)((QuadTreeGroundState*)self)->overridePositionWords =
            GetGroundPoint__9CQuadTreeF7WVectorbPib_BV(self, *(WVector*)&p, false, 0, false);
	}
	void* info = GolfBall__Fv();
	void* mgr = ((CGolfDoc*)g_sharedDoc->golfDoc); // 0x801CC1F4 lwz r6,lbl / 0x801CC204 lwz r8,0xb90(r6), no extra dereference
	int* v = (int*)((HolePositionRecord*)AsCGolfDoc(mgr)->holeRecords)[AsCGolfDoc(mgr)->currentHoleIndex].positionWords;
	float f = ((CGolfBall*)info)->m_radius; // original +0xC0
	GroundPositionWords hole((const unsigned int*)v);
	// 0x801CC1FC lwz r7,0x21ec(r31) / subf / 0x801CC258 stw r3,0x21ec(r31): only +0x21EC is
	// cleared; the original never stores to +0x21E4.
	int collisionCount = self->collisionEventCount;
	self->collisionEventCount -= collisionCount;
	s_quadTreePerShotResetWord = 0;
	((QuadTreeGroundState*)self)->objectAnimationFraction = kFloat_0_805DF744;
	((QuadTreeGroundState*)self)->objectAnimationTick=0;
	((QuadTreeGroundState*)self)->recoveryHeight=FP_FNMSUBS(kGroundOverrideHoleRadiusScale, f, *(float*)&hole.y);
	(*(int*)&AsCQuadTree(self)->effectRandomState)=0;
	((QuadTreeGroundState*)self)->groundRegionIndex=-1;
	((QuadTreeGroundState*)self)->resetFlag=0;
	AsCQuadTree(self)->regionBounceCount=0;
	((QuadTreeGroundState*)self)->resetCounter=0;
	AsCQuadTree(self)->candidateTriangleOverflow=0;
	AsCQuadTree(self)->windEnabled=1;
}
#pragma pop

// Legacy input polling hook; this provider has no device sampling work here.
void WInputDev_Update() {}

extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);
extern void __dt__13WInstanceLockFv(void*);
extern WView* g_view;
extern unsigned char g_mouseInputEnabled;
extern int g_viewportWidthPixels;
extern int g_viewportHeightPixels;
extern signed char s_quadTreeInitialized;
extern void CQuadTree_Ctor(void*);
extern void* CQuadTree_dtor(void*, int);
extern void __register_global_object(void*, void* (*)(void*, int), char*);
extern char s_quadTreeDtorRegistration[];
extern char s_quadTreeStorage[];

extern void SoundManager_AcquireSoundBank(void*, const char*);
extern void* __nw__FUl(unsigned int);
extern void SoundManager_Ctor(void*);
extern void* g_soundManager;
extern GolfTaskStringPool golfTaskStrings;

struct SoundManagerAllocation { char storage[0x5C]; /* Construct the sound manager in its owned allocation. */ SoundManagerAllocation() { SoundManager_Ctor(this); } };
int Audio_CreateGlobalSoundManager(void*);

extern void* Generic3DPool_CreateFromFactory(void*, const char*, const char*);
extern void* appMallocUni__FUl(unsigned int);

// These locks recurse on the SAME OS mutex. Each guard balances one acquisition, including unwinds.
struct InstanceLock_u801CAB90 {
    void* p;
    // Acquire one recursive level for the duration of the guarded pool/list operation.
    InstanceLock_u801CAB90(void* p_) : p(p_) { OSLockMutex(p); }
    // Release exactly the recursive level acquired by this guard.
    ~InstanceLock_u801CAB90() { OSUnlockMutex(p); }
};
// Three nullable circular rings share 0x18-byte nodes, with prev/next at +0xC/+0x10.
// allocatedBlocks links each allocation's FIRST node; nodes 1..nodesPerBlock-1 enter freeNodes.
// A free node is removed before device/context/state are filled and it enters activeDevices.
// Ring insertion goes before the saved head; singleton removal sets the ring pointer to null.
// The first node is a block header, not a usable device slot: nodesPerBlock must exceed 1.
struct ProjectDeviceState { char prefix[0x68]; void* activeDevices; char pad6C[4]; void* allocatedBlocks; void* freeNodes; int nodesPerBlock; char pad7C[0x0C]; char mutex[0x28]; void* deviceFactory; void* registrations; };
struct ProjectDeviceNode { void* device; void* context; unsigned char state; char pad09[3]; ProjectDeviceNode* prev; ProjectDeviceNode* next; char pad14[4]; };
// Virtual slots 0..2 are unresolved; slot 3 borrows the registration/callback
// interface, which can be null. It does not allocate a new device.
struct InputDeviceInterface { 
virtual void v0(); virtual void v1(); virtual void v2(); virtual void* Method();
};
// Borrow the factory device interface used by the project callback registry.
static inline void* BorrowDeviceRegistration(void* device) {
    return ((InputDeviceInterface*)device)->Method();
}
#pragma opt_common_subs off
// Create legacy keyboard and mouse devices, register their callbacks and initialize audio.
// Returns 0 only on window-provider failure, 1 after device/audio setup. Registration entries
// borrow interfaces from factory devices; pool nodes own no additional device allocation here.
// Registration policy: each device's interface (slot 3 of the created object) is appended to
// self->registrations with the shared window context; the device object itself is added to the
// active ring (activeDevices) with state 0 / context 0. Keyboard (Phases 1-2) and mouse
// (Phase 3) are the same sequence twice, differing only in the factory name string and the
// global that receives the adapter (g_bindingDeviceAdapter vs g_pointerDeviceAdapter).
// Pool node mechanics repeated inside each phase (self->mutex guards every ring edit):
//   ring insert : empty ring -> node links to itself; else splice the node before the head
//   reserve     : if no free node, malloc one block of nodesPerBlock 0x18-byte nodes, put node 0
//                 on allocatedBlocks and the rest on freeNodes
//   claim       : unlink the head of freeNodes, set device/state/context, insert on activeDevices
int CProjectG_InitializeLegacyDevices(ProjectDeviceState* self) {
	// Device policy: window first; keyboard -> binding adapter; mouse -> pointer adapter;
	// then audio. Both devices use the same input provider and a null context/state=0
	// in the active ring. A null registration skips callbacks, not active-device insertion.
	// Phase 1: create the window, then publish/register the keyboard adapter's callback interface.
	if (!CProjectG_CreateWindowProvider((ProjectWindowState*)self)) return 0;
	void* createdDevice=Generic3DPool_CreateFromFactory(self->deviceFactory,golfTaskStrings.inputProvider,golfTaskStrings.keyboardDevice);
	g_bindingDeviceAdapter=createdDevice;
	void* registration=BorrowDeviceRegistration(createdDevice);
	if (registration) {
		void* windowContext=g_legacyWindowContext;
		void* registrationOwner=self->registrations;
		((DeviceRegistrationLink*)registration)->owner=registrationOwner;
		DeviceRegistrationList* registrations=(DeviceRegistrationList*)registrationOwner;
		registrations->entries[registrations->count].context=windowContext;
		registrations->entries[registrations->count].object=registration;
		registrations->count=registrations->count+1;
	}
	void* keyboardForPool=g_bindingDeviceAdapter;
	{
		// Phase 2: keyboard pool append. Grow/register a block if needed, pop one free node,
		// then append it to the active ring. Nested locks reflect the original library layers.
		ProjectDeviceNode* node;
		InstanceLock_u801CAB90 lockA(self->mutex);
		{
			InstanceLock_u801CAB90 lockB(self->mutex);
			// Reserve: retain allocation node 0 in allocatedBlocks; publish only nodes 1..N-1.
			if (0 == self->freeNodes) {
				node=(ProjectDeviceNode*)appMallocUni__FUl(self->nodesPerBlock*0x18);
				char* ringMutex=self->mutex;
				__typeof__(self->allocatedBlocks) savedBlockHead = self->allocatedBlocks; OSLockMutex(ringMutex);
				ProjectDeviceNode* blockHead=(ProjectDeviceNode*)savedBlockHead;
				if (blockHead == 0) {
					node->prev=node; node->next=node;
					OSUnlockMutex(ringMutex);
					blockHead=node;
				} else {
					node->next=blockHead; node->prev=blockHead->prev; blockHead->prev=node; node->prev->next=node;
					OSUnlockMutex(ringMutex);
				}
				self->allocatedBlocks=blockHead;
				for (int i=1;i<self->nodesPerBlock;i++) {
					ProjectDeviceNode* freeHead=(ProjectDeviceNode*)self->freeNodes;
					node=node + 1;
					char* freeRingMutex=self->mutex;
					OSLockMutex(freeRingMutex);
					if (freeHead == 0) {
						node->prev=node; node->next=node;
						OSUnlockMutex(freeRingMutex);
						freeHead=node;
					} else {
						node->next=freeHead; node->prev=freeHead->prev; freeHead->prev=node; node->prev->next=node;
						OSUnlockMutex(freeRingMutex);
					}
					self->freeNodes=freeHead;
				}
			}
			// Claim: unlink one free node; nextFree becomes null when the ring was singleton.
			ProjectDeviceNode* nextFree;
			{
				char* ringMutex=self->mutex; __typeof__(self->freeNodes) savedFreeHead = self->freeNodes;
				OSLockMutex(ringMutex);
				node=(ProjectDeviceNode*)savedFreeHead;
				node->next->prev=node->prev;
				node->prev->next=node->next;
				nextFree=node->next;
				if (nextFree==node) nextFree=0;
				OSUnlockMutex(ringMutex);
			}
			self->freeNodes=nextFree;
		}
		// The node has left the free ring; initialize payload before exposing it as active.
		node->device=keyboardForPool;
		node->state=0;
		node->context=0;
		char* ringMutex=self->mutex;
		ProjectDeviceNode* activeHead=(ProjectDeviceNode*)self->activeDevices;
		OSLockMutex(ringMutex);
		if (activeHead == 0) {
			node->prev=node; node->next=node;
			OSUnlockMutex(ringMutex);
			activeHead=node;
		} else {
			node->next=activeHead; node->prev=activeHead->prev; activeHead->prev=node; node->prev->next=node;
			OSUnlockMutex(ringMutex);
		}
		self->activeDevices=activeHead;
	}
	// Phase 3: publish/register the mouse adapter, then perform the same pool append as above.
	createdDevice=Generic3DPool_CreateFromFactory(self->deviceFactory,golfTaskStrings.inputProvider,golfTaskStrings.mouseDevice); g_pointerDeviceAdapter=createdDevice;
	registration=BorrowDeviceRegistration(createdDevice);
	if (registration) {
		void* windowContext=g_legacyWindowContext;
		void* registrationOwner=self->registrations;
		DeviceRegistrationList* registrations=(DeviceRegistrationList*)registrationOwner;
		((DeviceRegistrationLink*)registration)->owner=registrationOwner;
		registrations->entries[registrations->count].context=windowContext;
		registrations->entries[registrations->count].object=registration;
		registrations->count=registrations->count+1;
	}
	void* mouseForPool=g_pointerDeviceAdapter;
	{
		ProjectDeviceNode* node;
		InstanceLock_u801CAB90 lockC(self->mutex);
		{
			InstanceLock_u801CAB90 lockD(self->mutex);
			// Reserve the mouse slot with the same block-header/free-node split as keyboard.
			if (!self->freeNodes) {
				node=(ProjectDeviceNode*)appMallocUni__FUl(self->nodesPerBlock*0x18);
				char* ringMutex=self->mutex;
				ProjectDeviceNode* blockHead=(ProjectDeviceNode*)self->allocatedBlocks;
				OSLockMutex(ringMutex);
				if (blockHead == 0) {
					node->prev=node; node->next=node;
					OSUnlockMutex(ringMutex);
					blockHead=node;
				} else {
					node->next=blockHead; node->prev=blockHead->prev; blockHead->prev=node; node->prev->next=node;
					OSUnlockMutex(ringMutex);
				}
				self->allocatedBlocks=blockHead;
				for (int i=1;i<self->nodesPerBlock;i++) {
					ProjectDeviceNode* freeHead=(ProjectDeviceNode*)self->freeNodes;
					node=node + 1;
					char* freeRingMutex=self->mutex;
					OSLockMutex(freeRingMutex);
					if (freeHead == 0) {
						node->prev=node; node->next=node;
						OSUnlockMutex(freeRingMutex);
						freeHead=node;
					} else {
						node->next=freeHead; node->prev=freeHead->prev; freeHead->prev=node; node->prev->next=node;
						OSUnlockMutex(freeRingMutex);
					}
					self->freeNodes=freeHead;
				}
			}
			// Claim: unlink one free node; nextFree becomes null when the ring was singleton.
			ProjectDeviceNode* nextFree;
			{
					__typeof__(self->freeNodes) savedFreeHead = self->freeNodes; char* ringMutex=self->mutex;
				OSLockMutex(ringMutex);
					node=(ProjectDeviceNode*)savedFreeHead;
						node->next->prev=node->prev;
						node->prev->next=node->next;
				nextFree=node->next;
				if (nextFree==node) nextFree=0;
				OSUnlockMutex(ringMutex);
			}
			self->freeNodes=nextFree;
		}
		// Publish only after removing the node from freeNodes and initializing its payload.
		node->device=mouseForPool;
		node->state=0;
		node->context=0;
		char* ringMutex=self->mutex;
		ProjectDeviceNode* activeHead=(ProjectDeviceNode*)self->activeDevices;
		OSLockMutex(ringMutex);
		if (!activeHead) {
			node->prev=node; node->next=node;
			OSUnlockMutex(ringMutex);
			activeHead=node;
		} else {
			node->next=activeHead; node->prev=activeHead->prev; activeHead->prev=node; node->prev->next=node;
			OSUnlockMutex(ringMutex);
		}
		self->activeDevices=activeHead;
	}
	// Phase 4: device setup is complete; construct audio and acquire its two persistent banks.
	Audio_CreateGlobalSoundManager(self);
	return 1;
}
#pragma opt_common_subs on

// Create the shared sound manager and acquire the permanent and menu banks.
int Audio_CreateGlobalSoundManager(void*) {
	g_soundManager = new SoundManagerAllocation();
	SoundManager_AcquireSoundBank(g_soundManager, golfTaskStrings.permanentSoundBank);
	SoundManager_AcquireSoundBank(g_soundManager, golfTaskStrings.menuSoundBank);
	return 0;
}

typedef DeviceRegistrationEntry ProjectCallbackEntry;
struct ProjectCallbackList {
	ProjectCallbackEntry entries[16];
	int count;
};
struct ProjectCallbackState { char prefix[0xB4]; ProjectCallbackList* list; };

// Dispatch a callback to each registered object and combine its result flags.
// arg1..arg3 are forwarded unchanged; returns true only for aggregate 0 or -1 (no list returns 0).
int CProjectG_DispatchRegisteredObjectCallback(ProjectCallbackState* self, void* unused, void* arg1, void* arg2, void* arg3) {
	ProjectCallbackList* list = self->list;
	if (list == 0)
		return 0;
	int result = 0;
	ProjectCallbackEntry* e = list->entries;
	for (int i = 0; i < list->count; ++i, ++e) {
		void* object = e->object;
		result |= ((int (*)(void*, void*, void*, void*))(*(void***)object)[3])(object, arg1, arg2, arg3);
	}
	return result == 0 || result == -1;
}

struct ProjectDisplayState { char prefix[0xA4]; unsigned char flagA4; };

struct ViewUpdateInterface { 
virtual void a(); virtual void b(); virtual void c(); virtual void Update();
};
// Render-target vtable slot 19 returns 1 while input should be suspended,
// 0 when it may resume. Other codes leave this function's input state unchanged.
struct RenderTargetStatusInterface {
	virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3(); virtual void v4();
	virtual void v5(); virtual void v6(); virtual void v7(); virtual void v8(); virtual void v9();
	virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
	virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual int Method();
};
enum RenderTargetInputStatus { kRenderTargetAcceptsInput = 0, kRenderTargetSuspendsInput = 1 };
// Update the viewport size and suspend/restore input according to render-target status.
void CProjectG_UpdateDisplayStatus(ProjectDisplayState* self) {
	((ViewUpdateInterface*)g_view)->Update();
	int viewportWidth = (int)AsWView(g_view)->viewportWidth;
	g_viewportWidthPixels = viewportWidth;
	int viewportHeight = (int)AsWView(g_view)->viewportHeight;
	g_viewportHeightPixels = viewportHeight;
	void* obj = AsWView(g_view)->renderTarget;
	if (obj != 0) {
		int result = ((RenderTargetStatusInterface*)obj)->Method();
		if (result == kRenderTargetSuspendsInput && g_mouseInputEnabled) {
			g_mouseInputEnabled = 0;
			self->flagA4 = 1;
		} else if (self->flagA4 && result == kRenderTargetAcceptsInput && !g_mouseInputEnabled) {
			g_mouseInputEnabled = 1;
			self->flagA4 = 0;
		}
	}
}

// Lazily construct and register the shared course-collision tree.
void* CQuadTree_GetInstance() {
	if (!s_quadTreeInitialized) {
		CQuadTree_Ctor(s_quadTreeStorage);
		__register_global_object(s_quadTreeStorage, CQuadTree_dtor, s_quadTreeDtorRegistration);
		s_quadTreeInitialized = 1;
	}
	return s_quadTreeStorage;
}

struct ProjectDeviceIterationState { char prefix[0x68]; void* first; void* second; char rest[0x18]; char mutex[0x28]; };
// Iteration view of ProjectDeviceNode: +0 is the DEVICE POINTER (typed as node* only for ABI),
// +0xC is the backward ring link hidden in reserved04, and +0x10 is the forward ring link.
// first anchors the ring; second is the next node to visit, set to null on wraparound.
struct DeviceIterationNode { DeviceIterationNode* devicePayload; char reserved04[0x0C]; DeviceIterationNode* linked; };

// Visit each registered device under the list mutex and dispatch its timed update.
// Locks protect fetching/advancing a node, NOT the virtual call: device callbacks run unlocked.
// g_currentTimeMilliseconds is an absolute timestamp, not a per-frame delta.
void CProjectG_UpdateRegisteredDevices(ProjectDeviceIterationState* self) {
	// Byte addressing keeps CodeWarrior scheduling identical to the original mutex access.
	DeviceIterationNode* node;
	{
		InstanceLock_u801CAB90 lock((char*)self + kDeviceListMutexOffset);
		self->second = self->first;
		DeviceIterationNode* firstNode = (DeviceIterationNode*)self->second;
		OSLockMutex((char*)self + kDeviceListMutexOffset);
		if (firstNode != 0) {
			DeviceIterationNode* linked = firstNode->linked;
			if (linked == (DeviceIterationNode*)self->first) linked = 0;
			self->second = linked;
			OSUnlockMutex((char*)self + kDeviceListMutexOffset);
			node = firstNode->devicePayload;
		} else {
			OSUnlockMutex((char*)self + kDeviceListMutexOffset);
			node = 0;
		}
	}
	while (node != 0) {
		typedef void (*Call)(void*, unsigned int);
		Call call = (*(Call**)node)[9];
		call(node, g_currentTimeMilliseconds);
		OSLockMutex((char*)self + kDeviceListMutexOffset);
		DeviceIterationNode* current = (DeviceIterationNode*)self->second;
		if (current != 0) {
			DeviceIterationNode* linked = current->linked;
			if (linked == (DeviceIterationNode*)self->first) linked = 0;
			self->second = linked;
			OSUnlockMutex((char*)self + kDeviceListMutexOffset);
			node = current->devicePayload;
		} else {
			OSUnlockMutex((char*)self + kDeviceListMutexOffset);
			node = 0;
		}
	}
}

// Report the base provider status as zero.
int WInputDev_InitDevice() {
	return 0;
}

struct RawCameraVector { unsigned int x, y, z; };
struct RawCameraTransform { RawCameraVector a, b, c, d; };
extern RawCameraTransform s_cameraTransform;
extern unsigned int g_transientDiagnosticText[];
extern unsigned int g_diagnosticTextStartTimeMs;
extern unsigned int g_renderedFrameCount;
extern void* g_sceneManager;
extern void* g_chatMsg;
extern const float kDiagnosticTextCenterFactor;
extern const float kDiagnosticTextBottomMargin;
extern float kFloat_0_Scene;
extern void CCheckDriveStatus_SetErrorDialogsEnabled(int);
extern void FxShot_AccumulateDistanceWeightedOffsets(void*, RawCameraVector*, unsigned char*);
extern void WView_SetViewMatrix(void*, RawCameraTransform*);
extern void CTask_DrawCurrentTaskOrLoading(void*); // Dispatch the current task draw/loading path.
extern void MouseCursor_EmptyHook(void*); // Empty cursor draw hook in this build.
extern unsigned int OS_GetTimeMilliseconds(void);
extern void WFont_Print(void*, void*, const char*, unsigned int, unsigned int, float, float);
extern void Sprite_DrawColoredRect(void*, float*, int, unsigned int);

struct ViewportSizeSnapshot {
	char pad[0x188];
	float x;
	float y;
};

struct FontResourceInterface {
	void** vtable;
	char pad[0x10];
	void* object;
};

#pragma opt_dead_assignments off
// Draw the scene, transient diagnostic text and effect overlay, then count the frame.
void CProjectG_Draw() {
	CCheckDriveStatus_SetErrorDialogsEnabled(0);
	unsigned char flag = 0;
	RawCameraTransform s1 = s_cameraTransform;
	RawCameraTransform s2;
	s2 = s1;
	RawCameraTransform table;
	table = s2;
	if (g_fx) FxShot_AccumulateDistanceWeightedOffsets(g_fx, &table.d, &flag);
	WView_SetViewMatrix(g_view, &table);
	((void (*)(void*, int, int))(*(void***)g_view)[3])(g_view, 0, 7);
	CTask_DrawCurrentTaskOrLoading(g_taskManager);
	MouseCursor_EmptyHook(g_mouseCursor);
	if (!g_sceneManager)
		((void (*)(void*))(*(void***)g_fx)[2])(g_fx);
	unsigned int now = OS_GetTimeMilliseconds();
	if ((unsigned int)(now - g_diagnosticTextStartTimeMs) < kDiagnosticDurationMs && g_chatMsg) {
		FontResourceInterface* layout = (FontResourceInterface*)((CChatMsg*)g_chatMsg)->defaultFixedFont;
		float y = AsWView(g_view)->viewportHeight;
		float x = AsWView(g_view)->viewportWidth;
		float width = ((float (*)(void*, void*, const char*))layout->vtable[5])(layout, g_view, (const char*)g_transientDiagnosticText); // original r4 = g_view (0x801CAF98), r5 = text (0x801CAF9C)
		float centered = kDiagnosticTextCenterFactor * x - kDiagnosticTextCenterFactor * width;
		WFont_Print(layout, g_view, (const char*)g_transientDiagnosticText, 0, kDiagnosticTextColor, centered, y - kDiagnosticTextBottomMargin);
		((void (*)(void*, void*))(*(void***)layout)[9])(layout, g_view);
	}
	if (flag) {
		float rect[4] = { kFloat_0_Scene, kFloat_0_Scene, AsWView(g_view)->viewportWidth, AsWView(g_view)->viewportHeight };
		((void (*)(void*, float*, int, unsigned int, float))Sprite_DrawColoredRect)(g_view, rect, 0, ((unsigned int)flag << 24) | kWhiteRgb, kFloat_0_Scene);
	}
	++g_renderedFrameCount;
	CCheckDriveStatus_SetErrorDialogsEnabled(1);
}

struct ProjectPropertyState {
	char prefix[0xBC];
	unsigned int propertyValue;
};

// Read the project extension property; its consumer-defined meaning is unresolved.
unsigned int CProjectG_GetExtensionProperty(ProjectPropertyState* arg) {
	return arg->propertyValue;
}

// Store the project extension property without interpreting its value.
void CProjectG_SetExtensionProperty(ProjectPropertyState* arg, unsigned int value) {
	arg->propertyValue = value;
}

void CProjectG_dtor(void* arg, int shouldDelete);

// Adjust the first secondary-interface pointer before destroying the project.
void CProjectG_dtor_ThunkMinus16(char* arg, int shouldDelete) {
	CProjectG_dtor(arg - 0x10, shouldDelete);
}

// Adjust the second secondary-interface pointer before destroying the project.
void CProjectG_dtor_ThunkMinus32(char* arg, int shouldDelete) {
	CProjectG_dtor(arg - 0x20, shouldDelete);
}


// Clear per-shot collision/recovery counters and enable wind for the next simulation.
void CQuadTree_ResetPerShotSimulationState(CQuadTree* arg) {
	*(unsigned int*)&arg->collisionEventCount -= *(int*)&arg->collisionEventCount;
	s_quadTreePerShotResetWord = 0;
	((QuadTreeGroundState*)arg)->resetFlag = 0;
	arg->regionBounceCount = 0;
	((QuadTreeGroundState*)arg)->resetCounter = 0;
	arg->candidateTriangleOverflow = 0;
	arg->windEnabled = 1;
}

struct SceneLighting {
	unsigned int mode;
	float directionX;
	float directionY;
	float directionZ;
	unsigned int diffuseColor;
	unsigned int ambientColor;
	unsigned int effectTintColor;
};

extern SceneLighting s_sceneLighting;
extern float s_twoPi_801CAB90_805E2AC8;
extern unsigned int g_ownedSceneAuxiliaryObject;
extern char s_sceneAuxiliaryDtorRegistration[];
extern const float kFloat_6_2831855;
extern float kFloat_0_Scene;
void __register_global_object(void*, void* (*)(void*, int), char*);
void __dl__FPv(void*);

// Destroy an owned object pointer, then optionally release its wrapper storage.
void* SceneOwnedObject_dtor(void* arg, int deletingFlag) {
	if (arg != 0) {
		if (*(void**)arg != 0)
			__dl__FPv(*(void**)arg);
		if (deletingFlag > 0)
			__dl__FPv(arg);
	}
	return arg;
}

// Initialize scene lighting and register cleanup for its owned auxiliary object.
void Scene_InitializeLightingGlobals() {
	s_twoPi_801CAB90_805E2AC8 = kFloat_6_2831855;
	s_sceneLighting.mode = 0;
	s_sceneLighting.diffuseColor = 0;
	s_sceneLighting.ambientColor = 0;
	s_sceneLighting.effectTintColor = kWhiteRgb;
	s_sceneLighting.directionZ = kFloat_0_Scene;
	s_sceneLighting.directionY = kFloat_0_Scene;
	s_sceneLighting.directionX = kFloat_0_Scene;
	g_ownedSceneAuxiliaryObject = 0;
	__register_global_object(&g_ownedSceneAuxiliaryObject, SceneOwnedObject_dtor, s_sceneAuxiliaryDtorRegistration);
}

extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern char* strcpy(char*, const char*);
extern unsigned int strlen(const char*);
extern void __throw(void*, void*, void*);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*, int);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

} // extern "C"

// Construct a bounds record or exception object in caller-provided storage without allocation.
inline void* operator new(new_size_t, void* p) { return p; }

extern "C" {

#pragma CPP_exceptions on
struct CBB18_Flag {
    volatile bool b;
    // Preserve the empty allocator's compiler-visible tag/lifetime used by vector exception paths.
    CBB18_Flag() : b(false) {}
} __attribute__((aligned(4)));
struct CBB18_Vector {
    unsigned int x, y, z;
};
// Stored payload: two adjacent XYZ endpoints (+0/+12), followed by a region tag (+24).
// AABB_Copy writes both endpoints starting at a; neither member owns storage.
struct CBB18_Record {
    CBB18_Vector a, b; // First/second endpoint in the source AABB word order.
    int id; // Caller-provided regionId, copied without interpretation.
    // Zero the two consecutive XYZ triples; together a/b are the 24-byte bounds copied below.
    CBB18_Record() { Vec3_Zero(&a); Vec3_Zero(&b); }
    // Copy both bounds endpoints and the region tag without changing float bit patterns.
    CBB18_Record(const CBB18_Record& other) {
        struct Vec3Raw { unsigned int v[3]; };
        try {
            *(Vec3Raw*)&a = *(const Vec3Raw*)&other.a;
            *(Vec3Raw*)&b = *(const Vec3Raw*)&other.b;
            id = other.id;
        } catch (...) { throw; }
    }
};
struct CBB18_BufferGuard {
    char* data;
    volatile bool* allocator;
    // Guard a newly allocated exception-message buffer until its shared owner is installed.
    CBB18_BufferGuard(char* p, volatile bool* a) : data(p), allocator(a) {}
    // Free only a buffer whose ownership has not been committed to the shared wrapper.
    ~CBB18_BufferGuard() { if (data) __dla__FPv(data); }
};
struct CBB18_Shared {
    void* vtable;
    int refs, owners;
    char* text;
    // Adopt the exception text buffer with one initial reference and owner.
    CBB18_Shared(char* p) { refs = 1; owners = 1; vtable = s_sharedCharDeleterVtable; text = p; }
};
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct CBB18_ErrorBase {
    void* vtable;
    char* text;
    CBB18_Shared* shared;
    // Copy a diagnostic string into a shared buffer for a vector length exception.
    CBB18_ErrorBase(const char* s) {
        vtable = __vt__Q23std11logic_error;
        CBB18_Flag other;
        CBB18_Flag allocator;
        unsigned int n = strlen(s) + 1;
        text = (char*)__nwa__FUl(n * 2 - n);
        {
            char* p = text;
            CBB18_BufferGuard guard(p, &allocator.b);
            shared = new CBB18_Shared(p);
            guard.data = 0;
        }
        strcpy(text, s);
    }
    // Run the reconstructed standard-exception base destructor.
    ~CBB18_ErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
} // extern "C"
namespace std {
struct length_error : CBB18_ErrorBase {
    // Build length_error around the copied vector diagnostic.
    length_error(const char* s) : CBB18_ErrorBase(s) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {
struct CBB18_Storage {
    CBB18_Record* data;
    unsigned int count, capacity;
    unsigned int* allocator;
    unsigned int offset; // Uncopied prefix length; reaches zero after backward copy.
    // Start a temporary owned vector used to commit growth atomically at the final swap.
    CBB18_Storage(unsigned int* a) : data(0), count(0), capacity(0), allocator(a), offset(0) {}
    // Release uncommitted new storage, or the old allocation after a successful swap.
    ~CBB18_Storage() {
        if (data) {
            count -= *(int*)&count;
            appFreeUni__FPv(data);
        }
    }
};
struct CBB18_Empty {};
struct CBB18_Holder {
    CBB18_Empty* ref;
    // Preserve the empty allocator reference used for in-place record construction.
    CBB18_Holder(CBB18_Empty* r) : ref(r) {}
};
struct CBB18_Manager {
    char pad[0x2078];
    CBB18_Record* data;
    unsigned int count, capacity;
};
// Select the larger capacity increment while preserving the library's reference result.
inline const unsigned int& CBB18_Max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Semantic aliases retain the constructor/type symbols required by the matching ABI.
typedef CBB18_Record GroundBoundsValue;
typedef CBB18_Storage BoundsGrowthTransaction;
typedef CBB18_Flag BoundsAllocatorMarker;
typedef CBB18_Empty EmptyBoundsAllocator;
typedef CBB18_Holder BoundsAllocatorReference;
typedef CBB18_Manager GroundBoundsListView;
enum GroundBoundsCapacityLimits {
    kGroundBoundsMaxCount = 0x09249249, // floor(0xFFFFFFFF / 28)
    kGroundBoundsThreeFifthsLimit = 0x030c30c3,
    kGroundBoundsHalfLimit = 0x06186186
};
// Borrow the beginning of the six-word AABB payload in a tagged record.
static inline CBB18_Vector* GroundBoundsEndpoints(GroundBoundsValue* value) { return &value->a; }

#pragma use_lmw_stmw on
// Append a tagged bounding box, growing the region array with exception-safe ownership transfer.
// regions owns data[0..count); capacity counts 28-byte records. bounds borrows six consecutive
// float words (two XYZ endpoints); regionId is copied verbatim. No pointer to bounds is retained.
// Roles: GroundBoundsValue (CBB18_Record) is the 28-byte stored element {a[3], b[3], id};
// BoundsGrowthTransaction (CBB18_Storage) is the temporary replacement array that owns the new
// block until the final swap (then it holds, and frees, the OLD block); regions->data/count/
// capacity is the container being grown (growth rule: +3/5 below 0x030c30c3 elements, +1/2
// below 0x06186186, else the maximum, always at least the needed extra).
// Edit the payload in CBB18_Record above; the aliases below distinguish it from the
// BoundsGrowthTransaction that owns replacement storage. Allocation/exception tags
// are kept in their original scopes; field names are provisional.
void GroundRegionList_AppendBounds(GroundBoundsListView* regions, char* bounds, int regionId) {
    GroundBoundsValue value;
    unsigned int requiredExtra;
    unsigned int halfGrowth;
    unsigned int threeFifthsGrowth;
    AABB_Copy(GroundBoundsEndpoints(&value), bounds);
    value.id = regionId;
    BoundsAllocatorMarker appendAllocatorMarker;
    // Fast path: construct one record in spare capacity; existing records stay in place.
    if (regions->count < regions->capacity) {
        EmptyBoundsAllocator empty;
        BoundsAllocatorReference holder(&empty);
        new (regions->data + regions->count) GroundBoundsValue(value);
        ++regions->count;
    } else {
        // Growth phase 1: validate count and choose capacity using the original 28-byte limit.
        const unsigned int max = kGroundBoundsMaxCount;
        BoundsAllocatorMarker limitAllocatorMarker;
        BoundsAllocatorMarker growthAllocatorMarker;
        if (max - regions->capacity < 1) {
            const char* msg = courseObjectSoundStrings.vectorLengthError;
            {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct(msg);
                exception_runtime::__throw(exception_runtime::courseObjectSoundStrings.lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
        }
        requiredExtra = regions->count + 1 - regions->capacity;
        BoundsGrowthTransaction storage(&regions->capacity);
        unsigned int newCapacity = regions->capacity;
        if (requiredExtra > max - newCapacity) {
            const char* msg = courseObjectSoundStrings.vectorLengthError;
            {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct(msg);
                exception_runtime::__throw(exception_runtime::courseObjectSoundStrings.lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
        }
        if (newCapacity < kGroundBoundsThreeFifthsLimit) {
            threeFifthsGrowth = (newCapacity + 1) * 3 / 5;
            newCapacity += CBB18_Max(threeFifthsGrowth, requiredExtra);
        } else if (newCapacity < kGroundBoundsHalfLimit) {
            halfGrowth = (newCapacity + 1) / 2;
            newCapacity += CBB18_Max(halfGrowth, requiredExtra);
        } else newCapacity = max;
        BoundsAllocatorMarker reserveAllocatorMarker;
        if (newCapacity > max) {
            const char* msg = courseObjectSoundStrings.vectorLengthError;
            {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct(msg);
                exception_runtime::__throw(exception_runtime::courseObjectSoundStrings.lengthErrorType, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
        }
        // Growth phase 2: allocate a guarded buffer and construct the appended record first.
        storage.data = (GroundBoundsValue*)appMallocDummy__FUl(newCapacity * sizeof(GroundBoundsValue));
        storage.capacity = newCapacity;
        storage.offset = regions->count;
        BoundsAllocatorMarker appendConstructionMarker;
        GroundBoundsValue* dest = storage.data + storage.offset + storage.count;
        for (unsigned int n = 1; n != 0; --n, ++dest) {
            new (dest) GroundBoundsValue(value);
            ++storage.count;
        }
        BoundsAllocatorMarker prefixCopyMarker;
        // Growth phase 3: copy old records backward into the prefix before the new record.
        GroundBoundsValue* src = regions->data + regions->count;
        dest = storage.data + storage.offset;
        while (src > regions->data) {
            --dest;
            --src;
            new (dest) GroundBoundsValue(*src);
            --storage.offset;
            ++storage.count;
        }
        BoundsAllocatorMarker commitMarker;
        // Commit: exchange buffers/capacities; storage's destructor now frees the old allocation.
        regions->count = 0;
        GroundBoundsValue* old = regions->data;
        regions->data = storage.data;
        storage.data = old;
        unsigned int oldCapacity = regions->capacity;
        regions->capacity = storage.capacity;
        storage.capacity = oldCapacity;
        BoundsAllocatorMarker swapMarker;
        regions->count = storage.count;
        storage.count = 0;
    }
}
#pragma use_lmw_stmw reset
#pragma opt_common_subs on

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462194", _ctors_u801CAB90_0, Scene_InitializeLightingGlobals);
#pragma pop
