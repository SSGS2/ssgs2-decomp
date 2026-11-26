// The slope grid drawn around the ball, and the parts of CGolfTask that live in
// this unit: its actor-priority display override and the interface thunks.
//
// The grid samples the terrain into a cell array, builds and draws the grid
// lines and slope markers, and highlights the ball cell and the pin cell.
#include <decomp/static_init.h>
#include "classes/WView.h"
#include "classes/CGolfTask.h"
#include "classes/CGrid.h"
#include "classes/CSharedDoc.h"
#define AsCGrid(p) ((CGrid*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "sdk_protos.h"
#include "game/types.h"
#include "game/wvector.h"
#include "game/golfball.h"
#include <decomp/cw_bitfields.h>
#include "classes/CQuadTree.h"
#include "classes/CTask.h"

// Strings shared with the task actor factory, in the order the data stores them.
struct GridStringPool {
    char actorName[9], slopeBitmap[11], onePointMode[4], screenActor[8];
};
struct GridTaskStringPool {
    char reserved00[0xB];
    char ruleActor[9];
    char reserved14[0x37B];
    char taskClass[10];
};
#define GRID_OFFSET(type, field) ((int)&((type*)0)->field)
enum GridState {
    kGridOutOfRange = 1,
    kGridWideArea = 2,
    kGridHidden = 4,
    kGridShowSelection = 8,
    kGridGeometryDirty = 0x10
};
enum GridMessage {
    kGridRotate = 0,
    kGridSetOrigin = 1,
    kGridShow = 2,
    kGridHide = 3,
    kGridQuerySize = 4,
    kGridSetHideOverride = 5,
    kGridSetProfileScale = 6,
    kGridRefreshProfile = 7,
    kGridSetHeading = 0x2717
};
enum GridGeometry {
    kGridCellBytes = sizeof(CGridCell),
    kGridRowBytes = sizeof(CGridCell) * 10,
    kGridMarkerCapacity = 180,
    kGridLineVertexFormat = 0xF001,
    kGridMarkerVertexFormat = 0x11F001,
    kGroundGreen = 2,
    kQuadPrimitive = 0x80,
    kTriangleStripPrimitive = 0x98,
    kLinePrimitive = 0xA8
};
// Colour bytes of the four overlay vertices, addressed through the byte pointer
// that walks them.
enum GridOverlayColor {
    kOverlayRed = 0x188C,
    kOverlayGreen = 0x188D,
    kOverlayBlue = 0x188E,
    kOverlayAlpha = 0x188F
};
// Counts are grid sample points (quads/intervals use count-1), not screen pixels.
// Variant 3 selects the dense green behavior under the ground and override tests below.
// Spacing and cull distances use engine world units.
enum GridPreset {
    kDenseGreenVariant = 3,
    kNearGridColumns = 8,
    kNearGridRows = 16,
    kDenseGridColumns = 10,
    kDenseGridRows = 11,
    kWideGridColumns = 10,
    kWideGridRows = 11
};
// The spacings are written as literals so they load from the unit's constant pool.
#define GRID_NEAR_SPACING_WORLD 6.4f
#define GRID_DENSE_SPACING_WORLD 3.2f
#define GRID_WIDE_SPACING_WORLD 12.8f
struct GridCommand {
    unsigned int reserved;
    int code;
    union {
        void* payload;
        unsigned int value;
        int signedValue;
    };
    void* countOutput;
};
struct GridModeDescriptor {
    char reserved00[0xA];
    unsigned char modeVariant;
    char reserved0B[0xD];
    char modeName[1];
};
// Instance data contract: descriptor+0xA chooses the variant, descriptor+0x18 contains mode tags;
// instance+0x48 chooses profile sampling scale, +0x4C bit 0 forces dirty geometry, +0x50 overrides
// dense markers. The one-point tag adds extraRows/extraColumns/extraSpacing wherever queried.
// The singleton and its mode tag are read again at each use; caching them changes the code.
struct GridInstanceState {
    char reserved00[4];
    GridModeDescriptor* descriptor;
    char reserved08[0x40];
    unsigned char markerScaleMode;
    char reserved49[3];
    unsigned int gridUpdateFlags;
    unsigned char markerModeOverride;
};
struct GridTreeLinks {
    void* left;
    void* right;
};
struct GridHoleRecord {
    char reserved00[0xC];
    unsigned int pinPosition[3];
    char reserved18[8];
};
struct GridPlayerRecord {
    char reserved00[0x1C];
    float position[3];
};

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

struct GridTexVertex {
    char positionAndColor[0x10];
    float u;
    float v;
};

struct GridVectorBits {
    unsigned x, y, z;
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

struct RefCounted {
    CW_BITS2(unsigned int count : 16, unsigned int flags : 16)
};

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
void* IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void (*)(void*), const char*);
void CGolfTask_Create(void*);
void __ct__18NativeBase80201D18Fv(void*);
void ResourceManager_ReleaseId(void*, void*);
// The deleting destructor, called directly by the interface thunk below.
void* __dt__9CGolfTaskFv(void*, int);
void MPBase1_DrawIfInitialized(void*);
void GolfTask_NoOpStatus();
int SharedDoc_GetCourseId();
void* TaskActorTable_FindByName(void*, const char*);
void CommonDialogRegistry_CloseEligibleDialogs();
void ResourcePreloadQueue_AddRequest(void*, int, int, float, void*, float, const char*, int, int, int);
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
extern const char s_cameraTransform[];
void Vector_CrossProduct(V3*, V3*, V3*);
void Matrix_SetAxisRotation(V3*, float, int);
void Vec3_DivideAssign(V3*, float);
float __ml__FRC7WVectorRC7WVector(void*, void*);

extern void* s_gridTypeInfo;
extern float s_gridFullTurnConstantStorage;
extern unsigned char kIdentityMatrix4x4[];
extern unsigned int s_gridRenderStateTables[];
extern void* g_view;
extern float kFloat_0_4_GridMarker;
extern float kFloat_0_2_Grid;
void Engine_SetRenderStateList(void* data, int count);
void Engine_SetTextureStageStateList(int, void* data, int count);
void Engine_SetTextureStageParameterList(int, void* data, int count);
void Matrix_ExpandAffineTo4x4(void* self, void* out);
void SetTransform(int type, void* matrix);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageResource(int, int);
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

} // extern "C"

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
    if (hit == 0)
        extra = 0;
    else
        extra = *(int*)(p + extraOffset);
    return count + extra;
}

