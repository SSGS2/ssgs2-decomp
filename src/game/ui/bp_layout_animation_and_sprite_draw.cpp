// Bp layout/player-panel animation scheduling and sprite draw traversal.
// Frame rate lookup: BpCtrl_UpdateAndDrawNodeList schedules 18 channels per
// sprite, ValueAnimation_UpdateTracksAtPosition seeks/advances their integer
// tick cursors, and bp_sprite_animation_and_draw_callbacks.cpp supplies ValueAnimation_AdvanceKeyframe.
// Duration scaling is s_valueAnimationDurationScale (u800BA958_data.cpp), NOT
// a seconds delta or display interval. Keep endpoint crossings, integer seek
// comparisons and loop topology when designing a later 60-fps mod.
// Scheduling knobs: g_valueAnimationUpdateMode (phase1_shared_data.cpp, default
// 1), s_valueAnimationUpdateCounter (u800BB334_data.cpp), and the position word
// supplied by callers. Mode bits are independent: 4 increments the global
// counter per CONTROLLER draw and always advances; otherwise 2 advances only
// while counter<=1; otherwise 1 resets it to zero without advancing. Seeking
// happens only while counter<=1, even in mode 4. Drawing multiple controllers
// therefore shares one gate; do not treat it as one counter per sprite/frame.
// Four draw dispatch sites here are candidate interpolation points in
// shipping manifests. These are context sites, not retail interpolation policy.
// Geometry knobs: kBpScaleOne=1024, kBpAlphaOne=128, rotation=integer degrees,
// source rectangles=texture pixels, destination=layout/screen pixels. Preserve
// divisions/truncation, parent composition and callback-specific argument order.
// Pools own objects/keyframes; returned pointers and callback stack arrays are
// borrowed. Reset/remove marks storage reusable, invalidating playback references.
// No controller mutation or callback retention during traversal is supported.
// Shared ABI layouts stay unchanged: each view below names a phase of the same
// 0x34-byte controller or 0x1C8-byte sprite, not separately owned allocations.
// Translation unit 0x800BB334-0x800BC91C (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "classes/BpSpriteDrawView.h"
#define AsBpDrawObject(p) ((BpSpriteDrawView*)(p))
// BpCtrl_UpdateAndDrawNodeList passes its resource as c to the draw helpers and to
// BpSprite_ComputeUnrotatedRect/ComputeRotatedQuad: existing sprite property layout.

