#include <decomp/static_init.h>
#include "classes/WView.h"
#include "classes/CGolfTask.h"
#define AsCGolfTask(p) ((CGolfTask*)(p))
#include "classes/CGridLayout.h"
#include "classes/CSharedDoc.h"
#define AsCGrid(p) ((CGridLayout*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "sdk_protos.h"
#include "game/types.h"
#include "game/wvector.h"
#include "game/golfball.h"
#include <decomp/cw_bitfields.h>
// CGrid_BuildCells / selection (0x801718A4 unit): physics singleton ground type, radius and state.

// Packed strings verified by global research rounds 5 and 7; field-derived byte addresses preserve scheduling.
struct GridStringPool { char actorName[9], slopeBitmap[11], onePointMode[4], screenActor[8]; };
struct GridTaskStringPool { char reserved00[0xB]; char ruleActor[9]; char reserved14[0x37B]; char taskClass[10]; };
#define GRID_OFFSET(type, field) ((int)&((type*)0)->field)
enum GridState {
    kGridOutOfRange = 1, kGridWideArea = 2, kGridHidden = 4,
    kGridShowSelection = 8, kGridGeometryDirty = 0x10
};
enum GridMessage {
    kGridRotate = 0, kGridSetOrigin = 1, kGridShow = 2, kGridHide = 3,
    kGridQuerySize = 4, kGridSetHideOverride = 5, kGridSetProfileScale = 6,
    kGridRefreshProfile = 7, kGridSetHeading = 0x2717
};
enum GridGeometry {
    kGridCellBytes = sizeof(CGridCell), kGridRowBytes = sizeof(CGridCell) * 10,
    kGridMarkerCapacity = 180, kGridLineVertexFormat = 0xF001,
    kGridMarkerVertexFormat = 0x11F001, kGroundGreen = 2,
    kQuadPrimitive = 0x80, kTriangleStripPrimitive = 0x98, kLinePrimitive = 0xA8
};
// Counts are grid sample points (quads/intervals use count-1), not screen pixels.
// Variant 3 selects dense green behavior only under the additional ground/override tests below;
// its wider game-mode meaning is unresolved. Spacing and cull distances use engine world units.
enum GridPreset {
    kDenseGreenVariant = 3,
    kNearGridColumns = 8, kNearGridRows = 16,
    kDenseGridColumns = 10, kDenseGridRows = 11,
    kWideGridColumns = 10, kWideGridRows = 11
};
// Literal aliases retain the existing constant-pool references in this matching unit.
#define GRID_NEAR_SPACING_WORLD 6.4f
#define GRID_DENSE_SPACING_WORLD 3.2f
#define GRID_WIDE_SPACING_WORLD 12.8f
struct GridCommand {
    unsigned int reserved; int code;
    union { void* payload; unsigned int value; int signedValue; };
    void* countOutput;
};
struct GridModeDescriptor {
    char reserved00[0xA]; unsigned char modeVariant;
    char reserved0B[0xD]; char modeName[1];
};
// Instance data contract: descriptor+0xA chooses the variant, descriptor+0x18 contains mode tags;
// instance+0x48 chooses profile sampling scale, +0x4C bit 0 forces dirty geometry, +0x50 overrides
// dense markers. The one-point tag adds extraRows/extraColumns/extraSpacing wherever queried.
// Repeated singleton/tag reads are retained; do not cache them across calls without re-verifying code.
struct GridInstanceState {
    char reserved00[4]; GridModeDescriptor* descriptor;
    char reserved08[0x40]; unsigned char markerScaleMode;
    char reserved49[3]; unsigned int gridUpdateFlags;
    unsigned char markerModeOverride;
};
struct GridTreeLinks { void* left; void* right; };
struct GridHoleRecord {
    char reserved00[0xC]; unsigned int pinPosition[3]; char reserved18[8];
};
struct GridPlayerRecord { char reserved00[0x1C]; float position[3]; };

// Shared declarations for the original CGrid translation unit.
typedef Vec3f V3;

class V2C {
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
    virtual void v2C(void*);
};

struct NativeBase80201D18 {
    char storage[0x18];
    NativeBase80201D18();
    ~NativeBase80201D18();
};

struct NativeMember80201D18 {
    char storage[4];
    // Preserve the empty native member-construction step; no storage is initialized here.
    NativeMember80201D18() {}
    ~NativeMember80201D18();
};

struct CGrid : NativeBase80201D18, NativeMember80201D18 {
    char storage[0x1864];
    CGrid();
};

struct GridTexVertex {
    char positionAndColor[0x10];
    float u;
    float v;
};

struct GridVectorBits { unsigned x, y, z; };