// Display the task's actors, close eligible dialogs and tell the GolfRule actor
// which pause resources are active.
void CGolfTask::DisplayActorsByPriority() {
    ((CTask*)this)->DisplayActorsByPriority();
    CommonDialogRegistry_CloseEligibleDialogs();
    CGolfDoc* golfDoc = (CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned int flags = golfDoc->gameStateFlags;
    // The constant bit makes this test always true; the original still sends the message.
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
        void* obj =
            TaskActorTable_FindByName(this, (golfTaskActorStrings + GRID_OFFSET(GridTaskStringPool, ruleActor)));
        if (obj) {
            ((V2C*)obj)->v2C(args);
        }
    }
    golfDoc = (CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    flags = golfDoc->gameStateFlags;
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
        void* obj =
            TaskActorTable_FindByName(this, (golfTaskActorStrings + GRID_OFFSET(GridTaskStringPool, ruleActor)));
        if (obj) {
            ((V2C*)obj)->v2C(args);
        }
    }
    golfDoc = (CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    flags = golfDoc->gameStateFlags;
    if (flags & 1)
        GolfTask_NoOpStatus();
    if (messageActive)
        MPBase1_DrawIfInitialized((void*)pauseScoreMenu);
}

// Finish the task callback, release its loading drawable and invoke the no-op status hook.
void CGolfTask::DrawLoadingScreen() {
    ((CTask*)this)->vfunc_19();
    void* ptr = (void*)loadingDraw;
    if (ptr != 0)
        MPBase1_DrawIfInitialized(ptr);
    GolfTask_NoOpStatus();
}

// Invoke the no-op status hook and send message 0x1A to the GolfRule actor.
void CGolfTask::RequestRuleResetHoleBallWind() {
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
    void* obj = TaskActorTable_FindByName(this, (golfTaskActorStrings + GRID_OFFSET(GridTaskStringPool, ruleActor)));
    if (obj) {
        ((V2C*)obj)->v2C(args);
    }
}

// Return the golf task runtime type descriptor.
void* CGolfTask::GetRTTI() {
    return &s_golfTaskTypeInfo;
}

#pragma use_lmw_stmw on
// Free a binary tree in postorder using the original three-level unrolled traversal.
extern "C" void GolfTaskTree_FreeSubtreeNodes(int owner, void* root) {
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
extern "C" void CGolfTask_RegisterType() {
    s_golfTaskFullTurnConstantStorage = kFloat_6_2831855;
    IntPair_Set(&s_golfTaskTypeInfo, (golfTaskActorStrings + GRID_OFFSET(GridTaskStringPool, taskClass)),
                &s_taskTypeInfo);
    void* task = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(task, CGolfTask_Create, s_golfTaskFactoryName);
}

// Adjust the secondary interface pointer before forwarding the task callback.
void CGolfTask::vfunc_14Thunk4(void* arg1, void* arg2) {
    ((CGolfTask*)((char*)this - 4))->LoadActorsCallback((int)arg1, (int)arg2);
}

// Adjust the interface pointer before forwarding the task destructor callback.
void* CGolfTask::Vfunc1Thunk8(int forwarded1) {
    return __dt__9CGolfTaskFv((char*)this - 8, forwarded1);
}

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
    CreateBuffers();
}

// Release both grid bitmap resource IDs when the resource manager is available.
void CGrid::OnDestroy() {
    if (g_resourceManager && bitmapId) {
        ResourceManager_ReleaseId(g_resourceManager, bitmapId);
        bitmapId = 0;
    }
    if (g_resourceManager && secondaryResourceId) {
        ResourceManager_ReleaseId(g_resourceManager, secondaryResourceId);
        secondaryResourceId = 0;
    }
}

#pragma opt_propagation off
// Return the pointed-to float, or the fallback when the pointer is null.
inline float GridDefaultFloat(const float* value, const float& fallback) {
    return value ? *value : fallback;
}

#pragma scheduling off
// Initialize quad UVs/link records, reset selection/color and queue the grid actor preload.
void CGrid::OnPreLoadInit() {
    // The named r0/r4..r7 slots fix the link-record setup's register allocation.
    int r0;
    void* r4;
    void* r5;
    void* r6;
    void* r7;
    char* p = (char*)this;
    CGrid* self = this;
    float _f0;
    const float* optional0 = 0;
    _f0 = GridDefaultFloat(optional0, kFloat_0_grid);
    quadVertices[0].u = _f0;
    float uv1;
    const float* optional1 = 0;
    uv1 = GridDefaultFloat(optional1, kFloat_1_Grid);
    quadVertices[0].v = uv1;
    const float* optional2 = 0;
    _f0 = GridDefaultFloat(optional2, kFloat_0_grid);
    quadVertices[1].u = _f0;
    quadVertices[1].v = kFloat_0_grid;
    quadVertices[2].u = kFloat_1_Grid;
    const float* optional3 = 0;
    uv1 = GridDefaultFloat(optional3, kFloat_1_Grid);
    quadVertices[2].v = uv1;
    quadVertices[3].u = kFloat_1_Grid;
    quadVertices[3].v = kFloat_0_grid;
    float f0;
    float f1;
    r7 = &quadVertices[0];
    f1 = kFloat_0_grid;
    r6 = &quadVertices[1];
    f0 = kFloat_neg1_Grid;
    r5 = &quadVertices[3];
    r4 = &quadVertices[2];
    r0 = 0;
    quadVertex0 = r7;
    quadVertex1 = r6;
    quadVertex3 = r5;
    quadVertex2 = r4;
    quadVertexListEnd = reinterpret_cast<void*>(r0);
    heading = f1;
    profileScale = f0;
    ResetCellsAndColor();
    // The original guards the interface-pointer adjustment with a null test.
    if (self)
        self = (CGrid*)(p + 0x18);
    const char* name = slopeGridStrings;
    void* manager = g_sceneManager;
    float one = kFloat_1_Grid;
    void* render = self;
    float zero = kFloat_0_grid;
    ResourcePreloadQueue_AddRequest(manager, 1, 0, one, render, zero, name, 0, 0, 0);
}
#pragma opt_propagation reset
#pragma scheduling reset

// Load the slope marker bitmap.
void CGrid::OnLoad() {
    bitmapId = ResourceManager_LoadBitmap(g_resourceManager,
                                          (slopeGridStrings + GRID_OFFSET(GridStringPool, slopeBitmap)), 0, 0, 0);
}

// Forward the inherited reload calling convention to the grid reset routine.
void CGrid::OnReload() {
    ResetCellsAndColor();
}

// Clear all cell marks and selection state, then choose the course/variant palette; course 13 falls through.
void CGrid::ResetCellsAndColor() {
    // Phase 1: clear persistent ball marks and reset selection/hide latches.
    unsigned char* p = reinterpret_cast<unsigned char*>(this);
    unsigned char* cellCursor = p;
    // cellCursor counts from the object base; the cell offset is added at each access.
    for (int i = 0; i < 28; i += 2) {
        for (int j = 0; j < 15; j++) {
            ((CGridCell*)(cellCursor + GRID_OFFSET(CGrid, cells)))->marked = 0;
            cellCursor += kGridCellBytes;
        }
    }
    unsigned int flags = this->flags;
    selectionUpdated = 0;
    hideEnabled = 0;
    this->flags = flags & ~kGridHidden;
    // Phase 2: select RGBA bytes by course and palette variant, preserving course-13 fallthrough.
    unsigned char courseId = static_cast<unsigned char>(SharedDoc_GetCourseId());
    switch (courseId) {
    case 11: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 6;
            green = 72;
            blue = 95;
            alpha = 255;
            break;
        case 1:
            red = 15;
            green = 55;
            blue = 115;
            alpha = 255;
            break;
        case 2:
            red = 45;
            green = 170;
            blue = 160;
            alpha = 255;
            break;
        default:
            red = 6;
            green = 72;
            blue = 95;
            alpha = 255;
            break;
        }
        break;
    }
    case 12: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
        case 1:
        case 2:
            red = 251;
            green = 184;
            blue = 23;
            alpha = 255;
            break;
        default:
            red = 251;
            green = 184;
            blue = 23;
            alpha = 255;
            break;
        }
        break;
    }
    case 13: {
        // This palette is overwritten by the following course-0/1 case.
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 40;
            green = 95;
            blue = 125;
            alpha = 255;
            break;
        case 1:
        case 2:
            red = 20;
            green = 75;
            blue = 100;
            alpha = 255;
            break;
        default:
            red = 40;
            green = 95;
            blue = 125;
            alpha = 255;
            break;
        }
    }
    case 0:
    case 1: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 6;
            green = 67;
            blue = 90;
            alpha = 255;
            break;
        case 1:
            red = 25;
            green = 195;
            blue = 225;
            alpha = 255;
            break;
        case 2:
            red = 25;
            green = 150;
            blue = 200;
            alpha = 255;
            break;
        default:
            red = 6;
            green = 67;
            blue = 90;
            alpha = 255;
            break;
        }
        break;
    }
    case 2:
    case 3: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 20;
            green = 80;
            blue = 60;
            alpha = 255;
            break;
        case 1:
            red = 20;
            green = 180;
            blue = 240;
            alpha = 255;
            break;
        case 2:
            red = 20;
            green = 130;
            blue = 190;
            alpha = 255;
            break;
        default:
            red = 20;
            green = 80;
            blue = 60;
            alpha = 255;
            break;
        }
        break;
    }
    case 4: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 6;
            green = 62;
            blue = 85;
            alpha = 255;
            break;
        case 1:
            red = 6;
            green = 190;
            blue = 220;
            alpha = 255;
            break;
        case 2:
            red = 6;
            green = 160;
            blue = 220;
            alpha = 255;
            break;
        default:
            red = 6;
            green = 62;
            blue = 85;
            alpha = 255;
            break;
        }
        break;
    }
    case 5: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 14;
            green = 50;
            blue = 100;
            alpha = 255;
            break;
        case 1:
            red = 30;
            green = 180;
            blue = 140;
            alpha = 255;
            break;
        case 2:
            red = 30;
            green = 164;
            blue = 120;
            alpha = 255;
            break;
        default:
            red = 14;
            green = 50;
            blue = 100;
            alpha = 255;
            break;
        }
        break;
    }
    case 6: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
        case 1:
        case 2:
            red = 175;
            green = 165;
            blue = 50;
            alpha = 255;
            break;
        default:
            red = 175;
            green = 165;
            blue = 50;
            alpha = 255;
            break;
        }
        break;
    }
    case 7: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 6;
            green = 60;
            blue = 70;
            alpha = 255;
            break;
        case 1:
            red = 50;
            green = 180;
            blue = 220;
            alpha = 255;
            break;
        case 2:
            red = 6;
            green = 140;
            blue = 190;
            alpha = 255;
            break;
        default:
            red = 6;
            green = 60;
            blue = 70;
            alpha = 255;
            break;
        }
        break;
    }
    case 8: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 35;
            green = 80;
            blue = 115;
            alpha = 255;
            break;
        case 1:
            red = 20;
            green = 70;
            blue = 130;
            alpha = 255;
            break;
        case 2:
            red = 20;
            green = 70;
            blue = 100;
            alpha = 255;
            break;
        default:
            red = 35;
            green = 80;
            blue = 115;
            alpha = 255;
            break;
        }
        break;
    }
    case 9: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 40;
            green = 80;
            blue = 90;
            alpha = 255;
            break;
        case 1:
            red = 40;
            green = 90;
            blue = 120;
            alpha = 255;
            break;
        case 2:
            red = 40;
            green = 50;
            blue = 90;
            alpha = 255;
            break;
        default:
            red = 40;
            green = 80;
            blue = 90;
            alpha = 255;
            break;
        }
        break;
    }
    case 10: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
            red = 30;
            green = 80;
            blue = 100;
            alpha = 255;
            break;
        case 1:
            red = 40;
            green = 20;
            blue = 126;
            alpha = 255;
            break;
        case 2:
            red = 10;
            green = 170;
            blue = 220;
            alpha = 255;
            break;
        default:
            red = 30;
            green = 80;
            blue = 100;
            alpha = 255;
            break;
        }
        break;
    }
    default: {
        int paletteVariant = AsCSharedDoc(g_sharedDoc)->weatherType;
        switch (paletteVariant) {
        case 0:
        case 1:
        case 2:
            red = 20;
            green = 55;
            blue = 20;
            alpha = 255;
            break;
        default:
            red = 20;
            green = 55;
            blue = 20;
            alpha = 255;
            break;
        }
        break;
    }
    }
}

