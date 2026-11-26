// Sprites and billboards: the W3dSpr sprite (owned bitmap, four vertices, color, size and
// position), the billboard and rotated-billboard renderers, the packed vertex helpers that
// read and scale positions and texture coordinates, and the round-player list setup.

#include <decomp/static_init.h>
#include <decomp/offset_of.h>
#include "MSL/list.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::list<void*, app::app_allocator<void*> > MSLPointerList;
#include "classes/W3dSpr.h"
#include "classes/W3dSprVertex.h"
#include "classes/CSceneManager.h"
#include "classes/WView.h"

#include "sdk_protos.h"
#include "game/wvector.h"

extern "C" {

enum {
    kQuadVertexCount = 4,
    kBitmapResourceMask = 0x7FF,
    kRequireProjectedQuadFlags = 0x06000000,
    kProjectedQuadDrawMode = 16,
    kBillboardFacingCodeBase = 6
};
enum BillboardFacing {
    kFaceEye = 0,
    kKeepWorldUp = 1,
    kUseViewAxes = 2
};
// FVF is an opaque packed format code here, not a float count. Only the XYZ/color/UV
// layouts below are established. Other spellings retain their hex code rather than
// inventing component names; the position-address switch documents their byte behavior.
// UvAtN names use float-lane indices for the FIRST UV pair; PositionOnly means
// extraction copies XYZ only, not that the format has no other attributes.
// Layout_hex retains an unresolved encoding; its address behavior is base+0.
enum SpriteVertexFormat {
    kFvfXYZ = 0x1,
    kFvfXYZAlt = 0x5,
    kFvfXYZColor = 0xF001,
    kFvfXYZColorAlt = 0xF005,
    kFvfXYZUv = 0x110001,
    kFvfXYZUvAlt = 0x110005,
    kFvfXYZColorUv = 0x11F001,
    kFvfXYZColorUvAlt = 0x11F005,
    // Names expose only behavior verified here; the remaining components are unknown.
    kFvfPositionOnly_0401 = 0x401,
    kFvfLayout_6005 = 0x6005,
    kFvfPositionOnly_F00D = 0xF00D,
    kFvfUvAt6_110401 = 0x110401,
    kFvfUvAt9_110541 = 0x110541,
    kFvfUvAt7_11F401 = 0x11F401,
    kFvfUvAt6_120401 = 0x120401,
    kFvfUvAt4_12F005 = 0x12F005,
    kFvfLayout_130005 = 0x130005,
    kFvfUvAt6_130401 = 0x130401,
    kFvfLayout_140401 = 0x140401,
    kFvfLayout_400521 = 0x400521,
    kFvfPositionOnly_40F121 = 0x40F121,
    kFvfUvAt8_510521 = 0x510521,
    kFvfUvAt9_510531 = 0x510531,
    kFvfUvAt10_510541 = 0x510541,
    kFvfUvAt8_520521 = 0x520521,
    kFvfUvAt9_520531 = 0x520531,
    kFvfUvAt10_520541 = 0x520541,
    kFvfUvAt8_530521 = 0x530521,
    kFvfUvAt9_530531 = 0x530531,
    kFvfUvAt10_530541 = 0x530541,
    kFvfPositionBytePlus1_800401 = 0x800401,
    kFvfPositionBytePlus1_910401 = 0x910401,
    kFvfPositionBytePlus1_920401 = 0x920401,
    kFvfPositionBytePlus1_930401 = 0x930401
};
enum VertexChunkKind {
    kScalarChunk = 0,
    kVector3Chunk = 1,
    kVector4Chunk = 2,
    kUnscaledChunk3 = 3,
    kUnscaledChunk4 = 4,
    kUnscaledChunk5 = 5,
    kPackedChunkKind6 = 6,
    kPackedChunkKind7 = 7,
    kPackedChunkKind8 = 8,
    kPackedChunkKind9 = 9
};

extern const float kFloat_6_2831855;
extern float s_spriteUnitTwoPiCache0;
extern float s_spriteUnitTwoPiCache2;
extern float s_spriteUnitTwoPiCache1;
extern char __vt__6W3dSpr[];
struct InitList {
    unsigned int count;
    void* next;
    void* prev;
};
unsigned char s_roundPlayerListDtorRegistration[12];
InitList s_roundPlayerList;
unsigned char s_shotActionRestrictionState[0x4c];
extern void* g_sceneManager;
extern int s_rotatedBillboardCornerSelectors[];
extern const float kBillboardPackedAngleScale1024;
extern const float kFloat_0_001953125_W3dSprite;
extern const float kFloat_0_9980469_ScreenCapture;
extern const float kFloat_0_EventPosition;
extern const float kFloat_1_ScreenCapture;
extern const double kSignedIntToDoubleBias_SpriteUnit;
extern const float kFloat_255_W3dSprite;
extern const float kFloat_0_5_SpriteTexture;
extern const float kFloat_0_00390625_SpriteUnit;

// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook0() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook1() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook2() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook3() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook4() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook5() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook6() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook7() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook8() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook9() {}
// Reserved no-op hook; this entry point performs no work.
void SpriteUnit_NoOpHook10() {}

// Return the input address, adding one byte for formats 0x800401/0x910401/0x920401/0x930401.
// The integer +1 is present in the original; it does not skip a 32-bit word.
// Units: vertexAddress is a 32-bit byte address carried in an int. Unknown codes
// also return it unchanged. The four adjusted codes must be passed on with +1 byte;
// neither this helper nor ExtractVertexPositionAndTexCoords skips a prefix word.
int FlexibleVertexFormat_GetPositionAddress(int vertexAddress, int vertexFormat) {
    switch (vertexFormat) {
    default:
        return vertexAddress;
    case kFvfXYZ:
    case kFvfXYZAlt:
        return vertexAddress;
    case kFvfPositionOnly_0401:
    case kFvfLayout_6005:
        return vertexAddress;
    case kFvfXYZColor:
    case kFvfXYZColorAlt:
        return vertexAddress;
    case kFvfPositionOnly_F00D:
    case kFvfXYZUv:
        return vertexAddress;
    case kFvfXYZUvAlt:
    case kFvfUvAt6_110401:
        return vertexAddress;
    case kFvfUvAt9_110541:
    case kFvfXYZColorUv:
        return vertexAddress;
    case kFvfXYZColorUvAlt:
    case kFvfUvAt7_11F401:
        return vertexAddress;
    case kFvfUvAt6_120401:
    case kFvfUvAt4_12F005:
        return vertexAddress;
    case kFvfLayout_130005:
    case kFvfUvAt6_130401:
        return vertexAddress;
    case kFvfLayout_140401:
    case kFvfLayout_400521:
        return vertexAddress;
    case kFvfPositionOnly_40F121:
    case kFvfUvAt8_510521:
        return vertexAddress;
    case kFvfUvAt9_510531:
    case kFvfUvAt10_510541:
        return vertexAddress;
    case kFvfUvAt8_520521:
        return vertexAddress;
    case kFvfUvAt9_520531:
        return vertexAddress;
    case kFvfUvAt10_520541:
        return vertexAddress;
    case kFvfUvAt8_530521:
        return vertexAddress;
    case kFvfUvAt9_530531:
        return vertexAddress;
    case kFvfUvAt10_530541:
        return vertexAddress;
    case kFvfPositionBytePlus1_800401:
        return vertexAddress + 1;
    case kFvfPositionBytePlus1_910401:
        return vertexAddress + 1;
    case kFvfPositionBytePlus1_920401:
        return vertexAddress + 1;
    case kFvfPositionBytePlus1_930401:
        return vertexAddress + 1;
    }
}

// Initialize this sprite helper's cached full-turn angle.
void SpriteUnit_InitTwoPiCache0() {
    s_spriteUnitTwoPiCache0 = kFloat_6_2831855;
}

extern void* ResourceManager_LoadBitmap(void*, void*, void*, int, int);
extern void ResourceManager_ReleaseId(void*, int);
extern void BaseObject_dtor(void*, int);
extern void __dl__FPv(void*);
extern void* List_DtorFreeNodes(void*, int);
extern void* memset(void*, int, unsigned int);
extern void appFreeUni__FPv(void*);
extern void* BaseObject_ctor(void*);

struct ListLinks {
    ListLinks* previous;
    ListLinks* next;
};
struct OwnedList {
    int count;
    ListLinks sentinel;
};

// Detach and free the circular list nodes, then optionally delete its header.
void* List_DtorFreeNodes(void* self, int flags) {
    return ::std::__list_deleting_dtor((MSLPointerList*)self, flags);
}

struct InitPair {
    int x, y;
};
extern const InitPair s_spriteQuadUvCornerSelectors[];

} // extern "C"

