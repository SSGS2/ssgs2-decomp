#include <decomp/static_init.h>
// Main clock, letter overlays and Wii Remote pointer cursor.
// Frame rate lookup: CMainFrame_UpdateVirtualHooks computes seconds/update
// from kMainFrameDefaultUpdatesPerSecond (30) or the duration-scaling branch
// kMainFrameScaledUpdatesPerSecond (25), defined in u80194F9C_data.cpp.
// This is simulated time per call, not measured wall time or a retrace interval.
// Presentation lives in Engine_PresentFrame (gx_renderer_state_and_resources.cpp); update-call counters
// in title/warning tasks (title_top_and_multiplay_menu.cpp/warning_logo_screens.cpp) require separate timing policy.
// Cursor placement/aspect lookup: CMouseCursor_Reset uses g_view's current
// viewport width/height with 0.625/0.5 fractions; it does not select a resolution.
// CMouseCursor_Update consumes seconds and pointer screen-space coordinates.
// Letter text uses atlas/layout coordinates; CLetter_DrawText sets font scale.
// Shared types: CMouseCursor.h, InputPointerState.h, CLetter.h and WView.h.
// Resources in the letter registry are borrowed; nodes/buckets and cursor arrays
// are released by their destructors. Keep hook order and float operations intact.
// Packed byte view of the existing string pool; storage and encoding stay in the original symbol.
struct LetterOverlayStrings {
    char reserved000[0x56];
    char replay[7]; // 0x56: "Replay"
    char reserved05D[0x11];
    char fastForward[12]; // 0x6E: "FastForward"
    char reserved07A[0x44];
    char replaySuppressedName[3]; // 0xBE: "\u682a"
};
#include "game/singletons.h"
#include "game/types.h"
#include "classes/CLetter.h"
#include "classes/CMouseCursor.h"
#include "classes/InputPointerState.h"
#include "classes/WView.h"
// CMouseCursor_Reset (0x80195CD4): viewport dimensions from g_view.
// Cursor offsets below: CMouseCursor_Reset (0x80195CD4), CMouseCursor_Update.


#define AsCMouseCursor(p) ((CMouseCursor*)(p))

#define AsCLetter(p) ((CLetter*)(p))
// Translation unit 0x80194F9C-0x8019631C (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern unsigned char s_letterTypeInfo;
extern const float kFloat_6_2831855;
extern float s_letterOverlayTwoPi;
extern float s_twoPi_80194F9C_805E2A68;
extern const float kFloat_0_MessageDraw_805DF140;
extern const float kFloat_0_35_MouseCursor;
extern const float kFloat_0_MouseCursor;
extern void* g_mouseCursor;

extern void MatrixUtilities_NoOpHookA(); // Empty base-initialization hook in raw_file_stream_and_matrix_utils.cpp.
extern void MatrixUtilities_NoOpHookB(void*); // Empty base-cleanup hook in raw_file_stream_and_matrix_utils.cpp.
extern void Matrix_SetIdentityAffine(const char*);
extern void __dl__FPv(void*);
extern void BaseObject_ctor(void*);
extern void BaseObject_dtor(void*, int);
extern void Vec2_Copy(void*, float*);
extern void appFreeUni__FPv(void*);
extern void Pointer_SetPositionClamped(void*, float, float);
extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void appFreeUni__FPv(void*);
extern void* g_view;
extern void* g_inputPointer;
extern const float kFloat_0_625_MouseCursor;
extern const float kFloat_0_5_MouseCursor;
extern const float kMainFrameScaledUpdatesPerSecond;
extern const float kMainFrameDefaultUpdatesPerSecond;
extern const float kMainFrameTimeUnity;
extern const float kMainFrameSecondsPerMillisecond;
extern int memset(void*, int, unsigned int);
extern int EventTiming_IsDurationScalingEnabled(void*);
extern char* strchr(void*, int);
extern float WTitleFont_GetAtlasGlyphAdvance(void*, void*, int);
extern float WFont_DrawAtlasGlyph(void*, void*, int, float, float, int, int);
extern unsigned int strlen(const char*);

