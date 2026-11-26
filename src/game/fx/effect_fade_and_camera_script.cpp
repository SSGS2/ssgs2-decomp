// Screen fade and camera scripts: the full-screen fade quad, the CAM_SET camera commands that
// play through CEventCameraSC, and the camera-file loader with its byte-vector helpers.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
#include "iff/IffStruct.h"

// Normal destruction remains explicit. This owner handles exceptional exits.
struct EH5Cleanup {
    void* object;
    void (*destroy)(void*, int);
    bool active;
    // Arm exceptional cleanup for borrowed local storage.
    EH5Cleanup(void* p, void (*d)(void*, int)) : object(p), destroy(d), active(true) {}
    // Disarm before the explicit normal-exit destructor call.
    void release() {
        active = false;
    }
    // Destroy the local object only if an exception bypasses explicit cleanup.
    ~EH5Cleanup() {
        if (active)
            destroy(object, -1);
    }
};

#include "classes/ByteVectorState.h"
struct CameraPathOwnerView {
    char reserved00[0x14];
    ByteVectorState path;
};
#include "classes/CSharedDoc.h"
#include "classes/CHashPath.h"

#include "sdk_protos.h"
#include "game/wvector.h"
#define FIELD_OFFSET(type, field) ((int)&((type*)0)->field)
struct CameraSetStringPool {
    char emptyPrefix[1];
    char extension[5];
    char directory[21];
};
struct CameraSetCommandInput {
    unsigned short opcode, groupIndex;
    char fileName[1];
};
enum CameraSetOffsets {
    kCameraExtension = FIELD_OFFSET(CameraSetStringPool, extension),
    kCameraDirectory = FIELD_OFFSET(CameraSetStringPool, directory),
    kCameraFileName = FIELD_OFFSET(CameraSetCommandInput, fileName),
    kCameraGroupIndex = FIELD_OFFSET(CameraSetCommandInput, groupIndex)
};
struct CameraSampleView {
    WVector position, target, up;
    float fieldOfView;
};
// World-space basis/translation in the engine's 3x4 affine matrix order.
struct CameraAffineFrameView {
    WVector right, up, forward, position;
};
enum CameraAffineOffset {
    kCameraUpOffset = FIELD_OFFSET(CameraAffineFrameView, up),
    kCameraForwardOffset = FIELD_OFFSET(CameraAffineFrameView, forward),
    kCameraPositionOffset = FIELD_OFFSET(CameraAffineFrameView, position),
    kCameraPlaybackComplete = 1
};
struct CameraFrameScratchView {
    char reserved00[8];
    WVector correctedUp, right, forward;
    char reserved2C[4];
    int playbackWords[3];
    char reserved3C[4];
    WVector target, position;
    CameraSampleView sample;
    char reserved80[0x10];
};
// Fade ownership/color map: owner word is ARGB in big-endian memory; every
// screen vertex stores XYZ followed by RGBA. Vertices are TL, TR, BL, BR in a
// triangle strip. Keep individual channel stores to retain code and write order.
enum EffectRenderConstants {
    kQuadTriangleStrip = 0x98,
    kScreenColorVertexFormat = 0xF005,
    kScreenColorVertexStride = 0x10
};
enum ByteGrowthLimit {
    kThreeFifthsGrowthLimit = 0x55555555u,
    kHalfGrowthLimit = 0xaaaaaaaau
};

#include "game/eventconte.h"
#include "classes/CEffectFade.h"
#include "classes/CEventCameraSC.h"
#include "classes/CShopProductParts.h"
#include "classes/CVCGPlayer.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CVCGFile.h"
#include "classes/CVCGSource.h"
#include "classes/CFile.h"
extern "C" void* Destroy__Q23gtl5CFileFi(void*, int);

#define AsCEffectFade(p) ((CEffectFade*)(p))