extern "C" {

// The parent channel keeps its previous/current object IDs together.
struct BpParentLink { char padding[0x90]; int previousParentId, parentId; };


// Sprite transforms use 10 fractional bits; alpha uses 7.
enum { kBpControllerCount = 40, kBpChannelCount = 18,
       kBpScaleOne = 1024, kBpAlphaOne = 128,
       kBpObjectStride = 0x1C8, kBpControllerStride = 0x34,
       kBpChannelStride = 0x18, kBpNodeStride = 0x10,
       kBpHidden = 0x10, kBpDrawKindMask = 0xF,
       kBpStopAtTail = 0x100, kBpLoopStart = 0x200,
       kBpPreserveChannelValue = 0x800,
       kBpFormatMask = 0xF000, kBpTileFlipMask = 0xF0000, kBpBlendMask = 0xF0 };
// The low nibble selects a draw path, independently of kBpHidden.
enum BpDrawKind { kBpControllerSprite = 0, kBpParentSprite = 1,
                 kBpRotatedCallback = 2, kBpTransformedCallback = 3 };
enum BpUpdatePolicyBits {
    kBpResetSeekCounter = 1, kBpAdvanceDuringInitialSeek = 2,
    kBpAdvanceEveryTraversal = 4, kBpLastInitialSeekCounter = 1,
    kBpFreezeChannel = 1
};
struct BpControllerTransform {
    int id; char padding04[4]; void* objects; void* keyframes;
    int objectCapacity; unsigned int keyframeCapacity, keyframeScanIndex; // keyframeScanIndex is a scan cursor, not a free-slot guarantee.
    int x, y, opacity, scaleX, scaleY;
    void* firstObject;
};
struct BpNodeHeader {
    short poolIndex, drawFlags;
    int id, image, drawCallback, quadCallback;
    void* next;
};
// View of sprite channels passed independently of the object header.
struct BpSpriteChannels {
    char padding000[0x4];
    int image;
    char padding008[0x14];
    int sourceX;
    char padding020[0x14];
    int sourceY;
    char padding038[0x14];
    int sourceWidth;
    char padding050[0x14];
    int sourceHeight;
    char padding068[0x2C];
    int positionX;
    char padding098[0x8];
    char* formatProperty;
    char padding0A4[0x8];
    int positionY;
    char padding0B0[0x14];
    int scaleX;
    char padding0C8[0x14];
    int scaleY;
    char padding0E0[0x14];
    int opacity;
    char padding0F8[0x8];
    char* blendProperty;
    char padding104[0x20];
    int tileColumns;
    char padding128[0x8];
    char* tileFlipProperty;
    char padding134[0x8];
    int tileRows;
    char padding140[0x44];
    int rotation;
    char padding188[0x14];
    int tintColor;
};
// The alternate channel base must remain exactly 0x18 bytes after the shared object base.
#define BP_CHANNEL_CHECK(field) typedef char BpChannel_##field[((int)&((BpSpriteChannels*)0)->field + kBpChannelStride == (int)&((BpSpriteDrawView*)0)->field) ? 1 : -1]
BP_CHANNEL_CHECK(image);
BP_CHANNEL_CHECK(sourceX);
BP_CHANNEL_CHECK(sourceY);
BP_CHANNEL_CHECK(sourceWidth);
BP_CHANNEL_CHECK(sourceHeight);
BP_CHANNEL_CHECK(positionX);
BP_CHANNEL_CHECK(positionY);
BP_CHANNEL_CHECK(scaleX);
BP_CHANNEL_CHECK(scaleY);
BP_CHANNEL_CHECK(opacity);
BP_CHANNEL_CHECK(rotation);
BP_CHANNEL_CHECK(tintColor);
BP_CHANNEL_CHECK(formatProperty);
BP_CHANNEL_CHECK(blendProperty);
BP_CHANNEL_CHECK(tileColumns);
BP_CHANNEL_CHECK(tileRows);
BP_CHANNEL_CHECK(tileFlipProperty);
#undef BP_CHANNEL_CHECK
struct BpPropertyFlags { int value; unsigned int flags; };


extern void BpCtrl_InvokeGlobalHook(int, const char *, int, const char *);
extern int BpSprite_DrawWithControllerTransform(int, void *, int, void *, float *);
extern void ValueAnimation_AdvanceKeyframe(void *);
extern void BpSprite_DrawQuad(void *, void *, int, int, int, int, int);
extern void BpSprite_ComputeUnrotatedRect(void*, void*, void*);
extern void BpSprite_ComputeRotatedQuad(void *, void *, int);
extern void *Color_ModulatePackedChannels(void *, int, int, int, int);
extern void BpSprite_DrawTiles(void *, void *, int, int, int, int, int);
extern void BpSprite_DrawTransformedQuad(void *, void *, int, int, int, int, int);
extern char kBpControllerSourceFilename[];
extern unsigned char bpAnimationChannelDiagnostic[];
extern unsigned char bpObjectPoolExhaustedDiagnostic[];
extern unsigned char bpChannelDefaultOverrides[];
extern unsigned char s_bpObjectEntries[];
extern unsigned char bpControllerCallbackDiagnosticStrings[];
extern unsigned char bpControllerGetHeadHookNames[];
extern volatile unsigned int g_bpDefaultRectDrawCallback; // Default rectangular draw callback address.
extern volatile unsigned int g_bpDrawObjectCallback;
extern volatile unsigned int g_bpTileDrawCallback;
extern unsigned int g_valueAnimationUpdateMode;
extern volatile unsigned int s_bpCachedRenderCallbackValue;
extern volatile unsigned int g_bpCurrentDrawNode;
extern volatile unsigned int s_valueAnimationUpdateCounter;
extern volatile unsigned int g_bpDefaultSolidQuadCallback;
// Callback arguments are borrowed for the call; bounds is four screen-space xy pairs.
// image is a resource word, scale is /1024, opacity is /128, rotation is integer
// degrees (360 per turn). ComputeRotatedQuad forwards the same unconverted field
// to Math_RotateIntegerPoints2D, which indexes its trig table by angle*1024/360.
// The callback receives that angle in addition to already rotated screen corners;
// do not rotate the geometry a second time solely because this argument is nonzero.
// Concrete callback domains (InstallPlatformCallbacks in b2d_sprite_and_bp_texture_draw.cpp):
// - kind0 rectangular path: image is a CBpObject texture ID. The installed
//   SubmitScaledTextureRegion resolves it through CBpObject_FindTextureById;
//   require a loaded ID in that object's texture map, not a GPU handle/address.
// - kind1 solid path: first argument is a POINTER to sourceX's current word,
//   not its integer value. SubmitSolidQuad reads its four bytes as script color;
//   nominal scale/opacity 1024/128. The source-channel record is borrowed.
// - kind2/kind3 (the typedefs below): data comes from sprite header+8, untouched
//   32-bit application data. Installed B2d_DefaultSpriteQuadCallback and
//   B2d_DefaultTransformedQuadCallback are no-ops returning 0. Therefore these
//   paths impose no resource-ID/pointer interpretation; a custom callback must
//   agree with its data producer. BBC58Call likewise forwards an unchanged word.
// No implicit C++ receiver is supplied: all are free function pointers. The
// platform kind0/1 renderer obtains its CBpObject receiver from borrowed global
// g_currentBpCallbackObject, set around traversal by BpObject_TryInvokeCurrentCallback
// (bp_object_sprite_resources.cpp). Do not cache that global across callbacks or reenter traversal.
// tint is a packed SCRIPT color word, not GX RGBA: big-endian bytes [3,2,1]
// become R,G,B in the rectangular platform renderer; [0] is ignored there and
// opacity separately supplies A. Each color lane has nominal unity128; renderer
// expands n>0 to min(2*n-1,255). Parent composition multiplies each of four
// lanes /128 and clamps to a byte (Color_ModulatePackedChannels, bp_sprite_controller_and_drawing.cpp).
// Kind2 composes parent tint; kind3 passes local tint/scale/rotation, composes
// only controller opacity and screen corners. Their default no-op consumers
// do not decode tint; custom consumers must explicitly choose that script-color
// contract or supply a documented conversion. All 32 bits are forwarded.
// Source rectangle pixels must fit the loaded texture; widths/heights and scale
// products must fit signed int. Supported nominal opacity is 0..128 (no clamp
// here); zero skips some platform paths. Corners are four ordered xy pairs,
// integer layout pixels, top-left origin, X right/Y down. No callback may retain
// stack corner arrays. Return values are discarded by this traversal.
// These callbacks have different signatures and are not interchangeable.
typedef void (*BpTransformedQuadCallback)(int image, int* bounds, int rotation, int tint,
                             int scaleX, int scaleY, int opacity);
extern BpTransformedQuadCallback g_bpDefaultTransformedQuadCallback;
typedef void (*BpRotatedQuadCallback)(int image, int sourceWidth, int sourceHeight,
                            int tileColumns, int tileRows, int* bounds,
                            int rotation, int tint, int scaleX, int scaleY,
                            int opacity, unsigned int packedRenderFlags);
extern BpRotatedQuadCallback g_bpSpriteQuadDrawCallback;

int BpCtrl_AppendAnimationChannelNode(int index, void *base, int listIndex, int flags, void *keyframeValueAddress, void *durationOrCursor);
// firstKeyframe is the chain origin; playbackKeyframe is the playback cursor.
// Appending to a nonempty chain does not update the playback cursor.
struct BpAnimationChannelView { int startValue; int currentValue; short reserved08; short controlFlags; int position; void *firstKeyframe; void *playbackKeyframe; }; // +14 is the playback cursor.
int ValueAnimation_UpdateTracksAtPosition(BpAnimationChannelView *, int);
int BpSprite_DrawWithControllerTransform(int, void *, int, void *, float *);
int BpSprite_DrawWithParentTransform(void *, void *, void *, void *, float *);
int BpSprite_SubmitRotatedQuadCallback(void *, void *, void *, void *, float *);
int BpSprite_SubmitTransformedQuadCallback(void *, void *, void *, void *, float *);

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes on
#pragma opt_propagation off
// 0x800BBE70: finish the resource search at the first matching id.
// Find a pooled object by ID, reporting a missing ID through the global hook.
inline void BpCtrl_FindResource(void*& resource, int id, int& slotIndex, int& slotOffset, char*& items) {
    int slotCount = ((BpControllerTransform*)resource)->objectCapacity;
    slotIndex = 0;
    slotOffset = 0;
    for (; slotIndex < slotCount; slotOffset += kBpObjectStride, ++slotIndex) {
        items = (char*)((BpControllerTransform*)resource)->objects;
        if (*(int *)(items + slotOffset + 4) == id) { resource = items + slotIndex * kBpObjectStride; return; }
    }
    BpCtrl_InvokeGlobalHook(id, kBpControllerSourceFilename, 0x17E, (const char *)bpControllerGetHeadHookNames);
    resource = 0;
}

// Advance animation channels, resolve parent objects, and draw visible nodes in list order.
// index selects one of 40 controller slots. base is a numeric animation position
// carried as void* by the recovered ABI; value is optional screen xy offset.
// Return -2 without a render callback, -1 for an invalid controller, otherwise
// the sum of channels advanced. The global current-node/callback cache is reset
// after traversal. Parent IDs must resolve; this routine has no missing-parent guard.
int BpCtrl_UpdateAndDrawNodeList(int index, void *base, float *value) {
    void *first;
    void *slot;
    void *node;
    void *entry;
    int result;
    void *layout;
    char *items;
    void *resource;
    int i;
    int off;
    result = 0;
    if (g_bpDefaultRectDrawCallback == 0) return -2;
    if (index < 0 || index >= kBpControllerCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x84, (const char *)bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = s_bpObjectEntries + index * kBpControllerStride;
        if (*(int *)entry == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x89, (const char *)bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    if (entry == 0) return -1;
    // Phase 1: select counter behavior, then advance tracks across the entire active list.
    if (g_valueAnimationUpdateMode & kBpAdvanceEveryTraversal) ++s_valueAnimationUpdateCounter;
    else if ((g_valueAnimationUpdateMode & kBpAdvanceDuringInitialSeek) == 0 && (g_valueAnimationUpdateMode & kBpResetSeekCounter)) s_valueAnimationUpdateCounter = 0;
    for (first = ((BpControllerTransform*)entry)->firstObject; first != 0; first = ((BpNodeHeader*)first)->next) {
        if (g_valueAnimationUpdateMode & kBpAdvanceEveryTraversal) result += ValueAnimation_UpdateTracksAtPosition((BpAnimationChannelView *)first, (int)base);
        else if ((g_valueAnimationUpdateMode & kBpAdvanceDuringInitialSeek) && s_valueAnimationUpdateCounter <= kBpLastInitialSeekCounter) result += ValueAnimation_UpdateTracksAtPosition((BpAnimationChannelView *)first, (int)base);
    }
    // Phase 2: draw-list traversal. Resolve parent IDs within this same controller;
    // skip hidden parents, then test the child visibility/opacity and dispatch kind.
    int slotOffset = index * kBpControllerStride;
    slot = s_bpObjectEntries + slotOffset;
    node = ((BpControllerTransform*)entry)->firstObject;
    // 0x800BBE70: advance the list in the for expression, including skipped nodes.
    for (; node != 0; node = ((BpNodeHeader*)node)->next) {
        g_bpCurrentDrawNode = (unsigned int)node;
        layout = (char *)node + kBpChannelStride;
        int id = ((BpParentLink*)node)->parentId;
        resource = 0;
        if (id != -1) {
            if (index < 0 || index >= kBpControllerCount) {
                BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x84, (const char *)bpControllerCallbackDiagnosticStrings);
                resource = 0;
            } else if (*(int *)slot == -1) {
                BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x89, (const char *)bpControllerCallbackDiagnosticStrings);
                resource = 0;
            } else {
                resource = slot;
            }
            if (resource == 0) {
                resource = 0;
            } else {
                BpCtrl_FindResource(resource, id, i, off, items);
            }
            if (((BpNodeHeader*)resource)->drawFlags & kBpHidden) continue;
        }
        if ((((BpNodeHeader*)node)->drawFlags & kBpHidden) == 0 && ((BpSpriteChannels*)layout)->opacity != 0) {
            if ((((BpNodeHeader*)node)->drawFlags & kBpHidden) == 0) {
                switch (((BpNodeHeader*)node)->drawFlags & kBpDrawKindMask) {
                case kBpParentSprite: BpSprite_DrawWithParentTransform(entry, node, resource, layout, value); break;
                case kBpControllerSprite: BpSprite_DrawWithControllerTransform(index, node, (int)resource, layout, value); break;
                case kBpRotatedCallback: BpSprite_SubmitRotatedQuadCallback(entry, node, resource, layout, value); break;
                case kBpTransformedCallback: BpSprite_SubmitTransformedQuadCallback(entry, node, resource, layout, value); break;
                }
            }
        }
    }
    g_bpCurrentDrawNode = 0;
    s_bpCachedRenderCallbackValue = (unsigned int)-1;
    return result;
}
#pragma opt_common_subs reset
#pragma pop

// Combine controller and parent transforms, then submit a solid or rotated quad.
// channels points at sprite+0x18, not at a separate allocation. Optional parent
// contributes sprite-space scale/opacity/rotation; controller then maps coordinates
// to screen pixels and adds screenOffset (float xy truncated to integer pixels).
// Return 0 after submission, -1 if the required default callback is absent.
int BpSprite_DrawWithParentTransform(void *controller, void *sprite, void *parent, void *channels, float *screenOffset) {
    int defaultCallback;
    int scaleX = ((BpSpriteChannels*)channels)->scaleX;
    int scaleY = ((BpSpriteChannels*)channels)->scaleY;
    int opacity = ((BpSpriteChannels*)channels)->opacity;
    int rotation = ((BpSpriteChannels*)channels)->rotation;
    int isnull = screenOffset == 0;
    int x = isnull ? 0 : (int)screenOffset[0];
    int y = isnull ? 0 : (int)screenOffset[1];
    defaultCallback = (int)g_bpDefaultSolidQuadCallback;
    if (defaultCallback == 0) return -1;
    if (parent != 0) {
        scaleX = (scaleX * AsBpDrawObject(parent)->scaleX) / kBpScaleOne; // original +0xDC.
        scaleY = (scaleY * AsBpDrawObject(parent)->scaleY) / kBpScaleOne; // original +0xF4.
        opacity = (opacity * AsBpDrawObject(parent)->opacity) / kBpAlphaOne; // original +0x10C.
        rotation += AsBpDrawObject(parent)->rotation; // original +0x19C.
    }
    if (rotation != 0) {
        BpSprite_DrawQuad(sprite, parent, ((BpControllerTransform*)controller)->x + x, ((BpControllerTransform*)controller)->y + y,
            ((BpControllerTransform*)controller)->scaleX, ((BpControllerTransform*)controller)->scaleY, ((BpControllerTransform*)controller)->opacity);
    } else {
        void *cb = (void*)((BpNodeHeader*)sprite)->drawCallback;
        cb = cb != 0 ? cb : (void *)defaultCallback;
        int dims[4];
        BpSprite_ComputeUnrotatedRect(dims, sprite, parent);
        int packed = (((BpPropertyFlags*)((BpSpriteChannels*)channels)->formatProperty)->flags & kBpFormatMask) |
            (((BpPropertyFlags*)((BpSpriteChannels*)channels)->tileFlipProperty)->flags & kBpTileFlipMask) |
            (((BpPropertyFlags*)((BpSpriteChannels*)channels)->blendProperty)->flags & kBpBlendMask);
        // Packed-source callback: source-channel address, source width/height,
        // screen xy, controller-composed scale xy, composed opacity, render flags.
        typedef void (*CBCall)(void* sourceChannels, int sourceWidth, int sourceHeight,
                               int screenX, int screenY, int scaleX, int scaleY,
                               int opacity, int renderFlags);
        ((CBCall)cb)((void*)&((BpSpriteChannels*)channels)->sourceX, ((BpSpriteChannels*)channels)->sourceWidth, ((BpSpriteChannels*)channels)->sourceHeight,
            (dims[0] * ((BpControllerTransform*)controller)->scaleX) / kBpScaleOne + (x + ((BpControllerTransform*)controller)->x),
            (dims[1] * ((BpControllerTransform*)controller)->scaleY) / kBpScaleOne + (y + ((BpControllerTransform*)controller)->y),
            (scaleX * ((BpControllerTransform*)controller)->scaleX) / kBpScaleOne,
            (scaleY * ((BpControllerTransform*)controller)->scaleY) / kBpScaleOne,
            (opacity * ((BpControllerTransform*)controller)->opacity) / kBpAlphaOne,
            packed);
    }
    return 0;
}

// Forward the unchanged application-data word and borrowed eight-int corners to
// the explicit/default kind3 free function. No implicit receiver, retain, tint
// conversion or resource lookup. Requires a nonzero selected callback; see the
// callback domain contract above. Historical helper spelling remains searchable.
inline void BBC58Call(int data, int *bounds, int rotation, int tint, int scaleX, int scaleY, int opacity, int callback) {
    typedef void (*CB)(int image, int* screenCorners, int rotation, int tint,
                       int scaleX1024, int scaleY1024, int opacity128);
    callback = callback != 0 ? callback : (int)g_bpDefaultTransformedQuadCallback;
    ((CB)callback)(data, bounds, rotation, tint, scaleX, scaleY, opacity);
}

#pragma opt_lifetimes off
// Transform all four corners and submit rotation, tint, scale, and opacity to the callback.
// bounds begins in sprite space; each xy pair becomes screen space via
// screen = controllerScale*corner/1024 + controllerPosition + screenOffset.
// The callback borrows the stack bounds array. Return -1 without the default,
// otherwise 0. This path deliberately does not compose the optional parent here.
int BpSprite_SubmitTransformedQuadCallback(void *controllerArg, void *spriteArg, void *parent, void *channels, float *screenOffset) {
    int scaleX = ((BpSpriteChannels*)channels)->scaleX;
    int scaleY = ((BpSpriteChannels*)channels)->scaleY;
    int opacity = ((BpSpriteChannels*)channels)->opacity;
    int rotation = ((BpSpriteChannels*)channels)->rotation;
    int tint = ((BpSpriteChannels*)channels)->tintColor;
    void *sprite = spriteArg;
    void *controller = controllerArg;
    int isnull = screenOffset == 0;
    int x = isnull ? 0 : (int)screenOffset[0];
    int y = isnull ? 0 : (int)screenOffset[1];
    if (g_bpDefaultTransformedQuadCallback == 0) return -1;
    int bounds[8];
    BpSprite_ComputeRotatedQuad(bounds, sprite, 0);
    int *corners = bounds;
    corners[0] = (((BpControllerTransform*)controller)->scaleX * corners[0]) / kBpScaleOne + ((BpControllerTransform*)controller)->x + x;
    corners[1] = (((BpControllerTransform*)controller)->scaleY * corners[1]) / kBpScaleOne + ((BpControllerTransform*)controller)->y + y;
    corners[2] = (((BpControllerTransform*)controller)->scaleX * corners[2]) / kBpScaleOne + ((BpControllerTransform*)controller)->x + x;
    corners[3] = (((BpControllerTransform*)controller)->scaleY * corners[3]) / kBpScaleOne + ((BpControllerTransform*)controller)->y + y;
    corners[4] = (((BpControllerTransform*)controller)->scaleX * corners[4]) / kBpScaleOne + ((BpControllerTransform*)controller)->x + x;
    corners[5] = (((BpControllerTransform*)controller)->scaleY * corners[5]) / kBpScaleOne + ((BpControllerTransform*)controller)->y + y;
    corners[6] = (((BpControllerTransform*)controller)->scaleX * corners[6]) / kBpScaleOne + ((BpControllerTransform*)controller)->x + x;
    corners[7] = (((BpControllerTransform*)controller)->scaleY * corners[7]) / kBpScaleOne + ((BpControllerTransform*)controller)->y + y;
    (((BpTransformedQuadCallback)((BpNodeHeader*)sprite)->drawCallback) ? ((BpTransformedQuadCallback)((BpNodeHeader*)sprite)->drawCallback) : g_bpDefaultTransformedQuadCallback)(
        ((BpNodeHeader*)sprite)->image, bounds, rotation, tint, scaleX, scaleY,
        (opacity * ((BpControllerTransform*)controller)->opacity) / kBpAlphaOne);
    return 0;
}
#pragma opt_lifetimes reset

// Submit rotated sprite corners with controller/parent scaling and packed render flags.
// Parent geometry is included by ComputeRotatedQuad; controller transforms the
// resulting four xy pairs to screen space. Parent color/opacity/scale/rotation
// are composed separately for the callback. Bounds stays borrowed until it returns.
// Return -1 when the default callback is absent (even if the sprite has one),
// otherwise 0. controller/sprite/channels and their property pointers are required.
int BpSprite_SubmitRotatedQuadCallback(void *controller, void *sprite, void *parent, void *channels, float *screenOffset) {
    int isnull = screenOffset == 0;
    int scaleX = ((BpSpriteChannels*)channels)->scaleX;
    int scaleY = ((BpSpriteChannels*)channels)->scaleY;
    int opacity = ((BpSpriteChannels*)channels)->opacity;
    int rotation = ((BpSpriteChannels*)channels)->rotation;
    int tint = ((BpSpriteChannels*)channels)->tintColor;
    int x = isnull ? 0 : (int)screenOffset[0];
    int y = isnull ? 0 : (int)screenOffset[1];
    if (g_bpSpriteQuadDrawCallback == 0) return -1;
    int bounds[8];
    BpSprite_ComputeRotatedQuad(bounds, sprite, (int)parent);
    int *out = bounds;
    out[0] = (((BpControllerTransform*)controller)->scaleX * out[0]) / kBpScaleOne + (x + ((BpControllerTransform*)controller)->x);
    out[1] = (((BpControllerTransform*)controller)->scaleY * out[1]) / kBpScaleOne + (y + ((BpControllerTransform*)controller)->y);
    out[2] = (((BpControllerTransform*)controller)->scaleX * out[2]) / kBpScaleOne + (x + ((BpControllerTransform*)controller)->x);
    out[3] = (((BpControllerTransform*)controller)->scaleY * out[3]) / kBpScaleOne + (y + ((BpControllerTransform*)controller)->y);
    out[4] = (((BpControllerTransform*)controller)->scaleX * out[4]) / kBpScaleOne + (x + ((BpControllerTransform*)controller)->x);
    out[5] = (((BpControllerTransform*)controller)->scaleY * out[5]) / kBpScaleOne + (y + ((BpControllerTransform*)controller)->y);
    out[6] = (((BpControllerTransform*)controller)->scaleX * out[6]) / kBpScaleOne + (x + ((BpControllerTransform*)controller)->x);
    out[7] = (((BpControllerTransform*)controller)->scaleY * out[7]) / kBpScaleOne + (y + ((BpControllerTransform*)controller)->y);
    if (parent != 0) {
        scaleX = (scaleX * AsBpDrawObject(parent)->scaleX) / kBpScaleOne;
        scaleY = (scaleY * AsBpDrawObject(parent)->scaleY) / kBpScaleOne;
        rotation += AsBpDrawObject(parent)->rotation;
        opacity = (opacity * AsBpDrawObject(parent)->opacity) / kBpAlphaOne;
        tint = (int)((void *(*)(void *, int, int))Color_ModulatePackedChannels)((void *)tint, AsBpDrawObject(parent)->tintColor, AsBpDrawObject(parent)->opacity); // +0x1B4/+0x10C.
    }
    (((BpRotatedQuadCallback)((BpNodeHeader*)sprite)->drawCallback) ? ((BpRotatedQuadCallback)((BpNodeHeader*)sprite)->drawCallback) : g_bpSpriteQuadDrawCallback)(
        ((BpNodeHeader*)sprite)->image, ((BpSpriteChannels*)channels)->sourceWidth, ((BpSpriteChannels*)channels)->sourceHeight,
        ((BpSpriteChannels*)channels)->tileColumns, ((BpSpriteChannels*)channels)->tileRows, bounds, rotation, tint,
        scaleX, scaleY, (opacity * ((BpControllerTransform*)controller)->opacity) / kBpAlphaOne,
        (((BpPropertyFlags*)((BpSpriteChannels*)channels)->formatProperty)->flags & kBpFormatMask) |
        (((BpPropertyFlags*)((BpSpriteChannels*)channels)->tileFlipProperty)->flags & kBpTileFlipMask) |
        (((BpPropertyFlags*)((BpSpriteChannels*)channels)->blendProperty)->flags & kBpBlendMask));

    return 0;
}

// One 16-byte pooled keyframe layout, with word-preserving numeric/ABI views.
// duration == -1 is free; next is cleared when returning a node to the pool.
struct BpKeyframe {
    union { int value; void* keyframeValue; };
    int flags;
    union { int duration; void* durationWord; };
    BpKeyframe* next;
};
typedef char BpKeyframe_size[(sizeof(BpKeyframe) == kBpNodeStride) ? 1 : -1];

struct BpChannelHeadView {
    unsigned char padding[0x10];
    BpKeyframe *first;
};

#pragma push
#pragma use_lmw_stmw on
// Seek every active channel to the requested position and return the number advanced.
// slots is the object base, not the first BpAnimationChannelView: channels start at +0x18.
// position/duration use integer animation ticks; Advance then adds one tick.
// position is nonnegative and chains must have valid durations/links; the seek
// comparison is UNSIGNED and strict (<), so equality stays on the current key.
// An exhausted tail breaks the scan without replacing the existing cursor.
// All 18 headers must exist, even if inactive. Frozen channels may still seek.
// Channel/keyframe relationship: object+0x18+i*0x18 is a 24-byte header
// {startBits,currentBits,s16 valueType,s16 controlFlags,s32 tickPosition,
//  BpKeyframe* first,BpKeyframe* playback}. BpSpriteChannels is a strided view
// of those SAME currentBits/properties, beginning at object+0x18; it is not a
// contiguous array of sprite values or a second allocation. Keyframes normally
// live in the controller's separate 16-byte node pool. The ABI permits aliasing
// of a header with a keyframe; retain each member reload after a header write.
// Counter<=1 allows seeking; frozen control bit0 skips Advance only.
int ValueAnimation_UpdateTracksAtPosition(BpAnimationChannelView *slots, int position) {
    BpAnimationChannelView *slot;
    int updatedCount;
    int channelIndex;
    int zero;
    slot = (BpAnimationChannelView *)((char *)slots + kBpChannelStride);
    updatedCount = 0;
    channelIndex = 0;
    zero = 0;
    while (channelIndex < kBpChannelCount) {
        BpAnimationChannelView *channel = slot;
        int nextIndex = channelIndex + 1;
        if (nextIndex < kBpChannelCount) {
            slot = (BpAnimationChannelView *)((char *)slots + nextIndex * 0x18 + 0x18);
        }
        BpKeyframe *keyframe = (BpKeyframe *)channel->firstKeyframe;
        int remainingPosition = position; // each slot starts from the same position
        if (channel->playbackKeyframe != 0) {
            if (s_valueAnimationUpdateCounter <= kBpLastInitialSeekCounter) {
                while ((unsigned int)keyframe->duration < (unsigned int)remainingPosition) {
                    remainingPosition -= keyframe->duration;
                    if (keyframe->next == 0)
                        break;
                    // The slot may overlap the keyframe, so each field is read after the previous store, as the original does.
                    channel->position = zero;
                    channel->startValue = keyframe->value;
                    channel->currentValue = keyframe->value;
                    channel->playbackKeyframe = (void *)keyframe->next;
                    keyframe = keyframe->next;
                }
                channel->position = remainingPosition;
            }
            if ((channel->controlFlags & kBpFreezeChannel) == 0) {
                ValueAnimation_AdvanceKeyframe(channel);
                ++updatedCount;
            }
        }
        ++channelIndex;
    }
    return updatedCount;
}
#pragma pop

// Apply controller and optional parent transforms, selecting tiled, rotated, or rectangular drawing.
// index must identify a live controller (0..39); diagnostics alone do not guard
// later entry dereferences. sprite/channels are required, parent is a nullable
// sprite address carried as int, screenOffset is nullable float xy. Return 0;
// fallback draw return values are ignored. Callback pointers are borrowed.
int BpSprite_DrawWithControllerTransform(int index, void *sprite, int parent, void *channels, float *screenOffset) {
    // Phase 1: resolve the controller, screen offset and image callback cache.
    unsigned char *entry;
    int x;
    int y;
    if (0 > index || index >= kBpControllerCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x84, (const char *)bpControllerCallbackDiagnosticStrings);
        entry = 0;
    } else {
        entry = s_bpObjectEntries + index * kBpControllerStride;
        if (*(int *)entry == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x89, (const char *)bpControllerCallbackDiagnosticStrings);
            entry = 0;
        }
    }
    int positionX = ((BpSpriteChannels*)channels)->positionX; int positionY = ((BpSpriteChannels*)channels)->positionY;
    int scaleX = ((BpSpriteChannels*)channels)->scaleX; int scaleY = ((BpSpriteChannels*)channels)->scaleY;
    int opacity = ((BpSpriteChannels*)channels)->opacity; int rotation = ((BpSpriteChannels*)channels)->rotation;

    unsigned int tint = ((BpSpriteChannels*)channels)->tintColor;
    int isnull = ((!screenOffset));
    if (isnull) x = 0; else x = (int)screenOffset[0];
    if (isnull) y = 0; else y = (int)screenOffset[1];
    if (((BpSpriteChannels*)channels)->image != (int)s_bpCachedRenderCallbackValue) {
        typedef int (*SelectDrawObjectCallback)(int controllerIndex);
        SelectDrawObjectCallback cb = (SelectDrawObjectCallback)g_bpDrawObjectCallback;
        if (cb) s_bpCachedRenderCallbackValue = cb(index);
    }
    // Phase 2: compose parent sprite values before applying controller scale/translation.
    if (parent) {
        int parentScaleX = AsBpDrawObject(parent)->scaleX;
        int parentScaleY = AsBpDrawObject(parent)->scaleY;
        int scaledX = scaleX * parentScaleX;
        rotation += AsBpDrawObject(parent)->rotation;
        positionX += AsBpDrawObject(parent)->positionX; // original +0xAC.
        int parentOpacity = AsBpDrawObject(parent)->opacity;
        int shiftedScaleX = scaledX >> 10;
        int parentY = AsBpDrawObject(parent)->positionY; // original +0xC4.
        int scaledY = scaleY * parentScaleY;
        int parentTint = AsBpDrawObject(parent)->tintColor;
        scaleX = scaledX / kBpScaleOne;
        positionY += parentY;
        int shiftedScaleY = scaledY >> 10;
        scaleY = scaledY / kBpScaleOne;
        opacity = (opacity * parentOpacity) / kBpAlphaOne;
        tint = (int)Color_ModulatePackedChannels((void *)tint, parentTint, parentOpacity, shiftedScaleY, shiftedScaleX);
    }

    // Phase 3: choose tiled, rotated or axis-aligned submission. Source rectangles
    // remain texture coordinates; destination xy uses controller scale then screen offset.
    if (((BpSpriteChannels*)channels)->tileColumns != 1 || ((BpSpriteChannels*)channels)->tileRows != 1) {
        int (*cbp)(int, ...) = (int (*)(int, ...))g_bpTileDrawCallback;
        if (0 != cbp) {

        typedef void (*CBCall2)(int image, int sourceX, int sourceY,
                               int sourceWidth, int sourceHeight,
                               int screenX, int screenY, int scaleX, int scaleY,
                               int opacity, int tint, int tileColumns, int tileRows,
                               unsigned int renderFlags);
            ((CBCall2)cbp)(((BpSpriteChannels*)channels)->image, ((BpSpriteChannels*)channels)->sourceX, ((BpSpriteChannels*)channels)->sourceY, ((BpSpriteChannels*)channels)->sourceWidth, ((BpSpriteChannels*)channels)->sourceHeight,
            (positionX * ((BpControllerTransform*)entry)->scaleX) / kBpScaleOne + ((BpControllerTransform*)entry)->x + x,
            (positionY * ((BpControllerTransform*)entry)->scaleY) / kBpScaleOne + ((BpControllerTransform*)entry)->y + y,
            (scaleX * ((BpControllerTransform*)entry)->scaleX) / kBpScaleOne,
            (scaleY * ((BpControllerTransform*)entry)->scaleY) / kBpScaleOne,
            (opacity * ((BpControllerTransform*)entry)->opacity) / kBpAlphaOne,
            tint, ((BpSpriteChannels*)channels)->tileColumns, ((BpSpriteChannels*)channels)->tileRows, ((((BpPropertyFlags*)((BpSpriteChannels*)channels)->formatProperty)->flags & kBpFormatMask) |
            (((BpPropertyFlags*)((BpSpriteChannels*)channels)->tileFlipProperty)->flags & kBpTileFlipMask) |
            (((BpPropertyFlags*)((BpSpriteChannels*)channels)->blendProperty)->flags & kBpBlendMask)));
        } else {
            BpSprite_DrawTiles(sprite, (void *)parent, ((BpControllerTransform*)entry)->x + x, ((BpControllerTransform*)entry)->y + y, ((BpControllerTransform*)entry)->scaleX, ((BpControllerTransform*)entry)->scaleY, ((BpControllerTransform*)entry)->opacity);

        }
    } else {
        if (rotation) {
            BpSprite_DrawTransformedQuad(sprite, (void *)parent, ((BpControllerTransform*)entry)->x + x, ((BpControllerTransform*)entry)->y + y, ((BpControllerTransform*)entry)->scaleX, ((BpControllerTransform*)entry)->scaleY, ((BpControllerTransform*)entry)->opacity);
        } else { void *cb = (void*)((BpNodeHeader*)sprite)->drawCallback;  int dims[4];

            cb = (cb != 0) ? cb : (void *)g_bpDefaultRectDrawCallback;

              ((void (*)(void *, void *, void *))BpSprite_ComputeUnrotatedRect)(dims, sprite, (void *)parent);

            typedef void (*CBCall)(int image, int sourceX, int sourceY,
                                  int sourceWidth, int sourceHeight,
                                  int screenX, int screenY, int scaleX, int scaleY,
                                  int opacity, int tint, unsigned int renderFlags);
            unsigned int packed = (((BpPropertyFlags*)((BpSpriteChannels*)channels)->formatProperty)->flags & kBpFormatMask) |
                (((BpPropertyFlags*)((BpSpriteChannels*)channels)->tileFlipProperty)->flags & kBpTileFlipMask) |
            (((BpPropertyFlags*)((BpSpriteChannels*)channels)->blendProperty)->flags & kBpBlendMask);
            ((CBCall)cb)(((BpSpriteChannels*)channels)->image, ((BpSpriteChannels*)channels)->sourceX, ((BpSpriteChannels*)channels)->sourceY, ((BpSpriteChannels*)channels)->sourceWidth, ((BpSpriteChannels*)channels)->sourceHeight,
            (dims[0] * ((BpControllerTransform*)entry)->scaleX) / kBpScaleOne + ((BpControllerTransform*)entry)->x + x,
            (dims[1] * ((BpControllerTransform*)entry)->scaleY) / kBpScaleOne + ((BpControllerTransform*)entry)->y + y,
            (scaleX * ((BpControllerTransform*)entry)->scaleX) / kBpScaleOne,
            (scaleY * ((BpControllerTransform*)entry)->scaleY) / kBpScaleOne,
            (opacity * ((BpControllerTransform*)entry)->opacity) / kBpAlphaOne,
            tint, packed);
        }
    }
    return 0;
}