struct Vtx {
	float x;
	float y;
	float z;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

struct GridOverlayStorage {
    char reserved00[0x1880]; Vtx vertices[4];
};
struct Pair {
	unsigned short a;
	unsigned short b;
};

struct CGridUpdate {
	virtual int vf0();
	virtual int vf1();
	virtual int vf2();
	virtual int vf3();
	virtual int vf4();
	virtual int vf5();
	virtual int vf6();
	virtual int vf7();
	virtual void releaseResources();
	virtual int vf9();
	virtual int vf10();
	virtual int vf11();
	virtual int vf12();
	virtual int vf13();
	virtual int vf14();
};

struct CDev {
	virtual int vf0(int);
	virtual int vf1(int);
	virtual int vf2(int);
	virtual int vf3(int);
	virtual int vf4(int);
	virtual int vf5(int);
	virtual int vf6(int);
	virtual int vf7(int);
	virtual int vf8(int);
	virtual int vf9(int);
	virtual int vf10(int);
	virtual int vf11(int);
	virtual int vf12(int);
	virtual int vf13(int);
	virtual int vf14(int);
	virtual int vf15(int);
	virtual int vf16(int);
	virtual int vf17(int);
	virtual int vf18(int);
	virtual int vf19(int);
	virtual int vf20(int);
	virtual int vf21(int);
	virtual int vf22(int);
	virtual int vf23(int);
	virtual int vf24(int);
	virtual int vf25(int);
	virtual int vf26(int);
	virtual int vf27(int);
	virtual int vf28(int);
	virtual int vf29(int);
};

struct HeightRow { char head[0x2c]; float y; char tail[0xe8]; };

struct RefCounted { CW_BITS2( unsigned int count : 16, unsigned int flags : 16) };

extern "C" {

extern char s_golfTaskTypeInfo;
extern const char slopeGridStrings[];
extern void* g_resourceManager;
extern const float kFloat_6_2831855;
extern float s_golfTaskFullTurnConstantStorage;
extern float kFloat_0_grid;
extern float kFloat_1_Grid;
extern float kFloat_neg1_Grid;
extern char s_taskTypeInfo;
extern void* g_sceneManager;
extern void* g_sharedDoc;
extern const char s_golfTaskFactoryName[];
extern char __vt__5CGrid[];
extern const char golfTaskActorStrings[];
void IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void (*)(void*), const char*);
void CGolfTask_Create(void*);
void __ct__18NativeBase80201D18Fv(void*);
void CGrid_CreateBuffers(void*);
void ResourceManager_ReleaseId(void*, void*);
void CGrid_ResetCellsAndColor(void*);
void* CGolfTask_LoadActorsCallback(void*, void*, void*);
void* CGolfTask_dtor(void*, int);
void CTask_vfunc_19(void*);
void MPBase1_DrawIfInitialized(void*);
void GolfTask_NoOpStatus();
int SharedDoc_GetCourseId();
void* TaskActorTable_FindByName(void*, const char*);
void CTask_DisplayActorsByPriority(void*);
void CommonDialogRegistry_CloseEligibleDialogs();
void ResourcePreloadQueue_AddRequest(void*, int, int, float, void*, float, const char*, int, int, int);
void CGolfTask_RequestRuleResetHoleBallWind(void*);
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
void CubeResourceStorage_dtor(void*, int);
void VertexBuffer_Create(int, int, int, int, void**, int);
void IndexBuffer_Create(int, int, int, int, void**, int, int);
void Resource_GetOffsetPtr(void*, int, int, void*, int);
void GraphicsBuffer_Flush(void*);
void IndexBuffer_GetOffsetPtr(void*, int, int, void*, int);
void IndexBuffer_Flush(void*);
void appFreeUni__FPv(void*);

extern float kFloat_128_Grid;
extern float kFloat_6_4_SlopeGrid;
extern float kFloat_3_2_SlopeGrid;
extern float kFloat_320_Grid;
extern float kFloat_neg0_45_Grid;
extern float kFloat_0_45_Grid;
extern float kFloat_8_Grid;
extern float kFloat_0_5_Grid;
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
void* GolfBall__Fv();
void* InstanceTable_Get();
void* CQuadTree_GetInstance();
float Magnitude__7WVectorCFv(void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __pl__FRC7WVectorRC7WVector(V3*, V3*, V3*);
void __as__7WVectorFRC7WVector(void*, void*);
void __ml__FfRC7WVector(V3*, V3*, float);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(V3*, void*, V3*, int, int, int);
float CQuadTree_GetGreenPos(void*, void*);
extern const char s_cameraTransform[];
void Vector_CrossProduct(V3*, V3*, V3*);
void Matrix_SetAxisRotation(V3*, float, int);
void Vec3_DivideAssign(V3*, float);
float __ml__FRC7WVectorRC7WVector(void*, void*);
void CGrid_UpdateSelectionAndProfile(void*);

extern void* s_gridTypeInfo;
extern float s_gridFullTurnConstantStorage;
extern unsigned char kIdentityMatrix4x4[];
extern unsigned int s_gridRenderStateTables[];
extern void* g_view;
extern float kFloat_0_4_GridMarker;
extern float kFloat_0_2_Grid;
void CGrid_BuildCells(void* self);
void CGrid_SetupRenderState(void* self);
void __dt__18NativeBase80201D18Fv(void* self, int deleting);
void __dl__FPv(void* self);
void Engine_SetRenderStateList(void* data, int count);
void Engine_SetTextureStageStateList(int, void* data, int count);
void Engine_SetTextureStageParameterList(int, void* data, int count);
void Matrix_ExpandAffineTo4x4(void* self, void* out);
void SetTransform(int type, void* matrix);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageResource(int, int);
void CGrid_Draw(void* self);
void* __nwa__FUl(unsigned int);
void IActor_DispatchMessage(void*, void*, const char*, int, int, int, float*, int);
void __dla__FPv(void*);
void Vector_CopyComponents(void*, void*);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetVertexStream(int, void*, int, int);
void Engine_SetIndexBuffer(void*, int);
void Engine_DrawIndexedPrimitive(int, int, int, int, int, int);
void Engine_SetTexturePalette(int, int);
void DrawPrimitiveUP(int, int, void*, int);
// Find a mode-name tag in the active instance descriptor.
static inline char* findName(const char* n, unsigned char* dummy) {
    void* state = InstanceTable_Get();
    return strstr(((GridInstanceState*)state)->descriptor->modeName, n);
}

// Add the mode-specific grid extension only when the active mode contains the requested tag.
static inline int gridCount(unsigned char* p, const char* name, int countOffset, int extraOffset) {
    char* hit = strstr(((GridInstanceState*)InstanceTable_Get())->descriptor->modeName, name);
    int count = *(int*)(p + countOffset);
    int extra;
    if (hit == 0) extra = 0;
    else extra = *(int*)(p + extraOffset);
    return count + extra;
}

// Display task actors, close eligible dialogs and send rule messages before releasing active pause resources.
void CGolfTask_DisplayActorsByPriority(void* this_) {
    CTask_DisplayActorsByPriority(this_);
    CommonDialogRegistry_CloseEligibleDialogs();
    char* p = static_cast<char*>(this_);
    void* manager = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned int flags = (*(unsigned int*)&AsCGolfDoc(manager)->gameStateFlags);
    // The literal OR makes this condition always true; retain the original rule notification.
    if (((flags & 0x00020000) | 0x04000000) != 0) {
        unsigned int args[8];
        args[0] = 0;
        args[1] = 0x28;
        args[2] = 0;
        args[3] = 0;
        args[4] = 0;
        args[5] = 0;
        args[6] = 0;
        args[7] = 0;
        void* obj = TaskActorTable_FindByName(this_, (golfTaskActorStrings + GRID_OFFSET(GridTaskStringPool, ruleActor)));
        if (obj) {
            ((V2C*)obj)->v2C(args);
        }
    }
    manager = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    flags = (*(unsigned int*)&AsCGolfDoc(manager)->gameStateFlags);
    if (flags & 0x80) {
        unsigned int args[8];
        args[0] = 0;
        args[1] = 0x29;
        args[2] = 0;
        args[3] = 0;
        args[4] = 0;
        args[5] = 0;
        args[6] = 0;
        args[7] = 0;
        void* obj = TaskActorTable_FindByName(this_, (golfTaskActorStrings + GRID_OFFSET(GridTaskStringPool, ruleActor)));
        if (obj) {
            ((V2C*)obj)->v2C(args);
        }
    }
    manager = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    flags = (*(unsigned int*)&AsCGolfDoc(manager)->gameStateFlags);
    if (flags & 1) GolfTask_NoOpStatus();
    if ((void*)AsCGolfTask(p)->messageActive)
    MPBase1_DrawIfInitialized((void*)AsCGolfTask(p)->pauseScoreMenu);
}

// Finish the task callback, release its loading drawable and invoke the no-op status hook.
void CGolfTask_DrawLoadingScreen(void* this_) {
    CTask_vfunc_19(this_);
    void* ptr = (void*)AsCGolfTask(this_)->loadingDraw;
    if (ptr != 0)
    MPBase1_DrawIfInitialized(ptr);
    GolfTask_NoOpStatus();
}

// Invoke the no-op status hook and send message 0x1A to the GolfRule actor.
void CGolfTask_RequestRuleResetHoleBallWind(void* this_) {
    GolfTask_NoOpStatus();
    unsigned int args[8];
    args[0] = 0;
    args[1] = 0x1A;
    args[2] = 0;
    args[3] = 0;
    args[4] = 0;
    args[5] = 0;
    args[6] = 0;
    args[7] = 0;
    void* obj = TaskActorTable_FindByName(this_, (golfTaskActorStrings + GRID_OFFSET(GridTaskStringPool, ruleActor)));
    if (obj) {
        ((V2C*)obj)->v2C(args);
    }
}

// Return the golf task runtime type descriptor.
void* CGolfTask_GetRTTI() {
    return &s_golfTaskTypeInfo;
}

#pragma use_lmw_stmw on
// Free a binary tree in postorder using the original three-level unrolled traversal.
void GolfTaskTree_FreeSubtreeNodes(int owner, void* root) {
    void* leftBranch = ((GridTreeLinks*)root)->left;
    if (leftBranch) {
        void* leftLeft = ((GridTreeLinks*)leftBranch)->left;
        if (leftLeft) {
            void* leftLeftLeft = ((GridTreeLinks*)leftLeft)->left;
            if (leftLeftLeft) {
                if (((GridTreeLinks*)leftLeftLeft)->left)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)leftLeftLeft)->left);
                if (((GridTreeLinks*)leftLeftLeft)->right)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)leftLeftLeft)->right);
                appFreeUni__FPv(leftLeftLeft);
            }
            void* leftLeftRight = ((GridTreeLinks*)leftLeft)->right;
            if (leftLeftRight) {
                if (((GridTreeLinks*)leftLeftRight)->left)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)leftLeftRight)->left);
                if (((GridTreeLinks*)leftLeftRight)->right)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)leftLeftRight)->right);
                appFreeUni__FPv(leftLeftRight);
            }
            appFreeUni__FPv(leftLeft);
        }
        void* leftRight = ((GridTreeLinks*)leftBranch)->right;
        if (leftRight) {
            void* leftRightLeft = ((GridTreeLinks*)leftRight)->left;
            if (leftRightLeft) {
                if (((GridTreeLinks*)leftRightLeft)->left)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)leftRightLeft)->left);
                if (((GridTreeLinks*)leftRightLeft)->right)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)leftRightLeft)->right);
                appFreeUni__FPv(leftRightLeft);
            }
            void* leftRightRight = ((GridTreeLinks*)leftRight)->right;
            if (leftRightRight) {
                if (((GridTreeLinks*)leftRightRight)->left)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)leftRightRight)->left);
                if (((GridTreeLinks*)leftRightRight)->right)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)leftRightRight)->right);
                appFreeUni__FPv(leftRightRight);
            }
            appFreeUni__FPv(leftRight);
        }
        appFreeUni__FPv(leftBranch);
    }
    void* rightBranch = ((GridTreeLinks*)root)->right;
    if (rightBranch) {
        void* r30d = ((GridTreeLinks*)rightBranch)->left;
        if (r30d) {
            void* rightLeft = ((GridTreeLinks*)r30d)->left;
            if (rightLeft) {
                if (((GridTreeLinks*)rightLeft)->left)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)rightLeft)->left);
                if (((GridTreeLinks*)rightLeft)->right)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)rightLeft)->right);
                appFreeUni__FPv(rightLeft);
            }
            void* rightLeftRight = ((GridTreeLinks*)r30d)->right;
            if (rightLeftRight) {
                if (((GridTreeLinks*)rightLeftRight)->left)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)rightLeftRight)->left);
                if (((GridTreeLinks*)rightLeftRight)->right)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)rightLeftRight)->right);
                appFreeUni__FPv(rightLeftRight);
            }
            appFreeUni__FPv(r30d);
        }
        void* r30e = ((GridTreeLinks*)rightBranch)->right;
        if (r30e) {
            void* rightRight = ((GridTreeLinks*)r30e)->left;
            if (rightRight) {
                if (((GridTreeLinks*)rightRight)->left)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)rightRight)->left);
                if (((GridTreeLinks*)rightRight)->right)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)rightRight)->right);
                appFreeUni__FPv(rightRight);
            }
            void* rightRightRight = ((GridTreeLinks*)r30e)->right;
            if (rightRightRight) {
                if (((GridTreeLinks*)rightRightRight)->left)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)rightRightRight)->left);
                if (((GridTreeLinks*)rightRightRight)->right)
                GolfTaskTree_FreeSubtreeNodes(owner, ((GridTreeLinks*)rightRightRight)->right);
                appFreeUni__FPv(rightRightRight);
            }
            appFreeUni__FPv(r30e);
        }
        appFreeUni__FPv(rightBranch);
    }
    appFreeUni__FPv(root);
}
#pragma use_lmw_stmw reset

// Initialize the task constant, type descriptor and actor factory registration.
void CGolfTask_RegisterType() {
    s_golfTaskFullTurnConstantStorage = kFloat_6_2831855;
    IntPair_Set(&s_golfTaskTypeInfo, (golfTaskActorStrings + GRID_OFFSET(GridTaskStringPool, taskClass)), &s_taskTypeInfo);
    void* task = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(task, CGolfTask_Create, s_golfTaskFactoryName);
}

// Adjust the secondary interface pointer before forwarding the task callback.
void* CGolfTask_vfunc_14Thunk4(void* this_, void* arg1, void* arg2) {
    return CGolfTask_LoadActorsCallback(static_cast<char*>(this_) - 4, arg1, arg2);
}

// Adjust the interface pointer before forwarding the task destructor callback.
void* CGolfTask_Vfunc1Thunk8(void* this_, int forwarded1) {
    return CGolfTask_dtor(static_cast<char*>(this_) - 8, forwarded1);
}

} // extern "C"

// Construct base interfaces, clear resource handles and allocate grid buffers.
CGrid::CGrid() {
    void* this_ = this;
    *reinterpret_cast<const char**>(this_) = __vt__5CGrid;
    AsCGrid(this_)->renderVtable = const_cast<char*>(__vt__5CGrid + 0x3C);
    AsCGrid(this_)->bitmapId = 0;
    AsCGrid(this_)->secondaryResourceId = 0;
    AsCGrid(this_)->vertexBuffer = 0;
    AsCGrid(this_)->indexBuffer = 0;
    AsCGrid(this_)->markerVertexBuffer = 0;
    AsCGrid(this_)->markerIndexBuffer = 0;
    AsCGrid(this_)->unk_8 = 1;
    CGrid_CreateBuffers(this_);
}

