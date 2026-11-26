// Balloon actors (CBubble) with their message and fade handling, the beach render state
// (CBeach), and the camera actor's creation and init.
// Also the token input buffers and string helpers that message text is read through.

#include <decomp/static_init.h>
#include "iff/IffStruct.h"
#include "classes/CCamera.h"
#include "classes/CameraTransform800D.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CGolfDoc.h"
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "game/types.h"
#include "game/wvector.h"
#include "game/golfball.h"

#include "sdk_protos.h"
#include "game/exception.h"
#include "classes/CBeach.h"
#include "classes/TokenInputState.h"
#include "classes/CBubble.h"
#include "classes/CGenericPrimitive.h"

extern "C" {

extern void* s_bubbleTypeInfo;
extern const float kFloat_6_2831855;
extern float s_twoPi_800D1108;
extern float s_tokenVectorTwoPi; // s_tokenVectorTwoPi.
extern float s_chatBubbleTwoPi;  // s_chatBubbleTwoPi.
extern char bubbleActorFactoryName;
extern char bubbleTaskClassName;
extern char __vt__7cTokenV[];
extern char __vt__7cTokenS[];
extern char chatBubbleStrings[];
extern const char waterResourceParseStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char s_actorTypeInfo;
extern char __vt__7CCamera[];
extern char __vt__7CBubble[];
extern char cTokenVtable[];
extern const float kFloat_0_camera;
extern float kBubbleGroundType2ScreenOffsetX, kBubbleGroundType2ScreenOffsetY;
extern const float kFloat_0_Bubble;
extern const float kBubbleFadeStartDistance, kBubbleFadeInverseDistanceSpan, kFloat_1_BubbleOpacity;
extern CSharedDoc* g_sharedDoc;
extern void* g_view;
extern char s_cameraTransform[];

extern void __dt__8CWaveletFv(void*, int);
extern void* IntPair_Set(void*, const char*, void*);
extern void* ActorFactoryRegistry_GetInstance();
extern void StringMap_SetByName(void*, void* (*)(), const char*);
extern void* TaskRegistry_GetInstance();
extern void TaskRegistry_AppendFactory(void*, const char*);
extern void* __nw__FUl(unsigned int);
extern void __ct__18NativeBase80201D18Fv(void*);
extern void IActor_AddSkipFlags(void*, int);
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern void* __nwa__FUl(unsigned int);
extern void CaddieMessageRenderer_Destroy(void*, int); // CaddieMessageRenderer_Destroy.

void* CBubble_Create();
extern void __ct__16Fn801FCEC4ObjectFPCc(void*, const char*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void* memcpy(void*, const void*, unsigned int);
extern void RBTree_RebalanceAfterInsert(void*, void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*);
extern void* Matrix_TransformPoint(void*, void*, void*);
extern void WView_ProjectViewPoint(void*, void*, void*, int);
extern void* GolfBall__Fv();
extern float WVectorLen__FRC7WVector(void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void IActor_DispatchMessage(void*, void*, const char*, int, int, int, int, int);

// Two independent balloons: message renderer, world anchor, and completion notification.
struct BubbleEntry {
    void* renderer;
    WVector position;
    unsigned char notifyWhenFinished;
    char padding[3];
};
enum {
    kBubbleCount = 2,
    kBubbleGroundType = 2,
    kBubbleHiddenDuringShot = 0x200,
    kShiftJisLeadBit = 0x80,
    kCameraSkipFlags = 0x84C82,
    kBubbleSkipFlags = 0x854C2
};
// Actor skip masks combine game-state bits: camera skips
// 0x2|0x80|0x400|0x800|0x4000|0x80000; bubble adds 0x40|0x1000
// and omits 0x800. Individual state meanings remain unresolved.
// The display's separate 0x200 gate suppresses balloons during the shot state.

struct ListPayload {
    int d[11];
};
struct ListNode {
    ListNode* left;
    ListNode* right;
    void* parent;
    ListPayload payload;
};
struct BubblePayloadTree {
    unsigned int size;
    ListNode* root;
    void* reserved;
    ListNode* last;
};

extern void __dt__Q33std6detail16char_array_guardFv(void*, int);

struct ListAllocation {
    char* pointer;
    char* allocator;
    // Guard a message array while borrowing its allocator marker.
    ListAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Release the array unless construction transferred it to the control block.
    ~ListAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct ListControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize one strong and one weak reference for the shared message owner.
    ListControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct ListControl : ListControlBase {
    char* pointer;
    // Attach the message array to its newly initialized shared owner.
    ListControl(char* p) {
        pointer = p;
    }
};
namespace std {
struct logic_error : exception {
    // Install the exception vtable in the first ABI word.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    // Initialize the exception's logic_error base vtable.
    logic_error() {
        SetTable(__vt__Q23std11logic_error);
    }

    ~logic_error();
};
}
namespace std {
struct length_error : logic_error {
    char* text;
    void* control;
    // Build the shared vector-length error message and install the derived vtable.
    length_error() {
        const char* message = waterResourceParseStrings + 0x0E;
        struct Allocator {
            char value;
            char pad[3];
            char second;
        } allocator;
        allocator.second = 0;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            ListAllocation allocation(p, &allocator.value);
            control = new ListControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        SetTable(__vt__Q23std12length_error);
    }

    ~length_error();
};
}
struct NodeGuard_u800D1108 {
    void* allocator;
    void* pointer;
    // Guard a newly allocated node until its list/tree insertion commits.
    NodeGuard_u800D1108(void* p, void* a) : pointer(p), allocator(a) {}
    // Free an uncommitted node, leaving its borrowed allocator marker untouched.
    ~NodeGuard_u800D1108() {
        if (pointer)
            appFreeUni__FPv(pointer);
    }
};
}
// Placement construction borrows already allocated payload storage.
inline void* operator new(size_t, void* p) {
    return p;
}
extern "C" {

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Allocate a 44-byte payload tree node, link it at the insertion point, and rebalance the owner tree.
ListNode* Payload44Tree_InsertAndRebalance(void* owner, ListNode** where, int before, int update, const void* source) {
    ListNode* node;
    volatile char allocator;
    unsigned int word;
    word = ((BubblePayloadTree*)owner)->size;
    if (word == 0xFFFFFFFF)
        throw std::length_error();
    allocator = 0;
    node = (ListNode*)appMallocDummy__FUl(0x38);
    NodeGuard_u800D1108 guard(node, &((BubblePayloadTree*)owner)->root);
    try {
        new (&node->payload) ListPayload(*(const ListPayload*)source);
    } catch (...) {
        throw;
    }
    node->right = 0;
    guard.pointer = 0;
    node->left = 0;
    if ((char*)node + IFF_OFFSET(ListNode, parent) != 0)
        *(void**)((char*)node + IFF_OFFSET(ListNode, parent)) = where;
    if (before)
        *where = node;
    else
        where[1] = node;
    ((BubblePayloadTree*)owner)->size = ((BubblePayloadTree*)owner)->size + 1;
    RBTree_RebalanceAfterInsert(node, ((BubblePayloadTree*)owner)->root);
    if (update)
        *(void**)&((BubblePayloadTree*)owner)->last = node;
    return node;
}
#pragma pop

struct BubbleOwner {
    char pad[0x18];
    BubbleEntry* entries;
};
void BubbleEntry_DefaultConstruct(void*); // BubbleEntry_DefaultConstruct.

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Allocate two balloon records and construct one caddie-message renderer for each.
void CBubble::OnLoad() {
    BubbleOwner* self = (BubbleOwner*)this;
    char* base;
    int zero;
    void* obj;
    int offset;
    int i;
    base = (char*)__nwa__FUl(sizeof(BubbleEntry) * kBubbleCount + 0x10); // + 16-byte array cookie
    if (base != 0)
        base = (char*)__construct_new_array((void*)base, (void*)BubbleEntry_DefaultConstruct, (void*)0,
                                            (size_t)sizeof(BubbleEntry), (size_t)kBubbleCount);
    self->entries = (BubbleEntry*)base;
    i = 0;
    offset = 0;
    zero = 0;
    for (; i < 2; ++i) {
        obj = __nw__FUl(0x30);
        if (obj != 0) {
            try {
                __ct__16Fn801FCEC4ObjectFPCc(obj, chatBubbleStrings);
            } catch (...) {
                __dl__FPv(obj);
                throw;
            }
        }
        *(void**)((char*)self->entries + offset) = obj;
        void* e = (void*)((char*)self->entries + offset);
        ((BubbleEntry*)e)->notifyWhenFinished = (unsigned char)zero;
        offset = offset + (int)sizeof(BubbleEntry);
    }
}
#pragma pop

struct Event {
    char pad[4];
    int type;
    void* value;
    void* worldPosition;
};
typedef BubbleEntry EventEntry;
struct EventOwner {
    char pad[0x18];
    EventEntry* entries;
};
#pragma push
#pragma opt_propagation off
// Set either balloon message and world anchor, and arm its completion notification.
void CBubble::HandleMsg(const Event* event) {
    EventOwner* self = (EventOwner*)this;
    switch (event->type) {
    case 0:
        ((CBubble*)self->entries[0].renderer)->SetCaddieMessage((wchar_t*)event->value);
        self->entries[0].position = *(WVector*)event->worldPosition;
        self->entries[0].notifyWhenFinished = 1;
        break;
    case 1:
        ((CBubble*)self->entries[1].renderer)->SetCaddieMessage((wchar_t*)event->value);
        self->entries[1].position = *(WVector*)event->worldPosition;
        self->entries[1].notifyWhenFinished = 1;
        break;
    default:
        break;
    }
}
#pragma pop

struct Buffer {
    virtual void slot0();
    virtual char* data3();
    int used;
    int inputLength;
    char pad0C[8];
    void* data;
};

#pragma push
#pragma use_lmw_stmw on
// Original 0x800D1108 unit: return from the inline scan when either delimiter matches.
static inline void ReadUntilDelimiterScan(Buffer* self, char* output, const char* input, int count, int& cursor,
                                          int& delimiterFound, int& wideDelimiter, int& delimiterOffset,
                                          int& delimiterIndex) {
    while (cursor < self->inputLength) {
        delimiterOffset = 0;
        delimiterIndex = 0;
        while (delimiterIndex < count) {
            unsigned char c = input[delimiterOffset];
            if ((signed char)c == self->data3()[cursor]) {
                if (c & kShiftJisLeadBit) {
                    ++delimiterOffset;
                    if (input[delimiterOffset] == self->data3()[cursor + 1]) {
                        delimiterFound = wideDelimiter = 1;
                        ++cursor;
                        return;
                    }
                } else {
                    delimiterFound = 1;
                    return;
                }
            } else if (c & kShiftJisLeadBit) {
                ++delimiterOffset;
            }
            ++delimiterIndex;
            ++delimiterOffset;
        }
        output[cursor - self->used] = self->data3()[cursor];
        if (((unsigned char*)self->data3())[cursor] & kShiftJisLeadBit) {
            ++cursor;
            if (cursor < self->inputLength) {
                char* data = self->data3();
                output[cursor - self->used] = data[cursor];
            }
        }
        ++cursor;
    }
}

// Copy through the next delimiter, preserving two-byte Shift-JIS characters and advancing the cursor.
int cToken_ReadUntilDelimiter(Buffer* self, char* output, const char* input, int count) {
    output[0] = 0;
    if (!self->data3())
        return 0;
    int cursor = self->used;
    int delimiterFound = 0;
    int wideDelimiter = 0;
    // Keep scan counters in the caller to preserve register order after inlining.
    int delimiterOffset;
    int delimiterIndex;
    ReadUntilDelimiterScan(self, output, input, count, cursor, delimiterFound, wideDelimiter, delimiterOffset,
                           delimiterIndex);
    if (delimiterFound && wideDelimiter)
        output[cursor - self->used - 1] = 0;
    else
        output[cursor - self->used] = 0;
    self->used = cursor < self->inputLength ? cursor + 1 : self->inputLength;
    return delimiterFound;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Remove leading/trailing characters in the supplied set in place, preserving two-byte leading characters.
char* String_TrimCharacterSet(char* text, void* set) {
    char* p;
    unsigned char c;
    int start;
    int end;
    start = 0;
    end = (int)strlen(text) - 1;
    p = text;
    while (start <= end && (c = (unsigned char)*p, strrchr((const char*)set, (signed char)c))) {
        if (c & kShiftJisLeadBit) {
            ++start;
            ++p;
        }
        ++start;
        ++p;
    }
    if (start <= end) {
        p = text + end;
        while (strrchr((const char*)set, (signed char)*p)) {
            --end;
            --p;
        }
        int count = end - start + 1;
        memmove(text, text + start, count);
        text[count] = 0;
    } else {
        text[0] = 0;
    }
    return text;
}
#pragma pop

// Copy input into an owned buffer, reallocating if needed, then reset the scan cursor.
int Buffer_SetData(Buffer* self, void* source, int size) {
    if (source == 0 || size < 1)
        return 0;
    if (size > self->inputLength) {
        if (self->data != 0)
            __dla__FPv(self->data);
        self->data = __nwa__FUl(size);
        if (self->data == 0) {
            self->inputLength = 0;
            return 0;
        }
    }
    memcpy(self->data, source, size);
    self->inputLength = size;
    self->used = 0;
    return 1;
}

struct VectorObject {
    void* vtable;
    char pad04[4];
    int size;
    int capacity;
    void* data;
};

// Release the token's owned array and optionally its object; return the original self.
void* cToken_dtor(VectorObject* self, int deletingFlag) {
    if (self != 0) {
        void* data = self->data;
        self->vtable = cTokenVtable;
        if (data != 0)
            __dla__FPv(data);
        if (deletingFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

struct DtorObject {
    void* vtable;
    char pad04[0x0C];
    void* data;
    void* data2;
};

// Release the string-token buffer and its base token storage, then optionally free the object.
void* cTokenS_dtor(DtorObject* self, int deletingFlag) {
    if (self != 0) {
        void* data = self->data2;
        self->vtable = __vt__7cTokenS;
        if (data != 0)
            __dla__FPv(data);
        if (self != 0) {
            data = self->data;
            self->vtable = cTokenVtable;
            if (data != 0)
                __dla__FPv(data);
        }
        if (deletingFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release the vector-token base storage and optionally free the wrapper.
void* cTokenV_dtor(VectorObject* self, int deletingFlag) {
    if (self != 0) {
        if (self != 0) {
            void* data = self->data;
            self->vtable = cTokenVtable;
            if (data != 0)
                __dla__FPv(data);
        }
        if (deletingFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

// Ensure room for one trailing NUL; expanding discards the previous buffer contents.
int FxSprayScriptBuffer_EnsureTerminatorCapacity(VectorObject* self) {
    if (self->size + 1 > self->capacity) {
        if (self->data != 0)
            __dla__FPv(self->data);
        self->data = __nwa__FUl(self->size + 1);
        if (self->data == 0) {
            self->capacity = 0;
            return 0;
        }
        self->capacity = self->size + 1;
    }
    return 1;
}

// (Placed between the camera and balloon code in the original layout; it registers the balloon actor only.)
// Register the balloon actor factory, task class, and inherited runtime type information.
void CBubble_RegisterType() {
    float tmp = kFloat_6_2831855;
    char* p = chatBubbleStrings;
    s_chatBubbleTwoPi = tmp;
    IntPair_Set(&s_bubbleTypeInfo, p + 0x15, &s_actorTypeInfo);
    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), CBubble_Create, &bubbleActorFactoryName);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), &bubbleTaskClassName);
}

// Common actor base (vtable, display priority) shared by the camera and balloon actors; the name is the original's
// address-tagged symbol (NativeBase80201D18), shared with other units, so it is not renamed here.
struct NativeBase80201D18 {
    void* vtable;
    char pad04[4];
    int displayPriority;
    char pad0C[0x0C];
    NativeBase80201D18();
    ~NativeBase80201D18();
};
// Storage for one CCamera actor (the constructor installs __vt__7CCamera; this is NOT a balloon).
struct CameraActorStorage : NativeBase80201D18 {
    char pad[0xE8];
    // Construct the camera actor with its game-state suppression mask.
    CameraActorStorage() {
        vtable = __vt__7CCamera;
        IActor_AddSkipFlags(this, kCameraSkipFlags);
    }
};

// Words at 0x88 and 0xC8..0xD4 of the camera that CCamera.h still leaves as padding; CCamera::OnInit only zeroes them,
// so their meaning is unknown. Supplemental initialization view for camera words not placed in the shared header.
struct CameraInitExtraWords {
    char padding[0x88];
    float unk88;
    char paddingToCallbacks[0x3C];
    int unkC8, unkCC, unkD0, unkD4;
};

// Reset camera input ramps, heading and transition state while leaving position and target intact.
void CCamera::OnInit() {
    CameraInitExtraWords* self = (CameraInitExtraWords*)this;
    float zero = kFloat_0_camera;
    ((CCamera*)self)->useLinearHeightInterpolation = 0;
    ((CCamera*)self)->rotationInputRamp = zero;
    ((CCamera*)self)->heightInputRamp = zero;
    ((CCamera*)self)->movementInputRamp = zero;
    ((CCamera*)self)->heightRatio = zero;
    ((CCamera*)self)->heightOffsetRatio = zero;
    ((CCamera*)self)->inputDelay = zero;
    ((CCamera*)self)->radialMenuEnabled = 0;
    ((CCamera*)self)->headingUpdateTimer = zero;
    ((CCamera*)self)->headingChanged = 0;
    ((CCamera*)self)->processingEnabled = 0;
    ((CCamera*)self)->remainingRotationAngle = zero;
    ((CCamera*)self)->totalRotationAngle = zero;
    ((CCamera*)self)->transitionTime = zero;
    ((CCamera*)self)->transitionActive = 0;
    self->unk88 = zero;
    ((CCamera*)self)->unk_8C = zero;
    ((CCamera*)self)->currentInput = 0;
    ((CCamera*)self)->previousInput = 0;
    ((CCamera*)self)->modeChanged = 0;
    ((CCamera*)self)->mode = 0;
    self->unkC8 = 0;
    self->unkD0 = 0;
    self->unkCC = 0;
    self->unkD4 = 0;
    ((CCamera*)self)->zoomInputBlocked = 0;
    ((CCamera*)self)->timedControlMode = 0;
    ((CCamera*)self)->timedControlDuration = zero;
    ((CCamera*)self)->directionalInputEnabled = 1;
}

typedef BubbleEntry RenderList;
struct RenderOwner {
    char pad[0x18];
    RenderList* list;
};
// Destroy both balloon renderers, then release the array allocation including its runtime cookie.
void CBubble::OnDestroy() {
    RenderOwner* self = (RenderOwner*)this;
    if (self->list != 0) {
        int zero;
        int offset;
        int i;
        i = 0;
        offset = 0;
        zero = 0;
        do {
            void* item = *(void**)((char*)self->list + offset);
            if (item != 0) {
                CaddieMessageRenderer_Destroy(item, 1);
                *(void**)((char*)self->list + offset) = (void*)zero;
            }
            ++i;
            offset += (int)sizeof(BubbleEntry);
        } while (i < 2);
        if (self->list != 0) {
            if (self->list != 0) {
                __dla__FPv((char*)self->list - 0x10);
            }
            self->list = 0;
        }
    }
}

// Allocate a camera actor; ownership of the returned object passes to the caller.
void* CCamera_Create() {
    return new CameraActorStorage;
}

// Storage for one CBubble actor: base plus the pointer to its two-entry BubbleEntry array.
struct BubbleActorStorage : NativeBase80201D18 {
    BubbleEntry* entries;
    // Construct an empty balloon actor with display priority 5 and its skip mask.
    BubbleActorStorage() {
        vtable = __vt__7CBubble;
        entries = 0;
        displayPriority = 5;
        IActor_AddSkipFlags(this, kBubbleSkipFlags);
    }
};

// Allocate a balloon actor; ownership of the returned object passes to the caller.
void* CBubble_Create() {
    return new BubbleActorStorage;
}

extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void* Camera_GetRenderState(void*);
extern void SetTransform(int, void*);
extern void Engine_SetType7Resource(int);
extern void Engine_SetTextureStageParameter(int, int, int);
extern void Engine_SetCachedRenderState(int, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);
extern void Resource_GetOffsetPtr(void*, int, int, void**, int);
extern void Vector_CopyComponents(void*, void*);
extern void GraphicsBuffer_Flush(void*);
extern void Engine_SetVertexStream(int, void*, int, int);
extern void DrawPrimitiveFromBoundBuffers(int, int, int);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern void* CQuadTree_GetInstance();
extern void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);

typedef Vec3f Matrix3;
typedef Vec3f StripPoint;
struct Table1 {
    unsigned int w[30];
};
struct Table2 {
    unsigned int w[12];
};
struct Table3 {
    unsigned int w[10];
};
extern const unsigned int beachRenderStateTables[]; // beachRenderStateTables.
extern char s_sceneLighting[];
extern const float kFloat_0_8_TokenRendering, kFloat_0_VectorMath, kFloat_0_2_CourseHelpers,
    kFloat_1_5_TokenSideYOffset, kFloat_1_TokenDefaultYOffset;
struct BeachRenderManager {
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void* texture(unsigned int);
    virtual void* textureState(unsigned int);
};
struct CameraRenderMatrices {
    char padding[0x20];
    float view[16];
    float projection[16];
};
struct BeachStripParameters {
    float offsetScale;
    char padding[4];
    float alpha;
    float sortDepth;
    char paddingToState[4];
    int state;
};
struct BubbleOpacity {
    char padding[0x0C];
    float opacity;
};
struct BeachRenderState {
    int value;
    virtual void unused();
    virtual void apply();
};

struct BeachMaterialView {
    char padding[0x44];
    BeachRenderState material;
};
// Conform shoreline strips to terrain, tint/fade their vertices, then draw them.
// Mesh contract: source row 0x24 bytes = {side0 XYZ, side1 XYZ, offset XYZ};
// output row = two 0x18-byte vertices {XYZ, RGBA bytes, two remaining words}.
// RGBA occupies +0c..+0f; remaining vertex words are preserved. Terrain queries
// receive world-space coordinates with Y as height.
void CBeach::Draw() {
    // Phase 1: bind shared shoreline render state and derive the lighting tint.
    unsigned int scanOffset = 0;
    Table3 t3;
    const char* tables = (const char*)beachRenderStateTables;
    Table1 t1 = *(Table1*)((0 + tables));
    Table2 t2 = *(Table2*)(tables + 0x78);
    t3 = *(Table3*)(tables + 0xA8);
    Engine_SetRenderStateList(&t1, 15);
    Engine_SetTextureStageStateList(0, &t2, 6);
    Engine_SetTextureStageParameterList(0, &t3, 5);
    void* state = Camera_GetRenderState(g_view);
    SetTransform(0, ((CameraRenderMatrices*)state)->view);
    SetTransform(1, ((CameraRenderMatrices*)state)->projection);
    Engine_SetType7Resource(0);
    ((BeachMaterialView*)this)->material.apply();
    Engine_SetTextureStageParameter(0, 0, 1);
    Engine_SetTextureStageParameter(0, 1, 1);
    Engine_SetCachedRenderState(13, 4);
    Engine_SetCachedRenderState(14, 5);
    Engine_SetCachedRenderState(5, 0);
    Engine_SetCachedRenderState(3, 3);
    Engine_SetFlexibleVertexFormat(0x11F001);
    BeachRenderManager* mgr = (*(BeachRenderManager**)&AsWView(g_view)->renderTarget);
    void* texture = mgr->texture(bitmap);
    Engine_SetTextureStageResource(0, texture);
    mgr = (*(BeachRenderManager**)&AsWView(g_view)->renderTarget);
    void* textureState = mgr->textureState(bitmap);
    Engine_SetTexturePalette(0, textureState);
    unsigned int max = 0;
    unsigned int color = *(unsigned int*)(s_sceneLighting + 0x18);

    unsigned char green = (unsigned char)(255 - (color >> 8));

    unsigned char red2 =
        (unsigned char)(255 - (int)(kFloat_0_8_TokenRendering * ((unsigned char)(255 - (color >> 16)))));
    unsigned char green2 = (unsigned char)(255 - (int)(kFloat_0_8_TokenRendering * green));
    unsigned char blue2 = (unsigned char)(255 - (int)(kFloat_0_8_TokenRendering * ((unsigned char)(255 - color))));
    int count = stripCount;
    float largest = kFloat_0_VectorMath;

    // Phase 2: find the largest sortDepth, then visit strips cyclically from it.
    // This rotates the existing order; it does not sort the parameter array.
    for (unsigned int i = 0; count > i; ++i, scanOffset += 0x18) {
        float z = ((BeachStripParameters*)((char*)stripParameters + scanOffset))->sortDepth;
        if (z > largest) {
            largest = z;
            max = i;
        }
    }
    Matrix3 result;
    for (unsigned int n = 0; n < stripCount; ++n) {
        unsigned int idx = (n + max) % stripCount;
        if (((BeachStripParameters*)((char*)stripParameters + idx * (int)sizeof(BeachStripParameters)))->state == 2)
            continue;
        void* mesh;
        Resource_GetOffsetPtr(*(void**)((char*)vertexBuffers + 4 * idx), 0, 0, &mesh, 0);
        unsigned int half = stripRowCount >> 1;

        // Phase 3: walk from the middle toward the last row, then back toward row 0.
        // out[side] carries the previous terrain-conformed height on that edge;
        // the reverse walk continues from those values, as the original does.
        StripPoint out[2];
        unsigned int r = half;
        unsigned int vertex = (2 * half);
        int rowBytes = r * 0x24;
        unsigned int vertexBytes = vertex * 0x18;
        while (r < stripRowCount) {
            int last = (r == stripRowCount - 1);
            int sideBytes = 0;
            for (int side = 0; side < 2; ++side, sideBytes += 0x18) {
                float amount =
                    ((BeachStripParameters*)((char*)stripParameters + idx * (int)sizeof(BeachStripParameters)))
                        ->offsetScale;
                if (side)
                    amount *= kFloat_0_2_CourseHelpers;
                else if (last)
                    amount *= kFloat_0_8_TokenRendering;
                char* elem = (char*)stripRows + rowBytes;
                Matrix3 offset;
                Matrix3 base;
                __ml__FfRC7WVector(&base, elem + 0x18, amount);
                __mi__FRC7WVectorRC7WVector(&offset, elem + side * 0x0C, &base);
                Matrix3 res;
                *(WVector*)&result = *(WVector*)&offset;
                Matrix3 pos = result;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&res, CQuadTree_GetInstance(), &pos, 1, 0, 1);
                *(WVector*)&result = *(WVector*)&res;
                result.y += side ? kFloat_1_5_TokenSideYOffset : kFloat_1_TokenDefaultYOffset;
                if (r != (stripRowCount >> 1)) {
                    float width;
                    float delta;
                    float bound = out[side].y;
                    delta = bound - result.y;
                    width = maxRowHeightDelta;
                    if (delta > width)
                        result.y = bound - width;
                    else if (delta < -width)
                        result.y = width + bound;
                }
                *(WVector*)(&out[side]) = *(WVector*)&result;
                Vector_CopyComponents((char*)mesh + ((side + vertex)) * 0x18, &result);
                if (last)
                    ((unsigned char*)mesh)[sideBytes + vertexBytes + 0xF] = 0;
                else
                    ((unsigned char*)mesh)[vertexBytes + sideBytes + 0xF] =
                        (int)((BeachStripParameters*)((char*)stripParameters + idx * (int)sizeof(BeachStripParameters)))
                            ->alpha;
                ((unsigned char*)mesh)[vertexBytes + sideBytes + 0xC] = red2;
                ((unsigned char*)mesh)[sideBytes + vertexBytes + 0xD] = green2;
                ((unsigned char*)mesh)[vertexBytes + sideBytes + 0xE] = blue2;
            }
            vertexBytes += 0x30;
            vertex += 2;
            r++;
            rowBytes += 0x24;
        }
        int r2 = (int)(stripRowCount >> 1) - 1;
        vertex = r2 * 2;
        rowBytes = r2 * 0x24;
        vertexBytes = vertex * 0x18;
        for (; 0 <= r2; vertex -= 2, vertexBytes -= 0x30, --r2, rowBytes -= 0x24) {
            int last = (r2 == 0);
            unsigned int sideBytes = 0;
            for (int side = 0; side < 2; ++side, sideBytes += 0x18) {
                float amount =
                    ((BeachStripParameters*)((char*)stripParameters + idx * (int)sizeof(BeachStripParameters)))
                        ->offsetScale;
                if (side)
                    amount *= kFloat_0_2_CourseHelpers;
                else if (last)
                    amount *= kFloat_0_8_TokenRendering;
                char* elem = (char*)stripRows + rowBytes;
                Matrix3 offset;
                Matrix3 base;
                __ml__FfRC7WVector(&base, elem + 0x18, amount);
                __mi__FRC7WVectorRC7WVector(&offset, elem + side * 0x0C, &base);
                *(WVector*)&result = *(WVector*)&offset;
                Matrix3 res;
                Matrix3 pos = result;
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&res, CQuadTree_GetInstance(), &pos, 1, 0, 1);
                *(WVector*)&result = *(WVector*)&res;
                result.y += side ? kFloat_1_5_TokenSideYOffset : kFloat_1_TokenDefaultYOffset;
                if ((unsigned int)r2 != (stripRowCount >> 1)) {
                    float width;
                    float delta;
                    float bound = out[side].y;
                    delta = bound - result.y;
                    width = maxRowHeightDelta;
                    if (delta > width)
                        result.y = bound - width;
                    else if (delta < -width)
                        result.y = width + bound;
                }
                *(WVector*)(&out[side]) = *(WVector*)&result;
                Vector_CopyComponents((char*)mesh + (side + vertex) * 0x18, &result);
                if (last)
                    ((unsigned char*)mesh)[sideBytes + vertexBytes + 0xF] = 0;
                else
                    ((unsigned char*)mesh)[sideBytes + vertexBytes + 0xF] =
                        (int)((BeachStripParameters*)((char*)stripParameters + idx * (int)sizeof(BeachStripParameters)))
                            ->alpha;
                ((unsigned char*)mesh)[vertexBytes + sideBytes + 0xC] = red2;
                ((unsigned char*)mesh)[(vertexBytes + sideBytes) + 0xD] = green2;
                ((unsigned char*)mesh)[sideBytes + vertexBytes + 0xE] = blue2;
            }
        }
        // Phase 4: flush modified positions/colors before binding the vertex stream.
        GraphicsBuffer_Flush(*(void**)((char*)vertexBuffers + 4 * idx));
        Engine_SetVertexStream(0, *(void**)((char*)vertexBuffers + (4 * idx)), 0, 0x18);
        DrawPrimitiveFromBoundBuffers(0x98, 0, stripRowCount * 2 - 2);
    }
}

// Default primitive callback; this base implementation does no work.
void CGenericPrimitive::vfunc_1() {}
CBeach::~CBeach() {
    __dt__8CWaveletFv(this, 0);
}
// Initialize this module's mutable two-pi constant during startup.
void TokenBase_InitModuleConstants() {
    s_twoPi_800D1108 = kFloat_6_2831855;
}
// Borrow count input elements and reset the cursor; return 0 without writes for null/empty input, else 1.
int cTokenV_SetInputBuffer(void* self, void* value, int count) {
    if (value == 0 || count < 1)
        return 0;
    AsTokenInputState(self)->inputBuffer = value; // original +0x14
    AsTokenInputState(self)->inputCount = count;  // original +0x08
    AsTokenInputState(self)->cursor = 0;          // original +0x04
    return 1;
}
// Initialize the token module's mutable two-pi constant during startup.
void TokenVector_InitModuleConstants() {
    s_tokenVectorTwoPi = kFloat_6_2831855;
}
// Default construction of a balloon entry intentionally leaves its storage unchanged.
void BubbleEntry_DefaultConstruct(void*) {}

// Fade active balloons by camera distance and report completion of the first requested balloon.
void CBubble::OnProcess(float deltaTime) {
    BubbleOwner* self = (BubbleOwner*)this;
    for (int i = 0; i < kBubbleCount; ++i) {
        if (((CBubble*)self->entries[i].renderer)->IsActiveForCurrentPlayer()) {
            char displacement[0x18];
            __mi__FRC7WVectorRC7WVector(displacement, &self->entries[i].position, s_cameraTransform + 0x24);
            float amount = WVectorLen(*(WVector*)displacement);
            amount -= kBubbleFadeStartDistance;
            amount *= kBubbleFadeInverseDistanceSpan;
            if (amount < kFloat_0_Bubble)
                amount = kFloat_0_Bubble;
            else if (amount > kFloat_1_BubbleOpacity)
                amount = kFloat_1_BubbleOpacity;
            ((BubbleOpacity*)self->entries[i].renderer)->opacity = kFloat_1_BubbleOpacity - amount;
            ((CBubble*)self->entries[i].renderer)->UpdateTextFadeOut(deltaTime);
        }
    }
    if (!((CBubble*)self->entries[0].renderer)->IsActiveForCurrentPlayer() && self->entries[0].notifyWhenFinished) {
        self->entries[0].notifyWhenFinished = 0;
        IActor_DispatchMessage(this, this, chatBubbleStrings + 0x0E, 0x0C, 0, 0, 0, 0);
    }
}

struct RenderScreen {
    BubbleEntry first;
    BubbleEntry second;
};
struct ScreenOwner {
    char pad[0x18];
    RenderScreen* screen;
};
// Project the two world anchors and draw active balloons with the ball-ground-specific offset.
// inputTransform maps world anchors to view space; ProjectViewPoint writes screen
// X/Y at result+0/+4 for DrawAlignedBalloon. Other result words are not consumed.
// The ground-type-2 offset (-20,-10) applies only to the first balloon, in screen units.
void CBubble::OnDisplay() {
    ScreenOwner* self = (ScreenOwner*)this;
    struct Locals {
        float x;
        float y;
        char m1[0x0C];
        char m0[0x0C];
        char result[0x40];
    };
    Locals L;
    void* state = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (!((*(unsigned int*)&AsCGolfDoc(state)->gameStateFlags) & kBubbleHiddenDuringShot)) {
        void* second = self->screen->second.renderer;
        if (((CBubble*)second)->IsActiveForCurrentPlayer()) {
            Matrix_TransformPoint(L.m0, &self->screen->second.position, (char*)AsWView(g_view)->inputTransform);
            WView_ProjectViewPoint(g_view, L.result, L.m0, 0);
            ((CBubble*)(self->screen->second.renderer))
                ->DrawAlignedBalloon(g_view, *(float*)(L.result + 0x00), *(float*)(L.result + 0x04), 1);
        }
        void* first = self->screen->first.renderer;
        if (((CBubble*)first)->IsActiveForCurrentPlayer()) {
            Matrix_TransformPoint(L.m1, &self->screen->first.position, (char*)AsWView(g_view)->inputTransform);
            WView_ProjectViewPoint(g_view, L.result, L.m1, 0);
            unsigned char mode = ((CGolfBall*)GolfBall__Fv())->m_groundType; // original ball +0x59
            int type;
            if (mode == kBubbleGroundType) {
                L.x = kBubbleGroundType2ScreenOffsetX;
                L.y = kBubbleGroundType2ScreenOffsetY;
                type = 2;
            } else {
                L.x = kFloat_0_Bubble;
                L.y = kFloat_0_Bubble;
                type = 1;
            }
            ((CBubble*)(self->screen->first.renderer))
                ->DrawAlignedBalloon(g_view, *(float*)(L.result + 0x00) + L.x, *(float*)(L.result + 0x04) + L.y, type);
        }
    }
}

struct InitObject {
    void* vtable;
    unsigned int cursor;
    unsigned int inputCount;
    unsigned int bufferCapacity;
    unsigned int ownedBuffer;
    unsigned int inputBuffer;
};

// Initialize an empty token view without allocating input storage.
void cTokenV_Init(InitObject* self) {
    void* vtable = &__vt__7cTokenV;
    self->bufferCapacity = 0;
    self->cursor = 0;
    self->inputCount = 0;
    self->ownedBuffer = 0;
    self->vtable = vtable;
    self->inputBuffer = 0;
}

// Initialize an empty owning string-token buffer.
void cTokenS_Init(InitObject* self) {
    void* vtable = &__vt__7cTokenS;
    self->bufferCapacity = 0;
    self->cursor = 0;
    self->inputCount = 0;
    self->ownedBuffer = 0;
    self->vtable = vtable;
    self->inputBuffer = 0;
}

// Return the current string-token input buffer without transferring ownership.
void* cTokenS_GetInputBuffer(void* self) {
    return AsTokenInputState(self)->inputBuffer;
}
// Return the current vector-token input buffer without transferring ownership.
void* cTokenV_GetInputBuffer(void* self) {
    return AsTokenInputState(self)->inputBuffer;
}
// Return the static balloon type descriptor.
void* CBubble::GetRTTI() {
    return &s_bubbleTypeInfo;
}
// Destroy the balloon's actor base and optionally free self; return the original pointer.
CBubble::~CBubble() {
    __dt__18NativeBase80201D18Fv(this, 0);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620D8", _ctors_u800D1108_0,
                  TokenBase_InitModuleConstants); // TokenBase_InitModuleConstants.
STATIC_INIT_ENTRY(".CRT$XCU804620D8", _ctors_u800D1108_1,
                  TokenVector_InitModuleConstants); // TokenVector_InitModuleConstants.
STATIC_INIT_ENTRY(".CRT$XCU804620D8", _ctors_u800D1108_2, CBubble_RegisterType);
#pragma pop