// Return one keyframe to the controller pool and advance to its saved link.
// Repeated link snapshots retain the verified CodeWarrior register allocation;
// they all read the same link before duration/next/flags/value are cleared.
static inline void ReturnBpKeyframeToPool(BpKeyframe*& node) {
    __typeof__(node->next) nextBeforeReset1 = node->next;
    __typeof__(node->next) nextBeforeReset2 = node->next;
    __typeof__(node->next) nextBeforeReset3 = node->next;
    __typeof__(nextBeforeReset3) nextBeforeReset4 = node->next;
    __typeof__(node->next) nextBeforeReset5 = nextBeforeReset1;
    __typeof__(nextBeforeReset2) nextBeforeReset6 = node->next;
    __typeof__(nextBeforeReset4) nextBeforeReset7 = node->next;
    __typeof__(node->next) nextBeforeReset8 = nextBeforeReset5;
    __typeof__(nextBeforeReset6) nextBeforeReset9 = node->next;
    __typeof__(nextBeforeReset8) nextBeforeReset10 = nextBeforeReset9;
    __typeof__(nextBeforeReset10) nextBeforeReset11 = nextBeforeReset7;
    node->duration = -1;
    node->next = 0;
    node->flags = 0;
    node->value = 0;
    node = nextBeforeReset11;
}