// Recreate grid line and slope-marker buffers and fill the marker UVs/quad indices.
// Grid owns one reference to each buffer; mapped CPU pointers are borrowed until the matching flush.
void CGrid::CreateBuffers() {
    // Phase 1: replace line vertex/index buffers and the textured marker vertex buffer.
    RefCounted* ref = (RefCounted*)vertexBuffer;
    if (ref) {
        if (--ref->count == 0)
            CubeResourceStorage_dtor(ref, 1);
        vertexBuffer = 0;
    }
    VertexBuffer_Create(0xD20, 0, kGridLineVertexFormat, 0, &vertexBuffer, 0);
    ref = (RefCounted*)indexBuffer;
    if (ref) {
        if (--ref->count == 0)
            CubeResourceStorage_dtor(ref, 1);
        indexBuffer = 0;
    }
    IndexBuffer_Create(0x614, 0, 3, 0, &indexBuffer, 0, 0);
    ref = (RefCounted*)markerVertexBuffer;
    if (ref) {
        if (--ref->count == 0)
            CubeResourceStorage_dtor(ref, 1);
        markerVertexBuffer = 0;
    }
    VertexBuffer_Create(0x4380, 0, kGridMarkerVertexFormat, 0, &markerVertexBuffer, 0);
    // Phase 2: seed four UV corners per marker, then upload quad indices in 0,1,3,2 order.
    void* verts;
    Resource_GetOffsetPtr(markerVertexBuffer, 0, 0, &verts, 0);
    for (int i = 0; i < kGridMarkerCapacity; i++) {
        for (int j = 0; j < 4; j++) {
            reinterpret_cast<GridTexVertex*>(verts)[i * 4 + j].u = static_cast<float>(j & 1);
            reinterpret_cast<GridTexVertex*>(verts)[i * 4 + j].v = static_cast<float>(j >> 1);
        }
    }
    GraphicsBuffer_Flush(markerVertexBuffer);
    ref = (RefCounted*)markerIndexBuffer;
    if (ref) {
        if (--ref->count == 0)
            CubeResourceStorage_dtor(ref, 1);
        markerIndexBuffer = 0;
    }
    IndexBuffer_Create(0x5A0, 0, 3, 0, &markerIndexBuffer, 0, 0);
    unsigned short* idx;
    IndexBuffer_GetOffsetPtr(markerIndexBuffer, 0, 0, &idx, 0);
    int off = 0;
    for (int n = 0; n < kGridMarkerCapacity; n++, off += 8) {
        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(idx) + off + 0) = n << 2;
        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(idx) + off + 2) = (n << 2) + 1;
        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(idx) + off + 4) = (n << 2) + 3;
        *reinterpret_cast<unsigned short*>(reinterpret_cast<char*>(idx) + off + 6) = (n << 2) + 2;
    }
    IndexBuffer_Flush(markerIndexBuffer);
}