extern "C" {

// Release both grid bitmap resource IDs when the resource manager is available.
void CGrid_OnDestroy(void* this_) {
    if (g_resourceManager && AsCGrid(this_)->bitmapId) {
        ResourceManager_ReleaseId(g_resourceManager, AsCGrid(this_)->bitmapId);
        AsCGrid(this_)->bitmapId = 0;
    }
    if (g_resourceManager && AsCGrid(this_)->secondaryResourceId) {
        ResourceManager_ReleaseId(g_resourceManager, AsCGrid(this_)->secondaryResourceId);
        AsCGrid(this_)->secondaryResourceId = 0;
    }
}

#pragma opt_propagation off
// Read an optional float or use the supplied fallback without changing its load semantics.
inline float GridDefaultFloat(const float* value, const float& fallback) {
    return value ? *value : fallback;
}

#pragma scheduling off
// Initialize quad UVs/link records, reset selection/color and queue the grid actor preload.
void CGrid_OnPreLoadInit(void* this_) {
    int r0;
    void* r4;
    void* r5;
    void* r6;
    void* r7;
    char* p = static_cast<char*>(this_);
    float _f0;
    const float* optional0 = 0;
    _f0 = GridDefaultFloat(optional0, kFloat_0_grid);
    AsCGrid(p)->quadVertex0U = _f0;
    float uv1;
    const float* optional1 = 0;
    uv1 = GridDefaultFloat(optional1, kFloat_1_Grid);
    AsCGrid(p)->quadVertex0V = uv1;
    const float* optional2 = 0;
    _f0 = GridDefaultFloat(optional2, kFloat_0_grid);
    AsCGrid(p)->quadVertex1U = _f0;
    AsCGrid(p)->quadVertex1V = kFloat_0_grid;
    AsCGrid(p)->quadVertex2U = kFloat_1_Grid;
    const float* optional3 = 0;
    uv1 = GridDefaultFloat(optional3, kFloat_1_Grid);
    AsCGrid(p)->quadVertex2V = uv1;
    AsCGrid(p)->quadVertex3U = kFloat_1_Grid;
    AsCGrid(p)->quadVertex3V = kFloat_0_grid;
    float f0;
    float f1;
    r7 = p + GRID_OFFSET(CGridLayout, pad_1788);
    f1 = kFloat_0_grid;
    r6 = p + GRID_OFFSET(CGridLayout, pad_17A8[24]);
    f0 = kFloat_neg1_Grid;
    r5 = p + GRID_OFFSET(CGridLayout, pad_1818[24]);
    r4 = p + GRID_OFFSET(CGridLayout, pad_17E0[24]);
    r0 = 0;
    AsCGrid(p)->quadVertex0 = r7;
    AsCGrid(p)->quadVertex1 = r6;
    AsCGrid(p)->quadVertex3 = r5;
    AsCGrid(p)->quadVertex2 = r4;
    AsCGrid(p)->quadVertexListTerminator = reinterpret_cast<void*>(r0);
    AsCGrid(p)->heading = f1;
    AsCGrid(p)->profileScale = f0;
    CGrid_ResetCellsAndColor(this_);
    if (this_) this_ = p + 0x18;
    const char* name = slopeGridStrings;
    void* manager = g_sceneManager;
    float one = kFloat_1_Grid;
    void* render = this_;
    float zero = kFloat_0_grid;
    ResourcePreloadQueue_AddRequest(manager, 1, 0, one, render, zero, name, 0, 0, 0);
}
#pragma opt_propagation reset
#pragma scheduling reset

// Load the slope marker bitmap.
void CGrid_OnLoad(void* this_) {
    AsCGrid(this_)->bitmapId =
    ResourceManager_LoadBitmap(g_resourceManager, (slopeGridStrings + GRID_OFFSET(GridStringPool, slopeBitmap)), 0, 0, 0);
}

// Forward the inherited reload calling convention to the grid reset routine.
void CGrid_OnReload() {
    reinterpret_cast<void (*)()>(CGrid_ResetCellsAndColor)();
}

// Clear all cell marks and selection state, then choose the course/variant palette; course 13 falls through.
void CGrid_ResetCellsAndColor(void* this_) {
    // Phase 1: clear persistent ball marks and reset selection/hide latches.
    unsigned char* p = static_cast<unsigned char*>(this_);
    unsigned char* cellCursor = p;
    for (int i = 0; i < 28; i += 2) {
        for (int j = 0; j < 15; j++) {
            ((CGridCell*)(cellCursor + GRID_OFFSET(CGridLayout, cells)))->marked = 0;
            cellCursor += kGridCellBytes;
        }
    }
    unsigned int flags = AsCGrid(p)->flags;
    AsCGrid(p)->selectionUpdated = 0;
    AsCGrid(p)->hideEnabled = 0;
    AsCGrid(p)->flags = flags & ~kGridHidden;
    // Phase 2: select RGBA bytes by course and palette variant, preserving course-13 fallthrough.
    unsigned char courseId = static_cast<unsigned char>(SharedDoc_GetCourseId());
    switch (courseId) {
        case 11: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=6;
                AsCGrid(p)->green=72;
                AsCGrid(p)->blue=95;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=15;
                AsCGrid(p)->green=55;
                AsCGrid(p)->blue=115;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=45;
                AsCGrid(p)->green=170;
                AsCGrid(p)->blue=160;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=6;
                AsCGrid(p)->green=72;
                AsCGrid(p)->blue=95;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 12: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0:
                case 1:
                case 2: AsCGrid(p)->red=251;
                AsCGrid(p)->green=184;
                AsCGrid(p)->blue=23;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=251;
                AsCGrid(p)->green=184;
                AsCGrid(p)->blue=23;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 13: {
            // This palette is overwritten by the following course-0/1 case.
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=40;
                AsCGrid(p)->green=95;
                AsCGrid(p)->blue=125;
                AsCGrid(p)->alpha=255;
                break;
                case 1:
                case 2: AsCGrid(p)->red=20;
                AsCGrid(p)->green=75;
                AsCGrid(p)->blue=100;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=40;
                AsCGrid(p)->green=95;
                AsCGrid(p)->blue=125;
                AsCGrid(p)->alpha=255;
                break;
            }
        }
        case 0:
        case 1: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=6;
                AsCGrid(p)->green=67;
                AsCGrid(p)->blue=90;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=25;
                AsCGrid(p)->green=195;
                AsCGrid(p)->blue=225;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=25;
                AsCGrid(p)->green=150;
                AsCGrid(p)->blue=200;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=6;
                AsCGrid(p)->green=67;
                AsCGrid(p)->blue=90;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 2: case 3: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=20;
                AsCGrid(p)->green=80;
                AsCGrid(p)->blue=60;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=20;
                AsCGrid(p)->green=180;
                AsCGrid(p)->blue=240;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=20;
                AsCGrid(p)->green=130;
                AsCGrid(p)->blue=190;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=20;
                AsCGrid(p)->green=80;
                AsCGrid(p)->blue=60;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 4: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=6;
                AsCGrid(p)->green=62;
                AsCGrid(p)->blue=85;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=6;
                AsCGrid(p)->green=190;
                AsCGrid(p)->blue=220;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=6;
                AsCGrid(p)->green=160;
                AsCGrid(p)->blue=220;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=6;
                AsCGrid(p)->green=62;
                AsCGrid(p)->blue=85;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 5: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=14;
                AsCGrid(p)->green=50;
                AsCGrid(p)->blue=100;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=30;
                AsCGrid(p)->green=180;
                AsCGrid(p)->blue=140;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=30;
                AsCGrid(p)->green=164;
                AsCGrid(p)->blue=120;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=14;
                AsCGrid(p)->green=50;
                AsCGrid(p)->blue=100;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 6: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0:
                case 1:
                case 2: AsCGrid(p)->red=175;
                AsCGrid(p)->green=165;
                AsCGrid(p)->blue=50;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=175;
                AsCGrid(p)->green=165;
                AsCGrid(p)->blue=50;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 7: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=6;
                AsCGrid(p)->green=60;
                AsCGrid(p)->blue=70;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=50;
                AsCGrid(p)->green=180;
                AsCGrid(p)->blue=220;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=6;
                AsCGrid(p)->green=140;
                AsCGrid(p)->blue=190;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=6;
                AsCGrid(p)->green=60;
                AsCGrid(p)->blue=70;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 8: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=35;
                AsCGrid(p)->green=80;
                AsCGrid(p)->blue=115;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=20;
                AsCGrid(p)->green=70;
                AsCGrid(p)->blue=130;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=20;
                AsCGrid(p)->green=70;
                AsCGrid(p)->blue=100;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=35;
                AsCGrid(p)->green=80;
                AsCGrid(p)->blue=115;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 9: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=40;
                AsCGrid(p)->green=80;
                AsCGrid(p)->blue=90;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=40;
                AsCGrid(p)->green=90;
                AsCGrid(p)->blue=120;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=40;
                AsCGrid(p)->green=50;
                AsCGrid(p)->blue=90;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=40;
                AsCGrid(p)->green=80;
                AsCGrid(p)->blue=90;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        case 10: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0: AsCGrid(p)->red=30;
                AsCGrid(p)->green=80;
                AsCGrid(p)->blue=100;
                AsCGrid(p)->alpha=255;
                break;
                case 1: AsCGrid(p)->red=40;
                AsCGrid(p)->green=20;
                AsCGrid(p)->blue=126;
                AsCGrid(p)->alpha=255;
                break;
                case 2: AsCGrid(p)->red=10;
                AsCGrid(p)->green=170;
                AsCGrid(p)->blue=220;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=30;
                AsCGrid(p)->green=80;
                AsCGrid(p)->blue=100;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
        default: {
            int paletteVariant = *(unsigned char*)&AsCSharedDoc(g_sharedDoc)->pad_9C6[0];
            switch (paletteVariant) {
                case 0:
                case 1:
                case 2: AsCGrid(p)->red=20;
                AsCGrid(p)->green=55;
                AsCGrid(p)->blue=20;
                AsCGrid(p)->alpha=255;
                break;
                default: AsCGrid(p)->red=20;
                AsCGrid(p)->green=55;
                AsCGrid(p)->blue=20;
                AsCGrid(p)->alpha=255;
                break;
            }
            break;
        }
    }
}

