// Screen mouse cursor: where the pointer is over the golf HUD, the toolbar, the playfield and NPCs,
// which action it selects, and how it moves and saves its position.
// Also creates NPC objects with their child lists.

#include <decomp/static_init.h>
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "classes/CMouseCursor.h"
#include "classes/InputPointerState.h"
#include "classes/COption.h"
#define AsCMouseCursor(p) ((CMouseCursor*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/types.h"

#include "sdk_protos.h"
#include "classes/CNPC.h"

// Coordinates: screen X/Y use the view's logical pixel dimensions (Y increases
// downward); world geometry uses X/Z horizontally and Y vertically. Hit testing
// writes hitRegion and action-enable flags; a return value is an input response,
// not the region ID. Bounds helpers include edges; circles/annuli use strict tests.
// Camera flags are corroborated by camera_input_and_ground_view.cpp/camera_movement_and_input.cpp. Rule 1 has match-play
// consumers (CPangInfo_CalculateMatchPlayHoleRewards); remaining mode names are
// deliberately numeric until a gameplay identity is established.
enum CursorEligibility {
    kCameraForwardOrFreeHeight = 0x300,
    kCameraNamedViewOnly = 0x8000,
    kRuleMatchPlay = 1,
    kRuleTutorialControls = 2,
    kTutorialControlStageEnd = 5,
    kTutorialStageCount = 6,
    kGameMode5 = 5,
    kSettingsMode3 = 3,
    kControlMode2 = 2
};
enum CursorResponse {
    kCursorNoAction = 0,
    kCursorHit = 1,
    kCursorHeldAction = 2,
    kCursorTopEdge = 4,
    kCursorBottomEdge = 5
};
enum CursorRegion {
    kCursorLeftHudRectangle = 1,
    kCursorLeftHudCircle = 2,
    kCursorHeldLeftCircle = 3,
    kCursorUpperAnnularSector = 6,
    kCursorLowerAnnularSector = 7,
    kCursorAlternateHudRow = 0x12,
    kCursorRightHud = 0x13,
    kCursorTimeLimit = 0x14,
    kCursorOverlayBlock = 0x15,
    kCursorRadialSectorNeg3 = 0x1B,
    kCursorRadialSectorNeg2 = 0x1C,
    kCursorBottomToolbar0 = 0x3F,
    kCursorBottomToolbar1 = 0x40,
    kCursorBottomToolbar2 = 0x41,
    kCursorBottomToolbar3 = 0x42,
    kCursorBottomToolbar4 = 0x47,
    kCursorUpperToolbar0 = 0x43,
    kCursorUpperToolbar1 = 0x44,
    kCursorNoRegion = 0,
    kCursorTopEdgeRegion = 8,
    kCursorBottomEdgeRegion = 9,
    kCursorPlayerPanelBase = 10,
    kCursorPlayerBarBase = 14,
    kCursorRadialCenter = 0x16,
    kCursorPlayfield = 0x3E,
    kCursorClubMenu = 5,
    kCursorPanLeft = 0x17,
    kCursorPanRight = 0x18,
    kCursorRaiseTarget = 0x19,
    kCursorLowerTarget = 0x1A,
    kCursorTutorialAction = 0x1F,
    kCursorDelayedTutorial = 0x20,
    kCursorOneShotClose = 0x23
};
// Confirmed consumers: camera_movement_and_input.cpp camera movement handles 8/9 and 0x17..1A;
// ingame_hud_screen.cpp HUD dispatch handles club menu 5, player entries 10..13, tutorial
// action 0x1F, delayed control 0x20 and one-shot close 0x23. Other labels unresolved.
// First-match order is policy: player/tutorial -> edges -> radial -> ordinary HUD.
// Observed region map:
// 1/2: left HUD rectangle/circle; 3: held/idle left circle; 5: club-menu area;
// 6/7: two annular sectors; 8/9: top/bottom screen edge;
// 10..13: player panels, 14..17: their narrow bars; 0x12: variable-width row;
// 0x13/14/15: right HUD/timeout/overlay; 0x16: radial center;
// radial sectors floor(degrees/60): -3/-2/-1/0/1/2 -> 0x1B/1C/17/18/1A/19;
// 0x1F/20/23: tutorial/mode-specific rectangles; 0x3E: world playfield;
// 0x3F/40/41/42/47: bottom toolbar; 0x43/44: upper-right toolbar pair.