// Initialize bitmap ownership and the four embedded vertices with half-texel UVs.
void* W3dSpr::Init() {
    BaseObject_ctor(this);
    resourceManager = 0;
    vtbl = (void*)__vt__6W3dSpr;
    bitmap = 0;
    ownsBitmap = 0;
    char* point = (char*)vertices;
    W3dSpr* vertexSlotOwner = this;
    const InitPair* data = s_spriteQuadUvCornerSelectors;
    int idx = 0;
    for (int i = 0; i < kQuadVertexCount; ++i) {
        int y0 = data->y;
        int x0 = data->x;
        (*(char**)&vertexSlotOwner->vertex0) = point;
        point += sizeof(W3dSprVertex);
        vertexSlotOwner->vertex0->textureU = (kFloat_255_W3dSprite * (float)x0 + kFloat_0_5_SpriteTexture) / 256.0f;
        vertexSlotOwner->vertex0->textureV = (kFloat_255_W3dSprite * (float)y0 + kFloat_0_5_SpriteTexture) / 256.0f;
        vertexSlotOwner->vertex0->color = -1;
        vertexSlotOwner = (W3dSpr*)((char*)vertexSlotOwner + sizeof(void*));
        idx += 1;
        data += 1;
    }
    // Zero the word after the four vertex pointers (vertexListTerminator), indexed by the loop count.
    *(int*)((char*)this + idx * sizeof(void*) + 0x114) = 0;
    return this;
}

