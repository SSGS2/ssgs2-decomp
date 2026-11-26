// B2D sprite and texture drawing: prompt-sheet textures, the Tour board layout, and the
// default draw callbacks that B2D scripts call.
// Also the Wii drawing and texture hooks, render state setup, colored quads and group lookups.

#include <decomp/static_init.h>
// B2D script sprite / prompt-sheet textures and Tour board layout drawing.
// Mod lookup:
// - Visible layout bounds: kFloat_640_SpriteClip and kFloat_480_Sprite (640x480)
//   in the data section, read by SubmitScaledTextureRegion/SubmitSolidQuad.
//   These are coarse rejection edges, NOT framebuffer resolution or GX scissor.
//   Wider layouts must coordinate these edges with script positions and the
//   UI projection/viewport; arbitrary-corner callbacks do not test these edges.
// - Sprite scaling: kSpriteFixedScale = 1024 represents 1.0 for each axis.
// - Prompt-sheet rectangle: SubmitScaledTextureRegion sourceX/Y/Width/Height are
//   texture texels; destination x/y use layout coordinates, +X right, +Y down.
//   SubmitTexturedQuad instead receives four source UV pairs in TEXELS.
// - Resource choice: BpScript_CreateNamedGsrTexture concatenates script path,
//   resourceName and s_bpTextureResourceSuffix; CBpTex's setter loads the result.
//   Decode/format selection is outside this unit: sheet-size edits must keep
//   descriptor dimensions, source rectangles and resource loading in sync.
// - Color/opacity: textured callbacks expand script RGB; all expand opacity via
//   min(255,2*n-1), with zero special-cased. Solid RGB stays unscaled. Script 128
//   produces 255 alpha. There is no lower clamp or general range validation.
// Render callbacks require g_currentBpCallbackObject installed by CBpObject's
// dispatcher, a valid sprite pool and valid texture ID. They enqueue copied
// vertices, retain no caller buffers, and always return 0 (also on rejection).
// Resource objects become owned by the Bp object on replacement. Preserve float
// evaluation, vertex order, and temporary string lifetime. No frame rate knob.
// B2D sprite flags passed by script drawing callbacks.
enum BpSpriteFlags {
    kSpriteForceMode = 0x10,
    kSpritePreserveMode = 0x20,
    kSpriteFlipU = 0x1000,
    kSpriteFlipV = 0x2000,
    kSpriteChainInvalid = 0x100,
    kSpriteItemDisabled = 1
};
// Fixed-point denominator, not display width; a larger denominator shrinks quads.
static const float kSpriteFixedScale = 1024.0f;
// Only the per-quad render-mode word is accessed here.
struct SpriteRenderModeView {
    char reserved[0x70];
    int renderMode;
};

