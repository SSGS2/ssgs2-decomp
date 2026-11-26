// Render views (WView) and the camera: view and projection matrices, the viewport, frustum
// planes, screen-to-ray and projection, and submitting primitives to the renderer.
// Also the asynchronous DVD file reader (CFileAsync) and the table locks for shared entries.

#include <decomp/static_init.h>
#include <decomp/record_sizes.h>
#include <decomp/local_class.h>
#include <decomp/offset_of.h>
// WView camera/projection, world-to-viewport drawing, and bundled async-file/
// table-lock utilities. Shared schemas: WView.h, WViewProjectedPoint.h;
// scalar storage: the data section. Render targets and targetSlot are borrowed:
// targetSlot points to a live external pointer slot, renderTarget caches *slot.
// The view never releases the target; its slot must outlive all queries/draws.
// Modding lookup (widescreen / aspect ratio / viewport / scissor):
// - Camera_SetFov: horizontal radians; default pi/2 (90 degrees) in WView_ctor.
// - Camera_BuildFrustumPlanes: target pixel width/height set slopes, half-viewport
//   centers, and kFloat_4_CameraBounds=4-pixel inset. Increasing the inset
//   narrows the inner frustum; changing projection requires consistent culling.
// - Camera_Apply/RebuildCameraPerspective: vertical FOV is derived from h/w and
//   horizontal FOV, but MtxPerspectiveFovRH gets a SEPARATE fixed aspect scalar
//   kViewPerspectiveAspectRatio=1.25 at 0x805DD870. Do not substitute w/h silently. A larger
//   aspect widens that perspective at fixed vertical FOV; safe range unestablished.
// - WView_SetViewportRect: optional {left,top,width,height} in target pixels;
//   Camera_SetTarget refreshes full dimensions from target slots 15/16. This
//   unit sets no GX scissor or physical display resolution. Keep target, camera,
//   renderer viewport and UI/hit-test transforms consistent in a later mod.
// - Camera_SetClip: near=3/far=100000 scene units at construction. Keep clip
//   distances, depth scale/offset and projection invalidation together. Values stay in
//   scene units, with no golf-yard conversion; valid finite near/far must differ.
// - Camera_Apply offsets: pixel displacements divided by screenCenterX/Y;
//   projection indices 8/9 receive +dx/centerX and -dy/centerY plus frustum bias.
//   Offsets do not move the viewport or change its dimensions.
// Geometry contract: affine frames are 12 floats (three XYZ axes, XYZ position).
// BuildFrustumPlanes negates right/forward before inversion. Projection divides
// homogeneous coordinates to x/y in [-1,1], z in [0,1]; viewport pixels increase
// right/down. Preserve matrix operand order and all float operations/conversions.
// Frustum storage is twelve {normal.xyz,distance} records: full then inset,
// ordered near/far/left/right/top/bottom. Byte-address expressions below address
// this same float schema; they are not a second layout or serialized pointers.
// No geometry/target pointers transfer ownership. Draw scratch and shared box
// slots are borrowed for immediate submission; callers must serialize use.
// Default virtual hooks forward to the WGeneric3DR renderer: see the concrete
// receiver/slot/payload map beside VirtualObject, including tail-forwarded ABI
// arguments. File/lock contracts
// below are separate from rendering and must retain handle/error/cleanup order.
// Dirty bits are consumed by Camera_SetTarget and Camera_Apply.
enum ViewDirtyFlags {
    kViewTargetDirty = 1u,
    kViewMatrixDirty = 0x10000000u,
    kViewProjectionDirty = 0x20000000u,
    kViewAndTargetDirty = kViewMatrixDirty | kViewTargetDirty,
    kViewAndProjectionDirty = kViewMatrixDirty | kViewProjectionDirty
};
enum ViewPrimitiveFlags {
    kPrimitiveKindMask = 0x06000000u,
    kLinePrimitive = 0x04000000u,
    kOptionalSubmitFlag = 0x08000000u,
    kSceneOverrideClearedFlag = 0x00400000u,
    kPackedDrawArgumentMask = 0x7FFu,
    kPackedDrawFlagsMask = 0xFFFFF800u,
    kDisableCachedRenderState = 0x10u,
    kSkipCommonClipTest = 0x80u,
    kTranslucentBoxFlags = 0x15000000u
};
enum ViewClipFlags {
    kClipBeforeNear = 1,
    kClipBeyondFar = 2,
    kClipLeft = 4,
    kClipRight = 8,
    kClipAbove = 0x10,
    kClipBelow = 0x20,
    kAllClipBits = 0xFF,
    kSceneOverrideClipFlags = 0xFE
};
enum {
    kPreparedVertexType = 0x20,
    kPositionColorVertexType = 0x18,
    kProjectedVertexType = 0x10,
    kMaximumCircleSegments = 24,
    kBoxCornerCount = 8,
    kBoxFaceCount = 6,
    kBoxFaceCornerCount = 4,
    kProjectedVertexBytes = 0x38,
    kFullAlpha = 255,
    kHighlightGreen = 0xFF00FF00u,
    kDefaultViewTintRgb = 0xFFFFFF
};
enum ViewProjectionContract {
    kProjectionHorizontalScale = 0,
    kProjectionCenterX = 8,
    kProjectionCenterY = 9,
    kAppliedViewChanged = 1,
    kAppliedProjectionChanged = 2,
    kViewFloatSignBit = 0x80000000u
};
// Box topology and its four reusable projected vertices occupy one shared data block.
struct BoxDrawTables {
    unsigned char header[0x18];
    int cornerSelectors[8][3];
    unsigned int faceIndices[6][4];
    void* projectedVertexList[5];
};
struct BoxFaceVertex {
    float screenPosition[4];
    unsigned int color;
    char projectedFields[0x24];
};
typedef char BoxTables_faces_offset[((int)DECOMP_OFFSETOF(BoxDrawTables, faceIndices) == 0x78) ? 1 : -1];
typedef char BoxTables_vertices_offset[((int)DECOMP_OFFSETOF(BoxDrawTables, projectedVertexList) == 0xD8) ? 1 : -1];
typedef char BoxVertex_size[(sizeof(BoxFaceVertex) == 0x38) ? 1 : -1];
// Bounds are minimum XYZ followed by maximum XYZ in the input-transform space.
struct BoxInputBounds {
    float minimum[3];
    float maximum[3];
};
struct CameraAffineFrame {
    float axes[3][3];
    float position[3];
};
struct RenderTargetDepthState {
    char targetHeader[8];
    float depthScale;
    float depthOffset;
};
struct SceneViewOverride {
    char sceneState[0x161];
    unsigned char enabled;
};

#include "classes/WView.h"
#include "classes/WViewProjectedPoint.h"
#define AsWView(p) ((WView*)(p))

// Keep byte-address additions here: member-address CSE changes this function's register allocation.
enum ViewStorageOffset {
    kInputTransformOffset = (int)DECOMP_OFFSETOF(WView, inputTransform[0]),
    kViewMatrixOffset = (int)DECOMP_OFFSETOF(WView, viewMatrix[0]),
    kForwardAxisOffset = (int)DECOMP_OFFSETOF(WView, viewMatrix[6]),
    kCameraPositionOffset = (int)DECOMP_OFFSETOF(WView, viewMatrix[9]),
    kAppliedViewMatrixOffset = (int)DECOMP_OFFSETOF(WView, appliedViewMatrix[0]),
    kPendingViewMatrixOffset = (int)DECOMP_OFFSETOF(WView, pendingViewMatrix[0]),
    kNearPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[0]),
    kFarPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[4]),
    kLeftPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[8]),
    kRightPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[12]),
    kTopPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[16]),
    kBottomPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[20]),
    kInsetNearPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[24]),
    kInsetFarPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[28]),
    kInsetLeftPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[32]),
    kInsetRightPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[36]),
    kInsetTopPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[40]),
    kInsetBottomPlaneOffset = (int)DECOMP_OFFSETOF(WView, frustumPlanes[44]),
};