// Recreate grid line and slope-marker buffers and fill the marker UVs/quad indices.
// Grid owns one reference to each buffer; mapped CPU pointers are borrowed until the matching flush.
void CGrid_CreateBuffers(void* this_) {
    // Phase 1: replace line vertex/index buffers and the textured marker vertex buffer.
    char* p = static_cast<char*>(this_);
    RefCounted* ref = (*(RefCounted**)&AsCGrid(p)->vertexBuffer);
    if (ref) {
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        AsCGrid(p)->vertexBuffer = 0;
    }
    VertexBuffer_Create(0xD20, 0, kGridLineVertexFormat, 0, reinterpret_cast<void**>(p + GRID_OFFSET(CGridLayout, vertexBuffer)), 0);
    ref = (*(RefCounted**)&AsCGrid(p)->indexBuffer);
    if (ref) {
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        AsCGrid(p)->indexBuffer = 0;
    }
    IndexBuffer_Create(0x614, 0, 3, 0, reinterpret_cast<void**>(p + GRID_OFFSET(CGridLayout, indexBuffer)), 0, 0);
    ref = (*(RefCounted**)&AsCGrid(p)->markerVertexBuffer);
    if (ref) {
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        AsCGrid(p)->markerVertexBuffer = 0;
    }
    VertexBuffer_Create(0x4380, 0, kGridMarkerVertexFormat, 0, reinterpret_cast<void**>(p + GRID_OFFSET(CGridLayout, markerVertexBuffer)), 0);
    // Phase 2: seed four UV corners per marker, then upload quad indices in 0,1,3,2 order.
    void* verts;
    Resource_GetOffsetPtr(AsCGrid(p)->markerVertexBuffer, 0, 0, &verts, 0);
    for (int i = 0; i < kGridMarkerCapacity; i++) {
        for (int j = 0; j < 4; j++) {
            reinterpret_cast<GridTexVertex*>(verts)[i * 4 + j].u =
            static_cast<float>(j & 1);
            reinterpret_cast<GridTexVertex*>(verts)[i * 4 + j].v =
            static_cast<float>(j >> 1);
        }
    }
    GraphicsBuffer_Flush(AsCGrid(p)->markerVertexBuffer);
    ref = (*(RefCounted**)&AsCGrid(p)->markerIndexBuffer);
    if (ref) {
        if (--ref->count == 0) CubeResourceStorage_dtor(ref, 1);
        AsCGrid(p)->markerIndexBuffer = 0;
    }
    IndexBuffer_Create(0x5A0, 0, 3, 0, reinterpret_cast<void**>(p + GRID_OFFSET(CGridLayout, markerIndexBuffer)), 0, 0);
    unsigned short* idx;
    IndexBuffer_GetOffsetPtr(AsCGrid(p)->markerIndexBuffer, 0, 0, &idx, 0);
    int off = 0;
    for (int n = 0; n < kGridMarkerCapacity; n++, off += 8) {
        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(idx) + off + 0) = n << 2;
        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(idx) + off + 2) = (n << 2) + 1;
        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(idx) + off + 4) = (n << 2) + 3;
        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(idx) + off + 6) = (n << 2) + 2;
    }
    IndexBuffer_Flush(AsCGrid(p)->markerIndexBuffer);
}