extern const char __vt__10CMainFrame[];
extern const char s_cameraTransform[];
extern const char __vt__12CMouseCursor[];
extern const char s_mouseCursorInitialVtable[];
extern const float kFloat_0_Letter;
extern const float kFloat_0_5_Rival;
extern const float kFloat_255_LetterFade;
extern const float kFloat_0_00001_MouseCursor;
extern const float kFloat_3_MouseCursor;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_mouseCursorInputDestructorStorage[];
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern void __register_global_object(void*, void (*)(), void*);
extern void InputManager_SetPointerPositionOverride(void*, float*);
struct CursorPoint { float x, y; };
typedef Vec3f CursorVector;
extern CursorPoint CursorVec2_Scale(void*, float);
extern CursorPoint Vec2_Add(void*, CursorPoint*);
extern int Vec3_NearlyEqualWithTolerance(void*, float*, float);
extern void Pointer_GetMotionDelta(float*, void*);
extern int Pointer_GetButtonState(void*, int);
extern int CMouseCursor_UpdateGolfActions(void*);
extern int CMouseCursor_HitTestRegions(void*);
extern unsigned int g_currentTimeMilliseconds;
extern char golfLetterOverlayStrings[];
extern void* s_letterKnownMessageNames[];
extern int strcmp(void*, void*);
extern int strstr(void*, void*);
extern void GolfTipsDialog_RequestClose(int, float);
// Atlas text drawing sets the font's vertical glyph scale before measuring and drawing.
struct LetterFontScale { char reserved[0x38]; float glyphHeightScale; };
// Borrowed message ABI (scheduledTime is an absolute millisecond timestamp):
// type          +8 nameOrEnabled       +C value                 +10
// Clear         optional substring     unused                  unused
// Show          resource name          optional float*         show immediately
// FadeIn/Out    unused                 unused                  unused
// SetEnabled    integer exactly 1      unused                  unused
// SetOptions    resource name          LetterOverlayOptions*   unused
// Replay/FastForward: remaining payload fields are unused here.
// Strings and payloads must survive this call; selected resources are borrowed
// from the overlay registry. SetOptions requires value != 0 when lookup succeeds.
struct LetterMessage {
    unsigned int sender;
    unsigned int messageType;
    union { void* pointer; unsigned int integer; } nameOrEnabled;
    void* value;
    int showImmediately;
    char reserved14[8];
    unsigned int scheduledTime;
};
// Caption options copied by SetOptions to the selected Effect's +4C..+58.
// Consumer: CLetter_OnDisplay, LetterCaptionOptions in golf_letter_overlays.cpp:
// +0 showCaption (byte; forced to 1 after copying, even if input is zero),
// +4 captionValue (signed 32-bit integer formatted as decimal),
// +8 glyphHeightScale (binary32 multiplier, not a pixel height),
// +C packedColor (32-bit draw color, not the overlay's fading alpha).
// Padding +1..3 is not copied. All named overlay resources use this SAME schema;
// it is not resource-specific metadata. OnDisplay passes the +50 word through
// a pointer-shaped varargs slot, but the pool's actual format is %d (definition:
// or retained: all signed 32-bit values are representable as decimal text; the
// stock fontCharacters includes digits and minus. Replacement fonts must keep
// those glyphs: missing glyph subtraction in CLetter_DrawText is unchecked.
// The options packet itself is borrowed only for this synchronous copy.
// Use a finite scale; the consumer has no sign clamp or established visual range
// (zero collapses glyph height, negative reverses signed extents). Color bits pass
// unchanged. Visibility still requires the display/message/game-state gates.
struct LetterOverlayOptions { unsigned char showCaption; char alignment[3]; int captionValue; float glyphHeightScale; unsigned int packedColor; };
// Message states: fade in, fully visible, or fade out.
enum LetterVisibilityState { kLetterFadingIn=1, kLetterFullyVisible=2, kLetterFadingOut=3 };
struct LetterOverlayResource {
    char reserved00[0x24];
    float messageValue;
    char resourceName[0x24];
    LetterOverlayOptions options;
};
typedef char LetterMessageTimeCheck[((int)&((LetterMessage*)0)->scheduledTime==0x1C) ? 1 : -1];
typedef char LetterOverlayOptionsCheck[((int)&((LetterOverlayResource*)0)->options==0x4C) ? 1 : -1];
enum LetterMessageType { kLetterClear=0, kLetterShow=1, kLetterFadeIn=2, kLetterFadeOut=3,
    kLetterSetEnabled=4, kLetterSetOptions=5, kLetterReplay=6, kLetterFastForward=7 };
enum LetterTextAlignment { kTextLeft=0, kTextCentered=1, kTextRight=2 };
enum CursorHitTestMode { kHitTestNone=0, kHitTestGolf=1, kHitTestRegions=2, kHitTestMode3=3 };
enum CursorAppearance { kCursorIdle=0, kCursorHover=1, kCursorAction=2, kCursorDrag=3 };
enum CursorButton { kPrimaryButton=0, kSecondaryButton=1, kMiddleButton=2, kPointerButtonCount=3 };
enum CursorButtonState { kButtonNewPress=1, kButtonHeld=3 };
enum MainFrameUpdateFlags { kSuppressElapsedMillisecondsHook = 0x10 };

// Field-derived byte addresses retain the original opaque-call aliasing and register lifetimes.
static const int kLetterMutexOffset = (int)&((CLetter*)0)->nameHashMutex;
static const int kCursorPositionOffset = (int)&((CMouseCursor*)0)->screenX;
struct CursorOwnedStorage {
    const void* vtable;
    char pad04[0x38];
    void* resetPointer3C;
    char pad40[4];
    float initialScalar44;
    void* resetPointer48;
    char pad4C[0x14];
    void* clipRect;
    char pad64[0x10];
    void* ownedArray;
    int ownedArrayCount;
    int ownedArrayCapacity;
    char pad80[0x14];
    float actionPosition[2];
    char pad9C[0x20];
    unsigned char autoHideEnabled;
    char padBD[0x48];
    int moveToActive;
};
// These are partial offset views of one cursor, not independent objects:
// clipRect and ownedArray are allocations released by CMouseCursor_dtor;
// actionPosition is a screen-space pair; moveToActive at +0x108 controls the
// scripted target/weight path. Unnamed reset fields have no established role.

extern "C" void* NONE(void*, int);

struct MainFrameValueInterface {
    virtual void unused0();
    virtual void unused1();
    virtual void unused2();
    virtual void unused3();
    virtual int get_value();
};

struct MainFrameState {
    const void* vtable;
    int applicationResult;
    float elapsedMilliseconds;
    int initialValue;
};

// Reset-only fields at 0x5C and 0x70 have no established consumers in this unit.
struct CursorResetState {
    char pad00[0x10]; float screenX; float screenY; char pad1C[0x28]; float idleTime;
    char pad44[8]; unsigned char visible, pointerInputEnabled; char pad4E[2]; int hitRegion,cursorState;
    char pad58[4]; float resetScalar5C; char pad60[4]; unsigned char b64; char pad65[3];
    float moveWeight; char pad6C[4]; int resetWord70; char pad74[4]; int ownedArrayCount;
    char pad7C[4]; unsigned char b80; char pad81[3]; int primaryButtonRegion,secondaryButtonRegion,middleButtonRegion; char pad90[0x16]; int pA8; char padAC[0x11];
    unsigned char flagBD; char padBE[0x48];
};

struct MainFrameUpdateState {
    void* vtable;
    void* applicationResult;
    float elapsedMilliseconds;
};