extern "C" {

// Empty cursor subsystem hook.
void MouseCursor_EmptyHook();

void Vec2_Copy(void*, void*);
void Pointer_SetPositionClamped(void*, float, float);
struct InputPointerStateView {
    char pointerPrefix[0x54];
    int screenXBits;
    int screenYBits;
    int screenZBits;
};
extern InputPointerStateView* g_inputPointer;
extern void* g_view;
extern const float kFloat_0_65_MouseCursor;
extern const float kFloat_150_MouseCursor;
extern const float kFloat_0_13_MouseCursor;
extern const float kFloat_0_MouseCursor;
extern const float kFloat_0_00001_MouseCursor;
extern const float kFloat_416_MouseCursor;
extern const float kFloat_454_MouseCursor;
extern const float kFloat_428_MouseCursor;
extern const float kFloat_466_MouseCursor;
extern const float kFloat_459_MouseCursor;
extern const float kFloat_497_MouseCursor;
extern const float kFloat_502_MouseCursor;
extern const float kFloat_540_MouseCursor;
extern const float kFloat_545_MouseCursor;
extern const float kFloat_583_MouseCursor;
extern const float kFloat_588_MouseCursor;
extern const float kFloat_626_MouseCursor;
extern const float kFloat_560_MouseCursor;
extern const float kFloat_635_MouseCursor;
extern const float kFloat_115_MouseCursor;
extern const float kFloat_135_MouseCursor;
extern const float kFloat_485_MouseCursor;
extern const float kFloat_1_MouseCursor;
extern const float kFloat_300_MouseCursor;
extern const float kFloat_neg1_MouseCursor;
extern char s_cameraTransform[0x100];
void Pointer_SetPositionClamped(void*, float, float);
struct Vec2SubtractResult {
    void* a;
    void* b;
};
Vec2SubtractResult Vec2_Subtract(void*, void*);
void Vec2_Copy(void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void Normalize__7WVectorFv(void*);
void __amu__7WVectorFf(void*, float);
void __ml__FfRC7WVector(void*, void*, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void* g_view;
void Camera_ScreenToRay(float*, void*, float, float, float);

// Restore the saved position when enabling pointer input; save it when disabling.

// Enable the radial menu and animate the cursor toward its default position.

// Select a normal cursor state or mark an extended state using the extra flag.

// Copy the input pointer X/Y bit patterns while preserving the original volatile samples.

void __ct__18NativeBase80201D18Fv(void*);
extern char __vt__4CNPC[0x100];
struct CNPCView {
    void* vtable;
    char baseObject[0x14];
    int childCount;
    void* childPrevious;
    void* childNext;
    // Construct the native base and an empty circular NPC child-list header.
    CNPCView() {
        __ct__18NativeBase80201D18Fv(this);
        vtable = &__vt__4CNPC;
        childCount = 0;
        // Keep byte-based sentinel formation: taking the field address changes register allocation.
        childNext = (char*)this + (int)&((CNPCView*)0)->childPrevious;
        childPrevious = (char*)this + (int)&((CNPCView*)0)->childPrevious;
    }
};
// Construct an NPC with an empty circular child list.
CNPCView* CNPC_Create();

extern const float kFloat_6_2831855;
extern float s_twoPi_8019631C;
// Initialize the cursor unit's writable two-pi cache.
void MouseCursor_InitTwoPiCache();

void Vec2_Copy(void*, void*);
// Save the current screen position for a later pointer-input transition.

void appFreeUni__FPv(void*);
struct fn_801978F0_VT {
    char pad[0x7C];
    void (*dtor)(void*, int);
};
struct NpcChildObject {
    virtual void unused00();
    virtual void unused01();
    virtual void unused02();
    virtual void unused03();
    virtual void unused04();
    virtual void unused05();
    virtual void unused06();
    virtual void unused07();
    virtual void unused08();
    virtual void unused09();
    virtual void unused10();
    virtual void unused11();
    virtual void unused12();
    virtual void unused13();
    virtual void unused14();
    virtual void unused15();
    virtual void unused16();
    virtual void unused17();
    virtual void unused18();
    virtual void unused19();
    virtual void unused20();
    virtual void unused21();
    virtual void unused22();
    virtual void unused23();
    virtual void unused24();
    virtual void unused25();
    virtual void unused26();
    virtual void unused27();
    virtual void unused28();
    virtual ~NpcChildObject();
};
struct NpcChildLinkView {
    NpcChildLinkView* prev;
    NpcChildLinkView* next;
    NpcChildObject* item;
};
struct fn_801978F0_List {
    char baseObject[0x18];
    int count;
    NpcChildLinkView head;
};
#pragma opt_propagation off
// Destroy every owned NPC item, then unlink and free all child-list nodes.
#pragma opt_propagation reset

// Move to screen X/Y immediately for negative duration, otherwise start a weighted move.
// Duration uses the cursor update's time unit; zero follows the existing 1/duration
// path and is not special-cased. This routine always sets moveToActive, even for a snap.

// Test whether the screen cursor is inside inclusive X/Y bounds.
static inline int CursorWithinBounds(unsigned char* p, const float& x0, const float& x1, const float& y0,
                                     const float& y1) {
    float x = AsCMouseCursor(p)->screenX;
    if (x >= x0 && x <= x1) {
        float y = AsCMouseCursor(p)->screenY;
        if (y >= y0 && y <= y1)
            return 1;
    }
    return 0;
}

// Identify toolbar hit regions, or intersect the pointer ray with the golf playfield.

extern void* g_sharedDoc;
extern void* g_option;
extern char s_cameraTransform[0x100];
extern int GolfTask_NoOpStatus();
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern int Pointer_GetButtonState(void*, int);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float WVectorLen__FRC7WVector(void*);
extern float Vector_SignedAngleXZ(void*, void*);
extern int GolfTask_NoOpStatus();
struct CursorModeSettings {
    char prefix[0xA];
    unsigned char mode;
};
struct CursorInstanceContext {
    char prefix[4];
    CursorModeSettings* settings;
    char reserved[0x58];
    int controlMode;
};
extern void* InstanceTable_Get();
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern int Pointer_GetButtonState(void*, int);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float WVectorLen__FRC7WVector(void*);
extern float Math_SquareRootApprox(float);
extern double atan(double);

extern const float kFloat_20_MouseCursor;
extern const float kFloat_118_MouseCursor;
extern const float kFloat_127_MouseCursor;
extern const float kFloat_49_MouseCursor;
extern const float kFloat_4_MouseCursor;
extern const float kFloat_59_MouseCursor;
extern const float kFloat_65_MouseCursor;
extern const float kFloat_7_MouseCursor;
extern const float kFloat_74_MouseCursor;
extern const float kFloat_43_MouseCursor;
extern const float kFloat_47_MouseCursor;
extern const float kFloat_45_MouseCursor;
extern const float kFloat_123_MouseCursor;
extern const float kFloat_30_MouseCursor;
extern const float kFloat_64_MouseCursor;
extern const float kFloat_19_MouseCursor;
extern const float kFloat_50_MouseCursor;
extern const float kFloat_130_MouseCursor;
extern const float kFloat_150_MouseCursor;
extern const float kFloat_245_MouseCursor;
extern const float kFloat_275_MouseCursor;
extern const float kFloat_2_MouseCursor;
extern const float kFloat_0_65_MouseCursor;
extern const float kFloat_42_MouseCursor;
extern const float kFloat_15_MouseCursor;
extern const float kFloat_57_29578_MouseCursor;
extern const float kFloat_60_MouseCursor;
extern const float kFloat_119_MouseCursor;
extern const float kFloat_109_MouseCursor;
extern const float kFloat_32_MouseCursor;
extern const float kFloat_34_MouseCursor;
extern const float kFloat_361_MouseCursor;
extern const float kFloat_1600_MouseCursor;
extern const float kFloat_88_MouseCursor;
extern const float kFloat_40_MouseCursor;
extern const float kFloat_100_MouseCursor;
extern const float kFloat_144_MouseCursor;
extern const float kFloat_521_MouseCursor;
extern const float kFloat_6_MouseCursor;
extern const float kFloat_31_MouseCursor;
extern const float kFloat_17_MouseCursor;
extern const float kFloat_14_MouseCursor;
extern const float kFloat_25_MouseCursor;
extern const float kFloat_67_MouseCursor;
extern const float kFloat_2025_MouseCursor;
extern const float kFloat_625_MouseCursor;
extern const float kFloat_132_MouseCursor;
extern const float kFloat_70_MouseCursor;
extern const float kFloat_75_MouseCursor;
extern const float kFloat_10_MouseCursor;

struct ScreenRect {
    float x, y, w, h;
};

extern void* InstanceTable_Get();

// Clamp a screen coordinate to the supplied inclusive range.
static inline float ClampScreenCoordinate(float v, float lo, float hi) {
    return (v < lo) ? lo : ((v > hi) ? hi : v);
}

// Test whether the cursor lies inside a rectangle given by origin and size.
static inline int CursorWithinRect(unsigned char* p, const ScreenRect& r) {
    float x = AsCMouseCursor(p)->screenX;
    float y = AsCMouseCursor(p)->screenY;
    if (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h)
        return 1;
    return 0;
}

#pragma opt_common_subs off
// Clamp the cursor and identify the active golf HUD or radial-menu action.
// First matching region wins. Returns 0/1/2/4/5 per CursorResponse; some paths
// return 0 with a nonzero region (overlay), and no golfDoc returns without
// clearing old region/flags. Callers must not treat return == 0 as a full reset.

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462168", _ctors_u8019631C_0, MouseCursor_InitTwoPiCache);
#pragma pop

// Resolve ordinary HUD shapes in first-match order, including overlay blocking and the no-hit reset.
// Screen coordinates are logical pixels; option hudBaselineY anchors the movable HUD group.
static inline int HitOrdinaryGolfHud(unsigned char*& cursor) {
    // Phase 5: test ordinary HUD shapes relative to the configurable baseline.
    // Squared thresholds 361/1600/100/2025/625 are radii 19/40/10/45/25 squared.
    float hudBaselineY = AsCOption(g_option)->hudBaselineY;
    {
        float x = AsCMouseCursor(cursor)->screenX;
        if (x >= kFloat_64_MouseCursor && x <= kFloat_119_MouseCursor) {
            float y;
            float y0 = hudBaselineY - kFloat_109_MouseCursor;
            y = AsCMouseCursor(cursor)->screenY;
            if (y >= y0 && y <= kFloat_45_MouseCursor + y0) {
                AsCMouseCursor(cursor)->secondaryActionEnabled = 1;
                AsCMouseCursor(cursor)->hitRegion = kCursorLeftHudRectangle;
                return 1;
            }
        }
    }
    {
        float dy = AsCMouseCursor(cursor)->screenY - (hudBaselineY - kFloat_34_MouseCursor);
        float dx = AsCMouseCursor(cursor)->screenX - kFloat_32_MouseCursor;
        if (dx * dx + dy * dy < kFloat_361_MouseCursor) {
            AsCMouseCursor(cursor)->hitRegion = kCursorLeftHudCircle;
            return 1;
        }
    }
    if (((CursorInstanceContext*)InstanceTable_Get())->settings->mode != kSettingsMode3) {
        float w = AsWView(g_view)->viewportWidth;
        float dy = AsCMouseCursor(cursor)->screenY - (kFloat_4_MouseCursor + hudBaselineY);
        float dx = AsCMouseCursor(cursor)->screenX - (w - kFloat_50_MouseCursor);
        // Original +0x9C4 rounds x*x, then fuses y*y into that squared-distance sum.
        if (__fmadds(dy, dy, dx * dx) < kFloat_1600_MouseCursor) {
            AsCMouseCursor(cursor)->hitRegion = kCursorRightHud;
            return 1;
        }
        float dy2;
        float dx2 = AsCMouseCursor(cursor)->screenX - (w - kFloat_20_MouseCursor);
        dy2 = AsCMouseCursor(cursor)->screenY - (kFloat_32_MouseCursor + hudBaselineY);
        if (__fmadds(dy2, dy2, dx2 * dx2) < kFloat_361_MouseCursor) {
            AsCMouseCursor(cursor)->hitRegion = kCursorRightHud;
            return 1;
        }
    }
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs) != 0) {
        float dy = AsCMouseCursor(cursor)->screenY - (kFloat_40_MouseCursor + hudBaselineY);
        float dx = AsCMouseCursor(cursor)->screenX - kFloat_88_MouseCursor;
        if (dx * dx + dy * dy < kFloat_100_MouseCursor) {
            AsCMouseCursor(cursor)->hitRegion = kCursorTimeLimit;
            return 1;
        }
    }
    if (AsCMouseCursor(cursor)->alternateHudMode == 0 &&
        ((CursorInstanceContext*)InstanceTable_Get())->controlMode == kControlMode2) {
        if (((CursorInstanceContext*)InstanceTable_Get())->settings->mode != kSettingsMode3) {
            float x = AsCMouseCursor(cursor)->screenX;
            if (x >= kFloat_144_MouseCursor && x <= kFloat_521_MouseCursor) {
                float y;
                float y0 = hudBaselineY - kFloat_6_MouseCursor;
                y = AsCMouseCursor(cursor)->screenY;
                if (y >= y0 && y <= kFloat_31_MouseCursor + y0) {
                    AsCMouseCursor(cursor)->hitRegion = kCursorClubMenu;
                    return 1;
                }
            }
        } else {
            float x = AsCMouseCursor(cursor)->screenX;
            if (x >= kFloat_144_MouseCursor && x <= kFloat_521_MouseCursor) {
                float y;
                float y0 = hudBaselineY - kFloat_17_MouseCursor;
                y = AsCMouseCursor(cursor)->screenY;
                if (y >= y0 && y <= kFloat_42_MouseCursor + y0) {
                    AsCMouseCursor(cursor)->hitRegion = kCursorClubMenu;
                    return 1;
                }
            }
        }
        float x = AsCMouseCursor(cursor)->screenX;
        if (x >= kFloat_119_MouseCursor && x <= kFloat_144_MouseCursor) {
            float y;
            float y0 = kFloat_14_MouseCursor + hudBaselineY;
            y = AsCMouseCursor(cursor)->screenY;
            if (y >= y0 && y <= kFloat_25_MouseCursor + y0) {
                AsCMouseCursor(cursor)->hitRegion = kCursorClubMenu;
                return 1;
            }
        }
    }
    if (((CursorInstanceContext*)InstanceTable_Get())->settings->mode != kSettingsMode3) {
        float dy = AsCMouseCursor(cursor)->screenY - (kFloat_4_MouseCursor + hudBaselineY);
        float dx = AsCMouseCursor(cursor)->screenX - kFloat_67_MouseCursor;
        float d2 = dx * dx + dy * dy;
        if (d2 < kFloat_2025_MouseCursor) {
            if (Pointer_GetButtonState(g_inputPointer, 0) > 0) {
                AsCMouseCursor(cursor)->hitRegion = kCursorHeldLeftCircle;
                return 2;
            }
        }
        if (d2 < kFloat_625_MouseCursor) {
            if (Pointer_GetButtonState(g_inputPointer, 0) == 0) {
                AsCMouseCursor(cursor)->hitRegion = kCursorHeldLeftCircle;
                return 1;
            }
        }
    }
    if (AsCMouseCursor(cursor)->alternateHudMode != 0 && AsCMouseCursor(cursor)->screenX > kFloat_132_MouseCursor) {
        unsigned char* currentPlayer = (unsigned char*)GolfPlayer_GetEntryByIndex(
            (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc),
            AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer);
        float rowItemCount = (float)currentPlayer[0x9C];
        float rowRight = kFloat_15_MouseCursor + (kFloat_132_MouseCursor + kFloat_49_MouseCursor * rowItemCount);
        if (AsCMouseCursor(cursor)->screenX < rowRight) {
            float y;
            float y1 = hudBaselineY - kFloat_25_MouseCursor;
            y = AsCMouseCursor(cursor)->screenY;
            if (y > y1 && y < kFloat_50_MouseCursor + y1) {
                AsCMouseCursor(cursor)->secondaryActionEnabled = 1;
                AsCMouseCursor(cursor)->hitRegion = kCursorAlternateHudRow;
                return 1;
            }
        }
    }
    {
        float fx = AsCMouseCursor(cursor)->screenX - kFloat_67_MouseCursor;
        float fy = (kFloat_4_MouseCursor + hudBaselineY) - AsCMouseCursor(cursor)->screenY;
        float ang = kFloat_57_29578_MouseCursor * (float)atan((double)(fy / fx));
        float dist = Math_SquareRootApprox(fx * fx + fy * fy);
        if (AsCMouseCursor(cursor)->screenX > kFloat_67_MouseCursor && dist > kFloat_45_MouseCursor &&
            dist < kFloat_70_MouseCursor) {
            if (ang > kFloat_40_MouseCursor && ang < kFloat_75_MouseCursor) {
                AsCMouseCursor(cursor)->hitRegion = kCursorUpperAnnularSector;
                return 1;
            }
            if (ang > kFloat_10_MouseCursor && ang < kFloat_40_MouseCursor) {
                AsCMouseCursor(cursor)->hitRegion = kCursorLowerAnnularSector;
                return 1;
            }
        }
    }
    {
        float x;
        float bx = AsCMouseCursor(cursor)->overlayAnchorX - kFloat_32_MouseCursor;
        float by = AsCMouseCursor(cursor)->overlayAnchorY - kFloat_47_MouseCursor;
        x = AsCMouseCursor(cursor)->screenX;
        if (x >= bx && x <= kFloat_64_MouseCursor + bx) {
            float y = AsCMouseCursor(cursor)->screenY;
            if (y >= by && y <= kFloat_64_MouseCursor + by) {
                AsCMouseCursor(cursor)->primaryActionEnabled = 0;
                AsCMouseCursor(cursor)->hitRegion = kCursorOverlayBlock;
                return 0;
            }
        }
    }
    // No shape matched: clear both action channels and the region together.
    AsCMouseCursor(cursor)->primaryActionEnabled = 0;
    AsCMouseCursor(cursor)->secondaryActionEnabled = 0;
    AsCMouseCursor(cursor)->hitRegion = kCursorNoRegion;
    return 0;
}

// Game entry points in original address order.
extern "C" {

#pragma push
#pragma opt_common_subs reset
// Test fixed toolbar bounds, then intersect the pointer ray with the world playfield.
int CMouseCursor::HitTestRegions(void* mouseCursor) {
    // Logical screen pixels; shared constants keep their original storage.
#define kBottomToolbarTop kFloat_428_MouseCursor
#define kBottomToolbarBottom kFloat_466_MouseCursor
#define kBottomToolbar0Left kFloat_416_MouseCursor
#define kPlayfieldBottomY kFloat_416_MouseCursor
#define kBottomToolbar0Right kFloat_454_MouseCursor
#define kBottomToolbar1Left kFloat_459_MouseCursor
#define kBottomToolbar1Right kFloat_497_MouseCursor
#define kBottomToolbar2Left kFloat_502_MouseCursor
#define kBottomToolbar2Right kFloat_540_MouseCursor
#define kBottomToolbar3Left kFloat_545_MouseCursor
#define kBottomToolbar3Right kFloat_583_MouseCursor
#define kBottomToolbar4Left kFloat_588_MouseCursor
#define kBottomToolbar4Right kFloat_626_MouseCursor
#define kUpperToolbarDividerX kFloat_560_MouseCursor
#define kUpperToolbar0Right kFloat_635_MouseCursor
#define kUpperToolbar1Left kFloat_485_MouseCursor
#define kUpperToolbarTop kFloat_115_MouseCursor
#define kUpperToolbarBottom kFloat_135_MouseCursor
    unsigned char* cursor = (unsigned char*)mouseCursor;
    if (CursorWithinBounds(cursor, kBottomToolbar0Left, kBottomToolbar0Right, kBottomToolbarTop,
                           kBottomToolbarBottom)) {
        AsCMouseCursor(cursor)->hitRegion = kCursorBottomToolbar0;
        return 1;
    }
    if (CursorWithinBounds(cursor, kBottomToolbar1Left, kBottomToolbar1Right, kBottomToolbarTop,
                           kBottomToolbarBottom)) {
        AsCMouseCursor(cursor)->hitRegion = kCursorBottomToolbar1;
        return 1;
    }
    if (CursorWithinBounds(cursor, kBottomToolbar2Left, kBottomToolbar2Right, kBottomToolbarTop,
                           kBottomToolbarBottom)) {
        AsCMouseCursor(cursor)->hitRegion = kCursorBottomToolbar2;
        return 1;
    }
    if (CursorWithinBounds(cursor, kBottomToolbar3Left, kBottomToolbar3Right, kBottomToolbarTop,
                           kBottomToolbarBottom)) {
        AsCMouseCursor(cursor)->hitRegion = kCursorBottomToolbar3;
        return 1;
    }
    if (CursorWithinBounds(cursor, kBottomToolbar4Left, kBottomToolbar4Right, kBottomToolbarTop,
                           kBottomToolbarBottom)) {
        AsCMouseCursor(cursor)->hitRegion = kCursorBottomToolbar4;
        return 1;
    }
    if (CursorWithinBounds(cursor, kUpperToolbarDividerX, kUpperToolbar0Right, kUpperToolbarTop, kUpperToolbarBottom)) {
        AsCMouseCursor(cursor)->hitRegion = kCursorUpperToolbar0;
        return 1;
    }
    if (CursorWithinBounds(cursor, kUpperToolbar1Left, kUpperToolbarDividerX, kUpperToolbarTop, kUpperToolbarBottom)) {
        AsCMouseCursor(cursor)->hitRegion = kCursorUpperToolbar1;
        return 1;
    }

    // Toolbar misses fall through to world picking. The normalized camera ray
    // is scaled to a 300-world-unit segment; the camera origin is transform +24.
    float scaledRay[3];
    float ray[3];
    Camera_ScreenToRay(ray, g_view, AsCMouseCursor(cursor)->screenX, AsCMouseCursor(cursor)->screenY,
                       kFloat_1_MouseCursor);
    __as__7WVectorFRC7WVector(cursor + 0xAC, ray);
    Normalize__7WVectorFv(cursor + 0xAC);
    __amu__7WVectorFf(cursor + 0xAC, kFloat_300_MouseCursor);

    float absoluteRayY;
    unsigned int rayYBits = (*(unsigned int*)&AsCMouseCursor(cursor)->rayDirectionY);
    if (rayYBits & 0x80000000)
        absoluteRayY = -AsCMouseCursor(cursor)->rayDirectionY;
    else
        absoluteRayY = AsCMouseCursor(cursor)->rayDirectionY;

    float intersectionScale;
    int rayIsHorizontal = absoluteRayY < kFloat_0_00001_MouseCursor;
    if (rayIsHorizontal) {
        intersectionScale = kFloat_1_MouseCursor;
    } else {
        char* cameraTransform = s_cameraTransform;
        // Solve originY + t*rayY == 0. X/Z bounds are tested at that world point.
        intersectionScale =
            kFloat_neg1_MouseCursor * *(float*)(cameraTransform + 0x28) / AsCMouseCursor(cursor)->rayDirectionY;
        AsCMouseCursor(cursor)->rayIntersectionScale = intersectionScale;
        __ml__FfRC7WVector(scaledRay, cursor + 0xAC, intersectionScale);
        float worldIntersection[3];
        __pl__FRC7WVectorRC7WVector(worldIntersection, cameraTransform + 0x24, scaledRay);
        if (worldIntersection[0] > AsCMouseCursor(cursor)->minWorldX &&
            worldIntersection[0] < AsCMouseCursor(cursor)->maxWorldX &&
            worldIntersection[2] > AsCMouseCursor(cursor)->minWorldZ &&
            worldIntersection[2] < AsCMouseCursor(cursor)->maxWorldZ) {
            intersectionScale = AsCMouseCursor(cursor)->rayIntersectionScale;
        } else {
            intersectionScale = kFloat_1_MouseCursor;
        }
    }

    if (intersectionScale < kFloat_1_MouseCursor) {
        if (AsCMouseCursor(cursor)->screenY < kPlayfieldBottomY) {
            AsCMouseCursor(cursor)->hitRegion = kCursorPlayfield;
            return kCursorHit;
        }
    }
    return 0;
}
#undef kBottomToolbarTop
#undef kBottomToolbarBottom
#undef kBottomToolbar0Left
#undef kPlayfieldBottomY
#undef kBottomToolbar0Right
#undef kBottomToolbar1Left
#undef kBottomToolbar1Right
#undef kBottomToolbar2Left
#undef kBottomToolbar2Right
#undef kBottomToolbar3Left
#undef kBottomToolbar3Right
#undef kBottomToolbar4Left
#undef kBottomToolbar4Right
#undef kUpperToolbarDividerX
#undef kUpperToolbar0Right
#undef kUpperToolbar1Left
#undef kUpperToolbarTop
#undef kUpperToolbarBottom
#pragma pop

// Choose the first eligible player, tutorial, radial or HUD action for the screen cursor.
int CMouseCursor::UpdateGolfActions(void* mouseCursor) {
    ScreenRect hitRect;
    unsigned char* cursor = (unsigned char*)mouseCursor;
    unsigned char* sharedDocument = (unsigned char*)g_sharedDoc;
    if ((*(int*)&AsCSharedDoc(sharedDocument)->golfDoc) == 0)
        return 0;
    // Phase 1: constrain screen coordinates and synchronize the input pointer.
    unsigned char* clipRectangleX = AsCMouseCursor(cursor)->clipRect;
    AsCMouseCursor(cursor)->primaryActionEnabled = 1;
    AsCMouseCursor(cursor)->secondaryActionEnabled = 0;
    if (clipRectangleX != 0) {
        float clipLeft = ((ScreenRect*)clipRectangleX)->x;
        float clipRight = clipLeft + ((ScreenRect*)clipRectangleX)->w;
        AsCMouseCursor(cursor)->screenX = ClampScreenCoordinate(AsCMouseCursor(cursor)->screenX, clipLeft, clipRight);
        unsigned char* clipRectangleY = AsCMouseCursor(cursor)->clipRect;
        float clipTop = ((ScreenRect*)clipRectangleY)->y;
        float clipBottom = clipTop + ((ScreenRect*)clipRectangleY)->h;
        AsCMouseCursor(cursor)->screenY = ClampScreenCoordinate(AsCMouseCursor(cursor)->screenY, clipTop, clipBottom);
        Pointer_SetPositionClamped(g_inputPointer, AsCMouseCursor(cursor)->screenX, AsCMouseCursor(cursor)->screenY);
    }
    // Phase 2: test player panels or tutorial/tutorialStage controls before generic HUD controls.
    if (GolfTask_NoOpStatus() != 0) {
        unsigned char* panelDocument = (unsigned char*)g_sharedDoc;
        unsigned char* golfDocument = (*(unsigned char**)&AsCSharedDoc(panelDocument)->golfDoc);
        if (!((*(unsigned int*)&AsCGolfDoc(golfDocument)->gameStateFlags) & kCameraForwardOrFreeHeight)) {
            if (AsCSharedDoc(panelDocument)->golfRuleType == kRuleMatchPlay) {
                unsigned char* viewState = (unsigned char*)g_view;
                hitRect.y = kFloat_20_MouseCursor;
                float panelWidth = kFloat_118_MouseCursor;
                hitRect.w = panelWidth;
                hitRect.h = kFloat_127_MouseCursor;
                float secondaryRectY = kFloat_45_MouseCursor;
                float barOffsetY = kFloat_59_MouseCursor;
                float barWidth = kFloat_65_MouseCursor;
                float barHeight = kFloat_7_MouseCursor;
                float secondaryRectWidth = kFloat_43_MouseCursor;
                float secondaryRectHeight = kFloat_47_MouseCursor;
                for (int i = 0; i < 2; i++) {
                    int odd = i & 1;
                    float xx = odd ? (*(float*)(viewState + 0x188) - panelWidth) : kFloat_0_MouseCursor;
                    hitRect.x = xx;
                    if (CursorWithinRect(cursor, hitRect)) {
                        float add = odd ? kFloat_49_MouseCursor : kFloat_4_MouseCursor;
                        hitRect.x = hitRect.x + add;
                        hitRect.y = hitRect.y + barOffsetY;
                        hitRect.w = barWidth;
                        hitRect.h = barHeight;
                        if (CursorWithinRect(cursor, hitRect)) {
                            AsCMouseCursor(cursor)->hitRegion = i + kCursorPlayerBarBase;
                            return 1;
                        }
                        float xx2 = odd ? (*(float*)(viewState + 0x188) - panelWidth) : kFloat_74_MouseCursor;
                        hitRect.x = xx2;
                        hitRect.w = secondaryRectWidth;
                        hitRect.h = secondaryRectHeight;
                        for (int k = 0;
                             k < AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(panelDocument)->golfDoc))->playerCount;
                             k++) {
                            if (k == i) {
                                hitRect.y = secondaryRectY;
                                if (CursorWithinRect(cursor, hitRect)) {
                                    AsCMouseCursor(cursor)->hitRegion = k + kCursorPlayerPanelBase;
                                    return 1;
                                }
                            }
                        }
                    }
                }
            } else {
                float panelWidth;
                float panelHeight = kFloat_64_MouseCursor;
                panelWidth = kFloat_118_MouseCursor;
                for (unsigned char j = 0;
                     j < AsCGolfDoc((*(unsigned char**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount; j++) {
                    unsigned char* item =
                        (unsigned char*)GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), j);
                    if (*(int*)(item + 8) != 3) {

                        int rightMask = j & 2;
                        float xx = rightMask ? (AsWView(g_view)->viewportWidth - panelWidth) : kFloat_0_MouseCursor;
                        hitRect.x = xx;
                        int b1 = j & 1;
                        float yy = b1 ? kFloat_123_MouseCursor : kFloat_30_MouseCursor;
                        hitRect.y = yy;
                        hitRect.w = panelWidth;
                        hitRect.h = panelHeight;
                        if (CursorWithinRect(cursor, hitRect)) {
                            float add = rightMask ? kFloat_49_MouseCursor : kFloat_4_MouseCursor;
                            hitRect.x = hitRect.x + add;
                            hitRect.y = hitRect.y + kFloat_19_MouseCursor;
                            hitRect.w = kFloat_65_MouseCursor;
                            hitRect.h = kFloat_7_MouseCursor;
                            if (CursorWithinRect(cursor, hitRect)) {
                                AsCMouseCursor(cursor)->hitRegion = j + kCursorPlayerBarBase;
                                return 1;
                            }
                            AsCMouseCursor(cursor)->hitRegion = j + kCursorPlayerPanelBase;
                            return 1;
                        }
                    }
                }
            }
        }
    } else {
        unsigned char* tutorialDocument = (unsigned char*)g_sharedDoc;
        unsigned char* golfDocument = (*(unsigned char**)&AsCSharedDoc(tutorialDocument)->golfDoc);
        int tutorialStage = AsCGolfDoc(golfDocument)->tutorialStage;
        if (tutorialStage < kTutorialStageCount) {
            if (AsCSharedDoc(tutorialDocument)->golfRuleType == kRuleTutorialControls) {
                if (tutorialStage < kTutorialControlStageEnd) {
                    float leftEdge = AsWView(g_view)->viewportWidth - kFloat_50_MouseCursor;
                    int hit = AsCMouseCursor(cursor)->screenX >= leftEdge &&
                              AsCMouseCursor(cursor)->screenX <= kFloat_50_MouseCursor + leftEdge &&
                              AsCMouseCursor(cursor)->screenY >= kFloat_130_MouseCursor &&
                              AsCMouseCursor(cursor)->screenY <= kFloat_150_MouseCursor;
                    if (hit) {
                        AsCMouseCursor(cursor)->hitRegion = kCursorTutorialAction;
                        return 1;
                    }
                }
                int hit = AsCMouseCursor(cursor)->screenX >= kFloat_0_MouseCursor &&
                          AsCMouseCursor(cursor)->screenX <= kFloat_30_MouseCursor &&
                          AsCMouseCursor(cursor)->screenY >= kFloat_245_MouseCursor &&
                          AsCMouseCursor(cursor)->screenY <= kFloat_275_MouseCursor;
                if (hit) {
                    AsCMouseCursor(cursor)->hitRegion = kCursorDelayedTutorial;
                    return 1;
                }
            }
        } else {
            if (AsCSharedDoc(tutorialDocument)->gameMode == kGameMode5) {
                int hit = AsCMouseCursor(cursor)->screenX >= kFloat_0_MouseCursor &&
                          AsCMouseCursor(cursor)->screenX <= kFloat_30_MouseCursor &&
                          AsCMouseCursor(cursor)->screenY >= kFloat_245_MouseCursor &&
                          AsCMouseCursor(cursor)->screenY <= kFloat_275_MouseCursor;
                if (hit) {
                    AsCMouseCursor(cursor)->hitRegion = kCursorOneShotClose;
                    return 1;
                }
            }
        }
    }

    // Phase 3: exact clamped screen-edge hits take priority over the radial menu.
    float edgeInset = kFloat_2_MouseCursor;
    float screenY = AsCMouseCursor(cursor)->screenY;
    if (screenY == edgeInset) {
        AsCMouseCursor(cursor)->hitRegion = kCursorTopEdgeRegion;
        return kCursorTopEdge;
    }
    if (screenY == AsWView(g_view)->viewportHeight - edgeInset) {
        AsCMouseCursor(cursor)->hitRegion = kCursorBottomEdgeRegion;
        return kCursorBottomEdge;
    }
    // Phase 4: latch a radial drag only when the original press was within radius
    // 42 of (0.65*viewWidth, viewHeight-150), then test the current radius/sector.
    if (Pointer_GetButtonState(g_inputPointer, 0) != 0) {
        Arr3 out;
        Arr3 dir;
        Arr3 target;
        float viewHeight = AsWView(g_view)->viewportHeight;
        float viewWidth = AsWView(g_view)->viewportWidth;
        target.v[1] = viewHeight - kFloat_150_MouseCursor;
        target.v[0] = kFloat_0_65_MouseCursor * viewWidth;
        target.v[2] = kFloat_0_MouseCursor;
        dir = *(Arr3*)&((InputPointerState*)g_inputPointer)
                   ->buttons[0]
                   .pressPosition[0]; // original pointer +0x64: button 0 press position
        __mi__FRC7WVectorRC7WVector(&out, &dir, &target);
        if (WVectorLen__FRC7WVector(&out) < kFloat_42_MouseCursor)
            AsCMouseCursor(cursor)->radialMenuDragActive = 1;
    }
    if (AsCMouseCursor(cursor)->radialMenuEnabled != 0 &&
        (AsCMouseCursor(cursor)->radialMenuDragActive != 0 || Pointer_GetButtonState(g_inputPointer, 0) == 0)) {
        float dy = AsCMouseCursor(cursor)->screenY - (AsWView(g_view)->viewportHeight - kFloat_150_MouseCursor);
        float dx = AsCMouseCursor(cursor)->screenX - AsWView(g_view)->viewportWidth * kFloat_0_65_MouseCursor;
        float dist = Math_SquareRootApprox(dx * dx + dy * dy);
        if (dist < kFloat_15_MouseCursor) {
            AsCMouseCursor(cursor)->hitRegion = kCursorRadialCenter;
            return 1;
        }
        if (dist < kFloat_42_MouseCursor) {
            Arr3 zAxis, radialOffset;
            float ddy = AsCMouseCursor(cursor)->screenY - (AsWView(g_view)->viewportHeight - kFloat_150_MouseCursor);
            float ddx = AsCMouseCursor(cursor)->screenX - AsWView(g_view)->viewportWidth * kFloat_0_65_MouseCursor;
            radialOffset.v[1] = kFloat_0_MouseCursor;
            radialOffset.v[0] = ddx;
            radialOffset.v[2] = ddy;
            zAxis.v[0] = kFloat_0_MouseCursor;
            zAxis.v[1] = kFloat_0_MouseCursor;
            zAxis.v[2] = kFloat_1_MouseCursor;
            float ang = Vector_SignedAngleXZ(&zAxis, &radialOffset);
            float fl = (float)floor(kFloat_57_29578_MouseCursor * ang / kFloat_60_MouseCursor);
            switch ((int)fl) {
            case -2:
                AsCMouseCursor(cursor)->hitRegion = kCursorRadialSectorNeg2;
                break;
            case -1:
                AsCMouseCursor(cursor)->hitRegion = kCursorPanLeft;
                break;
            case 0:
                AsCMouseCursor(cursor)->hitRegion = kCursorPanRight;
                break;
            case 1:
                AsCMouseCursor(cursor)->hitRegion = kCursorLowerTarget;
                break;
            case 2:
                AsCMouseCursor(cursor)->hitRegion = kCursorRaiseTarget;
                break;
            case -3:
                AsCMouseCursor(cursor)->hitRegion = kCursorRadialSectorNeg3;
                break;
            }
            if ((*(unsigned int*)&AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags) ==
                    kCameraNamedViewOnly &&
                (unsigned int)(AsCMouseCursor(cursor)->hitRegion - kCursorRadialSectorNeg3) <= 1) {
                AsCMouseCursor(cursor)->hitRegion = 0;
                return 0;
            }
            return 1;
        }
    }

    return HitOrdinaryGolfHud(cursor);
}

#pragma push
#pragma opt_common_subs reset
// Preserve the empty cursor subsystem hook.
void MouseCursor_EmptyHook() {}
#pragma pop

#pragma push
#pragma opt_common_subs reset
// Save or restore cursor position when pointer input is toggled.
void CMouseCursor::SetPointerInputEnabled(void* mouseCursor, int flag) {
    volatile char* p = (volatile char*)mouseCursor;
    if (flag != 0) {
        Vec2_Copy(((char*)&AsCMouseCursor(p)->screenX), &AsCMouseCursor(p)->savedScreenX); // original cursor +0x18
        float f0 = kFloat_0_MouseCursor;
        ((volatile CMouseCursor*)p)->cursorState = 0;
        float f1 = ((volatile CMouseCursor*)p)->screenX;
        AsCMouseCursor(p)->idleTime = f0;
        float f2 = ((volatile CMouseCursor*)p)->screenY;
        Pointer_SetPositionClamped((void*)g_inputPointer, f1, f2);
        AsCMouseCursor(p)->visible = 1;
    } else {
        Vec2_Copy(&AsCMouseCursor(p)->savedScreenX, ((char*)&AsCMouseCursor(p)->screenX));
        AsCMouseCursor(p)->hitRegion = 0;
        AsCMouseCursor(p)->cursorState = 0;
    }
    AsCMouseCursor(p)->pointerInputEnabled = (unsigned char)flag;
}
#pragma pop

#pragma push
#pragma opt_common_subs reset
// Save screen position for the next pointer-input transition.
void CMouseCursor::SavePosition() {
    Vec2_Copy(&savedScreenX, ((char*)&screenX)); // original +0x18
}
#pragma pop

#pragma push
#pragma opt_common_subs reset
// Enable the radial menu and animate toward its default screen anchor.
void CMouseCursor::SetRadialMenuEnabled(int flag) {
    radialMenuEnabled = (unsigned char)flag;
    if ((unsigned char)flag != 0) {
        float t1 = kFloat_0_65_MouseCursor;
        float t2 = AsWView(g_view)->viewportWidth;
        float radialCenterX = t1 * t2;
        float radialCenterY = AsWView(g_view)->viewportHeight - kFloat_150_MouseCursor;
        MoveTo(radialCenterX, radialCenterY, kFloat_0_13_MouseCursor);
        float zero = kFloat_0_MouseCursor;
        visible = 1;
        idleTime = zero;
        cursorState = 0;
    }
}
#pragma pop

#pragma push
#pragma opt_common_subs reset
// Snap for negative duration or initialize a weighted screen-space move.
void CMouseCursor::MoveTo(float targetX, float targetY, float duration) {
    int one = 1;
    float zero = kFloat_0_MouseCursor;
    float savedTargetX = targetX;
    float savedTargetY = targetY;
    float durationValue = duration;
    unsigned char* p = (unsigned char*)this;
    AsCMouseCursor(p)->visible = (unsigned char)one;
    AsCMouseCursor(p)->idleTime = zero;
    if (duration < zero) {
        Pointer_SetPositionClamped((void*)g_inputPointer, savedTargetX, savedTargetY);
        AsCMouseCursor(p)->screenX = savedTargetX;
        AsCMouseCursor(p)->screenY = savedTargetY;
    } else {
        AsCMouseCursor(p)->targetX = targetX;
        AsCMouseCursor(p)->targetY = targetY;
        Vec2SubtractResult pr = Vec2_Subtract(((char*)&screenX), ((char*)&this->targetX));
        Vec2_Copy(&moveDeltaX, &pr.a); // original +0x30
        float div;
        float weight;
        weight = kFloat_1_MouseCursor;
        div = weight / durationValue;
        AsCMouseCursor(p)->moveWeight = weight;
        AsCMouseCursor(p)->moveWeightDecayRate = div;
    }
    AsCMouseCursor(p)->cursorState = 0;
    AsCMouseCursor(p)->hitRegion = 0;
    AsCMouseCursor(p)->moveToActive = 1;
}
#pragma pop

#pragma push
#pragma opt_common_subs reset
// Set a normal cursor state or mark an extended state.
void CMouseCursor::SetState(int value) {
    if (value < 7) {
        cursorState = value;
        extendedStateActive = 0;
    } else {
        cursorState = 1;
        extendedStateActive = 1;
    }
}
#pragma pop

#pragma push
#pragma opt_common_subs reset
// Copy the sampled input-pointer X/Y bits into the caller output.
void CMouseCursor::GetInputPosition(float* out) {
    // These integer loads carry IEEE-754 float bits. Volatile discarded samples
    // preserve original reads/spills; replacing with a float-vector copy changes code.
    if (out) {
        volatile int discardedFirstZ;
        volatile int discardedFirstY;
        unsigned int firstYBits = g_inputPointer->screenYBits;
        unsigned int firstZBits = g_inputPointer->screenZBits;
        discardedFirstZ = firstZBits;
        discardedFirstY = firstYBits;
        int sampleXBits = g_inputPointer->screenXBits;
        out[0] = (*(float*)&(sampleXBits));
        volatile int discardedSecondZ;
        unsigned int sampleYBits;
        volatile int discardedSecondX;
        sampleYBits = g_inputPointer->screenYBits;

        int secondXBits = g_inputPointer->screenXBits;
        discardedSecondX = secondXBits;
        float sampleY = *(float*)&(sampleYBits);
        int secondZBits = g_inputPointer->screenZBits;
        discardedSecondZ = secondZBits;
        out[1] = sampleY;
    }
}
#pragma pop

#pragma push
#pragma opt_common_subs reset
// Initialize the cursor unit two-pi cache.
void MouseCursor_InitTwoPiCache() {
    s_twoPi_8019631C = kFloat_6_2831855;
}
#pragma pop

#pragma push
#pragma opt_common_subs reset
// Allocate and construct an NPC with an empty circular child list.
CNPCView* CNPC_Create() {
    CNPCView* obj = new CNPCView;
    return obj;
}
#pragma pop

#pragma push
#pragma opt_common_subs reset
#pragma opt_propagation off
// Destroy owned NPC items before unlinking and freeing their child-list nodes.
void CNPC::OnDestroy(fn_801978F0_List* list) {
    NpcChildLinkView* it;
    NpcChildObject* zero;
    zero = 0;
    it = list->head.next;
    NpcChildLinkView* end = &list->head;
    for (; it != end; it = it->next) {
        NpcChildObject* item = it->item;
        if (item != 0) {
            delete item;
            it->item = zero;
        }
    }
    NpcChildLinkView* sentinel = &list->head;
    NpcChildLinkView* n = list->head.next;
    if (n != sentinel) {
        NpcChildLinkView* lastNode = sentinel->prev;
        n->prev->next = lastNode->next;
        lastNode->next->prev = n->prev;
        while (n != sentinel) {
            NpcChildLinkView* cur = n;
            n = n->next;
            appFreeUni__FPv(cur);
            list->count = list->count - 1;
        }
    }
}
#pragma pop
} // extern "C"