#pragma push
#pragma opt_strength_reduction off
// Choose a green/near/far grid footprint, project cells onto terrain and position slope markers between neighbors.
// Footprint map: green or pin distance <128 -> 8x16 at 6.4; dense green variant 3 -> 10x11 at 3.2;
// otherwise -> 10x11 at 12.8, built immediately around origin in world X/Z axes.
// Near footprints are rebuilt by OnProcess around the ball with heading-rotated axes.
// One-point mode adjusts near counts/spacing using the stored deltas; preserve that separate contract.
void CGrid_BuildCells(void* self) {
    V3 horizontalPinOffset;
    unsigned char *rowCursor;
    unsigned char *cell;
    unsigned char *nextRowCell;
    V3 pinPosition;
    V3 raisedMarkerPosition;
    V3 markerGroundPosition;
    V3 markerProjectionInput;
    V3 markerWorldPosition;
    V3 scaledMarkerOffset;
    V3 markerOffset;
    V3 rowDisplacement;
    V3 columnDisplacement;
    V3 markerLift;
    V3 initialColumnAxis;
    V3 initialRowAxis;
    V3 cellLift;
    V3 cellProjectionInput;
    V3 cellGroundPosition;
    V3 raisedCellPosition;
    V3 localCellOffset;
    V3 cellWorldPosition;
    V3 cellProjectionCopy;
    unsigned char *markerRow;
    const char* modeTag;
    int columnOrRowIndex;
    int markerRowIndex;
    // Phase 1: choose footprint from ball ground type, horizontal pin distance and instance variant.
    unsigned char* golfDoc=(unsigned char*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char* gridBase=(unsigned char*)self;
    void* playerRecord=GolfPlayer_GetEntryByIndex(golfDoc,AsCGolfDoc(golfDoc)->currentPlayer);
    golfDoc=(unsigned char*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char* holeRecord=(unsigned char*)AsCGolfDoc(golfDoc)->holeRecords+(AsCGolfDoc(golfDoc)->currentHoleIndex<<5);
    unsigned pinXBits = ((GridHoleRecord*)holeRecord)->pinPosition[0];
    unsigned pinYBits = ((GridHoleRecord*)holeRecord)->pinPosition[1];
    ((GridVectorBits*)&pinPosition)->x = pinXBits;
    ((GridVectorBits*)&pinPosition)->y = pinYBits;
    ((GridVectorBits*)&pinPosition)->z = ((GridHoleRecord*)holeRecord)->pinPosition[2];
    __mi__FRC7WVectorRC7WVector(&horizontalPinOffset,&pinPosition,(V3*)((GridPlayerRecord*)playerRecord)->position);
    horizontalPinOffset.y=kFloat_0_grid;
    if (((CGolfBall*)GolfBall__Fv())->m_groundType==kGroundGreen || Magnitude__7WVectorCFv(&horizontalPinOffset)<kFloat_128_Grid) {
        AsCGrid(gridBase)->flags&=~kGridWideArea;
        if (((CGolfBall*)GolfBall__Fv())->m_groundType!=kGroundGreen || ((GridInstanceState*)InstanceTable_Get())->descriptor->modeVariant!=kDenseGreenVariant || ((GridInstanceState*)InstanceTable_Get())->markerModeOverride!=0) {
            AsCGrid(gridBase)->columns=kNearGridColumns;
            AsCGrid(gridBase)->rows=kNearGridRows;
            AsCGrid(gridBase)->spacing=GRID_NEAR_SPACING_WORLD;
            AsCGrid(gridBase)->extraColumns=-2;
            AsCGrid(gridBase)->extraRows=5;
            AsCGrid(gridBase)->extraSpacing=0.0f;
        } else {
            AsCGrid(gridBase)->columns=kDenseGridColumns;
            AsCGrid(gridBase)->rows=kDenseGridRows;
            AsCGrid(gridBase)->spacing=GRID_DENSE_SPACING_WORLD;
            AsCGrid(gridBase)->extraColumns=-4;
            AsCGrid(gridBase)->extraRows=0;
            AsCGrid(gridBase)->extraSpacing=GRID_DENSE_SPACING_WORLD;
        }
    } else {
        const float minSlopeOffset=-0.45f;
        const float markerHalf=0.5f;
        AsCGrid(gridBase)->flags|=kGridWideArea;
        AsCGrid(gridBase)->columns=kWideGridColumns;
        AsCGrid(gridBase)->rows=kWideGridRows;
        AsCGrid(gridBase)->spacing=GRID_WIDE_SPACING_WORLD;
        AsCGrid(gridBase)->extraColumns=0;
        AsCGrid(gridBase)->extraRows=0;
        AsCGrid(gridBase)->extraSpacing=0.0f;
        // Phase 2 (wide only): project centered world-X/Z sample points, lifting Y by ball radius.
        unsigned char *cellRow;
        cellRow=gridBase + GRID_OFFSET(CGridLayout, cells);
        rowCursor=cellRow;
        for(columnOrRowIndex=0;columnOrRowIndex<AsCGrid(gridBase)->rows;rowCursor+=kGridRowBytes,columnOrRowIndex++) {
            cell=rowCursor;
            for(int column=0;column<AsCGrid(gridBase)->columns;cell+=kGridCellBytes,column++) {
                int centeredColumn=column-(AsCGrid(gridBase)->columns/2);
                int centeredRow=columnOrRowIndex-(AsCGrid(gridBase)->rows/2);
                float spacing=AsCGrid(gridBase)->spacing;
                localCellOffset.y=0.0f;
                localCellOffset.z=spacing*(float)(centeredRow);
                localCellOffset.x=spacing*(float)(centeredColumn);
                __pl__FRC7WVectorRC7WVector(&cellWorldPosition,(V3*)(gridBase + GRID_OFFSET(CGridLayout, origin)),&localCellOffset);
                __as__7WVectorFRC7WVector(&cellProjectionCopy,&cellWorldPosition);
                float ballRadius=((CGolfBall*)GolfBall__Fv())->m_radius;
                cellLift.x=0.0f;
                cellLift.y=(ballRadius);
                cellLift.z=0.0f;
                *(GridVectorBits*)&cellProjectionInput=*(GridVectorBits*)&cellProjectionCopy;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&cellGroundPosition,CQuadTree_GetInstance(),&cellProjectionInput,1,0,1);
                __pl__FRC7WVectorRC7WVector(&raisedCellPosition,&cellGroundPosition,&cellLift);
                __as__7WVectorFRC7WVector((V3*)cell,&raisedCellPosition);
            }
        }
        // Phase 3: use world +Z rows/+X columns and place each slope marker inside its quad.
        // Fraction = 0.5 + clamp(8*atan(heightDelta/spacing), -0.45, 0.45), in each axis.
        // Thus markers stay within [0.05,0.95] of the interval before ground projection/radius lift.
        initialRowAxis.x=kFloat_0_grid;
        initialRowAxis.y=kFloat_0_grid;
        initialRowAxis.z=kFloat_1_Grid;
        __as__7WVectorFRC7WVector((V3*)(gridBase + GRID_OFFSET(CGridLayout, rowAxis)),&initialRowAxis);
        initialColumnAxis.x=kFloat_1_Grid;
        initialColumnAxis.y=kFloat_0_grid;
        initialColumnAxis.z=kFloat_0_grid;
        __as__7WVectorFRC7WVector((V3*)(gridBase + GRID_OFFSET(CGridLayout, columnAxis)),&initialColumnAxis);
        const char* name=(slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        void* ui = ((void* (*)(const void*))InstanceTable_Get)(name);
        char* active = strstr(((GridInstanceState*)ui)->descriptor->modeName, name);
        float cellSpacing = AsCGrid(gridBase)->spacing + (active ? AsCGrid(gridBase)->extraSpacing : kFloat_0_grid);
        modeTag=(slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        const float maxSlopeOffset=0.45f;
        markerRowIndex=0;
        markerRow=gridBase + GRID_OFFSET(CGridLayout, cells[0][0].markerPosition);
        while (markerRowIndex < gridCount(gridBase, modeTag, GRID_OFFSET(CGridLayout, rows), GRID_OFFSET(CGridLayout, extraRows)) - 1) {
            cell=((cellRow));
            columnOrRowIndex=0;
            rowCursor=markerRow;
            nextRowCell=gridBase + GRID_OFFSET(CGridLayout, cells)+((((markerRowIndex)+1)))*kGridRowBytes;
            while (columnOrRowIndex < gridCount(gridBase, modeTag, GRID_OFFSET(CGridLayout, columns), GRID_OFFSET(CGridLayout, extraColumns)) - 1) {
                const float slopeScale=8.0f;
                float neighbor=CQuadTree_GetGreenPos(CQuadTree_GetInstance(),(((nextRowCell))));
                float angle=slopeScale*(float)atan((CQuadTree_GetGreenPos(CQuadTree_GetInstance(),cell)-neighbor)/(cellSpacing));
                if(angle < minSlopeOffset) angle=minSlopeOffset;
                else if(angle>maxSlopeOffset) angle=maxSlopeOffset;
                float rowSlopeFraction=((markerHalf))+((angle));
                neighbor=CQuadTree_GetGreenPos(CQuadTree_GetInstance(),cellRow+(((columnOrRowIndex)+1))*kGridCellBytes);
                angle=slopeScale*(float)atan((CQuadTree_GetGreenPos(CQuadTree_GetInstance(),cell)-(neighbor))/(cellSpacing));
                if(angle < minSlopeOffset) angle=minSlopeOffset;
                else if(angle>maxSlopeOffset) angle=maxSlopeOffset;
                float columnSlopeFraction = markerHalf + angle;
                float ballRadius=((CGolfBall*)GolfBall__Fv())->m_radius;
                const float zero=0.0f;
                markerLift.x=(zero);
                markerLift.y=ballRadius;
                markerLift.z=zero;
                __ml__FfRC7WVector(&columnDisplacement,(V3*)(gridBase + GRID_OFFSET(CGridLayout, columnAxis)),(columnSlopeFraction));
                __ml__FfRC7WVector(&rowDisplacement,(V3*)(gridBase + GRID_OFFSET(CGridLayout, rowAxis)),(rowSlopeFraction));
                __pl__FRC7WVectorRC7WVector(&markerOffset,&rowDisplacement,&columnDisplacement);
                __ml__FfRC7WVector(&scaledMarkerOffset,&markerOffset,cellSpacing);
                __pl__FRC7WVectorRC7WVector(&markerWorldPosition,(V3*)cell,&scaledMarkerOffset);
                *(GridVectorBits*)&markerProjectionInput=*(GridVectorBits*)&markerWorldPosition;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&markerGroundPosition,CQuadTree_GetInstance(),&markerProjectionInput,1,0,1);
                __pl__FRC7WVectorRC7WVector(&raisedMarkerPosition,&markerGroundPosition,&markerLift);
                __as__7WVectorFRC7WVector((V3*)(rowCursor),&raisedMarkerPosition);
                cell+=kGridCellBytes;
                rowCursor+=kGridCellBytes;
                nextRowCell+=kGridCellBytes;
                columnOrRowIndex = ((((columnOrRowIndex)) + 1));
            }
            cellRow+=kGridRowBytes;
            markerRow+=kGridRowBytes;
            markerRowIndex = (1 + markerRowIndex);
        }
    }
    // Phase 4: discard stale marks and refresh the palette for the chosen footprint.
    CGrid_ResetCellsAndColor(self);
}
#pragma pop

#pragma push
#pragma opt_strength_reduction off
#pragma opt_propagation off
// Cull the grid, rebuild dirty cells and slope markers, update the screen profile and mark the ball cell.
void CGrid_OnProcess(void* self) {
    unsigned char* gridBase = (unsigned char*)self;
    // Projection scratch: world vectors for cell placement; declarations retain compiler lifetimes.
    V3 cameraOffsetXZ;
    V3 up;
    V3 axis;
    V3 cellLift;
    V3 cellProjectionInput;
    V3 cellGroundPosition;
    V3 raisedCellPosition;
    V3 cellColumnDisplacement;
    V3 cellRowDisplacement;
    V3 cellOffset;
    V3 scaledCellOffset;
    V3 cellWorldPosition;
    V3 cellProjectionCopy;
    // Marker scratch: separate from cell projection so the original copy/load order is visible.
    V3 markerLift;
    V3 columnDisplacement;
    V3 rowDisplacement;
    V3 markerOffset;
    V3 scaledMarkerOffset;
    V3 markerWorldPosition;
    V3 markerProjectionInput;
    V3 markerGroundPosition;
    V3 raisedMarkerPosition;
    // Selection scratch: world displacement, then grid-axis coordinates measured in intervals.
    V3 ballOffset;
    V3 headingMatrix[4];
    int ballColumn;
    int ballRow;
    int column;
    const char* name;
    int rowIndex;
    int columnIndex;
    unsigned char* nextRowCell;
    unsigned char* cell;
    unsigned char* markerRow;
    unsigned char* markerCell;
    unsigned char* rowCursor;
    unsigned char* cellRow;
    float markerZero;
    float neighbor;
    float columnSlopeFraction;
    float markerCenter;
    float maxSlopeOffset;
    float minSlopeOffset;
    float slopeScale;
    float rowSlopeFraction;
    float cellSpacing;
    float extraSpacing;
    float markerHalf;
    float zero;
    // Phase 1: gate tutorial/visibility, mark forced updates dirty, and cull in horizontal world distance.
    if (AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->tutorialStage < 4) return;
    if ((((GridInstanceState*)InstanceTable_Get())->gridUpdateFlags & 1) != 0) AsCGrid(gridBase)->flags |= kGridGeometryDirty;
    if ((AsCGrid(gridBase)->flags & kGridHidden) != 0) return;
    __mi__FRC7WVectorRC7WVector(&cameraOffsetXZ, (V3*)(s_cameraTransform + 0x24), (V3*)(gridBase + GRID_OFFSET(CGridLayout, origin)));
    cameraOffsetXZ.y = 0.0f;
    if (Magnitude__7WVectorCFv(&cameraOffsetXZ) > kFloat_320_Grid) {
        AsCGrid(gridBase)->flags |= kGridOutOfRange;
        return;
    }
    unsigned flags = AsCGrid(gridBase)->flags;
    unsigned cleared = flags & ~kGridOutOfRange;
    AsCGrid(gridBase)->flags = cleared;
    if ((flags & kGridWideArea) == 0) {
        if ((cleared & kGridGeometryDirty) != 0) {
            // Phase 2 (dirty near grid): rotate row/column axes about world Y; project points from ball.
            Matrix_SetAxisRotation(headingMatrix, AsCGrid(gridBase)->heading, 1);
            __as__7WVectorFRC7WVector((V3*)(gridBase + GRID_OFFSET(CGridLayout, rowAxis)), &headingMatrix[2]);
            axis.x = 0.0f;
            axis.y = kFloat_1_Grid;
            axis.z = 0.0f;
            Vector_CrossProduct(&up, &axis, (V3*)(gridBase + GRID_OFFSET(CGridLayout, rowAxis)));
            __as__7WVectorFRC7WVector((V3*)(gridBase + GRID_OFFSET(CGridLayout, columnAxis)), &up);
            cellRow = gridBase + GRID_OFFSET(CGridLayout, cells);
            name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            markerHalf = kFloat_0_5_Grid;
            zero = 0.0f;
            markerCell = cellRow;
            rowIndex = 0;
            while (rowIndex < gridCount(gridBase, name, GRID_OFFSET(CGridLayout, rows), GRID_OFFSET(CGridLayout, extraRows))) {
                rowCursor = markerCell;
                column = 0;
                while (column < gridCount(gridBase, name, GRID_OFFSET(CGridLayout, columns), GRID_OFFSET(CGridLayout, extraColumns))) {
                    int extraColumnCount;
                    if (0 != strstr(((GridInstanceState*)InstanceTable_Get())->descriptor->modeName, name)) extraColumnCount = AsCGrid(gridBase)->extraColumns;
                    else extraColumnCount = 0;
                    if (0 != strstr(((GridInstanceState*)InstanceTable_Get())->descriptor->modeName, name)) extraSpacing = AsCGrid(gridBase)->extraSpacing;
                    else extraSpacing = 0.0f;
                    int effectiveColumnCount = AsCGrid(gridBase)->columns + extraColumnCount;
                    __ml__FfRC7WVector(&cellColumnDisplacement, (V3*)(gridBase + GRID_OFFSET(CGridLayout, columnAxis)), (markerHalf + (float)column) - (float)(effectiveColumnCount / 2));
                    __ml__FfRC7WVector(&cellRowDisplacement, (V3*)(gridBase + GRID_OFFSET(CGridLayout, rowAxis)), (float)rowIndex);
                    __pl__FRC7WVectorRC7WVector(&cellOffset, &cellRowDisplacement, &cellColumnDisplacement);
                    __ml__FfRC7WVector(&scaledCellOffset, &cellOffset, AsCGrid(gridBase)->spacing + extraSpacing);
                    __pl__FRC7WVectorRC7WVector(&cellWorldPosition, (V3*)GolfBall__Fv(), &scaledCellOffset);
                    __as__7WVectorFRC7WVector(&cellProjectionCopy, &cellWorldPosition);
                    float ballRadius = ((CGolfBall*)GolfBall__Fv())->m_radius;
                    cellLift.x = zero;
                    cellLift.y = ballRadius;
                    cellLift.z = zero;
                    *(GridVectorBits*)&cellProjectionInput = *(GridVectorBits*)&cellProjectionCopy;
                    GetGroundPoint__9CQuadTreeF7WVectorbPib(&cellGroundPosition, CQuadTree_GetInstance(), &cellProjectionInput, 1, 0, 1);
                    __pl__FRC7WVectorRC7WVector(&raisedCellPosition, &cellGroundPosition, &cellLift);
                    __as__7WVectorFRC7WVector((V3*)rowCursor, &raisedCellPosition);
                    rowCursor += kGridCellBytes;
                    column++;
                }
                markerCell += kGridRowBytes;
                rowIndex++;
            }
            // Phase 3: publish pin selection/profile, then rebuild slope markers from neighbor heights.
            CGrid_UpdateSelectionAndProfile(self);
            name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            cellSpacing = AsCGrid(gridBase)->spacing + (strstr(((GridInstanceState*)InstanceTable_Get())->descriptor->modeName, name) ? AsCGrid(gridBase)->extraSpacing : 0.0f);
            minSlopeOffset = kFloat_neg0_45_Grid;
            maxSlopeOffset = kFloat_0_45_Grid;
            slopeScale = kFloat_8_Grid;
            name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            markerCenter = kFloat_0_5_Grid;
            markerRow = gridBase + GRID_OFFSET(CGridLayout, cells[0][0].markerPosition);
            for (rowIndex = 0; rowIndex < AsCGrid(gridBase)->rows + (0 == findName(name, gridBase) ? 0 : AsCGrid(gridBase)->extraRows) - 1; cellRow += kGridRowBytes, markerRow += kGridRowBytes, rowIndex++)
            {
                cell = cellRow;
                markerCell = markerRow;
                nextRowCell = gridBase + GRID_OFFSET(CGridLayout, cells) + (rowIndex + 1) * kGridRowBytes;
                for (columnIndex = 0; columnIndex < AsCGrid(gridBase)->columns + (0 == findName(name, gridBase) ? 0 : AsCGrid(gridBase)->extraColumns) - 1; nextRowCell += kGridCellBytes, cell += kGridCellBytes, markerCell += kGridCellBytes, columnIndex++)
                {
                    neighbor = CQuadTree_GetGreenPos(CQuadTree_GetInstance(), nextRowCell);
                    float angle = slopeScale * (float)atan((CQuadTree_GetGreenPos(CQuadTree_GetInstance(), cell) - neighbor) / cellSpacing);
                    if (angle < minSlopeOffset) angle = minSlopeOffset;
                    else if (angle > maxSlopeOffset) angle = maxSlopeOffset;
                    rowSlopeFraction = markerCenter + angle;
                    neighbor = CQuadTree_GetGreenPos(CQuadTree_GetInstance(), cellRow + (columnIndex + 1) * kGridCellBytes);
                    angle = slopeScale * (float)atan((CQuadTree_GetGreenPos(CQuadTree_GetInstance(), cell) - neighbor) / cellSpacing);
                    if (angle < minSlopeOffset) angle = minSlopeOffset;
                    else if (angle > maxSlopeOffset) angle = maxSlopeOffset;
                    columnSlopeFraction = markerCenter + angle;
                    float markerRadius = ((CGolfBall*)GolfBall__Fv())->m_radius;
                    markerLift.x = 0.0f;
                    markerLift.y = markerRadius;
                    markerLift.z = 0.0f;
                    __ml__FfRC7WVector(&columnDisplacement, (V3*)(gridBase + GRID_OFFSET(CGridLayout, columnAxis)), columnSlopeFraction);
                    __ml__FfRC7WVector(&rowDisplacement, (V3*)(gridBase + GRID_OFFSET(CGridLayout, rowAxis)), rowSlopeFraction);
                    __pl__FRC7WVectorRC7WVector(&markerOffset, &rowDisplacement, &columnDisplacement);
                    __ml__FfRC7WVector(&scaledMarkerOffset, &markerOffset, cellSpacing);
                    __pl__FRC7WVectorRC7WVector(&markerWorldPosition, (V3*)cell, &scaledMarkerOffset);
                    *(GridVectorBits*)&markerProjectionInput = *(GridVectorBits*)&markerWorldPosition;
                    GetGroundPoint__9CQuadTreeF7WVectorbPib(&markerGroundPosition, CQuadTree_GetInstance(), &markerProjectionInput, 1, 0, 1);
                    __pl__FRC7WVectorRC7WVector(&raisedMarkerPosition, &markerGroundPosition, &markerLift);
                    __as__7WVectorFRC7WVector((V3*)markerCell, &raisedMarkerPosition);
                }
            }
            AsCGrid(gridBase)->flags &= ~kGridShowSelection;
        } else {
            AsCGrid(gridBase)->flags = cleared | kGridShowSelection;
        }
    }
    // Phase 4: geometry is current; optionally mark the ball's interval in the existing cell array.
    // Existing code checks upper limits only: callers must keep ball-axis indices nonnegative.
    AsCGrid(gridBase)->flags &= ~kGridGeometryDirty;
    if (((CGolfBall*)GolfBall__Fv())->m_state == 0) return;
    if (AsCGrid(gridBase)->hideEnabled == 0) return;
    __mi__FRC7WVectorRC7WVector(&ballOffset, (V3*)GolfBall__Fv(), (V3*)(gridBase + GRID_OFFSET(CGridLayout, cells)));
    ballOffset.y = 0.0f;
    {
        float extraSpacingForCell;
        name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        if (0 != findName(name, gridBase)) extraSpacingForCell = AsCGrid(gridBase)->extraSpacing;
        else extraSpacingForCell = 0.0f;
        Vec3_DivideAssign(&ballOffset, AsCGrid(gridBase)->spacing + extraSpacingForCell);
    }
    ballRow = (int)__ml__FRC7WVectorRC7WVector(&ballOffset, (V3*)(gridBase + GRID_OFFSET(CGridLayout, rowAxis)));
    ballColumn = (int)__ml__FRC7WVectorRC7WVector(&ballOffset, (V3*)(gridBase + GRID_OFFSET(CGridLayout, columnAxis)));
    {
        int extra;
        name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        char* hit = findName(name, gridBase);
        int cnt = AsCGrid(gridBase)->rows;
        if (hit) extra = AsCGrid(gridBase)->extraRows;
        else extra = 0;
        if (ballRow >= cnt + extra - 1) return;
    }
    {
        int extra;
        name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        char* hit = findName(name, gridBase);
        int cnt = AsCGrid(gridBase)->columns;
        if (hit) extra = AsCGrid(gridBase)->extraColumns;
        else extra = 0;
        if (ballColumn >= cnt + extra - 1) return;
    }
    unsigned char* block = gridBase + ballRow * kGridRowBytes;
    *(unsigned char*)((unsigned int)block + ballColumn * kGridCellBytes + GRID_OFFSET(CGridLayout, cells[0][0].marked)) = 1;
}
#pragma pop

// Upload/draw grid lines and slope-marker quads, then overlay marked and selected cells.
void CGrid_Draw(void* self) {
    char* rowp;
    int base;
    char* cellCursor;
    int row;
    int column;
    int lineCount;
    char* gridBase = (char*)self;
    if (((CGridUpdate*)self)->vf12() != 0) return;
    void* mgr = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    if (AsCGolfDoc(mgr)->tutorialStage >= 4) {
        unsigned int flags = AsCGrid(gridBase)->flags;
        if (!(flags & kGridOutOfRange) && !(flags & kGridHidden)) {
            const char* name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            void* ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
            char* extra = strstr(((GridInstanceState*)ui)->descriptor->modeName, name);
            int rows = AsCGrid(gridBase)->rows + (extra ? AsCGrid(gridBase)->extraRows : 0);
            ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
            extra = strstr(((GridInstanceState*)ui)->descriptor->modeName, name);
            int cols = AsCGrid(gridBase)->columns + (extra ? AsCGrid(gridBase)->extraColumns : 0);
            // Phase 1: flatten terrain samples into colored line vertices and horizontal/vertical edges.
            void* lineVertices;
            void* lineIndices;
            void* markerVertices;
            Resource_GetOffsetPtr(AsCGrid(gridBase)->vertexBuffer, 0, 0, &lineVertices, 0);
            {
                rowp = gridBase + GRID_OFFSET(CGridLayout, cells);
                row = 0;
                for (; row < rows; ++row) {
                    cellCursor = rowp;
                    base = row * cols;
                    for (column = 0; column < cols; ++column) {
                        Vector_CopyComponents(&((Vtx*)lineVertices)[column + base], cellCursor);
                        ((Vtx*)lineVertices)[column + base].a = AsCGrid(gridBase)->alpha;
                        ((Vtx*)lineVertices)[column + base].r = AsCGrid(gridBase)->red;
                        ((Vtx*)lineVertices)[column + base].g = AsCGrid(gridBase)->green;
                        ((Vtx*)lineVertices)[column + base].b = AsCGrid(gridBase)->blue;
                        cellCursor += kGridCellBytes;
                    }
                    rowp += kGridRowBytes;
                }
            }
            GraphicsBuffer_Flush(AsCGrid(gridBase)->vertexBuffer);
            lineCount = 0;
            IndexBuffer_GetOffsetPtr(AsCGrid(gridBase)->indexBuffer, 0, 0, &lineIndices, 0);
            {
                for (int row = 0; row < rows; ++row) {
                    int base = row * cols;
                    for (int column = 0; column < cols - 1; ++column) {
                        ((Pair*)lineIndices)[lineCount].a = column + base;
                        ((Pair*)lineIndices)[lineCount].b = column + base + 1;
                        ++lineCount;
                    }
                }
                for (int column = 0; column < cols; ++column) {
                    for (int row = 0; row < rows - 1; ++row) {
                        ((Pair*)lineIndices)[lineCount].a = column + row * cols;
                        ((Pair*)lineIndices)[lineCount].b = row * cols + (cols + column);
                        ++lineCount;
                    }
                }
            }
            IndexBuffer_Flush(AsCGrid(gridBase)->indexBuffer);
            CGrid_SetupRenderState(gridBase);
            Engine_SetFlexibleVertexFormat(kGridLineVertexFormat);
            Engine_SetVertexStream(0, AsCGrid(gridBase)->vertexBuffer, 0, 16);
            Engine_SetIndexBuffer(AsCGrid(gridBase)->indexBuffer, 0);
            Engine_SetTextureStageResource(0, 0);
            Engine_DrawIndexedPrimitive(kLinePrimitive, 0, 0, lineCount * 2, 0, lineCount);
            // Phase 2: build/draw world-X/Z marker quads; marked intervals are red.
            int markerVertexCount = 0;
            int useLargeMarker;
            const char* name3;
            if (((CGolfBall*)GolfBall__Fv())->m_groundType != kGroundGreen ||
            ((GridInstanceState*)InstanceTable_Get())->descriptor->modeVariant != kDenseGreenVariant ||
            ((GridInstanceState*)InstanceTable_Get())->markerModeOverride != 0 ||
            (name3 = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode)),
            ui = ((void* (*)(const char*))InstanceTable_Get)(name3),
            strstr(((GridInstanceState*)ui)->descriptor->modeName, name3)) != 0)
            useLargeMarker = 1;
            else useLargeMarker = 0;
            float scale = useLargeMarker ? kFloat_0_4_GridMarker : kFloat_0_2_Grid;
            Resource_GetOffsetPtr(AsCGrid(gridBase)->markerVertexBuffer, 0, 0, &markerVertices, 0);
            {
                struct MarkerVtx {
                    float x, y, z;
                    unsigned char r, g, b, a;
                    float u, v;
                };
                char* rowp;
                int offset;
                char* cellCursor;
                int row;
                int column;
                int k;
                rowp = gridBase;
                row = 0;
                offset = 0;
                for (; row < rows - 1; ++row) {
                    cellCursor = rowp;
                    for (column = 0; column < cols - 1; ++column) {
                        for (k = 0; k < 4; ++k) {
                            ((MarkerVtx*)((char*)markerVertices + offset))->x = AsCGrid(cellCursor)->cells[0][0].markerPosition[0] + ((k & 2) ? scale : -scale);
                            ((MarkerVtx*)((char*)markerVertices + offset))->y = AsCGrid(cellCursor)->cells[0][0].markerPosition[1];
                            ((MarkerVtx*)((char*)markerVertices + offset))->z = AsCGrid(cellCursor)->cells[0][0].markerPosition[2] + ((k & 1) ? -scale : scale);
                            if (AsCGrid(cellCursor)->cells[0][0].marked != 0) {
                                ((MarkerVtx*)((char*)markerVertices + offset))->r = 0xff;
                                ((MarkerVtx*)((char*)markerVertices + offset))->a = 0xff;
                                ((MarkerVtx*)((char*)markerVertices + offset))->b = 0;
                                ((MarkerVtx*)((char*)markerVertices + offset))->g = 0;
                            } else {
                                ((MarkerVtx*)((char*)markerVertices + offset))->a = AsCGrid(gridBase)->alpha;
                                ((MarkerVtx*)((char*)markerVertices + offset))->r = AsCGrid(gridBase)->red;
                                ((MarkerVtx*)((char*)markerVertices + offset))->g = AsCGrid(gridBase)->green;
                                ((MarkerVtx*)((char*)markerVertices + offset))->b = AsCGrid(gridBase)->blue;
                            }
                            offset += 24;
                            ++markerVertexCount;
                        }
                        cellCursor += kGridCellBytes;
                    }
                    rowp += kGridRowBytes;
                }
            }
            GraphicsBuffer_Flush(AsCGrid(gridBase)->markerVertexBuffer);
            CGrid_SetupRenderState(gridBase);
            Engine_SetFlexibleVertexFormat(kGridMarkerVertexFormat);
            Engine_SetVertexStream(0, AsCGrid(gridBase)->markerVertexBuffer, 0, 24);
            Engine_SetIndexBuffer(AsCGrid(gridBase)->markerIndexBuffer, 0);
            {
                int v = ((CDev*)((WView*)g_view)->renderTarget)->vf26((*(int*)&AsCGrid(gridBase)->bitmapId));
                Engine_SetTextureStageResource(0, v);
                v = ((CDev*)((WView*)g_view)->renderTarget)->vf27((*(int*)&AsCGrid(gridBase)->bitmapId));
                Engine_SetTexturePalette(0, v);
            }
            Engine_DrawIndexedPrimitive(kQuadPrimitive, 0, 0, markerVertexCount, 0, markerVertexCount / 4);
            // Phase 3: draw translucent white marked-cell overlays, then green pin selection.
            CGrid_SetupRenderState(gridBase);
            Engine_SetFlexibleVertexFormat(kGridLineVertexFormat);
            Engine_SetTextureStageResource(0, 0);
            {
                char* rowp;
                char* cellCursor;
                Vtx* qv;
                unsigned char* color;
                int row;
                int column;
                int k;
                rowp = gridBase;
                for (row = 0; row < rows - 1; ++row) {
                    cellCursor = rowp;
                    for (column = 0; column < cols - 1; ++column) {
                        if (AsCGrid(cellCursor)->cells[0][0].marked != 0) {
                            qv = (Vtx*)(gridBase + GRID_OFFSET(GridOverlayStorage, vertices));
                            color = (unsigned char*)gridBase;
                            for (k = 0; k < 4; ++k) {
                                Vector_CopyComponents(qv, gridBase + GRID_OFFSET(CGridLayout, cells) + (row + (k & 1)) * kGridRowBytes + (column + (k >> 1)) * kGridCellBytes);
                                color[GRID_OFFSET(GridOverlayStorage, vertices[0].a)] = 0x80;
                                color[GRID_OFFSET(GridOverlayStorage, vertices[0].b)] = 0xff;
                                color[GRID_OFFSET(GridOverlayStorage, vertices[0].g)] = 0xff;
                                color[GRID_OFFSET(GridOverlayStorage, vertices[0].r)] = 0xff;
                                color += 16;
                                ++qv;
                            }
                            DrawPrimitiveUP(kTriangleStripPrimitive, 2, gridBase + GRID_OFFSET(GridOverlayStorage, vertices), 16);
                        }
                        cellCursor += kGridCellBytes;
                    }
                    rowp += kGridRowBytes;
                }
            }
            flags = AsCGrid(gridBase)->flags;
            if (!(flags & kGridWideArea) && (flags & kGridShowSelection) && AsCGrid(gridBase)->selectedRow >= 0 && AsCGrid(gridBase)->selectedColumn >= 0) {
                Vtx* qv = (Vtx*)(gridBase + GRID_OFFSET(GridOverlayStorage, vertices));
                unsigned char* color = (unsigned char*)gridBase;
                for (int k = 0; k < 4; ++k) {
                    int row = AsCGrid(gridBase)->selectedRow + (k & 1);
                    int column = AsCGrid(gridBase)->selectedColumn + (k >> 1);
                    Vector_CopyComponents(&qv[k], gridBase + GRID_OFFSET(CGridLayout, cells) + row * kGridRowBytes + column * kGridCellBytes);
                    color[GRID_OFFSET(GridOverlayStorage, vertices[0].a)] = 0x50;
                    color[GRID_OFFSET(GridOverlayStorage, vertices[0].r)] = 0x00;
                    color[GRID_OFFSET(GridOverlayStorage, vertices[0].g)] = 0xff;
                    color[GRID_OFFSET(GridOverlayStorage, vertices[0].b)] = 0x00;
                    color += 16;
                }
                DrawPrimitiveUP(kTriangleStripPrimitive, 2, gridBase + GRID_OFFSET(GridOverlayStorage, vertices), 16);
            }
        }
    }
}