struct BpChannelResetView {
    char padding00[0x18];
    int startValue, currentValue;
    char padding20[4];
    int position;
};
// Reset channel position, copy current to start value, and return queued keyframes to the pool.
// base is an object base, index selects a 0x18-byte channel at base+0x18+index*0x18.
// Each keyframe has value/flags/duration/next; duration=-1 marks a free pool node.
// Nodes are marked free in place, never deallocated. Return 0. A valid index is a
// precondition: the position/start stores precede the diagnostic range check.
// Only firstKeyframe is cleared: playbackKeyframe retains its old address into
// reusable pool storage. Caller must reseed playback before advancing again.
int BpController_ResetAnimationState(void *base, int index) {
    unsigned char *entry = (unsigned char *)base + index * kBpChannelStride;
    ((BpChannelResetView*)entry)->position = 0;
    *(int *)(entry + 0x18) = ((BpChannelResetView*)entry)->currentValue;
    BpKeyframe *node;
    BpAnimationChannelView* channel = (BpAnimationChannelView*)(entry + kBpChannelStride);
    if (0 > index || index >= kBpChannelCount) {
        BpCtrl_InvokeGlobalHook(-1, kBpControllerSourceFilename, 0x1A2, (const char *)bpAnimationChannelDiagnostic);
        channel = 0;
    }
    if (channel) {
        if (channel->firstKeyframe != 0) {
            node = (BpKeyframe*)channel->firstKeyframe;
            while (node) {
                ReturnBpKeyframeToPool(node);
            }
            channel->firstKeyframe = 0;
        }
    }
    return 0;
}