extern "C" {

// Scale only the position/vector lanes present in the selected packed vertex chunk.
// Float-lane map (indices, not bytes):
//   kind 0: [0]; 1: [0..2]; 2: [0..3]; 3/4/5: no stores
//   kind 6: [1], [3..4]; 7: [2..3], [6..9]
//   kind 8: [3..5], [9..14]; 9: [4..7], [12..19].
// All other lanes are opaque and preserved. Chunk storage must cover the highest
// selected lane; an unknown kind is a successful no-op (return is always 1).
int PetMesh_ScaleVertexChunkByKind(float* chunk, unsigned int kind, float scale) {
    switch (kind) {
    case kScalarChunk:
        chunk[0] *= scale;
        break;
    case kVector3Chunk:
        chunk[0] *= scale;
        chunk[1] *= scale;
        chunk[2] *= scale;
        break;
    case kVector4Chunk:
        chunk[0] *= scale;
        chunk[1] *= scale;
        chunk[2] *= scale;
        chunk[3] *= scale;
        break;
    case kPackedChunkKind6:
        chunk[4] *= scale;
        chunk[1] *= scale;
        chunk[3] *= scale;
        break;
    case kPackedChunkKind7:
        chunk[8] *= scale;
        chunk[9] *= scale;
        chunk[2] *= scale;
        chunk[3] *= scale;
        chunk[6] *= scale;
        chunk[7] *= scale;
        break;
    case kPackedChunkKind8:
        chunk[12] *= scale;
        chunk[13] *= scale;
        chunk[14] *= scale;
        chunk[3] *= scale;
        chunk[4] *= scale;
        chunk[5] *= scale;
        chunk[9] *= scale;
        chunk[10] *= scale;
        chunk[11] *= scale;
        break;
    case kPackedChunkKind9:
        chunk[16] *= scale;
        chunk[17] *= scale;
        chunk[18] *= scale;
        chunk[19] *= scale;
        chunk[4] *= scale;
        chunk[5] *= scale;
        chunk[6] *= scale;
        chunk[7] *= scale;
        chunk[12] *= scale;
        chunk[13] *= scale;
        chunk[14] *= scale;
        chunk[15] *= scale;
        break;
    case kUnscaledChunk3:
    case kUnscaledChunk4:
    case kUnscaledChunk5:

        break;
    default:
        break;
    }
    return 1;
}

// Extract XYZ and, when present and requested, the first UV pair from a packed vertex.
// vertex is borrowed float-addressed storage in the selected format. positionOut
// needs three floats; optional texCoordsOut needs two. Unsupported codes leave both
// outputs unchanged; recognized non-UV codes leave texCoordsOut unchanged.
void PetMesh_ExtractVertexPositionAndTexCoords(float* positionOut, float* texCoordsOut, float* vertex,
                                               int vertexFormat) {
    switch (vertexFormat) {
    case kFvfXYZ:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        return;
    case kFvfXYZColor:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        return;
    case kFvfXYZColorAlt:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        return;
    case kFvfXYZAlt:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        return;
    case kFvfXYZColorUvAlt:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[4];
        texCoordsOut[1] = vertex[5];
        return;
    case kFvfUvAt4_12F005:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[4];
        texCoordsOut[1] = vertex[5];
        return;
    case kFvfXYZUvAlt:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[3];
        texCoordsOut[1] = vertex[4];
        return;
    case kFvfXYZUv:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[3];
        texCoordsOut[1] = vertex[4];
        return;
    case kFvfPositionOnly_0401:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        return;
    case kFvfUvAt9_110541:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[9];
        texCoordsOut[1] = vertex[10];
        return;
    case kFvfUvAt7_11F401:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[7];
        texCoordsOut[1] = vertex[8];
        return;
    case kFvfXYZColorUv:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[4];
        texCoordsOut[1] = vertex[5];
        return;
    case kFvfUvAt6_110401:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[6];
        texCoordsOut[1] = vertex[7];
        return;
    case kFvfUvAt6_120401:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[6];
        texCoordsOut[1] = vertex[7];
        return;
    case kFvfUvAt6_130401:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[6];
        texCoordsOut[1] = vertex[7];
        return;
    case kFvfPositionOnly_40F121:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        return;
    case kFvfUvAt8_510521:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[8];
        texCoordsOut[1] = vertex[9];
        return;
    case kFvfUvAt8_520521:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[8];
        texCoordsOut[1] = vertex[9];
        return;
    case kFvfUvAt8_530521:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[8];
        texCoordsOut[1] = vertex[9];
        return;
    case kFvfUvAt9_510531:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[9];
        texCoordsOut[1] = vertex[10];
        return;
    case kFvfUvAt9_520531:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[9];
        texCoordsOut[1] = vertex[10];
        return;
    case kFvfUvAt9_530531:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[9];
        texCoordsOut[1] = vertex[10];
        return;
    case kFvfUvAt10_510541:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[10];
        texCoordsOut[1] = vertex[11];
        return;
    case kFvfUvAt10_520541:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[10];
        texCoordsOut[1] = vertex[11];
        return;
    case kFvfUvAt10_530541:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        if (texCoordsOut == 0)
            return;
        texCoordsOut[0] = vertex[10];
        texCoordsOut[1] = vertex[11];
        return;
    case kFvfPositionOnly_F00D:
        positionOut[0] = vertex[0];
        positionOut[1] = vertex[1];
        positionOut[2] = vertex[2];
        return;
    default:
        return;
    }
}

} // extern "C"

