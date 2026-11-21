#include <decomp/ct_member.h>
typedef unsigned long new_size_t;
#include "MSL/error_message.h"
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char s_layoutStringPool[];
}
}

// Layout system (translation unit 0x80077F74-0x8007D2D4, boundaries inferred by decomp-toolkit from extab).
// known yet, so the functions are extern "C" to keep the names used in config/R2PE9B/symbols.txt.
// Mod lookup: element placement/scale/color -> Layout_SetElement* and fields
// below; pointer eligibility -> playerFlags / Layout_SetActive;
// hover offsets/scale -> item records and Layout_UpdateCursorHover[Feedback];
// asynchronous readiness -> Layout_PollLoadComplete, UpdateStandard/Extended.
// All views use the Wii 32-bit ABI. Array indices and player indices (0..3)
// are caller-validated; getter/setter pointers are required, borrowed for call.
// Geometry is in sprite/layout coordinates: position minus scaled origin gives
// the hit rectangle's upper-left, size*scale its extent. No world/yards mapping.
// Position/scale/source-size setters do NOT rebuild the cached hit rectangle;
// use Layout_WriteElementRect as well for a mod that moves/resizes input areas.
// Hover-only offsets/scale change drawing without changing cached hit bounds.
enum LayoutLoadPhase { kLayoutWaitingForFile = 0, kLayoutBuildingItems = 1,
    kLayoutReady = 2 };
#include "game/singletons.h"
#include <decomp/cw_bitfields.h>
extern "C" unsigned long Runtime_ConvertDoubleToUnsigned(double x);

// Accessor contract (all Set/GetElement definitions below): live layout and
// elements, 0<=index<elementCount, required in/out covering the stated scalar,
// pair or rectangle; inputs/outputs must not overlap the member storage in a
// way that changes later reads. No validation, retention, allocation or failure
// status unless the definition explicitly says otherwise. Stored values are
// read/written verbatim; negative geometry/scales are not clamped here.
struct LayoutElement {
    char unk0[0x4];
    int drawEnabled; // Draw enable: SetElementEnabled/initialization -> hover draw gate.
    int activationSoundEnabled; // Nonzero admits activation sound; separate from draw/player gates.
    int hitLeft; // Cached hit left, integer truncation of position.X - origin.X*scale.X.
    int hitTop; // Cached hit top, integer truncation of position.Y - origin.Y*scale.Y.
    int hitRight; // Cached hit right = integer truncation of (leftFloat + width*scale.X).
    int hitBottom; // Cached hit bottom = integer truncation of (topFloat + height*scale.Y).
    float positionX; // Base position X (layout units).
    float positionY; // Base position Y.
    float scaleX; // Dimensionless X scale.
    float scaleY; // Dimensionless Y scale.
    float sourceWidth; // Source width before scale.
    float sourceHeight; // Source height before scale.
    float originX; // Source origin X before scale.
    float originY; // Source origin Y before scale.
    float rotationRadians; // Rotation radians, Sprite getter -> geometry setter (hit AABB ignores rotation).
    int packedColor; // Packed color; opacity path multiplies ONLY low byte by t.
    int clonedItemId; // Cloned sprite/square ID when duplicate IDs exist, zero -> item.id.
    int unk48i; // Owned child resource handles; standard uses first two,
    int unk4Ci; // extended uses all four. PollLoadComplete waits on ready().
    int unk50i;
    int unk54i;
    char normalValueSelector; // Value-state byte consumed by Layout_GetElementValue.
    char hoverValueSelector; // Hover selector consumed by Layout_GetElementValue(alt!=0).
    char reserved5A[0x2];
    int playerFlags[4]; // 0x5C: one per player
}; // size 0x6C

struct LayoutItem {
    short id;
    signed char hitShapeSelector; // Zero: rectangle; nonzero: circle (hover fallback accepts 0/1).
    char unk3[0x15];
    CW_BITSP1(signed char, 7, signed char flag : 1) // 0x18
    char reserved19[0x3];
}; // size 0x1C

struct LayoutItemEx {
    short id;
    signed char hitShapeSelector; // Zero: rectangle; nonzero: circle (hover fallback accepts 0/1).
    char unk3[0xE];
    signed char flag; // 0x11
    char unk12[0x12];
}; // size 0x24

struct Layout {
    char unk0[0x4];
    void* spdRoot;
    LayoutItem* items; // 0x8
    int elementCount; // Element/item count. Standard supplied by caller; extended from resource table.
    char unk10[0x1C];
    int w2C;
    int loadPhase; // LayoutLoadPhase, written by PollLoadComplete; see transition map below.
    int allPlayersMode; // Nonzero: test all four players; zero: selected player only.
    int hoverFeedbackPending; // Update publishes 1; first hover draw consumes/reset to 0.
    char soundAdmissionBytes[0x4]; // +3C: first byte enables activation sound.
    LayoutElement* elements; // 0x40
    int w44;
    char activationActionIdBytes[0x4]; // +48: native int action ID via LayoutActionView.
    int extended; // 0x4C: nonzero selects the extended update path
    char spdResourceTable[0x4];
    LayoutItemEx* itemsEx; // 0x54: used instead of items when extended
};

// Supplemental byte views for sound admission, separate from sprite/render
// flags. The extended sound gate at +10 is NOT LayoutItemEx::flag at +11.
struct LayoutSoundAdmissionView { char prefix[0x3C]; unsigned char enabled; };
struct LayoutActionView { char prefix[0x48]; int actionId; };
struct LayoutExtendedSoundItemView { char prefix[0x10]; signed char soundGate; char suffix[0x13]; };
typedef char LayoutExtendedSoundItemSize[sizeof(LayoutExtendedSoundItemView) == 0x24 ? 1 : -1];
extern "C" {

int Layout_UpdateStandard(Layout* layout);
int Layout_UpdateExtended(Layout* layout);

// 0x80078440: called for every active layout, every frame. Returns the update result: the original tail-calls
// both helpers (b at 0x8007844C/0x80078450), and CBallWind2d_vfunc_3 (0x8030D804) uses r3 & 1.
int Layout_Update(Layout* layout) {
    if (layout->extended) {
        return Layout_UpdateExtended(layout);
    } else {
        return Layout_UpdateStandard(layout);
    }
}

// visibility).
void Layout_SetElementPlayerFlagsAndState(Layout* layout, int index, int value) {
    if (&layout->elements[index] != 0) {
        layout->elements[index].playerFlags[0] = value;
        layout->elements[index].playerFlags[1] = value;
        layout->elements[index].playerFlags[2] = value;
        layout->elements[index].playerFlags[3] = value;
    }
    layout->elements[index].drawEnabled = value;
}

// 0x8007AF6C: reads one element's flags for a player, or all players OR'd together for player 5.
void Layout_GetElementPlayerFlags(Layout* layout, int index, int* out, int player) {
    *out = 0;
    if (&layout->elements[index] == 0) {
        return;
    }
    if (player == 5) {
        *out = layout->elements[index].playerFlags[0];
        *out |= layout->elements[index].playerFlags[1];
        *out |= layout->elements[index].playerFlags[2];
        *out |= layout->elements[index].playerFlags[3];
    } else {
        *out = layout->elements[index].playerFlags[player];
    }
}

int SpdManager_GetSpriteAt(void* p, int id);

void Layout_UpdateCursorHover(Layout* layout, float t);
void Layout_UpdateCursorHoverFeedback(Layout* layout, float t);

// Draw one hover/opacity pass (t dimensionless); dispatch by record format.
// Requires a loaded layout; drawing may consume hoverFeedbackPending and update render state.
void Layout_DrawWithOpacity(Layout* layout, float t) {
    if (layout->extended) {
        Layout_UpdateCursorHover(layout, t);
    } else {
        Layout_UpdateCursorHoverFeedback(layout, t);
    }
}

// Set the element draw gate verbatim (zero disables); index uses the file-wide accessor contract.
void Layout_SetElementEnabled(Layout* layout, int index, int value) {
    layout->elements[index].drawEnabled = value;
}

// Store the secondary activation gate verbatim; sound policy requires nonzero as well as drawEnabled.
void Layout_SetElementSecondaryState(Layout* layout, int index, int value) {
    layout->elements[index].activationSoundEnabled = value;
}

// Write one required int output: the stored draw gate, without boolean normalization.
void Layout_GetElementEnabled(Layout* layout, int index, int* out) {
    *out = layout->elements[index].drawEnabled;
}

// Copy two required input floats (dimensionless XY). After loading, update the
// cached hit bounds explicitly with Layout_WriteElementRect when resizing input areas.
void Layout_SetElementScale(Layout* layout, int index, float* in) {
    layout->elements[index].scaleX = in[0];
    layout->elements[index].scaleY = in[1];
}

// Write two required floats with the stored dimensionless XY scale; no side effects.
void Layout_GetElementScale(Layout* layout, int index, float* out) {
    out[0] = layout->elements[index].scaleX;
    out[1] = layout->elements[index].scaleY;
}

// Copy two required input floats in layout XY units; this does not recompute cached hit bounds.
void Layout_SetElementPosition(Layout* layout, int index, float* in) {
    layout->elements[index].positionX = in[0];
    layout->elements[index].positionY = in[1];
}

// Write two required floats in layout XY units; no conversion or resource lookup.
void Layout_GetElementPosition(Layout* layout, int index, float* out) {
    out[0] = layout->elements[index].positionX;
    out[1] = layout->elements[index].positionY;
}

// Store two required source-space width/height floats before scaling; no hit-cache recomputation.
void Layout_SetElementSourceSize(Layout* layout, int index, float* in) {
    layout->elements[index].sourceWidth = in[0];
    layout->elements[index].sourceHeight = in[1];
}

// Write two required floats: unscaled source width/height, not the cached hit rectangle.
void Layout_GetElementSize(Layout* layout, int index, float* out) {
    out[0] = layout->elements[index].sourceWidth;
    out[1] = layout->elements[index].sourceHeight;
}

// Write one required byte output with the normal value selector used by Layout_GetElementValue.
void Layout_GetElementByte58(Layout* layout, int index, char* out) {
    *out = layout->elements[index].normalValueSelector;
}

// 0x8007B968
// Store the packed color word; opacity rendering scales only its low byte.
void Layout_SetElementColor(Layout* layout, int index, int value) {
    layout->elements[index].packedColor = value;
}

// 0x8007B97C
// Store the normal value-selector byte verbatim; later value lookup interprets it.
void Layout_SetElementValueState(Layout* layout, int index, char value) {
    layout->elements[index].normalValueSelector = value;
}

// 0x8007B9A8
// Store the hover value-selector byte verbatim; used when value lookup requests alt!=0.
void Layout_SetElementFeedbackState(Layout* layout, int index, char value) {
    layout->elements[index].hoverValueSelector = value;
}

// 0x8007BAB8
// Return a borrowed sprite handle for a sprite-kind record (flag==0), using
// clonedItemId when nonzero, otherwise the signed resource ID. Square-kind
// records return zero. Required matching item table/root/index; no retain or
// allocation, and a zero lookup result is returned unchanged.
int Layout_GetElementSprite(Layout* layout, int index) {
    int result = 0;
    int id = 0;
    int flag;
    if (layout->extended) {
        id |= layout->itemsEx[index].id;
        flag = layout->itemsEx[index].flag;
    } else {
        flag = layout->items[index].flag;
        id |= layout->items[index].id;
    }
    if (flag == 0) {
        if (layout->elements[index].clonedItemId != 0) {
            result = SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[index].clonedItemId);
        } else {
            result = SpdManager_GetSpriteAt(layout->spdRoot, (short)id);
        }
    }
    return result;
}

extern int g_layoutHitTestingEnabled;

// 0x8007BB70
// Set the process-wide hit-test gate verbatim; affects every layout and activation sound.
void Layout_SetHitTestingEnabled(int value) {
    g_layoutHitTestingEnabled = value;
}

// 0x8007BD18
// Write the sound-admission byte at required obj+3C; return 1 unconditionally.
int Layout_SetModeByte(char* obj, char value) {
    obj[0x3C] = value;
    return 1;
}

// Write two required floats with the source-space origin before scaling; no mutation.
void Layout_GetElementOrigin(Layout* layout, int index, float* out) {
    out[0] = layout->elements[index].originX;
    out[1] = layout->elements[index].originY;
}

// Copy four required integer inputs [left,top,right,bottom] to the hit cache in layout units.
void Layout_WriteElementRect(Layout* layout, int index, const int* in) {
    int a = in[0];
    int b = in[1];
    LayoutElement* e = &layout->elements[index];
    int c = in[2];
    e->hitLeft = a;
    int d = in[3];
    e->hitTop = b;
    e->hitRight = c;
    e->hitBottom = d;
}