struct BpChannelHeaderView {
    void *startValue;
    void *currentValue;
    unsigned char padding08[4];
    int position;
    BpKeyframe *first;
    BpKeyframe *playbackKeyframe; // Playback cursor at +14, initialized only for an empty channel.
};
typedef char BpChannelHeader_size[(sizeof(BpChannelHeaderView) == kBpChannelStride) ? 1 : -1];

// A non-owning address view used in two disjoint append phases: first a channel
// header, then its loop-start keyframe. Only the selected member is accessed;
// no union object is allocated or copied over the 16-byte keyframe pool storage.
union BpAppendLinkView { BpChannelHeaderView channel; BpKeyframe loopStart; };

struct BpControllerPoolView {
    unsigned char padding[0x0C];
    unsigned char *nodes;
    int reserved;
    int capacity;
    int keyframeScanIndex; // Next keyframe scan position; may name an occupied slot or capacity.
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
// 0x800BC248: scan free slots, retrying once from the beginning.
// Scan unused keyframes from the allocation cursor, wrapping once before reporting exhaustion.
// Logical result: node is borrowed pool storage, or null. slotIndex/nodeOffset/
// startIndex are scratch lifetimes retained for code generation, not extra results.
// A free slot has duration==-1; scanIndex can equal capacity and is not a free list.
inline void BpCtrl_FindFreeNode(BpControllerPoolView*& entry, int& index, unsigned char*& messages, int& slotIndex, BpKeyframe*& node, int& nodeOffset, int& startIndex) {
    for (;;) {
        startIndex = entry->keyframeScanIndex;
        slotIndex = startIndex;
        nodeOffset = startIndex * kBpNodeStride;
        while (slotIndex < entry->capacity) {
            BpKeyframe *candidate = (BpKeyframe *)(entry->nodes + nodeOffset);
            if ((int)candidate->duration == -1) {
                entry->keyframeScanIndex = slotIndex + 1;
                node = (BpKeyframe *)(entry->nodes + slotIndex * kBpNodeStride);
                return;
            }
            nodeOffset += kBpNodeStride;
            ++slotIndex;
        }
        if (startIndex != 0) {
            entry->keyframeScanIndex = 0;
            continue;
        }
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x5AB, (const char *)((messages + 0x90)));
        node = 0;
        break;
    }
}