#pragma push
#pragma opt_strength_reduction off
// Choose a green/near/far grid footprint, project cells onto terrain and position slope markers between neighbors.
// Footprint map: green or pin distance <128 -> 8x16 at 6.4; dense green variant 3 -> 10x11 at 3.2;
// otherwise -> 10x11 at 12.8, built immediately around origin in world X/Z axes.
// Near footprints are rebuilt by OnProcess around the ball with heading-rotated axes.
// One-point mode adjusts near counts/spacing using the stored deltas.
void CGrid::BuildCells() {
    V3 horizontalPinOffset;
    unsigned char* rowCursor;
    unsigned char* cell;
    unsigned char* nextRowCell;
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
    unsigned char* markerRow;
    const char* modeTag;
    int columnOrRowIndex;
    int markerRowIndex;
    // Phase 1: choose footprint from ball ground type, horizontal pin distance and instance variant.
    unsigned char* golfDoc = (unsigned char*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char* gridBase = reinterpret_cast<unsigned char*>(this);
    void* playerRecord = GolfPlayer_GetEntryByIndex(golfDoc, AsCGolfDoc(golfDoc)->currentPlayer);
    golfDoc = (unsigned char*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char* holeRecord =
        (unsigned char*)AsCGolfDoc(golfDoc)->holeRecords + (AsCGolfDoc(golfDoc)->currentHoleIndex << 5);
    unsigned pinXBits = ((GridHoleRecord*)holeRecord)->pinPosition[0];
    unsigned pinYBits = ((GridHoleRecord*)holeRecord)->pinPosition[1];
    ((GridVectorBits*)&pinPosition)->x = pinXBits;
    ((GridVectorBits*)&pinPosition)->y = pinYBits;
    ((GridVectorBits*)&pinPosition)->z = ((GridHoleRecord*)holeRecord)->pinPosition[2];
    __mi__FRC7WVectorRC7WVector(&horizontalPinOffset, &pinPosition, (V3*)((GridPlayerRecord*)playerRecord)->position);
    horizontalPinOffset.y = kFloat_0_grid;
    if (((CGolfBall*)GolfBall__Fv())->m_groundType == kGroundGreen ||
        Magnitude__7WVectorCFv(&horizontalPinOffset) < kFloat_128_Grid) {
        flags &= ~kGridWideArea;
        if (((CGolfBall*)GolfBall__Fv())->m_groundType != kGroundGreen ||
            ((GridInstanceState*)InstanceTable_Get())->descriptor->modeVariant != kDenseGreenVariant ||
            ((GridInstanceState*)InstanceTable_Get())->markerModeOverride != 0) {
            columns = kNearGridColumns;
            rows = kNearGridRows;
            spacing = GRID_NEAR_SPACING_WORLD;
            extraColumns = -2;
            extraRows = 5;
            extraSpacing = 0.0f;
        } else {
            columns = kDenseGridColumns;
            rows = kDenseGridRows;
            spacing = GRID_DENSE_SPACING_WORLD;
            extraColumns = -4;
            extraRows = 0;
            extraSpacing = GRID_DENSE_SPACING_WORLD;
        }
    } else {
        const float minSlopeOffset = -0.45f;
        const float markerHalf = 0.5f;
        flags |= kGridWideArea;
        columns = kWideGridColumns;
        rows = kWideGridRows;
        spacing = GRID_WIDE_SPACING_WORLD;
        extraColumns = 0;
        extraRows = 0;
        extraSpacing = 0.0f;
        // Phase 2 (wide only): project centered world-X/Z sample points, lifting Y by ball radius.
        unsigned char* cellRow;
        cellRow = (unsigned char*)&cells[0][0];
        rowCursor = cellRow;
        for (columnOrRowIndex = 0; columnOrRowIndex < rows; rowCursor += kGridRowBytes, columnOrRowIndex++) {
            cell = rowCursor;
            for (int column = 0; column < columns; cell += kGridCellBytes, column++) {
                int centeredColumn = column - (columns / 2);
                int centeredRow = columnOrRowIndex - (rows / 2);
                float spacing = this->spacing;
                localCellOffset.y = 0.0f;
                localCellOffset.z = spacing * (float)(centeredRow);
                localCellOffset.x = spacing * (float)(centeredColumn);
                __pl__FRC7WVectorRC7WVector(&cellWorldPosition, (V3*)&origin, &localCellOffset);
                __as__7WVectorFRC7WVector(&cellProjectionCopy, &cellWorldPosition);
                float ballRadius = ((CGolfBall*)GolfBall__Fv())->m_radius;
                cellLift.x = 0.0f;
                cellLift.y = (ballRadius);
                cellLift.z = 0.0f;
                *(GridVectorBits*)&cellProjectionInput = *(GridVectorBits*)&cellProjectionCopy;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&cellGroundPosition, CQuadTree_GetInstance(),
                                                        &cellProjectionInput, 1, 0, 1);
                __pl__FRC7WVectorRC7WVector(&raisedCellPosition, &cellGroundPosition, &cellLift);
                __as__7WVectorFRC7WVector((V3*)cell, &raisedCellPosition);
            }
        }
        // Phase 3: use world +Z rows/+X columns and place each slope marker inside its quad.
        // Fraction = 0.5 + clamp(8*atan(heightDelta/spacing), -0.45, 0.45), in each axis.
        // Thus markers stay within [0.05,0.95] of the interval before ground projection/radius lift.
        initialRowAxis.x = kFloat_0_grid;
        initialRowAxis.y = kFloat_0_grid;
        initialRowAxis.z = kFloat_1_Grid;
        __as__7WVectorFRC7WVector((V3*)&rowAxis, &initialRowAxis);
        initialColumnAxis.x = kFloat_1_Grid;
        initialColumnAxis.y = kFloat_0_grid;
        initialColumnAxis.z = kFloat_0_grid;
        __as__7WVectorFRC7WVector((V3*)&columnAxis, &initialColumnAxis);
        const char* name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        void* ui = ((void* (*)(const void*))InstanceTable_Get)(name);
        char* active = strstr(((GridInstanceState*)ui)->descriptor->modeName, name);
        float cellSpacing = spacing + (active ? extraSpacing : kFloat_0_grid);
        modeTag = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        const float maxSlopeOffset = 0.45f;
        markerRowIndex = 0;
        markerRow = (unsigned char*)&cells[0][0].markerPosition;
        while (markerRowIndex <
               gridCount(gridBase, modeTag, GRID_OFFSET(CGrid, rows), GRID_OFFSET(CGrid, extraRows)) - 1) {
            cell = ((cellRow));
            columnOrRowIndex = 0;
            rowCursor = markerRow;
            nextRowCell = (unsigned char*)&cells[0][0] + ((((markerRowIndex) + 1))) * kGridRowBytes;
            while (columnOrRowIndex <
                   gridCount(gridBase, modeTag, GRID_OFFSET(CGrid, columns), GRID_OFFSET(CGrid, extraColumns)) - 1) {
                const float slopeScale = 8.0f;
                float neighbor = ((CQuadTree*)CQuadTree_GetInstance())->GetGreenPos((((nextRowCell))));
                float angle =
                    slopeScale *
                    (float)atan((((CQuadTree*)CQuadTree_GetInstance())->GetGreenPos(cell) - neighbor) / (cellSpacing));
                if (angle < minSlopeOffset)
                    angle = minSlopeOffset;
                else if (angle > maxSlopeOffset)
                    angle = maxSlopeOffset;
                float rowSlopeFraction = ((markerHalf)) + ((angle));
                neighbor = ((CQuadTree*)CQuadTree_GetInstance())
                               ->GetGreenPos(cellRow + (((columnOrRowIndex) + 1)) * kGridCellBytes);
                angle =
                    slopeScale * (float)atan((((CQuadTree*)CQuadTree_GetInstance())->GetGreenPos(cell) - (neighbor)) /
                                             (cellSpacing));
                if (angle < minSlopeOffset)
                    angle = minSlopeOffset;
                else if (angle > maxSlopeOffset)
                    angle = maxSlopeOffset;
                float columnSlopeFraction = markerHalf + angle;
                float ballRadius = ((CGolfBall*)GolfBall__Fv())->m_radius;
                const float zero = 0.0f;
                markerLift.x = (zero);
                markerLift.y = ballRadius;
                markerLift.z = zero;
                __ml__FfRC7WVector(&columnDisplacement, (V3*)&columnAxis, (columnSlopeFraction));
                __ml__FfRC7WVector(&rowDisplacement, (V3*)&rowAxis, (rowSlopeFraction));
                __pl__FRC7WVectorRC7WVector(&markerOffset, &rowDisplacement, &columnDisplacement);
                __ml__FfRC7WVector(&scaledMarkerOffset, &markerOffset, cellSpacing);
                __pl__FRC7WVectorRC7WVector(&markerWorldPosition, (V3*)cell, &scaledMarkerOffset);
                *(GridVectorBits*)&markerProjectionInput = *(GridVectorBits*)&markerWorldPosition;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&markerGroundPosition, CQuadTree_GetInstance(),
                                                        &markerProjectionInput, 1, 0, 1);
                __pl__FRC7WVectorRC7WVector(&raisedMarkerPosition, &markerGroundPosition, &markerLift);
                __as__7WVectorFRC7WVector((V3*)(rowCursor), &raisedMarkerPosition);
                cell += kGridCellBytes;
                rowCursor += kGridCellBytes;
                nextRowCell += kGridCellBytes;
                columnOrRowIndex = ((((columnOrRowIndex)) + 1));
            }
            cellRow += kGridRowBytes;
            markerRow += kGridRowBytes;
            markerRowIndex = (1 + markerRowIndex);
        }
    }
    // Phase 4: discard stale marks and refresh the palette for the chosen footprint.
    ResetCellsAndColor();
}
#pragma pop

