#include <decomp/static_init.h>
#pragma ipa file
#pragma auto_inline off
#include "classes/CSharedDoc.h"
#include "classes/SpringGridVertex.h"
#include "classes/COption.h"
#include "classes/ProjectOwnedListNode.h"
#include "classes/SpringGridSurfaceView.h"
// Translation unit 0x801C9600-0x801CAB90 (17 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/types.h"
#include "sdk_protos.h"
#include "classes/CProjectG.h"
#include "classes/CProject.h"
#include "classes/CMouseCursor.h"
#define AsCMouseCursor(p) ((CMouseCursor*)(p))

#define AsCProject(p) ((CProject*)(p))

#define AsCProjectG(p) ((CProjectG*)(p))

extern "C" {

enum ProjectConstants {
    kMinimumGridDimension = 2,
    kProjectUpdateActive = 2,
    kSkipProjectInputUpdate = 0x80,
    kInitialDeviceNodeCount = 16,
    kInitialInputNodeCount = 32,
    kInitialTaskCommand = 5,
    kGridCellBytes = sizeof(SpringGridVertex),
    kPrimaryInterfaceVtableSlot = 8,
    kSecondaryInterfaceVtableSlot = 11,
    kProjectInterfaceOffset = (int)&((CProjectG*)0)->interfaceVtable
};
// Byte offsets of NUL-terminated names inside the golfTaskStrings pool (identified by how they are used below):
// 1 and 0xE are sound-bank names released at shutdown, 0x16 is the task name passed to CTask_RequestTransition,
// 0x21 is the name of the first task queued by CProjectG_InitializeGameObjects.
enum GolfTaskStringOffset {
    kPermanentSoundBank = 1,
    kMenuSoundBank = 0xE,
    kTitleTaskName = 0x16,
    kTitleMainName = 0x21
};


// Default grid-cell array constructor; initialization is performed by the grid setup loops.
void GridCell_DefaultCtor() {}

// Local one-off views of the grid data (the real layouts are the GridCell/GridSurface class headers included above):
// GridWords = the x,y,z floats of a cell read as three ints (copied as raw words); GridRes = ground-query result
// (triangle index, hit point); VecCell = one relative position; GridItem = one simulated cell; GridObj = the grid owner.
extern void* __nwa__FUl(unsigned int);
struct GridWords { int a, b, c; };
struct GridRes { volatile int index; Vec3f b; };
struct VecCell {
	float x, y, z;
	// Leave vector components for the grid setup loop to initialize.
	VecCell() {}
	// Allocate the vector array through the game's array allocator.
	static void* operator new[](size_t s) { return __nwa__FUl(s); }
};
struct GridItem {
	float x, y, z;
	float normalX, normalY, normalZ;
	float acceleration, velocity, texU, texV;
	unsigned char active;
	char padding29[3];
	GridItem();
	// Allocate the cell array through the game's array allocator.
	static void* operator new[](size_t s) { return __nwa__FUl(s); }
};
struct GridObj {
	char padding00[4];
	int itemCount, groupCount;
	GridItem** items;
	Vec3f** vectors;
	char padding14[8];
	float x0;
	char padding20[4];
	float y0, x1, height, y1;
	char mtx[0x30];
};
extern void* __nwa__FUl(unsigned int);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void* = 0);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, void*, int);
extern void CQuadTree_GetMeshNormal(void*, void*, int);
extern int g_currentTimeMilliseconds;
extern const float kFloat_0_805DF6F4;
extern const float kFloat_0_5_Collision;
extern const float kFloat_1_GeneTex3D;
// Allocate a 2-by-2 spring surface, initialize normals/UVs, and activate cells above suitable ground.
void GroundGrid_InitializeTwoByTwo(GridObj* self) {
	// Phase 1: allocate separately owned row arrays for cells and relative vectors.
	// Bounds/height are world coordinates; vectors subtract the XYZ at mtx's
	// start (this call does not apply a matrix). Existing arrays are not released.
	Vec3f relativePosition;
	Vec3f upNormal;
	Vec3f groundNormal;
	GridWords queryPosition;
	Vec3f groundPoint;
	GridRes groundResult;
	int row;
	int groundRow, groundColumn;
	int first, second;
	GridWords* p;
	self->groupCount = kMinimumGridDimension;
	self->itemCount = kMinimumGridDimension;
	float spacingX = (self->x1 - self->x0) / (self->groupCount - 1);
	float spacingZ = (self->y1 - self->y0) / (self->itemCount - 1);
	self->items = (GridItem**)__nwa__FUl(8);
	self->vectors = (Vec3f**)__nwa__FUl(self->groupCount * 4);
	for (row = 0; row < self->groupCount; ++row) {
		void* storage = __nwa__FUl(self->itemCount * sizeof(GridItem) + 16);
		self->items[row] = storage ? (GridItem*)__construct_new_array(storage,
		    (void*)GridCell_DefaultCtor, 0, sizeof(GridItem), self->itemCount) : 0;
		self->vectors[row] = (Vec3f*)new VecCell[self->itemCount];
	}
	// Phase 2: place the flat world-space grid, set upward normals and centered UVs.
	for (int row = 0; row < self->groupCount; ++row) {
		for (int column = 0; column < self->itemCount; ++column) {
			self->items[row][column].x = spacingX * column + self->x0;
			self->items[row][column].y = self->height;
			self->items[row][column].z = spacingZ * row + self->y0;
			__mi__FRC7WVectorRC7WVector(&relativePosition, &self->items[row][column], self->mtx);
			__as__7WVectorFRC7WVector(&self->vectors[row][column], &relativePosition);
			self->items[row][column].acceleration = kFloat_0_805DF6F4;
			self->items[row][column].velocity = kFloat_0_805DF6F4;
			self->items[row][column].texU = kFloat_0_5_Collision;
			self->items[row][column].texV = kFloat_0_5_Collision;
			upNormal.x = kFloat_0_805DF6F4;
			upNormal.y = kFloat_1_GeneTex3D;
			upNormal.z = kFloat_0_805DF6F4;
			__as__7WVectorFRC7WVector(&AsGridCell801C9600(&self->items[row][column])->normalX, &upNormal);
			self->items[row][column].active = 0;
		}
	}
	AsGridSurface801C9600(self)->updatedAt = g_currentTimeMilliseconds;
	// Phase 3: activate cells only above ground whose upward normal exceeds 0.5.
	unsigned char active = 1;
	for (groundRow = 0; groundRow < self->groupCount; ++groundRow) {
		for (groundColumn = 0; groundColumn < self->itemCount; ++groundColumn) {
			p = (GridWords*)&self->items[groundRow][groundColumn];
			first = p->a;
			second = p->b;
			queryPosition.a = first;
			queryPosition.b = second;
			queryPosition.c = p->c;
			GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint, CQuadTree_GetInstance(), &queryPosition, 1, (void*)&groundResult.index, 0);
			__as__7WVectorFRC7WVector(&groundResult.b, &groundPoint);
			if (groundResult.index >= 0 && groundResult.b.y < self->height) {
				CQuadTree_GetMeshNormal(&groundNormal, CQuadTree_GetInstance(), groundResult.index);
				if (groundNormal.y > kFloat_0_5_Collision) self->items[groundRow][groundColumn].active = active;
			}
		}
	}
}