#pragma opt_propagation off
// Apply grid transform/visibility commands or return profile dimensions through message outputs.
void CGrid_HandleMsg(void* self, const void* command) {
    switch (((const GridCommand*)command)->code) {
        case kGridSetHeading: {
            AsCGrid(self)->heading = *(float*)((const GridCommand*)command)->payload;
            CGrid_BuildCells(self);
            AsCGrid(self)->flags |= kGridGeometryDirty;
            break;
        }
        case kGridRotate:
        AsCGrid(self)->heading += *(float*)((const GridCommand*)command)->payload;
        AsCGrid(self)->flags |= kGridGeometryDirty;
        break;
        case kGridSetOrigin: {
            void* p = ((const GridCommand*)command)->payload;
            if (p) (*(WVector*)&AsCGrid(self)->origin) = *(WVector*)p;
            CGrid_BuildCells(self);
            break;
        }
        case kGridQuerySize: {
            const char* name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            void* ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
            void* base = ((GridInstanceState*)ui)->descriptor;
            char* active = strstr(((GridModeDescriptor*)base)->modeName, name);
            float value = active ? AsCGrid(self)->extraSpacing : kFloat_0_grid;
            float sum = AsCGrid(self)->spacing + value;
            *(float*)((const GridCommand*)command)->payload = sum;
            const char* name2 = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
            base = ((GridInstanceState*)ui)->descriptor;
            int add;
            int cnt;
            char* active2 = strstr(((GridModeDescriptor*)base)->modeName, name2);
            cnt = AsCGrid(self)->rows;
            add = active2 ? AsCGrid(self)->extraRows : 0;
            *(int*)((const GridCommand*)command)->countOutput = cnt + add;
            break;
        }
        case kGridShow:
        AsCGrid(self)->flags &= ~kGridHidden;
        break;
        case kGridHide:
        if (AsCGrid(self)->hideEnabled == 0)
        AsCGrid(self)->flags |= kGridHidden;
        break;
        case kGridSetHideOverride: {
            unsigned int value = ((const GridCommand*)command)->value;
            AsCGrid(self)->hideEnabled = value != 0;
            break;
        }
        case kGridSetProfileScale: {
            int value = ((const GridCommand*)command)->signedValue;
            // Command values use the game's 3.2 world-unit scale for the profile display.
            AsCGrid(self)->profileScale = kFloat_3_2_SlopeGrid * (float)value;
            break;
        }
        case kGridRefreshProfile:
        CGrid_UpdateSelectionAndProfile(self);
        break;
    }
}
#pragma opt_propagation reset

