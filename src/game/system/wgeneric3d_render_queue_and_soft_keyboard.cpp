#include <decomp/static_init.h>
#include <decomp/offset_of.h>
// Soft keyboard input and generic renderer command staging (merged ABI unit).
// Navigation / mod questions:
// - Keyboard text, kana and hit rectangles: SoftKeyboardData/KeyboardPage below,
//   CSoftKeyboard_UpdateInput (input) and SoftKeyboard_HitTestRectangle. Coordinates
//   are UI pixels; page/sound strings live in layoutControlAndExceptionStrings.
// - Model vertex packing / color / fog: GenericVertexSource, VertexPacking,
//   WGeneric3D_PackVertexPointerArray and WGeneric3D_EnqueueRenderEntry. Match the
//   packing selector to the output stride before changing a vertex producer.
// - Draw order / batching: RenderFlags, PrimitiveTag, WGeneric3D_Flush and
//   WGeneric3D_DrawTaggedQuadBatches; RenderEntry.h and WGeneric3D.h are the
//   shared schemas. Queue storage belongs to the renderer; Execute is the backend
//   boundary (WGeneric3DR_DrawImmediatePrimitive in wgeneric3dr_immediate_renderer.cpp).
// - Billboard rotation: TaggedPrimitiveRecord::rotation1024 stores unsigned
//   radians * 1024. Math_SinApprox/Math_CosApprox in gtl_vector_matrix_math.cpp reduce by 2*pi.
//   Keep this quantization and the unsigned conversion together with producers.
// - Texture slots / ownership: RenderSlotArguments, TextureSlotFlags and
//   ReserveOrCreateTextureSlot/UpdateTextureSlot below. Concrete hook bindings
//   are in phase1_class_tables.cpp; loading/cached handles in wgeneric3dr_immediate_renderer.cpp.
// - Defaults: u8008199C_data.cpp and s_generic3DQuadRenderDefaults; these are
//   render state/packing controls, not a viewport or display frame-rate setting.
// Keyboard and renderer operations have separate receivers; their adjacency
// reflects the original unit merge, not a shared lifetime or update dependency.
#define AsWGeneric3D(p) ((WGeneric3D*)(p))
#include "classes/RenderEntry.h"
#include "classes/WGeneric3D.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include <decomp/cw_bitfields.h>

extern "C" {
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern double pow(double, double);
extern const double kDouble_OneOver256_Generic3DRamp;
extern const double kDouble_0_7692307974459868_Generic3DColorRamp;
extern const float kFloat_0_Generic3DDefaults;
extern const float kFloat_1_Generic3DColorRamp;
extern char __vt__10WGeneric3D[];

enum { kFogEnabledOffset = 0x4848, kClearEightSlotBytes = 8 * sizeof(unsigned int) };

enum KeyboardHitState { kKeyboardCharacter, kKeyboardAccept, kKeyboardDelete,
    kKeyboardPageTab, kKeyboardNoHit = 6 };
enum { kKeyboardConfirmAction = 4, kFullWidthSpace = 0x3000,
    kDakuten = 0x309b, kHandakuten = 0x309c,
    kKeyboardPageSoundOffset = 0x4b, kKeyboardAcceptSoundOffset = 0x5a,
    kKeyboardDeleteSoundOffset = 0x6d, kKeyboardErrorSoundOffset = 0x84,
    kKeyboardCharacterSoundOffset = 0x8d };
enum RenderLimits { kRenderEntryCapacity = 0x100, kRenderItemCapacity = 0x800,
    kFirstGeneralSlot = 0x80, kLastReservedSlot = 0x7f, kLastGeneralSlot = 0x7ff,
    kGeneralSlotAttempts = 0x780, kVertexBufferBytes = 0x100000,
    kVertexBufferFlushThreshold = 0xccccc, kSortRangeMask = 0x3ff,
    kColorRampEntryCount = 256, kQuadBatchCapacity = 32, kFogProbeAlphaOffset = 0x13, kFogProbeStride = 0x1c };
enum RenderFlags { kTextureSlotMask = 0x7ff, kSecondaryTextureMask = 0x0003f800,
    kSecondaryPassModeBit = 0x00040000, kRenderPassMask = 0x00300000,
    kQuadBlendModeMask = 0x01800000, kQuadAlphaBlend = 0,
    kQuadAdditiveAlphaBlend = 0x00800000, kQuadBlendMode2 = 0x01000000,
    kQuadBlendMode3 = 0x01800000,
    kEarlyRenderPass = 0x00100000, kLateRenderPass = 0x00300000,
    kQueuePrimitive = 0x00400000, kPrimitiveTopologyMask = 0x06000000,
    kTriangleTopologyBit = 0x02000000,
    kOpenLineStrip = 0x04000000, kClosedLineStrip = 0x06000000,
    kUseFog = 0x08000000, kReplayPassFlag = 0x20000000,
    kOmitVertexColor = 0x40000000 };
enum FlushFlags { kFlushTwoPasses = 1, kFlushReverseOrder = 2, kFlushQuadBatch = 4 };
enum PrimitiveType { kQuadPrimitive = 0x80, kTriangleStripPrimitive = 0x98,
    kTriangleFanPrimitive = 0xa0, kLineStripPrimitive = 0xb0 };
enum PrimitiveTag { kCameraFacingQuad = 0, kUprightQuad = 1, kXYQuad = 2,
    kYZQuad = 3, kXZQuad = 4, kRotatedCameraQuad = 5, kRotatedCameraQuadAlt = 6,
    kUprightQuadAlt = 7, kExplicitQuadCorners = 0xfe, kTransformedQuad = 0xff };
// Flexible vertex format: low bits select position packing (1 or 5), bits
// 12..15 select color bytes, bits 16..23 select texture-coordinate packing,
// high bit requests fog processing. These are engine formats, not GX VAT IDs.
enum VertexPacking {
    kPositionPacking1 = 1, kPositionPacking5 = 5, kPackedColorMask = 0xF000,
    // Indexed single-texture submission uses its own packing selector.
    kIndexedSingleTexturePacking = 0x60000,
    kSingleTexturePacking = 0x110000, kTwoTexturePacking = 0x120000,
    kFogPackingBit = 0x80000000u,
    kQuadVertexPacking1 = 0x11F001, kQuadVertexPacking5 = 0x11F005,
    kPackedQuadVertexBytes = 0x18
};
// Queued secondary-pass recipes, expressed using the RenderFlags fields above.
// Bit 23 is kept as an unresolved pass modifier; these names describe their use.
enum SecondaryPassRecipe {
    kSecondaryPassModifier = 0x00800000,
    kSecondaryColoredPass = kReplayPassFlag | kEarlyRenderPass | kSecondaryPassModifier,
    kSecondaryUncoloredPass = kSecondaryColoredPass | kOmitVertexColor,
    kSecondaryDefaultPass = kReplayPassFlag | kQuadBlendMode2 | kEarlyRenderPass | kOmitVertexColor,
    kSecondaryFogPass = kSecondaryDefaultPass | kUseFog
};


struct Dtor80082914Data {
    void* value00;
    void* value04;
};

extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[0x1000];
extern char s_generic3DInputDtorRegistration[];
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern int InputManager_GetSelectedPlayer(void*);
extern void InputManager_GetPointerPosition(void*, void*, int);
extern int Input_IsActionPressed(void*, int, int);
extern void* g_soundManager;
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);

extern const float kFloat_neg4_SoftKeyboard;

extern const char layoutControlAndExceptionStrings[];
extern char layoutWhitespaceComparisonStrings[4];
struct TempString_u8008199C {
    union {
        struct { CW_BITS2( unsigned char allocated:1, unsigned char size:7) unsigned char pad; unsigned short chars[5]; } small;
        struct { unsigned int bits; int length; unsigned short* ptr; } large;
    };
    // Start an empty inline UTF-16 temporary.
    TempString_u8008199C() {
        large.bits=0; large.length=0; large.ptr=0;
    }
    // Store one UTF-16 code unit plus its terminator in the active representation.
    void assign(unsigned short c) {
        unsigned short* p;
        if (large.bits >> 31) { large.length=1; p=large.ptr; }
        else { small.size=1; p=small.chars; }
        p[0]=c; p[1]=0;
    }
    // Report the high heap-ownership bit.
// An MSL inline string keeps its heap flag in bit 31 of word 0; on the Wii that is the high bit of the first
// UTF-16 unit.
    int isHeap() { return large.bits >> 31; }
    // Report inline character storage.
    int isLocal() { if (isHeap()) return 0; return 1; }
    // Report whether character access follows the stored heap pointer.
    int isIndirect() { if (isLocal()) return 0; return 1; }
    // Borrow the active UTF-16 character buffer.
    unsigned short* data() { return isIndirect() == 0 ? small.chars : large.ptr; }
    // Release only a heap-backed temporary buffer.
    ~TempString_u8008199C() { if(large.bits >> 31) appFreeUni__FPv(large.ptr); }
};
struct KeyboardPage { int x0, y0, x1, y1; unsigned short* text; int columns, rows; };
struct CodePair { unsigned short from, to; };
struct CodeTable { unsigned short code, pad; int reserved04; CodePair* pairs; int reserved0C, reserved10, count; };
struct SoftKeyboardData {
    char reserved00[4]; float pointerX, pointerY;
    union {
        struct { unsigned int textStorageFlags; unsigned int heapTextLength; unsigned short* heapText; };
        struct { unsigned short stringHeader; unsigned short inlineText[5]; };
    };
    struct Position { int x,y; }* keyCoordinates; int* characterPages; unsigned char* characterModifiers;
    unsigned int hoverState, hoverX, hoverY; char reserved30[4];
    int cursorIndex, pageIndex, textLength;
    int gridX0, gridY0, gridX1, gridY1;
    int acceptX0, acceptY0, acceptX1, acceptY1, deleteX0, deleteY0, deleteX1, deleteY1;
    char reserved70[0x10]; KeyboardPage* pages;
    char reserved84[8]; CodeTable* modifierTables; unsigned int modifierTableCount;
    char reserved94[8]; int maxCharacters;
    int pageCount; float offsetX, offsetY;
};
// Editing contract: arrays are indexed by cursorIndex, with maxCharacters > 0.
// Normal insertion advances the cursor until maxCharacters-1, then overwrites
// that last slot while textLength may reach maxCharacters. A cursor equal to
// textLength denotes the append position; full-width spaces represent cleared
// interior slots. UTF-16 storage and metadata arrays must cover all editable slots.
// Kana conversion can inspect the previous slot when the cursor is on blank/NUL;
// the caller must provide a valid predecessor in that case (no bounds guard here).

// Select inline or heap UTF-16 storage from the string ownership bit.
static unsigned short* textData(SoftKeyboardData* d) {
    return !(d->textStorageFlags >> 31) ? d->inlineText : d->heapText;
}
// Test the pointer against a keyboard rectangle after applying its screen offset.
static int hit(SoftKeyboardData* d, const int* r) {
    if ((float)r[0] + d->offsetX < d->pointerX &&
        (float)r[1] + d->offsetY < d->pointerY &&
        (float)r[2] + d->offsetX > d->pointerX &&
        (float)r[3] + d->offsetY > d->pointerY) return 1;
    return 0;
}
extern const float kFloat_0_5_LayoutSound, kFloat_200_LayoutControl, kFloat_1_LayoutControl;
// Play a keyboard sound using an entry in the shared layout string pool.
static void sound(int offset) {
    Sound_PlayByName(g_soundManager, layoutControlAndExceptionStrings + offset, 0, 0, 0, 0,kFloat_0_5_LayoutSound,kFloat_200_LayoutControl,kFloat_1_LayoutControl);
}
// Replace one character and clear its original key position, page and modifier.
static void clearChar(SoftKeyboardData* d, unsigned short c) {
    int index = d->cursorIndex;
    textData(d)[index] = c;
    d->keyCoordinates[d->cursorIndex].x = 0;
    d->keyCoordinates[d->cursorIndex].y = 0;
    d->characterPages[d->cursorIndex] = 0;
    d->characterModifiers[d->cursorIndex] = 0;
}

// Construct the shared game input manager on first use.
static void initInput() {
    if (Sing_805DC5C8() == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, &s_generic3DInputDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        Sing_805DC5C8() = &s_gameInputManager;
    }
}

extern void* __nwa__FUl(unsigned int size);

extern void Engine_SetCachedRenderState(int, int);

extern const float kGeneric3DColorRampByteScale;

extern float s_generic3DRotationPeriod;
extern const float kFloat_6_2831855;

float Generic3D_GetFirstVertexSortDepth(void*, void*, int n);

struct TaggedPrimitiveRecord;
struct GenericVertexSource;
void WGeneric3D_QueueTaggedPrimitive(void*, TaggedPrimitiveRecord**, int, int);
int WGeneric3D_PackVertexPointerArray(void*, int*, int, int, int);
void WGeneric3D_EnqueueRenderEntry(void*, int, int, int, int, int, float);



extern char s_layoutZeroWordPair[8];
extern int s_generic3DSortRangeBuffer[2048];

