// Golf cup preview: CGolfCup renders the cup model into a 128x128 texture for the hole preview,
// placing a capture camera at the cup and swapping the red and blue channels of the captured pixels.
// It also builds the scaled cup mesh and holds the buffers and render-state guards it uses.

#include "game/types.h"
#include "game/wvector.h"
#include "game/exception.h"
#include "classes/CGolfCup.h"
#include "classes/GolfCupFields.h"
#include "classes/GtlPrimitive.h"

#pragma use_lmw_stmw on
extern "C" {
extern int g_golfCupPreview;
void BaseObject_dtor(void*, int);
void __dl__FPv(void*);
void Engine_SetCachedRenderState(void*, unsigned int);

// Return the capture-texture reference subobject.
void* GolfCup_GetCaptureTextureReference(char* self) {
    return &((GolfCupFields*)self)->captureTexture;
}

// Clear the global preview registration, destroy the base, and optionally free storage.
void* __dt__11GolfCupBaseFv(void* self, int deleting) {
    if (self) {
        g_golfCupPreview = 0;
        BaseObject_dtor(self, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct Entry {
    void* data;
    unsigned int newValue;
};
struct Object {
    Entry entries[5];
    unsigned int savedValues[5];
};
// Restore the five saved render-state values and optionally free the guard.
void* RenderStateGuard_RestoreAndFree(Object* self, int deleting) {
    if (self) {
        unsigned int* savedValuesCursor = (unsigned int*)self;
        Entry* entry = self->entries;
        for (unsigned int i = 0; i < 5; ++i, ++savedValuesCursor, ++entry) {
            Engine_SetCachedRenderState(entry->data, savedValuesCursor[10]);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

extern "C" {
enum CupPreviewLimits {
    kMaterialResourceKind = 3,
    kHardwareLightCount = 8,
    kCaptureTextureSize = 128
};
enum CupPixelLayout {
    kCaptureRgbaBytes = 4,
    kCaptureChannelFull = 255,
    kCapturePlatformLayout = 7
};
enum CupMaterialStorageLimit {
    kMaximumCupMaterialSlots = 0x3fffffff
};
enum CupMeshLayout {
    kCupVertexCount = 30,
    kCupTriangleCount = 10,
    kCupPositionNormalFVF = 0x401,
    kCupTrianglePrimitive = 0x90
};
enum CupStringOffsets {
    kCupVectorLengthError = 10,
    kCupLengthErrorType = 30
};

void CubeResourceStorage_dtor(Ref*, int);
// Release the capture texture reference and optionally free its wrapper.
void* __dt__15GolfCupMatricesFv(RefPtr* self, int deleting) {
    if (self) {
        Ref* p = self->ptr;
        if (p) {
            if (--p->count == 0)
                CubeResourceStorage_dtor(p, 1);
            self->ptr = 0;
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
}

extern "C" {
// Cup scale 0.7; camera height 0.6 and Z offset 0.0001; near/far planes 0.1/100000.
extern const float kFloat_0_7_GolfCupModelScale, kFloat_0_6_GolfCupCameraHeight, kFloat_0_0001_GolfCupCameraZOffset,
    kFloat_1_92_Exhibition, kFloat_0_1_GolfCupNearPlane, kFloat_100000_GolfCupFarPlane;
typedef Vec3f Vec;
struct Mat {
    float v[9];
    Vec pos;
};
// Borrow access to the scene-owned capture queue.
static inline void*& CupCaptureQueue(CGolfCup* scene) {
    return scene->captureQueue;
}
// Borrow access to the retained shadow mesh; this does not change its reference count.
static inline void*& CupShadowMesh(CGolfCup* scene) {
    return scene->shadowMesh;
}
// Borrow the texture-reference wrapper retained from the queue's first item.
static inline RefPtr& CupCaptureTexture(CGolfCup* scene) {
    return scene->captureTexture;
}
// Ownership: model is released through ResourceManager; captureQueue is an
// owned TextureCaptureQueue; shadowMesh is retained; captureTexture retains the
// capture texture. buffer is an owning Ref* array, size/capacity in entries,
// temporarily retaining removed model materials during CaptureModel.
// active means capture is complete: SetPosition clears it, CaptureModel sets it.
// camera maps world -> capture view; projection is the orthographic capture.
// Byte-based capacity loads preserve the compiler's alias/reload behavior during vector growth.
enum CupFieldOffset {
    kMaterialCapacityOffset = (int)&((CGolfCup*)0)->capacity
};
void Matrix_SetIdentityAffine(Mat*);
void Matrix_CopyScaledBasis(Mat*, const Mat*, float);
void Matrix_CopyAffine(Mat*, const Mat*);
void __as__7WVectorFRC7WVector(Vec*, const Vec*);
void Model_SetMatrixRecursive(void*, int, Mat*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(Vec*, void*, Vec*, int, int*, int);
extern Vec kUnitYAxis;
void MTXLookAt(void*, Vec*, const Vec*, Vec*);
void Mtx44_SetOrthoCentered(void*, float, float, float, float);
// Place the cup model and aim the orthographic capture camera at it.
void CupPreview_SetPosition(CGolfCup* self, const Vec* pos) {
    // pos is a borrowed world-space point, copied into self. Y is vertical.
    // Model starts at scale 0.7; eye is just above pos, with a small Z offset
    // so look-at does not exactly align with its Y-up vector. Ground hit is queried
    // but not used to replace pos. CaptureModel later restores an unscaled basis.
    Mat matrix;
    Mat scaled;
    Vec result;
    Vec eye;
    Vec input;
    int hit;
    Matrix_SetIdentityAffine(&matrix);
    Matrix_CopyScaledBasis(&scaled, &matrix, kFloat_0_7_GolfCupModelScale);
    Matrix_CopyAffine(&matrix, &scaled);
    *(WVector*)(&matrix.pos) = *(WVector*)pos;
    Model_SetMatrixRecursive(self->model, 0, &matrix);
    struct Words {
        unsigned int x, y, z;
    };
    *(Words*)&input = *(const Words*)pos;
    GetGroundPoint__9CQuadTreeF7WVectorbPib(&result, CQuadTree_GetInstance(), &input, 1, &hit, 0);
    eye.x = pos->x;
    eye.y = kFloat_0_6_GolfCupCameraHeight + pos->y;
    eye.z = kFloat_0_0001_GolfCupCameraZOffset + pos->z;
    MTXLookAt(self->camera, &eye, pos, &kUnitYAxis);
    Mtx44_SetOrthoCentered(self->projection, kFloat_1_92_Exhibition, kFloat_1_92_Exhibition,
                           kFloat_0_1_GolfCupNearPlane, kFloat_100000_GolfCupFarPlane);
    *(WVector*)(&self->pos) = *(WVector*)pos;
    self->active = 0;
}
}

extern "C" {
extern void* g_resourceManager;
extern char __vt__8CGolfCup[];
void ResourceManager_ReleaseObject(void*, void*, int);
void TextureCaptureQueue_Clear(void*);
void TextureCaptureQueue_Dtor(void*, int);
void appFreeUni__FPv(void*);
struct SharedBase {
    unsigned int count;
};
struct Shared : SharedBase {
    virtual void destroy(int);
};
struct Buffer {
    void* ptr;
    unsigned int size;
};
// Subtract the old logical count using the target allocator's reference ABI.
inline void subtractSize(unsigned int& a, const int& b) {
    a -= b;
}
// Clear logical size and free the owned pointer array, without freeing elements.
inline void destroyBuffer1(Buffer* p) {
    if (p) {
        if (p->ptr) {
            subtractSize(p->size, p->size);
            appFreeUni__FPv(p->ptr);
        }
    }
}
// Preserve the nested allocator-destructor boundary around pointer-array cleanup.
inline void destroyBuffer2(Buffer* p) {
    if (p)
        destroyBuffer1(p);
}
// Forward cleanup through the next allocator wrapper.
inline void destroyBuffer3(Buffer* p) {
    if (p)
        destroyBuffer2(p);
}
// Forward cleanup through the outer buffer wrapper.
inline void destroyBuffer4(Buffer* p) {
    if (p)
        destroyBuffer3(p);
}
// Enter the complete buffer-wrapper destructor chain.
inline void destroyBuffer5(Buffer* p) {
    if (p)
        destroyBuffer4(p);
}
// Drop the retained texture reference, destroying the resource on its last owner.
inline void destroyResource(RefPtr* self) {
    if (self) {
        Ref* p = self->ptr;
        if (p) {
            if (--p->count == 0)
                CubeResourceStorage_dtor(p, 1);
            self->ptr = 0;
        }
    }
}
// Unregister the preview and destroy its base without deleting outer storage.
inline void destroyBase(void* self) {
    if (self) {
        g_golfCupPreview = 0;
        BaseObject_dtor(self, 0);
    }
}
} // extern "C"

// Release the cup model, capture queue, mesh, material array, and texture
// reference, then unregister the preview.
CGolfCup::~CGolfCup() {
    // The shadow-mesh reference must be non-null: its count is read before the null test.
    vtbl = __vt__8CGolfCup;
    if (model)
        ResourceManager_ReleaseObject(g_resourceManager, model, 0);
    TextureCaptureQueue_Clear(CupCaptureQueue(this));
    if (CupCaptureQueue(this)) {
        TextureCaptureQueue_Dtor(CupCaptureQueue(this), 1);
        CupCaptureQueue(this) = 0;
    }
    Shared* p = (Shared*)CupShadowMesh(this);
    if (--p->count == 0) {
        if (p)
            p->destroy(1);
    }
    CupShadowMesh(this) = 0;
    destroyBuffer5((Buffer*)&buffer);
    destroyResource(&captureTexture); // Direct member address preserves destructor inlining.
    destroyBase(this);
}

extern "C" {
void WPuppet_GetModelResource(void*, int, int, Ref**);
void WPuppet_SetModelResource(void*, int, int, Ref*);
void Engine_GetLightEnabled(int, int*);
void Engine_SetLightEnabled(int, int);
void TextureCaptureQueue_BeginDraw(void*, void*);
void Render_ClearLoadingFramebuffer(void*);
void Engine_SetRenderStateList(void*, int);
void SetTransform(int, void*);
void gtl_GenericRenderer_SetViewMatrix(void*);
void gtl_GenericRenderer_SetProjectionMatrix(void*);
void* WPuppet_GetSkinMatrices(void*);
void TextureCaptureQueue_DrawNextCapture(void*);
void TextureCaptureQueue_EndDraw(void*);
void TextureCapture_SwapRedBlueAndMaskAlpha(CGolfCup*);
extern const float kFloat_1_GolfCupModelRenderArg;
struct RenderStates {
    int words[22];
};
struct ConstantWords12 {
    unsigned int a, b, c, d, e, f, g, h, i, j, k, l;
};
struct ConstantWords10 {
    unsigned int a, b, c, d, e, f, g, h, i, j;
};
struct RenderConstants {
    RenderStates first, second;
    ConstantWords12 vertices;
    ConstantWords10 indices;
};
extern RenderConstants golfCupRenderDefaults; // Capture/model render-state blocks and texture-stage settings.
extern char kIdentityMatrix4x4[];
struct RenderModel {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();

    virtual void render(void*, int, int, int, int, void*, int, float);
};
// Capture the cup without its material resources, then restore resources and lights.
void CupPreview_CaptureModel(CGolfCup* self, void* renderContext) {
    Mat matrix;
    // Requires initialized resources, material scratch sized for every model
    // buffer, and an invalidated capture (active == 0). Returns early otherwise.
    int* savedPtr;
    unsigned int zero;
    int stage;
    int* restorePtr;
    int offset2;
    int j;
    if (!self->model || 0 == CupCaptureQueue(self))
        return;
    if (self->active)
        return;

    // Phase 1: retain/detach materials and save/disable all hardware lights.
    stage = 0;
    int i = 0;
    zero = i;
    int count = (int)self->size;
    for (; count > i; ++i, stage += 4) {
        *(Ref**)((char*)self->buffer + stage) = (Ref*)zero;
        WPuppet_GetModelResource(self->model, kMaterialResourceKind, i, (Ref**)((char*)self->buffer + stage));
        WPuppet_SetModelResource(self->model, kMaterialResourceKind, i, 0);
    }
    int saved[kHardwareLightCount];
    savedPtr = saved;
    for (stage = 0; kHardwareLightCount > stage; ++stage, ++savedPtr) {
        Engine_GetLightEnabled(stage, savedPtr);
        Engine_SetLightEnabled(stage, 0);
    }
    // Phase 2: render with capture view/projection and the capture-state template.
    TextureCaptureQueue_BeginDraw(CupCaptureQueue(self), renderContext);
    Render_ClearLoadingFramebuffer(CupCaptureQueue(self));
    RenderStates states = golfCupRenderDefaults.first;
    Engine_SetRenderStateList(&states, 11);
    SetTransform(0, self->camera);
    gtl_GenericRenderer_SetViewMatrix(self->camera);
    SetTransform(1, self->projection);
    gtl_GenericRenderer_SetProjectionMatrix(self->projection);
    SetTransform(11, kIdentityMatrix4x4);
    void* data = WPuppet_GetSkinMatrices(self->model);
    ((RenderModel*)self->model)->render(renderContext, 1, 1, 1, 0, data, 0, kFloat_1_GolfCupModelRenderArg);
    TextureCaptureQueue_DrawNextCapture(CupCaptureQueue(self));
    TextureCaptureQueue_EndDraw(CupCaptureQueue(self));
    // Phase 3: restore materials/lights; relinquish the scratch references.
    zero = 0;
    // Byte offset into saved Ref* slots; renderContext retains its view meaning.
    unsigned int materialByteOffset;
    for (j = 0, materialByteOffset = 0; j < count; ++j, materialByteOffset += sizeof(Ref*)) {
        WPuppet_SetModelResource(self->model, kMaterialResourceKind, j,
                                 *(Ref**)((char*)self->buffer + materialByteOffset));
        Ref* p = *(Ref**)((char*)self->buffer + materialByteOffset);
        if (p) {
            if (--p->count == 0)
                CubeResourceStorage_dtor(p, 1);
            *(Ref**)((char*)self->buffer + materialByteOffset) = (Ref*)zero;
        }
    }
    restorePtr = saved;
    for (stage = 0; stage < kHardwareLightCount; ++stage, ++restorePtr)
        Engine_SetLightEnabled(stage, *restorePtr);
    // Phase 4: convert captured pixels, reset the model basis and mark it cached.
    TextureCapture_SwapRedBlueAndMaskAlpha(self);

    Matrix_SetIdentityAffine(&matrix);
    *(WVector*)(&matrix.pos) = *(WVector*)(&self->pos);
    Model_SetMatrixRecursive(self->model, 0, &matrix);
    self->active = 1;
}
}

extern "C" {
// Shadow material value 0.5; vertical offsets 1.5 and 0.18.
extern const float kFloat_0_5_GolfCupShadowMaterial, kFloat_1_5_GolfCupShadowDepth, kFloat_0_18_GolfCupShadowOffset,
    kFloat_0_GolfCup;
struct Matrix17 {
    float v[17];
};
struct TenWords {
    unsigned int a, b, c, d, e, f, g, h, i, j;
};
struct TwelveWords {
    unsigned int a, b, c, d, e, f, g, h, i, j, k, l;
};
extern Matrix17 golfCupShadowMaterialTemplate; // Material template used to draw the ground shadow.
extern TenWords golfCupShadowRenderStates;     // Five render-state ID/value pairs guarded during shadow drawing.
void Memory_Copy(void*, const void*, unsigned int);
void Engine_GetRenderState(void*, unsigned int*);
void Mtx44_SetTranslation(void*, float, float, float);
void Engine_SetMaterial(void*);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
extern void* g_view;
void Camera_Apply(void*, float, float);
void* Camera_GetRenderState(void*);
void WView_InvalidateViewAndProjection(void*);
}
struct SavedStates {
    Entry entries[5];
    unsigned int savedValues[5];
    // Save five render-state values and install the template's replacements.
    SavedStates(const TenWords& source) {
        TenWords initial = source;
        Memory_Copy(this, &initial, 0x28);
        Entry* entry = entries;
        unsigned int* savedValueCursor = savedValues;
        for (unsigned int i = 0; i < 5; ++i, ++entry, ++savedValueCursor) {
            Engine_GetRenderState(entry->data, savedValueCursor);
            Engine_SetCachedRenderState(entry->data, entry->newValue);
        }
    }
    // Restore all five values when the shadow-render scope ends.
    ~SavedStates() {
        unsigned int* savedValueCursor = (unsigned int*)this;
        Entry* entry = entries;
        for (unsigned int i = 0; i < 5; ++i, ++savedValueCursor, ++entry)
            Engine_SetCachedRenderState(entry->data, savedValueCursor[10]);
    }
};
// Draw the cup shadow and model while saving and restoring the affected render states.
extern "C" void CupPreview_DrawModel(CGolfCup* self, void* arg) {
    // Shadow translation uses world-space pos with a vertical subtraction.
    // The saved-state scope ends before the model's render templates are applied.
    RenderConstants* base = &golfCupRenderDefaults;
    if (self->model) {
        RenderStates states;
        float half = kFloat_0_5_GolfCupShadowMaterial;
        Matrix17 matrix = golfCupShadowMaterialTemplate;

        ConstantWords12 vertices;
        ConstantWords10 indices;
        float transform[16];
        matrix.v[3] = half;
        matrix.v[7] = half;
        matrix.v[11] = half;
        matrix.v[15] = half;
        {
            SavedStates saved(golfCupShadowRenderStates);

            Mtx44_SetTranslation(transform, self->pos.x,
                                 (self->pos.y - kFloat_1_5_GolfCupShadowDepth) - kFloat_0_18_GolfCupShadowOffset,
                                 self->pos.z);
            Engine_SetMaterial(&matrix);
            SetTransform(11, transform);
            ((gtl::IG3DXPrimitive*)(CupShadowMesh(self)))->Draw(0);
        }
        // Restore the main view/projection for the model, then invalidate the
        // WView cache because these transforms were installed through raw APIs.
        states = base->second;
        vertices = base->vertices;
        indices = base->indices;
        Engine_SetRenderStateList(&states, 11);
        Engine_SetTextureStageStateList(0, &vertices, 6);
        Engine_SetTextureStageParameterList(0, &indices, 5);
        Camera_Apply(g_view, kFloat_0_GolfCup, kFloat_0_GolfCup);
        char* camera = (char*)Camera_GetRenderState(arg);
        SetTransform(1, camera + 0x60);
        gtl_GenericRenderer_SetProjectionMatrix(camera + 0x60);
        SetTransform(0, camera + 0x20);
        gtl_GenericRenderer_SetViewMatrix(camera + 0x20);
        void* data = WPuppet_GetSkinMatrices(self->model);
        ((RenderModel*)self->model)->render(arg, 1, 1, 1, 0, data, 0, kFloat_1_GolfCupModelRenderArg);
        WView_InvalidateViewAndProjection(arg);
    }
}

extern "C" {
// Only five normal vectors are copied into the local aggregate. The loop below
// still indexes i/5 for 30 vertices: vertices 25..29 read index 5, outside this
// declared copy. In the ORIGINAL function the five Vecs occupy SP+0x98..
// SP+0xd3; positions begin at SP+0xd8. Group 5 reads SP+0xd4 (an unwritten
// gap word), then the first scaled position x/y. This is a stack overread in
// the original too, not evidence for a six-entry normal table. The current
// compiler can lay out that overread differently: keep it out of geometry edits
// until the owner chooses an explicit behavior fix. Extending/clamping changes code.
struct FiveVecs {
    Vec v[5];
};
enum CupNormalLookup {
    kCupVerticesPerNormal = 5,
    kCopiedCupNormalCount = 5,
    kRequiredCupNormalCount = (kCupVertexCount + kCupVerticesPerNormal - 1) / kCupVerticesPerNormal
};
// Modification contract: 30 vertices require SIX lookup groups, but only
// FIVE normals are copied. This code preserves the last-group stack read.
// Treat position count, normal stride, source extent, and local copy extent as
// one contract; do not use kCupVertexCount alone to add geometry.
struct ThirtyVecs {
    Vec v[30];
};
struct MeshVertex {
    Vec pos;
    Vec normal;
};
extern FiveVecs golfCupMeshNormals;     // Cup-mesh normal vectors; preserve the original table indexing.
extern ThirtyVecs golfCupMeshPositions; // Thirty unscaled cup-mesh vertex positions.
void Vec3_MultiplyComponents(Vec*, const Vec*, const Vec*);
#pragma use_lmw_stmw off
// Build the scaled cup mesh and optionally reverse its triangle winding.
int GolfCupMesh_CreateScaledTriangles(void*, void** out, int, int reverse, float x, float y, float z) {
    // x/y/z scale model-space positions componentwise; normals are copied
    // unchanged. reverse swaps vertices 1/2 per triangle, not the normal signs.
    // Always returns 1 and dereferences *out: creation success is a precondition.
    gtl::IG3DXPrimitive::CreateFromFVF((void*)kCupTriangleCount, (void*)kCupVertexCount, (void*)0,
                                       (void*)kCupPositionNormalFVF, (void*)kCupTrianglePrimitive, out);
    FiveVecs normals;
    normals = golfCupMeshNormals;
    void* mesh = *out;
    ThirtyVecs positions = golfCupMeshPositions;
    Vec scale;
    scale.x = x;
    scale.y = y;
    scale.z = z;
    Vec* p = positions.v;
    // Phase 1: scale the template positions, then emit position/normal records.
    for (unsigned int i = 0; i < kCupVertexCount; ++i, ++p)
        Vec3_MultiplyComponents(p, p, &scale);
    MeshVertex* vertices;
    ((gtl::IG3DXPrimitive*)mesh)->GetVertexData(0, &vertices);
    for (unsigned int i = 0; i < kCupVertexCount; ++i) {
        MeshVertex* dst = &vertices[i];
        dst->pos.x = positions.v[i].x;
        dst->pos.y = positions.v[i].y;
        dst->pos.z = positions.v[i].z;
        // Preserve the original i/5 lookup. The local FiveVecs declaration
        // covers only five entries although i/5 reaches five: extent unresolved.
        dst->normal = normals.v[i / kCupVerticesPerNormal];
    }
    // Phase 2: optionally reverse winding, then publish sequential indices.
    if (reverse) {
        MeshVertex* v = vertices;
        for (int i = 0; i < kCupTriangleCount; ++i) {
            MeshVertex* a = v + 1;
            MeshVertex* b = v + 2;
            MeshVertex temp = *a;
            *a = *b;
            *b = temp;
            v += 3;
        }
    }
    ((gtl::IG3DXPrimitive*)mesh)->FlushVertexBuffer();
    unsigned short* indices;
    ((gtl::IG3DXPrimitive*)mesh)->GetIndexData(0, &indices);
    for (unsigned int i = 0; i < kCupVertexCount; ++i)
        indices[i] = i;
    ((gtl::IG3DXPrimitive*)mesh)->FlushIndexBuffer();
    return 1;
}
}

#pragma use_lmw_stmw on
extern "C" {
extern char golfCupMeshStrings[], __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[],
    __vt__Q23std12length_error[];
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
unsigned int WPuppet_GetModelBufferCount(void*, int);
unsigned long strlen(const char*);
char* strcpy(char*, const char*);
void* memset(void*, int, unsigned long);
void* memcpy(void*, const void*, unsigned long);
void* __nwa__FUl(unsigned long);
void* appMallocDummy__FUl(unsigned long);
void __dt__Q33std6detail16char_array_guardFv(void*, int);
void __dla__FPv(void*);
void dtor_80046988(void*, int);
void __dt__Q23std12length_errorFv();
void __throw(const char*, void*, void (*)()) __attribute__((noreturn));
void TextureCaptureQueue_Init(void*);
void TextureCaptureQueue_SetSize(void*, int, int);
void TextureCaptureQueue_AddTexture(void*);
void* TextureItemQueue_GetListHeader(void*);
}
struct StringOwner {
    void* vtable;
    unsigned int one, two;
    char* data;
    // Adopt exception text into a newly allocated shared-deleter record.
    StringOwner(char* p) {
        one = 1;
        two = 1;
        vtable = s_sharedCharDeleterVtable;
        data = p;
    }
};
struct StringGuard {
    char* data;
    char* flag;
    // Hold allocated exception text until its shared owner is constructed.
    StringGuard(char* p, char* f) : data(p), flag(f) {}
    // Free text that was not handed to the shared owner.
    ~StringGuard() {
        if (data)
            __dla__FPv(data);
    }
};
struct ErrorBase_u80265514 : std::exception {
    // Install the target exception vtable without adding data members.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* data;
    StringOwner* owner;
    // Copy the error message and create its shared-deleter record.
    ErrorBase_u80265514(const char* s, char* flag) {
        const char* source = s;
        SetTable(__vt__Q23std11logic_error);
        unsigned long n = strlen(s) + 1;
        data = (char*)__nwa__FUl((n << 1) - n);
        char* allocated = data;
        {
            StringGuard guard(allocated, flag);
            owner = new StringOwner(allocated);
            guard.data = 0;
        }
        strcpy(data, source);
    }
};
struct ErrorString : ErrorBase_u80265514 {
    // Complete the vector length-error type after constructing its message.
    ErrorString(const char* s, char* f) : ErrorBase_u80265514(s, f) {
        SetTable(__vt__Q23std12length_error);
    }
};
// Throw a vector length error with the retained target exception representation.
inline void throwLength(const char* message) {
    char flag = 0;
    ErrorString err(message, &flag);
    __throw(golfCupMeshStrings + kCupLengthErrorType, &err, __dt__Q23std12length_errorFv);
}
struct GrowBuffer_u80265514 {
    Ref** data;
    volatile unsigned int size;
    unsigned int capacity;
    unsigned int* cap;
    unsigned int offset;
    // Start an empty owner for a replacement material-pointer allocation.
    GrowBuffer_u80265514(unsigned int* c) : data(0), size(0), capacity(0), cap(c), offset(0) {}
    // Free the allocation still owned after growth or the swap with the cup.
    ~GrowBuffer_u80265514() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
struct EffectInit {
    char bytes[0xa4];
    // Construct the capture queue in its newly allocated storage.
    EffectInit() {
        TextureCaptureQueue_Init(this);
    }
};
// Borrow the larger capacity-growth bound.
inline const unsigned int& larger(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Grow a Ref* array within the 0x3fffffff-element allocation limit.
inline unsigned int grown(unsigned int old, const unsigned int& need) {
    if (old < 0x15555555) {
        unsigned int growth = ((old + 1) * 3) / 5;
        return old + larger(growth, need);
    }
    if (old < 0x2aaaaaaa) {
        unsigned int growth = (old + 1) / 2;
        return old + larger(growth, need);
    }
    return kMaximumCupMaterialSlots;
}
struct TargetErrorBase_u80265514 : std::exception {
    // Install the target exception vtable in the existing base layout.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    // Initialize the logic-error base before building its message.
    TargetErrorBase_u80265514() {
        SetTable(__vt__Q23std11logic_error);
    }
};
struct TargetError_u80265514 : TargetErrorBase_u80265514 {
    char* data;
    StringOwner* owner;
    // Own a copy of the capacity-error message and install the length-error type.
    TargetError_u80265514(const char* s, char* flag) {
        const char* source = s;
        unsigned long n = strlen(source) + 1;
        data = (char*)__nwa__FUl((n << 1) - n);
        char* allocated = data;
        {
            StringGuard guard(allocated, flag);
            owner = new StringOwner(allocated);
            guard.data = 0;
        }
        strcpy(data, source);
        SetTable(__vt__Q23std12length_error);
    }
};
// Prepare the cup preview resources.
extern "C" int CupPreview_PollResources(CGolfCup* self) {
    // Return 0 only when the model request has not produced a resource. Return 1
    // means model and capture queue are available; it does not perform the capture.
    // Phase 1: load the model and size its temporary material-reference array.
    if (!self->model) {
        self->model = ResourceManager_LoadByExtension(g_resourceManager, golfCupMeshStrings, 0, 1, 0);
        if (0 == self->model)
            return 0;
        unsigned int count = WPuppet_GetModelBufferCount(self->model, kMaterialResourceKind);
        if (count > self->size) {
            unsigned int slotsToAppend = count - self->size;
            // Original 0x80265674: both resize paths join after buffer replacement.
            do {
                if (slotsToAppend > *(unsigned int*)((char*)self + kMaterialCapacityOffset) ||
                    self->size > *(unsigned int*)((char*)self + kMaterialCapacityOffset) - slotsToAppend) {
                    unsigned int currentSlotCount = *(volatile unsigned int*)&self->size;
                    unsigned int allocationCapacity = (*(volatile unsigned int*)&self->capacity);
                    if (currentSlotCount + slotsToAppend - *(unsigned int*)((char*)self + kMaterialCapacityOffset) >
                        kMaximumCupMaterialSlots - allocationCapacity) {
                        union {
                            char c;
                            int align;
                        } flag;
                        flag.c = 0;
                        TargetError_u80265514 err(kCupVectorLengthError + golfCupMeshStrings, &flag.c);
                        __throw(golfCupMeshStrings + kCupLengthErrorType, &err, __dt__Q23std12length_errorFv);
                    }
                    grown(allocationCapacity, (currentSlotCount + slotsToAppend) - *(unsigned int*)&self->capacity);
                } else {
                    memset((Ref**)self->buffer + self->size, 0, ((4 * slotsToAppend)));
                    self->size = slotsToAppend + self->size;
                    break;
                }
                {
                    // Growth transaction: allocate, zero new slots, transfer old pointer bits.
                    // pendingMaterials owns the replacement until the final swap;
                    // afterwards it frees the old allocation, never the resources.
                    unsigned int requiredCapacityGrowth = *(volatile unsigned int*)&self->size + slotsToAppend -
                                                          *(unsigned int*)((char*)self + kMaterialCapacityOffset);
                    GrowBuffer_u80265514 pendingMaterials(&self->capacity);
                    unsigned int allocationCapacity = *(unsigned int*)((char*)self + kMaterialCapacityOffset);
                    if (requiredCapacityGrowth > kMaximumCupMaterialSlots - allocationCapacity) {
                        union {
                            char c;
                            int align;
                        } flag;
                        flag.c = 0;
                        TargetError_u80265514 err((kCupVectorLengthError + golfCupMeshStrings), &flag.c);
                        __throw(golfCupMeshStrings + kCupLengthErrorType, &err, __dt__Q23std12length_errorFv);
                    }
                    allocationCapacity = grown(allocationCapacity, requiredCapacityGrowth);
                    if (allocationCapacity > kMaximumCupMaterialSlots) {
                        union {
                            char c;
                            unsigned int align;
                        } flag;
                        flag.c = 0;
                        TargetError_u80265514 err(kCupVectorLengthError + golfCupMeshStrings, &flag.c);
                        __throw(kCupLengthErrorType + golfCupMeshStrings, &err, __dt__Q23std12length_errorFv);
                    }
                    pendingMaterials.data = (Ref**)appMallocDummy__FUl((4 * allocationCapacity));
                    pendingMaterials.capacity = allocationCapacity;
                    __typeof__(self->size) oldSlotCount = self->size;
                    pendingMaterials.offset = self->size;
                    memset(pendingMaterials.size + ((pendingMaterials.data + oldSlotCount)), 0, 4 * slotsToAppend);

                    Ref** oldMaterials = (Ref**)self->buffer;
                    unsigned int prefixSlotCount = ((Ref**)self->buffer + self->size) - oldMaterials;
                    __typeof__(slotsToAppend + pendingMaterials.size) constructedTailCount =
                        pendingMaterials.size + slotsToAppend;
                    pendingMaterials.offset -= prefixSlotCount;
                    pendingMaterials.size = constructedTailCount;
                    memcpy((((pendingMaterials.data + pendingMaterials.offset))), oldMaterials,
                           ((4 * prefixSlotCount)));
                    memset(oldMaterials, 0, ((prefixSlotCount * 4)));
                    unsigned int replacementSlotCount = (pendingMaterials.size + prefixSlotCount);
                    pendingMaterials.size = replacementSlotCount;
                    void* previousBuffer = self->buffer;
                    Ref** previous = (Ref**)previousBuffer;
                    Ref** replacement = pendingMaterials.data;
                    __typeof__(self->capacity) savedCapacity = self->capacity;
                    self->size = 0;

                    pendingMaterials.data = previous;
                    self->capacity = pendingMaterials.capacity;
                    unsigned int previousCap = savedCapacity;
                    pendingMaterials.capacity = previousCap;
                    self->size = replacementSlotCount;
                    self->buffer = replacement;
                    pendingMaterials.size = 0;
                }
            } while (0);
            // Shrinking changes logical size only; no pointees are released here.
        } else if (count < self->size) {
            self->size -= self->size - count;
        }
    }
    // Phase 2: build the 128x128 queue once and retain its first texture.
    if (CupCaptureQueue(self) == 0) {
        CupCaptureQueue(self) = new EffectInit;
        TextureCaptureQueue_SetSize(CupCaptureQueue(self), kCaptureTextureSize, kCaptureTextureSize);
        TextureCaptureQueue_AddTexture(CupCaptureQueue(self));
        char* effect = (char*)TextureItemQueue_GetListHeader(CupCaptureQueue(self));
        Ref* prior = CupCaptureTexture(self).ptr;
        if (prior) {
            if (--prior->count == 0)
                CubeResourceStorage_dtor(prior, 1);
            CupCaptureTexture(self).ptr = 0;
        }
        Ref* p = *(Ref**)(*(char**)(effect + 8) + 8);
        CupCaptureTexture(self).ptr = p;
        p->count++;
    }
    return 1;
}

extern "C" {
// Texture level-description words, used by the allocation and format-lookup calls.
enum TextureDescriptionWord {
    kTexturePlatformFormatWord = 0,
    kTextureWidthWord = 7,
    kTextureHeightWord = 8
};
struct TextureInfo {
    unsigned int words[9];
};
struct TextureLock {
    unsigned int unused;
    unsigned char* data;
};
void Texture_GetLevelDescription(void*, int, TextureInfo*);
void* gtl_ImageFormat_FromPlatformFormat(unsigned int, int);
void* gtl_Image_GetRowPitch(unsigned int, void*, int);
void Texture_LockLevel(void*, int, TextureLock*, int, int);
int TextureFormat_IsPlatformCMPR_C8OrRGBA8(unsigned int);
unsigned int Texture_CalculateMipStorageSize(unsigned int, unsigned int, int, unsigned int);
void gtl_Image_CopyFromPlatform7Layout(void*, unsigned int, unsigned int, int, void*, void*, int, unsigned int);
void gtl_ImageFormat_ToSwizzled(void*);
void Texture_ConvertRGBA8ToTiledARGBPlanes(void*, void*, int, void*, int, int, int, int);
void Texture_UnlockLevel(void*, int);
}
struct ByteBuffer {
    unsigned char* data;
    unsigned int size;
    unsigned int capacity;
    // Begin with no linear-pixel storage.
    ByteBuffer() : data(0), size(0), capacity(0) {}
    // Release the temporary linear-pixel buffer.
    ~ByteBuffer() {
        if (data) {
            size -= (data + size) - data;
            appFreeUni__FPv(data);
        }
    }
};
struct ByteGrow {
    unsigned char* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* cap;
    unsigned int offset;
    // Begin an empty owner for replacement byte-buffer storage.
    ByteGrow(unsigned int* c) : data(0), size(0), capacity(0), cap(c), offset(0) {}
    // Free whichever byte allocation remains after the growth swap.
    ~ByteGrow() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Grow byte storage using the target vector's 3/5 then 1/2 growth policy.
inline unsigned int grownBytes(unsigned int old, const unsigned int& need) {
    if (old < 0x55555555) {
        unsigned int growth = ((old + 1) * 3) / 5;
        return old + larger(growth, need);
    }
    if (old < 0xaaaaaaaa) {
        unsigned int growth = (old + 1) / 2;
        return old + larger(growth, need);
    }
    return 0xffffffff;
}
// Byte view of the temporary LINEAR RGBA pixels, before rebuilding GPU tile planes.
struct CupCaptureRgbaPixel {
    unsigned char red, green, blue, alpha;
};

// Convert the captured texture channels and alpha through a temporary byte buffer.
extern "C" void TextureCapture_SwapRedBlueAndMaskAlpha(CGolfCup* self) {
    // Requires the fixed 128x128, four-byte capture texture. The pixel loop
    // is fixed-size even though description dimensions are queried below.
    // Phase 1: lock level 0 and, when required, obtain linear temporary pixels.
    void* texture = CupCaptureTexture(self).ptr;
    TextureInfo description;
    Texture_GetLevelDescription(texture, 0, &description);
    void* format =
        gtl_ImageFormat_FromPlatformFormat(description.words[kTexturePlatformFormatWord], kCapturePlatformLayout);
    struct FormatInfo {
        void* format;        // Pixel-layout object returned by FromPlatformFormat.
        unsigned int width;  // Width in pixels.
        unsigned int height; // Height in pixels.
        unsigned int one;    // Descriptor word set to 1; semantic meaning unresolved.
        unsigned int unused; // Never read or initialized by this path.
    };

    unsigned int width = description.words[kTextureWidthWord];
    FormatInfo imageLayout;
    ((volatile FormatInfo*)&imageLayout)->format = format;
    ((volatile FormatInfo*)&imageLayout)->width = width;
    ((volatile FormatInfo*)&imageLayout)->height = (description.words[kTextureHeightWord]);
    ((volatile FormatInfo*)&imageLayout)->one = 1;
    void* rowPitchWord = gtl_Image_GetRowPitch(width, format, kCapturePlatformLayout);

    TextureLock lock;
    Texture_LockLevel(texture, 0, &lock, 0, 0);
    unsigned char* pixels;
    ByteBuffer buffer;
    if (TextureFormat_IsPlatformCMPR_C8OrRGBA8(description.words[kTexturePlatformFormatWord])) {
        unsigned int requiredBytes =
            Texture_CalculateMipStorageSize(description.words[kTextureWidthWord], description.words[kTextureHeightWord],
                                            1, description.words[kTexturePlatformFormatWord]);
        // Storage phase: use spare bytes or stage a new allocation before publishing it.
        if (requiredBytes > buffer.size) {
            unsigned int bytesToAppend = requiredBytes - buffer.size;
            // Shared pixel swap code, run after either resize path.
            do {
                if (bytesToAppend > buffer.capacity || buffer.size > buffer.capacity - bytesToAppend) {
                    unsigned int current = *(volatile unsigned int*)&buffer.size;
                    unsigned int previousCapacity = (*(volatile unsigned int*)&buffer.capacity);
                    if (current + bytesToAppend - buffer.capacity > 0xffffffff - previousCapacity) {
                        union {
                            char c;
                            unsigned int align;
                        } flag;
                        flag.c = 0;
                        ErrorString err(golfCupMeshStrings + kCupVectorLengthError, &flag.c);
                        __throw((golfCupMeshStrings + kCupLengthErrorType), &err, __dt__Q23std12length_errorFv);
                    }
                    grownBytes(previousCapacity, current + bytesToAppend - buffer.capacity);
                } else {
                    memset((buffer.data + buffer.size), 0, bytesToAppend);
                    buffer.size += bytesToAppend;
                    break;
                }
                {
                    // Grow transaction: construct zeroed tail, transfer prefix, swap owners.
                    unsigned int requiredSize = (*(volatile unsigned int*)&buffer.size) + bytesToAppend;
                    ByteGrow pendingPixels(&buffer.capacity);
                    unsigned int previousCapacity = *(volatile unsigned int*)&buffer.capacity;
                    unsigned int requiredGrowth = requiredSize - previousCapacity;
                    if (requiredGrowth > 0xffffffff - previousCapacity) {
                        union {
                            char c;
                            unsigned int align;
                        } flag;
                        flag.c = 0;
                        ErrorString err(golfCupMeshStrings + kCupVectorLengthError, &flag.c);
                        __throw((golfCupMeshStrings + kCupLengthErrorType), &err, __dt__Q23std12length_errorFv);
                    }
                    previousCapacity = grownBytes(previousCapacity, requiredGrowth);
                    pendingPixels.data = (unsigned char*)appMallocDummy__FUl(previousCapacity);
                    pendingPixels.capacity = previousCapacity;
                    pendingPixels.offset = buffer.size;
                    memset(pendingPixels.data + buffer.size + pendingPixels.size, 0, bytesToAppend);
                    unsigned char* first = buffer.data;
                    pendingPixels.size += bytesToAppend;

                    unsigned char* end = buffer.data + (*(volatile unsigned int*)&buffer.size);
                    int prefixBytes = end - first;
                    pendingPixels.offset = pendingPixels.offset - prefixBytes;
                    memcpy((pendingPixels.offset + pendingPixels.data), first, prefixBytes);
                    memset(first, 0, prefixBytes);
                    pendingPixels.size += prefixBytes;
                    unsigned char* previous = buffer.data;
                    unsigned int previousCap = buffer.capacity;
                    buffer.data = pendingPixels.data;
                    buffer.capacity = pendingPixels.capacity;
                    pendingPixels.capacity = previousCap;
                    pendingPixels.data = previous;
                    buffer.size = pendingPixels.size;
                    pendingPixels.size = 0;
                }
            } while (0);
        } else if (requiredBytes < buffer.size)
            buffer.size -= buffer.size - requiredBytes;
        pixels = buffer.data;
        // Format phase: linearize locked GPU storage into the owned buffer.
        gtl_Image_CopyFromPlatform7Layout(lock.data, description.words[kTextureWidthWord],
                                          description.words[kTextureHeightWord], 0, pixels, rowPitchWord, 0,
                                          description.words[kTexturePlatformFormatWord]);

        gtl_ImageFormat_ToSwizzled(((volatile FormatInfo*)&imageLayout)->format);
    } else
        pixels = lock.data;
    // Phase 2: swap R/B, then clear alpha only when ALL three channels differ
    // from 255. A pixel with ANY channel equal to 255 retains its prior alpha.
    unsigned int i;
    CupCaptureRgbaPixel* pixel = (CupCaptureRgbaPixel*)pixels;
    for (i = 0; i < kCaptureTextureSize * kCaptureTextureSize; ++pixel, ++i) {
        unsigned char red = pixel->red;
        pixel->red = pixel->blue;
        pixel->blue = red;
        if (kCaptureChannelFull != pixel->red && pixel->green != kCaptureChannelFull &&
            pixel->blue != kCaptureChannelFull)
            pixel->alpha = 0;
    }
    // Phase 3: rebuild the GPU AR/GB tile planes and flush through UnlockLevel.
    Texture_ConvertRGBA8ToTiledARGBPlanes(pixels, rowPitchWord, 0, lock.data, kCaptureTextureSize, kCaptureTextureSize,
                                          0, kCaptureRgbaBytes);
    Texture_UnlockLevel(texture, 0);
}