// Project the pin into grid coordinates and send the screen actor an optional averaged height profile.
// Row/column axes are world-space unit directions. Dot products divided by spacing yield grid
// coordinates; column uses floor(x-0.5)+columnCount/2, row uses truncation. -1 marks upper overflow.
// Screen message 0xD receives distanceSteps, intervalCount, and optional heights[rows]. The height
// buffer is borrowed synchronously during dispatch and freed immediately afterward; it is not retained.
void CGrid_UpdateSelectionAndProfile(void* self) {
    // Phase 1: flatten pin-minus-ball to X/Z, then resolve grid coordinates and selection bounds.
    int centerColumn;
    const char* name;
    int profileRowCount;
    int distanceSteps;
    char* gridBase = (char*)self;
    float pinOffset[3];
    void* golfDoc = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char holeIndex = AsCGolfDoc(golfDoc)->currentHoleIndex;
    char* holeRecord = (char*)AsCGolfDoc(golfDoc)->holeRecords + holeIndex * 0x20;
    GridVectorBits vec = *(GridVectorBits*)((GridHoleRecord*)holeRecord)->pinPosition;
    void* ball = GolfBall__Fv();
    __mi__FRC7WVectorRC7WVector(pinOffset, &vec, ball);
    pinOffset[1] = kFloat_0_grid;
    name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
    void* ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    void* base = ((GridInstanceState*)ui)->descriptor;
    char* active = strstr(((GridModeDescriptor*)base)->modeName, name);
    float cellSpacing = AsCGrid(gridBase)->spacing + (active ? AsCGrid(gridBase)->extraSpacing : kFloat_0_grid);
    AsCGrid(gridBase)->selectedRow = (int)((*(WVector*)pinOffset * (*(WVector*)&AsCGrid(gridBase)->rowAxis)) / cellSpacing);
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    int extraColumns = strstr(((GridModeDescriptor*)base)->modeName, name) ? AsCGrid(gridBase)->extraColumns : 0;
    float columnCoordinate = floor((*(WVector*)pinOffset * (*(WVector*)&AsCGrid(gridBase)->columnAxis)) / cellSpacing - kFloat_0_5_Grid);
    int columnIndex = (int)columnCoordinate;
    int columnCount = AsCGrid(gridBase)->columns + extraColumns;
    AsCGrid(gridBase)->selectedColumn = columnIndex + columnCount / 2;
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    active = strstr(((GridModeDescriptor*)base)->modeName, name);
    int limit = *(int*)(gridBase + GRID_OFFSET(CGridLayout, rows)) + (active ? *(int*)(gridBase + GRID_OFFSET(CGridLayout, extraRows)) : 0);
    if (*(int*)(gridBase + GRID_OFFSET(CGridLayout, selectedRow)) > limit - 2) *(int*)(gridBase + GRID_OFFSET(CGridLayout, selectedRow)) = -1;
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    active = strstr(((GridModeDescriptor*)base)->modeName, name);
    limit = AsCGrid(gridBase)->columns + (active ? AsCGrid(gridBase)->extraColumns : 0);
    if (AsCGrid(gridBase)->selectedColumn > limit - 2) AsCGrid(gridBase)->selectedColumn = -1;
    AsCGrid(gridBase)->selectionUpdated = 1;
    // Phase 2: choose profile sample distance (6.4 for scale mode>1/one-point, otherwise 3.2).
    float scale;
    ui = InstanceTable_Get();
    if (((GridInstanceState*)ui)->markerScaleMode > 1) scale = kFloat_6_4_SlopeGrid;
    else {
        ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
        base = ((GridInstanceState*)ui)->descriptor;
        scale = strstr(((GridModeDescriptor*)base)->modeName, name) ? kFloat_6_4_SlopeGrid : kFloat_3_2_SlopeGrid;
    }
    distanceSteps = (int)((*(WVector*)pinOffset).Magnitude() / scale);
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    active = strstr(((GridModeDescriptor*)base)->modeName, name);
    profileRowCount = AsCGrid(gridBase)->rows + (active ? AsCGrid(gridBase)->extraRows : 0);
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    active = strstr(((GridModeDescriptor*)base)->modeName, name);
    int effectiveColumnCount = AsCGrid(gridBase)->columns + (active ? AsCGrid(gridBase)->extraColumns : 0);
    centerColumn = (int)(kFloat_0_5_Grid * (float)effectiveColumnCount);
    ball = GolfBall__Fv();
    if (!(((CGolfBall*)ball)->m_groundType == kGroundGreen &&
    (((GridInstanceState*)InstanceTable_Get())->descriptor->modeVariant == kDenseGreenVariant && centerColumn >= 1))) {
        IActor_DispatchMessage(gridBase, gridBase, (slopeGridStrings + GRID_OFFSET(GridStringPool, screenActor)), 0xd, distanceSteps, profileRowCount - 1, 0, 0);
        return;
    }
    // Phase 3 (dense green only): average the two center columns. First row uses two heights;
    // subsequent rows average four neighboring heights, then send/free the temporary profile.
    float* profileHeights = (float*)__nwa__FUl(profileRowCount * 4);
    int rightCenterColumnOffset = centerColumn * kGridCellBytes;
    int leftCenterColumnOffset = (centerColumn - 1) * kGridCellBytes;
    profileHeights[0] = kFloat_0_5_Grid * (((HeightRow*)(gridBase + leftCenterColumnOffset))[0].y + ((HeightRow*)(gridBase + rightCenterColumnOffset))[0].y);
    for (int i = 1; i < profileRowCount; ++i) {
        HeightRow* rightCenterHeights = (HeightRow*)(gridBase + rightCenterColumnOffset);
        HeightRow* leftCenterHeights = (HeightRow*)(gridBase + leftCenterColumnOffset);
        profileHeights[i] = 0.25f * (rightCenterHeights[i].y + (leftCenterHeights[i].y + (leftCenterHeights[i-1].y + rightCenterHeights[i-1].y)));
    }
    IActor_DispatchMessage(gridBase, gridBase, (slopeGridStrings + GRID_OFFSET(GridStringPool, screenActor)), 0xd, distanceSteps, profileRowCount - 1, profileHeights, 0);
    __dla__FPv(profileHeights);
}