// Unused variadic diagnostic hook; the retail implementation discards its arguments.
void GridSurface_DiagnosticStub(...) {}

struct GridSurfaceDimensions { char padding00[4]; int itemCount; int groupCount; void** groups; };
// groups[row][column] stores world-space X/Y/Z; Y alone is integrated here.
// Rows own separately allocated cell arrays. The outermost cells supply boundary
// heights; only interior active cells generate forces, but active boundary cells
// may receive those forces and are included in the integration pass.
extern const float kFloat_0_805DF6F4;
extern const float kGridDiagonalWeightStorage;
extern const float kFloat_2_GridSurface;
// Accumulate neighbor spring forces and integrate active-cell velocity and height above a lower bound.
void GridSurface_StepSpringHeights(GridSurfaceDimensions* self, float scale) {
    // scale multiplies both acceleration->velocity and velocity->height. It is
    // an integration step in caller units, not a spring-strength-only parameter.
    // Phase 1: clear force accumulators, including inactive/boundary cells.
	for (int row = 0; self->groupCount > row; row++) {
		for (int column = 0; column < self->itemCount; column++) AsGridCell801C9600(self->groups[row])[column].acceleration = kFloat_0_805DF6F4;
	}
	// Phase 2: apply paired equal/opposite contributions from interior active cells.
	for (int row = 1; row < self->groupCount - 1; row++) {
		for (int column = 1; column < self->itemCount - 1; column++) {
			if (!AsGridCell801C9600(self->groups[row])[column].active) continue;
			// Left neighbor: transfer the full height difference.
			{
				float heightDifference = AsGridCell801C9600(self->groups[row])[column].y
					- AsGridCell801C9600(self->groups[row])[column - 1].y;
				AsGridCell801C9600(self->groups[row])[column].acceleration -= heightDifference;
				AsGridCell801C9600(self->groups[row])[column - 1].acceleration += heightDifference;
			}
			// Preserve the original upY - upY contribution, including floating-point behavior.
			{
				float upY = AsGridCell801C9600(self->groups[row - 1])[column].y;
				float heightDifference = upY - upY;
				AsGridCell801C9600(self->groups[row])[column].acceleration -= heightDifference;
				AsGridCell801C9600(self->groups[row - 1])[column].acceleration += heightDifference;
			}
			// Right neighbor: transfer the full height difference.
			{
				float heightDifference = AsGridCell801C9600(self->groups[row])[column].y
					- AsGridCell801C9600(self->groups[row])[column + 1].y;
				AsGridCell801C9600(self->groups[row])[column].acceleration -= heightDifference;
				AsGridCell801C9600(self->groups[row])[column + 1].acceleration += heightDifference;
			}
			// Next row, same column: full weight.
			{
				float heightDifference = AsGridCell801C9600(self->groups[row])[column].y
				- AsGridCell801C9600(self->groups[(row + 1)])[column].y;
				AsGridCell801C9600(self->groups[row])[column].acceleration -= heightDifference;
				AsGridCell801C9600(self->groups[row + 1])[column].acceleration += heightDifference;
			}
			// Next row/right column: use the stored diagonal weight.
			{
				float heightDifference = (AsGridCell801C9600(self->groups[row])[column].y
					- AsGridCell801C9600(self->groups[row + 1])[column + 1].y) * kGridDiagonalWeightStorage;
				AsGridCell801C9600(self->groups[row])[column].acceleration -= heightDifference;
				AsGridCell801C9600(self->groups[row + 1])[column + 1].acceleration += heightDifference;
			}
			// Previous row/left column: use the stored diagonal weight.
			{
					float heightDifference = (AsGridCell801C9600(self->groups[row])[column].y
				- AsGridCell801C9600(self->groups[row - 1])[column - 1].y) * kGridDiagonalWeightStorage;
				AsGridCell801C9600(self->groups[row])[column].acceleration -= heightDifference;
				AsGridCell801C9600(self->groups[row - 1])[column - 1].acceleration += heightDifference;
			}
			// Next row/left column: use the stored diagonal weight.
			{
				float heightDifference = (AsGridCell801C9600(self->groups[row])[column].y
					- AsGridCell801C9600(self->groups[row + 1])[column - 1].y) * kGridDiagonalWeightStorage;
				AsGridCell801C9600(self->groups[row])[column].acceleration -= heightDifference;
				AsGridCell801C9600(self->groups[(row + 1)])[column - 1].acceleration += heightDifference;
			}
			// The original applies this diagonal contribution a second time; preserve it.
			{
				float heightDifference = (AsGridCell801C9600(self->groups[row])[column].y
					- AsGridCell801C9600(self->groups[1 + row])[column - 1].y) * kGridDiagonalWeightStorage;
				AsGridCell801C9600(self->groups[row])[column].acceleration -= heightDifference;
					AsGridCell801C9600(self->groups[(row + 1)])[column - 1].acceleration += heightDifference;
			}
		}
	}
	// Phase 3: integrate active cells and clamp Y to base height minus two units.
	// A clamped cell retains its computed velocity; no collision damping occurs.
	for (int row = 0; row < self->groupCount; row++) {
		for (int column = 0; column < self->itemCount; column++) {
			if (AsGridCell801C9600(self->groups[row])[column].active) { float nextVelocity = scale * (AsGridCell801C9600(self->groups[row])[column].acceleration) + AsGridCell801C9600(self->groups[row])[column].velocity;

			float clampedHeight;
			AsGridCell801C9600(self->groups[row])[column].velocity = nextVelocity;
			float nextHeight = scale * AsGridCell801C9600(self->groups[row])[column].velocity + AsGridCell801C9600(self->groups[row])[column].y;
			float minimumHeight = AsGridSurface801C9600(self)->height - kFloat_2_GridSurface;

			if (minimumHeight < nextHeight) clampedHeight = nextHeight;
			else clampedHeight = minimumHeight;
			AsGridCell801C9600(self->groups[row])[column].y = clampedHeight;
			}
		}
	}
}
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void Vector_CrossProduct(void*, void*, void*);
extern void Normalize__7WVectorFv(void*);
extern const float kFloat_1_GeneTex3D;
extern const float kFloat_0_5_Collision;
// Compute active-cell normals from neighboring positions and map their X/Z components to texture UVs.
// Requires at least two rows and columns. Tangents are world-space differences;
// normal = normalize(rowTangent x columnTangent), UV = 0.5*(1 + normal.xz).
// Last-row/last-column branches reverse their tangent direction as written;
// do not replace them with a symmetric central-difference formula.
void Grid_RecomputeNormalsAndNormalTexcoords(GridSurfaceDimensions* self, int unusedGroupSize) {
	float half = kFloat_0_5_Collision;
	float one = kFloat_1_GeneTex3D;
	for (int row = 0; row < self->groupCount; row++) {
		for (int column = 0; column < self->itemCount; column++) {
			if (AsGridCell801C9600((char*)self->groups[row] + column * kGridCellBytes)->active) {
				Vec3f normal, d, b, a, c;
				if (row == self->groupCount - 1) {
					__mi__FRC7WVectorRC7WVector(&a, (char*)self->groups[row] + (column == self->itemCount - 1 ? column - 1 : column + 1) * kGridCellBytes, (char*)self->groups[row] + column * kGridCellBytes);
					__as__7WVectorFRC7WVector(&b, &a);
					__mi__FRC7WVectorRC7WVector(&c, (char*)self->groups[row - 1] + column * kGridCellBytes, (char*)self->groups[row] + column * kGridCellBytes);
					__as__7WVectorFRC7WVector(&d, &c);
				} else if (column == self->itemCount - 1) {
					__mi__FRC7WVectorRC7WVector(&a, (char*)self->groups[row] + (column - 1) * kGridCellBytes, (char*)self->groups[row] + column * kGridCellBytes);
					__as__7WVectorFRC7WVector(&b, &a);
					__mi__FRC7WVectorRC7WVector(&c, (char*)self->groups[row + 1] + column * kGridCellBytes, (char*)self->groups[row] + column * kGridCellBytes);
					__as__7WVectorFRC7WVector(&d, &c);
				} else {
					__mi__FRC7WVectorRC7WVector(&a, (char*)self->groups[row] + (column + 1) * kGridCellBytes, (char*)self->groups[row] + column * kGridCellBytes);
					__as__7WVectorFRC7WVector(&b, &a);
					__mi__FRC7WVectorRC7WVector(&c, (char*)self->groups[row + 1] + column * kGridCellBytes, (char*)self->groups[row] + column * kGridCellBytes);
					__as__7WVectorFRC7WVector(&d, &c);
				}
				Vec3f crossProduct;
				Vector_CrossProduct(&crossProduct, &d, &b);
				__as__7WVectorFRC7WVector(&normal, &crossProduct);
				Normalize__7WVectorFv(&normal);
					__as__7WVectorFRC7WVector((Vec3f*)&AsGridCell801C9600(self->groups[row])[column].normalX, &normal); // original cell +0xC
				AsGridCell801C9600(self->groups[row])[column].texU = half * (one + normal.x);
				AsGridCell801C9600(self->groups[row])[column].texV = half * (one + normal.z);
			}
		}
	}
}

