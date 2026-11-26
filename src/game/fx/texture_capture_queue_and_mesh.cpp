// Render-to-texture capture and textured mesh layers: the capture queue draws puppet items into
// capture textures, the texture item list feeds it, and mesh layers scroll their UV coordinates.

#include <decomp/static_init.h>
typedef unsigned long new_size_t;
extern "C" {
extern unsigned char kProjectedShadowProjectionRenderStates[];
}
#include "classes/WView.h"
#include "classes/TextureItem801F.h"
#define AsTextureItem801F(p) ((TextureItem801F*)(p))
#include "classes/TextureListNode.h"
#define AsTextureListNode(p) ((TextureListNode*)(p))
#include "classes/TextureCaptureQueue.h"
// TextureCaptureQueue_DrawNextCapture (0x801EE640): cursor +8 is the
// capture resource and cursor +4 is the next list link.
#include "classes/TextureItem801F.h"
#include "classes/TextureListNode.h"
#include "game/wvector.h"
#include "classes/WPuppet.h"
#include <decomp/cw_bitfields.h>

// Render-state suffix not represented by the shared queue header.
struct CaptureViewport {
    int x, y, width, height;
    float nearDepth, farDepth;
};
struct CaptureScissor {
    int x, y, width, height;
};
struct CaptureVertex {
    float x, y, z;
    unsigned char red, green, blue, alpha;
};
struct CaptureRenderView {
    void* savedRenderTarget;
    int captureCount;
    void* headPrevious;
    void* headNext;
    void* drawCursor;
    CaptureViewport captureViewport;
    CaptureViewport savedViewport;
    CaptureScissor captureScissor;
    CaptureScissor savedScissor;
    CaptureVertex quad[4];
};
struct TextureItemQueueView {
    int itemCount;
    void* headPrevious;
    void* headNext;
    char padding0C[0x170];
    unsigned int bindFlags;
};
struct RenderCameraMatrices {
    char padding[0x20];
    float view[16];
    float projection[16];
};
struct TextureMeshLayer {
    float scrollVelocity[2];
    unsigned int color;
    char* texcoords;
    void* texture;
};
// Layers are 20-byte records; each references vertexCount UV pairs (8-byte
// stride). Shared position records are XYZ floats (12-byte stride). Draw replaces
// each position's Y with vertexY, so positions feed an XZ plane in world space;
// camera view/projection are applied separately. UVs are texture coordinates,
// not pixel offsets; ScrollTexcoords adds scrollVelocity * caller scale.
enum TextureMeshStrides {
    kMeshLayerBytes = 0x14,
    kMeshPositionBytes = 12,
    kMeshUVBytes = 8
};
struct TextureMeshView {
    char padding00[0x10];
    float vertexY;
    char padding14[4];
    char* positions;
    int vertexCount;
    char* layers;
    unsigned char vertexCapacity;
    char renderBufferRest[11];
    int layerCount;
};
// Keep byte-strided expressions where direct subobject pointers change scheduling.
enum {
    kTextureMeshRendererOffset = (int)&((TextureMeshView*)0)->vertexCapacity,
    kTextureMeshTexcoordsOffset = (int)&((TextureMeshLayer*)0)->texcoords
};
struct BonePositionView {
    char padding[0x9C];
    WVector position;
};
enum TextureRenderConstants {
    kShadowBindFlag = 0x100,
    kCaptureTextureFormat = 0x80000006,
    kDefaultCaptureSize = 256,
    kCaptureBorder = 2,
    kCaptureBorderWidth = 4,
    kCapturePrimitive = 0x98,
    kCaptureVertexFormat = 0xF005,
    kTextureMeshPrimitive = 0xA0,
    kCaptureSurfaceFormat = 0x2C,
    kTextureNodeBytes = sizeof(TextureListNode)
};