// Borrow the keyframe addressed by an append cursor after its tick word was consumed.
static inline BpKeyframe* BpKeyframeAt(void* cursorWord) { return (BpKeyframe*)cursorWord; }

// Allocate a keyframe and append it while preserving the channel stop/loop flags.
// keyframeValueAddress points to a borrowed value word copied into the pool node;
// durationOrTailWord begins as a tick count in a pointer-sized ABI word. After
// copying that count into the new node, it holds a keyframe address. Typed
// access is centralized in BpKeyframeAt; retaining the parameter storage is
// necessary for byte-identical register allocation in this compiler.
// No allocation ownership is transferred from these inputs.
// durationOrTailWord carries an INTEGER tick count in the 32-bit pointer ABI,
// never a dereferenced caller buffer. Use nonnegative durations; -1 is the pool's
// free-node sentinel and must not be appended. Flags low nibble: 1 linear,
// 2 hold,3 ease-in,4 ease-out (AdvanceKeyframe in bp_sprite_animation_and_draw_callbacks.cpp). For a duration
// reaching interpolation, other nibble values leave a zero divisor. Independent
// topology bits: 0x100 stop/tail marker,0x200 loop start,0x800 preserve header
// value on first append. Value word is int/float bits/four script-color bytes
// according to header valueType 0/1/3; keyframeValueAddress must point to that word.
// Zero duration is allowed as an immediate endpoint: Advance increments a tick,
// compares scaled duration, snaps value and follows next ONCE in that call;
// it never divides by zero or drains a chain in the same Advance call. Seeking
// differs: for positive position it skips zero-duration nodes without reducing
// remainingPosition. A cycle of only zero durations can therefore spin forever
// in seek; ensure every seek-reachable cycle includes a positive-duration node.
// Positive durations are scaled/truncated by s_valueAnimationDurationScale in
// Advance, whereas seek compares stored ticks without scaling. Preserve both.
// Return 0 on append, -1 for bad channel/corrupt traversal, -2 for unavailable pool
// node, -3 for a controller lookup failure while linking an existing chain.
int BpCtrl_AppendAnimationChannelNode(int index, void *base, int listIndex, int flags, void *keyframeValueAddress, void *durationOrTailWord) {
    // Phase 1: resolve the channel view and controller pool before allocating.
    unsigned char *b = bpChannelDefaultOverrides;
    char *slot;
    BpAppendLinkView* link;
    if (listIndex < 0 || listIndex >= kBpChannelCount) {
        BpCtrl_InvokeGlobalHook(-1, kBpControllerSourceFilename, 0x1A2, (const char *)(((char*)(b + 0x80))));
        link = 0;
    } else {
        slot = (char *)base + listIndex * kBpChannelStride;
        link = (BpAppendLinkView*)(slot + 0x18);
    }

    int i;
    BpControllerPoolView *entry;
    if (index < 0 || index >= kBpControllerCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x84, (const char *)(((char*)(b + 0x10))));
        entry = 0;
    } else {
        entry = (BpControllerPoolView *)(s_bpObjectEntries + index * kBpControllerStride);
        if (*(int *)entry == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x89, (const char *)(((char*)(b + 0x10))));
            entry = 0;
        }
    }

    // Phase 2: find a free pooled keyframe and initialize its copied value/flags/duration.
    BpKeyframe *node;
    if (entry == 0) {
        node = 0;
    } else {
        int offset;
        int start;
        BpCtrl_FindFreeNode(entry, index, b, i, node, offset, start);
    }
    if (node == 0) {
        node = 0;
    } else {
        node->keyframeValue = *(void **)keyframeValueAddress;
        node->flags = flags;
        node->durationWord = durationOrTailWord;
        node->next = 0;
    }

    // Phase 3: link topology (the playback cursor is not the chain tail):
    // empty: first=cursor=new, position=0; seed values unless PreserveChannelValue.
    // nonempty: walk next until null or StopAtTail; remember the latest LoopStart.
    // splice: clear old StopAtTail, oldTail.next=new; a stopping new node links
    // back to the remembered loop target. Existing playback cursor stays unchanged.
    // Errors below do not return the newly allocated node to the free pool.
    if (link == 0) return -1;
    if (node == 0) return -2;
    // The ABI tick word is committed; this register now holds a traversal cursor.
    durationOrTailWord = link->channel.first;
    if (!durationOrTailWord) {
        link->channel.first = node;
        link->channel.playbackKeyframe = node;
        link->channel.position = 0;
        if ((flags & kBpPreserveChannelValue) == 0) {
            link->channel.startValue = node->keyframeValue;
            link->channel.currentValue = node->keyframeValue;
        }
    } else {
        // The input flags are already in node->flags. Reuse their scalar scratch
        // as the traversal count, bounding cyclic/corrupt chains by pool capacity.
        flags = 0;
        // The loop target starts at the first keyframe and moves to the last
        // encountered loop-start marker. The header view is dead on this path;
        // link now addresses the loopStart member, never the channel member.
        link = (BpAppendLinkView*)durationOrTailWord;
        BpControllerPoolView *e2;
        if (index < 0 || index >= kBpControllerCount) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x84, (const char *)(((char*)(b + 0x10))));
            e2 = 0;
        } else {
            e2 = (BpControllerPoolView *)(s_bpObjectEntries + index * kBpControllerStride);
            if (*(int *)e2 == -1) {
                BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x89, (const char *)(((char*)(b + 0x10))));
                e2 = 0;
            }
        }
        if (e2 == 0) {
            return -3;
        }
        int tailFlags;
        BpKeyframe *next;
        while ((next = BpKeyframeAt(durationOrTailWord)->next) != 0) {
            tailFlags = BpKeyframeAt(durationOrTailWord)->flags;
            if ((tailFlags & kBpLoopStart) != 0) link = (BpAppendLinkView*)durationOrTailWord;
            if ((tailFlags & kBpStopAtTail) != 0) break;
            durationOrTailWord = next;
            if (++flags > e2->capacity) {
                BpCtrl_InvokeGlobalHook(-1, kBpControllerSourceFilename, 0x6F3, (const char *)(((char*)(b + 0xB4))));
                return -1;
            }
        }
        // Commit the splice only after bounded traversal and controller lookup succeed.
        BpKeyframeAt(durationOrTailWord)->flags &= ~kBpStopAtTail;
        BpKeyframeAt(durationOrTailWord)->next = node;
        if ((node->flags & kBpStopAtTail) != 0) node->next = &link->loopStart;
    }
    return 0;
}
#pragma opt_common_subs reset
#pragma pop