extern const float kFloat_6_2831855;
extern float s_gridSurfaceTwoPi;

// Initialize the grid unit's stored full-circle angle during static initialization.
void GridSurface_InitializeAngleConstant() {
	s_gridSurfaceTwoPi = kFloat_6_2831855;
}

struct ProjectClockView {
	char padding00[0xA8];
	int clockOrigin;
};

extern int g_currentTimeMilliseconds;

extern int OS_GetTimeMilliseconds();
extern void CMainFrame_UpdateVirtualHooks(void*, int);

// Only flags (+0x24) and state (+0xA0) of the project object are used here; the paddings skip unrelated fields.
struct ProjectUpdateView {
	char padding00[0x24];
	int flags;
	char padding28[0x78];
	unsigned int state;
};

// Per-frame update (virtual slot 12 of CProjectG; the symbol name only records the slot).
// Refresh global frame time and dispatch the project update with input-suppression flags as needed.
void CProjectG_UpdateVirtualHooks(ProjectUpdateView* self) {
	g_currentTimeMilliseconds = OS_GetTimeMilliseconds();
	if ((self->state & kProjectUpdateActive) != 0) {
		CMainFrame_UpdateVirtualHooks(self, self->flags);
	} else {
		CMainFrame_UpdateVirtualHooks(self, self->flags | kSkipProjectInputUpdate);
	}
}