#pragma push
#pragma opt_strength_reduction off
#pragma opt_propagation off
// Cull the grid, rebuild dirty cells and slope markers, update the screen profile and mark the ball cell.
void CGrid::OnProcess() {
    unsigned char* gridBase = reinterpret_cast<unsigned char*>(this);
    CGrid* self = this;
    // Projection scratch: world vectors for cell placement, declared in first-use order.
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
    if (AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->tutorialStage < 4)
        return;
    if ((((GridInstanceState*)InstanceTable_Get())->gridUpdateFlags & 1) != 0)
        self->flags |= kGridGeometryDirty;
    if ((self->flags & kGridHidden) != 0)
        return;
    __mi__FRC7WVectorRC7WVector(&cameraOffsetXZ, (V3*)(s_cameraTransform + 0x24), (V3*)&self->origin);
    cameraOffsetXZ.y = 0.0f;
    if (Magnitude__7WVectorCFv(&cameraOffsetXZ) > kFloat_320_Grid) {
        self->flags |= kGridOutOfRange;
        return;
    }
    unsigned flags = self->flags;
    unsigned cleared = flags & ~kGridOutOfRange;
    self->flags = cleared;
    if ((flags & kGridWideArea) == 0) {
        if ((cleared & kGridGeometryDirty) != 0) {
            // Phase 2 (dirty near grid): rotate row/column axes about world Y; project points from ball.
            Matrix_SetAxisRotation(headingMatrix, self->heading, 1);
            __as__7WVectorFRC7WVector((V3*)&self->rowAxis, &headingMatrix[2]);
            axis.x = 0.0f;
            axis.y = kFloat_1_Grid;
            axis.z = 0.0f;
            Vector_CrossProduct(&up, &axis, (V3*)&self->rowAxis);
            __as__7WVectorFRC7WVector((V3*)&self->columnAxis, &up);
            cellRow = (unsigned char*)&self->cells[0][0];
            name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            markerHalf = kFloat_0_5_Grid;
            zero = 0.0f;
            markerCell = cellRow;
            rowIndex = 0;
            while (rowIndex < gridCount(gridBase, name, GRID_OFFSET(CGrid, rows), GRID_OFFSET(CGrid, extraRows))) {
                rowCursor = markerCell;
                column = 0;
                while (column <
                       gridCount(gridBase, name, GRID_OFFSET(CGrid, columns), GRID_OFFSET(CGrid, extraColumns))) {
                    int extraColumnCount;
                    if (0 != strstr(((GridInstanceState*)InstanceTable_Get())->descriptor->modeName, name))
                        extraColumnCount = self->extraColumns;
                    else
                        extraColumnCount = 0;
                    if (0 != strstr(((GridInstanceState*)InstanceTable_Get())->descriptor->modeName, name))
                        extraSpacing = self->extraSpacing;
                    else
                        extraSpacing = 0.0f;
                    int effectiveColumnCount = self->columns + extraColumnCount;
                    __ml__FfRC7WVector(&cellColumnDisplacement, (V3*)&self->columnAxis,
                                       (markerHalf + (float)column) - (float)(effectiveColumnCount / 2));
                    __ml__FfRC7WVector(&cellRowDisplacement, (V3*)&self->rowAxis, (float)rowIndex);
                    __pl__FRC7WVectorRC7WVector(&cellOffset, &cellRowDisplacement, &cellColumnDisplacement);
                    __ml__FfRC7WVector(&scaledCellOffset, &cellOffset, self->spacing + extraSpacing);
                    __pl__FRC7WVectorRC7WVector(&cellWorldPosition, (V3*)GolfBall__Fv(), &scaledCellOffset);
                    __as__7WVectorFRC7WVector(&cellProjectionCopy, &cellWorldPosition);
                    float ballRadius = ((CGolfBall*)GolfBall__Fv())->m_radius;
                    cellLift.x = zero;
                    cellLift.y = ballRadius;
                    cellLift.z = zero;
                    *(GridVectorBits*)&cellProjectionInput = *(GridVectorBits*)&cellProjectionCopy;
                    GetGroundPoint__9CQuadTreeF7WVectorbPib(&cellGroundPosition, CQuadTree_GetInstance(),
                                                            &cellProjectionInput, 1, 0, 1);
                    __pl__FRC7WVectorRC7WVector(&raisedCellPosition, &cellGroundPosition, &cellLift);
                    __as__7WVectorFRC7WVector((V3*)rowCursor, &raisedCellPosition);
                    rowCursor += kGridCellBytes;
                    column++;
                }
                markerCell += kGridRowBytes;
                rowIndex++;
            }
            // Phase 3: publish pin selection/profile, then rebuild slope markers from neighbor heights.
            UpdateSelectionAndProfile();
            name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            cellSpacing = self->spacing + (strstr(((GridInstanceState*)InstanceTable_Get())->descriptor->modeName, name)
                                               ? self->extraSpacing
                                               : 0.0f);
            minSlopeOffset = kFloat_neg0_45_Grid;
            maxSlopeOffset = kFloat_0_45_Grid;
            slopeScale = kFloat_8_Grid;
            name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            markerCenter = kFloat_0_5_Grid;
            markerRow = (unsigned char*)&self->cells[0][0].markerPosition;
            for (rowIndex = 0; rowIndex < self->rows + (0 == findName(name, gridBase) ? 0 : self->extraRows) - 1;
                 cellRow += kGridRowBytes, markerRow += kGridRowBytes, rowIndex++) {
                cell = cellRow;
                markerCell = markerRow;
                nextRowCell = (unsigned char*)&self->cells[0][0] + (rowIndex + 1) * kGridRowBytes;
                for (columnIndex = 0;
                     columnIndex < self->columns + (0 == findName(name, gridBase) ? 0 : self->extraColumns) - 1;
                     nextRowCell += kGridCellBytes, cell += kGridCellBytes, markerCell += kGridCellBytes,
                    columnIndex++) {
                    neighbor = ((CQuadTree*)CQuadTree_GetInstance())->GetGreenPos(nextRowCell);
                    float angle =
                        slopeScale * (float)atan((((CQuadTree*)CQuadTree_GetInstance())->GetGreenPos(cell) - neighbor) /
                                                 cellSpacing);
                    if (angle < minSlopeOffset)
                        angle = minSlopeOffset;
                    else if (angle > maxSlopeOffset)
                        angle = maxSlopeOffset;
                    rowSlopeFraction = markerCenter + angle;
                    neighbor = ((CQuadTree*)CQuadTree_GetInstance())
                                   ->GetGreenPos(cellRow + (columnIndex + 1) * kGridCellBytes);
                    angle =
                        slopeScale * (float)atan((((CQuadTree*)CQuadTree_GetInstance())->GetGreenPos(cell) - neighbor) /
                                                 cellSpacing);
                    if (angle < minSlopeOffset)
                        angle = minSlopeOffset;
                    else if (angle > maxSlopeOffset)
                        angle = maxSlopeOffset;
                    columnSlopeFraction = markerCenter + angle;
                    float markerRadius = ((CGolfBall*)GolfBall__Fv())->m_radius;
                    markerLift.x = 0.0f;
                    markerLift.y = markerRadius;
                    markerLift.z = 0.0f;
                    __ml__FfRC7WVector(&columnDisplacement, (V3*)&self->columnAxis, columnSlopeFraction);
                    __ml__FfRC7WVector(&rowDisplacement, (V3*)&self->rowAxis, rowSlopeFraction);
                    __pl__FRC7WVectorRC7WVector(&markerOffset, &rowDisplacement, &columnDisplacement);
                    __ml__FfRC7WVector(&scaledMarkerOffset, &markerOffset, cellSpacing);
                    __pl__FRC7WVectorRC7WVector(&markerWorldPosition, (V3*)cell, &scaledMarkerOffset);
                    *(GridVectorBits*)&markerProjectionInput = *(GridVectorBits*)&markerWorldPosition;
                    GetGroundPoint__9CQuadTreeF7WVectorbPib(&markerGroundPosition, CQuadTree_GetInstance(),
                                                            &markerProjectionInput, 1, 0, 1);
                    __pl__FRC7WVectorRC7WVector(&raisedMarkerPosition, &markerGroundPosition, &markerLift);
                    __as__7WVectorFRC7WVector((V3*)markerCell, &raisedMarkerPosition);
                }
            }
            self->flags &= ~kGridShowSelection;
        } else {
            self->flags = cleared | kGridShowSelection;
        }
    }
    // Phase 4: geometry is current; optionally mark the ball's interval in the existing cell array.
    // Existing code checks upper limits only: callers must keep ball-axis indices nonnegative.
    self->flags &= ~kGridGeometryDirty;
    if (((CGolfBall*)GolfBall__Fv())->m_state == 0)
        return;
    if (self->hideEnabled == 0)
        return;
    __mi__FRC7WVectorRC7WVector(&ballOffset, (V3*)GolfBall__Fv(), (V3*)&self->cells[0][0]);
    ballOffset.y = 0.0f;
    {
        float extraSpacingForCell;
        name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        if (0 != findName(name, gridBase))
            extraSpacingForCell = self->extraSpacing;
        else
            extraSpacingForCell = 0.0f;
        Vec3_DivideAssign(&ballOffset, self->spacing + extraSpacingForCell);
    }
    ballRow = (int)__ml__FRC7WVectorRC7WVector(&ballOffset, (V3*)&self->rowAxis);
    ballColumn = (int)__ml__FRC7WVectorRC7WVector(&ballOffset, (V3*)&self->columnAxis);
    {
        int extra;
        name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        char* hit = findName(name, gridBase);
        int cnt = self->rows;
        if (hit)
            extra = self->extraRows;
        else
            extra = 0;
        if (ballRow >= cnt + extra - 1)
            return;
    }
    {
        int extra;
        name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        char* hit = findName(name, gridBase);
        int cnt = self->columns;
        if (hit)
            extra = self->extraColumns;
        else
            extra = 0;
        if (ballColumn >= cnt + extra - 1)
            return;
    }
    unsigned char* block = gridBase + ballRow * kGridRowBytes;
    *(unsigned char*)((unsigned int)block + ballColumn * kGridCellBytes + GRID_OFFSET(CGrid, cells[0][0].marked)) = 1;
}
#pragma pop