extern "C" {

extern float kFloat_6_2831855;
float s_textureCaptureFullTurnAngle; // Module angle storage initialized to one full turn.
extern const float kFloat_1_TextureRender;

void Engine_ClearFramebuffer(float, int, int, int, int, int);
void Memory_Clear(void*, int);
void TextureCaptureQueue_SetSize(char*, int, int);
void Engine_AcquireCurrentRenderTarget(int, void*);
void Engine_GetViewport(void*);
void Engine_SetViewport(void*);
void Engine_SetRenderTarget(int, void*);
void Engine_GetScissorRect(void*);
void Engine_SetScissorRect(void*);
int RenderSurface_CreateAdapter(void*, void*, int, int, int, int, void**, int); // Engine_CreateRenderTarget.
long long CursorVec2_Scale(void*, float);
void Vec2_AddAssign(void*, void*);
void CubeResourceStorage_dtor(void*, int);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void* g_textureCaptureRenderTarget;
void* s_textureCaptureTargetWidth;  // Numeric pixel width carried as a Wii word in this ABI; never dereference.
void* s_textureCaptureTargetHeight; // Numeric pixel height carried as a Wii word in this ABI; never dereference.
extern const float kFloat_0_Shadow;
extern const double kUnsignedIntToDoubleBias_TextureCapture;
extern unsigned int kIdentityMatrix4x4[16];
extern unsigned int textureCaptureRenderStates[18];
void Engine_SetTextureStageResource(int, int);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetType7Resource(int); // Engine_SetType7Resource.
void SetTransform(int, void*);
void Engine_SetRenderStateList(void*, int);
void DrawPrimitiveUP(int, int, void*, int);
void Engine_CopyFramebufferToTexture(
    int, int, void*, int, int, int, int, int, int, int,
    int); // 11 integer args: the original stores 0 to sp+0x8, 0xc and 0x10 (0x801EE6D8-0x801EE6F8)
int Texture_Create2D(void*, void*, int, int, int, int, void**, int);
void* appMallocDummy__FUl(int);
void ShadowModel_BindPuppetAndChildren(void*, void*, unsigned int);
int WPuppet_FindNamedBoneEntry(const char*, const char*);
void __as__7WVectorFRC7WVector(void*, const void*);
void __apl__7WVectorFRC7WVector(void*, const void*);
void Vec3_DivideAssign(void*, float);
void Vec3_CopyToOutput(void*, void*);
void* __nw__FUl(unsigned int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_SetTextureStageParameter(int, int, int);
void Engine_SetTexturePalette(int, int);
void* Camera_GetRenderState(void*);
void* g_view;

struct Elem8 {
    unsigned int w0;
    unsigned int w1;
};
struct Decl15 {
    Elem8 e[15];
};
struct Decl6 {
    Elem8 e[6];
};
struct Decl5 {
    Elem8 e[5];
};
struct Decls {
    Decl15 a;
    Decl6 b;
    Decl5 c;
};
extern Decls textureMeshRenderStateDefaults; // Mesh render, texture-stage, and texture-parameter defaults.
typedef struct {
    int x;
    union {
        int i;
        float f;
    } y;
    int z;
} V801F;

struct Pad801x4 {
    char pad[4];
};
struct CTextureMeshVertexWriter : Pad801x4 {
    virtual void vf0();
    // Prepare the embedded vertex renderer before setting indexed attributes.
    virtual void PrepareVertices();
    virtual void vf2();
    // Store the four color bytes for vertex i (byte order is retained as loaded).
    virtual void SetColor(unsigned char i, unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3);
    // Store XYZ float bits for vertex i; V801F's integer view is a bit copy.
    virtual void SetPosition(unsigned char i, V801F v);
    // Store the floating-point UV pair for vertex i.
    virtual void SetUV(unsigned char i, float f1, float f2);
    // Submit the selected primitive and primitive count using the stored attributes.
    virtual void SubmitVertices(int primitive, int primitiveCount);
};
// Prepare the borrowed embedded vertex writer before setting indexed attributes.
static inline void PrepareTextureMeshVertices(CTextureMeshVertexWriter* writer) {
    writer->PrepareVertices();
}
// Set XYZ float bits for one byte-indexed vertex; capacity was checked by the caller.
static inline void WriteTextureMeshPosition(CTextureMeshVertexWriter* writer, unsigned char vertex,
                                            const V801F& position) {
    writer->SetPosition(vertex, position);
}
// Set the UV pair for one byte-indexed vertex; capacity was checked by the caller.
static inline void WriteTextureMeshUV(CTextureMeshVertexWriter* writer, unsigned char vertex, float u, float v) {
    writer->SetUV(vertex, u, v);
}
// Set packed color bytes in their existing byte order for one indexed vertex.
static inline void WriteTextureMeshColor(CTextureMeshVertexWriter* writer, unsigned char vertex, unsigned char c0,
                                         unsigned char c1, unsigned char c2, unsigned char c3) {
    writer->SetColor(vertex, c0, c1, c2, c3);
}
// Submit the filled mesh writer using the supplied primitive and count contract.
static inline void SubmitTextureMeshVertices(CTextureMeshVertexWriter* writer, int primitive, int count) {
    writer->SubmitVertices(primitive, count);
}

struct TextureDevice801 {
    virtual void vf0();
    virtual void vf1();
    virtual void vf2();
    virtual void vf3();
    virtual void vf4();
    virtual void vf5();
    virtual void vf6();
    virtual void vf7();
    virtual void vf8();
    virtual void vf9();
    virtual void vf10();
    virtual void vf11();
    virtual void vf12();
    virtual void vf13();
    virtual void vf14();
    virtual void vf15();
    virtual void vf16();
    virtual void vf17();
    virtual void vf18();
    virtual void vf19();
    virtual void vf20();
    virtual void vf21();
    virtual void vf22();
    virtual void vf23();
    virtual void vf24();
    virtual void vf25();
    // Resolve the texture ID into the stage-resource handle used by Engine_SetTextureStageResource.
    virtual int vf26(void*);
    // Resolve the same ID into the texture handle used by Engine_SetTexturePalette.
    virtual int vf27(void*);
};

// This constructor view carries numeric counts/capacities in pointer-sized words.
// Their bits are scalars; only vertexData/items own allocations. The shared
// TextureItem801F view describes the same storage for cleanup operations.
struct Item801F {
    int reservedWord0;
    void* vertexData;
    void* vertexDataCount;
    void* vertexDataCapacity;
    void** items;
    int count;
    void* textureCapacity;
    Item801F(void*, int);
    ~Item801F();
};
extern unsigned int kVec3NormalizeZeroFallback[3];
extern char puppetBoneExceptionStrings[];

// Drop one packed high-half resource reference; destroy only after the packed-word reload.
// Caller clears its own pointer slot after this synchronous release operation.
static inline void ReleaseTextureReferenceWithReload(void* resource) {
    typedef struct {
        CW_BITS2(unsigned int hi : 16, unsigned int lo : 16)
    } U32HL;
    U32HL* bits = (U32HL*)resource;
    bits->hi = bits->hi - 1;
    unsigned int value = *(unsigned int*)resource;
    if ((value >> 16) == 0)
        CubeResourceStorage_dtor(resource, 1);
}

// Restore viewport/target, release the saved target reference, then restore scissor state.
int TextureCaptureQueue_EndDraw(char* arg) {
    Engine_SetViewport(&((CaptureRenderView*)arg)->savedViewport);
    Engine_SetRenderTarget(0, *(void**)arg);
    if (*(void**)arg != 0) {
        void* obj = *(void**)arg;
        ReleaseTextureReferenceWithReload(obj);
        *(void**)arg = 0;
    }
    Engine_SetScissorRect(&((CaptureRenderView*)arg)->savedScissor);
    Engine_ClearFramebuffer(kFloat_0_Shadow, 0, 0, 7, 0, 0);
    return 0;
}

struct Iterator3 {
    void* value;
    // Leave iterator scratch uninitialized until assigned.
    Iterator3() {}
    // Borrow a list-header address.
    Iterator3(void* p) : value(p) {}
    // Copy the borrowed iterator address.
    Iterator3& operator=(const Iterator3& other) {
        value = other.value;
        return *this;
    }
};
struct Guard3 {
    Iterator3 list;
    void* node;
    // Leave guard scratch for the insertion path to initialize.
    Guard3() {}
    // Free an allocated node unless insertion disarmed the guard.
    ~Guard3() {
        if (node)
            appFreeUni__FPv(node);
    }
};
extern "C++" {
// Construct into caller-provided storage without allocating.
inline void* operator new(new_size_t, void* p) {
    return p;
}
// Placement-delete cleanup leaves caller-owned storage intact.
inline void operator delete(void*, void*) {}
}
struct Link801F {
    void* value;
    // Store the texture reference transferred to this list node.
    Link801F(void* v) : value(v) {}
};

#pragma push
#pragma opt_propagation off
// Create a capture texture and insert it into the owned capture list.
int TextureCaptureQueue_AddTexture(void* captureQueue) {
    void* volatile temporary = 0;
    volatile unsigned char flag;
    void* volatile f10;
    void* volatile f0c;

    if (Texture_Create2D(((TextureCaptureQueue*)captureQueue)->captureWidth,
                         ((TextureCaptureQueue*)captureQueue)->captureHeight, 1, 0, kCaptureTextureFormat, 0,
                         (void**)&temporary, 0) == 0) {
        void* obj = temporary;
        if (obj != 0) {
            typedef struct {
                CW_BITS2(unsigned int hi : 16, unsigned int lo : 16)
            } U32HL;
            U32HL* bits = (U32HL*)obj;
            bits->hi = bits->hi - 1;
            unsigned int value = *(unsigned int*)obj;
            if ((value >> 16) == 0) {
                int one = 1;
                CubeResourceStorage_dtor(obj, (short)one);
            }
            temporary = 0;
        }
        return 0;
    }
    flag = 0;
    Iterator3 list(&((TextureCaptureQueue*)captureQueue)->headPrevious);
    void* node = appMallocDummy__FUl(kTextureNodeBytes);
    void** link = &((TextureListNode*)node)->value;
    {
        Guard3 guard;
        f10 = node;
        f0c = node;
        guard.list = list;
        guard.node = node;
        new (link) Link801F(temporary);
        void* first = *(void**)list.value;
        guard.node = 0;
        ((TextureListNode*)first)->next = node;
        *(void**)node = *(void**)list.value;
        *(void**)list.value = node;
        ((TextureListNode*)node)->next = list.value;
        ++((TextureCaptureQueue*)captureQueue)->captureCount;
    }
    return ((TextureCaptureQueue*)captureQueue)->captureCount;
}
#pragma pop

// Draw the capture quad, copy the framebuffer into its texture, and advance the cursor.
void TextureCaptureQueue_DrawNextCapture(void* captureQueue) {
    Engine_SetTextureStageResource(0, 0);
    Engine_SetFlexibleVertexFormat(kCaptureVertexFormat);
    Engine_SetType7Resource(0);
    SetTransform(0xB, (char*)&kIdentityMatrix4x4);
    struct {
        int pad[2];
        unsigned int matrix[18];
    } buf;
    unsigned int* src = (unsigned int*)&textureCaptureRenderStates;
    unsigned int* dst = buf.matrix;
    unsigned int* end = src + 18;
    while (src != end) {
        *dst++ = *src++;
        *dst++ = *src++;
    }
    Engine_SetRenderStateList(buf.matrix, 9);
    DrawPrimitiveUP(kCapturePrimitive, 2, ((CaptureRenderView*)captureQueue)->quad, sizeof(CaptureVertex));
    void* tex = ((TextureListNode*)((TextureCaptureQueue*)captureQueue)->drawCursor)->value;
    // The callee reads eleven integer arguments; the original call also loads an ignored f1.
    ((void (*)(int, int, void*, float, int, int, int, int, int, int, int, int))Engine_CopyFramebufferToTexture)(
        0x100, 0, tex, kFloat_0_Shadow, 0, 0, 0, 0, 0, 0, 0, 0);
    ((TextureCaptureQueue*)captureQueue)->drawCursor =
        ((TextureListNode*)((TextureCaptureQueue*)captureQueue)->drawCursor)->next;
}

struct NewItem97C {
    int reservedWord0;
    void* fields[6];
    // Initialize empty resource arrays and bind the puppet/children into this item.
    NewItem97C(void* item, unsigned int flags) {
        for (int i = 0; i < 6; ++i)
            fields[i] = 0;
        ShadowModel_BindPuppetAndChildren(this, item, flags);
    }
};

struct Iterator97C {
    void* value;
    // Leave iterator scratch for later assignment.
    Iterator97C() {}
    // Borrow the item-list header address.
    Iterator97C(void* p) : value(p) {}
    // Copy the borrowed header address.
    Iterator97C& operator=(const Iterator97C& other) {
        value = other.value;
        return *this;
    }
};
struct Guard97C {
    void* pad;
    Iterator97C list;
    void* node;
    // Leave insertion-guard scratch for the caller to initialize.
    Guard97C() {}
    // Free the node only if insertion did not commit it to the list.
    ~Guard97C() {
        if (node)
            appFreeUni__FPv(node);
    }
};

// Bind a puppet item and insert it at the front of the texture-item list.
void TextureItemQueue_PushFront(void* itemQueue, void* item) {
    NewItem97C* object = new NewItem97C(item, ((TextureItemQueueView*)itemQueue)->bindFlags & kShadowBindFlag);
    volatile unsigned char flag = 0;
    Iterator97C list(&((TextureItemQueueView*)itemQueue)->headPrevious);
    void* node = appMallocDummy__FUl(kTextureNodeBytes);
    void** link = &((TextureListNode*)node)->value;
    Guard97C guard;
    void* volatile f10;
    void* volatile f0c;
    f10 = node;
    f0c = node;
    guard.list = list;
    guard.node = node;
    new (link) NewItem97C*(object);
    void* first = *(void**)list.value;
    guard.node = 0;
    ((TextureListNode*)first)->next = node;
    *(void**)node = *(void**)list.value;
    *(void**)list.value = node;
    ((TextureListNode*)node)->next = list.value;
    ++*(int*)itemQueue;
}

struct GuardA78 {
    void* list;
    void* node;
    // Leave guard scratch for the replacement path to initialize.
    GuardA78() {}
    // Release the unlinked node on an incomplete insertion.
    ~GuardA78() {
        if (node)
            appFreeUni__FPv(node);
    }
};
struct NewItemA78 {
    int reservedWord0;
    void* fields[6];
    // Initialize item buffers and bind the replacement puppet and child resources.
    NewItemA78(void* item, unsigned int flags) {
        fields[0] = 0;
        fields[1] = 0;
        fields[2] = 0;
        fields[3] = 0;
        fields[4] = 0;
        fields[5] = 0;
        ShadowModel_BindPuppetAndChildren(this, item, flags);
    }
};
// Return one-past the resource array and publish the original cleanup scratch copies.
inline void** endA78(void* object, void* volatile& end1, void* volatile& end2) {
    void** end = ((TextureItem801F*)object)->resources + ((TextureItem801F*)object)->resourceCount;
    end1 = end;
    end2 = end;
    return end;
}

#pragma push
#pragma use_lmw_stmw on
// Release the front item resources and replace it with a newly bound puppet item.
// Owns front item/node allocations and one reference per resource slot; item
// is a borrowed puppet binding argument. Old ownership is gone before replacement
// allocation, so failure cannot restore the previous front. Volatile scratch below
// records compiler cleanup state; it is not queue state and must not be tuned.
void TextureItemQueue_ReplaceFront(void* itemQueue, void* item) {
    void* volatile frameHead;
    void* volatile headCopy;
    void* volatile node1b;
    void* volatile node1a;
    void* volatile begin2;
    void* volatile end2;
    void* volatile begin1;
    void* volatile end1;
    void* volatile node2b;
    void* volatile node2a;
    __declspec(align(4)) volatile unsigned char flag1;
    __declspec(align(4)) volatile unsigned char flagData;
    __declspec(align(4)) volatile unsigned char flagItems;
    __declspec(align(4)) volatile unsigned char flag2;
    register void* insertion;
    if (*(int*)itemQueue == 0) {
        // Empty queue: bind and insert without trying to release a front item.
        NewItemA78* object = new NewItemA78(item, ((TextureItemQueueView*)itemQueue)->bindFlags & kShadowBindFlag);
        flag1 = 0;
        insertion = &((TextureItemQueueView*)itemQueue)->headPrevious;
        void* node = appMallocDummy__FUl(kTextureNodeBytes);
        void** link = &((TextureListNode*)node)->value;
        GuardA78 guard;
        node1b = node;
        node1a = node;
        guard.list = insertion;
        guard.node = node;
        new (link) NewItemA78*(object);
        void* first = *(void**)insertion;
        guard.node = 0;
        ((TextureListNode*)first)->next = node;
        *(unsigned int*)node = *(unsigned int*)insertion;
        *(void**)insertion = node;
        ((TextureListNode*)node)->next = insertion;
        ++*(unsigned int*)itemQueue;
    } else {
        // Existing queue: release resource references and both buffers before unlinking.
        void* head = ((TextureItemQueueView*)itemQueue)->headNext;
        void* object = ((TextureListNode*)head)->value;
        headCopy = head;
        frameHead = head;
        if (object != 0) {
            if (object != 0) {
                *(int*)object = 0;
                if (((TextureItem801F*)object)->resourceCount != 0) {
                    void** iter = ((TextureItem801F*)object)->resources;
                    begin1 = iter;
                    begin2 = iter;
                    for (; iter != endA78(object, end1, end2); ++iter) {
                        void* current = *iter;
                        if (current != 0) {
                            typedef struct {
                                CW_BITS2(unsigned int hi : 16, unsigned int lo : 16)
                            } U32HL;
                            U32HL* bits = (U32HL*)current;
                            bits->hi = bits->hi - 1;
                            if (bits->hi == 0) {
                                int deleting = 1;
                                CubeResourceStorage_dtor(current, (short)deleting);
                            }
                            *iter = 0;
                        }
                    }
                }
                void* member = ((char*)&AsTextureItem801F(object)->resources);
                if (member != 0 && member != 0 && member != 0 && member != 0 && member != 0 &&
                    (*(void**)&((TextureItem801F*)object)->resources) != 0) {
                    int count = ((TextureItem801F*)object)->resourceCount;
                    int countCopy = count;
                    flagItems = 0;
                    ((TextureItem801F*)object)->resourceCount = count - countCopy;
                    appFreeUni__FPv((*(void**)&((TextureItem801F*)object)->resources));
                }
                member = &((TextureItem801F*)object)->data;
                if (member != 0 && member != 0 && member != 0 && member != 0 && member != 0 &&
                    ((TextureItem801F*)object)->data != 0) {
                    int count = ((TextureItem801F*)object)->dataCount;
                    int countCopy = count;
                    flagData = 0;
                    ((TextureItem801F*)object)->dataCount = count - countCopy;
                    appFreeUni__FPv(((TextureItem801F*)object)->data);
                }
                __dl__FPv(object);
            }
            ((TextureListNode*)frameHead)->value = 0;
        }
        head = frameHead;
        unsigned int previous = (*(unsigned int*)&((TextureListNode*)head)->next);
        void* first = *(void**)head;
        (*(unsigned int*)&((TextureListNode*)first)->next) = previous;
        unsigned int next = *(unsigned int*)head;
        void* last = ((TextureListNode*)head)->next;
        *(unsigned int*)last = next;
        appFreeUni__FPv(head);
        --*(int*)itemQueue;

        // Construct and link the replacement only after old ownership is gone.
        NewItemA78* replacement = new NewItemA78(item, ((TextureItemQueueView*)itemQueue)->bindFlags & kShadowBindFlag);
        flag2 = 0;
        insertion = ((TextureItemQueueView*)itemQueue)->headNext;
        void* node = appMallocDummy__FUl(kTextureNodeBytes);
        void** link = &((TextureListNode*)node)->value;
        GuardA78 guard;
        node2b = node;
        node2a = node;
        guard.list = &((TextureItemQueueView*)itemQueue)->headPrevious;
        guard.node = node;
        new (link) NewItemA78*(replacement);
        first = *(void**)insertion;
        guard.node = 0;
        ((TextureListNode*)first)->next = node;
        *(unsigned int*)node = *(unsigned int*)insertion;
        *(void**)insertion = node;
        ((TextureListNode*)node)->next = insertion;
        ++*(unsigned int*)itemQueue;
    }
}
#pragma pop

#pragma push
#pragma opt_propagation off
// Draw each textured mesh layer using its positions, scrolling UVs, and vertex color.
// Requires at least two vertices for the primitive-count contract and
// valid arrays for every layer; zero vertices exits before touching render state.
// Indexed writes are guarded by the embedded renderer's byte-sized capacity.
void TextureMesh_DrawLayers(char* mesh) {
    Decls* src = &textureMeshRenderStateDefaults;
    if (((TextureMeshView*)mesh)->vertexCount == 0)
        return;
    V801F v;
    // Apply mesh defaults and camera matrices, then prepare the embedded renderer.
    Decl5 textureStageParameters;
    Decl6 textureStageStates;
    Decl15 renderStates = src->a;
    textureStageStates = src->b;
    textureStageParameters = src->c;
    Engine_SetRenderStateList(&renderStates, 15);
    Engine_SetTextureStageStateList(0, &textureStageStates, 6);
    Engine_SetTextureStageParameterList(0, &textureStageParameters, 5);

    void* dev = Camera_GetRenderState(g_view);
    SetTransform(0, ((RenderCameraMatrices*)dev)->view);
    SetTransform(1, ((RenderCameraMatrices*)dev)->projection);
    Engine_SetType7Resource(0);
    PrepareTextureMeshVertices(((CTextureMeshVertexWriter*)(mesh + kTextureMeshRendererOffset)));
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(0xB, kIdentityMatrix4x4);
    Engine_SetTextureStageParameter(0, 0, 1);
    Engine_SetTextureStageParameter(0, 1, 1);

    int layerByteOffset;
    int positionByteOffset;
    int texcoordByteOffset;
    int layerIndex;
    char* layer;
    int i;
    layerIndex = 0;
    layerByteOffset = 0;
    while (layerIndex < ((TextureMeshView*)mesh)->layerCount) {
        // Fill this layer's vertex attributes from the shared XZ plane and its UVs.
        layer = ((TextureMeshView*)mesh)->layers + layerByteOffset;
        i = 0;
        positionByteOffset = 0;
        texcoordByteOffset = 0;
        for (; i < ((TextureMeshView*)mesh)->vertexCount; i++) {
            char* p = ((TextureMeshView*)mesh)->positions + positionByteOffset;
            v.x = *(int*)(p + 0);
            v.y.i = *(int*)(p + 4);
            v.z = *(int*)(p + 8);
            v.y.f = ((TextureMeshView*)mesh)->vertexY;
            if (((TextureMeshView*)mesh)->vertexCapacity > (unsigned char)i)
                WriteTextureMeshPosition(((CTextureMeshVertexWriter*)(mesh + kTextureMeshRendererOffset)),
                                         (unsigned char)i, v);

            char* q = ((TextureMeshLayer*)layer)->texcoords + texcoordByteOffset;
            float f2 = *(float*)(4 + q);
            float f1 = *(float*)(q + 0);

            if (((TextureMeshView*)mesh)->vertexCapacity > (unsigned char)i)
                WriteTextureMeshUV(((CTextureMeshVertexWriter*)(mesh + kTextureMeshRendererOffset)), (unsigned char)i,
                                   f1, f2);

            union {
                unsigned int i;
                unsigned char c[4];
            } col;
            col.i = ((TextureMeshLayer*)layer)->color;
            unsigned char c3 = (col.c[3]);
            unsigned char c2 = (col.c[2]);
            unsigned char c1 = (col.c[1]);
            if (((TextureMeshView*)mesh)->vertexCapacity > (unsigned char)i)
                WriteTextureMeshColor(((CTextureMeshVertexWriter*)(mesh + kTextureMeshRendererOffset)),
                                      (unsigned char)i, (col.c[0]), c1, c2, c3);
            positionByteOffset += kMeshPositionBytes;
            texcoordByteOffset += kMeshUVBytes;
        }
        void* id = ((TextureMeshLayer*)layer)->texture;
        // The device exposes distinct stage-resource and texture handles for one ID.
        Engine_SetTextureStageResource(0, (((TextureDevice801*)((WView*)g_view)->renderTarget))->vf26(id));
        Engine_SetTexturePalette(0, (((TextureDevice801*)((WView*)g_view)->renderTarget))->vf27(id));
        SubmitTextureMeshVertices(((CTextureMeshVertexWriter*)(mesh + kTextureMeshRendererOffset)),
                                  kTextureMeshPrimitive, ((TextureMeshView*)mesh)->vertexCount - 2);
        layerByteOffset += kMeshLayerBytes;
        layerIndex = layerIndex + 1;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Add each layer scroll velocity times the supplied scale to all of its texture coordinates.
// scale is supplied by the caller; no time conversion or UV wrapping occurs here.
// Each layer's first two floats are its velocity; texcoords points to UV pairs.
void TextureMesh_ScrollTexcoords(char* mesh, float scale) {
    int layerByteOffset;
    int texcoordByteOffset;
    int layerIndex;
    int vertexIndex;
    layerIndex = 0;
    layerByteOffset = 0;
    while (layerIndex < ((TextureMeshView*)mesh)->layerCount) {
        vertexIndex = 0;
        texcoordByteOffset = 0;
        while (vertexIndex < ((TextureMeshView*)mesh)->vertexCount) {
            // Address = layers base + layerIndex*20. Its first 8 bytes are the
            // velocity pair; the pointer at +12 selects vertexIndex*8 UV bytes.
            // Integer/byte views preserve the original instruction scheduling.
            long long scaledScrollUV =
                CursorVec2_Scale((void*)((int)((TextureMeshView*)mesh)->layers + layerByteOffset), scale);
            Vec2_AddAssign(*(char**)((char*)layerByteOffset + (int)((TextureMeshView*)mesh)->layers +
                                     kTextureMeshTexcoordsOffset) +
                               texcoordByteOffset,
                           &scaledScrollUV);
            texcoordByteOffset += kMeshUVBytes;
            vertexIndex++;
        }
        layerByteOffset += kMeshLayerBytes;
        layerIndex++;
    }
}
#pragma pop

// Replace the shared capture render target and remember its creation dimensions.
// width/height contain numeric pixel counts in pointer-sized argument words.
// The routine owns one target reference; failure leaves the old target released.
int TextureCapture_SetSharedRenderTargetSize(void* width, void* height) {
    void* obj = g_textureCaptureRenderTarget;
    if (obj != 0) {
        unsigned int value = *(unsigned int*)obj;
        unsigned int count = (value >> 16) - 1;
        typedef struct {
            CW_BITS2(unsigned int hi : 16, unsigned int lo : 16)
        } U32HL;
        U32HL* p = (U32HL*)obj;
        p->hi = count;
        value = *(unsigned int*)obj;
        if ((value >> 16) == 0)
            CubeResourceStorage_dtor(obj, 1);
        g_textureCaptureRenderTarget = 0;
    }
    if (!RenderSurface_CreateAdapter(width, height, kCaptureSurfaceFormat, 0, 0, 0, &g_textureCaptureRenderTarget, 0))
        return 0;
    s_textureCaptureTargetWidth = width;
    s_textureCaptureTargetHeight = height;
    return 1;
}

// Initialize an empty capture list and a default 256-by-256 render region.
void* TextureCaptureQueue_Init(void* captureQueue) {
    (*(int*)&((TextureCaptureQueue*)captureQueue)->resource) = 0;
    ((TextureCaptureQueue*)captureQueue)->captureCount = 0;
    ((TextureCaptureQueue*)captureQueue)->headNext = &((TextureCaptureQueue*)captureQueue)->headPrevious;
    ((TextureCaptureQueue*)captureQueue)->headPrevious = &((TextureCaptureQueue*)captureQueue)->headPrevious;
    Memory_Clear(&((CaptureRenderView*)captureQueue)->captureViewport, sizeof(CaptureViewport));
    TextureCaptureQueue_SetSize((char*)captureQueue, kDefaultCaptureSize, kDefaultCaptureSize);
    return captureQueue;
}

// Clear the loading framebuffer with the original depth/color flags.
int Render_ClearLoadingFramebuffer() {
    Engine_ClearFramebuffer(kFloat_1_TextureRender, 0, 0, 3, -1, 0);
    return 1;
}

// Save render state and switch to the capture target; reject nested capture draws.
int TextureCaptureQueue_BeginDraw(char* arg) {
    if (*(int*)arg != 0)
        return 0;
    Engine_AcquireCurrentRenderTarget(0, arg);
    Engine_GetViewport(&((CaptureRenderView*)arg)->savedViewport);
    Engine_SetViewport(&((CaptureRenderView*)arg)->captureViewport);
    Engine_SetRenderTarget(0, (void*)g_textureCaptureRenderTarget);
    (*(int*)&((TextureCaptureQueue*)arg)->drawCursor) = (*(int*)&((TextureCaptureQueue*)arg)->headNext);
    Engine_GetScissorRect(&((CaptureRenderView*)arg)->savedScissor);
    Engine_SetScissorRect(&((CaptureRenderView*)arg)->captureScissor);
    return 1;
}

// Return the resource pointer stored in the first word of this wrapper.
void* Object_GetFirstWord(void** arg) {
    return *arg;
}

// Return the embedded list header immediately after the wrapper resource word.
char* TextureItemQueue_GetListHeader(char* arg) {
    return arg + 4;
}

// Initialize the texture module full-turn angle constant.
void TextureCapture_InitAngleConstant() {
    s_textureCaptureFullTurnAngle = kFloat_6_2831855;
}

// Unlink and free list nodes, then optionally delete the list wrapper.
void* dtor_801EDED0(void* arg, int deleting) {
    if (arg != 0) {
        if (arg != 0 && arg != 0 && arg != 0) {
            void* sentinel = &((TextureItemQueueView*)arg)->headPrevious;
            void* end = ((TextureCaptureQueue*)arg)->headPrevious;
            if (end != sentinel) {
                void* node = *(void**)sentinel;
                void* last = *(void**)end;
                void* next = ((TextureListNode*)node)->next;
                ((TextureListNode*)last)->next = next;
                void* first = *(void**)end;
                next = ((TextureListNode*)node)->next;
                *(void**)next = first;
                while (end != sentinel) {
                    void* current = end;
                    end = ((TextureListNode*)end)->next;
                    appFreeUni__FPv(current);
                    --*(int*)arg;
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(arg);
    }
    return arg;
}

// Unlink and free texture-item list nodes, then optionally delete the wrapper.
void* TextureItemList_dtor(void* itemQueue, int deleting) {
    if (itemQueue != 0) {
        if (itemQueue != 0 && itemQueue != 0 && itemQueue != 0) {
            void* sentinel = &((TextureItemQueueView*)itemQueue)->headPrevious;
            void* end = ((TextureItemQueueView*)itemQueue)->headNext;
            if (end != sentinel) {
                void* node = *(void**)sentinel;
                void* last = *(void**)end;
                void* next = ((TextureListNode*)node)->next;
                ((TextureListNode*)last)->next = next;
                void* first = *(void**)end;
                next = ((TextureListNode*)node)->next;
                *(void**)next = first;
                while (end != sentinel) {
                    void* current = end;
                    end = ((TextureListNode*)end)->next;
                    appFreeUni__FPv(current);
                    --*(int*)itemQueue;
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(itemQueue);
    }
    return itemQueue;
}

#pragma push
#pragma opt_common_subs off
// Set the capture viewport, two-pixel inset scissor, and translucent white quad.
void TextureCaptureQueue_SetSize(char* arg, int width, int height) {
    int loopCount;
    int zeroInt;
    float zeroFloat;
    float oneFloat;
    CaptureRenderView* vertexBase;
    int k;

    loopCount = kCaptureBorder;
    zeroInt = 0;
    zeroFloat = kFloat_0_Shadow;
    oneFloat = kFloat_1_TextureRender;
    vertexBase = (CaptureRenderView*)arg;
    ((CaptureRenderView*)arg)->captureViewport.x = zeroInt;
    k = 0;
    ((CaptureRenderView*)arg)->captureViewport.y = zeroInt;
    ((CaptureRenderView*)arg)->captureViewport.width = width;
    ((CaptureRenderView*)arg)->captureViewport.height = height;
    ((CaptureRenderView*)arg)->captureViewport.nearDepth = zeroFloat;
    ((CaptureRenderView*)arg)->captureViewport.farDepth = oneFloat;
    ((CaptureRenderView*)arg)->captureScissor.x = loopCount;
    ((CaptureRenderView*)arg)->captureScissor.y = loopCount;
    ((CaptureRenderView*)arg)->captureScissor.width = width - kCaptureBorderWidth;
    ((CaptureRenderView*)arg)->captureScissor.height = height - kCaptureBorderWidth;

    for (; k < 4; k++) {
        float f0;
        if (k & 1) {
            f0 = (float)(*(unsigned int*)&((CaptureRenderView*)arg)->captureViewport.width);
        } else {
            f0 = kFloat_0_Shadow;
        }
        vertexBase->quad[0].x = f0;
        if (k & 2) {
            f0 = (float)(*(unsigned int*)&((CaptureRenderView*)arg)->captureViewport.height);
        } else {
            f0 = kFloat_0_Shadow;
        }
        vertexBase->quad[0].y = f0;
        vertexBase->quad[0].z = zeroFloat;
        vertexBase->quad[0].red = 0xFF;
        vertexBase->quad[0].green = 0xFF;
        vertexBase->quad[0].blue = 0xFF;
        vertexBase->quad[0].alpha = 0x80;
        vertexBase = (CaptureRenderView*)((char*)vertexBase + sizeof(CaptureVertex));
    }
}

#pragma pop

struct RefNode801 {
    RefNode801* prev;
    RefNode801* next;
    void* obj;
};
struct RefList801 {
    int reservedWord0;
    int count;
    RefNode801 head;
};

#pragma push
#pragma opt_common_subs off
// Release every capture texture reference and free all capture list nodes.
void* TextureCaptureQueue_Dtor(void* arg, int deleting) {
    RefList801* list = (RefList801*)arg;
    if (list != 0) {
        if (list->count != 0) {
            RefNode801* end;
            RefNode801* sentinel;
            sentinel = &list->head;
            end = list->head.next;
            while (end != sentinel) {
                void* obj = end->obj;
                ReleaseTextureReferenceWithReload(obj);
                end = end->next;
            }
            {
                RefNode801* sentinel2 = &list->head;
                RefNode801* end2 = list->head.next;
                if (end2 != sentinel2) {
                    RefNode801* node = sentinel2->prev;
                    RefNode801* last = end2->prev;
                    RefNode801* next = node->next;
                    last->next = next;
                    RefNode801* first = end2->prev;
                    next = node->next;
                    next->prev = first;
                    while (end2 != sentinel2) {
                        RefNode801* current = end2;
                        end2 = end2->next;
                        appFreeUni__FPv(current);
                        --list->count;
                    }
                }
            }
        }
        if (&list->count != 0 && &list->count != 0 && &list->count != 0 && &list->count != 0) {
            RefNode801* sentinel = &list->head;
            RefNode801* end = list->head.next;
            if (end != sentinel) {
                RefNode801* node = sentinel->prev;
                RefNode801* last = end->prev;
                RefNode801* next = node->next;
                last->next = next;
                RefNode801* first = end->prev;
                next = node->next;
                next->prev = first;
                while (end != sentinel) {
                    RefNode801* current = end;
                    end = end->next;
                    appFreeUni__FPv(current);
                    --list->count;
                }
            }
        }
        if (deleting > 0)
            __dl__FPv(arg);
    }
    return arg;
}
#pragma pop

// Free the resource-pointer and vertex-data arrays after their resource references
// were released. Repeated subobject guards and count reloads retain binary shape.
static inline void FreeTextureItemOwnedArrays(void* obj) {
    void* member = ((char*)&AsTextureItem801F(obj)->resources);
    if (member != 0 && member != 0 && member != 0 && member != 0 && member != 0 &&
        (*(void**)&((TextureItem801F*)obj)->resources) != 0) {
        unsigned int count = (*(unsigned int*)&((TextureItem801F*)obj)->resourceCount);
        (*(unsigned int*)&((TextureItem801F*)obj)->resourceCount) =
            count - (unsigned int)((TextureItem801F*)obj)->resourceCount;
        appFreeUni__FPv((*(void**)&((TextureItem801F*)obj)->resources));
    }
    member = &((TextureItem801F*)obj)->data;
    if (member != 0 && member != 0 && member != 0 && member != 0 && member != 0 && ((TextureItem801F*)obj)->data != 0) {
        unsigned int count = (*(unsigned int*)&((TextureItem801F*)obj)->dataCount);
        (*(unsigned int*)&((TextureItem801F*)obj)->dataCount) =
            count - (unsigned int)((TextureItem801F*)obj)->dataCount;
        appFreeUni__FPv(((TextureItem801F*)obj)->data);
    }
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Release each item texture reference and both owned buffers, then free the list nodes.
// Phase 1 releases payloads/buffers but leaves list links intact for traversal;
// phase 2 unlinks and frees the nodes. The containing list storage stays borrowed.
void TextureItemList_ClearAndRelease(void* node) {
    void* sentinel;
    void* obj;
    void* list;
    void* zero;
    void** p;
    list = node;
    sentinel = &((TextureItemQueueView*)list)->headPrevious;
    zero = 0;
    node = AsTextureListNode(list)->value;
    while (node != sentinel) {
        obj = ((TextureListNode*)node)->value;
        if (obj != 0) {
            if (obj != 0) {
                *(void**)obj = zero;
                if (((TextureItem801F*)obj)->resourceCount != 0) {
                    p = ((TextureItem801F*)obj)->resources;
                    while (p != ((TextureItem801F*)obj)->resources + ((TextureItem801F*)obj)->resourceCount) {
                        void* item = *p;
                        if (item != 0) {
                            ReleaseTextureReferenceWithReload(item);
                            *p = zero;
                        }
                        p++;
                    }
                }
                FreeTextureItemOwnedArrays(obj);
                __dl__FPv(obj);
            }
            ((TextureListNode*)node)->value = zero;
        }
        node = ((TextureListNode*)node)->next;
    }
    // Phase 2: payload slots are cleared; release links and decrement the list count.
    void* clearSentinel = &((TextureItemQueueView*)list)->headPrevious;
    void* end = AsTextureListNode(list)->value;
    if (end != clearSentinel) {
        void* first = *(void**)clearSentinel;
        void* last = *(void**)end;
        void* next = ((TextureListNode*)first)->next;
        ((TextureListNode*)last)->next = next;
        void* head = *(void**)end;
        next = ((TextureListNode*)first)->next;
        *(void**)next = head;
        while (end != clearSentinel) {
            void* current = end;
            end = ((TextureListNode*)end)->next;
            appFreeUni__FPv(current);
            --*(int*)list;
        }
    }
}
#pragma pop

// Release all capture texture references and clear the intrusive list.
void TextureCaptureQueue_Clear(void* captureQueue) {
    if (((TextureCaptureQueue*)captureQueue)->captureCount != 0) {
        void* node = ((TextureCaptureQueue*)captureQueue)->headNext;
        void* sentinel;
        sentinel = &((TextureCaptureQueue*)captureQueue)->headPrevious;
        while (sentinel != node) {
            void* obj = ((TextureListNode*)node)->value;
            unsigned int value = *(unsigned int*)obj;
            typedef struct {
                CW_BITS2(unsigned int hi : 16, unsigned int lo : 16)
            } U32HL;
            ((U32HL*)obj)->hi = (value >> 16) - 1;
            value = *(unsigned int*)obj;
            if ((value >> 16) == 0)
                CubeResourceStorage_dtor(obj, 1);
            node = ((TextureListNode*)node)->next;
        }
        void* begin = ((TextureCaptureQueue*)captureQueue)->headNext;
        void* clearSentinel = &((TextureCaptureQueue*)captureQueue)->headPrevious;
        node = begin;
        sentinel = clearSentinel;
        if (sentinel != node) {
            void* last = *(void**)node;
            void* previousLink = *(void**)sentinel;
            void* next = ((TextureListNode*)previousLink)->next;
            ((TextureListNode*)last)->next = next;
            next = ((TextureListNode*)previousLink)->next;
            void* first = *(void**)node;
            *(void**)next = first;
            while (sentinel != node) {
                void* current = node;
                node = ((TextureListNode*)node)->next;
                appFreeUni__FPv(current);
                --((TextureCaptureQueue*)captureQueue)->captureCount;
            }
        }
    }
}

// Accumulate repeated first-puppet anchor samples, then divide by the stored list count.
// The puppet is loaded ONCE below: traversing the list does not select
// subsequent puppets. Preserve this
// behavior when editing shadow placement; repeated float sums may round.
// result borrows three writable floats in world space. Empty list or missing
// first payload returns the shared fallback vector. The list count must agree
// with its sentinel-terminated links; later successful bone finds require a
// non-null bone payload (only the first lookup checks that payload explicitly).
void PuppetList_GetRepeatedFirstAnchorPosition(void* result, void* puppetList) {
    unsigned char accumulatedPosition[12]; // PuppetList_GetRepeatedFirstAnchorPosition.
    void* firstNode;
    const char* firstPuppet;
    void* anchorBoneEntry;
    void* remainingNode;
    void* listSentinel;
    if (*(int*)puppetList == 0) {
        unsigned int* p = kVec3NormalizeZeroFallback;
        unsigned int v0 = *p++;
        unsigned int v1 = *p++;
        unsigned int v2 = *p;
        *(unsigned int*)result = v0;
        *((unsigned int*)result + 1) = v1;
        *((unsigned int*)result + 2) = v2;
        return;
    }
    firstNode = *((void**)puppetList + 2);
    if (0 == *((void**)firstNode + 2)) {
        unsigned int* p = kVec3NormalizeZeroFallback;
        unsigned int v0 = *p++;
        unsigned int v1 = *p++;
        unsigned int v2 = *p;
        *(unsigned int*)result = v0;
        *((unsigned int*)result + 1) = v1;
        *((unsigned int*)result + 2) = v2;
        return;
    }
    // Select the first firstPuppet's preferred/fallback bone, else matrix translation.
    firstPuppet = *(const char**)(*((void**)firstNode + 2));
    anchorBoneEntry = (void*)WPuppet_FindNamedBoneEntry(firstPuppet, puppetBoneExceptionStrings);
    if (0 == anchorBoneEntry)
        anchorBoneEntry = (void*)WPuppet_FindNamedBoneEntry(firstPuppet, 6 + puppetBoneExceptionStrings);

    if (0 != anchorBoneEntry && *(void**)anchorBoneEntry != 0)
        *(WVector*)accumulatedPosition = ((BonePositionView*)*(void**)anchorBoneEntry)->position;
    else
        *(WVector*)accumulatedPosition = (*(WVector*)&((WPuppet*)firstPuppet)->worldMatrix[9]);

    if (*(unsigned int*)puppetList > 1) {
        // Walk remaining links for the sample count, retaining the SAME firstPuppet.
        remainingNode = ((TextureListNode*)firstNode)->next;
        listSentinel = &((TextureItemQueueView*)puppetList)->headPrevious;
        while (listSentinel != remainingNode) {
            anchorBoneEntry = (void*)WPuppet_FindNamedBoneEntry(firstPuppet, puppetBoneExceptionStrings);
            if (anchorBoneEntry == 0)
                anchorBoneEntry = (void*)WPuppet_FindNamedBoneEntry(firstPuppet, 6 + puppetBoneExceptionStrings);
            WVector next;
            const void* source = (anchorBoneEntry != 0)
                                     ? (const void*)&((BonePositionView*)*(void**)anchorBoneEntry)->position
                                     : (const void*)&((WPuppet*)firstPuppet)->worldMatrix[9];
            next = *(WVector*)source;
            *(WVector*)accumulatedPosition += next;
            remainingNode = ((TextureListNode*)remainingNode)->next;
        }
        Vec3_DivideAssign(accumulatedPosition, (float)*(unsigned int*)puppetList);
    }
    int output[3];
    Vec3_CopyToOutput(accumulatedPosition, output);
    *(unsigned int*)result = output[0];
    *((unsigned int*)result + 1) = output[1];
    *((unsigned int*)result + 2) = output[2];
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621B4", _ctors_u801EDA60_0, TextureCapture_InitAngleConstant);
#pragma pop