// Store eligibility for slot 0..3, or all four for sentinel 5.
// Required live layout/index; a null computed element address causes an unchanged return.
void Layout_SetElementPlayerFlag(Layout* layout, int index, int value, int slot) {
    LayoutElement* e = &layout->elements[index];
    if (e == 0) {
        return;
    }
    if (slot == 5) {
        e->playerFlags[0] = value;
        layout->elements[index].playerFlags[1] = value;
        layout->elements[index].playerFlags[2] = value;
        layout->elements[index].playerFlags[3] = value;
    } else {
        e->playerFlags[slot] = value;
    }
}

struct LayoutSingleton {
    char data[0x20];
};
extern char s_inputManagerInitialized;
extern LayoutSingleton s_gameInputManager;
extern char s_layoutInputManagerDtorRegistration[];

extern void InputManager_Init(LayoutSingleton*);
extern void InputManager_Dtor(LayoutSingleton*);
extern void* __register_global_object(void*, void*, void*);
extern int SharedFile_Create(int, int);
extern void InputManager_SetSelectedPlayer(LayoutSingleton*, int);

LayoutSingleton* LayoutManager_GetInstance();
int Layout_HitTest(Layout*, int, int);
extern int Input_IsActionPressed(LayoutSingleton*, int, int);
extern int InputManager_GetSelectedPlayer(LayoutSingleton*);
extern void InputManager_RaiseCursorAppearance(LayoutSingleton*, int, int);
extern // Append one copied int (used for element ordinals); ptr is a required
// initialized {data,size,capacity} vector, size<=capacity, capacity<=0x3FFFFFFF.
// value is a required readable int and may alias a live vector element: read
// it before clearing/freeing old storage. Own the new array, never the value.
// Spare capacity: publish size then store. Growth: check length, allocate,
// append, copy old prefix, zero old bytes, adopt new array, then free old via
// temp's destructor. Allocator must return usable storage or throw. Length or
// allocation throw before publication preserves vector/value; no rollback for
// invalid pointers. Volatile reloads and temporary lifetime retain EH behavior.
void LayoutIntVector_PushBack(void*, int*);
extern // Append one copied player ordinal to the optional activation companion
// vector; same initialized-vector/aliasing/ownership/length-failure contract as
// LayoutIntVector_PushBack. Does not coordinate a transaction with that vector:
// CollectActivated has already appended the index if this allocation throws.
// Growth stages: checked capacity -> append into new storage -> copy/zero old
// prefix -> publish new storage/count -> guard releases old allocation.
void LayoutActivatedVector_PushBack(void*, int*);
#pragma push
#pragma use_lmw_stmw on
// Append each (element,player) activation in element-major/player-minor order.
// indices is a required initialized LayoutIntVector; players is optional and,
// if present, a distinct initialized vector. Neither vector is cleared. Their
// existing lengths must agree if pairs are needed. Input must be sampled first.
// All-player mode tests players 0..3; selected mode admits action(-1) then hit
// tests selected player. A hit raises cursor appearance 3; returns 1 iff any.
// Allocation/length errors propagate; index append precedes player append, so
// failure can leave unequal output lengths and prior cursor effects. No rollback.
int Layout_CollectActivated(Layout* layout, void* indices, void* players) {
    int index;
    int singlePlayer;
    int result = 0;
    LayoutSingleton* input = LayoutManager_GetInstance();
    for (index = 0; (unsigned)index < (unsigned)layout->elementCount; ++index) {
        if (layout->allPlayersMode) {
            for (int player = 0; player < 4; ++player) {
                if (Input_IsActionPressed(input, *(int*)layout->activationActionIdBytes, player) && Layout_HitTest(layout, index, player)) {
                    LayoutIntVector_PushBack(indices, &index);
                    if (players) {
                        int value = player;
                        LayoutActivatedVector_PushBack(players, &value);
                    }
                    InputManager_RaiseCursorAppearance(input, 3, player);
                    result = 1;
                }
            }
        } else {
            singlePlayer = InputManager_GetSelectedPlayer(input);
            if (Input_IsActionPressed(input, *(int*)layout->activationActionIdBytes, -1) && Layout_HitTest(layout, index, singlePlayer)) {
                LayoutIntVector_PushBack(indices, &index);
                if (players) LayoutActivatedVector_PushBack(players, &singlePlayer);
                InputManager_RaiseCursorAppearance(input, 3, singlePlayer);
                result = 1;
            }
        }
    }
    return result;
}
#pragma pop
// Lazily construct/register the process-owned input manager, then return a
// borrowed singleton. Callers never delete it; initialization is unsynchronized.
LayoutSingleton* LayoutManager_GetInstance() {
    if (((LayoutSingleton *&)Sing_805DC5C8()) == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_layoutInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((LayoutSingleton *&)Sing_805DC5C8()) = &s_gameInputManager;
    }
    return ((LayoutSingleton *&)Sing_805DC5C8());
}

struct LayoutCounter {
    char pad[0x3C8];
    int count;
};
extern LayoutCounter lbl_804E5FC8c;

// Change all-player participation and adjust the singleton's active-layout
// count only on a value change. Required layout; callers should pass 0/1 and
// balance activation/deactivation before destruction. May initialize singleton;
// no ownership transfer, resource loading, or failure status.
void Layout_SetActive(Layout* layout, int value) {
    if (layout->allPlayersMode != value) {
        if (value != 0) {
            if (((LayoutSingleton *&)Sing_805DC5C8()) == 0) {
                if (s_inputManagerInitialized == 0) {
                    InputManager_Init(&s_gameInputManager);
                    __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_layoutInputManagerDtorRegistration);
                    s_inputManagerInitialized = 1;
                }
                ((LayoutSingleton *&)Sing_805DC5C8()) = &s_gameInputManager;
            }
            ((LayoutCounter*)((LayoutSingleton *&)Sing_805DC5C8()))->count++;
        } else {
            if (((LayoutSingleton *&)Sing_805DC5C8()) == 0) {
                if (s_inputManagerInitialized == 0) {
                    InputManager_Init(&s_gameInputManager);
                    __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_layoutInputManagerDtorRegistration);
                    s_inputManagerInitialized = 1;
                }
                ((LayoutSingleton *&)Sing_805DC5C8()) = &s_gameInputManager;
            }
            ((LayoutCounter*)((LayoutSingleton *&)Sing_805DC5C8()))->count--;
        }
        layout->allPlayersMode = value;
    }
}

// Start extended-format loading: reset count/readiness/hover state, acquire
// a shared-file handle from name and select player arg (0..3). Requires a fresh
// or previously released layout; does not release an old file/elements/root.
// Return 1 means request recorded; PollLoadComplete plus UpdateExtended finish
// loading. No rollback or validation of SharedFile_Create's handle result.
int Layout_LoadExtended(Layout* layout, int arg, int name) {
    layout->extended = 1;
    layout->elementCount = 0;
    int h = SharedFile_Create(name, 0);
    LayoutSingleton* s = ((LayoutSingleton *&)Sing_805DC5C8());
    layout->loadPhase = 0;
    layout->hoverFeedbackPending = 0;
    layout->w44 = h;
    if (s == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_layoutInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((LayoutSingleton *&)Sing_805DC5C8()) = &s_gameInputManager;
    }
    InputManager_SetSelectedPlayer(((LayoutSingleton *&)Sing_805DC5C8()), arg);
    return 1;
}

// Write four required ints [left,top,right,bottom] from the cached hit bounds
// in layout units and return 1. Null computed element address returns 0 with
// output unchanged; otherwise the common live-layout/index contract applies.
int Layout_ReadElementRect(Layout* layout, int index, int* out) {
    LayoutElement* e = &layout->elements[index];
    if (e == 0) {
        return 0;
    }
    out[0] = e->hitLeft;
    out[1] = e->hitTop;
    out[2] = e->hitRight;
    out[3] = e->hitBottom;
    return 1;
}

extern const float kHitRectangleOffset;

// 0x8007BE64
// Geometry-only rectangle test at cached player slot 0..3, with strict edges
// and shared -4 layout-unit offset on every bound. Required element/cache;
// ignores global gate, player eligibility, draw state and item hit-shape flag.
// Return 0/1 without mutation; circle testing is a separate entry point.
int Layout_HitTestPointerPointInElement(Layout* layout, int index, int slot) {
    int a;
    int b;
    float k = kHitRectangleOffset;
    int c; int d;  layout = layout;
// The original reads the bounds even when the element load is skipped (it uses the stale registers).

    if (index + layout->elements != 0) {

        a = layout->elements[index].hitLeft;
        b = layout->elements[index].hitTop;
        c = layout->elements[index].hitRight;
        d = layout->elements[index].hitBottom;
    }
    float* pt = (float*)((char*)layout + (8 * slot));
    if ((float)a + k < pt[4]) { if ((float)b + k < pt[5]) { if ((float)c + k > pt[4]) { if ((float)d + k > pt[5]) {
        return 1;
    } } } }
    return 0;
}

extern float Math_SquareRootApprox(float);
extern const float kSoundMinimumGain;

// 0x8007BD24
// Geometry-only inclusive circle test for player 0..3. Missing root returns
// 0; otherwise require valid cached bounds. Center is rectangle midpoint,
// radius is abs(width/2), ignoring height for radius. Apply shared -4 offset
// to center-minus-pointer deltas and use the existing square-root approximation.
// Ignore global/eligibility/draw gates; no cache/resource/ownership changes.
int Layout_HitTestElementCircle(Layout* layout, int index, int slot) {
    int a, c, b, d;
    struct P { float x, y; };
    if (layout->spdRoot == 0) {
        return 0;
    }
    P ctr = {0, 0};
    P unusedP = {0, 0};
    float k = kHitRectangleOffset;
    LayoutElement* e = layout->elements + index;
    if (e != 0) {
        a = e->hitLeft;
        b = e->hitTop;
        c = e->hitRight;
        d = e->hitBottom;
    }

    float* pt = (float*)((char*)layout + slot * 8);
    float hw = (float)(c - a) / 2.0f;
    ctr.x = (float)(c + a) / 2.0f;
    ctr.y = (float)(b + d) / 2.0f;

    float dx = k + (ctr.x - pt[4]);
    float dy = k + (ctr.y - pt[5]);
float r = Math_SquareRootApprox(hw * hw);
    float dist = Math_SquareRootApprox(dx * dx + dy * dy);
    if (dist <= r) return 1;
    return 0;
}

// 0x8007C08C
// Geometry-only test using item.hitShapeSelector: zero = strict rectangle, nonzero = circle.
// Required matching item table/live element and player slot 0..3. Shared -4
// offset applies; no global/eligibility/draw checks or writes. Returns 0/1.
int Layout_HitTestElement(Layout* layout, int index, int slot) {
    int a;
    unsigned int g;
    int b;
     int c; int d;
    if (!(layout->extended)) {
        g = layout->items[index].hitShapeSelector;
    } else {
        g = layout->itemsEx[index].hitShapeSelector;
    }
    if (g) {
        return Layout_HitTestElementCircle(layout, index, slot);
    }
    float k = kHitRectangleOffset;
    if (index + layout->elements != 0) {
        a = layout->elements[index].hitLeft;
        b = layout->elements[index].hitTop;
        c = layout->elements[index].hitRight;
        d = layout->elements[index].hitBottom;
    }
    float* pt = (float*)((char*)layout + (8 * slot));
    if ((float)a + k < pt[4]) { if ((float)b + k < pt[5]) { if ((float)c + k > pt[4] && (float)d + k > pt[5]) {
        return 1;
    } } }
    return 0;
}

// 0x8007BB78
// Resolve selected player (may initialize singleton), then perform the same
// geometry-only shape test as HitTestElement. Required loaded tables/bounds;
// ignores global/eligibility/draw gates. index becomes an integerized element
// address; slot becomes its eight-byte cursor offset. Those phases are ABI
// scratch, never persistent indices or owned pointers. Return 0/1.
int Layout_HitTestElementForSelectedPlayer(Layout* layout, int index) {
    int a;
    int slot = InputManager_GetSelectedPlayer(LayoutManager_GetInstance());
    int d;
    int b;
    int c;
    int g;

    if (layout->extended) g = layout->itemsEx[index].hitShapeSelector;
    else g = layout->items[index].hitShapeSelector;
    if (g) return Layout_HitTestElementCircle(layout, index, slot);
    float k = kHitRectangleOffset;
    index = (int)(*(LayoutElement* volatile*)&layout->elements + index);
    if (index) {
        a = ((LayoutElement*)index)->hitLeft;
        b = ((LayoutElement*)index)->hitTop;
        c = ((LayoutElement*)index)->hitRight;
        d = ((LayoutElement*)index)->hitBottom;
    }
    __typeof__(slot) _p35 = slot;
    __typeof__(8 * _p35) _p216 = _p35 * 8;
    __typeof__(_p216) _p3 = _p35 * 8;
    int slotValue = slot;
    __typeof__(8 * _p35) _p205 = _p35 * 8;
    __typeof__(_p35 * 8) _p430 = 8 * _p35;
    __typeof__(_p35 * 8) _p89 = _p35 * 8;
    __typeof__(_p89) _p437 = _p35 * 8;
    __typeof__(_p437) _p740 = _p35 * 8;
    slot = _p430;
    float* pt = (float*)((char*)layout + slot);
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}