#pragma use_lmw_stmw on
// Apply the grid render/texture tables and camera transform, then reset texture/vertex bindings.
void CGrid_SetupRenderState(void* self) {
    struct B24 {
        unsigned int v[24];
    };
    struct B12 {
        unsigned int v[12];
    };
    struct B10 {
        unsigned int v[10];
    };
    struct Tables {
        B24 first;
        B12 second;
        B10 third;
    };
    Tables* base = (Tables*)s_gridRenderStateTables;
    B24 first = base->first;
    B12 second = base->second;
    B10 third = base->third;
    Engine_SetRenderStateList(first.v, 12);
    Engine_SetTextureStageStateList(0, second.v, 6);
    Engine_SetTextureStageParameterList(0, third.v, 5);
    void* obj = ((WView*)g_view)->renderTarget;
    if (obj != 0) {
        ((CDev*)obj)->vf23(0x20200000);
        ((CDev*)obj)->vf24(0);
    }
    unsigned char matrix[64];
    Matrix_ExpandAffineTo4x4(((WView*)g_view)->inputTransform, matrix);
    SetTransform(0, matrix);
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetType7Resource(0);
    Engine_SetTextureStageResource(0, 0);
}
#pragma use_lmw_stmw off

// Return the grid runtime type descriptor.
void* CGrid_GetRTTI() {
    return &s_gridTypeInfo;
}

// Release grid resources and all four reference-counted buffers, destroy bases and optionally delete the object.
void* CGrid_dtor(void* self, int deleting) {
    if (self != 0) {
        *(void**)self = __vt__5CGrid;
        AsCGrid(self)->renderVtable = __vt__5CGrid + 0x3c;
        ((CGridUpdate*)self)->releaseResources();
        void* p = AsCGrid(self)->vertexBuffer;
        if (p != 0) {
            RefCounted* ref = (RefCounted*)p;
            --ref->count;
            if (ref->count == 0) CubeResourceStorage_dtor(p, 1);
            AsCGrid(self)->vertexBuffer = 0;
        }
        p = AsCGrid(self)->indexBuffer;
        if (p != 0) {
            RefCounted* ref = (RefCounted*)p;
            --ref->count;
            if (ref->count == 0) CubeResourceStorage_dtor(p, 1);
            AsCGrid(self)->indexBuffer = 0;
        }
        p = AsCGrid(self)->markerVertexBuffer;
        if (p != 0) {
            RefCounted* ref = (RefCounted*)p;
            --ref->count;
            if (ref->count == 0) CubeResourceStorage_dtor(p, 1);
            AsCGrid(self)->markerVertexBuffer = 0;
        }
        p = AsCGrid(self)->markerIndexBuffer;
        if (p != 0) {
            RefCounted* ref = (RefCounted*)p;
            --ref->count;
            if (ref->count == 0) CubeResourceStorage_dtor(p, 1);
            AsCGrid(self)->markerIndexBuffer = 0;
        }
        __dt__18NativeBase80201D18Fv(self, 0);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Initialize the grid translation unit's full-turn constant.
void CGrid_InitializeFullTurnConstant() {
    s_gridFullTurnConstantStorage = kFloat_6_2831855;
}

// Adjust the render interface pointer and forward to grid drawing.
void CGrid_DrawThunk18(void* self) {
    CGrid_Draw((char*)self - 0x18);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462134", _ctors_u801718A4_0, CGolfTask_RegisterType);
STATIC_INIT_ENTRY(".CRT$XCU80462134", _ctors_u801718A4_1, CGrid_InitializeFullTurnConstant);
#pragma pop

// Private views must retain the original 32-bit ABI offsets.
typedef char u801718A4_layout_check_0[(GRID_OFFSET(GridStringPool, onePointMode) == 0x14) ? 1 : -1];
typedef char u801718A4_layout_check_1[(GRID_OFFSET(GridStringPool, screenActor) == 0x18) ? 1 : -1];
typedef char u801718A4_layout_check_2[(GRID_OFFSET(GridTaskStringPool, taskClass) == 0x38F) ? 1 : -1];
typedef char u801718A4_layout_check_3[(GRID_OFFSET(GridOverlayStorage, vertices) == 0x1880) ? 1 : -1];
typedef char u801718A4_layout_check_4[(GRID_OFFSET(GridInstanceState, gridUpdateFlags) == 0x4C) ? 1 : -1];