#define GAME_NSTRING_RANGE_CONSTRUCTOR
#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u80065F78 {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    // Initialize an empty, non-owning inline string.
    LocalNString_u80065F78() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    // Construct owned text through the external string constructor.
    LocalNString_u80065F78(const char* text) {
        MWString_ConstructCString(this, text);
    }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    // Copy NUL-terminated text into inline or reserved heap storage.
    LocalNString_u80065F78(const char* text) {
        LocalNString_u80065F78* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    // Release only the heap-owned representation.
    ~LocalNString_u80065F78() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Return character count from the active string representation.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Borrow NUL-terminated inline or heap text until the next mutation.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

#include "game/types.h"
#include "sdk_protos.h"
#include "classes/CFileAsync.h"

// ---- merged from wview_camera_projection.cpp ----

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_ViewConstruction;
extern const float kFloat_0_0_CameraView;
extern const float kHalfPi_DefaultFieldOfView;
// Default near/far distances, in the input scene's units, not normalized depth.
extern const float kFloat_3_ViewNearClip;
extern const float kFloat_100000_CameraView;
// Depth mapping coefficient -near/(far-near); unrelated to aspect ratio.
extern const float kFloat_neg0_0000300009_WViewDepthProjection;
// +/-0.001 in submitted XY coordinate units: near-coincident line fallback.
extern const float kFloat_0_001_ViewPrimitive;
extern const float kFloat_neg0_001_WViewPointTolerance;
extern const float kTwoPi_FacingCircle;
extern const float kFloat_0_00001_CameraView;
extern float s_cameraVerticalAlignmentReference;
extern const float kFloat_0_5_View;
// Four TARGET PIXELS trimmed from each half-extent for the inset frustum.
extern const float kFloat_4_CameraBounds;
extern const float kFloat_neg1_View;
extern const float kFloat_1_view;
extern float kFloat_neg0_5_View;

int Vec3_NearlyEqualWithTolerance(void*, float*, float);
void Matrix_CopyAffine(void*, void*);
void Matrix_SetIdentityAffine(void*);
void BaseObject_ctor(void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void* Normalize__7WVectorFv(void*);
void Vector_CrossProduct(void*, void*, void*);
void Matrix_TransformDirection(void*, void*, void*);
void Matrix_InvertAffine(void*, void*);
void __amu__7WVectorFf(void*, float);
void __ml__FRC7WVectorf(void*, void*, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void* Plane_FromNormalAndPoint(void*, void*, void*);
void* Plane_Copy(void*, void*);
int __fpclassifyf(float);
void __as__7WVectorFRC7WVector(void*, void*);
void* memset(void*, int, unsigned int);

struct Vec4f {
    float x, y, z, w;
};
extern unsigned char s_boxDrawTopologyAndVertexPointers[];
extern unsigned char s_boxFaceProjectedVertex0[];
extern unsigned char __vt__5WView[];
void* Matrix_TransformPoint(void*, void*, void*);
void WView_ProjectViewPoint(void*, void*, void*, int);

// Initialize camera matrices, viewport, clip distances, projection bounds, and render state.
// self is required writable WView storage; returns self and allocates no target.
// Defaults include a 1x1 placeholder viewport until Camera_SetTarget refreshes
// it, symmetric +/-0.5 projection bounds and zero dirty flags. Existing field
// reads/conditional writes precede that final flag reset; preserve their order.
void* WView_ctor(void* self) {
    // Phases: initialize target/render state; set clip/depth coefficients; initialize
    // affine frames and viewport; clear and seed the two 4x4 matrices.
    BaseObject_ctor(self);
    float zero = kFloat_0_0_CameraView;
    float f1 = AsWView(self)->fieldOfView;
    float f0 = kHalfPi_DefaultFieldOfView;
    (*(int*)&AsWView(self)->targetSlot) = 0;
    *(void**)self = __vt__5WView;
    (*(int*)&AsWView(self)->renderTarget) = 0;
    AsWView(self)->renderState = 0;
    AsWView(self)->unk_1A28 = 0;
    AsWView(self)->unk_1A2C = 0;
    AsWView(self)->tintRgb = kDefaultViewTintRgb;
    AsWView(self)->unk_1A24 = zero;
    AsWView(self)->unk_1A20 = zero;
    AsWView(self)->unk_1A1C = zero;
    if (f1 != f0) {
        AsWView(self)->fieldOfView = f0;
        AsWView(self)->dirtyFlags |= kViewProjectionDirty;
    }
    if (AsWView(self)->nearClip != kFloat_3_ViewNearClip || AsWView(self)->farClip != kFloat_100000_CameraView)
        AsWView(self)->dirtyFlags |= kViewProjectionDirty;
    float nearP = kFloat_3_ViewNearClip;
    float farP = kFloat_100000_CameraView;
    float depthScale = kFloat_neg0_0000300009_WViewDepthProjection;
    float product = depthScale * nearP;
    AsWView(self)->nearClip = nearP;
    AsWView(self)->farClip = farP;
    double negProduct = -product;
    AsWView(self)->depthProjectionScale = depthScale;
    AsWView(self)->depthProjectionOffset = negProduct;
    float divisor = negProduct;
    AsWView(self)->unk_194 = 1;
    AsWView(self)->unk_190 = 0;
    AsWView(self)->customViewport = 0;
    (*(int*)&AsWView(self)->dirtyFlags) = 0;
    AsWView(self)->nearDepthProjectionRatio = nearP / divisor;
    AsWView(self)->farDepthProjectionRatio = farP / divisor;
    Matrix_SetIdentityAffine((&AsWView(self)->viewMatrix[0]));
    Matrix_SetIdentityAffine((&AsWView(self)->pendingViewMatrix[0]));
    unsigned char flag = AsWView(self)->customViewport;
    float one = kFloat_1_view;
    AsWView(self)->viewportWidth = one;
    AsWView(self)->viewportHeight = one;
    if (!flag) {
        volatile float tmp[4];
        float zero2 = kFloat_0_0_CameraView;
        tmp[2] = one;
        tmp[0] = zero2;
        tmp[1] = zero2;
        tmp[3] = one;
        AsWView(self)->viewportLeft = zero2;
        AsWView(self)->viewportTop = zero2;
        AsWView(self)->viewportExtentX = one;
        AsWView(self)->viewportExtentY = one;
    }
    float half = kFloat_0_5_View;
    float halfNeg = kFloat_neg0_5_View;
    float one2 = kFloat_1_view;
    AsWView(self)->unk_1A0C = 1;
    AsWView(self)->primitiveSubmissionGate = one2;
    AsWView(self)->projectionLeft = halfNeg;
    AsWView(self)->projectionRight = half;
    AsWView(self)->projectionMinY = halfNeg;
    AsWView(self)->projectionMaxY = half;
    AsWView(self)->suppressOptionalSubmitFlag = 0;
    AsWView(self)->matrixUpdateFlags = 0;
    memset((&AsWView(self)->savedViewMatrix[0]), 0, 0x40);
    memset((&AsWView(self)->projectionMatrix[0]), 0, 0x40);
    float zero2 = kFloat_0_0_CameraView;
    float oneT = kFloat_1_view;
    float neg = kFloat_neg1_View;
    AsWView(self)->savedViewMatrix[15] = oneT;
    AsWView(self)->projectionMatrix[11] = neg;
    AsWView(self)->projectionOffsetY = zero2;
    AsWView(self)->projectionOffsetX = zero2;
    return self;
}

extern void* g_sceneManager;
}

struct CamSub {
    virtual void v00();
    virtual void v01();
    virtual void v02();
    virtual void v03();
    virtual void v04();
    virtual void v05();
    virtual void v06();
    virtual void v07();
    virtual void v08();
    virtual void v09();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual void v16();
    virtual void v17();
    virtual void v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual void v24();
    virtual void v25();
    virtual void v26();
    virtual void v27();
    virtual void v28();
    virtual int IsActive();
};

// Borrowed vertex pointer list. The type selector determines the pointee
// record: raw XYZ or a WViewProjectedPoint, not an array of float pointers
// within a vertex. Prepare may substitute its own list or return null.
typedef float* ViewVertexData;
typedef ViewVertexData* ViewVertexList;

struct CamBase {
    virtual void v00();
    virtual void v01();
    virtual void v02();
    virtual void v03();
    virtual void v04();
    virtual void v05();
    virtual ViewVertexList Prepare(ViewVertexList list, int type, int n);
    virtual void v07();
    virtual void v08();
    virtual void Draw(ViewVertexList list, unsigned int flags, int n, void* arg);
};

// Read the constructor-seeded submission inhibitor. WView_ctor writes 1.0f;
// source search of WView fields/raw +0x19F8 finds no nondefault producer. The
// sole consumer below rejects >1; all initialized source-created views pass.
// No distance/time unit or tunable physical relationship is established. This
// is a dormant guard in implemented source, not an opacity/speed/clip setting.
// A mod introducing a writer must define its own domain and test the >1 boundary;
// existing values <=1 (including negative) pass, and unordered NaN also passes.
static inline float PrimitiveSubmissionGateValue(void* view) {
    return AsWView(view)->primitiveSubmissionGate;
}
// Borrow the target through its draw-activity interface without reinterpreting the pointer slot.
static inline CamSub* CurrentPrimitiveRenderTarget(void* view) {
    return (CamSub*)AsWView(view)->renderTarget;
}

// Submit prepared vertices, clearing the optional draw flag when the view requests it.
static inline void Submit(CamBase* s, ViewVertexList l, unsigned int flags, int n, void* arg) {
    // The view's suppression byte removes only the optional submission flag.
    s->Draw(l, AsWView(s)->suppressOptionalSubmitFlag ? (flags &= ~kOptionalSubmitFlag) : flags, n, arg);
}

extern "C" {
// Prepare and submit a line/polygon, applying scene overrides and degenerate-line handling.
// Concrete submission ABI (vtable offsets include two CodeWarrior header words):
//   receiver self=WView, +0x18 -> this wrapper;
//   self+0x20 -> WView_PrepareVertexList(self,list,type[,minimumCount unused]);
//   self+0x2C -> WView_ForwardPreparedPrimitive -> renderTarget+0x18,
//     WGeneric3D_DrawPolygonFan(target,list,flags,count,executeArgument).
// The last wrapper tail-forwards the live argument registers; its old one-arg
// declaration hid that protocol. Concrete renderer table: __vt__11WGeneric3DR,
// phase1_class_tables; source producer CreateWView (resource_manager_cache_and_loading.cpp) stores manager
// address as targetSlot, and CProjectG_CreateWindowProvider (project_g_app_and_devices.cpp) writes
// that manager's first pointer to the renderer provider. SetTarget caches it.
//
// Ordinary vertex record (GenericVertexSource, wgeneric3d_render_queue_and_soft_keyboard.cpp), 0x38 bytes/align4:
// +00 x,+04 y,+08 z,+0C w; +10 packed color; +14 reserved;
// +18/+1C uv0; +20/+24 uv1; +28/+2C/+30 divided clip xyz;
// +34 clip byte,3 padding. Renderer packing copies only selected fields.
// WViewProjectedPoint names the projection subset of this SAME storage.
// Type 0x18 callers such as DrawLine initialize world xyz/color, type 0x10
// DrawBox projects into this record first; 0x20 means caller-prepared storage.
// Prepare never transforms: it ANDs +34 clip bytes and sets cached state 0.
// A caller needing projection must do it before submitting this list.
// list: writable null-terminated array of pointers to those records.
// type: vertex format; kPreparedVertexType (0x20) means the list is already prepared and is used as is,
//       any other type goes through self->Prepare() which may return a replacement list (null = skip).
// flags: bits 0x06000000 select the primitive kind (0x04000000 = line; others are polygons);
//        only the immediate scene-override/tagged branch splits low 11 bits
//        (texture slot 0..2047) from upper flags. Ordinary Submit keeps flags
//        and executeArgument separately: executeArgument is a scalar execute argument in pointer ABI.
//        WGeneric3D flags (wgeneric3d_render_queue_and_soft_keyboard.cpp): 0x7FF texture0,0x3F800 texture1,
//        0x300000 pass,0x400000 queue/copy,0x06000000 topology,
//        0x08000000 fog (the optional bit suppressed here),0x40000000 omit color.
// Primitive table:
//   kind   | list shape                         | vertices submitted
//   line   | list[0], list[1], optional list[2] | 2; if list[0] and list[1] coincide in XY (within
//          |                                    | +-0.001) list[2] replaces list[1]
//   other  | list[0..n-1], then a null entry    | n = number of non-null pointers, at least 3
// Scene override: when the render target is inactive and the scene manager's override is enabled,
// a non-line (kind bits set) primitive just loses kSceneOverrideClearedFlag, while a kind-0 primitive
// is drawn immediately with clip flags 0xFE and no further processing.
// That count=-1 call uses TaggedPrimitiveRecord[0] rather than ordinary XYZ:
// +00..0C uv rectangle,+10 color,+14 rotation1024,+18..24 signed extents,
// +28..30 center xyz,+34 tag. Tags 0..7 choose facing/axis planes; FE supplies
// explicit corners and FF an extended transform (full schemas in wgeneric3d_render_queue_and_soft_keyboard.cpp).
// Supply a complete tagged record for this branch; a clip-byte edit alone does
// not convert ordinary world vertices into the tagged payload representation.
// Mutates a coincident line's pointer list and scene-override first clip flag;
// the caller must provide writable storage. No vertex/list ownership transfers.
// Preserve Prepare/Draw virtual order, early rejection and per-submission target
// reloads. Renderer staging copies borrowed records before queuing, so local
// lists need live only through this call. Closed strips require count<=63
// (renderer appends list[0] into a 64-pointer scratch); the wrapper does not check.
void WView_DrawPrimitive(CamBase* self, ViewVertexList list, int type, unsigned int flags, void* executeArgument) {
    // Constructor-only producer and comparison domain: see the gate adapter above.
    if (PrimitiveSubmissionGateValue(self) > kFloat_1_view)
        return;
    CamSub* obj = CurrentPrimitiveRenderTarget(self);
    if (!(obj ? obj->IsActive() : 0) && g_sceneManager && ((SceneViewOverride*)g_sceneManager)->enabled) {
        if (flags & kPrimitiveKindMask) {
            flags &= ~kSceneOverrideClearedFlag;
        } else {
            ((WViewProjectedPoint*)*list)->clipFlags = kSceneOverrideClipFlags;
            self->Draw(list, flags & kPackedDrawFlagsMask, -1, (void*)(flags & kPackedDrawArgumentMask));
            return;
        }
    }
    // Lines use two endpoints and may read optional list[2] as a fallback
    // for coincident XY. Polygons below require >=3 pointers plus a null
    // terminator. Prepare owns any replacement; Draw only borrows it.
    if ((flags & kPrimitiveKindMask) == kLinePrimitive) {
        if (type != kPreparedVertexType) {
            list = self->Prepare(list, type, 2);
            if (!list)
                return;
        }
        if (list[2]) {
            float dx = list[0][0] - list[1][0];
            float dy = list[0][1] - list[1][1];
            if (dx <= kFloat_0_001_ViewPrimitive && dx >= kFloat_neg0_001_WViewPointTolerance &&
                dy <= kFloat_0_001_ViewPrimitive && dy >= kFloat_neg0_001_WViewPointTolerance)
                list[1] = list[2];
        }
        Submit(self, list, flags, 2, executeArgument);
        return;
    }
    obj = CurrentPrimitiveRenderTarget(self);
    if (!(obj ? obj->IsActive() : 0) && type != kPreparedVertexType) {
        list = self->Prepare(list, type, 3);
        if (!list)
            return;
    }
    int count = 3;
    ViewVertexList p = list + 3;
    while (*p) {
        ++p;
        ++count;
    }
    Submit(self, list, flags, count, executeArgument);
}

// Build an orthonormal look-at frame, using an alternate up axis near vertical.
// cameraFrame layout (floats): [0..2] right axis, [3..5] up axis, [6..8] forward axis
// (normalized lookAt - eye), [9..11] eye position. Usual case: up hint (0,1,0); right = hint x forward,
// then up = forward x right. If |forward.y| is within a small epsilon of the reference constant
// (forward nearly vertical) the hint is not usable: up = forward x (1,0,0) and right = up x forward.
// Axes are normalized as they are built (the near-vertical branch relies on the cross products).
// Required nonaliasing output[12], eye[3], lookAt[3]; eye and lookAt must differ.
// No error return for degenerate axes; keep fallback branch and normalize order.
void Matrix_BuildCameraTransform(float* cameraFrame, float* eyePosition, float* lookAtPosition) {

    Matrix_SetIdentityAffine(cameraFrame);
    float forwardDifference[3];
    __mi__FRC7WVectorRC7WVector(forwardDifference, lookAtPosition, eyePosition);
    __as__7WVectorFRC7WVector(6 + cameraFrame, forwardDifference);
    Normalize__7WVectorFv(6 + cameraFrame);
    // Start with the forward-axis Y component, then measure distance from the alignment reference.
    float verticalAlignmentDifference = cameraFrame[7];
    verticalAlignmentDifference = (verticalAlignmentDifference > kFloat_0_0_CameraView)
                                      ? verticalAlignmentDifference
                                      : 0.0f - verticalAlignmentDifference;
    verticalAlignmentDifference = verticalAlignmentDifference - s_cameraVerticalAlignmentReference;
    if (*(unsigned int*)&verticalAlignmentDifference & 0x80000000)
        verticalAlignmentDifference = 0.0f - verticalAlignmentDifference;
    // original 0x800667E4-0x800667F4: fcmpo/mfcr/srwi. takes this branch when verticalAlignmentDifference < eps (forward nearly vertical);
    // its axis is (1,0,0): 0x800667F8-0x80066814 store kFloat_1_view at sp+0x3C, kFloat_0_0_CameraView at sp+0x40/+0x44
    int nearVertical = verticalAlignmentDifference < kFloat_0_00001_CameraView;
    if (nearVertical) {
        float crossProduct[3];
        float up[3] = {kFloat_1_view, kFloat_0_0_CameraView, kFloat_0_0_CameraView};

        Vector_CrossProduct(crossProduct, 6 + cameraFrame, up);
        float orthogonalAxis[3];
        __as__7WVectorFRC7WVector(3 + cameraFrame, crossProduct);
        Vector_CrossProduct(orthogonalAxis, cameraFrame + 3, (6 + cameraFrame));
        __as__7WVectorFRC7WVector(cameraFrame, orthogonalAxis);
    } else {
        float up[3] = {kFloat_0_0_CameraView, kFloat_1_view, kFloat_0_0_CameraView};
        float crossProduct[3];

        __as__7WVectorFRC7WVector(3 + cameraFrame, up);
        float orthogonalAxis[3];
        Vector_CrossProduct(crossProduct, (cameraFrame + 3), (6 + cameraFrame));
        __as__7WVectorFRC7WVector(cameraFrame, crossProduct);

        Normalize__7WVectorFv(cameraFrame);
        Vector_CrossProduct(orthogonalAxis, (cameraFrame + 6), cameraFrame);
        __as__7WVectorFRC7WVector(3 + cameraFrame, orthogonalAxis);
        Normalize__7WVectorFv(cameraFrame + 3);
    }
    __as__7WVectorFRC7WVector((9 + cameraFrame), eyePosition);
}

// Convert a viewport point and normalized depth into a rotated camera ray.
// Required output ray[3] and initialized camera, nonzero viewport width. Input
// screen xy is relative to full viewport, depth linearly selects near..far.
// Result is a rotated direction/displacement, NOT a unit vector or world origin:
// no camera position is added. Preserve final X/Z negations and transform order.
void Camera_ScreenToRay(float* ray, void* camera, float screenX, float screenY, float normalizedDepth) {
    float inputX = screenX;
    float farClip = AsWView(camera)->farClip;

    float h0 = kFloat_0_5_View;

    float nearClip = AsWView(camera)->nearClip;
    ray[2] = normalizedDepth * (farClip - nearClip) + nearClip;
    float angle = h0 * AsWView(camera)->fieldOfView;
    float tangent = (float)tan(angle);
    float half = kFloat_0_5_View;
    float w = AsWView(camera)->viewportWidth;
    float neg = kFloat_neg1_View;
    float scale = ray[2] * tangent;
    float denom = (w * half);
    scale /= denom;
    ray[0] = scale * (inputX - w * half);
    ray[1] = ((neg * scale)) * ((screenY)-half * AsWView(camera)->viewportHeight);
    float transformedRay[3];
    Matrix_TransformDirection(transformedRay, ray, (&AsWView(camera)->viewMatrix[0]));
    __as__7WVectorFRC7WVector(ray, transformedRay);
    float ox = ray[0];
    float n2 = kFloat_neg1_View;

    ray[0] = (ox * n2);
    ray[2] = (ray[2]) * n2;
}

// Refresh view transforms and build normal and inset frustum planes.
// Requires nonzero target width/height and usable camera basis; writes centers,
// twelve planes, input/applied transforms, and view dirty bit. No error status.
// Preserve full/inset pairing and float classification/normalization order;
// the four-pixel inset changes culling, independently of the perspective matrix.
void Camera_BuildFrustumPlanes(void* self) {
    float inverseInputFrame[12];
    char* p;
    int i;
    char* camera = (char*)self;

    float inverseAppliedFrame[12];
    float nearPlane[4];
    float nearWorldNormal[3];
    float nearViewNormal[3];
    float nearWorldPoint[3];
    float nearWorldOffset[3];
    float farPlane[4], farWorldNormal[3], farViewNormal[3], farWorldPoint[3], farWorldOffset[3];
    float leftPlane[4], leftWorldNormal[3], leftViewNormal[3];
    float insetLeftPlane[4], insetLeftWorldNormal[3], insetLeftViewNormal[3];
    float rightPlane[4], rightWorldNormal[3], rightViewNormal[3];
    float insetRightPlane[4], insetRightWorldNormal[3], insetRightViewNormal[3];
    float topPlane[4];
    float topWorldNormal[3];
    float topViewNormal[3];
    float insetTopPlane[4], insetTopWorldNormal[3], insetTopViewNormal[3];
    float bottomPlane[4];
    float bottomWorldNormal[3];
    float bottomViewNormal[3];
    float insetBottomPlane[4];
    float insetBottomWorldNormal[3];
    float insetBottomViewNormal[3];
    // Scratch planes are [normal.xyz, distance]; normals start in camera space.
    // sideSlopes = [inset X, inset Y, full X, full Y], measured as tan(half angle).
    float sideSlopes[4];
    float neg;

    // Phase 1: derive pixel center and full/inset projection slopes (four-pixel border).
    Matrix_CopyAffine(camera + kViewMatrixOffset, camera + kPendingViewMatrixOffset);
    float half = kFloat_0_5_View;

    float w = AsWView(camera)->viewportWidth;
    float h = AsWView(camera)->viewportHeight;
    float angle = half * (AsWView(camera)->fieldOfView);
    float product2 = half * h;
    float product3 = h * half;
    float halfW = half * w;
    float halfH = product3;
    AsWView(camera)->halfViewportWidth = halfW;
    AsWView(camera)->halfViewportHeight = halfH;
    AsWView(camera)->screenCenterX = halfW;
    AsWView(camera)->screenCenterY = halfH;
    sideSlopes[2] = (float)tan(angle);
    i = 0;
    p = camera;
    sideSlopes[3] = sideSlopes[2] * (AsWView(camera)->viewportHeight / AsWView(camera)->viewportWidth);
    sideSlopes[0] = (AsWView(camera)->halfViewportWidth - kFloat_4_CameraBounds) /
                    (AsWView(camera)->halfViewportWidth / sideSlopes[2]);
    sideSlopes[1] = (AsWView(camera)->halfViewportHeight - kFloat_4_CameraBounds) /
                    (AsWView(camera)->halfViewportHeight / sideSlopes[3]);
    float zero = kFloat_0_0_CameraView;
    do {
        if (__fpclassifyf(*(float*)(p + kViewMatrixOffset)) == 1)
            *(float*)(p + kViewMatrixOffset) = zero;
        p = (4 + p);
        i += 1;
    } while (12 > i);

    // Phase 2: convert the pending camera basis and cache world-to-view inverses.
    __amu__7WVectorFf(camera + kViewMatrixOffset, kFloat_neg1_View);
    __amu__7WVectorFf(camera + kForwardAxisOffset, kFloat_neg1_View);
    Matrix_InvertAffine(inverseInputFrame, camera + kViewMatrixOffset);
    Matrix_CopyAffine(camera + kInputTransformOffset, inverseInputFrame);
    Matrix_InvertAffine(inverseAppliedFrame, camera + kViewMatrixOffset);
    Matrix_CopyAffine(camera + kAppliedViewMatrixOffset, inverseAppliedFrame);

    // Phase 3: near/far planes share their points with the inset frustum.
    // Points are camera position + forward axis * clip distance; normals rotate to world space.
    nearViewNormal[0] = kFloat_0_0_CameraView;
    nearViewNormal[1] = kFloat_0_0_CameraView;
    nearViewNormal[2] = kFloat_neg1_View;
    __ml__FRC7WVectorf(nearWorldOffset, camera + kForwardAxisOffset, AsWView(camera)->nearClip);
    __pl__FRC7WVectorRC7WVector(nearWorldPoint, camera + kCameraPositionOffset, nearWorldOffset);
    Matrix_TransformDirection(nearWorldNormal, nearViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(
        camera + kInsetNearPlaneOffset,
        Plane_Copy(camera + kNearPlaneOffset, Plane_FromNormalAndPoint(nearPlane, nearWorldNormal, nearWorldPoint)));

    farViewNormal[0] = kFloat_0_0_CameraView;
    farViewNormal[1] = kFloat_0_0_CameraView;
    farViewNormal[2] = kFloat_1_view;
    __ml__FRC7WVectorf(farWorldOffset, camera + kForwardAxisOffset, AsWView(camera)->farClip);
    __pl__FRC7WVectorRC7WVector(farWorldPoint, camera + kCameraPositionOffset, farWorldOffset);
    Matrix_TransformDirection(farWorldNormal, farViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kInsetFarPlaneOffset,
               Plane_Copy(camera + kFarPlaneOffset, Plane_FromNormalAndPoint(farPlane, farWorldNormal, farWorldPoint)));

    // Phase 4: each side plane passes through the camera position. Normalize after
    // rotating its camera-space normal; inset sides use the smaller slopes.
    leftViewNormal[0] = kFloat_neg1_View;
    leftViewNormal[1] = kFloat_0_0_CameraView;
    leftViewNormal[2] = -sideSlopes[2];
    Matrix_TransformDirection(leftWorldNormal, leftViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kLeftPlaneOffset, Plane_FromNormalAndPoint(leftPlane, Normalize__7WVectorFv(leftWorldNormal),
                                                                   camera + kCameraPositionOffset));
    insetLeftViewNormal[0] = kFloat_neg1_View;
    insetLeftViewNormal[1] = kFloat_0_0_CameraView;
    insetLeftViewNormal[2] = -sideSlopes[0];
    Matrix_TransformDirection(insetLeftWorldNormal, insetLeftViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kInsetLeftPlaneOffset,
               Plane_FromNormalAndPoint(insetLeftPlane, Normalize__7WVectorFv(insetLeftWorldNormal),
                                        camera + kCameraPositionOffset));
    rightViewNormal[0] = kFloat_1_view;
    rightViewNormal[1] = kFloat_0_0_CameraView;
    rightViewNormal[2] = -sideSlopes[2];
    Matrix_TransformDirection(rightWorldNormal, rightViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kRightPlaneOffset, Plane_FromNormalAndPoint(rightPlane, Normalize__7WVectorFv(rightWorldNormal),
                                                                    camera + kCameraPositionOffset));
    insetRightViewNormal[0] = kFloat_1_view;
    insetRightViewNormal[1] = kFloat_0_0_CameraView;
    insetRightViewNormal[2] = -sideSlopes[0];
    Matrix_TransformDirection(insetRightWorldNormal, insetRightViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kInsetRightPlaneOffset,
               Plane_FromNormalAndPoint(insetRightPlane, Normalize__7WVectorFv(insetRightWorldNormal),
                                        camera + kCameraPositionOffset));
    topViewNormal[0] = kFloat_0_0_CameraView;
    topViewNormal[1] = kFloat_1_view;
    neg = -sideSlopes[3];
    topViewNormal[2] = neg;
    Matrix_TransformDirection(topWorldNormal, topViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kTopPlaneOffset, Plane_FromNormalAndPoint(topPlane, Normalize__7WVectorFv(topWorldNormal),
                                                                  camera + kCameraPositionOffset));
    neg = -sideSlopes[1];
    insetTopViewNormal[0] = kFloat_0_0_CameraView;
    insetTopViewNormal[1] = kFloat_1_view;
    insetTopViewNormal[2] = neg;
    Matrix_TransformDirection(insetTopWorldNormal, insetTopViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kInsetTopPlaneOffset,
               Plane_FromNormalAndPoint(insetTopPlane, Normalize__7WVectorFv(insetTopWorldNormal),
                                        camera + kCameraPositionOffset));
    bottomViewNormal[0] = kFloat_0_0_CameraView;
    neg = -sideSlopes[3];
    bottomViewNormal[1] = kFloat_neg1_View;
    bottomViewNormal[2] = neg;
    Matrix_TransformDirection(bottomWorldNormal, bottomViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kBottomPlaneOffset,
               Plane_FromNormalAndPoint(bottomPlane, Normalize__7WVectorFv(bottomWorldNormal),
                                        camera + kCameraPositionOffset));
    insetBottomViewNormal[0] = kFloat_0_0_CameraView;
    neg = -sideSlopes[1];
    insetBottomViewNormal[1] = kFloat_neg1_View;
    insetBottomViewNormal[2] = neg;
    Matrix_TransformDirection(insetBottomWorldNormal, insetBottomViewNormal, camera + kViewMatrixOffset);
    Plane_Copy(camera + kInsetBottomPlaneOffset,
               Plane_FromNormalAndPoint(insetBottomPlane, Normalize__7WVectorFv(insetBottomWorldNormal),
                                        camera + kCameraPositionOffset));
    AsWView(camera)->dirtyFlags |= kViewMatrixDirty;
}

// Refresh the render target and viewport, rebuild frustum planes, and clear projection offsets.
// Null targetSlot keeps prior dimensions/target, but still rebuilds. A nonnull
// slot must contain a live target (no null guard before dimension dispatch).
// Custom rect survives dimension changes; full-target rect tracks width/height.
void Camera_SetTarget(void* self) {
    unsigned int flags = AsWView(self)->dirtyFlags;
    AsWView(self)->dirtyFlags = flags & ~1u;
    void* slot = AsWView(self)->targetSlot;
    if (slot) {
        AsWView(self)->renderTarget = *(void**)slot;
        // Target vtable slots 15/16 supply X/Y pixel extents, respectively.
        typedef int (*GetDim)(void*);
        float targetHeight = (float)((GetDim)(*(void***)AsWView(self)->renderTarget)[16])(AsWView(self)->renderTarget);
        float targetWidth = (float)((GetDim)(*(void***)AsWView(self)->renderTarget)[15])(AsWView(self)->renderTarget);
        float zero = kFloat_0_0_CameraView;
        AsWView(self)->viewportHeight = targetHeight;
        AsWView(self)->viewportWidth = targetWidth;
        if (!AsWView(self)->customViewport) {
            float tmp[4];
            tmp[0] = zero;
            tmp[1] = zero;
            tmp[2] = targetWidth;
            tmp[3] = targetHeight;
            AsWView(self)->viewportLeft = zero;
            AsWView(self)->viewportTop = zero;
            AsWView(self)->viewportExtentX = targetWidth;
            AsWView(self)->viewportExtentY = targetHeight;
        }
    }
    Camera_BuildFrustumPlanes(self);
    AsWView(self)->projectionOffsetY = kFloat_0_0_CameraView;
    AsWView(self)->projectionOffsetX = kFloat_0_0_CameraView;
}

// Reject zero-length basis axes, then queue a new camera frame and mark the view dirty.
// matrix is required 12-float frame; rejection leaves pending frame/flags intact.
// The 0.00001 tolerance tests each axis against zero, not orthogonality/length=1.
void WView_SetViewMatrix(void* self, void* matrix) {
    struct ZeroVector {
        float v[3];
        // Construct a zero vector for the basis-axis validity test.
        ZeroVector() {
            v[0] = 0.0f;
            v[1] = 0.0f;
            v[2] = 0.0f;
        }
    };
    if (Vec3_NearlyEqualWithTolerance(matrix, ZeroVector().v, 0.00001f) ||
        Vec3_NearlyEqualWithTolerance(((CameraAffineFrame*)matrix)->axes[1], ZeroVector().v, 0.00001f) ||
        Vec3_NearlyEqualWithTolerance(((CameraAffineFrame*)matrix)->axes[2], ZeroVector().v, 0.00001f))
        return;
    AsWView(self)->dirtyFlags |= kViewAndTargetDirty;
    Matrix_CopyAffine((&AsWView(self)->pendingViewMatrix[0]), matrix);
}

// Use the full target or clamp the supplied rectangle to its current viewport.
// values is null or four borrowed floats {left,top,width,height}; null disables
// custom mode. Only origins are lower-clamped and sizes upper-clamped. Negative
// sizes or origins beyond the target can remain invalid: no full sanitization.
// Writes rect/custom flag only; caller must coordinate renderer/scissor updates.
void WView_SetViewportRect(void* self, float* values) {

    if (values == 0) {
        float unused[4];
        float zero = kFloat_0_0_CameraView;

        AsWView(self)->customViewport = 0;

        float* output = (float*)(((char*)&AsWView(self)->viewportLeft));
        unused[0] = zero;
        unused[1] = zero;
        unused[2] = AsWView(self)->viewportWidth;
        unused[3] = AsWView(self)->viewportHeight;
        output[0] = unused[0];
        output[3] = unused[3];
        output[2] = unused[2];
        output[1] = unused[1];
        return;
    }
    float* output = (float*)(((char*)&AsWView(self)->viewportLeft));

    float zero = kFloat_0_0_CameraView;

    AsWView(self)->customViewport = 1;
    output[0] = zero < values[0] ? values[0] : zero;
    float zero2 = kFloat_0_0_CameraView;
    output[1] = zero2 < values[1] ? values[1] : zero2;
    float upper = AsWView(self)->viewportWidth - output[0];
    output[2] = values[2] > upper ? upper : values[2];
    upper = AsWView(self)->viewportHeight - output[1];
    output[3] = values[3] > upper ? upper : values[3];
}

// Submit a two-endpoint colored line while enabling the target's immediate primitive mode.
// Both XYZ inputs are required in the view's input space. Colors are packed
// words, flags are WView primitive bits. Stack vertices/list cannot escape Draw;
// preserve volatile stores and target enable -> draw -> reloaded-target disable.
void WView_DrawLine(void* self, const float* startPosition, unsigned int startColor, float* endPosition,
                    unsigned int endColor, unsigned int flags) {
    struct Vertex {
        float x, y, z, w;
        unsigned int color;
        char rest[0x24];
    };
    struct Object {
        virtual void v00() DECOMP_LOCAL_VIRTUAL;
        virtual void v01() DECOMP_LOCAL_VIRTUAL;
        virtual void v02() DECOMP_LOCAL_VIRTUAL;
        virtual void v03() DECOMP_LOCAL_VIRTUAL;
        virtual void draw(void*, int, unsigned int, int) DECOMP_LOCAL_VIRTUAL;
        virtual void v07() DECOMP_LOCAL_VIRTUAL;
        virtual void v08() DECOMP_LOCAL_VIRTUAL;
        virtual void v09() DECOMP_LOCAL_VIRTUAL;
        virtual void v10() DECOMP_LOCAL_VIRTUAL;
        virtual void v11() DECOMP_LOCAL_VIRTUAL;
        virtual void v12() DECOMP_LOCAL_VIRTUAL;
        virtual void v13() DECOMP_LOCAL_VIRTUAL;
        virtual void v14() DECOMP_LOCAL_VIRTUAL;
        virtual void v15() DECOMP_LOCAL_VIRTUAL;
        virtual void v16() DECOMP_LOCAL_VIRTUAL;
        virtual void v17() DECOMP_LOCAL_VIRTUAL;
        virtual void v18() DECOMP_LOCAL_VIRTUAL;
        virtual void v19() DECOMP_LOCAL_VIRTUAL;
        virtual void v20() DECOMP_LOCAL_VIRTUAL;
        virtual void v21() DECOMP_LOCAL_VIRTUAL;
        virtual void v22() DECOMP_LOCAL_VIRTUAL;
        virtual void v23() DECOMP_LOCAL_VIRTUAL;
        virtual void v24() DECOMP_LOCAL_VIRTUAL;
        virtual void v25() DECOMP_LOCAL_VIRTUAL;
        virtual void v26() DECOMP_LOCAL_VIRTUAL;
        virtual void v27() DECOMP_LOCAL_VIRTUAL;
        virtual void v28() DECOMP_LOCAL_VIRTUAL;
        virtual void v29() DECOMP_LOCAL_VIRTUAL;
        virtual void enable(int) DECOMP_LOCAL_VIRTUAL;
    };
    Vertex second, first;
    float x1 = startPosition[0];
    float z2, y2, x2, z1, y1;
    y1 = startPosition[1];
    z1 = startPosition[2];
    void* volatile work[3];
    work[0] = &first;
    x2 = endPosition[0];
    *(volatile float*)&first.x = x1;
    Vec3f* p0 = (Vec3f*)((void* volatile*)work)[0];
    work[1] = &second;
    y2 = endPosition[1];
    work[2] = 0;
    z2 = endPosition[2];
    first.color = startColor;
    second.color = endColor;
    p0->y = y1;
    ((Vec3f*)((void* volatile*)work)[0])->z = z1;
    Vec3f* p = (Vec3f*)((void* volatile*)work)[1];
    p->x = x2;
    p->y = y2;
    p->z = z2;
    Object* object = (*(Object**)&AsWView(self)->renderTarget);
    if (object) {
        object->enable(1);
    }
    ((Object*)self)->draw((void*)work, kPositionColorVertexType, flags | kLinePrimitive, 0);
    object = (*(Object**)&AsWView(self)->renderTarget);
    if (object) {
        object->enable(0);
    }
}

// Reserved camera hook; the original function performs no work.
void WView_DefaultCameraHook() {}

// Set clip distances and depth coefficients, optionally updating the active render target.
// No allocation or clamping. Keep near/far ordered for the intended projection;
// equal distances divide by zero. Changes mark projection dirty, not frustum
// planes: coordinate any later culling rebuild with Camera_SetTarget.
void Camera_SetClip(void* self, float nearClip, float farClip, int update) {
    // nearClip-relative float slots: 0/1 near/far, 2/3 clip-to-depth-offset ratios,
    // 12/13 depthProjectionScale/Offset. z maps through scale + offset / z.
    // Caller supplies distinct clip distances; update != 0 also writes target coefficients.
    float* p = (float*)(((char*)&AsWView(self)->nearClip));
    if (p[0] != nearClip || p[1] != farClip)
        AsWView(self)->dirtyFlags |= kViewProjectionDirty;
    void* obj = AsWView(self)->renderTarget;
    float diff = nearClip - farClip;
    float ratio = nearClip / diff;
    p[0] = nearClip;
    p[1] = farClip;
    p[12] = ratio;
    p[13] = -(ratio * p[0]);
    p[2] = nearClip / p[13];
    p[3] = farClip / p[13];
    if (!obj)
        return;
    if (!update)
        return;
    ((RenderTargetDepthState*)obj)->depthScale = ratio;
    obj = AsWView(self)->renderTarget;
    ((RenderTargetDepthState*)obj)->depthOffset = p[13];
}

void* Engine_SetCachedRenderState(void*, int);

// Accept the borrowed vertex list unless every vertex shares an outside clip bit.
// list is null-terminated; return list on acceptance, null on rejection. Nothing
// is allocated, copied or freed. An empty list is rejected unless clipping is skipped.
// The cached render state is changed BEFORE testing: mode 0 when disabled, mode 2
// otherwise, even for rejected lists. This call does not submit any vertices.
void* WView_PrepareVertexList(void*, void** list, unsigned int flags) {
    void** cursor;
    unsigned int mask;
    int arg = 2;
    if (flags & kDisableCachedRenderState)
        arg = 0;
    Engine_SetCachedRenderState(0, arg);
    mask = kAllClipBits;
    if (!(flags & kSkipCommonClipTest)) {
        cursor = list;
        while (*cursor != 0) {
            void* item = *cursor;
            unsigned char byte = ((WViewProjectedPoint*)item)->clipFlags;
            mask = mask & byte;
            cursor = cursor + 1;
        }
        if (mask != 0)
            return 0;
    }
    return list;
}

// Set the pixel center used for projected-point placement.
// Set the pixel-valued divisors used for projection-center offsets; no rebuild.
// Values must be nonzero for nonzero offsets. BuildFrustumPlanes overwrites
// these with half full-target dimensions; preserve setter/rebuild sequencing.
void WView_SetScreenCenter(void* self, float centerX, float centerY) {
    AsWView(self)->screenCenterX = centerX;
    AsWView(self)->screenCenterY = centerY;
}

// Concrete WGeneric3DR target dispatch (phase1_class_tables; implementations in
// wgeneric3d_render_queue_and_soft_keyboard.cpp/wgeneric3dr_renderer_core.cpp/wgeneric3dr_immediate_renderer.cpp). C++ virtual ordinals below start at zero;
// actual table byte offset = (ordinal+2)*4. Do not mix these with raw table indices.
// ord7/+24: WGeneric3DR::DispatchCommand(command,value,output). Command11 sends value as
//   FLUSH FLAGS to ord33/+8C WGeneric3D_Flush: mask1 two-pass,mask2 reverse,
//   mask4 tagged-quad batches; resets staging/queue counts and executes pending
//   passes (suppressed renderer discards them). output=0 is unused for command11.
// ord30/+80: WGeneric3D_vfunc_30(textureOverride) stores resourceOverride for
//   subsequent submissions. Bitmap draws in overlay_sprite_drawing.cpp pass texture then 0 to
//   restore no override; borrowed resource word, not a retained reference.
// ord12/+38: CompleteRenderSubmission flushes with flags0 and changes active
//   submission state2->idle0; pending1 stays pending (wgeneric3dr_renderer_core.cpp).
// ord4/+18: SubmitPrimitive(target,list,flags,count,executeArgument), see above.
// ord29/+7C: WGeneric3D_vfunc_29 returns position-mode selector (not renderer readiness).
// ord16/+48: WGeneric3D_SetFogState stores incoming f1/f2 and low24 of r4 in renderer settings.
//   QueryCurrentRenderTarget's historical int signature hides those FP inputs;
//   its known callers in course_scene_render_and_geometry.cpp ignore the result. Only null has a defined
//   zero return here; do not use the nonnull return as a status/capability query.
// +80 alone permits null->no-op; +38/+18/+24 require a live renderer.
// No target/texture/payload ownership transfers. All render calls are serialized.
struct VirtualObject {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4(void*, unsigned int, int, void*);
    virtual void v5();
    virtual void v6();
    virtual void v7(int, void*, int);
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
    virtual int v16(int);
    virtual void v17();
    virtual int v18();
    virtual void v19();
    virtual void v20();
    virtual void v21();
    virtual void v22();
    virtual void v23();
    virtual void v24();
    virtual void v25();
    virtual void v26();
    virtual void v27();
    virtual void v28();
    virtual void v29();
    virtual void v30(void*);
    virtual void v31();
    virtual void v32();
};

struct DispatchOwner {
    char pad[0xA4];
    VirtualObject* object;
};

// Resolve the target slot and forward renderer parameters through v16, or return zero if null.
// Precondition: targetSlot points to a live slot; its contained target may be null.
int WView_ForwardRendererParameters(void* self, int value) {
    void* object = *(*(void***)&AsWView(self)->targetSlot);
    AsWView(self)->renderTarget = object;
    if (object == 0)
        return 0;
    return ((VirtualObject*)object)->v16(value);
}

struct Camera {
    char pad[0x1B0];
    float fov;
    char pad2[0x19F4 - 0x1B4];
    unsigned int flags;
};

// Update field of view only when it changes and invalidate the projection matrix.
// fov is horizontal radians (default pi/2). No clamp: callers must avoid
// degenerate tangent/projection inputs. This does not rebuild culling planes.
void Camera_SetFov(Camera* self, float fov) {
    if (self->fov == fov)
        return;
    self->fov = fov;
    self->flags |= kViewProjectionDirty;
}

// Flush the renderer through command 11. flushFlags carries flag BITS in the pointer
// word (0..7 combinations above), never a dereferenced payload buffer.
void WView_FlushRenderQueue(DispatchOwner* self, void* flushFlags) {
    self->object->v7(11, flushFlags, 0);
}

// Set/clear the optional target's borrowed texture override. Explicit second
// parameter exposes r4 previously passed by bitmap callers but hidden here.
void WView_SetOptionalTextureOverride(DispatchOwner* self, void* textureOverride) {
    if (self->object)
        self->object->v30(textureOverride);
}

// Complete render submission: flush and release the renderer's active phase.
// Requires a nonnull target, as does WView_ForwardPreparedPrimitive; slot 30 alone is optional.
void WView_CompleteRenderSubmission(DispatchOwner* self) {
    self->object->v12();
}

// Tail-forward the primitive list/flags/count/execute word to the real renderer.
// Named parameters expose the existing ABI; no input is retained by this shim.
void WView_ForwardPreparedPrimitive(DispatchOwner* self, void* list, unsigned int flags, int count,
                                    void* executeArgument) {
    self->object->v4(list, flags, count, executeArgument);
}

float Math_SinApprox(float);
float Math_CosApprox(float);

struct DrawVtx {
    float x, y, z, w;
    unsigned int color;
    char rest[0x24];
};

struct DrawObj {
    virtual void o0();
    virtual void o1();
    virtual void o2();
    virtual void o3();
    virtual void o4();
    virtual void o5();
    virtual void o6();
    virtual void o7();
    virtual void o8();
    virtual void o9();
    virtual void o10();
    virtual void o11();
    virtual void o12();
    virtual void o13();
    virtual void o14();
    virtual void o15();
    virtual void o16();
    virtual void o17();
    virtual void o18();
    virtual void o19();
    virtual void o20();
    virtual void o21();
    virtual void o22();
    virtual void o23();
    virtual void o24();
    virtual void o25();
    virtual void o26();
    virtual void o27();
    virtual void enable(int);
};

struct DrawSelf {
    virtual void d0();
    virtual void d1();
    virtual void d2();
    virtual void d3();
    virtual void draw(void* list, int stride, unsigned int flags, int arg);
};

// Submit a borrowed two-endpoint position/color list in the target's immediate mode.
// Draw may change the target, so reload it before disabling the mode. No list escapes.
static inline void submitImmediateLine(void* self, void* vertexList) {
    DrawObj* obj = (*(DrawObj**)&AsWView(self)->renderTarget);
    if (obj)
        obj->enable(1);
    ((DrawSelf*)self)->draw(vertexList, kPositionColorVertexType, kLinePrimitive, 0);
    obj = (*(DrawObj**)&AsWView(self)->renderTarget);
    if (obj)
        obj->enable(0);
}

// Draw a camera-facing ring, optionally highlighting periodic edges in green.
// Legacy return ABI is float but this body defines no return value; all known
// draw consumers ignore it. Do not interpret it as a radius/status or add a
// return computation during cleanup. Vec4 wrapper forwards the same ABI.
float WView_DrawFacingCircle(void* self, float* center, float radius, unsigned int color, int segmentCount,
                             int highlightInterval) {
    // Phases: generate the ring in the supplied facing basis, then submit consecutive
    // projected endpoints; the first edge joins point 0 to the last point,
    // followed by edges (1,0), (2,1), ... in point-index order.
    // center and radius use the input world space; viewMatrix supplies facing axes.
    // Caller supplies segmentCount > 0 (clamped to 24). Angles are radians.
    // Admitted highlighting interval: 0 disables it; 1..2147483646 highlights
    // every (highlightInterval+1)th edge. An interval >= the clamped segmentCount
    // produces no highlighted edge. No validation: -1 divides by zero, INT_MAX
    // overflows the signed increment, and other negative values are outside this
    // nonnegative spacing contract. Keep both caller limits when adding rings.
    // Draw ABI: a null-terminated pointer list, vertex type 0x18, packed line flags.
    // Target enable/disable brackets each edge and is refreshed after submission.
    if (segmentCount > kMaximumCircleSegments)
        segmentCount = kMaximumCircleSegments;

    float circlePoints[kMaximumCircleSegments + 1][3];
    DrawVtx plainEndpoints[2];
    DrawVtx highlightEndpoints[2];
    DrawVtx regularEndpoints[2];

    float worldPoint[3], centerPlusHorizontal[3], horizontalOffset[3], scaledRight[3], verticalOffset[3], scaledUp[3];

    float tau = kTwoPi_FacingCircle;
    int i;
    for (i = 0; i < segmentCount; ++i) {
        float angleRadians = tau * (float)i / (float)segmentCount;
        float sine = Math_SinApprox(angleRadians);
        __ml__FRC7WVectorf(scaledUp, (&AsWView(self)->viewMatrix[3]), radius);
        __ml__FRC7WVectorf(verticalOffset, scaledUp, sine);
        float cosineAngleRadians = tau * (float)i / (float)segmentCount;
        float cosine = Math_CosApprox(cosineAngleRadians);
        __ml__FRC7WVectorf(scaledRight, (&AsWView(self)->viewMatrix[0]), radius);
        __ml__FRC7WVectorf(horizontalOffset, scaledRight, cosine);
        __pl__FRC7WVectorRC7WVector(centerPlusHorizontal, center, horizontalOffset);
        __pl__FRC7WVectorRC7WVector(worldPoint, centerPlusHorizontal, verticalOffset);
        __as__7WVectorFRC7WVector(circlePoints[i], worldPoint);
    }

    int previousIndex = segmentCount - 1;
    float* currentPoint = (float*)circlePoints;
    i = 0;
    while (i < segmentCount) {
        if (highlightInterval == 0) {
            void* volatile plainVertexList[3];
            plainVertexList[0] = plainEndpoints;
            plainVertexList[1] = plainEndpoints + 1;
            plainVertexList[2] = 0;
            plainEndpoints[0].color = color;
            plainEndpoints[1].color = color;
            plainEndpoints[0].x = currentPoint[0];
            ((DrawVtx*)plainVertexList[0])->y = currentPoint[1];
            ((DrawVtx*)plainVertexList[0])->z = currentPoint[2];
            float* previousPoint = (float*)((char*)circlePoints + previousIndex * 12);
            DrawVtx* previousVertex = (DrawVtx*)plainVertexList[1];
            previousVertex->x = previousPoint[0];
            previousVertex->y = previousPoint[1];
            previousVertex->z = previousPoint[2];
            submitImmediateLine(self, (void*)plainVertexList);
        } else if ((i + 1) % (highlightInterval + 1) == 0) {
            void* volatile highlightVertexList[3];
            highlightVertexList[0] = highlightEndpoints;
            highlightVertexList[1] = highlightEndpoints + 1;
            highlightVertexList[2] = 0;
            highlightEndpoints[0].color = kHighlightGreen;
            highlightEndpoints[1].color = kHighlightGreen;
            highlightEndpoints[0].x = currentPoint[0];
            ((DrawVtx*)highlightVertexList[0])->y = currentPoint[1];
            ((DrawVtx*)highlightVertexList[0])->z = currentPoint[2];
            float* previousPoint = (float*)((char*)circlePoints + previousIndex * 12);
            DrawVtx* previousVertex = (DrawVtx*)highlightVertexList[1];
            previousVertex->x = previousPoint[0];
            previousVertex->y = previousPoint[1];
            previousVertex->z = previousPoint[2];
            submitImmediateLine(self, (void*)highlightVertexList);
        } else {
            void* volatile regularVertexList[3];
            regularVertexList[0] = regularEndpoints;
            regularVertexList[1] = regularEndpoints + 1;
            regularVertexList[2] = 0;
            regularEndpoints[0].color = color;
            regularEndpoints[1].color = color;
            regularEndpoints[0].x = currentPoint[0];
            ((DrawVtx*)regularVertexList[0])->y = currentPoint[1];
            ((DrawVtx*)regularVertexList[0])->z = currentPoint[2];
            float* previousPoint = (float*)((char*)circlePoints + previousIndex * 12);
            DrawVtx* previousVertex = (DrawVtx*)regularVertexList[1];
            previousVertex->x = previousPoint[0];
            previousVertex->y = previousPoint[1];
            previousVertex->z = previousPoint[2];
            submitImmediateLine(self, (void*)regularVertexList);
        }
        previousIndex = i;
        ++i;
        currentPoint += 3;
    }
    // The original returns no value: its callers never read one.
}

// Draw a ring using XYZ as its center and W as its radius.
// centerAndRadius is required four-float input; color is a packed 32-bit word
// carried as void*, never dereferenced. Same segment/highlight contract as above;
// forwarded float return is unspecified and must be ignored.
float WView_DrawFacingCircleVec4(void* view, Vec4f* centerAndRadius, void* color, int segmentCount,
                                 int highlightInterval) {
    return WView_DrawFacingCircle(view, (float*)centerAndRadius, centerAndRadius->w, (unsigned int)color, segmentCount,
                                  highlightInterval);
}

#pragma opt_strength_reduction off
// Draw transformed box faces or their edges using the shared corner/face tables.
// Required minXYZ/maxXYZ[6], valid view and shared tables. filled is boolean;
// return is void. Shared projected slots are overwritten, so calls cannot nest
// or run concurrently. Preserve per-slot draw timing and packed alpha handling.
void WView_DrawBox(void* self, float* minimumAndMaximum, unsigned int color, int filled) {
    // Phases: expand eight corners from min/max selectors; walk six indexed faces;
    // copy projected face vertices and submit either filled faces or outlines.
    // Each corner selector picks min/max independently for XYZ. Each face has
    // four corner indices; projectedVertexList borrows four shared 0x38-byte slots.
    // Filled mode submits after EACH slot update, as in the original draw ABI.
    // Packed color uses alpha in the top byte; translucent faces use the named flags.
    int* cornerSelectors;
    float boxCorners[kBoxCornerCount][3];
    float* cornerOutput;
    int i;
    unsigned char* boxDrawTables;
    unsigned char* faceIndices;
    unsigned int alpha;
    unsigned int* indices;
    unsigned char* vertex;
    int face;
    float cornerPosition[3];
    float viewPosition[3];
    void* volatile edgeVertexList[3];
    DrawVtx edgeEndpoints[2];

    boxDrawTables = s_boxDrawTopologyAndVertexPointers;
    cornerSelectors = ((BoxDrawTables*)boxDrawTables)->cornerSelectors[0];
    ((BoxFaceVertex*)s_boxFaceProjectedVertex0)[0].color = color;
    ((BoxFaceVertex*)s_boxFaceProjectedVertex0)[1].color = color;
    ((BoxFaceVertex*)s_boxFaceProjectedVertex0)[2].color = color;
    ((BoxFaceVertex*)s_boxFaceProjectedVertex0)[3].color = color;
    cornerOutput = (float*)boxCorners;
    for (i = 0; i < kBoxCornerCount; ++i) {
        float cornerZ = cornerSelectors[2] ? ((BoxInputBounds*)minimumAndMaximum)->maximum[2]
                                           : ((BoxInputBounds*)minimumAndMaximum)->minimum[2];
        float cornerY = cornerSelectors[1] ? ((BoxInputBounds*)minimumAndMaximum)->maximum[1]
                                           : ((BoxInputBounds*)minimumAndMaximum)->minimum[1];
        float cornerX = cornerSelectors[0] ? ((BoxInputBounds*)minimumAndMaximum)->maximum[0]
                                           : ((BoxInputBounds*)minimumAndMaximum)->minimum[0];
        cornerPosition[0] = cornerX;
        cornerPosition[1] = cornerY;
        cornerPosition[2] = cornerZ;
        __as__7WVectorFRC7WVector(cornerOutput, cornerPosition);
        cornerSelectors += 3;
        cornerOutput += 3;
    }
    faceIndices = (unsigned char*)((BoxDrawTables*)boxDrawTables)->faceIndices[0];
    alpha = color >> 24;
    for (face = 0; face < kBoxFaceCount; ++face, faceIndices += 0x10) {
        indices = (unsigned int*)faceIndices;
        vertex = s_boxFaceProjectedVertex0;
        for (int j = 0; j < 4; ++j, ++indices, vertex += 0x38) {
            if (filled) {
                Matrix_TransformPoint(viewPosition, boxCorners[*indices], (&AsWView(self)->inputTransform[0]));
                WView_ProjectViewPoint(self, vertex, viewPosition, 0);
                ((DrawSelf*)self)
                    ->draw((void*)((BoxDrawTables*)boxDrawTables)->projectedVertexList, kProjectedVertexType,
                           alpha < kFullAlpha ? kTranslucentBoxFlags : 0, 0);
            } else {
                const volatile float* previousCorner;
                const float* currentCorner = boxCorners[(int)*indices];
                previousCorner = boxCorners[((unsigned int*)faceIndices)[(j + 3) & 3]];
                edgeVertexList[0] = edgeEndpoints;
                edgeVertexList[1] = edgeEndpoints + 1;
                edgeVertexList[2] = 0;
                *(volatile unsigned int*)&edgeEndpoints[0].color = color;
                edgeEndpoints[1].color = color;
                *(volatile float*)edgeEndpoints = previousCorner[0];
                ((DrawVtx*)edgeVertexList[0])->y = previousCorner[1];
                ((DrawVtx*)edgeVertexList[0])->z = previousCorner[2];
                DrawVtx* currentVertex = (DrawVtx*)edgeVertexList[1];
                currentVertex->x = currentCorner[0];
                currentVertex->y = currentCorner[1];
                currentVertex->z = currentCorner[2];
                submitImmediateLine(self, (void*)edgeVertexList);
            }
        }
    }
}

#pragma opt_strength_reduction reset
// Initialize this view-construction unit's mutable two-pi constant.
void WView_InitializeConstructionUnitConstants() {
    s_twoPi_ViewConstruction = kFloat_6_2831855;
}

} // extern "C"

// ---- merged from wview_camera_projection.cpp ----
namespace ns1 {

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_ViewProjection;
extern float s_twoPi_WView;
extern float s_twoPi_FileAsync;

extern char __vt__10CFileAsync[];
extern char s_tableLockEntries[];
extern const float s_resultMenuSharedQuadRect[];
extern const unsigned int s_resultMenuSharedQuadVertices[];
extern const float kFloat_0_0_CameraView;
extern const float kFloat_neg1_View;
extern const float kFloat_1_view;
extern const float kFloat_0_5_View;
extern const float kFloat_0_00001_CameraView;
extern const float kFloat_2_View;
// Fixed dimensionless perspective aspect=1.25, NOT target width/height. Defined
// in the data section; only RebuildCameraPerspective reads it in retail source.
extern const float kViewPerspectiveAspectRatio;
extern const unsigned int kViewportOriginDepthTransformTemplate[];
extern const char kIdentityMatrix4x4[];
void Camera_Apply(void*, float, float);
void MtxPerspectiveFovRH(void*, float, float, float, float);
void* Matrix_TransformPoint(void*, void*, void*);
void Vec3_CopyToOutput(void*, void*);
void Mtx44_TransformPointPerspective(void*, void*, void*);
void Vec3_ProjectToViewport(void*, void*, void*, void*, void*, const char*);
void Engine_GetViewport(void*);
void PSMTX44Transpose(void*, void*);
void PSMTX44Concat(void*, void*, void*);
void __construct_array(void*, void*, int, int, int);
int FileIO_PollAsyncTransferResult(int*, int*, int*);
int FileIO_GetFileSize(int, int*, int, int);
void File_GetLastError();
void FileIO_CloseFile(int);
void BaseObject_dtor(int, int);
void OSInitMutex(void*);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void __dl__FPv(void*);
unsigned int strlen(const char*);
void Engine_SetFlexibleVertexFormat(unsigned int);
void DrawPrimitiveUP(int, int, void*, int);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
void FilePath_EnsureDvdPrefix(void*);
int FileIO_OpenFile(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                    unsigned int);
void appFreeUni__FPv(void*);

struct TableLock {
    int active;
    int index;
};

struct TableLockEntry {
    int initialized;
    char lock[0x18];
};

struct LocalString {
    unsigned int words[3];
};

// Return the ordered float comparison as an unsigned zero or one.
static inline unsigned int LessThanF(float a, float b) {
    unsigned int r = a < b;
    return r;
}
#pragma opt_common_subs off
// Rebuild perspective from horizontal FOV and viewport aspect, preserving FP order.
// The first tan call is retained because the existing binary evaluates it twice.
static inline void RebuildCameraPerspective(char* viewBytes, float horizontalFovRadians) {
    tan(kFloat_0_5_View * horizontalFovRadians);
    float halfFovTangent = (float)tan(kFloat_0_5_View * horizontalFovRadians);
    float halfFovCotangent = kFloat_1_view / halfFovTangent;
    float verticalHalfFovTangent =
        (AsWView(viewBytes)->viewportHeight / AsWView(viewBytes)->viewportWidth) / halfFovCotangent;
    float verticalFovRadians = kFloat_2_View * (float)atan(verticalHalfFovTangent);
    MtxPerspectiveFovRH((&AsWView(viewBytes)->projectionMatrix[kProjectionHorizontalScale]), verticalFovRadians,
                        kViewPerspectiveAspectRatio, AsWView(viewBytes)->nearClip, AsWView(viewBytes)->farClip);
}

// Pack the applied affine frame as the renderer's saved homogeneous matrix.
static inline void SaveAppliedCameraView(char* viewBytes, unsigned int* appliedChanges) {
    float* appliedAffine = (float*)((&AsWView(viewBytes)->appliedViewMatrix[0]));
    float* savedHomogeneous = (float*)((&AsWView(viewBytes)->savedViewMatrix[0]));
    savedHomogeneous[0] = appliedAffine[0];
    savedHomogeneous[1] = appliedAffine[1];
    savedHomogeneous[2] = appliedAffine[2];
    savedHomogeneous[3] = 0.0f;
    savedHomogeneous[4] = appliedAffine[3];
    savedHomogeneous[5] = appliedAffine[4];
    savedHomogeneous[6] = appliedAffine[5];
    savedHomogeneous[7] = 0.0f;
    savedHomogeneous[8] = appliedAffine[6];
    savedHomogeneous[9] = appliedAffine[7];
    savedHomogeneous[10] = appliedAffine[8];
    savedHomogeneous[11] = 0.0f;
    savedHomogeneous[12] = appliedAffine[9];
    savedHomogeneous[13] = appliedAffine[10];
    savedHomogeneous[14] = appliedAffine[11];
    savedHomogeneous[15] = 1.0f;
    *appliedChanges |= kAppliedViewChanged;
}

// Refresh dirty view/projection matrices and apply the requested projection-center offset.
// Required initialized WView; centerOffsetX/Y are target-pixel displacements
// normalized by screenCenterX/Y. Writes saved/projection matrices, cached offsets,
// dirtyFlags and matrixUpdateFlags. No target binding, allocation or status return.
// Keep duplicate tan calls, divide order and the equal-offset dirty path's reset
// to zero: identical arguments do not guarantee retaining a nonzero offset.
void Camera_Apply(void* camera, float centerOffsetX, float centerOffsetY) {
    // Phases: consume view dirty bits into the saved 4x4 matrix, then rebuild
    // perspective if needed and apply center offsets. matrixUpdateFlags reports
    // which matrices changed; the no-offset-change path may return early.
    float horizontalSum, horizontalSpan;
    float projectionMaximumY, projectionLeft, projectionMinimumY, projectionRight;
    float frustumHorizontalSum, frustumHorizontalSpan, verticalSum, verticalSpan;
    char* viewBytes = (char*)camera;
    unsigned int* dirtyFlags = (unsigned int*)(((char*)&AsWView(viewBytes)->dirtyFlags));
    unsigned int* appliedChanges = (unsigned int*)(((char*)&AsWView(viewBytes)->matrixUpdateFlags));
    float horizontalFovRadians = AsWView(viewBytes)->fieldOfView;
    if (*dirtyFlags & kViewMatrixDirty) {
        *dirtyFlags &= ~kViewMatrixDirty;
        SaveAppliedCameraView(viewBytes, appliedChanges);
    } else
        *appliedChanges &= ~kAppliedViewChanged;
    // Projection policy (dirty includes a near-zero horizontal scale):
    // offsets changed + clean: recenter only; offsets changed + dirty: rebuild, recenter.
    // offsets same + dirty: reset stored offsets, rebuild, use plain frustum center.
    // offsets same + clean: clear projection-changed and keep the existing matrix.
    if (centerOffsetX != AsWView(viewBytes)->projectionOffsetX ||
        centerOffsetY != AsWView(viewBytes)->projectionOffsetY) {
        AsWView(viewBytes)->projectionOffsetX = centerOffsetX;
        AsWView(viewBytes)->projectionOffsetY = centerOffsetY;
        // Rebuild when dirty or when the horizontal projection scale is near zero.
        do {
            if (!(*dirtyFlags & kViewProjectionDirty)) {
                unsigned int horizontalScaleBits =
                    (*(unsigned int*)&AsWView(viewBytes)->projectionMatrix[kProjectionHorizontalScale]);
                float horizontalScaleMagnitude;
                if (horizontalScaleBits & kViewFloatSignBit)
                    horizontalScaleMagnitude = -AsWView(viewBytes)->projectionMatrix[kProjectionHorizontalScale];
                else
                    horizontalScaleMagnitude = AsWView(viewBytes)->projectionMatrix[kProjectionHorizontalScale];
                if (!(LessThanF(horizontalScaleMagnitude, kFloat_0_00001_CameraView) & 1u))
                    break;
            }
            {
                *dirtyFlags &= ~kViewProjectionDirty;
                RebuildCameraPerspective(viewBytes, horizontalFovRadians);
            }
        } while (0);
        horizontalSum = AsWView(viewBytes)->projectionRight + AsWView(viewBytes)->projectionLeft;
        horizontalSpan = AsWView(viewBytes)->projectionRight - AsWView(viewBytes)->projectionLeft;
        *appliedChanges |= kAppliedProjectionChanged;
        AsWView(viewBytes)->projectionMatrix[kProjectionCenterX] =
            centerOffsetX / (AsWView(viewBytes)->screenCenterX) + horizontalSum / horizontalSpan;
        AsWView(viewBytes)->projectionMatrix[kProjectionCenterY] =
            (AsWView(viewBytes)->projectionMinY + AsWView(viewBytes)->projectionMaxY) /
                (AsWView(viewBytes)->projectionMinY - AsWView(viewBytes)->projectionMaxY) -
            centerOffsetY / (AsWView(viewBytes)->screenCenterY);
    } else {
        do {
            if (!(*dirtyFlags & kViewProjectionDirty)) {
                unsigned int horizontalScaleBits =
                    (*(unsigned int*)&AsWView(viewBytes)->projectionMatrix[kProjectionHorizontalScale]);
                float horizontalScaleMagnitude;
                if (horizontalScaleBits & kViewFloatSignBit)
                    horizontalScaleMagnitude = -AsWView(viewBytes)->projectionMatrix[kProjectionHorizontalScale];
                else
                    horizontalScaleMagnitude = AsWView(viewBytes)->projectionMatrix[kProjectionHorizontalScale];
                if (!(LessThanF(horizontalScaleMagnitude, kFloat_0_00001_CameraView) & 1u))
                    break;
            }
            {
                *dirtyFlags &= ~kViewProjectionDirty;
                AsWView(viewBytes)->projectionOffsetX = kFloat_0_0_CameraView;
                AsWView(viewBytes)->projectionOffsetY = kFloat_0_0_CameraView;
                RebuildCameraPerspective(viewBytes, horizontalFovRadians);
                projectionRight = AsWView(viewBytes)->projectionRight;
                projectionLeft = AsWView(viewBytes)->projectionLeft;
                projectionMinimumY = AsWView(viewBytes)->projectionMinY;
                frustumHorizontalSum = projectionRight + projectionLeft;
                projectionMaximumY = AsWView(viewBytes)->projectionMaxY;
                frustumHorizontalSpan = projectionRight - projectionLeft;
                verticalSum = projectionMinimumY + projectionMaximumY;
                verticalSpan = projectionMinimumY - projectionMaximumY;
                *appliedChanges |= kAppliedProjectionChanged;
                AsWView(viewBytes)->projectionMatrix[kProjectionCenterX] = frustumHorizontalSum / frustumHorizontalSpan;
                AsWView(viewBytes)->projectionMatrix[kProjectionCenterY] = verticalSum / verticalSpan;
            }
            return;
        } while (0);
        *appliedChanges &= ~kAppliedProjectionChanged;
    }
}
#pragma opt_common_subs reset

// Project a world point; later Y/Z clip failures overwrite earlier axis flags.
// Required input XYZ and writable WViewProjectedPoint output (0x38 bytes).
// Writes screen xy/z/rhw, projected xyz and clipFlags, leaving color/other words
// intact. Camera_Apply may rebuild global-facing cached matrices first. No
// clipping rejection/status: outside points are still projected and flagged.
void WView_ProjectWorldPoint(void* camera, void* output, void* input) {
    Camera_Apply(camera, kFloat_0_0_CameraView, kFloat_0_0_CameraView);
    WView* c = (WView*)camera;
    // Phase 1: compute normalized projected coordinates and viewport coordinates
    // independently; screenZ is viewport Z minus one, rhw is fixed at one.
    // Representation follow-up: Engine_GetViewport/Viewport_Copy writes six
    // words {u32 x,y,width,height; float nearZ,farZ}; this legacy carrier declares
    // only three floats. Keep the existing compiled stack layout;
    // correcting the extent needs separate verification against the original.
    float viewport[3];
    float projectedPosition[3];
    float screenPosition[3];
    float worldPosition[3];
    float viewPosition[3];
    Matrix_TransformPoint(viewPosition, input, (&AsWView(camera)->inputTransform[0]));
    Vec3_CopyToOutput(viewPosition, projectedPosition);
    Mtx44_TransformPointPerspective(projectedPosition, projectedPosition, c->projectionMatrix);
    Vec3_CopyToOutput(input, worldPosition);
    Engine_GetViewport(viewport);
    Vec3_ProjectToViewport(screenPosition, worldPosition, viewport, c->projectionMatrix, c->savedViewMatrix,
                           kIdentityMatrix4x4);
    WViewProjectedPoint* result = (WViewProjectedPoint*)output;
    float component, minimumXY, projectedX;
    projectedX = projectedPosition[0];
    minimumXY = kFloat_neg1_View;
    result->projectedX = projectedX;
    component = projectedPosition[1];
    result->projectedY = component;
    component = projectedPosition[2];
    result->projectedZ = component;
    component = screenPosition[0];
    result->screenX = component;
    component = screenPosition[1];
    result->screenY = component;
    component = screenPosition[2];
    component = minimumXY + component;
    result->rhw = kFloat_1_view;
    result->clipFlags = 0;
    result->screenZ = component;
    // Phase 2: choose ONE priority clip flag: failing Z overrides Y, which
    // overrides X. Passing an axis leaves an earlier failure intact.
    // Example: outside left and above -> Above; also beyond far -> BeyondFar.
    if (projectedX < minimumXY) {
        result->clipFlags = kClipLeft;
    } else if (projectedX > kFloat_1_view) {
        result->clipFlags = kClipRight;
    }
    component = result->projectedY;
    if (component < kFloat_neg1_View) {
        result->clipFlags = kClipBelow;
    } else {
        if (component > kFloat_1_view) {
            result->clipFlags = kClipAbove;
        }
    }
    component = result->projectedZ;
    if (component < kFloat_0_0_CameraView) {
        result->clipFlags = kClipBeforeNear;
    } else {
        if (component > kFloat_1_view) {
            result->clipFlags = kClipBeyondFar;
        }
    }
}

// Project a view-space point to the viewport and optionally compute priority clip flags.
// Required input[3]/output WViewProjectedPoint; same partial-output contract as
// ProjectWorldPoint. skipClipTest!=0 forces clipFlags=0, it does not skip projection.
// Viewport farZ must be nonzero (reciprocal below). Preserve separate origin/depth
// matrix composition and screenZ-1 convention; output is in renderer viewport pixels.
void WView_ProjectViewPoint(void* camera, void* output, void* input, int skipClipTest) {
    float projectedPosition[3];
    struct Viewport {
        unsigned int x, y, width, height;
        float nearZ, farZ;
    } viewport;
    typedef char ViewportRecordSizeCheck[sizeof(Viewport) == kViewportBytes ? 1 : -1];
    float combinedViewportMatrix[16];
    float viewportTransform[16];
    struct Matrix {
        float m[16];
    } depthAndOriginTransform;

    Vec3_CopyToOutput(input, projectedPosition);

    float* out = (float*)output;
    float zero = kFloat_0_0_CameraView;
    Camera_Apply(camera, zero, zero);
    Mtx44_TransformPointPerspective(projectedPosition, projectedPosition, (&AsWView(camera)->projectionMatrix[0]));
    float first = projectedPosition[0];
    // Store the clip-space (perspective-divided) point: out[10..12] == projectedX/Y/Z (WViewProjectedPoint 0x28..0x30).
    out[10] = first;
    out[11] = projectedPosition[1];
    out[12] = projectedPosition[2];
    ((WViewProjectedPoint*)output)->clipFlags = 0;
    if (!skipClipTest) {
        float f1 = first;
        if (f1 < kFloat_neg1_View) {
            ((WViewProjectedPoint*)output)->clipFlags = kClipLeft;
        } else if (f1 > kFloat_1_view) {
            ((WViewProjectedPoint*)output)->clipFlags = kClipRight;
        }
        f1 = out[11];
        if (f1 < kFloat_neg1_View) {
            ((WViewProjectedPoint*)output)->clipFlags = kClipBelow;
        } else if (f1 > kFloat_1_view) {
            ((WViewProjectedPoint*)output)->clipFlags = kClipAbove;
        }
        f1 = out[12];
        if (f1 < kFloat_0_0_CameraView) {
            ((WViewProjectedPoint*)output)->clipFlags = kClipBeforeNear;
        } else if (f1 > kFloat_1_view) {
            ((WViewProjectedPoint*)output)->clipFlags = kClipBeyondFar;
        }
    }
    Engine_GetViewport(&viewport);
    WView::BuildViewportTransform(viewportTransform, &viewport);
    depthAndOriginTransform = *(Matrix*)kViewportOriginDepthTransformTemplate;
    depthAndOriginTransform.m[3] = (float)viewport.x;
    depthAndOriginTransform.m[7] = (float)viewport.y;
    depthAndOriginTransform.m[10] = kFloat_1_view / viewport.farZ;
    PSMTX44Transpose(&depthAndOriginTransform, &depthAndOriginTransform);
    PSMTX44Concat(combinedViewportMatrix, viewportTransform, &depthAndOriginTransform);
    Mtx44_TransformPointPerspective(projectedPosition, projectedPosition, combinedViewportMatrix);
    // Final screen-space point: out[0..3] == screenX/Y/Z and rhw (1.0). Coordinates are viewport
    // pixels; the last two lines re-centre x/y by (screenCenter - viewportSize/2).
    float one, f1h, f2, f3, f4; // f1h = 0.5; f2c/f2d below = viewport width/height
    f4 = projectedPosition[0];
    one = kFloat_1_view;
    out[0] = f4;
    f1h = kFloat_0_5_View;
    f3 = projectedPosition[1];
    out[1] = f3;
    f2 = projectedPosition[2];
    out[2] = f2;
    out[3] = one;
    float f2c = AsWView(camera)->viewportWidth;
    out[0] += AsWView(camera)->screenCenterX - f2c * f1h;
    float f2d = AsWView(camera)->viewportHeight;
    out[1] += AsWView(camera)->screenCenterY - f2d * f1h;
}

// Draw the shared textured rectangle as a two-triangle strip.
// s_resultMenuSharedQuadRect supplies layout-space left/top/right/bottom; vertex
// template supplies the remaining four-float records. No camera/resolution update.
// Preserve encoded format 0xF005, 16-byte stride and strip count 2 together.
void ResultMenu_DrawSharedQuad() {
    struct Rect {
        float left, top, right, bottom;
    };
    struct Vertex {
        float x, y, u, v;
    };
    struct Vertices {
        Vertex v[4];
    } vertices;
    Rect rect = *(Rect*)s_resultMenuSharedQuadRect;
    vertices = *(Vertices*)s_resultMenuSharedQuadVertices;
    vertices.v[0].x = rect.left;
    vertices.v[0].y = rect.top;
    vertices.v[1].x = rect.right;
    vertices.v[1].y = rect.top;
    vertices.v[2].x = rect.left;
    vertices.v[2].y = rect.bottom;
    vertices.v[3].x = rect.right;
    vertices.v[3].y = rect.bottom;
    Engine_SetFlexibleVertexFormat(0xF005);
    DrawPrimitiveUP(0x98, 2, &vertices, 0x10);
}

} // extern "C"

} // namespace ns1

typedef LocalNString_u80065F78 OwnedString80068;

struct Alloc80068 {
    char c;
};

enum {
    kFileReadAccess = 0x80000000u,
    kFileOpenExisting = 3,
    kFileSynchronousOptions = 0x20000000u,
    kFileAsyncOptions = 0x60000000u,
    kFileErrorBias = 0x10000,
    kBiasedInvalidFileHandle = 0xFFFF,
    kInvalidFileSize = 0xFFFFFFFFu,
    kTableLockCount = 7
};

typedef void (*Assign80068)(void*, int, int, const char*, const char*, Alloc80068);
typedef int (*Open80068)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                         unsigned int, unsigned int, unsigned int, unsigned int);

// File functions used by the CFileAsync member functions below.
extern "C" {
extern char __vt__10CFileAsync[];
void FilePath_EnsureDvdPrefix(void*);
int FileIO_OpenFile(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int,
                    unsigned int);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, char*);
int FileIO_GetFileSize(int, int*, int, int);
void File_GetLastError();
void FileIO_CloseFile(int);
int FileIO_ReadFile(void*, void*, int, int, int, int, int, int);
int FileIO_PollAsyncTransferResult(int*, int*, int*);
void __dl__FPv(void*);
}