// 0x8007C1B0
// Test selected player with BOTH global hit gate and per-player eligibility,
// then item.hitShapeSelector shape geometry. May initialize singleton; draw state is ignored.
// Required loaded tables/bounds and selected player 0..3. index is reused as
// bottom edge and slot as cursor byte offset after lookup; no output mutation.
int Layout_HitTestPlayer(Layout* layout, int index) {
    unsigned int slot = InputManager_GetSelectedPlayer(LayoutManager_GetInstance());

    int g = 0; int a; int b; int c;
    if (g_layoutHitTestingEnabled == 0) return 0;
    if (layout->extended) g |= layout->itemsEx[index].hitShapeSelector;
    else g |= layout->items[index].hitShapeSelector;
    if (!layout->elements[index].playerFlags[slot]) return 0;
    if (g) return Layout_HitTestElementCircle(layout, index, slot);
    float k = kHitRectangleOffset;
    LayoutElement* e = *(LayoutElement* volatile*)&layout->elements + index;
    if (e) {
        a = e->hitLeft;
        b = e->hitTop;
        c = e->hitRight;
        index = e->hitBottom;
    }
    slot = slot * 8;
    float* pt = (float*)((char*)layout + slot);
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)index + k > pt[5]) return 1;
    return 0;
}

// 0x8007B9BC
// Borrow the child-resource handle selected for rendering: standard alt==0
// uses +0x48, alt!=0 uses +0x4C. Extended mode uses the corresponding normal/
// hover selector and MUST constrain it to 0..3, selecting +48/+4C/+50/+54.
// Invalid extended selectors fall off this reconstructed nonvoid function;
// there is no failure sentinel or retain. Null child handles may be returned.
int Layout_GetElementValue(Layout* layout, int index, int alt) {
    if (layout->extended) {
        if (alt) {
            LayoutElement* e = &layout->elements[index];
            switch (e->hoverValueSelector) {
            case 0: return e->unk48i;
            case 1: return e->unk4Ci;
            case 2: return e->unk50i;
            case 3: return e->unk54i;
            }
        } else {
            LayoutElement* e = &layout->elements[index];
            switch (e->normalValueSelector) {
            case 0: return e->unk48i;
            case 1: return e->unk4Ci;
            case 2: return e->unk50i;
            case 3: return e->unk54i;
            }
        }
    } else {
        if (alt) {
            return layout->elements[index].unk4Ci;
        }
        return layout->elements[index].unk48i;
    }
}

extern void* __nwa__FUl(int);
extern void* __construct_new_array(void*, void*, void*, int, int);
extern void CSPDPangyaIcon_PangyaIconObject_ctor();
extern void CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor();

struct LayoutElementObj : LayoutElement {
    LayoutElementObj();
    ~LayoutElementObj();
};

// 0x80077F74
#pragma push
#pragma use_lmw_stmw on
// Initialize a fresh/released standard layout. a8 is a borrowed Wii pointer
// word to count LayoutItem records (must outlive layout), count>=0, name selects
// shared-file resource, arg selects player 0..3. Construct an owned element
// array (array cookie + count*0x6C bytes), store the file handle, reset readiness
// and select player. Existing storage is not released. Allocation exceptions
// propagate; null allocation/file failure is not represented by return status:
// returns 1 and later update/poll assumes valid resources. Not synchronous load.
int Layout_InitStandard(Layout* layout, int arg, int a8, int count, int name) {
    layout->extended = 0;
    layout->elementCount = count;
    layout->items = (LayoutItem*)(long)a8;
    void* storage = __nwa__FUl(count * sizeof(LayoutElementObj) + 16);
    layout->elements = storage ? (LayoutElement*)__construct_new_array(storage,
        (void*)CSPDPangyaIcon_PangyaIconObject_ctor, (void*)CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor,
        sizeof(LayoutElementObj), count) : 0;
    int h = SharedFile_Create(name, 0);
    layout->w44 = h;
    layout->loadPhase = 0;
    layout->hoverFeedbackPending = 0;
    if (((LayoutSingleton *&)Sing_805DC5C8()) == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_layoutInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((LayoutSingleton *&)Sing_805DC5C8()) = &s_gameInputManager;
    }
    InputManager_SetSelectedPlayer(((LayoutSingleton *&)Sing_805DC5C8()), arg);
    return 1;
}
#pragma pop

// Activation/collection geometry with global gate and player eligibility;
// unlike HitTestElement, this can reject an otherwise geometrically valid hit.
// Required loaded item/element arrays and slot 0..3; draw state is ignored.
// index phases: element ordinal -> integerized element address -> slot*8 byte
// offset. Keep those uses in order; output is only the 0/1 hit result.
int Layout_HitTest(Layout* layout, int index, int slot) { int a;  int b;  int c;       int g = 0; int d;

    if (0 == g_layoutHitTestingEnabled) return 0;
    if (layout->extended) g |= layout->itemsEx[index].hitShapeSelector;
    else g |= layout->items[index].hitShapeSelector;
    if (0 == layout->elements[index].playerFlags[slot]) return 0;
    if (0 != g) return Layout_HitTestElementCircle(layout, index, slot);
    index = (int)(*(LayoutElement* volatile*)&layout->elements + index);
    float k = kHitRectangleOffset;
    if (index) { a = ((LayoutElement*)index)->hitLeft; b = ((LayoutElement*)index)->hitTop; c = ((LayoutElement*)index)->hitRight; d = ((LayoutElement*)index)->hitBottom; }

    index = (8 * slot);
    float* pt = (float*)((char*)layout + index);
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}
extern int CSharedFilePtr_GetLoadedBuffer(int);
extern void* Table_ResolveDataOffset(void*, int);
extern signed char* OffsetStringNode_GetText(void*);
struct LayoutReadyObject { 
virtual void pad0(); virtual void pad1(); virtual void pad2(); virtual void pad3(); virtual int ready();
};
// Call required borrowed child handle's readiness virtual slot 4; return its
// status verbatim. No reference transfer; zero means pending to the poller.
inline int LayoutReadyCall(int value) {
    LayoutReadyObject* obj = (LayoutReadyObject*)value;
    return obj->ready();
}
// Resolve extended item's resource-name text for readiness admission.
// index is a BYTE OFFSET (0x24*item ordinal), field is word ordinal 5..8.
// Returned text is borrowed from the table. Missing entry returns null;
// PollLoadComplete dereferences it, so valid poll inputs must resolve text.
inline signed char* LayoutReadyFlag(Layout* layout, int index, int field) {
    void* entry = Table_ResolveDataOffset(*(void**)layout->spdResourceTable, ((int*)((char*)layout->itemsEx + index))[field]);
    return entry ? OffsetStringNode_GetText(entry) : 0;
}
#pragma push
#pragma use_lmw_stmw on
// Advance asynchronous readiness without constructing resources. Required
// initialized layout, shared-file handle, and valid optional-child descriptors.
// phase 0: pending file -> return 0; loaded -> set phase 1, still return 0.
// phase 1: absent root -> return 0; otherwise poll requested nonnull children
// (standard two / extended four), set phase 2 and return 1 only when all ready.
// A requested null child does not block readiness. Other phases return 1.
// Table/text references must be valid: missing extended text is not an error
// result. No allocation, release or rollback; child ready callbacks may run.
// Pair with UpdateStandard/Extended, which construct resources during phase 1.
int Layout_PollLoadComplete(Layout* layout) {
    unsigned exoff, elemoff, itemoff;
    int result;
    unsigned i;
    int state = layout->loadPhase;
    switch (state) {
    case 0:
        if (!CSharedFilePtr_GetLoadedBuffer(layout->w44)) { result = 0; break; }
        layout->loadPhase = (result = 0) + 1; break;
    case 1:
        if (!layout->spdRoot) { result = 0; break; }
        result = 1;
        for (i = 0, exoff = 0, elemoff = 0, itemoff = 0; i < (unsigned)layout->elementCount; exoff += 0x24, elemoff += 0x6c, itemoff += 0x1c, ++i) {
            if (layout->extended) {
                if (*LayoutReadyFlag(layout, exoff, 5)) {
                    int obj = ((LayoutElement*)((char*)layout->elements + elemoff))->unk48i;
                    if (obj && !LayoutReadyCall(obj)) result = 0;
                }
                if (*LayoutReadyFlag(layout, exoff, 6)) {
                    int obj = ((LayoutElement*)((char*)layout->elements + elemoff))->unk4Ci;
                    if (obj && !LayoutReadyCall(obj)) { result = 0; break; }
                }
                if (*LayoutReadyFlag(layout, exoff, 7)) {
                    int obj = ((LayoutElement*)((char*)layout->elements + elemoff))->unk50i;
                    if (obj && !LayoutReadyCall(obj)) { result = 0; break; }
                }
                if (*LayoutReadyFlag(layout, exoff, 8)) {
                    int obj = ((LayoutElement*)((char*)layout->elements + elemoff))->unk54i;
                    if (obj && !LayoutReadyCall(obj)) { result = 0; break; }
                }
            } else {
                if (((int*)((char*)layout->items + itemoff))[1]) {
                    int obj = ((LayoutElement*)((char*)layout->elements + elemoff))->unk48i;
                    if (obj && !LayoutReadyCall(obj)) result = 0;
                }
                if (((int*)((char*)layout->items + itemoff))[2]) {
                    int obj = ((LayoutElement*)((char*)layout->elements + elemoff))->unk4Ci;
                    if (obj && !LayoutReadyCall(obj)) { result = 0; break; }
                }
            }
        }
        if (result) layout->loadPhase = 2;
        break;
    default:
        result = 1;
    }
    return result;
}
#pragma pop

struct LayoutObject {
    virtual void v0();
    virtual void v1();
};

void UIRender_SetDefaultState(Layout* layout);
int SpdManager_GetSquareAt(void* p, int id);
void Sprite_SetPosition2D(void* o, float* p);
void Sprite_SetOrigin2D(void* o, float* p);
void Sprite_SetScale2D(void* o, float* p);
void Sprite_SetSourceSize2D(void* o, float* p);
void Sprite_SetPackedColor(void* o, unsigned int color);
void SpdGeometry_SetRotationRadians(void* o, float v);
void TextureRegion_SetSourceAndNormalizeBounds(void* o, int v);
void SimpleSpd_DrawBpSprite(void* o);

extern const float lbl_805DD984;
extern float kFloat_1_LayoutUnit;

// Mixes an element colour's alpha byte toward the element's own alpha by t (0..1): 0 below 0, the colour above 1.
static inline unsigned int LayoutFade(unsigned int color, float t) {
    unsigned int a;
    if (t <= lbl_805DD984) {
        a = 0;
    } else if (t >= kFloat_1_LayoutUnit) {
        a = color;
    } else {
        a = (color & 0xFF00) | (unsigned char)Runtime_ConvertDoubleToUnsigned((float)(unsigned char)color * t) | (color & 0xFFFF0000);
    }
    return a;
}