// Load and own the bitmap; return zero on success and one on failure.
int W3dSpr::LoadSprite(void* name, void* loadOptions) {
    void* p = ResourceManager_LoadBitmap(resourceManager, name, loadOptions, 0, 0);
    bitmap = p;
    if (p != 0) {
        ownsBitmap = 1;
        return 0;
    }
    return 1;
}

extern "C" {

// Initialize the round-player sentinel list, destructor registration and shot restrictions.
void RoundPlayerList_InitializeGlobals() {
    s_spriteUnitTwoPiCache1 = kFloat_6_2831855;
    void* dtor = reinterpret_cast<void*>(List_DtorFreeNodes);
    unsigned char* base = s_roundPlayerListDtorRegistration;
    InitList* obj = &s_roundPlayerList;
    void* p = &obj->next;
    void* b = base;
    obj->count = 0;
    obj->prev = p;
    obj->next = p;
    __register_global_object(obj, dtor, b);
    memset(s_shotActionRestrictionState, 0, sizeof(s_shotActionRestrictionState));
}

} // extern "C"

// Release an owned bitmap before the base object and optional allocation.
void* W3dSpr::Destroy(int flags) {
    if (this != 0) {
        unsigned char owns = ownsBitmap;
        vtbl = (void*)__vt__6W3dSpr;
        if (owns != 0) {
            void* rm = resourceManager;
            if (rm != 0) {
                void* bmp = bitmap;
                if (bmp != 0) {
                    ResourceManager_ReleaseId(rm, (int)bmp);
                    bitmap = 0;
                }
            }
        }
        if (this != 0) {
            BaseObject_dtor(this, 0);
        }
        if (flags > 0) {
            __dl__FPv(this);
        }
    }
    return this;
}

