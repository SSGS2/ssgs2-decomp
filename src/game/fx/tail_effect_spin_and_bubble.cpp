#include <decomp/static_init.h>
#include "ppc_double_words.h"
#include "classes/CTaskMain.h"
#include "MSL/vector.h"
struct TVec;
namespace std {
template <> struct __vec_destroys_elements<TVec> { enum { value = 4 }; };
}
typedef std::vector<TVec, app::app_allocator<TVec> > TrailRecordVector;
#include "game/golfball.h"
#include "classes/CSharedDoc.h"
#include "classes/CMsgDraw.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "game/itemwindow.h"
#include "game/spin.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "game/wvector.h"
#include "game/wstring.h"
#include "classes/CGeneTex3D.h"
#include "classes/CTailEffect.h"

#define AsCTailEffect(p) ((CTailEffect*)(p))

#define AsCGeneTex3D(p) ((CGeneTex3D*)(p))

namespace std {
class LocalNString_u801FB14C {
public:
    // flags bit 31 selects owned heap storage; otherwise this is an inline string.
    // Legacy field capacity is the heap data pointer at +8, not an element count.
    // Field name is provisional.
    unsigned int flags; unsigned int length; void* heapData;
    // Initialize an empty non-owning string representation.
    LocalNString_u801FB14C() {
        *(volatile unsigned int*)&flags = 0;
        *(volatile unsigned int*)&length = 0;
        *(void* volatile*)&heapData = 0;
    }
    // Free heap storage only when the ownership flag is set.
    ~LocalNString_u801FB14C() { if (flags >> 31) appFreeUni__FPv(heapData); }
};
}