// 0x8007AAA0: sets up one extended element's widget (transform, color, parent) and submits it.
// Draw one element, requiring valid itemsEx even though child lookup contains
// a standard-table fallback. Missing root or out-of-range index returns before
// render-state changes. Otherwise require nonnull borrowed sprite/square lookup
// results and valid normal selector. Ignore drawEnabled here: caller chooses
// admission. Mutate object transform/color/texture region, run virtual slot 1
// and submit it; t is dimensionless opacity. No ownership transfer or status.
void Layout_DrawElementWithOpacity(Layout* layout, unsigned int index, float t) {
    if (layout->spdRoot == 0) {
        return;
    }
    if (index >= (unsigned int)layout->elementCount) {
        return;
    }
    UIRender_SetDefaultState(layout);
    float uv[2] = {0.0f, 0.0f}; // kept from the original: the pair is initialised but never read
    (void)uv;
    LayoutObject* obj;
    if (layout->itemsEx[index].flag == 0) {
        if (layout->elements[index].clonedItemId != 0) {
            obj = (LayoutObject*)SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[index].clonedItemId);
        } else {
            obj = (LayoutObject*)SpdManager_GetSpriteAt(layout->spdRoot, layout->itemsEx[index].id);
        }
        Sprite_SetPosition2D(obj, &layout->elements[index].positionX);
        Sprite_SetOrigin2D(obj, &layout->elements[index].originX);
        Sprite_SetScale2D(obj, &layout->elements[index].scaleX);
        Sprite_SetSourceSize2D(obj, &layout->elements[index].sourceWidth);
        Sprite_SetPackedColor(obj, LayoutFade(layout->elements[index].packedColor, t));
        SpdGeometry_SetRotationRadians(obj, layout->elements[index].rotationRadians);
        if (layout->spdRoot != 0) {
            int v = Layout_GetElementValue(layout, index, 0);
            void* child;
            if (layout->elements[index].clonedItemId != 0) {
                child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[index].clonedItemId);
            } else if (layout->extended) {
                child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->itemsEx[index].id);
            } else {
                child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->items[index].id);
            }
            TextureRegion_SetSourceAndNormalizeBounds(child, v);
        }
        obj->v1();
        SimpleSpd_DrawBpSprite(obj);
    } else {
        if (layout->elements[index].clonedItemId != 0) {
            obj = (LayoutObject*)SpdManager_GetSquareAt(layout->spdRoot, layout->elements[index].clonedItemId);
        } else {
            obj = (LayoutObject*)SpdManager_GetSquareAt(layout->spdRoot, layout->itemsEx[index].id);
        }
        Sprite_SetPosition2D(obj, &layout->elements[index].positionX);
        Sprite_SetOrigin2D(obj, &layout->elements[index].originX);
        Sprite_SetScale2D(obj, &layout->elements[index].scaleX);
        Sprite_SetSourceSize2D(obj, &layout->elements[index].sourceWidth);
        Sprite_SetPackedColor(obj, LayoutFade(layout->elements[index].packedColor, t));
        SpdGeometry_SetRotationRadians(obj, layout->elements[index].rotationRadians);
        obj->v1();
        SimpleSpd_DrawBpSprite(obj);
    }
}

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[], s_layoutStringPool[];
extern unsigned long strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* memcpy(void*, const void*, unsigned long);
extern void* memset(void*, int, unsigned long);
extern void* __nw__FUl(unsigned long);
extern void* appMallocDummy__FUl(unsigned long);
extern void appFreeUni__FPv(void*);
extern void __throw(void*, void*, void*);
extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct LayoutVectorAllocation : std::detail::__message_buffer_guard {
    // Guard exception-message bytes until the shared message owner adopts them.
    LayoutVectorAllocation(char* p, char* a) : std::detail::__message_buffer_guard(p, a) {}
    // Dispose unadopted exception-message storage on return/unwind.
    ~LayoutVectorAllocation() { dispose(); }
};
typedef std::detail::__message_control LayoutVectorControl;
struct LayoutVectorErrorBase_layout {
    void* table;
    // Initialize the standard logic_error base vtable before attaching its message.
    LayoutVectorErrorBase_layout() { table = __vt__Q23std11logic_error; }
    // Tear down the exception base without deleting its enclosing stack object.
    ~LayoutVectorErrorBase_layout() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : LayoutVectorErrorBase_layout {
    ::std::__error_message<LayoutVectorAllocation, LayoutVectorControl> message_;
    // Own a copy of the shared vector-length error text and install the derived vtable.
    length_error() {
        const char* message = s_layoutStringPool;
        message_.assign(message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
} // extern C
// Placement construction borrows supplied storage; never allocates or owns it.
inline void* operator new(new_size_t, void* p) { return p; }
extern "C" {
// Build a length_error and transfer its cleanup to the exception runtime; never returns.
inline void LayoutVectorThrow() {
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct();
        exception_runtime::__throw(exception_runtime::s_layoutStringPool + 0x14, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
struct LayoutIntVector { int* volatile data; unsigned size, capacity; };
struct LayoutVectorBuffer {
    int* data; volatile unsigned size; unsigned capacity; unsigned* owner; volatile unsigned offset;
    // Start with no array; borrow the owner capacity word for the growth transaction.
    LayoutVectorBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free whichever allocation remains guard-owned (old array after publication).
    ~LayoutVectorBuffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
struct LayoutHoverBuffer {
    int* data; unsigned size; unsigned capacity; unsigned* owner; unsigned offset;
    // Nonvolatile growth guard for hover append; no allocation at construction.
    LayoutHoverBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free the guarded array on normal return or unwinding; never free owner metadata.
    ~LayoutHoverBuffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Select a borrowed capacity candidate by unsigned comparison; ties keep a.
inline const unsigned& LayoutVectorMax(const unsigned& a, const unsigned& b) { return a < b ? b : a; }

#pragma push
#pragma use_lmw_stmw on
// Only explicit volatile accesses below require reloads; ordinary size/
// capacity arithmetic is unsigned. Unevaluated typeof/unused snapshots
// have been removed after checking the emitted code and EH entries.
void LayoutIntVector_PushBack(void* ptr, int* value) {
    LayoutIntVector* v = (LayoutIntVector*)ptr;
    unsigned int size = v->size;

    if (size < v->capacity) {
        int next = 1 + size;
        int item = *value;
        v->size = next;
        v->data[next - 1] = item;
    } else { unsigned int count;
        if (0x3FFFFFFFu - v->capacity < 1) LayoutVectorThrow();

        unsigned needed = *(volatile unsigned*)&v->size + 1 - *(volatile unsigned*)&v->capacity;
        LayoutVectorBuffer allocationGuard(&v->capacity);
        unsigned capacity = v->capacity;
        if (needed > 0x3FFFFFFFu - capacity) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::s_layoutStringPool + 0x14, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        if (capacity < 0x15555555u) {
            capacity += LayoutVectorMax((3 * (capacity + 1)) / 5, needed);
        } else if (capacity < 0x2AAAAAAAu) {
            unsigned largeCapacity = capacity;
            unsigned halfGrowthNumerator = 1 + largeCapacity;
            capacity += LayoutVectorMax((halfGrowthNumerator) / 2, needed);
        } else capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::s_layoutStringPool + 0x14, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        unsigned capacityValue2 = capacity;
        int* data = (int*)appMallocDummy__FUl(capacityValue2 * 4);
        unsigned int oldSize = v->size;
        allocationGuard.offset = oldSize;

        ((oldSize + data))[allocationGuard.size++] = *value;
        unsigned sizeBeforeMove = v->size; int* oldData = v->data;

        int* oldEnd = sizeBeforeMove + oldData;
        count = ((char*)(oldEnd) - (char*)oldData) / 4;
        unsigned capacityValue3 = capacity;
        allocationGuard.capacity = capacityValue3;

        allocationGuard.data = data;
        unsigned int countValue = count;
        int offset = oldSize - countValue;
        int offsetValue = offset;
        allocationGuard.offset = offsetValue;

        unsigned countForBytes = count;
        unsigned bytesToMove = countForBytes * 4;
        int bytes;

        bytes = bytesToMove;

        memcpy((data + offset), oldData, bytes);
        memset(oldData, 0, bytes);
        unsigned int newSize = (allocationGuard.size + count);

        oldData = v->data;
        unsigned capacityValue4 = v->capacity;

        data = allocationGuard.data;
        allocationGuard.size = newSize;
        allocationGuard.data = oldData;
        v->size = 0;
        v->capacity = allocationGuard.capacity;
        allocationGuard.size = 0;
        v->data = data;

        v->size = newSize;
        allocationGuard.capacity = (capacityValue4);

    }
}
#pragma pop
#pragma push
#pragma use_lmw_stmw on
void LayoutActivatedVector_PushBack(void* ptr, int* value) {

    LayoutIntVector* v = (LayoutIntVector*)ptr;
    unsigned int size = v->size;

    if (size < (v->capacity)) {
        int item;
        int next = 1 + size;

        item = *value;
        v->size = next;
        v->data[next - 1] = item;
    } else { __typeof__(v->capacity) capacityAtGrowthEntry = v->capacity; unsigned capacityValue5 = v->capacity; int count; __typeof__(v->size) sizeAtGrowthEntry = v->size;
        if (0x3FFFFFFFu - v->capacity < 1) LayoutVectorThrow();
        __typeof__(sizeAtGrowthEntry + 1) nextSizeFromEntryRead = 1 + sizeAtGrowthEntry;
        __typeof__(sizeAtGrowthEntry + 1) nextSizeFromEntryReadAgain = sizeAtGrowthEntry + 1;
        __typeof__(v->size) sizeBeforeGuard = v->size;
        unsigned capacityValue6 = v->capacity;
        __typeof__(sizeBeforeGuard + 1) nextSizeBeforeGuard = sizeBeforeGuard + 1;
        __typeof__(v->size) sizeGuardRead = v->size;
        __typeof__(v->size) sizeGuardReadAgain = v->size;
        __typeof__(sizeGuardReadAgain + 1) nextSizeGuardRead = sizeGuardReadAgain + 1;
        __typeof__(nextSizeGuardRead) nextSizeGuardReadAgain = sizeGuardReadAgain + 1;
        __typeof__(v->size) sizeBeforeAllocation = v->size;
        __typeof__(nextSizeBeforeGuard) nextSizeBeforeAllocation = 1 + sizeBeforeAllocation;
        LayoutVectorBuffer allocationGuard(&v->capacity);
        unsigned needed = *(volatile unsigned*)&v->size + 1 - *(volatile unsigned*)&v->capacity;
        unsigned capacity = v->capacity;
        if (needed > 0x3FFFFFFFu - capacity) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::s_layoutStringPool + 0x14, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        if (capacity < 0x15555555u) {
            __typeof__(capacity) smallCapacity = capacity;
            capacity += LayoutVectorMax((3 * (1 + smallCapacity)) / 5, needed);
        } else if (capacity < 0x2AAAAAAAu) {
            __typeof__(capacity) largeCapacity = capacity;
            __typeof__(1 + largeCapacity) halfGrowthNumerator = largeCapacity + 1;
            __typeof__(halfGrowthNumerator) halfGrowthNumeratorAgain = 1 + largeCapacity;
            __typeof__(halfGrowthNumeratorAgain) halfGrowthNumeratorFinal = largeCapacity + 1;
            capacity += LayoutVectorMax((halfGrowthNumeratorFinal) / 2, needed);
        } else capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu) {
            union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::s_layoutStringPool + 0x14, error,
                (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        unsigned capacityValue7 = capacity;
        int* data = (int*)appMallocDummy__FUl(capacityValue7 * 4);
        int oldSize = v->size;
        allocationGuard.offset = oldSize;
        __typeof__(v->size) sizeBeforeAppend = v->size;
        ((data + oldSize))[allocationGuard.size++] = *value;

        __typeof__(sizeBeforeAppend) sizeBeforeMove = v->size;

        int* oldData = v->data; count = ((char*)(oldData + sizeBeforeMove) - (char*)oldData) / 4;
        int countValue2 = count;
        unsigned capacityValue8 = capacity;
        allocationGuard.capacity = capacityValue8;
        allocationGuard.data = data;
        unsigned offset = oldSize - countValue2;
        unsigned offsetValue2 = offset;
        __typeof__(count) countForBytes = count;
        __typeof__(4 * countForBytes) bytesToMove = 4 * countForBytes;
        int bytes;
        bytes = bytesToMove;

        allocationGuard.offset = offsetValue2;
        unsigned offsetValue3 = offset;
        memcpy(offsetValue3 + data, oldData, bytes);
        memset(oldData, 0, bytes);
        __typeof__(count) countAfterMove = count;
        int countValue3 = count;
        __typeof__(countAfterMove + allocationGuard.size) sizeAfterMove = allocationGuard.size + countAfterMove;
        int newSize = sizeAfterMove;
        __typeof__(v->capacity) capacityBeforePublish = v->capacity;
        oldData = v->data;
        allocationGuard.size = newSize;
        unsigned capacityValue9 = v->capacity;
        v->size = 0;
        data = allocationGuard.data;
        v->data = data;
        allocationGuard.data = oldData;
        v->capacity = allocationGuard.capacity;
        allocationGuard.capacity = (capacityValue9);
        v->size = newSize;
        allocationGuard.size = 0;

    }
}

#pragma pop
extern void* g_soundManager;
extern float kSoundAttenuationRange, kFloat_1_LayoutUnit;
extern void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
// Sound geometry adapter: required live layout/index/player 0..3. Nonzero
// circleHitShape delegates to circle hit test; otherwise load cached rectangle
// into caller scratch and compare strict interior at playerCursor[4/5]. The
// cursor is layout+8*player, so those words are layout+0x10+8*player XY. Bounds
// scratch persists only to retain original load/register lifetimes; no ownership.
inline int Layout_SoundHit(Layout* layout, int index, int player, int& circleHitShape, float rectangleOffset, float* playerCursor, int& left, int& top, int& right, int& bottom) {
    if (circleHitShape) return Layout_HitTestElementCircle(layout, index, player);
    LayoutElement* e = layout->elements + index;
    if (e) { left = e->hitLeft; top = e->hitTop; right = e->hitRight; bottom = e->hitBottom; }

    if ((float)left + rectangleOffset < playerCursor[4] && (float)top + rectangleOffset < playerCursor[5] && (float)right + rectangleOffset > playerCursor[4] && (float)bottom + rectangleOffset > playerCursor[5]) return 1;
    return 0;
}
// Same geometry contract as Layout_SoundHit, forming the selected player's
// cursor internally. Only rectangle scratch outputs are written; no state change.
inline int Layout_SoundHitSingle(Layout* layout, int index, int player, int& circleHitShape, float rectangleOffset, int& left, int& top, int& right, int& bottom) {
    if (circleHitShape) return Layout_HitTestElementCircle(layout, index, player);
    LayoutElement* e = layout->elements + index;
    if (e) { left = e->hitLeft; top = e->hitTop; right = e->hitRight; bottom = e->hitBottom; }

    float* playerCursor = (float*)((char*)layout + player * 8);
    if ((float)left + rectangleOffset < playerCursor[4] && (float)top + rectangleOffset < playerCursor[5] && (float)right + rectangleOffset > playerCursor[4] && (float)bottom + rectangleOffset > playerCursor[5]) return 1;
    return 0;
}
// One sound at most per update. Required loaded layout and sampled input.
// Admission: layout+3C low byte and global hit-testing gate must both be nonzero.
// Per-element hit shape is item.hitShapeSelector (zero rectangle, nonzero circle). Sound gate
// is a DIFFERENT format view: extended signed byte +10; standard second bit
// of byte +18 (0x40). Do not substitute the sprite-kind flag accessor (+11 for
// extended) or merge render/hover bitfields: those have different consumers.
// After an action AND geometry hit, require secondary state (+8), sound gate,
// and drawEnabled. playerFlags[player]!=0 selects eligible feedback, zero
// selects ineligible feedback. All-player mode ORs four players; selected mode
// first requires action(-1), then action(selectedPlayer). Every element is
// inspected; any eligible hit wins over ALL ineligible hits. Neither -> silence.
// Rectangle offset kHitRectangleOffset is -4 layout units, applied to EVERY bound:
// changing it translates the rectangle in X and Y, without widening it. This
// shared literal also serves other hit/hover paths; a sound-only mod needs a
// separate setting. The accepted tuning range has not been established.
// Sound names are s_layoutStringPool+4B/+59; shared 9A8/9AC/988 scalar arguments keep
// their existing sound API units/order. No vector mutation or retained pointers.
void Layout_PlayActivationSound(Layout* layout) {
    float* playerCursor;
    int playerFlagByteOffset;
    int circleHitShape;
    int left, top, right, bottom;
    int selectedLeft, selectedTop, selectedRight, selectedBottom;
    if (!((LayoutSoundAdmissionView*)layout)->enabled) return;
    int hasIneligibleActivation = 0;
    int hasEligibleActivation = 0;
    if (!g_layoutHitTestingEnabled) return;
    float rectangleOffset = kHitRectangleOffset;
    for (int index = 0; (unsigned)index < (unsigned)layout->elementCount; ++index) {
        int soundGate;
        int hitShapeByte;
        if (layout->extended) {
            hitShapeByte = (unsigned char)layout->itemsEx[index].hitShapeSelector;
            circleHitShape = (signed char)hitShapeByte;
            soundGate = ((LayoutExtendedSoundItemView*)&layout->itemsEx[index])->soundGate;
        } else {
            hitShapeByte = (unsigned char)layout->items[index].hitShapeSelector;
            circleHitShape = (signed char)hitShapeByte;
            struct Bits { CW_BITSP2(unsigned char, 6, unsigned char unused:1, signed char value:1) };
            soundGate = ((Bits*)((char*)&layout->items[index] + 0x18))->value;
        }
        if (layout->allPlayersMode) {
            playerCursor = (float*)layout;
            int ineligibleHit[4], eligibleHit[4];
            for (int player = 0, unused = (playerFlagByteOffset = 0); player < 4; ++player, playerCursor += 2, playerFlagByteOffset += 4) {
                ineligibleHit[player] = 0;
                eligibleHit[player] = 0;
                if (Input_IsActionPressed(LayoutManager_GetInstance(), ((LayoutActionView*)layout)->actionId, player) && Layout_SoundHit(layout, index, player, circleHitShape, rectangleOffset, playerCursor, left, top, right, bottom)) {
                    if (layout->elements[index].activationSoundEnabled && soundGate && *(int*)((char*)layout->elements[index].playerFlags + playerFlagByteOffset) && layout->elements[index].drawEnabled) eligibleHit[player] = 1;
                    if (layout->elements[index].activationSoundEnabled && soundGate && !*(int*)((char*)layout->elements[index].playerFlags + playerFlagByteOffset) && layout->elements[index].drawEnabled) ineligibleHit[player] = 1;
                }
                hasEligibleActivation |= eligibleHit[player];
                hasIneligibleActivation |= ineligibleHit[player];
            }
        } else {
            if (Input_IsActionPressed(LayoutManager_GetInstance(), ((LayoutActionView*)layout)->actionId, -1)) {
                int ineligibleHit = 0;
                int eligibleHit = 0;
                int player = InputManager_GetSelectedPlayer(LayoutManager_GetInstance());

                if (Input_IsActionPressed(LayoutManager_GetInstance(), ((LayoutActionView*)layout)->actionId, player) && Layout_SoundHitSingle(layout, index, player, circleHitShape, rectangleOffset, selectedLeft, selectedTop, selectedRight, selectedBottom)) {
                    if (layout->elements[index].activationSoundEnabled && soundGate && layout->elements[index].playerFlags[player] && layout->elements[index].drawEnabled) eligibleHit = 1;
                    if (layout->elements[index].activationSoundEnabled && soundGate && !layout->elements[index].playerFlags[player] && layout->elements[index].drawEnabled) ineligibleHit = 1;
                }
                hasEligibleActivation |= eligibleHit;
                hasIneligibleActivation |= ineligibleHit;
            }
        }
    }
    if (hasEligibleActivation) Sound_PlayByName(g_soundManager, s_layoutStringPool + 0x4B, 0, 0, 0, 0, kSoundMinimumGain, kSoundAttenuationRange, kFloat_1_LayoutUnit);
    else if (hasIneligibleActivation) Sound_PlayByName(g_soundManager, s_layoutStringPool + 0x59, 0, 0, 0, 0, kSoundMinimumGain, kSoundAttenuationRange, kFloat_1_LayoutUnit);
}
} // extern "C"

extern "C" {
// Test one borrowed element for hover collection: global gate, per-player
// eligibility, then circle/rectangle. pt points at layout+8*player; pt[4/5]
// are the sampled pointer XY. k is the existing rectangle offset. No writes.
inline int LayoutCollectHit(Layout* layout, int index, int player, float* pt, float k) {
    if (!g_layoutHitTestingEnabled) return 0;
    int g;
    if (layout->extended) g = layout->itemsEx[index].hitShapeSelector;
    else g = layout->items[index].hitShapeSelector;
    if (!layout->elements[index].playerFlags[player]) return 0;
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    int d, c, b, a;
    index = (int)(layout->elements + index);
    if (index) { a = ((LayoutElement*)index)->hitLeft; b = ((LayoutElement*)index)->hitTop; c = ((LayoutElement*)index)->hitRight; d = ((LayoutElement*)index)->hitBottom; }
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}
// Append a hovered element ordinal; same initialized-vector and copied-value
// ownership contract as LayoutIntVector_PushBack. This inline expansion uses a
// nonvolatile guard: it owns fresh storage until adoption, then old storage
// until destructor cleanup. Length/allocation throws leave the vector intact;
// input is consumed before the old array is cleared (live-element alias allowed).
inline void LayoutPushBack(LayoutIntVector* v, const int* value) {
    unsigned int size = v->size;
    if (size < v->capacity) {
        int next = 1 + size;
        int* d = v->data;
        *(volatile unsigned*)&v->size = next;
        (d + next)[-1] = *value;
    } else {
        if (0x3FFFFFFFu - v->capacity < 1) LayoutVectorThrow();
        LayoutHoverBuffer temp(&v->capacity);
        unsigned needed = 1 + v->size - v->capacity;
        unsigned capacity = v->capacity;
        if (needed > 0x3FFFFFFFu - capacity) LayoutVectorThrow();
        if (capacity < 0x15555555u) capacity += LayoutVectorMax((3 * (capacity + 1)) / 5, needed);
        else if (capacity < 0x2AAAAAAAu) capacity += LayoutVectorMax((capacity + 1) / 2, needed);
        else capacity = 0x3FFFFFFF;
        if (capacity > 0x3FFFFFFFu) LayoutVectorThrow();
        int* data = (int*)appMallocDummy__FUl(capacity * 4);
        unsigned int oldSize = v->size;
        temp.offset = oldSize;
        temp.data = data;
        (data + oldSize)[temp.size++] = *value;
        int* oldData = v->data;
        temp.capacity = capacity;
        int count = ((char*)(oldData + v->size) - (char*)oldData) / 4;
        int offset = oldSize - count;
        temp.offset = offset;
        unsigned int bytes = count * 4;
        memcpy(data + offset, oldData, bytes);
        memset(oldData, 0, bytes);
        unsigned zero = 0;
        int newSize = count + temp.size;
        unsigned int oldCapacity = v->capacity;
        oldData = v->data;
        v->size = zero;
        data = temp.data;
        temp.data = oldData;
        temp.size = newSize;
        v->capacity = temp.capacity;
        temp.capacity = oldCapacity;
        v->size = newSize;
        v->data = data;
        temp.size = zero;
    }
}
#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Replace v's live contents with hovered element ordinals for player 0..3,
// or selected player when sentinel 5. Required loaded layout and initialized
// owning int vector (not aliased with layout); preserve allocation/capacity.
// Clear size first, then append in ascending element order. Return nonzero iff
// nonempty; global hit gate/eligibility can produce empty. Allocation errors
// propagate leaving an already-collected prefix, not the old contents.
int Layout_CollectHovered(Layout* layout, LayoutIntVector* v, int player) {
    v->size -= (unsigned int)v->size;
    if (player == 5) {
        if (!((LayoutSingleton *&)Sing_805DC5C8())) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(&s_gameInputManager);
                __register_global_object(&s_gameInputManager, (void*)InputManager_Dtor, s_layoutInputManagerDtorRegistration);
                s_inputManagerInitialized = 1;
            }
            ((LayoutSingleton *&)Sing_805DC5C8()) = &s_gameInputManager;
        }
        player = InputManager_GetSelectedPlayer(((LayoutSingleton *&)Sing_805DC5C8()));
    }
    float k = kHitRectangleOffset;
    float* pt = (float*)((char*)layout + 8 * player);
    for (unsigned index = 0; index < (unsigned)layout->elementCount; ++index) {
        if (LayoutCollectHit(layout, index, player, pt, k)) LayoutPushBack(v, (int*)&index);
    }
    return 0 != v->size;
}
#pragma opt_propagation reset
#pragma pop
}
extern "C" {
void __ct__Q23ns08CSpdCtrlFv(void*);
void CBpTex_Init(void*);
int CSharedFilePtr_GetLoadedBuffer(int);
void SpdManager_BuildItemsFromResource(void*, void*, int);
int SpdSquareList_CloneEntry(void*, unsigned int index);
int CSpdSprite_CloneAndRegister(void*, unsigned int index);
void Sprite_GetPosition2D(void*, float*);
void Sprite_GetScale2D(void*, float*);
void Sprite_GetOrigin2D(void*, float*);
void Sprite_GetSourceSize2D(void*, float*);
void Sprite_GetPackedColor(void*, int*);
void SpdGeometry_GetRotationRadians(void*, float*);
void InputManager_GetPointerPosition(LayoutSingleton*, void*, int);
void InputManager_RequestPointerInput(LayoutSingleton*, int);
void Layout_PlayActivationSound(Layout*);
struct LayoutStandardRoot {
    char data[0x30];
    // Construct the owned standard sprite-manager root in caller-provided new storage.
    LayoutStandardRoot() { __ct__Q23ns08CSpdCtrlFv(this); }
};
struct LayoutStandardVirtual { 
virtual void a(); virtual void load(void*);
};
struct LayoutStandardChild {
    void (**table)(); char data[0x24];
    // Initialize one owned optional texture child before asynchronous resource loading.
    LayoutStandardChild() { CBpTex_Init(this); }
    // Synchronously dispatch the borrowed resource descriptor to virtual load slot 1.
    void load(void* p) { ((LayoutStandardVirtual*)this)->load(p); }
};
struct LayoutStandardItem {
    short id; char pad2[2]; void* resource4; void* resource8; char padC[12];
    CW_BITSP2(signed char, 6, signed char flag:1, signed char enabled:1) char pad19[3];
};
#pragma push
#pragma use_lmw_stmw on
// Readiness protocol shared with UpdateExtended and PollLoadComplete:
// waiting(0): update does no resource work; poll waits for file, then writes 1.
// building(1): update constructs root/children once (root!=null skips rebuild);
// poll remains 1 until root exists and requested children report ready, then 2.
// ready(2): update samples selected pointer first, remaining players in ascending
// order, plays activation feedback, and requests pointer input if eligible.
// These updates return 1 even while waiting; callers must poll for completion.
// No error state/rollback: allocation/load failures propagate or leave partial
// state. Destroy/reset owns release; readiness does not transfer child ownership.
// Each update also arms hoverFeedbackPending for one hover pass, regardless of load phase.
int Layout_UpdateStandard(Layout* layout) {
    layout->hoverFeedbackPending = 1;
    int p;

    switch (layout->loadPhase) {
    case kLayoutWaitingForFile: break;
    case kLayoutBuildingItems:
        if (layout->spdRoot) break;
        layout->spdRoot = new LayoutStandardRoot;
        SpdManager_BuildItemsFromResource(layout->spdRoot, (void*)CSharedFilePtr_GetLoadedBuffer(layout->w44), 0);
        for (int i = 0; (unsigned)i < (unsigned)layout->elementCount; ++i) {

            if (!(((LayoutStandardItem*)layout->items)[i].flag)) {
                if (((LayoutStandardItem*)layout->items)[i].resource4) {
                    layout->elements[i].unk48i = (int)new LayoutStandardChild;
                    ((LayoutStandardChild*)layout->elements[i].unk48i)->load(((LayoutStandardItem*)layout->items)[i].resource4);
                }
                if (((LayoutStandardItem*)layout->items)[i].resource8) {
                    layout->elements[i].unk4Ci = (int)new LayoutStandardChild;
                    ((LayoutStandardChild*)layout->elements[i].unk4Ci)->load(((LayoutStandardItem*)layout->items)[i].resource8);
                }
            }
            layout->elements[i].playerFlags[0] = ((LayoutStandardItem*)layout->items)[i].enabled;
            layout->elements[i].playerFlags[1] = ((LayoutStandardItem*)layout->items)[i].enabled;
            layout->elements[i].playerFlags[2] = ((LayoutStandardItem*)layout->items)[i].enabled;
            layout->elements[i].playerFlags[3] = ((LayoutStandardItem*)layout->items)[i].enabled;
        }
        for (int i = 0; (unsigned)i < (unsigned)layout->elementCount; ++i) layout->elements[i].clonedItemId = 0;
        for (int i = 1; (unsigned)i < (unsigned)layout->elementCount; ++i) {
            for (int j = 0; (unsigned)j < (unsigned)i; ++j) {
                short idI = layout->items[i].id;
                short idJ = layout->items[j].id;
                if (idI == idJ) {

                    if ((((LayoutStandardItem*)layout->items)[i].flag)) layout->elements[i].clonedItemId = SpdSquareList_CloneEntry(layout->spdRoot, idI);
                    else layout->elements[i].clonedItemId = CSpdSprite_CloneAndRegister(layout->spdRoot, idI);
                }
            }
        }
        for (int i = 0; (unsigned)i < (unsigned)layout->elementCount; ++i) {
            void* obj;

            if (!(!((((LayoutStandardItem*)layout->items)[i].flag)))) {
                if (layout->elements[i].clonedItemId) obj = (void*)SpdManager_GetSquareAt(layout->spdRoot, layout->elements[i].clonedItemId);
                else obj = (void*)SpdManager_GetSquareAt(layout->spdRoot, layout->items[i].id);
            } else {
                if (layout->elements[i].clonedItemId) obj = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[i].clonedItemId);
                else obj = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->items[i].id);
            }
            Sprite_GetPosition2D(obj, &layout->elements[i].positionX);
            Sprite_GetScale2D(obj, &layout->elements[i].scaleX);
            Sprite_GetOrigin2D(obj, &layout->elements[i].originX);
            Sprite_GetSourceSize2D(obj, &layout->elements[i].sourceWidth);
            Sprite_GetPackedColor(obj, &layout->elements[i].packedColor);
            SpdGeometry_GetRotationRadians(obj, &layout->elements[i].rotationRadians);
            layout->elements[i].drawEnabled = 1;
            layout->elements[i].activationSoundEnabled = 1;
            float position[2] = {0,0};
            float size[2] = {0,0};
            float scale[2] = {0,0};
            float origin[2] = {0,0};
            Sprite_GetPosition2D(obj, position);
            Sprite_GetSourceSize2D(obj, size);
            Sprite_GetScale2D(obj, scale);
            Sprite_GetOrigin2D(obj, origin);
            float product2 = origin[0] * scale[0];
            float product3 = scale[1] * origin[1];
            origin[1] = product3;
            origin[0] = product2;
            size[0] = size[0] * scale[0];
            size[1] = (size[1] * scale[1]);
            position[0] = position[0] - origin[0];
            position[1] = position[1] - origin[1];

            LayoutElement* e = &layout->elements[(unsigned)i];
            float left = position[0];
            float leftValue = left;
            float top = position[1];
            float topValue = top;
            unsigned int y1 = (int)topValue;
            __typeof__(top) _p436 = top;
            __typeof__(_p436 + size[1]) _p345 = _p436 + size[1];
            __typeof__(_p345) _p562 = _p436 + size[1];
            int y2 = (int)(_p562);
            int x1 = (int)leftValue;
            e->hitLeft = x1;
            unsigned int x2 = (int)(leftValue + size[0]);
            unsigned int x2Value = x2;
            int y2Value = y2;
            e->hitTop = y1;
            e->hitRight = x2Value;
            e->hitBottom = y2Value;
        }
        break;
    case kLayoutReady:
        LayoutSingleton* input = LayoutManager_GetInstance();
        int player = InputManager_GetSelectedPlayer(input);
        __typeof__(player) _p850 = player;
        __typeof__(8 * _p850) _p684 = 8 * _p850;
        __typeof__(_p850 * 8) _p390 = _p850 * 8;
        __typeof__(8 * _p850) _p442 = _p850 * 8;
        __typeof__(8 * _p850) _p998 = 8 * _p850;
        __typeof__(8 * _p850) _p360 = 8 * _p850;
        __typeof__(8 * _p850) _p932 = 8 * _p850;
        __typeof__(_p932) _p645 = 8 * _p850;
        __typeof__(8 * _p850) _p906 = 8 * _p850;
        __typeof__(_p906) _p648 = 8 * _p850;
        __typeof__(_p850 * 8) _p386 = _p850 * 8;
        InputManager_GetPointerPosition(input, (char*)layout + _p386 + 0x10, _p850);
        char* point = (char*)layout + 0x10;
        for (p = 0; 4 > p; ++p, point += 8) {
            if (p != player) InputManager_GetPointerPosition(input, point, p);
        }
        Layout_PlayActivationSound(layout);
        int enabled = 0;
        LayoutSingleton* input2 = LayoutManager_GetInstance();
        for (int i = 0; (unsigned)i < (unsigned)layout->elementCount; ++i) {
            if (layout->allPlayersMode) {
                enabled |= layout->elements[i].playerFlags[0];
                enabled |= layout->elements[i].playerFlags[1];
                enabled |= layout->elements[i].playerFlags[2];
                enabled |= layout->elements[i].playerFlags[3];
            } else enabled |= layout->elements[i].playerFlags[InputManager_GetSelectedPlayer(input2)];
        }
        int enabledValue = enabled;
        InputManager_RequestPointerInput(input2, enabledValue);
        break;
    }
    return 1;
}
#pragma pop
}

extern "C" {
void* SpdManager_GetResourceFlags(void*);
void* Table_GetEntry(void*, int);
#pragma push
#pragma use_lmw_stmw on
struct LayoutExtendedChild { 
virtual void pad(); virtual void load(void*);
};
// Same phase protocol as UpdateStandard; extended records come from the loaded
// table and create four optional children via relative resource references.
// itemsEx borrows table storage; elements is a constructed owned 0x6C-stride array.
int Layout_UpdateExtended(Layout* layout) {
    int p;
    LayoutSingleton* input;

    layout->hoverFeedbackPending = 1;
    LayoutSingleton* input2;
    switch (layout->loadPhase) {
    case kLayoutWaitingForFile: break;
    case kLayoutBuildingItems:
        if (layout->spdRoot) break;
        layout->spdRoot = new LayoutStandardRoot;
        SpdManager_BuildItemsFromResource(layout->spdRoot, (void*)CSharedFilePtr_GetLoadedBuffer(layout->w44), 0);
        *(void**)layout->spdResourceTable = SpdManager_GetResourceFlags(layout->spdRoot);
        layout->itemsEx = (LayoutItemEx*)Table_GetEntry(*(void**)layout->spdResourceTable, 0);
        int count = *(void**)layout->spdResourceTable ? ((int*)*(void**)layout->spdResourceTable)[2] : 0;
        int countValue5 = count;
        int countValue6 = count;
        layout->elementCount = countValue5;
        void* elements = __nwa__FUl(countValue6 * 0x6c + 0x10);
        if (elements) elements = __construct_new_array(elements, (void*)CSPDPangyaIcon_PangyaIconObject_ctor, (void*)CSPDPangyaIcon_PANGYA_ICON_OBJECT_VAR_dtor, 0x6c, count);
        layout->elements = (LayoutElement*)elements;
        for (int i = 0; (unsigned)i < (unsigned)layout->elementCount; ++i) {
            if (!layout->itemsEx[i].flag) {
                if (*LayoutReadyFlag(layout, i * 0x24, 5)) {
                    layout->elements[i].unk48i = (int)new LayoutStandardChild;
                    ((LayoutExtendedChild*)layout->elements[i].unk48i)->load(LayoutReadyFlag(layout, i * 0x24, 5));
                }
                if (*LayoutReadyFlag(layout, i * 0x24, 6)) {
                    layout->elements[i].unk4Ci = (int)new LayoutStandardChild;
                    ((LayoutExtendedChild*)layout->elements[i].unk4Ci)->load(LayoutReadyFlag(layout, i * 0x24, 6));
                }
                if (*LayoutReadyFlag(layout, i * 0x24, 7)) {
                    layout->elements[i].unk50i = (int)new LayoutStandardChild;
                    ((LayoutExtendedChild*)layout->elements[i].unk50i)->load(LayoutReadyFlag(layout, i * 0x24, 7));
                }
                if (*LayoutReadyFlag(layout, i * 0x24, 8)) {
                    layout->elements[i].unk54i = (int)new LayoutStandardChild;
                    ((LayoutExtendedChild*)layout->elements[i].unk54i)->load(LayoutReadyFlag(layout, i * 0x24, 8));
                }
            }
            layout->elements[i].playerFlags[0] = *(signed char*)((char*)&layout->itemsEx[i] + 0x10);
            layout->elements[i].playerFlags[1] = *(signed char*)((char*)&layout->itemsEx[i] + 0x10);
            layout->elements[i].playerFlags[2] = *(signed char*)((char*)&layout->itemsEx[i] + 0x10);
            layout->elements[i].playerFlags[3] = *(signed char*)((char*)&layout->itemsEx[i] + 0x10);
        }
        for (int i = 0; (unsigned)i < (unsigned)layout->elementCount; ++i) layout->elements[i].clonedItemId = 0;
        for (int i = 1; (unsigned)i < (unsigned)layout->elementCount; ++i) {
            for (int j = 0; (unsigned)j < (unsigned)i; ++j) {
                if (layout->itemsEx[j].id == layout->itemsEx[i].id) {

                    if ((layout->itemsEx[i].flag)) layout->elements[i].clonedItemId = SpdSquareList_CloneEntry(layout->spdRoot, layout->itemsEx[i].id);
                    else layout->elements[i].clonedItemId = CSpdSprite_CloneAndRegister(layout->spdRoot, layout->itemsEx[i].id);
                }
            }
        }
        for (int i = 0; (unsigned)i < (unsigned)layout->elementCount; ++i) {
            void* obj;

                    if (!(!(!(!(!(!((layout->itemsEx[i].flag)))))))) {
                if (layout->elements[i].clonedItemId) obj = (void*)SpdManager_GetSquareAt(layout->spdRoot, layout->elements[i].clonedItemId);
                else obj = (void*)SpdManager_GetSquareAt(layout->spdRoot, layout->itemsEx[i].id);
            } else {
                if (layout->elements[i].clonedItemId) obj = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[i].clonedItemId);
                else obj = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->itemsEx[i].id);
            }
            Sprite_GetPosition2D(obj, &layout->elements[i].positionX);
            Sprite_GetScale2D(obj, &layout->elements[i].scaleX);
            Sprite_GetOrigin2D(obj, &layout->elements[i].originX);
            Sprite_GetSourceSize2D(obj, &layout->elements[i].sourceWidth);
            Sprite_GetPackedColor(obj, &layout->elements[i].packedColor);
            SpdGeometry_GetRotationRadians(obj, &layout->elements[i].rotationRadians);
            layout->elements[i].drawEnabled = 1;
            layout->elements[i].activationSoundEnabled = 1;
            float position[2] = {0,0};
            float size[2] = {0,0};
            float scale[2] = {0,0};
            float origin[2] = {0,0};
            Sprite_GetPosition2D(obj, position);
            Sprite_GetSourceSize2D(obj, size);
            Sprite_GetScale2D(obj, scale);
            __typeof__(scale[1] * origin[1]) _p423 = scale[1] * origin[1];
            Sprite_GetOrigin2D(obj, origin);
            float product4 = origin[1] * scale[1];
            origin[1] = product4;
            __typeof__(size[0] * scale[0]) _p743 = size[0] * scale[0];
            origin[0] = origin[0] * scale[0];
            __typeof__(_p743) _p251 = size[0] * scale[0];
            __typeof__(size[0] * scale[0]) _p957 = size[0] * scale[0];
            float product5 = size[0] * scale[0];
            float product6 = scale[0] * size[0];
            size[1] = size[1] * scale[1];
            size[0] = product6;

            LayoutElement* e = &layout->elements[(unsigned)i];
            float top = position[1] - origin[1];
            float topValue2 = top;
            float sum2 = top + size[1];
            float topValue3 = top;
            float left = position[0] - origin[0];
            __typeof__(size[0] + left) _p620 = size[0] + left;
            __typeof__(_p620) _p571 = size[0] + left;
            __typeof__(left) _p70 = left;
            __typeof__(_p571) _p457 = _p70 + size[0];
            __typeof__(left) _p307 = left;
            __typeof__(_p307 + size[0]) _p21 = _p307 + size[0];
            __typeof__(_p21) _p312 = size[0] + _p307;
            float leftValue2 = left;
            int y1 = (int)topValue2;
            int y2 = (int)(sum2);
            unsigned int x2 = (int)(_p457);
            unsigned int x2Value2 = x2;
            int x1 = (int)leftValue2;
            e->hitLeft = x1;
            e->hitRight = x2Value2;
            e->hitTop = y1;
            int y2Value2 = y2;
            e->hitBottom = y2Value2;
        }
        break;
    case kLayoutReady:
        input = LayoutManager_GetInstance();
        int player = InputManager_GetSelectedPlayer(input);
        __typeof__(player) _p546 = player; __typeof__(8 * _p546) _p279 = 8 * _p546; __typeof__(_p279) _p470 = _p546 * 8; InputManager_GetPointerPosition(input, (char*)layout + _p470 + 0x10, _p546);
        char* point = (char*)layout + 0x10;
        for (p = 0; 4 > p; ++p, point += 8) {
            if (p != player) InputManager_GetPointerPosition(input, point, p);
        }
        Layout_PlayActivationSound(layout);
        int enabled = 0;
        input2 = LayoutManager_GetInstance();
        for (int i = 0; (unsigned)i < (unsigned)layout->elementCount; ++i) {
            if (layout->allPlayersMode) {
                __typeof__(layout->elements[i].playerFlags[2]) _p667 = layout->elements[i].playerFlags[2];
                __typeof__(layout->elements[i].playerFlags[2]) _p966 = layout->elements[i].playerFlags[2];
                int player2FlagsSnapshot = layout->elements[i].playerFlags[2];
                int player2FlagsSnapshot2 = layout->elements[i].playerFlags[2];
                int player2FlagsSnapshot3 = layout->elements[i].playerFlags[2];
                int player2FlagsSnapshot4 = layout->elements[i].playerFlags[2];
                int player1FlagsSnapshot = layout->elements[i].playerFlags[1];
                int player1FlagsSnapshot2 = layout->elements[i].playerFlags[1];
                __typeof__(layout->elements[i].playerFlags[2]) _p611 = layout->elements[i].playerFlags[2];
                __typeof__(_p611) _p887 = layout->elements[i].playerFlags[2];
                __typeof__(layout->elements[i].playerFlags[0]) _p958 = layout->elements[i].playerFlags[0];
                __typeof__(_p958) _p140 = layout->elements[i].playerFlags[0];
                __typeof__(_p140 | enabled) _p450 = enabled | _p140;
                __typeof__(enabled) _p897 = enabled;
                __typeof__(_p140 | _p897) _p945 = _p897 | _p140;
                __typeof__(_p140 | enabled) _p321 = enabled | _p140;
                int enabledValue2 = enabled;
                int enabledValue3 = enabled;
                __typeof__(enabled) _p76 = enabled;
                __typeof__(_p321) _p279 = _p76 | _p140;
                enabled = _p279;
                enabled |= player1FlagsSnapshot2;
                __typeof__(_p887 | enabled) _p683 = _p887 | enabled;
                enabled = _p683;
                enabled = enabled | layout->elements[i].playerFlags[3];
            } else enabled |= layout->elements[i].playerFlags[InputManager_GetSelectedPlayer(input2)];
        }
        int enabledValue4 = enabled;
        InputManager_RequestPointerInput(input2, enabledValue4);
        break;
    }

    return 1;
}
#pragma pop
}

extern "C" {
struct LayoutUpdatePair { float x,y; };
extern const LayoutUpdatePair s_layoutUpdateScaleX;
extern int s_layoutUpdateScaleY;
inline int LayoutUpdateHit(Layout* layout, unsigned index, int player, float* pt, float k) {
    if (!g_layoutHitTestingEnabled) return 0;
    signed char g;
    if (layout->extended) g = (unsigned char)layout->itemsEx[index].hitShapeSelector;
    else g = (unsigned char)layout->items[index].hitShapeSelector;
    if (!layout->elements[index].playerFlags[player]) return 0;
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    int d, c, b, a;
    LayoutElement* e = layout->elements + index;
    if (e) { a = e->hitLeft; b = e->hitTop; c = e->hitRight; d = e->hitBottom; }
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}inline int LayoutUpdateFallbackHoverHit(Layout* layout, unsigned index, int player, int g, float k, float* pt) {
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    int d, c, b, a;
    LayoutElement* e = layout->elements + index;
    if (e) { a = e->hitLeft; b = e->hitTop; c = e->hitRight; d = e->hitBottom; }

    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}
inline int LayoutUpdateFallbackHoverHitSingle(Layout* layout, unsigned index, int player, int g, float k) {
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    int d, c, b, a;
    LayoutElement* e = layout->elements + index;
    if (e) { a = e->hitLeft; b = e->hitTop; c = e->hitRight; d = e->hitBottom; }

    float* pt = (float*)((char*)layout + player * 8);
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}

inline int LayoutUpdateHitMulti(Layout* layout, unsigned index, int player, float* pt, float k, int& a, int& b, int& c, int& d) {
    if (!g_layoutHitTestingEnabled) return 0;
    signed char g;
    if (layout->extended) g = (unsigned char)layout->itemsEx[index].hitShapeSelector;
    else g = (unsigned char)layout->items[index].hitShapeSelector;
    if (!layout->elements[index].playerFlags[player]) return 0;
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    LayoutElement* e = layout->elements + index;
    if (e) { a = e->hitLeft; b = e->hitTop; c = e->hitRight; d = e->hitBottom; }
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}inline int LayoutUpdateFallbackHoverHitMulti(Layout* layout, unsigned index, int player, int g, float k, float* pt, int& a, int& b, int& c, int& d) {
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    LayoutElement* e = layout->elements + index;
    if (e) { a = e->hitLeft; b = e->hitTop; c = e->hitRight; d = e->hitBottom; }

    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}

#pragma push
#pragma use_lmw_stmw on
// Extended hover/render contract (standard variant below has the same phases):
// 1. Require a root, set UI render state, require input singleton, consume hoverFeedbackPending.
//    A second draw before Update still renders but cannot raise cursor feedback.
// 2. Hit eligibility uses global input gate + playerFlags[player], separately
//    from element draw-enable. g==0 tests strict rectangle inequalities; g!=0
//    uses circle. Feedback fallback accepts only g==0/1 and the item's extra gate.
//    Rectangle k=-4 shifts all four edges by -4, it does NOT enlarge the box.
// 3. OR hits across players when allPlayersMode!=0. Raise cursor appearance 2 for an eligible
//    hit, 1 for enabled fallback feedback. No click/activation is committed here.
// 4. Draw enabled elements: hover offsets/scale + base geometry, low-byte opacity
//    (t<=0 -> entire color zero; t>=1 -> original), value texture region for
//    sprites, then object update and draw. t is dimensionless opacity, not time.
// Extended item schema: +2=g, +4/+8=hover XY offset, +C=uniform hover scale,
// +10=fallback feedback gate, +11=sprite(0)/square(nonzero). Standard stores
// offsets at +C/+10, scale +14, and packed gates/type at +18 (see variant).
// All objects are borrowed from root; no ownership changes. Hit helpers with
// reference scratch outputs only populate edges on their rectangle path; do
// not read those outputs after an eligibility return or a circle branch.
void Layout_UpdateCursorHover(Layout* layout, float t) {
    int ha;
    float* pt;
    int hb; int hc; int hd;
    int sa; int sb; int sc; int sd;
    if (!layout->spdRoot) return;
    UIRender_SetDefaultState(layout);
    LayoutSingleton* input = LayoutManager_GetInstance();
    if (!input) return;
    int previous = layout->hoverFeedbackPending;
    layout->hoverFeedbackPending = 0;
    float k = -4.0f;
    for (int index = 0; (unsigned)index < (unsigned)layout->elementCount; ++index) {
        unsigned int hovered = 0;
        unsigned int enabled = layout->elements[index].drawEnabled;
        if (layout->allPlayersMode) {
            pt = (float*)layout;
            for (int player = 0; player < 4; ++player, pt += 2) {
                if (previous && LayoutUpdateHitMulti(layout,index,player,pt,k,ha,hb,hc,hd)) {
                    InputManager_RaiseCursorAppearance(input,2,player);
                    hovered = 1;
                } else if (enabled) {
                    if (!layout->itemsEx[index].hitShapeSelector) {
                        if (*(signed char*)((char*)&layout->itemsEx[index]+0x10) && LayoutUpdateFallbackHoverHitMulti(layout,index,player,0,k,pt,sa,sb,sc,sd) && previous) InputManager_RaiseCursorAppearance(input,1,player);
                    } else if (1 == layout->itemsEx[index].hitShapeSelector) {
                        if (*(signed char*)((char*)&layout->itemsEx[index]+0x10) && Layout_HitTestElementCircle(layout,index,player) && previous) InputManager_RaiseCursorAppearance(input,1,player);
                    }
                }
            }
        } else {
            int player = InputManager_GetSelectedPlayer(input);
            if (previous && LayoutUpdateHit(layout,index,player,(float*)((char*)layout+player*8),k)) {
                InputManager_RaiseCursorAppearance(input,2,player);
                hovered = 1;
            } else if (enabled) {
                if (layout->itemsEx[index].hitShapeSelector == 0) {
                    if (*(signed char*)((char*)&layout->itemsEx[index]+0x10) && LayoutUpdateFallbackHoverHitSingle(layout,index,player,0,k) && previous) InputManager_RaiseCursorAppearance(input,1,player);
                } else if (layout->itemsEx[index].hitShapeSelector == 1) {
                    if (*(signed char*)((char*)&layout->itemsEx[index]+0x10) && Layout_HitTestElementCircle(layout,index,player) && previous) InputManager_RaiseCursorAppearance(input,1,player);
                }
            }
        }
        if (enabled) {
            float temp[2];
            float position[2] = {0,0};
            LayoutUpdatePair scale;
            *(int*)&scale.x = *(int*)&s_layoutUpdateScaleX; *(int*)&scale.y = s_layoutUpdateScaleY;
    if (0 == layout->itemsEx[index].flag) {
        LayoutObject* obj;
        if (!(!(!(layout->elements[index].clonedItemId != 0)))) {
            obj = (LayoutObject*)SpdManager_GetSpriteAt(layout->spdRoot, layout->itemsEx[index].id);
        } else {
            obj = (LayoutObject*)SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[index].clonedItemId);
        }
        if (hovered) {
            float z = *(float*)((char*)&layout->itemsEx[index]+0xc);
            float y;
            float x = *(float*)((char*)&layout->itemsEx[index]+4);
            y = *(float*)((char*)&layout->itemsEx[index]+8);
            position[0] = x; position[1] = y; scale.x = z; scale.y = z;
        }
        temp[0] = position[0] + (*(LayoutElement* volatile*)&layout->elements)[index].positionX;
        temp[1] = position[1] + (*(LayoutElement* volatile*)&layout->elements)[index].positionY;
        Sprite_SetPosition2D(obj,temp);
        temp[0] = (*(LayoutElement* volatile*)&layout->elements)[index].originX; temp[1] = (*(LayoutElement* volatile*)&layout->elements)[index].originY; Sprite_SetOrigin2D(obj,temp);
        temp[0] = scale.x * (*(LayoutElement* volatile*)&layout->elements)[index].scaleX; temp[1] = scale.y * (*(LayoutElement* volatile*)&layout->elements)[index].scaleY; Sprite_SetScale2D(obj,temp);
        Sprite_SetSourceSize2D(obj, &layout->elements[index].sourceWidth);
        unsigned color = layout->elements[index].packedColor;
        unsigned faded;
        if (t <= 0.0f) faded = 0;
        else if (t >= 1.0f) faded = color;
        else faded = (color & 0xff00) | (unsigned char)Runtime_ConvertDoubleToUnsigned((float)(unsigned char)color * t) | (color & 0xffff0000);
        Sprite_SetPackedColor(obj, faded);
        SpdGeometry_SetRotationRadians(obj, layout->elements[index].rotationRadians);
        if (0 != layout->spdRoot) {
            void* child;
            unsigned int v = Layout_GetElementValue(layout, index, hovered);
            if (layout->elements[(unsigned)index].clonedItemId != 0) {
                child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[(unsigned)index].clonedItemId);
            } else if (!(layout->extended)) {
                        child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->items[(unsigned)index].id);
            } else {
                child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->itemsEx[(unsigned)index].id);
            }
            TextureRegion_SetSourceAndNormalizeBounds(child, v);
        }
        obj->v1();
        SimpleSpd_DrawBpSprite(obj);
    } else {
        unsigned color;
        LayoutObject* obj;
        if (!(layout->elements[index].clonedItemId)) {
                    obj = (LayoutObject*)SpdManager_GetSquareAt(layout->spdRoot, layout->itemsEx[index].id);
        } else {
            obj = (LayoutObject*)SpdManager_GetSquareAt(layout->spdRoot, layout->elements[index].clonedItemId);
        }
        if (hovered) {
            float z = *(float*)((char*)&layout->itemsEx[index]+0xc);
            float y;
            float x = *(float*)((char*)&layout->itemsEx[index]+4);
            y = *(float*)((char*)&layout->itemsEx[index]+8);
            position[0] = x; position[1] = y; scale.x = z; scale.y = z;
        }
        temp[0] = position[0] + (*(LayoutElement* volatile*)&layout->elements)[index].positionX;
        temp[1] = position[1] + (*(LayoutElement* volatile*)&layout->elements)[index].positionY;
        Sprite_SetPosition2D(obj,temp);
                temp[0] = (*(LayoutElement* volatile*)&layout->elements)[index].originX;
                temp[1] = (*(LayoutElement* volatile*)&layout->elements)[index].originY;
                Sprite_SetOrigin2D(obj,temp);
                temp[0] = scale.x * (*(LayoutElement* volatile*)&layout->elements)[index].scaleX;
                temp[1] = scale.y * (*(LayoutElement* volatile*)&layout->elements)[index].scaleY;
                Sprite_SetScale2D(obj,temp);
        Sprite_SetSourceSize2D(obj, &layout->elements[index].sourceWidth);
        color = layout->elements[index].packedColor;
        unsigned faded;
        if (t <= 0.0f) faded = 0;
        else if (t >= 1.0f) faded = color;
        else faded = (color & 0xff00) | (unsigned char)Runtime_ConvertDoubleToUnsigned((float)(unsigned char)color * t) | (color & 0xffff0000);
                Sprite_SetPackedColor(obj, faded);
        SpdGeometry_SetRotationRadians(obj, layout->elements[index].rotationRadians);
        obj->v1();
        SimpleSpd_DrawBpSprite(obj);
    }
        }
    }
}