struct BpObjectPoolRecord {
    short index;
    unsigned char padding[2];
    int id;
    int image;
    int drawRectCallback;
    int drawQuadCallback;
    unsigned char channelStorage[0x1B4];
};

struct BpObjectPoolView {
    unsigned char padding[8];
    BpObjectPoolRecord *entries;
    unsigned char padding0C[4];
    int count;
};

struct BpActiveObjectView {
    short poolIndex;
    short kind;
    int id;
    int image;
    int drawRectCallback;
    int drawQuadCallback;
    BpActiveObjectView *nextObj;
    unsigned char padding[0x18];
};

struct BpControllerObjectView {
    int id;
    unsigned char padding[4];
    void *items;
    int keyframePoolAddress;
    int count;
    unsigned char poolAndTransformStorage[0x1C];
    BpActiveObjectView *objects;
};

#pragma push
#pragma use_lmw_stmw on


// 0x800BC634: resolve the requested object before unlinking it.
// Resolve an object ID within a controller pool before removal.
inline void BpCtrl_FindObject(BpControllerObjectView*& requested, int& id, unsigned char*& base, BpActiveObjectView*& object, int& slotCount, int& slotIndex, int& slotOffset, char*& items) {
    slotCount = requested->count;
    slotIndex = 0; slotOffset = 0;
    for (; slotIndex < slotCount; slotOffset += kBpObjectStride, ++slotIndex) {
        items = (char *)requested->items;
        if (id == *(int *)(items + slotOffset + 4)) {
            object = (BpActiveObjectView *)(items + slotIndex * kBpObjectStride);
            return;
        }
    }
    BpCtrl_InvokeGlobalHook(id, kBpControllerSourceFilename, 0x17E, (const char *)(((char*)(base + 0x60))));
    object = 0;
}