// Open the normalized DVD path and record the handle. A failed open leaves the synchronous flag unchanged.
int CFileAsync::Open(const char* path, int synchronous) {
    OwnedString80068 str(path);
    FilePath_EnsureDvdPrefix(&str);
    unsigned int options = synchronous ? kFileSynchronousOptions : kFileAsyncOptions;
    unsigned int result = ((Open80068)FileIO_OpenFile)((unsigned int)str.c_str(), kFileReadAccess, 0, 0,
                                                       kFileOpenExisting, options, 0, 0, 0, 0);
    handle = result;
    if ((result + kFileErrorBias) == kBiasedInvalidFileHandle) {
        return 0;
    }
    this->synchronous = synchronous;
    return 1;
}

// Open the DVD path; the handle stays invalid if the open fails.
CFileAsync::CFileAsync(const char* path, int synchronous) {
    vtable = __vt__10CFileAsync;
    handle = -1;
    this->synchronous = 0;
    requestedReadSize = 0;
    do {
        {
            struct {
                Alloc80068 alloc;
                char padding[3];
                Alloc80068 copy;
            } allocator;
            OwnedString80068 str;
            str.flags = 0;
            str.length = 0;
            str.data = 0;
            unsigned int length = strlen(path);
            String_ReserveCapacity(&str, length);
            allocator.copy = allocator.alloc;
            String_ReplaceRange(&str, 0, 0, path, path + length, &allocator.copy.c);
            FilePath_EnsureDvdPrefix(&str);
            unsigned int options = synchronous ? kFileSynchronousOptions : kFileAsyncOptions;
            unsigned int result = ((Open80068)FileIO_OpenFile)((unsigned int)str.c_str(), kFileReadAccess, 0, 0,
                                                               kFileOpenExisting, options, 0, 0, 0, 0);
            handle = result;
            if ((result + kFileErrorBias) == kBiasedInvalidFileHandle)
                break;
            this->synchronous = synchronous;
        }
    } while (0);
}