class MainFrameUpdateInterface {
public:
// CProjectG's primary table: m3/+0C Update(seconds), m4/+10 Draw(), m5/+14 UpdateDisplayStatus(), m6/+18 vfunc_4,
// m7/+1C UpdateRegisteredDevices(). CProjectG_UpdateVirtualHooks refreshes the wall clock and calls this
// dispatcher; the device hook ignores its argument.
    virtual void m0();
    // Names m3..m7 are historical local spellings, not vtable byte offsets.
    // CProjectG's primary table (phase1_class_tables.cpp) supplies:
    // m3/+0C = CProjectG_Update(seconds): task/actors, cursor, effects;
    // m4/+10 = CProjectG_Draw(): view, task/loading screen, diagnostics, effects;
    // m5/+14 = CProjectG_UpdateDisplayStatus(): publish viewport dimensions and
    //          suspend/restore mouse input according to render-target status;
    // m6/+18 = CProjectG_vfunc_4 (not invoked here);
    // m7/+1C = CProjectG_UpdateRegisteredDevices(): visits device callbacks using
    //          global wall-clock g_currentTimeMilliseconds, not its unused r4.
    // Implementations: project_g_app_and_devices.cpp (draw/status/devices), CProjectG_Update.
    // CProjectG_UpdateVirtualHooks (project_base_and_grid_surface.cpp) refreshes that wall clock,
    // then calls this base dispatcher. Thus suppression changes the argument
    // sent to m7, but the current CProjectG device hook ignores that argument.
    // Draw and status refresh still happen even when input time is suppressed;
    // status sees render-target changes made during drawing in this same call;
    // the new mouse-input gate affects the subsequent project update.
    virtual void m3(float);
    virtual void m4();
    virtual void m5();
    virtual void m6();
    virtual void m7(int);
};

// Place the pointer at viewport fractions (0.625 W, 0.5 H), clear interaction state and enable every region.
// object is required initialized cursor storage; pointerInputEnabled is narrowed
// to a byte. g_view/g_inputPointer must exist. Coordinates are screen pixels in
// the view's viewport; preserve the paired pointer-driver update when relocating.
void CMouseCursor_Reset(void* object, int pointerInputEnabled) {
    float initialViewportXFraction = kFloat_0_625_MouseCursor;
    float initialViewportYFraction = kFloat_0_5_MouseCursor;
    CursorResetState* self = (CursorResetState*)object;
    void* view = g_view;
    self->screenX = initialViewportXFraction * ((WView*)view)->viewportWidth;
    view = g_view;
    self->screenY = initialViewportYFraction * ((WView*)view)->viewportHeight;
    Vec2_Copy(&AsCMouseCursor(self)->savedScreenX, &self->screenX);
    self->idleTime = kFloat_0_MouseCursor;
    Pointer_SetPositionClamped(g_inputPointer, self->screenX, self->screenY);
    volatile int* arrayCountWord = &self->ownedArrayCount;
    int oldArrayCount = *arrayCountWord;
    int zeroed = oldArrayCount - oldArrayCount;
    self->pointerInputEnabled = (unsigned char)pointerInputEnabled;
    AsCMouseCursor(self)->pointerUpdateOverrideEnabled = 0;
    self->visible = 1;
    AsCMouseCursor(self)->radialMenuDragActive = 0;
    self->resetScalar5C = kFloat_0_MouseCursor;
    self->hitRegion = 0;
    self->cursorState = 0;
    AsCMouseCursor(self)->radialMenuEnabled = 0;
    self->moveWeight = kFloat_0_MouseCursor;
    self->resetWord70 = 0;
    AsCMouseCursor(self)->alternateHudMode = 0;
    AsCMouseCursor(self)->extendedStateActive = 0;
    *arrayCountWord = zeroed;
    self->primaryButtonRegion = 0;
    self->secondaryButtonRegion = 0;
    self->middleButtonRegion = 0;
    memset((char*)&AsCMouseCursor(self)->regionEnabled, 1, 0x48);
}

// Advance simulated main-frame time and dispatch clock/update hooks in ABI order.
// self must implement MainFrameUpdateInterface. Returns the application result
// word after all hooks, carried in a pointer-shaped ABI slot (not an allocation
// or ownership transfer). Application_RequestExit writes 1 to the same +4 word;
// zero is not a load-completion status here.
// flags bit 0x10 sends zero to m7 only: accumulation and the seconds hook m3
// continue. m7 gets truncated accumulated milliseconds; m3 gets this call's
// seconds. Preserve reciprocal -> milliseconds -> seconds operation order:
// replacing it with a direct reciprocal changes rounding at the hook boundary.
// Subclass map is beside MainFrameUpdateInterface. The concrete CProjectG
// device callback uses wall time; do not equate it to elapsedMilliseconds here.
// Accumulator contract: initialized to 0 by CMainFrame_Ctor and explicitly reset
// by CMainFrame_ResetElapsedTimeAndCaptureValue; never reset/wrapped in this
// dispatcher. Require finite positive updatesPerSecond and, when bit 0x10 is clear,
// finite accumulated milliseconds in [0,2147483648) BEFORE the signed conversion
// (largest binary32 below the upper bound is 2147483520). The integer hook is
// truncation toward zero. This is about 24.85 days of simulated time, not wall
// time; float spacing reduces clock resolution well before that boundary and
// can eventually stop small increments. No overflow recovery/saturation exists.
// Suppression skips conversion but still accumulates; re-enabling it requires the
// same range. Preserve float addition/reciprocal order for long-running clocks.
void* CMainFrame_UpdateVirtualHooks(MainFrameUpdateState* self, unsigned int flags) {
    // Select the original clock branch, then accumulate before notifying m7.
    float updatesPerSecond = EventTiming_IsDurationScalingEnabled(self) ? kMainFrameScaledUpdatesPerSecond : kMainFrameDefaultUpdatesPerSecond;
    float elapsedStepMilliseconds = (kMainFrameTimeUnity / updatesPerSecond) / kMainFrameSecondsPerMillisecond;
    self->elapsedMilliseconds += elapsedStepMilliseconds;
    MainFrameUpdateInterface* frameHooks = (MainFrameUpdateInterface*)self;
    frameHooks->m7(!(flags & kSuppressElapsedMillisecondsHook) ? (int)self->elapsedMilliseconds : 0);
    // Dispatch seconds before the two parameterless phases; subclasses may
    // publish applicationResult from any of these hooks.
    frameHooks->m3(kMainFrameSecondsPerMillisecond * elapsedStepMilliseconds);
    frameHooks->m4();
    frameHooks->m5();
    return self->applicationResult;
}