struct TaggedPrimitiveRecord {
    float u0, v0, u1, v1;
    unsigned int argb; int rotation1024;
    float left, top, right, bottom, centerX, centerY, centerZ;
    unsigned char tag;
};
// Ordinary tagged payload: UV endpoints are texture coordinates; left/right and
// top/bottom are signed corner offsets from center in the selected plane/basis.
// tag changes the payload interpretation: FE supplies four source records, FF
// overlays an extended transform and moves UV endpoints to +0x88. QueueTaggedPrimitive
// copies these payloads into staging storage; rendering borrows that staged copy.

// Copy one tagged primitive record while preserving its field-wise load/store order.
static void copyData38(TaggedPrimitiveRecord* dst, const TaggedPrimitiveRecord* src) {
    dst->u0 = src->u0; dst->v0 = src->v0; dst->u1 = src->u1; dst->v1 = src->v1;
    dst->argb = src->argb; dst->rotation1024 = src->rotation1024;
    dst->left = src->left; dst->top = src->top; dst->right = src->right;
    dst->bottom = src->bottom; dst->centerX = src->centerX; dst->centerY = src->centerY; dst->centerZ = src->centerZ;
    dst->tag = src->tag;
}

struct Range { int low; int high; };

struct VtxColor { unsigned char r, g, b, a; };

// Extract green from the byte-layout color value.
static inline unsigned char vcG(VtxColor c) { return c.g; }
// Extract blue from the byte-layout color value.
static inline unsigned char vcB(VtxColor c) { return c.b; }
// Extract alpha from the byte-layout color value.
static inline unsigned char vcA(VtxColor c) { return c.a; }
// Extract red from the byte-layout color value.
static inline unsigned char vcR(VtxColor c) { return c.r; }

// Source records store ARGB as a word; packed render vertices store R, G, B, A bytes.
struct GenericVertexSource {
    float x, y, z, reserved0C; unsigned int argb; int reserved14;
    float u0, v0, u1, v1; char reserved28[0x10];
};
// Colorless one-texture output has no reserved color word: XYZ immediately followed by UV.
struct PackedTextureVertex { float x, y, z, u, v; };
// Float-slot views of GenericVertexSource and PackedTextureVertex for alias-sensitive copies.
enum SourceVertexFloatSlot { kSourceX = 0, kSourceY = 1, kSourceZ = 2, kSourceU = 6, kSourceV = 7 };
enum PackedTextureFloatSlot { kOutputX = 0, kOutputY = 1, kOutputZ = 2, kOutputU = 3, kOutputV = 4 };
enum PackedVertexStride { kPositionColorBytes = 16, kPositionTextureBytes = 20,
    kPositionColorTextureBytes = 24, kPositionColorTwoTextureBytes = 32 };
struct PackedVertex { float x, y, z; unsigned char red, green, blue, alpha; float u0, v0, u1, v1; };
union VtxWord { unsigned int w; unsigned char b[4]; };

// Bridge the typed input contract to the legacy word-pointer packing ABI.
// Keeping that ABI inside the implementation preserves CodeWarrior alias analysis.
static inline int PackGenericVertexSources(void* renderer, GenericVertexSource* const* sources,
        int count, int packing, int retainBytes) {
    return WGeneric3D_PackVertexPointerArray(renderer, (int*)sources, count, packing, retainBytes);
}

// Copy source xyz into a packed vertex without touching color or texture words.
static inline void copyVertexPosition(void* destination, const void* source) {
    ((PackedVertex*)destination)->x = ((const GenericVertexSource*)source)->x;
    ((PackedVertex*)destination)->y = ((const GenericVertexSource*)source)->y;
    ((PackedVertex*)destination)->z = ((const GenericVertexSource*)source)->z;
}

// ARGB word -> RGBA byte stores, in B/G/R/A store order on big-endian PowerPC.
// Keep four independent word temporaries: a tested inline-function replacement
// changes .text and relocations (stack-slot allocation). This macro is scoped
// per component to retain those lifetimes; each argument must be side-effect free.
#define COPY_ARGB_TO_RGBA(destination, source) \
    { VtxWord t0; t0.w = ((const GenericVertexSource*)(source))->argb; ((PackedVertex*)(destination))->blue = (t0.b[3]); } \
    { VtxWord t1; t1.w = ((const GenericVertexSource*)(source))->argb; ((PackedVertex*)(destination))->green = (t1.b[2]); } \
    { VtxWord t2; t2.w = ((const GenericVertexSource*)(source))->argb; ((PackedVertex*)(destination))->red = (t2.b[1]); } \
    { VtxWord t3; t3.w = ((const GenericVertexSource*)(source))->argb; ((PackedVertex*)(destination))->alpha = (t3.b[0]); }

// Six-word immediate quad vertex: renderer-space XYZ, RGBA bytes, texture UV.
struct QuadBatchVertex { float x, y, z; unsigned char color[4]; float u, v; };
typedef char QuadBatchVertex_size[(sizeof(QuadBatchVertex) == kPackedQuadVertexBytes) ? 1 : -1];
typedef char QuadBatchVertex_color[((int)DECOMP_OFFSETOF(QuadBatchVertex, color) == 12) ? 1 : -1];
typedef char QuadBatchVertex_uv[((int)DECOMP_OFFSETOF(QuadBatchVertex, u) == 16) ? 1 : -1];
typedef Vec3f Vec3_83;
extern float s_cameraTransform[12];
struct TblA2 { int w[10]; };
struct TblB2 { int w[12]; };
struct TblC2 { int w[24]; };
struct TblAll { TblB2 b; TblA2 a; TblC2 c; int flags[4]; };
extern TblAll s_generic3DQuadRenderDefaults;
extern void* g_view;
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void Engine_SetType7Resource(int);
extern void Engine_SetTextureStageResource(int, void*);
extern void Engine_SetTexturePalette(int, void*);
extern int kIdentityMatrix4x4[];
extern void SetTransform(int, void*);
extern void DrawPrimitiveUP(int, int, void*, int);
extern void* Camera_GetRenderState(void*);
extern float Math_CosApprox(float);
extern float Math_SinApprox(float);
extern void Vec3_Scale(Vec3_83*, const float*, float);
extern void Vec3_Add(Vec3_83*, const Vec3_83*, const Vec3_83*);
extern void Mtx44_TransformPointToVec4(Vec3_83*, const Vec3_83*, const float*);
extern void* ISharedResource_GetLocked(void*);
extern void* ISharedResource_GetPaletteObject(void*);

// Extended records used only by transformed quads and camera-facing billboards.
struct TransformedQuadPayload {
    float axisX[3]; float reserved0C;
    unsigned int argb; int rotation1024;
    float left, top, right, bottom, centerX, centerY, centerZ;
    unsigned char tag; char reserved35[3];
    float axisY[3]; char reserved44[0xc];
    float translation[3]; char reserved5C[4];
    float axisZ[3]; char reserved6C[0x1c];
    float u0, v0, u1, v1;
};
typedef char TransformedQuadAxisYOffset[((int)DECOMP_OFFSETOF(TransformedQuadPayload, axisY) == 0x38) ? 1 : -1];
typedef char TransformedQuadUvOffset[((int)DECOMP_OFFSETOF(TransformedQuadPayload, u0) == 0x88) ? 1 : -1];
struct BillboardCameraAxes { char reserved00[0x44]; float right[3]; float up[3]; };
struct CameraRenderMatrices { char reserved00[0x20]; float projection[16]; float view[16]; };
struct RenderSlotArguments { int resourceHandle; int reserved04; void* descriptor; int reserved0C; void* data; };
struct TextureSlotDescriptor { int reserved00; int width; int height; unsigned short reserved0C; unsigned short format; };
enum RenderSlotState { kFreeSlot, kReservedSlot, kInitializedSlot };
// Only the uses below establish these flag meanings; format/type selector
// values are passed through to the backend and are not identified as GX enums.
enum TextureSlotFlags {
    kTextureModeBit11 = 0x800, kReserveTextureSlotOnly = 0x1000,
    kTextureFilterBit13 = 0x2000, kUseReservedTextureSlots = 0x20000000,
    kTextureType3 = 0x40000000, kTextureCreationArg7Is24 = 0x80000000u
};

// Bind the borrowed shared-resource word to an already reserved renderer slot.
static inline void BindTextureSlotResource(void* renderer, int resource, int slot) {
    ((WGeneric3D*)renderer)->slot40(resource, slot);
}

// Allocate backend storage for a slot. Width/height are pixels; the remaining
// selectors and final argument (16 or 24) have unresolved backend meanings.
static inline void CreateTextureSlotStorage(void* renderer, int width, int height,
        int typeSelector, int filterSelector, int slot, int modeSelector, int creationArg7) {
    ((WGeneric3D*)renderer)->slot39(width, height, typeSelector, filterSelector,
                                  slot, modeSelector, creationArg7);
}

// Borrow descriptor/data for virtual slot 41. Both recovered vtables (WGeneric3D
// and WGeneric3DR in phase1_class_tables.cpp) bind WGeneric3D_NoOpTextureDataUpdate, the empty
// hook in wgeneric3dr_renderer_core.cpp: no copy, retain, upload, callback or deferred use occurs.
// Thus caller storage may expire on return for these backends. An added backend
// must define its own upload lifetime here; do not infer transfer from void*.
static inline void UpdateTextureSlotData(void* renderer, int slot, void* descriptor,
                                        void* data, int flags) {
    ((WGeneric3D*)renderer)->slot41(slot, descriptor, data, flags);
}

struct TaggedEntryFlags { char reserved00[0xa8]; int flags; };




// Append a render entry to the current sort queue.
static inline void enqueue(WGeneric3D* self, void* entry) {
    self->queue[self->queuedEntryCount++] = entry;
}

// Cursor ABI: slot is renderer base + index*4. Only the state-table
// displacement is valid relative to it; it is not a WGeneric3D object.
// Keep this narrow view separate so no backend/vtable access can use it.
struct ShiftedRenderSlotView {
    char beforeState[(int)DECOMP_OFFSETOF(WGeneric3D, items[0])];
    int state;
};

// Read the selected slot cursor and its allocation state using volatile loads.
static inline int occupiedSlot(void* self, int offset, int& index, int*& slot) {
    index = *(volatile int*)((char*)self + offset);
    slot = (int*)((char*)self + index * 4);
    return *(volatile int*)&((ShiftedRenderSlotView*)slot)->state;
}

// Return the logical number of characters in the keyboard text.
unsigned int CSoftKeyboard_GetTextLength(SoftKeyboardData* data) {
    return data->textLength;
}

// Test the pointer against an explicitly supplied keyboard rectangle.
int SoftKeyboard_HitTestRectangle(SoftKeyboardData* data, int x0, int y0, int x1, int y1) {
    if ((float)x0 + data->offsetX < data->pointerX &&
        (float)y0 + data->offsetY < data->pointerY &&
        (float)x1 + data->offsetX > data->pointerX &&
        (float)y1 + data->offsetY > data->pointerY)
        return 1;
    return 0;
}

// Reset keyboard hover coordinates and screen offsets.
int CSoftKeyboard_InitHoverOffsets(SoftKeyboardData* data) {
    float value = kFloat_neg4_SoftKeyboard;
    data->hoverX = 0;
    data->hoverY = 0;
    data->offsetX = value;
    data->offsetY = value;
    return 1;
}