extern const LayoutUpdatePair s_layoutDefaultScaleX;
extern int s_layoutDefaultScaleY;
inline int LayoutStandardHit(Layout* layout, unsigned index, int player, float* pt, float k) {
    if (!g_layoutHitTestingEnabled) return 0;
    int g;
    if (layout->extended) g = layout->itemsEx[index].hitShapeSelector;
    else g = layout->items[index].hitShapeSelector;
    if (!layout->elements[index].playerFlags[player]) return 0;
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    int d, c, b, a;
    LayoutElement* e = layout->elements + index;
    if (e) { a = e->hitLeft; b = e->hitTop; c = e->hitRight; d = e->hitBottom; }
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}inline int LayoutStandardFallbackHoverHit(Layout* layout, unsigned index, int player, int g, float k, float* pt) {
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    int d, c, b, a;
    LayoutElement* e = layout->elements + index;
    if (e) { a = e->hitLeft; b = e->hitTop; c = e->hitRight; d = e->hitBottom; }

    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}
inline int LayoutStandardFallbackHoverHitSingle(Layout* layout, unsigned index, int player, int g, float k) {
    if (g) return Layout_HitTestElementCircle(layout, index, player);
    int d, c, b, a;
    LayoutElement* e = layout->elements + index;
    if (e) { a = e->hitLeft; b = e->hitTop; c = e->hitRight; d = e->hitBottom; }

    float* pt = (float*)((char*)layout + player * 8);
    if ((float)a + k < pt[4] && (float)b + k < pt[5] && (float)c + k > pt[4] && (float)d + k > pt[5]) return 1;
    return 0;
}