extern "C" {

// PartyItemSlots_GetInstance supplies optional wind and spin/curve stat overrides.
struct ShotStatOverrides {
    char reserved0[0x28]; float windMagnitude;
    char reserved2C[8]; float spinRange; float curveRange;
    char reserved3C[0x14]; int normalizeWind; int overrideWindMagnitude; int overrideSpinCurve;
};
struct RoundPlayerStats {
    char reserved0[0xA0]; unsigned int equippedItemId;
    char reservedA4[0x40]; int spin; int curve;
};


extern void* s_spinTypeInfo;
extern void* g_resourceManager;
extern char __vt__11CTailEffect[];
extern char __vt__10CGeneTex3D[];
extern const float kFloat_6_2831855;
extern float s_tailEffectAngularPeriod;
extern const float kFloat_0_spin;
extern const float kFloat_0_TailEffect;
extern const float kFloat_0_3_TailEffect;
extern const float kFloat_30_PointerHitRange;
extern const float kFloat_neg1_Spin;
extern const float kFloat_1_StrokePointShared;
extern const double kDouble_SignedIntConversionBias_StrokePoint;
extern void appFreeUni__FPv(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern void __dl__FPv(void*);
extern void ResourceManager_ReleaseId(void*, void*);
extern void __dla__FPv(void*);
extern void* InstanceTable_Get();
extern void* GolfBall__Fv();
extern void __dt__13GeneTexBufferFv(void*);
extern void __dt__20NativeMember80201D18Fv(void*);
extern int Vec3_NearlyEqual(void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern const float kFloat_5_TailEffect;
extern const float kFloat_3_TailEffect;
extern const float kFloat_neg1_TailEffect;
extern const float kFloat_1_TailEffect;
extern const float kFloat_6_TailEffect;
extern const float kFloat_4_TailEffect;
extern const float kFloat_neg3_TailEffect;
extern float kFloat_3_1415927_StrokePoint;
extern float kFloat_neg1_5707964_Spin;
extern float kFloat_50_StrokePointLinearRate;
extern float kFloat_1_5707964_StrokePoint;
extern float kFloat_neg3_1415927_Spin;
extern float kFloat_0_5_StrokePointSoundArgument;
extern float kFloat_200_StrokePointMove;
extern void* g_sharedDoc;
extern signed char s_inputManagerInitialized;
extern void* g_inputPointer;
extern void* g_soundManager;
extern void* PartyItemSlots_GetInstance();
extern void* GolfPlayer_GetEntryByIndex(void*, int);
extern void* ItemInventory_GetItemIdAt(void*, int);
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern int Input_IsActionButtonDown(void*, int, int);
extern void IActor_DispatchMessage(void*, void*, const char*, int, void*, int, int, int);
extern void Sound_StopByAlias(void*, const char*);
extern void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
extern float Math_SquareRootApprox(float);
extern float Math_CosApprox(float);
extern float Math_SinApprox(float);
extern double acos(double);
extern const char strokePointMoveStrings[];
extern char s_gameInputManager[];
extern char s_spinInputManagerDtorRegistration[];
extern void CThread_IsFinished(void*);
extern void Object_DeleteVirtual(void*, void*);
extern void ActorFactoryInterface_InitVtable(void*);
extern void CAnotherHand_Ctor(void*);
extern void* appMallocUni__FUl(unsigned int);
extern char __vt__5CTask[];
extern int SharedDoc_IsPlayerFlag0Set(unsigned char);
extern const float kFloat_3_BubbleText;
extern float kFloat_255_BubbleText;
extern void CMsgDraw_ResetTextState(void*);
extern void Text_SetPosition(void*, void*);
extern void CMsgDraw_ClearFormattingAndRevealAll(void*);
extern void Text_DrawGlyphs(void*);
extern void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, int, unsigned int, int);
extern void* ItemInventory_FindCaddieData(void*, void*);
extern void String_ReplaceRange(void*, int, int, void*, void*, void*);
extern void WString_ReplaceRange(void*, int, int, void*, void*, void*);
extern void MessageDrawPairList_Dtor(void*);
extern void Text_SetContent(void*, void*);
extern void CMsgDraw_GetTextExtent(void*, void*);
extern char s_caddieMessagePathSuffix[]; // Suffix appended to the selected caddie's data path.
extern wchar_t s_caddieMessageWidePrefix[]; // Wide prefix inserted before the caddie message.
extern void __dt__11AuxMenuBaseFv(void*);
extern void dtor_8029EA18(void*, int);
extern void CAnotherHand_dtor(void*, int);
extern void PairVector_InitReserved(void*);
extern void PointerPairList_RemoveByKey(void*, void*);
extern signed char s_pointerPairListInitialized;
extern char s_messageDrawPairList[];
extern char s_bubbleMessageDrawPairListDtorRegistration[];
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void* Camera_GetRenderState(void*);
extern void SetTransform(int, void*);
extern void Engine_SetType7Resource(int);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);
extern void Engine_SetTextureStageParameter(int, int, int);
extern void Engine_SetCachedRenderState(int, int);
extern int GX_GetPrimitiveCount(int, int);
extern unsigned int s_tailEffectRenderStateTables[];
extern char kIdentityMatrix4x4[];
extern void* g_view;
extern const float kFloat_255_TailEffect;
extern float s_bubbleVisibilityZero;
extern const float kFloat_0_U801FB14C;
extern const float kFloat_0_00001_BubbleVisibility;
extern const float kFloat_12_BubbleText;
extern const float kFloat_0_5_BubbleLayoutAndAtlas;
extern float kSceneEffectScreenWidth;
extern float kFloat_20_BubbleAnchorOffset;
extern const float kFloat_6_BubbleTextInset;
extern float kFloat_0_234375_BubbleAtlas;
extern float kFloat_0_375_BubbleAtlas;
extern float kFloat_0_3125_BubbleAtlas;
extern float kFloat_5_Bubble;
extern float kFloat_480_BubbleReferenceHeight;
extern float kFloat_0_046875_BubbleAtlasSideCap;
extern float kFloat_0_09375_BubbleText;
extern const float kFloat_1_BubbleText_805DFC58;
extern int GolfTips_IsDialogActive(void*);
extern void* __nw__FUl(unsigned int);
extern void* WOverlay_Create(void*, const char*, int);
extern void* Runtime_DynamicCast(void*, int, void*, void*, int);
extern void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
extern void* CTask_Ctor(void*);
extern void __ct__18NativeBase80201D18Fv(void*);
extern const float kFloat_neg1_MessageOverlay;
extern void* g_chatMsg;
extern unsigned char __RTTI__10WFixedFont;
extern unsigned char __RTTI__5WFont;
extern unsigned char __vt__9CTaskMain[];
extern float s_twoPi_801FB14C;
extern float s_taskAngularPeriod;

// Clamp normalized spin/curve input to the inclusive range [-1, 1].
static inline float clampf(float a) {
    return a < kFloat_neg1_Spin ? kFloat_neg1_Spin : (a > kFloat_1_StrokePointShared ? kFloat_1_StrokePointShared : a);
}

// Select the greater value, preserving operand order for unordered comparisons.
static inline float fmaxf(float a, float b) { return a < b ? b : a; }
// Select the lesser value, preserving operand order for unordered comparisons.
static inline float fminf(float a, float b) { return a > b ? b : a; }

enum SpinMessageCode {
    kSpinGetNormalizedHit = 0x1,
    kSpinGetRangeNormalizedHit = 0x2,
    kSpinEnable = 0x3,
    kSpinDisable = 0x4,
    kSpinCopyBallSelection = 0x5,
    kSpinGetScreenPosition = 0x6,
    kSpinGetColorAndScreenPosition = 0x7,
    kSpinGetHitPosition = 0x8,
    kSpinGetRange = 0x9,
    kSpinSetIntegerHit = 0xa,
    kSpinReset = 0x2719,
};

enum { kTrailRecordCapacity = 30, kTrailSamplesPerSpan = 5 };

enum { kTaskAuxVtableOffset = 0x10, kTaskActorVtableOffset = 0x1C, kTaskMainActorVtableOffset = 0x3C };

enum { kTrailTextureGeneratorOffset = 0x54 };
struct MessageFontProvider { char reserved0[0x10]; void* defaultFont; };
struct InstanceMode { char reserved0[0xA]; unsigned char state; };
struct InstanceSelection { void* reserved0; InstanceMode* mode; };

struct SpinMessageArguments {
    // Queries use x.output/y.output as borrowed writable floats. ID 7 also
    // writes color. ID 10 instead carries immediate signed integer coordinates
    // in x.bits/y.bits; they are converted numerically, not reinterpreted floats.
    // IDs 1/2 return hit/30 and hit/range respectively; 6/7 absolute HUD
    // position, 8 relative hit offset, 9 ellipse radii. No pointer is retained.
    void* sender; int code;
    union Coordinate { float* output; unsigned int bits; } x, y;
    int* color;
};

struct TArr3 { int v[3]; };
struct TVec { unsigned char active; float age; TArr3 firstEndpoint; TArr3 secondEndpoint; };
union TFlag { unsigned char c; int i; };
struct TBuf {
    TVec* data; int size; unsigned int capacity;
    // Initialize an empty owned trail-vector header.
    TBuf() { volatile TFlag fc; data = 0; size = 0; capacity = 0; fc.c = 0; }
    // Clear the vector size and free its owned record storage.
    ~TBuf() {
        volatile TFlag fd;
        if (data) { fd.c = 0; char* d = (char*)data; size -= (d + size) - d; appFreeUni__FPv(data); }
    }
};
// Trail layout: +4 current records, +0x10 pending records (two vector headers),
// +0x34/+0x38 time scalars in seconds, +0x3C/+0x40 current ring cursors,
// +0x48 texture ID, +0x4C/+0x50 pending cursors, +0x54 geometry interface.
// Fn801FC824Object/Fn801FC998Obj expose portions of this same storage; endpoint
// words are XYZ float bit patterns and their copies perform no conversion.
struct TTail {
    void* vt; TBuf records; TBuf pendingRecords; char reserved1c[0x18];
    float recordAge; float interpolationTime; int writeIndex; int readIndex; unsigned char c44; char p45[3];
    int textureId; int pendingWriteIndex; int pendingReadIndex; unsigned char c54; char p55[3]; void* textureGeneratorVtable; int renderBuffer;
};

// Transfer replacement trail storage while retaining the previous allocation for cleanup.
static inline void tbuf_swap(TBuf& cur, TBuf& first) {
    volatile TFlag f2;
    TVec* old = cur.data;
    *(volatile int*)&cur.size = 0;
    TVec* nd = first.data;
    int oc = cur.capacity;
    int ocap = first.capacity;
    int os = *(volatile int*)&cur.size;
    first.data = old;
    f2.c = 0;
    cur.capacity = ocap;
    first.capacity = oc;
    cur.data = nd;
    cur.size = first.size;
    first.size = os;
}

// Install trail-vector backing storage without changing its allocation policy.
static inline void tbuf_setdata(TBuf& b, TVec* d) {
    volatile TFlag f1;
    b.data = d;
    f1.c = 0;
}

// Grow a trail record vector to the fixed ring capacity.
static inline void tbuf_grow(TBuf& cur) {
    TBuf first;
    int base = first.size;
    TVec* newd = (TVec*)appMallocDummy__FUl(0x3c0);
    TVec* src = cur.data;
    int cursize = cur.size;
    TVec* end = src + cursize;
    first.capacity = kTrailRecordCapacity;
    TVec* dst = newd + base;
    tbuf_setdata(first, newd);
    while (src < end) {
        if (dst) { try { dst->active=src->active; dst->age=src->age; dst->firstEndpoint=src->firstEndpoint; dst->secondEndpoint=src->secondEndpoint; } catch (...) { throw; } }
        ++first.size; ++src; ++dst;
    }
    tbuf_swap(cur, first);
}

// Ensure trail storage can hold the requested record count.
static inline void tbuf_reserve(TBuf& cur, unsigned int n) {
    if (cur.capacity < n) tbuf_grow(cur);
}

struct Fn801FC824Entry { unsigned char active; char reserved01[3]; float time; char reserved08[0x18]; };
struct Fn801FC824Object { char reserved00[4]; Fn801FC824Entry* entries; char reserved08[0x34]; int count; int current; char reserved44[8]; int pendingWriteIndex; int pendingReadIndex; };

struct Fn801FC998Vec { unsigned int x, y, z; };
struct Fn801FC998Edge { char reserved00[8]; Fn801FC998Vec a; Fn801FC998Vec b; };
struct Fn801FC998Sub;
struct Fn801FC998Vtbl {
    void* reservedSlots[3];
    void (*begin)(Fn801FC998Sub*);
    unsigned char (*resize)(Fn801FC998Sub*, unsigned char);
    void (*color)(Fn801FC998Sub*, unsigned char, unsigned char, int, int, int);
    void (*position)(Fn801FC998Sub*, int, Fn801FC998Vec*);
    void (*texCoord)(Fn801FC998Sub*, unsigned char, float, float);
    void (*finish)(Fn801FC998Sub*, int, int);
};
struct Fn801FC998Sub {
    unsigned char count;
    virtual void unused();
    virtual void begin();
    virtual unsigned char resize(unsigned char);
    virtual void color(unsigned char, unsigned char, int, int, int);
    virtual void position(unsigned char, Fn801FC998Vec*);
    virtual void texCoord(unsigned char, float, float);
    virtual void finish(int, int);
};
struct Fn801FC998Mgr;
struct Fn801FC998MgrVtbl {
    char reserved00[0x70];
    void* (*get70)(Fn801FC998Mgr*, int);
    void* (*get74)(Fn801FC998Mgr*, int);
};
struct Fn801FC998Mgr {
    virtual void unused00(); virtual void unused01(); virtual void unused02(); virtual void unused03();
    virtual void unused04(); virtual void unused05(); virtual void unused06(); virtual void unused07();
    virtual void unused08(); virtual void unused09(); virtual void unused10(); virtual void unused11();
    virtual void unused12(); virtual void unused13(); virtual void unused14(); virtual void unused15();
    virtual void unused16(); virtual void unused17(); virtual void unused18(); virtual void unused19();
    virtual void unused20(); virtual void unused21(); virtual void unused22(); virtual void unused23();
    virtual void unused24(); virtual void unused25();
    virtual void* get70(int);
    virtual void* get74(int);
};
struct Fn801FC998Game { char reserved00[0xA4]; Fn801FC998Mgr* mgr; };
struct Fn801FC998Obj {
    char reserved00[4]; Fn801FC998Edge* edges; char reserved08[0x34]; int head; int tail; char reserved44[4];
    int index; char reserved4C[8]; Fn801FC998Sub sub;
};
struct Fn801FC998Block0 { unsigned int w[kTrailRecordCapacity]; };
struct Fn801FC998Block1 { unsigned int w[12]; };
struct Fn801FC998Block2 { unsigned int w[10]; };
struct Fn801FC998Data { Fn801FC998Block0 b0; Fn801FC998Block1 b1; Fn801FC998Block2 b2; };

// Return either endpoint of an indexed trail edge.
inline Fn801FC998Vec fn_801FC998Point(Fn801FC998Obj* o, int e, int side) {
    if (side != 0) return o->edges[e].b;
    return o->edges[e].a;
}

struct Fn801FDBC8Node { void* object; char reserved04[8]; Fn801FDBC8Node* prev; Fn801FDBC8Node* next; };

// Release a task object through its deleting virtual destructor.
inline void fn_801FDBC8Release(void* p) {
    if (p) ((void (**)(void*, int))*(void***)p)[3](p, 1);
}

// Task storage vocabulary: +0x10 active ring, +0x14 borrowed iteration cursor,
// +0x18 allocation-block ring, +0x1C recycled-node ring; +0x2C hash buckets.
// Fn801FDAF0Object is the pool subobject at task+0x10, so its +8 head is
// the task's allocation-block ring. Payload iteration never unlinks ring nodes.
// Legacy field/method spellings below are provisional, not contracts.
struct Fn801FDBC8Object {
    void* vtable; void* vtable04; void* vtable08; char reserved0C[4];
    Fn801FDBC8Node* first; Fn801FDBC8Node* current; Fn801FDBC8Node* allocationBlocks; Fn801FDBC8Node* freeNodes;
    char reserved20[8]; int bucketCount; void** hashBuckets; char mutex[0x1C]; void* field4C; char reserved50[0x20];

    // Unlink a task ring node under its mutex; return its successor or null.
    Fn801FDBC8Node* remove(Fn801FDBC8Node* node) {
        OSLockMutex((OSMutex*)((char*)this + (int)&((Fn801FDBC8Object*)0)->mutex));
        node->next->prev = node->prev;
        node->prev->next = node->next;
        Fn801FDBC8Node* next = node->next;
        if (next == node) next = 0;
        OSUnlockMutex((OSMutex*)((char*)this + (int)&((Fn801FDBC8Object*)0)->mutex));
        return next;
    }
    // Advance the task iteration cursor and return a borrowed task; do not erase its node.
    void* popFront() {
        Fn801FDBC8Node* node;
        OSLockMutex((OSMutex*)((char*)this + (int)&((Fn801FDBC8Object*)0)->mutex));
        node = current;
        if (node) {
            Fn801FDBC8Node* next = node->next;
            void* result = node->object;
            if (next == this->first) next = 0;
            current = next;
            OSUnlockMutex((OSMutex*)((char*)this + (int)&((Fn801FDBC8Object*)0)->mutex));
            return result;
        }
        OSUnlockMutex((OSMutex*)((char*)this + (int)&((Fn801FDBC8Object*)0)->mutex));
        return 0;
    }
    // Link a task node before the existing ring head under the mutex.
    Fn801FDBC8Node* addHead(Fn801FDBC8Node* node) {
        Fn801FDBC8Node* head = this->freeNodes;
        OSLockMutex((OSMutex*)((char*)this + (int)&((Fn801FDBC8Object*)0)->mutex));
        if (!head) {
            node->prev = node;
            node->next = node;
            OSUnlockMutex((OSMutex*)((char*)this + (int)&((Fn801FDBC8Object*)0)->mutex));
            return node;
        }
        node->next = head;
        node->prev = head->prev;
        head->prev = node;
        node->prev->next = node;
        OSUnlockMutex((OSMutex*)((char*)this + (int)&((Fn801FDBC8Object*)0)->mutex));
        return head;
    }
};

struct WInstanceLock {
    void* ptr;
    // Hold one recursive task-list mutex level until guard destruction.
    WInstanceLock(void* p): ptr(p) { OSLockMutex((OSMutex*)ptr); }
    // Release the task-list mutex level owned by this guard.
    ~WInstanceLock() { OSUnlockMutex((OSMutex*)ptr); }
};

struct Fn801FD8A8ListNode { char reserved00[0x0C]; Fn801FD8A8ListNode* prev; Fn801FD8A8ListNode* next; };
struct Fn801FD8A8Object {
    void* vtable; void* vtable04; void* vtable08; unsigned char flag;
    char reserved0D[3]; Fn801FD8A8ListNode* first; int iterationCursor; int freeNodes; Fn801FD8A8ListNode* head;
    int nodeBlockSize; int mask; int size; void** items; char mutex[0x18]; int mainActor; int ownedActor;
    char reserved50[0x350]; int unk3A0; unsigned char unk3A4; unsigned char unk3A5;
};

// Balloon layout shared by the method-specific ABI views below:
// +0 font, +4 overlay, +0x0C visibility scale, +0x10 opacity in [0,255],
// +0x14 fade seconds, +0x18/+0x1C text extent in HUD pixels, +0x20 renderer,
// +0x24..+0x2F tagged UTF-16 text (inline characters start at +0x26).
// The constructor's WString at +0x24 owns its heap representation. Drawing
// borrows the renderer/overlay and does not retain camera or coordinate inputs.
// One authoritative draw/update layout; aliases below preserve method ABI names.
// Synonyms are legacy member spellings, not separate storage or separate roles.
struct BubbleStorageView {
    void* font;
    union { void* overlay; void* text; };
    char reserved08[4];
    float visibilityScale;
    float opacity;
    float fadeSecondsRemaining;
    union { float width; float textWidth; };
    union { float height; float textHeight; };
    void* renderer;
    unsigned int textFlags;
    unsigned int textLength;
    void* heapText;
};
typedef char BubbleStorageSize[(sizeof(BubbleStorageView) == 0x30) ? 1 : -1];
typedef BubbleStorageView Fn801FD110Object;

typedef std::LocalNString_u801FB14C Fn801FD110String;
struct Fn801FD110Ch { unsigned char v; unsigned char pad[3]; };

struct Fn801FD360Quad { float x, y, z, w; };
struct Fn801FD360Pair { float x, y; };
typedef BubbleStorageView Fn801FD360Object;

// Test whether a scalar is within the balloon fade epsilon of zero.
inline int fn_801FD360Tiny(float v) {
    float a = (*(unsigned int*)&v & 0x80000000) ? -v : v;
    return a < kFloat_0_00001_BubbleVisibility;
}

typedef BubbleStorageView Fn801FCF5CObject;
class Dm0 { public: 
virtual void v08(int);
};

struct Fn801FDAF0Link { char reserved00[0x0C]; Fn801FDAF0Link* prev; Fn801FDAF0Link* next; };
struct Fn801FDAF0Object {
    char reserved00[8]; Fn801FDAF0Link* head; char reserved0C[0x10]; void* field1C;
    char mutex[0x18];
    // Unlink a task-list node under its mutex; return successor or null.
    Fn801FDAF0Link* remove(Fn801FDAF0Link* node) {
        OSLockMutex((OSMutex*)((char*)this + (int)&((Fn801FDAF0Object*)0)->mutex));
        node->next->prev = node->prev;
        node->prev->next = node->next;
        Fn801FDAF0Link* next = node->next;
        if (next == node) next = 0;
        OSUnlockMutex((OSMutex*)((char*)this + (int)&((Fn801FDAF0Object*)0)->mutex));
        return next;
    }
};

struct Fn801FD814Object {
    char storage[0x3A8];
    // Construct the main task container in its allocated storage.
    Fn801FD814Object() { CTask_Ctor(this); }
};

// Construction view of CTaskMain; canonical members: classes/CTaskMain.h.
// Includes the actor interface at +0x18; shared header only describes +0.
struct CTaskMainConstructorView {
    void* vtable;
    char reserved04[0x14];
    void* secondaryVtable;

    // Construct the lightweight task actor and install its two interface vtables.
    CTaskMainConstructorView() {
        __ct__18NativeBase80201D18Fv(this);
        vtable = __vt__9CTaskMain;
        secondaryVtable = (char*)__vt__9CTaskMain + kTaskMainActorVtableOffset;
    }
};

struct Fn801FD7ACObject {
    char reserved00[0x10];
    float value;
};

struct Fn801FD7ACState {
    char reserved00[0xB90];
    void* object;
};

struct Fn801FDF48Object {
    char reserved00[0x48];
    int value;
};

struct Dtor801FCE48Object {
    void* data;
    int count;
};

typedef BubbleStorageView Fn801FD080Object;

struct Fn801FCEC4Object {
    void* font;
    void* overlay;
    char reserved08[4];
    float visibilityScale;
    float opacity;
    float fadeSecondsRemaining;
    char reserved18[0x0C];
    WString text;
    Fn801FCEC4Object(const char* resource);
};

// Move the spin/curve point within its stat-dependent ellipse and update movement audio.
// dt is seconds. hitX/hitY are offsets from centerX/centerY in HUD coordinates;
// rangeX is curve, rangeY is spin. hudX/hudY are center+offset screen
// coordinates; shared hudX/hudY fields store these same HUD positions. The allowed ellipse is
// (x/rangeX)^2+(y/rangeY)^2<=1. Existing code assumes usable nonzero ranges.
void CSpin_OnProcess(CSpin* self, float dt) {
    float boundaryAngle;
    float angleLimit;
    float lower;
    float candidateAngle;
    CSpin* s = self;
    // Phase 1: input gates. An early return preserves the prior selection/audio state.
    if (AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->tutorialStage < 6) return;
    if (CItemWindow::IsClosed() == 0) return;
    if (((InstanceSelection*)InstanceTable_Get())->mode->state == 3) return;
    switch (s->active) {
    default: {

    void* pad = PartyItemSlots_GetInstance();
    void* playerStats;
    void* inventory;

    // Phase 2: derive ellipse radii. Equipment slots 0/8 or the power-shot override
    // force 30; otherwise use the optional stat override, then the player's stat.
    inventory = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
    playerStats = GolfPlayer_GetEntryByIndex(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc,
        AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
    if ((*(void**)&((RoundPlayerStats*)playerStats)->equippedItemId) == ItemInventory_GetItemIdAt(inventory, 0)
        || (inventory = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory),
            playerStats = GolfPlayer_GetEntryByIndex(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc,
            AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer),
            (*(void**)&((RoundPlayerStats*)playerStats)->equippedItemId) == ItemInventory_GetItemIdAt(inventory, 8))
        || AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->powerShotMeterOverride != 0) {
        s->rangeY = kFloat_30_PointerHitRange;
    } else if (((ShotStatOverrides*)pad)->overrideSpinCurve != 0) {
        s->rangeY = ((ShotStatOverrides*)pad)->spinRange;
    } else {
        s->rangeY = (float)*(int*)((char*)GolfPlayer_GetEntryByIndex(
            *(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc,
            AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer) + (int)&((RoundPlayerStats*)0)->spin);
    }
    inventory = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
    playerStats = GolfPlayer_GetEntryByIndex(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc,
        AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
    if ((*(void**)&((RoundPlayerStats*)playerStats)->equippedItemId) == ItemInventory_GetItemIdAt(inventory, 0)
        || (inventory = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory),
            playerStats = GolfPlayer_GetEntryByIndex(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc,
            AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer),
            (*(void**)&((RoundPlayerStats*)playerStats)->equippedItemId) == ItemInventory_GetItemIdAt(inventory, 8))
        || AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->powerShotMeterOverride != 0) {
        s->rangeX = kFloat_30_PointerHitRange;
    } else if (((ShotStatOverrides*)pad)->overrideSpinCurve != 0) {
        s->rangeX = ((ShotStatOverrides*)pad)->curveRange;
    } else {
        s->rangeX = (float)*(int*)((char*)GolfPlayer_GetEntryByIndex(
            *(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc,
            AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer) + (int)&((RoundPlayerStats*)0)->curve);
    }

    // Phase 3: query stroke-panel readiness from the screen (message 0x31).
    // payload0 points to out.flag, written as an int by CScreen_HandleMsg.
    // The extra copied pointer records are unused by that query but retained.
    // If the panel is not ready, publish position and stop movement audio.
    struct Arr3 { int v[3]; };
    struct Out { int flag; Arr3 a; Arr3 b; Arr3 c; } out;
    unsigned int source;
    source = (unsigned int)g_inputPointer;
    out.b = out.c = out.a = *(Arr3*)(source + 0x54);
    out.flag = 0;
    IActor_DispatchMessage(s, s, strokePointMoveStrings + 0x10, 0x31, &out, 0, 0, 0);

    if (((int&)Sing_805DC5C8()) == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_spinInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
    }
    pad = (void*)((int&)Sing_805DC5C8());

    if (out.flag == 0) {
        float hudY;
        float hudX;
        float baseY;
        float spinY;
        float baseX;
        float spinX;
        spinY = s->hitY;
        baseY = s->centerY;
        spinX = s->hitX;
        hudY = spinY + baseY;
        baseX = s->centerX;
        hudX = spinX + baseX;
        s->hudY = hudY;
        s->hudX = hudX;
        if (s->soundPlaying != 0) {
            Sound_StopByAlias(g_soundManager, strokePointMoveStrings);
            s->soundPlaying = 0;
        }
        return;
    }

    // Phase 4: actions 0/1/2/3 move -Y/+Y/-X/+X at 50 units/second.
    // Within the ellipse move linearly. At its edge, acos/asin reconstruct an
    // angle and advance around the perimeter by pi*dt radians. Branch order
    // matters for simultaneous directional inputs and endpoint clamping.
    float x = s->hitX;
    float y = s->hitY;
    float step = kFloat_3_1415927_StrokePoint * dt;

    if (Input_IsActionButtonDown(pad, 0, -1)
        && !Input_IsActionButtonDown(pad, 3, -1)
        && !Input_IsActionButtonDown(pad, 2, -1)) {
        float lim = kFloat_neg1_Spin * s->rangeY * Math_SquareRootApprox(
            kFloat_1_StrokePointShared - (x * x) / (s->rangeX * s->rangeX));
        if (y == lim) {
            float t = x / s->rangeX;
            t = clampf(t);
            boundaryAngle = acos(t);
            if (x > kFloat_0_spin) {
                candidateAngle = kFloat_neg1_Spin * boundaryAngle - step;
                angleLimit = kFloat_neg1_5707964_Spin;
                s->angle = candidateAngle < angleLimit ? angleLimit : candidateAngle;
            } else {
                candidateAngle = kFloat_neg1_Spin * boundaryAngle + step;
                lower = kFloat_neg1_5707964_Spin;
                s->angle = candidateAngle > lower ? lower : candidateAngle;
            }
            x = s->rangeX * Math_CosApprox(s->angle);
            y = s->rangeY * Math_SinApprox(s->angle);
            if (y < -s->rangeY) y = -s->rangeY;
        } else {
            y -= kFloat_50_StrokePointLinearRate * dt;
            if (y < lim) {
                y = lim;
                s->color = 0xA0FFFFFF;
            } else {
                s->color = -1;
            }
        }
    }

    if (Input_IsActionButtonDown(pad, 1, -1)
        && !Input_IsActionButtonDown(pad, 3, -1)
        && !Input_IsActionButtonDown(pad, 2, -1)) {
        float lim = s->rangeY * Math_SquareRootApprox(
            kFloat_1_StrokePointShared - (x * x) / (s->rangeX * s->rangeX));
        if (y == lim) {
            float t = x / s->rangeX;
            t = clampf(t);
            boundaryAngle = acos(t);
            if (x < kFloat_0_spin) {
                s->angle = fmaxf(boundaryAngle - step, kFloat_1_5707964_StrokePoint);
            } else {
                s->angle = fminf(boundaryAngle + step, kFloat_1_5707964_StrokePoint);
            }
            x = s->rangeX * Math_CosApprox(s->angle);
            y = s->rangeY * Math_SinApprox(s->angle);
            if (y > s->rangeY) y = s->rangeY;
        } else {
            y += kFloat_50_StrokePointLinearRate * dt;
            if (y > lim) {
                y = lim;
                s->color = 0xA0FFFFFF;
            } else {
                s->color = -1;
            }
        }
    }

    if (Input_IsActionButtonDown(pad, 2, -1)) {
        float lim = kFloat_neg1_Spin * s->rangeX * Math_SquareRootApprox(
            kFloat_1_StrokePointShared - (y * y) / (s->rangeY * s->rangeY));
        if (x == lim) {
            float t = x / s->rangeX;
            t = clampf(t);
            boundaryAngle = acos(t);
            if (y > kFloat_0_spin) {
                s->angle = fminf(boundaryAngle + step, kFloat_3_1415927_StrokePoint);
            } else {
                candidateAngle = kFloat_neg1_Spin * boundaryAngle - step;
                angleLimit = kFloat_neg3_1415927_Spin;
                s->angle = candidateAngle < angleLimit ? angleLimit : candidateAngle;
            }
            x = s->rangeX * Math_CosApprox(s->angle);
            y = s->rangeY * Math_SinApprox(s->angle);
            if (x < -s->rangeX) x = -s->rangeX;
        } else {
            x -= kFloat_50_StrokePointLinearRate * dt;
            if (x < lim) {
                x = lim;
                s->color = 0xA0FFFFFF;
            } else {
                s->color = -1;
            }
        }
    }

    if (Input_IsActionButtonDown(pad, 3, -1)) {
        float lim = s->rangeX * Math_SquareRootApprox(
            kFloat_1_StrokePointShared - (y * y) / (s->rangeY * s->rangeY));
        if (x == lim) {
            float t = x / s->rangeX;
            t = clampf(t);
            boundaryAngle = acos(t);
            if (y < kFloat_0_spin) {
                s->angle = fminf(kFloat_neg1_Spin * boundaryAngle + step, kFloat_0_spin);
            } else {
                s->angle = fmaxf(boundaryAngle - step, kFloat_0_spin);
            }
            x = s->rangeX * Math_CosApprox(s->angle);
            y = s->rangeY * Math_SinApprox(s->angle);
            if (x > s->rangeX) x = s->rangeX;
        } else {
            x += kFloat_50_StrokePointLinearRate * dt;
            if (x > lim) {
                x = lim;
                s->color = 0xA0FFFFFF;
            } else {
                s->color = -1;
            }
        }
    }

    // Phase 5: hold one movement sound while any direction is down, then publish
    // both the relative hit offset and its absolute HUD position.
    if ((Input_IsActionButtonDown(pad, 0, -1) || Input_IsActionButtonDown(pad, 1, -1)
        || Input_IsActionButtonDown(pad, 2, -1) || Input_IsActionButtonDown(pad, 3, -1))
        && s->soundPlaying == 0) {
            Sound_PlayByName(g_soundManager, strokePointMoveStrings + 0x17, 0, 1, strokePointMoveStrings, 0, kFloat_0_5_StrokePointSoundArgument, kFloat_200_StrokePointMove, kFloat_1_StrokePointShared);
            s->soundPlaying = 1;
    } else if (!Input_IsActionButtonDown(pad, 0, -1) && !Input_IsActionButtonDown(pad, 1, -1)
        && !Input_IsActionButtonDown(pad, 2, -1) && !Input_IsActionButtonDown(pad, 3, -1)) {
        if (s->soundPlaying != 0) {
            Sound_StopByAlias(g_soundManager, strokePointMoveStrings);
            s->soundPlaying = 0;
        }
    }

    s->hitX = x;
    s->hitY = y;
    s->hudX = x + s->centerX;
    s->hudY = y + s->centerY;
    }
    case 0: return;
    }
}

#pragma opt_propagation off
// Read, enable, reset or replace the spin selection according to the actor message code.
void CSpin_HandleMsg(CSpin* self, const void* args) {
    // Query cases only write caller storage; enable/reset/copy/set cases mutate
    // the selection. CopyBallSelection recomputes absolute position; SetIntegerHit
    // writes offsets alone and leaves absolute coordinates until later processing.
    int mode = ((const SpinMessageArguments*)args)->code;
    switch (mode) {
    case kSpinGetNormalizedHit: {
        unsigned char* state = (unsigned char*)InstanceTable_Get();
        state = *(unsigned char**)(state + 4);
        if (state[0xa] == 3) {
            float z = kFloat_0_spin;
            float* py = ((const SpinMessageArguments*)args)->y.output;
            float* px = ((const SpinMessageArguments*)args)->x.output;
            *px = z;
            *py = z;
        } else {
            float a = self->hitX;
            float d = kFloat_30_PointerHitRange;
            float* py = ((const SpinMessageArguments*)args)->y.output;
            a = a / d;
            float* px = ((const SpinMessageArguments*)args)->x.output;
            *px = a;
            float b = self->hitY;
            b = b / d;
            *py = b;
        }
        break;
    }
    case kSpinEnable: self->active=1; break;
    case kSpinDisable: self->active=0; break;
    case kSpinReset:
    {
        self->Reset();
        break;
    }
    case kSpinCopyBallSelection: {
        void* o=GolfBall__Fv();
        self->hitX=kFloat_30_PointerHitRange*clampf(((CGolfBall*)o)->m_initCurve);
        o=GolfBall__Fv();
        float cy=clampf(((CGolfBall*)o)->m_initSpin);
        float y=kFloat_30_PointerHitRange*cy;
        float x=self->hitX;
        float w=self->rangeX;
        float h=self->rangeY;
        float xx=x*x;
        float ww=w*w;
        float yy=y*y;
        self->hitY=y;
        float one=kFloat_1_StrokePointShared;
        float hh=h*h;
        float t1=xx/ww;
        float t2=yy/hh;
        float sum=t1+t2;
        if (sum < one) self->color=-1;
        else self->color=0xA0FFFFFF;
        self->active=1;
        self->hudY=self->hitY+self->centerY;
        self->hudX=self->hitX+self->centerX; break;
    }
    case kSpinGetColorAndScreenPosition:
        *((const SpinMessageArguments*)args)->color = self->active ? self->color : 0;
        // The color query also returns screen position through the following case.
    case kSpinGetScreenPosition:
      {
        float* px = ((const SpinMessageArguments*)args)->x.output;
        float v0 = self->hudX;
        float* py = ((const SpinMessageArguments*)args)->y.output;
        *px = v0;
        *py = self->hudY;
        break;
      }
    case kSpinGetRangeNormalizedHit:
      {
        float v0 = self->hitX / self->rangeX;
        float* px = ((const SpinMessageArguments*)args)->x.output;
        float* py = ((const SpinMessageArguments*)args)->y.output;
        *px = v0;
        *py = self->hitY / self->rangeY;
        break;
      }
    case kSpinGetHitPosition:
      {
        float* px = ((const SpinMessageArguments*)args)->x.output;
        float v0 = self->hitX;
        float* py = ((const SpinMessageArguments*)args)->y.output;
        *px = v0;
        *py = self->hitY;
        break;
      }
    case kSpinGetRange:
      {
        float* px = ((const SpinMessageArguments*)args)->x.output;
        float v0 = self->rangeX;
        float* py = ((const SpinMessageArguments*)args)->y.output;
        *px = v0;
        *py = self->rangeY;
        break;
      }
    case kSpinSetIntegerHit: {
        unsigned int a = ((const SpinMessageArguments*)args)->x.bits;
        unsigned int b = ((const SpinMessageArguments*)args)->y.bits;
        union { unsigned int u[2]; double d; } ca, cb;
        ca.u[PPC_DW_HI] = 0x43300000u; ca.u[PPC_DW_LO] = a ^ 0x80000000u;
        cb.u[PPC_DW_HI] = 0x43300000u; cb.u[PPC_DW_LO] = b ^ 0x80000000u;
        self->hitX = (float)(ca.d - kDouble_SignedIntConversionBias_StrokePoint);
        self->hitY = (float)(cb.d - kDouble_SignedIntConversionBias_StrokePoint);
        break;
    }
    default: break;
    }
}
#pragma opt_propagation reset

// Return the spin actor type descriptor.
void* CSpin_GetRTTI() {
    return &s_spinTypeInfo;
}

// Initialize the trail module's full-turn constant.
void CTailEffect_InitializeAngularConstant() {
    s_tailEffectAngularPeriod = kFloat_6_2831855;
}

// Initialize raw/smoothed trail rings and load their bitmap texture.
void* CTailEffect_Ctor(void* self, void* args) {
    TTail* t = (TTail*)self;
    float fw = kFloat_0_3_TailEffect;
    float fz = kFloat_0_TailEffect;

    t->records.data = 0;
    t->vt = __vt__11CTailEffect;
    t->records.size = 0;
    t->records.capacity = 0;
    t->pendingRecords.data = 0;
    t->pendingRecords.size = 0;
    t->pendingRecords.capacity = 0;
    t->c54 = 0;
    t->textureGeneratorVtable = __vt__10CGeneTex3D;
    t->renderBuffer = 0;
    t->textureId = 0;
    t->writeIndex = 0;
    t->readIndex = 0;
    t->c44 = 0;
    t->interpolationTime = fz;
    t->pendingReadIndex = 0;
    t->pendingWriteIndex = 0;
    t->recordAge = fw;

    tbuf_reserve(t->records, kTrailRecordCapacity);
    if (t->pendingRecords.capacity < kTrailRecordCapacity) tbuf_grow(t->pendingRecords);
    unsigned int v = *(unsigned int*)args;
    const char* name = (v >> 31) == 0 ? (const char*)((char*)args + 1) : *(const char**)((char*)args + 8);
    t->textureId = (int)ResourceManager_LoadBitmap(g_resourceManager, name, 0, 0, 0);
    return self;
}

// Free a trail-vector backing allocation and optionally its owner.
void* dtor_801FC110(void* self, int shouldDelete) {
    return std::__vector_deleting_dtor((TrailRecordVector*)self, shouldDelete);
}

// Release the trail texture, render buffer and both record vectors.
void* CTailEffect_dtor(void* self, int shouldDelete) {
    if (self) {
        void** p = (void**)self;
        void* x = AsCTailEffect(self)->texture;
        *p = (void*)__vt__11CTailEffect;
        if (x) { ResourceManager_ReleaseId((void*)g_resourceManager, x); AsCTailEffect(self)->texture = 0; }
        if ((char*)self + kTrailTextureGeneratorOffset) {
            x = AsCTailEffect(self)->renderBuffer;
            AsCTailEffect(self)->textureGeneratorVtable = (void*)__vt__10CGeneTex3D;
            if (x) { __dla__FPv(x); AsCTailEffect(self)->renderBuffer = 0; }
        }
        if (&AsCTailEffect(self)->pendingRecords) { // +0x10
            if (&AsCTailEffect(self)->pendingRecords) {
                if (&AsCTailEffect(self)->pendingRecords) {
                    if (&AsCTailEffect(self)->pendingRecords) {
                        x = AsCTailEffect(self)->pendingRecords; // +0x10
                        if (x) {
                            volatile int* counter = (volatile int*)&AsCTailEffect(self)->pendingCount; // +0x14
                            *counter = *counter - *counter;
                            appFreeUni__FPv(x);
                        }
                    }
                }
            }
        }
        if (&AsCTailEffect(self)->records) { // +4
            if (&AsCTailEffect(self)->records) {
                if (&AsCTailEffect(self)->records) {
                    if (&AsCTailEffect(self)->records) {
                        x = AsCTailEffect(self)->records; // +4
                        if (x) {
                            volatile int* counter = (volatile int*)&AsCTailEffect(self)->recordCount; // +8
                            *counter = *counter - *counter;
                            appFreeUni__FPv(x);
                        }
                    }
                }
            }
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

#pragma opt_strength_reduction off
// Invalidate the thirty rendered records and reset both ring cursors.
void CTailEffect_ClearTrail(void* self) {
    struct Record { unsigned char flag; char pad[31]; };
    for (int i = 0; i < kTrailRecordCapacity; ++i) {
        (*(Record**)&AsCTailEffect(self)->records)[i].flag = 0;
    }
    char* data = *(char**)&AsCTailEffect(self)->records;
    int value = AsCTailEffect(self)->recordCount;
    AsCTailEffect(self)->readIndex = 0;
    AsCTailEffect(self)->recordCount -= (data + value) - data;
    AsCTailEffect(self)->writeIndex = 0;
    AsCTailEffect(self)->pendingReadIndex = 0;
    AsCTailEffect(self)->pendingWriteIndex = 0;
}
#pragma opt_strength_reduction reset

#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
// Append endpoint samples and rebuild a uniform cubic B-spline trail from four-sample spans.
// first/second are borrowed 3D endpoints in the same space as the rendered trail.
// Each raw record is one cross-trail edge, not two successive positions:
// time ->     p0       p1       p2       p3
// edge A:     A0 ------ A1 ------ A2 ------ A3
// edge B:     B0 ------ B1 ------ B2 ------ B3
// Interpolate the A and B curves independently with identical weights; rendered
// quads join adjacent output edges. These are uniform B-spline control points,
// so the curve generally does not pass through the individual input endpoints.
// Both rings have 30 slots and retain at most 29 edges (read==write means empty).
void CTailEffect_AddSegment(void* self, void* first, void* second) {
    struct Record { unsigned char flag; float age; float a[3]; float b[3]; };
    Record input;
    Record output;
    float va[3];
    float vb[3];
    char* s = (char*)self;
    // Phase 1: unchanged endpoints retire one pending edge; changed endpoints
    // append a raw edge and drop the oldest if the pending ring becomes full.
    if (Vec3_NearlyEqual(AsCTailEffect(s)->previousFirst, first) && Vec3_NearlyEqual(AsCTailEffect(s)->previousSecond, second)) {
        if (AsCTailEffect(s)->pendingReadIndex != AsCTailEffect(s)->pendingWriteIndex)
            AsCTailEffect(s)->pendingReadIndex = (AsCTailEffect(s)->pendingReadIndex + 1) % kTrailRecordCapacity;
    } else {
        *(WVector*)input.a = *(WVector*)first;
        *(WVector*)input.b = *(WVector*)second;
        input.flag = 1;
        input.age = AsCTailEffect(s)->recordAge;
        Record* data;
        int next, old;
        old = AsCTailEffect(s)->pendingWriteIndex;
        data = *(Record**)&AsCTailEffect(s)->pendingRecords;
        next = old + 1;
        AsCTailEffect(s)->pendingWriteIndex = next;
        Record* rec = data + old;
        rec->flag = input.flag;
        rec->age = input.age;
        *(WVector*)rec->a = *(WVector*)input.a;
        *(WVector*)rec->b = *(WVector*)input.b;
        AsCTailEffect(s)->pendingWriteIndex %= kTrailRecordCapacity;
        if (AsCTailEffect(s)->pendingWriteIndex == AsCTailEffect(s)->pendingReadIndex)
            AsCTailEffect(s)->pendingReadIndex = (AsCTailEffect(s)->pendingReadIndex + 1) % kTrailRecordCapacity;
        *(WVector*)&AsCTailEffect(s)->previousFirst = *(WVector*)first;
        *(WVector*)&AsCTailEffect(s)->previousSecond = *(WVector*)second;
        int dist;
        int head, tail;
        tail = AsCTailEffect(s)->pendingReadIndex;
        head = AsCTailEffect(s)->pendingWriteIndex;
        dist = head - tail;
        if (head < tail) dist = kTrailRecordCapacity - (tail - head);
        // Phase 2: rebuild the rendered ring from every complete four-edge span.
        AsCTailEffect(s)->readIndex = 0;
        AsCTailEffect(s)->writeIndex = 0;
        if (dist > 3) {
            const float& f25 = kFloat_5_TailEffect;
            const float& f26 = kFloat_3_TailEffect;
            const float& f27 = kFloat_neg1_TailEffect;
            const float& f28 = kFloat_1_TailEffect;
            const float& f29 = kFloat_6_TailEffect;
            const float& f30 = kFloat_4_TailEffect;
            const float& f31 = kFloat_neg3_TailEffect;
            for (int i = 0; i < dist - 3; ++i) {
                int base = i + AsCTailEffect(s)->pendingReadIndex;
                int p0 = base % kTrailRecordCapacity;
                int p1 = (base + 1) % kTrailRecordCapacity;
                int p2 = (base + 2) % kTrailRecordCapacity;
                int p3 = (base + 3) % kTrailRecordCapacity;
                // Five samples at t=0,1/5,...,4/5; t=1 belongs to the next span.
                // Numerators e1/e2/e3/t3 correspond to p0/p1/p2/p3:
                // (1-3t+3t^2-t^3), (4-6t^2+3t^3), (1+3t+3t^2-3t^3), t^3.
                // Divide their weighted sum by 6. Preserve evaluation order.
                for (int j = 0; j < kTrailSamplesPerSpan; ++j) {
                    float t3, e1, e2, e3, e7, e8, t, t2;
                    Record* data = *(Record**)&AsCTailEffect(s)->pendingRecords;
                    Record* p = data + p0;
                    Record* q = data + p1;
                    Record* r = data + p2;
                    Record* u = data + p3;
                    t = (float)j / f25;
                    t2 = t * t;
                    t3 = t2 * t;
                    e8 = f26 * t2;
                    e7 = f27 * t3 + e8;
                    e7 = -(f26 * t - e7);
                    e1 = f28 + e7;
                    e2 = f26 * t3 - f29 * t2;
                    e2 = f30 + e2;
                    float e1b = f31 * t3 + e8;
                    e8 = f26 * t + e1b;
                    e3 = f28 + e8;
                    va[0] = (t3*u->a[0] + (e3*r->a[0] + (e1*p->a[0] + e2*q->a[0]))) / f29;
                    va[1] = (t3*u->a[1] + (e3*r->a[1] + (e1*p->a[1] + e2*q->a[1]))) / f29;
                    va[2] = (t3*u->a[2] + (e3*r->a[2] + (e1*p->a[2] + e2*q->a[2]))) / f29;
                    vb[0] = (t3*u->b[0] + (e3*r->b[0] + (e1*p->b[0] + e2*q->b[0]))) / f29;
                    vb[1] = (t3*u->b[1] + (e3*r->b[1] + (e1*p->b[1] + e2*q->b[1]))) / f29;
                    vb[2] = (t3*u->b[2] + (e3*r->b[2] + (e1*p->b[2] + e2*q->b[2]))) / f29;
                    *(WVector*)output.a = *(WVector*)va;
                    *(WVector*)output.b = *(WVector*)vb;
                    output.flag = 1;
                    output.age = AsCTailEffect(s)->recordAge;
                    Record* dest = *(Record**)&AsCTailEffect(s)->records + AsCTailEffect(s)->writeIndex;
                    dest->flag = output.flag;
                    dest->age = output.age;
                    *(WVector*)dest->a = *(WVector*)output.a;
                    *(WVector*)dest->b = *(WVector*)output.b;
                    AsCTailEffect(s)->writeIndex = (AsCTailEffect(s)->writeIndex + 1) % kTrailRecordCapacity;
                    if (AsCTailEffect(s)->writeIndex == AsCTailEffect(s)->readIndex)
                        AsCTailEffect(s)->readIndex = (AsCTailEffect(s)->readIndex + 1) % kTrailRecordCapacity;
                }
            }
        }
    }
}
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset

// Age rendered trail entries, advance expired entries and clear an empty ring.
void CTailEffect_UpdateLifetimes(Fn801FC824Object* self, float dt) {
    int current = self->current;
    int count = self->count;
    int remaining = count - current;
    if (count < current) remaining = kTrailRecordCapacity - (current - count);
    if (!remaining) return;
    for (int i = 0; i < remaining; ++i) {
            int index = (current + i) % kTrailRecordCapacity;
        Fn801FC824Entry* entry = &self->entries[index];
        if (entry->active) {
            entry->time -= dt;
            if (self->entries[index].time <= kFloat_0_TailEffect) self->current = (self->current + 1) % kTrailRecordCapacity;
        }
    }
    int count2 = self->count;
    int current2 = self->current;
    int left = count2 - current2;
    left = (count2 < current2) ? kTrailRecordCapacity - (current2 - count2) : left;
    if (left) return;
    for (int i = 0; i < 5; ++i) {
        self->entries[i * 6].active = 0;
        self->entries[i * 6 + 1].active = 0;
        self->entries[i * 6 + 2].active = 0;
        self->entries[i * 6 + 3].active = 0;
        self->entries[i * 6 + 4].active = 0;
        self->entries[i * 6 + 5].active = 0;
    }
    int saved = AsCTailEffect(self)->recordCount;
    self->current = 0;
    AsCTailEffect(self)->recordCount = saved - saved;
    self->count = 0;
    self->pendingReadIndex = 0;
    self->pendingWriteIndex = 0;
}

// Emit textured trail edges with per-entry opacity using the current scene renderer.
void CTailEffect_Draw(Fn801FC998Obj* self) {
    int head;
    Fn801FC998Data* data;
    int tail;
    int span;
    tail = self->tail;
    head = self->head;
    data = (Fn801FC998Data*)s_tailEffectRenderStateTables;
    span = head - tail;
    if (head < tail) span = kTrailRecordCapacity - (tail - head);
    if (span >= 2) {
        Fn801FC998Block0 tex = data->b0;
        Fn801FC998Block2 b;
        Fn801FC998Block1 a = data->b1;
        b = data->b2;
        Engine_SetRenderStateList(&tex, 15);
        Engine_SetTextureStageStateList(0, &a, 6);
        Engine_SetTextureStageParameterList(0, &b, 5);
        char* renderer = (char*)Camera_GetRenderState(g_view);
        SetTransform(0, renderer + 0x20);
        SetTransform(1, renderer + 0x60);
        Engine_SetType7Resource(0);
        self->sub.begin();
        int idx = self->index;
        Fn801FC998Mgr* mgr = ((Fn801FC998Game*)g_view)->mgr;
        Engine_SetTextureStageResource(0, mgr->get70(idx));
        mgr = ((Fn801FC998Game*)g_view)->mgr;
        Engine_SetTexturePalette(0, mgr->get74(idx));
        SetTransform(2, kIdentityMatrix4x4);
        SetTransform(11, kIdentityMatrix4x4);
        Engine_SetTextureStageParameter(0, 0, 1);
        Engine_SetTextureStageParameter(0, 1, 1);
        Engine_SetCachedRenderState(13, 4);
        Engine_SetCachedRenderState(14, 1);
        Engine_SetCachedRenderState(5, 0);
        if (self->sub.count != (unsigned char)(span * 2)) {
            self->sub.count = self->sub.resize((unsigned char)(span * 2));
        }
        int base;
        int i;
        i = 0;
        base = 0;
        for (; i < span; ++i) {
            for (int side = 0; side < 2; ++side) {
                int e = (self->tail + i) % kTrailRecordCapacity;
                Fn801FC998Vec pos = side != 0 ? self->edges[e].b : self->edges[e].a;
                int n = side + base;
                Fn801FC998Vec p8 = pos;
                if (self->sub.count > (unsigned char)n) {
                    Fn801FC998Vec p14 = pos;
                    self->sub.position((unsigned char)n, &p14);
                }
                if (self->sub.count > (unsigned char)n) {
                    self->sub.texCoord((unsigned char)n, (float)i / (float)span, (float)(1 - side));
                }
                if (self->sub.count > (unsigned char)n) {
                    float t = kFloat_255_TailEffect * (float)i;
                    self->sub.color((unsigned char)n, (unsigned char)(int)(t / (float)span), 255, 255, 255);
                }
            }
            base += 2;
        }
        if (self->sub.count != 0) {
            self->sub.finish(0x98, GX_GetPrimitiveCount(self->sub.count, 0x98));
        }
    }
}

// Free the owned trail auxiliary allocation and optionally its owner.
void* dtor_801FCE48(Dtor801FCE48Object* self, int shouldDelete) {
    if (self) {
        if (self->data) {
            union { int value; unsigned int bits; } count;
            count.value = *(volatile int*)&self->count;
            self->count = count.value - count.bits;
            appFreeUni__FPv(self->data);
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Initialize the bubble module's full-turn constant.
void CBubble_InitializeAngularConstant() {
    s_twoPi_801FB14C = kFloat_6_2831855;
}

// Create the bubble overlay and shared font renderer, then initialize its text state.
Fn801FCEC4Object::Fn801FCEC4Object(const char* resource) : font(0), overlay(0), text() {
    float three;
    float minusOne;
    float one;
    one = kFloat_1_BubbleText_805DFC58;
    minusOne = kFloat_neg1_MessageOverlay;
    three = kFloat_3_BubbleText;
    visibilityScale = one;
    opacity = minusOne;
    fadeSecondsRemaining = three;
    overlay = WOverlay_Create(g_resourceManager, resource, 0);
    font = Runtime_DynamicCast(((MessageFontProvider*)g_chatMsg)->defaultFont, 0,
                                &__RTTI__10WFixedFont, &__RTTI__5WFont, 0);
    WFixedFont_CreateAndRegisterMsgDraw(font, &reserved18[8], 1);
}

// Release a bubble's text renderer, overlay and heap-backed text.
void* CaddieMessageRenderer_Destroy(Fn801FCF5CObject* self, int shouldDelete) {
    if (self) {
        if (g_resourceManager) {
            if (self->overlay) {
                Object_DeleteVirtual(g_resourceManager, self->overlay);
                self->overlay = 0;
            }
        }
        if (!Sing_805DC650()) {
            if (!s_pointerPairListInitialized) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, (void*)MessageDrawPairList_Dtor, s_bubbleMessageDrawPairListDtorRegistration);
                s_pointerPairListInitialized = 1;
            }
            Sing_805DC650() = s_messageDrawPairList;
        }
        PointerPairList_RemoveByKey(Sing_805DC650(), self->renderer);
        if (self->renderer) {
            if (self->renderer) {
((Dm0*)self->renderer)->v08(1);
            }
            self->renderer = 0;
        }
        if (&self->textFlags) {
            if (self->textFlags >> 31) appFreeUni__FPv(self->heapText);
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}

// Count down display time, then reduce text opacity while the bubble is active.
void CBubble_UpdateTextFadeOut(Fn801FD080Object* self, float dt) {
    if (!GolfTips_IsDialogActive(self))
        self->fadeSecondsRemaining -= dt;
    float timer = self->fadeSecondsRemaining;
    float zero = kFloat_0_U801FB14C;
    if (timer < zero && self->opacity > zero) {
        float nv = self->opacity - kFloat_255_BubbleText * dt;
        self->opacity = nv;
        if (nv <= zero)
            CMsgDraw_ResetTextState(self->renderer);
    }
}

#pragma push
#pragma use_lmw_stmw on
// Replace the wide caddie message, measure its text and restart its display timer.
void CBubble_SetCaddieMessage(Fn801FD110Object* self, wchar_t* input) {
    if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer)) return; // +0x3C1
    unsigned int len0;
    char* st = (char*)g_sharedDoc;
    char* path = (char*)ItemInventory_FindCaddieData(AsCSharedDoc(st)->itemInventory, // +0xA04
        *(void**)(st + AsCGolfDoc(*(void**)&AsCSharedDoc(st)->golfDoc)->currentPlayer * 0x1D4 + 0x1B8));
    if (!path) return;
    Fn801FD110String temp;
    len0 = *((unsigned char*)&temp.flags) & 0x7F;
    volatile Fn801FD110Ch c0;
    Fn801FD110Ch k0;
    k0.v = c0.v;
    unsigned int pathLength = strlen(path);
    Fn801FD110Ch* allocator0 = &k0;
    char* pathEnd = path + pathLength;
    String_ReplaceRange(&temp, 0, len0, path, pathEnd, allocator0);
    unsigned int pos1;
    char* s1 = s_caddieMessagePathSuffix;
    if ((temp.flags >> 31) == 0) pos1 = *((unsigned char*)&temp.flags) & 0x7F;
    else pos1 = temp.length;
    volatile Fn801FD110Ch c1;
    Fn801FD110Ch k1;
    k1.v = c1.v;
    String_ReplaceRange(&temp, pos1, 0, s1, s1 + strlen(s1), &k1);
    if (!(self->textFlags >> 31)) {
        *(unsigned short*)((char*)&self->textFlags + 2) = 0;
        *((signed char*)&self->textFlags) &= (signed char)0x80;
    } else {
        *(unsigned short*)self->heapText = 0;
        self->textLength = 0;
    }
    int pos28;
    wchar_t* s2 = s_caddieMessageWidePrefix;
    if ((self->textFlags >> 31) == 0) pos28 = *((unsigned char*)&self->textFlags) & 0x7F;
    else pos28 = (int)self->textLength;
    volatile Fn801FD110Ch c2;
    Fn801FD110Ch k2;
    k2.v = c2.v;
    unsigned int len2 = wcslen(s2);
    Fn801FD110Ch* allocator2 = &k2;
    wchar_t* end2 = s2 + len2;
    WString_ReplaceRange(&self->textFlags, 0, pos28,
        s2, end2, allocator2);
    int pos28b;
    if ((self->textFlags >> 31) == 0) pos28b = *((unsigned char*)&self->textFlags) & 0x7F;
    else pos28b = (int)self->textLength;
    volatile Fn801FD110Ch c3;
    Fn801FD110Ch k3;
    k3.v = c3.v;
    unsigned int len3 = wcslen(input);
    Fn801FD110Ch* allocator3 = &k3;
    wchar_t* end3 = input + len3;
    WString_ReplaceRange(&self->textFlags, pos28b, 0,
        input, end3, allocator3);
    CMsgDraw_ResetTextState(self->renderer);
    void* text;
    if ((self->textFlags >> 31) == 0) text = (char*)&self->textFlags + 2;
    else text = self->heapText;
    Text_SetContent(self->renderer, text);
    CMsgDraw_GetTextExtent(self->renderer, &self->textWidth);
    float full;
    float three;
    three = kFloat_3_BubbleText;
    full = kFloat_255_BubbleText;
    *(volatile float*)&self->fadeSecondsRemaining = three;
    *(volatile float*)&self->opacity = full;
}
#pragma opt_common_subs reset
#pragma pop

enum BubbleHorizontalAlignment { kBubbleAnchorLeft = 0, kBubbleAnchorCenter = 1, kBubbleAnchorRight = 2 };
enum { kBubbleBackgroundDrawFlags = 0x80000 };

// Clamp an aligned balloon to the screen, draw its three background pieces and its text.
// x/y are HUD pixels; the balloon sits above the anchor and is clamped to the
// screen width and 480-pixel reference height. Other mode values behave as left.
void CBubble_DrawAlignedBalloon(Fn801FD360Object* self, void* camera, float x, float y, int mode) {
    if (self->opacity < kFloat_0_U801FB14C) return;
    if (fn_801FD360Tiny(self->visibilityScale - s_bubbleVisibilityZero)) return;
    if (SharedDoc_IsPlayerFlag0Set(AsCGolfDoc(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer)) return; // +0x3C1
    if (((int (*)())GolfTips_IsDialogActive)()) return;
    // Phase 1: align the outer width, reject offscreen anchors, then clamp X.
    float width = kFloat_12_BubbleText + self->width;
    Fn801FD360Quad balloonAtlasRect;
    Fn801FD360Quad pieceAtlasRect;
    Fn801FD360Quad pieceScreenRect;
    Fn801FD360Quad oneLineAtlasRect, twoLineAtlasRect, threeLineAtlasRect;
    Fn801FD360Quad textPosition;
    Fn801FD360Pair balloonTopLeft;
    switch (mode) {
    case kBubbleAnchorLeft: balloonTopLeft.x = x; break;
    case kBubbleAnchorCenter: balloonTopLeft.x = x - kFloat_0_5_BubbleLayoutAndAtlas * width; break;
    case kBubbleAnchorRight: balloonTopLeft.x = x - width; break;
    default: balloonTopLeft.x = x; break;
    }
    if (balloonTopLeft.x > kSceneEffectScreenWidth) return;
    if (balloonTopLeft.x < -width) return;
    if (balloonTopLeft.x + width >= kSceneEffectScreenWidth) balloonTopLeft.x = kSceneEffectScreenWidth - width;
    if (balloonTopLeft.x < kFloat_0_U801FB14C) balloonTopLeft.x = kFloat_0_U801FB14C;
    balloonTopLeft.y = kFloat_1_BubbleText_805DFC58 + (y - kFloat_20_BubbleAnchorOffset - (kFloat_6_BubbleTextInset + self->height));
    // Phase 2: select the authored atlas row from the parsed line count.
    switch (((CMsgDraw*)self->renderer)->parsedLineCount) {
    case 1:
        oneLineAtlasRect.x = kFloat_0_U801FB14C; oneLineAtlasRect.y = kFloat_0_U801FB14C; oneLineAtlasRect.z = kFloat_0_5_BubbleLayoutAndAtlas; oneLineAtlasRect.w = kFloat_0_234375_BubbleAtlas;
        balloonAtlasRect = oneLineAtlasRect;
        break;
    case 2:
        twoLineAtlasRect.x = kFloat_0_U801FB14C; twoLineAtlasRect.y = kFloat_0_375_BubbleAtlas; twoLineAtlasRect.z = kFloat_0_5_BubbleLayoutAndAtlas; twoLineAtlasRect.w = kFloat_0_3125_BubbleAtlas;
        balloonAtlasRect = twoLineAtlasRect;
        break;
    case 3:
default:
        threeLineAtlasRect.x = kFloat_0_5_BubbleLayoutAndAtlas; threeLineAtlasRect.y = kFloat_0_U801FB14C; threeLineAtlasRect.z = kFloat_0_5_BubbleLayoutAndAtlas; threeLineAtlasRect.w = kFloat_0_375_BubbleAtlas;
        balloonAtlasRect = threeLineAtlasRect;
        break;
    }
    // Phase 3: pack each draw API's alpha order and clamp vertical placement.
    unsigned int alpha = Runtime_ConvertDoubleToUnsigned(self->opacity * self->visibilityScale);
    unsigned int glyphTint = alpha | 0xFFFFFF00;
    unsigned int backgroundTint = (alpha << 24) | 0x00FFFFFF;
    if (balloonTopLeft.y < kFloat_5_Bubble) balloonTopLeft.y = kFloat_5_Bubble;
    float maxY = kFloat_480_BubbleReferenceHeight - (kFloat_6_BubbleTextInset + self->height);
    if (balloonTopLeft.y >= maxY) balloonTopLeft.y = maxY;
    // Phase 4: draw left cap, stretchable middle, and right cap in screen pixels.
    pieceAtlasRect.w = balloonAtlasRect.w;
    pieceAtlasRect.y = balloonAtlasRect.y;
    pieceAtlasRect.x = balloonAtlasRect.x;
    pieceAtlasRect.z = kFloat_0_046875_BubbleAtlasSideCap;
    pieceScreenRect.x = balloonTopLeft.x;
    pieceScreenRect.y = balloonTopLeft.y;
    pieceScreenRect.z = kFloat_6_BubbleTextInset;
    pieceScreenRect.w = kFloat_6_BubbleTextInset + self->height;
    WOverlay_DrawTransformedRect(self->text, camera, &pieceAtlasRect, kFloat_0_U801FB14C, &pieceScreenRect, kBubbleBackgroundDrawFlags, backgroundTint, 0);
    pieceAtlasRect.x = pieceAtlasRect.x + pieceAtlasRect.z;
    pieceAtlasRect.z = balloonAtlasRect.z - kFloat_0_09375_BubbleText;
    pieceScreenRect.x = pieceScreenRect.x + pieceScreenRect.z;
    pieceScreenRect.z = self->width;
    WOverlay_DrawTransformedRect(self->text, camera, &pieceAtlasRect, kFloat_0_U801FB14C, &pieceScreenRect, kBubbleBackgroundDrawFlags, backgroundTint, 0);
    pieceAtlasRect.x = pieceAtlasRect.x + pieceAtlasRect.z;
    pieceAtlasRect.z = kFloat_0_046875_BubbleAtlasSideCap;
    pieceScreenRect.x = pieceScreenRect.x + pieceScreenRect.z;
    pieceScreenRect.z = kFloat_6_BubbleTextInset;
    WOverlay_DrawTransformedRect(self->text, camera, &pieceAtlasRect, kFloat_0_U801FB14C, &pieceScreenRect, kBubbleBackgroundDrawFlags, backgroundTint, 0);
    // Phase 5: place inset text, apply its separate tint, and reveal all glyphs.
    textPosition.x = kFloat_6_BubbleTextInset + balloonTopLeft.x;
    textPosition.y = kFloat_3_BubbleText + balloonTopLeft.y;
    textPosition.z = textPosition.x;
    void* textRenderer = self->renderer;
    textPosition.w = textPosition.y;
    Text_SetPosition(textRenderer, &textPosition);
    CMsgDraw_ResetTextState(self->renderer);
    *(unsigned int*)&((CMsgDraw*)self->renderer)->colorMultiplier = glyphTint; // 0x801FD360: renderer +0x68
    Text_SetContent(self->renderer, !(self->textFlags >> 31) ? (void*)((char*)&self->textFlags + 2) : self->heapText);
    CMsgDraw_ClearFormattingAndRevealAll(self->renderer);
    Text_DrawGlyphs(self->renderer);
    CMsgDraw_ResetTextState(self->renderer);
}

// Report whether this bubble is visible for the current non-suppressed player.
int CBubble_IsActiveForCurrentPlayer(Fn801FD7ACObject* self) {
    Fn801FD7ACState* state = (Fn801FD7ACState*)g_sharedDoc;
    unsigned char flag = AsCGolfDoc(state->object)->currentPlayer; // +0x3C1
    if (SharedDoc_IsPlayerFlag0Set(flag))
        return 0;
    return self->value > kFloat_0_U801FB14C;
}

// Initialize the task module's full-turn constant.
void CTask_InitializeAngularConstant() {
    s_taskAngularPeriod = kFloat_6_2831855;
}

// Allocate and construct the main task container.
Fn801FD814Object* CTask_Create() {
    return new Fn801FD814Object;
}

// Allocate and construct the lightweight task-main actor.
CTaskMainConstructorView* CTaskMain_Create() {
    return new CTaskMainConstructorView;
}

#pragma push
#pragma use_lmw_stmw on
void* dtor_801FDAF0(Fn801FDAF0Object*, int);
// Initialize task hash/list storage and the auxiliary hand actor with exception cleanup.
void* CTask_Ctor(void* object) {
    Fn801FD8A8Object* self = (Fn801FD8A8Object*)object;
    ActorFactoryInterface_InitVtable(self);
    bool tableReady = false;
    bool handReady = false;
    try {
    self->vtable = __vt__5CTask;
    self->vtable04 = (char*)__vt__5CTask + kTaskAuxVtableOffset;
    self->vtable08 = (char*)__vt__5CTask + kTaskActorVtableOffset;
    self->flag = 1;
    self->items = 0;
    OSInitMutex((OSMutex*)self->mutex);
    OSMutex* list = (OSMutex*)self->mutex;
    OSLockMutex(list);
    int count = 16;
    int i;
    self->first = 0;
    self->head = 0;
    self->freeNodes = 0;
    self->nodeBlockSize = count;
    self->iterationCursor = 0;
    self->size = count;
    if (self->size > 0) {
        try {
            self->items = (void**)appMallocUni__FUl(0x40);
        } catch (...) {
            OSUnlockMutex(list);
            throw;
        }
        self->mask = self->size - 1;
        for (i = 0; i < self->size; ++i) self->items[i] = 0;
    } else {
        self->items = 0;
    }
    OSUnlockMutex(list);
    tableReady = true;
    self->mainActor = 0;
    self->ownedActor = 0;
    CAnotherHand_Ctor(self->reserved50);
    handReady = true;
    list = (OSMutex*)self->mutex;
    self->unk3A0 = 0;
    self->unk3A4 = 0;
    self->unk3A5 = 0;
    OSLockMutex(list);
    if (self->first && self->head) {
        self->first->prev->next = self->head->next;
        self->head->next->prev = self->first->prev;
        self->head->next = self->first;
        self->first->prev = self->head;
        self->first = 0;
    } else {
        Fn801FD8A8ListNode* node;
        Fn801FD8A8ListNode* next;
        Fn801FD8A8ListNode* head;
        OSMutex* lock = (OSMutex*)self->mutex;
        while ((node = self->first) != 0) {
            OSLockMutex(lock);
            node->next->prev = node->prev;
            node->prev->next = node->next;
            next = node->next == node ? 0 : node->next;
            OSUnlockMutex(lock);
            node = self->first;
            head = self->head;
            OSLockMutex(lock);
            if (head == 0) {
                node->prev = node;
                node->next = node;
                OSUnlockMutex(lock);
            } else {
                node->next = head;
                node->prev = head->prev;
                head->prev = node;
                node->prev->next = node;
                OSUnlockMutex(lock);
                node = head;
            }
            self->head = node;
            self->first = next;
        }
    }
    for (i = 0; i < self->size; ++i) ((void**)self->items)[i] = 0;
    OSUnlockMutex(list);
    self->unk3A0 = 0;
    return self;
    } catch (...) {
        if (handReady) CAnotherHand_dtor(self->reserved50, -1);
        if (tableReady) dtor_801FDAF0((Fn801FDAF0Object*)((char*)self + (int)&((Fn801FDBC8Object*)0)->first), -1);
        ((void (*)(void*, int))__dt__11AuxMenuBaseFv)((char*)self + 8, -1);
        dtor_8029EA18(self, -1);
        throw;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Destroy mutex-protected task list nodes and their hash backing storage.
void* dtor_801FDAF0(Fn801FDAF0Object* self, int shouldDelete) {
    if (self) {
        {
            WInstanceLock _lock(self->mutex);
            if (self->head) {
                Fn801FDAF0Link* node;
                while ((node = self->head) != 0) {
                    self->head = self->remove(node);
                    appFreeUni__FPv(node);
                }
            }
            if (self->field1C) { appFreeUni__FPv(self->field1C); self->field1C = 0; }
        }
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Release task actors, auxiliary hand state, list nodes and hash storage.
void* CTask_dtor(Fn801FDBC8Object* self, int shouldDelete) {
    Fn801FDBC8Node* node;
    Fn801FDBC8Node* next;
    if (self) {
        self->vtable = __vt__5CTask;
        self->vtable04 = (char*)__vt__5CTask + kTaskAuxVtableOffset;
        self->vtable08 = (char*)__vt__5CTask + kTaskActorVtableOffset;
        CThread_IsFinished(self->reserved50);
        {
            WInstanceLock lock1(self->mutex);
            self->current = self->first;
            node = (Fn801FDBC8Node*)self->popFront();
        }
        while (node) {
            fn_801FDBC8Release(node->object);
            __dla__FPv(node);
            node = (Fn801FDBC8Node*)self->popFront();
        }
        {
            WInstanceLock lock2(self->mutex);
            if (self->first && self->freeNodes) {
                self->first->prev->next = self->freeNodes->next;
                self->freeNodes->next->prev = self->first->prev;
                self->freeNodes->next = self->first;
                self->first->prev = self->freeNodes;
                self->first = 0;
            } else {
                while ((node = self->first) != 0) {
                    next = self->remove(node);
                    self->freeNodes = self->addHead(self->first);
                    self->first = next;
                }
            }
            for (int i = 0; i < self->bucketCount; ++i) self->hashBuckets[i] = 0;
        }
        if (self->field4C) {
            fn_801FDBC8Release(self->field4C);
            self->field4C = 0;
        }
        CAnotherHand_dtor(self->reserved50, -1);
        if ((char*)self + (int)&((Fn801FDBC8Object*)0)->first) {
            WInstanceLock lock3(self->mutex);
            node = self->allocationBlocks;
            while (node) {
                next = self->remove(node);
                self->allocationBlocks = next;
                appFreeUni__FPv(node);
                node = self->allocationBlocks;
            }
            if (self->hashBuckets) { appFreeUni__FPv(self->hashBuckets); self->hashBuckets = 0; }
        }
        dtor_8029EA18(self, 0);
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma opt_common_subs reset
#pragma pop

// Set the task container's main actor handle.
void CTask_SetMainActor(Fn801FDF48Object* self, int value) {
    self->value = value;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621D0", _ctors_u801FB14C_0, CTailEffect_InitializeAngularConstant);
STATIC_INIT_ENTRY(".CRT$XCU804621D0", _ctors_u801FB14C_1, CBubble_InitializeAngularConstant);
STATIC_INIT_ENTRY(".CRT$XCU804621D0", _ctors_u801FB14C_2, CTask_InitializeAngularConstant);
#pragma pop