// 0x800BC634: a successful unlink shares the caller's final reset.
// Unlink a non-head object, reporting an inconsistent active list through the hook.
inline void BpCtrl_UnlinkObject(BpActiveObjectView*& first, BpActiveObjectView*& object, unsigned char*& base, BpActiveObjectView*& previous, BpActiveObjectView*& next) {
    previous = first;
    while ((next = previous->nextObj) != 0) {
        if (next == object) {
            previous->nextObj = object->nextObj;
            return;
        }
        previous = next;
    }
    BpCtrl_InvokeGlobalHook(-2, kBpControllerSourceFilename, 0x7D3, (const char *)(((char*)(base + 0xC8))));
}

// Clear parent references, release animation nodes, unlink the object, and mark its ID unused.
// Phase order: resolve controller/object -> detach children for draw kind 3 ->
// mark each channel's nodes free -> unlink active object -> set ID=-1.
// Pool storage remains owned by the controller; no heap storage is freed here.
// Return -2 for invalid controller, -1 for missing object, otherwise 0.
int BpCtrl_RemoveObject(int index, int id) {
    unsigned char *base = bpChannelDefaultOverrides;
    BpControllerObjectView *entry;
    if (index < 0 || index >= kBpControllerCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x84, (const char *)(((char*)(base + 0x10))));
        entry = 0;
    } else {
        entry = (BpControllerObjectView *)(s_bpObjectEntries + index * kBpControllerStride);
        if (entry->id == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x89, (const char *)(((char*)(base + 0x10))));
            entry = 0;
        }
    }
    if (entry == 0) return -2;

    BpControllerObjectView *requested;
    if (index < 0 || index >= kBpControllerCount) {
        BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x84, (const char *)(((char*)(base + 0x10))));
        requested = 0;
    } else {
        requested = (BpControllerObjectView *)(s_bpObjectEntries + index * kBpControllerStride);
        if (requested->id == -1) {
            BpCtrl_InvokeGlobalHook(index, kBpControllerSourceFilename, 0x89, (const char *)(((char*)(base + 0x10))));
            requested = 0;
        }
    }

    BpActiveObjectView *object;
    if (requested == 0) {
        object = 0;
    } else {
        int count;
        int i, off;
        char *items;
        BpCtrl_FindObject(requested, id, base, object, count, i, off, items);
    }
    if (object == 0) return -1;

    if ((object->kind & kBpDrawKindMask) == 3) {
        BpActiveObjectView *node = entry->objects;
        while (node != 0) {
            if (((BpParentLink*)node)->parentId == object->id) {
                ((BpParentLink*)node)->parentId = -1;
                ((BpParentLink*)node)->previousParentId = -1;
            }
            node = node->nextObj;
        }
    }

    // Release all 18 channel chains back to the pool before unlinking the active object.
    {
        char *cur = (char *)object + kBpChannelStride;
        int i = 0;
        do {
            BpChannelHeadView *head;
            if (i < 0 || i >= kBpChannelCount) {
                BpCtrl_InvokeGlobalHook(-1, kBpControllerSourceFilename, 0x1A2, (const char *)(((char*)(base + 0x80))));
                head = 0;
            } else {
                head = (BpChannelHeadView *)cur;
            }
            if (head != 0) {
                BpKeyframe *node = head->first;
                if (node != 0) {
                    while (node != 0) {
                        BpKeyframe *next = node->next;
                        node->duration = -1;
                        node->next = 0;
                        node->flags = 0;
                        node->value = 0;
                        node = next;
                    }
                    head->first = 0;
                }
            }
            ++i;
            cur += 0x18;
        } while (i < kBpChannelCount);
    }

    // Detach from the active chain; ID=-1 returns this object slot to the free pool.
    BpActiveObjectView *first = entry->objects;
    if (first == 0) {
        BpCtrl_InvokeGlobalHook(-1, kBpControllerSourceFilename, 0x7C2, (const char *)(((char*)(base + 0xC8))));
    } else if (first == object) {
        entry->objects = object->nextObj;
    } else {
        BpActiveObjectView *previous;
        BpActiveObjectView *nxt;
        BpCtrl_UnlinkObject(first, object, base, previous, nxt);
    }
    object->id = -1;
    return 0;
}
#pragma pop

// Return an existing ID or prepare a free pooled object and report whether it already existed.
// table/found are required; id must differ from the unused marker -1. Writes
// *found=1 only on an ID hit, otherwise 0. A free slot only has its pool index
// and callbacks prepared: caller must assign ID and initialize/link the object.
// Exhaustion reports through the hook and returns null; no heap allocation.
void *BpCtrl_FindOrAllocObject(BpObjectPoolView *table, int id, int *found) {
    *found = 0;
    int index = 0;
    for (; index < table->count; ++index) {
        BpObjectPoolRecord *entry = (BpObjectPoolRecord *)((char *)table->entries + index * kBpObjectStride);
        if (id == entry->id) {
            *found = 1;
            return (char *)table->entries + index * kBpObjectStride;
        }
    }
    index = 0;
    int count = table->count;
    int offset = 0;
    for (; index < count; offset += kBpObjectStride, ++index) {
        if (*(int *)((char *)table->entries + offset + 0x4) == -1) {
            table->entries[index].index = (short)index;
            table->entries[index].drawRectCallback = 0;
            table->entries[index].drawQuadCallback = 0;
            return (char *)table->entries + index * kBpObjectStride;
        }
    }
    BpCtrl_InvokeGlobalHook(-1, kBpControllerSourceFilename, 0x826, (const char *)bpObjectPoolExhaustedDiagnostic);
    return 0;
}

} // extern "C"