struct WInstanceLock {
    void* lock;
    // Acquire the instance mutex for the full lexical lookup/destruction scope.
    WInstanceLock(void* l) { lock = l; OSLockMutex(l); }
    // Release the same mutex on normal return or unwind.
    ~WInstanceLock() { OSUnlockMutex(lock); }
};

#pragma push
#pragma use_lmw_stmw on
// Free overlay lookup nodes and hash buckets under the instance mutex, then destroy the base.
void* CLetter_dtor(void* self, int freeFlag) {
    if (self) {
        char* obj = (char*)self;
        if (&AsCLetter(obj)->pad_01C) {
            WInstanceLock g(obj + kLetterMutexOffset);
            void** newHead;
            if (AsCLetter(obj)->nodeAllocationHead) {
                void* list = obj + kLetterMutexOffset;
                void** n;
                while ((n = (*(void***)&AsCLetter(obj)->nodeAllocationHead)) != 0) {
                    OSLockMutex(list);
                    ((void**)n[4])[3] = n[3];
                    ((void**)n[3])[4] = n[4];
                    newHead = (void**)n[4];
                    if (newHead == n) newHead = 0;
                    OSUnlockMutex(list);
                    (*(void***)&AsCLetter(obj)->nodeAllocationHead) = newHead;
                    appFreeUni__FPv(n);
                }
            }
            void* field38 = AsCLetter(obj)->nameHashBuckets;
            if (field38) { appFreeUni__FPv(field38); AsCLetter(obj)->nameHashBuckets = 0; }
        }
        __dt__18NativeBase80201D18Fv(self, 0);
        if (freeFlag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Default main-frame hook; this base implementation has no work.
void CMainFrame_vfunc_5() {}
// Default main-frame hook; this base implementation has no work.
void CMainFrame_vfunc_3() {}
// Return the letter overlay runtime type descriptor.
unsigned char* CLetter_GetRTTI() { return &s_letterTypeInfo; }
// Initialize this module's two-pi value from the shared constant.
void LetterOverlay_InitializeTwoPi() { s_letterOverlayTwoPi = kFloat_6_2831855; }
// Set the application exit-request word.
void Application_RequestExit(int* arg0) { arg0[1] = 1; }
// Initialize the cursor module's two-pi value from the shared constant.
void MouseCursor_InitializeTwoPi() { s_twoPi_80194F9C_805E2A68 = kFloat_6_2831855; }

// Measure atlas glyph advances for alignment, then draw each character at its next x position.
// object/font/view must exist; null or empty text draws nothing and leaves font
// scale unchanged. Every input byte must occur in fontCharacters: missing glyphs
// are not checked before pointer subtraction. x/y are atlas UI draw coordinates;
// charHeight sets the font's persistent glyphHeightScale before measuring.
// charWidth is the added horizontal spacing; alignment measurement uses the
// original charWidth*charHeight term while drawing adds charWidth directly.
// Preserve this asymmetry, signed byte lookup, float order and packed draw/color
// arguments when changing font sheets. text/font bytes remain borrowed.
void CLetter_DrawText(void* object, const char* text, float x, float y, float charWidth, float charHeight, int drawFlags, int color) {
    char* font;
    float width;
    const char* cursor;
    if (text == 0) return;
    if (strlen(text) == 0) return;
    font = ((char*)AsCLetter(object)->fontCharacters);
    cursor = text;
    {
        void* fontObject = AsCLetter(object)->titleFont;
        ((LetterFontScale*)fontObject)->glyphHeightScale = (width = 0.0f, charHeight);
    }
    switch (AsCLetter(object)->textAlignment) {
    case kTextLeft: break;
    case kTextCentered:
        while (*cursor != 0) {
            char* glyph = strchr(font, (signed char)*cursor++);
            float advance = charWidth * charHeight + WTitleFont_GetAtlasGlyphAdvance(AsCLetter(object)->titleFont, g_view, glyph - font);
            width += advance;
        }
        width -= charWidth;
        width *= kFloat_0_5_Rival;
        break;
    case kTextRight:
        while (*cursor != 0) {
            char* glyph = strchr(font, (signed char)*cursor++);
            float advance = charWidth * charHeight + WTitleFont_GetAtlasGlyphAdvance(AsCLetter(object)->titleFont, g_view, glyph - font);
            width += advance;
        }
        width -= charWidth;
        break;
    }
    x -= width;
    cursor = text;
    while (*cursor != 0) {
        char* glyph = strchr(font, (signed char)*cursor++);
        float glyphAdvance = WFont_DrawAtlasGlyph(AsCLetter(object)->titleFont, g_view, glyph - font, x, y, drawFlags, color);
        float advance = glyphAdvance + charWidth;
        x += advance;
    }
}

// Install the cursor singleton, initialize owned storage and reset its interaction state.
// Lifecycle: CProjectG_InitializeGameObjects (project_base_and_grid_surface.cpp) constructs view and input
// pointer first, then a fresh 0x10C cursor. BaseObject_ctor (gsr_resource_bitmap_and_heap.cpp) can invoke
// g_baseObjectConstructionHook BEFORE publication: that hook sees only a base,
// while g_mouseCursor still names the previous instance (or null). It must not
// access derived cursor fields, delete/reenter self, or use a retained self until
// construction finishes. If that hook throws, this function never publishes and
// the base's incremented bookkeeping is not rolled back by BaseObject_ctor.
// After publication, initialization calls Vec2_Copy, then Reset; Reset calls
// Pointer_SetPositionClamped (input_manager_and_notice_queue.cpp, direct coordinate stores, no callback).
// No recovered call after publication invokes a user hook, and no rollback
// restores the former singleton on an exception. A replacement initializer must
// preserve this order or separately define new exception/reentrancy semantics.
// Serialize construction with readers and destruction. Finish/free the former
// cursor BEFORE installing another: both cursor destructors unconditionally clear
// g_mouseCursor even if it has since been rebound. Caller owns returned storage;
// CMouseCursor_dtor(...,1) releases clip/array/base/storage. Never expose a
// partially initialized cursor to update/draw or retain its owned buffers.
void* CMouseCursor_Ctor(CursorOwnedStorage* self) {
    BaseObject_ctor(self);
    self->vtable = (const void*)s_mouseCursorInitialVtable;
    g_mouseCursor = self;
    self->vtable = (const void*)__vt__12CMouseCursor;
    self->resetPointer3C = 0; // Remaining constructor-only scalars/pointers retain offset-based names.
    self->clipRect = 0;
    self->ownedArray = 0;
    self->ownedArrayCount = 0;
    self->ownedArrayCapacity = 0;
    self->initialScalar44 = kFloat_0_35_MouseCursor;
    self->resetPointer48 = 0;
    float local[2];
    local[0] = kFloat_0_MouseCursor;
    local[1] = kFloat_0_MouseCursor;
    Vec2_Copy(&self->actionPosition[0], local);
    self->autoHideEnabled = 0;
    AsCMouseCursor(self)->enabled = 1;
    self->moveToActive = 0;
    if (self->clipRect) {
        __dl__FPv(self->clipRect);
        self->clipRect = 0;
    }
    CMouseCursor_Reset(self, 0);
    AsCMouseCursor(self)->hitTestMode = 0;
    return self;
}

// Clear the cursor singleton and destroy its base, optionally freeing the allocation.
void* NONE(void* self, int freeFlag) {
    if (self) {
        g_mouseCursor = 0;
        BaseObject_dtor(self, 0);
        if (freeFlag > 0) __dl__FPv(self);
    }
    return self;
}

// Release the clip rectangle and owned array, clear the singleton and destroy the base.
void* CMouseCursor_dtor(CursorOwnedStorage* self, int freeFlag) {
    if (self) {
        void* mem = self->clipRect;
        self->vtable = __vt__12CMouseCursor;
        if (mem) {
            __dl__FPv(mem);
            self->clipRect = 0;
        }
        if ((void*)&self->ownedArray) {
            if ((void*)&self->ownedArray && self->ownedArray) {
                volatile int* ownedArrayCount = &self->ownedArrayCount;
                int v = *ownedArrayCount;
                *ownedArrayCount = v - v;
                appFreeUni__FPv(self->ownedArray);
            }
        }
    }
    if (self) {
        g_mouseCursor = 0;
        BaseObject_dtor(self, 0);
        if (freeFlag > 0) __dl__FPv(self);
    }
    return self;
}

// Snapshot the main-frame virtual value and reset its elapsed time.
void CMainFrame_ResetElapsedTimeAndCaptureValue(MainFrameState* frame) {
    int initialValue = ((MainFrameValueInterface*)frame)->get_value();
    float zeroElapsedTime = kFloat_0_MessageDraw_805DF140;
    frame->initialValue = initialValue;
    frame->elapsedMilliseconds = zeroElapsedTime;
}

// Advance scripted motion, handle idle visibility and select hover/action/drag cursor states.
// object and the pointer driver are required. delta is a nonnegative seconds
// step from the caller; it is not multiplied by a local frame-rate constant.
// Hit-test providers must return an index inside regionEnabled (72 byte flags).
// Updates cursor fields and possibly the shared input-manager position override;
// the stack point passed to that manager is consumed by the call, not owned here.
// Preserve early returns and per-button sampling order for scripted movement.
void CMouseCursor_Update(void* object, float delta) {
    // delta is seconds, matching the main-frame m3 step. Positions/deltas below
    // are viewport pixels; moveWeight is dimensionless and decays per second.
    // Phase 1: scripted motion overrides the pointer; a positive-weight step
    // returns before idle/hit/button processing, even on the step reaching zero.
    unsigned char* self = (unsigned char*)object;
    if (!AsCMouseCursor(self)->enabled) return;
    if (!AsCMouseCursor(self)->pointerInputEnabled && 0 == AsCMouseCursor(self)->pointerUpdateOverrideEnabled && !AsCMouseCursor(self)->moveToActive) return;
    float current = AsCMouseCursor(self)->moveWeight;
    if (current > kFloat_0_MouseCursor) {
        if (AsCMouseCursor(self)->moveToActive) {
            CursorPoint inputOverridePosition;
            float zero = kFloat_0_MouseCursor;
            if ((AsCMouseCursor(self)->moveWeight = current - AsCMouseCursor(self)->moveWeightDecayRate * delta) < zero) {
                AsCMouseCursor(self)->moveWeight = zero;
                Vec2_Copy(&AsCMouseCursor(self)->savedScreenX, &AsCMouseCursor(self)->targetX);
            }
            CursorPoint scriptedPosition = Vec2_Add(&AsCMouseCursor(self)->targetX, &(CursorVec2_Scale(&AsCMouseCursor(self)->moveDeltaX, AsCMouseCursor(self)->moveWeight)));
            Vec2_Copy(self + kCursorPositionOffset, &scriptedPosition.x);
            Pointer_SetPositionClamped(g_inputPointer, AsCMouseCursor(self)->screenX, AsCMouseCursor(self)->screenY);
            inputOverridePosition.x = AsCMouseCursor(self)->screenX;
            inputOverridePosition.y = AsCMouseCursor(self)->screenY;
            if (!Sing_805DC5C8()) {
                if (s_inputManagerInitialized == 0) {
                    InputManager_Init(&s_gameInputManager);
                    __register_global_object(&s_gameInputManager, InputManager_Dtor, &s_mouseCursorInputDestructorStorage);
                    s_inputManagerInitialized = 1;
                }
                Sing_805DC5C8() = &s_gameInputManager;
            }
            InputManager_SetPointerPositionOverride(Sing_805DC5C8(), &inputOverridePosition.x);
            return;
        }
    }
    if (AsCMouseCursor(self)->moveToActive) {
        Vec2_Copy(self + kCursorPositionOffset, &AsCMouseCursor(self)->targetX);
        CursorPoint targetOverridePosition;
        targetOverridePosition.x = AsCMouseCursor(self)->screenX;
        targetOverridePosition.y = AsCMouseCursor(self)->screenY;
        if (Sing_805DC5C8() == 0) {
            if (0 == s_inputManagerInitialized) {
                InputManager_Init(&s_gameInputManager);
                __register_global_object(&s_gameInputManager, InputManager_Dtor, &s_mouseCursorInputDestructorStorage);
                s_inputManagerInitialized = 1;
            }
            Sing_805DC5C8() = &s_gameInputManager;
        }
        InputManager_SetPointerPositionOverride(Sing_805DC5C8(), &targetOverridePosition.x);
    }
    else AsCMouseCursor(self)->moveToActive = 0;
    // Phase 2: idle time advances only with negligible motion and no buttons;
    // three seconds of inactivity hide the cursor. Any activity shows it again.
    if (AsCMouseCursor(self)->autoHideEnabled) {
        float motionDelta[3];
        float zeroMotion[3];
        zeroMotion[2] = zeroMotion[1] = zeroMotion[0] = kFloat_0_MouseCursor;
        Pointer_GetMotionDelta(motionDelta, g_inputPointer);
        if (Vec3_NearlyEqualWithTolerance(motionDelta, zeroMotion, kFloat_0_00001_MouseCursor) && !Pointer_GetButtonState(g_inputPointer,kPrimaryButton) && !Pointer_GetButtonState(g_inputPointer,kSecondaryButton) && !Pointer_GetButtonState(g_inputPointer,kMiddleButton)) {
            float hideAfterSeconds;
            float idleSeconds = AsCMouseCursor(self)->idleTime;
            idleSeconds += delta;
            hideAfterSeconds = kFloat_3_MouseCursor;
            AsCMouseCursor(self)->idleTime = idleSeconds;
            if (idleSeconds > hideAfterSeconds) AsCMouseCursor(self)->visible = 0;
        }
        else {
            float z = kFloat_0_MouseCursor;
            AsCMouseCursor(self)->visible = 1;
            AsCMouseCursor(self)->idleTime=z;
        }
    }
    else {
        AsCMouseCursor(self)->visible = 1;
        AsCMouseCursor(self)->idleTime = kFloat_0_MouseCursor;
    }
    // Transition/publication map (CMouseCursor_SetState/Get* in mouse_cursor_hit_testing.cpp):
    // mode 0 or 3: position/idle processing above only; keep action state/region,
    //              button latches and action-enable flags from the prior call.
    // mode 1 visible: UpdateGolfActions publishes region 0..0x47, enables and
    //                 appearance 0/1/2/4/5 (edge states 4/5 bypass buttons below).
    // mode 2 visible: HitTestRegions publishes world/layout hit region and 0/1.
    // mode 1/2 hidden: retain previous appearance/region/enables, then STILL
    //                 apply region mask and button transitions below.
    // unsupported modes: visible sets appearance 0, but retains region; do not
    //                    use these as another way to disable interaction.
    // mask rejection -> region=0,state=0. New presses (button value 1) latch the
    // region for all three buttons. State 0 + primary HELD(3) + any pixel motion
    // ->3 (drag); state 1 + primary first, else secondary, nonzero button ->
    // same latched region or radial drag ? (enabled?2:3) : 0. Others unchanged.
    // Middle button latches but does not promote state 1. Release is reflected
    // by the next visible hit test; hidden state 2 can persist through release.
    // Public SetState accepts 0..6 as normal, >=7 publishes state=1 plus
    // extendedStateActive=1; Update does not clear that separate byte. Reset
    // clears both. Extended mode is therefore NOT appearance 7. Negative input
    // passes through SetState unchecked and is outside the supported domain.
    // Require all hit/latch region IDs 0..0x47 even when hidden. State 6 is an
    // externally set normal state with no local transition; retain it unchanged.
    // Phase 3: sample screen position and update the hit region. Hidden cursors
    // retain their last hit result; the region mask can still reject that result.
    // Preserve separate snapshots: coalescing them changes the original loads.
    CursorVector pointerPositionForX = *(CursorVector*)((InputPointerState*)g_inputPointer)->position;
    AsCMouseCursor(self)->screenX = pointerPositionForX.x;
    CursorVector pointerPositionForY = *(CursorVector*)((InputPointerState*)g_inputPointer)->position;
    AsCMouseCursor(self)->screenY = pointerPositionForY.y;
    int mode = AsCMouseCursor(self)->hitTestMode;
    // Mode 3 bypasses action updates, including its otherwise unreachable switch case.
    if (kHitTestMode3 == mode || kHitTestNone == mode) return;
    if (AsCMouseCursor(self)->visible) {
        int r;
        AsCMouseCursor(self)->primaryActionEnabled=1;
        AsCMouseCursor(self)->secondaryActionEnabled=0;
        switch (mode) {
            case kHitTestGolf:
            r=CMouseCursor_UpdateGolfActions(self);
            break;
            case kHitTestRegions:
            r=CMouseCursor_HitTestRegions(self);
            break;
            case kHitTestMode3:
            AsCMouseCursor(self)->primaryActionEnabled=1;
            AsCMouseCursor(self)->secondaryActionEnabled=1;
            AsCMouseCursor(self)->hitRegion=0;
            r=0;
            break;
            default:
            r=0;
            break;
        }
        AsCMouseCursor(self)->cursorState=r;
    }
    if (!AsCMouseCursor(self)->regionEnabled[AsCMouseCursor(self)->hitRegion]) {
        AsCMouseCursor(self)->hitRegion=0;
        AsCMouseCursor(self)->cursorState=kCursorIdle;
    }
    // Phase 4: latch the region at each button's press, then distinguish a held
    // action from dragging away. Region IDs returned by hit tests must index
    // regionEnabled; buttonDownRegions stores three consecutive word-sized IDs.
    {
        unsigned char* q = self;
        for (int i=0;3>i;i++, q+=4) if (Pointer_GetButtonState(g_inputPointer,i)==kButtonNewPress) AsCMouseCursor(q)->buttonDownRegions[0]=AsCMouseCursor(self)->hitRegion;
    }
    switch (AsCMouseCursor(self)->cursorState) {
        case kCursorIdle:
        if (Pointer_GetButtonState(g_inputPointer,kPrimaryButton)==kButtonHeld) {
            CursorVector pressPositionForX = *(CursorVector*)((InputPointerState*)g_inputPointer)->buttons[0].pressPosition;
            float horizontalDragPixels=pressPositionForX.x-AsCMouseCursor(self)->screenX;
            horizontalDragPixels = horizontalDragPixels>kFloat_0_MouseCursor ? horizontalDragPixels : -horizontalDragPixels;
            CursorVector pressPositionForY = *(CursorVector*)((InputPointerState*)g_inputPointer)->buttons[0].pressPosition;
            float verticalDragPixels=pressPositionForY.y-AsCMouseCursor(self)->screenY;
            verticalDragPixels = verticalDragPixels>kFloat_0_MouseCursor ? verticalDragPixels : -verticalDragPixels;
            if (horizontalDragPixels+verticalDragPixels>kFloat_0_MouseCursor) AsCMouseCursor(self)->cursorState=kCursorDrag;
        }
        break;
        case kCursorHover:
        if (Pointer_GetButtonState(g_inputPointer,kPrimaryButton)) {
            if (AsCMouseCursor(self)->hitRegion==AsCMouseCursor(self)->buttonDownRegions[0]||AsCMouseCursor(self)->radialMenuDragActive) {
                if (AsCMouseCursor(self)->primaryActionEnabled) AsCMouseCursor(self)->cursorState=kCursorAction;
                else AsCMouseCursor(self)->cursorState=kCursorDrag;
            }
            else AsCMouseCursor(self)->cursorState=kCursorIdle;
        }
        else if (Pointer_GetButtonState(g_inputPointer,kSecondaryButton)) {
            if (AsCMouseCursor(self)->hitRegion==AsCMouseCursor(self)->buttonDownRegions[1]||AsCMouseCursor(self)->radialMenuDragActive) {
                if (AsCMouseCursor(self)->secondaryActionEnabled) AsCMouseCursor(self)->cursorState=kCursorAction;
                else AsCMouseCursor(self)->cursorState=kCursorDrag;
            }
            else AsCMouseCursor(self)->cursorState=kCursorIdle;
        }
        break;
        default:
        break;
        case kCursorAction:
        break;
    }
}

// Initialize the main frame and reset the global affine camera transform.
MainFrameState* CMainFrame_Ctor(MainFrameState* frame) {
    float zero = kFloat_0_MessageDraw_805DF140;
    frame->vtable = __vt__10CMainFrame;
    frame->applicationResult = 0;
    frame->elapsedMilliseconds = zero;
    MatrixUtilities_NoOpHookA();
    Matrix_SetIdentityAffine(s_cameraTransform);
    return frame;
}

// Run main-frame base cleanup and optionally free its allocation.
MainFrameState* CMainFrame_dtor(MainFrameState* frame, int freeFlag) {
    if (frame != 0) {
        frame->vtable = __vt__10CMainFrame;
        MatrixUtilities_NoOpHookB(frame);
        if (freeFlag > 0)
            __dl__FPv(frame);
    }
    return frame;
}

struct LetterMutexView { char pad[0x3c]; char mutex[4]; };
// The overlay owns nodes via its allocation list; buckets only link them.
// This view names only established words:
// +0 borrowed resource, +4 NUL-terminated key, +14 next collision node.
// The intervening three words remain opaque; lookup returns no ownership.
struct OverlayNameHashNode {
    void* resource;
    const char* name;
    unsigned int opaque[3];
    OverlayNameHashNode* next;
};

// Select a locked hash bucket from the name length and first/last character bits.
static inline unsigned int LetterOverlay_HashName(char* self, const char* name) {
    void* l = ((LetterMutexView*)self)->mutex;
    OSLockMutex(l);
    int len = strlen(name);
    const unsigned char* un = (const unsigned char*)name;
    unsigned int hash = (un[0] & 0x2a) | (un[(len - 1) & ((-len & ~len) >> 31)] & 0x15);
    hash = (hash & 0x3f) | (len << 6);
    unsigned int idx = hash & AsCLetter(self)->nameHashMask;
    OSUnlockMutex(l);
    return idx;
}

// Find a named overlay resource through its hash collision chain under the mutex.
static inline void* LetterOverlay_FindByName(char* self, const char* name) {
    WInstanceLock g(self + kLetterMutexOffset);
    if (!name || !AsCLetter(self)->nameHashBuckets) return 0;
    unsigned int idx = LetterOverlay_HashName(self, name);
    OverlayNameHashNode* node = (OverlayNameHashNode*)((void**)AsCLetter(self)->nameHashBuckets)[idx];
    while (node) {
        if (strcmp((void*)node->name, (void*)name) == 0) return node->resource;
        node = node->next;
    }
    return 0;
}

// Apply scheduled overlay show/clear/fade/options and replay-icon messages.
// Scheduling is an admission test, not an owned timer queue. With CLetter's
// table, raw vtable+0x30 resolves to IActor_OnRestorePreserved (empty default,
// task_registry_actor_and_debug_text.cpp / phase1_class_tables.cpp); it receives self ONLY. It cannot
// retain the packet because no packet argument is passed and none is installed
// on self here. No future-message retry is registered by this callback.
// IActor_DispatchMessage constructs its packet on the caller stack and delivers
// synchronously; it neither owns nor deep-copies pointer payloads. Its final
// argument is absolute due time (+1C). For delayed delivery the sending system
// must keep/recreate its own packet and pointed-to strings/options/float until
// it explicitly redispatches at dueTime<=g_currentTimeMilliseconds. A one-shot
// future dispatch is simply ignored by this receiver. Do not retain its stack
// address or infer a retry guarantee from the historical restore-hook name.
void CLetter_HandleMsg(void* selfPtr, void* messagePointer) {
    // Phase 1: future messages invoke the +0x30 virtual hook and return. The
    // dispatcher does not wait, copy, retain or free the message payload itself.
    unsigned char* self = (unsigned char*)selfPtr;
    void* message = messagePointer;
    unsigned int* messageWords = (unsigned int*)message;
    if (((LetterMessage*)message)->scheduledTime > g_currentTimeMilliseconds) {
        ((void(**)(void*))(*(void**)self))[0x30/4](self);
        return;
    }
    unsigned int messageType = ((LetterMessage*)message)->messageType;
    if (messageType > 7) return;
    switch (messageType) {
    // Phase 2: translate the message union according to its type, as documented
    // at LetterMessage. Registry resource pointers remain borrowed throughout.
    case kLetterReplay: {
        char* replayName = ((LetterOverlayStrings*)golfLetterOverlayStrings)->replay;
        void* overlay = LetterOverlay_FindByName((char*)self, replayName);
        AsCLetter(self)->replayIcon = overlay;
        AsCLetter(self)->replayBlinkTimer = kFloat_0_5_Rival;
        break;
    }
    case kLetterFastForward: {
        char* fastForwardName = ((LetterOverlayStrings*)golfLetterOverlayStrings)->fastForward;
        void* overlay = LetterOverlay_FindByName((char*)self, fastForwardName);
        AsCLetter(self)->fastForwardIcon = overlay;
        break;
    }
    case kLetterClear: {
        void* currentOverlay = AsCLetter(self)->currentMessage;
        if (currentOverlay && ((LetterMessage*)message)->nameOrEnabled.pointer) {
            if (strstr(((LetterOverlayResource*)currentOverlay)->resourceName, ((LetterMessage*)message)->nameOrEnabled.pointer) != 0) {
                AsCLetter(self)->currentMessage = 0;
            }
        } else {
            AsCLetter(self)->currentMessage = 0;
            AsCLetter(self)->replayIcon = 0;
            AsCLetter(self)->fastForwardIcon = 0;
        }
        GolfTipsDialog_RequestClose(-1, kFloat_0_Letter);
        break;
    }
    case kLetterFadeIn:
        AsCLetter(self)->messageState = kLetterFadingIn;
        break;
    case kLetterFadeOut:
        AsCLetter(self)->messageState = kLetterFadingOut;
        GolfTipsDialog_RequestClose(-1, kFloat_0_5_Rival);
        break;
    case kLetterShow: {
        char* overlayName = (char*)((LetterMessage*)message)->nameOrEnabled.pointer;
        void* result = LetterOverlay_FindByName((char*)self, overlayName);
        AsCLetter(self)->currentMessage = result;
        AsCLetter(self)->scriptedMessageIndex = 0;
        // Snapshot the known overlay-name table before matching the requested name.
        struct KnownOverlayNames { void* a[16]; };
        KnownOverlayNames knownNames = *(KnownOverlayNames*)s_letterKnownMessageNames;
        void* requestedName = ((LetterMessage*)message)->nameOrEnabled.pointer;
        if (requestedName) {
            int knownNameIndex = 1;
            char** knownNameCursor = (char**)(knownNames.a + 1);
            while (knownNameIndex < 16) {
                if (strcmp(requestedName, *knownNameCursor) == 0) {
                    AsCLetter(self)->scriptedMessageIndex = knownNameIndex;
                    break;
                }
                knownNameIndex++;
                knownNameCursor++;
            }
        }
        // message->0xc
        void* messageValue = ((LetterMessage*)message)->value;
        if (messageValue) {
            AsCLetter(self)->knownMessageValue = (AsCLetter(self)->scriptedMessageIndex) ? *(float*)messageValue : kFloat_0_Letter;
        } else {
            AsCLetter(self)->knownMessageValue = kFloat_0_Letter;
        }
        if (AsCLetter(self)->scriptedMessageIndex) {
            AsCLetter(self)->messageState = kLetterFadingIn;
        }
        if (AsCLetter(self)->scriptedMessageIndex) {
            unsigned int nameIndexOrEnabled = (*(unsigned int*)&AsCLetter(self)->scriptedMessageIndex);
            void* knownName = knownNames.a[nameIndexOrEnabled];
            if (strcmp(knownName, ((LetterOverlayStrings*)golfLetterOverlayStrings)->replaySuppressedName) == 0 && AsCLetter(self)->replayIcon) {
                AsCLetter(self)->currentMessage = 0;
                AsCLetter(self)->replayIcon = 0;
                AsCLetter(self)->fastForwardIcon = 0;
            }
        }
        {
            unsigned char* node = (unsigned char*)AsCLetter(self)->currentMessage;
            if (node) {
                float* overlayValue = (float*)((LetterMessage*)message)->value;
                ((LetterOverlayResource*)node)->messageValue = overlayValue ? *overlayValue : kFloat_0_Letter;
                if (((LetterMessage*)message)->showImmediately) {
                    AsCLetter(self)->messageState = kLetterFullyVisible;
                    AsCLetter(self)->messageAlpha = kFloat_255_LetterFade;
                } else {
                    AsCLetter(self)->messageState = kLetterFadingIn;
                    AsCLetter(self)->messageAlpha = kFloat_0_Letter;
                }
            }
        }
        break;
    }
    case kLetterSetOptions: {
        char* overlayName = (char*)((LetterMessage*)message)->nameOrEnabled.pointer;
        void* result = LetterOverlay_FindByName((char*)self, overlayName);
        AsCLetter(self)->currentMessage = result;
        if (result) {
            void* options = ((LetterMessage*)message)->value;
            unsigned char* node = (unsigned char*)result;
            ((LetterOverlayResource*)node)->options.showCaption = ((LetterOverlayOptions*)options)->showCaption;
            ((LetterOverlayResource*)node)->options.captionValue = ((LetterOverlayOptions*)options)->captionValue;
            ((LetterOverlayResource*)node)->options.glyphHeightScale = ((LetterOverlayOptions*)options)->glyphHeightScale;
            ((LetterOverlayResource*)node)->options.packedColor = ((LetterOverlayOptions*)options)->packedColor;
            unsigned char* currentOverlay = (unsigned char*)AsCLetter(self)->currentMessage;
            ((LetterOverlayResource*)currentOverlay)->options.showCaption = 1;
        }
        break;
    }
    case kLetterSetEnabled: {
        unsigned int enabledValue = ((LetterMessage*)message)->nameOrEnabled.integer;
        unsigned int nameIndexOrEnabled = (enabledValue == 1) ? 1 : 0;
        AsCLetter(self)->messageDisplayEnabled = (unsigned char)nameIndexOrEnabled;
        break;
    }
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462160", _ctors_u80194F9C_0, LetterOverlay_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80462160", _ctors_u80194F9C_1, MouseCursor_InitializeTwoPi);
#pragma pop