// Upload/draw grid lines and slope-marker quads, then overlay marked and selected cells.
void CGrid::Draw() {
    char* rowp;
    int base;
    char* cellCursor;
    int row;
    int column;
    int lineCount;
    char* gridBase = (char*)this;
    if (((CGridUpdate*)(void*)this)->vf12() != 0)
        return;
    CGolfDoc* golfDoc = (CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    if (golfDoc->tutorialStage >= 4) {
        unsigned int flags = this->flags;
        if (!(flags & kGridOutOfRange) && !(flags & kGridHidden)) {
            const char* name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
            void* ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
            char* extra = strstr(((GridInstanceState*)ui)->descriptor->modeName, name);
            int rows = this->rows + (extra ? this->extraRows : 0);
            ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
            extra = strstr(((GridInstanceState*)ui)->descriptor->modeName, name);
            int cols = this->columns + (extra ? this->extraColumns : 0);
            // Phase 1: flatten terrain samples into colored line vertices and horizontal/vertical edges.
            void* lineVertices;
            void* lineIndices;
            void* markerVertices;
            Resource_GetOffsetPtr(vertexBuffer, 0, 0, &lineVertices, 0);
            {
                rowp = gridBase + GRID_OFFSET(CGrid, cells);
                row = 0;
                for (; row < rows; ++row) {
                    cellCursor = rowp;
                    base = row * cols;
                    for (column = 0; column < cols; ++column) {
                        Vector_CopyComponents(&((CGridVertex*)lineVertices)[column + base], cellCursor);
                        ((CGridVertex*)lineVertices)[column + base].a = alpha;
                        ((CGridVertex*)lineVertices)[column + base].r = red;
                        ((CGridVertex*)lineVertices)[column + base].g = green;
                        ((CGridVertex*)lineVertices)[column + base].b = blue;
                        cellCursor += kGridCellBytes;
                    }
                    rowp += kGridRowBytes;
                }
            }
            GraphicsBuffer_Flush(vertexBuffer);
            lineCount = 0;
            IndexBuffer_GetOffsetPtr(indexBuffer, 0, 0, &lineIndices, 0);
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
            IndexBuffer_Flush(indexBuffer);
            SetupRenderState();
            Engine_SetFlexibleVertexFormat(kGridLineVertexFormat);
            Engine_SetVertexStream(0, vertexBuffer, 0, 16);
            Engine_SetIndexBuffer(indexBuffer, 0);
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
            else
                useLargeMarker = 0;
            float scale = useLargeMarker ? kFloat_0_4_GridMarker : kFloat_0_2_Grid;
            Resource_GetOffsetPtr(markerVertexBuffer, 0, 0, &markerVertices, 0);
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
                // rowp counts from the object base; the cell offset is added at each access.
                rowp = gridBase;
                row = 0;
                offset = 0;
                for (; row < rows - 1; ++row) {
                    cellCursor = rowp;
                    for (column = 0; column < cols - 1; ++column) {
                        for (k = 0; k < 4; ++k) {
                            CGridCell* cell = (CGridCell*)(cellCursor + GRID_OFFSET(CGrid, cells));
                            ((MarkerVtx*)((char*)markerVertices + offset))->x =
                                cell->markerPosition.x + ((k & 2) ? scale : -scale);
                            ((MarkerVtx*)((char*)markerVertices + offset))->y = cell->markerPosition.y;
                            ((MarkerVtx*)((char*)markerVertices + offset))->z =
                                cell->markerPosition.z + ((k & 1) ? -scale : scale);
                            if (cell->marked != 0) {
                                ((MarkerVtx*)((char*)markerVertices + offset))->r = 0xff;
                                ((MarkerVtx*)((char*)markerVertices + offset))->a = 0xff;
                                ((MarkerVtx*)((char*)markerVertices + offset))->b = 0;
                                ((MarkerVtx*)((char*)markerVertices + offset))->g = 0;
                            } else {
                                ((MarkerVtx*)((char*)markerVertices + offset))->a = alpha;
                                ((MarkerVtx*)((char*)markerVertices + offset))->r = red;
                                ((MarkerVtx*)((char*)markerVertices + offset))->g = green;
                                ((MarkerVtx*)((char*)markerVertices + offset))->b = blue;
                            }
                            offset += 24;
                            ++markerVertexCount;
                        }
                        cellCursor += kGridCellBytes;
                    }
                    rowp += kGridRowBytes;
                }
            }
            GraphicsBuffer_Flush(markerVertexBuffer);
            SetupRenderState();
            Engine_SetFlexibleVertexFormat(kGridMarkerVertexFormat);
            Engine_SetVertexStream(0, markerVertexBuffer, 0, 24);
            Engine_SetIndexBuffer(markerIndexBuffer, 0);
            {
                int v = ((CDev*)((WView*)g_view)->renderTarget)->vf26((*(int*)&bitmapId));
                Engine_SetTextureStageResource(0, v);
                v = ((CDev*)((WView*)g_view)->renderTarget)->vf27((*(int*)&bitmapId));
                Engine_SetTexturePalette(0, v);
            }
            Engine_DrawIndexedPrimitive(kQuadPrimitive, 0, 0, markerVertexCount, 0, markerVertexCount / 4);
            // Phase 3: draw translucent white marked-cell overlays, then green pin selection.
            SetupRenderState();
            Engine_SetFlexibleVertexFormat(kGridLineVertexFormat);
            Engine_SetTextureStageResource(0, 0);
            {
                char* rowp;
                char* cellCursor;
                CGridVertex* qv;
                unsigned char* color;
                int row;
                int column;
                int k;
                // The overlay loop uses the same base-relative cell walk as the marker loop.
                rowp = gridBase;
                for (row = 0; row < rows - 1; ++row) {
                    cellCursor = rowp;
                    for (column = 0; column < cols - 1; ++column) {
                        if (((CGridCell*)(cellCursor + GRID_OFFSET(CGrid, cells)))->marked != 0) {
                            qv = overlayQuad;
                            color = (unsigned char*)gridBase;
                            for (k = 0; k < 4; ++k) {
                                Vector_CopyComponents(qv, &cells[row + (k & 1)][column + (k >> 1)]);
                                color[kOverlayAlpha] = 0x80;
                                color[kOverlayBlue] = 0xff;
                                color[kOverlayGreen] = 0xff;
                                color[kOverlayRed] = 0xff;
                                color += 16;
                                ++qv;
                            }
                            DrawPrimitiveUP(kTriangleStripPrimitive, 2, overlayQuad, 16);
                        }
                        cellCursor += kGridCellBytes;
                    }
                    rowp += kGridRowBytes;
                }
            }
            flags = this->flags;
            if (!(flags & kGridWideArea) && (flags & kGridShowSelection) && selectedRow >= 0 && selectedColumn >= 0) {
                CGridVertex* qv = overlayQuad;
                unsigned char* color = (unsigned char*)gridBase;
                for (int k = 0; k < 4; ++k) {
                    int row = selectedRow + (k & 1);
                    int column = selectedColumn + (k >> 1);
                    Vector_CopyComponents(&qv[k], &cells[row][column]);
                    color[kOverlayAlpha] = 0x50;
                    color[kOverlayRed] = 0x00;
                    color[kOverlayGreen] = 0xff;
                    color[kOverlayBlue] = 0x00;
                    color += 16;
                }
                DrawPrimitiveUP(kTriangleStripPrimitive, 2, overlayQuad, 16);
            }
        }
    }
}