// Apply the same packed color to all four vertices.
void W3dSpr::SetColor(unsigned int value) {
    vertex0->color = value;
    vertex1->color = value;
    vertex2->color = value;
    vertex3->color = value;
}

// Convert dimensions and pivot into signed corner extents.
void W3dSpr::SetRect(float width, float height, float pivotX, float pivotY) {
    left = -pivotX;
    right = width - pivotX;
    top = -pivotY;
    bottom = height - pivotY;
}

// Copy the sprite world position.
void W3dSpr::SetPos(void* source) {
    position = *(const WVector*)source;
}

extern "C" {
extern void __as__7WVectorFRC7WVector(void*, void*);

extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void Vector_CrossProduct(void*, void*, void*);
extern void Normalize__7WVectorFv(void*);
extern float Math_CosApprox(float);
extern float Math_SinApprox(float);
extern void __ml__FRC7WVectorf(void*, void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void* Matrix_TransformPoint(void*, void*, void*);
extern void WView_ProjectViewPoint(void*, void*, void*, int);

struct BillboardVector {
    float x, y, z;
};

struct BillboardVertex {
    float uMin, vMin, uMax, vMax;
    int color;
    int packedRotation;
    float extentLeft, extentTop, extentRight, extentBottom;
    float positionX, positionY, positionZ;
    unsigned char facingCode;
    char pad35[3];
};

struct OrientedBillboard {
    char pad0[0x14];
    unsigned int bitmapId;
    float left, right, top, bottom;
    BillboardVector position;
    char pad34[0x114 - 0x34];
    BillboardVertex* vertex0;
    BillboardVertex* vertex1;
    BillboardVertex* vertex2;
    BillboardVertex* vertex3;
};

struct BillboardRenderer {
    virtual void vm0();
    virtual void vm1();
    virtual void vm2();
    virtual void vm3();
    virtual void drawProjectedQuad(void*, int, unsigned int, int);
    virtual void vm5();
    virtual void vm6();
    virtual void vm7();
    virtual void vm8();
    virtual void drawCompactBillboard(void*, unsigned int, int, unsigned int);
    // WView data layout; direct WView casts hoist matrix addresses across calls in this compiler.
    char pad_004[0x10];
    float viewTransform[12];
    float rightAxis[3];
    float upAxis[3];
    char pad_05C[0xC];
    float eyePosition[3];
};

typedef char BillboardView_inputOffset
    [((int)DECOMP_OFFSETOF(BillboardRenderer, viewTransform) == (int)DECOMP_OFFSETOF(WView, inputTransform)) ? 1 : -1];
typedef char BillboardView_eyeOffset
    [((int)DECOMP_OFFSETOF(BillboardRenderer, eyePosition) == (int)DECOMP_OFFSETOF(WView, viewMatrix[9])) ? 1 : -1];

// Submit a compact billboard when permitted, otherwise build and project its four corners.
// self/renderer and all four writable vertices are borrowed. facing must be one of
// BillboardFacing: there is no default basis. Extents are local world-unit offsets.
// The explicit path forms world = center + right*x + up*y, transforms world -> view,
// then projects into renderer vertex coordinates; compact drawing delegates that work.
void Billboard_DrawOrientedQuad(OrientedBillboard* self, BillboardRenderer* renderer, unsigned int drawFlags,
                                int facing) {
    unsigned int bitmapId = self->bitmapId;
    void* sceneManager = g_sceneManager;
    if (bitmapId == 0)
        return;
    if (sceneManager == 0 ||
        ((CSceneManager*)sceneManager)->effectRenderPassActive != 0) { // 0x8021ECB8: scene singleton.
        if ((drawFlags & kRequireProjectedQuadFlags) == 0) {
            self->vertex0->positionX = self->position.x;
            self->vertex0->positionY = self->position.y;
            self->vertex0->positionZ = self->position.z;
            self->vertex0->extentLeft = self->left;
            self->vertex0->extentTop = self->top;
            self->vertex0->extentRight = self->right;
            self->vertex0->extentBottom = self->bottom;
            self->vertex0->facingCode = (unsigned char)(facing + kBillboardFacingCodeBase);
            self->vertex0->packedRotation = 0;
            self->vertex0->uMin = kFloat_0_001953125_W3dSprite;
            self->vertex0->vMin = kFloat_0_001953125_W3dSprite;
            self->vertex0->uMax = kFloat_0_9980469_ScreenCapture;
            self->vertex0->vMax = kFloat_0_9980469_ScreenCapture;
            renderer->drawCompactBillboard(&self->vertex0, drawFlags, -1, bitmapId & kBitmapResourceMask);
            return;
        }
    }
    // Phase 2: build a world-space basis (eye-facing, world-up, or camera axes).
    BillboardVector rightAxis, upAxis, facingDirection;
    switch (facing) {
    case kFaceEye: {
        BillboardVector eyeDelta, rightCross;
        BillboardVector worldUp;
        BillboardVector upCross;
        __mi__FRC7WVectorRC7WVector(&eyeDelta, renderer->eyePosition, &self->position);
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&facingDirection, &eyeDelta);
        worldUp.x = kFloat_0_EventPosition;
        worldUp.y = kFloat_1_ScreenCapture;
        worldUp.z = kFloat_0_EventPosition;
        Vector_CrossProduct(&rightCross, &worldUp, &facingDirection);
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&rightAxis, &rightCross);
        (*(WVector*)&facingDirection).Normalize();
        (*(WVector*)&rightAxis).Normalize();
        Vector_CrossProduct(&upCross, &facingDirection, &rightAxis);
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&upAxis, &upCross);
        (*(WVector*)&upAxis).Normalize();
        break;
    }
    case kKeepWorldUp: {
        BillboardVector eyeDelta, rightCross;
        BillboardVector worldUp;
        BillboardVector upCross;
        __mi__FRC7WVectorRC7WVector(&eyeDelta, renderer->eyePosition, &self->position);
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&facingDirection, &eyeDelta);
        worldUp.x = kFloat_0_EventPosition;
        worldUp.y = kFloat_1_ScreenCapture;
        worldUp.z = kFloat_0_EventPosition;
        Vector_CrossProduct(&rightCross, &worldUp, &facingDirection);
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&rightAxis, &rightCross);
        (*(WVector*)&rightAxis).Normalize();
        upCross.x = kFloat_0_EventPosition;
        upCross.y = kFloat_1_ScreenCapture;
        upCross.z = kFloat_0_EventPosition;
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&upAxis, &upCross);
        break;
    }
    case kUseViewAxes:
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&rightAxis, renderer->rightAxis);
        ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(&upAxis, renderer->upAxis);
        break;
    }
    {
        // Phase 3: project corners in order bottom-left, bottom-right, top-right, top-left.
        BillboardVector viewCorner, worldCorner, horizontalPoint, horizontalOffset, verticalOffset;
        __ml__FRC7WVectorf(&verticalOffset, &upAxis, self->bottom);
        __ml__FRC7WVectorf(&horizontalOffset, &rightAxis, self->left);
        __pl__FRC7WVectorRC7WVector(&horizontalPoint, &self->position, &horizontalOffset);
        __pl__FRC7WVectorRC7WVector(&worldCorner, &horizontalPoint, &verticalOffset);
        Matrix_TransformPoint(&viewCorner, &worldCorner, renderer->viewTransform);
        WView_ProjectViewPoint(renderer, self->vertex0, &viewCorner, 0);
    }
    {
        BillboardVector viewCorner, worldCorner, horizontalPoint, horizontalOffset, verticalOffset;
        __ml__FRC7WVectorf(&verticalOffset, &upAxis, self->bottom);
        __ml__FRC7WVectorf(&horizontalOffset, &rightAxis, self->right);
        __pl__FRC7WVectorRC7WVector(&horizontalPoint, &self->position, &horizontalOffset);
        __pl__FRC7WVectorRC7WVector(&worldCorner, &horizontalPoint, &verticalOffset);
        Matrix_TransformPoint(&viewCorner, &worldCorner, renderer->viewTransform);
        WView_ProjectViewPoint(renderer, self->vertex1, &viewCorner, 0);
    }
    {
        BillboardVector viewCorner, worldCorner, horizontalPoint, horizontalOffset, verticalOffset;
        __ml__FRC7WVectorf(&verticalOffset, &upAxis, self->top);
        __ml__FRC7WVectorf(&horizontalOffset, &rightAxis, self->right);
        __pl__FRC7WVectorRC7WVector(&horizontalPoint, &self->position, &horizontalOffset);
        __pl__FRC7WVectorRC7WVector(&worldCorner, &horizontalPoint, &verticalOffset);
        Matrix_TransformPoint(&viewCorner, &worldCorner, renderer->viewTransform);
        WView_ProjectViewPoint(renderer, self->vertex2, &viewCorner, 0);
    }
    {
        BillboardVector viewCorner, worldCorner, horizontalPoint, horizontalOffset, verticalOffset;
        __ml__FRC7WVectorf(&verticalOffset, &upAxis, self->top);
        __ml__FRC7WVectorf(&horizontalOffset, &rightAxis, self->left);
        __pl__FRC7WVectorRC7WVector(&horizontalPoint, &self->position, &horizontalOffset);
        __pl__FRC7WVectorRC7WVector(&worldCorner, &horizontalPoint, &verticalOffset);
        Matrix_TransformPoint(&viewCorner, &worldCorner, renderer->viewTransform);
        WView_ProjectViewPoint(renderer, self->vertex3, &viewCorner, 0);
    }
    renderer->drawProjectedQuad(&self->vertex0, kProjectedQuadDrawMode,
                                (self->bitmapId & kBitmapResourceMask) | drawFlags, 0);
}