// Handle keyboard tabs, accept/delete buttons, character insertion and kana modifiers.
int CSoftKeyboard_UpdateInput(SoftKeyboardData* keyboard) {
    unsigned int row;
    // Return 0 only when Accept is activated; all editing/hover paths return 1.
    // Phase 1: sample the selected player's screen pointer and handle page tabs.
    int index;
    unsigned int column;
    initInput();
    void* manager = Sing_805DC5C8();
    initInput();
    InputManager_GetPointerPosition(manager, &keyboard->pointerX, InputManager_GetSelectedPlayer(Sing_805DC5C8()));
    keyboard->hoverState = kKeyboardNoHit;
    for (index=0, row=0; index < (keyboard->pageCount); ++index, row+=0x1c) {
        int i=index;
        int itemOffset=row;
        KeyboardPage* it = (KeyboardPage*)((char*)keyboard->pages + itemOffset);
        if (hit(keyboard, &it->x0) == 1) {
            initInput();
            if (Input_IsActionPressed(Sing_805DC5C8(), kKeyboardConfirmAction, -1)) {
                switch (i) {
                    case 0:
                    keyboard->pageIndex = i;
                    break;
                    case 1:
                    keyboard->pageIndex = i;
                    break;
                    case 2:
                    keyboard->pageIndex = i;
                    break;
                    case 3:
                    keyboard->pageIndex = i;
                    break;
                }
                sound(kKeyboardPageSoundOffset);
                keyboard->pageIndex = i;
            }
            keyboard->hoverX = ((KeyboardPage*)((char*)keyboard->pages+itemOffset))->x0;
            keyboard->hoverY = ((KeyboardPage*)((char*)keyboard->pages+itemOffset))->y0;
            keyboard->hoverState = kKeyboardPageTab;
            break;
        }
    }
    // Phase 2: accept or delete. Deletion distinguishes a full final slot,
    // append position and interior blanks; keep cursor/textLength changes paired.
    if (hit(keyboard, &keyboard->acceptX0) == 1) {
        initInput();
        if (Input_IsActionPressed(Sing_805DC5C8(), kKeyboardConfirmAction, -1)) {
            sound(kKeyboardAcceptSoundOffset);
            return 0;
        }
        keyboard->hoverX = keyboard->acceptX0;
        keyboard->hoverY = keyboard->acceptY0;
        keyboard->hoverState = kKeyboardAccept;
    }
    else if (hit(keyboard, &keyboard->deleteX0) == 1) {
        initInput();
        if (Input_IsActionPressed(Sing_805DC5C8(), kKeyboardConfirmAction, -1)) {
            if (keyboard->cursorIndex > 0) {
                if (keyboard->textLength == keyboard->maxCharacters && textData(keyboard)[keyboard->cursorIndex] != 0 && 1 + keyboard->cursorIndex == keyboard->textLength) {
                    sound(kKeyboardDeleteSoundOffset);
                    keyboard->textLength--;
                    clearChar(keyboard, 0);
                }
                else {
                    sound(kKeyboardDeleteSoundOffset);
                    if (keyboard->cursorIndex == keyboard->textLength) {
                        keyboard->cursorIndex = keyboard->cursorIndex - 1;
                        keyboard->textLength -= 1;
                        clearChar(keyboard, 0);
                    }
                    else if (textData(keyboard)[keyboard->cursorIndex] == 0 || textData(keyboard)[keyboard->cursorIndex] == kFullWidthSpace) {
                        --keyboard->cursorIndex;
                        clearChar(keyboard, kFullWidthSpace);
                    }
                    else {
                        clearChar(keyboard, kFullWidthSpace);
                    }
                }
            }
            else if (keyboard->cursorIndex == 0) {
                clearChar(keyboard, kFullWidthSpace);
            }
            else {
                sound(kKeyboardErrorSoundOffset);
            }
        }
        keyboard->hoverX = keyboard->deleteX0;
        keyboard->hoverY = keyboard->deleteY0;
        keyboard->hoverState = kKeyboardDelete;
    }
    // Phase 3: find a character cell in keyboard-local pixels plus screen offset.
    if ((float)keyboard->gridX0 + keyboard->offsetX < keyboard->pointerX) {
        if ((float)keyboard->gridY0 + keyboard->offsetY < keyboard->pointerY) {
            KeyboardPage* selected;
            for (row = 0; (int)row < (selected=&keyboard->pages[keyboard->pageIndex])->rows; ++row) {
                for (column = 0; (int)column < *(volatile int*)&selected->columns; ++column) {
                    __typeof__(keyboard->gridX0) gridLeftSnapshot = keyboard->gridX0;
                    __typeof__(keyboard->gridX0) gridLeftTypeSource = keyboard->gridX0;
                    int rect[4];
                    __typeof__(gridLeftTypeSource) cellGridLeft = gridLeftSnapshot;
                    rect[0] = cellGridLeft + (keyboard->gridX1 - keyboard->gridX0) * column;
                    rect[2] = keyboard->gridX1 + (keyboard->gridX1 - keyboard->gridX0) * column;
                    rect[3] = keyboard->gridY1 + (keyboard->gridY1 - keyboard->gridY0) * row;
                    rect[1] = keyboard->gridY0 + (keyboard->gridY1 - keyboard->gridY0) * row;
                    if (SoftKeyboard_HitTestRectangle((SoftKeyboardData*)keyboard, rect[0], rect[1], rect[2], rect[3]) != 1) continue;
                    initInput();
                    if (Input_IsActionPressed(Sing_805DC5C8(), kKeyboardConfirmAction, -1)) {
                        sound(kKeyboardCharacterSoundOffset);
                        unsigned short* chars = keyboard->pages[keyboard->pageIndex].text;
                        TempString_u8008199C c;
                        c.assign(chars[column + row*keyboard->pages[keyboard->pageIndex].columns]);
                        if (c.data()[0] == kDakuten || c.data()[0] == kHandakuten) {
                            // Modifier keys replace a matching code unit in place; they do
                            // not advance cursor/length or use the normal insertion metadata.
                            int codeOffset=0;
                            for (unsigned int i=0; i<(keyboard->modifierTableCount); ++i, codeOffset+=0x18) {
                                CodeTable* entry=(CodeTable*)((char*)keyboard->modifierTables+codeOffset);
                                if (entry->code == c.data()[0]) {
                                    unsigned int local = !(keyboard->textStorageFlags >>31);
                                    CodeTable table = *entry;
                                    index = keyboard->cursorIndex;
                                    if (!wcscmp((const wchar_t*)((local ? keyboard->inlineText : keyboard->heapText)+index), (const wchar_t*)layoutWhitespaceComparisonStrings) || !wcscmp((const wchar_t*)((local ? keyboard->inlineText : keyboard->heapText)+index), (const wchar_t*)(2+layoutWhitespaceComparisonStrings))) --index;
                                    for (int j=0; j<(table.count); ++j) {
                                        if (table.pairs[j].from == (local ? keyboard->inlineText : keyboard->heapText)[index]) {
                                            (local ? keyboard->inlineText : keyboard->heapText)[index] = table.pairs[j].to;
                                            keyboard->keyCoordinates[index].x=0;
                                            keyboard->keyCoordinates[index].y=0;
                                            keyboard->characterModifiers[index]=1;
                                            break;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        else {
                            // Normal key: write the code unit and parallel source-key metadata,
                            // then advance within the fixed editable range described above.
                            unsigned int index = keyboard->cursorIndex;
                            textData(keyboard)[index] = chars[column+row*keyboard->pages[keyboard->pageIndex].columns];
                            keyboard->characterModifiers[keyboard->cursorIndex]=0;
                            keyboard->keyCoordinates[keyboard->cursorIndex].x=(1+column);
                            keyboard->keyCoordinates[keyboard->cursorIndex].y=(row+1);
                            keyboard->characterPages[keyboard->cursorIndex]=keyboard->pageIndex;
                            if (keyboard->cursorIndex < keyboard->maxCharacters-1) ++keyboard->cursorIndex;
                            else if (keyboard->cursorIndex == keyboard->maxCharacters-1 && keyboard->textLength == keyboard->maxCharacters-1) ++keyboard->textLength;
                            if (keyboard->textLength < keyboard->maxCharacters && keyboard->cursorIndex-1 == keyboard->textLength) ++keyboard->textLength;
                        }
                    }
                    int gridX0Value = keyboard->gridX0;
                    keyboard->hoverX=gridX0Value+(keyboard->gridX1-keyboard->gridX0)*column;
                    keyboard->hoverState = kKeyboardCharacter;
                    keyboard->hoverY=keyboard->gridY0+(keyboard->gridY1-keyboard->gridY0)*row;
                    break;
                }
                if (kKeyboardCharacter == keyboard->hoverState) break;
            }
        }
    }
    return 1;
}

// Report success for the keyboard virtual hook.
int CSoftKeyboard_vfunc_4() {
    return 1;
}

// Report success for the keyboard virtual hook.
int CSoftKeyboard_vfunc_3() {
    return 1;
}

// Report success for the keyboard virtual hook.
int CSoftKeyboard_vfunc_5() {
    return 1;
}

// Release the owned Unicode buffer and optionally delete its two-word owner.
void* dtor_80082914(Dtor80082914Data* data, int deleting) {
    if (data != 0) {
        data->value04 = 0;
        if (data != 0 && data->value00 != 0) {
            data->value04 = 0;
            appFreeUni__FPv(data->value00);
        }
        if (deleting > 0)
            __dl__FPv(data);
    }
    return data;
}

// Initialize the generic-renderer rotation-period value.
void Generic3D_InitRotationPeriod() {
    s_generic3DRotationPeriod = kFloat_6_2831855;
}

// Initialize render queues, slot allocation, color ramps and fog state.
void* WGeneric3D_ctor(WGeneric3D* data) {
    *(void**)data = (void*)__vt__10WGeneric3D;
    data->vertexBuffer = 0;
    data->vertexBufferBytesUsed = 0;
    data->queuedEntryCount = 0;
    data->renderEntryCount = 0;
    data->nextRenderSlot = kFirstGeneralSlot;
    data->nextReservedRenderSlot = 1;
    int count = kRenderItemCapacity / 8;
    WGeneric3D* p = data;
    do {
        p->items[0] = 0;
        p->items[1] = 0;
        p->items[2] = 0;
        p->items[3] = 0;
        p->items[4] = 0;
        p->items[5] = 0;
        p->items[6] = 0;
        p->items[7] = 0;
        p = (WGeneric3D*)((char*)p + kClearEightSlotBytes);
    } while (--count);
    for (unsigned int n = 0; n < kColorRampEntryCount; n += 8) {
        data->colors[n + 0] = (unsigned char)(0xFF - n);
        data->colors[n + 1] = (unsigned char)(0xFF - (n + 1));
        data->colors[n + 2] = (unsigned char)(0xFF - (n + 2));
        data->colors[n + 3] = (unsigned char)(0xFF - (n + 3));
        data->colors[n + 4] = (unsigned char)(0xFF - (n + 4));
        data->colors[n + 5] = (unsigned char)(0xFF - (n + 5));
        data->colors[n + 6] = (unsigned char)(0xFF - (n + 6));
        data->colors[n + 7] = (unsigned char)(0xFF - (n + 7));
    }
    float f0;
    float f1;
    f1 = kFloat_0_Generic3DDefaults;
    data->separateTexturePassesEnabled = 0;
    f0 = kFloat_1_Generic3DColorRamp;
    data->colorRampStart = f1;
    data->colorRampEnd = f0;
    data->colorRampEndTransparency = 0xFF;
    for (int i = 0; i < kColorRampEntryCount; ++i) {
        pow((double)i * kDouble_OneOver256_Generic3DRamp, kDouble_0_7692307974459868_Generic3DColorRamp);
    }
    data->fogEnabled = 0;
    data->value4850 = 0;
    data->usePositionMode1 = 0;
    data->resourceOverride = 0;
    return data;
}

#pragma push
#pragma use_lmw_stmw on
// Dispose occupied render slots, release the vertex buffer and optionally delete the renderer.
void* WGeneric3D_dtor(WGeneric3D* data, int deleting) {
    if (data != 0) {
        *(void**)data = (void*)__vt__10WGeneric3D;
        unsigned int i = 0;
        WGeneric3D* p = data;
        do {
            if (p->items[0] != 0) {
                data->dispose(i);
                p->items[0] = 0;
            }
            ++i;
            p = (WGeneric3D*)((char*)p + 4);
        } while (i < kRenderItemCapacity);
        if (data->vertexBuffer != 0) {
            __dla__FPv(data->vertexBuffer);
            data->vertexBuffer = 0;
        }
        if (deleting > 0)
            __dl__FPv(data);
    }
    return data;
}
#pragma pop

// Allocate the shared vertex staging buffer and enable rendering.
void WGeneric3D_AllocateVertexBuffer(WGeneric3D* data) {
    data->vertexBuffer = __nwa__FUl(kVertexBufferBytes);
    data->submissionState = 0;
}

// Dispose every occupied render slot and release vertex staging storage.
void WGeneric3D_ReleaseItemsAndBuffer(WGeneric3D* data) {
    for (unsigned int i = 0; i < kRenderItemCapacity; ++i) {
        if (data->items[i] != 0) {
            data->dispose(i);
            data->items[i] = 0;
        }
    }
    if (data->vertexBuffer != 0) {
        __dla__FPv(data->vertexBuffer);
        data->vertexBuffer = 0;
    }
}

// Set RGB and endpoint transparency, then rebuild the 256-entry transparency ramp.
void WGeneric3D_SetColorRamp(WGeneric3D* data, unsigned int rgb, int startTransparency, unsigned int endTransparency, float startDepth, float endDepth) {
    if (rgb == 0 || (!startTransparency && endTransparency == 0)) {
        data->v16(0);
        return;
    }
    if (data->colorRampRgb != rgb)
        data->colorRampRgb = rgb & 0xFFFFFF;
    if (data->colorRampEndTransparency != endTransparency)
        data->colorRampEndTransparency = endTransparency;
    if (data->colorRampStart != startDepth)
        data->colorRampStart = startDepth;
    if (data->colorRampEnd != endDepth)
        data->colorRampEnd = endDepth;
    data->colorRampInverseSpan = kGeneric3DColorRampByteScale / (data->colorRampEnd - data->colorRampStart);

    int rampNumerator = 0;
    int transparencyDelta = endTransparency - startTransparency;
    for (int i = 0; 32 > i; ++i) {
        for (int j = 0; 8 > j; ++j) {
            int k = rampNumerator + transparencyDelta * j;
            data->colors[i * 8 + j] = (unsigned char)(255 - (startTransparency + k / 255));
        }
        rampNumerator += transparencyDelta * 8;
    }
    data->v16(1);
}

// Set fog enablement and return its previous value.
unsigned char WGeneric3D_SetFogEnabled(WGeneric3D* data, unsigned int value) {
    unsigned char previous = data->fogEnabled;
    data->fogEnabled = value;
    Engine_SetCachedRenderState(1, (unsigned char)(value & 0xFFFFFF));
    return previous;
}

// Store two unresolved parameters and the low 24 bits of their accompanying word.
// Only writes are recovered for +4854/+4858: their units and consumer are
// unresolved. They are distinct from the established depth ramp at +4840.
// Do not infer near/far or distance units from adjacency alone.
void WGeneric3D_SetFogState(WGeneric3D* data, float firstParameter, float secondParameter, unsigned int packedLow24) {
    data->value4854 = firstParameter;
    data->value4858 = secondParameter;
    data->value485C = packedLow24 & 0xFFFFFF;
}

// Set the selector that chooses packed position mode one rather than five.
void WGeneric3D_vfunc_28(WGeneric3D* data, unsigned int value) {
    data->usePositionMode1 = value;
}

// Return the packed-position mode selector.
unsigned int WGeneric3D_vfunc_29(WGeneric3D* data) {
    return data->usePositionMode1;
}

// Set the resource override captured by subsequently queued render entries.
void WGeneric3D_vfunc_30(WGeneric3D* data, unsigned int value) {
    data->resourceOverride = value;
}

// Submission policy, in precedence order:
// 1. Negative count: tagged payload ABI; bypass all vertex-array policy below.
// 2. Queue flag: retain one packed copy and enqueue one command.
// 3. Secondary texture + separate passes: render base, then emit secondary.
//    Without fog: base slot present -> immediate base + queued secondary;
//                 base slot absent -> immediate secondary only.
//    With fog: draw base when textured; queue secondary only if a probe
//              alpha is below 255 (fully opaque probes need no fog pass).
// 4. Otherwise: pack once and execute one immediate command.
// Choose topology and vertex packing, then queue or execute the required rendering passes.
void WGeneric3D_DrawPolygonFan(unsigned char* self, unsigned* src, unsigned flags, int count, int executeArgument) {
    // count >= 0: src borrows count vertex pointers. A closed strip needs count
    // <= 63 because a repeat of src[0] is added to closedStripVertices[64]. count < 0 switches
    // src to the tagged-payload ABI and executeArgument to a texture slot. Packing copies
    // vertices to renderer-owned staging storage; queued entries refer to that copy.
    // Phase 1: choose topology and derive independent draw flags / packing bits.
    unsigned closedStripVertices[64];
    unsigned int positionPacking;
    int primitiveType;
    if (count < 0) { WGeneric3D_QueueTaggedPrimitive(self, (TaggedPrimitiveRecord**)src, executeArgument, flags); return; }
    if ((*(int*)&AsWGeneric3D(self)->submissionState) == 1) return;
    positionPacking = (*(int*)&AsWGeneric3D(self)->usePositionMode1) ? kPositionPacking1 : kPositionPacking5;
    unsigned topology = flags & kPrimitiveTopologyMask;
    switch (topology) {
    case kClosedLineStrip: {
        int i = 0;
        for (; i < count; i++) closedStripVertices[i] = src[i];
        closedStripVertices[i] = src[0];
            count = (1 + count);
        src = closedStripVertices;
        flags &= ~kTriangleTopologyBit;
        primitiveType = kLineStripPrimitive;
        break;
    }
    case kOpenLineStrip:
        flags &= ~kTriangleTopologyBit;
        primitiveType = kLineStripPrimitive;
        break;
    case kTriangleTopologyBit:
    case 0x00000000:
        primitiveType = kTriangleFanPrimitive;
        break;
    }
    int vertexFormat = positionPacking;
    if (!(flags & kOmitVertexColor)) vertexFormat |= kPackedColorMask;
    if ((flags & kSecondaryTextureMask) >> 11) vertexFormat |= kTwoTexturePacking;
    else if ((flags & kTextureSlotMask) || (*(int*)&AsWGeneric3D(self)->resourceOverride)) vertexFormat |= kSingleTexturePacking;
    if (flags & kUseFog) {
        if (self[kFogEnabledOffset]) vertexFormat |= kFogPackingBit;
        if (!self[kFogEnabledOffset]) flags &= ~kUseFog;
    }

    if (flags & kQueuePrimitive) {
        // Phase 2a: stage once and enqueue. Fixed early/late passes use depth zero;
        // sortable passes take the first source vertex's depth as their key.
        unsigned renderPass = flags & kRenderPassMask;
        int sortByDepth = renderPass != kRenderPassMask && renderPass != kEarlyRenderPass;
        float sortDepth;
        if (sortByDepth) sortDepth = Generic3D_GetFirstVertexSortDepth(self, (TaggedPrimitiveRecord**)src, count);
        else sortDepth = kFloat_0_Generic3DDefaults;
        int stagedVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, vertexFormat, 1);
        WGeneric3D_EnqueueRenderEntry(self, flags, count, vertexFormat, stagedVertices, primitiveType, sortDepth);
        return;
    }

    // Phase 2b: separate texture passes may draw the base immediately and queue a
    // secondary pass. Texture bits 11..17 become the secondary pass's slot 0..6.
    // Packing with commitCursor=0 allows reuse; commit=1 retains staged bytes.
    if ((flags & kSecondaryTextureMask)) { if ((*(int*)&AsWGeneric3D(self)->separateTexturePassesEnabled) == 1) {
        if (!(vertexFormat & kFogPackingBit)) {
            if (flags & kTextureSlotMask) {
                // This split path now needs only the source-color presence bits.
                vertexFormat &= kPackedColorMask;
                if (vertexFormat) {
                    unsigned passVertexFormat = (unsigned)positionPacking | (kSingleTexturePacking | kPackedColorMask);

                    ((WGeneric3D*)self)->Execute(flags & ~(kSecondaryTextureMask | kSecondaryPassModeBit), count, passVertexFormat, (PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, passVertexFormat, 0)), primitiveType, executeArgument);
                } else {
                    unsigned passVertexFormat = (unsigned)positionPacking | kSingleTexturePacking;
                    int stagedVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, passVertexFormat, 0);
                    ((WGeneric3D*)self)->Execute(flags & ~(kSecondaryTextureMask | kSecondaryPassModeBit), count, passVertexFormat, stagedVertices, primitiveType, executeArgument);
                }
                if (flags & kSecondaryPassModeBit) {
                    if (vertexFormat) {
                        int stagedPassVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, 0, 1);
                        WGeneric3D_EnqueueRenderEntry(self, (unsigned)((int)(flags & kSecondaryTextureMask) >> 11) | kSecondaryColoredPass, count, (unsigned)positionPacking | kSingleTexturePacking, stagedPassVertices, primitiveType, kFloat_0_Generic3DDefaults);
                    } else {
                        int stagedPassVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, kPackedColorMask, 1);
                        WGeneric3D_EnqueueRenderEntry(self, (unsigned)((int)(flags & kSecondaryTextureMask) >> 11) | kSecondaryUncoloredPass, count, ((unsigned)positionPacking | kSingleTexturePacking) | kPackedColorMask, stagedPassVertices, primitiveType, kFloat_0_Generic3DDefaults);
                    }
                } else {
                    int stagedPassVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, 0, 1);
                    WGeneric3D_EnqueueRenderEntry(self, (unsigned)((int)(flags & kSecondaryTextureMask) >> 11) | kSecondaryDefaultPass, count, (unsigned)positionPacking | kSingleTexturePacking, stagedPassVertices, primitiveType, kFloat_0_Generic3DDefaults);
                }
            } else {
                // This split path now needs only the source-color presence bits.
                vertexFormat &= kPackedColorMask;
                if (vertexFormat) {
                    int stagedPassVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, 0, 1);
                    ((WGeneric3D*)self)->Execute((unsigned)((int)(flags & kSecondaryTextureMask) >> 11), count, (unsigned)positionPacking | kSingleTexturePacking, stagedPassVertices, primitiveType, executeArgument);
                } else {
                    int stagedPassVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, kPackedColorMask, 1);
                    ((WGeneric3D*)self)->Execute((unsigned)((int)(flags & kSecondaryTextureMask) >> 11) | kOmitVertexColor, count, ((unsigned)positionPacking | kSingleTexturePacking) | kPackedColorMask, stagedPassVertices, primitiveType, executeArgument);
                }
            }
        } else {
            if (flags & (kTextureSlotMask | kSecondaryTextureMask)) {
                unsigned passVertexFormat = (unsigned)positionPacking | (kFogPackingBit | kSingleTexturePacking);
                int stagedVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, passVertexFormat, 0);
                ((WGeneric3D*)self)->Execute(flags & ~kSecondaryTextureMask, count, (unsigned)positionPacking | kSingleTexturePacking, stagedVertices, primitiveType, executeArgument);
            }
            unsigned* fogProbe = (unsigned*)PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, kFogPackingBit, 1);
            for (int i = 0; i < count; i++) {
                if (*((unsigned char*)fogProbe + kFogProbeAlphaOffset) != 0xff) {
                    WGeneric3D_EnqueueRenderEntry(self, (unsigned)((int)(flags & kSecondaryTextureMask) >> 11) | kSecondaryFogPass, count, (unsigned)positionPacking | kSingleTexturePacking, (int)fogProbe, primitiveType, kFloat_0_Generic3DDefaults);
                    return;
                }
                fogProbe = (unsigned*)((unsigned char*)fogProbe + kFogProbeStride);
            }
        }
        return;
    } }

    // Phase 2c: one immediate draw when neither queue nor split-pass path applies.
    int stagedVertices = PackGenericVertexSources(self, (GenericVertexSource* const*)src, count, vertexFormat, 0);
    ((WGeneric3D*)self)->Execute(flags, count, vertexFormat, stagedVertices, primitiveType, executeArgument);
}