// Singleton pointers are declared as int here in this unit (other units use typed pointers).
extern int g_option;
extern int g_project;
extern int g_inputActionManager;
extern void BaseObject_dtor(void*, int);
extern void __dl__FPv(void*);

// Clear the project singleton, destroy its base object and optionally free the instance.
void* CProjectBase_dtor(void* ptr, int shouldDelete) {
	if (ptr != 0) {
		g_project = 0;
		BaseObject_dtor(ptr, 0);
		if (shouldDelete > 0) {
			__dl__FPv(ptr);
		}
	}
	return ptr;
}

// Clear the input-action singleton, destroy its base object and optionally free the instance.
void* CInputActionManagerBase_dtor(void* ptr, int shouldDelete) {
	if (ptr != 0) {
		g_inputActionManager = 0;
		BaseObject_dtor(ptr, 0);
		if (shouldDelete > 0) {
			__dl__FPv(ptr);
		}
	}
	return ptr;
}

// Destroy the option singleton through its virtual deleting-destructor entry.
void* COption_VirtualDtor(void* ptr, int shouldDelete) {
	if (ptr != 0) {
		if (ptr != 0) {
			g_option = 0;
			BaseObject_dtor(ptr, 0);
		}
		if (shouldDelete > 0) {
			__dl__FPv(ptr);
		}
	}
	return ptr;
}

// Clear the option singleton and release its base object and optional allocation.
void* COption_dtor(void* ptr, int shouldDelete) {
	if (ptr != 0) {
		g_option = 0;
		BaseObject_dtor(ptr, 0);
		if (shouldDelete > 0) {
			__dl__FPv(ptr);
		}
	}
	return ptr;
}

extern int g_inputPointer;
extern int g_soundManager;
extern int g_fx;
extern int g_mouseCursor;
extern void InputManager_ClearNodeFlagBit1(void*);
extern void Pointer_ClearInputSuppression(void*);
extern void SoundManager_StopAndClearChannels(void*);
extern void CFx_ClearEffects(void*, int);

// Reset pointer suppression, stop audio and effects, and restore the mouse cursor hit-test mode.
void GolfTransition_ClearInputAudioAndEffects() {
	if (g_inputPointer != 0) {
		InputManager_ClearNodeFlagBit1((void*)g_inputPointer);
		Pointer_ClearInputSuppression((void*)g_inputPointer);
	}
	if (g_soundManager != 0) SoundManager_StopAndClearChannels((void*)g_soundManager);
	if (g_fx != 0) CFx_ClearEffects((void*)g_fx, 0);
	if (g_mouseCursor != 0) AsCMouseCursor(g_mouseCursor)->hitTestMode = 0;
}