// Size of the open file in its low 32 bits; 0xFFFFFFFF on failure.
int CFileAsync::GetSize() const {
    struct {
        unsigned int high;
        unsigned int low;
    } size;
    size.low = ((unsigned int (*)(int, void*, int, int))FileIO_GetFileSize)(handle, &size, 0, 0);
    if (size.low == kInvalidFileSize)
        File_GetLastError();
    return size.low;
}

namespace ns1 {

extern "C" {

// Lazily initialize and acquire an indexed mutex when the guard is active.
// Required self, index 0..6 if active!=0. Inactive leaves index uninitialized;
// dtor never reads it on that path. Returns self. Preserve lazy-init/acquire
// order; callers must serialize first initialization of each shared entry.
void* TableLock_ctor(TableLock* self, int index, int active) {
    TableLockEntry* entry;
    self->active = active;
    if (active != 0) {
        int offset = index * sizeof(TableLockEntry);
        self->index = index;
        entry = (TableLockEntry*)((char*)s_tableLockEntries + offset);
        if (entry->initialized == 0) {
            OSInitMutex(&entry->lock);
            entry->initialized = 1;
        }
        OSLockMutex(&entry->lock);
    }
    return self;
}

// Release an active indexed mutex and optionally delete the guard.
// Null self is accepted/returned; shouldDelete>0 releases guard storage after
// unlock. Balanced construction/destruction is required; mutex itself persists.
void* __dt__9TableLockFv(TableLock* self, int shouldDelete) {
    if (self != 0) {
        if (self->active != 0) {
            int index = self->index;
            TableLockEntry* entry = (TableLockEntry*)((char*)s_tableLockEntries + index * sizeof(TableLockEntry));
            OSUnlockMutex(&entry->lock);
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

} // namespace ns1

// Close the file if it is open.
CFileAsync::~CFileAsync() {
    unsigned int value = handle;
    unsigned int check = value + kFileErrorBias;
    vtable = __vt__10CFileAsync;
    if (check != kBiasedInvalidFileHandle) {
        FileIO_CloseFile(value);
        handle = -1;
    }
}

// Close the file if it is open and mark the handle invalid.
void CFileAsync::Close() {
    unsigned int value = handle;
    unsigned int check = value + kFileErrorBias;
    if (check != kBiasedInvalidFileHandle) {
        FileIO_CloseFile(value);
        handle = -1;
    }
}

namespace ns1 {

extern "C" {

// Destroy the view base object and optionally release its allocation.
void* WView_dtor(void* ptr, int deletingFlag) {
    if (ptr != 0) {
        if ((volatile void*)ptr != 0)
            BaseObject_dtor((int)ptr, 0);
        if (deletingFlag > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}

// Default view status hook returns false.
int WView_vfunc_5() {
    return 0;
}

// Initialize this projection unit's mutable two-pi constant.
void WView_InitializeProjectionUnitConstants() {
    s_twoPi_ViewProjection = kFloat_6_2831855;
}

// Mark an indexed table mutex as not yet initialized.
void TableLockEntry_ClearInitialized(int* value) {
    *value = 0;
}

} // extern "C"

} // namespace ns1

// A closed file with no pending read.
CFileAsync::CFileAsync() {
    vtable = &__vt__10CFileAsync;
    handle = -1;
    synchronous = 0;
    requestedReadSize = 0;
}

namespace ns1 {

extern "C" {

// Initialize the file unit's two-pi value and all seven table-lock entries.
void TableLock_InitializeEntriesAndConstants() {
    s_twoPi_FileAsync = kFloat_6_2831855;
    __construct_array(&s_tableLockEntries, (void*)TableLockEntry_ClearInitialized, 0, sizeof(TableLockEntry),
                      kTableLockCount);
}

// Reserved projection hook; the original function performs no work.
void WView_DefaultProjectionHook() {}

// Reserved view hook; the original function performs no work.
void WView_vfunc_7() {}

void WView_ProjectViewPoint(void*, void*, void*, int);

// Project a view-space point while suppressing clip tests.
void WView_ProjectViewPointWithoutClipTest(void* view, void* output, void* input) {
    WView_ProjectViewPoint(view, output, input, 1);
}

// Default render capability query returns true.
int WView_DefaultCapabilityTrueA() {
    return 1;
}

// Default render capability query returns true.
int WView_DefaultCapabilityTrueC() {
    return 1;
}

// Default render capability query returns true.
// Accepting base hook: always returns 1; it reads no sphere and performs no culling.
int WView_DefaultAcceptVisibility() {
    return 1;
}

// Default view capability hook returns false.
int WView_vfunc_10() {
    return 0;
}

// Default view capability hook returns true.
int WView_vfunc_8() {
    return 1;
}

// Return the view's embedded render-state block.
// Borrowed pointer at +0x1A18 lives with self. Layout is the constructor's
// seven-word state/tint block; it is not a newly allocated render context.
char* Camera_GetRenderState(char* self) {
    return (char*)&AsWView(self)->renderState;
}

// Initialize the view utility unit's mutable two-pi constant.
void WView_InitializeUtilityUnitConstants() {
    s_twoPi_WView = kFloat_6_2831855;
}

// Mark both saved view and projection matrices for rebuilding.
void WView_InvalidateViewAndProjection(unsigned int* self) {
    AsWView(self)->dirtyFlags |= kViewAndProjectionDirty;
}

int FileIO_ReadFile(void*, void*, int, int, int, int, int, int);

} // extern "C"

} // namespace ns1

// Start a read into a caller buffer that must stay valid until PollRead reports completion.
// Returns the FileIO_ReadFile status.
int CFileAsync::StartRead(void* destination, int byteCount) {
    requestedReadSize = byteCount;
    return FileIO_ReadFile((void*)handle, destination, byteCount, 0, 0, -1, 0, 0);
}

// Synchronous files complete at once; otherwise poll the pending read.
int CFileAsync::PollRead(int* out, int* value, int unusedWord) {
    if (synchronous != 0) {
        *out = 1;
        if (value != 0)
            *value = requestedReadSize;
        return 1;
    }
    return FileIO_PollAsyncTransferResult((int*)handle, out, value);
}

extern "C" {
void WView_InitializeConstructionUnitConstants();
void WView_InitializeProjectionUnitConstants();
void TableLock_InitializeEntriesAndConstants();
void WView_InitializeUtilityUnitConstants();
}
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FE0", _ctors_u80065F78_0, WView_InitializeConstructionUnitConstants);
STATIC_INIT_ENTRY(".CRT$XCU80461FE0", _ctors_u80065F78_1, WView_InitializeProjectionUnitConstants);
STATIC_INIT_ENTRY(".CRT$XCU80461FE0", _ctors_u80065F78_2, TableLock_InitializeEntriesAndConstants);
STATIC_INIT_ENTRY(".CRT$XCU80461FE0", _ctors_u80065F78_3, WView_InitializeUtilityUnitConstants);
#pragma pop