// Expand indexed triangles or closed line loops and submit their render commands.
void WGeneric3D_DrawIndexedTriangles(void* self, int* vertexMap, int unused, unsigned short* indices,
                int count, int flags, int executeArgument) {
    int texturePacking;
    int localFlags = flags;

    if (((*(int*)&AsWGeneric3D(self)->submissionState)) == 1)
        return;

    int vertexFormat;
    vertexFormat = (*(int*)&AsWGeneric3D(self)->usePositionMode1) != 0 ? 1 : 5;

    // Secondary slots are bits 11..17; zero selects base-slot packing.
    if (!((flags >> 11) & 0x7f)) {
        int bits = flags & kTextureSlotMask;
        texturePacking = (0 != bits) ? kIndexedSingleTexturePacking : 0;
    } else {
        texturePacking = kTwoTexturePacking;
    }

    int colorPacking = kPackedColorMask;
    colorPacking = (flags & kOmitVertexColor) ? 0 : colorPacking;
    vertexFormat = texturePacking | (vertexFormat | colorPacking);

    if (flags & kUseFog) {
        int enabled = AsWGeneric3D(self)->fogEnabled;
        if (enabled)
            vertexFormat |= kFogPackingBit;
        if (enabled == 0)
            localFlags = flags & ~kUseFog;
    }

    if (localFlags & kQueuePrimitive) { int i = 0;
        unsigned int offset = 0;

        int type = localFlags & kRenderPassMask;
        while (i < count) {
            int verts[3];
            verts[0] = vertexMap[indices[offset++]];
            verts[1] = vertexMap[indices[offset++]];
            verts[2] = vertexMap[indices[offset++]];

            int entryVertexFormat = vertexFormat;
            i = i + 3;
            int copyVertexFormat = vertexFormat;
            WGeneric3D_EnqueueRenderEntry(self, localFlags, 3, entryVertexFormat, (PackGenericVertexSources(self, (GenericVertexSource* const*)verts, 3, copyVertexFormat, 1)), kTriangleFanPrimitive, (((type != kRenderPassMask && type != kEarlyRenderPass)) ? Generic3D_GetFirstVertexSortDepth(self, verts, 3) : kFloat_0_Generic3DDefaults));
        }
    } else { int i = 0;
        unsigned int primitive = localFlags & kPrimitiveTopologyMask;
        if (primitive == 0 || primitive == 0x02000000)
            return;
        int offset = 0;

        while (i < count) { int first = vertexMap[indices[offset++]];

            int verts[4];
            verts[0] = first;
            verts[1] = vertexMap[indices[offset++]];
            verts[2] = vertexMap[indices[offset++]];
            i = i + 3;
            int loopCopyVertexFormat = vertexFormat;
            verts[3] = first;
            int result = PackGenericVertexSources(self, (GenericVertexSource* const*)verts, 4, loopCopyVertexFormat, 0);
            int loopDrawVertexFormat = vertexFormat;
            ((WGeneric3D*)self)->Execute(localFlags, 4, loopDrawVertexFormat, result, kLineStripPrimitive, executeArgument);
        }
    }
}