struct RotatedBillboardVertex {
    float uMin, vMin, uMax, vMax;
    char pad10[4];
    int packedRotation;
    float extentLeft, extentTop, extentRight, extentBottom, positionX, positionY, positionZ;
    unsigned char facingCode;
};
struct RotatedBillboard {
    char pad0[0x14];
    int bitmapId;
    float left, right, top, bottom, positionX, positionY, positionZ;
    char pad34[0x114 - 0x34];
    RotatedBillboardVertex* vertices[4];
};

struct RotatedBillboardRenderer;
struct RotatedBillboardRenderer {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void drawProjectedQuad(void*, int, unsigned int, int);
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void drawCompactBillboard(void*, unsigned int, int, unsigned int);
    char pad_004[0x10];
    float viewTransform[12];
    char viewAxes[0x24];
    float eyePosition[3];
};

// Rotate a camera-facing quad in its local plane and use compact or projected rendering.
// angle is in radians for sin/cos; compact submission converts it with the packed
// angle scale. Buffers below each hold one XYZ vector (12 bytes), passed through
// the external WVector ABI without changing their stack storage/alignment.
void Particle_DrawRotatedBillboard(RotatedBillboard* self, RotatedBillboardRenderer* renderer, float angle,
                                   unsigned int drawFlags) {
    unsigned int bitmapId = self->bitmapId;
    if (!bitmapId)
        return;
    if (0 == g_sceneManager || ((CSceneManager*)g_sceneManager)->effectRenderPassActive != 0) {
        if ((drawFlags & kRequireProjectedQuadFlags) == 0) {
            float scaled = kBillboardPackedAngleScale1024 * angle;
            __typeof__(self->positionX) savedPositionX = self->positionX;
            self->vertices[0]->positionX = savedPositionX;
            self->vertices[0]->positionY = self->positionY;
            self->vertices[0]->positionZ = self->positionZ;
            self->vertices[0]->extentLeft = self->left;
            self->vertices[0]->extentTop = self->top;
            self->vertices[0]->extentRight = self->right;
            self->vertices[0]->extentBottom = self->bottom;
            self->vertices[0]->facingCode = kBillboardFacingCodeBase;
            self->vertices[0]->packedRotation = Runtime_ConvertDoubleToUnsigned((double)scaled);
            float c1 = kFloat_0_9980469_ScreenCapture;
            float c0 = kFloat_0_001953125_W3dSprite;
            self->vertices[0]->uMin = c0;
            self->vertices[0]->vMin = c0;
            self->vertices[0]->uMax = c1;
            self->vertices[0]->vMax = c1;

            renderer->drawCompactBillboard(&self->vertices[0], drawFlags, -1, bitmapId & kBitmapResourceMask);
            return;
        }
    }
    // Explicit path: construct normalized world-space right/up from center to eye.
    char rightAxis[12], upAxis[12], facingDirection[12], eyeDelta[12], rightCross[12];
    float worldUp[3];
    unsigned char upCross[12];
    __mi__FRC7WVectorRC7WVector(eyeDelta, renderer->eyePosition, &self->positionX);
    unsigned char viewCorner[12], worldCorner[12], horizontalPoint[12], horizontalOffset[12], verticalOffset[12];
    ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(facingDirection, eyeDelta);
    worldUp[0] = kFloat_0_EventPosition;
    worldUp[1] = kFloat_1_ScreenCapture;
    worldUp[2] = kFloat_0_EventPosition;
    Vector_CrossProduct(rightCross, worldUp, facingDirection);
    ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(rightAxis, rightCross);
    Vector_CrossProduct(upCross, facingDirection, rightAxis);
    ((void (*)(void*, void*))__as__7WVectorFRC7WVector)(upAxis, upCross);
    Normalize__7WVectorFv(facingDirection);
    Normalize__7WVectorFv(rightAxis);
    Normalize__7WVectorFv(upAxis);
    float cosAngle = Math_CosApprox(angle);
    float sinAngle = Math_SinApprox(angle);
    // Rotate local (x,y), then center + right*rotatedX + up*rotatedY -> view -> projected.
    int* cornerSelectors = s_rotatedBillboardCornerSelectors;
    // This moving object view addresses vertices[i] only. Advancing it by a pointer
    // selects the next vertex slot; it must never be used for other object fields.
    RotatedBillboard* vertexSlotOwner = self;
    for (int i = 0; kQuadVertexCount > i; ++i) {
        float yForX = cornerSelectors[1] ? self->top : self->bottom;
        float xForX = cornerSelectors[0] ? self->left : self->right;
        float rotatedX = cosAngle * xForX - (sinAngle * yForX);
        float yForY = cornerSelectors[1] ? self->top : self->bottom;
        float xForY = cornerSelectors[0] ? self->left : self->right;
        float rotatedY = sinAngle * xForY + cosAngle * yForY;

        __ml__FRC7WVectorf(verticalOffset, upAxis, rotatedY);
        __ml__FRC7WVectorf(horizontalOffset, rightAxis, rotatedX);
        __pl__FRC7WVectorRC7WVector(horizontalPoint, &self->positionX, horizontalOffset);
        __pl__FRC7WVectorRC7WVector(worldCorner, horizontalPoint, verticalOffset);
        Matrix_TransformPoint(viewCorner, worldCorner, &renderer->viewTransform);
        WView_ProjectViewPoint(renderer, vertexSlotOwner->vertices[0], viewCorner, 0);
        cornerSelectors += 2;
        vertexSlotOwner = (RotatedBillboard*)((char*)vertexSlotOwner + sizeof(void*));
    }
    renderer->drawProjectedQuad(self->vertices, kProjectedQuadDrawMode,
                                (self->bitmapId & kBitmapResourceMask) | drawFlags, 0);
}

} // extern "C"

// Assign a bitmap id without releasing the old bitmap or changing ownsBitmap.
// Caller must arrange the ownership flag/old-resource cleanup before replacement.
int W3dSpr::AttachTexture(unsigned int value) {
    bitmap = (void*)value;
    return 1;
}

// Borrow the current bitmap resource; this accessor transfers no ownership.
void* W3dSpr::GetBitmap() {
    return bitmap;
}

extern "C" {

// Initialize this sprite helper's cached full-turn angle.
void SpriteUnit_InitTwoPiCache2() {
    s_spriteUnitTwoPiCache2 = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046220C", _ctors_u8021DA94_0, SpriteUnit_InitTwoPiCache0);
STATIC_INIT_ENTRY(".CRT$XCU8046220C", _ctors_u8021DA94_1, RoundPlayerList_InitializeGlobals);
STATIC_INIT_ENTRY(".CRT$XCU8046220C", _ctors_u8021DA94_2, SpriteUnit_InitTwoPiCache2);
#pragma pop