struct ProjectNodeBlock {
	char padding00[0xc];
	ProjectNodeBlock* prev;
	ProjectNodeBlock* next;
	char padding14[8];
};
struct ProjectNodePool {
	char padding00[8];
	ProjectNodeBlock* head;
	char padding0C[0x10];
	void* member1C;
	char mutex[0x18];
};
extern void appFreeUni__FPv(void*);
struct WInstanceLock {
	void* lock;
	// Acquire the borrowed mutex for this scope.
	WInstanceLock(void* l) { lock = l; OSLockMutex((OSMutex*)l); }
	// Release the mutex on normal or exceptional scope exit.
	~WInstanceLock() { OSUnlockMutex((OSMutex*)lock); }
};
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_strength_reduction off
// Release node allocation blocks and buckets under the pool mutex, then optionally free the pool.
void* ProjectNodePool_dtor(ProjectNodePool* self, int shouldDelete) {
	if (self != 0) {
		ProjectNodeBlock* next;
		ProjectNodeBlock* sentinel;
		ProjectNodeBlock* node;
		{
			WInstanceLock outer((OSMutex*)(self->mutex));
			if (self->head != 0) {
				sentinel = (ProjectNodeBlock*)(self->mutex);
				while ((node = self->head) != 0) {
					{
						WInstanceLock g(sentinel);
						node->next->prev = node->prev;
						node->prev->next = node->next;
						next = node->next == node ? 0 : node->next;
					}
					self->head = next;
					appFreeUni__FPv(node);
				}
			}
			if (self->member1C != 0) {
				appFreeUni__FPv(self->member1C);
				self->member1C = 0;
			}
		}
		if (shouldDelete > 0) __dl__FPv(self);
	}
	return self;
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
#pragma use_lmw_stmw reset

extern CSharedDoc* g_sharedDoc;
extern int g_resourceManager;
extern int g_view;
extern int g_inputActionManager;
extern int g_taskManager;
extern void* __nw__FUl(unsigned int);
extern void* appMallocUni__FUl(unsigned int);
extern void CSharedDoc_BindItemSelectionInventory(void*);
extern void SharedDoc_InitDefaultCourseSettings(void*);
extern void* ResourceManager_CreateWView(void*, int);
extern void* BaseObject_ctor(void* = 0);
extern void Pointer_Reset(void*);
extern void __ct__8CChatMsgFv(void*);
extern void CCheckDriveStatus_SetLatch(); // Acquire the drive-status monitor and set its startup flag.
extern void CMouseCursor_Ctor(void*);
extern void CHeadIcon_ctor(void*);
extern void CPvsFxBox_ctor(void*);
extern void GolfTips_ResetSharedRegistryAndResources();
extern void Layout_SetHitTestingEnabled(int);
extern void CTask_RequestTransition(void*, const char*, const char*);
extern void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
extern char s_inputManagerConstructionVtable[];
extern char __vt__13CInputManager[];
extern char golfTaskStrings[];
extern const float kDefaultProjectTimeScale;
extern const float kDefaultHudBaselineY;

extern void CMainFrame_Ctor(void*);
extern void srand(unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
extern void WInstanceTable_Initialize(void*, int, int);
extern void SharedResourceLoader_Ctor(void*);
extern void CSharedDoc_Ctor(void*);
extern void CMainFrame_ResetElapsedTimeAndCaptureValue(void*);
extern void CTaskManager_Ctor(void*);

extern void __dt__11AuxMenuBaseFv(void*);
// CMainFrame_dtor: base-class destructor reached by address (the original symbol name carries no role).
extern void CMainFrame_dtor(void*, int);
extern void dtor_80048F84(void*);
extern int s_projectInterfaceConstructionVtable[];
extern int __vt__9CProjectG[];
extern int s_optionConstructionVtable[];
extern int __vt__7COption[];
extern int g_currentTimeMilliseconds;
extern int g_option;
extern int g_project;
extern int g_resourceManager;
extern char golfTaskStrings[];

struct ProjectConstructionStorage { char data[0xC0]; };
// Initialize the device instance table in its fixed-size allocation.
struct N120 { char d[0x120]; N120() { WInstanceTable_Initialize(this, 0, 9); } };
// Clear the registration-state word; other storage is left for later initialization.
struct N84 { char d[0x80]; int v; N84() { v = 0; } };
// Construct the shared resource loader in its fixed-size allocation.
struct N548 { char d[0x548]; N548() { SharedResourceLoader_Ctor(this); } };
// Construct and publish the shared document singleton.
struct Nb98 { char d[0xb98]; Nb98() { CSharedDoc_Ctor(this); } };
// Construct/publish the options singleton with its default HUD baseline.
struct N14 { char d[0x14]; N14() { BaseObject_ctor(this); AsCOption(this)->vtbl = s_optionConstructionVtable; g_option = (int)this; AsCOption(this)->vtbl = __vt__7COption; AsCOption(this)->hudBaselineY = kDefaultHudBaselineY; } };
// Construct the task manager in its fixed-size allocation.
struct N3c { char d[0x3c]; N3c() { CTaskManager_Ctor(this); } };
struct VObj801CA034 {
	virtual void v0();
	virtual void v1();
	virtual void v2();
	virtual void v3();
	virtual int vf6();
};
struct Q801CA034 {
	char pre[0x68];
	int deviceHead, deviceCursor, deviceBlocks, freeDeviceNodes, deviceBatchSize, deviceHashMask, deviceBucketCount;
	int* deviceBuckets;
	char deviceMutex[0x18];
	// Initialize empty device-pool links and a zeroed bucket table.
	void init(int cnt) {
		deviceHead = 0;
		freeDeviceNodes = 0;
		deviceBlocks = 0;
		deviceBatchSize = cnt;
		deviceCursor = 0;
		deviceBucketCount = cnt;
		if (deviceBucketCount > 0) {
			deviceBuckets = (int*)appMallocUni__FUl(0x40);
			deviceHashMask = deviceBucketCount - 1;
			for (int i = 0; i < deviceBucketCount; i++) deviceBuckets[i] = 0;
		} else {
			deviceBuckets = 0;
		}
	}
};
enum { kDeviceMutexOffset = (int)&((Q801CA034*)0)->deviceMutex };
// Initialize the project's device-node pool and bucket table with the requested capacity.
static inline void fn_801CA034_init(ProjectConstructionStorage* self, int cnt) {
	((Q801CA034*)self)->init(cnt);
}
// Construct project interfaces and device pools, initialize the clock, and create core game singletons.
// Keep byte/integer address forms for the base interface, title and mutex; typed replacements change register lifetimes.
void* CProject_Ctor(ProjectConstructionStorage* self) {
	// Phase 1: construct the two interfaces and publish the project singleton.
	CMainFrame_Ctor(self);
	void* projectInterface = (char*)self + kProjectInterfaceOffset;
	BaseObject_ctor(projectInterface);
	*(void**)projectInterface = s_projectInterfaceConstructionVtable;
	if (projectInterface != 0) projectInterface = (char*)projectInterface - 0x10;
	g_project = (int)projectInterface;
	*(void**)self = __vt__9CProjectG;
	AsCProjectG(self)->interfaceVtable = __vt__9CProjectG + kPrimaryInterfaceVtableSlot;
	AsCProjectG(self)->secondaryInterfaceVtable = __vt__9CProjectG + kSecondaryInterfaceVtableSlot;
	AsCProjectG(self)->frameUpdateFlags = 0;
	(*(int*)&AsCProjectG(self)->projectNameTag) = 0;
	AsCProjectG(self)->projectNameLongLength = 0;
	AsCProjectG(self)->projectNameHeap = 0;
	AsCProjectG(self)->string1Tag = 0;
	AsCProjectG(self)->string1LongLength = 0;
	AsCProjectG(self)->string1Heap = 0;
	AsCProjectG(self)->string2Tag = 0;
	AsCProjectG(self)->string2LongLength = 0;
	AsCProjectG(self)->string2Heap = 0;
	AsCProjectG(self)->string3Tag = 0;
	AsCProjectG(self)->string3LongLength = 0;
	AsCProjectG(self)->string3Heap = 0;
	// Phase 2: establish an empty device pool before creating dependent services.
	AsCProjectG(self)->deviceBuckets = 0;
	OSInitMutex((OSMutex*)((char*)self + kDeviceMutexOffset));
	OSLockMutex((OSMutex*)((int)self + kDeviceMutexOffset));
	fn_801CA034_init(self, kInitialDeviceNodeCount);
	OSUnlockMutex((OSMutex*)((int)self + kDeviceMutexOffset));
	AsCProjectG(self)->projectFlags = 0;
	g_currentTimeMilliseconds = OS_GetTimeMilliseconds();
	srand(g_currentTimeMilliseconds);
	AsCProjectG(self)->deviceFactory = 0;
	AsCProjectG(self)->deviceRegistrations = 0;
	AsCProjectG(self)->unk_28 = 0; // Additional device-registration state; only initialization is known here.
	// Phase 3: assign the title using the existing small-string representation.
	char* initialTitle = golfTaskStrings;
	int titleLength;
	if ((AsCProjectG(self)->projectNameTag >> 31) == 0) {
		titleLength = AsCProject(self)->projectNameTagByte & 0x7f;
	} else {
		titleLength = AsCProjectG(self)->projectNameLongLength;
	}
	int allocatorStorage;
	char allocatorArgument = *(char*)&allocatorStorage;
	String_ReplaceRange((void*)((int)self + 0x2c), 0, titleLength, initialTitle, initialTitle + strlen(initialTitle), &allocatorArgument);
	AsCProjectG(self)->unk_38 = 0;
	AsCProjectG(self)->unk_39 = 0;
	AsCProjectG(self)->restoreInputPending = 0;
	// Phase 4: create device/resource services, capture the clock origin, then
	// create the shared document, options, and task manager in dependency order.
	(*(void**)&AsCProjectG(self)->deviceFactory) = new N120();
	(*(void**)&AsCProjectG(self)->deviceRegistrations) = new N84();
	g_resourceManager = (int)new N548();
	AsCProjectG(self)->clockOriginMilliseconds = 0;
	AsCProjectG(self)->clockOriginMilliseconds = ((VObj801CA034*)self)->vf6();
	AsCProjectG(self)->unk_AC = 4; // Initial frame-clock setting; the consuming code is outside this unit.
	new Nb98();
	new N14();
	CMainFrame_ResetElapsedTimeAndCaptureValue(self);
	AsCProjectG(self)->timeScale = kDefaultProjectTimeScale;
	new N3c();
	AsCProjectG(self)->unk_BC = 0;
	return self;
}
struct CInputManager {
	void* vtable0;
	char padding04[0x0c];
	int inputHead;
	int inputCursor;
	int inputBlocks;
	int freeInputNodes;
	int inputBatchSize;
	int inputHashMask;
	int inputBucketCount;
	int* inputBuckets;
	char mutex[0xb0];
	float inputTimeScale;
	char inputStateByte;
	unsigned char inputEnabled;
	// Publish input services, initialize their locked pool, then reset pointer state.
	CInputManager() {
		BaseObject_ctor(this);
		vtable0 = (void*)s_inputManagerConstructionVtable;
		g_inputActionManager = (int)this;
		vtable0 = __vt__13CInputManager;
		inputBuckets = 0;
		OSInitMutex((OSMutex*)mutex);
		{
			WInstanceLock g((OSMutex*)mutex);
			inputHead = 0;
			freeInputNodes = 0;
			inputBlocks = 0;
			inputBatchSize = kInitialInputNodeCount;
			inputCursor = 0;
			inputBucketCount = kInitialInputNodeCount;
			int cap = inputBucketCount;
			if (cap > 0) {
				inputBuckets = (int*)appMallocUni__FUl(0x80);
				inputHashMask = inputBucketCount - 1;
				for (int i = 0; i < inputBucketCount; i++) inputBuckets[i] = 0;
			} else {
				inputBuckets = 0;
			}
		}
		inputTimeScale = kDefaultProjectTimeScale;
		inputEnabled = 0;
		Pointer_Reset(this);
	}
};

// Construct the chat-message service in its fixed-size allocation.
struct CChatMsg8 { char d[0x14]; CChatMsg8() { __ct__8CChatMsgFv(this); } };
// Construct the mouse-cursor service in its fixed-size allocation.
struct CCursor8 { char d[0x10c]; CCursor8() { CMouseCursor_Ctor(this); } };
// Construct the head-icon service in its fixed-size allocation.
struct CHeadIcon8 { char d[0x108]; CHeadIcon8() { CHeadIcon_ctor(this); } };
// Construct the scene-effect service in its fixed-size allocation.
struct CPvsFxBox8 { char d[0x3bc]; CPvsFxBox8() { CPvsFxBox_ctor(this); } };

#pragma opt_common_subs off
// Bind shared inventory, create the view/input/UI services and enqueue the initial title task.
void CProjectG_InitializeGameObjects() {
	CSharedDoc_BindItemSelectionInventory((void*)g_sharedDoc);
	SharedDoc_InitDefaultCourseSettings((void*)g_sharedDoc);
	g_view = (int)ResourceManager_CreateWView((void*)g_resourceManager, 0);
	CInputManager* obj = new CInputManager();
	g_inputPointer = (int)obj;
	new CChatMsg8();
	CCheckDriveStatus_SetLatch();
	new CCursor8();
	new CHeadIcon8();
	new CPvsFxBox8();
	GolfTips_ResetSharedRegistryAndResources();
	Layout_SetHitTestingEnabled(1);
	AsCMouseCursor(g_mouseCursor)->hitTestMode = 0;
	CTask_RequestTransition((void*)g_taskManager, golfTaskStrings + kTitleTaskName, golfTaskStrings);
	TaskCommandQueue_AppendNamedRequest((void*)g_taskManager, 0, golfTaskStrings + kTitleMainName, kInitialTaskCommand, 0, 0, 0, 0);
}
#pragma opt_common_subs reset

// Return milliseconds elapsed since the project clock origin.
int CProjectG_vfunc_4(ProjectClockView* self) {
	return g_currentTimeMilliseconds - self->clockOrigin;
}

extern int g_view;
extern int g_resourceManager;
extern int g_taskManager;
extern int g_chatMsg;
extern CSharedDoc* g_sharedDoc;
extern int g_fx;
extern int g_mouseCursor;
extern int g_inputPointer;
extern int g_soundManager;
extern int g_headIcon;
extern void Object_DeleteVirtual(void*, int);
extern void RawFile_NoOpHook();
extern void ResourceManager_dtor(void*, int);
extern void Generic3DPool_dtor(void*, int);
extern void RefObjectList_ReleaseAllSharedObjects();
extern void SoundManager_ReleaseSoundBank(void*, const char*);
extern void SoundManager_Dtor(void*, int);
extern void appFreeUni__FPv(void*);
extern void BaseObject_dtor(void*, int);
extern void __dl__FPv(void*);
extern int __vt__9CProjectG[];

struct VBase {
	virtual void v0(int);
	virtual void v1();
	virtual void v2();
	virtual void v3();
	virtual void v4(int);
	virtual void v5();
	virtual void v6();
};
struct ProjectDestructionView {
	char padding00[0x2c];
	unsigned int projectNameTag;
	char padding30[4];
	void* projectNameHeap;
	char padding38[4];
	unsigned int string1Tag;
	char padding40[4];
	void* string1Heap;
	unsigned int string2Tag;
	char padding4c[4];
	void* string2Heap;
	unsigned int string3Tag;
	char padding58[4];
	void* string3Heap;
	char padding60[8];
	char lock68[8];
	void* head70;
	char padding74[0x10];
	void* deviceBuckets;
	char padding88[0x28];
	void* deviceFactory;
	void* deviceRegistrations;
};

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Destroy game services and device resources, free node/string storage, and release project base interfaces.
void* CProjectG_dtor(ProjectDestructionView* self, int shouldDelete) {
	// Phase 1: destroy view/UI/game/input services before sound/resource providers.
	void* next;
	void* sentinel;
	void* node;
	if (self != 0) {
		*(void**)self = __vt__9CProjectG;
		AsCProjectG(self)->interfaceVtable = __vt__9CProjectG + kPrimaryInterfaceVtableSlot;
		AsCProjectG(self)->secondaryInterfaceVtable = __vt__9CProjectG + kSecondaryInterfaceVtableSlot;
		if (g_view != 0) {
			Object_DeleteVirtual((void*)g_resourceManager, g_view);
			g_view = 0;
		}
		RawFile_NoOpHook();
		if (g_headIcon != 0) if (g_headIcon != 0) ((VBase*)g_headIcon)->v0(1);
		if (g_taskManager != 0) if (g_taskManager != 0) ((VBase*)g_taskManager)->v0(1);
		if (g_chatMsg != 0) if (g_chatMsg != 0) ((VBase*)g_chatMsg)->v0(1);
		if (g_mouseCursor != 0) if (g_mouseCursor != 0) ((VBase*)g_mouseCursor)->v0(1);
		if (g_sharedDoc != 0) if (g_sharedDoc != 0) ((VBase*)g_sharedDoc)->v0(1);
		if (g_option != 0) if (g_option != 0) ((VBase*)g_option)->v0(1);
		if (g_fx != 0) if (g_fx != 0) ((VBase*)g_fx)->v4(1);
		RefObjectList_ReleaseAllSharedObjects();
		if (g_inputPointer != 0) {
			if (g_inputPointer != 0) ((VBase*)g_inputPointer)->v0(1);
			g_inputPointer = 0;
		}
		if (g_soundManager != 0) {
			SoundManager_ReleaseSoundBank((void*)g_soundManager, golfTaskStrings + kPermanentSoundBank);
			SoundManager_ReleaseSoundBank((void*)g_soundManager, golfTaskStrings + kMenuSoundBank);
		}
		if (g_soundManager != 0) {
			SoundManager_Dtor((void*)g_soundManager, 1);
			g_soundManager = 0;
		}
		if (g_resourceManager != 0) { ResourceManager_dtor((void*)g_resourceManager, 1); g_resourceManager = 0; }
		// Phase 2: release device services, then their pool blocks and bucket storage.
		if (self->deviceFactory != 0) { Generic3DPool_dtor(self->deviceFactory, 1); self->deviceFactory = 0; }
		if (self->deviceRegistrations != 0) { __dl__FPv(self->deviceRegistrations); self->deviceRegistrations = 0; }
		if ((char*)&((Q801CA034*)self)->deviceHead != 0) {
			WInstanceLock outer((void*)((Q801CA034*)self)->deviceMutex);
			if (AsCProjectG(self)->allocatedDeviceBlocks != 0) {
				sentinel = (void*)((Q801CA034*)self)->deviceMutex;
				while ((node = AsCProjectG(self)->allocatedDeviceBlocks) != 0) {
					OSLockMutex((OSMutex*)sentinel);
					AsProjectListNode801C9600(AsProjectListNode801C9600(node)->next)->previous = AsProjectListNode801C9600(node)->previous;
					AsProjectListNode801C9600(AsProjectListNode801C9600(node)->previous)->next = AsProjectListNode801C9600(node)->next;
					next = AsProjectListNode801C9600(node)->next;
					if (next == node) next = 0;
					OSUnlockMutex((OSMutex*)sentinel);
					AsCProjectG(self)->allocatedDeviceBlocks = next;
					appFreeUni__FPv(node);
				}
			}
			if (self->deviceBuckets != 0) { appFreeUni__FPv(self->deviceBuckets); self->deviceBuckets = 0; }
		}
		// Note: the '(char*)&field != 0' parts of the four tests below are always true (address of a member); the
		// real condition is tag bit 31 set = string data lives on the heap. Kept in this form to reproduce the original code.
		// Phase 3: release heap-backed strings and base interfaces; inline strings
		// need no free. shouldDelete > 0 additionally frees the project allocation.
		if (((char*)&AsCProjectG(self)->string3Tag) != 0 && (self->string3Tag >> 31) != 0) appFreeUni__FPv(self->string3Heap);
		if (((char*)&AsCProjectG(self)->string2Tag) != 0 && (self->string2Tag >> 31) != 0) appFreeUni__FPv(self->string2Heap);
		if (((char*)&AsCProjectG(self)->string1Tag) != 0 && (self->string1Tag >> 31) != 0) appFreeUni__FPv(self->string1Heap);
		if (((char*)&AsCProjectG(self)->projectNameTag) != 0 && (self->projectNameTag >> 31) != 0) appFreeUni__FPv(self->projectNameHeap);
		if (((char*)&AsCProjectG(self)->interfaceVtable) != 0) {
			g_project = 0;
			BaseObject_dtor((void*)&AsCProjectG(self)->interfaceVtable, 0);
		}
		CMainFrame_dtor(self, 0);
		if (shouldDelete > 0) __dl__FPv(self);
	}
	return self;
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. With "#pragma ipa file" the code is generated at
   the end of the file, so force_active must still be on there: a push/pop around the entry drops the flag, the linker
   dead-strips the entry and the initialiser never runs. */
#pragma force_active on
STATIC_INIT_ENTRY_NC(".CRT$XCU80462190", _ctors_u801C9600_0, GridSurface_InitializeAngleConstant);