#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u800C0C18 {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    // Start an empty inline string; the high flag bit denotes owned heap storage.
    LocalNString_u800C0C18() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    // Copy a required NUL-terminated path into this temporary string.
    LocalNString_u800C0C18(const char* text) {
        MWString_ConstructCString(this, text);
    }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    // Copy a required NUL-terminated path through the range-string ABI.
    LocalNString_u800C0C18(const char* text) {
        LocalNString_u800C0C18* object = this;
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
    // Release owned heap storage; inline bytes need no free.
    ~LocalNString_u800C0C18() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    // Read the byte length from the inline tag or heap length word.
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    // Borrow NUL-terminated bytes until mutation or destruction.
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

#include "classes/CBpObject.h"
#include "classes/CBpSprite.h"
#include "classes/CBpTex.h"
// Borrow the callback receiver's ABI layout; no null check or ownership change.
static inline CBpObject* AsCBpObject(void* p) {
    return (CBpObject*)p;
}

struct RenderTableA {
    unsigned int data[12];
};
struct RenderTableB {
    unsigned int data[10];
};
struct RenderTableC {
    unsigned int data[26];
};
struct RenderTables {
    RenderTableA a;
    RenderTableB b;
    RenderTableC c;
};

extern "C" {

extern float kFloat_6_2831855;
extern float s_b2dSpriteTwoPi;
extern float s_twoPi_800C0C18_805E28E0;
extern const char b2dSpriteSourceFileName[];
// Diagnostic function name "B2dSprGetGroupObj" for a missing sprite group.
extern const char b2dSpriteGetGroupObjectFunctionName[];
extern const char bpErrorDiagnosticStrings[];
extern const RenderTables uiDefaultRenderStateStorage;
extern const unsigned char kIdentityMatrix4x4[];
extern void* g_currentBpCallbackObject;
extern const char s_bpTextureResourceSuffix[];

int BpCtrl_InvokeGlobalHook(int, const char*, int, const char*);
int BpSlot_FindItemById(int, int);
void BpCtrl_NoopVariadicCallback(const char*, ...);
void BpCtrl_SetDefaultRectDrawCallback(void (*)());
void BpCtrl_SetSolidQuadCallback(void (*)());
void BpCtrl_SetTexturedQuadCallback(void (*)());
void BpCtrl_SetColoredQuadCallback(void (*)());
void BpCtrl_SetSpriteQuadDrawCallback(void (*)());
void BpCtrl_SetDefaultTransformedQuadCallback(void (*)());
void B2d_SetInputCallback(void (*)());
void BpCtrl_SetDrawObjectCallback(void (*)());
void B2d_SetTextureSubcommandCallback(void (*)());
extern "C" void __ct__6CBpTexFv(void*);
void* BpSlot_GetSpriteListHead(int index);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_SetType7Resource(int);
void SetTransform(int, const void*);
void* BpSpritePool_Acquire(void*);
unsigned int strlen(const char*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
void appFreeUni__FPv(void*);
extern "C" void __ct__6CBpTexFv(void*);

int BpSprite_SubmitScaledTextureRegion(void*, int, int, int, int, int, int, int, int, int, unsigned int, unsigned int);
int BpSprite_SubmitSolidQuad(unsigned char*, int, int, int, int, int, int, int, unsigned int);
int BpSprite_SubmitTexturedQuad(void* context, int* rect, const int* source, int enabled, unsigned int color,
                                unsigned int mode);
int BpSprite_DrawColoredQuad(unsigned char*, int*, int, unsigned int);
int B2d_DefaultInputCallback();
int B2d_DefaultDrawObjectCallback();
int B2d_DefaultSpriteQuadCallback();
int B2d_DefaultTransformedQuadCallback();
void BpTextureCallback_InitTwoPiCache();
int CBpObject_CreateAndReplaceNamedTexture(void*, void*, void*);

// Install the Wii drawing, texture-loading and input hooks used by B2D scripts.
void BpCtrl_InstallPlatformCallbacks() {
    BpCtrl_SetDefaultRectDrawCallback((void (*)())BpSprite_SubmitScaledTextureRegion);
    BpCtrl_SetSolidQuadCallback((void (*)())BpSprite_SubmitSolidQuad);
    BpCtrl_SetTexturedQuadCallback((void (*)())BpSprite_SubmitTexturedQuad);
    BpCtrl_SetColoredQuadCallback((void (*)())BpSprite_DrawColoredQuad);
    BpCtrl_SetSpriteQuadDrawCallback((void (*)())B2d_DefaultSpriteQuadCallback);
    BpCtrl_SetDefaultTransformedQuadCallback((void (*)())B2d_DefaultTransformedQuadCallback);
    B2d_SetInputCallback((void (*)())B2d_DefaultInputCallback);
    B2d_SetTextureSubcommandCallback((void (*)())CBpObject_CreateAndReplaceNamedTexture);
    BpCtrl_SetDrawObjectCallback((void (*)())B2d_DefaultDrawObjectCallback);
}

#pragma use_lmw_stmw on
// Emit the four-argument B2D diagnostic through the optional logging callback.
void B2d_LogFourArgumentDiagnostic(int a, int b, int c, int d) {
    BpCtrl_NoopVariadicCallback(bpErrorDiagnosticStrings);
    BpCtrl_NoopVariadicCallback(bpErrorDiagnosticStrings + 2);
    BpCtrl_NoopVariadicCallback(bpErrorDiagnosticStrings + 0x25, a, b, c, d);
    BpCtrl_NoopVariadicCallback(bpErrorDiagnosticStrings + 2);
}
#pragma use_lmw_stmw reset

struct EventObj {
    void* vtable;
    char padding[0x24];
    // Initialize a CBpTex instance; replacement transfers it to the Bp owner.
    EventObj() {
        ((CBpTex*)this)->__ct();
    }
};
struct EventVt {
    virtual void slot0();
    virtual void setArg(void*);
};

#pragma opt_propagation off
// Create a texture from the supplied resource and replace the script object with this name.
// owner/key/arg required; key selects the script object and arg supplies a resource.
// Slot 1 is a texture setter, as in WidgetVt below (setText is an ABI placeholder).
// New texture ownership transfers to owner. Returns 0, not a success indicator.
int CBpObject_CreateAndReplaceNamedTexture(void* owner, void* key, void* arg) {
    EventObj* object;
    void* savedOwner;
    void* savedArg;
    void* savedKey;
    savedOwner = owner;
    savedArg = arg;
    savedKey = key;
    object = new EventObj;
    ((EventVt*)object)->setArg(savedArg);
    ((CBpObject*)savedOwner)->ReplaceObjectById((int)savedKey, (int)object);
    return 0;
}
#pragma opt_propagation reset

struct B2dGroupStateView {
    char reservedPrefix[0x14];
    int active;
};

struct B2dSpriteGroupView {
    char reservedPrefix[0x94];
    int parentGroupId;
};

// Report whether a non-null group has its active word set.
int B2dGroup_IsActive(B2dGroupStateView* obj) {
    if (obj == 0) {
        return 0;
    }
    int value = obj->active;
    return (unsigned int)(-value | value) >> 31;
}

// Apply the script override/preserve flags to a sprite render-mode word.
// Null obj returns 0 without a write. Otherwise returns 1; force takes precedence,
// preserve leaves the old word alone, neither flag clears it. No resource transfer.
int BpSprite_ApplyRenderModeFlags(unsigned int flags, void* obj) {
    if (obj == 0) {
        return 0;
    }
    int* field = &((SpriteRenderModeView*)obj)->renderMode;
    if ((flags & kSpriteForceMode) != 0) {
        *field = 1;
    } else if ((flags & kSpritePreserveMode) == 0) {
        *field = 0;
    }
    return 1;
}

// Resolve the sprite parent-group ID through its owning B2D slot.
// arg is the slot index; result is a borrowed group handle or 0 for no parent.
// Null obj emits the diagnostic then returns 0; parent -1 returns 0 silently.
int B2dSprite_GetParentGroupObject(int arg, B2dSpriteGroupView* obj) {
    if (obj == 0) {
        BpCtrl_InvokeGlobalHook(-1, b2dSpriteSourceFileName, 0x14C, b2dSpriteGetGroupObjectFunctionName);
        return 0;
    }
    int index = obj->parentGroupId;
    if (index == -1) {
        return 0;
    }
    return BpSlot_FindItemById(arg, index);
}

// Default input hook: report no input.
int B2d_DefaultInputCallback() {
    return 0;
}

// Default draw-object hook: no work is required.
int B2d_DefaultDrawObjectCallback() {
    return 0;
}

// Default sprite-quad hook: no work is required.
int B2d_DefaultSpriteQuadCallback() {
    return 0;
}

// Default transformed-quad hook: no work is required.
int B2d_DefaultTransformedQuadCallback() {
    return 0;
}

// Initialize this module's cached full-turn angle.
void B2dSprite_InitTwoPiCache() {
    s_b2dSpriteTwoPi = kFloat_6_2831855;
}

// Initialize the texture-callback module's cached full-turn angle.
void BpTextureCallback_InitTwoPiCache() {
    s_twoPi_800C0C18_805E28E0 = kFloat_6_2831855;
}

struct B2dCommand {
    int reservedWord, flags;
    int value;
    B2dCommand* next;
};
struct B2dItem {
    char reservedPrefix[0x0A];
    short disabledFlags;
    char reservedBeforeChain[4];
    B2dCommand* commands;
    char pad[4];
};
struct B2dItemPage {
    char reservedPrefix[0x14];
    B2dItemPage* next;
    B2dItem items[18];
};
// Choose the enabled item with the greatest POSITIVE command total across pages.
// Returns a borrowed item or null (empty list, all disabled/invalid, or all totals
// <= 0). Equal totals keep the first item. Pages have 18 items, each with a nullable
// command chain; invalid bit 0x100 rejects the whole chain, not just that command.
void* B2d_SelectHighestCommandTotalItem(int index) {
    int best = 0;
    B2dItemPage* head = (B2dItemPage*)BpSlot_GetSpriteListHead(index);
    B2dItem* item;
    void* bestItem = 0;
    if (!head)
        return 0;
    while (head) {
        item = head->items;
        for (int i = 0; i < 18; ++item, ++i) {
            B2dCommand* node;
            int total = 0;
            node = item->commands;
            if ((item->disabledFlags & kSpriteItemDisabled) == 0) {
                while (node) {
                    total += node->value;
                    if (node->flags & kSpriteChainInvalid) {
                        total = -1;
                        break;
                    }
                    node = node->next;
                }
                if (total >= 0 && best < total) {
                    best = total;
                    bestItem = item;
                }
            }
        }
        head = head->next;
    }
    return bestItem;
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Apply B2D render/texture presets and identity WORLD transform (engine state 2).
// Lists contain 13/6/5 key-value pairs, not that many words. They replace selected
// global render states; this does not save or restore the caller's state.
void UIRender_SetDefaultState() {
    RenderTableA textureStageStates;
    RenderTableB textureStageParameters = uiDefaultRenderStateStorage.b;
    textureStageStates = uiDefaultRenderStateStorage.a;
    RenderTableC renderStates = uiDefaultRenderStateStorage.c;
    Engine_SetRenderStateList(&renderStates, 13);
    Engine_SetTextureStageStateList(0, &textureStageStates, 6);
    Engine_SetTextureStageParameterList(0, &textureStageParameters, 5);
    Engine_SetType7Resource(0);
    SetTransform(2, kIdentityMatrix4x4);
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

extern float kFloat_0_Sprite;
// Vertex ABI: 24 bytes: layout XYZ +0, RGBA bytes +12, normalized UV +16.
// Workspace r = {uLeft,vTop,uRight,vBottom}; dimensions = uint32 width,height.
struct SpriteVertex {
    float x, y, z;
    unsigned char c[4];
    float u, v;
};
struct TexturedQuadWorkspace {
    float r[4];
    SpriteVertex vertex;
    unsigned int dimensions[2];
    SpriteVertex spare;
};
extern double kDouble_SignedIntConversionBias_Sprite;
extern double kDouble_SignedIntConversionBias_Sprite;
extern float kFloat_640_SpriteClip;
extern float kFloat_480_Sprite;
extern float kFloat_1_TextureQuad;
extern double kSpriteUnsignedIntConversionBias;
extern void BpSprite_GetTextureDescriptor(void*, void*);
extern double kDouble_SignedIntConversionBias_Sprite;
extern double kSpriteUnsignedIntConversionBias;
extern float kFloat_0_Sprite;
// Reject an off-layout scaled texture region, normalize UVs, and submit its quad.
// context is a borrowed texture ID; source dimensions and scale must not overflow
// signed-int products. Texture descriptor dimensions must be nonzero. Opacity 0
// returns before any receiver/resource access. The test rejects whole quads only:
// partly offscreen geometry and UVs are NOT clipped. Preserve this distinction.
int BpSprite_SubmitScaledTextureRegion(void* context, int sourceX, int sourceY, int sourceWidth, int sourceHeight,
                                       int x, int y, int scaleX, int scaleY, int opacity, unsigned int color,
                                       unsigned int flags) {
    if (!opacity)
        return 0;
    unsigned int colorCopy;
    float left = (float)x;
    float top = (float)y;
    float right = (float)(sourceWidth * scaleX) / kSpriteFixedScale + left;
    float bottom = (float)(sourceHeight * scaleY) / kSpriteFixedScale + top;
    TexturedQuadWorkspace w;
    int visible;
    if (left > kFloat_640_SpriteClip)
        visible = 0;
    else if (top > kFloat_480_Sprite)
        visible = 0;
    else if (right < kFloat_0_Sprite)
        visible = 0;
    else
        visible = !(bottom < kFloat_0_Sprite);
    if (visible == 0)
        return 0;

    // Resolve the borrowed texture and pooled sprite after coarse visibility.
    void* manager = g_currentBpCallbackObject;
    void* object = (void*)BpSpritePool_Acquire(
        AsCBpObject(manager)->objectArray); // +0x24; BpObject_TryInvokeCurrentCallback sets g_currentBpCallbackObject
    void* item = ((CBpObject*)manager)->FindTextureById((int)context);
    ((CBpSprite*)object)->BeginVertices((int)item);
    BpSprite_GetTextureDescriptor(item, w.dimensions);

    float textureWidth = (float)w.dimensions[0];
    float textureHeight = (float)w.dimensions[1];
    float sourceLeft = (float)sourceX;
    float sourceTop = (float)sourceY;
    float sourceSpanX = (float)sourceWidth;
    float sourceSpanY = (float)sourceHeight;
    w.r[0] = sourceLeft / textureWidth;
    w.r[1] = sourceTop / textureHeight;
    w.r[2] = (sourceLeft + sourceSpanX) / textureWidth;
    w.r[3] = (sourceTop + sourceSpanY) / textureHeight;
    if (flags & kSpriteFlipU) {
        float t = w.r[0];
        w.r[0] = w.r[2];
        w.r[2] = t;
    }
    if (flags & kSpriteFlipV) {
        float t = w.r[1];
        w.r[1] = w.r[3];
        w.r[3] = t;
    }
    if (object) {
        if (flags & kSpriteForceMode)
            ((SpriteRenderModeView*)object)->renderMode = 1;
        else if (!(flags & kSpritePreserveMode))
            ((SpriteRenderModeView*)object)->renderMode = 0;
    }

    // Target big-endian script word bytes are consumed [3],[2],[1] as RGB.
    // color's byte [0] is ignored; opacity supplies alpha independently.
    colorCopy = color;
    // The colour is a numeric word whose big-endian bytes [0..3] are {ignored, B, G, R}.
    unsigned char* cb = (unsigned char*)&colorCopy;
    int t;
    t = cb[3] == 0 ? 0 : cb[3] * 2 - 1;
    if (t > 255)
        t = 255;
    w.vertex.c[0] = t;
    t = cb[2] == 0 ? 0 : cb[2] * 2 - 1;
    if (t > 255)
        t = 255;
    w.vertex.c[1] = t;
    t = cb[1] == 0 ? 0 : cb[1] * 2 - 1;
    if (t > 255)
        t = 255;
    w.vertex.c[2] = t;
    t = opacity == 0 ? 0 : opacity * 2 - 1;
    if (t > 255)
        t = 255;
    w.vertex.c[3] = t;

    w.vertex.u = w.r[0];
    w.vertex.v = w.r[1];
    w.vertex.x = left;
    w.vertex.y = top;
    w.vertex.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&w.vertex);
    w.vertex.u = w.r[2];
    w.vertex.v = w.r[1];
    w.vertex.x = right;
    w.vertex.y = top;
    w.vertex.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&w.vertex);
    w.vertex.u = w.r[0];
    w.vertex.v = w.r[3];
    w.vertex.x = left;
    w.vertex.y = bottom;
    w.vertex.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&w.vertex);
    w.vertex.u = w.r[2];
    w.vertex.v = w.r[3];
    w.vertex.x = right;
    w.vertex.y = bottom;
    w.vertex.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&w.vertex);
    ((CBpSprite*)object)->ActivateAndClearVertexCount();
    return 0;
}

struct Vtx1510 {
    float x, y, z;
    unsigned char c[4];
    float u, v;
};
struct Work1510 {
    unsigned int color;
    unsigned int pad;
    Vtx1510 vertex;
    float ratio[8];
    unsigned int dimensions[2];
    Vtx1510 spare;
};
// Submit a script quadrilateral with texel source pairs normalized to UVs.
// rect/source required, each int[8] of four XY/UV pairs; they are not LTRB boxes.
// No layout rejection. mode handles force/preserve ONLY: U/V flip flags are not
// read here. Input corner order is emitted 0,3,1,2; keep paired UVs in that order.
int BpSprite_SubmitTexturedQuad(void* context, int* rect, const int* source, int opacity, unsigned int color,
                                unsigned int mode) {
    if (!opacity)
        return 0;
    void* manager = g_currentBpCallbackObject;
    void* object = BpSpritePool_Acquire(AsCBpObject(manager)->objectArray);
    void* item = ((CBpObject*)manager)->FindTextureById((int)context);
    ((CBpSprite*)object)->BeginVertices((int)item);

    Work1510 w;
    BpSprite_GetTextureDescriptor(item, w.dimensions);

    w.ratio[0] = (float)source[0] / (float)w.dimensions[0];
    w.ratio[1] = (float)source[1] / (float)w.dimensions[1];
    w.ratio[2] = (float)source[2] / (float)w.dimensions[0];
    w.ratio[3] = (float)source[3] / (float)w.dimensions[1];
    w.ratio[4] = (float)source[4] / (float)w.dimensions[0];
    w.ratio[5] = (float)source[5] / (float)w.dimensions[1];
    w.ratio[6] = (float)source[6] / (float)w.dimensions[0];
    w.ratio[7] = (float)source[7] / (float)w.dimensions[1];

    w.color = color;
    unsigned char* cb = (unsigned char*)&w.color;
    int t;
    t = cb[3] == 0 ? 0 : cb[3] * 2 - 1;
    if (t > 255)
        t = 255;
    w.vertex.c[0] = t;
    t = cb[2] == 0 ? 0 : cb[2] * 2 - 1;
    if (t > 255)
        t = 255;
    w.vertex.c[1] = t;
    t = cb[1] == 0 ? 0 : cb[1] * 2 - 1;
    if (t > 255)
        t = 255;
    w.vertex.c[2] = t;
    t = opacity == 0 ? 0 : opacity * 2 - 1;
    if (t > 255)
        t = 255;
    w.vertex.c[3] = t;

    if (object) {
        if (mode & kSpriteForceMode)
            ((SpriteRenderModeView*)object)->renderMode = 1;
        else if (!(mode & kSpritePreserveMode))
            ((SpriteRenderModeView*)object)->renderMode = 0;
    }

    w.vertex.u = w.ratio[0];
    w.vertex.v = w.ratio[1];
    w.vertex.x = (float)rect[0];
    w.vertex.y = (float)rect[1];
    w.vertex.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&w.vertex);

    w.vertex.u = w.ratio[6];
    w.vertex.v = w.ratio[7];
    w.vertex.x = (float)rect[6];
    w.vertex.y = (float)rect[7];
    w.vertex.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&w.vertex);

    w.vertex.u = w.ratio[2];
    w.vertex.v = w.ratio[3];
    w.vertex.x = (float)rect[2];
    w.vertex.y = (float)rect[3];
    w.vertex.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&w.vertex);

    w.vertex.u = w.ratio[4];
    w.vertex.v = w.ratio[5];
    w.vertex.x = (float)rect[4];
    w.vertex.y = (float)rect[5];
    w.vertex.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&w.vertex);

    ((CBpSprite*)object)->ActivateAndClearVertexCount();
    return 0;
}
#define BP_SCRIPT_COLOR_BYTE(color, i) ((color)[i])
struct SolidSpriteVertex {
    float x, y, z;
    unsigned char color[4];
    float u, v;
};
// Reject a fully off-layout solid rectangle and submit it with script alpha.
// color is a required borrowed 4-byte script word, bytes 3/2/1 become RGB without
// expansion. width/height and scale share SubmitScaledTextureRegion's fixed-point
// contract. No texture is acquired; emitted UVs still cover [0,1].
int BpSprite_SubmitSolidQuad(unsigned char* color, int width, int height, int x, int y, int scaleX, int scaleY,
                             int opacity, unsigned int flags) {
    if (!opacity)
        return 0;

    float left = (float)x;
    float top = (float)y;
    float right = (float)((width * scaleX)) / kSpriteFixedScale + left;
    float bottom = (float)((height * scaleY)) / kSpriteFixedScale + top;
    int visible;
    if (left > kFloat_640_SpriteClip)
        visible = 0;
    else if (top > kFloat_480_Sprite)
        visible = 0;
    else if (right < kFloat_0_Sprite)
        visible = 0;
    else
        visible = !(bottom < kFloat_0_Sprite);
    if (!visible)
        return 0;

    void* manager = g_currentBpCallbackObject;
    void* object = BpSpritePool_Acquire(AsCBpObject(manager)->objectArray);
    if (object) {
        if (flags & kSpriteForceMode)
            ((SpriteRenderModeView*)object)->renderMode = 1;
        else if (!(flags & kSpritePreserveMode))
            ((SpriteRenderModeView*)object)->renderMode = 0;
    }
    ((CBpSprite*)object)->BeginVertices(0);

    SolidSpriteVertex vtx;
    vtx.color[0] = BP_SCRIPT_COLOR_BYTE(color, 3);
    vtx.color[1] = BP_SCRIPT_COLOR_BYTE(color, 2);
    int alpha = !opacity ? 0 : opacity * 2 - 1;
    vtx.color[2] = BP_SCRIPT_COLOR_BYTE(color, 1);
    if (alpha > 255)
        alpha = 255;
    vtx.u = kFloat_0_Sprite;
    vtx.color[3] = (unsigned char)alpha;
    vtx.v = kFloat_0_Sprite;
    vtx.x = left;
    vtx.y = top;
    vtx.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&vtx);

    vtx.u = kFloat_1_TextureQuad;
    vtx.v = kFloat_0_Sprite;
    vtx.x = right;
    vtx.y = top;
    vtx.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&vtx);

    vtx.u = kFloat_0_Sprite;
    vtx.v = kFloat_1_TextureQuad;
    vtx.x = left;
    vtx.y = bottom;
    vtx.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&vtx);

    vtx.u = kFloat_1_TextureQuad;
    vtx.v = kFloat_1_TextureQuad;

    vtx.x = right;
    vtx.y = bottom;
    vtx.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&vtx);

    ((CBpSprite*)object)->ActivateAndClearVertexCount();
    return 0;
}
// Submit the four script rectangle corners as an untextured colored quad.
// color required (same byte order as solid rectangle); rects required int[8],
// four XY pairs emitted 0,3,1,2. No clipping; all UVs are zero, opacity is expanded.
int BpSprite_DrawColoredQuad(unsigned char* color, int* rects, int opacity, unsigned int flags) {
    if (!opacity)
        return 0;
    void* manager = g_currentBpCallbackObject;
    void* object = BpSpritePool_Acquire(AsCBpObject(manager)->objectArray);
    ((CBpSprite*)object)->BeginVertices(0);
    if (object) {
        if (flags & kSpriteForceMode)
            ((SpriteRenderModeView*)object)->renderMode = 1;
        else if (!(flags & kSpritePreserveMode))
            ((SpriteRenderModeView*)object)->renderMode = 0;
    }
    SolidSpriteVertex vtx;
    vtx.color[0] = BP_SCRIPT_COLOR_BYTE(color, 3);
    vtx.color[1] = BP_SCRIPT_COLOR_BYTE(color, 2);
    vtx.color[2] = BP_SCRIPT_COLOR_BYTE(color, 1);
    int alpha = opacity == 0 ? 0 : opacity * 2 - 1;
    if (alpha > 255)
        alpha = 255;
    vtx.color[3] = (unsigned char)alpha;

    vtx.u = kFloat_0_Sprite;
    vtx.v = kFloat_0_Sprite;
    vtx.x = (float)rects[0];
    vtx.y = (float)rects[1];
    vtx.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&vtx);

    vtx.u = kFloat_0_Sprite;
    vtx.v = kFloat_0_Sprite;
    vtx.x = (float)rects[6];
    vtx.y = (float)rects[7];
    vtx.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&vtx);

    vtx.u = kFloat_0_Sprite;
    vtx.v = kFloat_0_Sprite;
    vtx.x = (float)rects[2];
    vtx.y = (float)rects[3];
    vtx.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&vtx);

    vtx.u = kFloat_0_Sprite;
    vtx.v = kFloat_0_Sprite;
    vtx.x = (float)rects[4];
    vtx.y = (float)rects[5];
    vtx.z = kFloat_0_Sprite;
    ((CBpSprite*)object)->AppendVertex(&vtx);

    ((CBpSprite*)object)->ActivateAndClearVertexCount();
    return 0;
}