#pragma opt_propagation off
// Apply grid transform/visibility commands or return profile dimensions through message outputs.
void CGrid::HandleMsg(const GridCommand* command) {
    switch (command->code) {
    case kGridSetHeading: {
        heading = *(float*)command->payload;
        BuildCells();
        flags |= kGridGeometryDirty;
        break;
    }
    case kGridRotate:
        heading += *(float*)command->payload;
        flags |= kGridGeometryDirty;
        break;
    case kGridSetOrigin: {
        void* p = command->payload;
        if (p)
            origin = *(WVector*)p;
        BuildCells();
        break;
    }
    case kGridQuerySize: {
        const char* name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        void* ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
        void* base = ((GridInstanceState*)ui)->descriptor;
        char* active = strstr(((GridModeDescriptor*)base)->modeName, name);
        float value = active ? extraSpacing : kFloat_0_grid;
        float sum = spacing + value;
        *(float*)command->payload = sum;
        const char* name2 = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
        ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
        base = ((GridInstanceState*)ui)->descriptor;
        int add;
        int cnt;
        char* active2 = strstr(((GridModeDescriptor*)base)->modeName, name2);
        cnt = rows;
        add = active2 ? extraRows : 0;
        *(int*)command->countOutput = cnt + add;
        break;
    }
    case kGridShow:
        flags &= ~kGridHidden;
        break;
    case kGridHide:
        if (hideEnabled == 0)
            flags |= kGridHidden;
        break;
    case kGridSetHideOverride: {
        unsigned int value = command->value;
        hideEnabled = value != 0;
        break;
    }
    case kGridSetProfileScale: {
        int value = command->signedValue;
        // Command values use the game's 3.2 world-unit scale for the profile display.
        profileScale = kFloat_3_2_SlopeGrid * (float)value;
        break;
    }
    case kGridRefreshProfile:
        UpdateSelectionAndProfile();
        break;
    }
}
#pragma opt_propagation reset