// Sort entry pointers by descending numeric sortKey (depth or texture-slot key).
void WGeneric3D_SortEntriesDescending(void* self, void* volatile* entries, int count) {
    if (count <= 0)
        return;

    Range* range;
    Range* ranges = (Range*)&s_generic3DSortRangeBuffer;
    int head = 0;
    int tail = 1;
    int last = count - 1;
    s_generic3DSortRangeBuffer[0] = 0;
    ranges->high = last;

    while (head != tail) {
        int low = ranges[head].low;
        range = &ranges[head];
        int rightIndex = range->high + 1;
        int leftIndex = low;
        float pivot = AsRenderEntry(entries[low])->sortKey;
        do {
            while (leftIndex < rightIndex - 1 &&
                   AsRenderEntry(entries[++leftIndex])->sortKey > pivot) {}
            do {
                --rightIndex;
            } while (AsRenderEntry(entries[rightIndex])->sortKey < pivot);
            if (leftIndex < rightIndex) {
                void* temp = entries[leftIndex];
                entries[leftIndex] = entries[rightIndex];
                entries[rightIndex] = temp;
            }
        } while (leftIndex < rightIndex);

        void* rightValue = entries[rightIndex];
        int lowNow = range->low;
        void* temp = entries[lowNow];
        entries[lowNow] = rightValue;
        entries[rightIndex] = temp;

        if (lowNow < rightIndex - 1) {
            ranges[tail].low = low;
            ranges[tail].high = rightIndex - 1;
            tail = (tail + 1) & kSortRangeMask;
        }
        if (rightIndex + 1 < range->high) {
            ranges[tail].low = rightIndex + 1;
            ranges[tail].high = range->high;
            tail = (tail + 1) & kSortRangeMask;
        }
        head = (head + 1) & kSortRangeMask;
    }
}

// Flush queued and fixed-pass commands in the order selected by the flush flags.
void WGeneric3D_Flush(void* self, int flags) {
    WGeneric3D* dispatch = (WGeneric3D*)self;
    dispatch->slot32();
    (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed) = 0;

    if ((*(int*)&AsWGeneric3D(self)->submissionState) == 1) {
        (*(int*)&AsWGeneric3D(self)->queuedEntryCount) = 0;
        (*(int*)&AsWGeneric3D(self)->renderEntryCount) = 0;
        return;
    }

    if ((*(int*)&AsWGeneric3D(self)->queuedEntryCount) == 0 &&
        (*(int*)&AsWGeneric3D(self)->renderEntryCount) == 0)
        return;

    dispatch->slot34();
    if (flags & kFlushQuadBatch) {
        dispatch->slot38();
    } else {
        if ((*(int*)&AsWGeneric3D(self)->queuedEntryCount) > 0) {
            if (flags & kFlushTwoPasses)
                dispatch->slot35(flags);
            else
                dispatch->slot36(flags);
        }
        dispatch->slot37();
    }

    (*(int*)&AsWGeneric3D(self)->queuedEntryCount) = 0;
    (*(int*)&AsWGeneric3D(self)->renderEntryCount) = 0;
}

#pragma push
#pragma use_lmw_stmw on
// Replay sorted entries in two passes, adjusting replay flags for the chosen direction.
void WGeneric3D_ReplaySortedEntriesTwoPass(void* self, int flags) {
    WGeneric3D_SortEntriesDescending(self, (void**)((char*)AsWGeneric3D(self)->queue), (*(int*)&AsWGeneric3D(self)->queuedEntryCount));

    char* current = (char*)self + ((*(int*)&AsWGeneric3D(self)->queuedEntryCount) - 1) * 4;
    int i = (*(int*)&AsWGeneric3D(self)->queuedEntryCount) - 1;

    while (0 <= i) {
        void* entry = AsWGeneric3D(current)->queue[0];
        if (*(unsigned char*)entry == 0) {
            AsRenderEntry(entry)->flagsOrTextureSlot &= 0x7fffffff;
            (*(int*)&AsWGeneric3D(self)->resourceOverride) = AsRenderEntry(entry)->resourceOverride;
            ((WGeneric3D*)self)->Execute(AsRenderEntry(entry)->flagsOrTextureSlot,
                                            AsRenderEntry(entry)->vertexCount,
                                            AsRenderEntry(entry)->vertexFormatOrPrimitiveFlags,
                                            AsRenderEntry(entry)->vertexData,
                                            AsRenderEntry(entry)->primitiveType, 0);
            (*(int*)&AsWGeneric3D(self)->resourceOverride) = 0;
        } else {
            ((TaggedEntryFlags*)entry)->flags &= 0x7fffffff;
        }
        i -= 1;
        current = current - 4;
    }

    if (!(!(kFlushReverseOrder & flags))) {
        i = (*(int*)&AsWGeneric3D(self)->queuedEntryCount) - 1;
        current = (char*)self + ((*(int*)&AsWGeneric3D(self)->queuedEntryCount) - 1) * 4;
        while (0 <= i) {
            void* entry = AsWGeneric3D(current)->queue[0];
            if (*(unsigned char*)entry == 0) {
                AsRenderEntry(entry)->flagsOrTextureSlot &= 0xdfffffff;
                (*(int*)&AsWGeneric3D(self)->resourceOverride) = AsRenderEntry(entry)->resourceOverride;
                ((WGeneric3D*)self)->Execute(AsRenderEntry(entry)->flagsOrTextureSlot,
                                                AsRenderEntry(entry)->vertexCount,
                                                AsRenderEntry(entry)->vertexFormatOrPrimitiveFlags,
                                                AsRenderEntry(entry)->vertexData,
                                                AsRenderEntry(entry)->primitiveType, 0);
                (*(int*)&AsWGeneric3D(self)->resourceOverride) = 0;
            } else {
                ((TaggedEntryFlags*)entry)->flags &= 0xdfffffff;
            }
            current = current - 4;
            i--;
        }
    } else {
        current = (char*)self;
        i = 0;
        while (i < (*(int*)&AsWGeneric3D(self)->queuedEntryCount)) {
            void* entry = AsWGeneric3D(current)->queue[0];
            if (*(unsigned char*)entry == 0) {
                AsRenderEntry(entry)->flagsOrTextureSlot |= 0x20000000;
                (*(int*)&AsWGeneric3D(self)->resourceOverride) = AsRenderEntry(entry)->resourceOverride;
                ((WGeneric3D*)self)->Execute(AsRenderEntry(entry)->flagsOrTextureSlot,
                                                AsRenderEntry(entry)->vertexCount,
                                                AsRenderEntry(entry)->vertexFormatOrPrimitiveFlags,
                                                AsRenderEntry(entry)->vertexData,
                                                AsRenderEntry(entry)->primitiveType, 0);
                (*(int*)&AsWGeneric3D(self)->resourceOverride) = 0;
            } else {
                ((TaggedEntryFlags*)entry)->flags |= 0x20000000;
            }

            current = (4 + current);
            i += 1;
        }
    }
}
#pragma pop

// Replay sorted entries in one pass, adjusting replay flags for the chosen direction.
void WGeneric3D_FlushQueueSinglePass(void* self, int flags) {
    WGeneric3D_SortEntriesDescending(self, (void**)((char*)AsWGeneric3D(self)->queue),
                (*(int*)&AsWGeneric3D(self)->queuedEntryCount));

    if (kFlushReverseOrder & flags) {
        char* current;
        int i = (*(int*)&AsWGeneric3D(self)->queuedEntryCount) - 1;
        current = (char*)self + i * 4;
        while (i >= 0) {
            void* entry = AsWGeneric3D(current)->queue[0];
            if (*(unsigned char*)entry == 0) {
                AsRenderEntry(entry)->flagsOrTextureSlot &= 0x5fffffff;
                (*(int*)&AsWGeneric3D(self)->resourceOverride) = AsRenderEntry(entry)->resourceOverride;
                ((WGeneric3D*)self)->Execute(AsRenderEntry(entry)->flagsOrTextureSlot,
                                                AsRenderEntry(entry)->vertexCount,
                                                AsRenderEntry(entry)->vertexFormatOrPrimitiveFlags,
                                                AsRenderEntry(entry)->vertexData,
                                                AsRenderEntry(entry)->primitiveType, 0);
                (*(int*)&AsWGeneric3D(self)->resourceOverride) = 0;
            } else {
                ((TaggedEntryFlags*)entry)->flags &= 0x5fffffff;
            }
            current -= 4;
            --i;
        }
    } else {
        int i = 0;
        char* current = (char*)self;
        while (i < (*(int*)&AsWGeneric3D(self)->queuedEntryCount)) {
            void* entry = AsWGeneric3D(current)->queue[0];
            if (*(unsigned char*)entry == 0) {
                AsRenderEntry(entry)->flagsOrTextureSlot =
                    (AsRenderEntry(entry)->flagsOrTextureSlot & 0x7fffffff) | 0x20000000;
                (*(int*)&AsWGeneric3D(self)->resourceOverride) = AsRenderEntry(entry)->resourceOverride;
                ((WGeneric3D*)self)->Execute(AsRenderEntry(entry)->flagsOrTextureSlot,
                                                AsRenderEntry(entry)->vertexCount,
                                                AsRenderEntry(entry)->vertexFormatOrPrimitiveFlags,
                                                AsRenderEntry(entry)->vertexData,
                                                AsRenderEntry(entry)->primitiveType, 0);
                (*(int*)&AsWGeneric3D(self)->resourceOverride) = 0;
            } else {
                ((TaggedEntryFlags*)entry)->flags =
                    (((TaggedEntryFlags*)entry)->flags & 0x7fffffff) | 0x20000000;
            }
            current = (current + 4);
            ++i;
        }
    }
}

// Install the two blend-state words for a new tagged-quad batch.
// State IDs 13/14 retain their engine protocol numbers; values are the existing recipes.
static inline void ApplyTaggedQuadBlendMode(int blendMode) {
    switch (blendMode) {
        case kQuadAlphaBlend:
        Engine_SetCachedRenderState(13, 4);
        Engine_SetCachedRenderState(14, 5);
        break;
        case kQuadAdditiveAlphaBlend:
        Engine_SetCachedRenderState(13, 4);
        Engine_SetCachedRenderState(14, 1);
        break;
        case kQuadBlendMode2:
        Engine_SetCachedRenderState(13, 0);
        Engine_SetCachedRenderState(14, 2);
        break;
        case kQuadBlendMode3:
        Engine_SetCachedRenderState(13, 0);
        Engine_SetCachedRenderState(14, 3);
        break;
        default:
        Engine_SetCachedRenderState(13, 4);
        Engine_SetCachedRenderState(14, 5);
        break;
    }
}