struct Widget {
    void* vtable;
    char pad[0x24];
    // Initialize a CBpTex resource object for transfer to its owning Bp object.
    Widget() {
        ((CBpTex*)this)->__ct();
    }
};
struct WidgetVt {
    virtual void slot0();
    virtual void setText(void* text);
};
typedef LocalNString_u800C0C18 SmallString;
union Byte4 {
    unsigned int w;
    unsigned char u[4];
    char s[4];
};
#pragma use_lmw_stmw on
// Build the script-relative texture path and replace its named script object.
// arg is unused ABI context. objectName/resourceName are required NUL-terminated
// strings. Current callback object supplies the path prefix and takes ownership
// of widget; local path storage dies on return, so slot 1 consumes it synchronously.
// Volatile scratch bytes passed to String_ReplaceRange are allocator-tag storage,
// NOT path separators; only prefix+resourceName+suffix form the string.
// Does not return load status (always 0); no null/allocation recovery here.
int BpScript_CreateNamedGsrTexture(void* arg, const char* objectName, const char* resourceName) {
    (void)arg;
    void* state = g_currentBpCallbackObject;
    Widget* widget = new Widget;
    const char* prefix;
    unsigned int textLength;
    // 0x800C1B98: script path at +0x60; BpObject_TryInvokeCurrentCallback supplies state.
    if ((*(unsigned int*)AsCBpObject(state)->scriptPathStorage >> 31) == 0)
        prefix = AsCBpObject(state)->scriptPathStorage + 1;
    else
        prefix = *(const char**)&AsCBpObject(state)->scriptPathStorage[8];
    volatile unsigned int pad[3];
    SmallString result;
    volatile Byte4 srcA;
    Byte4 allocatorTag1;
    volatile Byte4 srcB;
    Byte4 allocatorTag2;
    volatile Byte4 srcC;
    Byte4 allocatorTag3;
    // Phase 1: copy the script prefix, then append the resource and format suffix.
    textLength = strlen(prefix);
    String_ReserveCapacity(&result, textLength);
    allocatorTag1.s[0] = srcA.u[0];
    String_ReplaceRange(&result, 0, 0, prefix, prefix + textLength, &allocatorTag1.s[0]);
    unsigned int prefixLength;
    if ((result.flags >> 31) == 0)
        prefixLength = ((unsigned char*)&result)[0] & 0x7F;
    else
        prefixLength = result.length;
    allocatorTag2.s[0] = srcB.u[0];
    textLength = strlen(resourceName);
    String_ReplaceRange(&result, prefixLength, 0, resourceName, resourceName + textLength, &allocatorTag2.s[0]);
    unsigned int pathLength;
    const char* suffix = s_bpTextureResourceSuffix;
    if ((result.flags >> 31) == 0)
        pathLength = ((unsigned char*)&result)[0] & 0x7F;
    else
        pathLength = result.length;
    allocatorTag3.s[0] = srcC.u[0];
    String_ReplaceRange(&result, pathLength, 0, suffix, suffix + strlen(suffix), &allocatorTag3.s[0]);
    const char* resourcePath;
    if ((result.flags >> 31) == 0)
        resourcePath = (char*)&result + 1;
    else
        resourcePath = result.data;
    // Phase 2: consume the path while its string is alive, then publish the object.
    ((WidgetVt*)widget)->setText((void*)resourcePath);
    ((CBpObject*)state)->ReplaceObjectById((int)objectName, (int)widget);
    return 0;
}
#pragma use_lmw_stmw reset

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620A0", _ctors_u800C0C18_0, B2dSprite_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU804620A0", _ctors_u800C0C18_1, BpTextureCallback_InitTwoPiCache);
#pragma pop