// Project the pin into grid coordinates and send the screen actor an optional averaged height profile.
// Row/column axes are world-space unit directions. Dot products divided by spacing yield grid
// coordinates; column uses floor(x-0.5)+columnCount/2, row uses truncation. -1 marks upper overflow.
// Screen message 0xD receives distanceSteps, intervalCount, and optional heights[rows]. The height
// buffer is used only during dispatch and freed immediately afterward.
void CGrid::UpdateSelectionAndProfile() {
    // Phase 1: flatten pin-minus-ball to X/Z, then resolve grid coordinates and selection bounds.
    int centerColumn;
    const char* name;
    int profileRowCount;
    int distanceSteps;
    char* gridBase = (char*)this;
    WVector pinOffset;
    void* golfDoc = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
    unsigned char holeIndex = AsCGolfDoc(golfDoc)->currentHoleIndex;
    char* holeRecord = (char*)AsCGolfDoc(golfDoc)->holeRecords + holeIndex * 0x20;
    GridVectorBits vec = *(GridVectorBits*)((GridHoleRecord*)holeRecord)->pinPosition;
    void* ball = GolfBall__Fv();
    __mi__FRC7WVectorRC7WVector(&pinOffset, &vec, ball);
    pinOffset.y = kFloat_0_grid;
    name = (slopeGridStrings + GRID_OFFSET(GridStringPool, onePointMode));
    void* ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    void* base = ((GridInstanceState*)ui)->descriptor;
    char* active = strstr(((GridModeDescriptor*)base)->modeName, name);
    float cellSpacing = spacing + (active ? extraSpacing : kFloat_0_grid);
    selectedRow = (int)((pinOffset * rowAxis) / cellSpacing);
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    int extraColumns = strstr(((GridModeDescriptor*)base)->modeName, name) ? this->extraColumns : 0;
    float columnCoordinate = floor((pinOffset * columnAxis) / cellSpacing - kFloat_0_5_Grid);
    int columnIndex = (int)columnCoordinate;
    int columnCount = columns + extraColumns;
    selectedColumn = columnIndex + columnCount / 2;
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    active = strstr(((GridModeDescriptor*)base)->modeName, name);
    int limit = rows + (active ? extraRows : 0);
    if (selectedRow > limit - 2)
        selectedRow = -1;
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    active = strstr(((GridModeDescriptor*)base)->modeName, name);
    limit = columns + (active ? this->extraColumns : 0);
    if (selectedColumn > limit - 2)
        selectedColumn = -1;
    selectionUpdated = 1;
    // Phase 2: choose profile sample distance (6.4 for scale mode>1/one-point, otherwise 3.2).
    float scale;
    ui = InstanceTable_Get();
    if (((GridInstanceState*)ui)->markerScaleMode > 1)
        scale = kFloat_6_4_SlopeGrid;
    else {
        ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
        base = ((GridInstanceState*)ui)->descriptor;
        scale = strstr(((GridModeDescriptor*)base)->modeName, name) ? kFloat_6_4_SlopeGrid : kFloat_3_2_SlopeGrid;
    }
    distanceSteps = (int)(pinOffset.Magnitude() / scale);
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    active = strstr(((GridModeDescriptor*)base)->modeName, name);
    profileRowCount = rows + (active ? extraRows : 0);
    ui = ((void* (*)(const void*))InstanceTable_Get)(slopeGridStrings);
    base = ((GridInstanceState*)ui)->descriptor;
    active = strstr(((GridModeDescriptor*)base)->modeName, name);
    int effectiveColumnCount = columns + (active ? this->extraColumns : 0);
    centerColumn = (int)(kFloat_0_5_Grid * (float)effectiveColumnCount);
    ball = GolfBall__Fv();
    if (!(((CGolfBall*)ball)->m_groundType == kGroundGreen &&
          (((GridInstanceState*)InstanceTable_Get())->descriptor->modeVariant == kDenseGreenVariant &&
           centerColumn >= 1))) {
        IActor_DispatchMessage(gridBase, gridBase, (slopeGridStrings + GRID_OFFSET(GridStringPool, screenActor)), 0xd,
                               distanceSteps, profileRowCount - 1, 0, 0);
        return;
    }
    // Phase 3 (dense green only): average the two center columns. First row uses two heights;
    // subsequent rows average four neighboring heights, then send/free the temporary profile.
    float* profileHeights = (float*)__nwa__FUl(profileRowCount * 4);
    profileHeights[0] = kFloat_0_5_Grid * (cells[0][centerColumn - 1].position.y + cells[0][centerColumn].position.y);
    for (int i = 1; i < profileRowCount; ++i) {
        profileHeights[i] =
            0.25f * (cells[i][centerColumn].position.y +
                     (cells[i][centerColumn - 1].position.y +
                      (cells[i - 1][centerColumn - 1].position.y + cells[i - 1][centerColumn].position.y)));
    }
    IActor_DispatchMessage(gridBase, gridBase, (slopeGridStrings + GRID_OFFSET(GridStringPool, screenActor)), 0xd,
                           distanceSteps, profileRowCount - 1, profileHeights, 0);
    __dla__FPv(profileHeights);
}

#pragma use_lmw_stmw on
// Apply the grid render/texture tables and camera transform, then reset texture/vertex bindings.
void CGrid::SetupRenderState() {
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
void* CGrid::GetRTTI() {
    return &s_gridTypeInfo;
}

// Release grid resources and all four reference-counted buffers.
CGrid::~CGrid() {
    vtbl = __vt__5CGrid;
    renderVtable = __vt__5CGrid + 0x3c;
    ((CGridUpdate*)this)->releaseResources();
    void* buffer = vertexBuffer;
    if (buffer != 0) {
        RefCounted* ref = (RefCounted*)buffer;
        --ref->count;
        if (ref->count == 0)
            CubeResourceStorage_dtor(buffer, 1);
        vertexBuffer = 0;
    }
    buffer = indexBuffer;
    if (buffer != 0) {
        RefCounted* ref = (RefCounted*)buffer;
        --ref->count;
        if (ref->count == 0)
            CubeResourceStorage_dtor(buffer, 1);
        indexBuffer = 0;
    }
    buffer = markerVertexBuffer;
    if (buffer != 0) {
        RefCounted* ref = (RefCounted*)buffer;
        --ref->count;
        if (ref->count == 0)
            CubeResourceStorage_dtor(buffer, 1);
        markerVertexBuffer = 0;
    }
    buffer = markerIndexBuffer;
    if (buffer != 0) {
        RefCounted* ref = (RefCounted*)buffer;
        --ref->count;
        if (ref->count == 0)
            CubeResourceStorage_dtor(buffer, 1);
        markerIndexBuffer = 0;
    }
}

// Initialize the grid's full-turn constant.
void CGrid::InitializeFullTurnConstant() {
    s_gridFullTurnConstantStorage = kFloat_6_2831855;
}

// Adjust the render interface pointer and forward to grid drawing.
void CGrid::DrawThunk() {
    ((CGrid*)((char*)this - 0x18))->Draw();
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462134", _ctors_u801718A4_0, CGolfTask_RegisterType);
STATIC_INIT_ENTRY(".CRT$XCU80462134", _ctors_u801718A4_1, CGrid::InitializeFullTurnConstant);
#pragma pop

// The private view offsets below are checked at compile time.
typedef char u801718A4_layout_check_0[(GRID_OFFSET(GridStringPool, onePointMode) == 0x14) ? 1 : -1];
typedef char u801718A4_layout_check_1[(GRID_OFFSET(GridStringPool, screenActor) == 0x18) ? 1 : -1];
typedef char u801718A4_layout_check_2[(GRID_OFFSET(GridTaskStringPool, taskClass) == 0x38F) ? 1 : -1];
typedef char u801718A4_layout_check_4[(GRID_OFFSET(GridInstanceState, gridUpdateFlags) == 0x4C) ? 1 : -1];