// Batch tagged quads by texture, blend mode and vertex format before drawing them.
void WGeneric3D_DrawTaggedQuadBatches(void* self) {
    // One batch holds <=32 quads (128 vertices), all sharing texture slot, blend
    // mode and explicit-corner packing. QuadBatchVertex describes the six-word
    // xyz/RGBA/UV layout, keeping color bytes separate from float coordinates.
    // Phase 1: sort keys descending and install shared view/projection/render state.
    // QueueTaggedPrimitive uses the texture-slot number as its key, not depth.
    int cornerSideFlags[4];
    TblB2 textureStageStates;
    Vec3_83 basisOffset;
    TblC2 renderStates;
    QuadBatchVertex batchVertices[kQuadBatchCapacity * 4];
    Vec3_83 quadCenter;
    float localCorner[3];
    TblAll* defaults = &s_generic3DQuadRenderDefaults;
    if ((*(int*)&AsWGeneric3D(self)->queuedEntryCount) == 0) return;
    WGeneric3D_SortEntriesDescending(self, (void* volatile*)((char*)AsWGeneric3D(self)->queue), (*(int*)&AsWGeneric3D(self)->queuedEntryCount));
    textureStageStates = defaults->b;
    TblA2 a;
    a = defaults->a;
    renderStates = defaults->c;
    Engine_SetRenderStateList(&renderStates, 12);
    Engine_SetTextureStageStateList(0, &textureStageStates, 6);
    Engine_SetTextureStageParameterList(0, &a, 5);
    Engine_SetCachedRenderState(1, AsWGeneric3D(self)->fogEnabled);
    void* device = Camera_GetRenderState(g_view);
    SetTransform(1, ((CameraRenderMatrices*)device)->view);
    int firstCornerFlag = defaults->flags[0];
    SetTransform(0, ((CameraRenderMatrices*)device)->projection);
    SetTransform(2, kIdentityMatrix4x4);
    SetTransform(11, kIdentityMatrix4x4);
    Engine_SetType7Resource(0);
    Engine_SetFlexibleVertexFormat(kQuadVertexPacking1);
    cornerSideFlags[0] = firstCornerFlag;
    cornerSideFlags[1] = defaults->flags[1];
    cornerSideFlags[2] = defaults->flags[2];
    int quadCount = 0;
    cornerSideFlags[3] = defaults->flags[3];
    char* queueCursor = (char*)self;
    int previousTextureSlot = -1;
    unsigned int previousBlendMode = 0x7fffffff;
    int previousExplicitCorners = 0;
    for (int n = 0; n < (*(int*)&AsWGeneric3D(self)->queuedEntryCount); ++n) {
        // Phase 2: flush before changing a batch's texture or blend state.
        // The previous-texture key is the slot ID or captured resource token;
        // the captured override is consulted when that key changes.
        RenderEntry* queuedEntry = (RenderEntry*)AsWGeneric3D(queueCursor)->queue[0];
        char* payload = (char*)queuedEntry->vertexData;
        int textureSlot = queuedEntry->flagsOrTextureSlot;
        int explicitCorners = 0;
        if (previousTextureSlot != textureSlot) {
            previousTextureSlot = textureSlot;
            if (quadCount) {
                DrawPrimitiveUP(kQuadPrimitive, quadCount, batchVertices, kPackedQuadVertexBytes);
                quadCount = 0;
            }
            void* overrideResource = (*(void**)&AsRenderEntry(queuedEntry)->resourceOverride);
            if (overrideResource) {
                Engine_SetTextureStageResource(0, ISharedResource_GetLocked((*(void**)&AsRenderEntry(queuedEntry)->resourceOverride)));
                Engine_SetTexturePalette(0, ISharedResource_GetPaletteObject((*(void**)&AsRenderEntry(queuedEntry)->resourceOverride)));
            }
            else {
                Engine_SetTextureStageResource(0, ((WGeneric3D*)self)->slot26(queuedEntry->flagsOrTextureSlot));
                Engine_SetTexturePalette(0, ((WGeneric3D*)self)->slot27(queuedEntry->flagsOrTextureSlot));
            }
        }
        int blendMode = (*(unsigned int*)&AsRenderEntry(queuedEntry)->vertexFormatOrPrimitiveFlags) & kQuadBlendModeMask;
        if (blendMode != previousBlendMode) {
            previousBlendMode = blendMode;
            if (quadCount) {
                DrawPrimitiveUP(kQuadPrimitive, quadCount, batchVertices, kPackedQuadVertexBytes);
                quadCount = 0;
            }
        }
        if (!quadCount) {
            ApplyTaggedQuadBlendMode(blendMode);
        }
        if (((TaggedPrimitiveRecord*)payload)->tag == kExplicitQuadCorners) explicitCorners = 1;
        if (previousExplicitCorners != explicitCorners) {
            DrawPrimitiveUP(kQuadPrimitive, quadCount, batchVertices, kPackedQuadVertexBytes);
            if (explicitCorners) Engine_SetFlexibleVertexFormat(kQuadVertexPacking5);
            else Engine_SetFlexibleVertexFormat(kQuadVertexPacking1);
            previousExplicitCorners = explicitCorners;
            quadCount = 0;
        }
        unsigned int firstVertex = quadCount << 2;
        // Phase 3: expand each tag into four renderer-space corners. The installed
        // view/projection handles the following draw; do not transform twice.
        switch (((TaggedPrimitiveRecord*)payload)->tag) {
            case kCameraFacingQuad:
            {
                // x/y use center + cameraRight*horizontal + cameraUp*vertical.
                // z deliberately selects the OPPOSITE horizontal/vertical extents;
                // preserve that component convention rather than vectorizing it.
                QuadBatchVertex* vertex = (QuadBatchVertex*)((char*)batchVertices + firstVertex * kPackedQuadVertexBytes);
                int* cornerSideCursor = cornerSideFlags;
                char* cameraView = (char*)g_view;
                for (int v = 0; v < 4; ++v, ++cornerSideCursor, ++vertex) {
                    float horizontalOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left;
                    int vValue = v;
                    float selectedOffset = (vValue & 2) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom;
                    vertex[0].x = ((BillboardCameraAxes*)cameraView)->right[0] * horizontalOffset + ((TaggedPrimitiveRecord*)payload)->centerX + ((BillboardCameraAxes*)cameraView)->up[0] * selectedOffset;
                    horizontalOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left;
                    selectedOffset = (2 & v) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom;
                    vertex[0].y = ((BillboardCameraAxes*)cameraView)->right[1] * horizontalOffset + ((TaggedPrimitiveRecord*)payload)->centerY + ((BillboardCameraAxes*)cameraView)->up[1] * selectedOffset;
                    selectedOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->left : ((TaggedPrimitiveRecord*)payload)->right;
                    vertex[0].z = ((BillboardCameraAxes*)cameraView)->right[2] * selectedOffset + ((TaggedPrimitiveRecord*)payload)->centerZ + ((BillboardCameraAxes*)cameraView)->up[2] * ((v & 2) ? ((TaggedPrimitiveRecord*)payload)->bottom : ((TaggedPrimitiveRecord*)payload)->top);
                }
                break;
            }
            case kUprightQuad:
            case kUprightQuadAlt:
            {
                // Camera-right offsets supply the horizontal direction, while
                // top/bottom add directly to the center's vertical component.
                char* cameraView = (char*)g_view;
                QuadBatchVertex* vertex = (QuadBatchVertex*)((char*)batchVertices + firstVertex * kPackedQuadVertexBytes);
                int* cornerSideCursor = cornerSideFlags;
                for (int v = 0; v < 4; ++v, ++cornerSideCursor, ++vertex) {
                    float horizontalOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left;
                    vertex[0].x = ((BillboardCameraAxes*)cameraView)->right[0] * horizontalOffset + ((TaggedPrimitiveRecord*)payload)->centerX;
                    horizontalOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left;
                    float verticalOffset = (v >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom;
                    vertex[0].y = verticalOffset + (((BillboardCameraAxes*)cameraView)->right[1] * horizontalOffset + ((TaggedPrimitiveRecord*)payload)->centerY);
                    horizontalOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left;
                    vertex[0].z = ((BillboardCameraAxes*)cameraView)->right[2] * horizontalOffset + ((TaggedPrimitiveRecord*)payload)->centerZ;
                }
                break;
            }
            case kXYQuad:
            {
                // Fixed XY plane: z stays at center; emit two corners per row.
                int* cornerSideCursor = cornerSideFlags;
                QuadBatchVertex* vertex = (QuadBatchVertex*)((char*)batchVertices + firstVertex * kPackedQuadVertexBytes);
                int v = 0;
                for (int i = 0; 2 > i; ++i) {
                    vertex[0].x = ((TaggedPrimitiveRecord*)payload)->centerX + (cornerSideCursor[0] ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left);
                    vertex[0].y = ((TaggedPrimitiveRecord*)payload)->centerY + ((v >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom);
                    vertex[0].z = ((TaggedPrimitiveRecord*)payload)->centerZ;
                    ++v;
                    vertex[1].x = ((TaggedPrimitiveRecord*)payload)->centerX + (cornerSideCursor[1] ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left);
                    vertex[1].y = ((TaggedPrimitiveRecord*)payload)->centerY + ((v >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom);
                    v++;
                    vertex[1].z = ((TaggedPrimitiveRecord*)payload)->centerZ;
                    vertex = vertex + 2;
                    cornerSideCursor = cornerSideCursor + 2;
                }
                break;
            }
            case kYZQuad:
            {
                // Fixed YZ plane: x stays at center and horizontal offsets negate z.
                int* cornerSideCursor = cornerSideFlags;
                int v = 0;
                QuadBatchVertex* vertex = (QuadBatchVertex*)((char*)batchVertices + firstVertex * kPackedQuadVertexBytes);
                for (int i = 0; 2 > i; ++i) {
                    vertex[0].x = ((TaggedPrimitiveRecord*)payload)->centerX;
                    int vValue2 = v;
                    vertex[0].y = ((TaggedPrimitiveRecord*)payload)->centerY + ((vValue2 >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom);
                    vertex[0].z = ((TaggedPrimitiveRecord*)payload)->centerZ - (cornerSideCursor[0] ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left);
                    v += 1;
                    vertex[1].x = ((TaggedPrimitiveRecord*)payload)->centerX;
                    int vValue3 = v;
                    vertex[1].y = ((TaggedPrimitiveRecord*)payload)->centerY + ((vValue3 >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom);
                    vertex[1].z = ((TaggedPrimitiveRecord*)payload)->centerZ - (cornerSideCursor[1] ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left);
                    v++;
                    cornerSideCursor = 2 + cornerSideCursor;
                    vertex += 2;
                }
                break;
            }
            case kXZQuad:
            {
                // Fixed XZ plane: y stays at center; top/bottom select z offsets.
                int* cornerSideCursor = cornerSideFlags;
                int v = 0;
                QuadBatchVertex* vertex = (QuadBatchVertex*)((char*)batchVertices + firstVertex * kPackedQuadVertexBytes);
                for (int i = 0; 2 > i; ++i) {
                    vertex[0].x = ((TaggedPrimitiveRecord*)payload)->centerX + (cornerSideCursor[0] ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left);
                    vertex[0].y = ((TaggedPrimitiveRecord*)payload)->centerY;
                    int vValue4 = v;
                    vertex[0].z = ((TaggedPrimitiveRecord*)payload)->centerZ + ((vValue4 >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom);
                    ++v;
                    vertex[1].x = ((TaggedPrimitiveRecord*)payload)->centerX + (cornerSideCursor[1] ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left);
                    vertex[1].y = ((TaggedPrimitiveRecord*)payload)->centerY;
                    int vValue5 = v;
                    vertex[1].z = ((TaggedPrimitiveRecord*)payload)->centerZ + ((vValue5 >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom);
                    v += 1;
                    cornerSideCursor += 2;
                    vertex += 2;
                }
                break;
            }
            case kRotatedCameraQuad:
            case kRotatedCameraQuadAlt:
            {
                // Rotate corner offsets in 2D, then combine the first two shared
                // camera basis triples with center. Trig argument = unsigned
                // rotation1024 / 1024 radians (Math_*Approx in gtl_vector_matrix_math.cpp
                // folds whole turns using kInvTwoPi_AngleWrap). This is an
                // unsigned fixed-point radian field, not 1024 steps per turn.
                quadCenter.x = ((TaggedPrimitiveRecord*)payload)->centerX;
                quadCenter.y = ((TaggedPrimitiveRecord*)payload)->centerY;
                quadCenter.z = ((TaggedPrimitiveRecord*)payload)->centerZ;
                if (((TaggedPrimitiveRecord*)payload)->rotation1024 != 0) {
                    float trigAngle = (float)(unsigned int)((TaggedPrimitiveRecord*)payload)->rotation1024 * 0.0009765625f;
                    float cosAngle = Math_CosApprox(trigAngle);
                    float sinAngle = Math_SinApprox(trigAngle);
                    int* cornerSideCursor = cornerSideFlags;
                    for (int v = 0; 4 > v; ++v, ++cornerSideCursor) {
                        float horizontalOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left;
                        float cornerOffset = (v >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom;
                        float rotatedHorizontal = cosAngle * horizontalOffset - sinAngle * cornerOffset;
                        cornerOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left;
                        float verticalOffset = (v >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom;
                        float rotatedVertical = cosAngle * verticalOffset + (sinAngle * cornerOffset);
                        Vec3_Scale(&basisOffset, s_cameraTransform, rotatedHorizontal);
                        Vec3_83* outputPosition = (Vec3_83*)((char*)batchVertices + (v + firstVertex) * kPackedQuadVertexBytes);
                        Vec3_Add(outputPosition, &quadCenter, &basisOffset);
                        Vec3_Scale(&basisOffset, (3 + s_cameraTransform), rotatedVertical);
                        Vec3_Add(outputPosition, outputPosition, &basisOffset);
                    }
                }
                else {
                    int* cornerSideCursor = cornerSideFlags;
                    for (int v = 0; 4 > v; ++v, ++cornerSideCursor) {
                        float horizontalOffset = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->right : ((TaggedPrimitiveRecord*)payload)->left;
                        float verticalOffset = (v >> 1) ? ((TaggedPrimitiveRecord*)payload)->top : ((TaggedPrimitiveRecord*)payload)->bottom;
                        Vec3_Scale(&basisOffset, s_cameraTransform, horizontalOffset);
                        Vec3_83* outputPosition = (Vec3_83*)((char*)batchVertices + (v + firstVertex) * kPackedQuadVertexBytes);
                        Vec3_Add(outputPosition, &quadCenter, &basisOffset);
                        Vec3_Scale(&basisOffset, 3 + s_cameraTransform, verticalOffset);
                        Vec3_Add(outputPosition, outputPosition, &basisOffset);
                    }
                }
                break;
            }
            case kTransformedQuad:
            {
                // Build the payload's affine basis/translation, transform local
                // (horizontal,vertical,0), then add center separately. Here the
                // first payload words are basis data, not ordinary UV endpoints.
                int* cornerSideCursor = cornerSideFlags;
                float localTransform[16];
                localTransform[0] = ((TransformedQuadPayload*)payload)->axisX[0];
                QuadBatchVertex* vertex = (QuadBatchVertex*)((char*)batchVertices + firstVertex * kPackedQuadVertexBytes);
                localTransform[1] = ((TransformedQuadPayload*)payload)->axisX[1];
                localTransform[2] = ((TransformedQuadPayload*)payload)->axisX[2];
                localTransform[3] = 0.0f;
                localTransform[4] = ((TransformedQuadPayload*)payload)->axisY[0];
                localTransform[5] = ((TransformedQuadPayload*)payload)->axisY[1];
                localTransform[6] = ((TransformedQuadPayload*)payload)->axisY[2];
                localTransform[7] = 0.0f;
                localTransform[8] = ((TransformedQuadPayload*)payload)->axisZ[0];
                localTransform[9] = ((TransformedQuadPayload*)payload)->axisZ[1];
                localTransform[10] = ((TransformedQuadPayload*)payload)->axisZ[2];
                localTransform[11] = 0.0f;
                localTransform[12] = ((TransformedQuadPayload*)payload)->translation[0];
                localTransform[13] = ((TransformedQuadPayload*)payload)->translation[1];
                localTransform[14] = ((TransformedQuadPayload*)payload)->translation[2];
                localTransform[15] = 1.0f;
                localCorner[2] = 0.0f;
                for (int v = 0; 4 > v; ++v, ++cornerSideCursor, ++vertex) {
                    localCorner[0] = *cornerSideCursor ? ((TransformedQuadPayload*)payload)->right : ((TransformedQuadPayload*)payload)->left;
                    localCorner[1] = (v >> 1) ? ((TransformedQuadPayload*)payload)->top : ((TransformedQuadPayload*)payload)->bottom;
                    float transformedCorner[4];
                    Mtx44_TransformPointToVec4((Vec3_83*)transformedCorner, (Vec3_83*)localCorner, localTransform);
                    vertex[0].x = ((TransformedQuadPayload*)payload)->centerX + transformedCorner[0];
                    vertex[0].y = ((TransformedQuadPayload*)payload)->centerY + transformedCorner[1];
                    vertex[0].z = ((TransformedQuadPayload*)payload)->centerZ + transformedCorner[2];
                }
                break;
            }
            case kExplicitQuadCorners:
            {
                // Copy explicit xyz/UV corners with a z sign flip; the common
                // color fill below still uses the first payload's packed color.
                QuadBatchVertex* vertex = (QuadBatchVertex*)((char*)batchVertices + firstVertex * kPackedQuadVertexBytes);
                vertex[0].x = ((GenericVertexSource*)payload)[0].x;
                vertex[0].y = ((GenericVertexSource*)payload)[0].y;
                vertex[0].z = -((GenericVertexSource*)payload)[0].z;
                vertex[0].u = ((GenericVertexSource*)payload)[0].u0;
                vertex[0].v = ((GenericVertexSource*)payload)[0].v0;
                vertex[1].x = ((GenericVertexSource*)payload)[1].x;
                vertex[1].y = ((GenericVertexSource*)payload)[1].y;
                vertex[1].z = -((GenericVertexSource*)payload)[1].z;
                vertex[1].u = ((GenericVertexSource*)payload)[1].u0;
                vertex[1].v = ((GenericVertexSource*)payload)[1].v0;
                vertex[2].x = ((GenericVertexSource*)payload)[2].x;
                vertex[2].y = ((GenericVertexSource*)payload)[2].y;
                vertex[2].z = -((GenericVertexSource*)payload)[2].z;
                vertex[2].u = ((GenericVertexSource*)payload)[2].u0;
                vertex[2].v = ((GenericVertexSource*)payload)[2].v0;
                vertex[3].x = ((GenericVertexSource*)payload)[3].x;
                vertex[3].y = ((GenericVertexSource*)payload)[3].y;
                vertex[3].z = -((GenericVertexSource*)payload)[3].z;
                vertex[3].u = ((GenericVertexSource*)payload)[3].u0;
                vertex[3].v = ((GenericVertexSource*)payload)[3].v0;
                break;
            }
            default:
            break;
        }
        {
            // Phase 4: fill RGBA and tag-specific UVs, retaining FE's explicit UVs.
            QuadBatchVertex* vertex = (QuadBatchVertex*)((char*)batchVertices + firstVertex * kPackedQuadVertexBytes);
            int* cornerSideCursor = cornerSideFlags;
            for (int v = 0; v < 4; ++v, ++cornerSideCursor, ++vertex) {
                vertex->color[2] = ((TaggedPrimitiveRecord*)payload)->argb;
                vertex->color[1] = ((TaggedPrimitiveRecord*)payload)->argb >> 8;
                vertex->color[0] = ((TaggedPrimitiveRecord*)payload)->argb >> 16;
                vertex->color[3] = ((TaggedPrimitiveRecord*)payload)->argb >> 24;
                if (((TaggedPrimitiveRecord*)payload)->tag == kExplicitQuadCorners) {
                }
                else if (((TaggedPrimitiveRecord*)payload)->tag == kTransformedQuad) {
                    vertex[0].u = *cornerSideCursor ? ((TransformedQuadPayload*)payload)->u1 : ((TransformedQuadPayload*)payload)->u0;
                    int vValue6 = v;
                    vertex[0].v = (2 & vValue6) ? ((TransformedQuadPayload*)payload)->v1 : ((TransformedQuadPayload*)payload)->v0;
                }
                else {
                    vertex[0].u = *cornerSideCursor ? ((TaggedPrimitiveRecord*)payload)->u1 : ((TaggedPrimitiveRecord*)payload)->u0;
                    vertex[0].v = (2 & v) ? ((TaggedPrimitiveRecord*)payload)->v1 : ((TaggedPrimitiveRecord*)payload)->v0;
                }
            }
        }
        // Phase 5: count the completed quad and flush at the 32-quad boundary.
        int sum2 = quadCount + 1;
        quadCount = sum2;
        if (quadCount == kQuadBatchCapacity) {
            DrawPrimitiveUP(kQuadPrimitive, kQuadBatchCapacity, batchVertices, kPackedQuadVertexBytes);
            quadCount = 0;
        }
        queueCursor = queueCursor + 4;
    }
    // Submit the tail; queue ownership/count is managed by the caller's flush path.
    if (quadCount) DrawPrimitiveUP(kQuadPrimitive, quadCount, batchVertices, kPackedQuadVertexBytes);
}

// Execute commands assigned to the early fixed render pass.
void WGeneric3D_ExecuteEarlyRenderPass(void* self) {
    int clearedResourceOverride = 0;
    char* entry = (char*)AsWGeneric3D(self)->entries;
    int i = 0;

    while (i < (*(int*)&AsWGeneric3D(self)->renderEntryCount)) {
        int flags = AsRenderEntry(entry)->flagsOrTextureSlot;
        if ((flags & kRenderPassMask) == kEarlyRenderPass) {
            (*(volatile int*)&AsWGeneric3D(self)->resourceOverride) = AsRenderEntry(entry)->resourceOverride;
            ((WGeneric3D*)self)->Execute(AsRenderEntry(entry)->flagsOrTextureSlot, AsRenderEntry(entry)->vertexCount,
                                            AsRenderEntry(entry)->vertexFormatOrPrimitiveFlags, AsRenderEntry(entry)->vertexData,
                                            AsRenderEntry(entry)->primitiveType, 0);
            (*(volatile int*)&AsWGeneric3D(self)->resourceOverride) = clearedResourceOverride;
        }
        ++i;
        entry += 0x20;
    }
}

// Execute commands assigned to the late fixed render pass.
void WGeneric3D_ExecuteLateRenderPass(void* self) {
    int clearedResourceOverride = 0;
    char* entry = (char*)AsWGeneric3D(self)->entries;
    int i = 0;

    while (i < (*(int*)&AsWGeneric3D(self)->renderEntryCount)) {
        int flags = AsRenderEntry(entry)->flagsOrTextureSlot;
        if ((flags & kRenderPassMask) == kLateRenderPass) {
            (*(volatile int*)&AsWGeneric3D(self)->resourceOverride) = AsRenderEntry(entry)->resourceOverride;
            ((WGeneric3D*)self)->Execute(AsRenderEntry(entry)->flagsOrTextureSlot, AsRenderEntry(entry)->vertexCount,
                                            AsRenderEntry(entry)->vertexFormatOrPrimitiveFlags, AsRenderEntry(entry)->vertexData,
                                            AsRenderEntry(entry)->primitiveType, 0);
            (*(volatile int*)&AsWGeneric3D(self)->resourceOverride) = clearedResourceOverride;
        }
        ++i;
        entry += 0x20;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Capture draw arguments and resource override, queuing sortable commands and flushing at capacity.
void WGeneric3D_EnqueueRenderEntry(void* self, int drawFlags, int vertexCount, int vertexFormat, int vertexData, int primitiveType, float sortKey) {
    int count = (*(int*)&AsWGeneric3D(self)->renderEntryCount);
    (*(int*)&AsWGeneric3D(self)->renderEntryCount) = count + 1;

    RenderEntry* entry = (RenderEntry*)((char*)AsWGeneric3D(self)->entries) + count;
    entry->state = 0;
    entry->flagsOrTextureSlot = drawFlags;
    entry->vertexCount = vertexCount;
    entry->vertexFormatOrPrimitiveFlags = vertexFormat;
    entry->vertexData = vertexData;
    entry->primitiveType = primitiveType;
    entry->resourceOverride = (*(int*)&AsWGeneric3D(self)->resourceOverride);

    if ((drawFlags & kRenderPassMask) == 0) {
        entry->sortKey = sortKey;
        enqueue((WGeneric3D*)self, entry);
    }

    if ((*(int*)&AsWGeneric3D(self)->renderEntryCount) == kRenderEntryCapacity) {
        ((WGeneric3D*)self)->Flush(0);
    }
}
#pragma pop

// Copy a tagged quad payload into staging storage and enqueue its depth/texture entry.
void WGeneric3D_QueueTaggedPrimitive(void* self, TaggedPrimitiveRecord** inputs, int textureSlot, int primitiveFlags) {
    if ((*(int*)&AsWGeneric3D(self)->renderEntryCount) == kRenderEntryCapacity) {
        ((WGeneric3D*)self)->slot38();
        (*(int*)&AsWGeneric3D(self)->renderEntryCount) = 0;
        (*(int*)&AsWGeneric3D(self)->queuedEntryCount) = 0;
    }

    int index = (*(int*)&AsWGeneric3D(self)->renderEntryCount);
    (*(int*)&AsWGeneric3D(self)->renderEntryCount) = index + 1;
    RenderEntry* entry = (RenderEntry*)((char*)AsWGeneric3D(self)->entries) + index;
    entry->state = 0;
    entry->flagsOrTextureSlot = textureSlot;
    entry->vertexFormatOrPrimitiveFlags = primitiveFlags;
    entry->primitiveType = kTriangleStripPrimitive;
    entry->resourceOverride = (*(int*)&AsWGeneric3D(self)->resourceOverride);
    if ((*(int*)&AsWGeneric3D(self)->resourceOverride) != 0)
        entry->flagsOrTextureSlot = (*(int*)&AsWGeneric3D(self)->resourceOverride);

    entry->vertexData = (*(int*)&AsWGeneric3D(self)->vertexBuffer) + (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed);

    TaggedPrimitiveRecord* src0 = *inputs;
    char* out = (char*)(*(int*)&AsWGeneric3D(self)->vertexBuffer);
    *(float*)(out += (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed)) = src0->u0;
    ((TaggedPrimitiveRecord*)out)->v0 = src0->v0;
    ((TaggedPrimitiveRecord*)out)->u1 = src0->u1;
    ((TaggedPrimitiveRecord*)out)->v1 = src0->v1;
    ((TaggedPrimitiveRecord*)out)->argb = src0->argb;
    ((TaggedPrimitiveRecord*)out)->rotation1024 = src0->rotation1024;
    ((TaggedPrimitiveRecord*)out)->left = src0->left;
    ((TaggedPrimitiveRecord*)out)->top = src0->top;
    ((TaggedPrimitiveRecord*)out)->right = src0->right;
    ((TaggedPrimitiveRecord*)out)->bottom = src0->bottom;
    ((TaggedPrimitiveRecord*)out)->centerX = src0->centerX;
    ((TaggedPrimitiveRecord*)out)->centerY = src0->centerY;
    ((TaggedPrimitiveRecord*)out)->centerZ = src0->centerZ;
    ((TaggedPrimitiveRecord*)out)->tag = src0->tag;

    (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed) = (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed) + 0x38;

    unsigned char tag = ((TaggedPrimitiveRecord*)*inputs)->tag;
    if (tag == kTransformedQuad) {
        copyData38(((TaggedPrimitiveRecord*)out + 1), inputs[1]);
        copyData38(((TaggedPrimitiveRecord*)out + 2), inputs[2]);
        (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed) = (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed) + 0x70;
    } else if (tag == kExplicitQuadCorners) {
        copyData38(((TaggedPrimitiveRecord*)out + 1), inputs[1]);
        copyData38(((TaggedPrimitiveRecord*)out + 2), inputs[3]);
        copyData38(((TaggedPrimitiveRecord*)out + 3), inputs[2]);
        (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed) = (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed) + 0xa8;
    }

    entry->sortKey = (float)textureSlot;
    enqueue((WGeneric3D*)self, entry);
}

// Reserve the next unused slot in either the reserved or general slot range.
int WGeneric3D_AllocateRenderSlot(void* self, int flag) {
    register int* slot;
    register int index;
    register int tries;

    if (flag != 0) {
        tries = 0;
        while (occupiedSlot(self, 0x2428, index, slot) != 0 && tries < kLastReservedSlot) {
            (*(volatile int*)&AsWGeneric3D(self)->nextReservedRenderSlot) = index >= kLastReservedSlot ? 1 : index + 1;
            ++tries;
        }
        ((ShiftedRenderSlotView*)slot)->state = kReservedSlot;
        return (*(volatile int*)&AsWGeneric3D(self)->nextReservedRenderSlot);
    } else {
        tries = 0;
        while (occupiedSlot(self, 0x2424, index, slot) != 0 && tries < kGeneralSlotAttempts) {
            (*(volatile int*)&AsWGeneric3D(self)->nextRenderSlot) = index >= kLastGeneralSlot ? kFirstGeneralSlot : index + 1;
            ++tries;
        }
        ((ShiftedRenderSlotView*)slot)->state = kReservedSlot;
        return (*(volatile int*)&AsWGeneric3D(self)->nextRenderSlot);
    }
}

// Reserve a texture slot and initialize it from a resource handle or descriptor.
// args uses RenderSlotArguments; reserve-only leaves kReservedSlot. Return the
// chosen index. Backend hooks have no failure result here; successful creation
// is assumed before marking initialized. Range exhaustion follows AllocateRenderSlot.
int WGeneric3D_CreateTexture(void* self, int* args, int flags) {
    // Allocation marks the slot reserved; reserve-only deliberately stops there.
    int index = WGeneric3D_AllocateRenderSlot(self, (flags >> 29) & 1);
    if (flags & kReserveTextureSlotOnly)
        return index;

    if (args[0] != 0) {
        BindTextureSlotResource(self, args[0], index);
    } else {
        char* object = (char*)args[2];
        int count = (flags & kTextureCreationArg7Is24) ? 0x18 : 0x10;
        int mode = (((TextureSlotDescriptor*)object)->format < 0x20) ? ((flags >> 11) & 1) : 2;
        int firstArg = ((TextureSlotDescriptor*)object)->width;
        int secondArg = ((TextureSlotDescriptor*)object)->height;
        register int filter = (flags >> 13) & 1;
        int type = (flags & kTextureType3) ? 3 : 1;
        CreateTextureSlotStorage(self, firstArg, secondArg, type, filter, index, mode, count);
    }
    (*(int*)&AsWGeneric3D(self)->items[index]) = kInitializedSlot;
    return index;
}

#pragma push
#pragma use_lmw_stmw on
// Initialize a reserved slot if needed, then update its resource data.
// Resource-handle input binds only while reserved. Descriptor input always
// updates data, creating default storage first when reserved. index must name
// a valid slot; this routine does not validate it or release input ownership.
void WGeneric3D_UpdateTexture(void* self, int index, int* args, int flags) {
    if (args[0] != 0) {
        int* state = (int*)&AsWGeneric3D(self)->items[index];
        if (*state == kReservedSlot) {
            BindTextureSlotResource(self, args[0], index);
            *state = kInitializedSlot;
        }
    } else {
        int* state = (int*)&AsWGeneric3D(self)->items[index];
        void* object = ((RenderSlotArguments*)args)->descriptor;
        void* data = ((RenderSlotArguments*)args)->data;
        if (*state == kReservedSlot) {
            CreateTextureSlotStorage(self, ((TextureSlotDescriptor*)object)->width,
                                           ((TextureSlotDescriptor*)object)->height, 1, 0,
                                           index, 0, 0x10);
            *state = kInitializedSlot;
        }
        UpdateTextureSlotData(self, index, object, data, flags);
    }
}

#pragma pop

// Mark a render slot free.
void WGeneric3D_DestroyTexture(void* base, int index) {
    (*(int*)&AsWGeneric3D(base)->items[index]) = kFreeSlot;
}

// Return the renderer virtual hook default failure value.
int WDevice_ExternProc() {
    return 0;
}

// Return the renderer shared default-data pointer; its contents are not established here.
void* WDevice_EnumModeName() {
    return s_layoutZeroWordPair;
}

// Pack vertex pointers into the selected position/color/UV layout and optionally commit the byte cursor.
int WGeneric3D_PackVertexPointerArray(void* self, int* sourceVertices, int vertexCount, int vertexFormat, int commitCursor) {
    // Borrow vertexCount source records. Return a renderer-owned staging address;
    // commitCursor retains those bytes for queued draws, otherwise they may be reused.
    // Formats 1/5 share the copies here but remain distinct backend position modes.
    // Unsupported formats reserve no bytes and still return the current staging address.
    // Phase 1: make room, flushing queued work at the staging threshold.
    int writeOffset = (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed);
    if (writeOffset >= kVertexBufferFlushThreshold) {
        ((WGeneric3D*)self)->Flush(0);
        writeOffset = 0;
    }
    char* packedVertices = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;

    // Phase 2: copy XYZ, optional RGBA and one/two UV pairs in the selected layout.
    switch (vertexFormat) {
    case (int)(kPackedColorMask | kPositionPacking5):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorBytes;
            COPY_ARGB_TO_RGBA(dst, src);
        }
        break;
    case (int)(kFogPackingBit | kPackedColorMask | kPositionPacking5):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorBytes;
            COPY_ARGB_TO_RGBA(dst, src);
        }
        break;
    case (int)(kSingleTexturePacking | kPackedColorMask | kPositionPacking5):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorTextureBytes;
            COPY_ARGB_TO_RGBA(dst, src);
            ((PackedVertex*)dst)->u0 = ((GenericVertexSource*)src)->u0;
            ((PackedVertex*)dst)->v0 = ((GenericVertexSource*)src)->v0;
        }
        break;
    case (int)(kFogPackingBit | kSingleTexturePacking | kPackedColorMask | kPositionPacking5):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorTextureBytes;
            COPY_ARGB_TO_RGBA(dst, src);
            ((PackedVertex*)dst)->u0 = ((GenericVertexSource*)src)->u0;
            ((PackedVertex*)dst)->v0 = ((GenericVertexSource*)src)->v0;
        }
        break;
    case (int)(kSingleTexturePacking | kPositionPacking5):
        for (int i = 0; i < vertexCount; i++) {
            float* src = (float*)sourceVertices[i];
            float* dst = (float*)((*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset);
            dst[kOutputX] = src[kSourceX];
            dst[kOutputY] = src[kSourceY];
            dst[kOutputZ] = src[kSourceZ];
            dst[kOutputU] = src[kSourceU];
            dst[kOutputV] = src[kSourceV];
            writeOffset += kPositionTextureBytes;
        }
        break;
    case (int)(kFogPackingBit | kSingleTexturePacking | kPositionPacking5):
        for (int i = 0; i < vertexCount; i++) {
            float* src = (float*)sourceVertices[i];
            float* dst = (float*)((*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset);
            dst[kOutputX] = src[kSourceX];
            dst[kOutputY] = src[kSourceY];
            dst[kOutputZ] = src[kSourceZ];
            dst[kOutputU] = src[kSourceU];
            dst[kOutputV] = src[kSourceV];
            writeOffset += kPositionTextureBytes;
        }
        break;
    case (int)(kTwoTexturePacking | kPackedColorMask | kPositionPacking5):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorTwoTextureBytes;
            ((PackedVertex*)dst)->u0 = ((GenericVertexSource*)src)->u0;
            ((PackedVertex*)dst)->v0 = ((GenericVertexSource*)src)->v0;
            ((PackedVertex*)dst)->u1 = ((GenericVertexSource*)src)->u1;
            ((PackedVertex*)dst)->v1 = ((GenericVertexSource*)src)->v1;
            COPY_ARGB_TO_RGBA(dst, src);
        }
        break;
    case (int)(kFogPackingBit | kTwoTexturePacking | kPackedColorMask | kPositionPacking5):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorTwoTextureBytes;
            COPY_ARGB_TO_RGBA(dst, src);
            ((PackedVertex*)dst)->u0 = ((GenericVertexSource*)src)->u0;
            ((PackedVertex*)dst)->v0 = ((GenericVertexSource*)src)->v0;
            ((PackedVertex*)dst)->u1 = ((GenericVertexSource*)src)->u1;
            ((PackedVertex*)dst)->v1 = ((GenericVertexSource*)src)->v1;
        }
        break;
    case (int)(kPackedColorMask | kPositionPacking1):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorBytes;
            COPY_ARGB_TO_RGBA(dst, src);
        }
        break;
    case (int)(kFogPackingBit | kPackedColorMask | kPositionPacking1):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorBytes;
            COPY_ARGB_TO_RGBA(dst, src);
        }
        break;
    case (int)(kSingleTexturePacking | kPackedColorMask | kPositionPacking1):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorTextureBytes;
            COPY_ARGB_TO_RGBA(dst, src);
            ((PackedVertex*)dst)->u0 = ((GenericVertexSource*)src)->u0;
            ((PackedVertex*)dst)->v0 = ((GenericVertexSource*)src)->v0;
        }
        break;
    case (int)(kFogPackingBit | kSingleTexturePacking | kPackedColorMask | kPositionPacking1):
        for (int i = 0; i < vertexCount; i++) {
            char* src = (char*)sourceVertices[i];
            char* dst = (*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset;
            copyVertexPosition(dst, src);
            writeOffset += kPositionColorTextureBytes;
            COPY_ARGB_TO_RGBA(dst, src);
            ((PackedVertex*)dst)->u0 = ((GenericVertexSource*)src)->u0;
            ((PackedVertex*)dst)->v0 = ((GenericVertexSource*)src)->v0;
        }
        break;
    case (int)(kSingleTexturePacking | kPositionPacking1):
        for (int i = 0; i < vertexCount; i++) {
            float* src = (float*)sourceVertices[i];
            float* dst = (float*)((*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset);
            dst[kOutputX] = src[kSourceX];
            dst[kOutputY] = src[kSourceY];
            dst[kOutputZ] = src[kSourceZ];
            dst[kOutputU] = src[kSourceU];
            dst[kOutputV] = src[kSourceV];
            writeOffset += kPositionTextureBytes;
        }
        break;
    case (int)(kFogPackingBit | kSingleTexturePacking | kPositionPacking1):
        for (int i = 0; i < vertexCount; i++) {
            float* src = (float*)sourceVertices[i];
            float* dst = (float*)((*(char**)&AsWGeneric3D(self)->vertexBuffer) + writeOffset);
            dst[kOutputX] = src[kSourceX];
            dst[kOutputY] = src[kSourceY];
            dst[kOutputZ] = src[kSourceZ];
            dst[kOutputU] = src[kSourceU];
            dst[kOutputV] = src[kSourceV];
            writeOffset += kPositionTextureBytes;
        }
        break;
    case 0:
    case (int)(kFogPackingBit):
    case 0x100:
    case (int)(kFogPackingBit | kTwoTexturePacking | kPositionPacking5):
    case (int)(kFogPackingBit | kTwoTexturePacking | kPositionPacking1):
    case (int)(kTwoTexturePacking | kPositionPacking5):
    case (int)(kTwoTexturePacking | kPositionPacking1):
    default:
        break;
    }

    // Phase 3: publish the cursor only when the caller retains this packed data.
    if (commitCursor)
        (*(int*)&AsWGeneric3D(self)->vertexBufferBytesUsed) = writeOffset;
    return (int)packedVertices;
}

// Defined after its callers to preserve their out-of-line calls under mwcc auto-inlining.
// Return the first input vertex depth as the primitive sort key.
float Generic3D_GetFirstVertexSortDepth(void* self, void* object, int n) {
    return (*(TaggedPrimitiveRecord**)object)->centerZ;
}


// Guard the local views used by this unit against accidental layout changes.
#define POLISH_OFFSET(T, F, O) typedef char polish_##T##_##F[((int)DECOMP_OFFSETOF(T, F) == (O)) ? 1 : -1]
POLISH_OFFSET(TaggedPrimitiveRecord, argb, 0x10);
POLISH_OFFSET(TaggedPrimitiveRecord, tag, 0x34);
POLISH_OFFSET(TransformedQuadPayload, axisY, 0x38);
POLISH_OFFSET(TransformedQuadPayload, translation, 0x50);
POLISH_OFFSET(TransformedQuadPayload, axisZ, 0x60);
POLISH_OFFSET(TransformedQuadPayload, u0, 0x88);
POLISH_OFFSET(BillboardCameraAxes, right, 0x44);
POLISH_OFFSET(CameraRenderMatrices, view, 0x60);
POLISH_OFFSET(TextureSlotDescriptor, format, 0xe);
#undef POLISH_OFFSET
typedef char polish_size_TaggedPrimitiveRecord[(sizeof(TaggedPrimitiveRecord) == 0x38) ? 1 : -1];
typedef char polish_size_GenericVertexSource[(sizeof(GenericVertexSource) == 0x38) ? 1 : -1];
typedef char polish_size_PackedVertex[(sizeof(PackedVertex) == 0x20) ? 1 : -1];
typedef char polish_size_Vertex83[(sizeof(QuadBatchVertex) == 0x18) ? 1 : -1];

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462034", _ctors_u8008199C_0, Generic3D_InitRotationPeriod);
#pragma pop