struct LayoutStandardBits { CW_BITSP2(signed char, 5, signed char pad : 1, signed char sound : 2) };
struct LayoutStandardRenderBits { CW_BITSP1(signed char, 6, signed char render : 2) };
// Standard hover/render: see extended contract above. Packed +18 decoding is
// intentionally sign-extending: masks 0x60 (feedback) and 0xC0 (draw kind)
// overlap, and shifting makes the high selected bit decide zero vs negative.
// Preserve this decoding rather than treating the masked field as a Boolean.
// Default offset/scale and hit adjustment come from s_layoutDefaultScaleX/97C/980;
// these are shared pool values, not independent tuning settings.
void Layout_UpdateCursorHoverFeedback(Layout* layout, float t) {
    float* pt;
    if (!layout->spdRoot) return;
    UIRender_SetDefaultState(layout);
    LayoutSingleton* input = LayoutManager_GetInstance();
    if (!input) return;
    int previous = layout->hoverFeedbackPending;
    layout->hoverFeedbackPending = 0;
    float k = kHitRectangleOffset;
    for (int index = 0; (unsigned)index < (unsigned)layout->elementCount; ++index) {
        int hovered = 0;
        int enabled = layout->elements[index].drawEnabled;
        if (layout->allPlayersMode) {
            pt = (float*)layout;
            for (int player = 0; player < 4; ++player, pt += 2) {
                if (previous && LayoutStandardHit(layout,index,player,pt,k)) {
                    InputManager_RaiseCursorAppearance(input,2,player);
                    hovered = 1;
                } else if (enabled) {
                    if (layout->items[index].hitShapeSelector == 0) {
                        if (((int)((*((unsigned char*)&layout->items[index]+0x18) & 0x60u) << 25) >> 31) && LayoutStandardFallbackHoverHit(layout,index,player,0,k,pt) && previous) InputManager_RaiseCursorAppearance(input,1,player);
                    } else if (layout->items[index].hitShapeSelector == 1) {
                        if (((int)((*((unsigned char*)&layout->items[index]+0x18) & 0x60u) << 25) >> 31) && Layout_HitTestElementCircle(layout,index,player) && previous) InputManager_RaiseCursorAppearance(input,1,player);
                    }
                }
            }
        } else {
            int player = InputManager_GetSelectedPlayer(input);
            if (previous && LayoutStandardHit(layout,index,player,(float*)((char*)layout+player*8),k)) {
                InputManager_RaiseCursorAppearance(input,2,player);
                hovered = 1;
            } else if (enabled) {
                if (layout->items[index].hitShapeSelector == 0) {
                    if (((int)((*((unsigned char*)&layout->items[index]+0x18) & 0x60u) << 25) >> 31) && LayoutStandardFallbackHoverHitSingle(layout,index,player,0,k) && previous) InputManager_RaiseCursorAppearance(input,1,player);
                } else if (layout->items[index].hitShapeSelector == 1) {
                    if (((int)((*((unsigned char*)&layout->items[index]+0x18) & 0x60u) << 25) >> 31) && Layout_HitTestElementCircle(layout,index,player) && previous) InputManager_RaiseCursorAppearance(input,1,player);
                }
            }
        }
        if (enabled) {
            float temp[2];
            float position[2] = {0,0};
            LayoutUpdatePair scale;
            LayoutItem* items = layout->items;
            *(int*)&scale.x = *(int*)&s_layoutDefaultScaleX; *(int*)&scale.y = s_layoutDefaultScaleY;
    if (((int)((*((unsigned char*)&items[index]+0x18) & 0xc0u) << 24) >> 31) == 0) {
        LayoutObject* obj;
        if (layout->elements[index].clonedItemId != 0) {
            obj = (LayoutObject*)SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[index].clonedItemId);
        } else {
            obj = (LayoutObject*)SpdManager_GetSpriteAt(layout->spdRoot, layout->items[index].id);
        }
        if (hovered) {
            float z;
            float y;
            z = *(float*)((char*)&layout->items[index]+0x14);
            float x = *(float*)((char*)&layout->items[index]+0xc);
            y = *(float*)((char*)&layout->items[index]+0x10);
            position[0] = x; position[1] = y;
            scale.x = z; scale.y = z;
        }
        temp[0] = position[0] + (*(LayoutElement* volatile*)&layout->elements)[index].positionX;
        temp[1] = position[1] + (*(LayoutElement* volatile*)&layout->elements)[index].positionY;
        Sprite_SetPosition2D(obj,temp);
        temp[0] = (*(LayoutElement* volatile*)&layout->elements)[index].originX; temp[1] = (*(LayoutElement* volatile*)&layout->elements)[index].originY; Sprite_SetOrigin2D(obj,temp);
        temp[0] = scale.x * (*(LayoutElement* volatile*)&layout->elements)[index].scaleX; temp[1] = scale.y * (*(LayoutElement* volatile*)&layout->elements)[index].scaleY; Sprite_SetScale2D(obj,temp);
        Sprite_SetSourceSize2D(obj, &layout->elements[index].sourceWidth);
        unsigned color = layout->elements[index].packedColor;
        unsigned faded;
        if (t <= 0.0f) faded = 0;
        else if (t >= 1.0f) faded = color;
        else faded = (color & 0xff00) | (unsigned char)Runtime_ConvertDoubleToUnsigned((float)(unsigned char)color * t) | (color & 0xffff0000);
        Sprite_SetPackedColor(obj, faded);
        SpdGeometry_SetRotationRadians(obj, layout->elements[index].rotationRadians);
        if (layout->spdRoot != 0) {
            int v = Layout_GetElementValue(layout, index, hovered);
            void* child;
            if (layout->elements[(unsigned)index].clonedItemId != 0) {
                child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->elements[(unsigned)index].clonedItemId);
            } else if (layout->extended) {
                child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->itemsEx[(unsigned)index].id);
            } else {
                child = (void*)SpdManager_GetSpriteAt(layout->spdRoot, layout->items[(unsigned)index].id);
            }
            TextureRegion_SetSourceAndNormalizeBounds(child, v);
        }
        obj->v1();
        SimpleSpd_DrawBpSprite(obj);
    } else {
        unsigned color;
        LayoutObject* obj;
        if (layout->elements[index].clonedItemId != 0) {
            obj = (LayoutObject*)SpdManager_GetSquareAt(layout->spdRoot, layout->elements[index].clonedItemId);
        } else {
            obj = (LayoutObject*)SpdManager_GetSquareAt(layout->spdRoot, layout->items[index].id);
        }
        if (hovered) {
            float z;
            float y;
            z = *(float*)((char*)&layout->items[index]+0x14);
            float x = *(float*)((char*)&layout->items[index]+0xc);
            y = *(float*)((char*)&layout->items[index]+0x10);
            position[0] = x; position[1] = y;
            scale.x = z; scale.y = z;
        }
        temp[0] = position[0] + (*(LayoutElement* volatile*)&layout->elements)[index].positionX;
        temp[1] = position[1] + (*(LayoutElement* volatile*)&layout->elements)[index].positionY;
        Sprite_SetPosition2D(obj,temp);
        temp[0] = (*(LayoutElement* volatile*)&layout->elements)[index].originX; temp[1] = (*(LayoutElement* volatile*)&layout->elements)[index].originY; Sprite_SetOrigin2D(obj,temp);
        temp[0] = scale.x * (*(LayoutElement* volatile*)&layout->elements)[index].scaleX; temp[1] = scale.y * (*(LayoutElement* volatile*)&layout->elements)[index].scaleY; Sprite_SetScale2D(obj,temp);
        Sprite_SetSourceSize2D(obj, &layout->elements[index].sourceWidth);
        color = layout->elements[index].packedColor;
        unsigned faded;
        if (t <= 0.0f) faded = 0;
        else if (t >= 1.0f) faded = color;
        else faded = (color & 0xff00) | (unsigned char)Runtime_ConvertDoubleToUnsigned((float)(unsigned char)color * t) | (color & 0xffff0000);
        Sprite_SetPackedColor(obj, faded);
        SpdGeometry_SetRotationRadians(obj, layout->elements[index].rotationRadians);
        obj->v1();
        SimpleSpd_DrawBpSprite(obj);
    }
        }
    }
}
#pragma pop
}