extern "C" {

extern void __dl__FPv(void*);
extern void ShopPartsPointerArray_dtor(void*, int);
extern void ShopPartsPointerArray_Initialize(void*, void*);
extern int SaveData_TestPartFlag(void*, unsigned int, int);
extern void* ItemInventory_FindPartData(void*, unsigned int);
extern void appFreeUni__FPv(void*);
extern void __dt__10CVCGSourceFv(void*, int);
extern void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, char*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void* __nw__FUl(unsigned int);
extern void CameraMotionGroupVector_Clear(void*);
extern int GameHook_ReservedFalse(void); // Select loose camera files when enabled; retail hook returns zero.
extern void* SharedFile_Create(const char*, int);
extern void* CSharedFilePtr_GetLoadedBuffer(void*);
extern void GroupVector_GetCount(void*, unsigned int*);
extern void CameraScript_ParseGroupData(void*, void*);
extern void* CameraScript_LoadFileByPath(void*, const char*);
extern void UIRender_SetDefaultState(void);
extern void Engine_SetTextureStageResource(int, int);
extern void Engine_SetTexturePalette(int, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetType7Resource(int);
extern void DrawPrimitiveUP(int, int, void*, int);
extern unsigned int __vt__8CVCGFile[16];
extern unsigned int __vt__11CEffectFade[16];
extern unsigned int __vt__17CShopProductParts[16];
extern unsigned int __vt__22CEventCameraSC_CAM_SET[16];
extern const char eventCameraSetStrings[];
extern CSharedDoc* g_sharedDoc;
extern float kFloat_640_EffectFade;
extern const float kFloat_0_FadeState;
extern float kFloat_480_EffectFadeScreenHeight;
extern void* GX_GetPrimitiveCount(unsigned int, unsigned int);
extern void ByteVector_ResizeZeroFilled(void*, unsigned int);
extern int Vec3_CopySourceToDestination(void*, void*);
extern void Matrix_SetIdentityAffine(void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern float Normalize__7WVectorFv(void*);
extern void Vector_CrossProduct(void*, void*, void*);
extern void WView_SetViewMatrix(void*, void*);
extern void Camera_SetFov(void*, float);
extern unsigned int s_cameraScriptPathTerminator;
extern unsigned char s_cameraTransform[];
extern void* g_view;

#pragma push
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Return a provided fade-quad coordinate, or zero for an absent coordinate.
inline float FadeCoordinate(const float* value) {
    if (value)
        return *value;
    return kFloat_0_FadeState;
}
// Initialize the full-screen fade quad with zero opacity and a 640 by 480 extent.
CEffectFade::CEffectFade() {
    unsigned char* fade = (unsigned char*)this;
    AsCEffectFade(fade)->vtbl = (unsigned int)&__vt__11CEffectFade;
    float f0 = *(const volatile float*)&kFloat_0_FadeState;
    float f1;
    const float* value;
    value = 0;
    f1 = FadeCoordinate(value);
    AsCEffectFade(fade)->topLeftX = f1;
    value = 0;
    f1 = FadeCoordinate(value);
    AsCEffectFade(fade)->topLeftY = f1;
    f1 = kFloat_640_EffectFade;
    AsCEffectFade(fade)->topLeftZ = f0;
    AsCEffectFade(fade)->topRightX = f1;
    value = 0;
    f1 = FadeCoordinate(value);
    AsCEffectFade(fade)->topRightY = f1;
    AsCEffectFade(fade)->topRightZ = f0;
    value = 0;
    f1 = FadeCoordinate(value);
    AsCEffectFade(fade)->bottomLeftX = f1;
    f1 = kFloat_480_EffectFadeScreenHeight;
    AsCEffectFade(fade)->bottomLeftY = f1;
    f1 = kFloat_640_EffectFade;
    AsCEffectFade(fade)->bottomLeftZ = f0;
    AsCEffectFade(fade)->bottomRightX = f1;
    f1 = kFloat_480_EffectFadeScreenHeight;
    AsCEffectFade(fade)->bottomRightY = f1;
    int primitiveType = kQuadTriangleStrip;
    AsCEffectFade(fade)->bottomRightZ = f0;
    AsCEffectFade(fade)->primitiveCount = GX_GetPrimitiveCount(4, primitiveType);
    *(unsigned int*)fade = 0;
    unsigned char ownerAlpha, ownerRed, ownerGreen, ownerBlue;
    ownerBlue = AsCEffectFade(fade)->blue;
    ownerGreen = AsCEffectFade(fade)->green;
    ownerRed = AsCEffectFade(fade)->red;
    ownerAlpha = AsCEffectFade(fade)->alpha;
    AsCEffectFade(fade)->topLeftBlue = ownerBlue;
    AsCEffectFade(fade)->topLeftGreen = ownerGreen;
    AsCEffectFade(fade)->topLeftRed = ownerRed;
    AsCEffectFade(fade)->topLeftAlpha = ownerAlpha;
    AsCEffectFade(fade)->topRightBlue = ownerBlue;
    AsCEffectFade(fade)->topRightGreen = ownerGreen;
    AsCEffectFade(fade)->topRightRed = ownerRed;
    AsCEffectFade(fade)->topRightAlpha = ownerAlpha;
    AsCEffectFade(fade)->bottomLeftBlue = ownerBlue;
    AsCEffectFade(fade)->bottomLeftGreen = ownerGreen;
    AsCEffectFade(fade)->bottomLeftRed = ownerRed;
    AsCEffectFade(fade)->bottomLeftAlpha = ownerAlpha;
    AsCEffectFade(fade)->bottomRightBlue = ownerBlue;
    AsCEffectFade(fade)->bottomRightGreen = ownerGreen;
    AsCEffectFade(fade)->bottomRightRed = ownerRed;
    AsCEffectFade(fade)->bottomRightAlpha = ownerAlpha;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma pop

struct CameraFileSourceView {
    unsigned int vtable;
    unsigned char unk4[0x10];
    unsigned int sourceBuffer;
    unsigned int sourceSize;
    unsigned int sourceCapacity;
};

struct ShopPartSourceView {
    unsigned int vtable;
    unsigned char data[0x12C];
    unsigned int mode;
};
struct ShopPartBaseView {
    unsigned int vtable;
};
struct ByteVectorSizeView {
    unsigned char unk0[4];
    unsigned int value;
};

struct ByteVectorDataView {
    unsigned int value;
};

struct ByteVectorClearView {
    unsigned char unk0[4];
    unsigned int value;
};

struct ByteVectorWords {
    unsigned int value0;
    unsigned int value4;
    unsigned int value8;
};

struct CameraPathFlagBytes {
    union {
        unsigned int word;
        unsigned char bytes[4];
    };
};
struct CameraSetCommandStorage {
    unsigned int vtable, flags, length;
    char* heap;
    void* resource;
};
struct CameraPathStorage {
    unsigned int flags;
    unsigned int length;
    char* heap;
};
struct CameraSetConstructionScratch {
    char f0[8];
    char f1[8];
    char f2[8];
    char f3[8];
    char f4[8];
    unsigned int count;
    CameraPathStorage path;
};

struct CameraAnimationDestroyInterface {
    virtual void v0(int);
};

struct ImmediateOverlayView {
    unsigned char enabled;
    unsigned char pad[3];
    unsigned char vertices[0x40];
    unsigned int count;
};
struct ByteVectorDestroyView {
    void* object;
    volatile unsigned int clear;
};
struct CameraFileDestroyView {
    unsigned char unk[0x14];
    void* object;
    volatile unsigned int clear;
};

extern void VcgByteVector_Init(ByteVectorWords*);
extern unsigned int VcgByteVector_GetSize(ByteVectorSizeView*);
extern unsigned int VcgByteVector_GetDataAtOffset(ByteVectorDataView*, unsigned int);
extern void VcgByteVector_ClearSize(ByteVectorClearView*);
extern void* VcgByteVector_dtor(ByteVectorDestroyView*, int);
extern void ByteVector_AppendByte(unsigned char*, const char*);

// The quad owns no separate allocations, so there is nothing to release before the optional delete.
CEffectFade::~CEffectFade() {}

// Destroy the base part collection, then optionally free the shop filter.
CShopProductParts::~CShopProductParts() {
    ShopPartsPointerArray_dtor(this, 0);
}

// Clear owner ARGB and propagate transparent black to all four RGBA vertex colors.
void CEffectFade::ClearColor() {
    *(unsigned int*)this = 0;
    unsigned char ownerAlpha, ownerRed, ownerGreen, ownerBlue;
    ownerBlue = blue;
    ownerGreen = green;
    ownerRed = red;
    ownerAlpha = alpha;
    topLeftBlue = ownerBlue;
    topLeftGreen = ownerGreen;
    topLeftRed = ownerRed;
    topLeftAlpha = ownerAlpha;
    topRightBlue = ownerBlue;
    topRightGreen = ownerGreen;
    topRightRed = ownerRed;
    topRightAlpha = ownerAlpha;
    bottomLeftBlue = ownerBlue;
    bottomLeftGreen = ownerGreen;
    bottomLeftRed = ownerRed;
    bottomLeftAlpha = ownerAlpha;
    bottomRightBlue = ownerBlue;
    bottomRightGreen = ownerGreen;
    bottomRightRed = ownerRed;
    bottomRightAlpha = ownerAlpha;
}

// Replace RGB while retaining owner alpha, then update all four vertex colors.
void FadeQuad_SetRgb(unsigned char* fade, unsigned int value) {
    // The owner word is ARGB in big-endian memory: byte 0 alpha (kept), 1 red, 2 green, 3 blue.
    unsigned int t = AsCEffectFade(fade)->alpha;
    t <<= 24;
    t = (value & 0x00FFFFFF) | (t & 0xFF000000);
    *(unsigned int*)fade = t;
    unsigned char ownerAlpha, ownerRed, ownerGreen, ownerBlue;
    ownerBlue = AsCEffectFade(fade)->blue;
    ownerGreen = AsCEffectFade(fade)->green;
    ownerRed = AsCEffectFade(fade)->red;
    ownerAlpha = AsCEffectFade(fade)->alpha;
    AsCEffectFade(fade)->topLeftBlue = ownerBlue;
    AsCEffectFade(fade)->topLeftGreen = ownerGreen;
    AsCEffectFade(fade)->topLeftRed = ownerRed;
    AsCEffectFade(fade)->topLeftAlpha = ownerAlpha;
    AsCEffectFade(fade)->topRightBlue = ownerBlue;
    AsCEffectFade(fade)->topRightGreen = ownerGreen;
    AsCEffectFade(fade)->topRightRed = ownerRed;
    AsCEffectFade(fade)->topRightAlpha = ownerAlpha;
    AsCEffectFade(fade)->bottomLeftBlue = ownerBlue;
    AsCEffectFade(fade)->bottomLeftGreen = ownerGreen;
    AsCEffectFade(fade)->bottomLeftRed = ownerRed;
    AsCEffectFade(fade)->bottomLeftAlpha = ownerAlpha;
    AsCEffectFade(fade)->bottomRightBlue = ownerBlue;
    AsCEffectFade(fade)->bottomRightGreen = ownerGreen;
    AsCEffectFade(fade)->bottomRightRed = ownerRed;
    AsCEffectFade(fade)->bottomRightAlpha = ownerAlpha;
}

// Replace owner alpha and copy it with the existing RGB to all four vertices.
void CEffectFade::SetAlpha(unsigned char value) {
    unsigned char ownerRed, ownerGreen, ownerBlue;
    ownerBlue = blue;
    ownerGreen = green;
    ownerRed = red;
    alpha = value;
    topLeftBlue = ownerBlue;
    topLeftGreen = ownerGreen;
    topLeftRed = ownerRed;
    topLeftAlpha = value;
    topRightBlue = ownerBlue;
    topRightGreen = ownerGreen;
    topRightRed = ownerRed;
    topRightAlpha = value;
    bottomLeftBlue = ownerBlue;
    bottomLeftGreen = ownerGreen;
    bottomLeftRed = ownerRed;
    bottomLeftAlpha = value;
    bottomRightBlue = ownerBlue;
    bottomRightGreen = ownerGreen;
    bottomRightRed = ownerRed;
    bottomRightAlpha = value;
}

// Initialize a camera-motion file source with an empty buffer.
CVCGFile::CVCGFile() {
    CameraFileSourceView* source = (CameraFileSourceView*)this;
    ((CVCGSource*)source)->__ct();
    source->sourceBuffer = 0;
    source->vtable = (unsigned int)&__vt__8CVCGFile;
    source->sourceSize = 0;
    source->sourceCapacity = 0;
}

// Copy the part-source parameters, save filter mode and invoke its setup hook.
CShopProductParts::CShopProductParts(const unsigned int* src, unsigned int mode, void* initArg, void* dispatchArg) {
    ShopPartSourceView* self = (ShopPartSourceView*)this;
    ShopPartsPointerArray_Initialize(self, initArg);
    self->vtable = (unsigned int)&__vt__17CShopProductParts;
    struct Copy {
        unsigned int words[0x47];
    };
    *(Copy*)((unsigned char*)self + FIELD_OFFSET(CameraPathOwnerView, path)) = *(const Copy*)src;
    self->mode = mode;
    struct EA8Obj {
        virtual void f0(void*) DECOMP_LOCAL_VIRTUAL;
        virtual void f1(void*) DECOMP_LOCAL_VIRTUAL;
        virtual void f2(void*) DECOMP_LOCAL_VIRTUAL;
        virtual void f3(void*) DECOMP_LOCAL_VIRTUAL;
        virtual void f4(void*) DECOMP_LOCAL_VIRTUAL;
        virtual void f5(void*) DECOMP_LOCAL_VIRTUAL;
    };
    ((EA8Obj*)self)->f5(dispatchArg);
}

// Accept owned parts for the selected character whose equipment slot is allowed by the mask.
int CShopProductParts::AcceptsPart(IFF_STRUCT::sPart* arg, int test) {
    unsigned int mask;
    unsigned int field = filterFlags;
    unsigned int flag = arg->common.typeId;
    int found = SaveData_TestPartFlag(saveData, flag, test);
    mask = partTypeMask;
    unsigned int eligible;
    void* obj;
    if (((flag >> 9) & 3) != 0 || ((flag >> 18) & 0xFF) != (field & 0x03FFFFFF)) {
        eligible = 0;
    } else if ((obj = ItemInventory_FindPartData(((CSharedDoc*)g_sharedDoc)->itemInventory, flag)) != 0 // +0xA04
               && (mask & (1u << ((IFF_STRUCT::sPart*)obj)->partType)) != 0) {
        eligible = 1;
    } else {
        eligible = 0;
    }
    return eligible && found;
}

// Return the current fade opacity byte.
unsigned char CEffectFade::GetAlpha() {
    return alpha;
}

#pragma push
#pragma use_lmw_stmw on
// Read an aligned camera-script file, parse its groups and remember its null-terminated path.
// key is a borrowed NUL-terminated path. Local aligned bytes are temporary;
// group parsing must retain/copy what it needs before resource cleanup. The return
// is the underlying read result, or null on open failure; no extra read validation.
void* CameraScript_LoadFileByPath(void* self, const char* key) {
    ByteVectorWords resource;
    unsigned char string[0x20];
    VcgByteVector_Init((ByteVectorWords*)&resource);
    EH5Cleanup resourceCleanup(&resource, (void (*)(void*, int))VcgByteVector_dtor);
    ((gtl::CFile*)string)->Init();
    EH5Cleanup fileCleanup(string, (void (*)(void*, int))Destroy__Q23gtl5CFileFi);
    void* item = CHashPath::FindPath(key);
    if (!((gtl::CFile*)string)->Open((const char*)item, 0x10000)) {
        {
            fileCleanup.release();
            ((gtl::CFile*)string)->Destroy(-1);
        }
        {
            resourceCleanup.release();
            VcgByteVector_dtor((ByteVectorDestroyView*)&resource, -1);
        }
        return 0;
    }
    // Phase 2: round read length up to 32 bytes and parse before local storage dies.
    unsigned int size = (((gtl::CFile*)string)->GetFileSize() + 0x1F) & ~31;
    ByteVector_ResizeZeroFilled(&resource, size);
    unsigned int length = VcgByteVector_GetSize((ByteVectorSizeView*)&resource);
    void* pos = (void*)VcgByteVector_GetDataAtOffset((ByteVectorDataView*)&resource, 0);
    void* result = (void*)((gtl::CFile*)string)->Read(pos, length);
    ((gtl::CFile*)string)->Close();
    pos = (void*)VcgByteVector_GetDataAtOffset((ByteVectorDataView*)&resource, 0);
    CameraScript_ParseGroupData(self, pos);
    // Phase 3: keep a separate, explicitly terminated copy of the source path.
    VcgByteVector_ClearSize((ByteVectorClearView*)((unsigned char*)self + FIELD_OFFSET(CameraPathOwnerView, path)));
    int i = 0;
    while (key[i]) {
        ByteVector_AppendByte((unsigned char*)self + FIELD_OFFSET(CameraPathOwnerView, path), key + i);
        ++i;
    }
    ByteVector_AppendByte((unsigned char*)self + FIELD_OFFSET(CameraPathOwnerView, path),
                          (const char*)&s_cameraScriptPathTerminator);
    {
        fileCleanup.release();
        ((gtl::CFile*)string)->Destroy(-1);
    }
    {
        resourceCleanup.release();
        VcgByteVector_dtor((ByteVectorDestroyView*)&resource, -1);
    }
    return result;
}
#pragma pop

// Return the byte-vector element count.
unsigned int VcgByteVector_GetSize(ByteVectorSizeView* arg) {
    return arg->value;
}

extern const float kFloat_6_2831855;
extern float s_effectFadeFullTurnAngle;
extern float s_cameraSetCommandFullTurnAngle;
extern float s_cameraPlaybackFullTurnAngle;
extern float s_immediateOverlayFullTurnAngle;
extern float s_vcgFileFullTurnAngle;
extern unsigned int s_vcgAllocatorCallbacks[2];
extern unsigned int __vt__Q23std11logic_error[];
extern unsigned int s_sharedCharDeleterVtable[];
extern unsigned int __vt__Q23std12length_error[];
extern const char vectorLengthExceptionStrings_ResourceList[];
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void* memset(void*, int, unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
struct DtorVtable {
    unsigned int unk;
    void (*destroy)(void*);
};

extern void __dt__Q23std12length_errorFv(void*);
struct VectorErrorMessageOwner {
    unsigned int type;
    unsigned int refs;
    unsigned int controlWord08;
    char* data;
};
// Live growth transaction: hold replacement storage until the final ownership swap.
// On commit, heldBuffer/heldCapacity become the old allocation to release.
struct ByteVectorGrowthTransactionView {
    void* heldBuffer;
    unsigned int constructedBytes;
    unsigned int heldCapacity;
    void* capacityAddress;
    unsigned int prefixBytes;
};

struct VectorGrowthScratch {
    unsigned char empty2; // 0x08
    unsigned char pad00[3];
    unsigned char empty1; // 0x0c
    unsigned char pad01[3];
    unsigned int required; // 0x10
    unsigned int candB;    // 0x14
    unsigned int candA;    // 0x18
    unsigned int pad02;    // 0x1c
    char* tmp2data;        // 0x20
    void* tmp2end;         // 0x24
    char* tmp1data;        // 0x28
    void* tmp1end;         // 0x2c
    unsigned int err2vptr; // 0x30
    char* err2data;        // 0x34
    void* err2rep;         // 0x38
    unsigned int err1vptr; // 0x3c
    char* err1data;        // 0x40
    void* err1rep;         // 0x44
    union {
        struct {
            void* guard0;        // 0x48
            unsigned int guard1; // 0x4c
            unsigned int guard2; // 0x50
            void* guard3;        // 0x54
            unsigned int guard4; // 0x58
        };
        ByteVectorGrowthTransactionView growth;
    };
    unsigned int pad03[3]; // 0x5c
};

// Return the larger capacity requirement without changing reference identity.
inline const unsigned int& LargerByteCapacity(const unsigned int& stepCapacity, const unsigned int& extraCapacity) {
    return stepCapacity < extraCapacity ? extraCapacity : stepCapacity;
}

// Add max(extraCapacity, floor(3*(cap+1)/5)) below UINT_MAX/3;
// then max(extraCapacity, floor((cap+1)/2)) below 2*UINT_MAX/3; otherwise UINT_MAX.
// Outputs are arithmetic scratch: only the selected branch assigns its step.
// Callers check extraCapacity <= UINT_MAX-cap before applying this policy.
inline unsigned int ComputeByteVectorGrowthCapacity(unsigned int cap, const unsigned int& extraCapacity,
                                                    unsigned int& threeFifthsStep, unsigned int& halfStep) {
    if (cap < kThreeFifthsGrowthLimit) {
        threeFifthsStep = 3u * (cap + 1u) / 5u;
        return cap + LargerByteCapacity(threeFifthsStep, extraCapacity);
    } else if (cap < kHalfGrowthLimit) {
        halfStep = (cap + 1u) / 2u;
        return cap + LargerByteCapacity(halfStep, extraCapacity);
    }
    return 0xffffffffu;
}

// Compute a growth capacity using the original allocation-growth policy.
inline unsigned int ProbeByteVectorGrowthCapacity(unsigned int cap, const unsigned int& required) {
    unsigned int threeFifthsStep, halfStep;
    return ComputeByteVectorGrowthCapacity(cap, required, threeFifthsStep, halfStep);
}

// Return the byte distance between two buffer positions.
inline unsigned int ByteRangeLength(const unsigned char* begin, const unsigned char* end) {
    return end - begin;
}

// Delete a temporary exception-message character allocation.
inline void DeleteTemporaryCharacterBuffer(char* data) {
    if (data)
        __dla__FPv(data);
}

struct VectorLengthErrorStorage {
    unsigned int type;
    char* data;
    void* rep;
    // Install the logic_error base vtable before constructing its message.
    inline VectorLengthErrorStorage* initializeBase() {
        type = (unsigned int)__vt__Q23std11logic_error;
        return this;
    }
    inline VectorLengthErrorStorage* initialize(unsigned char& empty, char*& temporary, void*& end);
};

// Build an owned length-error message and its shared deleter control block.
inline VectorLengthErrorStorage* VectorLengthErrorStorage::initialize(unsigned char& empty, char*& temporary,
                                                                      void*& end) {
    const char* msg = vectorLengthExceptionStrings_ResourceList;
    VectorLengthErrorStorage* error = initializeBase();
    empty = 0;
    unsigned int n = strlen(msg) + 1;
    char* buffer = (char*)__nwa__FUl(n * 2 - n);
    data = buffer;
    temporary = buffer;
    end = &empty;
    VectorErrorMessageOwner* rep;
    try {
        rep = (VectorErrorMessageOwner*)__nw__FUl(0x10);
    } catch (...) {
        __dla__FPv(buffer);
        throw;
    }
    if (rep) {
        rep->refs = 1;
        rep->controlWord08 = 1;
        rep->type = (unsigned int)s_sharedCharDeleterVtable;
        rep->data = buffer;
    }
    this->rep = rep;
    temporary = 0;
    DeleteTemporaryCharacterBuffer(temporary);
    strcpy(data, msg);
    type = (unsigned int)__vt__Q23std12length_error;
    return error;
}

#pragma push
#pragma use_lmw_stmw on

// Resize the byte vector, growing capacity when needed and zero-filling added bytes.
// Vector is {data,size,capacity}; callbacks[0] allocates and [1] frees.
// Growth allocates replacement storage, writes new zero bytes, copies the old
// prefix, clears its old bytes, swaps ownership, then frees the old allocation.
// Shrinking changes size only; truncated bytes and capacity remain unchanged.
void ByteVector_ResizeZeroFilled(void* selfPtr, unsigned int requestedBytes) {
    unsigned char* self = (unsigned char*)selfPtr;
    VectorGrowthScratch operation;
    unsigned int replacementCapacity;
    unsigned int size = AsByteVectorState(self)->size;
    if (requestedBytes > size) {
        unsigned int growth = requestedBytes - size;
        unsigned int cap = AsByteVectorState(self)->capacity;
        if (growth > cap || size > cap - growth) {
            unsigned int newSize = *(volatile unsigned int*)(self + 4) + growth;
            unsigned int required = newSize - cap;
            unsigned int previousCapacity = *(volatile unsigned int*)&AsByteVectorState(self)->capacity;
            if (required > 0xffffffffu - previousCapacity) {
                VectorLengthErrorStorage* error =
                    ((VectorLengthErrorStorage*)&operation.err1vptr)
                        ->initialize(operation.empty1, operation.tmp1data, operation.tmp1end);
                __throw(vectorLengthExceptionStrings_ResourceList + 0x14, error, (void*)__dt__Q23std12length_errorFv);
            }
            ProbeByteVectorGrowthCapacity(previousCapacity, required);
        } else {
            memset((unsigned char*)*(void**)self + size, 0, growth);
            *(unsigned int*)(self + 4) += growth;
            return;
        }
        // Reserve a transaction before changing vector ownership.
        unsigned int required2 = *(unsigned int*)(self + 4) + growth - *(unsigned int*)(self + 8);
        unsigned int* capacityAddress = (unsigned int*)(self + 8);
        operation.growth.heldBuffer = 0;
        operation.required = required2;
        unsigned int previousCapacity = *capacityAddress;
        operation.growth.constructedBytes = 0;
        operation.growth.heldCapacity = 0;
        operation.growth.capacityAddress = capacityAddress;
        operation.growth.prefixBytes = 0;
        if (operation.required > 0xffffffffu - previousCapacity) {
            VectorLengthErrorStorage* error = ((VectorLengthErrorStorage*)&operation.err2vptr)
                                                  ->initialize(operation.empty2, operation.tmp2data, operation.tmp2end);
            __throw(vectorLengthExceptionStrings_ResourceList + 0x14, error, (void*)__dt__Q23std12length_errorFv);
        }
        replacementCapacity =
            ComputeByteVectorGrowthCapacity(previousCapacity, operation.required, operation.candA, operation.candB);
        void* replacementBuffer = ((void* (*)(unsigned int))s_vcgAllocatorCallbacks[0])(replacementCapacity);
        unsigned int oldSize = *(unsigned int*)(self + 4);
        operation.growth.heldBuffer = replacementBuffer;
        operation.growth.heldCapacity = replacementCapacity;
        operation.growth.prefixBytes = oldSize;
        memset((char*)((unsigned int)operation.growth.constructedBytes + ((unsigned int)replacementBuffer + oldSize)),
               0, growth);
        unsigned char* oldBytes = *(unsigned char**)self;
        unsigned char* oldEnd = oldBytes + *(unsigned int*)(self + 4);
        unsigned int prefixLength = ByteRangeLength(oldBytes, oldEnd);
        operation.growth.constructedBytes = (unsigned int)operation.growth.constructedBytes + growth;
        unsigned int tail = operation.growth.prefixBytes - prefixLength;
        operation.growth.prefixBytes = tail;
        // Copy and clear the previous prefix after constructing the zero suffix.
        memcpy((char*)(unsigned int)operation.growth.heldBuffer + tail, oldBytes, prefixLength);
        memset(oldBytes, 0, prefixLength);
        void* bufferToRelease = *(void**)self;
        unsigned int capacityToCommit = operation.growth.heldCapacity;
        void* replacementBytes = operation.growth.heldBuffer;
        unsigned int newTotal = (unsigned int)operation.growth.constructedBytes + prefixLength;
        operation.growth.constructedBytes = newTotal;
        unsigned int capacityToRelease = *(unsigned int*)(self + 8);
        // Commit the new header; the transaction now holds the allocation to free.
        *(unsigned int*)(self + 4) = 0;
        operation.growth.heldBuffer = bufferToRelease;
        AsByteVectorState(self)->capacity = capacityToCommit;
        operation.growth.heldCapacity = capacityToRelease;
        *(void**)self = replacementBytes;
        *(unsigned int*)(self + 4) = newTotal;
        operation.growth.constructedBytes = 0;
        if (bufferToRelease) {
            operation.growth.constructedBytes = 0;
            ((void (*)(void*))s_vcgAllocatorCallbacks[1])(bufferToRelease);
        }
    } else if (requestedBytes < size) {
        unsigned int diff = size - requestedBytes;
        AsByteVectorState(self)->size -= diff;
    }
}

struct Fn80239FECFrame {
    unsigned char empty2; // 0x08
    unsigned char pad00[3];
    unsigned char empty1; // 0x0c
    unsigned char pad01[3];
    unsigned int candA;    // 0x10
    unsigned int candB;    // 0x14
    unsigned int required; // 0x18
    unsigned int pad02;    // 0x1c
    char* tmp1data;        // 0x20
    void* tmp1end;         // 0x24
    char* tmp2data;        // 0x28
    void* tmp2end;         // 0x2c
    unsigned int err1vptr; // 0x30
    char* err1data;        // 0x34
    void* err1rep;         // 0x38
    unsigned int err2vptr; // 0x3c
    char* err2data;        // 0x40
    void* err2rep;         // 0x44
    union {
        struct {
            void* guard0;        // 0x48
            unsigned int guard1; // 0x4c
            unsigned int guard2; // 0x50
            void* guard3;        // 0x54
            unsigned int guard4; // 0x58
        };
        ByteVectorGrowthTransactionView growth;
    };
};

// Append storage has two independent error-construction regions plus one growth
// transaction; the physical order differs from VectorGrowthScratch above.
typedef Fn80239FECFrame ByteVectorAppendStorage;

// Append one byte, reallocating with the configured VCG allocator when full.
void ByteVector_AppendByte(unsigned char* self, const char* value) {
    unsigned int size = AsByteVectorState(self)->size;
    if (size < AsByteVectorState(self)->capacity) {
        unsigned int newSize = size + 1;
        AsByteVectorState(self)->size = newSize;
        (*(unsigned char**)self)[newSize - 1] = (unsigned char)*value;
        return;
    }
    // Full-vector path: check length, allocate, append, copy prefix, then commit.
    // *value is consumed before releasing bufferToRelease storage, including when it aliases it.
    ByteVectorAppendStorage operation;
    if (0xFFFFFFFFu - AsByteVectorState(self)->capacity < 1u) {
        VectorLengthErrorStorage* error = ((VectorLengthErrorStorage*)&operation.err1vptr)
                                              ->initialize(operation.empty2, operation.tmp1data, operation.tmp1end);
        __throw(vectorLengthExceptionStrings_ResourceList + 0x14, error, (void*)__dt__Q23std12length_errorFv);
    }
    unsigned int newSize = AsByteVectorState(self)->size + 1;
    unsigned int capFirst = AsByteVectorState(self)->capacity;
    void* capacityAddress = self + 8;
    operation.growth.heldBuffer = 0;
    operation.required = newSize - capFirst;
    unsigned int cap = AsByteVectorState(self)->capacity;
    operation.growth.constructedBytes = 0;
    operation.growth.heldCapacity = 0;
    operation.growth.capacityAddress = capacityAddress;
    operation.growth.prefixBytes = 0;
    if (operation.required > 0xFFFFFFFFu - cap) {
        VectorLengthErrorStorage* error = ((VectorLengthErrorStorage*)&operation.err2vptr)
                                              ->initialize(operation.empty1, operation.tmp2data, operation.tmp2end);
        __throw(vectorLengthExceptionStrings_ResourceList + 0x14, error, (void*)__dt__Q23std12length_errorFv);
    }
    unsigned int replacementCapacity =
        ComputeByteVectorGrowthCapacity(cap, operation.required, operation.candA, operation.candB);
    void* replacementBuffer = ((void* (*)(unsigned int))s_vcgAllocatorCallbacks[0])(replacementCapacity);
    unsigned int oldSize = AsByteVectorState(self)->size;
    operation.growth.heldBuffer = replacementBuffer;
    operation.growth.heldCapacity = replacementCapacity;
    operation.growth.prefixBytes = oldSize;
    memset((char*)((unsigned int)operation.growth.constructedBytes + ((unsigned int)replacementBuffer + oldSize)),
           (unsigned char)*value, 1);
    unsigned char* oldData = *(unsigned char**)self;
    unsigned char* oldEnd = oldData + AsByteVectorState(self)->size;
    unsigned int prefixLength = ByteRangeLength(oldData, oldEnd);
    operation.growth.constructedBytes = (unsigned int)operation.growth.constructedBytes + 1;
    unsigned int tail = operation.growth.prefixBytes - prefixLength;
    operation.growth.prefixBytes = tail;
    // Transfer the prefix after consuming the appended value (which may alias it).
    memcpy((char*)operation.growth.heldBuffer + tail, oldData, prefixLength);
    memset(oldData, 0, prefixLength);
    void* bufferToRelease = *(void**)self;
    unsigned int newTotal = (unsigned int)operation.growth.constructedBytes + prefixLength;
    unsigned char* replacementBytes = (unsigned char*)operation.growth.heldBuffer;
    operation.growth.constructedBytes = newTotal;
    unsigned int capacityToRelease = AsByteVectorState(self)->capacity;
    unsigned int capacityToCommit = operation.growth.heldCapacity;
    // Commit the new header; the transaction now holds the allocation to free.
    AsByteVectorState(self)->size = 0;
    operation.growth.heldBuffer = bufferToRelease;
    AsByteVectorState(self)->capacity = capacityToCommit;
    operation.growth.heldCapacity = capacityToRelease;
    *(unsigned char**)self = replacementBytes;
    AsByteVectorState(self)->size = newTotal;
    operation.growth.constructedBytes = 0;
    if (bufferToRelease) {
        operation.growth.constructedBytes = 0;
        ((void (*)(void*))s_vcgAllocatorCallbacks[1])(bufferToRelease);
    }
}

#pragma pop

// Clear the module allocation and release callbacks.
void VCG_ClearAllocatorCallbacks() {
    s_vcgAllocatorCallbacks[0] = 0;
    s_vcgAllocatorCallbacks[1] = 0;
}

// Initialize the byte vector data, size and capacity words to zero.
void VcgByteVector_Init(ByteVectorWords* byteVector) {
    byteVector->value0 = 0;
    byteVector->value4 = 0;
    byteVector->value8 = 0;
}

// Install the caller-supplied allocation and release callbacks.
int Vcg_SetAllocatorCallbacks(ByteVectorWords* callbacks) {
    unsigned int first = callbacks->value0;
    unsigned int* output = s_vcgAllocatorCallbacks;
    unsigned int second = callbacks->value4;
    output[0] = first;
    output[1] = second;
    return 1;
}

// Initialize this camera-file module angle storage to one full turn.
void VcgFile_InitAngleConstant() {
    s_vcgFileFullTurnAngle = kFloat_6_2831855;
}

// Return a byte-vector data address at the requested byte offset.
unsigned int VcgByteVector_GetDataAtOffset(ByteVectorDataView* arg, unsigned int offset) {
    return arg->value + offset;
}

// Clear a byte-vector count while retaining its allocation.
void VcgByteVector_ClearSize(ByteVectorClearView* arg) {
    arg->value = arg->value - *(int*)&arg->value;
}

// Draw an enabled untextured screen-space color quad with the UI render state.
void ImmediateOverlay_DrawUntexturedVertices(ImmediateOverlayView* overlay) {
    if (overlay->enabled) {
        UIRender_SetDefaultState();
        Engine_SetTextureStageResource(0, 0);
        Engine_SetTexturePalette(0, 0);
        Engine_SetFlexibleVertexFormat(kScreenColorVertexFormat);
        Engine_SetType7Resource(0);
        DrawPrimitiveUP(kQuadTriangleStrip, overlay->count, overlay->vertices, kScreenColorVertexStride);
    }
}

#pragma push
#pragma use_lmw_stmw on
// Release the byte-vector allocation through the installed VCG release callback.
void* VcgByteVector_dtor(ByteVectorDestroyView* byteVector, int freeFlag) {
    if (byteVector) {
        if (&byteVector->object && &byteVector->object && &byteVector->object && &byteVector->object) {
            if (byteVector->object) {
                unsigned int* tbl = s_vcgAllocatorCallbacks;
                unsigned int old = byteVector->clear;
                byteVector->clear = old - old;
                ((void (*)(void*))tbl[1])(byteVector->object);
            }
        }
        if (freeFlag > 0)
            __dl__FPv(byteVector);
    }
    return byteVector;
}
#pragma pop

// Release camera-file data through the VCG allocator, then destroy the source base.
CVCGFile::~CVCGFile() {
    CameraFileDestroyView* source = (CameraFileDestroyView*)this;
    if (&source->object && &source->object && &source->object && &source->object && &source->object) {
        if (source->object) {
            unsigned int old = source->clear;
            unsigned int* tbl = s_vcgAllocatorCallbacks;
            source->clear = old - old;
            ((void (*)(void*))tbl[1])(source->object);
        }
    }
    __dt__10CVCGSourceFv(source, 0);
}

// Initialize the fade module angle storage to one full turn.
void EffectFade_InitAngleConstant() {
    s_effectFadeFullTurnAngle = kFloat_6_2831855;
}

// Initialize the camera-command module angle storage to one full turn.
void CameraSetCommand_InitAngleConstant() {
    s_cameraSetCommandFullTurnAngle = kFloat_6_2831855;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off

} // extern "C"
struct CEventCameraSC_CAM_SET : CEventConte {
    unsigned int flags;  // 0x4: bit 31 set when the path is on the heap, else it starts inline at +5
    unsigned int length; // 0x8
    char* heap;          // 0xC
    void* resource;      // 0x10: the camera player
    CEventCameraSC_CAM_SET(const char* strings);
    void* Destroy(int freeFlag);
};
// Build the VCG path, load its groups and start the requested group if it exists.
// Input layout: u16 opcode, u16 groupIndex, then a NUL-terminated basename.
// The caller supplies a valid packed command; no length or alignment checks.
// The command owns its path string and player; SharedFile handles are temporary.
CEventCameraSC_CAM_SET::CEventCameraSC_CAM_SET(const char* packedCommand) {
    CameraSetCommandStorage* self = (CameraSetCommandStorage*)this;
    CameraSetConstructionScratch loc;
    const char* base = eventCameraSetStrings;
    void* newPlayer;
    const char* extension;
    unsigned int mask;
    unsigned int pathLength;
    // Phase 1: decode the basename and build emptyPrefix + basename + extension.
    const char* name = packedCommand + kCameraFileName;
    *(volatile unsigned int*)&self->flags = 0;
    *(volatile unsigned int*)&self->length = 0;
    *(volatile unsigned int*)&self->heap = 0;
    *(volatile unsigned int*)&self->vtable = (unsigned int)&__vt__22CEventCameraSC_CAM_SET;
    mask = ((CameraPathFlagBytes*)&self->flags)->bytes[0] & 0x7f;
    loc.f4[0] = loc.f4[4];
    unsigned int textLength = strlen(base);
    String_ReplaceRange(&self->flags, 0, mask, base, base + textLength, loc.f4);
    if ((self->flags >> 31) == 0)
        pathLength = ((CameraPathFlagBytes*)&self->flags)->bytes[0] & 0x7f;
    else
        pathLength = self->length;
    loc.f3[0] = loc.f3[4];
    textLength = strlen(name);
    String_ReplaceRange(&self->flags, pathLength, 0, name, name + textLength, loc.f3);
    extension = eventCameraSetStrings + kCameraExtension;
    if ((self->flags >> 31) == 0)
        pathLength = ((CameraPathFlagBytes*)&self->flags)->bytes[0] & 0x7f;
    else
        pathLength = self->length;
    loc.f2[0] = loc.f2[4];
    textLength = strlen(extension);
    String_ReplaceRange(&self->flags, pathLength, 0, extension, extension + textLength, loc.f2);
    // Phase 2: create an empty player before selecting loose-file or shared-file IO.
    newPlayer = __nw__FUl(0x30);
    if (newPlayer)
        ((CVCGPlayer*)newPlayer)->__ct();
    self->resource = newPlayer;
    CameraMotionGroupVector_Clear(newPlayer);
    if (GameHook_ReservedFalse() != 0) {
        loc.path.flags = 0;
        loc.path.length = 0;
        loc.path.heap = 0;
        const char* directory = eventCameraSetStrings + kCameraDirectory;
        unsigned int len = strlen(directory);
        String_ReserveCapacity(&loc.path, len);
        loc.f1[0] = loc.f1[4];
        String_ReplaceRange(&loc.path, 0, 0, directory, directory + len, loc.f1);
        const char* part;
        if ((self->flags >> 31) == 0)
            part = (char*)self + 5;
        else
            part = self->heap;
        unsigned int pathLength;
        if ((loc.path.flags >> 31) == 0)
            pathLength = ((CameraPathFlagBytes*)&loc.path.flags)->bytes[0] & 0x7f;
        else
            pathLength = loc.path.length;
        loc.f0[0] = loc.f0[4];
        unsigned int plen = strlen(part);
        String_ReplaceRange(&loc.path, pathLength, 0, part, part + plen, loc.f0);
        const char* full;
        if ((loc.path.flags >> 31) == 0)
            full = (char*)&loc.path + 1;
        else
            full = loc.path.heap;
        CameraScript_LoadFileByPath(self->resource, full);
        if (loc.path.flags >> 31)
            appFreeUni__FPv(loc.path.heap);
    } else {
        const char* part;
        if ((self->flags >> 31) == 0)
            part = (char*)self + 5;
        else
            part = self->heap;
        void* handle = SharedFile_Create(part, 1);
        void* loadedBytes = CSharedFilePtr_GetLoadedBuffer(handle);
        CameraScript_ParseGroupData(self->resource, loadedBytes);
        if (handle)
            CSharedFileHandle::Destroy(handle, 1);
    }
    // Phase 3: start an in-range group; otherwise destroy the player so update
    // reports immediate completion. Loading/parsing failures are not checked here.
    GroupVector_GetCount(self->resource, (unsigned int*)&loc.count);
    unsigned short groupIndex = *(unsigned short*)((char*)packedCommand + kCameraGroupIndex);
    if ((int)groupIndex < (int)loc.count) {
        ((CVCGPlayer*)(self->resource))->StartGroupPlayback(groupIndex, 0);
    } else {
        if (self->resource) {
            if (self->resource)
                ((CameraAnimationDestroyInterface*)self->resource)->v0(1);
            self->resource = 0;
        }
    }
}

#pragma opt_propagation reset
#pragma opt_lifetimes reset
// Destroy the active camera animation and the command path string. This is a named
// member, not a destructor: a real one would also call the base class destructor.
void* CEventCameraSC_CAM_SET::Destroy(int freeFlag) {
    if (this) {
        unsigned int* flagWord = &flags;
        unsigned int oldFlags = *flagWord;
        vtable = (void*)__vt__22CEventCameraSC_CAM_SET;
        if ((oldFlags >> 31) == 0) {
            *((unsigned char*)flagWord + 1) = 0;
            unsigned int old = *((unsigned char*)flagWord);
            *((unsigned char*)flagWord) = (unsigned char)(old & 0xFFFFFF80u);
        } else {
            *heap = 0;
            length = 0;
        }
        if (resource && resource) {
            void* __c = resource;
            (*(void (***)(void*, int))__c)[2](__c, 1);
            resource = 0;
        }
        // The original compiles an always-true address test here; removing it lowers the score.
        if ((char*)this + 4) {
            if ((*flagWord >> 31) != 0)
                appFreeUni__FPv(heap);
        }
        if (freeFlag > 0)
            __dl__FPv(this);
    }
    return this;
}

extern "C" {
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma pop

// Initialize the camera-playback module angle storage to one full turn.
void CameraPlayback_InitAngleConstant() {
    s_cameraPlaybackFullTurnAngle = kFloat_6_2831855;
}

// Initialize the overlay module angle storage to one full turn.
void ImmediateOverlay_InitAngleConstant() {
    s_immediateOverlayFullTurnAngle = kFloat_6_2831855;
}

// Sample camera position/target/up and FOV, update the view and advance until playback completes.
int CEventCameraSC::CAM_SET_UpdateCameraAnimation() {
    // World-space target-position gives forward; sampled up cross forward gives
    // right, and forward cross right gives corrected up. The affine camera frame
    // is passed to WView_SetViewMatrix; its internal inversion convention is external.
    // Byte storage follows CameraFrameScratchView; named offsets retain the
    // original sampler ABI and avoid constructing temporary WVector objects.
    unsigned char frameBytes[0x90];
    if (!cameraAnimation)
        return 1;
    if (!((CVCGPlayer*)cameraAnimation)
             ->SampleCurrentMotion(frameBytes + FIELD_OFFSET(CameraFrameScratchView, sample.position)))
        return 0;
    Vec3_CopySourceToDestination(frameBytes + FIELD_OFFSET(CameraFrameScratchView, sample.position),
                                 frameBytes + FIELD_OFFSET(CameraFrameScratchView, position));
    Vec3_CopySourceToDestination(frameBytes + FIELD_OFFSET(CameraFrameScratchView, sample.target),
                                 frameBytes + FIELD_OFFSET(CameraFrameScratchView, target));
    // Build an orthonormal world frame: forward, right, corrected up, translation.
    Matrix_SetIdentityAffine(s_cameraTransform);
    __mi__FRC7WVectorRC7WVector(frameBytes + FIELD_OFFSET(CameraFrameScratchView, forward),
                                frameBytes + FIELD_OFFSET(CameraFrameScratchView, target),
                                frameBytes + FIELD_OFFSET(CameraFrameScratchView, position));
    *(WVector*)(s_cameraTransform + kCameraForwardOffset) =
        *(WVector*)(frameBytes + FIELD_OFFSET(CameraFrameScratchView, forward));
    (*(WVector*)(s_cameraTransform + kCameraForwardOffset)).Normalize();
    Vec3_CopySourceToDestination(frameBytes + FIELD_OFFSET(CameraFrameScratchView, sample.up),
                                 s_cameraTransform + kCameraUpOffset);
    Vector_CrossProduct(frameBytes + FIELD_OFFSET(CameraFrameScratchView, right), s_cameraTransform + kCameraUpOffset,
                        s_cameraTransform + kCameraForwardOffset);
    *(WVector*)s_cameraTransform = *(WVector*)(frameBytes + FIELD_OFFSET(CameraFrameScratchView, right));
    (*(WVector*)s_cameraTransform).Normalize();
    Vector_CrossProduct(frameBytes + FIELD_OFFSET(CameraFrameScratchView, correctedUp),
                        s_cameraTransform + kCameraForwardOffset, s_cameraTransform);
    *(WVector*)(s_cameraTransform + kCameraUpOffset) =
        *(WVector*)(frameBytes + FIELD_OFFSET(CameraFrameScratchView, correctedUp));
    (*(WVector*)(s_cameraTransform + kCameraUpOffset)).Normalize();
    *(WVector*)(s_cameraTransform + kCameraPositionOffset) =
        *(WVector*)(frameBytes + FIELD_OFFSET(CameraFrameScratchView, position));
    WView_SetViewMatrix(g_view, s_cameraTransform);
    Camera_SetFov(g_view, *(float*)(frameBytes + FIELD_OFFSET(CameraFrameScratchView, sample.fieldOfView)));
    // Publish FOV and finish or advance this playback after publishing the frame.
    ((CVCGPlayer*)cameraAnimation)->GetPlaybackState(frameBytes + FIELD_OFFSET(CameraFrameScratchView, playbackWords));
    if (*(int*)(frameBytes + FIELD_OFFSET(CameraFrameScratchView, playbackWords[2])) == kCameraPlaybackComplete) {
        void* animation = cameraAnimation;
        if (animation)
            ((void (**)(void*, int))(*(void***)animation))[2](animation, 1);
        cameraAnimation = 0;
        return 1;
    }
    ((CVCGPlayer*)cameraAnimation)->AdvancePlayback();
    return 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046226C", _ctors_u80238E9C_0, EffectFade_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046226C", _ctors_u80238E9C_1, CameraSetCommand_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046226C", _ctors_u80238E9C_2, CameraPlayback_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046226C", _ctors_u80238E9C_3, ImmediateOverlay_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046226C", _ctors_u80238E9C_4, VcgFile_InitAngleConstant);
#pragma pop

// Compile-time guards for the byte scratch layouts passed to the camera sampler.
typedef char CameraFrameScratch_size[(sizeof(CameraFrameScratchView) == 0x90) ? 1 : -1];
typedef char CameraFrameSample_offset[(FIELD_OFFSET(CameraFrameScratchView, sample) == 0x58) ? 1 : -1];
typedef char CameraSetCommand_size[(sizeof(CEventCameraSC_CAM_SET) == 0x14) ? 1 : -1];
